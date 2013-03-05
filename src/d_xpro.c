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
// DESCRIPTION: Extended Network Protocol -- Actual Protocol

/***************
*** INCLUDES ***
***************/

#include "d_xpro.h"
#include "d_netcmd.h"
#include "doomstat.h"
#include "console.h"
#include "dstrings.h"
#include "w_wad.h"

/*****************
*** STRUCTURES ***
*****************/

#define WADCHECKLEN	48

/* D_XWADCheck_t -- WADs to check for */
typedef struct D_XWADCheck_s
{
	size_t Count;								// Count of WADs to check
	char Long[WADCHECKLEN];						// Long Name
	char DOS[WADCHECKLEN];						// DOS Name
	char Sum[WADCHECKLEN];						// Checksum
	const WL_WADFile_t* WAD;					// WAD Specified
	const WL_WADFile_t* OrderWAD;				// WAD that is currently here
} D_XWADCheck_t;

/**************
*** GLOBALS ***
**************/

extern CONL_StaticVar_t l_SVJoinWindow;

/*************
*** LOCALS ***
*************/

static D_XWADCheck_t* l_WADChecks = NULL;		// WAD Checks
static bool_t l_PerfJoins = false;				// Performing Joins
static int32_t l_SaveBeingSent = 0;				// Save game being sent

/****************
*** FUNCTIONS ***
****************/

/* D_XPCleanup() -- Cleans up some things */
void D_XPCleanup(void)
{
	/* Clear WAD check list */
	if (l_WADChecks)
	{
		Z_Free(l_WADChecks);
		l_WADChecks = NULL;
	}
}

/* D_XPDropXPlay() -- Drops another XPlayer from the game */
void D_XPDropXPlay(D_XPlayer_t* const a_XPlay, const char* const a_Reason)
{
	/* Check */
	if (!g_XSocket || !a_XPlay || !D_XNetIsServer())
		return;
	
	/* Master Mode */
	if (g_XSocket->Master)
	{
		// Delete their endpoint
		D_XBDelEndPoint(a_XPlay->Socket.EndPoint, a_Reason);
		
		// Remove their reliable connection
		D_XBDropHost(&a_XPlay->Socket.Address);
	}
	
	/* Proxy Mode */
	else
	{
	}
}

/* D_XPRunConnection() -- Runs a connection */
void D_XPRunConnection(void)
{
	/* Run Neutral Socket */
	// Neutral socket is say login server, master, etc.
	
	/* Client/Server Communication */
	if (g_XSocket)
		D_XPRunCS(g_XSocket);
}

/* D_XPSendDisconnect() -- Sends disconnect */
void D_XPSendDisconnect(D_XDesc_t* const a_Desc, D_BS_t* const a_BS, I_HostAddress_t* const a_Addr, const char* const a_Reason)
{
	/* Check */
	if (!a_Desc || !a_BS || !a_Addr)
		return;
	
	/* Build Packet */
	D_BSBaseBlock(a_BS, "DISC");
	
	// Place Message
	D_BSwu32(a_BS, 0);
	
	if (a_Reason)
		D_BSws(a_BS, a_Reason);
	else
		D_BSwu8(a_BS, 0);
	
	// Send away
	D_BSRecordNetBlock(a_BS, a_Addr);
	
	/* Drop host too */
	D_XBDropHost(a_Addr);
}

/* DS_DumpSplitInfo() -- Dumps split info into packet */
static void DS_DumpSplitInfo(D_BS_t* const a_BS)
{
#define BUFSIZE 64
	char Buf[BUFSIZE];
	int i;
	D_ProfileEx_t* Prof;
	
	/* Check */
	if (!a_BS)
		return;
		
	/* Dump split info strings */
	for (i = 0; i < MAXSPLITSCREEN; i++)
	{
		// No player here?
		if (!D_ScrSplitHasPlayer(i))
			continue;
	
		// Process ID of screen
		memset(Buf, 0, sizeof(Buf));
		snprintf(Buf, BUFSIZE - 1, "pid+%i=%08x", i, g_Splits[i].ProcessID);
		D_BSws(a_BS, Buf);
	
		// Profile?
		Prof = g_Splits[i].Profile;
	
		// Dump profile info
		if (Prof)
		{
			// UUID of profile
			memset(Buf, 0, sizeof(Buf));
			snprintf(Buf, BUFSIZE - 1, "puuid+%i=%s", i, Prof->UUID);
			D_BSws(a_BS, Buf);
		
			// Display Name
			memset(Buf, 0, sizeof(Buf));
			snprintf(Buf, BUFSIZE - 1, "pdn+%i=%s", i, Prof->DisplayName);
			D_BSws(a_BS, Buf);
			
			// Color
			memset(Buf, 0, sizeof(Buf));
			snprintf(Buf, BUFSIZE - 1, "pc+%i=%i", i, Prof->Color);
			D_BSws(a_BS, Buf);
			
			// CounterOp
			memset(Buf, 0, sizeof(Buf));
			snprintf(Buf, BUFSIZE - 1, "pcp+%i=%s", i, (Prof->CounterOp ? "true" : "false"));
			D_BSws(a_BS, Buf);
			
			// VTeam
			memset(Buf, 0, sizeof(Buf));
			snprintf(Buf, BUFSIZE - 1, "pvt+%i=%i", i, Prof->VTeam);
			D_BSws(a_BS, Buf);
		}
	}
#undef BUFSIZE
}

