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

/****************
*** CONSTANTS ***
****************/

/* P_NLFlags_t -- Flags for triggers */
typedef enum P_NLFlags_e
{
	PNLF_RETRIG			= UINT32_C(0x00000001),	// Retrigger lines
	PNLF_USETAG			= UINT32_C(0x00000002),	// Requires tag to work
	PNLF_MONSTER		= UINT32_C(0x00000004),	// Monster can activate
	PNLF_BOOM			= UINT32_C(0x00000008),	// Boom Support Required
	PNLF_CLEARNOTBOOM	= UINT32_C(0x00000010),	// Clear Special when not Boom
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
		return 0;
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
				if (!a_Object->player)
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

/*****************************************************************************/

// c_LineTrigs -- Static line triggers
static const P_NLTrig_t c_LineTrigs[] =
{
	// Manual Doors (EV_VerticalDoor)
	{1, 0, LAT_SWITCH, PNLF_RETRIG | PNLF_MONSTER, EV_VerticalDoor, 5,
		{1, sfx_doropn, normalDoor, 0, 0}},
	{26, 0, LAT_SWITCH, PNLF_RETRIG, EV_VerticalDoor, 5,
		{1, sfx_None, normalDoor, 0, INFO_BLUEKEYCOMPAT}},
	{27, 0, LAT_SWITCH, PNLF_RETRIG, EV_VerticalDoor, 5,
		{1, sfx_None, normalDoor, 0, INFO_YELLOWKEYCOMPAT}},
	{28, 0, LAT_SWITCH, PNLF_RETRIG, EV_VerticalDoor, 5,
		{1, sfx_None, normalDoor, 0, INFO_REDKEYCOMPAT}},
	{31, 0, LAT_SWITCH, 0, EV_VerticalDoor, 5,	// *1
		{0, sfx_doropn, dooropen, 0, 0}},
	{32, 0, LAT_SWITCH, PNLF_MONSTER, EV_VerticalDoor, 5,	// *1
		{0, sfx_None, dooropen, 0, INFO_BLUEKEYCOMPAT}},
	{33, 0, LAT_SWITCH, PNLF_MONSTER, EV_VerticalDoor, 5,	// *1
		{0, sfx_None, dooropen, 0, INFO_REDKEYCOMPAT}},
	{34, 0, LAT_SWITCH, PNLF_MONSTER, EV_VerticalDoor, 5,	// *1
		{0, sfx_None, dooropen, 0, INFO_YELLOWKEYCOMPAT}},
	{117, 0, LAT_SWITCH, PNLF_RETRIG, EV_VerticalDoor, 5,
		{1, sfx_bdopn, blazeRaise, VDOORSPEED * 4, 0}},
	{118, 0, LAT_SWITCH, 0, EV_VerticalDoor, 5,	// *1
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
				if (!a_Object->player)
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
					if (c_LineTrigs[i].Flags & PNLF_USETAG)
						if (!a_Line->tag)
							return false;
			
			// Call function
			if (c_LineTrigs[i].TrigFunc(a_Line, a_Side, a_Object, a_Type, a_Flags, a_UseAgain, c_LineTrigs[i].ArgC, c_LineTrigs[i].ArgV))
			{
				if (devparm)
					CONL_PrintF("Trig %p by %p (side %+1i): Via %c, %8x\n", a_Line, a_Object, a_Side, (a_Type == LAT_WALK ? 'W' : (a_Type == LAT_SHOOT ? 'G' : (a_Type == LAT_MAPSTART ? 'M' : 'S'))), a_Line->special);
				
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
			
			// Not triggered?
			return false;
		}
	
	/* Failed */
	return false;
}

/* P_NLDefDoorCloseSnd() -- Returns the default door closing sound */
int32_t P_NLDefDoorCloseSnd(void)
{
	return sfx_dorcls;
}

