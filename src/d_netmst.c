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
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Master Server and Other interaction protocols

/***************
*** INCLUDES ***
***************/

#include "d_net.h"
#include "d_netmst.h"
#include "doomstat.h"

/****************
*** CONSTANTS ***
****************/

#define SNSERVERTIMEOUT (TICRATE * 120)			// Time before a server goes poof

/*************
*** LOCALS ***
*************/

static I_NetSocket_t* l_MCast[2];				// Multi-cast sockets
static D_BS_t* l_MBS[2];						// Multi-cast block

static D_SNServer_t** l_Servers;				// Visible servers
static int32_t l_NumServers;					// Count of them

/****************
*** FUNCTIONS ***
****************/

/* I_BootHTTPSpy() -- Permits game spying via HTTP */
bool_t I_BootHTTPSpy(void)
{
	uint16_t Port;
	
	/* Only allow if -spy is passed */
	if (!M_CheckParm("-spy"))
		return false;
	
	// Takes parameter
	if (!M_IsNextParm())
		return false;
	
	// Read Port
	Port = C_strtou32(M_GetNextParm(), NULL, 10);
	
	// Bad port?
	if (!Port)
		return false;
	
	/* Create TCP Server */
	return false;
}

/* I_UpdateHTTPSpy() -- Handles HTTP Spy */
void I_UpdateHTTPSpy(void)
{
}


/* D_SNOpenMCast() -- Open multicast sockets */
void D_SNOpenMCast(void)
{
	int32_t i;
	
	/* Variables */
	
	/* Setup Multicast */
	for (i = 0; i < 2; i++)
		if (!(l_MCast[i] = I_NetOpenMultiCastSocket(!!i, 29500)))
			CONL_PrintF("Failed to open multi-cast for IPv%i\n", (!i ? 4 : 6));
		else
			l_MBS[i] = D_BSCreateNetStream(l_MCast[i]);
}

/* D_SNDoMultiCast() -- Do multicasting */
void D_SNDoMultiCast(void)
{
#define BUFSIZE 128
	char Buf[BUFSIZE];
	static tic_t LastTime;
	int32_t i;
	D_BS_t* BS;
	char Header[5];
	I_HostAddress_t Addr;
	D_SNServer_t* Server;
	
	/* Every 10 seconds */
	if (g_ProgramTic < LastTime)
		return;
	
	// Set next update time
	LastTime = g_ProgramTic + (TICRATE * 10);
	
	/* Handle interfaces */
	for (i = 0; i < 2; i++)
	{
		// Get it
		if (!(BS = l_MBS[i]))
			continue;
		
		// Try reading block
		memset(Header, 0, sizeof(Header));
		memset(&Addr, 0, sizeof(Addr));
		while (D_BSPlayNetBlock(BS, Header, &Addr))
		{
			// Try to find this server
			Server = D_SNFindServerByAddr(&Addr);
			
			// If it does not exist, create it
			if (!Server)
				Server = D_SNCreateServer(&Addr);
			
			// Still oops,
			if (!Server)
				continue;
			
			// Set server last seen
			Server->UpdatedAt = g_ProgramTic;
		}
	}
#undef BUFSIZE
}

/* D_SNFindServerByAddr() -- Finds server by address */
D_SNServer_t* D_SNFindServerByAddr(I_HostAddress_t* const a_Addr)
{
	int32_t i;
	D_SNServer_t* Server;
	
	/* Check */
	if (!a_Addr)
		return NULL;
	
	/* Loop */
	for (i = 0; i < l_NumServers; i++)
		if ((Server = l_Servers[i]))
			if (I_NetCompareHost(a_Addr, &Server->Addr))
				return Server;
	
	/* Failed */
	return NULL;
}

/* D_SNFindServerByIndex() -- Finds server by index */
D_SNServer_t* D_SNFindServerByIndex(const int32_t a_Index)
{
	/* Check */
	if (a_Index < 0 || a_Index >= l_NumServers)
		return NULL;
	
	/* Return specific index */
	return l_Servers[a_Index];
}

/* D_SNCreateServer() -- Creates a new server */
D_SNServer_t* D_SNCreateServer(I_HostAddress_t* const a_Addr)
{
	D_SNServer_t* New;
	int32_t i;
	
	/* Check */
	if (!a_Addr)
		return NULL;
	
	/* See if it already exists */
	if ((New = D_SNFindServerByAddr(a_Addr)))
		return New;
	
	/* Create new reference */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	// Copy Address
	memmove(&New->Addr, a_Addr, sizeof(*a_Addr));
	
	// Set times
	New->FirstSeen = New->UpdatedAt = g_ProgramTic;
	New->OutAt = g_ProgramTic + SNSERVERTIMEOUT;
	
	/* Add to list */
	for (i = 0; i < l_NumServers; i++)
		if (!l_Servers[i])
		{
			l_Servers[i] = New;
			break;
		}
	
	// No room?
	if (i >= l_NumServers)
	{
		Z_ResizeArray((void**)&l_Servers, sizeof(*l_Servers),
			l_NumServers, l_NumServers + 1);
		l_Servers[l_NumServers++] = New;
	}
	
	/* Return it */
	return New;
}

/* D_SNUpdateServers() -- Updates Servers */
void D_SNUpdateServers(void)
{
	int32_t i;
	D_SNServer_t* Server;
	
	/* Go through servers in list */
	for (i = 0; i < l_NumServers; i++)
	{
		// Get this server
		if (!(Server = l_Servers[i]))
			continue;
		
		// Ping server, if needed
	}
}