/* DS_BringInClient() -- Brings in a connected client */
static bool_t DS_BringInClient(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP, D_BS_t* const a_BS)
{
#define BUFSIZE 72
	char Buf[BUFSIZE];
	char *EqS, *Plus;
	int32_t s, i;
	
	uint32_t ProcessID;
	D_XEndPoint_t* EP;
	D_XEndPoint_t Hold;
	
	/* Init Settings */
	ProcessID = 0;
	memset(&Hold, 0, sizeof(Hold));
	
	/* Read String Settings */
	for (;;)
	{
		// Read new string
		memset(Buf, 0, sizeof(Buf));
		D_BSrs(a_BS, Buf, BUFSIZE - 1);
		
		// End of strings?
		if (!Buf[0])
			break;
		
		// Get equal sign and zero it out
		EqS = strchr(Buf, '=');
		
		if (!EqS)
			continue;
		
		*(EqS++) = 0;
		
		// Process dynamic setting
			// Version
		if (!strcasecmp(Buf, "version"))
		{
		}
			
			// ProcessID of a split
		else if (!strncasecmp(Buf, "pid+", 4))
		{
			if ((Plus = strchr(Buf, '+')))
				if ((s = C_strtoi32(Plus + 1, NULL, 10)))
					if (s >= 0 && s < MAXSPLITSCREEN)
					{
						Hold.Splits[s].Active = true;
						Hold.Splits[s].ProcessID = C_strtou32(EqS, NULL, 0);
						
						if (!ProcessID)
							ProcessID = Hold.Splits[s].ProcessID;
					}
		}
		
			// Profile UUID of split
		else if (!strncasecmp(Buf, "puuid+", 6))
		{
			if ((Plus = strchr(Buf, '+')))
				if ((s = C_strtoi32(Plus + 1, NULL, 10)))
					if (s >= 0 && s < MAXSPLITSCREEN)
					{
						Hold.Splits[s].Active = true;
						strncpy(Hold.Splits[s].ProfUUID, EqS, MAXUUIDLENGTH);
					}
		}
		
			// Profile Display Name of split
		else if (!strncasecmp(Buf, "pdn+", 4))
		{
			if ((Plus = strchr(Buf, '+')))
				if ((s = C_strtoi32(Plus + 1, NULL, 10)))
					if (s >= 0 && s < MAXSPLITSCREEN)
					{
						Hold.Splits[s].Active = true;
						strncpy(Hold.Splits[s].DispName, EqS, MAXPLAYERNAME);
					}
		}
			
			// Color of a split
		else if (!strncasecmp(Buf, "pc+", 3))
		{
			if ((Plus = strchr(Buf, '+')))
				if ((s = C_strtoi32(Plus + 1, NULL, 10)))
					if (s >= 0 && s < MAXSPLITSCREEN)
					{
						Hold.Splits[s].Active = true;
						Hold.Splits[s].Color = C_strtou32(EqS, NULL, 0);
					}
		}
			
			// VTeam of a split
		else if (!strncasecmp(Buf, "vt+", 3))
		{
			if ((Plus = strchr(Buf, '+')))
				if ((s = C_strtoi32(Plus + 1, NULL, 10)))
					if (s >= 0 && s < MAXSPLITSCREEN)
					{
						Hold.Splits[s].Active = true;
						Hold.Splits[s].VTeam = C_strtou32(EqS, NULL, 0);
					}
		}
			
			// CounterOp Status of a split
		else if (!strncasecmp(Buf, "pcp+", 3))
		{
			if ((Plus = strchr(Buf, '+')))
				if ((s = C_strtoi32(Plus + 1, NULL, 10)))
					if (s >= 0 && s < MAXSPLITSCREEN)
					{
						Hold.Splits[s].Active = true;
						Hold.Splits[s].CounterOp = INFO_BoolFromString(EqS);
					}
		}
	}
	
	/* See if endpoint was already added? */
	EP = D_XBEndPointForAddr(a_Addr);
	
	// It was, no need to resend because it is reliable packet
	if (EP)
		return true;
	
	/* Create new endpoint */
	EP = D_XBNewEndPoint(a_Desc, a_Addr);
	
	EP->ProcessID = ProcessID;
	memmove(EP->Splits, Hold.Splits, sizeof(Hold.Splits));
	
	// Make the client reliable now
	D_BSStreamIOCtl(a_Desc->RelBS, DRBSIOCTL_ADDHOST, (intptr_t)a_Addr);
	
	// Inform client of their endpoint
	D_BSBaseBlock(a_Desc->RelBS, (a_Desc->Master ? "SYNJ" : "SYNC"));
	
	D_BSwu32(a_Desc->RelBS, EP->HostID);
	D_BSwu32(a_Desc->RelBS, EP->ProcessID);
	
	memset(Buf, 0, sizeof(Buf));
	I_NetHostToString(&EP->Addr, Buf, BUFSIZE - 1);
	D_BSws(a_Desc->RelBS, Buf);
	
	// Send away
	D_BSRecordNetBlock(a_Desc->RelBS, a_Addr);
	
	/* Put message on server */
	// Count players joining
	for (i = s = 0; i < MAXSPLITSCREEN; i++)
		if (EP->Splits[i].Active)
			s++;
	
	// Show message
	CONL_OutputUT(CT_NETWORK, DSTR_DXP_CLCONNECT, "%s%i%s\n", Buf, s, (s == 1 ? "" : "s"));
	
	/* Success! */
	return true;
#undef BUFSIZE
}

