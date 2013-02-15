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
//      Enemy thinking, AI.
//      Action Pointer Functions
//      that are associated with states/frames.

#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "r_main.h"
#include "r_state.h"
#include "s_sound.h"
#include "m_random.h"
#include "p_demcmp.h"
#include "p_info.h"

void FastMonster_OnChange(void);

//
// P_NewChaseDir related LUT.
//
static dirtype_t opposite[] =
{
	DI_WEST, DI_SOUTHWEST, DI_SOUTH, DI_SOUTHEAST,
	DI_EAST, DI_NORTHEAST, DI_NORTH, DI_NORTHWEST, DI_NODIR
};

static dirtype_t diags[] =
{
	DI_NORTHWEST, DI_NORTHEAST, DI_SOUTHWEST, DI_SOUTHEAST
};

void A_Fall(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV);

void FastMonster_OnChange(void)
{
}

//
// ENEMY THINKING
// Enemies are allways spawned
// with targetplayer = -1, threshold = 0
// Most monsters are spawned unaware of all players,
// but some can be made preaware
//

//
// Called by P_NoiseAlert.
// Recursively traverse adjacent sectors,
// sound blocking lines cut off traversal.
//

static mobj_t* soundtarget;

static sector_t** l_SoundSecs;
static size_t l_NumSoundSecs;

/* P_ClearRecursiveSound() -- Clears recursive sounds */
void P_ClearRecursiveSound(void)
{
	if (l_SoundSecs)
		Z_Free(l_SoundSecs);
	l_SoundSecs = NULL;
	l_NumSoundSecs = 0;
}

/* P_RemoveRecursiveSound() -- Remove recursive sounds from sectors */
void P_RemoveRecursiveSound(mobj_t* const a_Mo)
{
	size_t i;
	
	/* Look through sound sectors */
	for (i = 0; i < l_NumSoundSecs; i++)
		if (l_SoundSecs[i])
			if (l_SoundSecs[i]->soundtarget == a_Mo)
				l_SoundSecs[i]->soundtarget = NULL;
}

static void P_RecursiveSound(sector_t* sec, int soundblocks)
{
	int i;
	line_t* check;
	sector_t* other;
	
	// wake up all monsters in this sector
	if (sec->validcount == validcount && sec->soundtraversed <= soundblocks + 1)
	{
		return;					// already flooded
	}
	
	sec->validcount = validcount;
	sec->soundtraversed = soundblocks + 1;
	sec->soundtarget = soundtarget;

	// GhostlyDeath <April 21, 2012> -- Add to referenced sectors	
	if (!sec->SoundSecRef)
	{
		Z_ResizeArray((void**)&l_SoundSecs, sizeof(*l_SoundSecs), l_NumSoundSecs, l_NumSoundSecs + 1);
		sec->SoundSecRef = ++l_NumSoundSecs;
		l_SoundSecs[sec->SoundSecRef - 1] = sec;
	}
	
	for (i = 0; i < sec->linecount; i++)
	{
		check = sec->lines[i];
		if (!(check->flags & ML_TWOSIDED))
			continue;
			
		P_LineOpening(check);
		
		if (openrange <= 0)
			continue;			// closed door
			
		if (sides[check->sidenum[0]].sector == sec)
			other = sides[check->sidenum[1]].sector;
		else
			other = sides[check->sidenum[0]].sector;
			
		if (check->flags & ML_SOUNDBLOCK)
		{
			if (!soundblocks)
				P_RecursiveSound(other, 1);
		}
		else
			P_RecursiveSound(other, soundblocks);
	}
}

//
// P_NoiseAlert
// If a monster yells at a player,
// it will alert other monsters to the player.
//
void P_NoiseAlert(mobj_t* target, mobj_t* emmiter)
{
	soundtarget = target;
	validcount++;
	P_RecursiveSound(emmiter->subsector->sector, 0);
}

//
// P_CheckMeleeRange
//
static bool_t P_CheckMeleeRange(mobj_t* actor)
{
	mobj_t* pl;
	fixed_t dist;
	
	if (!actor->target)
		return false;
		
	pl = actor->target;
	dist = P_AproxDistance(pl->x - actor->x, pl->y - actor->y);
	
	if (dist >= MELEERANGE - 20 * FRACUNIT + pl->info->radius)
		return false;
		
	//added:19-03-98: check height now, so that damn imps cant attack
	//                you if you stand on a higher ledge.
	if (P_XGSVal(PGS_COLIMITMONSTERZMATTACK) && ((pl->z > actor->z + actor->height) || (actor->z > pl->z + pl->height)))
		return false;
		
	if (!P_CheckSight(actor, actor->target))
		return false;
		
	return true;
}

//
// P_CheckMissileRange
//
static bool_t P_CheckMissileRange(mobj_t* actor)
{
	fixed_t dist;
	
	// GhostlyDeath <April 29, 2012> -- Players always missile
	if (actor->player)
		return true;
	
	if (!P_CheckSight(actor, actor->target))
		return false;
		
	if (actor->flags & MF_JUSTHIT)
	{
		// the target just hit the enemy,
		// so fight back!
		actor->flags &= ~MF_JUSTHIT;
		return true;
	}
	
	if (actor->reactiontime)
		return false;			// do not attack yet
		
	// OPTIMIZE: get this from a global checksight
	dist = P_AproxDistance(actor->x - actor->target->x, actor->y - actor->target->y) - 64 * FRACUNIT;
	
	if (!actor->info->meleestate)
		dist -= 128 * FRACUNIT;	// no melee attack, so fire more
		
	dist >>= 16;
	
	// GhostlyDeath <March 6, 2012> -- Distance caps
	// Distance less than said range?
	if (actor->info->RMissileDist[0])
		if (dist < actor->info->RMissileDist[0])
			return false;
	
	// Distance more than said range?
	if (actor->info->RMissileDist[1])
		if (dist > actor->info->RMissileDist[1])
			return false;
	
	// GhostlyDeath <March 6, 2012> -- Cut missile range in half?
	if (actor->RXFlags[0] & MFREXA_HALFMISSILERANGE)
		dist >>= 1;
	
	if (dist > 200)
		dist = 200;
		
	// GhostlyDeath <March 6, 2012> -- Cap missile distance?
	if (actor->info->RCapMissileDist)
		if (dist > actor->info->RCapMissileDist)
			dist = actor->info->RCapMissileDist;
		
	if (P_Random() < dist)
		return false;
		
	return true;
}

//
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
//
static const fixed_t xspeed[8] = { FRACUNIT, 47000, 0, -47000, -FRACUNIT, -47000, 0, 47000 };
static const fixed_t yspeed[8] = { 0, 47000, FRACUNIT, 47000, 0, -47000, -FRACUNIT, -47000 };

static bool_t P_Move(mobj_t* actor)
{
	fixed_t tryx;
	fixed_t tryy;
	
	line_t* ld;
	
	bool_t good;
	
	if (actor->movedir == DI_NODIR)
		return false;
		
#ifdef PARANOIA
	if ((unsigned)actor->movedir >= 8)
		I_Error("Weird actor->movedir!");
#endif
		
	tryx = actor->x + __REMOOD_GETSPEEDMO(actor) * xspeed[actor->movedir];
	tryy = actor->y + __REMOOD_GETSPEEDMO(actor) * yspeed[actor->movedir];
	
	if (!P_TryMove(actor, tryx, tryy, false, NULL, NULL))
	{
		// open any specials
		if (actor->flags & MF_FLOAT && floatok)
		{
			// must adjust height
			if (actor->z < tmfloorz)
				actor->z += FLOATSPEED;
			else
				actor->z -= FLOATSPEED;
				
			actor->flags |= MF_INFLOAT;
			return true;
		}
		
		if (!numspechit)
			return false;
		
		// GhostlyDeath <April 29, 2012> -- Monster control
		if (!actor->player)
			actor->movedir = DI_NODIR;
		
		good = false;
		while ((numspechit--) > 0)
		{
			ld = lines + spechit[numspechit];
			// if the special is not a door
			// that can be opened,
			// return false
			if (P_UseSpecialLine(actor, ld, 0))
				good = true;
		}
		return good;
	}
	else
	{
		actor->flags &= ~MF_INFLOAT;
	}
	
	if (!(actor->flags & MF_FLOAT))
	{
		if (actor->z > actor->floorz)
			P_HitFloor(actor);
		actor->z = actor->floorz;
	}
	return true;
}

//
// TryWalk
// Attempts to move actor on
// in its current (ob->moveangle) direction.
// If blocked by either a wall or an actor
// returns FALSE
// If move is either clear or blocked only by a door,
// returns TRUE and sets...
// If a door is in the way,
// an OpenDoor call is made to start it opening.
//
static bool_t P_TryWalk(mobj_t* actor)
{
	if (!P_Move(actor))
	{
		return false;
	}
	
	if (!actor->player)
		actor->movecount = P_Random() & 15;
	return true;
}

