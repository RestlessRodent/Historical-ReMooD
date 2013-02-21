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
// Copyright (C) 2012-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: ReMooD Protocol Implementation

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "doomdef.h"
#include "i_util.h"
#include "ip.h"
#include "ip_prv.h"
#include "console.h"
#include "dstrings.h"
#include "m_argv.h"

/****************
*** CONSTANTS ***
****************/

#define CONNRETRYDELAY (TICRATE)
#define SERVERADDR(x) ((I_HostAddress_t*)(&((x)->Conn->RemAddr.Private)))

typedef enum IP_RmdFlags_e
{
	IPRF_SERVER			= UINT32_C(0x00000001),	// Requires being server
	IPRF_CLIENT			= UINT32_C(0x00000002),	// Requires being client
	IPRF_AUTH			= UINT32_C(0x00000004),	// Authorized
	IPRF_NEEDCONN		= UINT32_C(0x00000008),	// Needs to be connected
	IPRF_NOTCONN		= UINT32_C(0x00000010),	// Must not be connected
} IP_RmdFlags_t;

/*****************
*** STRUCTURES ***
*****************/

/* IP_RmdData_t -- Connection Data */
typedef struct IP_RmdData_s
{
	IP_Conn_t* Conn;						// Connection
	I_NetSocket_t* Socket;						// Socket to server
	
	D_BS_t* BS;									// Block Stream
	D_BS_t* BSnet;								// Net Stream
	
	bool_t IsConnected;							// Connected To Server
	tic_t LastConnectTry;						// Last time connection was tried
} IP_RmdData_t;

/* IP_RmdConnInfo_t -- Connection Info */
typedef struct IP_RmdConnInfo_s
{
	I_HostAddress_t* Addr;						// Address connecting from
	uint32_t BootClProcessID;					// Initial process ID
	IP_RmdData_t* Data;
} IP_RmdConnInfo_t;

/****************
*** FUNCTIONS ***
****************/

typedef void (*IPR_HandleType_t)(IP_RmdData_t* const a_Data, D_BS_t* const a_BS, I_HostAddress_t* const a_Addr);

/*---------------------------------------------------------------------------*/
// As Server

/* IPS_RMD_WhenServer() -- This is run when we are a server */
static void IPS_RMD_WhenServer(const IP_Proto_t* a_Proto, IP_Conn_t* const a_Conn, IP_RmdData_t* const a_Data)
{
}

/* IPRS_RINF() -- Request Info */
static void IPRS_RINF(IP_RmdData_t* const a_Data, D_BS_t* const a_BS, I_HostAddress_t* const a_Addr)
{
}

/* IPRS_SCON() -- Simple Connect */
static void IPRS_SCON(IP_RmdData_t* const a_Data, D_BS_t* const a_BS, I_HostAddress_t* const a_Addr)
{
	IP_Addr_t RemAddr;
	IP_WaitClient_t* New;
	
	/* Convert to IP compatible format */
	memset(&RemAddr, 0, sizeof(RemAddr));
	IP_IHostToIPAddr(&RemAddr, a_Addr, a_Data->Conn->Handler);
	
	/* See if already connected. */
	New = IP_WaitByConnAddr(a_Data->Conn, &RemAddr);
	
	// They are, but also check for multiple XPlayers
	if (New || D_XNetPlayerByAddr(a_Addr) || D_XNetPlayerByIPAddr(&RemAddr))
		return;
	
	/* They need to be authentic by the reliable protocol */
	// Otherwise any packets they send us will be dropped
	D_BSStreamIOCtl(a_BS, DRBSIOCTL_ADDHOST, (intptr_t*)a_Addr);
	
	/* Set them to join now */
	New = IP_WaitAdd(a_Data->Conn, &RemAddr, D_XNetMakeID(0));
	
	// Oops?
	if (!New)
		return;
	
	New->Remote.ProcessID = D_BSru32(a_BS);
	
	/* Send information to client */
	D_BSBaseBlock(a_BS, "SCOK");
	
	// Tell the remote client, their host ID
	D_BSwu32(a_BS, New->HostID);
	
	// Send away
	D_BSRecordNetBlock(a_BS, a_Addr);
	
	/* Print Info */
	CONL_OutputUT(CT_NETWORK, DSTR_IPC_CLCONNECT, "%s\n", IP_AddrToString(&RemAddr));
}

