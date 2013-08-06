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
//      Floor animation: raising stairs.

#include "p_spec.h"
#include "s_sound.h"
#include "p_demcmp.h"
#include "p_local.h"









// ==========================================================================
//                              FLOORS
// ==========================================================================

//
// Move a plane (floor or ceiling) and check for crushing
//
//SoM: I had to copy the entire function from Boom because it was causing errors.
result_e T_MovePlane(sector_t* sector, fixed_t speed, fixed_t dest, bool_t crush, int floorOrCeiling, int direction)
{
	bool_t flag;
	fixed_t lastpos;
	fixed_t destheight;			//jff 02/04/98 used to keep floors/ceilings
	
	// from moving thru each other
	
	switch (floorOrCeiling)
	{
		case 0:
			// Moving a floor
			switch (direction)
			{
				case -1:
					//SoM: 3/20/2000: Make splash when platform floor hits water
					if (P_XGSVal(PGS_COBOOMSUPPORT) && sector->heightsec != -1 && sector->altheightsec == 1)
					{
						if ((sector->floorheight - speed) <
						        sectors[sector->heightsec].floorheight && sector->floorheight > sectors[sector->heightsec].floorheight)
							S_StartSound((mobj_t*)&sector->soundorg, sfx_gloop);
					}
					// Moving a floor down
					if (sector->floorheight - speed < dest)
					{
						lastpos = sector->floorheight;
						sector->floorheight = dest;
						flag = P_CheckSector(sector, crush);
						if (flag == true && sector->numattached)
						{
							sector->floorheight = lastpos;
							P_CheckSector(sector, crush);
						}
						return pastdest;
					}
					else
					{
						lastpos = sector->floorheight;
						sector->floorheight -= speed;
						flag = P_CheckSector(sector, crush);
						if (flag == true && sector->numattached)
						{
							sector->floorheight = lastpos;
							P_CheckSector(sector, crush);
							return crushed;
						}
					}
					break;
					
				case 1:
					// Moving a floor up
					// keep floor from moving thru ceilings
					//SoM: 3/20/2000: Make splash when platform floor hits water
					if (P_XGSVal(PGS_COBOOMSUPPORT) && sector->heightsec != -1 && sector->altheightsec == 1)
					{
						if ((sector->floorheight + speed) >
						        sectors[sector->heightsec].floorheight && sector->floorheight < sectors[sector->heightsec].floorheight)
							S_StartSound((mobj_t*)&sector->soundorg, sfx_gloop);
					}
					destheight = (!P_XGSVal(PGS_COBOOMSUPPORT) || dest < sector->ceilingheight) ? dest : sector->ceilingheight;
					if (sector->floorheight + speed > destheight)
					{
						lastpos = sector->floorheight;
						sector->floorheight = destheight;
						flag = P_CheckSector(sector, crush);
						if (flag == true)
						{
							sector->floorheight = lastpos;
							P_CheckSector(sector, crush);
						}
						return pastdest;
					}
					else
					{
						// crushing is possible
						lastpos = sector->floorheight;
						sector->floorheight += speed;
						flag = P_CheckSector(sector, crush);
						if (flag == true)
						{
							if (!P_XGSVal(PGS_COBOOMSUPPORT))
							{
								if (crush == true)
									return crushed;
							}
							sector->floorheight = lastpos;
							P_CheckSector(sector, crush);
							return crushed;
						}
					}
					break;
			}
			break;
			
		case 1:
			// moving a ceiling
			switch (direction)
			{
				case -1:
					if (P_XGSVal(PGS_COBOOMSUPPORT) && sector->heightsec != -1 && sector->altheightsec == 1)
					{
						if ((sector->ceilingheight - speed) <
						        sectors[sector->heightsec].floorheight && sector->ceilingheight > sectors[sector->heightsec].floorheight)
							S_StartSound((mobj_t*)&sector->soundorg, sfx_gloop);
					}
					// moving a ceiling down
					// keep ceiling from moving thru floors
					destheight = (!P_XGSVal(PGS_COBOOMSUPPORT) || dest > sector->floorheight) ? dest : sector->floorheight;
					if (sector->ceilingheight - speed < destheight)
					{
						lastpos = sector->ceilingheight;
						sector->ceilingheight = destheight;
						flag = P_CheckSector(sector, crush);
						
						if (flag == true)
						{
							sector->ceilingheight = lastpos;
							P_CheckSector(sector, crush);
						}
						return pastdest;
					}
					else
					{
						// crushing is possible
						lastpos = sector->ceilingheight;
						sector->ceilingheight -= speed;
						flag = P_CheckSector(sector, crush);
						
						if (flag == true)
						{
							if (crush == true)
								return crushed;
							sector->ceilingheight = lastpos;
							P_CheckSector(sector, crush);
							return crushed;
						}
					}
					break;
					
				case 1:
					if (P_XGSVal(PGS_COBOOMSUPPORT) && sector->heightsec != -1 && sector->altheightsec == 1)
					{
						if ((sector->ceilingheight + speed) >
						        sectors[sector->heightsec].floorheight && sector->ceilingheight < sectors[sector->heightsec].floorheight)
							S_StartSound((mobj_t*)&sector->soundorg, sfx_gloop);
					}
					// moving a ceiling up
					if (sector->ceilingheight + speed > dest)
					{
						lastpos = sector->ceilingheight;
						sector->ceilingheight = dest;
						flag = P_CheckSector(sector, crush);
						if (flag == true && sector->numattached)
						{
							sector->ceilingheight = lastpos;
							P_CheckSector(sector, crush);
						}
						return pastdest;
					}
					else
					{
						lastpos = sector->ceilingheight;
						sector->ceilingheight += speed;
						flag = P_CheckSector(sector, crush);
						if (flag == true && sector->numattached)
						{
							sector->ceilingheight = lastpos;
							P_CheckSector(sector, crush);
							return crushed;
						}
					}
					break;
			}
			break;
	}
	return ok;
}

