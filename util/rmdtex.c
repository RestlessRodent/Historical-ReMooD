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
// Copyright (C) 2011 GhostlyDeath <ghostlydeath@gmail.com>
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
// DESCRIPTION: ReMooD `deutex` Clone, for what ReMooD uses and bonus stuff

/***************
*** INCLUDES ***
***************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <string.h>

/********************
*** BYTE SWAPPING ***
********************/

/****************
*** CONSTANTS ***
****************/

#define MAXEXTENSIONS 4

/* LumpType_t -- Type of lump this is */
typedef enum LumpType_e
{
	WT_LUMP,
	WT_FLAT,
	WT_GRAPHIC,
	WT_PICT,
	WT_RAWPIC,
	WT_SOUND,
	WT_SPRITE,
	
	NUMLUMPTYPES
} LumpType_t;

/*****************
*** STRUCTURES ***
*****************/

/* V_ColorEntry_t -- HSV table */
typedef struct V_ColorEntry_s
{
	struct
	{
		uint8_t R;
		uint8_t G;
		uint8_t B;
	} RGB;
	
	struct
	{
		uint8_t H;
		uint8_t S;
		uint8_t V;
	} HSV;
} V_ColorEntry_t;

/* WADEntry_t -- A single WAD entry */
typedef struct WADEntry_s
{
	char Name[9];							// Name of the entry
	uint8_t* Data;							// Data to write to WAD
	uint32_t Offset;						// Offset to the data
	uint32_t Size;							// Size to write to WAD
} WADEntry_t;

struct LumpDir_s;

/* PushyData_t -- Data to push to WAD */
typedef struct PushyData_s
{
	uint8_t* Data;
	size_t Size;
} PushyData_t;

typedef int (*HandleFunc_t)(struct LumpDir_s* const a_LumpDir, FILE* const File, const size_t Size, const char* const Ext, const char** const Args, PushyData_t* const Pushy);

/* LumpDir_t -- Directory where lumps are located */
typedef struct LumpDir_s
{
	LumpType_t Type;
	const char Dir[PATH_MAX];
	const char Extensions[MAXEXTENSIONS][4];
	HandleFunc_t Handler;
	const char NoteName[32];
} LumpDir_t;

/* Image_t -- An image */
typedef struct Image_s
{
	uint16_t Width;
	uint16_t Height;
	uint8_t* Pixels;
} Image_t;

/* Post_t -- A Post in a patch */
typedef struct Post_s
{
	uint16_t Offset;
	uint16_t Size;
	
	uint8_t* Ptr;
	int Trans;
} Post_t;

/**********************
*** LOWER CONSTANTS ***
**********************/

static int Handler_Lump(struct LumpDir_s* const a_LumpDir, FILE* const File, const size_t Size, const char* const Ext, const char** const Args, PushyData_t* const Pushy);
static int Handler_PicT(struct LumpDir_s* const a_LumpDir, FILE* const File, const size_t Size, const char* const Ext, const char** const Args, PushyData_t* const Pushy);
static int Handler_RawPic(struct LumpDir_s* const a_LumpDir, FILE* const File, const size_t Size, const char* const Ext, const char** const Args, PushyData_t* const Pushy);
static int Handler_PatchT(struct LumpDir_s* const a_LumpDir, FILE* const File, const size_t Size, const char* const Ext, const char** const Args, PushyData_t* const Pushy);
static int Handler_Sound(struct LumpDir_s* const a_LumpDir, FILE* const File, const size_t Size, const char* const Ext, const char** const Args, PushyData_t* const Pushy);

/* c_LumpDirs -- Directory where stuff is located */
static const LumpDir_t c_LumpDirs[NUMLUMPTYPES] =
{
	{WT_LUMP, "lumps", {"lmp"}, Handler_Lump, "lump"},
	{WT_FLAT, "flats", {"ppm"}, Handler_RawPic, "flat"},
	{WT_GRAPHIC, "graphics", {"ppm"}, Handler_PatchT, "patch_t"},
	{WT_PICT, "picts", {"ppm"}, Handler_PicT, "pic_t"},
	{WT_RAWPIC, "rawpics", {"ppm"}, Handler_RawPic, "raw image"},
	{WT_SOUND, "sounds", {"wav", "txt"}, Handler_Sound, "sound"},
	{WT_SPRITE, "sprites", {"ppm"}, Handler_PatchT, "sprite"},
};

