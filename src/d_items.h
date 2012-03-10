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
// DESCRIPTION: Items: key cards, artifacts, weapon, ammunition.

#ifndef __D_ITEMS__
#define __D_ITEMS__

#include "doomdef.h"

// ==================================
// Difficulty/skill settings/filters.
// ==================================

// Skill flags.
#define MTF_EASY                1
#define MTF_NORMAL              2
#define MTF_HARD                4

// Deaf monsters/do not react to sound.
#define MTF_AMBUSH              8

//Hurdler: special option to tell the things has been spawned by an FS
#define MTF_FS_SPAWNED    0x1000

//
// Key cards.
//
typedef enum
{
	it_bluecard = 1,
	it_yellowcard = 2,
	it_redcard = 4,
	it_blueskull = 8,
	it_yellowskull = 16,
	it_redskull = 32,
	
	it_allcards = it_bluecard | it_yellowcard | it_redcard,
	it_allskulls = it_blueskull | it_yellowskull | it_redskull,
	it_allkeys = it_allcards | it_allskulls,
	
	it_gallkeys = it_allkeys,
	
	NUMCARDS = 9,
} card_t;

#define NUMINVENTORYSLOTS  14
#define MAXARTECONT        16
typedef struct
{
	uint8_t type;
	uint8_t count;
} inventory_t;

// Power up artifacts.
typedef enum
{
	pw_invulnerability,
	pw_strength,
	pw_invisibility,
	pw_ironfeet,
	pw_allmap,
	pw_infrared,
	
	NUMPOWERS
} powertype_t;

//
// Power up durations,
//  how many seconds till expiration,
//  assuming TICRATE is 35 ticks/second.
//
typedef enum
{
	INVULNTICS = (30 * TICRATE),
	INVISTICS = (60 * TICRATE),
	INFRATICS = (120 * TICRATE),
	IRONTICS = (60 * TICRATE)
} powerduration_t;

// The defined weapons,
//  including a marker indicating
//  user has not changed weapon.
typedef enum
{
	wp_fist,
	wp_pistol,
	wp_shotgun,
	wp_chaingun,
	wp_missile,
	wp_plasma,
	wp_bfg,
	wp_chainsaw,
	wp_supershotgun,
	
	NUMWEAPONS,
	
	// No pending weapon change.
	wp_nochange
} weapontype_t;

// Ammunition types defined.
typedef enum
{
	am_clip,					// Pistol / chaingun ammo.
	am_shell,					// Shotgun / double barreled shotgun.
	am_cell,					// Plasma rifle, BFG.
	am_misl,					// Missile launcher.
	
	NUMAMMO,
	am_noammo					// Unlimited for chainsaw / fist.
} ammotype_t;

/* WeaponFlags_t -- Flags for weapons */
typedef enum WeaponFlags_e
{
	// Game bases
	WF_ISDOOM					= 0x00000001,	// Weapon appears in Doom
	WF_ISHERETIC				= 0x00000002,	// Weapon appears in Heretic
	WF_ISHEXEN					= 0x00000004,	// Weapon appears in Hexen
	WF_ISSTRIFE					= 0x00000008,	// Weapon appears in Strife
	
	// Visibility Status
	WF_NOTSHAREWARE				= 0x00000010,	// Does not appear in shareware
	WF_INCOMMERCIAL				= 0x00000020,	// Appears in commercial mode
	WF_INREGISTERED				= 0x00000040,	// Appears in registered mode
	
	// Other
	WF_BERSERKTOGGLE			= 0x00000080,	// Only accept least weapon when berserk
} WeaponFlags_t;

// Weapon info: sprite frames, ammunition use.
typedef struct
{
	ammotype_t ammo;
	int ammopershoot;
	int upstate;
	int downstate;
	int readystate;
	int atkstate;
	int holdatkstate;
	int flashstate;
	
	// ReMooD Extended
	char* DropWeaponClass;		// Thing to "drop" when a player dies
	char* NiceName;				// Name of weapon (obit)
	char* ClassName;			// Weapon class Name
	int32_t SwitchOrder;		// Weapon switch order
	int8_t SlotNum;				// Weapon slot number
	uint32_t WeaponFlags;		// Flags for weapon
	int32_t GetAmmo;			// Amount of ammo to pick up for this weapon
} weaponinfo_t;

extern weaponinfo_t wpnlev1info[NUMWEAPONS];

#if 1
extern weaponinfo_t* wpnlev2info;
#else
extern weaponinfo_t wpnlev2info[NUMWEAPONS];
#endif

// GhostlyDeath <March 10, 2012> -- Ammo Information

/* ammoinfo_t -- Hold ammo information */
typedef struct ammoinfo_s
{
	char* ClassName;							// Class name
	int32_t ClipAmmo;							// Ammo in clip
	int32_t MaxAmmo;							// Max ammo held
} ammoinfo_t;

extern ammoinfo_t ammoinfo[NUMAMMO];

#endif
