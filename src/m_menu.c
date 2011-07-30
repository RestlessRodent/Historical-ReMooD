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
#include "v_video.h"
#include "screen.h"
#include "d_netcmd.h"
#include "d_event.h"
#include "g_input.h"

/********************
*** GUI CONSTANTS ***
********************/

/*********************
*** MENU CONSTANTS ***
*********************/

#define MENUSTACKMULTIPLE		8				// Menu stack multiple

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
	MMF_NOPAUSEGAME	= 0x00000002,				// Do not pause the game even when playing solo
	MNF_UNIQUE		= 0x00000004,				// Menu is unique (only one can be open at once)
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

/* M_MenuLoadedData_t -- Menu loaded data */
typedef struct M_MenuLoadedData_s
{
	const M_MenuDef_t* Template;				// Template for loaded menu
	size_t CurrentItem;							// Current Item being selected
	boolean PauseProp;							// Pause game propogation
} M_MenuLoadedData_t;

/*************
*** LOCALS ***
*************/

static M_MenuDef_t** l_Menus = NULL;			// All menus available
static size_t l_NumMenus = 0;					// Number of menus

static M_MenuLoadedData_t** l_MenuStack[MAXSPLITSCREENPLAYERS + 1];
static size_t l_MenuStackSize[MAXSPLITSCREENPLAYERS + 1];
static size_t l_MenuOpenCount[MAXSPLITSCREENPLAYERS + 1];

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

/* MS_ProcessMenuItem() -- Process a menu item */
boolean MS_ProcessMenuItem(Z_Table_t* const a_Sub, void* const a_Data)
{
	M_MenuDef_t* CurrentMenu;
	
	/* Check */
	if (!a_Sub || !a_Data)
		return false;
	
	/* Data is the menu */
	CurrentMenu = a_Data;
	
	if (devparm)
		CONS_Printf("RMOD/Menu: Subtable \"%s\"\n", Z_TableName(a_Sub));
	
	/* Success */
	return true;
}

/* M_LoadMenuTable() -- Loads a menu from a table */
boolean M_LoadMenuTable(Z_Table_t* const a_Table, const char* const a_ID, void* const a_Data)
{
#define BUFSIZE 512
	char Temp[BUFSIZE];
	M_MenuDef_t* NewMenu;
	const char* p;
	const char* Flags;
	
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
	
	// Flags
	if ((p = Z_TableGetValue(a_Table, "flags")))
	{
	}
	
	/* Process menu items */
	Z_TableSuperCallback(a_Table, MS_ProcessMenuItem, NewMenu);
	
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
	M_MenuDef_t* Template;
	M_MenuLoadedData_t* Loader;
	size_t p, i;
	
	/* Check */
	if (!Name || a_PlayerID > MAXSPLITSCREENPLAYERS + 1)
		return;
	
	/* Find menu */
	Template = M_PushOrFindMenu(Name, false);
	
	// not found?
	if (!Template)
		return;
	
	/* If it was, resize the menu stack */
	// Simple
	p = a_PlayerID;
	
	// Resize the stack if needed
	i = l_MenuOpenCount[p] + 1;
	
	if (l_MenuOpenCount[p] + 1 > l_MenuStackSize[p])
	{
		Z_ResizeArray(&l_MenuStack[p], sizeof(*l_MenuStack[p]), l_MenuStackSize[p], l_MenuStackSize[p] + MENUSTACKMULTIPLE);
		l_MenuStackSize[p] += MENUSTACKMULTIPLE;
	}
	
	// Place at end
	i = l_MenuOpenCount[p]++;
	
	/* Create loader */
	Loader = Z_Malloc(sizeof(*Loader), PU_STATIC, NULL);
	l_MenuStack[p][i] = Loader;
	
	/* Fill Loader */
	Loader->Template = Template;
}

/* M_PopMenu() -- Pops the menu off the stack */
void M_PopMenu(const size_t a_PlayerID)
{
}

/* M_ActiveMenu() -- Returns the name of the current active menu */
const char* M_ActiveMenu(const size_t a_PlayerID)
{
	/* Check */
	if (a_PlayerID > MAXSPLITSCREENPLAYERS + 1)
		return NULL;
	
	/* Get submenu */
	// Is there actually a menu open?
	if (l_MenuOpenCount[a_PlayerID])
		return l_MenuStack[a_PlayerID][l_MenuOpenCount[a_PlayerID] - 1]->Template->MenuID;
	return NULL;
}

/* M_StartMessage() -- Starts a single message */
void M_StartMessage(const char* const a_Str, void* A_Unk, const MessageMode_t a_Mode)
{
}

/*****************************************************************************/

/* M_Responder() -- Responds to events passed from below */
boolean M_Responder(event_t* const a_Event)
{
	/* Check */
	if (!a_Event)
		return false;
	
	/* Which event type? */
	switch (a_Event->type)
	{
			// Key is pressed
		case ev_keydown:
			if (a_Event->data1 == KEY_ESCAPE)
			{
				// If a menu is open, go back
				if (M_ActiveMenu(0))
					M_PopMenu(0);
				
				// Otherwise open a root menu
				else
					M_SpawnMenu("root", 0);
				
				// Event was eaten
				return true;
			}
			
			// Not eaten
			return false;
		
			// Unhandled
		default:
			return false;
	}
}

/* M_Ticker() -- Ticks the menu system */
void M_Ticker(void)
{
}

/* M_GetSSBox() -- Get splitscreen box location */
void M_GetSSBox(const size_t a_Player, const size_t a_SplitCount, int32_t* const a_x, int32_t* const a_y, int32_t* const a_w, int32_t* const a_h)
{
	/* Check */
	if (!a_SplitCount || a_SplitCount > MAXSPLITSCREENPLAYERS || a_Player >= a_SplitCount || (!a_x && !a_y && !a_w && !a_h))
		return;
	
	/* Number of players in game */
	switch (a_SplitCount)
	{
			// 1 player
		case 1:
		
			// 2 players
		case 2:
		
			// 3 or 4 players
		case 3:
		case 4:
		
			// Unknown
		default:
			if (a_x)
				*a_x = 0;
			if (a_y)
				*a_y = 0;
			if (a_w)
				*a_w = BASEVIDWIDTH;
			if (a_h)
				*a_h = BASEVIDHEIGHT;
			break;
	}
}

/* M_DrawMenu() -- Draws a menu */
static void M_DrawMenu(M_MenuLoadedData_t* const a_Data, const int32_t a_x, const int32_t a_y, const int32_t a_w, const int32_t a_h)
{
	V_DrawFadeScreen();
}

/* M_Drawer() -- Draws the menu */
void M_Drawer(void)
{	
	size_t i = 0;
	boolean Skip = false;
	int32_t x, y, w, h;
	
	/* Render each menu */
	for (i = 0; i <= MAXSPLITSCREENPLAYERS; i++)	// +1
	{
		// Skip rendering player menus?
		if (i >= 1 && Skip)
			return;
		
		// Is there not a menu open?
		if (!l_MenuOpenCount[i])
			continue;
		
		// A menu was open, if this is for everyone, skip everyone else
		if (i == 0)
			Skip = true;
		
		// Get location to draw
		M_GetSSBox((i >= 1 ? i - 1 : 0), cv_splitscreen.value, &x, &y, &w, &h);
		
		// Render the top most menu
		M_DrawMenu(l_MenuStack[i][l_MenuOpenCount[i] - 1], x, y, w, h);
	}
}

/*****************************************************************************/

