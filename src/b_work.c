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
#include "p_inter.h"

/****************
*** FUNCTIONS ***
****************/

/*****************************************************************************/

/* B_CalcMoVsMo() -- Calculates Object vs Object */
int32_t B_CalcMoVsMo(mobj_t* const a_Red, mobj_t* const a_Blue, int32_t* const a_RedCount, int32_t* const a_BlueCount)
{
	player_t* RedPlayer, *BluePlayer;
	int32_t RedC, BlueC;
	PI_wep_t* RedWep, *BlueWep;
	int32_t* PlayC, *MonC;
	int32_t r, b;
	mobj_t* MonMo;
	
	/* Check */
	if (!a_Red || !a_Blue)
	{
		if (a_RedCount)
			*a_RedCount = 0;
		if (a_BlueCount)
			*a_BlueCount = 0;
		return 0;
	}
	
	/* Get players, if any */
	RedPlayer = BluePlayer = NULL;
	if (P_MobjIsPlayer(a_Red))
		RedPlayer = a_Red->player;
	if (P_MobjIsPlayer(a_Blue))
		BluePlayer = a_Blue->player;
	
	/* Initialize Counts */
	RedC = BlueC = 0;
	
	/* Player vs Player */
	if (RedPlayer && BluePlayer)
	{
		// Get current weapons
		RedWep = RedPlayer->weaponinfo[RedPlayer->readyweapon];
		BlueWep = BluePlayer->weaponinfo[BluePlayer->readyweapon];
		
		// Base is the switch order of the weapon
			// But switch orders are in the hundreds, so make them count less
		RedC = RedWep->SwitchOrder / 10;
		BlueC = BlueWep->SwitchOrder / 10;
		
		// If the weapon is a super weapon, give 25% Bonus
		if (RedWep->WeaponFlags & WF_SUPERWEAPON)
			RedC += RedC >> 2;
		if (BlueWep->WeaponFlags & WF_SUPERWEAPON)
			BlueC += BlueC >> 2;
		
		// However, if the weapon is melee, cut in half, unless both players
		// have a melee weapon
		if ((RedWep->WeaponFlags & WF_ISMELEE) ^ (BlueWep->WeaponFlags & WF_ISMELEE))
		{
			if (RedWep->WeaponFlags & WF_ISMELEE)
				RedC >>= 1;
			if (BlueWep->WeaponFlags & WF_ISMELEE)
				BlueC >>= 1;
		}
		
		// Calculate the armor, not being that effective
		r = RedPlayer->armorpoints >> 1;
		b = BluePlayer->armorpoints >> 1;
		
		// Blue armor gives a 50% bonus
		if (RedPlayer->armortype >= 2)
			r += (r >> 1);
		if (BluePlayer->armortype >= 2)
			b += (b >> 1);
		
		// Add in armor points
		RedC += r;
		BlueC += b;
		
		// Add in health
		RedC += RedPlayer->health;
		BlueC += BluePlayer->health;
		
		// If a player has more health, give that player the difference
		if (RedPlayer->health > BluePlayer->health)
			RedC += RedPlayer->health - BluePlayer->health;
		else
			BlueC += BluePlayer->health - RedPlayer->health;
	}
	
	/* Player vs Monster / Monster vs Player */
	else if ((RedPlayer && !BluePlayer) || (!RedPlayer && BluePlayer))
	{
		// Calculates are the same, so to reduce code bloat
			// Red side
		if (RedPlayer)
		{
			PlayC = &RedC;
			MonC = &BlueC;
			MonMo = a_Blue;
			RedPlayer = RedPlayer;
		}
		
			// Blue side
		else
		{
			PlayC = &BlueC;
			MonC = &RedC;
			MonMo = a_Red;
			RedPlayer = BluePlayer;
		}
		
		// Players get a bonus of 50
		*PlayC += 50;
		
		// Monsters get their health by 10s
		*MonC = MonMo->health / 10;
		
		// Players get their health by 5s
		*PlayC += RedPlayer->health / 5;
		
		// And they get armor bonus by 10s
		*PlayC += RedPlayer->armorpoints / 10;
	}
	
	/* Monster vs Monster */
	else
	{
	}
	
	/* Return totals */
	if (a_RedCount)
		*a_RedCount = RedC;
	if (a_BlueCount)
		*a_BlueCount = BlueC;
	return RedC + BlueC;
}