/*---------------------------------------------------------------------------*/
// As Client

/* IPS_RMD_WhenClientDisconn() -- Disconnected as a client */
static void IPS_RMD_WhenClientDisconn(const IP_Proto_t* a_Proto, IP_Conn_t* const a_Conn, IP_RmdData_t* const a_Data)
{
	/* Expired connect attempt */
	if (!a_Data->LastConnectTry || g_ProgramTic >= a_Data->LastConnectTry)
	{
		// Set new timeout to now
		a_Data->LastConnectTry = g_ProgramTic + CONNRETRYDELAY;
		
		// Print Message
		CONL_OutputUT(CT_NETWORK, DSTR_IPC_CONNECTING, "%s\n", IP_AddrToString(&a_Data->Conn->RemAddr));
		
		// Send simple connection packet to the server
		D_BSBaseBlock(a_Data->BSnet, "SCON");
		D_BSwu32(a_Data->BSnet, g_Splits[0].ProcessID);
		D_BSRecordNetBlock(a_Data->BSnet, SERVERADDR(a_Data));
	}
}

/* IPS_RMD_WhenClientConn() -- Connected as a client */
static void IPS_RMD_WhenClientConn(const IP_Proto_t* a_Proto, IP_Conn_t* const a_Conn, IP_RmdData_t* const a_Data)
{
}

/* IPRS_SCOK() -- Simple Connect, Authorized */
static void IPRS_SCOK(IP_RmdData_t* const a_Data, D_BS_t* const a_BS, I_HostAddress_t* const a_Addr)
{
	/* Print Message */
	CONL_OutputUT(CT_NETWORK, DSTR_IPC_NOWFORJOINWINDOW, "%s\n", IP_AddrToString(&a_Data->Conn->RemAddr));
	
	/* Set to the specified host ID */
	D_XNetSetHostID(D_BSru32(a_BS));
	
	/* Set as connected */
	a_Data->IsConnected = true;
}

/*---------------------------------------------------------------------------*/

/* c_RMDProtoHeads -- Protocol headers */
static const struct
{
	char Header[5];								// Header
	IPR_HandleType_t Func;						// handler func
	uint32_t Flags;								// Flags for Handling
} c_RMDProtoHeads[] =
{
	{"RINF", IPRS_RINF, IPRF_SERVER},
	{"SCON", IPRS_SCON, IPRF_SERVER},
	{"SCOK", IPRS_SCOK, IPRF_CLIENT | IPRF_AUTH | IPRF_NOTCONN},
	
	// Done
	{""},
};

/*****************************************************************************/

/* IP_RMD_VerifyF() -- Verify Protocol */
bool_t IP_RMD_VerifyF(const IP_Proto_t* a_Proto, const char* const a_Host, const uint32_t a_Port, const char* const a_Options, const uint32_t a_Flags)
{
	uint32_t RealPort;
	
	/* Check Port */
	// Get the real port
	if (!a_Port)
		RealPort = 29500;
	else
		RealPort = a_Port;
	
	// Range
	if (RealPort <= 0 || RealPort >= 65536)
		return false;
	
	/* Success */
	return true;
}

