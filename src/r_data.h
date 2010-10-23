// Emacs style mode select   -*- C++ -*- 
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
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           TetrisMaster512        (tetrismaster512@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2010 The ReMooD Team..
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
// DESCRIPTION: Refresh module, data I/O, caching, retrieval of graphics by name.

#ifndef __R_DATA__
#define __R_DATA__

#include "r_defs.h"
#include "r_state.h"
#include "w_wad.h"

#ifdef __GNUG__
#pragma interface
#endif

// moved here for r_sky.c (texture_t is used)

//
// Texture definition.
// Each texture is composed of one or more patches,
// with patches being lumps stored in the WAD.
// The lumps are referenced by number, and patched
// into the rectangular texture space using origin
// and possibly other attributes.
//
typedef struct
{
	short originx;
	short originy;
	short patch;
	short stepdir;
	short colormap;
} mappatch_t;

//
// Texture definition.
// A DOOM wall texture is a list of patches
// which are to be combined in a predefined order.
//
typedef struct
{
	char name[8];
	boolean masked;
	short width;
	short height;

	UInt32 columndirectory;		// JUST IN CASE! but they say it's obsolete so..
	//void                **columndirectory;      // OBSOLETE

	short patchcount;
	mappatch_t patches[1];
} maptexture_t;

// A single patch from a texture definition,
//  basically a rectangular area within
//  the texture rectangle.
typedef struct
{
	// Block origin (allways UL),
	// which has allready accounted
	// for the internal origin of the patch.
	int originx;
	int originy;
	int patch;
} texpatch_t;

// A maptexturedef_t describes a rectangular texture,
//  which is composed of one or more mappatch_t structures
//  that arrange graphic patches.
typedef struct
{
	// Keep name for switch changing, etc.
	char name[9];
	short width;
	short height;

	// All the patches[patchcount]
	//  are drawn back to front into the cached texture.
	short patchcount;
	texpatch_t patches[1];

} texture_t;

// all loaded and prepared textures from the start of the game
extern texture_t **textures;

//extern lighttable_t    *colormaps;
extern CV_PossibleValue_t Color_cons_t[];

// Load TEXTURE1/TEXTURE2/PNAMES definitions, create lookup tables
void R_LoadTextures(void);
void R_FlushTextureCache(void);

// Retrieve column data for span blitting.
byte *R_GetColumn(int tex, size_t col);

byte *R_GetFlat(int flatnum);

// I/O, setting up the stuff.
void R_InitData(void);
void R_PrecacheLevel(void);

// Retrieval.
// Floor/ceiling opaque texture tiles,
// lookup by name. For animation?
int R_GetFlatNumForName(char *name);
int P_FlagNumForName(char *flatname);
#define R_FlatNumForName(x)    R_GetFlatNumForName(x)

// Called by P_Ticker for switches and animations,
// returns the texture number for the texture name.
int R_TextureNumForName(char *name);
int R_CheckTextureNumForName(char *name);

void R_ClearColormaps();
int R_ColormapNumForName(char *name);
int R_CreateColormap(char *p1, char *p2, char *p3);
char *R_ColormapNameForNum(int num);
#endif

