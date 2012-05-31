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
// DESCRIPTION: Global RMOD Parsing

/***************
*** INCLUDES ***
***************/

#include "doomdef.h"
#include "d_rmod.h"
#include "m_menu.h"
#include "console.h"
#include "z_zone.h"
#include "w_wad.h"
#include "d_block.h"							// Cache

// RMOD Handlers are here
#include "v_widget.h"
#include "p_spec.h"
#include "info.h"

/****************
*** CONSTANTS ***
****************/

#define TOKENBUFSIZE 					256		// Max token buffer size

/*****************
*** STRUCTURES ***
*****************/

/* D_WXRMODPrivate_t -- RMOD Private Data */
typedef struct D_WXRMODPrivate_s
{
	Z_Table_t* RMODTable;						// Table to store RMOD data
} D_WXRMODPrivate_t;

/* D_RMODTokenInfo_t -- RMOD Token Information */
typedef struct D_RMODTokenInfo_s
{
	WL_EntryStream_t* Stream;					// Stream reader
	size_t StreamEnd;							// When the stream ends
	const char* TokenProblem;					// Problem with token
	
	char Token[TOKENBUFSIZE];					// Current token
	size_t TokenSize;							// Size of buffer
	size_t PutAt;								// Location to put
	
	int32_t CurRow;								// Current row being parsed
	int32_t CurCol;								// Current column being parsed
} D_RMODTokenInfo_t;

/* D_RMODHandler_t -- Handles RMOD Data */
typedef struct D_RMODHandler_s
{
	const char* TableType;						// Type of table
	D_RMODHandleFunc_t HandleFunc;				// Handler function
	D_RMODOCCBFunc_t OrderFunc;					// OCCB Variant
} D_RMODHandler_t;

/* D_RMODWADStuff_t -- Stuff in for a WAD */
typedef struct D_RMODWADStuff_s
{
	const WL_WADFile_t* WAD;					// WAD this belongs to
	D_RMODPrivate_t Private[NUMDRMODPRIVATES];	// Private Stuff
} D_RMODWADStuff_t;

/*************
*** LOCALS ***
*************/

