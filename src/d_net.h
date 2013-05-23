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

/****************
*** CONSTANTS ***
****************/

#if !defined(MAXUUIDLENGTH)
	#define MAXUUIDLENGTH	(MAXPLAYERNAME * 2)	// Length of UUIDs
#endif

/*****************
*** STRUCTURES ***
*****************/

struct D_BS_s;
struct D_ProfileEx_s;
struct B_BotTemplate_s;

/* D_SNPort_t -- Port which controls a specific player or a spectator */
typedef struct D_SNPort_s
{
	player_t* Player;							// Player
	
} D_SNPort_t;

/* D_SNHost_t -- Host which controls a set of playing players */
typedef struct D_SNHost_s
{
	D_SNPort_t** Ports;							// Ports
	int32_t NumPorts;							// Number of ports
} D_SNHost_t;

/*****************
*** PROTOTYPES ***
*****************/

bool_t D_NetSetPlayerName(const int32_t a_PlayerID, const char* const a_Name);
bool_t D_NetPlayerChangedPause(const int32_t a_PlayerID);



#endif							/* __D_NET_H__ */

