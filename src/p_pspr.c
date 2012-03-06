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
//      Weapon sprite animation, weapon objects.
//      Action functions for weapons.

#include "doomdef.h"
#include "d_event.h"
#include "p_local.h"
#include "p_pspr.h"
#include "p_inter.h"
#include "s_sound.h"
#include "g_game.h"
#include "g_input.h"
#include "r_main.h"
#include "m_random.h"
#include "p_inter.h"
#include "p_demcmp.h"

#define LOWERSPEED              FRACUNIT*6
#define RAISESPEED              FRACUNIT*6

#define WEAPONBOTTOM            128*FRACUNIT
#define WEAPONTOP               32*FRACUNIT

#define FLAME_THROWER_TICS      (10*TICRATE)
#define MAGIC_JUNK              1234
#define MAX_MACE_SPOTS          8

mobjtype_t PuffType;

//
// P_SetPsprite
//
void P_SetPsprite(player_t* player, int position, statenum_t stnum)
{
	pspdef_t* psp;
	state_t* state;
	
	psp = &player->psprites[position];
	
	do
	{
		if (!stnum)
		{
			// object removed itself
			psp->state = NULL;
			break;
		}
		// GhostlyDeath <November 3, 2010> -- PARANOIA removal
		if (stnum >= NUMSTATES)
		{
			CONL_PrintF("WARNING - P_SetPsprite: State %i exceeds %i. (%s:%i).\n", stnum, NUMSTATES, __FILE__, __LINE__);
			return;
		}
		
		state = &states[stnum];
		psp->state = state;
		psp->tics = state->tics;	// could be 0
		
		/* UNUSED
		   if (state->misc1)
		   {
		   // coordinate set
		   psp->sx = state->misc1 << FRACBITS;
		   psp->sy = state->misc2 << FRACBITS;
		   }
		 */
		// Call action routine.
		// Modified handling.
		if (state->action.acp2)
		{
			state->action.acp2(player, psp);
			if (!psp->state)
				break;
		}
		
		stnum = psp->state->nextstate;
		
	}
	while (!psp->tics);
	// an initial state of 0 could cycle through
}

//
// P_CalcSwing
//

/* BP: UNUSED

fixed_t         swingx;
fixed_t         swingy;

void P_CalcSwing (player_t*     player)
{
    fixed_t     swing;
    int         angle;

    // OPTIMIZE: tablify this.
    // A LUT would allow for different modes,
    //  and add flexibility.

    swing = player->bob;

    angle = (FINEANGLES/70*leveltime)&FINEMASK;
    swingx = FixedMul ( swing, finesine[angle]);

    angle = (FINEANGLES/70*leveltime+FINEANGLES/2)&FINEMASK;
    swingy = -FixedMul ( swingx, finesine[angle]);
}
*/

//
// P_BringUpWeapon
// Starts bringing the pending weapon up
// from the bottom of the screen.
// Uses player
//
void P_BringUpWeapon(player_t* player)
{
	statenum_t newstate;
	
	/* Check */
	if (!player)
		return;
		
	if (player->pendingweapon == wp_nochange)
		player->pendingweapon = player->readyweapon;
		
	if (player->pendingweapon == wp_chainsaw)
		S_StartSound(&player->mo->NoiseThinker, sfx_sawup);
		
	// GhostlyDeath <November 3, 2010> -- PARANOIA removal
	if (player->pendingweapon >= NUMWEAPONS)
	{
		CONL_PrintF("WARNING - P_BringUpWeapon: %i (player->pendingweapon) >= %i (%s:%i).\n", player->pendingweapon, NUMWEAPONS, __FILE__, __LINE__);
		return;
	}
	
	newstate = player->weaponinfo[player->pendingweapon].upstate;
	
	player->pendingweapon = wp_nochange;
	player->psprites[ps_weapon].sy = WEAPONBOTTOM;
	
	P_SetPsprite(player, ps_weapon, newstate);
}

