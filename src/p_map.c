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
//      Movement, collision handling.
//      Shooting and aiming.

#include "doomdef.h"
#include "g_game.h"
#include "m_bbox.h"
#include "m_random.h"
#include "p_local.h"
#include "p_inter.h"
#include "r_state.h"
#include "r_main.h"
#include "r_sky.h"
#include "s_sound.h"

#include "r_splats.h"			//faB: testing

#include "z_zone.h"				//SoM: 3/15/2000
#include "p_demcmp.h"
#include "g_input.h"
#include "d_main.h"

fixed_t tmbbox[4];
mobj_t* tmthing;
int tmflags;
fixed_t tmx;
fixed_t tmy;

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
bool_t floatok;

fixed_t tmfloorz;
fixed_t tmceilingz;
fixed_t tmdropoffz;

mobj_t* tmfloorthing;			// the thing corresponding to tmfloorz

// or NULL if tmfloorz is from a sector

//added:28-02-98: used at P_ThingHeightClip() for moving sectors
fixed_t tmsectorfloorz;
fixed_t tmsectorceilingz;

// keep track of the line that lowers the ceiling,
// so missiles don't explode against sky hack walls
line_t* ceilingline;

// set by PIT_CheckLine() for any line that stopped the PIT_CheckLine()
// that is, for any line which is 'solid'
line_t* blockingline;

// keep track of special lines as they are hit,
// but don't process them until the move is proven valid
int* spechit;					//SoM: 3/15/2000: Limit removal
int numspechit;

//SoM: 3/15/2000
msecnode_t* sector_list = NULL;

//SoM: 3/15/2000
static int pe_x;				// Pain Elemental position for Lost Soul checks
static int pe_y;				// Pain Elemental position for Lost Soul checks
static int ls_x;				// Lost Soul position for Lost Soul checks
static int ls_y;				// Lost Soul position for Lost Soul checks

//
// TELEPORT MOVE
//

//
// PIT_StompThing
//
static bool_t PIT_StompThing(mobj_t* thing, void* a_Arg)
{
	fixed_t blockdist;
	
	//SoM: 3/15/2000: Move self check to start of routine.
	
	// don't clip against self
	
	if (thing == tmthing)
		return true;
		
	if (!(thing->flags & MF_SHOOTABLE))
		return true;
		
	blockdist = thing->radius + tmthing->radius;
	
	if (abs(thing->x - tmx) >= blockdist || abs(thing->y - tmy) >= blockdist)
		return true;			// didn't hit it
		
	// monsters don't stomp things except on boss level
	if (!P_MobjIsPlayer(tmthing) && gamemap != 30)
		return false;
		
	// Not allowed to stomp things
	if (!(tmthing->flags2 & MF2_TELESTOMP))
		return (false);
	
	tmthing->RXAttackAttackType = PRXAT_TELEFRAG;
	P_DamageMobj(thing, tmthing, tmthing, 10000);
	
	return true;
}

//SoM: Not unused. See p_user.c
//SoM: 3/15/2000
// P_GetMoveFactor() returns the value by which the x,y
// movements are multiplied to add to player movement.

int P_GetMoveFactor(mobj_t* mo)
{
	int movefactor = ORIG_FRICTION_FACTOR;
	
	// If the floor is icy or muddy, it's harder to get moving. This is where
	// the different friction factors are applied to 'trying to move'. In
	// p_mobj.c, the friction factors are applied as you coast and slow down.
	
	int momentum, friction;
	
	if (P_XGSVal(PGS_COBOOMSUPPORT) && P_XGSVal(PGS_COVARIABLEFRICTION) && !(mo->flags & (MF_NOGRAVITY | MF_NOCLIP)))
	{
		friction = mo->friction;
		if (friction == ORIG_FRICTION)	// normal floor
			;
		else if (friction > ORIG_FRICTION)	// ice
		{
			movefactor = mo->movefactor;
			mo->movefactor = ORIG_FRICTION_FACTOR;	// reset
		}
		else					// sludge
		{
		
			// phares 3/11/98: you start off slowly, then increase as
			// you get better footing
			
			momentum = (P_AproxDistance(mo->momx, mo->momy));
			movefactor = mo->movefactor;
			if (momentum > MORE_FRICTION_MOMENTUM << 2)
				movefactor <<= 3;
				
			else if (momentum > MORE_FRICTION_MOMENTUM << 1)
				movefactor <<= 2;
				
			else if (momentum > MORE_FRICTION_MOMENTUM)
				movefactor <<= 1;
				
			mo->movefactor = ORIG_FRICTION_FACTOR;	// reset
		}
	}
	return (movefactor);
}

//
// P_TeleportMove
//
bool_t P_TeleportMove(mobj_t* thing, fixed_t x, fixed_t y)
{
	int xl;
	int xh;
	int yl;
	int yh;
	int bx;
	int by;
	
	subsector_t* newsubsec;
	
	// kill anything occupying the position
	tmthing = thing;
	tmflags = thing->flags;
	
	tmx = x;
	tmy = y;
	
	tmbbox[BOXTOP] = y + tmthing->radius;
	tmbbox[BOXBOTTOM] = y - tmthing->radius;
	tmbbox[BOXRIGHT] = x + tmthing->radius;
	tmbbox[BOXLEFT] = x - tmthing->radius;
	
	newsubsec = R_PointInSubsector(x, y);
	ceilingline = NULL;
	
	// The base floor/ceiling is from the subsector
	// that contains the point.
	// Any contacted lines the step closer together
	// will adjust them.
	tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
	tmceilingz = newsubsec->sector->ceilingheight;
	
	validcount++;
	numspechit = 0;
	
	// stomp on any things contacted
	xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
	xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
	yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
	yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;
	
	for (bx = xl; bx <= xh; bx++)
		for (by = yl; by <= yh; by++)
			if (!P_BlockThingsIterator(bx, by, PIT_StompThing, NULL))
				return false;
				
	// the move is ok,
	// so link the thing into its new position
	P_UnsetThingPosition(thing);
	
	thing->floorz = tmfloorz;
	thing->ceilingz = tmceilingz;
	thing->x = x;
	thing->y = y;
	
	P_SetThingPosition(thing);
	
	return true;
}

// =========================================================================
//                       MOVEMENT ITERATOR FUNCTIONS
// =========================================================================

int spechit_max = 0;
static void add_spechit(line_t* ld)
{
	//SoM: 3/15/2000: Boom limit removal.
	if (numspechit >= spechit_max)
	{
		spechit_max = spechit_max ? spechit_max * 2 : 16;
		
		Z_ResizeArray((void**)&spechit, sizeof(*spechit), numspechit, spechit_max);
		Z_ChangeTag(spechit, PU_LEVEL);
	}
	
	spechit[numspechit] = ld - lines;
	numspechit++;
}

/* P_PITCTSettings_t -- Settings for PIT_CheckThing */
typedef struct P_PITCTSettings_s
{
	bool_t VsInfo;								// Use against mobjinfo, not mobj_t
} P_PITCTSettings_t;

/* PIT_CheckThing() -- Check thing */
static bool_t PIT_CheckThing(mobj_t* thing, void* a_Arg)
{
	fixed_t blockdist;
	bool_t solid, Dammed;
	int damage;
	P_PITCTSettings_t* SettingsP = a_Arg;
	
	//added:22-02-98:
	fixed_t topz;
	fixed_t tmtopz;
	
	//SoM: 3/15/2000: Moved to front.
	
	// don't clip against self
	
	if (thing == tmthing)
		return true;
		
	if (!(thing->flags & (MF_SOLID | MF_SPECIAL | MF_SHOOTABLE)))
		return true;
		
	blockdist = thing->radius + tmthing->radius;
	
	if (abs(thing->x - tmx) >= blockdist || abs(thing->y - tmy) >= blockdist)
	{
		// didn't hit it
		return true;
	}
	
	// heretic stuffs
	if (tmthing->flags2 & MF2_PASSMOBJ)
	{
		if (tmthing->z >= thing->z + thing->height && !(thing->flags & MF_SPECIAL))
		{
			return (true);
		}
		else if (tmthing->z + tmthing->height < thing->z && !(thing->flags & MF_SPECIAL))
		{
			// under thing
			return (true);
		}
	}
	
	// check for skulls slamming into things
	if (tmflags & MF_SKULLFLY)
	{
		damage = ((P_Random() % 8) + 1) * tmthing->info->damage;
		
		P_DamageMobj(thing, tmthing, tmthing, damage);
		
		tmthing->flags &= ~MF_SKULLFLY;
		tmthing->momx = tmthing->momy = tmthing->momz = 0;
		
		P_SetMobjState(tmthing, tmthing->info->spawnstate);
		
		return false;			// stop moving
	}
	
	// missiles can hit other things
	if (tmthing->flags & MF_MISSILE)
	{
		// Check for passing through a ghost (heretic)
		if ((thing->flags & MF_SHADOW) && (tmthing->flags2 & MF2_THRUGHOST))
			return true;
			
		// see if it went over / under
		if (tmthing->z > thing->z + thing->height)
			return true;		// overhead
		if (tmthing->z + tmthing->height < thing->z)
			return true;		// underneath
		
		// Don't hit self with missile
		if (thing == tmthing->target)
			return true;
		
		// Only harm things in differing families
		if (!P_MobjOnSameFamily(tmthing->target, thing))
		{
			if (!(thing->flags & MF_SHOOTABLE))
			{
				// didn't do any damage
				return !(thing->flags & MF_SOLID);
			}
			
			// more heretic stuff
			if (tmthing->flags2 & MF2_RIP)
			{
				damage = ((P_Random() & 3) + 2) * tmthing->info->damage;
				if (thing->flags2 & MF2_PUSHABLE && !(tmthing->flags2 & MF2_CANNOTPUSH))
				{
					// Push thing
					thing->momx += tmthing->momx >> 2;
					thing->momy += tmthing->momy >> 2;
				}
				numspechit = 0;
				return (true);
			}
		
			// damage / explode
			damage = ((int32_t)((P_Random() % 8) + 1)) * tmthing->info->damage;
			
			// GhostlyDeath <October 5, 2012> -- Freeze demo on death
			if (g_TitleScreenDemo)
				if (tmthing->RXFlags[1] & MFREXB_FREEZEDEMO)
					g_DemoFreezeTics += TICRATE * 2;
			
			Dammed = P_DamageMobj(thing, tmthing, tmthing->target, damage);
			if (Dammed && (thing->flags & MF_NOBLOOD) == 0 &&
					P_XGSVal(PGS_COENABLEBLOODSPLATS))
				P_SpawnBloodSplats(tmthing->x, tmthing->y, tmthing->z, damage, thing->momx, thing->momy, thing);
		}
			
		// don't traverse any more
		return false;
	}
	if (thing->flags2 & MF2_PUSHABLE && !(tmthing->flags2 & MF2_CANNOTPUSH))
	{
		// Push thing
		thing->momx += tmthing->momx >> 2;
		thing->momy += tmthing->momy >> 2;
	}
	
	// check for special pickup
	if (thing->flags & MF_SPECIAL)
	{
		solid = thing->flags & MF_SOLID;
		if (tmflags & MF_PICKUP)
		{
			// can remove thing
			P_TouchSpecialThing(thing, tmthing);
		}
		return !solid;
	}
	
	// check again for special pickup
	if (P_XGSVal(PGS_CODOUBLEPICKUPCHECK) && tmthing->flags & MF_SPECIAL)
	{
		solid = tmthing->flags & MF_SOLID;
		if (thing->flags & MF_PICKUP)
		{
			// can remove thing
			P_TouchSpecialThing(tmthing, thing);
		}
		return !solid;
	}
	
	//added:24-02-98:compatibility with old demos, it used to return with...
	//added:27-02-98:for version 112+, nonsolid things pass through other things
	if (P_XGSVal(PGS_CONONSOLIDPASSTHRUOLD) || P_XGSVal(PGS_CONONSOLIDPASSTHRUNEW) || !(tmthing->flags & MF_SOLID))
		return !(thing->flags & MF_SOLID);
		
	//added:22-02-98: added z checking at last
	//SoM: 3/10/2000: Treat noclip things as non-solid!
	if ((thing->flags & MF_SOLID) && (tmthing->flags & MF_SOLID) && !(thing->flags & MF_NOCLIP) && !(tmthing->flags & MF_NOCLIP))
	{
		// pass under
		tmtopz = tmthing->z + tmthing->height;
		
		if (tmtopz < thing->z)
		{
			if (thing->z < tmceilingz)
				tmceilingz = thing->z;
			return true;
		}
		
		topz = thing->z + thing->height + FRACUNIT;
		
		// block only when jumping not high enough,
		// (dont climb max. 24units while already in air)
		// if not in air, let P_TryMove() decide if its not too high
		if (P_MobjIsPlayer(tmthing) && tmthing->z < topz && tmthing->z > tmthing->floorz)	// block while in air
			return false;
			
		if (topz > tmfloorz)
		{
			tmfloorz = topz;
			tmfloorthing = thing;	//thing we may stand on
		}
		
	}
	
	// not solid not blocked
	return true;
}

