// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.sourceforge.net/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           Demyx                  (demyx@endgameftw.com)
//                    Dragan                 (poliee13@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2009 The ReMooD Team..
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION:
//      Enemy thinking, AI.
//      Action Pointer Functions
//      that are associated with states/frames.

#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "../src_cl/r_main.h"
#include "../src_cl/r_state.h"
#include "s_sound.h"
#include "m_random.h"
#include "p_demcmp.h"

void FastMonster_OnChange(void);

// enable the solid corpses option : still not finished
consvar_t cv_solidcorpse = { "solidcorpse", "0", CV_NETVAR | CV_SAVE, CV_OnOff };
consvar_t cv_fastmonsters = { "fastmonsters", "0", CV_NETVAR | CV_CALL, CV_OnOff,
	FastMonster_OnChange
};
consvar_t cv_predictingmonsters = { "predictingmonsters", "0", CV_NETVAR | CV_SAVE, CV_OnOff };	//added by AC for predmonsters
consvar_t cv_classicmonsterlogic = { "classicmonsterlogic", "0", CV_NETVAR | CV_SAVE, CV_YesNo };

typedef enum
{
	DI_EAST,
	DI_NORTHEAST,
	DI_NORTH,
	DI_NORTHWEST,
	DI_WEST,
	DI_SOUTHWEST,
	DI_SOUTH,
	DI_SOUTHEAST,
	DI_NODIR,
	NUMDIRS
} dirtype_t;

//
// P_NewChaseDir related LUT.
//
static dirtype_t opposite[] = {
	DI_WEST, DI_SOUTHWEST, DI_SOUTH, DI_SOUTHEAST,
	DI_EAST, DI_NORTHEAST, DI_NORTH, DI_NORTHWEST, DI_NODIR
};

static dirtype_t diags[] = {
	DI_NORTHWEST, DI_NORTHEAST, DI_SOUTHWEST, DI_SOUTHEAST
};

void A_Fall(mobj_t * actor);

