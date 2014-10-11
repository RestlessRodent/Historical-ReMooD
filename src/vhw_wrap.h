// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Hardware Video Drawer

#ifndef __VHW_WRAP_H__
#define __VHW_WRAP_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"


/* Define V_Image_t */
#if !defined(__REMOOD_VIMAGET_DEFINED)
	typedef struct V_Image_s V_Image_t;
	#define __REMOOD_VIMAGET_DEFINED
#endif

/****************
*** CONSTANTS ***
****************/

/* VHW_Mode_t -- Hardware Mode */
typedef enum VHW_Mode_e
{
	VHWMODE_IDXSOFT,							// Indexed Software Mode
	VHWMODE_OPENGL,								// OpenGL
} VHW_Mode_t;

typedef double vhw_t;
typedef uint32_t vhwrgb_t;

#define VHWT_C(x) ((double)(x))

#define VHWRGB(r,g,b) ((vhwrgb_t)(((((uint32_t)(r)) & UINT32_C(0xFF)) << UINT32_C(24)) | ((((uint32_t)(g)) & UINT32_C(0xFF)) << UINT32_C(16)) | ((((uint32_t)(b)) & UINT32_C(0xFF)) << UINT32_C(8))))

#define VHWRGB_red(x) 	((((uint32_t)(x)) >> UINT32_C(24)) & UINT32_C(0xFF))
#define VHWRGB_green(x) ((((uint32_t)(x)) >> UINT32_C(16)) & UINT32_C(0xFF))
#define VHWRGB_blue(x) 	((((uint32_t)(x)) >> UINT32_C(8)) & UINT32_C(0xFF))

#define VHW_abs(x)		((vhw_t)fabs((vhw_t)(x)))
#define VHW_mul(x,y)	((vhw_t)(x) * (vhw_t)(y))
#define VHW_div(x,y)	((vhw_t)(x) / (vhw_t)(y))
#define VHW_inc(x)		((x) += 1.0)
#define VHW_dec(x)		((x) -= 1.0)
#define VHW_round(x)	(round((x)))
#define VHW_int(x)		(rint((x)))
#define VHW_frac(x)		(modf((x), &__vhw_junk))
#define VHW_rfrac(x)	(1.0 - VHW_frac((x)))

/****************
*** FUNCTIONS ***
****************/

bool_t VHW_UseGLMode(void);
VHW_Mode_t VHW_GetMode(void);
bool_t VHW_Init(const VHW_Mode_t a_Mode);

extern void (*VHW_HUDDrawLine)(const vhwrgb_t a_RGB, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2);

extern void (*VHW_HUDDrawImageComplex)(const uint32_t a_Flags, V_Image_t* const a_Image, const int32_t a_X, const int32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const fixed_t a_XScale, const fixed_t a_YScale, const uint8_t* const a_ExtraMap);

extern void (*VHW_SetViewport)(const int32_t a_X, const int32_t a_Y, const uint32_t a_W, const uint32_t a_H);

extern void (*VHW_HUDBlurBack)(const uint32_t a_Flags, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2);

extern void (*VHW_HUDDrawBox)(const uint32_t a_Flags, const uint8_t a_R, const uint8_t a_G, const uint8_t a_B, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2);

extern void (*VHW_ClearScreen)(const uint8_t a_R, const uint8_t a_G, const uint8_t a_B);

#endif /* __VHW_WRAP_H__ */


