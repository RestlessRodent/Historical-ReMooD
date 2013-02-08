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
//      Player related stuff.
//      Bobbing POV/weapon, movement.
//      Pending weapon.

#include "doomdef.h"

#include "g_game.h"
#include "p_local.h"
#include "r_main.h"
#include "s_sound.h"
#include "p_setup.h"
#include "p_inter.h"
#include "m_random.h"
#include "p_demcmp.h"
#include "d_main.h"

// Index of the special effects (INVUL inverse) map.
#define INVERSECOLORMAP         32

//
// Movement.
//

// 16 pixels of bob
#define MAXBOB  0x100000

bool_t onground;

/* P_Thrust() -- Moves the given origin along a given angle. */
void P_Thrust(player_t* player, angle_t angle, fixed_t move)
{
	fixed_t cX, cY;
	
	angle >>= ANGLETOFINESHIFT;
	
	if (!(player->mo->RXFlags[1] & MFREXB_NOHERETICFRICT) && player->mo->subsector && (player->mo->subsector->sector->special & REXS_HFRICTMASK) && player->mo->z <= player->mo->floorz)	// Friction_Low
	{
		cX = FixedMul(move >> 2, finecosine[angle]);
		cY = FixedMul(move >> 2, finesine[angle]);
	}
	else
	{
		cX = FixedMul(move, finecosine[angle]);
		cY = FixedMul(move, finesine[angle]);
	}
	
	/* Change momentum */
	player->MoveMom += move;
	player->mo->momx += cX;
	player->mo->momy += cY;
}

/* P_FakeThrust() -- Sets fake momentum */
void P_FakeThrust(player_t* player, angle_t angle, fixed_t move)
{
	fixed_t cX, cY;
	
	/* Use normal momentum */
	angle >>= ANGLETOFINESHIFT;
	cX = FixedMul(move, finecosine[angle]);
	cY = FixedMul(move, finesine[angle]);
	
	/* Change momentum */
	player->FakeMom[0] += cX;
	player->FakeMom[1] += cY;
}

//
// P_CalcHeight
// Calculate the walking / running height adjustment
//
void P_CalcHeight(player_t* player)
{
	fixed_t ViewHeight;
	fixed_t Diff;
	fixed_t bob;
	fixed_t AvgMomX, AvgMomY;
	int angle;
	subsector_t* SubS;
	
	/* Base */
	// Player
	if (player->mo->RXFlags[0] & MFREXA_ISPLAYEROBJECT)
		if (player->ProfileEx)
			ViewHeight = player->ProfileEx->ViewHeight;
		else
			ViewHeight = VIEWHEIGHT << FRACBITS;
	
	// Monster
	else	// height * (41 / 56)
		ViewHeight = FixedMul(player->mo->height, 47981);
	
	// Calculate bobbing
		// Middle
	if (!player->ProfileEx || (player->ProfileEx && player->ProfileEx->BobMode == 1))
	{
		AvgMomX = (player->FakeMom[0] + player->mo->momx) >> 1;
		AvgMomY = (player->FakeMom[1] + player->mo->momy) >> 1;
	}
		// Doom
	else if (player->ProfileEx && player->ProfileEx->BobMode == 0)
	{
		AvgMomX = player->mo->momx;
		AvgMomY = player->mo->momy;
	}
		// Effort
	else if (player->ProfileEx && player->ProfileEx->BobMode == 2)
	{
		AvgMomX = player->FakeMom[0];
		AvgMomY = player->FakeMom[1];
	}
		// No Bobbing
	else
	{
		AvgMomX = 0;
		AvgMomY = 0;
	}
	
	player->bob = ((FixedMul(AvgMomX, AvgMomX) + FixedMul(AvgMomY, AvgMomY))) >> 2;
	
	if (player->bob > MAXBOB)
		player->bob = MAXBOB;
		
	if (player->mo->flags2 & MF2_FLY && !onground)
		player->bob = FRACUNIT / 2;
		
	angle = (FINEANGLES / 20 * localgametic) & FINEMASK;
	bob = FixedMul(player->bob / 2, finesine[angle]);
	
	/* Set target */
	if (player->playerstate == PST_DEAD)
		player->viewz = player->mo->z + player->viewheight;
	else
	{
		player->TargetViewZ = player->mo->z + ViewHeight;
		
		// Near ledge (stepping partly off ledge)
		/*SubS = R_PointInSubsector(player->mo->x, player->mo->z);
		
		   if (player->mo->z <= player->mo->floorz && player->mo->z > SubS->sector->floorheight)
		   player->TargetViewZ -= FOOTCLIPSIZE; */
		
		// Water
		/*if (player->mo->flags2 & MF2_FEETARECLIPPED
		   && player->playerstate != PST_DEAD && player->mo->z <= player->mo->floorz)
		   {
		   player->TargetViewZ -= FOOTCLIPSIZE;
		   } */
		
		/* Move viewz to target */
		// Below
		if (player->viewz + bob < player->TargetViewZ)
		{
			Diff = FixedDiv(abs(player->TargetViewZ - (player->viewz + bob)), 2 << FRACBITS);
			player->viewz += Diff;
		}
		
		// Above
		else if (player->viewz + bob > player->TargetViewZ)
		{
			Diff = FixedDiv(abs(player->TargetViewZ - (player->viewz + bob)), 2 << FRACBITS);
			player->viewz -= Diff;
		}
		
		// Clip viewz Player object
		if (player->viewz > player->mo->z + (player->mo->height + (player->mo->height >> 2)))
			player->viewz = player->mo->z + (player->mo->height + (player->mo->height >> 2));
		if (player->viewz < player->mo->z + (player->mo->height >> 1))
			player->viewz = player->mo->z + (player->mo->height >> 1);
	}
	
	/* Clip viewz */
	// Real world
	if (player->viewz > player->mo->ceilingz - 6 * FRACUNIT)
		player->viewz = player->mo->ceilingz - 6 * FRACUNIT;
	if (player->viewz < player->mo->floorz + 6 * FRACUNIT)
		player->viewz = player->mo->floorz + 6 * FRACUNIT;
	if (player->viewz < player->mo->z + 6 * FRACUNIT)
		player->viewz = player->mo->z + 6 * FRACUNIT;
		
	/* Translate back to height for death */
	player->viewheight = player->viewz - player->mo->z;
#if 0
	int angle;
	fixed_t bob;
	fixed_t viewheight;
	fixed_t Diff;
	mobj_t* mo;
	
	// Regular movement bobbing
	// (needs to be calculated for gun swing
	// even if not on ground)
	// OPTIMIZE: tablify angle
	// Note: a LUT allows for effects
	//  like a ramp with low health.
	
	mo = player->mo;
	
	player->bob = ((FixedMul(mo->momx, mo->momx) + FixedMul(mo->momy, mo->momy))) >> 2;
	//player->bob = FixedMul(player->MoveMom, player->MoveMom) >> 2;
	
	if (player->bob > MAXBOB)
		player->bob = MAXBOB;
		
	if (player->mo->flags2 & MF2_FLY && !onground)
		player->bob = FRACUNIT / 2;
		
	if ((player->cheats & CF_NOMOMENTUM) || mo->z > mo->floorz)
	{
		//added:15-02-98: it seems to be useless code!
		//player->viewz = player->mo->z + (cv_viewheight.value<<FRACBITS);
		
		//if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
		//    player->viewz = player->mo->ceilingz-4*FRACUNIT;
		player->viewz = mo->z + player->viewheight;
		return;
	}
	
	angle = (FINEANGLES / 20 * localgametic) & FINEMASK;
	bob = FixedMul(player->bob / 2, finesine[angle]);
	
	// move viewheight
	viewheight = cv_viewheight.value << FRACBITS;	// default eye view height
	
	if (player->playerstate == PST_LIVE)
	{
		player->viewheight += player->deltaviewheight;
		
		if (player->viewheight > viewheight)
		{
			player->viewheight = viewheight;
			player->deltaviewheight = 0;
		}
		
		if (player->viewheight < viewheight / 2)
		{
			player->viewheight = viewheight / 2;
			if (player->deltaviewheight <= 0)
				player->deltaviewheight = 1;
		}
		
		if (player->deltaviewheight)
		{
			player->deltaviewheight += FRACUNIT / 4;
			if (!player->deltaviewheight)
				player->deltaviewheight = 1;
		}
	}
	
	if (player->chickenTics)
		player->viewz = mo->z + player->viewheight - (20 * FRACUNIT);
	else
		player->viewz = mo->z + player->viewheight + bob;
		
	if (player->mo->flags2& MF2_FEETARECLIPPED && player->playerstate != PST_DEAD && player->mo->z <= player->mo->floorz)
	{
		player->viewz -= FOOTCLIPSIZE;
	}
	
	if (player->viewz > mo->ceilingz - 4 * FRACUNIT)
		player->viewz = mo->ceilingz - 4 * FRACUNIT;
	if (player->viewz < mo->floorz + 4 * FRACUNIT)
		player->viewz = mo->floorz + 4 * FRACUNIT;
#endif
}