void FastMonster_OnChange(void)
{
	static boolean fast = false;
	static const struct
	{
		mobjtype_t type;
		int speed[2];
	} MonsterMissileInfo[] =
	{
		// doom
		{
			MT_BRUISERSHOT,
			{
		15, 20}},
		{
			MT_HEADSHOT,
			{
		10, 20}},
		{
			MT_TROOPSHOT,
			{
		10, 20}},
		// heretic
		{
			MT_IMPBALL,
			{
		10, 20}},
		{
			MT_MUMMYFX1,
			{
		9, 18}},
		{
			MT_KNIGHTAXE,
			{
		9, 18}},
		{
			MT_REDAXE,
			{
		9, 18}},
		{
			MT_BEASTBALL,
			{
		12, 20}},
		{
			MT_WIZFX1,
			{
		18, 24}},
		{
			MT_SNAKEPRO_A,
			{
		14, 20}},
		{
			MT_SNAKEPRO_B,
			{
		14, 20}},
		{
			MT_HEADFX1,
			{
		13, 20}},
		{
			MT_HEADFX3,
			{
		10, 18}},
		{
			MT_MNTRFX1,
			{
		20, 26}},
		{
			MT_MNTRFX2,
			{
		14, 20}},
		{
			MT_SRCRFX1,
			{
		20, 28}},
		{
			MT_SOR2FX1,
			{
		20, 28}},
		{
			-1,
			{
		-1, -1}}				// Terminator
	};

	int i;
	if (cv_fastmonsters.value && !fast)
	{
		for (i = S_SARG_RUN1; i <= S_SARG_PAIN2; i++)
			states[i].tics >>= 1;
		fast = true;
	}
	else if (!cv_fastmonsters.value && fast)
	{
		for (i = S_SARG_RUN1; i <= S_SARG_PAIN2; i++)
			states[i].tics <<= 1;
		fast = false;
	}

	for (i = 0; MonsterMissileInfo[i].type != -1; i++)
	{
		mobjinfo[MonsterMissileInfo[i].type].speed
			= MonsterMissileInfo[i].speed[cv_fastmonsters.value] << FRACBITS;
	}
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

static mobj_t *soundtarget;

static void P_RecursiveSound(sector_t * sec, int soundblocks)
{
	int i;
	line_t *check;
	sector_t *other;

	// wake up all monsters in this sector
	if (sec->validcount == validcount && sec->soundtraversed <= soundblocks + 1)
	{
		return;					// already flooded
	}

	sec->validcount = validcount;
	sec->soundtraversed = soundblocks + 1;
	sec->soundtarget = soundtarget;

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
void P_NoiseAlert(mobj_t * target, mobj_t * emmiter)
{
	soundtarget = target;
	validcount++;
	P_RecursiveSound(emmiter->subsector->sector, 0);
}

consvar_t cv_classicmeleerange = { "classicmeleerange", "0", CV_NETVAR, CV_YesNo };

//
// P_CheckMeleeRange
//
static boolean P_CheckMeleeRange(mobj_t * actor)
{
	mobj_t *pl;
	fixed_t dist;

	if (!actor->target)
		return false;

	pl = actor->target;
	dist = P_AproxDistance(pl->x - actor->x, pl->y - actor->y);

	if (dist >= MELEERANGE - 20 * FRACUNIT + pl->info->radius)
		return false;

	//added:19-03-98: check height now, so that damn imps cant attack
	//                you if you stand on a higher ledge.
	if (!DEMOCVAR(classicmeleerange).value &&
		((pl->z > actor->z + actor->height) || (actor->z > pl->z + pl->height)))
		return false;

	if (!P_CheckSight(actor, actor->target))
		return false;

	return true;
}

//
// P_CheckMissileRange
//
static boolean P_CheckMissileRange(mobj_t * actor)
{
	fixed_t dist;

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
	dist = P_AproxDistance(actor->x - actor->target->x,
						   actor->y - actor->target->y) - 64 * FRACUNIT;

	if (!actor->info->meleestate)
		dist -= 128 * FRACUNIT;	// no melee attack, so fire more

	dist >>= 16;

	if (actor->type == MT_VILE)
	{
		if (dist > 14 * 64)
			return false;		// too far away
	}

	if (actor->type == MT_UNDEAD)
	{
		if (dist < 196)
			return false;		// close for fist attack
		dist >>= 1;
	}

	if (actor->type == MT_CYBORG || actor->type == MT_SPIDER || actor->type == MT_SKULL)
	{
		dist >>= 1;
	}

	if (dist > 200)
		dist = 200;

	if (actor->type == MT_CYBORG && dist > 160)
		dist = 160;

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

static boolean P_Move(mobj_t * actor)
{
	fixed_t tryx;
	fixed_t tryy;

	line_t *ld;

	boolean good;

	if (actor->movedir == DI_NODIR)
		return false;

#ifdef PARANOIA
	if ((unsigned)actor->movedir >= 8)
		I_Error("Weird actor->movedir!");
#endif

	tryx = actor->x + actor->info->speed * xspeed[actor->movedir];
	tryy = actor->y + actor->info->speed * yspeed[actor->movedir];

	if (!P_TryMove(actor, tryx, tryy, false))
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

		actor->movedir = DI_NODIR;
		good = false;
		while (numspechit--)
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
static boolean P_TryWalk(mobj_t * actor)
{
	if (!P_Move(actor))
	{
		return false;
	}
	actor->movecount = P_Random() & 15;
	return true;
}

static void P_NewChaseDir(mobj_t * actor)
{
	fixed_t deltax;
	fixed_t deltay;

	dirtype_t d[3];

	int tdir;
	dirtype_t olddir;

	dirtype_t turnaround;

	if (!actor->target)
		I_Error("P_NewChaseDir: called with no target");

	olddir = actor->movedir;
	turnaround = opposite[olddir];

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
static boolean P_LookForPlayers(mobj_t * actor, boolean allaround)
{
	int c;
	int stop;
	player_t *player;
	sector_t *sector;
	angle_t an;
	fixed_t dist;
	
	// Single player game and player is dead, look for monsters
	if (gamemode == heretic && !multiplayer && players[0].health <= 0)
		return P_LookForMonsters(actor);

	sector = actor->subsector->sector;

	// BP: first time init, this allow minimum lastlook changes
	if (actor->lastlook < 0 && demoversion >= 129)
		actor->lastlook = P_Random() % MAXPLAYERS;

	c = 0;
	stop = (actor->lastlook - 1) & PLAYERSMASK;

	for (;; actor->lastlook = (actor->lastlook + 1) & PLAYERSMASK)
	{
		// done looking
		if (actor->lastlook == stop)
			return false;

		if (!playeringame[actor->lastlook])
			continue;

		if (c++ == 2)
			return false;

		player = &players[actor->lastlook];

		if (player->health <= 0)
			continue;			// dead

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
		
		if (gamemode == heretic && player->mo->flags & MF_SHADOW)
		{						// Player is invisible
			if ((P_AproxDistance(player->mo->x - actor->x,
								 player->mo->y - actor->y) > 2 * MELEERANGE)
				&& P_AproxDistance(player->mo->momx, player->mo->momy) < 5 * FRACUNIT)
			{					// Player is sneaking - can't detect
				return (false);
			}
			if (P_Random() < 225)
			{					// Player isn't sneaking, but still didn't detect
				return (false);
			}
		}

		actor->target = player->mo;
		return true;
	}

	return false;
}

//
// ACTION ROUTINES
//

//
// A_Look
// Stay in state until a player is sighted.
//
void A_Look(mobj_t * actor)
{
	mobj_t *targ;

	actor->threshold = 0;		// any shot will wake up
	targ = actor->subsector->sector->soundtarget;

	if (targ && (targ->flags & MF_SHOOTABLE))
	{
		actor->target = targ;

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
	if (actor->info->seesound)
	{
		int sound;

		switch (actor->info->seesound)
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
				sound = actor->info->seesound;
				break;
		}

		if (actor->type == MT_SPIDER || actor->type == MT_CYBORG || (actor->flags2 & MF2_BOSS))
		{
			// full volume
			S_StartSound(NULL, sound);
		}
		else
			S_StartScreamSound(actor, sound);

	}

	P_SetMobjState(actor, actor->info->seestate);
}

//
// A_Chase
// Actor has a melee attack,
// so it tries to close as fast as possible
//
void A_Chase(mobj_t * actor)
{
	int delta;

	if (actor->reactiontime)
		actor->reactiontime--;

	// modify target threshold
	if (actor->threshold)
	{
		if (DEMOCVAR(classicmonsterlogic).value)
		{
			if (gamemode != heretic || !actor->target || actor->target->health <= 0)
				actor->threshold = 0;
			else
				actor->threshold--;
		}
		else if (gamemode != heretic || (!actor->target || actor->target->health <= 0 || (actor->target->flags & MF_CORPSE)))
			actor->threshold = 0;
	}
	
	if (gamemode == heretic && cv_fastmonsters.value)
	{	// Monsters move faster in nightmare mode
		actor->tics -= actor->tics / 2;
		if (actor->tics < 3)
			actor->tics = 3;
	}

	// turn towards movement direction if not there yet
	if (actor->movedir < 8)
	{
		actor->angle &= (7 << 29);
		delta = actor->angle - (actor->movedir << 29);

		if (delta > 0)
			actor->angle -= ANG90 / 2;
		else if (delta < 0)
			actor->angle += ANG90 / 2;
	}

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
		if (!cv_fastmonsters.value)
			P_NewChaseDir(actor);
		return;
	}

	// check for melee attack
	if (actor->info->meleestate && P_CheckMeleeRange(actor))
	{
		if (actor->info->attacksound)
			S_StartAttackSound(actor, actor->info->attacksound);

		P_SetMobjState(actor, actor->info->meleestate);
		return;
	}

	// check for missile attack
	if (actor->info->missilestate)
	{
		if (!cv_fastmonsters.value && actor->movecount)
		{
			goto nomissile;
		}

		if (!P_CheckMissileRange(actor))
			goto nomissile;

		P_SetMobjState(actor, actor->info->missilestate);
		actor->flags |= MF_JUSTATTACKED;
		return;
	}

	// ?
  nomissile:
	// possibly choose another target
	if (multiplayer && !actor->threshold && !P_CheckSight(actor, actor->target))
	{
		if (P_LookForPlayers(actor, true))
			return;				// got a new target
	}

	// chase towards player
	if (--actor->movecount < 0 || !P_Move(actor))
	{
		P_NewChaseDir(actor);
	}

	// make active sound
	if (actor->info->activesound && P_Random() < 3)
	{
		if (actor->type == MT_WIZARD && P_Random() < 128)
			S_StartScreamSound(actor, actor->info->seesound);
		else if (actor->type == MT_SORCERER2)
			S_StartSound(NULL, actor->info->activesound);
		else
			S_StartScreamSound(actor, actor->info->activesound);
	}
}

//
// A_FaceTarget
//
void A_FaceTarget(mobj_t * actor)
{
	if (!actor->target)
		return;

	actor->flags &= ~MF_AMBUSH;

	actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);

	if (actor->target->flags & MF_SHADOW)
		actor->angle += P_SignedRandom() << 21;
}

//
// A_PosAttack
//
void A_PosAttack(mobj_t * actor)
{
	int angle;
	int damage;
	int slope;

	if (!actor->target)
		return;

	PuffType = MT_PUFF;
	A_FaceTarget(actor);
	angle = actor->angle;
	slope = P_AimLineAttack(actor, angle, MISSILERANGE);

	S_StartAttackSound(actor, sfx_pistol);
	angle += P_SignedRandom() << 20;
	damage = ((P_Random() % 5) + 1) * 3;
	P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
}

void A_SPosAttack(mobj_t * actor)
{
	int i;
	int angle;
	int bangle;
	int damage;
	int slope;

	if (!actor->target)
		return;
	PuffType = MT_PUFF;
	S_StartAttackSound(actor, sfx_shotgn);
	A_FaceTarget(actor);
	bangle = actor->angle;
	slope = P_AimLineAttack(actor, bangle, MISSILERANGE);

	for (i = 0; i < 3; i++)
	{
		angle = (P_SignedRandom() << 20) + bangle;
		damage = ((P_Random() % 5) + 1) * 3;
		P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
	}
}

void A_CPosAttack(mobj_t * actor)
{
	int angle;
	int bangle;
	int damage;
	int slope;

	if (!actor->target)
		return;
	PuffType = MT_PUFF;
	S_StartAttackSound(actor, sfx_shotgn);
	A_FaceTarget(actor);
	bangle = actor->angle;
	slope = P_AimLineAttack(actor, bangle, MISSILERANGE);

	angle = (P_SignedRandom() << 20) + bangle;

	damage = ((P_Random() % 5) + 1) * 3;
	P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
}

void A_CPosRefire(mobj_t * actor)
{
	// keep firing unless target got out of sight
	A_FaceTarget(actor);

	if (P_Random() < 40)
		return;

	if (!actor->target
		|| actor->target->health <= 0
		|| actor->target->flags & MF_CORPSE || !P_CheckSight(actor, actor->target))
	{
		P_SetMobjState(actor, actor->info->seestate);
	}
}

void A_SpidRefire(mobj_t * actor)
{
	// keep firing unless target got out of sight
	A_FaceTarget(actor);

	if (P_Random() < 10)
		return;

	if (!actor->target
		|| actor->target->health <= 0
		|| actor->target->flags & MF_CORPSE || !P_CheckSight(actor, actor->target))
	{
		P_SetMobjState(actor, actor->info->seestate);
	}
}

void A_BspiAttack(mobj_t * actor)
{
	if (!actor->target)
		return;

	A_FaceTarget(actor);

	// launch a missile
	P_SpawnMissile(actor, actor->target, MT_ARACHPLAZ);
}

//
// A_TroopAttack
//
void A_TroopAttack(mobj_t * actor)
{
	int damage;

	if (!actor->target)
		return;

	A_FaceTarget(actor);
	if (P_CheckMeleeRange(actor))
	{
		S_StartAttackSound(actor, sfx_claw);
		damage = (P_Random() % 8 + 1) * 3;
		P_DamageMobj(actor->target, actor, actor, damage);
		return;
	}

	// launch a missile
	P_SpawnMissile(actor, actor->target, MT_TROOPSHOT);
}

void A_SargAttack(mobj_t * actor)
{
	int damage;

	if (!actor->target)
		return;

	A_FaceTarget(actor);
	if (P_CheckMeleeRange(actor))
	{
		damage = ((P_Random() % 10) + 1) * 4;
		P_DamageMobj(actor->target, actor, actor, damage);
	}
}

void A_HeadAttack(mobj_t * actor)
{
	int damage;

	if (!actor->target)
		return;

	A_FaceTarget(actor);
	if (P_CheckMeleeRange(actor))
	{
		damage = (P_Random() % 6 + 1) * 10;
		P_DamageMobj(actor->target, actor, actor, damage);
		return;
	}

	// launch a missile
	P_SpawnMissile(actor, actor->target, MT_HEADSHOT);
}

void A_CyberAttack(mobj_t * actor)
{
	if (!actor->target)
		return;

	A_FaceTarget(actor);
	P_SpawnMissile(actor, actor->target, MT_ROCKET);
}

void A_BruisAttack(mobj_t * actor)
{
	int damage;

	if (!actor->target)
		return;

	if (P_CheckMeleeRange(actor))
	{
		S_StartAttackSound(actor, sfx_claw);
		damage = (P_Random() % 8 + 1) * 10;
		P_DamageMobj(actor->target, actor, actor, damage);
		return;
	}

	// launch a missile
	P_SpawnMissile(actor, actor->target, MT_BRUISERSHOT);
}

//
// A_SkelMissile
//
void A_SkelMissile(mobj_t * actor)
{
	mobj_t *mo;

	if (!actor->target)
		return;

	A_FaceTarget(actor);
	actor->z += 16 * FRACUNIT;	// so missile spawns higher
	mo = P_SpawnMissile(actor, actor->target, MT_TRACER);
	actor->z -= 16 * FRACUNIT;	// back to normal

	if (mo)
	{
		mo->x += mo->momx;
		mo->y += mo->momy;
		mo->tracer = actor->target;
	}
}

int TRACEANGLE = 0xc000000;

void A_Tracer(mobj_t * actor)
{
	angle_t exact;
	fixed_t dist;
	fixed_t slope;
	mobj_t *dest;
	mobj_t *th;

	// TODO: Demo Desync!
	if (gametic % (4 * NEWTICRATERATIO))
		return;

	// spawn a puff of smoke behind the rocket
	PuffType = MT_PUFF;
	P_SpawnPuff(actor->x, actor->y, actor->z);

	th = P_SpawnMobj(actor->x - actor->momx, actor->y - actor->momy, actor->z, MT_SMOKE);

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
	actor->momx = FixedMul(actor->info->speed, finecosine[exact]);
	actor->momy = FixedMul(actor->info->speed, finesine[exact]);

	// change slope
	dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);

	dist = dist / actor->info->speed;

	if (dist < 1)
		dist = 1;
	slope = (dest->z + 40 * FRACUNIT - actor->z) / dist;

	if (slope < actor->momz)
		actor->momz -= FRACUNIT / 8;
	else
		actor->momz += FRACUNIT / 8;
}

void A_SkelWhoosh(mobj_t * actor)
{
	if (!actor->target)
		return;
	A_FaceTarget(actor);
	// judgecutor:
	// CHECK ME!
	S_StartAttackSound(actor, sfx_skeswg);
}

void A_SkelFist(mobj_t * actor)
{
	int damage;

	if (!actor->target)
		return;

	A_FaceTarget(actor);

	if (P_CheckMeleeRange(actor))
	{
		damage = ((P_Random() % 10) + 1) * 6;
		S_StartAttackSound(actor, sfx_skepch);
		P_DamageMobj(actor->target, actor, actor, damage);
	}
}

//
// PIT_VileCheck
// Detect a corpse that could be raised.
//
mobj_t *corpsehit;
mobj_t *vileobj;
fixed_t viletryx;
fixed_t viletryy;

boolean PIT_VileCheck(mobj_t * thing)
{
	int maxdist;
	boolean check;

	if (!(thing->flags & MF_CORPSE))
		return true;			// not a monster

	if (thing->tics != -1)
		return true;			// not lying still yet

	if (thing->info->raisestate == S_NULL)
		return true;			// monster doesn't have a raise state

	maxdist = thing->info->radius + mobjinfo[MT_VILE].radius;

	if (abs(thing->x - viletryx) > maxdist || abs(thing->y - viletryy) > maxdist)
		return true;			// not actually touching

	corpsehit = thing;
	corpsehit->momx = corpsehit->momy = 0;
	corpsehit->height <<= 2;
	check = P_CheckPosition(corpsehit, corpsehit->x, corpsehit->y);
	corpsehit->height >>= 2;

	if (!check)
		return true;			// doesn't fit here

	return false;				// got one, so stop checking
}

//
// A_VileChase
// Check for ressurecting a body
//
void A_VileChase(mobj_t * actor)
{
	int xl;
	int xh;
	int yl;
	int yh;

	int bx;
	int by;

	mobjinfo_t *info;
	mobj_t *temp;

	if (actor->movedir != DI_NODIR)
	{
		// check for corpses to raise
		viletryx = actor->x + actor->info->speed * xspeed[actor->movedir];
		viletryy = actor->y + actor->info->speed * yspeed[actor->movedir];

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
				if (!P_BlockThingsIterator(bx, by, PIT_VileCheck))
				{
					// got one!
					temp = actor->target;
					actor->target = corpsehit;
					A_FaceTarget(actor);
					actor->target = temp;

					P_SetMobjState(actor, S_VILE_HEAL1);
					S_StartSound(corpsehit, sfx_slop);
					info = corpsehit->info;

					P_SetMobjState(corpsehit, info->raisestate);
					if (demoversion < 129)
						corpsehit->height <<= 2;
					else
					{
						corpsehit->height = info->height;
						corpsehit->radius = info->radius;
					}
					corpsehit->flags = info->flags;
					corpsehit->health = info->spawnhealth;
					corpsehit->target = NULL;

					return;
				}
			}
		}
	}

	// Return to normal attack.
	A_Chase(actor);
}