/* IP_RMD_CreateF() -- Create connection */
IP_Conn_t* IP_RMD_CreateF(const IP_Proto_t* a_Proto, const char* const a_Host, const uint32_t a_Port, const char* const a_Options, const uint32_t a_Flags)
{
	struct IP_Addr_s Addr;
	IP_Conn_t* New;
	uint16_t Port, ClPort;
	I_NetSocket_t* Socket;
	IP_RmdData_t* Data;
	int32_t i;
	bool_t IsV6;
	
	/* Valid Port */
	// Default port setup
	Port = a_Port;
	ClPort = 0;
	
	// Client
	if (!(a_Flags & IPF_INPUT))
	{
		// Use -port option
		if (M_CheckParm("-port"))
			if (M_IsNextParm())
				ClPort = C_strtoi32(M_GetNextParm(), NULL, 10);
			
		// Random port for client
		if (!ClPort || ClPort < 0 || ClPort >= 65536)
		{
			ClPort = D_CMakePureRandom() & UINT32_C(0x7FFF);
			ClPort |= UINT32_C(0x8000);
		}
	}
	
	// Server
	else
	{
		// Use -port option
		if (M_CheckParm("-port"))
			if (M_IsNextParm())
				ClPort = C_strtoi32(M_GetNextParm(), NULL, 10);
		
		// Default port for client
		if (!ClPort || ClPort < 0 || ClPort >= 65536)
			ClPort = 29500;
	}
	
	// If server, alwayse use clport
	if (a_Flags & IPF_INPUT)
		Port = ClPort;
	
	/* Attempt Host Resolution */
	if (!IP_UDPResolveHost(a_Proto, &Addr, a_Host, Port))
	{
		// If making a server, warn that binding to said address is not possible
		if (a_Flags & IPF_INPUT)
			CONL_OutputUT(CT_NETWORK, DSTR_IPC_NOHOSTCANNOTBIND, "%s%u\n", a_Host, Port);
		
		// If connecting as a client, die
		if (!(a_Flags & IPF_INPUT))
			return NULL;
	}
	
	/* Setup Socket */
	// V6?
	IsV6 = false;
	
	if (M_CheckParm("-ipv6"))
		IsV6 = true;
	else if (/*(a_Flags & IPF_INPUT) &&*/ (((I_HostAddress_t*)&Addr.Private)->IPvX == INIPVN_IPV6))
		IsV6 = true;
	
	// Try making sockets
	for (i = 0; i < IPMAXSOCKTRIES; i++)
	{
		// Create socket to server
		Socket = I_NetOpenSocket((IsV6 ? INSF_V6 : 0), ((a_Flags & IPF_INPUT) ? &Addr.Private : NULL), ((a_Flags & IPF_INPUT) ? Port + i : ClPort + i));
	
		// Worked?
		if (Socket)
			break;
	}
	
	// No socket created
	if (!Socket)
		return NULL;
	
	/* Return new connection allocation */
	New = IP_AllocConn(a_Proto, a_Flags, &Addr);
	
	/* Initialize Mode */
	// Create Data
	New->Size = sizeof(*Data);
	Data = New->Data = Z_Malloc(New->Size, PU_STATIC, NULL);
	
	// Set Data
	Data->Socket = Socket;
	Data->Conn = New;
	Data->BSnet = D_BSCreateNetStream(Data->Socket);
	Data->BS = D_BSCreateReliableStream(Data->BSnet);
	
	// If a client, make sure the server gets a permitted connection
	if (!(a_Flags & IPF_INPUT))
		D_BSStreamIOCtl(Data->BS, DRBSIOCTL_ADDHOST, &Addr.Private);
	
	/* Return it */
	return New;
}

/* IP_RMD_DeleteConnF() -- Deletes connection */
void IP_RMD_DeleteConnF(const IP_Proto_t* a_Proto, IP_Conn_t* const a_Conn)
{
	IP_RmdData_t* Data;
	
	/* Check */
	if (!a_Proto || !a_Conn)
		return;
	
	/* Get Data */
	Data = a_Conn->Data;
	
	/* Close Streams */
	D_BSCloseStream(Data->BS);
	D_BSCloseStream(Data->BSnet);
	
	/* Close Socket */
	I_NetCloseSocket(Data->Socket);
	
	/* Clear Data */
	Z_Free(Data);
}