/* DS_JWJoins() -- Do join window joins */
static void DS_JWJoins(D_XPlayer_t* const a_Player, void* const a_Data)
{
	D_XEndPoint_t* EP;
	int32_t s;
	
	/* Get endpoint */
	EP = a_Data;
	
	/* Initialize Player */
	a_Player->Flags = 0;
	a_Player->HostID = EP->HostID;
	a_Player->Socket.EndPoint = EP;
	memmove(&a_Player->Socket.Address, &EP->Addr, sizeof(EP->Addr));
	
	/* Get screen to add for */
	s = EP->ScreenToAdd;
	
	/* Fill in info */
	a_Player->Color = EP->Splits[s].Color;
	a_Player->VTeam = EP->Splits[s].VTeam;
	a_Player->CounterOp = EP->Splits[s].CounterOp;
	a_Player->ClProcessID = EP->Splits[s].ProcessID;
	
	// Copy names
	strncat(a_Player->DisplayName, EP->Splits[s].DispName, MAXPLAYERNAME);
	strncat(a_Player->ProfileUUID, EP->Splits[s].ProfUUID, MAXUUIDLENGTH);
}

/* DS_HandleJoinWindow() -- Handle join windows */
static void DS_HandleJoinWindow(D_XDesc_t* const a_Desc)
{
#define BUFSIZE 128
#define MINISIZE 16
	char Buf[BUFSIZE];
	char Mini[MINISIZE];
	D_XEndPoint_t* Joins[MAXPLAYERS];
	D_XPlayer_t* XJoins[MAXPLAYERS * MAXSPLITSCREEN];
	int32_t i, j, s, ns, fs, x, k;
	
	/* Perform join windows? */
	if (g_ProgramTic >= a_Desc->CS.Server.JoinWindowTime)
	{
		// Clear joins
		memset(Joins, 0, sizeof(Joins));
		memset(XJoins, 0, sizeof(XJoins));
		
		// Go through all clients
		j = x = 0;
		if (!l_PerfJoins)
			for (i = 0; i < g_NumXEP; i++)
				if (g_XEP[i])
					if (!g_XEP[i]->Latched && g_XEP[i]->SignalReady &&
						g_ProgramTic >= g_XEP[i]->ReadyTime)
					{
						// Make them join
						if (j < MAXPLAYERS)
							Joins[j++] = g_XEP[i];
					}
		
		// Increase until next join window
		a_Desc->CS.Server.JoinWindowTime = g_ProgramTic + (l_SVJoinWindow.Value->Int * TICRATE);
		
		// Enough players waiting
		if (j > 0)
		{
			// Performing joins now
			l_PerfJoins = true;
			
			// Go through all endpoints and do massive xplayer generation
				// For each of their screens
			for (i = 0; i < j; i++)
			{
				// Count how many screens are playing
				for (s = ns = 0, fs = -1; s < MAXSPLITSCREEN; s++)
					if (Joins[i]->Splits[s].Active)
					{
						if (fs == -1)
							fs = s;
						ns++;
					}
				
				// Now add each player
				for (s = 0; s < MAXSPLITSCREEN; s++)
				{
					// Player has no split players or their split player happens
						// to be P2, P3, or P4, but no P1
					if (!(Joins[i]->Splits[s].Active || (!ns && s == 0) || (s == fs)))
						continue;
					
					// Create their XPlayer
					Joins[i]->ScreenToAdd = s;
					
					if (x < MAXPLAYERS * MAXSPLITSCREEN)
						XJoins[x++] = D_XNetAddPlayer(DS_JWJoins, Joins[i], false);
				}
			}
			
			// Save the game and put in temporary dir
				// Setup a name
			memset(Buf, 0, sizeof(Buf));
			memset(Mini, 0, sizeof(Mini));
			I_GetStorageDir(Buf, BUFSIZE - 24, DST_TEMP);
			strncat(Buf, "/", BUFSIZE);
			snprintf(Mini, MINISIZE - 1, "netg%04x.rsv", I_GetCurrentPID() & 0xFFFF);
			strncat(Buf, Mini, BUFSIZE);
			
				// Save the game!
			if (!P_SaveGameEx("Network Save", Buf, strlen(Buf), NULL, NULL))
			{
				// Guess we failed, guess it is time to kick the newcomers
				for (i = 0; i < x; i++)
					D_XNetKickPlayer(XJoins[i], "Failed to save game.", false);
				return;
			}
			
			// Send file away
			l_SaveBeingSent = 0;
			
			if (!D_XFPrepFile(Buf, &l_SaveBeingSent))
			{
				for (i = 0; i < x; i++)
					D_XNetKickPlayer(XJoins[i], "Failed to initialize file for sending.", false);
				return;
			}
			
			// Send file to each host
			for (i = 0; i < j; i++)
				if (!D_XFSendFile(l_SaveBeingSent, &Joins[i]->Addr, Joins[i]->Desc->RelBS, Joins[i]->Desc->StdBS))
					for (k = 0; k < x; k++)
						if (XJoins[k]->HostID == XJoins[i]->HostID)
						{
							D_XNetKickPlayer(XJoins[i], "Failed to send savegame.", false);
							XJoins[k] = NULL;
						}
		}
	}
#undef BUFSIZE
#undef MINISIZE
}

