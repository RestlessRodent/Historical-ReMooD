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
#include "d_block.h"

/****************
*** CONSTANTS ***
****************/

#if !defined(MAXUUIDLENGTH)
	#define MAXUUIDLENGTH	(MAXPLAYERNAME * 2)	// Length of UUIDs
#endif

#define MAXLBTSIZE						16		// Max tics in local buffer
#define MAXQUITREASON					128		// Reason for quit

/*****************
*** STRUCTURES ***
*****************/

#if !defined(__REMOOD_DBSTDEFINED)
	typedef struct D_BS_s D_BS_t;
	#define __REMOOD_DBSTDEFINED
#endif

struct B_BotTemplate_s;

typedef struct D_SNHost_s D_SNHost_t;

#if !defined(__REMOOD_DPROFTDEFINED)
	#define __REMOOD_DPROFTDEFINED
	typedef struct D_Prof_s D_Prof_t;
#endif

/* D_SNPort_t -- Port which controls a specific player or a spectator */
typedef struct D_SNPort_s
{
	char Name[MAXPLAYERNAME];					// Name of player
	struct player_s* Player;					// Player controlling
	D_SNHost_t* Host;							// Controlling host
	int32_t Screen;								// Screen number
	bool_t Bot;									// Bot controls this port
	D_Prof_t* Profile;							// Profile of player
	uint32_t ID;								// ID of Port
	ticcmd_t LocalBuf[MAXLBTSIZE];				// Local Buffer
	int8_t LocalAt;								// Currently Place At...
	bool_t WillJoin;							// Will join game
	uint32_t StatFlags;							// Status Flags
} D_SNPort_t;

/* D_SNHost_t -- Host which controls a set of playing players */
struct D_SNHost_s
{
	D_SNPort_t** Ports;							// Ports
	int32_t NumPorts;							// Number of ports
	bool_t Local;								// Local host
	uint32_t ID;								// ID of host
	I_HostAddress_t Addr;						// Host Address
	D_BS_t* BS;									// Block Stream
	bool_t Cleanup;								// Cleanup host
	char QuitReason[MAXQUITREASON];				// Reason for leaving
	
	struct
	{
		bool_t Want;							// Wants save
		bool_t Has;								// Has save
		int32_t Slot;							// Transmit slot
		tic_t TicTime;							// Time for savetic
		tic_t PTimer;							// Program Timer
		bool_t Latched;							// Latched
	} Save;										// Savegame status
	
	int32_t Ping;								// Ping of host
};

/*****************
*** PROTOTYPES ***
*****************/

bool_t D_NetSetPlayerName(const int32_t a_PlayerID, const char* const a_Name);
bool_t D_NetPlayerChangedPause(const int32_t a_PlayerID);

/*** GLOBAL TICS ***/

bool_t D_SNExtCmdInGlobal(const uint8_t a_ID, uint8_t** const a_Wp);
bool_t D_SNExtCmdInTicCmd(const uint8_t a_ID, uint8_t** const a_Wp, ticcmd_t* const a_TicCmd);

/*** SERVER CONTROL ***/

void D_SNDropAllClients(const char* const a_Reason);
void D_SNDisconnect(const bool_t a_FromDemo, const char* const a_Reason);
void D_SNPartialDisconnect(const char* const a_Reason);
bool_t D_SNIsConnected(void);
void D_SNSetConnected(const bool_t a_Set);
bool_t D_SNIsServer(void);
void D_SNStartWaiting(void);
void D_SNAddLocalProfiles(const int32_t a_NumLocal, const char** const a_Profs);
bool_t D_SNStartServer(const int32_t a_NumLocal, const char** const a_Profs, const bool_t a_JoinPlayers);
bool_t D_SNStartLocalServer(const int32_t a_NumLocal, const char** const a_Profs, const bool_t a_JoinPlayers);
bool_t D_SNServerInit(void);

/*** LOOP ***/

void D_SNUpdateLocalPorts(void);
void D_SNUpdate(void);

/*** HOST CONTROL ***/

