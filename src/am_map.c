// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.org/
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2011 GhostlyDeath (ghostlydeath@gmail.com)
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION: the automap code

/* Includes */
#include "doomdef.h"
#include "doomstat.h"
#include "g_game.h"
#include "am_map.h"
#include "g_input.h"
#include "m_cheat.h"
#include "p_local.h"
#include "r_defs.h"
#include "v_video.h"
#include "st_stuff.h"
#include "i_system.h"
#include "i_video.h"
#include "r_state.h"
#include "dstrings.h"
#include "keys.h"
#include "w_wad.h"
#include "z_zone.h"
#include "r_draw.h"
#include "r_main.h"
#include "p_info.h"
#include "d_main.h"

// For use if I do walls with outsides/insides
static uint8_t REDS = (256 - 5 * 16);
static uint8_t REDRANGE = 16;
static uint8_t BLUES = (256 - 4 * 16 + 8);
static uint8_t BLUERANGE = 8;
static uint8_t GREENS = (7 * 16);
static uint8_t GREENRANGE = 16;
static uint8_t GRAYS = (6 * 16);
static uint8_t GRAYSRANGE = 16;
static uint8_t BROWNS = (4 * 16);
static uint8_t BROWNRANGE = 16;
static uint8_t YELLOWS = (256 - 32);
static uint8_t YELLOWRANGE = 8;
static uint8_t DBLACK = 0;
static uint8_t DWHITE = (256 - 47);

// Automap colors
#define BACKGROUND      DBLACK
#define YOURCOLORS      DWHITE
#define YOURRANGE       0
#define WALLCOLORS      REDS
#define WALLRANGE       REDRANGE
#define TSWALLCOLORS    GRAYS
#define TSWALLRANGE     GRAYSRANGE
#define FDWALLCOLORS    BROWNS
#define FDWALLRANGE     BROWNRANGE
#define CDWALLCOLORS    (YELLOWS + 7)
#define CDWALLRANGE     YELLOWRANGE
#define THINGCOLORS     GREENS
#define THINGRANGE      GREENRANGE
#define SECRETWALLCOLORS WALLCOLORS
#define SECRETWALLRANGE WALLRANGE
#define GRIDCOLORS      (GRAYS + GRAYSRANGE/2)
#define GRIDRANGE       0
#define XHAIRCOLORS     GRAYS

// drawing stuff
#define FB              0

#define AM_PANDOWNKEY   KEY_DOWNARROW
#define AM_PANUPKEY     KEY_UPARROW
#define AM_PANRIGHTKEY  KEY_RIGHTARROW
#define AM_PANLEFTKEY   KEY_LEFTARROW
#define AM_ZOOMINKEY    '='
#define AM_ZOOMOUTKEY   '-'
#define AM_STARTKEY     KEY_TAB
#define AM_ENDKEY       KEY_TAB
#define AM_GOBIGKEY     '0'
#define AM_FOLLOWKEY    'f'
#define AM_GRIDKEY      'g'
#define AM_MARKKEY      'm'
#define AM_CLEARMARKKEY 'c'

#define AM_NUMMARKPOINTS 10

// scale on entry
#define INITSCALEMTOF (.2*FRACUNIT)
// how much the automap moves window per tic in frame-buffer coordinates
// moves 140 pixels in 1 second
#define F_PANINC        4
// how much zoom-in per tic
// goes to 2x in 1 second
#define M_ZOOMIN        ((int) (1.02*FRACUNIT))
// how much zoom-out per tic
// pulls out to 0.5x in 1 second
#define M_ZOOMOUT       ((int) (FRACUNIT/1.02))

// translates between frame-buffer and map distances
#define FTOM(x) FixedMul(((x)<<16),scale_ftom)
#define MTOF(x) (FixedMul((x),scale_mtof)>>16)
// translates between frame-buffer and map coordinates
#define CXMTOF(x)  (f_x + MTOF((x)-m_x))
#define CYMTOF(y)  (f_y + (f_h - MTOF((y)-m_y)))

// the following is crap
#define LINE_NEVERSEE ML_DONTDRAW

typedef struct
{
	fixed_t x, y;
} mpoint_t;

typedef struct
{
	mpoint_t a, b;
} mline_t;

typedef struct
{
	fixed_t slp, islp;
} islope_t;

//
// The vector graphics for the automap.
//  A line drawing of the player pointing right,
//   starting from the middle.
//
#define R ((8*PLAYERRADIUS)/7)
mline_t player_arrow[] = {
	{{-R + R / 8, 0}, {R, 0}},	// -----
	{{R, 0}, {R - R / 2, R / 4}},	// ----->
	{{R, 0}, {R - R / 2, -R / 4}},
	{{-R + R / 8, 0}, {-R - R / 8, R / 4}},	// >---->
	{{-R + R / 8, 0}, {-R - R / 8, -R / 4}},
	{{-R + 3 * R / 8, 0}, {-R + R / 8, R / 4}},	// >>--->
	{{-R + 3 * R / 8, 0}, {-R + R / 8, -R / 4}}
};
#undef R
#define NUMPLYRLINES (sizeof(player_arrow)/sizeof(mline_t))

