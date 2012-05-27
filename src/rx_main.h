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
// Copyright (C) 2012 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: ReMooD Renderer

#ifndef __RX_MAIN_H__
#define __RX_MAIN_H__

/***************
*** INCLUDES ***
***************/

#include "doomdef.h"
#include "d_player.h"

/*****************
*** STRUCTURES ***
*****************/

/* RX_RenderSpec_t -- Render specification */
typedef struct RX_RenderSpec_s
{
	player_t* Player;							// Player being drawn
	int32_t RPVScreen;							// Screen
	
	int32_t ScrX, ScrY, ScrW, ScrH;				// Screen Bounds
	int32_t ScrEndX, ScrEndY;					// End of screen
	
	int32_t* SkyBend;							// Sky Bend
	
	/* Projection */
	angle_t Ang[3];								// Angles
	fixed_t ViewSin[3], ViewCos[3];				// Calcs for angles
	fixed_t CamD[3];							// Camera Position
	fixed_t ProjD[3];							// Projection D
	
	/* Buffers */
	int16_t* DepthBuffer;						// Depth Buffer
} RX_RenderSpec_t;

/****************
*** FUNCTIONS ***
****************/

/* rx_main.c */
void R_ExecuteSetViewSize_REMOOD(void);
void R_SetupFrame_REMOOD(player_t* player);
void R_RenderPlayerView_REMOOD(player_t* player, const size_t a_Screen);

/* rx_sky.c */
void RX_DrawSky(RX_RenderSpec_t* const a_Spec);

/* rx_draw.c */
void RX_DrawPoly(RX_RenderSpec_t* const a_Spec, const uint8_t a_Color, const fixed_t a_x1, const fixed_t a_y1, const fixed_t a_z1, const fixed_t a_x2, const fixed_t a_y2, const fixed_t a_z2, const fixed_t a_x3, const fixed_t a_y3, const fixed_t a_z3);

#endif /* __RX_MAIN_H__ */

