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
// Copyright (C) 2012-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Bot Node Related Code

/***************
*** INCLUDES ***
***************/

#include "b_priv.h"

/****************
*** FUNCTIONS ***
****************/

/* B_WorkShoreMove() -- Utilize shore movement */
bool_t B_WorkShoreMove(struct B_GhostBot_s* a_Bot, const size_t a_JobID)
{
	B_BotTarget_t* FFree, *ShoreTarg;
	B_ShoreNode_t* This;
	B_GhostNode_t* Node;
	mobj_t* Mo;
	int32_t i;
	fixed_t Dist;
	
	/* Get Object */
	Mo = a_Bot->Mo;
	
	/* If no path exists, ignore */
	if (!a_Bot->NumShore || !Mo)
	{
		a_Bot->Jobs[a_JobID].Sleep = gametic + (TICRATE);
		return true;
	}
	
	/* If target is reached, clear targets */
	// Also check if we still even desire this thing we are moving twords
	if ((a_Bot->ConfirmDesireF && !a_Bot->ConfirmDesireF(a_Bot)) || a_Bot->ShoreIt >= a_Bot->NumShore)
	{
		B_ShoreClear(a_Bot, true);
		a_Bot->Jobs[a_JobID].Sleep = gametic + (TICRATE);
		return true;
	}
	
	/* Find existing target, if it exists */
	FFree = ShoreTarg = NULL;
	for (i = 0; i < MAXBOTTARGETS; i++)
	{
		// If not set, set first free
		if (!a_Bot->Targets[i].IsSet)
		{
			if (!FFree)
				FFree = &a_Bot->Targets[i];
			continue;
		}
		
		// Shore target?
		if (a_Bot->Targets[i].MoveTarget)
			if (a_Bot->Targets[i].Key == SHOREKEY)
			{
				ShoreTarg = &a_Bot->Targets[i];
				break;
			}
	}
	
	// No shore target
	if (!ShoreTarg)
	{
		// Need a free target
		if (!FFree)
		{
			a_Bot->Jobs[a_JobID].Sleep = gametic + (TICRATE);
			return true;
		}
		
		// Set as the free one
		ShoreTarg = FFree;
	}
	
	/* Setup target */
	ShoreTarg->IsSet = true;
	ShoreTarg->MoveTarget = true;
	ShoreTarg->ExpireTic = gametic + (TICRATE * 5);
	ShoreTarg->Priority = 100;
	ShoreTarg->Key = SHOREKEY;
	
	/* Get current iterated target */
	This = a_Bot->Shore[a_Bot->ShoreIt];
	
	/* If the current node is a head or tail... */
#if 0
	if (This->Type == BST_HEAD || This->Type == BST_TAIL)
	{
#endif
		// Set position to this target
		ShoreTarg->x = This->Pos[0];
		ShoreTarg->y = This->Pos[1];
		
		// If near the target, iterate
		Dist = P_AproxDistance(Mo->x - ShoreTarg->x, Mo->y - ShoreTarg->y);
		
		if (Dist < FIXEDT_C(24))
			a_Bot->ShoreIt++;
#if 0
	}
	
	/* Otherwise, it is a standard node */
	else
	{
		// Set position to target node
		ShoreTarg->x = This->Pos[0];
		ShoreTarg->y = This->Pos[1];
		
		// If navigation node is this target, iterate
		if (a_Bot->AtNode == This->BotNode)
			a_Bot->ShoreIt++;
	}
#endif
	
	/* Continue */
	a_Bot->Jobs[a_JobID].Sleep = gametic + (TICRATE >> 1);
	return true;
}

/*****************************************************************************/

/* B_Desire_t -- Something the bot desires */
typedef struct B_Desire_s
{
	bool_t IsWeapon;							// Weapon
	bool_t IsHealth;							// Health
	bool_t IsAmmo;								// Ammo
	bool_t IsArmor;								// Armor
	
	int32_t SpecID;								// Specific ID of thing wanted
	
	mobj_t* Thing;								// Thing it wants
	fixed_t Dist;								// Distance
} B_Desire_t;

