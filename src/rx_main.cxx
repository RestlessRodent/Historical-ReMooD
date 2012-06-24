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

/***************
*** INCLUDES ***
***************/

#include "doomdef.h"
#include "d_player.h"
#include "rx_main.h"

/*****************
*** STRUCTURES ***
*****************/

/* RX_FrameInfo_t -- Frame Info */
typedef struct RX_FrameInfo_s
{
	int32_t x, y, w, h;							// Position
	int32_t* SkyBend;							// Bend of the sky
} RX_FrameInfo_t;

/*************
*** LOCALS ***
*************/

static RX_FrameInfo_t l_RXFrames[MAXSPLITSCREEN];

/****************
*** FUNCTIONS ***
****************/

/* RX_SetupFrame() -- Sets up the drawing frame */
void RX_SetupFrame(RX_RenderSpec_t* const a_Spec)
{
	player_t* Player;
	mobj_t* Mo;
	size_t i;
	
	/* Get Object */
	Player = a_Spec->Player;
	Mo = Player->mo;
	
	/* Projection */
	a_Spec->Ang[0] = Mo->angle;
	a_Spec->Ang[1] = Player->aiming;
	a_Spec->Ang[2] = 0;				// No Rolling
	
	/* Calculate View/Sin for all of the angles */
	for (i = 0; i < 3; i++)
	{
		a_Spec->ViewSin[i] = finesine[a_Spec->Ang[i] >> ANGLETOFINESHIFT];
		a_Spec->ViewCos[i] = finecosine[a_Spec->Ang[i] >> ANGLETOFINESHIFT];
	}
	
	/* Camera */
	a_Spec->CamD[0] = Mo->x;
	a_Spec->CamD[1] = Mo->y;
	a_Spec->CamD[2] = Mo->z;
}

/* R_ExecuteSetViewSize_REMOOD() -- Sets view size */
void R_ExecuteSetViewSize_REMOOD(void)
{
	int32_t n, i;
	fixed_t SkyFOV;
	
	/* Setup each split-screen */
	for (n = 0; n < g_SplitScreen + 1; n++)
	{
		// Initial Screen Box
			// 1 Player
		if (g_SplitScreen <= 0)
		{
			l_RXFrames[n].w = vid.width;
			l_RXFrames[n].h = vid.height;
			l_RXFrames[n].x = 0;
			l_RXFrames[n].y = 0;
		}
			// 2 Players
		else if (g_SplitScreen == 1)
		{
			l_RXFrames[n].w = vid.width;
			l_RXFrames[n].h = vid.height / 2;
			l_RXFrames[n].x = 0;
			l_RXFrames[n].y = l_RXFrames[n].h * n;
		}
			// 3+ Players
		else
		{
			l_RXFrames[n].w = vid.width / 2;
			l_RXFrames[n].h = vid.height / 2;
			l_RXFrames[n].x = l_RXFrames[n].w * (n & 1);
			l_RXFrames[n].y = l_RXFrames[n].h * ((n >> 1) & 1);
		}
		
		// Allocate?
		if (l_RXFrames[n].SkyBend)
			Z_Free(l_RXFrames[n].SkyBend);
		l_RXFrames[n].SkyBend = (int32_t*)Z_Malloc(sizeof(*l_RXFrames[n].SkyBend) * l_RXFrames[n].w, PU_STATIC, NULL);
		
		// Bending of the sky (texture offsets)
		SkyFOV = FixedMul(l_RXFrames[n].w, 186413);	// (256 / 90)
		for (i = 0; i < l_RXFrames[n].w; i++)
			l_RXFrames[n].SkyBend[i] = i + i;
	}
}

/* R_RenderPlayerView_REMOOD() -- Renders player view */
void R_RenderPlayerView_REMOOD(player_t* player, const size_t a_Screen)
{
	RX_RenderSpec_t Spec;
	
	/* Init Spec */
	// Clear
	memset(&Spec, 0, sizeof(Spec));
	
	// Set fields
	Spec.Player = player;
	Spec.RPVScreen = a_Screen;
	
	// Set values from screen ID
	Spec.ScrX = l_RXFrames[a_Screen].x;
	Spec.ScrY = l_RXFrames[a_Screen].y;
	Spec.ScrW = l_RXFrames[a_Screen].w;
	Spec.ScrH = l_RXFrames[a_Screen].h;
	Spec.ScrEndX = Spec.ScrX + Spec.ScrW;
	Spec.ScrEndY = Spec.ScrY + Spec.ScrH;
	
	// Allocate Depth Buffer
	Spec.DepthBuffer = (int16_t*)Z_Malloc(sizeof(*Spec.DepthBuffer) * (Spec.ScrW * Spec.ScrH), PU_STATIC, NULL);
	
	// Sky Info
	Spec.SkyBend = l_RXFrames[a_Screen].SkyBend;
	
	/* Setup Frame */
	RX_SetupFrame(&Spec);
	
	/* Draw Everything */
	// Sky is the background
	RX_DrawSky(&Spec);
	
	// Draw the level
	
	// Draw sprites
	
	/* Finalize */
	// Destroy the depth buffer
	Z_Free(Spec.DepthBuffer);
}