#define R ((8*PLAYERRADIUS)/7)
mline_t cheat_player_arrow[] = {
	{{-R + R / 8, 0}, {R, 0}},	// -----
	{{R, 0}, {R - R / 2, R / 6}},	// ----->
	{{R, 0}, {R - R / 2, -R / 6}},
	{{-R + R / 8, 0}, {-R - R / 8, R / 6}},	// >----->
	{{-R + R / 8, 0}, {-R - R / 8, -R / 6}},
	{{-R + 3 * R / 8, 0}, {-R + R / 8, R / 6}},	// >>----->
	{{-R + 3 * R / 8, 0}, {-R + R / 8, -R / 6}},
	{{-R / 2, 0}, {-R / 2, -R / 6}},	// >>-d--->
	{{-R / 2, -R / 6}, {-R / 2 + R / 6, -R / 6}},
	{{-R / 2 + R / 6, -R / 6}, {-R / 2 + R / 6, R / 4}},
	{{-R / 6, 0}, {-R / 6, -R / 6}},	// >>-dd-->
	{{-R / 6, -R / 6}, {0, -R / 6}},
	{{0, -R / 6}, {0, R / 4}},
	{{R / 6, R / 4}, {R / 6, -R / 7}},	// >>-ddt->
	{{R / 6, -R / 7}, {R / 6 + R / 32, -R / 7 - R / 32}},
	{{R / 6 + R / 32, -R / 7 - R / 32}, {R / 6 + R / 10, -R / 7}}
};
#undef R
#define NUMCHEATPLYRLINES (sizeof(cheat_player_arrow)/sizeof(mline_t))

#define R (FRACUNIT)
mline_t triangle_guy[] = {
	{{(fixed_t) - .867 * R, (fixed_t) - .5 * R},
	 {(fixed_t) .867 * R, (fixed_t) - .5 * R}},
	{{(fixed_t) .867 * R, (fixed_t) - .5 * R}, {(fixed_t) 0, (fixed_t) R}},
	{{(fixed_t) 0, (fixed_t) R}, {(fixed_t) - .867 * R, (fixed_t) - .5 * R}}
};
#undef R
#define NUMTRIANGLEGUYLINES (sizeof(triangle_guy)/sizeof(mline_t))

#define R (FRACUNIT)
mline_t thintriangle_guy[] = {
	{{-.5 * R, -.7 * R}, {R, 0}},
	{{R, 0}, {-.5 * R, .7 * R}},
	{{-.5 * R, .7 * R}, {-.5 * R, -.7 * R}}
};
#undef R
#define NUMTHINTRIANGLEGUYLINES (sizeof(thintriangle_guy)/sizeof(mline_t))

static int bigstate;			//added:24-01-98:moved here, toggle between
							   // user view and large view (full map view)

int am_cheating = 0;
static int grid = 0;

static int leveljuststarted = 1;	// kluge until AM_LevelInit() is called

bool_t automapactive = false;
bool_t automapoverlay = false;
bool_t am_recalc = false;		//added:05-02-98:true when screen size
									   //               changes

// location of window on screen
static int f_x;
static int f_y;

// size of window on screen
static int f_w;
static int f_h;

static int lightlev;			// used for funky strobing effect
static uint8_t *fb;				// pseudo-frame buffer
static int amclock;

static mpoint_t m_paninc;		// how far the window pans each tic (map coords)
static fixed_t mtof_zoommul;	// how far the window zooms in each tic (map coords)
static fixed_t ftom_zoommul;	// how far the window zooms in each tic (fb coords)

static fixed_t m_x, m_y;		// LL x,y where the window is on the map (map coords)
static fixed_t m_x2, m_y2;		// UR x,y where the window is on the map (map coords)

//
// width/height of window on map (map coords)
//
static fixed_t m_w;
static fixed_t m_h;

// based on level size
static fixed_t min_x;
static fixed_t min_y;
static fixed_t max_x;
static fixed_t max_y;

static fixed_t max_w;			// max_x-min_x,
static fixed_t max_h;			// max_y-min_y

// based on player size
static fixed_t min_w;
static fixed_t min_h;

static fixed_t min_scale_mtof;	// used to tell when to stop zooming out
static fixed_t max_scale_mtof;	// used to tell when to stop zooming in

// old stuff for recovery later
static fixed_t old_m_w, old_m_h;
static fixed_t old_m_x, old_m_y;

// old location used by the Follower routine
static mpoint_t f_oldloc;

// used by MTOF to scale from map-to-frame-buffer coords
static fixed_t scale_mtof = INITSCALEMTOF;
// used by FTOM to scale from frame-buffer-to-map coords (=1/scale_mtof)
static fixed_t scale_ftom;

static player_t *plr;			// the player represented by an arrow

static patch_t *marknums[10];	// numbers used for marking by the automap
static mpoint_t markpoints[AM_NUMMARKPOINTS];	// where the points are
static int markpointnum = 0;	// next point to be assigned

static int followplayer = 1;	// specifies whether to follow the player around

static bool_t stopped = true;

static uint8_t BLUEKEYCOLOR;
static uint8_t YELLOWKEYCOLOR;
static uint8_t REDKEYCOLOR;

// function for drawing lines, depends on rendermode
typedef void (*AMDRAWFLINEFUNC) (fline_t * fl, int color);
static AMDRAWFLINEFUNC AM_drawFline;

void V_MarkRect(int x, int y, int width, int height);

void AM_drawFline_soft(fline_t * fl, int color);

