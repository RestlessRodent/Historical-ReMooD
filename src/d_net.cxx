// -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
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
#include "p_saveg.h"
#include "p_setup.h"
#include "m_random.h"
#include "i_video.h"
#include "p_local.h"
#include "p_inter.h"

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
bool D_SyncNetIsArbiter(void)
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
bool D_SyncNetIsPaused(void)
{
	if (D_SyncNetIsSolo() && !demoplayback && (M_ExUIActive() || (l_CONPauseGame.Value->Int && CONL_IsActive())))
		return true;
	return false;
}

/* D_SyncNetIsSolo() -- Is solo game (non-networked) */
bool D_SyncNetIsSolo(void)
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
bool D_SyncNetUpdate(void)
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
bool D_NetSetPlayerName(const int32_t a_PlayerID, const char* const a_Name)
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
bool D_NetPlayerChangedPause(const int32_t a_PlayerID)
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

/*** CONSTANTS ***/

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

// c_CVPVMaxSplit -- Max Splitscreen Players
const CONL_VarPossibleValue_t c_CVPVMaxSplit[] =
{
	// End
	{1, "MINVAL"},
	{MAXSPLITSCREEN, "MAXVAL"},
	{0, NULL},
};

// c_CVPVMaxPlayers -- Max Players
const CONL_VarPossibleValue_t c_CVPVMaxPlayers[] =
{
	// End
	{1, "MINVAL"},
	{MAXPLAYERS, "MAXVAL"},
	{0, NULL},
};

// sv_maxsplitscreen -- Max players per connection
CONL_StaticVar_t l_SVMaxSplitScreen =
{
	CLVT_INTEGER, c_CVPVMaxSplit, CLVF_SAVE,
	"sv_maxsplitscreen", DSTR_CVHINT_SVMAXSPLITSCREEN, CLVVT_STRING, "4",
	NULL
};

// sv_maxplayers -- Max players allowed inside the game
CONL_StaticVar_t l_SVMaxPlayers =
{
	CLVT_INTEGER, c_CVPVMaxPlayers, CLVF_SAVE,
	"sv_maxplayers", DSTR_CVHINT_SVMAXPLAYERS, CLVVT_STRING, "32",
	NULL
};

// c_CVPVMaxClients -- Max Clients
const CONL_VarPossibleValue_t c_CVPVMaxClients[] =
{
	// End
	{0, "MINVAL"},
	{230, "MAXVAL"},
	{0, NULL},
};

// sv_maxclients -- Max clients allowed to connect
CONL_StaticVar_t l_SVMaxClients =
{
	CLVT_INTEGER, c_CVPVMaxClients, CLVF_SAVE,
	"sv_maxclients", DSTR_CVHINT_SVMAXCLIENTS, CLVVT_STRING, "32",
	NULL
};

/*** FUNCTIONS ***/

