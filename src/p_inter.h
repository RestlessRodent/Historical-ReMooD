// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION:

#ifndef __P_INTER__
#define __P_INTER__

#include "doomtype.h"
#include "d_player.h"

void P_PlayerSwitchToFavorite(player_t* const a_Player, const bool_t a_JustSpawned);
PI_wepid_t P_PlayerBestWeapon(player_t* const a_Player, const bool_t a_Best);

bool_t P_GivePower(player_t*, int);
void P_CheckFragLimit(player_t* p);

void P_KillMobj(mobj_t* target, mobj_t* inflictor, mobj_t* source);
bool_t P_GiveBody(player_t* player, int num);

/* P_PMType_t -- Player Message Type */
typedef enum P_PMType_e
{
	PPM_UNKNOWN,								// Unknown
	PPM_PICKUP, 								// Object Picked up
	PPM_SECRET,									// Found a secret
	PPM_REDLOCK,								// Red Door
	PPM_YELLOWLOCK,								// Yellow Door
	PPM_BLUELOCK,								// Blue Door
	PPM_GENLOCK,								// General Locked Door
	
	NUMPPMTYPES
} P_PMType_t;

void P_FlashKeys(player_t* const a_Player, const bool_t a_WildCard, const uint32_t a_SetA, const uint32_t a_SetB);
void P_PlayerMessage(const P_PMType_t a_Type, mobj_t* const a_Picker, mobj_t* const a_Upper, const char** const a_MessageRef);

void P_BroadcastMessage(const char* const a_Message);
void P_ExitMessage(mobj_t* const a_Exiter, const char* const a_Message);
void P_DeathMessages(mobj_t* target, mobj_t* inflictor, mobj_t* source);

typedef bool_t (*P_TouchFunc_t)(mobj_t* const, mobj_t* const);

uint32_t P_TouchFuncToID(P_TouchFunc_t a_Func);
P_TouchFunc_t P_TouchIDToFunc(const uint32_t a_ID);

//added:28-02-98: boooring handling of thing(s) on top of thing(s)

/* BUGGY CODE
void P_CheckSupportThings (mobj_t* mobj);
void P_MoveSupportThings (mobj_t* mobj, fixed_t xmove,
                                        fixed_t ymove,
                                        fixed_t zmove);
void P_LinkFloorThing(mobj_t* mobj);
void P_UnlinkFloorThing(mobj_t* mobj);
*/

#endif