/* BS_GHOST_CDF_Weapon() -- Confirm that we want the weapon still */
static bool_t BS_GHOST_CDF_Weapon(struct B_GhostBot_s* a_Bot)
{
	/* Illegal type? */
	if (a_Bot->DesireType < 0 || a_Bot->DesireType >= NUMWEAPONS)
		return false;
	
	/* Object is not around? */
	if (!a_Bot->DesireMo)
		return false;
	
	/* We just happen to own this weapon? */
	// Maybe a script gave it to us, or someone died and we ran over it, or
	// we picked it up on the way to get the weapon we want.
	if (a_Bot->Player->weaponowned[a_Bot->DesireType])
		return false;
	
	/* Otherwise, we use it still */
	return true;
}

/* BS_GHOST_CDF_Armor() -- Still wants armor */
static bool_t BS_GHOST_CDF_Armor(struct B_GhostBot_s* a_Bot)
{
	/* Enough points */
	if (a_Bot->Player->armorpoints >= a_Bot->Player->MaxArmor[0])
		return false;
	
	/* Otherwise, still want */
	return true;
}

/* BS_GHOST_CDF_Health() -- Still wants Health */
static bool_t BS_GHOST_CDF_Health(struct B_GhostBot_s* a_Bot)
{
	/* Enough points */
	if (a_Bot->Player->health >= a_Bot->Player->MaxHealth[0])
		return false;
	
	/* Otherwise, still want */
	return true;
}

