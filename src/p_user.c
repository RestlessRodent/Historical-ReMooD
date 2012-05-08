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
//      Player related stuff.
//      Bobbing POV/weapon, movement.
//      Pending weapon.

#include "doomdef.h"
#include "d_event.h"
#include "g_game.h"
#include "p_local.h"
#include "r_main.h"
#include "s_sound.h"
#include "p_setup.h"
#include "p_inter.h"
#include "m_random.h"
#include "p_demcmp.h"

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
	if (player->mo->subsector->sector->special == 15 && !(!(player->mo->z <= player->mo->floorz)))	// Friction_Low
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
	int angle;
	subsector_t* SubS;
	
	/* Base */
	if (player->mo->RXFlags[0] & MFREXA_ISPLAYEROBJECT)
		ViewHeight = cv_viewheight.value << FRACBITS;
	else
		ViewHeight = player->mo->height - 15;
	
	// Calculate bobbing
	player->bob = ((FixedMul(player->FakeMom[0], player->FakeMom[0]) + FixedMul(player->FakeMom[1], player->FakeMom[1]))) >> 2;
	
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
		if (player->viewz < player->mo->z + (player->mo->height - (player->mo->height >> 1)))
			player->viewz = player->mo->z + (player->mo->height - (player->mo->height >> 1));
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
	int fly = 0;
	fixed_t MoveAmount;
	
	cmd = &player->cmd;

#if 0
#ifndef ABSOLUTEANGLE
	player->mo->angle += (cmd->angleturn << 16);
#else
	if (!P_EXGSGetValue(PEXGSBID_COABSOLUTEANGLE))
		player->mo->angle += (cmd->angleturn << 16);
	else
		player->mo->angle = (cmd->angleturn << 16);
#endif
#else
	player->mo->angle = (cmd->angleturn << 16);
