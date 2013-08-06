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
// DESCRIPTION:
//      Sky rendering. The DOOM sky is a texture map like any
//      wall, wrapping around. A 1024 columns equal 360 degrees.
//      The default sky map is 256 columns and repeats 4 times
//      on a 320 screen?

#include "r_sky.h"
#include "r_things.h"









// SoM: I know I should be moving portals out of r_sky.c and as soon
// as I have time and a I will... But for now, they are mostly used
// for sky boxes anyway so they have a mostly appropriate home here.

//
// sky mapping
//
int skyflatnum;
int skytexture;
int skytexturemid;

fixed_t skyscale;
int skymode = 0;				// old (0), new (1) (quick fix!)

//
// R_InitSkyMap called at startup, once.
//
void R_InitSkyMap(void)
{
	// set at P_LoadSectors
	//skyflatnum = R_FlatNumForName ( SKYFLATNAME );
}

// set the correct scale for the sky at setviewsize
void R_SetSkyScale(void)
{
	//fix this quick mess
	if (skytexturemid > 100 << FRACBITS)
	{
		// normal aspect ratio corrected scale
		skyscale = FixedDiv(FRACUNIT, pspriteyscale);
	}
	else
	{
		// double the texture vertically, bleeergh!!
		skyscale = FixedDiv(FRACUNIT, pspriteyscale) >> 1;
	}
}
