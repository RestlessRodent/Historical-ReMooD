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
// Copyright (C) 2008-2011 GhostlyDeath (ghostlydeath@gmail.com)
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
//      Moving object handling. Spawn functions.

#include "doomdef.h"
#include "g_game.h"
#include "g_input.h"
#include "st_stuff.h"
#include "hu_stuff.h"
#include "p_local.h"
#include "p_inter.h"
#include "p_setup.h"			//levelflats to test if mobj in water sector
#include "r_main.h"
#include "r_things.h"
#include "r_sky.h"
#include "s_sound.h"
#include "z_zone.h"
#include "m_random.h"
#include "d_clisrv.h"
#include "r_splats.h"			//faB: in dev.
#include "p_demcmp.h"

// protos.
CV_PossibleValue_t viewheight_cons_t[] = { {16, "MIN"}
	, {56, "MAX"}
	, {0, NULL}
};
CV_PossibleValue_t maxsplats_cons_t[] = { {1, "MIN"}
	, {MAXLEVELSPLATS, "MAX"}
	, {0, NULL}
};

consvar_t cv_viewheight = { "viewheight", VIEWHEIGHTS, 0, viewheight_cons_t, NULL };

//Fab:26-07-98:
consvar_t cv_gravity = { "gravity", "1", CV_NETVAR | CV_FLOAT | CV_SHOWMODIF };
consvar_t cv_splats = { "splats", "1", CV_SAVE, CV_OnOff };
consvar_t cv_maxsplats = { "maxsplats", "512", CV_SAVE, maxsplats_cons_t, NULL };
consvar_t cv_classicblood = { "classicblood", "0", CV_NETVAR | CV_SAVE, CV_YesNo, NULL };

static const fixed_t FloatBobOffsets[64] =
{
	0, 51389, 102283, 152192,
	200636, 247147, 291278, 332604,
	370727, 405280, 435929, 462380,
	484378, 501712, 514213, 521763,
	524287, 521763, 514213, 501712,
	484378, 462380, 435929, 405280,
	370727, 332604, 291278, 247147,
	200636, 152192, 102283, 51389,
	-1, -51390, -102284, -152193,
	-200637, -247148, -291279, -332605,
	-370728, -405281, -435930, -462381,
	-484380, -501713, -514215, -521764,
	-524288, -521764, -514214, -501713,
	-484379, -462381, -435930, -405280,
	-370728, -332605, -291279, -247148,
	-200637, -152193, -102284, -51389
};

//
// P_SetMobjState
// Returns true if the mobj is still present.
//
//SoM: 4/7/2000: Boom code...
bool_t P_SetMobjState(mobj_t* mobj, statenum_t state)
{
	state_t* st;
	
	//remember states seen, to detect cycles:
	
	static statenum_t seenstate_tab[NUMSTATES];	// fast transition table
	statenum_t* seenstate = seenstate_tab;	// pointer to table
	static int recursion;		// detects recursion
	statenum_t i = state;		// initial state
	bool_t ret = true;			// return value
	statenum_t tempstate[NUMSTATES];	// for use with recursion
	
	if (recursion++)			// if recursion detected,
		memset(seenstate = tempstate, 0, sizeof tempstate);	// clear state table
		
	do
	{
		if (state == S_NULL)
		{
			mobj->state = (state_t*) S_NULL;
			P_RemoveMobj(mobj);
			ret = false;
			break;				// killough 4/9/98
		}
		
		st = &states[state];
		mobj->state = st;
		mobj->tics = st->tics;
		mobj->sprite = st->sprite;
		mobj->frame = st->frame;
		
		// Modified handling.
		// Call action functions when the state is set
		
		if (st->action.acp1)
			st->action.acp1(mobj);
			
		seenstate[state] = 1 + st->nextstate;	// killough 4/9/98
		
		state = st->nextstate;
	}
	while (!mobj->tics && !seenstate[state]);	// killough 4/9/98
	
	if (ret && !mobj->tics)		// killough 4/9/98: detect state cycles
		CONS_Printf("Warning: State Cycle Detected");
		
	if (!--recursion)
		for (; (state = seenstate[i]); i = state - 1)
			seenstate[i] = 0;	// killough 4/9/98: erase memory of states
			
	return ret;
}

//----------------------------------------------------------------------------
//
// FUNC P_SetMobjStateNF
//
// Same as P_SetMobjState, but does not call the state function.
//
//----------------------------------------------------------------------------

bool_t P_SetMobjStateNF(mobj_t* mobj, statenum_t state)
{
	state_t* st;
	
	if (state == S_NULL)
	{
		// Remove mobj
		P_RemoveMobj(mobj);
		return (false);
	}
	st = &states[state];
	mobj->state = st;
	mobj->tics = st->tics;
	mobj->sprite = st->sprite;
	mobj->frame = st->frame;
	return (true);
}

//
// P_ExplodeMissile
//
void P_ExplodeMissile(mobj_t* mo)
{
	mo->momx = mo->momy = mo->momz = 0;
	
	P_SetMobjState(mo, mobjinfo[mo->type].deathstate);
	
	mo->tics -= P_Random() & 3;
	
	if (mo->tics < 1)
		mo->tics = 1;
		
	mo->flags &= ~MF_MISSILE;
	
	if (mo->info->deathsound)
		S_StartSound(&mo->NoiseThinker, mo->info->deathsound);
}

//----------------------------------------------------------------------------
//
// PROC P_FloorBounceMissile
//
//----------------------------------------------------------------------------

void P_FloorBounceMissile(mobj_t* mo)
{
	mo->momz = -mo->momz;
	P_SetMobjState(mo, mobjinfo[mo->type].deathstate);
}

//----------------------------------------------------------------------------
//
// PROC P_ThrustMobj
//
//----------------------------------------------------------------------------

void P_ThrustMobj(mobj_t* mo, angle_t angle, fixed_t move)
{
	angle >>= ANGLETOFINESHIFT;
	mo->momx += FixedMul(move, finecosine[angle]);
	mo->momy += FixedMul(move, finesine[angle]);
}

//
// P_XYMovement
//
#define STOPSPEED               (0x1000)
#define FRICTION                0xe800	//0.90625
#define FRICTION_LOW            0xf900
#define FRICTION_FLY            0xeb00

//added:22-02-98: adds friction on the xy plane
void P_XYFriction(mobj_t* mo, fixed_t oldx, fixed_t oldy, bool_t oldfriction)
{
	//valid only if player avatar
	player_t* player = mo->player;
	
	if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED && mo->momy > -STOPSPEED
	        && mo->momy < STOPSPEED && (!player || (player->cmd.forwardmove == 0 && player->cmd.sidemove == 0)))
	{
		// if in a walking frame, stop moving
		if (player && (mo->type != MT_SPIRIT))
		{
			if ((unsigned)((player->mo->state - states) - S_PLAY_RUN1) < 4)
				P_SetMobjState(player->mo, S_PLAY);
		}
		
		mo->momx = 0;
		mo->momy = 0;
	}
	else
	{
		if (oldfriction)
		{
			mo->momx = FixedMul(mo->momx, FRICTION);
			mo->momy = FixedMul(mo->momy, FRICTION);
		}
		else
		{
			//SoM: 3/28/2000: Use boom friction.
			if ((oldx == mo->x) && (oldy == mo->y))	// Did you go anywhere?
			{
				mo->momx = FixedMul(mo->momx, ORIG_FRICTION);
				mo->momy = FixedMul(mo->momy, ORIG_FRICTION);
			}
			else
			{
				mo->momx = FixedMul(mo->momx, mo->friction);
				mo->momy = FixedMul(mo->momy, mo->friction);
			}
			mo->friction = ORIG_FRICTION;
		}
	}
}

