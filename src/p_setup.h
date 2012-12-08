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
// DESCRIPTION: Setup a game, startup stuff.

#ifndef __P_SETUP__
#define __P_SETUP__

#include "doomdata.h"
#include "r_defs.h"

#include "p_info.h"

// Player spawn spots for deathmatch.
#define MAX_DM_STARTS   64
extern mapthing_t* deathmatchstarts[MAX_DM_STARTS];
extern int numdmstarts;

//extern  mapthing_t**    deathmatch_p;

extern int lastloadedmaplumpnum;	// for comparative savegame

//
// MAP used flats lookup table
//
typedef struct
{
	char name[8];				// resource name from wad
	int lumpnum;				// lump number of the flat
	
	// for flat animation
	int baselumpnum;
	int animseq;				// start pos. in the anim sequence
	int numpics;
	int speed;
} levelflat_t;

extern int numlevelflats;
extern levelflat_t* levelflats;
int P_AddLevelFlat(char* flatname, levelflat_t* levelflat);
char* P_FlatNameForNum(int num);

extern int nummapthings;
extern mapthing_t* mapthings;

// NOT called by W_Ticker. Fixme.
bool_t P_SetupLevel(int episode, int map, G_Skill_t skill, char* mapname);

subsector_t* R_PointInSubsector(fixed_t x, fixed_t y);

extern bool_t newlevel;
extern bool_t doom1level;
extern char* levelmapname;

/*******************************************************************************
********************************************************************************
*******************************************************************************/

/*** CONSTANTS ***/

/* P_ExLLFlags_t -- Load level flags */
typedef enum P_ExLLFlags_e
{
	PEXLL_NOPLREVIVE			= 0x0000001,	// Do not revive players
	PEXLL_NOSPAWNMAPTHING		= 0x0000002,	// Do not spawn map things
	PEXLL_NOSPAWNSPECIALS		= 0x0000004,	// Do not spawn specials
	PEXLL_NOINITBRAIN			= 0x0000008,	// Do not initialize brain targets
	PEXLL_NOFINALIZE			= 0x0000010,	// Do not finalize the level
	PEXLL_NOCLEARLEVEL			= 0x0000020,	// Does not clear the level
} P_ExLLFlags_t;

/*** STRUCTURES ***/

/*** PROTOTYPES ***/
void P_InitSetupEx(void);

bool_t P_ExClearLevel(void);

bool_t P_ExLoadLevel(P_LevelInfoEx_t* const a_Info, const uint32_t a_Flags);
bool_t P_ExFinalizeLevel(void);

#endif

