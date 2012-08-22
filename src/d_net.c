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

/*************
*** LOCALS ***
*************/

static tic_t l_MapTime = 0;						// Map local time
static tic_t l_BaseTime = 0;					// Base Game Time
static tic_t l_LocalTime = 0;					// Local Time

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
	if (D_SyncNetIsSolo() && !demoplayback && (M_ExUIActive() || (l_CONPauseGame.Value->Int && CONL_IsActive())))
		return true;
	return false;
}

/* D_SyncNetIsSolo() -- Is solo game (non-networked) */
bool_t D_SyncNetIsSolo(void)
{
	return true;
}

/* D_SyncNetAllReady() -- Inidicates that all parties are ready to move to the next tic */
// It returns the tics in the future that everyone is ready to move to
tic_t D_SyncNetAllReady(void)
{
	tic_t ThisTime, DiffTime;
	
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
		return l_MapTime + 1;
	
	/* If we are the server, we dictate time */
	if (D_SyncNetIsArbiter())
	{
		// The map time is determined by the framerate
		ThisTime = (I_GetTimeMS() / TICSPERMS) - l_BaseTime;
		DiffTime = ThisTime - l_LocalTime;
		
		if (DiffTime > 0)
		{
			// Return the time difference
			l_LocalTime = ThisTime;
			return l_MapTime + DiffTime;
		}
		else
			return l_MapTime;
	}
	
	/* Otherwise time gets dictated to us */
	else
	{
		return l_MapTime;
	}
	
	/* Fell through? */
	return (tic_t)-1;
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

/*** LOCALS ***/

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

/*** FUNCTIONS ***/

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
	/* Check */
	if (!a_Player)
		return NULL;
}

