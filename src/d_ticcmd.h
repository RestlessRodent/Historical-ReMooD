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
// DESCRIPTION: System specific interface stuff.

#ifndef __D_TICCMD__
#define __D_TICCMD__

#include "m_fixed.h"
#include "doomtype.h"

//
// Button/action code definitions.
//

//added:16-02-98: bit of value 64 doesnt seem to be used,
//                now its used to jump

typedef enum
{
	// Press "Fire".
	BT_ATTACK = 1,
	// Use button, to open doors, activate switches.
	BT_USE = 2,
	
	// Flag, weapon change pending.
	// If true, the next 3 bits hold weapon num.
	BT_CHANGE = 4,
	
	// Jump button.
	BT_JUMP = 8,
	
	// Suicide
	BT_SUICIDE = 16,
	
	// The 3bit weapon mask and shift, convenience.
	BT_WEAPONMASK = (32 + 64 + 128 + 256),
	BT_WEAPONSHIFT = 5,
	BT_EXTRAWEAPON = 512,
	BT_SLOTMASK = (1024 + 2048 + 4096 + 8192),
	BT_SLOTSHIFT = 11,
} buttoncode_t;

/* ExtButtonCodes_t -- Extended button codes */
typedef enum ExtButtonCodes_e
{
	BTX_FLYLAND			= UINT32_C(0x00000001),	// Fly Up
	BTX_PANLOOK			= UINT32_C(0x00000002),	// Panning Loop
} ExtButtonCodes_t;

// The data sampled per tick (single player)
// and transmitted to other peers (multiplayer).
// Mainly movements/button commands per game tick,
// plus a checksum for internal state consistency.

typedef enum
{
	TICCMD_NOTHING,								// Do nothing
	TICCMD_INVLEFT,								// Move Inventory Left
	TICCMD_INVRIGHT,							// Move Inventory Right
	TICCMD_INVUSE,								// Use Inventory
};

#define TICCMD_INVACTSHIFT	UINT32_C(0)			// Action Shift
#define TICCMD_INVACTMASK	UINT32_C(0x3)		// Action Mask

#define MAXTCWEAPNAME						32	// Max length for weapon name
#define MAXTCDATABUF					384		// Data Buffer size

/* D_TicControl_t -- Tic Control */
typedef struct D_TicControl_s
{
	uint8_t Type;								// Type of Command
	
	// Timing
	uint64_t ProgramTic;						// Program Tic
	uint64_t GameTic;							// Last game tic
	uint16_t Ping;								// Players' Ping
} D_TicControl_t;

typedef union
{
	D_TicControl_t Ctrl;						// Control
	
	struct
	{
		D_TicControl_t Ctrl;						// Control
		uint16_t Player;						// Player it is meant for
		
		int8_t forwardmove;						// *2048 for move
		int8_t sidemove;						// *2048 for move
		int16_t angleturn;						// <<16 for angle delta
		// SAVED AS A BYTE into demos
		uint16_t aiming;						//added:16-02-98:mouse aiming
		uint16_t buttons;
		uint8_t artifact;						// For Heretic
	
		// Extended tic command stuff
		uint8_t XSNewWeapon[MAXTCWEAPNAME];		// New weapon (string based)
		//uint8_t XNewWeapon;					// New weapon to switch to
		int16_t BaseAngleTurn;					// Base angle turning
		int16_t BaseAiming;						// Base Aiming
		uint8_t InventoryBits;					// Inventory Control
		uint8_t ResetAim;						// Reset Aim
		
		uint32_t StatFlags;						// Player Status Flags
		uint32_t ExButtons;						// More Buttons
		int16_t FlySwim;						// Fly/Swim
		
		uint16_t DataSize;						// Size of Data
		uint8_t DataBuf[MAXTCDATABUF];			// Data Buffer
	} Std;
	
	struct
	{
		D_TicControl_t Ctrl;						// Control
		
		uint16_t DataSize;						// Size of Data
		uint8_t DataBuf[MAXTCDATABUF];			// Data Buffer
	} Ext;
} ticcmd_t;

void D_TicCmdFillWeapon(ticcmd_t* const a_Target, const int32_t a_ID);

#define MAXTCCBUFSIZE					32		// Max buffer size in commands

enum
{
	DTCT_NULL,									// NULL
	DTCT_JOIN,									// Join Sub Command
	DTCT_MAPCHANGE,								// Change the map
	DTCT_GAMEVAR,								// Game Variable
	DTCT_PART,									// Part Game
	DTCT_ADDSPEC,								// Add Spectator
	
	// Extended
	DTCT_XKICKPLAYER,							// Kick Player
	DTCT_XADDPLAYER,							// Adds a new player
	DTCT_XJOINPLAYER,							// Joins player
	
	NUMDTCT
};

/* D_TCJoinFlags_t -- Join Flags */
typedef enum D_TCJoinFlags_e
{
	DTCJF_ISBOT						= 0x0001,	// Is a Bot
	DTCJF_MONSTERTEAM				= 0x0002,	// On monster team
} D_TCJoinFlags_t;

/* D_DiffBits_t -- Diff bits */
typedef enum D_DiffBits_e
{
	DDB_FORWARD						= 0x0001,	// Forward Changes
	DDB_SIDE						= 0x0002,	// Side Changes
	DDB_AIMING						= 0x0004,	// Aiming Changes
	DDB_BUTTONS						= 0x0008,	// Button Changes
	DDB_WEAPON						= 0x0010,	// Weapon Changes
	DDB_BAT							= 0x0020,	// Base turn angle changes
	DDB_BAM							= 0x0040,	// Base aiming changes
	DDB_RESETAIM					= 0x0080,	// Aim is reset
	DDB_ANGLE						= 0x0100,	// Angle changes
	DDB_INVENTORY					= 0x0200,	// Inventory Control
	DDB_STATFLAGS					= 0x0400,	// Status Flags
	
	DDB_PLAYER						= 0x0800,	// Forward Changes
	DDB_ARTIFACT					= 0x1000,	// Artifact Changes
	DDB_EXBUTTONS					= 0x2000,	// More Buttons
	DDB_FLYSWIM						= 0x4000,	// Flying/Swimming
} D_DiffBits_t;

extern const int32_t c_TCDataSize[NUMDTCT];

#endif

