// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
//         :oCCCCOCoc.
//     .cCO8OOOOOOOOO8Oo:
//   .oOO8OOOOOOOOOOOOOOOCc
//  cO8888:         .:oOOOOC.                                                TM
// :888888:   :CCCc   .oOOOOC.     ###      ###                    #########
// C888888:   .ooo:   .C########   #####  #####  ######    ######  ##########
// O888888:         .oO###    ###  #####  ##### ########  ######## ####    ###
// C888888:   :8O.   .C##########  ### #### ### ##    ##  ##    ## ####    ###
// :8@@@@8:   :888c   o###         ### #### ### ########  ######## ##########
//  :8@@@@C   C@@@@   oo########   ###  ##  ###  ######    ######  #########
//    cO@@@@@@@@@@@@@@@@@Oc0
//      :oO8@@@@@@@@@@Oo.
//         .oCOOOOOCc.                                      http://remood.org/
// ----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
// ----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// ----------------------------------------------------------------------------
// DESCRIPTION: the automap code

/* Includes */
#include "doomtype.h"
#include "m_fixed.h"
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
#include "p_demcmp.h"

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
#define INITSCALEMTOF (13107)
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
#define FTOM(x) FixedMul(((x)<<16),scale_ftom[am_DrawPlayer])
#define MTOF(x) (FixedMul((x),scale_mtof[am_DrawPlayer])>>16)
// translates between frame-buffer and map coordinates
#define CXMTOF(x)  (f_x[am_DrawPlayer] + MTOF((x)-m_x[am_DrawPlayer]))
#define CYMTOF(y)  (f_y[am_DrawPlayer] + (f_h[am_DrawPlayer] - MTOF((y)-m_y[am_DrawPlayer])))

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
mline_t player_arrow[] =
{
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
mline_t cheat_player_arrow[] =
{
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
mline_t triangle_guy[] =
{
	{	{(fixed_t) - .867 * R, (fixed_t) - .5 * R},
		{(fixed_t) .867 * R, (fixed_t) - .5 * R}
	},
	{{(fixed_t) .867 * R, (fixed_t) - .5 * R}, {(fixed_t) 0, (fixed_t) R}},
	{{(fixed_t) 0, (fixed_t) R}, {(fixed_t) - .867 * R, (fixed_t) - .5 * R}}
};

#undef R
#define NUMTRIANGLEGUYLINES (sizeof(triangle_guy)/sizeof(mline_t))

#define R (FRACUNIT)
mline_t thintriangle_guy[] =
{
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

static int am_DrawPlayer = 0;

static int leveljuststarted = 1;	// kluge until AM_LevelInit() is called

bool_t automapactive = false;
bool_t automapoverlay = false;
bool_t am_recalc = false;		//added:05-02-98:true when screen size

//               changes


// location of window on screen
static int f_x[MAXSPLITSCREEN];
static int f_y[MAXSPLITSCREEN];

// size of window on screen
static int f_w[MAXSPLITSCREEN];
static int f_h[MAXSPLITSCREEN];

static int lightlev[MAXSPLITSCREEN];			// used for funky strobing effect
static uint8_t* fb;				// pseudo-frame buffer
static int amclock[MAXSPLITSCREEN];

static mpoint_t m_paninc[MAXSPLITSCREEN];		// how far the window pans each tic (map coords)
static fixed_t mtof_zoommul[MAXSPLITSCREEN];	// how far the window zooms in each tic (map coords)
static fixed_t ftom_zoommul[MAXSPLITSCREEN];	// how far the window zooms in each tic (fb coords)

static fixed_t m_x[MAXSPLITSCREEN], m_y[MAXSPLITSCREEN];		// LL x,y where the window is on the map (map coords)
static fixed_t m_xb[MAXSPLITSCREEN], m_yb[MAXSPLITSCREEN];		// UR x,y where the window is on the map (map coords)

//
// width/height of window on map (map coords)
//
static fixed_t m_w[MAXSPLITSCREEN];
static fixed_t m_h[MAXSPLITSCREEN];

// based on level size
static fixed_t min_x[MAXSPLITSCREEN];
static fixed_t min_y[MAXSPLITSCREEN];
static fixed_t max_x[MAXSPLITSCREEN];
static fixed_t max_y[MAXSPLITSCREEN];

static fixed_t max_w[MAXSPLITSCREEN];			// max_x[MAXSPLITSCREEN]-min_x[MAXSPLITSCREEN],
static fixed_t max_h[MAXSPLITSCREEN];			// max_y[MAXSPLITSCREEN]-min_y[MAXSPLITSCREEN]

// based on player size
static fixed_t min_w[MAXSPLITSCREEN];
static fixed_t min_h[MAXSPLITSCREEN];

static fixed_t min_scale_mtof[MAXSPLITSCREEN][MAXSPLITSCREEN];	// used to tell when to stop zooming out
static fixed_t max_scale_mtof[MAXSPLITSCREEN][MAXSPLITSCREEN];	// used to tell when to stop zooming in

// old stuff for recovery later
static fixed_t old_m_w[MAXSPLITSCREEN], old_m_h[MAXSPLITSCREEN];
static fixed_t old_m_x[MAXSPLITSCREEN], old_m_y[MAXSPLITSCREEN];

// old location used by the Follower routine
static mpoint_t f_oldloc[MAXSPLITSCREEN];

// used by MTOF to scale from map-to-frame-buffer coords
static fixed_t scale_mtof[MAXSPLITSCREEN] = {INITSCALEMTOF, INITSCALEMTOF, INITSCALEMTOF, INITSCALEMTOF};

// used by FTOM to scale from frame-buffer-to-map coords (=1/scale_mtof[MAXSPLITSCREEN])
static fixed_t scale_ftom[MAXSPLITSCREEN];

static player_t* plr[MAXSPLITSCREEN];			// the player represented by an arrow

static patch_t* marknums[MAXSPLITSCREEN][10];	// numbers used for marking by the automap
static mpoint_t markpoints[MAXSPLITSCREEN][AM_NUMMARKPOINTS];	// where the points are
static int markpointnum[MAXSPLITSCREEN] = {0, 0, 0, 0};	// next point to be assigned

static int followplayer[MAXSPLITSCREEN] = {1, 1, 1, 1};	// specifies whether to follow the player around


static bool_t stopped = true;

static uint8_t BLUEKEYCOLOR;
static uint8_t YELLOWKEYCOLOR;
static uint8_t REDKEYCOLOR;

// function for drawing lines, depends on rendermode
typedef void (*AMDRAWFLINEFUNC) (fline_t* fl, int color);
static AMDRAWFLINEFUNC AM_drawFline;

void AM_drawFline_soft(fline_t* fl, int color);

// Calculates the slope and slope according to the x-axis of a line
// segment in map coordinates (with the upright y-axis n' all) so
// that it can be used with the brain-dead drawing stuff.

void AM_getIslope(mline_t* ml, islope_t* is)
{
	int dx, dy;
	
	dy = ml->a.y - ml->b.y;
	dx = ml->b.x - ml->a.x;
	if (!dy)
		is->islp = (dx < 0 ? -INT_MAX : INT_MAX);
	else
		is->islp = FixedDiv(dx, dy);
	if (!dx)
		is->slp = (dy < 0 ? -INT_MAX : INT_MAX);
	else
		is->slp = FixedDiv(dy, dx);
		
}

//
void AM_activateNewScale(void)
{
	m_x[am_DrawPlayer] += m_w[am_DrawPlayer] / 2;
	m_y[am_DrawPlayer] += m_h[am_DrawPlayer] / 2;
	m_w[am_DrawPlayer] = FTOM(f_w[am_DrawPlayer]);
	m_h[am_DrawPlayer] = FTOM(f_h[am_DrawPlayer]);
	m_x[am_DrawPlayer] -= m_w[am_DrawPlayer] / 2;
	m_y[am_DrawPlayer] -= m_h[am_DrawPlayer] / 2;
	m_xb[am_DrawPlayer] = m_x[am_DrawPlayer] + m_w[am_DrawPlayer];
	m_yb[am_DrawPlayer] = m_y[am_DrawPlayer] + m_h[am_DrawPlayer];
}

//
void AM_saveScaleAndLoc(void)
{
	old_m_x[am_DrawPlayer] = m_x[am_DrawPlayer];
	old_m_y[am_DrawPlayer] = m_y[am_DrawPlayer];
	old_m_w[am_DrawPlayer] = m_w[am_DrawPlayer];
	old_m_h[am_DrawPlayer] = m_h[am_DrawPlayer];
}

//
void AM_restoreScaleAndLoc(void)
{

	m_w[am_DrawPlayer] = old_m_w[am_DrawPlayer];
	m_h[am_DrawPlayer] = old_m_h[am_DrawPlayer];
	if (!followplayer[am_DrawPlayer])
	{
		m_x[am_DrawPlayer] = old_m_x[am_DrawPlayer];
		m_y[am_DrawPlayer] = old_m_y[am_DrawPlayer];
	}
	else
	{
		m_x[am_DrawPlayer] = plr[am_DrawPlayer]->mo->x - m_w[am_DrawPlayer] / 2;
		m_y[am_DrawPlayer] = plr[am_DrawPlayer]->mo->y - m_h[am_DrawPlayer] / 2;
	}
	m_xb[am_DrawPlayer] = m_x[am_DrawPlayer] + m_w[am_DrawPlayer];
	m_yb[am_DrawPlayer] = m_y[am_DrawPlayer] + m_h[am_DrawPlayer];
	
	// Change the scaling multipliers
	scale_mtof[am_DrawPlayer] = FixedDiv(f_w[am_DrawPlayer] << FRACBITS, m_w[am_DrawPlayer]);
	scale_ftom[am_DrawPlayer] = FixedDiv(FRACUNIT, scale_mtof[am_DrawPlayer]);
}

//
// adds a marker at the current location
//
void AM_addMark(void)
{
	markpoints[am_DrawPlayer][markpointnum[am_DrawPlayer]].x = m_x[am_DrawPlayer] + m_w[am_DrawPlayer] / 2;
	markpoints[am_DrawPlayer][markpointnum[am_DrawPlayer]].y = m_y[am_DrawPlayer] + m_h[am_DrawPlayer] / 2;
	markpointnum[am_DrawPlayer] = (markpointnum[am_DrawPlayer] + 1) % AM_NUMMARKPOINTS;
	
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
	
	min_x[am_DrawPlayer] = min_y[am_DrawPlayer] = INT_MAX;
	max_x[am_DrawPlayer] = max_y[am_DrawPlayer] = -INT_MAX;
	
	for (i = 0; i < numvertexes; i++)
	{
		if (vertexes[i].x < min_x[am_DrawPlayer])
			min_x[am_DrawPlayer] = vertexes[i].x;
		else if (vertexes[i].x > max_x[am_DrawPlayer])
			max_x[am_DrawPlayer] = vertexes[i].x;
			
		if (vertexes[i].y < min_y[am_DrawPlayer])
			min_y[am_DrawPlayer] = vertexes[i].y;
		else if (vertexes[i].y > max_y[am_DrawPlayer])
			max_y[am_DrawPlayer] = vertexes[i].y;
	}
	
	max_w[am_DrawPlayer] = max_x[am_DrawPlayer] - min_x[am_DrawPlayer];
	max_h[am_DrawPlayer] = max_y[am_DrawPlayer] - min_y[am_DrawPlayer];
	
	min_w[am_DrawPlayer] = 2 * PLAYERRADIUS;	// const? never changed?
	min_h[am_DrawPlayer] = 2 * PLAYERRADIUS;
	
	a = FixedDiv(f_w[am_DrawPlayer] << FRACBITS, max_w[am_DrawPlayer]);
	b = FixedDiv(f_h[am_DrawPlayer] << FRACBITS, max_h[am_DrawPlayer]);
	
	min_scale_mtof[am_DrawPlayer][am_DrawPlayer] = a < b ? a : b;
	max_scale_mtof[am_DrawPlayer][am_DrawPlayer] = FixedDiv(f_h[am_DrawPlayer] << FRACBITS, 2 * PLAYERRADIUS);
	
}

//
void AM_changeWindowLoc(void)
{
	if (m_paninc[am_DrawPlayer].x || m_paninc[am_DrawPlayer].y)
	{
		followplayer[am_DrawPlayer] = 0;
		f_oldloc[am_DrawPlayer].x = INT_MAX;
	}
	
	m_x[am_DrawPlayer] += m_paninc[am_DrawPlayer].x;
	m_y[am_DrawPlayer] += m_paninc[am_DrawPlayer].y;
	
	if (m_x[am_DrawPlayer] + m_w[am_DrawPlayer] / 2 > max_x[am_DrawPlayer])
		m_x[am_DrawPlayer] = max_x[am_DrawPlayer] - m_w[am_DrawPlayer] / 2;
	else if (m_x[am_DrawPlayer] + m_w[am_DrawPlayer] / 2 < min_x[am_DrawPlayer])
		m_x[am_DrawPlayer] = min_x[am_DrawPlayer] - m_w[am_DrawPlayer] / 2;
		
	if (m_y[am_DrawPlayer] + m_h[am_DrawPlayer] / 2 > max_y[am_DrawPlayer])
		m_y[am_DrawPlayer] = max_y[am_DrawPlayer] - m_h[am_DrawPlayer] / 2;
	else if (m_y[am_DrawPlayer] + m_h[am_DrawPlayer] / 2 < min_y[am_DrawPlayer])
		m_y[am_DrawPlayer] = min_y[am_DrawPlayer] - m_h[am_DrawPlayer] / 2;
		
	m_xb[am_DrawPlayer] = m_x[am_DrawPlayer] + m_w[am_DrawPlayer];
	m_yb[am_DrawPlayer] = m_y[am_DrawPlayer] + m_h[am_DrawPlayer];
}

//
static void AM_initVariables(void)
{
	int pnum;
	static event_t st_notify = { ev_keyup, AM_MSGENTERED };
	size_t p;
	
	for (p = 0; p < MAXSPLITSCREEN; p++)
	{
		am_DrawPlayer = p;
		
		automapactive = true;
		automapoverlay = false;
		fb = screens[0];
	
		f_oldloc[am_DrawPlayer].x = INT_MAX;
		amclock[am_DrawPlayer] = 0;
		lightlev[am_DrawPlayer] = 0;
	
		m_paninc[am_DrawPlayer].x = m_paninc[am_DrawPlayer].y = 0;
		ftom_zoommul[am_DrawPlayer] = FRACUNIT;
		mtof_zoommul[am_DrawPlayer] = FRACUNIT;
	
		m_w[am_DrawPlayer] = FTOM(f_w[am_DrawPlayer]);
		m_h[am_DrawPlayer] = FTOM(f_h[am_DrawPlayer]);
	
		// find player to center on initially
		if (!playeringame[pnum = g_Splits[p].Console])
			for (pnum = 0; pnum < MAXPLAYERS; pnum++)
				if (playeringame[pnum])
					break;
				
		plr[am_DrawPlayer] = &players[pnum];
	
		if (plr[am_DrawPlayer]->mo)
		{
			m_x[am_DrawPlayer] = plr[am_DrawPlayer]->mo->x - m_w[am_DrawPlayer] / 2;
			m_y[am_DrawPlayer] = plr[am_DrawPlayer]->mo->y - m_h[am_DrawPlayer] / 2;
		}
		else
		{
			m_x[am_DrawPlayer] = m_w[am_DrawPlayer] / 2;
			m_y[am_DrawPlayer] = m_h[am_DrawPlayer] / 2;
		}
		AM_changeWindowLoc();
	
		// for saving & restoring
		old_m_x[am_DrawPlayer] = m_x[am_DrawPlayer];
		old_m_y[am_DrawPlayer] = m_y[am_DrawPlayer];
		old_m_w[am_DrawPlayer] = m_w[am_DrawPlayer];
		old_m_h[am_DrawPlayer] = m_h[am_DrawPlayer];
	
	
		BLUEKEYCOLOR = 200;
		YELLOWKEYCOLOR = 231;
		REDKEYCOLOR = 176;
	}
	
	// inform the status bar of the change
	ST_Responder(&st_notify);
}

static uint8_t* maplump;		// pointer to the raw data for the automap background.

//
static void AM_loadPics(void)
{
#if 0
	int i;
	char namebuf[9];
	
	for (i = 0; i < 10; i++)
	{
		sprintf(namebuf, "AMMNUM%d", i);
		marknums[am_DrawPlayer][i] = W_CachePatchName(namebuf, PU_STATIC);
	}
	if (W_CheckNumForName("AUTOPAGE") >= 0)
		maplump = W_CacheLumpName("AUTOPAGE", PU_STATIC);
	else
		maplump = NULL;
#endif
}

static void AM_unloadPics(void)
{
	int i;
	
	for (i = 0; i < 10; i++)
		Z_ChangeTag(marknums[am_DrawPlayer][i], PU_CACHE);
	if (maplump)
		Z_ChangeTag(maplump, PU_CACHE);
}

void AM_clearMarks(void)
{
	int i;
	
	for (i = 0; i < AM_NUMMARKPOINTS; i++)
		markpoints[am_DrawPlayer][i].x = -1;	// means empty
	markpointnum[am_DrawPlayer] = 0;
}

//
// should be called at the start of every level
// right now, i figure it out myself
//
void AM_LevelInit(void)
{
	size_t p;
	
	for (p = 0; p < MAXSPLITSCREEN; p++)
	{
		// Set current draw player
		am_DrawPlayer = p;
		
		// Normal initialize
		leveljuststarted = 0;
		
		// Normal 1 player view
		if (g_SplitScreen <= 0)
		{
			f_x[am_DrawPlayer] = f_y[am_DrawPlayer] = 0;
			f_w[am_DrawPlayer] = vid.width;
			f_h[am_DrawPlayer] = vid.height;//- stbarheight;
		}
		
		// 2-player Horizontal split
		else if (g_SplitScreen == 1)
		{
			f_x[am_DrawPlayer] = 0;
			f_y[am_DrawPlayer] = ((vid.height / 2) * (am_DrawPlayer & 1));
			f_w[am_DrawPlayer] = vid.width;
			f_h[am_DrawPlayer] = vid.height / 2;//- stbarheight;
		}
		
		// 3/4 player view
		else
		{
			f_x[am_DrawPlayer] = ((vid.width / 2) * (am_DrawPlayer & 1));
			f_y[am_DrawPlayer] = ((vid.height / 2) * ((am_DrawPlayer >> 1) & 1));
			f_w[am_DrawPlayer] = vid.width / 2;
			f_h[am_DrawPlayer] = vid.height / 2;//- stbarheight;
		}
	
		AM_drawFline = AM_drawFline_soft;
	
		AM_clearMarks();
	
		AM_findMinMaxBoundaries();
		scale_mtof[am_DrawPlayer] = FixedDiv(min_scale_mtof[am_DrawPlayer][am_DrawPlayer], (int)(0.7 * FRACUNIT));
		if (scale_mtof[am_DrawPlayer] > max_scale_mtof[am_DrawPlayer][am_DrawPlayer])
			scale_mtof[am_DrawPlayer] = min_scale_mtof[am_DrawPlayer][am_DrawPlayer];
		scale_ftom[am_DrawPlayer] = FixedDiv(FRACUNIT, scale_mtof[am_DrawPlayer]);
	}
}

//
void AM_Stop(void)
{
	static event_t st_notify = { 0, ev_keyup, AM_MSGEXITED };
	
	AM_unloadPics();
	automapactive = false;
	//automapoverlay = false;
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
	scale_mtof[am_DrawPlayer] = min_scale_mtof[am_DrawPlayer][am_DrawPlayer];
	scale_ftom[am_DrawPlayer] = FixedDiv(FRACUNIT, scale_mtof[am_DrawPlayer]);
	AM_activateNewScale();
}

//
// set the window scale to the minimum size
//
void AM_maxOutWindowScale(void)
{
	scale_mtof[am_DrawPlayer] = max_scale_mtof[am_DrawPlayer][am_DrawPlayer];
	scale_ftom[am_DrawPlayer] = FixedDiv(FRACUNIT, scale_mtof[am_DrawPlayer]);
	AM_activateNewScale();
}

//
// Handle events (user inputs) in automap mode
//
bool_t AM_Responder(event_t* ev)
{

	int rc;
	static int cheatstate = 0;
	static char buffer[20];
	
	am_DrawPlayer = 0;
	
	rc = false;
	
	if (!automapactive)
	{
		if (ev->type == ev_keydown && ev->data1 == AM_STARTKEY)
		{

		}
	}
	
	else if (ev->type == ev_keydown)
	{
	
		rc = true;
		switch (ev->data1)
		{
			case AM_PANRIGHTKEY:	// pan right
				if (!followplayer[am_DrawPlayer])
					m_paninc[am_DrawPlayer].x = FTOM(F_PANINC);
				else
					rc = false;
				break;
			case AM_PANLEFTKEY:	// pan left
				if (!followplayer[am_DrawPlayer])
					m_paninc[am_DrawPlayer].x = -FTOM(F_PANINC);
				else
					rc = false;
				break;
			case AM_PANUPKEY:	// pan up
				if (!followplayer[am_DrawPlayer])
					m_paninc[am_DrawPlayer].y = FTOM(F_PANINC);
				else
					rc = false;
				break;
			case AM_PANDOWNKEY:	// pan down
				if (!followplayer[am_DrawPlayer])
					m_paninc[am_DrawPlayer].y = -FTOM(F_PANINC);
				else
					rc = false;
				break;
			case AM_ZOOMOUTKEY:	// zoom out
				mtof_zoommul[am_DrawPlayer] = M_ZOOMOUT;
				ftom_zoommul[am_DrawPlayer] = M_ZOOMIN;
				break;
			case AM_ZOOMINKEY:	// zoom in
				mtof_zoommul[am_DrawPlayer] = M_ZOOMIN;
				ftom_zoommul[am_DrawPlayer] = M_ZOOMOUT;
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
				followplayer[am_DrawPlayer] = !followplayer[am_DrawPlayer];
				f_oldloc[am_DrawPlayer].x = INT_MAX;
				//plr[am_DrawPlayer]->message = followplayer[am_DrawPlayer] ? AMSTR_FOLLOWON : AMSTR_FOLLOWOFF;
				break;
			case AM_GRIDKEY:
				grid = !grid;
				//plr[am_DrawPlayer]->message = grid ? AMSTR_GRIDON : AMSTR_GRIDOFF;
				break;
			case AM_MARKKEY:
				//sprintf(buffer, "%s %d", AMSTR_MARKEDSPOT, markpointnum[am_DrawPlayer]);
				//plr[am_DrawPlayer]->message = buffer;
				AM_addMark();
				break;
			case AM_CLEARMARKKEY:
				AM_clearMarks();
				//plr[am_DrawPlayer]->message = AMSTR_MARKSCLEARED;
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
				if (!followplayer[am_DrawPlayer])
					m_paninc[am_DrawPlayer].x = 0;
				break;
			case AM_PANLEFTKEY:
				if (!followplayer[am_DrawPlayer])
					m_paninc[am_DrawPlayer].x = 0;
				break;
			case AM_PANUPKEY:
				if (!followplayer[am_DrawPlayer])
					m_paninc[am_DrawPlayer].y = 0;
				break;
			case AM_PANDOWNKEY:
				if (!followplayer[am_DrawPlayer])
					m_paninc[am_DrawPlayer].y = 0;
				break;
			case AM_ZOOMOUTKEY:
			case AM_ZOOMINKEY:
				mtof_zoommul[am_DrawPlayer] = FRACUNIT;
				ftom_zoommul[am_DrawPlayer] = FRACUNIT;
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
	scale_mtof[am_DrawPlayer] = FixedMul(scale_mtof[am_DrawPlayer], mtof_zoommul[am_DrawPlayer]);
	scale_ftom[am_DrawPlayer] = FixedDiv(FRACUNIT, scale_mtof[am_DrawPlayer]);
	
	if (scale_mtof[am_DrawPlayer] < min_scale_mtof[am_DrawPlayer][am_DrawPlayer])
		AM_minOutWindowScale();
	else if (scale_mtof[am_DrawPlayer] > max_scale_mtof[am_DrawPlayer][am_DrawPlayer])
		AM_maxOutWindowScale();
	else
		AM_activateNewScale();
}

//
void AM_doFollowPlayer(void)
{

	if (plr[am_DrawPlayer]->mo && (f_oldloc[am_DrawPlayer].x != plr[am_DrawPlayer]->mo->x || f_oldloc[am_DrawPlayer].y != plr[am_DrawPlayer]->mo->y))
	{
		m_x[am_DrawPlayer] = FTOM(MTOF(plr[am_DrawPlayer]->mo->x)) - m_w[am_DrawPlayer] / 2;
		m_y[am_DrawPlayer] = FTOM(MTOF(plr[am_DrawPlayer]->mo->y)) - m_h[am_DrawPlayer] / 2;
		m_xb[am_DrawPlayer] = m_x[am_DrawPlayer] + m_w[am_DrawPlayer];
		m_yb[am_DrawPlayer] = m_y[am_DrawPlayer] + m_h[am_DrawPlayer];
		f_oldloc[am_DrawPlayer].x = plr[am_DrawPlayer]->mo->x;
		f_oldloc[am_DrawPlayer].y = plr[am_DrawPlayer]->mo->y;
		
		//  m_x[am_DrawPlayer] = FTOM(MTOF(plr[am_DrawPlayer]->mo->x - m_w[am_DrawPlayer]/2));
		//  m_y[am_DrawPlayer] = FTOM(MTOF(plr[am_DrawPlayer]->mo->y - m_h[am_DrawPlayer]/2));
		//  m_x[am_DrawPlayer] = plr[am_DrawPlayer]->mo->x - m_w[am_DrawPlayer]/2;
		//  m_y[am_DrawPlayer] = plr[am_DrawPlayer]->mo->y - m_h[am_DrawPlayer]/2;
		
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
	if (amclock[am_DrawPlayer] > nexttic)
	{
		lightlev[am_DrawPlayer] = litelevels[litelevelscnt++];
		if (litelevelscnt == sizeof(litelevels) / sizeof(int))
			litelevelscnt = 0;
		nexttic = amclock[am_DrawPlayer] + 6 - (amclock[am_DrawPlayer] % 6);
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
		
	amclock[am_DrawPlayer]++;
	
	if (followplayer[am_DrawPlayer])
		AM_doFollowPlayer();
		
	// Change the zoom if necessary
	if (ftom_zoommul[am_DrawPlayer] != FRACUNIT)
		AM_changeWindowScale();
		
	// Change x,y location
	if (m_paninc[am_DrawPlayer].x || m_paninc[am_DrawPlayer].y)
		AM_changeWindowLoc();
		
	// Update light level
	// AM_updateLightLev();
	
}

//
// Clear automap frame buffer.
//
void AM_clearFB(int color)
{
	size_t y;
	
	if (!maplump)
	{
		for (y = 0; y < f_h[am_DrawPlayer]; y++)
			memset(fb + ((vid.width * (y + f_y[am_DrawPlayer])) + f_x[am_DrawPlayer]), color, f_w[am_DrawPlayer]);
	}
	else
	{
		int i, y;
		int dmapx;
		int dmapy;
		static int mapxstart;
		static int mapystart;
		uint8_t* dest = screens[0], *src;
		
#define MAPLUMPHEIGHT (200-SBARHEIGHT)
		
		if (followplayer[am_DrawPlayer] && plr[am_DrawPlayer]->mo)
		{
			static vertex_t oldplr;
			
			dmapx = (MTOF(plr[am_DrawPlayer]->mo->x) - MTOF(oldplr.x));	//fixed point
			dmapy = (MTOF(oldplr.y) - MTOF(plr[am_DrawPlayer]->mo->y));
			
			oldplr.x = plr[am_DrawPlayer]->mo->x;
			oldplr.y = plr[am_DrawPlayer]->mo->y;
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
			mapxstart += (MTOF(m_paninc[am_DrawPlayer].x) >> 1);
			mapystart -= (MTOF(m_paninc[am_DrawPlayer].y) >> 1);
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
			for (y = 0; y < f_h[am_DrawPlayer]; y++)
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
bool_t AM_clipMline(mline_t* ml, fline_t* fl)
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
    else if ((my) >= f_h[am_DrawPlayer]) (oc) |= BOTTOM; \
    if ((mx) < 0) (oc) |= LEFT; \
    else if ((mx) >= f_w[am_DrawPlayer]) (oc) |= RIGHT;
	
	// do trivial rejects and outcodes
	if (ml->a.y > m_yb[am_DrawPlayer])
		outcode1 = TOP;
	else if (ml->a.y < m_y[am_DrawPlayer])
		outcode1 = BOTTOM;
		
	if (ml->b.y > m_yb[am_DrawPlayer])
		outcode2 = TOP;
	else if (ml->b.y < m_y[am_DrawPlayer])
		outcode2 = BOTTOM;
		
	if (outcode1 & outcode2)
		return false;			// trivially outside
		
	if (ml->a.x < m_x[am_DrawPlayer])
		outcode1 |= LEFT;
	else if (ml->a.x > m_xb[am_DrawPlayer])
		outcode1 |= RIGHT;
		
	if (ml->b.x < m_x[am_DrawPlayer])
		outcode2 |= LEFT;
	else if (ml->b.x > m_xb[am_DrawPlayer])
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
			tmp.x = fl->a.x + (dx * (fl->a.y - f_h[am_DrawPlayer])) / dy;
			tmp.y = f_h[am_DrawPlayer] - 1;
		}
		else if (outside & RIGHT)
		{
			dy = fl->b.y - fl->a.y;
			dx = fl->b.x - fl->a.x;
			tmp.y = fl->a.y + (dy * (f_w[am_DrawPlayer] - 1 - fl->a.x)) / dx;
			tmp.x = f_w[am_DrawPlayer] - 1;
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
void AM_drawFline_soft(fline_t* fl, int color)
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
	if (fl->a.x < 0 || fl->a.x >= f_w[am_DrawPlayer] || fl->a.y < 0 || fl->a.y >= f_h[am_DrawPlayer] || fl->b.x < 0 || fl->b.x >= f_w[am_DrawPlayer] || fl->b.y < 0 || fl->b.y >= f_h[am_DrawPlayer])
	{
		CONL_PrintF("line clipping problem %d \r", fuck++);
		return;
	}
#endif
	
#define PUTDOT(xx,yy,cc) fb[(f_y[am_DrawPlayer] + (yy) * vid.width) + (f_x[am_DrawPlayer] + (xx))] = (cc)
//#define PUTDOT(xx,yy,cc) fb[((yy) * vid.width) + ((xx))] = (cc)
	
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
void AM_drawMline(mline_t* ml, int color)
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
	start = m_x[am_DrawPlayer];
	if ((start - bmaporgx) % (MAPBLOCKUNITS << FRACBITS))
		start += (MAPBLOCKUNITS << FRACBITS) - ((start - bmaporgx) % (MAPBLOCKUNITS << FRACBITS));
	end = m_x[am_DrawPlayer] + m_w[am_DrawPlayer];
	
	// draw vertical gridlines
	ml.a.y = m_y[am_DrawPlayer];
	ml.b.y = m_y[am_DrawPlayer] + m_h[am_DrawPlayer];
	for (x = start; x < end; x += (MAPBLOCKUNITS << FRACBITS))
	{
		ml.a.x = x;
		ml.b.x = x;
		AM_drawMline(&ml, color);
	}
	
	// Figure out start of horizontal gridlines
	start = m_y[am_DrawPlayer];
	if ((start - bmaporgy) % (MAPBLOCKUNITS << FRACBITS))
		start += (MAPBLOCKUNITS << FRACBITS) - ((start - bmaporgy) % (MAPBLOCKUNITS << FRACBITS));
	end = m_y[am_DrawPlayer] + m_h[am_DrawPlayer];
	
	// draw horizontal gridlines
	ml.a.x = m_x[am_DrawPlayer];
	ml.b.x = m_x[am_DrawPlayer] + m_w[am_DrawPlayer];
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
				AM_drawMline(&l, WALLCOLORS + lightlev[am_DrawPlayer]);
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
								AM_drawMline(&l, SECRETWALLCOLORS + lightlev[am_DrawPlayer]);
							else
								AM_drawMline(&l, WALLCOLORS + lightlev[am_DrawPlayer]);
						}
						else if (lines[i].backsector->floorheight != lines[i].frontsector->floorheight)
						{
							AM_drawMline(&l, FDWALLCOLORS + lightlev[am_DrawPlayer]);	// floor level change
						}
						else if (lines[i].backsector->ceilingheight != lines[i].frontsector->ceilingheight)
						{
							AM_drawMline(&l, CDWALLCOLORS + lightlev[am_DrawPlayer]);	// ceiling level change
						}
						else if (am_cheating)
						{
							AM_drawMline(&l, TSWALLCOLORS + lightlev[am_DrawPlayer]);
						}
				}
			}
		}
		else if (plr[am_DrawPlayer]->powers[pw_allmap])
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
void AM_rotate(fixed_t* x, fixed_t* y, angle_t a)
{
	fixed_t tmpx;
	
	tmpx = FixedMul(*x, finecosine[a >> ANGLETOFINESHIFT]) - FixedMul(*y, finesine[a >> ANGLETOFINESHIFT]);
	
	*y = FixedMul(*x, finesine[a >> ANGLETOFINESHIFT]) + FixedMul(*y, finecosine[a >> ANGLETOFINESHIFT]);
	
	*x = tmpx;
}

void AM_drawLineCharacter(mline_t* lineguy, int lineguylines, fixed_t scale, angle_t angle, int color, fixed_t x, fixed_t y)
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
	player_t* p;
	int color;
	
	if (!multiplayer)
	{
		if (plr[am_DrawPlayer]->mo)
		{
			if (am_cheating)
				AM_drawLineCharacter(cheat_player_arrow, NUMCHEATPLYRLINES, 0, plr[am_DrawPlayer]->mo->angle, DWHITE, plr[am_DrawPlayer]->mo->x, plr[am_DrawPlayer]->mo->y);
			else
				AM_drawLineCharacter(player_arrow, NUMPLYRLINES, 0, plr[am_DrawPlayer]->mo->angle, DWHITE, plr[am_DrawPlayer]->mo->x, plr[am_DrawPlayer]->mo->y);
		}
		return;
	}
	
	// multiplayer
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i])
			continue;
			
		p = &players[i];
		
		if ((P_XGSVal(PGS_GAMEDEATHMATCH) && !singledemo) && p != plr[am_DrawPlayer])
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
		
		AM_drawLineCharacter(player_arrow, NUMPLYRLINES, 0, p->mo->angle, color, p->mo->x, p->mo->y);
	}
	
}

void A_Look();

void AM_drawThings(int colors, int colorrange)
{
	int i;
	int color = colors;
	angle_t tangle;
	mobj_t* t;
	
	for (i = 0; i < numsectors; i++)
	{
		t = sectors[i].thinglist;
		
		while (t)
		{
			// Modify Angle
			if (t->RXFlags[0] & MFREXA_ISTELEFOG)
				tangle = t->angle + (ANG45 * (gametic % 8));
			else
				tangle = t->angle;
			
			// Modify Color
			if (t->health <= 0 || t->flags & MF_CORPSE)
				color = GRAYS + (GRAYSRANGE >> 1);
			else if (t->info->flags & MF_COUNTKILL || (t->RXFlags[0] & MFREXA_ISMONSTER))
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
				if (t->RXFlags[0] & MFREXA_ISPOWERUP)
					color = YELLOWS + ((gametic >> 1) % YELLOWRANGE);
				else
					color = YELLOWS + 7;
			}
			else if (t->player)
				color = *(translationtables + ((t->player->skincolor - 1) << 8) + GREENS + 8);
			else
				color = colors + lightlev[am_DrawPlayer];
				
			AM_drawLineCharacter
			(thintriangle_guy, NUMTHINTRIANGLEGUYLINES, (t->info->radius > 0 ? t->info->radius / FRACUNIT : 2) << FRACBITS, tangle, color, t->x, t->y);
			t = t->snext;
		}
	}
}

void AM_drawMarks(void)
{
	int i, fx, fy, w, h;
	
	for (i = 0; i < AM_NUMMARKPOINTS; i++)
	{
		if (markpoints[am_DrawPlayer][i].x != -1)
		{
			//      w = LittleSwapInt16(marknums[am_DrawPlayer][i]->width);
			//      h = LittleSwapInt16(marknums[am_DrawPlayer][i]->height);
			w = 5;				// because something's wrong with the wad, i guess
			h = 6;				// because something's wrong with the wad, i guess
			fx = CXMTOF(markpoints[am_DrawPlayer][i].x);
			fy = CYMTOF(markpoints[am_DrawPlayer][i].y);
			if (fx >= f_x[am_DrawPlayer] && fx <= f_w[am_DrawPlayer] - w && fy >= f_y[am_DrawPlayer] && fy <= f_h[am_DrawPlayer] - h)
				V_DrawPatch(fx, fy, FB, marknums[am_DrawPlayer][i]);
		}
	}
	
}

void AM_drawCrosshair(int color)
{
	if (scr_bpp == 1)
		fb[(f_w[am_DrawPlayer] * (f_h[am_DrawPlayer] + 1)) / 2] = color;	// single point for now
	else
		*((short*)fb + (f_w[am_DrawPlayer] * (f_h[am_DrawPlayer] + 1)) / 2) = color;
}

void AM_Drawer(void)
{
	int p;
	
	if (!automapactive)
		return;
	
	for (p = 0; p < g_SplitScreen + 1; p++)
	{	
		am_DrawPlayer = p;
		
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
		
			V_DrawStringA(VFONT_SMALL, 0, P_LevelNameEx(), 20, y - V_StringHeightA(VFONT_SMALL, 0, P_LevelNameEx()));
		}
	}
}

