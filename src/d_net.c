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
// Copyright (C) 2011-2012 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION:

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "doomdef.h"
#include "g_game.h"
#include "i_net.h"
#include "i_system.h"
#include "m_argv.h"
#include "d_net.h"
#include "w_wad.h"
#include "d_clisrv.h"
#include "z_zone.h"
#include "i_util.h"
#include "d_block.h"
#include "console.h"
#include "p_info.h"
#include "p_demcmp.h"
#include "d_main.h"
#include "m_menu.h"
#include "r_main.h"
#include "doomstat.h"
#include "p_setup.h"
#include "b_bot.h"

/*************
*** LOCALS ***
*************/

static tic_t l_MapTime = 0;						// Map local time
static tic_t l_BaseTime = 0;					// Base Game Time
static tic_t l_LocalTime = 0;					// Local Time
static bool_t l_ConsistencyFailed = false;		// Consistency failed
static bool_t l_SoloNet = true;					// Solo Network
static bool_t l_IsConnected = false;			// Connected to server

/****************
*** FUNCTIONS ***
****************/

/* D_SyncNetDebugMessage() -- Debug message for syncrhonized networking */
void D_SyncNetDebugMessage(const char* const a_Format, ...)
{
#define BUFSIZE 512
	va_list ArgPtr;
	char Text[BUFSIZE];
	
	/* Check */
	if (!M_CheckParm("-devnet"))
		return;
		
	// Make
	va_start(ArgPtr, a_Format);
	vsnprintf(Text, BUFSIZE, a_Format, ArgPtr);
	va_end(ArgPtr);
	
	// Print
	CONL_PrintF("%s\n", Text);
#undef BUFSIZE
}

/* D_SyncNetIsArbiter() -- Do we control the game? */
bool_t D_SyncNetIsArbiter(void)
{
	D_NetClient_t* Server;
	Server = D_NCFindClientIsServer();
	
	/* Which? */
	if (!Server)
		return false;
	
	/* Return if local */
	return Server->IsLocal;
}

/* D_SyncNetSetMapTime() -- Sets the new map time */
void D_SyncNetSetMapTime(const tic_t a_Time)
{
	l_MapTime = a_Time;
}

/* D_SyncNetMapTime() -- Returns the current map time */
tic_t D_SyncNetMapTime(void)
{
	return l_MapTime;
}

/* D_SyncNetRealTime() -- Returns the real game time */
tic_t D_SyncNetRealTime(void)
{
	/* Just return the number of tics that has passed */
	return I_GetTimeMS() / (tic_t)TICSPERMS;
}

extern CONL_StaticVar_t l_CONPauseGame;

/* D_SyncNetIsPaused() -- Returns true if the game is paused */
// NOTE THAT IS IS NOT THE SAME AS "paused". THIS IS ACTUAL TRUE PAUSES AS IF
// SAY A MENU IS OPEN. IF SO, DEMOS ARE NOT WRITTEN TO AND THE MAP TIME IS NOT
// INCREMENTED AT ALL.
bool_t D_SyncNetIsPaused(void)
{
	if (D_SyncNetIsSolo() && !demoplayback &&
		((M_ExAllUIActive() && g_ResumeMenu <= 0)
			|| (l_CONPauseGame.Value->Int && CONL_IsActive())))
	{
		// Appeal to the time code so the game doesn't speed up
		D_SyncNetAppealTime();
		return true;
	}
	return false;
}

/* D_SyncNetIsSolo() -- Is solo game (non-networked) */
bool_t D_SyncNetIsSolo(void)
{
	return l_SoloNet;
}

/* D_SyncNetAppealTime() -- Appeals to the time code */
void D_SyncNetAppealTime(void)
{
	l_LocalTime = (I_GetTimeMS() / TICSPERMS) - l_BaseTime;
}

/* D_SyncNetUpdate() -- Update synchronized networking */
bool_t D_SyncNetUpdate(void)
{
	/* Old Update Code */
	NetUpdate();
	
	/* Update all networked players */
	D_NCSNetUpdateAll();
	
	/* Update network code */
	D_NCUpdate();
	D_XNetUpdate();
	
	/* Success */
	return true;
}

/*****************************************************************************/

/* D_NetSetPlayerName() -- Sets name of player */
bool_t D_NetSetPlayerName(const int32_t a_PlayerID, const char* const a_Name)
{
	uint32_t OldNameHash;
	uint32_t NewNameHash;
	char OldName[MAXPLAYERNAME + 1];
	
	/* Check */
	if (a_PlayerID < 0 || a_PlayerID >= MAXPLAYERS || !a_Name)
		return false;
	
	/* Hash old name */
	memset(OldName, 0, sizeof(OldName));
	strncpy(OldName, player_names[a_PlayerID], MAXPLAYERNAME);
	OldNameHash = Z_Hash(player_names[a_PlayerID]);
	
	/* Copy name over */
	strncpy(player_names[a_PlayerID], a_Name, MAXPLAYERNAME);
	player_names[a_PlayerID][MAXPLAYERNAME - 1] = 0;
	NewNameHash = Z_Hash(player_names[a_PlayerID]);
	
	/* Inform? */
	if (OldNameHash != NewNameHash)
		CONL_OutputU(DSTR_NETPLAYERRENAMED, "%s%s\n", OldName, player_names[a_PlayerID]);
	
	/* Success! */
	return true;
}

/* D_NetPlayerChangedPause() -- Player changed the pause state */
bool_t D_NetPlayerChangedPause(const int32_t a_PlayerID)
{
	/* Check */
	if (a_PlayerID < 0 || a_PlayerID >= MAXPLAYERS)
		return false;
	
	/* Paused or not paused? */
	CONL_OutputU((paused ? DSTR_GAMEPAUSED : DSTR_GAMEUNPAUSED), "%s\n", player_names[a_PlayerID]);
	
	/* Success! */
	return true;
}

/*****************************************************************************/

/*** STRUCTURES ***/

/* D_NetQueueCommand_t -- Net queue */
typedef struct D_NetQueueCommand_s
{
	D_NCQCFunc_t Func;							// Function to execute
	void* Data;									// Data to pass
} D_NetQueueCommand_t;

/*** GLOBALS ***/

uint32_t g_NetStat[4] = {0, 0, 0, 0};			// Network stats
tic_t g_LastServerTic = 0;						// Server's Last tic
D_LastConsistData_t g_LastConsist = {0, 0, 0};	// Last consistency

/*** LOCALS ***/

static uint32_t l_ConnectKey[MAXCONNKEYSIZE];	// Connection Key

static D_NetQueueCommand_t** l_ComQueue = NULL;	// Command Queue
static size_t l_NumComQueue = 0;				// Number of commands

static uint32_t l_LocalStat[4];					// Local Stats

static D_NetClient_t** l_Clients = NULL;		// Networked Clients
static size_t l_NumClients = 0;					// Number of net clients

// sv_name -- Name of Server
CONL_StaticVar_t l_SVName =
{
	CLVT_STRING, NULL, CLVF_SAVE,
	"sv_name", DSTR_CVHINT_SVNAME, CLVVT_STRING, "Untitled ReMooD Server",
	NULL
};

// sv_email -- Administrator E-Mail of Server
CONL_StaticVar_t l_SVEMail =
{
	CLVT_STRING, NULL, CLVF_SAVE,
	"sv_email", DSTR_CVHINT_SVEMAIL, CLVVT_STRING, "nobody@localhost",
	NULL
};

// sv_url -- Administrator URL of Server
CONL_StaticVar_t l_SVURL =
{
	CLVT_STRING, NULL, CLVF_SAVE,
	"sv_url", DSTR_CVHINT_SVURL, CLVVT_STRING, "http://remood.org/",
	NULL
};

// sv_wadurl -- Where WADs should be downloaded from
CONL_StaticVar_t l_SVWADURL =
{
	CLVT_STRING, NULL, CLVF_SAVE,
	"sv_wadurl", DSTR_CVHINT_SVURL, CLVVT_STRING, "http://remood.org/wads/",
	NULL
};

// sv_irc -- Administrator IRC Channel of Server
CONL_StaticVar_t l_SVIRC =
{
	CLVT_STRING, NULL, CLVF_SAVE,
	"sv_irc", DSTR_CVHINT_SVIRC, CLVVT_STRING, "irc://irc.oftc.net/remood",
	NULL
};

// sv_motd -- Message Of The Day
CONL_StaticVar_t l_SVMOTD =
{
	CLVT_STRING, NULL, CLVF_SAVE,
	"sv_motd", DSTR_CVHINT_SVMOTD, CLVVT_STRING, "Welcome to ReMooD! Enjoy your stay!",
	NULL
};

// sv_connectpassword -- Password needed to connect
CONL_StaticVar_t l_SVConnectPassword =
{
	CLVT_STRING, NULL, CLVF_SAVE,
	"sv_connectpassword", DSTR_CVHINT_SVCONNECTPASSWORD, CLVVT_STRING, "",
	NULL
};

// sv_joinpassword -- Password needed to join
CONL_StaticVar_t l_SVJoinPassword =
{
	CLVT_STRING, NULL, CLVF_SAVE,
	"sv_joinpassword", DSTR_CVHINT_SVJOINPASSWORD, CLVVT_STRING, "",
	NULL
};

// sv_maxclients -- Name of Server
CONL_StaticVar_t l_SVMaxClients =
{
	CLVT_INTEGER, c_CVPVPositive, CLVF_SAVE,
	"sv_maxclients", DSTR_CVHINT_SVMAXCLIENTS, CLVVT_INTEGER, "32",
	NULL
};

// sv_readyby -- Name of Server
CONL_StaticVar_t l_SVReadyBy =
{
	CLVT_INTEGER, c_CVPVPositive, CLVF_SAVE,
	"sv_readyby", DSTR_CVHINT_SVREADYBY, CLVVT_INTEGER, "2000",
	NULL
};

// sv_maxcatchup -- Limit to the amount of time the server can catchup
CONL_StaticVar_t l_SVMaxCatchup =
{
	CLVT_INTEGER, c_CVPVPositive, CLVF_SAVE,
	"sv_maxcatchup", DSTR_CVHINT_SVMAXCATCHUP, CLVVT_INTEGER, "105",
	NULL
};

// sv_maxdemocatchup -- Similar to sv_maxcatchup, but for demos
CONL_StaticVar_t l_SVMaxDemoCatchup =
{
	CLVT_INTEGER, c_CVPVPositive, CLVF_SAVE,
	"sv_maxdemocatchup", DSTR_CVHINT_SVMAXDEMOCATCHUP, CLVVT_INTEGER, "52",
	NULL
};

/*** FUNCTIONS ***/

/* D_SyncNetAllReady() -- Indicates that all parties are ready to move to the next tic */
// It returns the tics in the future that everyone is ready to move to
tic_t D_SyncNetAllReady(void)
{
	tic_t ThisTime, DiffTime;
	int nc;
	
	/*** START BIG HACK AREA ***/
	static fixed_t CurVal, ModVal, TicsPerMS = TICSPERMS;
	
	if (!TicsPerMS)
		TicsPerMS = TICSPERMS;
	
	/* Slow */
#if 0
	if (CurVal != cv_g_gamespeed.value)
	{
		ModVal = CurVal = cv_g_gamespeed.value;
		if (ModVal < 16384)		// limit to 0.25 speed
			ModVal = 16384;
			
		ModVal = FixedDiv(1 << FRACBITS, cv_g_gamespeed.value);
		
		// Calculate new speed
		TicsPerMS = FixedMul((TICSPERMS << FRACBITS), ModVal) >> FRACBITS;
		
		if (TicsPerMS < 1)
			TicsPerMS = 1;
	}
#endif
	
	/*** END BIG HACK AREA ***/
	
	/* -timedemo */
	if (singletics)
		return 1;
	
	/* If we are the server, we dictate time */
	if (D_SyncNetIsArbiter())
	{
		// Determine if everyone is ready to play
		for (nc = 0; nc < l_NumClients; nc++)
			if (l_Clients[nc])
				if (!l_Clients[nc]->ReadyToPlay)
					return l_MapTime;
		
		// The map time is determined by the framerate
		ThisTime = (I_GetTimeMS() / TICSPERMS) - l_BaseTime;
		DiffTime = ThisTime - l_LocalTime;
		
		if (DiffTime > 0)
		{
			// Return the time difference
			l_LocalTime = ThisTime;
			
			// Catch-up limit?
			if (DiffTime > l_SVMaxCatchup.Value->Int)
				return l_SVMaxCatchup.Value->Int + 1;	// prevent zero!
			return DiffTime;
		}
		else
			return 0;
	}
	
	/* Otherwise time gets dictated to us */
	else
	{
		if (D_TicReady(gametic))
			return 1;
		return 0;
	}
	
	/* Fell through? */
	return 0;//(tic_t)-1;
}

/* D_NCAllocClient() -- Creates a new network client */
D_NetClient_t* D_NCAllocClient(void)
{
	size_t i;
	D_NetClient_t* New;
	
	/* Allocate */
	New = Z_Malloc(sizeof(*New), PU_NETWORK, NULL);
	
	/* Find free spot in list */
	for (i = 0; i < l_NumClients; i++)
		if (!l_Clients[i])
			return (l_Clients[i] = New);
	
	/* Append to end */
	Z_ResizeArray((void**)&l_Clients, sizeof(*l_Clients), l_NumClients, l_NumClients + 1);
	return (l_Clients[l_NumClients++] = New);
}

/* D_NCFindClientIsServer() -- Find client that is the server */
D_NetClient_t* D_NCFindClientIsServer(void)
{
	size_t i;
	
	/* Look through list */
	for (i = 0; i < l_NumClients; i++)
		if (l_Clients[i])
			if (l_Clients[i]->IsServer)
				return l_Clients[i];
	
	/* Not Found */
	return NULL;
}

/* D_NCFindClientByNetPlayer() -- Finds client by net player */
D_NetClient_t* D_NCFindClientByNetPlayer(D_NetPlayer_t* const a_NetPlayer)
{
	/* Check */
	if (!a_NetPlayer)
		return NULL;
	
	return NULL;
}

/* D_NCFindClientByHost() -- Finds client by host name */
D_NetClient_t* D_NCFindClientByHost(I_HostAddress_t* const a_Host)
{
	size_t i, j;
	
	/* Check */
	if (!a_Host)
		return NULL;
	
	/* Go through each client and compare the host */
	for (i = 0; i < l_NumClients; i++)
		if (l_Clients[i])
		{
			// Nothing set? And is 0.0.0.0
			if (!a_Host->IPvX && !l_Clients[i]->Address.IPvX)
				if (a_Host->Host.v4.u == 0 && l_Clients[i]->Address.Host.v4.u == 0)
					return l_Clients[i];
			
			// Port mismatch?
			if (a_Host->Port != l_Clients[i]->Address.Port)
				continue;
			
			// Host match? (v4)
			if ((a_Host->IPvX & INIPVN_IPV4) && (l_Clients[i]->Address.IPvX & INIPVN_IPV4))
				if (a_Host->Host.v4.u == l_Clients[i]->Address.Host.v4.u)
					return l_Clients[i];
			
			// Host match? (v6)
			if ((a_Host->IPvX & INIPVN_IPV6) && (l_Clients[i]->Address.IPvX & INIPVN_IPV6))
			{
				for (j = 0; j < 4; j++)
					if (a_Host->Host.v6.u[j] != l_Clients[i]->Address.Host.v6.u[j])
						break;
				
				// Matched?
				if (j >= 4)
					return l_Clients[i];
			}
		}
	
	/* Not Found */
	return NULL;
}

/* D_NCFindClientByPlayer() -- Find client by player */
D_NetClient_t* D_NCFindClientByPlayer(struct player_s* const a_Player)
{
	size_t i, j;
	
	/* Check */
	if (!a_Player)
		return NULL;
	
	/* Go through each client and compare the host */
	for (i = 0; i < l_NumClients; i++)
		if (l_Clients[i])
			for (j = 0; j < l_Clients[i]->NumArbs; j++)
				if (l_Clients[i]->Arbs[j])
					if (l_Clients[i]->Arbs[j]->Player == a_Player)
						return l_Clients[i];
	
	/* No match */
	return NULL;
}

/* D_NCFindClientByID() -- Finds client by ID */
D_NetClient_t* D_NCFindClientByID(const uint32_t a_ID)
{
	D_NetClient_t* Server;
	int i;
	
	/* Obtain server */
	Server = D_NCFindClientIsServer();
	
	/* Go through all clients to find the match */
	for (i = 0; i < l_NumClients; i++)
		if (l_Clients[i])
			if (l_Clients[i]->HostID == a_ID)
				// Return our own ID
				if (l_Clients[i]->IsLocal ||
						(!l_Clients[i]->IsLocal && Server->IsLocal))
					return l_Clients[i];
	
	/* If not a server, return the server */
	if (!Server->IsLocal)
		return Server;
	
	// Otherwise return nothing (no client)
	else
		return NULL;
}

/* D_NCGetMyHostID() -- Finds your own host ID */
uint32_t D_NCGetMyHostID(void)
{
	int i;
	
	/* Go through all clients */
	for (i = 0; i < l_NumClients; i++)
		if (l_Clients[i])
			if (l_Clients[i]->IsLocal)
				return l_Clients[i]->HostID;
	
	/* Not found */
	return 0;
}

/* D_NCFudgeOffHostStream() -- Fudge off host by stream */
void D_NCFudgeOffHostStream(I_HostAddress_t* const a_Host, struct D_BS_s* a_Stream, const char a_Code, const char* const a_Reason)
{
	int nc;
	D_NetClient_t* NetClient;
	
	/* Check */
	if (!a_Host || !a_Stream)
		return;
	
	/* Send FOFF Message */
	D_BSBaseBlock(a_Stream, "FOFF");
	D_BSwu8(a_Stream, a_Code);
	D_BSws(a_Stream, a_Reason);
	D_BSRecordNetBlock(a_Stream, a_Host);
	D_BSFlushStream(a_Stream);
	
	/* Disconnect from all streams */
	for (nc = 0; nc < l_NumClients; nc++)
	{
		// Get current
		NetClient = l_Clients[nc];
		
		// Nothing?
		if (!NetClient)
			continue;
		
		// Wrong host?
		if (!I_NetCompareHost(a_Host, &NetClient->Address))
			continue;
			
		// Server Message
		CONL_PrintF("Disconnecting %s [Code: %c, Reason: %s]...\n", NetClient->ReverseDNS, a_Code, a_Reason);
		
		// Send reliable drop
		D_BSStreamIOCtl(NetClient->PerfectStream, DRBSIOCTL_DROPHOST, a_Host);
		
		// Remove from table
		Z_Free(NetClient);
		l_Clients[nc] = NULL;
	}
}

/* D_NCFudgeOffClient() -- Disconnect client */
void D_NCFudgeOffClient(D_NetClient_t* const a_Client, const char a_Code, const char* const a_Reason)
{
	/* Check */
	if (!a_Client)
		return;
	
	/* Make sure we aren't telling ourself to fudge off */
	if (a_Client->IsLocal)
		return;
	
	/* Send FOFF Message */
	// To that particular client
	D_NCFudgeOffHostStream(&a_Client->Address, a_Client->Streams[DNCSP_PERFECTWRITE], a_Code, a_Reason);
	
	/* Send Vaporize Player */
	// To everyone else (for sync and demo usage)
}

