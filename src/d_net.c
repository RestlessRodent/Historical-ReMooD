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
// Copyright (C) 2011-2013 GhostlyDeath <ghostlydeath@remood.org>
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
#include "doomstat.h"
#include "d_net.h"
#include "m_argv.h"
#include "p_demcmp.h"
#include "r_main.h"
#include "p_info.h"
#include "g_game.h"
#include "d_prof.h"
#include "d_netmst.h"
#include "p_setup.h"
#include "d_main.h"
#include "p_inter.h"
#include "p_local.h"

/****************
*** CONSTANTS ***
****************/

#define MAXGLOBALBUFSIZE					8	// Size of global buffer

/*************
*** LOCALS ***
*************/

static bool_t l_DedSv;							// Dedicated server
static bool_t l_Connected;						// Connected
static bool_t l_Server;							// We are server

static ticcmd_t l_GlobalBuf[MAXGLOBALBUFSIZE];	// Global buffer
static int32_t l_GlobalAt = -1;					// Position Global buf is at

static D_SNHost_t* l_MyHost;					// This games host
static D_SNHost_t** l_Hosts;					// Hosts
static int32_t l_NumHosts;						// Number of hosts

D_SNHost_t*** g_HostsP = &l_Hosts;
int32_t* g_NumHostsP = &l_NumHosts;

static D_SNTicBuf_t l_NowTic[2];				// Tic that is now!
static uint8_t l_NowPress;						// Which "now" is pressed

static D_SNTicBuf_t l_LocalBuf[MAXNETXTICS];	// Local tic buffer
static int32_t l_LocalAt;						// Local tics at

/****************
*** FUNCTIONS ***
****************/

bool_t D_NetSetPlayerName(const int32_t a_PlayerID, const char* const a_Name)
{
	return true;
}

bool_t D_NetPlayerChangedPause(const int32_t a_PlayerID)
{
	return true;
}

/*** GLOBAL TICS ***/

/* D_SNExtCmdInGlobal() -- Grabs extended command in global command */
bool_t D_SNExtCmdInGlobal(const uint8_t a_ID, uint8_t** const a_Wp)
{
	/* Check */
	if (a_ID < 0 || a_ID >= NUMDTCT || !a_Wp)
		return false;
	
	/* Prevent global buffer overflow */
	if (l_GlobalAt >= MAXGLOBALBUFSIZE - 1)
		return false;
	
	/* Nothing grabbed? */
	if (l_GlobalAt < 0)
	{
		l_GlobalAt = 0;
		memset(&l_GlobalBuf[l_GlobalAt], 0, sizeof(l_GlobalBuf[l_GlobalAt]));
	}
	
	/* First attempt to grab, from first set */
	l_GlobalBuf[l_GlobalAt].Ctrl.Type = 1;	// Set extended
	if (D_SNExtCmdInTicCmd(a_ID, a_Wp, &l_GlobalBuf[l_GlobalAt]))
		return true;
	
	/* Failed, increase global at and try next one */
	l_GlobalAt++;
	memset(&l_GlobalBuf[l_GlobalAt], 0, sizeof(l_GlobalBuf[l_GlobalAt]));
	l_GlobalBuf[l_GlobalAt].Ctrl.Type = 1;	// Set extended
	if (D_SNExtCmdInTicCmd(a_ID, a_Wp, &l_GlobalBuf[l_GlobalAt]))
		return true;
	
	/* Completely failed */
	return false;
}

/* D_SNExtCmdInTicCmd() -- Grabs extended command in tic command */
bool_t D_SNExtCmdInTicCmd(const uint8_t a_ID, uint8_t** const a_Wp, ticcmd_t* const a_TicCmd)
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

/* D_SNDropAllClients() -- Drops all clients from the game */
void D_SNDropAllClients(const char* const a_Reason)
{
	static bool_t Dropping;
	int32_t i;
	D_SNHost_t* Host;
	
	/* Do not double drop */
	if (Dropping)
		return;
	Dropping = true;
	
	/* Clear hosts */
	if (l_Hosts)
		for (i = 0; i < l_NumHosts; i++)
			if ((Host = l_Hosts[i]))
				D_SNDisconnectHost(Host, (a_Reason ? a_Reason : "Server disconnect"));
	
	/* Done dropping */
	Dropping = false;
}

