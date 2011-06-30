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
extern boolean inventory;		// true with heretic and hexen

// Set if homebrew PWAD stuff has been added.
extern boolean modifiedgame;

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
extern byte gameepisode;
extern byte gamemap;

// Nightmare mode flag, single player.
// extern  boolean         respawnmonsters;

// GhostlyDeath -- Currently optional new network code
extern boolean newnet_use;
extern boolean newnet_solo;

// Netgame? only true in a netgame
extern boolean netgame;
extern boolean serverside;
extern boolean localgame;
// Only true if >1 player. netgame => multiplayer but not (multiplayer=>netgame)
extern boolean multiplayer;

// Flag: true only if started as net deathmatch.
// An enum might handle altdeath/cooperative better.
extern consvar_t cv_deathmatch;

// ========================================
// Internal parameters for sound rendering.
// ========================================

extern boolean nomusic;			//defined in d_main.c
extern boolean nosound;
extern boolean digmusic;		// SSNTails 12-13-2002

// =========================
// Status flags for refresh.
// =========================
//

// Depending on view size - no status bar?
// Note that there is no way to disable the
//  status bar explicitely.
extern boolean statusbaractive;

extern boolean menuactive;		// Menu overlayed?
extern boolean paused;			// Game Pause?

extern boolean nodrawers;
extern boolean noblit;

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
#define localgametic  leveltime

// Player spawn spots.
extern mapthing_t *playerstarts[MAXPLAYERS];

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

#ifdef __MACOS__
#define DEBFILE(msg) I_OutputMsg(msg)
extern FILE *debugfile;
#else
#define DEBUGFILE
#ifdef DEBUGFILE
#define DEBFILE(msg) { if(debugfile) fputs(msg,debugfile); }
extern FILE *debugfile;
#else
#define DEBFILE(msg) {}
extern FILE *debugfile;
#endif
#endif							//__MACOS__

// if true, load all graphics at level load
extern boolean precache;

// wipegamestate can be set to -1
//  to force a wipe on the next draw
extern gamestate_t wipegamestate;

//?
// debug flag to cancel adaptiveness
extern boolean singletics;

#define   BODYQUESIZE     32

extern mobj_t *bodyque[BODYQUESIZE];
extern int bodyqueslot;

// =============
// Netgame stuff
// =============

//extern  ticcmd_t        localcmds[BACKUPTICS];

extern ticcmd_t netcmds[BACKUPTICS][MAXPLAYERS];

extern boolean novideo;

#endif							//__D_STATE__

