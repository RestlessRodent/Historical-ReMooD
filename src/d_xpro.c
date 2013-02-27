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

/**************
*** GLOBALS ***
**************/

/****************
*** FUNCTIONS ***
****************/

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
		
		memset(Buf, 0, sizeof(Buf));
		snprintf(Buf, BUFSIZE - 1, "processid=%08x", g_Splits[0].ProcessID);
		D_BSws(a_Desc->StdBS, Buf);
		
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
}

/*---------------------------------------------------------------------------*/

/* DXP_GSYN() -- Game Synchronize Connection */
static bool_t DXP_GSYN(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr)
{
#define BUFSIZE 72
	char Buf[BUFSIZE];
	uint8_t RemoteIsServer;
	char *EqS;
	
	uint32_t ProcessID;
	D_XEndPoint_t* EP;
	
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
			
			// ProcessID
		else if (!strcasecmp(Buf, "processid"))
			ProcessID = C_strtou32(EqS, NULL, 0);
	}
	
	/* See if endpoint was already added? */
	EP = D_XBEndPointForAddr(a_Addr);
	
	// It was, no need to resend because it is reliable packet
	if (EP)
		return true;
	
	/* Create new endpoint */
	EP = D_XBNewEndPoint(a_Desc, a_Addr);
	
	EP->ProcessID = ProcessID;
	
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
	CONL_OutputUT(CT_NETWORK, DSTR_DXP_CLCONNECT, "%s\n", Buf);
	
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
	// Game Synchronize Connection
	{"GSYN", DXP_GSYN, PF_MASTER},
	{"DISC", DXP_DISC, PF_SLAVE},
	{"SYNJ", DXP_SYNJ, PF_SLAVE | PF_REL},
	
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

