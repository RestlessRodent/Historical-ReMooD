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
#include "p_saveg.h"
#include "g_game.h"
#include "p_demcmp.h"
#include "st_stuff.h"

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

bool_t g_LockJW = false;						// Lock the join window

extern CONL_StaticVar_t l_SVJoinWindow;
extern CONL_StaticVar_t l_SVLagStat;
extern CONL_StaticVar_t l_SVMaxClients;

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
	if (!a_BS || !a_Addr)
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
	
	// Flush, just in case (so it is sent before they go bye)
	D_BSFlushStream(a_BS);
	
	/* Drop host too */
	D_XBDropHost(a_Addr);
}

/* D_XPRequestScreen() -- Request Screen */
void D_XPRequestScreen(const int32_t a_ScreenID)
{
	/* Range Cap? */
	if (a_ScreenID < 0 || a_ScreenID >= MAXSPLITSCREEN)
		return;
}

/* D_XPCalcConnection() -- Calculates Connection */
void D_XPCalcConnection(D_XEndPoint_t* const a_EP)
{
	int32_t Hit, Missed;
	int32_t Total, i, bn;
	D_XNetLagStat_t* Stat;
	uint32_t MSBits[NUMLAGSTATPROBES];
	uint32_t CLBits[NUMLAGSTATPROBES];
	
	/* Check */
	if (!a_EP)
		return;
	
	/* Init */
	Stat = &a_EP->LagStat;
	bn = 0;
	memset(MSBits, 0, sizeof(MSBits));
	
	/* Calculate Total */
	if (a_EP->LagNLSP < NUMLAGSTATPROBES)
		Total = a_EP->LagNLSP;
	else
		Total = NUMLAGSTATPROBES;
	
	// Sent cap?
	if (a_EP->LagSent < Total)
		Total = a_EP->LagSent;
	
	/* Go through and find hit/miss, high low */
	Hit = Missed = 0;
	for (i = 0; i < Total; i++)
	{
		// If hit, calculate 
		if (Stat->Probes[i].Got)
		{
			// yay!
			Hit++;
			
			// Add to ms counts
			CLBits[bn] = Stat->Probes[i].recvMS;
			MSBits[bn++] = Stat->Probes[i].sendMS;
		}
		
		// Missed (packet loss)
		else
			Missed++;
	}
	
	/* Fill info */
	a_EP->NetSpeed.PacketGain = FixedMul(FixedDiv(Hit << FRACBITS, Total << FRACBITS), 100 << FRACBITS) >> FRACBITS;
	a_EP->NetSpeed.PacketLoss = FixedMul((1 << FRACBITS) - a_EP->NetSpeed.PacketGain, 100 << FRACBITS) >> FRACBITS;
	
	CONL_PrintF("gain: %i, loss: %i\n", a_EP->NetSpeed.PacketGain, a_EP->NetSpeed.PacketLoss);
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
	
	/* Determine max clients limit */
	for (s = i = 0; i < g_NumXEP; i++)
		if (g_XEP[i])
			s++;
	
	// Passed max?
	if (s >= l_SVMaxClients.Value->Int)
	{
		D_XPSendDisconnect(a_Desc, a_Desc->StdBS, a_Addr, "The number of permitted clients has been exceeded.");
		return false;
	}
	
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
	bool_t FreeToJoin;
	
	/* Players waiting in join window? */
	FreeToJoin = true;
	for (i = 0; i < g_NumXEP; i++)
		if (g_XEP[i])
			if (g_XEP[i]->ActiveJoinWindow)
			{
				FreeToJoin = false;
				break;
			}
	
	/* Perform join windows? */
	// Never join mid-tic
	if (!g_LockJW && FreeToJoin && g_ProgramTic >= a_Desc->CS.Server.JoinWindowTime)
	{
		// Clear joins
		memset(Joins, 0, sizeof(Joins));
		memset(XJoins, 0, sizeof(XJoins));
		
		// Go through all clients
		j = x = 0;
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
					{
						XJoins[x] = D_XNetAddPlayer(DS_JWJoins, Joins[i], false);
						
						// Set their gametic
						if (XJoins[x])
							XJoins[x]->LastRanTic = gametic;
						
						x++;
					}
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
			{
				if (!D_XFSendFile(l_SaveBeingSent, &Joins[i]->Addr, Joins[i]->Desc->RelBS, Joins[i]->Desc->StdBS))
				{
					for (k = 0; k < x; k++)
						if (XJoins[k]->HostID == XJoins[i]->HostID)
						{
							D_XNetKickPlayer(XJoins[i], "Failed to send savegame.", false);
							XJoins[k] = NULL;
						}
				}
				
				// Set endpoint inside of a join window
				else
					Joins[i]->ActiveJoinWindow = true;
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
	int32_t i;
	D_XEndPoint_t* EP;
	
	/* Handle join windows */
	DS_HandleJoinWindow(a_Desc);
	
	/* For each connection */
	for (i = 0; i < g_NumXEP; i++)
	{
		// Check endpoint
		EP = g_XEP[i];
		
		if (!EP)
			continue;
		
		// Begin sending new stat?
		if (g_ProgramTic >= EP->LastLagStat)
		{
			// Reset fields
			EP->LagNextBit = 0;
			EP->LagBitNum = 0;
			EP->LastLagStat = 0;
			EP->LagSent = 0;
			
			memset(&EP->LagStat, 0, sizeof(EP->LagStat));
			
			while (EP->LagStatID == 0)
				EP->LagStatID = D_CMakePureRandom();
			
			// Next stat will be in difference specified
			EP->LastLagStat = g_ProgramTic + (l_SVLagStat.Value->Int * TICRATE * 60);
		}
		
		// Send individual stat thingies on shitty channel
		if (g_ProgramTic >= EP->LagNextBit)
		{
			// Need to send bits
			if (EP->LagBitNum < NUMLAGSTATPROBES)
			{
				D_BSBaseBlock(a_Desc->StdBS, "LAGP");
				
				D_BSwu32(a_Desc->StdBS, EP->LagStatID);
				D_BSwu8(a_Desc->StdBS, EP->LagBitNum);
				D_BSwu64(a_Desc->StdBS, g_ProgramTic);
				D_BSwu32(a_Desc->StdBS, I_GetTimeMS());
				
				D_BSRecordNetBlock(a_Desc->StdBS, &EP->Addr);
				
				// Increment
				EP->LagBitNum++;
				EP->LagSent++;
			}
			
			// Request lag stat from client, to determine lag stuff
			else if (EP->LagBitNum == NUMLAGSTATPROBES)
			{
				D_BSBaseBlock(a_Desc->RelBS, "LAGR");
				
				D_BSRecordNetBlock(a_Desc->RelBS, &EP->Addr);
				
				// Increment
				EP->LagBitNum++;
			}
			
			// Increase time by a second
			EP->LagNextBit = g_ProgramTic + (TICRATE >> 1);
		}
	}
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
				a_Desc->CS.Client.JRTime = g_ProgramTic + TICRATE;
			}
			
			// Remove ready time?
			if (g_ProgramTic >= a_Desc->CS.Client.JRTime)
				a_Desc->CS.Client.JoinWait = false;
			break;
		
			// Playing
		case DXSL_PLAYING:
			if (!a_Desc->CS.Client.GotInGame)
			{
				if (!a_Desc->CS.Client.SentReady)
				{
					// Send request to server
					D_BSBaseBlock(RelBS, "PLAY");
					D_BSwu32(RelBS, D_XNetGetHostID());
		
					// Send away!
					D_BSRecordNetBlock(RelBS, HostAddr);
			
					// Send a second later, in case of drop
					a_Desc->CS.Client.SentReady = true;
					a_Desc->CS.Client.SRTime = g_ProgramTic + TICRATE;
				}
			
				// Remove sent ready?
				if (g_ProgramTic >= a_Desc->CS.Client.SRTime)
				{
					a_Desc->CS.Client.SentReady = false;
					a_Desc->CS.Client.SRTime = 0;
				}
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
	uint32_t Code, i;
	
	/* Read Code */
	Code = D_BSru32(a_Desc->RelBS);
	D_BSrs(a_Desc->RelBS, Buf, BUFSIZE);
	
	/* If slave, disconnect */
	if (!a_Desc->Master)
	{
		CONL_OutputUT(CT_NETWORK, DSTR_DXP_DISCONNED, "%s\n", Buf);
		D_XNetDisconnect(false);
	}
	
	/* We are server */
	else if (D_XNetIsServer())
	{
		// Remove endpoints if any
		if (a_EP)
		{
			// Kick XPlayers with said reason
			for (i = 0; i < g_NumXPlays; i++)
				if (g_XPlays[i])
					if (g_XPlays[i]->HostID == a_EP->HostID)
						D_XNetKickPlayer(g_XPlays[i], Buf, false);
			
			// Remove endpoint
			//D_XBDelEndPoint(a_EP, Buf);
		}
	}
	
	/* We are client */
	else
	{
		// Disconnect from server
		CONL_OutputUT(CT_NETWORK, DSTR_DXP_DISCONNED, "%s\n", Buf);
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
	bool_t AlreadyIn;
	
	/* No endpoint? */
	if (!a_EP)
		return false;
	
	/* Read HostID */
	HostID = D_BSru32(a_Desc->RelBS);
	
	// Check
	if (!HostID || HostID != a_EP->HostID)
		return false;
	
	/* Already latched? */
	AlreadyIn = false;
	if (a_EP->Latched)
		AlreadyIn = true;
	
	/* Clear active join from end point */
	if (!AlreadyIn)
	{
		a_EP->ActiveJoinWindow = false;
		
		// Also set as latched
		a_EP->Latched = true;
	}
	
	/* Go through players */
	for (i = 0; i < g_NumXPlays; i++)
	{
		XPlay = g_XPlays[i];
		
		// Nothing?
		if (!XPlay)
			continue;
		
		// HostID must match
		if (a_EP->HostID != XPlay->HostID)
			continue;
		
		// Set as ready now
		XPlay->TransSave = true;
		
		// Message
		if (!AlreadyIn)
			CONL_OutputUT(CT_NETWORK, DSTR_DXP_PLAYERISPLAYING, "%s%s\n", XPlay->DisplayName, XPlay->AccountName);
		
		// Send in game message
		D_BSBaseBlock(a_Desc->RelBS, (AlreadyIn ? "AIGM" : "NIGM"));
		D_BSRecordNetBlock(a_Desc->RelBS, a_Addr);
	}
	
	/* Success! */
	return true;
}

/* DXP_FILE() -- File Handling */
static bool_t DXP_FILE(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
	return D_XFFilePacket(a_Desc, a_Header, a_Flags, a_Addr, a_EP);
}

/* DXP_TICS() -- Handling of tic commands from server */
static bool_t DXP_TICS(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
	D_XNetTicBuf_t TicBuf;
	D_XNetTicBuf_t* To;
	D_BS_t* BS;
	I_HostAddress_t* AddrP;
	uint8_t* EncDP;
	uint32_t EncSP;
	
	/* Decode Data */
	memset(&TicBuf, 0, sizeof(TicBuf));
	
	// Got some tics
	a_Desc->CS.Client.GotTics = true;
	
	// Read Server Timing
	a_Desc->CS.Client.SvLastRanTic = D_BSrcu64(a_Desc->RelBS);
	a_Desc->CS.Client.SvLastXMit = D_BSrcu64(a_Desc->RelBS);
	a_Desc->CS.Client.SvLastAckTic = D_BSrcu64(a_Desc->RelBS);
	a_Desc->CS.Client.SvProgTic = D_BSrcu64(a_Desc->RelBS);
	a_Desc->CS.Client.SvMilli = D_BSru32(a_Desc->RelBS);
	
	// Only of valid size, no negative
	if (a_Desc->RelBS->BlkSize > a_Desc->RelBS->ReadOff)
		if (!D_XNetDecodeTicBuf(&TicBuf, a_Desc->RelBS->BlkData + a_Desc->RelBS->ReadOff, a_Desc->RelBS->BlkSize - a_Desc->RelBS->ReadOff))
			CONL_PrintF("Tic buffer checksum mismatch on tic %u!\n", (unsigned)gametic);
	
	/* Don't need? */
	if (TicBuf.GameTic < gametic)
		return false;
	
	/* Create tic command to write to */
	To = D_XNetBufForTic(TicBuf.GameTic, true);
	
	/* Copy tic data there */
	// Already got tick
	if (To->GotTic)
		return false;
	
	// Copy over
	memmove(To, &TicBuf, sizeof(*To));
	To->GotTic = true;
	
	/* Reply to server that we got it */
	a_Desc->CS.Client.ClLastAckTic = g_ProgramTic;
	
	BS = D_XBRouteToServer(NULL, &AddrP);
	
	D_BSBaseBlock(BS, "TACK");
	
	D_BSwcu64(BS, To->GameTic);
	D_BSwcu64(BS, gametic);
	D_BSwcu64(BS, g_ProgramTic);
	D_BSwcu64(BS, a_Desc->CS.Client.SvLastRanTic);
	D_BSwcu64(BS, a_Desc->CS.Client.SvLastXMit);
	D_BSwcu64(BS, a_Desc->CS.Client.SvLastAckTic);
	D_BSwcu64(BS, a_Desc->CS.Client.SvProgTic);
	D_BSwu32(BS, a_Desc->CS.Client.SvMilli);
	D_BSwu32(BS, I_GetTimeMS());
	
	// Possibly encode all local screen tics
	D_XPPossiblyEncodeTics(BS);
	
	D_BSRecordNetBlock(BS, a_Addr);
	
	/* Success! */
	return true;
}

/* DXP_TACK() -- Client acknowledged they recieved a tic */
static bool_t DXP_TACK(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
	D_XPlayer_t* XPlay, *Target;
	tic_t AckTic;
	uint32_t MsTime;
	D_XNetTicBuf_t TicBuf;
	int32_t i, j;
	
	uint32_t EncSize;
	static uint8_t* Buf;
	static size_t BufSize;
	
	/* Check */
	if (!a_EP)
		return false;
	
	/* Get XPlayer for this host */
	XPlay = D_XNetPlayerByHostID(a_EP->HostID);
	
	// Not found?
	if (!XPlay)
		return false;
	
	/* Read */
	AckTic = D_BSrcu64(a_Desc->RelBS);
	a_EP->ClGameTic = D_BSrcu64(a_Desc->RelBS);
	a_EP->ClProgTic = D_BSrcu64(a_Desc->RelBS);
	a_EP->PongLRT = D_BSrcu64(a_Desc->RelBS);
	a_EP->PongLXM = D_BSrcu64(a_Desc->RelBS);
	a_EP->PongLAT = D_BSrcu64(a_Desc->RelBS);
	a_EP->PongPT = D_BSrcu64(a_Desc->RelBS);
	
	MsTime = D_BSru32(a_Desc->RelBS);
	if (MsTime > a_EP->PongMS || a_EP->PongMS >= MsTime + 300000)
		a_EP->PongMS = MsTime;
	
	a_EP->ClMS = D_BSru32(a_Desc->RelBS);
	
	/* Calculate Player Ping */
	D_XNetCalcPing(XPlay);
	
	/* Read Commands (might be duped, oh well) */
	// Read and mask
	j = D_BSru32(a_Desc->RelBS);
	
	// Only if there is a mask
	if (j)
	{
		// Read encoding size
		EncSize = D_BSru16(a_Desc->RelBS);
	
		// Only if there is data encoded
		if (EncSize)
		{
			// Setup buffer for buffer read
			if (EncSize > BufSize)
			{
				if (Buf)
					Z_Free(Buf);
				Buf = Z_Malloc(EncSize + 512, PU_STATIC, NULL);
				BufSize = EncSize;
			}
	
			// Read Buffer
			memset(Buf, 0, BufSize + 512);
			D_BSReadChunk(a_Desc->RelBS, Buf, EncSize);
	
			// Decode
			memset(&TicBuf, 0, sizeof(TicBuf));
			D_XNetDecodeTicBuf(&TicBuf, Buf, EncSize);
	
			// Go through masked players
			for (i = 0; i < MAXPLAYERS; i++)
			{
				// No mask for this player
				if (!(j & (1 << i)))
					continue;	
				
				// Not in game?
				if (!playeringame[i])
					continue;
		
				// Find target player (their X buffer)
				Target = players[i].XPlayer;
		
				// No target?
				if (!Target)
					continue;
		
				// Illegal data change (can't tic another player)
				if (Target->HostID != XPlay->HostID)
					continue;
				
				// Append to end
				if (Target->LocalAt < MAXLBTSIZE - 1)
				{
					memmove(&Target->LocalBuf[Target->LocalAt], &TicBuf.Tics[i], sizeof(ticcmd_t));
					Target->LocalAt++;
				}
			}
		}
	}
	
	/* Already acked this tic? */
	if (AckTic <= XPlay->LastAckTic)
		return true;
	
	/* Set as acknowledged */
	if (AckTic > XPlay->LastAckTic)
		XPlay->LastAckTic = AckTic;
	
	/* Success! */
	return true;
}

/* DXP_TRUN() -- Server acknowledged they recieved a tic */
static bool_t DXP_TRUN(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
#define BUFSIZE 72
	D_XPlayer_t* XPlay;
	tic_t GameTic;
	uint32_t SyncCode, i;
	D_XNetTicBuf_t* TicBuf;
	char Buf[BUFSIZE];
	
	/* Check */
	if (!a_EP)
		return false;
	
	/* Get XPlayer for this host */
	XPlay = D_XNetPlayerByHostID(a_EP->HostID);
	
	// Not found?
	if (!XPlay)
		return false;
	
	/* Read */
	GameTic = D_BSrcu64(a_Desc->RelBS);
	SyncCode = D_BSru32(a_Desc->RelBS);
	
	/* Already ran this tic? */
	if (GameTic <= XPlay->LastRanTic)
		return false;
	
	/* Check syncs */
	if (GameTic > XPlay->LastRanTic)
		XPlay->LastRanTic = GameTic;
	
	// Obtain the tic buffer for this tic
	TicBuf = D_XNetBufForTic(GameTic, false);
	
	// No tic buffer or mismatch?
	if (!TicBuf || (TicBuf && SyncCode != TicBuf->SyncCode))
	{
		// Setup message
		memset(Buf, 0, sizeof(Buf));
		
		if (TicBuf)
			snprintf(Buf, BUFSIZE - 1, "Game State Desync (%08x != %08x @ %u",
					SyncCode, TicBuf->SyncCode, ((unsigned int)GameTic)
				);
		else if (GameTic > gametic)
			snprintf(Buf, BUFSIZE - 1, "In the future (%u > %u)",
					((unsigned int)GameTic), ((unsigned int)gametic)
				);
		else
			snprintf(Buf, BUFSIZE - 1, "Out of Reality (@ %u)",
					((unsigned int)GameTic)
				);
		
		// Kick all associated XPlayers
		for (i = 0; i < g_NumXPlays; i++)
		{
			// Get XPlayer
			XPlay = g_XPlays[i];
			
			if (!XPlay)
				continue;
			
			// Not this host
			if (XPlay->HostID != a_EP->HostID)
				continue;
			
			// Kick em
			D_XNetKickPlayer(XPlay, Buf, false);
		}
		
		// I'd say it failed
		return false;
	}
	
	/* Success! */
	return true;
#undef BUFSIZE
}

/* DXP_TRYJ() -- Player wants to try to join the game */
static bool_t DXP_TRYJ(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
#define BUFSIZE 128
	char Buf[BUFSIZE];
	D_XPlayer_t* XPlay;
	uint32_t ClientID;
	
	/* Check */
	if (!a_EP)
		return false;
	
	/* Read client ID for the player that does want to join */
	ClientID = D_BSru32(a_Desc->RelBS);
	
	// Find XPlayer for this ID
	XPlay = D_XNetPlayerByID(ClientID);
	
	// No player?
	if (!XPlay)
		return false;
	
	/* Unauthorized? */
	if (XPlay->HostID != a_EP->HostID)
		return false;
	
	/* Already playing? */
	if (XPlay->Player)
		return false;
	
	/* Otherwise read join data */
	// Place that info into that player, in case it does happen to be missing
		// Display Name
	memset(Buf, 0, sizeof(Buf));
	D_BSrs(a_Desc->RelBS, Buf, BUFSIZE - 1);
	D_XNetPlayerPref(XPlay, false, DXPP_DISPLAYNAME, Buf);
	
		// Hexen Class
	memset(Buf, 0, sizeof(Buf));
	D_BSrs(a_Desc->RelBS, Buf, BUFSIZE - 1);
	D_XNetPlayerPref(XPlay, false, DXPP_HEXENCLASS, Buf);
	
		// Color
	D_XNetPlayerPref(XPlay, false, DXPP_SKINCOLOR, D_BSru32(a_Desc->RelBS));
	
		// VTeam
	D_XNetPlayerPref(XPlay, false, DXPP_VTEAM, D_BSru32(a_Desc->RelBS));
	
	/* Attempt actual join */
	D_XNetTryJoin(XPlay);
	return true;
#undef BUFSIZE
}

/* DXP_TREQ() -- Requesting tics from the server */
static bool_t DXP_TREQ(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
	tic_t GameTic;
	D_XNetTicBuf_t* Buf;
	D_XPlayer_t* XPlay;
	
	/* Check */
	if (!a_EP)
		return false;
	
	/* Obtain XPlayer */
	XPlay = D_XNetPlayerByHostID(a_EP->HostID);
	
	// Failed
	if (!XPlay)
		return false;
	
	/* Read gametic they want */
	GameTic = D_BSrcu64(a_Desc->RelBS);
	
	/* Obtain that tic, if it hopefully exists */
	Buf = D_XNetBufForTic(GameTic, false);
	
	// Nope!
	if (!Buf)
		return false;
	
	/* Hopefully this works */
	// Reset their ACKs
	if (GameTic < XPlay->LastXMit)
		XPlay->LastXMit = GameTic;
	
	if (GameTic < XPlay->LastAckTic)
		XPlay->LastAckTic = GameTic;
	
	// Send the tic over
	D_XNetSendTicToHost(Buf, XPlay);
	return true;
}

extern CONL_StaticVar_t l_SVName;

/* DXP_INFO() -- Information Request */
static bool_t DXP_INFO(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
	D_BS_t* BS;
	bool_t IsServer;
	uint32_t GameID;
	const WL_WADFile_t* Rover;
	int32_t i, j;
	D_XPlayer_t* XPlay;
	player_t* Player;
	uint8_t Bits;
	
	/* Read Game ID */
	GameID = D_BSru32(a_Desc->RelBS);
	
	// Do not permit game ID queries of non-zero
		// These are reserved for the proxy server
	if (GameID > 0)
		return false;
	
	/* Reply on standard stream */
	BS = a_Desc->StdBS;
	
	IsServer = D_XNetIsServer();
	
	/* Build game information */
	// Base Info
	D_BSBaseBlock(BS, "IXBI");
	
	D_BSwu8(BS, (IsServer ? 0x01 : 0x00));		// More bits for later usage
	D_BSws(BS, l_SVName.Value->String);			// Name of server
	
	D_BSRecordNetBlock(BS, a_Addr);
	
	// Done if wer are not a server
	if (!IsServer)
		return true;
	
	// Settings (bits in giant array)
	D_BSBaseBlock(BS, "IXST");
	
	for (i = 0; i < 256; i++)
		D_BSwi32(BS, P_XGSVal(i));
	
	D_BSRecordNetBlock(BS, a_Addr);
	
	// Players
	for (j = 0, i = 0, GameID = D_CMakePureRandom(); i <= g_NumXPlays; i++)
	{
		XPlay = g_XPlays[i];
		
		// Init new block?
		if (i >= j || i >= g_NumXPlays)
		{
			if (j > 0)
				D_BSRecordNetBlock(BS, a_Addr);
			
			if (i >= g_NumXPlays)
				break;
			
			j = i + 8;
			
			D_BSBaseBlock(BS, "IXPL");
			D_BSwu32(BS, GameID);
			D_BSwu16(BS, MAXPLAYERS);
			D_BSwu8(BS, g_NumXPlays);
			D_BSwu8(BS, i);
			D_BSwu8(BS, j);
		}
		
		if (XPlay)
		{
			// Direct flags
			D_BSwu8(BS, 1);
			D_BSwu32(BS, XPlay->Flags);
		
			// Stuff
			D_BSws(BS, D_XNetGetPlayerName(XPlay));
			D_BSws(BS, XPlay->LoginUUID);
			D_BSwi8(BS, XPlay->ScreenID);
			D_BSwi16(BS, XPlay->InGameID);
			D_BSwu16(BS, XPlay->Ping);
		
			// If in game, in game stuff
			if (XPlay->InGameID >= 0 && XPlay->InGameID <= MAXPLAYERS)
			{
				Player = &players[XPlay->InGameID];
			
				Bits = 0;
			
				if (Player->playerstate == PST_DEAD)
					Bits |= 0x01;
				if (Player->CounterOpPlayer)
					Bits |= 0x02;
			
				D_BSwu8(BS, Bits);
				D_BSwi16(BS, Player->killcount);
				D_BSwi16(BS, Player->itemcount);
				D_BSwi16(BS, Player->secretcount);
				D_BSwi32(BS, ST_PlayerFrags(Player - players));
				D_BSwi32(BS, Player->TotalFrags);
				D_BSwi32(BS, Player->TotalDeaths);
				D_BSwi32(BS, 0);	// Reserved for Score
				D_BSwu8(BS, Player->skincolor);
				D_BSwu8(BS, P_GetMobjTeam(Player->mo));
			}
		}
	}
	
	// WADs
	D_BSBaseBlock(BS, "IXWD");
	
	for (i = 0, Rover = WL_IterateVWAD(NULL, true); Rover; i++, Rover = WL_IterateVWAD(Rover, true))
	{
		// Ignore remood.wad
		if (i == 1)
			continue;
		
		// Flags for future use (first bit indicates WAD is here)
		D_BSwu8(BS, 1);
		
		// Names of WAD
		D_BSws(BS, WL_GetWADName(Rover, false));
		D_BSws(BS, WL_GetWADName(Rover, true));
		
		// Sum
		D_BSws(BS, Rover->CheckSumChars);
	}
	
	// End of list
	D_BSwu8(BS, 0);
	
	D_BSRecordNetBlock(BS, a_Addr);
	
	/* Success! */
	return true;
}

/* DXP_PREF() -- Preference Change */
static bool_t DXP_PREF(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
#define BUFSIZE (MAXPLAYERNAME + MAXPLAYERNAME + 1)
	int32_t i;
	D_XPlayer_t* XPlay;
	
	uint32_t ID;
	uint16_t Code;
	uint8_t Type;
	int32_t IntVal;
	char StrVal[BUFSIZE];
	
	/* Check */
	if (!a_EP)
		return false;
	
	/* Load Packet */
	// Clear buffer
	memset(StrVal, 0, sizeof(StrVal));
	
	ID = D_BSru32(a_Desc->RelBS);
	Code = D_BSru16(a_Desc->RelBS);
	Type = D_BSru8(a_Desc->RelBS);
	
	/* Which type? */
	switch (Type)
	{
			// Integer
		case 0:
			IntVal = D_BSri32(a_Desc->RelBS);
			break;
		
			// String
		case 1:
			D_BSrs(a_Desc->RelBS, StrVal, BUFSIZE - 1);
			break;
		
			// Unknown
		default:
			return false;
	}
	
	/* Find player by ID */
	XPlay = D_XNetPlayerByID(ID);
	
	// Confirm player
	if (!XPlay || XPlay->HostID != a_EP->HostID)
		return false;
	
	/* Call function */
	D_XNetPlayerPref(XPlay, false, Code, (Type == 1 ? (intptr_t)StrVal : IntVal));
	return true;
#undef BUFSIZE
}

/* DXP_NIGM() -- Now In Game */
static bool_t DXP_NIGM(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
	if (a_Desc->CS.Client.SyncLevel == DXSL_PLAYING)
		a_Desc->CS.Client.GotInGame = true;
	return true;
}

/* DXP_CHAT() -- Chat Message */
static bool_t DXP_CHAT(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
#define BUFSIZE 128
	uint32_t Source, Target;
	uint8_t Mode;	
	D_XPlayer_t* XPlay, *XDest;
	char Buf[BUFSIZE];
	int32_t Len;
	uint32_t ChatID;
	
	/* Check */
	if (!a_EP)
		return false;
	
	/* Read Info */
	Source = D_BSru32(a_Desc->RelBS);
	Mode = D_BSru8(a_Desc->RelBS);
	Target = D_BSru32(a_Desc->RelBS);
	ChatID = D_BSru32(a_Desc->RelBS);
	
	memset(Buf, 0, sizeof(Buf));
	D_BSrs(a_Desc->RelBS, Buf, BUFSIZE);
	
	// Get source player
	XPlay = D_XNetPlayerByID(Source);
	XDest = D_XNetPlayerByID(Target);
	
	// Wrong host? indiv and no target?
	if ((XPlay->HostID != a_EP->HostID) || (Mode == 3 && !XDest))
		return false;
	
	// Already sent this or older message?
	if (ChatID <= XPlay->ChatID)
		return false;
	
	// Length of current message
	Len = strlen(Buf);
	
	// Needs cooling down?
	if (g_ProgramTic + Len < XPlay->ChatCoolDown)
		return false;
	
	// Set cooldown to length of string
	XPlay->ChatCoolDown = g_ProgramTic + Len;
	XPlay->ChatID = ChatID;	// to prevent same message spam due to lag
	
	/* Direct encode */
	D_XNetDirectChatEncode(Source, Mode, Target, Buf);
	
	/* It worked */
	return true;
#undef BUFSIZE
}

/* DXP_LAGP() -- Lag Probe */
static bool_t DXP_LAGP(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
	D_XNetLagStat_t* Stat;
	uint32_t ID, MS;
	uint8_t Num;
	tic_t Tic;
	
	tic_t OurPT;
	uint32_t OurMS;
	
	/* Obtain stat */
	Stat = &a_Desc->CS.Client.OurLagStat;
	
	// Read in
	ID = D_BSru32(a_Desc->RelBS);
	Num = D_BSru8(a_Desc->RelBS);
	Tic = D_BSru64(a_Desc->RelBS);
	MS = D_BSru32(a_Desc->RelBS);
	
	OurPT = g_ProgramTic;
	OurMS = I_GetTimeMS();
	
	/* If different ID, purge */
	if (Stat->ID != ID)
		memset(Stat, 0, sizeof(*Stat));
	
	/* Set fields */
	// ID and count
	Stat->ID = ID;
	
	// Probe number
	if (Num >= 0 && Num < NUMLAGSTATPROBES)
		if (!Stat->Probes[Num].Got)	// once is enough
		{
			// Got it now
			Stat->Probes[Num].Got = true;
			
			// Set inners
			Stat->Probes[Num].sendPTic = Tic;
			Stat->Probes[Num].sendMS = MS;
			Stat->Probes[Num].recvPTic = OurPT;
			Stat->Probes[Num].recvMS = OurMS;
			
			// Increase count
			Stat->Count++;
		}
	
	return true;
}

/* DXP_LAGR() -- Request lag probe results */
static bool_t DXP_LAGR(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
	int32_t i;
	D_XNetLagStat_t* Stat;
	
	/* Obtain stat */
	Stat = &a_Desc->CS.Client.OurLagStat;
	
	/* Send back to server */
	D_BSBaseBlock(a_Desc->RelBS, "LAGS");
	
	D_BSwu32(a_Desc->RelBS, Stat->ID);
	D_BSwu8(a_Desc->RelBS, Stat->Count);
	D_BSwu8(a_Desc->RelBS, NUMLAGSTATPROBES);
	
	for (i = 0; i < NUMLAGSTATPROBES; i++)
	{
		D_BSwu8(a_Desc->RelBS, Stat->Probes[i].Got);
		
		D_BSwu64(a_Desc->RelBS, Stat->Probes[i].sendPTic);
		D_BSwu32(a_Desc->RelBS, Stat->Probes[i].sendMS);
		
		D_BSwu64(a_Desc->RelBS, Stat->Probes[i].recvPTic);
		D_BSwu32(a_Desc->RelBS, Stat->Probes[i].recvMS);
	}
	
	D_BSRecordNetBlock(a_Desc->RelBS, a_Addr);
	
	/* Done */
	return true;
}

/* DXP_LAGS() -- Lag probe results are in */
static bool_t DXP_LAGS(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
	int32_t i;
	D_XNetLagStat_t* Stat;
	uint32_t ID;
	uint8_t Count, NLSP;
	
	/* Check EP */
	if (!a_EP)
		return false;
	
	/* Obtain stat */
	Stat = &a_EP->LagStat;
	
	// Read Header
	ID = D_BSru32(a_Desc->RelBS);
	Count = D_BSru8(a_Desc->RelBS);
	NLSP = D_BSru8(a_Desc->RelBS);
	
	// Same ID?
	if (ID == Stat->ID)
		return false;	// already got it
	
	// Clear
	memset(Stat, 0, sizeof(*Stat));
	
	// Set
	Stat->ID = ID;
	Stat->Count = Count;
	a_EP->LagNLSP = NLSP;
	
	// Read fields
	for (i = 0; i < NLSP && i < NUMLAGSTATPROBES; i++)
	{
		Stat->Probes[i].Got = D_BSru8(a_Desc->RelBS);
		
		Stat->Probes[i].sendPTic = D_BSru64(a_Desc->RelBS);
		Stat->Probes[i].sendMS = D_BSru32(a_Desc->RelBS);
		
		Stat->Probes[i].recvPTic = D_BSru64(a_Desc->RelBS);
		Stat->Probes[i].recvMS = D_BSru32(a_Desc->RelBS);
	}
	
	/* Calculate Net Config */
	D_XPCalcConnection(a_EP);
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
	{"DISC", DXP_DISC, PF_ONAUTH},
	{"SYNJ", DXP_SYNJ, PF_ONAUTH | PF_SLAVE | PF_REL},
	{"SYNC", DXP_SYNC, PF_ONAUTH | PF_MASTER | PF_REL},
	{"RQFL", DXP_RQFL, PF_ONAUTH | PF_SERVER | PF_REL},
	{"WADS", DXP_WADS, PF_ONAUTH | PF_CLIENT | PF_REL},
	{"JRDY", DXP_JRDY, PF_ONAUTH | PF_SERVER | PF_REL},
	{"PLAY", DXP_PLAY, PF_ONAUTH | PF_SERVER | PF_REL},
	{"TICS", DXP_TICS, PF_ONAUTH | PF_CLIENT | PF_REL},
	{"TACK", DXP_TACK, PF_ONAUTH | PF_SERVER | PF_REL},
	{"TRUN", DXP_TRUN, PF_ONAUTH | PF_SERVER | PF_REL},
	{"TRYJ", DXP_TRYJ, PF_ONAUTH | PF_SERVER | PF_REL},
	{"TREQ", DXP_TREQ, PF_ONAUTH | PF_SERVER | PF_REL},
	{"FILE", DXP_FILE, PF_ONAUTH},
	{"INFO", DXP_INFO, PF_MASTER | PF_NOREL},
	{"PREF", DXP_PREF, PF_ONAUTH | PF_SERVER | PF_REL},
	{"NIGM", DXP_NIGM, PF_ONAUTH | PF_CLIENT | PF_REL},
	{"AIGM", DXP_NIGM, PF_ONAUTH | PF_CLIENT | PF_REL},
	{"CHAT", DXP_CHAT, PF_ONAUTH | PF_SERVER | PF_REL},
	{"LAGP", DXP_LAGP, PF_ONAUTH | PF_CLIENT},
	{"LAGR", DXP_LAGR, PF_ONAUTH | PF_CLIENT | PF_REL},
	{"LAGS", DXP_LAGS, PF_ONAUTH | PF_SERVER | PF_REL},
	
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
			// Delete file
			I_FileDeletePath(a_Path);
			
			CONL_OutputUT(CT_NETWORK, DSTR_DXP_BADSAVELOAD, "%s\n", Name);
			D_XNetDisconnect(false);
			return;
		}
		
		// Set as playing now if waiting for window
		if (a_Desc->CS.Client.SyncLevel == DXSL_WAITFORWINDOW)
			a_Desc->CS.Client.SyncLevel = DXSL_PLAYING;
		
		// Delete file
		I_FileDeletePath(a_Path);
		
		// If recording demo, save to demo (so they can play the net demo)
		if (demorecording)
			G_EncodeSaveGame();
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

/* D_XPPossiblyEncodeTics() -- Possibly encodes all our local tics */
void D_XPPossiblyEncodeTics(D_BS_t* const a_BS)
{
	static tic_t LastPT;	
	
	/* Encode if there was a new tic */
	if (LastPT != g_ProgramTic)
	{
		LastPT = g_ProgramTic;
		D_XPEncodeLocalTics(a_BS);
	}
	
	/* No encoding */
	else
		D_BSwu32(a_BS, 0);
}

/* D_XPEncodeLocalTics() -- Encodes local tics to send to server */
// Then applies to the local angles, etc.
void D_XPEncodeLocalTics(D_BS_t* const a_BS)
{
	D_XNetTicBuf_t Buf;
	bool_t Cleared;
	int32_t i;
	D_XPlayer_t* XPlay;
	int8_t Map[MAXSPLITSCREEN];
	uint8_t* EncDP;
	uint32_t EncSP;
	ticcmd_t* TicCmd;
	
	/* Check */
	if (!a_BS)
		return;
	
	/* Init */
	Cleared = false;
	
	/* Go through our screens (faster) */
	for (i = 0; i < MAXSPLITSCREEN; i++)
	{
		// Clear map
		Map[i] = -1;
		
		// No player in split?
		if (!D_ScrSplitHasPlayer(i))
			continue;
		
		// Get XPlayer
		XPlay = g_Splits[i].XPlayer;
		
		// No player or not playing?
		if (!XPlay || XPlay->InGameID < 0 || XPlay->InGameID >= MAXPLAYERS || g_Splits[i].Console < 0)
			continue;
		
		// No tics to encode?
		if (!XPlay->LocalAt)
			continue;
		
		// Tics need clearing?
		if (!Cleared)
		{
			// Init
			memset(&Buf, 0, sizeof(Buf));
			
			// Now cleared
			Cleared = true;
		}
		
		// Set mask and map
		Buf.PIGRevMask |= (1 << g_Splits[i].Console);
		Map[i] = g_Splits[i].Console;
		
		// Merge tics into said buffer
		D_XNetMergeTics(&Buf.Tics[g_Splits[i].Console], XPlay->LocalBuf, XPlay->LocalAt);
		XPlay->LocalAt = 0;
		memset(XPlay->LocalBuf, 0, sizeof(XPlay->LocalBuf));
	}
	
	/* Only if tics were cleared */
	if (Cleared)
	{
		// Apply split screen angle/aiming
			// Done first so the server gets our wanted look/aim angles
		for (i = 0; i < MAXSPLITSCREEN; i++)
			if (Map[i] != -1)
			{
				// Tic command of encoded player
				TicCmd = &Buf.Tics[Map[i]];
				
				// Absolute Angles
				if (P_XGSVal(PGS_COABSOLUTEANGLE))
				{
					localangle[i] += TicCmd->Std.BaseAngleTurn << 16;
					TicCmd->Std.angleturn = localangle[i] >> 16;
				}

				// Doom Angles
				else
					TicCmd->Std.angleturn = TicCmd->Std.BaseAngleTurn;
			
				// Aiming Angle
				if (TicCmd->Std.buttons & BT_RESETAIM)
					localaiming[i] = 0;
				else
				{
					// Panning Look
					if (TicCmd->Std.buttons & BT_PANLOOK)
						localaiming[i] = TicCmd->Std.BaseAiming << 16;
					
					// Standard Look
					else
						localaiming[i] += TicCmd->Std.BaseAiming << 16;
				}
				
				// Clip aiming pitch to not exceed bounds
				TicCmd->Std.aiming = G_ClipAimingPitch(&localaiming[i]);
			}
			
		// Set encoding mask
		D_BSwu32(a_BS, Buf.PIGRevMask);
		
		// Reverse the mask
		Buf.PIGRevMask = ~Buf.PIGRevMask;
		
		// Encode buffer data
		EncDP = NULL;
		EncSP = 0;
		
		D_XNetEncodeTicBuf(&Buf, &EncDP, &EncSP, DXNTBV_LATEST);
		
		// Size of encoding
		D_BSwu16(a_BS, EncSP);
		
		// Data to encode?
		if (EncDP && EncSP)
			D_BSWriteChunk(a_BS, EncDP, EncSP);
	}
	
	// No players encoded
	else
		D_BSwu32(a_BS, 0);
}

/*---------------------------------------------------------------------------*/

