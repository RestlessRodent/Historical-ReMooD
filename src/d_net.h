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
// DESCRIPTION: Networking stuff.
//              part of layer 4 (transport) (tp4) of the osi model
//              assure the reception of packet and proceed a checksums

#ifndef __D_NET_H__
#define __D_NET_H__

/***************
*** INCLUDES ***
***************/

#include "i_net.h"
#include "i_util.h"

/*****************
*** STRUCTURES ***
*****************/

struct D_BS_s;
struct D_ProfileEx_s;
struct B_BotTemplate_s;

/*****************
*** PROTOTYPES ***
*****************/

bool_t D_CheckNetGame(void);

bool_t D_NetSetPlayerName(const int32_t a_PlayerID, const char* const a_Name);
bool_t D_NetPlayerChangedPause(const int32_t a_PlayerID);

/******************************
*** NEW EXTENDED NETWORKING ***
******************************/

/*** CONSTANTS ***/

#define MAXXSOCKTEXTSIZE				64		// Max size of text fields

#if !defined(MAXUUIDLENGTH)
	#define MAXUUIDLENGTH	(MAXPLAYERNAME * 2)	// Length of UUIDs
#endif

#define MAXLBTSIZE						16		// Max tics in local buffer

/* D_NCStreamPath_t -- Communication Paths */
typedef enum D_NCStreamPath_e
{
	DNCSP_READ,									// Standard Read Stream
	DNCSP_WRITE,								// Standard Write Stream
	DNCSP_PERFECTREAD,							// Perfect Read Stream
	DNCSP_PERFECTWRITE,							// Perfect Write Stream	
	
	NUMDNCSTREAMS
} D_NCStreamPath_t;

/* D_XPlayerFlags_t -- Player Flags */
typedef enum D_XPlayerFlags_e
{
	DXPF_LOCAL			= UINT32_C(0x0000001),	// Player is Local
	DXPF_SERVER			= UINT32_C(0x0000002),	// Player is Server
	DXPF_NOLOGIN		= UINT32_C(0x0000004),	// Player has no login (local)
	DXPF_DEMO			= UINT32_C(0x0000008),	// Generated from a demo
	DXPF_CHALLENGED		= UINT32_C(0x0000010),	// Connection Challenged
	DXPF_BOT			= UINT32_C(0x0000020),	// Bot Controller Player
	DXPF_DEFUNCT		= UINT32_C(0x0000040),	// Remove player next cycle
	
	DXPF_CONVEYED = DXPF_SERVER | DXPF_NOLOGIN | DXPF_DEMO | DXPF_BOT,
} D_XPlayerFlags_t;

/* D_XPlayerStatBits_t -- XPlayer status bits */
typedef enum D_XPlayerStatBits_e
{
	DXPSB_LBOVERFLOW	= UINT32_C(0x0000001),	// Local buffer overflowing
	DXPSB_NEEDSPROFILE	= UINT32_C(0x0000002),	// Needs profile loaded
	DXPSB_MISSINGTICS	= UINT32_C(0x0000004),	// Missing Tics
	DXPSB_CAUSEOFLAG	= UINT32_C(0x0000008),	// Is Causing the lag
} D_XPlayerStatBits_t;

/*** STRUCTURES ***/

struct D_XPlayer_s;

struct player_s;
struct D_ProfileEx_s;
struct B_GhostBot_s;
struct D_XDesc_s;
struct D_XEndPoint_s;