/* DS_NetMultiComm() -- Connection multi-command */
static int DS_NetMultiComm(const uint32_t a_ArgC, const char** const a_ArgV)
{
#define BUFSIZE 128
	D_NetClient_t* ServerNC;
	I_HostAddress_t Host;
	int nc;
	D_XPlayer_t* XPlay;
	char Buf[BUFSIZE];
	
	/* Clear Host */
	memset(&Host, 0, sizeof(Host));
	
	/* List active clients */
	if (strcasecmp("listclients", a_ArgV[0]) == 0)
	{
		// Print Header
		CONL_OutputUT(CT_NETWORK, DSTR_DNETC_PLAYERLISTENT,
				"%3s%-8s%s\n",
				"Cl#",
				"ID/Host#",
				"Name/Account^Server"
			);
		CONL_OutputUT(CT_NETWORK, DSTR_DNETC_PLAYERLISTENT,
				"%3s%-8s%s\n",
				"---",
				"--------",
				"-------------------------"
			);
		
		// Go through list
		for (nc = 0; nc < g_NumXPlays; nc++)
		{
			XPlay = g_XPlays[nc];
			
			// No player?
			if (!XPlay)
				continue;
			
			// List details
			CONL_OutputUT(CT_NETWORK, DSTR_DNETC_PLAYERLISTENT,
					"%3u%-08x%s\n",
					nc,
					XPlay->ID,
					XPlay->DisplayName
				);
			
			// Account w/ Server
			snprintf(Buf, BUFSIZE, "%s^%s", XPlay->AccountName, XPlay->AccountServer);
			CONL_OutputUT(CT_NETWORK, DSTR_DNETC_PLAYERLISTENT,
					"%3s%-08x%s\n",
					"",
					XPlay->HostID,
					Buf
				);
		}
		
		// Success!
		return 0;
	}
	
	/* Start A Server */
	else if (strcasecmp("startserver", a_ArgV[0]) == 0)
	{
		D_NCServize();
		
		return 0;
	}
	
	/* Connect */
	else if (strcasecmp("connect", a_ArgV[0]) == 0)
	{
		// Not enough args?
		if (a_ArgC < 2)
		{
			CONL_PrintF("%s <address> (password) (join password)", a_ArgV[0]);
			return 1;
		}
		
		// Determination loop
		for (nc = 0; nc < l_NumClients; nc++)
			if (l_Clients[nc])
			{
				// Not local (isn't ours)
				if (!l_Clients[nc]->IsLocal)
					continue;
				
				// No net socket?
				if (!l_Clients[nc]->NetSock)
					continue;
				
				// Get hostname
				if (!I_NetNameToHost(l_Clients[nc]->NetSock, &Host, a_ArgV[1]))
				{
					CONL_OutputU(DSTR_NET_BADHOSTRESOLVE, "\n");
					continue;
				}
				
				// No port?
				if (!Host.Port)
					Host.Port = __REMOOD_BASEPORT;
		
				// Attempt connect to server
				D_NCClientize(l_Clients[nc], &Host, (a_ArgC >= 3 ? a_ArgV[2] : ""), (a_ArgC >= 4 ? a_ArgV[3] : ""));
				
				return 0;
			}
		
		return 1;
	}
	
	/* Disconnect */
	else if (strcasecmp("disconnect", a_ArgV[0]) == 0)
	{
		// Disconnect
		D_NCDisconnect(false);
		
		// Success!
		return 0;
	}
	
	/* Reconnect */
#if 0
	else if (strcasecmp("reconnect", a_ArgV[0]) == 0)
	{
		// Find server host
		ServerNC = D_NCFindClientIsServer();
		
		// No server found?
		if (!ServerNC)
			return 1;
		
		// We are the server?
		if (ServerNC->IsServer && ServerNC->IsLocal)
		{
			CONL_OutputU(DSTR_NET_RECONNECTYOUARESERVER, "\n");
			return 1;
		}
		
		// Copy host
		memmove(&Host, &ServerNC->Address, sizeof(Host));
		
		// Disconnect
		D_NCDisconnect();
		
		// Connect to server
			// TODO FIXME: Server Password
		D_NCClientize(ServerNC, &Host, NULL, NULL);
		
		// Success! I hope
		return 0;
	}
#endif
	
	/* Failure */
	return 1;
#undef BUFSIZE
}

/* D_CheckNetGame() -- Checks whether the game was started on the network */
bool_t D_CheckNetGame(void)
{
	I_NetSocket_t* Socket;
	D_NetClient_t* Client;
	bool_t ret = false;
	uint16_t i, v, PortNum, ShowPort;
	
	// I_InitNetwork sets doomcom and netgame
	// check and initialize the network driver
	
	multiplayer = false;
	
	// only dos version with external driver will return true
	netgame = false;
	if (netgame)
		netgame = false;
	
	/* Register server commands */
	// Control
	CONL_AddCommand("startserver", DS_NetMultiComm);
	CONL_AddCommand("connect", DS_NetMultiComm);
	CONL_AddCommand("disconnect", DS_NetMultiComm);
	CONL_AddCommand("reconnect", DS_NetMultiComm);
	
	CONL_AddCommand("listclients", DS_NetMultiComm);
	
	/* Register variables */
	CONL_VarRegister(&l_SVName);
	CONL_VarRegister(&l_SVEMail);
	CONL_VarRegister(&l_SVURL);
	CONL_VarRegister(&l_SVWADURL);
	CONL_VarRegister(&l_SVIRC);
	CONL_VarRegister(&l_SVMOTD);
	CONL_VarRegister(&l_SVConnectPassword);
	CONL_VarRegister(&l_SVJoinPassword);
	CONL_VarRegister(&l_SVMaxClients);
	CONL_VarRegister(&l_SVReadyBy);
	CONL_VarRegister(&l_SVMaxCatchup);
	CONL_VarRegister(&l_SVMaxDemoCatchup);
	
	/* Initial Disconnect */
	D_XNetInit();
	D_XNetDisconnect(false);
	
	return ret;
}

/* D_NCAddQueueCommand() -- Add command */
void D_NCAddQueueCommand(const D_NCQCFunc_t a_Func, void* const a_Data)
{
	size_t i;
	D_NetQueueCommand_t* NQC;	
	
	/* Add somewhere in an empty spot */
	NQC = NULL;
	for (i = 0; i < l_NumComQueue; i++)
		if (!l_ComQueue[i])
		{
			NQC = l_ComQueue[i] = Z_Malloc(sizeof(*NQC), PU_NETWORK, NULL);
			break;
		}
	
	// No Spot?
	if (!NQC)
	{
		Z_ResizeArray((void**)&l_ComQueue, sizeof(*l_ComQueue), l_NumComQueue, l_NumComQueue + 1);
		NQC = l_ComQueue[l_NumComQueue++] = Z_Malloc(sizeof(*NQC), PU_NETWORK, NULL);
	}
	
	/* Slap data in */
	NQC->Func = a_Func;
	NQC->Data = a_Data;
}

/* D_NCRunCommands() -- Run all commands in Queue */
void D_NCRunCommands(void)
{
	size_t i;
	D_NetQueueCommand_t* NQC;
	
	/* Go through each one */
	while (l_NumComQueue && l_ComQueue[0])
	{
		// Get
		NQC = l_ComQueue[0];
		
		// Check
		if (!NQC)
			break;
		
		// Execute
		NQC->Func(NQC->Data);
		
		// Wipe away
		Z_Free(NQC);
		
		// Move all down
		l_ComQueue[0] = NULL;
		memmove(l_ComQueue, l_ComQueue + 1, sizeof(*l_ComQueue) * (l_NumComQueue - 1));
		l_ComQueue[l_NumComQueue - 1] = NULL;
	}
}

/* DS_NCDoDisconnect() -- Performs actual disconnect */
static void DS_NCDoDisconnect(void* const a_Data)
{
	/* Do the disconnect */
	D_NCDisconnect(false);
}

/* D_NCQueueDisconnect() -- Queue Disconnect */
void D_NCQueueDisconnect(void)
{
	D_NCAddQueueCommand(DS_NCDoDisconnect, NULL);
}

/* D_NCDisconnect() -- Disconnect from existing server */
void D_NCDisconnect(const bool_t a_FromDemo)
{
	size_t i, j;
	
	/* Use Newer Code Instead */
	D_XNetDisconnect(a_FromDemo);
	return;
	
	/* Demos? */
	if (demoplayback)
	{
		// Don't quit when the demo stops
		singledemo = false;
		
		// Stop it
		G_StopDemoPlay();
	}
	
	/* Clear network stuff */
	// Tics
	D_ClearNetTics();
	
	// Consistency problems
	l_ConsistencyFailed = false;
	
	// Revert back to solo networking
	l_SoloNet = true;
	l_IsConnected = false;	// Set disconnected
	
	/* Clear all player information */
	// Reset all vars
	P_XGSSetAllDefaults();
	
	// Just wipe ALL of it!
	memset(players, 0, sizeof(players));
	memset(playeringame, 0, sizeof(playeringame));
	
	// If connected, reset splits
	if (l_IsConnected)
		D_NCResetSplits(a_FromDemo);
	
	// Always reset demo splits though
	D_NCResetSplits(true);
	
	// Initialize some players some
	for (i = 0; i < MAXPLAYERS; i++)
		G_InitPlayer(&players[i]);
	
	/* Destroy the level */
	P_ExClearLevel();
	
	/* Go back to the title screen */
	gamestate = GS_DEMOSCREEN;
	
	/* Clear Command Queue */
	// Don't wait stray commands being executed now!
		// TODO FIXME: Slight memory leak if the stuff in queue isn't freed
	for (i = 0; i < l_NumComQueue; i++)
		if (l_ComQueue[i])
		{
			Z_Free(l_ComQueue[i]);
			l_ComQueue[i] = NULL;
		}
	
	/* Clear non-local NetClients */
	// And tell them to fudge off
	for (i = 0; i < l_NumClients; i++)
		if (l_Clients[i])
		{
			// Modify local ID
			if (l_Clients[i]->IsLocal)
			{
				l_Clients[i]->HostID = D_CMakePureRandom();
			}
			
			// Remove arbritation list
			if (l_Clients[i]->NumArbs)
			{
				// Run through arbritation list and zap players
				for (j = 0; j < l_Clients[i]->NumArbs; j++)
					D_NCZapNetPlayer(l_Clients[i]->Arbs[j]);
				
				// Free it
				Z_Free(l_Clients[i]->Arbs);
				l_Clients[i]->Arbs = NULL;
				l_Clients[i]->NumArbs = 0;
			}
			
			// Tell remote client that the server no longer exists.
			if (!l_Clients[i]->IsLocal)
			{
				// Fudge off!
				D_NCFudgeOffClient(l_Clients[i], 'X', "Disconnected.");
			}
			
			else
			{
				// Drop host from stream
				D_BSStreamIOCtl(l_Clients[i]->PerfectStream, DRBSIOCTL_DROPHOST, &l_Clients[i]->Address);
				
				// Reset reliable stream
				D_BSStreamIOCtl(l_Clients[i]->PerfectStream, DRBSIOCTL_RELRESET, "");
			}
		}
}

/* D_NCServize() -- Turn into server */
void D_NCServize(void)
{
	int32_t i;
	
	/* Use new code instead */
	D_XNetMakeServer(false, 0);
	return;
	
	/* First Disconnect */
	// This does most of the work for us
	D_NCDisconnect(false);
	
	/* Go through local clients and set as server */
	for (i = 0; i < l_NumClients; i++)
		if (l_Clients[i])
		{
			// Not Local
			if (!l_Clients[i]->IsLocal)
				continue;
			
			// Set as server (and fully ready)
			l_Clients[i]->IsServer = true;
			l_Clients[i]->ReadyToPlay = true;
			l_Clients[i]->SaveGameSent = true;
		}
	
	/* Rebase Time */
	l_BaseTime = I_GetTimeMS() / TICSPERMS;
	l_LocalTime = 0;
	l_MapTime = 0;
	
	/* Set as connected */
	l_IsConnected = true;
	
	/* Change gamestate to waiting for player */
	gamestate = wipegamestate = GS_WAITINGPLAYERS;
	S_ChangeMusicName("D_WAITIN", 1);			// Waiting for game to start
}

/* D_NCClientize() -- Turn into client and connect to server */
void D_NCClientize(D_NetClient_t* a_BoundClient, I_HostAddress_t* const a_Host, const char* const a_Pass, const char* const a_JoinPass)
{
	size_t i;
	D_NetClient_t* Server;
	D_NetClient_t* NetClient;
	I_NetSocket_t* Socket;
	D_BS_t* Stream;
	
	/* Check */
	if (!a_BoundClient || !a_Host)
		return;
	
	/* See if already connected to this server */
	Server = D_NCFindClientByHost(a_Host);
	
	// Server was found and isn't local
	if (Server && !Server->IsLocal)
		// Compare address for a match
		if (I_NetCompareHost(&Server->Address, a_Host))
		{
			CONL_OutputU(DSTR_NET_CONNECTINGTOSAMESERVER, "\n");
			return;
		}
	
	/* First Disconnect */
	D_NCDisconnect(false);
	
	/* Revoke self serverness */
	for (i = 0; i < l_NumClients; i++)
		if (l_Clients[i])
		{
			l_Clients[i]->IsServer = false;
			l_Clients[i]->ReadyToPlay = false;
			l_Clients[i]->SaveGameSent = false;
		}
	
	/* Set non-solo */
	l_SoloNet = false;
	
	/* Create NetClient for server */
	NetClient = D_NCAllocClient();
	
	// Fill stuff in it
	memmove(&NetClient->Address, a_Host, sizeof(*a_Host));
	
	// Create stream from it
	NetClient->CoreStream = a_BoundClient->CoreStream;
	
	// Create encapsulated perfect stream
	NetClient->PerfectStream = a_BoundClient->PerfectStream;
	
	// Create streams for server connection
	NetClient->Streams[DNCSP_READ] = NULL;//NetClient->CoreStream;
	NetClient->Streams[DNCSP_WRITE] = NetClient->CoreStream;
	NetClient->Streams[DNCSP_PERFECTREAD] = NULL;//NetClient->PerfectStream;
	NetClient->Streams[DNCSP_PERFECTWRITE] = NetClient->PerfectStream;
	
	// Set as server
	NetClient->IsServer = true;
	
	/* Prepare for connection to server */
	gamestate = wipegamestate = GS_WAITFORJOINWINDOW;
	
	/* Send connection command to server */
	// Send perfect write packets
	Stream = NetClient->Streams[DNCSP_PERFECTWRITE];
	
	// Write out the data
	D_BSBaseBlock(Stream, "CONN");
	
	// Key -- Generate and send
	for (i = 0; i < MAXCONNKEYSIZE; i++)
	{
		l_ConnectKey[i] = D_CMakePureRandom();
		D_BSwu32(Stream, l_ConnectKey[i]);
	}
	
	// Write version
	D_BSwu8(Stream, VERSION);
	D_BSwu8(Stream, REMOOD_MAJORVERSION);
	D_BSwu8(Stream, REMOOD_MINORVERSION);
	D_BSwu8(Stream, REMOOD_RELEASEVERSION);
	
	// Passwords
	D_BSws(Stream, (a_Pass ? a_Pass : ""));
	D_BSws(Stream, (a_JoinPass ? a_JoinPass : ""));
	
	// Send to server
	D_BSRecordNetBlock(Stream, a_Host);
	
	// Flush streams
	D_BSFlushStream(NetClient->Streams[DNCSP_WRITE]);
	D_BSFlushStream(NetClient->Streams[DNCSP_PERFECTWRITE]);
	
	/* Print message to avid player */
	CONL_OutputU(DSTR_NET_CONNECTINGTOSERVER, "\n");
}

/* D_NCHostOnBanList() -- Checks whether a host is on your banlist */
bool_t D_NCHostOnBanList(I_HostAddress_t* const a_Host)
{
	/* Check */
	if (!a_Host)
		return false;
	
	/* No ban found */
	return false;
}

/*****************************************************************************/

/* D_NCZapNetPlayer() -- Zap net player */
void D_NCZapNetPlayer(struct D_NetPlayer_s* const a_Player)
{
	/* Check */
	if (!a_Player)
		return;
}

/* D_NCReqPrefChange() -- Request profile settings */
void D_NCReqPrefChange(struct D_ProfileEx_s* a_Profile, struct B_BotTemplate_s* a_Bot, const uint32_t a_Player)
{
	D_NetClient_t* Server;
	D_BS_t* Stream;
	
	uint8_t outColor;
	const char* outName;
	const char* outWPref;
	const char* outHexClass;
	fixed_t outCamDist, outCamHeight, outCamSpeed;
	
	/* Check */
	if (a_Player < 0 || a_Player >= MAXPLAYERS || (!a_Profile && !a_Bot))
		return;
	
	/* Find server to send request to */
	Server = D_NCFindClientIsServer();
	
	// Not found?
	if (!Server)
		return;
	
	/* Send Bot Info? */
	if (a_Bot)
	{
		outName = a_Bot->DisplayName;
		outColor = a_Bot->SkinColor;
		outCamDist = 128 << FRACBITS;
		outCamHeight = 20 << FRACBITS;
		outCamSpeed = 16384;
		outWPref = a_Bot->WeaponOrder;
		outHexClass = a_Bot->HexenClass;
	}
	
	/* Send Player Info? */
	else
	{
		outName = a_Profile->DisplayName;
		outColor = a_Profile->Color;
		outCamDist = a_Profile->CamDist;
		outCamHeight = a_Profile->CamHeight;
		outCamSpeed = a_Profile->CamSpeed;
		outWPref = "";
		outHexClass = "Fighter";
	}
	
	/* Tell server to add player */
	// Use server stream
	Stream = Server->Streams[DNCSP_PERFECTWRITE];
	
	// Put Data
	D_BSBaseBlock(Stream, "PREF");
	
	D_BSwu8(Stream, a_Player);
	D_BSwu8(Stream, outColor);
	D_BSwi32(Stream, outCamDist);
	D_BSwi32(Stream, outCamHeight);
	D_BSwi32(Stream, outCamSpeed);
	D_BSws(Stream, (outName ? outName : ""));
	D_BSws(Stream, (outHexClass ? outHexClass : ""));
	D_BSws(Stream, (outWPref ? outWPref : ""));
	
	D_BSRecordNetBlock(Stream, &Server->Address);
}

/* D_NCReqVarChange() -- Request variable change */
void D_NCReqVarChange(const uint32_t a_Code, const int32_t a_NewVal)
{
	D_NetClient_t* Server;
	D_BS_t* Stream;
	
	/* Find server to send request to */
	Server = D_NCFindClientIsServer();
	
	// Not found?
	if (!Server)
		return;
	
	/* Tell server to add player */
	// Use server stream
	Stream = Server->Streams[DNCSP_PERFECTWRITE];
	
	// Put Data
	D_BSBaseBlock(Stream, "GVAR");
	D_BSwu32(Stream, a_Code);
	D_BSwi32(Stream, a_NewVal);
	D_BSRecordNetBlock(Stream, &Server->Address);
}

/* D_NCLocalPlayerAdd() -- Adds a local player */
void D_NCLocalPlayerAdd(const char* const a_Name, const bool_t a_Bot, const uint32_t a_JoyID, const int8_t a_ScreenID, const bool_t a_UseJoy)
{
	uint32_t ProcessID, PlaceAt;
	D_ProfileEx_t* Profile;
	const B_BotTemplate_t* BotTemplate;
	D_NetClient_t* Server;
	D_BS_t* Stream;
		
	/* Only servers are allowed to add bots */
	if (!D_SyncNetIsArbiter() && a_Bot)
		return;
	
	/* Find first free slot */
	if (!a_Bot)
	{
		for (PlaceAt = 0; PlaceAt < MAXSPLITSCREEN; PlaceAt++)
			// If nobody is waiting here (also ignore active in demos)
			if (!D_ScrSplitHasPlayer(PlaceAt))
				break;
	
		// No Room?
		if (PlaceAt >= MAXSPLITSCREEN)
			return;
	}
	
	/* Create Process ID */
	do
	{
		ProcessID = D_CMakePureRandom();
	} while (!ProcessID || D_NCSFindNetPlayerByProcess(ProcessID) || D_NCSFindSplitByProcess(ProcessID) >= 0);
	
	/* If not a bot, bind to a local screen */
	if (!a_Bot)
	{
		// Find Profile
		Profile = D_FindProfileEx(a_Name);
		
		if (!Profile)
			Profile = D_FindProfileEx("guest");
		
		if (!demoplayback)
		{
			g_Splits[PlaceAt].Active = false;
			g_Splits[PlaceAt].Console = 0;
			g_Splits[PlaceAt].Display = 0;
		}
		
		g_Splits[PlaceAt].RequestSent = false;
		g_Splits[PlaceAt].GiveUpAt = 0;
		
		// Bind stuff here
		g_Splits[PlaceAt].Waiting = true;
		//g_Splits[PlaceAt].Profile = Profile;
		g_Splits[PlaceAt].JoyBound = a_UseJoy;
		g_Splits[PlaceAt].JoyID = a_JoyID;
		g_Splits[PlaceAt].ProcessID = ProcessID;
		
		// Resize Splits
		if (!demoplayback)
		{
			g_SplitScreen++;
			R_ExecuteSetViewSize();
		}
	}
	
	/* Otherwise it "is not bound to a slot */
	else
	{
		// Find template
		BotTemplate = NULL;
		if (a_Name)
			BotTemplate = B_GHOST_FindTemplate(a_Name);
		
		// Not found?
		if (!BotTemplate)
			BotTemplate = B_GHOST_RandomTemplate();
		
		/* Find server to send request to */
		Server = D_NCFindClientIsServer();
	
		// Not found?
		if (!Server)
			return;
	
		/* Tell server to add player */
		// Use server stream
		Stream = Server->Streams[DNCSP_PERFECTWRITE];
		
		// Base
		D_BSBaseBlock(Stream, "JOIN");
		
		// Write
		D_BSwu8(Stream, a_Bot);
		D_BSws(Stream, BotTemplate->AccountName);
		D_BSwu32(Stream, BotTemplate->BotIDNum);
		D_BSwu32(Stream, 0);	// No process ID
		D_BSws(Stream, BotTemplate->AccountName);
		D_BSws(Stream, BotTemplate->DisplayName);
		D_BSwu8(Stream, BotTemplate->SkinColor);
		D_BSws(Stream, BotTemplate->HexenClass);
		
		// Send
		D_BSRecordNetBlock(Stream, &Server->Address);
	}
	
	/* Add resume menu time */
	// i.e. HOM when adding a player in the menu
	g_ResumeMenu++;
}

