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
// DESCRIPTION: Simplistic Line Handling Code

/***************
*** INCLUDES ***
***************/

#include "p_nwline.h"
#include "r_local.h"
#include "dstrings.h"
#include "p_local.h"
#include "p_inter.h"
#include "p_demcmp.h"
#include "m_random.h"

/****************
*** CONSTANTS ***
****************/

/* P_NLFlags_t -- Flags for triggers */
typedef enum P_NLFlags_e
{
	PNLF_RETRIG			= UINT32_C(0x00000001),	// Retrigger lines
	PNLF_IGNORETAG		= UINT32_C(0x00000002),	// Does not need a tag
	PNLF_MONSTER		= UINT32_C(0x00000004),	// Monster can activate
	PNLF_BOOM			= UINT32_C(0x00000008),	// Boom Support Required
	PNLF_CLEARNOTBOOM	= UINT32_C(0x00000010),	// Clear Special when not Boom
	PNLF_ALWAYS			= UINT32_C(0x00000020),	// Always return true
} P_NLFlags_t;

/*****************
*** STRUCTURES ***
*****************/

typedef struct P_NLTrig_s
{
	uint32_t Start;								// Start of line (>=)
	uint32_t Length;							// Lines to consider (<=)
	EV_TryGenType_t TrigType;					// Trigger Type
	uint32_t Flags;								// Flags
	
	P_NLTrigFunc_t TrigFunc;					// Trigger Function
	uint32_t ArgC;								// Argument Count
	int32_t ArgV[10];							// Arguments
} P_NLTrig_t;

/****************
*** FUNCTIONS ***
****************/

/*****************************************************************************/

/* EV_VerticalDoor() -- open a door manually, no tag value */
//  1: Rebound Door
//  2: Door Sound
//  3: Door Type
//  4: Speed
//  5: Lock
bool_t EV_VerticalDoor(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV)
{
	player_t* player;
	int secnum;
	sector_t* sec;
	vldoor_t* door;
	P_PMType_t MsgType;
	const char** MsgRef;
	
	/* Check */
	if (!a_Object)
		return false;
	
	/* Check for locked door */
	player = NULL;
	if (P_MobjIsPlayer(a_Object))
		player = a_Object->player;
	
	if (a_ArgC >= 5 && a_ArgV[4])
	{
		// Not a player?
		if (!player)
			return false;
		
		// Check if player has lacks keys
		if (((player->KeyCards[0] | player->KeyCards[1]) & a_ArgV[4]) != a_ArgV[4])
		{
			// Off sound on door
			S_StartSound(&player->mo->NoiseThinker, sfx_oof);
			
			// Setup Message Info
				// Red
			if (a_ArgV[4] & INFO_REDKEYCOMPAT)
			{
				MsgType = PPM_REDLOCK;
				MsgRef = DS_GetStringRef(DSTR_DEP_PD_REDK);
			}
				
				// Yellow
			else if (a_ArgV[4] & INFO_YELLOWKEYCOMPAT)
			{
				MsgType = PPM_YELLOWLOCK;
				MsgRef = DS_GetStringRef(DSTR_DEP_PD_YELLOWK);
			}
				
				// Blue
			else if (a_ArgV[4] & INFO_BLUEKEYCOMPAT)
			{
				MsgType = PPM_BLUELOCK;
				MsgRef = DS_GetStringRef(DSTR_DEP_PD_BLUEK);
			}
				
				// Unknown
			else
			{
				MsgType = PPM_GENLOCK;
				MsgRef = DS_GetStringRef(DSTR_DNWLINE_LOCKEDDOOR);
			}
			
			// Send message to player and flash in status bar
			P_PlayerMessage(MsgType, a_Object, NULL, MsgRef);
			P_FlashKeys(player, true, a_ArgV[4], a_ArgV[4]);
			
			// Do nothing
			return false;
		}
	}
	
	//SoM: 3/6/2000
	// if the wrong side of door is pushed, give oof sound
	if (a_Line->sidenum[1] == -1)	// killough
	{
		S_StartSound(&player->mo->NoiseThinker, sfx_oof);	// killough 3/20/98
		return false;
	}
	
	// if the sector has an active thinker, use it
	sec = sides[a_Line->sidenum[1]].sector;
	secnum = sec - sectors;
	
	/* Rebound Door? */
	if (sec->ceilingdata)		//SoM: 3/6/2000
	{
		door = sec->ceilingdata;	//SoM: 3/6/2000
		
		// Reboundable?
		if (a_ArgC >= 1 && a_ArgV[0])
		{
			// go back up
			if (door->direction == -1)
				door->direction = 1;
			
			// start going down immediately
			else
			{
				if (!P_MobjIsPlayer(a_Object))
					return false;	// JDC: bad guys never close doors
					
				door->direction = -1;
			}
			
			return true;
		}
	}
	
	/* Play Door Sound */
	if (a_ArgC >= 2 && a_ArgV[1] != sfx_None)
		S_StartSound((mobj_t*)&sec->soundorg, a_ArgV[1]);
	else
		S_StartSound((mobj_t*)&sec->soundorg, sfx_doropn);
	
	// new door thinker
	door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
	P_AddThinker(&door->thinker, PTT_VERTICALDOOR);
	sec->ceilingdata = door;	//SoM:3/6/2000
	door->thinker.function.acp1 = (actionf_p1) T_VerticalDoor;
	door->sector = sec;
	door->direction = 1;
	door->speed = VDOORSPEED;
	door->topwait = VDOORWAIT;
	door->line = a_Line;			// SoM: 3/6/2000: remember line that triggered the door
	
	/* Set door properties */
	// Type of door
	if (a_ArgC >= 3)
		door->type = a_ArgV[2];
	
	// Speed of door
	if (a_ArgC >= 4 && a_ArgV[3])
		door->speed = a_ArgV[3];
	
	// find the top and bottom of the movement range
	door->topheight = P_FindLowestCeilingSurrounding(sec);
	door->topheight -= 4 * FRACUNIT;
	return true;
}


/* EV_SpawnScroller() -- Spawns a floor scroller */
// 1: scoll_t::type parameter
// 2: Scrolling speed is static
// 3: X Change
// 4: Y Change
// 5: Affectee is same line
bool_t EV_SpawnScroller(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV)
{
	scroll_t* Scroll;
	
	/* Start of map and not scroll order? */
	if (a_Type == LAT_MAPSTART && a_Side != PMSS_SCROLLERS)
		return false;
	
	/* Setup Thinker */
	// Allocate
	Scroll = Z_Malloc(sizeof(*Scroll), PU_LEVSPEC, NULL);
	
	// Initialize
	Scroll->thinker.function.acp1 = (actionf_p1)T_Scroll;
	
	/* Add Thinker */
	P_AddThinker(&Scroll->thinker, PTT_SCROLL);
	
	/* Always return true */
	return true;
}

