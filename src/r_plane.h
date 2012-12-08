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
// Copyright (C) 2008-2013 GhostlyDeath <ghostlydeath@remood.org>
//                                      <ghostlydeath@gmail.com>
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
// DESCRIPTION: Refresh, visplane stuff (floor, ceilings).

#ifndef __R_PLANE__
#define __R_PLANE__

#include "screen.h"
#include "r_data.h"

//
// Now what is a visplane, anyway?
// Simple : kinda floor/ceiling polygon optimised for Doom rendering.
// 4124 bytes!
//
typedef struct visplane_s
{
	struct visplane_s* next;	//SoM: 3/17/2000
	
	fixed_t height;
	fixed_t viewz;
	angle_t viewangle;
	int picnum;
	int lightlevel;
	int minx;
	int maxx;
	sector_t* Sector;							// Sector to draw
	int32_t SkyTexture;							// Sky Texture
	fixed_t SkyTextureMid;						// Sky Texture Mid
	
	//SoM: 4/3/2000: Colormaps per sector!
	extracolormap_t* extra_colormap;
	
	// leave pads for [minx-1]/[maxx+1]
	
	//faB: words sucks .. should get rid of that.. but eats memory
	//added:08-02-98: THIS IS UNSIGNED! VERY IMPORTANT!!
	uint16_t* top;
	uint16_t* bottom;
	
	int high, low;				// SoM: R_PlaneBounds should set these.
	
	fixed_t xoffs, yoffs;		// SoM: 3/6/2000: Srolling flats.
	
	// SoM: frontscale should be stored in the first seg of the subsector
	// where the planes themselves are stored. I'm doing this now because
	// the old way caused trouble with the drawseg array was re-sized.
	int scaleseg;
	
	struct ffloor_s* ffloor;
} visplane_t;

#define           MAXVISPLANES      128

extern visplane_t** visplane_ptr;
extern visplane_t* floorplane;
extern visplane_t* ceilingplane;

// Visplane related.
extern short* lastopening;

typedef void (*planefunction_t) (int top, int bottom);

extern planefunction_t floorfunc;
extern planefunction_t ceilingfunc_t;

extern short* floorclip;
extern short* ceilingclip;
extern short* waterclip;		//added:18-02-98:WATER!
extern fixed_t* frontscale;
extern fixed_t* yslopetab;

extern fixed_t* yslope;
extern fixed_t* distscale;

void R_InitPlanes(void);
void R_ClearPlanes(player_t* player);

void R_MapPlane(int y, int x1, int x2);

void R_MakeSpans(int x, int t1, int b1, int t2, int b2);

void R_DrawPlanes(void);

visplane_t* R_FindPlane(fixed_t height, int picnum, int lightlevel, fixed_t xoff, fixed_t yoff, extracolormap_t* planecolormap, ffloor_t* ffloor, sector_t* const a_Sector);

visplane_t* R_CheckPlane(visplane_t* pl, int start, int stop);

void R_ExpandPlane(visplane_t* pl, int start, int stop);

// SoM: Draws a single visplane. If !handlesource, it won't allocate or
// remove ds_source.
void R_DrawSinglePlane(visplane_t* pl, bool_t handlesource);
void R_PlaneBounds(visplane_t* plane);

typedef struct planemgr_s
{
	visplane_t* plane;
	fixed_t height;
	bool_t mark;
	fixed_t f_pos;				// `F' for `Front sector'.
	fixed_t b_pos;				// `B' for `Back sector'
	fixed_t f_frac;
	fixed_t f_step;
	fixed_t b_frac;
	fixed_t b_step;
	short* f_clip;
	short* c_clip;
	
	struct ffloor_s* ffloor;
} planemgr_t;

extern planemgr_t ffloor[MAXFFLOORS];
extern int numffloors;
extern int* spanstart;

#endif
