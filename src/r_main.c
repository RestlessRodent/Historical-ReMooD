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
// DESCRIPTION:
//      Rendering main loop and setup functions,
//       utility functions (BSP, geometry, trigonometry).
//      See tables.c, too.

#include "doomdef.h"
#include "g_game.h"
#include "g_input.h"
#include "r_local.h"
#include "r_splats.h"			//faB(21jan):testing
#include "r_sky.h"
#include "st_stuff.h"
#include "p_local.h"
#include "keys.h"
#include "i_video.h"
#include "i_system.h"
#include "m_menu.h"
#include "p_local.h"
#include "t_func.h"
#include "am_map.h"
#include "d_main.h"

// Fineangles in the SCREENWIDTH wide window.
#define FIELDOFVIEW             2048

int viewangleoffset;

// increment every time a check is made
int validcount = 1;

lighttable_t *fixedcolormap;

int centerx;
int centery;
int centerypsp;					//added:06-02-98:cf R_DrawPSprite

fixed_t centerxfrac;
fixed_t centeryfrac;
fixed_t projection;
//added:02-02-98:fixing the aspect ration stuff...
fixed_t projectiony;

// just for profiling purposes
int framecount;

int sscount;
int linecount;
int loopcount;

fixed_t viewx;
fixed_t viewy;
fixed_t viewz;

angle_t viewangle;
angle_t aimingangle;

fixed_t viewcos;
fixed_t viewsin;

player_t *viewplayer;

// 0 = high, 1 = low
int detailshift;

//
// precalculated math tables
//
angle_t clipangle;

// The viewangletox[viewangle + FINEANGLES/4] lookup
// maps the visible view angles to screen X coordinates,
// flattening the arc to a flat projection plane.
// There will be many angles mapped to the same X.
int viewangletox[FINEANGLES / 2];

// The xtoviewangleangle[] table maps a screen pixel
// to the lowest viewangle that maps back to x ranges
// from clipangle to -clipangle.
angle_t* xtoviewangle = NULL;

// UNUSED.
// The finetangentgent[angle+FINEANGLES/4] table
// holds the fixed_t tangent values for view angles,
// ranging from MININT to 0 to MAXINT.
// fixed_t              finetangent[FINEANGLES/2];

// fixed_t              finesine[5*FINEANGLES/4];
fixed_t *finecosine = &finesine[FINEANGLES / 4];

lighttable_t *scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *scalelightfixed[MAXLIGHTSCALE];
lighttable_t *zlight[LIGHTLEVELS][MAXLIGHTZ];

//SoM: 3/30/2000: Hack to support extra boom colormaps.
int num_extra_colormaps;
extracolormap_t extra_colormaps[MAXCOLORMAPS];

// bumped light from gun blasts
int extralight;

consvar_t cv_chasecam = { "chasecam", "0", 0, CV_OnOff };
consvar_t cv_allowmlook = { "allowmlook", "1", CV_NETVAR, CV_YesNo };

consvar_t cv_psprites = { "playersprites", "1", 0, CV_OnOff };
consvar_t cv_perspcorr = { "perspectivecrunch", "0", 0, CV_OnOff };

CV_PossibleValue_t viewsize_cons_t[] = { {3, "MIN"}, {12, "MAX"}, {0, NULL} };
CV_PossibleValue_t detaillevel_cons_t[] = { {0, "High"}, {1, "Low"}, {0, NULL} };

consvar_t cv_viewsize = { "viewsize", "10", CV_SAVE | CV_CALL, viewsize_cons_t, R_SetViewSize };	//3-12
consvar_t cv_detaillevel = { "detaillevel", "0", CV_SAVE | CV_CALL, detaillevel_cons_t, R_SetViewSize };	// UNUSED
consvar_t cv_scalestatusbar = { "scalestatusbar", "0", CV_SAVE | CV_CALL, CV_YesNo, R_SetViewSize };

CV_PossibleValue_t grtranslucenthud_cons_t[] = { {1, "MIN"}, {255, "MAX"}, {0, NULL} };
consvar_t cv_grtranslucenthud = { "gr_translucenthud", "255", CV_SAVE | CV_CALL, grtranslucenthud_cons_t, R_SetViewSize };	//Hurdler: support translucent HUD