/* B_FindGOASpot() -- Finds a GOA */
static B_BotGOA_t* B_FindGOASpot(B_Bot_t* a_Bot, const B_BotGOAType_t a_Type, const fixed_t a_Distance, const int32_t a_Priority, thinker_t* const a_Thinker)
{
	B_BotGOA_t* Free;
	B_BotGOA_t* Worst;
	int32_t i;
	
	/* Go through all */
	Free = Worst = NULL;
	for (i = 0; i < MAXBOTGOA; i++)
	{
		// Free?
		if (a_Bot->GOA[i].Type == BBGOAT_NULL)
			Free = &a_Bot->GOA[i];
		
		// Worse?
		else
		{
			if (Worst)
			{
				if (!a_Priority)
				{
					if (a_Bot->GOA[i].Dist >= Worst->Dist)
						Worst = &a_Bot->GOA[i];
				}
				
				else
				{
					if (a_Bot->GOA[i].Priority <= Worst->Priority)
						Worst = &a_Bot->GOA[i];
				}
			}
			
			else if (a_Type == a_Bot->GOA[i].Type)
			{
				if (!a_Priority)
				{
					if (a_Distance <= a_Bot->GOA[i].Dist)
						Worst = &a_Bot->GOA[i];
				}
				
				else
				{
					if (a_Priority >= a_Bot->GOA[i].Priority)
						Worst = &a_Bot->GOA[i];
				}
			}
		}
		
		// This thinker?
		if (a_Thinker)
			if (a_Bot->GOA[i].Thinker == a_Thinker)
				return &a_Bot->GOA[i];
	}
	
	/* Return free if any, if not, use worst */
	if (Free)
		return Free;
	if (Worst)
		return Worst;
	
	// Fail case, just in case?
	return &a_Bot->GOA[(MAXBOTGOA - 1) - a_Type];
}