//
// P_CheckAmmo
// Returns true if there is enough ammo to shoot.
// If not, selects the next weapon to use.
//
bool_t P_CheckAmmo(player_t* player)
{
	ammotype_t ammo;
	int count;
	
	ammo = player->weaponinfo[player->readyweapon].ammo;
	
	if (cv_infiniteammo.value)
		return true;
		
	// Minimal amount for one shot varies.
	count = player->weaponinfo[player->readyweapon].ammopershoot;
	
	// Some do not need ammunition anyway.
	// Return if current ammunition sufficient.
	if (ammo == am_noammo || player->ammo[ammo] >= count)
		return true;
		
	// Out of ammo, pick a weapon to change to.
	// Preferences are set here.
	// added by Boris : preferred weapons order
	if (!player->originalweaponswitch)
		VerifFavoritWeapon(player);
		
		
	do
	{
		if (player->weaponowned[wp_plasma] && player->ammo[am_cell] >= player->weaponinfo[wp_plasma].ammopershoot && (gamemode != shareware))
		{
			player->pendingweapon = wp_plasma;
		}
		else if (player->weaponowned[wp_supershotgun]
		         && player->ammo[am_shell] >= player->weaponinfo[wp_supershotgun].ammopershoot && (gamemode == commercial))
		{
			player->pendingweapon = wp_supershotgun;
		}
		else if (player->weaponowned[wp_chaingun] && player->ammo[am_clip] >= player->weaponinfo[wp_chaingun].ammopershoot)
		{
			player->pendingweapon = wp_chaingun;
		}
		else if (player->weaponowned[wp_shotgun] && player->ammo[am_shell] >= player->weaponinfo[wp_shotgun].ammopershoot)
		{
			player->pendingweapon = wp_shotgun;
		}
		else if (player->ammo[am_clip] >= player->weaponinfo[wp_pistol].ammopershoot)
		{
			player->pendingweapon = wp_pistol;
		}
		else if (player->weaponowned[wp_chainsaw])
		{
			player->pendingweapon = wp_chainsaw;
		}
		else if (player->weaponowned[wp_missile] && player->ammo[am_misl] >= player->weaponinfo[wp_missile].ammopershoot)
		{
			player->pendingweapon = wp_missile;
		}
		else if (player->weaponowned[wp_bfg] && player->ammo[am_cell] >= player->weaponinfo[wp_bfg].ammopershoot && (gamemode != shareware))
		{
			player->pendingweapon = wp_bfg;
		}
		else
		{
			// If everything fails.
			player->pendingweapon = wp_fist;
		}
		
	}
	while (player->pendingweapon == wp_nochange);
	
	// Now set appropriate weapon overlay.
	P_SetPsprite(player, ps_weapon, player->weaponinfo[player->readyweapon].downstate);
	
	return false;
}

//
// P_FireWeapon.
//
void P_FireWeapon(player_t* player)
{
	statenum_t newstate;
	
	if (!P_CheckAmmo(player))
		return;
		
	P_SetMobjState(player->mo, player->mo->info->RPlayerMeleeAttackState);
	newstate = player->weaponinfo[player->readyweapon].atkstate;
	
	P_SetPsprite(player, ps_weapon, newstate);
	P_NoiseAlert(player->mo, player->mo);
}

//
// P_DropWeapon
// Player died, so put the weapon away.
//
void P_DropWeapon(player_t* player)
{
	P_SetPsprite(player, ps_weapon, player->weaponinfo[player->readyweapon].downstate);
}