/* c_Colors -- FreeDOOM's Palette */
static const V_ColorEntry_t c_Colors[256] =
{
	{{0000, 0000, 0000}, {0000, 0000, 0000}},	{{0x1f, 0x17, 0x0b}, {0x1f, 0x17, 0x0b}},
	{{0x17, 0x0f, 0x07}, {0x17, 0x0f, 0x07}},	{{0x4b, 0x4b, 0x4b}, {0x4b, 0x4b, 0x4b}},
	{{0xff, 0xff, 0xff}, {0xff, 0xff, 0xff}},	{{0x1b, 0x1b, 0x1b}, {0x1b, 0x1b, 0x1b}},
	{{0x13, 0x13, 0x13}, {0x13, 0x13, 0x13}},	{{0x0b, 0x0b, 0x0b}, {0x0b, 0x0b, 0x0b}},
	{{0x07, 0x07, 0x07}, {0x07, 0x07, 0x07}},	{{0x2f, 0x37, 0x1f}, {0x2f, 0x37, 0x1f}},
	{{0x23, 0x2b, 0x0f}, {0x23, 0x2b, 0x0f}},	{{0x17, 0x1f, 0x07}, {0x17, 0x1f, 0x07}},
	{{0x0f, 0x17, 0000}, {0x0f, 0x17, 0000}},	{{0x4f, 0x3b, 0x2b}, {0x4f, 0x3b, 0x2b}},
	{{0x47, 0x33, 0x23}, {0x47, 0x33, 0x23}},	{{0x3f, 0x2b, 0x1b}, {0x3f, 0x2b, 0x1b}},
	{{0xff, 0xb7, 0xb7}, {0xff, 0xb7, 0xb7}},	{{0xf7, 0xab, 0xab}, {0xf7, 0xab, 0xab}},
	{{0xf3, 0xa3, 0xa3}, {0xf3, 0xa3, 0xa3}},	{{0xeb, 0x97, 0x97}, {0xeb, 0x97, 0x97}},
	{{0xe7, 0x8f, 0x8f}, {0xe7, 0x8f, 0x8f}},	{{0xdf, 0x87, 0x87}, {0xdf, 0x87, 0x87}},
	{{0xdb, 0x7b, 0x7b}, {0xdb, 0x7b, 0x7b}},	{{0xd3, 0x73, 0x73}, {0xd3, 0x73, 0x73}},
	{{0xcb, 0x6b, 0x6b}, {0xcb, 0x6b, 0x6b}},	{{0xc7, 0x63, 0x63}, {0xc7, 0x63, 0x63}},
	{{0xbf, 0x5b, 0x5b}, {0xbf, 0x5b, 0x5b}},	{{0xbb, 0x57, 0x57}, {0xbb, 0x57, 0x57}},
	{{0xb3, 0x4f, 0x4f}, {0xb3, 0x4f, 0x4f}},	{{0xaf, 0x47, 0x47}, {0xaf, 0x47, 0x47}},
	{{0xa7, 0x3f, 0x3f}, {0xa7, 0x3f, 0x3f}},	{{0xa3, 0x3b, 0x3b}, {0xa3, 0x3b, 0x3b}},
	{{0x9b, 0x33, 0x33}, {0x9b, 0x33, 0x33}},	{{0x97, 0x2f, 0x2f}, {0x97, 0x2f, 0x2f}},
	{{0x8f, 0x2b, 0x2b}, {0x8f, 0x2b, 0x2b}},	{{0x8b, 0x23, 0x23}, {0x8b, 0x23, 0x23}},
	{{0x83, 0x1f, 0x1f}, {0x83, 0x1f, 0x1f}},	{{0x7f, 0x1b, 0x1b}, {0x7f, 0x1b, 0x1b}},
	{{0x77, 0x17, 0x17}, {0x77, 0x17, 0x17}},	{{0x73, 0x13, 0x13}, {0x73, 0x13, 0x13}},
	{{0x6b, 0x0f, 0x0f}, {0x6b, 0x0f, 0x0f}},	{{0x67, 0x0b, 0x0b}, {0x67, 0x0b, 0x0b}},
	{{0x5f, 0x07, 0x07}, {0x5f, 0x07, 0x07}},	{{0x5b, 0x07, 0x07}, {0x5b, 0x07, 0x07}},
	{{0x53, 0x07, 0x07}, {0x53, 0x07, 0x07}},	{{0x4f, 0000, 0000}, {0x4f, 0000, 0000}},
	{{0x47, 0000, 0000}, {0x47, 0000, 0000}},	{{0x43, 0000, 0000}, {0x43, 0000, 0000}},
	{{0xff, 0xeb, 0xdf}, {0xff, 0xeb, 0xdf}},	{{0xff, 0xe3, 0xd3}, {0xff, 0xe3, 0xd3}},
	{{0xff, 0xdb, 0xc7}, {0xff, 0xdb, 0xc7}},	{{0xff, 0xd3, 0xbb}, {0xff, 0xd3, 0xbb}},
	{{0xff, 0xcf, 0xb3}, {0xff, 0xcf, 0xb3}},	{{0xff, 0xc7, 0xa7}, {0xff, 0xc7, 0xa7}},
	{{0xff, 0xbf, 0x9b}, {0xff, 0xbf, 0x9b}},	{{0xff, 0xbb, 0x93}, {0xff, 0xbb, 0x93}},
	{{0xff, 0xb3, 0x83}, {0xff, 0xb3, 0x83}},	{{0xf7, 0xab, 0x7b}, {0xf7, 0xab, 0x7b}},
	{{0xef, 0xa3, 0x73}, {0xef, 0xa3, 0x73}},	{{0xe7, 0x9b, 0x6b}, {0xe7, 0x9b, 0x6b}},
	{{0xdf, 0x93, 0x63}, {0xdf, 0x93, 0x63}},	{{0xd7, 0x8b, 0x5b}, {0xd7, 0x8b, 0x5b}},
	{{0xcf, 0x83, 0x53}, {0xcf, 0x83, 0x53}},	{{0xcb, 0x7f, 0x4f}, {0xcb, 0x7f, 0x4f}},
	{{0xbf, 0x7b, 0x4b}, {0xbf, 0x7b, 0x4b}},	{{0xb3, 0x73, 0x47}, {0xb3, 0x73, 0x47}},
	{{0xab, 0x6f, 0x43}, {0xab, 0x6f, 0x43}},	{{0xa3, 0x6b, 0x3f}, {0xa3, 0x6b, 0x3f}},
	{{0x9b, 0x63, 0x3b}, {0x9b, 0x63, 0x3b}},	{{0x8f, 0x5f, 0x37}, {0x8f, 0x5f, 0x37}},
	{{0x87, 0x57, 0x33}, {0x87, 0x57, 0x33}},	{{0x7f, 0x53, 0x2f}, {0x7f, 0x53, 0x2f}},
	{{0x77, 0x4f, 0x2b}, {0x77, 0x4f, 0x2b}},	{{0x6b, 0x47, 0x27}, {0x6b, 0x47, 0x27}},
	{{0x5f, 0x43, 0x23}, {0x5f, 0x43, 0x23}},	{{0x53, 0x3f, 0x1f}, {0x53, 0x3f, 0x1f}},
	{{0x4b, 0x37, 0x1b}, {0x4b, 0x37, 0x1b}},	{{0x3f, 0x2f, 0x17}, {0x3f, 0x2f, 0x17}},
	{{0x33, 0x2b, 0x13}, {0x33, 0x2b, 0x13}},	{{0x2b, 0x23, 0x0f}, {0x2b, 0x23, 0x0f}},
	{{0xef, 0xef, 0xef}, {0xef, 0xef, 0xef}},	{{0xe7, 0xe7, 0xe7}, {0xe7, 0xe7, 0xe7}},
	{{0xdf, 0xdf, 0xdf}, {0xdf, 0xdf, 0xdf}},	{{0xdb, 0xdb, 0xdb}, {0xdb, 0xdb, 0xdb}},
	{{0xd3, 0xd3, 0xd3}, {0xd3, 0xd3, 0xd3}},	{{0xcb, 0xcb, 0xcb}, {0xcb, 0xcb, 0xcb}},
	{{0xc7, 0xc7, 0xc7}, {0xc7, 0xc7, 0xc7}},	{{0xbf, 0xbf, 0xbf}, {0xbf, 0xbf, 0xbf}},
	{{0xb7, 0xb7, 0xb7}, {0xb7, 0xb7, 0xb7}},	{{0xb3, 0xb3, 0xb3}, {0xb3, 0xb3, 0xb3}},
	{{0xab, 0xab, 0xab}, {0xab, 0xab, 0xab}},	{{0xa7, 0xa7, 0xa7}, {0xa7, 0xa7, 0xa7}},
	{{0x9f, 0x9f, 0x9f}, {0x9f, 0x9f, 0x9f}},	{{0x97, 0x97, 0x97}, {0x97, 0x97, 0x97}},
	{{0x93, 0x93, 0x93}, {0x93, 0x93, 0x93}},	{{0x8b, 0x8b, 0x8b}, {0x8b, 0x8b, 0x8b}},
	{{0x83, 0x83, 0x83}, {0x83, 0x83, 0x83}},	{{0x7f, 0x7f, 0x7f}, {0x7f, 0x7f, 0x7f}},
	{{0x77, 0x77, 0x77}, {0x77, 0x77, 0x77}},	{{0x6f, 0x6f, 0x6f}, {0x6f, 0x6f, 0x6f}},
	{{0x6b, 0x6b, 0x6b}, {0x6b, 0x6b, 0x6b}},	{{0x63, 0x63, 0x63}, {0x63, 0x63, 0x63}},
	{{0x5b, 0x5b, 0x5b}, {0x5b, 0x5b, 0x5b}},	{{0x57, 0x57, 0x57}, {0x57, 0x57, 0x57}},
	{{0x4f, 0x4f, 0x4f}, {0x4f, 0x4f, 0x4f}},	{{0x47, 0x47, 0x47}, {0x47, 0x47, 0x47}},
	{{0x43, 0x43, 0x43}, {0x43, 0x43, 0x43}},	{{0x3b, 0x3b, 0x3b}, {0x3b, 0x3b, 0x3b}},
	{{0x37, 0x37, 0x37}, {0x37, 0x37, 0x37}},	{{0x2f, 0x2f, 0x2f}, {0x2f, 0x2f, 0x2f}},
	{{0x27, 0x27, 0x27}, {0x27, 0x27, 0x27}},	{{0x23, 0x23, 0x23}, {0x23, 0x23, 0x23}},
	{{0x77, 0xff, 0x6f}, {0x77, 0xff, 0x6f}},	{{0x6f, 0xef, 0x67}, {0x6f, 0xef, 0x67}},
	{{0x67, 0xdf, 0x5f}, {0x67, 0xdf, 0x5f}},	{{0x5f, 0xcf, 0x57}, {0x5f, 0xcf, 0x57}},
	{{0x5b, 0xbf, 0x4f}, {0x5b, 0xbf, 0x4f}},	{{0x53, 0xaf, 0x47}, {0x53, 0xaf, 0x47}},
	{{0x4b, 0x9f, 0x3f}, {0x4b, 0x9f, 0x3f}},	{{0x43, 0x93, 0x37}, {0x43, 0x93, 0x37}},
	{{0x3f, 0x83, 0x2f}, {0x3f, 0x83, 0x2f}},	{{0x37, 0x73, 0x2b}, {0x37, 0x73, 0x2b}},
	{{0x2f, 0x63, 0x23}, {0x2f, 0x63, 0x23}},	{{0x27, 0x53, 0x1b}, {0x27, 0x53, 0x1b}},
	{{0x1f, 0x43, 0x17}, {0x1f, 0x43, 0x17}},	{{0x17, 0x33, 0x0f}, {0x17, 0x33, 0x0f}},
	{{0x13, 0x23, 0x0b}, {0x13, 0x23, 0x0b}},	{{0x0b, 0x17, 0x07}, {0x0b, 0x17, 0x07}},
	{{0xbf, 0xa7, 0x8f}, {0xbf, 0xa7, 0x8f}},	{{0xb7, 0x9f, 0x87}, {0xb7, 0x9f, 0x87}},
	{{0xaf, 0x97, 0x7f}, {0xaf, 0x97, 0x7f}},	{{0xa7, 0x8f, 0x77}, {0xa7, 0x8f, 0x77}},
	{{0x9f, 0x87, 0x6f}, {0x9f, 0x87, 0x6f}},	{{0x9b, 0x7f, 0x6b}, {0x9b, 0x7f, 0x6b}},
	{{0x93, 0x7b, 0x63}, {0x93, 0x7b, 0x63}},	{{0x8b, 0x73, 0x5b}, {0x8b, 0x73, 0x5b}},
	{{0x83, 0x6b, 0x57}, {0x83, 0x6b, 0x57}},	{{0x7b, 0x63, 0x4f}, {0x7b, 0x63, 0x4f}},
	{{0x77, 0x5f, 0x4b}, {0x77, 0x5f, 0x4b}},	{{0x6f, 0x57, 0x43}, {0x6f, 0x57, 0x43}},
	{{0x67, 0x53, 0x3f}, {0x67, 0x53, 0x3f}},	{{0x5f, 0x4b, 0x37}, {0x5f, 0x4b, 0x37}},
	{{0x57, 0x43, 0x33}, {0x57, 0x43, 0x33}},	{{0x53, 0x3f, 0x2f}, {0x53, 0x3f, 0x2f}},
	{{0x9f, 0x83, 0x63}, {0x9f, 0x83, 0x63}},	{{0x8f, 0x77, 0x53}, {0x8f, 0x77, 0x53}},
	{{0x83, 0x6b, 0x4b}, {0x83, 0x6b, 0x4b}},	{{0x77, 0x5f, 0x3f}, {0x77, 0x5f, 0x3f}},
	{{0x67, 0x53, 0x33}, {0x67, 0x53, 0x33}},	{{0x5b, 0x47, 0x2b}, {0x5b, 0x47, 0x2b}},
	{{0x4f, 0x3b, 0x23}, {0x4f, 0x3b, 0x23}},	{{0x43, 0x33, 0x1b}, {0x43, 0x33, 0x1b}},
	{{0x7b, 0x7f, 0x63}, {0x7b, 0x7f, 0x63}},	{{0x6f, 0x73, 0x57}, {0x6f, 0x73, 0x57}},
	{{0x67, 0x6b, 0x4f}, {0x67, 0x6b, 0x4f}},	{{0x5b, 0x63, 0x47}, {0x5b, 0x63, 0x47}},
	{{0x53, 0x57, 0x3b}, {0x53, 0x57, 0x3b}},	{{0x47, 0x4f, 0x33}, {0x47, 0x4f, 0x33}},
	{{0x3f, 0x47, 0x2b}, {0x3f, 0x47, 0x2b}},	{{0x37, 0x3f, 0x27}, {0x37, 0x3f, 0x27}},
	{{0xff, 0xff, 0x73}, {0xff, 0xff, 0x73}},	{{0xeb, 0xdb, 0x57}, {0xeb, 0xdb, 0x57}},
	{{0xd7, 0xbb, 0x43}, {0xd7, 0xbb, 0x43}},	{{0xc3, 0x9b, 0x2f}, {0xc3, 0x9b, 0x2f}},
	{{0xaf, 0x7b, 0x1f}, {0xaf, 0x7b, 0x1f}},	{{0x9b, 0x5b, 0x13}, {0x9b, 0x5b, 0x13}},
	{{0x87, 0x43, 0x07}, {0x87, 0x43, 0x07}},	{{0x73, 0x2b, 0000}, {0x73, 0x2b, 0000}},
	{{0xff, 0xff, 0xff}, {0xff, 0xff, 0xff}},	{{0xff, 0xdb, 0xdb}, {0xff, 0xdb, 0xdb}},
	{{0xff, 0xbb, 0xbb}, {0xff, 0xbb, 0xbb}},	{{0xff, 0x9b, 0x9b}, {0xff, 0x9b, 0x9b}},
	{{0xff, 0x7b, 0x7b}, {0xff, 0x7b, 0x7b}},	{{0xff, 0x5f, 0x5f}, {0xff, 0x5f, 0x5f}},
	{{0xff, 0x3f, 0x3f}, {0xff, 0x3f, 0x3f}},	{{0xff, 0x1f, 0x1f}, {0xff, 0x1f, 0x1f}},
	{{0xff, 0000, 0000}, {0xff, 0000, 0000}},	{{0xef, 0000, 0000}, {0xef, 0000, 0000}},
	{{0xe3, 0000, 0000}, {0xe3, 0000, 0000}},	{{0xd7, 0000, 0000}, {0xd7, 0000, 0000}},
	{{0xcb, 0000, 0000}, {0xcb, 0000, 0000}},	{{0xbf, 0000, 0000}, {0xbf, 0000, 0000}},
	{{0xb3, 0000, 0000}, {0xb3, 0000, 0000}},	{{0xa7, 0000, 0000}, {0xa7, 0000, 0000}},
	{{0x9b, 0000, 0000}, {0x9b, 0000, 0000}},	{{0x8b, 0000, 0000}, {0x8b, 0000, 0000}},
	{{0x7f, 0000, 0000}, {0x7f, 0000, 0000}},	{{0x73, 0000, 0000}, {0x73, 0000, 0000}},
	{{0x67, 0000, 0000}, {0x67, 0000, 0000}},	{{0x5b, 0000, 0000}, {0x5b, 0000, 0000}},
	{{0x4f, 0000, 0000}, {0x4f, 0000, 0000}},	{{0x43, 0000, 0000}, {0x43, 0000, 0000}},
	{{0xe7, 0xe7, 0xff}, {0xe7, 0xe7, 0xff}},	{{0xc7, 0xc7, 0xff}, {0xc7, 0xc7, 0xff}},
	{{0xab, 0xab, 0xff}, {0xab, 0xab, 0xff}},	{{0x8f, 0x8f, 0xff}, {0x8f, 0x8f, 0xff}},
	{{0x73, 0x73, 0xff}, {0x73, 0x73, 0xff}},	{{0x53, 0x53, 0xff}, {0x53, 0x53, 0xff}},
	{{0x37, 0x37, 0xff}, {0x37, 0x37, 0xff}},	{{0x1b, 0x1b, 0xff}, {0x1b, 0x1b, 0xff}},
	{{0000, 0000, 0xff}, {0000, 0000, 0xff}},	{{0000, 0000, 0xe3}, {0000, 0000, 0xe3}},
	{{0000, 0000, 0xcb}, {0000, 0000, 0xcb}},	{{0000, 0000, 0xb3}, {0000, 0000, 0xb3}},
	{{0000, 0000, 0x9b}, {0000, 0000, 0x9b}},	{{0000, 0000, 0x83}, {0000, 0000, 0x83}},
	{{0000, 0000, 0x6b}, {0000, 0000, 0x6b}},	{{0000, 0000, 0x53}, {0000, 0000, 0x53}},
	{{0xff, 0xff, 0xff}, {0xff, 0xff, 0xff}},	{{0xff, 0xeb, 0xdb}, {0xff, 0xeb, 0xdb}},
	{{0xff, 0xd7, 0xbb}, {0xff, 0xd7, 0xbb}},	{{0xff, 0xc7, 0x9b}, {0xff, 0xc7, 0x9b}},
	{{0xff, 0xb3, 0x7b}, {0xff, 0xb3, 0x7b}},	{{0xff, 0xa3, 0x5b}, {0xff, 0xa3, 0x5b}},
	{{0xff, 0x8f, 0x3b}, {0xff, 0x8f, 0x3b}},	{{0xff, 0x7f, 0x1b}, {0xff, 0x7f, 0x1b}},
	{{0xf3, 0x73, 0x17}, {0xf3, 0x73, 0x17}},	{{0xeb, 0x6f, 0x0f}, {0xeb, 0x6f, 0x0f}},
	{{0xdf, 0x67, 0x0f}, {0xdf, 0x67, 0x0f}},	{{0xd7, 0x5f, 0x0b}, {0xd7, 0x5f, 0x0b}},
	{{0xcb, 0x57, 0x07}, {0xcb, 0x57, 0x07}},	{{0xc3, 0x4f, 0000}, {0xc3, 0x4f, 0000}},
	{{0xb7, 0x47, 0000}, {0xb7, 0x47, 0000}},	{{0xaf, 0x43, 0000}, {0xaf, 0x43, 0000}},
	{{0xff, 0xff, 0xff}, {0xff, 0xff, 0xff}},	{{0xff, 0xff, 0xd7}, {0xff, 0xff, 0xd7}},
	{{0xff, 0xff, 0xb3}, {0xff, 0xff, 0xb3}},	{{0xff, 0xff, 0x8f}, {0xff, 0xff, 0x8f}},
	{{0xff, 0xff, 0x6b}, {0xff, 0xff, 0x6b}},	{{0xff, 0xff, 0x47}, {0xff, 0xff, 0x47}},
	{{0xff, 0xff, 0x23}, {0xff, 0xff, 0x23}},	{{0xff, 0xff, 0000}, {0xff, 0xff, 0000}},
	{{0xa7, 0x3f, 0000}, {0xa7, 0x3f, 0000}},	{{0x9f, 0x37, 0000}, {0x9f, 0x37, 0000}},
	{{0x93, 0x2f, 0000}, {0x93, 0x2f, 0000}},	{{0x87, 0x23, 0000}, {0x87, 0x23, 0000}},
	{{0x4f, 0x3b, 0x27}, {0x4f, 0x3b, 0x27}},	{{0x43, 0x2f, 0x1b}, {0x43, 0x2f, 0x1b}},
	{{0x37, 0x23, 0x13}, {0x37, 0x23, 0x13}},	{{0x2f, 0x1b, 0x0b}, {0x2f, 0x1b, 0x0b}},
	{{0000, 0000, 0x53}, {0000, 0000, 0x53}},	{{0000, 0000, 0x47}, {0000, 0000, 0x47}},
	{{0000, 0000, 0x3b}, {0000, 0000, 0x3b}},	{{0000, 0000, 0x2f}, {0000, 0000, 0x2f}},
	{{0000, 0000, 0x23}, {0000, 0000, 0x23}},	{{0000, 0000, 0x17}, {0000, 0000, 0x17}},
	{{0000, 0000, 0x0b}, {0000, 0000, 0x0b}},	{{0000, 0000, 0000}, {0000, 0000, 0000}},	// 247 == TRANSPARENT!
	{{0xff, 0x9f, 0x43}, {0xff, 0x9f, 0x43}},	{{0xff, 0xe7, 0x4b}, {0xff, 0xe7, 0x4b}},
	{{0xff, 0x7b, 0xff}, {0xff, 0x7b, 0xff}},	{{0xff, 0000, 0xff}, {0xff, 0000, 0xff}},
	{{0xcf, 0000, 0xcf}, {0xcf, 0000, 0xcf}},	{{0x9f, 0000, 0x9b}, {0x9f, 0000, 0x9b}},
	{{0x6f, 0000, 0x6b}, {0x6f, 0000, 0x6b}},	{{0xa7, 0x6b, 0x6b}, {0xa7, 0x6b, 0x6b}},
};