//
// P_MovePlayer
//
void P_MovePlayer(player_t* player)
{
	ticcmd_t* cmd;
	int movefactor = 2048;		//For Boom friction
	int fly = 0, i;
	fixed_t MoveAmount;
	
	cmd = &player->cmd;
	
	if (!P_XGSVal(PGS_COABSOLUTEANGLE))
		player->mo->angle += (((int32_t)cmd->Std.angleturn) << 16);
	else
		player->mo->angle = (((int32_t)cmd->Std.angleturn) << 16);
	
	// GhostlyDeath <August 26, 2011> -- Update listener angle (for sounds)
	player->mo->NoiseThinker.Angle = player->mo->angle;
	
	// GhostlyDeath <August 26, 2011> -- "Effort" bobbing
	if (cmd->Std.forwardmove)
		P_FakeThrust(player, player->mo->angle, cmd->Std.forwardmove * 2048);
	if (cmd->Std.sidemove)
		P_FakeThrust(player, player->mo->angle - ANG90, cmd->Std.sidemove * 2048);
		
	// Do not let the player control movement
	//  if not onground.
	onground = (player->mo->z <= player->mo->floorz) || (player->cheats & CF_FLYAROUND) || (player->mo->flags2 & (MF2_ONMOBJ | MF2_FLY));
	
	if (P_XGSVal(PGS_COOLDJUMPOVER))
	{
		bool_t jumpover = player->cheats & CF_JUMPOVER;
		
		if (cmd->Std.forwardmove && (onground || jumpover))
		{
			// dirty hack to let the player avatar walk over a small wall
			// while in the air
			if (jumpover && player->mo->momz > 0)
				MoveAmount = 5 * 2048;
			else if (!jumpover)
				MoveAmount = cmd->Std.forwardmove * 2048;
				
			P_Thrust(player, player->mo->angle, MoveAmount);
		}
		
		if (cmd->Std.sidemove && onground)
			P_Thrust(player, player->mo->angle - ANG90, cmd->Std.sidemove * 2048);
		
		player->aiming = (signed char)cmd->Std.aiming;
	}
	else
	{
		fixed_t movepushforward = 0, movepushside = 0;
		
		player->aiming = cmd->Std.aiming << 16;
		if (player->chickenTics)
			movefactor = 2500;
		if (P_XGSVal(PGS_COBOOMSUPPORT) && variable_friction)
		{
			//SoM: This seems to be buggy! Can anyone figure out why??
			movefactor = P_GetMoveFactor(player->mo);
			//CONL_PrintF("movefactor: %i\n", movefactor);
		}
		
		if (cmd->Std.forwardmove)
		{
			movepushforward = cmd->Std.forwardmove * movefactor;
			
			if (player->mo->eflags & MF_UNDERWATER)
			{
				// half forward speed when waist under water
				// a little better grip if feets touch the ground
				if (!onground)
					movepushforward = FixedMul(movepushforward, P_XGSFix(PGS_GAMEMIDWATERFRICTION));
				else
					movepushforward = FixedMul(movepushforward, P_XGSFix(PGS_GAMEWATERFRICTION));
					
				if (gametic > player->flushdelay + TICRATE)
				{
					S_StartSound(&player->mo->NoiseThinker, sfx_floush);
					player->flushdelay = gametic;
				}
			}
			else
			{
				// allow very small movement while in air for gameplay
				if (!onground)
					movepushforward = FixedMul(movepushforward, P_XGSFix(PGS_GAMEAIRFRICTION));
			}
			
			P_Thrust(player, player->mo->angle, movepushforward);
		}
		
		if (cmd->Std.sidemove)
		{
			movepushside = cmd->Std.sidemove * movefactor;
			if (player->mo->eflags & MF_UNDERWATER)
			{
				if (!onground)
					movepushside = FixedMul(movepushside, P_XGSFix(PGS_GAMEMIDWATERFRICTION));
				else
					movepushside = FixedMul(movepushside, P_XGSFix(PGS_GAMEWATERFRICTION));
					
				if (gametic > player->flushdelay + TICRATE)
				{
					S_StartSound(&player->mo->NoiseThinker, sfx_floush);
					player->flushdelay = gametic;
				}
			}
			else if (!onground)
				movepushside = FixedMul(movepushside, P_XGSFix(PGS_GAMEAIRFRICTION));
				
			P_Thrust(player, player->mo->angle - ANG90, movepushside);
		}
		
		// GhostlyDeath <October 23, 2010> -- Slow down
		if (!cmd->Std.forwardmove && !cmd->Std.sidemove)
			player->MoveMom >>= 1;
			
		// mouselook swim when waist underwater
		player->mo->eflags &= ~MF_SWIMMING;
		if (player->mo->eflags & MF_UNDERWATER)
		{
			fixed_t a;
			
			// swim up/down full move when forward full speed
			a = FixedMul(movepushforward * 50, finesine[(player->aiming >> ANGLETOFINESHIFT)] >> 5);
			
			if (a != 0)
			{
				player->mo->eflags |= MF_SWIMMING;
				player->mo->momz += a;
			}
		}
	}
	
	// GhostlyDeath <April 12, 2012> -- Enable jumping
	// For some reason, despite cv_allowjump being false, you could still jump
	// since the check was done in P_BuildTicCommand(). So despite not being
	// allowed to jump, a hacked client could jump anyway. This fixes it here.
	i = P_XGSVal(PGS_PLENABLEJUMPING);
	if ((P_XGSVal(PGS_COJUMPREGARDLESS) && !i) || i)
	{
		//added:22-02-98: jumping
		if (cmd->Std.buttons & BT_JUMP)
		{
			if (player->mo->flags2 & MF2_FLY)
				player->flyheight = 10;
			if (player->mo->eflags & MF_UNDERWATER)
			{
				player->mo->momz = P_XGSFix(PGS_PLJUMPGRAVITY) / 2;
				if (gametic > player->flushdelay + TICRATE)
				{
					S_StartSound(&player->mo->NoiseThinker, sfx_floush);
					player->flushdelay = gametic;
				}
			}
			else
				// can't jump while in air, can't jump while jumping
				if (onground && !(player->jumpdown & 1))
				{
					player->mo->momz = P_XGSFix(PGS_PLJUMPGRAVITY);
					if (!(player->cheats & CF_FLYAROUND))
					{
						S_StartSound(&player->mo->NoiseThinker, sfx_jump);
						// keep jumping ok if FLY mode.
						player->jumpdown |= 1;
					}
				}
		}
		else
			player->jumpdown &= ~1;
	}
		
	if (cmd->Std.forwardmove || cmd->Std.sidemove)
	{
		if (player->mo->info->RPlayerRunState)
			if (player->mo->state == states[player->mo->info->spawnstate])
				P_SetMobjState(player->mo, player->mo->info->RPlayerRunState);
	}
	
	/* HERETODO
	   fly = cmd->lookfly>>4;
	   if(fly > 7)
	   fly -= 16;
	   if(fly && player->powers[pw_flight])
	   {
	   if(fly != TOCENTER)
	   {
	   player->flyheight = fly*2;
	   if(!(player->mo->flags2&MF2_FLY))
	   {
	   player->mo->flags2 |= MF2_FLY;
	   player->mo->flags |= MF_NOGRAVITY;
	   }
	   }
	   else
	   {
	   player->mo->flags2 &= ~MF2_FLY;
	   player->mo->flags &= ~MF_NOGRAVITY;
	   }
	   }
	   else if(fly > 0)
	   {
	   P_PlayerUseArtifact(player, arti_fly);
	   }
	 */
	
	if (player->mo->flags2 & MF2_FLY)
	{
		player->mo->momz = player->flyheight * FRACUNIT;
		if (player->flyheight)
			player->flyheight /= 2;
	}
}

