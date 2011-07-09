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
*** GUI CONSTANTS ***
********************/

/*********************
*** MENU CONSTANTS ***
*********************/

/* M_MenuItemType_t -- Type of menu item */
typedef enum M_MenuItemType_e
{
	MMIT_NOTHING,								// Nothing important
	MMIT_FUNCTION,								// Function
	MMIT_SUBMENU,								// Another menu
	MMIT_CVAR,									// Console variable
	
	NUMMENUITEMTYPES
} M_MenuItemType_t;

/* M_MenuFlags_t -- Flags for the menu */
typedef enum M_MenuFlags_e
{
	MMF_AUTOADJUST	= 0x00000001,				// Menu is autoadjusted
} M_MenuFlags_t;

/**********************
*** MENU STRUCTURES ***
**********************/

/* M_MenuItem_t -- A menu item */
typedef struct M_MenuItem_s
{
	M_MenuItemType_t Type;						// Type of menu item
} M_MenuItem_t;

/* M_MenuDef_t -- Menu definition */
typedef struct M_MenuDef_s
{
	char* MenuID;								// ID of the menu
	char* TitleStrID;							// String ID of the title to show
	char* TitlePic;								// Title picture name
	char** TitleStrRef;							// Pointer to pointer of UTF-8 string
	uint32_t Flags;								// Flags for menu
	M_MenuItem_t* Items;						// Menu items
	size_t NumItems;							// Number of items
} M_MenuDef_t;

/* M_WADPrivateJunk_t -- Private stuff for said WAD */
typedef struct M_WADPrivateJunk_s
{
	M_MenuDef_t* Menus;							// Available Menus
	size_t NumMenus;							// Number of menus
	
	M_MenuDef_t* EditMenu;						// Menu currently being edited
} M_WADPrivateJunk_t;

/*************
*** LOCALS ***
*************/

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

/* M_XMLDataParse() -- Parse XML data */
void M_XMLDataParse(D_XMLPassedData_t* const a_PassDat)
{
	M_WADPrivateJunk_t** JunkPtrPtr;
	M_WADPrivateJunk_t* JunkPtr;
	const char* Base;
	
	/* Check */
	if (!a_PassDat)
		return;
		
	/* Get junk */
	JunkPtrPtr = a_PassDat->PrivateJunk;
	JunkPtr = *JunkPtrPtr;
	
	/* If a standard build is being done */
	if (a_PassDat->Action == DXMLBA_BUILD)
	{
		// If junk is not allocated
		JunkPtrPtr = a_PassDat->PrivateJunk;
	
		if (!*JunkPtrPtr)
		{
			*a_PassDat->PrivateSize = sizeof(M_WADPrivateJunk_t)
			*JunkPtrPtr = Z_Malloc(*a_PassDat->PrivateSize, PU_STATIC, NULL);
			JunkPtr = *JunkPtrPtr;
		}
	
		// If we closed the menu tag do some cleanup
		if (a_PassDat->CheckRetVal == -1)
		{
			JunkPtr->EditMenu = NULL;
			
			if (devparm)
				CONS_Printf("M_XMLDataParse: Finalize menus.\n");
			return;
		}
	
		// If we opened the menu, prepare to modify it
		else if (a_PassDat->CheckRetVal == 2)
		{
			if (devparm)
				CONS_Printf("M_XMLDataParse: Begin menus.\n");
			return;
		}
		
		// A submenu is being defined
		Base = a_PassDat->Key;
		if ((XMLPass->CheckRetVal = D_XMLCheckKey(Base, "SubMenuMenu", &Base)) != 0)
		{
			switch (XMLPass->CheckRetVal)
			{
					// SubMenu being closed
				case -1:
					if (devparm)
						CONS_Printf("M_XMLDataParse: Closed submenu\n");
					
					JunkPtr->EditMenu = NULL;
					break;
				
					// SubMenu being edited
				case 1:
					
					break;
				
					// SubMenu being opened
				case 2:
					if (devparm)
						CONS_Printf("M_XMLDataParse: Opened submenu.\n");
						
					// Resize the menu list
					Z_ResizeArray(&JunkPtr->Menus, sizeof(M_MenuDef_t), JunkPtr->NumMenus, JunkPtr->NumMenus + 1);
					
					JunkPtr->EditMenu = &JunkPtr->Menus[JunkPtr->NumMenus++];
					break;
				
					// unhandled
				default:
					break;
			}
		}
	
		// Otherwise, we recreate a new menu now
		CONS_Printf("%s: %s\n", a_PassDat->Key, a_PassDat->Value);
	}
	
	/* Clearing built data in a WAD */
	else if (a_PassDat->Action == DXMLBA_CLEARBUILD)
	{
	}
	
	/* Building a composite */
	else if (a_PassDat->Action == DXMLBA_COMPOSITE)
	{
	}
	
	/* Clearing a composite */
	else if (a_PassDat->Action == DXMLBA_CLEARCOMPOSITE)
	{
	}
}

