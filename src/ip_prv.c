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
// DESCRIPTION: Standard Wrapping

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "doomdef.h"
#include "i_util.h"
#include "d_netcmd.h"
#include "d_net.h"
#include "console.h"
#include "ip_prv.h"

/****************
*** CONSTANTS ***
****************/

/*****************
*** STRUCTURES ***
*****************/

/*************
*** LOCALS ***
*************/

/****************
*** FUNCTIONS ***
****************/

/* IP_UDPResolveHost() -- Resolves UDP Host */
bool_t IP_UDPResolveHost(const IP_Proto_t* a_Proto, IP_Addr_t* const a_Dest, const char* const a_Name, const uint32_t a_Port)
{
	I_HostAddress_t* AddrData;
	
	/* Check */
	if (!a_Proto || !a_Dest || !a_Name || !a_Port)
		return false;
	
	/* Clear Address */
	memset(a_Dest, 0, sizeof(*a_Dest));
	
	/* Set address data */
	AddrData = (I_HostAddress_t*)(&a_Dest->Private);
	
	/* Resolve */
	if (!I_NetNameToHost(NULL, AddrData, a_Name))
		return false;
	
	/* Set Port */
	AddrData->Port = a_Port;
	a_Dest->IsValid = true;
	a_Dest->Handler = a_Proto;
	
	a_Dest->Port = a_Port;
	snprintf(a_Dest->HostName, IPADDRHOSTLEN - 1, "%s:%u", a_Name, AddrData->Port);
	
	/* Success */
	return true;
}

/* IP_AddrToString() -- Converts address to string */
char* IP_AddrToString(IP_Addr_t* const a_Addr)
{
#define BUFSIZE 128
	static char Buf[BUFSIZE];
	
	/* Clear Buffer */
	memset(Buf, 0, sizeof(Buf));
	
	/* Illegal Address */
	if (!a_Addr)
		snprintf(Buf, BUFSIZE - 1, "unknown://");
	
	/* Legal */
	else
	{
		strncat(Buf, a_Addr->Handler->Name, BUFSIZE - 1);
		strncat(Buf, "://", BUFSIZE - 1);
		strncat(Buf, a_Addr->HostName, BUFSIZE - 1);
	}
	
	/* Always return the buffer */
	return Buf;	
#undef BUFSIZE
}

/* IP_IHostToIPAddr() -- Converts address (I) to address (IP) */
bool_t IP_IHostToIPAddr(IP_Addr_t* const a_Dest, I_HostAddress_t* const a_Host, const IP_Proto_t* a_Proto)
{
	I_HostAddress_t* AddrData;
	
	/* Check */
	if (!a_Proto || !a_Dest || !a_Host)
		return false;
	
	/* Clear Address */
	memset(a_Dest, 0, sizeof(*a_Dest));
	
	/* Set address data */
	AddrData = (I_HostAddress_t*)(&a_Dest->Private);
	
	/* Setup fields */
	// Standard
	a_Dest->IsValid = true;
	a_Dest->Handler = a_Proto;
	a_Dest->Port = a_Host->Port;
	
	// Resolve hostname or plain copy if that fails
	if (!I_NetHostToName(NULL, a_Host, a_Dest->HostName, IPADDRHOSTLEN - 1))
		I_NetHostToString(a_Host, a_Dest->HostName, IPADDRHOSTLEN - 1);
	
	// IP Data
	memmove(AddrData, a_Host, sizeof(*AddrData));
	
	/* Success! */
	return true;
}

