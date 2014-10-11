// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Global RMOD Parsing

#ifndef __D_RMOD_H__
#define __D_RMOD_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"






/****************
*** CONSTANTS ***
****************/

/* D_RMODCommand_t -- REMOODAT Command */
typedef enum D_RMODCommand_e
{
	DRC_OPEN,									// Opening {
	DRC_CLOSE,									// Closing }
	DRC_DATA,									// Data Entry
	DRC_INIT,									// Initialize
	DRC_FINAL,									// Finalize
	
	DRC_FIRST,									// First Time
	DRC_LAST,									// Last Time
	
	NUMDRMODCOMMANDS
} D_RMODCommand_t;

/*****************
*** STRUCTURES ***
*****************/

typedef bool_t (*D_RMODKeyerFuncType_t)(void** a_DataPtr, const int32_t a_Stack, const D_RMODCommand_t a_Command, const char* const a_Field, const char* const a_Value);

/*****************
*** PROTOTYPES ***
*****************/

void D_InitRMOD(void);

#endif							/* __D_RMOD_H__ */