consvar_t cv_screenshotdir = { "screenshotdir", "", CV_SAVE, NULL };

// added 16-6-98:splitscreen

void SplitScreen_OnChange(void);

CV_PossibleValue_t splitscreen_cons_t[] = { {0, "MIN"}, {3, "MAX"}, {0, NULL} };
consvar_t cv_splitscreen = { "splitscreen", "0", CV_CALL, splitscreen_cons_t, SplitScreen_OnChange };

void SplitScreen_OnChange(void)
{
	int i, j, k;

	// recompute screen size
	R_ExecuteSetViewSize();

	// change the menu
	//M_SwitchSplitscreen();

	if (demoplayback)
	{
		int i;
		for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
		{
			displayplayer[i] = i;
			consoleplayer[i] = i;
		}
		
		k = 0;
		for (j = 0; j < MAXPLAYERS; j++)
			if (playeringame[j])
				k++;
		
		multiplayer = k > 1;
	}
	else if (gamestate == GS_LEVEL)
	{
		for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
		{
			if (playeringame[i])
			{	// KEEP or REMOVE
				if (i > cv_splitscreen.value)	// remove
				{
					players[consoleplayer[i]].profile = NULL;
					if (players[consoleplayer[i]].mo)
						P_RemoveMobj(players[consoleplayer[i]].mo);
					playeringame[i] = 0;
				}

				if (!cv_splitscreen.value)
					multiplayer = 0;
			}
			else
			{	// ADD
				if (i > 0 && i < cv_splitscreen.value+1)
				{
					j = 0;

					while (playeringame[j] && j <= MAXPLAYERS) j++;
					if (j < MAXPLAYERS)
					{
						consoleplayer[i] = j;
						displayplayer[i] = consoleplayer[i];
						playeringame[consoleplayer[i]] = 1;
						players[consoleplayer[i]].playerstate = PST_REBORN;
					}

					multiplayer = 1;
				}
			}
		}
	}
}

//
// R_PointOnSide
// Traverse BSP (sub) tree,
//  check point against partition plane.
// Returns side 0 (front) or 1 (back).
//
int R_PointOnSide(fixed_t x, fixed_t y, node_t * node)
{
	fixed_t dx;
	fixed_t dy;
	fixed_t left;
	fixed_t right;

	if (!node->dx)
	{
		if (x <= node->x)
			return node->dy > 0;

		return node->dy < 0;
	}
	if (!node->dy)
	{
		if (y <= node->y)
			return node->dx < 0;

		return node->dx > 0;
	}

	dx = (x - node->x);
	dy = (y - node->y);

	// Try to quickly decide by looking at sign bits.
	if ((node->dy ^ node->dx ^ dx ^ dy) & 0x80000000)
	{
		if ((node->dy ^ dx) & 0x80000000)
		{
			// (left is negative)
			return 1;
		}
		return 0;
	}

	left = FixedMul(node->dy >> FRACBITS, dx);
	right = FixedMul(dy, node->dx >> FRACBITS);

	if (right < left)
	{
		// front side
		return 0;
	}
	// back side
	return 1;
}

int R_PointOnSegSide(fixed_t x, fixed_t y, seg_t * line)
{
	fixed_t lx;
	fixed_t ly;
	fixed_t ldx;
	fixed_t ldy;
	fixed_t dx;
	fixed_t dy;
	fixed_t left;
	fixed_t right;

	lx = line->v1->x;
	ly = line->v1->y;

	ldx = line->v2->x - lx;
	ldy = line->v2->y - ly;

	if (!ldx)
	{
		if (x <= lx)
			return ldy > 0;

		return ldy < 0;
	}
	if (!ldy)
	{
		if (y <= ly)
			return ldx < 0;

		return ldx > 0;
	}

	dx = (x - lx);
	dy = (y - ly);

	// Try to quickly decide by looking at sign bits.
	if ((ldy ^ ldx ^ dx ^ dy) & 0x80000000)
	{
		if ((ldy ^ dx) & 0x80000000)
		{
			// (left is negative)
			return 1;
		}
		return 0;
	}

	left = FixedMul(ldy >> FRACBITS, dx);
	right = FixedMul(dy, ldx >> FRACBITS);

	if (right < left)
	{
		// front side
		return 0;
	}
	// back side
	return 1;
}

