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
// DESCRIPTION: Doom Status Bar

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
//#include "st_stuff.h"
//#include "d_prof.h"
//#include "v_video.h"

/*************
*** LOCALS ***
*************/

static bool_t l_BarInit;
static V_Image_t* l_ImgBackFace;

/****************
*** FUNCTIONS ***
****************/

/* ST_InitDoomBar() -- Initializes the status bar */
void ST_InitDoomBar(void)
{
	l_ImgBackFace = V_ImageFindA("STBAR", VCP_NONE);
}

#define IDFLAGS (VEX_NOSCALESTART)

/* ST_DoomModShape() -- Modifies shape of view bar */
void ST_DoomModShape(const size_t a_PID, int32_t* const a_X, int32_t* const a_Y, int32_t* const a_W, int32_t* const a_H, player_t* const a_ConsoleP, player_t* const a_DisplayP, D_Prof_t* a_Profile)
{
	fixed_t gsy;
	int32_t sbh;
	int32_t y, h;
	
	/* Initilialize Bar */
	if (!l_BarInit)
	{
		ST_InitDoomBar();
		l_BarInit = true;
	}
	
	/* Initialize sizing specification */
	// Screen real position
	y = a_Y;
	h = a_H;
	
	// Status bar is scaled to screen
	if (true)
		gsy = vid.fxdupy;
	
	// Status bar is not scaled to screen
	else
		gsy = 1 << FRACBITS;
	
	// Top of bar
	*a_H -= FixedMul(l_ImgBackFace->Height << FRACBITS, gsy) >> FRACBITS;
}

/* ST_DoomBar() -- Doom Status Bar */
void ST_DoomBar(const size_t a_PID, const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H, player_t* const a_ConsoleP, player_t* const a_DisplayP, D_Prof_t* a_Profile)
{
	fixed_t sx, sy, gsx, gsy;
	int32_t sbx, sbh, sbtop;
	int32_t x, y, w, h;
	
	/* Initilialize Bar */
	if (!l_BarInit)
	{
		ST_InitDoomBar();
		l_BarInit = true;
	}
	
	/* Initialize sizing specification */
	// Scale on normal screen (split screen scale)
	sx = FixedDiv(a_W << FRACBITS, 320 << FRACBITS);
	sy = FixedDiv(a_H << FRACBITS, 200 << FRACBITS);
	
	// Screen real position
	x = FixedMul(a_X << FRACBITS, vid.fxdupx) >> FRACBITS;
	y = FixedMul(a_Y << FRACBITS, vid.fxdupy) >> FRACBITS;
	w = FixedMul(a_W << FRACBITS, vid.fxdupx) >> FRACBITS;
	h = FixedMul(a_H << FRACBITS, vid.fxdupy) >> FRACBITS;
	
	// Status bar is scaled to screen
	if (true)
	{
		gsx = FixedMul(sx, vid.fxdupx);
		gsy = FixedMul(sy, vid.fxdupy);
		
		// Coordinates of status bar (when scaled)
		sbx = x;
	}
	
	// Status bar is not scaled to screen
	else
	{
		gsx = sx;
		gsy = sy;
		
		// Center bar on screen
		sbx = 0;
	}
	
	// Top of bar
	sbh = FixedMul(l_ImgBackFace->Height << FRACBITS, gsy) >> FRACBITS;
	sbtop = y + (h - sbh);
	
	/* Draw backface */
	V_ImageDrawScaled(IDFLAGS, l_ImgBackFace, sbx, sbtop, gsx, gsy, NULL);
	
	//void V_ImageDrawScaled(const uint32_t a_Flags, V_Image_t* const a_Image, const int32_t a_X, const int32_t a_Y, const fixed_t a_XScale, const fixed_t a_YScale, const uint8_t* const a_ExtraMap);
}

