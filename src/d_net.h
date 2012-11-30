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
#include "ip.h"

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
#define MAXUUIDLENGTH	(MAXPLAYERNAME * 2)		// Length of UUIDs
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
	
	DXPF_CONVEYED = DXPF_SERVER | DXPF_NOLOGIN | DXPF_DEMO | DXPF_BOT,
} D_XPlayerFlags_t;

/* D_XPlayerStatBits_t -- XPlayer status bits */
typedef enum D_XPlayerStatBits_e
{
	DXPSB_LBOVERFLOW	= UINT32_C(0x0000001),	// Local buffer overflowing
	DXPSB_NEEDSPROFILE	= UINT32_C(0x0000002),	// Needs profile loaded
	DXPSB_MISSINGTICS	= UINT32_C(0x0000004),	// Missing Tics
} D_XPlayerStatBits_t;

/*** STRUCTURES ***/

struct D_XPlayer_s;

/* D_XSocket_t -- Socket used to communicate to player */
typedef struct D_XSocket_s
{
	// Identification
	uint32_t ID;								// Unique Socket ID	
	
	// Socket Info
	I_NetSocket_t* NetSock;						// Network socket
	struct D_BS_s* CoreStream;					// Core stream
	struct D_BS_s* PerfectStream;				// Core stream
	struct D_BS_s* Streams[NUMDNCSTREAMS];		// Client Streams
	
	// Reverse Player Lookup
	struct D_XPlayer_s** Players;				// Players using socket
	size_t NumPlayers;							// Number of slots
} D_XSocket_t;

struct player_s;
struct D_ProfileEx_s;
struct B_GhostBot_s;

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
	char ProfileUUID[MAXPLAYERNAME];			// Player's Profile UUID
	char LoginUUID[MAXUUIDLENGTH];				// UUID used for login (cookie rather)
	
	// Socket
	D_XSocket_t* Socket;						// Socket player uses
	I_HostAddress_t Address;					// Address to player
	char ReverseDNS[MAXXSOCKTEXTSIZE];			// Reverse DNS of Host
	
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
	uint64_t LastProgramTic[2];					// Remote/Local program tic
	
	// Tics
	ticcmd_t LocalBuf[MAXLBTSIZE];				// Local Buffer
	int8_t LocalAt;								// Currently Place At...
	
	// Game/Profile Stuff
	tic_t LastJoinAttempt;						// Last join attempt
	tic_t CoopSpyTime;							// Time to wait to respy
	tic_t TurnHeld;								// Time turning is held
	int32_t Scores;								// Scoreboard showing
	ticcmd_t BackupTicCmd;						// Backup Tic Command
	
	// Connection Socket
	IP_Conn_t* IPTo;							// Connection To Player
	IP_Conn_t* IPFrom;							// Connection From Player (Secure)
	IP_Addr_t* IPAddr;							// Address of Player
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

extern D_XSocket_t** g_XSocks;					// Extended Sockets
extern size_t g_NumXSocks;						// Number of them

extern D_XPlayer_t** g_XPlays;					// Extended Players
extern size_t g_NumXPlays;						// Number of them

extern tic_t g_DemoFreezeTics;					// Tics to freeze demo for

/*** FUNCTIONS ***/

void D_XNetDisconnect(const bool_t a_FromDemo);
void D_XNetMakeServer(const bool_t a_Networked, const uint16_t a_NetPort);
void D_XNetConnect(const char* const a_URI);

bool_t D_XNetIsServer(void);
uint32_t D_XNetGetHostID(void);

D_XPlayer_t* D_XNetPlayerByID(const uint32_t a_ID);
D_XPlayer_t* D_XNetLocalPlayerByPID(const uint32_t a_ID);
D_XPlayer_t* D_XNetPlayerByString(const char* const a_Str);

void D_XNetDelSocket(D_XSocket_t* const a_Socket);
D_XPlayer_t* D_XNetAddPlayer(void (*a_PacketBack)(D_XPlayer_t* const a_Player, void* const a_Data), void* const a_Data);
void D_XNetKickPlayer(D_XPlayer_t* const a_Player, const char* const a_Reason);
void D_XNetSendQuit(void);
void D_XNetPartLocal(D_XPlayer_t* const a_Player);
void D_XNetChangeVar(const uint32_t a_Code, const int32_t a_Value);
void D_XNetChangeMap(const char* const a_Map);
void D_XNetChangeLocalProf(const int32_t a_ScreenID, struct D_ProfileEx_s* const a_Profile);
void D_XNetSendColors(D_XPlayer_t* const a_Player);
void D_XNetTryJoin(D_XPlayer_t* const a_Player);
void D_XNetCreatePlayer(D_XJoinPlayerData_t* const a_JoinData);
void D_XNetSetServerName(const char* const a_NewName);

bool_t D_XNetBindConn(struct IP_Conn_s* a_Conn);
void D_XNetDelConn(struct IP_Conn_s* a_Conn);

void D_XNetInit(void);
void D_XNetMultiTics(ticcmd_t* const a_TicCmd, const bool_t a_Write, const int32_t a_Player);
tic_t D_XNetTicsToRun(void);
void D_XNetUpdate(void);
bool_t D_XNetHandleEvent(const I_EventEx_t* const a_Event);

#endif							/* __D_NET_H__ */

