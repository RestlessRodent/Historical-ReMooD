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
// Copyright (C) 2011-2012 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: ReMooD Scripting Virtual Machine (Core Machine Execution)


/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "t_dsvm.h"
#include "doomstat.h"

/*************
*** LOCALS ***
*************/

static int32_t l_ScopeDepth = 0;				// Depth of scopes

static const WL_WADEntry_t** l_IncStack = NULL;	// Include Stack
static size_t l_NumIncStack = 0;				// Thing on the stack

/****************
*** FUNCTIONS ***
****************/

/* T_DSVM_ReadToken() -- Reads Token */
bool_t T_DSVM_ReadToken(WL_EntryStream_t* const a_Stream, const size_t a_End, size_t* const a_Row, char* const a_Buf, const size_t a_BufSize)
{
	bool_t ReadSomething;
	char Char;
	size_t o;
	int8_t Mode;
	bool_t ReadString, Ident, Number, Symbol;
	bool_t DidPeriod, EscapeQuote;
	
	/* Clear Buffer */
	memset(a_Buf, 0, sizeof(*a_Buf) * a_BufSize);
	ReadSomething = false;		// Read something at least
	o = 0;
	
	/* Read Loop */
	Mode = 0;
	ReadString = Ident = Number = Symbol = DidPeriod = EscapeQuote = false;
	while ((Char = WL_StreamReadChar(a_Stream)))
	{
		// EOS?
		if (!Char || WL_StreamEOF(a_Stream) || WL_StreamTell(a_Stream) >= a_End)
			return ReadSomething;
		
		// Ignore Whitespace
		if ((!ReadString && (Char == ' ' || Char == '\t')) || (Char == '\r' || Char == '\n'))
		{
			// If something was read, return it
			if (ReadSomething)
				return true;
			
			// Otherwise continue
			continue;
		}
		
		// Ignore Comments
		if (!ReadString && (Char == '/'))
		{
			// Read Next Char
			Char = WL_StreamReadChar(a_Stream);
			
			// End of read?
			if (!Char || WL_StreamEOF(a_Stream) || WL_StreamTell(a_Stream) >= a_End)
				return ReadSomething;
			
			// If it is a '/' it is a comment, otherwise it is division
			if (Char == '/')
			{
				// Continue until '\n'
				while (Char = WL_StreamReadChar(a_Stream))
				{
					// End of read?
					if (!Char || WL_StreamEOF(a_Stream) || WL_StreamTell(a_Stream) >= a_End)
						return ReadSomething;
					
					// New line?
					if (Char == '\n')
						break;
				}
				
				// Return if we read something
				if (ReadSomething)
					return ReadSomething;
				
				// Otherwise continue
				continue;	// A full comment line
			}
			
			// It isn't, so it is division
			else
			{
				// If it is not a symbol, go back one
				// This is for `foo+2/5-1` like cases
				if ((Ident || Number) && !Symbol)
				{
					WL_StreamSeek(a_Stream, WL_StreamTell(a_Stream) - 1, false);
					return ReadSomething;
				}
				
				// Otherwise it is a full blown free operator
				else
				{
					if (o < a_BufSize)
						a_Buf[o++] = Char;
					ReadSomething = true;
					Symbol = true;
					return true;
				}
			}
		}
		
		// Reading something, but not the same something?
		if (!ReadString &&
			((Ident && !((Char >= 'a' && Char <= 'z') || (Char >= 'A' && Char <= 'Z') || (Char >= '0' && Char <= '9') || (Char == '_'))) ||
			(Number && !((Char >= '0' && Char <= '9') || (Char == '.' && !DidPeriod))) ||
			(Symbol)))
		{
			// Backup and return what was read
			WL_StreamSeek(a_Stream, WL_StreamTell(a_Stream) - 1, false);
			return ReadSomething;
		}
		
		// Copy Character to Buffer
		if (o < a_BufSize)
			a_Buf[o++] = Char;
		ReadSomething = true;
		
		// Number period?
		if (Number && Char == '.')
			DidPeriod = true;
			
		// End of string?
		if (ReadString && !EscapeQuote && Char == '\"')
			return ReadSomething;
		
		// Check for string escape
		if (ReadString)
		{
			if (EscapeQuote)
				EscapeQuote = false;
			else if (Char == '\\')
				EscapeQuote = true;
		}
		
		// Not reading anything, set associated flags
		if (!Ident && !Number && !Symbol && !ReadString)
		{
			// Identifier
			if ((Char >= 'a' && Char <= 'z') || (Char >= 'A' && Char <= 'Z') || (Char == '_'))
				Ident = true;
			
			// Number
			else if (Char >= '0' && Char <= '9')
				Number = true;
			
			// String
			else if (Char == '\"')
				ReadString = true;
			
			// Otherwise it is a symbol
			else
				Symbol = true;
		}
		
		// Return symbols always
		if (Symbol)
			return ReadSomething;
	}
	
	/* No more tokens */
	return false;
}

/* T_DSVM_ScriptError() -- Error script */
void T_DSVM_ScriptError(const char* const a_Str, const uint32_t a_Line)
{
	/* Play sound and print message */
	S_StartSound(NULL, sfx_lotime);
	CONL_PrintF("Script Error (Line %u): %s\n", a_Line, a_Str);
}

