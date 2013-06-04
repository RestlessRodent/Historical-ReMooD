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
		memset(&l_GlobalBuf[l_GlobalAt], 0, sizeof(&l_GlobalBuf[l_GlobalAt]));
	}
	
	/* First attempt to grab, from first set */
	l_GlobalBuf[l_GlobalAt].Ctrl.Type = 1;	// Set extended
	if (D_SNExtCmdInTicCmd(a_ID, a_Wp, &l_GlobalBuf[l_GlobalAt]))
		return true;
	
	/* Failed, increase global at and try next one */
	l_GlobalAt++;
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

/* D_SNDisconnect() -- Disconnects from server */
void D_SNDisconnect(const bool_t a_FromDemo)
{
	static bool_t InDis;	
	int32_t i;
	
	/* If disconnected already, stop */
	if (InDis)
		return;
	
	// Do not double disconnect
	InDis = true;
	
	/* Clear hosts */
	l_MyHost = NULL;
	if (l_Hosts)
	{
		// Individual host
		for (i = 0; i < l_NumHosts; i++)
			D_SNDestroyHost(l_Hosts[i]);
		Z_Free(l_Hosts);
	}
	
	// Clear pointers
	l_Hosts = NULL;
	l_NumHosts = 0;
	
	/* Clear the global buffer */
	l_GlobalAt = -1;
	memset(l_GlobalBuf, 0, sizeof(l_GlobalBuf));
	
	/* Done disconnecting */
	InDis = true;
}

/* D_SNIsConnected() -- Connected to server */
bool_t D_SNIsConnected(void)
{
	return l_Server || l_Connected;
}

/* D_SNStartServer() -- Starts local server */
bool_t D_SNStartServer(const int32_t a_NumLocal, const char** const a_Profs)
{
	int32_t i;
	const char* ProfN;
	
	/* Disconnect first */
	D_SNDisconnect(false);
	
	/* Set flags */
	l_Server = l_Connected = true;
	
	/* Set the proper gamestate */
	gamestate = wipegamestate = GS_WAITINGPLAYERS;
	S_ChangeMusicName("D_WAITIN", 1);			// A nice tune
	
	/* Set game settings */
	NG_ApplyVars();
	
	/* Add local profile players */
	for (i = 0; i < a_NumLocal; i++)
	{
		// Get profile to add from
		ProfN = a_Profs[i];
		
		// Add profile to screens
		D_SNAddLocalPlayer(ProfN, 0, i, false);
		
		// If first player, do not steal
		if (!i && !ProfN)
			g_Splits[0].DoNotSteal = true;
	}
	
	/* Calculate Split-screen */
	R_ExecuteSetViewSize();
	
	/* Add local host player */
	l_MyHost = D_SNCreateHost();
	l_MyHost->Local = true; // must be local
	
	/* Created */
	return true;
}

/* D_SNServerInit() -- Initializes Server Mode */
bool_t D_SNServerInit(void)
{
#define BUFSIZE 256
	char Buf[BUFSIZE];
	const char* PProfs[MAXPLAYERS];
	int32_t np, i;
	
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
		for (np = 0, i = 0; i < MAXPLAYERS; i++)
		{
			// Command to check for (-pX)
			snprintf(Buf, BUFSIZE - 1, "-p%i", i + 1);
		
			// See if argument is set
			if (M_CheckParm(Buf))
				if (M_IsNextParm())
					PProfs[np++] = M_GetNextParm();
			
			// If player 1 missing?
			if (i == 0 && !np)
				np = 1;
		}
	}
	
	/* Networked or local? */
	// Command line local game
	if (NG_IsAutoStart())
	{
		// Start Server
		D_SNStartServer(np, PProfs);
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
	int32_t i;
	D_SplitInfo_t* Split;
	D_SNPort_t* Port;
	ticcmd_t* TicCmdP;
	
	/* Do not perform this when not connected */
	if (!l_Connected)
		return;
	
	/* Go through local screens */
	if (!l_DedSv)	// Dedicated server only has bots
		for (i = 0; i < MAXSPLITSCREEN; i++)
		{
			// Does not have player
			if (!D_ScrSplitHasPlayer(i))
				continue;
		
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
			
				// Try to grab a port
				Split->Port = D_SNRequestPort();
			
				// If grabbed, set local screen ID
				if (Split->Port)
					Split->Port->Screen = i;
			}
		
			// Screen has no profile
			else if (!Split->Profile)
			{
			}
		}
	
	/* Go throgh local ports */
	for (i = 0; i < l_MyHost->NumPorts; i++)
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

/* D_SNUpdate() -- Updates network state */
void D_SNUpdate(void)
{
	int32_t h, p;
	D_SNHost_t* Host;
	D_SNPort_t* Port;
	
	/* Do not update in demo */
	if (demoplayback)
		return;	
	
	/* Update ports */
	D_SNUpdateLocalPorts();
	
	/* Server only ahead */
	if (!l_Server)
		return;
	
	/* Go through all hosts and ports */
	for (h = 0; h < l_NumHosts; h++)
		if ((Host = l_Hosts[h]))
		{
			// Go through ports
			for (p = 0; p < Host->NumPorts; p++)
				if ((Port = Host->Ports[p]))
				{
					// If will join is set, try joining this port
					if (Port->WillJoin)
						D_SNPortTryJoin(Port);
				}
		}
}

