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

/*****************
*** STRUCTURES ***
*****************/

struct D_NetPlayer_s;
struct D_BS_s;
struct D_ProfileEx_s;
struct B_BotTemplate_s;

/*****************
*** PROTOTYPES ***
*****************/

void D_SyncNetDebugMessage(const char* const a_Format, ...);
bool_t D_SyncNetIsArbiter(void);

void D_SyncNetSetMapTime(const tic_t a_Time);
tic_t D_SyncNetMapTime(void);
tic_t D_SyncNetRealTime(void);
tic_t D_SyncNetAllReady(void);
void D_SyncNetAppealTime(void);

bool_t D_SyncNetIsPaused(void);
bool_t D_SyncNetUpdate(void);
bool_t D_SyncNetIsSolo(void);

bool_t D_CheckNetGame(void);

bool_t D_NetSetPlayerName(const int32_t a_PlayerID, const char* const a_Name);
bool_t D_NetPlayerChangedPause(const int32_t a_PlayerID);

/*****************************************************************************/

#define __REMOOD_NCSNET

#define NETCLIENTRHLEN					256		// Reverse host length
#define MAXCONNKEYSIZE					8		// Connection Key Size

/*** STRUCTURES ***/

struct player_s;

/* D_NCStreamPath_t -- Communication Paths */
typedef enum D_NCStreamPath_e
{
	DNCSP_READ,									// Standard Read Stream
	DNCSP_WRITE,								// Standard Write Stream
	DNCSP_PERFECTREAD,							// Perfect Read Stream
	DNCSP_PERFECTWRITE,							// Perfect Write Stream	
	
	NUMDNCSTREAMS
} D_NCStreamPath_t;

/* D_LastConsistData_t -- Last Consistency Data of last tic */
typedef struct D_LastConsistData_s
{
	tic_t GameTic;								// Tic Ran
	uint8_t PrIndex;							// P_Random() Index
	uint32_t PosMask;							// Position Mask
} D_LastConsistData_t;

/* D_NetClient_t -- Network Client */
typedef struct D_NetClient_s
{
	/* Player Ownership */
	struct D_NetPlayer_s** Arbs;				// Arbitrating net players
	size_t NumArbs;								// Number of arbs
	
	/* Location and Pathing */
	I_HostAddress_t Address;					// Address to client
	I_NetSocket_t* NetSock;						// Network socket
	struct D_BS_s* CoreStream;		// Core stream
	struct D_BS_s* PerfectStream;		// Core stream
	struct D_BS_s* Streams[NUMDNCSTREAMS];	// Client Streams
	
	/* Flags */
	bool_t IsLocal;								// Local client
	bool_t IsServer;							// Is a server
	
	/* Other Info */
	char ReverseDNS[NETCLIENTRHLEN];			// Hostname of player
	bool_t ReadyToPlay;							// Client is ready to play
	bool_t SaveGameSent;						// Save game was sent
	uint32_t HostID;							// Host Identity (Unique, hopefully)
	uint32_t Key[MAXCONNKEYSIZE];				// Connection Key
	uint32_t GenKey[MAXCONNKEYSIZE];			// Generated Server-Side Key
	uint32_t ReadyTimeout;						// Time until must be ready
	
	D_LastConsistData_t Consist;				// Consistency
	uint64_t ProgramTic;						// Last recieved program tic
	tic_t GameTic;								// Client Gametic
	bool_t Protected;							// Protected (anti-spoof)
} D_NetClient_t;

typedef void (*D_NCQCFunc_t)(void* const a_Data);

/*** GLOBALS ***/

extern uint32_t g_NetStat[4];					// Network stats
extern tic_t g_LastServerTic;					// Last Server Tic
extern D_LastConsistData_t g_LastConsist;		// Last consistency

/*** FUNCTIONS ***/

D_NetClient_t* D_NCAllocClient(void);
D_NetClient_t* D_NCFindClientIsServer(void);
D_NetClient_t* D_NCFindClientByNetPlayer(struct D_NetPlayer_s* const a_NetPlayer);
D_NetClient_t* D_NCFindClientByHost(I_HostAddress_t* const a_Host);
D_NetClient_t* D_NCFindClientByPlayer(struct player_s* const a_Player);
D_NetClient_t* D_NCFindClientByID(const uint32_t a_ID);
uint32_t D_NCGetMyHostID(void);

void D_NCFudgeOffHostStream(I_HostAddress_t* const a_Host, struct D_BS_s* a_Stream, const char a_Code, const char* const a_Reason);
void D_NCFudgeOffClient(D_NetClient_t* const a_Client, const char a_Code, const char* const a_Reason);

void D_NCUpdate(void);
void D_NCDrawer(void);

void D_NCAddQueueCommand(const D_NCQCFunc_t a_Func, void* const a_Data);
void D_NCRunCommands(void);

void D_NCQueueDisconnect(void);

void D_NCDisconnect(const bool_t a_FromDemo);
void D_NCServize(void);
void D_NCClientize(D_NetClient_t* a_BoundClient, I_HostAddress_t* const a_Host, const char* const a_Pass, const char* const a_JoinPass);

