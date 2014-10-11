// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------

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
