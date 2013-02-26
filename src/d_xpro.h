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
// DESCRIPTION: Extended Network Protocol

#ifndef __D_XPRO_H__
#define __D_XPRO_H__

/***************
*** INCLUDES ***
***************/

#include "d_net.h"
#include "d_block.h"
#include "i_util.h"

/*****************
*** STRUCTURES ***
*****************/

/* D_XDesc_t -- Socket descriptor */
typedef struct D_XDesc_s
{
	I_NetSocket_t* Socket;						// Actual net socket
	I_HostAddress_t BoundTo;					// Address bound to
	D_BS_t* StdBS;								// Standard Block Stream
	D_BS_t* RelBS;								// Reliable Block Stream
	
	bool_t Master;								// Master (Is the server)
	
	union
	{
		struct
		{
		} Master;								// Master Data
		
		struct
		{
			bool_t Synced;						// Synced to remote side
		} Slave;								// Slave Data
	} Data;										// Specific Data
} D_XDesc_t;

/* D_XEndPoint_t -- Endpoint connection */
typedef struct D_XEndPoint_s
{
	D_XDesc_t* Desc;							// Descriptor being used
	I_HostAddress_t Addr;						// Address allocated
	tic_t LastSeen;								// Last seen at
} D_XEndPoint_t;

/**************
*** GLOBALS ***
**************/

extern D_XDesc_t* g_XSocket;					// Master Socket

/****************
*** FUNCTIONS ***
****************/

/*** D_XPRO.C ***/

void D_XPDropXPlay(D_XPlayer_t* const a_XPlay, const char* const a_Reason);

void D_XPRunConnection(void);

void D_XPRunCS(D_XDesc_t* const a_Desc);

/*** D_XBIND.C ***/

bool_t D_XBHasConnection(void);

bool_t D_XBValidIP(I_HostAddress_t* const a_Addr);
bool_t D_XBWaitForCall(I_HostAddress_t* const a_BindTo);
bool_t D_XBCallHost(I_HostAddress_t* const a_ToCall, const uint32_t a_GameID);

void D_XBSocketDestroy(void);

void D_XBDropHost(I_HostAddress_t* const a_Addr);

D_XDesc_t* D_XBPathToXPlay(D_XPlayer_t* const a_XPlay, I_HostAddress_t** const a_HostPP, D_BS_t** const a_StdBSPP, D_BS_t** const a_RelBSPP);

#endif /* __D_XPRO_H__ */