//
// P_DeathThink
// Fall on your face when dying.
// Decrease POV height to floor height.
//
#define ANG5    (ANG90/18)

void P_DeathThink(player_t* player)
{
	angle_t angle;
	angle_t delta;
	mobj_t* attacker;			//added:22-02-98:
	fixed_t dist;				//added:22-02-98:
	int pitch;					//added:22-02-98:
	
	P_MovePsprites(player);
	
	// fall to the ground
	/*if (player->viewheight > 6 * FRACUNIT)
	   player->viewheight -= FRACUNIT;
	
	   if (player->viewheight < 6 * FRACUNIT)
	   player->viewheight = 6 * FRACUNIT; */
	
	player->deltaviewheight = 0;
	onground = player->mo->z <= player->mo->floorz;
	
	// GhostlyDeath <September 15, 2011> -- Only lower view if on the floor
	if (onground)
	{
		if (player->viewheight > 6 * FRACUNIT)
			player->viewheight -= FRACUNIT;
			
		if (player->viewheight < 6 * FRACUNIT)
			player->viewheight = 6 * FRACUNIT;
	}
	
	P_CalcHeight(player);
	
	attacker = player->attacker;
	
	// watch my killer (if there is one)
	if (attacker && attacker != player->mo)
	{
		angle = R_PointToAngle2(player->mo->x, player->mo->y, player->attacker->x, player->attacker->y);
		
		delta = angle - player->mo->angle;
		
		if (delta < ANG5 || delta > (unsigned)-ANG5)
		{
			// Looking at killer,
			//  so fade damage flash down.
			player->mo->angle = angle;
			
			if (player->damagecount)
				player->damagecount--;
		}
		else if (delta < ANG180)
			player->mo->angle += ANG5;
		else
			player->mo->angle -= ANG5;
			
		//added:22-02-98:
		// change aiming to look up or down at the attacker (DOESNT WORK)
		// FIXME : the aiming returned seems to be too up or down... later
		
			// FIXME
			// ==00:00:02:56.682 32109== Invalid read of size 4
			// ==00:00:02:56.682 32109==    at 0x465437: P_DeathThink (p_user.c:550)
			// ==00:00:02:56.682 32109==    by 0x465DE5: P_PlayerThink (p_user.c:830)
			// ==00:00:02:56.683 32109==  Address 0x6f8ab10 is 64 bytes inside a block of size 352 free'd
		dist = P_AproxDistance(attacker->x - player->mo->x, attacker->y - player->mo->y);
		//if (dist)
		//    pitch = FixedMul ((160<<FRACBITS), FixedDiv (attacker->z + (attacker->height>>1), dist)) >>FRACBITS;
		//else
		//    pitch = 0;
			// FIXME
			// ==00:00:02:56.683 32109== Invalid read of size 4
			// ==00:00:02:56.683 32109==    at 0x46545B: P_DeathThink (p_user.c:555)
			// ==00:00:02:56.684 32109==    by 0x465DE5: P_PlayerThink (p_user.c:830)
			// ==00:00:02:56.684 32109==    by 0x4644FA: P_Ticker (p_tick.c:141)
			// ==00:00:02:56.684 32109==  Address 0x6f8ab18 is 72 bytes inside a block of size 352 free'd
		pitch = (attacker->z - player->mo->z) >> 17;
		player->aiming = G_ClipAimingPitch(&pitch);
		
	}
	else if (player->damagecount)
		player->damagecount--;
		
	if (player->cmd.Std.buttons & BT_USE)
	{
		player->playerstate = PST_REBORN;
		player->mo->special2 = 666;
	}
}

//
// P_MoveCamera : make sure the camera is not outside the world
//                and looks at the player avatar
//

//#define VIEWCAM_DIST    (128<<FRACBITS)
//#define VIEWCAM_HEIGHT  (20<<FRACBITS)

void P_ResetCamera(player_t* player)
{
	fixed_t x;
	fixed_t y;
	fixed_t z;
	
	player->camera.chase = true;
	x = player->mo->x;
	y = player->mo->y;
	z = player->mo->z + (VIEWHEIGHT << FRACBITS);
	
	// hey we should make sure that the sounds are heard from the camera
	// instead of the marine's head : TO DO
	
	// set bits for the camera
	if (!player->camera.mo)
		player->camera.mo = P_SpawnMobj(x, y, z, INFO_GetTypeByName("LegacyChasecam"));
	else
	{
		player->camera.mo->x = x;
		player->camera.mo->y = y;
		player->camera.mo->z = z;
	}
	
	player->camera.mo->angle = player->mo->angle;
	player->camera.aiming = 0;
}

bool_t PTR_FindCameraPoint(intercept_t* in, void* a_Data)
{

	/*    int         side;
	   fixed_t             slope;
	   fixed_t             dist;
	   line_t*             li;
	
	   li = in->d.line;
	
	   if ( !(li->flags & ML_TWOSIDED) )
	   return false;
	
	   // crosses a two sided line
	   //added:16-02-98: Fab comments : sets opentop, openbottom, openrange
	   //                lowfloor is the height of the lowest floor
	   //                         (be it front or back)
	   P_LineOpening (li);
	
	   dist = FixedMul (attackrange, in->frac);
	
	   if (li->frontsector->floorheight != li->backsector->floorheight)
	   {
	   //added:18-02-98: comments :
	   // find the slope aiming on the border between the two floors
	   slope = FixedDiv (openbottom - cameraz , dist);
	   if (slope > aimslope)
	   return false;
	   }
	
	   if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
	   {
	   slope = FixedDiv (opentop - shootz , dist);
	   if (slope < aimslope)
	   goto hitline;
	   }
	
	   return true;
	
	   // hit line
	   hitline: */
	// stop the search
	return false;
}

fixed_t cameraz;