/*****************************************************************************/

/* D_NCMessageFlag_t -- Message Flags */
typedef enum D_NCMessageFlag_e
{
	DNCMF_NORMAL					= 0x001,	// Accept: Non-Perfect
	DNCMF_PERFECT					= 0x002,	// Accept: Perfect
	DNCMF_CLIENT					= 0x004,	// Client Accepted (when joined)
	DNCMF_SERVER					= 0x008,	// Server Accepted (when hosting)
	DNCMF_HOST						= 0x010,	// Only accepted by game host (arb)
	DNCMF_DEMO						= 0x020,	// Saved to demos
	DNCMF_REMOTECL					= 0x040,	// Remote client must exist
} D_NCMessageFlag_t;

struct D_NCMessageData_s;
typedef bool_t (*D_NCMessageHandlerFunc_t)(struct D_NCMessageData_s* const a_Data);

/* D_NCMessageType_t -- Message Type to handle */
typedef struct D_NCMessageType_s
{
	int8_t Valid;								// Valid
	char Header[5];								// Message Header
	D_NCMessageHandlerFunc_t Func;				// Handler Func
	D_NCMessageFlag_t Flags;					// Flags
} D_NCMessageType_t;

/* D_NCMessageData_t -- Message Data */
typedef struct D_NCMessageData_s
{
	const D_NCMessageType_t* Type;				// Type of message
	D_NetClient_t* NetClient;					// Client it is from
	D_NetClient_t* RCl;							// Attached Remote Client
	D_BS_t* InStream;					// Stream to read from
	D_BS_t* OutStream;				// Stream to write to
	I_HostAddress_t* FromAddr;					// Address message is from
	uint32_t FlagsMask;							// Mask for flags
} D_NCMessageData_t;

/* D_NetTicData_t -- Network tic data */
typedef struct D_NetTicData_s
{
	tic_t RunAt;								// Run on this tic
	uint8_t PRandom;							// P_Random() Index
	uint32_t PosMask;							// In game player position mask
	ticcmd_t Data[MAXPLAYERS + 1];				// Tic Data
} D_NetTicData_t;

/*** LOCALS ***/

#define MAXGLOBALBUFSIZE					32	// Size of global buffer

static tic_t l_GlobalTime[MAXGLOBALBUFSIZE];	// Time
static ticcmd_t l_GlobalBuf[MAXGLOBALBUFSIZE];	// Global buffer
static int32_t l_GlobalAt = -1;					// Position Global buf is at
static D_NetTicData_t** l_NetTicBuf;			// Buffer
static size_t l_NetTicBufSize;					// Size of buffer
static ticcmd_t l_StoreCmds[2][MAXPLAYERS + 1];	// Stored server commands
static D_NetTicData_t l_ClientRunTic;			// Tic for client to run

/*** PACKET HANDLER FUNCTIONS ***/

/* DS_GrabGlobal() -- Grabs the next global command */
static ticcmd_t* DS_GrabGlobal(const uint8_t a_ID, const int32_t a_NeededSize, void** const a_Wp)
{
	ticcmd_t* Placement;
	void* Wp;
	
	/* Clear */
	Placement = NULL;
	
	/* Determine */
	if (l_GlobalAt < MAXGLOBALBUFSIZE - 1)
	{
		// Append to last global tic command, if possible
		if (l_GlobalAt >= 0)
		{
			if (l_GlobalBuf[l_GlobalAt].Ext.DataSize < MAXTCDATABUF - (a_NeededSize + 2))
				Placement = &l_GlobalBuf[l_GlobalAt];
			else
			{
				// Write Commands Here
				Placement = &l_GlobalBuf[++l_GlobalAt];
		
				// Clear it
				memset(Placement, 0, sizeof(*Placement));
		
				// Set as extended
				Placement->Ctrl.Type = 1;
			}
		}
		
		// Otherwise eat first spot
		else
		{
			Placement = &l_GlobalBuf[++l_GlobalAt];
			Placement->Ctrl.Type = 1;
		}
	}
	
	/* Worked? */
	if (Placement)
	{
		*a_Wp = &Placement->Ext.DataBuf[Placement->Ext.DataSize];
		WriteUInt8((uint8_t**)a_Wp, a_ID);
		l_GlobalBuf[l_GlobalAt].Ext.DataSize += a_NeededSize + 1;
	}
	
	/* Return it */
	return Placement;
}

/* D_NCMH_GVAR() -- Change Variable */
bool_t D_NCMH_GVAR(struct D_NCMessageData_s* const a_Data)
{
	ticcmd_t* Placement;
	D_NetClient_t* ServerNC;
	D_BS_t* Stream;
	uint32_t Code;
	int32_t NewVal;
	uint8_t* Wp;
	
	/* Get server client */
	ServerNC = D_NCFindClientIsServer();
	
	// We are not the server?
	if (!ServerNC->IsLocal)
		return true;
	
	/* Remote client is not ready */
	if (!a_Data->RCl->ReadyToPlay || !a_Data->RCl->SaveGameSent)
		return true;
	
	/* Currently only allow the server to change it */
	if (!a_Data->RCl->IsLocal)
		return true;
	
	/* Get Stream */
	Stream = a_Data->InStream;
	
	/* Read Data */
	Code = D_BSru32(Stream);
	NewVal = D_BSri32(Stream);
	
	/* Encode in command */
	Placement = DS_GrabGlobal(DTCT_GAMEVAR, c_TCDataSize[DTCT_GAMEVAR], &Wp);
	
	if (Placement)
	{
		// Fill in data
		LittleWriteUInt32((uint32_t**)&Wp, Code);
		LittleWriteUInt16((uint32_t**)&Wp, NewVal);
	}
	
	/* Done processing */
	return true;
}

/* D_NCMH_JOIN() -- Player wants to join */
bool_t D_NCMH_JOIN(struct D_NCMessageData_s* const a_Data)
{
	ticcmd_t* Placement;
	D_NetClient_t* ServerNC;
	D_NetPlayer_t* NetPlayer;
	D_BS_t* Stream;
	uint8_t UUID[(MAXPLAYERNAME * 2) + 1];
	uint8_t PName[MAXPLAYERNAME], AName[MAXPLAYERNAME];
	uint8_t HexClass[MAXPLAYERNAME], Color;
	int16_t FreeSlot;
	uint32_t PInstance, SplitInstance, UniqueID;
	void* Wp;
	int i, j;
	
	uint8_t IsBot;
	uint32_t JoinFlags;
	B_BotTemplate_t* BotTemplate;
	
	/* Get server client */
	ServerNC = D_NCFindClientIsServer();
	
	// We are not the server?
	if (!ServerNC->IsLocal)
		return true;
	
	/* Remote client is not ready */
	if (!a_Data->RCl->ReadyToPlay || !a_Data->RCl->SaveGameSent)
		return true;
	
	/* Get Stream */
	Stream = a_Data->InStream;
	
	// Read Data
	IsBot = D_BSru8(Stream);
	D_BSrs(Stream, UUID, (MAXPLAYERNAME * 2) + 1);
	PInstance = D_BSru32(Stream);
	SplitInstance = D_BSru32(Stream);
	D_BSrs(Stream, AName, MAXPLAYERNAME);
	D_BSrs(Stream, PName, MAXPLAYERNAME);
	
	// Added 2012/09/05
	Color = D_BSru8(Stream);
	D_BSrs(Stream, HexClass, MAXPLAYERNAME);
	
	/* Only the server can add bots */
	if (IsBot && !a_Data->RCl->IsServer)
		return true;
	
	/* Non-Bot: Check client max splits */
	NetPlayer = NULL;
	j = 0;
	if (!IsBot)
	{
		// Count non-spectators
		for (i = 0; i < a_Data->RCl->NumArbs; i++)
			if (a_Data->RCl->Arbs[i])
				if (a_Data->RCl->Arbs[i]->Type != DNPT_SPECTATOR)
					j++;
				
				// Is a spectator, claim it
				else if (!NetPlayer)
					NetPlayer = a_Data->RCl->Arbs[i];
	}
	
	/* Find Free Player Slot */
	FreeSlot = -1;
	for (i = 0; i < MAXPLAYERS; i++)
		if (!playeringame[i])
		{
			FreeSlot = i;
			break;
		}
	
	// No slot?
	if (FreeSlot == -1)
		return true;
	
	/* Generate Unique ID */
	// This links players and spectators together, etc.
		// No player, or there is already one, create new ID for spec
	if (!NetPlayer || j > 1)
		do
		{
			UniqueID = D_CMakePureRandom();
		} while (!UniqueID || D_NCSFindNetPlayerByUnique(UniqueID));
		
		// Otherwise, use the spectators unique ID
	else
	{
		UniqueID = NetPlayer->UniqueID;
		
		// Set as networked
		NetPlayer->Type = DNPT_NETWORK;
	}
	
	/* Place */
	Wp = NULL;
	Placement = DS_GrabGlobal(DTCT_JOIN, c_TCDataSize[DTCT_JOIN], &Wp);
	
	if (Placement)
	{
		// Setup Flags
		JoinFlags = 0;
		
		if (IsBot)
			JoinFlags |= DTCJF_ISBOT;
		
		JoinFlags = LittleSwapUInt32(JoinFlags);
		
		// Fill in data
		LittleWriteUInt32((uint32_t**)&Wp, a_Data->RCl->HostID);
		LittleWriteUInt16((uint16_t**)&Wp, FreeSlot);
		LittleWriteUInt32((uint32_t**)&Wp, JoinFlags);
		LittleWriteUInt32((uint32_t**)&Wp, PInstance);
		LittleWriteUInt32((uint32_t**)&Wp, SplitInstance);
		LittleWriteUInt32((uint32_t**)&Wp, UniqueID);
		WriteUInt8((uint8_t**)&Wp, Color);
		
		for (i = 0; i < MAXPLAYERNAME; i++)
			WriteUInt8((uint8_t**)&Wp, AName[i]);
		
		for (i = 0; i < MAXPLAYERNAME; i++)
			WriteUInt8((uint8_t**)&Wp, HexClass[i]);
	}
	
	/* Do not continue */
	return true;
}

tic_t g_WatchTic = 0;

/* D_NCMH_TICS() -- Recieved player tics */
bool_t D_NCMH_TICS(struct D_NCMessageData_s* const a_Data)
{
	uint64_t GameTic;
	D_BS_t* Stream;
	D_NetTicData_t* Data;
	int i, j, Blank;
	ticcmd_t* Target;
	
	uint8_t u8;
	uint16_t u16, BufSize, DiffBits;
	
	/* Ignore any tic commands from the local server */
	//if (a_Data->NetClient->IsLocal)
	//	return false;
		
	/* Get Stream */
	Stream = a_Data->InStream;
	
	/* Read Data */
	// Game tic
	GameTic = D_BSru64(Stream);
	
	// Old tic (probably a dupe or old retransmit)
	if (GameTic < gametic)
		return true;
	
	/* See if this tic was never buffered */
	Blank = -1;	// Find blank spot while we are at it
	for (i = 0; i < l_NetTicBufSize; i++)
		if (l_NetTicBuf[i])
		{
			if (l_NetTicBuf[i]->RunAt == GameTic)
				return true;	// Same
		}
		else
			Blank = i;
	
	/* No blank? */
	if (Blank == -1)
	{
		Z_ResizeArray((void**)&l_NetTicBuf, sizeof(*l_NetTicBuf),
			l_NetTicBufSize, l_NetTicBufSize + 1);
		Blank = l_NetTicBufSize++;
	}
	
	/* Create in spot */
	l_NetTicBuf[Blank] = Data = Z_Malloc(sizeof(*Data), PU_STATIC, NULL);
	
	/* Read packet data into storage */
	// Set current tic
	Data->RunAt = GameTic;
	
	// Tic buffer size
	BufSize = D_BSru16(Stream);
	
	/* Read consistency info */
	Data->PRandom = D_BSru8(Stream);
	Data->PosMask = D_BSru32(Stream);
	
	/* Read global commands */
	Data->Data[MAXPLAYERS].Ctrl.Type = 1;
	u16 = D_BSru16(Stream);
	if (u16)
		Data->Data[MAXPLAYERS].Ext.DataSize = u16;
	
	for (i = 0; i < u16; i++)
	{
		u8 = D_BSru8(Stream);
		if (i < MAXTCDATABUF)
			Data->Data[MAXPLAYERS].Ext.DataBuf[i] = u8;
	}
	
	/* Per-Player Commands */
	for (i = 0; i < MAXPLAYERS; i++)
	{
		// Determine target
		Target = &Data->Data[i];
		
		// Get playeringame status
		u8 = D_BSru8(Stream);
		
		// Not in game, don't bother
		if (!u8)
			continue;
		
		// Read Timing Code
		Target->Ctrl.ProgramTic = D_BSru64(Stream);
		Target->Ctrl.GameTic = D_BSru64(Stream);
		Target->Ctrl.Ping = D_BSru16(Stream);
		
		// Read Diff Bits
		DiffBits = D_BSru16(Stream);
		
		if (DiffBits & DDB_FORWARD)
			Target->Std.forwardmove = D_BSri8(Stream);
		if (DiffBits & DDB_SIDE)
			Target->Std.sidemove = D_BSri8(Stream);
		if (DiffBits & DDB_ANGLE)
			Target->Std.angleturn = D_BSri16(Stream);
		if (DiffBits & DDB_AIMING)
			Target->Std.aiming = D_BSru16(Stream);
		if (DiffBits & DDB_BUTTONS)
			Target->Std.buttons = D_BSru16(Stream);
		if (DiffBits & DDB_RESETAIM)
			Target->Std.ResetAim = D_BSru8(Stream);
		if (DiffBits & DDB_INVENTORY)
			Target->Std.InventoryBits = D_BSru8(Stream);
		if (DiffBits & DDB_STATFLAGS)
			Target->Std.StatFlags = D_BSru32(Stream);
		
		if (DiffBits & DDB_WEAPON)
		{
			j = 0;
			do
			{
				u8 = D_BSru8(Stream);
				if (j < MAXTCWEAPNAME)
					Target->Std.XSNewWeapon[j++] = u8;
			} while (u8);
		}
		
		// Data bits
		u16 = D_BSru16(Stream);
		if (u16)
			Target->Std.DataSize = u16;

		for (j = 0; j < u16; j++)
		{
			u8 = D_BSru8(Stream);
			if (i < MAXTCDATABUF)
				Target->Std.DataBuf[j] = u8;
		}
	}
	
	/* No more handling */
	return true;
}

/* D_NCMH_TCMD() -- Tic Command */
bool_t D_NCMH_TCMD(struct D_NCMessageData_s* const a_Data)
{
	D_NetClient_t* ServerNC;
	D_BS_t* Stream;
	int SplitScreen, i, Player;
	ticcmd_t* WriteTo;
	ticcmd_t Garbage;
	uint64_t ProgramTic;
	tic_t GameTic;
	
	/* Get server client */
	ServerNC = D_NCFindClientIsServer();
	
	// We are not the server?
	if (!ServerNC->IsLocal)
		return true;
	
	/* Remote client is not ready */
	if (!a_Data->RCl->ReadyToPlay || !a_Data->RCl->SaveGameSent)
		return true;
	
	/* Get Stream */
	Stream = a_Data->InStream;
	
	/* Read Timing Data */
	GameTic = D_BSru64(Stream);
	ProgramTic = D_BSru64(Stream);
	
	// Older tic?
	if (ProgramTic <= a_Data->RCl->ProgramTic)
		return true;
	
	a_Data->RCl->GameTic = GameTic;
	a_Data->RCl->ProgramTic = ProgramTic;
	a_Data->RCl->Consist.GameTic = D_BSru64(Stream);
	a_Data->RCl->Consist.PrIndex = D_BSru8(Stream);
	a_Data->RCl->Consist.PosMask = D_BSru32(Stream);
	
	/* Read Split Count */
	SplitScreen = D_BSri8(Stream);
	
	/* Read input commands */
	for (i = 0; i < SplitScreen + 1; i++)
	{
		// Something not here?
		if (!D_BSru8(Stream))
			continue;
		
		// Read player to control
		Player = D_BSru8(Stream);
		
		// Check control of player
		if (Player >= 0 && Player < MAXPLAYERS && players[Player].NetPlayer &&
			players[Player].NetPlayer->NetClient == a_Data->RCl &&
			players[Player].NetPlayer->TicTotal < MAXDNETTICCMDCOUNT - 1)
			WriteTo = &players[Player].NetPlayer->TicCmd[players[Player].NetPlayer->TicTotal = 1];
		else
			WriteTo = &Garbage;
		
		// Read Timing
		WriteTo->Ctrl.ProgramTic = D_BSru64(Stream);
		WriteTo->Ctrl.GameTic = D_BSru64(Stream);
		WriteTo->Ctrl.Ping = D_BSru16(Stream);
		
		// Fake player ping here, ouch
		WriteTo->Ctrl.Ping = (gametic - WriteTo->Ctrl.GameTic) * TICSPERMS;
		
		// Read Stuff
		WriteTo->Std.forwardmove = D_BSri8(Stream);
		WriteTo->Std.sidemove = D_BSri8(Stream);
		WriteTo->Std.angleturn = D_BSri16(Stream);
		WriteTo->Std.aiming = D_BSru16(Stream);
		WriteTo->Std.buttons = D_BSru16(Stream);
		WriteTo->Std.BaseAngleTurn = D_BSri16(Stream);
		WriteTo->Std.BaseAiming = D_BSri16(Stream);
		WriteTo->Std.ResetAim = D_BSru8(Stream);
		WriteTo->Std.InventoryBits = D_BSru8(Stream);
		WriteTo->Std.StatFlags = D_BSru32(Stream);
		D_BSrs(Stream, WriteTo->Std.XSNewWeapon, MAXTCWEAPNAME);
	}

	/* Done handling */
	return true;
}

/* D_NCMH_CONN() -- Connection Request */
bool_t D_NCMH_CONN(struct D_NCMessageData_s* const a_Data)
{
#define BUFSIZE 64
	int i, c;
	uint32_t ConnectKey[MAXCONNKEYSIZE], HostID;
	D_NetClient_t* ServerNC, *FreshClient;
	D_BS_t* Stream;
	uint8_t Ver[4];
	uint32_t KeyMask;
	char ServerPass[BUFSIZE], JoinPass[BUFSIZE];
	
	/* Get server client */
	ServerNC = D_NCFindClientIsServer();
	
	// No server?
	if (!ServerNC)
		return true;
	
	// Not the server?
	if (!ServerNC->IsLocal)
		return true;
	
	/* Get Stream */
	Stream = a_Data->InStream;
	
	/* Read Data */
	// Key
	for (i = 0; i < MAXCONNKEYSIZE; i++)
		ConnectKey[i] = D_BSru32(Stream);
	
	// Version
	for (i = 0; i < 4; i++)
		Ver[i] = D_BSru8(Stream);
	
	// Passwords
	D_BSrs(Stream, ServerPass, BUFSIZE);
	D_BSrs(Stream, JoinPass, BUFSIZE);
	
	/* Version mismatch? */
	if (Ver[1] != REMOOD_MAJORVERSION || Ver[2] != REMOOD_MINORVERSION ||
		Ver[3] != REMOOD_RELEASEVERSION)
	{
	}
	
	/* Too many clients? */
	for (i = 0, c = 0; i < l_NumClients; i++)
		if (l_Clients[i])
			if (!l_Clients[i]->IsLocal)
				c++;
	
	// Too many now?
	if (c >= l_SVMaxClients.Value->Int)
	{
	}
	
	/* Password Mismatch */
	if (strlen(l_SVConnectPassword.Value->String) > 0)
		if (strcasecmp(l_SVConnectPassword.Value->String, ServerPass) != 0)
		{
		}
	
	/* Add to clients */
	FreshClient = D_NCAllocClient();
	
	// Set non-solo
	l_SoloNet = false;
	
	// Info
	memmove(&FreshClient->Address, a_Data->FromAddr, sizeof(I_HostAddress_t));
	FreshClient->CoreStream = a_Data->NetClient->CoreStream;
	FreshClient->PerfectStream = a_Data->NetClient->PerfectStream;
	FreshClient->IsLocal = false;
	FreshClient->IsServer = false;
	memmove(FreshClient->Key, ConnectKey, sizeof(FreshClient->Key));
	
	do
	{
		HostID = D_CMakePureRandom();
	} while (D_NCFindClientByID(HostID));
	FreshClient->HostID = HostID;
	FreshClient->ReadyTimeout = I_GetTimeMS() + l_SVReadyBy.Value->Int;
	
	// Streams
	FreshClient->Streams[DNCSP_WRITE] = a_Data->NetClient->Streams[DNCSP_WRITE];
	FreshClient->Streams[DNCSP_PERFECTWRITE] = a_Data->NetClient->Streams[DNCSP_PERFECTWRITE];
	
	// Reverse DNS
	I_NetHostToName(NULL, &FreshClient->Address, FreshClient->ReverseDNS, NETCLIENTRHLEN);
	
	/* Tell client their host information */
	// Write message to them (perfect output)
	Stream = FreshClient->Streams[DNCSP_PERFECTWRITE];
	
	// Base
	D_BSBaseBlock(Stream, "PLAY");
	
	// Generate server side key to authenticate client with
	KeyMask = 0;
	for (i = 0; i < MAXCONNKEYSIZE; i++)
	{
		FreshClient->GenKey[i] = D_CMakePureRandom();
		D_BSwu32(Stream, FreshClient->GenKey[i]);
		
		// Mask key (client verification)
		KeyMask ^= ConnectKey[i];
	}
	
	// Info
	D_BSwu32(Stream, KeyMask);
	D_BSwu64(Stream, gametic + 1);				// Current game tic
	D_BSwu32(Stream, HostID);					// Their server identity
	D_BSwu8(Stream, P_GetRandIndex());			// Random Index
	
	// Send away
	D_BSRecordNetBlock(Stream, &FreshClient->Address);
	
	/* Info */
	CONL_OutputU(DSTR_DNETC_CONNECTFROM, "%s%i%i%i%c\n",
			"TODO FIXME: Implement name lookup here",
			Ver[1], Ver[2], Ver[3]
		);
	
	return true;
#undef BUFSIZE
}