//
// MOVE A FLOOR TO IT'S DESTINATION (UP OR DOWN)
//
void T_MoveFloor(floormove_t* floor)
{
	result_e res = 0;
	
	res = T_MovePlane(floor->sector, floor->speed, floor->floordestheight, floor->crush, 0, floor->direction);
	
	if (!(leveltime % (8)))
		S_StartSound((mobj_t*)&floor->sector->soundorg, ceilmovesound);
		
	if (res == pastdest)
	{
		//floor->sector->specialdata = NULL;
		if (floor->direction == 1)
		{
			switch (floor->type)
			{
				case donutRaise:
					floor->sector->special = floor->newspecial;
					floor->sector->floorpic = floor->texture;
					break;
				case genFloorChgT:	//SoM: 3/6/2000: Add support for General types
				case genFloorChg0:
					floor->sector->special = floor->newspecial;
					//SoM: 3/6/2000: this records the old special of the sector
					floor->sector->oldspecial = floor->oldspecial;
					// Don't break.
				case genFloorChg:
					floor->sector->floorpic = floor->texture;
					break;
				default:
					break;
			}
		}
		else if (floor->direction == -1)
		{
			switch (floor->type)
			{
				case lowerAndChange:
					floor->sector->special = floor->newspecial;
					// SoM: 3/6/2000: Store old special type
					floor->sector->oldspecial = floor->oldspecial;
					floor->sector->floorpic = floor->texture;
					break;
				case genFloorChgT:
				case genFloorChg0:
					floor->sector->special = floor->newspecial;
					floor->sector->oldspecial = floor->oldspecial;
					// Don't break
				case genFloorChg:
					floor->sector->floorpic = floor->texture;
					break;
				default:
					break;
			}
		}
		
		floor->sector->floordata = NULL;	// Clear up the thinker so others can use it
		P_RemoveThinker(&floor->thinker);
		
		// SoM: This code locks out stair steps while generic, retriggerable generic stairs
		// are building.
		
		if (floor->sector->stairlock == -2)	// if this sector is stairlocked
		{
			sector_t* sec = floor->sector;
			
			sec->stairlock = -1;	// thinker done, promote lock to -1
			
			while (sec->prevsec != -1 && sectors[sec->prevsec].stairlock != -2)
				sec = &sectors[sec->prevsec];	// search for a non-done thinker
			if (sec->prevsec == -1)	// if all thinkers previous are done
			{
				sec = floor->sector;	// search forward
				while (sec->nextsec != -1 && sectors[sec->nextsec].stairlock != -2)
					sec = &sectors[sec->nextsec];
				if (sec->nextsec == -1)	// if all thinkers ahead are done too
				{
					while (sec->prevsec != -1)	// clear all locks
					{
						sec->stairlock = 0;
						sec = &sectors[sec->prevsec];
					}
					sec->stairlock = 0;
				}
			}
		}
		
		S_StartSound((mobj_t*)&floor->sector->soundorg, sfx_pstop);
	}
	
}