void P_XYMovement(mobj_t* mo)
{
	fixed_t ptryx;
	fixed_t ptryy;
	player_t* player;
	fixed_t xmove;
	fixed_t ymove;
	fixed_t oldx, oldy;			//reducing bobbing/momentum on ice
	
	//when up against walls
	static int windTab[3] = { 2048 * 5, 2048 * 10, 2048 * 25 };
	
	//added:18-02-98: if it's stopped
	if (!mo->momx && !mo->momy)
	{
		if (mo->flags & MF_SKULLFLY)
		{
			// the skull slammed into something
			mo->flags &= ~MF_SKULLFLY;
			mo->momx = mo->momy = mo->momz = 0;
			
			//added:18-02-98: comment: set in 'search new direction' state?
			P_SetMobjState(mo, mo->info->spawnstate);
		}
		return;
	}
	if (mo->flags2 & MF2_WINDTHRUST)
	{
		int special = mo->subsector->sector->special;
		
		switch (special)
		{
			case 40:
			case 41:
			case 42:			// Wind_East
				P_ThrustMobj(mo, 0, windTab[special - 40]);
				break;
			case 43:
			case 44:
			case 45:			// Wind_North
				P_ThrustMobj(mo, ANG90, windTab[special - 43]);
				break;
			case 46:
			case 47:
			case 48:			// Wind_South
				P_ThrustMobj(mo, ANG270, windTab[special - 46]);
				break;
			case 49:
			case 50:
			case 51:			// Wind_West
				P_ThrustMobj(mo, ANG180, windTab[special - 49]);
				break;
		}
	}
	
	player = mo->player;		//valid only if player avatar
	
	if (mo->momx > MAXMOVE)
		mo->momx = MAXMOVE;
	else if (mo->momx < -MAXMOVE)
		mo->momx = -MAXMOVE;
		
	if (mo->momy > MAXMOVE)
		mo->momy = MAXMOVE;
	else if (mo->momy < -MAXMOVE)
		mo->momy = -MAXMOVE;
		
	xmove = mo->momx;
	ymove = mo->momy;
	
	oldx = mo->x;
	oldy = mo->y;
	
	do
	{
		if (xmove > MAXMOVE / 2 || ymove > MAXMOVE / 2)
		{
			ptryx = mo->x + xmove / 2;
			ptryy = mo->y + ymove / 2;
			xmove >>= 1;
			ymove >>= 1;
		}
		else
		{
			ptryx = mo->x + xmove;
			ptryy = mo->y + ymove;
			xmove = ymove = 0;
		}
		
		if (!P_TryMove(mo, ptryx, ptryy, true))	//SoM: 4/10/2000
		{
			// blocked move
			
			// gameplay issue : let the marine move forward while trying
			//                  to jump over a small wall
			//    (normally it can not 'walk' while in air)
			// BP:1.28 no more use Cf_JUMPOVER, but i leave it for backward lmps compatibility
			if (mo->player)
			{
				if (tmfloorz - mo->z > MAXSTEPMOVE)
				{
					if (mo->momz > 0)
						mo->player->cheats |= CF_JUMPOVER;
					else
						mo->player->cheats &= ~CF_JUMPOVER;
				}
			}
			
			if (mo->flags2 & MF2_SLIDE)
			{
				// try to slide along it
				P_SlideMove(mo);
			}
			else if (mo->flags & MF_MISSILE)
			{
				// explode a missile
				if (ceilingline && ceilingline->backsector &&
				        ceilingline->backsector->ceilingpic == skyflatnum &&
				        ceilingline->frontsector && ceilingline->frontsector->ceilingpic == skyflatnum && mo->subsector->sector->ceilingheight == mo->ceilingz)
					if (!boomsupport || mo->z > ceilingline->backsector->ceilingheight)	//SoM: 4/7/2000: DEMO'S
					{
						// Hack to prevent missiles exploding
						// against the sky.
						// Does not handle sky floors.
						//SoM: 4/3/2000: Check frontsector as well..
						P_RemoveMobj(mo);
						return;
					}
				// draw damage on wall
				//SPLAT TEST ----------------------------------------------------------
#ifdef WALLSPLATS
				if (blockingline && demoversion >= 129)	//set by last P_TryMove() that failed
				{
					divline_t divl;
					divline_t misl;
					fixed_t frac;
					
					P_MakeDivline(blockingline, &divl);
					misl.x = mo->x;
					misl.y = mo->y;
					misl.dx = mo->momx;
					misl.dy = mo->momy;
					frac = P_InterceptVector(&divl, &misl);
					R_AddWallSplat(blockingline, P_PointOnLineSide(mo->x, mo->y, blockingline), "A_DMG3", mo->z, frac, SPLATDRAWMODE_SHADE);
				}
#endif
				// --------------------------------------------------------- SPLAT TEST
				
				P_ExplodeMissile(mo);
			}
			else
				mo->momx = mo->momy = 0;
		}
		else
			// hack for playability : walk in-air to jump over a small wall
			if (mo->player)
				mo->player->cheats &= ~CF_JUMPOVER;
				
	}
	while (xmove || ymove);
	
	// slow down
	if (player)
	{
		if (player->cheats & CF_NOMOMENTUM)
		{
			// debug option for no sliding at all
			mo->momx = mo->momy = 0;
			return;
		}
		else if ((player->cheats & CF_FLYAROUND) || (player->mo->flags2 & MF2_FLY))
		{
			P_XYFriction(mo, oldx, oldy, true);
			return;
		}
//        if(mo->z <= mo->subsector->sector->floorheight)
//          P_XYFriction (mo, oldx, oldy, false);
	}
	
	if (mo->flags & (MF_MISSILE | MF_SKULLFLY))
		return;					// no friction for missiles ever
		
	// slow down in water, not too much for playability issues
	if (demoversion >= 128 && (mo->eflags & MF_UNDERWATER))
	{
		mo->momx = FixedMul(mo->momx, FRICTION * 3 / 4);
		mo->momy = FixedMul(mo->momy, FRICTION * 3 / 4);
		return;
	}
	
	if (mo->z > mo->floorz && !(mo->flags2 & MF2_FLY) && !(mo->flags2 & MF2_ONMOBJ))
		return;					// no friction when airborne
		
	if (mo->flags & MF_CORPSE)
	{
		// do not stop sliding
		//  if halfway off a step with some momentum
		if (mo->momx > FRACUNIT / 4 || mo->momx < -FRACUNIT / 4 || mo->momy > FRACUNIT / 4 || mo->momy < -FRACUNIT / 4)
		{
			if (demoversion < 132)
			{
				if (mo->z != mo->subsector->sector->floorheight)
					return;
			}
			else
			{
				if (mo->z != mo->floorz)
					return;
			}
		}
	}
	P_XYFriction(mo, oldx, oldy, demoversion < 132);
}

//
// P_ZMovement
//
void P_ZMovement(mobj_t* mo)
{
	fixed_t dist;
	fixed_t delta;
	
// Intercept the stupid 'fall through 3dfloors' bug SSNTails 06-13-2002
	if (mo->subsector->sector->ffloors)
	{
		ffloor_t* rover;
		fixed_t delta1;
		fixed_t delta2;
		int thingtop = mo->z + mo->height;
		
		for (rover = mo->subsector->sector->ffloors; rover; rover = rover->next)
		{
			if (!(rover->flags & FF_SOLID) || !(rover->flags & FF_EXISTS))
				continue;
				
			delta1 = mo->z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight) / 2));
			delta2 = thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight) / 2));
			if (*rover->topheight > mo->floorz && abs(delta1) < abs(delta2))
				mo->floorz = *rover->topheight;
			if (*rover->bottomheight < mo->ceilingz && abs(delta1) >= abs(delta2))
				mo->ceilingz = *rover->bottomheight;
		}
	}
	
	if (mo->player)
	{
		// check for smooth step up
		if (mo->z < mo->floorz && mo->type != MT_SPIRIT)
		{
			mo->player->viewheight -= mo->floorz - mo->z;
			
			mo->player->deltaviewheight = ((cv_viewheight.value << FRACBITS) - mo->player->viewheight) >> 3;
		}
		else if (mo->flags2 & MF2_ONMOBJ)
			mo->player->viewheight = mo->height + mo->player->bob;
	}
	// adjust height
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
	
	if (mo->player && mo->flags2& MF2_FLY && !(mo->z <= mo->floorz) && leveltime & 2)
	{
		mo->z += finesine[(FINEANGLES / 20 * leveltime >> 2) & FINEMASK];
	}
	// clip movement
	
	// Spawn splashes, etc.
	// Hit the floor
	
	if (mo->z <= mo->floorz)
	{
		// hit the floor
		if (mo->flags & MF_MISSILE)
		{
			mo->z = mo->floorz;
			if (mo->flags2 & MF2_FLOORBOUNCE)
			{
				P_FloorBounceMissile(mo);
				return;
			}
			else
			{
				if ((mo->flags & MF_NOCLIP) == 0)
				{
					P_ExplodeMissile(mo);
					return;
				}
			}
		}
		// Spawn splashes, etc.
		if (mo->z - mo->momz > mo->floorz)
			P_HitFloor(mo);
		mo->z = mo->floorz;
		
		// Note (id):
		//  somebody left this after the setting momz to 0,
		//  kinda useless there.
		if (mo->z - mo->momz > mo->floorz)
			P_HitFloor(mo);
		mo->z = mo->floorz;
		if (mo->flags & MF_SKULLFLY)
		{
			// the skull slammed into something
			mo->momz = -mo->momz;
		}
		
		if (mo->momz < 0)		// falling
		{
			if (mo->player && (mo->momz < -8 * FRACUNIT) && !(mo->flags2 & MF2_FLY))
			{
				// Squat down.
				// Decrease viewheight for a moment
				// after hitting the ground (hard),
				// and utter appropriate sound.
				mo->player->deltaviewheight = mo->momz >> 3;
				S_StartSound(&mo->NoiseThinker, sfx_oof);
			}
			// set it once and not continuously
			if (mo->z < mo->floorz)
				mo->eflags |= MF_JUSTHITFLOOR;
				
			mo->momz = 0;
		}
		if (mo->info->crashstate && (mo->flags & MF_CORPSE))
		{
			P_SetMobjState(mo, mo->info->crashstate);
			mo->flags &= ~MF_CORPSE;
			return;
		}
	}
	else if (mo->flags2 & MF2_LOGRAV)
	{
		if (mo->momz == 0)
			mo->momz = -(cv_gravity.value >> 3) * 2;
		else
			mo->momz -= cv_gravity.value >> 3;
	}
	else if (!(mo->flags & MF_NOGRAVITY))	// Gravity here!
	{
		fixed_t gravityadd;
		
		//Fab: NOT SURE WHETHER IT IS USEFUL, just put it here too
		//     TO BE SURE there is no problem for the release..
		//     (this is done in P_Mobjthinker below normally)
		mo->eflags &= ~MF_JUSTHITFLOOR;
		
		gravityadd = -cv_gravity.value;
		
		// if waist under water, slow down the fall
		if (mo->eflags & MF_UNDERWATER)
		{
			if (mo->eflags & MF_SWIMMING)
				gravityadd = 0;	// gameplay: no gravity while swimming
			else
				gravityadd >>= 2;
		}
		else if (mo->momz == 0)
			// mobj at stop, no floor, so feel the push of gravity!
			gravityadd <<= 1;
			
		mo->momz += gravityadd;
	}
	
	if (mo->z + mo->height > mo->ceilingz)
	{
		mo->z = mo->ceilingz - mo->height;
		
		//added:22-02-98: player avatar hits his head on the ceiling, ouch!
		if (mo->player && (demoversion >= 112) && !(mo->player->cheats & CF_FLYAROUND) && !(mo->flags2 & MF2_FLY) && mo->momz > 8 * FRACUNIT)
			S_StartSound(&mo->NoiseThinker, sfx_ouch);
			
		// hit the ceiling
		if (mo->momz > 0)
			mo->momz = 0;
			
		if (mo->flags & MF_SKULLFLY)
		{
			// the skull slammed into something
			mo->momz = -mo->momz;
		}
		
		if ((mo->flags & MF_MISSILE) && !(mo->flags & MF_NOCLIP))
		{
			//SoM: 4/3/2000: Don't explode on the sky!
			if (demoversion >= 129 && mo->subsector->sector->ceilingpic == skyflatnum && mo->subsector->sector->ceilingheight == mo->ceilingz)
			{
				P_RemoveMobj(mo);
				return;
			}
			
			P_ExplodeMissile(mo);
			return;
		}
	}
	// z friction in water
	if (demoversion >= 128 && ((mo->eflags & MF_TOUCHWATER) || (mo->eflags & MF_UNDERWATER)) && !(mo->flags & (MF_MISSILE | MF_SKULLFLY)))
	{
		mo->momz = FixedMul(mo->momz, FRICTION * 3 / 4);
	}
	
}

