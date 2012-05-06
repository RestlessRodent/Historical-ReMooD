// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION:
//  Generalized linedef type handlers
//  Floors, Ceilings, Doors, Locked Doors, Lifts, Stairs, Crushers

#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "r_data.h"
#include "m_random.h"
#include "s_sound.h"
#include "z_zone.h"
#include "p_spec.h"

/*
  SoM: 3/9/2000: Copied this entire file from Boom sources to Legacy sources.
  This file contains all routiens for Generalized linedef types.
*/

//
// EV_DoGenFloor()
//
// Handle generalized floor types
//
// Passed the line activating the generalized floor function
// Returns true if a thinker is created
//
int EV_DoGenFloor(line_t* line, mobj_t* const a_Object)
{
	int secnum;
	int rtn;
	bool_t manual;
	sector_t* sec;
	floormove_t* floor;
	unsigned value = (unsigned)line->special - GenFloorBase;
	
	// parse the bit fields in the line's special type
	
	int Crsh = (value & FloorCrush) >> FloorCrushShift;
	int ChgT = (value & FloorChange) >> FloorChangeShift;
	int Targ = (value & FloorTarget) >> FloorTargetShift;
	int Dirn = (value & FloorDirection) >> FloorDirectionShift;
	int ChgM = (value & FloorModel) >> FloorModelShift;
	int Sped = (value & FloorSpeed) >> FloorSpeedShift;
	int Trig = (value & TriggerType) >> TriggerTypeShift;
	
	rtn = 0;
	
	// check if a manual trigger, if so do just the sector on the backside
	manual = false;
	if (Trig == PushOnce || Trig == PushMany)
	{
		if (!(sec = line->backsector))
			return rtn;
		secnum = sec - sectors;
		manual = true;
		goto manual_floor;
	}
	
	secnum = -1;
	// if not manual do all sectors tagged the same as the line
	while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
	{
		sec = &sectors[secnum];
		
manual_floor:
		// Do not start another function if floor already moving
		if (P_SectorActive(floor_special, sec))
		{
			if (!manual)
				continue;
			else
				return rtn;
		}
		// new floor thinker
		rtn = 1;
		floor = Z_Malloc(sizeof(floormove_t), PU_LEVSPEC, 0);
		P_AddThinker(&floor->thinker);
		sec->floordata = floor;
		floor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
		floor->crush = Crsh;
		floor->direction = Dirn ? 1 : -1;
		floor->sector = sec;
		floor->texture = sec->floorpic;
		floor->newspecial = sec->special;
		floor->oldspecial = sec->oldspecial;
		floor->type = genFloor;
		
		// set the speed of motion
		switch (Sped)
		{
			case SpeedSlow:
				floor->speed = FLOORSPEED;
				break;
			case SpeedNormal:
				floor->speed = FLOORSPEED * 2;
				break;
			case SpeedFast:
				floor->speed = FLOORSPEED * 4;
				break;
			case SpeedTurbo:
				floor->speed = FLOORSPEED * 8;
				break;
			default:
				break;
		}
		
		// set the destination height
		switch (Targ)
		{
			case FtoHnF:
				floor->floordestheight = P_FindHighestFloorSurrounding(sec);
				break;
			case FtoLnF:
				floor->floordestheight = P_FindLowestFloorSurrounding(sec);
				break;
			case FtoNnF:
				floor->floordestheight = Dirn ? P_FindNextHighestFloor(sec, sec->floorheight) : P_FindNextLowestFloor(sec, sec->floorheight);
				break;
			case FtoLnC:
				floor->floordestheight = P_FindLowestCeilingSurrounding(sec);
				break;
			case FtoC:
				floor->floordestheight = sec->ceilingheight;
				break;
			case FbyST:
				floor->floordestheight = (floor->sector->floorheight >> FRACBITS) + floor->direction * (P_FindShortestTextureAround(secnum) >> FRACBITS);
				if (floor->floordestheight > 32000)
					floor->floordestheight = 32000;
				if (floor->floordestheight < -32000)
					floor->floordestheight = -32000;
				floor->floordestheight <<= FRACBITS;
				break;
			case Fby24:
				floor->floordestheight = floor->sector->floorheight + floor->direction * 24 * FRACUNIT;
				break;
			case Fby32:
				floor->floordestheight = floor->sector->floorheight + floor->direction * 32 * FRACUNIT;
				break;
			default:
				break;
		}
		
		// set texture/type change properties
		if (ChgT)				// if a texture change is indicated
		{
			if (ChgM)			// if a numeric model change
			{
				sector_t* sec;
				
				sec = (Targ == FtoLnC || Targ == FtoC) ?
				      P_FindModelCeilingSector(floor->floordestheight, secnum) : P_FindModelFloorSector(floor->floordestheight, secnum);
				if (sec)
				{
					floor->texture = sec->floorpic;
					switch (ChgT)
					{
						case FChgZero:	// zero type
							floor->newspecial = 0;
							floor->oldspecial = 0;
							floor->type = genFloorChg0;
							break;
						case FChgTyp:	// copy type
							floor->newspecial = sec->special;
							floor->oldspecial = sec->oldspecial;
							floor->type = genFloorChgT;
							break;
						case FChgTxt:	// leave type be
							floor->type = genFloorChg;
							break;
						default:
							break;
					}
				}
			}
			else				// else if a trigger model change
			{
				floor->texture = line->frontsector->floorpic;
				switch (ChgT)
				{
					case FChgZero:	// zero type
						floor->newspecial = 0;
						floor->oldspecial = 0;
						floor->type = genFloorChg0;
						break;
					case FChgTyp:	// copy type
						floor->newspecial = line->frontsector->special;
						floor->oldspecial = line->frontsector->oldspecial;
						floor->type = genFloorChgT;
						break;
					case FChgTxt:	// leave type be
						floor->type = genFloorChg;
					default:
						break;
				}
			}
		}
		if (manual)
			return rtn;
	}
	return rtn;
}

