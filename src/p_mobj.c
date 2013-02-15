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
#include "m_cheat.h"
#include "b_bot.h"

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

/* PS_AdjMobjStateTics() -- Adjust object state tics */
static void PS_AdjMobjStateTics(mobj_t* const a_Object)
{
	int32_t BloodTime;
	
	/* Check */
	if (!a_Object)
		return;
	
	/* Special value? */
	if (a_Object->state->ExtraStateFlags & __REMOOD_BLOODTIMECONST)
		if (P_XGSVal(PGS_COENABLEBLOODTIME))
		{
			BloodTime = P_XGSVal(PGS_GAMEBLOODTIME);
			
			if (BloodTime <= 0)
				a_Object->tics = 0;
			else
				a_Object->tics = (BloodTime * TICRATE) - 16;
		}
}

//
// P_SetMobjState
// Returns true if the mobj is still present.
//
//SoM: 4/7/2000: Boom code...
bool_t P_SetMobjState(mobj_t* mobj, PI_stateid_t state)
{
	PI_state_t* st;
	
	//remember states seen, to detect cycles:
	
	static PI_stateid_t* seenPI_state_tab;	// fast transition table
	static size_t OldNumStates;
	
	PI_stateid_t* seenstate = seenPI_state_tab;	// pointer to table
	static int recursion;		// detects recursion
	PI_stateid_t i = state;		// initial state
	bool_t ret = true;			// return value
	PI_stateid_t* tempstate;	// for use with recursion

	// GhostlyDeath <May 21, 2012> -- Allocate locally for VC6
	tempstate = Z_Malloc(sizeof(*tempstate) * NUMSTATES, PU_STATIC, NULL);
	
	// GhostlyDeath <April 23, 2012> -- Seen cycles
	if (!seenPI_state_tab || OldNumStates != NUMSTATES)
	{
		if (seenPI_state_tab)
			Z_Free(seenPI_state_tab);
		seenPI_state_tab = Z_Malloc(sizeof(*seenPI_state_tab) * NUMSTATES, PU_STATIC, NULL);
		OldNumStates = NUMSTATES;
	}
	
	seenstate = seenPI_state_tab;
	
	if (recursion++)			// if recursion detected,
		memset(seenstate = tempstate, 0, sizeof(*tempstate) * NUMSTATES);	// clear state table
		
	do
	{
		if (state == S_NULL)
		{
			mobj->state = (PI_state_t*) S_NULL;
			P_RemoveMobj(mobj);
			ret = false;
			break;				// killough 4/9/98
		}
		
		st = states[state];
		mobj->state = st;
		
		// GhostlyDeath <March 5, 2012> -- Remove hack in p_enemy -fast onchange
		if (st->RMODFastTics && P_XGSVal(PGS_MONFASTMONSTERS))
			mobj->tics = st->RMODFastTics;
		else
			mobj->tics = st->tics;
		
		// GhostlyDeath <March 6, 2012> -- Special tic constants?
		PS_AdjMobjStateTics(mobj);
		
		mobj->sprite = st->sprite;
		mobj->frame = st->frame;
		
		// Modified handling.
		// Call action functions when the state is set
		
		if (st->action.acp5)
			st->action.acp5(mobj, mobj->player, NULL, st->ArgC, st->ArgV);
			
		seenstate[state] = 1 + st->nextstate;	// killough 4/9/98
		
		state = st->nextstate;
	}
	while (!mobj->tics && !seenstate[state]);	// killough 4/9/98
	
	if (ret && !mobj->tics)		// killough 4/9/98: detect state cycles
		CONL_PrintF("Warning: State Cycle Detected");
		
	if (!--recursion)
		for (; (state = seenstate[i]); i = state - 1)
			seenstate[i] = 0;	// killough 4/9/98: erase memory of states
	
	// Free local storage
	Z_Free(tempstate);

	return ret;
}

//
// P_ExplodeMissile
//
void P_ExplodeMissile(mobj_t* mo)
{
	mo->momx = mo->momy = mo->momz = 0;
	
	P_SetMobjState(mo, mobjinfo[mo->type]->deathstate);
	
	mo->tics -= P_Random() & 3;
	
	if (mo->tics < 1)
		mo->tics = 1;
		
	mo->flags &= ~MF_MISSILE;
	
	if (mo->info->RDeathSound)
		S_StartSound(&mo->NoiseThinker, S_SoundIDForName(mo->info->RDeathSound));
}

//----------------------------------------------------------------------------
//
// PROC P_FloorBounceMissile
//
//----------------------------------------------------------------------------

void P_FloorBounceMissile(mobj_t* mo)
{
	mo->momz = -mo->momz;
	P_SetMobjState(mo, mobjinfo[mo->type]->deathstate);
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
	player_t* player;
	
	player = NULL;
	if (P_MobjIsPlayer(mo))
		player = mo->player;
	
	if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED && mo->momy > -STOPSPEED
	        && mo->momy < STOPSPEED && (!player || (player->cmd.Std.forwardmove == 0 && player->cmd.Std.sidemove == 0)))
	{
		// if in a walking frame, stop moving
		if (player && (mo->RXFlags[1] & MFREXB_USEPLAYERMOVEMENT) && !(mo->RXFlags[0] & MFREXA_NOPLAYERWALK))
			if (player->mo->state->IOSG == IOSG_PLAYERRUN && player->mo->state->FrameID < 4)
				P_SetMobjState(player->mo, player->mo->info->spawnstate);
		
		mo->momx = 0;
		mo->momy = 0;
	}
	else
	{
		// Flying Friction
		if (mo->flags2 & MF2_FLY && !(mo->z <= mo->floorz) && !(mo->flags2 & MF2_ONMOBJ))
		{
                mo->momx = FixedMul(mo->momx, FRICTION_FLY);
                mo->momy = FixedMul(mo->momy, FRICTION_FLY);
		}
		
		// Heretic Friction
		else if (P_XGSVal(PGS_COHERETICFRICTION))
		{
            if (mo->subsector->sector->special & REXS_HFRICTMASK)      // Friction_Low
            {
                mo->momx = FixedMul(mo->momx, FRICTION_LOW);
                mo->momy = FixedMul(mo->momy, FRICTION_LOW);
            }
            else
            {
                mo->momx = FixedMul(mo->momx, FRICTION);
                mo->momy = FixedMul(mo->momy, FRICTION);
            }
		}
		
		// Old Friction
		else if (oldfriction)
		{
			mo->momx = FixedMul(mo->momx, FRICTION);
			mo->momy = FixedMul(mo->momy, FRICTION);
		}
		
		// Boom Friction
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
	uint32_t Special;
	
	//when up against walls
	static const int windTab[8] = {
		2048*5,
		2048*10,
		2048*15,
		2048*20,
		2048*25,
		2048*30,
		2048*35,
		2048*70
	};
	
	
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
		Special = mo->subsector->sector->special;
		
		// Windy Thrust
		if (Special & REXS_WINDMASK)
			P_ThrustMobj(mo, ANG45 * ((Special & REXS_DIRMASK) >> REXS_DIRSHIFT), windTab[(Special & REXS_SPEEDMASK) >> REXS_SPEEDSHIFT]);
	}
	
	player = NULL;
	if (P_MobjIsPlayer(mo))
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
		
		if (!P_TryMove(mo, ptryx, ptryy, true, &ptryx, &ptryy))	//SoM: 4/10/2000
		{
			// blocked move
			
			// gameplay issue : let the marine move forward while trying
			//                  to jump over a small wall
			//    (normally it can not 'walk' while in air)
			// BP:1.28 no more use Cf_JUMPOVER, but i leave it for backward lmps compatibility
			if (P_MobjIsPlayer(mo))
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
					if (!P_XGSVal(PGS_COBOOMSUPPORT) || mo->z > ceilingline->backsector->ceilingheight)	//SoM: 4/7/2000: DEMO'S
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
				if (blockingline && P_XGSVal(PGS_COENABLEBLOODSPLATS))	//set by last P_TryMove() that failed
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
					R_AddWallSplat(
							blockingline,
							P_PointOnLineSide(mo->x, mo->y, blockingline),
							(mo->info->RMissileSplat ? mo->info->RMissileSplat : "A_DMG3"),
							mo->z, frac, SPLATDRAWMODE_SHADE
						);
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
			if (P_MobjIsPlayer(mo))
				mo->player->cheats &= ~CF_JUMPOVER;
				
	}
	while (xmove || ymove);
	
	// slow down
	if (player && (mo->RXFlags[1] & MFREXB_USEPLAYERMOVEMENT))
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
	if (P_XGSVal(PGS_COSLOWINWATER) && (mo->eflags & MF_UNDERWATER))
	{
		mo->momx = FixedMul(mo->momx, FixedMul(FRICTION, P_XGSFix(PGS_GAMEWATERFRICTION)));
		mo->momy = FixedMul(mo->momy, FixedMul(FRICTION, P_XGSFix(PGS_GAMEWATERFRICTION)));
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
			if (!P_XGSVal(PGS_COSLIDEOFFMOFLOOR))
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
	
	P_XYFriction(mo, oldx, oldy, P_XGSVal(PGS_COOLDFRICTIONMOVE));
}