//
// P_NightmareRespawn
//
void P_NightmareRespawn(mobj_t* mobj)
{
	fixed_t x;
	fixed_t y;
	fixed_t z;
	subsector_t* ss;
	mobj_t* mo;
	mapthing_t* mthing;
	
	mthing = mobj->spawnpoint;
	
	if (!mthing)				// Hurdler: respawn FS spawned mobj at their last position (they have no mapthing)
	{
		x = mobj->x;
		y = mobj->y;
	}
	else
	{
		x = mthing->x << FRACBITS;
		y = mthing->y << FRACBITS;
	}
	
	// somthing is occupying it's position?
	if (!P_CheckPosition(mobj, x, y))
		return;					// no respwan
		
	// spawn a teleport fog at old spot
	// because of removal of the body?
	if (mthing->options & MTF_FS_SPAWNED)
		mo = P_SpawnMobj(mobj->x, mobj->y, mobj->z + (0), MT_TFOG);
	else
		mo = P_SpawnMobj(mobj->x, mobj->y, mobj->subsector->sector->floorheight + (0), MT_TFOG);
	// initiate teleport sound
	S_StartSound(&mo->NoiseThinker, sfx_telept);
	
	// spawn a teleport fog at the new spot
	ss = R_PointInSubsector(x, y);
	
	mo = P_SpawnMobj(x, y, ss->sector->floorheight + (0), MT_TFOG);
	
	S_StartSound(&mo->NoiseThinker, sfx_telept);
	
	// spawn it
	if (mobj->info->flags & MF_SPAWNCEILING)
		z = ONCEILINGZ;
	else if (mthing->options & MTF_FS_SPAWNED)
		z = mobj->z;
	else
		z = ONFLOORZ;
		
	// inherit attributes from deceased one
	mo = P_SpawnMobj(x, y, z, mobj->type);
	mo->spawnpoint = mobj->spawnpoint;
	if (!mthing)				// Hurdler: respawn FS spawned mobj at their last position (they have no mapthing)
		mo->angle = mobj->angle;
	else
		mo->angle = ANG45 * (mthing->angle / 45);
		
	if (!mthing)				// Hurdler: respawn FS spawned mobj at their last position (they have no mapthing)
		mo->flags |= mobj->flags & (MTF_AMBUSH ? MF_AMBUSH : 0);
	else if (mthing->options & MTF_AMBUSH)
		mo->flags |= MF_AMBUSH;
		
	mo->reactiontime = 18;
	
	// remove the old monster,
	P_RemoveMobj(mobj);
}

consvar_t cv_spawnmonsters = { "spawnmonsters", "1", CV_NETVAR, CV_YesNo };
consvar_t cv_respawnmonsters = { "respawnmonsters", "0", CV_NETVAR, CV_OnOff };
consvar_t cv_respawnmonsterstime = { "respawnmonsterstime", "12", CV_NETVAR, CV_Unsigned };

//
// P_MobjCheckWater : check for water, set stuff in mobj_t struct for
//                    movement code later, this is called either by P_MobjThinker() or P_PlayerThink()
void P_MobjCheckWater(mobj_t* mobj)
{
	sector_t* sector;
	fixed_t z;
	int oldeflags;
	
	if (demoversion < 128 || mobj->type == MT_SPLASH || mobj->type == MT_SPIRIT)	// splash don't do splash
		return;
	//
	// see if we are in water, and set some flags for later
	//
	sector = mobj->subsector->sector;
	z = sector->floorheight;
	oldeflags = mobj->eflags;
	
	//SoM: 3/28/2000: Only use 280 water type of water. Some boom levels get messed up.
	if ((sector->heightsec > -1 && sector->altheightsec == 1) || (sector->floortype == FLOOR_WATER && sector->heightsec == -1))
	{
		if (sector->heightsec > -1)	//water hack
			z = (sectors[sector->heightsec].floorheight);
		else
			z = sector->floorheight + (FRACUNIT / 4);	// water texture
			
		if (mobj->z <= z && mobj->z + mobj->height > z)
			mobj->eflags |= MF_TOUCHWATER;
		else
			mobj->eflags &= ~MF_TOUCHWATER;
			
		if (mobj->z + (mobj->height >> 1) <= z)
			mobj->eflags |= MF_UNDERWATER;
		else
			mobj->eflags &= ~MF_UNDERWATER;
	}
	else if (sector->ffloors)
	{
		ffloor_t* rover;
		
		mobj->eflags &= ~(MF_UNDERWATER | MF_TOUCHWATER);
		
		for (rover = sector->ffloors; rover; rover = rover->next)
		{
			if (!(rover->flags & FF_SWIMMABLE) || rover->flags & FF_SOLID)
				continue;
			if (*rover->topheight <= mobj->z || *rover->bottomheight > (mobj->z + (mobj->info->height >> 1)))
				continue;
				
			if (mobj->z + mobj->info->height > *rover->topheight)
				mobj->eflags |= MF_TOUCHWATER;
			else
				mobj->eflags &= ~MF_TOUCHWATER;
				
			if (mobj->z + (mobj->info->height >> 1) < *rover->topheight)
				mobj->eflags |= MF_UNDERWATER;
			else
				mobj->eflags &= ~MF_UNDERWATER;
				
			if (!(oldeflags & (MF_TOUCHWATER | MF_UNDERWATER)) && ((mobj->eflags & MF_TOUCHWATER) || (mobj->eflags & MF_UNDERWATER)) && mobj->type != MT_BLOOD)
				P_SpawnSplash(mobj, *rover->topheight);
		}
		return;
	}
	else
		mobj->eflags &= ~(MF_UNDERWATER | MF_TOUCHWATER);
		
	/*
	   if( (mobj->eflags ^ oldeflags) & MF_TOUCHWATER)
	   CONS_Printf("touchewater %d\n",mobj->eflags & MF_TOUCHWATER ? 1 : 0);
	   if( (mobj->eflags ^ oldeflags) & MF_UNDERWATER)
	   CONS_Printf("underwater %d\n",mobj->eflags & MF_UNDERWATER ? 1 : 0);
	 */
	// blood doesnt make noise when it falls in water
	if (!(oldeflags & (MF_TOUCHWATER | MF_UNDERWATER)) &&
	        ((mobj->eflags & MF_TOUCHWATER) || (mobj->eflags & MF_UNDERWATER)) && mobj->type != MT_BLOOD && demoversion < 132)
		P_SpawnSplash(mobj, z);	//SoM: 3/17/2000
}

//===========================================================================
//
// PlayerLandedOnThing
//
//===========================================================================
static void PlayerLandedOnThing(mobj_t* mo, mobj_t* onmobj)
{
	mo->player->deltaviewheight = mo->momz >> 3;
	if (mo->momz < -23 * FRACUNIT)
	{
		//P_FallingDamage(mo->player);
		P_NoiseAlert(mo, mo);
	}
	else if (mo->momz < -8 * FRACUNIT && !mo->player->chickenTics)
	{
		S_StartSound(&mo->NoiseThinker, sfx_oof);
	}
}

//
// P_MobjThinker
//
void P_MobjThinker(mobj_t* mobj)
{
	bool_t checkedpos = false;	//added:22-02-98:
	
	// check mobj against possible water content, before movement code
	P_MobjCheckWater(mobj);
	
	//
	// momentum movement
	//
	if (mobj->momx || mobj->momy || (mobj->flags & MF_SKULLFLY))
	{
		P_XYMovement(mobj);
		checkedpos = true;
		
		// FIXME: decent NOP/NULL/Nil function pointer please.
		if ((mobj->thinker.function.acv == (actionf_v) (-1)))
			return;				// mobj was removed
	}
	if (mobj->flags2 & MF2_FLOATBOB)
	{
		// Floating item bobbing motion
		mobj->z = mobj->floorz + FloatBobOffsets[(mobj->health++) & 63];
	}
	else
		//added:28-02-98: always do the gravity bit now, that's simpler
		//                BUT CheckPosition only if wasn't do before.
		if ((mobj->eflags & MF_ONGROUND) == 0 || (mobj->z != mobj->floorz) || mobj->momz)
		{
			// BP: since version 1.31 we use heretic z-cheching code
			//     kept old code for backward demo compatibility
			if (demoversion < 131)
			{
			
				// if didnt check things Z while XYMovement, do the necessary now
				if (!checkedpos && (demoversion >= 112))
				{
					// FIXME : should check only with things, not lines
					P_CheckPosition(mobj, mobj->x, mobj->y);
					
					mobj->floorz = tmfloorz;
					mobj->ceilingz = tmceilingz;
					if (tmfloorthing)
						mobj->eflags &= ~MF_ONGROUND;	//not on real floor
					else
						mobj->eflags |= MF_ONGROUND;
						
					// now mobj->floorz should be the current sector's z floor
					// or a valid thing's top z
				}
				
				P_ZMovement(mobj);
			}
			else if (mobj->flags2 & MF2_PASSMOBJ)
			{
				mobj_t* onmo;
				
				onmo = P_CheckOnmobj(mobj);
				if (!onmo)
				{
					P_ZMovement(mobj);
					if (mobj->player && mobj->flags & MF2_ONMOBJ)
						mobj->flags2 &= ~MF2_ONMOBJ;
				}
				else
				{
					if (mobj->player)
					{
						if (mobj->momz < -8 * FRACUNIT && !(mobj->flags2 & MF2_FLY))
						{
							PlayerLandedOnThing(mobj, onmo);
						}
						if (onmo->z + onmo->height - mobj->z <= 24 * FRACUNIT)
						{
							mobj->player->viewheight -= onmo->z + onmo->height - mobj->z;
							mobj->player->deltaviewheight = (VIEWHEIGHT - mobj->player->viewheight) >> 3;
							mobj->z = onmo->z + onmo->height;
							mobj->flags2 |= MF2_ONMOBJ;
							mobj->momz = 0;
						}
						else
						{
							// hit the bottom of the blocking mobj
							mobj->momz = 0;
						}
					}
				}
			}
			else
				P_ZMovement(mobj);
				
			// FIXME: decent NOP/NULL/Nil function pointer please.
			if (mobj->thinker.function.acv == (actionf_v) (-1))
				return;				// mobj was removed
		}
		else
			mobj->eflags &= ~MF_JUSTHITFLOOR;
			
	// SoM: Floorhuggers stay on the floor allways...
	// BP: tested here but never set ?!
	if (mobj->info->flags & MF_FLOORHUGGER)
	{
		mobj->z = mobj->floorz;
	}
	// cycle through states,
	// calling action functions at transitions
	if (mobj->tics != -1)
	{
		mobj->tics--;
		
		// you can cycle through multiple states in a tic
		if (!mobj->tics)
			if (!P_SetMobjState(mobj, mobj->state->nextstate))
				return;			// freed itself
	}
	else
	{
		// check for nightmare respawn
		if (!cv_respawnmonsters.value)
			return;
			
		if (!(mobj->flags & MF_COUNTKILL))
			return;
			
		mobj->movecount++;
		
		if (mobj->movecount < cv_respawnmonsterstime.value * TICRATE)
			return;
			
		if (leveltime % (32))
			return;
			
		if (P_Random() > 4)
			return;
			
		P_NightmareRespawn(mobj);
	}
	
}

void P_MobjNullThinker(mobj_t* mobj)
{
}