/* D_NCAllocClient() -- Creates a new network client */
D_NetClient_t* D_NCAllocClient(void)
{
	size_t i;
	D_NetClient_t* New;
	
	/* Allocate */
	New = (D_NetClient_t*)Z_Malloc(sizeof(*New), PU_NETWORK, NULL);
	
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
void D_NCFudgeOffHostStream(I_HostAddress_t* const a_Host, struct D_RBlockStream_s* a_Stream, const char a_Code, const char* const a_Reason)
{
	/* Check */
	if (!a_Host || !a_Stream)
		return;
	
	/* Send FOFF Message */
	D_RBSBaseBlock(a_Stream, "FOFF");
	D_RBSWriteUInt8(a_Stream, a_Code);
	D_RBSWriteString(a_Stream, a_Reason);
	D_RBSRecordNetBlock(a_Stream, a_Host);
	
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
static CONL_ExitCode_t DS_ConnectMultiCom(const uint32_t a_ArgC, const char** const a_ArgV)
{
	D_NetClient_t* ServerNC;
	I_HostAddress_t Host;
	
	/* Clear Host */
	memset(&Host, 0, sizeof(Host));
	
	/* Connect */
	if (strcasecmp("startserver", a_ArgV[0]) == 0)
	{
		DNetController::StartServer();//D_NCServize();
		
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
bool D_CheckNetGame(void)
{
	I_NetSocket_t* Socket;
	D_NetClient_t* Client;
	bool ret = false;
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
	CONL_VarRegister(&l_SVMaxSplitScreen);
	CONL_VarRegister(&l_SVMaxPlayers);
	CONL_VarRegister(&l_SVMaxClients);
		
	/* Create LoopBack Client */
	Client = D_NCAllocClient();
	Client->IsLocal = true;
	Client->CoreStream = D_RBSCreateLoopBackStream();
	
	// Create perfection Wrapper
	Client->PerfectStream = D_RBSCreatePerfectStream(Client->CoreStream);
	
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
		Client->CoreStream = D_RBSCreateNetStream(Client->NetSock);
		
		// Create encapsulated perfect stream
		Client->PerfectStream = D_RBSCreatePerfectStream(Client->CoreStream);
	
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
			NQC = l_ComQueue[i] = (D_NetQueueCommand_t*)Z_Malloc(sizeof(*NQC), PU_NETWORK, NULL);
			break;
		}
	
	// No Spot?
	if (!NQC)
	{
		Z_ResizeArray((void**)&l_ComQueue, sizeof(*l_ComQueue), l_NumComQueue, l_NumComQueue + 1);
		NQC = l_ComQueue[l_NumComQueue++] = (D_NetQueueCommand_t*)Z_Malloc(sizeof(*NQC), PU_NETWORK, NULL);
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
					D_RBSCloseStream(l_Clients[i]->PerfectStream);
				if (l_Clients[i]->CoreStream)
					D_RBSCloseStream(l_Clients[i]->CoreStream);
					
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
bool D_NCHostOnBanList(I_HostAddress_t* const a_Host)
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
void D_NCReqAddPlayer(struct D_ProfileEx_s* a_Profile, const bool a_Bot)
{
	D_NetClient_t* Server;
	D_RBlockStream_t* Stream;
	
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
	D_RBSBaseBlock(Stream, "LPRJ");
	D_RBSWriteString(Stream, a_Profile->UUID);
	D_RBSWriteString(Stream, a_Profile->AccountName);
	D_RBSWriteString(Stream, a_Profile->DisplayName);
	D_RBSWriteUInt8(Stream, a_Profile->Color);
	D_RBSWriteUInt8(Stream, a_Bot);
	D_RBSRecordNetBlock(Stream, &Server->Address);
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
typedef bool (*D_NCMessageHandlerFunc_t)(struct D_NCMessageData_s* const a_Data);

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
	D_NetClient_t* RemoteClient;				// Attached Remote Client
	D_RBlockStream_t* InStream;					// Stream to read from
	D_RBlockStream_t* OutStream;				// Stream to write to
	I_HostAddress_t* FromAddr;					// Address message is from
	uint32_t FlagsMask;							// Mask for flags
	
	RBPerfectStream_c* PIn;
	RBPerfectStream_c* POut;
	RBStream_c* BIn;
	RBStream_c* BOut;
	bool IsServ, IsClient, IsHost, IsPerf;
	DNetController* DNC;
	DNetController* RC;
	RBAddress_c* ReadAddr;
	uint8_t NodeNum;							// Node number of the source
} D_NCMessageData_t;

/*** FUNCTIONS ***/

bool D_NCMH_LocalPlayerRJ(struct D_NCMessageData_s* const a_Data);
bool D_NCMH_PlayerJoinOK(struct D_NCMessageData_s* const a_Data);

// c_NCMessageCodes -- Local messages
static const D_NCMessageType_t c_NCMessageCodes[] =
{
	{1, "LPRJ", D_NCMH_LocalPlayerRJ, (D_NCMessageFlag_t)(DNCMF_PERFECT | DNCMF_SERVER | DNCMF_HOST | DNCMF_REMOTECL)},
	{1, "PJOK", D_NCMH_PlayerJoinOK, (D_NCMessageFlag_t)(DNCMF_PERFECT | DNCMF_SERVER | DNCMF_REMOTECL | DNCMF_DEMO)},
	
	// EOL
	{0, ""},
};

/* D_NCUpdate() -- Update all networking stuff */
void D_NCUpdate(void)
{
	int32_t nc, IsPerf, tN;
	char Header[5];
	I_HostAddress_t FromAddress;
	D_NetClient_t* NetClient, *RemoteClient;
	D_RBlockStream_t* PIn, *POut, *BOut;
	bool IsServ, IsClient, IsHost;
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
		while (PIn && D_RBSPlayNetBlock(PIn, Header, &FromAddress))
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
			D_RBSStreamIOCtl(PIn, DRBSIOCTL_ISPERFECT, &IsPerf);
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
		D_RBSFlushStream(POut);
		D_RBSFlushStream(BOut);
	}


#if 0
#define BUFSIZE 512
	char Buf[BUFSIZE];
	char ZBuf[BUFSIZE];
	char Header[5];
	D_RBlockStream_t* Stream, *OutStream, *GenOut;
	D_NetClient_t* NetClient, *OtherClient;
	size_t nc, snum, i, p, j;
	I_HostAddress_t FromAddress;
	D_NetPlayer_t* NetPlayer;
	D_ProfileEx_t* Profile;
	player_t* DoomPlayer;
	char* charp;
	char* chara[5];
	const char* charc;
	
	const WL_WADFile_t* RemIWAD;
	const WL_WADFile_t* RemRWAD;
	const WL_WADFile_t* FoundWAD;
	const WL_WADFile_t* LastWAD;
	
	bool SendKeep;
	bool SendPing, ReSend;
	uint32_t ThisTime, DiffTime;
	static uint32_t LastKeep;
	static uint32_t LastTime;
	
	bool DoContinue, IsOK, OrderOK;
	uint32_t u32a, u32b, u32c, u32d;
	uint8_t u8a, u8b, u8c, u8d;
	
	/* Send Ping Request? */
	ThisTime = I_GetTimeMS();
	
	// Send pings? Every 10s
	SendPing = false;
	if (ThisTime > LastTime + 10000)
	{
		DiffTime = ThisTime - LastTime;
		LastTime = ThisTime;
		SendPing = true;
		
		// Set global stat count to current local stats
		for (i = 0; i < 4; i++)
			g_NetStat[i] = l_LocalStat[i];
	}
	
	// Send keep alive? Every minute
		// Because perfect connections that idle for too long will eventually
		// get revoked.
	SendKeep = false;
	if (ThisTime > LastKeep + 60000)
	{
		LastKeep = ThisTime;
		SendKeep = true;
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
		
		// Determine streams to use
		Stream = NetClient->Streams[DNCSP_PERFECTREAD];
		OutStream = NetClient->Streams[DNCSP_PERFECTWRITE];
		GenOut = NetClient->Streams[DNCSP_WRITE];
		
		// Sending a ping?
			// If sending, send command and unstat stream
		if (SendPing)
		{
			// Unstat the stream
			D_RBSUnStatStream(GenOut);
			
			// Build PING command
			D_RBSBaseBlock(GenOut, "PING");
			D_RBSWriteUInt32(GenOut, ThisTime);
			D_RBSWriteUInt32(GenOut, DiffTime);
			D_RBSRecordNetBlock(GenOut, &NetClient->Address);
		}
			// Otherwise, Stat the stream and add to local counts
		else
		{
			// Stat it
			D_RBSStatStream(GenOut, &u32a, &u32b, &u32c, &u32d);
			
			// Add to local
			l_LocalStat[0] += u32a;
			l_LocalStat[1] += u32b;
			l_LocalStat[2] += u32c;
			l_LocalStat[3] += u32d;
		}
		
		// Send Keepalive to the perfect stream?
		if (SendKeep)
		{
			D_RBSBaseBlock(OutStream, "KEEP");
			D_RBSRecordNetBlock(OutStream, &NetClient->Address);
		}
		
		// Read from the "Perfect" Stream
			// The perfect stream knows whether a packet is perfect or not and
			// if a perfect packet is not yet ready it won't return any of them
			// Also, a read stream might not exist, a client could be using
			// another clients stream for reading, this would be the case for
			// network games over UDP. Why? Becuase all network players write
			// to the server (the local client) for commands.
		
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
		
		// REDY -- Client is ready
		else if (D_RBSCompareHeader("REDY", Header))
		{
			// Only accept if is a server
			if (!(NetClient->IsServer && NetClient->IsLocal))
				continue;
			
			// Find the client that wants to do this
			OtherClient = D_NCFindClientByHost(&FromAddress);
			
			// Nothing found?
			if (!OtherClient)
				continue;
			
			// Mark as ready
			OtherClient->ReadyToPlay = true;
			
			// Send Save Game
			D_NCHE_SendSaveGame(OtherClient);
		}
		
		// WADQ -- Query WADS
		else if (D_RBSCompareHeader("WADQ", Header))
		{
			// Only accept if is a server
			if (!(NetClient->IsServer && NetClient->IsLocal))
				continue;
			
			// Find the client that wants to do this
			OtherClient = D_NCFindClientByHost(&FromAddress);
			
			// Nothing found?
			if (!OtherClient)
				continue;
			
			// Send WAD configuration
			D_NSZZ_SendFullWADS(OutStream, &FromAddress);
		}
		
		// WADS -- Server's WAD Configuration
		else if (D_RBSCompareHeader("WADS", Header))
		{
			// Only accept if from a server and we aren't local
			if (!(NetClient->IsServer && !NetClient->IsLocal))
				continue;
			
			// Get current IWAD and ReMooD.WAD
			RemIWAD = WL_IterateVWAD(NULL, true);
			RemRWAD = WL_IterateVWAD(RemIWAD, true);
			FoundWAD = WL_IterateVWAD(NULL, true);
			
			// Reset
			OrderOK = true;
			DoContinue = true;
			
			// Read Input WADs
			CONL_PrintF("*** Server WADs ***\n");
			do
			{
				// Clear Buffers
				memset(ZBuf, 0, sizeof(ZBuf));
				
				// Read Marker
				u8a = D_RBSReadUInt8(Stream);
				
				// End?
				if (u8a == 'X')
					break;
				
				// Optional Bit
				u8b = D_RBSReadUInt8(Stream);
				
				// Read DOS Name, Real Name, SS, MD5
				for (j = 0; j < 4; j++)
				{
					D_RBSReadString(Stream, Buf, BUFSIZE - 1);
					strncat(ZBuf, Buf, BUFSIZE - 1);
					
					if (j < 3)
						strncat(ZBuf, "\1", BUFSIZE - 1);
				}
				
				// Convert all \1s to \0s
				memset(chara, 0, sizeof(chara));
				j = 0;
				charp = ZBuf;
				chara[j++] = charp;
				while ((charp = strchr(charp, 1)))
				{
					*(charp++) = 0;
					chara[j++] = charp;
				}
				
				// Print WAD
				charp = ZBuf;
				CONL_PrintF("%c: \"%s\"/\"%s\": [SS=%s, MD5=%s]\n",
					u8b, chara[0], chara[1], chara[2], chara[3]);
				charp += strlen(charp) + 1;
				
				// Compare current WAD to server
				IsOK = false;
				if (OrderOK)
				{
					// Compare name or sum?
					if (u8b == 'N')
						charc = FoundWAD->__Private.__DOSName;
					else
						charc = FoundWAD->SimpleSumChars;
					
					// Compare True
					if (strcasecmp(charc, (u8b == 'N' ? chara[0] : chara[2])) == 0)
					{
						// Set as OK
						IsOK = true;
					}
					
					// Mismatch
					else
					{
						// Message
						CONL_PrintF("\"%s\" [%s] != \"%s\" [%s]\n",
								FoundWAD->__Private.__DOSName,
								FoundWAD->SimpleSumChars,
								
								chara[0],
								chara[2]
							);
						
						// WADs were never popped
						if (u8b != 'O')	// Ignore optionals
							if (OrderOK)
							{
								// Lock OCCB
								WL_LockOCCB(true);
								
								// Pop until the current WAD
								do
								{
									LastWAD = WL_PopWAD();
								}
								while (LastWAD != FoundWAD);
							
								// Set as popped
								OrderOK = false;
							}
					}
				}
				
				// WAD was not OK (missing?)
				if (!OrderOK && !IsOK)
				{
					// Try loading the WAD
						// DOS Name
					FoundWAD = WL_OpenWAD(chara[0]);
					
					// Real Name?
					if (!FoundWAD)
						FoundWAD = WL_OpenWAD(chara[1]);
					
					// Still failed?
					if (!FoundWAD)
					{
						// Not ready to join the game
						DoContinue = false;
						
						// Request WAD from server
						D_NCSR_RequestWAD(chara[2]);
					}
					
					// Otherwise Push it
					else
						WL_PushWAD(FoundWAD);
				}
				
				// Otherwise, iterate to the next WAD
				else
					FoundWAD = WL_IterateVWAD(FoundWAD, true);
			} while (u8a != 'X');
			CONL_PrintF("*******************\n");
			
			// UnLock OCCB if WADs were changed
			if (!OrderOK)
				WL_LockOCCB(false);
			
			// Inform server that we are ready for save game transmit
			if (DoContinue)
				D_NCSR_SendServerReady();
				
#if 0
			// Lock OCCB
			WL_LockOCCB(true);
			
			// Pop all wads
			while (WL_PopWAD())
				;
			
			// Read all WADs
			OrderOK = true;
			DoContinue = true;
			do
			{
				// Read Marker
				u8a = D_RBSReadUInt8(Stream);
				
				// End?
				if (u8a == 'X')
					break;
				
				// Read whether WAD is required or not (this is important)
				u8b = D_RBSReadUInt8(Stream);
				
				// Read DOS Name -- And try opening that...
				D_RBSReadString(Stream, Buf, BUFSIZE - 1);
				
				FoundWAD = WL_OpenWAD(Buf);
				
				// Read Normal Name -- And try opening that if DOS failed us...
				D_RBSReadString(Stream, Buf, BUFSIZE - 1);
				
				if (!FoundWAD)
					FoundWAD = WL_OpenWAD(Buf);
				
				// No WAD was found at all and it is required
				if (!FoundWAD && u8b != 'R')
				{
					DoContinue = false;
					IsOK = true;
					
					// See if SS is on blacklist
					D_RBSReadString(Stream, Buf, BUFSIZE - 1);
					if (!D_CheckWADBlacklist(Buf))
						IsOK = false;
					
					// See if MD5 is on blacklist
					j = strlen(Buf);
					D_RBSReadString(Stream, Buf + j + 1, (BUFSIZE - j) - 2);
					if (!D_CheckWADBlacklist(Buf + j + 1))
						IsOK = false;
						
					// Request WAD Download, if possible
					if (IsOK)
						D_NCSR_RequestWAD(Buf);
				}
				
				// WAD was found, push it
				else
				{
					// Push to the stack
					WL_PushWAD(FoundWAD);
					
					// Ignore SUMs
					D_RBSReadString(Stream, Buf, BUFSIZE - 1);
					D_RBSReadString(Stream, Buf, BUFSIZE - 1);
				}
			} while (u8a == 'W');
			
			// Failed to find a WAD
			if (!DoContinue)
			{
				// Make sure there is an IWAD
				FoundWAD = WL_IterateVWAD(NULL, true);
				
				// If there isn't (we are very lacking today)
				if (!FoundWAD)
				{
					// Push original IWAD and RWAD
					WL_PushWAD(RemIWAD);
					WL_PushWAD(RemRWAD);
					
					// Disconnect from server
					D_NCQueueDisconnect();
				}
			}
			
			// UnLock OCCB
			WL_LockOCCB(false);
			
			// Success! Tell server we are ready for the join window
			if (DoContinue)
				D_NCSR_SendServerReady();
#endif
		}
		
		// WELC -- Connection successful
		else if (D_RBSCompareHeader("WELC", Header))
		{
			// Only accept if from a server and we aren't local
			if (!(NetClient->IsServer && !NetClient->IsLocal))
				continue;
			
			// Request MOTD from server (so it can be displayed)
		}
		
		// DISC -- Disconnection Request
		else if (D_RBSCompareHeader("DISC", Header))
		{
			// Only accept from non-local clients
			if (!(!NetClient->IsServer && !NetClient->IsLocal))
				continue;
		}
		
		// CONN -- Connection Request
		else if (D_RBSCompareHeader("CONN", Header))
		{
			// Read Version
			u8a = D_RBSReadUInt8(Stream);
			u8b = D_RBSReadUInt8(Stream);
			u8c = D_RBSReadUInt8(Stream);
			u8d = D_RBSReadUInt8(Stream);
			
			// Version Mismatch?
			if (u8a != VERSION &&
				u8b != REMOOD_MAJORVERSION &&
				u8c != REMOOD_MINORVERSION &&
				u8d != REMOOD_RELEASEVERSION)
			{
				snprintf(Buf, BUFSIZE - 1, "You need to version %i.%i%c (%i) and not version %i.%i%c (%i)",
						REMOOD_MAJORVERSION,
						REMOOD_MINORVERSION,
						REMOOD_RELEASEVERSION,
						VERSION,
						
						u8b, u8c, u8d, u8a
					);
				D_NCFudgeOffHostStream(&FromAddress, Stream, 'V', Buf);
				continue;
			}
			
			// Compare connect password
				// Read password
			memset(Buf, 0, sizeof(Buf));
			D_RBSReadString(Stream, Buf, BUFSIZE - 1);
			
			// Only if password is set
			if (strlen(l_SVConnectPassword.Value->String) > 0)
				if (strcasecmp(Buf, l_SVConnectPassword.Value->String) != 0)
				{
					snprintf(Buf, BUFSIZE - 1, "Incorrect password");
					D_NCFudgeOffHostStream(&FromAddress, Stream, 'P', Buf);
					continue;
				}
			
			// Successfully connected, add client to network clients
			OtherClient = D_NCAllocClient();
			
			// Set information
			memmove(&OtherClient->Address, &FromAddress, sizeof(FromAddress));
			
			// Set output streams to the current stream
			I_NetHostToName(&FromAddress, OtherClient->ReverseDNS, NETCLIENTRHLEN);
			OtherClient->Streams[DNCSP_WRITE] = NetClient->Streams[DNCSP_WRITE];
			OtherClient->Streams[DNCSP_PERFECTWRITE] = NetClient->Streams[DNCSP_PERFECTWRITE];
			
			// Send welcome message
			D_RBSBaseBlock(OutStream, "WELC");
			D_RBSRecordNetBlock(OutStream, &FromAddress);
			
			// Send the currently loaded WADs
			D_NSZZ_SendFullWADS(OutStream, &FromAddress);
			
			// Inform the server of the join
			CONL_OutputU(DSTR_NET_CLIENTCONNECTED, "%s\n", OtherClient->ReverseDNS);
		}
		
		// FOFF -- Server told us to get lost
		else if (D_RBSCompareHeader("FOFF", Header))
		{
			// Only accept if from a server and we aren't local
			if (!(NetClient->IsServer && !NetClient->IsLocal))
				continue;
			
			// Extract reason why
			u8a = D_RBSReadUInt8(Stream);				// Code
			memset(Buf, 0, sizeof(Buf));
			D_RBSReadString(Stream, Buf, BUFSIZE - 1);	// Reason
			
			// Write to console
			CONL_PrintF("%c: %s\n", u8a, Buf);
			
			// Disconnect
			D_NCDisconnect();
			
			// Tell the user why
			//M_ExUIMessageBox(const M_ExMBType_t a_Type, const uint32_t a_MessageID, const char* const a_Title, const char* const a_Message, const MBCallBackFunc_t a_CallBack);
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
				{
					if (devparm)
						CONL_OutputU(DSTR_NET_BADCLIENT, "\n");
					continue;
				}
				
				// Client not ready?
				if (!OtherClient->ReadyToPlay)
					continue;
				
				// Client was not sent savegame?
				if (!OtherClient->IsLocal && !OtherClient->SaveGameSent)
					continue;
				
				// Read the UUID
				D_RBSReadString(Stream, Buf, BUFSIZE - 1);
				
				// Only if one was found is it parsed
					// If not, maybe someone else is screwing with the server?
				// Client is arbing too many?
				if (OtherClient->NumArbs >= MAXSPLITSCREEN)
				{
					CONL_OutputU(DSTR_NET_EXCEEDEDSPLIT, "\n");
					continue;
				}
				
				// Check for free player slots
				for (p = 0; p < MAXPLAYERS; p++)
					if (!playeringame[p])
						break;
				
				// No Free Slots
				if (p >= MAXPLAYERS)
				{
					CONL_OutputU(DSTR_NET_ATMAXPLAYERS, "\n");
					continue;
				}
				
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
				
				// Queue Request
				D_NCHE_ServerCreatePlayer(p, NetPlayer, Profile, OtherClient);
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
	
	bool SendPing, AnythingWritten;
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
#endif
}

/* D_NCMH_LocalPlayerRJ() -- Player wants to join game */
bool D_NCMH_LocalPlayerRJ(struct D_NCMessageData_s* const a_Data)
{
	char UUID[MAXPLAYERNAME * 2];
	char AccountName[MAXPLAYERNAME];
	char DisplayName[MAXPLAYERNAME];
	uint8_t Color, Bot, Bits;
	int32_t i, PlCount, FreeSlot;
	uint32_t ProcessID;
	D_RBlockStream_t* Stream;
	
	/* Check */
	if (!a_Data)
		return false;
	
	/* Remote player is neither ready nor got a save game sent to them */
	if (!a_Data->RemoteClient->ReadyToPlay || !a_Data->RemoteClient->SaveGameSent)
		return false;
	
	/* Get their input stream */
	Stream = a_Data->InStream;
	
	/* Read Message Data */
	D_RBSReadString(Stream, UUID, MAXPLAYERNAME * 2);
	D_RBSReadString(Stream, AccountName, MAXPLAYERNAME);
	D_RBSReadString(Stream, DisplayName, MAXPLAYERNAME);
	Color = D_RBSReadUInt8(Stream);
	Bot = D_RBSReadUInt8(Stream);
	
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
		D_RBSBaseBlock(Stream, "PJOK");
		
		// Data
		D_RBSWriteUInt32(Stream, ProcessID);
		D_RBSWriteUInt8(Stream, FreeSlot);
		D_RBSWriteUInt8(Stream, Bot);
		D_RBSWriteString(Stream, UUID);
		D_RBSWriteString(Stream, AccountName);
		D_RBSWriteString(Stream, DisplayName);
		D_RBSWriteUInt8(Stream, Color);
		
		// Send away
		D_RBSRecordNetBlock(Stream, &l_Clients[i]->Address);
	}
	
	/* Don't handle again */
	return true;
}

/* D_NCMH_PlayerJoinOK() -- Player joins the game */
bool D_NCMH_PlayerJoinOK(struct D_NCMessageData_s* const a_Data)
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
	ProcessID = D_RBSReadUInt32(a_Data->InStream);
	FreeSlot = D_RBSReadUInt8(a_Data->InStream);
	Bot = D_RBSReadUInt8(a_Data->InStream);
	D_RBSReadString(a_Data->InStream, UUID, MAXPLAYERNAME * 2);
	D_RBSReadString(a_Data->InStream, AccountName, MAXPLAYERNAME);
	D_RBSReadString(a_Data->InStream, DisplayName, MAXPLAYERNAME);
	Color = D_RBSReadUInt8(a_Data->InStream);
	
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
	bool LocalHit;
	
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
	D_RBlockStream_t* Stream;
	
	/* Find Server */
	Server = D_NCFindClientIsServer();
	
	// Not found?
	if (!Server)
		return;
	
	/* Tell server to add player */
	// Use server stream
	Stream = Server->Streams[DNCSP_PERFECTWRITE];
	
	// Put Data
	D_RBSBaseBlock(Stream, "REDY");
	D_RBSRecordNetBlock(Stream, &Server->Address);
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
bool D_NSZZ_SendINFX(struct D_RBlockStream_s* a_Stream, size_t* const a_It)
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
		XVar = P_EXGSVarForBit((P_EXGSBitID_t)(*a_It));
		
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

/* D_NSZZ_SendFullWADS() -- Send WADs of Server */
void D_NSZZ_SendFullWADS(struct D_RBlockStream_s* a_Stream, I_HostAddress_t* const a_Host)
{
	const WL_WADFile_t* Rover;
	int i;
	
	/* Block */
	D_RBSBaseBlock(a_Stream, "WADS");
	
	/* Write WAD Info */
	for (i = 0, Rover = WL_IterateVWAD(NULL, true); Rover; Rover = WL_IterateVWAD(Rover, true), i++)
	{
		// Write start
		D_RBSWriteUInt8(a_Stream, 'W');
		
		// ReMooD.WAD is name matched only
		if (i == 1)
			D_RBSWriteUInt8(a_Stream, 'N');
		else
			// TODO: Optional WADs
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
	
	/* Record it */
	D_RBSRecordNetBlock(a_Stream, a_Host);
}

/* D_NCHE_SendSaveGame() -- Send savegame to client */
void D_NCHE_SendSaveGame(D_NetClient_t* const a_Client)
{
	struct D_RBlockStream_s* Stream;
	
	/* Check */
	if (!a_Client)
		return;
	
	/* Set Info */
	Stream = a_Client->Streams[DNCSP_PERFECTWRITE];
	
	/* Send Start */
	D_RBSBaseBlock(Stream, "SAVE");
	D_RBSRecordNetBlock(Stream, &a_Client->Address);
	
	/* Send Save */
	//P_SaveGameToBS(Stream, &a_Client->Address);
	
	/* Send End */
	D_RBSBaseBlock(Stream, "SAVX");
	D_RBSRecordNetBlock(Stream, &a_Client->Address);
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

/*****************************
*** CLASS BASED NETWORKING ***
*****************************/

/*** CONSTANTS ***/

static const fixed_t c_forwardmove[2] = { 25, 50 };
static const fixed_t c_sidemove[2] = { 24, 40 };
static const fixed_t c_angleturn[3] = { 640, 1280, 320 };	// + slow turn
#define MAXPLMOVE       (c_forwardmove[1])

#define MAXLOCALJOYS	4

/*** GLOBALS ***/

extern int32_t g_IgnoreWipeTics;				// Ignore tics ran during wipe (no speedup after wipe)

/*** LOCALS ***/

static DNetController** l_ServerNCS;			// Server Controllers
static size_t l_NumServerNCS;					// Number of controllers

DNetCommand** DNetController::p_CommandQ;		// Command Queue
size_t DNetController::p_NumCommandQ;			// Size of Q
tic_t DNetController::p_ReadyTime = 0;			// Current Ready Time
RBMultiCastStream_c* DNetController::p_MulCast;	// Multi Cast
int32_t DNetController::p_LocalCount = 0;		// Local network players count
int32_t DNetController::p_RemoteCount = 0;		// Remote network players count
tic_t DNetController::p_LastTime = 0;			// Last time for ready tics
bool DNetController::p_IsGameHost = false;		// Is host of the game
tic_t DNetController::p_Readies = 0;			// Tics that are ready
tic_t DNetController::p_ServerLag = 0;			// Extra Server Lag

DNetPlayer** DNetPlayer::p_Players = NULL;		// Net players
size_t DNetPlayer::p_NumPlayers = 0;			// Number of them

static bool l_PermitMouse = false;				// Use mouse input
static int32_t l_MouseMove[2] = {0, 0};			// Mouse movement (x/y)
static bool l_KeyDown[NUMIKEYBOARDKEYS];		// Keys that are down
static uint32_t l_JoyButtons[MAXLOCALJOYS];		// Local Joysticks
static int16_t l_JoyAxis[MAXLOCALJOYS][MAXJOYAXIS];

/*** CLASSES ***/

/* DNetPlayer::DNetPlayer() -- Constructor */
DNetPlayer::DNetPlayer(const uint32_t a_Code, const int32_t a_PID)
{
	size_t i;
	
	/* Copy Code */
	p_Code = a_Code;
	p_PID = a_PID;
	p_PlayerTime = gametic;
	
	/* Find free spot in player list */
	for (i = 0; i < p_NumPlayers; i++)
		if (!p_Players[i])
		{
			p_Players[i] = this;
			break;
		}
	
	// Not found?
	if (i >= p_NumPlayers)
	{
		Z_ResizeArray((void**)&p_Players, sizeof(*p_Players), p_NumPlayers, p_NumPlayers + 1);
		p_Players[p_NumPlayers++] = this;
	}
	
	/* Set player ref */
	p_Player = &players[p_PID];
}

/* DNetPlayer::~DNetPlayer() -- Deconstructor */
DNetPlayer::~DNetPlayer()
{
	size_t i;
	
	/* Find spot we are at in the player list */
	for (i = 0; i < p_NumPlayers; i++)
		if (p_Players[i] == this)
		{
			p_Players[i] = NULL;
			break;
		}
}

/* DNetPlayer::IsBot() -- Returns true if player is a bot */
bool DNetPlayer::IsBot(void)
{
	return p_Bot;
}

/* DNetPlayer::SetBot() -- Sets whether the player is a bot */
void DNetPlayer::SetBot(const bool a_Val)
{
	p_Bot = a_Val;
}

/* DNetPlayer::GetBotLastTime() -- Returns reference to bot's last time */
tic_t& DNetPlayer::GetBotLastTime(void)
{
	return p_BotLastTime;
}

/* DNetPlayer::GetLastCommand() -- Returns the player's last command */
ticcmd_t& DNetPlayer::GetLastCommand(void)
{
	return p_LastCmd;
}

/* DNetPlayer::IsLocal() -- Returns true if player is local */
bool DNetPlayer::IsLocal(void)
{
	return p_Local;
}

/* DNetPlayer::SetLocal() -- Set local player */
void DNetPlayer::SetLocal(const bool a_Val)
{
	p_Local = a_Val;
}

/* DNetPlayer::SetProfile() -- Sets network profile */
void DNetPlayer::SetProfile(D_ProfileEx_t* const a_Prof)
{
	p_Profile = a_Prof;
}

/* DNetPlayer::GetProfile() -- Returns network profile */
D_ProfileEx_t* DNetPlayer::GetProfile(void)
{
	return p_Profile;
}

/* DNetPlayer::GAMEKEYDOWN() -- Key is pressed? */
bool DNetPlayer::GAMEKEYDOWN(D_ProfileEx_t* const a_Profile, const uint8_t a_Key)
{
	size_t i;
	uint32_t CurrentButton;
	
	/* Check Keyboard */
	for (i = 0; i < 4; i++)
		if (a_Profile->Ctrls[a_Key][i] >= 0 && a_Profile->Ctrls[a_Key][i] < NUMIKEYBOARDKEYS)
			if (l_KeyDown[a_Profile->Ctrls[a_Key][i]])
				return true;
	
	/* Check Joysticks */
	if (a_Profile->Flags & DPEXF_GOTJOY)
		if (a_Profile->JoyControl >= 0 && a_Profile->JoyControl < 4)
			for (i = 0; i < 4; i++)
				if ((a_Profile->Ctrls[a_Key][i] & 0xF000) == 0x1000)
				{
					// Get current button
					CurrentButton = (a_Profile->Ctrls[a_Key][i] & 0x00FF);
				
					// Button pressed?
					if (CurrentButton >= 0 && CurrentButton < 32)
						if (l_JoyButtons[a_Profile->JoyControl] & (1 << CurrentButton))
							return true;
				}
	
	/* Not pressed */
	return false;
}

/* DNetPlayer::NextWeapon() -- Find next weapon */
uint8_t DNetPlayer::NextWeapon(struct player_s* const a_Player, int step)
{
	return 0;
}

/* DNetPlayer::BuildLocalTicCmd() -- Builds local tic command for player */
void DNetPlayer::BuildLocalTicCmd(const bool a_ForBot)
{
#define MAXWEAPONSLOTS 12
	D_ProfileEx_t* Profile;
	player_t* Player;
	int32_t TargetMove;
	size_t i, PID, SID;
	int8_t SensMod, MoveMod, MouseMod, MoveSpeed, TurnSpeed;
	int32_t SideMove, ForwardMove, BaseAT, BaseAM;
	bool IsTurning, GunInSlot, ResetAim;
	int slot, j, l, k;
	weapontype_t newweapon;
	weapontype_t SlotList[MAXWEAPONSLOTS];
	ticcmd_t TicCmd;
	
	/* Build Bot Command? */
	if (a_ForBot)
	{
		// TODO
		return;
	}
	
	/* Clear */
	memset(&TicCmd, 0, sizeof(TicCmd));
	
	/* Obtain profile */
	Profile = p_Profile;
	Player = p_Player;
	
	// No profile?
	if (!Profile)
		return;
	
	/* Find Player ID */
	PID = p_Player - players;
	
	// Illegal player?
	if (PID < 0 || PID >= MAXPLAYERS)
		return;
	
	/* Find Screen ID */
	for (SID = 0; SID < MAXSPLITSCREEN; SID++)
		if (g_PlayerInSplit[SID])
			if (consoleplayer[SID] == PID)
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
	if (GAMEKEYDOWN(Profile, DPEXIC_MOVEMENT))
		MoveMod = 1;
	else
		MoveMod = 0;
	
	// Mouse Modifier
	if (GAMEKEYDOWN(Profile, DPEXIC_LOOKING))
		MouseMod = 2;
	else if (MoveMod)
		MouseMod = 1;
	else 
		MouseMod = 0;
	
	// Moving Speed
	if (GAMEKEYDOWN(Profile, DPEXIC_SPEED))
		MoveSpeed = 1;
	else
		MoveSpeed = 0;
	
	// Turn Speed
	if ((Profile->Flags & DPEXF_SLOWTURNING) &&
			gametic < (p_TurnHeld + Profile->SlowTurnTime))
		TurnSpeed = 2;
	else if (MoveSpeed)
		TurnSpeed = 1;
	else
		TurnSpeed = 0;
	
	/* Player has joystick input? */
	if (Profile->Flags & DPEXF_GOTJOY)
	{
		// Read input for all axis
		for (i = 0; i < MAXJOYAXIS; i++)
		{
			// Modify with sensitivity
			TargetMove = ((float)l_JoyAxis[Profile->JoyControl][i]) * (((float)Profile->JoySens[SensMod]) / 100.0);
			
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
	if (GAMEKEYDOWN(Profile, DPEXIC_ATTACK))
		TicCmd.buttons |= BT_ATTACK;
	
	// Use
	if (GAMEKEYDOWN(Profile, DPEXIC_USE))
		TicCmd.buttons |= BT_USE;
	
	// Jump
	if (GAMEKEYDOWN(Profile, DPEXIC_JUMP))
		TicCmd.buttons |= BT_JUMP;
	
	// Keyboard Turning
	if (GAMEKEYDOWN(Profile, DPEXIC_TURNLEFT))
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
	if (GAMEKEYDOWN(Profile, DPEXIC_TURNRIGHT))
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
	if (GAMEKEYDOWN(Profile, DPEXIC_STRAFELEFT))
		SideMove -= c_sidemove[MoveSpeed];
	if (GAMEKEYDOWN(Profile, DPEXIC_STRAFERIGHT))
		SideMove += c_sidemove[MoveSpeed];
	if (GAMEKEYDOWN(Profile, DPEXIC_FORWARDS))
		ForwardMove += c_forwardmove[MoveSpeed];
	if (GAMEKEYDOWN(Profile, DPEXIC_BACKWARDS))
		ForwardMove -= c_forwardmove[MoveSpeed];
		
	// Looking
	if (GAMEKEYDOWN(Profile, DPEXIC_LOOKCENTER))
		ResetAim = true;
		//localaiming[SID] = 0;
	else
	{
		if (GAMEKEYDOWN(Profile, DPEXIC_LOOKUP))
			BaseAM += Profile->LookUpDownSpeed >> 16;
			//localaiming[SID] += Profile->LookUpDownSpeed;
		
		if (GAMEKEYDOWN(Profile, DPEXIC_LOOKDOWN))
			BaseAM -= Profile->LookUpDownSpeed >> 16;
			//localaiming[SID] -= Profile->LookUpDownSpeed;
	}
	
	// Weapons
		// Next
	if (GAMEKEYDOWN(Profile, DPEXIC_NEXTWEAPON))
	{
		// Set switch
		TicCmd.buttons |= BT_CHANGE;
		TicCmd.XNewWeapon = NextWeapon(Player, 1);
	}
		// Prev
	else if (GAMEKEYDOWN(Profile, DPEXIC_PREVWEAPON))
	{
		// Set switch
		TicCmd.buttons |= BT_CHANGE;
		TicCmd.XNewWeapon = NextWeapon(Player, -1);
	}
		// Best Gun
	else if (GAMEKEYDOWN(Profile, DPEXIC_BESTWEAPON))
	{
		newweapon = P_PlayerBestWeapon(Player, true);
		
		if (newweapon != Player->readyweapon)
		{
			TicCmd.buttons |= BT_CHANGE;
			TicCmd.XNewWeapon = newweapon;
		}
	}
		// Worst Gun
	else if (GAMEKEYDOWN(Profile, DPEXIC_WORSTWEAPON))
	{
		newweapon = P_PlayerBestWeapon(Player, false);
		
		if (newweapon != Player->readyweapon)
		{
			TicCmd.buttons |= BT_CHANGE;
			TicCmd.XNewWeapon = newweapon;
		}
	}
		// Slots
	else
	{
		// Which slot?
		slot = -1;
		
		// Look for keys
		for (i = DPEXIC_SLOT1; i <= DPEXIC_SLOT10; i++)
			if (GAMEKEYDOWN(Profile, i))
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
				TicCmd.buttons |= BT_CHANGE;
				TicCmd.XNewWeapon = newweapon;
			}
		}
	}
	
	// Inventory
	if (GAMEKEYDOWN(Profile, DPEXIC_NEXTINVENTORY))
		TicCmd.InventoryBits = TICCMD_INVRIGHT;
	else if (GAMEKEYDOWN(Profile, DPEXIC_PREVINVENTORY))
		TicCmd.InventoryBits = TICCMD_INVLEFT;
	else if (GAMEKEYDOWN(Profile, DPEXIC_USEINVENTORY))
		TicCmd.InventoryBits = TICCMD_INVUSE;
	
	/* Handle special functions */
	// Coop Spy
	if (GAMEKEYDOWN(Profile, DPEXIC_COOPSPY))
	{
		// Only every half second
		if (gametic > (p_CoopSpyTime + (TICRATE >> 1)))
		{
			do
			{
				displayplayer[SID] = (displayplayer[SID] + 1) % MAXPLAYERS;
			} while (!playeringame[displayplayer[SID]] || !P_PlayerOnSameTeam(&players[consoleplayer[SID]], &players[displayplayer[SID]]));
			
			// Print Message
			CONL_PrintF("%sYou are now watching %s.\n",
					(SID == 3 ? "\x6" : (SID == 2 ? "\x5" : (SID == 1 ? "\x4" : ""))),
					(displayplayer[SID] == consoleplayer[SID] ? "Yourself" : D_NCSGetPlayerName(displayplayer[SID]))
				);
			
			// Reset timeout
			p_CoopSpyTime = gametic + (TICRATE >> 1);
		}
	}
	
	// Key is unpressed to reduce time
	else
		Profile->CoopSpyTime = 0;
	
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
	TicCmd.sidemove = SideMove;
	TicCmd.forwardmove = ForwardMove;
	
	/* Slow turning? */
	if (!IsTurning)
		p_TurnHeld = gametic;
	
	/* Turning */
	TicCmd.BaseAngleTurn = BaseAT;
	TicCmd.BaseAiming = BaseAM;
	TicCmd.ResetAim = ResetAim;
	
	/* Push Command to local Q */
	if (p_LocalSpot[DNetController::p_ServerLag] < MAXLOCALTICS)
		memmove(&p_LocalTicCmdQ[DNetController::p_ServerLag][p_LocalSpot[DNetController::p_ServerLag]++], &TicCmd, sizeof(TicCmd));
#undef MAXWEAPONSLOTS
}

/* DNetPlayer::PopTicCmd() -- Pops the player's last tic command */
ticcmd_t* DNetPlayer::PopTicCmd(void)
{
	ticcmd_t* Ref;
	
	/* No Commands */
	if (!p_NumTicCmdQ)
		return NULL;
	
	/* Use first command */
	Ref = p_TicCmdQ[0];
	
	/* Move all commands down */
	memmove(&p_TicCmdQ[0], &p_TicCmdQ[1], sizeof(p_TicCmdQ[0]) * (p_NumTicCmdQ - 1));
	p_TicCmdQ[p_NumTicCmdQ - 1] = NULL;
	
	/* Return the command */
	return Ref;
}

/* DNetPlayer::NetPlayerByCode() -- Find net player with this code */
DNetPlayer* DNetPlayer::NetPlayerByCode(const uint32_t a_Code)
{
	size_t i;
	
	/* Find spot we are at in the player list */
	for (i = 0; i < p_NumPlayers; i++)
		if (p_Players[i])
			if (p_Players[i]->p_Code == a_Code)
				return p_Players[i];
	
	/* Not found */
	return NULL;
}

/* DNetPlayer::NetPlayerByPID() -- Get net player by PID */
DNetPlayer* DNetPlayer::NetPlayerByPID(const uint32_t a_PID)
{
	size_t i;
	
	/* Find spot we are at in the player list */
	for (i = 0; i < p_NumPlayers; i++)
		if (p_Players[i])
			if (p_Players[i]->p_PID == a_PID)
				return p_Players[i];
	
	/* Not found */
	return NULL;
}

/* DNetController::DNetController() -- Creates network controller */
DNetController::DNetController()
{
	/* Create multi-cast? */
	if (!p_MulCast)
		p_MulCast = new RBMultiCastStream_c();
}

/* DNetController::DNetController() -- Constructor */
DNetController::DNetController(RBStream_c* const a_STDStream)
{
	p_Master = true;
	
	/* Create multi-cast? */
	if (!p_MulCast)
		p_MulCast = new RBMultiCastStream_c();
	
	/* Set standard streams to this */
	p_STDStreams[0] = a_STDStream;
	p_STDStreams[1] = a_STDStream;
	
	/* Create perfect stream to wrap the standard stream */
	p_PStreams[0] = new RBPerfectStream_c(a_STDStream);
	p_PStreams[1] = p_PStreams[0];
}

/* DNetController::~DNetController() -- Destroys network controller */
DNetController::~DNetController()
{
	size_t i;
	
	/* Remove from multicast */
	p_MulCast->DelMultiCast(GetPerfectWrite(), &p_Address);
	
	/* Delete streams if this is a master controller */
	if (p_Master)
	{
		delete p_PStreams[0];	// Perfect stream needs to be removed first!
		delete p_STDStreams[0];
	}
}

/* DNetController::GetRead() -- Get read stream */
RBStream_c* DNetController::GetRead(void)
{
	return p_STDStreams[0];
}

/* DNetController::GetWrite() -- Get write stream */
RBStream_c* DNetController::GetWrite(void)
{
	return p_STDStreams[1];
}

/* DNetController::GetPerfectRead() -- Get perfect read stream */
RBPerfectStream_c* DNetController::GetPerfectRead(void)
{
	return p_PStreams[0];
}

/* DNetController::GetPerfectWrite() -- Get perfect write stream */
RBPerfectStream_c* DNetController::GetPerfectWrite(void)
{
	return p_PStreams[1];
}

/* DNetController::ArbCount() -- Counts filled arbitrations */
size_t DNetController::ArbCount(const bool a_OnlyPlayers)
{
	size_t Count, i;
	
	/* Go through all arbs */
	Count = 0;
	for (i = 0; i < p_NumArbs; i++)
		if (p_Arbs[i])
			if (!a_OnlyPlayers)
				Count++;
			else
				if (!p_Arbs[i]->IsBot())
					Count++;
	
	/* Return count */
	return Count;
}

/* DNetController::AddArb() -- Add arbitration to new player */
void DNetController::AddArb(DNetPlayer* const a_NetPlayer)
{
	size_t Count, i;
	
	/* Go through all arbs */
	// Use first free slot (if any)
	for (i = 0; i < p_NumArbs; i++)
		if (!p_Arbs[i])
		{
			p_Arbs[i] = a_NetPlayer;
			break;
		}
	
	// No Room
	if (i >= p_NumArbs)
	{
		Z_ResizeArray((void**)&p_Arbs, sizeof(*p_Arbs), p_NumArbs, p_NumArbs + 1);
		i = p_NumArbs++;
		p_Arbs[i] = a_NetPlayer;
	}
	
	/* Modify local/remote count */
	if (p_IsLocal)
		p_LocalCount++;
	else
		p_RemoteCount++;
}

/* DNetController::IsLocal() -- Returns true if connection is local */
bool DNetController::IsLocal(void)
{
	return p_IsLocal;
}

/* DNetController::IsServer() -- Returns true if connection is server */
bool DNetController::IsServer(void)
{
	return p_IsServer;
}

/* DNetController::GetAddress() -- Returns address of this connection */
RBAddress_c& DNetController::GetAddress(void)
{
	return p_Address;
}

/* DNetController::GetServer() -- Gets the current server */
DNetController* DNetController::GetServer(void)
{
	size_t i;
	
	/* Find server in chain */
	for (i = 0; i < l_NumServerNCS; i++)
		if (l_ServerNCS[i])
			if (l_ServerNCS[i]->p_IsServer)
				return l_ServerNCS[i];
	
	/* Not Found */
	return NULL;
}

/* DNetController::GetByAddress() -- Find host by address */
DNetController* DNetController::GetByAddress(RBAddress_c* const a_Address)
{
	size_t i;
	
	/* Check */
	if (!a_Address)
		return NULL;
	
	/* Find address in chain */
	for (i = 0; i < l_NumServerNCS; i++)
		if (l_ServerNCS[i])
			if (l_ServerNCS[i]->p_Address.CompareAddress(*a_Address))
				return l_ServerNCS[i];
	
	/* Not Found */
	return NULL;
}

/* DNetController::GetMultiCast() -- Returns multi-cast */
RBMultiCastStream_c* DNetController::GetMultiCast(void)
{
	return p_MulCast;
}

/* DNetController::Disconnect() -- Disconnects from server */
void DNetController::Disconnect(void)
{
	size_t i;
	
	/* Delete all controllers */
	for (i = 0; i < l_NumServerNCS; i++)
		if (l_ServerNCS[i])
		{
			// Remove from multicast
			p_MulCast->DelMultiCast(l_ServerNCS[i]->GetPerfectWrite(), &l_ServerNCS[i]->p_Address);
			
			delete l_ServerNCS[i];
			l_ServerNCS[i] = NULL;
		}
	
	/* Create initial controllers */
	// Allocate the first two, if needed
	if (l_NumServerNCS < 2)
	{
		Z_ResizeArray((void**)&l_ServerNCS, sizeof(*l_ServerNCS), 0, 2);
		l_NumServerNCS = 2;
	}
	
	// Create local loopback controller
	l_ServerNCS[0] = new DNetController(new RBLoopBackStream_c());
	l_ServerNCS[0]->p_IsLocal = true;
	l_ServerNCS[0]->p_Address.ChooseLocal();
	p_MulCast->AddMultiCast(l_ServerNCS[0]->GetPerfectWrite(), &l_ServerNCS[0]->p_Address);
	
	// Create Internet controller
	
	/* Reset Quick Determs */
	// Count of local/remote players (for timing code)
	p_LocalCount = 0;
	p_RemoteCount = 0;
	
	// Host of the game
	p_IsGameHost = false;
	
	// Server Tics
	p_LastTime = 0;
}

/* DNetController::StartServer() -- Starts a server */
void DNetController::StartServer(void)
{
	size_t i;
	
	/* First Disconnect */
	DNetController::Disconnect();
	
	/* Make local connections servers */
	for (i = 0; i < l_NumServerNCS; i++)
		if (l_ServerNCS[i])
			if (l_ServerNCS[i]->p_IsLocal)
			{
				l_ServerNCS[i]->p_IsServer = true;
				l_ServerNCS[i]->p_SaveSent = true;
			}
	
	/* Set as server */
	p_IsGameHost = true;
	
	/* Change game state to waiting mode */
	p_ReadyTime = 0;
	p_LastTime = I_GetTime();					// For no tics after server start speedup
	gamestate = wipegamestate = GS_WAITINGPLAYERS;
	S_ChangeMusicName("D_WAITIN", 1);			// Waiting for game to start
	
	// Set lag to zero
	p_ServerLag = 5;//10;		// ~300ms ping
}

/* DNetController::ReadyTics() -- Amount of tics ready to be played */
// i.e. the amount that statifies NetReadTicCmds for all players
tic_t DNetController::ReadyTics(void)
{
	static tic_t LastTime;
	tic_t ThisTime;
	tic_t DiffTime;
	uint64_t ThisTimeMS;
	static uint64_t LastTimeMS;
	size_t i;
	bool EveryoneIsReady;
	DNetPlayer* NetPlayer;
	
	/* Get the current time */
	ThisTime = I_GetTime();
	ThisTimeMS = I_GetTimeMS();
	
	/* Clear */
	p_Readies = 0;
	
	/* An empty server (no players) */
	// There's no need to waste a bunch of CPU cycles here. Also, say someone
	// loads up nuts or scythe map26(?) on your server, causes massive infighting
	// and then just leaves, don't want to lose that CPU.
	if (p_RemoteCount <= 0 && p_LocalCount <= 0)
	{
#define SERVERCOOLDOWN 4
		if (ThisTime >> SERVERCOOLDOWN > p_LastTime >> SERVERCOOLDOWN)
		{
			DiffTime = (ThisTime - p_LastTime) >> SERVERCOOLDOWN;
			p_LastTime = ThisTime;
			return DiffTime;
		}
		
		return 0;
#undef SERVERCOOLDOWN
	}
	
	/* Players Inside (and we are the host) */
	else if (p_IsGameHost)
	{
		// Still enough time for next tic? (or server is laggy and still behind lag time)
		if ((ThisTimeMS) < (LastTimeMS + (TICSPERMS >> 1)))
			return 0;
		
		// Set as ready (will be unset in the future)
		EveryoneIsReady = true;
		
		// Be sure everyone has at least one local tic command before
		// moving on.
		for (i = 0; i < DNetPlayer::p_NumPlayers; i++)
		{
			// Nothing here?
			if (!DNetPlayer::p_Players[i])
				continue;
			
			// Get player
			NetPlayer = DNetPlayer::p_Players[i];
			
			// Local tics not available?
			if (!NetPlayer->p_LocalSpot[0])
				EveryoneIsReady = false;
		}
		
		// Set last time
		LastTime = ThisTime;
		LastTimeMS = ThisTimeMS;
		
		// Proceed if everyone is ready
		if (EveryoneIsReady)
			return (p_Readies = 1);
		else
			return (p_Readies = 0);
	}
	
	/* We are the guest to a networked game */
	// We must wait on the server to send us tic commands to pass the time with
	else
	{
		return 0;
	}
	
#if 0
	/* At least 1 remote player */
	if (p_RemoteCount > 0)
	{
		// Wait on other players, TODO
		if (p_IsGameHost)
		{
			return 0;
		}
		
		// Waiting on server
		else
		{
			return 0;
		}
	}
	
	/* Only Local Players (Solo, Split) */
	else if (p_LocalCount > 0)
	{
		// Ignoring tics (due to wipe)
		if (g_IgnoreWipeTics)
		{
			p_LastTime = ThisTime;
			g_IgnoreWipeTics = 0;
			return 0;
		}
		
		// New Tic?
		else if (ThisTime > p_LastTime)
		{
			DiffTime = ThisTime - p_LastTime;
			p_LastTime = ThisTime;
			p_Readies = DiffTime;
			return DiffTime;
		}
		
		// No lost time or need to update
		else
			return 0;
	}
	
	/* An empty server (no players) */
	// There's no need to waste a bunch of CPU cycles here. Also, say someone
	// loads up nuts or scythe map26(?) on your server, causes massive infighting
	// and then just leaves, don't want to lose that CPU.
	else
	{
#define SERVERCOOLDOWN 4
		if (ThisTime >> SERVERCOOLDOWN > p_LastTime >> SERVERCOOLDOWN)
		{
			DiffTime = (ThisTime - p_LastTime) >> SERVERCOOLDOWN;
			p_LastTime = ThisTime;
			return DiffTime;
		}
		return 0;
#undef SERVERCOOLDOWN
	}
#endif
}

bool D_EXHC_EROR(struct D_NCMessageData_s* const a_Data);
bool D_EXHC_LPRJ(struct D_NCMessageData_s* const a_Data);
bool D_EXHC_PJOK(struct D_NCMessageData_s* const a_Data);
bool D_EXHC_MAPR(struct D_NCMessageData_s* const a_Data);
bool D_EXHC_MAPC(struct D_NCMessageData_s* const a_Data);

// c_NCMessageCodes -- Local messages
static const D_NCMessageType_t c_NCMessageCodesEx[] =
{
	{1, "EROR", D_EXHC_EROR, (D_NCMessageFlag_t)(DNCMF_PERFECT | DNCMF_NORMAL | DNCMF_SERVER)},
	{1, "LPRJ", D_EXHC_LPRJ, (D_NCMessageFlag_t)(DNCMF_PERFECT | DNCMF_CLIENT | DNCMF_SERVER | DNCMF_HOST | DNCMF_REMOTECL)},
	{1, "PJOK", D_EXHC_PJOK, (D_NCMessageFlag_t)(DNCMF_PERFECT | DNCMF_SERVER | DNCMF_DEMO)},
	{1, "MAPR", D_EXHC_MAPR, (D_NCMessageFlag_t)(DNCMF_PERFECT | DNCMF_SERVER | DNCMF_HOST | DNCMF_REMOTECL)},
	{1, "MAPC", D_EXHC_MAPC, (D_NCMessageFlag_t)(DNCMF_PERFECT | DNCMF_SERVER | DNCMF_DEMO)},
	
	// EOL
	{0, NULL},
};

/* DNetController::NetUpdate() -- Updates network code */
void DNetController::NetUpdate(void)
{
	RBAddress_c ReadAddr;
	int32_t i, j, k, rr, tN, Nn;
	char Header[5];
	int32_t SID;
	
	RBPerfectStream_c* PIn;
	RBPerfectStream_c* POut;
	RBStream_c* BOut;
	
	bool IsServ, IsClient, IsHost, IsPerf;
	
	DNetController* DNC;
	DNetController* RC;
	D_NCMessageData_t Data;
	
	DNetPlayer* NetPlay;
	tic_t CurTics, CurHalfTic;
	static tic_t LastLocalBuild;
	uint64_t CurMS;
	ticcmd_t* Top;
	DNetController* ServerNC;
	
	/* Get Current Time */
	CurTics = I_GetTime();
	CurHalfTic = I_GetTimeHalf();
	CurMS = I_GetTimeMS();
	
	/* Build tic commands for local players */
	// Build local commands for everyone
	for (i = 0; i < MAXPLAYERS; i++)
	{
		// Find network player
		NetPlay = DNetPlayer::NetPlayerByPID(i);
	
		// Not found?
		if (!NetPlay)
			continue;
		
		// No tics exist before the lag time?
		if ((gametic - NetPlay->p_PlayerTime) < p_ServerLag)
		{
			// Set tic here
			NetPlay->p_LocalSpot[0] = 1;
			
			// Clear
			memset(&NetPlay->p_LocalTicCmdQ[0][0], 0, sizeof(ticcmd_t));
			
			// Set angle to something (Absolute angles, so the player doesn't
			// immedietly face east).
			if (P_EXGSGetValue(PEXGSBID_COABSOLUTEANGLE))
				NetPlay->p_LocalTicCmdQ[0][0].angleturn = NetPlay->p_Player->mo->angle >> 16;
		}
		
		// If Local, build commands
		if (NetPlay->IsLocal())
		{
			// Bot (Run bot tic command generator)
			if (NetPlay->IsBot())
			{
				// Bot tics are only done once every 4 gametics
				if ((CurTics >> 2) > NetPlay->GetBotLastTime())
				{
					// Build Commands
					NetPlay->BuildLocalTicCmd(true);
				
					// Set the time
					NetPlay->GetBotLastTime() = (CurTics >> 2);
				}
			
				// Use the previously used tic commands to control the bot
				else
				{
				}
			}
		
			// Normal player (build from input)
			else
			{
				NetPlay->BuildLocalTicCmd(false);
			}
		}
	
		// Remote
		else
		{
		}
	}
	
	/* Read/Write Packets from all controllers */
	for (i = 0; i < l_NumServerNCS; i++)
	{
		// No Controller Here?
		if (!l_ServerNCS[i])
			continue;
		
		// Get
		DNC = l_ServerNCS[i];
		
		// Get streams for the client
		PIn = DNC->GetPerfectRead();
		POut = DNC->GetPerfectWrite();
		BOut = DNC->GetWrite();
		
		// Constantly read perfect blocks
		while (PIn && PIn->BlockPlay(&ReadAddr))
		{
			// Copy header
			memset(Header, 0, sizeof(Header));
			PIn->HeaderCopy(Header);
			
			// Debug
			if (devparm)
				CONL_PrintF("Read \"%s\"\n", Header);
			
			// Find the remote client that initiated this message
			// If they aren't in the client chain then this returns NULL.
			// If it does return NULL then that means they were never connected
			// in the first place.
			RC = DNetController::GetByAddress(&ReadAddr);
			
			// Determine where this packet came from for flag checking
				// Is Perfect Packet
			IsPerf = PIn->IsPerfect();
				// From Server
			IsServ = DNC->IsServer();
				// From Client
			IsClient = !IsServ;
				// We are the host
			IsHost = (DNC->IsServer() && DNC->IsLocal());
			
			// Go through head table
			for (tN = 0; c_NCMessageCodesEx[tN].Valid; tN++)
			{
				// Compare Header
				if (!RBStream_c::CompareHeader(Header, c_NCMessageCodesEx[tN].Header))
					continue;
				
				// Perfect but not set?
				if (IsPerf && !(c_NCMessageCodesEx[tN].Flags & DNCMF_PERFECT))
					continue;
				
				// Normal but not set?
				if (!IsPerf && !(c_NCMessageCodesEx[tN].Flags & DNCMF_NORMAL))
					continue;
				
				// From client but not accepted from client
				if (IsClient && !(c_NCMessageCodesEx[tN].Flags & DNCMF_CLIENT))
					continue;
				
				// From server but not accepted from server
				if (IsServ && !(c_NCMessageCodesEx[tN].Flags & DNCMF_SERVER))
					continue;
				
				// From somewhere but we are not the host of the game
				if (!IsHost && !(c_NCMessageCodesEx[tN].Flags & DNCMF_HOST))
					continue;
				
				// Requires a remote client exist (a connected player)
				if ((c_NCMessageCodesEx[tN].Flags & DNCMF_REMOTECL) && !RC)
					continue;
				
				// Clear data
				memset(&Data, 0, sizeof(Data));
				
				// Fill Data
				Data.Type = &c_NCMessageCodesEx[tN];
				
				Data.PIn = PIn;
				Data.POut = POut;
				//Data.BIn = BIn;
				Data.BOut = BOut;
				Data.IsServ = IsServ;
				Data.IsClient = IsClient;
				Data.IsHost = IsHost;
				Data.IsPerf = IsPerf;
				Data.DNC = DNC;
				Data.RC = RC;
				Data.ReadAddr = &ReadAddr;
				
				// Find node number for remote client
				Data.NodeNum = (uint8_t)-1;
				if (Data.RC)
					for (Nn = 0; Nn < l_NumServerNCS; Nn++)
						if (l_ServerNCS[Nn])
							if (l_ServerNCS[Nn] == RC)
							{
								Data.NodeNum = Nn;
								break;
							}
				
				// Call handler
				if (c_NCMessageCodesEx[tN].Func(&Data))
					break;
			}
			
			// Clear header for next read run
			memset(Header, 0, sizeof(Header));
		}
		
		// Flush output streams
		POut->BlockFlush();
		BOut->BlockFlush();
	}
	
	//CONL_PrintF("{%cReadies %lli %lli\n", ((p_Readies > 1) ? '7' : ((p_Readies == 1) ? '3' : 'a')), p_Readies, gametic);
	
	/* If playing as the server */
	// Merge the local commands of everyone
	ServerNC = GetServer();
	if (ServerNC && p_Readies > 0 && ServerNC->IsLocal())
	{
		// Merge for all players
		for (i = 0; i < MAXPLAYERS; i++)
		{
			// Find network player
			NetPlay = DNetPlayer::NetPlayerByPID(i);
	
			// Not found?
			if (!NetPlay)
				continue;
		
			// See if player is on this screen
			SID = -1;
			for (j = 0; j < g_SplitScreen + 1; j++)
				if (g_PlayerInSplit[j])
					if (i == consoleplayer[j])
					{
						SID = j;
						break;
					}
			
			// Ready all tics
			for (rr = 0; rr < p_Readies; rr++)
			{
				// Allocation
				if (NetPlay->p_NumTicCmdQ < p_Readies)
				{
					Z_ResizeArray((void**)&NetPlay->p_TicCmdQ, sizeof(*NetPlay->p_TicCmdQ), NetPlay->p_NumTicCmdQ, p_Readies);
					NetPlay->p_NumTicCmdQ = p_Readies;
				}
		
				// Move last tic here and merge
				NetPlay->p_TicCmdQ[rr] = new ticcmd_t();
			
				// No tics in Queue?
				if (!NetPlay->p_LocalSpot[0])
				{
					// Use last command in place
					memmove(NetPlay->p_TicCmdQ[rr], &NetPlay->GetLastCommand(), sizeof(ticcmd_t));
					continue;
				}
			
				// Merge in all tics
				D_NCSNetMergeTics(NetPlay->p_TicCmdQ[rr], NetPlay->p_LocalTicCmdQ[0], NetPlay->p_LocalSpot[0]);
		
				// Clear
				memset(NetPlay->p_LocalTicCmdQ[0], 0, sizeof(NetPlay->p_LocalTicCmdQ[0]));
				NetPlay->p_LocalSpot[0] = 0;
		
				// Set local aiming and such
				if (SID >= 0 && SID < MAXSPLITSCREEN)
				{
					// Absolute Angles
					if (P_EXGSGetValue(PEXGSBID_COABSOLUTEANGLE))
					{
						localangle[SID] += NetPlay->p_TicCmdQ[rr]->BaseAngleTurn << 16;
						NetPlay->p_TicCmdQ[rr]->angleturn = localangle[SID] >> 16;
					}
			
					// Doom Angles
					else
						NetPlay->p_TicCmdQ[rr]->angleturn = NetPlay->p_TicCmdQ[rr]->BaseAngleTurn;
				
					if (NetPlay->p_TicCmdQ[rr]->ResetAim)
						localaiming[SID] = 0;
					else
						localaiming[SID] += NetPlay->p_TicCmdQ[rr]->BaseAiming << 16;
					G_ClipAimingPitch(&localaiming[SID]);
				}
			
				// Set last command
				memmove(&NetPlay->GetLastCommand(), NetPlay->p_TicCmdQ[rr], sizeof(ticcmd_t));
				
				// Move player buffers down
				memmove(&NetPlay->p_LocalTicCmdQ[0], &NetPlay->p_LocalTicCmdQ[1], sizeof(NetPlay->p_LocalTicCmdQ[0]) * (DNCMAXINPUTLAG - 1));
				memmove(&NetPlay->p_LocalSpot[0], &NetPlay->p_LocalSpot[1], sizeof(NetPlay->p_LocalSpot[0]) * (DNCMAXINPUTLAG - 1));
				
				memset(&NetPlay->p_LocalTicCmdQ[DNCMAXINPUTLAG - 1], 0, sizeof(NetPlay->p_LocalTicCmdQ[DNCMAXINPUTLAG - 1]));
				memset(&NetPlay->p_LocalSpot[DNCMAXINPUTLAG - 1], 0, sizeof(NetPlay->p_LocalSpot[DNCMAXINPUTLAG - 1]));
				
#if 0
				CONL_PrintF(">> ");
				for (tN = 0; tN < DNCMAXINPUTLAG; tN++)
					CONL_PrintF("%i ", (int)NetPlay->p_LocalSpot[tN]);
				CONL_PrintF("<<\n");
#endif
			}
		}
		
		// Reset ready count
		p_Readies = 0;
		
		// Send Everyone the server's allocated tics
	}
}

/* DNetController::ReadTicCmd() -- Reads queued commands for specific player */
void DNetController::ReadTicCmd(ticcmd_t* const a_Cmd, const int32_t a_PlayerNum)
{
	DNetPlayer* NP;
	ticcmd_t* Top;
	
	/* Check */
	if (!a_Cmd || a_PlayerNum < 0 || a_PlayerNum >= MAXPLAYERS)
		return;
	
	/* Get net player */
	NP = DNetPlayer::NetPlayerByPID(a_PlayerNum);
	
	/* Read the top command and copy it */
	Top = NP->PopTicCmd();
	
	// Copy
	memset(a_Cmd, 0, sizeof(*a_Cmd));
	if (Top)
		memmove(a_Cmd, Top, sizeof(ticcmd_t));
	
	// Delete
	delete Top;
}

/* DNetController::ExecutePreCmds() -- Execute Pre Commands */
void DNetController::ExecutePreCmds(void)
{
	/* Demo is playing */
	if (demoplayback)
		G_DemoPreGTicker();		// Pushes commands to ReMooD format
	
	/* Write Commands to demo (if recording) */
	if (demorecording)
		;	// TODO
	
	/* Execute Commands */
	// TODO
	
	/* Delete Commands */
	// TODO
}

/* DNetController::ExecutePostCmds() -- Execute Post Commands */
void DNetController::ExecutePostCmds(void)
{
	/* Demo is playing */
	if (demoplayback)
		G_DemoPostGTicker();		// Pushes commands to ReMooD format
	
	/* Write Commands to demo (if recording) */
	if (demorecording)
		;	// TODO
	
	/* Execute Commands */
	// TODO
	
	/* Delete Commands */
	// TODO
}

/* DNetController::PushCommand() -- Adds commands to queue */
void DNetController::PushCommand(DNetCommand* const a_Cmd)
{
	int32_t Spot;
	
	/* Check */
	if (!a_Cmd)
		return;
	
	/* First first last spot in Q */
	for (Spot = ((int32_t)p_NumCommandQ) - 1; Spot >= 0; Spot--)
		if (!p_CommandQ[Spot])
			break;
	
	// No spot found?
	if (Spot <= 0)
	{
		Z_ResizeArray((void**)&p_CommandQ, sizeof(*p_CommandQ), p_NumCommandQ, p_NumCommandQ + 1);
		Spot = p_NumCommandQ++;
	}
	
	/* Place In This spot */
	p_CommandQ[Spot] = new DNetCommand();
	memmove(p_CommandQ[Spot], a_Cmd, sizeof(*a_Cmd)); 
}

/*** HANDLERS ***/

/* D_EXHC_EROR() -- Error Occured */
bool D_EXHC_EROR(struct D_NCMessageData_s* const a_Data)
{
	uint8_t Code;
	
	/* Read Code */
	Code = a_Data->PIn->ReadUInt8();
	
	/* Bad code? */
	if (Code < 0 || Code >= NUMDNETERRORNUM)
		return true;
	
	/* Print to screen */
	CONL_PrintF("{1Error %i: ", Code);
	CONL_OutputU((UnicodeStringID_t)(DSTR_DNEN_SUCCESS + Code), "");
	CONL_PrintF("{z\n");
	
	/* Stop handling */
	return true;
}

/* D_EXHC_LPRJ() -- Player wants to join */
bool D_EXHC_LPRJ(struct D_NCMessageData_s* const a_Data)
{
	RBMultiCastStream_c* MC;
	
	bool FailJoin;
	int32_t i, j, CurPlayerCount, ArbCount, NewCode;
	uint8_t IsBot, MaxSplit, PlayID; 
	uint8_t* SplitList;
	int8_t NextSplit, CurSplits;
	char UUID[MAXPLAYERNAME * 2];
	char AccountName[MAXPLAYERNAME], DisplayName[MAXPLAYERNAME];
	DNetErrorNum_e FailCode;
	
	/* Read Data */
	IsBot = a_Data->PIn->ReadUInt8();
	a_Data->PIn->ReadString(UUID, MAXPLAYERNAME * 2);
	a_Data->PIn->ReadString(AccountName, MAXPLAYERNAME);
	a_Data->PIn->ReadString(DisplayName, MAXPLAYERNAME);
	CurSplits = a_Data->PIn->ReadInt8();
	MaxSplit = a_Data->PIn->ReadUInt8();
	SplitList = new uint8_t[MaxSplit];
	for (i = 0; i < MaxSplit; i++)
		SplitList[i] = a_Data->PIn->ReadUInt8();
	NextSplit = a_Data->PIn->ReadInt8();
	
	/* Init */
	FailJoin = false;
	FailCode = DNEN_SUCCESS;
	
	/* A demo is playing? */
	if (demoplayback)
	{
		FailJoin = true;
		FailCode = DNEN_DEMOPLAYBACK;
	}
	
	/* Check Global Limits */
	CurPlayerCount = D_CNetPlayerCount();
	
	// Too many inside?
	if (!FailJoin && (CurPlayerCount >= MAXPLAYERS || CurPlayerCount >= l_SVMaxPlayers.Value->Int))
	{
		FailJoin = true;
		FailCode = DNEN_MAXPLAYERSLIMIT;
	}
	
	/* Bot Check */
	if (!FailJoin && IsBot)
	{
		// Not the server?
		if (!FailJoin && !a_Data->RC->IsLocal())
		{
			FailJoin = true;
			FailCode = DNEN_BOTSSERVERONLY;
		}
	}
	
	/* Standard Player Check */
	else if (!FailJoin)
	{
		// Get arb count
		ArbCount = a_Data->RC->ArbCount(true);
		
		// Too many players per split?
		if (!FailJoin && (ArbCount >= l_SVMaxSplitScreen.Value->Int || ArbCount >= MAXSPLITSCREEN || ArbCount >= MaxSplit))
		{
			FailJoin = true;
			FailCode = DNEN_MAXSPLITLIMIT;
		}
	}
	
	/* If failed to join, inform the joiner */
	if (FailJoin)
	{
		D_CRepSendError(a_Data->POut, a_Data->ReadAddr, FailCode);
		return true;	// Stop handling
	}
	
	/* Find the first free spot that doesn't have any players */
	PlayID = 0;
	for (i = 0; i < MAXPLAYERS; i++)
		if (!playeringame[i])
		{
			PlayID = i;
			break;
		}
	
	/* Find a free code that can be used */
	do
	{
		// Generate a new code
		NewCode = D_CMakePureRandom();
	} while (DNetPlayer::NetPlayerByCode(NewCode));
	
	if (devparm)
		CONL_PrintF("Add player! [c=%08x b=%i uuid=%s an=%s dn=%s cs=%i ms=%i ns=%i]\n",
				NewCode, IsBot, UUID, AccountName, DisplayName, CurSplits,
				MaxSplit, NextSplit
			);
	
	/* Send to everyone that the join is OK */
	MC = DNetController::GetMultiCast();
	
	// Base
	MC->BlockBase("PJOK");
	
	// Write Data
		// Server Usage
	MC->WriteUInt8(a_Data->NodeNum);			// Node where the player is (server/demo usage!)
	MC->WriteUInt8(IsBot);						// Player is a bot
		// Player Location and Limitations
	MC->WriteLittleUInt32(NewCode);				// Anti-Duplication (prevent duped ident players)
	MC->WriteUInt8(l_SVMaxPlayers.Value->Int);	// Server's MAXPLAYERS (1st Come, 1st Serve)
	MC->WriteUInt8(l_SVMaxSplitScreen.Value->Int);	// Server's MAXSPLITSCREEN (1st Come, 1st Serve)
	MC->WriteUInt8(PlayID);						// players[] location
	MC->WriteInt8(CurSplits);					// Current Splitscreen
	MC->WriteUInt8(MaxSplit);					// Max Splitscreen
	MC->WriteInt8(NextSplit);					// Next split to use
		// Player Identification (Locally)
	MC->WriteString(UUID);						// UUID (oh well so you can track players)
	MC->WriteString(AccountName);				// Account Name
	MC->WriteString(DisplayName);				// Display Name
	
	// Send it to everyone
	MC->BlockRecord();
	
	/* Stop handling */
	return true;
}

/* D_EXHC_PJOK() -- Player Join OK */
bool D_EXHC_PJOK(struct D_NCMessageData_s* const a_Data)
{
	uint8_t NodeNum, IsBot, SVMP, SVMSS, PlayID, MaxSplit, CurTPlayers, CurArbC;
	int8_t CurSplit, NextSplit;
	uint32_t NewCode;
	char UUID[MAXPLAYERNAME * 2];
	char AccountName[MAXPLAYERNAME], DisplayName[MAXPLAYERNAME];
	D_ProfileEx_t* Profile;
	DNetPlayer* NewNetPlayer;
	
	/* Read Packet Data */
	NodeNum = a_Data->PIn->ReadUInt8();
	IsBot = a_Data->PIn->ReadUInt8();
	NewCode = a_Data->PIn->ReadLittleUInt32();
	SVMP = a_Data->PIn->ReadUInt8();
	SVMSS = a_Data->PIn->ReadUInt8();
	PlayID = a_Data->PIn->ReadUInt8();
	CurSplit = a_Data->PIn->ReadInt8();
	MaxSplit = a_Data->PIn->ReadUInt8();
	NextSplit = a_Data->PIn->ReadInt8();
	a_Data->PIn->ReadString(UUID, MAXPLAYERNAME * 2);
	a_Data->PIn->ReadString(AccountName, MAXPLAYERNAME);
	a_Data->PIn->ReadString(DisplayName, MAXPLAYERNAME);
	
	/* Check Bounds */
	// Code already used?
	if (DNetPlayer::NetPlayerByCode(NewCode))
		return true;
	
	// Too many players in game?
	CurTPlayers = D_CNetPlayerCount();
	if (CurTPlayers >= MAXPLAYERS || CurTPlayers >= SVMP)
		return true;
	
	// Too many players in split?
	if (!IsBot)
		if (NextSplit >= SVMSS || NextSplit >= MAXSPLITSCREEN || NextSplit >= MaxSplit)
			return true;
	
	// See if the current slot is empty
	for (; PlayID < MAXPLAYERS; PlayID++)
		if (!playeringame[PlayID])
			break;
	
	// No slots left?
	if (PlayID >= MAXPLAYERS)
		return true;
	
	// Player exceeds ARB count (server only, non-bot)
	if (a_Data->IsHost && !IsBot)
	{
		CurArbC = a_Data->RC->ArbCount(true);
		if (CurArbC >= SVMSS || CurArbC >= MAXSPLITSCREEN || CurArbC >= MaxSplit)
			return true;
	}
	
	/* Create Net Player */
	NewNetPlayer = new DNetPlayer(NewCode, PlayID);
	
	// Bot? (server only, bot)
	if (a_Data->IsHost && IsBot)
		NewNetPlayer->SetBot(true);
	
	// Add arbitration to remote client size
	a_Data->RC->AddArb(NewNetPlayer);
	
	/* Successful join! Add player now */
	playeringame[PlayID] = true;
	G_AddPlayer(PlayID);
	
	/* See if the player is a local one */
	// Check the UUID
	Profile = D_FindProfileEx(UUID);
	
	// It is!
	if (Profile)
	{
		// Add to split screen
		if (g_SplitScreen < 3)
		{
			g_PlayerInSplit[g_SplitScreen + 1] = true;
			consoleplayer[g_SplitScreen + 1] = displayplayer[g_SplitScreen + 1] = PlayID;
			g_SplitScreen++;
			
			// Recalc
			R_ExecuteSetViewSize();
		}
		
		// Set local player's profile
		players[PlayID].ProfileEx = Profile;
		NewNetPlayer->SetProfile(Profile);
		NewNetPlayer->SetLocal(true);
	}
	
	CONL_PrintF("Joined player!\n");
	
	CONL_PrintF("%s %s\n",
			(a_Data->IsServ ? "Server" : "Client"),
			(a_Data->IsHost ? "Host" : "Client")
		);
	
	/* Stop handling */
	return true;
}

/* D_EXHC_MAPR() -- Request Map */
bool D_EXHC_MAPR(struct D_NCMessageData_s* const a_Data)
{
	RBMultiCastStream_c* MC;
	uint8_t SwitchLevel;
	char LumpName[MAXPLIEXFIELDWIDTH];
	P_LevelInfoEx_t* Info;
	
	/* Read Info */
	SwitchLevel = a_Data->PIn->ReadUInt8();
	a_Data->PIn->ReadString(LumpName, MAXPLIEXFIELDWIDTH);
	
	/* Try and locate the level */
	Info = P_FindLevelByNameEx(LumpName, NULL);
	
	// Not found?
	if (!Info)
	{
		D_CRepSendError(a_Data->POut, a_Data->ReadAddr, DNEN_NOTALEVEL);
		return true;
	}
	
	/* Tell everyone to change to this map */
	MC = DNetController::GetMultiCast();
	
	// Base
	MC->BlockBase("MAPC");
	
	// Data
	MC->WriteUInt8(SwitchLevel);
	MC->WriteString(Info->LumpName);
	
	// Send
	MC->BlockRecord();
	
	/* Stop handling */
	return true;
}

/* D_EXHC_MAPC() -- Change Map */
bool D_EXHC_MAPC(struct D_NCMessageData_s* const a_Data)
{
	RBMultiCastStream_c* MC;
	uint8_t SwitchLevel, i;
	char LumpName[MAXPLIEXFIELDWIDTH];
	P_LevelInfoEx_t* Info;
	
	/* Read Info */
	SwitchLevel = a_Data->PIn->ReadUInt8();
	a_Data->PIn->ReadString(LumpName, MAXPLIEXFIELDWIDTH);
	
	/* Try and locate the level */
	Info = P_FindLevelByNameEx(LumpName, NULL);
	
	// Not found?
	if (!Info)
		return true;
	
	/* Switch to this map */
	// Reset players?
	if (!SwitchLevel)
		for (i = 0; i < MAXPLAYERS; i++)
			players[i].playerstate = PST_REBORN;
	
	// Load it
	P_ExLoadLevel(Info, 0);
	
	/* Stop handling */
	return true;
}

/*** FUNCTIONS ***/

/* D_CNetInit() -- Initialize Networking */
void D_CNetInit(void)
{
	/* Initial Disconnect */
	DNetController::Disconnect();
}

/* D_CNetPlayerCount() -- Return current player count */
uint32_t D_CNetPlayerCount(void)
{
	uint32_t i, Count;
	
	/* Determine the total */
	for (Count = 0, i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
			Count++;
	
	/* Return count */
	return Count;
}

/* D_CRepSendError() -- Sends an error response */
void D_CRepSendError(RBStream_c* const a_Stream, RBAddress_c* const a_Address, const DNetErrorNum_e a_Num)
{
	/* Check */
	if (!a_Stream || !a_Address || a_Num < DNEN_SUCCESS || a_Num >= NUMDNETERRORNUM)
		return;
	
	/* Write Header */
	a_Stream->BlockBase("EROR");
	a_Stream->WriteUInt8(a_Num);
	a_Stream->BlockRecord(a_Address);
}

/* D_CReqLocalPlayer() -- Requests the server add a local player */
void D_CReqLocalPlayer(D_ProfileEx_t* const a_Profile, const bool a_Bot)
{
	DNetController* Server;
	RBPerfectStream_c* WriteTo;
	int i, j;
	
	/* Check */
	if (!a_Profile)
		return;
	
	/* Find Server */
	Server = DNetController::GetServer();
	
	// Not found?
	if (!Server)
		return;
	
	/* Non-Local and wants Bot? */
	if (!Server->IsLocal() && a_Bot)
		return;
	
	/* Get stream to write to */
	WriteTo = Server->GetPerfectWrite();
	
	// Not around?
	if (!WriteTo)
		return;
	
	/* Fill Block Info */
	// Base
	WriteTo->BlockBase("LPRJ");
	
	// Data
	WriteTo->WriteUInt8(a_Bot);
	WriteTo->WriteString(a_Profile->UUID);
	WriteTo->WriteString(a_Profile->AccountName);
	WriteTo->WriteString(a_Profile->DisplayName);
	
	// Send the current local player configuration
	WriteTo->WriteInt8(g_SplitScreen);
	WriteTo->WriteUInt8(MAXSPLITSCREEN);
	for (j = -1, i = 0; i < MAXSPLITSCREEN; i++)
	{
		WriteTo->WriteUInt8(g_PlayerInSplit[i]);
		
		if (j == -1 && !g_PlayerInSplit[i])
			j = i;
	}
	WriteTo->WriteInt8(j);
	
	// Send
	WriteTo->BlockRecord(&Server->GetAddress());
}

/* D_CReqMapChange() -- Requests that the server change the map */
void D_CReqMapChange(P_LevelInfoEx_s* const a_Level, const bool a_Switch)
{		
	DNetController* Server;
	RBPerfectStream_c* WriteTo;
	int i, j;
	
	/* Check */
	if (!a_Level)
		return;
	
	/* Find Server */
	Server = DNetController::GetServer();
	
	// Not found?
	if (!Server)
		return;
	
	/* Get stream to write to */
	WriteTo = Server->GetPerfectWrite();
	
	// Not around?
	if (!WriteTo)
		return;
	
	/* Fill Block Info */
	// Base
	WriteTo->BlockBase("MAPR");
	
	// Data
	WriteTo->WriteUInt8(a_Switch);
	WriteTo->WriteString(a_Level->LumpName);
	
	// Send
	WriteTo->BlockRecord(&Server->GetAddress());
}

/* D_CMakePureRandom() -- Create a pure random number */
uint32_t D_CMakePureRandom(void)
{
	uint32_t Garbage, i;
	uint32_t* RawBits;
	
	/* Allocate Raw Bits */
	RawBits = (uint32_t*)I_SysAlloc(sizeof(*RawBits) * 16);
	
	/* Attempt number generation */
	// Init
	Garbage = 0;
	
	// Current Time
	Garbage ^= ((int)I_GetTime() * (int)I_GetTime());
	
	// Address of this function
	Garbage ^= (uint32_t)(((uintptr_t)D_CMakePureRandom) * ((uintptr_t)D_CMakePureRandom));
	
	// Address of garbage
	Garbage ^= (uint32_t)(((uintptr_t)&Garbage) * ((uintptr_t)&Garbage));
	
	// Current PID
	Garbage ^= ((uint32_t)I_GetCurrentPID() * (uint32_t)I_GetCurrentPID());
	
	// Allocated Data
	if (RawBits)
	{
		// Raw bits address
		Garbage ^= (uint32_t)(((uintptr_t)RawBits) * ((uintptr_t)RawBits));
	
		// Raw bits data (unitialized memory)
		for (i = 0; i < 16; i++)
			Garbage ^= RawBits[i];
	
		// Cleanup
		I_SysFree(RawBits);
	}
	
	/* Return the garbage number */
	return Garbage;
}

/* D_CMakeUUID() -- Makes a UUID */
void D_CMakeUUID(char* const a_Buf)
{
	size_t i, FailCount;
	uint8_t Char;
	uint32_t Garbage;
	
	/* Generate a hopefully random ID */
	for (i = 0; i < (MAXPLAYERNAME * 2) - 1; i++)
	{
		// Hopefully random enough
		Garbage = D_CMakePureRandom();
		Char = (((int)(M_Random())) + Garbage);
		FailCount = 0;
		
		// Limit Char
		while (!((Char >= '0' && Char <= '9') || (Char >= 'a' && Char <= 'z') || (Char >= 'A' && Char <= 'Z')))
		{
			if (Char <= 'A')
				Char += 15;
			else if (Char >= 'z')
				Char -= 15;
			else
				Char ^= D_CMakePureRandom();
			
			if (++FailCount >= 10)
				if (M_Random() & 1)
					Char = 'A' + (M_Random() % ('Y' - 'A'));
				else
					Char = 'a' + (M_Random() % ('y' - 'a'));
		}
		
		// Last character is the same as this?
		if (i > 0 && Char == a_Buf[i - 1])
		{
			i--;
			continue;
		}
		
		// Set as
		a_Buf[i] = Char;
		
		// Sleep for some unknown time
		I_WaitVBL(M_Random() & 1);
	}
}

/* D_CNetHandleEvent() -- Handle advanced events */
bool D_CNetHandleEvent(const I_EventEx_t* const a_Event)
{
	int32_t ButtonNum, LocalJoy;
	
	/* Check */
	if (!a_Event)
		return false;
	
	/* Which kind of event? */
	switch (a_Event->Type)
	{
			// Mouse
		case IET_MOUSE:
			// Add position to movement
			l_MouseMove[0] += a_Event->Data.Mouse.Move[0];
			l_MouseMove[1] += a_Event->Data.Mouse.Move[1];
			break;
			
			// Keyboard
		case IET_KEYBOARD:
			if (a_Event->Data.Keyboard.KeyCode >= 0 && a_Event->Data.Keyboard.KeyCode < NUMIKEYBOARDKEYS)
				l_KeyDown[a_Event->Data.Keyboard.KeyCode] = a_Event->Data.Keyboard.Down;
			break;
			
			// Joystick
		case IET_JOYSTICK:
			// Get local joystick
			LocalJoy = a_Event->Data.Joystick.JoyID;
			
			// Now determine which action
			if (LocalJoy >= 0 && LocalJoy < MAXLOCALJOYS)
			{
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
			break;
		
			// Unknown
		default:
			break;
	}
	
	/* Un-Handled */
	return false;
}
