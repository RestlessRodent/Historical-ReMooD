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
// Copyright (C) 2011-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Global RMOD Parsing

/***************
*** INCLUDES ***
***************/

#include "d_rmod.h"
#include "z_zone.h"
#include "dstrings.h"
#include "w_wad.h"
#include "console.h"









// RMOD Handlers are here




/****************
*** CONSTANTS ***
****************/

#define TOKENBUFSIZE 					256		// Max token buffer size

/*****************
*** STRUCTURES ***
*****************/

/* D_RMODTokenInfo_t -- RMOD Token Information */
typedef struct D_RMODTokenInfo_s
{
	WL_ES_t* Stream;					// Stream reader
	size_t StreamEnd;							// When the stream ends
	const char* TokenProblem;					// Problem with token
	
	char Token[TOKENBUFSIZE];					// Current token
	size_t TokenSize;							// Size of buffer
	size_t PutAt;								// Location to put
	
	int32_t CurRow;								// Current row being parsed
	int32_t CurCol;								// Current column being parsed
	int32_t BaseRow;							// Start row being parsed
	int32_t BaseCol;							// Start column being parsed
	const char* ErrStr;							// Error String
} D_RMODTokenInfo_t;

/*************
*** LOCALS ***
*************/

/****************
*** FUNCTIONS ***
****************/

/* DS_RMODPDCRemove() -- Removes loaded data */
static void DS_RMODPDCRemove(const struct WL_WADFile_s* a_WAD)
{
	/* Check */
	if (!a_WAD)
		return;
}

/* DS_RMODReadToken() -- Reads a token from an RMOD */
static bool_t DS_RMODReadToken(D_RMODTokenInfo_t* const a_Info)
{
	uint16_t wc, wcLast;
	int Action;
	bool_t Flipped;
	
	/* Check */
	if (!a_Info)
		return false;
	
	/* Clear current token */
	memset(a_Info->Token, 0, sizeof(*a_Info->Token) * a_Info->TokenSize);
	a_Info->PutAt = 0;
	
	// Also clear problem
	a_Info->TokenProblem = NULL;
	
	// Setup base row (where token starts)
	a_Info->BaseRow = a_Info->CurRow;
	a_Info->BaseCol = a_Info->CurCol;
	
	/* Read Loop */
	for (Action = 0, Flipped = false, wc = WL_Src(a_Info->Stream); WL_StreamTell(a_Info->Stream) < a_Info->StreamEnd; wc = WL_Src(a_Info->Stream))
	{
		// Always increment column
		a_Info->CurCol++;
		
		// White space -- Ignore
		if (wc == ' ' || wc == '\t' || wc == '\r' || wc == '\n')
		{
			// If this is a new line, reset columns and add row
			if (wc == '\n')
			{
				a_Info->CurCol = 0;
				a_Info->CurRow++;
			}
			
			// A token has been read and this is now whitespace
			if (Action != 0)
				break;
			
			// Continue to read whitespace
			continue;
		}
		
		// Comments
		else if (wc == '/')
		{
			// Not a comment or whitespace
			if (Action != 4 && Action != 0)
			{
				a_Info->TokenProblem = "Tokens must be separated by whitespace.";
				break;
			}
			
			// Set as comment
			Action = 4;
			
			// Read character
			wc = WL_Src(a_Info->Stream);
			
			// Check for '/'
			if (wc != '/')
			{
				a_Info->TokenProblem = "Expected // for a comment.";
				return false;
			}
			
			// Already is a comment so ignore character until it is '\n'
			do
			{
				// Read char
				wc = WL_Src(a_Info->Stream);
				
				// Stop at newline
				if (wc == '\n')
					break;
			} while (WL_StreamTell(a_Info->Stream) < a_Info->StreamEnd);
			
			a_Info->CurCol = 0;
			a_Info->CurRow++;
			Action = 0;
			continue;
		}
		
		// Quoted String
		else if (wc == '\"')
		{
			// If not a quote or whitespace
			if (Action != 3 && Action != 0)
			{
				a_Info->TokenProblem = "Tokens must be separated by whitespace.";
				break;
			}
			
			// Set as quote
			Action = 3;
			
			// Place quote in buffer
			if (a_Info->PutAt < a_Info->TokenSize)
				a_Info->Token[a_Info->PutAt++] = wc;
			
			// Read into buffer, until quote is found again
			for (wcLast = 0, wc = WL_Src(a_Info->Stream); WL_StreamTell(a_Info->Stream) < a_Info->StreamEnd; wc = WL_Src(a_Info->Stream))
			{
				// Copy to buffer
				if (a_Info->PutAt < a_Info->TokenSize)
					a_Info->Token[a_Info->PutAt++] = wc;
				
				// Is a quote and is not escaped quote
				if (wc == '\"' && wcLast != '\\')
					break;
				
				// Set last
				wcLast = wc;
			}
			
			// Always break here
			break;
		}
		
		// Singular Tokens
		else if (wc == '{' || wc == '}' || wc == ';')
		{
			// Not a single or whitespace?
			if (Action != 2 && Action != 0)
			{
				a_Info->TokenProblem = "Tokens must be separated by whitespace.";
				break;
			}
			
			// Set as single
			Action = 2;
			
			// Copy to buffer
			if (a_Info->PutAt < a_Info->TokenSize)
				a_Info->Token[a_Info->PutAt++] = wc;
			
			// Done here
			break;
		}
		
		// Standard Tokens
		else
		{
			// Not whitespace and not a standard token
			if (Action != 1 && Action != 0)
				break;
			
			// Copy to buffer
			if (a_Info->PutAt < a_Info->TokenSize)
				a_Info->Token[a_Info->PutAt++] = wc;
			
			// Set as plain token
			Action = 1;
		}
	}
	
	/* There was an actual token here? */
	if (a_Info->PutAt)
	{
		// Check for overflow
		if (a_Info->PutAt >= a_Info->TokenSize)
		{
			a_Info->TokenProblem = "Token buffer overflow.";
			return false;
		}
		
		// Success
		return true;
	}
	
	/* No more tokens */
	return false;
}

