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
	BT_ATTACK = 0x1,
	// Use button, to open doors, activate switches.
	BT_USE = 0x2,
	
	// Flag, weapon change pending.
	// If true, the next 3 bits hold weapon num.
	BT_CHANGE = 0x4,
	
	// Jump button.
	BT_JUMP = 0x8,
	
	// Suicide
	BT_SUICIDE = 0x10,
	
	// The 3bit weapon mask and shift, convenience.
	BT_WEAPONMASK = 0x1E0,
	BT_WEAPONSHIFT = 5,
	
	BT_EXTRAWEAPON = 0x200,
	
	BT_SLOTMASK = 0x3C00,
	BT_SLOTSHIFT = 11,
	
	BT_FLYLAND			= UINT32_C(0x00010000),	// Fly Up
	BT_PANLOOK			= UINT32_C(0x00020000),	// Panning Loop
	BT_RESETAIM			= UINT32_C(0x00040000),	// Reset aim
} buttoncode_t;

// The data sampled per tick (single player)
// and transmitted to other peers (multiplayer).
// Mainly movements/button commands per game tick,
// plus a checksum for internal state consistency.

enum
{
	TICCMD_NOTHING,								// Do nothing
	TICCMD_INVLEFT,								// Move Inventory Left
	TICCMD_INVRIGHT,							// Move Inventory Right
	TICCMD_INVUSE,								// Use Inventory
};

#define TICCMD_INVACTSHIFT	UINT32_C(0)			// Action Shift
#define TICCMD_INVACTMASK	UINT32_C(0x3)		// Action Mask

#define MAXTCSTRINGCAT					48		// Max length for string cat
#define MAXTCWEAPNAME					32		// Max length for weapon name
#define MAXTCDATABUF					384		// Data Buffer size

#define TICPINGSIGNALSHIFT UINT16_C(14)
#define TICPINGSIGNALMASK UINT16_C(0xC000)
#define TICPINGAMOUNTMASK UINT16_C(0x3FFF)

/* D_TicControl_t -- Tic Control */
typedef struct D_TicControl_s
{
	uint8_t Type;								// Type of Command
	uint16_t Ping;								// Players' Ping
} D_TicControl_t;

typedef union
{
	D_TicControl_t Ctrl;						// Control
	
	struct
	{
		D_TicControl_t Ctrl;					// Control
		
		int8_t forwardmove;						// *2048 for move
		int8_t sidemove;						// *2048 for move
		int16_t angleturn;						// <<16 for angle delta
		// SAVED AS A BYTE into demos
		uint16_t aiming;						//added:16-02-98:mouse aiming
		uint32_t buttons;
		uint8_t artifact;						// For Heretic
	
		// Extended tic command stuff
		uint8_t XSNewWeapon[MAXTCWEAPNAME];		// New weapon (string based)
		//uint8_t XNewWeapon;					// New weapon to switch to
		int16_t BaseAngleTurn;					// Base angle turning
		int16_t BaseAiming;						// Base Aiming
		uint8_t InventoryBits;					// Inventory Control
		
		uint32_t StatFlags;						// Player Status Flags
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
	
	// Simple Networking
	DTCT_SNJOINPLAYER,							// Player Joins
	DTCT_MAPCHANGE,								// Change the map
	DTCT_GAMEVAR,								// Game Variable
	DTCT_XCHANGEMONSTERTEAM,					// Changes Monster Team
	DTCT_XMORPHPLAYER,							// Morph Player
	DTCT_SNQUITREASON,							// Reason for quitting
	DTCT_SNCLEANUPHOST,							// Tell clients to cleanup host
	DTCT_SNJOINHOST,							// Host Joins
	DTCT_SNPARTPLAYER,							// Player parts
	DTCT_SNJOINPORT,							// Port is joined
	DTCT_SNCHATFRAG,							// Chat Fragment
	DTCT_SNPORTSETTING,							// Setting of port
	
	NUMDTCT
};

/* D_TCJoinFlags_t -- Join Flags */
typedef enum D_TCJoinFlags_e
{
	DTCJF_ISBOT						= 0x0001,	// Is a Bot
	DTCJF_MONSTERTEAM				= 0x0002,	// On monster team
} D_TCJoinFlags_t;

/* D_TCStatFlags_t -- Status Flags */
typedef enum D_TCStatFlags_e
{
	DTSF_LOCALSTICK		= UINT32_C(0x00000001),	// Local client is sticking
} D_TCStatFlags_t;

/* D_DiffBits_t -- Diff bits */
typedef enum D_DiffBits_e
{
	/* Important <= 128 */
	DDB_FORWARD						= 0x0001,	// Forward Changes
	DDB_SIDE						= 0x0002,	// Side Changes
	DDB_AIMING						= 0x0004,	// Aiming Changes
	DDB_BUTTONS						= 0x0008,	// Button Changes
	DDB_WEAPON						= 0x0010,	// Weapon Changes
	DDB_ANGLE						= 0x0020,	// Angle changes
	DDB_STATFLAGS					= 0x0040,	// Status Flags
	
	/* Less Important > 128 */
	DDB_BAT							= 0x0080,	// Base turn angle changes
	DDB_BAM							= 0x0100,	// Base aiming changes
	DDB_INVENTORY					= 0x0200,	// Inventory Control
	DDB_ARTIFACT					= 0x0400,	// Artifact Changes
	DDB_FLYSWIM						= 0x0800,	// Flying/Swimming
	DDB_UNUSED1000					= 0x1000,	// Not used
	DDB_UNUSED2000					= 0x2000,	// Not used
	DDB_UNUSED4000					= 0x4000,	// Not used
	
	/* DO NOT USE */
	DDB_UNUSED8000					= 0x8000,	// Unused [ DO NOT USE! ]
} D_DiffBits_t;

extern const int32_t c_TCDataSize[NUMDTCT];


/* D_SNTicBufVersion_t -- TicBuf version number */
// This is for net compat, but mostly for demos!
typedef enum D_SNTicBufVersion_s
{
	DXNTBV_ILLEGALVERSION,						// Illegal Version	
	
	DXNTBV_VER20130327,							// 2013/03/27
	DXNTBV_VER20130731,							// 2013/07/31
	
	DXNTBV_LATEST = DXNTBV_VER20130731,			// Lastest Version
} D_SNTicBufVersion_t;


/* D_SNSyncCodeInfo_t -- Sync Code Data (sync debugging) */
typedef struct D_SNSyncCodeInfo_s
{
	uint32_t Code;								// Generated Code
} D_SNSyncCodeInfo_t;

/* D_SNTicBuf_t -- Tic Command Buffer */
typedef struct D_SNTicBuf_s
{
	tic_t GameTic;								// Gametic to run at
	uint32_t SyncCode;							// Synchronization Code
	bool_t GotTic;								// Got tic
	ticcmd_t Tics[MAXPLAYERS + 1];				// Tic Commands
	uint32_t PIGRevMask;						// Player in reverse game mask
} D_SNTicBuf_t;

#endif