//
// EV_DoGenCeiling()
//
// Handle generalized ceiling types
//
// Passed the linedef activating the ceiling function
// Returns true if a thinker created
//
int EV_DoGenCeiling(line_t* line, mobj_t* const a_Object)
{
	int secnum;
	int rtn;
	bool_t manual;
	fixed_t targheight;
	sector_t* sec;
	ceiling_t* ceiling;
	unsigned value = (unsigned)line->special - GenCeilingBase;
	
	// parse the bit fields in the line's special type
	
	int Crsh = (value & CeilingCrush) >> CeilingCrushShift;
	int ChgT = (value & CeilingChange) >> CeilingChangeShift;
	int Targ = (value & CeilingTarget) >> CeilingTargetShift;
	int Dirn = (value & CeilingDirection) >> CeilingDirectionShift;
	int ChgM = (value & CeilingModel) >> CeilingModelShift;
	int Sped = (value & CeilingSpeed) >> CeilingSpeedShift;
	int Trig = (value & TriggerType) >> TriggerTypeShift;
	
	rtn = 0;
	
	// check if a manual trigger, if so do just the sector on the backside
	manual = false;
	if (Trig == PushOnce || Trig == PushMany)
	{
		if (!(sec = line->backsector))
			return rtn;
		secnum = sec - sectors;
		manual = true;
		goto manual_ceiling;
	}
	
	secnum = -1;
	// if not manual do all sectors tagged the same as the line
	while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
	{
		sec = &sectors[secnum];
		
manual_ceiling:
		// Do not start another function if ceiling already moving
		if (P_SectorActive(ceiling_special, sec))
		{
			if (!manual)
				continue;
			else
				return rtn;
		}
		// new ceiling thinker
		rtn = 1;
		ceiling = Z_Malloc(sizeof(ceiling_t), PU_LEVSPEC, 0);
		P_AddThinker(&ceiling->thinker);
		sec->ceilingdata = ceiling;
		ceiling->thinker.function.acp1 = (actionf_p1) T_MoveCeiling;
		ceiling->crush = Crsh;
		ceiling->direction = Dirn ? 1 : -1;
		ceiling->sector = sec;
		ceiling->texture = sec->ceilingpic;
		ceiling->newspecial = sec->special;
		ceiling->oldspecial = sec->oldspecial;
		ceiling->tag = sec->tag;
		ceiling->type = genCeiling;
		
		// set speed of motion
		switch (Sped)
		{
			case SpeedSlow:
				ceiling->speed = CEILSPEED;
				break;
			case SpeedNormal:
				ceiling->speed = CEILSPEED * 2;
				break;
			case SpeedFast:
				ceiling->speed = CEILSPEED * 4;
				break;
			case SpeedTurbo:
				ceiling->speed = CEILSPEED * 8;
				break;
			default:
				break;
		}
		
		// set destination target height
		targheight = sec->ceilingheight;
		switch (Targ)
		{
			case CtoHnC:
				targheight = P_FindHighestCeilingSurrounding(sec);
				break;
			case CtoLnC:
				targheight = P_FindLowestCeilingSurrounding(sec);
				break;
			case CtoNnC:
				targheight = Dirn ? P_FindNextHighestCeiling(sec, sec->ceilingheight) : P_FindNextLowestCeiling(sec, sec->ceilingheight);
				break;
			case CtoHnF:
				targheight = P_FindHighestFloorSurrounding(sec);
				break;
			case CtoF:
				targheight = sec->floorheight;
				break;
			case CbyST:
				targheight = (ceiling->sector->ceilingheight >> FRACBITS) + ceiling->direction * (P_FindShortestUpperAround(secnum) >> FRACBITS);
				if (targheight > 32000)
					targheight = 32000;
				if (targheight < -32000)
					targheight = -32000;
				targheight <<= FRACBITS;
				break;
			case Cby24:
				targheight = ceiling->sector->ceilingheight + ceiling->direction * 24 * FRACUNIT;
				break;
			case Cby32:
				targheight = ceiling->sector->ceilingheight + ceiling->direction * 32 * FRACUNIT;
				break;
			default:
				break;
		}
		//that doesn't compile under windows
		//Dirn? ceiling->topheight : ceiling->bottomheight = targheight;
		if (Dirn)
			ceiling->topheight = targheight;
		else
			ceiling->bottomheight = targheight;
			
		// set texture/type change properties
		if (ChgT)				// if a texture change is indicated
		{
			if (ChgM)			// if a numeric model change
			{
				sector_t* sec;
				
				sec = (Targ == CtoHnF || Targ == CtoF) ? P_FindModelFloorSector(targheight, secnum) : P_FindModelCeilingSector(targheight, secnum);
				if (sec)
				{
					ceiling->texture = sec->ceilingpic;
					switch (ChgT)
					{
						case CChgZero:	// type is zeroed
							ceiling->newspecial = 0;
							ceiling->oldspecial = 0;
							ceiling->type = genCeilingChg0;
							break;
						case CChgTyp:	// type is copied
							ceiling->newspecial = sec->special;
							ceiling->oldspecial = sec->oldspecial;
							ceiling->type = genCeilingChgT;
							break;
						case CChgTxt:	// type is left alone
							ceiling->type = genCeilingChg;
							break;
						default:
							break;
					}
				}
			}
			else				// else if a trigger model change
			{
				ceiling->texture = line->frontsector->ceilingpic;
				switch (ChgT)
				{
					case CChgZero:	// type is zeroed
						ceiling->newspecial = 0;
						ceiling->oldspecial = 0;
						ceiling->type = genCeilingChg0;
						break;
					case CChgTyp:	// type is copied
						ceiling->newspecial = line->frontsector->special;
						ceiling->oldspecial = line->frontsector->oldspecial;
						ceiling->type = genCeilingChgT;
						break;
					case CChgTxt:	// type is left alone
						ceiling->type = genCeilingChg;
						break;
					default:
						break;
				}
			}
		}
		P_AddActiveCeiling(ceiling);	// add this ceiling to the active list
		if (manual)
			return rtn;
	}
	return rtn;
}

