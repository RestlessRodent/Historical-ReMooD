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

/*************
*** LOCALS ***
*************/

static D_XWADCheck_t* l_WADChecks = NULL;		// WAD Checks

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

/*---------------------------------------------------------------------------*/

/* DS_DoMaster() -- Handles master connection */
static void DS_DoMaster(D_XDesc_t* const a_Desc)
{
}

/* DS_DoSlave() -- Handles slave connection */
static void DS_DoSlave(D_XDesc_t* const a_Desc)
{
#define BUFSIZE 64
	char Buf[BUFSIZE];
	int i;
	D_ProfileEx_t* Prof;
	
	/* Not Syncronized? */
	if (!a_Desc->Data.Slave.Synced)
	{
		// Sent requst already?
		if (g_ProgramTic < a_Desc->Data.Slave.LastSyncReq)
			return;
		
		// Build syncronization packet
		D_BSBaseBlock(a_Desc->StdBS, "GSYN");
		
		// Record Data
		D_BSwu8(a_Desc->StdBS, D_XNetIsServer());	// We are server?
		
		D_BSws(a_Desc->StdBS, "version=" REMOOD_FULLVERSIONSTRING);
		
		for (i = 0; i < MAXSPLITSCREEN; i++)
		{
			// No player here?
			if (!D_ScrSplitHasPlayer(i))
				continue;
			
			// Process ID of screen
			memset(Buf, 0, sizeof(Buf));
			snprintf(Buf, BUFSIZE - 1, "pid+%i=%08x", i, g_Splits[i].ProcessID);
			D_BSws(a_Desc->StdBS, Buf);
			
			// Profile?
			Prof = g_Splits[i].Profile;
			
			// Dump profile info
			if (Prof)
			{
				// UUID of profile
				memset(Buf, 0, sizeof(Buf));
				snprintf(Buf, BUFSIZE - 1, "puuid+%i=%s", i, Prof->UUID);
				D_BSws(a_Desc->StdBS, Buf);
				
				// Display Name
				memset(Buf, 0, sizeof(Buf));
				snprintf(Buf, BUFSIZE - 1, "pdn+%i=%s", i, Prof->DisplayName);
				D_BSws(a_Desc->StdBS, Buf);
			}
		}
		
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
	LevelP = &a_Desc->Client.SyncLevel;
	
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
				for (i = 0; i < g_NumXEP; i++)
					if (g_XEP[i])
					{
						*LevelP += 1;
						break;
					}
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
			if (!a_Desc->Client.SentReqWAD)
			{
				// Send request to server
				D_BSBaseBlock(RelBS, "RQFL");
				
				// Send away!
				D_BSRecordNetBlock(RelBS, HostAddr);
				
				// Sent request
				a_Desc->Client.SentReqWAD = true;
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
			*LevelP = DXSL_GETSAVE;
			break;
		
			// Unhandled
		default:
			break;
	}
}

/*---------------------------------------------------------------------------*/

/* DXP_GSYN() -- Game Synchronize Connection */
static bool_t DXP_GSYN(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr)
{
#define BUFSIZE 72
	char Buf[BUFSIZE];
	uint8_t RemoteIsServer;
	char *EqS, *Plus;
	int32_t s, i;
	
	uint32_t ProcessID;
	D_XEndPoint_t* EP;
	D_XEndPoint_t Hold;
	
	/* Read if the remote says it is a server */
	RemoteIsServer = D_BSru8(a_Desc->RelBS);
	
	// Incompatible
	if (!a_Desc->Master || ((!!RemoteIsServer) == (!!D_XNetIsServer())))
	{
		D_XPSendDisconnect(a_Desc, a_Desc->StdBS, a_Addr, "Remote end has same connection type gender.");
		return true;
	}
	
	/* Init Settings */
	ProcessID = 0;
	memset(&Hold, 0, sizeof(Hold));
	
	/* Read String Settings */
	for (;;)
	{
		// Read new string
		memset(Buf, 0, sizeof(Buf));
		D_BSrs(a_Desc->RelBS, Buf, BUFSIZE - 1);
		
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
	D_BSBaseBlock(a_Desc->RelBS, "SYNJ");
	
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

/* DXP_DISC() -- Disconnect Received */
static bool_t DXP_DISC(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr)
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
static bool_t DXP_SYNJ(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr)
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

/* DXP_RQFL() -- Request File List */
static bool_t DXP_RQFL(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr)
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
static bool_t DXP_WADS(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr)
{
#define BUFSIZE 48
	uint8_t Code;
	char LongName[BUFSIZE];
	char DOSName[WLMAXDOSNAME];
	char CheckSum[BUFSIZE];
	
	char* RealLong, *RealDOS;
	D_XWADCheck_t* Check;
	
	/* In wrong sync mode? */
	if (a_Desc->Client.SyncLevel != DXSL_LISTWADS)
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
	a_Desc->Client.SyncLevel++;
	
	/* Success! */
	return true;
#undef BUFSIZE
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
	PF_SYNCED			= UINT32_C(0x00000040),	// Remote must be synced
} D_CSPackFlag_t;

/* c_CSPacks -- Client/Server Packets */
static const struct
{
	const char* Header;
	bool_t (*Func)(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr);
	uint32_t Flags;
} c_CSPacks[] =
{
	{"GSYN", DXP_GSYN, PF_MASTER},
	{"DISC", DXP_DISC, PF_SLAVE},
	{"SYNJ", DXP_SYNJ, PF_SLAVE | PF_REL},
	{"RQFL", DXP_RQFL, PF_SERVER | PF_REL},
	{"WADS", DXP_WADS, PF_CLIENT | PF_REL},
	
	{NULL}
};

/* D_XPRunCS() -- Run client server operation */
void D_XPRunCS(D_XDesc_t* const a_Desc)
{
	char Header[5];
	int32_t i, b;
	I_HostAddress_t RemAddr;
	bool_t Continue;
	uint32_t Flags;
	bool_t Authed;
	
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
				
				// Call handler function
				if (c_CSPacks[i].Func)
					c_CSPacks[i].Func(a_Desc, Header, Flags, &RemAddr);
				
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
	D_BSFlushStream(a_Desc->RelBS);
}

/*---------------------------------------------------------------------------*/

