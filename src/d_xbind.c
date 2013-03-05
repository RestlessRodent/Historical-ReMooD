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
#include "m_argv.h"
#include "d_netcmd.h"

/**************
*** GLOBALS ***
**************/

D_XDesc_t* g_XSocket = NULL;					// Master Socket
D_XEndPoint_t** g_XEP = NULL;					// End points
size_t g_NumXEP = 0;							// Number of them

/****************
*** FUNCTIONS ***
****************/

/* D_XBHasConnection() -- A connection has been established */
bool_t D_XBHasConnection(void)
{
	return !!g_XSocket;
}

/* DS_XBInitSocket() -- Initializes Socket */
static void DS_XBInitSocket(D_XDesc_t* const a_Desc, I_NetSocket_t* const a_Sock, I_HostAddress_t* const a_Addr)
{
	/* Setup standard stuff */
	a_Desc->Socket = a_Sock;
	a_Desc->StdBS = D_BSCreateNetStream(a_Desc->Socket);
	a_Desc->RelBS = D_BSCreateReliableStream(a_Desc->StdBS);
	
	/* Copy Addr */
	memmove(&a_Desc->BoundTo, a_Addr, sizeof(*a_Addr));
}

/* D_XBValidIP() -- IP Address is valid? */
bool_t D_XBValidIP(I_HostAddress_t* const a_Addr)
{
	/* IPv4 */
	if (a_Addr && a_Addr->IPvX == INIPVN_IPV4)
	{
		// Broadcast or zero address
		if (a_Addr->Host.v4.b[3] == 0 || a_Addr->Host.v4.b[3] == 255)
			return false;
		
		// Multicast
		if (a_Addr->Host.v4.b[0] == 224)
			return false;
	}
	
	/* IPv6 */
	else if (a_Addr && a_Addr->IPvX == INIPVN_IPV6)
	{
		// Multicast and friends
		if ((a_Addr->Host.v6.Addr.s[0] & UINT16_C(0xFFF0)) == UINT16_C(0xFF00))
			return false;
	}
	
	/* Invalid type is bad */
	else
		return false;
	
	/* Otherwise address appears OK */
	return true;
}

/* D_XBWaitForCall() -- Waits for incoming connection */
bool_t D_XBWaitForCall(I_HostAddress_t* const a_BindTo)
{
	I_NetSocket_t* SvSock;
	I_HostAddress_t BindAddr;
	uint32_t SockFlags;
	D_XDesc_t* New;
	uint16_t Port, End;
	
	/* Destroy old socket, if any */
	D_XBSocketDestroy();
	
	/* Socket always listens */
	SockFlags = INSF_LISTEN;
	
	/* Initialize binding address */
	// To determine how we want to host this server
	memset(&BindAddr, 0, sizeof(BindAddr));
	if (a_BindTo && D_XBValidIP(a_BindTo))
		memmove(&BindAddr, a_BindTo, sizeof(BindAddr));
	
	// Hosting IPv6 Server
	if (M_CheckParm("-ipv6") || (BindAddr.IPvX && BindAddr.IPvX == INIPVN_IPV6))
		SockFlags |= INIPVN_IPV6;
	
	/* IPv6 enabled but address not v6 */
	if (BindAddr.IPvX)
		if ((SockFlags & INIPVN_IPV6) && BindAddr.IPvX != INIPVN_IPV6)
			return false;
	
	/* Forced Port */
	Port = 0;
	if (BindAddr.IPvX && BindAddr.Port)
		Port = BindAddr.Port;
	
	// Passed by argument?
	if (!Port)
		if (M_CheckParm("-port") && M_IsNextParm())
			Port = C_strtoi32(M_GetNextParm(), NULL, 10);
	
	// Still failed
	if (!Port)
		Port = UINT16_C(29500);
		
	// Last port to check
	End = Port + MAXPLAYERS;
	if (End < Port)
		End = UINT16_C(65535);
	
	/* Attempt socket creation */
	// Socket creation loop
	SvSock = NULL;
	for (; Port <= End; Port++)
	{
		SvSock = I_NetOpenSocket(SockFlags, (BindAddr.IPvX ? &BindAddr : NULL), Port);
		
		// Created?
		if (SvSock)
			break;
	}
	
	// Failed to create
	if (!SvSock)
		return false;
	
	/* It worked, so create info */
	New = g_XSocket = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	New->Master = true;
	
	// Init
	DS_XBInitSocket(New, SvSock, &BindAddr);
	
	// Success!
	return true;
}