//
// A_WeaponReady
// The player can fire the weapon
// or change to another weapon at this time.
// Follows after getting weapon up,
// or after previous attack/fire sequence.
//
void A_WeaponReady(player_t* player, pspdef_t* psp)
{

	// get out of attack state
	if (player->mo->state == &states[player->mo->info->RPlayerRangedAttackState] || player->mo->state == &states[player->mo->info->RPlayerMeleeAttackState])
	{
		P_SetMobjState(player->mo, player->mo->info->spawnstate);
	}
	
	if (player->readyweapon == wp_chainsaw && psp->state == &states[S_SAW])
	{
		S_StartSound(&player->mo->NoiseThinker, sfx_sawidl);
	}
	// check for change
	//  if player is dead, put the weapon away
	if (player->pendingweapon != wp_nochange || !player->health)
	{
		// change weapon
		//  (pending weapon should allready be validated)
		P_SetPsprite(player, ps_weapon, player->weaponinfo[player->readyweapon].downstate);
		return;
	}
	// check for fire
	//  the missile launcher and bfg do not auto fire
	if (player->cmd.buttons & BT_ATTACK)
	{
		if (!player->attackdown || (player->readyweapon != wp_missile && (player->readyweapon != wp_bfg)))
		{
			player->attackdown = true;
			P_FireWeapon(player);
			return;
		}
	}
	else
		player->attackdown = false;
	{
		int angle;
		
		// bob the weapon based on movement speed
		angle = (128 * leveltime) & FINEMASK;
		psp->sx = FRACUNIT + FixedMul(player->bob, finecosine[angle]);
		angle &= FINEANGLES / 2 - 1;
		psp->sy = WEAPONTOP + FixedMul(player->bob, finesine[angle]);
	}
}

// client prediction stuff
void A_TicWeapon(player_t* player, pspdef_t* psp)
{
	if ((void*)psp->state->action.acp2 == (void*)A_WeaponReady && psp->tics == psp->state->tics)
	{
		int angle;
		
		// bob the weapon based on movement speed
		angle = (128 * localgametic) & FINEMASK;
		psp->sx = FRACUNIT + FixedMul(player->bob, finecosine[angle]);
		angle &= FINEANGLES / 2 - 1;
		psp->sy = WEAPONTOP + FixedMul(player->bob, finesine[angle]);
	}
}

//
// A_ReFire
// The player can re-fire the weapon
// without lowering it entirely.
//
void A_ReFire(player_t* player, pspdef_t* psp)
{

	// check for fire
	//  (if a weaponchange is pending, let it go through instead)
	if ((player->cmd.buttons & BT_ATTACK) && player->pendingweapon == wp_nochange && player->health)
	{
		player->refire++;
		P_FireWeapon(player);
	}
	else
	{
		player->refire = 0;
		P_CheckAmmo(player);
	}
}

void A_CheckReload(player_t* player, pspdef_t* psp)
{
	P_CheckAmmo(player);
#if 0
	if (player->ammo[am_shell] < 2)
		P_SetPsprite(player, ps_weapon, S_DSNR1);
#endif
}

//
// A_Lower
// Lowers current weapon,
//  and changes weapon at bottom.
//
void A_Lower(player_t* player, pspdef_t* psp)
{
	if (player->chickenTics)
		psp->sy = WEAPONBOTTOM;
	else
		psp->sy += LOWERSPEED;
		
	// Is already down.
	if (psp->sy < WEAPONBOTTOM)
		return;
		
	// Player is dead.
	if (player->playerstate == PST_DEAD)
	{
		psp->sy = WEAPONBOTTOM;
		
		// don't bring weapon back up
		return;
	}
	// The old weapon has been lowered off the screen,
	// so change the weapon and start raising it
	if (!player->health)
	{
		// Player is dead, so keep the weapon off screen.
		P_SetPsprite(player, ps_weapon, S_NULL);
		return;
	}
	
	player->readyweapon = player->pendingweapon;
	
	P_BringUpWeapon(player);
}

//
// A_Raise
//
void A_Raise(player_t* player, pspdef_t* psp)
{
	psp->sy -= RAISESPEED;
	
	if (psp->sy > WEAPONTOP)
		return;
		
	psp->sy = WEAPONTOP;
	
	// The weapon has been raised all the way,
	//  so change to the ready state.
	P_SetPsprite(player, ps_weapon, player->weaponinfo[player->readyweapon].readystate);
}

