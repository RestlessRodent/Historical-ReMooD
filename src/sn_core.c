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
// DESCRIPTION:

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "sn.h"
#include "g_state.h"
#include "d_player.h"
#include "d_netcmd.h"
#include "d_prof.h"
#include "console.h"
#include "dstrings.h"
#include "z_zone.h"
#include "p_demcmp.h"
#include "r_main.h"
#include "m_argv.h"
#include "g_game.h"
#include "p_setup.h"
#include "s_sound.h"
#include "d_clisrv.h"

/****************
*** CONSTANTS ***
****************/

#define MAXGLOBALBUFSIZE					8	// Size of global buffer
#define MAXRRSYNCSIZE			(TICRATE * 2)	// Round robin size

/*************
*** LOCALS ***
*************/

static struct
{
	tic_t GameTic;								// Gametic
	uint32_t Code;								// Sync Code
} l_SyncCodeRR[MAXRRSYNCSIZE];					// Round robin for sync code
static int32_t l_SyncRRAt;						// Position of Round Robin

static bool_t l_DedSv;							// Dedicated server
static bool_t l_Connected;						// Connected
static bool_t l_Server;							// We are server

static ticcmd_t* l_GlobalBuf;					// Global buffer
static int32_t l_GlobalAt = -1;					// Position Global buf is at
static int32_t l_GlobalMax = 0;					// Max size of global buffer

static SN_Host_t* l_MyHost;					// This games host
static SN_Host_t** l_Hosts;					// Hosts
static int32_t l_NumHosts;						// Number of hosts

SN_Host_t*** g_HostsP = &l_Hosts;
int32_t* g_NumHostsP = &l_NumHosts;

static SN_TicBuf_t l_NowTic[2];				// Tic that is now!
static uint8_t l_NowPress;						// Which "now" is pressed

static SN_TicBuf_t l_LocalBuf[MAXNETXTICS];	// Local tic buffer
static int32_t l_LocalAt;						// Local tics at

/****************
*** FUNCTIONS ***
****************/

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
		CONL_OutputUT(CT_NETWORK, DSTR_NETPLAYERRENAMED, "%s%s\n", OldName, player_names[a_PlayerID]);
	
	/* Success! */
	return true;
}

bool_t D_NetPlayerChangedPause(const int32_t a_PlayerID)
{
	return true;
}

/*** GLOBAL TICS ***/

/* SN_ExtCmdInGlobal() -- Grabs extended command in global command */
bool_t SN_ExtCmdInGlobal(const uint8_t a_ID, uint8_t** const a_Wp)
{
	/* Check */
	if (a_ID < 0 || a_ID >= NUMDTCT || !a_Wp)
		return false;
	
	/* Prevent global buffer overflow */
	if (l_GlobalAt >= l_GlobalMax - 1)
	{
		Z_ResizeArray((void**)&l_GlobalBuf, sizeof(*l_GlobalBuf),
			l_GlobalMax, l_GlobalMax + 1);
		l_GlobalMax++;
	}
	
	/* Nothing grabbed? */
	if (l_GlobalAt < 0)
	{
		l_GlobalAt = 0;
		memset(&l_GlobalBuf[l_GlobalAt], 0, sizeof(l_GlobalBuf[l_GlobalAt]));
	}
	
	/* First attempt to grab, from first set */
	l_GlobalBuf[l_GlobalAt].Ctrl.Type = 1;	// Set extended
	if (SN_ExtCmdInTicCmd(a_ID, a_Wp, &l_GlobalBuf[l_GlobalAt]))
		return true;
	
	/* Failed, increase global at and try next one */
	l_GlobalAt++;
	memset(&l_GlobalBuf[l_GlobalAt], 0, sizeof(l_GlobalBuf[l_GlobalAt]));
	l_GlobalBuf[l_GlobalAt].Ctrl.Type = 1;	// Set extended
	if (SN_ExtCmdInTicCmd(a_ID, a_Wp, &l_GlobalBuf[l_GlobalAt]))
		return true;
	
	/* Completely failed */
	return false;
}

/* SN_ExtCmdInTicCmd() -- Grabs extended command in tic command */
bool_t SN_ExtCmdInTicCmd(const uint8_t a_ID, uint8_t** const a_Wp, ticcmd_t* const a_TicCmd)
{
	uint16_t* dsP;
	uint8_t* dbP;
	
	/* Check */
	if (a_ID < 0 || a_ID >= NUMDTCT || !a_Wp || !a_TicCmd)
		return false;
	
	/* Extended Tic */
	if (a_TicCmd->Ctrl.Type == 1)
	{
		dsP = &a_TicCmd->Ext.DataSize;
		dbP = a_TicCmd->Ext.DataBuf;
	}
		
	/* Standard Tic */
	else if (a_TicCmd->Ctrl.Type == 0)
	{
		dsP = &a_TicCmd->Std.DataSize;
		dbP = a_TicCmd->Std.DataBuf;
	}
	
	/* Bad Type */
	else
		return false;
	
	/* Not enough room to store extended command? */
	if (c_TCDataSize[a_ID] + 2 >= MAXTCDATABUF - *dsP)
		return false;
		
	/* Write Command at point */
	*a_Wp = &((dbP)[*dsP]);
	WriteUInt8((uint8_t**)a_Wp, a_ID);
	*dsP += c_TCDataSize[a_ID] + 1;
	
	// Was written OK
	return true;
}

/*** SERVER CONTROL ***/

/* SN_DropAllClients() -- Drops all clients from the game */
void SN_DropAllClients(const char* const a_Reason)
{
	static bool_t Dropping;
	int32_t i;
	SN_Host_t* Host;
	
	/* Do not double drop */
	if (Dropping)
		return;
	Dropping = true;
	
	/* Clear hosts */
	if (l_Hosts)
		for (i = 0; i < l_NumHosts; i++)
			if ((Host = l_Hosts[i]))
				SN_DisconnectHost(Host, (a_Reason ? a_Reason : "Server disconnect"));
	
	/* Done dropping */
	Dropping = false;
}

/* SN_CommonDiscStuff() -- Common disconnect stuff */
static void SN_CommonDiscStuff(void)
{
	int32_t i;
	
	/* Clear the global buffer */
	if (l_GlobalBuf)
		Z_Free(l_GlobalBuf);
	
	l_GlobalBuf = NULL;
	l_GlobalAt = -1;
	l_GlobalMax = 0;
	
	/* Clear local Buffer */
	memset(l_LocalBuf, 0, sizeof(l_LocalBuf));
	l_LocalAt = 0;
	
	/* Clear now tics and jobs */
	memset(l_NowTic, 0, sizeof(l_NowTic));
	l_NowPress = 0;
	SN_ClearJobs();
	
	/* Draw stuff */
	SN_SetServerLagWarn(0);
	
	/* Sync Codes */
	l_SyncRRAt = 0;
	memset(l_SyncCodeRR, 0, sizeof(l_SyncCodeRR));
}

