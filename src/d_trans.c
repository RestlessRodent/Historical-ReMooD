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
// DESCRIPTION: Network transmission

/***************
*** INCLUDES ***
***************/

#include "d_net.h"
#include "i_util.h"
#include "doomstat.h"
#include "d_main.h"
#include "console.h"

/****************
*** CONSTANTS ***
****************/

/* D_ClientStage_t -- Client connect stage */
typedef enum D_ClientStage_e
{
	DCS_LISTWADS,
	DCS_CHECKWADS,
	DCS_DOWNLOADWADS,
	DCS_REQUESTSAVE,
	DCS_GETSAVE,
	
	DCS_PLAYINGGAME
} D_ClientStage_t;

/*****************
*** STRUCTURES ***
*****************/

#define CHAINSIZE 128

/* D_WADChain_t -- WAD Chain */
typedef struct D_WADChain_s
{
	char DOS[CHAINSIZE];
	char Full[CHAINSIZE];
	char Sum[CHAINSIZE];
	const WL_WADFile_t* WAD;
	struct D_WADChain_s* Prev;
} D_WADChain_t;

/*************
*** LOCALS ***
*************/

static I_NetSocket_t* l_Sock;
static I_HostAddress_t l_HostAddr;
static D_BS_t* l_BS;
static D_WADChain_t* l_WADTail;
static D_ClientStage_t l_Stage;

/****************
*** FUNCTIONS ***
****************/

/* D_SNOkTics() -- Tics that can be run by the game */
int32_t D_SNOkTics(void)
{
	/* Do not move forward if not connected */
	if (!D_SNIsConnected())
		return 0;	
	
	/* Server */
	if (D_SNIsServer())
	{
		return 1;
	}
	
	/* Client */
	else
	{
		return 0;
	}
}

/* D_SNNetCreate() -- Creates network connection */
// Either listener or remote
bool_t D_SNNetCreate(const bool_t a_Listen, const char* const a_Addr, const uint16_t a_Port)
{
	I_HostAddress_t Host;
	uint32_t Flags;
	uint16_t Port;
	
	/* Check */
	// Need address if not listening
	if (!a_Listen && !a_Addr)
		return false;
	
	/* If socket exists, terminate */
	if (l_Sock)
		D_SNNetTerm("Creating new socket");
	
	/* Create socket */
	// Clear
	memset(&Host, 0, sizeof(Host));
	
	// Initial Flags
	Flags = 0;
	
	// Setup host
	if (a_Addr)
		I_NetNameToHost(NULL, &Host, a_Addr);
	
	// IPv6?
	if (Host.IPvX == INIPVN_IPV6)
		Flags |= INSF_V6;
	
	// Port
	Port = 0;
	if (a_Port != 0)
		Port = a_Port;
	else if (Host.IPvX && Host.Port != 0)
		Port = Host.Port;
	
	// Cap, port
	if (Port < 1 || Port >= 65536)
		Port = __REMOOD_BASEPORT;
	
	// Create socket
	if (!(l_Sock = I_NetOpenSocket(Flags, (a_Listen && Host.IPvX ? &Host : NULL), Port)))
		return false;	// failed
	
	/* Copy host and setup stream */
	memmove(&l_HostAddr, &Host, sizeof(Host));
	l_BS = D_BSCreateNetStream(l_Sock);
	
	/* Socket created */
	return true;
}

/* D_SNNetTerm() -- Terminates network connection */
void D_SNNetTerm(const char* const a_Reason)
{
	const char* Reason;
	int32_t i;
	
	/* If there is no socket, do not terminate */
	if (!l_Sock)
		return;
	
	/* Reason */
	if (a_Reason)
		Reason = a_Reason;
	else
		Reason = "None given";
	
	/* Send Bye */	
	D_BSBaseBlock(l_BS, "QUIT");
	D_BSws(l_BS, Reason);
	
	for (i = 0; i < 5; i++)
		D_BSRecordNetBlock(l_BS, &l_HostAddr);
	
	/* Drop all clients */
	D_SNDropAllClients(a_Reason);
	
	/* Close socket */
	D_BSCloseStream(l_BS);
	l_BS = NULL;
	I_NetCloseSocket(l_Sock);
	l_Sock = NULL;
	
	/* Clear host */
	memset(&l_HostAddr, 0, sizeof(l_HostAddr));
	
	/* Disconnect message */
	CONL_OutputUT(CT_NETWORK, DSTR_DXP_DISCONNED, "%s\n", Reason);
}