//
// A_GunFlash
//
void A_GunFlash(player_t* player, pspdef_t* psp)
{
	P_SetMobjState(player->mo, player->mo->info->RPlayerRangedAttackState);
	P_SetPsprite(player, ps_flash, player->weaponinfo[player->readyweapon].flashstate);
}

//
// WEAPON ATTACKS
//

//
// A_Punch
//
void A_Punch(player_t* player, pspdef_t* psp)
{
	angle_t angle;
	angle_t* locang = NULL;
	int damage;
	int slope;
	int i;
	
	PuffType = MT_PUFF;
	damage = (P_Random() % 10 + 1) << 1;
	
	if (player->powers[pw_strength])
		damage *= 10;
		
	angle = player->mo->angle;
	angle += (P_Random() << 18);	// WARNING: don't put this in one line
	angle -= (P_Random() << 18);	// else this expretion is ambiguous (evaluation order not diffined)
	
	slope = P_AimLineAttack(player->mo, angle, MELEERANGE);
	P_LineAttack(player->mo, angle, MELEERANGE, slope, damage);
	
	// turn to face target
	if (linetarget)
	{
		S_StartSound(&player->mo->NoiseThinker, sfx_punch);
		player->mo->angle = R_PointToAngle2(player->mo->x, player->mo->y, linetarget->x, linetarget->y);
		
		// GhostlyDeath -- Effect Local Aiming yknow
		for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
			if (playeringame[consoleplayer[i]] && player == &players[consoleplayer[i]])
				locang = &localangle[i];
				
		if (locang)
		{
			angle_t victimangle = 0;
			angle_t myangle = 0;
			angle_t actualangle = 0;
			angle_t virtualangle = 0;
			int someactualangle = 0;
			int somevirtualangle = 0;
			int somemyangle = 0;
			int someoffset = 0;
			
			// First Face the target
			actualangle = R_PointToAngle2(player->mo->x, player->mo->y, linetarget->x, linetarget->y);
			virtualangle = *locang;
			myangle = *locang;
			
			someactualangle = actualangle >> 16;
			somevirtualangle = virtualangle >> 16;
			somemyangle = myangle >> 16;
			
			while (somevirtualangle != (someactualangle))
			{
				if (somevirtualangle + someoffset < (someactualangle))
					someoffset++;
				else if (somevirtualangle + someoffset > (someactualangle))
					someoffset--;
				else
				{
					*locang += (someoffset << 16);
					break;
				}
			}
		}
	}
}