/* EV_DoDoor() -- Opens Door */
// 1: Door Type
// 2: Speed
bool_t EV_DoDoor(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV)
{
	int secnum, rtn;
	sector_t* sec;
	vldoor_t* door;
	
	secnum = -1;
	rtn = 0;
	
	while ((secnum = P_FindSectorFromLineTag(a_Line, secnum)) >= 0)
	{
		sec = &sectors[secnum];
		if (P_SectorActive(ceiling_special, sec))	//SoM: 3/6/2000
			continue;
			
		// new door thinker
		rtn = 1;
		door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
		P_AddThinker(&door->thinker, PTT_VERTICALDOOR);
		sec->ceilingdata = door;	//SoM: 3/6/2000
		
		door->thinker.function.acp1 = (actionf_p1) T_VerticalDoor;
		door->sector = sec;
		door->type = a_ArgV[0];
		door->topwait = VDOORWAIT;
		door->speed = a_ArgV[1];
		door->line = a_Line;		//SoM: 3/6/2000: Remember the line that triggered the door.
		
		switch (a_ArgV[0])
		{
			case blazeClose:
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				door->direction = -1;
				S_StartSound((mobj_t*)&door->sector->soundorg, sfx_bdcls);
				break;
				
			case doorclose:
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				door->direction = -1;
				S_StartSound((mobj_t*)&door->sector->soundorg, P_NLDefDoorCloseSnd());
				break;
				
			case close30ThenOpen:
				door->topheight = sec->ceilingheight;
				door->direction = -1;
				S_StartSound((mobj_t*)&door->sector->soundorg, P_NLDefDoorCloseSnd());
				break;
				
			case blazeRaise:
			case blazeOpen:
				door->direction = 1;
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				if (door->topheight != sec->ceilingheight)
					S_StartSound((mobj_t*)&door->sector->soundorg, sfx_bdopn);
				break;
				
			case normalDoor:
			case dooropen:
				door->direction = 1;
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				if (door->topheight != sec->ceilingheight)
					S_StartSound((mobj_t*)&door->sector->soundorg, sfx_doropn);
				break;
				
			default:
				break;
		}
		
	}
	return rtn;
}

/* EV_DoLockedDoor() -- Opens Locked Door */
// 1: Lock
// 2: Door Type
// 3: Speed
bool_t EV_DoLockedDoor(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV)
{
	player_t* player;
	P_PMType_t MsgType;
	const char** MsgRef;
	
	/* Check */
	if (!a_Object)
		return false;
	
	/* Check for locked door */
	player = NULL;
	if (P_MobjIsPlayer(a_Object))
		player = a_Object->player;
	
	if (a_ArgC >= 1 && a_ArgV[0])
	{
		// Not a player?
		if (!player)
			return false;
		
		// Check if player has lacks keys
		if (((player->KeyCards[0] | player->KeyCards[1]) & a_ArgV[0]) != a_ArgV[0])
		{
			// Off sound on door
			S_StartSound(&player->mo->NoiseThinker, sfx_oof);
			
			// Setup Message Info
				// Red
			if (a_ArgV[0] & INFO_REDKEYCOMPAT)
			{
				MsgType = PPM_REDLOCK;
				MsgRef = DS_GetStringRef(DSTR_DEP_PD_REDK);
			}
				
				// Yellow
			else if (a_ArgV[0] & INFO_YELLOWKEYCOMPAT)
			{
				MsgType = PPM_YELLOWLOCK;
				MsgRef = DS_GetStringRef(DSTR_DEP_PD_YELLOWK);
			}
				
				// Blue
			else if (a_ArgV[0] & INFO_BLUEKEYCOMPAT)
			{
				MsgType = PPM_BLUELOCK;
				MsgRef = DS_GetStringRef(DSTR_DEP_PD_BLUEK);
			}
				
				// Unknown
			else
			{
				MsgType = PPM_GENLOCK;
				MsgRef = DS_GetStringRef(DSTR_DNWLINE_LOCKEDDOOR);
			}
			
			// Send message to player and flash in status bar
			P_PlayerMessage(MsgType, a_Object, NULL, MsgRef);
			P_FlashKeys(player, true, a_ArgV[0], a_ArgV[0]);
			
			// Do nothing
			return false;
		}
	}
	
	return EV_DoDoor(a_Line, a_Side, a_Object, a_Type, a_Flags, a_UseAgain, a_ArgC - 1, a_ArgV + 1);
}

/* EV_ExitLevel() -- Exits the level */
// 1: Secret Exit
bool_t EV_ExitLevel(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV)
{
	/* Check if exiting is not permitted */
	if (!P_XGSVal(PGS_GAMEALLOWLEVELEXIT))
		return false;
	
	/* Change switch texture */
	// The game is rendered so this makes it so the button appears pressed
	if (a_Type == LAT_SWITCH || a_Type == LAT_SHOOT)
		P_ChangeSwitchTexture(a_Line, 0);
	
	/* Now exit the level */
	G_ExitLevel(a_ArgV[0], a_Object, NULL);
	
	/* Line always works */
	return true;
}