/* B_WorkFindGoodies() -- Finds Goodies */
bool_t B_WorkFindGoodies(struct B_GhostBot_s* a_Bot, const size_t a_JobID)
{
#define MAXDESIRE 16
	player_t* Player;
	mobj_t* Mo, *Rover;
	mobj_t* PickupTarget;
	int32_t OwnCount, MaxNeeds, i, DesIt;
	thinker_t* TRover;
	PI_touch_t* TSpec;
	bool_t OK;
	fixed_t Dist;
	B_Desire_t Desires[MAXDESIRE];
	B_Desire_t* Want;
	
	/* If a path already exists, take that instead */
	// or if the bot is not a player.
	if (a_Bot->NumShore || !a_Bot->IsPlayer)
	{
		a_Bot->Jobs[a_JobID].Sleep = gametic + (TICRATE * 2);
		return true;
	}
	
	/* Get player and object */
	Player = a_Bot->Player;
	Mo = a_Bot->Mo;
	
	// No player or object?
	if (!Player || !Mo)
		return true;
	
	/* Init */
	PickupTarget = false;
	memset(Desires, 0, sizeof(Desires));
	DesIt = 0;
	
	/* Figure out if we need something */
	// Need Weapons?
	for (OwnCount = MaxNeeds = 0, i = 0; i < NUMWEAPONS; i++)
	{
		// Weapon is locked? (BFG in shareware, Gauntlets in Doom, etc.)
		if (!P_WeaponIsUnlocked(i))
			continue;
		
		// If we own the gun, say so
		if (Player->weaponowned[i])
			OwnCount++;
		
		// If not, say we need it
		else
		{
			MaxNeeds++;
			
			if (DesIt < MAXDESIRE)
			{
				Desires[DesIt].IsWeapon = true;
				Desires[DesIt].SpecID = i;
				DesIt++;
			}
		}
	}
	
	// Need Health?
	if (Player->health < (Player->MaxHealth[0] >> 1))
		if (DesIt < MAXDESIRE)
		{
			Desires[DesIt].IsHealth = true;
			Desires[DesIt].SpecID = (Player->MaxHealth[0] >> 1);
			DesIt++;
		}
	
	// Need Ammo?
	
	// Need Armor?
	if (Player->armorpoints < (Player->MaxArmor[0] >> 1))
		if (DesIt < MAXDESIRE)
		{
			Desires[DesIt].IsArmor = true;
			Desires[DesIt].SpecID = (Player->MaxArmor[0] >> 1);
			DesIt++;
		}
	
	/* Nothing is desired? */
	if (!DesIt)
	{
		a_Bot->Jobs[a_JobID].Sleep = gametic + (TICRATE * 3);
		return true;
	}
	
	/* Go through map objects and look for pickups */
	for (TRover = thinkercap.next; TRover != &thinkercap; TRover = TRover->next)
	{
		// Not an object?
		if (TRover->Type != PTT_MOBJ)
			continue;
		
		// Get Object
		Rover = TRover;
		
		// Not pickupable?
		if (!(Rover->flags & MF_SPECIAL))
			continue;
		
		// Cannot fit where this object is
		if (Rover->subsector->sector->ceilingheight - Rover->subsector->sector->floorheight < a_Bot->Mo->height)
			continue;
		
		// Go through touch specials for this thing
		for (TSpec = NULL, i = 0; i < g_RMODNumTouchSpecials; i++)
		{
			// Wrong sprite?
			if (g_RMODTouchSpecials[i]->ActSpriteID != Rover->state->SpriteID)
				continue;
			
			// Suppose it is valid
			TSpec = g_RMODTouchSpecials[i];
		}
		
		// Not a known special
		if (!TSpec)
			continue;
		
		// Go through desireables and determine which desire meets this need
		for (i = 0; i < DesIt; i++)
		{
			OK = false;
			
			// Matching weapon?
			if (Desires[i].IsWeapon)
				if (TSpec->ActGiveWeapon >= 0 &&
						TSpec->ActGiveWeapon <= NUMWEAPONS)
					if (Desires[i].SpecID == TSpec->ActGiveWeapon)
						OK = true;
			
			// Matching Health
			if (Desires[i].IsHealth)
				if (TSpec->HealthAmount > 0)
				{
					OK = true;
					Desires[i].SpecID = TSpec->HealthAmount;
				}
			
			// Matching armor?
			if (Desires[i].IsArmor)
				if (TSpec->ArmorAmount > 0)
				{
					OK = true;
					Desires[i].SpecID = TSpec->ArmorAmount;
				}
			
			// Object is fine?
			if (OK)
			{
				Dist = P_AproxDistance(Mo->x - Rover->x, Mo->y - Rover->y);
				
				// If no object set, use that here or if it is close
				if (!Desires[i].Thing || Dist < Desires[i].Dist)
				{
					Desires[i].Thing = Rover;
					Desires[i].Dist = Dist;
				}
			}
		}
	}
	
	/* Find object to choose, the closest thing to the player */
	PickupTarget = NULL;
	Want = NULL;
	for (i = 0; i < DesIt; i++)
	{
		// Bad desire?
		if (!Desires[i].Thing)
			continue;
		
		// Closer?
		if (!PickupTarget || (PickupTarget && Desires[i].Dist < Dist))
		{
			PickupTarget = Desires[i].Thing;
			Dist = Desires[i].Dist;
			Want = &Desires[i];
		}
	}
	
	/* Move to target if any */
	if (PickupTarget)
	{
		CONL_PrintF("Bot wants to pickup %s (%i,%i) -> (%i, %i).\n",
				PickupTarget->info->RClassName,
				Mo->x >> 16, Mo->y >> 16,
				PickupTarget->x >> 16, PickupTarget->y >> 16
			);
		
		// Determination if we still like this
		a_Bot->DesireMo = PickupTarget;
		
		// Weapon
		if (Want->IsWeapon)
		{
			a_Bot->DesireType = Want->SpecID;
			a_Bot->ConfirmDesireF = BS_GHOST_CDF_Weapon;
		}
		
		// Armor
		else if (Want->IsArmor)
		{
			a_Bot->DesireType = Want->SpecID;
			a_Bot->ConfirmDesireF = BS_GHOST_CDF_Armor;
		}
		
		// Health
		else if (Want->IsHealth)
		{
			a_Bot->DesireType = Want->SpecID;
			a_Bot->ConfirmDesireF = BS_GHOST_CDF_Health;
		}
		
		// Move to destination
		if (B_ShorePath(a_Bot, Mo->x, Mo->y, PickupTarget->x, PickupTarget->y))
			B_ShoreApprove(a_Bot);
	}
	
	/* Done with job, probably */
	a_Bot->Jobs[a_JobID].Sleep = gametic + (TICRATE * 4);
	return true;
#undef MAXDESIRE
}

/*****************************************************************************/