//
// A_VileStart
//
void A_VileStart(mobj_t * actor)
{
	S_StartAttackSound(actor, sfx_vilatk);
}

//
// A_Fire
// Keep fire in front of player unless out of sight
//
void A_Fire(mobj_t * actor);

void A_StartFire(mobj_t * actor)
{
	S_StartSound(actor, sfx_flamst);
	A_Fire(actor);
}

void A_FireCrackle(mobj_t * actor)
{
	S_StartSound(actor, sfx_flame);
	A_Fire(actor);
}

void A_Fire(mobj_t * actor)
{
	mobj_t *dest;
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
void A_VileTarget(mobj_t * actor)
{
	mobj_t *fog;

	if (!actor->target)
		return;

	A_FaceTarget(actor);

	fog = P_SpawnMobj(actor->target->x, actor->target->x,	// Bp: shoul'nt be y ?
					  actor->target->z, MT_FIRE);

	actor->tracer = fog;
	fog->target = actor;
	fog->tracer = actor->target;
	A_Fire(fog);
}

//
// A_VileAttack
//
void A_VileAttack(mobj_t * actor)
{
	mobj_t *fire;
	int an;

	if (!actor->target)
		return;

	A_FaceTarget(actor);

	if (!P_CheckSight(actor, actor->target))
		return;

	S_StartSound(actor, sfx_barexp);
	P_DamageMobj(actor->target, actor, actor, 20);
	actor->target->momz = 1000 * FRACUNIT / actor->target->info->mass;

	an = actor->angle >> ANGLETOFINESHIFT;

	fire = actor->tracer;

	if (!fire)
		return;

	// move the fire between the vile and the player
	fire->x = actor->target->x - FixedMul(24 * FRACUNIT, finecosine[an]);
	fire->y = actor->target->y - FixedMul(24 * FRACUNIT, finesine[an]);
	P_RadiusAttack(fire, actor, 70);
}

//
// Mancubus attack,
// firing three missiles (bruisers)
// in three different directions?
// Doesn't look like it.
//
#define FATSPREAD       (ANG90/8)

void A_FatRaise(mobj_t * actor)
{
	A_FaceTarget(actor);
	S_StartAttackSound(actor, sfx_manatk);
}

void A_FatAttack1(mobj_t * actor)
{
	mobj_t *mo;
	int an;

	A_FaceTarget(actor);
	// Change direction  to ...
	actor->angle += FATSPREAD;
	P_SpawnMissile(actor, actor->target, MT_FATSHOT);

	mo = P_SpawnMissile(actor, actor->target, MT_FATSHOT);
	if (mo)
	{
		mo->angle += FATSPREAD;
		an = mo->angle >> ANGLETOFINESHIFT;
		mo->momx = FixedMul(mo->info->speed, finecosine[an]);
		mo->momy = FixedMul(mo->info->speed, finesine[an]);
	}
}

void A_FatAttack2(mobj_t * actor)
{
	mobj_t *mo;
	int an;

	A_FaceTarget(actor);
	// Now here choose opposite deviation.
	actor->angle -= FATSPREAD;
	P_SpawnMissile(actor, actor->target, MT_FATSHOT);

	mo = P_SpawnMissile(actor, actor->target, MT_FATSHOT);
	if (mo)
	{
		mo->angle -= FATSPREAD * 2;
		an = mo->angle >> ANGLETOFINESHIFT;
		mo->momx = FixedMul(mo->info->speed, finecosine[an]);
		mo->momy = FixedMul(mo->info->speed, finesine[an]);
	}
}

void A_FatAttack3(mobj_t * actor)
{
	mobj_t *mo;
	int an;

	A_FaceTarget(actor);

	mo = P_SpawnMissile(actor, actor->target, MT_FATSHOT);
	if (mo)
	{
		mo->angle -= FATSPREAD / 2;
		an = mo->angle >> ANGLETOFINESHIFT;
		mo->momx = FixedMul(mo->info->speed, finecosine[an]);
		mo->momy = FixedMul(mo->info->speed, finesine[an]);
	}

	mo = P_SpawnMissile(actor, actor->target, MT_FATSHOT);
	if (mo)
	{
		mo->angle += FATSPREAD / 2;
		an = mo->angle >> ANGLETOFINESHIFT;
		mo->momx = FixedMul(mo->info->speed, finecosine[an]);
		mo->momy = FixedMul(mo->info->speed, finesine[an]);
	}
}

//
// SkullAttack
// Fly at the player like a missile.
//
#define SKULLSPEED              (20*FRACUNIT)

void A_SkullAttack(mobj_t * actor)
{
	mobj_t *dest;
	angle_t an;
	int dist;

	if (!actor->target)
		return;

	dest = actor->target;
	actor->flags |= MF_SKULLFLY;
	S_StartScreamSound(actor, actor->info->attacksound);
	A_FaceTarget(actor);

	if (cv_predictingmonsters.value)	//added by AC for predmonsters
	{

		boolean canHit;
		fixed_t px, py, pz;
		int t, time;
		subsector_t *sec;

		dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);
		time = dist / SKULLSPEED;
		time = P_AproxDistance(dest->x + dest->momx * time - actor->x,
							   dest->y + dest->momy * time - actor->y) / SKULLSPEED;

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
			if (gamemode == heretic)
				an += P_SignedRandom() << 21;
			else
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

//
// A_PainShootSkull
// Spawn a lost soul and launch it at the target
//
void A_PainShootSkull(mobj_t * actor, angle_t angle)
{
	fixed_t x;
	fixed_t y;
	fixed_t z;

	mobj_t *newmobj;
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
            && ((mobj_t *)currentthinker)->type == MT_SKULL)
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

	prestep = 4 * FRACUNIT + 3 * (actor->info->radius + mobjinfo[MT_SKULL].radius) / 2;

	x = actor->x + FixedMul(prestep, finecosine[an]);
	y = actor->y + FixedMul(prestep, finesine[an]);
	z = actor->z + 8 * FRACUNIT;

	newmobj = P_SpawnMobj(x, y, z, MT_SKULL);

	// Check for movements.
	if (!P_TryMove(newmobj, newmobj->x, newmobj->y, false))
	{
		// kill it immediately
		P_DamageMobj(newmobj, actor, actor, 10000);
		return;
	}

	newmobj->target = actor->target;
	A_SkullAttack(newmobj);
}

//
// A_PainAttack
// Spawn a lost soul and launch it at the target
//
void A_PainAttack(mobj_t * actor)
{
	if (!actor->target)
		return;

	A_FaceTarget(actor);
	A_PainShootSkull(actor, actor->angle);
}

void A_PainDie(mobj_t * actor)
{
	A_Fall(actor);
	A_PainShootSkull(actor, actor->angle + ANG90);
	A_PainShootSkull(actor, actor->angle + ANG180);
	A_PainShootSkull(actor, actor->angle + ANG270);
}

void A_Scream(mobj_t * actor)
{
	int sound;

	switch (actor->info->deathsound)
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
			sound = actor->info->deathsound;
			break;
	}

	// Check for bosses.
	if (actor->type == MT_SPIDER || actor->type == MT_CYBORG)
	{
		// full volume
		S_StartSound(NULL, sound);
	}
	else
		S_StartScreamSound(actor, sound);
}

void A_XScream(mobj_t * actor)
{
	S_StartScreamSound(actor, sfx_slop);
}

void A_Pain(mobj_t * actor)
{
	if (actor->info->painsound)
		S_StartScreamSound(actor, actor->info->painsound);
}

//
//  A dying thing falls to the ground (monster deaths)
//
void A_Fall(mobj_t * actor)
{
	// actor is on ground, it can be walked over
	if (!cv_solidcorpse.value)
		actor->flags &= ~MF_SOLID;
	if (demoversion >= 131)
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
void A_Explode(mobj_t * actor)
{
	int damage = 128;

	switch (actor->type)
	{
		case MT_FIREBOMB:		// Time Bombs
			actor->z += 32 * FRACUNIT;
			actor->flags &= ~MF_SHADOW;
			break;
		case MT_MNTRFX2:		// Minotaur floor fire
			damage = 24;
			break;
		case MT_SOR2FX1:		// D'Sparil missile
			damage = 80 + (P_Random() & 31);
			break;
		default:
			break;
	}

	P_RadiusAttack(actor, actor->target, damage);
	P_HitFloor(actor);
}

static state_t *P_FinalState(statenum_t state)
{
	static char final_state[NUMSTATES];	//Hurdler: quick temporary hack to fix hacx freeze

	memset(final_state, 0, NUMSTATES);
	while (states[state].tics != -1)
	{
		final_state[state] = 1;
		state = states[state].nextstate;
		if (final_state[state])
			return NULL;
	}

	return &states[state];
}

//
// A_BossDeath
// Possibly trigger special effects
// if on first boss level
//
void A_BossDeath(mobj_t * mo)
{
	thinker_t *th;
	mobj_t *mo2;
	line_t junk;
	int i;

	if (gamemode == commercial)
	{
		if ((mo->type != MT_FATSO) && (mo->type != MT_BABY) && (mo->type != MT_KEEN))
			return;
	}
	else
	{
		switch (gameepisode)
		{
			case 1:
				if (gamemap != 8)
					return;

				if (mo->type != MT_BRUISER)
					return;
				break;

			case 2:
				if (gamemap != 8)
					return;

				if (mo->type != MT_CYBORG)
					return;
				break;

			case 3:
				if (gamemap != 8)
					return;

				if (mo->type != MT_SPIDER)
					return;

				break;

			case 4:
				switch (gamemap)
				{
					case 6:
						if (mo->type != MT_CYBORG)
							return;
						break;

					case 8:
						if (mo->type != MT_SPIDER)
							return;
						break;

					default:
						return;
						break;
				}
				break;

			default:
				if (gamemap != 8)
					return;
				break;
		}

	}

	// make sure there is a player alive for victory
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i] && players[i].health > 0)
			break;

	if (i == MAXPLAYERS)
		return;					// no one left alive, so do not end game

	// scan the remaining thinkers to see
	// if all bosses are dead
	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if (th->function.acp1 != (actionf_p1) P_MobjThinker)
			continue;

		mo2 = (mobj_t *) th;
		if (mo2 != mo && mo2->type == mo->type
			/*&& mo2->health > 0 */// the old one (doom original 1.9)
			&& mo2->state != P_FinalState(mo->info->deathstate))
		{
			// other boss not dead
			return;
		}
	}

	// victory!
	if (gamemode == commercial)
	{
		if (mo->type == MT_FATSO)
		{
			if (gamemap == 7)
			{
				junk.tag = 666;
				EV_DoFloor(&junk, lowerFloorToLowest);
			}
			return;
		}
		if (mo->type == MT_BABY)
		{
			if (gamemap == 7)
			{
				junk.tag = 667;
				EV_DoFloor(&junk, raiseToTexture);
			}
			return;
		}
		else if (mo->type == MT_KEEN)
		{
			junk.tag = 666;
			EV_DoDoor(&junk, dooropen, VDOORSPEED);
			return;
		}
	}
	else
	{
		switch (gameepisode)
		{
			case 1:
				junk.tag = 666;
				EV_DoFloor(&junk, lowerFloorToLowest);
				return;
				break;

			case 4:
				switch (gamemap)
				{
					case 6:
						junk.tag = 666;
						EV_DoDoor(&junk, blazeOpen, 4 * VDOORSPEED);
						return;
						break;

					case 8:
						junk.tag = 666;
						EV_DoFloor(&junk, lowerFloorToLowest);
						return;
						break;
				}
		}
	}
	if (cv_allowexitlevel.value)
		G_ExitLevel();
}

