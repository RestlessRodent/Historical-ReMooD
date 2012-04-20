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
//      Handling interactions (i.e., collisions).

#include "doomdef.h"
#include "i_system.h"			//I_Tactile currently has no effect
#include "am_map.h"
#include "dstrings.h"
#include "g_game.h"
#include "m_random.h"
#include "p_local.h"
#include "p_inter.h"
#include "s_sound.h"
#include "r_main.h"
#include "st_stuff.h"
#include "g_input.h"
#include "p_demcmp.h"
#include "z_zone.h"
#include "p_pspr.h"
#include "d_items.h"

#define BONUSADD        6

// a weapon is found with two clip loads,
// a big item has five clip loads
consvar_t cv_fragsweaponfalling = { "fragsweaponfalling", "0", CV_NETVAR, CV_YesNo };
consvar_t cv_infiniteammo = { "infiniteammo", "0", CV_NETVAR, CV_YesNo };

// added 4-2-98 (Boris) for dehacked patch
// (i don't like that but do you see another solution ?)
int MAXHEALTH = 100;

//
// GET STUFF
//

// added by Boris : preferred weapons order
void VerifFavoritWeapon(player_t* player)
{
	int newweapon;
	
	if (player->pendingweapon != wp_nochange)
		return;
		
	newweapon = FindBestWeapon(player);
	
	if (newweapon != player->readyweapon)
		player->pendingweapon = newweapon;
}

int FindBestWeapon(player_t* player)
{
	return wp_nochange;
}

//
// P_GiveAmmo
// Num is the number of clip loads,
// not the individual count (0= 1/2 clip).
// Returns false if the ammo can't be picked up at all
//

bool_t P_GiveAmmo(player_t* player, ammotype_t ammo, int count)
{
	int oldammo;
	size_t i;
	weapontype_t ChoseWeapon;
	
	if (ammo == am_noammo)
		return false;
		
	if (ammo < 0 || ammo > NUMAMMO)
	{
		CONL_PrintF("\2P_GiveAmmo: bad type %i", ammo);
		return false;
	}
	
	if (player->ammo[ammo] == player->maxammo[ammo])
		return false;
		
	/*
	   if (num)
	   num *= clipammo[ammo];
	   else
	   num = clipammo[ammo]/2;
	 */
	if (gameskill == sk_baby || gameskill == sk_nightmare)
	{
		// give double ammo in trainer mode,
		// you'll need in nightmare
		count <<= 1;
	}
	
	oldammo = player->ammo[ammo];
	player->ammo[ammo] += count;
	
	if (player->ammo[ammo] > player->maxammo[ammo])
		player->ammo[ammo] = player->maxammo[ammo];
		
	// If non zero ammo,
	// don't change up weapons,
	// player was lower on purpose.
	if (oldammo)
		return true;
		
	// We were down to zero,
	// so select a new weapon.
	// Preferences are not user selectable.
	
	// Boris hack for preferred weapons order...
	if (!player->originalweaponswitch)
	{
		if (player->ammo[player->weaponinfo[player->readyweapon]->ammo] < player->weaponinfo[player->readyweapon]->ammopershoot)
			VerifFavoritWeapon(player);
		return true;
	}
	else
	{
		// Clear
		ChoseWeapon = NUMWEAPONS;
		
		// Weapon is undesireable, so switch away from it
		if (player->weaponinfo[player->readyweapon]->WeaponFlags & WF_SWITCHFROMNOAMMO)
			// Only switch if our ammo isn't the same
			if (ammo != player->weaponinfo[player->readyweapon]->ammo)
				// Switch to the best gun for this ammo type, that the player has
				for (i = 0; i < NUMWEAPONS; i++)
					// Can use weapon?
					if (P_CanUseWeapon(player, i))
						// Only check for the same ammo
						if (ammo == player->weaponinfo[i]->ammo)
							// Only if the player has this gun
							if (player->weaponowned[i])
								// Got this gun, or it is better than this.
								if (ChoseWeapon == NUMWEAPONS ||
									(player->weaponinfo[i]->SwitchOrder > player->weaponinfo[ChoseWeapon]->SwitchOrder))
									ChoseWeapon = i;
		
		// Switch to gun?
		if (ChoseWeapon != NUMWEAPONS)
			if (ChoseWeapon != player->readyweapon)
				player->pendingweapon = ChoseWeapon;
	}
		
	return true;
}

static int has_ammo_dropped = 0;