/* D_NCFudgeOffHostStream() -- Fudge off host by stream */
void D_NCFudgeOffHostStream(I_HostAddress_t* const a_Host, struct D_BS_s* a_Stream, const char a_Code, const char* const a_Reason)
{
	/* Check */
	if (!a_Host || !a_Stream)
		return;
	
	/* Send FOFF Message */
	D_BSBaseBlock(a_Stream, "FOFF");
	D_BSwu8(a_Stream, a_Code);
	D_BSws(a_Stream, a_Reason);
	D_BSRecordNetBlock(a_Stream, a_Host);
	
	/* Destroy client structure */
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

/* DS_ConnectMultiCom() -- Connection multi-command */
static int DS_ConnectMultiCom(const uint32_t a_ArgC, const char** const a_ArgV)
{
	D_NetClient_t* ServerNC;
	I_HostAddress_t Host;
	
	/* Clear Host */
	memset(&Host, 0, sizeof(Host));
	
	/* Connect */
	if (strcasecmp("startserver", a_ArgV[0]) == 0)
	{
		D_NCServize();
		
		return CLE_SUCCESS;
	}
	
	else if (strcasecmp("connect", a_ArgV[0]) == 0)
	{
		// Not enough args?
		if (a_ArgC < 2)
		{
			CONL_PrintF("%s <address> (password) (join password)", a_ArgV[0]);
			return CLE_FAILURE;
		}
		
		// Get hostname
		if (!I_NetNameToHost(&Host, a_ArgV[1]))
		{
			CONL_OutputU(DSTR_NET_BADHOSTRESOLVE, "\n");
			return CLE_FAILURE;
		}
		
		// Attempt connect to server
		D_NCClientize(&Host, (a_ArgC >= 3 ? a_ArgV[2] : ""), (a_ArgC >= 4 ? a_ArgV[3] : ""));
		
		return CLE_SUCCESS;
	}
	
	/* Disconnect */
	else if (strcasecmp("disconnect", a_ArgV[0]) == 0)
	{
		// Disconnect
		D_NCDisconnect();
		
		// Success!
		return CLE_SUCCESS;
	}
	
	/* Reconnect */
	else if (strcasecmp("reconnect", a_ArgV[0]) == 0)
	{
		// Find server host
		ServerNC = D_NCFindClientIsServer();
		
		// No server found?
		if (!ServerNC)
			return CLE_FAILURE;
		
		// We are the server?
		if (ServerNC->IsServer && ServerNC->IsLocal)
		{
			CONL_OutputU(DSTR_NET_RECONNECTYOUARESERVER, "\n");
			return CLE_FAILURE;
		}
		
		// Copy host
		memmove(&Host, &ServerNC->Address, sizeof(Host));
		
		// Disconnect
		D_NCDisconnect();
		
		// Connect to server
			// TODO FIXME: Server Password
		D_NCClientize(&Host, NULL, NULL);
		
		// Success! I hope
		return CLE_SUCCESS;
	}
	
	/* Failure */
	return CLE_FAILURE;
}

/* D_CheckNetGame() -- Checks whether the game was started on the network */
bool_t D_CheckNetGame(void)
{
	I_NetSocket_t* Socket;
	D_NetClient_t* Client;
	bool_t ret = false;
	size_t i;
	
	// I_InitNetwork sets doomcom and netgame
	// check and initialize the network driver
	
	multiplayer = false;
	
	// only dos version with external driver will return true
	netgame = false;
	if (netgame)
		netgame = false;
	
	/* Register server commands */
	CONL_AddCommand("startserver", DS_ConnectMultiCom);
	CONL_AddCommand("connect", DS_ConnectMultiCom);
	CONL_AddCommand("disconnect", DS_ConnectMultiCom);
	CONL_AddCommand("reconnect", DS_ConnectMultiCom);
	
	/* Register variables */
	CONL_VarRegister(&l_SVName);
	CONL_VarRegister(&l_SVEMail);
	CONL_VarRegister(&l_SVURL);
	CONL_VarRegister(&l_SVWADURL);
	CONL_VarRegister(&l_SVIRC);
	CONL_VarRegister(&l_SVMOTD);
	CONL_VarRegister(&l_SVConnectPassword);
	CONL_VarRegister(&l_SVJoinPassword);
		
	/* Create LoopBack Client */
	Client = D_NCAllocClient();
	Client->IsLocal = true;
	Client->CoreStream = D_BSCreateLoopBackStream();
	
	// Create perfection Wrapper
	Client->PerfectStream = D_BSCreatePerfectStream(Client->CoreStream);
	
	// Set read/writes for all streams
	Client->Streams[DNCSP_READ] = Client->CoreStream;
	Client->Streams[DNCSP_WRITE] = Client->CoreStream;
	Client->Streams[DNCSP_PERFECTREAD] = Client->PerfectStream;
	Client->Streams[DNCSP_PERFECTWRITE] = Client->PerfectStream;
	
	/* Create Local Network Client */
	// Attempt creating a UDP Server
	Socket = NULL;
	for (i = 0; i < 20 && !Socket; i++)
		Socket = I_NetOpenSocket(true, NULL, __REMOOD_BASEPORT + i);
	
	// Initial input/output of stream
	if (Socket)
	{
		// Allocate local client
		Client = D_NCAllocClient();
		Client->IsLocal = true;
		
		// Copy socket
		Client->NetSock = Socket;
		
		// Create stream from it
		Client->CoreStream = D_BSCreateNetStream(Client->NetSock);
		
		// Create encapsulated perfect stream
		Client->PerfectStream = D_BSCreatePerfectStream(Client->CoreStream);
	
		// Set read/writes for all streams
		Client->Streams[DNCSP_READ] = Client->CoreStream;
		Client->Streams[DNCSP_WRITE] = Client->CoreStream;
		Client->Streams[DNCSP_PERFECTREAD] = Client->PerfectStream;
		Client->Streams[DNCSP_PERFECTWRITE] = Client->PerfectStream;
	}
	
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
	D_NCDisconnect();
}

/* D_NCQueueDisconnect() -- Queue Disconnect */
void D_NCQueueDisconnect(void)
{
	D_NCAddQueueCommand(DS_NCDoDisconnect, NULL);
}

/* D_NCDisconnect() -- Disconnect from existing server */
void D_NCDisconnect(void)
{
	size_t i, j;
	
	/* Demos? */
	if (demoplayback)
	{
		// Don't quit when the demo stops
		singledemo = false;
		
		// Stop it
		G_StopDemoPlay();
	}
	
	/* Clear all player information */
	// Just wipe ALL of it!
	memset(players, 0, sizeof(players));
	memset(playeringame, 0, sizeof(playeringame));
	memset(displayplayer, 0, sizeof(displayplayer));
	memset(consoleplayer, 0, sizeof(consoleplayer));
	memset(g_PlayerInSplit, 0, sizeof(g_PlayerInSplit));
	g_SplitScreen = -1;
	
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
			// Remove arbritation list
			if (l_Clients[i]->NumArbs)
			{
				// Run through arbritation list and zap players
				for (j = 0; j < l_Clients[i]->NumArbs; j++)
					D_NCZapNetPlayer(l_Clients[i]->Arbs[j]);
				
				// Free it
				Z_Free(l_Clients[i]->Arbs);
				l_Clients[i]->NumArbs = 0;
			}
			
			// Tell remote client that the server no longer exists.
			if (!l_Clients[i]->IsLocal)
			{
				// Fudge off!
				D_NCFudgeOffClient(l_Clients[i], 'X', "Server disconnected.");
				
				// Free streams, if any
				if (l_Clients[i]->PerfectStream)
					D_BSCloseStream(l_Clients[i]->PerfectStream);
				if (l_Clients[i]->CoreStream)
					D_BSCloseStream(l_Clients[i]->CoreStream);
					
				// Close socket, if any
				if (l_Clients[i]->NetSock)
					I_NetCloseSocket(l_Clients[i]->NetSock);
				
				// Free it
				Z_Free(l_Clients[i]);
				l_Clients[i] = NULL;
			}
		}
}