//
// A_KeenDie
// DOOM II special, map 32.
// Uses special tag 666.
//
void A_KeenDie(mobj_t * mo)
{
	A_Fall(mo);

	A_BossDeath(mo);
}

void A_Hoof(mobj_t * mo)
{
	S_StartSound(mo, sfx_hoof);
	A_Chase(mo);
}

void A_Metal(mobj_t * mo)
{
	S_StartSound(mo, sfx_metal);
	A_Chase(mo);
}

void A_BabyMetal(mobj_t * mo)
{
	S_StartSound(mo, sfx_bspwlk);
	A_Chase(mo);
}

void A_OpenShotgun2(player_t * player, pspdef_t * psp)
{
	S_StartAttackSound(player->mo, sfx_dbopn);
}

void A_LoadShotgun2(player_t * player, pspdef_t * psp)
{
	S_StartAttackSound(player->mo, sfx_dbload);
}

void A_ReFire(player_t * player, pspdef_t * psp);

void A_CloseShotgun2(player_t * player, pspdef_t * psp)
{
	S_StartAttackSound(player->mo, sfx_dbcls);
	A_ReFire(player, psp);
}

static mobj_t *braintargets[32];
static int numbraintargets;
static int braintargeton;

void P_InitBrainTarget()
{
	thinker_t *thinker;
	mobj_t *m;

	// find all the target spots
	numbraintargets = 0;
	braintargeton = 0;

	thinker = thinkercap.next;
	for (thinker = thinkercap.next; thinker != &thinkercap; thinker = thinker->next)
	{
		if (thinker->function.acp1 != (actionf_p1) P_MobjThinker)
			continue;			// not a mobj

		m = (mobj_t *) thinker;

		if (m->type == MT_BOSSTARGET)
		{
			braintargets[numbraintargets] = m;
			numbraintargets++;
		}
	}
}

void A_BrainAwake(mobj_t * mo)
{
	S_StartSound(NULL, sfx_bossit);
}

void A_BrainPain(mobj_t * mo)
{
	S_StartSound(NULL, sfx_bospn);
}

void A_BrainScream(mobj_t * mo)
{
	int x;
	int y;
	int z;
	mobj_t *th;

	for (x = mo->x - 196 * FRACUNIT; x < mo->x + 320 * FRACUNIT; x += FRACUNIT * 8)
	{
		y = mo->y - 320 * FRACUNIT;
		z = 128 + P_Random() * 2 * FRACUNIT;
		th = P_SpawnMobj(x, y, z, MT_ROCKET);
		th->momz = P_Random() * 512;

		P_SetMobjState(th, S_BRAINEXPLODE1);

		th->tics -= P_Random() & 7;
		if (th->tics < 1)
			th->tics = 1;
	}

	S_StartSound(NULL, sfx_bosdth);
}

void A_BrainExplode(mobj_t * mo)
{
	int x;
	int y;
	int z;
	mobj_t *th;

	x = (P_SignedRandom() << 11) + mo->x;
	y = mo->y;
	z = 128 + P_Random() * 2 * FRACUNIT;
	th = P_SpawnMobj(x, y, z, MT_ROCKET);
	th->momz = P_Random() * 512;

	P_SetMobjState(th, S_BRAINEXPLODE1);

	th->tics -= P_Random() & 7;
	if (th->tics < 1)
		th->tics = 1;
}

void A_BrainDie(mobj_t * mo)
{
	if (cv_allowexitlevel.value)
		G_ExitLevel();
}

void A_BrainSpit(mobj_t * mo)
{
	mobj_t *targ;
	mobj_t *newmobj;

	static int easy = 0;

	easy ^= 1;
	if (gameskill <= sk_easy && (!easy))
		return;

	if (numbraintargets > 0)
	{
		// shoot a cube at current target
		targ = braintargets[braintargeton];
		braintargeton = (braintargeton + 1) % numbraintargets;

		// spawn brain missile
		newmobj = P_SpawnMissile(mo, targ, MT_SPAWNSHOT);
		if (newmobj)
		{
			newmobj->target = targ;
			newmobj->reactiontime = ((targ->y - mo->y) / newmobj->momy) / newmobj->state->tics;
		}

		S_StartSound(NULL, sfx_bospit);
	}
}

void A_SpawnFly(mobj_t * mo);

// travelling cube sound
void A_SpawnSound(mobj_t * mo)
{
	S_StartSound(mo, sfx_boscub);
	A_SpawnFly(mo);
}

void A_SpawnFly(mobj_t * mo)
{
	mobj_t *newmobj;
	mobj_t *fog;
	mobj_t *targ;
	int r;
	mobjtype_t type;

	if (--mo->reactiontime)
		return;					// still flying

	targ = mo->target;

	// First spawn teleport fog.
	fog = P_SpawnMobj(targ->x, targ->y, targ->z, MT_SPAWNFIRE);
	S_StartSound(fog, sfx_telept);

	// Randomly select monster to spawn.
	r = P_Random();

	// Probability distribution (kind of :),
	// decreasing likelihood.
	if (r < 50)
		type = MT_TROOP;
	else if (r < 90)
		type = MT_SERGEANT;
	else if (r < 120)
		type = MT_SHADOWS;
	else if (r < 130)
		type = MT_PAIN;
	else if (r < 160)
		type = MT_HEAD;
	else if (r < 162)
		type = MT_VILE;
	else if (r < 172)
		type = MT_UNDEAD;
	else if (r < 192)
		type = MT_BABY;
	else if (r < 222)
		type = MT_FATSO;
	else if (r < 246)
		type = MT_KNIGHT;
	else
		type = MT_BRUISER;

	newmobj = P_SpawnMobj(targ->x, targ->y, targ->z, type);
	if (P_LookForPlayers(newmobj, true))
		P_SetMobjState(newmobj, newmobj->info->seestate);

	// telefrag anything in this spot
	P_TeleportMove(newmobj, newmobj->x, newmobj->y);

	// remove self (i.e., cube).
	P_RemoveMobj(mo);
}

void A_PlayerScream(mobj_t * mo)
{
	// Default death sound.
	int sound = sfx_pldeth;

	if ((gamemode == commercial) && (mo->health < -50))
	{
		// IF THE PLAYER DIES
		// LESS THAN -50% WITHOUT GIBBING
		sound = sfx_pdiehi;
	}
	S_StartScreamSound(mo, sound);
}

// Macros

#define MAX_BOSS_SPOTS 8

// Types

typedef struct
{
	fixed_t x;
	fixed_t y;
	angle_t angle;
} BossSpot_t;

// Private Data

static int BossSpotCount;
static BossSpot_t BossSpots[MAX_BOSS_SPOTS];

// Proto

//----------------------------------------------------------------------------
//
// PROC P_InitMonsters
//
// Called at level load.
//
//----------------------------------------------------------------------------

void P_InitMonsters(void)
{
	BossSpotCount = 0;
}

//----------------------------------------------------------------------------
//
// PROC P_AddBossSpot
//
//----------------------------------------------------------------------------

void P_AddBossSpot(fixed_t x, fixed_t y, angle_t angle)
{
	if (BossSpotCount == MAX_BOSS_SPOTS)
	{
		// BP:not a critical problem 
		CONS_Printf("Too many boss spots.");
		return;
	}
	BossSpots[BossSpotCount].x = x;
	BossSpots[BossSpotCount].y = y;
	BossSpots[BossSpotCount].angle = angle;
	BossSpotCount++;
}

//---------------------------------------------------------------------------
//
// FUNC P_LookForMonsters
//
//---------------------------------------------------------------------------

#define MONS_LOOK_RANGE (20*64*FRACUNIT)
#define MONS_LOOK_LIMIT 64

boolean P_LookForMonsters(mobj_t * actor)
{
	int count;
	mobj_t *mo;
	thinker_t *think;

	if (!P_CheckSight(players[0].mo, actor))
	{							// Player can't see monster
		return (false);
	}
	count = 0;
	for (think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if (think->function.acp1 != (actionf_p1) P_MobjThinker)
		{						// Not a mobj thinker
			continue;
		}
		mo = (mobj_t *) think;
		if (!(mo->flags & MF_COUNTKILL) || (mo == actor) || (mo->health <= 0))
		{						// Not a valid monster
			continue;
		}
		if (P_AproxDistance(actor->x - mo->x, actor->y - mo->y) > MONS_LOOK_RANGE)
		{						// Out of range
			continue;
		}
		if (P_Random() < 16)
		{						// Skip
			continue;
		}
		if (count++ > MONS_LOOK_LIMIT)
		{						// Stop searching
			return (false);
		}
		if (!P_CheckSight(actor, mo))
		{						// Out of sight
			continue;
		}
		// Found a target monster
		actor->target = mo;
		return (true);
	}
	return (false);
}

/*
===============================================================================

                                                ACTION ROUTINES

===============================================================================
*/

//----------------------------------------------------------------------------
//
// PROC A_DripBlood
//
//----------------------------------------------------------------------------

void A_DripBlood(mobj_t * actor)
{
	mobj_t *mo;
	int r, s;

	// evaluation order isn't define in C
	r = P_SignedRandom();
	s = P_SignedRandom();
	mo = P_SpawnMobj(actor->x + (r << 11), actor->y + (s << 11), actor->z, MT_BLOOD);
	mo->momx = P_SignedRandom() << 10;
	mo->momy = P_SignedRandom() << 10;
	mo->flags2 |= MF2_LOGRAV;
}

//----------------------------------------------------------------------------
//
// PROC A_KnightAttack
//
//----------------------------------------------------------------------------

void A_KnightAttack(mobj_t * actor)
{
	if (!actor->target)
	{
		return;
	}
	if (P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(3));
		S_StartSound(actor, sfx_kgtat2);
		return;
	}
	// Throw axe
	S_StartSound(actor, actor->info->attacksound);
	if (actor->type == MT_KNIGHTGHOST || P_Random() < 40)
	{							// Red axe
		P_SpawnMissile(actor, actor->target, MT_REDAXE);
		return;
	}
	// Green axe
	P_SpawnMissile(actor, actor->target, MT_KNIGHTAXE);
}

//----------------------------------------------------------------------------
//
// PROC A_ImpExplode
//
//----------------------------------------------------------------------------