/*---------------------------------------------------------------------------*/

/* DS_DoMaster() -- Handles master connection */
static void DS_DoMaster(D_XDesc_t* const a_Desc)
{
#define BUFSIZE 64
	char Buf[BUFSIZE];
	bool_t IsServer;
	int32_t i;	
	
	/* Client in master mode */
	if (!D_XNetIsServer())
	{
		// Did not anti-connect yet
		if (!a_Desc->Data.Master.RemSent)
		{
			// Sent requst already?
			if (g_ProgramTic < a_Desc->Data.Master.LastAnti)
				return;
		
			// wait a second and print a message
			a_Desc->Data.Master.LastAnti = g_ProgramTic + TICRATE;
			CONL_OutputUT(CT_NETWORK, DSTR_DXP_WAITINGFORCONN, "\n");
		
			// Find endpoint, which will belong to client
			for (i = 0; i < g_NumXEP; i++)
				if (g_XEP[i])
					break;
		
			// Not found? No client connection yet
			if (g_NumXEP == 0 || i >= g_NumXEP)
				return;
			
			// Do not send multiple times
			if (!a_Desc->Data.Master.RemSent)
			{
				// Anti-Connect sync packet
				D_BSBaseBlock(a_Desc->RelBS, "CSYN");
			
				// Record Data
				D_BSwu8(a_Desc->RelBS, false);	// Not a server
				D_BSws(a_Desc->RelBS, "version=" REMOOD_FULLVERSIONSTRING);
				DS_DumpSplitInfo(a_Desc->RelBS);
			
				D_BSwu8(a_Desc->RelBS, 0);	// End of strings
			
				// Send to remote bound host
				D_BSRecordNetBlock(a_Desc->RelBS, &g_XEP[i]->Addr);
			
				// Only need to send request once
				a_Desc->Data.Master.RemSent = true;
			}
		}
		
		// Anti connected
		else
		{
		}
	}
	
	/* Server Mode */
	else
	{
	}
#undef BUFSIZE
}

/* DS_DoSlave() -- Handles slave connection */
static void DS_DoSlave(D_XDesc_t* const a_Desc)
{
#define BUFSIZE 64
	char Buf[BUFSIZE];
	bool_t IsServer;
	
	/* Not Syncronized? */
	if (!a_Desc->Data.Slave.Synced)
	{
		// Sent requst already?
		if (g_ProgramTic < a_Desc->Data.Slave.LastSyncReq)
			return;
		
		// Get server mode
		IsServer = D_XNetIsServer();
		
		// Build syncronization packet
		D_BSBaseBlock(a_Desc->StdBS, "GSYN");
		
		// Record Data
		D_BSwu8(a_Desc->StdBS, IsServer);	// We are server?
		
		D_BSws(a_Desc->StdBS, "version=" REMOOD_FULLVERSIONSTRING);
		
		// Master side won't care about our splits if we are the server!
		if (!IsServer)
			DS_DumpSplitInfo(a_Desc->StdBS);
		
		D_BSwu8(a_Desc->StdBS, 0);	// End of strings
		
		// Send to remote bound host
		D_BSRecordNetBlock(a_Desc->StdBS, &a_Desc->BoundTo);
		
		// Wait 1 second before resync
		a_Desc->Data.Slave.LastSyncReq = g_ProgramTic + TICRATE;
		
		// Print message to inform client
		I_NetHostToString(&a_Desc->BoundTo, Buf, BUFSIZE);
		CONL_OutputUT(CT_NETWORK, DSTR_DXP_CONNECTING, "%s\n", Buf);
	}
	
	/* Synced */
	else
	{
	}
#undef BUFSIZE
}

/* DS_DoServer() -- Handles server connection */
static void DS_DoServer(D_XDesc_t* const a_Desc)
{
	/* Handle join windows */
	DS_HandleJoinWindow(a_Desc);
}

