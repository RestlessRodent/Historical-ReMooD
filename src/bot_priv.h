// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Private Bot Code Header

#ifndef __BOT_PRIV__H__
#define __BOT_PRIV__H__

/*****************************************************************************/

/***************
*** INCLUDES ***
***************/

#include "bot.h"
#include "mips.h"
#include "w_wad.h"
#include "d_player.h"
#include "p_mobj.h"

#define __REMOOD_INCLUDED
#include "bot_lib.h"

/****************
*** CONSTANTS ***
****************/

#define LSi32(x) LittleSwapInt32((x))
#define LSi64(x) LittleSwapInt64((x))
#define LSu32(x) LittleSwapUInt32((x))
#define LSu64(x) LittleSwapUInt64((x))

/*****************
*** STRUCTURES ***
*****************/

/* BOT_t -- Bot */
typedef struct BOT_s
{
	SN_Port_t* Port;							// Port which bot controls
	uint32_t ProcessID;							// Process ID of bot
	tic_t LastXMit;								// Last port transmit
	bool_t Stasis;								// In statis mode
	bool_t BasicInit;							// Basic Initialization
	tic_t LastGameTryJoin;						// Time last tried to join game
	int8_t Screen;								// Screen bot is on
	
	/* MIPS Stuff */
	const WL_WADEntry_t* CodeEnt;				// Code Entry
	void* CodeMap;								// Code mapping
	uint32_t CodeLen;							// Length of binary code
	MIPS_VM_t VM;								// MIPS VM
	void* Stack;								// Stack Data
	uint32_t StackLen;							// Stack Length
	uint32_t StackAddr;							// Stack Address
	uint32_t Speed;								// Execution Speed of Bot
	
	/* VM Specials */
	BL_TicCmd_t VMTicCmd;						// Virtual Machine Tic Command
	BL_BotAccount_t VMAccount;					// Account for bot
	BL_BotInfo_t VMBotInfo;						// Bot Information
	
	/* EBAPI */
	int32_t SleepCount;							// Sleep Count
	player_t* Player;							// Player bot controls
	mobj_t* Mo;									// Object bot controls
	BInfo_t BInfo;								// Bot Info
} BOT_t;

/**************
*** GLOBALS ***
**************/

extern bool_t l_BotDebug, g_CodeDebug;

extern BReal_t g_BotRT;

/****************
*** FUNCTIONS ***
****************/

/*** BOT_CORE.C ***/
BOT_t* BOT_ByProcessID(const uint32_t a_ProcessID);
BOT_t* BOT_ByPort(SN_Port_t* const a_Port);

/*** BOT_TICK.C ***/
void BOT_EBFillRealTime(void);
void BOT_IndivTic(BOT_t* const a_Bot);

/*** BOT_VM.C ***/
void BOT_RegisterVMJunk(BOT_t* const a_Bot);

/*****************************************************************************/

#endif /* __BOT_PRIV_H__ */

