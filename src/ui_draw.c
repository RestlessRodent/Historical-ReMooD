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
// Copyright (C) 2013-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: User Interface Drawer

/* NOT IN DEDICATED SERVER */
#if !defined(__REMOOD_DEDICATED)
/***************************/

/***************
*** INCLUDES ***
***************/

#include "ui.h"
#include "g_state.h"

/*****************
*** STRUCTURES ***
*****************/

/*****************
*** PROTOTYPES ***
*****************/

void D_UITitle(UI_BufferSpec_t* const a_Spec);

/****************
*** FUNCTIONS ***
****************/

/* UI_DrawLoop() -- UI Drawing Loop */
// This replaces D_Display()!
void UI_DrawLoop(void)
{
	UI_BufferSpec_t Spec;
	
	/* Obtain screen spec */
	// Screen is locked by soft buffer, if needed
	Spec.Data = I_VideoSoftBuffer(&Spec.w, &Spec.h, &Spec.d, &Spec.p);
	Spec.pd = Spec.p * Spec.d;
	
	/* Drawing is based on the current game state */
	switch (gamestate)
	{
			// Title Screen
		case GS_DEMOSCREEN:
			D_UITitle(&Spec);
			break;
		
			// Unknown
		default:
			break;
	}
	
	/* Draw any user interface elements on top */
	
	/* Update the screen */
	I_FinishUpdate();
	
	/* Unlock the screen */
	I_GetVideoBuffer(IVS_DONEWITHBUFFER, NULL);
}

/* NOT IN DEDICATED SERVER */
#endif
/***************************/