void A_ImpExplode(mobj_t * actor)
{
	mobj_t *mo;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_IMPCHUNK1);
	mo->momx = P_SignedRandom() << 10;
	mo->momy = P_SignedRandom() << 10;
	mo->momz = 9 * FRACUNIT;
	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_IMPCHUNK2);
	mo->momx = P_SignedRandom() << 10;
	mo->momy = P_SignedRandom() << 10;
	mo->momz = 9 * FRACUNIT;
	if (actor->special1 == 666)
	{							// Extreme death crash
		P_SetMobjState(actor, S_IMP_XCRASH1);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_BeastPuff
//
//----------------------------------------------------------------------------

void A_BeastPuff(mobj_t * actor)
{
	if (P_Random() > 64)
	{
		int r, s, t;
		r = P_SignedRandom();
		s = P_SignedRandom();
		t = P_SignedRandom();

		P_SpawnMobj(actor->x + (r << 10), actor->y + (s << 10), actor->z + (t << 10), MT_PUFFY);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_ImpMeAttack
//
//----------------------------------------------------------------------------

void A_ImpMeAttack(mobj_t * actor)
{
	if (!actor->target)
	{
		return;
	}
	S_StartSound(actor, actor->info->attacksound);
	if (P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, 5 + (P_Random() & 7));
	}
}

//----------------------------------------------------------------------------
//
// PROC A_ImpMsAttack
//
//----------------------------------------------------------------------------

void A_ImpMsAttack(mobj_t * actor)
{
	mobj_t *dest;
	angle_t an;
	int dist;

	if (!actor->target || P_Random() > 64)
	{
		P_SetMobjState(actor, actor->info->seestate);
		return;
	}
	dest = actor->target;
	actor->flags |= MF_SKULLFLY;
	S_StartSound(actor, actor->info->attacksound);
	A_FaceTarget(actor);
	an = actor->angle >> ANGLETOFINESHIFT;
	actor->momx = FixedMul(12 * FRACUNIT, finecosine[an]);
	actor->momy = FixedMul(12 * FRACUNIT, finesine[an]);
	dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);
	dist = dist / (12 * FRACUNIT);
	if (dist < 1)
	{
		dist = 1;
	}
	actor->momz = (dest->z + (dest->height >> 1) - actor->z) / dist;
}

//----------------------------------------------------------------------------
//
// PROC A_ImpMsAttack2
//
// Fireball attack of the imp leader.
//
//----------------------------------------------------------------------------

void A_ImpMsAttack2(mobj_t * actor)
{
	if (!actor->target)
	{
		return;
	}
	S_StartSound(actor, actor->info->attacksound);
	if (P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, 5 + (P_Random() & 7));
		return;
	}
	P_SpawnMissile(actor, actor->target, MT_IMPBALL);
}

//----------------------------------------------------------------------------
//
// PROC A_ImpDeath
//
//----------------------------------------------------------------------------

void A_ImpDeath(mobj_t * actor)
{
	actor->flags &= ~MF_SOLID;
	actor->flags2 |= MF2_FOOTCLIP;

	actor->flags |= MF_CORPSE | MF_DROPOFF;
	actor->height >>= 2;
	actor->radius -= (actor->radius >> 4);	//for solid corpses

	if (actor->z <= actor->floorz)
	{
		P_SetMobjState(actor, S_IMP_CRASH1);
		actor->flags &= ~MF_CORPSE;
	}
}

//----------------------------------------------------------------------------
//
// PROC A_ImpXDeath1
//
//----------------------------------------------------------------------------

void A_ImpXDeath1(mobj_t * actor)
{
	actor->flags &= ~MF_SOLID;
	actor->flags |= MF_NOGRAVITY;
	actor->flags |= MF_CORPSE | MF_DROPOFF;
	actor->height >>= 2;
	actor->radius -= (actor->radius >> 4);	//for solid corpses

	actor->flags2 |= MF2_FOOTCLIP;
	actor->special1 = 666;		// Flag the crash routine
}

//----------------------------------------------------------------------------
//
// PROC A_ImpXDeath2
//
//----------------------------------------------------------------------------

void A_ImpXDeath2(mobj_t * actor)
{
	actor->flags &= ~MF_NOGRAVITY;
	if (actor->z <= actor->floorz)
	{
		P_SetMobjState(actor, S_IMP_CRASH1);
		actor->flags &= ~MF_CORPSE;
	}
}

//----------------------------------------------------------------------------
//
// FUNC P_UpdateChicken
//
// Returns true if the chicken morphs.
//
//----------------------------------------------------------------------------

#define TELEFOGHEIGHT (32*FRACUNIT)
boolean P_UpdateChicken(mobj_t * actor, int tics)
{
	mobj_t *fog;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	mobjtype_t moType;
	mobj_t *mo;
	mobj_t oldChicken;

	actor->special1 -= tics;
	if (actor->special1 > 0)
	{
		return (false);
	}
	moType = actor->special2;
	x = actor->x;
	y = actor->y;
	z = actor->z;
	oldChicken = *actor;
	P_SetMobjState(actor, S_FREETARGMOBJ);
	mo = P_SpawnMobj(x, y, z, moType);
	if (P_TestMobjLocation(mo) == false)
	{							// Didn't fit
		P_RemoveMobj(mo);
		mo = P_SpawnMobj(x, y, z, MT_CHICKEN);
		mo->angle = oldChicken.angle;
		mo->flags = oldChicken.flags;
		mo->health = oldChicken.health;
		mo->target = oldChicken.target;
		mo->special1 = 5 * TICRATE;	// Next try in 5 seconds
		mo->special2 = moType;
		return (false);
	}
	mo->angle = oldChicken.angle;
	mo->target = oldChicken.target;
	fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, MT_TFOG);
	S_StartSound(fog, sfx_telept);
	return (true);
}

//----------------------------------------------------------------------------
//
// PROC A_ChicAttack
//
//----------------------------------------------------------------------------

void A_ChicAttack(mobj_t * actor)
{
	if (P_UpdateChicken(actor, 18))
	{
		return;
	}
	if (!actor->target)
	{
		return;
	}
	if (P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, 1 + (P_Random() & 1));
	}
}

//----------------------------------------------------------------------------
//
// PROC A_ChicLook
//
//----------------------------------------------------------------------------

void A_ChicLook(mobj_t * actor)
{
	if (P_UpdateChicken(actor, 10))
	{
		return;
	}
	A_Look(actor);
}

//----------------------------------------------------------------------------
//
// PROC A_ChicChase
//
//----------------------------------------------------------------------------

void A_ChicChase(mobj_t * actor)
{
	if (P_UpdateChicken(actor, 3))
	{
		return;
	}
	A_Chase(actor);
}

//----------------------------------------------------------------------------
//
// PROC A_ChicPain
//
//----------------------------------------------------------------------------

void A_ChicPain(mobj_t * actor)
{
	if (P_UpdateChicken(actor, 10))
	{
		return;
	}
	S_StartSound(actor, actor->info->painsound);
}

//----------------------------------------------------------------------------
//
// PROC A_Feathers
//
//----------------------------------------------------------------------------

void A_Feathers(mobj_t * actor)
{
	int i;
	int count;
	mobj_t *mo;

	if (actor->health > 0)
	{							// Pain
		count = P_Random() < 32 ? 2 : 1;
	}
	else
	{							// Death
		count = 5 + (P_Random() & 3);
	}
	for (i = 0; i < count; i++)
	{
		mo = P_SpawnMobj(actor->x, actor->y, actor->z + 20 * FRACUNIT, MT_FEATHER);
		mo->target = actor;
		mo->momx = P_SignedRandom() << 8;
		mo->momy = P_SignedRandom() << 8;
		mo->momz = FRACUNIT + (P_Random() << 9);
		P_SetMobjState(mo, S_FEATHER1 + (P_Random() & 7));
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MummyAttack
//
//----------------------------------------------------------------------------

void A_MummyAttack(mobj_t * actor)
{
	if (!actor->target)
	{
		return;
	}
	S_StartSound(actor, actor->info->attacksound);
	if (P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(2));
		S_StartSound(actor, sfx_mumat2);
		return;
	}
	S_StartSound(actor, sfx_mumat1);
}

//----------------------------------------------------------------------------
//
// PROC A_MummyAttack2
//
// Mummy leader missile attack.
//
//----------------------------------------------------------------------------

void A_MummyAttack2(mobj_t * actor)
{
	mobj_t *mo;

	if (!actor->target)
	{
		return;
	}
	//S_StartSound(actor, actor->info->attacksound);
	if (P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(2));
		return;
	}
	mo = P_SpawnMissile(actor, actor->target, MT_MUMMYFX1);
	//mo = P_SpawnMissile(actor, actor->target, MT_EGGFX);
	if (mo != NULL)
	{
		mo->tracer = actor->target;
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MummyFX1Seek
//
//----------------------------------------------------------------------------

void A_MummyFX1Seek(mobj_t * actor)
{
	P_SeekerMissile(actor, ANGLE_1 * 10, ANGLE_1 * 20);
}

//----------------------------------------------------------------------------
//
// PROC A_MummySoul
//
//----------------------------------------------------------------------------

void A_MummySoul(mobj_t * mummy)
{
	mobj_t *mo;

	mo = P_SpawnMobj(mummy->x, mummy->y, mummy->z + 10 * FRACUNIT, MT_MUMMYSOUL);
	mo->momz = FRACUNIT;
}

//----------------------------------------------------------------------------
//
// PROC A_Sor1Pain
//
//----------------------------------------------------------------------------

void A_Sor1Pain(mobj_t * actor)
{
	actor->special1 = 20;		// Number of steps to walk fast
	A_Pain(actor);
}

//----------------------------------------------------------------------------
//
// PROC A_Sor1Chase
//
//----------------------------------------------------------------------------

void A_Sor1Chase(mobj_t * actor)
{
	if (actor->special1)
	{
		actor->special1--;
		actor->tics -= 3;
	}
	A_Chase(actor);
}

//----------------------------------------------------------------------------
//
// PROC A_Srcr1Attack
//
// Sorcerer demon attack.
//
//----------------------------------------------------------------------------

void A_Srcr1Attack(mobj_t * actor)
{
	mobj_t *mo;
	fixed_t momz;
	angle_t angle;

	if (!actor->target)
	{
		return;
	}
	S_StartSound(actor, actor->info->attacksound);
	if (P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(8));
		return;
	}
	if (actor->health > (actor->info->spawnhealth / 3) * 2)
	{							// Spit one fireball
		P_SpawnMissile(actor, actor->target, MT_SRCRFX1);
	}
	else
	{							// Spit three fireballs
		mo = P_SpawnMissile(actor, actor->target, MT_SRCRFX1);
		if (mo)
		{
			momz = mo->momz;
			angle = mo->angle;
			P_SpawnMissileAngle(actor, MT_SRCRFX1, angle - ANGLE_1 * 3, momz);
			P_SpawnMissileAngle(actor, MT_SRCRFX1, angle + ANGLE_1 * 3, momz);
		}
		if (actor->health < actor->info->spawnhealth / 3)
		{						// Maybe attack again
			if (actor->special1)
			{					// Just attacked, so don't attack again
				actor->special1 = 0;
			}
			else
			{					// Set state to attack again
				actor->special1 = 1;
				P_SetMobjState(actor, S_SRCR1_ATK4);
			}
		}
	}
}

//----------------------------------------------------------------------------
//
// PROC A_SorcererRise
//
//----------------------------------------------------------------------------

void A_SorcererRise(mobj_t * actor)
{
	mobj_t *mo;

	actor->flags &= ~MF_SOLID;
	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_SORCERER2);
	P_SetMobjState(mo, S_SOR2_RISE1);
	mo->angle = actor->angle;
	mo->target = actor->target;
}

