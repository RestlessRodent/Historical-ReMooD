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

#include "doomtype.h"

#include "gbsp_ana.h"
#include "gbsp_blo.h"
#include "gbsp_lev.h"
#include "gbsp_nod.h"
#include "gbsp_rej.h"
#include "gbsp_seg.h"
#include "gbsp_str.h"

#define POLY_BOX_SZ  10

// stuff needed from level.c (this file closely related)
extern glbsp_vertex_t **lev_vertices;
extern glbsp_linedef_t **lev_linedefs;
extern glbsp_sidedef_t **lev_sidedefs;
extern glbsp_sector_t **lev_sectors;

/* ----- polyobj handling ----------------------------- */

static void MarkPolyobjSector(glbsp_sector_t * sector)
{
	int i;

	if (!sector)
		return;

	/* already marked ? */
	if (sector->has_polyobj)
		return;

	/* mark all lines of this sector as precious, to prevent the sector
	 * from being split.
	 */
	sector->has_polyobj = true;

	for (i = 0; i < num_linedefs; i++)
	{
		glbsp_linedef_t *L = lev_linedefs[i];

		if ((L->right && L->right->sector == sector) || (L->left && L->left->sector == sector))
		{
			L->is_precious = true;
		}
	}
}

static void MarkPolyobjPoint(double_t x, double_t y)
{
	int i;
	int inside_count = 0;

	double_t best_dist = 999999;
	glbsp_linedef_t *best_match = NULL;
	glbsp_sector_t *sector = NULL;

	double_t x1, y1;
	double_t x2, y2;

	// -AJA- First we handle the "awkward" cases where the polyobj sits
	//       directly on a linedef or even a vertex.  We check all lines
	//       that intersect a small box around the spawn point.

	int bminx = (int)(x - POLY_BOX_SZ);
	int bminy = (int)(y - POLY_BOX_SZ);
	int bmaxx = (int)(x + POLY_BOX_SZ);
	int bmaxy = (int)(y + POLY_BOX_SZ);

	for (i = 0; i < num_linedefs; i++)
	{
		glbsp_linedef_t *L = lev_linedefs[i];

		if (CheckLinedefInsideBox(bminx, bminy, bmaxx, bmaxy, (int)L->start->x, (int)L->start->y, (int)L->end->x, (int)L->end->y))
		{
			if (L->left)
				MarkPolyobjSector(L->left->sector);

			if (L->right)
				MarkPolyobjSector(L->right->sector);

			inside_count++;
		}
	}

	if (inside_count > 0)
		return;

	// -AJA- Algorithm is just like in DEU: we cast a line horizontally
	//       from the given (x,y) position and find all linedefs that
	//       intersect it, choosing the one with the closest distance.
	//       If the point is sitting directly on a (two-sided) line,
	//       then we mark the sectors on both sides.

	for (i = 0; i < num_linedefs; i++)
	{
		glbsp_linedef_t *L = lev_linedefs[i];

		double_t x_cut;

		x1 = L->start->x;
		y1 = L->start->y;
		x2 = L->end->x;
		y2 = L->end->y;

		/* check vertical range */
		if (fabs(y2 - y1) < DIST_EPSILON)
			continue;

		if ((y > (y1 + DIST_EPSILON) && y > (y2 + DIST_EPSILON)) || (y < (y1 - DIST_EPSILON) && y < (y2 - DIST_EPSILON)))
			continue;

		x_cut = x1 + (x2 - x1) * (y - y1) / (y2 - y1) - x;

		if (fabs(x_cut) < fabs(best_dist))
		{
			/* found a closer linedef */

			best_match = L;
			best_dist = x_cut;
		}
	}

	if (!best_match)
	{
		PrintWarn("Bad polyobj thing at (%1.0f,%1.0f).\n", x, y);
		return;
	}

	y1 = best_match->start->y;
	y2 = best_match->end->y;

	/* check orientation of line, to determine which side the polyobj is
	 * actually on.
	 */
	if ((y1 > y2) == (best_dist > 0))
		sector = best_match->right ? best_match->right->sector : NULL;
	else
		sector = best_match->left ? best_match->left->sector : NULL;

	if (!sector)
	{
		PrintWarn("Invalid Polyobj thing at (%1.0f,%1.0f).\n", x, y);
		return;
	}

	MarkPolyobjSector(sector);
}

