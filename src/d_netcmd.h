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
// DESCRIPTION: host/client network commands
//              commands are executed through the command buffer
//              like console commands

#ifndef __D_NETCMD_H__
#define __D_NETCMD_H__

#include "doomdef.h"
#include "command.h"
#include "d_ticcmd.h"
#include "i_util.h"
#include "d_net.h"

// add game commands, needs cleanup
void D_RegisterClientCommands(void);
void D_SendPlayerConfig(void);
void Command_ExitGame_f(void);

/*****************************
*** EXTENDED NETWORK STUFF ***
*****************************/

/*** CONSTANTS ***/

#define MAXDNETTICCMDCOUNT					64	// Max allowed buffered tics

/* D_NetPlayerType_t -- Profile Type */
typedef enum D_NetPlayerType_e
{
	DNPT_LOCAL,									// Local player
	DNPT_NETWORK,								// Network player
	DNPT_BOT,									// Bot player
	
	NUMDNETPLAYERTYPES
} D_NetPlayerType_t;

/* D_NetState_t -- Network State */
typedef enum D_NetState_e
{
	DNS_NULL,									// NULL State
	DNS_CONNECTING,								// Player is connecting
	DNS_DOWNLOADING,							// Downloading something
	DNS_LOADING,								// Loading the game
	DNS_PLAYING,								// Playing the game
	DNS_DROPPLAYER,								// Player should be dropped
	
	NUMDNETSTATES
} D_NetState_t;

/*** STRUCTURES ***/

struct D_ProfileEx_s;
struct player_s;
struct B_BotData_s;

/* D_NetPlayer_t -- Network Player */
typedef struct D_NetPlayer_s
{
	/* Generic */
	D_NetPlayerType_t Type;						// Type of network player
	struct D_ProfileEx_s* Profile;				// Linked Profile
	struct player_s* Player;					// Attached Player
	char UUID[MAXPLAYERNAME * 2];				// Network Player Unique ID
	char AccountName[MAXPLAYERNAME];			// Networked player account
	uint32_t ProcessID;							// Processing ID
	
	/* Player Control */
	// Sync
	int TicTotal;								// Total number of tic commands
	ticcmd_t TicCmd[MAXDNETTICCMDCOUNT];		// Tic Command to execute
	tic_t TicGameTime;							// Last tic in game time (client)
	int LocalTicTotal;							// Number of local tics
	ticcmd_t LastGoodTic;						// Last good player tics
	ticcmd_t LocalTicCmd[MAXDNETTICCMDCOUNT];	// Local Tic Commands
	tic_t LastLocalTic;							// Last local tic time
	char DisplayName[MAXPLAYERNAME];			// Name to show in network games
	D_NetState_t NetState;						// Current network state
	D_NetClient_t* NetClient;					// Network Client
	D_LastConsistData_t Consist;				// Consistency
	uint32_t ProgramTic;						// Last recieved program tic
	int32_t XMitCount;							// Transmit Count
	
	// Desync
	
	/* Specifics */
	int NetColor;								// Network Color
	
	/* Bot Related */
	struct B_BotData_s* BotData;				// Bot Data
	
	struct D_NetPlayer_s* ChainPrev;			// Previous
	struct D_NetPlayer_s* ChainNext;			// Next
} D_NetPlayer_t;

/* D_SplitInfo_t -- Split Screen Info */
typedef struct D_SplitInfo_s
{
	bool_t Active;								// Is Active
	int32_t Console;							// The console player
	int32_t Display;							// Display Player
	uint32_t ProcessID;							// Local Processing ID
	struct D_ProfileEx_s* Profile;				// Player Profile
	
	bool_t JoyBound;							// Joystick Bound
	uint32_t JoyID;								// Joystick ID
} D_SplitInfo_t;

/*** GLOBALS ***/

extern int g_SplitScreen;						// Players in splits
extern D_SplitInfo_t g_Splits[MAXSPLITSCREEN];	// Split Information

extern bool_t g_NetDev;

/*** FUNCTIONS ***/

struct player_s* D_NCSAddLocalPlayer(const char* const a_ProfileID);
struct player_s* D_NCSAddBotPlayer(const char* const a_ProfileID);

void D_NCSInit(void);
void D_NCSNetUpdateSingle(struct player_s* a_Player);
void D_NCSNetUpdateAll(void);
void D_NCSNetUpdateSingleTic(void);

void D_NCSNetSetState(const D_NetState_t a_State);
void D_NCSNetTicTransmit(D_NetPlayer_t* const a_NPp, ticcmd_t* const a_TicCmd);
void D_NCSNetMergeTics(ticcmd_t* const a_DestCmd, const ticcmd_t* const a_SrcList, const size_t a_NumSrc);

bool_t D_NCSHandleEvent(const I_EventEx_t* const a_Event);

D_NetPlayer_t* D_NCSAllocNetPlayer(void);
void D_NCSFreeNetPlayer(D_NetPlayer_t* const a_NetPlayer);

D_NetPlayer_t* D_NCSFindNetPlayer(const char* const a_Name);
D_NetPlayer_t* D_NCSFindNetPlayerByProcess(const uint32_t a_ID);
int8_t D_NCSFindSplitByProcess(const uint32_t a_ID);

const char* D_NCSGetPlayerName(const uint32_t a_PlayerID);

uint32_t D_CMakePureRandom(void);
void D_CMakeUUID(char* const a_Buf);

#endif

