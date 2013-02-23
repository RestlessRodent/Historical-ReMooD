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
// DESCRIPTION: Extended Network Protocol -- Binding

/***************
*** INCLUDES ***
***************/

#include "d_xpro.h"

/**************
*** GLOBALS ***
**************/

D_XDesc_t* g_XSocket = NULL;					// Master Socket

/****************
*** FUNCTIONS ***
****************/

/* D_XBHasConnection() -- A connection has been established */
bool_t D_XBHasConnection(void)
{
	return !!g_XSocket;
}

/* D_XBWaitForCall() -- Waits for incoming connection */
bool_t D_XBWaitForCall(I_HostAddress_t* const a_BindTo)
{
	I_NetSocket_t* SvSock;
	I_HostAddress_t BindAddr;
	uint32_t SockFlags;
	
	/* Destroy old socket, if any */
	D_XBSocketDestroy();
	
	/* Socket always listens */
	SockFlags = INSF_LISTEN;
	
	/* Initialize binding address */
	// To determine how we want to host this server
	memset(&BindAddr, 0, sizeof(BindAddr));
	if (a_BindTo)
		memmove(&BindAddr, a_BindTo, sizeof(BindAddr));
	
	// Hosting IPv6 Server
	if (M_CheckParm("-ipv6") || (BindAddr.IPvX && BindAddr.IPvX == INIPVN_IPV6))
		SockFlags = INIPVN_IPV6;
	
	/* IPv6 enabled but address not v6 */
	if (BindAddr.IPvX)
		if ((SockFlags & INIPVN_IPV6) && BindAddr.IPvX != INIPVN_IPV6)
			return false;
	
	/* Attempt socket creation */
	return false;
}

/* D_XBCallHost() -- Connects to another server */
bool_t D_XBCallHost(I_HostAddress_t* const a_ToCall, const uint32_t a_GameID)
{
	I_NetSocket_t* ClSock;
	I_HostAddress_t ConnAddr;
	uint32_t SockFlags;
	
	/* Requires address */
	if (!a_ToCall)
		return false;
	
	/* Destroy old socket, if any */
	D_XBSocketDestroy();
	
	/* By default, no flags used */
	SockFlags = 0;
	
	/* Initialize connection address */
	// To determine how we want to call this server
	memset(&ConnAddr, 0, sizeof(ConnAddr));
	if (a_ToCall)
		memmove(&ConnAddr, a_ToCall, sizeof(ConnAddr));
	
	// Connecting to IPv6 host?
	if (M_CheckParm("-ipv6") || (ConnAddr.IPvX && ConnAddr.IPvX == INIPVN_IPV6))
		SockFlags = INIPVN_IPV6;
		
	/* IPv6 enabled but address not v6 */
	if (ConnAddr.IPvX)
		if ((SockFlags & INIPVN_IPV6) && ConnAddr.IPvX != INIPVN_IPV6)
			return false;
	
	return false;
}

/* D_XBSocketDestroy() -- Destroys the connection socket */
void D_XBSocketDestroy(void)
{
}

/* D_XBDropHost() -- Drops host from the reliable buffer */
void D_XBDropHost(I_HostAddress_t* const a_Addr)
{
	/* Check */
	if (!g_XSocket || !a_Addr)
		return;
	
	/* Remove from reliable */
	D_BSStreamIOCtl(g_XSocket->RelBS, DRBSIOCTL_DROPHOST, a_Addr);
}

/* D_XBPathToXPlay() -- Returns path to XPlayer */
D_XDesc_t* D_XBPathToXPlay(D_XPlayer_t* const a_XPlay, I_HostAddress_t** const a_HostPP, D_BS_t** const a_StdBSPP, D_BS_t** const a_RelBSPP)
{
}