//
// P_GiveWeapon
// The weapon name may have a MF_DROPPED flag ored in.
//
bool_t P_GiveWeapon(player_t* player, weapontype_t weapon, bool_t dropped)
{
	bool_t gaveammo;
	bool_t gaveweapon;
	int ammo_count;
	int i;
	
	if (multiplayer && (cv_deathmatch.value != 2) && !dropped)
	{
		// leave placed weapons forever on net games
		if (player->weaponowned[weapon])
			return false;
			
		player->bonuscount += BONUSADD;
		player->weaponowned[weapon] = true;
		
		if (cv_deathmatch.value)
			P_GiveAmmo(player, player->weaponinfo[weapon]->ammo, 5 * ammoinfo[player->weaponinfo[weapon]->ammo]->ClipAmmo);
		else
			P_GiveAmmo(player, player->weaponinfo[weapon]->ammo, player->weaponinfo[weapon]->GetAmmo);
			
		// Boris hack preferred weapons order...
		// TODO FIXME: Reimplement player weapon order
//		if (player->originalweaponswitch || player->favoritweapon[weapon] > player->favoritweapon[player->readyweapon])
//			player->pendingweapon = weapon;	// do like Doom2 original
			
		//added:16-01-98:changed consoleplayer to displayplayer
		//               (hear the sounds from the viewpoint)
		for (i = 0; i < g_SplitScreen + 1; i++)
			if (player == &players[displayplayer[i]])
				S_StartSound(NULL, sfx_wpnup);
		return false;
	}
	
	if (player->weaponinfo[weapon]->ammo != am_noammo)
	{
		// give one clip with a dropped weapon,
		// two clips with a found weapon
		if (dropped)
		{
			ammo_count = has_ammo_dropped ? (has_ammo_dropped < 0 ? 0 : has_ammo_dropped) : ammoinfo[player->weaponinfo[weapon]->ammo]->ClipAmmo;
			//gaveammo = P_GiveAmmo (player, player->weaponinfo[weapon].ammo, clipammo[player->weaponinfo[weapon].ammo]);
		}
		else
		{
			//gaveammo = P_GiveAmmo (player, player->weaponinfo[weapon].ammo, player->weaponinfo[weapon].GetAmmo);
			ammo_count = player->weaponinfo[weapon]->GetAmmo;
		}
		gaveammo = P_GiveAmmo(player, player->weaponinfo[weapon]->ammo, ammo_count);
	}
	else
		gaveammo = false;
		
	if (player->weaponowned[weapon])
		gaveweapon = false;
	else
	{
		gaveweapon = true;
		player->weaponowned[weapon] = true;
		// TODO FIXME: Reimplement player weapon order
//		if (player->originalweaponswitch || player->favoritweapon[weapon] > player->favoritweapon[player->readyweapon])
//			player->pendingweapon = weapon;	// Doom2 original stuff
	}
	
	return (gaveweapon || gaveammo);
}

//
// P_GiveBody
// Returns false if the body isn't needed at all
//
bool_t P_GiveBody(player_t* player, int num)
{
	int max;
	
	max = MAXHEALTH;
	
	if (player->chickenTics)
		max = MAXCHICKENHEALTH;
		
	if (player->health >= max)
		return false;
		
	player->health += num;
	if (player->health > max)
		player->health = max;
	player->mo->health = player->health;
	
	return true;
}

//
// P_GiveArmor
// Returns false if the armor is worse
// than the current armor.
//
bool_t P_GiveArmor(player_t* player, int armortype)
{
	int hits;
	
	hits = armortype * 100;
	if (player->armorpoints >= hits)
		return false;			// don't pick up
		
	player->armortype = armortype;
	player->armorpoints = hits;
	
	return true;
}

//
// P_GiveCard
//
static bool_t P_GiveCard(player_t* player, card_t card)
{
	if (player->cards & card)
		return false;
		
	player->bonuscount = BONUSADD;
	player->cards |= card;
	return true;
}

//
// P_GivePower
//
bool_t P_GivePower(player_t* player, int /*powertype_t */ power)
{
	if (power == pw_invulnerability)
	{
		// Already have it
		if (inventory && player->powers[power] > BLINKTHRESHOLD)
			return false;
			
		player->powers[power] = INVULNTICS;
		return true;
	}
	else if (power == pw_invisibility)
	{
		// Already have it
		if (inventory && player->powers[power] > BLINKTHRESHOLD)
			return false;
			
		player->powers[power] = INVISTICS;
		player->mo->flags |= MF_SHADOW;
		return true;
	}
	else if (power == pw_infrared)
	{
		// Already have it
		if (player->powers[power] > BLINKTHRESHOLD)
			return (false);
			
		player->powers[power] = INFRATICS;
		return true;
	}
	else if (power == pw_ironfeet)
	{
		player->powers[power] = IRONTICS;
		return true;
	}
	else if (power == pw_strength)
	{
		P_GiveBody(player, 100);
		player->powers[power] = 1;
		return true;
	}
	
	if (player->powers[power])
		return false;			// already got it
		
	player->powers[power] = 1;
	return true;
}

// Boris stuff : dehacked patches hack
int max_armor = 200;
int green_armor_class = 1;
int blue_armor_class = 2;
int maxsoul = 200;
int soul_health = 100;
int mega_health = 200;

// eof Boris

/* PS_PickupMessage() -- Handles pickup messages */
void PS_PickupMessage(mobj_t* const a_Picker, mobj_t* const a_Upper, const char* const a_Message)
{
#define BUFSIZE 128
	int LocalPlayer, i;
	char Buf[BUFSIZE];
	
	/* Check */
	if (!a_Picker || !a_Upper)
		return;
		
	/* If the object picking it up is not a player... */
	if (!a_Picker->player)
		return;
		
	/* Find player that is picking this up */
	for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
		if (playeringame[displayplayer[i]] && &players[displayplayer[i]] == a_Picker->player)
			break;
			
	// Nobody on these screens
	if (i == MAXSPLITSCREENPLAYERS)
		return;
		
	/* Find message to print */
	memset(Buf, 0, sizeof(Buf));
	if (a_Message)
		strncpy(Buf, a_Message, BUFSIZE);
		
	/* Print to console (to that player only) */
	if (i >= 1)
		CONL_PrintF("%c", 4 + (i - 1));
	CONL_PrintF("%s\n", Buf);
#undef BUFSIZE
}