//
// R_PointToAngle
// To get a global angle from cartesian coordinates,
//  the coordinates are flipped until they are in
//  the first octant of the coordinate system, then
//  the y (<=x) is scaled and divided by x to get a
//  tangent (slope) value which is looked up in the
//  tantoangle[] table.

//
angle_t R_PointToAngle2(fixed_t x2, fixed_t y2, fixed_t x1, fixed_t y1)
{
	x1 -= x2;
	y1 -= y2;

	if ((!x1) && (!y1))
		return 0;

	if (x1 >= 0)
	{
		// x >=0
		if (y1 >= 0)
		{
			// y>= 0

			if (x1 > y1)
			{
				// octant 0
				return tantoangle[SlopeDiv(y1, x1)];
			}
			else
			{
				// octant 1
				return ANG90 - 1 - tantoangle[SlopeDiv(x1, y1)];
			}
		}
		else
		{
			// y<0
			y1 = -y1;

			if (x1 > y1)
			{
				// octant 8
				return -tantoangle[SlopeDiv(y1, x1)];
			}
			else
			{
				// octant 7
				return ANG270 + tantoangle[SlopeDiv(x1, y1)];
			}
		}
	}
	else
	{
		// x<0
		x1 = -x1;

		if (y1 >= 0)
		{
			// y>= 0
			if (x1 > y1)
			{
				// octant 3
				return ANG180 - 1 - tantoangle[SlopeDiv(y1, x1)];
			}
			else
			{
				// octant 2
				return ANG90 + tantoangle[SlopeDiv(x1, y1)];
			}
		}
		else
		{
			// y<0
			y1 = -y1;

			if (x1 > y1)
			{
				// octant 4
				return ANG180 + tantoangle[SlopeDiv(y1, x1)];
			}
			else
			{
				// octant 5
				return ANG270 - 1 - tantoangle[SlopeDiv(x1, y1)];
			}
		}
	}
	return 0;
}

angle_t R_PointToAngle(fixed_t x, fixed_t y)
{
	return R_PointToAngle2(viewx, viewy, x, y);
}

fixed_t R_PointToDist2(fixed_t x2, fixed_t y2, fixed_t x1, fixed_t y1)
{
	int angle;
	fixed_t dx;
	fixed_t dy;
	fixed_t dist;

	dx = abs(x1 - x2);
	dy = abs(y1 - y2);

	if (dy > dx)
	{
		fixed_t temp;

		temp = dx;
		dx = dy;
		dy = temp;
	}
	if (dy == 0)
		return dx;

	angle = (tantoangle[FixedDiv(dy, dx) >> DBITS] + ANG90) >> ANGLETOFINESHIFT;

	// use as cosine
	dist = FixedDiv(dx, finesine[angle]);

	return dist;
}

//SoM: 3/27/2000: Little extra utility. Works in the same way as
//R_PointToAngle2
fixed_t R_PointToDist(fixed_t x, fixed_t y)
{
	return R_PointToDist2(viewx, viewy, x, y);
}

//
// R_InitPointToAngle
//
void R_InitPointToAngle(void)
{
	// UNUSED - now getting from tables.c
#if 0
	int i;
	long t;
	float f;
//
// slope (tangent) to angle lookup
//
	for (i = 0; i <= SLOPERANGE; i++)
	{
		f = atan((float)i / SLOPERANGE) / (3.141592657 * 2);
		t = 0xffffffff * f;
		tantoangle[i] = t;
	}
#endif
}

