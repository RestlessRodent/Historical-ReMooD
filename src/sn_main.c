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
// DESCRIPTION: ReMooD Snow Renderer

/***************
*** INCLUDES ***
***************/

#include "sn_main.h"
#include "p_info.h"

/**************
*** GLOBALS ***
**************/

bool_t g_SnowBug = false;						// Debug

/*************
*** LOCALS ***
*************/

static P_LevelInfoEx_t* l_SNLastLev = NULL;

/****************
*** FUNCTIONS ***
****************/

/* SN_ViewResize() -- Resizes the view (if needed that is) */
void SN_ViewResize(void)
{
}

/* SN_RenderView() -- Renders an individual player view */
void SN_RenderView(player_t* player, const size_t a_Screen)
{
	/* Needs re-init? */
	if (g_CurrentLevelInfo != l_SNLastLev)
	{
		// Try Initializing Level
		if (!SN_InitLevel())
		{
			l_SNLastLev = NULL;
			return;
		}
		
		// Do not init if level does not change!
		l_SNLastLev = g_CurrentLevelInfo;
	}
}