//
// P_TouchSpecialThing
//
void P_TouchSpecialThing(mobj_t* special, mobj_t* toucher)
{
	player_t* player;
	int i;
	fixed_t delta;
	int sound;
	P_RMODTouchSpecial_t* Current;
	bool_t OKStat;
	
	delta = special->z - toucher->z;
	
	//SoM: 3/27/2000: For some reason, the old code allowed the player to
	//grab items that were out of reach...
	if (delta > toucher->height || delta < -special->height)
	{
		// out of reach
		return;
	}
	// Dead thing touching.
	// Can happen with a sliding player corpse.
	if (toucher->health <= 0 || toucher->flags & MF_CORPSE)
		return;
		
	sound = sfx_itemup;
	player = toucher->player;
		
	// FWF support
	has_ammo_dropped = special->dropped_ammo_count;
	
	/* Find sprite by ID and match to touch special list */
	OKStat = false;
	for (i = 0; i < g_RMODNumTouchSpecials; i++)
	{
		// Get current
		Current = g_RMODTouchSpecials[i];
		
		// Wrong sprite?
		if (Current->ActSpriteID != special->state->SpriteID)
			continue;
		
		// Player is grabbing
		if (player)
		{
			// Give gun?
			if (Current->ActGiveWeapon != NUMWEAPONS)
				OKStat |= P_GiveWeapon(player, Current->ActGiveWeapon, special->flags & MF_DROPPED);
			
			// Counts as an item?
			if (special->flags & MF_COUNTITEM)
			{
				player->itemcount++;
				player->bonuscount += BONUSADD;
			}
			
			//added:16-01-98:consoleplayer -> displayplayer (hear sounds from viewpoint)
			for (i = 0; i < g_SplitScreen + 1; i++)
				if (player == &players[displayplayer[i]])
					S_StartSound(NULL, sound);
		}
		
		// Monster is grabbing
		else
		{
			// Monster cannot pickup thing
			if (!Current->MonsterCanGrab)
				return;
			
			// Emit sound from monster
			S_StartSound(toucher, sound);
		}
		
		
		// Remove if we used it? or remove regardless
		if ((Current->KeepNotNeeded && OKStat) || !Current->KeepNotNeeded || Current->RemoveAlways)
			P_RemoveMobj(special);
		
		// Don't process anymore
		break;
	}
	
#if 0
	// Identify by sprite.
	switch (special->sprite)
	{
			// armor
		case SPR_ARM1:
			if (!P_GiveArmor(player, green_armor_class))
				return;
			PS_PickupMessage(toucher, special, GOTARMOR);
			break;
			
		case SPR_ARM2:
			if (!P_GiveArmor(player, blue_armor_class))
				return;
			PS_PickupMessage(toucher, special, GOTMEGA);
			break;
			
			// bonus items
		case SPR_BON1:
			player->health++;	// can go over 100%
			if (player->health > 2 * MAXHEALTH)
				player->health = 2 * MAXHEALTH;
			player->mo->health = player->health;
			if (cv_showmessages.value == 1)
				PS_PickupMessage(toucher, special, GOTHTHBONUS);
			break;
			
		case SPR_BON2:
			player->armorpoints++;	// can go over 100%
			if (player->armorpoints > max_armor)
				player->armorpoints = max_armor;
			if (!player->armortype)
				player->armortype = 1;
			PS_PickupMessage(toucher, special, GOTARMBONUS);
			break;
			
		case SPR_SOUL:
			player->health += soul_health;
			if (player->health > maxsoul)
				player->health = maxsoul;
			player->mo->health = player->health;
			PS_PickupMessage(toucher, special, GOTSUPER);
			sound = sfx_getpow;
			break;
			
		case SPR_MEGA:
			if (gamemode != commercial)
				return;
			player->health = mega_health;
			player->mo->health = player->health;
			P_GiveArmor(player, 2);
			PS_PickupMessage(toucher, special, GOTMSPHERE);
			sound = sfx_getpow;
			break;
			// cards
			// leave cards for everyone
		case SPR_BKEY:
			if (P_GiveCard(player, it_bluecard))
			{
				PS_PickupMessage(toucher, special, GOTBLUECARD);
			}
			if (!multiplayer)
				break;
			return;
			
		case SPR_YKEY:
			if (P_GiveCard(player, it_yellowcard))
			{
				PS_PickupMessage(toucher, special, GOTYELWCARD);
			}
			if (!multiplayer)
				break;
			return;
			
		case SPR_RKEY:
			if (P_GiveCard(player, it_redcard))
			{
				PS_PickupMessage(toucher, special, GOTREDCARD);
			}
			if (!multiplayer)
				break;
			return;
			
		case SPR_BSKU:
			if (P_GiveCard(player, it_blueskull))
			{
				PS_PickupMessage(toucher, special, GOTBLUESKUL);
			}
			if (!multiplayer)
				break;
			return;
			
		case SPR_YSKU:
			if (P_GiveCard(player, it_yellowskull))
			{
				PS_PickupMessage(toucher, special, GOTYELWSKUL);
			}
			if (!multiplayer)
				break;
			return;
			
		case SPR_RSKU:
			if (P_GiveCard(player, it_redskull))
			{
				PS_PickupMessage(toucher, special, GOTREDSKULL);
			}
			if (!multiplayer)
				break;
			return;
			
			// medikits, heals
		case SPR_STIM:
			if (!P_GiveBody(player, 10))
				return;
			PS_PickupMessage(toucher, special, GOTSTIM);	// TODO
			break;
			
		case SPR_MEDI:
			if (!P_GiveBody(player, 25))
				return;
			if (player->health < 25)
				PS_PickupMessage(toucher, special, GOTMEDINEED);
			else
				PS_PickupMessage(toucher, special, GOTMEDIKIT);
			break;
			
			// power ups
		case SPR_PINV:
			if (!P_GivePower(player, pw_invulnerability))
				return;
			PS_PickupMessage(toucher, special, GOTINVUL);
			sound = sfx_getpow;
			break;
			
		case SPR_PSTR:
			if (!P_GivePower(player, pw_strength))
				return;
			PS_PickupMessage(toucher, special, GOTBERSERK);
			if (player->readyweapon != wp_fist)
				player->pendingweapon = wp_fist;
			sound = sfx_getpow;
			break;
			
		case SPR_PINS:
			if (!P_GivePower(player, pw_invisibility))
				return;
			PS_PickupMessage(toucher, special, GOTINVIS);
			sound = sfx_getpow;
			break;
			
		case SPR_SUIT:
			if (!P_GivePower(player, pw_ironfeet))
				return;
			PS_PickupMessage(toucher, special, GOTSUIT);
			sound = sfx_getpow;
			break;
			
		case SPR_PMAP:
			if (!P_GivePower(player, pw_allmap))
				return;
			PS_PickupMessage(toucher, special, GOTMAP);
			sound = sfx_getpow;
			break;
			
		case SPR_PVIS:
			if (!P_GivePower(player, pw_infrared))
				return;
			PS_PickupMessage(toucher, special, GOTVISOR);
			sound = sfx_getpow;
			break;
			
			// ammo
		case SPR_CLIP:
			if (special->flags & MF_DROPPED)
			{
				if (!P_GiveAmmo(player, am_clip, ammoinfo[am_clip].ClipAmmo / 2))
					return;
			}
			else
			{
				if (!P_GiveAmmo(player, am_clip, ammoinfo[am_clip].ClipAmmo))
					return;
			}
			PS_PickupMessage(toucher, special, GOTCLIP);
			break;
			
		case SPR_AMMO:
			if (!P_GiveAmmo(player, am_clip, 5 * ammoinfo[am_clip].ClipAmmo))
				return;
			PS_PickupMessage(toucher, special, GOTCLIPBOX);
			break;
			
		case SPR_ROCK:
			if (!P_GiveAmmo(player, am_misl, ammoinfo[am_misl].ClipAmmo))
				return;
			PS_PickupMessage(toucher, special, GOTROCKET);
			break;
			
		case SPR_BROK:
			if (!P_GiveAmmo(player, am_misl, 5 * ammoinfo[am_misl].ClipAmmo))
				return;
			PS_PickupMessage(toucher, special, GOTROCKBOX);
			break;
			
		case SPR_CELL:
			if (!P_GiveAmmo(player, am_cell, ammoinfo[am_cell].ClipAmmo))
				return;
			PS_PickupMessage(toucher, special, GOTCELL);
			break;
			
		case SPR_CELP:
			if (!P_GiveAmmo(player, am_cell, 5 * ammoinfo[am_cell].ClipAmmo))
				return;
			PS_PickupMessage(toucher, special, GOTCELLBOX);
			break;
			
		case SPR_SHEL:
			if (!P_GiveAmmo(player, am_shell, ammoinfo[am_shell].ClipAmmo))
				return;
			PS_PickupMessage(toucher, special, GOTSHELLS);
			break;
			
		case SPR_SBOX:
			if (!P_GiveAmmo(player, am_shell, 5 * ammoinfo[am_shell].ClipAmmo))
				return;
			PS_PickupMessage(toucher, special, GOTSHELLBOX);
			break;
			
		case SPR_BPAK:
			if (!player->backpack)
			{
				for (i = 0; i < NUMAMMO; i++)
					player->maxammo[i] *= 2;
				player->backpack = true;
			}
			for (i = 0; i < NUMAMMO; i++)
				P_GiveAmmo(player, i, ammoinfo[i].ClipAmmo);
			PS_PickupMessage(toucher, special, GOTBACKPACK);
			break;
			
			// weapons
		case SPR_BFUG:
			if (!P_GiveWeapon(player, wp_bfg, special->flags & MF_DROPPED))
				return;
			PS_PickupMessage(toucher, special, GOTBFG9000);
			sound = sfx_wpnup;
			break;
			
		case SPR_MGUN:
			if (!P_GiveWeapon(player, wp_chaingun, special->flags & MF_DROPPED))
				return;
			PS_PickupMessage(toucher, special, GOTCHAINGUN);
			sound = sfx_wpnup;
			break;
			
		case SPR_CSAW:
			if (!P_GiveWeapon(player, wp_chainsaw, false))
				return;
			PS_PickupMessage(toucher, special, GOTCHAINSAW);
			sound = sfx_wpnup;
			break;
			
		case SPR_LAUN:
			if (!P_GiveWeapon(player, wp_missile, special->flags & MF_DROPPED))
				return;
			PS_PickupMessage(toucher, special, GOTLAUNCHER);
			sound = sfx_wpnup;
			break;
			
		case SPR_PLAS:
			if (!P_GiveWeapon(player, wp_plasma, special->flags & MF_DROPPED))
				return;
			PS_PickupMessage(toucher, special, GOTPLASMA);
			sound = sfx_wpnup;
			break;
			
		case SPR_SHOT:
			if (!P_GiveWeapon(player, wp_shotgun, special->flags & MF_DROPPED))
				return;
			PS_PickupMessage(toucher, special, GOTSHOTGUN);
			sound = sfx_wpnup;
			break;
			
		case SPR_SGN2:
			if (!P_GiveWeapon(player, wp_supershotgun, special->flags & MF_DROPPED))
				return;
			PS_PickupMessage(toucher, special, GOTSHOTGUN2);
			sound = sfx_wpnup;
			break;
			
		default:
			// SoM: New gettable things with FraggleScript!
			//CONL_PrintF ("\2P_TouchSpecialThing: Unknown gettable thing\n");
			return;
	}
	
	if (special->flags & MF_COUNTITEM)
		player->itemcount++;
	P_RemoveMobj(special);
	player->bonuscount += BONUSADD;
	
	//added:16-01-98:consoleplayer -> displayplayer (hear sounds from viewpoint)
	for (i = 0; i < g_SplitScreen + 1; i++)
		if (player == &players[displayplayer[i]])
			S_StartSound(NULL, sound);
#endif
}