#define TRANSPX 247

/*************
*** LOCALS ***
*************/

static WADEntry_t* l_Entries = NULL;						// WAD Entries
static uint32_t l_NumEntries = 0;								// Number of entries
static uint32_t l_TableSpot = 0;								// Location of the lump table

/****************
*** FUNCTIONS ***
****************/

/* AddLump() -- Adds a lump to the WAD */
static void AddLump(const char* const Name, void* const Data, const size_t Size)
{
	size_t i;
	
	l_Entries = realloc(l_Entries, sizeof(*l_Entries) * (l_NumEntries + 1));
	
	memset(&l_Entries[l_NumEntries], 0, sizeof(l_Entries[l_NumEntries]));
	strncpy(l_Entries[l_NumEntries].Name, Name, 9);
	l_Entries[l_NumEntries].Data = Data;
	l_Entries[l_NumEntries].Size = Size;
	
	// Uppercase name
	for (i = 0; i <= 8; i++)
		l_Entries[l_NumEntries].Name[i] = toupper(l_Entries[l_NumEntries].Name[i]);
	
	// If this is the first entry then set the offset to the base, 12
	if (!l_NumEntries)
		l_Entries[l_NumEntries].Offset = 12;
	
	// Otherwise it starts after the last lump
	else
		l_Entries[l_NumEntries].Offset = l_Entries[l_NumEntries - 1].Offset + l_Entries[l_NumEntries - 1].Size;
	
	// Move table ahead
	l_TableSpot = l_Entries[l_NumEntries].Offset + l_Entries[l_NumEntries].Size;
	
	l_NumEntries++;
}

