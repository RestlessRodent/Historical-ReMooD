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
#include "tables.h"

/****************
*** CONSTANTS ***
****************/

// Stack Bounds
#define MINSTACKSIZE	4096
#define DEFSTACKSIZE	16384
#define MAXSTACKSIZE	65536

// Stack Location
#define DEFSTACKADDR	UINT32_C(0x70000000)

/*************
*** LOCALS ***
*************/

static BOT_t** l_Bots;							// Local Bots
static int32_t l_NumBots;						// Number of bots
static bool_t l_BotDebug;						// Debugging

static BL_PortInfo_t l_PortInfo;				// Port Info

/****************
*** FUNCTIONS ***
****************/

/* BOT_BotCommand() -- Bot management */
static int BOT_BotCommand(const uint32_t a_ArgC, const char** const a_ArgV)
{
	/* Add bot */
	if (!strcasecmp(a_ArgV[0], "addbot"))
	{
		// Add bot
		BOT_Add(a_ArgC - 1, a_ArgV + 1);
		
		// Success!
		return 0;
	}	
	
	/* Failed */
	return 1;
}

/* BOT_Init() -- Initialize Bots */
void BOT_Init(void)
{
	/* Add console commands */
	CONL_AddCommand("addbot", BOT_BotCommand);
	
	/* Debugging Bots? */
	if (M_CheckParm("-devbot") || M_CheckParm("-botdev"))
		l_BotDebug = true;
	
	/* Initialize port information */
	l_PortInfo.VendorID = LittleSwapUInt32(BLVC_REMOOD);
	l_PortInfo.Version = LittleSwapUInt32(VERSION);
	strncpy(l_PortInfo.Name, "ReMooD", MAXPORTINFOFIELDLEN);
	strncpy(l_PortInfo.VerString, REMOOD_FULLVERSIONSTRING, MAXPORTINFOFIELDLEN);
}

/* BOT_IndivTic() -- Ticker for individual bot */
static inline void BOT_IndivTic(BOT_t* const a_Bot)
{
	bool_t Trans;
	ticcmd_t* TicCmd;
	BL_TicCmd_t* BL;
	
	/* If bot has no port, try to obtain one */
	if (!a_Bot->Port)
	{
		// Transmission delay
		Trans = (g_ProgramTic >= a_Bot->LastXMit);
		
		// Up the delay and transmit port request
		a_Bot->LastXMit = g_ProgramTic + TICRATE;
		a_Bot->Port = SN_RequestPort(a_Bot->ProcessID, Trans);
		
		// If still no port, stop
		if (!a_Bot->Port)
			return;
	}
	
	/* Needs basic initialization */
	if (!a_Bot->BasicInit)
	{
		a_Bot->BasicInit = true;
		
		// Set port as bot
		a_Bot->Port->Bot = true;
		a_Bot->Port->StatFlags |= DTCJF_ISBOT;	// for scoreboard
		a_Bot->Port->Screen = -1;	// not on any screen
		a_Bot->Port->BotPtr = a_Bot;
		
		// Change settings to that of a bot
		SN_PortSetting(a_Bot->Port, DSNPS_NAME, 0, "Bot", 0);
	}
	
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
	
	/* Run Virtual Machine */
	if (!MIPS_VMRun(&a_Bot->VM, 1, l_BotDebug))
	{
		// Unrecovered exception, destroy
		a_Bot->Stasis = true;
		SN_UnplugPort(a_Bot->Port);
	}
	
	/* Handle Tic Commands */
	TicCmd = &a_Bot->Port->LocalBuf[0];
	a_Bot->Port->LocalAt = 1;
	BL = &a_Bot->VMTicCmd;
	
	// Clear old
	memset(TicCmd, 0, sizeof(*TicCmd));
	
	// Most values are copied as is
	TicCmd->Std.forwardmove = BL->ForwardMove;
	TicCmd->Std.sidemove = BL->SideMove;
	TicCmd->Std.angleturn = BL->LookAngle >> UINT32_C(16);
	TicCmd->Std.aiming = BL->Aiming >> UINT32_C(16);
	TicCmd->Std.StatFlags = BL->StatFlags;
	TicCmd->Std.FlySwim = BL->FlySwim;
	
	// Handle Buttons
	if (BL->Buttons & BLT_ATTACK)
		TicCmd->Std.buttons |= BT_ATTACK;
	if (BL->Buttons & BLT_USE)
		TicCmd->Std.buttons |= BT_USE;
	if (BL->Buttons & BLT_JUMP)
	{
		TicCmd->Std.buttons |= BT_JUMP;
		BL->Buttons &= ~BLT_JUMP;
	}
	if (BL->Buttons & BLT_SUICIDE)
	{
		TicCmd->Std.buttons |= BT_SUICIDE;
		BL->Buttons &= ~BLT_SUICIDE;
	}
	if (BL->Buttons & BLT_CHANGE)
	{
		TicCmd->Std.buttons |= BT_CHANGE;
		strncpy(TicCmd->Std.XSNewWeapon, BL->Weapon, MAXTCWEAPNAME);
		TicCmd->Std.XSNewWeapon[MAXTCWEAPNAME - 1] = 0;
		memset(BL->Weapon, 0, sizeof(BL->Weapon));
		BL->Buttons &= ~BLT_CHANGE;
	}
	if (BL->Buttons & BLT_SPECTATE)
	{
		SN_RemovePlayer(a_Bot->Port->Player - players);
		BL->Buttons &= ~BLT_SPECTATE;
	}
}

