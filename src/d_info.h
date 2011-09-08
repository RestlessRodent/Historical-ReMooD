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
// Copyright (C) 2011 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: *INFO processing

#ifndef __D_INFO_H__
#define __D_INFO_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "w_wad.h"

/****************
*** CONSTANTS ***
****************/

/* D_InfoLoadType_t -- Type of loading to do */
typedef enum D_InfoLoadType_e
{
	DILT_BUILDONE,
	DILT_CLEARONE,
	DILT_BUILDALL,
	DILT_CLEARALL,
} D_InfoLoadType_t;

/*****************
*** STRUCTURES ***
*****************/

/*****************
*** PROTOTYPES ***
*****************/

void D_WXBuildInfos(const D_InfoLoadType_t a_Type, WX_WADFile_t* const a_WAD);

#endif /* __D_INFO_H__ */

