// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
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
// DESCRIPTION: Doom/Hexen game states

#ifndef __G_STATE__
#define __G_STATE__

#include "doomtype.h"

// skill levels
typedef enum
{
	sk_baby,
	sk_easy,
	sk_medium,
	sk_hard,
	sk_nightmare
} skill_t;

// the current state of the game
typedef enum
{
	GS_NULL = 0,				// at begin
	GS_LEVEL,					// we are playing
	GS_INTERMISSION,			// gazing at the intermission screen
	GS_FINALE,					// game final animation
	GS_DEMOSCREEN,				// looking at a demo
	//legacy
	GS_DEDICATEDSERVER,			// added 27-4-98 : new state for dedicated server
	GS_WAITINGPLAYERS,			// added 3-9-98 : waiting player in net game
	
	// GhostlyDeath <May 17, 2012> -- Waiting for join window
	GS_WAITFORJOINWINDOW,						// Player must wait to join
} gamestate_t;

typedef enum
{
	ga_nothing,
	ga_completed,
	ga_worlddone,
	//HeXen
	
	/*
	   ga_initnew,
	   ga_newgame,
	   ga_loadgame,
	   ga_savegame,
	   ga_leavemap,
	   ga_singlereborn
	 */
} gameaction_t;

extern gamestate_t gamestate;
extern gameaction_t gameaction;
extern skill_t gameskill;

extern bool_t demoplayback;

#endif							//__G_STATE__