/* SN_Disconnect() -- Disconnects from server */
void SN_Disconnect(const bool_t a_FromDemo, const char* const a_Reason)
{
	static bool_t InDis;
	SN_Host_t* Host;
	int i;
	
	/* If disconnected already, stop */
	if (InDis)
		return;
	
	// Do not double disconnect
	InDis = true;
	
	/* Stop demo playing */
	if (!a_FromDemo)
		if (demoplayback)
		{
			singledemo = false;
			G_StopDemoPlay();
		}
	
	/* Remove splits */
	if (l_Connected || SN_HasSocket())
		D_NCResetSplits(a_FromDemo);
	
	/* Clear hosts */
	SN_DropAllClients(a_Reason);
	
	/* Terminate network connection */
	SN_NetTerm(a_Reason);
	
	/* Destroy hosts */
	if (l_Hosts)
	{
		// Individual host
		for (i = 0; i < l_NumHosts; i++)
		{
			if ((Host = l_Hosts[i]))
				SN_DestroyHost(Host);
			l_Hosts[i] = NULL;
		}
		
		Z_Free(l_Hosts);
		l_NumHosts = 0;
	}
	
	// Clear pointers
	l_MyHost = NULL;
	l_Hosts = NULL;
	l_NumHosts = 0;
	
	/* Initialize some players some */
	for (i = 0; i < MAXPLAYERS; i++)
		G_InitPlayer(&players[i]);
	
	// In game
	memset(playeringame, 0, sizeof(playeringame));
	
	// Reset all variables
	P_XGSSetAllDefaults();
	
	/* Destroy the level */
	P_ExClearLevel();
	
	/* Common */
	SN_CommonDiscStuff();
	
	/* Clear flags */
	l_Connected = l_Server = false;
	
	/* Go back to the title screen */
	if (!a_FromDemo)
	{
		gamestate = GS_DEMOSCREEN;
		
		demosequence = -1;
		pagetic = -1;
	}
	
	/* Done disconnecting */
	InDis = false;
}

/* SN_PartialDisconnect() -- Partial Disconnect */
void SN_PartialDisconnect(const char* const a_Reason)
{
	static bool_t InDis;	
	int32_t i;
	SN_Host_t* Host;
	
	/* If disconnected already, stop */
	if (InDis)
		return;
	
	/* Show message */
	CONL_OutputUT(CT_NETWORK, DSTR_DNETC_PARTIALDISC, "%s\n", (a_Reason ? a_Reason : "No Reason"));
	
	/* Terminate network connection */
	SN_NetTerm(a_Reason);
	
	/* Common */
	SN_CommonDiscStuff();
	
	/* Magically become the server */
	l_Server = l_Connected = true;
	
	// Cleanup other players
	for (i = 0; i < l_NumHosts; i++)
		if ((Host = l_Hosts[i]))
			if (!Host->Local)	// Cleanup non-local players
			{
				Host->Cleanup = true;
				strncpy(Host->QuitReason, "Partial disconnect", MAXQUITREASON);
			}
	
	/* Done disconnecting */
	InDis = false;
}

/* SN_IsConnected() -- Connected to server */
bool_t SN_IsConnected(void)
{
	return l_Connected;
}

/* SN_SetConnected() -- Set connection status */
void SN_SetConnected(const bool_t a_Set)
{
	l_Connected = a_Set;
}

/* SN_IsServer() -- Is connected */
bool_t SN_IsServer(void)
{
	return l_Server;
}

/* SN_StartWaiting() -- Start waiting */
void SN_StartWaiting(void)
{
	/* Wipe from title screen */
	if (gamestate == GS_DEMOSCREEN)
		wipegamestate = GS_DEMOSCREEN;
	
	/* Change gamestate */
	gamestate = GS_WAITINGPLAYERS;

	/* Wipe into level? */
	// Only wipe from outside (title, etc.)
	if (wipegamestate == GS_NULL)
		wipegamestate = gamestate;
	S_ChangeMusicName("D_WAITIN", 1);			// A nice tune
}

/* SN_AddLocalProfiles() -- Adds local profile */
void SN_AddLocalProfiles(const int32_t a_NumLocal, const char** const a_Profs)
{
	int32_t i;
	const char* ProfN;
	
	/* Check */
	if (!a_Profs)
		return;
	
	/* Go through names */
	for (i = 0; i < a_NumLocal; i++)
	{
		// Get profile to add from
		ProfN = a_Profs[i];	// might be NULL
		
		// Add profile to screens
		SN_AddLocalPlayer(ProfN, 0, i, false);
		
		// If first player, do not steal
		//if (!i && !ProfN)
			g_Splits[i].DoNotSteal = true;
	}
}

/* SN_StartServer() -- Starts local server */
bool_t SN_StartServer(const int32_t a_NumLocal, const char** const a_Profs, const bool_t a_JoinPlayers)
{
	SN_Host_t* Host;
	SN_Port_t* Port;
	int32_t i, j;
	
	/* Disconnect first */
	SN_Disconnect(false, "Starting server");
	
	/* Set flags */
	l_Server = true;
	
	/* Go back to the first gametic */
	gametic = 0;
	
	/* Set the proper gamestate */
	SN_StartWaiting();
	
	/* Set game settings */
	NG_ApplyVars();
	
	/* Add local host player */
	l_MyHost = SN_CreateHost();
	l_MyHost->ID = D_CMakePureRandom();
	l_MyHost->Local = true; // must be local
	
	/* Add local profile players */
	SN_AddLocalProfiles(a_NumLocal, a_Profs);
	
	/* Calculate Split-screen */
	R_ExecuteSetViewSize();
	
	/* Force all available players to join */
	if (a_JoinPlayers)
		for (i = 0; i < l_NumHosts; i++)
			if ((Host = l_Hosts[i]))
				if (Host->Local)
					for (j = 0; j < Host->NumPorts; j++)
						if ((Port = Host->Ports[j]))
							SN_PortTryJoin(Port);
	
	/* Warp to map */
	// This is here so auto-joined players are joined before the map
	NG_WarpMap();
	
	/* Force run a single tic */
	D_RunSingleTic();
	
	/* Created */
	return true;
}

/* SN_StartLocalServer() -- Starts local server (just sets connected) */
bool_t SN_StartLocalServer(const int32_t a_NumLocal, const char** const a_Profs, const bool_t a_JoinPlayers, const bool_t a_MakePlayer)
{
	int32_t Local;
	const char* Profs[MAXSPLITSCREEN];
	
	/* Local copy */
	// If there are no local players, always make one
	// But cannot be done for saves
	if (a_MakePlayer)
	{
		if (a_NumLocal <= 0)
		{
			if (g_KeyDefaultProfile)
			{
				Profs[0] = g_KeyDefaultProfile->AccountName;
				Local = 1;
			}
		}
		else
			for (Local = 0; Local < a_NumLocal; Local++)
				if (Local < MAXSPLITSCREEN)
					Profs[Local] = a_Profs[Local];
	}
	
	// Save game (or dedicated local server!?), re-attach ports
	else
	{
		Local = 0;
		memset(Profs, 0, sizeof(Profs));
	}
	
	/* Normal statr */
	if (SN_StartServer(Local, Profs, a_JoinPlayers))
	{
		l_Connected = true;
		return true;
	}
	
	/* Failed */
	return false;
}

/* SN_ChangeMapCommand() -- Changes the current map */
static int SN_ChangeMapCommand(const uint32_t a_ArgC, const char** const a_ArgV)
{
	/* Not enough args or not the server */
	if (a_ArgC < 2 || !SN_IsServer() || !l_Connected)
		return 1;
	
	/* Set new map */
	SN_ChangeMap(a_ArgV[1], !(a_ArgV[0][0] == 'w'));
	
	/* Success */
	return 0;
}