/* D_NCMH_PLAY() -- We can play */
bool_t D_NCMH_PLAY(struct D_NCMessageData_s* const a_Data)
{
	D_NetClient_t* ServerNC;
	D_BS_t* Stream;
	int i;
	uint32_t SelfMask, CalcedMask;
	uint32_t ServerGenKey[MAXCONNKEYSIZE], HostID;
	uint64_t GameTic;
	uint8_t PrIndex;
	
	/* Get server client */
	ServerNC = D_NCFindClientIsServer();
	
	// We are the server?
	if (ServerNC->IsLocal)
		return true;
	
	// Server already ready?
	if (ServerNC->ReadyToPlay)
		return true;
	
	/* Get Stream */
	Stream = a_Data->InStream;
	
	/* Read in data */
	// Key
	CalcedMask = 0;
	for (i = 0; i < MAXCONNKEYSIZE; i++)
	{
		ServerGenKey[i] = D_BSru32(Stream);
		CalcedMask ^= l_ConnectKey[i];
	}
	SelfMask = D_BSru32(Stream);
	
	// Servers sent back mask does not match (probably hacked request)
	if (SelfMask != CalcedMask)
		return true;
	
	// Read Others
	GameTic = D_BSru64(Stream);
	HostID = D_BSru32(Stream);
	PrIndex = D_BSru8(Stream);
	gamestate = GS_WAITINGPLAYERS;
	
	// Set random index
	P_SetRandIndex(PrIndex);
	
	/* Local local ID */
	for (i = 0; i < l_NumClients; i++)
		if (l_Clients[i])
			if (l_Clients[i]->IsLocal)
				l_Clients[i]->HostID = HostID;
			else
				l_Clients[i]->HostID = NULL;
	
	/* Set the game tic and timing stuff */
	gametic = GameTic;
	
	/* Send REDY packet to server */
	// Get stream
	Stream = ServerNC->Streams[DNCSP_PERFECTWRITE];
	
	// Write to server, that we are ready (and send both keys!
	D_BSBaseBlock(Stream, "REDY");
	for (i = 0; i < MAXCONNKEYSIZE; i++)
	{
		D_BSwu32(Stream, ServerGenKey[i]);
		D_BSwu32(Stream, l_ConnectKey[i]);
	}
	D_BSRecordNetBlock(Stream, &ServerNC->Address);
	
	// Set ready
	ServerNC->ReadyToPlay = true;
	
	// TODO FIXME: set this after savegame sent
	l_IsConnected = true;
	
	/* Done processing */
	return true;
}

/* D_NCMH_REDY() -- We can play */
// Client ready to play
bool_t D_NCMH_REDY(struct D_NCMessageData_s* const a_Data)
{
	void* Wp;
	ticcmd_t* Placement;
	D_NetClient_t* ServerNC;
	D_BS_t* Stream;
	int i;
	uint32_t ServerGenKey[MAXCONNKEYSIZE], ClientGenKey[MAXCONNKEYSIZE];
	uint32_t UniqueID;
	D_NetPlayer_t* NetPlayer;
	
	/* Get server client */
	ServerNC = D_NCFindClientIsServer();
	
	/* Get Stream */
	Stream = a_Data->InStream;
	
	/* Read in data */
	// Key
	for (i = 0; i < MAXCONNKEYSIZE; i++)
	{
		ServerGenKey[i] = D_BSru32(Stream);
		ClientGenKey[i] = D_BSru32(Stream);
		
		if (ClientGenKey[i] != a_Data->RCl->Key[i] ||
			ServerGenKey[i] != a_Data->RCl->GenKey[i])
		{
			D_NCFudgeOffClient(a_Data->RCl, 'K', "Key authorization mismatch.");
			return true;
		}
	}
	
	/* Ready! */
	a_Data->RCl->ReadyToPlay = true;
	CONL_OutputU(DSTR_DNETC_CLIENTNOWREADY, "%s\n", a_Data->RCl->ReverseDNS);
	
	// TODO FIXME: Actually send a save game
	a_Data->RCl->SaveGameSent = true;
	
	/* Create Initial Spectator */
	// UUID For it
	do
	{
		UniqueID = D_CMakePureRandom();
	} while (!UniqueID || D_NCSFindNetPlayerByUnique(UniqueID));
	
	// Create NetPlayer
	NetPlayer = D_NCSAllocNetPlayer();
	
	// Add to arbitration
	for (i = 0; i < a_Data->RCl->NumArbs; i++)
		if (!a_Data->RCl->Arbs[i])
			break;
	
	// No Room?
	if (i >= a_Data->RCl->NumArbs)
	{
		Z_ResizeArray((void**)&a_Data->RCl->Arbs, sizeof(*a_Data->RCl->Arbs),
					a_Data->RCl->NumArbs, a_Data->RCl->NumArbs + 1);
		a_Data->RCl->NumArbs++;
	}
	
	// Place there
	a_Data->RCl->Arbs[i] = NetPlayer;
	
	// Setup as spectator
	NetPlayer->Type = DNPT_SPECTATOR;
	NetPlayer->UniqueID = UniqueID;
	
	// Setup Name
	strncpy(NetPlayer->AccountName, "NoAccount", MAXPLAYERNAME);
	
	/* Queue Add Spectator to clients */
	/* Place */
	Wp = NULL;
	Placement = DS_GrabGlobal(DTCT_ADDSPEC, c_TCDataSize[DTCT_ADDSPEC], &Wp);
	
	if (Placement)
	{
		// Write Unique ID_
		LittleWriteUInt32((uint32_t**)&Wp, UniqueID);
		LittleWriteUInt32((uint32_t**)&Wp, a_Data->RCl->HostID);
		
		// Write Account Name
		for (i = 0; i < MAXPLAYERNAME; i++)
			WriteUInt8((uint8_t**)&Wp, NetPlayer->AccountName[i]);
	}
	
	/* Done processing */
	return true;
}

// c_NCMessageCodes -- Local messages
static const D_NCMessageType_t c_NCMessageCodes[] =
{
	// Needs fast access
	{1, "TICS", D_NCMH_TICS, DNCMF_PERFECT | DNCMF_SERVER | DNCMF_REMOTECL},
	{1, "TCMD", D_NCMH_TCMD, DNCMF_PERFECT | DNCMF_CLIENT | DNCMF_REMOTECL | DNCMF_HOST},
	
	// Slower
	{1, "GVAR", D_NCMH_GVAR, DNCMF_PERFECT | DNCMF_SERVER | DNCMF_CLIENT | DNCMF_HOST | DNCMF_REMOTECL},
	{1, "JOIN", D_NCMH_JOIN, DNCMF_PERFECT | DNCMF_SERVER | DNCMF_CLIENT | DNCMF_HOST | DNCMF_REMOTECL},
	{1, "CONN", D_NCMH_CONN, DNCMF_PERFECT | DNCMF_CLIENT | DNCMF_HOST},
	{1, "PLAY", D_NCMH_PLAY, DNCMF_PERFECT | DNCMF_SERVER | DNCMF_REMOTECL},
	{1, "REDY", D_NCMH_REDY, DNCMF_PERFECT | DNCMF_CLIENT | DNCMF_REMOTECL | DNCMF_HOST},
	
	//{1, {0}, "LPRJ", D_NCMH_LocalPlayerRJ, DNCMF_PERFECT | DNCMF_SERVER | DNCMF_HOST | DNCMF_REMOTECL},
	//{1, {0}, "PJOK", D_NCMH_PlayerJoinOK, DNCMF_PERFECT | DNCMF_SERVER | DNCMF_REMOTECL | DNCMF_DEMO},
	
	// EOL
	{0, ""},
};

/*** FUNCTIONS ***/