/* AddMarker() -- Adds a marker to the WAD */
static void AddMarker(const char* const Name)
{
	static uint8_t Marker;
	AddLump(Name, &Marker, 0);
}

/* ReadPPMToken() -- Reads a PPM Token */
static void ReadPPMTokem(FILE* const File, uint8_t* const OutBuf, const size_t OutSize)
{
	size_t i;
	fpos_t Pos;
	uint8_t Byte;
	int ReadSomething, InComment;
	
	/* Reset */
	i = 0;
	ReadSomething = 0;
	memset(OutBuf, 0, OutSize);
	InComment = 0;
	
	/* Read bytes */
	while (fread(&Byte, 1, 1, File))
	{
		// Whitespace?
		if (isspace(Byte))
		{
			// If a comment is being read, only stop at \n
			if (InComment && Byte == '\n')
				return;
			
			// If not in a comment and something was read, stop
			else if (!InComment && ReadSomething)
				return;
			
			// Otherwise just continue on
			continue;
		}
		
		// Otherwise, read the data
		if (i < OutSize)
			OutBuf[i++] = Byte;
			
		// If it was a comment, set as comment (Stop at \n)
		if (Byte == '#')
			InComment = 1;
		
		// indicate that something was read
		ReadSomething = 1;
	}
}