/* D_ServerCommand() -- Server Commands */
static int D_ServerCommand(const uint32_t a_ArgC, const char** const a_ArgV)
{
#define BUFSIZE 256
	char Buf[BUFSIZE];
	int i;
	
	/* Clear */
	memset(Buf, 0, sizeof(Buf));
	
	/* (Partial) Disconnect */
	if (!strcasecmp(a_ArgV[0], "disconnect") || !strcasecmp(a_ArgV[0], "part"))
	{
		// Read reason
		for (i = 1; i < a_ArgC; i++)
		{
			strncat(Buf, a_ArgV[1], BUFSIZE - 1);
			
			// Space splitter
			if (i < a_ArgC - 1)
				strncat(Buf, " ", BUFSIZE - 1);
		}
		
		// No reason?
		if (!Buf[0])
			strncat(Buf, "None given", BUFSIZE - 1);
		
		// Full or partial?
		if (a_ArgV[0][0] == 'p')
			SN_PartialDisconnect(Buf);
		else
			SN_Disconnect(false, Buf);
		
		// Worked
		return 0;
	}
	
	/* Local Game/Server */
	else if (!strcasecmp(a_ArgV[0], "localgame") || !strcasecmp(a_ArgV[0], "localserver"))
	{
		// Start local server, with this kind of stuff
		SN_StartLocalServer(a_ArgC - 1, a_ArgV + 1, false, true);
		
		// Worked
		return 0;
	}
	
	/* Connect to server */
	else if (!strcasecmp(a_ArgV[0], "connect"))
	{
		// Need 1 argument
		if (a_ArgC < 2)
			return 1;
		
		// Look for --, start of profile data
		for (i = 0; i < a_ArgC; i++)
			if (!strcmp(a_ArgV[i], "--"))
				break;
		
		// There is no --
		if (i >= a_ArgC)
			i = 0;
		
		// Disconnect from old server first
		if (SN_IsConnected() || SN_HasSocket() || demoplayback)
			SN_Disconnect(false, "Connecting to another server");
		
		// Add local profiles if specified
		if (i > 0)
			SN_AddLocalProfiles(a_ArgC - i, a_ArgV + i);
		
		// Create server
		SN_NetCreate(false, a_ArgV[1], 0);
		
		// Make waiting for player
		SN_StartWaiting();
		
		// Worked
		return 0;
	}
	
	/* Failed */
	return 1;
#undef BUFSIZE
}

/* SN_ServerInit() -- Initializes Server Mode */
bool_t SN_ServerInit(void)
{
#define BUFSIZE 256
	char Buf[BUFSIZE];
	const char* PProfs[MAXSPLITSCREEN];
	int32_t np, i;
	char* Addr;
	uint16_t Port;
	bool_t Anti;
	
	/* Add commands */
	CONL_AddCommand("disconnect", D_ServerCommand);
	CONL_AddCommand("part", D_ServerCommand);
	CONL_AddCommand("localgame", D_ServerCommand);
	CONL_AddCommand("localserver", D_ServerCommand);
	CONL_AddCommand("connect", D_ServerCommand);
	
	// Other commands that might not belong here
	CONL_AddCommand("map", SN_ChangeMapCommand);
	CONL_AddCommand("warp", SN_ChangeMapCommand);
	
	/* Initialize Multicast */
	SN_OpenMCast();
	
	/* Clear initial profiles */
	memset(PProfs, 0, sizeof(PProfs));
	
	/* Dedicated server? */
	if (M_CheckParm("-dedicated"))
	{
		l_DedSv = true;
		
		// No players inside
		np = 0;
	}
	
	// Otherwise, set 1 player
	else
	{
		// Not dedicated
		l_DedSv = false;
		
		// Add more players, if they are set
		for (np = 0, i = 0; i < MAXSPLITSCREEN; i++)
		{
			// Command to check for (-pX)
			snprintf(Buf, BUFSIZE - 1, "-p%i", i + 1);
		
			// See if argument is set
			if (M_CheckParm(Buf))
			{
				if (M_IsNextParm())
					PProfs[np++] = M_GetNextParm();
				else
					PProfs[np++] = NULL;	// No name selected
			}
			
			// If player 1 missing?
			if (i == 0 && !np)
				np = 1;
		}
	}
	
	/* Anti-Connection */
	Anti = false;
	if (M_CheckParm("-anti"))
		Anti = true;
	
	/* Networked or local? */
	// Command line local game
	if (NG_IsAutoStart())
	{
		// Initial Command line server?
		if (M_CheckParm("-host"))
		{
			// Start server (but do not join any players)
			SN_StartServer(np, PProfs, false);
			
			// Address to bind to, if any
			if (M_IsNextParm())
				Addr = M_GetNextParm();
			else
				Addr = NULL;
			
			// Port
			if (M_CheckParm("-port") && M_IsNextParm())
				Port = C_strtoi32(M_GetNextParm(), NULL, 10);
			else
				Port = __REMOOD_BASEPORT;
			
			// Create
			SN_NetCreate(!Anti, Addr, Port);
			
			// Normal server is always connected
			if (!Anti)
				l_Connected = true;
		}
		
		// Local Game (force joining of players and make local)
		else
		{
			// Use wrapper func
			SN_StartLocalServer(np, PProfs, true, true);
		}
		
		// Warp to map
		NG_WarpMap();
		
		// Successfully started
		return true;
	}
	
	/* Wanting to connect to remote server */
	else if (M_CheckParm("-connect"))
	{
		// Address to bind to, if any
		if (M_IsNextParm())
			Addr = M_GetNextParm();
		else
			Addr = NULL;
		
		// Port
		if (M_CheckParm("-port") && M_IsNextParm())
			Port = C_strtoi32(M_GetNextParm(), NULL, 10);
		else if (Anti)
			Port = __REMOOD_BASEPORT;
		else
			do
			{
				Port = D_CMakePureRandom();
			} while (Port <= 32767 || Port >= 65534);
		
		// Create
		SN_AddLocalProfiles(np, PProfs);
		SN_NetCreate(Anti, Addr, Port);
		
		// Normal client is never connected
		if (Anti)
			l_Connected = true;
		
		// Make waiting for player
		SN_StartWaiting();
		
		// Started something
		NG_SetAutoStart(true);
		return true;
	}
	
	/* No server started */
	return false;
#undef BUFSIZE
}

/*** LOOP ***/