/* EV_DoFloor() -- Moves floor */
// 1: Floor Type
bool_t EV_DoFloor(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV)
{
	int secnum;
	int rtn;
	int i;
	sector_t* sec;
	floormove_t* floor;
	
	secnum = -1;
	rtn = 0;
	while ((secnum = P_FindSectorFromLineTag(a_Line, secnum)) >= 0)
	{
		sec = &sectors[secnum];
		
		// SoM: 3/6/2000: Boom has multiple thinkers per sector.
		// Don't start a second thinker on the same floor
		if (P_SectorActive(floor_special, sec))	//jff 2/23/98
			continue;
			
		// new floor thinker
		rtn = 1;
		floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);
		P_AddThinker(&floor->thinker, PTT_MOVEFLOOR);
		sec->floordata = floor;	//SoM: 2/5/2000
		floor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
		floor->type = a_ArgV[0];
		floor->crush = false;
		
		switch (a_ArgV[0])
		{
			case lowerFloor:
				floor->direction = -1;
				floor->sector = sec;
				floor->speed = FLOORSPEED;
				floor->floordestheight = P_FindHighestFloorSurrounding(sec);
				break;
				
				//jff 02/03/30 support lowering floor by 24 absolute
			case lowerFloor24:
				floor->direction = -1;
				floor->sector = sec;
				floor->speed = FLOORSPEED;
				floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;
				break;
				
				//jff 02/03/30 support lowering floor by 32 absolute (fast)
			case lowerFloor32Turbo:
				floor->direction = -1;
				floor->sector = sec;
				floor->speed = FLOORSPEED * 4;
				floor->floordestheight = floor->sector->floorheight + 32 * FRACUNIT;
				break;
				
			case lowerFloorToLowest:
				floor->direction = -1;
				floor->sector = sec;
				floor->speed = FLOORSPEED;
				floor->floordestheight = P_FindLowestFloorSurrounding(sec);
				break;
				
				//jff 02/03/30 support lowering floor to next lowest floor
			case lowerFloorToNearest:
				floor->direction = -1;
				floor->sector = sec;
				floor->speed = FLOORSPEED;
				floor->floordestheight = P_FindNextLowestFloor(sec, floor->sector->floorheight);
				break;
				
			case turboLower:
				floor->direction = -1;
				floor->sector = sec;
				floor->speed = FLOORSPEED * 4;
				floor->floordestheight = P_FindHighestFloorSurrounding(sec);
				if (floor->floordestheight != sec->floorheight)
					floor->floordestheight += 8 * FRACUNIT;
				break;
				
			case raiseFloorCrush:
				floor->crush = true;
			case raiseFloor:
				floor->direction = 1;
				floor->sector = sec;
				floor->speed = FLOORSPEED;
				floor->floordestheight = P_FindLowestCeilingSurrounding(sec);
				if (floor->floordestheight > sec->ceilingheight)
					floor->floordestheight = sec->ceilingheight;
				floor->floordestheight -= (8 * FRACUNIT) * (a_ArgV[0] == raiseFloorCrush);
				break;
				
			case raiseFloorTurbo:
				floor->direction = 1;
				floor->sector = sec;
				floor->speed = FLOORSPEED * 4;
				floor->floordestheight = P_FindNextHighestFloor(sec, sec->floorheight);
				break;
				
			case raiseFloorToNearest:
				floor->direction = 1;
				floor->sector = sec;
				floor->speed = FLOORSPEED;
				floor->floordestheight = P_FindNextHighestFloor(sec, sec->floorheight);
				break;
				
			case raiseFloor24:
				floor->direction = 1;
				floor->sector = sec;
				floor->speed = FLOORSPEED;
				floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;
				break;
				
				// SoM: 3/6/2000: support straight raise by 32 (fast)
			case raiseFloor32Turbo:
				floor->direction = 1;
				floor->sector = sec;
				floor->speed = FLOORSPEED * 4;
				floor->floordestheight = floor->sector->floorheight + 32 * FRACUNIT;
				break;
				
			case raiseFloor512:
				floor->direction = 1;
				floor->sector = sec;
				floor->speed = FLOORSPEED;
				floor->floordestheight = floor->sector->floorheight + 512 * FRACUNIT;
				break;
				
			case raiseFloor24AndChange:
				floor->direction = 1;
				floor->sector = sec;
				floor->speed = FLOORSPEED;
				floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;
				sec->floorpic = a_Line->frontsector->floorpic;
				sec->special = a_Line->frontsector->special;
				sec->oldspecial = a_Line->frontsector->oldspecial;
				break;
				
			case raiseToTexture:
				{
					int minsize = INT_MAX;
					side_t* side;
					
					if (P_XGSVal(PGS_COBOOMSUPPORT))
						minsize = 32000 << FRACBITS;	//SoM: 3/6/2000: ???
					floor->direction = 1;
					floor->sector = sec;
					floor->speed = FLOORSPEED;
					for (i = 0; i < sec->linecount; i++)
					{
						if (twoSided(secnum, i))
						{
							side = getSide(secnum, i, 0);
							
							// jff 8/14/98 don't scan texture 0, its not real
							if (side->bottomtexture > 0 || (!P_XGSVal(PGS_COBOOMSUPPORT) && !side->bottomtexture))
								if (textures[side->bottomtexture]->XHeight < minsize)
									minsize = textures[side->bottomtexture]->XHeight;
							side = getSide(secnum, i, 1);
							
							// jff 8/14/98 don't scan texture 0, its not real
							if (side->bottomtexture > 0 || (!P_XGSVal(PGS_COBOOMSUPPORT) && !side->bottomtexture))
								if (textures[side->bottomtexture]->XHeight < minsize)
									minsize = textures[side->bottomtexture]->XHeight;
						}
					}
					
					if (!P_XGSVal(PGS_COBOOMSUPPORT))
						floor->floordestheight = floor->sector->floorheight + minsize;
					else
					{
						floor->floordestheight = (floor->sector->floorheight >> FRACBITS) + (minsize >> FRACBITS);
						if (floor->floordestheight > 32000)
							floor->floordestheight = 32000;	//jff 3/13/98 do not
						floor->floordestheight <<= FRACBITS;	// allow height overflow
					}
					break;
				}
				
				//SoM: 3/6/2000: Boom changed allot of stuff I guess, and this was one of 'em
			case lowerAndChange:
				floor->direction = -1;
				floor->sector = sec;
				floor->speed = FLOORSPEED;
				floor->floordestheight = P_FindLowestFloorSurrounding(sec);
				floor->texture = sec->floorpic;
				
				// jff 1/24/98 make sure floor->newspecial gets initialized
				// in case no surrounding sector is at floordestheight
				// --> should not affect compatibility <--
				floor->newspecial = sec->special;
				//jff 3/14/98 transfer both old and new special
				floor->oldspecial = sec->oldspecial;
				
				//jff 5/23/98 use model subroutine to unify fixes and handling
				// BP: heretic have change something here
				sec = P_FindModelFloorSector(floor->floordestheight, sec - sectors);
				if (sec)
				{
					floor->texture = sec->floorpic;
					floor->newspecial = sec->special;
					//jff 3/14/98 transfer both old and new special
					floor->oldspecial = sec->oldspecial;
				}
				break;
				
				// Instant Lower SSNTails 06-13-2002
			case instantLower:
				floor->direction = -1;
				floor->sector = sec;
				floor->speed = INT_MAX / 2;	// Go too fast and you'll cause problems...
				floor->floordestheight = P_FindLowestFloorSurrounding(sec);
				break;
			default:
				break;
		}
	}
	
	return rtn;
}

/* EV_DoCeilOrFloor() -- Moves ceiling or floor */
// 1: Ceiling Type
// 2: Floor Tyoe
// 3: Do Both
bool_t EV_DoCeilOrFloor(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV)
{
	/* Either Or */
	if (!a_ArgV[2])
	{
		// Do Ceiling
		if (EV_DoCeiling(a_Line, a_ArgV[0]))
			return true;
	
		// Do Floor
		if (EV_DoFloor(a_Line, a_Side, a_Object, a_Type, a_Flags, a_UseAgain, 1, &a_ArgV[1]))
			return true;
		
		// Nothing worked
		return false;
	}
	
	/* Both */
	else
	{
		// Do ceiling then floor
		EV_DoCeiling(a_Line, a_ArgV[0]);
		EV_DoFloor(a_Line, a_Side, a_Object, a_Type, a_Flags, a_UseAgain, 1, &a_ArgV[1]);
		
		// Always succeed?
		return true;
	}
}

/* EV_DoPlat() -- Moves platform */
// 1: Platform Type
// 2: Amount
bool_t EV_DoPlat(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV)
{
	plat_t* plat;
	int secnum;
	int rtn;
	sector_t* sec;
	
	secnum = -1;
	rtn = 0;
	
	//  Activate all <type> plats that are in_stasis
	switch (a_ArgV[0])
	{
		case PPT_PERPRAISE:
			P_ActivateInStasis(a_Line->tag);
			break;
			
		case toggleUpDn:
			P_ActivateInStasis(a_Line->tag);
			rtn = 1;
			break;
			
		default:
			break;
	}
	
	while ((secnum = P_FindSectorFromLineTag(a_Line, secnum)) >= 0)
	{
		sec = &sectors[secnum];
		
		if (P_SectorActive(floor_special, sec))	//SoM: 3/7/2000:
			continue;
			
		// Find lowest & highest floors around sector
		rtn = 1;
		plat = Z_Malloc(sizeof(*plat), PU_LEVSPEC, 0);
		P_AddThinker(&plat->thinker, PTT_PLATRAISE);
		
		plat->type = a_ArgV[0];
		plat->sector = sec;
		plat->sector->floordata = plat;	//SoM: 3/7/2000
		plat->thinker.function.acp1 = (actionf_p1) T_PlatRaise;
		plat->crush = false;
		plat->tag = a_Line->tag;
		
		//jff 1/26/98 Avoid raise plat bouncing a head off a ceiling and then
		//going down forever -- default low to plat height when triggered
		plat->low = sec->floorheight;
		
		switch (a_ArgV[0])
		{
			case raiseToNearestAndChange:
				plat->speed = PLATSPEED / 2;
				sec->floorpic = sides[a_Line->sidenum[0]].sector->floorpic;
				plat->high = P_FindNextHighestFloor(sec, sec->floorheight);
				plat->wait = 0;
				plat->status = up;
				// NO MORE DAMAGE, IF APPLICABLE
				sec->special = 0;
				sec->oldspecial = 0;	//SoM: 3/7/2000: Clear old field.
				
				S_StartSound((mobj_t*)&sec->soundorg, sfx_stnmov);
				break;
				
			case raiseAndChange:
				plat->speed = PLATSPEED / 2;
				sec->floorpic = sides[a_Line->sidenum[0]].sector->floorpic;
				plat->high = sec->floorheight + a_ArgV[1] * FRACUNIT;
				plat->wait = 0;
				plat->status = up;
				
				S_StartSound((mobj_t*)&sec->soundorg, sfx_stnmov);
				break;
				
			case downWaitUpStay:
				plat->speed = PLATSPEED * 4;
				plat->low = P_FindLowestFloorSurrounding(sec);
				
				if (plat->low > sec->floorheight)
					plat->low = sec->floorheight;
					
				plat->high = sec->floorheight;
				plat->wait = 35 * PLATWAIT;
				plat->status = down;
				S_StartSound((mobj_t*)&sec->soundorg, sfx_pstart);
				break;
				
			case blazeDWUS:
				plat->speed = PLATSPEED * 8;
				plat->low = P_FindLowestFloorSurrounding(sec);
				
				if (plat->low > sec->floorheight)
					plat->low = sec->floorheight;
					
				plat->high = sec->floorheight;
				plat->wait = 35 * PLATWAIT;
				plat->status = down;
				S_StartSound((mobj_t*)&sec->soundorg, sfx_pstart);
				break;
				
			case PPT_PERPRAISE:
				plat->speed = PLATSPEED;
				plat->low = P_FindLowestFloorSurrounding(sec);
				
				if (plat->low > sec->floorheight)
					plat->low = sec->floorheight;
					
				plat->high = P_FindHighestFloorSurrounding(sec);
				
				if (plat->high < sec->floorheight)
					plat->high = sec->floorheight;
					
				plat->wait = 35 * PLATWAIT;
				plat->status = P_Random() & 1;
				
				S_StartSound((mobj_t*)&sec->soundorg, sfx_pstart);
				break;
				
			case toggleUpDn:	//SoM: 3/7/2000: Instant toggle.
				plat->speed = PLATSPEED;
				plat->wait = 35 * PLATWAIT;
				plat->crush = true;
				
				// set up toggling between ceiling, floor inclusive
				plat->low = sec->ceilingheight;
				plat->high = sec->floorheight;
				plat->status = down;
				break;
				
			default:
				break;
		}
		
		P_AddActivePlat(plat);
	}
	return rtn;
}

