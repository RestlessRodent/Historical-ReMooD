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

/*************
*** LOCALS ***
*************/

static tic_t l_MapTime = 0;		// Map local time

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
	fprintf(stderr, "%s\n", Text);
#undef BUFSIZE
}

/* D_SyncNetIsArbiter() -- Do we control the game? */
bool_t D_SyncNetIsArbiter(void)
{
	return true;
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

extern consvar_t cv_g_gamespeed;

/* D_SyncNetIsPaused() -- Returns true if the game is paused */
bool_t D_SyncNetIsPaused(void)
{
	if (paused || (!netgame && M_ExUIActive() && !demoplayback))
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
	static tic_t LocalTime = 0;
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
	
	/* If we are the server, we dictate time */
	if (D_SyncNetIsArbiter())
	{
		// The map time is determined by the framerate
		ThisTime = I_GetTimeMS() / TICSPERMS;
		DiffTime = ThisTime - LocalTime;
		
		if (DiffTime > 0)
		{
			// Return the time difference
			LocalTime = ThisTime;
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

/* DS_ConnectMultiCom() -- Connection multi-command */
static CONL_ExitCode_t DS_ConnectMultiCom(const uint32_t a_ArgC, const char** const a_ArgV)
{
	D_NetClient_t* ServerNC;
	I_HostAddress_t Host;
	
	/* Clear Host */
	memset(&Host, 0, sizeof(Host));
	
	/* Connect */
	if (strcasecmp("connect", a_ArgV[0]) == 0)
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
		D_NCClientize(&Host, (a_ArgC >= 3 ? : a_ArgV[2]), (a_ArgC >= 4 ? : a_ArgV[3]));
		
		return CLE_SUCCESS;
	}
	
	/* Disconnect */
	if (strcasecmp("disconnect", a_ArgV[0]) == 0)
	{
		// Disconnect
		D_NCDisconnect();
		
		// Success!
		return CLE_SUCCESS;
	}
	
	/* Reconnect */
	if (strcasecmp("reconnect", a_ArgV[0]) == 0)
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
	Client->CoreStream = D_RBSCreateLoopBackStream();
	
	// Create perfection Wrapper
	Client->PerfectStream = D_RBSCreatePerfectStream(Client->CoreStream);
	
	// Set read/writes for all streams
	Client->Streams[DNCSP_READ] = Client->CoreStream;
	Client->Streams[DNCSP_WRITE] = Client->CoreStream;
	Client->Streams[DNCSP_PERFECTREAD] = Client->PerfectStream;
	Client->Streams[DNCSP_PERFECTWRITE] = Client->PerfectStream;
	
	// Set as local and server
	Client->IsLocal = true;
	Client->IsServer = true;
	
	/* Create Local Network Client */
	// Attempt creating a UDP Server
	Socket = NULL;
	for (i = 0; i < 20 && !Socket; i++)
		Socket = I_NetOpenSocket(true, NULL, __REMOOD_BASEPORT + i);
	
	// Initial input/output of stream
	if (Socket)
	{
		// Allocate
		Client = D_NCAllocClient();
		
		// Copy socket
		Client->NetSock = Socket;
		
		// Create stream from it
		Client->CoreStream = D_RBSCreateNetStream(Client->NetSock);
		
		// Create encapsulated perfect stream
		Client->PerfectStream = D_RBSCreatePerfectStream(Client->CoreStream);
	
		// Set read/writes for all streams
		Client->Streams[DNCSP_READ] = Client->CoreStream;
		Client->Streams[DNCSP_WRITE] = Client->CoreStream;
		Client->Streams[DNCSP_PERFECTREAD] = Client->PerfectStream;
		Client->Streams[DNCSP_PERFECTWRITE] = Client->PerfectStream;
		
		// Set as local and server
		Client->IsLocal = true;
		Client->IsServer = true;
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

/* D_NCDisconnect() -- Disconnect from existing server */
void D_NCDisconnect(void)
{
	size_t i;
	
	/* Clear all player information */
	// Just wipe ALL of it!
	memset(players, 0, sizeof(players));
	memset(playeringame, 0, sizeof(playeringame));
	memset(displayplayer, 0, sizeof(displayplayer));
	memset(consoleplayer, 0, sizeof(consoleplayer));
	memset(g_PlayerInSplit, 0, sizeof(g_PlayerInSplit));
	g_SplitScreen = -1;
	
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
	// Also set all local stuff as servers
	for (i = 0; i < l_NumClients; i++)
		if (l_Clients[i])
		{
			// Not Local
			if (!l_Clients[i]->IsLocal)
			{
				// Free streams, if any
				if (l_Clients[i]->CoreStream)
					D_RBSCloseStream(l_Clients[i]->CoreStream);
				if (l_Clients[i]->PerfectStream)
					D_RBSCloseStream(l_Clients[i]->PerfectStream);
					
				// Close socket, if any
				if (l_Clients[i]->NetSock)
					I_NetCloseSocket(l_Clients[i]->NetSock);
				
				// Free it
				Z_Free(l_Clients[i]);
				l_Clients[i] = NULL;
			}
			
			// Is Local
			else
				l_Clients[i]->IsServer = true;
		}
}

/* D_NCServize() -- Turn into server */
void D_NCServize(void)
{
	/* First Disconnect */
	// This does most of the work for us
	D_NCDisconnect();
}

/* D_NCClientize() -- Turn into client and connect to server */
void D_NCClientize(I_HostAddress_t* const a_Host, const char* const a_Pass, const char* const a_JoinPass)
{
	size_t i;
	D_NetClient_t* Server;
	D_NetClient_t* NetClient;
	I_NetSocket_t* Socket;
	D_RBlockStream_t* Stream;
	
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
			l_Clients[i]->IsServer = false;
	
	/* Create NetClient for server */
	NetClient = D_NCAllocClient();
	
	// Fill stuff in it
	NetClient->NetSock = Socket;
	
	// Create stream from it
	NetClient->CoreStream = D_RBSCreateNetStream(NetClient->NetSock);
	
	// Create encapsulated perfect stream
	NetClient->PerfectStream = D_RBSCreatePerfectStream(NetClient->CoreStream);
	
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
	D_RBSBaseBlock(Stream, "CONN");
	
	// Write version
	D_RBSWriteUInt8(Stream, VERSION);
	D_RBSWriteUInt8(Stream, REMOOD_MAJORVERSION);
	D_RBSWriteUInt8(Stream, REMOOD_MINORVERSION);
	D_RBSWriteUInt8(Stream, REMOOD_RELEASEVERSION);
	
	// Passwords
	D_RBSWriteString(Stream, (a_Pass ? a_Pass : ""));
	D_RBSWriteString(Stream, (a_JoinPass ? a_JoinPass : ""));
	
	// Send to server
	D_RBSRecordNetBlock(Stream, a_Host);
	
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

/* D_NCUpdate() -- Update all networking stuff */
void D_NCUpdate(void)
{
#define BUFSIZE 512
	char Buf[BUFSIZE];
	char Header[5];
	D_RBlockStream_t* Stream, *OutStream, *GenOut;
	D_NetClient_t* NetClient, *OtherClient;
	size_t nc, snum, i, p, j;
	I_HostAddress_t FromAddress;
	D_NetPlayer_t* NetPlayer;
	D_ProfileEx_t* Profile;
	player_t* DoomPlayer;
	
	bool_t SendPing, ReSend;
	uint32_t ThisTime, DiffTime;
	static uint32_t LastTime;
	
	bool_t DoContinue;
	uint32_t u32a, u32b, u32c, u32d;
	uint8_t u8a, u8b;
	
	/* Send Ping Request? */
	ThisTime = I_GetTimeMS();
	
	// Send pings?
	SendPing = false;
	if (ThisTime > LastTime + 1000)
	{
		DiffTime = ThisTime - LastTime;
		LastTime = ThisTime;
		SendPing = true;
		
		// Set global stat count to current local stats
		for (i = 0; i < 4; i++)
			g_NetStat[i] = l_LocalStat[i];
	}
	
	/* Clear local stats */
	// This is for traffic monitoring
	for (i = 0; i < 4; i++)
		l_LocalStat[i] = 0;
	
	/* Go through each client and read/write commands */
	for (nc = 0; nc < l_NumClients; nc++)
	{
		// Get current
		NetClient = l_Clients[nc];
		
		// Failed?
		if (!NetClient)
			continue;
		
		// Initialize some things
		memset(&FromAddress, 0, sizeof(FromAddress));
		
		// Use base stream initially
		Stream = NetClient->CoreStream;
		
		// Sending a ping?
			// If sending, send command and unstat stream
		if (SendPing)
		{
			// Unstat the stream
			D_RBSUnStatStream(Stream);
			
			// Build PING command
			D_RBSBaseBlock(Stream, "PING");
			D_RBSWriteUInt32(Stream, ThisTime);
			D_RBSWriteUInt32(Stream, DiffTime);
			D_RBSRecordBlock(Stream);	// Send to default destination
		}
			// Otherwise, Stat the stream and add to local counts
		else
		{
			// Stat it
			D_RBSStatStream(Stream, &u32a, &u32b, &u32c, &u32d);
			
			// Add to local
			l_LocalStat[0] += u32a;
			l_LocalStat[1] += u32b;
			l_LocalStat[2] += u32c;
			l_LocalStat[3] += u32d;
		}
		
		// Read from the "Perfect" Stream
			// The perfect stream knows whether a packet is perfect or not and
			// if a perfect packet is not yet ready it won't return any of them
			// Also, a read stream might not exist, a client could be using
			// another clients stream for reading, this would be the case for
			// network games over UDP. Why? Becuase all network players write
			// to the server (the local client) for commands.
		Stream = NetClient->Streams[DNCSP_PERFECTREAD];
		OutStream = NetClient->Streams[DNCSP_PERFECTWRITE];
		
		GenOut = NetClient->Streams[DNCSP_WRITE];
		
		// Constantly read command packets
		memset(Header, 0, sizeof(Header));
		if (Stream)
		{
			// Constantly Read
			while (D_RBSPlayNetBlock(Stream, Header, &FromAddress))
			{
				// Debug?
				if (devparm)
					D_SyncNetDebugMessage("%i Got \"%c%c%c%c\" (From %08x:%i)...",
						(int)nc, Header[0], Header[1], Header[2], Header[3],
							SwapUInt32(FromAddress.Host.v4.u),
							FromAddress.Port
						);
				
	////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////
	////////////////
	//////////////// NON-PERFECT PACKETS
	////////////////
	
	// Everything -- Ping request
	if (D_RBSCompareHeader("PING", Header))
	{
		// Send PONG back to the from address (using generic stream)
		u32a = D_RBSReadUInt32(Stream);		// Rem: ThisTime
		u32b = D_RBSReadUInt32(Stream);		// Rem: DiffTime
		u32c = ThisTime;					// Loc: ThisTime
		
		// Create response and send away
		D_RBSBaseBlock(GenOut, "PONG");
		D_RBSWriteUInt32(GenOut, u32a);
		D_RBSWriteUInt32(GenOut, u32b);
		D_RBSWriteUInt32(GenOut, u32c);
		D_RBSRecordNetBlock(GenOut, &FromAddress);
	}
	
	// Everything -- Pong reply
	else if (D_RBSCompareHeader("PONG", Header))
	{
	}
	
	// Master Server -- Request List
	else if (D_RBSCompareHeader("MSRQ", Header))
	{
		// Read Cookie (Basic Security)
		u32a = D_RBSReadUInt32(Stream);
		u32b = D_RBSReadUInt32(Stream);
		
		// Setup Base Info
		D_RBSBaseBlock(GenOut, "MSLS");
		D_RBSWriteUInt32(GenOut, u32a);
		D_RBSWriteUInt32(GenOut, u32b);
		
		// Send Server Info
		D_RBSWriteUInt8(GenOut, 'R');	// Auto-remote end
		D_NSZZ_SendINFO(GenOut, ThisTime);
		
		// Send away
		D_RBSRecordNetBlock(GenOut, &FromAddress);
	}
	
	// Master Server -- List
	else if (D_RBSCompareHeader("MSLS", Header))
	{
		// Read Cookie (Basic Security)
		u32a = D_RBSReadUInt32(Stream);
		u32b = D_RBSReadUInt32(Stream);
	}
	
	// Server -- Request Game Info
	else if (D_RBSCompareHeader("RINF", Header))
	{
		// Read Cookie (Basic Security)
		u32a = D_RBSReadUInt32(Stream);
		u32b = D_RBSReadUInt32(Stream);
		
		// Write INFO
		D_RBSBaseBlock(GenOut, "INFO");
		D_RBSWriteUInt32(GenOut, u32a);
		D_RBSWriteUInt32(GenOut, u32b);
		
		// Send Server Info
		D_NSZZ_SendINFO(GenOut, ThisTime);
		
		// Send away
		D_RBSRecordNetBlock(GenOut, &FromAddress);
		
		// Write INFX
		ReSend = false;
		i = 0;
		do
		{
			D_RBSBaseBlock(GenOut, "INFX");
			D_RBSWriteUInt32(GenOut, u32a);
			D_RBSWriteUInt32(GenOut, u32b);
		
			// Send Server Info
			ReSend = D_NSZZ_SendINFX(GenOut, &i);
		
			// Send away
			D_RBSRecordNetBlock(GenOut, &FromAddress);
		} while (ReSend);
		
		// Send MOTD
		D_RBSBaseBlock(GenOut, "MOTD");
		D_RBSWriteUInt32(GenOut, u32a);
		D_RBSWriteUInt32(GenOut, u32b);
		D_NSZZ_SendMOTD(GenOut);
		D_RBSRecordNetBlock(GenOut, &FromAddress);
		
		// Send INFT
		for (i = 0; i < 2; i++)
		{
			D_RBSBaseBlock(GenOut, "INFT");
			D_RBSWriteUInt32(GenOut, u32a);
			D_RBSWriteUInt32(GenOut, u32b);
			D_RBSRecordNetBlock(GenOut, &FromAddress);
		}
	}
	
	// Client -- Recieve Game Info
	else if (D_RBSCompareHeader("INFO", Header))
	{
	}
	
	// Client -- Recieve Game Info Extended
	else if (D_RBSCompareHeader("INFX", Header))
	{
	}
	
	////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////
	////////////////
	//////////////// PERFECT PACKETS
	////////////////
	else if (D_RBSMarkedStream(Stream))
	{
		// Debug?
		if (devparm)
			D_SyncNetDebugMessage("Perfect!");
		
		// MAPC -- Map Change
		if (D_RBSCompareHeader("MAPC", Header))
		{
			// Only accept if from a server
			if (NetClient->IsServer)
			{
				// Read map name
				memset(Buf, 0, sizeof(Buf));
				D_RBSReadString(Stream, Buf, BUFSIZE - 1);
		
				// Add to command queue
				D_NCAddQueueCommand(D_NCQC_MapChange, Z_StrDup(Buf, PU_NETWORK, NULL));
			}
		}
		
		// LPRJ -- Local Player, Request Join
		else if (D_RBSCompareHeader("LPRJ", Header))
		{
			// Only accept if is a server
			if (NetClient->IsServer && NetClient->IsLocal)
			{
				// Find the client that wants to do this
				OtherClient = D_NCFindClientByHost(&FromAddress);
				
				// Nothing found?
				if (!OtherClient)
					CONL_OutputU(DSTR_NET_BADCLIENT, "\n");
				
				// Read the UUID
				D_RBSReadString(Stream, Buf, BUFSIZE - 1);
				
				// Only if one was found is it parsed
					// If not, maybe someone else is screwing with the server?
				DoContinue = true;
				if (OtherClient)
				{
					// Client is arbing too many?
					if (DoContinue && OtherClient->NumArbs >= MAXSPLITSCREEN)
					{
						CONL_OutputU(DSTR_NET_EXCEEDEDSPLIT, "\n");
						DoContinue = false;
					}
					
					// Check for free player slots
					for (p = 0; p < MAXPLAYERS; p++)
						if (!playeringame[p])
							break;
					
					// No Free Slots
					if (DoContinue && p >= MAXPLAYERS)
					{
						CONL_OutputU(DSTR_NET_ATMAXPLAYERS, "\n");
						DoContinue = false;
					}
					
					// Done, add to arbitration and inform everyone
					if (DoContinue)
					{
						// Create netplayer combo for this person
						NetPlayer = D_NCSAllocNetPlayer();
						NetPlayer->NetClient = OtherClient;
						
						// Create a profile for this player
						if (NetClient == OtherClient)	// Same system
							Profile = D_FindProfileEx(Buf);	// Use existing one
						
						// Read Player Account Name
						D_RBSReadString(Stream, Buf, BUFSIZE - 1);
						
						// Failed to find it? Then create it
						if (!Profile)
							Profile = D_CreateProfileEx(Buf);
							
						// Read Player Display Name
						D_RBSReadString(Stream, Buf, BUFSIZE - 1);
						
						// Mark profile as remote and copy display name
						if (NetClient != OtherClient)
						{
							Profile->Type = DPEXT_NETWORK;	// Set remote
							strncpy(Profile->DisplayName, Buf, MAXPLAYERNAME - 1);
							
							// Read Color
							Profile->Color = D_RBSReadUInt8(Stream);
						}
						
						// Set at arbs point
						Z_ResizeArray((void**)&OtherClient->Arbs, sizeof(*OtherClient->Arbs),
								OtherClient->NumArbs, OtherClient->NumArbs + 1);
						OtherClient->Arbs[OtherClient->NumArbs++] = NetPlayer;
						
						// Create Player Locally
						DoomPlayer = G_AddPlayer(p);
						DoomPlayer->NetPlayer = NetPlayer;
						DoomPlayer->ProfileEx = Profile;
						Profile->NetPlayer = NetPlayer;
						NetPlayer->Player = DoomPlayer;
						NetPlayer->Profile = Profile;
						G_InitPlayer(DoomPlayer);
						
						// Check split screen
						if (NetClient == OtherClient)
							for (j = 0; j < MAXSPLITSCREEN; j++)
								if (!g_PlayerInSplit[j])
								{
									g_PlayerInSplit[j] = true;
									consoleplayer[j] = displayplayer[j] = p;
									
									g_SplitScreen = j;
									R_ExecuteSetViewSize();
									break;
								}
						
						// Inform everyone else (when they aren't local)
						for (i = 0; i < l_NumClients; i++)
							if (l_Clients[i])
								if (!l_Clients[i]->IsLocal)
								{
									
								}
					}
				}
			}
		}

	////////////////
	//////////////// DONE
	////////////////
	////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////
				}
				
				// Clear from address
				memset(&FromAddress, 0, sizeof(FromAddress));
				memset(Header, 0, sizeof(Header));
			}
			
			// Flush write streams
			D_RBSFlushStream(OutStream);
			D_RBSFlushStream(GenOut);
		}
	}
	
#if 0
#define BUFSIZE 512
	char Buf[BUFSIZE];
	char Header[5];
	size_t nc, i, j, p;
	D_NetController_t* CurCtrl, *OtherCtrl, *HostCtrl;
	D_RBlockStream_t* Stream, *OtherStream;
	
	D_NetPlayer_t* NetPlayer;
	D_ProfileEx_t* Profile;
	player_t* DoomPlayer;
	
	uint32_t u32, u32b, u32c, u32d;
	uint8_t u8;
	
	bool_t SendPing, AnythingWritten;
	uint32_t ThisTime, DiffTime;
	static uint32_t LastTime;
	
	/* Get Host */
	HostCtrl = l_LocalController;
	
	/* Init */
	memset(Header, 0, sizeof(Header));
	
	/* Get Current Time */
	ThisTime = I_GetTimeMS();
	
	// Send pings?
	SendPing = false;
	if (ThisTime > LastTime + 1000)
	{
		DiffTime = ThisTime - LastTime;
		LastTime = ThisTime;
		SendPing = true;
	}
	
	/* Go through every controller */
	for (nc = 0; nc < l_NumControllers; nc++)
	{
		// Get current
		CurCtrl = l_Controllers[nc];
		
		// Nothing here?
		if (!CurCtrl)
			continue;
		
		// Init some things
		Stream = CurCtrl->BlockStream;
		
		// Send ping command
		if (SendPing)
		{
			// Clear stream stats
			D_RBSUnStatStream(Stream);
			for (i = 0; i < 4; i++)
			{
				g_NetStat[i] = l_LocalStat[i];
				l_LocalStat[i] = 0;
			}
			
			// Create ping
			D_RBSBaseBlock(Stream, "PING");
			D_RBSWriteUInt32(Stream, ThisTime);
			D_RBSWriteUInt32(Stream, DiffTime);
			
			// Record it
			D_RBSRecordBlock(Stream);
		}
		
		// Collect some infos
		else
		{
			// Stats
			D_RBSStatStream(Stream, &u32, &u32b, &u32c, &u32d);
			
			// Add to local
			l_LocalStat[0] = u32;
			l_LocalStat[1] = u32b;
			l_LocalStat[2] = u32c;
			l_LocalStat[3] = u32d;
		}
		
		// Constantly read blocks (packets)
		while (D_RBSPlayBlock(Stream, Header))
		{
			// PING -- Ping Request
			if (strcasecmp("PING", Header) == 0)
			{
				// Send a PONG back to it
				D_RBSRenameHeader(Stream, "PONG");
				D_RBSWriteUInt32(Stream, ThisTime);
				D_RBSRecordBlock(Stream);
			}
			
			// PONG -- Ping Reply
			else if (strcasecmp("PONG", Header) == 0)
			{
				CurCtrl->Ping = ThisTime - D_RBSReadUInt32(Stream);
			}
			
			// VERR -- Version Request
			else if (strcasecmp("VERR", Header) == 0)
			{
				// Create version reply
				D_RBSBaseBlock(Stream, "VERI");
				
				// Put in info
				D_RBSWriteUInt8(Stream, VERSION);
				D_RBSWriteUInt8(Stream, REMOOD_MAJORVERSION);
				D_RBSWriteUInt8(Stream, REMOOD_MINORVERSION);
				D_RBSWriteUInt8(Stream, REMOOD_RELEASEVERSION);
				D_RBSWriteString(Stream, REMOOD_FULLVERSIONSTRING);
				D_RBSWriteString(Stream, REMOOD_URL);
				
				// Send it away
				D_RBSRecordBlock(Stream);
			}
			
			// VERI -- Version Information
			else if (strcasecmp("VERI", Header) == 0)
			{
				// Read version info
				CurCtrl->VerLeg = D_RBSReadUInt8(Stream);
				CurCtrl->VerMaj = D_RBSReadUInt8(Stream);
				CurCtrl->VerMin = D_RBSReadUInt8(Stream);
				CurCtrl->VerRel = D_RBSReadUInt8(Stream);
			}
			
			// MESG -- Generic Message
			else if (strcasecmp("MESG", Header) == 0)
			{
				// Get Message
				memset(Buf, 0, sizeof(Buf));
				D_RBSReadString(Stream, Buf, BUFSIZE - 1);
				
				// Print
				CONL_PrintF("%s\n", Buf);
			}
			
			// MAPC -- Map Change
			else if (strcasecmp("MAPC", Header) == 0)
			{
				// Only accept if from a server
				if (CurCtrl->IsServer)
				{
					// Read map name
					memset(Buf, 0, sizeof(Buf));
					D_RBSReadString(Stream, Buf, BUFSIZE - 1);
					
					// Add to command queue
					D_NCAddQueueCommand(D_NCQC_MapChange, Z_StrDup(Buf, PU_NETWORK, NULL));
				}
			}
			
			// LPRJ -- Local Player, Request Join (Split screen)
			else if (strcasecmp("LPRJ", Header) == 0)
			{
				// Only accept if we are the server
				if (HostCtrl->IsServer && HostCtrl->IsLocal)
				{
					// Get Player UUID/AccountName
					memset(Buf, 0, sizeof(Buf));
					D_RBSReadString(Stream, Buf, BUFSIZE - 1);
					
					// Find player
					NetPlayer = D_NCSFindNetPlayer(Buf);
					
					// Find free player spot
					for (p = 0; p < MAXPLAYERS; p++)
						if (!playeringame[p])
							break;
					
					// Only resume request joining when there is no player
						// And the current local player count fits local players
						// And there is a free player spot
					if (!NetPlayer && CurCtrl->NumArbs < MAXSPLITSCREEN && p < MAXPLAYERS)
					{
						// Create player
						NetPlayer = D_NCSAllocNetPlayer();
						
						// Copy UUID Over
						strncpy(NetPlayer->UUID, Buf, MAXPLAYERNAME * 2);
						
						// Add to arbitrating players
						Z_ResizeArray((void**)&CurCtrl->Arbs, sizeof(*CurCtrl->Arbs),
								CurCtrl->NumArbs, CurCtrl->NumArbs + 1);
						CurCtrl->Arbs[CurCtrl->NumArbs++] = NetPlayer;
						
						// Create Profile
						D_RBSReadString(Stream, NetPlayer->AccountName, MAXPLAYERNAME);
						
						// If server split, use local profile
						if (CurCtrl->IsLocal)
						{
							// Try finding it
							Profile = D_FindProfileEx(NetPlayer->AccountName);
							
							// If not found, make fresh then
							if (!Profile)
								Profile = D_CreateProfileEx(NetPlayer->AccountName);
						}
						
						// Otherwise use remote profile
						else
							Profile = D_CreateProfileEx(NetPlayer->AccountName);
						
						// Fill info
						if (CurCtrl->IsLocal)	// Server Split
							Profile->Flags |= DPEXT_LOCAL;
						else					// Client split
							Profile->Flags |= DPEXT_NETWORK;
						D_RBSReadString(Stream, Profile->DisplayName, MAXPLAYERNAME);
						Profile->Color = D_RBSReadUInt8(Stream);
						
						// Inform everyone that a player has joined
						for (j = 0; j < l_NumControllers; j++)
						{
							// Get Other
							OtherCtrl = l_Controllers[j];
		
							// Nothing here?
							if (!OtherCtrl)
								continue;
		
							// Init some things
							OtherStream = OtherCtrl->BlockStream;
							
							// Write player join OK
							D_RBSBaseBlock(OtherStream, "PJOK");
							D_RBSWriteString(OtherStream, NetPlayer->UUID);
							D_RBSWriteUInt8(OtherStream, p);
							if (OtherStream == CurCtrl)
							{
								D_RBSWriteUInt8(OtherStream, CurCtrl->NumArbs);
								D_RBSWriteString(OtherStream, Profile->UUID);
							}
							else
							{
								D_RBSWriteUInt8(OtherStream, 0);
								D_RBSWriteString(OtherStream, "");
							}
							D_RBSWriteString(OtherStream, NetPlayer->AccountName);
							D_RBSWriteString(OtherStream, Profile->AccountName);
							D_RBSWriteString(OtherStream, Profile->DisplayName);
							D_RBSWriteUInt8(OtherStream, Profile->Color);
							D_RBSRecordBlock(OtherStream);
						}
						
						// Create Player In Local Server
						DoomPlayer = G_AddPlayer(p);
						DoomPlayer->NetPlayer = NetPlayer;
						DoomPlayer->ProfileEx = Profile;
						Profile->NetPlayer = NetPlayer;
						NetPlayer->Player = DoomPlayer;
						NetPlayer->Profile = Profile;
						G_InitPlayer(DoomPlayer);
						
						// Check split screen
						if (CurCtrl->IsLocal)
							for (j = 0; j < MAXSPLITSCREEN; j++)
								if (!g_PlayerInSplit[j])
								{
									g_PlayerInSplit[j] = true;
									consoleplayer[j] = displayplayer[j] = p;
									
									g_SplitScreen = j;
									break;
								}
					}
				}
			}
			
			// PJOK -- Player Join OK
			else if (strcasecmp("PJOK", Header) == 0)
			{
				// Only accept if from a server and non-local
					// Non-local because if it is local then the player would
					// already be in the structures.
				if (CurCtrl->IsServer && !CurCtrl->IsLocal && CurCtrl != HostCtrl)
				{
					// Create network player
					NetPlayer = D_NCSAllocNetPlayer();
					
					// Add to arbitrating players
					Z_ResizeArray((void**)&CurCtrl->Arbs, sizeof(*CurCtrl->Arbs),
							CurCtrl->NumArbs, CurCtrl->NumArbs + 1);
					CurCtrl->Arbs[CurCtrl->NumArbs++] = NetPlayer;
					
					// Read NetPlayer UUID and the local player number
					D_RBSReadString(Stream, NetPlayer->UUID, MAXPLAYERNAME * 2);
					p = D_RBSReadUInt8(Stream);
					
					// Determine if the player is our own screen player
					u8 = D_RBSReadUInt8(Stream);
					D_RBSReadString(Stream, Buf, BUFSIZE);
					
					// See if it is worth looking for a profile
					if (u8)
						Profile = D_FindProfileEx(NetPlayer->UUID);
					
					// No profile found or remote profile
					if (!Profile)
					{
						// Create blank slate
						u8 = 0;
						Profile = D_CreateProfileEx(NetPlayer->UUID);
						
						// Fill with guessed info
						Profile->Type = DPEXT_NETWORK;
						D_RBSReadString(Stream, NetPlayer->AccountName, MAXPLAYERNAME);
						D_RBSReadString(Stream, Profile->AccountName, MAXPLAYERNAME);
						D_RBSReadString(Stream, Profile->DisplayName, MAXPLAYERNAME);
						Profile->Color = D_RBSReadUInt8(Stream);
					}
					else
						Profile->Type = DPEXT_LOCAL;
					
					// Create Player In Local Game
					DoomPlayer = G_AddPlayer(p);
					DoomPlayer->NetPlayer = NetPlayer;
					DoomPlayer->ProfileEx = Profile;
					Profile->NetPlayer = NetPlayer;
					NetPlayer->Player = DoomPlayer;
					NetPlayer->Profile = Profile;
					G_InitPlayer(DoomPlayer);
					
					// Add player to split screen
					if (u8)
					{
						// Find free screen
						for (j = 0; j < MAXSPLITSCREEN; j++)
							if (!g_PlayerInSplit[j])
							{
								g_PlayerInSplit[j] = true;
								consoleplayer[j] = displayplayer[j] = p;
								
								g_SplitScreen = j;
								break;
							}
					}
				}
			}
		}
		
		// Flush commands (Send them together, if possible)
		D_RBSFlushStream(Stream);
	}
#endif

#undef BUFSIZE
}

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
	if (!Server->IsServer)
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
			D_RBSBaseBlock(l_Clients[i]->Streams[DNCSP_PERFECTWRITE], "MAPC");
			D_RBSWriteString(l_Clients[i]->Streams[DNCSP_PERFECTWRITE], a_Map);
			D_RBSRecordNetBlock(l_Clients[i]->Streams[DNCSP_PERFECTWRITE], &l_Clients[i]->Address);
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
	D_RBlockStream_t* Stream;
	
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
	D_RBSBaseBlock(Stream, "LPRJ");
	D_RBSWriteString(Stream, a_Profile->UUID);
	D_RBSWriteString(Stream, a_Profile->AccountName);
	D_RBSWriteString(Stream, a_Profile->DisplayName);
	D_RBSWriteUInt8(Stream, a_Profile->Color);
	D_RBSRecordNetBlock(Stream, &Server->Address);
}

/*** NSZZ FUNCTIONS ***/

/* D_NSZZ_SendINFO() -- Send server info */
void D_NSZZ_SendINFO(struct D_RBlockStream_s* a_Stream, const uint32_t a_LocalTime)
{
	const WL_WADFile_t* Rover;
	uint8_t u8;
	
	/* Write Version */
	D_RBSWriteUInt8(a_Stream, VERSION);
	D_RBSWriteUInt8(a_Stream, REMOOD_MAJORVERSION);
	D_RBSWriteUInt8(a_Stream, REMOOD_MINORVERSION);
	D_RBSWriteUInt8(a_Stream, REMOOD_RELEASEVERSION);
	D_RBSWriteString(a_Stream, REMOOD_VERSIONCODESTRING);
	
	/* Write Time Info */
	D_RBSWriteUInt32(a_Stream, a_LocalTime);
	D_RBSWriteUInt32(a_Stream, time(NULL));
	D_RBSWriteUInt32(a_Stream, D_SyncNetMapTime());
	D_RBSWriteUInt32(a_Stream, 0);
	
	/* Write Server Name */
	D_RBSWriteString(a_Stream, l_SVName.Value->String);
	D_RBSWriteString(a_Stream, l_SVEMail.Value->String);
	D_RBSWriteString(a_Stream, l_SVURL.Value->String);
	D_RBSWriteString(a_Stream, l_SVWADURL.Value->String);
	D_RBSWriteString(a_Stream, l_SVIRC.Value->String);
	
	/* Passwords */
	// Connect Password
	u8 = '-';
	if (strlen(l_SVConnectPassword.Value->String) > 0)
		u8 = 'P';
	D_RBSWriteUInt8(a_Stream, u8);
	
	// Join Password
	u8 = '-';
	if (strlen(l_SVJoinPassword.Value->String) > 0)
		u8 = 'J';
	D_RBSWriteUInt8(a_Stream, u8);
	
	/* Write WAD Info */
	for (Rover = WL_IterateVWAD(NULL, true); Rover; Rover = WL_IterateVWAD(Rover, true))
	{
		// Write start
		D_RBSWriteUInt8(a_Stream, 'W');
		
		// TODO: Optional WAD
		D_RBSWriteUInt8(a_Stream, 'R');
		
		// Write Names for WAD (DOS and Base)
		D_RBSWriteString(a_Stream, WL_GetWADName(Rover, false));
		D_RBSWriteString(a_Stream, WL_GetWADName(Rover, true));
		
		// Write File Sums
		D_RBSWriteString(a_Stream, Rover->SimpleSumChars);
		D_RBSWriteString(a_Stream, Rover->CheckSumChars);
	}
	
	// End List
	D_RBSWriteUInt8(a_Stream, 'X');
	
	/* Level Name */
	switch (gamestate)
	{
			// In Game
		case GS_LEVEL:
			D_RBSWriteString(a_Stream, (g_CurrentLevelInfo ? g_CurrentLevelInfo->LumpName : "<UNKNOWN"));
			break;
			
			// Non-Games
		case GS_INTERMISSION: D_RBSWriteString(a_Stream, "<INTERMISSION>"); break;
		case GS_FINALE: D_RBSWriteString(a_Stream, "<STORY>"); break;
		case GS_DEMOSCREEN: D_RBSWriteString(a_Stream, "<TITLESCREEN>"); break;
			
			// Unknown
		default:
			D_RBSWriteString(a_Stream, "<UNKNOWN>");
			break;
	}
}

/* D_NSZZ_SendINFX() -- Extended Info */
// This sends all variables
bool_t D_NSZZ_SendINFX(struct D_RBlockStream_s* a_Stream, size_t* const a_It)
{
	size_t EndIt;
	P_EXGSVariable_t* XVar;
	
	/* Get End */
	EndIt = *a_It + 5;
	
	/* Loop */
	for (; *a_It < EndIt && *a_It < PEXGSNUMBITIDS; (*a_It)++)
	{
		// Write Marker
		D_RBSWriteUInt8(a_Stream, 'V');
		
		// Get Var
		XVar = P_EXGSVarForBit(*a_It);
		
		if (!XVar)
			continue;
		
		// Write Name and value
		D_RBSWriteString(a_Stream, XVar->Name);
		D_RBSWriteUInt32(a_Stream, (XVar->WasSet ? XVar->ActualVal : XVar->DefaultVal));
	}
	
	/* End */
	D_RBSWriteUInt8(a_Stream, 'E');
	
	/* More variables available? */
	if (*a_It < PEXGSNUMBITIDS)
		return true;
	return false;
}

/* D_NSZZ_SendMOTD() -- Send Message Of The Day */
void D_NSZZ_SendMOTD(struct D_RBlockStream_s* a_Stream)
{
	D_RBSWriteString(a_Stream, l_SVMOTD.Value->String);
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