static void P_NewChaseDir(mobj_t* actor)
{
	fixed_t deltax;
	fixed_t deltay;
	
	dirtype_t d[3];
	
	int tdir;
	dirtype_t olddir;
	
	dirtype_t turnaround;
	
	olddir = actor->movedir;
	turnaround = opposite[olddir];
	
	if (actor->player)
		return;
	
	deltax = actor->target->x - actor->x;
	deltay = actor->target->y - actor->y;
	
	if (deltax > 10 * FRACUNIT)
		d[1] = DI_EAST;
	else if (deltax < -10 * FRACUNIT)
		d[1] = DI_WEST;
	else
		d[1] = DI_NODIR;
		
	if (deltay < -10 * FRACUNIT)
		d[2] = DI_SOUTH;
	else if (deltay > 10 * FRACUNIT)
		d[2] = DI_NORTH;
	else
		d[2] = DI_NODIR;
		
	// try direct route
	if (d[1] != DI_NODIR && d[2] != DI_NODIR)
	{
		actor->movedir = diags[((deltay < 0) << 1) + (deltax > 0)];
		if (actor->movedir != turnaround && P_TryWalk(actor))
			return;
	}
	
	// try other directions
	if (P_Random() > 200 || abs(deltay) > abs(deltax))
	{
		tdir = d[1];
		d[1] = d[2];
		d[2] = tdir;
	}
	
	if (d[1] == turnaround)
		d[1] = DI_NODIR;
	if (d[2] == turnaround)
		d[2] = DI_NODIR;
		
	if (d[1] != DI_NODIR)
	{
		actor->movedir = d[1];
		if (P_TryWalk(actor))
		{
			// either moved forward or attacked
			return;
		}
	}
	
	if (d[2] != DI_NODIR)
	{
		actor->movedir = d[2];
		
		if (P_TryWalk(actor))
			return;
	}
	
	// there is no direct path to the player,
	// so pick another direction.
	if (olddir != DI_NODIR)
	{
		actor->movedir = olddir;
		
		if (P_TryWalk(actor))
			return;
	}
	
	// randomly determine direction of search
	if (P_Random() & 1)
	{
		for (tdir = DI_EAST; tdir <= DI_SOUTHEAST; tdir++)
		{
			if (tdir != turnaround)
			{
				actor->movedir = tdir;
				
				if (P_TryWalk(actor))
					return;
			}
		}
	}
	else
	{
		for (tdir = DI_SOUTHEAST; tdir >= DI_EAST; tdir--)
		{
			if (tdir != turnaround)
			{
				actor->movedir = tdir;
				
				if (P_TryWalk(actor))
					return;
			}
		}
	}
	
	if (turnaround != DI_NODIR)
	{
		actor->movedir = turnaround;
		if (P_TryWalk(actor))
			return;
	}
	
	actor->movedir = DI_NODIR;	// can not move
}

//
// P_LookForPlayers
// If allaround is false, only look 180 degrees in front.
// Returns true if a player is targeted.
//
static bool_t P_LookForPlayers(mobj_t* actor, bool_t allaround)
{
	int c;
	int stop;
	player_t* player;
	sector_t* sector;
	angle_t an;
	fixed_t dist, BestDist;
	mobj_t* mo;
	mobj_t* BestMo;
	thinker_t* currentthinker;
	bool_t LoopOK;
	int32_t MaxPlayers;
	
	/* Demo Compatibility Breaking, but no real break */
	// The code here in 1.9 and such is incapable of handling situations where
	// nobody is actually playing the game. So say that a player wants to
	// record or experience a 1.09 game. This will allow the old logic to take
	// effect.
	if (P_XGSVal(PGS_COOLDLASTLOOKLOGIC))
	{
		// Count playing players
		c = 0;
		for (stop = 0; stop < MAXPLAYERS; stop++)
			if (playeringame[stop])
				c++;
		
		// If no players inside, return false
		if (!c)
			return false;
	}
	
	/* Get Max Players */
	// Demo Compat
	MaxPlayers = P_XGSVal(PGS_COLASTLOOKMAXPLAYERS);
	
	/* Look for players */
	if (!P_XGSVal(PGS_FUNNOTARGETPLAYER))
	{
		sector = actor->subsector->sector;
		
		// BP: first time init, this allow minimum lastlook changes
		if (actor->lastlook < 0 && P_XGSVal(PGS_CORANDOMLASTLOOK))
			actor->lastlook = P_Random() % MaxPlayers;
	
		c = 0;
		stop = (actor->lastlook - 1) & (MaxPlayers - 1);
	
		for (LoopOK = false;; actor->lastlook = (actor->lastlook + 1) & (MaxPlayers - 1))
		{
			// GhostlyDeath <June 21, 2012> -- Demo Compat
			if (P_XGSVal(PGS_COOLDLASTLOOKLOGIC))
			{
				// Player Missing
				if (!playeringame[actor->lastlook])
					continue;
				
				// Increment and possibly stop
				if (c++ == 2 || actor->lastlook == stop)
					break;
			}
			
			// Legacy (and thus ReMooD) changed the order around, this one
			// stops sooner rather than later
			else
			{
				// done looking
				if (actor->lastlook == stop)
					break;
	
				if (!playeringame[actor->lastlook])
					continue;
	
				if (c++ == 2)
					break;
			}
	
			player = &players[actor->lastlook];
			
			// Player is the same as the monster?
			if (actor == player->mo)
				continue;
			
			// Player and monster on the same team?
			if (!P_XGSVal(PGS_CODISABLETEAMPLAY))
				if (P_XGSVal(PGS_COENABLETEAMMONSTERS))
					if (P_MobjOnSameTeam(actor, player->mo))
						continue;
	
			if (player->health <= 0)
				continue;			// dead
			
			// No object?
			if (!player->mo)
				continue;
	
			if (!P_CheckSight(actor, player->mo))
				continue;			// out of sight
			
			if (!allaround)
			{
				an = R_PointToAngle2(actor->x, actor->y, player->mo->x, player->mo->y) - actor->angle;
	
				if (an > ANG90 && an < ANG270)
				{
					dist = P_AproxDistance(player->mo->x - actor->x, player->mo->y - actor->y);
					// if real close, react anyway
					if (dist > MELEERANGE)
						continue;	// behind back
				}
			}
	
			P_RefMobj(PMRT_TARGET, actor, player->mo);
			LoopOK = true;
			break;
		}
		
		// Loop was OK?
		if (LoopOK)
			return true;
	}
	
	// GhostlyDeath <June 17, 2012> -- Old Demo and Nothing Found?
	if (!P_XGSVal(PGS_COMONSTERLOOKFORMONSTER))
		return false;	// Nothing found
	
	/* Then look for other monsters */
	// Find closest target
	BestMo = NULL;
	BestDist = 20000 << FRACBITS;
	
	// Look through thinkers
	for (currentthinker = thinkercap.next; currentthinker != &thinkercap; currentthinker = currentthinker->next)
	{
		// Not a mobj?
		if (!((currentthinker->function.acp1 == (actionf_p1) P_MobjThinker)))
			continue;
		
		// Make mo
		mo = (mobj_t*)currentthinker;
	
		// Ourself?
		if (actor == mo)
			continue;
		
		// On the same team?
		if (!(P_XGSVal(PGS_FUNMONSTERFFA)))
			if (P_MobjOnSameTeam(actor, mo))
				continue;
		
		// A player? and cannot target them?
		if (P_XGSVal(PGS_FUNNOTARGETPLAYER) && P_MobjIsPlayer(mo))
			continue;
		
		// Not Shootable?
		if (!(mo->flags & MF_SHOOTABLE))
			continue;
		
		// Dead?
		if (mo->health <= 0)
			continue;
		
		// Is it in view?
		if (!P_CheckSight(actor, mo))
			continue;
		
		// Look all around?
		if (!allaround)
		{
			an = R_PointToAngle2(actor->x, actor->y, mo->x, mo->y) - actor->angle;

			if (an > ANG90 && an < ANG270)
			{
				dist = P_AproxDistance(mo->x - actor->x, mo->y - actor->y);
				
				// if real close, react anyway
				if (dist > MELEERANGE)
					continue;	// behind back
			}
		}
		
		// Distance Check
		dist = P_AproxDistance(actor->x - mo->x, actor->y - mo->y);
		if (dist < BestDist)
		{
			BestDist = dist;
			BestMo = mo;
			
			// Distance is REALLY close? Then target that thing
			if (BestDist < (128 << FRACBITS))
				break;
		}
	}
	
	// Found best?
	if (BestMo)
	{
		// target it then!
		P_RefMobj(PMRT_TARGET, actor, BestMo);
		return true;
	}
	
	/* Never targetted anything */
	return false;
}

//
// ACTION ROUTINES
//