/* D_NCServize() -- Turn into server */
void D_NCServize(void)
{
	int32_t i;
	
	/* First Disconnect */
	// This does most of the work for us
	D_NCDisconnect();
	
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
	
	/* Change gamestate to waiting for player */
	gamestate = wipegamestate = GS_WAITINGPLAYERS;
	S_ChangeMusicName("D_WAITIN", 1);			// Waiting for game to start
}

/* D_NCClientize() -- Turn into client and connect to server */
void D_NCClientize(I_HostAddress_t* const a_Host, const char* const a_Pass, const char* const a_JoinPass)
{
	size_t i;
	D_NetClient_t* Server;
	D_NetClient_t* NetClient;
	I_NetSocket_t* Socket;
	D_BS_t* Stream;
	
	/* Check */
	if (!a_Host)
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
	D_NCDisconnect();
	
	/* Try creating socket to server */
	for (i = 0; i < 10; i++)
	{
		Socket = I_NetOpenSocket(false, a_Host, __REMOOD_BASEPORT + i);
		
		// Was created?
		if (Socket)
			break;
	}
	
	// Failed to create?
	if (!Socket)
	{
		CONL_OutputU(DSTR_NET_CONNECTNOSOCKET, "\n");
		return;
	}
	
	/* Revoke self serverness */
	for (i = 0; i < l_NumClients; i++)
		if (l_Clients[i])
		{
			l_Clients[i]->IsServer = false;
			l_Clients[i]->ReadyToPlay = false;
			l_Clients[i]->SaveGameSent = false;
		}
	
	/* Create NetClient for server */
	NetClient = D_NCAllocClient();
	
	// Fill stuff in it
	NetClient->NetSock = Socket;
	memmove(&NetClient->Address, a_Host, sizeof(*a_Host));
	
	// Create stream from it
	NetClient->CoreStream = D_BSCreateNetStream(NetClient->NetSock);
	
	// Create encapsulated perfect stream
	NetClient->PerfectStream = D_BSCreatePerfectStream(NetClient->CoreStream);
	
	// Create streams for server connection
	NetClient->Streams[DNCSP_READ] = NetClient->CoreStream;
	NetClient->Streams[DNCSP_WRITE] = NetClient->CoreStream;
	NetClient->Streams[DNCSP_PERFECTREAD] = NetClient->PerfectStream;
	NetClient->Streams[DNCSP_PERFECTWRITE] = NetClient->PerfectStream;
	
	// Set as server
	NetClient->IsServer = true;
	
	/* Prepare for connection to server */
	gamestate = GS_WAITFORJOINWINDOW;
	
	/* Send connection command to server */
	// Send perfect write packets
	Stream = NetClient->Streams[DNCSP_PERFECTWRITE];
	
	// Write out the data
	D_BSBaseBlock(Stream, "CONN");
	
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

/* D_NCReqAddPlayer() -- Requests that the server add local player */
void D_NCReqAddPlayer(struct D_ProfileEx_s* a_Profile, const bool_t a_Bot)
{
	D_NetClient_t* Server;
	D_BS_t* Stream;
	
	/* Check */
	if (!a_Profile)
		return;
	
	/* Only servers are allowed to add bots */
	if (!D_SyncNetIsArbiter() && a_Bot)
		return;
	
	/* Find server to send request to */
	Server = D_NCFindClientIsServer();
	
	// Not found?
	if (!Server)
		return;
	
	/* Tell server to add player */
	// Use server stream
	Stream = Server->Streams[DNCSP_PERFECTWRITE];
	
	// Put Data
	D_BSBaseBlock(Stream, "LPRJ");
	D_BSws(Stream, a_Profile->UUID);
	D_BSws(Stream, a_Profile->AccountName);
	D_BSws(Stream, a_Profile->DisplayName);
	D_BSwu8(Stream, a_Profile->Color);
	D_BSwu8(Stream, a_Bot);
	D_BSRecordNetBlock(Stream, &Server->Address);
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
	uint32_t* HashPtr;							// Hash Ptr
	char Header[5];								// Message Header
	D_NCMessageHandlerFunc_t Func;				// Handler Func
	D_NCMessageFlag_t Flags;					// Flags
} D_NCMessageType_t;

/* D_NCMessageData_t -- Message Data */
typedef struct D_NCMessageData_s
{
	const D_NCMessageType_t* Type;				// Type of message
	D_NetClient_t* NetClient;					// Client it is from
	D_NetClient_t* RemoteClient;				// Attached Remote Client
	D_BS_t* InStream;					// Stream to read from
	D_BS_t* OutStream;				// Stream to write to
	I_HostAddress_t* FromAddr;					// Address message is from
	uint32_t FlagsMask;							// Mask for flags
} D_NCMessageData_t;

/*** PACKET HANDLER FUNCTIONS ***/

/* D_NCMH_JOIN() -- Player wants to join */
bool_t D_NCMH_JOIN(struct D_NCMessageData_s* const a_Data)
{
}

/* D_NCMH_TICS() -- Recieved player tics */
bool_t D_NCMH_TICS(struct D_NCMessageData_s* const a_Data)
{
	/* Ignore any tic commands from the local server */
	if (a_Data->NetClient->IsLocal)
		return false;
}

// c_NCMessageCodes -- Local messages
static const D_NCMessageType_t c_NCMessageCodes[] =
{
	{1, {0}, "JOIN", D_NCMH_JOIN, DNCMF_PERFECT | DNCMF_SERVER | DNCMF_HOST | DNCMF_REMOTECL},
	{1, {0}, "TICS", D_NCMH_TICS, DNCMF_PERFECT | DNCMF_SERVER | DNCMF_REMOTECL},
	
	//{1, {0}, "LPRJ", D_NCMH_LocalPlayerRJ, DNCMF_PERFECT | DNCMF_SERVER | DNCMF_HOST | DNCMF_REMOTECL},
	//{1, {0}, "PJOK", D_NCMH_PlayerJoinOK, DNCMF_PERFECT | DNCMF_SERVER | DNCMF_REMOTECL | DNCMF_DEMO},
	
	// EOL
	{0, NULL, ""},
};

/*** FUNCTIONS ***/

/* D_NetReadGlobalTicCmd() -- Reads global tic command */
void D_NetReadGlobalTicCmd(ticcmd_t* const a_TicCmd)
{
}

/* D_NetWriteGlobalTicCmd() -- Writes global tic command */
void D_NetWriteGlobalTicCmd(ticcmd_t* const a_TicCmd)
{
	int nc;
	D_NetClient_t* ServerNC;
	D_NetClient_t* NetClient;
	D_BS_t* Stream;
	
	/* Get server client */
	ServerNC =  D_NCFindClientIsServer();
	
	// Non-Local?
	if (!ServerNC->IsLocal)
		return;
	
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
		
		// Write message to them (perfect output)
		Stream = NetClient->Streams[DNCSP_PERFECTWRITE];
		
		// Base
		D_BSBaseBlock(Stream, "TICS");
		
		// Current Game Tic
		D_BSwu64(Stream, gametic);
		
		// Zero Marks Global
		D_BSwu8(Stream, 0);
		
		// Send away
		D_BSRecordNetBlock(Stream, &NetClient->Address);
	}
}