/* DS_DoClient() -- Handles client connection */
static void DS_DoClient(D_XDesc_t* const a_Desc)
{
	D_XSyncLevel_t* LevelP;
	int32_t i, j, k;
	
	D_BS_t* RelBS, *StdBS;
	I_HostAddress_t* HostAddr;
	const WL_WADFile_t* WAD;
	
	/* Get pointer to the current level */
	LevelP = &a_Desc->CS.Client.SyncLevel;
	
	/* Obtain route to server */
	RelBS = D_XBRouteToServer(&StdBS, &HostAddr);
	
	/* How synchronization with server should resume */
	switch (*LevelP)
	{
			// Initialization
		case DXSL_INIT:
			// Requires route to connect first
			if (!RelBS)
				return;
			
			// Master -- If a route to the server exists, level up
			if (a_Desc->Master)
			{
				// Needs anti-connect
				if (a_Desc->Data.Master.AntiConnect)
					*LevelP += 1;
			}
			
			// Slave -- Otherwise, wait until we are connected
			else
			{
				if (a_Desc->Data.Slave.Synced)
					*LevelP += 1;
			}
			
			break;
			
			// Listing WAD files used by server
		case DXSL_LISTWADS:
			// Send request to server to check the WADs it is using
			if (!a_Desc->CS.Client.SentReqWAD)
			{
				// Send request to server
				D_BSBaseBlock(RelBS, "RQFL");
				
				// Send away!
				D_BSRecordNetBlock(RelBS, HostAddr);
				
				// Sent request
				a_Desc->CS.Client.SentReqWAD = true;
			}
			break;
			
			// Checking WADs
		case DXSL_CHECKWADS:
			// No WADs?
			if (!l_WADChecks || !l_WADChecks->Count)
			{
				D_XNetDisconnect(false);
				return;
			}
			
			// Go through WADs and 
			for (i = j = k = 0; i < l_WADChecks->Count; i++)
			{
				// Ignore remood.wad
				if (i == 1)
					continue;
				
				// Increase total count
				k++;
				
				// Try opening said WAD
				l_WADChecks[i].WAD = WL_OpenWAD(l_WADChecks[i].Long, l_WADChecks[i].Sum);
				
				// If long name failed, try DOS name
				if (!l_WADChecks[i].WAD)
					l_WADChecks[i].WAD = WL_OpenWAD(l_WADChecks[i].DOS, l_WADChecks[i].Sum);
				
				// Increase total OK count
				if (l_WADChecks[i].WAD)
					j++;
			}
			
			// Selected WADs OK?
			if (j >= k)
				*LevelP = DXSL_SWITCHWADS;
			
			// Otherwise, downloading required
			else
				*LevelP = DXSL_DOWNLOADWADS;
			break;
			
			// Download WADs that need downloading
		case DXSL_DOWNLOADWADS:
			I_Error("WAD Downloading currently not implemented.");
			break;
			
			// Switch to the specified WADs
		case DXSL_SWITCHWADS:
			// Get our current WAD order and place alongside list
			WAD = NULL;
			for (i = 0, j = 1; i < l_WADChecks->Count; i++)
			{
				if (!j)
					l_WADChecks[i].OrderWAD = NULL;
				else
				{
					l_WADChecks[i].OrderWAD = WAD = WL_IterateVWAD(WAD, true);
					
					// Bad WAD?
					if (!WAD)
						j = 0;
				}
			}
			
			// Expect no more WADs
			if (j)
			{
				WAD = WL_IterateVWAD(WAD, true);
				
				if (WAD)
					j = 0;
			}
			
			// See if the order is the same
			if (j)
				for (i = 0; i < l_WADChecks->Count; i++)
					if (i != 1)
						if (l_WADChecks[i].OrderWAD != l_WADChecks[i].WAD)
						{
							j = 0;
							break;
						}
			
			// Need to reload?
			if (!j)
			{
				// Lock OCCB and pop all wads
				WL_LockOCCB(true);
				while (WL_PopWAD())
					;
				
				// Go through list and push new WADs
				for (i = 0; i < l_WADChecks->Count; i++)
					if (i == 1)	// keep remood.wad
						WL_PushWAD(l_WADChecks[i].OrderWAD);
					else
						WL_PushWAD(l_WADChecks[i].WAD);
				
				// Unlock OCCB to change WADs
				WL_LockOCCB(false);
				
				// Free all WADs not currently open
				WL_CloseNotStacked();
			}
			
			// Go to savegame now
			*LevelP = DXSL_WAITFORWINDOW;
			break;
		
			// Wait for join window
		case DXSL_WAITFORWINDOW:
			// Ready and waiting for the join window
			if (!a_Desc->CS.Client.JoinWait)
			{
				// Send request to server
				D_BSBaseBlock(RelBS, "JRDY");
			
				// Send away!
				D_BSRecordNetBlock(RelBS, HostAddr);
				
				// Don't send message again
				a_Desc->CS.Client.JoinWait = true;
			}
			break;
		
			// Playing
		case DXSL_PLAYING:
			if (!a_Desc->CS.Client.SentReady)
			{
				// Send request to server
				D_BSBaseBlock(RelBS, "PLAY");
				D_BSwu32(RelBS, D_XNetGetHostID());
			
				// Send away!
				D_BSRecordNetBlock(RelBS, HostAddr);
				
				// Send ready
				a_Desc->CS.Client.SentReady = true;
			}
			break;
		
			// Unhandled
		default:
			break;
	}
}

/*---------------------------------------------------------------------------*/

/* DXP_GSYN() -- Game Synchronize Connection */
static bool_t DXP_GSYN(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
	uint8_t RemoteIsServer;
	bool_t IsServer;
	int32_t i;
	
	/* Read if the remote says it is a server */
	RemoteIsServer = D_BSru8(a_Desc->RelBS);
	IsServer = D_XNetIsServer();
	
	// Incompatible
	if (!a_Desc->Master || ((!!RemoteIsServer) == (!!IsServer)))
	{
		D_XPSendDisconnect(a_Desc, a_Desc->StdBS, a_Addr, "Remote end has same connection type gender.");
		return true;
	}
	
	/* If we are a client, only permit one endpoint */
	if (!IsServer)
	{	
		// Break on the first sight
		for (i = 0; i < g_NumXEP; i++)
			if (g_XEP[i])
				break;
		
		// Something is around
		if (i < g_NumXEP)
			return false;
	}
	
	/* Otherwise always bring in the client */
	return DS_BringInClient(a_Desc, a_Header, a_Flags, a_Addr, a_EP, a_Desc->RelBS);
}