//
// A_Look
// Stay in state until a player is sighted.
//
void A_Look(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	mobj_t* targ;
	sfxid_t SoundID;
	
	// GhostlyDeath <April 29, 2012> -- If a player move to movement state
	if (P_XGSVal(PGS_MONENABLEPLAYASMONSTER))
		if (actor->player)
		{
			actor->threshold = 0;
			S_StartSound(&actor->NoiseThinker, S_SoundIDForName(actor->info->RSeeSound));
			P_SetMobjState(actor, actor->info->seestate);
			return;
		}
	
	actor->threshold = 0;		// any shot will wake up
	targ = actor->subsector->sector->soundtarget;
	
	// Target is a player?
	if (P_XGSVal(PGS_FUNNOTARGETPLAYER))
		if (targ && P_MobjIsPlayer(targ))
			targ = NULL;
	
	// GhostlyDeath <June 6, 2012> -- Target is on your team
	if (P_XGSVal(PGS_COENABLETEAMMONSTERS))
		if (P_GMIsTeam())
			if (targ && P_MobjOnSameTeam(targ, actor))
				targ = NULL;
	
	if (targ && (targ->flags & MF_SHOOTABLE))
	{
		P_RefMobj(PMRT_TARGET, actor, targ);
		
		if (actor->flags & MF_AMBUSH)
		{
			if (P_CheckSight(actor, actor->target))
				goto seeyou;
		}
		else
			goto seeyou;
	}
	
	if (!P_LookForPlayers(actor, false))
		return;
		
	// go into chase state
seeyou:
	
	SoundID = S_SoundIDForName(actor->info->RSeeSound);
	if (SoundID)
	{
		int sound;
		
		switch (SoundID)
		{
			case sfx_posit1:
			case sfx_posit2:
			case sfx_posit3:
				sound = sfx_posit1 + P_Random() % 3;
				break;
				
			case sfx_bgsit1:
			case sfx_bgsit2:
				sound = sfx_bgsit1 + P_Random() % 2;
				break;
				
			default:
				sound = SoundID;
				break;
		}
		
		if ((actor->RXFlags[0] & MFREXA_SOUNDEVERYWHERE) || (actor->flags2 & MF2_BOSS))
		{
			// full volume
			S_StartSound(NULL, sound);
		}
		else
			S_StartSound(&actor->NoiseThinker, sound);
			
	}
	
	P_SetMobjState(actor, actor->info->seestate);
}

void A_FaceTarget(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV);

//
// A_Chase
// Actor has a melee attack,
// so it tries to close as fast as possible
//
void A_Chase(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	int delta;
	G_Skill_t Skill;
	bool_t Controlled;
	
	// GhostlyDeath <June 23, 2012> -- Controlled monster?
	Controlled = false;
	if (P_XGSVal(PGS_MONENABLEPLAYASMONSTER) && actor->player)
		Controlled = true;
	
	// GhostlyDeath <June 17, 2012> -- Get Skill
	Skill = P_XGSVal(PGS_GAMESKILL);
	
	if (actor->reactiontime)
		actor->reactiontime--;
		
	// modify target threshold
	if (actor->threshold)
	{
		// No target or target is dead
		if (!P_XGSVal(PGS_HEREMONSTERTHRESH) && !P_MobjIsPlayer(actor) && (!actor->target || actor->target->health <= 0 || (actor->target->flags & MF_CORPSE)))
		{
			actor->threshold = 0;
		
			// GhostlyDeath <April 21, 2012> -- Deref here to remove reference
			if (P_XGSVal(PGS_COMONSTERDEADTARGET))
				P_RefMobj(PMRT_TARGET, actor, NULL);
		}
		
		// There is a target, it is alive
		// Or playing Heretic
		else
			actor->threshold--;
	}
	
	// turn towards movement direction if not there yet
	if (!Controlled)
		if (actor->movedir < 8)
		{
			actor->angle &= (7 << 29);
			delta = actor->angle - (actor->movedir << 29);
		
			if (delta > 0)
				actor->angle -= ANG90 / 2;
			else if (delta < 0)
				actor->angle += ANG90 / 2;
		}
	
	if (!Controlled)
		if (!actor->target || !(actor->target->flags & MF_SHOOTABLE))
		{
			// look for a new target
			if (P_LookForPlayers(actor, true))
				return;				// got a new target
			
			P_SetMobjState(actor, actor->info->spawnstate);
			return;
		}
	
	// do not attack twice in a row
	if (actor->flags & MF_JUSTATTACKED)
	{
		actor->flags &= ~MF_JUSTATTACKED;
		if (!P_XGSVal(PGS_MONFASTMONSTERS))
			P_NewChaseDir(actor);
		return;
	}
	
	if (!Controlled)
	{
		// check for melee attack
		if (actor->info->meleestate && P_CheckMeleeRange(actor))
		{
			if (actor->info->RAttackSound)
				S_StartSound(&actor->NoiseThinker, S_SoundIDForName(actor->info->RAttackSound));
			
			P_SetMobjState(actor, actor->info->meleestate);
			return;
		}
		
		// check for missile attack
		if (actor->info->missilestate)
		{
			if (!P_XGSVal(PGS_MONFASTMONSTERS) && actor->movecount)
				goto nomissile;
		
			if (!P_CheckMissileRange(actor))
				goto nomissile;
			
			P_SetMobjState(actor, actor->info->missilestate);
			actor->flags |= MF_JUSTATTACKED;
			return;
		}
	}
	
	// GhostlyDeath <April 29, 2012> -- Player controlled monster
	else if (Controlled)
	{
		// Attack down?
		if (actor->player->attackdown)
		{
			// Face target?
			A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
			
			// Melee State?
			if (actor->info->meleestate && P_CheckMeleeRange(actor))
			{
				if (actor->info->RAttackSound)
					S_StartSound(&actor->NoiseThinker, S_SoundIDForName(actor->info->RAttackSound));
			
				P_SetMobjState(actor, actor->info->meleestate);
			}
			
			// Missile State
			else if (actor->info->missilestate)
			{
				P_SetMobjState(actor, actor->info->missilestate);
				actor->flags |= MF_JUSTATTACKED;
			}
		}
	}
	
	// ?
nomissile:
	// possibly choose another target
	if (!Controlled)
		if (P_XGSVal(PGS_COMULTIPLAYER) && !actor->threshold && !P_CheckSight(actor, actor->target))
			if (P_LookForPlayers(actor, true))
				return;				// got a new target
	
	// chase towards player
	if (--actor->movecount < 0 || !P_Move(actor))
		P_NewChaseDir(actor);
	
	// make active sound
	if (actor->info->RActiveSound && P_Random() < 3)
		S_StartSound(&actor->NoiseThinker, S_SoundIDForName(actor->info->RActiveSound));
}

//
// A_FaceTarget
//
void A_FaceTarget(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	// GhostlyDeath <April 29, 2012> -- Player controlled monsters
	if (P_XGSVal(PGS_MONENABLEPLAYASMONSTER) && actor->player)
	{
		P_AimLineAttack(actor, actor->angle, MISSILERANGE, NULL);
		actor->target = linetarget;
	}
	
	// Normal Monsters
	else
	{
		if (!actor->target)
			return;
		
		actor->flags &= ~MF_AMBUSH;
	
		actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);
	
		if ((actor->target->flags & MF_SHADOW) || P_XGSVal(PGS_FUNMONSTERSMISSMORE))
			actor->angle += P_SignedRandom() << 21;
	}
}

//
// A_PosAttack
//
void A_PosAttack(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	int angle;
	int damage;
	int slope;
	
	PuffType = INFO_GetTypeByName("BulletPuff");
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	
	if (!actor->player && !actor->target)
		return;
	
	angle = actor->angle;
	slope = P_AimLineAttack(actor, angle, MISSILERANGE, NULL);
	
	S_StartSound(&actor->NoiseThinker, sfx_pistol);
	angle += P_SignedRandom() << 20;
	damage = ((P_Random() % 5) + 1) * 3;
	
	// GhostlyDeath <March 6, 2012> -- Obit check
	actor->RXAttackAttackType = PRXAT_RANGED;
	P_LineAttack(actor, angle, MISSILERANGE, slope, damage, NULL);
}

void A_SPosAttack(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	int i;
	int angle;
	int bangle;
	int damage;
	int slope;
	
	PuffType = INFO_GetTypeByName("BulletPuff");
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	
	if (!actor->player && !actor->target)
		return;
	
	S_StartSound(&actor->NoiseThinker, sfx_shotgn);
	bangle = actor->angle;
	slope = P_AimLineAttack(actor, bangle, MISSILERANGE, NULL);
	
	// GhostlyDeath <March 6, 2012> -- Obit check
	actor->RXAttackAttackType = PRXAT_RANGED;
	for (i = 0; i < 3; i++)
	{
		angle = (P_SignedRandom() << 20) + bangle;
		damage = ((P_Random() % 5) + 1) * 3;
		P_LineAttack(actor, angle, MISSILERANGE, slope, damage, NULL);
	}
}

void A_CPosAttack(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	int angle;
	int bangle;
	int damage;
	int slope;
	
	PuffType = INFO_GetTypeByName("BulletPuff");
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	
	if (!actor->player && !actor->target)
		return;
		
	S_StartSound(&actor->NoiseThinker, sfx_shotgn);
	
	bangle = actor->angle;
	slope = P_AimLineAttack(actor, bangle, MISSILERANGE, NULL);
	
	angle = (P_SignedRandom() << 20) + bangle;
	
	damage = ((P_Random() % 5) + 1) * 3;
	
	// GhostlyDeath <March 6, 2012> -- Obit check
	actor->RXAttackAttackType = PRXAT_RANGED;
	P_LineAttack(actor, angle, MISSILERANGE, slope, damage, NULL);
}