void P_MoveChaseCamera(player_t* player)
{
	angle_t angle;
	fixed_t x, y, z, viewpointx, viewpointy;
	fixed_t dist;
	mobj_t* mo;
	subsector_t* newsubsec;
	float f1, f2;
	
	if (!player->camera.mo)
		P_ResetCamera(player);
	mo = player->mo;
	
	angle = mo->angle;
	
	// sets ideal cam pos
	dist = player->CamDist;
	x = mo->x - FixedMul(finecosine[(angle >> ANGLETOFINESHIFT) & FINEMASK], dist);
	y = mo->y - FixedMul(finesine[(angle >> ANGLETOFINESHIFT) & FINEMASK], dist);
	z = mo->z + (VIEWHEIGHT << FRACBITS) + player->CamHeight;
	
	/*    P_PathTraverse ( mo->x, mo->y, x, y, PT_ADDLINES, PTR_UseTraverse, NULL); */
	
	// move camera down to move under lower ceilings
	newsubsec = R_IsPointInSubsector((mo->x + player->camera.mo->x) >> 1, (mo->y + player->camera.mo->y) >> 1);
	
	if (!newsubsec)
	{
		// use player sector
		if (mo->subsector->sector->ceilingheight - player->camera.mo->height < z)
			z = mo->subsector->sector->ceilingheight - player->camera.mo->height - 11 * FRACUNIT;	// don't be blocked by a opened door
	}
	else
		// camera fit ?
		if (newsubsec->sector->ceilingheight - player->camera.mo->height < z)
			// no fit
			z = newsubsec->sector->ceilingheight - player->camera.mo->height - 11 * FRACUNIT;
	// is the camera fit is there own sector
	newsubsec = R_PointInSubsector(player->camera.mo->x, player->camera.mo->y);
	if (newsubsec->sector->ceilingheight - player->camera.mo->height < z)
		z = newsubsec->sector->ceilingheight - player->camera.mo->height - 11 * FRACUNIT;
		
	// point viewed by the camera
	// this point is just 64 unit forward the player
	dist = 64 << FRACBITS;
	viewpointx = mo->x + FixedMul(finecosine[(angle >> ANGLETOFINESHIFT) & FINEMASK], dist);
	viewpointy = mo->y + FixedMul(finesine[(angle >> ANGLETOFINESHIFT) & FINEMASK], dist);
	
	player->camera.mo->angle = R_PointToAngle2(player->camera.mo->x, player->camera.mo->y, viewpointx, viewpointy);
	
	// folow the player
	player->camera.mo->momx = FixedMul(x - player->camera.mo->x, player->CamSpeed);
	player->camera.mo->momy = FixedMul(y - player->camera.mo->y, player->CamSpeed);
	player->camera.mo->momz = FixedMul(z - player->camera.mo->z, player->CamSpeed);
	
	// compute aming to look the viewed point
	f1 = FIXED_TO_FLOAT(viewpointx - player->camera.mo->x);
	f2 = FIXED_TO_FLOAT(viewpointy - player->camera.mo->y);
	dist = sqrt(f1 * f1 + f2 * f2) * FRACUNIT;
	angle = R_PointToAngle2(0, player->camera.mo->z, dist, mo->z + (mo->height >> 1) + finesine[(player->aiming >> ANGLETOFINESHIFT) & FINEMASK] * 64);
	
	G_ClipAimingPitch(&angle);
	dist = player->camera.aiming - angle;
	player->camera.aiming -= (dist >> 3);
}

// P_PlayerThink
//

bool_t playerdeadview;			//Fab:25-04-98:show dm rankings while in death view

/* P_WeaponIsUnlocked() -- Checks if a weapon is unlocked */
bool_t P_WeaponIsUnlocked(const PI_wepid_t a_Weapon)
{
	/* Check */
	if (a_Weapon < 0 || a_Weapon >= NUMWEAPONS)
		return false;
	
	/* Wrong Game? */
	// Playing Doom but weapon doesn't belong in Doom
	if (g_CoreGame == CG_DOOM && !(wpnlev1info[a_Weapon]->WeaponFlags & WF_ISDOOM))
		return false;
		
	// Playing Heretic but weapon doesn't belong in Heretic
	if (g_CoreGame == CG_HERETIC && !(wpnlev1info[a_Weapon]->WeaponFlags & WF_ISHERETIC))
		return false;
		
	// Playing Hexen but weapon doesn't belong in Hexen
	if (g_CoreGame == CG_HEXEN && !(wpnlev1info[a_Weapon]->WeaponFlags & WF_ISHEXEN))
		return false;
		
	// Playing Strife but weapon doesn't belong in Strife
	if (g_CoreGame == CG_STRIFE && !(wpnlev1info[a_Weapon]->WeaponFlags & WF_ISSTRIFE))
		return false;
	
	/* Specific Modes */
	// Playing in shareware and the gun is not in shareware
	if ((g_IWADFlags & CIF_SHAREWARE) && (wpnlev1info[a_Weapon]->WeaponFlags & WF_NOTSHAREWARE))
		return false;
	
	// Not playing in commercial and gun is in commercial (Doom II)
	if (!(g_IWADFlags & CIF_COMMERCIAL) && (wpnlev1info[a_Weapon]->WeaponFlags & WF_INCOMMERCIAL))
		return false;
		
	// Not playing in registered and gun is in registered (Heretic)
	if (!(g_IWADFlags & CIF_REGISTERED) && (wpnlev1info[a_Weapon]->WeaponFlags & WF_INREGISTERED))
		return false;
	
	// Not playing in extended and gun is in extended (Heretic)
	if (!(g_IWADFlags & CIF_EXTENDED) && (wpnlev1info[a_Weapon]->WeaponFlags & WF_INEXTENDED))
		return false;
		
	/* Yay it isn't unlocked */
	return true;
}

/* P_CanUseWeapon() -- Can use (switch to) this weapon */
bool_t P_CanUseWeapon(player_t* const a_Player, const PI_wepid_t a_Weapon)
{
	/* Check */
	if (!a_Player || a_Weapon < 0 || a_Weapon >= NUMWEAPONS)
		return false;
	
	/* Perform checks */
	// Available for the taking?
	if (!P_WeaponIsUnlocked(a_Weapon))
		return false;
	
	// Don't have this gun?
	if (!a_Player->weaponowned[a_Weapon])
		return false;
	
	/* Everything worked! */
	return true;
}