// c_RMODHandlers -- Handlers for RMOD
static const D_RMODHandler_t c_RMODHandlers[NUMDRMODPRIVATES] =
{
	// Widgets
	{
		"widget",
		V_WidgetRMODHandle,
		V_WidgetRMODOrder,
	},
	
	/* Objects */
	// Objects
	{
		"MapObject",
		INFO_RMODH_MapObjects,
		INFO_RMODO_MapObjects,
	},
	
	// Ammo
	{
		"MapAmmo",
		P_RMODH_WeaponsAmmo,
		P_RMODO_WeaponsAmmo,
	},
	
	// Weapons
	{
		"MapWeapon",
		P_RMODH_WeaponsAmmo,
		NULL,
	},
	
	/* Specials */
	// Sectors
	{
		"MapSectorSpecial",
		P_RMODH_Specials,
		P_RMODO_Specials,
	},
	
	// Touchers
	{
		"MapTouchSpecial",
		P_RMODH_Specials,
		NULL,
	},
};

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
	
	/* Read Loop */
	for (Action = 0, Flipped = false, wc = WL_StreamReadChar(a_Info->Stream); WL_StreamTell(a_Info->Stream) < a_Info->StreamEnd; wc = WL_StreamReadChar(a_Info->Stream))
	{
		// Always increment column
		a_Info->CurCol++;
		
		// White space -- Ignore
		if (wc == ' ' || wc == '\t' || wc == '\r' || wc == '\n')
		{
			// A token has been read and this is now whitespace
			if (Action != 0)
				break;
			
			// If this is a new line, reset columns and add row
			if (wc == '\n')
			{
				a_Info->CurCol = 0;
				a_Info->CurRow++;
			}
			
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
			wc = WL_StreamReadChar(a_Info->Stream);
			
			// Check for '/'
			if (wc != '/')
			{
				a_Info->TokenProblem = "Expected // for a comment.";
				return false;
			}
			
			// Already is a comment so ignore character until it is '\n'
			for (wc = WL_StreamReadChar(a_Info->Stream); WL_StreamTell(a_Info->Stream) < a_Info->StreamEnd; wc = WL_StreamReadChar(a_Info->Stream))
				if (wc == '\n')
					break;
			
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
			for (wcLast = 0, wc = WL_StreamReadChar(a_Info->Stream); WL_StreamTell(a_Info->Stream) < a_Info->StreamEnd; wc = WL_StreamReadChar(a_Info->Stream))
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

/* DS_RMODPDC() -- Creates private data from REMOODAT */
static bool_t DS_RMODPDC(const struct WL_WADFile_s* const a_WAD, const uint32_t a_Key, void** const a_DataPtr, size_t* const a_SizePtr, WL_RemoveFunc_t* const a_RemoveFuncPtr)
{
#define BUFSIZE 384
	const WL_WADEntry_t* DataEntry;
	WL_EntryStream_t* DataStream;
	int i = 0, n, Expected, Deepness;
	uint16_t wc;
	D_RMODTokenInfo_t Info;
	Z_Table_t* CurrentTable;
	const char* tP;
	D_RMODWADStuff_t* Stuff;
	D_RBlockStream_t* CacheStream;
	uint32_t u32a, u32b;
	
	static bool_t CheckRecursive = false;
	
	const char* ErrorText;
	char TokVals[2][BUFSIZE];
	bool_t DoCache = false;
	
	/* Recursive call? */
	if (CheckRecursive)
	{
		if (devparm)
			CONL_PrintF("DS_RMODPC: Recursive call to DS_RMODPDC, doing nothing.\n");
		return true;
	}
	
	/* Mark as recursive */
	CheckRecursive = true;
	
	/* Check to see if REMOODAT exists in this WAD */
	if (!(DataEntry = WL_FindEntry(a_WAD, 0, "REMOODAT")))
	{
		if (devparm)
			CONL_PrintF("DS_RMODPDC: There is no REMOODAT here.\n");
		CheckRecursive = false;
		return true;
	}
	
	/* Create stuff to store in WAD */
	*a_SizePtr = sizeof(D_RMODWADStuff_t);
	*a_DataPtr = Stuff = Z_Malloc(*a_SizePtr, PU_WLDKRMOD, NULL);
	
	// Initial Stuff
	Stuff->WAD = a_WAD;
	
	/* REMOODAT ZTABLE CACHE */
	// Determine pathname for cache file
		// Stored in ReMooD Data Directory
	memset(TokVals[0], 0, sizeof(TokVals[0]));
	memset(TokVals[1], 0, sizeof(TokVals[1]));
	
	// Directory to place at
	I_GetStorageDir(TokVals[0], BUFSIZE - 1, DST_DATA);
	
	// Filename to write to, then convert to lowercase
	snprintf(TokVals[1], BUFSIZE - 1, "%s", a_WAD->__Private.__DOSName);
	tP = strchr(TokVals[1], '.');
	if (tP)
		(*((char*)tP)) = 0;
	strncat(TokVals[1], ".dat", BUFSIZE - 1);
	C_strlwr(TokVals[1]);
	
	// Append
	strncat(TokVals[0], "/", BUFSIZE - 1);
	strncat(TokVals[0], TokVals[1], BUFSIZE - 1);
	
	// Try opening the stream RO
	if (devparm)
		CONL_PrintF("DS_RMODPDC: Attempting cache \"%s\"...\n", TokVals[0]);
	CacheStream = D_RBSCreateFileStream(TokVals[0], DRBSSF_READONLY);
	
	// If it failed, it does not exist
	if (!CacheStream)
		DoCache = false;
	
	// Otherwise read the first block and determine whether it is cache worthy
	else
	{
		// Intially use the cache
		DoCache = true;
		
		// Read the first block
		memset(TokVals[1], 0, sizeof(TokVals[1]));
		if (!D_RBSPlayBlock(CacheStream, TokVals[1]))
			DoCache = false;
		else
		{
			// Not the header we expected?
			if (!D_RBSCompareHeader("PWAD", TokVals[1]))
				DoCache = false;
				
			// Read into it more
			else
			{
				// Read Values
				u32a = D_RBSReadUInt32(CacheStream);
				u32b = D_RBSReadUInt32(CacheStream);
				memset(TokVals[1], 0, sizeof(TokVals[1]));
				D_RBSReadString(CacheStream, TokVals[1], sizeof(TokVals[1]));
			
				// If neither of them match! This is a different WAD
				if (u32a != a_WAD->__Private.__IndexOff ||
					u32b != a_WAD->__Private.__Size ||
					strcmp(TokVals[1], a_WAD->SimpleSumChars))
					DoCache = false;
			}
		}
		
		// Invalid, Non-Matching, Or Out of Date
		if (!DoCache)
		{
			D_RBSCloseStream(CacheStream);
			CacheStream = NULL;
		}
	}
	
	/* Cached REMOODAT */
	// Although the same table functions are called (for simplicity)
	// Parsing the file structure isn't needed at all
	if (DoCache)
	{
		// Restore table from the block cache
		while ((CurrentTable = Z_TableStreamToStore(CacheStream)))
		{
			// Find handler for this table type
			tP = Z_TableName(CurrentTable);
			
			fprintf(stderr, "Cache from %s\n", tP);
	
			// Sanity check
			if (!tP)
			{
				ErrorText = "Table has no name?";
				break;
			}
	
			// Copy until # is found
			n = strlen(tP);
			for (i = 0; i < n; i++)
				if (tP[i] == '#')
					break;
				else
					TokVals[1][i] = tP[i];
			TokVals[1][i] = 0;
			
			// Look in list for a match
			for (i = 0; i < NUMDRMODPRIVATES; i++)
				if (strcasecmp(TokVals[1], c_RMODHandlers[i].TableType) == 0)
				{
					// Call handler
					if (c_RMODHandlers[i].HandleFunc)
						if (!c_RMODHandlers[i].HandleFunc(CurrentTable, a_WAD, i, &Stuff->Private[i]))
						CONL_PrintF("DS_RMODPDC: Handler for \"%s\" failed.\n", TokVals[1]);
			
					// No more handling needed
					break;
				}
	
			// Not found?
			if (i == NUMDRMODPRIVATES)
				CONL_PrintF("DS_RMODPDC: No data handler for \"%s\".\n", TokVals[1]);
	
			// Destroy table, not needed
			Z_TableDestroy(CurrentTable);
			CurrentTable = NULL;
		}
	}
	
	/* Non-Cached REMOODAT */
	else
	{
		// Prepare for cache write
		CacheStream = D_RBSCreateFileStream(TokVals[0], DRBSSF_OVERWRITE);
		
		// Write Information header to the stream
		if (CacheStream)
		{
			// WAD Information Header
			D_RBSBaseBlock(CacheStream, "PWAD");
			D_RBSWriteUInt32(CacheStream, a_WAD->__Private.__IndexOff);
			D_RBSWriteUInt32(CacheStream, a_WAD->__Private.__Size);
			D_RBSWriteString(CacheStream, a_WAD->SimpleSumChars);
			D_RBSRecordBlock(CacheStream);
		}
		
		// Info
		if (devparm)
			CONL_PrintF("DS_RMODPDC: Parsing REMOODAT...\n");
		else
			CONL_EarlyBootTic("Parsing REMOODAT", true);
	
		// Use streamer
		DataStream = WL_StreamOpen(DataEntry);
	
		// Failed?
		if (!DataStream)
		{
			if (devparm)
				CONL_PrintF("DS_RMODPDC: Failed to open stream.\n");
			CheckRecursive = false;
			return false;
		}
	
		// Determine text type
		WL_StreamCheckUnicode(DataStream);
	
		// Begin stream parse
		// Prepare info
		memset(&Info, 0, sizeof(Info));
	
		Info.Stream = DataStream;
		Info.TokenSize = TOKENBUFSIZE;
		Info.StreamEnd = DataEntry->Size;
	
		// Reset variables
		Expected = 0;
		ErrorText = NULL;
		Deepness = 0;
		CurrentTable = NULL;
		
		// Token read loop
		while (DS_RMODReadToken(&Info))
		{
			// Closing Brace -- Step out of a grouplet
			if (Expected == 0 && Info.Token[0] == '}')
			{
				// Decrease deepness
				Deepness--;
			
				// Too deep?
				if (Deepness < 0)
				{
					ErrorText = "Too many closing braces \'}\'";
					break;
				}
			
				// Change the current table
				if (CurrentTable)
				{
					// No deepness
					if (Deepness == 0)
					{
						// Find handler for this table type
						tP = Z_TableName(CurrentTable);
					
						// Sanity check
						if (!tP)
						{
							ErrorText = "Table has no name?";
							break;
						}
					
						// Copy until # is found
						n = strlen(tP);
						for (i = 0; i < n; i++)
							if (tP[i] == '#')
								break;
							else
								TokVals[0][i] = tP[i];
						TokVals[0][i] = 0;
						
						// Cache the table for future usage
						Z_TableStoreToStream(CurrentTable, CacheStream);
					
						// Look in list for a match
						for (i = 0; i < NUMDRMODPRIVATES; i++)
							if (strcasecmp(TokVals[0], c_RMODHandlers[i].TableType) == 0)
							{
								// Call handler
								if (c_RMODHandlers[i].HandleFunc)
									if (!c_RMODHandlers[i].HandleFunc(CurrentTable, a_WAD, i, &Stuff->Private[i]))
									CONL_PrintF("DS_RMODPDC: Handler for \"%s\" failed.\n", TokVals[0]);
							
								// No more handling needed
								break;
							}
					
						// Not found?
						if (i == NUMDRMODPRIVATES)
							CONL_PrintF("DS_RMODPDC: No data handler for \"%s\".\n", TokVals[0]);
					
						// Destroy table, not needed
						Z_TableDestroy(CurrentTable);
						CurrentTable = NULL;
					}
				
					// Otherwise, go up a table
					else
						CurrentTable = Z_TableUp(CurrentTable);
				}
			}
		
			// Property -- Add to buffer
			else if (Expected == 0)
			{
				// Remember the token
				strncpy(TokVals[0], Info.Token, BUFSIZE);
			
				// Expect Value now
				Expected++;
			
				// Lowercase the property
				n = strlen(TokVals[0]);
				for (i = 0; i < n; i++)
				{
					// Lowercase
					TokVals[0][i] = tolower(TokVals[0][i]);
				
					// Not alphanumeric?
					if (!((TokVals[0][i] >= '0' && TokVals[0][i] <= '9') || (TokVals[0][i] >= 'a' && TokVals[0][i] <= 'z')))
					{
						ErrorText = "Properties must use only alphanumeric characters (a-z, A-Z, and 0-9)";
						break;
					}
				}
			
				// Error?
				if (ErrorText)
					break;
			
				// Only limit of 64 characters
				if (strlen(TokVals[0]) >= 64)
				{
					ErrorText = "Properties are limited to 64 characters";
					break;
				}
			}
		
			// Value -- Add to buffer
			else if (Expected == 1)
			{
				// Remember the token
				strncpy(TokVals[1], Info.Token, BUFSIZE);
			
				// Must start and end with quotes
				n = strlen(TokVals[1]);
				if (TokVals[1][0] != '\"' && TokVals[1][n - 1] != '\"')
				{
					ErrorText = "Values must be quoted";
					break;
				}
			
				// Move over and and remove last character (quote purge)
				TokVals[1][n - 1] = 0;
				memmove(&TokVals[1][0], &TokVals[1][1], sizeof(TokVals[1]) - (sizeof(*TokVals[1]) * 1));
			
				// Expect type now
				Expected++;
			}
		
			// Type -- Type of Prop/Val, Table or entry?
			else
			{
				// Check for illegal type first
				if (Info.Token[0] != '{' && Info.Token[0] != ';')
				{
					ErrorText = "Expected { or ; after value";
					break;
				}
			
				// If it is a table, create new table
				if (Info.Token[0] == '{')
				{			
					// Make value lowercase
					n = strlen(TokVals[1]);
					for (i = 0; i < n; i++)
					{
						// Lowercase
						TokVals[1][i] = tolower(TokVals[1][i]);
			
						// Not alphanumeric?
						if (!((TokVals[1][i] >= '0' && TokVals[1][i] <= '9') || (TokVals[1][i] >= 'a' && TokVals[1][i] <= 'z')))
						{
							ErrorText = "Table names (values) must use only alphanumeric characters (a-z, A-Z, and 0-9)";
							break;
						}
					}
				
					// Problem?
					if (ErrorText)
						break;
				
					// Only limit of 64 characters
					if (strlen(TokVals[1]) >= 64)
					{
						ErrorText = "Table names are limited to 64 characters";
						break;
					}
					
					// Concat # onto name
					strncat(TokVals[0], "#", BUFSIZE);
				
					// Concat value onto name
					strncat(TokVals[0], TokVals[1], BUFSIZE);
				
					// Create table (or subtable)
					if (Deepness == 0)	// Table
					{
						// Create the table with the specified value/key
						CurrentTable = Z_TableCreate(TokVals[0]);
					}
					else				// Subtable
					{
						// Create a sub table
						CurrentTable = Z_FindSubTable(CurrentTable, TokVals[0], true);
					}
				
					// Increase deepness
					Deepness++;
				}
			
				// Otherwise, it is a property, add to table
				else
				{
					// Cannot have properties at level 0
					if (Deepness == 0)
					{
						ErrorText = "Properties must be within tables";
						break;
					}
				
					// Add to table
					Z_TableSetValue(CurrentTable, TokVals[0], TokVals[1]);
				}
			
				// Reset expected and clear buffers
				Expected = 0;
				memset(TokVals, 0, sizeof(TokVals));
			}
		}
	
		// Was there a problem?
		if (Info.TokenProblem)
			CONL_PrintF("DS_RMODPDC: Token error \"%s\" at row %i, column %i.\n", Info.TokenProblem, Info.CurRow + 1, Info.CurCol);
	
		if (ErrorText)
			CONL_PrintF("DS_RMODPDC: Parse error \"%s\" at row %i, column %i.\n", ErrorText, Info.CurRow + 1, Info.CurCol);
	
		// Free streamer
		WL_StreamClose(DataStream);
		
		// Finish off Cache
		if (CacheStream)
		{
			// Done Marker
			D_RBSBaseBlock(CacheStream, "DONE");
			D_RBSRecordBlock(CacheStream);
		}
	}
	
	/* Close stream if it was left open */
	if (CacheStream)
		D_RBSCloseStream(CacheStream);
	
	/* Unmark recursive */
	CheckRecursive = false;
	
	return true;
#undef BUFSIZE
}

/* DS_RMODOCCB() -- Order change callback for REMOODAT */
static bool_t DS_RMODOCCB(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD)
{
	int i;
	
	/* Call each order notifier */
	for (i = 0; i < NUMDRMODPRIVATES; i++)
		if (c_RMODHandlers[i].OrderFunc)
			if (!c_RMODHandlers[i].OrderFunc(a_Pushed, a_WAD, i))
				if (devparm)
					CONL_PrintF("DS_RMODOCCB: Order change for \"%s\" failed.\n", c_RMODHandlers[i].TableType);
	
	/* Success! */
	return true;
}

/* D_InitRMOD() -- Initializes REMOODAT handling */
void D_InitRMOD(void)
{
	/* Hook WL handlers */
	// Register PDC
	if (!WL_RegisterPDC(WLDK_RMOD, WLDPO_RMOD, DS_RMODPDC, DS_RMODPDCRemove))
		I_Error("D_InitRMOD: Failed to register PDC.");
	
	// Register OCCB (this builds a composite)
	if (!WL_RegisterOCCB(DS_RMODOCCB, WLDCO_RMOD))
		I_Error("D_InitRMOD: Failed to register OCCB.");
}

/* D_GetRMODPrivate() -- Get private RMOD data from this WAD */
D_RMODPrivate_t* D_GetRMODPrivate(const WL_WADFile_t* const a_WAD, const D_RMODPrivates_t a_ID)
{
	D_RMODWADStuff_t* Stuff;
	int i;
	
	/* Check */
	if (!a_WAD || a_ID < 0 || a_ID >= NUMDRMODPRIVATES)
		return NULL;
	
	/* Get stuff from current WAD */
	Stuff = WL_GetPrivateData(a_WAD, WLDK_RMOD, NULL);
	
	// Failed?
	if (!Stuff)
		return NULL;
	
	/* Return the private stuff */
	return &Stuff->Private[a_ID];
}

/* D_RMODGetBool() -- Return boolean value of string */
bool_t D_RMODGetBool(const char* const a_Str)
{
	/* Check */
	if (!a_Str)
		return false;
	
	/* Dictionary Attack */
	if (strcasecmp(a_Str, "true") == 0 || strcasecmp(a_Str, "yes") == 0 || strcasecmp(a_Str, "on") == 0 || atoi(a_Str) != 0)
		return true;
	
	/* Everything else is false */
	return false;
}

/* D_RMODGetValueFixed() -- Get fixed value */
fixed_t D_RMODGetValueFixed(Z_Table_t* const a_Table, const char* const a_Value, const fixed_t a_MissingVal)
{
	const char* Value;
	
	/* Check */
	if (!a_Table || !a_Value)
		return a_MissingVal;
	
	if (!(Value = Z_TableGetValue(a_Table, a_Value)))
		return a_MissingVal;
	else
		return (fixed_t)(atof(Value) * 65536.0);
}

/* D_RMODGetValueInt() -- Get RMOD Value Int */
int32_t D_RMODGetValueInt(Z_Table_t* const a_Table, const char* const a_Value, const int32_t a_MissingVal)
{
	int32_t Value;
	bool_t Found;
	
	/* Check */
	if (!a_Table || !a_Value)
		return a_MissingVal;
		
	/* Obtain value */
	Value = Z_TableGetValueInt(a_Table, a_Value, &Found);
	
	// if it was found, return that value, otherwise the default
	if (Found)
		return Value;
	return a_MissingVal;
}

/* D_RMODGetValueInt() -- Get RMOD Value Int */
bool_t D_RMODGetValueBool(Z_Table_t* const a_Table, const char* const a_Value, const bool_t a_MissingVal)
{
	const char* Value;
	
	/* Check */
	if (!a_Table || !a_Value)
		return a_MissingVal;
	
	if (!(Value = Z_TableGetValue(a_Table, a_Value)))
		return a_MissingVal;
	else
		return D_RMODGetBool(Value);
}

/* D_RMODGetValueString() -- Get string value */
char* D_RMODGetValueString(Z_Table_t* const a_Table, const char* const a_Value, const char* const a_MissingVal)
{
	const char* Value;
	
	/* Check */
	if (!a_Table || !a_Value)
		return a_MissingVal;
	
	if (!(Value = Z_TableGetValue(a_Table, a_Value)))
		return (a_MissingVal ? Z_StrDup(a_MissingVal, PU_WLDKRMOD, NULL) : NULL);
	else
		return Z_StrDup(Value, PU_WLDKRMOD, NULL);
}