void A_CPosRefire(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	// keep firing unless target got out of sight
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	
	if (P_Random() < 40)
		return;
	
	if (actor->player)
	{
		if (!actor->player->attackdown)
			P_SetMobjState(actor, actor->info->seestate);
	}
	
	else
	{
		if (!actor->target || actor->target->health <= 0 || actor->target->flags & MF_CORPSE || !P_CheckSight(actor, actor->target))
			P_SetMobjState(actor, actor->info->seestate);
	}
}

void A_SpidRefire(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	// keep firing unless target got out of sight
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	
	if (P_Random() < 10)
		return;
		
	if (actor->player)
	{
		if (!actor->player->attackdown)
			P_SetMobjState(actor, actor->info->seestate);
	}
	
	else
	{
		if (!actor->target || actor->target->health <= 0 || actor->target->flags & MF_CORPSE || !P_CheckSight(actor, actor->target))
			P_SetMobjState(actor, actor->info->seestate);
	}
}

void A_BspiAttack(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	mobj_t* Missile;
		
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	
	if (!actor->player && !actor->target)
		return;
	
	// launch a missile
	Missile = P_SpawnMissile(actor, actor->target, INFO_GetTypeByName("ArachnotronShot"));
	
	// GhostlyDeath <March 6, 2012> -- Obituary Stuff
	if (Missile)
		Missile->RXAttackAttackType = PRXAT_RANGED;
}

//
// A_TroopAttack
//
void A_TroopAttack(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	int damage;
	mobj_t* Missile;
	
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	
	if (!actor->player && !actor->target)
		return;
	
	if (actor->target && P_CheckMeleeRange(actor))
	{
		S_StartSound(&actor->NoiseThinker, sfx_claw);
		damage = (P_Random() % 8 + 1) * 3;
		
		// GhostlyDeath <March 6, 2012> -- Obit check
		actor->RXAttackAttackType = PRXAT_MELEE;
		P_DamageMobj(actor->target, actor, actor, damage);
		return;
	}
	
	// launch a missile
	Missile = P_SpawnMissile(actor, actor->target, INFO_GetTypeByName("ImpShot"));
	
	// GhostlyDeath <March 6, 2012> -- Obituary Stuff
	if (Missile)
		Missile->RXAttackAttackType = PRXAT_RANGED;
}

void A_SargAttack(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	int damage;
		
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	
	if (!actor->player && !actor->target)
		return;
		
	if (actor->target && P_CheckMeleeRange(actor))
	{
		damage = ((P_Random() % 10) + 1) * 4;
		
		// GhostlyDeath <March 6, 2012> -- Obit check
		actor->RXAttackAttackType = PRXAT_MELEE;
		P_DamageMobj(actor->target, actor, actor, damage);
	}
}

void A_HeadAttack(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	int damage;
	mobj_t* Missile;
		
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	
	if (!actor->player && !actor->target)
		return;
		
	if (actor->target && P_CheckMeleeRange(actor))
	{
		damage = (P_Random() % 6 + 1) * 10;
		
		// GhostlyDeath <March 6, 2012> -- Obit check
		actor->RXAttackAttackType = PRXAT_MELEE;
		P_DamageMobj(actor->target, actor, actor, damage);
		return;
	}
	
	// launch a missile
	Missile = P_SpawnMissile(actor, actor->target, INFO_GetTypeByName("CacodemonShot"));
	
	// GhostlyDeath <March 6, 2012> -- Obituary Stuff
	if (Missile)
		Missile->RXAttackAttackType = PRXAT_RANGED;
}

void A_CyberAttack(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	mobj_t* Missile;
	
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	
	if (!actor->player && !actor->target)
		return;
		
	Missile = P_SpawnMissile(actor, actor->target, INFO_GetTypeByName("RocketShot"));
	
	// GhostlyDeath <March 6, 2012> -- Obituary Stuff
	if (Missile)
		Missile->RXAttackAttackType = PRXAT_RANGED;
}

void A_BruisAttack(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	int damage;
	mobj_t* Missile;
	
	if (!actor->player && !actor->target)
		return;
		
	if (actor->target && P_CheckMeleeRange(actor))
	{
		S_StartSound(&actor->NoiseThinker, sfx_claw);
		damage = (P_Random() % 8 + 1) * 10;
		
		// GhostlyDeath <March 6, 2012> -- Obit check
		actor->RXAttackAttackType = PRXAT_MELEE;
		P_DamageMobj(actor->target, actor, actor, damage);
		return;
	}
	
	// launch a missile
	Missile = P_SpawnMissile(actor, actor->target, INFO_GetTypeByName("BaronShot"));
	
	// GhostlyDeath <March 6, 2012> -- Obituary Stuff
	if (Missile)
		Missile->RXAttackAttackType = PRXAT_RANGED;
}

//
// A_SkelMissile
//
void A_SkelMissile(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	mobj_t* mo;
		
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	
	if (!actor->player && !actor->target)
		return;
		
	actor->z += 16 * FRACUNIT;	// so missile spawns higher
	mo = P_SpawnMissile(actor, actor->target, INFO_GetTypeByName("TracerShot"));
	actor->z -= 16 * FRACUNIT;	// back to normal
	
	if (mo)
	{
		mo->x += mo->momx;
		mo->y += mo->momy;
		P_RefMobj(PMRT_TRACER, mo, actor->target);
		mo->RXAttackAttackType = PRXAT_RANGED;
	}
}

int TRACEANGLE = 0xc000000;

void A_Tracer(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	angle_t exact;
	fixed_t dist;
	fixed_t slope;
	mobj_t* dest;
	mobj_t* th;
	
	// TODO: Demo Desync!
	
	// GhostlyDeath <September 2, 2011> -- Use map time instead of gametic
	if (gametic % (4))
		return;
		
	// spawn a puff of smoke behind the rocket
	PuffType = INFO_GetTypeByName("BulletPuff");
	P_SpawnPuff(actor->x, actor->y, actor->z);
	
	th = P_SpawnMobj(actor->x - actor->momx, actor->y - actor->momy, actor->z, INFO_GetTypeByName("TracerSmoke"));
	
	th->momz = FRACUNIT;
	th->tics -= P_Random() & 3;
	if (th->tics < 1)
		th->tics = 1;
		
	// adjust direction
	dest = actor->tracer;
	
	if (!dest || dest->health <= 0)
		return;
		
	// change angle
	exact = R_PointToAngle2(actor->x, actor->y, dest->x, dest->y);
	
	if (exact != actor->angle)
	{
		if (exact - actor->angle > 0x80000000)
		{
			actor->angle -= TRACEANGLE;
			if (exact - actor->angle < 0x80000000)
				actor->angle = exact;
		}
		else
		{
			actor->angle += TRACEANGLE;
			if (exact - actor->angle > 0x80000000)
				actor->angle = exact;
		}
	}
	
	exact = actor->angle >> ANGLETOFINESHIFT;
	actor->momx = FixedMul(__REMOOD_GETSPEEDMO(actor), finecosine[exact]);
	actor->momy = FixedMul(__REMOOD_GETSPEEDMO(actor), finesine[exact]);
	
	// change slope
	dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);
	
	dist = dist / __REMOOD_GETSPEEDMO(actor);
	
	if (dist < 1)
		dist = 1;
	slope = (dest->z + 40 * FRACUNIT - actor->z) / dist;
	
	if (slope < actor->momz)
		actor->momz -= FRACUNIT / 8;
	else
		actor->momz += FRACUNIT / 8;
}

void A_SkelWhoosh(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	
	if (!actor->player && !actor->target)
		return;
		
	// judgecutor:
	// CHECK ME!
	S_StartSound(&actor->NoiseThinker, sfx_skeswg);
}

void A_SkelFist(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	int damage;
		
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	
	if (!actor->player && !actor->target)
		return;
	
	if (actor->target && P_CheckMeleeRange(actor))
	{
		damage = ((P_Random() % 10) + 1) * 6;
		S_StartSound(&actor->NoiseThinker, sfx_skepch);
		
		// GhostlyDeath <March 6, 2012> -- Obit check
		actor->RXAttackAttackType = PRXAT_MELEE;
		P_DamageMobj(actor->target, actor, actor, damage);
	}
}

//
// PIT_VileCheck
// Detect a corpse that could be raised.
//
mobj_t* corpsehit;
mobj_t* vileobj;
fixed_t viletryx;
fixed_t viletryy;