static size_t V_BestHSVMatch(const V_ColorEntry_t* const Table, const V_ColorEntry_t HSV);
static V_ColorEntry_t V_RGBtoHSV(const V_ColorEntry_t RGB);
static size_t V_BestRGBMatch(const V_ColorEntry_t* const Table, const V_ColorEntry_t RGB);

/* LoadPPM() -- Loads a PPM */
static Image_t* LoadPPM(FILE* const File)
{
#define BUFSIZE 512
	Image_t* Create;
	uint8_t Buf[BUFSIZE];
	uint32_t Width, Height;
	uint32_t Depth;
	size_t Tokey, x, y;
	char* Tok;
	V_ColorEntry_t Color;
	
	/* Read header */
	// Check for P6
	ReadPPMTokem(File, Buf, BUFSIZE);
	
	if (strcmp(Buf, "P6") != 0)
	{
		fprintf(stderr, "Err: .ppm is not P6 (is \"%s\").\n", Buf);
		return NULL;
	}
	
	/* Constantly read data */
	Tokey = 0;
	while (Tokey < 3)
	{
		// Read another token
		ReadPPMTokem(File, Buf, BUFSIZE);
		
		// If it starts with #, skip it (a comment)
		if (Buf[0] == '#')
			continue;
		
		// Image Width
		if (Tokey == 0)
		{
			Width = atoi(Buf);
		}
		
		// Image height
		else if (Tokey == 1)
		{
			Height = atoi(Buf);
		}
		
		// File depth (bits)
		else if (Tokey == 2)
		{
			Depth = atoi(Buf);
		}
		
		// Increment
		Tokey++;
	}
	
	/* Create blank image */
	Create = malloc(sizeof(*Create) + (Width * Height));
	
	if (!Create)
		return NULL;
	
	memset(Create, 0, sizeof(*Create) + (Width * Height));
	Create->Width = Width;
	Create->Height = Height;
	Create->Pixels = ((uint8_t*)Create) + sizeof(*Create);
	
	/* Read image data */
	for (y = 0; y < Height; y++)
		for (x = 0; x < Width; x++)
		{
			// Clear color
			memset(&Color, 0, sizeof(Color));
			
			// Read color
			fread(&Color.RGB.R, 1, 1, File);
			fread(&Color.RGB.G, 1, 1, File);
			fread(&Color.RGB.B, 1, 1, File);
			
			// Find closest color
			Color = V_RGBtoHSV(Color);
			Create->Pixels[(y * Width) + x] = V_BestRGBMatch(c_Colors, Color);
		}
	
	/* Return created image */
	return Create;
#undef BUFSIZE
}

/* Handler_Lump() -- Handles lumps */
static int Handler_Lump(struct LumpDir_s* const a_LumpDir, FILE* const File, const size_t Size, const char* const Ext, const char** const Args, PushyData_t* const Pushy)
{
	/* Very simple */
	Pushy->Size = Size;
	Pushy->Data = malloc(Pushy->Size);
	return fread(Pushy->Data, Size, 1, File);
}

/* Handler_PicT() -- Handles pic_ts */
static int Handler_PicT(struct LumpDir_s* const a_LumpDir, FILE* const File, const size_t Size, const char* const Ext, const char** const Args, PushyData_t* const Pushy)
{
	Image_t* Image = NULL;
	size_t i;
	
	/* Is this a PPM? */
	if (strcasecmp(Ext, "ppm") == 0)
		Image = LoadPPM(File);
		
	// No Image?
	if (!Image)
	{
		fprintf(stderr, "Err: Failed to create image.\n");
		return 0;
	}
	
	/* Create pic_t structure */
	Pushy->Size = 8 + (Image->Width * Image->Height);
	Pushy->Data = malloc(Pushy->Size);
	memset(Pushy->Data, 0, Pushy->Size);
	
	/* Write Data to pic_t */
	((uint16_t*)Pushy->Data)[0] = Image->Width;
	((uint16_t*)Pushy->Data)[1] = 0;
	((uint16_t*)Pushy->Data)[2] = Image->Height;
	((uint16_t*)Pushy->Data)[3] = 0;
	
	for (i = 0; i < Image->Width * Image->Height; i++)
		Pushy->Data[8 + i] = Image->Pixels[i];
	
	/* Free Image */
	free(Image);
	
	/* Success */
	return 1;
}

/* Handler_RawPic() -- Raw picture */
static int Handler_RawPic(struct LumpDir_s* const a_LumpDir, FILE* const File, const size_t Size, const char* const Ext, const char** const Args, PushyData_t* const Pushy)
{
	Image_t* Image = NULL;
	size_t i;
	
	/* Is this a PPM? */
	if (strcasecmp(Ext, "ppm") == 0)
		Image = LoadPPM(File);
		
	// No Image?
	if (!Image)
	{
		fprintf(stderr, "Err: Failed to create image.\n");
		return 0;
	}
	
	/* Create pic_t structure */
	Pushy->Size = (Image->Width * Image->Height);
	Pushy->Data = malloc(Pushy->Size);
	memset(Pushy->Data, 0, Pushy->Size);
	
	/* Write Data to pic_t */
	for (i = 0; i < Image->Width * Image->Height; i++)
		Pushy->Data[i] = Image->Pixels[i];
	
	/* Free Image */
	free(Image);
	
	/* Success */
	return 1;
}

