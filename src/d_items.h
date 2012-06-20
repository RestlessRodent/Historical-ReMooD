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
#include "m_fixed.h"
#include "info.h"

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

typedef int32_t weapontype_t;
typedef int32_t ammotype_t;

#define wp_nochange				-1
#define am_noammo				-1
#define am_all					-2

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
	WF_SWITCHFROMNOAMMO			= 0x00000100,	// When player has 0 ammo, switch away!
	WF_STARTINGWEAPON			= 0x00000200,	// Start with this gun
	WF_NOAUTOFIRE				= 0x00000400,	// No automatic fire
	WF_NOTHRUST					= 0x00000800,	// No thrusting the enemy
	
	WF_INEXTENDED				= 0x00001000,	// Appears in extended mode
	WF_NOBLEEDTARGET			= 0x00002000,	// Do not bleed target
	WF_SUPERWEAPON				= 0x00004000,	// Is a Super Weapon
	WF_NONOISEALERT				= 0x00008000,	// Does not alert to noise
} WeaponFlags_t;

// Weapon info: sprite frames, ammunition use.
typedef struct
{
	ammotype_t ammo;
	statenum_t ammopershoot;
	statenum_t upstate;
	statenum_t downstate;
	statenum_t readystate;
	statenum_t atkstate;
	statenum_t holdatkstate;
	statenum_t flashstate;
	
	// ReMooD Extended
	int32_t DEHId;								// DeHackEd ID
	char* DropWeaponClass;						// Thing to "drop" when a player dies
	char* NiceName;								// Name of weapon (obit)
	char* ClassName;							// Weapon class Name
	int32_t SwitchOrder;						// Weapon switch order
	int8_t SlotNum;								// Weapon slot number
	uint32_t WeaponFlags;						// Flags for weapon
	int32_t GetAmmo;							// Amount of ammo to pick up for this weapon
	int32_t NoAmmoOrder;						// No Ammo Order
	fixed_t PSpriteSY;							// PSprite offset
	char* SBOGraphic;							// SBO Graphic
	char* AmmoClass;							// Name of ammo to use
	char* BringUpSound;							// Sound to play when brung up
	char* IdleNoise;							// Noise when idling (chainsaw)
	uint32_t WeaponID;							// Unique Weapon ID
	uint32_t RefStates[NUMPWEAPONSTATEGROUPS];	// Reference States
	char* ReplacePuffType;						// Replacement puff type (rather than default)
	char* ReplaceFireSound;						// Replacement Fire Sound
	char* GenericProjectile;					// Generic Projectile
	char* TracerSplat;							// Splat when tracing
	INFO_BotObjMetric_t BotMetric;				// Bot Metric
	
	// State References
	statenum_t* FlashStates;					// Weapon flash states
	size_t NumFlashStates;						// Number of flash states
} weaponinfo_t;

extern weaponinfo_t** wpnlev1info;
extern weaponinfo_t** wpnlev2info;
extern size_t NUMWEAPONS;

// GhostlyDeath <March 10, 2012> -- Ammo Information

/* AmmoFlags_t -- Ammunition Flags */
typedef enum AmmoFlags_e
{
	AF_INFINITE						= 0x0001,	// Infinite Ammo
} AmmoFlags_t;

/* ammoinfo_t -- Hold ammo information */
typedef struct ammoinfo_s
{
	char* ClassName;							// Class name
	int32_t ClipAmmo;							// Ammo in clip
	int32_t MaxAmmo;							// Max ammo held
	uint32_t Flags;								// Ammo Flags
	int32_t StartingAmmo;						// Starting Ammo
} ammoinfo_t;

extern ammoinfo_t** ammoinfo;
extern size_t NUMAMMO;

weapontype_t INFO_GetWeaponByName(const char* const a_Name);
ammotype_t INFO_GetAmmoByName(const char* const a_Name);

#endif