bool_t PIT_VileCheck(mobj_t* thing, void* a_Arg)
{
	int maxdist;
	bool_t check;
	mobj_t* TheVile = a_Arg;
	
	if (!(thing->flags & MF_CORPSE))
		return true;			// not a monster
		
	if (thing->tics != -1)
		return true;			// not lying still yet
		
	
	// GhostlyDeath <June 6, 2012> -- Player on same team as us?
	if (thing->RXFlags[0] & MFREXA_ISPLAYEROBJECT)
		if (P_MobjOnSameTeam(thing, TheVile))
			return true;
	
	// GhostlyDeath <April 16, 2012> -- Arch-Viles can ressurect anything variable
	if (!P_XGSVal(PGS_MONARCHVILEANYRESPAWN))
		if (thing->info->raisestate == S_NULL)
			return true;			// monster doesn't have a raise state
		
	maxdist = thing->info->radius + TheVile->info->radius;
	
	if (abs(thing->x - viletryx) > maxdist || abs(thing->y - viletryy) > maxdist)
		return true;			// not actually touching
		
	corpsehit = thing;
	corpsehit->momx = corpsehit->momy = 0;
	corpsehit->height <<= 2;
	check = P_CheckPosition(corpsehit, corpsehit->x, corpsehit->y, 0);
	corpsehit->height >>= 2;
	
	if (!check)
		return true;			// doesn't fit here
		
	return false;				// got one, so stop checking
}

//
// A_VileChase
// Check for ressurecting a body
//
void A_VileChase(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	int xl;
	int xh;
	int yl;
	int yh;
	
	int bx;
	int by;
	
	int KCMode;
	
	PI_mobj_t* info;
	mobj_t* temp;
	
	if (actor->movedir != DI_NODIR)
	{
		// check for corpses to raise
		viletryx = actor->x + __REMOOD_GETSPEEDMO(actor) * xspeed[actor->movedir];
		viletryy = actor->y + __REMOOD_GETSPEEDMO(actor) * yspeed[actor->movedir];
		
		xl = (viletryx - bmaporgx - MAXRADIUS * 2) >> MAPBLOCKSHIFT;
		xh = (viletryx - bmaporgx + MAXRADIUS * 2) >> MAPBLOCKSHIFT;
		yl = (viletryy - bmaporgy - MAXRADIUS * 2) >> MAPBLOCKSHIFT;
		yh = (viletryy - bmaporgy + MAXRADIUS * 2) >> MAPBLOCKSHIFT;
		
		vileobj = actor;
		for (bx = xl; bx <= xh; bx++)
		{
			for (by = yl; by <= yh; by++)
			{
				// Call PIT_VileCheck to check
				// whether object is a corpse
				// that canbe raised.
				if (!P_BlockThingsIterator(bx, by, PIT_VileCheck, actor))
				{
					// got one!
					temp = actor->target;
					P_RefMobj(PMRT_TARGET, actor, corpsehit);
					A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
					P_RefMobj(PMRT_TARGET, actor, temp);
					
					// GhostlyDeath <March 6, 2012> -- Use healing state (but only if it is set)
					if (actor->info->RVileHealState)
						P_SetMobjState(actor, actor->info->RVileHealState);
					S_StartSound(&corpsehit->NoiseThinker, sfx_slop);
					info = corpsehit->info;
					
					// GhostlyDeath <March 6, 2012> -- If there is no raise state, use spawn state
					if (info->raisestate != S_NULL)
						P_SetMobjState(corpsehit, info->raisestate);
					else
						P_SetMobjState(corpsehit, info->spawnstate);
					
					if (P_XGSVal(PGS_COUNSHIFTVILERAISE))
						corpsehit->height <<= 2;
					else
					{
						corpsehit->height = __REMOOD_GETHEIGHT(info);
						corpsehit->radius = info->radius;
					}
					
					corpsehit->flags = info->flags;
					corpsehit->health = info->spawnhealth;
					P_RefMobj(PMRT_TARGET, corpsehit, NULL);
					
					// Target is player?
					if (P_MobjIsPlayer(corpsehit))
					{
						corpsehit->player->playerstate = PST_LIVE;
						corpsehit->player->health = corpsehit->health;
						corpsehit->player->pendingweapon = corpsehit->player->DeadWeapon;
					}
					
					// GhostlyDeath <June 6, 2012> -- Re-align resurrected to monster team
					corpsehit->flags2 &= ~MF2_FRIENDLY;
					if (actor->flags2 & MF2_FRIENDLY)
						corpsehit->flags2 = MF2_FRIENDLY;
					corpsehit->SkinTeamColor = actor->SkinTeamColor;
					
					// GhostlyDeath <June 15, 2012> -- Modify kill counts
					KCMode = P_XGSVal(PGS_MONKILLCOUNTMODE);
					if (KCMode == 1)		// Once
						corpsehit->flags &= ~MF_COUNTKILL;
					else if (KCMode == 2)	// Only Count Dead Monsters
					{
						// Reduce level kills
						g_MapKIS[0]--;
						
						// Player killed it?
						if (corpsehit->KillerPlayer)
							if (playeringame[corpsehit->KillerPlayer - 1])
								if (players[corpsehit->KillerPlayer - 1].FraggerID == corpsehit->FraggerID)
									players[corpsehit->KillerPlayer - 1].killcount--;
						
						// Update Scores	
						P_UpdateScores();
					}
					
					return;
				}
			}
		}
	}
	
	// Return to normal attack.
	A_Chase(actor, player, psp, a_ArgC, a_ArgV);
}

//
// A_VileStart
//
void A_VileStart(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	S_StartSound(&actor->NoiseThinker, sfx_vilatk);
}

//
// A_Fire
// Keep fire in front of player unless out of sight
//
void A_Fire(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV);

void A_StartFire(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	S_StartSound(&actor->NoiseThinker, sfx_flamst);
	A_Fire(actor, player, psp, a_ArgC, a_ArgV);
}

void A_FireCrackle(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	S_StartSound(&actor->NoiseThinker, sfx_flame);
	A_Fire(actor, player, psp, a_ArgC, a_ArgV);
}

void A_Fire(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	mobj_t* dest;
	unsigned an;
	
	dest = actor->tracer;
	if (!dest)
		return;
		
	// don't move it if the vile lost sight
	if (!P_CheckSight(actor->target, dest))
		return;
		
	an = dest->angle >> ANGLETOFINESHIFT;
	
	P_UnsetThingPosition(actor);
	actor->x = dest->x + FixedMul(24 * FRACUNIT, finecosine[an]);
	actor->y = dest->y + FixedMul(24 * FRACUNIT, finesine[an]);
	actor->z = dest->z;
	P_SetThingPosition(actor);
}

//
// A_VileTarget
// Spawn the hellfire
//
void A_VileTarget(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	mobj_t* fog;
		
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	
	if (!actor->target)
		return;
	
	fog = P_SpawnMobj(actor->target->x,
			// GhostlyDeath <April 11, 2012> -- Correct Arch-Vile fire target position
		(P_XGSVal(PGS_COCORRECTVILETARGET) ? actor->target->y : actor->target->x),
		actor->target->z, INFO_GetTypeByName("VileFire"));
	 
	P_RefMobj(PMRT_TRACER, actor, fog);
	P_RefMobj(PMRT_TARGET, fog, actor);
	P_RefMobj(PMRT_TRACER, fog, actor->target);
	A_Fire(fog, player, psp, a_ArgC, a_ArgV);
}

//
// A_VileAttack
//
void A_VileAttack(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	mobj_t* fire;
	int an;
		
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	
	if (!actor->target)
		return;
	
	if (!P_CheckSight(actor, actor->target))
		return;
		
	S_StartSound(&actor->NoiseThinker, sfx_barexp);
	
	P_DamageMobj(actor->target, actor, actor, 20);
	
	actor->target->momz = 1000 * FRACUNIT / actor->target->info->mass;
	
	an = actor->angle >> ANGLETOFINESHIFT;
	
	fire = actor->tracer;
	
	if (!fire)
		return;
		
	// move the fire between the vile and the player
	fire->x = actor->target->x - FixedMul(24 * FRACUNIT, finecosine[an]);
	fire->y = actor->target->y - FixedMul(24 * FRACUNIT, finesine[an]);
	fire->RXAttackAttackType = PRXAT_RANGED;
	P_RadiusAttack(fire, actor, 70);
}

//
// Mancubus attack,
// firing three missiles (bruisers)
// in three different directions?
// Doesn't look like it.
//
#define FATSPREAD       (ANG90/8)

void A_FatRaise(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	S_StartSound(&actor->NoiseThinker, sfx_manatk);
}

void A_FatAttack1(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	mobj_t* mo;
	int an;
	
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	// Change direction  to ...
	actor->angle += FATSPREAD;
	mo = P_SpawnMissile(actor, actor->target, INFO_GetTypeByName("MancubusShot"));
	
	// GhostlyDeath <March 6, 2012> -- Obituary Stuff
	if (mo)
		mo->RXAttackAttackType = PRXAT_RANGED;
	
	mo = P_SpawnMissile(actor, actor->target, INFO_GetTypeByName("MancubusShot"));
	if (mo)
	{
		mo->angle += FATSPREAD;
		an = mo->angle >> ANGLETOFINESHIFT;
		mo->momx = FixedMul(__REMOOD_GETSPEEDMO(mo), finecosine[an]);
		mo->momy = FixedMul(__REMOOD_GETSPEEDMO(mo), finesine[an]);
		mo->RXAttackAttackType = PRXAT_RANGED;
	}
}