// SoM: 3/15/2000
// PIT_CrossLine
// Checks to see if a PE->LS trajectory line crosses a blocking
// line. Returns false if it does.
//
// tmbbox holds the bounding box of the trajectory. If that box
// does not touch the bounding box of the line in question,
// then the trajectory is not blocked. If the PE is on one side
// of the line and the LS is on the other side, then the
// trajectory is blocked.
//
// Currently this assumes an infinite line, which is not quite
// correct. A more correct solution would be to check for an
// intersection of the trajectory and the line, but that takes
// longer and probably really isn't worth the effort.
//
static bool_t PIT_CrossLine(line_t* ld, void* a_Arg)
{
	if (!(ld->flags & ML_TWOSIDED) || (ld->flags & (ML_BLOCKING | ML_BLOCKMONSTERS)))
		if (!(tmbbox[BOXLEFT] > ld->bbox[BOXRIGHT] ||
		        tmbbox[BOXRIGHT] < ld->bbox[BOXLEFT] || tmbbox[BOXTOP] < ld->bbox[BOXBOTTOM] || tmbbox[BOXBOTTOM] > ld->bbox[BOXTOP]))
			if (P_PointOnLineSide(pe_x, pe_y, ld) != P_PointOnLineSide(ls_x, ls_y, ld))
				return (false);	// line blocks trajectory
	return (true);				// line doesn't block trajectory
}

//
// PIT_CheckLine
// Adjusts tmfloorz and tmceilingz as lines are contacted
//
bool_t PIT_CheckLine(line_t* ld, void* a_Arg)
{
	if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
	        || tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT] || tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM] || tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
		return true;
		
	if (P_BoxOnLineSide(tmbbox, ld) != -1)
		return true;
		
	// A line has been hit
	
	// The moving thing's destination position will cross
	// the given line.
	// If this should not be allowed, return false.
	// If the line is special, keep track of it
	// to process later if the move is proven ok.
	// NOTE: specials are NOT sorted by order,
	// so two special lines that are only 8 pixels apart
	// could be crossed in either order.
	
	// 10-12-99 BP: moved this line to out of the if so upper and
	//              lower texture can be hit by a splat
	blockingline = ld;
	if (!ld->backsector)
	{
		if (P_XGSVal(PGS_COMISSILESPECHIT) && tmthing->flags & MF_MISSILE && ld->special)
			add_spechit(ld);
			
		return false;			// one sided line
	}
	// missil and Camera can cross uncrossable line
	if (!(tmthing->flags & MF_MISSILE) && !(tmthing->RXFlags[0] & MFREXA_ALLOWNOCROSSCROSS))
	{
		if (ld->flags & ML_BLOCKING)
			return false;		// explicitly blocking everything
		
		if (!(tmthing->RXFlags[1] & MFREXB_IGNOREBLOCKMONS))
			if (!P_MobjIsPlayer(tmthing) && ld->flags & ML_BLOCKMONSTERS)
				return false;		// block monsters only
	}
	// set openrange, opentop, openbottom
	P_LineOpening(ld);
	
	// adjust floor / ceiling heights
	if (opentop < tmceilingz)
	{
		tmsectorceilingz = tmceilingz = opentop;
		ceilingline = ld;
	}
	
	if (openbottom > tmfloorz)
		tmsectorfloorz = tmfloorz = openbottom;
		
	if (lowfloor < tmdropoffz)
		tmdropoffz = lowfloor;
		
	// if contacted a special line, add it to the list
	if (ld->special)
		add_spechit(ld);
		
	return true;
}

// =========================================================================
//                         MOVEMENT CLIPPING
// =========================================================================

//
// P_CheckPosition
// This is purely informative, nothing is modified
// (except things picked up).
//
// in:
//  a mobj_t (can be valid or invalid)
//  a position to be checked
//   (doesn't need to be related to the mobj_t->x,y)
//
// during:
//  special things are touched if MF_PICKUP
//  early out on solid lines?
//
// out:
//  newsubsec
//  tmfloorz
//  tmceilingz
//  tmdropoffz
//   the lowest point contacted
//   (monsters won't move to a dropoff)
//  speciallines[]
//  numspeciallines
//

//added:27-02-98:
//
// tmfloorz
//     the nearest floor or thing's top under tmthing
// tmceilingz
//     the nearest ceiling or thing's bottom over tmthing
//
bool_t P_CheckPosition(mobj_t* thing, fixed_t x, fixed_t y, uint32_t a_Flags)
{
	int xl, xh, yl, yh, bx, by;
	fixed_t nfz, ncz;
	subsector_t* newsubsec, *SS;
	P_PITCTSettings_t Settings;
	
	// Clear settings
	memset(&Settings, 0, sizeof(Settings));
	
	tmthing = thing;
	tmflags = thing->flags;
	
	tmx = x;
	tmy = y;
	
	tmbbox[BOXTOP] = y + tmthing->radius;
	tmbbox[BOXBOTTOM] = y - tmthing->radius;
	tmbbox[BOXRIGHT] = x + tmthing->radius;
	tmbbox[BOXLEFT] = x - tmthing->radius;
	
	newsubsec = R_PointInSubsector(x, y);
	ceilingline = blockingline = NULL;
	
	// The base floor / ceiling is from the subsector
	// that contains the point.
	// Any contacted lines the step closer together
	// will adjust them.
	tmfloorz = tmsectorfloorz = tmdropoffz = newsubsec->sector->floorheight;
	tmceilingz = tmsectorceilingz = newsubsec->sector->ceilingheight;
	
	// GhostlyDeath <November 21, 2012> -- Sloped floors/ceilings?
	if (P_XGSVal(PGS_COENABLESLOPES))
	{
		SS = R_PointInSubsector(x, y);
		nfz = P_ZAtPointSS(SS, false, x, y, thing->z);
		ncz = P_ZAtPointSS(SS, true, x, y, thing->z);
	
		if (nfz > tmfloorz)
			tmfloorz = nfz;
		if (ncz < tmceilingz)
			tmceilingz = ncz;
	}
	
	//SoM: 3/23/2000: Check list of fake floors and see if
	//tmfloorz/tmceilingz need to be altered.
	if (P_XGSVal(PGS_COMOVECHECKFAKEFLOOR))
		if (newsubsec->sector->ffloors)
		{
			ffloor_t* rover;
			fixed_t delta1;
			fixed_t delta2;
			int thingtop = thing->z + thing->height;
		
			for (rover = newsubsec->sector->ffloors; rover; rover = rover->next)
			{
				if (!(rover->flags & FF_SOLID) || !(rover->flags & FF_EXISTS))
					continue;
				
				delta1 = thing->z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight) / 2));
				delta2 = thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight) / 2));
				if (*rover->topheight > tmfloorz && abs(delta1) < abs(delta2))
					tmfloorz = tmdropoffz = *rover->topheight;
				if (*rover->bottomheight < tmceilingz && abs(delta1) >= abs(delta2))
					tmceilingz = *rover->bottomheight;
			}
		}
	// tmfloorthing is set when tmfloorz comes from a thing's top
	tmfloorthing = NULL;
	
	validcount++;
	numspechit = 0;
	
	if (tmflags & MF_NOCLIP)
		return true;
		
	// Check things first, possibly picking things up.
	// The bounding box is extended by MAXRADIUS
	// because mobj_ts are grouped into mapblocks
	// based on their origin point, and can overlap
	// into adjacent blocks by up to MAXRADIUS units.
	
	// BP: added MF_NOCLIPTHING :used by camera to don't be blocked by things
	if (!(thing->flags & MF_NOCLIPTHING) && 
		((a_Flags & PCPF_FORSPOTCHECK) ||
			(P_XGSVal(PGS_COOLDCHECKPOSITION) || (thing->flags & MF_SOLID || thing->flags & MF_MISSILE))))
		/* DarkWolf95:don't check non-solids against other things,
		   keep them in the map though, so still check against lines */
	{
		xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
		xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
		yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
		yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;
		
		if (a_Flags & PCPF_FORSPOTCHECK)
			Settings.VsInfo = true;
		
		for (bx = xl; bx <= xh; bx++)
			for (by = yl; by <= yh; by++)
				if (!P_BlockThingsIterator(bx, by, PIT_CheckThing, &Settings))
					return false;
	}
	// check lines
	xl = (tmbbox[BOXLEFT] - bmaporgx) >> MAPBLOCKSHIFT;
	xh = (tmbbox[BOXRIGHT] - bmaporgx) >> MAPBLOCKSHIFT;
	yl = (tmbbox[BOXBOTTOM] - bmaporgy) >> MAPBLOCKSHIFT;
	yh = (tmbbox[BOXTOP] - bmaporgy) >> MAPBLOCKSHIFT;
	
	for (bx = xl; bx <= xh; bx++)
		for (by = yl; by <= yh; by++)
			if (!P_BlockLinesIterator(bx, by, PIT_CheckLine, NULL))
				return false;
				
	return true;
}


/* PIT_CheckThingDet() -- Check thing */
static bool_t PIT_CheckThingDet(mobj_t* thing, void* a_Arg)
{
	fixed_t blockdist;
	bool_t solid;
	int damage;
	P_PITCTSettings_t* SettingsP = a_Arg;
	
	//added:22-02-98:
	fixed_t topz;
	fixed_t tmtopz;
	
	//SoM: 3/15/2000: Moved to front.
	
	// don't clip against self
	
	if (thing == tmthing)
		return true;
		
	if (!(thing->flags & (MF_SOLID | MF_SPECIAL | MF_SHOOTABLE)))
		return true;
		
	blockdist = thing->radius + tmthing->radius;
	
	if (abs(thing->x - tmx) >= blockdist || abs(thing->y - tmy) >= blockdist)
	{
		// didn't hit it
		return true;
	}
	
	// heretic stuffs
	if (tmthing->flags2 & MF2_PASSMOBJ)
	{
		if (tmthing->z >= thing->z + thing->height && !(thing->flags & MF_SPECIAL))
		{
			return (true);
		}
		else if (tmthing->z + tmthing->height < thing->z && !(thing->flags & MF_SPECIAL))
		{
			// under thing
			return (true);
		}
	}
	
	// check for skulls slamming into things
	if (tmflags & MF_SKULLFLY)
		return false;			// stop moving
	
	// missiles can hit other things
	if (tmthing->flags & MF_MISSILE)
	{
		// Check for passing through a ghost (heretic)
		if ((thing->flags & MF_SHADOW) && (tmthing->flags2 & MF2_THRUGHOST))
			return true;
			
		// see if it went over / under
		if (tmthing->z > thing->z + thing->height)
			return true;		// overhead
		if (tmthing->z + tmthing->height < thing->z)
			return true;		// underneath
			
		if (tmthing->target && (tmthing->target->type == thing->type || (tmthing->target->info->RBaseFamily && thing->info->RBaseFamily && tmthing->target->info->RBaseFamily == thing->info->RBaseFamily)))
		{
			// Don't hit same species as originator.
			if (thing == tmthing->target)
				return true;
			
			// Explode, but do no damage.
			// Let players missile other players.
			if (!(thing->RXFlags[0] & MFREXA_ISPLAYEROBJECT))
				//DarkWolf95:November 21, 2003: Monsters Infight!
				if (!P_XGSVal(PGS_FUNINFIGHTING))
					return false;
		}
		
		if (!(thing->flags & MF_SHOOTABLE))
		{
			// didn't do any damage
			return !(thing->flags & MF_SOLID);
		}
		// more heretic stuff
		if (tmthing->flags2 & MF2_RIP)
		{
			return (true);
		}
		// damage / explode
		// don't traverse any more
		return false;
	}
	if (thing->flags2 & MF2_PUSHABLE && !(tmthing->flags2 & MF2_CANNOTPUSH))
	{
	}
	
	// check for special pickup
	if (thing->flags & MF_SPECIAL)
	{
		solid = thing->flags & MF_SOLID;
		if (tmflags & MF_PICKUP)
		{
		}
		return !solid;
	}
	
	// check again for special pickup
	if (P_XGSVal(PGS_CODOUBLEPICKUPCHECK) && tmthing->flags & MF_SPECIAL)
	{
		solid = tmthing->flags & MF_SOLID;
		if (thing->flags & MF_PICKUP)
		{
		}
		return !solid;
	}
	
	//added:24-02-98:compatibility with old demos, it used to return with...
	//added:27-02-98:for version 112+, nonsolid things pass through other things
	if (P_XGSVal(PGS_CONONSOLIDPASSTHRUOLD) || P_XGSVal(PGS_CONONSOLIDPASSTHRUNEW) || !(tmthing->flags & MF_SOLID))
		return !(thing->flags & MF_SOLID);
		
	//added:22-02-98: added z checking at last
	//SoM: 3/10/2000: Treat noclip things as non-solid!
	if ((thing->flags & MF_SOLID) && (tmthing->flags & MF_SOLID) && !(thing->flags & MF_NOCLIP) && !(tmthing->flags & MF_NOCLIP))
	{
		// pass under
		tmtopz = tmthing->z + tmthing->height;
		
		if (tmtopz < thing->z)
		{
			return true;
		}
		
		topz = thing->z + thing->height + FRACUNIT;
		
		// block only when jumping not high enough,
		// (dont climb max. 24units while already in air)
		// if not in air, let P_TryMove() decide if its not too high
		if (P_MobjIsPlayer(tmthing) && tmthing->z < topz && tmthing->z > tmthing->floorz)	// block while in air
			return false;
			
		if (topz > tmfloorz)
		{
		}
		
	}
	
	// not solid not blocked
	return true;
}

