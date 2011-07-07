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

#include "m_menu.h"

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

/* M_WX_Build() -- Loads menu from WAD */
void M_WX_Build(WX_WADFile_t* const a_WAD)
{
	/* Check */
	if (!a_WAD)
		return;
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

