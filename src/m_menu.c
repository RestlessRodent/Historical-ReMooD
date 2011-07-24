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
	char* MenuTitle;							// Menu title
	char* TitleStrID;							// String ID of the title to show
	char* TitlePic;								// Title picture name
	char** TitleStrRef;							// Pointer to pointer of UTF-8 string
	uint32_t Flags;								// Flags for menu
	M_MenuItem_t* Items;						// Menu items
	size_t NumItems;							// Number of items
} M_MenuDef_t;

/*************
*** LOCALS ***
*************/

static M_MenuDef_t** l_Menus = NULL;			// All menus available
static size_t l_NumMenus = 0;					// Number of menus

/********************
*** GUI FUNCTIONS ***
********************/

/*********************
*** MENU FUNCTIONS ***
*********************/

/* M_PushOrFindMenu() -- Adds a new menu to the list or finds a menu and returns that */
static M_MenuDef_t* M_PushOrFindMenu(const char* const a_Name, const boolean a_Create)
{
	size_t i;
	
	/* Check */
	if (!a_Name)
		return NULL;
	
	/* Go through every menu */
	for (i = 0; i < l_NumMenus; i++)
		if (l_Menus[i])
			if (strcasecmp(l_Menus[i]->MenuID, a_Name) == 0)
				return l_Menus[i];
	
	/* Not found so create (if wanted) */
	if (!a_Create)
		return NULL;
	
	// Resize the array
	i = l_NumMenus++;
	Z_ResizeArray(&l_Menus, sizeof(*l_Menus), i, l_NumMenus);
	
	// Create menu here
	l_Menus[i] = Z_Malloc(sizeof(M_MenuDef_t), PU_STATIC, NULL);
	
	// Set menu stuff
	l_Menus[i]->MenuID = Z_StrDup(a_Name, PU_STATIC, NULL);
	
	return l_Menus[i];
}

/* M_LoadMenuTable() -- Loads a menu from a table */
boolean M_LoadMenuTable(Z_Table_t* const a_Table, const char* const a_ID, void* const a_Data)
{
#define BUFSIZE 512
	char Temp[BUFSIZE];
	M_MenuDef_t* NewMenu;
	const char* p;
	
	/* Check */
	if (!a_Table || !a_ID)
		return false;
	
	/* Debug */
	if (devparm)
		CONS_Printf("RMOD/Menu: Handling \"%s\".\n", a_ID);
	
	/* Obtain the root name */
	NewMenu = M_PushOrFindMenu(a_ID, true);
	
	/* Obtain stuff from the menu */
	// Title
	if ((p = Z_TableGetValue(a_Table, "title")))
		NewMenu->MenuTitle = Z_StrDup(p, PU_STATIC, NULL);
	
	// String ID
	if ((p = Z_TableGetValue(a_Table, "string")))
	{
		// Set
		NewMenu->TitleStrID = Z_StrDup(p, PU_STATIC, NULL);
		
		// Find reference
		NewMenu->TitleStrRef = DS_FindString(NewMenu->TitleStrID);
	}
	
	/* Success! */
	return true;
#undef BUFSIZE
}

/* M_DestroyMenu() -- Destroys a menu */
void M_DestroyMenu(M_MenuDef_t* const a_Menu)
{
	/* Check */
	if (!a_Menu)
		return;
	
	/* Clear things away */
	if (a_Menu->MenuTitle)
		Z_Free(a_Menu->MenuTitle);
	if (a_Menu->TitleStrID)
		Z_Free(a_Menu->TitleStrID);
}

/*****************************************************************************/

/* M_SpawnMenu() -- Opens an existing menu */
void M_SpawnMenu(const char* const Name, const size_t a_PlayerID)
{
}

/* M_ActiveMenu() -- Returns the name of the current active menu */
const char* M_ActiveMenu(const size_t a_PlayerID)
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