#ifdef thatsbuggycode
//
//  Tell each supported thing to check again its position,
//  because the 'base' thing has vanished or diminished,
//  the supported things might fall.
//
//added:28-02-98:
void P_CheckSupportThings(mobj_t* mobj)
{
	fixed_t supportz = mobj->z + mobj->height;
	
	while ((mobj = mobj->supportthings))
	{
		// only for things above support thing
		if (mobj->z > supportz)
			mobj->eflags |= MF_CHECKPOS;
	}
}

//
//  If a thing moves and supportthings,
//  move the supported things along.
//
//added:28-02-98:
void P_MoveSupportThings(mobj_t* mobj, fixed_t xmove, fixed_t ymove, fixed_t zmove)
{
	fixed_t supportz = mobj->z + mobj->height;
	mobj_t* mo = mobj->supportthings;
	
	while (mo)
	{
		//added:28-02-98:debug
		if (mo == mobj)
		{
			mobj->supportthings = NULL;
			break;
		}
		// only for things above support thing
		if (mobj->z > supportz)
		{
			mobj->eflags |= MF_CHECKPOS;
			mobj->momx += xmove;
			mobj->momy += ymove;
			mobj->momz += zmove;
		}
		
		mo = mo->supportthings;
	}
}

//
//  Link a thing to it's 'base' (supporting) thing.
//  When the supporting thing will move or change size,
//  the supported will then be aware.
//
//added:28-02-98:
void P_LinkFloorThing(mobj_t* mobj)
{
	mobj_t* mo;
	mobj_t* nmo;
	
	// no supporting thing
	if (!(mo = mobj->floorthing))
		return;
		
	// link mobj 'above' the lower mobjs, so that lower supporting
	// mobjs act upon this mobj
	while ((nmo = mo->supportthings) && (nmo->z <= mobj->z))
	{
		// dont link multiple times
		if (nmo == mobj)
			return;
			
		mo = nmo;
	}
	mo->supportthings = mobj;
	mobj->supportthings = nmo;
}