//
// R_ScaleFromGlobalAngle
// Returns the texture mapping scale
//  for the current line (horizontal span)
//  at the given angle.
// rw_distance must be calculated first.
//
//added:02-02-98:note: THIS IS USED ONLY FOR WALLS!
fixed_t R_ScaleFromGlobalAngle(angle_t visangle)
{
	// UNUSED
#if 0
	//added:02-02-98:note: I've tried this and it displays weird...
	fixed_t scale;
	fixed_t dist;
	fixed_t z;
	fixed_t sinv;
	fixed_t cosv;

	sinv = finesine[(visangle - rw_normalangle) >> ANGLETOFINESHIFT];
	dist = FixedDiv(rw_distance, sinv);
	cosv = finecosine[(viewangle - visangle) >> ANGLETOFINESHIFT];
	z = abs(FixedMul(dist, cosv));
	scale = FixedDiv(projection, z);
	return scale;

#else
	fixed_t scale;
	int anglea;
	int angleb;
	int sinea;
	int sineb;
	fixed_t num;
	int den;

	anglea = ANG90 + (visangle - viewangle);
	angleb = ANG90 + (visangle - rw_normalangle);

	// both sines are allways positive
	sinea = finesine[anglea >> ANGLETOFINESHIFT];
	sineb = finesine[angleb >> ANGLETOFINESHIFT];
	//added:02-02-98:now uses projectiony instead of projection for
	//               correct aspect ratio!
	num = FixedMul(projectiony, sineb) << detailshift;
	den = FixedMul(rw_distance, sinea);

	if (den > num >> 16)
	{
		scale = FixedDiv(num, den);

		if (scale > 64 * FRACUNIT)
			scale = 64 * FRACUNIT;
		else if (scale < 256)
			scale = 256;
	}
	else
		scale = 64 * FRACUNIT;

	return scale;
#endif
}

//
// R_InitTables
//
void R_InitTables(void)
{
	// UNUSED: now getting from tables.c
#if 0
	int i;
	float a;
	float fv;
	int t;

	// viewangle tangent table
	for (i = 0; i < FINEANGLES / 2; i++)
	{
		a = (i - FINEANGLES / 4 + 0.5) * PI * 2 / FINEANGLES;
		fv = FRACUNIT * tan(a);
		t = fv;
		finetangent[i] = t;
	}

	// finesine table
	for (i = 0; i < 5 * FINEANGLES / 4; i++)
	{
		// OPTIMIZE: mirror...
		a = (i + 0.5) * PI * 2 / FINEANGLES;
		t = FRACUNIT * sin(a);
		finesine[i] = t;
	}
#endif

}

// consvar_t cv_fov = {"fov","2048", CV_CALL | CV_NOINIT, NULL, R_ExecuteSetViewSize};

//
// R_InitTextureMapping
//
void R_InitTextureMapping(void)
{
	int i;
	int x;
	int t;
	fixed_t focallength;

	// Use tangent table to generate viewangletox:
	//  viewangletox will give the next greatest x
	//  after the view angle.
	//
	// Calc focallength
	//  so FIELDOFVIEW angles covers SCREENWIDTH.
	focallength = FixedDiv(centerxfrac, finetangent[FINEANGLES / 4 +
													/*cv_fov.value */
													FIELDOFVIEW / 2]);

	for (i = 0; i < FINEANGLES / 2; i++)
	{
		if (finetangent[i] > FRACUNIT * 2)
			t = -1;
		else if (finetangent[i] < -FRACUNIT * 2)
			t = viewwidth + 1;
		else
		{
			t = FixedMul(finetangent[i], focallength);
			t = (centerxfrac - t + FRACUNIT - 1) >> FRACBITS;

			if (t < -1)
				t = -1;
			else if (t > viewwidth + 1)
				t = viewwidth + 1;
		}
		viewangletox[i] = t;
	}

	// Scan viewangletox[] to generate xtoviewangle[]:
	//  xtoviewangle will give the smallest view angle
	//  that maps to x.
	for (x = 0; x <= viewwidth; x++)
	{
		i = 0;
		while (viewangletox[i] > x)
			i++;
		xtoviewangle[x] = (i << ANGLETOFINESHIFT) - ANG90;
	}

	// Take out the fencepost cases from viewangletox.
	for (i = 0; i < FINEANGLES / 2; i++)
	{
		t = FixedMul(finetangent[i], focallength);
		t = centerx - t;

		if (viewangletox[i] == -1)
			viewangletox[i] = 0;
		else if (viewangletox[i] == viewwidth + 1)
			viewangletox[i] = viewwidth;
	}

	clipangle = xtoviewangle[0];
}

//
// R_InitLightTables
// Only inits the zlight table,
//  because the scalelight table changes with view size.
//
#define DISTMAP         2

