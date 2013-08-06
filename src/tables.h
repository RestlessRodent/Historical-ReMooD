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
// DESCRIPTION: Lookup tables.
//              Do not try to look them up :-).
//              In the order of appearance:
//              int finetangent[4096]   - Tangens LUT.
//              Should work with BAM fairly well (12 of 16bit,
//              effectively, by shifting).
//              int finesine[10240]             - Sine lookup.
//              int tantoangle[2049]    - ArcTan LUT,
//              maps tan(angle) to angle fast. Gotta search.

#ifndef __TABLES__
#define __TABLES__

//#include "m_fixed.h"

#define FINEANGLES              8192
#define FINEMASK                (FINEANGLES-1)
#define ANGLETOFINESHIFT        19	// 0x100000000 to 0x2000

// Effective size is 10240.
extern fixed_t finesine[5 * FINEANGLES / 4];

// Re-use data, is just PI/2 phase shift.
extern fixed_t* finecosine;

// Effective size is 4096.
extern fixed_t finetangent[FINEANGLES / 2];

// to get a global angle from cartesian coordinates, the coordinates are
// flipped until they are in the first octant of the coordinate system, then
// the y (<=x) is scaled and divided by x to get a tangent (slope) value
// which is looked up in the tantoangle[] table.
#define SLOPERANGE  2048
#define SLOPEBITS   11
#define DBITS       (FRACBITS-SLOPEBITS)

// The +1 size is to handle the case when x==y without additional checking.
extern angle_t tantoangle[SLOPERANGE + 1];

// Utility function, called by R_PointToAngle.
angle_t SlopeDiv(angle_t num, angle_t den);

fixed_t TBL_BAMToDeg(const angle_t a_Angle);

extern const int16_t c_AngLUT[8192];

#endif

