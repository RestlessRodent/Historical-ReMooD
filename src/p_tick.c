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
// DESCRIPTION:
//      Archiving: SaveGame I/O.
//      Thinker, Ticker.

#include "doomstat.h"
#include "g_game.h"
#include "p_local.h"
#include "z_zone.h"
#include "t_script.h"
#include "d_net.h"

tic_t leveltime;

//
// THINKERS
// All thinkers should be allocated by Z_Malloc
// so they can be operated on uniformly.
// The actual structures will vary in size,
// but the first element must be thinker_t.
//

// Both the head and tail of the thinker list.
thinker_t thinkercap;

// g_ThinkerSizes -- Size of each thinker
const size_t g_ThinkerSizes[NUMPTHINKERTYPES] =
{
	sizeof(thinker_t),			// PTT_CAP,				// Thinker Cap
	sizeof(vldoor_t),			// PTT_VERTICALDOOR,	// T_VerticalDoor/vldoor_t
	sizeof(fireflicker_t),		// PTT_FIREFLICKER,		// T_FireFlicker/fireflicker_t
	sizeof(lightflash_t),		// PTT_LIGHTFLASH,		// T_LightFlash/lightflash_t
	sizeof(strobe_t),			// PTT_STROBEFLASH,		// T_StrobeFlash/strobe_t
	sizeof(glow_t),				// PTT_GLOW,			// T_Glow/glow_t
	sizeof(lightlevel_t),		// PTT_LIGHTFADE,		// T_LightFade/lightlevel_t
	sizeof(floormove_t),		// PTT_MOVEFLOOR,		// T_MoveFloor/floormove_t
	sizeof(ceiling_t),			// PTT_MOVECEILING,		// T_MoveCeiling/ceiling_t
	sizeof(plat_t),				// PTT_PLATRAISE,		// T_PlatRaise/plat_t
	sizeof(elevator_t),			// PTT_MOVEELEVATOR,	// T_MoveElevator/elevator_t
	sizeof(scroll_t),			// PTT_SCROLL,			// T_Scroll/scroll_t
	sizeof(friction_t),			// PTT_FRICTION,		// T_Friction/friction_t
	sizeof(pusher_t),			// PTT_PUSHER,			// T_Pusher/pusher_t
	sizeof(mobj_t),				// PTT_MOBJ,			// P_MobjThinker/mobj_t
};

//
// P_InitThinkers
//
void P_InitThinkers(void)
{
	// TODO FIXME: Fix memory leak here!
	thinkercap.prev = thinkercap.next = &thinkercap;
	thinkercap.Type = PTT_CAP;
}

//
// P_AddThinker
// Adds a new thinker at the end of the list.
//
void P_AddThinker(thinker_t* thinker, const P_ThinkerType_t a_Type)
{
	thinker->Type = a_Type;	
	
	thinkercap.prev->next = thinker;
	thinker->next = &thinkercap;
	thinker->prev = thinkercap.prev;
	thinkercap.prev = thinker;
}

//
// P_RemoveThinker
// Deallocation is lazy -- it will not actually be freed
// until its thinking turn comes up.
//
void P_RemoveThinker(thinker_t* thinker)
{
	// FIXME: NOP.
	thinker->function.acv = (actionf_v) (-1);
}

//
// P_RunThinkers
//
void P_RunThinkers(void)
{
	thinker_t* currentthinker;
	
	currentthinker = thinkercap.next;
	while (currentthinker != &thinkercap)
	{
		// GhostlyDeath <February 3, 2012> -- No thinkers?
		if (!currentthinker)
			break;
		
		if (currentthinker->function.acv == (actionf_v) (-1))
		{
			void* removeit;
			
			// time to remove it
			currentthinker->next->prev = currentthinker->prev;
			currentthinker->prev->next = currentthinker->next;
			removeit = currentthinker;
			currentthinker = currentthinker->next;
			
			// GhostlyDeath <April 20, 2012> -- Wipe away thinker
				// MIGHT BREAK DEMO COMPAT
			memset(removeit, 0xFF, sizeof(*removeit));
			
			Z_Free(removeit);
		}
		else
		{
			if (currentthinker->function.acp1)
				currentthinker->function.acp1(currentthinker);
			currentthinker = currentthinker->next;
		}
	}
}

//
// P_Ticker
//

void P_Ticker(void)
{
	tic_t LocalTic, SNAR;
	int i;
	
	/* If the game is paused, don't do anything */
	if (D_SyncNetIsPaused())
		return;
		
	/* While the game is behind, update it */
	while ((LocalTic = D_SyncNetMapTime()) < (SNAR = D_SyncNetAllReady()))
	{
		// GhostlyDeath <May 6, 2012> -- Player tic update
		D_NCSNetUpdateSingleTic();
		
		//fprintf(stderr, "Ran tic %lli / %lli.\n", LocalTic, SNAR);
		
		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i])
				P_PlayerThink(&players[i]);
				
		P_RunThinkers();
		P_UpdateSpecials();
		P_RespawnSpecials();
		
		// for par times
		leveltime++;
		
#ifdef FRAGGLESCRIPT
		// SoM: Update FraggleScript...
		T_DelayedScripts();
#endif
		
		// Increase local time
		D_SyncNetSetMapTime(++LocalTic);
	}
}
