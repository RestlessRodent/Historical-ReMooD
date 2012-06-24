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
// DESCRIPTION: ReMooD Renderer -- Draws Sky

/***************
*** INCLUDES ***
***************/

#include "doomdef.h"
#include "d_player.h"
#include "rx_main.h"
#include "doomstat.h"

/****************
*** FUNCTIONS ***
****************/

/* RX_DrawSky() -- Draws the sky */
void RX_DrawSky(RX_RenderSpec_t* const a_Spec)
{
	int32_t x, o;
	uint8_t* SkyCol;
	
	/* Get Sky Texture */
	
	/* Draw Sky */
	for (o = 0, x = a_Spec->ScrX; x < a_Spec->ScrEndX; x++, o++)
	{
		// Get sky column
		//SkyCol = R_GetColumn(skytexture, a_Spec->SkyBend[o]);
		
		// Draw column to screen
	}
	
	//uint8_t* R_GetColumn(skytexture, size_t col)
	
	//int32_t ScrX, ScrY, ScrW, ScrH;				// Screen Bounds
	//int32_t ScrEndX, ScrEndY;					// End of screen
}