/* BOT_Ticker() -- Bot ticker */
void BOT_Ticker(void)
{
	int32_t i;
	BOT_t* Bot;
	
	/* Go through each bot */
	for (i = 0; i < l_NumBots; i++)
		if ((Bot = l_Bots[i]))
			BOT_IndivTic(Bot);
}

/* BOT_Add() -- Adds bot via command line settings */
void BOT_Add(const int32_t a_ArgC, const char** const a_ArgV)
{
#define BUFSIZE 32
	char Buf[BUFSIZE];
	uint32_t ID;
	BOT_t* Bot;
	int32_t i, j;
	const char* e, *c;
	const WL_WADEntry_t* Ent;
	
	/* Do not add any bots, if not server */
	if (!SN_IsServer() || demoplayback)
		return;
	
	/* Create random process ID for bot */
	do
	{
		ID = UINT32_C(0xBEEF0000) | (D_CMakePureRandom() & UINT32_C(0xFFFF));
	} while (!ID || BOT_ByProcessID(ID) || SN_PortByID(ID) || SN_HostByID(ID));
	
	/* Setup new bot structure */
	Bot = Z_Malloc(sizeof(*Bot), PU_STATIC, NULL);
	
	Bot->ProcessID = ID;
	Bot->Stasis = true;
	Bot->StackLen = DEFSTACKSIZE;
	Bot->StackAddr = DEFSTACKADDR;
	
	/* Process arguments */
	for (i = 0; i < a_ArgC; i++)
	{
		// Find equal sign in string
		if (!(e = strchr(a_ArgV[i], '=')))
			continue;	// not found so ignore
		
		// Copy characters up until e to the buffer
		memset(Buf, 0, sizeof(Buf));
		for (c = a_ArgV[i], j = 0; c != e; c++)
			if (j < BUFSIZE - 1)
				Buf[j++] = *c;
		
		// Move e up to displace equals
		e++;
		
		// Handle argument based on buffer
			// Code mapping
		if (!strcasecmp("code", Buf))
		{
			// Clear existing mapping, if any
			if (Bot->CodeMap)
				WL_UnMapEntry(Bot->CodeMap);
			Bot->CodeMap = NULL;
			
			// Attempt locating the desired bot binary
			if ((Ent = WL_FindEntry(NULL, 0, e)))
				if ((Bot->CodeMap = WL_MapEntry(Ent)))
				{
					Bot->CodeEnt = Ent;
					Bot->CodeLen = Ent->Size;
				}
		}
			// Stack Size
		else if (!strcasecmp("stacksize", Buf))
		{
			Bot->StackLen = C_strtou32(e, NULL, 0);
			
			if (Bot->StackLen < MINSTACKSIZE)
				Bot->StackLen = MINSTACKSIZE;
			else if (Bot->StackLen > MAXSTACKSIZE)
				Bot->StackLen = MAXSTACKSIZE;
		}
			
			// Stack Address
		else if (!strcasecmp("stackaddr", Buf))
			Bot->StackAddr = C_strtou32(e, NULL, 0);
	}
	
	/* Initialize execution core */
	if (!Bot->CodeMap)
		CONL_PrintF("Bot has no attached binary code.\n");
	else
	{
		// Place binary code at start address
		MIPS_VMAddMap(&Bot->VM, Bot->CodeMap, UINT32_C(0x8000), Bot->CodeLen, MIPS_MFR | MIPS_MFX);
		Bot->VM.CPU.pc = UINT32_C(0x8000);
		Bot->VM.CPU.r[31] = Bot->VM.CPU.pc;	// Return to start?
	}
	
	/* Initialize communication layers with VM */
	// Stack
	Bot->Stack = Z_Malloc(Bot->StackLen, PU_STATIC, NULL);
	MIPS_VMAddMap(&Bot->VM, Bot->Stack, Bot->StackAddr, Bot->StackLen, MIPS_MFR | MIPS_MFW | MIPS_MFX);
	Bot->VM.CPU.r[29] = Bot->StackAddr + (Bot->StackLen - 4);
	
	// Port Info
	MIPS_VMAddMap(&Bot->VM, &l_PortInfo, EXTADDRPORTINFO, sizeof(l_PortInfo), MIPS_MFR);
	
	// Bot Info
	MIPS_VMAddMap(&Bot->VM, &Bot->VMBotInfo, EXTADDRBOTINFO, sizeof(Bot->VMBotInfo), MIPS_MFR | MIPS_MFW);
	
	// Tic Command
	MIPS_VMAddMap(&Bot->VM, &Bot->VMTicCmd, EXTADDRTICCMD, sizeof(Bot->VMTicCmd), MIPS_MFR | MIPS_MFW);
	
	// LOOKUP TABLES
	MIPS_VMAddMap(&Bot->VM, finesine, EXTADDRFINESINE, sizeof(finesine), MIPS_MFR);
	MIPS_VMAddMap(&Bot->VM, finecosine, EXTADDRFINECOSINE, sizeof((*finecosine) * (FINEANGLES / 4)), MIPS_MFR);
	MIPS_VMAddMap(&Bot->VM, finetangent, EXTADDRFINETANGENT, sizeof(finetangent), MIPS_MFR);
	MIPS_VMAddMap(&Bot->VM, tantoangle, EXTADDRTANTOANGLE, sizeof(tantoangle), MIPS_MFR);
	MIPS_VMAddMap(&Bot->VM, c_AngLUT, EXTADDRANGLUT, sizeof(c_AngLUT), MIPS_MFR);
	
	/* Link into list */
	for (i = 0; i < l_NumBots; i++)
		if (!l_Bots[i])
		{
			l_Bots[i] = Bot;
			break;
		}
	
	// No Room?
	if (i >= l_NumBots)
	{
		Z_ResizeArray((void**)&l_Bots, sizeof(*l_Bots),
			l_NumBots, l_NumBots + 1);
		l_Bots[l_NumBots++] = Bot;
	}
#undef BUFSIZE
}