/*** HOST CONTROL ***/

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

/* D_SNCreateHost() -- Creates new host */
D_SNHost_t* D_SNCreateHost(void)
{
	D_SNHost_t* New;
	int32_t i;
	
	/* Allocate new */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Place into list */
	for (i = 0; i < l_NumHosts; i++)
		if (!l_Hosts[i])
			return l_Hosts[i] = New;
	
	/* Need to resize */
	Z_ResizeArray((void**)&l_Hosts, sizeof(*l_Hosts), l_NumHosts, l_NumHosts + 1);
	return l_Hosts[l_NumHosts++] = New;
}

/* D_SNDestroyHost() -- Destroys host */
void D_SNDestroyHost(D_SNHost_t* const a_Host)
{
	int32_t i;
	
	/* Check */
	if (!a_Host)
		return;
	
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
				if ((Port = Host->Ports[i]))
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
	
	/* Free */
	Z_Free(a_Port);
}

/* D_SNRequestPort() -- Requests port from server */
D_SNPort_t* D_SNRequestPort(void)
{
	int32_t i;
	D_SNPort_t* Port;
	uint32_t ID;
	
	/* No local host */
	if (!l_MyHost)
		return NULL;
	
	/* Go through and see if any ports are not used */
	for (i = 0; i < l_MyHost->NumPorts; i++)
	{
		// See if port is in this spot
		if (!(Port = l_MyHost->Ports[i]))
			continue;
		
		// If not a bot and is free, use this one
		if (Port->Screen < 0 && !Port->Bot)
			return Port;
	}
	
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
	else
	{
	}
	
	/* No port found, yet */
	return NULL;
}

/* D_SNAddLocalPlayer() -- Adds local player to game */
bool_t D_SNAddLocalPlayer(const char* const a_Name, const uint32_t a_JoyID, const int8_t a_ScreenID, const bool_t a_UseJoy)
{
	uint32_t LastScreen;
	int32_t PlaceAt, i, UngrabbedScreen;
	D_Prof_t* Profile;
	bool_t BumpSplits;
	
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
	
	// Never redisplay
		// Also if a player is not active, then reset the display status
	if (!demoplayback)
		if (!g_Splits[PlaceAt].Active)
		{
			g_Splits[PlaceAt].Console = -1;
			g_Splits[PlaceAt].Display = -1;
		}
	
	g_Splits[PlaceAt].RequestSent = false;
	g_Splits[PlaceAt].GiveUpAt = 0;
	
	// Bind stuff here
	g_Splits[PlaceAt].Waiting = true;
	g_Splits[PlaceAt].Profile = Profile;
	g_Splits[PlaceAt].JoyBound = a_UseJoy;
	g_Splits[PlaceAt].JoyID = a_JoyID;
	
	// Resize Splits
	if (BumpSplits)
		if (!demoplayback)
		{
			if (PlaceAt >= g_SplitScreen)
				g_SplitScreen = ((int)PlaceAt);// - 1;
			R_ExecuteSetViewSize();
		}
	
	/* Added OK */
	return true;
}

/* D_SNTics() -- Handles tic commands */
void D_SNTics(ticcmd_t* const a_TicCmd, const bool_t a_Write, const int32_t a_Player)
{
	D_SNPort_t* Port;
	
	/* If not writing, clear tic command */
	if (!a_Write)
		memset(a_TicCmd, 0, sizeof(a_TicCmd));
	
	/* If server, we dictate commands */
	if (l_Server)
	{
		// Write Commands to clients
		if (a_Write)
		{
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
					return;	// oops!
				
				// Merge all the local stuff
				D_XNetMergeTics(a_TicCmd, Port->LocalBuf, Port->LocalAt);
				Port->LocalAt = 0;
				memset(Port->LocalBuf, 0, sizeof(Port->LocalBuf));
			}
		}
	}
	
	/* Otherwise, server dictates to us */
	else
	{
		// Client cannot write commands
		if (a_Write)
			return;
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
		if (!PortMap[p])
			break;
	
	// No room
	if (p >= MAXPLAYERS)
		return;
	
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
		
	/* Update Scores */
	P_UpdateScores();
	
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
			}
		}
}

/* D_SNHandleGT() -- Handles game command IDs */
void D_SNHandleGT(const uint8_t a_ID, const uint8_t** const a_PP)
{
	uint32_t HID, ID, i;
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
	
	/* Which Command? */
	switch (a_ID)
	{
			// Player Joins Game
		case DTCT_SNJOINPLAYER:
			D_SNHandleGTJoinPlayer(a_ID, a_PP, Host, Port, HID, ID, PID);
			break;
			
			// Unknown!?!
		default:
			break;
	}
}

