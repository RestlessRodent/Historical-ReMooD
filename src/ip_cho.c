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
// Portions Copyright (C) 2005-2013 Simon Howard
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
// DESCRIPTION: Chocolate Doom Protocol Implementation

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
#include "p_demcmp.h"
#include "p_info.h"

/****************
*** CONSTANTS ***
****************/

#define MAXPACKET		1500					// Max packet size

#define NET_MAGIC_NUMBER 3436803284U

/* net_packet_type_t -- Type of packet this is */
typedef enum 
{
    NET_PACKET_TYPE_SYN,
    NET_PACKET_TYPE_ACK,
    NET_PACKET_TYPE_REJECTED,
    NET_PACKET_TYPE_KEEPALIVE,
    NET_PACKET_TYPE_WAITING_DATA,
    NET_PACKET_TYPE_GAMESTART,
    NET_PACKET_TYPE_GAMEDATA,
    NET_PACKET_TYPE_GAMEDATA_ACK,
    NET_PACKET_TYPE_DISCONNECT,
    NET_PACKET_TYPE_DISCONNECT_ACK,
    NET_PACKET_TYPE_RELIABLE_ACK,
    NET_PACKET_TYPE_GAMEDATA_RESEND,
    NET_PACKET_TYPE_CONSOLE_MESSAGE,
    NET_PACKET_TYPE_QUERY,
    NET_PACKET_TYPE_QUERY_RESPONSE,
} net_packet_type_t;

/*****************
*** STRUCTURES ***
*****************/

/* IP_Choco_t -- Chocolate Doom Data */
typedef struct IP_Choco_s
{
	IP_Conn_t* Conn;							// Connection
	I_NetSocket_t* Socket;						// Socket to server
	
	uint8_t pD[2][MAXPACKET];					// Packet Data
	uint8_t* pC[2];								// Current read position
	uint8_t* pE[2];								// End of packet
} IP_Choco_t;

/****************
*** FUNCTIONS ***
****************/

/* IP_CHO_VerifyF() -- Verifies Settings */
bool_t IP_CHO_VerifyF(const IP_Proto_t* a_Proto, const char* const a_Host, const uint32_t a_Port, const char* const a_Options, const uint32_t a_Flags)
{
	uint32_t RealPort;
	
	/* Cannot host servers */
	if (a_Flags & IPF_INPUT)
		return false;
	
	/* Check Port */
	// Get the real port
	if (!a_Port)
		RealPort = 2342;
	else
		RealPort = a_Port;
	
	// Range
	if (RealPort <= 0 || RealPort >= 65536)
		return false;
	
	/* Success */
	return true;
}

/* IP_CHO_CreateF() -- Creates connection */
IP_Conn_t* IP_CHO_CreateF(const IP_Proto_t* a_Proto, const char* const a_Host, const uint32_t a_Port, const char* const a_Options, const uint32_t a_Flags)
{
	struct IP_Addr_s Addr;
	IP_Conn_t* New;
	uint16_t Port, ClPort;
	I_NetSocket_t* Socket;
	IP_Choco_t* Data;
	int32_t i;
	
	/* Valid Port */
	Port = a_Port;
	ClPort = 0;
	
	// Default remote port
	if (!Port)
		Port = 2342;
	
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
	
	/* Attempt Host Resolution */
	if (!IP_UDPResolveHost(a_Proto, &Addr, a_Host, Port))
		return NULL;
	
	/* Setup Socket */
	// Try making sockets
	for (i = 0; i < IPMAXSOCKTRIES; i++)
	{
		// Create socket to server
		Socket = I_NetOpenSocket(0, NULL, ClPort + i);
	
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
	
	/* Return it */
	return New;
}

/* IP_CHO_RunConnF() -- Runs connection */
void IP_CHO_RunConnF(const IP_Proto_t* a_Proto, IP_Conn_t* const a_Conn)
{
}

/* IP_CHO_DeleteConnF() -- Runs connection */
void IP_CHO_DeleteConnF(const IP_Proto_t* a_Proto, IP_Conn_t* const a_Conn)
{
	/* Check */
	if (!a_Proto || !a_Conn)
		return;
}

/* IP_CHO_SameAddrF() -- Same address? */
bool_t IP_CHO_SameAddrF(const IP_Proto_t* a_Proto, const IP_Addr_t* const a_A, const IP_Addr_t* const a_B)
{
	return I_NetCompareHost((I_HostAddress_t*)a_A->Private.Data, (I_HostAddress_t*)a_B->Private.Data);
}