/* D_LoadNetTic() -- Loads network tic */
void D_LoadNetTic(void)
{	
	size_t i;
	uint32_t PosMask;
	uint8_t PrIndex;
	
	/* Clear run tic */
	memset(&l_ClientRunTic, 0, sizeof(l_ClientRunTic));
	
	/* Determine Consistency */
	// Player position
	for (PosMask = 0, i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
			if (players[i].mo)
				PosMask ^= players[i].mo->x ^ players[i].mo->y ^ players[i].mo->z;
	
	// Random Index
	PrIndex = P_GetRandIndex();
	
	// Load into consistency info
	g_LastConsist.GameTic = gametic;
	g_LastConsist.PrIndex = PrIndex;
	g_LastConsist.PosMask = PosMask;

	/* Find it, copy it, delete it */
	for (i = 0; i < l_NetTicBufSize; i++)
		if (l_NetTicBuf[i])
			if (l_NetTicBuf[i]->RunAt == gametic)
			{
				// Detect consistency failure
				if (PrIndex != l_NetTicBuf[i]->PRandom || PosMask != l_NetTicBuf[i]->PosMask)
				{
					// Previously not failed?
					if (devparm && !l_ConsistencyFailed)
					{
						if (PrIndex != l_NetTicBuf[i]->PRandom)
							CONL_PrintF("pr: %02x !- %02x\n", PrIndex, l_NetTicBuf[i]->PRandom);
						if (PosMask != l_NetTicBuf[i]->PosMask)
							CONL_PrintF("pm: %08x != %08x\n", PosMask, l_NetTicBuf[i]->PosMask);
					}
					
					l_ConsistencyFailed = true;
				}
				
				// Clone
				memmove(&l_ClientRunTic, l_NetTicBuf[i], sizeof(l_ClientRunTic));
				
				// Delete
				Z_Free(l_NetTicBuf[i]);
				l_NetTicBuf[i] = NULL;
				
				// Return
				return;
			}
}

/* D_TicReady() -- Tic is enqueued (we have it) */
bool_t D_TicReady(const tic_t a_WhichTic)
{
	size_t i;
	
	/* Have it */
	for (i = 0; i < l_NetTicBufSize; i++)
		if (l_NetTicBuf[i])
			if (l_NetTicBuf[i]->RunAt == a_WhichTic)
				return true;
	
	/* Don't have it */
	return false;
}

/* D_ClearNetTics() -- Clears network tics */
void D_ClearNetTics(void)
{
	// static D_NetTicData_t** l_NetTicBuf;			// Buffer
	// static size_t l_NetTicBufSize;					// Size of buffer
	// l_StoreCmds
}

/* D_NetXMitCmds() -- Transmit commands */
void D_NetXMitCmds(void)
{
#define BUFSIZE 1500
	uint8_t OutBuf[BUFSIZE];
	uint8_t* p;
	int i, j, BufferSize, nc;
	D_NetClient_t* ServerNC;
	D_NetClient_t* NetClient;
	D_BS_t* Stream;
	uint16_t DiffBits;
	
	uint8_t PRi;
	uint32_t PosMask;
	
	ticcmd_t* Old, *New;
	
	/* Get Server */
	ServerNC = D_NCFindClientIsServer();
	
	// No server?
	if (!ServerNC)
		return;
	
	/* Not the server */
	if (!ServerNC->IsLocal)
		return;
	
	/* Clear output buffer */
	memset(OutBuf, 0, sizeof(BUFSIZE));
	p = OutBuf;
	
	/* Write into the buffer */
	// Consistency Info
	for (PosMask = 0, i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
			if (players[i].mo)
				PosMask ^= players[i].mo->x ^ players[i].mo->y ^ players[i].mo->z;
	
	PRi = P_GetRandIndex();
	WriteUInt8((uint8_t**)&p, PRi);
	LittleWriteUInt32((uint32_t**)&p, PosMask);
	
	// Global
	LittleWriteUInt16((uint16_t**)&p, l_StoreCmds[0][MAXPLAYERS].Ext.DataSize);
	
	for (i = 0; i < l_StoreCmds[0][MAXPLAYERS].Ext.DataSize; i++)
		WriteUInt8((uint8_t**)&p, l_StoreCmds[0][MAXPLAYERS].Ext.DataBuf[i]);
	
	// Players
	for (i = 0; i < MAXPLAYERS; i++)
	{
		// Get Old/New
		Old = &l_StoreCmds[1][i];
		New = &l_StoreCmds[0][i];
		
		// Write playeringame status
		WriteUInt8((uint8_t**)&p, playeringame[i]);
		
		// Don't write tics for players not in the game
		if (!playeringame[i])
			continue;
		
		// Write Timing from player
		LittleWriteUInt64((uint64_t**)&p, New->Ctrl.ProgramTic);
		LittleWriteUInt64((uint64_t**)&p, New->Ctrl.GameTic);
		LittleWriteUInt16((uint16_t**)&p, New->Ctrl.Ping);
		
		// Determine diff bits
		DiffBits = 0;
		
		if (New->Std.forwardmove)
			DiffBits |= DDB_FORWARD;
		if (New->Std.sidemove)
			DiffBits |= DDB_SIDE;
		if (New->Std.angleturn)
			DiffBits |= DDB_ANGLE;
		if (New->Std.aiming)
			DiffBits |= DDB_AIMING;
		if (New->Std.buttons)
			DiffBits |= DDB_BUTTONS;
		if (New->Std.BaseAngleTurn)
			DiffBits |= DDB_BAT;
		if (New->Std.BaseAiming)
			DiffBits |= DDB_BAM;
		if (New->Std.ResetAim)
			DiffBits |= DDB_RESETAIM;
		if (New->Std.InventoryBits)
			DiffBits |= DDB_INVENTORY;
		if (New->Std.StatFlags)
			DiffBits |= DDB_STATFLAGS;
		
		// Always set weapon
		DiffBits |= DDB_WEAPON;
		
		// Write bits
		LittleWriteUInt16((uint16_t**)&p, DiffBits);
		
		if (DiffBits & DDB_FORWARD)
			WriteInt8((int8_t**)&p, New->Std.forwardmove);
		if (DiffBits & DDB_SIDE)
			WriteInt8((int8_t**)&p, New->Std.sidemove);
		if (DiffBits & DDB_ANGLE)
			LittleWriteInt16((int16_t**)&p, New->Std.angleturn);
		if (DiffBits & DDB_AIMING)
			LittleWriteUInt16((uint16_t**)&p, New->Std.aiming);
		if (DiffBits & DDB_BUTTONS)
			LittleWriteUInt16((uint16_t**)&p, New->Std.buttons);
		if (DiffBits & DDB_RESETAIM)
			WriteUInt8((uint8_t**)&p, New->Std.ResetAim);
		if (DiffBits & DDB_INVENTORY)
			WriteUInt8((uint8_t**)&p, New->Std.InventoryBits);
		if (DiffBits & DDB_STATFLAGS)
			LittleWriteUInt32((uint32_t**)&p, New->Std.StatFlags);
		
		if (DiffBits & DDB_WEAPON)
		{
			for (j = 0; New->Std.XSNewWeapon[j] && j < MAXTCWEAPNAME; j++)
				WriteUInt8((uint8_t**)&p, New->Std.XSNewWeapon[j]);
			WriteUInt8((uint8_t**)&p, 0);
		}
		
		// Data Bits
		LittleWriteUInt16((uint16_t**)&p, l_StoreCmds[0][j].Std.DataSize);
		for (j = 0; j < l_StoreCmds[0][j].Std.DataSize; j++)
			WriteUInt8((uint8_t**)&p, l_StoreCmds[0][j].Std.DataBuf[j]);
	}
	
	/* Get buffer size */
	BufferSize = (uintptr_t)p - (uintptr_t)OutBuf;
	
	/* Send Commands to everyone */
	for (nc = 0; nc < l_NumClients; nc++)
	{
		// Get current
		NetClient = l_Clients[nc];
		
		// Failed?
		if (!NetClient)
			continue;
		
		// Is ourself?
		if (NetClient->IsLocal)
			continue;
		
		// Not ready?
		if (!NetClient->ReadyToPlay || !NetClient->SaveGameSent)
			continue;
		
		// Write message to them (perfect output)
		Stream = NetClient->Streams[DNCSP_PERFECTWRITE];
		
		// Base
		D_BSBaseBlock(Stream, "TICS");
		
		// Current Game Tic
		D_BSwu64(Stream, gametic);
		
		// Write buffer that was created
		D_BSwu16(Stream, BufferSize);
		D_BSWriteChunk(Stream, OutBuf, BufferSize);
		
		// Send away
		D_BSRecordNetBlock(Stream, &NetClient->Address);
	}
	
	/* Move stored commands */
	memmove(l_StoreCmds[1], l_StoreCmds[0], sizeof(l_StoreCmds[0]));
	memset(l_StoreCmds[0], 0, sizeof(l_StoreCmds[0]));
#undef BUFSIZE
}

/* D_NetReadGlobalTicCmd() -- Reads global tic command */
void D_NetReadGlobalTicCmd(ticcmd_t* const a_TicCmd)
{
	int nc;
	D_NetClient_t* ServerNC;
	D_NetClient_t* NetClient;
	D_BS_t* Stream;
	
	return;
	
	/* Get server client */
	ServerNC = D_NCFindClientIsServer();
	
	/* If we are the server... */
	// Read the global command tic buffer
	if (ServerNC->IsLocal)
	{
		// Something is in the buffer
		if (l_GlobalAt >= 0)
		{
			// Move the first item inside
			memmove(a_TicCmd, &l_GlobalBuf[0], sizeof(*a_TicCmd));
			
			// Move everything down
			memmove(&l_GlobalBuf[0], &l_GlobalBuf[1], sizeof(ticcmd_t) * (MAXGLOBALBUFSIZE - 1));
			l_GlobalAt--;
		}
	}
	
	/* Not the server */
	// Then only use the buffer sent to us via TICS from the server.
	else
	{
		memmove(a_TicCmd, &l_ClientRunTic.Data[MAXPLAYERS], sizeof(*a_TicCmd));
	}
}

/* D_NetWriteGlobalTicCmd() -- Writes global tic command */
void D_NetWriteGlobalTicCmd(ticcmd_t* const a_TicCmd)
{
	int nc, i;
	D_NetClient_t* ServerNC;
	D_NetClient_t* NetClient;
	D_BS_t* Stream;
	
	/* Get server client */
	ServerNC = D_NCFindClientIsServer();
	
	// No server?
	if (!ServerNC)
		return;
	
	// Non-Local?
	if (!ServerNC->IsLocal)
		return;
	
	/* Write in global store */
	memmove(&l_StoreCmds[0][MAXPLAYERS], a_TicCmd, sizeof(ticcmd_t));
}

/* D_NetReadTicCmd() -- Read tic commands from network */
void D_NetReadTicCmd(ticcmd_t* const a_TicCmd, const int a_Player)
{
	int nc, arb;
	D_NetClient_t* ServerNC;
	D_NetClient_t* NetClient;
	D_BS_t* Stream;
	D_NetPlayer_t* NetPlayer;
	
	/* Get server client */
	ServerNC = D_NCFindClientIsServer();
	
	/* Player not in game? */
	if (!playeringame[a_Player])
		return;
	
	/* If we are the server... */
	// See if the local player has any tics waiting.
	if (ServerNC->IsLocal)
	{
		NetPlayer = players[a_Player].NetPlayer;
		
		// Missing?
		if (!NetPlayer)
			return;
		
		// Tic waiting for us?
		if (NetPlayer->TicTotal > 0)
		{
#if 0
			D_NCSNetMergeTics(a_TicCmd, NetPlayer->TicCmd, NetPlayer->TicTotal);
			memset(NetPlayer->TicCmd, 0, sizeof(NetPlayer->TicCmd));
			NetPlayer->TicTotal = 0;
#else
			// Copy and splice down tic
			memmove(a_TicCmd, &NetPlayer->TicCmd[0], sizeof(*a_TicCmd));
			memmove(&NetPlayer->TicCmd[0], &NetPlayer->TicCmd[1], sizeof(ticcmd_t) * (MAXDNETTICCMDCOUNT - 1));
			NetPlayer->TicTotal--;
#endif
			
			// Copy into last good
			memmove(&NetPlayer->LastGoodTic, a_TicCmd, sizeof(*a_TicCmd));
		}
		
		// No tics available (dup based on player option)
		else
		{
			// Last Good Transmitted Tic
			memmove(a_TicCmd, &NetPlayer->LastGoodTic, sizeof(*a_TicCmd));
		}
		
		// Set player ID
		a_TicCmd->Std.Player = a_Player;
		
		// Successfully gave tic, so return now
		return;
	}
	
	/* Not the server */
	// Then only use the buffer sent to us via TICS from the server.
	else
	{
		memmove(a_TicCmd, &l_ClientRunTic.Data[a_Player], sizeof(*a_TicCmd));
	}
}

/* D_NetWriteTicCmd() -- Write tic commands to network */
void D_NetWriteTicCmd(ticcmd_t* const a_TicCmd, const int a_Player)
{
	int nc, i;
	D_NetClient_t* ServerNC;
	D_NetClient_t* NetClient;
	D_BS_t* Stream;
	static tic_t LastGT;
	ticcmd_t* InTic;
	D_NetPlayer_t* NetPlayer;
	
	/* Get server client */
	ServerNC = D_NCFindClientIsServer();
	
	// No server?
	if (!ServerNC)
		return;
	
	/* We are the server */
	if (ServerNC->IsLocal)
	{	
		// Write in store
		memmove(&l_StoreCmds[0][a_Player], a_TicCmd, sizeof(ticcmd_t));
	}
	
	/* We are the client */
	// This is a bit hacky here, but it can work
	else
	{
		// No local players?
		if (g_SplitScreen <= -1)
			return;
		
		// Game tic not changed?
		if (gametic == LastGT)
			return;
		
		// Set changed
		LastGT = gametic;
			
		// Use server stream
		Stream = ServerNC->Streams[DNCSP_PERFECTWRITE];
			
		// Build packet
		D_BSBaseBlock(Stream, "TCMD");
		
		// Current Time Codes
		D_BSwu64(Stream, gametic);
		D_BSwu64(Stream, g_ProgramTic);
		
		// Consistency Double-Check (the kicking part)
		D_BSwu64(Stream, g_LastConsist.GameTic);
		D_BSwu8(Stream, g_LastConsist.PrIndex);
		D_BSwu32(Stream, g_LastConsist.PosMask);
		
		// Send stuff for all our local players
		D_BSwi8(Stream, g_SplitScreen);
		for (i = 0; i < g_SplitScreen + 1; i++)
		{
			// Get net player
			NetPlayer = players[g_Splits[i].Console].NetPlayer;
			
			// Does not exist? No tics in buffer?
			if (!NetPlayer || (NetPlayer && !NetPlayer->TicTotal))
			{
				D_BSwu8(Stream, 0);
				continue;
			}
			
			// Write Marker
			D_BSwu8(Stream, 1);
			D_BSwu8(Stream, g_Splits[i].Console);
			
			// Get tic to send
			InTic = &NetPlayer->TicCmd[0];
			
			// Timing
			D_BSwu64(Stream, InTic->Ctrl.ProgramTic);
			D_BSwu64(Stream, InTic->Ctrl.GameTic);
			D_BSwu16(Stream, InTic->Ctrl.Ping);
			
			// Send Tic Data
			D_BSwi8(Stream, InTic->Std.forwardmove);
			D_BSwi8(Stream, InTic->Std.sidemove);
			D_BSwi16(Stream, InTic->Std.angleturn);
			D_BSwu16(Stream, InTic->Std.aiming);
			D_BSwu16(Stream, InTic->Std.buttons);
			D_BSwi16(Stream, InTic->Std.BaseAngleTurn);
			D_BSwi16(Stream, InTic->Std.BaseAiming);
			D_BSwu8(Stream, InTic->Std.ResetAim);
			D_BSwu8(Stream, InTic->Std.InventoryBits);
			D_BSwu32(Stream, InTic->Std.StatFlags);
			D_BSws(Stream, InTic->Std.XSNewWeapon);
			
			// Delete Command
			NetPlayer->TicTotal = 0;
			memset(InTic, 0, sizeof(InTic));
		}
		
		// Send
		D_BSRecordNetBlock(Stream, &ServerNC->Address);
	}
}

/* D_NCUpdate() -- Update all networking stuff */
void D_NCUpdate(void)
{
	int32_t nc, tN;
	intptr_t IsPerf;
	char Header[5];
	I_HostAddress_t FromAddress;
	D_NetClient_t* NetClient, *RemoteClient;
	D_BS_t* PIn, *POut, *BOut;
	bool_t IsServ, IsClient, IsHost;
	D_NCMessageData_t Data;
	uint32_t ThisTime;
	static int8_t LastSplitMode;
	
	return;
	
	/* Handle Profiling */
	for (nc = 0; nc < MAXSPLITSCREEN; nc++)
	{
		// Stop?
		if (!D_ScrSplitHasPlayer(nc))
			break;
		
		// Waiting for join?
		if (g_Splits[nc].Waiting)
			// No Profile Active?
			if (!g_Splits[nc].Profile)
			{
				// Annoy player with profile selection, if not doing so already
			}
			
			// Profile active (send/timeout, but only if connected)
			else if (l_IsConnected)
			{
				// Send request to server?
				if (!g_Splits[nc].RequestSent)
				{
					// Get server to send join request to
					RemoteClient = D_NCFindClientIsServer();
					
					if (RemoteClient)
					{
						// Get Stream
						POut = RemoteClient->Streams[DNCSP_PERFECTWRITE];
						
						// Put Data
						D_BSBaseBlock(POut, "JOIN");
						D_BSwu8(POut, 0);		// Not a bot
						D_BSws(POut, g_Splits[nc].Profile->UUID);
						D_BSwu32(POut, g_Splits[nc].Profile->InstanceID);
						D_BSwu32(POut, g_Splits[nc].ProcessID);
						D_BSws(POut, g_Splits[nc].Profile->AccountName);
						D_BSws(POut, g_Splits[nc].Profile->DisplayName);
						D_BSwu8(POut, g_Splits[nc].Profile->Color);
						D_BSws(POut, g_Splits[nc].Profile->HexenClass);
					
						// Send
						D_BSRecordNetBlock(POut, &RemoteClient->Address);
						
						// Set as sent and timeout at 3 minutes
						g_Splits[nc].RequestSent = true;
						g_Splits[nc].GiveUpAt = g_ProgramTic + (TICRATE * 180);
						
						// Message for the avid player
						CONL_OutputU(DSTR_DNETC_JOININGPLAYER, "%s\n", g_Splits[nc].Profile->DisplayName);
					}
				}
				
				// Request sent, check timeout
				else
				{
				}
			}
	}
	
	// If connected, reset splitscreen
	if (l_IsConnected)
	{
		g_SplitScreen = ((int32_t)nc) - 1;
		
		if (g_SplitScreen != LastSplitMode)
			R_ExecuteSetViewSize();
		
		LastSplitMode = g_SplitScreen;
	}
	
	// Remember Last
	else
		LastSplitMode = g_SplitScreen;
	
	/* Get current time */
	ThisTime = I_GetTimeMS();
	
	/* Go through each client and read/write commands */
	for (nc = 0; nc < l_NumClients; nc++)
	{
		// Get current
		NetClient = l_Clients[nc];
		
		// Failed?
		if (!NetClient)
			continue;
		
		// A client was not ready fast enough.
		if (!NetClient->IsServer && !NetClient->IsLocal)
			if (!NetClient->ReadyToPlay && ThisTime > NetClient->ReadyTimeout)
			{
				D_NCFudgeOffClient(NetClient, 'T', "Connection readyness timed out.");
				continue;
			}
		
		// Set streams to read/write from
		PIn = NetClient->Streams[DNCSP_PERFECTREAD];
		POut = NetClient->Streams[DNCSP_PERFECTWRITE];
		BOut = NetClient->Streams[DNCSP_WRITE];
		
		// Clear from address (since it is invalid for locals)
		memset(&FromAddress, 0, sizeof(FromAddress));
		
		// Constantly read from the perfect input stream (if it is set)
		memset(Header, 0, sizeof(Header));
		while (PIn && D_BSPlayNetBlock(PIn, Header, &FromAddress))
		{
			// Debug
			if (devparm)
				;//CONL_PrintF("Read \"%s\"\n", Header);
			
			// Find the remote client that initiated this message
			// If they aren't in the client chain then this returns NULL.
			// If it does return NULL then that means they were never connected
			// in the first place.
			RemoteClient = D_NCFindClientByHost(&FromAddress);
			
			// Server Protection
			if (RemoteClient)
				if (RemoteClient->Protected)
					continue;
			
			// Determine where this packet came from for flag checking
				// Is Perfect Packet
			IsPerf = 0;
			D_BSStreamIOCtl(PIn, DRBSIOCTL_ISPERFECT, &IsPerf);
				// From Server
			IsServ = false;
			if (RemoteClient)
				IsServ = RemoteClient->IsServer;
				// From Client
			IsClient = !IsServ;
				// We are the host
			IsHost = (NetClient->IsServer && NetClient->IsLocal);
			
			// Go through head table
			for (tN = 0; c_NCMessageCodes[tN].Valid; tN++)
			{
				// Wrong header packet?
				if (!D_BSCompareHeader(c_NCMessageCodes[tN].Header, Header))
				{
					//if (g_NetDev)
					//	CONL_PrintF("NET: \"%s\" not \"%s\".\n", Header, c_NCMessageCodes[tN].Header);
					continue;
				}
				
				// Perfect but not set?
				/*if (IsPerf && !(c_NCMessageCodes[tN].Flags & DNCMF_PERFECT))
					continue;*/
				
				// Normal but not set?
				/*if (!IsPerf && !(c_NCMessageCodes[tN].Flags & DNCMF_NORMAL))
					continue;*/
				
				// From client but not accepted from client
				if (IsClient && !(c_NCMessageCodes[tN].Flags & DNCMF_CLIENT))
				{
					if (g_NetDev)
						CONL_PrintF("NET: \"%s\" not client.\n", Header);
					continue;
				}
				
				// From server but not accepted from server
				if (IsServ && !(c_NCMessageCodes[tN].Flags & DNCMF_SERVER))
				{
					if (g_NetDev)
						CONL_PrintF("NET: \"%s\" not server.\n", Header);
					continue;
				}
				
				// From somewhere but we are not the host of the game
				if (IsHost && !(c_NCMessageCodes[tN].Flags & DNCMF_HOST))
				{
					if (g_NetDev)
						CONL_PrintF("NET: \"%s\" not host.\n", Header);
					continue;
				}
				
				// Requires a remote client exist (a connected player)
				if ((c_NCMessageCodes[tN].Flags & DNCMF_REMOTECL) && !RemoteClient)
				{
					if (g_NetDev)
						CONL_PrintF("NET: \"%s\" not remote client.\n", Header);
					continue;
				}
				
				// Clear data
				memset(&Data, 0, sizeof(Data));
				
				// Fill Data
				Data.Type = &c_NCMessageCodes[tN];
				Data.NetClient = NetClient;
				Data.RCl = RemoteClient;
				Data.InStream = PIn;
				Data.OutStream = (IsPerf ? POut : BOut);
				Data.FromAddr = &FromAddress;
				
				// Call handler
				if (c_NCMessageCodes[tN].Func(&Data))
					break;
				
				// Client removed?
				if (!l_Clients[nc])
					break;
			}
			
			// Clear header for next read run
			memset(Header, 0, sizeof(Header));
			
			// Client removed?
			if (!l_Clients[nc])
				break;
		}
		
		// Client removed?
		if (!l_Clients[nc])
			continue;
		
		// Flush write streams so our commands are sent
			// Commands could be in the local loopback which are unsent until
			// flushed.
		D_BSFlushStream(POut);
		D_BSFlushStream(BOut);
	}
}

/* D_NCDrawer() -- Draws net stuff */
void D_NCDrawer(void)
{
	/* Consistency failure message */
	if (l_ConsistencyFailed)
	{
		// Draw text
		V_DrawStringA(VFONT_LARGE, 0, DS_GetString(DSTR_DNETC_CONSISTFAIL), 10, 20);
		V_DrawStringA(VFONT_SMALL, 0, DS_GetString(DSTR_DNETC_PLEASERECON), 10, 35);
	}
}

/*****************************************************************************/

/*** NCSR FUNCTIONS ***/

/* D_NCSR_RequestMap() -- Requests that the map changes */
void D_NCSR_RequestMap(const char* const a_Map)
{
	D_NetClient_t* Server;
	P_LevelInfoEx_t* Info;
	size_t i, j;
	bool_t LocalHit;
	void* Wp;
	ticcmd_t* Placement;
	
	/* Check */
	if (!a_Map)
		return;
	
	/* Find Server */
	Server = D_NCFindClientIsServer();
	
	// Not found?
	if (!Server)
		return;
	
	// Found server, but we are not the server
	if (!Server->IsLocal)
	{
		CONL_OutputU(DSTR_NET_YOUARENOTTHESERVER, "\n");
		return;
	}
	
	/* Find level */
	Info = P_FindLevelByNameEx(a_Map, NULL);
	
	// Check
	if (!Info)
	{
		CONL_OutputU(DSTR_NET_LEVELNOTFOUND, "%s\n", a_Map);
		return;
	}
	
	/* Place */
	Wp = NULL;
	Placement = DS_GrabGlobal(DTCT_MAPCHANGE, c_TCDataSize[DTCT_MAPCHANGE], &Wp);
	
	if (Placement)
	{
		// Fill in data
		WriteUInt8((uint8_t**)&Wp, 0);
		
		for (i = 0, j = 0; i < 8; i++)
			if (!j)
			{
				WriteUInt8((uint8_t**)&Wp, Info->LumpName[i]);
				
				if (!Info->LumpName[i])
					j = 1;
			}
			else
				WriteUInt8((uint8_t**)&Wp, 1);
	}
}

/* D_NCSR_SendLoadingStatus() -- Tell the server we are loading something */
void D_NCSR_SendLoadingStatus(const int32_t a_MajIs, const int32_t a_MajOf, const int32_t a_MinIs, const int32_t a_MinOf)
{
}


/******************************
*** NEW EXTENDED NETWORKING ***
******************************/

/*** CONSTANTS ***/

static const fixed_t c_forwardmove[2] = { 25, 50 };
static const fixed_t c_sidemove[2] = { 24, 40 };
static const fixed_t c_angleturn[3] = { 640, 1280, 320 };	// + slow turn
#define MAXPLMOVE       (c_forwardmove[1])
#define MAXLOCALJOYS	MAXJOYSTICKS			// Max joysticks handled

/*** GLOBALS ***/

D_XSocket_t** g_XSocks = NULL;					// Extended Sockets
size_t g_NumXSocks = 0;							// Number of them

D_XPlayer_t** g_XPlays = NULL;					// Extended Players
size_t g_NumXPlays = 0;							// Number of them

tic_t g_DemoFreezeTics = 0;						// Tics to freeze demo for

/*** LOCALS ***/

static tic_t l_XNLastPTic;						// Last Program Tic

static bool_t l_PermitMouse = false;			// Use mouse input
static int32_t l_MouseMove[2] = {0, 0};			// Mouse movement (x/y)
static uint32_t l_MouseButtons[2];				// Mouse buttons down (sng/dbl)
static tic_t l_MouseLastTime[32];				// Last time pressed
static bool_t l_KeyDown[NUMIKEYBOARDKEYS];		// Keys that are down
static uint32_t l_JoyButtons[MAXLOCALJOYS];		// Local Joysticks
static int16_t l_JoyAxis[MAXLOCALJOYS][MAXJOYAXIS];

/*** FUNCTIONS ***/

/* D_XNetDisconnect() -- Disconnect from server/self server */
// Replaces D_NCDisconnect()
void D_XNetDisconnect(const bool_t a_FromDemo)
{
	int32_t i;
	static bool_t DoingDiscon;
	
	/* Already disconnecting? */
	if (DoingDiscon)
		return;
	DoingDiscon = true;
	
	/* Send quit message to server */
	D_XNetSendQuit();
	
	/* Disconnect all players */
	for (i = 0; i < g_NumXPlays; i++)
		if (g_XPlays[i])
			D_XNetKickPlayer(g_XPlays[i], DS_GetString(DSTR_NET_SERVERDISCON));
	
	/* Remove all sockets */
	for (i = 0; i < g_NumXSocks; i++)
		if (g_XSocks[i])
			D_XNetDelSocket(g_XSocks[i]);
	
	/* Demos? */
	if (demoplayback)
	{
		// Don't quit when the demo stops
		singledemo = false;
		
		// Stop it
		G_StopDemoPlay();
	}
	
	/* Clear network stuff */
	// Tics
	D_ClearNetTics();
	
	// Consistency problems
	l_ConsistencyFailed = false;
	
	// Revert back to solo networking
	l_SoloNet = true;
	l_IsConnected = false;	// Set disconnected
	
	// Reset time to now
	l_XNLastPTic = g_ProgramTic;
	g_DemoFreezeTics = 0;
	
	/* Clear all player information */
	// Reset all vars
	P_XGSSetAllDefaults();
	
	// Just wipe ALL of it!
	memset(players, 0, sizeof(players));
	memset(playeringame, 0, sizeof(playeringame));
	
	// If connected, reset splits
	if (l_IsConnected)
		D_NCResetSplits(a_FromDemo);
	
	// Always reset demo splits though
	D_NCResetSplits(true);
	
	// Initialize some players some
	for (i = 0; i < MAXPLAYERS; i++)
		G_InitPlayer(&players[i]);
	
	/* Destroy the level */
	P_ExClearLevel();
	
	/* Go back to the title screen */
	gamestate = GS_DEMOSCREEN;
	
	/* Done disconnecting */
	DoingDiscon = false;
}

/* DS_XNetMakeServPB() -- Callback for make player */
static void DS_XNetMakeServPB(D_XPlayer_t* const a_Player)
{
	/* Set initial information */
	a_Player->Flags |= DXPF_SERVER | DXPF_LOCAL;
}

/* D_XNetMakeServer() -- Creates a server, possibly networked */
void D_XNetMakeServer(const bool_t a_Networked, const uint16_t a_NetPort)
{
	int32_t i;
	D_XPlayer_t* SPlay;
	
	/* Disconnect First */
	D_XNetDisconnect(false);
	
	/* Create a starting spectator (the host) */
	SPlay = D_XNetAddPlayer(DS_XNetMakeServPB, NULL);
	
	// Set server infos
	
	/* Clear important flags */
	// not playing title screen demos
	g_TitleScreenDemo = false;
	
	/* Set the proper gamestate */
	gamestate = wipegamestate = GS_WAITINGPLAYERS;
	S_ChangeMusicName("D_WAITIN", 1);			// A nice tune
	
	/* Initialize screens */
	// Only give players to screens that have occupents
	for (i = 0; i < MAXSPLITSCREEN; i++)
		if (D_ScrSplitHasPlayer(i))
		{
			// Create initial player for server, but not for P1
			if (i)
				SPlay = D_XNetAddPlayer(DS_XNetMakeServPB, NULL);
			
			// Assign player
			g_Splits[i].XPlayer = SPlay;
			SPlay->ScreenID = i;
			SPlay->Profile = g_Splits[i].Profile;	// if lucky
			
			// Up splitscreen
			g_SplitScreen = i;
		}
	
	/* Calculate Split-screen */
	R_ExecuteSetViewSize();
}

/* D_XNetIsServer() -- Returns true if we are the server */
bool_t D_XNetIsServer(void)
{
	int32_t i;
	
	/* Grab server status from first local player */
	for (i = 0; i < g_NumXPlays; i++)
		if (g_XPlays[i])
			if (g_XPlays[i]->Flags & DXPF_LOCAL)
				return !!(g_XPlays[i]->Flags & DXPF_SERVER);
	
	/* Fell through */
	return false;
}

/* D_XNetGetHostID() -- Gets our host ID */
bool_t D_XNetGetHostID(void)
{
	int32_t i;
	
	/* Grab server status from first local player */
	for (i = 0; i < g_NumXPlays; i++)
		if (g_XPlays[i])
			if (g_XPlays[i]->Flags & DXPF_LOCAL)
				return g_XPlays[i]->HostID;
	
	/* Fell through */
	return 0;
}

/* D_XNetPlayerByID() -- Finds player by ID */
D_XPlayer_t* D_XNetPlayerByID(const uint32_t a_ID)
{
	int32_t i;
	
	/* Check */
	if (!a_ID)
		return NULL;
	
	/* Search players */
	for (i = 0; i < g_NumXPlays; i++)
		if (g_XPlays[i])
			if (g_XPlays[i]->ID == a_ID)
				return g_XPlays[i];
	
	/* Not Found */
	return NULL;
}

/* D_XNetLocalPlayerByPID() -- Finds local player by process ID */
D_XPlayer_t* D_XNetLocalPlayerByPID(const uint32_t a_ID)
{
	int32_t i;
	
	/* Check */
	if (!a_ID)
		return NULL;
	
	/* Search players */
	for (i = 0; i < g_NumXPlays; i++)
		if (g_XPlays[i])
			if (g_XPlays[i]->Flags & DXPF_LOCAL)
				if (g_XPlays[i]->ClProcessID == a_ID)
					return g_XPlays[i];
	
	/* Not Found */
	return NULL;
}

/* D_XNetDelSocket() -- Deletes Socket */
void D_XNetDelSocket(D_XSocket_t* const a_Socket)
{
}

/* D_XNetAddPlayer() -- Adds new player */
D_XPlayer_t* D_XNetAddPlayer(void (*a_PacketBack)(D_XPlayer_t* const a_Player, void* const a_Data), void* const a_Data)
{
	D_XPlayer_t* New;
	uint32_t ID, OurHID;
	void* Wp;
	ticcmd_t* Placement;
	int32_t i;
	
	/* Allocate */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Base initialization */
	New->InGameID = -1;
	New->ScreenID = -1;
	strncpy(New->AccountName, "I have no account!", MAXPLAYERNAME);
	strncpy(New->AccountServer, "remood.org", MAXXSOCKTEXTSIZE);
	
	/* Call callback */
	if (a_PacketBack)
		a_PacketBack(New, a_Data);
	
	// ID already taken?
	if (New->ID && D_XNetPlayerByID(New->ID) != New)
	{
		Z_Free(New);
		return NULL;
	}
	
	/* Create ID for the player */
	do
	{
		ID = D_CMakePureRandom();
	} while (!ID || D_XNetPlayerByID(ID));
	
	// Set ID, is hopefully really random
	New->ID = ID;
	
	/* Link into players list */
	// Find free spot
	for (i = 0; i < g_NumXPlays; i++)
		if (!g_XPlays[i])
			break;
	
	// No room?
	if (i >= g_NumXPlays)
	{
		Z_ResizeArray((void**)&g_XPlays, sizeof(*g_XPlays),
			g_NumXPlays, g_NumXPlays + 1);
		i = g_NumXPlays++;
	}
	
	// Set here
	g_XPlays[i] = New;
	
	/* Bind to screen */
	OurHID = D_XNetGetHostID();
	if (New->HostID == OurHID)
		for (i = 0; i < MAXSPLITSCREEN; i++)
			if (D_ScrSplitHasPlayer(i))
				if (g_Splits[i].ProcessID = New->ClProcessID)
					g_Splits[i].XPlayer = New;
	
	/* Send player creation packet (if server) */
	if (D_XNetIsServer())
	{
		// Grab global command
		Placement = DS_GrabGlobal(DTCT_XADDPLAYER, c_TCDataSize[DTCT_XADDPLAYER], &Wp);
		
		// Got one
		if (Placement)
		{
			// Addition Info
			LittleWriteUInt32((uint32_t**)&Wp, New->ID);
			LittleWriteUInt32((uint32_t**)&Wp, New->HostID);
			LittleWriteUInt32((uint32_t**)&Wp, New->ClProcessID);
			WriteUInt8((uint8_t**)&Wp, New->ScreenID);
			LittleWriteUInt32((uint32_t**)&Wp, 0);
			LittleWriteUInt32((uint32_t**)&Wp, New->Flags);
			
			// Write Names
			for (i = 0; i < MAXPLAYERNAME; i++)
			{
				WriteUInt8((uint8_t**)&Wp, New->AccountName[i]);
				WriteUInt8((uint8_t**)&Wp, New->AccountCookie);
			}
		}
	}
	
	/* Write console message */
	CONL_OutputUT(CT_NETWORK, DSTR_NET_CLIENTCONNECTED, "%s\n",
			New->AccountName
		);
	
	/* Update Scores */
	P_UpdateScores();
	
	/* Return fresh player */
	return New;
}

/* D_XNetKickPlayer() -- Kicks player for some reason */
void D_XNetKickPlayer(D_XPlayer_t* const a_Player, const char* const a_Reason)
{
	size_t Slot;
	int32_t i, j;
	void* Wp;
	ticcmd_t* Placement;
	uint32_t OurHID;
	
	/* Check */
	if (!a_Player)
		return;
		
	/* Get our host ID */
	OurHID = D_XNetGetHostID();
	
	/* Find slot player uses */
	for (Slot = 0; Slot < g_NumXPlays; Slot++)
		if (a_Player == g_XPlays[Slot])
			break;
	
	// Not in any slot? Must have been removed then
	if (Slot >= g_NumXPlays)
		return;
	
	/* Remove from slot */
	g_XPlays[Slot] = NULL;
	
	/* Local Player */
	if (a_Player->Flags & DXPF_LOCAL)
	{
		// Disconnect from server, if no more local players remain
		for (j = 0, i = 0; i < g_NumXPlays; i++)
			if (g_XPlays[i])
				if (g_XPlays[i]->Flags & DXPF_LOCAL)
					j++;
		
		// No players, so disconnect ourself
		if (j <= 0)
			D_XNetDisconnect(false);
		
		// Remove from local screen
		if (a_Player->HostID == OurHID)
			for (i = 0; i < MAXSPLITSCREEN; i++)
				if (D_ScrSplitVisible(i))
					if (a_Player == g_Splits[i].XPlayer)
						D_NCRemoveSplit(i, demoplayback);
	}
	
	/* Remote Player */
	else
	{
		// Send disconnection packet to them, if we are the server
		if (D_XNetIsServer())
		{
			// See if the player is not shared by any more hosts (all gone)
			for (j = 0, i = 0; i < g_NumXPlays; i++)
				if (g_XPlays[i])
					if (g_XPlays[i]->HostID == a_Player->HostID)
						j++;
			
			// None left
			if (!j)
			{
			}
		}
	}
	
	/* Enqueue Special Command, if server */
	// This informs the other clients (w/ demo) to delete the player. This is
	// also read for the game state, to remove players who have no controller.
	// So if a player is dropping and everyone is stuck on a WFP screen and the
	// player is removed, they will still be "in" the game until the tic command
	// removes them.
	if (D_XNetIsServer())
	{
		// Grab global command
		Placement = DS_GrabGlobal(DTCT_XKICKPLAYER, c_TCDataSize[DTCT_XKICKPLAYER], &Wp);
		
		// Got one
		if (Placement)
		{
			// Send player to remove
			LittleWriteUInt16((uint16_t**)&Wp, a_Player->InGameID);
			LittleWriteUInt32((uint32_t**)&Wp, a_Player->ID);
			
			// Write reason
			for (i = 0, j = 0; i < MAXTCCBUFSIZE; i++)
				if (a_Reason && a_Reason[j] && j < MAXTCCBUFSIZE - 1)
					WriteUInt8((uint8_t**)&Wp, a_Reason[j++]);
				else
					WriteUInt8((uint8_t**)&Wp, 0);
		}
	}
	
	/* Delete bot, if any */
	if (a_Player->BotData)
	{
		B_XDestroyBot(a_Player->BotData);
		a_Player->BotData = NULL;
	}
	
	/* Write console message */
	CONL_OutputUT(CT_NETWORK, DSTR_NET_CLIENTGONE, "%s%s\n",
			a_Player->AccountName,
			(!a_Reason ? DS_GetString(DSTR_NET_NOREASON) : a_Reason)
		);
	
	/* Free associated data */
	Z_Free(a_Player);
	
	/* Update Scores */
	P_UpdateScores();
}

/* D_XNetSendQuit() -- Informs the server we are quitting */
void D_XNetSendQuit(void)
{
	/* If we are not the server, tell the server */
	if (!D_XNetIsServer())
	{
	}
	
	/* Disconnect from the server */
	D_XNetDisconnect(false);
}

/* D_XNetChangeVar() -- Change Variable */
void D_XNetChangeVar(const uint32_t a_Code, const int32_t a_Value)
{
	ticcmd_t* Placement;
	void* Wp;
	
	/* Server can always change variables */
	if (D_XNetIsServer())
	{
		// Encode in command
		Placement = DS_GrabGlobal(DTCT_GAMEVAR, c_TCDataSize[DTCT_GAMEVAR], &Wp);
	
		if (Placement)
		{
			// Fill in data
			LittleWriteUInt32((uint32_t**)&Wp, a_Code);
			LittleWriteInt32((int32_t**)&Wp, a_Value);
		}
	}
	
	/* Client must ask the server */
	else
	{
	}
}

/* D_XNetChangeMap() -- Changes the map */
void D_XNetChangeMap(const char* const a_Map)
{
	P_LevelInfoEx_t* Info;
	size_t i, j;
	void* Wp;
	ticcmd_t* Placement;
	
	/* Find level */
	Info = P_FindLevelByNameEx(a_Map, NULL);
	
	// Check
	if (!Info)
	{
		CONL_OutputUT(CT_NETWORK, DSTR_NET_LEVELNOTFOUND, "%s\n", a_Map);
		return;
	}
	
	/* Server can change the map no problem */
	if (D_XNetIsServer())
	{
		Wp = NULL;
		Placement = DS_GrabGlobal(DTCT_MAPCHANGE, c_TCDataSize[DTCT_MAPCHANGE], &Wp);
	
		if (Placement)
		{
			// Fill in data
			WriteUInt8((uint8_t**)&Wp, 0);
		
			for (i = 0, j = 0; i < 8; i++)
				if (!j)
				{
					WriteUInt8((uint8_t**)&Wp, Info->LumpName[i]);
				
					if (!Info->LumpName[i])
						j = 1;
				}
				else
					WriteUInt8((uint8_t**)&Wp, 1);
		}
	}
	
	/* Client must ask the server though */
	else
	{
	}
}

/* D_XNetChangeLocalProf() -- Change local screen to use this profile */
void D_XNetChangeLocalProf(const int32_t a_ScreenID, struct D_ProfileEx_s* const a_Profile)
{
	/* Check */
	if (a_ScreenID < 0 || a_ScreenID >= MAXSPLITSCREEN || !a_Profile)
		return;
	
	/* No player? */
	if (!g_Splits[a_ScreenID].XPlayer)
		return;
	
	/* Set screen to use profile */
	g_Splits[a_ScreenID].XPlayer->Profile = a_Profile;
	g_Splits[a_ScreenID].Profile = a_Profile;
	
	/* Send information to server to change colors and stuff */
	D_XNetSendColors(g_Splits[a_ScreenID].XPlayer);
}

/* D_XNetSendColors() -- Sends colors and such to server */
void D_XNetSendColors(D_XPlayer_t* const a_Player)
{
	/* Check */
	if (!a_Player)
		return;
}

/* D_XNetTryJoin() -- Player attempt to join game */
void D_XNetTryJoin(D_XPlayer_t* const a_Player)
{
	const B_BotTemplate_t* BotTemplate;
	D_ProfileEx_t* Profile;
	ticcmd_t* Placement;
	void* Wp;
	int32_t i;
	
	/* Already playing? */
	if (a_Player->Player)
		return;
	
	/* If we are the server and this is ourself, join ourself */
	if (D_XNetIsServer())
	{
		// Grab global command
		Placement = DS_GrabGlobal(DTCT_XJOINPLAYER, c_TCDataSize[DTCT_XJOINPLAYER], &Wp);
		
		// Got one
		if (Placement)
		{
			// Is a bot
			if (a_Player->Flags & DXPF_BOT)
			{
				// Get Template
				BotTemplate = B_BotGetTemplateDataPtr(a_Player->BotData);
				
				// Write to network
				LittleWriteUInt32((uint32_t**)&Wp, a_Player->ID);
				LittleWriteUInt32((uint32_t**)&Wp, a_Player->ClProcessID);
				LittleWriteUInt32((uint32_t**)&Wp, a_Player->HostID);
				LittleWriteUInt32((uint32_t**)&Wp, DTCJF_ISBOT);
				WriteUInt8((uint8_t**)&Wp, BotTemplate->SkinColor);
				WriteUInt8((uint8_t**)&Wp, 0);
				LittleWriteUInt32((uint32_t**)&Wp, 0);
				
				for (i = 0; i < MAXPLAYERNAME; i++)
					WriteUInt8((uint8_t**)&Wp, BotTemplate->DisplayName[i]);
					
				for (i = 0; i < MAXPLAYERNAME; i++)
					WriteUInt8((uint8_t**)&Wp, BotTemplate->HexenClass[i]);
			}
			
			// Normal Player
			else
			{
				// Get player
				Profile = a_Player->Profile;
				
				// Write to network
				LittleWriteUInt32((uint32_t**)&Wp, a_Player->ID);
				LittleWriteUInt32((uint32_t**)&Wp, a_Player->ClProcessID);
				LittleWriteUInt32((uint32_t**)&Wp, a_Player->HostID);
				LittleWriteUInt32((uint32_t**)&Wp, 0);
				WriteUInt8((uint8_t**)&Wp, Profile->Color);
				WriteUInt8((uint8_t**)&Wp, 0);
				LittleWriteUInt32((uint32_t**)&Wp, 0);
				
				for (i = 0; i < MAXPLAYERNAME; i++)
					WriteUInt8((uint8_t**)&Wp, Profile->DisplayName[i]);
					
				for (i = 0; i < MAXPLAYERNAME; i++)
					WriteUInt8((uint8_t**)&Wp, Profile->HexenClass[i]);
			}
		}
	}
	
	/* Otherwise, we must ask the server */
	else
	{
	}
}

/* D_XNetCreatePlayer() -- Create player joined in game */
void D_XNetCreatePlayer(D_XJoinPlayerData_t* const a_JoinData)
{
	int i, j, k;
	D_XPlayer_t* XPlay;
	player_t* Player;
	
	/* Check */
	if (!a_JoinData)
		return;
	
	/* Check player limits */
	k = -1;
	for (j = i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
			j++;
		else
			if (k < 0)
				k = i;
	
	// Too many players?
	if (j >= MAXPLAYERS)
		return;
	
	/* Change variables */
	// Set multiplayer mode, if player already exists
	if (j > 0)
		P_XGSSetValue(true, PGS_COMULTIPLAYER, 1);
	
	/* Find XPlayer */
	XPlay = D_XNetPlayerByID(a_JoinData->ID);
	
	/* Setup XPlayer */
	XPlay->InGameID = k;
	
	/* Setup Game Player */
	XPlay->Player = Player = G_AddPlayer(k);
	
	Player->XPlayer = XPlay;
	Player->skincolor = a_JoinData->Color;
	D_NetSetPlayerName(k, a_JoinData->DisplayName);
	
	/* Setup Screen */
	for (i = 0; i < MAXSPLITSCREEN; i++)
		if (D_ScrSplitVisible(i))
			if (g_Splits[i].XPlayer == XPlay)
			{
				g_Splits[i].Active = true;
				g_Splits[i].Waiting = false;
				g_Splits[i].Console = g_Splits[i].Display = k;
			}
	
	/* Print Message */
	CONL_OutputUT(CT_NETWORK, DSTR_NET_PLAYERJOINED, "%s\n",
			player_names[k]
		);
	
	/* Update Scores */
	P_UpdateScores();
}

/* D_XNetMultiTics() -- Read/Write Tics all in one */
void D_XNetMultiTics(ticcmd_t* const a_TicCmd, const bool_t a_Write, const int32_t a_Player)
{
	int32_t i;
	D_XPlayer_t* XPlay;
	
	/* We are the server */
	if (D_XNetIsServer())
	{
		// Write commands to clients
		if (a_Write)
		{
		}
	
		// Read commands
		else
		{
			// Global
			if (a_Player < 0)
			{
				// Something is in the buffer
				if (l_GlobalAt >= 0)
				{
					// Move the first item inside
					memmove(a_TicCmd, &l_GlobalBuf[0], sizeof(*a_TicCmd));
					
					// Move everything down
					memmove(&l_GlobalBuf[0], &l_GlobalBuf[1], sizeof(ticcmd_t) * (MAXGLOBALBUFSIZE - 1));
					l_GlobalAt--;
				}
			}
			
			// Individual player
			else
			{
				// Get player
				XPlay = players[a_Player].XPlayer;
				
				// No player?
				if (!XPlay)
					return;
				
				// No commands available in queue? (lag)
				if (!XPlay->LocalAt)
				{
					// Ouch =(
					memmove(a_TicCmd, &XPlay->BackupTicCmd, sizeof(ticcmd_t));
					
					// Set as missing commands
					XPlay->StatusBits |= DXPSB_MISSINGTICS;
				}
				
				// Merge local tic commands
				else
				{
					D_NCSNetMergeTics(a_TicCmd, XPlay->LocalBuf, XPlay->LocalAt);
					XPlay->LocalAt = 0;
					memset(XPlay->LocalBuf, 0, sizeof(XPlay->LocalBuf));
				
					// If player is local, modify angle set
					if (XPlay->Flags & DXPF_LOCAL)
						if (XPlay->ScreenID >= 0 && XPlay->ScreenID < MAXSPLITSCREEN)
						{
							// Absolute Angles
							if (P_XGSVal(PGS_COABSOLUTEANGLE))
							{
								localangle[XPlay->ScreenID] += a_TicCmd->Std.BaseAngleTurn << 16;
								a_TicCmd->Std.angleturn = localangle[XPlay->ScreenID] >> 16;
							}
			
							// Doom Angles
							else
								a_TicCmd->Std.angleturn = a_TicCmd->Std.BaseAngleTurn;
						
							// Aiming Angle
							if (a_TicCmd->Std.ResetAim)
								localaiming[XPlay->ScreenID] = 0;
							else
								localaiming[XPlay->ScreenID] += a_TicCmd->Std.BaseAiming << 16;
							a_TicCmd->Std.aiming = G_ClipAimingPitch(&localaiming[XPlay->ScreenID]);
						}
					
					// Copy command to backup command
					memmove(&XPlay->BackupTicCmd, a_TicCmd, sizeof(ticcmd_t));
					
					// Clear missing commands
					XPlay->StatusBits &= ~DXPSB_MISSINGTICS;
				}
				
				// Always move over status bits
				a_TicCmd->Std.StatFlags = XPlay->StatusBits;
			}
		}
	}
	
	/* We are the client */
	else
	{
	}
}

/* D_XNetTicsToRun() -- Amount of tics to run */
tic_t D_XNetTicsToRun(void)
{
	tic_t Diff, ThisTic;
	
	/* Get current tic */
	ThisTic = I_GetTime();
	
	/* Playing a demo back */
	if (demoplayback)
	{
		// Timing Demo
		if (singletics)
			return 1;
		
		// Get time difference
		Diff = ThisTic - l_XNLastPTic;
		l_XNLastPTic = ThisTic;
		
		// Frozen Demo
		if (Diff)
			if (g_DemoFreezeTics)
			{
				D_XFakePlayerTicker();	// Tic fake camera for effects
				g_DemoFreezeTics--;
				return 0;
			}
		
		// If the difference is too great, cap it
			// i.e. debug break, moving window, etc. doesn't cause the game to
			// catchup for all the time that was lost
		if (Diff > l_SVMaxDemoCatchup.Value->Int)
			return l_SVMaxDemoCatchup.Value->Int + 1;
		return Diff;
	}
	
	/* We are the server */
	else if (D_XNetIsServer())
	{
		// Timing Server
		if (singletics)
			return 1;
		
		// See if everyone is not lagging behind
			// If not, freeze the game until everyone catches up some
		
		// Clients need to catchup
		if (false)
		{
			l_XNLastPTic = ThisTic;
			return 0;
		}
			
		// Get time difference
		Diff = ThisTic - l_XNLastPTic;
		l_XNLastPTic = ThisTic;
		
		// If the difference is too great, cap it
			// i.e. debug break, moving window, etc. doesn't cause the game to
			// catchup for all the time that was lost
		if (Diff > l_SVMaxCatchup.Value->Int)
			return l_SVMaxCatchup.Value->Int + 1;
		return Diff;
	}
	
	/* We are the client */
	else
	{
		// If time smoothing is enabled...
			// If running to far behind, execute more tics
			// If running within threshold, execute tic at certain time
			// so that the client game is not jerky and is consistent.
		if (false)
		{
		}
		
		// Otherwise, return the amount of tics that are in queue
		else
		{
		}
	}
	
	/* Fell through? */
	return 0;
}

/* DS_PBAddBot() -- Adds a bot */
static void DS_PBAddBot(D_XPlayer_t* const a_Player, void* const a_Data)
{
	B_BotTemplate_t* BotTemp;
	
	/* Set Initial Flags */
	a_Player->Flags |= DXPF_BOT | DXPF_NOLOGIN | DXPF_LOCAL;
	
	/* Init Bot */
	BotTemp = a_Data;
	a_Player->BotData = B_InitBot(BotTemp);
	
	strncpy(a_Player->AccountName, BotTemp->AccountName, MAXPLAYERNAME);
	strncpy(a_Player->DisplayName, BotTemp->DisplayName, MAXPLAYERNAME);
}

/* DS_XNetCon() -- Command */
static int DS_XNetCon(const uint32_t a_ArgC, const char** const a_ArgV)
{
	B_BotTemplate_t* BotTemp;
	
	/* Not enough args? */
	if (a_ArgC < 2)
	{
		return 1;
	}
	
	/* Adding Bot */
	if (strcasecmp(a_ArgV[1], "addbot") == 0)
	{
		// Not server?
		if (!D_XNetIsServer())
			return 1;
		
		// Use Bot Template?
		BotTemp = NULL;
		if (a_ArgC >= 3)
			BotTemp = B_GHOST_FindTemplate(a_ArgV[2]);
		
		// Random?
		if (!BotTemp)
			BotTemp = B_GHOST_RandomTemplate();
		
		// Add Player
		D_XNetAddPlayer(DS_PBAddBot, BotTemp);
		
		// Success?
		return 0;
	}
	
	/* Failure */
	return 1;
}

/* D_XNetInit() -- Initializes the Extended Network Code */
void D_XNetInit(void)
{
	CONL_AddCommand("xnet", DS_XNetCon);
}

/* D_XNetHandleEvent() -- Handle advanced events */
bool_t D_XNetHandleEvent(const I_EventEx_t* const a_Event)
{
	int32_t ButtonNum, LocalJoy;
	uint32_t Bit;
	
	/* Check */
	if (!a_Event)
		return false;
	
	/* Clear events if not playing */
	if (gamestate == GS_DEMOSCREEN || demoplayback)
	{
		memset(l_MouseMove, 0, sizeof(l_MouseMove));
		memset(l_KeyDown, 0, sizeof(l_KeyDown));
		memset(l_JoyButtons, 0, sizeof(l_JoyButtons));
		memset(l_JoyAxis, 0, sizeof(l_JoyAxis));
		return false;
	}
	
	/* Which kind of event? */
	switch (a_Event->Type)
	{
			// Mouse
		case IET_MOUSE:
			// Add position to movement
			l_MouseMove[0] += a_Event->Data.Mouse.Move[0];
			l_MouseMove[1] += a_Event->Data.Mouse.Move[1];
			
			// Handling of buttons (with double click)
			if (a_Event->Data.Mouse.Button > 0 && a_Event->Data.Mouse.Button < 32)
			{
				// Determine bit
				ButtonNum = a_Event->Data.Mouse.Button - 1U;
				Bit = 1U << (ButtonNum);
				
				// Unpressed?
				if (!a_Event->Data.Mouse.Down)
				{
					l_MouseButtons[0] &= ~Bit;
					l_MouseButtons[1] &= ~Bit;
				}
				else
				{
					// Always set single bit
					l_MouseButtons[0] |= Bit;
					
					// Double Click?
						// TODO make this a CVAR of sorts
					if (g_ProgramTic - l_MouseLastTime[ButtonNum] < 17)
					{
						l_MouseButtons[1] |= Bit;
						l_MouseLastTime[ButtonNum] = 0;
					}
				
					// Single Click (set last time for double)
					else
						l_MouseLastTime[ButtonNum] = g_ProgramTic;
				}
			}
			return true;
			
			// Keyboard
		case IET_KEYBOARD:
			if (a_Event->Data.Keyboard.KeyCode >= 0 && a_Event->Data.Keyboard.KeyCode < NUMIKEYBOARDKEYS)
				l_KeyDown[a_Event->Data.Keyboard.KeyCode] = a_Event->Data.Keyboard.Down;
			return true;
			
			// Joystick
		case IET_JOYSTICK:
			// Get local joystick
			LocalJoy = a_Event->Data.Joystick.JoyID;
			
			// Now determine which action
			if (LocalJoy >= 0 && LocalJoy < MAXLOCALJOYS)
			{
				// Not bound? Then remove anything remembered (prevents stuckness)
				if (!D_JoyToPort(LocalJoy + 1))
				{
					l_JoyButtons[LocalJoy] = 0;
					memset(&l_JoyAxis[LocalJoy], 0, sizeof(l_JoyAxis[LocalJoy]));
					break;
				}
				
				// Button Pressed Down
				if (a_Event->Data.Joystick.Button)
				{
					// Get Number
					ButtonNum = a_Event->Data.Joystick.Button;
					ButtonNum--;
					
					// Limited to 32 buttons =(
					if (ButtonNum >= 0 && ButtonNum < 32)
					{
						// Was it pressed?
						if (a_Event->Data.Joystick.Down)
							l_JoyButtons[LocalJoy] |= (1 << ButtonNum);
						else
							l_JoyButtons[LocalJoy] &= ~(1 << ButtonNum);
					}
				}
				
				// Axis Moved
				else if (a_Event->Data.Joystick.Axis)
				{
					ButtonNum = a_Event->Data.Joystick.Axis;
					ButtonNum--;
					
					if (ButtonNum >= 0 && ButtonNum < MAXJOYAXIS)
						l_JoyAxis[LocalJoy][ButtonNum] = a_Event->Data.Joystick.Value;
				}
			}
			return true;
		
			// Unknown
		default:
			break;
	}
	
	/* Un-Handled */
	return false;
}

/* NextWeapon() -- Finds the next weapon in the chain */
// This is for PrevWeapon and NextWeapon
// Rewritten for RMOD Support!
// This uses the fields in weaponinfo_t for ordering info
static uint8_t DS_XNetNextWeapon(player_t* player, int step)
{
	size_t g, w, fw, BestNum;
	int32_t s, StepsLeft, StepsAdd, BestDiff, ThisDiff;
	size_t MostOrder, LeastOrder;
	bool_t Neg;
	weaponinfo_t** weapons;
	
	/* No player? */
	if (!player)
		return 0;
	
	/* Get current weapon info */
	weapons = player->weaponinfo;
	
	/* Get the weapon with the lowest and highest order */
	// Find first gun the player has (so order is correct)
	MostOrder = LeastOrder = 0;
	for (w = 0; w < NUMWEAPONS; w++)
		if (P_CanUseWeapon(player, w))
		{
			// Got the first available gun
			MostOrder = LeastOrder = w;
			break;
		}
	
	// Now go through
	for (w = 0; w < NUMWEAPONS; w++)
	{
		// Can't use this gun?
		if (!P_CanUseWeapon(player, w))
			continue;
		
		// Least
		if (weapons[w]->SwitchOrder < weapons[LeastOrder]->SwitchOrder)
			LeastOrder = w;
		
		// Most
		if (weapons[w]->SwitchOrder > weapons[MostOrder]->SwitchOrder)
			MostOrder = w;
	}
	
	/* Look for the current weapon in the weapon list */
	// Well that was easy
	fw = s = g = player->readyweapon;
	
	/* Constantly change the weapon */
	// Prepare variables
	Neg = (step < 0 ? true : false);
	StepsAdd = (Neg ? -1 : 1);
	StepsLeft = step * StepsAdd;
	
	// Go through the weapon list, step times
	while (StepsLeft > 0)
	{
		// Clear variables
		BestDiff = 9999999;		// The worst weapon difference ever
		BestNum = NUMWEAPONS;
		
		// Go through every weapon and find the next in the order
		for (w = 0; w < NUMWEAPONS; w++)
		{
			// Ignore the current weapon (don't want to switch back to it)
			if (w == fw)		// Otherwise BestDiff is zero!
				continue;
			
			// Can't use this gun?
			if (!P_CanUseWeapon(player, w))
				continue;
			
			// Only consider worse/better weapons?
			if ((Neg && weapons[w]->SwitchOrder > weapons[fw]->SwitchOrder) || (!Neg && weapons[w]->SwitchOrder < weapons[fw]->SwitchOrder))
				continue;
			
			// Get current diff
			ThisDiff = abs(weapons[fw]->SwitchOrder - weapons[w]->SwitchOrder);
			
			// Closer weapon?
			if (ThisDiff < BestDiff)
			{
				BestDiff = ThisDiff;
				BestNum = w;
			}
		}
		
		// Found no weapon? Then "loop" around
		if (BestNum == NUMWEAPONS)
		{
			// Switch to the highest gun if going down
			if (Neg)
				fw = MostOrder;
			
			// And if going up, go to the lowest
			else
				fw = LeastOrder;
		}
		
		// Found a weapon
		else
		{
			// Switch to this gun
			fw = BestNum;
		}
		
		// Next step
		StepsLeft--;
	}
	
	/* Return the weapon we want */
	return fw;
}

/* GAMEKEYDOWN() -- Checks if a key is down */
static bool_t GAMEKEYDOWN(D_ProfileEx_t* const a_Profile, const uint8_t a_SID, const uint8_t a_Key)
{
	static bool_t Recoursed;
	bool_t MoreDown;
	size_t i;
	uint32_t CurrentButton;
	
	/* Determine if more key is down */
	// But do not infinite loop here
	MoreDown = false;
	if (!Recoursed)
	{
		Recoursed = true;
		MoreDown = GAMEKEYDOWN(a_Profile, a_SID, DPEXIC_MORESTUFF);
		Recoursed = false;
	}
	
	/* Check Keyboard */
	for (i = 0; i < 4; i++)
		if (a_Profile->Ctrls[a_Key][i] >= 0 && a_Profile->Ctrls[a_Key][i] < NUMIKEYBOARDKEYS)
			if (l_KeyDown[a_Profile->Ctrls[a_Key][i]])
				return true;
	
	/* Check Joysticks */
	//if (a_Profile->Flags & DPEXF_GOTJOY)
		//if (a_Profile->JoyControl >= 0 && a_Profile->JoyControl < 4)
	if (a_SID >= 0 && a_SID < MAXSPLITSCREEN && g_Splits[a_SID].JoyBound)
		if (g_Splits[a_SID].JoyID >= 1 && g_Splits[a_SID].JoyID <= MAXLOCALJOYS)
			if (l_JoyButtons[g_Splits[a_SID].JoyID - 1])
				for (i = 0; i < 4; i++)
					if ((a_Profile->Ctrls[a_Key][i] & 0xF000) == (MoreDown ? 0x4000 : 0x1000))
					{
						// Get current button
						CurrentButton = (a_Profile->Ctrls[a_Key][i] & 0x00FF);
				
						// Button pressed?
						if (CurrentButton >= 0 && CurrentButton < 32)
							if (l_JoyButtons[g_Splits[a_SID].JoyID - 1] & (1 << CurrentButton))
								return true;
					}
				
	/* Check Mice */
	if (a_Profile->Flags & DPEXF_GOTMOUSE)
		if (l_MouseButtons[0] || l_MouseButtons[1])
			for (i = 0; i < 4; i++)
			{
				// Single
				if ((a_Profile->Ctrls[a_Key][i] & 0xF000) == (MoreDown ? 0x5000 : 0x2000))
				{
					// Get current button
					CurrentButton = (a_Profile->Ctrls[a_Key][i] & 0x00FF);
		
					// Button pressed?
					if (CurrentButton >= 0 && CurrentButton < 32)
						if (l_MouseButtons[0] & (1 << CurrentButton))
							return true;
				}
		
				// Double
				if ((a_Profile->Ctrls[a_Key][i] & 0xF000) == (MoreDown ? 0x6000 : 0x3000))
				{
					// Get current button
					CurrentButton = (a_Profile->Ctrls[a_Key][i] & 0x00FF);
		
					// Button pressed?
					if (CurrentButton >= 0 && CurrentButton < 32)
						if (l_MouseButtons[1] & (1 << CurrentButton))
							return true;
				}
			}
	
	/* Not pressed */
	return false;
}

short G_ClipAimingPitch(int* aiming);

/* D_XNetBuildTicCmd() -- Builds tic command for player */
void D_XNetBuildTicCmd(D_XPlayer_t* const a_NPp, ticcmd_t* const a_TicCmd)
{
#define MAXWEAPONSLOTS 12
	D_ProfileEx_t* Profile;
	player_t* Player;
	int32_t TargetMove;
	size_t i, PID, SID;
	int8_t SensMod, MoveMod, MouseMod, MoveSpeed, TurnSpeed;
	int32_t SideMove, ForwardMove, BaseAT, BaseAM;
	bool_t IsTurning, GunInSlot, ResetAim;
	int slot, j, l, k;
	weapontype_t newweapon;
	weapontype_t SlotList[MAXWEAPONSLOTS];
	
	/* Check */
	if (!a_NPp || !a_TicCmd)
		return;
	
	/* Obtain profile */
	Profile = a_NPp->Profile;
	Player = a_NPp->Player;
	
	// No profile?
	if (!Profile)
		return;
	
	/* Find Player ID */
	PID = a_NPp->Player - players;
	
	/* Find Screen ID */
	for (SID = 0; SID < MAXSPLITSCREEN; SID++)
		if (D_ScrSplitVisible(SID))
			if (g_Splits[SID].XPlayer == a_NPp)
				break;
	
	// Not found?
	if (SID >= MAXSPLITSCREEN)
		return;
	
	/* Reset Some Things */
	SideMove = ForwardMove = BaseAT = BaseAM = 0;
	IsTurning = ResetAim = false;
	
	/* Modifiers */
	// Mouse Sensitivity
	SensMod = 0;
	
	// Movement Modifier
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_MOVEMENT))
		MoveMod = 1;
	else
		MoveMod = 0;
	
	// Mouse Modifier
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_LOOKING))
		MouseMod = 2;
	else if (MoveMod)
		MouseMod = 1;
	else 
		MouseMod = 0;
	
	// Moving Speed
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_SPEED))
		MoveSpeed = 1;
	else
		MoveSpeed = 0;
	
	// Turn Speed
	if ((Profile->Flags & DPEXF_SLOWTURNING) &&
			gametic < (a_NPp->TurnHeld + Profile->SlowTurnTime))
		TurnSpeed = 2;
	else if (MoveSpeed)
		TurnSpeed = 1;
	else
		TurnSpeed = 0;
	
	/* Player has joystick input? */
	// Read input for all axis
	if (SID >= 0 && SID < MAXSPLITSCREEN && g_Splits[SID].JoyBound)
		if (g_Splits[SID].JoyID >= 1 && g_Splits[SID].JoyID <= MAXLOCALJOYS)
			for (i = 0; i < MAXJOYAXIS; i++)
			{
				// Modify with sensitivity
				TargetMove = ((float)l_JoyAxis[g_Splits[SID].JoyID - 1][i]) * (((float)Profile->JoySens[SensMod]) / 100.0);
			
				// Which movement to perform?
				switch (Profile->JoyAxis[MouseMod][i])
				{
						// Movement
					case DPEXCMA_MOVEX:
					case DPEXCMA_MOVEY:
						// Movement is fractionally based
						TargetMove = (((float)TargetMove) / ((float)32767.0)) * ((float)c_forwardmove[MoveSpeed]);
					
						// Now which action really?
						if (Profile->JoyAxis[MouseMod][i] == DPEXCMA_MOVEX)
							SideMove += TargetMove;
						else
							ForwardMove -= TargetMove;
						break;
					
						// Looking Left/Right
					case DPEXCMA_LOOKX:
						TargetMove = (((float)TargetMove) / ((float)32767.0)) * ((float)c_angleturn[TurnSpeed]);
						IsTurning = true;
						BaseAT -= TargetMove;
						break;
					
						// Looking Up/Down
					case DPEXCMA_LOOKY:
						break;
				
					default:
						break;
				}
			}
	
	/* Player has mouse input? */
	if (l_PermitMouse && (Profile->Flags & DPEXF_GOTMOUSE))
	{
		// Read mouse input for both axis
		for (i = 0; i < 2; i++)
		{
			// Modify with sensitivity
			TargetMove = l_MouseMove[i] * ((((float)(Profile->MouseSens[SensMod] * Profile->MouseSens[SensMod])) / 110.0) + 0.1);
			
			// Do action for which movement type?
			switch (Profile->MouseAxis[MouseMod][i])
			{
					// Strafe Left/Right
				case DPEXCMA_MOVEX:
					SideMove += TargetMove;
					break;
					
					// Move Forward/Back
				case DPEXCMA_MOVEY:
					ForwardMove += TargetMove;
					break;
					
					// Left/Right Look
				case DPEXCMA_LOOKX:
					BaseAT -= TargetMove * 8;
					break;
					
					// Up/Down Look
				case DPEXCMA_LOOKY:
					BaseAM += TargetMove << 3;
					//localaiming[SID] += TargetMove << 19;
					break;
				
					// Unknown
				default:
					break;
			}
		}
		
		// Clear mouse permission
		l_PermitMouse = false;
		
		// Clear mouse input
		l_MouseMove[0] = l_MouseMove[1] = 0;
	}
	
	/* Handle Player Control Keyboard Stuff */
	// Weapon Attacks
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_ATTACK))
		a_TicCmd->Std.buttons |= BT_ATTACK;
	
	// Use
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_USE))
		a_TicCmd->Std.buttons |= BT_USE;
	
	// Jump
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_JUMP))
		a_TicCmd->Std.buttons |= BT_JUMP;
	
	// Suicide
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_SUICIDE))
		a_TicCmd->Std.buttons |= BT_SUICIDE;
	
	// Keyboard Turning
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_TURNLEFT))
	{
		// Strafe
		if (MoveMod)
			SideMove -= c_sidemove[MoveSpeed];
		
		// Turn
		else
		{
			BaseAT += c_angleturn[TurnSpeed];
			IsTurning = true;
		}
	}
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_TURNRIGHT))
	{
		// Strafe
		if (MoveMod)
			SideMove += c_sidemove[MoveSpeed];
		
		// Turn
		else
		{
			BaseAT -= c_angleturn[TurnSpeed];
			IsTurning = true;
		}
	}
	
	// Keyboard Moving
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_STRAFELEFT))
		SideMove -= c_sidemove[MoveSpeed];
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_STRAFERIGHT))
		SideMove += c_sidemove[MoveSpeed];
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_FORWARDS))
		ForwardMove += c_forwardmove[MoveSpeed];
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_BACKWARDS))
		ForwardMove -= c_forwardmove[MoveSpeed];
		
	// Looking
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_LOOKCENTER))
		ResetAim = true;
		//localaiming[SID] = 0;
	else
	{
		if (GAMEKEYDOWN(Profile, SID, DPEXIC_LOOKUP))
			BaseAM += Profile->LookUpDownSpeed >> 16;
			//localaiming[SID] += Profile->LookUpDownSpeed;
		
		if (GAMEKEYDOWN(Profile, SID, DPEXIC_LOOKDOWN))
			BaseAM -= Profile->LookUpDownSpeed >> 16;
			//localaiming[SID] -= Profile->LookUpDownSpeed;
	}
	
	// Weapons
	if (Player)
	{
		// Next
		if (GAMEKEYDOWN(Profile, SID, DPEXIC_NEXTWEAPON))
		{
			// Set switch
			a_TicCmd->Std.buttons |= BT_CHANGE;
			D_TicCmdFillWeapon(a_TicCmd, DS_XNetNextWeapon(Player, 1));
		}
		// Prev
		else if (GAMEKEYDOWN(Profile, SID, DPEXIC_PREVWEAPON))
		{
			// Set switch
			a_TicCmd->Std.buttons |= BT_CHANGE;
			D_TicCmdFillWeapon(a_TicCmd, DS_XNetNextWeapon(Player, -1));
		}
		// Best Gun
		else if (GAMEKEYDOWN(Profile, SID, DPEXIC_BESTWEAPON))
		{
			newweapon = P_PlayerBestWeapon(Player, true);
		
			if (newweapon != Player->readyweapon)
			{
				a_TicCmd->Std.buttons |= BT_CHANGE;
				D_TicCmdFillWeapon(a_TicCmd, newweapon);
			}
		}
		// Worst Gun
		else if (GAMEKEYDOWN(Profile, SID, DPEXIC_WORSTWEAPON))
		{
			newweapon = P_PlayerBestWeapon(Player, false);
		
			if (newweapon != Player->readyweapon)
			{
				a_TicCmd->Std.buttons |= BT_CHANGE;
				D_TicCmdFillWeapon(a_TicCmd, newweapon);
			}
		}
		// Slots
		else
		{
			// Which slot?
			slot = -1;
		
			// Look for keys
			for (i = DPEXIC_SLOT1; i <= DPEXIC_SLOT10; i++)
				if (GAMEKEYDOWN(Profile, SID, i))
				{
					slot = (i - DPEXIC_SLOT1) + 1;
					break;
				}
		
			// Hit slot?
			if (slot != -1)
			{
				// Clear flag
				GunInSlot = false;
				l = 0;
		
				// Figure out weapons that belong in this slot
				for (j = 0, i = 0; i < NUMWEAPONS; i++)
					if (P_CanUseWeapon(Player, i))
					{
						// Weapon not in this slot?
						if (Player->weaponinfo[i]->SlotNum != slot)
							continue;
				
						// Place in slot list before the highest
						if (j < (MAXWEAPONSLOTS - 1))
						{
							// Just place here
							if (j == 0)
							{
								// Current weapon is in this slot?
								if (Player->readyweapon == i)
								{
									GunInSlot = true;
									l = j;
								}
						
								// Place in last spot
								SlotList[j++] = i;
							}
					
							// Otherwise more work is needed
							else
							{
								// Start from high to low
									// When the order is lower, we know to insert now
								for (k = 0; k < j; k++)
									if (Player->weaponinfo[i]->SwitchOrder < Player->weaponinfo[SlotList[k]]->SwitchOrder)
									{
										// Current gun may need shifting
										if (!GunInSlot)
										{
											// Current weapon is in this slot?
											if (Player->readyweapon == i)
											{
												GunInSlot = true;
												l = k;
											}
										}
								
										// Possibly shift gun
										else
										{
											// If the current gun is higher then this gun
											// then it will be off by whatever is more
											if (Player->weaponinfo[SlotList[l]]->SwitchOrder > Player->weaponinfo[i]->SwitchOrder)
												l++;
										}
								
										// move up
										memmove(&SlotList[k + 1], &SlotList[k], sizeof(SlotList[k]) * (MAXWEAPONSLOTS - k - 1));
								
										// Place in slightly upper spot
										SlotList[k] = i;
										j++;
								
										// Don't add it anymore
										break;
									}
						
								// Can't put it anywhere? Goes at end then
								if (k == j)
								{
									// Current weapon is in this slot?
									if (Player->readyweapon == i)
									{
										GunInSlot = true;
										l = k;
									}
							
									// Put
									SlotList[j++] = i;
								}
							}
						}
					}
		
				// No guns in this slot? Then don't switch to anything
				if (j == 0)
					newweapon = Player->readyweapon;
		
				// If the current gun is in this slot, go to the next in the slot
				else if (GunInSlot)		// from [best - worst]
					newweapon = SlotList[((l - 1) + j) % j];
		
				// Otherwise, switch to the best gun there
				else
					// Set it to the highest valued gun
					newweapon = SlotList[j - 1];
		
				// Did it work?
				if (newweapon != Player->readyweapon)
				{
					a_TicCmd->Std.buttons |= BT_CHANGE;
					D_TicCmdFillWeapon(a_TicCmd, newweapon);
				}
			}
		}
	}
	
	// Inventory
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_NEXTINVENTORY))
		a_TicCmd->Std.InventoryBits = TICCMD_INVRIGHT;
	else if (GAMEKEYDOWN(Profile, SID, DPEXIC_PREVINVENTORY))
		a_TicCmd->Std.InventoryBits = TICCMD_INVLEFT;
	else if (GAMEKEYDOWN(Profile, SID, DPEXIC_USEINVENTORY))
		a_TicCmd->Std.InventoryBits = TICCMD_INVUSE;
	
	/* Handle special functions */
	// Show Scores
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_TOPSCORES))
		a_NPp->Scores = 1;
	else if (GAMEKEYDOWN(Profile, SID, DPEXIC_BOTTOMSCORES))
		a_NPp->Scores = -1;
	else
		a_NPp->Scores = 0;
	
	// Coop Spy
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_COOPSPY))
	{
		// Only every half second
		if (gametic > (a_NPp->CoopSpyTime + (TICRATE >> 1)))
		{
			do
			{
				g_Splits[SID].Display = (g_Splits[SID].Display + 1) % MAXPLAYERS;
			} while (!playeringame[g_Splits[SID].Display] || !P_PlayerOnSameTeam(&players[g_Splits[SID].Console], &players[g_Splits[SID].Display]));
			
			// Print Message
			CONL_PrintF("%sYou are now watching %s.\n",
					(SID == 3 ? "\x6" : (SID == 2 ? "\x5" : (SID == 1 ? "\x4" : ""))),
					(g_Splits[SID].Display == g_Splits[SID].Console ? "Yourself" : D_NCSGetPlayerName(g_Splits[SID].Display))
				);
			
			// Reset timeout
			a_NPp->CoopSpyTime = gametic + (TICRATE >> 1);
		}
	}
	
	// Key is unpressed to reduce time
	else
		a_NPp->CoopSpyTime = 0;
	
	/* Set Movement Now */
	// Cap
	if (SideMove > MAXPLMOVE)
		SideMove = MAXPLMOVE;
	else if (SideMove < -MAXPLMOVE)
		SideMove = -MAXPLMOVE;
	
	if (ForwardMove > MAXPLMOVE)
		ForwardMove = MAXPLMOVE;
	else if (ForwardMove < -MAXPLMOVE)
		ForwardMove = -MAXPLMOVE;
	
	// Set
	a_TicCmd->Std.sidemove = SideMove;
	a_TicCmd->Std.forwardmove = ForwardMove;
	
	/* Slow turning? */
	if (!IsTurning)
		a_NPp->TurnHeld = gametic;
	
	/* Turning */
	a_TicCmd->Std.BaseAngleTurn = BaseAT;
	a_TicCmd->Std.BaseAiming = BaseAM;
	a_TicCmd->Std.ResetAim = ResetAim;
	