/* PIT_CheckLineDet() -- Deterministic check line */
bool_t PIT_CheckLineDet(line_t* ld, void* a_Arg)
{
	line_t* tempblockingline;
	
	if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
	        || tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT] || tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM] || tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
		return true;
		
	if (P_BoxOnLineSide(tmbbox, ld) != -1)
		return true;
		
	// A line has been hit
	blockingline = ld;
	
	// The moving thing's destination position will cross
	// the given line.
	// If this should not be allowed, return false.
	// If the line is special, keep track of it
	// to process later if the move is proven ok.
	// NOTE: specials are NOT sorted by order,
	// so two special lines that are only 8 pixels apart
	// could be crossed in either order.
	
	// 10-12-99 BP: moved this line to out of the if so upper and
	//              lower texture can be hit by a splat
	if (!ld->backsector)
	{
		return false;			// one sided line
	}
	// missil and Camera can cross uncrossable line
	if (!(tmthing->flags & MF_MISSILE) && !(tmthing->RXFlags[0] & MFREXA_ALLOWNOCROSSCROSS))
	{
		if (ld->flags & ML_BLOCKING)
			return false;		// explicitly blocking everything
		
		if (!(tmthing->RXFlags[1] & MFREXB_IGNOREBLOCKMONS))
			if (!P_MobjIsPlayer(tmthing) && ld->flags & ML_BLOCKMONSTERS)
				return false;		// block monsters only
	}
		
	return true;
}

/* P_CheckPositionDetermine() -- Use non modifying positional check */
bool_t P_CheckPositionDetermine(mobj_t* thing, fixed_t x, fixed_t y, uint32_t a_Flags)
{
	int xl, xh, yl, yh, bx, by;
	subsector_t* newsubsec;
	P_PITCTSettings_t Settings;
	
	// Clear settings
	memset(&Settings, 0, sizeof(Settings));
	
	tmthing = thing;
	tmflags = thing->flags;
	
	tmx = x;
	tmy = y;
	
	tmbbox[BOXTOP] = y + tmthing->radius;
	tmbbox[BOXBOTTOM] = y - tmthing->radius;
	tmbbox[BOXRIGHT] = x + tmthing->radius;
	tmbbox[BOXLEFT] = x - tmthing->radius;
	
	newsubsec = R_PointInSubsector(x, y);
	ceilingline = blockingline = NULL;
	
	// The base floor / ceiling is from the subsector
	// that contains the point.
	// Any contacted lines the step closer together
	// will adjust them.
	tmfloorz = tmsectorfloorz = tmdropoffz = newsubsec->sector->floorheight;
	tmceilingz = tmsectorceilingz = newsubsec->sector->ceilingheight;
	
	//SoM: 3/23/2000: Check list of fake floors and see if
	//tmfloorz/tmceilingz need to be altered.
	if (newsubsec->sector->ffloors)
	{
		ffloor_t* rover;
		fixed_t delta1;
		fixed_t delta2;
		int thingtop = thing->z + thing->height;
		
		for (rover = newsubsec->sector->ffloors; rover; rover = rover->next)
		{
			if (!(rover->flags & FF_SOLID) || !(rover->flags & FF_EXISTS))
				continue;
				
			delta1 = thing->z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight) / 2));
			delta2 = thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight) / 2));
			if (*rover->topheight > tmfloorz && abs(delta1) < abs(delta2))
				tmfloorz = tmdropoffz = *rover->topheight;
			if (*rover->bottomheight < tmceilingz && abs(delta1) >= abs(delta2))
				tmceilingz = *rover->bottomheight;
		}
	}
	// tmfloorthing is set when tmfloorz comes from a thing's top
	tmfloorthing = NULL;
	
	validcount++;
	numspechit = 0;
	
	if (tmflags & MF_NOCLIP)
		return true;
		
	// Check things first, possibly picking things up.
	// The bounding box is extended by MAXRADIUS
	// because mobj_ts are grouped into mapblocks
	// based on their origin point, and can overlap
	// into adjacent blocks by up to MAXRADIUS units.
	
	// BP: added MF_NOCLIPTHING :used by camera to don't be blocked by things
	if (!(thing->flags & MF_NOCLIPTHING) && 
		((a_Flags & PCPF_FORSPOTCHECK) ||
			(P_XGSVal(PGS_COOLDCHECKPOSITION) || (thing->flags & MF_SOLID || thing->flags & MF_MISSILE))))
		/* DarkWolf95:don't check non-solids against other things,
		   keep them in the map though, so still check against lines */
	{
		xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
		xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
		yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
		yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;
		
		if (a_Flags & PCPF_FORSPOTCHECK)
			Settings.VsInfo = true;
		
		for (bx = xl; bx <= xh; bx++)
			for (by = yl; by <= yh; by++)
				if (!P_BlockThingsIterator(bx, by, PIT_CheckThingDet, &Settings))
					return false;
	}
	// check lines
	xl = (tmbbox[BOXLEFT] - bmaporgx) >> MAPBLOCKSHIFT;
	xh = (tmbbox[BOXRIGHT] - bmaporgx) >> MAPBLOCKSHIFT;
	yl = (tmbbox[BOXBOTTOM] - bmaporgy) >> MAPBLOCKSHIFT;
	yh = (tmbbox[BOXTOP] - bmaporgy) >> MAPBLOCKSHIFT;
	
	for (bx = xl; bx <= xh; bx++)
		for (by = yl; by <= yh; by++)
			if (!P_BlockLinesIterator(bx, by, PIT_CheckLineDet, NULL))
				return false;
				
	return true;
}

//==========================================================================
//
// CheckMissileImpact
//
//==========================================================================

/* P_CMIFlags_t -- CMI Flags */
typedef enum P_CMIFlags_e
{
	PCMIF_NOSHOOTLINE			= 0x00000001,	// No shoot line
} P_CMIFlags_t;

/* CheckMissileImpact() -- Checks for missile impact against wall */
static void CheckMissileImpact(mobj_t* mobj, const uint32_t a_Flags)
{
	int i;
	
	if (P_XGSVal(PGS_CODISABLEMISSILEIMPACTCHECK) || !numspechit || !(mobj->flags & MF_MISSILE) || !mobj->target)
		return;
		
	if (!P_MobjIsPlayer(mobj->target))
		return;
	
	if (!(a_Flags & PCMIF_NOSHOOTLINE))
		for (i = numspechit - 1; i >= 0; i--)
			P_ShootSpecialLine(mobj->target, lines + spechit[i]);
}

/* P_SubTryMoveFlags_t -- Sub try attempt flags */
typedef enum P_SubTryMoveFlags_s
{
	PSTMF_DONTACTUALLYMOVE		= 0x00000001,	// Don't actually move the thing
	PSTMF_DETCHECKPOSITION		= 0x00000002,	// Use deterministic check move
	PSTMF_IGNOREPOSCHECK		= 0x00000004,	// Ignore position check
} P_SubTryMoveFlags_t;

/* PS_SubTryMove() -- Try sub move */
static bool_t PS_SubTryMove(mobj_t* thing, fixed_t x, fixed_t y, bool_t allowdropoff, const uint32_t a_Flags)
{
	fixed_t oldx;
	subsector_t* SS;
	fixed_t oldy;
	int side;
	int oldside;
	line_t* ld;
	bool_t CeilStepDownOK = false;
	uint32_t Flags;
	
	floatok = false;
	
	Flags = 0;
	if (a_Flags & PSTMF_DONTACTUALLYMOVE)
		Flags |= PCMIF_NOSHOOTLINE;
	
	if (a_Flags & PSTMF_DETCHECKPOSITION)
	{
		if (!P_CheckPositionDetermine(thing, x, y, 0))
		{
			CheckMissileImpact(thing, Flags);
			return false;			// solid wall or thing
		}
	}
	else
	{
		if ((a_Flags & PSTMF_IGNOREPOSCHECK))
			return false;
		else
		{
			if (!P_CheckPosition(thing, x, y, 0))
			{
				CheckMissileImpact(thing, Flags);
				return false;			// solid wall or thing
			}
		}
	}
	
	if (!(thing->flags & MF_NOCLIP))
	{
		fixed_t maxstep = MAXSTEPMOVE;
		
		if (tmceilingz - tmfloorz < thing->height)
		{
			CheckMissileImpact(thing, Flags);
			return false;		// doesn't fit
		}
		
		floatok = true;
		
		// GhostlyDeath <March 6, 2012> -- Can step from the ceiling
		CeilStepDownOK = false;
		if (thing->RXFlags[0] & MFREXA_CANCEILINGSTEP)
			if ((thing->z + thing->height) - tmceilingz < maxstep)
				CeilStepDownOK = true;
		
		// Stepping from ceiling not OK? Check collision here
		if (!CeilStepDownOK)
			if (!(thing->flags & MF_TELEPORT) && tmceilingz - thing->z < thing->height && !(thing->flags2 & MF2_FLY))
			{
				CheckMissileImpact(thing, Flags);
				return false;		// mobj must lower itself to fit
			}
		
		if (thing->flags2 & MF2_FLY)
		{
			if (thing->z + thing->height > tmceilingz)
			{
				thing->momz = -8 * FRACUNIT;
				return false;
			}
			else if (thing->z < tmfloorz && tmfloorz - tmdropoffz > 24 * FRACUNIT)
			{
				thing->momz = 8 * FRACUNIT;
				return false;
			}
		}
		// jump out of water
		if ((thing->eflags & (MF_UNDERWATER | MF_TOUCHWATER)) == (MF_UNDERWATER | MF_TOUCHWATER))
			maxstep = 37 * FRACUNIT;
		
		if (!(thing->flags & MF_TELEPORT) && (tmfloorz - thing->z > maxstep))
		{
			CheckMissileImpact(thing, Flags);
			return false;		// too big a step up
		}
		
		if ((thing->flags & MF_MISSILE) && tmfloorz > thing->z)
			CheckMissileImpact(thing, Flags);
			
		if (!P_XGSVal(PGS_COBOOMSUPPORT) || !allowdropoff)
			if (!(thing->flags & (MF_DROPOFF | MF_FLOAT)) && !tmfloorthing && tmfloorz - tmdropoffz > MAXSTEPMOVE)
				return false;	// don't stand over a dropoff
	}
	
	// GhostlyDeath <April 27, 2012> -- Don't actually move!
	if (a_Flags & PSTMF_DONTACTUALLYMOVE)
		return true;
	
	// the move is ok,
	// so link the thing into its new position
	P_UnsetThingPosition(thing);
	
	//added:28-02-98: gameplay hack : walk over a small wall while 	jumping
	//                stop jumping it succeeded
	// BP: removed in 1.28 because we can move in air now
	if (P_XGSVal(PGS_COJUMPCHECK) && P_XGSVal(PGS_COOLDJUMPOVER) && P_MobjIsPlayer(thing) && (thing->player->cheats & CF_JUMPOVER))
		if (tmfloorz > thing->floorz + MAXSTEPMOVE)
			thing->momz >>= 2;
	
	oldx = thing->x;
	oldy = thing->y;
	thing->floorz = tmfloorz;
	thing->ceilingz = tmceilingz;
	thing->x = x;
	thing->y = y;
	
	//added:28-02-98:
	if (tmfloorthing)
		thing->eflags &= ~MF_ONGROUND;	//not on real floor
	else
		thing->eflags |= MF_ONGROUND;
		
	P_SetThingPosition(thing);
	if (thing->flags2 & MF2_FOOTCLIP && P_GetThingFloorType(thing) != FLOOR_SOLID)
		thing->flags2 |= MF2_FEETARECLIPPED;
	else if (thing->flags2 & MF2_FEETARECLIPPED)
		thing->flags2 &= ~MF2_FEETARECLIPPED;
		
	// if any special lines were hit, do the effect
	if (!(thing->flags & (MF_TELEPORT | MF_NOCLIP)) && !(thing->RXFlags[0] & MFREXA_NEVERCROSSTRIGGER))
	{
		while (numspechit--)
		{
			// see if the line was crossed
			ld = lines + spechit[numspechit];
			side = P_PointOnLineSide(thing->x, thing->y, ld);
			oldside = P_PointOnLineSide(oldx, oldy, ld);
			if (side != oldside)
			{
				if (ld->special)
					P_CrossSpecialLine(ld - lines, oldside, thing);
			}
		}
	}
	
	return true;
}

