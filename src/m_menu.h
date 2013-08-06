// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2013 GhostlyDeath <ghostlydeath@remood.org>
//                                      <ghostlydeath@gmail.com>
// ----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// ----------------------------------------------------------------------------
// DESCRIPTION: Menu widget stuff, episode selection and such.

#ifndef __M_MENU__
#define __M_MENU__

#include "doomtype.h"
#include "i_util.h"

//#include "d_prof.h"

//#include "w_wad.h"
//#include "z_zone.h"

//#include "d_rmod.h"

//#include "i_util.h"

/*******************
*** SIMPLE MENUS ***
*******************/

/*** CONSTANTS ***/

/* M_SMMenus_t -- Possible Menus */
typedef enum M_SMMenus_e
{
	MSM_MAINDOOM,								// Main menu (Doom)
	MSM_NEWGAME,								// New Game
	MSM_SKILLSELECTDOOM,						// Select Skill (Doom)
	MSM_EPISELECTDOOM,							// Select Episode (Doom)
	MSM_EPISELECTUDOOM,							// Select Episode (Ult Doom)
	MSM_ADVANCEDCREATEGAME,						// Advanced Game Creation
	MSM_QUITGAME,								// Quit Game
	MSM_JOINUNLISTEDSERVER,						// Connect to Unlisted Server
	MSM_OPTIONS,								// Options Menu
	MSM_MAINHERETIC,							// Main Menu (Heretic)
	MSM_MAIN,									// Main Menu alias
	MSM_PROFMAN,								// Profile Manager
	MSM_PROFMOD,								// Modify Profile
	
	NUMMSMMENUS
} M_SMMenus_t;

/*** FUNCTIONS ***/

void M_SMInit(void);
bool_t M_SMHandleEvent(const I_EventEx_t* const a_Event);
bool_t M_SMDoGrab(void);
bool_t M_SMGenSynth(const int32_t a_ScreenID);
bool_t M_SMFreezeGame(void);
bool_t M_SMMenuVisible(void);
bool_t M_SMPlayerMenuVisible(const int32_t a_ScreenID);
void M_SMDrawer(void);
void M_SMTicker(void);
void* M_SMSpawn(const int32_t a_ScreenID, const M_SMMenus_t a_MenuID);

#endif