/* P_PlayerThink() -- Player thinking */
void P_PlayerThink(player_t* player)
{
#define MAXWEAPONSLOTS 12
	ticcmd_t* cmd;
	PI_wepid_t newweapon;
	int slot;
	int validguns;
	int waterz;
	int i, j, k, l;
	angle_t delta;
	int32_t Screen;
	
	bool_t GunInSlot;
	PI_wepid_t SlotList[MAXWEAPONSLOTS];
	
	/* Find screen for this player */
	// This is the display player that is
	for (Screen = 0; Screen < MAXSPLITSCREEN; Screen++)
		if (P_SpecGetPOV(Screen) == player)
			break;
	
	/* Handle keycard flashing */
	for (i = 0; i < 2; i++)
		for (j = 0; j < 32; j++)
			if (player->KeyFlash[i][j])
			{
				// Play sound every half second or so
				if (Screen >= 0 && Screen < MAXSPLITSCREEN)
					if ((player->KeyFlash[i][j] & 0xF) == 0)
						S_StartSound(NULL, sfx_itemup);
				
				// Remove some time
				player->KeyFlash[i][j]--;
			}

	// GhostlyDeath <May 17, 2012> -- Instead of crashing, spawn a player
	if (!player->mo)
	{
		// Counter-Op Player?
		if (P_XGSVal(PGS_MONENABLEPLAYASMONSTER) && player->CounterOpPlayer)
			P_ControlNewMonster(player);
		
		// Still bad? Then the middle of nowhere is better than nothing
		if (!player->mo)
			P_SpawnPlayerBackup(player - players);
	}
	
	// fixme: do this in the cheat code
	if (player->cheats & CF_NOCLIP)
		player->mo->flags |= MF_NOCLIP;
	else
		player->mo->flags &= ~MF_NOCLIP;
		
	// chain saw run forward
	cmd = &player->cmd;
	if (player->mo->flags & MF_JUSTATTACKED)
	{
// added : now angle turn is a absolute value not relative
		if (!P_XGSVal(PGS_COABSOLUTEANGLE))
			cmd->Std.angleturn = 0;
		cmd->Std.forwardmove = 0xc800 / 512;
		cmd->Std.sidemove = 0;
		player->mo->flags &= ~MF_JUSTATTACKED;
	}
	
	// GhostlyDeath <May 17, 2012> -- If player is reborning still, reborn again
	if (player->playerstate == PST_REBORN)
		G_DoReborn(player - players);
	
	/* Failed to reborn? */
	if (!player->mo)
	{
		// In the middle of nowhere could be better than nothing
		player->mo = P_SpawnMobj(0, 0, 0, INFO_GetTypeByName("DoomPlayer"));
	
		// Set as reborn
		player->playerstate = PST_REBORN;
	}
	
	/* Suicide Pill */
	// GhostlyDeath <September 19, 2012> -- In case one gets stuck?
	if (cmd->Std.buttons & BT_SUICIDE)
		if (gametic >= player->SuicideDelay)
		{
			// If playing as monster
			if (P_XGSVal(PGS_MONENABLEPLAYASMONSTER) && player->CounterOpPlayer)
				P_ControlNewMonster(player);
			
			// A player, but only if suicides are enabled
			else
				if (P_XGSVal(PGS_PLALLOWSUICIDE))
					if (player->mo && player->health > 0)
					{
						player->mo->RXAttackAttackType = PRXAT_SUICIDE;
						P_KillMobj(player->mo, player->mo, player->mo);
					}
			
			// Prevent suicide abuse
			player->SuicideDelay = gametic + (TICRATE * P_XGSVal(PGS_PLSUICIDEDELAY));
		}
	
	if (player->playerstate == PST_DEAD)
	{
		//Fab:25-04-98: show the dm rankings while dead, only in deathmatch
		//DarkWolf95:July 03, 2003:fixed bug where rankings only show on player1's death
		if (player == &players[g_Splits[0].Display] || player == &players[g_Splits[1].Display])
			playerdeadview = true;
			
		P_DeathThink(player);
		
		//added:26-02-98:camera may still move when guy is dead
		if (player->camera.chase)
			P_MoveChaseCamera(player);
		return;
	}
	else if (player == &players[g_Splits[0].Display])
		playerdeadview = false;
		
	// check water content, set stuff in mobj
	P_MobjCheckWater(player->mo);
	
	// Move around.
	// Reactiontime is used to prevent movement
	//  for a bit after a teleport.
	if (player->mo->reactiontime)
		player->mo->reactiontime--;
	else
	{
		// Use player movement?
		if (player->mo->RXFlags[1] & MFREXB_USEPLAYERMOVEMENT)
			P_MovePlayer(player);
		
		// Use monster movement
		else
		{
			// Change direction
			player->mo->movedir = (((uint32_t)player->cmd.Std.angleturn & 0xE000) >> 13U);
			
			// Odd?
			if ((player->cmd.Std.angleturn >> 12) & 1)
				player->mo->movedir++;
			
			// Correct direction
			if (player->mo->movedir < 0)
				player->mo->movedir = DI_NODIR - 1;
			else if (player->mo->movedir >= DI_NODIR)
				player->mo->movedir = 0;
			
			// Set angle
			player->mo->angle = ((angle_t)player->cmd.Std.angleturn) << 16;
			
			// Move forward?
			if (player->cmd.Std.forwardmove >= 5)
				player->mo->movecount = 2;
			else
				player->mo->movecount = -1;
		}
	}
		
	// GhostlyDeath <September 16, 2011> -- Apply fake friction to "effort" bobbing
	player->FakeMom[0] = FixedMul(player->FakeMom[0], ORIG_FRICTION);
	player->FakeMom[1] = FixedMul(player->FakeMom[1], ORIG_FRICTION);
	
	//added:22-02-98: bob view only if looking by the marine's eyes
	if (!player->camera.chase)
		P_CalcHeight(player);
		
	//added:26-02-98: calculate the camera movement
	if (player->camera.chase)
	{
		P_MoveChaseCamera(player);
	
		// GhostlyDeath <March 6, 2012> -- Camera cannot see player?
		if (!P_CheckSight(player->camera.mo, player->mo))
			P_ResetCamera(player);
	}
		
	// check special sectors : damage & secrets
	if (player->mo->RXFlags[0] & MFREXA_ISPLAYEROBJECT)
		P_PlayerInSpecialSector(player);
	
	//
	// water splashes
	//
	if (P_XGSVal(PGS_COENABLESPLASHES) && player->specialsector >= 887 && player->specialsector <= 888)
	{
		if ((player->mo->momx > (2 * FRACUNIT) || player->mo->momx < (-2 * FRACUNIT) || player->mo->momy > (2 * FRACUNIT) || player->mo->momy < (-2 * FRACUNIT) || player->mo->momz > (2 * FRACUNIT)) &&	// jump out of water
		        !(gametic % (32)))
		{
			//
			// make sure we disturb the surface of water (we touch it)
			//
			if (player->specialsector == 887)
				//FLAT TEXTURE 'FWATER'
				waterz = player->mo->subsector->sector->floorheight + (FRACUNIT / 4);
			else
				//faB's current water hack using negative sector tags
				waterz = -(player->mo->subsector->sector->tag << FRACBITS);
				
			// half in the water
			if (player->mo->eflags & MF_TOUCHWATER)
			{
				if (player->mo->z <= player->mo->floorz)	// onground
				{
					fixed_t whater_height = waterz - player->mo->subsector->sector->floorheight;
					
					if (whater_height < (player->mo->height >> 2))
						S_StartSound(&player->mo->NoiseThinker, sfx_splash);
					else
						S_StartSound(&player->mo->NoiseThinker, sfx_floush);
				}
				else
					S_StartSound(&player->mo->NoiseThinker, sfx_floush);
			}
		}
	}
	
	// Check for weapon change.
		// GhostlyDeath <March 9, 2012> -- Rewritten for RMOD
	if (cmd->Std.buttons & BT_CHANGE)
	{
		// Get slot to switch to (if any)
		slot = ((cmd->Std.buttons & BT_SLOTMASK) >> BT_SLOTSHIFT) + 1;
		
		// Deprecated button shifty? or the new way (more guns)?
		if (!cmd->Std.XSNewWeapon[0])
			newweapon = (cmd->Std.buttons & BT_WEAPONMASK) >> BT_WEAPONSHIFT;
		else
			newweapon = INFO_GetWeaponByName(cmd->Std.XSNewWeapon);
		
		// Slot based switching?
		if (cmd->Std.buttons & BT_EXTRAWEAPON)
		{
			// ONLY USED FOR OLD VANILLA DEMOS
			GunInSlot = false;
			l = 0;
		
			// Figure out weapons that belong in this slot
			for (j = 0, i = 0; i < NUMWEAPONS; i++)
				if (P_CanUseWeapon(player, i))
				{
					// Weapon not in this slot?
					if (player->weaponinfo[i]->SlotNum != slot)
						continue;
				
					// Place in slot list before the highest
					if (j < (MAXWEAPONSLOTS - 1))
					{
						// Just place here
						if (j == 0)
						{
							// Current weapon is in this slot?
							if (player->readyweapon == i)
							{
								GunInSlot = true;
								l = j;
							}
						
							// Place in last spot
							SlotList[j++] = i;
						}
					
						// Otherwise more work is needed
						else
						{
							// Start from high to low
								// When the order is lower, we know to insert now
							for (k = 0; k < j; k++)
								if (player->weaponinfo[i]->SwitchOrder < player->weaponinfo[SlotList[k]]->SwitchOrder)
								{
									// Current gun may need shifting
									if (!GunInSlot)
									{
										// Current weapon is in this slot?
										if (player->readyweapon == i)
										{
											GunInSlot = true;
											l = k;
										}
									}
								
									// Possibly shift gun
									else
									{
										// If the current gun is higher then this gun
										// then it will be off by whatever is more
										if (player->weaponinfo[SlotList[l]]->SwitchOrder > player->weaponinfo[i]->SwitchOrder)
											l++;
									}
								
									// move up
									memmove(&SlotList[k + 1], &SlotList[k], sizeof(SlotList[k]) * (MAXWEAPONSLOTS - k - 1));
								
									// Place in slightly upper spot
									SlotList[k] = i;
									j++;
								
									// Don't add it anymore
									break;
								}
						
							// Can't put it anywhere? Goes at end then
							if (k == j)
							{
								// Current weapon is in this slot?
								if (player->readyweapon == i)
								{
									GunInSlot = true;
									l = k;
								}
							
								// Put
								SlotList[j++] = i;
							}
						}
					}
				}
		
			// No guns in this slot? Then don't switch to anything
			if (j == 0)
				newweapon = player->readyweapon;
		
			// If the current gun is in this slot, go to the next in the slot
			else if (GunInSlot)		// from [best - worst]
				newweapon = SlotList[((l - 1) + j) % j];
		
			// Otherwise, switch to the best gun there
			else
				// Set it to the highest valued gun
				newweapon = SlotList[j - 1];
		}
		
		// Weapon ID based switching
		else
		{
			// Well, nothing needs to be done here really!
		}
		
		// New weapon is NOT the current weapon?
		if (newweapon != player->readyweapon)
		{
			// Weapon switching to is berserk cap flagged
			if (player->weaponinfo[newweapon]->WeaponFlags & WF_BERSERKTOGGLE)
				// Only before Legacy 1.28
				if (P_XGSVal(PGS_COFORCEBERSERKSWITCH))
					// Use the lowest weapon with this flag
					for (i = 0; i < NUMWEAPONS; i++)
					{
						// Does not own gun? ignore
						if (!player->weaponowned[i])
							continue;
						
						// Weapon is not berserk flagged
						if (!(player->weaponinfo[i]->WeaponFlags & WF_BERSERKTOGGLE))
							continue;
						
						// Weapon is of lower quality
						if (player->weaponinfo[newweapon]->SwitchOrder < player->weaponinfo[i]->SwitchOrder)
							newweapon = i;
					}
			
			// Can't use this gun?
			if (!P_CanUseWeapon(player, newweapon))
				newweapon = player->readyweapon;
			
			// Set pending weapon (if it is still OK)
			if (newweapon != player->readyweapon)
				player->pendingweapon = newweapon;
		}
	}
	
	// check for use
	if (cmd->Std.buttons & BT_USE)
	{
		if (!player->usedown)
		{
			// GhostlyDeath <May 8, 2012> -- Only allow players to use things
			if (player->mo->RXFlags[0] & MFREXA_ISPLAYEROBJECT)
				P_UseLines(player);
			player->usedown = true;
		}
	}
	else
		player->usedown = false;
		
	// Chicken counter
	if (player->chickenTics)
	{
		// Chicken attack counter
		if (player->chickenPeck)
			player->chickenPeck -= 3;
	}
	// cycle psprites
	P_MovePsprites(player);
	// Counters, time dependend power ups.
	
	// Strength counts up to diminish fade.
	if (player->powers[pw_strength])
		player->powers[pw_strength]++;
		
	if (player->powers[pw_invulnerability])
		player->powers[pw_invulnerability]--;
		
	// the MF_SHADOW activates the tr_transhi translucency while it is set
	// (it doesnt use a preset value through FF_TRANSMASK)
	if (player->powers[pw_invisibility])
		if (!--player->powers[pw_invisibility])
			player->mo->flags &= ~MF_SHADOW;
			
	if (player->powers[pw_infrared])
		player->powers[pw_infrared]--;
		
	if (player->powers[pw_ironfeet])
		player->powers[pw_ironfeet]--;
		
	if (player->damagecount)
		player->damagecount--;
		
	if (player->bonuscount)
		player->bonuscount--;
		
	// Handling colormaps.
	if (player->powers[pw_invulnerability])
	{
		if (player->powers[pw_invulnerability] > BLINKTHRESHOLD || (player->powers[pw_invulnerability] & 8))
			player->fixedcolormap = INVERSECOLORMAP;
		else
			player->fixedcolormap = 0;
	}
	else if (player->powers[pw_infrared])
	{
		if (player->powers[pw_infrared] > BLINKTHRESHOLD || (player->powers[pw_infrared] & 8))
		{
			// almost full bright
			player->fixedcolormap = 1;
		}
		else
			player->fixedcolormap = 0;
	}
	else
		player->fixedcolormap = 0;
#undef MAXWEAPONSLOTS
}