/* D_NetReadTicCmd() -- Read tic commands from network */
void D_NetReadTicCmd(ticcmd_t* const a_TicCmd, const int a_Player)
{
}

/* D_NetWriteTicCmd() -- Write tic commands to network */
void D_NetWriteTicCmd(ticcmd_t* const a_TicCmd, const int a_Player)
{
	int nc;
	D_NetClient_t* ServerNC;
	D_NetClient_t* NetClient;
	D_BS_t* Stream;
	
	/* Get server client */
	ServerNC =  D_NCFindClientIsServer();
	
	// Non-Local?
	if (!ServerNC->IsLocal)
		return;
	
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
		
		// Write message to them (perfect output)
		Stream = NetClient->Streams[DNCSP_PERFECTWRITE];
		
		// Base
		D_BSBaseBlock(Stream, "TICS");
		
		// Current Game Tic
		D_BSwu64(Stream, gametic);
		
		// Positive Marks Player
		D_BSwu8(Stream, a_Player + 1);
		
		// Send away
		D_BSRecordNetBlock(Stream, &NetClient->Address);
	}
}

/* D_NCUpdate() -- Update all networking stuff */
void D_NCUpdate(void)
{
	int32_t nc, IsPerf, tN;
	char Header[5];
	I_HostAddress_t FromAddress;
	D_NetClient_t* NetClient, *RemoteClient;
	D_BS_t* PIn, *POut, *BOut;
	bool_t IsServ, IsClient, IsHost;
	D_NCMessageData_t Data;
	
	/* Go through each client and read/write commands */
	for (nc = 0; nc < l_NumClients; nc++)
	{
		// Get current
		NetClient = l_Clients[nc];
		
		// Failed?
		if (!NetClient)
			continue;
		
		// Set streams to read/write from
		PIn = NetClient->Streams[DNCSP_PERFECTREAD];
		POut = NetClient->Streams[DNCSP_PERFECTWRITE];
		BOut = NetClient->Streams[DNCSP_WRITE];
		
		// Constantly read from the perfect input stream (if it is set)
		memset(Header, 0, sizeof(Header));
		while (PIn && D_BSPlayNetBlock(PIn, Header, &FromAddress))
		{
			// Debug
			if (devparm)
				CONL_PrintF("Read \"%s\"\n", Header);
			
			// Find the remote client that initiated this message
			// If they aren't in the client chain then this returns NULL.
			// If it does return NULL then that means they were never connected
			// in the first place.
			RemoteClient = D_NCFindClientByHost(&FromAddress);
			
			// Determine where this packet came from for flag checking
				// Is Perfect Packet
			IsPerf = 0;
			D_BSStreamIOCtl(PIn, DRBSIOCTL_ISPERFECT, &IsPerf);
				// From Server
			IsServ = NetClient->IsServer;
				// From Client
			IsClient = !IsServ;
				// We are the host
			IsHost = (NetClient->IsServer && NetClient->IsLocal);
			
			// Go through head table
			for (tN = 0; c_NCMessageCodes[tN].Valid; tN++)
			{
				// Perfect but not set?
				if (IsPerf && !(c_NCMessageCodes[tN].Flags & DNCMF_PERFECT))
					continue;
				
				// Normal but not set?
				if (!IsPerf && !(c_NCMessageCodes[tN].Flags & DNCMF_NORMAL))
					continue;
				
				// From client but not accepted from client
				if (IsClient && !(c_NCMessageCodes[tN].Flags & DNCMF_CLIENT))
					continue;
				
				// From server but not accepted from server
				if (IsServ && !(c_NCMessageCodes[tN].Flags & DNCMF_SERVER))
					continue;
				
				// From somewhere but we are not the host of the game
				if (IsHost && !(c_NCMessageCodes[tN].Flags & DNCMF_HOST))
					continue;
				
				// Requires a remote client exist (a connected player)
				if ((c_NCMessageCodes[tN].Flags& DNCMF_REMOTECL) && !RemoteClient)
					continue;
				
				// Clear data
				memset(&Data, 0, sizeof(Data));
				
				// Fill Data
				Data.Type = &c_NCMessageCodes[tN];
				Data.NetClient = NetClient;
				Data.RemoteClient = RemoteClient;
				Data.InStream = PIn;
				Data.OutStream = (IsPerf ? POut : BOut);
				Data.FromAddr = &FromAddress;
				
				// Call handler
				if (c_NCMessageCodes[tN].Func(&Data))
					break;
			}
			
			// Clear header for next read run
			memset(Header, 0, sizeof(Header));
		}
		
		// Flush write streams so our commands are sent
			// Commands could be in the local loopback which are unsent until
			// flushed.
		D_BSFlushStream(POut);
		D_BSFlushStream(BOut);
	}
}