/* DXP_CSYN() -- Client Synchronize Connection */
static bool_t DXP_CSYN(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
	uint8_t RemoteIsServer;
	bool_t IsServer;
	int32_t i;
	
	/* Read if the remote says it is a server */
	RemoteIsServer = D_BSru8(a_Desc->RelBS);
	IsServer = D_XNetIsServer();
	
	// Incompatible
	if (a_Desc->Master || ((!!RemoteIsServer) == (!!IsServer)))
	{
		D_XPSendDisconnect(a_Desc, a_Desc->StdBS, a_Addr, "Remote end has same connection type gender.");
		return true;
	}
	
	/* If we are a server, only permit one endpoint */
	if (IsServer)
	{	
		// Break on the first sight
		for (i = 0; i < g_NumXEP; i++)
			if (g_XEP[i])
				break;
		
		// Something is around
		if (i < g_NumXEP)
			return false;
	}
	
	/* Otherwise always bring in the client */
	return DS_BringInClient(a_Desc, a_Header, a_Flags, a_Addr, a_EP, a_Desc->RelBS);
}

/* DXP_DISC() -- Disconnect Received */
static bool_t DXP_DISC(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
#define BUFSIZE 72
	char Buf[BUFSIZE];
	uint32_t Code;
	
	/* Read Code */
	Code = D_BSru32(a_Desc->RelBS);
	D_BSrs(a_Desc->RelBS, Buf, BUFSIZE);
	
	/* Disconnect notice */
	CONL_OutputUT(CT_NETWORK, DSTR_DXP_DISCONNED, "%s\n", Buf);
	
	/* If playing... */
	if (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION)
	{
		// Detach from socket
		D_XBSocketDestroy();
		
		// Transform self into server (everyone went bye)
		D_XNetBecomeServer();
	}
	
	/* Otherwise, a network disconnect */
	else
	{
		// Cannot connect to server maybe?
		D_XNetDisconnect(false);
	}
	
	/* Success! */
	return true;
#undef BUFSIZE
}

/* DXP_SYNJ() -- Synchronization Accepted */
static bool_t DXP_SYNJ(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
	uint32_t HostID;
	
	/* Read out HostID */
	HostID = D_BSru32(a_Desc->RelBS);
	
	// Check for invalid address
	if (!HostID)
		return false;
	
	// Set address
	D_XNetSetHostID(HostID);
	
	/* Set as connected now */
	a_Desc->Data.Slave.Synced = true;
	
	/* Success! */
	return true;
}

/* DXP_SYNC() -- Anti-Synchronization Accepted */
static bool_t DXP_SYNC(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
	uint32_t HostID;
	
	/* Read out HostID */
	HostID = D_BSru32(a_Desc->RelBS);
	
	// Check for invalid address
	if (!HostID)
		return false;
	
	// Set address
	D_XNetSetHostID(HostID);
	
	/* Set as connected now */
	a_Desc->Data.Master.AntiConnect = true;
	
	/* Success! */
	return true;
}

/* DXP_RQFL() -- Request File List */
static bool_t DXP_RQFL(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
	const WL_WADFile_t* WAD;
	int i;
	
	/* Base packet */
	D_BSBaseBlock(a_Desc->RelBS, "WADS");
	
	/* Go through all WADs */
	WAD = NULL;
	while ((WAD = WL_IterateVWAD(WAD, true)))
	{
		// WAD Exists here
		D_BSwu8(a_Desc->RelBS, 1);
		
		// Write known names
		D_BSws(a_Desc->RelBS, WL_GetWADName(WAD, false));
		D_BSws(a_Desc->RelBS, WL_GetWADName(WAD, true));
		
		// Write MD5
		D_BSws(a_Desc->RelBS, WAD->CheckSumChars);
	}
	
	// End of list
	D_BSwu8(a_Desc->RelBS, 0);
	
	/* Send to remote end */
	D_BSRecordNetBlock(a_Desc->RelBS, a_Addr);
	
	/* Success! */
	return true;
}

/* DXP_WADS() -- Got list of WADs that server is using */
static bool_t DXP_WADS(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
#define BUFSIZE 48
	uint8_t Code;
	char LongName[BUFSIZE];
	char DOSName[WLMAXDOSNAME];
	char CheckSum[BUFSIZE];
	
	char* RealLong, *RealDOS;
	D_XWADCheck_t* Check;
	
	/* In wrong sync mode? */
	if (a_Desc->CS.Client.SyncLevel != DXSL_LISTWADS)
		return false;	
	
	/* Show Header */
	CONL_OutputUT(CT_NETWORK, DSTR_DXP_WADHEADER, "\n");
	
	/* Read from list */
	for (;;)
	{
		// Read Code
		Code = D_BSru8(a_Desc->RelBS);
		
		// End of WADs?
		if (!Code)
			break;
		
		// Read names and sums
		D_BSrs(a_Desc->RelBS, LongName, BUFSIZE);
		D_BSrs(a_Desc->RelBS, DOSName, WLMAXDOSNAME);
		D_BSrs(a_Desc->RelBS, CheckSum, BUFSIZE);
		
		// Get base names of WADs
			// Using explicit paths that will probably be very different is a
			// bad idea. Also, this would prevent possible hackery involved in
			// file transfers where one could replace system files through a
			// download done by malicious servers.
		RealLong = WL_BaseNameEx(LongName);
		RealDOS = WL_BaseNameEx(DOSName);
		
		// Check the extension, if it is not valid then disconnect!
			// This is to prevent nasties like having a remote server
			// fake a -file and name the file something like remood.exe so
			// that clients cannot be exposed to malicious servers.
		if (!WL_ValidExt(RealLong) || !WL_ValidExt(RealDOS))
		{
			CONL_OutputUT(CT_NETWORK, DSTR_DXP_BADWADEXT, "%s%s\n", RealLong, RealDOS);
			D_XNetDisconnect(false);
			return false;
		}
		
		// Show on console
		CONL_OutputUT(CT_NETWORK, DSTR_DXP_WADENTRY, "%s%s%s\n", RealLong, RealDOS, CheckSum);
		
		// Add to list
			// Nothing in list
		if (!l_WADChecks)
			Check = l_WADChecks = Z_Malloc(sizeof(*Check), PU_STATIC, NULL);
			
			// Something is there
		else
		{
			Z_ResizeArray((void**)&l_WADChecks, sizeof(*l_WADChecks),
				l_WADChecks[0].Count, l_WADChecks[0].Count + 1);
			Check = &l_WADChecks[l_WADChecks[0].Count];
		}
		
		// Increase check count
		l_WADChecks[0].Count++;
		
		// Fill in check data
		strncpy(Check->Long, RealLong, WADCHECKLEN - 1);
		strncpy(Check->DOS, RealDOS, WADCHECKLEN - 1);
		strncpy(Check->Sum, CheckSum, WADCHECKLEN - 1);
	}
	
	/* Move to next synchronization level */
	a_Desc->CS.Client.SyncLevel++;
	
	/* Success! */
	return true;
#undef BUFSIZE
}