D_SNHost_t* D_SNHostByAddr(const I_HostAddress_t* const a_Host);
D_SNHost_t* D_SNHostByID(const uint32_t a_ID);
D_SNHost_t* D_SNMyHost(void);
void D_SNSetMyHost(D_SNHost_t* const a_Host);
D_SNHost_t* D_SNCreateHost(void);
void D_SNDestroyHost(D_SNHost_t* const a_Host);

/*** PORT CONTROL ***/

D_SNPort_t* D_SNPortByID(const uint32_t a_ID);
D_SNPort_t* D_SNAddPort(D_SNHost_t* const a_Host);
void D_SNRemovePort(D_SNPort_t* const a_Port);
D_SNPort_t* D_SNRequestPort(void);
bool_t D_SNAddLocalPlayer(const char* const a_Name, const uint32_t a_JoyID, const int8_t a_ScreenID, const bool_t a_UseJoy);
D_SNTicBuf_t* D_SNBufForGameTic(const tic_t a_GameTic);
void D_SNStartTic(const tic_t a_GameTic);
void D_SNTics(ticcmd_t* const a_TicCmd, const bool_t a_Write, const int32_t a_Player);
void D_SNSyncCode(const tic_t a_GameTic, const uint32_t a_Code);
void D_SNSetPortProfile(D_SNPort_t* const a_Port, D_Prof_t* const a_Profile);
void D_SNPortRequestJoin(D_SNPort_t* const a_Port);
void D_SNPortTryJoin(D_SNPort_t* const a_Port);

/*** GAME CONTROL ***/

void D_SNRemovePlayer(const int32_t a_PlayerID);
void D_SNChangeMap(const char* const a_NewMap, const bool_t a_Reset);
void D_SNHandleGT(const uint8_t a_ID, const uint8_t** const a_PP);

/*** DRAWERS ***/

void D_SNDrawLobby(void);

/*** BUILD TIC COMMANDS ***/

bool_t D_SNHandleEvent(const I_EventEx_t* const a_Event);
void D_SNPortTicCmd(D_SNPort_t* const a_Port, ticcmd_t* const a_TicCmd);

uint32_t D_SNTicBufSum(D_SNTicBuf_t* const a_TicBuf,  const D_SNTicBufVersion_t a_VersionNum, const uint32_t a_Players);
void D_SNEncodeTicBuf(D_SNTicBuf_t* const a_TicBuf, uint8_t** const a_OutD, uint32_t* const a_OutSz, const D_SNTicBufVersion_t a_VersionNum);
bool_t D_SNDecodeTicBuf(D_SNTicBuf_t* const a_TicBuf, const uint8_t* const a_InD, const uint32_t a_InSz);

/*** TRANSMISSION ***/

void D_SNXMitTics(const tic_t a_GameTic, D_SNTicBuf_t* const a_Buffer);
int32_t D_SNOkTics(tic_t* const a_LocalP, tic_t* const a_LastP);
bool_t D_SNNetCreate(const bool_t a_Listen, const char* const a_Addr, const uint16_t a_Port);
void D_SNNetTerm(const char* const a_Reason);
bool_t D_SNHasSocket(void);
void D_SNDoTrans(void);
bool_t D_SNGotFile(const char* const a_PathName);
void D_SNDisconnectHost(D_SNHost_t* const a_Host, const char* const a_Reason);

/*** FILES ***/

void D_SNClearFiles(void);
void D_SNCloseFile(const int32_t a_Handle);
int32_t D_SNPrepFile(const char* const a_PathName, const uint32_t a_Modes);
int32_t D_SNPrepSave(void);
void D_SNSendFile(const int32_t a_Handle, D_SNHost_t* const a_Host);
void D_SNFileInit(D_BS_t* const a_BS, D_SNHost_t* const a_Host, I_HostAddress_t* const a_Addr);
void D_SNFileReady(D_BS_t* const a_BS, D_SNHost_t* const a_Host, I_HostAddress_t* const a_Addr);
void D_SNFileRecv(D_BS_t* const a_BS, D_SNHost_t* const a_Host, I_HostAddress_t* const a_Addr);
void D_SNChunkReq(D_BS_t* const a_BS, D_SNHost_t* const a_Host, I_HostAddress_t* const a_Addr);
void D_SNFileLoop(void);

#endif							/* __D_NET_H__ */