/* B_WorkGOAAct() -- Act upon the GOA table */
bool_t B_WorkGOAAct(B_Bot_t* a_Bot, const size_t a_JobID)
{
	int32_t i;
	B_BotGOA_t* GOA, *ShoreHere, *ShootAt, *BarrelTarg;
	B_BotTarget_t* Targ, *FreeTarg;
	mobj_t* Mo;
	
	/* If not in a level, don't bother */
	if (gamestate != GS_LEVEL)
		return true;
		
	/* Init */
	ShoreHere = ShootAt = BarrelTarg = NULL;
	Targ = FreeTarg = NULL;
	
	/* Go through GOA list */
	for (i = 0; i < MAXBOTGOA; i++)
	{
		GOA = &a_Bot->GOA[i];
		
		// No thinker? Not Important?
		if (!GOA->Thinker || GOA->Priority <= 0 || (GOA->ExpireSeen && gametic >= GOA->ExpireSeen))
		{
			memset(GOA, 0, sizeof(*GOA));
			continue;
		}
		
		// Remove gone things
		if (GOA->Type == BBGOAT_BARREL || GOA->Type == BBGOAT_ENEMY)
		{
			Mo = (mobj_t*)GOA->Thinker;
			
			// If this thing is now dead, screw it
			if (Mo->health <= 0 || (Mo->flags & MF_CORPSE))
			{
				memset(GOA, 0, sizeof(*GOA));
				continue;
			}
		}
		
		// Possibly shoot at this thing
		if (GOA->Type == BBGOAT_ENEMY)
		{
			// If we see it, shoot at it
			if (GOA->Data.Mo.Seen || P_CheckSight(a_Bot->Mo, (mobj_t*)GOA->Thinker))
				if (!ShootAt || GOA->Dist < ShootAt->Dist)
					ShootAt = GOA;
		}
		
		// Failed to shore, so it can never become an active GOA
		if (gametic < GOA->ShoreFailWait)
			continue;
		
		// If no action is assigned, then set it
		// If one is assigned but the type is a mismatch
		// Otherwise, choose the more important one
		if (!a_Bot->ActGOA[GOA->Type] ||
				a_Bot->ActGOA[GOA->Type]->Type != GOA->Type ||
				(GOA->Priority > a_Bot->ActGOA[GOA->Type]->Priority &&
					GOA->Dist <= a_Bot->ActGOA[GOA->Type]->Dist))
		{
			if (GOA->Type == BBGOAT_ENEMY)
				CONL_PrintF("Considering %s (%p)\n", ((mobj_t*)GOA->Thinker)->info->RClassName, (mobj_t*)GOA->Thinker);
			a_Bot->ActGOA[GOA->Type] = GOA;
		}
	}
	
	/* Perform actions for each */
	for (i = 0; i < NUMBOTGOATYPES; i++)
	{
		// Get Ref
		GOA = a_Bot->ActGOA[i];
		
		// Missing?
		if (!GOA)
			continue;
		
		// Ignore NULL and Barrels
		if (i == BBGOAT_BARREL || i == BBGOAT_NULL)
			continue;
		
		// Lost Priority?
		if (GOA->Priority <= 0 || GOA->Ignore)
		{
			a_Bot->ActGOA[i] = NULL;
			continue;
		}
		
		// Shore to this area? Actual shoring is done later
		if (!GOA->ShoreFailWait || gametic >= GOA->ShoreFailWait)
			if (GOA->Priority > 0 && GOA->XRef && GOA->YRef)
				if (!ShoreHere || (GOA->Priority > ShoreHere->Priority && GOA->Dist <= ShoreHere->Dist))
				{
					// If an ally, don't bother them if they are fine
					if (i == BBGOAT_ALLY)
						continue;
					
					ShoreHere = GOA;
				}
	}
	
	/* Shoreing to an area */
	// But not if there is an existing shore, however, unless the priority is
	// really high!
	if (ShoreHere && (a_Bot->ShoreIt >= a_Bot->NumShore || (a_Bot->GOAShorePri && ShoreHere->Priority >= (a_Bot->GOAShorePri << 1))))
	{
		CONL_PrintF("Shore to %s (%p) + %u/%u\n", ((mobj_t*)ShoreHere->Thinker)->info->RClassName, ShoreHere->Thinker, (unsigned)ShoreHere->ShoreFailWait, (unsigned)gametic);
		
		// Build Path to target
		if (B_ShorePath(a_Bot, a_Bot->Mo->x, a_Bot->Mo->y, *ShoreHere->XRef, *ShoreHere->YRef))
		{
			CONL_PrintF("OK!\n");
			B_ShoreApprove(a_Bot);
			a_Bot->GOAShorePri = ShoreHere->Priority;
		}
		
		// Failed?
		else
		{
			CONL_PrintF("Fail!\n");
			// Wait 60 seconds for monsters, 10 for players before trying again
			if (!P_MobjIsPlayer((mobj_t*)ShoreHere->Thinker))
				ShoreHere->ShoreFailWait = gametic + (TICRATE * 60);
			else
				ShoreHere->ShoreFailWait = gametic + (TICRATE * 10);
			
			// Clear from active, if it is
			for (i = 0; i < NUMBOTGOATYPES; i++)
				if (ShoreHere == a_Bot->ActGOA[i])
					a_Bot->ActGOA[i] = NULL;
		}
	}
	
	/* Shooting at thing? */
	if (ShootAt)
	{
		// If there exists a barrel, try shooting at it if it is near...
		if (a_Bot->ActGOA[BBGOAT_BARREL])
			for (i = 0; i < NUMBOTGOATYPES; i++)
				if (a_Bot->GOA[i].Type == BBGOAT_BARREL)
				{
					// See if it is in sight
					if (!P_CheckSight(a_Bot->Mo, (mobj_t*)a_Bot->GOA[i].Thinker))
						continue;
					
					// Has to be within 64 units from enemy
					if (P_AproxDistance(*a_Bot->GOA[i].XRef - *ShootAt->XRef, *a_Bot->GOA[i].YRef - *ShootAt->YRef) >= FIXEDT_C(64))
						continue;
					
					// Cool, set it
					BarrelTarg = &a_Bot->GOA[i];
				}
		
		// Go through targets and replace existing shoot target, if any
		for (i = 0; i < MAXBOTTARGETS; i++)
		{
			Targ = &a_Bot->Targets[i];
			
			// Target not set? or this is a target previously acquired?
			if ((!FreeTarg && !Targ->IsSet) || (Targ->IsSet && Targ->Key == GOASHOOTKEY))
				FreeTarg = Targ; 
		}
		
		// If free target was found
		if (FreeTarg)
		{
			FreeTarg->IsSet = true;
			FreeTarg->MoveTarget = false;
			FreeTarg->ExpireTic = gametic + TICRATE;
			FreeTarg->Priority = ShootAt->Priority;
			FreeTarg->Key = GOASHOOTKEY;
			
			// Shooting at barrel?
			if (BarrelTarg)
			{
				FreeTarg->x = *BarrelTarg->XRef;
				FreeTarg->y = *BarrelTarg->YRef;
			}
			
			// Shooting at object
			else
			{
				FreeTarg->x = *ShootAt->XRef;
				FreeTarg->y = *ShootAt->YRef;
			}
		}
	}
	
	// Otherwise, do not shoot at anything, and clear existing target if any
	else
	{
		for (i = 0; i < MAXBOTTARGETS; i++)
			if (a_Bot->Targets[i].IsSet && a_Bot->Targets[i].Key == GOASHOOTKEY)
				memset(&a_Bot->Targets[i], 0, sizeof(a_Bot->Targets[i]));
	}
	
	/* Always keep job */
	a_Bot->Jobs[a_JobID].Sleep = gametic + (TICRATE >> 1);
	return true;
}

