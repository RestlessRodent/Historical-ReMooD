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
// DESCRIPTION: Hardware Video Drawer

/***************
*** INCLUDES ***
***************/

#include "vhw_wrap.h"
#include "vhw_locl.h"
#include "console.h"
#include "dstrings.h"
#include "screen.h"

/*************
*** LOCALS ***
*************/

/****************
*** FUNCTIONS ***
****************/

unsigned char NearestColor(unsigned char r, unsigned char g, unsigned char b);

/* VHW_SIDX_HUDDrawLine() -- Draws HUD Line */
void VHW_SIDX_HUDDrawLine(const vhwrgb_t a_RGB, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2)
{
	uint8_t* vBase, Color;
	uint32_t Pitch;
	int32_t dx, dy, xp1, yp1, xp2, yp2, n, d, na, np, cp, x, y;
	
	/* Check */
	// Both points off left side
	if (a_X1 < 0 && a_X2 < 0)
		return;
	
	// Both points off right side
	if (a_X1 >= vid.width && a_X2 >= vid.width)
		return;
	
	// Both points off top side
	if (a_Y1 < 0 && a_Y2 < 0)
		return;
	
	// Both points off bottom side
	if (a_Y1 >= vid.height && a_Y2 >= vid.height)
		return;
	
	/* Obtain screen */
	vBase = I_GetVideoBuffer(IVS_BACKBUFFER, &Pitch);
	
	/* Find Color */
	Color = NearestColor(VHWRGB_red(a_RGB), VHWRGB_green(a_RGB), VHWRGB_blue(a_RGB));
	
	/* Copy Position */
	x = a_X1;
	y = a_Y1;
	
	/* Calculate Draw */
	// Get Difference
	dx = a_X2 - a_X1;
	dy = a_Y2 - a_Y1;
	
	if (dx < 0)
		dx = -dx;
	if (dy < 0)
		dy = -dy;
	
	// X Increase/Decrease
	if (a_X2 >= a_X1)
		xp1 = xp2 = 1;
	else
		xp1 = xp2 = -1;
		
	// Y Increase/Decrease
	if (a_Y2 >= a_Y1)
		yp1 = yp2 = 1;
	else
		yp1 = yp2 = -1;
	
	// At least 1 x for every y
	if (dx >= dy)
	{
		xp1 = 0;
		yp2 = 0;
		d = dx;
		n = dx >> 1;
		na = dy;
		np = dx;
	}
	
	// At least 1 y for every x
	else
	{
		xp2 = 0;
		yp1 = 0;
		d = dy;
		n = dy >> 1;
		na = dx;
		np = dy;
	}
		
	/* Now Draw */
	for (cp = 0; cp <= np; cp++)
	{
		// Put Pixel
		if (x >= 0 && x < vid.width && y >= 0 && y < vid.height)
			vBase[(y * Pitch) + x] = Color;
		
		n += na;
		
		if (n >= d)
		{
			n -= d;
			x += xp1;
			y += yp1;
		}
		
		x += xp2;
		y += yp2;
	}
	
	/* Done with buffer */
	I_GetVideoBuffer(IVS_DONEWITHBUFFER, NULL);
}

/* VHW_SIDX_HUDDrawImageComplex() -- Draws complex image onto the screen */
void VHW_SIDX_HUDDrawImageComplex(const uint32_t a_Flags, V_Image_t* const a_Image, const int32_t a_X, const int32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const fixed_t a_XScale, const fixed_t a_YScale, const uint8_t* const a_ExtraMap)
{
	uint8_t* vBase;
	uint32_t Pitch;
	
	/* Obtain screen */
	vBase = I_GetVideoBuffer(IVS_BACKBUFFER, &Pitch);
	
	/* Draw it into the screen buffer */
	V_ImageDrawScaledIntoBuffer(a_Flags, a_Image, a_X, a_Y, a_Image->Width, a_Image->Height, a_XScale, a_YScale, a_ExtraMap, vBase, Pitch, vid.width, vid.height, vid.fxdupx, vid.fxdupy, vid.fdupx, vid.fdupy);
	
	I_GetVideoBuffer(IVS_DONEWITHBUFFER, NULL);
}

/* VHW_SIDX_SetViewport() -- Sets the viewport of the screen */
void VHW_SIDX_SetViewport(const int32_t a_X, const int32_t a_Y, const uint32_t a_W, const uint32_t a_H)
{
}

/* VHW_SIDX_HUDBlurBack() -- Blurs the background */
void VHW_SIDX_HUDBlurBack(const uint32_t a_Flags, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2)
{
	V_DrawFadeConsBackEx(a_Flags, a_X1, a_Y1, a_X2, a_Y2);
}

/* VHW_SIDX_HUDDrawBox() -- Draws colorized box */
void VHW_SIDX_HUDDrawBox(const uint32_t a_Flags, const uint8_t a_R, const uint8_t a_G, const uint8_t a_B, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2)
{
	V_DrawColorBoxEx(a_Flags, NearestColor(a_R, a_G, a_B), a_X1, a_Y1, a_X2, a_Y2);
}

/* VHW_SIDX_ClearScreen() -- Clears the screen */
void VHW_SIDX_ClearScreen(const uint8_t a_R, const uint8_t a_G, const uint8_t a_B)
{
}