/* P_TryMove() -- Trying move */
bool_t P_TryMove(mobj_t* thing, fixed_t x, fixed_t y, bool_t allowdropoff, fixed_t* const a_OutX, fixed_t* const a_OutY)
{
#define MAXMOVETRIES 4
	bool_t MoveUp;
	fixed_t ox, oy;
	fixed_t mx, my;
	fixed_t dx, dy;
	fixed_t zx, zy;
	fixed_t fx, fy;
	fixed_t MoveFrac;
	fixed_t i, Dist, zIn, zOut;
	bool_t OKs[MAXMOVETRIES], FailNow;
	line_t* BlockLine;
	
	/* Smooth Traversing */
#if 0
	// GhostlyDeath <April 27, 2012> -- Improve Traversing
	if (P_XGSVal(PGS_COIMPROVEPATHTRAVERSE))
	{
		// No clipping?
		if ((thing->flags & MF_NOCLIP) || (thing->player && (thing->player->cheats & CF_NOCLIP)))
			return PS_SubTryMove(thing, x, y, allowdropoff, 0);
		
		// Clear OKs
		memset(&OKs, 0, sizeof(OKs));
		OKs[0] = true;	// First move is always OK!
		BlockLine = NULL;
		
		// Set move fraction
		MoveFrac = FixedDiv(1 << FRACBITS, MAXMOVETRIES << FRACBITS);
		
		// Get original points
		fx = ox = thing->x;
		fy = oy = thing->y;
		dx = x;
		dy = y;
		
		// Get difference of points
		zx = dx - ox;
		zy = dy - oy;
		
		Dist = P_AproxDistance(zx, zy);
		
		// Don't care enough to try?
		if (Dist <= (thing->radius >> 1))
			return PS_SubTryMove(thing, dx, dy, allowdropoff, 0);
		
		// Run through tries
		FailNow = false;
		for (i = 1; i <= MAXMOVETRIES; i++)
		{
			// Position to check
			mx = ox + FixedMul(zx, (MoveFrac * i));
			my = oy + FixedMul(zy, (MoveFrac * i));
			
			// This spot OK?
			if (R_IsPointInSubsector(mx, my))
				if (PS_SubTryMove(thing, mx, my, allowdropoff, PSTMF_DONTACTUALLYMOVE | PSTMF_DETCHECKPOSITION)/* || FailNow*/)
				{
					// Set OK and set furthest x/y pos
					OKs[i] = (FailNow ? false: true);
					fx = mx;
					fy = my;
					
					if (FailNow)
						break;
				}
				else	// The line that blocked this thing from moving
				{
					BlockLine = blockingline;
					FailNow = true;
				}
		}
		
		// Move to the furthest guessed position now
		PS_SubTryMove(thing, fx, fy, allowdropoff, 0);
		blockingline = BlockLine;	// It may never have hit this line, so just in case!
		
		// Change positional pointers
		if (a_OutX)
			*a_OutX = fx;
		if (a_OutY)
			*a_OutY = fy;
		
		// Now check each position and see if anything returned false
		for (i = 0; i < MAXMOVETRIES; i++)
			if (!OKs[i])
				return false;
		
		// Never returned false!
		return true;
	}
#endif
	
	/* Non-Smooth Traversing */
	//else
	{
		return PS_SubTryMove(thing, x, y, allowdropoff, 0);
	}
#undef MAXMOVETRIES
}

//
// P_ThingHeightClip
// Takes a valid thing and adjusts the thing->floorz,
// thing->ceilingz, and possibly thing->z.
// This is called for all nearby monsters
// whenever a sector changes height.
// If the thing doesn't fit,
// the z will be set to the lowest value
// and false will be returned.
//
bool_t P_ThingHeightClip(mobj_t* thing)
{
	bool_t onfloor;
	
	onfloor = (thing->z <= thing->floorz);
	
	P_CheckPosition(thing, thing->x, thing->y, 0);
	
	// what about stranding a monster partially off an edge?
	
	thing->floorz = tmfloorz;
	thing->ceilingz = tmceilingz;
	
	if (!tmfloorthing && onfloor && !(thing->flags & MF_NOGRAVITY))
	{
		// walking monsters rise and fall with the floor
		thing->z = thing->floorz;
	}
	else
	{
		// don't adjust a floating monster unless forced to
		//added:18-04-98:test onfloor
		if (!onfloor)			//was tmsectorceilingz
			if (thing->z + thing->height > tmceilingz)
				thing->z = thing->ceilingz - thing->height;
				
		//thing->eflags &= ~MF_ONGROUND;
	}
	
	//debug : be sure it falls to the floor
	thing->eflags &= ~MF_ONGROUND;
	
	//added:28-02-98:
	// test sector bouding top & bottom, not things
	
	//if (tmsectorceilingz - tmsectorfloorz < thing->height)
	//    return false;
	
	if (thing->ceilingz - thing->floorz < thing->height
	        // BP: i know that this code cause many trouble but this fix alos
	        // lot of problem, mainly this is implementation of the stepping
	        // for mobj (walk on solid corpse without jumping or fake 3d bridge)
	        // problem is imp into imp at map01 and monster going at top of others
	        && thing->z >= thing->floorz)
		return false;
		
	return true;
}

//
// SLIDE MOVE
// Allows the player to slide along any angled walls.
//
fixed_t bestslidefrac;
fixed_t secondslidefrac;

line_t* bestslideline;
line_t* secondslideline;

mobj_t* slidemo;

fixed_t tmxmove;
fixed_t tmymove;

//
// P_HitSlideLine
// Adjusts the xmove / ymove
// so that the next move will slide along the wall.
//
void P_HitSlideLine(line_t* ld)
{
	int side;
	
	angle_t lineangle;
	angle_t moveangle;
	angle_t deltaangle;
	
	fixed_t movelen;
	fixed_t newlen;
	
	if (ld->slopetype == ST_HORIZONTAL)
	{
		tmymove = 0;
		return;
	}
	
	if (ld->slopetype == ST_VERTICAL)
	{
		tmxmove = 0;
		return;
	}
	
	side = P_PointOnLineSide(slidemo->x, slidemo->y, ld);
	
	lineangle = R_PointToAngle2(0, 0, ld->dx, ld->dy);
	
	if (side == 1)
		lineangle += ANG180;
		
	moveangle = R_PointToAngle2(0, 0, tmxmove, tmymove);
	deltaangle = moveangle - lineangle;
	
	if (deltaangle > ANG180)
		deltaangle += ANG180;
	//  I_Error ("SlideLine: ang>ANG180");
	
	lineangle >>= ANGLETOFINESHIFT;
	deltaangle >>= ANGLETOFINESHIFT;
	
	movelen = P_AproxDistance(tmxmove, tmymove);
	newlen = FixedMul(movelen, finecosine[deltaangle]);
	
	tmxmove = FixedMul(newlen, finecosine[lineangle]);
	tmymove = FixedMul(newlen, finesine[lineangle]);
}

//
// PTR_SlideTraverse
//
bool_t PTR_SlideTraverse(intercept_t* in, void* a_Data)
{
	line_t* li;
	
	li = in->d.line;
	
	if (!(li->flags & ML_TWOSIDED))
	{
		if (P_PointOnLineSide(slidemo->x, slidemo->y, li))
		{
			// don't hit the back side
			return true;
		}
		goto isblocking;
	}
	// set openrange, opentop, openbottom
	P_LineOpening(li);
	
	if (openrange < slidemo->height)
		goto isblocking;		// doesn't fit
		
	if (opentop - slidemo->z < slidemo->height)
		goto isblocking;		// mobj is too high
		
	if (openbottom - slidemo->z > 24 * FRACUNIT)
		goto isblocking;		// too big a step up
		
	// this line doesn't block movement
	return true;
	
	// the line does block movement,
	// see if it is closer than best so far
isblocking:

	if (in->frac < bestslidefrac)
	{
		secondslidefrac = bestslidefrac;
		secondslideline = bestslideline;
		bestslidefrac = in->frac;
		bestslideline = li;
	}
	
	return false;				// stop
}

//
// P_SlideMove
// The momx / momy move is bad, so try to slide
// along a wall.
// Find the first line hit, move flush to it,
// and slide along it
//
// This is a kludgy mess.
//
void P_SlideMove(mobj_t* mo)
{
	fixed_t leadx;
	fixed_t leady;
	fixed_t trailx;
	fixed_t traily;
	fixed_t newx, oX, oY;
	fixed_t newy;
	int hitcount;
	
	slidemo = mo;
	hitcount = 0;
	
retry:
	if (++hitcount == 3)
		goto stairstep;			// don't loop forever
		
	// trace along the three leading corners
	if (mo->momx > 0)
	{
		leadx = mo->x + mo->radius;
		trailx = mo->x - mo->radius;
	}
	else
	{
		leadx = mo->x - mo->radius;
		trailx = mo->x + mo->radius;
	}
	
	if (mo->momy > 0)
	{
		leady = mo->y + mo->radius;
		traily = mo->y - mo->radius;
	}
	else
	{
		leady = mo->y - mo->radius;
		traily = mo->y + mo->radius;
	}
	
	bestslidefrac = FRACUNIT + 1;
	
	P_PathTraverse(leadx, leady, leadx + mo->momx, leady + mo->momy, PT_ADDLINES, PTR_SlideTraverse, NULL);
	P_PathTraverse(trailx, leady, trailx + mo->momx, leady + mo->momy, PT_ADDLINES, PTR_SlideTraverse, NULL);
	P_PathTraverse(leadx, traily, leadx + mo->momx, traily + mo->momy, PT_ADDLINES, PTR_SlideTraverse, NULL);
	
	// move up to the wall
	if (bestslidefrac == FRACUNIT + 1)
	{
		// the move most have hit the middle, so stairstep
stairstep:
		if (!P_TryMove(mo, mo->x, mo->y + mo->momy, true, NULL, NULL))	//SoM: 4/10/2000
			P_TryMove(mo, mo->x + mo->momx, mo->y, true, NULL, NULL);	//Allow things to
		return;					//drop off.
	}
	
	// fudge a bit to make sure it doesn't hit
	bestslidefrac -= 0x800;
	if (bestslidefrac > 0)
	{
		newx = FixedMul(mo->momx, bestslidefrac);
		newy = FixedMul(mo->momy, bestslidefrac);
		
		if (!P_TryMove(mo, mo->x + newx, mo->y + newy, true, NULL, NULL))
			goto stairstep;
	}
	// Now continue along the wall.
	// First calculate remainder.
	bestslidefrac = FRACUNIT - (bestslidefrac + 0x800);
	
	if (bestslidefrac > FRACUNIT)
		bestslidefrac = FRACUNIT;
		
	if (bestslidefrac <= 0)
		return;
		
	tmxmove = FixedMul(mo->momx, bestslidefrac);
	tmymove = FixedMul(mo->momy, bestslidefrac);
	
	P_HitSlideLine(bestslideline);	// clip the moves
	
	mo->momx = tmxmove;
	mo->momy = tmymove;
	
	if (!P_TryMove(mo, mo->x + tmxmove, mo->y + tmymove, true, &oX, &oY))
	{
		goto retry;
	}
}

//
// P_LineAttack
//
mobj_t* linetarget;				// who got hit (or NULL)
mobj_t* shootthing;

// Height if not aiming up or down
// ???: use slope for monsters?
fixed_t shootz;
fixed_t lastz;					//SoM: The last z height of the bullet when it crossed a line

int la_damage;
fixed_t attackrange;

fixed_t aimslope;