/* B_WorkGOAUpdate() -- Updates the GOA table */
bool_t B_WorkGOAUpdate(B_Bot_t* a_Bot, const size_t a_JobID)
{
	thinker_t* Thinker;
	mobj_t* Mo;
	int32_t Priority, VsPro;
	B_BotGOAType_t Type;
	fixed_t Dist;
	B_BotGOA_t* New;
	
	/* If not in a level, don't bother */
	if (gamestate != GS_LEVEL)
		return true;
	
	/* Go through the thinker table */
	for (Thinker = thinkercap.next; Thinker != &thinkercap; Thinker = Thinker->next)
	{
		// Ignore our own thinker
		if (Thinker == (thinker_t*)a_Bot->Mo)
			continue;
		
		// Init
		Type = BBGOAT_NULL;
		Priority = VsPro = 0;
		
		// Which object type is this?
		switch (Thinker->Type)
		{
				// Map Object
			case PTT_MOBJ:
				Mo = (mobj_t*)Thinker;
				
				// Can be picked up
				if (Mo->flags & MF_SPECIAL)
				{
					Type = BBGOAT_PICKUP;
				}
				
				// Can be shot
				else if (Mo->flags & MF_SHOOTABLE)
				{
					// Dead?
					if (Mo->health <= 0 || (Mo->flags & MF_CORPSE))
						continue;
					
					// A barrel?
					if (Mo->info->BotMetric == INFOBM_BARREL)
					{
						Type = BBGOAT_BARREL;
					}
					
					// Everything else
					else
					{
						// On same team?
						if (P_MobjOnSameTeam(a_Bot->Mo, Mo))
						{
							Type = BBGOAT_ALLY;
							Priority = 50;
						}
			
						// Enemy Team
						else
						{
							Type = BBGOAT_ENEMY;
							Priority = VsPro = 0;
							B_CalcMoVsMo(a_Bot->Mo, Mo, &Priority, &VsPro);
						}
					}
				}
				
				// Not Valid? Low Priority?
				if (Type == BBGOAT_NULL || Priority <= 0)
					continue;
				
				// Distance to object
				Dist = P_AproxDistance(Mo->x - a_Bot->Mo->x, Mo->y - a_Bot->Mo->y);
				
				// If not a player, ignore far away objects
				if (!P_MobjIsPlayer(Mo))
					if (Dist >= FIXEDT_C(4096))
						continue;
				
				// Find new slot
				New = B_FindGOASpot(a_Bot, Type, Dist, 0/*Priority*/, (thinker_t*)Mo);
				
				// Failed?
				if (!New)
					continue;
				
				// Debug
				if (New->Thinker != (thinker_t*)Mo)
				{
					memset(New, 0, sizeof(*New));
					
					New->FirstSeen = gametic;
					New->ExpireSeen = New->FirstSeen + (TICRATE * 15);
				}
				else
				{
					// Only update the timer if not ignoring
					if (!New->Ignore)
						New->ExpireSeen = gametic + (TICRATE * 15);
				}
				
				New->Type = Type;
				New->Thinker = (thinker_t*)Mo;
				New->Priority = Priority;
				New->Dist = Dist;
				New->Data.Mo.Seen = P_CheckSight(a_Bot->Mo, Mo);
				New->XRef = &Mo->x;
				New->YRef = &Mo->y;
				
				if (Type == BBGOAT_ENEMY)
				{
					New->Data.Mo.MyPower = Priority;
					New->Data.Mo.TargetPower = VsPro;
				}
				
				// If unseen, cut priority by 1/4
				if (!New->Data.Mo.Seen)
					New->Priority -= (New->Priority >> 2);
				break;
			
			default:
				break;
		}
	}
	
	/* Always keep job */
	a_Bot->Jobs[a_JobID].Sleep = gametic + TICRATE;
	return true;
}