/* D_NCMH_LocalPlayerRJ() -- Player wants to join game */
bool_t D_NCMH_LocalPlayerRJ(struct D_NCMessageData_s* const a_Data)
{
	char UUID[MAXPLAYERNAME * 2];
	char AccountName[MAXPLAYERNAME];
	char DisplayName[MAXPLAYERNAME];
	uint8_t Color, Bot, Bits;
	int32_t i, PlCount, FreeSlot;
	uint32_t ProcessID;
	D_BS_t* Stream;
	
	/* Check */
	if (!a_Data)
		return false;
	
	/* Remote player is neither ready nor got a save game sent to them */
	if (!a_Data->RemoteClient->ReadyToPlay || !a_Data->RemoteClient->SaveGameSent)
		return false;
	
	/* Get their input stream */
	Stream = a_Data->InStream;
	
	/* Read Message Data */
	D_BSrs(Stream, UUID, MAXPLAYERNAME * 2);
	D_BSrs(Stream, AccountName, MAXPLAYERNAME);
	D_BSrs(Stream, DisplayName, MAXPLAYERNAME);
	Color = D_BSru8(Stream);
	Bot = D_BSru8(Stream);
	
	/* Disallow non-server from adding bots */
	if (Bot && !a_Data->RemoteClient->IsServer)
		return true;
	
	/* If not adding a bot, don't exceed non-bot arbs */
	if (!Bot)
	{
		PlCount = 0;
		for (i = 0; i < a_Data->RemoteClient->NumArbs; i++)
			if (a_Data->RemoteClient->Arbs[i]->Type == DNPT_LOCAL ||
				a_Data->RemoteClient->Arbs[i]->Type == DNPT_NETWORK)
				PlCount++;
	
		// Exceeds max permitted splitscreen count?
		if (PlCount >= MAXSPLITSCREEN)
			return true;
	}
	
	/* Find free player slot */
	FreeSlot = -1;
	for (i = 0; i < MAXPLAYERS; i++)
		if (!playeringame[i])
		{
			FreeSlot = i;
			break;
		}
	
	// No slot found (too many players inside)
	if (FreeSlot == -1)
		return true;
	
	/* Appears that the join is OK, so join em in! */
	// Create process ID for them (this is so the server doesn't add it's own
	// local player twice for each listener, since multiple NetClients can point
	// to the same place.)
	ProcessID = Z_Hash(UUID) ^ Z_Hash(AccountName) ^ Z_Hash(DisplayName);
	
	// Modify bits some more (to hopefully prevent process ID attacks)
		// Also someone could add the same profile over again (multiple guests)
	for (i = 0; i < 31; i += 3)
	{
		// Generate Random Junk
		Bits = (((int)(M_Random())) + ((int)I_GetTime() * (int)I_GetTime()));
		
		// XOR it in
		ProcessID ^= ((uint32_t)Bits) << i;
	}
	
	/* Send command to all clients */
	for (i = 0; i < l_NumClients; i++)
	{
		// Missing?
		if (!l_Clients[i])
			continue;
		
		// Write message to them (perfect output)
		Stream = l_Clients[i]->Streams[DNCSP_PERFECTWRITE];
		
		// Base
		D_BSBaseBlock(Stream, "PJOK");
		
		// Data
		D_BSwu32(Stream, ProcessID);
		D_BSwu8(Stream, FreeSlot);
		D_BSwu8(Stream, Bot);
		D_BSws(Stream, UUID);
		D_BSws(Stream, AccountName);
		D_BSws(Stream, DisplayName);
		D_BSwu8(Stream, Color);
		
		// Send away
		D_BSRecordNetBlock(Stream, &l_Clients[i]->Address);
	}
	
	/* Don't handle again */
	return true;
}

