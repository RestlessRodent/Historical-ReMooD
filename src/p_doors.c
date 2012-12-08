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
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION:
//      Door animation code (opening/closing)

#include "doomdef.h"
#include "doomstat.h"
#include "dstrings.h"
#include "p_local.h"
#include "r_state.h"
#include "s_sound.h"
#include "z_zone.h"
#include "p_demcmp.h"

#if 0
//
// Sliding door frame information
//
slidename_t slideFrameNames[MAXSLIDEDOORS] =
{
	{
		"GDOORF1", "GDOORF2", "GDOORF3", "GDOORF4",	// front
		"GDOORB1", "GDOORB2", "GDOORB3", "GDOORB4"
	}
	,							// back
	
	{"\0", "\0", "\0", "\0"}
};
#endif

// =========================================================================
//                            VERTICAL DOORS
// =========================================================================

int doorclosesound = sfx_dorcls;

//
// T_VerticalDoor
//
void T_VerticalDoor(vldoor_t* door)
{
	result_e res;
	
	switch (door->direction)
	{
		case 0:
			// WAITING
			if (!--door->topcountdown)
			{
				switch (door->type)
				{
					case blazeRaise:
					case genBlazeRaise:	//SoM: 3/6/2000
						door->direction = -1;	// time to go back down
						S_StartSound((mobj_t*)&door->sector->soundorg, sfx_bdcls);
						break;
						
					case normalDoor:
					case genRaise:	//SoM: 3/6/2000
						door->direction = -1;	// time to go back down
						S_StartSound((mobj_t*)&door->sector->soundorg, doorclosesound);
						break;
						
					case close30ThenOpen:
					case genCdO:	//SoM: 3/6/2000
						door->direction = 1;
						S_StartSound((mobj_t*)&door->sector->soundorg, sfx_doropn);
						break;
						
						//SoM: 3/6/2000
					case genBlazeCdO:
						door->direction = 1;	// time to go back up
						S_StartSound((mobj_t*)&door->sector->soundorg, sfx_bdopn);
						break;
						
					default:
						break;
				}
			}
			break;
			
		case 2:
			//  INITIAL WAIT
			if (!--door->topcountdown)
			{
				switch (door->type)
				{
					case raiseIn5Mins:
						door->direction = 1;
						door->type = normalDoor;
						S_StartSound((mobj_t*)&door->sector->soundorg, sfx_doropn);
						break;
						
					default:
						break;
				}
			}
			break;
			
		case -1:
			// DOWN
			res = T_MovePlane(door->sector, door->speed, door->sector->floorheight, false, 1, door->direction);
			if (res == pastdest)
			{
				switch (door->type)
				{
					case blazeRaise:
					case blazeClose:
					case genBlazeRaise:
					case genBlazeClose:
						door->sector->ceilingdata = NULL;	// SoM: 3/6/2000
						P_RemoveThinker(&door->thinker);	// unlink and free
						if (P_XGSVal(PGS_COBOOMSUPPORT))	//SoM: Removes the double closing sound of doors.
							S_StartSound((mobj_t*)&door->sector->soundorg, sfx_bdcls);
						break;
						
					case normalDoor:
					case doorclose:
					case genRaise:
					case genClose:
						door->sector->ceilingdata = NULL;	//SoM: 3/6/2000
						P_RemoveThinker(&door->thinker);	// unlink and free
						break;
						
					case close30ThenOpen:
						door->direction = 0;
						door->topcountdown = 35 * 30;
						break;
						
						//SoM: 3/6/2000: General door stuff
					case genCdO:
					case genBlazeCdO:
						door->direction = 0;
						door->topcountdown = door->topwait;
						break;
						
					default:
						break;
				}
				//SoM: 3/6/2000: Code to turn lighting off in tagged sectors.
				// Hurdler: FIXME: there is a bug in map27 with door->line not being correct
				//                 after a save game / load game
				if (P_XGSVal(PGS_COBOOMSUPPORT) && door->line && door->line->tag)
				{
					if (door->line->special > GenLockedBase && (door->line->special & 6) == 6)
						EV_TurnTagLightsOff(door->line);
					else
					{
						switch (door->line->special)
						{
							case 1:
							case 31:
							case 26:
							case 27:
							case 28:
							case 32:
							case 33:
							case 34:
							case 117:
							case 118:
								EV_TurnTagLightsOff(door->line);
								break;
							default:
								break;
						}
					}
				}
			}
			else if (res == crushed)
			{
				switch (door->type)
				{
					case genClose:	//SoM: 3/6/2000
					case genBlazeClose:	//SoM: 3/6/2000
					case blazeClose:
					case doorclose:	// DO NOT GO BACK UP!
						break;
					default:
						door->direction = 1;
						S_StartSound((mobj_t*)&door->sector->soundorg, sfx_doropn);
						break;
				}
			}
			break;
			
		case 1:
			// UP
			res = T_MovePlane(door->sector, door->speed, door->topheight, false, 1, door->direction);
			if (res == pastdest)
			{
				switch (door->type)
				{
					case blazeRaise:
					case normalDoor:
					case genRaise:	//SoM: 3/6/2000
					case genBlazeRaise:	//SoM: 3/6/2000
						door->direction = 0;	// wait at top
						door->topcountdown = door->topwait;
						break;
						
					case close30ThenOpen:
					case blazeOpen:
					case dooropen:
					case genBlazeOpen:
					case genOpen:
					case genCdO:
					case genBlazeCdO:
						door->sector->ceilingdata = NULL;
						P_RemoveThinker(&door->thinker);	// unlink and free
						
						break;
						
					default:
						break;
				}
				//SoM: 3/6/2000: turn lighting on in tagged sectors of manual doors
				if (P_XGSVal(PGS_COBOOMSUPPORT) && door->line && door->line->tag)
				{
					if (door->line->special > GenLockedBase && (door->line->special & 6) == 6)	//jff 3/9/98 all manual doors
						EV_LightTurnOn(door->line, 0);
					else
					{
						switch (door->line->special)
						{
							case 1:
							case 31:
							case 26:
							case 27:
							case 28:
							case 32:
							case 33:
							case 34:
							case 117:
							case 118:
								EV_LightTurnOn(door->line, 0);
								break;
							default:
								break;
						}
					}
				}
			}
			break;
	}
}

