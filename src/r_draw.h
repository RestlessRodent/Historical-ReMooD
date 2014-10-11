// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: low level span/column drawer functions.

#ifndef __R_DRAW__
#define __R_DRAW__

#include "doomtype.h"

/* Define lightetable_t */
#if !defined(__REMOOD_LITETABLE_DEFINED)
	typedef uint8_t lighttable_t;
	#define __REMOOD_LITETABLE_DEFINED
#endif

//
// Frame flags:
// handles maximum brightness (torches, muzzle flare, light sources)
//

// faB: I noticed they didn't use the 32 bits of the frame field,
//      so now we use the upper 16 bits for new effects.

#define FF_FRAMEMASK    0x7fff	// only the frame number
#define FF_FULLBRIGHT   0x8000	// frame always appear full bright (fixedcolormap)

// faB:
//  MF_SHADOW is no more used to activate translucency (or the old fuzzy)
//  The frame field allows to set translucency per frame, instead of per sprite.
//  Now, (frame & FF_TRANSMASK) is the translucency table number, if 0
//  it is not translucent.

// Note:
//  MF_SHADOW still affects the targeting for monsters (they miss more)

#define FF_TRANSMASK   0x0F0000	// 0 = no trans(opaque), 1-7 = transl. table
#define FF_TRANSSHIFT       16

// faB: new 'alpha' shade effect, for smoke..

#define FF_SMOKESHADE  0x800000	// sprite is an alpha channel

// -------------------------------
// COMMON STUFF FOR 8bpp AND 16bpp
// -------------------------------
extern uint8_t** activeylookup;
extern uint8_t** ylookup;
extern uint8_t** ylookup1;
extern uint8_t** ylookup2;
extern uint8_t** ylookup4[MAXSPLITS];
extern int* columnofs;

// -------------------------
// COLUMN DRAWING CODE STUFF
// -------------------------

extern lighttable_t* dc_colormap;
extern lighttable_t* dc_wcolormap;	//added:24-02-98:WATER!
extern int dc_x;
extern int dc_yl;
extern int dc_yh;
extern int dc_yw;				//added:24-02-98:WATER!
extern fixed_t dc_iscale;
extern fixed_t dc_texturemid;

extern uint8_t* dc_source;		// first pixel in a column

// translucency stuff here
extern uint8_t* transtables;	// translucency tables, should be (*transtables)[5][256][256]
extern uint8_t* dc_transmap;

// Variable flat sizes SSNTails 06-10-2003
extern int flatsize;
extern int flatmask;
extern int flatsubtract;

// translation stuff here

extern uint8_t* translationtables;
extern uint8_t* dc_translation;

extern struct r_lightlist_s* dc_lightlist;
extern int dc_numlights;
extern int dc_maxlights;

//Fix TUTIFRUTI
extern int dc_texheight;

// -----------------------
// SPAN DRAWING CODE STUFF
// -----------------------

extern int ds_y;
extern int ds_x1;
extern int ds_x2;

extern lighttable_t* ds_colormap;

extern fixed_t ds_xfrac;
extern fixed_t ds_yfrac;
extern fixed_t ds_xstep;
extern fixed_t ds_ystep;

extern uint8_t* ds_source;		// start of a 64*64 tile image
extern uint8_t* ds_transmap;

// viewborder patches lump numbers
#define BRDR_T      0
#define BRDR_B      1
#define BRDR_L      2
#define BRDR_R      3
#define BRDR_TL     4
#define BRDR_TR     5
#define BRDR_BL     6
#define BRDR_BR     7

extern int viewborderlump[8];

// ------------------------------------------------
// r_draw.c COMMON ROUTINES FOR BOTH 8bpp and 16bpp
// ------------------------------------------------

//added:26-01-98: called by SCR_Recalc() when video mode changes
void R_RecalcFuzzOffsets(void);

// Initialize color translation tables, for player rendering etc.
void R_InitTranslationTables(void);

void R_InitViewBuffer(int width, int height, int yextra);

void R_InitViewBorder(void);

void R_VideoErase(unsigned ofs, int count);

// Rendering function.
void R_FillBackScreen(void);

// If the view size is not full screen, draws a border around it.
void R_DrawViewBorder(void);

// -----------------
// 8bpp DRAWING CODE
// -----------------

#ifdef HORIZONTALDRAW
//Fab 17-06-98
void R_DrawHColumn_8(void);
#endif

void R_DrawColumn_8(void);
void R_DrawSkyColumn_8(void);
void R_DrawShadeColumn_8(void);	//smokie test..
void R_DrawFuzzColumn_8(void);
void R_DrawTranslucentColumn_8(void);
void R_DrawTranslatedColumn_8(void);
void R_DrawSpan_8(void);
void R_DrawPaintballColumn_8(void);

// SSNTails 11-11-2002
void R_DrawTranslatedTranslucentColumn_8(void);

void R_DrawTranslucentSpan_8(void);
void R_DrawFogSpan_8(void);
void R_DrawFogColumn_8(void);	//SoM: Test
void R_DrawColumnShadowed_8(void);
void R_DrawPortalColumn_8(void);

// ------------------
// 16bpp DRAWING CODE
// ------------------

void R_DrawColumn_16(void);
void R_DrawSkyColumn_16(void);
void R_DrawFuzzColumn_16(void);
void R_DrawTranslucentColumn_16(void);
void R_DrawTranslatedColumn_16(void);
void R_DrawSpan_16(void);

// =========================================================================
#endif							// __R_DRAW__