/* D_NCMH_PlayerJoinOK() -- Player joins the game */
bool_t D_NCMH_PlayerJoinOK(struct D_NCMessageData_s* const a_Data)
{
	char UUID[MAXPLAYERNAME * 2];
	char AccountName[MAXPLAYERNAME];
	char DisplayName[MAXPLAYERNAME];
	uint8_t Color, Bot;
	int32_t FreeSlot;
	uint32_t ProcessID;
	D_NetPlayer_t* NetPlayer;
	
	/* Check */
	if (!a_Data)
		return false;
		
	/* Read packet data */
	ProcessID = D_BSru32(a_Data->InStream);
	FreeSlot = D_BSru8(a_Data->InStream);
	Bot = D_BSru8(a_Data->InStream);
	D_BSrs(a_Data->InStream, UUID, MAXPLAYERNAME * 2);
	D_BSrs(a_Data->InStream, AccountName, MAXPLAYERNAME);
	D_BSrs(a_Data->InStream, DisplayName, MAXPLAYERNAME);
	Color = D_BSru8(a_Data->InStream);
	
	// Cap off strings (to prevent any string based attacks)
	AccountName[MAXPLAYERNAME - 1] = 0;
	DisplayName[MAXPLAYERNAME - 1] = 0;
	
	/* See if this process ID was already selected */
	NetPlayer = D_NCSFindNetPlayerByProcess(ProcessID);
	
	// It is there already?
	if (NetPlayer)
		return true;
	
	/* Enqueue an add player command into the tic stream */
	// This is saved into demos by any encoder that can handle them. So you can
	// say record Legacy demos with this method. Demos also read it directly
	// and do not handle packet types at all (abstraction).
	
	/* Don't handle again */
	return true;
}

/*****************************************************************************/

/*** NCSR FUNCTIONS ***/

/* D_NCSR_RequestMap() -- Requests that the map changes */
void D_NCSR_RequestMap(const char* const a_Map)
{
	D_NetClient_t* Server;
	P_LevelInfoEx_t* Info;
	size_t i;
	bool_t LocalHit;
	
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
	
	/* Inform that everyone should change the map */
	LocalHit = false;	// Prevent sending via double-client
	for (i = 0; i < l_NumClients; i++)
	{
		// Skip empty spots
		if (!l_Clients[i])
			continue;
		
		// Send packet?
		if ((l_Clients[i]->IsLocal && !LocalHit) || (!l_Clients[i]->IsLocal))
		{
			D_BSBaseBlock(l_Clients[i]->Streams[DNCSP_PERFECTWRITE], "MAPC");
			D_BSws(l_Clients[i]->Streams[DNCSP_PERFECTWRITE], a_Map);
			D_BSRecordNetBlock(l_Clients[i]->Streams[DNCSP_PERFECTWRITE], &l_Clients[i]->Address);
		}
		
		// Set local
		if (l_Clients[i]->IsLocal)
			LocalHit = true;
	}
}

/* D_NCSR_RequestNewPlayer() -- Requests that a local profile join remote server */
void D_NCSR_RequestNewPlayer(struct D_ProfileEx_s* a_Profile)
{
	D_NetClient_t* Server;
	D_BS_t* Stream;
	
	/* Check */
	if (!a_Profile)
		return;
	
	/* Find Server */
	Server = D_NCFindClientIsServer();
	
	// Not found?
	if (!Server)
		return;
	
	/* Tell server to add player */
	// Use server stream
	Stream = Server->Streams[DNCSP_PERFECTWRITE];
	
	// Put Data
	D_BSBaseBlock(Stream, "LPRJ");
	D_BSws(Stream, a_Profile->UUID);
	D_BSws(Stream, a_Profile->AccountName);
	D_BSws(Stream, a_Profile->DisplayName);
	D_BSwu8(Stream, a_Profile->Color);
	D_BSRecordNetBlock(Stream, &Server->Address);
	
	// Debug
	if (devparm)
		CONL_PrintF("Request Sent!\n");
}

/* D_NCSR_RequestWAD() -- Request WAD from server */
void D_NCSR_RequestWAD(const char* const a_WADSum)
{
	/* Check */
	if (!a_WADSum)
		return;
}

/* D_NCSR_RequestServerWADs() -- Request the server send WAD Info */
void D_NCSR_RequestServerWADs(void)
{
}