/* B_WorkRandomNav() -- Random navigation */
bool_t B_WorkRandomNav(struct B_GhostBot_s* a_GhostBot, const size_t a_JobID)
{
	B_GhostNode_t* ThisNode;
	B_GhostNode_t* TargetNode;
	int32_t lox, loy, i;
	
	/* Check */
	if (!a_GhostBot)
		return false;
	
	/* Find node in random direction, and move to it */
	while (!a_GhostBot->RoamX && !a_GhostBot->RoamY)
	{
		a_GhostBot->RoamX = B_Random(a_GhostBot) % 3;
		a_GhostBot->RoamY = B_Random(a_GhostBot) % 3;
		a_GhostBot->RoamX -= 1;
		a_GhostBot->RoamY -= 1;
	}
	
	/* Get link operation */
	lox = a_GhostBot->RoamX + 1;
	loy = a_GhostBot->RoamY + 1;
	
	/* See if there is a target there */
	// Get current node
	ThisNode = (B_GhostNode_t*)a_GhostBot->AtNode;
	
	// No current node?
	if (!ThisNode)
		return true;
	
	// Get node to try to move to
	TargetNode = ThisNode->Links[lox][loy].Node;
	
	// Try traversing to that node there
	if (TargetNode)
		if (!B_NodeNtoN(a_GhostBot, ThisNode, TargetNode, false))
			TargetNode = NULL;
	
	// No node there? Or we are at that node
	if (!TargetNode || TargetNode == a_GhostBot->AtNode)
	{
		// Increase X pos
		a_GhostBot->RoamX++;
		
		// Increase Y pos
		if (a_GhostBot->RoamX > 1)
		{
			a_GhostBot->RoamX = -1;
			a_GhostBot->RoamY++;
			
			if (a_GhostBot->RoamY > 1)
				a_GhostBot->RoamY = -1;
		}
		
		// Hit zero zero?
		if (!a_GhostBot->RoamX && !a_GhostBot->RoamY)
			a_GhostBot->RoamX++;
		
		// Keep this job
		return true;
	}
	
	/* Set Destination There */
	for (i = 0; i < MAXBOTTARGETS; i++)
		if (!a_GhostBot->Targets[i].IsSet)
		{
			a_GhostBot->Targets[i].IsSet = true;
			a_GhostBot->Targets[i].MoveTarget = true;
			a_GhostBot->Targets[i].ExpireTic = gametic + (TICRATE >> 1);
			a_GhostBot->Targets[i].Priority = 25;
			a_GhostBot->Targets[i].x = TargetNode->x;
			a_GhostBot->Targets[i].y = TargetNode->y;
			break;
		}
	
	/* Sleep Job */
	a_GhostBot->Jobs[a_JobID].Sleep = gametic + (TICRATE >> 1);
	
	/* Keep random navigation */
	return true;
}

/*****************************************************************************/

