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
// DESCRIPTION: Bot Code

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "bot.h"
#include "sn.h"
#include "d_netcmd.h"
#include "bot_priv.h"
#include "console.h"
#include "g_state.h"
#include "z_zone.h"
#include "w_wad.h"
#include "m_argv.h"
#include "d_player.h"

/****************
*** FUNCTIONS ***
****************/

/* BOT_RequestPort() -- Requests controller port for bot */
bool_t BOT_RequestPort(BOT_t* const a_Bot)
{
	bool_t Trans;
	
	/* Transmission delay */
	Trans = (g_ProgramTic >= a_Bot->LastXMit);
	
	/* Up the delay and transmit port request */
	a_Bot->LastXMit = g_ProgramTic + TICRATE;
	a_Bot->Port = SN_RequestPort(a_Bot->ProcessID, Trans);
}

/* BOT_CommitAccount() -- Commits bot account */
void BOT_CommitAccount(BOT_t* const a_Bot)
{
	/* Clear commit flag */
}

/* BOT_BasicInit() -- Initializes the bot with basic information */
void BOT_BasicInit(BOT_t* const a_Bot)
{
	/* Now initialized */
	a_Bot->BasicInit = true;
	
	/* Set port as bot */
	a_Bot->Port->Bot = true;
	a_Bot->Port->StatFlags |= DTCJF_ISBOT;	// for scoreboard
	a_Bot->Port->Screen = -1;	// not on any screen
	a_Bot->Port->BotPtr = a_Bot;
	
	/* Setup Default Settings for Bot */
	
	
	// Change settings to that of a bot
	SN_PortSetting(a_Bot->Port, DSNPS_NAME, 0, "Bot", 0);
}

/* BOT_IndivTic() -- Ticker for individual bot */
void BOT_IndivTic(BOT_t* const a_Bot)
{
	ticcmd_t* TicCmd;
	BL_TicCmd_t* BL;
	player_t* Player;
	mobj_t* Mo;
	uint32_t SFlags;
	
	/* If bot has no port, try to obtain one */
	if (!a_Bot->Port)
		if (!BOT_RequestPort(a_Bot))
			return;
	
	/* Needs basic initialization */
	if (!a_Bot->BasicInit)
		BOT_BasicInit(a_Bot);
	
	/* In statis */
	if (a_Bot->Stasis)
	{
		// Try joining the game
		if (g_ProgramTic >= a_Bot->LastGameTryJoin)
		{
			a_Bot->LastGameTryJoin = g_ProgramTic + (TICRATE * 3);
			
			// Try to join
			SN_PortTryJoin(a_Bot->Port);
		}
		
		// Do not perform normal thinking
		return;
	}
	
	/* Determine VM Bot Info */
	a_Bot->VMBotInfo.Flags = 0;
	
	// Bot's player
	a_Bot->VMBotInfo.InternalPlayer = Player = (a_Bot->Port ? a_Bot->Port->Player : NULL);
	
	// Position of bot, if playing
	a_Bot->VMBotInfo.InternalMobj = Mo = (Player ? Player->mo : NULL);
	
	// Player is OK
	if (Player)
	{
		// Dead Bot?
		if (Player->health <= 0)
			a_Bot->VMBotInfo.Flags |= BLBIF_DEAD;
	}
	
	// Mobj is OK
	if (Mo)
	{
	}
	
	// Swap back flags
	a_Bot->VMBotInfo.Flags = LittleSwapUInt32(a_Bot->VMBotInfo.Flags);
	
	/* Run Virtual Machine */
	if (!MIPS_VMRun(&a_Bot->VM, a_Bot->Speed, g_CodeDebug))
	{
		// Unrecovered exception, destroy
		a_Bot->Stasis = true;
		SN_UnplugPort(a_Bot->Port);
		return;
	}
	
	/* Commit account settings? */
	if (a_Bot->VMAccount.Commit)
		BOT_CommitAccount(a_Bot);
	
	/* Handle Tic Commands */
	TicCmd = &a_Bot->Port->LocalBuf[0];
	a_Bot->Port->LocalAt = 1;
	BL = &a_Bot->VMTicCmd;
	
	// Clear old
	memset(TicCmd, 0, sizeof(*TicCmd));
	
	// Prevent Bot from turboing
	TicCmd->Std.forwardmove = LittleSwapInt32(BL->ForwardMove);
	TicCmd->Std.sidemove = LittleSwapInt32(BL->SideMove);
	
	if (TicCmd->Std.forwardmove > MAXRUNSPEED)
		TicCmd->Std.forwardmove = MAXRUNSPEED;
	else if (TicCmd->Std.forwardmove < -MAXRUNSPEED)
		TicCmd->Std.forwardmove = -MAXRUNSPEED;
	
	if (TicCmd->Std.sidemove > MAXRUNSPEED)
		TicCmd->Std.sidemove = MAXRUNSPEED;
	else if (TicCmd->Std.sidemove < -MAXRUNSPEED)
		TicCmd->Std.sidemove = -MAXRUNSPEED;
	
	// Most values are copied as is
	TicCmd->Std.angleturn = LittleSwapUInt32(BL->LookAngle) >> UINT32_C(16);
	TicCmd->Std.aiming = LittleSwapUInt32(BL->Aiming) >> UINT32_C(16);
	TicCmd->Std.StatFlags = LittleSwapUInt32(BL->StatFlags);
	TicCmd->Std.FlySwim = LittleSwapInt32(BL->FlySwim);
	
	// Load swapped buttons (for big endian)
	SFlags = LittleSwapUInt32(BL->Buttons);
	
	// Handle Buttons
	if (SFlags & BLT_ATTACK)
		TicCmd->Std.buttons |= BT_ATTACK;
	if (SFlags & BLT_USE)
		TicCmd->Std.buttons |= BT_USE;
	if (SFlags & BLT_JUMP)
	{
		TicCmd->Std.buttons |= BT_JUMP;
		SFlags &= ~BLT_JUMP;
	}
	if (SFlags & BLT_SUICIDE)
	{
		TicCmd->Std.buttons |= BT_SUICIDE;
		SFlags &= ~BLT_SUICIDE;
	}
	if (SFlags & BLT_CHANGE)
	{
		TicCmd->Std.buttons |= BT_CHANGE;
		strncpy(TicCmd->Std.XSNewWeapon, BL->Weapon, MAXTCWEAPNAME);
		TicCmd->Std.XSNewWeapon[MAXTCWEAPNAME - 1] = 0;
		memset(BL->Weapon, 0, sizeof(BL->Weapon));
		SFlags &= ~BLT_CHANGE;
	}
	if (SFlags & BLT_SPECTATE)
	{
		SN_RemovePlayer(a_Bot->Port->Player - players);
		SFlags &= ~BLT_SPECTATE;
	}
	
	// Swap any changes back to the bot
	BL->Buttons = LittleSwapUInt32(SFlags);
}