#undef MAXWEAPONSLOTS
}

/* D_XNetUpdate() -- Updates Extended Network */
void D_XNetUpdate(void)
{
	int32_t i, a, j, ScrID;
	D_XPlayer_t* XPlay;
	ticcmd_t* TicCmdP;
	M_UIMenu_t* ProfMenu;
	
	/* Not playing? */
	if (gamestate == GS_DEMOSCREEN || demoplayback)
		return;
	
	/* Handle local players and splits */ 
	if (!demoplayback && gamestate != GS_DEMOSCREEN)
	{
		// Players in split, but not mapped player?
		for (a = 0, i = 0; i < MAXSPLITSCREEN; i++)
			if (D_ScrSplitHasPlayer(i))
			{
				a++;
				
				if (!g_Splits[i].XPlayer)
					g_Splits[i].XPlayer = D_XNetLocalPlayerByPID(g_Splits[i].ProcessID);
			}
		
		// No local players? Give a local player, if possible
		if (!a)
		{
			for (i = 0; i < g_NumXPlays; i++)
			{
				// Get player
				XPlay = g_XPlays[i];
				
				// Missing?
				if (!XPlay)
					continue;
				
				// Local and not a bot?
				if ((XPlay->Flags & (DXPF_LOCAL | DXPF_BOT)) == DXPF_LOCAL)
				{
					XPlay->ScreenID = 0;
					g_Splits[0].Waiting = true;
					g_Splits[0].XPlayer = XPlay;
					g_Splits[0].ProcessID = XPlay->ID;
					g_SplitScreen = 0;
				}
			}
		}
	}
	
	/* Enable Mouse Input */
	l_PermitMouse = true;
	
	/* Build Local Tic Commands (possibly) */
	for (i = 0; i < g_NumXPlays; i++)
	{
		// Get current player
		XPlay = g_XPlays[i];
		
		// Missing?
		if (!XPlay)
			continue;
		
		// Non-local player?
		if (!(XPlay->Flags & DXPF_LOCAL))
			continue;
		
		// Map to screen
		for (ScrID = 0; ScrID < MAXSPLITSCREEN; ScrID++)
			if (XPlay == g_Splits[ScrID].XPlayer)
				break;
		
		// Illegal?
		if (ScrID >= MAXSPLITSCREEN)
			ScrID = -1;
		
		// No profile loaded?
		if (ScrID >= 0)
			if (!XPlay->Profile)
			{
				// Not in level
				if (gamestate != GS_LEVEL)
					continue;
				
				// Set as needing a profile
				XPlay->StatusBits |= DXPSB_NEEDSPROFILE;
				
				// Check players menus for profile prompt
				ProfMenu = M_ExMakeMenu(M_ExMenuIDByName("profileselect"), NULL);
				
				// Failed?
				if (!ProfMenu)
					continue;
				
				// See where the menu is
				a = M_ExFirstMenuSpot(ScrID, ProfMenu);
			
				// Not in first spot?
				if (a != 0)
				{
					// Clear all menus and place there
					M_ExPopAllMenus(ScrID);
					M_ExPushMenu(ScrID, ProfMenu);
					g_ResumeMenu++;
				}
				continue;
			}
		
		// Does not need profile
		XPlay->StatusBits &= ~DXPSB_NEEDSPROFILE;
		
		// Place tic command at last spot, when possible
		TicCmdP = NULL;
		if (XPlay->LocalAt < MAXLBTSIZE - 1)
			TicCmdP = &XPlay->LocalBuf[XPlay->LocalAt++];
		else
		{
			TicCmdP = &XPlay->LocalBuf[0];
			XPlay->StatusBits |= DXPSB_LBOVERFLOW;
		}
		
		// Clear overflow bit
		XPlay->StatusBits &= ~DXPSB_LBOVERFLOW;
		
		// Bot Player?
		if (XPlay->Flags & DXPF_BOT)
			B_BuildBotTicCmd(XPlay, XPlay->BotData, TicCmdP);
		
		// Human player
		else
			D_XNetBuildTicCmd(XPlay, TicCmdP);
		
		// No player and use is down? Attempt joining
		if (!XPlay->Player && g_ProgramTic >= XPlay->LastJoinAttempt)
			if (TicCmdP->Std.buttons & BT_USE)
			{
				XPlay->LastJoinAttempt = g_ProgramTic + TICRATE;
				D_XNetTryJoin(XPlay);
			}
	}
}