//
// DetectPolyobjSectors
//
// Based on code courtesy of Janis Legzdinsh.
//
void DetectPolyobjSectors(void)
{
	int i;
	int hexen_style;

	// -JL- There's a conflict between Hexen polyobj thing types and Doom thing
	//      types. In Doom type 3001 is for Imp and 3002 for Demon. To solve
	//      this problem, first we are going through all lines to see if the
	//      level has any polyobjs. If found, we also must detect what polyobj
	//      thing types are used - Hexen ones or ZDoom ones. That's why we
	//      are going through all things searching for ZDoom polyobj thing
	//      types. If any found, we assume that ZDoom polyobj thing types are
	//      used, otherwise Hexen polyobj thing types are used.

	// -JL- First go through all lines to see if level contains any polyobjs
	for (i = 0; i < num_linedefs; i++)
	{
		glbsp_linedef_t *L = lev_linedefs[i];

		if (L->type == HEXTYPE_POLY_START || L->type == HEXTYPE_POLY_EXPLICIT)
			break;
	}

	if (i == num_linedefs)
	{
		// -JL- No polyobjs in this level
		return;
	}

	// -JL- Detect what polyobj thing types are used - Hexen ones or ZDoom ones
	hexen_style = true;

	for (i = 0; i < num_things; i++)
	{
		glbsp_thing_t *T = LookupThing(i);

		if (T->type == ZDOOM_PO_SPAWN_TYPE || T->type == ZDOOM_PO_SPAWNCRUSH_TYPE)
		{
			// -JL- A ZDoom style polyobj thing found
			hexen_style = false;
			break;
		}
	}
	
	for (i = 0; i < num_things; i++)
	{
		glbsp_thing_t *T = LookupThing(i);

		double_t x = (double_t) T->x;
		double_t y = (double_t) T->y;

		// ignore everything except polyobj start spots
		if (hexen_style)
		{
			// -JL- Hexen style polyobj things
			if (T->type != PO_SPAWN_TYPE && T->type != PO_SPAWNCRUSH_TYPE)
				continue;
		}
		else
		{
			// -JL- ZDoom style polyobj things
			if (T->type != ZDOOM_PO_SPAWN_TYPE && T->type != ZDOOM_PO_SPAWNCRUSH_TYPE)
				continue;
		}

		MarkPolyobjPoint(x, y);
	}
}

/* ----- analysis routines ----------------------------- */

static int LineVertexLowest(const glbsp_linedef_t * L)
{
	// returns the "lowest" vertex (normally the left-most, but if the
	// line is vertical, then the bottom-most) => 0 for start, 1 for end.

	return ((int)L->start->x < (int)L->end->x || ((int)L->start->x == (int)L->end->x && (int)L->start->y < (int)L->end->y)) ? 0 : 1;
}

static int LineStartCompare(const void *p1, const void *p2)
{
	int line1 = ((const int *)p1)[0];
	int line2 = ((const int *)p2)[0];

	glbsp_linedef_t *A = lev_linedefs[line1];
	glbsp_linedef_t *B = lev_linedefs[line2];

	glbsp_vertex_t *C;
	glbsp_vertex_t *D;

	if (line1 == line2)
		return 0;

	// determine left-most vertex of each line
	C = LineVertexLowest(A) ? A->end : A->start;
	D = LineVertexLowest(B) ? B->end : B->start;

	if ((int)C->x != (int)D->x)
		return (int)C->x - (int)D->x;

	return (int)C->y - (int)D->y;
}

static int LineEndCompare(const void *p1, const void *p2)
{
	int line1 = ((const int *)p1)[0];
	int line2 = ((const int *)p2)[0];

	glbsp_linedef_t *A = lev_linedefs[line1];
	glbsp_linedef_t *B = lev_linedefs[line2];

	glbsp_vertex_t *C;
	glbsp_vertex_t *D;

	if (line1 == line2)
		return 0;

	// determine right-most vertex of each line
	C = LineVertexLowest(A) ? A->start : A->end;
	D = LineVertexLowest(B) ? B->start : B->end;

	if ((int)C->x != (int)D->x)
		return (int)C->x - (int)D->x;

	return (int)C->y - (int)D->y;
}

