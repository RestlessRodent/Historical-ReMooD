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
// DESCRIPTION: Bot Code

#ifndef __BOT_H__
#define __BOT_H__

/*****************************************************************************/

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"

/*****************
*** STRUCTURES ***
*****************/

/* Define SN_Port_t */
#if !defined(__REMOOD_SNPORT_DEFINED)
	typedef struct SN_Port_s SN_Port_t;
	#define __REMOOD_SNPORT_DEFINED
#endif

/****************
*** FUNCTIONS ***
****************/

void BOT_Init(void);
void BOT_Ticker(void);
void BOT_Add(const int32_t a_ArgC, const char** const a_ArgV);
void BOT_RegisterLevel(void);

void BOT_DestroyByPort(SN_Port_t* const a_Port);
void BOT_LeaveStasis(SN_Port_t* const a_Port);
void BOT_EnterStatis(SN_Port_t* const a_Port);

/*****************************************************************************/

#endif /* __BOT_H__ */

