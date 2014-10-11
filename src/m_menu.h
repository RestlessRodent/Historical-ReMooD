// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Menu widget stuff, episode selection and such.

#ifndef __M_MENU__
#define __M_MENU__

#include "doomtype.h"
#include "i_util.h"

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

