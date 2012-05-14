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

/* D_NetController_t() -- Network Controller */
typedef struct D_NetController_s
{
	/* Arbitration for players */
	struct D_NetPlayer_s** Arbs;				// Players this controller controls
	size_t NumArbs;								// Number of them
	
	/* Socket Stuff */
	I_NetSocket_t* NetSock;						// Network Socket
	I_NetHost_t NetTo;							// Where to send to
	I_NetHost_t NetFrom;						// Where to recieve from
	
	/* Streaming */
	bool_t IsLocal;								// Is this a local thing?
	struct D_RBlockStream_s* BlockStream;		// Block stream
	uint32_t Ping;								// Ping delay
	
	/* Game Stuff */
	uint8_t VerLeg, VerMaj, VerMin, VerRel;		// Versions of ReMooD
	bool_t IsServer;							// Is the server
	bool_t IsServerLink;						// This controller links to server (arb)
} D_NetController_t;

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

typedef void (*D_NCQCFunc_t)(void* const a_Data);

/*** GLOBALS ***/

extern uint32_t g_NetStat[4];					// Network stats

/*** FUNCTIONS ***/

D_NetController_t* D_NCAllocController(void);
void D_NCUpdate(void);
void D_NCAddQueueCommand(const D_NCQCFunc_t a_Func, void* const a_Data);
void D_NCRunCommands(void);

D_NetController_t* D_NCGetLocal(void);
D_NetController_t* D_NCGetServer(void);
D_NetController_t* D_NCGetServerLink(void);

void D_NCCommRequestMap(const char* const a_Map);

/*** NCSR Funcs ***/
void D_NCSR_RequestNewPlayer(struct D_ProfileEx_s* a_Profile);

/*** NCQC Funcs ***/
void D_NCQC_MapChange(void* const a_Data);

#endif							/* __D_NET_H__ */