/* D_NCSR_SendServerReady() -- Tell server we are ready */
void D_NCSR_SendServerReady(void)
{
	D_NetClient_t* Server;
	D_BS_t* Stream;
	
	/* Find Server */
	Server = D_NCFindClientIsServer();
	
	// Not found?
	if (!Server)
		return;
	
	/* Tell server to add player */
	// Use server stream
	Stream = Server->Streams[DNCSP_PERFECTWRITE];
	
	// Put Data
	D_BSBaseBlock(Stream, "REDY");
	D_BSRecordNetBlock(Stream, &Server->Address);
}

/* D_NCSR_SendLoadingStatus() -- Tell the server we are loading something */
void D_NCSR_SendLoadingStatus(const int32_t a_MajIs, const int32_t a_MajOf, const int32_t a_MinIs, const int32_t a_MinOf)
{
}

/*** NCHE FUNCTIONS ***/

/* D_NCHE_ServerCreatePlayer() -- Server creates player */
void D_NCHE_ServerCreatePlayer(const size_t a_pNum, struct D_NetPlayer_s* const a_NetPlayer, struct D_ProfileEx_s* const a_Profile, D_NetClient_t* const a_NetClient)
{
	size_t i, j;
	D_NetClient_t* Server;
	player_t* DoomPlayer;
	
	/* Create Player Locally */
	DoomPlayer = G_AddPlayer(a_pNum);
	DoomPlayer->NetPlayer = a_NetPlayer;
	DoomPlayer->ProfileEx = a_Profile;
	a_Profile->NetPlayer = a_NetPlayer;
	a_NetPlayer->Player = DoomPlayer;
	a_NetPlayer->Profile = a_Profile;
	G_InitPlayer(DoomPlayer);
	
	/* See if we are the server */
	Server = D_NCFindClientIsServer();
	
	// Not found?
	if (!Server)
		return;
	
	// Check split screen
		// If we are the server
		// If this is the local client
	if (a_NetClient == Server || a_NetClient->IsLocal)
	{
		for (j = 0; j < MAXSPLITSCREEN; j++)
			if (!g_PlayerInSplit[j])
			{
				g_PlayerInSplit[j] = true;
				consoleplayer[j] = displayplayer[j] = a_pNum;
				
				g_SplitScreen = j;
				R_ExecuteSetViewSize();
				break;
			}
	}
	
	// Inform everyone else if we are the server
	if (Server->IsLocal)
	{
		for (i = 0; i < l_NumClients; i++)
			if (l_Clients[i])
				if (!l_Clients[i]->IsLocal)
				{
				
				}
	}
}

/*** NSZZ FUNCTIONS ***/

/* D_NSZZ_SendINFO() -- Send server info */
void D_NSZZ_SendINFO(struct D_BS_s* a_Stream, const uint32_t a_LocalTime)
{
	const WL_WADFile_t* Rover;
	uint8_t u8;
	
	/* Write Version */
	D_BSwu8(a_Stream, VERSION);
	D_BSwu8(a_Stream, REMOOD_MAJORVERSION);
	D_BSwu8(a_Stream, REMOOD_MINORVERSION);
	D_BSwu8(a_Stream, REMOOD_RELEASEVERSION);
	D_BSws(a_Stream, REMOOD_VERSIONCODESTRING);
	
	/* Write Time Info */
	D_BSwu32(a_Stream, a_LocalTime);
	D_BSwu32(a_Stream, time(NULL));
	D_BSwu32(a_Stream, D_SyncNetMapTime());
	D_BSwu32(a_Stream, 0);
	
	/* Write Server Name */
	D_BSws(a_Stream, l_SVName.Value->String);
	D_BSws(a_Stream, l_SVEMail.Value->String);
	D_BSws(a_Stream, l_SVURL.Value->String);
	D_BSws(a_Stream, l_SVWADURL.Value->String);
	D_BSws(a_Stream, l_SVIRC.Value->String);
	
	/* Passwords */
	// Connect Password
	u8 = '-';
	if (strlen(l_SVConnectPassword.Value->String) > 0)
		u8 = 'P';
	D_BSwu8(a_Stream, u8);
	
	// Join Password
	u8 = '-';
	if (strlen(l_SVJoinPassword.Value->String) > 0)
		u8 = 'J';
	D_BSwu8(a_Stream, u8);
	
	/* Write WAD Info */
	for (Rover = WL_IterateVWAD(NULL, true); Rover; Rover = WL_IterateVWAD(Rover, true))
	{
		// Write start
		D_BSwu8(a_Stream, 'W');
		
		// TODO: Optional WAD
		D_BSwu8(a_Stream, 'R');
		
		// Write Names for WAD (DOS and Base)
		D_BSws(a_Stream, WL_GetWADName(Rover, false));
		D_BSws(a_Stream, WL_GetWADName(Rover, true));
		
		// Write File Sums
		D_BSws(a_Stream, Rover->SimpleSumChars);
		D_BSws(a_Stream, Rover->CheckSumChars);
	}
	
	// End List
	D_BSwu8(a_Stream, 'X');
	
	/* Level Name */
	switch (gamestate)
	{
			// In Game
		case GS_LEVEL:
			D_BSws(a_Stream, (g_CurrentLevelInfo ? g_CurrentLevelInfo->LumpName : "<UNKNOWN"));
			break;
			
			// Non-Games
		case GS_INTERMISSION: D_BSws(a_Stream, "<INTERMISSION>"); break;
		case GS_FINALE: D_BSws(a_Stream, "<STORY>"); break;
		case GS_DEMOSCREEN: D_BSws(a_Stream, "<TITLESCREEN>"); break;
			
			// Unknown
		default:
			D_BSws(a_Stream, "<UNKNOWN>");
			break;
	}
}

