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
// DESCRIPTION: Simplistic Line Handling Code

#ifndef __P_NEWLINE_H__
#define __P_NEWLINE_H__

/***************
*** INCLUDES ***
***************/

//#include "doomtype.h"
//#include "p_local.h"
//#include "p_spec.h"

/****************
*** CONSTANTS ***
****************/

/* P_NLExtraProp_t -- Extra properties of lines (flags) */
typedef enum P_NLExtraProp_s
{
	PNLXP_DOORLIGHT		= UINT32_C(0x000001),	// Has manual door light
} P_NLExtraProp_t;

/**************
*** STRUCTS ***
**************/

typedef bool_t (*P_NLTrigFunc_t)(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV);

/****************
*** FUNCTIONS ***
****************/

bool_t P_NLTrigger(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain);
void P_NLCreateStartLines(void);

uint32_t P_NLSpecialXProp(line_t* const a_Line);

int32_t P_NLDefDoorCloseSnd(void);

#endif /* __P_NEWLINE_H__ */