//
//  Unlink a thing from it's support,
//  when it's 'floorthing' has changed,
//  before linking with the new 'floorthing'.
//
//added:28-02-98:
void P_UnlinkFloorThing(mobj_t* mobj)
{
	mobj_t* mo;
	
	if (!(mo = mobj->floorthing))	// just to be sure (may happen)
		return;
		
	while (mo->supportthings)
	{
		if (mo->supportthings == mobj)
		{
			mo->supportthings = NULL;
			break;
		}
		mo = mo->supportthings;
	}
}
#endif

/* PS_GetMobjNoun() -- Gets the noun of the object */
static const char* PS_GetMobjNoun(mobj_t* const a_Mobj, bool_t* const a_Special, const bool_t a_IsInflictor, mobj_t* const a_Source)
{
	/* If there is no object */
	if (!a_Mobj)
	{
		// Nothing special here
		if (a_Special)
			*a_Special = false;
		return "Nothing";
	}
	
	/* If there is an object */
	else
	{
		// If the mobj is a player, use that player's name
		if (a_Mobj->player)
		{
			// Special
			if (a_Special)
				*a_Special = true;
				
			// Return the player's name
			if (!a_IsInflictor)
				return player_names[a_Mobj->player - players];
			
			// Return the name of the gun
			else
			{
				// Inflictor is the source (melee or gun attack?)
				if (a_Mobj == a_Source)
					return a_Mobj->player->weaponinfo[a_Mobj->player->readyweapon]->NiceName;
				
				// It must be a missile then, return the weapon there
				else
				{
					if (a_Mobj->RXShotWithWeapon >= 0 && a_Mobj->RXShotWithWeapon < NUMWEAPONS)
						return a_Mobj->player->weaponinfo[a_Mobj->RXShotWithWeapon]->NiceName;
					else
						return "Unknown Weapon";
				}
			}
		}
		// Otherwise it is a monster or otherwise
		else
		{
			// Not Special
			if (a_Special)
				*a_Special = true;
			
			// If the source has a weapon attached to it 
			if (a_Mobj->RXShotWithWeapon >= 0 && a_Mobj->RXShotWithWeapon < NUMWEAPONS)
			{
				// Check to see if there is a source player (use that name)
				if (a_Source->player)
					return a_Source->player->weaponinfo[a_Mobj->RXShotWithWeapon]->NiceName;
				
				// There is no player source, so use standard gun name
				else
					return wpnlev1info[a_Mobj->RXShotWithWeapon]->NiceName;
			}
			
			// If we never returned, then there was no weapon used
			if (!a_IsInflictor)
				// Return nice name of object
				return MT2ReMooDClass[a_Mobj->type];
			
			// Otherwise return the attack type
			else
			{
				if (a_Mobj->RXUsedMelee && !a_Mobj->RXUsedSpell)
					return "Melee Attack";
				else if (a_Mobj->RXUsedMelee && a_Mobj->RXUsedSpell)
					return "Melee Spell";
				else if (!a_Mobj->RXUsedMelee && !a_Mobj->RXUsedSpell)
					return "Ranged Attack";
				else
					return "Ranged Spell";
			}
		}
	}
}