/*** FAKE PLAYER ***/

static player_t l_XFakePlayer[MAXSPLITSCREEN];	// Fake Player
static mobj_t l_XFakeMobj[MAXSPLITSCREEN];		// Fake Mobj

/* D_XFakePlayerInit() -- Initializes the fake player */
void D_XFakePlayerInit(void)
{
	int i;
	mapthing_t* MapThing;
	mobj_t* AnotherMo;
	subsector_t* SubS;
	
	/* Reset */
	memset(l_XFakePlayer, 0, sizeof(l_XFakePlayer));
	memset(l_XFakeMobj, 0, sizeof(l_XFakeMobj));
	
	/* Find a map thing to initialize the view from */
	MapThing = NULL;
	AnotherMo = NULL;
	
	// Try existing players
	for (i = 0; i < MAXPLAYERS && !AnotherMo; i++)
		if (playeringame[i])
			if (players[i].mo)
				AnotherMo = players[i].mo;
	
	// Try player starts first
	if (!MapThing)
		for (i = 0; i < MAXPLAYERS && !MapThing; i++)
			MapThing = playerstarts[i];
	
	// Then deathmatch starts
	if (!MapThing)
		for (i = 0; i < MAX_DM_STARTS && !MapThing; i++)
			MapThing = deathmatchstarts[i];
	
	/* Initialize each player */
	for (i = 0; i < MAXSPLITSCREEN; i++)
	{
		// Bind objects
		l_XFakePlayer[i].mo = &l_XFakeMobj[i];
		l_XFakeMobj[i].player = &l_XFakePlayer[i];
		
		// Player has object
		if (AnotherMo)
		{
			l_XFakeMobj[i].x = AnotherMo->x;
			l_XFakeMobj[i].y = AnotherMo->y;
			l_XFakeMobj[i].z = AnotherMo->z + (AnotherMo->height >> 1);
		}
		
		// Only if MapThing is set
		else if (MapThing)
		{
			// Set Initial Position
			l_XFakeMobj[i].x = MapThing->x;
			l_XFakeMobj[i].y = MapThing->y;
		
			// Calculate Z position
			l_XFakeMobj[i].subsector = SubS = R_PointInSubsector(l_XFakeMobj[i].x, l_XFakeMobj[i].y);
			l_XFakeMobj[i].z = SubS->sector->floorheight + ((SubS->sector->ceilingheight + SubS->sector->floorheight) >> 1);
		}
		
		// Correct View Hieght
		l_XFakePlayer[i].viewz = l_XFakeMobj[i].z;
	}
}

