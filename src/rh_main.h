// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
// Copyright (C) 1996-1998 Activision and Raven Software
// Copyright (C) 2012-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Heretic Renderer

#ifndef __RH_MAIN_H__
#define __RH_MAIN_H__

/***************
*** INCLUDES ***
***************/

#include "doomdef.h"
#include "d_player.h"
#include "m_fixed.h"
#include "r_defs.h"

/****************
*** CONSTANTS ***
****************/

#define MAXRHVISPLANES					128		// Visplane limit

/*****************
*** STRUCTURES ***
*****************/

/* RH_VisPlane_t -- Heretic VisPlane */
typedef struct RH_VisPlane_s
{
	fixed_t Height;								// Height of plane
	int32_t PicNum;								// Texture to draw
	int32_t LightLevel;							// Light level
	uint32_t Special;							// Sector Special
	int32_t MinX, MaxX;							// Min/Max X Positions
	int16_t* Top;								// Top Offsets
	int16_t* Bottom;							// Bottom Offsets
	
	struct RH_VisPlane_s* Prev;					// Previous Plane
	struct RH_VisPlane_s* Next;					// Next Plane
} RH_VisPlane_t;

/* RH_RenderStat_t -- Heretic Renderer Status */
typedef struct RH_RenderStat_s
{
	/* Normal Ported Stuff */
	player_t* viewplayer;						// View player
	fixed_t viewx, viewy, viewz;				// View Positions
	int viewangleoffset;						// Viewing angle offset
	angle_t viewangle;							// Viewing Angle
	fixed_t viewcos, viewsin;					// Math for angle
	int extralight;								// Extra Lighting
	int32_t viewwindowx, viewwindowy;			// Window Positions
	int32_t viewwidth, viewheight;				// Window Size
	int32_t scaledviewwidth, scaledviewheight;	// Scaled views
	int32_t centery, centerx;					// Center positions
	fixed_t centeryfrac, centerxfrac;			// Center positions as fixed
	fixed_t projection;							// Projection position
	int32_t* columnofs;							// Column offsets
	uint8_t** ylookup;							// Y Lookup Table
	fixed_t* yslopetab;							// Y Slope
	fixed_t* yslope;							// Y Slope
	int32_t aspectx;							// X Aspect Ratio
	fixed_t aspectfx;							// Ratio as fixed
	sector_t* frontsector;						// Front Sector
	sector_t* backsector;						// Back Sector
	RH_VisPlane_t* floorplane;					// Floor plane
	RH_VisPlane_t* ceilingplane;				// Ceiling plane
	int32_t* spanstart;							// Start of spans
	int32_t* spanstop;							// End of spans
	fixed_t* cachedheight;						// Cache
	fixed_t* cacheddistance;					// Cache
	fixed_t* cachedxstep;						// Cache
	fixed_t* cachedystep;						// Cache
	fixed_t* distscale;							// Distance Scale
	fixed_t basexscale;							// Scale
	fixed_t baseyscale;							// Scale
	angle_t* xtoviewangle;						// X to view angle
	int* viewangletox;							// View angle to x
	angle_t clipangle;							// Clipping angle
	int rw_angle1;								// Angle
	
	/* Render Structs */
	RH_VisPlane_t* FirstPlaneLink;				// Link to first plane
	RH_VisPlane_t** VisPlanes[MAXRHVISPLANES];	// Visplanes available
	size_t NumVisPlanes[MAXRHVISPLANES];		// Number of visplanes
	
	/* ReMooD Stuff */
	void* HLocal;								// Heretic Local View
} RH_RenderStat_t;

/****************
*** FUNCTIONS ***
****************/

/* rh_main.c */
void R_ExecuteSetViewSize_HERETIC(void);
void R_RenderPlayerView_HERETIC(player_t* player, const size_t a_Screen);

void RH_SetupFrame(RH_RenderStat_t* const a_Stat);

/* rh_bsp.c */
void RH_RenderBSPNode(RH_RenderStat_t* const a_Stat, const int a_NodeNum);

/* rh_plane.c */
void RH_RenderPlanes(RH_RenderStat_t* const a_Stat);
RH_VisPlane_t* RH_FindPlane(RH_RenderStat_t* const a_Stat, RH_VisPlane_t* const a_Input);
bool_t RH_ComparePlane(RH_RenderStat_t* const a_Stat, RH_VisPlane_t* const a_A, RH_VisPlane_t* const a_B);

/* rh_thing.c */
void RH_AddSprites(RH_RenderStat_t* const a_Stat, sector_t* const a_Sector);

/* rh_draw.c */
void RH_DrawSpan(RH_RenderStat_t* const a_Stat, RH_VisPlane_t* const a_VisPlane, int ds_y, int ds_x1, int ds_x2, lighttable_t *ds_colormap, fixed_t ds_xfrac, fixed_t ds_yfrac, fixed_t ds_xstep, fixed_t ds_ystep, uint8_t *ds_source);

#endif /* __RH_MAIN_H__ */

