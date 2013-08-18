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
// DESCRIPTION: Private Bot Code Header

#ifndef __BOT_PRIV__H__
#define __BOT_PRIV__H__

/*****************************************************************************/

/***************
*** INCLUDES ***
***************/

#include "bot.h"
#include "mips.h"
#include "w_wad.h"

/*****************
*** STRUCTURES ***
*****************/

/* BOT_t -- Bot */
typedef struct BOT_s
{
	SN_Port_t* Port;							// Port which bot controls
	uint32_t ProcessID;							// Process ID of bot
	tic_t LastXMit;								// Last port transmit
	bool_t Statis;								// In statis mode
	bool_t BasicInit;							// Basic Initialization
	tic_t LastGameTryJoin;						// Time last tried to join game
	
	/* MIPS Stuff */
	const WL_WADEntry_t* CodeEnt;				// Code Entry
	void* CodeMap;								// Code mapping
	uint32_t CodeLen;							// Length of binary code
	MIPS_VM_t VM;								// MIPS VM
} BOT_t;

/****************
*** FUNCTIONS ***
****************/

BOT_t* BOT_ByProcessID(const uint32_t a_ProcessID);
BOT_t* BOT_ByPort(SN_Port_t* const a_Port);

/*****************************************************************************/

#endif /* __BOT_PRIV_H__ */

