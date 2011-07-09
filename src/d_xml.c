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

/* D_XMLCheckKey() -- Checks a key and returns its type */
int D_XMLCheckKey(const char* const a_Key, const char* const a_CheckValue, const char** const a_Next)
{
	const char* Base;
	size_t Len;
	boolean Open, Closed;
	
	/* Check */
	if (!a_Key || !a_CheckValue)
		return 0;
	
	/* Check now */
	// Get length of check
	Len = strlen(a_CheckValue);
	
	if (!Len)
		return 0;
		
	// if the first character in a key is / or ? move the base
	Open = Closed = false;
	switch (a_Key[0])
	{
		case '?':
			Open = true;
		case '/':
			Closed = !Open;
			
			// Boost
			Base = a_Key + 1;
			break;
		
			// Nothing
		default:
			Base = a_Key;
			break;
	}
	
	// Check to see if it matches
	if (strncasecmp(Base, a_CheckValue, Len) != 0)
		return 0;
	
	/* It does match */
	// Check to see which kind of ending this is
	switch (Base[Len])
	{
			// Just this
		case '\0':
			// Tag was opened
			if (Open)
				return 2;
				
			// Tag was closed
			if (Closed)
				return -1;
			break;
		
			// Key is followed by something else
		case '<':
		default:
			if (a_Next)
				*a_Next = &Base[Len + 1];
			return 1;
	}
	
	/* Shouldn't really reach here */
	return 0;
}

/* D_WX_XMLBuildXMLBack() -- XML Callback */
static boolean D_WX_XMLBuildXMLBack(void* const a_Data, const char* const a_Key, const char* const a_Value)
{
	D_XMLPassedData_t* XMLPass = a_Data;
	const char* Base = a_Key;
	
	CONS_Printf("D_WX_XMLBuildXMLBack: %s: %s\n", a_Key, a_Value);
	
	/* Check */
	if (!a_Data || !a_Key || !a_Value)
		return false;
	
	/* Set stuff */
	XMLPass->Key = a_Key;
	XMLPass->Value = a_Value;
	
	/* Check to make sure we are in the ReMooD namespace */
	if (D_XMLCheckKey(Base, "ReMooD", &Base) > 0)
	{
		CONS_Printf("-> %s\n", Base);
		
		// Now which subnamespace is this?
			// Identity -- About this XML file
		if ((XMLPass->CheckRetVal = D_XMLCheckKey(Base, "Identity", &Base)) != 0)
		{
			XMLPass->KeyJunk = DXMLKJ_IDENTITY;
			XMLPass->Key = Base;
			
			// Send to parser
		}
			// WadIndex -- IWAD info
		else if ((XMLPass->CheckRetVal = D_XMLCheckKey(Base, "WadIndex", &Base)) != 0)
		{
			XMLPass->KeyJunk = DXMLKJ_WADINDEX;
			XMLPass->Key = Base;
			
			// Send to parser
		}
			// Menu -- Main menu
		else if ((XMLPass->CheckRetVal = D_XMLCheckKey(Base, "Menu", &Base)) != 0)
		{
			XMLPass->KeyJunk = DXMLKJ_MENU;
			XMLPass->Key = Base;
			if (!WX_GetVirtualPrivateData(XMLPass->WAD, WXDPID_MENU, &XMLPass->PrivateJunk, &XMLPass->PrivateSize))
			{
				XMLPass->PrivateJunk = NULL;
				XMLPass->PrivateSize = 0;
			}
			
			// Send to parser
			M_XMLDataParse(XMLPass);
		}
			// Unknown
		else
		{
			// TODO: warn about it (if -devparm)
		}
	}
	
	return true;
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

