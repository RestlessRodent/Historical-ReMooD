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
// Copyright (C) 2012-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Binary FraggleScript (LegacyScript)

#ifndef __T_BFS_H__
#define __T_BFS_H__

/*****************************************************************************/

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"

/****************
*** CONSTANTS ***
****************/

/* TBFS_Vis_t() -- Script visibility */
typedef enum TBFS_Vis_e
{
	TBFSV_LEVEL,								// Only to this level
	TBFSV_HUB,									// Only to this hub
	TBFSV_GAME,									// Only to this game session
} TBFS_Vis_t;

/*****************
*** STRUCTURES ***
*****************/

/* Define WL_ES_t */
#if !defined(__REMOOD_WLEST_DEFINED)
	typedef struct WL_ES_s WL_ES_t;
	#define __REMOOD_WLEST_DEFINED
#endif

/* Define WL_WADEntry_t */
#if !defined(__REMOOD_WLWADENT_DEFINED)
	typedef struct WL_WADEntry_s WL_WADEntry_t;
	#define __REMOOD_WLWADENT_DEFINED
#endif

/****************
*** FUNCTIONS ***
****************/

void TBFS_ClearScripts(const TBFS_Vis_t a_Vis);
bool_t TBFS_LoadScript(const TBFS_Vis_t a_Vis, WL_ES_t* const a_WL, const uint32_t a_Start, const uint32_t a_End);
bool_t TBFS_LoadEntry(const TBFS_Vis_t a_Vis, const WL_WADEntry_t* const a_Ent);

/*****************************************************************************/

#endif /* __T_BFS_H__ */

