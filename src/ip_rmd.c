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
} IP_RmdFlags_t;

/*****************
*** STRUCTURES ***
*****************/

/* IP_RmdData_t -- Connection Data */
typedef struct IP_RmdData_s
{
	struct IP_Conn_s* Conn;						// Connection
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

/* IPRS_RINF() -- Request Info */
static void IPRS_RINF(IP_RmdData_t* const a_Data, D_BS_t* const a_BS, I_HostAddress_t* const a_Addr)
{
	CONL_PrintF("Info request\n");
}

/* IPRS_SCONBaseXPlay() -- Basic XPlayer for SCON Connect */
static void IPRS_SCONBaseXPlay(D_XPlayer_t* const a_Player, void* const a_Data)
{
	IP_RmdConnInfo_t* InfoP;
	uint32_t ID;
	
	/* Grab Info */
	InfoP = a_Data;
	
	/* Setup internals based on Info */
	memmove(&a_Player->Address, InfoP->Addr, sizeof(a_Player->Address));
	a_Player->ClProcessID = InfoP->BootClProcessID;
	a_Player->IPConn = InfoP->Data->Conn;
	
	/* Set unique host ID */
	if (!a_Player->HostID)
	{
		do
		{
			ID = D_CMakePureRandom();
		} while (!ID || D_XNetPlayerByHostID(ID));

		// Set ID, is hopefully really random
		a_Player->HostID = ID;
	}
}

/* IPRS_SCON() -- Simple Connect */
static void IPRS_SCON(IP_RmdData_t* const a_Data, D_BS_t* const a_BS, I_HostAddress_t* const a_Addr)
{
	int32_t i;
	D_XPlayer_t* XPlay;
	IP_RmdConnInfo_t Info;
	
	CONL_PrintF("Simple connect request\n");
	
	/* See if the client has already connected... */
	XPlay = D_XNetPlayerByAddr(a_Addr);
	
	// He is connected!
	if (XPlay)
		return;
	
	/* Init Info */
	memset(&Info, 0, sizeof(Info));
	Info.Addr = a_Addr;
	Info.BootClProcessID = D_BSru32(a_BS);
	Info.Data = a_Data;
	
	/* Create a new XPlayer */
	XPlay = D_XNetAddPlayer(IPRS_SCONBaseXPlay, &Info, false);
	
	/* Send information to client */
	D_BSBaseBlock(a_BS, "SCOK");
	
	// Tell the remote client, their host ID
	D_BSwu32(a_BS, XPlay->HostID);
	
	// Send away
	D_BSRecordNetBlock(a_BS, a_Addr);
}

/* IPRS_SCOK() -- Simple Connect, Authorized */
static void IPRS_SCOK(IP_RmdData_t* const a_Data, D_BS_t* const a_BS, I_HostAddress_t* const a_Addr)
{
	CONL_PrintF("Connected!");	
	
	/* Set to the specified host ID */
	D_XNetSetHostID(D_BSru32(a_BS));
	
	/* Set as connected */
	a_Data->IsConnected = true;
}

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
	{"SCOK", IPRS_SCOK, IPRF_CLIENT | IPRF_AUTH},
	
	// Done
	{""},
};

/*---------------------------------------------------------------------------*/

/* IP_RMD_VerifyF() -- Verify Protocol */
bool_t IP_RMD_VerifyF(const struct IP_Proto_s* a_Proto, const char* const a_Host, const uint32_t a_Port, const char* const a_Options, const uint32_t a_Flags)
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
struct IP_Conn_s* IP_RMD_CreateF(const struct IP_Proto_s* a_Proto, const char* const a_Host, const uint32_t a_Port, const char* const a_Options, const uint32_t a_Flags)
{
	struct IP_Addr_s Addr;
	struct IP_Conn_s* New;
	uint16_t Port;
	I_NetSocket_t* Socket;
	IP_RmdData_t* Data;
	int32_t i;
	bool_t IsV6;
	
	/* Valid Port */
	Port = a_Port;
	if (!Port)
	{
		if (M_CheckParm("-port"))
			if (M_IsNextParm())
				Port = C_strtoi32(M_GetNextParm(), NULL, 10);
		
		if (!Port || Port < 0 || Port >= 65536)
			// Random port for client
			if (!(a_Flags & IPF_INPUT))
			{
				Port = D_CMakePureRandom() & UINT32_C(0x7FFF);
				Port |= UINT32_C(0x8000);
			}
			
			// If server, use default port
			else
				Port = 29500;
	}
	
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
		Socket = I_NetOpenSocket((IsV6 ? INSF_V6 : 0), ((a_Flags & IPF_INPUT) ? &Addr.Private : NULL), Port + i);
	
		// Worked?
		if (Socket)
			break;
	}
	
	// No socket created
	if (!Socket)
		return NULL;
	
	// Failed?
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
void IP_RMD_DeleteConnF(const struct IP_Proto_s* a_Proto, struct IP_Conn_s* const a_Conn)
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
void IP_RMD_RunConnF(const struct IP_Proto_s* a_Proto, struct IP_Conn_s* const a_Conn)
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
	{
	}
	
	// We are a client
	else
	{
		// If not connected, attempt connect
		if (!Data->IsConnected)
		{
			// Expired connect attempt
			if (!Data->LastConnectTry || g_ProgramTic >= Data->LastConnectTry)
			{
				// Set new timeout to now
				Data->LastConnectTry = g_ProgramTic + CONNRETRYDELAY;
				
				// Print Message
				CONL_PrintF("{zConnecting to {9%s{z...\n", IP_AddrToString(&Data->Conn->RemAddr));
				
				// Send simple connection packet to the server
				D_BSBaseBlock(Data->BSnet, "SCON");
				D_BSwu32(Data->BSnet, g_Splits[0].ProcessID);
				D_BSRecordNetBlock(Data->BSnet, SERVERADDR(Data));
			}
		}
		
		// We are connected
		else
		{
		}
	}
	
	/* Flush stream */
	// This is so packets are sent, etc.
	D_BSFlushStream(Data->BS);
}

/* IP_RMD_ConnTrashIPF() -- Trash IP Address */
void IP_RMD_ConnTrashIPF(const struct IP_Proto_s* a_Proto, struct IP_Conn_s* const a_Conn, I_HostAddress_t* const a_Addr)
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

