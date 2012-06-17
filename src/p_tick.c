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

// g_ThinkerData -- Size of each thinker
extern const G_ThinkerInfo_t g_ThinkerData[NUMPTHINKERTYPES] =
{
	{sizeof(thinker_t), {NULL}},				// PTT_CAP
	{sizeof(vldoor_t), {T_VerticalDoor}},		// PTT_VERTICALDOOR
	{sizeof(fireflicker_t), {T_FireFlicker}},	// PTT_FIREFLICKER
	{sizeof(lightflash_t), {T_LightFlash}},		// PTT_LIGHTFLASH
	{sizeof(strobe_t), {T_StrobeFlash}},		// PTT_STROBEFLASH
	{sizeof(glow_t), {T_Glow}},					// PTT_GLOW
	{sizeof(lightlevel_t), {T_LightFade}},		// PTT_LIGHTFADE
	{sizeof(floormove_t), {T_MoveFloor}},		// PTT_MOVEFLOOR
	{sizeof(ceiling_t), {T_MoveCeiling}},		// PTT_MOVECEILING
	{sizeof(plat_t), {T_PlatRaise}},			// PTT_PLATRAISE
	{sizeof(elevator_t), {T_MoveElevator}},		// PTT_MOVEELEVATOR
	{sizeof(scroll_t), {T_Scroll}},				// PTT_SCROLL
	{sizeof(friction_t), {T_Friction}},			// PTT_FRICTION
	{sizeof(pusher_t), {T_Pusher}},				// PTT_PUSHER
	{sizeof(mobj_t), {P_MobjThinker}},			// PTT_MOBJ
};


thinker_t** g_ThinkerList = NULL;				// List of thinkers
size_t g_NumThinkerList = 0;					// Thinkers in list

//
// P_InitThinkers
//
void P_InitThinkers(void)
{
	// TODO FIXME: Fix memory leak here!
	thinkercap.prev = thinkercap.next = &thinkercap;
	thinkercap.Type = PTT_CAP;
	g_ThinkerList = NULL;
	g_NumThinkerList = 0;
}

//
// P_AddThinker
// Adds a new thinker at the end of the list.
//
void P_AddThinker(thinker_t* thinker, const P_ThinkerType_t a_Type)
{
	size_t i;
	
	/* Set Type */
	thinker->Type = a_Type;	
	
	/* Set Links */
	thinkercap.prev->next = thinker;
	thinker->next = &thinkercap;
	thinker->prev = thinkercap.prev;
	thinkercap.prev = thinker;
	
	/* Add to static list */
	// Find an empty spot in the list
	for (i = 0; i < g_NumThinkerList; i++)
		if (!g_ThinkerList[i])
		{
			g_ThinkerList[i] = thinker;
			return;
		}
	
	// Otherwise, add to end
	Z_ResizeArray((void**)&g_ThinkerList, sizeof(g_ThinkerList), g_NumThinkerList, g_NumThinkerList + 1);
	g_ThinkerList[g_NumThinkerList++] = thinker;
	
	// Make sure it gets freed
	Z_ChangeTag(g_ThinkerList, PU_LEVEL);
}

//
// P_RemoveThinker
// Deallocation is lazy -- it will not actually be freed
// until its thinking turn comes up.
//
void P_RemoveThinker(thinker_t* thinker)
{
	size_t i;
	
	// FIXME: NOP.
	thinker->function.acv = (actionf_v) (-1);
	
	// Remove from list
	for (i = 0; i < g_NumThinkerList; i++)
		if (g_ThinkerList[i] == thinker)
			g_ThinkerList[i] = NULL;
}

//
// P_RunThinkers
//
void P_RunThinkers(void)
{
	size_t i;
	thinker_t* currentthinker;
	thinker_t* removeit;
	
	currentthinker = thinkercap.next;
	while (currentthinker != &thinkercap)
	{
		// GhostlyDeath <February 3, 2012> -- No thinkers?
		if (!currentthinker)
			break;
		
		if (currentthinker->function.acv == (actionf_v) (-1))
		{
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
	int i;
	
	/* If the game is paused, don't do anything */
	if (D_SyncNetIsPaused())
		return;
	
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
}

