//------------------------------------------------------------------------
// SEG : Choose the best Seg to use for a node line.
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

#ifndef __GLBSP_SEG_H__
#define __GLBSP_SEG_H__

#include "gbsp_str.h"

#define DEFAULT_FACTOR  11

#define IFFY_LEN  4.0

// smallest distance between two points before being considered equal
#define DIST_EPSILON  (1.0 / 128.0)

// smallest degrees between two angles before being considered equal
#define ANG_EPSILON  (1.0 / 1024.0)

// an "intersection" remembers the vertex that touches a BSP divider
// line (especially a new vertex that is created at a seg split).

typedef struct glbsp_intersection_s
{
	// link in list.  The intersection list is kept sorted by
	// along_dist, in ascending order.
	struct glbsp_intersection_s *next;
	struct glbsp_intersection_s *prev;

	// vertex in question
	glbsp_vertex_t *vertex;

	// how far along the partition line the vertex is.  Zero is at the
	// partition seg's start point, positive values move in the same
	// direction as the partition's direction, and negative values move
	// in the opposite direction.
	double_t along_dist;

	// true if this intersection was on a self-referencing linedef
	bool_t self_ref;

	// sector on each side of the vertex (along the partition),
	// or NULL when that direction isn't OPEN.
	glbsp_sector_t *before;
	glbsp_sector_t *after;
}
glbsp_intersection_t;

/* -------- functions ---------------------------- */

// scan all the glsegs in the list, and choose the best seg to use as a
// partition line, returning it.  If no seg can be used, returns NULL.
// The 'depth' parameter is the current depth in the tree, used for
// computing  the current progress.  When stale_nd is not NULL, it can
// be used to quickly find a matching seg -- but if none match, or it
// is unsuitable, the pointer should be cleared.
//
glbsp_seg_t *PickNode(glbsp_superblock_t * seg_list, int depth, glbsp_glbsp_node_t ** stale_nd, int *stale_opposite);

// compute the boundary of the list of glsegs
void FindLimits(glbsp_superblock_t * seg_list, glbsp_bbox_t * bbox);

// compute the seg private info (psx/y, pex/y, pdx/y, etc).
void RecomputeSeg(glbsp_seg_t * seg);

// take the given seg 'cur', compare it with the partition line, and
// determine it's fate: moving it into either the left or right lists
// (perhaps both, when splitting it in two).  Handles partners as
// well.  Updates the intersection list if the seg lies on or crosses
// the partition line.
//
void DivideOneSeg(glbsp_seg_t * cur, glbsp_seg_t * part, glbsp_superblock_t * left_list, glbsp_superblock_t * right_list, glbsp_intersection_t ** cut_list);

// remove all the glsegs from the list, partitioning them into the left
// or right lists based on the given partition line.  Adds any
// intersections onto the intersection list as it goes.
//
void SeparateSegs(glbsp_superblock_t * seg_list, glbsp_seg_t * part, glbsp_superblock_t * left_list, glbsp_superblock_t * right_list, glbsp_intersection_t ** cut_list);

// analyse the intersection list, and add any needed minisegs to the
// given seg lists (one miniseg on each side).  All the intersection
// structures will be freed back into a quick-alloc list.
//
void AddMinisegs(glbsp_seg_t * part, glbsp_superblock_t * left_list, glbsp_superblock_t * right_list, glbsp_intersection_t * cut_list);

// free the quick allocation cut list
void FreeQuickAllocCuts(void);

#endif							/* __GLBSP_SEG_H__ */