//
// P_SpawnMobj
//
mobj_t* P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type)
{
	mobj_t* mobj;
	state_t* st;
	mobjinfo_t* info;
	
	mobj = Z_Malloc(sizeof(*mobj), PU_LEVEL, NULL);
	memset(mobj, 0, sizeof(*mobj));
	info = &mobjinfo[type];
	
	mobj->type = type;
	mobj->info = info;
	mobj->x = x;
	mobj->y = y;
	mobj->radius = info->radius;
	mobj->height = info->height;
	mobj->flags = info->flags;
	mobj->flags2 = info->flags2;
	
	mobj->health = info->spawnhealth;
	
	if (gameskill != sk_nightmare)
		mobj->reactiontime = info->reactiontime;
		
	if (demoversion < 129 && mobj->type != MT_CHASECAM)
		mobj->lastlook = P_Random() % MAXPLAYERS;
	else
		mobj->lastlook = -1;	// stuff moved in P_enemy.P_LookForPlayer
		
	// do not set the state with P_SetMobjState,
	// because action routines can not be called yet
	st = &states[info->spawnstate];
	
	mobj->state = st;
	mobj->tics = st->tics;
	mobj->sprite = st->sprite;
	mobj->frame = st->frame;	// FF_FRAMEMASK for frame, and other bits..
	mobj->touching_sectorlist = NULL;	//SoM: 4/7/2000
	mobj->friction = ORIG_FRICTION;	//SoM: 4/7/2000
	
	// BP: SoM right ? if not ajust in p_saveg line 625 and 979
	mobj->movefactor = ORIG_FRICTION_FACTOR;
	
	// set subsector and/or block links
	P_SetThingPosition(mobj);
	
	mobj->floorz = mobj->subsector->sector->floorheight;
	mobj->ceilingz = mobj->subsector->sector->ceilingheight;
	
	//added:27-02-98: if ONFLOORZ, stack the things one on another
	//                so they do not occupy the same 3d space
	//                allow for some funny thing arrangements!
	if (z == ONFLOORZ)
	{
		//if (!P_CheckPosition(mobj,x,y))
		// we could send a message to the console here, saying
		// "no place for spawned thing"...
		
		//added:28-02-98: defaults onground
		mobj->eflags |= MF_ONGROUND;
		
		//added:28-02-98: dirty hack : dont stack monsters coz it blocks
		//                moving floors and anyway whats the use of it?
		/*
		   if (mobj->flags & MF_NOBLOOD)
		   {
		   mobj->z = mobj->floorz;
		
		   // first check the tmfloorz
		   P_CheckPosition(mobj,x,y);
		   mobj->z = tmfloorz+FRACUNIT;
		
		   // second check at the good z pos
		   P_CheckPosition(mobj,x,y);
		
		   mobj->floorz = tmfloorz;
		   mobj->ceilingz = tmsectorceilingz;
		   mobj->z = tmfloorz;
		   // thing not on solid ground
		   if (tmfloorthing)
		   mobj->eflags &= ~MF_ONGROUND;
		
		   //if (mobj->type == MT_BARREL)
		   //   fprintf(stderr,"barrel at z %d floor %d ceiling %d\n",mobj->z,mobj->floorz,mobj->ceilingz);
		
		   }
		   else
		 */
		mobj->z = mobj->floorz;
		
	}
	else if (z == ONCEILINGZ)
		mobj->z = mobj->ceilingz - mobj->info->height;
	else if (z == FLOATRANDZ)
	{
		fixed_t space = ((mobj->ceilingz) - (mobj->height)) - mobj->floorz;
		
		if (space > 48 * FRACUNIT)
		{
			space -= 40 * FRACUNIT;
			mobj->z = ((space * P_Random()) >> 8) + mobj->floorz + 40 * FRACUNIT;
		}
		else
			mobj->z = mobj->floorz;
	}
	else
	{
		//CONS_Printf("mobj spawned at z %d\n",z>>16);
		mobj->z = z;
	}
	
	if (mobj->spawnpoint)
	{
		if ((mobj->spawnpoint->options >> 7) != 0 && !mobj->spawnpoint->z)
		{
			if (z == ONFLOORZ)
				mobj->z = R_PointInSubsector(x, y)->sector->floorheight + ((mobj->spawnpoint->options >> 7) << FRACBITS);
			else if (z == ONCEILINGZ)
				mobj->z = R_PointInSubsector(x, y)->sector->ceilingheight - (((mobj->spawnpoint->options >> 7) << FRACBITS) - mobj->height);
		}
		else if (mobj->spawnpoint->z)
			mobj->z = mobj->spawnpoint->z << FRACBITS;
	}
	// TODO - GhostlyDeath: ReAdd Foot Clipping
	if (mobj->flags2 & MF2_FOOTCLIP && P_GetThingFloorType(mobj) != FLOOR_SOLID && mobj->floorz == mobj->subsector->sector->floorheight)
		mobj->flags2 |= MF2_FEETARECLIPPED;
	else
		mobj->flags2 &= ~MF2_FEETARECLIPPED;
		
	// added 16-6-98: special hack for spirit
	if (mobj->type == MT_SPIRIT)
		mobj->thinker.function.acv = (actionf_p1) P_MobjNullThinker;
	else
	{
		mobj->thinker.function.acp1 = (actionf_p1) P_MobjThinker;
		P_AddThinker(&mobj->thinker);
	}
	
	if (mobj->spawnpoint)
		mobj->spawnpoint->z = mobj->z >> FRACBITS;
		
	return mobj;
}

//
// P_RemoveMobj
//
mapthing_t* itemrespawnque[ITEMQUESIZE];
tic_t itemrespawntime[ITEMQUESIZE];
int iquehead;
int iquetail;

void P_RemoveMobj(mobj_t* mobj)
{
	if (!mobj)
		return;
		
	if ((mobj->flags & MF_SPECIAL) && !(mobj->flags & MF_DROPPED) && (mobj->type != MT_INV) && (mobj->type != MT_INS))
	{
		itemrespawnque[iquehead] = mobj->spawnpoint;
		itemrespawntime[iquehead] = leveltime;
		iquehead = (iquehead + 1) & (ITEMQUESIZE - 1);
		
		// lose one off the end?
		if (iquehead == iquetail)
			iquetail = (iquetail + 1) & (ITEMQUESIZE - 1);
	}
	// unlink from sector and block lists
	P_UnsetThingPosition(mobj);
	
	//SoM: 4/7/2000: Remove touching_sectorlist from mobj.
	if (sector_list)
	{
		P_DelSeclist(sector_list);
		sector_list = NULL;
	}
	// stop any playing sound
	S_StopSound(&mobj->NoiseThinker);
	
	// free block
	P_RemoveThinker((thinker_t*) mobj);
}

consvar_t cv_itemrespawntime = { "respawnitemtime", "30", CV_NETVAR, CV_Unsigned };
consvar_t cv_itemrespawn = { "respawnitem", "0", CV_NETVAR, CV_OnOff };

//
// P_RespawnSpecials
//
void P_RespawnSpecials(void)
{
	fixed_t x;
	fixed_t y;
	fixed_t z;
	
	mobj_t* mo;
	mapthing_t* mthing;
	subsector_t* ss;
	
	int i;
	
	// only respawn items in deathmatch
	if (!cv_itemrespawn.value)
		return;					//
		
	// nothing left to respawn?
	if (iquehead == iquetail)
		return;
		
	// the first item in the queue is the first to respawn
	// wait at least 30 seconds
	if (leveltime - itemrespawntime[iquetail] < (tic_t)cv_itemrespawntime.value * TICRATE)
		return;
		
	mthing = itemrespawnque[iquetail];
	
	if (!mthing)				// Hurdler: grrrr, very ugly hack that need to be fixed!!!
	{
		CONS_Printf("Warning: couldn't respawn a thing. This is a known bug with FS and saved games.\n");
		// pull it from the que
		iquetail = (iquetail + 1) & (ITEMQUESIZE - 1);
		return;
	}
	
	x = mthing->x << FRACBITS;
	y = mthing->y << FRACBITS;
	
	// spawn a teleport fog at the new spot
	ss = R_PointInSubsector(x, y);
	if (mthing->options & MTF_FS_SPAWNED)
		mo = P_SpawnMobj(x, y, mthing->z << FRACBITS, MT_IFOG);
	else
		mo = P_SpawnMobj(x, y, ss->sector->floorheight, MT_IFOG);
	S_StartSound(&mo->NoiseThinker, sfx_itmbk);
	
	// find which type to spawn
	for (i = 0; i < NUMMOBJTYPES; i++)
		if (mthing->type == mobjinfo[i].doomednum)
			break;
			
	// spawn it
	if (mobjinfo[i].flags & MF_SPAWNCEILING)
		z = ONCEILINGZ;
	else if (mthing->options & MTF_FS_SPAWNED)
		z = mthing->z << FRACBITS;	//DarkWolf95:This still wasn't fixed?! Keep Z for FS stuff.
	else
		z = ONFLOORZ;
		
	mo = P_SpawnMobj(x, y, z, i);
	mo->spawnpoint = mthing;
	mo->angle = ANG45 * (mthing->angle / 45);
	
	// pull it from the que
	iquetail = (iquetail + 1) & (ITEMQUESIZE - 1);
}

// used when we are going from deathmatch 2 to deathmatch 1
void P_RespawnWeapons(void)
{
	fixed_t x;
	fixed_t y;
	fixed_t z;
	
	subsector_t* ss;
	mobj_t* mo;
	mapthing_t* mthing;
	
	int i, j, freeslot;
	
	freeslot = iquetail;
	for (j = iquetail; j != iquehead; j = (j + 1) & (ITEMQUESIZE - 1))
	{
		mthing = itemrespawnque[j];
		
		i = 0;
		switch (mthing->type)
		{
			case 2001:			//mobjinfo[MT_SHOTGUN].doomednum  :
				i = MT_SHOTGUN;
				break;
			case 82:			//mobjinfo[MT_SUPERSHOTGUN].doomednum :
				i = MT_SUPERSHOTGUN;
				break;
			case 2002:			//mobjinfo[MT_CHAINGUN].doomednum :
				i = MT_CHAINGUN;
				break;
			case 2006:			//mobjinfo[MT_BFG9000].doomednum   : // bfg9000
				i = MT_BFG9000;
				break;
			case 2004:			//mobjinfo[MT_PLASMAGUN].doomednum   : // plasma launcher
				i = MT_PLASMAGUN;
				break;
			case 2003:			//mobjinfo[MT_ROCKETLAUNCH].doomednum   : // rocket launcher
				i = MT_ROCKETLAUNCH;
				break;
			case 2005:			//mobjinfo[MT_SHAINSAW].doomednum   : // shainsaw
				i = MT_SHAINSAW;
				break;
			default:
				if (freeslot != j)
				{
					itemrespawnque[freeslot] = itemrespawnque[j];
					itemrespawntime[freeslot] = itemrespawntime[j];
				}
				
				freeslot = (freeslot + 1) & (ITEMQUESIZE - 1);
				continue;
		}
		// respwan it
		x = mthing->x << FRACBITS;
		y = mthing->y << FRACBITS;
		
		// spawn a teleport fog at the new spot
		ss = R_PointInSubsector(x, y);
		if (mthing->options & MTF_FS_SPAWNED)
			mo = P_SpawnMobj(x, y, mthing->z << FRACBITS, MT_IFOG);
		else
			mo = P_SpawnMobj(x, y, ss->sector->floorheight, MT_IFOG);
		S_StartSound(&mo->NoiseThinker, sfx_itmbk);
		
		// spawn it
		if (mobjinfo[i].flags & MF_SPAWNCEILING)
			z = ONCEILINGZ;
		else if (mthing->options & MTF_FS_SPAWNED)
			z = mthing->z << FRACBITS;
		else
			z = ONFLOORZ;
			
		mo = P_SpawnMobj(x, y, z, i);
		mo->spawnpoint = mthing;
		mo->angle = ANG45 * (mthing->angle / 45);
		// here don't increment freeslot
	}
	iquehead = freeslot;
}