/* SN_UpdateLocalPorts() -- Updates local ports */
void SN_UpdateLocalPorts(void)
{
	int32_t i, j, pc;
	D_SplitInfo_t* Split;
	SN_Port_t* Port;
	ticcmd_t* TicCmdP;
	char* Profs[1];
	
	/* Do not perform this when not connected */
	if (!l_Connected)
		return;
	
	/* Go through local screens */
	if (!l_DedSv)	// Dedicated server only has bots
	{
		// No players in game
		pc = 0;
		
		// Go through all screens
		for (i = 0; i < MAXSPLITSCREEN; i++)
		{
			// Does not have player
			if (!D_ScrSplitHasPlayer(i))
				continue;
			
			// Increase local player count
			pc++;
			
			// Quick
			Split = &g_Splits[i];
		
			// Screen has no port
			if (!Split->Port)
			{
				// If request not sent
				if (!Split->RequestSent)
				{
					// Sent and give up in 5 sconds
					Split->RequestSent = true;
					Split->GiveUpAt = g_ProgramTic + (TICRATE * 5);
				}
			
				// Request sent
				else
				{
					// Ran out of time?
					if (g_ProgramTic >= Split->GiveUpAt)
					{
						D_NCRemoveSplit(i, false);
						i--;
						continue;
					}
				}
			
				// Try to grab a port (xmit once per second)
				Split->Port = SN_RequestPort(Split->ProcessID, (g_ProgramTic >= Split->PortTimeOut));
				Split->PortTimeOut = g_ProgramTic + TICRATE;
		
				// If grabbed, set local screen ID
				if (Split->Port)
					Split->Port->Screen = i;
			}
		
			// Screen has no profile
			else if (!Split->Profile)
			{
				// Setup selection if not selecting
				if (!Split->SelProfile)
				{
					// If default profile exists, use that (if nobody else is using it)
					if (g_KeyDefaultProfile)
					{
						// Go through other screens
						for (j = 0; j < MAXSPLITSCREEN; j++)
							if (i != j && D_ScrSplitHasPlayer(j))
								if (g_Splits[j].Profile == g_KeyDefaultProfile)
									break;
						
						// Nobody is using default
						if (j >= MAXSPLITSCREEN)
							Split->Profile = g_KeyDefaultProfile;
					}
				
					// Start selecting if no profile found
					if (!Split->Profile)
					{
						Split->SelProfile = true;
						Split->AtProf = D_ProfFirst();
					}
				}
			}
			
			// If port has no profile, set it
			else if (!Split->Port->Profile)
				SN_SetPortProfile(Split->Port, Split->Profile);
		}
		
		// No local players in game (always create one)
		if (!pc)
		{
			Profs[0] = NULL;
			SN_AddLocalProfiles(1, (const char ** const)Profs);
		}
	}
	
	/* My host not set? */
	if (!l_MyHost && !demoplayback)
		for (i = 0; i < l_NumHosts; i++)
			if (l_Hosts[i])
				if (l_Hosts[i]->Local)
				{
					l_MyHost = l_Hosts[i];
					break;
				}
	
	/* Go throgh local ports */
	for (i = 0; l_MyHost && i < l_MyHost->NumPorts; i++)
	{
		// No port here?
		if (!(Port = l_MyHost->Ports[i]))
			continue;
			
		// If bot, run bot commands
		if (Port->Bot)
		{
		}
		
		// Is a player
		else
		{
			// Get split
			Split = NULL;
			if (Port->Screen >= 0 && Port->Screen < MAXSPLITSCREEN)
				Split = &g_Splits[Port->Screen];
			
			// If no profile set, ask for one
			if (!Port->Profile)
			{
				// If split has profile, use that
				if (Split->Profile)
					SN_SetPortProfile(Port, Split->Profile);
				
				// Otherwise ask
				else
				{
				}
			}
			
			// Add tic commands for this port
			else
			{
				// Place tic command at last spot, when possible
				TicCmdP = NULL;
				if (Port->LocalAt < MAXLBTSIZE - 1)
					TicCmdP = &Port->LocalBuf[Port->LocalAt++];
				else
					TicCmdP = &Port->LocalBuf[0];
				
				// Clear Tic command before rebuilding
				memset(TicCmdP, 0, sizeof(*TicCmdP));
				
				// Build tic commands
				SN_PortTicCmd(Port, TicCmdP);
			}
		}
	}
}

/* SN_CleanupHost() -- Cleans up host */
bool_t SN_CleanupHost(SN_Host_t* const a_Host)
{
	char* s;
	uint8_t* Wp;
	int32_t At, Cat, i;
	const char* Name;
	SN_Port_t* Port;
	
	/* Check */
	if (!a_Host)
		return false;
	
	/* If server, create tic command */
	if (l_Server)
	{
		// Remove players that the host controls
		for (i = 0; i < a_Host->NumPorts; i++)
			if ((Port = a_Host->Ports[i]))
				if (Port->Player)
					SN_RemovePlayer(Port->Player - players);
		
		// Reason
		for (Cat = 0, Wp = NULL, s = a_Host->QuitReason; *s; s++)
		{
			// Need more Wp
			if (!Wp)
			{
				if (!SN_ExtCmdInGlobal(DTCT_SNQUITREASON, &Wp))
					break;	// Could not fit reason
				At = 0;	// Reset at
				
				// Write ID and cat count
				LittleWriteUInt32((uint32_t**)&Wp, a_Host->ID);
				LittleWriteUInt32((uint32_t**)&Wp, 0);
				WriteUInt8((uint8_t**)&Wp, 0);
				WriteUInt8(&Wp, Cat++);
				
				// Cap the cat
				if (Cat > 1)
					Cat = 1;
			}
			
			// Copy Character
			Wp[At++] = *s;
			
			// At is near the end
			if (At >= MAXTCSTRINGCAT - 2)
				Wp = NULL;
		}
		
		// Host cleanup
		if (SN_ExtCmdInGlobal(DTCT_SNCLEANUPHOST, &Wp))
		{
			LittleWriteUInt32((uint32_t**)&Wp, a_Host->ID);
			LittleWriteUInt32((uint32_t**)&Wp, 0);
			WriteUInt8((uint8_t**)&Wp, 0);
		}
	}
	
	/* Message */
	// Figure out name
	Name = "Client";
	
	// Show message
	CONL_OutputUT(CT_NETWORK, DSTR_NET_CLIENTGONE, "%s%s\n", Name, a_Host->QuitReason);
	
	// Delete their host
	SN_DestroyHost(a_Host);
	return true;
}

/* SN_Update() -- Updates network state */
void SN_Update(void)
{
	int32_t h, p;
	SN_Host_t* Host;
	SN_Port_t* Port;
	
	/* Multicast */
	SN_DoMultiCast();
	
	/* Do not update ports or transport during demo */
	if (!demoplayback)
	{
		// Update ports
		SN_UpdateLocalPorts();
	
		// HTTP Interface
		I_UpdateHTTPSpy();
	
		// Network Socket
		SN_DoTrans();
	}
	
	/* Go through all hosts and ports */
	for (h = 0; h < l_NumHosts; h++)
		if ((Host = l_Hosts[h]))
		{
			// Cleaning up?
			if (Host->Cleanup)
				if (SN_CleanupHost(Host))
					continue;
			
			// Server only
			if (l_Server)
			{
				// Handle all ports
				for (p = 0; p < Host->NumPorts; p++)
					if ((Port = Host->Ports[p]))
					{
						// If will join is set, try joining this port
						if (Port->WillJoin)
							SN_PortTryJoin(Port);
					}
			}
		}
}

/*** HOST CONTROL ***/

/* SN_HostByAddr() -- Finds host by address */
SN_Host_t* SN_HostByAddr(const I_HostAddress_t* const a_Host)
{
	int32_t i;
	SN_Host_t* Host;
	
	/* Check */
	if (!a_Host)
		return NULL;
	
	/* Through host list */
	for (i = 0; i < l_NumHosts; i++)
		if ((Host = l_Hosts[i]))
			if (I_NetCompareHost(&Host->Addr, a_Host))
				return Host;
	
	/* Not found */
	return NULL;
}

