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
#include "r_defs.h"
#include "r_state.h"
#include "m_bbox.h"
#include "p_info.h"

/*************
*** LOCALS ***
*************/

/****************
*** FUNCTIONS ***
****************/

/* SN_ClearLevel() -- Clear the level */
void SN_ClearLevel(void)
{
}

/* SN_InitLevel() -- Initializes the level for rendering */
bool_t SN_InitLevel(void)
{
	SN_Poly_t* BigPoly;
	fixed_t Box[4];
	node_t* LastNode;
	int i;
	
	/* Debug */
	if (M_CheckParm("-devsnow"))
		g_SnowBug = true;
	
	/* First clear the level */
	SN_ClearLevel();
	
	/* Success? */
	return true;
}