//
// PTR_AimTraverse
// Sets linetarget and aimslope when a target is aimed at.
//
//added:15-02-98: comment
// Returns true if the thing is not shootable, else continue through..
//
bool_t PTR_AimTraverse(intercept_t* in, void* a_Data)
{
	line_t* li;
	mobj_t* th;
	fixed_t slope;
	fixed_t thingtopslope;
	fixed_t thingbottomslope;
	fixed_t dist;
	int dir;
	
	if (in->isaline)
	{
		li = in->d.line;
		
		if (!(li->flags & ML_TWOSIDED))
			return false;		// stop
			
		// Crosses a two sided line.
		// A two sided line will restrict
		// the possible target ranges.
		tmthing = NULL;
		P_LineOpening(li);
		
		if (openbottom >= opentop)
			return false;		// stop
			
		dist = FixedMul(attackrange, in->frac);
		
		// GhostlyDeath -- If a line doesn't have a backsector or frontsector then stop
		// since the lines here are 2 sided!
		if (!li->backsector || !li->frontsector)
			return false;
			
		if (li->frontsector->floorheight != li->backsector->floorheight)
		{
			slope = FixedDiv(openbottom - shootz, dist);
			if (slope > bottomslope)
				bottomslope = slope;
		}
		
		if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
		{
			slope = FixedDiv(opentop - shootz, dist);
			if (slope < topslope)
				topslope = slope;
		}
		
		if (topslope <= bottomslope)
			return false;		// stop
		
		// GhostlyDeath <June 17, 2012> -- Demo Comp
		if (P_XGSVal(PGS_COAIMCHECKFAKEFLOOR))
			if (li->frontsector->ffloors || li->backsector->ffloors)
			{
				int frontflag;
			
				dir = aimslope > 0 ? 1 : aimslope < 0 ? -1 : 0;
			
				frontflag = P_PointOnLineSide(shootthing->x, shootthing->y, li);
			
				//SoM: Check 3D FLOORS!
				if (li->frontsector->ffloors)
				{
					ffloor_t* rover = li->frontsector->ffloors;
					fixed_t highslope, lowslope;
				
					for (; rover; rover = rover->next)
					{
						if (!(rover->flags & FF_SOLID) || !(rover->flags & FF_EXISTS))
							continue;
						
						highslope = FixedDiv(*rover->topheight - shootz, dist);
						lowslope = FixedDiv(*rover->bottomheight - shootz, dist);
						if ((aimslope >= lowslope && aimslope <= highslope))
							return false;
						
						if (lastz > *rover->topheight && dir == -1 && aimslope < highslope)
							frontflag |= 0x2;
						
						if (lastz < *rover->bottomheight && dir == 1 && aimslope > lowslope)
							frontflag |= 0x2;
					}
				}
			
				if (li->backsector->ffloors)
				{
					ffloor_t* rover = li->backsector->ffloors;
					fixed_t highslope, lowslope;
				
					for (; rover; rover = rover->next)
					{
						if (!(rover->flags & FF_SOLID) || !(rover->flags & FF_EXISTS))
							continue;
						
						highslope = FixedDiv(*rover->topheight - shootz, dist);
						lowslope = FixedDiv(*rover->bottomheight - shootz, dist);
						if ((aimslope >= lowslope && aimslope <= highslope))
							return false;
						
						if (lastz > *rover->topheight && dir == -1 && aimslope < highslope)
							frontflag |= 0x4;
						
						if (lastz < *rover->bottomheight && dir == 1 && aimslope > lowslope)
							frontflag |= 0x4;
					}
				}
				if ((!(frontflag & 0x1) && frontflag & 0x2) || (frontflag & 0x1 && frontflag & 0x4))
					return false;
			}
		
		lastz = FixedMul(aimslope, dist) + shootz;
		
		return true;			// shot continues
	}
	
	// shoot a thing
	th = in->d.thing;
	if (th == shootthing)
		return true;			// can't shoot self
		
	if ((!(th->flags & MF_SHOOTABLE)) || (th->flags & MF_CORPSE))
		return true;			// corpse or something
		
	// check angles to see if the thing can be aimed at
	dist = FixedMul(attackrange, in->frac);
	thingtopslope = FixedDiv(th->z + th->height - shootz, dist);
	
	//added:15-02-98: bottomslope is negative!
	if (thingtopslope < bottomslope)
		return true;			// shot over the thing
		
	thingbottomslope = FixedDiv(th->z - shootz, dist);
	
	if (thingbottomslope > topslope)
		return true;			// shot under the thing
		
	// this thing can be hit!
	if (thingtopslope > topslope)
		thingtopslope = topslope;
		
	if (thingbottomslope < bottomslope)
		thingbottomslope = bottomslope;
		
	//added:15-02-98: find the slope just in the middle(y) of the thing!
	aimslope = (thingtopslope + thingbottomslope) / 2;
	linetarget = th;
	
	return false;				// don't go any farther
}

//
// PTR_ShootTraverse
//
//added:18-02-98: added clipping the shots on the floor and ceiling.
//
bool_t PTR_ShootTraverse(intercept_t* in, void* a_Data)
{
	fixed_t x;
	fixed_t y;
	fixed_t z, tfz, tcz;
	fixed_t frac;
	subsector_t* SS;
	
	line_t* li;
	sector_t* sector = NULL;
	mobj_t* th;
	player_t* Player;
	
	fixed_t slope;
	fixed_t dist;
	fixed_t thingtopslope;
	fixed_t thingbottomslope;
	
	fixed_t floorz = 0;			//SoM: Bullets should hit fake floors!
	fixed_t ceilingz = 0;
	
	//added:18-02-98:
	fixed_t distz;				//dist between hit z on wall       and gun z
	fixed_t clipz;				//dist between hit z on floor/ceil and gun z
	bool_t hitplane;			//true if we clipped z on floor/ceil plane
	bool_t diffheights;			//check for sky hacks with different ceil heights
	
	int sectorside;
	int dir;
	
	P_LineAtkArgs_t* Args = a_Data;
	
	/* Get Player */
	if (P_MobjIsPlayer(shootthing))
		Player = shootthing->player;
	else
		Player = NULL;
	
	if (aimslope > 0)
		dir = 1;
	else if (aimslope < 0)
		dir = -1;
	else
		dir = 0;
		
	if (in->isaline)
	{
		//shut up compiler, otherwise it's only used when TWOSIDED
		diffheights = false;
		
		li = in->d.line;
		
		if (li->special)
			P_ShootSpecialLine(shootthing, li);
			
		if (!(li->flags & ML_TWOSIDED))
			goto hitline;
			
		// crosses a two sided line
		//added:16-02-98: Fab comments : sets opentop, openbottom, openrange
		//                lowfloor is the height of the lowest floor
		//                         (be it front or back)
		tmthing = NULL;
		P_LineOpening(li);
		
		dist = FixedMul(attackrange, in->frac);
		
		// GhostlyDeath -- 2 sided line without a front or back sector?
		if (!li->frontsector || !li->backsector)
			goto hitline;
			
		// hit lower texture ?
		if (li->frontsector->floorheight != li->backsector->floorheight)
		{
			//added:18-02-98: comments :
			// find the slope aiming on the border between the two floors
			slope = FixedDiv(openbottom - shootz, dist);
			if (slope > aimslope)
				goto hitline;
		}
		
		// hit upper texture ?
		if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
		{
			//added:18-02-98: remember : diff ceil heights
			diffheights = true;
			
			slope = FixedDiv(opentop - shootz, dist);
			if (slope < aimslope)
				goto hitline;
		}
		
		// GhostlyDeath <June 17, 2012> -- Demo Comp
		if (P_XGSVal(PGS_COSHOOTCHECKFAKEFLOOR))
			if (li->frontsector->ffloors || li->backsector->ffloors)
			{
				int frontflag;
			
				frontflag = P_PointOnLineSide(shootthing->x, shootthing->y, li);
			
				//SoM: Check 3D FLOORS!
				if (P_XGSVal(PGS_COSHOOTCHECKFAKEFLOOR))
					if (li->frontsector->ffloors)
					{
						ffloor_t* rover = li->frontsector->ffloors;
						fixed_t highslope, lowslope;
				
						for (; rover; rover = rover->next)
						{
							if (!(rover->flags & FF_SOLID) || !(rover->flags & FF_EXISTS))
								continue;
						
							highslope = FixedDiv(*rover->topheight - shootz, dist);
							lowslope = FixedDiv(*rover->bottomheight - shootz, dist);
							if ((aimslope >= lowslope && aimslope <= highslope))
								goto hitline;
						
							if (lastz > *rover->topheight && dir == -1 && aimslope < highslope)
								frontflag |= 0x2;
						
							if (lastz < *rover->bottomheight && dir == 1 && aimslope > lowslope)
								frontflag |= 0x2;
						}
					}
				
				if (P_XGSVal(PGS_COSHOOTCHECKFAKEFLOOR))
					if (li->backsector->ffloors)
					{
						ffloor_t* rover = li->backsector->ffloors;
						fixed_t highslope, lowslope;
				
						for (; rover; rover = rover->next)
						{
							if (!(rover->flags & FF_SOLID) || !(rover->flags & FF_EXISTS))
								continue;
						
							highslope = FixedDiv(*rover->topheight - shootz, dist);
							lowslope = FixedDiv(*rover->bottomheight - shootz, dist);
							if ((aimslope >= lowslope && aimslope <= highslope))
								goto hitline;
						
							if (lastz > *rover->topheight && dir == -1 && aimslope < highslope)
								frontflag |= 0x4;
						
							if (lastz < *rover->bottomheight && dir == 1 && aimslope > lowslope)
								frontflag |= 0x4;
						}
					}
				
				if ((!(frontflag & 0x1) && frontflag & 0x2) || (frontflag & 0x1 && frontflag & 0x4))
					goto hitline;
			}
		
		lastz = FixedMul(aimslope, dist) + shootz;
		
		// shot continues
		return true;
		
		// hit line
hitline:

		// position a bit closer
		frac = in->frac - FixedDiv(4 * FRACUNIT, attackrange);
		dist = FixedMul(frac, attackrange);	//dist to hit on line
		
		distz = FixedMul(aimslope, dist);	//z add between gun z and hit z
		z = shootz + distz;		// hit z on wall
		
		// GhostlyDeath <June 17, 2012> -- Demo Comp
		//added:17-02-98: clip shots on floor and ceiling
		//                use a simple triangle stuff a/b = c/d ...
		// BP:13-3-99: fix the side usage
		hitplane = false;
		if (P_XGSVal(PGS_COSHOOTFLOORCLIPPING))
		{
			sectorside = P_PointOnLineSide(shootthing->x, shootthing->y, li);
			if (li->sidenum[sectorside] != -1)	// can happen in nocliping mode
			{
				sector = sides[li->sidenum[sectorside]].sector;
				
				floorz = sector->floorheight;
				ceilingz = sector->ceilingheight;
				
				// GhostlyDeath <November 21, 2012> -- Sloped Floor/Ceiling
				if (P_XGSVal(PGS_COENABLESLOPES))
				{
					x = trace.x + FixedMul(trace.dx, frac);
					y = trace.y + FixedMul(trace.dy, frac);
				
					SS = R_PointInSubsector(x, y);
					tcz = P_ZAtPointSS(SS, true, x, y, z);
					tfz = P_ZAtPointSS(SS, false, x, y, z);
					
					if (tfz > floorz)
						floorz = tfz;
					if (tcz < ceilingz)
						ceilingz = tcz;
				}
				
				if (sector->ffloors)
				{
					ffloor_t* rover;
				
					for (rover = sector->ffloors; rover; rover = rover->next)
					{
						if (!(rover->flags & FF_SOLID))
							continue;
						
						if (dir == 1 && *rover->bottomheight < ceilingz && *rover->bottomheight > lastz)
							ceilingz = *rover->bottomheight;
						if (dir == -1 && *rover->topheight > floorz && *rover->topheight < lastz)
							floorz = *rover->topheight;
					}
				}
			
				if ((z > ceilingz) && distz)
				{
					clipz = ceilingz - shootz;
					frac = FixedDiv(FixedMul(frac, clipz), distz);
					hitplane = true;
				}
				else if ((z < floorz) && distz)
				{
					clipz = shootz - floorz;
					frac = -FixedDiv(FixedMul(frac, clipz), distz);
					hitplane = true;
				}
				
				if (sector->ffloors || P_XGSVal(PGS_COENABLESLOPES))
				{
					if (dir == 1 && z > ceilingz)
						z = ceilingz;
					if (dir == -1 && z < floorz)
						z = floorz;
				}
			}
		
			//SPLAT TEST ----------------------------------------------------------
			if (!hitplane && P_XGSVal(PGS_COMISSILESPLATONWALL))
			{
				divline_t divl;
				fixed_t frac;
			
				P_MakeDivline(li, &divl);
				frac = P_InterceptVector(&divl, &trace);
				R_AddWallSplat(
						li,
						sectorside,
						(Player && Player->weaponinfo[Player->readyweapon]->TracerSplat ?
							Player->weaponinfo[Player->readyweapon]->TracerSplat :
							"A_DMG1"),
						z,
						frac,
						SPLATDRAWMODE_SHADE
					);
			}
			// --------------------------------------------------------- SPLAT TEST
		}
		
		x = trace.x + FixedMul(trace.dx, frac);
		y = trace.y + FixedMul(trace.dy, frac);
		
		if (li->frontsector->ceilingpic == skyflatnum)
		{
			// don't shoot the sky!
			if (z > li->frontsector->ceilingheight)
				return false;
				
			//added:24-02-98: compatibility with older demos
			if (P_XGSVal(PGS_COHITSCANSSLIDEONFLATS))
			{
				diffheights = true;
				hitplane = false;
			}
			
			// it's a sky hack wall
			if ((!hitplane &&	//added:18-02-98:not for shots on planes
			        li->backsector && diffheights &&	//added:18-02-98:skip only REAL sky hacks
			        //   eg: they use different ceil heights.
			        li->backsector->ceilingpic == skyflatnum))
				return false;
		}
		
		// GhostlyDeath <June 17, 2012> -- Demo Comp
		if (P_XGSVal(PGS_COSHOOTCHECKFAKEFLOOR))
			if (sector && sector->ffloors)
			{
				if (dir == 1 && z + (16 << FRACBITS) > ceilingz)
					z = ceilingz - (16 << FRACBITS);
				if (dir == -1 && z < floorz)
					z = floorz;
			}
		
		// Spawn bullet puffs.
		P_SpawnPuff(x, y, z);
		
		// don't go any farther
		return false;
	}
	
	// shoot a thing
	th = in->d.thing;
	if (th == shootthing)
		return true;			// can't shoot self
		
	if (!(th->flags & MF_SHOOTABLE))
		return true;			// corpse or something
		
	// check angles to see if the thing can be aimed at
	dist = FixedMul(attackrange, in->frac);
	thingtopslope = FixedDiv(th->z + th->height - shootz, dist);
	
	if (thingtopslope < aimslope)
		return true;			// shot over the thing
		
	thingbottomslope = FixedDiv(th->z - shootz, dist);
	
	if (thingbottomslope > aimslope)
		return true;			// shot under the thing
		
	// SoM: SO THIS IS THE PROBLEM!!!
	// heh.
	// A bullet would travel through a 3D floor until it hit a LINEDEF! Thus
	// it appears that the bullet hits the 3D floor but it actually just hits
	// the line behind it. Thus allowing a bullet to hit things under a 3D
	// floor and still be clipped a 3D floor.
	if (P_XGSVal(PGS_COSHOOTCHECKFAKEFLOOR))
		if (th->subsector->sector->ffloors)
		{
			sector_t* sector = th->subsector->sector;
			ffloor_t* rover;
		
			for (rover = sector->ffloors; rover; rover = rover->next)
			{
				if (!(rover->flags & FF_SOLID))
					continue;
				
				if (dir == -1 && *rover->topheight < lastz && *rover->topheight > th->z + th->height)
					return true;
				if (dir == 1 && *rover->bottomheight > lastz && *rover->bottomheight < th->z)
					return true;
			}
		}
	
	// hit thing
	// position a bit closer
	frac = in->frac - FixedDiv(10 * FRACUNIT, attackrange);
	
	x = trace.x + FixedMul(trace.dx, frac);
	y = trace.y + FixedMul(trace.dy, frac);
	z = shootz + FixedMul(aimslope, FixedMul(frac, attackrange));
	
	if (!P_XGSVal(PGS_CONEWBLOODHITSCANCODE))
	{
		// Spawn bullet puffs or blood spots,
		// depending on target type.
		if ((in->d.thing->flags & MF_NOBLOOD) || (Player && Player->weaponinfo[Player->readyweapon]->WeaponFlags & WF_NOBLEEDTARGET))
			P_SpawnPuff(x, y, z);
		else
			P_SpawnBlood(x, y, z, la_damage, th);
	}
	
	if (la_damage)
		hitplane = P_DamageMobj(th, shootthing, shootthing, la_damage);
	else
		hitplane = false;
		
	if (P_XGSVal(PGS_CONEWBLOODHITSCANCODE))
	{
		// Spawn bullet puffs or blood spots,
		// depending on target type.
		if ((in->d.thing->flags & MF_NOBLOOD) || (Player && Player->weaponinfo[Player->readyweapon]->WeaponFlags & WF_NOBLEEDTARGET))
			P_SpawnPuff(x, y, z);
		else
		{
			P_SpawnBlood(x, y, z, la_damage, th);	//P_SpawnPuff(x, y, z);
			
			if (hitplane)
				P_SpawnBloodSplats(x, y, z, la_damage, trace.dx, trace.dy, th);
		}
	}
	
	// GhostlyDeath <April 17, 2012> -- Through map object
	if (Args && (Args->Flags & PLAF_THRUMOBJ))
		return true;
	
	// don't go any farther
	else
		return false;
}