void A_FatAttack2(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	mobj_t* mo;
	int an;
	
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	// Now here choose opposite deviation.
	actor->angle -= FATSPREAD;
	mo = P_SpawnMissile(actor, actor->target, INFO_GetTypeByName("MancubusShot"));
	
	// GhostlyDeath <March 6, 2012> -- Obituary Stuff
	if (mo)
		mo->RXAttackAttackType = PRXAT_RANGED;
	
	mo = P_SpawnMissile(actor, actor->target, INFO_GetTypeByName("MancubusShot"));
	if (mo)
	{
		mo->angle -= FATSPREAD * 2;
		an = mo->angle >> ANGLETOFINESHIFT;
		mo->momx = FixedMul(__REMOOD_GETSPEEDMO(mo), finecosine[an]);
		mo->momy = FixedMul(__REMOOD_GETSPEEDMO(mo), finesine[an]);
		mo->RXAttackAttackType = PRXAT_RANGED;
	}
}

void A_FatAttack3(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	mobj_t* mo;
	int an;
	
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	
	mo = P_SpawnMissile(actor, actor->target, INFO_GetTypeByName("MancubusShot"));
	if (mo)
	{
		mo->angle -= FATSPREAD / 2;
		an = mo->angle >> ANGLETOFINESHIFT;
		mo->momx = FixedMul(__REMOOD_GETSPEEDMO(mo), finecosine[an]);
		mo->momy = FixedMul(__REMOOD_GETSPEEDMO(mo), finesine[an]);
		mo->RXAttackAttackType = PRXAT_RANGED;
	}
	
	mo = P_SpawnMissile(actor, actor->target, INFO_GetTypeByName("MancubusShot"));
	if (mo)
	{
		mo->angle += FATSPREAD / 2;
		an = mo->angle >> ANGLETOFINESHIFT;
		mo->momx = FixedMul(__REMOOD_GETSPEEDMO(mo), finecosine[an]);
		mo->momy = FixedMul(__REMOOD_GETSPEEDMO(mo), finesine[an]);
		mo->RXAttackAttackType = PRXAT_RANGED;
	}
}

//
// SkullAttack
// Fly at the player like a missile.
//
#define SKULLSPEED              (20*FRACUNIT)

void A_SkullAttack(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	mobj_t* dest;
	angle_t an;
	int dist;
	
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	
	// Non-player Monster, No target
	if (!actor->target && !actor->player)
		return;
	
	// Player Monster, No target
	else if (!actor->target && actor->player)
	{
		actor->flags |= MF_SKULLFLY;
		S_StartSound(&actor->NoiseThinker, S_SoundIDForName(actor->info->RAttackSound));
		
		an = actor->angle >> ANGLETOFINESHIFT;
		actor->momx = FixedMul(SKULLSPEED, finecosine[an]);
		actor->momy = FixedMul(SKULLSPEED, finesine[an]);
	}
	
	// There is a target
	else
	{	
		dest = actor->target;
		actor->flags |= MF_SKULLFLY;
		S_StartSound(&actor->NoiseThinker, S_SoundIDForName(actor->info->RAttackSound));
	
		if (!P_MobjIsPlayer(actor) && P_XGSVal(PGS_MONPREDICTMISSILES))	//added by AC for predmonsters
		{
	
			bool_t canHit;
			fixed_t px, py, pz;
			int t, time;
			subsector_t* sec;
		
			dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);
			time = dist / SKULLSPEED;
			time = P_AproxDistance(dest->x + dest->momx * time - actor->x, dest->y + dest->momy * time - actor->y) / SKULLSPEED;
		
			canHit = 0;
			t = time + 4;
			do
			{
				t -= 4;
				if (t < 1)
					t = 1;
				px = dest->x + dest->momx * t;
				py = dest->y + dest->momy * t;
				pz = dest->z + dest->momz * t;
				canHit = P_CheckSight2(actor, dest, px, py, pz);
			}
			while (!canHit && (t > 1));
		
			sec = R_PointInSubsector(px, py);
			if (!sec)
				sec = dest->subsector;
			
			if (pz < sec->sector->floorheight)
				pz = sec->sector->floorheight;
			else if (pz > sec->sector->ceilingheight)
				pz = sec->sector->ceilingheight - dest->height;
			
			an = R_PointToAngle2(actor->x, actor->y, px, py);
		
			// fuzzy player
			if (dest->flags & MF_SHADOW)
			{
				an += P_SignedRandom() << 20;
			}
		
			actor->angle = an;
			an >>= ANGLETOFINESHIFT;
			actor->momx = FixedMul(SKULLSPEED, finecosine[an]);
			actor->momy = FixedMul(SKULLSPEED, finesine[an]);
		
			actor->momz = (pz + (dest->height >> 1) - actor->z) / t;
		}
		else
		{
			an = actor->angle >> ANGLETOFINESHIFT;
			actor->momx = FixedMul(SKULLSPEED, finecosine[an]);
			actor->momy = FixedMul(SKULLSPEED, finesine[an]);
			dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);
			dist = dist / SKULLSPEED;
		
			if (dist < 1)
				dist = 1;
			actor->momz = (dest->z + (dest->height >> 1) - actor->z) / dist;
		}
	}
}

//
// P_PainShootSkull
// Spawn a lost soul and launch it at the target
//
void P_PainShootSkull(mobj_t* actor, angle_t angle, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	fixed_t x;
	fixed_t y;
	fixed_t z;
	
	PI_mobjid_t TargetType;
	mobj_t* newmobj;
	angle_t an;
	int prestep;
	
	/*  --------------- SKULL LIMITE CODE -----------------
	   int         count;
	   thinker_t*  currentthinker;
	
	   // count total number of skull currently on the level
	   count = 0;
	
	   currentthinker = thinkercap.next;
	   while (currentthinker != &thinkercap)
	   {
	   if (   (currentthinker->function.acp1 == (actionf_p1)P_MobjThinker)
	   && ((mobj_t *)currentthinker)->type == "LostSoul")
	   count++;
	   currentthinker = currentthinker->next;
	   }
	
	   // if there are allready 20 skulls on the level,
	   // don't spit another one
	   if (count > 20)
	   return;
	   ---------------------------------------------------
	 */
	
	// okay, there's place for another one
	an = angle >> ANGLETOFINESHIFT;
	
	// GhostlyDeath <March 6, 2012> -- Get target type
	TargetType = INFO_GetTypeByName("LostSoul");
	
	// Bad object?
	if (!(TargetType >= 0 && TargetType < NUMMOBJTYPES))
		return;
	
	prestep = 4 * FRACUNIT + 3 * (actor->info->radius + mobjinfo[TargetType]->radius) / 2;
	
	x = actor->x + FixedMul(prestep, finecosine[an]);
	y = actor->y + FixedMul(prestep, finesine[an]);
	z = actor->z + 8 * FRACUNIT;
	
	newmobj = P_SpawnMobj(x, y, z, TargetType);
	
	// Check for movements.
	if (!P_TryMove(newmobj, newmobj->x, newmobj->y, false, NULL, NULL))
	{
		// kill it immediately
		P_DamageMobj(newmobj, actor, actor, 10000);
		return;
	}
	
	P_RefMobj(PMRT_TARGET, newmobj, actor->target);
	A_SkullAttack(newmobj, player, psp, a_ArgC, a_ArgV);
	
	// GhostlyDeath <June 6, 2012> -- Make on same side as PE
	newmobj->SkinTeamColor = actor->SkinTeamColor;
	newmobj->flags2 |= actor->flags2 & MF2_FRIENDLY;
}

//
// A_PainAttack
// Spawn a lost soul and launch it at the target
//
void A_PainAttack(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	
	if (!actor->player && !actor->target)
		return;
		
	P_PainShootSkull(actor, actor->angle, player, psp, a_ArgC, a_ArgV);
}

void A_PainDie(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	A_Fall(actor, player, psp, a_ArgC, a_ArgV);
	P_PainShootSkull(actor, actor->angle + ANG90, player, psp, a_ArgC, a_ArgV);
	P_PainShootSkull(actor, actor->angle + ANG180, player, psp, a_ArgC, a_ArgV);
	P_PainShootSkull(actor, actor->angle + ANG270, player, psp, a_ArgC, a_ArgV);
}

void A_Scream(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	int sound;
	sfxid_t SoundID;
	
	SoundID = S_SoundIDForName(actor->info->RDeathSound);
	switch (SoundID)
	{
		case 0:
			return;
			
		case sfx_podth1:
		case sfx_podth2:
		case sfx_podth3:
			sound = sfx_podth1 + P_Random() % 3;
			break;
			
		case sfx_bgdth1:
		case sfx_bgdth2:
			sound = sfx_bgdth1 + P_Random() % 2;
			break;
			
		default:
			sound = SoundID;
			break;
	}
	
	// Check for bosses.
	
	if (actor->RXFlags[0] & MFREXA_SOUNDEVERYWHERE)
	{
		// full volume
		S_StartSound(NULL, sound);
	}
	else
		S_StartSound(&actor->NoiseThinker, sound);
}

