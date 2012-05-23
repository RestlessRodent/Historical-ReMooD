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
// DESCRIPTION: low level span/column drawer functions.

#ifndef __R_DRAW__
#define __R_DRAW__

#include "r_defs.h"

// -------------------------------
// COMMON STUFF FOR 8bpp AND 16bpp
// -------------------------------
extern uint8_t** activeylookup;
extern uint8_t** ylookup;
extern uint8_t** ylookup1;
extern uint8_t** ylookup2;
extern uint8_t** ylookup4[MAXSPLITSCREENPLAYERS];
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

void R_InitViewBuffer(int width, int height);

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
