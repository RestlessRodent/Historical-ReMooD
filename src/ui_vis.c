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
// Copyright (C) 2013-2013 GhostlyDeath <ghostlydeath@remood.org>
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

void UI_Drawer(void) {}	// Move to UI_DRAW.C

bool_t UI_HandleEvent(I_EventEx_t* const a_Event, const bool_t a_Early) {return false;}	// Move to UI_EVENT.C

/* NOT IN DEDICATED SERVER */
#endif
/***************************/

