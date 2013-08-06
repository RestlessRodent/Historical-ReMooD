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
// Legacy Script compiler

#ifndef __T_COMP_H__
#define __T_COMP_H__

/***************
*** INCLUDES ***
***************/

//#include "doomtype.h"
//#include "doomdef.h"
//#include "w_wad.h"
//#include "z_zone.h"

/**********************
*** UPPER CONSTANTS ***
**********************/

#define TLS_MAXCODESIZE		65536	// Only 65536 bytes for execution

/* TLS_VariableType_t -- Type of variable available */
typedef enum TLS_VariableType_e
{
	TLSVT_NULL,					// Nothing
	TLSVT_CONST,				// Constant
	TLSVT_INT,					// int
	TLSVT_FIXED,				// Fixed point
	TLSVT_MOBJ,					// Map object
	TLSVT_STRING,				// String
	
	NUMTLSVARIABLETYPES
} TLS_VariableType_t;

/*****************
*** STRUCTURES ***
*****************/

/**********************
*** LOWER CONSTANTS ***
**********************/

/**************
*** GLOBALS ***
**************/

/*****************
*** PROTOTYPES ***
*****************/

bool_t TLS_ClearScripts(void);
bool_t TLS_CompileLump(const WadIndex_t Index);

#endif							/* __T_COMP_H__ */
