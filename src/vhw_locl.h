// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Hardware Video Drawer

#ifndef __VHW_LOCL_H__
#define __VHW_LOCL_H__

/***************
*** INCLUDES ***
***************/



/****************
*** FUNCTIONS ***
****************/

/*** OPENGL ***/

#if defined(__REMOOD_OPENGL_SUPPORTED) && !defined(__REMOOD_OPENGL_CANCEL)

void VHW_GL_HUDDrawLine(const vhwrgb_t a_RGB, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2);
void VHW_GL_HUDDrawImageComplex(const uint32_t a_Flags, V_Image_t* const a_Image, const int32_t a_X, const int32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const fixed_t a_XScale, const fixed_t a_YScale, const uint8_t* const a_ExtraMap);
void VHW_GL_SetViewport(const int32_t a_X, const int32_t a_Y, const uint32_t a_W, const uint32_t a_H);
void VHW_GL_HUDBlurBack(const uint32_t a_Flags, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2);
void VHW_GL_HUDDrawBox(const uint32_t a_Flags, const uint8_t a_R, const uint8_t a_G, const uint8_t a_B, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2);
void VHW_GL_ClearScreen(const uint8_t a_R, const uint8_t a_G, const uint8_t a_B);

#endif

/*** SOFTWARE INDEXED ***/

void VHW_SIDX_HUDDrawLine(const vhwrgb_t a_RGB, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2);
void VHW_SIDX_HUDDrawImageComplex(const uint32_t a_Flags, V_Image_t* const a_Image, const int32_t a_X, const int32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const fixed_t a_XScale, const fixed_t a_YScale, const uint8_t* const a_ExtraMap);
void VHW_SIDX_SetViewport(const int32_t a_X, const int32_t a_Y, const uint32_t a_W, const uint32_t a_H);
void VHW_SIDX_HUDBlurBack(const uint32_t a_Flags, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2);
void VHW_SIDX_HUDDrawBox(const uint32_t a_Flags, const uint8_t a_R, const uint8_t a_G, const uint8_t a_B, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2);
void VHW_SIDX_ClearScreen(const uint8_t a_R, const uint8_t a_G, const uint8_t a_B);

#endif /* __VHW_LOCL_H__ */