/* D_SNDisconnect() -- Disconnects from server */
void D_SNDisconnect(const bool_t a_FromDemo, const char* const a_Reason)
{
	static bool_t InDis;
	D_SNHost_t* Host;
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
	if (l_Connected)
		D_NCResetSplits(a_FromDemo);
	
	/* Clear hosts */
	D_SNDropAllClients(a_Reason);
	
	/* Terminate network connection */
	D_SNNetTerm(a_Reason);
	
	/* Destroy hosts */
	if (l_Hosts)
	{
		// Individual host
		for (i = 0; i < l_NumHosts; i++)
		{
			if ((Host = l_Hosts[i]))
				D_SNDestroyHost(Host);
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
	
	/* Clear the global buffer */
	l_GlobalAt = -1;
	memset(l_GlobalBuf, 0, sizeof(l_GlobalBuf));
	
	/* Clear local Buffer */
	memset(l_LocalBuf, 0, sizeof(l_LocalBuf));
	l_LocalAt = 0;
	
	/* Clear flags */
	l_Connected = l_Server = false;
	
	/* Clear now tics and jobs */
	memset(l_NowTic, 0, sizeof(l_NowTic));
	l_NowPress = 0;
	D_SNClearJobs();
	
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

/* D_SNPartialDisconnect() -- Partial Disconnect */
void D_SNPartialDisconnect(const char* const a_Reason)
{
	static bool_t InDis;	
	int32_t i;
	D_SNHost_t* Host;
	
	/* If disconnected already, stop */
	if (InDis)
		return;
	
	/* Show message */
	CONL_OutputUT(CT_NETWORK, DSTR_DNETC_PARTIALDISC, "%s\n", (a_Reason ? a_Reason : "No Reason"));
	
	/* Terminate network connection */
	D_SNNetTerm(a_Reason);
	
	/* Clear the global buffer */
	l_GlobalAt = -1;
	memset(l_GlobalBuf, 0, sizeof(l_GlobalBuf));
	
	/* Clear now tics and jobs */
	memset(l_NowTic, 0, sizeof(l_NowTic));
	l_NowPress = 0;
	D_SNClearJobs();
	
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

/* D_SNIsConnected() -- Connected to server */
bool_t D_SNIsConnected(void)
{
	return l_Connected;
}

/* D_SNSetConnected() -- Set connection status */
void D_SNSetConnected(const bool_t a_Set)
{
	l_Connected = a_Set;
}

/* D_SNIsServer() -- Is connected */
bool_t D_SNIsServer(void)
{
	return l_Server;
}

/* D_SNStartWaiting() -- Start waiting */
void D_SNStartWaiting(void)
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

/* D_SNAddLocalProfiles() -- Adds local profile */
void D_SNAddLocalProfiles(const int32_t a_NumLocal, const char** const a_Profs)
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
		D_SNAddLocalPlayer(ProfN, 0, i, false);
		
		// If first player, do not steal
		//if (!i && !ProfN)
			g_Splits[i].DoNotSteal = true;
	}
}

/* D_SNStartServer() -- Starts local server */
bool_t D_SNStartServer(const int32_t a_NumLocal, const char** const a_Profs, const bool_t a_JoinPlayers)
{
	D_SNHost_t* Host;
	D_SNPort_t* Port;
	int32_t i, j;
	
	/* Disconnect first */
	D_SNDisconnect(false, "Starting server");
	
	/* Set flags */
	l_Server = true;
	
	/* Set the proper gamestate */
	D_SNStartWaiting();
	
	/* Set game settings */
	NG_ApplyVars();
	
	/* Add local host player */
	l_MyHost = D_SNCreateHost();
	l_MyHost->ID = D_CMakePureRandom();
	l_MyHost->Local = true; // must be local
	
	/* Add local profile players */
	D_SNAddLocalProfiles(a_NumLocal, a_Profs);
	
	/* Calculate Split-screen */
	R_ExecuteSetViewSize();
	
	/* Force all available players to join */
	if (a_JoinPlayers)
		for (i = 0; i < l_NumHosts; i++)
			if ((Host = l_Hosts[i]))
				if (Host->Local)
					for (j = 0; j < Host->NumPorts; j++)
						if ((Port = Host->Ports[j]))
							D_SNPortTryJoin(Port);
	
	/* Warp to map */
	// This is here so auto-joined players are joined before the map
	NG_WarpMap();
	
	/* Force run a single tic */
	D_RunSingleTic();
	
	/* Created */
	return true;
}

/* D_SNStartLocalServer() -- Starts local server (just sets connected) */
bool_t D_SNStartLocalServer(const int32_t a_NumLocal, const char** const a_Profs, const bool_t a_JoinPlayers, const bool_t a_MakePlayer)
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
	if (D_SNStartServer(Local, Profs, a_JoinPlayers))
	{
		l_Connected = true;
		return true;
	}
	
	/* Failed */
	return false;
}

/* D_SNServerInit() -- Initializes Server Mode */
bool_t D_SNServerInit(void)
{
#define BUFSIZE 256
	char Buf[BUFSIZE];
	const char* PProfs[MAXSPLITSCREEN];
	int32_t np, i;
	char* Addr;
	uint16_t Port;
	bool_t Anti;
	
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
			D_SNStartServer(np, PProfs, false);
			
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
			D_SNNetCreate(!Anti, Addr, Port);
			
			// Normal server is always connected
			if (!Anti)
				l_Connected = true;
		}
		
		// Local Game (force joining of players and make local)
		else
		{
			// Use wrapper func
			D_SNStartLocalServer(np, PProfs, true, true);
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
		D_SNAddLocalProfiles(np, PProfs);
		D_SNNetCreate(Anti, Addr, Port);
		
		// Normal client is never connected
		if (Anti)
			l_Connected = true;
		
		// Make waiting for player
		D_SNStartWaiting();
		
		// Started something
		NG_SetAutoStart(true);
		return true;
	}
	
	/* No server started */
	return false;
#undef BUFSIZE
}