/* D_XPlayer_t -- A player, spectator, bot, whatever */
typedef struct D_XPlayer_s
{
	uint32_t Flags;								// Flags
	
	// Security
	char SSToken[MAXUUIDLENGTH];				// Token set by server
	char CLToken[MAXUUIDLENGTH];				// Token set by client
	
	// Identification
	uint32_t ID;								// Unique Player ID
	uint32_t HostID;							// ID to the host (shared)
	uint32_t ClProcessID;						// Client's Process ID
	char AccountName[MAXPLAYERNAME];			// Player's Account Name
	char DisplayName[MAXPLAYERNAME];			// Player's Display Name
	char ProfileUUID[MAXUUIDLENGTH];			// Player's Profile UUID
	char LoginUUID[MAXUUIDLENGTH];				// UUID used for login (cookie rather)
	
	// Socket
	struct
	{
		struct D_XEndPoint_s* EndPoint;			// Endpoint connection
		I_HostAddress_t Address;				// Address to player
		char ReverseDNS[MAXXSOCKTEXTSIZE];		// Reverse DNS of Host
	} Socket;									// Socket Information
	
	// Account Server
	char AccountCookie[MAXPLAYERNAME];			// Cookie for account
	char AccountServer[MAXXSOCKTEXTSIZE];		// Server that manages the account
	I_HostAddress_t AccountServAddr;			// Address to account server
	char AccountServRDNS[MAXXSOCKTEXTSIZE];		// Reverse DNS to account server
	
	// In-Game
	int8_t ScreenID;							// Screen Identity
	int32_t InGameID;							// Player in game number
	struct player_s* Player;					// Pointer to player
	struct D_ProfileEx_s* Profile;				// Profile Used by player
	struct B_GhostBot_s* BotData;				// Bot data used by player
	int32_t Ping;								// Player's Ping
	uint32_t StatusBits;						// Status Flags
	
	// Timing
	tic_t LastRanTic;							// Last tic ran
	tic_t LastXMit;								// Last XMit time
	tic_t LastAckTic;							// Last acknowledged tic
	uint64_t LastProgramTic[2];					// Remote/Local program tic
	
	// Tics
	ticcmd_t LocalBuf[MAXLBTSIZE];				// Local Buffer
	int8_t LocalAt;								// Currently Place At...
	
	// Game Stuff
	tic_t LastJoinAttempt;						// Last join attempt
	tic_t CoopSpyTime;							// Time to wait to respy
	tic_t TurnHeld;								// Time turning is held
	int32_t Scores;								// Scoreboard showing
	ticcmd_t BackupTicCmd;						// Backup Tic Command
	bool_t Turned180;							// Did 180 degre turn
	bool_t TransSave;							// Save game transmitted and loaded
	tic_t LagStart;								// Start of lag
	tic_t LagKill;								// Kill at this lag time
	tic_t LagThreshold;							// Threshold of lag
	tic_t LagThreshExpire;						// Time when threshold expires
	bool_t TriedToJoin;							// Tried to join already
	bool_t DidConnectTrans;						// Did connect transport
	bool_t SaveSent;							// Was Sent savegame
	bool_t CounterOp;							// On Counterop Team
	int32_t VTeam;								// Virtual Team On
	int32_t Color;								// Color
} D_XPlayer_t;

/* D_XJoinPlayerData_t -- Data for joining player */
typedef struct D_XJoinPlayerData_s
{
	// Standard
	uint32_t ID;								// XPlayer ID
	uint32_t ProcessID;							// ClProcessID
	uint32_t HostID;							// Host ID
	uint32_t Flags;								// Flags
	uint8_t Color;								// Color
	uint8_t CTFTeam;							// CTFTeam
	uint32_t SkinHash;							// Skin Hash
	char DisplayName[MAXPLAYERNAME];			// Display name
	char HexenClass[MAXPLAYERNAME];				// Hexen Class
} D_XJoinPlayerData_t;

/*** GLOBALS ***/

extern D_XPlayer_t** g_XPlays;					// Extended Players
extern size_t g_NumXPlays;						// Number of them

extern tic_t g_DemoFreezeTics;					// Tics to freeze demo for

/*** FUNCTIONS ***/

void D_NCLocalPlayerAdd(const char* const a_Name, const bool_t a_Bot, const uint32_t a_JoyID, const int8_t a_ScreenID, const bool_t a_UseJoy);