// Calculates the slope and slope according to the x-axis of a line
// segment in map coordinates (with the upright y-axis n' all) so
// that it can be used with the brain-dead drawing stuff.

void AM_getIslope(mline_t * ml, islope_t * is)
{
	int dx, dy;

	dy = ml->a.y - ml->b.y;
	dx = ml->b.x - ml->a.x;
	if (!dy)
		is->islp = (dx < 0 ? -MAXINT : MAXINT);
	else
		is->islp = FixedDiv(dx, dy);
	if (!dx)
		is->slp = (dy < 0 ? -MAXINT : MAXINT);
	else
		is->slp = FixedDiv(dy, dx);

}

//
void AM_activateNewScale(void)
{
	m_x += m_w / 2;
	m_y += m_h / 2;
	m_w = FTOM(f_w);
	m_h = FTOM(f_h);
	m_x -= m_w / 2;
	m_y -= m_h / 2;
	m_x2 = m_x + m_w;
	m_y2 = m_y + m_h;
}

//
void AM_saveScaleAndLoc(void)
{
	old_m_x = m_x;
	old_m_y = m_y;
	old_m_w = m_w;
	old_m_h = m_h;
}

//
void AM_restoreScaleAndLoc(void)
{

	m_w = old_m_w;
	m_h = old_m_h;
	if (!followplayer)
	{
		m_x = old_m_x;
		m_y = old_m_y;
	}
	else
	{
		m_x = plr->mo->x - m_w / 2;
		m_y = plr->mo->y - m_h / 2;
	}
	m_x2 = m_x + m_w;
	m_y2 = m_y + m_h;

	// Change the scaling multipliers
	scale_mtof = FixedDiv(f_w << FRACBITS, m_w);
	scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

//
// adds a marker at the current location
//
void AM_addMark(void)
{
	markpoints[markpointnum].x = m_x + m_w / 2;
	markpoints[markpointnum].y = m_y + m_h / 2;
	markpointnum = (markpointnum + 1) % AM_NUMMARKPOINTS;

}

//
// Determines bounding box of all vertices,
// sets global variables controlling zoom range.
//
void AM_findMinMaxBoundaries(void)
{
	int i;
	fixed_t a;
	fixed_t b;

	min_x = min_y = MAXINT;
	max_x = max_y = -MAXINT;

	for (i = 0; i < numvertexes; i++)
	{
		if (vertexes[i].x < min_x)
			min_x = vertexes[i].x;
		else if (vertexes[i].x > max_x)
			max_x = vertexes[i].x;

		if (vertexes[i].y < min_y)
			min_y = vertexes[i].y;
		else if (vertexes[i].y > max_y)
			max_y = vertexes[i].y;
	}

	max_w = max_x - min_x;
	max_h = max_y - min_y;

	min_w = 2 * PLAYERRADIUS;	// const? never changed?
	min_h = 2 * PLAYERRADIUS;

	a = FixedDiv(f_w << FRACBITS, max_w);
	b = FixedDiv(f_h << FRACBITS, max_h);

	min_scale_mtof = a < b ? a : b;
	max_scale_mtof = FixedDiv(f_h << FRACBITS, 2 * PLAYERRADIUS);

}

//
void AM_changeWindowLoc(void)
{
	if (m_paninc.x || m_paninc.y)
	{
		followplayer = 0;
		f_oldloc.x = MAXINT;
	}

	m_x += m_paninc.x;
	m_y += m_paninc.y;

	if (m_x + m_w / 2 > max_x)
		m_x = max_x - m_w / 2;
	else if (m_x + m_w / 2 < min_x)
		m_x = min_x - m_w / 2;

	if (m_y + m_h / 2 > max_y)
		m_y = max_y - m_h / 2;
	else if (m_y + m_h / 2 < min_y)
		m_y = min_y - m_h / 2;

	m_x2 = m_x + m_w;
	m_y2 = m_y + m_h;
}

//
static void AM_initVariables(void)
{
	int pnum;
	static event_t st_notify = { ev_keyup, AM_MSGENTERED };

	automapactive = true;
	automapoverlay = false;
	fb = screens[0];

	f_oldloc.x = MAXINT;
	amclock = 0;
	lightlev = 0;

	m_paninc.x = m_paninc.y = 0;
	ftom_zoommul = FRACUNIT;
	mtof_zoommul = FRACUNIT;

	m_w = FTOM(f_w);
	m_h = FTOM(f_h);

	// find player to center on initially
	if (!playeringame[pnum = consoleplayer[0]])
		for (pnum = 0; pnum < MAXPLAYERS; pnum++)
			if (playeringame[pnum])
				break;

	plr = &players[pnum];
	m_x = plr->mo->x - m_w / 2;
	m_y = plr->mo->y - m_h / 2;
	AM_changeWindowLoc();

	// for saving & restoring
	old_m_x = m_x;
	old_m_y = m_y;
	old_m_w = m_w;
	old_m_h = m_h;
	

	BLUEKEYCOLOR = 200;
	YELLOWKEYCOLOR = 231;
	REDKEYCOLOR = 176;

	// inform the status bar of the change
	ST_Responder(&st_notify);

}

static uint8_t *maplump;			// pointer to the raw data for the automap background.

//
static void AM_loadPics(void)
{
	int i;
	char namebuf[9];

	for (i = 0; i < 10; i++)
	{
		sprintf(namebuf, "AMMNUM%d", i);
		marknums[i] = W_CachePatchName(namebuf, PU_STATIC);
	}
	if (W_CheckNumForName("AUTOPAGE") >= 0)
		maplump = W_CacheLumpName("AUTOPAGE", PU_STATIC);
	else
		maplump = NULL;
}

static void AM_unloadPics(void)
{
	int i;
	
	for (i = 0; i < 10; i++)
		Z_ChangeTag(marknums[i], PU_CACHE);
	if (maplump)
		Z_ChangeTag(maplump, PU_CACHE);
}

void AM_clearMarks(void)
{
	int i;

	for (i = 0; i < AM_NUMMARKPOINTS; i++)
		markpoints[i].x = -1;	// means empty
	markpointnum = 0;
}

//
// should be called at the start of every level
// right now, i figure it out myself
//
void AM_LevelInit(void)
{
	leveljuststarted = 0;

	f_x = f_y = 0;
	f_w = vid.width;
	f_h = vid.height - stbarheight;

	AM_drawFline = AM_drawFline_soft;

	AM_clearMarks();

	AM_findMinMaxBoundaries();
	scale_mtof = FixedDiv(min_scale_mtof, (int)(0.7 * FRACUNIT));
	if (scale_mtof > max_scale_mtof)
		scale_mtof = min_scale_mtof;
	scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

//
void AM_Stop(void)
{
	static event_t st_notify = { 0, ev_keyup, AM_MSGEXITED };

	AM_unloadPics();
	automapactive = false;
	automapoverlay = false;
	ST_Responder(&st_notify);
	stopped = true;
}

//
void AM_Start(void)
{
	static int lastlevel = -1, lastepisode = -1;

	if (!stopped)
		AM_Stop();
	stopped = false;
	if (lastlevel != gamemap || lastepisode != gameepisode || am_recalc)	//added:05-02-98:screen size changed
	{
		am_recalc = false;

		AM_LevelInit();
		lastlevel = gamemap;
		lastepisode = gameepisode;
	}
	AM_initVariables();
	AM_loadPics();
}

//
// set the window scale to the maximum size
//
void AM_minOutWindowScale(void)
{
	scale_mtof = min_scale_mtof;
	scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
	AM_activateNewScale();
}

//
// set the window scale to the minimum size
//
void AM_maxOutWindowScale(void)
{
	scale_mtof = max_scale_mtof;
	scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
	AM_activateNewScale();
}

//
// Handle events (user inputs) in automap mode
//
bool_t AM_Responder(event_t * ev)
{

	int rc;
	static int cheatstate = 0;
	static char buffer[20];

	rc = false;

	if (!automapactive)
	{
		if (ev->type == ev_keydown && ev->data1 == AM_STARTKEY)
		{
			//faB: prevent alt-tab in win32 version to activate automap just before minimizing the app
			//         doesn't do any harm to the DOS version
			if (!gamekeydown[KEY_ALT])
			{
				bigstate = 0;	//added:24-01-98:toggle off large view
				AM_Start();
				rc = true;
			}
		}
	}

	else if (ev->type == ev_keydown)
	{

		rc = true;
		switch (ev->data1)
		{
			case AM_PANRIGHTKEY:	// pan right
				if (!followplayer)
					m_paninc.x = FTOM(F_PANINC);
				else
					rc = false;
				break;
			case AM_PANLEFTKEY:	// pan left
				if (!followplayer)
					m_paninc.x = -FTOM(F_PANINC);
				else
					rc = false;
				break;
			case AM_PANUPKEY:	// pan up
				if (!followplayer)
					m_paninc.y = FTOM(F_PANINC);
				else
					rc = false;
				break;
			case AM_PANDOWNKEY:	// pan down
				if (!followplayer)
					m_paninc.y = -FTOM(F_PANINC);
				else
					rc = false;
				break;
			case AM_ZOOMOUTKEY:	// zoom out
				mtof_zoommul = M_ZOOMOUT;
				ftom_zoommul = M_ZOOMIN;
				break;
			case AM_ZOOMINKEY:	// zoom in
				mtof_zoommul = M_ZOOMIN;
				ftom_zoommul = M_ZOOMOUT;
				break;
			case AM_ENDKEY:
				// GhostlyDeath <June 13, 2008> -- Overlay hack
				if (!automapoverlay)
					automapoverlay = true;
				else
				{
					automapoverlay = true;
					AM_Stop();
				}
				break;
			case AM_GOBIGKEY:
				bigstate = !bigstate;
				if (bigstate)
				{
					AM_saveScaleAndLoc();
					AM_minOutWindowScale();
				}
				else
					AM_restoreScaleAndLoc();
				break;
			case AM_FOLLOWKEY:
				followplayer = !followplayer;
				f_oldloc.x = MAXINT;
				plr->message = followplayer ? AMSTR_FOLLOWON : AMSTR_FOLLOWOFF;
				break;
			case AM_GRIDKEY:
				grid = !grid;
				plr->message = grid ? AMSTR_GRIDON : AMSTR_GRIDOFF;
				break;
			case AM_MARKKEY:
				sprintf(buffer, "%s %d", AMSTR_MARKEDSPOT, markpointnum);
				plr->message = buffer;
				AM_addMark();
				break;
			case AM_CLEARMARKKEY:
				AM_clearMarks();
				plr->message = AMSTR_MARKSCLEARED;
				break;
			default:
				cheatstate = 0;
				rc = false;
		}
	}

	else if (ev->type == ev_keyup)
	{
		rc = false;
		switch (ev->data1)
		{
			case AM_PANRIGHTKEY:
				if (!followplayer)
					m_paninc.x = 0;
				break;
			case AM_PANLEFTKEY:
				if (!followplayer)
					m_paninc.x = 0;
				break;
			case AM_PANUPKEY:
				if (!followplayer)
					m_paninc.y = 0;
				break;
			case AM_PANDOWNKEY:
				if (!followplayer)
					m_paninc.y = 0;
				break;
			case AM_ZOOMOUTKEY:
			case AM_ZOOMINKEY:
				mtof_zoommul = FRACUNIT;
				ftom_zoommul = FRACUNIT;
				break;
		}
	}

	return rc;

}

//
// Zooming
//
void AM_changeWindowScale(void)
{

	// Change the scaling multipliers
	scale_mtof = FixedMul(scale_mtof, mtof_zoommul);
	scale_ftom = FixedDiv(FRACUNIT, scale_mtof);

	if (scale_mtof < min_scale_mtof)
		AM_minOutWindowScale();
	else if (scale_mtof > max_scale_mtof)
		AM_maxOutWindowScale();
	else
		AM_activateNewScale();
}

//
void AM_doFollowPlayer(void)
{

	if (f_oldloc.x != plr->mo->x || f_oldloc.y != plr->mo->y)
	{
		m_x = FTOM(MTOF(plr->mo->x)) - m_w / 2;
		m_y = FTOM(MTOF(plr->mo->y)) - m_h / 2;
		m_x2 = m_x + m_w;
		m_y2 = m_y + m_h;
		f_oldloc.x = plr->mo->x;
		f_oldloc.y = plr->mo->y;

		//  m_x = FTOM(MTOF(plr->mo->x - m_w/2));
		//  m_y = FTOM(MTOF(plr->mo->y - m_h/2));
		//  m_x = plr->mo->x - m_w/2;
		//  m_y = plr->mo->y - m_h/2;

	}

}

//
void AM_updateLightLev(void)
{
	static int nexttic = 0;
	//static int litelevels[] = { 0, 3, 5, 6, 6, 7, 7, 7 };
	static int litelevels[] = { 0, 4, 7, 10, 12, 14, 15, 15 };
	static int litelevelscnt = 0;

	// Change light level
	if (amclock > nexttic)
	{
		lightlev = litelevels[litelevelscnt++];
		if (litelevelscnt == sizeof(litelevels) / sizeof(int))
			litelevelscnt = 0;
		nexttic = amclock + 6 - (amclock % 6);
	}

}

//
// Updates on Game Tick
//
void AM_Ticker(void)
{
	if (dedicated)
		return;

	if (!automapactive)
		return;

	amclock++;

	if (followplayer)
		AM_doFollowPlayer();

	// Change the zoom if necessary
	if (ftom_zoommul != FRACUNIT)
		AM_changeWindowScale();

	// Change x,y location
	if (m_paninc.x || m_paninc.y)
		AM_changeWindowLoc();

	// Update light level
	// AM_updateLightLev();

}

//
// Clear automap frame buffer.
//
void AM_clearFB(int color)
{
	if (!maplump)
	{
		memset(fb, color, f_w * f_h * vid.bpp);
	}
	else
	{
		int i, y;
		int dmapx;
		int dmapy;
		static int mapxstart;
		static int mapystart;
		uint8_t *dest = screens[0], *src;
#define MAPLUMPHEIGHT (200-SBARHEIGHT)

		if (followplayer)
		{
			static vertex_t oldplr;

			dmapx = (MTOF(plr->mo->x) - MTOF(oldplr.x));	//fixed point
			dmapy = (MTOF(oldplr.y) - MTOF(plr->mo->y));

			oldplr.x = plr->mo->x;
			oldplr.y = plr->mo->y;
			mapxstart += dmapx >> 1;
			mapystart += dmapy >> 1;

			while (mapxstart >= 320)
				mapxstart -= 320;
			while (mapxstart < 0)
				mapxstart += 320;
			while (mapystart >= MAPLUMPHEIGHT)
				mapystart -= MAPLUMPHEIGHT;
			while (mapystart < 0)
				mapystart += MAPLUMPHEIGHT;
		}
		else
		{
			mapxstart += (MTOF(m_paninc.x) >> 1);
			mapystart -= (MTOF(m_paninc.y) >> 1);
			if (mapxstart >= 320)
				mapxstart -= 320;
			if (mapxstart < 0)
				mapxstart += 320;
			if (mapystart >= MAPLUMPHEIGHT)
				mapystart -= MAPLUMPHEIGHT;
			if (mapystart < 0)
				mapystart += MAPLUMPHEIGHT;
		}

		//blit the automap background to the screen.
		if (!automapoverlay)
			for (y = 0; y < f_h; y++)
			{
				src = maplump + mapxstart + (y + mapystart) * 320;
				for (i = 0; i < 320 * vid.dupx; i++)
				{
					while (src > maplump + 320 * MAPLUMPHEIGHT)
						src -= 320 * MAPLUMPHEIGHT;
					*dest++ = *src++;
				}
				dest += vid.width - vid.dupx * 320;
			}
	}
}

//
// Automap clipping of lines.
//
// Based on Cohen-Sutherland clipping algorithm but with a slightly
// faster reject and precalculated slopes.  If the speed is needed,
// use a hash algorithm to handle  the common cases.
//
bool_t AM_clipMline(mline_t * ml, fline_t * fl)
{
	enum
	{
		LEFT = 1,
		RIGHT = 2,
		BOTTOM = 4,
		TOP = 8
	};

	register int outcode1 = 0;
	register int outcode2 = 0;
	register int outside;

	fpoint_t tmp;
	int dx;
	int dy;

#define DOOUTCODE(oc, mx, my) \
    (oc) = 0; \
    if ((my) < 0) (oc) |= TOP; \
    else if ((my) >= f_h) (oc) |= BOTTOM; \
    if ((mx) < 0) (oc) |= LEFT; \
    else if ((mx) >= f_w) (oc) |= RIGHT;

	// do trivial rejects and outcodes
	if (ml->a.y > m_y2)
		outcode1 = TOP;
	else if (ml->a.y < m_y)
		outcode1 = BOTTOM;

	if (ml->b.y > m_y2)
		outcode2 = TOP;
	else if (ml->b.y < m_y)
		outcode2 = BOTTOM;

	if (outcode1 & outcode2)
		return false;			// trivially outside

	if (ml->a.x < m_x)
		outcode1 |= LEFT;
	else if (ml->a.x > m_x2)
		outcode1 |= RIGHT;

	if (ml->b.x < m_x)
		outcode2 |= LEFT;
	else if (ml->b.x > m_x2)
		outcode2 |= RIGHT;

	if (outcode1 & outcode2)
		return false;			// trivially outside

	// transform to frame-buffer coordinates.
	fl->a.x = CXMTOF(ml->a.x);
	fl->a.y = CYMTOF(ml->a.y);
	fl->b.x = CXMTOF(ml->b.x);
	fl->b.y = CYMTOF(ml->b.y);

	DOOUTCODE(outcode1, fl->a.x, fl->a.y);
	DOOUTCODE(outcode2, fl->b.x, fl->b.y);

	if (outcode1 & outcode2)
		return false;

	while (outcode1 | outcode2)
	{
		// may be partially inside box
		// find an outside point
		if (outcode1)
			outside = outcode1;
		else
			outside = outcode2;

		// clip to each side
		if (outside & TOP)
		{
			dy = fl->a.y - fl->b.y;
			dx = fl->b.x - fl->a.x;
			tmp.x = fl->a.x + (dx * (fl->a.y)) / dy;
			tmp.y = 0;
		}
		else if (outside & BOTTOM)
		{
			dy = fl->a.y - fl->b.y;
			dx = fl->b.x - fl->a.x;
			tmp.x = fl->a.x + (dx * (fl->a.y - f_h)) / dy;
			tmp.y = f_h - 1;
		}
		else if (outside & RIGHT)
		{
			dy = fl->b.y - fl->a.y;
			dx = fl->b.x - fl->a.x;
			tmp.y = fl->a.y + (dy * (f_w - 1 - fl->a.x)) / dx;
			tmp.x = f_w - 1;
		}
		else if (outside & LEFT)
		{
			dy = fl->b.y - fl->a.y;
			dx = fl->b.x - fl->a.x;
			tmp.y = fl->a.y + (dy * (-fl->a.x)) / dx;
			tmp.x = 0;
		}

		if (outside == outcode1)
		{
			fl->a = tmp;
			DOOUTCODE(outcode1, fl->a.x, fl->a.y);
		}
		else
		{
			fl->b = tmp;
			DOOUTCODE(outcode2, fl->b.x, fl->b.y);
		}

		if (outcode1 & outcode2)
			return false;		// trivially outside
	}

	return true;
}
#undef DOOUTCODE

//
// Classic Bresenham w/ whatever optimizations needed for speed
//
void AM_drawFline_soft(fline_t * fl, int color)
{
	register int x;
	register int y;
	register int dx;
	register int dy;
	register int sx;
	register int sy;
	register int ax;
	register int ay;
	register int d;

#ifdef PARANOIA
	static int fuck = 0;

	// For debugging only
	if (fl->a.x < 0 || fl->a.x >= f_w
		|| fl->a.y < 0 || fl->a.y >= f_h
		|| fl->b.x < 0 || fl->b.x >= f_w || fl->b.y < 0 || fl->b.y >= f_h)
	{
		CONS_Printf("line clipping problem %d \r", fuck++);
		return;
	}
#endif

#define PUTDOT(xx,yy,cc) fb[(yy)*f_w+(xx)]=(cc)

	dx = fl->b.x - fl->a.x;
	ax = 2 * (dx < 0 ? -dx : dx);
	sx = dx < 0 ? -1 : 1;

	dy = fl->b.y - fl->a.y;
	ay = 2 * (dy < 0 ? -dy : dy);
	sy = dy < 0 ? -1 : 1;

	x = fl->a.x;
	y = fl->a.y;

	if (ax > ay)
	{
		d = ay - ax / 2;
		for (;;)
		{
			PUTDOT(x, y, color);
			if (x == fl->b.x)
				return;
			if (d >= 0)
			{
				y += sy;
				d -= ax;
			}
			x += sx;
			d += ay;
		}
	}
	else
	{
		d = ax - ay / 2;
		for (;;)
		{
			PUTDOT(x, y, color);
			if (y == fl->b.y)
				return;
			if (d >= 0)
			{
				x += sx;
				d -= ay;
			}
			y += sy;
			d += ax;
		}
	}
}

//
// Clip lines, draw visible parts of lines.
//
void AM_drawMline(mline_t * ml, int color)
{
	static fline_t fl;

	if (AM_clipMline(ml, &fl))
		AM_drawFline(&fl, color);	// draws it on frame buffer using fb coords
}

//
// Draws flat (floor/ceiling tile) aligned grid lines.
//
void AM_drawGrid(int color)
{
	fixed_t x, y;
	fixed_t start, end;
	mline_t ml;

	// Figure out start of vertical gridlines
	start = m_x;
	if ((start - bmaporgx) % (MAPBLOCKUNITS << FRACBITS))
		start += (MAPBLOCKUNITS << FRACBITS) - ((start - bmaporgx) % (MAPBLOCKUNITS << FRACBITS));
	end = m_x + m_w;

	// draw vertical gridlines
	ml.a.y = m_y;
	ml.b.y = m_y + m_h;
	for (x = start; x < end; x += (MAPBLOCKUNITS << FRACBITS))
	{
		ml.a.x = x;
		ml.b.x = x;
		AM_drawMline(&ml, color);
	}

	// Figure out start of horizontal gridlines
	start = m_y;
	if ((start - bmaporgy) % (MAPBLOCKUNITS << FRACBITS))
		start += (MAPBLOCKUNITS << FRACBITS) - ((start - bmaporgy) % (MAPBLOCKUNITS << FRACBITS));
	end = m_y + m_h;

	// draw horizontal gridlines
	ml.a.x = m_x;
	ml.b.x = m_x + m_w;
	for (y = start; y < end; y += (MAPBLOCKUNITS << FRACBITS))
	{
		ml.a.y = y;
		ml.b.y = y;
		AM_drawMline(&ml, color);
	}
}

//
// Determines visible lines, draws them.
// This is LineDef based, not LineSeg based.
//
void AM_drawWalls(void)
{
	int i;
	static mline_t l;

	for (i = 0; i < numlines; i++)
	{
		l.a.x = lines[i].v1->x;
		l.a.y = lines[i].v1->y;
		l.b.x = lines[i].v2->x;
		l.b.y = lines[i].v2->y;
		if (am_cheating || (lines[i].flags & ML_MAPPED))
		{
			if ((lines[i].flags & LINE_NEVERSEE) && !am_cheating)
				continue;
			if (!lines[i].backsector)
			{
				AM_drawMline(&l, WALLCOLORS + lightlev);
			}
			else
			{
				switch (lines[i].special)
				{
					case 39:
						// teleporters
						AM_drawMline(&l, WALLCOLORS + WALLRANGE / 2);
						break;
					case 26:
					case 32:
						AM_drawMline(&l, BLUEKEYCOLOR);
						break;
					case 27:
					case 34:
						AM_drawMline(&l, YELLOWKEYCOLOR);
						break;
					case 28:
					case 33:
						AM_drawMline(&l, REDKEYCOLOR);	// green for heretic
						break;
					default:
						if (lines[i].flags & ML_SECRET)	// secret door
						{
							if (am_cheating)
								AM_drawMline(&l, SECRETWALLCOLORS + lightlev);
							else
								AM_drawMline(&l, WALLCOLORS + lightlev);
						}
						else if (lines[i].backsector->floorheight
								 != lines[i].frontsector->floorheight)
						{
							AM_drawMline(&l, FDWALLCOLORS + lightlev);	// floor level change
						}
						else if (lines[i].backsector->ceilingheight
								 != lines[i].frontsector->ceilingheight)
						{
							AM_drawMline(&l, CDWALLCOLORS + lightlev);	// ceiling level change
						}
						else if (am_cheating)
						{
							AM_drawMline(&l, TSWALLCOLORS + lightlev);
						}
				}
			}
		}
		else if (plr->powers[pw_allmap])
		{
			if (!(lines[i].flags & LINE_NEVERSEE))
				AM_drawMline(&l, GRAYS + 3);
		}
	}
}

//
// Rotation in 2D.
// Used to rotate player arrow line character.
//
void AM_rotate(fixed_t * x, fixed_t * y, angle_t a)
{
	fixed_t tmpx;

	tmpx =
		FixedMul(*x, finecosine[a >> ANGLETOFINESHIFT])
		- FixedMul(*y, finesine[a >> ANGLETOFINESHIFT]);

	*y = FixedMul(*x, finesine[a >> ANGLETOFINESHIFT])
		+ FixedMul(*y, finecosine[a >> ANGLETOFINESHIFT]);

	*x = tmpx;
}

void AM_drawLineCharacter(mline_t * lineguy,
						  int lineguylines,
						  fixed_t scale, angle_t angle, int color, fixed_t x, fixed_t y)
{
	int i;
	mline_t l;

	for (i = 0; i < lineguylines; i++)
	{
		l.a.x = lineguy[i].a.x;
		l.a.y = lineguy[i].a.y;

		if (scale)
		{
			l.a.x = FixedMul(scale, l.a.x);
			l.a.y = FixedMul(scale, l.a.y);
		}

		if (angle)
			AM_rotate(&l.a.x, &l.a.y, angle);

		l.a.x += x;
		l.a.y += y;

		l.b.x = lineguy[i].b.x;
		l.b.y = lineguy[i].b.y;

		if (scale)
		{
			l.b.x = FixedMul(scale, l.b.x);
			l.b.y = FixedMul(scale, l.b.y);
		}

		if (angle)
			AM_rotate(&l.b.x, &l.b.y, angle);

		l.b.x += x;
		l.b.y += y;

		AM_drawMline(&l, color);
	}
}

void AM_drawPlayers(void)
{
	int i;
	player_t *p;
	int color;

	if (!multiplayer)
	{
		if (am_cheating)
			AM_drawLineCharacter
				(cheat_player_arrow, NUMCHEATPLYRLINES, 0,
				 plr->mo->angle, DWHITE, plr->mo->x, plr->mo->y);
		else
			AM_drawLineCharacter
				(player_arrow, NUMPLYRLINES, 0, plr->mo->angle, DWHITE, plr->mo->x, plr->mo->y);
		return;
	}

	// multiplayer
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i])
			continue;

		p = &players[i];

		if ((cv_deathmatch.value && !singledemo) && p != plr)
			continue;

		if (p->powers[pw_invisibility])
			color = 246;		// *close* to black
		else
		{
			if (p->skincolor == 0)
				color = GREENS;
			else
				color = *(translationtables + ((p->skincolor - 1) << 8) + GREENS + 8);
		}

		AM_drawLineCharacter
			(player_arrow, NUMPLYRLINES, 0, p->mo->angle, color, p->mo->x, p->mo->y);
	}

}

