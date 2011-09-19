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
// DESCRIPTION: Refresh module, BSP traversal and handling.

#ifndef __R_BSP__
#define __R_BSP__

extern seg_t* curline;
extern side_t* sidedef;
extern line_t* linedef;
extern sector_t* frontsector;
extern sector_t* backsector;

extern bool_t skymap;

// faB: drawsegs are now allocated on the fly ... see r_segs.c
// extern drawseg_t*       drawsegs;
//SoM: 3/26/2000: Use boom code.
extern drawseg_t* drawsegs;
extern size_t maxdrawsegs;
extern drawseg_t* ds_p;
extern drawseg_t* firstnewseg;

extern lighttable_t** hscalelight;
extern lighttable_t** vscalelight;
extern lighttable_t** dscalelight;

typedef void (*drawfunc_t) (int start, int stop);

// BSP?
void R_ClearClipSegs(void);
void R_SetupClipSegs();
void R_ClearDrawSegs(void);

void R_RenderBSPNode(int bspnum);

sector_t* R_FakeFlat(sector_t* sec, sector_t* tempsec, int* floorlightlevel, int* ceilinglightlevel, bool_t back);

int R_GetPlaneLight(sector_t* sector, fixed_t planeheight, bool_t underside);
void R_Prep3DFloors(sector_t* sector);
#endif
