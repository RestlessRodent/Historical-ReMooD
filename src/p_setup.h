// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.sourceforge.net/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           Demyx                  (demyx@endgameftw.com)
//                    Dragan                 (poliee13@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2009 The ReMooD Team..
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION: Setup a game, startup stuff.

#ifndef __P_SETUP__
#define __P_SETUP__

#include "doomdata.h"
#include "r_defs.h"

// Player spawn spots for deathmatch.
#define MAX_DM_STARTS   64
extern mapthing_t *deathmatchstarts[MAX_DM_STARTS];
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
extern levelflat_t *levelflats;
int P_AddLevelFlat(char *flatname, levelflat_t * levelflat);
char *P_FlatNameForNum(int num);

extern int nummapthings;
extern mapthing_t *mapthings;

// NOT called by W_Ticker. Fixme.
boolean P_SetupLevel(int episode, int map, skill_t skill, char *mapname);

boolean P_AddWadFile(char *wadfilename, char **firstmapname);

subsector_t *R_PointInSubsector(fixed_t x, fixed_t y);

extern boolean newlevel;
extern boolean doom1level;
extern char *levelmapname;

#endif