/* BOT_Destroy() -- Destroys bot */
void BOT_Destroy(BOT_t* const a_Bot)
{
	int32_t i;
	
	/* Check */
	if (!a_Bot)
		return;
	
	/* Unlink */
	for (i = 0; i < l_NumBots; i++)
		if (l_Bots[i] == a_Bot)
		{
			l_Bots[i] = NULL;
			break;
		}
	
	/* Disconnect from network game */
	SN_UnplugPort(a_Bot->Port);
	
	/* Cleanup code resources */
	if (a_Bot->CodeEnt)
		WL_UnMapEntry(a_Bot->CodeEnt);
	
	/* Free resources */
	// Stack
	if (a_Bot->Stack)
		Z_Free(a_Bot->Stack);
	
	// Actual bot
	Z_Free(a_Bot);
}

/* BOT_ByProcessID() -- Finds bot by process ID */
BOT_t* BOT_ByProcessID(const uint32_t a_ProcessID)
{
	int32_t i;
	BOT_t* Bot;
	
	/* Check */
	if (!a_ProcessID)
		return NULL;
	
	/* Look */
	for (i = 0; i < l_NumBots; i++)
		if ((Bot = l_Bots[i]))
			if (Bot->ProcessID == a_ProcessID)
				return Bot;
	
	/* Not found */
	return NULL;
}

/* BOT_ByPort() -- Finds bot by process ID */
BOT_t* BOT_ByPort(SN_Port_t* const a_Port)
{
	int32_t i;
	BOT_t* Bot;
	
	/* Check */
	if (!a_Port)
		return NULL;
	
	/* Look */
	for (i = 0; i < l_NumBots; i++)
		if ((Bot = l_Bots[i]))
			if (Bot->Port == a_Port)
				return Bot;
	
	/* Not found */
	return NULL;
}

/* BOT_DestroyByPort() -- Destroys bot by port */
void BOT_DestroyByPort(SN_Port_t* const a_Port)
{
	BOT_t* Bot;
	
	/* Check */
	if (!(Bot = BOT_ByPort(a_Port)))
		return;
	
	/* Destroy this bot */
	BOT_Destroy(Bot);
}

/* BOT_LeaveStasis() -- Bot is ingame, so it becomes an active thinker */
void BOT_LeaveStasis(SN_Port_t* const a_Port)
{
	BOT_t* Bot;
	
	/* Check */
	if (!(Bot = BOT_ByPort(a_Port)))
		return;
	
	/* Change stasis mode */
	Bot->Stasis = false;
}

/* BOT_EnterStatis() -- Bot is spectating, do not cost any CPU time */
void BOT_EnterStatis(SN_Port_t* const a_Port)
{
	BOT_t* Bot;
	
	/* Check */
	if (!(Bot = BOT_ByPort(a_Port)))
		return;
	
	/* Enter stasis mode */
	Bot->Stasis = true;
}