/* DXP_JRDY() -- Client is ready for the join window */
static bool_t DXP_JRDY(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
#define BUFSIZE 72
	char Buf[BUFSIZE];
	
	/* Check */
	if (!a_EP)
		return false;
	
	/* Already signaled? */
	if (a_EP->SignalReady)
		return true;
	
	/* Otherwise, set as being ready to enter the true game */
	a_EP->SignalReady = true;
	a_EP->ReadyTime = g_ProgramTic;
	I_NetHostToString(a_Addr, Buf, BUFSIZE);
	CONL_OutputUT(CT_NETWORK, DSTR_DXP_CLIENTREADYWAIT, "%s\n", Buf);
	
	/* Success! */
	return true;
#undef BUFSIZE
}

/* DXP_PLAY() -- Player is ready */
static bool_t DXP_PLAY(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
	uint32_t HostID, i;
	D_XPlayer_t* XPlay;
	
	/* Read HostID */
	HostID = D_BSru32(a_Desc->RelBS);
	
	// Check
	if (!HostID)
		return false;
	
	/* Go through players */
	for (i = 0; i < g_NumXPlays; i++)
	{
		XPlay = g_XPlays[i];
		
		// Nothing?
		if (!XPlay)
			continue;
		
		// HostID and from address must match
		if (HostID != XPlay->HostID || !I_NetCompareHost(a_Addr, &XPlay->Socket.Address))
			continue;
		
		// Set as ready now
		XPlay->TransSave = true;
		XPlay->LastRanTic = gametic;
		
		// Message
		CONL_OutputUT(CT_NETWORK, DSTR_DXP_PLAYERISPLAYING, "%s%s\n", XPlay->DisplayName, XPlay->AccountName);
	}
	
	/* Success! */
	return true;
}

/* DXP_FILE() -- File Handling */
static bool_t DXP_FILE(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
	return D_XFFilePacket(a_Desc, a_Header, a_Flags, a_Addr, a_EP);
}

/* D_CSPackFlag_t -- Packet flags */
typedef enum D_CSPackFlag_e
{
	PF_MASTER			= UINT32_C(0x00000001),	// Need to be master
	PF_SLAVE			= UINT32_C(0x00000002),	// Need to be slave
	PF_SERVER			= UINT32_C(0x00000004),	// Need to be server
	PF_CLIENT			= UINT32_C(0x00000008),	// Need to be client
	PF_REL				= UINT32_C(0x00000010), // Must be reliable
	PF_NOREL			= UINT32_C(0x00000020),	// Must not be reliable
	PF_SYNCED			= UINT32_C(0x00000040),	// Remote must be synced (Slave)
	PF_ONAUTH			= UINT32_C(0x00000080),	// Authorized Source
} D_CSPackFlag_t;

/* c_CSPacks -- Client/Server Packets */
static const struct
{
	const char* Header;
	bool_t (*Func)(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP);
	uint32_t Flags;
} c_CSPacks[] =
{
	{"GSYN", DXP_GSYN, PF_MASTER},
	{"CSYN", DXP_CSYN, PF_SLAVE},
	{"DISC", DXP_DISC, PF_ONAUTH | PF_SLAVE},
	{"SYNJ", DXP_SYNJ, PF_ONAUTH | PF_SLAVE | PF_REL},
	{"SYNC", DXP_SYNC, PF_ONAUTH | PF_MASTER | PF_REL},
	{"RQFL", DXP_RQFL, PF_ONAUTH | PF_SERVER | PF_REL},
	{"WADS", DXP_WADS, PF_ONAUTH | PF_CLIENT | PF_REL},
	{"JRDY", DXP_JRDY, PF_ONAUTH | PF_SERVER | PF_REL},
	{"PLAY", DXP_PLAY, PF_ONAUTH | PF_SERVER | PF_REL},
	{"FILE", DXP_FILE, PF_ONAUTH},
	
	{NULL}
};

