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
// Copyright (C) 2012-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: ReMooD Snow Renderer

#ifndef __SN_MAIN_H__
#define __SN_MAIN_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "doomdef.h"
#include "doomdata.h"
#include "m_fixed.h"
#include "r_defs.h"
#include "d_player.h"

/********************
*** GENERIC TYPES ***
********************/

#if defined(__REMOOD_SNOWFLOAT)
	/* Floating Point */
	typedef double raster_t;
	
	#define rAd(a,b) (((raster_t)(a)) + ((raster_t)(b)))
	#define rSu(a,b) (((raster_t)(a)) - ((raster_t)(b)))
	#define rMu(a,b) (((raster_t)(a)) * ((raster_t)(b)))
	#define rDi(a,b) (((raster_t)(a)) / ((raster_t)(b)))
	
	#define RASTERT_C(x) ((double)(x))
	
	#define SN_RasterToInt(x) ((int32_t)(x))
	#define SN_FixedToRaster(x) ((double)(FIXED_TO_FLOAT(((fixed_t)(x)))))
#else
	/* Fixed Point */
	typedef fixed_t raster_t;
	
	#define rAd(a,b) (((raster_t)(a)) + ((raster_t)(b)))
	#define rSu(a,b) (((raster_t)(a)) - ((raster_t)(b)))
	#define rMu(a,b) (FixedMul(((raster_t)(a)), ((raster_t)(b))))
	#define rDi(a,b) (FixedDiv(((raster_t)(a)), ((raster_t)(b))))
	
	#define RASTERT_C(x) (((fixed_t)(x)) << FRACBITS)
	
	#define SN_RasterToInt(x) ((int32_t)(((fixed_t)(x)) >> FRACBITS))
	#define SN_FixedToRaster(x) ((fixed_t)(x))
#endif

/**************
*** GLOBALS ***
**************/

extern bool_t g_SnowBug;						// Debug

/****************
*** FUNCTIONS ***
****************/

/*** SN_PREP.C ***/
bool_t SN_InitLevel(void);

/*** SN_MAIN.C ***/
void SN_ViewResize(void);
void SN_RenderView(player_t* player, const size_t a_Screen);

#endif /* __SN_MAIN_H__ */