//
// EV_DoGenLift()
//
// Handle generalized lift types
//
// Passed the linedef activating the lift
// Returns true if a thinker is created
//
int EV_DoGenLift(line_t* line, mobj_t* const a_Object)
{
	plat_t* plat;
	int secnum;
	int rtn;
	bool_t manual;
	sector_t* sec;
	unsigned value = (unsigned)line->special - GenLiftBase;
	
	// parse the bit fields in the line's special type
	
	int Targ = (value & LiftTarget) >> LiftTargetShift;
	int Dely = (value & LiftDelay) >> LiftDelayShift;
	int Sped = (value & LiftSpeed) >> LiftSpeedShift;
	int Trig = (value & TriggerType) >> TriggerTypeShift;
	
	secnum = -1;
	rtn = 0;
	
	// Activate all <type> plats that are in_stasis
	
	if (Targ == LnF2HnF)
		P_ActivateInStasis(line->tag);
		
	// check if a manual trigger, if so do just the sector on the backside
	manual = false;
	if (Trig == PushOnce || Trig == PushMany)
	{
		if (!(sec = line->backsector))
			return rtn;
		secnum = sec - sectors;
		manual = true;
		goto manual_lift;
	}
	// if not manual do all sectors tagged the same as the line
	while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
	{
		sec = &sectors[secnum];
		
manual_lift:
		// Do not start another function if floor already moving
		if (P_SectorActive(floor_special, sec))
		{
			if (!manual)
				continue;
			else
				return rtn;
		}
		// Setup the plat thinker
		rtn = 1;
		plat = Z_Malloc(sizeof(*plat), PU_LEVSPEC, 0);
		P_AddThinker(&plat->thinker);
		
		plat->sector = sec;
		plat->sector->floordata = plat;
		plat->thinker.function.acp1 = (actionf_p1) T_PlatRaise;
		plat->crush = false;
		plat->tag = line->tag;
		
		plat->type = genLift;
		plat->high = sec->floorheight;
		plat->status = down;
		
		// setup the target destination height
		switch (Targ)
		{
			case F2LnF:
				plat->low = P_FindLowestFloorSurrounding(sec);
				if (plat->low > sec->floorheight)
					plat->low = sec->floorheight;
				break;
			case F2NnF:
				plat->low = P_FindNextLowestFloor(sec, sec->floorheight);
				break;
			case F2LnC:
				plat->low = P_FindLowestCeilingSurrounding(sec);
				if (plat->low > sec->floorheight)
					plat->low = sec->floorheight;
				break;
			case LnF2HnF:
				plat->type = genPerpetual;
				plat->low = P_FindLowestFloorSurrounding(sec);
				if (plat->low > sec->floorheight)
					plat->low = sec->floorheight;
				plat->high = P_FindHighestFloorSurrounding(sec);
				if (plat->high < sec->floorheight)
					plat->high = sec->floorheight;
				plat->status = P_Random() & 1;
				break;
			default:
				break;
		}
		
		// setup the speed of motion
		switch (Sped)
		{
			case SpeedSlow:
				plat->speed = PLATSPEED * 2;
				break;
			case SpeedNormal:
				plat->speed = PLATSPEED * 4;
				break;
			case SpeedFast:
				plat->speed = PLATSPEED * 8;
				break;
			case SpeedTurbo:
				plat->speed = PLATSPEED * 16;
				break;
			default:
				break;
		}
		
		// setup the delay time before the floor returns
		switch (Dely)
		{
			case 0:
				plat->wait = 1 * TICRATE;
				break;
			case 1:
				plat->wait = PLATWAIT * TICRATE;
				break;
			case 2:
				plat->wait = 5 * TICRATE;
				break;
			case 3:
				plat->wait = 10 * TICRATE;
				break;
		}
		
		S_StartSound((S_NoiseThinker_t*)&sec->soundorg, sfx_pstart);
		P_AddActivePlat(plat);	// add this plat to the list of active plats
		
		if (manual)
			return rtn;
	}
	return rtn;
}

//
// EV_DoGenStairs()
//
// Handle generalized stair building
//
// Passed the linedef activating the stairs
// Returns true if a thinker is created
//
int EV_DoGenStairs(line_t* line, mobj_t* const a_Object)
{
	int secnum;
	int osecnum;
	int height;
	int i;
	int newsecnum;
	int texture;
	int ok;
	int rtn;
	bool_t manual;
	
	sector_t* sec;
	sector_t* tsec;
	
	floormove_t* floor;
	
	fixed_t stairsize;
	fixed_t speed;
	
	unsigned value = (unsigned)line->special - GenStairsBase;
	
	// parse the bit fields in the line's special type
	
	int Igno = (value & StairIgnore) >> StairIgnoreShift;
	int Dirn = (value & StairDirection) >> StairDirectionShift;
	int Step = (value & StairStep) >> StairStepShift;
	int Sped = (value & StairSpeed) >> StairSpeedShift;
	int Trig = (value & TriggerType) >> TriggerTypeShift;
	
	rtn = 0;
	
	// check if a manual trigger, if so do just the sector on the backside
	manual = false;
	if (Trig == PushOnce || Trig == PushMany)
	{
		if (!(sec = line->backsector))
			return rtn;
		secnum = sec - sectors;
		manual = true;
		goto manual_stair;
	}
	
	secnum = -1;
	// if not manual do all sectors tagged the same as the line
	while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
	{
		sec = &sectors[secnum];
		
manual_stair:
		//Do not start another function if floor already moving
		//Add special lockout condition to wait for entire
		//staircase to build before retriggering
		if (P_SectorActive(floor_special, sec) || sec->stairlock)
		{
			if (!manual)
				continue;
			else
				return rtn;
		}
		// new floor thinker
		rtn = 1;
		floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);
		P_AddThinker(&floor->thinker);
		sec->floordata = floor;
		floor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
		floor->direction = Dirn ? 1 : -1;
		floor->sector = sec;
		
		// setup speed of stair building
		switch (Sped)
		{
			default:
			case SpeedSlow:
				floor->speed = FLOORSPEED / 4;
				break;
			case SpeedNormal:
				floor->speed = FLOORSPEED / 2;
				break;
			case SpeedFast:
				floor->speed = FLOORSPEED * 2;
				break;
			case SpeedTurbo:
				floor->speed = FLOORSPEED * 4;
				break;
		}
		
		// setup stepsize for stairs
		switch (Step)
		{
			default:
			case 0:
				stairsize = 4 * FRACUNIT;
				break;
			case 1:
				stairsize = 8 * FRACUNIT;
				break;
			case 2:
				stairsize = 16 * FRACUNIT;
				break;
			case 3:
				stairsize = 24 * FRACUNIT;
				break;
		}
		
		speed = floor->speed;
		height = sec->floorheight + floor->direction * stairsize;
		floor->floordestheight = height;
		texture = sec->floorpic;
		floor->crush = false;
		floor->type = genBuildStair;
		
		sec->stairlock = -2;
		sec->nextsec = -1;
		sec->prevsec = -1;
		
		osecnum = secnum;
		// Find next sector to raise
		// 1.     Find 2-sided line with same sector side[0]
		// 2.     Other side is the next sector to raise
		do
		{
			ok = 0;
			for (i = 0; i < sec->linecount; i++)
			{
				if (!((sec->lines[i])->backsector))
					continue;
					
				tsec = (sec->lines[i])->frontsector;
				newsecnum = tsec - sectors;
				
				if (secnum != newsecnum)
					continue;
					
				tsec = (sec->lines[i])->backsector;
				newsecnum = tsec - sectors;
				
				if (!Igno && tsec->floorpic != texture)
					continue;
					
				if (!boomsupport)
					height += floor->direction * stairsize;
					
				if (P_SectorActive(floor_special, tsec) || tsec->stairlock)
					continue;
					
				if (boomsupport)
					height += floor->direction * stairsize;
					
				// link the stair chain in both directions
				// lock the stair sector until building complete
				sec->nextsec = newsecnum;	// link step to next
				tsec->prevsec = secnum;	// link next back
				tsec->nextsec = -1;	// set next forward link as end
				tsec->stairlock = -2;	// lock the step
				
				sec = tsec;
				secnum = newsecnum;
				floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);
				
				P_AddThinker(&floor->thinker);
				
				sec->floordata = floor;
				floor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
				floor->direction = Dirn ? 1 : -1;
				floor->sector = sec;
				floor->speed = speed;
				floor->floordestheight = height;
				floor->crush = false;
				floor->type = genBuildStair;
				
				ok = 1;
				break;
			}
		}
		while (ok);
		if (manual)
			return rtn;
		secnum = osecnum;
	}
	// retriggerable generalized stairs build up or down alternately
	if (rtn)
		line->special ^= StairDirection;	// alternate dir on succ activations
	return rtn;
}

