//------------------------------------------------------------------------
// LEVEL : Level structures & read/write functions.
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

#ifndef __GLBSP_LEVEL_H__
#define __GLBSP_LEVEL_H__

#include "structs.h"
#include "wad.h"

struct glbsp_glbsp_node_s;
struct glbsp_sector_s;
struct glbsp_superblock_s;

// a wall_tip is where a wall meets a vertex
typedef struct glbsp_wall_tip_s
{
	// link in list.  List is kept in ANTI-clockwise order.
	struct glbsp_wall_tip_s *next;
	struct glbsp_wall_tip_s *prev;

	// angle that line makes at vertex (degrees).
	angle_g angle;

	// sectors on each side of wall.  Left is the side of increasing
	// angles, right is the side of decreasing angles.  Either can be
	// NULL for one sided walls.
	struct glbsp_sector_s *left;
	struct glbsp_sector_s *right;
}
glbsp_wall_tip_t;

typedef struct glbsp_vertex_s
{
	// coordinates
	float_g x, y;

	// vertex index.  Always valid after loading and pruning of unused
	// vertices has occurred.  For GL vertices, bit 30 will be set.
	int index;

	// reference count.  When building normal node info, unused vertices
	// will be pruned.
	int ref_count;

	// usually NULL, unless this vertex occupies the same location as a
	// previous vertex.  Only used during the pruning phase.
	struct glbsp_vertex_s *equiv;

	// set of wall_tips
	glbsp_wall_tip_t *tip_set;

	// contains a duplicate vertex, needed when both normal and V2 GL
	// nodes are being built at the same time (this is the vertex used
	// for the normal segs).  Normally NULL.  Note: the wall tips on
	// this vertex are not created.
	struct glbsp_vertex_s *normal_dup;
}
glbsp_vertex_t;

#define IS_GL_VERTEX  (1 << 30)

typedef struct glbsp_sector_s
{
	// sector index.  Always valid after loading & pruning.
	int index;

	// allow segs from other sectors to coexist in a subsector.
	char coalesce;

	// -JL- non-zero if this sector contains a polyobj.
	int has_polyobj;

	// reference count.  When building normal nodes, unused sectors will
	// be pruned.
	int ref_count;

	// heights
	int floor_h, ceil_h;

	// textures
	char floor_tex[8];
	char ceil_tex[8];

	// attributes
	int light;
	int special;
	int tag;

	// used when building REJECT table.  Each set of sectors that are
	// isolated from other sectors will have a different group number.
	// Thus: on every 2-sided linedef, the sectors on both sides will be
	// in the same group.  The rej_next, rej_prev fields are a link in a
	// RING, containing all sectors of the same group.
	int rej_group;

	struct glbsp_sector_s *rej_next;
	struct glbsp_sector_s *rej_prev;

	// suppress superfluous mini warnings
	int warned_facing;
	char warned_unclosed;
}
glbsp_sector_t;

typedef struct glbsp_sidedef_s
{
	// adjacent sector.  Can be NULL (invalid sidedef)
	glbsp_sector_t *sector;

	// offset values
	int x_offset, y_offset;

	// texture names
	char upper_tex[8];
	char lower_tex[8];
	char mid_tex[8];

	// sidedef index.  Always valid after loading & pruning.
	int index;

	// reference count.  When building normal nodes, unused sidedefs will
	// be pruned.
	int ref_count;

	// usually NULL, unless this sidedef is exactly the same as a
	// previous one.  Only used during the pruning phase.
	struct glbsp_sidedef_s *equiv;

	// this is true if the sidedef is on a special line.  We don't merge
	// these sidedefs together, as they might scroll, or change texture
	// when a switch is pressed.
	int on_special;
}
glbsp_sidedef_t;

typedef struct glbsp_linedef_s
{
	// link for list
	struct glbsp_linedef_s *next;

	glbsp_vertex_t *start;			// from this vertex...
	glbsp_vertex_t *end;				// ... to this vertex

	glbsp_sidedef_t *right;			// right sidedef
	glbsp_sidedef_t *left;			// left sidede, or NULL if none

	// line is marked two-sided
	char two_sided;

	// prefer not to split
	char is_precious;

	// zero length (line should be totally ignored)
	char zero_len;

	// sector is the same on both sides
	char self_ref;

	// one-sided linedef used for a special effect (windows).
	// The value refers to the opposite sector on the back side.
	glbsp_sector_t *window_effect;

	int flags;
	int type;
	int tag;

	// Hexen support
	int specials[5];

	// normally NULL, except when this linedef directly overlaps an earlier
	// one (a rarely-used trick to create higher mid-masked textures).
	// No segs should be created for these overlapping linedefs.
	struct glbsp_linedef_s *overlap;

	// linedef index.  Always valid after loading & pruning of zero
	// length lines has occurred.
	int index;
}
glbsp_linedef_t;

typedef struct glbsp_thing_s
{
	int x, y;
	int type;
	int options;

	// other info (angle, and hexen stuff) omitted.  We don't need to
	// write the THING lump, only read it.

	// Always valid (thing indices never change).
	int index;
}
glbsp_thing_t;

