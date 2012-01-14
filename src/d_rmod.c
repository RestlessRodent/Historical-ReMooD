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
// Copyright (C) 2011 GhostlyDeath <ghostlydeath@remood.org>
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

/****************
*** CONSTANTS ***
****************/

#define TOKENBUFSIZE 					512		// Max token buffer size

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
			
			if (wc == '\n')
			{
				a_Info->CurCol = 0;
				a_Info->CurRow++;
				Action = 0;
				continue;
			}
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
		return true;
	
	/* No more tokens */
	return false;
}

/* DS_RMODPDC() -- Creates private data from REMOODAT */
static bool_t DS_RMODPDC(const struct WL_WADFile_s* const a_WAD, const uint32_t a_Key, void** const a_DataPtr, size_t* const a_SizePtr, WL_RemoveFunc_t* const a_RemoveFuncPtr)
{
#define BUFSIZE 512
	const WL_WADEntry_t* DataEntry;
	WL_EntryStream_t* DataStream;
	int i = 0;
	uint16_t wc;
	D_RMODTokenInfo_t Info;
	
	/* Check to see if REMOODAT exists in this WAD */
	if (!(DataEntry = WL_FindEntry(a_WAD, 0, "REMOODAT")))
	{
		if (devparm)
			CONS_Printf("DS_RMODPDC: There is no REMOODAT here.\n");
		return true;
	}
	
	// Info
	if (devparm)
		CONS_Printf("DS_RMODPDC: Parsing REMOODAT...\n");
	
	/* Use streamer */
	DataStream = WL_StreamOpen(DataEntry);
	
	// Failed?
	if (!DataStream)
	{
		if (devparm)
			CONS_Printf("DS_RMODPDC: Failed to open stream.\n");
		return false;
	}
	
	/* Determine text type */
	WL_StreamCheckUnicode(DataStream);
	
	/* Begin stream parse */
	// Prepare info
	memset(&Info, 0, sizeof(Info));
	
	Info.Stream = DataStream;
	Info.TokenSize = TOKENBUFSIZE;
	Info.StreamEnd = DataEntry->Size;
	
	// Token read loop
	while (DS_RMODReadToken(&Info))
	{
		CONS_Printf(">> `%s`\n", Info.Token);
	}
	
	// Was there a problem?
	if (Info.TokenProblem)
		CONS_Printf("DS_RMODPDC: Parse error \"%s\" at row %i, column %i.\n", Info.TokenProblem, Info.CurRow + 1, Info.CurCol);
	
	/* Free streamer */
	WL_StreamClose(DataStream);
	
	return true;
#undef BUFSIZE
}