/* D_SNHasSocket() -- Socket exists */
bool_t D_SNHasSocket(void)
{
	return !!l_Sock;
}

/* D_SNDisconnectHost() -- Disconnects another host */
void D_SNDisconnectHost(D_SNHost_t* const a_Host, const char* const a_Reason)
{
	int i;
	/* Check */
	if (!l_BS || !a_Host || !D_SNIsServer() || a_Host->Local)
		return;
	
	/* Already cleaning up */
	if (a_Host->Cleanup)
		return;
	
	/* Cleanup */
	a_Host->Cleanup = true;
	strncpy(a_Host->QuitReason, (a_Reason ? a_Reason : "No Reason"), MAXQUITREASON);
	a_Host->QuitReason[MAXQUITREASON - 1] = 0;
	
	/* Show they disconnected */
	CONL_OutputUT(CT_NETWORK, DSTR_NET_CLIENTGONE, "%s\n", a_Host->QuitReason);
	
	/* Send them a packet */
	D_BSBaseBlock(l_BS, "QUIT");
	D_BSws(l_BS, a_Host->QuitReason);
	for (i = 0; i < 5; i++)
		D_BSRecordNetBlock(l_BS, &a_Host->Addr);
}

/* D_SNDoConnect() -- Do connection logic */
void D_SNDoConnect(void)
{
#define BUFSIZE 128
	char Buf[BUFSIZE];
	static tic_t Timeout;
	
	/* Not yet timed out */
	if (g_ProgramTic < Timeout)
		return;
	
	/* Time out in the future */
	Timeout = g_ProgramTic + TICRATE;
	I_NetHostToString(&l_HostAddr, Buf, BUFSIZE);
	CONL_OutputUT(CT_NETWORK, DSTR_DXP_CONNECTING, "%s\n", Buf);
	
	/* Build connect info */
	D_BSBaseBlock(l_BS, "CONN");
	
	D_BSwu8(l_BS, D_SNIsServer());
	
	D_BSRecordNetBlock(l_BS, &l_HostAddr);
#undef BUFSIZE
}

/* D_SNDoServer() -- Do server stuff */
void D_SNDoServer(D_BS_t* const a_BS)
{
}

/* D_SNDoClient() -- Do client stuff */
void D_SNDoClient(D_BS_t* const a_BS)
{
	static tic_t WADTimeout, SaveTimeout;	
	
	/* Which stage? */
	switch (l_Stage)
	{
			// Ask server for list of WADs
		case DCS_LISTWADS:
			if (g_ProgramTic < WADTimeout)
				return;
			
			WADTimeout = g_ProgramTic + TICRATE;
			D_BSBaseBlock(a_BS, "LIST");
			D_BSRecordNetBlock(a_BS, &l_HostAddr);
			return;
			
			// Check local WADs
		case DCS_CHECKWADS:
			break;
			
			// Download WADs
		case DCS_DOWNLOADWADS:
			D_SNDisconnect(false, "Downloading WADs not implemented");
			break;
			
			// Request save game
		case DCS_REQUESTSAVE:
			break;
		
			// Get savegame
		case DCS_GETSAVE:
			if (g_ProgramTic < SaveTimeout)
				return;
			
			SaveTimeout = g_ProgramTic + TICRATE;
			D_BSBaseBlock(a_BS, "SAVE");
			D_BSRecordNetBlock(a_BS, &l_HostAddr);
			break;
			
			// Unknown Stage
		default:
			return;
	}
}

/*****************************************************************************/

