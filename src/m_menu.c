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
// DESCRIPTION: XML Menu Code

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "doomdef.h"
#include "m_menu.h"
#include "dstrings.h"

/********************
*** GUI FUNCTIONS ***
********************/

/*********************
*** MENU FUNCTIONS ***
*********************/

/* M_SpawnMenu() -- Opens an existing menu */
void M_SpawnMenu(const char* const Name)
{
}

/* M_ActiveMenu() -- Returns the name of the current active menu */
const char* M_ActiveMenu(void)
{
	return NULL;
}

/* M_StartMessage() -- Starts a single message */
void M_StartMessage(const char* const a_Str, void* A_Unk, const MessageMode_t a_Mode)
{
}

/*****************************************************************************/

/* M_Responder() -- Responds to events passed from below */
boolean M_Responder(event_t* const Event)
{
	return false;
}

/* M_Ticker() -- Ticks the XML menu system */
void M_Ticker(void)
{
}

/* M_Drawer() -- Draws the menu */
void M_Drawer(void)
{
}

/*****************************************************************************/


/* M_WX_BuildXMLBack() -- XML Callback, handle menu stuff */
static boolean M_WX_BuildXMLBack(void* const a_Data, const char* const a_Key, const char* const a_Value)
{
	return true;
}

/* M_WX_Build() -- Loads menu from WAD */
void M_WX_Build(WX_WADFile_t* const a_WAD)
{
	XMLData_t* XML;
	WX_WADEntry_t* Entry;
	
	/* Check */
	if (!a_WAD)
		return;
	
	/* Find entry and parse XML */
	Entry = WX_EntryForName(a_WAD, "RMD_XDAT", false);
	
	// Exists?
	if (!Entry)
	{
		if (devparm)
			CONS_Printf("M_WX_Build: RMD_XDAT missing!\n");
		return;
	}
	
	XML = DS_StartXML(WX_CacheEntry(Entry, WXCT_RAW, WXCT_RAW), WX_GetEntrySize(Entry));
	
	// Got XML?
	if (!XML)
		return;
	
	if (devparm)
		CONS_Printf("M_WX_Build: Starting to parse XML...\n");
		
	/* While we are parsing the XML... */
	DS_ParseXML(XML, NULL, M_WX_BuildXMLBack);
	
	if (devparm)
		CONS_Printf("M_WX_Build: Done!\n");
}

/* M_WX_ClearBuild() -- Clears menu from WAD */
void M_WX_ClearBuild(WX_WADFile_t* const a_WAD)
{
	/* Check */
	if (!a_WAD)
		return;
}

/* M_WX_Composite() -- Merges all menus together */
void M_WX_Composite(WX_WADFile_t* const a_WAD)
{
	/* Check */
	if (!a_WAD)
		return;
}

/* M_WX_ClearComposite() -- Clears merged menus */
void M_WX_ClearComposite(void)
{
}