/* D_XPRunCS() -- Run client server operation */
void D_XPRunCS(D_XDesc_t* const a_Desc)
{
	char Header[5];
	int32_t i, b;
	I_HostAddress_t RemAddr;
	I_HostAddress_t* SvRoute;
	bool_t Continue;
	uint32_t Flags;
	bool_t Authed;
	D_XEndPoint_t* EP;
	
	/* Handle Files */
	D_XFHandleFiles();
	
	/* Handle standard state based packets */
	// Master
	if (a_Desc->Master)
		DS_DoMaster(a_Desc);
	
	// Slave
	else
		DS_DoSlave(a_Desc);
	
	// Server
	if (D_XNetIsServer())
		DS_DoServer(a_Desc);
	
	else
		DS_DoClient(a_Desc);
	
	/* Handle individual packets */
	do
	{
		// Disconected?
		if (!g_XSocket)
			break;
		
		// Reset
		memset(Header, 0, sizeof(Header));
		memset(&RemAddr, 0, sizeof(RemAddr));
		
		// Read Packet
		Continue = D_BSPlayNetBlock(a_Desc->RelBS, Header, &RemAddr);
		
		// Handle Packet
		if (Continue)
		{
			// Invalid IP?
			if (!D_XBValidIP(&RemAddr))
				continue;
			
			// Determine scenario packet is in
			Flags = 0;
			
				// Master/Slave
			if (a_Desc->Master)
				Flags |= PF_MASTER;
			else
			{
				Flags |= PF_SLAVE;
				
				// Packet MUST be from master side
				if (!I_NetCompareHost(&a_Desc->BoundTo, &RemAddr))
					continue;
			}
				
				// Server/Client
			if (D_XNetIsServer())
				Flags |= PF_SERVER;
			else
				Flags |= PF_CLIENT;
				
				// Reliable
			Authed = false;
			D_BSStreamIOCtl(a_Desc->RelBS, DRBSIOCTL_ISAUTH, (intptr_t)&Authed);
			if (Authed)
				Flags |= PF_REL;
			else
				Flags |= PF_NOREL;
				
				// On Auth List
			Authed = false;
			D_BSStreamIOCtl(a_Desc->RelBS, DRBSIOCTL_CHECKISONLIST, (intptr_t)&RemAddr);
			D_BSStreamIOCtl(a_Desc->RelBS, DRBSIOCTL_GETISONLIST, (intptr_t)&Authed);
			if (Authed)
				Flags |= PF_ONAUTH;
				
				// Synced
			if (!a_Desc->Master && a_Desc->Data.Slave.Synced)
				Flags |= PF_SYNCED;
			
			// Look in list
			for (i = 0; c_CSPacks[i].Header; i++)
			{
				// Diff Header?
				if (!D_BSCompareHeader(Header, c_CSPacks[i].Header))
					continue;
				
				// Check Flags
				for (b = 0; b < 31; b++)
					if (c_CSPacks[i].Flags & (1 << b))
						if ((c_CSPacks[i].Flags & (1 << b)) != (Flags & (1 << b)))
							break;
				
				// Mismatch?
				if (b < 31)
					continue;
				
				// Reference endpoint
				EP = D_XBEndPointForAddr(&RemAddr);
				
				// Call handler function
				if (c_CSPacks[i].Func)
					c_CSPacks[i].Func(a_Desc, Header, Flags, &RemAddr, EP);
				
				// Always break
				break;
			}
			
			// Not handled?
			if (devparm)
				if (!c_CSPacks[i].Header)
					CONL_PrintF("Unknown \'%c%c%c%c\'\n", Header[0], Header[1], Header[2], Header[3]);
		}
	} while (Continue);
	
	/* Flush reliable stream */
	// Otherwise nothing is xmitted/rcved
	if (g_XSocket)
		D_BSFlushStream(a_Desc->RelBS);
}

/* D_XPGotFile() -- Got file from someone */
void D_XPGotFile(D_XDesc_t* const a_Desc, const char* const a_Path, const char* const a_Sum, const uint32_t a_Size, I_HostAddress_t* const a_Addr, const tic_t a_TimeStart, const tic_t a_TimeEnd)
{
	tic_t DlTime;
	uint32_t BpS, BpT;
	char* Name, *Ext;
	
	/* Check */
	if (!a_Path || !a_Sum || !a_Size || !a_Addr)
		return;
	
	/* Message */
	DlTime = a_TimeEnd - a_TimeStart;
	if (DlTime < 1)
		DlTime = 1;
	BpT = a_Size / DlTime;
	BpS = BpT / TICRATE;
	
	/* Detect extension */
	Name = WL_BaseNameEx(a_Path);
	Ext = strrchr(Name, '.');
	
	// No extension found?
	if (!Ext)
		return;
		
	/* Save Game */
	if (!strcasecmp(Ext, ".rsv"))
	{
		// Load Game
		if (!P_LoadGameEx(NULL, a_Path, strlen(a_Path), NULL, NULL))
		{
			CONL_OutputUT(CT_NETWORK, DSTR_DXP_BADSAVELOAD, "%s\n", Name);
			D_XNetDisconnect(false);
			return;
		}
		
		// Set as playing now if waiting for window
		if (a_Desc->CS.Client.SyncLevel == DXSL_WAITFORWINDOW)
			a_Desc->CS.Client.SyncLevel = DXSL_PLAYING;
	}
	
	/* WAD File */
	else if (WL_ValidExt(a_Path))
	{
	}
	
	/* Unknown */
	else
	{
	}
}

/*---------------------------------------------------------------------------*/

