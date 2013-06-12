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
//      Handling interactions (i.e., collisions).

#include "doomdef.h"
#include "i_system.h"			//I_Tactile currently has no effect

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

// added 4-2-98 (Boris) for dehacked patch
// (i don't like that but do you see another solution ?)
int MAXHEALTH = 100;

/* P_PlayerBestWeapon() -- Returns the best (or worst) weapon this player has */
PI_wepid_t P_PlayerBestWeapon(player_t* const a_Player, const bool_t a_Best)
{
	int i, j, BestSlot, CurrentSlot;
	PI_wepid_t Best;
	
	/* Check */
	if (!a_Player)
		return wp_nochange;
	
	/* Go through all weapons */
	Best = wp_nochange;
	for (i = 0; i < NUMWEAPONS; i++)
	{
		// Don't own this gun?
		if (!a_Player->weaponowned[i])
			continue;
		
		// Locked weapon?
		if (!P_WeaponIsUnlocked(i))
			continue;
		
		// No Ammo?
		if (a_Player->weaponinfo[i]->ammo != am_noammo)
			if (a_Player->ammo[a_Player->weaponinfo[i]->ammo] <= 0)
				continue;
		
		// Find slot where this weapon is
		for (j = 0; j < NUMWEAPONS; j++)
			if (a_Player->FavoriteWeapons[j] == i)
			{
				CurrentSlot = j;
				break;
			}
		
		// Better or worse weapon?
		if (Best == wp_nochange ||
			(a_Best && CurrentSlot > BestSlot) ||
			(!a_Best && CurrentSlot < BestSlot))
		{
			Best = i;
			BestSlot = CurrentSlot;
		}
	}
	
	/* None found? */
	if (Best == wp_nochange)
		return wp_nochange;
	
	/* Current is Best */
	if (Best == a_Player->readyweapon)
		return wp_nochange;
	
	/* Return Best */
	return Best;
}

/* P_PlayerSwitchToFavorite() -- Switch to favorite weapon */
void P_PlayerSwitchToFavorite(player_t* const a_Player, const bool_t a_JustSpawned)
{
	PI_wepid_t NewGun;
	
	/* Don't switch when not freshly reborn */
	if (!(a_JustSpawned && P_XGSVal(PGS_PLSPAWNWITHFAVGUN)) && (a_Player->pendingweapon != wp_nochange))
		return;
	
	/* Change to the best gun */
	NewGun = P_PlayerBestWeapon(a_Player, true);

	if (NewGun != wp_nochange && NewGun != NUMWEAPONS &&
		NewGun != a_Player->readyweapon)
	{
		a_Player->pendingweapon = NewGun;
		
		// If we just spawned (and spawnfav enabled), set the ready weapon also
		if (a_JustSpawned && P_XGSVal(PGS_PLSPAWNWITHFAVGUN))
			a_Player->readyweapon = NewGun;
	}
}

//
// GET STUFF
//

int FindBestWeapon(player_t* player)
{
	return wp_nochange;
}

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

//
// P_GiveAmmo
// Num is the number of clip loads,
// not the individual count (0= 1/2 clip).
// Returns false if the ammo can't be picked up at all
//

