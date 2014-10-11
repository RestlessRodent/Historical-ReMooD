// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION:

#include "d_think.h"
#include "p_spec.h"
#include "p_mobj.h"
#include "z_zone.h"
#include "d_player.h"
#include "p_local.h"
#include "t_script.h"








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
	{sizeof(thinker_t), {NULL}},				// PTT_DEFUNCT
	{sizeof(thinker_t), {NULL}},				// PTT_DELETEME
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
		
		if (currentthinker->function.acv == (actionf_v)(-1) ||
			currentthinker->Type == PTT_DELETEME ||
			currentthinker->Type == PTT_DEFUNCT)
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
	
	/* If the game is paused, do nothing */
	if (paused)
		return;
	
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
			P_PlayerThink(&players[i]);
			
	P_RunThinkers();
	P_UpdateSpecials();
	P_RespawnSpecials();
	
	// for par times
	leveltime++;
	
	// SoM: Update FraggleScript...
	T_DelayedScripts();
}

/* G_ThinkTypeToFunc() -- Converts thinker type to function */
actionf_t G_ThinkTypeToFunc(const P_ThinkerType_t a_Type)
{
	actionf_t Bad;
	
	/* Check */
	if (a_Type < 0 || a_Type >= NUMPTHINKERTYPES)
	{
		Bad.acv = NULL;
		return Bad;
	}
	
	/* Delete? */
	if (a_Type == PTT_DELETEME)
	{
		Bad.acv = (actionf_v)(-1);
		return Bad;
	}
	
	/* Return from table */
	return g_ThinkerData[a_Type].Func;
}

/* G_ThinkFuncToType() -- Converts function to thinker type */
P_ThinkerType_t G_ThinkFuncToType(actionf_t a_Func)
{
	int32_t i;
	
	/* Missing func? */
	if (!a_Func.acv)
		return 0;
	
	/* Delete? */
	if (a_Func.acv == (actionf_v)(-1))
		return PTT_DELETEME;
	
	/* Find match */
	for (i = 0; i < NUMPTHINKERTYPES; i++)
		if (a_Func.acv == g_ThinkerData[i].Func.acv)
			return i;
	
	/* Not found */
	return 0;
}

