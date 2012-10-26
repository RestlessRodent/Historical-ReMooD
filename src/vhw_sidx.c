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
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
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

/* VHW_SIDX_HUDDrawLine() -- Draws HUD Line */
void VHW_SIDX_HUDDrawLine(const vhwrgb_t a_RGB, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2)
{
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

unsigned char NearestColor(unsigned char r, unsigned char g, unsigned char b);

/* VHW_SIDX_HUDDrawBox() -- Draws colorized box */
void VHW_SIDX_HUDDrawBox(const uint32_t a_Flags, const uint8_t a_R, const uint8_t a_G, const uint8_t a_B, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2)
{
	V_DrawColorBoxEx(a_Flags, NearestColor(a_R, a_G, a_B), a_X1, a_Y1, a_X2, a_Y2);
}


/* VHW_SIDX_ClearScreen() -- Clears the screen */
void VHW_SIDX_ClearScreen(const uint8_t a_R, const uint8_t a_G, const uint8_t a_B)
{
}