void A_Look();

void AM_drawThings(int colors, int colorrange)
{
	int i;
	int color = colors;
	angle_t tangle;
	mobj_t *t;

	for (i = 0; i < numsectors; i++)
	{
		t = sectors[i].thinglist;
		
		while (t)
		{	
			// Modify Angle
			switch (t->type)
			{
				case MT_TFOG:
				case MT_IFOG:
					tangle = t->angle + (ANG45 * (gametic % 8));
					break;
					
				default:
					tangle = t->angle;
					break;
			}
			
			// Modify Color
			if (t->info->flags & MF_COUNTKILL || t->type == MT_SKULL)
			{
				if (t->health <= 0 || t->flags & MF_CORPSE)
					color = GRAYS + (GRAYSRANGE >> 1);
				else
				{
					if (t->target && t->state && t->state->action.acv != A_Look)
						color = REDS + (gametic % REDRANGE);
					else
						color = REDS + ((gametic >> 1) % REDRANGE);
				}
			}
			else if (t->info->flags & MF_SPECIAL)
			{
				switch (t->type)
				{
					case MT_MISC12:		// Soul Sphere
					case MT_MEGA:		// Mega Sphere
					case MT_INV:		// Invincibility Sphere
					case MT_INS:		// Partial Invisibility Sphere
					case MT_MISC13:		// Berzerker
					case MT_MISC14:		// Radiation Suit
					case MT_MISC15:		// Computer Map
					case MT_MISC16:		// Light Amplification Goggles
						color = YELLOWS + ((gametic >> 1) % YELLOWRANGE);
						break;
					
					default:
						color = YELLOWS + 7;
						break;
				}
			}
			else
				color = colors + lightlev;
			
			AM_drawLineCharacter
				(thintriangle_guy, NUMTHINTRIANGLEGUYLINES,
				 (t->info->radius > 0 ? t->info->radius / FRACUNIT: 2) << FRACBITS, tangle, color, t->x, t->y);
			t = t->snext;
		}
	}
}