/* Handler_PatchT() -- handles patch_ts */
static int Handler_PatchT(struct LumpDir_s* const a_LumpDir, FILE* const File, const size_t Size, const char* const Ext, const char** const Args, PushyData_t* const Pushy)
{
#define BUFJUMP 1024
	Image_t* Image = NULL;
	size_t i, j, x, y;
	Post_t** Posts = NULL;
	uint16_t* NumPosts = NULL;
	uint8_t p;
	uint32_t* ColOffs;
	uint8_t* PostBuf;
	size_t PostSz, PostSpot, Bar;
	
	/* Is this a PPM? */
	if (strcasecmp(Ext, "ppm") == 0)
		Image = LoadPPM(File);
		
	// No Image?
	if (!Image)
	{
		fprintf(stderr, "Err: Failed to create image.\n");
		return 0;
	}
	
	/* Get posts */
	Posts = malloc(sizeof(*Posts) * Image->Width);
	NumPosts = malloc(sizeof(*NumPosts) * Image->Width);
	memset(Posts, 0, sizeof(Posts));
	memset(NumPosts, 0, sizeof(NumPosts));
	
	// go through each columns
	for (x = 0; x < Image->Width; x++)
	{
		// Create initial post
		Posts[x] = malloc(sizeof(*Posts[x]));
		memset(Posts[x], 0, sizeof(*Posts[x]));
		NumPosts[x] = 1;
		
		// Is the first pixel transparent?
		Posts[x][NumPosts[x] - 1].Trans = (Image->Pixels[x] == TRANSPX);
		Posts[x][NumPosts[x] - 1].Offset = 0;
		Posts[x][NumPosts[x] - 1].Size = 1;
		Posts[x][NumPosts[x] - 1].Ptr = &Image->Pixels[x];
		
		// Go through each columns
		for (y = 1; y < Image->Height; y++)
		{
			// Get pixel
			p = Image->Pixels[(y * Image->Width) + x];
			
			// Change in transparency?
			if ((p == TRANSPX && Posts[x][NumPosts[x] - 1].Trans == 0) || (p != TRANSPX && Posts[x][NumPosts[x] - 1].Trans == 1))
			{
				// Increase Add a new post
				Posts[x] = realloc(Posts[x], sizeof(*Posts[x]) * (NumPosts[x] + 1));
				Posts[x][NumPosts[x]].Offset = y;//Posts[x][NumPosts[x] - 1].Offset + Posts[x][NumPosts[x] - 1].Size;
				Posts[x][NumPosts[x]].Size = 1;
				Posts[x][NumPosts[x]].Ptr = &Image->Pixels[(y * Image->Width) + x];
				Posts[x][NumPosts[x]].Trans = (p == TRANSPX);
				NumPosts[x]++;
			}
			
			// Normal
			else
			{
#if 0
				// Transparent?
				if (Posts[x][NumPosts[x] - 1].Trans)
					Posts[x][NumPosts[x] - 1].Offset++;
				
				// Not transparent
				else
#endif
					Posts[x][NumPosts[x] - 1].Size++;
			}
		}
		
		// Debug
		/*for (i = 0; i < NumPosts[x]; i++)
			fprintf(stderr, "C %2i P %2i: Off %4u Sz %4u %s\n", x, i, Posts[x][i].Offset, Posts[x][i].Size, (Posts[x][i].Trans ? "TRANS" : "OPAQ"));*/
	}
	
	/* Setup offsets */
	ColOffs = malloc(sizeof(*ColOffs) * Image->Width);
	memset(ColOffs, 0, sizeof(*ColOffs) * Image->Width);
	
	/* Draw posts into a buffer of sorts */
	PostBuf = NULL;
	PostSpot = PostSz = 0;
	for (x = 0; x < Image->Width; x++)
	{
		// Buffer too small?
		if (!PostSz || PostSpot >= PostSz - (BUFJUMP >> 1))
		{
			PostBuf = realloc(PostBuf, sizeof(*PostBuf) * (PostSz + BUFJUMP));
			PostSz += BUFJUMP;
		}
		
		// Set offset here
		ColOffs[x] = PostSpot;
		
		// Go through all the posts and draw into buffer (PostSz could be exceeded)
		if (NumPosts[x] > 1 || (NumPosts[x] == 1 && !Posts[x][0].Trans))
			for (i = 0; i < NumPosts[x]; i++)
			{
				// Skip transparent posts
				if (Posts[x][i].Trans)
					continue;
			
				// Draw post info into buffer
				//fprintf(stderr, "%x %i %i\n", i, Posts[x][i].Offset, Posts[x][i].Size);
				PostBuf[PostSpot++] = Posts[x][i].Offset;
				PostBuf[PostSpot++] = Posts[x][i].Size;
				PostBuf[PostSpot++] = 0;
			
				// Draw data
				for (j = 0; j < Posts[x][i].Size; j++)
					PostBuf[PostSpot++] = Posts[x][i].Ptr[j * Image->Width];
			
				// Cap off
				PostBuf[PostSpot++] = 0;
			}
		
		// column contains a single post, and it is completely transparent
		else if (Posts[x][0].Trans)
		{
			// Draw post info into buffer
			PostBuf[PostSpot++] = 0;
			PostBuf[PostSpot++] = 0;
			PostBuf[PostSpot++] = 0;
			PostBuf[PostSpot++] = 0;
		}
		
		// Done drawing posts, end it
		PostBuf[PostSpot++] = 0xFF;
	}
	
	/* Write Data into lump */
	//fprintf(stderr, "image data: %i\n", PostSpot);
	
	Pushy->Size = 8 + (4 * Image->Width) + PostSpot + 1;
	Pushy->Data = malloc(Pushy->Size);
	memset(Pushy->Data, 0, Pushy->Size);
	
	//fprintf(stderr, "%s a: %i %i\n", Args[0], atoi(Args[1]), atoi(Args[2]));
	// Write Header
	((int16_t*)Pushy->Data)[0] = Image->Width;
	((int16_t*)Pushy->Data)[1] = Image->Height;
	((int16_t*)Pushy->Data)[2] = atoi(Args[1]);
	((int16_t*)Pushy->Data)[3] = atoi(Args[2]);
	
	// Write Column offsets
	Bar = 8 + (4 * Image->Width);
	for (i = 0; i < Image->Width; i++)
		((int32_t*)Pushy->Data)[2 + i] = Bar + ColOffs[i];
	
	// Write Image data here
	memmove(&Pushy->Data[Bar], PostBuf, PostSpot);
	
	/* Free Image */
	// Free stuff left over
	for (i = 0; i < Image->Width; i++)
		free(Posts[i]);
	free(ColOffs);
	free(Posts);
	free(NumPosts);
	free(PostBuf);
	free(Image);
	
	return 1;
#undef BUFJUMP
}