bool_t D_XNetGlobalTic(const uint8_t a_ID, void** const a_Wp);
bool_t D_XNetGetCommand(const uint8_t a_ID, const uint32_t a_Size, void** const a_Wp, ticcmd_t* const a_TicCmd);

void D_XNetPlaceTicCmd(const tic_t a_GameTic, const int32_t a_Player, ticcmd_t* const a_Cmd);
void D_XNetFinalCmds(const tic_t a_GameTic, const uint32_t a_SyncCode);

void D_XNetDisconnect(const bool_t a_FromDemo);
void D_XNetMakeServer(const bool_t a_Networked, I_HostAddress_t* const a_Addr, const uint32_t a_GameID, const bool_t a_NotHost);
void D_XNetConnect(I_HostAddress_t* const a_Addr, const uint32_t a_GameID, const bool_t a_NotClient);

bool_t D_XNetIsServer(void);
bool_t D_XNetIsConnected(void);
uint32_t D_XNetGetHostID(void);
void D_XNetSetHostID(const uint32_t a_NewID);

D_XPlayer_t* D_XNetPlayerByXPlayerHost(D_XPlayer_t* const a_XPlayer);
D_XPlayer_t* D_XNetPlayerByID(const uint32_t a_ID);
D_XPlayer_t* D_XNetPlayerByHostID(const uint32_t a_ID);
D_XPlayer_t* D_XNetLocalPlayerByPID(const uint32_t a_ID);
D_XPlayer_t* D_XNetPlayerByString(const char* const a_Str);
D_XPlayer_t* D_XNetPlayerByAddr(const I_HostAddress_t* const a_Addr);

D_XPlayer_t* D_XNetAddPlayer(void (*a_PacketBack)(D_XPlayer_t* const a_Player, void* const a_Data), void* const a_Data, const bool_t a_FromGTicker);
void D_XNetKickPlayer(D_XPlayer_t* const a_Player, const char* const a_Reason, const bool_t a_FromGTicker);
void D_XNetClearDefunct(void);
void D_XNetSpectate(const int32_t a_PlayerID);
void D_XNetSendQuit(void);
void D_XNetPartLocal(D_XPlayer_t* const a_Player);
void D_XNetChangeVar(const uint32_t a_Code, const int32_t a_Value);
void D_XNetChangeMap(const char* const a_Map, const bool_t a_Reset);
void D_XNetChangeLocalProf(const int32_t a_ScreenID, struct D_ProfileEx_s* const a_Profile);
void D_XNetSendColors(D_XPlayer_t* const a_Player);
void D_XNetTryJoin(D_XPlayer_t* const a_Player);
void D_XNetCreatePlayer(D_XJoinPlayerData_t* const a_JoinData);
void D_XNetSetServerName(const char* const a_NewName);

D_XNetTicBuf_t* D_XNetBufForTic(const tic_t a_GameTic, const bool_t a_Create);
void D_XNetEncodeTicBuf(D_XNetTicBuf_t* const a_TicBuf, uint8_t** const a_OutD, uint32_t* const a_OutSz);
void D_XNetDecodeTicBuf(D_XNetTicBuf_t* const a_TicBuf, const uint8_t* const a_InD, const uint32_t a_InSz);

void D_XNetInit(void);
void D_XNetUpPlayerTics(D_XPlayer_t* const a_Player, const tic_t a_GameTic, ticcmd_t* const a_TicCmd);
void D_XNetMultiTics(ticcmd_t* const a_TicCmd, const bool_t a_Write, const int32_t a_Player);
tic_t D_XNetTicsToRun(void);
void D_XNetForceLag(void);
void D_XNetPushJW(void);
void D_XNetUpdate(void);
bool_t D_XNetHandleEvent(const I_EventEx_t* const a_Event);

void D_XNetInitialServer(void);
void D_XNetBecomeServer(void);

uint32_t D_XNetMakeID(const uint32_t a_ID);

#endif							/* __D_NET_H__ */

