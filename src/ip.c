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
// Copyright (C) 2012 GhostlyDeath <ghostlydeath@remood.org>
// Portions Copyright (C) Odamex <http://odamex.net/>
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

#define NUMPROTOS 2								// Number of protocols

/*****************
*** STRUCTURES ***
*****************/

/*************
*** LOCALS ***
*************/

static const IP_Proto_t l_Protos[NUMPROTOS] =
{
	// ReMooD
	{
		"remood",
		
		IP_RMD_VerifyF,
		IP_RMD_CreateF,
	},
	
	// Odamex
	{
		"odamex",
		
		IP_ODA_VerifyF,
		IP_ODA_CreateF,
	},
};

/****************
*** FUNCTIONS ***
****************/

/* IP_Init() -- Initialize protocols */
void IP_Init(void)
{
}


/* IP_ProtoByName() -- Finds protocol by name */
const struct IP_Proto_s* IP_ProtoByName(const char* const a_Name)
{
	size_t i;
	
	/* Check */
	if (!a_Name)
		return NULL;
	
	/* Find in list */
	for (i = 0; i < NUMPROTOS; i++)
		if (!strcasecmp(a_Name, l_Protos[i].Name))
			return &l_Protos[i];
	
	/* Not found */
	return NULL;
}

/* IP_Create() -- Creates a new protocol connectable */
struct IP_Conn_s* IP_Create(const char* const a_URI, const uint32_t a_Flags)
{
#define PROTOSIZE 16
#define HOSTSIZE 64
#define PORTBUF 8
	char ProtoBuf[PROTOSIZE];
	char HostBuf[HOSTSIZE];
	char PortBuf[PORTBUF];
	char* p, *q, *x, *s;
	size_t i;
	uint32_t Port;
	const struct IP_Proto_s* Proto;
	
	/* Check */
	if (!a_URI)
		return NULL;
	
	/* Debug */
	if (g_NetDev)
		CONL_OutputUT(CT_NETWORK, DSTR_IPC_CREATECONN, "%s%08x\n", a_URI, a_Flags); 
	
	/* Find protocol, if any */
	p = strchr(a_URI, ':');
	
	// Make sure it is ://
	if (p && !strncmp(p, "://", 3))
	{
		q = p;
		q += 3;
	}
	else
		p = NULL;
	
	// Not found, assume remood
	if (!p)
	{
		strncpy(ProtoBuf, "remood", PROTOSIZE);
		q = a_URI;
	}
	
	// Copy only the protocol
	else
	{
		for (i = 0, x = a_URI; x < p; x++)
			if (i < PROTOSIZE - 1)
				ProtoBuf[i++] = *x;
		ProtoBuf[i++] = 0;
	}
	
	/* Check */
	if (!*q)
		return NULL;
	
	/* Extract Host */
	p = strchr(q, ':');
	
	// No port
	if (!p)
		strncpy(HostBuf, q, HOSTSIZE);
	
	// There is a port
	else
	{
		for (i = 0, x = q; x < p; x++)
			if (i < HOSTSIZE - 1)
				HostBuf[i++] = *x;
		HostBuf[i++] = 0;
	}
	
	/* Extract Port */
	// No port
	if (!p || (p && (!*p || (*p && (!*(p+1) || *(p+1) == '/')))))
	{
		// Find slash for later work
		s = strchr(q, '/');
		
		// Use default port
		Port = 0;
	}
	
	// There is a port
	else
	{
		// Find '/'
		p++;
		q = strchr(p, '/');
		
		// None
		if (!q)
			q = p + strlen(p);
		else
			s = q;
		
		// Extract
		for (i = 0, x = p; x < q; x++)
			if (i < HOSTSIZE - 1)
				PortBuf[i++] = *x;
		PortBuf[i++] = 0;
		
		// Convert
		Port = C_strtou32(PortBuf, NULL, 10);
	}
	
	/* Options? */
	if (!s)
	{
		s = &ProtoBuf[PROTOSIZE - 1];
		*s = 0;
	}
	
	/* Find Protocol */
	Proto = IP_ProtoByName(ProtoBuf);
	
	// Not found?
	if (!Proto)
		return NULL;
	
	/* Verify Flags */
	if (!Proto->VerifyF(Proto, HostBuf, Port, s, a_Flags))
		return NULL;
	
	/* Create connection */
	return Proto->CreateF(Proto, HostBuf, Port, s, a_Flags);
#undef PROTOSIZE
#undef HOSTSIZE
}

/* IP_Destroy() -- Destroys protocol connection */
void IP_Destroy(struct IP_Conn_s* const a_Conn)
{
	/* Check */
	if (!a_Conn)
		return;
}

/* IP_ConnById() -- Returns connection by ID */
struct IP_Conn_s* IP_ConnById(const uint32_t a_UUID)
{
	size_t i;
	
	/* Check */
	if (!a_UUID)
		return NULL;
}

/* IP_ConnRun() -- Runs connection handling */
void IP_ConnRun(struct IP_Conn_s* const a_Conn)
{
	/* Check */
	if (!a_Conn)
		return;
}

