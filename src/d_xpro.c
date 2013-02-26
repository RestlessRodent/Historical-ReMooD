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

/*---------------------------------------------------------------------------*/

/* DS_DoMaster() -- Handles master connection */
static void DS_DoMaster(D_XDesc_t* const a_Desc)
{
}

/* DS_DoSlave() -- Handles slave connection */
static void DS_DoSlave(D_XDesc_t* const a_Desc)
{
	/* Not Syncronized? */
	if (!a_Desc->Data.Slave.Synced)
	{
	}
	
	/* Synced */
	else
	{
	}
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
	uint32_t Flags;
} c_CSPacks[] =
{
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
				Flags |= PF_SLAVE;
				
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
				// Check Flags
				for (b = 0; b < 31; b++)
					if (c_CSPacks[i].Flags & (1 << b))
						if ((c_CSPacks[i].Flags & (1 << b)) != (Flags & (1 << b)))
							break;
				
				// Mismatch?
				if (b >= 31)
					continue;
				
				// Call handler function
			}
			
			// Not handled?
			if (devparm)
				if (!c_CSPacks[i].Header)
					CONL_PrintF("Unknown \'%c%c%c%c\'\n", Header[0], Header[1], Header[2], Header[3]);
		}
	} while (Continue);
}

/*---------------------------------------------------------------------------*/

