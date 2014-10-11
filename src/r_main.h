// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: System specific interface stuff.

#ifndef __R_MAIN__
#define __R_MAIN__

#include "r_data.h"

//
// POV related.
//
extern fixed_t viewcos;
extern fixed_t viewsin;

extern int viewwidth;
extern int viewheight;
extern int viewwindowx;
extern int viewwindowy;

extern int centerx;
extern int centery;

extern int centerypsp;

extern fixed_t centerxfrac;
extern fixed_t centeryfrac;
extern fixed_t projection;
extern fixed_t projectiony;		//added:02-02-98:aspect ratio test...

extern int validcount;

extern int linecount;
extern int loopcount;

extern int framecount;

//
// Lighting LUT.
// Used for z-depth cuing per column/row,
//  and other lighting effects (sector ambient, flash).
//

// Lighting constants.
// Now why not 32 levels here?
#define LIGHTLEVELS             16
#define LIGHTSEGSHIFT            4

#define MAXLIGHTSCALE           48
#define LIGHTSCALESHIFT         12
#define MAXLIGHTZ              128
#define LIGHTZSHIFT             20

extern lighttable_t* scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t* scalelightfixed[MAXLIGHTSCALE];
extern lighttable_t* zlight[LIGHTLEVELS][MAXLIGHTZ];

extern int extralight;
extern lighttable_t* fixedcolormap;

// Number of diminishing brightness levels.
// There a 0-31, i.e. 32 LUT in the COLORMAP lump.
#define NUMCOLORMAPS            32

// Blocky/low detail mode.
//B remove this?
//  0 = high, 1 = low
extern int detailshift;

//
// Utility functions.
int R_PointOnSide(fixed_t x, fixed_t y, node_t* node);

int R_PointOnSegSide(fixed_t x, fixed_t y, seg_t* line);

angle_t R_PointToAngle(fixed_t x, fixed_t y);

angle_t R_PointToAngle2(fixed_t x2, fixed_t y2, fixed_t x1, fixed_t y1);

fixed_t R_PointToDist(fixed_t x, fixed_t y);

//SoM: 3/27/2000
fixed_t R_PointToDist2(fixed_t x2, fixed_t y2, fixed_t x1, fixed_t y1);

fixed_t R_ScaleFromGlobalAngle(angle_t visangle);

subsector_t* R_PointInSubsector(fixed_t x, fixed_t y);

subsector_t* R_IsPointInSubsector(fixed_t x, fixed_t y);

void R_AddPointToBox(int x, int y, fixed_t* box);

//
// REFRESH - the actual rendering functions.
//

// Called by startup code.
void R_Init(void);

// just sets setsizeneeded true
extern bool_t setsizeneeded;
void R_SetViewSize(void);

// add commands related to engine, at game startup
void R_RegisterEngineStuff(void);

/*** DOOM RENDERER ***/
void R_ExecuteSetViewSize_DOOM(void);
void R_RenderPlayerView_DOOM(player_t* player, const size_t a_Screen);

/*** GENERIC CALLS ***/
extern void (*R_ExecuteSetViewSize)(void);
extern void (*R_RenderPlayerView)(player_t* player, const size_t a_Screen);

#endif