//
// P_AimLineAttack
//
fixed_t P_AimLineAttack(mobj_t* t1, angle_t angle, fixed_t distance, P_AimLineAtkArgs_t* const a_Args)
{
	fixed_t x2;
	fixed_t y2;
	
#ifdef PARANOIA
	if (!t1)
		I_Error("P_aimlineattack: mobj == NULL !!!");
#endif
		
	angle >>= ANGLETOFINESHIFT;
	shootthing = t1;
	
	if (P_MobjIsPlayer(t1) && P_XGSVal(PGS_CONEWAIMINGCODE))
	{
		fixed_t cosineaiming = finecosine[t1->player->aiming >> ANGLETOFINESHIFT];
		int aiming = ((int)t1->player->aiming) >> ANGLETOFINESHIFT;
		
		x2 = t1->x + FixedMul(FixedMul(distance, finecosine[angle]), cosineaiming);
		y2 = t1->y + FixedMul(FixedMul(distance, finesine[angle]), cosineaiming);
		
		topslope = 100 * FRACUNIT / 160 + finetangent[(2048 + aiming) & FINEMASK];
		bottomslope = -100 * FRACUNIT / 160 + finetangent[(2048 + aiming) & FINEMASK];
	}
	else
	{
		x2 = t1->x + (distance >> FRACBITS) * finecosine[angle];
		y2 = t1->y + (distance >> FRACBITS) * finesine[angle];
		
		//added:15-02-98: Fab comments...
		// Doom's base engine says that at a distance of 160,
		// the 2d graphics on the plane x,y correspond 1/1 with plane units
		topslope = 100 * FRACUNIT / 160;
		bottomslope = -100 * FRACUNIT / 160;
	}
	shootz = lastz = t1->z + (t1->height >> 1) + 8 * FRACUNIT;
	
	// can't shoot outside view angles
	
	attackrange = distance;
	linetarget = NULL;
	
	//added:15-02-98: comments
	// traverse all linedefs and mobjs from the blockmap containing t1,
	// to the blockmap containing the dest. point.
	// Call the function for each mobj/line on the way,
	// starting with the mobj/linedef at the shortest distance...
	P_PathTraverse(t1->x, t1->y, x2, y2, PT_ADDLINES | PT_ADDTHINGS, PTR_AimTraverse, NULL);
	
	//added:15-02-98: linetarget is only for mobjs, not for linedefs
	if (linetarget)
		return aimslope;
		
	return 0;
}

//
// P_LineAttack
// If damage == 0, it is just a test trace
// that will leave linetarget set.
//
//added:16-02-98: Fab comments...
//                t1       est l'attaquant (player ou monstre)
//                angle    est l'angle de tir sur le plan x,y (orientation)
//                distance est la porte maximale de la balle
//                slope    est la pente vers la destination (up/down)
//                damage   est les degats infliges par la balle
void P_LineAttack(mobj_t* t1, angle_t angle, fixed_t distance, fixed_t slope, int damage, P_LineAtkArgs_t* const a_Args)
{
	fixed_t x2;
	fixed_t y2;
	fixed_t cosangle;
	
	angle >>= ANGLETOFINESHIFT;
	shootthing = t1;
	la_damage = damage;
	
	// GhostlyDeath <December 28, 2012> -- Set killing method
	t1->RXAttackAttackType = PRXAT_RANGED;
	
	// player autoaimed attack,
	if (!P_XGSVal(PGS_CONEWAIMINGCODE) || !P_MobjIsPlayer(t1))
	{
		x2 = t1->x + (distance >> FRACBITS) * finecosine[angle];
		y2 = t1->y + (distance >> FRACBITS) * finesine[angle];
	}
	else
	{
		cosangle = finecosine[t1->player->aiming >> ANGLETOFINESHIFT];
		
		x2 = t1->x + FixedMul(FixedMul(distance, finecosine[angle]), cosangle);
		y2 = t1->y + FixedMul(FixedMul(distance, finesine[angle]), cosangle);
	}
	
	shootz = lastz = t1->z + (t1->height >> 1) + 8 * FRACUNIT;
	if (t1->flags2 & MF2_FEETARECLIPPED)
		shootz -= FOOTCLIPSIZE;
		
	attackrange = distance;
	aimslope = slope;
	
	tmthing = shootthing;
	
	P_PathTraverse(t1->x, t1->y, x2, y2, PT_ADDLINES | PT_ADDTHINGS, PTR_ShootTraverse, a_Args);
}

//
// USE LINES
//
mobj_t* usething;

bool_t PTR_UseTraverse(intercept_t* in, void* a_Data)
{
	int side;
	
	tmthing = NULL;
	if (!in->d.line->special)
	{
		P_LineOpening(in->d.line);
		if (openrange <= 0)
		{
			S_StartSound(&usething->NoiseThinker, sfx_noway);
			// can't use through a wall
			return false;
		}
		// not a special line, but keep checking
		return true;
	}
	
	side = 0;
	
	// TODO: verify
	if (P_PointOnLineSide(usething->x, usething->y, in->d.line) == 1)
		return false;
	//  side = 1;
	
	//  return false;           // don't use back side
	P_UseSpecialLine(usething, in->d.line, side);
	
	// can't use for than one special line in a row
	// SoM: USE MORE THAN ONE!
	if (P_XGSVal(PGS_COBOOMSUPPORT) && (in->d.line->flags & ML_PASSUSE))
		return true;
	else
		return false;
}

//
// P_UseLines
// Looks for special lines in front of the player to activate.
//
void P_UseLines(player_t* player)
{
	int angle;
	fixed_t x1;
	fixed_t y1;
	fixed_t x2;
	fixed_t y2;
	
	usething = player->mo;
	
	angle = player->mo->angle >> ANGLETOFINESHIFT;
	
	x1 = player->mo->x;
	y1 = player->mo->y;
	x2 = x1 + (USERANGE >> FRACBITS) * finecosine[angle];
	y2 = y1 + (USERANGE >> FRACBITS) * finesine[angle];
	
	P_PathTraverse(x1, y1, x2, y2, PT_ADDLINES, PTR_UseTraverse, NULL);
}

//
// RADIUS ATTACK
//
mobj_t* bombsource;
mobj_t* bombspot;
int bombdamage;

//
// PIT_RadiusAttack
// "bombsource" is the creature
// that caused the explosion at "bombspot".
//
bool_t PIT_RadiusAttack(mobj_t* thing, void* a_Arg)
{
	fixed_t dx;
	fixed_t dy;
	fixed_t dz;
	fixed_t dist;
	int damage = 0;
	fixed_t momx = 0, momy = 0;
	
	if (!(thing->flags & MF_SHOOTABLE))
		return true;
		
	// Boss spider and cyborg
	// take no damage from concussion.
		// GhostlyDeath <March 6, 2012> -- Immune to radius attacks
	if (thing->RXFlags[0] & MFREXA_RADIUSATTACKPROOF)
		return true;
		
	dx = abs(thing->x - bombspot->x);
	dy = abs(thing->y - bombspot->y);
	
	dist = dx > dy ? dx : dy;
	dist -= thing->radius;
	
	//added:22-02-98: now checks also z dist for rockets exploding
	//                above yer head...
	if (!P_XGSVal(PGS_COINFINITEROCKETZ))
	{
		dz = abs(thing->z + (thing->height >> 1) - bombspot->z);
		dist = dist > dz ? dist : dz;
	}
	dist >>= FRACBITS;
	
	if (dist < 0)
		dist = 0;
		
	if (dist >= bombdamage)
		return true;			// out of range
		
	// GhostlyDeath <June 17, 2012> -- Demo Compatibility
	if (!P_XGSVal(PGS_COBOMBTHRUFLOOR))
	{
		if (thing->floorz > bombspot->z && bombspot->ceilingz < thing->z)
			return true;
		
		if (thing->ceilingz < bombspot->z && bombspot->floorz > thing->z)
			return true;
	}
		
	if (P_CheckSight(thing, bombspot))
	{
		damage = bombdamage - dist;
		
		// GhostlyDeath <June 17, 2012> -- Demo Comp
		if (P_XGSVal(PGS_COOLDEXPLOSIONS))
		{
			P_DamageMobj(thing, bombspot, bombsource, damage);
		}
		
		// Legacy 1.32+ Explosions
		else
		{
			if (dist)
			{
				momx = (thing->x - bombspot->x) / dist;
				momy = (thing->y - bombspot->y) / dist;
			}
		
			// must be in direct path
			if (P_DamageMobj(thing, bombspot, bombsource, damage) && (thing->flags & MF_NOBLOOD) == 0 && P_XGSVal(PGS_COENABLEBLOODSPLATS))
				P_SpawnBloodSplats(thing->x, thing->y, thing->z, damage, momx, momy, thing);
		}
	}
	
	return true;
}