/* SN_HostByID() -- Finds host by ID */
SN_Host_t* SN_HostByID(const uint32_t a_ID)
{
	int32_t i;
	SN_Host_t* Host;
	
	/* Through host list */
	for (i = 0; i < l_NumHosts; i++)
		if ((Host = l_Hosts[i]))
			if (Host->ID == a_ID)
				return Host;
	
	/* Not found */
	return NULL;
}

/* SN_MyHost() -- Return your host */
SN_Host_t* SN_MyHost(void)
{
	return l_MyHost;
}

/* SN_SetMyHost() -- Sets your current host */
void SN_SetMyHost(SN_Host_t* const a_Host)
{
	l_MyHost = a_Host;
}

/* SN_CreateHost() -- Creates new host */
SN_Host_t* SN_CreateHost(void)
{
	SN_Host_t* New;
	int32_t i;
	
	/* Allocate new */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Initialize */
	New->Save.Slot = -1;
	
	/* Place into list */
	for (i = 0; i < l_NumHosts; i++)
		if (!l_Hosts[i])
			return (l_Hosts[i] = New);
	
	/* Need to resize */
	Z_ResizeArray((void**)&l_Hosts, sizeof(*l_Hosts), l_NumHosts, l_NumHosts + 1);
	return (l_Hosts[l_NumHosts++] = New);
}

/* SN_DestroyHost() -- Destroys host */
void SN_DestroyHost(SN_Host_t* const a_Host)
{
	int32_t i;
	
	/* Check */
	if (!a_Host)
		return;
	
	/* Send disconnect */
	SN_DisconnectHost(a_Host, NULL);
	
	/* Remove from list */
	for (i = 0; i < l_NumHosts; i++)
		if (l_Hosts[i] == a_Host)
			l_Hosts[i] = NULL;
	
	// This is our host?
	if (a_Host == l_MyHost)
		l_MyHost = NULL;
	
	/* Cleanup host */
	// Clear ports
	if (a_Host->Ports)
	{
		for (i = 0; i < a_Host->NumPorts; i++)
			if (a_Host->Ports[i])
				SN_RemovePort(a_Host->Ports[i]);
		Z_Free(a_Host->Ports);
	}
	
	// Free
	Z_Free(a_Host);
}

/*** PORT CONTROL ***/

/* SN_PortByID() -- Finds port by ID */
SN_Port_t* SN_PortByID(const uint32_t a_ID)
{
	int32_t i, j;
	SN_Host_t* Host;
	SN_Port_t* Port;
	
	/* Through host list */
	for (i = 0; i < l_NumHosts; i++)
		if ((Host = l_Hosts[i]))
			for (j = 0; j < Host->NumPorts; j++)
				if ((Port = Host->Ports[j]))
					if (Port->ID == a_ID)
						return Port;
	
	/* Not found */
	return NULL;
}

/* SN_AddPort() -- Add port to host */
SN_Port_t* SN_AddPort(SN_Host_t* const a_Host)
{
	SN_Port_t* New;
	int32_t i;
	
	/* Check */
	if (!a_Host)
		return NULL;
	
	/* Allocate */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	// Add to list
	for (i = 0; i < a_Host->NumPorts; i++)
		if (!a_Host->Ports[i])
		{
			a_Host->Ports[i] = New;
			break;
		}
	
	// Need to resize
	if (i >= a_Host->NumPorts)
	{
		Z_ResizeArray((void**)&a_Host->Ports, sizeof(*a_Host->Ports), a_Host->NumPorts, a_Host->NumPorts + 1);
		a_Host->Ports[a_Host->NumPorts++] = New;
	}
	
	/* Set host */
	New->Host = a_Host;
	
	/* Return fresh port */
	return New;
}

/* SN_RemovePort() -- Removes port */
void SN_RemovePort(SN_Port_t* const a_Port)
{
	SN_Host_t* Host;
	int32_t i;
	uint8_t* Wp;
	
	/* Check */
	if (!a_Port)
		return;
	
	/* Get host */
	Host = a_Port->Host;
	
	/* Remove from host list */
	for (i = 0; i < Host->NumPorts; i++)
		if (Host->Ports[i] == a_Port)
		{
			Host->Ports[i] = NULL;
			break;
		}
	
	// Port already removed (might be from a bot call)
	if (i >= Host->NumPorts)
		return;
	
	/* Before removing port, place in command to let others know */
	// But only as the server
	if (l_Server)
		if (SN_ExtCmdInGlobal(DTCT_SNUNPLUGPORT, &Wp))
		{
			LittleWriteUInt32((uint32_t**)&Wp, a_Port->Host->ID);
			LittleWriteUInt32((uint32_t**)&Wp, a_Port->ID);
			WriteUInt8((uint8_t**)&Wp, 0);
		}
			
	/* Remove bot (if any) */
	// DBP is recursive on the server
	BOT_DestroyByPort(a_Port);
	
	/* Remove references by screen and player */
	// Player
	for (i = 0; i < MAXPLAYERS; i++)
		if (players[i].Port == a_Port)
			players[i].Port = NULL;
	
	// Screen
	for (i = 0; i < MAXSPLITSCREEN; i++)
		if (D_ScrSplitVisible(i))
			if (g_Splits[i].Port == a_Port)
			{
				D_NCRemoveSplit(i, false);
				i = -1;
			}
	
	/* Free */
	Z_Free(a_Port);
}

/* SN_UnplugPort() -- Unplugs port */
void SN_UnplugPort(SN_Port_t* const a_Port)
{
	/* Check */
	if (!a_Port)
		return;
	
	/* If Server, just remove */
	if (l_Server)
	{
		// If port is playing, kill player that is controlled by it
		if (a_Port->Player)
			SN_RemovePlayer(a_Port->Player - players);	
		
		SN_RemovePort(a_Port);
	}
	
	/* Otherwise, tell server to remove */
	else
		SN_UnplugPortNet(a_Port);
}

/* SN_RequestPort() -- Requests port from server */
// a_XMit is used by the client when a transmission request should be made
// Otherwise, there will be an ugly 1 second delay ALL THE TIME when adding new
// local players. So if the server happens to send you a port in the middle of
// your timeout, then you still grab it.
SN_Port_t* SN_RequestPort(const uint32_t a_ProcessID, const bool_t a_XMit)
{
	int32_t i;
	SN_Port_t* Port, *Best, *PM;
	uint32_t ID;
	
	/* No local host */
	if (!l_MyHost)
		return NULL;
	
	/* Go through and see if any ports are not used */
	Best = PM = NULL;
	for (i = 0; i < l_MyHost->NumPorts; i++)
	{
		// See if port is in this spot
		if (!(Port = l_MyHost->Ports[i]))
			continue;
		
		// ProcessID match
		if (a_ProcessID && Port->ProcessID == a_ProcessID)
			PM = Port;
		
		// If not a bot and is free, use this one
		if (Port->Screen < 0 && !Port->Bot)
			Best = Port;
	}
	
	// If no process ID is set and there is a best, use that
	if (Best && !a_ProcessID)
		return Best;
	
	// Check for a process ID match
	else if (PM && a_ProcessID)
		return PM;
	
	/* In network situation, ask for one */
	// If server, can just create our own port
	if (l_Server)
	{
		// Add port
		Port = SN_AddPort(l_MyHost);
		
		// Set port ID
		do
		{
			ID = D_CMakePureRandom();
		} while (!ID || SN_PortByID(ID) || SN_HostByID(ID));
		Port->ID = ID;
		
		// Set process ID (if any)
		Port->ProcessID = a_ProcessID;
		
		// Return it
		return Port;
	}
	
	// Otherwise, need to send some packets
	else if (l_Connected)
	{
		// Only go over the net if required
		if (a_XMit)
			SN_RequestPortNet(a_ProcessID);
	}
	
	/* No port found, yet */
	return NULL;
}