/* D_XFakePlayerGet() -- Returns the fake player */
struct player_s* D_XFakePlayerGet(const int32_t a_Screen)
{
	return &l_XFakePlayer[a_Screen];
}

/* D_XFakePlayerTicker() -- Ticks fake players */
void D_XFakePlayerTicker(void)
{
#define TSCAMDIST FIXEDT_C(128)
#define BUDGEDIST FIXEDT_C(32)
#define TSMOVEUNIT FIXEDT_C(16)
	int32_t i;
	player_t* Mod, *VPlay;
	mobj_t* ChaseThis, *PeerThis, *CamMo, *AttackSpec;
	fixed_t Dist, DistX, DistY, ToMove, MyAng, TargAng;
	fixed_t VeerX, VeerY;
	angle_t Angle, PeerAngle;
	bool_t DeadView;
	
	fixed_t ToDist, ToPos[0], BCPos[2], CDist, BCDist;
	
	/* Title Screen Demo */
	if (g_TitleScreenDemo)
	{
		// For each player split
		for (i = 0; i < MAXSPLITSCREEN; i++)
			if (g_Splits[i].Active)
			{
				// Player to modify
				Mod = &l_XFakePlayer[i];
				VPlay = &players[g_Splits[i].Console];
				
				// Camera Object
				CamMo = Mod->mo;
				
				// Get subsector
				CamMo->subsector = R_PointInSubsector(CamMo->x, CamMo->y);
				
				// Chase the player's object
				ChaseThis = VPlay->mo;
				
				// Player is alive and has a BFG ball
				if (VPlay->LastBFGBall && VPlay->health > 0)
				{
					// Chase and look after the ball
					PeerThis = ChaseThis = VPlay->LastBFGBall;
				}
				
				// Player is alive
				else if (VPlay->health > 0)
				{
					DeadView = false;
					
					// If player is under attack or attacking
					PeerThis = NULL;
					
					// First check attacker
					AttackSpec = VPlay->attacker;
					
					if (!(AttackSpec && AttackSpec->health > 0 &&
						P_CheckSight(VPlay->mo, AttackSpec)))
						AttackSpec = NULL;
					
					// Now check attackee
					AttackSpec = VPlay->Attackee;
					
					if (!(AttackSpec && AttackSpec->health > 0 &&
						P_CheckSight(VPlay->mo, AttackSpec)))
						AttackSpec = NULL;
					
					// It is OK
					if (AttackSpec)
					{
						// Get attacker distance
						Dist = P_AproxDistance(AttackSpec->x - ChaseThis->x,
								AttackSpec->y - ChaseThis->y);
					
						// Object close by
						if (Dist < FIXEDT_C(2048))
							PeerThis = AttackSpec;
					}
				
					// Otherwise stare at player object
					if (!PeerThis)
						PeerThis = VPlay->mo;
				}
				
				// Player is dead
				else
				{
					DeadView = true;
					
					// Chase killer
					ChaseThis = VPlay->attacker;
					
					if (!ChaseThis)
						ChaseThis = VPlay->mo;
					
					// Stare at player object
					PeerThis = VPlay->mo;
				}

#if 1
				// Get 128 units behind chase target
				BCPos[0] = ChaseThis->x + FixedMul(
						finecosine[ChaseThis->angle >> ANGLETOFINESHIFT],
						-TSCAMDIST
					);
				BCPos[1] = ChaseThis->y + FixedMul(
						finesine[ChaseThis->angle >> ANGLETOFINESHIFT],
						-TSCAMDIST
					);
				
				// Get Distance to chase target
				CDist = P_AproxDistance(
							CamMo->x - ChaseThis->x,
							CamMo->y - ChaseThis->y
						);
				
				// Get Distance to behind target
				BCDist = P_AproxDistance(
							CamMo->x - BCPos[0],
							CamMo->y - BCPos[1]
						);
				
				// If too far away from target, move to it
					// Or it cannot be seen by the camera
				if (!P_CheckSight(CamMo, ChaseThis) || (CDist > TSCAMDIST + BUDGEDIST) && !VPlay->attackdown)
				{
					ToPos[0] = ChaseThis->x;
					ToPos[1] = ChaseThis->y;
					ToDist = CDist / 16;
				}
				
				// If really close, back off
#if 0
				else if (CDist < BUDGEDIST)
				{
					ToPos[0] = ChaseThis->x;
					ToPos[1] = ChaseThis->y;
					ToDist = -(CDist / 4);
				}
#endif
				
				// If close enough, move to behind target then
				else
				{
					ToPos[0] = BCPos[0];
					ToPos[1] = BCPos[1];
					
					if (VPlay->attackdown)
						ToDist = BCDist / 8;
					else
						ToDist = BCDist / 32;
				}
				
				// Move to location
				Angle = R_PointToAngle2(CamMo->x, CamMo->y,
					ToPos[0], ToPos[1]);
				CamMo->x += FixedMul(finecosine[Angle >> ANGLETOFINESHIFT], ToDist);
				CamMo->y += FixedMul(finesine[Angle >> ANGLETOFINESHIFT], ToDist);
#else
				// Get distance and angle to chase target
				Dist = P_AproxDistance(
						CamMo->x - ChaseThis->x,
						CamMo->y - ChaseThis->y
					);
				
				
				// Move Camera closer to object chasing
				if (DeadView || Dist > TSCAMDIST)
				{
					Dist /= 16;
					
					CamMo->x += FixedMul(finecosine[Angle >> ANGLETOFINESHIFT], Dist);
					CamMo->y += FixedMul(finesine[Angle >> ANGLETOFINESHIFT], Dist);
				}
				
				// Slowly drift camera behind chase object
					// Location we want to drift to
				VeerX = ChaseThis->x + FixedMul(finecosine[
							ChaseThis->angle >> ANGLETOFINESHIFT], -TSCAMDIST);
				VeerY = ChaseThis->y + FixedMul(finesine[
							ChaseThis->angle >> ANGLETOFINESHIFT], -TSCAMDIST);
					// Get distance to point
				Dist = P_AproxDistance(
						CamMo->x - VeerX,
						CamMo->y - VeerY
					);
					// Reduce it alot!
				Dist /= 32;
					// Move
				CamMo->x += FixedMul(finecosine[ChaseThis->angle >> ANGLETOFINESHIFT], Dist);
				CamMo->y += FixedMul(finecosine[ChaseThis->angle >> ANGLETOFINESHIFT], Dist);
#endif
				Angle = R_PointToAngle2(CamMo->x, CamMo->y,
					ChaseThis->x, ChaseThis->y);
				PeerAngle = R_PointToAngle2(CamMo->x, CamMo->y,
					PeerThis->x, PeerThis->y);

				// Pan Camera to peer
				MyAng = TBL_BAMToDeg(CamMo->angle);
				TargAng = TBL_BAMToDeg(PeerAngle);
				
				// Get dual angles
				DistX = abs(MyAng - TargAng);
				DistY = abs((MyAng + 360) - TargAng);
				
				// Move in the smaller direction
				if (DistY < DistX)
					CamMo->angle += (ANGLE_1 * ((DistY / 4) >> FRACBITS));
				else
					CamMo->angle -= (ANGLE_1 * ((DistX / 4) >> FRACBITS));
				
				// Normalize Height
				Dist = (CamMo->z - (ChaseThis->z + (ChaseThis->height >> 1))) >> 2;
				CamMo->z -= Dist;
				
				// Raise above the floor
				CamMo->subsector = R_PointInSubsector(CamMo->x, CamMo->y);
				
#if 0
				if (CamMo->z < CamMo->subsector->sector->floorheight)
					CamMo->z = CamMo->subsector->sector->floorheight;
				else if (CamMo->z > CamMo->subsector->sector->ceilingheight)
					CamMo->z = CamMo->subsector->sector->ceilingheight;
#endif
				
				// Correct height
				Mod->viewz = CamMo->z;
			}
	}
	
	/* Normal fake */
	else
	{
	}
#undef TSCAMDIST
#undef TSMOVEUNIT
}