//
// EV_DoGenCrusher()
//
// Handle generalized crusher types
//
// Passed the linedef activating the crusher
// Returns true if a thinker created
//
int EV_DoGenCrusher(line_t* line, mobj_t* const a_Object)
{
	int secnum;
	int rtn;
	bool_t manual;
	sector_t* sec;
	ceiling_t* ceiling;
	unsigned value = (unsigned)line->special - GenCrusherBase;
	
	// parse the bit fields in the line's special type
	
	int Slnt = (value & CrusherSilent) >> CrusherSilentShift;
	int Sped = (value & CrusherSpeed) >> CrusherSpeedShift;
	int Trig = (value & TriggerType) >> TriggerTypeShift;
	
	rtn = P_ActivateInStasisCeiling(line);
	
	// check if a manual trigger, if so do just the sector on the backside
	manual = false;
	if (Trig == PushOnce || Trig == PushMany)
	{
		if (!(sec = line->backsector))
			return rtn;
		secnum = sec - sectors;
		manual = true;
		goto manual_crusher;
	}
	
	secnum = -1;
	// if not manual do all sectors tagged the same as the line
	while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
	{
		sec = &sectors[secnum];
		
manual_crusher:
		// Do not start another function if ceiling already moving
		if (P_SectorActive(ceiling_special, sec))
		{
			if (!manual)
				continue;
			else
				return rtn;
		}
		// new ceiling thinker
		rtn = 1;
		ceiling = Z_Malloc(sizeof(*ceiling), PU_LEVSPEC, 0);
		P_AddThinker(&ceiling->thinker);
		sec->ceilingdata = ceiling;
		ceiling->thinker.function.acp1 = (actionf_p1) T_MoveCeiling;
		ceiling->crush = true;
		ceiling->direction = -1;
		ceiling->sector = sec;
		ceiling->texture = sec->ceilingpic;
		ceiling->newspecial = sec->special;
		ceiling->tag = sec->tag;
		ceiling->type = Slnt ? genSilentCrusher : genCrusher;
		ceiling->topheight = sec->ceilingheight;
		ceiling->bottomheight = sec->floorheight + (8 * FRACUNIT);
		
		// setup ceiling motion speed
		switch (Sped)
		{
			case SpeedSlow:
				ceiling->speed = CEILSPEED;
				break;
			case SpeedNormal:
				ceiling->speed = CEILSPEED * 2;
				break;
			case SpeedFast:
				ceiling->speed = CEILSPEED * 4;
				break;
			case SpeedTurbo:
				ceiling->speed = CEILSPEED * 8;
				break;
			default:
				break;
		}
		ceiling->oldspeed = ceiling->speed;
		
		P_AddActiveCeiling(ceiling);	// add to list of active ceilings
		if (manual)
			return rtn;
	}
	return rtn;
}