bool_t P_GiveAmmo(player_t* player, PI_ammoid_t ammo, int count)
{
	int oldammo;
	size_t i;
	PI_wepid_t ChoseWeapon;
	G_Skill_t Skill;
	
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
	 
	Skill = P_XGSVal(PGS_GAMESKILL);
	if (Skill == sk_baby || Skill == sk_nightmare || P_XGSVal(PGS_PLDOUBLEAMMO))
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
			P_PlayerSwitchToFavorite(player, false);
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
bool_t P_GiveWeapon(player_t* player, PI_wepid_t weapon, bool_t dropped)
{
	bool_t gaveammo;
	bool_t gaveweapon;
	int ammo_count;
	int i;
	PI_ammoid_t AmmoType;
	
	/* Obtain Ammo Type */
	AmmoType = player->weaponinfo[weapon]->ammo;
	
	/* Coop/DM Mode */
	if (P_XGSVal(PGS_ITEMSKEEPWEAPONS) && !dropped)
	{
		// leave placed weapons forever on net games
		if (player->weaponowned[weapon])
			return false;
			
		player->bonuscount += BONUSADD;
		player->weaponowned[weapon] = true;
		
		if (AmmoType >= 0 && AmmoType < NUMAMMO)
		{
			if (P_GMIsDM())
				P_GiveAmmo(player, player->weaponinfo[weapon]->ammo, 5 * ammoinfo[player->weaponinfo[weapon]->ammo]->ClipAmmo);
			else
				P_GiveAmmo(player, player->weaponinfo[weapon]->ammo, player->weaponinfo[weapon]->GetAmmo);
		}
		
		// Boris hack preferred weapons order...
		// TODO FIXME: Reimplement player weapon order
//		if (player->originalweaponswitch || player->favoritweapon[weapon] > player->favoritweapon[player->readyweapon])
//			player->pendingweapon = weapon;	// do like Doom2 original
		
		// GhostlyDeath <May 20, 2012> -- Force weapon switch
		if (P_XGSVal(PGS_PLFORCEWEAPONSWITCH) || player->originalweaponswitch)
			player->pendingweapon = weapon;
		
		// Or Select Favorite
		else
			P_PlayerSwitchToFavorite(player, false);
			
		//added:16-01-98:changed consoleplayer to displayplayer
		//               (hear the sounds from the viewpoint)
		for (i = 0; i < g_SplitScreen + 1; i++)
			if (player == &players[g_Splits[i].Display])
				S_StartSound(NULL, sfx_wpnup);
		return true;
	}
	
	if (AmmoType >= 0 && AmmoType < NUMAMMO)
	{
		// give one clip with a dropped weapon,
		// two clips with a found weapon
		if (dropped)
			ammo_count = has_ammo_dropped ? (has_ammo_dropped < 0 ? 0 : has_ammo_dropped) : ammoinfo[player->weaponinfo[weapon]->ammo]->ClipAmmo;
		else
			ammo_count = player->weaponinfo[weapon]->GetAmmo;
		
		// Give ammo, possibly
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
		
		// GhostlyDeath <May 20, 2012> -- Force weapon switch
		if (P_XGSVal(PGS_PLFORCEWEAPONSWITCH) || player->originalweaponswitch)
			player->pendingweapon = weapon;
		
		// Or Select Favorite
		else
			P_PlayerSwitchToFavorite(player, false);
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

/* P_FlashKeys() -- Flashes keycards needed by player */
void P_FlashKeys(player_t* const a_Player, const bool_t a_WildCard, const uint32_t a_SetA, const uint32_t a_SetB)
{
	uint32_t Bits, m, s;
	uint32_t TimeBase;
	
	/* Check */
	if (!a_Player || (!a_SetA && !a_SetB))
		return;
	
	/* Init */
	// Timebase is the initial tics to show a key icon for
	TimeBase = 0x10 * 4;
	
	/* Loop */
	for (m = 0; m < 2; m++)
	{
		// Init Bits
		Bits = (m ? a_SetB : a_SetA);
		
		// Handle each bit
		for (s = 0; s < 32; s++)
			if (Bits & (1 << s))
			{
				// Set time to base
				a_Player->KeyFlash[m][s] = TimeBase;
				
				// Create a scroll like effect when wildcarding
				if (a_WildCard)
					TimeBase += 0x8;
			}
	}
}

/* P_PlayerMessage() -- Handles player messages */
void P_PlayerMessage(const P_PMType_t a_Type, mobj_t* const a_Picker, mobj_t* const a_Upper, const char** const a_MessageRef)
{
#define BUFSIZE 128
	int32_t LocalPlayer, i, s;
	char Buf[BUFSIZE];
	D_Prof_t* Prof;
	uint8_t Color;
	
	/* Check */
	if (!a_Picker || !a_MessageRef)
		return;
	
	/* Message references nothing */
	if (!*a_MessageRef)
		return;
		
	/* If the object picking it up is not a player... */
	if (!P_MobjIsPlayer(a_Picker))
		return;
	
	/* Handle Message for everyone (multiple screens) */
	// This is so that if multiple players are viewing the same player, they get
	// the same pickup messages rather than the first one.
	for (s = 0; s < MAXSPLITSCREEN; s++)
	{
		// Not POV player?
		if (P_SpecGetPOV(s) != a_Picker->player)
			continue;
		
		// Invisible split?
		if (s > g_SplitScreen)
			continue;
	
		// Get Profile
		Prof = a_Picker->player->ProfileEx;
		
		// Find message to print
		memset(Buf, 0, sizeof(Buf));
		strncpy(Buf, *a_MessageRef, BUFSIZE);
		
		// Print to console (to that player only)
		if (s >= 1)
			CONL_PrintF("%c", 4 + (s - 1));
		
		// Send pickup color
		Color = 0;
	
		switch (a_Type)
		{
			case PPM_PICKUP: Color = (Prof ? Prof->ColorPickup : VEX_MAP_WHITE); break;
			case PPM_SECRET: Color = (Prof ? Prof->ColorSecret : VEX_MAP_BRIGHTWHITE); break;
			case PPM_REDLOCK: Color = (Prof ? Prof->ColorLock[0] : VEX_MAP_RED); break;
			case PPM_YELLOWLOCK: Color = (Prof ? Prof->ColorLock[1] : VEX_MAP_YELLOW); break;
			case PPM_BLUELOCK: Color = (Prof ? Prof->ColorLock[2] : VEX_MAP_BLUE); break;
			case PPM_GENLOCK: Color = (Prof ? Prof->ColorLock[3] : VEX_MAP_GRAY); break;
			default: break;
		}
	
		// Print Color
		if (Color >= 0 && Color < NUMVEXCOLORS)
			CONL_PrintF("{%c", (Color < 10 ? '0' + Color : 'a' + (Color - 10)));
	
		// Send actual message
		CONL_PrintF("%s{z\n", Buf);
	}
#undef BUFSIZE
}

bool_t P_FlagTouchFunc(struct mobj_s* const a_Special, struct mobj_s* const a_Toucher);
bool_t P_HomeTouchFunc(struct mobj_s* const a_Special, struct mobj_s* const a_Toucher);

/* c_TouchFuncList() -- Touch function list */
static const struct
{
	uint32_t ID;
	P_TouchFunc_t Func;
} c_TouchFuncList[] =
{
	{UINT32_C(0xF7AC0001), P_FlagTouchFunc},
	{UINT32_C(0xF7AC0002), P_HomeTouchFunc},
	
	{0, NULL},
};

/* P_TouchFuncToID() -- Converts touch function to ID */
uint32_t P_TouchFuncToID(P_TouchFunc_t a_Func)
{
	int32_t i;
	
	/* Look in list */
	for (i = 0; c_TouchFuncList[i].ID; i++)
		if (a_Func == c_TouchFuncList[i].Func)
			return c_TouchFuncList[i].ID;
	
	/* Failed */
	return 0;
}

/* P_TouchIDToFunc() -- Converts ID to touch function */
P_TouchFunc_t P_TouchIDToFunc(const uint32_t a_ID)
{
	int32_t i;
	
	/* Look in list */
	for (i = 0; c_TouchFuncList[i].ID; i++)
		if (a_ID == c_TouchFuncList[i].ID)
			return c_TouchFuncList[i].Func;
	
	/* Failed */
	return NULL;
}

/* P_TouchSpecialThing() -- Performed when toucher with MF_PICKUP touches special with MF_SPECIAL */
bool_t P_TouchSpecialThing(mobj_t* special, mobj_t* toucher)
{
#define BUFSIZE 16
	char Buf[BUFSIZE], FirstChar, LastChar;
	const char* HackRef;
	player_t* player;
	int i, j, k, n;
	fixed_t delta;
	int sound;
	PI_touch_t* Current;
	bool_t OKStat, NewWear, PickedUp, CancelRemove;
	int32_t Target, Max, Amount;
	char* SplitMessageRef;
	PI_key_t* RMODKey;
	
	delta = special->z - toucher->z;
	
	//SoM: 3/27/2000: For some reason, the old code allowed the player to
	//grab items that were out of reach...
	if (delta > toucher->height || delta < -special->height)
	{
		// out of reach
		return false;
	}
	
	// Dead thing touching.
	// Can happen with a sliding player corpse.
	if (toucher->health <= 0 || toucher->flags & MF_CORPSE)
		return false;
	
	/* If there is an alternative pickup function, use that */
	if (special->AltTouchFunc)
		return special->AltTouchFunc(special, toucher);
	
	sound = sfx_itemup;
	
	player = NULL;
	if (P_MobjIsPlayer(toucher))
		player = toucher->player;
		
	// FWF support
	has_ammo_dropped = special->dropped_ammo_count;
	
	/* Find sprite by ID and match to touch special list */
	OKStat = PickedUp = false;
	for (i = 0; i < g_RMODNumTouchSpecials; i++)
	{
		// Get current
		Current = g_RMODTouchSpecials[i];
		
		// Wrong sprite?
		if (Current->ActSpriteID != special->state->SpriteID)
			continue;
		
		// Check if monster is grabbing and cannot pick this thing up
		if (!player)
			// Monster cannot pickup thing
			if (!(Current->Flags & PMTSF_MONSTERCANGRAB))
				return false;
		
		// Cancel removal
		CancelRemove = false;
		
		// Weapon
		if (player)
		{
			// Give key?
			if (Current->ActGiveKey >= 0 && Current->ActGiveKey < g_RMODNumKeys)
			{
				RMODKey = g_RMODKeys[Current->ActGiveKey];
				
				// Valid?
				if (RMODKey)
				{
					// Set player keygroup to mask value
						// Provided we lack this key
					if (((Current->Flags & PMTSF_KEEPNOTNEEDED) && !(player->KeyCards[RMODKey->Group] & RMODKey->Bit)) || !(Current->Flags & PMTSF_KEEPNOTNEEDED))
					{
						player->KeyCards[RMODKey->Group] |= RMODKey->Bit;
						OKStat |= true;
					}
					
					if (OKStat)
						PickedUp = true;
				}
			}
			
			// Give gun?
			if (Current->ActGiveWeapon >= 0 && Current->ActGiveWeapon < NUMWEAPONS)
			{
				// Give weapon?
				if (!P_XGSVal(PGS_ITEMSKEEPWEAPONS) ||
					(P_XGSVal(PGS_ITEMSKEEPWEAPONS) && !player->weaponowned[Current->ActGiveWeapon]) ||
					(special->flags & MF_DROPPED))
					OKStat |= P_GiveWeapon(player, Current->ActGiveWeapon, special->flags & MF_DROPPED);
				
				// Remove?
				if (OKStat)
				{
					PickedUp = true;
					
					// Cancel removal in coop/dm
					if (P_XGSVal(PGS_ITEMSKEEPWEAPONS))
						CancelRemove = !(special->flags & MF_DROPPED);
				}
			}
			
			// Give Ammo?
			if ((Current->ActGiveAmmo >= 0 && Current->ActGiveAmmo < NUMAMMO && Current->ActGiveAmmo != am_noammo) || Current->ActGiveAmmo == am_all)
			{
				// Giving all ammo
				if (Current->ActGiveAmmo == am_all)
				{
					j = 0;
					n = NUMAMMO;
				}
				
				// Giving only single ammo type
				else
				{
					j = Current->ActGiveAmmo;
					n = j + 1;
				}
				
				// Give ammo types
				for (; j < n; j++)
				{
					Amount = ammoinfo[j]->ClipAmmo * Current->AmmoMul;
					
					// Dropped ammo?
					if (special->flags & MF_DROPPED)
						Amount /= 2;
					
					// No ammo?
					if (Amount <= 0)
						Amount = 1;
					
					OKStat |= P_GiveAmmo(player, j, Amount);
					if (OKStat)
						PickedUp = true;
						
					// Modify max ammo?
					// When setbackpack is cleared or it is set and we don't have a backpack
					if ((!(Current->Flags & PMTSF_SETBACKPACK)) || ((Current->Flags & PMTSF_SETBACKPACK) && !player->backpack))
						player->maxammo[j] *= Current->MaxAmmoMul;
				}
				
				// Set backpack?
				if (OKStat || PickedUp)
					if ((Current->Flags & PMTSF_SETBACKPACK) && !player->backpack)
						if (Current->Flags & PMTSF_SETBACKPACK)
							player->backpack = true;
			}
		}	
		
		// Health
		if (Current->HealthAmount)
		{
			// Get target amount
			Target = toucher->health + Current->HealthAmount;
			NewWear = true;
			
			// Determine caps?
			if ((Current->Flags & PMTSF_CAPNORMSTAT))
				Max = player->MaxHealth[0];
			else if ((Current->Flags & PMTSF_CAPMAXSTAT))
				Max = player->MaxHealth[1];
			else
				Max = 9999999;
			
			// Limit Health?
			if (Target >= Max)
			{
				if (toucher->health >= Max)
				{
					Amount = toucher->health;
					NewWear = false;
				}
				else
					Amount = Max;
			}
			else
				Amount = Target;
			
			// Allow devaluing
			if ((Current->Flags & PMTSF_DEVALUE) && toucher->health >= Max)
				Amount = Max;
			
			// Not Needed? Reverse of that
			if (!(!NewWear && (Current->Flags & PMTSF_KEEPNOTNEEDED)))
			{
				// Change Health
				if (player)
					player->health = Amount;
				toucher->health = Amount;
				OKStat = true;
				
				// Set as picked up
				PickedUp = true;
			}
		}
		
		// Armor
		if (player && Current->ArmorAmount)
		{
			// Get target amount
			Target = player->armorpoints + Current->ArmorAmount;
			NewWear = true;
			
			// Determine caps?
			if ((Current->Flags & PMTSF_CAPNORMSTAT))
				Max = player->MaxArmor[0];
			else if ((Current->Flags & PMTSF_CAPMAXSTAT))
				Max = player->MaxArmor[1];
			else
				Max = 9999999;
			
			// Limit Armor?
			if (Target >= Max)
			{
				if (player->armorpoints >= Max)
				{
					Amount = player->armorpoints;
					NewWear = false;
				}
				else
					Amount = Max;
			}
			else
				Amount = Target;
			
			// Allow devaluing
			if ((Current->Flags & PMTSF_DEVALUE) && player->armorpoints >= Max)
				Amount = Max;
			
			// Not Needed? Reverse of that
			if (!(!NewWear && (Current->Flags & PMTSF_KEEPNOTNEEDED)))
			{
				// Change Armor
				player->armorpoints = Amount;
				OKStat = true;
					
				// Set as picked up
				PickedUp = true;
			
				// Change armor class?
				CONL_PrintF("AC = %i\n", Current->ArmorClass);
				if (Current->ArmorClass)
				{
					// Get target armor class
					Target = Current->ArmorClass;
				
					// Limited?
					if ((Current->Flags & PMTSF_GREATERARMORCLASS))
						Max = player->armortype;
					else
						Max = 0;
				
					// Actual?
					if (Current->ArmorClass >= Max)
						Amount = Current->ArmorClass;
					else
						Amount = player->armortype;
					
					CONL_PrintF("P = %i, A = %i, M = %i, C = %i\n",
							player->armortype,
							Amount,
							Max,
							Current->ArmorClass
						);
				
					// Change armor
					player->armortype = Amount;
				}
			}
		}
		
		// Not picked up?
		if (!PickedUp)
			return false;
		
		// Emit sounds and change colors
			// Determine sound
		if (Current->PickupSnd)
			sound = S_SoundIDForName(Current->PickupSnd);
		else
			sound = sfx_itemup;
		
			// For Player
		if (player)
		{
			// Counts as an item?
			if (special->flags & MF_COUNTITEM)
			{
				player->itemcount++;
				g_MapKIS[1]++;
				P_UpdateScores();
			}
			
			player->bonuscount += BONUSADD;
			
			//added:16-01-98:consoleplayer -> displayplayer (hear sounds from viewpoint)
			for (i = 0; i < g_SplitScreen + 1; i++)
				if (player == &players[g_Splits[i].Display])
					S_StartSound(NULL, sound);
		}
		
			// For Monster
		else
		{
			// Counts as item?
			if (special->flags & MF_COUNTITEM)
				g_MapKIS[1]++;
			
			// Emit sound from monster
			S_StartSound(toucher, sound);
		}
		
		// Remember Short Nice Name
		SplitMessageRef = special->info->RSNiceName;
		
		// The object picking this up happens to have died?
		if (toucher->health <= 0)
			P_KillMobj(toucher, special, special);
		
		// Cancel removing in multi-player mode?
		if (Current->Flags & PMTSF_KEEPINMULTI)
			if (P_XGSVal(PGS_COMULTIPLAYER))
				CancelRemove = true;
		
		// Remove if we used it? or remove regardless
		if (!CancelRemove && (((Current->Flags & PMTSF_KEEPNOTNEEDED) && OKStat) || !(Current->Flags & PMTSF_KEEPNOTNEEDED) || (Current->Flags & PMTSF_REMOVEALWAYS)))
			P_RemoveMobj(special);
		
		// Message?
			// 3/4 Player (Hacky)
		if (g_SplitScreen > 1)
		{
			// Get characters of short name (of the object)
			FirstChar = tolower(SplitMessageRef[0]);
			LastChar = tolower(SplitMessageRef[strlen(SplitMessageRef) - 1]);
			
			// Plural?
			if (LastChar == 's')
				snprintf(Buf, BUFSIZE, "%s", SplitMessageRef);
			
			// Singular
			else
			{
				// Vowel?
				if (FirstChar == 'a' || FirstChar == 'e' || FirstChar == 'i' ||
					FirstChar == 'o' || FirstChar == 'u')
					snprintf(Buf, BUFSIZE, "an %s", SplitMessageRef);
				else
					snprintf(Buf, BUFSIZE, "a %s", SplitMessageRef);
			}
			
			// Show it
			HackRef = Buf;
			P_PlayerMessage(PPM_PICKUP, toucher, special, &HackRef);
		}
			// 1/2 Player
		else
		{
			if (Current->PickupMsgRef)
				P_PlayerMessage(PPM_PICKUP, toucher, special, Current->PickupMsgRef);
		}
		
		// Don't process anymore
		return true;
	}
	
	/* Nothing picked up */
	return false;
	
#if 0
	// Identify by sprite.
	switch (special->sprite)
	{
			
			// power ups
		case SPR_PINV:
			if (!P_GivePower(player, pw_invulnerability))
				return;
			P_PlayerMessage(toucher, special, GOTINVUL);
			sound = sfx_getpow;
			break;
			
		case SPR_PSTR:
			if (!P_GivePower(player, pw_strength))
				return;
			P_PlayerMessage(toucher, special, GOTBERSERK);
			if (player->readyweapon != wp_fist)
				player->pendingweapon = wp_fist;
			sound = sfx_getpow;
			break;
			
		case SPR_PINS:
			if (!P_GivePower(player, pw_invisibility))
				return;
			P_PlayerMessage(toucher, special, GOTINVIS);
			sound = sfx_getpow;
			break;
			
		case SPR_SUIT:
			if (!P_GivePower(player, pw_ironfeet))
				return;
			P_PlayerMessage(toucher, special, GOTSUIT);
			sound = sfx_getpow;
			break;
			
		case SPR_PMAP:
			if (!P_GivePower(player, pw_allmap))
				return;
			P_PlayerMessage(toucher, special, GOTMAP);
			sound = sfx_getpow;
			break;
			
		case SPR_PVIS:
			if (!P_GivePower(player, pw_infrared))
				return;
			P_PlayerMessage(toucher, special, GOTVISOR);
			sound = sfx_getpow;
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
			P_PlayerMessage(toucher, special, GOTBACKPACK);
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
		if (player == &players[g_Splits[i].Display])
			S_StartSound(NULL, sound);
#endif
#undef BUFSIZE
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
		if (P_MobjIsPlayer(a_Mobj))
		{
			// Special
			if (a_Special)
				*a_Special = true;
				
			// Return the player's name
			if (!a_IsInflictor)
				return D_NCSGetPlayerName(a_Mobj->player - players);
			
			// Return the name of the gun
			else
			{
				// Check for telefrags
				if (a_Mobj->RXAttackAttackType == PRXAT_TELEFRAG)
					return "TeleFrag";
					
				// Check for suicide
				if (a_Mobj->RXAttackAttackType == PRXAT_SUICIDE)
					return "Suicide Pill";
					
				// Inflictor is the source (melee or gun attack?)
				else if (a_Mobj == a_Source && a_Mobj->player->weaponinfo)
					return a_Mobj->player->weaponinfo[a_Mobj->player->readyweapon]->NiceName;
				
				// It must be a missile then, return the weapon there
				else
				{
					if (a_Mobj->RXShotWithWeapon >= 0 && a_Mobj->RXShotWithWeapon < NUMWEAPONS && a_Mobj->player->weaponinfo)
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
				*a_Special = false;
			
			// If the source has a weapon attached to it 
			if (a_Mobj->RXShotWithWeapon >= 0 && a_Mobj->RXShotWithWeapon < NUMWEAPONS)
			{
				// Check to see if there is a source player (use that name)
				if (P_MobjIsPlayer(a_Source))
					return a_Source->player->weaponinfo[a_Mobj->RXShotWithWeapon]->NiceName;
				
				// There is no player source, so use standard gun name
				else
					return wpnlev1info[a_Mobj->RXShotWithWeapon]->NiceName;
			}
			
			// If we never returned, then there was no weapon used
			if (!a_IsInflictor)
			{
				// Return nice name of object
				if (g_SplitScreen > 1)
					return a_Mobj->info->RSNiceName;
				else
					return a_Mobj->info->RNiceName;
			}
			
			// Return attack type if the inflictor is the source
			else if (a_IsInflictor && a_Mobj == a_Source)
			{
				// 3/4 Player Split
				if (g_SplitScreen > 1)
					switch (a_Mobj->RXAttackAttackType)
					{
						case PRXAT_MELEE:
							return "Melee";
						
						case PRXAT_RANGED:
							return "Ranged";
						
						case PRXAT_TELEFRAG:
							return "TeleFrag";
							
						case PRXAT_SUICIDE:
							return "Suicide";
						
						case PRXAT_UNKNOWN:
						default:
							return "Unknown";
					}
				
				// 1/2 Player Split
				else
					switch (a_Mobj->RXAttackAttackType)
					{
						case PRXAT_MELEE:
							return "Melee Attack";
						
						case PRXAT_RANGED:
							return "Ranged Attack";
						
						case PRXAT_TELEFRAG:
							return "TeleFrag";
							
						case PRXAT_SUICIDE:
							return "Suicide Pill";
						
						case PRXAT_UNKNOWN:
						default:
							return "Unknown";
					}
			}
			
			// Otherwise return the inflictor
			else if (a_IsInflictor && a_Mobj != a_Source)
			{
				// Return nice name of object
				if (g_SplitScreen > 1)
					return a_Mobj->info->RSNiceName;
				else
					return a_Mobj->info->RNiceName;
			}
		}
	}

	/* No name */
	return NULL;
}

/* P_BroadcastMessage() -- Broadcasts message to all players */
void P_BroadcastMessage(const char* const a_Message)
{
	CONL_PrintF("\x7{z%s\n", a_Message);
}

/* P_ExitMessage() -- Player exited the level */
void P_ExitMessage(mobj_t* const a_Exiter, const char* const a_Message)
{
	const char* eNoun;
	
	/* Object exited the level */
	if (a_Exiter)
	{
		// Get name of the object that exited
		eNoun = PS_GetMobjNoun(a_Exiter, NULL, false, a_Exiter);

		CONL_PrintF("\x7{z%s{z exited the level.\n", eNoun);
	}
	
	/* Normal message */
	else if (a_Message)
	{
		CONL_PrintF("\x7{z%s\n", a_Message);
	}
}

/* P_DeathMessages() -- Display message of thing dying */
void P_DeathMessages(mobj_t* target, mobj_t* inflictor, mobj_t* source)
{
#define BUFSIZE 128
	char Message[BUFSIZE];
	const char* tNoun, *iNoun, *sNoun, **chNoun;
	bool_t tSpecial, iSpecial, sSpecial;
	const char* SrcPrefix, *TargPrefix;
	char SrcColor, TargColor;
	
	int32_t i, c, n, x;
	char PLSource[MAXPLAYERNAME + 1];
	char PLTarget[MAXPLAYERNAME + 1];
	mobj_t** MoP;
	char* NameBuf, *p;
	uint16_t WChar;
	size_t BSkip;
	
	/* Determine nouns of objects */
	tNoun = PS_GetMobjNoun(target, &tSpecial, false, source);
	iNoun = PS_GetMobjNoun(inflictor, &iSpecial, true, source);
	sNoun = PS_GetMobjNoun(source, &sSpecial, false, source);
	
	/* If neither side is special, who cares? */
	// Only care for target specials
	if (!(tSpecial/* | sSpecial*/))
		return;
	
	/* Colors to use */
	// Default Colors
	SrcPrefix = "";
	TargPrefix = "";
	SrcColor = '4';
	TargColor = '5';
	
	// Check Team Game
	if (P_GMIsTeam())
	{
		// Source is on a team
		if (source)
			if (P_MobjIsPlayer(source) || source->SkinTeamColor)
			{
				SrcPrefix = "x7";
				if (P_MobjIsPlayer(source))
					SrcColor = source->player->skincolor;
				else
					SrcColor = (source->SkinTeamColor - 1);
				
				if (SrcColor >= 10)
					SrcColor = 'a' + (SrcColor - 10);
				else
					SrcColor = '0' + SrcColor;
			}
			
		// Target is on a team
		if (target)
			if (P_MobjIsPlayer(target) || target->SkinTeamColor)
			{
				TargPrefix = "x7";
				if (P_MobjIsPlayer(target))
					TargColor = target->player->skincolor;
				else
					TargColor = (target->SkinTeamColor - 1);
					
				if (TargColor >= 10)
					TargColor = 'a' + (TargColor - 10);
				else
					TargColor = '0' + TargColor;
			}
	}
	
	/* Remap source and target? */
	// This is for 3/4 split to make some room available
	if (g_SplitScreen > 1)
		for (i = 0; i < 2; i++)
		{
			// Source
			if (!i)
			{
				MoP = &source;
				NameBuf = PLSource;
				chNoun = &sNoun;
			}
		
			// Target
			else
			{
				MoP = &target;
				NameBuf = PLTarget;
				chNoun = &tNoun;
			}
		
			// No mobj?
			if (!*MoP)
				continue;
		
			// Not a player?
			if (!P_MobjIsPlayer(*MoP))
				continue;
			
			// Clear Buffer
			memset(NameBuf, 0, MAXPLAYERNAME + 1);
			
			// Go through name
			for (p = *chNoun, BSkip = 1, c = 0, n = 0; *p && c < 6; p += BSkip)
			{
				// Convert to wide
				WChar = V_ExtMBToWChar(p, &BSkip);
				
				// End?
				if (WChar == 0)
					break;
				
				// Convert back to buffer
				for (x = 0; x < BSkip; x++)
					if (n <= MAXPLAYERNAME)
						NameBuf[n++] = p[x];
				
				// If color code stuff, do not add
				if (WChar < 0xF100 || WChar > 0xFFFF)
					c++;
			}
			
			// Set use new buffer
			*chNoun = NameBuf;
		}
	
	/* Print message */
	// 3/4 Split
	if (g_SplitScreen > 1)
		if (target == source)
			CONL_PrintF("\x7{%s%c%s{0< {2({3%s{2)\n", SrcPrefix, SrcColor, sNoun, iNoun);
		else
			CONL_PrintF("\x7{%s%c%s{0/{%s%c%s {2({3%s{2)\n", SrcPrefix, SrcColor, sNoun, TargPrefix, TargColor, tNoun, iNoun);
	
	// 1/2 Split
	else
		if (target == source)
			CONL_PrintF("\x7{%s%c%s{0 <- {2({3%s{2)\n", SrcPrefix, SrcColor, sNoun, iNoun);
		else
			CONL_PrintF("\x7{%s%c%s{0 -> {%s%c%s {2({3%s{2)\n", SrcPrefix, SrcColor, sNoun, TargPrefix, TargColor, tNoun, iNoun);
	
#undef BUFSIZE
}

// WARNING : check cv_fraglimit>0 before call this function !
void P_CheckFragLimit(player_t* p)
{
	if (P_GMIsTeam())
	{
		int fragteam = 0, i;
		
		for (i = 0; i < MAXPLAYERS; i++)
			if (P_MobjOnSameTeam(p->mo, players[i].mo))
				fragteam += ST_PlayerFrags(i);
				
		if (P_XGSVal(PGS_GAMEFRAGLIMIT) <= fragteam)
			G_ExitLevel(false, NULL, DS_GetString(DSTR_PINTERC_FRAGLIMITREACHED));
	}
	else
	{
		if (P_XGSVal(PGS_GAMEFRAGLIMIT) <= ST_PlayerFrags(p - players))
			G_ExitLevel(false, NULL, DS_GetString(DSTR_PINTERC_FRAGLIMITREACHED));
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
	PI_ammoid_t ammo = player->weaponinfo[player->readyweapon]->ammo;
	int ammo_count = player->ammo[ammo];
	
	return ammo == am_noammo ? 0 : ammo_count ? ammo_count : -1;
}

// P_KillMobj
//
//      source is the attacker,
//      target is the 'target' of the attack, target dies...
//                                          113
void P_KillMobj(mobj_t* target, mobj_t* inflictor, mobj_t* source)
{
	PI_mobjid_t GibsType = 0;
	PI_mobjid_t item = 0;
	mobj_t* mo;
	int drop_ammo_count = 0;
	int i, GibTarget;
	
	// GhostlyDeath <May 22, 2012> -- Death total
	if (P_MobjIsPlayer(target))
	{
		target->player->TotalDeaths++;
		g_MapKIS[4]++;
		P_UpdateScores();
		
		// If using new game mode, set color to match VTeam
		target->FakeColor = P_GetMobjTeam(target->player->mo) + 1;
	}
	
	// dead target is no more shootable
	if (!P_XGSVal(PGS_GAMESOLIDCORPSES))
		target->flags &= ~MF_SHOOTABLE;
		
	target->flags &= ~(MF_FLOAT | MF_SKULLFLY);
	
	// GhostlyDeath <March 6, 2012> -- Keep gravity on death?
	if (!(target->RXFlags[0] & MFREXA_KEEPGRAVONDEATH))
		target->flags &= ~MF_NOGRAVITY;
		
	// scream a corpse :)
	if (target->flags & MF_CORPSE)
	{
		// turn it to gibs
		GibsType = INFO_GetTypeByName("CrushedGibs"); 
		if (GibsType != NUMMOBJTYPES)
			P_SetMobjState(target, mobjinfo[GibsType]->spawnstate);
		
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
	if ((target->RXFlags[0] & MFREXA_CARRYKILLER) && source && P_MobjIsPlayer(source))
		P_RefMobj(PMRT_TARGET, target, source);
	
	// GhostlyDeath <April 8, 2012> -- If modifying corpses in A_Fall, then don't modify here
	if (!P_XGSVal(PGS_COMODIFYCORPSE))
	{
		// in version 131 and higer this is done later in a_fall
		// (this fix the stepping monster)
		target->flags |= MF_CORPSE | MF_DROPOFF;
		target->height >>= 2;
		
		if (P_XGSVal(PGS_COOLDCUTCORPSERADIUS))
			target->radius -= (target->radius >> 4);	//for solid corpses
	}
	// GhostlyDeath <September 17, 2011> -- Change the way obituaries are done
	// Legacy only did it if the target/source was the first console player
	P_DeathMessages(target, inflictor, source);
	
	// if killed by a player
	target->KillerPlayer = NULL;
	if (source && P_MobjIsPlayer(source))
	{
		// count for intermission
		if (target->flags & MF_COUNTKILL)
		{
			source->player->killcount++;
			target->KillerPlayer = (source->player - players) + 1;
			target->FraggerID = source->player->FraggerID;
			g_MapKIS[0]++;
			P_UpdateScores();
		}
			
		// count frags if player killed player
		if (P_MobjIsPlayer(target))
		{
			// Suicides count against you
			if (target->player == source->player)
				source->player->TotalFrags--;
			
			// Otherwise increase frags
			else
				source->player->TotalFrags++;
			source->player->frags[target->player - players]++;
			g_MapKIS[3]++;
			P_UpdateScores();
			
			// check fraglimit cvar
			if (P_XGSVal(PGS_GAMEFRAGLIMIT))
				P_CheckFragLimit(source->player);
		}
	}
	else if (target->flags & MF_COUNTKILL)
	{
		// count all monster deaths,
		// even those caused by other monsters
		// But unlike Doom, they aren't given to player 1 except in compat mode
		g_MapKIS[0]++;
		
		if (P_XGSVal(PGS_COKILLSTOPLAYERONE) && !P_XGSVal(PGS_COMULTIPLAYER))
			players[0].killcount++;
	}
	
	// GhostlyDeath <June 15, 2012> -- Kill count once?
	if (target->flags & MF_COUNTKILL)
		if (P_XGSVal(PGS_MONKILLCOUNTMODE) == 1)
			target->flags &= ~MF_COUNTKILL;
	
	/* if a player avatar dies... */
	if (P_MobjIsPlayer(target))
	{
		// GhostlyDeath <June 6, 2012> -- Remember ready weapon
		target->player->DeadWeapon = target->player->readyweapon;
		
		// count environment kills against you (you fragged yourself!)
		if (!source)
		{
			target->player->TotalFrags--;
			target->player->frags[target->player - players]++;
			P_UpdateScores();
		}
			
		target->flags &= ~MF_SOLID;	// does not block
		target->flags2 &= ~MF2_FLY;
		target->player->playerstate = PST_DEAD;
		P_DropWeapon(target->player);	// put weapon away
		
		// GhostlyDeath <December 28, 2012> -- Reset aiming angle on death
		for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
			if (D_ScrSplitHasPlayer(i))
				localaiming[i] = 0;
	}
	
	// Target is playing as a monster, needs to be dead for recontrol
	else if (target->player)
		target->player->playerstate = PST_DEAD;
	
	// Gib Target
	GibTarget = target->info->spawnhealth;
	if (P_XGSVal(PGS_GAMEHERETICGIBBING))
		GibTarget >>= 1;
	
	if (target->health < -GibTarget && target->info->xdeathstate)
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
	
	// Drop weapons when player is killed (non-monster players only)
	if (P_MobjIsPlayer(target) && P_XGSVal(PGS_PLDROPWEAPONS))
	{
		drop_ammo_count = P_AmmoInWeapon(target->player);
		//if (!drop_ammo_count)
		//    return;
		
		// Which item to drop?
		if (target->player->weaponinfo[target->player->readyweapon]->DropWeaponClass)
			item = INFO_GetTypeByName(target->player->weaponinfo[target->player->readyweapon]->DropWeaponClass);
		
		// GhostlyDeath <June 6, 2012> -- Weapon was dropped by player, so
		// find the first weapon. This is the case when
		if (item)
		{
			// Set weapon as un-owned
			target->player->weaponowned[target->player->DeadWeapon] = false;
			
			// Switch to the first weapon that the player owns instead
			for (i = 0; i < NUMWEAPONS; i++)
				if (target->player->weaponowned[i])
				{
					target->player->DeadWeapon = i;
					break;
				}
		}
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
		mo = P_SpawnMobj(target->x, target->y, (!P_XGSVal(PGS_COSPAWNDROPSONMOFLOORZ) ? ONFLOORZ : target->floorz), item);
		mo->flags |= MF_DROPPED;	// special versions of items
	
		if (!P_XGSVal(PGS_PLDROPWEAPONS))
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
	angle_t ang;
	int32_t saved;
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
	
	player = NULL;
	if (P_MobjIsPlayer(target))
		player = target->player;
	
	if (P_MobjIsPlayer(target))
		if (P_XGSVal(PGS_GAMESKILL) == sk_baby || P_XGSVal(PGS_PLHALFDAMAGE))
			damage >>= 1;		// take half damage in trainer mode
	
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
	        && !(target->flags & MF_NOCLIP) && !(inflictor->flags2 & MF2_NODMGTHRUST) && (!source || !P_MobjIsPlayer(source) || !(P_MobjIsPlayer(source) && (source->player->weaponinfo[source->player->readyweapon]->WeaponFlags & WF_NOTHRUST))))
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
		if (source && P_XGSVal(PGS_COROCKETZTHRUST) && (!P_XGSVal(PGS_COALLOWROCKETJUMPING) || !P_XGSVal(PGS_GAMEALLOWROCKETJUMP)))
		{
			fixed_t dist, z;
			
			if (source == target)	// rocket in yourself (suicide)
			{
				z = inflictor->z;
				dist = R_PointToDist2(inflictor->x, inflictor->y, target->x, target->y);
			}
			else
			{
				z = source->z;
				dist = R_PointToDist2(source->x, source->y, target->x, target->y);
			}
			
			ang = R_PointToAngle2(0, z, dist, target->z);
			
			ang >>= ANGLETOFINESHIFT;
			amomz = FixedMul(thrust, finesine[ang]);
		}
		else					//SoM: 2/28/2000: Added new function.
			if (P_XGSVal(PGS_COALLOWROCKETJUMPING) && P_XGSVal(PGS_GAMEALLOWROCKETJUMP))
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
	if (player && (P_MobjIsPlayer(target)) && (target->flags & MF_CORPSE) == 0)
	{
		// end of game hell hack
		if (target->subsector->sector->special == 11 && damage >= target->health)
		{
			damage = target->health - 1;
		}
		
		// Below certain threshold,
		// ignore damage in GOD mode, or with INVUL power.
		if (damage < 1000 && ((player->cheats & CF_GODMODE) || player->powers[pw_invulnerability]))
			return false;
		
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
		
		player->attacker = source;
		
		if (source && P_MobjIsPlayer(source))
			source->player->Attackee = target;
	}
	else
		takedamage = true;
	
	// GhostlyDeath <June 6, 2012> -- Team Damage?
	if (source && target)
		if (damage < 1000)	// Allow telefrags!
			if (!P_MobjDamageTeam(source, target, inflictor))
				return false;
	
	// Player Specific	
	if (player && (P_MobjIsPlayer(target)) && (target->flags & MF_CORPSE) == 0)
	{
		// added team play and teamdamage (view logboris at 13-8-98 to understand)
		if (P_XGSVal(PGS_CODISABLETEAMPLAY) ||	// support old demo version
		        P_XGSVal(PGS_GAMETEAMDAMAGE) || damage > 1000 ||	// telefrag
		        source == target || !source || !(P_MobjIsPlayer(target)) || !(P_MobjIsPlayer(source) && P_MobjIsPlayer(source)) || (P_GMIsDM() && (!P_GMIsTeam() || !P_MobjOnSameTeam(source->player->mo, player->mo))))
		{
			player->health -= damage;	// mirror mobj health here for Dave
			if (player->health < 0)
				player->health = 0;
			takedamage = true;
			
			player->damagecount += damage;	// add damage after armor / invuln
			
			if (player->damagecount > 100)
				player->damagecount = 100;	// teleport stomp does 10k points...
				
			//added:22-02-98: force feedback ??? electro-shock???
			if (player == &players[g_Splits[0].Console])
				I_Tactile(40, 10, 40 + (damage < 100 ? damage : 100) * 2);
		}
	}
	
	// GhostlyDeath <April 3, 2013> -- Damage count for monster players
	else if (P_GMIsCounter() && target->player)
	{
		// Damage is fraction of life
		saved = 0;	// in case of zero
		if (target->info->spawnhealth > 0)
			saved = FixedDiv(damage << FRACBITS, target->info->spawnhealth << FRACBITS);
		saved = FixedMul(saved, 100 << FRACBITS);
		saved >>= FRACBITS;
		
		// Cap to zero
		if (saved < 0)
			saved = 0;
		
		// Add red screen
		target->player->damagecount += saved;
		
		// Do not fade for a really long time
		if (target->player->damagecount > 100)
			target->player->damagecount = 100;
	}
	
	if (takedamage)
		if (target->RXFlags[1] & MFREXB_DONTTAKEDAMAGE)
		{
			if (damage >= 1000)
				takedamage = true;
			else
				takedamage = false;
		}
	
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
		if (P_XGSVal(PGS_GAMEONEHITKILLS))
		{
			// Set health to zero
			if (target->health > 0)
				target->health = 0;
			
			// Set player's health to zero also
			if (P_MobjIsPlayer(target))
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
		if (P_XGSVal(PGS_FUNNOTARGETPLAYER))
			if (P_MobjIsPlayer(source))
				return takedamage;
		
		// if not intent on another player,
		// chase after this one
		P_RefMobj(PMRT_TARGET, target, source);
		target->threshold = BASETHRESHOLD;
		if (target->state == states[target->info->spawnstate] && target->info->seestate != S_NULL)
			P_SetMobjState(target, target->info->seestate);
	}
	
	return takedamage;
}

