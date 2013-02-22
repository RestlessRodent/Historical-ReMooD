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
	CONL_PrintF("Hosting\n");
	return false;
}

/* D_XBCallHost() -- Connects to another server */
bool_t D_XBCallHost(I_HostAddress_t* const a_ToCall, const uint32_t a_GameID)
{
#define BUFSIZE 128
	char Buf[BUFSIZE];
	I_NetHostToString(a_ToCall, Buf, BUFSIZE - 1);
	CONL_PrintF("Calling `%s` extention %u\n", Buf, a_GameID);
	return false;
#undef BUFSIZE
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

