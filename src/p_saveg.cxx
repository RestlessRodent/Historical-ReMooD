// -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
//         :oCCCCOCoc.
//     .cCO8OOOOOOOOO8Oo:
//   .oOO8OOOOOOOOOOOOOOOCc
//  cO8888:         .:oOOOOC.                                                TM
// :888888:   :CCCc   .oOOOOC.     ###      ###                    #########
// C888888:   .ooo:   .C########   #####  #####  ######    ######  ##########
// O888888:         .oO###    ###  #####  ##### ########  ######## ####    ###
// C888888:   :8O.   .C##########  ### #### ### ##    ##  ##    ## ####    ###
// :8@@@@8:   :888c   o###         ### #### ### ########  ######## ##########
//  :8@@@@C   C@@@@   oo########   ###  ##  ###  ######    ######  #########
//    cO@@@@@@@@@@@@@@@@@Oc0
//      :oO8@@@@@@@@@@Oo.
//         .oCOOOOOCc.                                      http://remood.org/
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION:
//      Archiving: SaveGame I/O.

/***************
*** INCLUDES ***
***************/

#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "r_state.h"
#include "z_zone.h"
#include "w_wad.h"
#include "p_setup.h"
#include "t_vari.h"
#include "t_script.h"
#include "t_func.h"
#include "m_random.h"
#include "m_misc.h"
#include "p_saveg.h"
#include "console.h"
#include "p_demcmp.h"
#include "m_menu.h"

/****************
*** FUNCTIONS ***
****************/

/* CLC_SaveGame() -- Saves the game */
static CONL_ExitCode_t CLC_SaveGame(const uint32_t a_ArgC, const char** const a_ArgV)
{
	/* Check */
	if (a_ArgC < 2)
	{
		CONL_OutputF("Usage: %s \"<filename>\"\n", a_ArgV[0]);
		return CLE_INVALIDARGUMENT;
	}
	
	/* Save the game */
	if (strcasecmp(a_ArgV[0], "save") == 0)
	{
		if (P_SaveGameEx(NULL, a_ArgV[1], strlen(a_ArgV[1]), NULL, NULL))
			return CLE_SUCCESS;
	}
	
	/* Load the game */
	else if (strcasecmp(a_ArgV[0], "load") == 0)
	{
		if (P_LoadGameEx(NULL, a_ArgV[1], strlen(a_ArgV[1]), NULL, NULL))
			return CLE_SUCCESS;
	}
	
	/* Return success always */
	return CLE_FAILURE;
}

/* P_InitSGConsole() -- Initialize save command */
void P_InitSGConsole(void)
{
	/* Add command */
	CONL_AddCommand("save", CLC_SaveGame);
	CONL_AddCommand("load", CLC_SaveGame);
}

#define VERSIONSIZE 16
uint8_t* save_p = NULL;			// Pointer to the data

/*** REAL STUFF ***/
uint8_t* SaveBlock = NULL;
uint8_t* SaveStart = NULL;
size_t SaveLimit = 0;

/*** PROTOTYPES ***/
void P_SAVE_WadState(void);
void P_SAVE_Console(void);
void P_SAVE_LevelState(void);

void P_SAVE_Players(void);

void P_SAVE_MapObjects(void);

/*** SAVING AND LOADING ***/

/* P_CheckSizeEx() -- Resize buffer */
bool P_CheckSizeEx(size_t Need)
{
	return true;
}

/* P_SaveGameEx() -- Extended savegame */
bool P_SaveGameEx(const char* SaveName, const char* ExtFileName, size_t ExtFileNameLen, size_t* SaveLen, uint8_t** Origin)
{
	return false;
}

/* P_LoadGameEx() -- Load an extended save game */
bool P_LoadGameEx(const char* FileName, const char* ExtFileName, size_t ExtFileNameLen, size_t* SaveLen, uint8_t** Origin)
{
	return false;
}