//
// EV_DoLockedDoor
// Move a locked door up/down
//
// SoM: Removed the player checks at every different color door (checking to make sure 'p' is
// not NULL) because you only need to do that once.
int EV_DoLockedDoor(line_t* line, vldoor_e type, mobj_t* thing, fixed_t speed)
{
	player_t* p;
	
	p = thing->player;
	
	if (!p)
		return 0;
		
	switch (line->special)
	{
		case 99:				// Blue Lock
		case 133:
			if (((!(p->cards & it_bluecard) && !(p->cards & it_blueskull))))
			{
				//p->message = PD_BLUEO;
				S_StartSound(&p->mo->NoiseThinker, sfx_oof);	//SoM: 3/6/200: killough's idea
				return 0;
			}
			break;
			
		case 134:				// Red Lock
		case 135:
			if (((!(p->cards & it_redcard) && !(p->cards & it_redskull))))
			{
				//p->message = PD_REDO;
				S_StartSound(&p->mo->NoiseThinker, sfx_oof);	//SoM: 3/6/200: killough's idea
				return 0;
			}
			break;
			
		case 136:				// Yellow Lock
		case 137:
			if (((!(p->cards & it_yellowcard) && !(p->cards & it_yellowskull))))
			{
				//p->message = PD_YELLOWO;
				S_StartSound(&p->mo->NoiseThinker, sfx_oof);	//SoM: 3/6/200: killough's idea
				return 0;
			}
			break;
	}
	
	return EV_DoDoor(line, type, speed);
}