void R_InitLightTables(void)
{
	int i;
	int j;
	int level;
	int startmap;
	int scale;

	// Calculate the light levels to use
	//  for each level / distance combination.
	for (i = 0; i < LIGHTLEVELS; i++)
	{
		startmap = ((LIGHTLEVELS - 1 - i) * 2) * NUMCOLORMAPS / LIGHTLEVELS;
		for (j = 0; j < MAXLIGHTZ; j++)
		{
			//added:02-02-98:use BASEVIDWIDTH, vid.width is not set already,
			// and it seems it needs to be calculated only once.
			scale = FixedDiv((BASEVIDWIDTH / 2 * FRACUNIT), (j + 1) << LIGHTZSHIFT);
			scale >>= LIGHTSCALESHIFT;
			level = startmap - scale / DISTMAP;

			if (level < 0)
				level = 0;

			if (level >= NUMCOLORMAPS)
				level = NUMCOLORMAPS - 1;

			zlight[i][j] = colormaps + level * 256;
		}
	}
}

//
// R_SetViewSize
// Do not really change anything here,
//  because it might be in the middle of a refresh.
// The change will take effect next refresh.
//
bool_t setsizeneeded;

void R_SetViewSize(void)
{
	setsizeneeded = true;
}

//
// R_ExecuteSetViewSize
//

// now uses screen variables cv_viewsize, cv_detaillevel
//
void R_ExecuteSetViewSize(void)
{
	fixed_t cosadj;
	fixed_t dy;
	int i;
	int j;
	int level;
	int startmap;

	int setdetail;

	int aspectx;				//added:02-02-98:for aspect ratio calc. below...
	
	// GhostlyDeath <April 25, 2008> -- Fix crash when using server subsystem for the client
	if (!graphics_started)
		return;

	setsizeneeded = false;
	// no reduced view in splitscreen mode
	if (cv_splitscreen.value && cv_viewsize.value < 11)
		CV_SetValue(&cv_viewsize, 11);

	setdetail = cv_detaillevel.value;

	// status bar overlay at viewsize 11
	st_overlay = (cv_viewsize.value == 11);

	// clamp detail level (actually ignore it, keep it for later who knows)
	if (setdetail)
	{
		setdetail = 0;
		CONS_Printf("lower detail mode n.a.\n");
		CV_SetValue(&cv_detaillevel, setdetail);
	}

	stbarheight = ST_HEIGHT;

	if (!cv_splitscreen.value && cv_scalestatusbar.value || cv_viewsize.value >= 11)
		stbarheight *= vid.fdupy;

	
	//added 01-01-98: full screen view, without statusbar
	if (cv_splitscreen.value || cv_viewsize.value > 10 || TRANSPARENTSTATUSBAR)
	{
		scaledviewwidth = vid.width;
		viewheight = vid.height;
	}
	else
	{
		//added 01-01-98: always a multiple of eight
		scaledviewwidth = (cv_viewsize.value * vid.width / 10) & ~7;
		//added:05-02-98: make viewheight multiple of 2 because sometimes
		//                a line is not refreshed by R_DrawViewBorder()
		viewheight = (cv_viewsize.value * (vid.height - stbarheight) / 10) & ~1;
	}

	// added 16-6-98:splitscreen
	if (cv_splitscreen.value)
		viewheight >>= 1;
	if (cv_splitscreen.value > 1)
		scaledviewwidth >>= 1;

	detailshift = setdetail;
	viewwidth = scaledviewwidth >> detailshift;

	centery = viewheight / 2;
	centerx = viewwidth / 2;
	centerxfrac = centerx << FRACBITS;
	centeryfrac = centery << FRACBITS;

	//added:01-02-98:aspect ratio is now correct, added an 'projectiony'
	//      since the scale is not always the same between horiz. & vert.
	projection = centerxfrac;
	projectiony = (((vid.height * centerx * BASEVIDWIDTH) / BASEVIDHEIGHT) / vid.width) << FRACBITS;

	//
	// no more low detail mode, it used to setup the right drawer routines
	// for either detail mode here
	//
	// if (!detailshift) ... else ...

	R_InitViewBuffer(scaledviewwidth, viewheight);

	R_InitTextureMapping();

	// psprite scales
	centerypsp = viewheight / 2;	//added:06-02-98:psprite pos for freelook

	pspritescale = (viewwidth << FRACBITS) / BASEVIDWIDTH;
	pspriteiscale = (BASEVIDWIDTH << FRACBITS) / viewwidth;	// x axis scale
	//added:02-02-98:now aspect ratio correct for psprites
	pspriteyscale = (((vid.height * viewwidth) / vid.width) << FRACBITS) / BASEVIDHEIGHT;

	// thing clipping
	for (i = 0; i < viewwidth; i++)
		screenheightarray[i] = viewheight;

	// setup sky scaling for old/new skies (uses pspriteyscale)
	R_SetSkyScale();

	// planes
	//added:02-02-98:now correct aspect ratio!
	aspectx = (((vid.height * centerx * BASEVIDWIDTH) / BASEVIDHEIGHT) / vid.width);

	// this is only used for planes rendering in software mode
	j = viewheight * 4;
	for (i = 0; i < j; i++)
	{
		//added:10-02-98:(i-centery) became (i-centery*2) and centery*2=viewheight
		dy = ((i - viewheight * 2) << FRACBITS) + FRACUNIT / 2;
		dy = abs(dy);
		yslopetab[i] = FixedDiv(aspectx * FRACUNIT, dy);
	}

	for (i = 0; i < viewwidth; i++)
	{
		cosadj = abs(finecosine[xtoviewangle[i] >> ANGLETOFINESHIFT]);
		distscale[i] = FixedDiv(FRACUNIT, cosadj);
	}

	// Calculate the light levels to use
	//  for each level / scale combination.
	for (i = 0; i < LIGHTLEVELS; i++)
	{
		startmap = ((LIGHTLEVELS - 1 - i) * 2) * NUMCOLORMAPS / LIGHTLEVELS;
		for (j = 0; j < MAXLIGHTSCALE; j++)
		{
			level = startmap - j * vid.width / (viewwidth << detailshift) / DISTMAP;

			if (level < 0)
				level = 0;

			if (level >= NUMCOLORMAPS)
				level = NUMCOLORMAPS - 1;

			scalelight[i][j] = colormaps + level * 256;
		}
	}

	st_recalc = true;
	am_recalc = true;
}