/* D_XBCallHost() -- Connects to another server */
bool_t D_XBCallHost(I_HostAddress_t* const a_ToCall, const uint32_t a_GameID)
{
	I_NetSocket_t* ClSock;
	I_HostAddress_t ConnAddr;
	uint32_t SockFlags;
	uint16_t Port, End;
	D_XDesc_t* New;
	
	/* Requires address */
	if (!a_ToCall)
		return false;
	
	// No IP to call? Invalid Addr?
	if (!a_ToCall->IPvX || !D_XBValidIP(a_ToCall))
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
		SockFlags |= INIPVN_IPV6;
		
	/* IPv6 enabled but address not v6 */
	if (ConnAddr.IPvX)
		if ((SockFlags & INIPVN_IPV6) && ConnAddr.IPvX != INIPVN_IPV6)
			return false;
	
	/* Forced Port */
	Port = 0;
	
	// Passed by argument?
	if (!Port)
		if (M_CheckParm("-port") && M_IsNextParm())
			Port = C_strtoi32(M_GetNextParm(), NULL, 10);
	
	// Still failed
	if (!Port)
	{
		Port = D_CMakePureRandom();
		Port |= UINT16_C(0x8000);
	}
		
	// Last port to check
	End = Port + MAXPLAYERS;
	if (End < Port)
		End = UINT16_C(65535);
	
	/* Attempt socket creation */
	// Socket creation loop
	for (; Port <= End; Port++)
	{
		// Open a socket on said random port, but bind to no address.
		ClSock = I_NetOpenSocket(SockFlags, NULL, Port);
		
		// Created?
		if (ClSock)
			break;
	}
	
	// Failed to create
	if (!ClSock)
		return false;
	
	/* It worked, so create info */
	New = g_XSocket = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	New->Master = false;
	
	// Init
	DS_XBInitSocket(New, ClSock, &ConnAddr);
	
	// Accept reliable packets from the server
	D_BSStreamIOCtl(New->RelBS, DRBSIOCTL_ADDHOST, (intptr_t)&ConnAddr);
	
	// Success!
	return true;
}

/* D_XBSocketDestroy() -- Destroys the connection socket */
void D_XBSocketDestroy(void)
{
	int32_t i;	
	
	/* Check */
	if (!g_XSocket)
		return;
		
	/* Remove endpoints */
	for (i = 0; i < g_NumXEP; i++)
		if (g_XEP[i])
			D_XBDelEndPoint(g_XEP[i], "Server disconnect.");
	
	/* Delete all file transfers */
	D_XFStopAll();
	
	/* Delete everything in reverse order */
	D_BSCloseStream(g_XSocket->RelBS);
	D_BSCloseStream(g_XSocket->StdBS);
	I_NetCloseSocket(g_XSocket->Socket);
	
	/* Free socket */
	Z_Free(g_XSocket);
	g_XSocket = NULL;
	
	/* Cleanup some things */
	D_XPCleanup();
}

/* D_XBDropHost() -- Drops host from the reliable buffer */
void D_XBDropHost(I_HostAddress_t* const a_Addr)
{
	/* Check */
	if (!g_XSocket || !a_Addr)
		return;
	
	/* Remove from reliable */
	D_BSStreamIOCtl(g_XSocket->RelBS, DRBSIOCTL_DROPHOST, (intptr_t)a_Addr);
}

