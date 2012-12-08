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
// DESCRIPTION: all external data is defined here
//              most of the data is loaded into different structures at run time
//              some internal structures shared by many modules are here
//

#ifndef __DOOMDATA__
#define __DOOMDATA__

// The most basic types we use, portability.
#include "doomtype.h"

// Some global defines, that configure the game.
#include "doomdef.h"

#include "info.h"

//
// Map level types.
// The following data structures define the persistent format
// used in the lumps of the WAD files.
//

// Lump order in a map WAD: each map needs a couple of lumps
// to provide a complete scene geometry description.
enum
{
	ML_LABEL,					// A separator, name, ExMx or MAPxx
	ML_THINGS,					// Monsters, items..
	ML_LINEDEFS,				// LineDefs, from editing
	ML_SIDEDEFS,				// SideDefs, from editing
	ML_VERTEXES,				// Vertices, edited and BSP splits generated
	ML_SEGS,					// LineSegs, from LineDefs split by BSP
	ML_SSECTORS,				// SubSectors, list of LineSegs
	ML_NODES,					// BSP nodes
	ML_SECTORS,					// Sectors, from editing
	ML_REJECT,					// LUT, sector-sector visibility
	ML_BLOCKMAP,				// LUT, motion clipping, walls/grid element
	
	ML_BEHAVIOR,				// Hexen
};

// A single Vertex.
typedef struct
{
	short x;
	short y;
} mapvertex_t;

// A SideDef, defining the visual appearance of a wall,
// by setting textures and offsets.
typedef struct
{
	short textureoffset;
	short rowoffset;
	char toptexture[8];
	char bottomtexture[8];
	char midtexture[8];
	// Front sector, towards viewer.
	short sector;
} mapsidedef_t;

// A LineDef, as used for editing, and as input
// to the BSP builder.
typedef struct
{
	int16_t v1;
	int16_t v2;
	int16_t flags;
	int16_t special;
	int16_t tag;
	// sidenum[1] will be -1 if one sided
	int16_t sidenum[2];
} maplinedef_t;

/* HexenMapLineDef_t -- Hexen linedef */
typedef struct HexenMapLineDef_s
{
	int16_t v1;
	int16_t v2;
	int16_t flags;
	uint8_t special;
	uint8_t Args[5];
	int16_t sidenum[2];
} HexenMapLineDef_t;

//
// LineDef attributes.
//

// Solid, is an obstacle.
#define ML_BLOCKING             1

// Blocks monsters only.
#define ML_BLOCKMONSTERS        2

// Backside will not be present at all
//  if not two sided.
#define ML_TWOSIDED             4

// If a texture is pegged, the texture will have
// the end exposed to air held constant at the
// top or bottom of the texture (stairs or pulled
// down things) and will move with a height change
// of one of the neighbor sectors.
// Unpegged textures allways have the first row of
// the texture at the top pixel of the line for both
// top and bottom textures (use next to windows).

// upper texture unpegged
#define ML_DONTPEGTOP           8

// lower texture unpegged
#define ML_DONTPEGBOTTOM        16

// In AutoMap: don't map as two sided: IT'S A SECRET!
#define ML_SECRET               32

// Sound rendering: don't let sound cross two of these.
#define ML_SOUNDBLOCK           64

// Don't draw on the automap at all.
#define ML_DONTDRAW             128

// Set if already seen, thus drawn in automap.
#define ML_MAPPED               256

//SoM: 3/29/2000: If flag is set, the player can use through it.
#define ML_PASSUSE              512

//SoM: 4/1/2000: If flag is set, anything can trigger the line.
#define ML_ALLTRIGGER           1024

#define ML_REPEAT_SPECIAL	0x0200	// special is repeatable
#define ML_SPAC_SHIFT		10
#define ML_SPAC_MASK		0x1c00

// Sector definition, from editing.
typedef struct
{
	short floorheight;
	short ceilingheight;
	char floorpic[8];
	char ceilingpic[8];
	short lightlevel;
	short special;
	short tag;
} mapsector_t;

// SubSector, as generated by BSP.
typedef struct
{
	short numsegs;
	// Index of first one, segs are stored sequentially.
	short firstseg;
} mapsubsector_t;

// LineSeg, generated by splitting LineDefs
// using partition lines selected by BSP builder.
typedef struct
{
	short v1;
	short v2;
	short angle;
	short linedef;
	short side;
	short offset;
} mapseg_t;

// BSP node structure.

// Indicate a leaf.
#define NF_SUBSECTOR    0x8000

typedef struct
{
	// Partition line from (x,y) to x+dx,y+dy)
	short x;
	short y;
	short dx;
	short dy;
	
	// Bounding box for each child,
	// clip against view frustum.
	short bbox[2][4];
	
	// If NF_SUBSECTOR its a subsector,
	// else it's a node of another subtree.
	unsigned short children[2];
	
} mapnode_t;

// Thing definition, position, orientation and type,
// plus skill/visibility flags and attributes.
typedef struct
{
	short x;
	short y;
	short z;					// Z support for objects SSNTails 07-24-2002
	short angle;
	short type;
	short options;
	
	struct mobj_s* mobj;
	
	// Hexen Stuff
	bool_t IsHexen;								// Hexen Defined
	int16_t HeightOffset;						// Height offset
	uint16_t ID;								// Hexen Thing ID
	uint8_t Special;							// Hexen Special
	uint8_t Args[5];							// Hexen arguments
	
	// Other Stuff
	PI_mobjid_t MoType;							// Type of spawned object
	bool_t MarkedWeapon;						// Marked as a weapon to respawn
} mapthing_t;

/* HexenMapThingDef_t -- Hexen map thing */
typedef struct HexenMapThingDef_s
{
	int16_t ID;
	int16_t x;
	int16_t y;
	int16_t z;					// Z support for objects SSNTails 07-24-2002
	int16_t angle;
	int16_t type;
	int16_t options;
	uint8_t Special;
	uint8_t Args[5];
	
	struct mobj_s* mobj;
} HexenMapThingDef_t;

extern char* Color_Names[MAXSKINCOLORS];

#endif							// __DOOMDATA__
