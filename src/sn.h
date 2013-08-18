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
// DESCRIPTION: Networking stuff.
//              part of layer 4 (transport) (tp4) of the osi model
//              assure the reception of packet and proceed a checksums

#ifndef __D_NET_H__
#define __D_NET_H__

/***************
*** INCLUDES ***
***************/

#include "i_util.h"
#include "d_ticcmd.h"
#include "bot.h"

/****************
*** CONSTANTS ***
****************/

#if !defined(MAXUUIDLENGTH)
	#define MAXUUIDLENGTH	(MAXPLAYERNAME * 2)	// Length of UUIDs
#endif

#define MAXSERVERNAME					128		// Max name length of server
#define MAXLBTSIZE						16		// Max tics in local buffer
#define MAXQUITREASON					128		// Reason for quit

#define MAXNETXTICS				TICRATE

#define MAXPINGWINDOWS					8		// Max ping window count
#define MAXCHATLINE 128							// max one can blurt

/* SN_PortSetting_t -- Port Settings */
typedef enum SN_PortSetting_e
{
	DSNPS_VTEAM,								// Virtual Team
	DSNPS_COLOR,								// Color
	DSNPS_NAME,									// Name of player
	DSNPS_COUNTEROP,							// Counter-operative
	
	NUMDSNPS
} SN_PortSetting_t;

/*****************
*** STRUCTURES ***
*****************/

/* SN_PingWin_t -- Ping window */
typedef struct SN_PingWin_s
{
	uint32_t Code;								// Unique Code (Security)
	tic_t SendTime;								// Send time
	uint32_t Millis;							// Milliseconds
} SN_PingWin_t;

/* Define D_BS_t */
#if !defined(__REMOOD_DBSTDEFINED)
	typedef struct D_BS_s D_BS_t;
	#define __REMOOD_DBSTDEFINED
#endif

typedef struct SN_Host_s SN_Host_t;

/* Define D_Prof_t */
#if !defined(__REMOOD_DPROFTDEFINED)
	#define __REMOOD_DPROFTDEFINED
	typedef struct D_Prof_s D_Prof_t;
#endif

/* Define player_t */
#if !defined(__REMOOD_PLAYERT_DEFINED)
	typedef struct player_s player_t;
	#define __REMOOD_PLAYERT_DEFINED
#endif

/* Define SN_Port_t */
#if !defined(__REMOOD_SNPORT_DEFINED)
	typedef struct SN_Port_s SN_Port_t;
	#define __REMOOD_SNPORT_DEFINED
#endif

/* Define CL_View_t */
#if !defined(__REMOOD_CLVIEW_DEFINED)
	typedef struct CL_View_s CL_View_t;
	#define __REMOOD_CLVIEW_DEFINED
#endif

/* SN_Port_t -- Port which controls a specific player or a spectator */
struct SN_Port_s
{
	char Name[MAXPLAYERNAME];					// Name of player
	player_t* Player;							// Player controlling
	SN_Host_t* Host;							// Controlling host
	int32_t Screen;								// Screen number
	bool_t Bot;									// Bot controls this port
	D_Prof_t* Profile;							// Profile of player
	uint32_t ID;								// ID of Port
	ticcmd_t LocalBuf[MAXLBTSIZE];				// Local Buffer
	int8_t LocalAt;								// Currently Place At...
	bool_t WillJoin;							// Will join game
	uint32_t StatFlags;							// Status Flags
	uint32_t ProcessID;							// Local player process ID
	bool_t AttachMsg;							// Displayed attach message
	ticcmd_t BackupCmd;							// Backup tic command
	uint32_t LocalStatFlags;					// Local Status Flags
	tic_t JoinWait;								// Join wait (to not spam server)
	char ChatBuf[MAXCHATLINE];					// Chat buffer
	int16_t ChatAt;								// Chat currently at
	tic_t ChatCoolDown;							// Chat cooldown time
	uint32_t ChatID;							// Chat ID Number
	int8_t VTeam;								// Player's Team
	int8_t Color;								// Player's Color
	bool_t CounterOp;							// CounterOp Player
	CL_View_t* View;							// Port's viewport
	void* BotPtr;								// Bot Pointer (Speed)
};

/* SN_Host_t -- Host which controls a set of playing players */
struct SN_Host_s
{
	SN_Port_t** Ports;							// Ports
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
	tic_t MinTic;								// Minimum tic bound
	tic_t LastSeen;								// Last time seen
	SN_PingWin_t Pings[MAXPINGWINDOWS];		// Ping windows
	int8_t PingAt;								// Current window
	tic_t NextPing;								// Time of next ping
	tic_t LastPing;								// Last ping time
};

