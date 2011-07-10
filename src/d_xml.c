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

/***************
*** INCLUDES ***
***************/

#include "doomdef.h"
#include "d_xml.h"
#include "m_menu.h"
#include "console.h"
#include "z_zone.h"

/*****************
*** STRUCTURES ***
*****************/

/* D_XMLHidden_t -- Contained within WAD */
typedef struct D_XMLHidden_s
{
	Z_Table_t* XMLTable;								// XML Data table
} D_XMLHidden_t;

/****************
*** FUNCTIONS ***
****************/

/* D_WX_XMLBuildXMLBack() -- XML Callback */
static boolean D_WX_XMLBuildXMLBack(void* const a_Data, const char* const a_Key, const char* const a_Value)
{
#define BUFSIZE 512
	D_XMLPassedData_t* XMLPass = a_Data;
	const char* Base = a_Key;
	const char* Used;
	char Temp[BUFSIZE];
	boolean Missing;
	D_XMLEntry_t* Rover, *OldRover;
	Z_Table_t* CurTable;
	
	/* Check */
	if (!a_Data || !a_Key || !a_Value)
		return false;
	
	/* Link of current table */
	CurTable = XMLPass->Table;
	
	/* Traverse key */
	Base = a_Key;
	while (Base[0])
	{
		// > means a traversal
		if (Base[0] == '>')
		{
			// Ignore >
			Base += 1;
			
			// Create temporary
			memset(Temp, 0, sizeof(Temp));
			snprintf(Temp, BUFSIZE, "%s", Base, XMLPass->j++);
			
			// Create table with tempkey
			CurTable = Z_FindSubTable(CurTable, Temp, true);
		}
		
		// ? means new table
		else if (Base[0] == '?')
		{
		}
		
		// ! means data
		else if (Base[0] == '!')
		{
			
		}
	
	
		// If value contains ? at the start, it is another fresh table
		if (Base[0] == '?')
		{
			// Ignore this, don't care
			Base += 1;
			//Z_FindSubTable(CurTable, Base, true);
			break;
		}
		
		// ! means data
		else if (Base[0] == '!')
		{
			Base += 1;
			break;
		}
		
		// Closing
		else if (Base[0] == '/')
		{
			// Ignore this, don't care
			break;
		}
		
		// Otherwise, we keep traversing
		else
		{
			Base += strlen(Base) + 1;
			
			if (Base[0] == '?' || Base[0] == '!' || Base[0] == '/')
				Base += 1;
			CurTable = Z_FindSubTable(CurTable, Base, true);	// choose new table
		}
	}
	
	return true;
#undef BUFSIZE
}


/* D_WX_XMLBuild() -- Build per wad XML data */
void D_WX_XMLBuild(WX_WADFile_t* const a_WAD)
{
	XMLData_t* XML;
	WX_WADEntry_t* Entry;
	D_XMLPassedData_t XMLPass;
	D_XMLHidden_t* Hidden;
	
	/* Check */
	if (!a_WAD)
		return;
	
	/* Find entry and parse XML */
	Entry = WX_EntryForName(a_WAD, "RMD_XDAT", false);
	
	// Exists?
	if (!Entry)
	{
		if (devparm)
			CONS_Printf("D_WX_XMLBuild: RMD_XDAT missing!\n");
		return;
	}
	
	XML = DS_StartXML(WX_CacheEntry(Entry, WXCT_RAW, WXCT_RAW), WX_GetEntrySize(Entry));
	
	// Got XML?
	if (!XML)
		return;
	
	if (devparm)
		CONS_Printf("D_WX_XMLBuild: Starting to parse XML...\n");
		
	/* While we are parsing the XML... */
	// Clear pass and set a few things
	memset(&XMLPass, 0, sizeof(XMLPass));
	XMLPass.Action = DXMLBA_BUILD;
	XMLPass.WAD = a_WAD;
	
	// Create private data
	WX_GetVirtualPrivateData(XMLPass.WAD, WXDPID_XML, &XMLPass.PrivateJunk, &XMLPass.PrivateSize);
	
	// Attempt retrieval of hidden stuff
	Hidden = *XMLPass.PrivateJunk;
	
	// Does not exist
	if (!Hidden)
	{
		*XMLPass.PrivateSize = sizeof(*Hidden);
		Hidden = *XMLPass.PrivateJunk = Z_Malloc(*XMLPass.PrivateSize, PU_STATIC, NULL);
		Hidden->XMLTable = Z_TableCreate("ReMooD");
	}
	
	// Clone table here
	XMLPass.Table = Hidden->XMLTable;
	
	// Send
	DS_ParseXML(XML, &XMLPass, D_WX_XMLBuildXMLBack);
	
	// Print table when debugging
	if (devparm)
		Z_TablePrint(Hidden->XMLTable, ">");
	
	if (devparm)
		CONS_Printf("D_WX_XMLBuild: Done!\n");
}

/* D_WX_XMLClearBuild() -- Clear per wad XML Data */
void D_WX_XMLClearBuild(WX_WADFile_t* const a_WAD)
{
	/* Check */
	if (!a_WAD)
		return;
}

/* D_WX_XMLComposite() -- Compile XML Composite */
void D_WX_XMLComposite(WX_WADFile_t* const a_WAD)
{
	/* Check */
	if (!a_WAD)
		return;
}

/* D_WX_XMLClearComposite() -- Clear XML Composite */
void D_WX_XMLClearComposite(void)
{
}

