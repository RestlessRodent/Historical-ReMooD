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
	
	// The 3bit weapon mask and shift, convenience.
	BT_WEAPONMASK = (32 + 64 + 128 + 256),
	BT_WEAPONSHIFT = 5,
	BT_EXTRAWEAPON = 512,
	BT_SLOTMASK = (1024 + 2048 + 4096 + 8192),
	BT_SLOTSHIFT = 11,
} buttoncode_t;

// The data sampled per tick (single player)
// and transmitted to other peers (multiplayer).
// Mainly movements/button commands per game tick,
// plus a checksum for internal state consistency.

// bits in angleturn
#define TICCMD_RECEIVED 1
#define TICCMD_XY       2
#define BT_FLYDOWN      4

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
#define MAXTCDATABUF					1024	// Data Buffer size

typedef union
{
	uint8_t Type;								// Command Type
	
	struct
	{
		uint8_t Type;							// Command Type
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
		bool_t ResetAim;						// Reset Aim
		
		uint16_t DataSize;						// Size of Data
		uint8_t DataBuf[MAXTCDATABUF];			// Data Buffer
	} Std;
	
	struct
	{
		uint8_t Type;							// Command Type
		
		uint16_t DataSize;						// Size of Data
		uint8_t DataBuf[MAXTCDATABUF];			// Data Buffer
	} Ext;
} ticcmd_t;

void D_TicCmdFillWeapon(ticcmd_t* const a_Target, const int32_t a_ID);

enum
{
	DTCT_NULL,									// NULL
	DTCT_JOIN,									// Join Sub Command
	DTCT_MAPCHANGE,								// Change the map
	
	NUMDTCT
};

extern const int32_t c_TCDataSize[NUMDTCT];

#endif

