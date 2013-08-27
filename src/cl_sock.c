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
// DESCRIPTION: Game Sockets

/***************
*** INCLUDES ***
***************/

#include "cl.h"
#include "i_util.h"
#include "z_zone.h"

/*****************
*** STRUCTURES ***
*****************/

/**************
*** GLOBALS ***
**************/

CL_Socket_t** g_CLSocks = NULL;					// Sockets
int32_t g_NumCLSocks = 0;						// Number of sockets

/****************
*** FUNCTIONS ***
****************/

/* CL_InitSocks() -- Initialize sockets */
int32_t CL_InitSocks(void)
{
	int32_t i;
	CL_Socket_t* This;
	
	/* Allocate sturctures */
	g_NumCLSocks = 1 + I_NumJoysticks();
	g_CLSocks = Z_Malloc(sizeof(*g_CLSocks) * g_NumCLSocks, PU_STATIC, NULL);
	
	/* Add Keyboard/Mouse Socket */
	This = g_CLSocks[0] = Z_Malloc(sizeof(*This), PU_STATIC, NULL);
	
	// Initialize
	This->JoyID = -1;
	
	/* Add sockets for every joystick */
	for (i = 0; i < I_NumJoysticks(); i++)
	{
		// Allocate
		This = g_CLSocks[1 + i] = Z_Malloc(sizeof(*This), PU_STATIC, NULL);
		
		// Initialize
		This->Flags |= CLSF_JOYSTICK;
		This->JoyID = i;
	}
	
	/* Return number of control sockets */
	return g_NumCLSocks;
}