void DetectOverlappingLines(void)
{
	// Algorithm:
	//   Sort all lines by left-most vertex.
	//   Overlapping lines will then be near each other in this set.
	//   Note: does not detect partially overlapping lines.

	int i;
	int *array = UtilCalloc(num_linedefs * sizeof(int));
	int count = 0;

	DisplayTicker();

	// sort array of indices
	for (i = 0; i < num_linedefs; i++)
		array[i] = i;

	qsort(array, num_linedefs, sizeof(int), LineStartCompare);

	for (i = 0; i < num_linedefs - 1; i++)
	{
		int j;

		for (j = i + 1; j < num_linedefs; j++)
		{
			if (LineStartCompare(array + i, array + j) != 0)
				break;

			if (LineEndCompare(array + i, array + j) == 0)
			{
				glbsp_linedef_t *A = lev_linedefs[array[i]];
				glbsp_linedef_t *B = lev_linedefs[array[j]];

				// found an overlap !
				B->overlap = A->overlap ? A->overlap : A;

				count++;
			}
		}
	}

	if (count > 0)
	{
		PrintVerbose("Detected %d overlapped linedefs\n", count);
	}

	UtilFree(array);
}

static void CountWallTips(glbsp_vertex_t * vert, int *one_sided, int *two_sided)
{
	glbsp_wall_tip_t *tip;

	*one_sided = 0;
	*two_sided = 0;

	for (tip = vert->tip_set; tip; tip = tip->next)
	{
		if (!tip->left || !tip->right)
			(*one_sided) += 1;
		else
			(*two_sided) += 1;
	}
}

void TestForWindowEffect(glbsp_linedef_t * L)
{
	// cast a line horizontally or vertically and see what we hit.
	// OUCH, we have to iterate over all linedefs.

	int i;

	double_t mx = (L->start->x + L->end->x) / 2.0;
	double_t my = (L->start->y + L->end->y) / 2.0;

	double_t dx = L->end->x - L->start->x;
	double_t dy = L->end->y - L->start->y;

	int cast_horiz = fabs(dx) < fabs(dy) ? 1 : 0;

	double_t back_dist = 999999.0;
	glbsp_sector_t *back_open = NULL;
	int back_line = -1;

	double_t front_dist = 999999.0;
	glbsp_sector_t *front_open = NULL;
	int front_line = -1;

	for (i = 0; i < num_linedefs; i++)
	{
		glbsp_linedef_t *N = lev_linedefs[i];

		double_t dist;
		bool_t is_front;
		glbsp_sidedef_t *hit_side;

		double_t dx2, dy2;

		if (N == L || N->zero_len || N->overlap)
			continue;

		if (cast_horiz)
		{
			dx2 = N->end->x - N->start->x;
			dy2 = N->end->y - N->start->y;

			if (fabs(dy2) < DIST_EPSILON)
				continue;

			if ((MAX(N->start->y, N->end->y) < my - DIST_EPSILON) || (MIN(N->start->y, N->end->y) > my + DIST_EPSILON))
				continue;

			dist = (N->start->x + (my - N->start->y) * dx2 / dy2) - mx;

			is_front = ((dy > 0) == (dist > 0)) ? true : false;

			dist = fabs(dist);
			if (dist < DIST_EPSILON)					   // too close (overlapping lines ?)
				continue;

			hit_side = ((dy > 0) ^ (dy2 > 0) ^ !is_front) ? N->right : N->left;
		}
		else											   /* vert */
		{
			dx2 = N->end->x - N->start->x;
			dy2 = N->end->y - N->start->y;

			if (fabs(dx2) < DIST_EPSILON)
				continue;

			if ((MAX(N->start->x, N->end->x) < mx - DIST_EPSILON) || (MIN(N->start->x, N->end->x) > mx + DIST_EPSILON))
				continue;

			dist = (N->start->y + (mx - N->start->x) * dy2 / dx2) - my;

			is_front = ((dx > 0) != (dist > 0)) ? true : false;

			dist = fabs(dist);

			hit_side = ((dx > 0) ^ (dx2 > 0) ^ !is_front) ? N->right : N->left;
		}

		if (dist < DIST_EPSILON)						   // too close (overlapping lines ?)
			continue;

		if (is_front)
		{
			if (dist < front_dist)
			{
				front_dist = dist;
				front_open = hit_side ? hit_side->sector : NULL;
				front_line = i;
			}
		}
		else
		{
			if (dist < back_dist)
			{
				back_dist = dist;
				back_open = hit_side ? hit_side->sector : NULL;
				back_line = i;
			}
		}
	}

	if (back_open && front_open && L->right->sector == front_open)
	{
		L->window_effect = back_open;
		PrintMiniWarn("Linedef #%d seems to be a One-Sided Window (back faces sector #%d).\n", L->index, back_open->index);
	}
}

