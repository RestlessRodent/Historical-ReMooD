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
// Copyright (C) 2011-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Global RMOD Parsing

#ifndef __D_RMOD_H__
#define __D_RMOD_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "dstrings.h"
#include "w_wad.h"
#include "z_zone.h"
#include "m_fixed.h"
#include "d_block.h"

/****************
*** CONSTANTS ***
****************/

/* D_RMODCommand_t -- REMOODAT Command */
typedef enum D_RMODCommand_e
{
	DRC_OPEN,									// Opening {
	DRC_CLOSE,									// Closing }
	DRC_DATA,									// Data Entry
	DRC_INIT,									// Initialize
	DRC_FINAL,									// Finalize
	
	DRC_FIRST,									// First Time
	DRC_LAST,									// Last Time
	
	NUMDRMODCOMMANDS
} D_RMODCommand_t;

/*****************
*** STRUCTURES ***
*****************/

typedef bool_t (*D_RMODKeyerFuncType_t)(void** a_DataPtr, const int32_t a_Stack, const D_RMODCommand_t a_Command, const char* const a_Field, const char* const a_Value);

/*****************
*** PROTOTYPES ***
*****************/

void D_InitRMOD(void);

#endif							/* __D_RMOD_H__ */

