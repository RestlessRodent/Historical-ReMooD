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
// DESCRIPTION: C Compiling Stubs

#if defined(__REMOOD_USECCSTUB)

/*****************************************************************************/

/***************
*** INCLUDES ***
***************/



#if defined(__palmos__)
	#include <PalmOS.h>
	#include <StringMgr.h>
	#include <VFSMgr.h>
	#include <PceNativeCall.h>
#endif

/**************
*** GLOBALS ***
**************/

const void *gEmulStateP = 0;
Call68KFuncType *gCall68KFuncP = 0;

/****************
*** FUNCTIONS ***
****************/

/* mkdir() -- Makes directory */
int mkdir(const char* const a_PathName)
{
	// NOT IMPLEMENTED
	return 0;
}

/* socket() -- Opens Socket */
int socket(int domain, int type, int protocol)
{
#if defined(__palmos__)
	/* Palm OS */
	return -1;
	
#else
	/* NOT IMPLEMENTED */
	return -1;
#endif
}

/*****************************************************************************/

#endif /* __REMOOD_USECCSTUB */