// SoM: 3/6/2000: Lots'o'copied code here.. Elevators.
//
// T_MoveElevator()
//
// Move an elevator to it's destination (up or down)
// Called once per tick for each moving floor.
//
// Passed an elevator_t structure that contains all pertinent info about the
// move. See P_SPEC.H for fields.
// No return.
//
// SoM: 3/6/2000: The function moves the plane differently based on direction, so if it's
// traveling really fast, the floor and ceiling won't hit each other and stop the lift.
// GhostlyDeath <June 8, 2012> -- Perpetual Elevator Enhanced
void T_MoveElevator(elevator_t* elevator)
{
	vldoor_t* NewDoor;
	result_e res = 0;
	bool_t StartMove, WaitingOnDoor;
	fixed_t LowFloor, HighFloor, ThisFloor;
	line_t* AdjLine;
	sector_t* AdjSector;
	int32_t i;
	
	/* Stop Elevator */
	if (elevator->type == EVGHEELEVT_STOPPERP)
	{
		elevator->sector->floordata = NULL;	//jff 2/22/98
		elevator->sector->ceilingdata = NULL;	//jff 2/22/98
	
		// make floor stop sound
		if (!elevator->Silent)
			S_StartSound((mobj_t*)&elevator->sector->soundorg, sfx_pstop);
	
		P_RemoveThinker(&elevator->thinker);	// remove elevator from actives
	}
	
	/* Moving Perpetual Elevator */
	else if (elevator->type == EVGHEELEVT_PERPUP || elevator->type == EVGHEELEVT_PERPDOWN)
	{
		// Perp Elevator Stopped
		if (elevator->direction == 0)
		{
			// Clear
			StartMove = false;
			WaitingOnDoor = false;
			
			// Elevator being called?
			if (elevator->CallLine)
			{
				// Always start moving
				StartMove = true;
			}
			
			// Normal elevator move
			else
			{
				// Still waiting?
				if (--elevator->PerpTicsLeft > 0)
					return;
				
				// Done waiting, start to move
				StartMove = true;
			}
			
			// Going to move elevator
			if (StartMove)
			{
				// Close any nearby doors and make sure they are closed too
				// If any doors are still open, then don't move yet
				for (i = 0; i < elevator->sector->linecount; i++)
				{
					// Get current line
					AdjLine = elevator->sector->lines[i];
					
					// Not part of an elevator
					if ((AdjLine->special & EVGENHE_TYPEMASK) != EVGENHE_TYPEBASE(EVGHET_XELEVATOR))
						continue;
					
					// Not an elevator door?
					if (!((AdjLine->special & ~(EVGENHE_TYPEMASK)) & EVGENGE_ELEVDOORMASK))
						continue;
					
					// Get Adjacent Sector (front or back side)
					if (AdjLine->frontsector == elevator->sector)
						AdjSector = AdjLine->backsector;
					else
						AdjSector = AdjLine->frontsector;
					
					// No sector on other side?
					if (!AdjSector)
						continue;
					
					// See if sector is closed (ceil = floor)
					if (AdjSector->ceilingheight == AdjSector->floorheight)
						continue;	// Don't bother it
					
					// Waiting on door
					WaitingOnDoor = true;
					
					// If door was not told to close, then close it
					if (!AdjSector->ceilingdata)
					{
						NewDoor = P_SpawnDoorCloseIn(AdjSector, 1, doorclose);
						NewDoor->direction = -1;
						NewDoor->speed = elevator->PDoorSpeed;
					}
				}
				
				// Ding bell to indicate moving
				if (!elevator->Silent)
					if (!elevator->Dinged)
					{
						S_StartSound((mobj_t*)&elevator->sector->soundorg, sfx_elvcal);
						elevator->Dinged = true;
					}
				
				// Waiting on a door to close
				if (WaitingOnDoor)
					return;
					
				// Was called
				if (elevator->CallLine)
				{
					// Move Down?
					if (elevator->sector->floorheight > elevator->CallLine->frontsector->floorheight)
						elevator->direction = -1;
					
					// Move Up?
					else if (elevator->sector->floorheight < elevator->CallLine->frontsector->floorheight)
						elevator->direction = 1;
					
					// Heights are simple
					elevator->floordestheight = elevator->CallLine->frontsector->floorheight;
					elevator->ceilingdestheight = elevator->floordestheight + (elevator->sector->ceilingheight - elevator->sector->floorheight);
				}
				
				// Resume normal direction
				else
				{
					// Get lowest and highest floors
					ThisFloor = elevator->sector->floorheight;
					LowFloor = P_FindNextLowestFloor(elevator->sector, elevator->sector->floorheight);
					HighFloor = P_FindNextHighestFloor(elevator->sector, elevator->sector->floorheight);
					
					// Determine position to move to
						// Continue moving down?
					if (elevator->OldDirection < 0)
					{
						if (LowFloor < ThisFloor)
							elevator->direction = -1;
						else
							elevator->direction = 1;	// Go up!
					}
						// Continue moving up?
					else
					{
						if (HighFloor > ThisFloor)
							elevator->direction = 1;
						else
							elevator->direction = -1;	// Go down!
					}
					
					// Start actually moving it
					if (elevator->direction < 0)
						elevator->floordestheight = LowFloor;
					else
						elevator->floordestheight = HighFloor;
					elevator->ceilingdestheight = elevator->floordestheight + (elevator->sector->ceilingheight - elevator->sector->floorheight);
				}
				
				// Clear calling line (moving to it, hopefully)
				elevator->CallLine = NULL;
			}
		}
		
		// Perp Elevator Moving
		else
		{
			// Down
			if (elevator->direction < 0)
			{
				res = T_MovePlane		//jff 4/7/98 reverse order of ceiling/floor
					  (elevator->sector, elevator->speed, elevator->ceilingdestheight, 0, 1,	// move floor
					   elevator->direction);
				if (res == ok || res == pastdest)	// jff 4/7/98 don't move ceil if blocked
					T_MovePlane(elevator->sector, elevator->speed, elevator->floordestheight, 0, 0,	// move ceiling
							    elevator->direction);
			}
			
			// Up
			else
			{
				res = T_MovePlane		//jff 4/7/98 reverse order of ceiling/floor
					  (elevator->sector, elevator->speed, elevator->floordestheight, 0, 0,	// move ceiling
					   elevator->direction);
				if (res == ok || res == pastdest)	// jff 4/7/98 don't move floor if blocked
					T_MovePlane(elevator->sector, elevator->speed, elevator->ceilingdestheight, 0, 1,	// move floor
							    elevator->direction);
			}
			
			// make floor move sound
			if (!elevator->Silent)
				if (!(leveltime % (8)))
					S_StartSound((mobj_t*)&elevator->sector->soundorg, sfx_stnmov);
					
			// Elevator reached destination
			if (res == pastdest)
			{
				// Ding the elevator bell
				if (!elevator->Silent)
					S_StartSound((mobj_t*)&elevator->sector->soundorg, sfx_elvcal);
				
				// Setup elevator wait
				elevator->OldDirection = elevator->direction;
				elevator->direction = 0;
				elevator->PerpTicsLeft = elevator->PerpWait;
				elevator->Dinged = false;
				
				// Clear Calling Line if at this level
				if (elevator->CallLine)
					if (elevator->CallLine->frontsector->floorheight == elevator->sector->floorheight)
						elevator->CallLine = NULL;
				
				// Open any nearby doors
				for (i = 0; i < elevator->sector->linecount; i++)
				{
					// Get current line
					AdjLine = elevator->sector->lines[i];
					
					// Not part of an elevator
					if ((AdjLine->special & EVGENHE_TYPEMASK) != EVGENHE_TYPEBASE(EVGHET_XELEVATOR))
						continue;
					
					// Not an elevator door?
					if (!((AdjLine->special & ~(EVGENHE_TYPEMASK)) & EVGENGE_ELEVDOORMASK))
						continue;
					
					// Get Adjacent Sector (front or back side)
					if (AdjLine->frontsector == elevator->sector)
						AdjSector = AdjLine->backsector;
					else
						AdjSector = AdjLine->frontsector;
					
					// No sector on other side?
					if (!AdjSector)
						continue;
					
					// Door is not on this floor?
					if (elevator->sector->floorheight != AdjSector->floorheight)
						continue;	// Don't bother it
					
					// Open the door
					NewDoor = P_SpawnDoorRaiseIn(AdjSector, false, 1, dooropen);
					NewDoor->direction = 1;
					NewDoor->speed = elevator->PDoorSpeed;
				}
			}
		}
	}
	
	/* Moving Generic Boom Elevator */
	else
	{
		if (elevator->direction < 0)	// moving down
		{
			res = T_MovePlane		//jff 4/7/98 reverse order of ceiling/floor
				  (elevator->sector, elevator->speed, elevator->ceilingdestheight, 0, 1,	// move floor
				   elevator->direction);
			if (res == ok || res == pastdest)	// jff 4/7/98 don't move ceil if blocked
				T_MovePlane(elevator->sector, elevator->speed, elevator->floordestheight, 0, 0,	// move ceiling
					        elevator->direction);
		}
		else						// up
		{
			res = T_MovePlane		//jff 4/7/98 reverse order of ceiling/floor
				  (elevator->sector, elevator->speed, elevator->floordestheight, 0, 0,	// move ceiling
				   elevator->direction);
			if (res == ok || res == pastdest)	// jff 4/7/98 don't move floor if blocked
				T_MovePlane(elevator->sector, elevator->speed, elevator->ceilingdestheight, 0, 1,	// move floor
					        elevator->direction);
		}
	
		// make floor move sound
		if (!elevator->Silent)
			if (!(leveltime % (8)))
				S_StartSound((mobj_t*)&elevator->sector->soundorg, sfx_stnmov);
	
		if (res == pastdest)		// if destination height acheived
		{
			elevator->sector->floordata = NULL;	//jff 2/22/98
			elevator->sector->ceilingdata = NULL;	//jff 2/22/98
		
			// make floor stop sound
			if (!elevator->Silent)
				S_StartSound((mobj_t*)&elevator->sector->soundorg, sfx_pstop);
		
			P_RemoveThinker(&elevator->thinker);	// remove elevator from actives
		}
	}
}

// SoM: 3/6/2000: Function for chaning just the floor texture and type.
//
// EV_DoChange()
//
// Handle pure change types. These change floor texture and sector type
// by trigger or numeric model without moving the floor.
//
// The linedef causing the change and the type of change is passed
// Returns true if any sector changes
//
int EV_DoChange(line_t* line, change_e changetype)
{
	int secnum;
	int rtn;
	sector_t* sec;
	sector_t* secm;
	
	secnum = -1;
	rtn = 0;
	// change all sectors with the same tag as the linedef
	while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
	{
		sec = &sectors[secnum];
		
		rtn = 1;
		
		// handle trigger or numeric change type
		switch (changetype)
		{
			case trigChangeOnly:
				sec->floorpic = line->frontsector->floorpic;
				sec->special = line->frontsector->special;
				sec->oldspecial = line->frontsector->oldspecial;
				break;
			case numChangeOnly:
				secm = P_FindModelFloorSector(sec->floorheight, secnum);
				if (secm)		// if no model, no change
				{
					sec->floorpic = secm->floorpic;
					sec->special = secm->special;
					sec->oldspecial = secm->oldspecial;
				}
				break;
			default:
				break;
		}
	}
	return rtn;
}


