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
// Copyright (C) 1999 Lee Killough.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com).
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
		
		state = states[stnum];
		psp->state = state;
		psp->tics = state->tics;	// could be 0
		
		// GhostlyDeath <June 9, 2012> -- Faster Weapons
		if (P_EXGSGetValue(PEXGSBID_PLFASTERWEAPONS))
			if (psp->tics > 1)
				psp->tics = 1;
		
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
		if (state->action.acp5)
		{
			state->action.acp5(player->mo, player, psp, state->ArgC, state->ArgV);
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
	
	if (player->weaponinfo[player->pendingweapon]->BringUpSound)
		S_StartSound(&player->mo->NoiseThinker, S_SoundIDForName(player->weaponinfo[player->pendingweapon]->BringUpSound));
		
	// GhostlyDeath <November 3, 2010> -- PARANOIA removal
	if (player->pendingweapon >= NUMWEAPONS)
	{
		CONL_PrintF("WARNING - P_BringUpWeapon: %i (player->pendingweapon) >= %i (%s:%i).\n", player->pendingweapon, NUMWEAPONS, __FILE__, __LINE__);
		return;
	}
	
	newstate = player->weaponinfo[player->pendingweapon]->upstate;
	
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
	size_t i;
	weapontype_t BestWeapon;
	
	ammo = player->weaponinfo[player->readyweapon]->ammo;
	
	if (P_EXGSGetValue(PEXGSBID_PLINFINITEAMMO))
		return true;
		
	// Minimal amount for one shot varies.
	count = player->weaponinfo[player->readyweapon]->ammopershoot;
	
	// GhostlyDeath <June 12, 2012> -- All Ammo
	if (ammo == am_all)
	{
		// Check all ammo types
		for (i = 0; i < NUMAMMO; i++)
			if (player->ammo[i] < count)
				break;
		
		// Enough ammo?
		if (i >= NUMAMMO)
			return true;
	}
	
	// Some do not need ammunition anyway.
	// Return if current ammunition sufficient.
	else if (ammo == am_noammo || player->ammo[ammo] >= count)
		return true;
		
	// Out of ammo, pick a weapon to change to.
	// Preferences are set here.
	// added by Boris : preferred weapons order
	if (!player->originalweaponswitch)
		VerifFavoritWeapon(player);
	
	/* Find the best weapon, when out of ammo */
	// This uses NoAmmoOrder, the higher the better!
	for (BestWeapon = NUMWEAPONS, i = 0; i < NUMWEAPONS; i++)
	{
		// Cannot use weapon?
		if (!P_CanUseWeapon(player, i))
			continue;
		
		// Already using this gun?
		if (i == player->readyweapon)
			continue;
		
		// Got no ammo for this gun?
		if (player->ammo[player->weaponinfo[i]->ammo] < player->weaponinfo[i]->ammopershoot)
			continue;
		
		// Better than the best?
		if ((BestWeapon == NUMWEAPONS) || (BestWeapon != NUMWEAPONS && player->weaponinfo[i]->NoAmmoOrder > player->weaponinfo[BestWeapon]->NoAmmoOrder))
			BestWeapon = i;
	}
	
	// Switch to best?
	if (BestWeapon != NUMWEAPONS)
	{
		player->pendingweapon = BestWeapon;
	
		// Now set appropriate weapon overlay.
		P_SetPsprite(player, ps_weapon, player->weaponinfo[player->readyweapon]->downstate);
	}
	
	return false;
}

//
// P_FireWeapon.
//
void P_FireWeapon(player_t* player)
{
	statenum_t newstate;
	
	// GhostlyDeath <April 29, 2012> -- Object cannot use weapons?
	if (player->mo && !(player->mo->RXFlags[1] & MFREXB_CANUSEWEAPONS))
		return;
	
	if (!P_CheckAmmo(player))
		return;
	
	if (player->mo->info->RPlayerMeleeAttackState)
		P_SetMobjState(player->mo, player->mo->info->RPlayerMeleeAttackState);
	
	if (player->refire && player->weaponinfo[player->readyweapon]->holdatkstate != S_NULL)
		newstate = player->weaponinfo[player->readyweapon]->holdatkstate;
	else
		newstate = player->weaponinfo[player->readyweapon]->atkstate;
	
	P_SetPsprite(player, ps_weapon, newstate);
	
	// GhostlyDeath <June 7, 2012> -- Allow silenced weapons
	if (!(player->weaponinfo[player->readyweapon]->WeaponFlags & WF_NONOISEALERT))
		P_NoiseAlert(player->mo, player->mo);
}

//
// P_DropWeapon
// Player died, so put the weapon away.
//
void P_DropWeapon(player_t* player)
{
	P_SetPsprite(player, ps_weapon, player->weaponinfo[player->readyweapon]->downstate);
}