/* EV_Teleport() -- Teleports a thing */
// 0: Allow Player
// 1: Allow Monster
bool_t EV_Teleport(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV)
{
	int i;
	int tag;
	mobj_t* m;
	thinker_t* thinker;
	sector_t* sector;
	
	/* Allow Players/Monsters? */
	if (P_MobjIsPlayer(a_Object))
	{
		if (!a_ArgV[0])
			return false;
	}
	else
	{
		if (!a_ArgV[1])
			return false;
	}
	
	// don't teleport missiles
	if (!(a_Object->RXFlags[0] & MFREXA_ALWAYSTELEPORT))
		if (((a_Object->flags & MF_MISSILE)) || (a_Object->flags2 & MF2_NOTELEPORT))
			return false;
		
	// Don't teleport if hit back of line,
	//  so you can get out of teleporter.
	if (a_Side)
		return false;
		
	tag = a_Line->tag;
	for (i = 0; i < numsectors; i++)
	{
		if (sectors[i].tag == tag)
		{
			thinker = thinkercap.next;
			for (thinker = thinkercap.next; thinker != &thinkercap; thinker = thinker->next)
			{
				// not a mobj
				if (thinker->function.acp1 != (actionf_p1)P_MobjThinker)
					continue;
					
				m = (mobj_t*)thinker;
				
				// not a teleportman
				if (!(m->RXFlags[0] & MFREXA_ISTELEPORTMAN))
					continue;
					
				sector = m->subsector->sector;
				// wrong sector
				if (sector - sectors != i)
					continue;
					
				return P_Teleport(a_Object, m->x, m->y, m->angle);
			}
		}
	}
	return false;
}

/*
  SoM: 3/15/2000
  Added new boom teleporting functions.
*/
/* EV_SilentTeleport() -- Same as teleport, but no fog */
// 0: Allow Player
// 1: Allow Monster
bool_t EV_SilentTeleport(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV)
{
	int i;
	mobj_t* m;
	thinker_t* th;
	fixed_t deltaviewheight;
	player_t* player;
	fixed_t z, momx, momy, s, c;
	angle_t angle;
	
	/* Allow Players/Monsters? */
	if (P_MobjIsPlayer(a_Object))
	{
		if (!a_ArgV[0])
			return false;
	}
	else
	{
		if (!a_ArgV[1])
			return false;
	}
	
	// Don't teleport if hit back of line, so you can get out of teleporter.
	if (a_Side)
		return false;
	
	// don't teleport missiles
	else if (!(a_Object->RXFlags[0] & MFREXA_ALWAYSTELEPORT))
		if (a_Object->flags & MF_MISSILE)
			return false;
	
	for (i = -1; (i = P_FindSectorFromLineTag(a_Line, i)) >= 0;)
		for (th = thinkercap.next; th != &thinkercap; th = th->next)
			if (th->function.acp1 == (actionf_p1) P_MobjThinker && ((m = (mobj_t*)th)->RXFlags[0] & MFREXA_ISTELEPORTMAN) && m->subsector->sector - sectors == i)
			{
				// Height of thing above ground, in case of mid-air teleports:
				z = a_Object->z - a_Object->floorz;
				
				// Get the angle between the exit thing and source linedef.
				// Rotate 90 degrees, so that walking perpendicularly across
				// teleporter linedef causes thing to exit in the direction
				// indicated by the exit thing.
				angle = R_PointToAngle2(0, 0, a_Line->dx,
				                                a_Line->dy) - m->angle + ANG90;
				                                
				// Sine, cosine of angle adjustment
				s = finesine[angle >> ANGLETOFINESHIFT];
				c = finecosine[angle >> ANGLETOFINESHIFT];
				
				// Momentum of thing crossing teleporter linedef
				momx = a_Object->momx;
				momy = a_Object->momy;
				
				// Whether this is a player, and if so, a pointer to its player_t
				player = NULL;
				if (P_MobjIsPlayer(a_Object))
					player = a_Object->player;
				
				// Attempt to teleport, aborting if blocked
				if (!P_TeleportMove(a_Object, m->x, m->y))
					return false;
					
				// Rotate thing according to difference in angles
				a_Object->angle += angle;
				
				// Adjust z position to be same height above ground as before
				a_Object->z = z + a_Object->floorz;
				
				// Rotate thing's momentum to come out of exit just like it entered
				a_Object->momx = FixedMul(momx, c) - FixedMul(momy, s);
				a_Object->momy = FixedMul(momy, c) + FixedMul(momx, s);
				
				// Adjust player's view, in case there has been a height change
				// Voodoo dolls are excluded by making sure player->mo == thing.
				if (player && player->mo == a_Object)
				{
					// Save the current deltaviewheight, used in stepping
					deltaviewheight = player->deltaviewheight;
					
					// Clear deltaviewheight, since we don't want any changes
					player->deltaviewheight = 0;
					
					// Set player's view according to the newly set parameters
					P_CalcHeight(player);
					
					// Reset the delta to have the same dynamics as before
					player->deltaviewheight = deltaviewheight;
					
					// SoM: 3/15/2000: move chasecam at new player location
					if (a_Object->player->camera.chase)
						P_ResetCamera(a_Object->player);
						
				}
				
				return true;
			}
	
	return false;
}