/* SN_Server_t -- Remote Server */
typedef struct SN_Server_s
{
	tic_t FirstSeen;							// First Seen at
	tic_t UpdatedAt;							// Updated at
	tic_t OutAt;								// Time where it goes bye
	char Name[MAXSERVERNAME];					// Name of server
	I_HostAddress_t Addr;						// Address
	SN_PingWin_t Pings[MAXPINGWINDOWS];		// Ping windows
	int8_t PingAt;								// Current window
	tic_t NextPing;								// Time of next ping
	tic_t LastPing;								// Last ping time
} SN_Server_t;

/*****************
*** PROTOTYPES ***
*****************/

bool_t D_NetSetPlayerName(const int32_t a_PlayerID, const char* const a_Name);
bool_t D_NetPlayerChangedPause(const int32_t a_PlayerID);

/*** GLOBAL TICS ***/

bool_t SN_ExtCmdInGlobal(const uint8_t a_ID, uint8_t** const a_Wp);
bool_t SN_ExtCmdInTicCmd(const uint8_t a_ID, uint8_t** const a_Wp, ticcmd_t* const a_TicCmd);

/*** SERVER CONTROL ***/

void SN_DropAllClients(const char* const a_Reason);
void SN_Disconnect(const bool_t a_FromDemo, const char* const a_Reason);
void SN_PartialDisconnect(const char* const a_Reason);
bool_t SN_IsConnected(void);
void SN_SetConnected(const bool_t a_Set);
bool_t SN_IsServer(void);
void SN_StartWaiting(void);
void SN_AddLocalProfiles(const int32_t a_NumLocal, const char** const a_Profs);
bool_t SN_StartServer(const int32_t a_NumLocal, const char** const a_Profs, const bool_t a_JoinPlayers);
bool_t SN_StartLocalServer(const int32_t a_NumLocal, const char** const a_Profs, const bool_t a_JoinPlayers, const bool_t a_MakePlayer);
bool_t SN_ServerInit(void);

/*** LOOP ***/

void SN_UpdateLocalPorts(void);
void SN_Update(void);

/*** HOST CONTROL ***/

SN_Host_t* SN_HostByAddr(const I_HostAddress_t* const a_Host);
SN_Host_t* SN_HostByID(const uint32_t a_ID);
SN_Host_t* SN_MyHost(void);
void SN_SetMyHost(SN_Host_t* const a_Host);
SN_Host_t* SN_CreateHost(void);
void SN_DestroyHost(SN_Host_t* const a_Host);
bool_t SN_CleanupHost(SN_Host_t* const a_Host);

/*** PORT CONTROL ***/

SN_Port_t* SN_PortByID(const uint32_t a_ID);
SN_Port_t* SN_AddPort(SN_Host_t* const a_Host);
void SN_RemovePort(SN_Port_t* const a_Port);
void SN_UnplugPort(SN_Port_t* const a_Port);
SN_Port_t* SN_RequestPort(const uint32_t a_ProcessID, const bool_t a_XMit);
bool_t SN_AddLocalPlayer(const char* const a_Name, const uint32_t a_JoyID, const int8_t a_ScreenID, const bool_t a_UseJoy);
SN_TicBuf_t* SN_BufForGameTic(const tic_t a_GameTic);
int32_t SN_NumSeqTics(void);
void SN_StartTic(const tic_t a_GameTic);
void SN_LocalTurn(SN_Port_t* const a_Port, ticcmd_t* const a_TicCmd);
void SN_Tics(ticcmd_t* const a_TicCmd, const bool_t a_Write, const int32_t a_Player);
void SN_SyncCode(const tic_t a_GameTic, const uint32_t a_Code);
void SN_CheckSyncCode(SN_Host_t* const a_Host, const tic_t a_GameTic, const uint32_t a_Code);
void SN_SetPortProfile(SN_Port_t* const a_Port, D_Prof_t* const a_Profile);
void SN_PortRequestJoin(SN_Port_t* const a_Port);
void SN_PortTryJoin(SN_Port_t* const a_Port);
const char* SN_GetPortName(SN_Port_t* const a_Port);
void SN_PortSetting(SN_Port_t* const a_Port, const SN_PortSetting_t a_Setting, const int32_t a_IntVal, const char* const a_StrVal, const uint32_t a_StrLen);
void SN_PortSettingOnPort(SN_Port_t* const a_Port, const SN_PortSetting_t a_Setting, const int32_t a_IntVal, const char* const a_StrVal, const uint32_t a_StrLen);

