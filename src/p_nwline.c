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

/*****************
*** STRUCTURES ***
*****************/

typedef struct P_NLTrig_s
{
	uint32_t Start;								// Start of line (>=)
	uint32_t Length;							// Lines to consider (<=)
	EV_TryGenType_t TrigType;					// Trigger Type
	bool_t ReTrig;								// Retrigger
	
	P_NLTrigFunc_t TrigFunc;					// Trigger Function
	uint32_t ArgC;								// Argument Count
	int32_t ArgV[10];							// Arguments
} P_NLTrig_t;

/****************
*** FUNCTIONS ***
****************/

/*****************************************************************************/


/* EV_VerticalDoor() -- open a door manually, no tag value */
bool_t EV_VerticalDoor(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV)
{
	player_t* player;
	int secnum;
	sector_t* sec;
	vldoor_t* door;
	
	/* Check */
	if (!a_Object)
		return false;
	
	//  Check for locks
	player = a_Object->player;
	
	switch (a_Line->special)
	{
		case 26:				// Blue Lock
		case 32:
			if (!player)
				return 0;
			if (((!(player->cards & it_bluecard) && !(player->cards & it_blueskull))))
			{
				//player->message = PD_BLUEK;
				S_StartSound(&player->mo->NoiseThinker, sfx_oof);	//SoM: 3/6/2000: Killough's idea
				return 0;
			}
			break;
			
		case 27:				// Yellow Lock
		case 34:
			if (!player)
				return 0;
				
			if (((!(player->cards & it_yellowcard) && !(player->cards & it_yellowskull))))
			{
				//player->message = PD_YELLOWK;
				S_StartSound(&player->mo->NoiseThinker, sfx_oof);	//SoM: 3/6/2000: Killough's idea
				return 0;
			}
			break;
			
		case 28:				// Red Lock
		case 33:
			if (!player)
				return 0;
				
			if (((!(player->cards & it_redcard) && !(player->cards & it_redskull))))
			{
				//player->message = PD_REDK;
				S_StartSound(&player->mo->NoiseThinker, sfx_oof);	//SoM: 3/6/2000: Killough's idea
				return 0;
			}
			break;
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
	
	if (sec->ceilingdata)		//SoM: 3/6/2000
	{
		door = sec->ceilingdata;	//SoM: 3/6/2000
		switch (a_Line->special)
		{
			case 1:			// ONLY FOR "RAISE" DOORS, NOT "OPEN"s
			case 26:
			case 27:
			case 28:
			case 117:
				if (door->direction == -1)
					door->direction = 1;	// go back up
				else
				{
					if (!a_Object->player)
						return 0;	// JDC: bad guys never close doors
						
					door->direction = -1;	// start going down immediately
				}
				return 1;
		}
	}
	// for proper sound
	switch (a_Line->special)
	{
		case 117:				// BLAZING DOOR RAISE
		case 118:				// BLAZING DOOR OPEN
			S_StartSound((mobj_t*)&sec->soundorg, sfx_bdopn);
			break;
			
		case 1:				// NORMAL DOOR SOUND
		case 31:
			S_StartSound((mobj_t*)&sec->soundorg, sfx_doropn);
			break;
			
		default:				// LOCKED DOOR SOUND
			S_StartSound((mobj_t*)&sec->soundorg, sfx_doropn);
			break;
	}
	
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
	if (a_ArgC >= 1)
		door->type = a_ArgV[0];
	
	switch (a_Line->special)
	{
		case 1:
		case 26:
		case 27:
		case 28:
			door->type = normalDoor;
			break;
			
		case 31:
		case 32:
		case 33:
		case 34:
			door->type = dooropen;
			a_Line->special = 0;
			break;
			
		case 117:				// blazing door raise
			door->type = blazeRaise;
			door->speed = VDOORSPEED * 4;
			break;
		case 118:				// blazing door open
			door->type = blazeOpen;
			a_Line->special = 0;
			door->speed = VDOORSPEED * 4;
			break;
	}
	
	// find the top and bottom of the movement range
	door->topheight = P_FindLowestCeilingSurrounding(sec);
	door->topheight -= 4 * FRACUNIT;
	return 1;
}

/*****************************************************************************/

// c_LineTrigs -- Static line triggers
static const P_NLTrig_t c_LineTrigs[] =
{
	// Standard Doom Doors
	{1, 0, EVTGT_SWITCH, true, EV_VerticalDoor, 2, {normalDoor, VDOORSPEED}},
	
	// End
	{0, 0, 0, false, NULL, 0, {0}},
};

/* P_NLTrigger() -- Triggers a line */
bool_t P_NLTrigger(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain)
{
	uint32_t i;
	
	/* Debug */
	if (devparm)
		CONL_PrintF("Trig %p by %p (side %+1i): Via %c, %8x\n", a_Line, a_Object, a_Side, (a_Type == EVTGT_WALK ? 'W' : (a_Type == EVTGT_SHOOT ? 'G' : 'S')), a_Line->special);
	
	/* Look in funcs */
	// For matching line trigger ID
	for (i = 0; c_LineTrigs[i].Start; i++)
		if (a_Line->special >= c_LineTrigs[i].Start && a_Line->special <= (c_LineTrigs[i].Start + c_LineTrigs[i].Length))
		{
			// No function?
			if (!c_LineTrigs[i].TrigFunc)
				continue;
			
			// Check trigger compatibility
			if (a_Type != c_LineTrigs[i].TrigType)
				continue;
			
			// Call function
			if (c_LineTrigs[i].TrigFunc(a_Line, a_Side, a_Object, a_Type, a_Flags, a_UseAgain, c_LineTrigs[i].ArgC, c_LineTrigs[i].ArgV))
			{
				// Set use again as trigger type
				if (a_UseAgain)
					*a_UseAgain = c_LineTrigs[i].ReTrig;
				
				// Now set as succesful
				return true;
			}
			
			// Not triggered?
			return false;
		}
	
	/* Failed */
	return false;
}

