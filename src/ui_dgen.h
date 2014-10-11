// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: General Drawing

#ifndef __UI_DGEN_H__
#define __UI_DGEN_H__

/* NOT IN DEDICATED SERVER */
#if !defined(__REMOOD_DEDICATED)
/***************************/

/*****************************************************************************/

#include "m_fixed.h"

#define ___MERGE(a,b) a##b
#define __MERGE(a,b) ___MERGE(a,b)
#define MAKENAME(Str) __MERGE(UI_,__MERGE(__MERGE(UIDG_CODE,_),Str))

/* DrawImg() -- Draws image */
void MAKENAME(DrawImg)(UI_BufferSpec_t* const a_Spec, UI_Img_t* const a_Img, const int32_t a_X, const int32_t a_Y)
{
	register int x1, y1, x2, y2;
	register int y, ox, oy;
	ui_pixel_t* Dest, *Src;
	
	/* Check */
	if (!a_Spec || !a_Img)
		return;
	
	/* Coordinates outside buffer */
	if (a_X >= a_Spec->w || a_Y >= a_Spec->h)
		return;
	
	/* Calculate clipping coordinates */
	x2 = a_X + a_Img->l[0];
	y2 = a_Y + a_Img->l[1];
	
	if (x2 >= a_Spec->w)
		x2 = a_Spec->w - 1;
	if (y2 >= a_Spec->h)
		y2 = a_Spec->h - 1;
	
	// Resulting draw end is zero length or negative
	if (x2 <= a_X || y2 <= a_Y)
		return;
	
	// Left bound offset?
	if (a_X < 0)
	{
		x1 = 0;
		ox = a_X;
	}
	else
	{
		x1 = a_X;
		ox = 0;
	}
		
	if (a_Y < 0)
	{
		y1 = 0;
		oy = a_Y;
	}
	else
	{
		y1 = a_Y;
		oy = 0;
	}
	
	/* Draw loop */
	Dest = ((ui_pixel_t*)a_Spec->Data) + (uintptr_t)((y1 * a_Spec->p) + x1);
	Src = ((ui_pixel_t*)a_Img->Data) + (uintptr_t)((oy * a_Img->p) + ox);
	for (y = y1; y < y2; y++)
	{
		memcpy(Dest, Src, (x2 - x1) * sizeof(ui_pixel_t));
		
		Dest = (ui_pixel_t*)((uintptr_t)Dest + a_Spec->pd);
		Src = (ui_pixel_t*)((uintptr_t)Src + a_Img->pd);
	}
}

/* DrawImgScale() -- Draws Image scaled */
// Start coords are NOT scaled!
// TODO FIXME: RANGE CAP THIS FUNCTION FOR SECURITY AND TO PREVENT CRASHES!
void MAKENAME(DrawImgScale)(UI_BufferSpec_t* const a_Spec, UI_Img_t* const a_Img, const int32_t a_X, const int32_t a_Y, const fixed_t a_sW, const fixed_t a_sH)
{
	register fixed_t ix, iy;
	register fixed_t sw, sh;
	register int32_t x1, y1, x2, y2;
	register fixed_t x, y;
	register fixed_t ppx, ppy, rppx;
	ui_pixel_t* Dest, *Src;
	
	/* Check */
	if (!a_Spec || !a_Img || !a_sW || !a_sH)
		return;
	
	/* Off screen edge */
	if (a_X >= a_Spec->w || a_Y >= a_Spec->h)
		return;
	
	/* Determine scaled width and height */
	sw = FixedMul(a_Img->l[0] << FRACBITS, a_sW);
	sh = FixedMul(a_Img->l[1] << FRACBITS, a_sH);
	
	/* Determine possibly capped destination range coordinates */
	x1 = a_X;
	y1 = a_Y;
	x2 = x1 + (sw >> FRACBITS);
	y2 = x1 + (sh >> FRACBITS);
	
	/* Inverted scale coords */
	ix = FixedDiv(1 << FRACBITS, a_sW);
	iy = FixedDiv(1 << FRACBITS, a_sH);
	
	/* Draw Loop */
	rppx = 0;
	ppy = 0;
	
	Dest = ((ui_pixel_t*)a_Spec->Data) + (uintptr_t)((y1 * a_Spec->p) + x1);
	for (y = y1; y < y2; y++, ppy += iy)
	{
		// Calculate source coordinate
		Src = ((ui_pixel_t*)a_Img->Data) + (uintptr_t)(((ppy >> FRACBITS) * a_Img->p) + (rppx >> FRACBITS));
		
		// Draw from source
		for (ppx = rppx, x = x1; x < x2; x++, ppx += ix)
			Dest[x] = Src[ppx >> FRACBITS];
		
		// Add to dest coordinate
		Dest = (ui_pixel_t*)((uintptr_t)Dest + a_Spec->pd);
	}
}

/*****************************************************************************/

/* NOT IN DEDICATED SERVER */
#endif
/***************************/

#endif /* __UI_DGEN_H__ */