//----------------------------------------------------------------------------
//
// PROC P_DSparilTeleport
//
//----------------------------------------------------------------------------

void P_DSparilTeleport(mobj_t * actor)
{
	int i;
	fixed_t x;
	fixed_t y;
	fixed_t prevX;
	fixed_t prevY;
	fixed_t prevZ;
	mobj_t *mo;

	if (!BossSpotCount)
	{							// No spots
		return;
	}
	i = P_Random();
	do
	{
		i++;
		x = BossSpots[i % BossSpotCount].x;
		y = BossSpots[i % BossSpotCount].y;
	}
	while (P_AproxDistance(actor->x - x, actor->y - y) < 128 * FRACUNIT);
	prevX = actor->x;
	prevY = actor->y;
	prevZ = actor->z;
	if (P_TeleportMove(actor, x, y))
	{
		mo = P_SpawnMobj(prevX, prevY, prevZ, MT_SOR2TELEFADE);
		S_StartSound(mo, sfx_telept);
		P_SetMobjState(actor, S_SOR2_TELE1);
		S_StartSound(actor, sfx_telept);
		actor->z = actor->floorz;
		actor->angle = BossSpots[i % BossSpotCount].angle;
		actor->momx = actor->momy = actor->momz = 0;
	}
}

//----------------------------------------------------------------------------
//
// PROC A_Srcr2Decide
//
//----------------------------------------------------------------------------