//
// P_ZMovement
//
void P_ZMovement(mobj_t* mo)
{
	fixed_t dist;
	fixed_t delta;
	fixed_t BounceMomZ;
	
	// GhostlyDeath <April 26, 2012> -- Improved on map object
	if (P_XGSVal(PGS_COIMPROVEDMOBJONMOBJ))
	{
	}
	
	// GhostlyDeath <April 26, 2012> -- Max Z obtained
	if (mo->z > mo->MaxZObtained)
		mo->MaxZObtained = mo->z;
	
	// Intercept the stupid 'fall through 3dfloors' bug SSNTails 06-13-2002
	if (P_XGSVal(PGS_COMOVECHECKFAKEFLOOR))
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
	
	if (P_MobjIsPlayer(mo))
	{
		// check for smooth step up
		if (mo->z < mo->floorz && !(mo->RXFlags[0] & MFREXA_NOSMOOTHSTEPUP))
		{
			mo->player->viewheight -= mo->floorz - mo->z;
			
			if (mo->player->ProfileEx)
				mo->player->deltaviewheight = ((mo->player->ProfileEx->ViewHeight) - mo->player->viewheight) >> 3;
			else
				mo->player->deltaviewheight = ((VIEWHEIGHT << FRACBITS) - mo->player->viewheight) >> 3;
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
	
	if (P_MobjIsPlayer(mo) && mo->flags2& MF2_FLY && !(mo->z <= mo->floorz) && leveltime & 2)
		mo->z += finesine[(FINEANGLES / 20 * leveltime >> 2) & FINEMASK];
	// clip movement
	
	// Spawn splashes, etc.
	// Hit the floor
	
	if (mo->z <= mo->floorz)
	{
		// GhostlyDeath <April 26, 2012> -- Precalc Bounce
		BounceMomZ = -mo->momz;
		
		// Bounce Factor
		BounceMomZ = FixedMul(BounceMomZ, mo->info->RBounceFactor);
				
		// GhostlyDeath <April 26, 2012> -- Non-missile bouncing
		if (!(mo->flags & MF_MISSILE) && (mo->RXFlags[1] & MFREXB_NONMISSILEFLBOUNCE))
			mo->momz = BounceMomZ;
		
		// hit the floor
		else if (mo->flags & MF_MISSILE)
		{
			mo->z = mo->floorz;
			if (mo->flags2 & MF2_FLOORBOUNCE)
			{
				P_FloorBounceMissile(mo);
				return;
			}
			else if (mo->flags2 & MF2_BOUNCES)
			{
				mo->momz = BounceMomZ;
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
		
		// GhostlyDeath <April 26, 2012> -- Max Z (For Bouncing)
		mo->MaxZObtained = mo->floorz;
		
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
			if (P_MobjIsPlayer(mo) && (mo->momz < -8 * FRACUNIT) && !(mo->flags2 & MF2_FLY))
			{
				// Squat down.
				// Decrease viewheight for a moment
				// after hitting the ground (hard),
				// and utter appropriate sound.
				mo->player->deltaviewheight = mo->momz >> 3;
				if (mo->RXFlags[0] & MFREXA_ISPLAYEROBJECT)
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
			mo->momz = -(P_XGSVal(PGS_GAMEGRAVITY) >> 3) * 2;
		else
			mo->momz -= P_XGSVal(PGS_GAMEGRAVITY) >> 3;
	}
	else if (!(mo->flags & MF_NOGRAVITY))	// Gravity here!
	{
		fixed_t gravityadd;
		
		//Fab: NOT SURE WHETHER IT IS USEFUL, just put it here too
		//     TO BE SURE there is no problem for the release..
		//     (this is done in P_Mobjthinker below normally)
		mo->eflags &= ~MF_JUSTHITFLOOR;
		
		gravityadd = -P_XGSVal(PGS_GAMEGRAVITY);
		
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
		// GhostlyDeath <April 26, 2012> -- Precalc Bounce
		BounceMomZ = -mo->momz;
		
		// Bounce Factor
		BounceMomZ = FixedMul(BounceMomZ, mo->info->RBounceFactor);
		
		mo->z = mo->ceilingz - mo->height;
		
		//added:22-02-98: player avatar hits his head on the ceiling, ouch!
		if (P_MobjIsPlayer(mo) && (mo->RXFlags[0] & MFREXA_ISPLAYEROBJECT) && (P_XGSVal(PGS_COOUCHONCEILING)) && !(mo->player->cheats & CF_FLYAROUND) && !(mo->flags2 & MF2_FLY) && mo->momz > 8 * FRACUNIT)
			S_StartSound(&mo->NoiseThinker, sfx_ouch);
		
		if (mo->flags2 & MF2_BOUNCES)
		{
			mo->momz = BounceMomZ;
			return;
		}
		
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
			if (P_XGSVal(PGS_COREMOVEMOINSKYZ) && mo->subsector->sector->ceilingpic == skyflatnum && mo->subsector->sector->ceilingheight == mo->ceilingz)
			{
				P_RemoveMobj(mo);
				return;
			}
			
			P_ExplodeMissile(mo);
			return;
		}
	}
	
	// z friction in water
	if (P_XGSVal(PGS_COWATERZFRICTION) && ((mo->eflags & MF_TOUCHWATER) || (mo->eflags & MF_UNDERWATER)) && !(mo->flags & (MF_MISSILE | MF_SKULLFLY)))
	{
		mo->momz = FixedMul(mo->momz, FixedMul(FRICTION, P_XGSFix(PGS_GAMEWATERFRICTION)));
	}
	
}

//
// P_NightmareRespawn
//
void P_NightmareRespawn(mobj_t* mobj, const bool_t a_ForceRespawn)
{
	fixed_t x;
	fixed_t y;
	fixed_t z;
	subsector_t* ss;
	mobj_t* mo;
	mapthing_t* mthing;
	int KCMode, i;
	
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
	if (!a_ForceRespawn)
		if (!P_CheckPosition(mobj, x, y, 0))
			return;					// no respwan
		
	// spawn a teleport fog at old spot
	// because of removal of the body?
	if (!mthing || (mthing && mthing->options & MTF_FS_SPAWNED))
		mo = P_SpawnMobj(mobj->x, mobj->y, mobj->z + (0), INFO_GetTypeByName("TeleportFog"));
	else
		mo = P_SpawnMobj(mobj->x, mobj->y, mobj->subsector->sector->floorheight + (0), INFO_GetTypeByName("TeleportFog"));
	// initiate teleport sound
	S_StartSound(&mo->NoiseThinker, sfx_telept);
	
	// spawn a teleport fog at the new spot
	ss = R_PointInSubsector(x, y);
	
	mo = P_SpawnMobj(x, y, ss->sector->floorheight + (0), INFO_GetTypeByName("TeleportFog"));
	
	S_StartSound(&mo->NoiseThinker, sfx_telept);
	
	// spawn it
	if (mobj->info->flags & MF_SPAWNCEILING)
		z = ONCEILINGZ;
	else if (!mthing || (mthing && mthing->options & MTF_FS_SPAWNED))
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
	
	KCMode = P_XGSVal(PGS_MONKILLCOUNTMODE);
	if (KCMode == 1)		// Count only once
		mo->flags &= ~MF_COUNTKILL;
	
	else if (KCMode == 2)	// Only Count Dead Monsters
	{
		// Reduce level kills
		g_MapKIS[0]--;
		
		// Player killed it?
		if (mobj->KillerPlayer)
			if (playeringame[mobj->KillerPlayer - 1])
				if (players[mobj->KillerPlayer - 1].FraggerID == mobj->FraggerID)
					players[mobj->KillerPlayer - 1].killcount--;
	}
	
	/* remove the old monster */
	// If under control of a player, remap player
	mo->player = mobj->player;
	mobj->player = NULL;
	
	// Player points to this object?
	if (mo->player && mo->player->mo == mobj)
	{
		mo->player->mo = mo;
		mo->player->health = mo->health;
		mo->player->playerstate = PST_LIVE;
		
		// Find local screen for player and correct angle
		for (i = 0; i < MAXSPLITSCREEN; i++)
			if (g_Splits[i].Active)
				if (mo->player == &players[g_Splits[i].Console])
					localangle[i] = mo->angle;
	}
	
	// Remove it
	P_RemoveMobj(mobj);
}

//
// P_MobjCheckWater : check for water, set stuff in mobj_t struct for
//                    movement code later, this is called either by P_MobjThinker() or P_PlayerThink()
void P_MobjCheckWater(mobj_t* mobj)
{
	sector_t* sector;
	fixed_t z;
	int oldeflags;
	
	// GhostlyDeath <March 6, 2012> -- Some things are not be in the water
	if (P_XGSVal(PGS_CONOUNDERWATERCHECK) || (mobj->RXFlags[0] & MFREXA_NOCHECKWATER))
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
			if (*rover->topheight <= mobj->z || *rover->bottomheight > (mobj->z + (__REMOOD_GETHEIGHT(mobj->info) >> 1)))
				continue;
				
			if (mobj->z + __REMOOD_GETHEIGHT(mobj->info) > *rover->topheight)
				mobj->eflags |= MF_TOUCHWATER;
			else
				mobj->eflags &= ~MF_TOUCHWATER;
				
			if (mobj->z + (__REMOOD_GETHEIGHT(mobj->info) >> 1) < *rover->topheight)
				mobj->eflags |= MF_UNDERWATER;
			else
				mobj->eflags &= ~MF_UNDERWATER;
				
			if (!(oldeflags & (MF_TOUCHWATER | MF_UNDERWATER)) &&
				((mobj->eflags & MF_TOUCHWATER) ||
				(mobj->eflags & MF_UNDERWATER)) && !(mobj->RXFlags[0] & MFREXA_NOWATERSPLASH))
				P_SpawnSplash(mobj, *rover->topheight);
		}
		return;
	}
	else
		mobj->eflags &= ~(MF_UNDERWATER | MF_TOUCHWATER);
		
	/*
	   if( (mobj->eflags ^ oldeflags) & MF_TOUCHWATER)
	   CONL_PrintF("touchewater %d\n",mobj->eflags & MF_TOUCHWATER ? 1 : 0);
	   if( (mobj->eflags ^ oldeflags) & MF_UNDERWATER)
	   CONL_PrintF("underwater %d\n",mobj->eflags & MF_UNDERWATER ? 1 : 0);
	 */
	// blood doesnt make noise when it falls in water
	if (!(oldeflags & (MF_TOUCHWATER | MF_UNDERWATER)) &&
	        ((mobj->eflags & MF_TOUCHWATER) || (mobj->eflags & MF_UNDERWATER)) && !(mobj->RXFlags[0] & MFREXA_NOWATERSPLASH) && P_XGSVal(PGS_COSPLASHTRANSWATER))
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
		if (mo->RXFlags[0] & MFREXA_ISPLAYEROBJECT)
			S_StartSound(&mo->NoiseThinker, sfx_oof);
	}
}

extern mobj_t* tmthing;
extern int tmflags;
extern fixed_t tmx;
extern fixed_t tmy;

/* PIT_StackThings() -- Stack map objects */
static bool_t PIT_StackThings(mobj_t* thing, void* a_Arg)
{
	fixed_t blockdist;
	mobj_t* SourceMo;
	size_t i, j;
	bool_t Less;
	fixed_t dx, dy, dr;
	
	/* Get Source */
	SourceMo = (mobj_t*)a_Arg;
	
	/* Ignore self and non-solids */
	if ((thing == tmthing) || (thing == SourceMo))
		return true;
		
	if (!(thing->flags & (MF_SOLID | MF_SPECIAL | MF_SHOOTABLE)))
		return true;
		
	/* Check Distance */
	dr = thing->radius + SourceMo->radius;
	dx = abs(thing->x - SourceMo->x);
	dy = abs(thing->y - SourceMo->y);
	
	// Too far away?
	if (P_AproxDistance(dx, dy) >= dr)
		return true;
	
	/* Z less than source? */
	// Place it on bottom
	if (thing->z < SourceMo->z)
	{
		i = 1;
		Less = true;
	}
	
	/* Z more than source? */
	// Place it on top
	else
	{
		i = 0;
		Less = false;
	}
	
	/* Inject in the list */
	Z_ResizeArray((void**)&SourceMo->MoOn[i], sizeof(*SourceMo->MoOn[i]), SourceMo->MoOnCount[i], SourceMo->MoOnCount[i] + 1);
	SourceMo->MoOn[i][SourceMo->MoOnCount[i]++] = thing;
	
	/* Continue on */
	return true;
}

//
// P_MobjThinker
//
void P_MobjThinker(mobj_t* mobj)
{
	int xl, xh, yl, yh, bx, by;
	bool_t checkedpos = false;	//added:22-02-98:
	size_t i, j;
	thinker_t Hold;
	uint32_t* TicA, TimeBase;
	bool_t ZMoveOffFloor;
	
	if (g_CheatFlags & MCF_FREEZETIME)
	{
		// Only players are not frozen
		if (!P_MobjIsPlayer(mobj))
			return;
	}
	
	// GhostlyDeath <June 12, 2012> -- Object Timers
	for (i = 0; i < 2; i++)
	{
		// Set the correct array
		if (!i)
			TicA = mobj->TimeThinking;
		else
			TicA = mobj->TimeFromDead;
		
		// Increase counter
		TicA[0]++;
		
		// Crossed minute?
		if (TicA[0] >= (35 * 60))
		{
			// Reset to zero
			TicA[0] = 0;
			
			// Add
			TicA[1]++;
		}
	}
	
	// If object is not dead, reset dead time
	if (mobj->health > 0 && !(mobj->flags & MF_CORPSE))
		mobj->TimeFromDead[0] = mobj->TimeFromDead[1] = 0;
	
	// Remove object that has been dead for a long time?
	else if (!P_MobjIsPlayer(mobj) && P_XGSVal(PGS_MONENABLECLEANUP))
	{
		// Get time base
		if (mobj->info->raisestate)
			TimeBase = P_XGSVal(PGS_MONCLEANUPRESPTIME);
		else
			TimeBase = P_XGSVal(PGS_MONCLEANUPNONRTIME);
		
		// Exceeded?
		if (mobj->TimeFromDead[1] >= TimeBase)
		{
			// Remove object
			P_RemoveMobj(mobj);
			return;
		}
	}
	
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
	{
		// GhostlyDeath <April 26, 2012> -- Improved on map object
		if (P_XGSVal(PGS_COIMPROVEDMOBJONMOBJ))
		{
		}
		
		//added:28-02-98: always do the gravity bit now, that's simpler
		//                BUT CheckPosition only if wasn't do before.
		if ((mobj->eflags & MF_ONGROUND) == 0 || (mobj->z != mobj->floorz) || mobj->momz)
		{
			// BP: since version 1.31 we use heretic z-cheching code
			//     kept old code for backward demo compatibility
			if (P_XGSVal(PGS_COUSEOLDZCHECK))
			{
			
				// if didnt check things Z while XYMovement, do the necessary now
				if (!checkedpos && P_XGSVal(PGS_COCHECKXYMOVE))
				{
					// FIXME : should check only with things, not lines
					P_CheckPosition(mobj, mobj->x, mobj->y, 0);
					
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
			
			// GhostlyDeath <April 26, 2012> -- Improved on map object
			else if (P_XGSVal(PGS_COIMPROVEDMOBJONMOBJ))
			{
				// Modify floorz/ceilingz of object based on stacks
				
				// Always perform Z Movement
				P_ZMovement(mobj);
			}
			
			else if (mobj->flags2 & MF2_PASSMOBJ)
			{
				mobj_t* onmo;
				
				onmo = P_CheckOnmobj(mobj);
				if (!onmo)
				{
					P_ZMovement(mobj);
					if (P_MobjIsPlayer(mobj) && mobj->flags & MF2_ONMOBJ)
						mobj->flags2 &= ~MF2_ONMOBJ;
				}
				else
				{
					if (P_MobjIsPlayer(mobj))
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
	}
			
	// SoM: Floorhuggers stay on the floor allways...
	// BP: tested here but never set ?!
	if (mobj->info->flags2 & MF2_FLOORHUGGER)
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
		if (!P_XGSVal(PGS_MONRESPAWNMONSTERS))
			return;
		
		// Only monsters respawn
		if (!(mobj->RXFlags[0] & MFREXA_ISMONSTER))
			return;
		
		// GhostlyDeath <June 15, 2012> -- No Nightmare Respawn
		if (mobj->RXFlags[1] & MFREXB_NONMRESPAWN)
			return;
		
		// GhostlyDeath <December 29, 2012> -- Object is corpse
		if (P_XGSVal(PGS_CORESPAWNCORPSESONLY))
			if (!(mobj->flags & MF_CORPSE))
				return;
			
		mobj->movecount++;
		
		if (mobj->movecount < P_XGSVal(PGS_MONRESPAWNMONSTERSTIME) * TICRATE)
			return;
		
		if (!P_XGSVal(PGS_MONSTATICRESPAWNTIME))
		{
			if (leveltime % (32))
				return;
			
			if (P_Random() > 4)
				return;
		}
			
		P_NightmareRespawn(mobj, false);
	}
	
}

void P_MobjNullThinker(mobj_t* mobj)
{
}

//
// P_SpawnMobj
//
mobj_t* P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, PI_mobjid_t type)
{
	mobj_t* mobj;
	PI_state_t* st;
	PI_mobj_t* info;
	int i;
	static uint32_t LastOrder;
	
	/* Check Type */
	if (type < 0 || type >= NUMMOBJTYPES)
		return NULL;
	
	mobj = Z_Malloc(sizeof(*mobj), PU_LEVEL, NULL);
	memset(mobj, 0, sizeof(*mobj));
	info = mobjinfo[type];
	
	mobj->SpawnOrder = ++LastOrder;
	mobj->type = type;
	mobj->info = info;
	mobj->x = x;
	mobj->y = y;
	mobj->radius = info->radius;
	mobj->height = __REMOOD_GETHEIGHT(info);
	mobj->flags = info->flags;
	mobj->flags2 = info->flags2;
	mobj->RXShotWithWeapon = NUMWEAPONS;
	mobj->SkinTeamColor = 0;
	
	for (i = 0; i < NUMINFORXFIELDS; i++)
		mobj->RXFlags[i] = info->RXFlags[i];
	
	mobj->health = info->spawnhealth;
	
	if (P_XGSVal(PGS_GAMESKILL) != sk_nightmare)
		mobj->reactiontime = info->reactiontime;
		
	if (P_XGSVal(PGS_CORANOMLASTLOOKSPAWN) && !(mobj->RXFlags[0] & MFREXA_NORANDOMPLAYERLOOK))
		mobj->lastlook = P_Random() % P_XGSVal(PGS_COLASTLOOKMAXPLAYERS);
	else
		mobj->lastlook = -1;	// stuff moved in P_enemy.P_LookForPlayer
		
	// do not set the state with P_SetMobjState,
	// because action routines can not be called yet
	st = states[info->spawnstate];
	
	mobj->state = st;
	mobj->tics = st->tics;
	mobj->sprite = st->sprite;
	mobj->frame = st->frame;	// FF_FRAMEMASK for frame, and other bits..
	mobj->touching_sectorlist = NULL;	//SoM: 4/7/2000
	mobj->friction = ORIG_FRICTION;	//SoM: 4/7/2000
	
	// GhostlyDeath <March 6, 2012> -- Init tics
	PS_AdjMobjStateTics(mobj);
	
	// BP: SoM right ? if not ajust in p_saveg line 625 and 979
	mobj->movefactor = ORIG_FRICTION_FACTOR;
	
	// set subsector and/or block links
	P_SetThingPosition(mobj);
	
	mobj->floorz = mobj->subsector->sector->floorheight;
	mobj->ceilingz = mobj->subsector->sector->ceilingheight;
	
	// GhostlyDeath <April 26, 2012> -- Max Z (For Bouncing)
	mobj->MaxZObtained = mobj->floorz;
	
	//added:27-02-98: if ONFLOORZ, stack the things one on another
	//                so they do not occupy the same 3d space
	//                allow for some funny thing arrangements!
	if (z == ONFLOORZ)
	{
		//if (!P_CheckPosition(mobj,x,y, 0))
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
		   P_CheckPosition(mobj,x,y, 0);
		   mobj->z = tmfloorz+FRACUNIT;
		
		   // second check at the good z pos
		   P_CheckPosition(mobj,x,y, 0);
		
		   mobj->floorz = tmfloorz;
		   mobj->ceilingz = tmsectorceilingz;
		   mobj->z = tmfloorz;
		   // thing not on solid ground
		   if (tmfloorthing)
		   mobj->eflags &= ~MF_ONGROUND;
			
		
		   }
		   else
		 */
		mobj->z = mobj->floorz;
		
	}
	else if (z == ONCEILINGZ)
		mobj->z = mobj->ceilingz - __REMOOD_GETHEIGHT(mobj->info);
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
		//CONL_PrintF("mobj spawned at z %d\n",z>>16);
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
		// GhostlyDeath <March 6, 2012> -- Don't use thinker for object
	if (mobj->RXFlags[0] & MFREXA_USENULLMOTHINKER)
		mobj->thinker.function.acv = (actionf_p1) P_MobjNullThinker;
	else
	{
		mobj->thinker.function.acp1 = (actionf_p1) P_MobjThinker;
		P_AddThinker(&mobj->thinker, PTT_MOBJ);
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
	size_t i;
	thinker_t Hold;
	
	if (!mobj)
		return;
		
	/* Remove object from bots */
	B_RemoveThinker(mobj);
	
	// GhostlyDeath <May 8, 2012> -- Remove from queue
	P_RemoveFromBodyQueue(mobj);
	
	if ((mobj->flags & MF_SPECIAL) && !(mobj->flags & MF_DROPPED) && !(mobj->RXFlags[0] & MFREXA_NOALTDMRESPAWN) && mobj->spawnpoint)
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
	
	// GhostlyDeath <January 23, 2012> -- If spawn point is set, remove from there
		// GhostlyDeath <March 6, 2012> -- Moved from top (otherwise stops respawning items)
	if (mobj->spawnpoint)
	{
		mobj->spawnpoint->mobj = NULL;
		mobj->spawnpoint = NULL;
	}
	
	// GhostlyDeath <April 21, 2012> -- Remove sound reference
	P_RemoveRecursiveSound(mobj);
	
	// GhostlyDeath <April 20, 2012> -- Remove refs from players so they don't get followed for dead players
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
		{
			// Remove attacker (follow player)
			if (players[i].attacker == mobj)
				players[i].attacker = NULL;
			
			// Remove BFG ball from player
			if (players[i].LastBFGBall == mobj)
				players[i].LastBFGBall = NULL;
			
			// Remove attackee
			if (players[i].Attackee == mobj)
				players[i].Attackee = NULL;
			
			// Remove references done by players
			if (players[i].mo)
			{
				// Remove Target
				if (players[i].mo->target)
					P_RefMobj(PMRT_TARGET, players[i].mo, NULL);
				
				// Remove Tracer
				if (players[i].mo->tracer)
					P_RefMobj(PMRT_TRACER, players[i].mo, NULL);
				
				// Remove Follow Player
				if (players[i].mo->FollowPlayer)
					P_RefMobj(PMRT_FOLLOWPLAYER, players[i].mo, NULL);
				
				// Object is our player object!
				if (players[i].mo == mobj)
				{
					players[i].mo = NULL;
					players[i].playerstate = PST_REBORN;
				}
			}
		}
	
	// GhostlyDeath <April 21, 2012> -- Remove object references
	for (i = 0; i < NUMPMOBJREFTYPES; i++)
		P_RefMobj(i, mobj, NULL);
	
	// GhostlyDeath <April 21, 2012> -- Clear objects that reference this
	P_ClearMobjRefs(mobj);
	
	// GhostlyDeath <April 21, 2012> -- Check reference count
	for (i = 0; i < NUMPMOBJREFTYPES; i++)
		if (mobj->RefCount[i] > 0)
		{
			CONL_PrintF("mobj %p is still referenced (%i = %i).\n", mobj, (int)i, (int)mobj->RefCount[i]);
			P_FindMobjRef(i, mobj);
			I_Error("mobj still referenced.\n");
			break;
		}
	
	// Invalidate some things
	mobj->RemType = mobj->type;
	mobj->type = -1;
	mobj->info = NULL;
	
	// De-allocate some final things
	for (i = 0; i < NUMPMOBJREFTYPES; i++)
	{
		if (mobj->RefList[i])
			Z_Free(mobj->RefList[i]);
		mobj->RefList[i] = NULL;
	}
	
	// Remove Thinker
	P_RemoveThinker((thinker_t*) mobj);

	// GhostlyDeath <April 20, 2012> -- Invalidate the entire map object structure
		// MIGHT BREAK DEMO COMPAT
	memmove(&Hold, &mobj->thinker, sizeof(thinker_t));
	memset(mobj, 0xFF, sizeof(*mobj));
	memmove(&mobj->thinker, &Hold, sizeof(thinker_t));
	
	// Set to crash
	mobj->thinker.Type = PTT_DELETEME;
	P_SetMobjToCrash(mobj);
}

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
	if (!P_XGSVal(PGS_ITEMRESPAWNITEMS))
		return;					//
		
	// nothing left to respawn?
	if (iquehead == iquetail)
		return;
		
	// the first item in the queue is the first to respawn
	// wait at least 30 seconds
	if (leveltime - itemrespawntime[iquetail] < (tic_t)P_XGSVal(PGS_ITEMRESPAWNITEMSTIME) * TICRATE)
		return;
		
	mthing = itemrespawnque[iquetail];
	
	if (!mthing)				// Hurdler: grrrr, very ugly hack that need to be fixed!!!
	{
		CONL_PrintF("Warning: couldn't respawn a thing. This is a known bug with FS and saved games.\n");
		// pull it from the que
		iquetail = (iquetail + 1) & (ITEMQUESIZE - 1);
		return;
	}
	
	x = mthing->x << FRACBITS;
	y = mthing->y << FRACBITS;
	
	// spawn a teleport fog at the new spot
		// TODO -- GhostlyDeath: How about custom respawn fogs for items
	ss = R_PointInSubsector(x, y);
	if (mthing->options & MTF_FS_SPAWNED)
		mo = P_SpawnMobj(x, y, mthing->z << FRACBITS, INFO_GetTypeByName("ItemFog"));
	else
		mo = P_SpawnMobj(x, y, ss->sector->floorheight, INFO_GetTypeByName("ItemFog"));
	S_StartSound(&mo->NoiseThinker, sfx_itmbk);
	
	// find which type to spawn
	for (i = 0; i < NUMMOBJTYPES; i++)
		if (mthing->type == mobjinfo[i]->EdNum[g_CoreGame])
			break;
			
	// spawn it
	if (mobjinfo[i]->flags & MF_SPAWNCEILING)
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
		
		// GhostlyDeath <March 6, 2012> -- Use remembered values here
			// If the mthing was marked as a gun then respawn it
		if (mthing->MarkedWeapon && mthing->MoType >= 0 && mthing->MoType <= NUMMOBJTYPES)
			i = mthing->MoType;
		
		// Not remembered? remove from queue and move on
		else
		{
			// Free slot?
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
			// TODO -- GhostlyDeath: How about custom respawn fogs for items
		ss = R_PointInSubsector(x, y);
		if (mthing->options & MTF_FS_SPAWNED)
			mo = P_SpawnMobj(x, y, mthing->z << FRACBITS, INFO_GetTypeByName("ItemFog"));
		else
			mo = P_SpawnMobj(x, y, ss->sector->floorheight, INFO_GetTypeByName("ItemFog"));
		S_StartSound(&mo->NoiseThinker, sfx_itmbk);
		
		// spawn it
		if (mobjinfo[i]->flags & MF_SPAWNCEILING)
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

/* P_RemoveFromBodyQueue() -- Remove body from queue */
void P_RemoveFromBodyQueue(mobj_t* const a_Mo)
{
	size_t i;
	
	/* Look for it and remove */
	for (i = 0; i < BODYQUESIZE; i++)
		if (bodyque[i] == a_Mo)
		{
			bodyque[i] = NULL;
			break;
		}
}

/* P_SpawnPlayerBackup() -- Spawns player at (0, 0) */
void P_SpawnPlayerBackup(int32_t const a_PlayerNum)
{
	mapthing_t ZeroThing;
	
	/* Setup Thing */
	memset(&ZeroThing, 0, sizeof(ZeroThing));
	
	// Set player id
	ZeroThing.type = a_PlayerNum + 1;
	
	/* Spawn */
	P_SpawnPlayer(&ZeroThing);
}

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
	int32_t TeamColor;
	
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
		CONL_PrintF("WARNING - P_SpawnPlayer: playernum not valid %i. (%s:%i).\n", playernum, __FILE__, __LINE__);
		return;
	}
	
	// Remove bodies
	// GhostlyDeath <April 20, 2012> -- Remove bodies here so they actually GET removed!
	if (P_XGSVal(PGS_COBETTERPLCORPSEREMOVAL))
	{
		if (bodyqueslot >= BODYQUESIZE)
			P_RemoveMobj(bodyque[bodyqueslot % BODYQUESIZE]);
		bodyque[bodyqueslot % BODYQUESIZE] = players[playernum].mo;
		bodyqueslot++;
	}
	
	p = &players[playernum];
	
	if (p->playerstate == PST_REBORN)
		G_PlayerReborn(playernum);
		
	x = mthing->x << FRACBITS;
	y = mthing->y << FRACBITS;
	
	z = ONFLOORZ;
	
	mobj = P_SpawnMobj(x, y, z, INFO_GetTypeByName("DoomPlayer"));
	//SoM:
	mthing->mobj = mobj;
	
	// set color translations for player sprites
	// added 6-2-98 : change color : now use skincolor (befor is mthing->type-1
	
	/* Set player sprite color */
	// New Game Modes Enabled and is a team mode
	mobj->flags |= (p->skincolor) << MF_TRANSSHIFT;
	
	//
	// set 'spritedef' override in mobj for player skins.. (see ProjectSprite)
	// (usefulness : when body mobj is detached from player (who respawns),
	//  the dead body mobj retain the skin through the 'spritedef' override).
	mobj->skin = p->skin;
	
	mobj->angle = ANG45 * (mthing->angle / 45);
	
	for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
		if (g_Splits[i].Active)
			if (playernum == g_Splits[i].Console)
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
	
	if (p->ProfileEx)
		p->viewheight = p->ProfileEx->ViewHeight;
	else
		p->viewheight = VIEWHEIGHT << FRACBITS;
	
	// added 2-12-98
	p->viewz = p->mo->z + p->viewheight;
	
	// GhostlyDeath <April 13, 2012> -- Fix weapons
	p->weaponinfo = wpnlev1info;
	
	p->flamecount = 0;
	p->flyheight = 0;
	
	// GhostlyDeath <May 7, 2012> -- Don't effort bob the new player
	p->MoveMom = 0;
	p->TargetViewZ = p->viewheight;
	p->FakeMom[0] = p->FakeMom[1] = p->FakeMom[2] = 0;
	
	// setup gun psprite
	P_SetupPsprites(p);
	
	// give all cards in death match mode
	if (P_GMIsDM() || P_XGSVal(PGS_PLSPAWNWITHALLKEYS))
	{
		p->cards = it_allkeys;
		p->KeyCards[0] = 0;
		p->KeyCards[0] = p->KeyCards[1] = ~p->KeyCards[0];
	}
		
	if (playernum == g_Splits[0].Console)
	{
		// wake up the status bar
		ST_Start();
	}
	
	if (p->camera.chase)
		P_ResetCamera(p);
		
	// GhostlyDeath <April 20, 2012> -- Telefrag whatever was here
	if (P_XGSVal(PGS_PLSPAWNTELEFRAG))
	{
		P_TeleportMove(mobj, mobj->x, mobj->y);
		mobj->reactiontime = 0;	// Don't telefreeze
	}
}

//
// P_SpawnMapThing
// The fields of the mapthing should
// already be in host uint8_t order.
//
void P_SpawnMapThing(mapthing_t* mthing)
{
	int i, j, pid;
	int bit;
	mobj_t* mobj;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	G_Skill_t Skill;
	
	// GhostlyDeath <March 6, 2012> -- Clear thing ID
	mthing->MoType = NUMMOBJTYPES;
	mthing->MarkedWeapon = false;
	
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
		
		// Player ID
		pid = mthing->type - 1;
		
		// old version spawn player now, new version spawn player when level is
		// loaded, or in network event later when player join game
		// TODO: GhostlyDeath -- This has to do with voodoo dolls!
		if (!P_GMIsDM() && ((playeringame[pid] && !players[pid].mo) || P_XGSVal(PGS_COVOODOODOLLS)))
			if (!players[pid].CounterOpPlayer)
				P_SpawnPlayer(mthing);
		
		return;
	}
	
	// GhostlyDeath <February 15, 2012> -- Check for Team Starts
	// 5081 = Red
	// 5080 = Blue
	if (mthing->type >= 5080 && mthing->type <= 5084)
	{
		// Get Real Team Color
		switch (mthing->type)
		{
			case 5081:	i = 0; break;
			case 5080:	i = 1; break;
			case 5083:	i = 2; break;
			case 5084:	i = 3; break;
			default:	i = -1; break;
		}
		
		// Find free spot in start list
		if (i >= 0)
			for (j = 0; j < MAXPLAYERS; j++)
				if (!g_TeamStarts[i][j])
				{
					g_TeamStarts[i][j] = mthing;
					mthing->type = i + 1;
					break;
				}
		
		// Return
		return;
	}
	
	// Multiplayer Spawns
	if (!P_XGSVal(PGS_GAMESPAWNMULTIPLAYER) && (mthing->options & 16))
		return;
		
	//SoM: 4/7/2000: Implement "not deathmatch" thing flag
	if (P_GMIsDM() && (mthing->options & 32))
		return;
		
	//SoM: 4/7/2000: Implement "not cooperative" thing flag
	if (!P_GMIsDM() && (mthing->options & 64))
		return;
	
	// check for apropriate skill level
	Skill = P_XGSVal(PGS_GAMESKILL);
	
	if (Skill <= sk_easy)
		bit = 0x0001;
	else if (Skill >= sk_hard)
		bit = 0x0004;
	else
		bit = 0x0002;
		
	if (!(mthing->options & bit))
		return;
		
	// find which type to spawn (woo hacky and I like it)
	for (i = NUMMOBJTYPES; i > 0; i--)
		if (mthing->type == mobjinfo[i - 1]->EdNum[g_CoreGame])
			break;
			
	if (i == 0)
	{
		CONL_PrintF("\2P_SpawnMapThing: Unknown type %i at (%i, %i)\n", mthing->type, mthing->x, mthing->y);
		return;
	}
	
	// Subtract to get real ID
	i -= 1;
	
	// GhostlyDeath <June 6, 2012> -- Spawn Pickups?
	if (!P_XGSVal(PGS_ITEMSSPAWNPICKUPS) && (mobjinfo[i]->flags & MF_SPECIAL))
		return;
	
	// GhostlyDeath <March 6, 2012> -- Set thing ID and mark with weapon if possible
	mthing->MoType = i;
	
	if (mobjinfo[mthing->MoType]->RXFlags[0] & MFREXA_MARKRESTOREWEAPON)
		mthing->MarkedWeapon = true;
	
	// don't spawn keycards and players in deathmatch
	if (P_GMIsDM() && mobjinfo[i]->flags & MF_NOTDMATCH)
		return;
		
	// don't spawn any monsters if -nomonsters
	if (!P_XGSVal(PGS_MONSPAWNMONSTERS) && ((mobjinfo[i]->RXFlags[0] & MFREXA_ISMONSTER) || (mobjinfo[i]->flags & MF_COUNTKILL)))
		return;
		
	// spawn it
	x = mthing->x << FRACBITS;
	y = mthing->y << FRACBITS;
	
	if (mobjinfo[i]->flags & MF_SPAWNCEILING)
		z = ONCEILINGZ;
	else if (mobjinfo[i]->flags2 & MF2_SPAWNFLOAT)
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
	
	if (!P_XGSVal(PGS_COENABLESPLASHES))
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
	th = P_SpawnMobj(mo->x + 1, mo->y + 1, z, INFO_GetTypeByName("LegacySplash"));
	
	if( z - mo->subsector->sector->floorheight > 4*FRACUNIT)
		S_StartSound(&th->NoiseThinker, sfx_gloop);
	else
	    S_StartSound(&th->NoiseThinker, sfx_splash);
	th->tics -= P_Random() & 3;
	
	if (th->tics < 1)
		th->tics = 1;
}

// --------------------------------------------------------------------------
// P_SpawnSmoke
// --------------------------------------------------------------------------
// when player gets hurt by lava/slime, spawn at feet
void P_SpawnSmoke(fixed_t x, fixed_t y, fixed_t z)
{
	mobj_t* th;
	
	if (!P_XGSVal(PGS_COENABLESMOKE))
		return;
		
	x = x - ((P_Random() & 8) * FRACUNIT) - 4 * FRACUNIT;
	y = y - ((P_Random() & 8) * FRACUNIT) - 4 * FRACUNIT;
	z += (P_Random() & 3) * FRACUNIT;
	
	th = P_SpawnMobj(x, y, z, INFO_GetTypeByName("LegacySmoke"));
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
	
	if (!puff)
		return;
	
	// GhostlyDeath <March 6, 2012> -- Randomized puff time?
	if (puff->RXFlags[0] & MFREXA_RANDOMPUFFTIME)
	{
		puff->momz = FRACUNIT;
		puff->tics -= P_Random() & 3;
		
		if (puff->tics < 1)
			puff->tics = 1;
	}
	
	// GhostlyDeath <March 6, 2012> -- Close range state
	if (attackrange == MELEERANGE)
		if (puff->info->RMeleePuffState)
			P_SetMobjState(puff, puff->info->RMeleePuffState);
}

// --------------------------------------------------------------------------
// P_SpawnBlood
// --------------------------------------------------------------------------

static mobj_t* bloodthing;
static fixed_t bloodspawnpointx, bloodspawnpointy;

#ifdef WALLSPLATS
bool_t PTR_BloodTraverse(intercept_t* in, void* a_Data)
{
	line_t* li;
	divline_t divl;
	fixed_t frac;
	mobj_t* BleedThing = a_Data;
	
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
		R_AddWallSplat(
				li,
				P_PointOnLineSide(bloodspawnpointx, bloodspawnpointy, li),
				(BleedThing && BleedThing->info->RBloodSplat ? BleedThing->info->RBloodSplat : "BLUDC0"),
				z,
				frac,
				SPLATDRAWMODE_TRANS
			);
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
void P_SpawnBloodSplats(fixed_t x, fixed_t y, fixed_t z, int damage, fixed_t momx, fixed_t momy, mobj_t* const a_BleedThing)
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
	P_SpawnBlood(x, y, z, damage, a_BleedThing);
	//CONL_PrintF ("spawned blood counter %d\n", counter++);
	if (!P_XGSVal(PGS_COENABLEBLOODSPLATS))
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
		
	//CONL_PrintF ("spawning %d bloodsplats at distance of %d\n", numsplats, distance);
	//CONL_PrintF ("damage %d\n", damage);
	bloodspawnpointx = x;
	bloodspawnpointy = y;
	//uses 'bloodthing' set by P_SpawnBlood()
	for (i = 0; i < numsplats; i++)
	{
		// find random angle between 0-180deg centered on damage angle
		anglesplat = angle + (((P_Random() - 128) * FINEANGLES / 512 * anglemul) << ANGLETOFINESHIFT);
		x2 = x + distance * finecosine[anglesplat >> ANGLETOFINESHIFT];
		y2 = y + distance * finesine[anglesplat >> ANGLETOFINESHIFT];
		
		P_PathTraverse(x, y, x2, y2, PT_ADDLINES, PTR_BloodTraverse, a_BleedThing);
	}
#endif
}

// P_SpawnBlood
// spawn a blood sprite with falling z movement, at location
// the duration and first sprite frame depends on the damage level
// the more damage, the longer is the sprite animation
void P_SpawnBlood(fixed_t x, fixed_t y, fixed_t z, int damage, mobj_t* const a_BleedThing)
{
	mobj_t* th;
	
	z += P_SignedRandom() << 10;
	th = P_SpawnMobj(
			x, y, z,
			INFO_GetTypeByName(
				(a_BleedThing && a_BleedThing->info->RBloodSpewClass ?
						a_BleedThing->info->RBloodSpewClass :
						__REMOOD_GETBLOODKIND))
		);
	
	// GhostlyDeath <April 12, 2012> -- 1.28 and up added blood spewing
	if (P_XGSVal(PGS_CORANDOMBLOODDIR))
	{
		th->momx = P_SignedRandom() << 12;	//faB:19jan99
		th->momy = P_SignedRandom() << 12;	//faB:19jan99
	}
	
	th->momz = FRACUNIT * 2;
	th->tics -= P_Random() & 3;
	
	if (th->tics < 1)
		th->tics = 1;
	
	// GhostlyDeath <March 6, 2012> -- Less blood?
	if (damage <= 12 && damage >= 9)
	{
		if (th->info->RLessBlood[0])
			P_SetMobjState(th, th->info->RLessBlood[0]);
	}
	else if (damage < 9)
	{
		if (th->info->RLessBlood[1])
			P_SetMobjState(th, th->info->RLessBlood[1]);
	}
		
	bloodthing = th;
}

/* P_HitFloor() -- Object hits floor */
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
	
	// GhostlyDeath <June 7, 2012> -- No splash effects?
	if (!(thing->RXFlags[0] & MFREXA_NOWATERSPLASH))
		return floortype;
	
	// Legacy Water
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
	
	if (!P_TryMove(th, th->x, th->y, false, NULL, NULL))
	{
		P_ExplodeMissile(th);
		return false;
	}
	return true;
}

//
// P_SpawnMissile
//
mobj_t* P_SpawnMissile(mobj_t* source, mobj_t* dest, PI_mobjid_t type)
{
	mobj_t* th;
	angle_t an;
	int dist;
	fixed_t z;
	
	// GhostlyDeath <April 29, 2012> -- Player shooting missile
	if (source->player)
		return P_SpawnPlayerMissile(source, type);
	
	// GhostlyDeath <November 3, 2010> -- Paranoia removal
	if (!source || !dest)
	{
		CONL_PrintF("WARNING - P_SpawnMissile: source %p, dest %p (%s:%i).\n", source, dest, __FILE__, __LINE__);
		return NULL;
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
	
	if (th->info->RSeeSound)
		S_StartSound(&th->NoiseThinker, S_SoundIDForName(th->info->RSeeSound));
		
	P_RefMobj(PMRT_TARGET, th, source);		// where it came from
	
	if (P_XGSVal(PGS_MONPREDICTMISSILES))	//added by AC for predmonsters
	{
		bool_t canHit;
		fixed_t px, py, pz;
		int time, t;
		subsector_t* sec;
		
		// TODO FIXME: If someone specifies a speed of zero the game will crash!
		
		dist = P_AproxDistance(dest->x - source->x, dest->y - source->y);
		time = dist / __REMOOD_GETSPEEDMO(th);
		time = P_AproxDistance(dest->x + dest->momx * time - source->x, dest->y + dest->momy * time - source->y) / __REMOOD_GETSPEEDMO(th);
		
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
		if ((dest->flags & MF_SHADOW) || P_XGSVal(PGS_FUNMONSTERSMISSMORE))
			an += P_SignedRandom() << 20;
			
		th->angle = an;
		an >>= ANGLETOFINESHIFT;
		th->momx = FixedMul(__REMOOD_GETSPEEDMO(th), finecosine[an]);
		th->momy = FixedMul(__REMOOD_GETSPEEDMO(th), finesine[an]);
		
		if (t < 1)
			t = 1;
			
		th->momz = (pz - source->z) / t;
	}
	else
	{
		an = R_PointToAngle2(source->x, source->y, dest->x, dest->y);
		
		// fuzzy player
		if ((dest->flags & MF_SHADOW) || P_XGSVal(PGS_FUNMONSTERSMISSMORE))
		{
			an += P_SignedRandom() << 20;
		}
		
		th->angle = an;
		an >>= ANGLETOFINESHIFT;
		th->momx = FixedMul(__REMOOD_GETSPEEDMO(th), finecosine[an]);
		th->momy = FixedMul(__REMOOD_GETSPEEDMO(th), finesine[an]);
		
		dist = P_AproxDistance(dest->x - source->x, dest->y - source->y);
		dist = dist / __REMOOD_GETSPEEDMO(th);
		
		if (dist < 1)
			dist = 1;
			
		th->momz = (dest->z - source->z) / dist;
	}
	
	dist = P_CheckMissileSpawn(th);
	if (P_XGSVal(PGS_COALWAYSRETURNDEADSPMISSILE))
		return th;
	else
		return dist ? th : NULL;
}

//
// P_SpawnPlayerMissile
// Tries to aim at a nearby monster
//
mobj_t* P_SPMAngle(mobj_t* source, PI_mobjid_t type, angle_t angle)
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
	if (P_XGSVal(PGS_COFORCEAUTOAIM) || ((source->player->autoaim_toggle && P_XGSVal(PGS_PLALLOWAUTOAIM))))
	{
		// see which target is to be aimed at
		slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT, NULL);
		
		if (!linetarget)
		{
			an += 1 << 26;
			slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT, NULL);
			
			if (!linetarget)
			{
				an -= 2 << 26;
				slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT, NULL);
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
	
	if (!P_XGSVal(PGS_COFORCEAUTOAIM))
		if (!(source->player->autoaim_toggle && P_XGSVal(PGS_PLALLOWAUTOAIM)) || (!linetarget && P_XGSVal(PGS_COMOUSEAIM)))
		{
			if (P_XGSVal(PGS_COUSEMOUSEAIMING))
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
	
	if (th->info->RSeeSound)
		S_StartSound(&th->NoiseThinker, S_SoundIDForName(th->info->RSeeSound));
		
	P_RefMobj(PMRT_TARGET, th, source);
	
	th->angle = an;
	th->momx = FixedMul(__REMOOD_GETSPEEDMO(th), finecosine[an >> ANGLETOFINESHIFT]);
	th->momy = FixedMul(__REMOOD_GETSPEEDMO(th), finesine[an >> ANGLETOFINESHIFT]);
	
	if (P_XGSVal(PGS_COFIXPLAYERMISSILEANGLE))
	{
		// 1.28 fix, allow full aiming must be much precise
		th->momx = FixedMul(th->momx, finecosine[source->player->aiming >> ANGLETOFINESHIFT]);
		th->momy = FixedMul(th->momy, finecosine[source->player->aiming >> ANGLETOFINESHIFT]);
	}
	
	th->momz = FixedMul(__REMOOD_GETSPEEDMO(th), slope);
	
	slope = P_CheckMissileSpawn(th);
	
	// GhostlyDeath <March 6, 2012> -- Set weapon fired with from player
	if (P_MobjIsPlayer(source))
		th->RXShotWithWeapon = source->player->readyweapon;
	
	else	// Otherwise carry the original weapon
		th->RXShotWithWeapon = source->RXShotWithWeapon;
	
	if (P_XGSVal(PGS_COALWAYSRETURNDEADSPMISSILE))
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

bool_t inventory = false;

/******************************************************************************/

/* P_RefMobj() -- Reference Object */
mobj_t* P_RefMobjReal(const P_MobjRefType_t a_Type, mobj_t* const a_SourceRef, mobj_t* const a_RefThis
#if defined(_DEBUG)
		, const char* const File, const int Line
#endif
	)
{
	P_MobjRefLog_t* Log;
	mobj_t** ChangePtr;
	size_t i;
	
	/* Check */
	if (!a_SourceRef || a_Type < 0 || a_Type >= NUMPMOBJREFTYPES)
		return NULL;
	
	/* Determine options */
	if (a_Type == PMRT_TARGET)
		ChangePtr = &a_SourceRef->target;
	else if (a_Type == PMRT_TRACER)
		ChangePtr = &a_SourceRef->tracer;
	else if (a_Type == PMRT_FOLLOWPLAYER)
		ChangePtr = &a_SourceRef->FollowPlayer;
	else
		ChangePtr = NULL;
	
	/* Nothing to change? */
	// Oops!
	if (!ChangePtr)
	{
		if (devparm)
			CONL_PrintF("Unknown ref type %u?\n", a_Type);
		return NULL;
	}
	
	/* Remove old reference */
	if (*ChangePtr)
	{
		// Find reference in list and remove it
		for (i = 0; i < (*ChangePtr)->RefListSz[a_Type]; i++)
			if ((*ChangePtr)->RefList[a_Type][i] == a_SourceRef)
				(*ChangePtr)->RefList[a_Type][i] = NULL;
		
		// Reduce reference of original
		(*ChangePtr)->RefCount[a_Type]--;
		
		// Remove old reference
		(*ChangePtr) = NULL;
	}
	
	/* Set new reference */
	(*ChangePtr) = a_RefThis;
	
	// And increment it
	if (a_RefThis)
	{
		// Probably illegal?
		if (a_RefThis > 0 && a_RefThis < sizeof(*a_RefThis))
			if (devparm)
				I_Error("Illegal low reference.");
		
		// Add reference count
		(*ChangePtr)->RefCount[a_Type]++;
		
		// Find blank spot in reference list
		for (i = 0; i < (*ChangePtr)->RefListSz[a_Type]; i++)
			if (!(*ChangePtr)->RefList[a_Type][i])
			{
				(*ChangePtr)->RefList[a_Type][i] = a_SourceRef;
				break;
			}
		
		// No spot?
		if (i >= (*ChangePtr)->RefListSz[a_Type])
		{
			// Resize it
			Z_ResizeArray(
					(void**)&((*ChangePtr)->RefList[a_Type]),
					sizeof((*ChangePtr)->RefList[a_Type]),
					(*ChangePtr)->RefListSz[a_Type],
					(*ChangePtr)->RefListSz[a_Type] + 1
				);
			
			// Add to back
			(*ChangePtr)->RefList[a_Type][(*ChangePtr)->RefListSz[a_Type]++] = a_SourceRef;
			
			// Change tag
			Z_ChangeTag((*ChangePtr)->RefList[a_Type], PU_LEVEL);
		}
	}

#if defined(_DEBUG)
	if (a_SourceRef->RefFile[a_Type])
		Z_Free(a_SourceRef->RefFile[a_Type]);
	//a_SourceRef->RefFile[a_Type] = Z_StrDup(File, PU_LEVEL, NULL);
	a_SourceRef->RefLine[a_Type] = Line;
#endif
	
	/* Return the changed to reference */
	return (*ChangePtr);
}

/* P_FindMobjRef() -- Find reference of object */
void P_FindMobjRef(const P_MobjRefType_t a_Type, mobj_t* const a_SourceRef)
{
#if defined(_DEBUG)
	thinker_t* currentthinker;
	mobj_t* mo;
	mobj_t** ChangePtr;

	for (currentthinker = thinkercap.next; currentthinker != &thinkercap; currentthinker = currentthinker->next)
	{
		// Only Objects
		if ((currentthinker->function.acp1 != (actionf_p1)P_MobjThinker))
			continue;

		// Convert to object
		mo = (mobj_t*)currentthinker;
		
		// Which type?
		if (a_Type == PMRT_TARGET)
			ChangePtr = &mo->target;
		else if (a_Type == PMRT_TRACER)
			ChangePtr = &mo->tracer;

		// Found it here?
		if (*ChangePtr == a_SourceRef)
		{
			fprintf(stderr, "Object %p,%i (%s/%s) [%s:%i] refs %p,%i (%s/%s) [%s:%i] by %i\n",
					mo,
					mo->RefCount[a_Type],
					(mo->type >= NUMMOBJTYPES ? "Free" : mobjinfo[mo->type]->RClassName),
					(mo->type >= NUMMOBJTYPES ? mobjinfo[mo->RemType]->RClassName : "Active"),
					mo->RefFile[a_Type], mo->RefLine[a_Type],
					a_SourceRef,
					a_SourceRef->RefCount[a_Type],
					(a_SourceRef->type >= NUMMOBJTYPES ? "Free" : mobjinfo[a_SourceRef->type]->RClassName),
					(a_SourceRef->type >= NUMMOBJTYPES ? mobjinfo[a_SourceRef->RemType]->RClassName : "Active"),
					a_SourceRef->RefFile[a_Type], a_SourceRef->RefLine[a_Type],
					a_Type
				);
		}
	}
#endif
}

/* P_ClearMobjRefs() -- Clear references from object */
void P_ClearMobjRefs(mobj_t* const a_Mo)
{
	size_t i, j;
	
	/* Check */
	if (!a_Mo)
		return;
	
	/* Clear the list away */
	for (i = 0; i < NUMPMOBJREFTYPES; i++)
		for (j = 0; j < a_Mo->RefListSz[i]; j++)
			if (a_Mo->RefList[i][j])
				P_RefMobj(i, a_Mo->RefList[i][j], NULL);
}

/* P_SetMobjToCrash() -- Set mobj to crash */
void P_SetMobjToCrash(mobj_t* const a_Mo)
{
	/* Check */
	if (!a_Mo)
		return;
	
	/* Make these things invalid */
	a_Mo->x = a_Mo->y = a_Mo->z = 32765 << FRACBITS;
	a_Mo->sprite = g_NumExSprites;
	a_Mo->frame = ~a_Mo->frame;
	a_Mo->skin = ~a_Mo->skin;
	a_Mo->floorz = a_Mo->ceilingz = a_Mo->radius = a_Mo->height = 32765 << FRACBITS;
	a_Mo->momx = a_Mo->momy = a_Mo->momz = 32765 << FRACBITS;
	a_Mo->type = -1;
	a_Mo->tics = 0;
	a_Mo->flags = a_Mo->eflags = a_Mo->flags2 = a_Mo->special1 = a_Mo->special2 = ~0;
	a_Mo->health = 1337;
	a_Mo->movedir = a_Mo->movecount = -8;
	a_Mo->reactiontime = 0;
	a_Mo->threshold = -14;
	a_Mo->lastlook = -17;
	
	/* Invalid pointers */
	a_Mo->snext = (void*)((uintptr_t)4);
	a_Mo->sprev = (void*)((uintptr_t)5);
	a_Mo->bnext = (void*)((uintptr_t)6);
	a_Mo->bprev = (void*)((uintptr_t)7);
	a_Mo->subsector = (void*)((uintptr_t)8);
	a_Mo->info = (void*)((uintptr_t)9);
	a_Mo->state = (void*)((uintptr_t)10);
	a_Mo->target = (void*)((uintptr_t)11);
	a_Mo->player = (void*)((uintptr_t)12);
	a_Mo->tracer = (void*)((uintptr_t)13);
	a_Mo->touching_sectorlist = (void*)((uintptr_t)14);
	a_Mo->spawnpoint = (void*)((uintptr_t)16);
}

/* P_MorphObjectClass() -- Morphs an object to another class */
void P_MorphObjectClass(mobj_t* const a_Mo, const PI_mobjid_t a_NewClass)
{
	PI_mobj_t* OldI, *NewI;
	fixed_t HealthP, RadiusP, HeightP;
	INFO_ObjectStateGroup_t OldGroup, NewGroup;
	PI_stateid_t NewState;
	size_t i;
	
	/* Check */
	if (!a_Mo || a_NewClass < 0 || a_NewClass >= NUMMOBJTYPES)
		return;
	
	/* Obtain old and new infos */
	OldI = a_Mo->info;
	NewI = mobjinfo[a_NewClass];
	
	/* Obtain stat differentials */
	HealthP = FixedDiv(a_Mo->health << FRACBITS, OldI->spawnhealth << FRACBITS);
	RadiusP = FixedDiv(a_Mo->radius, OldI->radius);
	HeightP = FixedDiv(a_Mo->height, __REMOOD_GETHEIGHT(OldI));
	
	/* Remove from body queue */
	P_RemoveFromBodyQueue(a_Mo);
	
	/* Remove position */
	P_UnsetThingPosition(a_Mo);
	
	/* Change object info now */
	a_Mo->type = a_NewClass;
	a_Mo->health = FixedMul(NewI->spawnhealth << FRACBITS, HealthP) >> FRACBITS;
	a_Mo->radius = FixedMul(NewI->radius, RadiusP);
	a_Mo->height = FixedMul(__REMOOD_GETHEIGHT(NewI), HeightP);
	a_Mo->info = NewI;
	
	// Setup Flags
	a_Mo->flags = NewI->flags;
	a_Mo->flags2 = NewI->flags2;
	for (i = 0; i < NUMINFORXFIELDS; i++)
		a_Mo->RXFlags[i] = NewI->RXFlags[i];
	
#if 1
	/* Go to specific state */
	// Dead
	if (a_Mo->health <= 0)
	{
		// Death state available
		if (NewI->deathstate || NewI->xdeathstate)
		{
			// Choose death state
			if (NewI->deathstate)
				NewState = NewI->deathstate;
			else
				NewState = NewI->xdeathstate;
		}
		
		// None available
		else
		{
			// Respawn
			a_Mo->health = NewI->spawnhealth;
			NewState = NewI->spawnstate;
		}
	}
	
	// Alive to begin with
	else
		NewState = NewI->spawnstate;
	
#else
	/* Determine state to change to */
	OldGroup = (a_Mo->state->Marker & 0xFFFF0000U) >> 16U;
	
	// Does the state not exist in the new group
	if (!NewI->RefStates[OldGroup])
	{
#define __REMOOD_STATECHECKS(x,y) if (OldGroup == (x) && NewI->RefStates[(x)]) NewGroup = (y)
		// Which 
		__REMOOD_STATECHECKS(IOSG_CRASH, IOSG_DEATH);
		else __REMOOD_STATECHECKS(IOSG_DEATH, IOSG_GIB);
		else __REMOOD_STATECHECKS(IOSG_GIB, IOSG_DEATH);
		else __REMOOD_STATECHECKS(IOSG_PLAYERRUN, IOSG_ACTIVE);
		else __REMOOD_STATECHECKS(IOSG_PLAYERMELEE, IOSG_MELEEATTACK);
		else __REMOOD_STATECHECKS(IOSG_PLAYERRANGED, IOSG_RANGEDATTACK);
		else __REMOOD_STATECHECKS(IOSG_RANGEDATTACK, IOSG_MELEEATTACK);
		else __REMOOD_STATECHECKS(IOSG_MELEEATTACK, IOSG_RANGEDATTACK);
		else NewGroup = IOSG_SPAWN;
#undef __REMOOD_STATECHECKS
	}
	else
		// Use existing group
		NewGroup = OldGroup;
	
	// Choose state based on group
	if (NewGroup == IOSG_SPAWN) NewState = NewI->spawnstate;
	else if (NewGroup == IOSG_ACTIVE) NewState = NewI->seestate;
	else if (NewGroup == IOSG_PAIN) NewState = NewI->painstate;
	else if (NewGroup == IOSG_MELEEATTACK) NewState = NewI->meleestate;
	else if (NewGroup == IOSG_RANGEDATTACK) NewState = NewI->missilestate;
	else if (NewGroup == IOSG_CRASH) NewState = NewI->crashstate;
	else if (NewGroup == IOSG_DEATH) NewState = NewI->deathstate;
	else if (NewGroup == IOSG_GIB) NewState = NewI->xdeathstate;
	else if (NewGroup == IOSG_RAISE) NewState = NewI->raisestate;
	else if (NewGroup == IOSG_PLAYERRUN) NewState = NewI->RPlayerRunState;
	else if (NewGroup == IOSG_PLAYERMELEE) NewState = NewI->RPlayerMeleeAttackState;
	else if (NewGroup == IOSG_PLAYERRANGED) NewState = NewI->RPlayerRangedAttackState;
	else if (NewGroup == IOSG_VILEHEAL) NewState = NewI->RVileHealState;
	else if (NewGroup == IOSG_LESSBLOODA) NewState = NewI->RLessBlood[0];
	else if (NewGroup == IOSG_LESSBLOODB) NewState = NewI->RLessBlood[1];
	else if (NewGroup == IOSG_BRAINEXPLODE) NewState = NewI->RBrainExplodeState;
	else if (NewGroup == IOSG_MELEEPUFF) NewState = NewI->RMeleePuffState;
	else NewState = 0;
#endif
	
	// Setup player health
	if (a_Mo->player)
		a_Mo->player->health = a_Mo->health;
	
	// Set state to that
	P_SetMobjState(a_Mo, NewState);
	
	/* Set position */
	P_SetThingPosition(a_Mo);
}

/* P_MobjIsPlayer() -- Object is player? */
bool_t P_MobjIsPlayer(mobj_t* const a_Mo)
{
	/* Check */
	if (!a_Mo)
		return false;
	
	/* Is a player? */
	if (a_Mo->player && (a_Mo->RXFlags[0] & MFREXA_ISPLAYEROBJECT))
		return true;
	
	/* Not one */
	return false;
}

/* P_MobjOnSameFamily() -- Determines whether two objects are the same family */
bool_t P_MobjOnSameFamily(mobj_t* const a_ThisMo, mobj_t* const a_OtherMo)
{
	bool_t IsThisPlayer, IsOtherPlayer;
	
	/* Check */
	if (!a_ThisMo || !a_OtherMo)
		return false;
	
	/* Self */
	if (a_ThisMo == a_OtherMo)
		return true;
	
	/* Determine if this is a standard player or a monster player */
	IsThisPlayer = IsOtherPlayer = false;
	if ((a_ThisMo->RXFlags[0] & MFREXA_ISPLAYEROBJECT))
		IsThisPlayer = true;
	if ((a_OtherMo->RXFlags[0] & MFREXA_ISPLAYEROBJECT))
		IsOtherPlayer = true;
	
	/* Players are never on the same family */
	if (IsThisPlayer || IsOtherPlayer)
		return false;
		
	/* When infighting, never on same family */
	if (P_XGSVal(PGS_FUNINFIGHTING))
		return false;
	
	/* Not on same team? */
	if (!P_MobjOnSameTeam(a_ThisMo, a_OtherMo))
		return false;
	
	/* Thing Type Match */
	if (a_ThisMo->type == a_OtherMo->type)
		return true;
		
	/* Base Family Matching */
	if (a_ThisMo->info->RBaseFamily || a_OtherMo->info->RBaseFamily)
	{
		// This Family vs Other Type
		if (a_ThisMo->info->RBaseFamily &&
			a_ThisMo->info->RBaseFamily == a_OtherMo->type)
			return true;
		
		// Other Family vs This Type
		if (a_OtherMo->info->RBaseFamily &&
			a_OtherMo->info->RBaseFamily == a_ThisMo->type)
			return true;
		
		// Same Family
		if (a_ThisMo->info->RBaseFamily == a_OtherMo->info->RBaseFamily)
			return true;
	}
	
	/* Un-Handled */
	return false;
}

/* P_MobjDamageTeam() -- Determines whether two objects will hurt each other */
bool_t P_MobjDamageTeam(mobj_t* const a_ThisMo, mobj_t* const a_OtherMo, mobj_t* const a_Inflictor)
{
	bool_t IsThisPlayer, IsOtherPlayer;
	
	/* Check */
	if (!a_ThisMo || !a_OtherMo)
		return true;
	
	/* Always hurt self */
	if (a_ThisMo == a_OtherMo)
		return true;
	
	/* Determine if this is a standard player or a monster player */
	IsThisPlayer = IsOtherPlayer = false;
	if (P_MobjIsPlayer(a_ThisMo))
		IsThisPlayer = true;
	if (P_MobjIsPlayer(a_OtherMo))
		IsOtherPlayer = true;
	
	/* Team Play Enabled */
	if (P_GMIsTeam())
	{
		// Team Damage is On -- Always do damage
		if (P_XGSVal(PGS_GAMETEAMDAMAGE))
			return true;
		
		// Off, check for differing team
		else
		{
			// Don't hurt same team
			if (P_MobjOnSameTeam(a_ThisMo, a_OtherMo))
				return false;
		}
	}
	
	/* Team Play Disabled */
	else
	{
		// Involving only players
		if (IsThisPlayer && IsOtherPlayer)
		{
			// Cooperative
			if (!P_GMIsDM())
			{
				// If team damage is on, hurt
				if (P_XGSVal(PGS_GAMETEAMDAMAGE))
					return true;
				
				// Otherwise, don't hurt
				else
					return false;
			}
			
			// Deathmatch
			else
			{
				// Always hurt
				return true;
			}
		}
		
		// Inter-Monster involvement
		else
		{
			// Inflictor is a missile
			if (a_Inflictor && a_Inflictor->flags & MF_MISSILE)
				// Don't hurt same family
				return !P_MobjOnSameFamily(a_ThisMo, a_OtherMo);
			
			// Otherwise, always hurt
			else
				return true;
		}
	}
	
	/* Un-Handled */
	return true;
}

/* P_GetMobjTeam() -- Get object's team */
int32_t P_GetMobjTeam(mobj_t* const a_Mo)
{
	int32_t RetVal;
	bool_t VTeam;
	
	/* Check */
	if (!a_Mo)
		return -1;
	
	/* Virtual Teams? */
	VTeam = P_XGSVal(PGS_CONEWGAMEMODES);
	
	/* Player? */
	if (P_MobjIsPlayer(a_Mo))
	{
		if (VTeam)
		{
			// Teams
			if (P_GMIsTeam())
				RetVal = a_Mo->player->VTeamColor;
			
			// No Teams
			else
				return -1;
		}
		
		else
			if (P_XGSVal(PGS_GAMETEAMPLAY) == 2)
				RetVal = a_Mo->player->skin;
			else
				RetVal = a_Mo->player->skincolor;
	}
	
	/* Monster? */
	else
	{
		RetVal =  a_Mo->SkinTeamColor - 1;
	}
	
	/* Cap team to team limit */
	if (VTeam)
		;//RetVal = RetVal % P_XGSVal();
	
	/* Return */
	return RetVal;
}

/* P_MobjOnSameTeam() -- Determines whether two objects are on the same team */
bool_t P_MobjOnSameTeam(mobj_t* const a_ThisMo, mobj_t* const a_OtherMo)
{
	bool_t IsThisPlayer, IsOtherPlayer;
	int32_t TeamA, TeamB;
	
	/* Check */
	if (!a_ThisMo || !a_OtherMo)
		return false;
		
	/* Self object is always on the same team */
	if (a_ThisMo == a_OtherMo)
		return true;
	
	/* Determine if this is a standard player or a monster player */
	IsThisPlayer = IsOtherPlayer = false;
	if (P_MobjIsPlayer(a_ThisMo))
		IsThisPlayer = true;
	if (P_MobjIsPlayer(a_OtherMo))
		IsOtherPlayer = true;
	
	/* Get team numbers */
	TeamA = P_GetMobjTeam(a_ThisMo);
	TeamB = P_GetMobjTeam(a_OtherMo);
	
	/* Cooperative Players */
	if (!P_GMIsDM() && IsThisPlayer && IsOtherPlayer)
		return true;
	
	/* Deathmatch Players */
	if (P_GMIsDM() && IsThisPlayer && IsOtherPlayer)
	{
		// Team play?
		if (P_GMIsTeam())
		{
			// Teamless for some reason?
			if (TeamA <= -1 || TeamB <= -1)
				return false;
			
			// On same team?
			else if (TeamA == TeamB)
				return true;
		}
		
		// Free-For-All
		else
			return false;
	}
	
	/* Player and friendly monster */
	if ((IsThisPlayer && (a_OtherMo->flags2 & MF2_FRIENDLY)) ||
		((IsOtherPlayer && (a_ThisMo->flags2 & MF2_FRIENDLY))))
		return true;
	
	/* Monster Teams */
	// Team Play Enabled
	if (P_GMIsTeam())
	{
		// Player and monster on the same colored team?
		if (((IsThisPlayer && !IsOtherPlayer) || (IsOtherPlayer && !IsThisPlayer)) && TeamA == TeamB)
			return true;
	
		// Monsters on the same skin team
		if (a_ThisMo->SkinTeamColor > 0 && a_OtherMo->SkinTeamColor > 0)
		{
			// Same team?
			if (TeamA == TeamB)
				return true;
			
			// Different Team
			else
				return false;
		}
	}
	
	// Team Play Disabled
	else
	{
		// Cooperative
		if (!P_GMIsDM())
		{
			// Player and teamed monster
			if ((IsThisPlayer && !IsOtherPlayer && a_OtherMo->SkinTeamColor > 0) ||
				(!IsThisPlayer && IsOtherPlayer && a_ThisMo->SkinTeamColor > 0))
				return true;
			
			// Other teamed monsters
			if (!IsThisPlayer && !IsOtherPlayer && a_ThisMo->SkinTeamColor && a_OtherMo->SkinTeamColor)
				return true;
		}
		
		// Deathmatch
		else
		{
		}
	}
	
	/* Monsters on a team vs teamless monsters */
	if (a_ThisMo->SkinTeamColor > 0 && !a_OtherMo->SkinTeamColor)
		return false;
	if (!a_ThisMo->SkinTeamColor && a_OtherMo->SkinTeamColor > 0)
		return false;
	
	/* Friendly monsters and other friendly monsters */
	if ((!IsThisPlayer && !IsOtherPlayer) &&
		(((a_ThisMo->flags2 & MF2_FRIENDLY) && (a_OtherMo->flags2 & MF2_FRIENDLY))))
		return true;
	
	/* Friendly monster and non-friendly monster */
	if ((!IsThisPlayer && !IsOtherPlayer) &&
		(((a_ThisMo->flags2 & MF2_FRIENDLY) && !(a_OtherMo->flags2 & MF2_FRIENDLY)) ||
		(!(a_ThisMo->flags2 & MF2_FRIENDLY) && (a_OtherMo->flags2 & MF2_FRIENDLY))))
		return false;
	
	/* Monsters and other monsters */
	if (!IsThisPlayer && !IsOtherPlayer)
		return true;
	
	/* Not on same team */
	return false;
}

/* P_ControlNewMonster() -- Control new monster, as player */
void P_ControlNewMonster(struct player_s* const a_Player)
{
#define MAXCTRLCANDIDATES 16
	thinker_t* oldthinker;
	thinker_t* currentthinker;
	mobj_t* mo;
	mobj_t* Cands[MAXCTRLCANDIDATES];
	size_t i, c;
	
	/* Check */
	if (!a_Player)
		return;
		
	/* Clear Candidate List */
	c = 0;
	memset(Cands, 0, sizeof(Cands));
	
	/* Go through all thinkers */
	// Figure out current thinker
	oldthinker = (thinker_t*)a_Player->mo;
	
	if (!oldthinker)
	{
		oldthinker = &thinkercap;
		currentthinker = thinkercap.next;
	}
	else
		currentthinker = oldthinker->next;
	
	// Look through thinkers
	for (; currentthinker != oldthinker; currentthinker = currentthinker->next)
	{
		// Not a mobj?
		if (!((currentthinker->function.acp1 == (actionf_p1) P_MobjThinker)))
			continue;
		
		// Make mo
		mo = (mobj_t*)currentthinker;
	
		// Ourself?
		if (a_Player->mo == mo)
			continue;
		
		// Controlled by a player?
		if (mo->player)
			continue;
		
		// Not a monster?
		if (!(mo->RXFlags[0] & MFREXA_ISMONSTER))
			continue;
		
		// Dead?
		if (mo->health <= 0 || (mo->flags & MF_CORPSE))
			continue;
		
		// Candidate?
		if (!c)
			Cands[c++] = mo;
		else
		{
			if (P_Random() & 1)
				Cands[c++] = mo;
		}
		
		if (c >= MAXCTRLCANDIDATES)
			break;
	}
	
	/* Choose a random candidate */
	if (c > 0)
	{
		// Random
		mo = Cands[P_Random() % c];
		
		// Take posession of this monster
		if (a_Player->mo)
			a_Player->mo->player = NULL;	// Old body owns no player now
		a_Player->mo = mo;				// Use this new body
		mo->player = a_Player;			// Set as this body
		a_Player->playerstate = PST_LIVE;
	
		// Setup player health
		a_Player->health = mo->health;
	
		// Set local angle
		for (i = 0; i < MAXSPLITSCREEN; i++)
			if (g_Splits[i].Active)
				if (g_Splits[i].Console == a_Player - players)
				{
					localangle[i] = mo->angle;
					break;
				}
		
		// Clear weapon shot
		mo->RXShotWithWeapon = -1;
		
		// Took control of something
		return;
	}
	
	/* If this point was reach, no object was associated */
	// Which means everything else is dead, so if this is the case
	// Ressurect self
	if (a_Player->mo)
	{
		P_NightmareRespawn(a_Player->mo, true);
		
		a_Player->playerstate = PST_LIVE;
	}
	
	// But if that fails? Create a random monster
	else
	{
	}
#undef MAXCTRLCANDIDATES
}