bool_t D_NCHostOnBanList(I_HostAddress_t* const a_Host);

void D_NCReqPrefChange(struct D_ProfileEx_s* a_Profile, struct B_BotTemplate_s* a_Bot, const uint32_t a_Player);
void D_NCReqVarChange(const uint32_t a_Code, const int32_t a_NewVal);
void D_NCLocalPlayerAdd(const char* const a_Name, const bool_t a_Bot, const uint32_t a_JoyID, const int8_t a_ScreenID, const bool_t a_UseJoy);
void D_NCReqAddPlayer(struct D_ProfileEx_s* a_Profile, const bool_t a_Bot);

void D_NCZapNetPlayer(struct D_NetPlayer_s* const a_Player);

/*** NSZZ Funcs ***/
void D_NSZZ_SendINFO(struct D_BS_s* a_Stream, const uint32_t a_LocalTime);
bool_t D_NSZZ_SendINFX(struct D_BS_s* a_Stream, size_t* const a_It);
void D_NSZZ_SendMOTD(struct D_BS_s* a_Stream);
void D_NSZZ_SendFullWADS(struct D_BS_s* a_Stream, I_HostAddress_t* const a_Host);

/*** NCSR Funcs ***/
void D_NCSR_RequestMap(const char* const a_Map);
void D_NCSR_RequestNewPlayer(struct D_ProfileEx_s* a_Profile);
void D_NCSR_RequestWAD(const char* const a_WADSum);
void D_NCSR_RequestServerWADs(void);
void D_NCSR_SendServerReady(void);
void D_NCSR_SendLoadingStatus(const int32_t a_MajIs, const int32_t a_MajOf, const int32_t a_MinIs, const int32_t a_MinOf);

/**** NCHE Funcs ***/
struct D_NetPlayer_s;
struct D_ProfileEx_s;

void D_NCHE_ServerCreatePlayer(const size_t a_pNum, struct D_NetPlayer_s* const a_NetPlayer, struct D_ProfileEx_s* const a_Profile, D_NetClient_t* const a_NetClient);
void D_NCHE_SendSaveGame(D_NetClient_t* const a_Client);

/*** NCQC Funcs ***/
void D_NCQC_MapChange(void* const a_Data);









void D_LoadNetTic(void);
bool_t D_TicReady(const tic_t a_WhichTic);
void D_ClearNetTics(void);
void D_NetXMitCmds(void);

void D_NetReadGlobalTicCmd(ticcmd_t* const a_TicCmd);
void D_NetWriteGlobalTicCmd(ticcmd_t* const a_TicCmd);

void D_NetReadTicCmd(ticcmd_t* const a_TicCmd, const int a_Player);
void D_NetWriteTicCmd(ticcmd_t* const a_TicCmd, const int a_Player);

/******************************
*** NEW EXTENDED NETWORKING ***
******************************/

/*** CONSTANTS ***/

#define MAXXSOCKTEXTSIZE				64		// Max size of text fields
#define MAXUUIDLENGTH	(MAXPLAYERNAME * 2)		// Length of UUIDs

/* D_XPlayerFlags_t -- Player Flags */
typedef enum D_XPlayerFlags_e
{
	DXPF_LOCAL			= UINT32_C(0x0000001),	// Player is Local
	DXPF_SERVER			= UINT32_C(0x0000002),	// Player is Server
	DXPF_NOLOGIN		= UINT32_C(0x0000004),	// Player has no login (local)
	DXPF_DEMO			= UINT32_C(0x0000008),	// Generated from a demo
	DXPF_CHALLENGED		= UINT32_C(0x0000010),	// Connection Challenged
	DXPF_BOT			= UINT32_C(0x0000020),	// Bot Controller Player
} D_XPlayerFlags_t;

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
struct B_BotData_s;

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
	struct B_BotData_s* BotData;				// Bot data used by player
	int32_t Ping;								// Player's Ping
} D_XPlayer_t;

/*** GLOBALS ***/

extern D_XSocket_t** g_XSocks;					// Extended Sockets
extern size_t g_NumXSocks;						// Number of them

extern D_XPlayer_t** g_XPlays;					// Extended Players
extern size_t g_NumXPlays;						// Number of them

/*** FUNCTIONS ***/

void D_XNetDisconnect(const bool_t a_FromDemo);
void D_XNetMakeServer(const bool_t a_Networked, const uint16_t a_NetPort);

bool_t D_XNetIsServer(void);

D_XPlayer_t* D_XNetPlayerByID(const uint32_t a_ID);

void D_XNetDelSocket(D_XSocket_t* const a_Socket);
D_XPlayer_t* D_XNetAddPlayer(void (*a_PacketBack)(D_XPlayer_t* const a_Player, void* const a_Data), void* const a_Data);
void D_XNetKickPlayer(D_XPlayer_t* const a_Player, const char* const a_Reason);
void D_XNetSendQuit(void);

tic_t D_XNetTicsToRun(void);
void D_XNetUpdate(void);

#endif							/* __D_NET_H__ */