//
// EV_DoGenLockedDoor()
//
// Handle generalized locked door types
//
// Passed the linedef activating the generalized locked door
// Returns true if a thinker created
//
int EV_DoGenLockedDoor(line_t* line, mobj_t* const a_Object)
{
	int secnum, rtn;
	sector_t* sec;
	vldoor_t* door;
	bool_t manual;
	unsigned value = (unsigned)line->special - GenLockedBase;
	
	// parse the bit fields in the line's special type
	
	int Kind = (value & LockedKind) >> LockedKindShift;
	int Sped = (value & LockedSpeed) >> LockedSpeedShift;
	int Trig = (value & TriggerType) >> TriggerTypeShift;
	
	rtn = 0;
	
	// check if a manual trigger, if so do just the sector on the backside
	manual = false;
	if (Trig == PushOnce || Trig == PushMany)
	{
		if (!(sec = line->backsector))
			return rtn;
		secnum = sec - sectors;
		manual = true;
		goto manual_locked;
	}
	
	secnum = -1;
	rtn = 0;
	
	// if not manual do all sectors tagged the same as the line
	while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
	{
		sec = &sectors[secnum];
manual_locked:
		// Do not start another function if ceiling already moving
		if (P_SectorActive(ceiling_special, sec))
		{
			if (!manual)
				continue;
			else
				return rtn;
		}
		// new door thinker
		rtn = 1;
		door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
		P_AddThinker(&door->thinker);
		sec->ceilingdata = door;
		
		door->thinker.function.acp1 = (actionf_p1) T_VerticalDoor;
		door->sector = sec;
		door->topwait = VDOORWAIT;
		door->line = line;
		door->topheight = P_FindLowestCeilingSurrounding(sec);
		door->topheight -= 4 * FRACUNIT;
		door->direction = 1;
		
		// setup speed of door motion
		switch (Sped)
		{
			default:
			case SpeedSlow:
				door->type = Kind ? genOpen : genRaise;
				door->speed = VDOORSPEED;
				break;
			case SpeedNormal:
				door->type = Kind ? genOpen : genRaise;
				door->speed = VDOORSPEED * 2;
				break;
			case SpeedFast:
				door->type = Kind ? genBlazeOpen : genBlazeRaise;
				door->speed = VDOORSPEED * 4;
				break;
			case SpeedTurbo:
				door->type = Kind ? genBlazeOpen : genBlazeRaise;
				door->speed = VDOORSPEED * 8;
				
				break;
		}
		
		// killough 4/15/98: fix generalized door opening sounds
		// (previously they always had the blazing door close sound)
		
		S_StartSound((S_NoiseThinker_t*)&door->sector->soundorg,	// killough 4/15/98
		             door->speed >= VDOORSPEED * 4 ? sfx_bdopn : sfx_doropn);
		             
		if (manual)
			return rtn;
	}
	return rtn;
}

//
// EV_DoGenDoor()
//
// Handle generalized door types
//
// Passed the linedef activating the generalized door
// Returns true if a thinker created
//
int EV_DoGenDoor(line_t* line, mobj_t* const a_Object)
{
	int secnum, rtn;
	sector_t* sec;
	bool_t manual;
	vldoor_t* door;
	unsigned value = (unsigned)line->special - GenDoorBase;
	
	// parse the bit fields in the line's special type
	
	int Dely = (value & DoorDelay) >> DoorDelayShift;
	int Kind = (value & DoorKind) >> DoorKindShift;
	int Sped = (value & DoorSpeed) >> DoorSpeedShift;
	int Trig = (value & TriggerType) >> TriggerTypeShift;
	
	rtn = 0;
	
	// check if a manual trigger, if so do just the sector on the backside
	manual = false;
	if (Trig == PushOnce || Trig == PushMany)
	{
		if (!(sec = line->backsector))
			return rtn;
		secnum = sec - sectors;
		manual = true;
		goto manual_door;
	}
	
	secnum = -1;
	rtn = 0;
	
	// if not manual do all sectors tagged the same as the line
	while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
	{
		sec = &sectors[secnum];
manual_door:
		// Do not start another function if ceiling already moving
		if (P_SectorActive(ceiling_special, sec))
		{
			if (!manual)
				continue;
			else
				return rtn;
		}
		// new door thinker
		rtn = 1;
		door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
		P_AddThinker(&door->thinker);
		sec->ceilingdata = door;
		
		door->thinker.function.acp1 = (actionf_p1) T_VerticalDoor;
		door->sector = sec;
		// setup delay for door remaining open/closed
		switch (Dely)
		{
			default:
			case 0:
				door->topwait = 35;
				break;
			case 1:
				door->topwait = VDOORWAIT;
				break;
			case 2:
				door->topwait = 2 * VDOORWAIT;
				break;
			case 3:
				door->topwait = 7 * VDOORWAIT;
				break;
		}
		
		// setup speed of door motion
		switch (Sped)
		{
			default:
			case SpeedSlow:
				door->speed = VDOORSPEED;
				break;
			case SpeedNormal:
				door->speed = VDOORSPEED * 2;
				break;
			case SpeedFast:
				door->speed = VDOORSPEED * 4;
				break;
			case SpeedTurbo:
				door->speed = VDOORSPEED * 8;
				break;
		}
		door->line = line;
		
		// set kind of door, whether it opens then close, opens, closes etc.
		// assign target heights accordingly
		switch (Kind)
		{
			case OdCDoor:
				door->direction = 1;
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				if (door->topheight != sec->ceilingheight)
					S_StartSound((S_NoiseThinker_t*)&door->sector->soundorg,	// killough 4/15/98
						door->speed >= VDOORSPEED * 4 ? sfx_bdopn : sfx_doropn);
				door->type = Sped >= SpeedFast ? genBlazeRaise : genRaise;
				break;
			case ODoor:
				door->direction = 1;
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				if (door->topheight != sec->ceilingheight)
					S_StartSound((S_NoiseThinker_t*)&door->sector->soundorg,	// killough 4/15/98
						door->speed >= VDOORSPEED * 4 ? sfx_bdopn : sfx_doropn);
				door->type = Sped >= SpeedFast ? genBlazeOpen : genOpen;
				break;
			case CdODoor:
				door->topheight = sec->ceilingheight;
				door->direction = -1;
				S_StartSound((S_NoiseThinker_t*)&door->sector->soundorg, sfx_dorcls);
				door->type = Sped >= SpeedFast ? genBlazeCdO : genCdO;
				break;
			case CDoor:
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				door->direction = -1;
				S_StartSound((S_NoiseThinker_t*)&door->sector->soundorg, sfx_dorcls);
				door->type = Sped >= SpeedFast ? genBlazeClose : genClose;
				break;
			default:
				break;
		}
		
		if (manual)
			return rtn;
	}
	return rtn;
}

/****************************************************************************/

