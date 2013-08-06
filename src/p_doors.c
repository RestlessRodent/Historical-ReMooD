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

#include "p_spec.h"
#include "s_sound.h"
#include "z_zone.h"
#include "p_demcmp.h"
#include "p_nwline.h"

//#include "doomdef.h"
//#include "doomstat.h"
//#include "dstrings.h"
//#include "p_local.h"
//#include "r_state.h"
//#include "s_sound.h"
//#include "z_zone.h"
//#include "p_demcmp.h"
//#include "p_nwline.h"

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
	int32_t LightArg;
	
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
					if ((door->line->special > GenLockedBase && (door->line->special & 6) == 6) || (P_NLSpecialXProp(door->line) & PNLXP_DOORLIGHT))
						EV_TurnTagLightsOff(door->line);
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
				LightArg = 0;
				if (P_XGSVal(PGS_COBOOMSUPPORT) && door->line && door->line->tag)
					if ((door->line->special > GenLockedBase && (door->line->special & 6) == 6) || (P_NLSpecialXProp(door->line) & PNLXP_DOORLIGHT))	//jff 3/9/98 all manual doors
						EV_LightTurnOn(door->line, -1, NULL, LAT_FROMLINE, 0, NULL, 1, &LightArg);
			}
			break;
	}
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