/* DT_CONN() -- Connection request */
void DT_CONN(D_BS_t* const a_BS, D_SNHost_t* const a_Host, I_HostAddress_t* const a_Addr)
{
#define BUFSIZE 256
	char Buf[BUFSIZE];
	uint8_t IsServer;
	D_SNHost_t* New;
	
	/* If not allocated, build host for it */
	if (!a_Host)
	{
		// Read data bits
		IsServer = D_BSru8(a_BS);
		
		// Same side?
		if (!!IsServer == D_SNIsServer())
			return;
		
		// Put trying to connect...
		I_NetHostToString(a_Addr, Buf, BUFSIZE);
		CONL_OutputUT(CT_NETWORK, DSTR_DXP_CLCONNECT, "%s\n", Buf);
		
		// Create a host for them
		New = D_SNCreateHost();
		
		// Fill info
		memmove(&New->Addr, a_Addr, sizeof(*a_Addr));
	}
	
	// Use host here
	else
		New = a_Host;
	
	/* Reply with host info */
	D_BSBaseBlock(a_BS, "HELO");
	
	// Write ID
	D_BSwu32(a_BS, New->ID);
	
	D_BSRecordNetBlock(a_BS, a_Addr);
#undef BUFSIZE
}

/* DT_HELO() -- Connection granted */
void DT_HELO(D_BS_t* const a_BS, D_SNHost_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	D_SNHost_t* New;
	
	/* Ignore if already connected */
	if (D_SNIsConnected())
		return;
	
	/* Set connected */
	D_SNSetConnected(true);
	
	/* Create host */
	New = D_SNCreateHost();
	New->ID = D_BSru32(a_BS);
	New->Local = true;
	l_Stage = 0;
}

/* DT_LIST() -- List WADS */
void DT_LIST(D_BS_t* const a_BS, D_SNHost_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	const WL_WADFile_t* Rover;
	int32_t i;
	
	/* Reply with wad list */
	D_BSBaseBlock(a_BS, "WADL");
	
	// Send WAD Order
	for (Rover = WL_IterateVWAD(NULL, true), i = 0; Rover; Rover = WL_IterateVWAD(Rover, true), i++)
	{
		// Ignore ReMooD.WAD
		if (i == 1)
			continue;
		
		// Names and MD5
		D_BSws(a_BS, WL_GetWADName(Rover, false));
		D_BSws(a_BS, WL_GetWADName(Rover, true));
		D_BSws(a_BS, Rover->CheckSumChars);
	}
	
	// NUL for end
	D_BSwu32(a_BS, 0);
	
	/* Send */
	D_BSRecordNetBlock(a_BS, a_Addr);
}

/* DT_WADL() -- WAD List */
void DT_WADL(D_BS_t* const a_BS, D_SNHost_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	D_WADChain_t* Rover;
	D_WADChain_t Temp;
	const WL_WADFile_t* WAD;
	
	/* Only on first stage */
	if (l_Stage != DCS_LISTWADS || D_SNIsServer())
		return;
	
	/* Store WADs from server */
	// Clear tail
	for (Rover = l_WADTail; Rover; Rover = l_WADTail)
	{
		// Set previous
		l_WADTail = Rover->Prev;
		
		// Free current
		Z_Free(Rover);
	}
	
	l_WADTail = NULL;
	CONL_OutputUT(CT_NETWORK, DSTR_DXP_WADHEADER, "\n");
	
	// Read in contents
	for (;;)
	{
		// Clear
		memset(&Temp, 0, sizeof(Temp));
		
		// DOS, Full, Sum
		D_BSrs(a_BS, Temp.DOS, CHAINSIZE);
		D_BSrs(a_BS, Temp.Full, CHAINSIZE);
		D_BSrs(a_BS, Temp.Sum, CHAINSIZE);
		
		// End
		if (!Temp.DOS[0])
			break;
		
		// Print
		CONL_OutputUT(CT_NETWORK, DSTR_DXP_WADENTRY, "%s%s%s\n", Temp.DOS, Temp.Full, Temp.Sum);
		
		// Try to find WAD by sum
		for (WAD = WL_IterateVWAD(NULL, true); WAD; WAD = WL_IterateVWAD(WAD, true))
			if (!strcasecmp(Temp.Sum, WAD->CheckSumChars))
			{
				Temp.WAD = WAD;
				break;
			}
		
		// Allocate
		Rover = Z_Malloc(sizeof(*Rover), PU_STATIC, NULL);
		memmove(Rover, &Temp, sizeof(*Rover));
		
		// Chain in
		Rover->Prev = l_WADTail;
		l_WADTail = Rover;
	}
	
	/* Increase Stage */
	l_Stage++;
}

