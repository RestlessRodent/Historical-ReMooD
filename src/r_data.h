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
// DESCRIPTION: Refresh module, data I/O, caching, retrieval of graphics by name.

#ifndef __R_DATA__
#define __R_DATA__

#include "r_defs.h"
#include "r_state.h"
#include "w_wad.h"

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
} mappatch_t;	// DEPRECATED

//
// Texture definition.
// A DOOM wall texture is a list of patches
// which are to be combined in a predefined order.
//
typedef struct
{
	char name[8];
	bool_t masked;
	short width;
	short height;
	
	uint32_t columndirectory;	// JUST IN CASE! but they say it's obsolete so..
	//void                **columndirectory;      // OBSOLETE
	
	short patchcount;
	mappatch_t patches[1];
} maptexture_t;	// DEPRECATED

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
	int patch;									// DEPRECATED
	
	/* Unified Data */
	char PatchName[9];							// Name of patch to use
	const WL_WADEntry_t* Entry;					// Entry image
	size_t PatchListRef;						// Reference to patch list
} texpatch_t;

// A maptexturedef_t describes a rectangular texture,
//  which is composed of one or more mappatch_t structures
//  that arrange graphic patches.
typedef struct
{
	// Keep name for switch changing, etc.
	char name[9];
	int32_t width;
	int32_t height;
	
	// All the patches[patchcount]
	//  are drawn back to front into the cached texture.
	uint32_t patchcount;
	texpatch_t* patches;
	
	/* Unified Data */
	uint32_t InternalOrder;						// Internal texture order
	uint32_t CombinedOrder;						// Texture order (when ordered)
	bool_t IsFlat;								// Is flat texture (floor)
	bool_t NonPowerTwo;							// Non power of 2 width texture (slow)
	uint32_t* ColumnOffs;						// Column offsets
	uint8_t** Composite;						// Composite Data
	uint8_t* Cache;								// Texture cache
	uint8_t* FlatCache;							// Flat cache
	uint32_t WidthMask;							// Mask to width
	fixed_t XWidth;								// fixed_t width of texture
	fixed_t XHeight;							// fixed_t height of texture
	uint32_t Translation;						// Which texture to draw in place of this one
	uint32_t CacheSize;							// Size of cache
	void* FlatEntry;							// Entry for flat
	bool_t Marked;								// Marked?
	void* FlatImage;							// Image for flat
	uint16_t OrderMul;							// Order Multiplier
} texture_t;

// all loaded and prepared textures from the start of the game
extern texture_t** textures;
extern int numtextures;

//extern lighttable_t    *colormaps;
extern CV_PossibleValue_t Color_cons_t[];

// Load TEXTURE1/TEXTURE2/PNAMES definitions, create lookup tables
void R_LoadTextures(void);
void R_FlushTextureCache(void);

// Retrieve column data for span blitting.
uint8_t* R_GetColumn(int tex, size_t col);

uint8_t* R_GetFlat(int flatnum);

// I/O, setting up the stuff.
void R_InitData(void);
void R_PrecacheLevel(void);

// Retrieval.
// Floor/ceiling opaque texture tiles,
// lookup by name. For animation?
int R_GetFlatNumForName(char* name);
int P_FlagNumForName(char* flatname);

#define R_FlatNumForName(x)    R_GetFlatNumForName(x)

// Called by P_Ticker for switches and animations,
// returns the texture number for the texture name.
int R_TextureNumForName(char* name);
int R_CheckTextureNumForName(char* name);

void R_ClearColormaps();
int R_ColormapNumForName(char* name);
int R_CreateColormap(char* p1, char* p2, char* p3);
char* R_ColormapNameForNum(int num);

void R_SetSpriteLumpCount(const size_t a_Count);

void R_InitExSpriteInfo(R_SpriteInfoEx_t* const a_Info, const V_ColorPal_t a_ColorPal);

#endif
