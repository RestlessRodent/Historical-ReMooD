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
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION: Networking stuff.
//              part of layer 4 (transport) (tp4) of the osi model
//              assure the reception of packet and proceed a checksums

#ifndef __D_NET_H__
#define __D_NET_H__

/***************
*** INCLUDES ***
***************/

#include "i_net.h"
#include "i_util.h"

/*****************
*** STRUCTURES ***
*****************/

struct D_NetPlayer_s;
struct D_RBlockStream_s;
struct D_ProfileEx_s;

/*****************
*** PROTOTYPES ***
*****************/

void D_SyncNetDebugMessage(const char* const a_Format, ...);
bool_t D_SyncNetIsArbiter(void);
void D_SyncNetSetMapTime(const tic_t a_Time);
tic_t D_SyncNetMapTime(void);
tic_t D_SyncNetRealTime(void);
tic_t D_SyncNetAllReady(void);
bool_t D_SyncNetIsPaused(void);
bool_t D_SyncNetUpdate(void);
bool_t D_SyncNetIsSolo(void);

bool_t D_CheckNetGame(void);

/*****************************************************************************/

#define __REMOOD_NCSNET

/*** STRUCTURES ***/

struct player_s;

/* D_NCStreamPath_t -- Communication Paths */
typedef enum D_NCStreamPath_e
{
	DNCSP_READ,									// Standard Read Stream
	DNCSP_WRITE,								// Standard Write Stream
	DNCSP_PERFECTREAD,							// Perfect Read Stream
	DNCSP_PERFECTWRITE,							// Perfect Write Stream	
	
	NUMDNCSTREAMS
} D_NCStreamPath_t;

/* D_NetClient_t -- Network Client */
typedef struct D_NetClient_s
{
	/* Player Ownership */
	struct D_NetPlayer_s** Arbs;				// Arbitrating net players
	size_t NumArbs;								// Number of arbs
	
	/* Location and Pathing */
	I_HostAddress_t Address;					// Address to client
	I_NetSocket_t* NetSock;						// Network socket
	struct D_RBlockStream_s* CoreStream;		// Core stream
	struct D_RBlockStream_s* PerfectStream;		// Core stream
	struct D_RBlockStream_s* Streams[NUMDNCSTREAMS];	// Client Streams
	
	/* Flags */
	bool_t IsLocal;								// Local client
	bool_t IsServer;							// Is a server
} D_NetClient_t;

typedef void (*D_NCQCFunc_t)(void* const a_Data);

/*** GLOBALS ***/

extern uint32_t g_NetStat[4];					// Network stats

/*** FUNCTIONS ***/

D_NetClient_t* D_NCAllocClient(void);
D_NetClient_t* D_NCFindClientByNetPlayer(struct D_NetPlayer_s* const a_NetPlayer);
D_NetClient_t* D_NCFindClientByHost(I_HostAddress_t* const a_Host);
D_NetClient_t* D_NCFindClientByPlayer(struct player_s* const a_Player);

void D_NCUpdate(void);
void D_NCAddQueueCommand(const D_NCQCFunc_t a_Func, void* const a_Data);
void D_NCRunCommands(void);

/*** NSZZ Funcs ***/
void D_NSZZ_SendINFO(struct D_RBlockStream_s* a_Stream);

/*** NCSR Funcs ***/
void D_NCSR_RequestMap(const char* const a_Map);
void D_NCSR_RequestNewPlayer(struct D_ProfileEx_s* a_Profile);

/*** NCQC Funcs ***/
void D_NCQC_MapChange(void* const a_Data);

#endif							/* __D_NET_H__ */