/* P_PlayerOnSameTeam() -- Returns true if the player is on the same team */
bool_t P_PlayerOnSameTeam(player_t* const a_A, player_t* const a_B)
{
	/* Check */
	if (!a_A || !a_B)
		return false;
	
	/* Same Player */
	if (a_A == a_B)
		return true;
	
	/* Coop? */
	if (!P_XGSVal(PGS_GAMEDEATHMATCH))
		return true;
	
	/* Not the same */
	return false;
}

/* P_UpdateScores() -- Updates the local scoreboard */
void P_UpdateScores(void)
{
	/* Build the scoreboard */
	WI_BuildScoreBoard(NULL, false);
}


/*** SPECTATOR PLAYER ***/

static player_t l_SpecPlayers[MAXSPLITSCREEN];	// Fake Player
static mobj_t l_SpecMobjs[MAXSPLITSCREEN];		// Fake Mobj

/* P_SpecInitOne() -- Initializes single player */
static void P_SpecInitOne(const int32_t a_PlayerNum)
{
	int i;
	mapthing_t* MapThing;
	mobj_t* AnotherMo;
	subsector_t* SubS;
	D_XPlayer_t* XPlay;
	
	/* Check */
	if (a_PlayerNum < 0 || a_PlayerNum >= MAXSPLITSCREEN)
		return;
	
	/* Only During Levels */
	if (gamestate != GS_LEVEL)
		return;
	
	/* Reset */
	memset(&l_SpecPlayers[a_PlayerNum], 0, sizeof(l_SpecPlayers[a_PlayerNum]));
	memset(&l_SpecMobjs[a_PlayerNum], 0, sizeof(l_SpecMobjs[a_PlayerNum]));
	
	/* Find a map thing to initialize the view from */
	MapThing = NULL;
	AnotherMo = NULL;
	
	// Try existing players
	for (i = 0; i < MAXPLAYERS && !AnotherMo; i++)
		if (playeringame[i])
			if (players[i].mo)
				AnotherMo = players[i].mo;
	
	// Try player starts first
	if (!MapThing)
		for (i = 0; i < MAXPLAYERS && !MapThing; i++)
			MapThing = playerstarts[i];
	
	// Then deathmatch starts
	if (!MapThing)
		for (i = 0; i < MAX_DM_STARTS && !MapThing; i++)
			MapThing = deathmatchstarts[i];
	
	/* Initialize each player */
	// Quick
	i = a_PlayerNum;
	
	// Bind objects
	l_SpecPlayers[i].mo = &l_SpecMobjs[i];
	l_SpecMobjs[i].player = &l_SpecPlayers[i];
	
	// Player has object
	if (AnotherMo)
	{
		l_SpecMobjs[i].x = AnotherMo->x;
		l_SpecMobjs[i].y = AnotherMo->y;
		l_SpecMobjs[i].z = AnotherMo->z + (AnotherMo->height >> 1);
		l_SpecMobjs[i].angle = AnotherMo->angle;
	}
	
	// Only if MapThing is set
	else if (MapThing)
	{
		// Set Initial Position
		l_SpecMobjs[i].x = ((fixed_t)MapThing->x) << 16;
		l_SpecMobjs[i].y = ((fixed_t)MapThing->y) << 16;
	
		// Calculate Z position
		l_SpecMobjs[i].subsector = SubS = R_PointInSubsector(l_SpecMobjs[i].x, l_SpecMobjs[i].y);
		l_SpecMobjs[i].z = SubS->sector->floorheight + ((SubS->sector->ceilingheight - SubS->sector->floorheight) >> 1);
		
		// Angle
		l_SpecMobjs[i].angle = MapThing->angle * ANGLE_1;
	}
	
	// Correct View Hieght
	l_SpecPlayers[i].viewz = l_SpecMobjs[i].z;
	
	// Set viewing angle correctly, if not playing
	if (!g_Splits[i].Active)
		localangle[i] = l_SpecMobjs[i].angle;
	
	// Don't apply heretic friction to the spectator
	l_SpecPlayers[i].mo->RXFlags[1] |= MFREXB_NOHERETICFRICT;
	
	/* Map fake screens to XPlayers */
	for (i = 0; i < g_NumXPlays; i++)
	{
		// Get
		XPlay = g_XPlays[i];
		
		// Missing?
		if (!XPlay)
			continue;
		
		// Local?
		if ((XPlay->Flags & (DXPF_LOCAL | DXPF_BOT)) == DXPF_LOCAL)
			if (XPlay->ScreenID >= 0 && XPlay->ScreenID < MAXSPLITSCREEN)
				l_SpecPlayers[XPlay->ScreenID].XPlayer = XPlay;
	}
}