int EV_DoDoor(line_t* line, vldoor_e type, fixed_t speed)
{
	int secnum, rtn;
	sector_t* sec;
	vldoor_t* door;
	
	secnum = -1;
	rtn = 0;
	
	while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
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
		door->type = type;
		door->topwait = VDOORWAIT;
		door->speed = speed;
		door->line = line;		//SoM: 3/6/2000: Remember the line that triggered the door.
		
		switch (type)
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
				S_StartSound((mobj_t*)&door->sector->soundorg, doorclosesound);
				break;
				
			case close30ThenOpen:
				door->topheight = sec->ceilingheight;
				door->direction = -1;
				S_StartSound((mobj_t*)&door->sector->soundorg, doorclosesound);
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

//
// EV_OpenDoor
// Generic function to open a door (used by FraggleScript)

void EV_OpenDoor(int sectag, int speed, int wait_time)
{
	vldoor_e door_type;
	int secnum = -1;
	vldoor_t* door;
	
	if (speed < 1)
		speed = 1;
		
	// find out door type first
	
	if (wait_time)				// door closes afterward
	{
		if (speed >= 4)			// blazing ?
			door_type = blazeRaise;
		else
			door_type = normalDoor;
	}
	else
	{
		if (speed >= 4)			// blazing ?
			door_type = blazeOpen;
		else
			door_type = dooropen;
	}
	
	// open door in all the sectors with the specified tag
	
	while ((secnum = P_FindSectorFromTag(sectag, secnum)) >= 0)
	{
		sector_t* sec = &sectors[secnum];
		
		// if the ceiling already moving, don't start the door action
		if (P_SectorActive(ceiling_special, sec))
			continue;
			
		// new door thinker
		door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
		P_AddThinker(&door->thinker, PTT_VERTICALDOOR);
		sec->ceilingdata = door;
		
		door->thinker.function.acp1 = (actionf_p1) T_VerticalDoor;
		door->sector = sec;
		door->type = door_type;
		door->topwait = wait_time;
		door->speed = VDOORSPEED * speed;
		door->line = NULL;		// not triggered by a line
		door->topheight = P_FindLowestCeilingSurrounding(sec) - 4 * FRACUNIT;
		door->direction = 1;
		
		if (door->topheight != sec->ceilingheight)
			S_StartSound((mobj_t*)&door->sector->soundorg, speed >= 4 ? sfx_bdopn : sfx_doropn);
	}
}

//
// EV_CloseDoor
//
// Used by FraggleScript
void EV_CloseDoor(int sectag, int speed)
{
	vldoor_e door_type;
	int secnum = -1;
	vldoor_t* door;
	
	if (speed < 1)
		speed = 1;
		
	// find out door type first
	
	if (speed >= 4)				// blazing ?
		door_type = blazeClose;
	else
		door_type = doorclose;
		
	// open door in all the sectors with the specified tag
	
	while ((secnum = P_FindSectorFromTag(sectag, secnum)) >= 0)
	{
		sector_t* sec = &sectors[secnum];
		
		// if the ceiling already moving, don't start the door action
		if (P_SectorActive(ceiling_special, sec))	//jff 2/22/98
			continue;
			
		// new door thinker
		door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
		P_AddThinker(&door->thinker, PTT_VERTICALDOOR);
		sec->ceilingdata = door;
		
		door->thinker.function.acp1 = (actionf_p1) T_VerticalDoor;
		door->sector = sec;
		door->type = door_type;
		door->speed = VDOORSPEED * speed;
		door->line = NULL;		// not triggered by a line
		door->topheight = P_FindLowestCeilingSurrounding(sec) - 4 * FRACUNIT;
		door->direction = -1;
		
		S_StartSound((mobj_t*)&door->sector->soundorg, speed >= 4 ? sfx_bdcls : sfx_dorcls);
	}
}

//
// EV_VerticalDoor : open a door manually, no tag value
//
//SoM: 3/6/2000: Needs int return for boom compatability. Also removed "side" and used boom
//methods insted.
int EV_VerticalDoor(line_t* line, mobj_t* thing)
{
	player_t* player;
	int secnum;
	sector_t* sec;
	vldoor_t* door;
	
//    int         side; //SoM: 3/6/2000

//    side = 0;   // only front sides can be used

	//  Check for locks
	player = thing->player;
	
	switch (line->special)
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
	if (line->sidenum[1] == -1)	// killough
	{
		S_StartSound(&player->mo->NoiseThinker, sfx_oof);	// killough 3/20/98
		return 0;
	}
	// if the sector has an active thinker, use it
	sec = sides[line->sidenum[1]].sector;
	secnum = sec - sectors;
	
	if (sec->ceilingdata)		//SoM: 3/6/2000
	{
		door = sec->ceilingdata;	//SoM: 3/6/2000
		switch (line->special)
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
					if (!thing->player)
						return 0;	// JDC: bad guys never close doors
						
					door->direction = -1;	// start going down immediately
				}
				return 1;
		}
	}
	// for proper sound
	switch (line->special)
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
	door->line = line;			// SoM: 3/6/2000: remember line that triggered the door
	
	switch (line->special)
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
			line->special = 0;
			break;
			
		case 117:				// blazing door raise
			door->type = blazeRaise;
			door->speed = VDOORSPEED * 4;
			break;
		case 118:				// blazing door open
			door->type = blazeOpen;
			line->special = 0;
			door->speed = VDOORSPEED * 4;
			break;
	}
	
	// find the top and bottom of the movement range
	door->topheight = P_FindLowestCeilingSurrounding(sec);
	door->topheight -= 4 * FRACUNIT;
	return 1;
}