//
// P_RadiusAttack
// Source is the creature that caused the explosion at spot.
//
void P_RadiusAttack(mobj_t* spot, mobj_t* source, int damage)
{
	int x;
	int y;
	
	int xl;
	int xh;
	int yl;
	int yh;
	
	fixed_t dist;
	
	dist = (damage + MAXRADIUS) << FRACBITS;
	yh = (spot->y + dist - bmaporgy) >> MAPBLOCKSHIFT;
	yl = (spot->y - dist - bmaporgy) >> MAPBLOCKSHIFT;
	xh = (spot->x + dist - bmaporgx) >> MAPBLOCKSHIFT;
	xl = (spot->x - dist - bmaporgx) >> MAPBLOCKSHIFT;
	bombspot = spot;
	bombsource = source;
	bombdamage = damage;
	
	for (y = yl; y <= yh; y++)
		for (x = xl; x <= xh; x++)
			P_BlockThingsIterator(x, y, PIT_RadiusAttack, NULL);
}

/* PIT_CheckRadius() -- Check radius of object */
bool_t PIT_CheckRadius(mobj_t* thing, void* a_Arg)
{
	fixed_t blockdist;
	bool_t solid;
	int damage;
	P_PITCTSettings_t* SettingsP = a_Arg;
	
	//added:22-02-98:
	fixed_t topz;
	fixed_t tmtopz;
	
	fixed_t* Args = a_Arg;
	
	// don't clip against self
	if (!(thing->flags & (MF_SOLID | MF_SPECIAL | MF_SHOOTABLE)))
		return true;
		
	blockdist = thing->radius + Args[0];
	
	if (abs(thing->x - tmx) >= blockdist || abs(thing->y - tmy) >= blockdist)
	{
		// didn't hit it
		return true;
	}
	
	return false;
}

/* PIT_CheckRadiusLine() -- Checks line for radius */
bool_t PIT_CheckRadiusLine(line_t* ld, void* a_Arg)
{
	if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
	        || tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT] || tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM] || tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
		return true;
		
	if (P_BoxOnLineSide(tmbbox, ld) != -1)
		return true;
		
	// A line has been hit
	
	// The moving thing's destination position will cross
	// the given line.
	// If this should not be allowed, return false.
	// If the line is special, keep track of it
	// to process later if the move is proven ok.
	// NOTE: specials are NOT sorted by order,
	// so two special lines that are only 8 pixels apart
	// could be crossed in either order.
	
	// 10-12-99 BP: moved this line to out of the if so upper and
	//              lower texture can be hit by a splat
	blockingline = ld;
	if (!ld->backsector)
	{
		return false;			// one sided line
	}
	
	// set openrange, opentop, openbottom
	P_LineOpening(ld);
	
	// adjust floor / ceiling heights
	if (opentop < tmceilingz)
	{
		tmsectorceilingz = tmceilingz = opentop;
		ceilingline = ld;
	}
	
	if (openbottom > tmfloorz)
		tmsectorfloorz = tmfloorz = openbottom;
		
	if (lowfloor < tmdropoffz)
		tmdropoffz = lowfloor;
		
	return true;
}

/* P_CheckPosRadius() -- Checks for solid objects within radius */
// Based off P_CheckPosition() somewhat
bool_t P_CheckPosRadius(fixed_t x, fixed_t y, fixed_t Radius)
{	
	int xl;
	int xh;
	int yl;
	int yh;
	
	fixed_t bx, by;
	
	fixed_t dist;
	fixed_t Args[3];
	
	Args[0] = Radius;
	
	tmbbox[BOXTOP] = y + Radius;
	tmbbox[BOXBOTTOM] = y - Radius;
	tmbbox[BOXRIGHT] = x + Radius;
	tmbbox[BOXLEFT] = x - Radius;

	xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
	xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
	yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
	yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;
	
	tmx = x;
	tmy = y;
	
	for (bx = xl; bx <= xh; bx++)
		for (by = yl; by <= yh; by++)
			if (!P_BlockThingsIterator(bx, by, PIT_CheckRadius, Args))
				return false;
	
	// check lines
	xl = (tmbbox[BOXLEFT] - bmaporgx) >> MAPBLOCKSHIFT;
	xh = (tmbbox[BOXRIGHT] - bmaporgx) >> MAPBLOCKSHIFT;
	yl = (tmbbox[BOXBOTTOM] - bmaporgy) >> MAPBLOCKSHIFT;
	yh = (tmbbox[BOXTOP] - bmaporgy) >> MAPBLOCKSHIFT;
	
	for (bx = xl; bx <= xh; bx++)
		for (by = yl; by <= yh; by++)
			if (!P_BlockLinesIterator(bx, by, PIT_CheckRadiusLine, NULL))
				return false;
	
	return true;
}

//
// SECTOR HEIGHT CHANGING
// After modifying a sectors floor or ceiling height,
// call this routine to adjust the positions
// of all things that touch the sector.
//
// If anything doesn't fit anymore, true will be returned.
// If crunch is true, they will take damage
//  as they are being crushed.
// If Crunch is false, you should set the sector height back
//  the way it was and call P_ChangeSector again
//  to undo the changes.
//
bool_t crushchange;
bool_t nofit;
sector_t* sectorchecked;

//
// PIT_ChangeSector
//
bool_t PIT_ChangeSector(mobj_t* thing, void* a_Arg)
{
	mobj_t* mo;
	PI_mobjid_t GibsType;
	
	if (P_ThingHeightClip(thing))
	{
		// keep checking
		return true;
	}
	// crunch bodies to giblets
	if (thing->flags & MF_CORPSE)
	{
		GibsType = INFO_GetTypeByName("CrushedGibs"); 
		if (GibsType != NUMMOBJTYPES)
			P_SetMobjState(thing, mobjinfo[GibsType]->spawnstate);
		
		thing->flags &= ~MF_SOLID;
		//added:22-02-98: lets have a neat 'crunch' sound!
		S_StartSound(&thing->NoiseThinker, sfx_slop);
		
		thing->height = 0;
		thing->radius = 0;
		thing->skin = 0;
		
		// keep checking
		return true;
	}
	// crunch dropped items
	if (thing->flags & MF_DROPPED)
	{
		P_RemoveMobj(thing);
		
		// keep checking
		return true;
	}
	
	if (!(thing->flags & MF_SHOOTABLE))
	{
		// assume it is bloody gibs or something
		return true;
	}
	
	nofit = true;
	
	// GhostlyDeath <April 12, 2012> -- Whoops, I thought Legacy changed the
	// way crusher blood worked, turns out it didn't. It seems I never actually
	// realized that (x & 3) and (x % 4) are exactly the same. However, Legacy
	// did add some added logic to the blood spraying. Legacy sprays a little
	// less blood. The only difference here is that 
	if (crushchange && !(leveltime & 3))
	{
		// Regardless, things always get hurt here
		P_DamageMobj(thing, NULL, NULL, 10);
		
		// Before Legacy 1.32, there was more blood, and everything bled, even barrels!
		if (P_XGSVal(PGS_COMORECRUSHERBLOOD) || ((!(leveltime % (16)) && !(thing->flags & MF_NOBLOOD))))
		{
			// spray blood in a random direction
			mo = P_SpawnMobj(thing->x, thing->y, thing->z + thing->height / 2, INFO_GetTypeByName(__REMOOD_GETBLOODKIND));
			
			// Signed random because P_Random() is now uint8_t!
			mo->momx = P_SignedRandom() << 12;
			mo->momy = P_SignedRandom() << 12;
		}
	}
	
	// keep checking (crush other things)
	return true;
}

//
// P_ChangeSector
//
bool_t P_ChangeSector(sector_t* sector, bool_t crunch)
{
	int x;
	int y;
	
	nofit = false;
	crushchange = crunch;
	sectorchecked = sector;
	
	// re-check heights for all things near the moving sector
	for (x = sector->blockbox[BOXLEFT]; x <= sector->blockbox[BOXRIGHT]; x++)
		for (y = sector->blockbox[BOXBOTTOM]; y <= sector->blockbox[BOXTOP]; y++)
			P_BlockThingsIterator(x, y, PIT_ChangeSector, NULL);
			
	return nofit;
}

//SoM: 3/15/2000: New function. Much faster.
bool_t P_CheckSector(sector_t* sector, bool_t crunch)
{
	msecnode_t* n;
	
	if (!P_XGSVal(PGS_COBOOMSUPPORT))			// use the old routine for old demos though
		return P_ChangeSector(sector, crunch);
		
	nofit = false;
	crushchange = crunch;
	
	// killough 4/4/98: scan list front-to-back until empty or exhausted,
	// restarting from beginning after each thing is processed. Avoids
	// crashes, and is sure to examine all things in the sector, and only
	// the things which are in the sector, until a steady-state is reached.
	// Things can arbitrarily be inserted and removed and it won't mess up.
	//
	// killough 4/7/98: simplified to avoid using complicated counter
	
	if (sector->numattached)
	{
		int i;
		sector_t* sec;
		
		for (i = 0; i < sector->numattached; i++)
		{
			sec = &sectors[sector->attached[i]];
			for (n = sec->touching_thinglist; n; n = n->m_snext)
				n->visited = false;
				
			sec->moved = true;
			
			do
			{
				for (n = sec->touching_thinglist; n; n = n->m_snext)
					if (!n->visited)
					{
						n->visited = true;
						if (!(n->m_thing->flags & MF_NOBLOCKMAP))
							PIT_ChangeSector(n->m_thing, NULL);
						break;
					}
			}
			while (n);
		}
	}
	// Mark all things invalid
	sector->moved = true;
	
	for (n = sector->touching_thinglist; n; n = n->m_snext)
		n->visited = false;
		
	do
	{
		for (n = sector->touching_thinglist; n; n = n->m_snext)	// go through list
			if (!n->visited)	// unprocessed thing found
			{
				n->visited = true;	// mark thing as processed
				if (!(n->m_thing->flags & MF_NOBLOCKMAP))	//jff 4/7/98 don't do these
					PIT_ChangeSector(n->m_thing, NULL);	// process it
				break;			// exit and start over
			}
	}
	while (n);					// repeat from scratch until all things left are marked valid
	
	return nofit;
}

/*
  SoM: 3/15/2000
  Lots of new Boom functions that work faster and add functionality.
*/

static msecnode_t* headsecnode = NULL;

// GhostlyDeath <May 10, 2012> -- Sector Node List (Save Game Support)
msecnode_t** g_MSecNodes = NULL;				// Active sector nodes
size_t g_NumMSecNodes = 0;						// Number of sector nodes

void P_Initsecnode(void)
{
	headsecnode = NULL;

	// GhostlyDeath <May 10, 2012> -- Sector Node List (Save Game Support)
	g_MSecNodes = NULL;
	g_NumMSecNodes = 0;
}

// P_GetSecnode() retrieves a node from the freelist. The calling routine
// should make sure it sets all fields properly.

msecnode_t* P_GetSecnode()
{
	msecnode_t* node;
	
	if (headsecnode)
	{
		node = headsecnode;
		headsecnode = headsecnode->m_snext;
	}
	
	else
	{
		node = Z_Malloc(sizeof(*node), PU_LEVEL, NULL);
		
		// GhostlyDeath <May 10, 2012> -- Sector Node List (Save Game Support)
			// Sector nodes never get freed, so this list will collect with any
			// new nodes that do get created.
		Z_ResizeArray((void**)&g_MSecNodes, sizeof(*g_MSecNodes),
						g_NumMSecNodes, g_NumMSecNodes + 1);
		g_MSecNodes[g_NumMSecNodes++] = node;
		
		//if (devparm)
		//	CONL_PrintF("P_GetSecnode: New %p @ %u\n", node, (unsigned)g_NumMSecNodes - 1);
		
		// Make sure it gets deleted!
		Z_ChangeTag(g_MSecNodes, PU_LEVEL);
	}
	
	return (node);
}

/* P_GetIDFromSecNode() -- Returns ID of sector node */
int32_t P_GetIDFromSecNode(msecnode_t* const a_Node)
{
	int32_t i;
	
	/* Check */
	if (!a_Node)
		return -1;
	
	/* Go through list */
	for (i = 0; i < g_NumMSecNodes; i++)
		if (g_MSecNodes[i] == a_Node)
			return i;
	
	/* Not found */
	return -1;
}

/* P_GetSecNodeFromID() -- Returns sector node from ID */
msecnode_t* P_GetSecNodeFromID(const int32_t a_ID)
{
	/* Check */
	if (a_ID < 0 || a_ID >= g_NumMSecNodes)
		return NULL;
	
	/* Return reference of it */
	return g_MSecNodes[a_ID];
}

// P_PutSecnode() returns a node to the freelist.

void P_PutSecnode(msecnode_t* node)
{
	node->m_snext = headsecnode;
	headsecnode = node;
}

// P_AddSecnode() searches the current list to see if this sector is
// already there. If not, it adds a sector node at the head of the list of
// sectors this object appears in. This is called when creating a list of
// nodes that will get linked in later. Returns a pointer to the new node.