/* P_SpecInit() -- Initializes the fake player */
void P_SpecInit(const int32_t a_PlayerNum)
{
	int i;
	static const P_LevelInfoEx_t* LastSpecLevel;
	static bool_t LastFlipped;
	
	/* All */
	// This is called on a new level, possibly
	if (a_PlayerNum < 0)
	{
		// If the map did not change, do not reset (so specs don't get jerked
		// all the time). However -2 forces it, in case of savegames
		if (a_PlayerNum != -2 && LastSpecLevel == g_CurrentLevelInfo && LastFlipped == P_XGSVal(PGS_FUNFLIPLEVELS))
			return;
		
		LastSpecLevel = g_CurrentLevelInfo;
		LastFlipped = P_XGSVal(PGS_FUNFLIPLEVELS);
		
		// Initialize each player
		for (i = 0; i < MAXSPLITSCREEN; i++)
			P_SpecInitOne(i);
		
		// Verify Coop spy
		P_VerifyCoopSpy();
	}
	
	/* Or one */
	else
		P_SpecInitOne(a_PlayerNum);
}

/* P_SpecGet() -- Returns the fake player */
struct player_s* P_SpecGet(const int32_t a_Screen)
{
	/* Check */
	if (a_Screen < 0 || a_Screen >= MAXSPLITSCREEN)
		return NULL;
	
	/* Return associated screen */
	return &l_SpecPlayers[a_Screen];
}

/* P_SpecTicker() -- Ticks fake players */
void P_SpecTicker(void)
{
#define TSCAMDIST FIXEDT_C(128)
#define BUDGEDIST FIXEDT_C(32)
#define TSMOVEUNIT FIXEDT_C(16)
	int32_t i;
	player_t* Mod, *VPlay;
	mobj_t* ChaseThis, *PeerThis, *CamMo, *AttackSpec;
	fixed_t Dist, DistX, DistY, ToMove, MyAng, TargAng;
	fixed_t VeerX, VeerY;
	angle_t Angle, PeerAngle;
	bool_t DeadView;
	
	fixed_t ToDist, ToPos[0], BCPos[2], CDist, BCDist;
	
	/* Title Screen Demo */
	if (g_TitleScreenDemo)
	{
		// For each player split
		for (i = 0; i < MAXSPLITSCREEN; i++)
			if (g_Splits[i].Active)
			{
				// Player to modify
				Mod = &l_SpecPlayers[i];
				VPlay = &players[g_Splits[i].Console];
				
				// Camera Object
				CamMo = Mod->mo;
				
				// Get subsector
				CamMo->subsector = R_PointInSubsector(CamMo->x, CamMo->y);
				
				// Chase the player's object
				ChaseThis = VPlay->mo;
				
				// Player is alive and has a BFG ball
				if (VPlay->LastBFGBall && VPlay->health > 0)
				{
					// Chase and look after the ball
					PeerThis = ChaseThis = VPlay->LastBFGBall;
				}
				
				// Player is alive
				else if (VPlay->health > 0)
				{
					DeadView = false;
					
					// If player is under attack or attacking
					PeerThis = NULL;
					
					// First check attacker
					AttackSpec = VPlay->attacker;
					
					if (!(AttackSpec && AttackSpec->health > 0 &&
						P_CheckSight(VPlay->mo, AttackSpec)))
						AttackSpec = NULL;
					
					// Now check attackee
					AttackSpec = VPlay->Attackee;
					
					if (!(AttackSpec && AttackSpec->health > 0 &&
						P_CheckSight(VPlay->mo, AttackSpec)))
						AttackSpec = NULL;
					
					// It is OK
					if (AttackSpec)
					{
						// Get attacker distance
						Dist = P_AproxDistance(AttackSpec->x - ChaseThis->x,
								AttackSpec->y - ChaseThis->y);
					
						// Object close by
						if (Dist < FIXEDT_C(2048))
							PeerThis = AttackSpec;
					}
				
					// Otherwise stare at player object
					if (!PeerThis)
						PeerThis = VPlay->mo;
				}
				
				// Player is dead
				else
				{
					DeadView = true;
					
					// Chase killer
					ChaseThis = VPlay->attacker;
					
					if (!ChaseThis)
						ChaseThis = VPlay->mo;
					
					// Stare at player object
					PeerThis = VPlay->mo;
				}

#if 1
				// Get 128 units behind chase target
				BCPos[0] = ChaseThis->x + FixedMul(
						finecosine[ChaseThis->angle >> ANGLETOFINESHIFT],
						-TSCAMDIST
					);
				BCPos[1] = ChaseThis->y + FixedMul(
						finesine[ChaseThis->angle >> ANGLETOFINESHIFT],
						-TSCAMDIST
					);
				
				// Get Distance to chase target
				CDist = P_AproxDistance(
							CamMo->x - ChaseThis->x,
							CamMo->y - ChaseThis->y
						);
				
				// Get Distance to behind target
				BCDist = P_AproxDistance(
							CamMo->x - BCPos[0],
							CamMo->y - BCPos[1]
						);
				
				// If too far away from target, move to it
					// Or it cannot be seen by the camera
				if (!P_CheckSight(CamMo, ChaseThis) || (CDist > TSCAMDIST + BUDGEDIST) && !VPlay->attackdown)
				{
					ToPos[0] = ChaseThis->x;
					ToPos[1] = ChaseThis->y;
					ToDist = CDist / 16;
				}
				
				// If really close, back off
#if 0
				else if (CDist < BUDGEDIST)
				{
					ToPos[0] = ChaseThis->x;
					ToPos[1] = ChaseThis->y;
					ToDist = -(CDist / 4);
				}
#endif
				
				// If close enough, move to behind target then
				else
				{
					ToPos[0] = BCPos[0];
					ToPos[1] = BCPos[1];
					
					if (VPlay->attackdown)
						ToDist = BCDist / 8;
					else
						ToDist = BCDist / 32;
				}
				
				// Move to location
				Angle = R_PointToAngle2(CamMo->x, CamMo->y,
					ToPos[0], ToPos[1]);
				CamMo->x += FixedMul(finecosine[Angle >> ANGLETOFINESHIFT], ToDist);
				CamMo->y += FixedMul(finesine[Angle >> ANGLETOFINESHIFT], ToDist);
#else
				// Get distance and angle to chase target
				Dist = P_AproxDistance(
						CamMo->x - ChaseThis->x,
						CamMo->y - ChaseThis->y
					);
				
				
				// Move Camera closer to object chasing
				if (DeadView || Dist > TSCAMDIST)
				{
					Dist /= 16;
					
					CamMo->x += FixedMul(finecosine[Angle >> ANGLETOFINESHIFT], Dist);
					CamMo->y += FixedMul(finesine[Angle >> ANGLETOFINESHIFT], Dist);
				}
				
				// Slowly drift camera behind chase object
					// Location we want to drift to
				VeerX = ChaseThis->x + FixedMul(finecosine[
							ChaseThis->angle >> ANGLETOFINESHIFT], -TSCAMDIST);
				VeerY = ChaseThis->y + FixedMul(finesine[
							ChaseThis->angle >> ANGLETOFINESHIFT], -TSCAMDIST);
					// Get distance to point
				Dist = P_AproxDistance(
						CamMo->x - VeerX,
						CamMo->y - VeerY
					);
					// Reduce it alot!
				Dist /= 32;
					// Move
				CamMo->x += FixedMul(finecosine[ChaseThis->angle >> ANGLETOFINESHIFT], Dist);
				CamMo->y += FixedMul(finecosine[ChaseThis->angle >> ANGLETOFINESHIFT], Dist);
#endif
				Angle = R_PointToAngle2(CamMo->x, CamMo->y,
					ChaseThis->x, ChaseThis->y);
				PeerAngle = R_PointToAngle2(CamMo->x, CamMo->y,
					PeerThis->x, PeerThis->y);

				// Pan Camera to peer
				MyAng = TBL_BAMToDeg(CamMo->angle);
				TargAng = TBL_BAMToDeg(PeerAngle);
				
				// Get dual angles
				DistX = abs(MyAng - TargAng);
				DistY = abs((MyAng + 360) - TargAng);
				
				// Move in the smaller direction
				if (DistY < DistX)
					CamMo->angle += (ANGLE_1 * ((DistY / 4) >> FRACBITS));
				else
					CamMo->angle -= (ANGLE_1 * ((DistX / 4) >> FRACBITS));
				
				// Normalize Height
				Dist = (CamMo->z - (ChaseThis->z + (ChaseThis->height >> 1))) >> 2;
				CamMo->z -= Dist;
				
				// Raise above the floor
				CamMo->subsector = R_PointInSubsector(CamMo->x, CamMo->y);
				
#if 0
				if (CamMo->z < CamMo->subsector->sector->floorheight)
					CamMo->z = CamMo->subsector->sector->floorheight;
				else if (CamMo->z > CamMo->subsector->sector->ceilingheight)
					CamMo->z = CamMo->subsector->sector->ceilingheight;
#endif
				
				// Correct height
				Mod->viewz = CamMo->z;
			}
	}
	
	/* Normal fake */
	else
	{
		// Apply momentum
		for (i = 0; i < MAXSPLITSCREEN; i++)
		{
			// Get Objects
			Mod = &l_SpecPlayers[i];
			CamMo = Mod->mo;
			
			// No object?
			if (!CamMo)
				continue;
			
			// No XPlayer? Correct
			if (!Mod->XPlayer)
				Mod->XPlayer = g_Splits[i].XPlayer;
			
			// Flying
			CamMo->momz = Mod->flyheight << 16;
			if (Mod->flyheight)
				Mod->flyheight >>= 1;
			
			// Apply momentum to object
			CamMo->x += CamMo->momx;
			CamMo->y += CamMo->momy;
			CamMo->z += CamMo->momz;
			
			// Reduce momentum (for friction)
			CamMo->momx = FixedMul(CamMo->momx, ORIG_FRICTION);
			CamMo->momy = FixedMul(CamMo->momy, ORIG_FRICTION);
			
			// Set sound thinker
			CamMo->NoiseThinker.x = CamMo->x;
			CamMo->NoiseThinker.y = CamMo->y;
			CamMo->NoiseThinker.z = CamMo->z;
			CamMo->NoiseThinker.momx = CamMo->momx;
			CamMo->NoiseThinker.momy = CamMo->momy;
			CamMo->NoiseThinker.momz = CamMo->momz;
			CamMo->NoiseThinker.Angle = CamMo->angle;
			CamMo->NoiseThinker.Pitch = FIXEDT_C(1);
			CamMo->NoiseThinker.Volume = FIXEDT_C(1);
			
			// Set Camera Z
			Mod->TargetViewZ = Mod->viewz = CamMo->z;
		}
	}
#undef TSCAMDIST
#undef TSMOVEUNIT
}