/* Handler_Sound() -- Handles sounds */
static int Handler_Sound(struct LumpDir_s* const a_LumpDir, FILE* const File, const size_t Size, const char* const Ext, const char** const Args, PushyData_t* const Pushy)
{
	return 0;
}

/* V_HSVtoRGB() -- Convert HSV to RGB */
static V_ColorEntry_t V_HSVtoRGB(const V_ColorEntry_t HSV)
{
	int R, G, B, H, S, V, P, Q, T, F;
	V_ColorEntry_t Ret;
	
	/* Get inital values */
	Ret.HSV.H = H = HSV.HSV.H;
	Ret.HSV.S = S = HSV.HSV.S;
	Ret.HSV.V = V = HSV.HSV.V;
	
	/* Gray Color? */
	if (!S)
	{
		R = G = B = V;
	}
	
	/* Real Color */
	else
	{
		// Calculate Hue Shift
		F = ((H % 60) * 255) / 60;
		H /= 60;
		
		// Calculate channel values
		P = (V * (256 - S)) / 256;
		Q = (V * (256 - (S * F) / 256)) / 256;
		T = (V * (256 - (S * (256 - F)) / 256)) / 256;
		
		switch (H)
		{
			case 0:
				R = V;
				G = T;
				B = P;
				break;
			
			case 1:
				R = Q;
				G = V;
				B = P;
				break;
			
			case 2:
				R = P;
				G = V;
				B = T;
				break;
			
			case 3:
				R = P;
				G = Q;
				B = V;
				break;
			
			case 4:
				R = T;
				G = P;
				B = V;
				break;
			
			default:
				R = V;
				G = P;
				B = Q;
				break;
		}
	}
	
	/* Set Return */
	Ret.RGB.R = R;
	Ret.RGB.G = G;
	Ret.RGB.B = B;
	
	/* Return */
	return Ret;
}

/* V_RGBtoHSV() -- Convert RGB to HSV */
static V_ColorEntry_t V_RGBtoHSV(const V_ColorEntry_t RGB)
{
	V_ColorEntry_t Ret;
	uint8_t rMin, rMax, rDif;
	
	/* Copy original */
	Ret.RGB.R = RGB.RGB.R;
	Ret.RGB.G = RGB.RGB.G;
	Ret.RGB.B = RGB.RGB.B;
	
	// Get min/max
	rMin = 255;
	rMax = 0;

	// Get RGB minimum
	if (RGB.RGB.R < rMin)
		rMin = RGB.RGB.R;
	if (RGB.RGB.G < rMin)
		rMin = RGB.RGB.G;
	if (RGB.RGB.B < rMin)
		rMin = RGB.RGB.B;

	// Get RGB maximum
	if (RGB.RGB.R > rMax)
		rMax = RGB.RGB.R;
	if (RGB.RGB.G > rMax)
		rMax = RGB.RGB.G;
	if (RGB.RGB.B > rMax)
		rMax = RGB.RGB.B;

	// Obtain value
	Ret.HSV.V = rMax;

	// Short circuit?
	if (Ret.HSV.V == 0)
	{
		Ret.HSV.H = Ret.HSV.S = 0;
		return Ret;
	}

	// Obtain difference
	rDif = rMax - rMin;

	// Obtain saturation
	Ret.HSV.S = (uint8_t)(((uint32_t)255 * (uint32_t)rDif) / (uint32_t)Ret.HSV.V);

	// Short circuit?
	if (Ret.HSV.S == 0)
	{
		Ret.HSV.H = 0;
		return Ret;
	}

	/* Obtain hue */
	if (rMax == RGB.RGB.R)
		Ret.HSV.H = 43 * (RGB.RGB.G - RGB.RGB.B) / rMax;
	else if (rMax == RGB.RGB.G)
		Ret.HSV.H = 85 + (43 * (RGB.RGB.B - RGB.RGB.R) / rMax);
	else
		Ret.HSV.H = 171 + (43 * (RGB.RGB.R - RGB.RGB.G) / rMax);
	
	return Ret;
}

/* V_BestRGBMatch() -- Best match between RGB for tables */
static size_t V_BestRGBMatch(const V_ColorEntry_t* const Table, const V_ColorEntry_t RGB)
{
	size_t i, Best;
	V_ColorEntry_t tRGB, iRGB;
	int32_t BestSqr, ThisSqr, Dr, Dg, Db;
	FILE* LumpFile;
	
	/* Check */
	if (!Table)
		return 0;
	
	/* Convert input to RGB */
	iRGB = RGB;
	
	/* Transparent color? */
	if (iRGB.RGB.R == 0 && iRGB.RGB.G == 255 && iRGB.RGB.B == 255)
		return TRANSPX;
	
	/* Loop colors */
	for (Best = 0, BestSqr = 0x7FFFFFFFUL, i = 0; i < 256; i++)
	{
		// Convert table entry to RGB
		tRGB = Table[i];
		
		// Ignore transparent pixel
		if (i == TRANSPX)
			continue;
		
		// Perfect match?
		if (iRGB.RGB.R == tRGB.RGB.R && iRGB.RGB.B == tRGB.RGB.B && iRGB.RGB.G == tRGB.RGB.G)
			return i;
		
		// Distance of colors
		Dr = tRGB.RGB.R - iRGB.RGB.R;
		Dg = tRGB.RGB.G - iRGB.RGB.G;
		Db = tRGB.RGB.B - iRGB.RGB.B;
		ThisSqr = (Dr * Dr) + (Dg * Dg) + (Db * Db);
		
		// Closer?
		if (ThisSqr < BestSqr)
		{
			Best = i;
			BestSqr = ThisSqr;
		}
	}
	
	/* Fail */
	return Best;
}

/* V_BestHSVMatch() -- Best match between HSV for tables */
static size_t V_BestHSVMatch(const V_ColorEntry_t* const Table, const V_ColorEntry_t HSV)
{
	size_t i, Best;
	V_ColorEntry_t tRGB, iRGB;
	int32_t BestSqr, ThisSqr, Dr, Dg, Db;
	FILE* LumpFile;
	
	/* Check */
	if (!Table)
		return 0;
	
	/* Convert input to RGB */
	iRGB = V_HSVtoRGB(HSV);
	
	/* Loop colors */
	for (Best = 0, BestSqr = 0x7FFFFFFFUL, i = 0; i < 256; i++)
	{
		// Convert table entry to RGB
		tRGB = V_HSVtoRGB(Table[i]);
		
		// Perfect match?
		if (iRGB.RGB.R == tRGB.RGB.R && iRGB.RGB.B == tRGB.RGB.B && iRGB.RGB.G == tRGB.RGB.G)
			return i;
		
		// Distance of colors
		Dr = tRGB.RGB.R - iRGB.RGB.R;
		Dg = tRGB.RGB.G - iRGB.RGB.G;
		Db = tRGB.RGB.B - iRGB.RGB.B;
		ThisSqr = (Dr * Dr) + (Dg * Dg) + (Db * Db);
		
		// Closer?
		if (ThisSqr < BestSqr)
		{
			Best = i;
			BestSqr = ThisSqr;
		}
	}
	
	/* Fail */
	return Best;
}

