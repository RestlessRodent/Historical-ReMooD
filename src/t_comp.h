// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.org/
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
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
// Legacy Script compiler

#ifndef __T_COMP_H__
#define __T_COMP_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "doomdef.h"
#include "w_wad.h"
#include "z_zone.h"

/**********************
*** UPPER CONSTANTS ***
**********************/

#define TLS_MAXCODESIZE		65536						// Only 65536 bytes for execution

/* TLS_VariableType_t -- Type of variable available */
typedef enum TLS_VariableType_e
{
	TLSVT_NULL,											// Nothing
	TLSVT_CONST,										// Constant
	TLSVT_INT,											// int
	TLSVT_FIXED,										// Fixed point
	TLSVT_MOBJ,											// Map object
	TLSVT_STRING,										// String
	
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

boolean TLS_ClearScripts(void);
boolean TLS_CompileLump(const WadIndex_t Index);

#endif /* __T_COMP_H__ */