/* EV_HExDoGenPlat() -- Do high extended generalized platform */
bool_t EV_HExDoGenPlat(line_t* const a_Line, mobj_t* const a_Object)
{
	plat_t* NewPlat;
	EV_GenHEFType_t HEFType;
	EV_GenHESpeed_t HEFSpeed;
	EV_GenHEFCDWait_t HEFWait;
	floorchange_e HEFMode;
	bool_t Ret, StnMove;
	int SectorNum;
	sector_t* CurSec;
	
	/* Check */
	if (!a_Line)
		return false;
	
	/* Init */
	Ret = false;
	StnMove = false;
	
	/* Extract Variables */
	HEFSpeed = (a_Line->special & EVGENGE_SPEEDMASK) >> EVGENGE_SPEEDSHIFT;
	HEFType = (a_Line->special & EVGENGE_FCPTYPEMASK) >> EVGENGE_FCPTYPESHIFT;
	HEFWait = (a_Line->special & EVGENGE_FCDWAITMASK) >> EVGENGE_FCDWAITSHIFT;
	HEFMode = (a_Line->special & EVGENGE_FCPMODEMASK) >> EVGENGE_FCPMODESHIFT;
		
	/* Activate plats and such in status */
	if (HEFType == EVGHEPLATT_LHFPERP || HEFType == EVGHEPLATT_CEILTOGGLE)
	{
		P_ActivateInStasis(a_Line->tag);
		
		// Toggle?
		if (HEFType == EVGHEPLATT_CEILTOGGLE)
			Ret = true;
	}
	
	/* Create plats when needed */
	SectorNum = -1; 
	
	// Run through everything
	while ((SectorNum = P_FindSectorFromLineTag(a_Line, SectorNum)) >= 0)
	{
		// Get current sector
		CurSec = &sectors[SectorNum];
		
		// If the sector is active, ignore it
		if (P_SectorActive(floor_special, CurSec))
			continue;
		
		// Create new plat here
		Ret = true;
		NewPlat = Z_Malloc(sizeof(*NewPlat), PU_LEVSPEC, 0);
		P_AddThinker(&NewPlat->thinker);
		
		// Base plat stuff
		NewPlat->sector = CurSec;
		NewPlat->sector->floordata = NewPlat;
		NewPlat->thinker.function.acp1 = (actionf_p1)T_PlatRaise;
		NewPlat->tag = a_Line->tag;
		NewPlat->low = CurSec->floorheight;
		
		// Determine plat type
			// Perpetual Lift
		if (HEFType == EVGHEPLATT_LHFPERP)
		{
			NewPlat->type = perpetualRaise;
			
			// Lower Area
			NewPlat->low = P_FindLowestFloorSurrounding(CurSec);
			if (NewPlat->low > CurSec->floorheight)
				NewPlat->low = CurSec->floorheight;
				
			// Higher Area
			NewPlat->high = P_FindHighestFloorSurrounding(CurSec);
			if (NewPlat->high < CurSec->floorheight)
				NewPlat->high = CurSec->floorheight;
			
			// Randomize up/down
			NewPlat->status = P_Random() & 1;
			
		}
			// Floor/Ceiling Toggle
		else if (HEFType == EVGHEPLATT_CEILTOGGLE)
		{
			NewPlat->type = toggleUpDn;
			
			NewPlat->crush = true;
			NewPlat->low = CurSec->ceilingheight;
			NewPlat->high = CurSec->floorheight;
			NewPlat->status = down;
		}
			// Down, Wait, Up, Stay (A Lift)
		else if (HEFType == EVGHEPLATT_DWUS)
		{
			NewPlat->type = downWaitUpStay;
			
			// Find where to drop to
			NewPlat->low = P_FindLowestFloorSurrounding(CurSec);
			if (NewPlat->low > CurSec->floorheight)
				NewPlat->low = CurSec->floorheight;
				
			NewPlat->high = CurSec->floorheight;
			NewPlat->status = down;
		}
			// Raise And Change
		else if (HEFType == EVGHEPLATT_RAISE32 || HEFType == EVGHEPLATT_RAISE24)
		{
			NewPlat->type = raiseAndChange;
			
			NewPlat->high = CurSec->floorheight +
				(HEFType == EVGHEPLATT_RAISE32 ? 32 : 24) * FRACUNIT;
			NewPlat->status = up;
			
			StnMove = true;
		}
			// Next Nearest and Change
		else if (HEFType == EVGHEPLATT_NNF)
		{
			NewPlat->type = raiseToNearestAndChange;
			
			NewPlat->high = P_FindNextHighestFloor(CurSec, CurSec->floorheight);
			NewPlat->status = up;
			
			StnMove = true;
		}
			// Unknown
		else
			NewPlat->type = 0;
		
		// Texture/Type changing
		if (HEFMode == FChgTxt || HEFMode == FChgZero)
			CurSec->floorpic = sides[a_Line->sidenum[0]].sector->floorpic;
			
		// NO MORE DAMAGE, IF APPLICABLE
		if (HEFMode == FChgZero)
		{
			CurSec->special = 0;
			CurSec->oldspecial = 0;	//SoM: 3/7/2000: Clear old field.
		}
		
		// Speed
		if (HEFSpeed == EVGHES_SLOWEST)
			NewPlat->speed = PLATSPEED / 8;
		else if (HEFSpeed == EVGHES_SLOWER)
			NewPlat->speed = PLATSPEED / 4;
		else if (HEFSpeed == EVGHES_SLOW)
			NewPlat->speed = PLATSPEED / 2;
		else if (HEFSpeed == EVGHES_NORMAL)
			NewPlat->speed = PLATSPEED;
		else if (HEFSpeed == EVGHES_FAST)
			NewPlat->speed = PLATSPEED * 2;
		else if (HEFSpeed == EVGHES_FASTER)
			NewPlat->speed = PLATSPEED * 4;
		else if (HEFSpeed == EVGHES_FASTEST)
			NewPlat->speed = PLATSPEED * 8;
		
		// Wait
		if (HEFType == EVGHEPLATT_LHFPERP || HEFType == EVGHEPLATT_CEILTOGGLE)
		{
			if (HEFWait == EVGHEFCDW_WAIT1S)
				NewPlat->wait = 1 * TICRATE;
			else if (HEFWait == EVGHEFCDW_WAIT3S)
				NewPlat->wait = 3 * TICRATE;
			else if (HEFWait == EVGHEFCDW_WAIT4S)
				NewPlat->wait = 4 * TICRATE;
			else if (HEFWait == EVGHEFCDW_WAIT5S)
				NewPlat->wait = 5 * TICRATE;
			else if (HEFWait == EVGHEFCDW_WAIT9S)
				NewPlat->wait = 9 * TICRATE;
			else if (HEFWait == EVGHEFCDW_WAIT10S)
				NewPlat->wait = 10 * TICRATE;
			else if (HEFWait == EVGHEFCDW_WAIT30S)
				NewPlat->wait = 30 * TICRATE;
			else if (HEFWait == EVGHEFCDW_WAIT60S)
				NewPlat->wait = 60 * TICRATE;
		}
		
		// Make noise
		S_StartSound((S_NoiseThinker_t*)&CurSec->soundorg, (StnMove ? sfx_stnmov : sfx_pstart));
		
		// Add this plat
		P_AddActivePlat(NewPlat);
	}
	
	/* Activated */
	return Ret;
#if 0
		
		switch (type)
		{
				break;
				
				
				
			default:
				break;
		}
		P_AddActivePlat(plat);
	}
	return rtn;
