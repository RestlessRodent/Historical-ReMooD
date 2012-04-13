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
// DESCRIPTION: holds the weapon info for now...

#include <stdio.h>

// We are referring to sprite numbers.
#include "info.h"
#include "d_items.h"

//
// PSPRITE ACTIONS for weapons.
// This struct controls the weapon animations.
//
// Each entry is:
//  ammo/amunition type
//  upstate
//  downstate
//  readystate
//  atkstate, i.e. attack/fire/hit frame
//  flashstate, muzzle flash
//

/* D_DumpItemRMOD() -- Dump RMOD for weapons and ammo */
void D_DumpItemRMOD(void)
{
	size_t i;
	FILE* f;
	weaponinfo_t* ThisWep;
	
	/* Create file */
	f = fopen("rmoddump", "w+b");
	
	/* Dump Ammo */
	for (i = 0; i < NUMAMMO; i++)
	{
	}
	
	/* Dump Weapons */
	for (i = 0; i < NUMWEAPONS; i++)
	{
		// Get the current weapon
		ThisWep = &wpnlev1info[i];
		
		// Print opener
		fprintf(f, "MapWeapon \"%s\"\n", ThisWep->ClassName);
		fprintf(f, "{\n");
		
		// Print stuff about weapon
		
		// Print weapon states
		
		// Print closure
		fprintf(f, "}\n");
		fprintf(f, "\n");
	}
	
	/* Close file */
	fclose(f);
}

