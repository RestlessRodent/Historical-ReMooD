// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.org/
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2011 GhostlyDeath (ghostlydeath@gmail.com)
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
	byte type;
	byte count;
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

} weaponinfo_t;

extern weaponinfo_t wpnlev1info[NUMWEAPONS];
extern weaponinfo_t wpnlev2info[NUMWEAPONS];

#endif