/* main() -- Main entry point */
int main(int argc, char** argv)
{
#define BUFSIZE 512
#define MAXARGS 4
	char Buf[BUFSIZE];
	char Arg[MAXARGS][BUFSIZE];
	char* Args[MAXARGS];
	char Ext[5];
	char* p;
	char* Tok;
	size_t i, j, n, ld, q;
	FILE* InTXT;
	FILE* OutWAD;
	FILE* LumpFile;
	LumpType_t Type = 0, NewType;
	size_t LumpSize;
	PushyData_t Push;
	
	/* Check */
	if (argc < 3)
	{
		fprintf(stderr, "Usage: %s <wadinfo.txt> <output.wad>\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	/* Open inputs and outputs */
	InTXT = fopen(argv[1], "rt");
	OutWAD = fopen(argv[2], "wt");
	
	// Check
	if (!InTXT || !OutWAD)
	{
		if (InTXT)
			fclose(InTXT);
		if (OutWAD)
			fclose(OutWAD);
		return EXIT_FAILURE;
	}
	
	/* Prepare arg pointers */
	for (i = 0; i < MAXARGS; i++)
		Args[i] = Arg[i];
	
	/* Read inputs */
	ld = 0;
	while (!feof(InTXT))
	{
		// Read line
		fgets(Buf, BUFSIZE, InTXT);
		p = Buf;
		
		// Strip any useless whitespace
		while (*p && isspace(*p))
			p++;
		
		for (n = strlen(p) - 1; isspace(p[n]) && n >= 0 && n != (size_t)-1; n--)
			p[n] = '\0';
		
		// Translate all tabs to spaces
		for (i = 0, n = strlen(p); i < n; i++)
			if (p[i] == '\t')
				p[i] = ' ';
		
		// Comment? Blank Lines? Ignore
		if (*p == '#' || !strlen(p))
			continue;
			
		// Lower case
		for (i = 0; i < strlen(p); i++)
			p[i] = tolower(p[i]);
		
		// A new type group
		if (*p == '[')
		{
			// Remove [..]
			p++;
			p[strlen(p) - 1] = '\0';
			
			// Find type
			NewType = WT_LUMP;
			for (ld = 0; ld < NUMLUMPTYPES; ld++)
				if (strcasecmp(c_LumpDirs[ld].Dir, p) == 0)
				{
					fprintf(stderr, "Not: Loading %s\n", c_LumpDirs[ld].Dir);
					NewType = c_LumpDirs[ld].Type;
					break;
				}
			
			// Change of type?
			if (NewType != Type)
			{
				// Start of Sprites
				if (NewType == WT_SPRITE)
					AddMarker("SS_START");
				
				// End of sprites
				if (Type == WT_SPRITE)
					AddMarker("SS_END");
					
				// Start of flats
				if (NewType == WT_FLAT)
					AddMarker("FF_START");
				
				// End of flats
				if (Type == WT_FLAT)
					AddMarker("FF_END");
			}
			
			// Update
			Type = NewType;
		}
		
		// A normal lump
		else
		{
			// Clear arguments
			memset(Arg, 0, sizeof(Arg));
			
			// They are space separated
			i = 0;
			Tok = strtok(p, " ");
			while (Tok)
			{
				// Copy arguments
				if (i < MAXARGS)
					strncpy(Arg[i++], Tok, BUFSIZE);
				
				// Get next
				Tok = strtok(NULL, " ");
			}
			
			// Symlink?
			if (Arg[1][0] == '~')
			{
				for (q = 0; q < l_NumEntries; q++)
					if (strcasecmp(l_Entries[q].Name, Arg[2]) == 0)
						break;
				
				// Found?
				if (q < l_NumEntries)
				{
					AddLump(Arg[0], l_Entries[q].Data, l_Entries[q].Size);
					fprintf(stderr, "Symlinked \"%s\" >> \"%s\"\n", Arg[0], Arg[2]);
					continue;
				}
				
				// Not found, erase
				Arg[1][0] = Arg[2][0] = '\0';
			}
			
			// Determine filename to load
			memset(Ext, 0, sizeof(Ext));
			LumpFile = NULL;
			for (i = 0; i < MAXEXTENSIONS; i++)
				if (c_LumpDirs[ld].Extensions[i][0])
				{
					memset(Buf, 0, sizeof(Buf));
					strncpy(Buf, c_LumpDirs[ld].Dir, BUFSIZE);
					strncat(Buf, "/", BUFSIZE);
					
					// Alias?
					if (Arg[1][0] == '=')
						strncat(Buf, Arg[2], BUFSIZE);
					else
						strncat(Buf, Arg[0], BUFSIZE);
					
					// Pop extension
					strncat(Buf, ".", BUFSIZE);
					strncat(Buf, c_LumpDirs[ld].Extensions[i], BUFSIZE);
					
					// Attempt file open
					LumpFile = fopen(Buf, "rb");
					
					// it worked!
					if (LumpFile)
					{
						strncpy(Ext, c_LumpDirs[ld].Extensions[i], 5);
						break;
					}
				}
			
			if (!LumpFile)
			{
				fprintf(stderr, "Err: Failed to find data for \"%s\"\n", Arg[0]);
				continue;
			}
			
			// Common stuff, determine size
			fseek(LumpFile, 0, SEEK_END);
			LumpSize = ftell(LumpFile);
			fseek(LumpFile, 0, SEEK_SET);
			
			// Handle Data Type
			memset(&Push, 0, sizeof(Push));
			
			if (c_LumpDirs[ld].Handler)
				if (!c_LumpDirs[ld].Handler(&c_LumpDirs[ld], LumpFile, LumpSize, Ext, Args, &Push))
					fprintf(stderr, "Err: Handler had trouble with %s \"%s\"\n", c_LumpDirs[ld].NoteName, Arg[0]);
				else
				{
					// Push `Data` and `Size` in WAD
					AddLump(Arg[0], Push.Data, Push.Size);
					fprintf(stderr, "Not: Added %s \"%s\"\n", c_LumpDirs[ld].NoteName, Arg[0]);
				}
			else
				fprintf(stderr, "Err: No handler for %s \"%s\"\n", c_LumpDirs[ld].NoteName, Arg[0]);
			
			// Close data file
			fclose(LumpFile);
			LumpFile = NULL;
		}
	}
	
	/* Create WAD */
	// Create header
	fwrite("PWAD", 4, 1, OutWAD);
	fwrite(&l_NumEntries, 4, 1, OutWAD);
	fwrite(&l_TableSpot, 4, 1, OutWAD);
	
	// Write entry data
	for (i = 0; i < l_NumEntries; i++)
		fwrite(l_Entries[i].Data, l_Entries[i].Size, 1, OutWAD);
	
	// Write table
	for (i = 0; i < l_NumEntries; i++)
	{
		fwrite(&l_Entries[i].Offset, 4, 1, OutWAD);
		fwrite(&l_Entries[i].Size, 4, 1, OutWAD);
		fwrite(l_Entries[i].Name, 8, 1, OutWAD);
	}
	
	/* Close */
	fclose(InTXT);
	fclose(OutWAD);
	
	/* Success! */
	return EXIT_SUCCESS;
#undef BUFSIZE
#undef MAXARGS
}

