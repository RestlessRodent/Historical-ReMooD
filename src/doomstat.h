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
//#include "doomdata.h"

// We need the player data structure as well.
//#include "d_player.h"
//#include "d_clisrv.h"

// Game mode handling - identify IWAD version,
//  handle IWAD dependend animations etc.

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
extern bool_t inventory;		// true with heretic and hexen

// =========
// Language.
// =========
//
extern language_t language;

// =============================
// Selected skill type, map etc.
// =============================

// Nightmare mode flag, single player.
// extern  bool_t         respawnmonsters;

// GhostlyDeath -- Currently optional new network code
extern bool_t newnet_use;
extern bool_t newnet_solo;

// Netgame? only true in a netgame
extern bool_t netgame;
extern bool_t serverside;

// Only true if >1 player. netgame => multiplayer but not (multiplayer=>netgame)
extern bool_t multiplayer;


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

extern bool_t nodrawers;

extern int viewwindowx;
extern int viewwindowy;
extern int viewheight;
extern int viewwidth;
extern int scaledviewwidth;

// This one is related to the 3-screen display mode.
// ANG90 = left side, ANG270 = right
extern int viewangleoffset;


// ============================================
// Statistics on a given map, for intermission.
// ============================================
//
extern int totalkills;
extern int totalitems;
extern int totalsecret;

extern int32_t g_MapKIS[5];

// ===========================
// Internal parameters, fixed.
// ===========================
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.


#define localgametic  leveltime

// Player spawn spots.
extern mapthing_t* playerstarts[MAXPLAYERS];

#define MAX_DM_STARTS   64
extern mapthing_t* deathmatchstarts[MAX_DM_STARTS];
extern int numdmstarts;

extern mapthing_t* g_TeamStarts[MAXSKINCOLORS][MAXPLAYERS];

// Intermission stats.
// Parameters for world map / intermission.
extern wbstartstruct_t wminfo;

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


//?
// debug flag to cancel adaptiveness

#define   BODYQUESIZE     MAXPLAYERS

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

#endif							//__D_STATE__

