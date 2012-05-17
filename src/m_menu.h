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
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION: Menu widget stuff, episode selection and such.

#ifndef __M_MENU__
#define __M_MENU__

#include "d_event.h"
#include "command.h"
#include "d_prof.h"

#include "w_wad.h"
#include "z_zone.h"

#include "d_rmod.h"

#include "i_util.h"

/*******************************************************************************
********************************************************************************
*******************************************************************************/

/*** CONSTANTS ***/

/* M_ExMBType_t -- Message Box Type */
typedef enum M_ExMBType_e
{
	MEXMBT_OK						= 0x0001,	// OK
	MEXMBT_YES						= 0x0002,	// Yes
	MEXMBT_NO						= 0x0004,	// No
	MEXMBT_CANCEL					= 0x0008,	// Cancel
	MEXMBT_DONTCARE					= 0x0010,	// Don't Care
} M_ExMBType_t;

/*** STRUCTURES ***/

typedef void (*MBCallBackFunc_t)(const uint32_t a_MessageID, const M_ExMBType_t a_Response, const char** const a_TitleP, const char** const a_MessageP);

/*** FUNCTIONS ***/

bool_t M_ExUIActive(void);

bool_t M_ExUIMessageBox(const M_ExMBType_t a_Type, const uint32_t a_MessageID, const char* const a_Title, const char* const a_Message, const MBCallBackFunc_t a_CallBack);

bool_t M_ExUIHandleEvent(const I_EventEx_t* const a_Event);
void M_ExUIDrawer(void);

#endif