extern uint8_t weapontobutton[NUMWEAPONS];

//
// P_SpawnPlayer
// Called when a player is spawned on the level.
// Most of the player structure stays unchanged
//  between levels.
//
// BP: spawn it at a playerspawn mapthing
void P_SpawnPlayer(mapthing_t* mthing)
{
	player_t* p;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	int playernum;
	mobj_t* mobj;
	int i;
	
	if (!mthing)
	{
		I_Error("P_SpawnPlayer: mthing is NULL!\n");
		return;
	}
	
	playernum = mthing->type - 1;
	
	// not playing?
	if (!playeringame[playernum])
		return;
		
	// GhostlyDeath <November 3, 2010> -- PARANOIA removal
	if (playernum < 0 && playernum >= MAXPLAYERS)
	{
		CONS_Printf("WARNING - P_SpawnPlayer: playernum not valid %i. (%s:%i).\n", playernum, __FILE__, __LINE__);
		return;
	}
	
	p = &players[playernum];
	
	if (p->playerstate == PST_REBORN)
		G_PlayerReborn(playernum);
		
	x = mthing->x << FRACBITS;
	y = mthing->y << FRACBITS;
	
	z = ONFLOORZ;
	
	mobj = P_SpawnMobj(x, y, z, MT_PLAYER);
	//SoM:
	mthing->mobj = mobj;
	
	// set color translations for player sprites
	// added 6-2-98 : change color : now use skincolor (befor is mthing->type-1
	mobj->flags |= (p->skincolor) << MF_TRANSSHIFT;
	
	//
	// set 'spritedef' override in mobj for player skins.. (see ProjectSprite)
	// (usefulness : when body mobj is detached from player (who respawns),
	//  the dead body mobj retain the skin through the 'spritedef' override).
	mobj->skin = p->skin;
	
	mobj->angle = ANG45 * (mthing->angle / 45);
	for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
		if (playernum == consoleplayer[i])
			localangle[i] = mobj->angle;
			
	mobj->player = p;
	mobj->health = p->health;
	
	p->mo = mobj;
	p->playerstate = PST_LIVE;
	p->refire = 0;
	p->message = NULL;
	p->damagecount = 0;
	p->bonuscount = 0;
	p->chickenTics = 0;
	p->rain1 = NULL;
	p->rain2 = NULL;
	p->extralight = 0;
	p->fixedcolormap = 0;
	p->viewheight = cv_viewheight.value << FRACBITS;
	// added 2-12-98
	p->viewz = p->mo->z + p->viewheight;
	
	p->flamecount = 0;
	p->flyheight = 0;
	
	// setup gun psprite
	P_SetupPsprites(p);
	
	// give all cards in death match mode
	if (cv_deathmatch.value)
		p->cards = it_allkeys;
		
	if (playernum == consoleplayer[0])
	{
		// wake up the status bar
		ST_Start();
		// wake up the heads up text
		HU_Start();
	}
	
	if (camera.chase && displayplayer[0] == playernum)
		P_ResetCamera(p);
}

//
// P_SpawnMapThing
// The fields of the mapthing should
// already be in host uint8_t order.
//
void P_SpawnMapThing(mapthing_t* mthing)
{
	int i, j;
	int bit;
	mobj_t* mobj;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	
	if (!mthing->type)
		return;					//SoM: 4/7/2000: Ignore type-0 things as NOPs
		
	// count deathmatch start positions
	if (mthing->type == 11)
	{
		if (numdmstarts < MAX_DM_STARTS)
		{
			deathmatchstarts[numdmstarts] = mthing;
			mthing->type = 0;
			numdmstarts++;
		}
		return;
	}
	// check for players specially
	// added 9-2-98 type 5 -> 8 player[x] starts for cooperative
	//              support ctfdoom cooperative playerstart
	//SoM: 4/7/2000: Fix crashing bug.
	if ((mthing->type > 0 && mthing->type <= 4) || (mthing->type <= 4028 && mthing->type >= 4001))
	{
		if (mthing->type > 4000)
			mthing->type = (mthing->type - 4001) + 5;
			
		// save spots for respawning in network games
		playerstarts[mthing->type - 1] = mthing;
		
		// old version spawn player now, new version spawn player when level is
		// loaded, or in network event later when player join game
		// TODO: GhostlyDeath -- This has to do with voodoo dolls!
		if (!cv_deathmatch.value)
			P_SpawnPlayer(mthing);
			
		return;
	}
	// check for apropriate skill level
	if (!multiplayer && (mthing->options & 16))
		return;
		
	//SoM: 4/7/2000: Implement "not deathmatch" thing flag
	if (multiplayer && cv_deathmatch.value && (mthing->options & 32))
		return;
		
	//SoM: 4/7/2000: Implement "not cooperative" thing flag
	if (multiplayer && !cv_deathmatch.value && (mthing->options & 64))
		return;
		
	if (gameskill == sk_baby)
		bit = 1;
	else if (gameskill == sk_nightmare)
		bit = 4;
	else
		bit = 1 << (gameskill - 1);
		
	if (!(mthing->options & bit))
		return;
		
	// find which type to spawn (woo hacky and I like it)
	for (i = 0; i < NUMMOBJTYPES; i++)
		if (mthing->type == mobjinfo[i].doomednum)
			break;
			
	if (i == NUMMOBJTYPES)
	{
		CONS_Printf("\2P_SpawnMapThing: Unknown type %i at (%i, %i)\n", mthing->type, mthing->x, mthing->y);
		return;
	}
	// don't spawn keycards and players in deathmatch
	if (cv_deathmatch.value && mobjinfo[i].flags & MF_NOTDMATCH)
		return;
		
	// don't spawn any monsters if -nomonsters
	if (!cv_spawnmonsters.value && (i == MT_SKULL || (mobjinfo[i].flags & MF_COUNTKILL)))
		return;
		
	// spawn it
	x = mthing->x << FRACBITS;
	y = mthing->y << FRACBITS;
	
	if (mobjinfo[i].flags & MF_SPAWNCEILING)
		z = ONCEILINGZ;
	else if (mobjinfo[i].flags2 & MF2_SPAWNFLOAT)
		z = FLOATRANDZ;
	else
		z = ONFLOORZ;
		
	mobj = P_SpawnMobj(x, y, z, i);
	mobj->spawnpoint = mthing;
	
	// Seed random starting index for bobbing motion
	if (mobj->flags2 & MF2_FLOATBOB)
		mobj->health = P_Random();
		
	if (mobj->tics > 0)
		mobj->tics = 1 + (P_Random() % mobj->tics);
	if (mobj->flags & MF_COUNTKILL)
		totalkills++;
	if (mobj->flags & MF_COUNTITEM)
		totalitems++;
		
	mobj->angle = mthing->angle * ANGLE_1;	// SSNTails 06-10-2003
	if (mthing->options & MTF_AMBUSH)
		mobj->flags |= MF_AMBUSH;
		
	mthing->mobj = mobj;
}

//
// GAME SPAWN FUNCTIONS
//

//
// P_SpawnSplash
//
// when player moves in water
// SoM: Passing the Z height saves extra calculations...
void P_SpawnSplash(mobj_t* mo, fixed_t z)
// flatwater : old water FWATER flat texture
{
	mobj_t* th;
	
	//fixed_t     z;
	
	if (demoversion < 125)
		return;
		
	// we are supposed to be in water sector and my current
	// hack uses negative tag as water height
	/*
	   if (flatwater)
	   z = mo->subsector->sector->floorheight + (FRACUNIT/4);
	   else
	   z = sectors[mo->subsector->sector->heightsec].floorheight;
	 *///SoM: 3/17/2000
	
	// need to touch the surface because the splashes only appear at surface
	if (mo->z > z || mo->z + mo->height < z)
		return;
		
	// note pos +1 +1 so it doesn't eat the sound of the player..
	th = P_SpawnMobj(mo->x + 1, mo->y + 1, z, MT_SPLASH);
	
	if( z - mo->subsector->sector->floorheight > 4*FRACUNIT)
		S_StartSound(&th->NoiseThinker, sfx_gloop);
	else
	    S_StartSound(&th->NoiseThinker, sfx_splash);
	th->tics -= P_Random() & 3;
	
	if (th->tics < 1)
		th->tics = 1;
		
	// get rough idea of speed
	/*
	   thrust = (mo->momx + mo->momy) >> FRACBITS+1;
	
	   if (thrust >= 2 && thrust<=3)
	   P_SetMobjState (th,S_SPLASH2);
	   else
	   if (thrust < 2)
	   P_SetMobjState (th,S_SPLASH3);
	 */
}

// --------------------------------------------------------------------------
// P_SpawnSmoke
// --------------------------------------------------------------------------
// when player gets hurt by lava/slime, spawn at feet
void P_SpawnSmoke(fixed_t x, fixed_t y, fixed_t z)
{
	mobj_t* th;
	
	if (demoversion < 125)
		return;
		
	x = x - ((P_Random() & 8) * FRACUNIT) - 4 * FRACUNIT;
	y = y - ((P_Random() & 8) * FRACUNIT) - 4 * FRACUNIT;
	z += (P_Random() & 3) * FRACUNIT;
	
	th = P_SpawnMobj(x, y, z, MT_SMOK);
	th->momz = FRACUNIT;
	th->tics -= P_Random() & 3;
	
	if (th->tics < 1)
		th->tics = 1;
}

// --------------------------------------------------------------------------
// P_SpawnPuff
// --------------------------------------------------------------------------
void P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z)
{
	mobj_t* puff;
	
	z += P_SignedRandom() << 10;
	
	// GhostlyDeath <January 7, 2008> -- So I can use heretic stuff in Doom!
	puff = P_SpawnMobj(x, y, z, PuffType);
	
	switch (PuffType)
	{
		case MT_PUFF:
			puff->momz = FRACUNIT;
			puff->tics -= P_Random() & 3;
			
			if (puff->tics < 1)
				puff->tics = 1;
				
			// don't make punches spark on the wall
			if (attackrange == MELEERANGE)
				P_SetMobjState(puff, S_PUFF3);
			break;
		default:
			break;
	}
}

// --------------------------------------------------------------------------
// P_SpawnBlood
// --------------------------------------------------------------------------

static mobj_t* bloodthing;
static fixed_t bloodspawnpointx, bloodspawnpointy;