/* EV_SilentLineTeleport() -- Silent linedef-based TELEPORTATION, by Lee Killough. Primarily for rooms-over-rooms etc. This is the complete player-preserving kind of teleporter. It has advantages over the teleporter with thing exits. */
// 0: Allow Player
// 1: Allow Monster
// 2: Reverse Angle
bool_t EV_SilentLineTeleport(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV)
{
#define FUDGEFACTOR 10	// maximum fixed_t units to move object to avoid hiccups
	int i;
	line_t* l;
	fixed_t pos, x, y, s, c, z;
	angle_t angle;
	int32_t fudge, stepdown, side;
	player_t* player;
	
	/* Allow Players/Monsters? */
	if (P_MobjIsPlayer(a_Object))
	{
		if (!a_ArgV[0])
			return false;
	}
	else
	{
		if (!a_ArgV[1])
			return false;
	}
	
	if (a_Side || a_Object->flags & MF_MISSILE)
		return false;
		
	for (i = -1; (i = P_FindLineFromLineTag(a_Line, i)) >= 0;)
		if ((l = lines + i) != a_Line && l->backsector)
		{
			// Get the thing's position along the source linedef
			pos = abs(a_Line->dx) > abs(a_Line->dy) ? FixedDiv(a_Object->x - a_Line->v1->x, a_Line->dx) : FixedDiv(a_Object->y - a_Line->v1->y, a_Line->dy);
			
			// Get the angle between the two linedefs, for rotating
			// orientation and momentum. Rotate 180 degrees, and flip
			// the position across the exit linedef, if reversed.
			angle = (a_ArgV[2] ? pos = FRACUNIT - pos, 0 : ANG180) + R_PointToAngle2(0, 0, l->dx, l->dy) - R_PointToAngle2(0, 0, a_Line->dx, a_Line->dy);
			
			// Interpolate position across the exit linedef
			x = l->v2->x - FixedMul(pos, l->dx);
			y = l->v2->y - FixedMul(pos, l->dy);
			
			// Sine, cosine of angle adjustment
			s = finesine[angle >> ANGLETOFINESHIFT];
			c = finecosine[angle >> ANGLETOFINESHIFT];
			
			// Maximum distance thing can be moved away from interpolated
			// exit, to ensure that it is on the correct side of exit linedef
			fudge = FUDGEFACTOR;
			
			// Whether this is a player, and if so, a pointer to its player_t.
			// Voodoo dolls are excluded by making sure thing->player->mo==thing.
			player = NULL;
			if (P_MobjIsPlayer(player))
				player = a_Object->player && a_Object->player->mo == a_Object ? a_Object->player : NULL;
			
			// Whether walking towards first side of exit linedef steps down
			stepdown = l->frontsector->floorheight < l->backsector->floorheight;
			
			// Height of thing above ground
			z = a_Object->z - a_Object->floorz;
			
			// Side to exit the linedef on positionally.
			//
			// Notes:
			//
			// This flag concerns exit position, not momentum. Due to
			// roundoff error, the thing can land on either the left or
			// the right side of the exit linedef, and steps must be
			// taken to make sure it does not end up on the wrong side.
			//
			// Exit momentum is always towards side 1 in a reversed
			// teleporter, and always towards side 0 otherwise.
			//
			// Exiting positionally on side 1 is always safe, as far
			// as avoiding oscillations and stuck-in-wall problems,
			// but may not be optimum for non-reversed teleporters.
			//
			// Exiting on side 0 can cause oscillations if momentum
			// is towards side 1, as it is with reversed teleporters.
			//
			// Exiting on side 1 slightly improves player viewing
			// when going down a step on a non-reversed teleporter.
			
			side = a_ArgV[2] || (player && stepdown);
			
			// Make sure we are on correct side of exit linedef.
			while (P_PointOnLineSide(x, y, l) != side && --fudge >= 0)
				if (abs(l->dx) > abs(l->dy))
					y -= l->dx < 0 != side ? -1 : 1;
				else
					x += l->dy < 0 != side ? -1 : 1;
					
			// Attempt to teleport, aborting if blocked
			if (!P_TeleportMove(a_Object, x, y))
				return false;
				
			// Adjust z position to be same height above ground as before.
			// Ground level at the exit is measured as the higher of the
			// two floor heights at the exit linedef.
			a_Object->z = z + sides[l->sidenum[stepdown]].sector->floorheight;
			
			// Rotate thing's orientation according to difference in linedef angles
			a_Object->angle += angle;
			
			// Momentum of thing crossing teleporter linedef
			x = a_Object->momx;
			y = a_Object->momy;
			
			// Rotate thing's momentum to come out of exit just like it entered
			a_Object->momx = FixedMul(x, c) - FixedMul(y, s);
			a_Object->momy = FixedMul(y, c) + FixedMul(x, s);
			
			// Adjust a player's view, in case there has been a height change
			if (player)
			{
				// Save the current deltaviewheight, used in stepping
				fixed_t deltaviewheight = player->deltaviewheight;
				
				// Clear deltaviewheight, since we don't want any changes now
				player->deltaviewheight = 0;
				
				// Set player's view according to the newly set parameters
				P_CalcHeight(player);
				
				// Reset the delta to have the same dynamics as before
				player->deltaviewheight = deltaviewheight;
				
				// SoM: 3/15/2000: move chasecam at new player location
				if (a_Object->player->camera.chase)
					P_ResetCamera(a_Object->player);
			}
			
			return true;
		}
	
	return false;
}

/*****************************************************************************/