/* B_WorkShootStuff() -- Shoot Nearby Stuff */
bool_t B_WorkShootStuff(struct B_GhostBot_s* a_GhostBot, const size_t a_JobID)
{
#define CLOSEMOS 8
	int32_t s, i, m, BigTarg;
	sector_t* CurSec;
	mobj_t* Mo;
	mobj_t* ListMos[CLOSEMOS];
	int slope;
	INFO_BotObjMetric_t GunMetric;
	fixed_t Mod;
	
	/* Sleep Job */
	a_GhostBot->Jobs[a_JobID].Sleep = gametic + (TICRATE >> 1);
	
	/* Clear object list */
	memset(ListMos, 0, sizeof(ListMos));
	m = 0;
	
	/* Get metric of current gun */
	// Player
	if (a_GhostBot->IsPlayer)
		GunMetric = a_GhostBot->Player->weaponinfo[a_GhostBot->Player->readyweapon]->BotMetric;
	
	// Monster
	else
	{
		// Only Melee Attack
		if (!a_GhostBot->Mo->info->missilestate && a_GhostBot->Mo->info->meleestate)
			GunMetric = INFOBM_WEAPONMELEE;
		
		// Other kinds of attack
		else
			GunMetric = 0;
	}
	
	/* Go through adjacent sectors */
	// Get current sector
	s = a_GhostBot->Mo->subsector->sector - sectors;
	
	// Start Roving
	for (i = 0; i < l_BNumAdj[s]; i++)
	{
		// Get Current sector here
		CurSec = l_BAdj[s][i];
		
		// Failed?
		if (!CurSec)
			break;
		
		// Go through all things in the chains
		for (Mo = CurSec->thinglist; Mo; Mo = Mo->snext)
		{
			// Object is ourself!
			if (Mo == a_GhostBot->Mo)
				continue;
			
			// Object is missing some flags?
			if (!(Mo->flags & MF_SHOOTABLE))
				continue;
			
			// Object has some flags?
			if (Mo->flags & MF_CORPSE)
				continue;
			
			// Object is dead?
			if (Mo->health <= 0)
				continue;
			
			// Object on same team
			if (P_MobjOnSameTeam(a_GhostBot->Mo, Mo))
				continue;
			
			// Object is not seen?
			if (!P_CheckSight(a_GhostBot->Mo, Mo))
				continue;
			
			// See if autoaim acquires a friendly target, but do not perform
			// this check if shoot allies is enabled.
			if (!(a_GhostBot->BotTemplate.Flags & BGBF_SHOOTALLIES))
			{
				slope = P_AimLineAttack(a_GhostBot->Mo, a_GhostBot->Mo->angle, MISSILERANGE, NULL);
			
				if (linetarget && linetarget != a_GhostBot->Mo && P_MobjOnSameTeam(a_GhostBot->Mo, linetarget))
					continue;
			}
			
			// Set in chain
			if (m < CLOSEMOS)
				ListMos[m++] = Mo;
			
			// Close object overflow?
			if (m >= CLOSEMOS)
				break;
		}
		
		// Close object overflow?
		if (m >= CLOSEMOS)
			break;
	}
	
	/* Find most important object */
	BigTarg = -1;
	
	/* Go through objects and update pre-existings */
	for (i = 0; i < MAXBOTTARGETS; i++)
		if (a_GhostBot->Targets[i].IsSet)
			for (s = 0; s < m; s++)
				if (ListMos[s] && a_GhostBot->Targets[i].Key == (uintptr_t)ListMos[s])
				{
					// Update This
					a_GhostBot->Targets[i].ExpireTic += (TICRATE >> 1);
					a_GhostBot->Targets[i].Priority += 10;	// Make it a bit more important
					a_GhostBot->Targets[i].x = ListMos[s]->x;
					a_GhostBot->Targets[i].y = ListMos[s]->y;
					a_GhostBot->Targets[i].Key = (uintptr_t)ListMos[s];
					
					// Based on metric, possibly move target location
					if (GunMetric == INFOBM_SPRAYPLASMA)
					{
						// Modify X Value
						Mod = B_Random(a_GhostBot) - 128;
						Mod = FixedMul(Mod << FRACBITS, INT32_C(1024));
						Mod = FixedMul(Mod, FIXEDT_C(64));
						a_GhostBot->Targets[i].x += Mod;
						
						// Modify Y Value
						Mod = B_Random(a_GhostBot) - 128;
						Mod = FixedMul(Mod << FRACBITS, INT32_C(1024));
						Mod = FixedMul(Mod, FIXEDT_C(48));
						a_GhostBot->Targets[i].y += Mod;
					}
					
					// Force Attacking
					if (a_GhostBot->IsPlayer)
						if (a_GhostBot->Player->pendingweapon < 0 || a_GhostBot->Player->pendingweapon >= NUMWEAPONS)
							a_GhostBot->TicCmdPtr->Std.buttons |= BT_ATTACK;
					
					// Clear from current
					ListMos[s] = NULL;
					
					// Big time target?
					if (BigTarg == -1 ||
						a_GhostBot->Targets[i].Priority >
							a_GhostBot->Targets[BigTarg].Priority)
						BigTarg = i;
				}
	
	/* Put objects into the target list */
	for (s = 0, i = 0; s < m && i < MAXBOTTARGETS; i++)
		if (ListMos[s])
			if (!a_GhostBot->Targets[i].IsSet)
			{
				// Setup
				a_GhostBot->Targets[i].IsSet = true;
				a_GhostBot->Targets[i].MoveTarget = false;
				a_GhostBot->Targets[i].ExpireTic = gametic + (TICRATE >> 1);
				a_GhostBot->Targets[i].Priority = (-ListMos[s]->health) + 100;
				a_GhostBot->Targets[i].x = ListMos[s]->x;
				a_GhostBot->Targets[i].y = ListMos[s]->y;
				a_GhostBot->Targets[i].Key = (uintptr_t)ListMos[s];
				
				// Force Attacking
				if (a_GhostBot->IsPlayer)
					if (a_GhostBot->Player->pendingweapon < 0 || a_GhostBot->Player->pendingweapon >= NUMWEAPONS)
						a_GhostBot->TicCmdPtr->Std.buttons |= BT_ATTACK;
				
				// Big time target?
				if (BigTarg == -1 ||
					a_GhostBot->Targets[i].Priority >
						a_GhostBot->Targets[BigTarg].Priority)
					BigTarg = i;
				
				// Update List
				s++;
				break;
			}
	
	/* Big time target? Move to it! */
	if (BigTarg != -1)
		switch (GunMetric)
		{
				// Melee Attack
			case INFOBM_WEAPONMELEE:
				// Make a movement target at the target spot
				for (i = 0; i < MAXBOTTARGETS; i++)
					if (!a_GhostBot->Targets[i].IsSet)
					{
						// Clone directly
						a_GhostBot->Targets[i] = a_GhostBot->Targets[BigTarg];
						
						// Increase Priority and make it a move target
						a_GhostBot->Targets[i].MoveTarget = true;
						a_GhostBot->Targets[i].Priority = (a_GhostBot->Targets[i].Priority / 2) + 10;
					}
				break;
				
				// No Metric
			default:
				break;
		}

//static sector_t* (*l_BAdj)[MAXBGADJDEPTH] = NULL;	// Adjacent sector list
//static size_t* l_BNumAdj = NULL;				// Number of adjacent sectors
//static size_t l_BNumSecs = 0;					// Number of sectors

	return true;
#undef CLOSEMOS
}