/* P_DeathMessages() -- Display message of thing dying */
static void P_DeathMessages(mobj_t* target, mobj_t* inflictor, mobj_t* source)
{
#define BUFSIZE 128
	char Message[BUFSIZE];
	const char* tNoun, *iNoun, *sNoun;
	bool_t tSpecial, iSpecial, sSpecial;
	
	/* Determine nouns of objects */
	tNoun = PS_GetMobjNoun(target, &tSpecial, false, source);
	iNoun = PS_GetMobjNoun(inflictor, &iSpecial, true, source);
	sNoun = PS_GetMobjNoun(source, &sSpecial, false, source);
	
	/* If neither side is special, who cares? */
	/*if (!(tSpecial | sSpecial))
	   return; */
	
	/* Print message */
	CONL_PrintF("\x7{4%s{0 -> {5%s {2({3%s{2)\n", sNoun, tNoun, iNoun);
	
#undef BUFSIZE
}

// WARNING : check cv_fraglimit>0 before call this function !
void P_CheckFragLimit(player_t* p)
{
	if (cv_teamplay.value)
	{
		int fragteam = 0, i;
		
		for (i = 0; i < MAXPLAYERS; i++)
			if (ST_SameTeam(p, &players[i]))
				fragteam += ST_PlayerFrags(i);
				
		if (cv_fraglimit.value <= fragteam)
			G_ExitLevel();
	}
	else
	{
		if (cv_fraglimit.value <= ST_PlayerFrags(p - players))
			G_ExitLevel();
	}
}

/************************************************************
 *
 *  Returns ammo count in current weapon
 *
 ************************************************************
 */
static int P_AmmoInWeapon(player_t* player)
{
	ammotype_t ammo = player->weaponinfo[player->readyweapon]->ammo;
	int ammo_count = player->ammo[ammo];
	
	return ammo == am_noammo ? 0 : ammo_count ? ammo_count : -1;
}

// Rule for gibbing
CV_PossibleValue_t GibRules_cons_t[] =
{
	{0, "Doom"}
	,							// < 100% health required for gibbing
	{1, "Heretic"}
	,							// < 50% health required for gibbing
	
	{0, NULL}
};

consvar_t cv_g_gibrules = { "g_gibrules", "1", CV_SAVE, GibRules_cons_t };