#ifdef WALLSPLATS
bool_t PTR_BloodTraverse(intercept_t* in)
{
	line_t* li;
	divline_t divl;
	fixed_t frac;
	
	fixed_t z;
	
	if (in->isaline)
	{
		li = in->d.line;
		
		z = bloodthing->z + (P_SignedRandom() << (FRACBITS - 3));
		if (!(li->flags & ML_TWOSIDED))
			goto hitline;
			
		P_LineOpening(li);
		
		// hit lower texture ?
		if (li->frontsector->floorheight != li->backsector->floorheight)
		{
			if (openbottom > z)
				goto hitline;
		}
		// hit upper texture ?
		if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
		{
			if (opentop < z)
				goto hitline;
		}
		// else don't hit
		return true;
		
hitline:
		P_MakeDivline(li, &divl);
		frac = P_InterceptVector(&divl, &trace);
		R_AddWallSplat(li, P_PointOnLineSide(bloodspawnpointx, bloodspawnpointy, li), "BLUDC0", z, frac, SPLATDRAWMODE_TRANS);
		return false;
	}
	//continue
	return true;
}
#endif

// P_SpawnBloodSplats
// the new SpawnBlood : this one first calls P_SpawnBlood for the usual blood sprites
// then spawns blood splats around on walls
//
void P_SpawnBloodSplats(fixed_t x, fixed_t y, fixed_t z, int damage, fixed_t momx, fixed_t momy)
{
#ifdef WALLSPLATS
//static int  counter =0;
	fixed_t x2, y2;
	angle_t angle, anglesplat;
	int distance;
	angle_t anglemul = 1;
	int numsplats;
	int i;
#endif
	// spawn the usual falling blood sprites at location
	P_SpawnBlood(x, y, z, damage);
	//CONS_Printf ("spawned blood counter %d\n", counter++);
	if (demoversion < 129)
		return;
		
#ifdef WALLSPLATS
	// traverse all linedefs and mobjs from the blockmap containing t1,
	// to the blockmap containing the dest. point.
	// Call the function for each mobj/line on the way,
	// starting with the mobj/linedef at the shortest distance...
	
	if (!momx && !momy)
	{
		// from inside
		angle = 0;
		anglemul = 2;
	}
	else
	{
		// get direction of damage
		x2 = x + momx;
		y2 = y + momy;
		angle = R_PointToAngle2(x, y, x2, y2);
	}
	distance = damage * 6;
	numsplats = damage / 3 + 1;
	// BFG is funy without this check
	if (numsplats > 20)
		numsplats = 20;
		
	//CONS_Printf ("spawning %d bloodsplats at distance of %d\n", numsplats, distance);
	//CONS_Printf ("damage %d\n", damage);
	bloodspawnpointx = x;
	bloodspawnpointy = y;
	//uses 'bloodthing' set by P_SpawnBlood()
	for (i = 0; i < numsplats; i++)
	{
		// find random angle between 0-180deg centered on damage angle
		anglesplat = angle + (((P_Random() - 128) * FINEANGLES / 512 * anglemul) << ANGLETOFINESHIFT);
		x2 = x + distance * finecosine[anglesplat >> ANGLETOFINESHIFT];
		y2 = y + distance * finesine[anglesplat >> ANGLETOFINESHIFT];
		
		P_PathTraverse(x, y, x2, y2, PT_ADDLINES, PTR_BloodTraverse);
	}
#endif
}

// P_SpawnBlood
// spawn a blood sprite with falling z movement, at location
// the duration and first sprite frame depends on the damage level
// the more damage, the longer is the sprite animation
void P_SpawnBlood(fixed_t x, fixed_t y, fixed_t z, int damage)
{
	mobj_t* th;
	
	z += P_SignedRandom() << 10;
	th = P_SpawnMobj(x, y, z, MT_BLOOD);
	if (!DEMOCVAR(classicblood).value)
	{
		th->momx = P_SignedRandom() << 12;	//faB:19jan99
		th->momy = P_SignedRandom() << 12;	//faB:19jan99
	}
	th->momz = FRACUNIT * 2;
	th->tics -= P_Random() & 3;
	
	if (th->tics < 1)
		th->tics = 1;
		
	if (damage <= 12 && damage >= 9)
		P_SetMobjState(th, S_BLOOD2);
	else if (damage < 9)
		P_SetMobjState(th, S_BLOOD3);
		
	bloodthing = th;
}

//---------------------------------------------------------------------------
//
// FUNC P_HitFloor
//
//---------------------------------------------------------------------------

int P_HitFloor(mobj_t* thing)
{
	mobj_t* mo;
	int floortype;
	
	if (thing->floorz != thing->subsector->sector->floorheight)
	{
		// don't splash if landing on the edge above water/lava/etc....
		return (FLOOR_SOLID);
	}
	floortype = P_GetThingFloorType(thing);
	
	if (floortype == FLOOR_WATER)
		P_SpawnSplash(thing, thing->floorz);
		
	// do not down the viewpoint
	return (FLOOR_SOLID);
}

//
// P_CheckMissileSpawn
// Moves the missile forward a bit
//  and possibly explodes it right there.
//
bool_t P_CheckMissileSpawn(mobj_t* th)
{
	th->tics -= P_Random() & 3;
	if (th->tics < 1)
		th->tics = 1;
		
	// move a little forward so an angle can
	// be computed if it immediately explodes
	th->x += (th->momx >> 1);
	th->y += (th->momy >> 1);
	th->z += (th->momz >> 1);
	
	if (!P_TryMove(th, th->x, th->y, false))
	{
		P_ExplodeMissile(th);
		return false;
	}
	return true;
}

//
// P_SpawnMissile
//
mobj_t* P_SpawnMissile(mobj_t* source, mobj_t* dest, mobjtype_t type)
{
	mobj_t* th;
	angle_t an;
	int dist;
	fixed_t z;
	
	// GhostlyDeath <November 3, 2010> -- Paranoia removal
	if (!source || !dest)
	{
		CONS_Printf("WARNING - P_SpawnMissile: source %p, dest %p (%s:%i).\n", source, dest, __FILE__, __LINE__);
		return;
	}
	
	switch (type)
	{
		default:
			z = source->z + 32 * FRACUNIT;
			break;
	}
	if (source->flags2 & MF2_FEETARECLIPPED)
		z -= FOOTCLIPSIZE;
		
	th = P_SpawnMobj(source->x, source->y, z, type);
	
	if (th->info->seesound)
		S_StartSound(&th->NoiseThinker, th->info->seesound);
		
	th->target = source;		// where it came from
	
	if (DEMOCVAR(predictingmonsters).value)	//added by AC for predmonsters
	{
		bool_t canHit;
		fixed_t px, py, pz;
		int time, t;
		subsector_t* sec;
		
		dist = P_AproxDistance(dest->x - source->x, dest->y - source->y);
		time = dist / th->info->speed;
		time = P_AproxDistance(dest->x + dest->momx * time - source->x, dest->y + dest->momy * time - source->y) / th->info->speed;
		
		canHit = false;
		t = time + 4;
		do
		{
			t -= 4;
			if (t < 1)
				t = 1;
			px = dest->x + dest->momx * t;
			py = dest->y + dest->momy * t;
			pz = dest->z + dest->momz * t;
			canHit = P_CheckSight2(source, dest, px, py, pz);
		}
		while (!canHit && (t > 1));
		pz = dest->z + dest->momz * time;
		
		sec = R_PointInSubsector(px, py);
		if (!sec)
			sec = dest->subsector;
			
		if (pz < sec->sector->floorheight)
			pz = sec->sector->floorheight;
		else if (pz > sec->sector->ceilingheight)
			pz = sec->sector->ceilingheight - dest->height;
			
		an = R_PointToAngle2(source->x, source->y, px, py);
		
		// fuzzy player
		if (dest->flags & MF_SHADOW)
			an += P_SignedRandom() << 20;
			
		th->angle = an;
		an >>= ANGLETOFINESHIFT;
		th->momx = FixedMul(th->info->speed, finecosine[an]);
		th->momy = FixedMul(th->info->speed, finesine[an]);
		
		if (t < 1)
			t = 1;
			
		th->momz = (pz - source->z) / t;
	}
	else
	{
		an = R_PointToAngle2(source->x, source->y, dest->x, dest->y);
		
		// fuzzy player
		if (dest->flags & MF_SHADOW)
		{
			an += P_SignedRandom() << 20;
		}
		
		th->angle = an;
		an >>= ANGLETOFINESHIFT;
		th->momx = FixedMul(th->info->speed, finecosine[an]);
		th->momy = FixedMul(th->info->speed, finesine[an]);
		
		dist = P_AproxDistance(dest->x - source->x, dest->y - source->y);
		dist = dist / th->info->speed;
		
		if (dist < 1)
			dist = 1;
			
		th->momz = (dest->z - source->z) / dist;
	}
	
	dist = P_CheckMissileSpawn(th);
	if (demoversion < 131)
		return th;
	else
		return dist ? th : NULL;
}

//
// P_SpawnPlayerMissile
// Tries to aim at a nearby monster
//
mobj_t* P_SPMAngle(mobj_t* source, mobjtype_t type, angle_t angle)
{
	mobj_t* th;
	angle_t an;
	
	fixed_t x;
	fixed_t y;
	fixed_t z;
	fixed_t slope = 0;
	
	// angle at which you fire, is player angle
	an = angle;
	
	//added:16-02-98: autoaim is now a toggle
	if ((source->player->autoaim_toggle && DEMOCVAR(allowautoaim).value) || DEMOCVAR(forceautoaim).value)
	{
		// see which target is to be aimed at
		slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT);
		
		if (!linetarget)
		{
			an += 1 << 26;
			slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT);
			
			if (!linetarget)
			{
				an -= 2 << 26;
				slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT);
			}
			
			if (!linetarget)
			{
				an = angle;
				slope = 0;
			}
		}
	}
	//added:18-02-98: if not autoaim, or if the autoaim didnt aim something,
	//                use the mouseaiming
	if (((source->player->autoaim_toggle && DEMOCVAR(allowautoaim).value) || DEMOCVAR(forceautoaim).value) && !linetarget)
	{
		if (demoversion >= 128)
			slope = AIMINGTOSLOPE(source->player->aiming);
		else
			slope = (source->player->aiming << FRACBITS) / 160;
	}
	
	x = source->x;
	y = source->y;
	z = source->z + 4 * 8 * FRACUNIT;
	if (source->flags2 & MF2_FEETARECLIPPED)
		z -= FOOTCLIPSIZE;
		
	th = P_SpawnMobj(x, y, z, type);
	
	if (th->info->seesound)
		S_StartSound(&th->NoiseThinker, th->info->seesound);
		
	th->target = source;
	
	th->angle = an;
	th->momx = FixedMul(th->info->speed, finecosine[an >> ANGLETOFINESHIFT]);
	th->momy = FixedMul(th->info->speed, finesine[an >> ANGLETOFINESHIFT]);
	
	if (demoversion >= 128)
	{
		// 1.28 fix, allow full aiming must be much precise
		th->momx = FixedMul(th->momx, finecosine[source->player->aiming >> ANGLETOFINESHIFT]);
		th->momy = FixedMul(th->momy, finecosine[source->player->aiming >> ANGLETOFINESHIFT]);
	}
	th->momz = FixedMul(th->info->speed, slope);
	
	slope = P_CheckMissileSpawn(th);
	
	if (demoversion < 131)
		return th;
	else
		return slope ? th : NULL;
}