/*****************************************************************************/

/* B_WorkGunControl() -- Determine weapon changing */
bool_t B_WorkGunControl(struct B_GhostBot_s* a_GhostBot, const size_t a_JobID)
{
#define MAXGUNSWITCHERS 32
	int32_t i, b;
	int32_t SwitchChance[MAXGUNSWITCHERS];
	fixed_t AmmoCount;
	PI_ammoid_t AmmoType;
	PI_wepid_t FavoriteGun;
	
	/* Sleep Job */
	a_GhostBot->Jobs[a_JobID].Sleep = gametic + (TICRATE * 10);
	
	// If not a player, then don't mess with our guns
	if (!a_GhostBot->IsPlayer)
		return true;
	
	/* Get Our Favorite Gun */
	FavoriteGun = P_PlayerBestWeapon(a_GhostBot->Player);
	
	/* Determine guns to switch to */
	for (i = 0; i < MAXGUNSWITCHERS && i < NUMWEAPONS; i++)
	{
		// Not owned?
		if (!a_GhostBot->Player->weaponowned[i])
		{
			SwitchChance[i] = -1000;
			continue;
		}
		
		// Base chance is weapon power
		SwitchChance[i] = a_GhostBot->Player->weaponinfo[i]->SwitchOrder;
		
		// Get ammo amount
		AmmoType = a_GhostBot->Player->weaponinfo[i]->ammo;
		
		if (AmmoType >= 0 && AmmoType < NUMAMMO)
			AmmoCount = FixedDiv(a_GhostBot->Player->ammo[AmmoType] << FRACBITS,
					a_GhostBot->Player->maxammo[AmmoType] << FRACBITS);
		else
			AmmoCount = 32768;
		
		// Modified by the amount of ammo the weapon holds
		SwitchChance[i] = FixedMul(SwitchChance[i] << FRACBITS, AmmoCount);
		
		// Favorite Gun Boost
		if (i == FavoriteGun)
			SwitchChance[i] += 350;
	}
	
	/* Find gun to switch to */
	// Most wanted to switch to
	for (i = 0, b = 0; i < MAXGUNSWITCHERS && i < NUMWEAPONS; i++)
		if (SwitchChance[i] > SwitchChance[b])
			b = i;
		
	/* Not using this gun? */
	// Switch to that gun
	if (a_GhostBot->Player->readyweapon != b && a_GhostBot->Player->pendingweapon != b)
	{
		a_GhostBot->TicCmdPtr->Std.buttons |= BT_CHANGE;
		D_TicCmdFillWeapon(a_GhostBot->TicCmdPtr, b);
	}
	
	/* Always keep this job */
	return true;
#undef MAXGUNSWITCHERS
}