//
// R_Init
//

void R_Init(void)
{
	if (dedicated)
		return;

	//added:24-01-98: screensize independent
	if (devparm)
		CONS_Printf("\nR_InitData");
	R_InitData();

	if (devparm)
		CONS_Printf("\nR_InitPointToAngle");
	R_InitPointToAngle();

	if (devparm)
		CONS_Printf("\nR_InitTables");
	R_InitTables();

	R_InitViewBorder();

	R_SetViewSize();			// setsizeneeded is set true

	if (devparm)
		CONS_Printf("\nR_InitPlanes");
	R_InitPlanes();

	//added:02-02-98: this is now done by SCR_Recalc() at the first mode set
	if (devparm)
		CONS_Printf("\nR_InitLightTables");
	R_InitLightTables();

	if (devparm)
		CONS_Printf("\nR_InitSkyMap");
	R_InitSkyMap();

	if (devparm)
		CONS_Printf("\nR_InitTranslationsTables");
	R_InitTranslationTables();

	R_InitDrawNodes();

	framecount = 0;
}

//
// R_PointInSubsector
//
subsector_t *R_PointInSubsector(fixed_t x, fixed_t y)
{
	node_t *node;
	int side;
	int nodenum;

	// single subsector is a special case
	if (!numnodes)
		return subsectors;

	nodenum = numnodes - 1;

	while (!(nodenum & NF_SUBSECTOR))
	{
		node = &nodes[nodenum];
		side = R_PointOnSide(x, y, node);
		nodenum = node->children[side];
	}

	return &subsectors[nodenum & ~NF_SUBSECTOR];
}

//
// R_IsPointInSubsector, same of above but return 0 if not in subsector
//
subsector_t *R_IsPointInSubsector(fixed_t x, fixed_t y)
{
	node_t *node;
	int side;
	int nodenum, i;
	subsector_t *ret;

	// single subsector is a special case
	if (!numnodes)
		return subsectors;

	nodenum = numnodes - 1;

	while (!(nodenum & NF_SUBSECTOR))
	{
		node = &nodes[nodenum];
		side = R_PointOnSide(x, y, node);
		nodenum = node->children[side];
	}

	ret = &subsectors[nodenum & ~NF_SUBSECTOR];
	for (i = 0; i < ret->numlines; i++)
	{
		if (R_PointOnSegSide(x, y, &segs[ret->firstline + i]))
			return 0;
	}

	return ret;
}