//---------------------------------------------------------------------------
//
// PROC A_ContMobjSound
//
//---------------------------------------------------------------------------

void A_ContMobjSound(mobj_t* actor)
{
	switch (actor->type)
	{
		default:
			break;
	}
}

//----------------------------------------------------------------------------
//
// FUNC P_FaceMobj
//
// Returns 1 if 'source' needs to turn clockwise, or 0 if 'source' needs
// to turn counter clockwise.  'delta' is set to the amount 'source'
// needs to turn.
//
//----------------------------------------------------------------------------
int P_FaceMobj(mobj_t* source, mobj_t* target, angle_t* delta)
{
	angle_t diff;
	angle_t angle1;
	angle_t angle2;
	
	angle1 = source->angle;
	angle2 = R_PointToAngle2(source->x, source->y, target->x, target->y);
	if (angle2 > angle1)
	{
		diff = angle2 - angle1;
		if (diff > ANGLE_180)
		{
			*delta = ANGLE_MAX - diff;
			return (0);
		}
		else
		{
			*delta = diff;
			return (1);
		}
	}
	else
	{
		diff = angle1 - angle2;
		if (diff > ANGLE_180)
		{
			*delta = ANGLE_MAX - diff;
			return (1);
		}
		else
		{
			*delta = diff;
			return (0);
		}
	}
}

//----------------------------------------------------------------------------
//
// FUNC P_SeekerMissile
//
// The missile tracer field must be mobj_t *target.  Returns true if
// target was tracked, false if not.
//
//----------------------------------------------------------------------------

bool_t P_SeekerMissile(mobj_t* actor, angle_t thresh, angle_t turnMax)
{
	int dir;
	int dist;
	angle_t delta;
	angle_t angle;
	mobj_t* target;
	
	target = actor->tracer;
	if (target == NULL)
	{
		return (false);
	}
	if (!(target->flags & MF_SHOOTABLE))
	{
		// Target died
		actor->tracer = 0;
		return (false);
	}
	dir = P_FaceMobj(actor, target, &delta);
	if (delta > thresh)
	{
		delta >>= 1;
		if (delta > turnMax)
		{
			delta = turnMax;
		}
	}
	if (dir)
	{
		// Turn clockwise
		actor->angle += delta;
	}
	else
	{
		// Turn counter clockwise
		actor->angle -= delta;
	}
	angle = actor->angle >> ANGLETOFINESHIFT;
	actor->momx = FixedMul(actor->info->speed, finecosine[angle]);
	actor->momy = FixedMul(actor->info->speed, finesine[angle]);
	if (actor->z + actor->height < target->z || target->z + target->height < actor->z)
	{
		// Need to seek vertically
		dist = P_AproxDistance(target->x - actor->x, target->y - actor->y);
		dist = dist / actor->info->speed;
		if (dist < 1)
			dist = 1;
		actor->momz = (target->z + (target->height >> 1) - (actor->z + (actor->height >> 1))) / dist;
	}
	return (true);
}

//---------------------------------------------------------------------------
//
// FUNC P_SpawnMissileAngle
//
// Returns NULL if the missile exploded immediately, otherwise returns
// a mobj_t pointer to the missile.
//
//---------------------------------------------------------------------------

mobj_t* P_SpawnMissileAngle(mobj_t* source, mobjtype_t type, angle_t angle, fixed_t momz)
{
	fixed_t z;
	mobj_t* mo;
	
	switch (type)
	{
		default:
			z = source->z + 32 * FRACUNIT;
			break;
	}
	if (source->flags2 & MF2_FEETARECLIPPED)
	{
		z -= FOOTCLIPSIZE;
	}
	mo = P_SpawnMobj(source->x, source->y, z, type);
	if (mo->info->seesound)
	{
		S_StartSound(&mo->NoiseThinker, mo->info->seesound);
	}
	mo->target = source;		// Originator
	mo->angle = angle;
	angle >>= ANGLETOFINESHIFT;
	mo->momx = FixedMul(mo->info->speed, finecosine[angle]);
	mo->momy = FixedMul(mo->info->speed, finesine[angle]);
	mo->momz = momz;
	return (P_CheckMissileSpawn(mo) ? mo : NULL);
}

bool_t inventory = false;

/******************************************************************************/

/******************************************************************************/

/******************************************************************************/

/************** LEGACY FLAG COMPATIBILITY **************/
// This will convert legacy flags into extended flags
// When switching to extended flags, functions may call these after they are
// entered and before they return. This allows functions to be converted one at
// a time. Unsupported flags when converting will not be lost, the same goes
// both ways. The Legacy to Extended function sets MFXD_LEGACYCOMPATIBILITY
// while the Extended to Legacy unsets this.
// Legacy flags are given the term natural since they share Doom and Heretic's
// flags for the most part.
// Not all flags are converted

// TODO: Remove array when all the code relysMFXD_LEGACYCOMPATIBILITY on extended flags (+RMOD) and not normal flags
typedef struct XFlagConv_s
{
	uint32_t Flag;
	uint32_t Flag2;
	uint32_t FlagE;
	uint32_t XFlagA;
	uint32_t XFlagB;
	uint32_t XFlagC;
	uint32_t XFlagD;
} XFlagConv_t;