//
// A_Saw
//
void A_Saw(player_t* player, pspdef_t* psp)
{
	angle_t angle;
	angle_t* locang = NULL;
	int damage;
	int slope;
	int i;
	
	PuffType = MT_PUFF;
	damage = 2 * (P_Random() % 10 + 1);
	angle = player->mo->angle;
	angle += (P_Random() << 18);	// WARNING: don't put this in one line
	angle -= (P_Random() << 18);	// else this expretion is ambiguous (evaluation order not diffined)
	
	// use meleerange + 1 se the puff doesn't skip the flash
	slope = P_AimLineAttack(player->mo, angle, MELEERANGE + 1);
	P_LineAttack(player->mo, angle, MELEERANGE + 1, slope, damage);
	
	if (!linetarget)
	{
		S_StartSound(&player->mo->NoiseThinker, sfx_sawful);
		return;
	}
	S_StartSound(&player->mo->NoiseThinker, sfx_sawhit);
	
	// turn to face target
	angle = R_PointToAngle2(player->mo->x, player->mo->y, linetarget->x, linetarget->y);
	if (angle - player->mo->angle > ANG180)
	{
		if (angle - player->mo->angle < -ANG90 / 20)
			player->mo->angle = angle + ANG90 / 21;
		else
			player->mo->angle -= ANG90 / 20;
	}
	else
	{
		if (angle - player->mo->angle > ANG90 / 20)
			player->mo->angle = angle - ANG90 / 21;
		else
			player->mo->angle += ANG90 / 20;
	}
	
	// GhostlyDeath -- Effect Local Aiming yknow
	for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
		if (playeringame[consoleplayer[i]] && player == &players[consoleplayer[i]])
			locang = &localangle[i];
			
	if (locang)
	{
		angle_t victimangle = 0;
		angle_t myangle = 0;
		angle_t actualangle = 0;
		angle_t virtualangle = 0;
		int someactualangle = 0;
		int somevirtualangle = 0;
		int somemyangle = 0;
		int someoffset = 0;
		
		// First Face the target
		actualangle = R_PointToAngle2(player->mo->x, player->mo->y, linetarget->x, linetarget->y);
		virtualangle = *locang;
		myangle = *locang;
		
		someactualangle = actualangle >> 16;
		somevirtualangle = virtualangle >> 16;
		somemyangle = myangle >> 16;
		
		while (somevirtualangle != (someactualangle))
		{
			if (somevirtualangle + someoffset < (someactualangle))
				someoffset++;
			else if (somevirtualangle + someoffset > (someactualangle))
				someoffset--;
			else
			{
				*locang += (someoffset << 16);
				break;
			}
		}
	}
	
	player->mo->flags |= MF_JUSTATTACKED;
}

//
// A_FireMissile : rocket launcher fires a rocket
//
void A_FireMissile(player_t* player, pspdef_t* psp)
{
	if (!cv_infiniteammo.value)
		player->ammo[player->weaponinfo[player->readyweapon].ammo] -= player->weaponinfo[player->readyweapon].ammopershoot;
	//added:16-02-98: added player arg3
	P_SpawnPlayerMissile(player->mo, MT_ROCKET);
}

//
// A_FireBFG
//
void A_FireBFG(player_t* player, pspdef_t* psp)
{
	if (!cv_infiniteammo.value)
		player->ammo[player->weaponinfo[player->readyweapon].ammo] -= player->weaponinfo[player->readyweapon].ammopershoot;
	//added:16-02-98:added player arg3
	P_SpawnPlayerMissile(player->mo, MT_BFG);
}

//
// A_FirePlasma
//
void A_FirePlasma(player_t* player, pspdef_t* psp)
{
	if (!cv_infiniteammo.value)
		player->ammo[player->weaponinfo[player->readyweapon].ammo] -= player->weaponinfo[player->readyweapon].ammopershoot;
		
	P_SetPsprite(player, ps_flash, player->weaponinfo[player->readyweapon].flashstate + (P_Random() & 1));
	
	//added:16-02-98: added player arg3
	P_SpawnPlayerMissile(player->mo, MT_PLASMA);
}

//
// P_BulletSlope
// Sets a slope so a near miss is at aproximately
// the height of the intended target
//
fixed_t bulletslope;

//added:16-02-98: Fab comments: autoaim for the bullet-type weapons
void P_BulletSlope(mobj_t* mo)
{
	angle_t an;
	
	//added:18-02-98: if AUTOAIM, try to aim at something
	if (!((mo->player->autoaim_toggle && (DEMOCVAR(allowautoaim).value || demoversion <= 111)) || DEMOCVAR(forceautoaim).value))
		goto notagetfound;
		
	// see which target is to be aimed at
	an = mo->angle;
	bulletslope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT);
	
	if (!linetarget)
	{
		an += 1 << 26;
		bulletslope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT);
		if (!linetarget)
		{
			an -= 2 << 26;
			bulletslope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT);
		}
		if (!linetarget)
		{
notagetfound:
			if (demoversion >= 128)
				bulletslope = AIMINGTOSLOPE(mo->player->aiming);
			else
				bulletslope = (mo->player->aiming << FRACBITS) / 160;
		}
	}
}