#endif
}

/****************************************************************************/

/* EV_TryGenTrigger() -- Tries to trigger a line */
bool_t EV_TryGenTrigger(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain)
{
	triggertype_e TrigMode;
	uint32_t TypeBase;
	
	/* Check */
	if (!a_Line)
		return false;
	
	/* Ignore Non-Special Lines */
	if (a_Line->special == 0)
		return false;
	
	/* Debug */
	if (devparm)
		CONL_PrintF("Trig %p by %p (side %+1i): Via %c, %8x\n", a_Line, a_Object, a_Side, (a_Type == EVTGT_WALK ? 'W' : (a_Type == EVTGT_SHOOT ? 'G' : 'S')), a_Line->special);
	
	/* Standard Boom Types */
	// These are available in Boom
	if (a_Line->special >= 0x2F80 && a_Line->special <= 0x7FFF)
	{
		// Determine the current trigger type
		TrigMode = (a_Line->special & TriggerType) >> TriggerTypeShift;
		
		// Trigger only once?
		*a_UseAgain = (TrigMode & 1);
		
		// Lose that bit
		TrigMode &= ~1;
		
		// Determine trigger compatibility
		if ((TrigMode == WalkOnce && a_Type != EVTGT_WALK) ||
			((TrigMode == PushOnce || TrigMode == SwitchOnce) && a_Type != EVTGT_SWITCH) ||
			(TrigMode == GunOnce && a_Type != EVTGT_SHOOT) ||
			(a_Type == EVTGT_MAPSTART))
			return false;
		
		// Generic Floors
		if (a_Line->special >= GenFloorBase)
		{
			// Check for Monster trigger
			if (!a_Object->player)
				if (((a_Line->special & FloorChange) || !(a_Line->special & FloorModel)) && !(a_Flags & EVTGTF_FORCEUSE))
					return false;
			
			// Non-manual push
			if (!a_Line->tag && TrigMode != PushOnce)
				return false;
			
			// Do the command and return
			return EV_DoGenFloor(a_Line, a_Object);
		}
		
		// Generic Ceilings
		else if (a_Line->special >= GenCeilingBase)
		{
			// Check for monster
			if (!a_Object->player)
				if (((a_Line->special & CeilingChange) || !(a_Line->special & CeilingModel)) && !(a_Flags & EVTGTF_FORCEUSE))
					return false;	// CeilingModel is "Allow Monsters" if CeilingChange is 0
			
			// Only accept push w/o tag
			if (!a_Line->tag && TrigMode != PushOnce)	//all non-manual
				return false;	//generalized types require tag
			
			return EV_DoGenCeiling(a_Line, a_Object);
		}
		
		// Generic Doors
		else if (a_Line->special >= GenDoorBase)
		{
			// Check for monster
			if (!a_Object->player)
			{
				if (!(a_Line->special & DoorMonster) && !(a_Flags & EVTGTF_FORCEUSE))
					return;		// monsters disallowed from this door
				
				if (a_Line->flags & ML_SECRET)	// they can't open secret doors either
					return;
			}
			
			// Only accept push w/o tag
			if (!a_Line->tag && TrigMode != PushOnce)	//all non-manual
				return false;	//generalized types require tag
			
			return EV_DoGenDoor(a_Line, a_Object);
		}
		
		// Generic Locked Doors
		else if (a_Line->special >= GenLockedBase)
		{
			// Players Only
			if (!a_Object->player)
				return false;
			
			// No keys to unlock door
			if (!P_CanUnlockGenDoor(a_Line, a_Object->player))
				return false;
			
			// Only accept push w/o tag
			if (!a_Line->tag && TrigMode != PushOnce)	//all non-manual
				return false;	//generalized types require tag
			
			return EV_DoGenLockedDoor(a_Line, a_Object);
		}
		
		// Generic Lifts
		else if (a_Line->special >= GenLiftBase)
		{
			// Check for Monster trigger
			if (!a_Object->player)
				if ((!(a_Line->special & LiftMonster)) && !(a_Flags & EVTGTF_FORCEUSE))
					return false;
			
			// Gunshots and pushable don't need tags
			if (!a_Line->tag && (TrigMode != PushOnce && TrigMode != GunOnce))
				return false;
			
			// Do the command and return
			return EV_DoGenLift(a_Line, a_Object);
		}
		
		// Generic Stairs
		else if (a_Line->special >= GenStairsBase)
		{
			// Check for Monster trigger
			if (!a_Object->player)
				if ((!(a_Line->special & StairMonster)) && !(a_Flags & EVTGTF_FORCEUSE))
					return false;
			
			// Tag Required
			if (!a_Line->tag && TrigMode != PushOnce)	//all non-manual
				return false;
			
			// Do the command and return
			return EV_DoGenStairs(a_Line, a_Object);
		}
		
		// Generic Crushers
		else if (a_Line->special >= GenCrusherBase)
		{
			// Check for Monster trigger
			if (!a_Object->player)
				if ((!(a_Line->special & CrusherMonster)) && !(a_Flags & EVTGTF_FORCEUSE))
					return false;
			
			// Tag Required
			if (!a_Line->tag && TrigMode != PushOnce)	//all non-manual
				return false;
				
			// Do the command and return
			return EV_DoGenCrusher(a_Line, a_Object);
		}
		
		// Unknown
		else
			return false;
	}
	
	/* ReMooD Low-Extended Specials */
	// These use the 16-bit as a sign
	else if (a_Line->special >= 0x8000 && a_Line->special <= 0xFFFF)
	{
	}
	
	/* ReMooD High-Extended Specials */
	// These are only accessable via RMD_EGLL or perhaps UDMF. These are mostly
	// for internal support.
	else if (a_Line->special > 0xFFFF)
	{
		// Get Type Base
		TypeBase = ((a_Line->special & EVGENHE_TYPEMASK) >> EVGENHE_TYPESHIFT);
		
		// If the type is Odd, it has a trigger
		if (TypeBase & 1)
		{
			// Extract Trigger
			TrigMode = (a_Line->special & EVGENGE_TRIGMASK) >> EVGENGE_TRIGSHIFT;
			
			// Trigger only once?
			*a_UseAgain = (TrigMode & 1);
			
			// Determine trigger compatibility
			if ((TrigMode == WalkOnce && a_Type != EVTGT_WALK) ||
				((TrigMode == PushOnce || TrigMode == SwitchOnce) && a_Type != EVTGT_SWITCH) ||
				(TrigMode == GunOnce && a_Type != EVTGT_SHOOT) ||
				(TrigMode == EVTGT_MAPSTART))
				return false;
			
			// Player and not player triggered
			if (a_Object->player)
				if (!((a_Line->special & EVGENGE_PLAYERMASK) >> EVGENGE_PLAYERSHIFT))
					return false;
			
			// Check Monster Trigger
			if (!a_Object->player)
				if (!((a_Line->special & EVGENGE_MONSTERMASK) >> EVGENGE_MONSTERSHIFT))
					return false;
			
			// Gunshots and pushable don't need tags
				// Along with certain types
			if (TypeBase != EVGHET_XEXIT)
				if (!a_Line->tag && (TrigMode != PushOnce && TrigMode != GunOnce))
					return false;
		}
		
		// Otherwise, if it is even it is done at map start
		else
		{
			// Not map start
			if (TrigMode != EVTGT_MAPSTART)
				return false;
			
			// Never use map starts again
			*a_UseAgain = false;
		}
		
		// Activate Extended Triggers
			// Plat
		if (TypeBase == EVGHET_XPLAT)
			return EV_HExDoGenPlat(a_Line, a_Object);
			
			// Level Exit
		else if (TypeBase == EVGHET_XEXIT)
		{
			// Secret?
			if ((a_Line->special & EVGENGE_EXITSECRETMASK) >> EVGENGE_EXITSECRETSHIFT)
				G_SecretExitLevel();
			
			// Normal
			else
				G_ExitLevel();
			
			// Success!
			return true;
		}
			
			// Damage Object
		else if (TypeBase == EVGHET_XDAMAGE)
		{
			if (a_Object)
			{
				P_DamageMobj(a_Object, NULL, NULL,
					(a_Line->special & EVGENHE_XDAMGMASK) >> EVGENHE_XDAMGSHIFT);
				return true;
			}
		}
	}
	
	/* Failed */
	return false;
}