// c_LineTrigs -- Static line triggers
static const P_NLTrig_t c_LineTrigs[] =
{
	// Manual Doors (EV_VerticalDoor)
		// Switch
	{1, 0, LAT_SWITCH, PNLF_RETRIG | PNLF_MONSTER | PNLF_IGNORETAG, EV_VerticalDoor, 5,
		{1, sfx_doropn, normalDoor, 0, 0}},
	{26, 0, LAT_SWITCH, PNLF_RETRIG | PNLF_IGNORETAG, EV_VerticalDoor, 5,
		{1, sfx_None, normalDoor, 0, INFO_BLUEKEYCOMPAT}},
	{27, 0, LAT_SWITCH, PNLF_RETRIG | PNLF_IGNORETAG, EV_VerticalDoor, 5,
		{1, sfx_None, normalDoor, 0, INFO_YELLOWKEYCOMPAT}},
	{28, 0, LAT_SWITCH, PNLF_RETRIG | PNLF_IGNORETAG, EV_VerticalDoor, 5,
		{1, sfx_None, normalDoor, 0, INFO_REDKEYCOMPAT}},
	{31, 0, LAT_SWITCH, PNLF_IGNORETAG, EV_VerticalDoor, 5,	// *1
		{0, sfx_doropn, dooropen, 0, 0}},
	{32, 0, LAT_SWITCH, PNLF_MONSTER | PNLF_IGNORETAG, EV_VerticalDoor, 5,	// *1
		{0, sfx_None, dooropen, 0, INFO_BLUEKEYCOMPAT}},
	{33, 0, LAT_SWITCH, PNLF_MONSTER | PNLF_IGNORETAG, EV_VerticalDoor, 5,	// *1
		{0, sfx_None, dooropen, 0, INFO_REDKEYCOMPAT}},
	{34, 0, LAT_SWITCH, PNLF_MONSTER | PNLF_IGNORETAG, EV_VerticalDoor, 5,	// *1
		{0, sfx_None, dooropen, 0, INFO_YELLOWKEYCOMPAT}},
	{117, 0, LAT_SWITCH, PNLF_RETRIG | PNLF_IGNORETAG, EV_VerticalDoor, 5,
		{1, sfx_bdopn, blazeRaise, VDOORSPEED * 4, 0}},
	{118, 0, LAT_SWITCH, PNLF_IGNORETAG, EV_VerticalDoor, 5,	// *1
		{0, sfx_bdopn, blazeOpen, VDOORSPEED * 4, 0}},
	
	// Standard Doors (EV_DoDoor)
		// Walk
	{2, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoDoor, 2,
		{dooropen, VDOORSPEED}},
	{3, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoDoor, 2,
		{doorclose, VDOORSPEED}},
	{4, 0, LAT_WALK, PNLF_CLEARNOTBOOM | PNLF_MONSTER, EV_DoDoor, 2,
		{normalDoor, VDOORSPEED}},
	{16, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoDoor, 2,
		{close30ThenOpen, VDOORSPEED}},
	{75, 0, LAT_WALK, PNLF_RETRIG, EV_DoDoor, 2,
		{doorclose, VDOORSPEED}},
	{76, 0, LAT_WALK, PNLF_RETRIG, EV_DoDoor, 2,
		{close30ThenOpen, VDOORSPEED}},
	{86, 0, LAT_WALK, PNLF_RETRIG, EV_DoDoor, 2,
		{dooropen, VDOORSPEED}},
	{90, 0, LAT_WALK, PNLF_RETRIG, EV_DoDoor, 2,
		{normalDoor, VDOORSPEED}},
	{105, 0, LAT_WALK, PNLF_RETRIG, EV_DoDoor, 2,
		{blazeRaise, 4 * VDOORSPEED}},
	{106, 0, LAT_WALK, PNLF_RETRIG, EV_DoDoor, 2,
		{blazeOpen, 4 * VDOORSPEED}},
	{107, 0, LAT_WALK, PNLF_RETRIG, EV_DoDoor, 2,
		{blazeClose, 4 * VDOORSPEED}},
	{108, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoDoor, 2,
		{blazeRaise, 4 * VDOORSPEED}},
	{109, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoDoor, 2,
		{blazeOpen, 4 * VDOORSPEED}},
	{110, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoDoor, 2,
		{blazeClose, 4 * VDOORSPEED}},
		
		// Switch
	{29, 0, LAT_SWITCH, 0, EV_DoDoor, 2,
		{normalDoor, VDOORSPEED}},
	{42, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoDoor, 2,
		{doorclose, VDOORSPEED}},
	{50, 0, LAT_SWITCH, 0, EV_DoDoor, 2,
		{doorclose, VDOORSPEED}},
	{61, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoDoor, 2,
		{dooropen, VDOORSPEED}},
	{63, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoDoor, 2,
		{normalDoor, VDOORSPEED}},
	{103, 0, LAT_SWITCH, 0, EV_DoDoor, 2,
		{dooropen, VDOORSPEED}},
	{111, 0, LAT_SWITCH, 0, EV_DoDoor, 2,
		{blazeRaise, 4 * VDOORSPEED}},
	{112, 0, LAT_SWITCH, 0, EV_DoDoor, 2,
		{blazeOpen, 4 * VDOORSPEED}},
	{113, 0, LAT_SWITCH, 0, EV_DoDoor, 2,
		{blazeClose, 4 * VDOORSPEED}},
	{114, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoDoor, 2,
		{blazeRaise, 4 * VDOORSPEED}},
	{115, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoDoor, 2,
		{blazeOpen, 4 * VDOORSPEED}},
	{116, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoDoor, 2,
		{blazeClose, 4 * VDOORSPEED}},
	{175, 0, LAT_SWITCH, PNLF_BOOM, EV_DoDoor, 2,
		{close30ThenOpen, VDOORSPEED}},
	{196, 0, LAT_SWITCH, PNLF_BOOM | PNLF_RETRIG, EV_DoDoor, 2,
		{close30ThenOpen, VDOORSPEED}},
		
		// Gun
	{46, 0, LAT_SHOOT, PNLF_RETRIG | PNLF_CLEARNOTBOOM | PNLF_MONSTER, EV_DoDoor, 2,
		{dooropen, VDOORSPEED}},
	
	// Locked Doors (EV_DoLockedDoor)
		// Switch
	{99, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoLockedDoor, 5,
		{INFO_BLUEKEYCOMPAT, blazeOpen, 4 * VDOORSPEED}},
	{133, 0, LAT_SWITCH, 0, EV_DoLockedDoor, 5,
		{INFO_BLUEKEYCOMPAT, blazeOpen, 4 * VDOORSPEED}},
	{134, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoLockedDoor, 5,
		{INFO_REDKEYCOMPAT, blazeOpen, 4 * VDOORSPEED}},
	{135, 0, LAT_SWITCH, 0, EV_DoLockedDoor, 5,
		{INFO_REDKEYCOMPAT, blazeOpen, 4 * VDOORSPEED}},
	{136, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoLockedDoor, 5,
		{INFO_YELLOWKEYCOMPAT, blazeOpen, 4 * VDOORSPEED}},
	{137, 0, LAT_SWITCH, 0, EV_DoLockedDoor, 5,
		{INFO_YELLOWKEYCOMPAT, blazeOpen, 4 * VDOORSPEED}},
	
	// Level Exit
		// Switch
	{11, 0, LAT_SWITCH, PNLF_IGNORETAG, EV_ExitLevel, 1,
		{false}},
	{51, 0, LAT_SWITCH, PNLF_IGNORETAG, EV_ExitLevel, 1,
		{true}},
		
		// Walk
	{52, 0, LAT_WALK, PNLF_IGNORETAG, EV_ExitLevel, 1,
		{false}},
	{124, 0, LAT_WALK, PNLF_IGNORETAG, EV_ExitLevel, 1,
		{true}},
		
		// Shoot
	{197, 0, LAT_SHOOT, PNLF_BOOM | PNLF_IGNORETAG, EV_ExitLevel, 1,
		{false}},
	{198, 0, LAT_SHOOT, PNLF_BOOM | PNLF_IGNORETAG, EV_ExitLevel, 1,
		{true}},
	
	// Floors (EV_DoFloor)
		// Walk
	{5, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoFloor, 1,
		{raiseFloor}},
	{19, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoFloor, 1,
		{lowerFloor}},
	{30, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoFloor, 1,
		{raiseToTexture}},
	{36, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoFloor, 1,
		{turboLower}},
	{37, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoFloor, 1,
		{lowerAndChange}},
	{38, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoFloor, 1,
		{lowerFloorToLowest}},
	{56, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoFloor, 1,
		{raiseFloorCrush}},
	{58, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoFloor, 1,
		{raiseFloor24}},
	{59, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoFloor, 1,
		{raiseFloor24AndChange}},
	{119, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoFloor, 1,
		{raiseFloorToNearest}},
	{130, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoFloor, 1,
		{raiseFloorTurbo}},
	{82, 0, LAT_WALK, PNLF_RETRIG, EV_DoFloor, 1,
		{lowerFloorToLowest}},
	{83, 0, LAT_WALK, PNLF_RETRIG, EV_DoFloor, 1,
		{lowerFloor}},
	{84, 0, LAT_WALK, PNLF_RETRIG, EV_DoFloor, 1,
		{lowerAndChange}},
	{91, 0, LAT_WALK, PNLF_RETRIG, EV_DoFloor, 1,
		{raiseFloor}},
	{92, 0, LAT_WALK, PNLF_RETRIG, EV_DoFloor, 1,
		{raiseFloor24}},
	{93, 0, LAT_WALK, PNLF_RETRIG, EV_DoFloor, 1,
		{raiseFloor24AndChange}},
	{94, 0, LAT_WALK, PNLF_RETRIG, EV_DoFloor, 1,
		{raiseFloorCrush}},
	{96, 0, LAT_WALK, PNLF_RETRIG, EV_DoFloor, 1,
		{raiseToTexture}},
	{98, 0, LAT_WALK, PNLF_RETRIG, EV_DoFloor, 1,
		{turboLower}},
	{128, 0, LAT_WALK, PNLF_RETRIG, EV_DoFloor, 1,
		{raiseFloorToNearest}},
	{129, 0, LAT_WALK, PNLF_RETRIG, EV_DoFloor, 1,
		{raiseFloorTurbo}},
	{142, 0, LAT_WALK, PNLF_BOOM, EV_DoFloor, 1,
		{raiseFloor512}},
	{219, 0, LAT_WALK, PNLF_BOOM, EV_DoFloor, 1,
		{lowerFloorToNearest}},
	{147, 0, LAT_WALK, PNLF_BOOM | PNLF_RETRIG, EV_DoFloor, 1,
		{raiseFloor512}},
	{220, 0, LAT_WALK, PNLF_BOOM | PNLF_RETRIG, EV_DoFloor, 1,
		{lowerFloorToNearest}},
		
		// Switch
	{18, 0, LAT_SWITCH, 0, EV_DoFloor, 1,
		{raiseFloorToNearest}},
	{23, 0, LAT_SWITCH, 0, EV_DoFloor, 1,
		{lowerFloorToLowest}},
	{55, 0, LAT_SWITCH, 0, EV_DoFloor, 1,
		{raiseFloorCrush}},
	{71, 0, LAT_SWITCH, 0, EV_DoFloor, 1,
		{turboLower}},
	{101, 0, LAT_SWITCH, 0, EV_DoFloor, 1,
		{raiseFloor}},
	{102, 0, LAT_SWITCH, 0, EV_DoFloor, 1,
		{lowerFloor}},
	{131, 0, LAT_SWITCH, 0, EV_DoFloor, 1,
		{raiseFloorTurbo}},
	{140, 0, LAT_SWITCH, 0, EV_DoFloor, 1,
		{raiseFloor512}},
	{158, 0, LAT_SWITCH, PNLF_BOOM, EV_DoFloor, 1,
		{raiseToTexture}},
	{159, 0, LAT_SWITCH, PNLF_BOOM, EV_DoFloor, 1,
		{lowerAndChange}},
	{160, 0, LAT_SWITCH, PNLF_BOOM, EV_DoFloor, 1,
		{raiseFloor24AndChange}},
	{161, 0, LAT_SWITCH, PNLF_BOOM, EV_DoFloor, 1,
		{raiseFloor24}},
	{221, 0, LAT_SWITCH, PNLF_BOOM, EV_DoFloor, 1,
		{lowerFloorToNearest}},
	{176, 0, LAT_SWITCH, PNLF_BOOM | PNLF_RETRIG, EV_DoFloor, 1,
		{raiseToTexture}},
	{177, 0, LAT_SWITCH, PNLF_BOOM | PNLF_RETRIG, EV_DoFloor, 1,
		{lowerAndChange}},
	{178, 0, LAT_SWITCH, PNLF_BOOM | PNLF_RETRIG, EV_DoFloor, 1,
		{raiseFloor512}},
	{179, 0, LAT_SWITCH, PNLF_BOOM | PNLF_RETRIG, EV_DoFloor, 1,
		{raiseFloor24AndChange}},
	{180, 0, LAT_SWITCH, PNLF_BOOM | PNLF_RETRIG, EV_DoFloor, 1,
		{raiseFloor24}},
	{222, 0, LAT_SWITCH, PNLF_BOOM | PNLF_RETRIG, EV_DoFloor, 1,
		{lowerFloorToNearest}},
	{45, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoFloor, 1,
		{lowerFloor}},
	{60, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoFloor, 1,
		{lowerFloorToLowest}},
	{64, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoFloor, 1,
		{raiseFloor}},
	{65, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoFloor, 1,
		{raiseFloorCrush}},
	{69, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoFloor, 1,
		{raiseFloorToNearest}},
	{70, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoFloor, 1,
		{turboLower}},
	{132, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoFloor, 1,
		{raiseFloorTurbo}},
		
		// Gun
	{24, 0, LAT_SHOOT, PNLF_CLEARNOTBOOM, EV_DoFloor, 1,
		{raiseFloor}},
	
	// Raise Ceiling or lower floor (EV_DoCeilOrFloor)
		// Walk
	{40, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoCeilOrFloor, 3,
		{raiseToHighest, lowerFloorToLowest, false}},
	{151, 0, LAT_WALK, PNLF_BOOM | PNLF_RETRIG, EV_DoCeilOrFloor, 3,
		{raiseToHighest, lowerFloorToLowest, true}},
		
		// Switch
	{166, 0, LAT_SWITCH, PNLF_BOOM, EV_DoCeilOrFloor, 3,
		{raiseToHighest, lowerFloorToLowest, false}},
	{186, 0, LAT_SWITCH, PNLF_BOOM | PNLF_RETRIG, EV_DoCeilOrFloor, 3,
		{raiseToHighest, lowerFloorToLowest, false}},
	
	// Platforms (EV_DoPlat)
		// Walk
	{10, 0, LAT_WALK, PNLF_CLEARNOTBOOM | PNLF_MONSTER, EV_DoPlat, 2,
		{downWaitUpStay, 0}},
	{22, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoPlat, 2,
		{raiseToNearestAndChange, 0}},
	{53, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoPlat, 2,
		{PPT_PERPRAISE, 0}},
	{121, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoPlat, 2,
		{blazeDWUS, 0}},
	{87, 0, LAT_WALK, PNLF_RETRIG, EV_DoPlat, 2,
		{PPT_PERPRAISE, 0}},
	{88, 0, LAT_WALK, PNLF_RETRIG | PNLF_MONSTER, EV_DoPlat, 2,
		{downWaitUpStay, 0}},
	{95, 0, LAT_WALK, PNLF_RETRIG, EV_DoPlat, 2,
		{raiseToNearestAndChange, 0}},
	{120, 0, LAT_WALK, PNLF_RETRIG, EV_DoPlat, 2,
		{blazeDWUS, 0}},
	{143, 0, LAT_WALK, PNLF_BOOM, EV_DoPlat, 2,
		{raiseAndChange, 24}},
	{144, 0, LAT_WALK, PNLF_BOOM, EV_DoPlat, 2,
		{raiseAndChange, 32}},
	{148, 0, LAT_WALK, PNLF_BOOM | PNLF_RETRIG, EV_DoPlat, 2,
		{raiseAndChange, 24}},
	{149, 0, LAT_WALK, PNLF_BOOM | PNLF_RETRIG, EV_DoPlat, 2,
		{raiseAndChange, 32}},
	{212, 0, LAT_WALK, PNLF_BOOM | PNLF_RETRIG, EV_DoPlat, 2,
		{toggleUpDn, 0}},
		
		// Gun
	{47, 0, LAT_WALK, PNLF_CLEARNOTBOOM, EV_DoPlat, 2,
		{raiseToNearestAndChange, 0}},
		
		// Switch
	{14, 0, LAT_SWITCH, 0, EV_DoPlat, 2,
		{raiseAndChange, 32}},
	{15, 0, LAT_SWITCH, 0, EV_DoPlat, 2,
		{raiseAndChange, 24}},
	{20, 0, LAT_SWITCH, 0, EV_DoPlat, 2,
		{raiseToNearestAndChange, 0}},
	{21, 0, LAT_SWITCH, 0, EV_DoPlat, 2,
		{downWaitUpStay, 0}},
	{122, 0, LAT_SWITCH, 0, EV_DoPlat, 2,
		{blazeDWUS, 0}},
	{162, 0, LAT_SWITCH, PNLF_BOOM, EV_DoPlat, 2,
		{PPT_PERPRAISE, 0}},
	{181, 0, LAT_SWITCH, PNLF_BOOM | PNLF_RETRIG | PNLF_ALWAYS, EV_DoPlat, 2,
		{PPT_PERPRAISE, 0}},
	{211, 0, LAT_SWITCH, PNLF_BOOM | PNLF_RETRIG, EV_DoPlat, 2,
		{toggleUpDn, 0}},
	{62, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoPlat, 2,
		{downWaitUpStay, 1}},
	{66, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoPlat, 2,
		{raiseAndChange, 24}},
	{67, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoPlat, 2,
		{raiseAndChange, 32}},
	{68, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoPlat, 2,
		{raiseToNearestAndChange, 0}},
	{123, 0, LAT_SWITCH, PNLF_RETRIG, EV_DoPlat, 2,
		{blazeDWUS, 0}},
	
	// Teleport
		// Switch
	{174, 0, LAT_SWITCH, PNLF_MONSTER, EV_Teleport, 2,
		{true, true}},
	{195, 0, LAT_SWITCH, PNLF_MONSTER | PNLF_RETRIG, EV_Teleport, 2,
		{true, true}},
		
		// Walk
	{39, 0, LAT_WALK, PNLF_CLEARNOTBOOM | PNLF_MONSTER, EV_Teleport, 2,
		{true, true}},
	{125, 0, LAT_WALK, PNLF_CLEARNOTBOOM | PNLF_MONSTER, EV_Teleport, 2,
		{false, true}},
	{97, 0, LAT_WALK, PNLF_CLEARNOTBOOM | PNLF_MONSTER | PNLF_RETRIG, EV_Teleport, 2,
		{true, true}},
	{126, 0, LAT_WALK, PNLF_CLEARNOTBOOM | PNLF_MONSTER | PNLF_RETRIG, EV_Teleport, 2,
		{false, true}},
	
	// Silent Teleport
		// Switch
	{209, 0, LAT_SWITCH, PNLF_MONSTER | PNLF_BOOM, EV_SilentTeleport, 2,
		{true, true}},
	{210, 0, LAT_SWITCH, PNLF_MONSTER | PNLF_RETRIG | PNLF_BOOM, EV_SilentTeleport, 2,
		{true, true}},
	
		// Walk
	{207, 0, LAT_WALK, PNLF_MONSTER | PNLF_BOOM, EV_SilentTeleport, 2,
		{true, true}},
	{268, 0, LAT_WALK, PNLF_MONSTER | PNLF_BOOM, EV_SilentTeleport, 2,
		{false, true}},
	{208, 0, LAT_WALK, PNLF_MONSTER | PNLF_RETRIG | PNLF_BOOM, EV_SilentTeleport, 2,
		{true, true}},
	{269, 0, LAT_WALK, PNLF_MONSTER | PNLF_RETRIG | PNLF_BOOM, EV_SilentTeleport, 2,
		{false, true}},
	
	// Silent Line Teleport
		// Walk
	{243, 0, LAT_WALK, PNLF_MONSTER | PNLF_BOOM, EV_SilentLineTeleport, 3,
		{true, true, false}},
	{262, 0, LAT_WALK, PNLF_MONSTER | PNLF_BOOM, EV_SilentLineTeleport, 3,
		{true, true, true}},
	{264, 0, LAT_WALK, PNLF_MONSTER | PNLF_BOOM, EV_SilentLineTeleport, 3,
		{false, true, true}},
	{266, 0, LAT_WALK, PNLF_MONSTER | PNLF_BOOM, EV_SilentLineTeleport, 3,
		{false, true, false}},
	{244, 0, LAT_WALK, PNLF_MONSTER | PNLF_RETRIG | PNLF_BOOM, EV_SilentLineTeleport, 3,
		{true, true, false}},
	{263, 0, LAT_WALK, PNLF_MONSTER | PNLF_RETRIG | PNLF_BOOM, EV_SilentLineTeleport, 3,
		{true, true, true}},
	{265, 0, LAT_WALK, PNLF_MONSTER | PNLF_RETRIG | PNLF_BOOM, EV_SilentLineTeleport, 3,
		{false, true, true}},
	{267, 0, LAT_WALK, PNLF_MONSTER | PNLF_RETRIG | PNLF_BOOM, EV_SilentLineTeleport, 3,
		{false, true, false}},
		
	
#if 0
	// Scrollers (EV_SpawnScroller)
	{48, 0, LAT_MAPSTART, 0, EV_SpawnScroller, 5,
		{sc_side, true, FRACUNIT, 0, true}},
	{85, 0, LAT_MAPSTART, 0, EV_SpawnScroller, 5,
		{sc_side, true, -FRACUNIT, 0, true}},
#endif
	
	// End
	{0},
};