/* D_XBRouteToServer() -- Returns route to server */
D_BS_t* D_XBRouteToServer(D_BS_t** const a_StdBSP, I_HostAddress_t** const a_AddrP)
{
	int32_t i;
	
	/* Check */
	if (!g_XSocket)
		return NULL;
	
	/* If we are server, return NULL */
	if (D_XNetIsServer())
		return NULL;
	
	/* Depending on the connection type */
	// Master -- The true server is on the endpoint side
	if (g_XSocket->Master)
	{
		for (i = 0; i < g_NumXEP; i++)
			if (g_XEP[i])
			{
				if (a_StdBSP)
					*a_StdBSP = g_XEP[i]->Desc->StdBS;
				if (a_AddrP)
					*a_AddrP = &g_XEP[i]->Addr;
				return g_XEP[i]->Desc->RelBS;
			}
	}
	
	// Slave -- Server is where the socket says it is
	else
	{
		if (a_StdBSP)
			*a_StdBSP = g_XSocket->StdBS;
		if (a_AddrP)
			*a_AddrP = &g_XSocket->BoundTo;
		return g_XSocket->RelBS;
	}
	
	/* Not Found */
	return NULL;
}

/* D_XBNewEndPoint() -- Creates new endpoint */
D_XEndPoint_t* D_XBNewEndPoint(D_XDesc_t* const a_Desc, I_HostAddress_t* const a_Addr)
{
	D_XEndPoint_t* New;
	int32_t i;
	
	/* Check */
	if (!a_Desc || !a_Addr)
		return NULL;
	
	/* Allocate and setup ID */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	New->HostID = D_XNetMakeID(0);
	New->Desc = a_Desc;
	memmove(&New->Addr, a_Addr, sizeof(New->Addr));
	
	/* Place into list */
	for (i = 0; i < g_NumXEP; i++)
		if (!g_XEP[i])
		{
			g_XEP[i] = New;
			break;
		}
	
	// No room? Resize array and place at end
	if (i >= g_NumXEP)
	{
		Z_ResizeArray((void**)&g_XEP, sizeof(*g_XEP),
			g_NumXEP, g_NumXEP + 1);
		g_XEP[g_NumXEP++] = New;
	}
	
	/* Return it */
	return New;
}

/* D_XBDelEndPoint() -- Deletes endpoint of player */
void D_XBDelEndPoint(D_XEndPoint_t* const a_XEP, const char* const a_Reason)
{
	int32_t i;
	
	/* Check */
	if (!a_XEP)
		return;
	
	/* No descriptor? */
	if (!a_XEP->Desc)
		return;
	
	/* Send them disconnect notice */
	D_XPSendDisconnect(a_XEP->Desc, a_XEP->Desc->RelBS, &a_XEP->Addr, a_Reason);
	
	/* Look in list form them */
	for (i = 0; i < g_NumXEP; i++)
		if (g_XEP[i] == a_XEP)
			g_XEP[i] = NULL;
	
	/* Delete endpoint */
	Z_Free(a_XEP);
}

/* D_XBEndPointForAddr() -- Locates endpoint by address */
D_XEndPoint_t* D_XBEndPointForAddr(I_HostAddress_t* const a_Addr)
{
	int32_t i;
	
	/* Check */
	if (!a_Addr)
		return NULL;
	
	/* Look in list */
	for (i = 0; i < g_NumXEP; i++)
		if (g_XEP[i])
			if (I_NetCompareHost(a_Addr, &g_XEP[i]->Addr))
				return g_XEP[i];
	
	/* Not found */
	return NULL;
}

/* D_XBEndPointForID() -- Locates endpoint by ID */
D_XEndPoint_t* D_XBEndPointForID(const uint32_t a_ID)
{
	int32_t i;
	
	/* Check */
	if (!a_ID)
		return NULL;
	
	/* Look in list */
	for (i = 0; i < g_NumXEP; i++)
		if (g_XEP[i])
			if (g_XEP[i]->HostID == a_ID)
				return g_XEP[i];
	
	/* Not found */
	return NULL;
}