/*** LOOP ***/

/* D_SNUpdateLocalPorts() -- Updates local ports */
void D_SNUpdateLocalPorts(void)
{
	int32_t i, j, pc;
	D_SplitInfo_t* Split;
	D_SNPort_t* Port;
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
				Split->Port = D_SNRequestPort(Split->ProcessID, (g_ProgramTic >= Split->PortTimeOut));
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
				D_SNSetPortProfile(Split->Port, Split->Profile);
		}
		
		// No local players in game (always create one)
		if (!pc)
		{
			Profs[0] = NULL;
			D_SNAddLocalProfiles(1, (const char ** const)Profs);
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
					D_SNSetPortProfile(Port, Split->Profile);
				
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
				D_SNPortTicCmd(Port, TicCmdP);
			}
		}
	}
}

/* D_SNCleanupHost() -- Cleans up host */
bool_t D_SNCleanupHost(D_SNHost_t* const a_Host)
{
	char* s;
	uint8_t* Wp;
	int32_t At, Cat, i;
	const char* Name;
	D_SNPort_t* Port;
	
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
					D_SNRemovePlayer(Port->Player - players);
		
		// Reason
		for (Cat = 0, Wp = NULL, s = a_Host->QuitReason; *s; s++)
		{
			// Need more Wp
			if (!Wp)
			{
				if (!D_SNExtCmdInGlobal(DTCT_SNQUITREASON, &Wp))
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
		if (D_SNExtCmdInGlobal(DTCT_SNCLEANUPHOST, &Wp))
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
	D_SNDestroyHost(a_Host);
	return true;
}

/* D_SNUpdate() -- Updates network state */
void D_SNUpdate(void)
{
	int32_t h, p;
	D_SNHost_t* Host;
	D_SNPort_t* Port;
	
	/* Do not update ports or transport during demo */
	if (!demoplayback)
	{
		// Update ports
		D_SNUpdateLocalPorts();
	
		// HTTP Interface
		I_UpdateHTTPSpy();
	
		// Network Socket
		D_SNDoTrans();
	}
	
	/* Go through all hosts and ports */
	for (h = 0; h < l_NumHosts; h++)
		if ((Host = l_Hosts[h]))
		{
			// Cleaning up?
			if (Host->Cleanup)
				if (D_SNCleanupHost(Host))
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
							D_SNPortTryJoin(Port);
					}
			}
		}
}