/* SN_AddLocalPlayer() -- Adds local player to game */
bool_t SN_AddLocalPlayer(const char* const a_Name, const uint32_t a_JoyID, const int8_t a_ScreenID, const bool_t a_UseJoy)
{
	uint32_t LastScreen;
	int32_t PlaceAt, UngrabbedScreen;
	D_Prof_t* Profile;
	bool_t BumpSplits;
	D_SplitInfo_t* Split;
	
	/* Check */
	if (a_ScreenID < 0 || a_ScreenID >= MAXSPLITSCREEN)
		return false;
		
	// Find Profile
	Profile = NULL;
	if (a_Name)
		Profile = D_FindProfileEx(a_Name);
	
	/* Find first free slot */
	// Find the last screened player
	UngrabbedScreen = -1;
	for (LastScreen = 0; LastScreen < MAXSPLITSCREEN; LastScreen++)
	{
		if (!D_ScrSplitHasPlayer(LastScreen))
			break;
		
		// If the split has no profile and profile IS being added, take over
		if (UngrabbedScreen == -1)
			if (!g_Splits[LastScreen].Profile && !g_Splits[LastScreen].DoNotSteal)
				UngrabbedScreen = LastScreen;
	}
	
	// Place at the first wanted spot, unless already bound
		// Assign joystick to player, can be in game
	if (a_UseJoy && D_ScrSplitHasPlayer(a_ScreenID) && !g_Splits[a_ScreenID].JoyBound)
		PlaceAt = a_ScreenID;
	
		// No joystick wanted
	else
	{
		// Cannot fit any more players
		if (LastScreen >= MAXSPLITSCREEN)
			return false;	
		
		if (UngrabbedScreen != -1)
			PlaceAt = UngrabbedScreen;
		else
			PlaceAt = LastScreen;
	}
	
	/* If not a bot, bind to a local screen */
	// Bump splits? Not if a screen has a player (controlling keyboarder)
	BumpSplits = true;
	if (D_ScrSplitHasPlayer(PlaceAt))
		BumpSplits = false;
	
	// Pointer
	Split = &g_Splits[PlaceAt];
	
	// Never redisplay
		// Also if a player is not active, then reset the display status
	if (!demoplayback)
		if (!Split->Active)
		{
			Split->Console = -1;
			Split->Display = -1;
		}
	
	Split->RequestSent = false;
	Split->GiveUpAt = 0;
	
	// Bind stuff here
	Split->Waiting = true;
	Split->Profile = Profile;
	Split->JoyBound = a_UseJoy;
	Split->JoyID = a_JoyID;
	
	// Generate unique process ID
	do
	{
		Split->ProcessID = D_CMakePureRandom();
	} while (!Split->ProcessID && D_NCSFindSplitByProcess(Split->ProcessID) != PlaceAt);
	
	// Resize Splits
	if (BumpSplits)
		if (!demoplayback)
		{
			if (PlaceAt >= g_SplitScreen)
				g_SplitScreen = ((int)PlaceAt);// - 1;
			R_ExecuteSetViewSize();
		}
	
	/* If server, add local port */
	if (l_Server && !Split->Port)
	{
		// Grab a port, only transmitting every second
		Split->Port = SN_RequestPort(Split->ProcessID, (g_ProgramTic >= Split->PortTimeOut));
		Split->PortTimeOut = g_ProgramTic + TICRATE;

		// If grabbed, set local screen ID
		if (Split->Port)
		{
			Split->Port->Screen = PlaceAt;
			Split->Port->ProcessID = Split->ProcessID;
			SN_SetPortProfile(Split->Port, Profile);
		}
	}
	
	/* Added OK */
	return true;
}

/* SN_BufForGameTic() -- Returns buffer for gametic */
SN_TicBuf_t* SN_BufForGameTic(const tic_t a_GameTic)
{
	/* In the past */
	if (a_GameTic < gametic)
		return NULL;
	
	/* Too far into the future */
	if (a_GameTic >= gametic + MAXNETXTICS)
		return NULL;
	
	/* Return reference of local buffer */
	return &l_LocalBuf[a_GameTic - gametic];
}

/* SN_StartTic() -- Start of a new tic */
void SN_StartTic(const tic_t a_GameTic)
{
	SN_TicBuf_t* Now = &l_NowTic[l_NowPress];
	
	/* Start of a new tic */
	// Clear the current press
	memset(Now, 0, sizeof(*Now));
	
	/* Write gametic here */
	Now->GameTic = a_GameTic;
}

/* SN_NumSeqTics() -- Returns number of sequential local tics */
int32_t SN_NumSeqTics(void)
{
	int32_t RetVal;
	
	/* Count */
	for (RetVal = 0; RetVal < MAXNETXTICS; RetVal++)
		if (!l_LocalBuf[RetVal].GotTic)
			break;	// No more tics received
	
	/* Return count */
	return RetVal;
}

/* SN_LocalTurn() -- Perform local turning */
void SN_LocalTurn(SN_Port_t* const a_Port, ticcmd_t* const a_TicCmd)
{
	int32_t h;
	
	// Handle local angles, if a local player
	if (a_Port->Host && a_Port->Host->Local)
	{
		// Find screen number
		for (h = 0; h < MAXSPLITSCREEN; h++)
			if (D_ScrSplitHasPlayer(h))
				if (g_Splits[h].Port == a_Port)
					break;
		
		if (h < MAXSPLITSCREEN)
		{
			// Absolute Angles
			if (P_XGSVal(PGS_COABSOLUTEANGLE))
			{
				localangle[h] += a_TicCmd->Std.BaseAngleTurn << 16;
				a_TicCmd->Std.angleturn = localangle[h] >> 16;
			}

			// Doom Angles
			else
				a_TicCmd->Std.angleturn = a_TicCmd->Std.BaseAngleTurn;
	
			// Aiming Angle
			if (a_TicCmd->Std.buttons & BT_RESETAIM)
				localaiming[h] = 0;
			else
			{
				// Panning Look
				if (a_TicCmd->Std.buttons & BT_PANLOOK)
					localaiming[h] = a_TicCmd->Std.BaseAiming << 16;
			
				// Standard Look
				else
					localaiming[h] += a_TicCmd->Std.BaseAiming << 16;
			}
		
			// Clip aiming pitch to not exceed bounds
			a_TicCmd->Std.aiming = G_ClipAimingPitch(&localaiming[h]);
		}
	}
}