XFlagConv_t XFlagConvList[] =
{
	{0, MF2_FEETARECLIPPED, 0, MFXA_AREFEETCLIPPED, 0, 0, 0},
	{0, 0, 0, MFXA_AUDIBLEPICKUPSOUND, 0, 0, 0},
	{0, MF2_BOUNCES, 0, MFXA_CANBOUNCE, 0, 0, 0},
	{MF_DROPOFF, 0, 0, MFXA_CANDROPOFF, 0, 0, 0},
	{0, MF2_FOOTCLIP, 0, MFXA_CANFEETCLIP, 0, 0, 0},
	{0, MF2_FLOORBOUNCE, 0, MFXA_CANFLOORBOUNCE, 0, 0, 0},
	{0, 0, 0, MFXA_CANGATHER, 0, 0, 0},
	{0, 0, 0, MFXA_CANJUMP, 0, 0, 0},
	{0, 0, 0, MFXA_CANMINOTAURSLAM, 0, 0, 0},
	{0, 0, 0, MFXA_CANOPENDOORS, 0, 0, 0},
	{0, 0, 0, MFXA_CANSLIDE, 0, 0, 0},
	{0, 0, 0, MFXA_CANSWIM, 0, 0, 0},
	{0, MF2_TELESTOMP, 0, MFXA_CANTELEPORTSTOMP, 0, 0, 0},
	{0, 0, 0, MFXA_CANWALLBOUNCE, 0, 0, 0},
	{0, 0, 0, MFXA_CARRYKILLER, 0, 0, 0},
	{0, MF2_PASSMOBJ, 0, MFXA_ENABLEZCHECK, 0, 0, 0},
	{0, MF2_FLOATBOB, 0, MFXA_FLOATBOB, 0, 0, 0},
	{0, 0, 0, MFXA_FORCEDSPARILTELEPORT, 0, 0, 0},
	{0, MF2_BOSS, 0, MFXA_ISBOSS, 0, 0, 0},
	{0, 0, 0, MFXA_ISBOSSCUBESPAWNABLE, 0, 0, 0},
	{MF_CORPSE, 0, 0, MFXA_ISCORPSE, 0, 0, 0},
	{MF_AMBUSH, 0, 0, MFXA_ISDEAF, 0, 0, 0},
	{0, 0, 0, MFXA_ISDOOMITEMFOG, 0, 0, 0},
	{0, 0, 0, MFXA_ISDOOMPLAYER, 0, 0, 0},
	{0, 0, 0, MFXA_ISDOOMTELEPORTFOG, 0, 0, 0},
	{MF_DROPPED, 0, 0, MFXA_ISDROPPED, 0, 0, 0},
	{0, 0, 0, MFXA_ISDSPARIL, 0, 0, 0},
	{0, 0, 0, MFXA_ISEXPLOSIONIMMUNE, 0, 0, 0},
	{MF_FLOAT, 0, 0, MFXA_ISFLOATABLE, 0, 0, 0},
	{MF_FLOORHUGGER, 0, 0, MFXA_ISFLOORHUGGER, 0, 0, 0},
	{0, MF2_FLY, 0, MFXA_ISFLYING, 0, 0, 0},
	{MF_SKULLFLY, 0, 0, MFXA_ISFLYINGSKULL, 0, 0, 0},
	{0, MF2_FRIENDLY, 0, 0, MFXB_ISFRIENDLY, 0, 0},
	{MF_SPECIAL, 0, 0, 0, MFXB_ISGATHERABLE, 0, 0},
	{0, 0, 0, 0, MFXB_ISHERETICITEMFOG, 0, 0},
	{0, 0, 0, 0, MFXB_ISHERETICPLAYER, 0, 0},
	{0, 0, 0, 0, MFXB_ISHERETICTELEPORTFOG, 0, 0},
	{0, 0, 0, 0, MFXB_ISINSTANTKILLIMMUNE, 0, 0},
	{MF_COUNTITEM, 0, 0, 0, MFXB_ISITEMCOUNTABLE, 0, 0},
	{MF_COUNTKILL, 0, 0, 0, MFXB_ISKILLCOUNTABLE, 0, 0},
	{0, MF2_LOGRAV, 0, 0, MFXB_ISLOWGRAVITY, 0, 0},
	{0, 0, 0, 0, MFXB_ISMISSILE, 0, 0},
	{0, 0, 0, 0, MFXB_ISMISSILEINSTANTKILL, 0, 0},
	{0, 0, 0, 0, MFXB_ISMONSTER, 0, 0},
	{0, 0, MF_ONGROUND, 0, MFXB_ISONGROUND, 0, 0},
	{0, MF2_ONMOBJ, 0, 0, MFXB_ISONMOBJ, 0, 0},
	{0, MF2_PUSHABLE, 0, 0, MFXB_ISPUSHABLE, 0, 0},
	{MF_SHADOW, 0, 0, 0, MFXB_ISSHADOW, 0, 0},
	{MF_SHOOTABLE, 0, 0, 0, MFXB_ISSHOOTABLE, 0, 0},
	{MF_SOLID, 0, 0, 0, MFXB_ISSOLID, 0, 0},
	{0, 0, MF_SWIMMING, 0, MFXB_ISSWIMMING, 0, 0},
	{0, MF2_FORCETRANSPARENCY, 0, 0, MFXB_ISTRANSPARENT, 0, 0},
	{0, 0, MF_TOUCHWATER, 0, MFXB_ISTOUCHINGWATER, 0, 0},
	{0, 0, MF_UNDERWATER, 0, MFXB_ISUNDERWATER, 0, 0},
	{0, 0, 0, 0, MFXB_ISUNIQUE, 0, 0},
	{0, MF2_WINDTHRUST, 0, 0, MFXB_ISWINDPUSHABLE, 0, 0},
	{MF_JUSTATTACKED, 0, 0, 0, MFXB_JUSTATTACKED, 0, 0},
	{MF_JUSTHIT, 0, 0, 0, MFXB_JUSTHIT, 0, 0},
	{0, 0, MF_JUSTHITFLOOR, 0, MFXB_JUSTHITFLOOR, 0, 0},
	{MF_SLIDE, 0, 0, 0, MFXB_KEEPSLIDE, 0, 0},
	{0, 0, 0, 0, MFXB_NOALTDEATHMATCH, 0, 0},
	{MF_INFLOAT, 0, 0, 0, MFXB_NOAUTOFLOAT, 0, 0},
	{MF_NOBLOCKMAP, 0, 0, 0, MFXB_NOBLOCKMAP, 0, 0},
	{MF_NOBLOOD, 0, 0, 0, MFXB_NOBLOOD, 0, 0},
	{0, 0, 0, 0, 0, MFXC_NOCHICKENMORPH, 0},
	{MF_NOCLIP, 0, 0, 0, 0, MFXC_NOCLIP, 0},
	{0, 0, 0, 0, 0, MFXC_NOCOOP, 0},
	{0, 0, 0, 0, 0, MFXC_NOCTF, 0},
	{0, MF2_NODMGTHRUST, 0, 0, 0, MFXC_NODAMAGETHRUST, 0},
	{MF_NOTDMATCH, 0, 0, 0, 0, MFXC_NODEATHMATCH, 0},
	{0, MF2_DONTDRAW, 0, 0, 0, MFXC_NODRAW, 0},
	{MF_NOGRAVITY, 0, 0, 0, 0, MFXC_NOGRAVITY, 0},
	{0, MF2_THRUGHOST, 0, 0, 0, MFXC_NOHITGHOST, 0},
	{0, MF2_RIP, 0, 0, 0, MFXC_NOHITSOLID, 0},
	{0, 0, 0, 0, 0, MFXC_NOMISSILEHURTSAMETYPE, 0},
	{0, 0, 0, 0, 0, MFXC_NOLINEACTIVATE, 0},
	{0, 0, 0, 0, 0, MFXC_NOLINECLIPPING, 0},
	{0, 0, 0, 0, 0, MFXC_NOMOVEOVERSAMETYPE, 0},
	{0, 0, 0, 0, 0, MFXC_NONEWDEATHMATCH, 0},
	{0, MF2_CANNOTPUSH, 0, 0, 0, MFXC_NOPUSHING, 0},
	{0, 0, 0, 0, 0, MFXC_NOSINGLEPLAYER, 0},
	{MF_NOSECTOR, 0, 0, 0, 0, MFXC_NOSECTORLINKS, 0},
	{0, 0, 0, 0, 0, MFXC_NOTARGET, 0},
	{0, MF2_NOTELEPORT, 0, 0, 0, MFXC_NOTELEPORT, 0},
	{MF_NOCLIPTHING, 0, 0, 0, 0, MFXC_NOTHINGCLIPPING, 0},
	{0, 0, MF_NOZCHECKING, 0, 0, MFXC_NOZCHECKING, 0},
	{0, 0, 0, 0, 0, MFXC_REDUCEDBOSSDAMAGE, 0},
	{0, 0, 0, 0, 0, MFXC_SLOWSPLAYER, 0},
	{0, MF2_SPAWNFLOAT, 0, 0, 0, MFXC_SPAWNATRANDOMZ, 0},
	{MF_SPAWNCEILING, 0, 0, 0, 0, MFXC_SPAWNONCEILING, 0},
	{0, 0, 0, 0, 0, MFXC_FULLBLASTWAKESOUND, 0},
	{0, 0, 0, 0, 0, MFXC_FULLBLASTDEATHSOUND, 0},
	{0, 0, 0, 0, 0, MFXC_VILEMISSILERANGE, 0},
	{0, 0, 0, 0, 0, MFXC_REVENANTMISSILERANGE, 0},
	{0, 0, 0, 0, 0, MFXC_HALFMISSILERANGE, 0},
	{0, 0, 0, 0, 0, MFXC_CYBERDEMONMISSILERANGE, 0},
	{0, 0, 0, 0, 0, 0, MFXD_ROAMSOUNDISSEESOUND},
	{0, 0, 0, 0, 0, 0, MFXD_FULLBLASTROAMSOUND},
	{0, 0, 0, 0, 0, 0, MFXD_MINOTAUREXPLOSION},
	{0, 0, 0, 0, 0, 0, MFXD_FIREBOMBEXPLOSION},
	{0, 0, 0, 0, 0, 0, MFXD_DSPARILEXPLOSION},
	{0, 0, 0, 0, 0, 0, MFXD_MAPSEVENSIXSIXSIX},
	{0, 0, 0, 0, 0, 0, MFXD_MAPSEVENSIXSIXSEVEN},
	{0, 0, 0, 0, 0, 0, MFXD_COMMERCIALSIXSIXSIX},
	{0, 0, 0, 0, 0, 0, MFXD_EPISODEONESIXSIXSIX},
	{0, 0, 0, 0, 0, 0, MFXD_EPISODETWOEXITLEVEL},
	{0, 0, 0, 0, 0, 0, MFXD_EPISODETHREEEXITLEVEL},
	{0, 0, 0, 0, 0, 0, MFXD_EPISODEFOURASIXSIXSIX},
	{0, 0, 0, 0, 0, 0, MFXD_EPISODEFOURBSIXSIXSIX},
	{0, 0, 0, 0, 0, 0, MFXD_BOSSBRAINTARGET},
	{0, 0, 0, 0, 0, 0, MFXD_HERETICEPISODEONESPECIAL},
	{0, 0, 0, 0, 0, 0, MFXD_HERETICEPISODETWOSPECIAL},
	{0, 0, 0, 0, 0, 0, MFXD_HERETICEPISODETHREESPECIAL},
	{0, 0, 0, 0, 0, 0, MFXD_HERETICEPISODEFOURSPECIAL},
	{0, 0, 0, 0, 0, 0, MFXD_HERETICEPISODEFIVESPECIAL},
	{0, MF2_SLIDE, 0, 0, 0, 0, MFXD_SLIDESALONGWALLS}
};

/* P_MobjFlagsNaturalToExtended() -- Converts natural flags to extended flags */
// Conversions that are not supported or unhandled are ignored
int P_MobjFlagsNaturalToExtended(mobj_t* MObj)
{
	size_t ConvCount = 0;
	size_t i;
	size_t n = sizeof(XFlagConvList) / sizeof(XFlagConv_t);
	
	if (!MObj)
		return 0;
		
#define _REMOOD_NFTOXFSET \
{\
	if (XFlagConvList[i].XFlagA)\
		MObj->XFlagsA |= XFlagConvList[i].XFlagA;\
	if (XFlagConvList[i].XFlagB)\
		MObj->XFlagsB |= XFlagConvList[i].XFlagB;\
	if (XFlagConvList[i].XFlagC)\
		MObj->XFlagsC |= XFlagConvList[i].XFlagC;\
	if (XFlagConvList[i].XFlagD)\
		MObj->XFlagsD |= XFlagConvList[i].XFlagD;\
	ConvCount++;\
}
		
#define _REMOOD_NFTOXFUNSET \
{\
	if (XFlagConvList[i].XFlagA)\
		MObj->XFlagsA &= ~(XFlagConvList[i].XFlagA);\
	if (XFlagConvList[i].XFlagB)\
		MObj->XFlagsB &= ~(XFlagConvList[i].XFlagB);\
	if (XFlagConvList[i].XFlagC)\
		MObj->XFlagsC &= ~(XFlagConvList[i].XFlagC);\
	if (XFlagConvList[i].XFlagD)\
		MObj->XFlagsD &= ~(XFlagConvList[i].XFlagD);\
	ConvCount++;\
}
		
#define REMOOD_NFTOXFGROUP(nf,mf) \
if (XFlagConvList[i].nf && (MObj->mf & XFlagConvList[i].nf))\
	_REMOOD_NFTOXFSET \
else if (XFlagConvList[i].nf)\
	_REMOOD_NFTOXFUNSET
		
	for (i = 0; i < n; i++)
	{
		REMOOD_NFTOXFGROUP(Flag, flags) REMOOD_NFTOXFGROUP(Flag2, flags2) REMOOD_NFTOXFGROUP(FlagE, eflags)
	}
	
	// Set "In Compatibility Mode"
	MObj->XFlagsD |= MFXD_LEGACYCOMPATIBILITY;
	
	return ConvCount;
}

/* P_MobjFlagsExtendedToNatural() -- Converts extended flags to natural flags */
// Conversions that are not supported or unhandled are ignored
int P_MobjFlagsExtendedToNatural(mobj_t* MObj)
{
	size_t ConvCount = 0;
	size_t i;
	size_t n = sizeof(XFlagConvList) / sizeof(XFlagConv_t);
	
	if (!MObj)
		return 0;
		
#define _REMOOD_XFTONFSET \
{\
	if (XFlagConvList[i].Flag)\
		MObj->flags |= XFlagConvList[i].Flag;\
	if (XFlagConvList[i].Flag2)\
		MObj->flags2 |= XFlagConvList[i].Flag2;\
	if (XFlagConvList[i].FlagE)\
		MObj->eflags |= XFlagConvList[i].FlagE;\
	ConvCount++;\
}
		
#define _REMOOD_XFTONFUNSET \
{\
	if (XFlagConvList[i].Flag)\
		MObj->flags &= ~(XFlagConvList[i].Flag);\
	if (XFlagConvList[i].Flag2)\
		MObj->flags2 &= ~(XFlagConvList[i].Flag2);\
	if (XFlagConvList[i].FlagE)\
		MObj->eflags &= ~(XFlagConvList[i].FlagE);\
	ConvCount++;\
}
		
#define REMOOD_XFTONFGROUP(xf,mf) \
if (XFlagConvList[i].xf && (MObj->mf & XFlagConvList[i].xf))\
	_REMOOD_XFTONFSET \
else if (XFlagConvList[i].xf)\
	_REMOOD_XFTONFUNSET
		
	for (i = 0; i < n; i++)
	{
		REMOOD_XFTONFGROUP(XFlagA, XFlagsA) REMOOD_XFTONFGROUP(XFlagB, XFlagsB) REMOOD_XFTONFGROUP(XFlagC, XFlagsC) REMOOD_XFTONFGROUP(XFlagD, XFlagsD)
	}
	
	// Unset "In Compatibility Mode"
	MObj->XFlagsD &= ~MFXD_LEGACYCOMPATIBILITY;
	
	return ConvCount;
}