/* DT_QUIT() -- Quit */
void DT_QUIT(D_BS_t* const a_BS, D_SNHost_t* const a_Host, I_HostAddress_t* const a_Addr)
{
#define BUFSIZE 128
	char Buf[BUFSIZE];
	
	/* Read reason */
	D_BSrs(a_BS, Buf, BUFSIZE);
	
	/* If not connected, disconnect */
	// Could be in the middle of a connect request, or not playing yet
	if (!D_SNIsConnected() || (!D_SNIsServer() && l_Stage < DCS_PLAYINGGAME))
	{
		D_SNDisconnect(false, Buf);
		return;
	}
	
	/* If we are the server, and this host is local do not disconnect */
	else if (D_SNIsServer() && a_Host && a_Host->Local)
		return;
	
	/* Cleanup remote client if non-local (they quit) */
	else if (a_Host && !a_Host->Local)
		D_SNDisconnectHost(a_Host, Buf);
	
	/* Otherwise, perform a partial disconnect */
	// As long as we are not the server!
	else if (!D_SNIsServer())
		D_SNPartialDisconnect(Buf);
#undef BUFSIZE
}

/* l_Packets -- Data packets */
static const struct
{
	union
	{
		uint8_t Char[4];
		uint32_t Int;
	} H;
	void (*Func)(D_BS_t* const a_BS, D_SNHost_t* const a_Host, I_HostAddress_t* const a_Addr);
	bool_t RemoteOnly;							// Only from remote end
} l_Packets[] =
{
	{{"CONN"}, DT_CONN, false},
	{{"HELO"}, DT_HELO, true},
	{{"LIST"}, DT_LIST, false},
	{{"WADL"}, DT_WADL, true},
	{{"QUIT"}, DT_QUIT, false},
	
	{{{0}}}
};

/* D_SNDoTrans() -- Do transmission */
void D_SNDoTrans(void)
{
	union
	{
		char Char[4];
		uint32_t Int;
	} Header;
	bool_t Continue;
	I_HostAddress_t Addr;
	D_SNHost_t* Host;
	int32_t i;
	
	/* No Socket */
	if (!l_Sock)
		return;
	
	/* Perform packet updates */
	for (Continue = true; Continue;)
	{
		// Clear
		memset(&Header, 0, sizeof(Header));
		memset(&Addr, 0, sizeof(Addr));
		
		// Play block
		if ((Continue = D_BSPlayNetBlock(l_BS, Header.Char, &Addr)))
		{
			// Find host, if any
			Host = D_SNHostByAddr(&Addr);
			
			// Which type?
			for (i = 0; l_Packets[i].H.Char[0]; i++)
				if (l_Packets[i].H.Int == Header.Int)
				{
					// Needs to be from remote
					if (l_Packets[i].RemoteOnly)
						if (!I_NetCompareHost(&Addr, &l_HostAddr))
							break;
					
					l_Packets[i].Func(l_BS, Host, &Addr);
					break;
				}
		}
	}
	
	/* If not connected, connect to remote side */
	if (!D_SNIsConnected())
	{
		D_SNDoConnect();
		return;
	}
	
	/* Do client or server stuff */
	if (D_SNIsServer())
		D_SNDoServer(l_BS);
	else
		D_SNDoClient(l_BS);
}

