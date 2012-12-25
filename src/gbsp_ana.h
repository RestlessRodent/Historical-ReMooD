//------------------------------------------------------------------------
// ANALYZE : Analyzing level structures
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

#ifndef __GLBSP_ANALYZE_H__
#define __GLBSP_ANALYZE_H__

#include "gbsp_str.h"
#include "gbsp_lev.h"

// detection routines
void DetectDuplicateVertices(void);
void DetectDuplicateSidedefs(void);
void DetectPolyobjSectors(void);
void DetectOverlappingLines(void);
void DetectWindowEffects(void);

// pruning routines
void PruneLinedefs(void);
void PruneVertices(void);
void PruneSidedefs(void);
void PruneSectors(void);

// computes the wall tips for all of the vertices
void CalculateWallTips(void);

// return a new vertex (with correct wall_tip info) for the split that
// happens along the given seg at the given location.
//
glbsp_vertex_t *NewVertexFromSplitSeg(glbsp_seg_t * seg, double_t x, double_t y);

// return a new end vertex to compensate for a seg that would end up
// being zero-length (after integer rounding).  Doesn't compute the
// wall_tip info (thus this routine should only be used _after_ node
// building).
//
glbsp_vertex_t *NewVertexDegenerate(glbsp_vertex_t * start, glbsp_vertex_t * end);

// check whether a line with the given delta coordinates and beginning
// at this vertex is open.  Returns a sector reference if it's open,
// or NULL if closed (void space or directly along a linedef).
//
glbsp_sector_t *VertexCheckOpen(glbsp_vertex_t * vert, double_t dx, double_t dy);

#endif							/* __GLBSP_ANALYZE_H__ */
