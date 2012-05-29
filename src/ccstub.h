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
// Copyright (C) 2012 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: C Compiling Stubs

#ifndef __CCSTUB_H__
#define __CCSTUB_H__

#if defined(__REMOOD_USECCSTUB)

/*****************************************************************************/

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"

/* Palm OS Hacks */
#if defined(__REMOOD_SYSTEM_PALMOS)
	#include <VFSMgr.h>							// File Handling
#endif

/*******************
*** DEFINE HACKS ***
*******************/

/* Palm OS Hacks */
#if defined(__REMOOD_SYSTEM_PALMOS)
	
	// Map VFS File to C one
	typedef FileRef FILE;
	
	// Limitations
	#ifndef INT_MAX
		#define INT_MAX __INT_MAX__
	#endif
#endif

/*****************************************************************************/

#endif /* __REMOOD_USECCSTUB */

#endif /* __CCSTUB_H__ */