msecnode_t* P_AddSecnode(sector_t* s, mobj_t* thing, msecnode_t* nextnode)
{
	msecnode_t* node;
	
	node = nextnode;
	while (node)
	{
		if (node->m_sector == s)	// Already have a node for this sector?
		{
			node->m_thing = thing;	// Yes. Setting m_thing says 'keep it'.
			return (nextnode);
		}
		node = node->m_tnext;
	}
	
	// Couldn't find an existing node for this sector. Add one at the head
	// of the list.
	
	node = P_GetSecnode();
	
	//mark new nodes unvisited.
	node->visited = 0;
	
	node->m_sector = s;			// sector
	node->m_thing = thing;		// mobj
	node->m_tprev = NULL;		// prev node on Thing thread
	node->m_tnext = nextnode;	// next node on Thing thread
	if (nextnode)
		nextnode->m_tprev = node;	// set back link on Thing
		
	// Add new node at head of sector thread starting at s->touching_thinglist
	
	node->m_sprev = NULL;		// prev node on sector thread
	node->m_snext = s->touching_thinglist;	// next node on sector thread
	if (s->touching_thinglist)
		node->m_snext->m_sprev = node;
	s->touching_thinglist = node;
	return (node);
}

// P_DelSecnode() deletes a sector node from the list of
// sectors this object appears in. Returns a pointer to the next node
// on the linked list, or NULL.

msecnode_t* P_DelSecnode(msecnode_t* node)
{
	msecnode_t* tp;				// prev node on thing thread
	msecnode_t* tn;				// next node on thing thread
	msecnode_t* sp;				// prev node on sector thread
	msecnode_t* sn;				// next node on sector thread
	
	if (node)
	{
	
		// Unlink from the Thing thread. The Thing thread begins at
		// sector_list and not from mobj_t->touching_sectorlist.
		
		tp = node->m_tprev;
		tn = node->m_tnext;
		if (tp)
			tp->m_tnext = tn;
		if (tn)
			tn->m_tprev = tp;
			
		// Unlink from the sector thread. This thread begins at
		// sector_t->touching_thinglist.
		
		sp = node->m_sprev;
		sn = node->m_snext;
		if (sp)
			sp->m_snext = sn;
		else
			node->m_sector->touching_thinglist = sn;
		if (sn)
			sn->m_sprev = sp;
			
		// Return this node to the freelist
		
		P_PutSecnode(node);
		return (tn);
	}
	return (NULL);
}

// Delete an entire sector list

void P_DelSeclist(msecnode_t* node)
{
	while (node)
		node = P_DelSecnode(node);
}

// PIT_GetSectors
// Locates all the sectors the object is in by looking at the lines that
// cross through it. You have already decided that the object is allowed
// at this location, so don't bother with checking impassable or
// blocking lines.

bool_t PIT_GetSectors(line_t* ld, void* a_Arg)
{
	if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT] ||
	        tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT] || tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM] || tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
		return true;
		
	if (P_BoxOnLineSide(tmbbox, ld) != -1)
		return true;
		
	// This line crosses through the object.
	
	// Collect the sector(s) from the line and add to the
	// sector_list you're examining. If the Thing ends up being
	// allowed to move to this position, then the sector_list
	// will be attached to the Thing's mobj_t at touching_sectorlist.
	
	sector_list = P_AddSecnode(ld->frontsector, tmthing, sector_list);
	
	// Don't assume all lines are 2-sided, since some Things
	// like "TeleportFog" are allowed regardless of whether their radius takes
	// them beyond an impassable linedef.
	
	// Use sidedefs instead of 2s flag to determine two-sidedness.
	
	if (ld->backsector)
		sector_list = P_AddSecnode(ld->backsector, tmthing, sector_list);
		
	return true;
}

// P_CreateSecNodeList alters/creates the sector_list that shows what sectors
// the object resides in.

void P_CreateSecNodeList(mobj_t* thing, fixed_t x, fixed_t y)
{
	int xl;
	int xh;
	int yl;
	int yh;
	int bx;
	int by;
	msecnode_t* node;
	
	// First, clear out the existing m_thing fields. As each node is
	// added or verified as needed, m_thing will be set properly. When
	// finished, delete all nodes where m_thing is still NULL. These
	// represent the sectors the Thing has vacated.
	
	node = sector_list;
	while (node)
	{
		node->m_thing = NULL;
		node = node->m_tnext;
	}
	
	tmthing = thing;
	tmflags = thing->flags;
	
	tmx = x;
	tmy = y;
	
	tmbbox[BOXTOP] = y + tmthing->radius;
	tmbbox[BOXBOTTOM] = y - tmthing->radius;
	tmbbox[BOXRIGHT] = x + tmthing->radius;
	tmbbox[BOXLEFT] = x - tmthing->radius;
	
	validcount++;				// used to make sure we only process a line once
	
	xl = (tmbbox[BOXLEFT] - bmaporgx) >> MAPBLOCKSHIFT;
	xh = (tmbbox[BOXRIGHT] - bmaporgx) >> MAPBLOCKSHIFT;
	yl = (tmbbox[BOXBOTTOM] - bmaporgy) >> MAPBLOCKSHIFT;
	yh = (tmbbox[BOXTOP] - bmaporgy) >> MAPBLOCKSHIFT;
	
	for (bx = xl; bx <= xh; bx++)
		for (by = yl; by <= yh; by++)
			P_BlockLinesIterator(bx, by, PIT_GetSectors, NULL);
			
	// Add the sector of the (x,y) point to sector_list.
	
	sector_list = P_AddSecnode(thing->subsector->sector, thing, sector_list);
	
	// Now delete any nodes that won't be used. These are the ones where
	// m_thing is still NULL.
	
	node = sector_list;
	while (node)
	{
		if (node->m_thing == NULL)
		{
			if (node == sector_list)
				sector_list = node->m_tnext;
			node = P_DelSecnode(node);
		}
		else
			node = node->m_tnext;
	}
}

// heretic code

//---------------------------------------------------------------------------
//
// PIT_CheckOnmobjZ
//
//---------------------------------------------------------------------------
mobj_t* onmobj;					//generic global onmobj...used for landing on pods/players

static bool_t PIT_CheckOnmobjZ(mobj_t* thing, void* a_Arg)
{
	fixed_t blockdist;
	
	if (!(thing->flags & (MF_SOLID | MF_SPECIAL | MF_SHOOTABLE)))
	{
		// Can't hit thing
		return (true);
	}
	blockdist = thing->radius + tmthing->radius;
	if (abs(thing->x - tmx) >= blockdist || abs(thing->y - tmy) >= blockdist)
	{
		// Didn't hit thing
		return (true);
	}
	if (thing == tmthing)
	{
		// Don't clip against self
		return (true);
	}
	if (tmthing->z > thing->z + thing->height)
	{
		return (true);
	}
	else if (tmthing->z + tmthing->height < thing->z)
	{
		// under thing
		return (true);
	}
	if (thing->flags & MF_SOLID)
	{
		onmobj = thing;
	}
	return (!(thing->flags & MF_SOLID));
}

//=============================================================================
//
// P_FakeZMovement
//
//              Fake the zmovement so that we can check if a move is legal
//=============================================================================

static void P_FakeZMovement(mobj_t* mo)
{
	int dist;
	int delta;
	
//
// adjust height
//
	mo->z += mo->momz;
	if (mo->flags & MF_FLOAT && mo->target)
	{
		// float down towards target if too close
		if (!(mo->flags & MF_SKULLFLY) && !(mo->flags & MF_INFLOAT))
		{
			dist = P_AproxDistance(mo->x - mo->target->x, mo->y - mo->target->y);
			delta = (mo->target->z + (mo->height >> 1)) - mo->z;
			if (delta < 0 && dist < -(delta * 3))
				mo->z -= FLOATSPEED;
			else if (delta > 0 && dist < (delta * 3))
				mo->z += FLOATSPEED;
		}
	}
	
	if (P_MobjIsPlayer(mo) && mo->flags2& MF2_FLY && !(mo->z <= mo->floorz) && leveltime & 2)
		mo->z += finesine[(FINEANGLES / 20 * leveltime >> 2) & FINEMASK];
//
// clip movement
//
	if (mo->z <= mo->floorz)
	{
		// Hit the floor
		mo->z = mo->floorz;
		if (mo->momz < 0)
		{
			mo->momz = 0;
		}
		if (mo->flags & MF_SKULLFLY)
		{
			// The skull slammed into something
			mo->momz = -mo->momz;
		}
		if (mo->info->crashstate && (mo->flags & MF_CORPSE))
		{
			return;
		}
	}
	else if (mo->flags2 & MF2_LOGRAV)
	{
		if (mo->momz == 0)
			mo->momz = -(P_XGSVal(PGS_GAMEGRAVITY) >> 3) * 2;
		else
			mo->momz -= P_XGSVal(PGS_GAMEGRAVITY) >> 3;
	}
	else if (!(mo->flags & MF_NOGRAVITY))
	{
		if (mo->momz == 0)
			mo->momz = -P_XGSVal(PGS_GAMEGRAVITY) * 2;
		else
			mo->momz -= P_XGSVal(PGS_GAMEGRAVITY);
	}
	
	if (mo->z + mo->height > mo->ceilingz)
	{
		// hit the ceiling
		if (mo->momz > 0)
			mo->momz = 0;
		mo->z = mo->ceilingz - mo->height;
		if (mo->flags & MF_SKULLFLY)
		{
			// the skull slammed into something
			mo->momz = -mo->momz;
		}
	}
}

//=============================================================================
//
// P_CheckOnmobj(mobj_t *thing)
//
//              Checks if the new Z position is legal
//=============================================================================

mobj_t* P_CheckOnmobj(mobj_t* thing)
{
	int xl, xh, yl, yh, bx, by;
	subsector_t* newsubsec;
	fixed_t x;
	fixed_t y;
	mobj_t oldmo;
	
	x = thing->x;
	y = thing->y;
	tmthing = thing;
	tmflags = thing->flags;
	oldmo = *thing;				// save the old mobj before the fake zmovement
	P_FakeZMovement(tmthing);
	
	tmx = x;
	tmy = y;
	
	tmbbox[BOXTOP] = y + tmthing->radius;
	tmbbox[BOXBOTTOM] = y - tmthing->radius;
	tmbbox[BOXRIGHT] = x + tmthing->radius;
	tmbbox[BOXLEFT] = x - tmthing->radius;
	
	newsubsec = R_PointInSubsector(x, y);
	ceilingline = NULL;
	
	//
	// the base floor / ceiling is from the subsector that contains the
	// point.  Any contacted lines the step closer together will adjust them
	//
	tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
	tmceilingz = newsubsec->sector->ceilingheight;
	
	validcount++;
	numspechit = 0;
	
	if (tmflags & MF_NOCLIP)
		return NULL;
		
	//
	// check things first, possibly picking things up
	// the bounding box is extended by MAXRADIUS because mobj_ts are grouped
	// into mapblocks based on their origin point, and can overlap into adjacent
	// blocks by up to MAXRADIUS units
	//
	xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
	xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
	yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
	yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;
	
	for (bx = xl; bx <= xh; bx++)
		for (by = yl; by <= yh; by++)
			if (!P_BlockThingsIterator(bx, by, PIT_CheckOnmobjZ, NULL))
			{
				*tmthing = oldmo;
				return onmobj;
			}
			
	*tmthing = oldmo;
	return NULL;
}

//----------------------------------------------------------------------------
//
// FUNC P_TestMobjLocation
//
// Returns true if the mobj is not blocked by anything at its current
// location, otherwise returns false.
//
//----------------------------------------------------------------------------

bool_t P_TestMobjLocation(mobj_t* mobj)
{
	int flags;
	
	flags = mobj->flags;
	mobj->flags &= ~MF_PICKUP;
	if (P_CheckPosition(mobj, mobj->x, mobj->y, 0))
	{
		// XY is ok, now check Z
		mobj->flags = flags;
		if ((mobj->z < mobj->floorz) || (mobj->z + mobj->height > mobj->ceilingz))
		{
			// Bad Z
			return (false);
		}
		return (true);
	}
	mobj->flags = flags;
	return (false);
}

/*****************************************************************************/

/* P_ZAtPointSS() -- Z for point, in subsector */
fixed_t P_ZAtPointSS(subsector_t* const a_SubS, const bool_t a_Ceil, const fixed_t a_X, const fixed_t a_Y, const fixed_t a_Z)
{
	/* Normal Sector */
	if (a_Ceil)
		return a_SubS->sector->ceilingheight;
	else
		return a_SubS->sector->floorheight;
}

/* P_ZAtPoint() -- Z at specified point */
fixed_t P_ZAtPoint(const bool_t a_Ceil, const fixed_t a_X, const fixed_t a_Y, const fixed_t a_Z)
{
	/* Call subsector variant at point */
	return P_ZAtPointSS(R_PointInSubsector(a_X, a_Y), a_Ceil, a_X, a_Y, a_Z);
}


