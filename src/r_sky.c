// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION:

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