//
// A_WeaponReady
// The player can fire the weapon
// or change to another weapon at this time.
// Follows after getting weapon up,
// or after previous attack/fire sequence.
//
void A_WeaponReady(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
{

	// get out of attack state
	if (player->mo->state == states[player->mo->info->RPlayerRangedAttackState] || player->mo->state == states[player->mo->info->RPlayerMeleeAttackState])
	{
		P_SetMobjState(player->mo, player->mo->info->spawnstate);
	}
	
	// GhostlyDeath <April 14, 2012> -- Chainsaw buzzing
	if (player->weaponinfo[player->readyweapon]->IdleNoise)
		if (psp->state == states[player->weaponinfo[player->readyweapon]->readystate])
			S_StartSound(&player->mo->NoiseThinker, S_SoundIDForName(player->weaponinfo[player->readyweapon]->IdleNoise));
	
	// check for change
	//  if player is dead, put the weapon away
	if (player->pendingweapon != wp_nochange || !player->health)
	{
		// change weapon
		//  (pending weapon should allready be validated)
		P_SetPsprite(player, ps_weapon, player->weaponinfo[player->readyweapon]->downstate);
		return;
	}
	// check for fire
	//  the missile launcher and bfg do not auto fire
	if (player->cmd.buttons & BT_ATTACK)
	{
		if (!player->attackdown || !(player->weaponinfo[player->readyweapon]->WeaponFlags & WF_NOAUTOFIRE))
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
void A_TicWeapon(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
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

/* P_ReduceAmmo() -- Reduces player ammo */
void P_ReduceAmmo(player_t* player)
{
	if (!P_EXGSGetValue(PEXGSBID_PLINFINITEAMMO))
		player->ammo[player->weaponinfo[player->readyweapon]->ammo] -= player->weaponinfo[player->readyweapon]->ammopershoot;
}

//
// A_ReFire
// The player can re-fire the weapon
// without lowering it entirely.
//
void A_ReFire(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
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

void A_CheckReload(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
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
void A_Lower(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
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
void A_Raise(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
{
	psp->sy -= RAISESPEED;
	
	if (psp->sy > WEAPONTOP)
		return;
		
	psp->sy = WEAPONTOP;
	
	// The weapon has been raised all the way,
	//  so change to the ready state.
	P_SetPsprite(player, ps_weapon, player->weaponinfo[player->readyweapon]->readystate);
}

//
// A_GunFlash
//
void A_GunFlash(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
{
	P_SetMobjState(player->mo, player->mo->info->RPlayerRangedAttackState);
	P_SetPsprite(player, ps_flash, player->weaponinfo[player->readyweapon]->flashstate);
}

//
// WEAPON ATTACKS
//

/* PS_GetPuffType() -- Gets the replacement puff */
mobjtype_t PS_GetPuffType(player_t* player)
{
	weaponinfo_t* Weapon;
	
	Weapon = player->weaponinfo[player->readyweapon];
	
	/* Which is returned? */
	if (Weapon->ReplacePuffType)
		return INFO_GetTypeByName(Weapon->ReplacePuffType);
	else
		return INFO_GetTypeByName("BulletPuff");
}

//
// A_Punch
//
void A_Punch(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
{
	angle_t angle;
	angle_t* locang = NULL;
	int damage;
	int slope;
	int i;
	
	angle_t victimangle = 0;
	angle_t myangle = 0;
	angle_t actualangle = 0;
	angle_t virtualangle = 0;
	int someactualangle = 0;
	int somevirtualangle = 0;
	int somemyangle = 0;
	int someoffset = 0;
	
	PuffType = PS_GetPuffType(player);
	damage = (P_Random() % 10 + 1) << 1;
	
	if (player->powers[pw_strength])
		damage *= 10;
		
	angle = player->mo->angle;
	angle += (P_Random() << 18);	// WARNING: don't put this in one line
	angle -= (P_Random() << 18);	// else this expretion is ambiguous (evaluation order not diffined)
	
	slope = P_AimLineAttack(player->mo, angle, MELEERANGE, NULL);
	P_LineAttack(player->mo, angle, MELEERANGE, slope, damage, NULL);
	
	// turn to face target
	if (linetarget)
	{
		S_StartSound(&player->mo->NoiseThinker, sfx_punch);
		player->mo->angle = R_PointToAngle2(player->mo->x, player->mo->y, linetarget->x, linetarget->y);
		
		// GhostlyDeath -- Affect Local Aiming yknow
		for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
			if (playeringame[consoleplayer[i]] && player == &players[consoleplayer[i]])
				locang = &localangle[i];
				
		if (locang)
		{
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
void A_Saw(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
{
	angle_t angle;
	angle_t* locang = NULL;
	int damage;
	int slope;
	int i;
	
	PuffType = PS_GetPuffType(player);
	damage = 2 * (P_Random() % 10 + 1);
	angle = player->mo->angle;
	angle += (P_Random() << 18);	// WARNING: don't put this in one line
	angle -= (P_Random() << 18);	// else this expretion is ambiguous (evaluation order not diffined)
	
	// use meleerange + 1 se the puff doesn't skip the flash
	slope = P_AimLineAttack(player->mo, angle, MELEERANGE + 1, NULL);
	P_LineAttack(player->mo, angle, MELEERANGE + 1, slope, damage, NULL);
	
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
	
	// GhostlyDeath -- Affect Local Aiming yknow
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
void A_FireMissile(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
{
	P_ReduceAmmo(player);
	
	//added:16-02-98: added player arg3
	P_SpawnPlayerMissile(player->mo, INFO_GetTypeByName("RocketShot"));
}

//
// A_FireBFG
//
void A_FireBFG(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
{
	P_ReduceAmmo(player);
	//added:16-02-98:added player arg3
	P_SpawnPlayerMissile(player->mo, INFO_GetTypeByName("BFGShot"));
}

//
// A_FirePlasma
//
void A_FirePlasma(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
{
	int StateDiff;
	
	P_ReduceAmmo(player);
	
	// GhostlyDeath <April 29, 2012> -- Set corresponding flash state
	StateDiff = P_Random() & 1;
	if (StateDiff < player->weaponinfo[player->readyweapon]->NumFlashStates)
		P_SetPsprite(player, ps_flash, player->weaponinfo[player->readyweapon]->FlashStates[StateDiff]);
	
	//added:16-02-98: added player arg3
	P_SpawnPlayerMissile(player->mo, INFO_GetTypeByName("PlasmaShot"));
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
	if (!P_EXGSGetValue(PEXGSBID_COFORCEAUTOAIM))
		if (!mo->player->autoaim_toggle || !P_EXGSGetValue(PEXGSBID_PLALLOWAUTOAIM) || !P_EXGSGetValue(PEXGSBID_COMOUSEAIM))
			goto notagetfound;
		
	// see which target is to be aimed at
	an = mo->angle;
	bulletslope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT, NULL);
	
	if (!linetarget)
	{
		an += 1 << 26;
		bulletslope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT, NULL);
		
		if (!linetarget)
		{
			an -= 2 << 26;
			bulletslope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT, NULL);
		}
		
		// GhostlyDeath <June 17, 2012> -- Only when mouse aiming is angle used
		if (P_EXGSGetValue(PEXGSBID_COMOUSEAIM))
			if (!linetarget)
			{
notagetfound:
				if (P_EXGSGetValue(PEXGSBID_COENABLEUPDOWNSHOOT))
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
	P_LineAtkArgs_t Args;
	
	/* Clear arguments */
	memset(&Args, 0, sizeof(Args));
	
	damage = 5 * (P_Random() % 3 + 1);
	angle = mo->angle;
	
	if (!accurate)
	{
		// GhostlyDeath <June 17, 2012> -- Demo Comp
			// This seems pretty ugly and is probably what breaks demos
		if (P_EXGSGetValue(PEXGSBID_CONEWGUNSHOTCODE))
		{
			angle += (P_Random() << 18);	// WARNING: don't put this in one line
			angle -= (P_Random() << 18);	// else this expretion is ambiguous (evaluation order not diffined)
		}
		
		// Can really just use signed random here.
		else
			angle += P_SignedRandom() << 18;
	}
	
	Args.Flags |= PLAF_THRUMOBJ;
	P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage, &Args);
}

//
// A_FirePistol
//
void A_FirePistol(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
{
	S_StartSound(&player->mo->NoiseThinker, sfx_pistol);
	
	PuffType = PS_GetPuffType(player);
	P_SetMobjState(player->mo, player->mo->info->RPlayerRangedAttackState);
	
	P_ReduceAmmo(player);
		
	P_SetPsprite(player, ps_flash, player->weaponinfo[player->readyweapon]->flashstate);
	
	P_BulletSlope(player->mo);
	P_GunShot(player->mo, !player->refire);
}

//
// A_FireShotgun
//
void A_FireShotgun(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
{
	int i;
	
	PuffType = PS_GetPuffType(player);
	S_StartSound(&player->mo->NoiseThinker, sfx_shotgn);
	P_SetMobjState(player->mo, player->mo->info->RPlayerRangedAttackState);
	
	P_ReduceAmmo(player);
	P_SetPsprite(player, ps_flash, player->weaponinfo[player->readyweapon]->flashstate);
	
	P_BulletSlope(player->mo);
	for (i = 0; i < 7; i++)
		P_GunShot(player->mo, false);
}

//
// A_FireShotgun2 (SuperShotgun)
//
void A_FireShotgun2(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
{
	int i;
	angle_t angle;
	int damage;
	
	PuffType = PS_GetPuffType(player);
	S_StartSound(&player->mo->NoiseThinker, sfx_dshtgn);
	P_SetMobjState(player->mo, player->mo->info->RPlayerRangedAttackState);
	
	P_ReduceAmmo(player);
		
	P_SetPsprite(player, ps_flash, player->weaponinfo[player->readyweapon]->flashstate);
	
	P_BulletSlope(player->mo);
	
	for (i = 0; i < 20; i++)
	{
		int slope = bulletslope + (P_SignedRandom() << 5);
		
		damage = 5 * (P_Random() % 3 + 1);
		angle = player->mo->angle + (P_SignedRandom() << 19);
		P_LineAttack(player->mo, angle, MISSILERANGE, slope, damage, NULL);
	}
}

//
// A_FireCGun
//
void A_FireCGun(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
{
	int StateDiff;
	
	S_StartSound(&player->mo->NoiseThinker, sfx_pistol);
	
	if (!P_EXGSGetValue(PEXGSBID_PLINFINITEAMMO))
		if (!player->ammo[player->weaponinfo[player->readyweapon]->ammo])
			return;
			
	PuffType = PS_GetPuffType(player);
	P_SetMobjState(player->mo, player->mo->info->RPlayerRangedAttackState);
	
	P_ReduceAmmo(player);
	
	// GhostlyDeath <April 29, 2012> -- Set corresponding flash state
	StateDiff = player->psprites[ps_weapon].state->FrameID - 1;
	if (StateDiff < player->weaponinfo[player->readyweapon]->NumFlashStates)
		P_SetPsprite(player, ps_flash, player->weaponinfo[player->readyweapon]->FlashStates[StateDiff]);
	
	P_BulletSlope(player->mo);
	P_GunShot(player->mo, !player->refire);
}

//
// Flash light when fire gun
//
void A_Light0(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
{
	player->extralight = 0;
}

void A_Light1(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
{
	player->extralight = 1;
}

void A_Light2(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
{
	player->extralight = 2;
}

//
// A_BFGSpray
// Spawn a BFG explosion on every monster in view
//
void A_BFGSpray(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
{
	int i;
	int j;
	int damage;
	angle_t an;
	mobj_t* extrabfg;
	mobj_t* BallOwner;
	weapontype_t OldWeapon;
	
	// Remember stuff about object
	BallOwner = mo->target;
	
	if (BallOwner)
		OldWeapon = BallOwner->RXShotWithWeapon;
	else
	{
		BallOwner = mo;
		OldWeapon = NUMWEAPONS;
	}
	
	// Set owner weapon to BFG, etc.
	BallOwner->RXShotWithWeapon = mo->RXShotWithWeapon;
	
	// offset angles from its attack angle
	for (i = 0; i < 40; i++)
	{
		an = mo->angle - ANG90 / 2 + ANG90 / 40 * i;
		
		// mo->target is the originator (player)
		//  of the missile
		P_AimLineAttack(BallOwner, an, 16 * 64 * FRACUNIT, NULL);
		
		if (!linetarget)
			continue;
		
		extrabfg = P_SpawnMobj(linetarget->x, linetarget->y, linetarget->z + (linetarget->height >> 2), INFO_GetTypeByName("BFGFlash"));
		P_RefMobj(PMRT_TARGET, extrabfg, BallOwner);
		extrabfg->RXShotWithWeapon = mo->RXShotWithWeapon;
		
		damage = 0;
		for (j = 0; j < 15; j++)
			damage += (P_Random() & 7) + 1;
		
		// GhostlyDeath <June 17, 2012> -- Compatible BFG
		if (P_EXGSGetValue(PEXGSBID_COOLDBFGSPRAY))
			P_DamageMobj(linetarget, mo->target, mo->target, damage);
		
		// This breaks demo comp all for obituaries! Great!
		else
			//BP: use extramobj as inflictor so we have the good death message
			P_DamageMobj(linetarget, extrabfg, mo->target, damage);
	}
	
	// Set owner weapon to BFG, etc.
	BallOwner->RXShotWithWeapon = OldWeapon;
}

//
// A_BFGsound
//
void A_BFGsound(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
{
	S_StartSound(&mo->NoiseThinker, sfx_bfg);
}

/* A_FireGenericProjectile() -- Fires a generic projectile */
void A_FireGenericProjectile(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
{
	/* Reduce ammo */
	P_ReduceAmmo(player);
	
	/* Spawn the projectile */
	if (player->weaponinfo[player->readyweapon]->GenericProjectile)
		P_SpawnPlayerMissile(player->mo, INFO_GetTypeByName(player->weaponinfo[player->readyweapon]->GenericProjectile));
}

/*** BETA BFG ***/
// This function emulates Doom's Pre-Beta BFG
// By Lee Killough 6/6/98, 7/11/98, 7/19/98, 8/20/98
//
// This code may not be used in other mods without appropriate credit given.
// Code leeches will be telefragged.
//
// GhostlyDeath <April 15, 2012> -- The GPL requires this anyway so why bother
// stating it? But eitherway, this is modified for ReMooD purposes.

/* A_FireOldBFG() -- Fires the BFG from Beta Doom */
void A_FireOldBFG(mobj_t* mo, player_t* player, pspdef_t* psp, const INFO_StateArgsNum_t a_ArgC, INFO_StateArgsParm_t* const a_ArgV)
{
	angle_t la, lb, lc;
	mobj_t* pMo, *BallMo;
	size_t i;
	
	/* Light up the area */
	player->extralight = 2;
	
	/* Get player object */
	pMo = player->mo;
	
	if (!P_CheckAmmo(player))
		return;
	
	// Reduce ammo
	P_ReduceAmmo(player);
	
	/* Fire Loop */
	for (i = 0; i < 2; i++)
	{
		// Get random aiming angles
		la = pMo->angle;
		lb = (((int)(P_Random() & 0x7F)) - 64) * (ANG90 / 768) + la;
		lc = (((int)(P_Random() & 0x7F)) - 64) * (ANG90 / 640) + ANG90;
		
		// Spawn fireball
		BallMo = P_SpawnPlayerMissile(pMo, (i == 0 ? INFO_GetTypeByName("LegacyPlasma1") : INFO_GetTypeByName("LegacyPlasma2")));
		
		// Modify angle
		if (BallMo)
		{
			// Modify angle and momentum
			BallMo->angle = lb;
			BallMo->momx = finecosine[lb >> ANGLETOFINESHIFT] * 25;
			BallMo->momy = finesine[lb >> ANGLETOFINESHIFT] * 25;
			BallMo->momz += finetangent[lc >> ANGLETOFINESHIFT] * 25;
		}
	}
	
#if 0
	do
	{
		mobj_t *th, *mo = player->mo;
		angle_t an = mo->angle;
		angle_t an1 = ((P_Random(pr_bfg)&127) - 64) * (ANG90/768) + an;
		angle_t an2 = ((P_Random(pr_bfg)&127) - 64) * (ANG90/640) + ANG90;
		extern int autoaim;

		if (autoaim || !beta_emulation)
		{
			// killough 8/2/98: make autoaiming prefer enemies
			int mask = MF_FRIEND;
			fixed_t slope;
			do
			{
				slope = P_AimLineAttack(mo, an, 16*64*FRACUNIT, mask);
				if (!linetarget)
				slope = P_AimLineAttack(mo, an += 1<<26, 16*64*FRACUNIT, mask);
				if (!linetarget)
				slope = P_AimLineAttack(mo, an -= 2<<26, 16*64*FRACUNIT, mask);
				if (!linetarget)
				slope = 0, an = mo->angle;
			}
			while (mask && (mask=0, !linetarget));     // killough 8/2/98
			an1 += an - mo->angle;
			an2 += tantoangle[slope >> DBITS];
		}

		th = P_SpawnMobj(mo->x, mo->y, mo->z + 62*FRACUNIT - player->psprites[ps_weapon].sy, type);
		P_SetTarget(&th->target, mo);
		th->angle = an1;
		th->momx = finecosine[an1>>ANGLETOFINESHIFT] * 25;
		th->momy = finesine[an1>>ANGLETOFINESHIFT] * 25;
		th->momz = finetangent[an2>>ANGLETOFINESHIFT] * 25;
		P_CheckMissileSpawn(th);
	}
	while ((type != MT_PLASMA2) && (type = MT_PLASMA2)); //killough: obfuscated!
#endif
}

/****************/

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
	
	// GhostlyDeath <May 8, 2012> -- If playing as monster, do not move sprites
	if (player->mo && !(player->mo->RXFlags[0] & MFREXA_ISPLAYEROBJECT))
	{
		// Only handle attack button
		if (player->cmd.buttons & BT_ATTACK)
			player->attackdown = true;
		else
			player->attackdown = false;
		return;
	}
	
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

static int MaceSpotCount;
static struct
{
	fixed_t x;
	fixed_t y;
} MaceSpots[MAX_MACE_SPOTS];

fixed_t bulletslope;

/* INFO_GetWeaponByName() -- Return weapon by name */
weapontype_t INFO_GetWeaponByName(const char* const a_Name)
{
	size_t i;
	
	/* Check */
	if (!a_Name)
		return NUMWEAPONS;
	
	/* Loop */
	for (i = 0; i < NUMWEAPONS; i++)
		if (strcasecmp(a_Name, wpnlev1info[i]->ClassName) == 0)
			return i;
	
	/* Failed */
	return NUMWEAPONS;
}

/* INFO_GetAmmoByName() -- Return ammo by name */
ammotype_t INFO_GetAmmoByName(const char* const a_Name)
{
	size_t i;
	
	/* Check */
	if (!a_Name)
		return am_noammo;
	
	/* Special Names */
	if (strcasecmp(a_Name, "noammo") == 0)
		return am_noammo;
	else if (strcasecmp(a_Name, "all") == 0)
		return am_all;
	
	/* Loop */
	for (i = 0; i < NUMAMMO; i++)
		if (strcasecmp(a_Name, ammoinfo[i]->ClassName) == 0)
			return i;
	
	/* Failed */
	return am_noammo;
}

/****************************
*** RMOD WEAPONS AND AMMO ***
****************************/

/*** CONSTANTS ***/

/*** STRUCTURES ***/

/* P_LocalWeaponsAndAmmo_t -- Local Weapons and ammo */
typedef struct P_LocalWeaponsAndAmmo_s
{
	weaponinfo_t** Weapons;
	size_t NumWeapons;
	
	ammoinfo_t** Ammo;
	size_t NumAmmo;
	
#if defined(__REMOOD_USEFLATTERSTATES)
	state_t* WeaponStates;
#else
	state_t** WeaponStates;
#endif
	size_t NumWeaponStates;
	size_t MaxWeaponStates;
} P_LocalWeaponsAndAmmo_t;

/* P_WepAmmoTransfer_t -- Transfer handler */
typedef struct P_WepAmmoTransfer_s
{
	P_LocalWeaponsAndAmmo_t* Local;
	weaponinfo_t* Weapon;
	int* StateValueP;
	int BaseStateNum;
	int NumStateNum;
	uint32_t WeaponID;
	int* StateSplasher;
	
	P_WeaponStateGroup_t StateGroup;
} P_WepAmmoTransfer_t;

/*** GLOBALS ***/
weaponinfo_t** wpnlev1info = NULL;
weaponinfo_t** wpnlev2info = NULL;
size_t NUMWEAPONS = 0;

ammoinfo_t** ammoinfo = NULL;
size_t NUMAMMO = 0;

/*** FUNCTIONS ***/

/* PS_RMODWeaponStateForName() -- Determines state value for name */
static statenum_t* PS_RMODWeaponStateForName(void* const a_Input, const char* const a_Name, INFO_ObjectStateGroup_t* const WSG, uint32_t** const a_RefState, uint32_t*** const a_LRefs, size_t** a_NumLRefs)
{
	weaponinfo_t* Weapon = a_Input;	
	
	/* Check */
	if (!Weapon || !a_Name)
		return NULL;
	
	/* Now which one? */
	if (strcasecmp(a_Name, "PrimaryBringUpState") == 0)
	{
		*WSG = PWSG_UP;
		if (a_RefState)
			*a_RefState = &Weapon->RefStates[*WSG];
		return &Weapon->upstate;
	}
	else if (strcasecmp(a_Name, "PrimaryPutDownState") == 0)
	{
		*WSG = PWSG_DOWN;
		if (a_RefState)
			*a_RefState = &Weapon->RefStates[*WSG];
		return &Weapon->downstate;
	}
	else if (strcasecmp(a_Name, "PrimaryReadyState") == 0)
	{
		*WSG = PWSG_READY;
		if (a_RefState)
			*a_RefState = &Weapon->RefStates[*WSG];
		return &Weapon->readystate;
	}
	else if (strcasecmp(a_Name, "PrimaryFireState") == 0)
	{
		*WSG = PWSG_ATTACK;
		if (a_RefState)
			*a_RefState = &Weapon->RefStates[*WSG];
		return &Weapon->atkstate;
	}
	else if (strcasecmp(a_Name, "PrimaryFireHeldState") == 0)
	{
		*WSG = PWSG_HOLDATTACK;
		if (a_RefState)
			*a_RefState = &Weapon->RefStates[*WSG];
		return &Weapon->holdatkstate;
	}
	else if (strcasecmp(a_Name, "PrimaryFlashState") == 0)
	{
		*WSG = PWSG_FLASH;
		if (a_RefState)
			*a_RefState = &Weapon->RefStates[*WSG];
		return &Weapon->flashstate;
	}
	
	/* Not Found */
	*WSG = -1;
	if (a_RefState)
		*a_RefState = NULL;
	return NULL;
}

/* P_RMODH_WeaponsAmmo() -- Handler for Weapons */
bool_t P_RMODH_WeaponsAmmo(Z_Table_t* const a_Table, const WL_WADFile_t* const a_WAD, const D_RMODPrivates_t a_ID, D_RMODPrivate_t* const a_Private)
{
	P_LocalWeaponsAndAmmo_t* LocalStuff;
	D_RMODPrivate_t* RealPrivate;
	const char* Value;
	weaponinfo_t TempWeapon;
	ammoinfo_t TempAmmo;
	P_WepAmmoTransfer_t WAT;
	INFO_RMODStateHelper_t Helper;
	static uint32_t WeaponIDBase;
	
	/* Check */
	if (!a_Table || !a_WAD || !a_Private)
		return false;
	
	/* Clear */
	memset(&WAT, 0, sizeof(WAT));
	memset(&TempWeapon, 0, sizeof(TempWeapon));
	memset(&TempAmmo, 0, sizeof(TempAmmo));
	
	/* Obtain private info for ammo data */
	RealPrivate = D_GetRMODPrivate(a_WAD, DRMODP_ITEMAMMO);
	
	// No real private?
	if (!RealPrivate)
		RealPrivate = a_Private;
	
	/* Create specials list */
	// Does not exist
	if (!RealPrivate->Data)
	{
		RealPrivate->Size = sizeof(*LocalStuff);
		RealPrivate->Data = Z_Malloc(RealPrivate->Size, PU_WLDKRMOD, (void**)&RealPrivate->Data);
	}
	
	// Get the local stuff
	LocalStuff = RealPrivate->Data;
	
	/* Get ClassName */
	Value = Z_TableName(a_Table);
	
	// Knock off #
	Value = strchr(Value, '#');
	
	// Not found?
	if (!Value)
		return false;
	
	// Add 1 to remove #
	Value++;
	
	// Early Boot Notice
	if (g_EarlyBootConsole)
		CONL_EarlyBootTic(Value, true);
	
	/* Ammunition */
	if (a_ID == DRMODP_ITEMAMMO)
	{
		// Copy Class
		TempAmmo.ClassName = Z_StrDup(Value, PU_WLDKRMOD, NULL);
		
		// Get Values
		TempAmmo.ClipAmmo = D_RMODGetValueInt(a_Table, "ClipAmmo", 0);
		TempAmmo.MaxAmmo = D_RMODGetValueInt(a_Table, "MaxAmmo", 0);
		TempAmmo.StartingAmmo = D_RMODGetValueInt(a_Table, "StartingAmmo", 0);
		
		// Add to end
		Z_ResizeArray((void**)&LocalStuff->Ammo, sizeof(*LocalStuff->Ammo), LocalStuff->NumAmmo, LocalStuff->NumAmmo + 1);
		LocalStuff->Ammo[LocalStuff->NumAmmo] = Z_Malloc(sizeof(*LocalStuff->Ammo[LocalStuff->NumAmmo]), PU_WLDKRMOD, NULL);
		memmove(LocalStuff->Ammo[LocalStuff->NumAmmo], &TempAmmo, sizeof(TempAmmo));
		LocalStuff->NumAmmo++;
		return true;
	}
	
	/* Weapons */
	else if (a_ID == DRMODP_ITEMWEAPON)
	{
		// Copy Class
		TempWeapon.ClassName = Z_StrDup(Value, PU_WLDKRMOD, NULL);
		
		// Get Values
		TempWeapon.ammopershoot = D_RMODGetValueInt(a_Table, "AmmoPerShot", 0);
		TempWeapon.SwitchOrder = D_RMODGetValueInt(a_Table, "SwitchOrder", 0);
		TempWeapon.GetAmmo = D_RMODGetValueInt(a_Table, "PickupAmmo", 0);
		TempWeapon.SlotNum = D_RMODGetValueInt(a_Table, "SlotNum", 0);
		TempWeapon.NoAmmoOrder = D_RMODGetValueInt(a_Table, "NoAmmoSwitchOrder", 0);
		
		// Get Booleans
		if (D_RMODGetValueBool(a_Table, "IsDoom", false))
			TempWeapon.WeaponFlags |= WF_ISDOOM;
		if (D_RMODGetValueBool(a_Table, "IsHeretic", false))
			TempWeapon.WeaponFlags |= WF_ISHERETIC;
		if (D_RMODGetValueBool(a_Table, "IsHexen", false))
			TempWeapon.WeaponFlags |= WF_ISHEXEN;
		if (D_RMODGetValueBool(a_Table, "IsStrife", false))
			TempWeapon.WeaponFlags |= WF_ISSTRIFE;
		if (D_RMODGetValueBool(a_Table, "IsNotShareware", false))
			TempWeapon.WeaponFlags |= WF_NOTSHAREWARE;
		if (D_RMODGetValueBool(a_Table, "IsInCommercial", false))
			TempWeapon.WeaponFlags |= WF_INCOMMERCIAL;
		if (D_RMODGetValueBool(a_Table, "IsRegistered", false))
			TempWeapon.WeaponFlags |= WF_INREGISTERED;
		if (D_RMODGetValueBool(a_Table, "IsBerserkToggle", false))
			TempWeapon.WeaponFlags |= WF_BERSERKTOGGLE;
		if (D_RMODGetValueBool(a_Table, "IsSwitchFromNoAmmo", false))
			TempWeapon.WeaponFlags |= WF_SWITCHFROMNOAMMO;
		if (D_RMODGetValueBool(a_Table, "IsStartingWeapon", false))
			TempWeapon.WeaponFlags |= WF_STARTINGWEAPON;
		if (D_RMODGetValueBool(a_Table, "NoThrust", false))
			TempWeapon.WeaponFlags |= WF_NOTHRUST;
		if (D_RMODGetValueBool(a_Table, "NoAutoFire", false))
			TempWeapon.WeaponFlags |= WF_NOAUTOFIRE;
		if (D_RMODGetValueBool(a_Table, "IsInExtended", false))
			TempWeapon.WeaponFlags |= WF_INEXTENDED;
		if (D_RMODGetValueBool(a_Table, "NoBleedTarget", false))
			TempWeapon.WeaponFlags |= WF_NOBLEEDTARGET;
		if (D_RMODGetValueBool(a_Table, "IsSuperWeapon", false))
			TempWeapon.WeaponFlags |= WF_SUPERWEAPON;
		if (D_RMODGetValueBool(a_Table, "NoNoiseAlert", false))
			TempWeapon.WeaponFlags |= WF_NONOISEALERT;
		
		// Get Fixed
		TempWeapon.PSpriteSY = D_RMODGetValueFixed(a_Table, "SpriteYOffset", 0);
		
		// Get Strings
		TempWeapon.DropWeaponClass = D_RMODGetValueString(a_Table, "DroppedObject", NULL);
		TempWeapon.NiceName = D_RMODGetValueString(a_Table, "NiceName", TempWeapon.ClassName);
		TempWeapon.SBOGraphic = D_RMODGetValueString(a_Table, "SBOGraphic", NULL);
		TempWeapon.BringUpSound = D_RMODGetValueString(a_Table, "BringUpSound", NULL);
		TempWeapon.IdleNoise = D_RMODGetValueString(a_Table, "IdleNoise", NULL);
		TempWeapon.AmmoClass = D_RMODGetValueString(a_Table, "Ammo", NULL);
		TempWeapon.ReplacePuffType = D_RMODGetValueString(a_Table, "PuffClass", NULL);
		TempWeapon.GenericProjectile = D_RMODGetValueString(a_Table, "GenericProjectile", NULL);
		TempWeapon.TracerSplat = D_RMODGetValueString(a_Table, "TracerSplat", NULL);
		
		// Weapon ID (A somewhat unique number)
		TempWeapon.WeaponID = (((uint32_t)(M_Random() & 0xFF)) | ((++WeaponIDBase) << 8)) | 0x80000000UL;
		
		// Bot Metrics
		if ((Value = Z_TableGetValue(a_Table, "BotMetric")))
			TempWeapon.BotMetric = INFO_BotMetricByName(Value);
		
		// Handle States
		//WAT.Local = LocalStuff;
		//WAT.Weapon = &TempWeapon;
		//WAT.WeaponID = TempWeapon.WeaponID;
		//Z_TableSuperCallback(a_Table, PS_RMODWeaponStateHandlers, (void*)&WAT);
		
		memset(&Helper, 0, sizeof(Helper));
		
		Helper.StatesRef = &LocalStuff->WeaponStates;
		Helper.NumStatesRef = &LocalStuff->NumWeaponStates;
		Helper.MaxStatesRef = &LocalStuff->MaxWeaponStates;
		Helper.StateForName = PS_RMODWeaponStateForName;
		Helper.InputPtr = &TempWeapon;
		Helper.ObjectID = TempWeapon.WeaponID;
		
		Z_TableSuperCallback(a_Table, INFO_RMODStateHandlers, (void*)&Helper);
		
		// Add to end
		Z_ResizeArray((void**)&LocalStuff->Weapons, sizeof(*LocalStuff->Weapons), LocalStuff->NumWeapons, LocalStuff->NumWeapons + 1);
		LocalStuff->Weapons[LocalStuff->NumWeapons] = Z_Malloc(sizeof(*LocalStuff->Weapons[LocalStuff->NumWeapons]), PU_WLDKRMOD, NULL);
		memmove(LocalStuff->Weapons[LocalStuff->NumWeapons], &TempWeapon, sizeof(TempWeapon));
		LocalStuff->NumWeapons++;
		return true;
	}
	
	/* Unknown? */
	else
		return false;
}

/* P_RMODO_WeaponsAmmo() -- Order for Weapons and Ammo */
bool_t P_RMODO_WeaponsAmmo(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD, const D_RMODPrivates_t a_ID)
{
	const WL_WADFile_t* RoveWAD;
	P_LocalWeaponsAndAmmo_t* LocalStuff;
	weaponinfo_t* TempWeapon;
	ammoinfo_t* TempAmmo;
	D_RMODPrivate_t* RMODPrivate;
	size_t i, j, k, FoundID, Base, Count, MergeBase, MergeCount;
	int* StateRef;
	uint32_t WepID, RefToFind;
	
	/* Reset */
	MergeBase = (size_t)-1;
	
	/* Clear old weapons and ammo */
	// Player Arrays (otherwise, kaboom!)
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (players[i].weaponowned)
			Z_Free(players[i].weaponowned);
		players[i].weaponowned = NULL;
		if (players[i].ammo)
			Z_Free(players[i].ammo);
		players[i].ammo = NULL;
		if (players[i].maxammo)
			Z_Free(players[i].maxammo);
		players[i].maxammo = NULL;
	}
	
	// Weapons
	if (wpnlev1info)
		Z_Free(wpnlev1info);
	wpnlev1info = NULL;
	if (wpnlev2info)
		Z_Free(wpnlev2info);
	wpnlev2info = NULL;
	NUMWEAPONS = 0;
	
	// Ammo
	if (ammoinfo)
		Z_Free(ammoinfo);
	ammoinfo = NULL;
	NUMAMMO = 0;
	
	// State initialization used to be here, now it is done in the INFO code
	
	/* Go through every WAD */
	for (RoveWAD = WL_IterateVWAD(NULL, true); RoveWAD; RoveWAD = WL_IterateVWAD(RoveWAD, true))
	{
		// Obtain private menu stuff for this WAD
		RMODPrivate = D_GetRMODPrivate(RoveWAD, a_ID);
		
		// Not found? Ignore this WAD then
		if (!RMODPrivate)
			continue;
		
		// Load menu stuff
		LocalStuff = RMODPrivate->Data;
		
		// Not found?
		if (!LocalStuff)
			continue;
		
		// Add ammo from this, overwriting everything
		for (i = 0; i < LocalStuff->NumAmmo; i++)
		{
			// Get Current
			TempAmmo = LocalStuff->Ammo[i];
			
			// See if it already exists
			FoundID = INFO_GetAmmoByName(TempAmmo->ClassName);
			
			// Not found? Add to end
			if (FoundID == NUMAMMO || FoundID == am_noammo)
			{
				Z_ResizeArray((void**)&ammoinfo, sizeof(*ammoinfo), NUMAMMO, NUMAMMO + 1);
				ammoinfo[NUMAMMO++] = TempAmmo;
			}
			
			// Found, replace
			else
			{
				// Replace here
				ammoinfo[FoundID] = TempAmmo;
			}
		}
		
		// Add weapons from this too
		for (i = 0; i < LocalStuff->NumWeapons; i++)
		{
			// Get Current
			TempWeapon = LocalStuff->Weapons[i];
			
			// See if it already exists
			FoundID = INFO_GetWeaponByName(TempWeapon->ClassName);
			
			// Not found? Add to end
			if (FoundID == NUMWEAPONS)
			{
				Z_ResizeArray((void**)&wpnlev1info, sizeof(*wpnlev1info), NUMWEAPONS, NUMWEAPONS + 1);
				wpnlev1info[NUMWEAPONS] = TempWeapon;
				
				Z_ResizeArray((void**)&wpnlev2info, sizeof(*wpnlev2info), NUMWEAPONS, NUMWEAPONS + 1);
				wpnlev2info[NUMWEAPONS] = TempWeapon;
				
				// Increase weapons
				NUMWEAPONS++;
			}
			
			// Found, replace
			else
			{
				// Replace here
				wpnlev1info[FoundID] = TempWeapon;
				wpnlev2info[FoundID] = TempWeapon;
			}
		}
		
		// Push all states
		if (LocalStuff->NumWeaponStates)
		{
			Base = NUMSTATES;
			Count = LocalStuff->NumWeaponStates;
			
			if (MergeBase == (size_t)-1)
			{
				MergeBase = Base;
				MergeCount = Count;
			}
			else
				MergeCount += Count;
			
			// Resize array of states
			Z_ResizeArray((void**)&states, sizeof(*states), NUMSTATES, NUMSTATES + Count);
			NUMSTATES += Count;
			
			// Reference every single state
			for (j = 0, i = Base; i < Base + Count; i++, j++)
				states[i] = __REMOOD_REFSTATE(LocalStuff->WeaponStates[j]);
		}
	}
	
	/* Normalize */
	for (i = 0; i < NUMWEAPONS; i++)
	{
		// Reference
		TempWeapon = wpnlev1info[i];
		
		// Normalize classes
		TempWeapon->ammo = INFO_GetAmmoByName(TempWeapon->AmmoClass);
		
		// Reference states to IDs
		for (j = 0; j < NUMPWEAPONSTATEGROUPS; j++)
		{
			// Get reference
			switch (j)
			{
				case PWSG_UP:
					StateRef = &TempWeapon->upstate;
					break;
				case PWSG_DOWN:
					StateRef = &TempWeapon->downstate;
					break;
				case PWSG_READY:
					StateRef = &TempWeapon->readystate;
					break;
				case PWSG_ATTACK:
					StateRef = &TempWeapon->atkstate;
					break;
				case PWSG_HOLDATTACK:
					StateRef = &TempWeapon->holdatkstate;
					break;
				case PWSG_FLASH:
					StateRef = &TempWeapon->flashstate;
					break;
				default:
					StateRef = NULL;
					break;
			}
			
			// Missing?
			if (!StateRef)
				continue;
			
			// Determine reference to find
			RefToFind = ((j & 0xFFFF) << 16) | 1;
			
			// Find states in merge bases
			for (k = MergeBase; k < MergeBase + MergeCount; k++)
				if (states[k]->ObjectID == TempWeapon->WeaponID)
					if (states[k]->Marker == RefToFind)
					{
						*StateRef = k;
						break;
					}
			
			// Find flash states
			if (j == PWSG_FLASH)
			{
				// Clear flash states for this weapon
				if (TempWeapon->FlashStates)
					Z_Free(TempWeapon->FlashStates);
				TempWeapon->FlashStates = NULL;
				TempWeapon->NumFlashStates = 0;
				
				// Get reference
				RefToFind = ((j & 0xFFFF) << 16);
				
				// Determine flash order
				for (k = MergeBase; k < MergeBase + MergeCount; k++)
					if (states[k]->ObjectID == TempWeapon->WeaponID)
						if ((states[k]->Marker & 0xFFFF0000U) == RefToFind)
						{
							// No func?
							if (!states[k]->Function)
								continue;
							
							// Function only on A_Light*()
							if (strcasecmp(states[k]->Function, "Light1") != 0 && strcasecmp(states[k]->Function, "Light2") != 0)
								continue;
								
							// Reference it
							WepID = (states[k]->Marker & 0xFFFFU);
							
							// Add to array
							Z_ResizeArray((void**)&TempWeapon->FlashStates, sizeof(*TempWeapon->FlashStates), TempWeapon->NumFlashStates, TempWeapon->NumFlashStates + 1);
							TempWeapon->FlashStates[TempWeapon->NumFlashStates++] = k;
						}
			}
		}
	}
	
	/* Normalize state references */
	INFO_StateNormalize(MergeBase, MergeCount);
	
	return true;
}

