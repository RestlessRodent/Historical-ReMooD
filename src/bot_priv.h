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
// Copyright (C) 2013-2013 GhostlyDeath <ghostlydeath@remood.org>
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