//
// P_GunShot
//
//added:16-02-98: used only for player (pistol,shotgun,chaingun)
//                supershotgun use p_lineattack directely
void P_GunShot(mobj_t* mo, bool_t accurate)
{
	angle_t angle;
	int damage;
	
	damage = 5 * (P_Random() % 3 + 1);
	angle = mo->angle;
	
	if (!accurate)
	{
		angle += (P_Random() << 18);	// WARNING: don't put this in one line
		angle -= (P_Random() << 18);	// else this expretion is ambiguous (evaluation order not diffined)
	}
	
	P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage);
}

//
// A_FirePistol
//
void A_FirePistol(player_t* player, pspdef_t* psp)
{
	S_StartSound(&player->mo->NoiseThinker, sfx_pistol);
	
	PuffType = MT_PUFF;
	P_SetMobjState(player->mo, player->mo->info->RPlayerRangedAttackState);
	
	if (!cv_infiniteammo.value)
		player->ammo[player->weaponinfo[player->readyweapon].ammo]--;
		
	P_SetPsprite(player, ps_flash, player->weaponinfo[player->readyweapon].flashstate);
	
	P_BulletSlope(player->mo);
	P_GunShot(player->mo, !player->refire);
}

//
// A_FireShotgun
//
void A_FireShotgun(player_t* player, pspdef_t* psp)
{
	int i;
	
	PuffType = MT_PUFF;
	S_StartSound(&player->mo->NoiseThinker, sfx_shotgn);
	P_SetMobjState(player->mo, player->mo->info->RPlayerRangedAttackState);
	
	if (!cv_infiniteammo.value)
		player->ammo[player->weaponinfo[player->readyweapon].ammo]--;
	P_SetPsprite(player, ps_flash, player->weaponinfo[player->readyweapon].flashstate);
	
	P_BulletSlope(player->mo);
	for (i = 0; i < 7; i++)
		P_GunShot(player->mo, false);
}

//
// A_FireShotgun2 (SuperShotgun)
//
void A_FireShotgun2(player_t* player, pspdef_t* psp)
{
	int i;
	angle_t angle;
	int damage;
	
	PuffType = MT_PUFF;
	S_StartSound(&player->mo->NoiseThinker, sfx_dshtgn);
	P_SetMobjState(player->mo, player->mo->info->RPlayerRangedAttackState);
	
	if (!cv_infiniteammo.value)
		player->ammo[player->weaponinfo[player->readyweapon].ammo] -= 2;
		
	P_SetPsprite(player, ps_flash, player->weaponinfo[player->readyweapon].flashstate);
	
	P_BulletSlope(player->mo);
	
	for (i = 0; i < 20; i++)
	{
		int slope = bulletslope + (P_SignedRandom() << 5);
		
		damage = 5 * (P_Random() % 3 + 1);
		angle = player->mo->angle + (P_SignedRandom() << 19);
		P_LineAttack(player->mo, angle, MISSILERANGE, slope, damage);
	}
}

//
// A_FireCGun
//
void A_FireCGun(player_t* player, pspdef_t* psp)
{
	S_StartSound(&player->mo->NoiseThinker, sfx_pistol);
	
	if (!cv_infiniteammo.value)
		if (!player->ammo[player->weaponinfo[player->readyweapon].ammo])
			return;
			
	PuffType = MT_PUFF;
	P_SetMobjState(player->mo, player->mo->info->RPlayerRangedAttackState);
	
	if (!cv_infiniteammo.value)
		player->ammo[player->weaponinfo[player->readyweapon].ammo]--;
		
	P_SetPsprite(player, ps_flash, player->weaponinfo[player->readyweapon].flashstate + psp->state - &states[player->weaponinfo[player->readyweapon].atkstate]/*&states[S_CHAIN1]*/);
	
	P_BulletSlope(player->mo);
	P_GunShot(player->mo, !player->refire);
}