// P_KillMobj
//
//      source is the attacker,
//      target is the 'target' of the attack, target dies...
//                                          113
void P_KillMobj(mobj_t* target, mobj_t* inflictor, mobj_t* source)
{
	mobjtype_t item = 0;
	mobj_t* mo;
	int drop_ammo_count = 0;
	int i;
	
	extern consvar_t cv_solidcorpse;
	
	// dead target is no more shootable
	if (!cv_solidcorpse.value)
		target->flags &= ~MF_SHOOTABLE;
		
	target->flags &= ~(MF_FLOAT | MF_SKULLFLY);
	
	// GhostlyDeath <March 6, 2012> -- Keep gravity on death?
	if (!(target->RXFlags[0] & MFREXA_KEEPGRAVONDEATH))
		target->flags &= ~MF_NOGRAVITY;
		
	// scream a corpse :)
	if (target->flags & MF_CORPSE)
	{
		// turn it to gibs
		P_SetMobjState(target, S_GIBS);
		
		target->flags &= ~MF_SOLID;
		target->height = 0;
		target->radius <<= 1;
		target->skin = 0;
		
		//added:22-02-98: lets have a neat 'crunch' sound!
		S_StartSound(&target->NoiseThinker, sfx_slop);
		return;
	}
	//added:22-02-98: remember who exploded the barrel, so that the guy who
	//                shot the barrel which killed another guy, gets the frag!
	//                (source is passed from barrel to barrel also!)
	//                (only for multiplayer fun, does not remember monsters)
	// GhostlyDeath <March 6, 2012> -- Use flag here
	if ((target->RXFlags[0] & MFREXA_CARRYKILLER) && source && source->player)
		target->target = source;
	
	// GhostlyDeath <April 8, 2012> -- If modifying corpses in A_Fall, then don't modify here
	if (!P_EXGSGetValue(PEXGSBID_COMODIFYCORPSE))
	{
		// in version 131 and higer this is done later in a_fall
		// (this fix the stepping monster)
		target->flags |= MF_CORPSE | MF_DROPOFF;
		target->height >>= 2;
		
		if (P_EXGSGetValue(PEXGSBID_COOLDCUTCORPSERADIUS))
			target->radius -= (target->radius >> 4);	//for solid corpses
	}
	// GhostlyDeath <September 17, 2011> -- Change the way obituaries are done
	// Legacy only did it if the target/source was the first console player
	P_DeathMessages(target, inflictor, source);
	
	// if killed by a player
	if (source && source->player)
	{
		// count for intermission
		if (target->flags & MF_COUNTKILL)
			source->player->killcount++;
			
		// count frags if player killed player
		if (target->player)
		{
			source->player->frags[target->player - players]++;
			
			// check fraglimit cvar
			if (cv_fraglimit.value)
				P_CheckFragLimit(source->player);
		}
	}
	else if (!multiplayer && (target->flags & MF_COUNTKILL))
	{
		// count all monster deaths,
		// even those caused by other monsters
		players[0].killcount++;
	}
	// if a player avatar dies...
	if (target->player)
	{
		// count environment kills against you (you fragged yourself!)
		if (!source)
			target->player->frags[target->player - players]++;
			
		target->flags &= ~MF_SOLID;	// does not block
		target->flags2 &= ~MF2_FLY;
		target->player->playerstate = PST_DEAD;
		P_DropWeapon(target->player);	// put weapon away
		
		for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
			if (playeringame[consoleplayer[i]] && target->player == &players[consoleplayer[i]])
				localaiming[i] = 0;
	}
	
	if (((target->health < -(target->info->spawnhealth >> cv_g_gibrules.value))) && target->info->xdeathstate)
		P_SetMobjState(target, target->info->xdeathstate);
	else
		P_SetMobjState(target, target->info->deathstate);
		
	target->tics -= P_Random() & 3;
	
	if (target->tics < 1)
		target->tics = 1;
		
	// Drop stuff.
	// This determines the kind of object spawned
	// during the death frame of a thing.

	// GhostlyDeath <March 6, 2012> -- Use invalid object	
	item = NUMMOBJTYPES;
	
	// Frags Weapon Falling support
	if (target->player && cv_fragsweaponfalling.value)
	{
		drop_ammo_count = P_AmmoInWeapon(target->player);
		//if (!drop_ammo_count)
		//    return;
		
		// Which item to drop?
		if (target->player->weaponinfo[target->player->readyweapon]->DropWeaponClass)
			item = INFO_GetTypeByName(target->player->weaponinfo[target->player->readyweapon]->DropWeaponClass);
	}
	else
	{
		// Which item to drop?
		if (target->info->RDropClass)
			item = INFO_GetTypeByName(target->info->RDropClass);
	}
	
	if (item && item >= 0 && item < NUMMOBJTYPES)
	{
		// SoM: Damnit! Why not use the target's floorz?
		mo = P_SpawnMobj(target->x, target->y, (!P_EXGSGetValue(PEXGSBID_COSPAWNDROPSONMOFLOORZ) ? ONFLOORZ : target->floorz), item);
		mo->flags |= MF_DROPPED;	// special versions of items
	
		if (!cv_fragsweaponfalling.value)
			drop_ammo_count = 0;	// Doom default ammo count
		
		mo->dropped_ammo_count = drop_ammo_count;
	}
}