/*** GAME CONTROL ***/

void SN_ChangeVar(const uint32_t a_Code, const int32_t a_Value);
void SN_DirectChat(const uint32_t a_HostID, const uint32_t a_ID, const uint8_t a_Mode, const uint32_t a_Target, const char* const a_Message);
void SN_RemovePlayer(const int32_t a_PlayerID);
void SN_ChangeMap(const char* const a_NewMap, const bool_t a_Reset);
void SN_HandleGT(const uint8_t a_ID, const uint8_t** const a_PP);

/*** DRAWERS ***/

void SN_DrawLobby(void);
void SN_Drawer(void);
void SN_SetServerLagWarn(const tic_t a_EstPD);

/*** BUILD TIC COMMANDS ***/

void SN_ChatDrawer(const int8_t a_Screen, const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H);
void SN_ClearChat(const int32_t a_Screen);
bool_t SN_HandleEvent(const I_EventEx_t* const a_Event);
void SN_PortTicCmd(SN_Port_t* const a_Port, ticcmd_t* const a_TicCmd);

uint32_t SN_TicBufSum(SN_TicBuf_t* const a_TicBuf,  const SN_TicBufVersion_t a_VersionNum, const uint32_t a_Players);
void SN_EncodeTicBuf(SN_TicBuf_t* const a_TicBuf, uint8_t** const a_OutD, uint32_t* const a_OutSz, const SN_TicBufVersion_t a_VersionNum);
bool_t SN_DecodeTicBuf(SN_TicBuf_t* const a_TicBuf, const uint8_t* const a_InD, const uint32_t a_InSz);

/*** TRANSMISSION ***/

void SN_ClearJobs(void);
void SN_XMitTics(const tic_t a_GameTic, SN_TicBuf_t* const a_Buffer);
int32_t SN_OkTics(tic_t* const a_LocalP, tic_t* const a_LastP);
bool_t SN_NetCreate(const bool_t a_Listen, const char* const a_Addr, const uint16_t a_Port);
void SN_NetTerm(const char* const a_Reason);
bool_t SN_HasSocket(void);
void SN_DoTrans(void);
bool_t SN_GotFile(const char* const a_PathName);
void SN_DisconnectHost(SN_Host_t* const a_Host, const char* const a_Reason);
void SN_RequestPortNet(const uint32_t a_ProcessID);
void SN_UnplugPortNet(SN_Port_t* const a_Port);
void SN_PortJoinGame(SN_Port_t* const a_Port);
void SN_ReqSpectatePort(const uint32_t a_Player);
bool_t SN_WaitingForSave(void);
void SN_SendSyncCode(const tic_t a_GameTic, const uint32_t a_Code);
void SN_SendChat(SN_Port_t* const a_Port, const bool_t a_Team, const char* const a_Text);
void SN_SetLastTic(void);
void SN_AppendLocalCmds(D_BS_t* const a_BS);
void SN_SendSettings(SN_Port_t* const a_Port, const SN_PortSetting_t a_Setting, const int32_t a_IntVal, const char* const a_StrVal, const uint32_t a_StrLen);

/*** FILES ***/

void SN_ClearFiles(void);
void SN_CloseFile(const int32_t a_Handle);
int32_t SN_PrepFile(const char* const a_PathName, const uint32_t a_Modes);
int32_t SN_PrepSave(void);
void SN_SendFile(const int32_t a_Handle, SN_Host_t* const a_Host);
void SN_FileInit(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr);
void SN_FileReady(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr);
void SN_FileRecv(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr);
void SN_ChunkReq(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr);
void SN_FileLoop(void);

/*** MASTER SERVER INTERFACE ***/

bool_t I_BootHTTPSpy(void);
void I_UpdateHTTPSpy(void);
void SN_OpenMCast(void);
void SN_DoMultiCast(void);
void SN_UpdateServers(void);
SN_Server_t* SN_FindServerByAddr(I_HostAddress_t* const a_Addr);
SN_Server_t* SN_FindServerByIndex(const int32_t a_Index);
SN_Server_t* SN_CreateServer(I_HostAddress_t* const a_Addr);

#endif							/* __D_NET_H__ */

