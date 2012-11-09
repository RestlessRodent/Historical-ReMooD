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

/*************
*** LOCALS ***
*************/

static VHW_Mode_t l_VHWMode = VHWMODE_IDXSOFT;

/****************
*** FUNCTIONS ***
****************/

void (*VHW_HUDDrawLine)(const vhwrgb_t a_RGB, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2) = NULL;
void (*VHW_HUDDrawImageComplex)(const uint32_t a_Flags, V_Image_t* const a_Image, const int32_t a_X, const int32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const fixed_t a_XScale, const fixed_t a_YScale, const uint8_t* const a_ExtraMap) = NULL;
void (*VHW_SetViewport)(const int32_t a_X, const int32_t a_Y, const uint32_t a_W, const uint32_t a_H) = NULL;
void (*VHW_HUDBlurBack)(const uint32_t a_Flags, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2) = NULL;
void (*VHW_HUDDrawBox)(const uint32_t a_Flags, const uint8_t a_R, const uint8_t a_G, const uint8_t a_B, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2) = NULL;
void (*VHW_ClearScreen)(const uint8_t a_R, const uint8_t a_G, const uint8_t a_B) = NULL;

/* VHW_UseGLMode() -- Use openGL Mode */
bool_t VHW_UseGLMode(void)
{
#if defined(__REMOOD_OPENGL_SUPPORTED) && !defined(__REMOOD_OPENGL_CANCEL)
	if (M_CheckParm("-gl"))
		return true;
#endif
	return false;
}

/* VHW_GetMode() -- Gets video mode */
VHW_Mode_t VHW_GetMode(void)
{
	return l_VHWMode;
}

/* VHW_Init() -- Initialize Hardware Layer */
bool_t VHW_Init(const VHW_Mode_t a_Mode)
{
	/* Set */
	l_VHWMode = a_Mode;
	
	/* Use OpenGL Calls */
#if defined(__REMOOD_OPENGL_SUPPORTED) && !defined(__REMOOD_OPENGL_CANCEL)
	if (a_Mode == VHWMODE_OPENGL)
	{
		VHW_HUDDrawLine = VHW_GL_HUDDrawLine;
		VHW_HUDDrawImageComplex = VHW_GL_HUDDrawImageComplex;
		VHW_SetViewport = VHW_GL_SetViewport;
		VHW_HUDBlurBack = VHW_GL_HUDBlurBack;
		VHW_HUDDrawBox = VHW_GL_HUDDrawBox;
		VHW_ClearScreen = VHW_GL_ClearScreen;
	}
	
	/* Software Mode Calls */
	else
#endif
	{
		VHW_HUDDrawLine = VHW_SIDX_HUDDrawLine;
		VHW_HUDDrawImageComplex = VHW_SIDX_HUDDrawImageComplex;
		VHW_SetViewport = VHW_SIDX_SetViewport;
		VHW_HUDBlurBack = VHW_SIDX_HUDBlurBack;
		VHW_HUDDrawBox = VHW_SIDX_HUDDrawBox;
		VHW_ClearScreen = VHW_SIDX_ClearScreen;
	}	
	
	/* Success! */
	return true;
}