//
// Spawn a door that closes after 30 seconds
//
vldoor_t* P_SpawnDoorCloseIn(sector_t* sec, const uint32_t a_Tics, const uint32_t a_Type)
{
	vldoor_t* door;
	
	door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
	
	P_AddThinker(&door->thinker, PTT_VERTICALDOOR);
	
	sec->ceilingdata = door;	//SoM: 3/6/2000
	sec->special = 0;
	
	door->thinker.function.acp1 = (actionf_p1) T_VerticalDoor;
	door->sector = sec;
	door->direction = 0;
	door->type = a_Type;
	door->speed = VDOORSPEED;
	door->topcountdown = a_Tics;
	door->line = NULL;			//SoM: Remember the line that triggered the door.
	
	return door;
}

void P_SpawnDoorCloseIn30(sector_t* sec)
{
	P_SpawnDoorCloseIn(sec, 30 * 35, normalDoor);
}

//
// Spawn a door that opens after 5 minutes
//
vldoor_t* P_SpawnDoorRaiseIn(sector_t* sec, const bool_t a_InitWait, const uint32_t a_Tics, const uint32_t a_Type)
{
	vldoor_t* door;
	
	door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
	
	P_AddThinker(&door->thinker, PTT_VERTICALDOOR);
	
	sec->ceilingdata = door;	//SoM: 3/6/2000
	sec->special = 0;
	
	door->thinker.function.acp1 = (actionf_p1) T_VerticalDoor;
	door->sector = sec;
	door->direction = (a_InitWait ? 2 : 1);
	door->type = a_Type;
	door->speed = VDOORSPEED;
	door->topheight = P_FindLowestCeilingSurrounding(sec);
	door->topheight -= 4 * FRACUNIT;
	door->topwait = VDOORWAIT;
	door->topcountdown = a_Tics;
	door->line = NULL;			//SoM: 3/6/2000: You know....
	
	return door;
}

void P_SpawnDoorRaiseIn5Mins(sector_t* sec, int secnum)
{
	P_SpawnDoorRaiseIn(sec, true, 5 * 60 * 35, raiseIn5Mins);
}

// ==========================================================================
//                        SLIDE DOORS, UNUSED
// ==========================================================================

#if 0							// ABANDONED TO THE MISTS OF TIME!!!
//
// EV_SlidingDoor : slide a door horizontally
// (animate midtexture, then set noblocking line)
//