void AM_drawMarks(void)
{
	int i, fx, fy, w, h;

	for (i = 0; i < AM_NUMMARKPOINTS; i++)
	{
		if (markpoints[i].x != -1)
		{
			//      w = LittleSwapInt16(marknums[i]->width);
			//      h = LittleSwapInt16(marknums[i]->height);
			w = 5;				// because something's wrong with the wad, i guess
			h = 6;				// because something's wrong with the wad, i guess
			fx = CXMTOF(markpoints[i].x);
			fy = CYMTOF(markpoints[i].y);
			if (fx >= f_x && fx <= f_w - w && fy >= f_y && fy <= f_h - h)
				V_DrawPatch(fx, fy, FB, marknums[i]);
		}
	}

}

void AM_drawCrosshair(int color)
{
	if (scr_bpp == 1)
		fb[(f_w * (f_h + 1)) / 2] = color;	// single point for now
	else
		*((short *)fb + (f_w * (f_h + 1)) / 2) = color;
}

void AM_Drawer(void)
{
	if (!automapactive)
		return;

	if (!automapoverlay)
		AM_clearFB(BACKGROUND);
	if (grid)
		AM_drawGrid(GRIDCOLORS);
	AM_drawWalls();
	AM_drawPlayers();
	if (am_cheating == 2)
		AM_drawThings(THINGCOLORS, THINGRANGE);

	AM_drawCrosshair(XHAIRCOLORS);

	AM_drawMarks();

	// mapname
	{
		int y;
		y = BASEVIDHEIGHT - (ST_HEIGHT) - 1;

		V_DrawStringA(VFONT_SMALL, 0, P_LevelName(),
			20, y - V_StringHeightA(VFONT_SMALL, 0, P_LevelName()));
	}

	V_MarkRect(f_x, f_y, f_w, f_h);

}

