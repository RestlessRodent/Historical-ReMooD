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
// DESCRIPTION: Virtual Machine
/***************
*** INCLUDES ***
***************/

#include "t_vm.h"
#include "dstrings.h"
#include "console.h"

/****************
*** CONSTANTS ***
****************/

#define TVMPP_COMMENT		UINT16_C(0x0001)	// In Comment
#define TVMPP_ESCAPE		UINT16_C(0x0002)	// Escaped Sequence
#define TVMPP_QUOTE			UINT16_C(0x0004)	// Quoted String

/*****************
*** STRUCTURES ***
*****************/

/* TVM_State_t -- State */
typedef struct TVM_State_s
{
	uint8_t Junk;								// Cannot have empty struct
} TVM_State_t;

/*************
*** LOCALS ***
*************/

static TVM_State_t l_MapState;					// Current Map State

/****************
*** FUNCTIONS ***
****************/

/* TVMS_IsSpace() -- Returns true if character is space */
static bool_t TVMS_IsSpace(const char a_Char)
{
	switch (a_Char)
	{
		case ' ':
		case '\r':
		case '\t':
			return true;
		
		default:
			return false;
	}
}

/* TVMS_CompileLine() -- Compiles a line (tokenizes it) */
static bool_t TVMS_CompileLine(TVM_State_t* const a_State, char* const a_Buf, const size_t a_Len)
{
	CONL_PrintF("%i %s\n", a_Len, a_Buf);
	return true;
}

/* TVM_Clear() -- Clears the VM */
void TVM_Clear(void)
{
}

/* TVMS_CompileWLESInt() -- Compiles stream segment */
// This is based off my C99 preprocessor, at least the tokenization part.
// However compared to C, this is alot smaller and alot faster too.
// It already is fast, but since Legacy Script lacks so many things there's no
// code to execute.
// So compared to C, there is no...
//  * Trigraphs
//  * Multi-line Comments
//  * Escaped Newlines
static void TVMS_CompileWLESInt(TVM_State_t* const a_State, WL_ES_t* const a_Stream, const uint32_t a_End)
{
#define EATCHAR {ReadLeft--; memmove(&ReadBuf[0], &ReadBuf[1], sizeof(*ReadBuf) * (READSIZE - 1));}
#define READSIZE 32
#define TOKENSIZE 256
	char ReadBuf[READSIZE];
	char Token[TOKENSIZE];
	uint32_t ReadLeft, TokAt;
	uint16_t Mode;
	uint32_t LineNum;
	
	/* Check */
	if (!a_Stream || !a_End)
		return;
	
	/* Message */
	CONL_OutputUT(CT_SCRIPTING, DSTR_TVMC_COMPILING, "%s\n", WL_StreamGetEntry(a_Stream)->Name);
	
	/* Initialize */
	ReadLeft = Mode = TokAt = 0;
	memset(ReadBuf, 0, sizeof(ReadBuf));
	memset(Token, 0, sizeof(Token));
	LineNum = 1;
	
	/* Tokenization Loop */
	while ((!WL_StreamEOF(a_Stream) && WL_StreamTell(a_Stream) < a_End) || ReadLeft > 0)
	{
		// Read Into Buffer
		while (ReadLeft < READSIZE - 1)
			if (!WL_StreamEOF(a_Stream) && WL_StreamTell(a_Stream) < a_End)
				ReadBuf[ReadLeft++] = WL_Src(a_Stream);
			else
			{
				ReadBuf[ReadLeft] = 0;
				break;
			}
		
		// Nothing read?
		if (ReadLeft == 0)
			break;
		
		// If not in quotes, transform whitespace to space
		if ((Mode & TVMPP_QUOTE) == 0)
			if (TVMS_IsSpace(ReadBuf[0]))
				ReadBuf[0] = ' ';
		
		// If not in quotes and multiple spaces...
		if ((Mode & TVMPP_QUOTE) == 0)
			while (ReadBuf[0] == ' ' && ReadBuf[1] == ' ')
				EATCHAR;
				
		// Detect Comment
		if ((Mode & TVMPP_QUOTE) == 0)
			if (ReadBuf[0] == '/' && ReadBuf[1] == '/')
			{
				// Set as comment mode
				Mode |= TVMPP_COMMENT;
				
				// Eat two characters and move on
				EATCHAR;
				EATCHAR;
				continue;
			}
		
		// Detect Quotes
		if ((Mode & TVMPP_COMMENT) == 0)
			if ((Mode & TVMPP_ESCAPE) == 0)
				if (ReadBuf[0] == '\"')
					Mode ^= TVMPP_QUOTE;
		
		// Handle Newline
		if (ReadBuf[0] == '\n')
		{
			// Eat newline
			EATCHAR;
			
			// Increase Line Number
			LineNum++;
			
			// Clear comment and escapedness, if any
			Mode &= ~(TVMPP_COMMENT | TVMPP_ESCAPE);
			
			// Send to tokenizer
			if (Token[0])
				if (!TVMS_CompileLine(a_State, Token, TokAt))
				{
					CONL_OutputUT(CT_SCRIPTING, DSTR_TVMC_ERROR, "%u\n", LineNum);
					return;
				}
			
			// Clear Token
			memset(Token, 0, sizeof(Token));
			TokAt = 0;
			
			// Continue on
			continue;
		}
		
		// Escape sequence
		Mode &= ~TVMPP_ESCAPE;
		if (ReadBuf[0] == '\\')
			Mode |= TVMPP_ESCAPE;
				
		// Single-line comments, ignore the remaining
		if (Mode & TVMPP_COMMENT)
		{
			EATCHAR;
			continue;
		}
		
		// Otherwise append to token buffer
		if (TokAt < TOKENSIZE - 1)
			Token[TokAt++] = ReadBuf[0];
		
		// Eat character
		EATCHAR;
	}
#undef READSIZE
#undef EATCHAR
}

/* TVM_CompileWLES() -- Wraps internal */
void TVM_CompileWLES(WL_ES_t* const a_Stream, const uint32_t a_End)
{
	TVMS_CompileWLESInt(&l_MapState, a_Stream, a_End);
}

/* TVM_CompileEntry() -- Compiles entry */
void TVM_CompileEntry(const WL_WADEntry_t* const a_Entry)
{
	WL_ES_t* ScriptStream;
	
	/* Open stream */
	ScriptStream = WL_StreamOpen(a_Entry);
	
	/* If it was created */
	if (ScriptStream)
	{
		// Check Unicode viability
		WL_StreamCheckUnicode(ScriptStream);
		
		// Compile it
		TVM_CompileWLES(ScriptStream, a_Entry->Size);
		
		// Close it
		WL_StreamClose(ScriptStream);
	}
}