bool_t INFO_REMOODATKeyer(void** a_DataPtr, const int32_t a_Stack, const D_RMODCommand_t a_Command, const char* const a_Field, const char* const a_Value);

/* DS_RMODOCCB() -- Order change callback for REMOODAT */
static bool_t DS_RMODOCCB(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD)
{
#define BUFSIZE	256
	char BufF[BUFSIZE], BufV[BUFSIZE];
	int i, ns, j, k, n;
	const WL_WADFile_t* CurWAD;
	WL_ES_t* DataStream;
	D_RMODTokenInfo_t Info;
	const WL_WADEntry_t* Entry;
	int32_t Stack, OldStack;
	bool_t ErrOut;
	void* DataRef;
	char* p;
	
	static const struct
	{
		const char* const LumpName;				// Name of Lump
		const char* const NiceName;				// Nice Name
		D_RMODKeyerFuncType_t Keyer;			// Data Keyer
	} c_RMODNamespaces[] = 
	{
		{"REMOODAT", "ReMooD Data", INFO_REMOODATKeyer},
		{NULL},
	};
	
	/* First Time Initialization */
	for (ns = 0; c_RMODNamespaces[ns].LumpName; ns++)
		if (c_RMODNamespaces[ns].Keyer)
			c_RMODNamespaces[ns].Keyer(NULL, -1, DRC_FIRST, NULL, NULL);
	
	/* Handle all WADs and namespaces */
	for (CurWAD = WL_IterateVWAD(NULL, true); CurWAD; CurWAD = WL_IterateVWAD(CurWAD, true))
		for (ns = 0; c_RMODNamespaces[ns].LumpName; ns++)
		{
			// Attempt locate of WAD Entry
			Entry = WL_FindEntry(CurWAD, 0, c_RMODNamespaces[ns].LumpName);
			
			// Not Found?
			if (!Entry)
			{
				if (devparm)
					CONL_OutputUT(CT_WDATA, DSTR_DRMOD_NAMESPACENOTINWAD, "%s%s\n",
							WL_GetWADName(CurWAD, false), c_RMODNamespaces[ns].NiceName
						);
				continue;
			}
			
			// Use streamer
			DataStream = WL_StreamOpen(Entry);
	
			// Failed?
			if (!DataStream)
			{
				if (devparm)
					CONL_OutputUT(CT_WDATA, DSTR_DRMOD_DATASTREAMERR, "%s%s\n",
							WL_GetWADName(CurWAD, false), c_RMODNamespaces[ns].NiceName
						);
				continue;
			}
			
			// Send Initialize
			DataRef = NULL;
			ErrOut = false;
			if (c_RMODNamespaces[ns].Keyer)
				if (!c_RMODNamespaces[ns].Keyer(&DataRef, -1, DRC_INIT, WL_GetWADName(CurWAD, false), Entry->Name))
				{
					ErrOut = true;
					Info.ErrStr = "No initialization function";
				}
	
			// Determine text type
			WL_StreamCheckUnicode(DataStream);
	
			// Prepare info
			memset(&Info, 0, sizeof(Info));
	
			Info.Stream = DataStream;
			Info.TokenSize = TOKENBUFSIZE;
			Info.StreamEnd = Entry->Size;
			
			// Clear out basic stuff
			Stack = OldStack = 0;
			j = 0;
			
			// Begin stream parse
			while (!ErrOut && DS_RMODReadToken(&Info))
			{
				// Which Handling?
				switch (j)
				{
						// Field
					case 0:
						// Closer
						if (strcasecmp(Info.Token, "}") == 0)
						{
							// Send close to keyer
							if (c_RMODNamespaces[ns].Keyer)
								if (!c_RMODNamespaces[ns].Keyer(&DataRef, Stack, DRC_CLOSE, NULL, NULL))
								{
									ErrOut = true;
									Info.ErrStr = "Close handler failed";
									break;
								}
							
							// Decrease the stack
							OldStack = Stack--;
							
							// Too much stack loss?
							if (Stack < 0)
							{
								ErrOut = true;
								Info.ErrStr = "Too many closing braces";
								break;
							}
														
							// Stay on zero to handle property possibly
						}
						
						// Standard Field Property
						else
						{
							// Validate Field Params
							n = strlen(p = Info.Token);
							for (k = 0; k < n; k++)
								if (!((*p >= 'a' && *p <= 'z') ||
									(*p >= 'A' && *p <= 'Z')))
										break;
							
							// Illegal?
							if (k < n)
							{
								ErrOut = true;
								Info.ErrStr = "Illegal field name";
								break;
							}
						
							// Copy to buffer
							strncpy(BufF, Info.Token, BUFSIZE - 1);
						
							// Legal so move on
							j++;
						}
						break;
						
						// Value
					case 1:
						// Validate
						n = strlen(p = Info.Token);
						
						// Starts and Ends with double quotes
						if (!(*p == '\"' && p[n - 1] == '\"'))
							{
								ErrOut = true;
								Info.ErrStr = "Misquoted String";
								break;
							}
						
						// Remove Last Quote and skip first
						p[n - 1] = 0;
						p++;
						
						// Copy to buffer
						strncpy(BufV, p, BUFSIZE - 1);
						
						// Legal so move on
						j++;
						break;
						
						// Successor
					case 2:
						// Opener?
						if (strcasecmp(Info.Token, "{") == 0)
							// Increase the stack
							OldStack = Stack++;
						
						// Property?
						else if (strcasecmp(Info.Token, ";") == 0)
							OldStack = Stack;
						
						// Illegal
						else
						{
							ErrOut = true;
							Info.ErrStr = "Incorrect line terminator";
							break;
						}
						
						// Send to keyer
						if (c_RMODNamespaces[ns].Keyer)
							if (!c_RMODNamespaces[ns].Keyer(&DataRef, Stack, (Stack == OldStack ? DRC_DATA : DRC_OPEN), BufF, BufV))
							{
								ErrOut = true;
								Info.ErrStr = "Open handler failed";
								break;
							}
						
						// Legal, go back to start
						j = 0;
						break;
				}
			}
			
			// Problems?
			if (ErrOut)
				if (devparm)
				{
					// Unknown error?
					if (!Info.ErrStr)
						if (Info.TokenProblem)
							Info.ErrStr = Info.TokenProblem;
						else
							Info.ErrStr = "Unknown error";
					
					// Print
					CONL_OutputUT(CT_WDATA, DSTR_DRMOD_PARSEERROR, "%s%i%s%i%i%s%08x%s%i\n",
							WL_GetWADName(CurWAD, false), Info.BaseRow, Entry->Name, Info.CurRow, Info.CurCol, Info.ErrStr, (int)WL_StreamTell(Info.Stream), Info.Token, Info.BaseCol
						);
				}
			
			// If any stack remains send closure
			ErrOut = false;
			while (Stack > 0)
			{
				// Send close to keyer to simulate short REMOODAT
				if (c_RMODNamespaces[ns].Keyer)
					if (c_RMODNamespaces[ns].Keyer(&DataRef, Stack, DRC_CLOSE, NULL, NULL))
						ErrOut = true;
				
				// Decrease Stack
				Stack--;
			}
			
			// Send Finalization
			if (c_RMODNamespaces[ns].Keyer)
				c_RMODNamespaces[ns].Keyer(&DataRef, -1, DRC_FINAL, WL_GetWADName(CurWAD, false), Entry->Name);
		}
		
	/* Last Time Initialization */
	for (ns = 0; c_RMODNamespaces[ns].LumpName; ns++)
		if (c_RMODNamespaces[ns].Keyer)
			c_RMODNamespaces[ns].Keyer(NULL, -1, DRC_LAST, NULL, NULL);
	
	/* Success! */
	return true;
#undef BUFSIZE
}

/* D_InitRMOD() -- Initializes REMOODAT handling */
void D_InitRMOD(void)
{
	/* Only the OCCB remains now */
	if (!WL_RegisterOCCB(DS_RMODOCCB, WLDCO_RMOD))
		I_Error("D_InitRMOD: Failed to register OCCB.");
}