void A_XScream(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	S_StartSound(&actor->NoiseThinker, sfx_slop);
}

void A_Pain(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	if (actor->info->RPainSound)
		S_StartSound(&actor->NoiseThinker, S_SoundIDForName(actor->info->RPainSound));
}

//
//  A dying thing falls to the ground (monster deaths)
//
void A_Fall(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	// actor is on ground, it can be walked over
	if (!P_XGSVal(PGS_GAMESOLIDCORPSES))
		actor->flags &= ~MF_SOLID;
	if (P_XGSVal(PGS_COMODIFYCORPSE))
	{
		actor->flags |= MF_CORPSE | MF_DROPOFF;
		actor->height >>= 2;
		actor->radius -= (actor->radius >> 4);	//for solid corpses
		actor->health = actor->info->spawnhealth >> 1;
	}
	// So change this if corpse objects
	// are meant to be obstacles.
}

//
// A_Explode
//
void A_Explode(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	int damage = 128;
	
	switch (actor->type)
	{
		default:
			break;
	}
	
	P_RadiusAttack(actor, actor->target, damage);
	
	// GhostlyDeath <June 17, 2012> -- Demo Comp
	if (P_XGSVal(PGS_COEXPLODEHITFLOOR))
		P_HitFloor(actor);
}

static PI_state_t* P_FinalState(PI_stateid_t state)
{
	static char* final_state;
	static size_t OldStateCount;
	
	// GhostlyDeath <April 23, 2012> -- Cheat the quick "temporary" hack
	if (NUMSTATES != OldStateCount || !final_state)
	{
		if (final_state)
			Z_Free(final_state);
		final_state = Z_Malloc(sizeof(*final_state) * NUMSTATES, PU_STATIC, NULL);
		OldStateCount = NUMSTATES;
	}
	
	memset(final_state, 0, NUMSTATES);
	while (states[state]->tics != -1)
	{
		final_state[state] = 1;
		state = states[state]->nextstate;
		if (final_state[state])
			return NULL;
	}
	
	return states[state];
}

//
// A_BossDeath
// Possibly trigger special effects
// if on first boss level
//
void A_BossDeath(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	bool_t OK;
	thinker_t* th;
	mobj_t* CheckMo;
	uint32_t CheckFlags;
	line_t junk;
	int32_t Args[2];
	
	/* Check */
	if (!mo)
		return;
	
	/* Do comparitive matching */
	// Set to false
	OK = false;
	
	if (
		// MAP07 Special (666 and 667 sectors)
		(g_CurrentLevelInfo->MapSevenSpecial && (mo->RXFlags[1] & (MFREXB_DOMAPSEVENSPECA | MFREXB_DOMAPSEVENSPECB))) ||
		
		// Baron special
		(g_CurrentLevelInfo->BaronSpecial && (mo->RXFlags[1] & (MFREXB_DOBARONSPECIAL))) ||
		
		// Cyberdemon special
		(g_CurrentLevelInfo->CyberSpecial && (mo->RXFlags[1] & (MFREXB_DOCYBERSPECIAL))) ||
		
		// Spider Mastermind special
		(g_CurrentLevelInfo->SpiderdemonSpecial && (mo->RXFlags[1] & (MFREXB_DOSPIDERSPECIAL))) ||
		
		// Normal command keen open door
		(mo->RXFlags[1] & MFREXB_DODOORSIXTHREEOPEN)
		)
		OK = true;
	
	/* Not OK? */
	if (!OK)
		return;
	
	/* Obtain flags to check */
	CheckFlags = mo->RXFlags[1] & (MFREXB_DOMAPSEVENSPECA | MFREXB_DOMAPSEVENSPECB | MFREXB_DOBARONSPECIAL | MFREXB_DOCYBERSPECIAL | MFREXB_DOSPIDERSPECIAL | MFREXB_DODOORSIXTHREEOPEN);
	
	/* Check to see if every other object (with flag set) is dead */
	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		// Not a map object?
		if (th->function.acp1 != (actionf_p1) P_MobjThinker)
			continue;
		
		// Get reference of it
		CheckMo = (mobj_t*)th;
		
		// Ignore Self
		if (CheckMo == mo)
			continue;
		
		// If the flags match then the object is of the same special kind
		if (CheckMo->RXFlags[1] & CheckFlags)
		{
			// See if it is not fully dead yet
				// TODO FIXME: May be a compat issue here
			//if (CheckMo->state != P_FinalState(CheckMo->info->deathstate))
			if (!(CheckMo->health <= 0 || CheckMo->flags & MF_CORPSE))
				return;
		}
	}
	
	/* Clear junk line */
	memset(&junk, 0, sizeof(junk));
	
	/* Everything with this flag is dead, so do the action */
#define MULTISPECFLAGS (MFREXB_DOBARONSPECIAL | MFREXB_DOCYBERSPECIAL | MFREXB_DOSPIDERSPECIAL)
	// Do MAP06 666/667 actions
	if (g_CurrentLevelInfo->MapSevenSpecial)
	{
		// 666
		if (CheckFlags & MFREXB_DOMAPSEVENSPECA)
		{
			junk.tag = 666;
			Args[0] = lowerFloorToLowest;
			EV_DoFloor(&junk, -1, mo, LAT_SWITCH, 0, NULL, 2, Args);
		}
		
		// 667
		if (CheckFlags & MFREXB_DOMAPSEVENSPECB)
		{
			junk.tag = 667;
			Args[0] = raiseToTexture;
			EV_DoFloor(&junk, -1, mo, LAT_SWITCH, 0, NULL, 2, Args);
		}
	}
	
	// Blazing 666 door open
	if (CheckFlags & MFREXB_DODOORSIXTHREEOPEN)
	{
		junk.tag = 666;
		Args[0] = dooropen;
		Args[1] = VDOORSPEED;
		EV_DoDoor(&junk, -1, mo, LAT_SWITCH, 0, NULL, 2, Args);
	}
	
	// Kill everything in the level
	if ((CheckFlags & MULTISPECFLAGS) && g_CurrentLevelInfo->KillMonstersOnSpecial)
	{
		// Go through thinker loop, again
		for (th = thinkercap.next; th != &thinkercap; th = th->next)
		{
			// Not a map object?
			if (th->function.acp1 != (actionf_p1) P_MobjThinker)
				continue;
		
			// Get reference of it
			CheckMo = (mobj_t*)th;
		
			// Ignore Self
			if (CheckMo == mo)
				continue;
			
			// Ignore players
			if (CheckMo->RXFlags[0] & MFREXA_ISPLAYEROBJECT)
				continue;
		}
	}
	
	// Open a door
	if ((CheckFlags & MULTISPECFLAGS) && g_CurrentLevelInfo->OpenDoorOnSpecial)
	{
		junk.tag = 666;
		Args[0] = blazeOpen;
		Args[1] = 4 * VDOORSPEED;
		EV_DoDoor(&junk, -1, mo, LAT_SWITCH, 0, NULL, 2, Args);
	}
	
	// Lower floor
	if ((CheckFlags & MULTISPECFLAGS) && g_CurrentLevelInfo->LowerFloorOnSpecial)
	{
		junk.tag = 666;
		Args[0] = lowerFloorToLowest;
		EV_DoFloor(&junk, -1, mo, LAT_SWITCH, 0, NULL, 2, Args);
	}
	
	// Level Exiting Last
	if ((CheckFlags & MULTISPECFLAGS) && g_CurrentLevelInfo->ExitOnSpecial)
	{
		if (P_XGSVal(PGS_GAMEALLOWLEVELEXIT))
			G_ExitLevel(false, mo->target, NULL);
	}
#undef MULTISPECFLAGS
}

//
// A_KeenDie
// DOOM II special, map 32.
// Uses special tag 666.
//
void A_KeenDie(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	A_Fall(mo, player, psp, a_ArgC, a_ArgV);
	
	A_BossDeath(mo, player, psp, a_ArgC, a_ArgV);
}

void A_Hoof(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	S_StartSound(&mo->NoiseThinker, sfx_hoof);
	A_Chase(mo, player, psp, a_ArgC, a_ArgV);
}

void A_Metal(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	S_StartSound(&mo->NoiseThinker, sfx_metal);
	A_Chase(mo, player, psp, a_ArgC, a_ArgV);
}

void A_BabyMetal(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	S_StartSound(&mo->NoiseThinker, sfx_bspwlk);
	A_Chase(mo, player, psp, a_ArgC, a_ArgV);
}

void A_OpenShotgun2(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	S_StartSound(&mo->NoiseThinker, sfx_dbopn);
}

void A_LoadShotgun2(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	S_StartSound(&mo->NoiseThinker, sfx_dbload);
}

void A_ReFire(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV);