/*****************************************************************************/

/* B_WorkShoreMove() -- Utilize shore movement */
bool_t B_WorkShoreMove(B_Bot_t* a_Bot, const size_t a_JobID)
{
	B_BotTarget_t* FFree, *ShoreTarg;
	B_ShoreNode_t* This;
	B_Node_t* Node;
	mobj_t* Mo;
	int32_t i;
	fixed_t Dist;
	B_PTPData_t PDat;
	
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
	
	/* See if the node we are chasing at can be traversed to */
	memset(&PDat, 0, sizeof(PDat));
	if (!B_NodePtoP(a_Bot, &PDat, Mo->x, Mo->y, This->Pos[0], This->Pos[1]))
	{
		// It is not, so clear the path we made
		B_ShoreClear(a_Bot, true);
		a_Bot->Jobs[a_JobID].Sleep = gametic + (TICRATE);
		return true;
	}
	
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
	a_Bot->Jobs[a_JobID].Sleep = gametic + 3;
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
static bool_t BS_GHOST_CDF_Weapon(B_Bot_t* a_Bot)
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
static bool_t BS_GHOST_CDF_Armor(B_Bot_t* a_Bot)
{
	/* Enough points */
	if (a_Bot->Player->armorpoints >= a_Bot->Player->MaxArmor[0])
		return false;
	
	/* Otherwise, still want */
	return true;
}

/* BS_GHOST_CDF_Health() -- Still wants Health */
static bool_t BS_GHOST_CDF_Health(B_Bot_t* a_Bot)
{
	/* Enough points */
	if (a_Bot->Player->health >= a_Bot->Player->MaxHealth[0])
		return false;
	
	/* Otherwise, still want */
	return true;
}

/* B_WorkFindGoodies() -- Finds Goodies */
bool_t B_WorkFindGoodies(B_Bot_t* a_Bot, const size_t a_JobID)
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
bool_t B_WorkRandomNav(B_Bot_t* a_Bot, const size_t a_JobID)
{
	B_Node_t* ThisNode;
	B_Node_t* TargetNode;
	int32_t lox, loy, i;
	
	/* Check */
	if (!a_Bot)
		return false;
	
	/* Find node in random direction, and move to it */
	while (!a_Bot->RoamX && !a_Bot->RoamY)
	{
		a_Bot->RoamX = B_Random(a_Bot) % 3;
		a_Bot->RoamY = B_Random(a_Bot) % 3;
		a_Bot->RoamX -= 1;
		a_Bot->RoamY -= 1;
	}
	
	/* Get link operation */
	lox = a_Bot->RoamX + 1;
	loy = a_Bot->RoamY + 1;
	
	/* See if there is a target there */
	// Get current node
	ThisNode = (B_Node_t*)a_Bot->AtNode;
	
	// No current node?
	if (!ThisNode)
		return true;
	
	// Get node to try to move to
	TargetNode = ThisNode->Links[lox][loy].Node;
	
	// Try traversing to that node there
	if (TargetNode)
		if (!B_NodeNtoN(a_Bot, ThisNode, TargetNode, false))
			TargetNode = NULL;
	
	// No node there? Or we are at that node
	if (!TargetNode || TargetNode == a_Bot->AtNode)
	{
		// Increase X pos
		a_Bot->RoamX++;
		
		// Increase Y pos
		if (a_Bot->RoamX > 1)
		{
			a_Bot->RoamX = -1;
			a_Bot->RoamY++;
			
			if (a_Bot->RoamY > 1)
				a_Bot->RoamY = -1;
		}
		
		// Hit zero zero?
		if (!a_Bot->RoamX && !a_Bot->RoamY)
			a_Bot->RoamX++;
		
		// Keep this job
		return true;
	}
	
	/* Set Destination There */
	for (i = 0; i < MAXBOTTARGETS; i++)
		if (!a_Bot->Targets[i].IsSet)
		{
			a_Bot->Targets[i].IsSet = true;
			a_Bot->Targets[i].MoveTarget = true;
			a_Bot->Targets[i].ExpireTic = gametic + (TICRATE >> 1);
			a_Bot->Targets[i].Priority = 25;
			a_Bot->Targets[i].x = TargetNode->x;
			a_Bot->Targets[i].y = TargetNode->y;
			break;
		}
	
	/* Sleep Job */
	a_Bot->Jobs[a_JobID].Sleep = gametic + (TICRATE >> 1);
	
	/* Keep random navigation */
	return true;
}

/*****************************************************************************/

/* B_WorkShootStuff() -- Shoot Nearby Stuff */
bool_t B_WorkShootStuff(B_Bot_t* a_Bot, const size_t a_JobID)
{
#define CLOSEMOS 8
	int32_t s, i, m, BigTarg;
	sector_t* CurSec;
	mobj_t* Mo;
	mobj_t* ListMos[CLOSEMOS];
	INFO_BotObjMetric_t GunMetric;
	fixed_t Mod;
	
	/* Sleep Job */
	a_Bot->Jobs[a_JobID].Sleep = gametic + (TICRATE >> 1);
	
	/* Clear object list */
	memset(ListMos, 0, sizeof(ListMos));
	m = 0;
	
	/* Get metric of current gun */
	// Player
	if (a_Bot->IsPlayer)
		GunMetric = a_Bot->Player->weaponinfo[a_Bot->Player->readyweapon]->BotMetric;
	
	// Monster
	else
	{
		// Only Melee Attack
		if (!a_Bot->Mo->info->missilestate && a_Bot->Mo->info->meleestate)
			GunMetric = INFOBM_WEAPONMELEE;
		
		// Other kinds of attack
		else
			GunMetric = 0;
	}
	
	/* Go through adjacent sectors */
	// Get current sector
	s = a_Bot->Mo->subsector->sector - sectors;
	
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
			if (Mo == a_Bot->Mo)
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
			if (P_MobjOnSameTeam(a_Bot->Mo, Mo))
				continue;
			
			// Object is not seen?
			if (!P_CheckSight(a_Bot->Mo, Mo))
				continue;
			
			// See if autoaim acquires a friendly target, but do not perform
			// this check if shoot allies is enabled.
			if (!(a_Bot->BotTemplate.Flags & BGBF_SHOOTALLIES))
			{
				P_AimLineAttack(a_Bot->Mo, a_Bot->Mo->angle, MISSILERANGE, NULL);
			
				if (linetarget && linetarget != a_Bot->Mo && P_MobjOnSameTeam(a_Bot->Mo, linetarget))
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
		if (a_Bot->Targets[i].IsSet)
			for (s = 0; s < m; s++)
				if (ListMos[s] && a_Bot->Targets[i].Key == (uintptr_t)ListMos[s])
				{
					// Update This
					a_Bot->Targets[i].ExpireTic += (TICRATE >> 1);
					a_Bot->Targets[i].Priority += 10;	// Make it a bit more important
					a_Bot->Targets[i].x = ListMos[s]->x;
					a_Bot->Targets[i].y = ListMos[s]->y;
					a_Bot->Targets[i].Key = (uintptr_t)ListMos[s];
					
					// Based on metric, possibly move target location
					if (GunMetric == INFOBM_SPRAYPLASMA)
					{
						// Modify X Value
						Mod = B_Random(a_Bot) - 128;
						Mod = FixedMul(Mod << FRACBITS, INT32_C(1024));
						Mod = FixedMul(Mod, FIXEDT_C(64));
						a_Bot->Targets[i].x += Mod;
						
						// Modify Y Value
						Mod = B_Random(a_Bot) - 128;
						Mod = FixedMul(Mod << FRACBITS, INT32_C(1024));
						Mod = FixedMul(Mod, FIXEDT_C(48));
						a_Bot->Targets[i].y += Mod;
					}
					
					// Force Attacking
					if (a_Bot->IsPlayer)
						if (a_Bot->Player->pendingweapon < 0 || a_Bot->Player->pendingweapon >= NUMWEAPONS)
							a_Bot->TicCmdPtr->Std.buttons |= BT_ATTACK;
					
					// Clear from current
					ListMos[s] = NULL;
					
					// Big time target?
					if (BigTarg == -1 ||
						a_Bot->Targets[i].Priority >
							a_Bot->Targets[BigTarg].Priority)
						BigTarg = i;
				}
	
	/* Put objects into the target list */
	for (s = 0, i = 0; s < m && i < MAXBOTTARGETS; i++)
		if (ListMos[s])
			if (!a_Bot->Targets[i].IsSet)
			{
				// Setup
				a_Bot->Targets[i].IsSet = true;
				a_Bot->Targets[i].MoveTarget = false;
				a_Bot->Targets[i].ExpireTic = gametic + (TICRATE >> 1);
				a_Bot->Targets[i].Priority = (-ListMos[s]->health) + 100;
				a_Bot->Targets[i].x = ListMos[s]->x;
				a_Bot->Targets[i].y = ListMos[s]->y;
				a_Bot->Targets[i].Key = (uintptr_t)ListMos[s];
				
				// Force Attacking
				if (a_Bot->IsPlayer)
					if (a_Bot->Player->pendingweapon < 0 || a_Bot->Player->pendingweapon >= NUMWEAPONS)
						a_Bot->TicCmdPtr->Std.buttons |= BT_ATTACK;
				
				// Big time target?
				if (BigTarg == -1 ||
					a_Bot->Targets[i].Priority >
						a_Bot->Targets[BigTarg].Priority)
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
					if (!a_Bot->Targets[i].IsSet)
					{
						// Clone directly
						a_Bot->Targets[i] = a_Bot->Targets[BigTarg];
						
						// Increase Priority and make it a move target
						a_Bot->Targets[i].MoveTarget = true;
						a_Bot->Targets[i].Priority = (a_Bot->Targets[i].Priority / 2) + 10;
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
bool_t B_WorkGunControl(B_Bot_t* a_Bot, const size_t a_JobID)
{
#define MAXGUNSWITCHERS 32
	int32_t i, b;
	int32_t SwitchChance[MAXGUNSWITCHERS];
	fixed_t AmmoCount;
	PI_ammoid_t AmmoType;
	PI_wepid_t FavoriteGun;
	
	/* Sleep Job */
	a_Bot->Jobs[a_JobID].Sleep = gametic + (TICRATE * 10);
	
	// If not a player, then don't mess with our guns
	if (!a_Bot->IsPlayer)
		return true;
	
	/* Get Our Favorite Gun */
	FavoriteGun = P_PlayerBestWeapon(a_Bot->Player, true);
	
	/* Determine guns to switch to */
	for (i = 0; i < MAXGUNSWITCHERS && i < NUMWEAPONS; i++)
	{
		// Not owned?
		if (!a_Bot->Player->weaponowned[i])
		{
			SwitchChance[i] = -1000;
			continue;
		}
		
		// Base chance is weapon power
		SwitchChance[i] = a_Bot->Player->weaponinfo[i]->SwitchOrder;
		
		// Get ammo amount
		AmmoType = a_Bot->Player->weaponinfo[i]->ammo;
		
		if (AmmoType >= 0 && AmmoType < NUMAMMO)
			AmmoCount = FixedDiv(a_Bot->Player->ammo[AmmoType] << FRACBITS,
					a_Bot->Player->maxammo[AmmoType] << FRACBITS);
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
	if (a_Bot->Player->readyweapon != b && a_Bot->Player->pendingweapon != b)
	{
		a_Bot->TicCmdPtr->Std.buttons |= BT_CHANGE;
		D_TicCmdFillWeapon(a_Bot->TicCmdPtr, b);
	}
	
	/* Always keep this job */
	return true;
#undef MAXGUNSWITCHERS
}