typedef struct glbsp_seg_s
{
	// link for list
	struct glbsp_seg_s *next;

	glbsp_vertex_t *start;			// from this vertex...
	glbsp_vertex_t *end;				// ... to this vertex

	// linedef that this seg goes along, or NULL if miniseg
	glbsp_linedef_t *linedef;

	// adjacent sector, or NULL if invalid sidedef or miniseg
	glbsp_sector_t *sector;

	// 0 for right, 1 for left
	int side;

	// seg on other side, or NULL if one-sided.  This relationship is
	// always one-to-one -- if one of the segs is split, the partner seg
	// must also be split.
	struct glbsp_seg_s *partner;

	// seg index.  Only valid once the seg has been added to a
	// subsector.  A negative value means it is invalid -- there
	// shouldn't be any of these once the BSP tree has been built.
	int index;

	// when 1, this seg has become zero length (integer rounding of the
	// start and end vertices produces the same location).  It should be
	// ignored when writing the SEGS or V1 GL_SEGS lumps.  [Note: there
	// won't be any of these when writing the V2 GL_SEGS lump].
	int degenerate;

	// the superblock that contains this seg, or NULL if the seg is no
	// longer in any superblock (e.g. now in a subsector).
	struct glbsp_superblock_s *block;

	// precomputed data for faster calculations
	float_g psx, psy;
	float_g pex, pey;
	float_g pdx, pdy;

	float_g p_length;
	float_g p_angle;
	float_g p_para;
	float_g p_perp;

	// linedef that this seg initially comes from.  For "real" segs,
	// this is just the same as the 'linedef' field above.  For
	// "minisegs", this is the linedef of the partition line.
	glbsp_linedef_t *source_line;
}
glbsp_seg_t;

typedef struct glbsp_subsec_s
{
	// list of segs
	glbsp_seg_t *seg_list;

	// count of segs
	int seg_count;

	// subsector index.  Always valid, set when the subsector is
	// initially created.
	int index;

	// approximate middle point
	float_g mid_x;
	float_g mid_y;
}
glbsp_subsec_t;

typedef struct glbsp_bbox_s
{
	int minx, miny;
	int maxx, maxy;
}
glbsp_bbox_t;

typedef struct glbsp_child_s
{
	// child node or subsector (one must be NULL)
	struct glbsp_glbsp_node_s *node;
	glbsp_subsec_t *subsec;

	// child bounding box
	glbsp_bbox_t bounds;
}
glbsp_child_t;

typedef struct glbsp_glbsp_node_s
{
	int x, y;					// starting point
	int dx, dy;					// offset to ending point

	// right & left children
	glbsp_child_t r;
	glbsp_child_t l;

	// node index.  Only valid once the NODES or GL_NODES lump has been
	// created.
	int index;

	// the node is too long, and the (dx,dy) values should be halved
	// when writing into the NODES lump.
	int too_long;
}
glbsp_glbsp_node_t;

typedef struct glbsp_superblock_s
{
	// parent of this block, or NULL for a top-level block
	struct glbsp_superblock_s *parent;

	// coordinates on map for this block, from lower-left corner to
	// upper-right corner.  Pseudo-inclusive, i.e (x,y) is inside block
	// if and only if x1 <= x < x2 and y1 <= y < y2.
	int x1, y1;
	int x2, y2;

	// sub-blocks.  NULL when empty.  [0] has the lower coordinates, and
	// [1] has the higher coordinates.  Division of a square always
	// occurs horizontally (e.g. 512x512 -> 256x512 -> 256x256).
	struct glbsp_superblock_s *subs[2];

	// number of real segs and minisegs contained by this block
	// (including all sub-blocks below it).
	int real_num;
	int mini_num;

	// list of segs completely contained by this block.
	glbsp_seg_t *segs;
}
glbsp_superblock_t;

#define SUPER_IS_LEAF(s)  \
    ((s)->x2-(s)->x1 <= 256 && (s)->y2-(s)->y1 <= 256)

/* ----- Level data arrays ----------------------- */

extern int num_vertices;
extern int num_linedefs;
extern int num_sidedefs;
extern int num_sectors;
extern int num_things;
extern int num_segs;
extern int num_subsecs;
extern int num_nodes;
extern int num_stale_nodes;

extern int num_normal_vert;
extern int num_gl_vert;
extern int num_complete_seg;

/* ----- function prototypes ----------------------- */

// allocation routines
glbsp_vertex_t *NewVertex(void);
glbsp_linedef_t *NewLinedef(void);
glbsp_sidedef_t *NewSidedef(void);
glbsp_sector_t *NewSector(void);
glbsp_thing_t *NewThing(void);
glbsp_seg_t *NewSeg(void);
glbsp_subsec_t *NewSubsec(void);
glbsp_glbsp_node_t *NewNode(void);
glbsp_glbsp_node_t *NewStaleNode(void);
glbsp_wall_tip_t *NewWallTip(void);

// lookup routines
glbsp_vertex_t *LookupVertex(int index);
glbsp_linedef_t *LookupLinedef(int index);
glbsp_sidedef_t *LookupSidedef(int index);
glbsp_sector_t *LookupSector(int index);
glbsp_thing_t *LookupThing(int index);
glbsp_seg_t *LookupSeg(int index);
glbsp_subsec_t *LookupSubsec(int index);
glbsp_glbsp_node_t *LookupNode(int index);
glbsp_glbsp_node_t *LookupStaleNode(int index);

// check whether the current level already has normal nodes
int CheckForNormalNodes(void);

// load all level data for the current level
void LoadLevel(void);

// free all level data
void FreeLevel(void);

// save the newly computed NODE info etc..
void SaveLevel(glbsp_glbsp_node_t * root_node);

#endif							/* __GLBSP_LEVEL_H__ */
