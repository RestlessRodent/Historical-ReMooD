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

void Translucency_OnChange(void);

consvar_t cv_translucency = { "translucency", "1", CV_CALL | CV_SAVE, CV_OnOff, Translucency_OnChange };

//
// Action routine, for the ROCKET thing.
// This one adds trails of smoke to the rocket.
// The action pointer of the S_ROCKET state must point here to take effect.
// This routine is based on the Revenant Fireball Tracer code A_Tracer()
//
void A_SmokeTrailer(mobj_t* actor)
{
	mobj_t* th;
	
	// GhostlyDeath <March 6, 2012> -- Check version (not before Legacy 1.25)
	if (P_EXGSGetValue(PEXGSBID_CONOSMOKETRAILS))
		return;
	
	// Only every 4 gametics
	if (gametic % (4))
		return;
	
	// GhostlyDeath <April 8, 2012> -- Fix flawed legacy compat options?
		// For some reason they say that bullets puffs are between 1.11 and 1.24
		// inclusive and smoke is used on and after 1.25, but with all the logic
		// inside, there is no such thing there. So what gives?
	// This uses bullet puffs
	if (!P_EXGSGetValue(PEXGSBID_COUSEREALSMOKE))
	{
		P_SpawnPuff(actor->x, actor->y, actor->z);
	}
	
	// This uses actual smoke
	else
	{
		// add the smoke behind the rocket
		th = P_SpawnMobj(actor->x - actor->momx, actor->y - actor->momy, actor->z, INFO_GetTypeByName("LegacySmoke"));
	
		th->momz = FRACUNIT;
		th->tics -= P_Random() & 3;
		if (th->tics < 1)
			th->tics = 1;
	}	
}

static bool_t resettrans = false;

//  Set the translucency map for each frame state of mobj
//
void R_SetTrans(statenum_t state1, statenum_t state2, transnum_t transmap)
{
	state_t* state = &states[state1];
	
	do
	{
		state->frame &= ~FF_TRANSMASK;
		if (!resettrans)
			state->frame |= (transmap << FF_TRANSSHIFT);
		state++;
	}
	while (state1++ < state2);
}

void R_SetSmokeShade(statenum_t state1, statenum_t state2, bool_t smoke)
{
	state_t* state = &states[state1];
	
	do
	{
		if (smoke)
			state->frame |= FF_SMOKESHADE;
		else
			state->frame &= ~FF_SMOKESHADE;
		state++;
	}
	while (state1++ < state2);
}

