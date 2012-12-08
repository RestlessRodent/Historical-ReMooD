// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
// Copyright (C) 2012-2013 GhostlyDeath <ghostlydeath@remood.org>
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
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

/*****************
*** STRUCTURES ***
*****************/

/* IP_RmdData_t -- Connection Data */
typedef struct IP_RmdData_s
{
	struct IP_Conn_s* Conn;						// Connection
	I_NetSocket_t* Socket;						// Socket to server
} IP_RmdData_t;

/****************
*** FUNCTIONS ***
****************/

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
	
	/* Valid Port */
	Port = a_Port;
	if (!Port)
		Port = 29500;
	
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
	// Try making sockets
	for (i = 0; i < IPMAXSOCKTRIES; i++)
	{
		// Create socket to server
		Socket = I_NetOpenSocket(0, ((a_Flags & IPF_INPUT) ? &Addr.Private : NULL), Port + i);
	
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
	
	/* Return it */
	return New;
}

/* IP_RMD_RunConnF() -- Runs connection */
void IP_RMD_RunConnF(const struct IP_Proto_s* a_Proto, struct IP_Conn_s* const a_Conn)
{
	/* Check */
	if (!a_Proto || !a_Conn)
		return;
}

/* IP_RMD_DeleteConnF() -- Deletes connection */
void IP_RMD_DeleteConnF(const struct IP_Proto_s* a_Proto, struct IP_Conn_s* const a_Conn)
{
	/* Check */
	if (!a_Proto || !a_Conn)
		return;
}