#endif
	
	// GhostlyDeath <August 26, 2011> -- Update listener angle (for sounds)
	player->mo->NoiseThinker.Angle = player->mo->angle;
	
	// GhostlyDeath <August 26, 2011> -- "Effort" bobbing
	if (cmd->forwardmove)
		P_FakeThrust(player, player->mo->angle, cmd->forwardmove * 2048);
	if (cmd->sidemove)
		P_FakeThrust(player, player->mo->angle - ANG90, cmd->sidemove * 2048);
		
	// Do not let the player control movement
	//  if not onground.
	onground = (player->mo->z <= player->mo->floorz) || (player->cheats & CF_FLYAROUND) || (player->mo->flags2 & (MF2_ONMOBJ | MF2_FLY));
	
	if (P_EXGSGetValue(PEXGSBID_COOLDJUMPOVER))
	{
		bool_t jumpover = player->cheats & CF_JUMPOVER;
		
		if (cmd->forwardmove && (onground || jumpover))
		{
			// dirty hack to let the player avatar walk over a small wall
			// while in the air
			if (jumpover && player->mo->momz > 0)
				MoveAmount = 5 * 2048;
			else if (!jumpover)
				MoveAmount = cmd->forwardmove * 2048;
				
			P_Thrust(player, player->mo->angle, MoveAmount);
		}
		
		if (cmd->sidemove && onground)
			P_Thrust(player, player->mo->angle - ANG90, cmd->sidemove * 2048);
		
		player->aiming = (signed char)cmd->aiming;
	}
	else
	{
		fixed_t movepushforward = 0, movepushside = 0;
		
		player->aiming = cmd->aiming << 16;
		if (player->chickenTics)
			movefactor = 2500;
		if (boomsupport && variable_friction)
		{
			//SoM: This seems to be buggy! Can anyone figure out why??
			movefactor = P_GetMoveFactor(player->mo);
			//CONL_PrintF("movefactor: %i\n", movefactor);
		}
		
		if (cmd->forwardmove)
		{
			movepushforward = cmd->forwardmove * movefactor;
			
			if (player->mo->eflags & MF_UNDERWATER)
			{
				// half forward speed when waist under water
				// a little better grip if feets touch the ground
				if (!onground)
					movepushforward = FixedMul(movepushforward, P_EXGSGetFixed(PEXGSBID_GAMEMIDWATERFRICTION));
				else
					movepushforward = FixedMul(movepushforward, P_EXGSGetFixed(PEXGSBID_GAMEWATERFRICTION));
					
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
					movepushforward = FixedMul(movepushforward, P_EXGSGetFixed(PEXGSBID_GAMEAIRFRICTION));
			}
			
			P_Thrust(player, player->mo->angle, movepushforward);
		}
		
		if (cmd->sidemove)
		{
			movepushside = cmd->sidemove * movefactor;
			if (player->mo->eflags & MF_UNDERWATER)
			{
				if (!onground)
					movepushside = FixedMul(movepushside, P_EXGSGetFixed(PEXGSBID_GAMEMIDWATERFRICTION));
				else
					movepushside = FixedMul(movepushside, P_EXGSGetFixed(PEXGSBID_GAMEWATERFRICTION));
					
				if (gametic > player->flushdelay + TICRATE)
				{
					S_StartSound(&player->mo->NoiseThinker, sfx_floush);
					player->flushdelay = gametic;
				}
			}
			else if (!onground)
				movepushside = FixedMul(movepushside, P_EXGSGetFixed(PEXGSBID_GAMEAIRFRICTION));
				
			P_Thrust(player, player->mo->angle - ANG90, movepushside);
		}
		
		// GhostlyDeath <October 23, 2010> -- Slow down
		if (!cmd->forwardmove && !cmd->sidemove)
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
	if (P_EXGSGetValue(PEXGSBID_COENABLEJUMPING))
	{
		//added:22-02-98: jumping
		if (cmd->buttons & BT_JUMP)
		{
			if (player->mo->flags2 & MF2_FLY)
				player->flyheight = 10;
			if (player->mo->eflags & MF_UNDERWATER)
			{
				player->mo->momz = P_EXGSGetFixed(PEXGSBID_PLJUMPGRAVITY) / 2;
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
					player->mo->momz = P_EXGSGetFixed(PEXGSBID_PLJUMPGRAVITY);
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
		
	if (cmd->forwardmove || cmd->sidemove)
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
		
	if (player->cmd.buttons & BT_USE)
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

consvar_t cv_cam_dist = { "cam_dist", "128", CV_FLOAT, NULL };
consvar_t cv_cam_height = { "cam_height", "20", CV_FLOAT, NULL };
consvar_t cv_cam_speed = { "cam_speed", "0.25", CV_FLOAT, NULL };

void P_ResetCamera(player_t* player)
{
	fixed_t x;
	fixed_t y;
	fixed_t z;
	
	player->camera.chase = true;
	x = player->mo->x;
	y = player->mo->y;
	z = player->mo->z + (cv_viewheight.value << FRACBITS);
	
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
	dist = cv_cam_dist.value;
	x = mo->x - FixedMul(finecosine[(angle >> ANGLETOFINESHIFT) & FINEMASK], dist);
	y = mo->y - FixedMul(finesine[(angle >> ANGLETOFINESHIFT) & FINEMASK], dist);
	z = mo->z + (cv_viewheight.value << FRACBITS) + cv_cam_height.value;
	
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
	player->camera.mo->momx = FixedMul(x - player->camera.mo->x, cv_cam_speed.value);
	player->camera.mo->momy = FixedMul(y - player->camera.mo->y, cv_cam_speed.value);
	player->camera.mo->momz = FixedMul(z - player->camera.mo->z, cv_cam_speed.value);
	
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
bool_t P_WeaponIsUnlocked(const weapontype_t a_Weapon)
{
	/* Check */
	if (a_Weapon < 0 || a_Weapon >= NUMWEAPONS)
		return false;
		
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
bool_t P_CanUseWeapon(player_t* const a_Player, const weapontype_t a_Weapon)
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

void P_PlayerThink(player_t* player)
{
	ticcmd_t* cmd;
	weapontype_t newweapon;
	int slot;
	int validguns;
	int waterz;
	int i, j, k, l;
	angle_t delta;
	
#ifdef PARANOIA
	if (!player->mo)
		I_Error("p_playerthink : players[%d].mo == NULL", player - players);
#endif
		
	// GhostlyDeath <July 10, 2008> -- Profile Check
	if (!player->profile && !demoplayback && !menuactive)
		for (i = 0; i < g_SplitScreen + 1; i++)
			if (playeringame[consoleplayer[i]] && player == &players[consoleplayer[i]])
			{
				M_ProfilePrompt(i);
				break;
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
#ifndef ABSOLUTEANGLE
		cmd->angleturn = 0;
#endif
		cmd->forwardmove = 0xc800 / 512;
		cmd->sidemove = 0;
		player->mo->flags &= ~MF_JUSTATTACKED;
	}
	
	if (player->playerstate == PST_REBORN)
	{
		return;
#ifdef PARANOIA
		I_Error("player %d is in PST_REBORN\n");
#else
		// it is not "normal" but far to be critical
		return;
#endif
	}
		
	if (player->playerstate == PST_DEAD)
	{
		//Fab:25-04-98: show the dm rankings while dead, only in deathmatch
		//DarkWolf95:July 03, 2003:fixed bug where rankings only show on player1's death
		if (player == &players[displayplayer[0]] || player == &players[displayplayer[1]])
			playerdeadview = true;
			
		P_DeathThink(player);
		
		//added:26-02-98:camera may still move when guy is dead
		if (player->camera.chase)
			P_MoveChaseCamera(player);
		return;
	}
	else if (player == &players[displayplayer[0]])
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
			player->mo->movedir = (((uint32_t)player->cmd.angleturn & 0xE000) >> 13U);
			
			// Odd?
			if ((player->cmd.angleturn >> 12) & 1)
				player->mo->movedir++;
			
			// Correct direction
			if (player->mo->movedir < 0)
				player->mo->movedir = DI_NODIR - 1;
			else if (player->mo->movedir >= DI_NODIR)
				player->mo->movedir = 0;
			
			// Set angle
			player->mo->angle = ((angle_t)player->cmd.angleturn) << 16;
			
			// Move forward?
			if (player->cmd.forwardmove >= 5)
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
	P_PlayerInSpecialSector(player);
	
	//
	// water splashes
	//
	if (P_EXGSGetValue(PEXGSBID_COENABLESPLASHES) && player->specialsector >= 887 && player->specialsector <= 888)
	{
		if ((player->mo->momx > (2 * FRACUNIT) || player->mo->momx < (-2 * FRACUNIT) || player->mo->momy > (2 * FRACUNIT) || player->mo->momy < (-2 * FRACUNIT) || player->mo->momz > (2 * FRACUNIT)) &&	// jump out of water
		        !(gametic % (32)))
		{
			//
			// make sur we disturb the surface of water (we touch it)
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
	if (cmd->buttons & BT_CHANGE)
	{
		// Get slot to switch to (if any)
		slot = ((cmd->buttons & BT_SLOTMASK) >> BT_SLOTSHIFT) + 1;
		
		// Deprecated button shifty? or the new way (more guns)?
		if (!cmd->XNewWeapon)
			newweapon = (cmd->buttons & BT_WEAPONMASK) >> BT_WEAPONSHIFT;
		else
			newweapon = cmd->XNewWeapon;
		
		// Slot based switching?
		if (cmd->buttons & BT_EXTRAWEAPON)
		{
			// DEPRECATED, DON'T USE AT ALL!!!
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
				if (P_EXGSGetValue(PEXGSBID_COFORCEBERSERKSWITCH))
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
	if (cmd->buttons & BT_USE)
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
	if (!cv_deathmatch.value)
		return true;
	
	/* Not the same */
	return false;
}

