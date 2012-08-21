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
// DESCRIPTION:
//      some new action routines, separated from the original doom
//      sources, so that you can include it or remove it easy.

#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "r_state.h"
#include "p_fab.h"
#include "m_random.h"
#include "p_demcmp.h"

//
// Action routine, for the ROCKET thing.
// This one adds trails of smoke to the rocket.
// The action pointer of the S_ROCKET state must point here to take effect.
// This routine is based on the Revenant Fireball Tracer code A_Tracer()
//
void A_SmokeTrailer(mobj_t* actor)
{
	mobj_t* th;
	
	// GhostlyDeath <March 6, 2012> -- Check version (not before Legacy 1.11)
	if (P_XGSVal(PGS_CONOSMOKETRAILS))
		return;
	
	// Only every 4 gametics
	if (gametic % (4))
		return;
		
	// GhostlyDeath <April 12, 2012> -- Extra puffs before v1.25
		// Before 1.25, bullet puffs appeared with smoke puffs for some reason.
	if (P_XGSVal(PGS_COEXTRATRAILPUFF))
	{
		PuffType = INFO_GetTypeByName("BulletPuff");
		P_SpawnPuff(actor->x, actor->y, actor->z);
	}
	
	// add the smoke behind the rocket
	th = P_SpawnMobj(actor->x - actor->momx, actor->y - actor->momy, actor->z, (P_XGSVal(PGS_COUSEREALSMOKE) ? INFO_GetTypeByName("LegacySmoke") : INFO_GetTypeByName("TracerSmoke")));

	th->momz = FRACUNIT;
	th->tics -= P_Random() & 3;
	if (th->tics < 1)
		th->tics = 1;
}

/* A_SmokeTrailerRocket() -- Trails for rockets */
void A_SmokeTrailerRocket(mobj_t* actor)
{
	A_SmokeTrailer(actor);
}

/* A_SmokeTrailerSkull() -- Trails for skulls */
void A_SmokeTrailerSkull(mobj_t* actor)
{
	/* Check flag */
	// Before v1.25? Lost souls did not emit smoke
	if (!P_XGSVal(PGS_COLOSTSOULTRAILS))
		return;
	
	A_SmokeTrailer(actor);
}

