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
// DESCRIPTION: MIPS Emulator/Simulator

/***************
*** INCLUDES ***
***************/

#include "mips.h"
#include "z_zone.h"

/*****************
*** STRUCTURES ***
*****************/

/****************
*** FUNCTIONS ***
****************/

/* MIPS_VMAddMap() -- Adds virtual memory mapping */
bool_t MIPS_VMAddMap(MIPS_VM_t* const a_VM, void* const a_Real, const uint32_t a_Fake, const uint32_t a_Len)
{
	MIPS_Map_t* New;
	
	/* Check */
	if (!a_VM || !a_Real || !a_Len)
		return false;
	
	/* Add room for 1 more map */
	Z_ResizeArray((void**)&a_VM->Maps, sizeof(*a_VM->Maps), a_VM->NumMaps, a_VM->NumMaps + 1);
	New = &a_VM->Maps[a_VM->NumMaps++];
	
	/* Place data here */
	New->Len = a_Len;
	New->VMOff = a_Fake;
	New->RealMem = a_Real;
	
	/* Success */
	return true;
}

/* MIPS_VMRun() -- Runs virtual machine, for count opcodes */
void MIPS_VMRun(MIPS_VM_t* const a_VM, const uint32_t a_Count)
{
}