void A_Srcr2Decide(mobj_t * actor)
{
	static int chance[] = {
		192, 120, 120, 120, 64, 64, 32, 16, 0
	};

	if (!BossSpotCount)
	{							// No spots
		return;
	}
	if (P_Random() < chance[actor->health / (actor->info->spawnhealth / 8)])
	{
		P_DSparilTeleport(actor);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_Srcr2Attack
//
//----------------------------------------------------------------------------

void A_Srcr2Attack(mobj_t * actor)
{
	int chance;

	if (!actor->target)
	{
		return;
	}
	S_StartSound(NULL, actor->info->attacksound);
	if (P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(20));
		return;
	}
	chance = actor->health < actor->info->spawnhealth / 2 ? 96 : 48;
	if (P_Random() < chance)
	{							// Wizard spawners
		P_SpawnMissileAngle(actor, MT_SOR2FX2, actor->angle - ANG45, FRACUNIT / 2);
		P_SpawnMissileAngle(actor, MT_SOR2FX2, actor->angle + ANG45, FRACUNIT / 2);
	}
	else
	{							// Blue bolt
		P_SpawnMissile(actor, actor->target, MT_SOR2FX1);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_BlueSpark
//
//----------------------------------------------------------------------------

void A_BlueSpark(mobj_t * actor)
{
	int i;
	mobj_t *mo;

	for (i = 0; i < 2; i++)
	{
		mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_SOR2FXSPARK);
		mo->momx = P_SignedRandom() << 9;
		mo->momy = P_SignedRandom() << 9;
		mo->momz = FRACUNIT + (P_Random() << 8);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_GenWizard
//
//----------------------------------------------------------------------------

void A_GenWizard(mobj_t * actor)
{
	mobj_t *mo;
	mobj_t *fog;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z - mobjinfo[MT_WIZARD].height / 2, MT_WIZARD);
	if (P_TestMobjLocation(mo) == false)
	{							// Didn't fit
		P_RemoveMobj(mo);
		return;
	}
	actor->momx = actor->momy = actor->momz = 0;
	P_SetMobjState(actor, mobjinfo[actor->type].deathstate);
	actor->flags &= ~MF_MISSILE;
	fog = P_SpawnMobj(actor->x, actor->y, actor->z, MT_TFOG);
	S_StartSound(fog, sfx_telept);
}

//----------------------------------------------------------------------------
//
// PROC A_Sor2DthInit
//
//----------------------------------------------------------------------------

void A_Sor2DthInit(mobj_t * actor)
{
	actor->special1 = 7;		// Animation loop counter
	P_Massacre();				// Kill monsters early
}

//----------------------------------------------------------------------------
//
// PROC A_Sor2DthLoop
//
//----------------------------------------------------------------------------

void A_Sor2DthLoop(mobj_t * actor)
{
	if (--actor->special1)
	{							// Need to loop
		P_SetMobjState(actor, S_SOR2_DIE4);
	}
}

//----------------------------------------------------------------------------
//
// D'Sparil Sound Routines
//
//----------------------------------------------------------------------------

void A_SorZap(mobj_t * actor)
{
	S_StartSound(NULL, sfx_sorzap);
}
void A_SorRise(mobj_t * actor)
{
	S_StartSound(NULL, sfx_sorrise);
}
void A_SorDSph(mobj_t * actor)
{
	S_StartSound(NULL, sfx_sordsph);
}
void A_SorDExp(mobj_t * actor)
{
	S_StartSound(NULL, sfx_sordexp);
}
void A_SorDBon(mobj_t * actor)
{
	S_StartSound(NULL, sfx_sordbon);
}
void A_SorSightSnd(mobj_t * actor)
{
	S_StartSound(NULL, sfx_sorsit);
}

//----------------------------------------------------------------------------
//
// PROC A_MinotaurAtk1
//
// Melee attack.
//
//----------------------------------------------------------------------------

void A_MinotaurAtk1(mobj_t * actor)
{
	player_t *player;

	if (!actor->target)
	{
		return;
	}
	S_StartSound(actor, sfx_stfpow);
	if (P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(4));
		if ((player = actor->target->player) != NULL)
		{						// Squish the player
			player->deltaviewheight = -16 * FRACUNIT;
		}
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MinotaurDecide
//
// Choose a missile attack.
//
//----------------------------------------------------------------------------

#define MNTR_CHARGE_SPEED (13*FRACUNIT)

void A_MinotaurDecide(mobj_t * actor)
{
	angle_t angle;
	mobj_t *target;
	int dist;

	target = actor->target;
	if (!target)
	{
		return;
	}
	S_StartSound(actor, sfx_minsit);
	dist = P_AproxDistance(actor->x - target->x, actor->y - target->y);
	if (target->z + target->height > actor->z
		&& target->z + target->height < actor->z + actor->height
		&& dist < 8 * 64 * FRACUNIT && dist > 1 * 64 * FRACUNIT && P_Random() < 150)
	{							// Charge attack
		// Don't call the state function right away
		P_SetMobjStateNF(actor, S_MNTR_ATK4_1);
		actor->flags |= MF_SKULLFLY;
		A_FaceTarget(actor);
		angle = actor->angle >> ANGLETOFINESHIFT;
		actor->momx = FixedMul(MNTR_CHARGE_SPEED, finecosine[angle]);
		actor->momy = FixedMul(MNTR_CHARGE_SPEED, finesine[angle]);
		actor->special1 = TICRATE / 2;	// Charge duration
	}
	else if (target->z == target->floorz && dist < 9 * 64 * FRACUNIT && P_Random() < 220)
	{							// Floor fire attack
		P_SetMobjState(actor, S_MNTR_ATK3_1);
		actor->special2 = 0;
	}
	else
	{							// Swing attack
		A_FaceTarget(actor);
		// Don't need to call P_SetMobjState because the current state
		// falls through to the swing attack
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MinotaurCharge
//
//----------------------------------------------------------------------------

void A_MinotaurCharge(mobj_t * actor)
{
	mobj_t *puff;

	if (actor->special1)
	{
		puff = P_SpawnMobj(actor->x, actor->y, actor->z, MT_PHOENIXPUFF);
		puff->momz = 2 * FRACUNIT;
		actor->special1--;
	}
	else
	{
		actor->flags &= ~MF_SKULLFLY;
		P_SetMobjState(actor, actor->info->seestate);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MinotaurAtk2
//
// Swing attack.
//
//----------------------------------------------------------------------------

void A_MinotaurAtk2(mobj_t * actor)
{
	mobj_t *mo;
	angle_t angle;
	fixed_t momz;

	if (!actor->target)
	{
		return;
	}
	S_StartSound(actor, sfx_minat2);
	if (P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(5));
		return;
	}
	mo = P_SpawnMissile(actor, actor->target, MT_MNTRFX1);
	if (mo)
	{
		S_StartSound(mo, sfx_minat2);
		momz = mo->momz;
		angle = mo->angle;
		P_SpawnMissileAngle(actor, MT_MNTRFX1, angle - (ANG45 / 8), momz);
		P_SpawnMissileAngle(actor, MT_MNTRFX1, angle + (ANG45 / 8), momz);
		P_SpawnMissileAngle(actor, MT_MNTRFX1, angle - (ANG45 / 16), momz);
		P_SpawnMissileAngle(actor, MT_MNTRFX1, angle + (ANG45 / 16), momz);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MinotaurAtk3
//
// Floor fire attack.
//
//----------------------------------------------------------------------------

void A_MinotaurAtk3(mobj_t * actor)
{
	mobj_t *mo;
	player_t *player;

	if (!actor->target)
	{
		return;
	}
	if (P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(5));
		if ((player = actor->target->player) != NULL)
		{						// Squish the player
			player->deltaviewheight = -16 * FRACUNIT;
		}
	}
	else
	{
		mo = P_SpawnMissile(actor, actor->target, MT_MNTRFX2);
		if (mo != NULL)
		{
			S_StartSound(mo, sfx_minat1);
		}
	}
	if (P_Random() < 192 && actor->special2 == 0)
	{
		P_SetMobjState(actor, S_MNTR_ATK3_4);
		actor->special2 = 1;
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MntrFloorFire
//
//----------------------------------------------------------------------------

void A_MntrFloorFire(mobj_t * actor)
{
	mobj_t *mo;
	int r, s;

	actor->z = actor->floorz;
	r = P_SignedRandom();
	s = P_SignedRandom();
	mo = P_SpawnMobj(actor->x + (r << 10), actor->y + (s << 10), ONFLOORZ, MT_MNTRFX3);
	mo->target = actor->target;
	mo->momx = 1;				// Force block checking
	P_CheckMissileSpawn(mo);
}

//----------------------------------------------------------------------------
//
// PROC A_BeastAttack
//
//----------------------------------------------------------------------------

void A_BeastAttack(mobj_t * actor)
{
	if (!actor->target)
	{
		return;
	}
	S_StartSound(actor, actor->info->attacksound);
	if (P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(3));
		return;
	}
	P_SpawnMissile(actor, actor->target, MT_BEASTBALL);
}

//----------------------------------------------------------------------------
//
// PROC A_HeadAttack
//
//----------------------------------------------------------------------------

void A_HHeadAttack(mobj_t * actor)
{
	int i;
	mobj_t *fire;
	mobj_t *baseFire;
	mobj_t *mo;
	mobj_t *target;
	int randAttack;
	static int atkResolve1[] = { 50, 150 };
	static int atkResolve2[] = { 150, 200 };
	int dist;

	// Ice ball             (close 20% : far 60%)
	// Fire column  (close 40% : far 20%)
	// Whirlwind    (close 40% : far 20%)
	// Distance threshold = 8 cells

	target = actor->target;
	if (target == NULL)
	{
		return;
	}
	A_FaceTarget(actor);
	if (P_CheckMeleeRange(actor))
	{
		P_DamageMobj(target, actor, actor, HITDICE(6));
		return;
	}
	dist = P_AproxDistance(actor->x - target->x, actor->y - target->y) > 8 * 64 * FRACUNIT;
	randAttack = P_Random();
	if (randAttack < atkResolve1[dist])
	{							// Ice ball
		P_SpawnMissile(actor, target, MT_HEADFX1);
		S_StartSound(actor, sfx_hedat2);
	}
	else if (randAttack < atkResolve2[dist])
	{							// Fire column
		baseFire = P_SpawnMissile(actor, target, MT_HEADFX3);
		if (baseFire != NULL)
		{
			P_SetMobjState(baseFire, S_HHEADFX3_4);	// Don't grow
			for (i = 0; i < 5; i++)
			{
				fire = P_SpawnMobj(baseFire->x, baseFire->y, baseFire->z, MT_HEADFX3);
				if (i == 0)
				{
					S_StartSound(actor, sfx_hedat1);
				}
				fire->target = baseFire->target;
				fire->angle = baseFire->angle;
				fire->momx = baseFire->momx;
				fire->momy = baseFire->momy;
				fire->momz = baseFire->momz;
				//fire->damage = 0;
				fire->health = (i + 1) * 2;
				P_CheckMissileSpawn(fire);
			}
		}
	}
	else
	{							// Whirlwind
		mo = P_SpawnMissile(actor, target, MT_WHIRLWIND);
		if (mo != NULL)
		{
			mo->z -= 32 * FRACUNIT;
			mo->tracer = target;
			mo->special2 = 50;	// Timer for active sound
			mo->health = 20 * TICRATE;	// Duration
			S_StartSound(actor, sfx_hedat3);
		}
	}
}

//----------------------------------------------------------------------------
//
// PROC A_WhirlwindSeek
//
//----------------------------------------------------------------------------

void A_WhirlwindSeek(mobj_t * actor)
{
	actor->health -= 3;
	if (actor->health < 0)
	{
		actor->momx = actor->momy = actor->momz = 0;
		P_SetMobjState(actor, mobjinfo[actor->type].deathstate);
		actor->flags &= ~MF_MISSILE;
		return;
	}
	if ((actor->special2 -= 3) < 0)
	{
		actor->special2 = 58 + (P_Random() & 31);
		S_StartSound(actor, sfx_hedat3);
	}
	if (actor->tracer && actor->tracer->flags & MF_SHADOW)
	{
		return;
	}
	P_SeekerMissile(actor, ANGLE_1 * 10, ANGLE_1 * 30);
}

//----------------------------------------------------------------------------
//
// PROC A_HeadIceImpact
//
//----------------------------------------------------------------------------

void A_HeadIceImpact(mobj_t * ice)
{
	int i;
	angle_t angle;
	mobj_t *shard;

	for (i = 0; i < 8; i++)
	{
		shard = P_SpawnMobj(ice->x, ice->y, ice->z, MT_HEADFX2);
		angle = i * ANG45;
		shard->target = ice->target;
		shard->angle = angle;
		angle >>= ANGLETOFINESHIFT;
		shard->momx = FixedMul(shard->info->speed, finecosine[angle]);
		shard->momy = FixedMul(shard->info->speed, finesine[angle]);
		shard->momz = -.6 * FRACUNIT;
		P_CheckMissileSpawn(shard);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_HeadFireGrow
//
//----------------------------------------------------------------------------

void A_HeadFireGrow(mobj_t * fire)
{
	fire->health--;
	fire->z += 9 * FRACUNIT;
	if (fire->health == 0)
	{
		//fire->damage = fire->info->damage;
		P_SetMobjState(fire, S_HHEADFX3_4);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_SnakeAttack
//
//----------------------------------------------------------------------------

void A_SnakeAttack(mobj_t * actor)
{
	if (!actor->target)
	{
		P_SetMobjState(actor, S_SNAKE_WALK1);
		return;
	}
	S_StartSound(actor, actor->info->attacksound);
	A_FaceTarget(actor);
	P_SpawnMissile(actor, actor->target, MT_SNAKEPRO_A);
}

//----------------------------------------------------------------------------
//
// PROC A_SnakeAttack2
//
//----------------------------------------------------------------------------

void A_SnakeAttack2(mobj_t * actor)
{
	if (!actor->target)
	{
		P_SetMobjState(actor, S_SNAKE_WALK1);
		return;
	}
	S_StartSound(actor, actor->info->attacksound);
	A_FaceTarget(actor);
	P_SpawnMissile(actor, actor->target, MT_SNAKEPRO_B);
}

//----------------------------------------------------------------------------
//
// PROC A_ClinkAttack
//
//----------------------------------------------------------------------------

void A_ClinkAttack(mobj_t * actor)
{
	int damage;

	if (!actor->target)
	{
		return;
	}
	S_StartSound(actor, actor->info->attacksound);
	if (P_CheckMeleeRange(actor))
	{
		damage = ((P_Random() % 7) + 3);
		P_DamageMobj(actor->target, actor, actor, damage);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_GhostOff
//
//----------------------------------------------------------------------------

void A_GhostOff(mobj_t * actor)
{
	actor->flags &= ~MF_SHADOW;
}

//----------------------------------------------------------------------------
//
// PROC A_WizAtk1
//
//----------------------------------------------------------------------------

void A_WizAtk1(mobj_t * actor)
{
	A_FaceTarget(actor);
	actor->flags &= ~MF_SHADOW;
}

//----------------------------------------------------------------------------
//
// PROC A_WizAtk2
//
//----------------------------------------------------------------------------

void A_WizAtk2(mobj_t * actor)
{
	A_FaceTarget(actor);
	actor->flags |= MF_SHADOW;
}

//----------------------------------------------------------------------------
//
// PROC A_WizAtk3
//
//----------------------------------------------------------------------------

void A_WizAtk3(mobj_t * actor)
{
	mobj_t *mo;
	angle_t angle;
	fixed_t momz;

	actor->flags &= ~MF_SHADOW;
	if (!actor->target)
	{
		return;
	}
	S_StartSound(actor, actor->info->attacksound);
	if (P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(4));
		return;
	}
	mo = P_SpawnMissile(actor, actor->target, MT_WIZFX1);
	if (mo)
	{
		momz = mo->momz;
		angle = mo->angle;
		P_SpawnMissileAngle(actor, MT_WIZFX1, angle - (ANG45 / 8), momz);
		P_SpawnMissileAngle(actor, MT_WIZFX1, angle + (ANG45 / 8), momz);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_Scream
//
//----------------------------------------------------------------------------

void A_HScream(mobj_t * actor)
{
	switch (actor->type)
	{
		case MT_CHICPLAYER:
		case MT_SORCERER1:
		case MT_MINOTAUR:
			// Make boss death sounds full volume
			S_StartSound(NULL, actor->info->deathsound);
			break;
		case MT_PLAYER:
			// Handle the different player death screams
			if (actor->special1 < 10)
			{					// Wimpy death sound
				S_StartSound(actor, sfx_plrwdth);
			}
			else if (actor->health > -50)
			{					// Normal death sound
				S_StartSound(actor, actor->info->deathsound);
			}
			else if (actor->health > -100)
			{					// Crazy death sound
				S_StartSound(actor, sfx_plrcdth);
			}
			else
			{					// Extreme death sound
				S_StartSound(actor, sfx_gibdth);
			}
			break;
		default:
			S_StartSound(actor, actor->info->deathsound);
			break;
	}
}

//---------------------------------------------------------------------------
//
// PROC P_DropItem
//
//---------------------------------------------------------------------------

void P_DropItem(mobj_t * source, mobjtype_t type, int special, int chance)
{
	mobj_t *mo;

	if (P_Random() > chance)
	{
		return;
	}
	mo = P_SpawnMobj(source->x, source->y, source->z + (source->height >> 1), type);
	mo->momx = P_SignedRandom() << 8;
	mo->momy = P_SignedRandom() << 8;
	mo->momz = FRACUNIT * 5 + (P_Random() << 10);
	mo->flags |= MF_DROPPED;
	mo->health = special;
}

//----------------------------------------------------------------------------
//
// PROC A_NoBlocking
//
//----------------------------------------------------------------------------

void A_NoBlocking(mobj_t * actor)
{
	A_Fall(actor);
	// Check for monsters dropping things
	switch (actor->type)
	{
		case MT_MUMMY:
		case MT_MUMMYLEADER:
		case MT_MUMMYGHOST:
		case MT_MUMMYLEADERGHOST:
			P_DropItem(actor, MT_AMGWNDWIMPY, 3, 84);
			break;
		case MT_KNIGHT:
		case MT_KNIGHTGHOST:
			P_DropItem(actor, MT_AMCBOWWIMPY, 5, 84);
			break;
		case MT_WIZARD:
			P_DropItem(actor, MT_AMBLSRWIMPY, 10, 84);
			P_DropItem(actor, MT_ARTITOMEOFPOWER, 0, 4);
			break;
		case MT_HHEAD:
			P_DropItem(actor, MT_AMBLSRWIMPY, 10, 84);
			P_DropItem(actor, MT_ARTIEGG, 0, 51);
			break;
		case MT_BEAST:
			P_DropItem(actor, MT_AMCBOWWIMPY, 10, 84);
			break;
		case MT_CLINK:
			P_DropItem(actor, MT_AMSKRDWIMPY, 20, 84);
			break;
		case MT_SNAKE:
			P_DropItem(actor, MT_AMPHRDWIMPY, 5, 84);
			break;
		case MT_MINOTAUR:
			P_DropItem(actor, MT_ARTISUPERHEAL, 0, 51);
			P_DropItem(actor, MT_AMPHRDWIMPY, 10, 84);
			break;
		default:
			break;
	}
}

//----------------------------------------------------------------------------
//
// PROC A_PodPain
//
//----------------------------------------------------------------------------

void A_PodPain(mobj_t * actor)
{
	int i;
	int count;
	int chance;
	mobj_t *goo;

	chance = P_Random();
	if (chance < 128)
	{
		return;
	}
	count = chance > 240 ? 2 : 1;
	for (i = 0; i < count; i++)
	{
		goo = P_SpawnMobj(actor->x, actor->y, actor->z + 48 * FRACUNIT, MT_PODGOO);
		goo->target = actor;
		goo->momx = P_SignedRandom() << 9;
		goo->momy = P_SignedRandom() << 9;
		goo->momz = FRACUNIT / 2 + (P_Random() << 9);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_RemovePod
//
//----------------------------------------------------------------------------

void A_RemovePod(mobj_t * actor)
{
	mobj_t *mo;

	if (actor->special2)
	{
		mo = (mobj_t *) actor->special2;
		if (mo->special1 > 0)
		{
			mo->special1--;
		}
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MakePod
//
//----------------------------------------------------------------------------

#define MAX_GEN_PODS 16

void A_MakePod(mobj_t * actor)
{
	mobj_t *mo;
	fixed_t x;
	fixed_t y;
	fixed_t z;

	if (actor->special1 == MAX_GEN_PODS)
	{							// Too many generated pods
		return;
	}
	x = actor->x;
	y = actor->y;
	z = actor->z;
	mo = P_SpawnMobj(x, y, ONFLOORZ, MT_POD);
	if (P_CheckPosition(mo, x, y) == false)
	{							// Didn't fit
		P_RemoveMobj(mo);
		return;
	}
	P_SetMobjState(mo, S_POD_GROW1);
	P_ThrustMobj(mo, P_Random() << 24, (fixed_t) (4.5 * FRACUNIT));
	S_StartSound(mo, sfx_newpod);
	actor->special1++;			// Increment generated pod count
	mo->special2 = (int)actor;	// Link the generator to the pod
	return;
}

//----------------------------------------------------------------------------
//
// PROC P_Massacre
//
// Kills all monsters.
//
//----------------------------------------------------------------------------

void P_Massacre(void)
{
	mobj_t *mo;
	thinker_t *think;

	for (think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if (think->function.acp1 != (actionf_p1) P_MobjThinker)
		{						// Not a mobj thinker
			continue;
		}
		mo = (mobj_t *) think;
		if ((mo->flags & MF_COUNTKILL) && (mo->health > 0))
		{
			P_DamageMobj(mo, NULL, NULL, 10000);
		}
	}
}

//----------------------------------------------------------------------------
//
// PROC A_BossDeath
//
// Trigger special effects if all bosses are dead.
//
//----------------------------------------------------------------------------

void A_HBossDeath(mobj_t * actor)
{
	mobj_t *mo;
	thinker_t *think;
	line_t dummyLine;
	static const mobjtype_t bossType[6] = {
		MT_HHEAD,
		MT_MINOTAUR,
		MT_SORCERER2,
		MT_HHEAD,
		MT_MINOTAUR,
		-1
	};

	if (gamemap != 8)
	{							// Not a boss level
		return;
	}
	if (actor->type != bossType[gameepisode - 1])
	{							// Not considered a boss in this episode
		return;
	}
	// Make sure all bosses are dead
	for (think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if (think->function.acp1 != (actionf_p1) P_MobjThinker)
		{						// Not a mobj thinker
			continue;
		}
		mo = (mobj_t *) think;
		if ((mo != actor) && (mo->type == actor->type) && !(mo->flags & MF_CORPSE))
		{						// Found a living boss
			return;
		}
	}
	if (gameepisode > 1)
	{							// Kill any remaining monsters
		P_Massacre();
	}
	dummyLine.tag = 666;
	EV_DoFloor(&dummyLine, lowerFloor);
}

//----------------------------------------------------------------------------
//
// PROC A_ESound
//
//----------------------------------------------------------------------------

void A_ESound(mobj_t * mo)
{
	int sound;

	switch (mo->type)
	{
		case MT_SOUNDWATERFALL:
			sound = sfx_waterfl;
			break;
		case MT_SOUNDWIND:
			sound = sfx_wind;
			break;
		default:
			sound = sfx_None;
			break;
	}
	S_StartSound(mo, sound);
}

//----------------------------------------------------------------------------
//
// PROC A_SpawnTeleGlitter
//
//----------------------------------------------------------------------------

void A_SpawnTeleGlitter(mobj_t * actor)
{
	mobj_t *mo;
	int r, s;

	r = P_Random();
	s = P_Random();
	mo = P_SpawnMobj(actor->x + ((r & 31) - 16) * FRACUNIT,
					 actor->y + ((s & 31) - 16) * FRACUNIT,
					 actor->subsector->sector->floorheight, MT_TELEGLITTER);
	mo->momz = FRACUNIT / 4;
}

//----------------------------------------------------------------------------
//
// PROC A_SpawnTeleGlitter2
//
//----------------------------------------------------------------------------

void A_SpawnTeleGlitter2(mobj_t * actor)
{
	mobj_t *mo;
	int r, s;

	r = P_Random();
	s = P_Random();
	mo = P_SpawnMobj(actor->x + ((r & 31) - 16) * FRACUNIT,
					 actor->y + ((s & 31) - 16) * FRACUNIT,
					 actor->subsector->sector->floorheight, MT_TELEGLITTER2);
	mo->momz = FRACUNIT / 4;
}

//----------------------------------------------------------------------------
//
// PROC A_AccTeleGlitter
//
//----------------------------------------------------------------------------

void A_AccTeleGlitter(mobj_t * actor)
{
	if (++actor->health > 35)
	{
		actor->momz += actor->momz / 2;
	}
}

//----------------------------------------------------------------------------
//
// PROC A_InitKeyGizmo
//
//----------------------------------------------------------------------------

void A_InitKeyGizmo(mobj_t * gizmo)
{
	mobj_t *mo;
	statenum_t state;

	switch (gizmo->type)
	{
		case MT_KEYGIZMOBLUE:
			state = S_KGZ_BLUEFLOAT1;
			break;
		case MT_KEYGIZMOGREEN:
			state = S_KGZ_GREENFLOAT1;
			break;
		case MT_KEYGIZMOYELLOW:
			state = S_KGZ_YELLOWFLOAT1;
			break;
		default:
			state = S_NULL;
			break;
	}
	mo = P_SpawnMobj(gizmo->x, gizmo->y, gizmo->z + 60 * FRACUNIT, MT_KEYGIZMOFLOAT);
	P_SetMobjState(mo, state);
}

//----------------------------------------------------------------------------
//
// PROC A_VolcanoSet
//
//----------------------------------------------------------------------------

void A_VolcanoSet(mobj_t * volcano)
{
	volcano->tics = 105 + (P_Random() & 127);
}

//----------------------------------------------------------------------------
//
// PROC A_VolcanoBlast
//
//----------------------------------------------------------------------------

void A_VolcanoBlast(mobj_t * volcano)
{
	int i;
	int count;
	mobj_t *blast;
	angle_t angle;

	count = 1 + (P_Random() % 3);
	for (i = 0; i < count; i++)
	{
		blast = P_SpawnMobj(volcano->x, volcano->y, volcano->z + 44 * FRACUNIT, MT_VOLCANOBLAST);	// MT_VOLCANOBLAST
		blast->target = volcano;
		angle = P_Random() << 24;
		blast->angle = angle;
		angle >>= ANGLETOFINESHIFT;
		blast->momx = FixedMul(1 * FRACUNIT, finecosine[angle]);
		blast->momy = FixedMul(1 * FRACUNIT, finesine[angle]);
		blast->momz = (2.5 * FRACUNIT) + (P_Random() << 10);
		S_StartSound(blast, sfx_volsht);
		P_CheckMissileSpawn(blast);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_VolcBallImpact
//
//----------------------------------------------------------------------------

void A_VolcBallImpact(mobj_t * ball)
{
	int i;
	mobj_t *tiny;
	angle_t angle;

	if (ball->z <= ball->floorz)
	{
		ball->flags |= MF_NOGRAVITY;
		ball->flags2 &= ~MF2_LOGRAV;
		ball->z += 28 * FRACUNIT;
		//ball->momz = 3*FRACUNIT;
	}
	P_RadiusAttack(ball, ball->target, 25);
	for (i = 0; i < 4; i++)
	{
		tiny = P_SpawnMobj(ball->x, ball->y, ball->z, MT_VOLCANOTBLAST);
		tiny->target = ball;
		angle = i * ANG90;
		tiny->angle = angle;
		angle >>= ANGLETOFINESHIFT;
		tiny->momx = FixedMul(FRACUNIT * .7, finecosine[angle]);
		tiny->momy = FixedMul(FRACUNIT * .7, finesine[angle]);
		tiny->momz = FRACUNIT + (P_Random() << 9);
		P_CheckMissileSpawn(tiny);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_SkullPop
//
//----------------------------------------------------------------------------

void A_SkullPop(mobj_t * actor)
{
	mobj_t *mo;
	player_t *player;

	actor->flags &= ~MF_SOLID;
	mo = P_SpawnMobj(actor->x, actor->y, actor->z + 48 * FRACUNIT, MT_BLOODYSKULL);
	//mo->target = actor;
	mo->momx = P_SignedRandom() << 9;
	mo->momy = P_SignedRandom() << 9;
	mo->momz = FRACUNIT * 2 + (P_Random() << 6);
	// Attach player mobj to bloody skull
	player = actor->player;
	actor->player = NULL;
	mo->player = player;
	mo->health = actor->health;
	mo->angle = actor->angle;
	player->mo = mo;
	player->aiming = 0;
	player->damagecount = 32;
}

//----------------------------------------------------------------------------
//
// PROC A_CheckSkullFloor
//
//----------------------------------------------------------------------------

void A_CheckSkullFloor(mobj_t * actor)
{
	if (actor->z <= actor->floorz)
	{
		P_SetMobjState(actor, S_BLOODYSKULLX1);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_CheckSkullDone
//
//----------------------------------------------------------------------------

void A_CheckSkullDone(mobj_t * actor)
{
	if (actor->special2 == 666)
	{
		P_SetMobjState(actor, S_BLOODYSKULLX2);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_CheckBurnGone
//
//----------------------------------------------------------------------------

void A_CheckBurnGone(mobj_t * actor)
{
	if (actor->special2 == 666)
	{
		//P_SetMobjState(actor, S_PLAY_FDTH20);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_FreeTargMobj
//
//----------------------------------------------------------------------------

void A_FreeTargMobj(mobj_t * mo)
{
	mo->momx = mo->momy = mo->momz = 0;
	mo->z = mo->ceilingz + 4 * FRACUNIT;
	mo->flags &= ~(MF_SHOOTABLE | MF_FLOAT | MF_SKULLFLY | MF_SOLID);
	mo->flags |= MF_CORPSE | MF_DROPOFF | MF_NOGRAVITY;
	mo->flags2 &= ~(MF2_PASSMOBJ | MF2_LOGRAV);
	mo->player = NULL;
}

//----------------------------------------------------------------------------
//
// PROC A_AddPlayerCorpse
//
//----------------------------------------------------------------------------

/*#define BODYQUESIZE 32
mobj_t *bodyque[BODYQUESIZE];
int bodyqueslot;*/

void A_AddPlayerCorpse(mobj_t * actor)
{
	if (bodyqueslot >= BODYQUESIZE)
	{							// Too many player corpses - remove an old one
		P_RemoveMobj(bodyque[bodyqueslot % BODYQUESIZE]);
	}
	bodyque[bodyqueslot % BODYQUESIZE] = actor;
	bodyqueslot++;
}

//----------------------------------------------------------------------------
//
// PROC A_FlameSnd
//
//----------------------------------------------------------------------------

void A_FlameSnd(mobj_t * actor)
{
	S_StartSound(actor, sfx_hedat1);	// Burn sound
}

