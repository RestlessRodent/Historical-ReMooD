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
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2011 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION: All the global variables that store the internal state.
//              Theoretically speaking, the internal state of the engine
//              should be found by looking at the variables collected
//              here, and every relevant module will have to include
//              this header file.
//              In practice, things are a bit messy.

#ifndef __D_STATE__
#define __D_STATE__

// We need globally shared data structures,
//  for defining the global state variables.
#include "doomdata.h"

// We need the player data structure as well.
#include "d_player.h"
#include "d_clisrv.h"

// Game mode handling - identify IWAD version,
//  handle IWAD dependend animations etc.
typedef enum
{
	shareware,					// DOOM 1 shareware, E1, M9
	registered,					// DOOM 1 registered, E3, M27
	commercial,					// DOOM 2 retail, E1 M34
	// DOOM 2 german edition not handled
	retail,						// DOOM 1 retail, E4, M36
	indetermined,				// Well, no IWAD found.
	chexquest1,					// DarkWolf95:July 14, 2003: Chex Quest Support
	
	numgamemodes,
	heretic
} gamemode_t;

// Mission packs - might be useful for TC stuff?
typedef enum
{
	doom,						// DOOM 1
	doom2,						// DOOM 2
	pack_tnt,					// TNT mission pack
	pack_plut,					// Plutonia pack
	pack_chex,					// Chex Quest
	none,
	
	numgamemissions
} gamemission_t;

// Identify language to use, software localization.
typedef enum
{
	english,
	french,
	german,
	unknown
} language_t;

// ===================================================
// Game Mode - identify IWAD as shareware, retail etc.
// ===================================================
//
extern gamemode_t gamemode;
extern gamemission_t gamemission;
extern bool_t inventory;		// true with heretic and hexen

// Set if homebrew PWAD stuff has been added.
extern bool_t modifiedgame;

// =========
// Language.
// =========
//
extern language_t language;

// =============================
// Selected skill type, map etc.
// =============================

// Selected by user.
extern skill_t gameskill;
extern uint8_t gameepisode;
extern uint8_t gamemap;

// Nightmare mode flag, single player.
// extern  bool_t         respawnmonsters;

// GhostlyDeath -- Currently optional new network code
extern bool_t newnet_use;
extern bool_t newnet_solo;

// Netgame? only true in a netgame
extern bool_t netgame;
extern bool_t serverside;
extern bool_t localgame;

// Only true if >1 player. netgame => multiplayer but not (multiplayer=>netgame)
extern bool_t multiplayer;

// Flag: true only if started as net deathmatch.
// An enum might handle altdeath/cooperative better.
extern consvar_t cv_deathmatch;

// ========================================
// Internal parameters for sound rendering.
// ========================================

extern bool_t nomusic;			//defined in d_main.c
extern bool_t nosound;
extern bool_t digmusic;			// SSNTails 12-13-2002

// =========================
// Status flags for refresh.
// =========================
//

// Depending on view size - no status bar?
// Note that there is no way to disable the
//  status bar explicitely.
extern bool_t statusbaractive;

extern bool_t menuactive;		// Menu overlayed?
extern bool_t paused;			// Game Pause?

extern bool_t nodrawers;
extern bool_t noblit;

extern int viewwindowx;
extern int viewwindowy;
extern int viewheight;
extern int viewwidth;
extern int scaledviewwidth;

// This one is related to the 3-screen display mode.
// ANG90 = left side, ANG270 = right
extern int viewangleoffset;

// Player taking events, and displaying.
extern int consoleplayer[MAXSPLITSCREENPLAYERS];
extern int displayplayer[MAXSPLITSCREENPLAYERS];

//added:16-01-98: player from which the statusbar displays the infos.
extern int statusbarplayer;

// ============================================
// Statistics on a given map, for intermission.
// ============================================
//
extern int totalkills;
extern int totalitems;
extern int totalsecret;

// ===========================
// Internal parameters, fixed.
// ===========================
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.

extern tic_t gametic;
extern tic_t g_ProgramTic;

#define localgametic  leveltime

// Player spawn spots.
extern mapthing_t* playerstarts[MAXPLAYERS];

// Intermission stats.
// Parameters for world map / intermission.
extern wbstartstruct_t wminfo;

// LUT of ammunition limits for each kind.
// This doubles with BackPack powerup item.
extern int maxammo[NUMAMMO];

// =====================================
// Internal parameters, used for engine.
// =====================================
//

// File handling stuff.
extern char basedefault[1024];

// if true, load all graphics at level load
extern bool_t precache;

// wipegamestate can be set to -1
//  to force a wipe on the next draw
extern gamestate_t wipegamestate;

//?
// debug flag to cancel adaptiveness
extern bool_t singletics;

#define   BODYQUESIZE     32

extern mobj_t* bodyque[BODYQUESIZE];
extern int bodyqueslot;

// =============
// Netgame stuff
// =============

//extern  ticcmd_t        localcmds[BACKUPTICS];

extern ticcmd_t netcmds[BACKUPTICS][MAXPLAYERS];

extern bool_t novideo;

/*********************
*** EXTENDED STUFF ***
*********************/

/* CoreGame_t -- Game being played... */
typedef enum CoreGame_e
{
	COREGAME_DOOM,							// Doom is being played
	COREGAME_HERETIC,						// Heretic is being played
	COREGAME_HEXEN,							// Hexen is being played
	COREGAME_STRIFE,						// Strife is being played
	
	NUMCOREGAMES
} CoreGame_t;

extern CoreGame_t g_CoreGame;				// Core game mode

#endif							//__D_STATE__