/* DS_RMODOCCB() -- Order change callback for REMOODAT */
static bool_t DS_RMODOCCB(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD)
{
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

/* DS_RMODNextToken() -- Returns the next token in an RMOD */
// Returns true if a token was found
static bool_t DS_RMODNextToken(const char* const a_DataBase, const size_t a_DataSize, const char** const a_P, char* const a_TokenBuf, const size_t a_TokenSize,
                               uint32_t* const a_Col, uint32_t* const a_Row)
{
	char c;
	int Action;					// Action to take
	size_t t;
	
	/* Check */
	if (!a_DataBase || !a_DataSize || !a_P || !*a_P || !a_TokenBuf || !a_TokenSize || !a_Col || !a_Row)
		return false;
		
	/* Clear token buffer */
	memset(a_TokenBuf, 0, a_TokenSize);
	
	/* Read loop */
	for (t = 0, Action = 0, c = **a_P;** a_P && *a_P < (a_DataBase + a_DataSize); c = *(++(*a_P)))
	{
		// Always increment on columns
		(*a_Col)++;
		
		// Whitespace, ignore
		if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
		{
			// If this is a newline, reset columns and bump rows
			if (c == '\n')
			{
				*a_Col = 0;
				(*a_Row)++;
			}
			// Token has been read
			if (Action != 0)
			{
				(*a_P)++;		// Increment due to break
				break;
			}
			// Continue reading whitespace
			continue;
		}
		// Comment
		else if (c == '/')
		{
			// If action is not a comment
			if (Action != 4 && Action != 0)
				break;
				
			// Already in comment? skip to end
			if (Action == 4)
			{
				// Read until newline
				for (c = **a_P;** a_P && *a_P < (a_DataBase + a_DataSize); c = *(++(*a_P)))
					if (c == '\n')
						break;
						
				*a_Col = 0;		// reset column since we will newlined
				(*a_Row)++;		// same for row
				Action = 0;		// Reset to nothing
				continue;		// Continue on
			}
			// Set action as comment
			Action = 4;
		}
		// Quoted literal
		else if (c == '\"')
		{
			// If action is not a quote
			if (Action != 3 && Action != 0)
				break;
				
			// Set action as quote
			Action = 3;
			
			// Chuck first quote on buffer
			if (t < a_TokenSize - 1)
				a_TokenBuf[t++] = c;
			(*a_P)++;			// Increment to break quote
			
			// Keep reading
			for (c = **a_P;** a_P && *a_P < (a_DataBase + a_DataSize); c = *(++(*a_P)))
			{
				// Quote?
				if (c == '\"')
				{
					if (t < a_TokenSize - 1)
						a_TokenBuf[t++] = '\"';	// slap quote
					(*a_P)++;	// Increment due to break
					break;
				}
				// Blind copy
				if (t < a_TokenSize - 1)
					a_TokenBuf[t++] = c;
			}
			
			break;
		}
		// Single tokens
		else if (c == '{' || c == '}' || c == ';')
		{
			// If action is not a single
			if (Action != 2 && Action != 0)
				break;
				
			// Set action as single
			Action = 2;
			
			a_TokenBuf[0] = c;
			t = 1;
			(*a_P)++;			// Increment due to break
			break;				// break here
		}
		// A normal token
		else
		{
			// If action is not a normal
			if (Action != 1 && Action != 0)
				break;
				
			// Set action as token
			Action = 1;
			
			// Copy to token buffer
			if (t < a_TokenSize - 1)
				a_TokenBuf[t++] = c;
		}
	}
	
	/* As long as t is positive */
	if (t != 0)
		return true;
		
	/* Nothing here */
	return false;
}

/* DS_RMODConfirmProperty() -- Checks a property to check whether it is valid */
static bool_t DS_REMODConfirmProperty(const char* const a_Property)
{
	const char* p;
	
	/* Check */
	if (!a_Property)
		return false;
		
	/* Limit property keys to 64 characters */
	if (strlen(a_Property) > 64)
		return false;
		
	/* Rove around */
	for (p = a_Property; *p; p++)
		// fail on illegal characters
		if (!((*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || (*p == '_') || (*p == '@')))
			return false;
			
	/* Success */
	return true;
}

/* DS_RMODTableHandler() -- Handles a table */
static bool_t DS_RMODTableHandler(Z_Table_t* const a_Sub, void* const a_Data)
{
#define PREFIXSIZE 48
	const char* q;
	const char* p;
	size_t n, j;
	struct
	{
		const char const Prefix[PREFIXSIZE];	// Prefix# to look for
		bool_t (*Handler) (Z_Table_t* const a_Table, const char* const a_ID, void* const a_Data);
	} Handlers[] =
	{
		//{"menu#", M_LoadMenuTable},
		{
			"", NULL
		}
	};
	
	/* Check */
	if (!a_Sub)
		return false;
		
	/* Obtain ID from thing and determine its type */
	p = q = Z_TableName(a_Sub);
	q = strchr(q, '#');
	n = q - p;
	
	// Increment to remove # for a_ID passing
	q++;
	
	/* Go through the list */
	for (j = 0; Handlers[j].Handler; j++)
	{
		// Try a name match
		if (strncasecmp(Handlers[j].Prefix, p, n) == 0)
		{
			// Send to handler
			Handlers[j].Handler(a_Sub, q, a_Data);
			break;				// No more searching needed
		}
	}
	
	// Failure?
	if (!Handlers[j].Handler)
		if (devparm)
			CONS_Printf("RMOD: Unknown table \"%.*s\"\n", n, p);
			
	/* Always return true! */
	return true;
	
#undef PREFIXSIZE
}

/* D_WX_RMODMultiBuild() -- RMOD Multi builder */
void D_WX_RMODMultiBuild(WX_WADFile_t* const a_WAD, const WX_BuildAction_t a_Action)
{
#define BUFSIZE 512
	char Token[BUFSIZE];
	int j;
	char KeyProperty[BUFSIZE], KeyValue[BUFSIZE];
	D_WXRMODPrivate_t* Private;
	WX_WADEntry_t* Entry;
	size_t Size, z;
	const char* Data;
	void** PvPtr;
	size_t* PvSize;
	Z_Table_t* CurrentTable;
	const char* p;
	const char* ErrP;
	bool_t Fail;
	WX_WADFile_t* Wover;
	
	uint32_t cCol, cRow, lCol, lRow;
	
	/* Check */
	if (!a_WAD)
		return;
		
	/* If we are building or clearing, get private data */
	if (a_Action >= WXBA_BUILDWAD && a_Action <= WXBA_CLEARWAD)
	{
		// Get private data from WAD
		if (!WX_GetVirtualPrivateData(a_WAD, WXDPID_RMOD, &PvPtr, &PvSize))
			return;
			
		// Check whether it really exists, if not create it
		if (!*PvPtr)
		{
			*PvSize = sizeof(D_WXRMODPrivate_t);
			*PvPtr = Z_Malloc(*PvSize, PU_STATIC, NULL);
		}
		// Set private data
		Private = (D_WXRMODPrivate_t*) * PvPtr;
	}
	else
		Private = 0;
		
	/* Based on action */
	switch (a_Action)
	{
			// Build single WAD
		case WXBA_BUILDWAD:
			// Obtain entry and make sure we got it
			Entry = WX_EntryForName(a_WAD, "REMOODAT", false);
			if (!Entry)
				return;
				
			// Attempt loading of entry data
			Data = WX_CacheEntry(Entry);
			if (!Data)
				return;
				
			// Size of entry
			Size = WX_GetEntrySize(Entry);
			
			// Create the root table
			Private->RMODTable = Z_TableCreate("remood");
			CurrentTable = Private->RMODTable;
			
			// Prepare RMOD Reading
			lCol = cCol = 0;	// Columns are zero based since they are always incremented
			lRow = cRow = 1;	// non-zero based rows
			p = Data;
			j = 0;
			
			// Read RMOD Data
			Fail = false;
			ErrP = "Unknown";
			while (DS_RMODNextToken(Data, Size, &p, Token, BUFSIZE, &cCol, &cRow))
			{
				// Expect property
				if (j == 0)
				{
					// End of table
					if (strcmp("}", Token) == 0)
					{
						// Get parent table
						CurrentTable = Z_TableUp(CurrentTable);
						
						// Did we go up too many times?
						if (!CurrentTable)
						{
							ErrP = "Too many closing braces";
							Fail = true;
							break;
						}
						// Continue to the next token
						continue;
					}
					// Lowercase it
					C_strlwr(Token);
					
					// Confirm the property
					if (!DS_REMODConfirmProperty(Token))
					{
						ErrP = "Property contains invalid characters or > 64";
						Fail = true;
						break;
					}
					// Copy to property buffer
					memset(KeyProperty, 0, sizeof(KeyProperty));
					strncpy(KeyProperty, Token, BUFSIZE);
					
					// Increment j and go to value now
					j++;
				}
				// Expect value
				else if (j == 1)
				{
					// Values must start and end with "
					z = strlen(Token);
					
					if (Token[0] != Token[z - 1] && Token[0] != '\"')
					{
						ErrP = "Values must always be quoted";
						Fail = true;
						break;
					}
					// Copy the quote free chunk
					memset(KeyValue, 0, sizeof(KeyValue));
					strncpy(KeyValue, Token + 1, z - 2);
					
					// Increment j and go to statement type
					j++;
				}
				// Now expect either { or ;
				else
				{
					// Create table
					if (strcmp(Token, "{") == 0)
					{
						// Lowercase value
						C_strlwr(KeyValue);
						
						// Table values are limited to properties
						if (!DS_REMODConfirmProperty(KeyValue))
						{
							ErrP = "Table values are under property name limitations";
							Fail = true;
							break;
						}
						// Add @ to KeyProperty
						strncat(KeyProperty, "#", BUFSIZE);
						
						// Add value to property
						strncat(KeyProperty, KeyValue, BUFSIZE);
						
						// Create table and set as current table
						CurrentTable = Z_FindSubTable(CurrentTable, KeyProperty, true);
					}
					// Create key
					else if (strcmp(Token, ";") == 0)
					{
						// Add new entry to the current table with said stuff
						Z_TableSetValue(CurrentTable, KeyProperty, KeyValue);
					}
					// Unknown
					else
					{
						ErrP = "Neither table nor key, must be } or ;";
						Fail = true;
						break;
					}
					
					// Reset j
					j = 0;
				}
				
				// Remember last column and row (since it is at parse time)
				lRow = cRow;
				lCol = cCol;
			}
			
			// Failure?
			if (Fail)
			{
				CONS_Printf("RMOD Parse Error at column %u on row %u (%s).\n", lRow, lCol, ErrP);
				return;
			}
			// Debugging
			if (devparm)
				Z_TablePrint(Private->RMODTable, "");
			break;
			
			// Clear single WAD
		case WXBA_CLEARWAD:
			// Delete table
			if (Private->RMODTable)
				Z_TableDestroy(Private->RMODTable);
			Private->RMODTable = NULL;
			break;
			
			// Build WAD composite
		case WXBA_BUILDCOMPOSITE:
			// Linearly read all RMODs for every WAD
			for (Wover = a_WAD; Wover; Wover = WX_RoveWAD(Wover, true, 1))
			{
				// Obtain private data
				if (!WX_GetVirtualPrivateData(Wover, WXDPID_RMOD, &PvPtr, &PvSize))
					continue;
					
				// Check whether it really exists, if not ignore it
				if (!*PvPtr)
					continue;
					
				// Set private data
				Private = (D_WXRMODPrivate_t*) * PvPtr;
				
				// Rove table and send to callback
				Z_TableSuperCallback(Private->RMODTable, DS_RMODTableHandler, Wover);
			}
			break;
			
			// Clear WAD composite
		case WXBA_CLEARCOMPOSITE:
			break;
			
			// Unknown
		default:
			break;
	}
#undef BUFSIZE
}