/* EV_DoomToGenTrigger() -- Translate old Doom Lines to generalized ones */
uint32_t EV_DoomToGenTrigger(const uint32_t a_Input)
{
	size_t i;
	
	/* Inputs at generalization base are unchanged */
	if (!a_Input || a_Input >= GenCrusherBase)
		return a_Input;
	
	/* Check through list */
	for (i = 0; i < g_NumReGenMap; i++)
		if (g_ReGenMap[i].Source == a_Input)
			return g_ReGenMap[i].Target;
	
	/* Otherwise return input */
	return a_Input;
}

/* EV_HexenToGenTrigger() -- Hexen to general trigger */
uint32_t EV_HexenToGenTrigger(const uint32_t a_Flags, const uint8_t a_Input, const uint8_t* const a_Args)
{
	EV_GenHEActivator_t Trig;
	
	/* Zero is nothing */
	if (!a_Input || !a_Args)
		return 0;
	
	/* Determine Trigger */
	Trig = 0;
	
	// Repeatable?
	if (a_Flags & ML_REPEAT_SPECIAL)
		Trig |= 1;
	
	// Determine the stuff
		// Player Cross
	if (((a_Flags & ML_SPAC_MASK) >> ML_SPAC_SHIFT) == 0)
		Trig |= ((WalkOnce << EVGENGE_TRIGSHIFT) & EVGENGE_TRIGMASK) | ((1 << EVGENGE_PLAYERSHIFT) & EVGENGE_PLAYERMASK);
		// Player Push
	else if (((a_Flags & ML_SPAC_MASK) >> ML_SPAC_SHIFT) == 1)
		Trig |= ((SwitchOnce << EVGENGE_TRIGSHIFT) & EVGENGE_TRIGMASK) | ((1 << EVGENGE_PLAYERSHIFT) & EVGENGE_PLAYERMASK);
		// Monster Cross
	else if (((a_Flags & ML_SPAC_MASK) >> ML_SPAC_SHIFT) == 0)
		Trig |= ((WalkOnce << EVGENGE_TRIGSHIFT) & EVGENGE_TRIGMASK) | ((1 << EVGENGE_MONSTERSHIFT) & EVGENGE_MONSTERMASK);
		// Player/Monster Push
	else if (((a_Flags & ML_SPAC_MASK) >> ML_SPAC_SHIFT) == 1)
		Trig |= ((SwitchOnce << EVGENGE_TRIGSHIFT) & EVGENGE_TRIGMASK) | ((1 << EVGENGE_PLAYERSHIFT) & EVGENGE_PLAYERMASK) | ((1 << EVGENGE_MONSTERSHIFT) & EVGENGE_MONSTERMASK);
	
	/* Which Special? */
	// Damage Object
	if (a_Input == 73)
		return Trig |= EVGENHE_TYPEBASE(EVGHET_XDAMAGE) |
				(((!a_Args[0] ? 10000 : a_Args[0]) << EVGENHE_XDAMGSHIFT) & EVGENHE_XDAMGMASK);
	
	/* Currently Unhandled */
	return 0;
}