//  hack the translucency in the states for a set of standard doom sprites
//
void P_SetTranslucencies(void)
{

	//revenant fireball
	R_SetTrans(S_TRACER, S_TRACER2, tr_transfir);
	R_SetTrans(S_TRACEEXP1, S_TRACEEXP3, tr_transmed);
	
	//rev. fireball. smoke trail
	R_SetTrans(S_SMOKE1, S_SMOKE5, tr_transmed);
	
	//imp fireball
	R_SetTrans(S_TBALL1, S_TBALL2, tr_transfir);
	R_SetTrans(S_TBALLX1, S_TBALLX3, tr_transmed);
	
	//archvile attack
	R_SetTrans(S_FIRE1, S_FIRE30, tr_transfir);
	
	//bfg ball
	R_SetTrans(S_BFGSHOT, S_BFGSHOT2, tr_transfir);
	R_SetTrans(S_BFGLAND, S_BFGLAND3, tr_transmed);
	R_SetTrans(S_BFGLAND4, S_BFGLAND6, tr_transmor);
	R_SetTrans(S_BFGEXP, 0, tr_transmed);
	R_SetTrans(S_BFGEXP2, S_BFGEXP4, tr_transmor);
	
	//plasma bullet
	R_SetTrans(S_PLASBALL, S_PLASBALL2, tr_transfir);
	R_SetTrans(S_PLASEXP, S_PLASEXP2, tr_transmed);
	R_SetTrans(S_PLASEXP3, S_PLASEXP5, tr_transmor);
	
	//bullet puff
	R_SetTrans(S_PUFF1, S_PUFF4, tr_transmor);
	
	//teleport fog
	R_SetTrans(S_TFOG, S_TFOG5, tr_transmed);
	R_SetTrans(S_TFOG6, S_TFOG10, tr_transmor);
	
	//respawn item fog
	R_SetTrans(S_IFOG, S_IFOG5, tr_transmed);
	
	//soulsphere
	R_SetTrans(S_SOUL, S_SOUL6, tr_transmed);
	//invulnerability
	R_SetTrans(S_PINV, S_PINV4, tr_transmed);
	//blur artifact
	R_SetTrans(S_PINS, S_PINS4, tr_transmed);
	//megasphere
	R_SetTrans(S_MEGA, S_MEGA4, tr_transmed);
	
	R_SetTrans(S_GREENTORCH, S_REDTORCH4, tr_transfx1);	// blue torch
	R_SetTrans(S_GTORCHSHRT, S_RTORCHSHRT4, tr_transfx1);	// short blue torch
	
	// flaming barrel !!
	R_SetTrans(S_BBAR1, S_BBAR3, tr_transfx1);
	
	//lost soul
	R_SetTrans(S_SKULL_STND, S_SKULL_DIE6, tr_transmed /*tr_transfx1 */ );
	//baron shot
	R_SetTrans(S_BRBALL1, S_BRBALL2, tr_transfir);
	R_SetTrans(S_BRBALLX1, S_BRBALLX3, tr_transmed);
	//demon spawnfire
	R_SetTrans(S_SPAWNFIRE1, S_SPAWNFIRE3, tr_transfir);
	R_SetTrans(S_SPAWNFIRE4, S_SPAWNFIRE8, tr_transmed);
	//caco fireball
	R_SetTrans(S_RBALL1, S_RBALL2, tr_transfir);
	R_SetTrans(S_RBALLX1, S_RBALLX3, tr_transmed);
	
	//arachno shot
	R_SetTrans(S_ARACH_PLAZ, S_ARACH_PLAZ2, tr_transfir);
	R_SetTrans(S_ARACH_PLEX, S_ARACH_PLEX2, tr_transmed);
	R_SetTrans(S_ARACH_PLEX3, S_ARACH_PLEX4, tr_transmor);
	R_SetTrans(S_ARACH_PLEX5, 0, tr_transhi);
	
	//blood puffs!
	//R_SetTrans (S_BLOOD1   ,            0, tr_transmed);
	//R_SetTrans (S_BLOOD2   , S_BLOOD3    , tr_transmor);
	
	//eye in symbol
	R_SetTrans(S_EVILEYE, S_EVILEYE4, tr_transmed);
	
	//mancubus fireball
	R_SetTrans(S_FATSHOT1, S_FATSHOT2, tr_transfir);
	R_SetTrans(S_FATSHOTX1, S_FATSHOTX3, tr_transmed);
	
	// rockets explosion
	R_SetTrans(S_EXPLODE1, S_EXPLODE2, tr_transfir);
	R_SetTrans(S_EXPLODE3, 0, tr_transmed);
	
	//Fab: lava/slime damage smoke test
	R_SetTrans(S_SMOK1, S_SMOK5, tr_transmed);
	R_SetTrans(S_SPLASH1, S_SPLASH3, tr_transmor);
	
	//R_SetSmokeShade(S_SMOK1, S_SMOK5, true);
}

void Translucency_OnChange(void)
{
	if (cv_translucency.value == 0)
		resettrans = true;
	if (!fuzzymode)
		P_SetTranslucencies();
	resettrans = false;
}

// =======================================================================
//                    FUNKY DEATHMATCH COMMANDS
// =======================================================================

void BloodTime_OnChange(void);

CV_PossibleValue_t bloodtime_cons_t[] = { {1, "MIN"}, {3600, "MAX"}, {0, NULL} };

// how much tics to last for the last (third) frame of blood (S_BLOODx)
consvar_t cv_bloodtime = { "bloodtime", "20", CV_NETVAR | CV_CALL | CV_SAVE, bloodtime_cons_t,
	BloodTime_OnChange
};

// Called when var. 'bloodtime' is changed : set the blood states duration
//
void BloodTime_OnChange(void)
{
	CONL_PrintF("blood lasts for %d seconds\n", cv_bloodtime.value);
}

void D_AddDeathmatchCommands(void)
{
	CV_RegisterVar(&cv_solidcorpse);	//p_enemy
	
	CV_RegisterVar(&cv_bloodtime);
	
	// BP:not realy in deathmatch but is just here
	CV_RegisterVar(&cv_translucency);
}