/* IP_RMD_RunConnF() -- Runs connection */
void IP_RMD_RunConnF(const IP_Proto_t* a_Proto, IP_Conn_t* const a_Conn)
{
	I_HostAddress_t RemAddr;
	char Header[5];
	IP_RmdData_t* Data;
	register int i;
	bool_t Auth;
	
	/* Check */
	if (!a_Proto || !a_Conn)
		return;
	
	/* Get Data */
	Data = a_Conn->Data;
		
	/* Constantly handle blocks */
	Header[4] = 0;
	while (D_BSPlayNetBlock(Data->BS, Header, &RemAddr))
	{
		for (i = 0; c_RMDProtoHeads[i].Header[0]; i++)
			if (D_BSCompareHeader(Header, c_RMDProtoHeads[i].Header))
			{
				// Not server and needs server?
				if ((c_RMDProtoHeads[i].Flags & (IPRF_SERVER | IPRF_CLIENT)) == IPRF_SERVER && !(a_Conn->Flags & IPF_INPUT))
					continue;
				
				// Not client and needs client?
				if ((c_RMDProtoHeads[i].Flags & (IPRF_SERVER | IPRF_CLIENT)) == IPRF_CLIENT && (a_Conn->Flags & IPF_INPUT))
					continue;
				
				// Packet authentic?
				Auth = false;
				D_BSStreamIOCtl(Data->BS, DRBSIOCTL_ISAUTH, (void*)&Auth);
				
				if ((c_RMDProtoHeads[i].Flags & IPRF_AUTH) && !Auth)
					continue;
				
				// If a client, Needs specific connection state?
				if (!(a_Conn->Flags & IPF_INPUT))
				{
					// Needs to be connected
					if (c_RMDProtoHeads[i].Flags & IPRF_NEEDCONN)
						if (!Data->IsConnected)
							continue;
					
					// Needs to be disconnected
					if (c_RMDProtoHeads[i].Flags & IPRF_NOTCONN)
						if (Data->IsConnected)
							continue;
				}
				
				c_RMDProtoHeads[i].Func(Data, Data->BS, &RemAddr);
				break;
			}
		
		// Illegal?
		if (devparm)
			if (!c_RMDProtoHeads[i].Header[0])
				CONL_PrintF("Unknown block \"%s\"\n", Header);
	}
	
	/* Do Client/Server Actions */
	// We are a server
	if (a_Conn->Flags & IPF_INPUT)
		IPS_RMD_WhenServer(a_Proto, a_Conn, Data);
	
	// We are a client
	else
		// Not Connected
		if (!Data->IsConnected)
			IPS_RMD_WhenClientDisconn(a_Proto, a_Conn, Data);
		
		// Connected
		else
			IPS_RMD_WhenClientConn(a_Proto, a_Conn, Data);
	
	/* Flush stream */
	// This is so packets are sent, etc.
	D_BSFlushStream(Data->BS);
}

/* IP_RMD_ConnTrashIPF() -- Trash IP Address */
void IP_RMD_ConnTrashIPF(const IP_Proto_t* a_Proto, IP_Conn_t* const a_Conn, I_HostAddress_t* const a_Addr)
{
	IP_RmdData_t* Data;
	
	/* Check */
	if (!a_Proto || !a_Conn)
		return;
	
	/* Get Data */
	Data = a_Conn->Data;
	
	/* Trash IP */
	D_BSStreamIOCtl(Data->BS, DRBSIOCTL_DROPHOST, a_Addr);
}

/* IP_RMD_SameAddrF() -- Same address? */
bool_t IP_RMD_SameAddrF(const IP_Proto_t* a_Proto, const IP_Addr_t* const a_A, const IP_Addr_t* const a_B)
{
	return I_NetCompareHost((I_HostAddress_t*)a_A->Private.Data, (I_HostAddress_t*)a_B->Private.Data);
}

/*****************************************************************************/

/* IPRS_BCST() -- Broadcast recieved */
static void IPRS_BCST(IP_RmdData_t* const a_Data, D_BS_t* const a_BS, I_HostAddress_t* const a_Addr)
{
	CONL_PrintF("Got broadcast\n");
}

/* c_RMDMasterProto -- Master headers */
static const struct
{
	char Header[5];								// Header
	IPR_HandleType_t Func;						// handler func
} c_RMDMasterProto[] =
{
	{"BCST", IPRS_BCST},
	
	// Done
	{""},
};

