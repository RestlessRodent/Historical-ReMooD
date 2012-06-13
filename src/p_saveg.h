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
// DESCRIPTION: Savegame I/O, archiving, persistence.

#ifndef __P_SAVEG__
#define __P_SAVEG__

/***************
*** INCLUDES ***
***************/

#include "d_block.h"

/****************
*** FUNCTIONS ***
****************/

void P_InitSGConsole(void);

// Persistent storage/archiving.
// These are the load / save game routines.

void P_SaveGame(void);
bool_t P_LoadGame(void);

bool_t P_LoadGameEx(const char* FileName, const char* ExtFileName, size_t ExtFileNameLen, size_t* SaveLen, uint8_t** Origin);
bool_t P_SaveGameEx(const char* Desc, const char* ExtFileName, size_t ExtFileNameLen, size_t* SaveLen, uint8_t** Origin);

bool_t P_LoadGameFromBS(D_RBlockStream_t* const a_Stream, I_HostAddress_t* const a_NetAddr);
bool_t P_SaveGameToBS(D_RBlockStream_t* const a_Stream, I_HostAddress_t* const a_NetAddr);

bool_t P_SGDXSpec(D_RBlockStream_t* const a_Stream, I_HostAddress_t* const a_NetAddr, bool_t a_Load);

extern uint8_t* save_p;

#endif

