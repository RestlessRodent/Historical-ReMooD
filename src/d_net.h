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
} D_NetClient_t;

typedef void (*D_NCQCFunc_t)(void* const a_Data);

/*** GLOBALS ***/

extern uint32_t g_NetStat[4];					// Network stats
extern tic_t g_LastServerTic;					// Last Server Tic

/*** FUNCTIONS ***/

D_NetClient_t* D_NCAllocClient(void);
D_NetClient_t* D_NCFindClientIsServer(void);
D_NetClient_t* D_NCFindClientByNetPlayer(struct D_NetPlayer_s* const a_NetPlayer);
D_NetClient_t* D_NCFindClientByHost(I_HostAddress_t* const a_Host);
D_NetClient_t* D_NCFindClientByPlayer(struct player_s* const a_Player);
D_NetClient_t* D_NCFindClientByID(const uint32_t a_ID);

void D_NCFudgeOffHostStream(I_HostAddress_t* const a_Host, struct D_BS_s* a_Stream, const char a_Code, const char* const a_Reason);
void D_NCFudgeOffClient(D_NetClient_t* const a_Client, const char a_Code, const char* const a_Reason);

void D_NCUpdate(void);

void D_NCAddQueueCommand(const D_NCQCFunc_t a_Func, void* const a_Data);
void D_NCRunCommands(void);

void D_NCQueueDisconnect(void);

void D_NCDisconnect(void);
void D_NCServize(void);
void D_NCClientize(D_NetClient_t* a_BoundClient, I_HostAddress_t* const a_Host, const char* const a_Pass, const char* const a_JoinPass);

bool_t D_NCHostOnBanList(I_HostAddress_t* const a_Host);

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


#endif							/* __D_NET_H__ */

