//------------------------------------------------------------------------
// STRUCT : Doom structures, raw on-disk layout
//------------------------------------------------------------------------
//
//  GL-Friendly Node Builder (C) 2000-2007 Andrew Apted
//
//  Based on 'BSP 2.3' by Colin Reed, Lee Killough and others.
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//------------------------------------------------------------------------

#ifndef __GLBSP_STRUCTS_H__
#define __GLBSP_STRUCTS_H__

#include "doomtype.h"
#include "gbsp_glb.h"

/* ----- The wad structures ---------------------- */

// wad header

typedef struct raw_wad_header_s
{
	char type[4];

	uint32_t num_entries;
	uint32_t dir_start;
}
raw_wad_header_t;

// directory entry

typedef struct raw_wad_entry_s
{
	uint32_t start;
	uint32_t length;

	char name[8];
}
raw_wad_entry_t;

// blockmap

typedef struct raw_blockmap_header_s
{
	int16_t x_origin, y_origin;
	int16_t x_blocks, y_blocks;
}
raw_blockmap_header_t;

/* ----- The level structures ---------------------- */

typedef struct raw_glbsp_vertex_s
{
	int16_t x, y;
}
raw_glbsp_vertex_t;

typedef struct raw_v2_glbsp_vertex_s
{
	int32_t x, y;
}
raw_v2_glbsp_vertex_t;

typedef struct raw_glbsp_linedef_s
{
	uint16_t start;				// from this vertex...
	uint16_t end;				// ... to this vertex
	uint16_t flags;				// linedef flags (impassible, etc)
	uint16_t type;				// linedef type (0 for none, 97 for teleporter, etc)
	int16_t tag;				// this linedef activates the sector with same tag
	uint16_t sidedef1;			// right sidedef
	uint16_t sidedef2;			// left sidedef (only if this line adjoins 2 sectors)
}
raw_glbsp_linedef_t;

typedef struct raw_hexen_glbsp_linedef_s
{
	uint16_t start;				// from this vertex...
	uint16_t end;				// ... to this vertex
	uint16_t flags;				// linedef flags (impassible, etc)
	uint8_t type;				// linedef type
	uint8_t specials[5];		// hexen specials
	uint16_t sidedef1;			// right sidedef
	uint16_t sidedef2;			// left sidedef
}
raw_hexen_glbsp_linedef_t;

#define LINEFLAG_TWO_SIDED  4

#define HEXTYPE_POLY_START     1
#define HEXTYPE_POLY_EXPLICIT  5

typedef struct raw_glbsp_sidedef_s
{
	int16_t x_offset;			// X offset for texture
	int16_t y_offset;			// Y offset for texture

	char upper_tex[8];			// texture name for the part above
	char lower_tex[8];			// texture name for the part below
	char mid_tex[8];			// texture name for the regular part

	uint16_t sector;			// adjacent sector
}
raw_glbsp_sidedef_t;

typedef struct raw_glbsp_sector_s
{
	int16_t floor_h;			// floor height
	int16_t ceil_h;				// ceiling height

	char floor_tex[8];			// floor texture
	char ceil_tex[8];			// ceiling texture

	uint16_t light;				// light level (0-255)
	uint16_t special;			// special behaviour (0 = normal, 9 = secret, ...)
	int16_t tag;				// sector activated by a linedef with same tag
}
raw_glbsp_sector_t;

typedef struct raw_glbsp_thing_s
{
	int16_t x, y;				// position of thing
	int16_t angle;				// angle thing faces (degrees)
	uint16_t type;				// type of thing
	uint16_t options;			// when appears, deaf, etc..
}
raw_glbsp_thing_t;

// -JL- Hexen thing definition
typedef struct raw_hexen_glbsp_thing_s
{
	int16_t tid;				// thing tag id (for scripts/specials)
	int16_t x, y;				// position
	int16_t height;				// start height above floor
	int16_t angle;				// angle thing faces
	uint16_t type;				// type of thing
	uint16_t options;			// when appears, deaf, dormant, etc..

	uint8_t special;			// special type
	uint8_t arg[5];				// special arguments
}
raw_hexen_glbsp_thing_t;

// -JL- Hexen polyobj thing types
#define PO_ANCHOR_TYPE      3000
#define PO_SPAWN_TYPE       3001
#define PO_SPAWNCRUSH_TYPE  3002

// -JL- ZDoom polyobj thing types
#define ZDOOM_PO_ANCHOR_TYPE      9300
#define ZDOOM_PO_SPAWN_TYPE       9301
#define ZDOOM_PO_SPAWNCRUSH_TYPE  9302

/* ----- The BSP tree structures ----------------------- */

typedef struct raw_glbsp_seg_s
{
	uint16_t start;				// from this vertex...
	uint16_t end;				// ... to this vertex
	uint16_t angle;				// angle (0 = east, 16384 = north, ...)
	uint16_t linedef;			// linedef that this seg goes along
	uint16_t flip;				// true if not the same direction as linedef
	uint16_t dist;				// distance from starting point
}
raw_glbsp_seg_t;

typedef struct raw_gl_glbsp_seg_s
{
	uint16_t start;				// from this vertex...
	uint16_t end;				// ... to this vertex
	uint16_t linedef;			// linedef that this seg goes along, or -1
	uint16_t side;				// 0 if on right of linedef, 1 if on left
	uint16_t partner;			// partner seg number, or -1
}
raw_gl_glbsp_seg_t;

typedef struct raw_v3_glbsp_seg_s
{
	uint32_t start;				// from this vertex...
	uint32_t end;				// ... to this vertex
	uint16_t linedef;			// linedef that this seg goes along, or -1
	uint16_t side;				// 0 if on right of linedef, 1 if on left
	uint32_t partner;			// partner seg number, or -1
}
raw_v3_glbsp_seg_t;

typedef struct raw_glbsp_bbox_s
{
	int16_t maxy, miny;
	int16_t minx, maxx;
}
raw_glbsp_bbox_t;

typedef struct raw_glbsp_glbsp_node_s
{
	int16_t x, y;				// starting point
	int16_t dx, dy;				// offset to ending point
	raw_glbsp_bbox_t b1, b2;			// bounding rectangles
	uint16_t right, left;		// children: Node or SSector (if high bit is set)
}
raw_glbsp_glbsp_node_t;

typedef struct raw_glbsp_subsec_s
{
	uint16_t num;				// number of Segs in this Sub-Sector
	uint16_t first;				// first Seg
}
raw_glbsp_subsec_t;

typedef struct raw_v3_glbsp_subsec_s
{
	uint32_t num;				// number of Segs in this Sub-Sector
	uint32_t first;				// first Seg
}
raw_v3_glbsp_subsec_t;

typedef struct raw_v5_glbsp_glbsp_node_s
{
	int16_t x, y;				// starting point
	int16_t dx, dy;				// offset to ending point
	raw_glbsp_bbox_t b1, b2;			// bounding rectangles
	uint32_t right, left;		// children: Node or SSector (if high bit is set)
}
raw_v5_glbsp_glbsp_node_t;

extern const nodebuildinfo_t *cur_info;

#endif							/* __GLBSP_STRUCTS_H__ */
