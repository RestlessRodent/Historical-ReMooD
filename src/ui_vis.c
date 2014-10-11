// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: User Interface Visibility

/* NOT IN DEDICATED SERVER */
#if !defined(__REMOOD_DEDICATED)
/***************************/

/***************
*** INCLUDES ***
***************/

#include "ui.h"

/*****************
*** STRUCTURES ***
*****************/

/****************
*** FUNCTIONS ***
****************/

/* UI_Visible() -- Any user interface is visible */
bool_t UI_Visible(void)
{
	return false;
}

/* UI_GrabMouse() -- Grabs the mouse? */
bool_t UI_GrabMouse(void)
{
	return false;
}

/* UI_ShouldFreezeGame() -- Should the game freeze? */
bool_t UI_ShouldFreezeGame(void)
{
	return false;
}

/* UI_Ticker() -- User interface ticker */
void UI_Ticker(void)
{
}

/* NOT IN DEDICATED SERVER */
#endif
/***************************/


