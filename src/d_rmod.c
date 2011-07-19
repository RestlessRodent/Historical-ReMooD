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
// DESCRIPTION: Global RMOD Parsing

/***************
*** INCLUDES ***
***************/

#include "doomdef.h"
#include "d_rmod.h"
#include "m_menu.h"
#include "console.h"
#include "z_zone.h"

/*****************
*** STRUCTURES ***
*****************/

/* D_WXRMODPrivate_t -- RMOD Private Data */ 
typedef struct D_WXRMODPrivate_s
{
	Z_Table_t* RMODTable;								// Table to store RMOD data
} D_WXRMODPrivate_t;

/****************
*** FUNCTIONS ***
****************/

/* D_WX_RMODMultiBuild() -- RMOD Multi builder */
void D_WX_RMODMultiBuild(WX_WADFile_t* const a_WAD, const WX_BuildAction_t a_Action)
{
#define BUFSIZE 512
	D_WXRMODPrivate_t* Private;
	WX_WADEntry_t* Entry;
	char* Data;
	void** PvPtr;
	size_t* PvSize;
	
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
		Private = (D_WXRMODPrivate_t*)*PvPtr;
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
			Data = WX_CacheEntry(Entry, WXCT_RAW, WXCT_RAW);
			if (!Data)
				return;
			
			// Create the root table
			Private->RMODTable = Z_TableCreate("ReMooD");
			
			// Debugging
			if (devparm)
				Z_TablePrint(Private->RMODTable, "|");
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