//
// R_SetupFrame
//

mobj_t *viewmobj;

void P_ResetCamera(player_t * player);

// WARNING : a should be unsigned but to add with 2048, it isn't !
#define AIMINGTODY(a) ((finetangent[(2048+(((int)a)>>ANGLETOFINESHIFT)) & FINEMASK]*160)>>FRACBITS)

void R_SetupFrame(player_t * player)
{
	int i;
	int fixedcolormap_setup;
	int dy = 0;					//added:10-02-98:

	extralight = player->extralight;

	//
	if (cv_chasecam.value && !camera.chase)
	{
		P_ResetCamera(player);
		camera.chase = true;
	}
	else if (!cv_chasecam.value)
		camera.chase = false;

#ifdef FRAGGLESCRIPT
	if (script_camera_on)
	{
		viewmobj = script_camera.mo;
#ifdef PARANOIA
		if (!viewmobj)
			I_Error("no mobj for the camera");
#endif
		viewz = viewmobj->z;
		fixedcolormap_setup = camera.fixedcolormap;
		aimingangle = script_camera.aiming;
		viewangle = viewmobj->angle;
	}
	else
#endif
	if (camera.chase)
		// use outside cam view
	{
		viewmobj = camera.mo;
#ifdef PARANOIA
		if (!viewmobj)
			I_Error("no mobj for the camera");
#endif
		viewz = viewmobj->z + (viewmobj->height >> 1);
		fixedcolormap_setup = camera.fixedcolormap;
		aimingangle = camera.aiming;
		viewangle = viewmobj->angle;
	}
	else
		// use the player's eyes view
	{
		viewz = player->viewz;
		viewmobj = player->mo;
		fixedcolormap_setup = player->fixedcolormap;
		aimingangle = player->aiming;
		viewangle = viewmobj->angle + viewangleoffset;

		if (!demoplayback && player->playerstate != PST_DEAD)
		{
			for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
				if (playeringame[consoleplayer[i]] && player == &players[consoleplayer[i]])
				{
					viewangle = localangle[i];
					aimingangle = localaiming[i];
				}
		}

	}

#ifdef PARANOIA
	if (!viewmobj)
		I_Error("R_Setupframe : viewmobj null (player %d)", player - players);
#endif
	viewplayer = player;
	viewx = viewmobj->x;
	viewy = viewmobj->y;

	viewsin = finesine[viewangle >> ANGLETOFINESHIFT];
	viewcos = finecosine[viewangle >> ANGLETOFINESHIFT];

	sscount = 0;

	if (fixedcolormap_setup)
	{
		fixedcolormap = colormaps + fixedcolormap_setup * 256 * sizeof(lighttable_t);

		walllights = scalelightfixed;

		for (i = 0; i < MAXLIGHTSCALE; i++)
			scalelightfixed[i] = fixedcolormap;
	}
	else
		fixedcolormap = 0;

	//added:06-02-98:recalc necessary stuff for mouseaiming
	//               slopes are already calculated for the full
	//               possible view (which is 4*viewheight).
	// clip it in the case we are looking a hardware 90 full aiming
	// (lmps, nework and use F12...)
	G_ClipAimingPitch(&aimingangle);
	
	if (cv_splitscreen.value != 1)
		dy = AIMINGTODY(aimingangle) * viewheight / BASEVIDHEIGHT;
	else
		dy = AIMINGTODY(aimingangle) * viewheight * 2 / BASEVIDHEIGHT;

	yslope = &yslopetab[(3 * viewheight / 2) - dy];
	centery = (viewheight / 2) + dy;
	centeryfrac = centery << FRACBITS;

	framecount++;
	validcount++;
}

// ================
// R_RenderView
// ================

//                     FAB NOTE FOR WIN32 PORT !! I'm not finished already,
// but I suspect network may have problems with the video buffer being locked
// for all duration of rendering, and being released only once at the end..
// I mean, there is a win16lock() or something that lasts all the rendering,
// so maybe we should release screen lock before each netupdate below..?

