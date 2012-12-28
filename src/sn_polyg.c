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
// DESCRIPTION: Level Trianglulation

/***************
*** INCLUDES ***
***************/

#include "sn_polyg.h"
#include "p_info.h"
#include "p_demcmp.h"
#include "m_bbox.h"
#include "r_local.h"
#include "sn_main.h"
#include "vhw_wrap.h"
#include "b_bot.h"

/*****************
*** STRUCTURES ***
*****************/

#define __REMOOD_DOUBLEPOLY

#if defined(__REMOOD_DOUBLEPOLY)
	typedef double polyf_t;
	
#else
	typedef fixed_t polyf_t;

#endif

/**************
*** GLOBALS ***
**************/

/*************
*** LOCALS ***
*************/

/****************
*** FUNCTIONS ***
****************/

/* SN_PolygonizeLevel() -- Polygonizes the level */
void SN_PolygonizeLevel(void)
{
}


