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

/****************
*** CONSTANTS ***
****************/

/* D_XSyncLevel_t -- Synchronization level */
typedef enum D_XSyncLevel_e
{
	DXSL_INIT,									// Initialize
	DXSL_LISTWADS,								// List WADs being used
	DXSL_CHECKWADS,								// Checks WADs being used
	DXSL_DOWNLOADWADS,							// Downloads WAD files
	DXSL_SWITCHWADS,							// Switch to WADs
	DXSL_WAITFORWINDOW,							// Wait for join window
	DXSL_GETSAVE,								// Get savegame
	DXSL_LOADSAVE,								// Load savegame
} D_XSyncLevel_t;

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
			bool_t AntiConnect;					// Performed an anti-connect
		} Master;								// Master Data
		
		struct
		{
			bool_t Synced;						// Synced to remote side
			tic_t LastSyncReq;					// Last Sync Request
		} Slave;								// Slave Data
	} Data;										// Specific Data
	
	struct
	{
		D_XSyncLevel_t SyncLevel;				// Synchronization level
		bool_t SentReqWAD;						// Sent requested WADs
		bool_t JoinWait;						// Join waiting
	} Client;									// Client Stuff
} D_XDesc_t;

/* D_XEndPoint_t -- Endpoint connection */
typedef struct D_XEndPoint_s
{
	D_XDesc_t* Desc;							// Descriptor being used
	I_HostAddress_t Addr;						// Address allocated
	tic_t LastSeen;								// Last seen at
	
	uint32_t HostID;							// Host ID
	uint32_t ProcessID;							// Standard Process ID
	D_XSyncLevel_t SyncLevel;					// Synchronization level
	
	struct
	{
		bool_t Active;							// Active Split
		uint32_t ProcessID;						// Initial Process ID
		char DispName[MAXPLAYERNAME];			// Display Name
		char ProfUUID[MAXUUIDLENGTH + 1];		// Profile UUID
	} Splits[MAXSPLITSCREEN];					// Splitscreens
	
	bool_t SignalReady;							// Signaled ready to play
} D_XEndPoint_t;

/**************
*** GLOBALS ***
**************/

extern D_XDesc_t* g_XSocket;					// Master Socket

extern D_XEndPoint_t** g_XEP;					// End points
extern size_t g_NumXEP;							// Number of them

/****************
*** FUNCTIONS ***
****************/

/*** D_XPRO.C ***/

void D_XPCleanup(void);

void D_XPDropXPlay(D_XPlayer_t* const a_XPlay, const char* const a_Reason);
void D_XPRunConnection(void);
void D_XPSendDisconnect(D_XDesc_t* const a_Desc, D_BS_t* const a_BS, I_HostAddress_t* const a_Addr, const char* const a_Reason);

void D_XPRunCS(D_XDesc_t* const a_Desc);

/*** D_XBIND.C ***/

bool_t D_XBHasConnection(void);

bool_t D_XBValidIP(I_HostAddress_t* const a_Addr);
bool_t D_XBWaitForCall(I_HostAddress_t* const a_BindTo);
bool_t D_XBCallHost(I_HostAddress_t* const a_ToCall, const uint32_t a_GameID);

void D_XBSocketDestroy(void);

void D_XBDropHost(I_HostAddress_t* const a_Addr);

D_BS_t* D_XBRouteToServer(D_BS_t** const a_StdBSP, I_HostAddress_t** const a_AddrP);

D_XEndPoint_t* D_XBNewEndPoint(D_XDesc_t* const a_Desc, I_HostAddress_t* const a_Addr);
void D_XBDelEndPoint(D_XEndPoint_t* const a_XEP, const char* const a_Reason);
D_XEndPoint_t* D_XBEndPointForAddr(I_HostAddress_t* const a_Addr);
D_XEndPoint_t* D_XBEndPointForID(const uint32_t a_ID);

#endif /* __D_XPRO_H__ */