//
// Flash light when fire gun
//
void A_Light0(player_t* player, pspdef_t* psp)
{
	player->extralight = 0;
}

void A_Light1(player_t* player, pspdef_t* psp)
{
	player->extralight = 1;
}

void A_Light2(player_t* player, pspdef_t* psp)
{
	player->extralight = 2;
}

//
// A_BFGSpray
// Spawn a BFG explosion on every monster in view
//
void A_BFGSpray(mobj_t* mo)
{
	int i;
	int j;
	int damage;
	angle_t an;
	mobj_t* extrabfg;;
	
	// offset angles from its attack angle
	for (i = 0; i < 40; i++)
	{
		an = mo->angle - ANG90 / 2 + ANG90 / 40 * i;
		
		// mo->target is the originator (player)
		//  of the missile
		P_AimLineAttack(mo->target, an, 16 * 64 * FRACUNIT);
		
		if (!linetarget)
			continue;
			
		extrabfg = P_SpawnMobj(linetarget->x, linetarget->y, linetarget->z + (linetarget->height >> 2), MT_EXTRABFG);
		extrabfg->target = mo->target;
		
		damage = 0;
		for (j = 0; j < 15; j++)
			damage += (P_Random() & 7) + 1;
			
		//BP: use extramobj as inflictor so we have the good death message
		P_DamageMobj(linetarget, extrabfg, mo->target, damage);
	}
}

//
// A_BFGsound
//
void A_BFGsound(player_t* player, pspdef_t* psp)
{
	S_StartSound(&player->mo->NoiseThinker, sfx_bfg);
}

//
// P_SetupPsprites
// Called at start of level for each player.
//
void P_SetupPsprites(player_t* player)
{
	int i;
	
	// remove all psprites
	for (i = 0; i < NUMPSPRITES; i++)
		player->psprites[i].state = NULL;
		
	// spawn the gun
	player->pendingweapon = player->readyweapon;
	P_BringUpWeapon(player);
}

//
// P_MovePsprites
// Called every tic by player thinking routine.
//
void P_MovePsprites(player_t* player)
{
	int i;
	pspdef_t* psp;
	state_t* state;
	
	psp = &player->psprites[0];
	for (i = 0; i < NUMPSPRITES; i++, psp++)
	{
		// a null state means not active
		if ((state = psp->state))
		{
			// drop tic count and possibly change state
			
			// a -1 tic count never changes
			if (psp->tics != -1)
			{
				psp->tics--;
				if (!psp->tics)
					P_SetPsprite(player, i, psp->state->nextstate);
			}
		}
	}
	
	player->psprites[ps_flash].sx = player->psprites[ps_weapon].sx;
	player->psprites[ps_flash].sy = player->psprites[ps_weapon].sy;
}

// P_pspr.c

/*
#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "r_main.h"
#include "r_state.h"
#include "s_sound.h"
#include "m_random.h"
*/
// Macros

#define LOWERSPEED FRACUNIT*6
#define RAISESPEED FRACUNIT*6
#define WEAPONBOTTOM 128*FRACUNIT
#define WEAPONTOP 32*FRACUNIT
#define MAGIC_JUNK 1234
#define MAX_MACE_SPOTS 8

static int MaceSpotCount;
static struct
{
	fixed_t x;
	fixed_t y;
} MaceSpots[MAX_MACE_SPOTS];

fixed_t bulletslope;