void R_DrawPlayerSprites(void);

void R_RenderPlayerViewEx(player_t * player, int quarter)
{
	register uint8_t* dest;
	int x, y, a, b, c, d;
	
	R_SetupFrame(player);

	// Clear buffers.
	R_ClearClipSegs();
	R_ClearDrawSegs();
	R_ClearPlanes(player);		//needs player for waterheight in occupied sector
	//R_ClearPortals ();
	R_ClearSprites();

	// check for new console commands.
	NetUpdate();

	// The head node is the last node output.

	R_RenderBSPNode(numnodes - 1);

	// Check for new console commands.
	NetUpdate();

	//R_DrawPortals ();
	R_DrawPlanes();

	// Check for new console commands.
	NetUpdate();

	// draw mid texture and sprite
	// SoM: And now 3D floors/sides!
	R_DrawMasked();

	// draw the psprites on top of everything
	//  but does not draw on side views
	if (!viewangleoffset && !camera.chase && cv_psprites.value && !script_camera_on)
		R_DrawPlayerSprites();
	
	// GhostlyDeath -- warp the view
	// ylookup is the row and columnofs is the column in the row
#if 0
	if (((!cv_chasecam.value && player->mo && player->mo->eflags & MF_UNDERWATER) ||
		(cv_chasecam.value && camera.mo && camera.mo->eflags & MF_UNDERWATER)) && !cv_splitscreen.value)
	{		
		for (y = 0; y < viewheight; y++)
		{
			a = (gametic % 32) - 16;
			
			if (a < 0)
				c = 0;
			else
				c = 1;
			
			if (c)
			{
				a += 2;
				if (a > 16)
					c = 0;
			}
			else
			{
				a -= 2;
				if (a < -16)
					c = 1;
			}
			
			for (x = 0; x < viewwidth; x++)
			{
				//a = ((gametic) % (viewwidth / 16)) - (viewheight / 32);// + (x % ((viewwidth / 8)) / 2);
				b = a;
				if (b < 0)
					b += (gametic+(x>>2)) % 16;
				else
					b -= (gametic+(x>>2)) % 16;
				if (b < -16)
					b = -16;
				else if (b > 16)
					b = 16;
				
				while (b + y < 0)
					b++;
				while (b + y > viewheight - (viewheight / 32))
					b--;
				
				if (ylookup[y + b])
				{
					if (b < 0)
						*(ylookup[y + b] + columnofs[x]) = *(ylookup[y] + columnofs[x]);
					else
						*(ylookup[y] + columnofs[x]) = *(ylookup[y + b] + columnofs[x]);
				}
			}
		}
	}
#endif

	// Check for new console commands.
	NetUpdate();
	player->mo->flags &= ~MF_NOSECTOR;	// don't show self (uninit) clientprediction code
}

void R_RenderPlayerView(player_t * player)
{
	R_RenderPlayerViewEx(player, 0);
}

// GhostlyDeath <July 6, 2008> -- Draws a quarter of the screen
void R_RenderPlayerQuarter(player_t * player)
{
	R_RenderPlayerViewEx(player, 1);
}

// =========================================================================
//                    ENGINE COMMANDS & VARS
// =========================================================================

void R_RegisterEngineStuff(void)
{
	//26-07-98
	CV_RegisterVar(&cv_gravity);

	CV_RegisterVar(&cv_chasecam);
	CV_RegisterVar(&cv_allowmlook);
	CV_RegisterVar(&cv_cam_dist);
	CV_RegisterVar(&cv_cam_height);
	CV_RegisterVar(&cv_cam_speed);

	CV_RegisterVar(&cv_viewsize);
	CV_RegisterVar(&cv_psprites);
	CV_RegisterVar(&cv_splitscreen);
//    CV_RegisterVar (&cv_fov);

	// Default viewheight is changeable,
	// initialized to standard viewheight
	CV_RegisterVar(&cv_viewheight);
	CV_RegisterVar(&cv_scalestatusbar);
	CV_RegisterVar(&cv_transparentstatusbar);
	CV_RegisterVar(&cv_transparentstatusbarmode);
	CV_RegisterVar(&cv_grtranslucenthud);

	CV_RegisterVar(&cv_screenshotdir);
}