void A_CloseShotgun2(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	S_StartSound(&mo->NoiseThinker, sfx_dbcls);
	A_ReFire(mo, player, psp, a_ArgC, a_ArgV);
}

/* A_GenericMonsterMissile() -- Generic Monster Projectile */
void A_GenericMonsterMissile(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	mobj_t* Missile;
		
	A_FaceTarget(actor, player, psp, a_ArgC, a_ArgV);
	
	if (!actor->player && !actor->target)
		return;
	
	// launch a missile
	if (actor->info->RGenericMissile)
	{
		Missile = P_SpawnMissile(actor, actor->target, INFO_GetTypeByName(actor->info->RGenericMissile));
	
		if (Missile)
			Missile->RXAttackAttackType = PRXAT_RANGED;
	}
}

static mobj_t* braintargets[32];
static int numbraintargets;
static int braintargeton;

void P_InitBrainTarget()
{
	thinker_t* thinker;
	mobj_t* m;
	
	// find all the target spots
	numbraintargets = 0;
	braintargeton = 0;
	
	thinker = thinkercap.next;
	for (thinker = thinkercap.next; thinker != &thinkercap; thinker = thinker->next)
	{
		if (thinker->function.acp1 != (actionf_p1) P_MobjThinker)
			continue;			// not a mobj
			
		m = (mobj_t*)thinker;
		
		// GhostlyDeath <March 6, 2012> -- Is this a brain target?
		if (m->RXFlags[0] & MFREXA_ISBRAINTARGET)
		{
			braintargets[numbraintargets] = m;
			numbraintargets++;
		}
	}
}

void A_BrainAwake(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	S_StartSound(NULL, sfx_bossit);
}

void A_BrainPain(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	S_StartSound(NULL, sfx_bospn);
}

void A_BrainScream(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	int x;
	int y;
	int z;
	mobj_t* th;
	PI_mobjid_t SpawnThing;
	
	// GhostlyDeath <March 6, 2012> -- Custom thing spawning
	if (mo->info->RBrainExplodeThing)
	{
		// Get the exploding thing class
		SpawnThing = INFO_GetTypeByName(mo->info->RBrainExplodeThing);
		
		// Spawn alot of objects
		if (SpawnThing >= 0 && SpawnThing < NUMMOBJTYPES)
			for (x = mo->x - 196 * FRACUNIT; x < mo->x + 320 * FRACUNIT; x += FRACUNIT * 8)
			{
				y = mo->y - 320 * FRACUNIT;
				z = 128 + P_Random() * 2 * FRACUNIT;
				th = P_SpawnMobj(x, y, z, SpawnThing);
				th->momz = P_Random() * 512;
				
				// Explode if possible
				if (th->info->RBrainExplodeState)
					P_SetMobjState(th, th->info->RBrainExplodeState);
		
				th->tics -= P_Random() & 7;
				if (th->tics < 1)
					th->tics = 1;
			}
	}
	
	S_StartSound(NULL, sfx_bosdth);
}

void A_BrainExplode(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	int x;
	int y;
	int z;
	mobj_t* th;
	
	x = (P_SignedRandom() << 11) + mo->x;
	y = mo->y;
	z = 128 + P_Random() * 2 * FRACUNIT;
	
	// GhostlyDeath <March 6, 2012> -- Spawn self again
	th = P_SpawnMobj(x, y, z, mo->type);
	th->momz = P_Random() * 512;
	
	// Explode if possible
	if (th->info->RBrainExplodeState)
		P_SetMobjState(th, th->info->RBrainExplodeState);
	
	th->tics -= P_Random() & 7;
	if (th->tics < 1)
		th->tics = 1;
}

void A_BrainDie(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	if (P_XGSVal(PGS_GAMEALLOWLEVELEXIT))
		G_ExitLevel(false, mo->target, NULL);
}

void A_BrainSpit(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	mobj_t* targ;
	mobj_t* newmobj;
	
	// GhostlyDeath <June 15, 2012> -- Flip the spit bit
	mo->RXFlags[1] ^= MFREXB_SPITBIT;
	if (P_XGSVal(PGS_GAMESKILL) <= sk_easy)
		if (!(mo->RXFlags[1] & MFREXB_SPITBIT))
			return;
		
	if (numbraintargets > 0)
	{
		// shoot a cube at current target
		targ = braintargets[braintargeton];
		braintargeton = (braintargeton + 1) % numbraintargets;
		
		// spawn brain missile
		newmobj = P_SpawnMissile(mo, targ, INFO_GetTypeByName("BossSpawnCube"));
		if (newmobj)
		{
			P_RefMobj(PMRT_TARGET, newmobj, targ);
			newmobj->reactiontime = ((targ->y - mo->y) / newmobj->momy) / newmobj->state->tics;
		}
		
		S_StartSound(NULL, sfx_bospit);
	}
}

void A_SpawnFly(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV);

// travelling cube sound
void A_SpawnSound(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	S_StartSound(&mo->NoiseThinker, sfx_boscub);
	A_SpawnFly(mo, player, psp, a_ArgC, a_ArgV);
}

void A_SpawnFly(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	mobj_t* newmobj;
	mobj_t* fog;
	mobj_t* targ;
	int r;
	PI_mobjid_t type;
	size_t i;
	
	if (--mo->reactiontime)
		return;					// still flying
		
	targ = mo->target;
	
	// First spawn teleport fog.
	fog = P_SpawnMobj(targ->x, targ->y, targ->z, INFO_GetTypeByName("BossSpawnFire"));
	S_StartSound(&fog->NoiseThinker, sfx_telept);
	
	// Randomly select monster to spawn.
	r = P_Random();
	
	// GhostlyDeath <March 8, 2012> -- Boss spawn list
	// Probability distribution (kind of :),
	// decreasing likelihood.
	for (i = 0; i < g_NumBossSpitList; i++)
		if (r < g_BossSpitList[i].Chance)
		{
			type = g_BossSpitList[i].Type;
			break;
		}
	
	// Make sure it really is valid
	if (type >= 0 && type < NUMMOBJTYPES)
	{
		newmobj = P_SpawnMobj(targ->x, targ->y, targ->z, type);
		if (P_LookForPlayers(newmobj, true))
			P_SetMobjState(newmobj, newmobj->info->seestate);
		
		// telefrag anything in this spot
		P_TeleportMove(newmobj, newmobj->x, newmobj->y);
	}
	
	// remove self (i.e., cube).
	P_RemoveMobj(mo);
}

void A_PlayerScream(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	// Default death sound.
	int sound = sfx_pldeth;
	
	// IF THE PLAYER DIES LESS THAN -50% WITHOUT GIBBING
	if (g_IWADFlags & CIF_COMMERCIAL)
		if (mo->health < -50)
			sound = sfx_pdiehi;
	
	// Play Sound
	S_StartSound(&mo->NoiseThinker, sound);
}

/* A_NextFrameIfMoving() -- Goes to the next frame if the object is moving */
void A_NextFrameIfMoving(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	/* Moving */
	if (mo->momx || mo->momy)
	{
	}
	
	/* Not Moving */
	else
	{
	}
}

//
// Action routine, for the ROCKET thing.
// This one adds trails of smoke to the rocket.
// The action pointer of the S_ROCKET state must point here to take effect.
// This routine is based on the Revenant Fireball Tracer code A_Tracer()
//
void A_SmokeTrailer(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	mobj_t* th;
	
	// GhostlyDeath <March 6, 2012> -- Check version (not before Legacy 1.11)
	if (P_XGSVal(PGS_CONOSMOKETRAILS))
		return;
	
	// Only every 4 gametics
	if (gametic % (4))
		return;
		
	// GhostlyDeath <April 12, 2012> -- Extra puffs before v1.25
		// Before 1.25, bullet puffs appeared with smoke puffs for some reason.
	if (P_XGSVal(PGS_COEXTRATRAILPUFF))
	{
		PuffType = INFO_GetTypeByName("BulletPuff");
		P_SpawnPuff(actor->x, actor->y, actor->z);
	}
	
	// add the smoke behind the rocket
	th = P_SpawnMobj(actor->x - actor->momx, actor->y - actor->momy, actor->z, (P_XGSVal(PGS_COUSEREALSMOKE) ? INFO_GetTypeByName("LegacySmoke") : INFO_GetTypeByName("TracerSmoke")));

	th->momz = FRACUNIT;
	th->tics -= P_Random() & 3;
	if (th->tics < 1)
		th->tics = 1;
}

/* A_SmokeTrailerRocket() -- Trails for rockets */
void A_SmokeTrailerRocket(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	A_SmokeTrailer(actor, player, psp, a_ArgC, a_ArgV);
}

/* A_SmokeTrailerSkull() -- Trails for skulls */
void A_SmokeTrailerSkull(mobj_t* actor, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	/* Check flag */
	// Before v1.25? Lost souls did not emit smoke
	if (!P_XGSVal(PGS_COLOSTSOULTRAILS))
		return;
	
	A_SmokeTrailer(actor, player, psp, a_ArgC, a_ArgV);
}

