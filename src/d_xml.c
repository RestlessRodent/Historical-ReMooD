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
	D_XMLEntry_t** CurrentTable;
	size_t* CurrentSize, i;
	char Temp[BUFSIZE];
	boolean Missing;
	
	CONS_Printf("D_WX_XMLBuildXMLBack: %s: %s\n", a_Key, a_Value);
	
	/* Check */
	if (!a_Data || !a_Key || !a_Value)
		return false;
	
	/* Constantly read key off the key stack */
	for (CurrentTable = XMLPass->Table, CurrentSize = XMLPass->TableSize; Base[0];)
	{
		// Ignore ? and /
		if (Base[0] == '?' || Base[0] == '/')
			Base++;
		
		// Place base in buffer and skip it
		strncpy(Temp, Base, BUFSIZE);
		Base += strlen(Base) + 1;
		
		// Search table for key
		i = 0;
		Missing = true;
		if (*CurrentTable)
			for (i = 0; i < *CurrentSize; i++)
				if (strcasecmp((*CurrentTable)[i].Key, Temp) == 0)
				{
					// Found in table
					Missing = false;	// No longer missing
					
					// Select table and use that instead (traverse)
					if ((*CurrentTable)[i].HasTable)
					{
						CurrentTable = &((*CurrentTable)[i].Data.Index.Table);
						CurrentSize = &((*CurrentTable)[i].Data.Index.Size);
						i = 0;
					}
					
					// Break out always
					break;
				}
		
		// Table is missing (resize table to add it)
		if (Missing)
		{
			i = 0;
			// Resize
			Z_ResizeArray(CurrentTable, sizeof(D_XMLEntry_t), *CurrentSize, *CurrentSize + 1);
			*CurrentSize += 1;
			
			// Set key
			(*CurrentTable)[i].Key = Z_StrDup(Temp, PU_STATIC, NULL);
			
			continue;
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
	WX_GetVirtualPrivateData(XMLPass.WAD, WXDPID_XML, &XMLPass.Table, &XMLPass.TableSize);
	
	// Send
	DS_ParseXML(XML, &XMLPass, D_WX_XMLBuildXMLBack);
	
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