//
// P_DamageMobj
// Damages both enemies and players
// "inflictor" is the thing that caused the damage
//  creature or missile, can be NULL (slime, etc)
// "source" is the thing to target after taking damage
//  creature or NULL
// Source and inflictor are the same for melee attacks.
// Source can be NULL for slime, barrel explosions
// and other environmental stuff.
//
bool_t P_DamageMobj(mobj_t* target, mobj_t* inflictor, mobj_t* source, int damage)
{
	unsigned ang;
	int saved;
	player_t* player;
	fixed_t thrust;
	bool_t takedamage;			// false on some case in teamplay
	
	if (!(target->flags & MF_SHOOTABLE))
		return false;			// shouldn't happen...
		
	if (target->health <= 0)
		return false;
		
	if (target->flags & MF_SKULLFLY)
	{
		// Minotaur is invulnerable during charge attack
		
		target->momx = target->momy = target->momz = 0;
	}
	
	player = target->player;
	if (player && gameskill == sk_baby)
		damage >>= 1;			// take half damage in trainer mode
		
	// Special damage types
	if (inflictor)
	{
		switch (inflictor->type)
		{
			default:
				break;
		}
	}
	// Some close combat weapons should not
	// inflict thrust and push the victim out of reach,
	// thus kick away unless using the chainsaw.
	if (inflictor
	        && !(target->flags & MF_NOCLIP) && !(inflictor->flags2 & MF2_NODMGTHRUST) && (!source || !source->player || !(source->player->weaponinfo[source->player->readyweapon]->WeaponFlags & WF_NOTHRUST)))
	{
		fixed_t amomx, amomy, amomz = 0;	//SoM: 3/28/2000
		
		ang = R_PointToAngle2(inflictor->x, inflictor->y, target->x, target->y);
		
		thrust = damage * (FRACUNIT >> 3) * 100 / target->info->mass;
		
		// sometimes a target shot down might fall off a ledge forwards
		if (damage < 40 && damage > target->health && target->z - inflictor->z > 64 * FRACUNIT && (P_Random() & 1))
		{
			ang += ANG180;
			thrust *= 4;
		}
		
		ang >>= ANGLETOFINESHIFT;
		
		amomx = FixedMul(thrust, finecosine[ang]);
		amomy = FixedMul(thrust, finesine[ang]);
		target->momx += amomx;
		target->momy += amomy;
		
		// added momz (do it better for missiles explosions)
		/* GhostlyDeath Note <June 6, 2008>
		   the following is wrong if they want to support classic demos because 1.09
		   never thrusted on the Z plane
		
		   if (source && demo_version >= 124 && (demo_version < 129 || !cv_allowrocketjump.value))
		 */
		if (source && P_EXGSGetValue(PEXGSBID_COROCKETZTHRUST) && (!P_EXGSGetValue(PEXGSBID_COALLOWROCKETJUMPING) || !DEMOCVAR(allowrocketjump).value))
		{
			int dist, z;
			
			if (source == target)	// rocket in yourself (suicide)
			{
				viewx = inflictor->x;
				viewy = inflictor->y;
				z = inflictor->z;
			}
			else
			{
				viewx = source->x;
				viewy = source->y;
				z = source->z;
			}
			dist = R_PointToDist(target->x, target->y);
			
			viewx = 0;
			viewy = z;
			ang = R_PointToAngle(dist, target->z);
			
			ang >>= ANGLETOFINESHIFT;
			amomz = FixedMul(thrust, finesine[ang]);
		}
		else					//SoM: 2/28/2000: Added new function.
			if (P_EXGSGetValue(PEXGSBID_COALLOWROCKETJUMPING) && DEMOCVAR(allowrocketjump).value)
			{
				fixed_t delta1 = abs(inflictor->z - target->z);
				fixed_t delta2 = abs(inflictor->z - (target->z + target->height));
				
				amomz = (abs(amomx) + abs(amomy)) >> 1;
				
				if (delta1 >= delta2 && inflictor->momz < 0)
					amomz = -amomz;
			}
			
		target->momz += amomz;
	}
	
	takedamage = false;
	// player specific
	if (player && (target->flags & MF_CORPSE) == 0)
	{
		// end of game hell hack
		if (target->subsector->sector->special == 11 && damage >= target->health)
		{
			damage = target->health - 1;
		}
		// Below certain threshold,
		// ignore damage in GOD mode, or with INVUL power.
		if (damage < 1000 && ((player->cheats & CF_GODMODE) || player->powers[pw_invulnerability]))
		{
			return false;
		}
		
		if (player->armortype)
		{
			if (player->armortype == 1)
				saved = damage / 3;
			else
				saved = damage / 2;
				
			if (player->armorpoints <= saved)
			{
				// armor is used up
				saved = player->armorpoints;
				player->armortype = 0;
			}
			player->armorpoints -= saved;
			damage -= saved;
		}
		// added team play and teamdamage (view logboris at 13-8-98 to understand)
		if (P_EXGSGetValue(PEXGSBID_CODISABLETEAMPLAY) ||	// support old demo version
		        cv_teamdamage.value || damage > 1000 ||	// telefrag
		        source == target || !source || !source->player || (cv_deathmatch.value && (!cv_teamplay.value || !ST_SameTeam(source->player, player))))
		{
			player->health -= damage;	// mirror mobj health here for Dave
			if (player->health < 0)
				player->health = 0;
			takedamage = true;
			
			player->damagecount += damage;	// add damage after armor / invuln
			
			if (player->damagecount > 100)
				player->damagecount = 100;	// teleport stomp does 10k points...
				
			//added:22-02-98: force feedback ??? electro-shock???
			if (player == &players[consoleplayer[0]])
				I_Tactile(40, 10, 40 + (damage < 100 ? damage : 100) * 2);
		}
		player->attacker = source;
	}
	else
		takedamage = true;
		
	if (takedamage)
	{
		// do the damage
		target->health -= damage;
		if (target->health <= 0)
		{
			target->special1 = damage;
			if (player && inflictor && !player->chickenTics)
			{
				// Check for flame death
				if ((inflictor->flags2 & MF2_FIREDAMAGE))
				{
					target->flags2 |= MF2_FIREDAMAGE;
				}
			}
			P_KillMobj(target, inflictor, source);
			return true;
		}
		
		// GhostlyDeath <April 20, 2012> -- One Hit Kills
		if (P_EXGSGetValue(PEXGSBID_GAMEONEHITKILLS))
		{
			// Set health to zero
			if (target->health > 0)
				target->health = 0;
			
			// Set player's health to zero also
			if (target->player)
				if (target->player->health > 0)
					target->player->health = 0;
			
			// Kill it
			P_KillMobj(target, inflictor, source);
			return true;
		}
		
		if ((P_Random() < target->info->painchance) && !(target->flags & (MF_SKULLFLY | MF_CORPSE)))
		{
			target->flags |= MF_JUSTHIT;	// fight back!
			
			P_SetMobjState(target, target->info->painstate);
		}
		
		target->reactiontime = 0;	// we're awake now...
	}
	
	if ((!target->threshold || (target->RXFlags[0] & MFREXA_NOTHRESHOLD)) && source && source != target && !(source->RXFlags[0] & MFREXA_NOTRETAILIATETARGET) && !(source->flags2 & MF2_BOSS))
	{
		// If this is another player, do not target
		if (P_EXGSGetValue(PEXGSBID_FUNNOTARGETPLAYER))
			if (source->player)
				return;
		
		// if not intent on another player,
		// chase after this one
		target->target = source;
		target->threshold = BASETHRESHOLD;
		if (target->state == &states[target->info->spawnstate] && target->info->seestate != S_NULL)
			P_SetMobjState(target, target->info->seestate);
	}
	
	return takedamage;
}
