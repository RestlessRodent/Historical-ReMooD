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
// ----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1999 Lee Killough.
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
//      Weapon sprite animation, weapon objects.
//      Action functions for weapons.

#include "doomtype.h"
#include "info.h"
#include "p_mobj.h"
#include "d_player.h"
#include "p_local.h"
#include "p_maputl.h"
#include "s_sound.h"
#include "p_demcmp.h"
#include "tables.h"
#include "g_state.h"
#include "console.h"
#include "m_random.h"
#include "p_inter.h"
#include "r_main.h"














#define LOWERSPEED              FRACUNIT*6
#define RAISESPEED              FRACUNIT*6

#define WEAPONBOTTOM            128*FRACUNIT
#define WEAPONTOP               32*FRACUNIT

#define FLAME_THROWER_TICS      (10*TICRATE)
#define MAGIC_JUNK              1234
#define MAX_MACE_SPOTS          8


PI_mobjid_t PuffType;

//
// P_SetPsprite
//
void P_SetPsprite(player_t* player, int position, PI_stateid_t stnum)
{
	pspdef_t* psp;
	PI_state_t* state;
	
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
		if (P_XGSVal(PGS_PLFASTERWEAPONS))
			if (psp->tics > 1)
				psp->tics = 1;
		
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
// P_BringUpWeapon
// Starts bringing the pending weapon up
// from the bottom of the screen.
// Uses player
//
void P_BringUpWeapon(player_t* player)
{
	PI_stateid_t newstate;
	
	/* Check */
	if (!player)
		return;
	
	/* No Info? */
	if (!player->weaponinfo)
		return;
		
	if (player->pendingweapon == wp_nochange)
		player->pendingweapon = player->readyweapon;
		
	// GhostlyDeath <November 3, 2010> -- PARANOIA removal
	if (player->pendingweapon >= NUMWEAPONS)
	{
		CONL_PrintF("WARNING - P_BringUpWeapon: %i (player->pendingweapon) >= %i (%s:%i).\n", player->pendingweapon, NUMWEAPONS, __FILE__, __LINE__);
		return;
	}
	
	if (player->weaponinfo[player->pendingweapon]->BringUpSound)
		S_StartSound(&player->mo->NoiseThinker, S_SoundIDForName(player->weaponinfo[player->pendingweapon]->BringUpSound));
	
	newstate = player->weaponinfo[player->pendingweapon]->upstate;
	
	player->pendingweapon = wp_nochange;
	player->psprites[ps_weapon].sy = WEAPONBOTTOM;
	
	P_SetPsprite(player, ps_weapon, newstate);
}

/* P_CheckAmmo() -- Returns true if there is enough ammo to shoot. If not, selects the next weapon to use. */
bool_t P_CheckAmmo(player_t* player)
{
	PI_ammoid_t ammo;
	int count;
	size_t i;
	PI_wepid_t BestWeapon;
	
	/* get ammo type */
	ammo = player->weaponinfo[player->readyweapon]->ammo;
	
	/* Early infinite ammo out */
	if (P_XGSVal(PGS_PLINFINITEAMMO))
		return true;
		
	// Minimal amount for one shot varies.
	count = player->weaponinfo[player->readyweapon]->ammopershoot;
	
	// GhostlyDeath <June 12, 2012> -- All Ammo
	if (ammo == am_all)
	{
		// Check all ammo types
		for (i = 0; i < NUMAMMO; i++)
			if (!(ammoinfo[i]->Flags & AF_INFINITE))
				if (player->ammo[i] < count)
					break;
		
		// Enough ammo?
		if (i >= NUMAMMO)
			return true;
	}
	
	// Some do not need ammunition anyway.
	// Return if current ammunition sufficient.
	else if (ammo == am_noammo || player->ammo[ammo] >= count ||
		(ammoinfo[ammo]->Flags & AF_INFINITE))
		return true;
		
	// Out of ammo, pick a weapon to change to.
	// Preferences are set here.
	// added by Boris : preferred weapons order
	if (!player->originalweaponswitch)
		P_PlayerSwitchToFavorite(player, false);
	
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
	PI_stateid_t newstate;
	
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
	if (player->weaponinfo)
		P_SetPsprite(player, ps_weapon, player->weaponinfo[player->readyweapon]->downstate);
}

//
// A_WeaponReady
// The player can fire the weapon
// or change to another weapon at this time.
// Follows after getting weapon up,
// or after previous attack/fire sequence.
//
void A_WeaponReady(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	int32_t angle;
	
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
	if (player->cmd.Std.buttons & BT_ATTACK)
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
	
	// bob the weapon based on movement speed
	angle = (128 * leveltime) & FINEMASK;
	psp->sx = FRACUNIT + FixedMul(player->FlatBob, finecosine[angle]);
	angle &= FINEANGLES / 2 - 1;
	psp->sy = WEAPONTOP + FixedMul(player->FlatBob, finesine[angle]);
}

/* P_ReduceAmmo() -- Reduces player ammo */
void P_ReduceAmmo(player_t* player)
{
	bool_t AmTypeInfinite;
	PI_ammoid_t Ammo;
	int32_t i;
	
	/* Check if the ammo type is infinite */
	AmTypeInfinite = true;	// Always infinite (prevent bounds)
	Ammo = player->weaponinfo[player->readyweapon]->ammo;
	if (Ammo != am_noammo && Ammo >= 0 && Ammo < NUMAMMO)
		if (!(ammoinfo[Ammo]->Flags & AF_INFINITE))
			AmTypeInfinite = false;
	
	/* If not infinite ammo, and the ammo type is not infinite */
	if (!P_XGSVal(PGS_PLINFINITEAMMO) && !AmTypeInfinite)
		// Uses all ammo types
		if (Ammo == am_all)
			for (i = 0; i < NUMAMMO; i++)
			{
				if (!(ammoinfo[i]->Flags & AF_INFINITE))
					player->ammo[i] -= player->weaponinfo[player->readyweapon]->ammopershoot;
			}
		
		// Only a single ammo type
		else
			player->ammo[player->weaponinfo[player->readyweapon]->ammo] -= player->weaponinfo[player->readyweapon]->ammopershoot;
}

//
// A_ReFire
// The player can re-fire the weapon
// without lowering it entirely.
//
void A_ReFire(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{

	// check for fire
	//  (if a weaponchange is pending, let it go through instead)
	if ((player->cmd.Std.buttons & BT_ATTACK) && player->pendingweapon == wp_nochange && player->health)
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

void A_CheckReload(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
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
void A_Lower(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
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
void A_Raise(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
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
void A_GunFlash(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	P_SetMobjState(player->mo, player->mo->info->RPlayerRangedAttackState);
	P_SetPsprite(player, ps_flash, player->weaponinfo[player->readyweapon]->flashstate);
}

//
// WEAPON ATTACKS
//

/* PS_GetPuffType() -- Gets the replacement puff */
PI_mobjid_t PS_GetPuffType(player_t* player)
{
	PI_wep_t* Weapon;
	
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
void A_Punch(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
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
	
	// GhostlyDeath <September 21, 2012> -- Reduce ammo (possibly)
	P_ReduceAmmo(player);
	
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
		
		// GhostlyDeath <October 21, 2012> -- Turn to face, locally
		P_UpdateViewAngles(player->mo);
	}
}

//
// A_Saw
//
void A_Saw(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	angle_t angle;
	angle_t* locang = NULL;
	int damage;
	int slope;
	int i;
	
	// GhostlyDeath <September 21, 2012> -- Reduce ammo (possibly)
	P_ReduceAmmo(player);
	
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
	if (!P_XGSVal(PGS_CONOSAWFACING))
	{
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
		
		// GhostlyDeath <October 21, 2012> -- Turn to face, locally
		P_UpdateViewAngles(player->mo);
	}
	
	player->mo->flags |= MF_JUSTATTACKED;
}

//
// A_FireMissile : rocket launcher fires a rocket
//
void A_FireMissile(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	P_ReduceAmmo(player);
	
	//added:16-02-98: added player arg3
	P_SpawnPlayerMissile(player->mo, INFO_GetTypeByName("RocketShot"));
}

//
// A_FireBFG
//
void A_FireBFG(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	P_ReduceAmmo(player);
	//added:16-02-98:added player arg3
	player->LastBFGBall = P_SpawnPlayerMissile(player->mo, INFO_GetTypeByName("BFGShot"));
}

//
// A_FirePlasma
//
void A_FirePlasma(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
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
	if (!P_XGSVal(PGS_COFORCEAUTOAIM))
		if (!mo->player->autoaim_toggle || !P_XGSVal(PGS_PLALLOWAUTOAIM) || !P_XGSVal(PGS_COMOUSEAIM))
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
		if (P_XGSVal(PGS_COMOUSEAIM))
			if (!linetarget)
			{
notagetfound:
				if (P_XGSVal(PGS_COENABLEUPDOWNSHOOT))
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
		if (P_XGSVal(PGS_CONEWGUNSHOTCODE))
		{
			angle += (P_Random() << 18);	// WARNING: don't put this in one line
			angle -= (P_Random() << 18);	// else this expretion is ambiguous (evaluation order not diffined)
		}
		
		// Can really just use signed random here.
		else
			angle += P_SignedRandom() << 18;
	}
	
	//Args.Flags |= PLAF_THRUMOBJ;
	P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage, &Args);
}

//
// A_FirePistol
//
void A_FirePistol(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
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
void A_FireShotgun(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
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
void A_FireShotgun2(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	int i;
	angle_t angle;
	int damage, slope;
	bool_t NewSpread;
	
	PuffType = PS_GetPuffType(player);
	S_StartSound(&player->mo->NoiseThinker, sfx_dshtgn);
	P_SetMobjState(player->mo, player->mo->info->RPlayerRangedAttackState);
	
	P_ReduceAmmo(player);
		
	P_SetPsprite(player, ps_flash, player->weaponinfo[player->readyweapon]->flashstate);
	
	P_BulletSlope(player->mo);
	
	// GhostlyDeath <June 17, 2012> -- Demo Comp (1.32 moved PR around)
	NewSpread = false;
	if (P_XGSVal(PGS_CONEWSSGSPREAD))
		NewSpread = true;
	
	for (i = 0; i < 20; i++)
	{
		if (NewSpread)
			slope = bulletslope + (P_SignedRandom() << 5);
	
		damage = 5 * (P_Random() % 3 + 1);
		angle = player->mo->angle + (P_SignedRandom() << 19);
		
		if (!NewSpread)
			slope = bulletslope + (P_SignedRandom() << 5);
		
		P_LineAttack(player->mo, angle, MISSILERANGE, slope, damage, NULL);
	}
}

//
// A_FireCGun
//
void A_FireCGun(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	int StateDiff;
	
	S_StartSound(&player->mo->NoiseThinker, sfx_pistol);
	
	if (!P_XGSVal(PGS_PLINFINITEAMMO))
		if (!player->ammo[player->weaponinfo[player->readyweapon]->ammo])
			return;
			
	PuffType = PS_GetPuffType(player);
	P_SetMobjState(player->mo, player->mo->info->RPlayerRangedAttackState);
	
	P_ReduceAmmo(player);
	
	// GhostlyDeath <April 29, 2012> -- Set corresponding flash state
	StateDiff = player->psprites[ps_weapon].state->FrameID/* - 1*/;
	if (StateDiff < player->weaponinfo[player->readyweapon]->NumFlashStates)
		P_SetPsprite(player, ps_flash, player->weaponinfo[player->readyweapon]->FlashStates[StateDiff]);
	
	P_BulletSlope(player->mo);
	P_GunShot(player->mo, !player->refire);
}

//
// Flash light when fire gun
//
void A_Light0(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	player->extralight = 0;
}

void A_Light1(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	player->extralight = 1;
}

void A_Light2(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	player->extralight = 2;
}

//
// A_BFGSpray
// Spawn a BFG explosion on every monster in view
//
void A_BFGSpray(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	int i;
	int j;
	int damage;
	angle_t an;
	mobj_t* extrabfg;
	mobj_t* BallOwner;
	PI_wepid_t OldWeapon;
	
	// Remember stuff about object
	BallOwner = mo->target;
	
	if (BallOwner)
		OldWeapon = BallOwner->RXShotWithWeapon;
	else
	{
		BallOwner = mo;
		OldWeapon = NUMWEAPONS;
	}
	
	// Remove fired BFG ball
	if (BallOwner->player)
		BallOwner->player->LastBFGBall = NULL;
	
	// Set owner weapon to BFG, etc.
	BallOwner->RXShotWithWeapon = mo->RXShotWithWeapon;
	
	// Set owner's attack to ranged
	BallOwner->RXAttackAttackType = PRXAT_RANGED;
	
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
		if (P_XGSVal(PGS_COOLDBFGSPRAY))
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
void A_BFGsound(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
{
	S_StartSound(&mo->NoiseThinker, sfx_bfg);
}

/* A_FireGenericProjectile() -- Fires a generic projectile */
void A_FireGenericProjectile(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
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
void A_FireOldBFG(mobj_t* mo, player_t* player, pspdef_t* psp, const PI_sargc_t a_ArgC, PI_sargv_t* const a_ArgV)
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
	PI_state_t* state;
	
	// GhostlyDeath <May 8, 2012> -- If playing as monster, do not move sprites
	if (player->mo && !(P_MobjIsPlayer(player->mo)))
	{
		// Only handle attack button
		if (player->cmd.Std.buttons & BT_ATTACK)
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