/* T_DSVM_CompileStream() -- Compiles a stream */
bool_t T_DSVM_CompileStream(WL_EntryStream_t* const a_Stream, const size_t a_End)
{
#define BUFSIZE 512
	char Buf[BUFSIZE];
	char ExtraBuf[BUFSIZE];
	size_t Row;
	bool_t ScriptProblem, QuickRet;
	const WL_WADEntry_t* Entry;
	WL_EntryStream_t* IncStream;
	int32_t i, j, n;

	/* Check */
	if (!a_Stream)
		return false;
	
	/* Debug */
	if (devparm)
		CONL_PrintF("T_DSVM_CompileStream: Compiling...\n");
	
	/* Always push to the include stack */
	// This helps prevent `include("MAP01");`
	Z_ResizeArray((void**)&l_IncStack, sizeof(*l_IncStack), l_NumIncStack, l_NumIncStack + 1);
	l_IncStack[l_NumIncStack++] = WL_StreamGetEntry(a_Stream);
	
	/* Constantly Read Tokens */
	ScriptProblem = false;
	while (T_DSVM_ReadToken(a_Stream, a_End, &Row, Buf, BUFSIZE - 1))
	{
		// Debug
		if (devparm)
			CONL_PrintF("Tok: `%s`\n", Buf);
		
		// Handle Includes
		if (strcasecmp(Buf, "include") == 0)
		{
			// Out of scope?
			if (l_ScopeDepth > 0)
			{
				T_DSVM_ScriptError("Include inside function or script.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Read token, and expect a '('
			QuickRet = T_DSVM_ReadToken(a_Stream, a_End, &Row, Buf, BUFSIZE - 1);
			if (!QuickRet || (QuickRet && strcasecmp(Buf, "(") != 0))
			{
				T_DSVM_ScriptError("Expected `(` to follow include.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Read String
			QuickRet = T_DSVM_ReadToken(a_Stream, a_End, &Row, Buf, BUFSIZE - 1);
			n = strlen(Buf);
			if (!QuickRet || (QuickRet && (Buf[0] != '\"' || n <= 1 || (n > 1 && Buf[n - 1] != '\"'))))
			{
				T_DSVM_ScriptError("Includes must be double quoted as strings.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Trim Quotes
			strncpy(ExtraBuf, Buf + 1, strlen(Buf) - 2);
			
			// Read token, and expect a ')'
			QuickRet = T_DSVM_ReadToken(a_Stream, a_End, &Row, Buf, BUFSIZE - 1);
			if (!QuickRet || (QuickRet && strcasecmp(Buf, ")") != 0))
			{
				T_DSVM_ScriptError("Expected `)` to follow include.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Read token, and expect a ';'
			QuickRet = T_DSVM_ReadToken(a_Stream, a_End, &Row, Buf, BUFSIZE - 1);
			if (!QuickRet || (QuickRet && strcasecmp(Buf, ";") != 0))
			{
				T_DSVM_ScriptError("Expected `;` to follow include.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Attempt location of include
			Entry = WL_FindEntry(NULL, 0, ExtraBuf);
			
			// Not found?
			if (!Entry)
			{
				// Change all periods to underscores
				n = strlen(ExtraBuf);
				for (i = 0; i < n; i++)
					if (ExtraBuf[i] == '.')
						ExtraBuf[i] = '_';
				
				// Try again
				Entry = WL_FindEntry(NULL, 0, ExtraBuf);
				
				// Did it fail twice?
				if (!Entry)
				{
					T_DSVM_ScriptError("Include not found.", Row);
					ScriptProblem = true;
					break;
				}
			}
			
			// Make sure it isn't on the stack
			for (i = 0; i < l_NumIncStack; i++)
				if (l_IncStack[i] == Entry)
					break;
			
			// It IS on there
			if (i < l_NumIncStack)
			{
				T_DSVM_ScriptError("Recursive include detected.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Open stream on it
			IncStream = WL_StreamOpen(Entry);
			
			// Failed to open?
			if (!IncStream)
			{
				T_DSVM_ScriptError("Failed to open include stream.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Call script parser on it
			QuickRet = T_DSVM_CompileStream(IncStream, Entry->Size);
			
			// Close Stream
			WL_StreamClose(IncStream);
			
			// Something bad happened when including?
			if (!QuickRet)
			{
				T_DSVM_ScriptError("Script error while including.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Continue onto the next token
			continue;
		}
	}
	
	/* Pop from stack */
	// Stack will be empty? Delete it instead
	if (l_NumIncStack == 1)
	{
		if (l_IncStack)
			Z_Free(l_IncStack);
		l_IncStack = NULL;
	}
	
	// Otherwise, resize down
	else
		Z_ResizeArray((void**)&l_IncStack, sizeof(*l_IncStack), l_NumIncStack, l_NumIncStack - 1);
	
	// Decrement
	l_NumIncStack--;
	
	/* Success? */
	return !ScriptProblem;
#undef BUFSIZE
}

/* T_DSVM_Cleanup() -- Cleanup */
bool_t T_DSVM_Cleanup(void)
{
	/* Reset Vars */
	l_ScopeDepth = 0;
	
	/* Free up things */
	if (l_IncStack)
		Z_Free(l_IncStack);
	l_IncStack = NULL;
	l_NumIncStack = 0;
	
	/* Success! */
	return true;
}

