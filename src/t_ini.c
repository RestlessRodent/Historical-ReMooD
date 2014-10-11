// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: INI-like parsing

/***************
*** INCLUDES ***
***************/

#include "t_ini.h"
#include "z_zone.h"
#include "w_wad.h"

/****************
*** CONSTANTS ***
****************/

#define MAXBUFSIZE							512	// Maximum Buffer Size

/*****************
*** STRUCTURES ***
*****************/

/* TINI_ConfigLine_s -- Configuration Line */
struct TINI_ConfigLine_s
{
	TINI_Section_t* Section;					// Mapped Section
	char Buffer[MAXBUFSIZE];					// Input buffer
};

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
		while (*p == ' ' || *p == '\t')
			p++;
		
		// Is a [? Begins a block
		if (*p == '[')
		{
			// Find ending ], if it exists
			Ep = strchr(p, ']');
			
			// Move pointers around
			p++;
			
			// Skip any whitespace
			while (*p == ' ' || *p == '\t')
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
				
				// Copy block name and stream
				strncpy(New->Name, p, (Ep - p > MAXSECTIONAME ? MAXSECTIONAME : Ep - p));
				New->Stream = a_Stream;
				
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
	TINI_Section_t* Rover, *Next;
	
	/* Check */
	if (!a_Iter)
		return;
		
	/* Free Left */
	for (Rover = a_Iter->Prev; Rover; Rover = Next)
	{
		Next = Rover->Prev;
		Z_Free(Rover);
	}
		
	/* Free Right */
	for (Rover = a_Iter->Next; Rover; Rover = Next)
	{
		Next = Rover->Next;
		Z_Free(Rover);
	}
	
	/* Free Iter */
	Z_Free(a_Iter);
}

/* TINI_BeginRead() -- Begins reading INI Section */
TINI_ConfigLine_t* TINI_BeginRead(TINI_Section_t* const a_Section)
{
	TINI_ConfigLine_t* New;
	
	/* Check */
	if (!a_Section)
		return NULL;
	
	/* Allocate */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	// Set
	New->Section = a_Section;
	
	/* Seek */
	WL_StreamSeek(New->Section->Stream, New->Section->Start, false);
	
	/* Return it */
	return New;
}

/* TINI_EndRead() -- Stops reading INI section */
void TINI_EndRead(TINI_ConfigLine_t* const a_Parms)
{
	/* Check */
	if (!a_Parms)
		return;
	
	/* Free */
	Z_Free(a_Parms);
}

/* TINI_ReadLine() -- Reads single INI Line */
bool_t TINI_ReadLine(TINI_ConfigLine_t* a_Parms, char** a_Option, char** a_Value)
{
	uint32_t Pos;
	char* Eq;
	
	/* Check */
	if (!a_Parms || !a_Option || !a_Value)
		return false;
	
	/* Reached end? */
	Pos = WL_StreamTell(a_Parms->Section->Stream);
	
	// Also check for end of stream
	if (WL_StreamEOF(a_Parms->Section->Stream) || Pos >= a_Parms->Section->End)
		return false;
	
	/* Read single line */
	memset(a_Parms->Buffer, 0, sizeof(a_Parms->Buffer));
	WL_Srl(a_Parms->Section->Stream, a_Parms->Buffer, MAXBUFSIZE);
	
	// Find equals sign
	Eq = strchr(a_Parms->Buffer, '=');
	
	// None there?
	if (!Eq)
		return false;
	
	/* Set pointers to locations */
	*a_Option = a_Parms->Buffer;
	*a_Value = Eq + 1;
	
	/* Format String */
	// Remove equal sign
	*Eq = 0;
	
	// Skip beginning white space
		// Option
	while (**a_Option == ' ' || **a_Option == '\t')
		(*a_Option)++;
		
		// Value
	while (**a_Value == ' ' || **a_Value == '\t')
		(*a_Value)++;
	
	// NULL?
	if (!**a_Option || !**a_Value)
		return false;
	
	// Skip whitespace at end
		// Option
	Eq = *a_Option + (strlen(*a_Option) - 1);
	while (Eq > *a_Option && (*Eq == ' ' || *Eq == '\t'))
		Eq--;
	*(Eq + 1) = 0;
	
		// Value
	Eq = *a_Value + (strlen(*a_Value) - 1);
	while (Eq > *a_Value && (*Eq == ' ' || *Eq == '\t'))
		Eq--;
	*(Eq + 1) = 0;
	
	// NULL?
	if (!**a_Option || !**a_Value)
		return false;
	
	/* Something was found */
	return true;
}

