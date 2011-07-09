// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.org/
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
// DESCRIPTION: Global XML Parsing

#ifndef __D_XML_H__
#define __D_XML_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "dstrings.h"
#include "w_wad.h"

/****************
*** CONSTANTS ***
****************/

#define MAXXMLHANDLERKEYSIZE				32		// buffer size of key handler key

/* D_XMLBuildAction_t -- Type of action being performed */
typedef enum D_XMLBuildAction_e
{
	DXMLBA_BUILD,									// Building for a WAD
	DXMLBA_CLEARBUILD,								// Cleaning out a WAD
	DXMLBA_COMPOSITE,								// Compositing virtual WADs
	DXMLBA_CLEARCOMPOSITE,							// Clear virtual composite
	
	MAXXMLBUILDACTIONS
} D_XMLBuildAction_t;

/* D_XMLKeyJunk_t -- Type of key junk */
typedef enum D_XMLKeyJunk_e
{
	DXMLKJ_IDENTITY,								// <Identity> -- XML Info
	DXMLKJ_MENU,									// <Menu> -- Menu definitions
	DXMLKJ_WADINDEX,								// <WadIndex> -- WAD Information (seeking)

	NUMXMLKEYJUNKS
} D_XMLKeyJunk_t;

/* D_XMLHandleMethod_t -- How to actually handle something */
typedef enum D_XMLHandleMethod_e
{
	DXMLHM_UNHANDLED,								// Ignore completely
	DXMLHM_SUBKEYS,									// Contains subkeys
	DXMLHM_STRDUP,									// Duplicate String
	DXMLHM_TOINT32,									// Convert value to int
	DXMLHM_TOFIXED,									// Convert value to fixed
	DXMLHM_CUSTOM,									// Custom handler
	DXMLHM_FLAGS,									// Flag handler
	
	NUMXMLHANDLEMETHODS
} D_XMLHandleMethod_t;

/*****************
*** STRUCTURES ***
*****************/

/* D_XMLHandler_t -- Handles an XML thing */
typedef struct D_XMLHandler_s
{
	const char KeyDef[MAXXMLHANDLERKEYSIZE];		// Actual Key to handle
	D_XMLHandleMethod_t Method;						// How do we handle this?
	void* Data;										// Data attached to method
	size_t Size;									// Size attached to method
} D_XMLHandler_t;

/* D_XMLEntry_t -- Entry in an XML table */
typedef struct D_XMLEntry_s
{
	char* Key;										// XML Key
	boolean HasTable;								// Is a table and not a value
	
	union
	{
		char* Value;								// Plain key value
		struct D_XMLEntry_s* SubTable;				// Subtable
	} Data;											// Entry data
	
	struct D_XMLEntry_s* Prev;						// Previous in chain
	struct D_XMLEntry_s* Next;						// Next in chain
} D_XMLEntry_t;

/* D_XMLPassedData_t -- Data to pass to an XML handler */
typedef struct D_XMLPassedData_s
{
	D_XMLBuildAction_t Action;						// What is happening?
	D_XMLKeyJunk_t KeyJunk;							// What is this?
	WX_WADFile_t* WAD;								// WAD File being checked
	void** PrivateJunk;								// Private Data
	size_t PrivateSize;								// Size of Private Data
	const char* Key;								// Passed Key
	const char* Value;								// Passed Value
	int CheckRetVal;								// Return value of checker
	
	D_XMLEntry_t** Table;							// XML Table
	size_t* TableSize;								// Size of table
} D_XMLPassedData_t;

/*****************
*** PROTOTYPES ***
*****************/

void D_WX_XMLBuild(WX_WADFile_t* const a_WAD);
void D_WX_XMLClearBuild(WX_WADFile_t* const a_WAD);
void D_WX_XMLComposite(WX_WADFile_t* const a_WAD);
void D_WX_XMLClearComposite(void);

#endif /* __D_XML_H__ */