/*slideframe_t slideFrames[MAXSLIDEDOORS];

void P_InitSlidingDoorFrames(void)
{
    int         i;
    int         f1;
    int         f2;
    int         f3;
    int         f4;

    // DOOM II ONLY...
    if ( gamemode != commercial)
        return;

    for (i = 0;i < MAXSLIDEDOORS; i++)
    {
        if (!slideFrameNames[i].frontFrame1[0])
            break;

        f1 = R_TextureNumForName(slideFrameNames[i].frontFrame1);
        f2 = R_TextureNumForName(slideFrameNames[i].frontFrame2);
        f3 = R_TextureNumForName(slideFrameNames[i].frontFrame3);
        f4 = R_TextureNumForName(slideFrameNames[i].frontFrame4);

        slideFrames[i].frontFrames[0] = f1;
        slideFrames[i].frontFrames[1] = f2;
        slideFrames[i].frontFrames[2] = f3;
        slideFrames[i].frontFrames[3] = f4;

        f1 = R_TextureNumForName(slideFrameNames[i].backFrame1);
        f2 = R_TextureNumForName(slideFrameNames[i].backFrame2);
        f3 = R_TextureNumForName(slideFrameNames[i].backFrame3);
        f4 = R_TextureNumForName(slideFrameNames[i].backFrame4);

        slideFrames[i].backFrames[0] = f1;
        slideFrames[i].backFrames[1] = f2;
        slideFrames[i].backFrames[2] = f3;
        slideFrames[i].backFrames[3] = f4;
    }
}

//
// Return index into "slideFrames" array
// for which door type to use
//
int P_FindSlidingDoorType(line_t*       line)
{
    int         i;
    int         val;

    for (i = 0;i < MAXSLIDEDOORS;i++)
    {
        val = sides[line->sidenum[0]].midtexture;
        if (val == slideFrames[i].frontFrames[0])
            return i;
    }

    return -1;
}

void T_SlidingDoor (slidedoor_t*        door)
{
    switch(door->status)
    {
      case sd_opening:
        if (!door->timer--)
        {
            if (++door->frame == SNUMFRAMES)
            {
                // IF DOOR IS DONE OPENING...
                sides[door->line->sidenum[0]].midtexture = 0;
                sides[door->line->sidenum[1]].midtexture = 0;
                door->line->flags &= ML_BLOCKING^0xff;

                if (door->type == sdt_openOnly)
                {
                    door->frontsector->ceilingdata = NULL;
                    P_RemoveThinker (&door->thinker);
                    break;
                }

                door->timer = SDOORWAIT;
                door->status = sd_waiting;
            }
            else
            {
                // IF DOOR NEEDS TO ANIMATE TO NEXT FRAME...
                door->timer = SWAITTICS;

                sides[door->line->sidenum[0]].midtexture =
                    slideFrames[door->whichDoorIndex].
                    frontFrames[door->frame];
                sides[door->line->sidenum[1]].midtexture =
                    slideFrames[door->whichDoorIndex].
                    backFrames[door->frame];
            }
        }
        break;

      case sd_waiting:
        // IF DOOR IS DONE WAITING...
        if (!door->timer--)
        {
            // CAN DOOR CLOSE?
            if (door->frontsector->thinglist != NULL ||
                door->backsector->thinglist != NULL)
            {
                door->timer = SDOORWAIT;
                break;
            }

            //door->frame = SNUMFRAMES-1;
            door->status = sd_closing;
            door->timer = SWAITTICS;
        }
        break;

      case sd_closing:
        if (!door->timer--)
        {
            if (--door->frame < 0)
            {
                // IF DOOR IS DONE CLOSING...
                door->line->flags |= ML_BLOCKING;
                door->frontsector->specialdata = NULL;
                P_RemoveThinker (&door->thinker);
                break;
            }
            else
            {
                // IF DOOR NEEDS TO ANIMATE TO NEXT FRAME...
                door->timer = SWAITTICS;

                sides[door->line->sidenum[0]].midtexture =
                    slideFrames[door->whichDoorIndex].
                    frontFrames[door->frame];
                sides[door->line->sidenum[1]].midtexture =
                    slideFrames[door->whichDoorIndex].
                    backFrames[door->frame];
            }
        }
        break;
    }
}

void
EV_SlidingDoor
( line_t*       line,
  mobj_t*       thing )
{
    sector_t*           sec;
    slidedoor_t*        door;

    // DOOM II ONLY...
    if (gamemode != commercial)
        return;

    // Make sure door isn't already being animated
    sec = line->frontsector;
    door = NULL;
    if (sec->specialdata)
    {
        if (!thing->player)
            return;

        door = sec->specialdata;
        if (door->type == sdt_openAndClose)
        {
            if (door->status == sd_waiting)
                door->status = sd_closing;
        }
        else
            return;
    }

    // Init sliding door vars
    if (!door)
    {
        door = Z_Malloc (sizeof(*door), PU_LEVSPEC, 0);
        P_AddThinker (&door->thinker);
        sec->specialdata = door;

        door->type = sdt_openAndClose;
        door->status = sd_opening;
        door->whichDoorIndex = P_FindSlidingDoorType(line);

        if (door->whichDoorIndex < 0)
            I_Error("EV_SlidingDoor: Can't use texture for sliding door!");

        door->frontsector = sec;
        door->backsector = line->backsector;
        door->thinker.function = T_SlidingDoor;
        door->timer = SWAITTICS;
        door->frame = 0;
        door->line = line;
    }
}*/
#endif
