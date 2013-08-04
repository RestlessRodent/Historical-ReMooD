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
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Master Server and Other interaction protocols

/***************
*** INCLUDES ***
***************/

#include "d_netmst.h"

/*************
*** LOCALS ***
*************/



/****************
*** FUNCTIONS ***
****************/

/* I_BootHTTPSpy() -- Permits game spying via HTTP */
bool_t I_BootHTTPSpy(void)
{
	uint16_t Port;
	
	/* Only allow if -spy is passed */
	if (!M_CheckParm("-spy"))
		return false;
	
	// Takes parameter
	if (!M_IsNextParm())
		return false;
	
	// Read Port
	Port = C_strtou32(M_GetNextParm(), NULL, 10);
	
	// Bad port?
	if (!Port)
		return false;
	
	/* Create TCP Server */
	return false;
}

/* I_UpdateHTTPSpy() -- Handles HTTP Spy */
void I_UpdateHTTPSpy(void)
{
}

