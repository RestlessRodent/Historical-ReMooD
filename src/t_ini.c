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
// DESCRIPTION: INI-like parsing

/***************
*** INCLUDES ***
***************/

#include "t_ini.h"

/*****************
*** STRUCTURES ***
*****************/

/****************
*** FUNCTIONS ***
****************/

/* TINI_FindNextSection() -- Finds next section */
TINI_Section_t* TINI_FindNextSection(TINI_Section_t* const a_Last, WL_ES_t* const a_Stream)
{
#define BUFSIZE 128
	char Buf[BUFSIZE], *p, *Ep;
	uint32_t FirstPos, LastPos;
	TINI_Section_t* Active, *New;
	
	/* Check */
	if (!a_Stream)
		return NULL;
	
	/* End? */
	if (WL_StreamEOF(a_Stream))
		return NULL;
	
	/* Resume? */
	Active = a_Last;
	New = NULL;
	
	/* Constantly read lines */
	while (!WL_StreamEOF(a_Stream))
	{
		// Current position
		FirstPos = WL_StreamTell(a_Stream);
		
		// Read line
		memset(Buf, 0, sizeof(Buf));
		WL_Srl(a_Stream, Buf, BUFSIZE - 1);
		
		// Current position
		LastPos = WL_StreamTell(a_Stream);
		
		// Determine text
		p = Buf;
		
		// Skip whitespace at start
		if (*p == ' ' || *p == '\t')
			p++;
		
		// Is a [?
		if (*p == '[')
		{
			// Find ending ], if it exists
			Ep = strchr(p, ']');
			
			// Move pointers around
			p++;
			
			// Create new section, from between brackets
			if (Ep && Ep - p > 0)
			{
				// Plop left?
				if (New)
				{
					New->Next = Z_Malloc(sizeof(*New->Next), PU_STATIC, NULL);
					New->Next->Prev = New;
					New = New->Next;
				}
				
				else
					Active = New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
				
				// Copy block name
				strncpy(New->Name, p, (Ep - p > MAXSECTIONAME ? MAXSECTIONAME : Ep - p));
				
				// Locations
					// Start position is after ]
				New->Start = LastPos;
				
					// Last end position is before [
				if (New->Prev)
					New->Prev->End = FirstPos;
			}
		}
	}
	
	/* Last position? */
	if (New)
		New->End = WL_StreamTell(a_Stream);
	
	/* Return active block */
	return Active;
#undef BUFSIZE
}

/* TINI_ClearSections() -- Deletes sections detected */
void TINI_ClearSections(TINI_Section_t* const a_Iter)
{
}

/* TINI_BeginRead() -- Begins reading INI Section */
TINI_ConfigLine_t* TINI_BeginRead(TINI_Section_t* const a_Section)
{
	return NULL;
}

/* TINI_EndRead() -- Stops reading INI section */
void TINI_EndRead(TINI_ConfigLine_t* const a_Parms)
{
}

/* TINI_ReadLine() -- Reads single INI Line */
bool_t TINI_ReadLine(TINI_ConfigLine_t* a_Parms, const char** a_Option, const char** a_Value)
{
	return false;
}

