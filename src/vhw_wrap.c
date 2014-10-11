// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Hardware Video Drawer

/***************
*** INCLUDES ***
***************/

#include "vhw_wrap.h"
#include "vhw_locl.h"
#include "console.h"
#include "dstrings.h"
#include "m_argv.h"

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