/* SN_Tics() -- Handles tic commands */
void SN_Tics(ticcmd_t* const a_TicCmd, const bool_t a_Write, const int32_t a_Player)
{
	SN_Host_t* Host;
	SN_Port_t* Port;
	int32_t h, p;
	SN_TicBuf_t* Now = &l_NowTic[l_NowPress];
	
	/* If not writing, clear tic command */
	if (!a_Write)
		memset(a_TicCmd, 0, sizeof(a_TicCmd));
	
	/* If server, we dictate commands */
	if (l_Server)
	{
		// Write Commands to clients
		if (a_Write)
		{
			// Save tic command in the now press
			p = (a_Player < 0 ? MAXPLAYERS : a_Player);
			memmove(&Now->Tics[p], a_TicCmd, sizeof(ticcmd_t));
		}
		
		// Read command from tic queues
		else
		{
			// Global
			if (a_Player < 0)
			{
				// Something is in the buffer
				if (l_GlobalAt >= 0)
				{
					// Use first global command
					memmove(a_TicCmd, l_GlobalBuf, sizeof(*a_TicCmd));
					
					// Move down other entries
					for (p = 1; p <= l_GlobalAt; p++)
						l_GlobalBuf[p - 1] = l_GlobalBuf[p];
					memset(&l_GlobalBuf[l_GlobalAt], 0, sizeof(l_GlobalBuf[l_GlobalAt]));
					
					// Reduce count
					l_GlobalAt--;
				}
				
				// Nothing
				else
					memset(a_TicCmd, 0, sizeof(*a_TicCmd));
			}
			
			// Player
			else
			{
				// Get port for this player
				if (!(Port = players[a_Player].Port))
				{
					// Slower code?
					for (h = 0; h < l_NumHosts; h++)
						if ((Host = l_Hosts[h]))
							for (p = 0; p < Host->NumPorts; p++)
								if (Host->Ports[p] && Host->Ports[p]->Player == &players[a_Player])
								{
									Port = Host->Ports[p];
									break;
								}
					
					if (!Port)
						return;	// oops!
				}
				
				// Merge all the local stuff
				if (Port->LocalAt > 0)
				{
					D_XNetMergeTics(a_TicCmd, Port->LocalBuf, Port->LocalAt);
					Port->LocalAt = 0;
					memset(Port->LocalBuf, 0, sizeof(Port->LocalBuf));
					
					// Store this tic as backup
					memmove(&Port->BackupCmd, a_TicCmd, sizeof(Port->BackupCmd));
					
					// Not lagging
					Port->LocalStatFlags &= ~DTSF_LOCALSTICK;
				}
				
				// Missed tic generation (use backup tic)
				else
				{
					memmove(a_TicCmd, &Port->BackupCmd, sizeof(Port->BackupCmd));
					
					// Lagging
					Port->LocalStatFlags |= DTSF_LOCALSTICK;
				}
				
				// Set ping from host
				a_TicCmd->Ctrl.Ping = Port->Host->Ping & TICPINGAMOUNTMASK;
				
				// Copy status flags from port
				if (!a_TicCmd->Ctrl.Type)
					a_TicCmd->Std.StatFlags = Port->StatFlags;
				
				// Do local turning and aiming
				SN_LocalTurn(Port, a_TicCmd);
			}
		}
	}
	
	/* Otherwise, server dictates to us */
	else
	{
		// Client cannot write commands
		if (a_Write)
			return;
		
		// Tic never received
		if (!l_LocalBuf[0].GotTic)
			return;
		
		// Copy tic owner
		memmove(a_TicCmd, &l_LocalBuf[0].Tics[(a_Player < 0 ? MAXPLAYERS : a_Player)], sizeof(*a_TicCmd));
	}
}

/* SN_SyncCode() -- Inputs sync code */
void SN_SyncCode(const tic_t a_GameTic, const uint32_t a_Code)
{
	SN_TicBuf_t* Now = &l_NowTic[l_NowPress];
	
	/* If not connected, do not care */
	if (!l_Connected)
		return;
	
	/* Time of last tic being ran */
	SN_SetLastTic();
	
	/* Server */
	if (l_Server)
	{
		// Invert press
		l_NowPress = !l_NowPress;
		
		// Encode sync code in this buffer
		Now->GameTic = a_GameTic;
		Now->SyncCode = a_Code;
		
		// Tic can now be sent to clients
		SN_XMitTics(a_GameTic, Now);
		
		// Round robin place
		l_SyncCodeRR[l_SyncRRAt].GameTic = a_GameTic;
		l_SyncCodeRR[l_SyncRRAt].Code = a_Code;
		l_SyncRRAt = (l_SyncRRAt + 1) % MAXRRSYNCSIZE;
	}
	
	/* Client */
	else
	{
		// Send sync code to server
		SN_SendSyncCode(a_GameTic, a_Code);
		
		// Move down all the local tics
		memmove(&l_LocalBuf[0], &l_LocalBuf[1], sizeof(l_LocalBuf[0]) * (MAXNETXTICS - 1));
		
		// Clear last local tic
		memset(&l_LocalBuf[MAXNETXTICS - 1], 0, sizeof(l_LocalBuf[0]));
	}
}

/* SN_CheckSyncCode() -- Checks sync code from client */
void SN_CheckSyncCode(SN_Host_t* const a_Host, const tic_t a_GameTic, const uint32_t a_Code)
{
	register int i;
	
	/* Check */
	if (!a_GameTic)
		return;
	
	/* Look in loop */
	for (i = 0; i < MAXRRSYNCSIZE; i++)
		if (l_SyncCodeRR[i].GameTic == a_GameTic)
		{
			// Synced
			if (l_SyncCodeRR[i].Code == a_Code)
				return;
			
			// Kick host
			SN_DisconnectHost(a_Host, "Game desynchronized");
			
			// Done anyway
			return;
		}
}

/* SN_SetPortProfile() -- Set port profile */
void SN_SetPortProfile(SN_Port_t* const a_Port, D_Prof_t* const a_Profile)
{
	/* Check */
	if (!a_Port)
		return;
	
	/* Set it locally */
	a_Port->Profile = a_Profile;
	
	// Clone to screen
	if (a_Port->Screen >= 0 && a_Port->Screen < MAXSPLITSCREEN)
		g_Splits[a_Port->Screen].Profile = a_Profile;
		
	// No profile
	if (!a_Profile)
		return;
	
	/* Broadcast information to everyone else */
	// This also sets in game details, if playing, etc.
	SN_PortSetting(a_Port, DSNPS_NAME, 0, a_Profile->DisplayName, 0);
	SN_PortSetting(a_Port, DSNPS_VTEAM, a_Profile->VTeam, NULL, 0);
	SN_PortSetting(a_Port, DSNPS_COLOR, a_Profile->Color, NULL, 0);
	SN_PortSetting(a_Port, DSNPS_COUNTEROP, a_Profile->CounterOp, NULL, 0);
}