/* D_NSZZ_SendINFX() -- Extended Info */
// This sends all variables
bool_t D_NSZZ_SendINFX(struct D_BS_s* a_Stream, size_t* const a_It)
{
	size_t EndIt;
	P_XGSVariable_t* XVar;
	
	/* Get End */
	EndIt = *a_It + 5;
	
	/* Loop */
	for (; *a_It < EndIt && *a_It < PEXGSNUMBITIDS; (*a_It)++)
	{
		// Write Marker
		D_BSwu8(a_Stream, 'V');
		
		// Get Var
		XVar = P_XGSVarForBit(*a_It);
		
		if (!XVar)
			continue;
		
		// Write Name and value
		D_BSws(a_Stream, XVar->Name);
		D_BSwu32(a_Stream, (XVar->WasSet ? XVar->ActualVal : XVar->DefaultVal));
	}
	
	/* End */
	D_BSwu8(a_Stream, 'E');
	
	/* More variables available? */
	if (*a_It < PEXGSNUMBITIDS)
		return true;
	return false;
}

/* D_NSZZ_SendMOTD() -- Send Message Of The Day */
void D_NSZZ_SendMOTD(struct D_BS_s* a_Stream)
{
	D_BSws(a_Stream, l_SVMOTD.Value->String);
}

/* D_NSZZ_SendFullWADS() -- Send WADs of Server */
void D_NSZZ_SendFullWADS(struct D_BS_s* a_Stream, I_HostAddress_t* const a_Host)
{
	const WL_WADFile_t* Rover;
	int i;
	
	/* Block */
	D_BSBaseBlock(a_Stream, "WADS");
	
	/* Write WAD Info */
	for (i = 0, Rover = WL_IterateVWAD(NULL, true); Rover; Rover = WL_IterateVWAD(Rover, true), i++)
	{
		// Write start
		D_BSwu8(a_Stream, 'W');
		
		// ReMooD.WAD is name matched only
		if (i == 1)
			D_BSwu8(a_Stream, 'N');
		else
			// TODO: Optional WADs
			D_BSwu8(a_Stream, 'R');
		
		// Write Names for WAD (DOS and Base)
		D_BSws(a_Stream, WL_GetWADName(Rover, false));
		D_BSws(a_Stream, WL_GetWADName(Rover, true));
		
		// Write File Sums
		D_BSws(a_Stream, Rover->SimpleSumChars);
		D_BSws(a_Stream, Rover->CheckSumChars);
	}
	
	// End List
	D_BSwu8(a_Stream, 'X');
	
	/* Record it */
	D_BSRecordNetBlock(a_Stream, a_Host);
}

/* D_NCHE_SendSaveGame() -- Send savegame to client */
void D_NCHE_SendSaveGame(D_NetClient_t* const a_Client)
{
	struct D_BS_s* Stream;
	
	/* Check */
	if (!a_Client)
		return;
	
	/* Set Info */
	Stream = a_Client->Streams[DNCSP_PERFECTWRITE];
	
	/* Send Start */
	D_BSBaseBlock(Stream, "SAVE");
	D_BSRecordNetBlock(Stream, &a_Client->Address);
	
	/* Send Save */
	P_SaveGameToBS(Stream, &a_Client->Address);
	
	/* Send End */
	D_BSBaseBlock(Stream, "SAVX");
	D_BSRecordNetBlock(Stream, &a_Client->Address);
}

/*** NCQC FUNCTIONS ***/

/* D_NCQC_MapChange() -- Change Map */
void D_NCQC_MapChange(void* const a_Data)
{
	char* MapName;
	P_LevelInfoEx_t* Info;
	
	/* Check */
	if (!a_Data)
		return;
	
	/* Get Map */
	MapName = (char*)a_Data;
	
	/* Switch to that level */
	// Try finding the level
	Info = P_FindLevelByNameEx(MapName, NULL);
	
	// Load the level
	if (!Info)
		CONL_PrintF("NET: Could not find level.\n");
	else
		if (!P_ExLoadLevel(Info, 0))
			CONL_PrintF("NET: Could not load level.\n");
	
	/* Free string (was Z_StrDup) */
	Z_Free(a_Data);
}