weaponinfo_t wpnlev1info[NUMWEAPONS] =
{
	{
		// fist
		am_noammo,
		0,
		S_PUNCHUP,
		S_PUNCHDOWN,
		S_PUNCH,
		S_PUNCH1,
		S_PUNCH1,
		S_NULL
	}
	,
	{
		// pistol
		am_clip,
		1,
		S_PISTOLUP,
		S_PISTOLDOWN,
		S_PISTOL,
		S_PISTOL1,
		S_PISTOL1,
		S_PISTOLFLASH
	}
	,
	{
		// shotgun
		am_shell,
		1,
		S_SGUNUP,
		S_SGUNDOWN,
		S_SGUN,
		S_SGUN1,
		S_SGUN1,
		S_SGUNFLASH1
	}
	,
	{
		// chaingun
		am_clip,
		1,
		S_CHAINUP,
		S_CHAINDOWN,
		S_CHAIN,
		S_CHAIN1,
		S_CHAIN1,
		S_CHAINFLASH1
	}
	,
	{
		// missile launcher
		am_misl,
		1,
		S_MISSILEUP,
		S_MISSILEDOWN,
		S_MISSILE,
		S_MISSILE1,
		S_MISSILE1,
		S_MISSILEFLASH1
	}
	,
	{
		// plasma rifle
		am_cell,
		1,
		S_PLASMAUP,
		S_PLASMADOWN,
		S_PLASMA,
		S_PLASMA1,
		S_PLASMA1,
		S_PLASMAFLASH1
	}
	,
	{
		// bfg 9000
		am_cell,
		40,
		S_BFGUP,
		S_BFGDOWN,
		S_BFG,
		S_BFG1,
		S_BFG1,
		S_BFGFLASH1
	}
	,
	{
		// chainsaw
		am_noammo,
		0,
		S_SAWUP,
		S_SAWDOWN,
		S_SAW,
		S_SAW1,
		S_SAW1,
		S_NULL
	}
	,
	{
		// super shotgun
		am_shell,
		2,
		S_DSGUNUP,
		S_DSGUNDOWN,
		S_DSGUN,
		S_DSGUN1,
		S_DSGUN1,
		S_DSGUNFLASH1
	},
};

weaponinfo_t wpnlev2info[NUMWEAPONS] =
{

	{
		// fist
		am_noammo,
		0,
		S_PUNCHUP,
		S_PUNCHDOWN,
		S_PUNCH,
		S_PUNCH1,
		S_PUNCH1,
		S_NULL
	}
	,
	{
		// pistol
		am_clip,
		1,
		S_PISTOLUP,
		S_PISTOLDOWN,
		S_PISTOL,
		S_PISTOL1,
		S_PISTOL1,
		S_PISTOLFLASH
	}
	,
	{
		// shotgun
		am_shell,
		1,
		S_SGUNUP,
		S_SGUNDOWN,
		S_SGUN,
		S_SGUN1,
		S_SGUN1,
		S_SGUNFLASH1
	}
	,
	{
		// chaingun
		am_clip,
		1,
		S_CHAINUP,
		S_CHAINDOWN,
		S_CHAIN,
		S_CHAIN1,
		S_CHAIN1,
		S_CHAINFLASH1
	}
	,
	{
		// missile launcher
		am_misl,
		1,
		S_MISSILEUP,
		S_MISSILEDOWN,
		S_MISSILE,
		S_MISSILE1,
		S_MISSILE1,
		S_MISSILEFLASH1
	}
	,
	{
		// plasma rifle
		am_cell,
		1,
		S_PLASMAUP,
		S_PLASMADOWN,
		S_PLASMA,
		S_PLASMA1,
		S_PLASMA1,
		S_PLASMAFLASH1
	}
	,
	{
		// bfg 9000
		am_cell,
		40,
		S_BFGUP,
		S_BFGDOWN,
		S_BFG,
		S_BFG1,
		S_BFG1,
		S_BFGFLASH1
	}
	,
	{
		// chainsaw
		am_noammo,
		0,
		S_SAWUP,
		S_SAWDOWN,
		S_SAW,
		S_SAW1,
		S_SAW1,
		S_NULL
	}
	,
	{
		// super shotgun
		am_shell,
		2,
		S_DSGUNUP,
		S_DSGUNDOWN,
		S_DSGUN,
		S_DSGUN1,
		S_DSGUN1,
		S_DSGUNFLASH1
	},
};
