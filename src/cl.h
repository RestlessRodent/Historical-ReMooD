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
// Copyright (C) 2013-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Local Game Client

#ifndef __CL_H__
#define __CL_H__

/*****************************************************************************/

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"

/****************
*** CONSTANTS ***
****************/

/* CL_SockFlags_t -- Socket Flags */
typedef enum CL_SockFlags_e
{
	CLSF_JOYSTICK		= UINT32_C(0x00000001),	// Using joystick
} CL_SockFlags_t;

/*****************
*** STRUCTURES ***
*****************/

/* Define SN_Port_t */
#if !defined(__REMOOD_SNPORT_DEFINED)
	typedef struct SN_Port_s SN_Port_t;
	#define __REMOOD_SNPORT_DEFINED
#endif

/* Define CL_View_t */
#if !defined(__REMOOD_CLVIEW_DEFINED)
	typedef struct CL_View_s CL_View_t;
	#define __REMOOD_CLVIEW_DEFINED
#endif

/* Define CL_Socket_t */
#if !defined(__REMOOD_CLSOCKET_DEFINED)
	typedef struct CL_Socket_s CL_Socket_t;
	#define __REMOOD_CLSOCKET_DEFINED
#endif

/* CL_View_t -- Client viewport */
struct CL_View_s
{
	SN_Port_t* Port;							// Port to control
};

/* CL_Socket_t -- Socket which recieves controller input */
struct CL_Socket_s
{
	uint32_t Flags;								// Flags
	int8_t JoyID;								// JoyStick ID
};

/**************
*** GLOBALS ***
**************/

extern CL_View_t g_CLViews[MAXSPLITS];		// Viewports

/****************
*** FUNCTIONS ***
****************/

int32_t CL_InitSocks(void);

/*****************************************************************************/

#endif /* __CL_H__ */