void DetectWindowEffects(void)
{
	// Algorithm:
	//   Scan the linedef list looking for possible candidates,
	//   checking for an odd number of one-sided linedefs connected
	//   to a single vertex.  This idea courtesy of Graham Jackson.

	int i;
	int one_siders;
	int two_siders;

	for (i = 0; i < num_linedefs; i++)
	{
		glbsp_linedef_t *L = lev_linedefs[i];

		if (L->two_sided || L->zero_len || L->overlap || !L->right)
			continue;

		CountWallTips(L->start, &one_siders, &two_siders);

		if ((one_siders % 2) == 1 && (one_siders + two_siders) > 1)
		{
			TestForWindowEffect(L);
			continue;
		}

		CountWallTips(L->end, &one_siders, &two_siders);

		if ((one_siders % 2) == 1 && (one_siders + two_siders) > 1)
		{
			TestForWindowEffect(L);
		}
	}
}

/* ----- vertex routines ------------------------------- */

static void VertexAddWallTip(glbsp_vertex_t * vert, double_t dx, double_t dy, glbsp_sector_t * left, glbsp_sector_t * right)
{
	glbsp_wall_tip_t *tip = NewWallTip();
	glbsp_wall_tip_t *after;

	tip->angle = UtilComputeAngle(dx, dy);
	tip->left = left;
	tip->right = right;

	// find the correct place (order is increasing angle)
	for (after = vert->tip_set; after && after->next; after = after->next)
	{
	}

	while(after && tip->angle + ANG_EPSILON < after->angle)
		after = after->prev;

	// link it in
	tip->next = after ? after->next : vert->tip_set;
	tip->prev = after;

	if (after)
	{
		if (after->next)
			after->next->prev = tip;

		after->next = tip;
	}
	else
	{
		if (vert->tip_set)
			vert->tip_set->prev = tip;

		vert->tip_set = tip;
	}
}

void CalculateWallTips(void)
{
	int i;

	DisplayTicker();

	for (i = 0; i < num_linedefs; i++)
	{
		glbsp_linedef_t *line = lev_linedefs[i];

		if (line->self_ref && cur_info->skip_self_ref)
			continue;

		double_t x1 = line->start->x;
		double_t y1 = line->start->y;
		double_t x2 = line->end->x;
		double_t y2 = line->end->y;

		glbsp_sector_t *left = (line->left) ? line->left->sector : NULL;
		glbsp_sector_t *right = (line->right) ? line->right->sector : NULL;

		VertexAddWallTip(line->start, x2 - x1, y2 - y1, left, right);
		VertexAddWallTip(line->end, x1 - x2, y1 - y2, right, left);
	}
}

//
// NewVertexFromSplitSeg
//
glbsp_vertex_t *NewVertexFromSplitSeg(glbsp_seg_t * seg, double_t x, double_t y)
{
	glbsp_vertex_t *vert = NewVertex();

	vert->x = x;
	vert->y = y;

	vert->ref_count = seg->partner ? 4 : 2;

	vert->index = num_gl_vert | IS_GL_VERTEX;
	num_gl_vert++;

	// compute wall_tip info

	VertexAddWallTip(vert, -seg->pdx, -seg->pdy, seg->sector, seg->partner ? seg->partner->sector : NULL);

	VertexAddWallTip(vert, seg->pdx, seg->pdy, seg->partner ? seg->partner->sector : NULL, seg->sector);

	return vert;
}

//
// VertexCheckOpen
//
glbsp_sector_t *VertexCheckOpen(glbsp_vertex_t * vert, double_t dx, double_t dy)
{
	glbsp_wall_tip_t *tip;

	double_t angle = UtilComputeAngle(dx, dy);

	// first check whether there's a wall_tip that lies in the exact
	// direction of the given direction (which is relative to the
	// vertex).

	for (tip = vert->tip_set; tip; tip = tip->next)
	{
		if (fabs(tip->angle - angle) < ANG_EPSILON || fabs(tip->angle - angle) > (360.0 - ANG_EPSILON))
		{
			// yes, found one
			return NULL;
		}
	}

	// OK, now just find the first wall_tip whose angle is greater than
	// the angle we're interested in.  Therefore we'll be on the RIGHT
	// side of that wall_tip.

	for (tip = vert->tip_set; tip; tip = tip->next)
	{
		if (angle + ANG_EPSILON < tip->angle)
		{
			// found it
			return tip->right;
		}

		if (!tip->next)
		{
			// no more tips, thus we must be on the LEFT side of the tip
			// with the largest angle.

			return tip->left;
		}
	}

	InternalError("Vertex %d has no tips !", vert->index);
	return false;
}