/*** HOST CONTROL ***/

/* D_SNHostByAddr() -- Finds host by address */
D_SNHost_t* D_SNHostByAddr(const I_HostAddress_t* const a_Host)
{
	int32_t i;
	D_SNHost_t* Host;
	
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

/* D_SNHostByID() -- Finds host by ID */
D_SNHost_t* D_SNHostByID(const uint32_t a_ID)
{
	int32_t i;
	D_SNHost_t* Host;
	
	/* Through host list */
	for (i = 0; i < l_NumHosts; i++)
		if ((Host = l_Hosts[i]))
			if (Host->ID == a_ID)
				return Host;
	
	/* Not found */
	return NULL;
}

/* D_SNMyHost() -- Return your host */
D_SNHost_t* D_SNMyHost(void)
{
	return l_MyHost;
}

/* D_SNSetMyHost() -- Sets your current host */
void D_SNSetMyHost(D_SNHost_t* const a_Host)
{
	l_MyHost = a_Host;
}

/* D_SNCreateHost() -- Creates new host */
D_SNHost_t* D_SNCreateHost(void)
{
	D_SNHost_t* New;
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

/* D_SNDestroyHost() -- Destroys host */
void D_SNDestroyHost(D_SNHost_t* const a_Host)
{
	int32_t i;
	
	/* Check */
	if (!a_Host)
		return;
	
	/* Send disconnect */
	D_SNDisconnectHost(a_Host, NULL);
	
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
				D_SNRemovePort(a_Host->Ports[i]);
		Z_Free(a_Host->Ports);
	}
	
	// Free
	Z_Free(a_Host);
}

/*** PORT CONTROL ***/

/* D_SNPortByID() -- Finds port by ID */
D_SNPort_t* D_SNPortByID(const uint32_t a_ID)
{
	int32_t i, j;
	D_SNHost_t* Host;
	D_SNPort_t* Port;
	
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

/* D_SNAddPort() -- Add port to host */
D_SNPort_t* D_SNAddPort(D_SNHost_t* const a_Host)
{
	D_SNPort_t* New;
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

/* D_SNRemovePort() -- Removes port */
void D_SNRemovePort(D_SNPort_t* const a_Port)
{
	D_SNHost_t* Host;
	int32_t i;
	
	/* Check */
	if (!a_Port)
		return;
	
	/* Get host */
	Host = a_Port->Host;
	
	/* Remove from host list */
	for (i = 0; i < Host->NumPorts; i++)
		if (Host->Ports[i] == a_Port)
			Host->Ports[i] = NULL;
	
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

/* D_SNRequestPort() -- Requests port from server */
// a_XMit is used by the client when a transmission request should be made
// Otherwise, there will be an ugly 1 second delay ALL THE TIME when adding new
// local players. So if the server happens to send you a port in the middle of
// your timeout, then you still grab it.
D_SNPort_t* D_SNRequestPort(const uint32_t a_ProcessID, const bool_t a_XMit)
{
	int32_t i;
	D_SNPort_t* Port, *Best, *PM;
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
		Port = D_SNAddPort(l_MyHost);
		
		// Set port ID
		do
		{
			ID = D_CMakePureRandom();
		} while (!ID || D_SNPortByID(ID) || D_SNHostByID(ID));
		Port->ID = ID;
		
		// Return it
		return Port;
	}
	
	// Otherwise, need to send some packets
	else if (l_Connected)
	{
		// Only go over the net if required
		if (a_XMit)
			D_SNRequestPortNet(a_ProcessID);
	}
	
	/* No port found, yet */
	return NULL;
}

/* D_SNAddLocalPlayer() -- Adds local player to game */
bool_t D_SNAddLocalPlayer(const char* const a_Name, const uint32_t a_JoyID, const int8_t a_ScreenID, const bool_t a_UseJoy)
{
	uint32_t LastScreen;
	int32_t PlaceAt, UngrabbedScreen, i;
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
		Split->Port = D_SNRequestPort(Split->ProcessID, (g_ProgramTic >= Split->PortTimeOut));
		Split->PortTimeOut = g_ProgramTic + TICRATE;

		// If grabbed, set local screen ID
		if (Split->Port)
		{
			Split->Port->Screen = PlaceAt;
			Split->Port->ProcessID = Split->ProcessID;
		}
	}
	
	/* Added OK */
	return true;
}

/* D_SNBufForGameTic() -- Returns buffer for gametic */
D_SNTicBuf_t* D_SNBufForGameTic(const tic_t a_GameTic)
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

/* D_SNStartTic() -- Start of a new tic */
void D_SNStartTic(const tic_t a_GameTic)
{
	D_SNTicBuf_t* Now = &l_NowTic[l_NowPress];
	
	/* Start of a new tic */
	// Clear the current press
	memset(Now, 0, sizeof(*Now));
	
	/* Write gametic here */
	Now->GameTic = a_GameTic;
}

/* D_SNNumSeqTics() -- Returns number of sequential local tics */
int32_t D_SNNumSeqTics(void)
{
	int32_t RetVal;
	
	/* Count */
	for (RetVal = 0; RetVal < MAXNETXTICS; RetVal++)
		if (!l_LocalBuf[RetVal].GotTic)
			break;	// No more tics received
	
	/* Return count */
	return RetVal;
}

/* D_SNTics() -- Handles tic commands */
void D_SNTics(ticcmd_t* const a_TicCmd, const bool_t a_Write, const int32_t a_Player)
{
	D_SNHost_t* Host;
	D_SNPort_t* Port;
	int32_t h, p;
	D_SNTicBuf_t* Now = &l_NowTic[l_NowPress];
	
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
					// Move the first item inside
					memmove(a_TicCmd, &l_GlobalBuf[0], sizeof(*a_TicCmd));
					
					// Move everything down
					memmove(&l_GlobalBuf[0], &l_GlobalBuf[1], sizeof(ticcmd_t) * (MAXGLOBALBUFSIZE - 1));
					memset(&l_GlobalBuf[MAXGLOBALBUFSIZE - 1], 0, sizeof(l_GlobalBuf[MAXGLOBALBUFSIZE - 1]));
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
				
				// Handle local angles, if a local player
				if (Port->Host && Port->Host->Local)
				{
					// Find screen number
					for (h = 0; h < MAXSPLITSCREEN; h++)
						if (D_ScrSplitHasPlayer(h))
							if (g_Splits[h].Port == Port)
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

/* D_SNSyncCode() -- Inputs sync code */
void D_SNSyncCode(const tic_t a_GameTic, const uint32_t a_Code)
{
	D_SNTicBuf_t* Now = &l_NowTic[l_NowPress];
	
	/* If not connected, do not care */
	if (!l_Connected)
		return;
	
	/* Time of last tic being ran */
	D_SNSetLastTic();
	
	/* Server */
	if (l_Server)
	{
		// Invert press
		l_NowPress = !l_NowPress;
		
		// Encode sync code in this buffer
		Now->GameTic = a_GameTic;
		Now->SyncCode = a_Code;
		
		// Tic can now be sent to clients
		D_SNXMitTics(a_GameTic, Now);
	}
	
	/* Client */
	else
	{
		// Send sync code to server
		
		// Move down all the local tics
		memmove(&l_LocalBuf[0], &l_LocalBuf[1], sizeof(l_LocalBuf[0]) * (MAXNETXTICS - 1));
		
		// Clear last local tic
		memset(&l_LocalBuf[MAXNETXTICS - 1], 0, sizeof(l_LocalBuf[0]));
	}
}

/* D_SNSetPortProfile() -- Set port profile */
void D_SNSetPortProfile(D_SNPort_t* const a_Port, D_Prof_t* const a_Profile)
{
	/* Check */
	if (!a_Port)
		return;
	
	/* Set it locally */
	a_Port->Profile = a_Profile;
	
	// Clone to screen
	if (a_Port->Screen >= 0 && a_Port->Screen < MAXSPLITSCREEN)
		g_Splits[a_Port->Screen].Profile = a_Profile;
	
	/* Broadcast information to everyone else */
	// This also sets in game details, if playing, etc.
}

/* D_SNPortRequestJoin() -- Request join on the server */
void D_SNPortRequestJoin(D_SNPort_t* const a_Port)
{
	int32_t ActN, HypoN, LocN;
	int32_t h, p;
	D_SNHost_t* Host;
	D_SNPort_t* Port;
	
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
	}
}

/* D_SNPortTryJoin() -- Try to join port game */
void D_SNPortTryJoin(D_SNPort_t* const a_Port)
{
	int32_t h, p;
	D_SNHost_t* Host;
	D_SNPort_t* Port;
	D_SNPort_t* PortMap[MAXPLAYERS];
	uint8_t* Wp;
	
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
	if (!D_SNExtCmdInGlobal(DTCT_SNJOINPLAYER, &Wp))
	{
		Port->Player = NULL;	// whoops!!
		return;
	}
	
	// Fill
	LittleWriteUInt32((uint32_t**)&Wp, Host->ID);
	LittleWriteUInt32((uint32_t**)&Wp, Port->ID);
	WriteUInt8((uint8_t**)&Wp, p);
}

/*** GAME CONTROL ***/

/* D_SNRemovePlayer() -- Remove player from game */
void D_SNRemovePlayer(const int32_t a_PlayerID)
{
	uint8_t* Wp;
	
	/* Check */
	if (!l_Server || a_PlayerID < 0 || a_PlayerID >= MAXPLAYERS)
		return;
	
	/* Build packet */
	if (D_SNExtCmdInGlobal(DTCT_SNPARTPLAYER, &Wp))
	{
		LittleWriteUInt32((uint32_t**)&Wp, 0);
		LittleWriteUInt32((uint32_t**)&Wp, 0);
		WriteUInt8((uint8_t**)&Wp, a_PlayerID);
	}
}

/* D_SNChangeMap() -- Changes the map */
void D_SNChangeMap(const char* const a_NewMap, const bool_t a_Reset)
{
	uint8_t* Wp;
	P_LevelInfoEx_t* Info;
	int32_t i, j;	
	
	/* Check */
	if (!a_NewMap)
		return;
	
	/* See if the map exists first */
	if (!(Info = P_FindLevelByNameEx(a_NewMap, NULL)))
		return;
	
	/* Server */
	if (l_Server)
	{
		// Attempt global grab
		if (D_SNExtCmdInGlobal(DTCT_MAPCHANGE, &Wp))
		{
			// Resetting players?
			WriteUInt8((uint8_t**)&Wp, a_Reset);
			
			// Map name
			for (i = 0, j = 0; i < 8; i++)
				if (!j)
				{
					WriteUInt8((uint8_t**)&Wp, Info->LumpName[i]);
				
					if (!Info->LumpName[i])
						j = 1;
				}
				else
					WriteUInt8((uint8_t**)&Wp, 0);
		}
	}
	
	/* Client */
	else
	{
	}
}

/* D_SNHandleGTJoinPlayer() -- Handle join player */
static void D_SNHandleGTJoinPlayer(const uint8_t a_ID, const uint8_t** const a_PP, D_SNHost_t* const a_Host, D_SNPort_t* const a_Port, const uint32_t a_HID, const uint32_t a_UID, const uint8_t a_PID)
{
	int32_t i, n;
	D_SNHost_t* Host;
	D_SNPort_t* Port;
	player_t* Player;
	D_SplitInfo_t* Split;
	
	/* Out of bounds? */
	if (a_PID < 0 || a_PID >= MAXPLAYERS)
		return;
	
	/* Already taken */
	if (playeringame[a_PID])
		return;
	
	/* Count existing players in game */
	for (i = n = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
			n++;
	
	// Set multiplayer mode and spawn multiplayer junk
	if (n > 0 && !P_XGSVal(PGS_COMULTIPLAYER))
	{
		P_XGSSetValue(true, PGS_COMULTIPLAYER, 1);
		P_XGSSetValue(true, PGS_GAMESPAWNMULTIPLAYER, 1);
	}
	
	/* If no host exists, create one */
	if (!(Host = a_Host))
	{
		// Base
		Host = D_SNCreateHost();
		
		// Set pointers and such
		Host->ID = a_HID;
		Host->Local = false;
	}
	
	/* If no port exists, create one */
	if (!(Port = a_Port))
	{
		// Base
		Port = D_SNAddPort(Host);
		
		// Fields
		Port->ID = a_UID;
		Port->Bot = false;
	}
	
	/* Mark in game */
	playeringame[a_PID] = true;
	
	/* Create player in the game */
	Port->Player = Player = G_AddPlayer(a_PID);
	
	// Set local fields
	Player->Port = Port;
	
	/* Update Scores */
	P_UpdateScores();
	
	/* Spawn player into game */
	// Control monster, initially
	if (P_GMIsCounter() && Player->CounterOpPlayer)
		P_ControlNewMonster(Player);
	
	// Spawn them normally
	else
		G_DoReborn(a_PID);
	
	/* Bind to split screens, if any on the local side */
	for (i = 0; i < MAXSPLITSCREEN; i++)
		if (D_ScrSplitVisible(i))
		{
			// Current split
			Split = &g_Splits[i];
			
			// Matches port
			if (Split->Port == Port)
			{
				Split->Active = true;
				Split->Waiting = false;
				Split->Console = Split->Display = a_PID;
				
				localaiming[i] = 0;
				if (Player->mo)
					localangle[i] = Player->mo->angle;
				else
					localangle[i] = 0;
				
				// Set profile to broadcast data changes
				D_SNSetPortProfile(Port, Split->Profile);
			}
		}
}

/* D_SNHandleGTPartPlayer() -- Handle leaving player */
static void D_SNHandleGTPartPlayer(const uint8_t a_ID, const uint8_t** const a_PP, D_SNHost_t* const a_Host, D_SNPort_t* const a_Port, const uint32_t a_HID, const uint32_t a_UID, const uint8_t a_PID)
{
	player_t* Player;
	int32_t i;
	mobj_t* Mo;
	
	/* Out of bounds? */
	if (a_PID < 0 || a_PID >= MAXPLAYERS)
		return;
	
	/* Get player */
	Player = &players[a_PID];
	
	/* Count as suicide against self */
	// This is for any scripting, held flags, etc.
	Mo = Player->mo;
	
	if (Mo)	// Do not kill a counter-op controlled monster however
		if (!P_GMIsCounter() || (P_GMIsCounter() && !Player->CounterOpPlayer))
			P_KillMobj(Mo, Mo, Mo);
	
	// Player is no longer in game
	if (Mo)
		Mo->player = NULL;
	Player->mo = NULL;
	playeringame[a_PID] = false;
	
	/* Make local split a spectator */
	for (i = 0; i < MAXSPLITSCREEN; i++)
		if (D_ScrSplitVisible(i))
			if (g_Splits[i].Console == a_PID)
			{
				// Remove console view
				g_Splits[i].Console = -1;
				
				// If F12ing other player, keep F12
				if (g_Splits[i].Display != g_Splits[i].Console)
					g_Splits[i].Display = -1;
				
				// Make inactive
				g_Splits[i].Active = false;
				
				if (!demoplayback)
					g_Splits[i].Waiting = true;
			}
	
	/* Unassign port */
	Player->Port = NULL;
	
	if (a_Port)
		a_Port->Player = NULL;
}

/* D_SNHandleGTCleanupHost() -- Handle delete host */
static void D_SNHandleGTQuitMsg(const uint8_t a_ID, const uint8_t** const a_PP, D_SNHost_t* const a_Host, D_SNPort_t* const a_Port, const uint32_t a_HID, const uint32_t a_UID, const uint8_t a_PID)
{
	char Buf[MAXTCSTRINGCAT + 1];
	int32_t Cat, i;
	
	/* Need Host */
	if (!a_Host)
		return;
	
	/* Concat? */
	Cat = ReadUInt8(a_PP);
	
	/* Read Message */
	for (i = 0; i < MAXTCSTRINGCAT; i++)
		Buf[i] = ReadUInt8(a_PP);
	Buf[MAXTCSTRINGCAT] = 0;
	
	/* Add to reason */
	// Clear reason if not cat
	if (!Cat)
		memset(a_Host->QuitReason, 0, sizeof(a_Host->QuitReason));
	
	// Cat always
	strncat(a_Host->QuitReason, Buf, MAXQUITREASON - 1);
	
}

/* D_SNHandleGTCleanupHost() -- Handle delete host */
static void D_SNHandleGTCleanupHost(const uint8_t a_ID, const uint8_t** const a_PP, D_SNHost_t* const a_Host, D_SNPort_t* const a_Port, const uint32_t a_HID, const uint32_t a_UID, const uint8_t a_PID)
{
	/* Check */
	if (!a_Host)
		return;
	
	/* Call cleanup */
	if (!l_Server)
		D_SNCleanupHost(a_Host);
}

/* D_SNHandleGTJoinHost() -- Host joins the game */
static void D_SNHandleGTJoinHost(const uint8_t a_ID, const uint8_t** const a_PP, D_SNHost_t* const a_Host, D_SNPort_t* const a_Port, const uint32_t a_HID, const uint32_t a_UID, const uint8_t a_PID)
{
	D_SNHost_t* New;	
	
	/* Create host if it does not exist */
	if (!a_Host)
	{
		New = D_SNCreateHost();
		New->ID = a_HID;
	}
	
	/* Display message */
	CONL_OutputUT(CT_NETWORK, DSTR_NET_CLIENTCONNECTED, "%s\n", "Client");
}

/* D_SNHandleGTJoinPort() -- Handles joining of a new port */
static void D_SNHandleGTJoinPort(const uint8_t a_ID, const uint8_t** const a_PP, D_SNHost_t* const a_Host, D_SNPort_t* const a_Port, const uint32_t a_HID, const uint32_t a_UID, const uint8_t a_PID)
{
	D_SNHost_t* Host;
	D_SNPort_t* New;
	
	/* Create host if it does not exist */
	if (!(Host = a_Host))
	{
		Host = D_SNCreateHost();
		Host->ID = a_HID;
	}
	
	/* If port does not exist, create it */
	// Give packet may have reached client already
	if (!a_Port)
	{
		/* Create new port belonging to this host */
		New = D_SNAddPort(Host);
	
		// Set fields
		New->ID = a_UID;
		New->ProcessID = LittleReadUInt32((const uint32_t**)a_PP);
	}
	
	// Does not exist
	else
		New = a_Port;
	
	/* Display message */
	// As long as one was never displayed...
	if (!New->AttachMsg)
	{
		CONL_OutputUT(CT_NETWORK, DSTR_NET_PORTCONNECTED, "%s\n", "Client");
		New->AttachMsg = true;
	}
}

/* D_SNHandleGT() -- Handles game command IDs */
void D_SNHandleGT(const uint8_t a_ID, const uint8_t** const a_PP)
{
	uint32_t HID, ID;
	uint8_t PID;
	D_SNPort_t* Port;
	D_SNHost_t* Host;
	
	/* Check */
	if (!a_ID || !a_PP)
		return;
	
	/* All start with ID and PID */
	HID = LittleReadUInt32((const uint32_t**)a_PP);
	ID = LittleReadUInt32((const uint32_t**)a_PP);
	PID = ReadUInt8((const uint8_t**)a_PP);
	
	// Find port and host
	Port = D_SNPortByID(ID);
	
	Host = NULL;
	if (Port)
		Host = Port->Host;
	else
		Host = D_SNHostByID(HID);
	
	/* Which Command? */
	switch (a_ID)
	{
			// Player Joins Game
		case DTCT_SNJOINPLAYER:
			D_SNHandleGTJoinPlayer(a_ID, a_PP, Host, Port, HID, ID, PID);
			break;
			
			// Disconnect reason
		case DTCT_SNQUITREASON:
			D_SNHandleGTQuitMsg(a_ID, a_PP, Host, Port, HID, ID, PID);
			break;
			
			// Delete Host
		case DTCT_SNCLEANUPHOST:
			D_SNHandleGTCleanupHost(a_ID, a_PP, Host, Port, HID, ID, PID);
			break;
			
			// Create Host
		case DTCT_SNJOINHOST:
			D_SNHandleGTJoinHost(a_ID, a_PP, Host, Port, HID, ID, PID);
			break;
			
			// Player leaves game
		case DTCT_SNPARTPLAYER:
			D_SNHandleGTPartPlayer(a_ID, a_PP, Host, Port, HID, ID, PID);
			break;
			
			// Port is added to host
		case DTCT_SNJOINPORT:
			D_SNHandleGTJoinPort(a_ID, a_PP, Host, Port, HID, ID, PID);
			break;
			
			// Unknown!?!
		default:
			break;
	}
}

