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
// Copyright (C) 2011-2012 GhostlyDeath <ghostlydeath@remood.org>
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

/****************
*** CONSTANTS ***
****************/

/* D_RMODPrivates_t -- RMOD Private info */
typedef enum D_RMODPrivates_e
{
	DRMODP_WIDGET,								// Widgets
	DRMODP_MENU,								// Menu related stuff
	
	/* Specials (Lines/Sectors) */
	DRMODP_SPECSECTOR,							// "MapSectorSpecial"
	DRMODP_SPECLINE,							// "MapLineSpecial"
	
	NUMDRMODPRIVATES
} D_RMODPrivates_t;

/*****************
*** STRUCTURES ***
*****************/

/* D_RMODPrivate_t -- RMOD Private Stuff */
typedef struct D_RMODPrivate_s
{
	void* Data;									// Data
	size_t Size;								// Size
} D_RMODPrivate_t;

typedef bool_t (*D_RMODHandleFunc_t)(Z_Table_t* const a_Table, const WL_WADFile_t* const a_WAD, const D_RMODPrivates_t a_ID, D_RMODPrivate_t* const a_Private);
typedef bool_t (*D_RMODOCCBFunc_t)(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD, const D_RMODPrivates_t a_ID);

/*****************
*** PROTOTYPES ***
*****************/

void D_InitRMOD(void);
D_RMODPrivate_t* D_GetRMODPrivate(const WL_WADFile_t* const a_WAD, const D_RMODPrivates_t a_ID);

bool_t D_RMODGetBool(const char* const a_Str);

#endif							/* __D_RMOD_H__ */