/* SN_PortRequestJoin() -- Request join on the server */
void SN_PortRequestJoin(SN_Port_t* const a_Port)
{
	int32_t ActN, HypoN, LocN;
	int32_t h, p;
	SN_Host_t* Host;
	SN_Port_t* Port;
	
	/* Check */
	if (!a_Port)
		return;
	
	/* Go through other ports to count players inside */
	ActN = HypoN = 0;
	for (h = 0; h < l_NumHosts; h++)
		if ((Host = l_Hosts[h]))
			for (p = 0; p < Host->NumPorts; p++)
				if ((Port = Host->Ports[p]))
				{
					if (!Port->Player && Port->WillJoin)
						HypoN++;
					if (Port->Player)
						ActN++;
				}
	
	// Add actual to hypo
	HypoN += ActN;
	
	/* Perform join */
	// If server, does not exactly care
	if (l_Server)
	{
		// No Room
		if (ActN >= MAXPLAYERS)
			return;
		
		// If port is bot, always allow
		if (a_Port->Bot)
			a_Port->WillJoin = true;
		
		// Otherwise, only allow if split screens are free
		Host = l_MyHost;
		LocN = 0;
		for (p = 0; p < Host->NumPorts; p++)
			if ((Port = Host->Ports[p]))
				if (!Port->Bot && Port->Player)
					LocN++;
		
		// There is room
		if (LocN < MAXSPLITSCREEN)
			a_Port->WillJoin = true;
	}
	
	// Client must request it
	else
	{
		// Do not spam server with join requests though
		if (g_ProgramTic > a_Port->JoinWait)
		{
			SN_PortJoinGame(a_Port);
			a_Port->JoinWait = g_ProgramTic + (TICRATE >> 1);
		}
	}
}

/* SN_PortTryJoin() -- Try to join port game */
void SN_PortTryJoin(SN_Port_t* const a_Port)
{
	int32_t h, p;
	SN_Host_t* Host;
	SN_Port_t* Port;
	SN_Port_t* PortMap[MAXPLAYERS];
	uint8_t* Wp;
	uint32_t Flags;
	
	/* Only server can do this */
	if (!l_Server)
		return;
	
	/* Check */
	if (!a_Port)
		return;
	
	/* Do not join if already playing */
	if (a_Port->Player)
		return;
	
	/* Unmark will join */
	a_Port->WillJoin = false;
	
	/* Go through other ports and map players */
	// Clear
	memset(PortMap, 0, sizeof(PortMap));
	
	// Loop
	for (h = 0; h < l_NumHosts; h++)
		if ((Host = l_Hosts[h]))
			for (p = 0; p < Host->NumPorts; p++)
				if ((Port = Host->Ports[p]))
					if (Port->Player)
						PortMap[Port->Player - players] = Port;
	
	/* Find free player spot */
	for (p = 0; p < MAXPLAYERS; p++)
		if (!PortMap[p] && !playeringame[p])
			break;
	
	// No room
	if (p >= MAXPLAYERS)
		return;
	
	// Go back to port
	Port = a_Port;
	
	// If Host was never set, set
	if (!Host)
		Host = Port->Host;
	
	/* Make this port own this player now */
	Port->Player = &players[p];
	
	/* Create join packet */
	if (!SN_ExtCmdInGlobal(DTCT_SNJOINPLAYER, &Wp))
	{
		Port->Player = NULL;	// whoops!!
		return;
	}
	
	// Flags
	Flags = 0;
	
	if (Port->CounterOp)
		Flags |= DTCJF_MONSTERTEAM;
	
	// Fill
	LittleWriteUInt32((uint32_t**)&Wp, Host->ID);
	LittleWriteUInt32((uint32_t**)&Wp, Port->ID);
	WriteUInt8((uint8_t**)&Wp, p);
	WriteUInt8((uint8_t**)&Wp, Port->VTeam);
	WriteUInt8((uint8_t**)&Wp, Port->Color);
	LittleWriteUInt32((uint32_t**)&Wp, Flags);
}

/* SN_GetPortName() -- Get name of port */
const char* SN_GetPortName(SN_Port_t* const a_Port)
{
	/* No port passed */
	if (!a_Port)
		return "Illegal Name";
	
	/* Return correct name */
	else if (a_Port->Player)
		return player_names[a_Port->Player - players];
	
	/* Do not know */
	else
		return "Player";
}

/* SN_PortSettingInt() -- Sets integer value of setting */
void SN_PortSetting(SN_Port_t* const a_Port, const SN_PortSetting_t a_Setting, const int32_t a_IntVal, const char* const a_StrVal, const uint32_t a_StrLen)
{
	uint8_t* Wp;
	int32_t i;
	const char* p;
	
	/* Check */
	if (!a_Port || a_Setting < 0 || a_Setting >= NUMDSNPS)
		return;
	
	/* Server */
	// Affect settings directly
	if (SN_IsServer())
	{
		// Take gloal
		if (!SN_ExtCmdInGlobal(DTCT_SNPORTSETTING, &Wp))
		{
			CONL_PrintF("missed global %i\n", l_GlobalAt);
			return;
		}
	
		// Write Data
		LittleWriteUInt32((uint32_t**)&Wp, a_Port->Host->ID);
		LittleWriteUInt32((uint32_t**)&Wp, a_Port->ID);
		WriteUInt8(&Wp, (a_Port->Player ? a_Port->Player - players : 255));
		
		LittleWriteUInt16((uint16_t**)&Wp, a_Setting);
		
		// String?
		if (a_StrVal)
		{
			// No length specified
			if (!a_StrLen)
				for (p = a_StrVal, i = 0; i < MAXTCSTRINGCAT; i++)
				{
					// Write
					WriteUInt8(&Wp, *p);
				
					// Increase p as long as it is not NULL
					if (*p)
						p++;
				}
			
			// Length Specified
			else
				for (i = 0; i < MAXTCSTRINGCAT; i++)
					if (i < a_StrLen)
						WriteUInt8(&Wp, ((uint8_t*)a_StrVal)[i]);
					else
						WriteUInt8(&Wp, 0);
		}
		
		// Integer
		else
		{
			LittleWriteInt32((int32_t**)&Wp, a_IntVal);
			
			// Padding
			for (i = 4; i < MAXTCSTRINGCAT; i++)
				WriteUInt8(&Wp, 0);
		}
	}
	
	/* Client */
	// Send to server
	else
	{
		SN_SendSettings(a_Port, a_Setting, a_IntVal, a_StrVal, a_StrLen);
	}
	
	/* If this is a local port, change settings internally for this port */
	if (a_Port->Host->Local)
		SN_PortSettingOnPort(a_Port, a_Setting, a_IntVal, a_StrVal, a_StrLen);
}

/* SN_PortSettingOnPort() -- Change setting on a port */
void SN_PortSettingOnPort(SN_Port_t* const a_Port, const SN_PortSetting_t a_Setting, const int32_t a_IntVal, const char* const a_StrVal, const uint32_t a_StrLen)
{
	/* Check */
	if (!a_Port || a_Setting < 0 || a_Setting >= NUMDSNPS)	
		return;
	
	/* Based on setting passed */
	switch (a_Setting)
	{
		case DSNPS_VTEAM:	a_Port->VTeam = a_IntVal; break;
		case DSNPS_COLOR:	a_Port->Color = a_IntVal; break;
		case DSNPS_COUNTEROP:	a_Port->CounterOp = !!a_IntVal; break;
		default: break;
	}
	
	/* String based settings (to prevent crashes) */
	if (a_StrVal)
		switch (a_Setting)
		{
			case DSNPS_NAME:
				strncpy(a_Port->Name, a_StrVal, MAXPLAYERNAME);
				a_Port->Name[MAXPLAYERNAME - 1] = 0;
				break;
			
			default: break;
		}
}