// *1 = a_Line->special set to zero in code, but makes no difference

/* P_NLTrigger() -- Triggers a line */
bool_t P_NLTrigger(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain)
{
	uint32_t i;
	
	/* Look in funcs */
	// For matching line trigger ID
	for (i = 0; c_LineTrigs[i].Start; i++)
		if (a_Line->special >= c_LineTrigs[i].Start && a_Line->special <= (c_LineTrigs[i].Start + c_LineTrigs[i].Length))
		{
			// No function?
			if (!c_LineTrigs[i].TrigFunc)
				return false;
			
			// Check trigger compatibility
			if (a_Type != c_LineTrigs[i].TrigType)
				return false;
			
			// Lacks Boom Support?
			if (c_LineTrigs[i].Flags & PNLF_BOOM)
				if (!P_XGSVal(PGS_COBOOMSUPPORT))
					return false;
			
			// Monster cannot activate?
			if (a_Type != LAT_MAPSTART)
				if (!P_MobjIsPlayer(a_Object))
				{
					// Secret lines cannot be activated
					if (a_Line->flags & ML_SECRET)
						return false;
				
					// Disabled in line and not forced activation (ALLTRIGGER)
					if (!(a_Flags & EVTGTF_FORCEUSE))
						if (!(c_LineTrigs[i].Flags & PNLF_MONSTER))
							return false;
				}
			
			// Requires Tag?
			if (P_XGSVal(PGS_COBOOMSUPPORT))
				if (a_Type == LAT_SWITCH || a_Type == LAT_WALK || a_Type == LAT_SHOOT)
					if (!(c_LineTrigs[i].Flags & PNLF_IGNORETAG))
						if (!a_Line->tag)
							return false;
			
			// Call function
			if (c_LineTrigs[i].TrigFunc(a_Line, a_Side, a_Object, a_Type, a_Flags, a_UseAgain, c_LineTrigs[i].ArgC, c_LineTrigs[i].ArgV))
			{
				if (devparm)
					CONL_PrintF("{4Trig %p by %p (side %+1i): Via %c, %8i\n", a_Line, a_Object, a_Side, (a_Type == LAT_WALK ? 'W' : (a_Type == LAT_SHOOT ? 'G' : (a_Type == LAT_MAPSTART ? 'M' : 'S'))), a_Line->special);
				
				// Set use again as trigger type
				if (a_UseAgain)
					*a_UseAgain = !!(c_LineTrigs[i].Flags & PNLF_RETRIG);
				
				// Now set as successful
				return true;
			}
			
			// Clear special regardless if trigger worked or not
			if (c_LineTrigs[i].Flags & PNLF_CLEARNOTBOOM)
				if (!P_XGSVal(PGS_COBOOMSUPPORT))
				{
					if (a_UseAgain)
						*a_UseAgain = false;
					
					a_Line->special = 0;
				}
			
			// Always successful
			if (c_LineTrigs[i].Flags & PNLF_ALWAYS)
				return true;
			
			// Not triggered?
			return false;
		}
	
	if (devparm && a_Object && P_MobjIsPlayer(a_Object))
		CONL_PrintF("{3Trig %p by %p (side %+1i): Via %c, %8i\n", a_Line, a_Object, a_Side, (a_Type == LAT_WALK ? 'W' : (a_Type == LAT_SHOOT ? 'G' : (a_Type == LAT_MAPSTART ? 'M' : 'S'))), a_Line->special);
	
	/* Failed */
	return false;
}

/* P_NLDefDoorCloseSnd() -- Returns the default door closing sound */
int32_t P_NLDefDoorCloseSnd(void)
{
	return sfx_dorcls;
}