void P_Thrust(player_t* player, angle_t angle, fixed_t move);

/* P_SpecRunTics() -- Tic commands on screen */
void P_SpecRunTics(const int32_t a_Screen, ticcmd_t* const a_TicCmd)
{
	player_t* Play;
	mobj_t* Mo;
	
	/* Check */
	if (a_Screen < 0 || a_Screen >= MAXSPLITSCREEN || !a_TicCmd)
		return;
	
	/* Player to modify */
	Play = &l_SpecPlayers[a_Screen];
	Mo = Play->mo;
	
	// Oops!
	if (!Mo)
		return;
	
	/* Modify local angles from tic command */
	//localangle[a_Screen] += a_TicCmd->Std.BaseAngleTurn << 16;
	localangle[a_Screen] += (uint32_t)a_TicCmd->Std.BaseAngleTurn << UINT32_C(16);
	
	// Modify Aiming Angle
	if (a_TicCmd->Std.ResetAim)
		localaiming[a_Screen] = 0;
	else
	{
		// Panning Look
		if (a_TicCmd->Std.ExButtons & BTX_PANLOOK)
			localaiming[a_Screen] = (uint32_t)a_TicCmd->Std.BaseAiming << UINT32_C(16);
		
		// Standard Look
		else
			localaiming[a_Screen] += (uint32_t)a_TicCmd->Std.BaseAiming << UINT32_C(16);
	}
	
	/* Set object looking angles to local */
	Mo->angle = localangle[a_Screen];
	Play->aiming = localaiming[a_Screen];
	
	/* Set Momentums */
	Mo->subsector = &subsectors[0];
	P_Thrust(Play, Mo->angle, a_TicCmd->Std.forwardmove * 2048);
	P_Thrust(Play, Mo->angle - ANG90, a_TicCmd->Std.sidemove * 2048);
	
	/* Flying */
	Play->flyheight = ((fixed_t)a_TicCmd->Std.FlySwim) * 2;
}

/* P_SpecGetPOV() -- Get player point of view */
struct player_s* P_SpecGetPOV(const int32_t a_Screen)
{
	/* Check */
	if (a_Screen < 0 || a_Screen >= MAXSPLITSCREEN)
		return NULL;
	
	/* Demo playing back? */
	if (demoplayback)
	{
		// Always return first POV, or standard screen POV for other players
		if (g_TitleScreenDemo || a_Screen > 0)
			return &players[g_Splits[a_Screen].Display];
		
		// Otherwise, return spec view
		else
		{
			if (g_Splits[0].Display == -1 || !playeringame[g_Splits[0].Display])
				return P_SpecGet(0);
			else
				return &players[g_Splits[a_Screen].Display];
		}
	}
	
	/* No XPlayer? */
	else if (!g_Splits[a_Screen].XPlayer)
		return &players[g_Splits[a_Screen].Display];
	
	/* There is one */
	else
		// Not playing? Return spectator
		if ((demoplayback && g_TitleScreenDemo) || !g_Splits[a_Screen].XPlayer->Player)
			if (g_Splits[a_Screen].Display < 0 ||
				g_Splits[a_Screen].Display >= MAXPLAYERS ||
				!playeringame[g_Splits[a_Screen].Display] || (demoplayback && g_TitleScreenDemo))
				return P_SpecGet(a_Screen);
			else
				return &players[g_Splits[a_Screen].Display];
		
		// Playing, return display
		else if (playeringame[g_Splits[a_Screen].Display])
			return &players[g_Splits[a_Screen].Display];
		
		// Return standard player
		else
		{
			g_Splits[a_Screen].Display = players - g_Splits[a_Screen].XPlayer->Player;
			return g_Splits[a_Screen].XPlayer->Player;
		}
	
	/* Failed */
	// This is never reached, but for GCC
	return NULL;
}

/* P_VerifyCoopSpy() -- Verify coop spy settings */
void P_VerifyCoopSpy(void)
{
}

