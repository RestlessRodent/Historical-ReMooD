// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: INI-like parsing

#ifndef __T_INI_H__
#define __T_INI_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "w_wad.h"

/****************
*** CONSTANTS ***
****************/

#define MAXSECTIONAME						48	// Size limitation of section names

/*****************
*** STRUCTURES ***
*****************/

/* TINI_Section_t -- INI Section */
typedef struct TINI_Section_s
{
	WL_ES_t* Stream;							// Associated Stream
	
	char Name[MAXSECTIONAME];					// Name of INI Section
	
	uint32_t Start;								// Start of data
	uint32_t End;								// End of data
	
	struct TINI_Section_s* Prev;				// Previous Link
	struct TINI_Section_s* Next;				// Next Link
} TINI_Section_t;

typedef struct TINI_ConfigLine_s TINI_ConfigLine_t;

/****************
*** FUNCTIONS ***
****************/

TINI_Section_t* TINI_FindNextSection(TINI_Section_t* const a_Last, WL_ES_t* const a_Stream);
void TINI_ClearSections(TINI_Section_t* const a_Iter);

TINI_ConfigLine_t* TINI_BeginRead(TINI_Section_t* const a_Section);
void TINI_EndRead(TINI_ConfigLine_t* const a_Parms);
bool_t TINI_ReadLine(TINI_ConfigLine_t* const a_Parms, char** a_Option, char** a_Value);

#endif /* __T_INI_H__ */

