//------------------------------------------------------------------------
// LEVEL : Level structure read/write functions.
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
//
//  ZDBSP format support based on code (C) 2002,2003 Randy Heit
// 
//------------------------------------------------------------------------

#include "w_wad.h"
#include "p_info.h"

#include "gbsp_ana.h"
#include "gbsp_blo.h"
#include "gbsp_lev.h"
#include "gbsp_nod.h"
#include "gbsp_rej.h"
#include "gbsp_seg.h"
#include "gbsp_str.h"

#define DEBUG_LOAD      0
#define DEBUG_BSP       0

#define ALLOC_BLKNUM  1024

// per-level variables

bool_t lev_doing_hexen;

static bool_t lev_force_v3;
static bool_t lev_force_v5;

#define LEVELARRAY(TYPE, BASEVAR, NUMVAR)  \
    TYPE ** BASEVAR = NULL;  \
    int NUMVAR = 0;

LEVELARRAY(glbsp_vertex_t, lev_vertices, num_vertices)
LEVELARRAY(glbsp_linedef_t, lev_linedefs, num_linedefs)
LEVELARRAY(glbsp_sidedef_t, lev_sidedefs, num_sidedefs) LEVELARRAY(glbsp_sector_t, lev_sectors, num_sectors) LEVELARRAY(glbsp_thing_t, lev_things, num_things)
static LEVELARRAY(glbsp_seg_t, segs, num_segs)
static LEVELARRAY(glbsp_subsec_t, subsecs, num_subsecs)
static LEVELARRAY(glbsp_glbsp_node_t, nodes, num_nodes)
static LEVELARRAY(glbsp_glbsp_node_t, stale_nodes, num_stale_nodes)
static LEVELARRAY(glbsp_wall_tip_t, wall_tips, num_wall_tips)
int num_normal_vert = 0;
int num_gl_vert = 0;
int num_complete_seg = 0;

/* ----- allocation routines ---------------------------- */

#define ALLIGATOR(TYPE, BASEVAR, NUMVAR)  \
{  \
  if ((NUMVAR % ALLOC_BLKNUM) == 0)  \
  {  \
    BASEVAR = UtilRealloc(BASEVAR, (NUMVAR + ALLOC_BLKNUM) *   \
        sizeof(TYPE *));  \
  }  \
  BASEVAR[NUMVAR] = (TYPE *) UtilCalloc(sizeof(TYPE));  \
  NUMVAR += 1;  \
  return BASEVAR[NUMVAR - 1];  \
}

glbsp_vertex_t *NewVertex(void) ALLIGATOR(glbsp_vertex_t, lev_vertices, num_vertices)
glbsp_linedef_t *NewLinedef(void) ALLIGATOR(glbsp_linedef_t, lev_linedefs, num_linedefs)
glbsp_sidedef_t *NewSidedef(void) ALLIGATOR(glbsp_sidedef_t, lev_sidedefs, num_sidedefs)
glbsp_sector_t *NewSector(void) ALLIGATOR(glbsp_sector_t, lev_sectors, num_sectors)
glbsp_thing_t *NewThing(void) ALLIGATOR(glbsp_thing_t, lev_things, num_things)
glbsp_seg_t *NewSeg(void) ALLIGATOR(glbsp_seg_t, segs, num_segs)
glbsp_subsec_t *NewSubsec(void) ALLIGATOR(glbsp_subsec_t, subsecs, num_subsecs)
glbsp_glbsp_node_t *NewNode(void) ALLIGATOR(glbsp_glbsp_node_t, nodes, num_nodes)
glbsp_glbsp_node_t *NewStaleNode(void) ALLIGATOR(glbsp_glbsp_node_t, stale_nodes, num_stale_nodes)
glbsp_wall_tip_t *NewWallTip(void) ALLIGATOR(glbsp_wall_tip_t, wall_tips, num_wall_tips)
/* ----- free routines ---------------------------- */
#define FREEMASON(TYPE, BASEVAR, NUMVAR)  \
{  \
  int i;  \
  for (i=0; i < NUMVAR; i++)  \
    UtilFree(BASEVAR[i]);  \
  if (BASEVAR)  \
    UtilFree(BASEVAR);  \
  BASEVAR = NULL; NUMVAR = 0;  \
}
void FreeVertices(void) FREEMASON(glbsp_vertex_t, lev_vertices, num_vertices)
void FreeLinedefs(void) FREEMASON(glbsp_linedef_t, lev_linedefs, num_linedefs)
void FreeSidedefs(void) FREEMASON(glbsp_sidedef_t, lev_sidedefs, num_sidedefs)
void FreeSectors(void) FREEMASON(glbsp_sector_t, lev_sectors, num_sectors)
void FreeThings(void) FREEMASON(glbsp_thing_t, lev_things, num_things)
void FreeSegs(void) FREEMASON(glbsp_seg_t, segs, num_segs)
void FreeSubsecs(void) FREEMASON(glbsp_subsec_t, subsecs, num_subsecs)
void FreeNodes(void) FREEMASON(glbsp_glbsp_node_t, nodes, num_nodes)
void FreeStaleNodes(void) FREEMASON(glbsp_glbsp_node_t, stale_nodes, num_stale_nodes)
void FreeWallTips(void) FREEMASON(glbsp_wall_tip_t, wall_tips, num_wall_tips)
/* ----- lookup routines ------------------------------ */
#define LOOKERUPPER(BASEVAR, NUMVAR, NAMESTR)  \
{  \
  if (index < 0 || index >= NUMVAR)  \
    FatalError("No such %s number #%d", NAMESTR, index);  \
    \
  return BASEVAR[index];  \
}
glbsp_vertex_t *LookupVertex(int index) LOOKERUPPER(lev_vertices, num_vertices, "vertex")
glbsp_linedef_t *LookupLinedef(int index) LOOKERUPPER(lev_linedefs, num_linedefs, "linedef")
glbsp_sidedef_t *LookupSidedef(int index) LOOKERUPPER(lev_sidedefs, num_sidedefs, "sidedef")
glbsp_sector_t *LookupSector(int index) LOOKERUPPER(lev_sectors, num_sectors, "sector")
glbsp_thing_t *LookupThing(int index) LOOKERUPPER(lev_things, num_things, "thing")
glbsp_subsec_t *LookupSubsec(int index) LOOKERUPPER(subsecs, num_subsecs, "subsector")
glbsp_glbsp_node_t *LookupStaleNode(int index) LOOKERUPPER(stale_nodes, num_stale_nodes, "stale_node")
/* ----- reading routines ------------------------------ */
//
// CheckForNormalNodes
//
int CheckForNormalNodes(void)
{
	// GhostlyDeath <December 25, 2012> -- Translated from glBSP to ReMooD	
	
	lump_t *lump;

	/* Note: an empty NODES lump can be valid */
	if (FindLevelLump("NODES") == NULL)
		return false;

	lump = FindLevelLump("SEGS");

	if (!lump || lump->length == 0 || CheckLevelLumpZero(lump))
		return false;

	lump = FindLevelLump("SSECTORS");

	if (!lump || lump->length == 0 || CheckLevelLumpZero(lump))
		return false;

	return true;
}

//
// GetVertices
//
void GetVertices(void)
{
	int i, count = -1;
	raw_glbsp_vertex_t *raw;
	lump_t *lump = FindLevelLump("VERTEXES");

	if (lump)
		count = lump->length / sizeof(raw_glbsp_vertex_t);

	DisplayTicker();

#if DEBUG_LOAD
#endif

	if (!lump || count == 0)
		FatalError("Couldn't find any Vertices");

	raw = (raw_glbsp_vertex_t *) lump->data;

	for (i = 0; i < count; i++, raw++)
	{
		glbsp_vertex_t *vert = NewVertex();

		vert->x = (double_t) SINT16(raw->x);
		vert->y = (double_t) SINT16(raw->y);

		vert->index = i;
	}

	num_normal_vert = num_vertices;
	num_gl_vert = 0;
	num_complete_seg = 0;
}

//
// GetSectors
//
void GetSectors(void)
{
	int i, count = -1;
	raw_glbsp_sector_t *raw;
	lump_t *lump = FindLevelLump("SECTORS");

	if (lump)
		count = lump->length / sizeof(raw_glbsp_sector_t);

	if (!lump || count == 0)
		FatalError("Couldn't find any Sectors");

	DisplayTicker();

#if DEBUG_LOAD
#endif

	raw = (raw_glbsp_sector_t *) lump->data;

	for (i = 0; i < count; i++, raw++)
	{
		glbsp_sector_t *sector = NewSector();

		sector->floor_h = SINT16(raw->floor_h);
		sector->ceil_h = SINT16(raw->ceil_h);

		memcpy(sector->floor_tex, raw->floor_tex, sizeof(sector->floor_tex));
		memcpy(sector->ceil_tex, raw->ceil_tex, sizeof(sector->ceil_tex));

		sector->light = UINT16(raw->light);
		sector->special = UINT16(raw->special);
		sector->tag = SINT16(raw->tag);

		sector->coalesce = (sector->tag >= 900 && sector->tag < 1000) ? true : false;

		/* sector indices never change */
		sector->index = i;

		sector->warned_facing = -1;

		/* Note: rej_* fields are handled completely in reject.c */
	}
}

//
// GetThings
//
void GetThings(void)
{
	int i, count = -1;
	raw_glbsp_thing_t *raw;
	lump_t *lump = FindLevelLump("THINGS");

	if (lump)
		count = lump->length / sizeof(raw_glbsp_thing_t);

	if (!lump || count == 0)
	{
		// Note: no error if no things exist, even though technically a map
		// will be unplayable without the player starts.
		PrintWarn("Couldn't find any Things!\n");
		return;
	}

	DisplayTicker();

#if DEBUG_LOAD
#endif

	raw = (raw_glbsp_thing_t *) lump->data;

	for (i = 0; i < count; i++, raw++)
	{
		glbsp_thing_t *thing = NewThing();

		thing->x = SINT16(raw->x);
		thing->y = SINT16(raw->y);

		thing->type = UINT16(raw->type);
		thing->options = UINT16(raw->options);

		thing->index = i;
	}
}

//
// GetThingsHexen
//
void GetThingsHexen(void)
{
	int i, count = -1;
	raw_hexen_glbsp_thing_t *raw;
	lump_t *lump = FindLevelLump("THINGS");

	if (lump)
		count = lump->length / sizeof(raw_hexen_glbsp_thing_t);

	if (!lump || count == 0)
	{
		// Note: no error if no things exist, even though technically a map
		// will be unplayable without the player starts.
		PrintWarn("Couldn't find any Things!\n");
		return;
	}

	DisplayTicker();

#if DEBUG_LOAD
#endif

	raw = (raw_hexen_glbsp_thing_t *) lump->data;

	for (i = 0; i < count; i++, raw++)
	{
		glbsp_thing_t *thing = NewThing();

		thing->x = SINT16(raw->x);
		thing->y = SINT16(raw->y);

		thing->type = UINT16(raw->type);
		thing->options = UINT16(raw->options);

		thing->index = i;
	}
}

//
// GetSidedefs
//
void GetSidedefs(void)
{
	int i, count = -1;
	raw_glbsp_sidedef_t *raw;
	lump_t *lump = FindLevelLump("SIDEDEFS");

	if (lump)
		count = lump->length / sizeof(raw_glbsp_sidedef_t);

	if (!lump || count == 0)
		FatalError("Couldn't find any Sidedefs");

	DisplayTicker();

#if DEBUG_LOAD
#endif

	raw = (raw_glbsp_sidedef_t *) lump->data;

	for (i = 0; i < count; i++, raw++)
	{
		glbsp_sidedef_t *side = NewSidedef();

		side->sector = (SINT16(raw->sector) == -1) ? NULL : LookupSector(UINT16(raw->sector));

		if (side->sector)
			side->sector->ref_count++;

		side->x_offset = SINT16(raw->x_offset);
		side->y_offset = SINT16(raw->y_offset);

		memcpy(side->upper_tex, raw->upper_tex, sizeof(side->upper_tex));
		memcpy(side->lower_tex, raw->lower_tex, sizeof(side->lower_tex));
		memcpy(side->mid_tex, raw->mid_tex, sizeof(side->mid_tex));

		/* sidedef indices never change */
		side->index = i;
	}
}

static glbsp_sidedef_t *SafeLookupSidedef(uint16_t num)
{
	if (num == 0xFFFF)
		return NULL;

	if ((int)num >= num_sidedefs && (int16_t) (num) < 0)
		return NULL;

	return LookupSidedef(num);
}

//
// GetLinedefs
//
void GetLinedefs(void)
{
	int i, count = -1;
	raw_glbsp_linedef_t *raw;
	lump_t *lump = FindLevelLump("LINEDEFS");

	if (lump)
		count = lump->length / sizeof(raw_glbsp_linedef_t);

	if (!lump || count == 0)
		FatalError("Couldn't find any Linedefs");

	DisplayTicker();

#if DEBUG_LOAD
#endif

	raw = (raw_glbsp_linedef_t *) lump->data;

	for (i = 0; i < count; i++, raw++)
	{
		glbsp_linedef_t *line;

		glbsp_vertex_t *start = LookupVertex(UINT16(raw->start));
		glbsp_vertex_t *end = LookupVertex(UINT16(raw->end));

		start->ref_count++;
		end->ref_count++;

		line = NewLinedef();

		line->start = start;
		line->end = end;

		/* check for zero-length line */
		line->zero_len = (fabs(start->x - end->x) < DIST_EPSILON) && (fabs(start->y - end->y) < DIST_EPSILON);

		line->flags = UINT16(raw->flags);
		line->type = UINT16(raw->type);
		line->tag = SINT16(raw->tag);

		line->two_sided = (line->flags & LINEFLAG_TWO_SIDED) ? true : false;
		line->is_precious = (line->tag >= 900 && line->tag < 1000) ? true : false;

		line->right = SafeLookupSidedef(UINT16(raw->sidedef1));
		line->left = SafeLookupSidedef(UINT16(raw->sidedef2));

		if (line->right)
		{
			line->right->ref_count++;
			line->right->on_special |= (line->type > 0) ? 1 : 0;
		}

		if (line->left)
		{
			line->left->ref_count++;
			line->left->on_special |= (line->type > 0) ? 1 : 0;
		}

		line->self_ref = (line->left && line->right && (line->left->sector == line->right->sector));

		line->index = i;
	}
}

//
// GetLinedefsHexen
//
void GetLinedefsHexen(void)
{
	int i, j, count = -1;
	raw_hexen_glbsp_linedef_t *raw;
	lump_t *lump = FindLevelLump("LINEDEFS");

	if (lump)
		count = lump->length / sizeof(raw_hexen_glbsp_linedef_t);

	if (!lump || count == 0)
		FatalError("Couldn't find any Linedefs");

	DisplayTicker();

#if DEBUG_LOAD
#endif

	raw = (raw_hexen_glbsp_linedef_t *) lump->data;

	for (i = 0; i < count; i++, raw++)
	{
		glbsp_linedef_t *line;

		glbsp_vertex_t *start = LookupVertex(UINT16(raw->start));
		glbsp_vertex_t *end = LookupVertex(UINT16(raw->end));

		start->ref_count++;
		end->ref_count++;

		line = NewLinedef();

		line->start = start;
		line->end = end;

		// check for zero-length line
		line->zero_len = (fabs(start->x - end->x) < DIST_EPSILON) && (fabs(start->y - end->y) < DIST_EPSILON);

		line->flags = UINT16(raw->flags);
		line->type = UINT8(raw->type);
		line->tag = 0;

		/* read specials */
		for (j = 0; j < 5; j++)
			line->specials[j] = UINT8(raw->specials[j]);

		// -JL- Added missing twosided flag handling that caused a broken reject
		line->two_sided = (line->flags & LINEFLAG_TWO_SIDED) ? true : false;

		line->right = SafeLookupSidedef(UINT16(raw->sidedef1));
		line->left = SafeLookupSidedef(UINT16(raw->sidedef2));

		// -JL- Added missing sidedef handling that caused all sidedefs to be pruned
		if (line->right)
		{
			line->right->ref_count++;
			line->right->on_special |= (line->type > 0) ? 1 : 0;
		}

		if (line->left)
		{
			line->left->ref_count++;
			line->left->on_special |= (line->type > 0) ? 1 : 0;
		}

		line->self_ref = (line->left && line->right && (line->left->sector == line->right->sector));

		line->index = i;
	}
}

//
// GetStaleNodes
//
void GetStaleNodes(void)
{
	int i, count = -1;
	raw_glbsp_glbsp_node_t *raw;
	lump_t *lump = FindLevelLump("NODES");

	if (lump)
		count = lump->length / sizeof(raw_glbsp_glbsp_node_t);

	if (!lump || count < 5)
		return;

	DisplayTicker();

#if DEBUG_LOAD
#endif

	raw = (raw_glbsp_glbsp_node_t *) lump->data;

	/* must allocate all the nodes beforehand, since they contain
	 * internal references to each other.
	 */
	for (i = 0; i < count; i++)
	{
		NewStaleNode();
	}

	for (i = 0; i < count; i++, raw++)
	{
		glbsp_glbsp_node_t *nd = LookupStaleNode(i);

		nd->x = SINT16(raw->x);
		nd->y = SINT16(raw->y);
		nd->dx = SINT16(raw->dx);
		nd->dy = SINT16(raw->dy);

		nd->index = i;

		/* Note: we ignore the subsector references */

		if ((UINT16(raw->right) & 0x8000U) == 0)
		{
			nd->r.node = LookupStaleNode(UINT16(raw->right));
		}

		if ((UINT16(raw->left) & 0x8000U) == 0)
		{
			nd->l.node = LookupStaleNode(UINT16(raw->left));
		}

		/* we also ignore the bounding boxes -- not needed */
	}
}

static int SegCompare(const void *p1, const void *p2)
{
	const glbsp_seg_t *A = ((const glbsp_seg_t **)p1)[0];
	const glbsp_seg_t *B = ((const glbsp_seg_t **)p2)[0];

	if (A->index < 0)
		InternalError("Seg %p never reached a subsector !", A);

	if (B->index < 0)
		InternalError("Seg %p never reached a subsector !", B);

	return (A->index - B->index);
}

/* ----- writing routines ------------------------------ */

static const uint8_t *lev_v2_magic = (uint8_t *) "gNd2";
static const uint8_t *lev_v3_magic = (uint8_t *) "gNd3";
static const uint8_t *lev_v5_magic = (uint8_t *) "gNd5";

void PutV2Vertices(int do_v5)
{
	int count, i;
	lump_t *lump;

	DisplayTicker();

	lump = CreateGLLump("GL_VERT");

	if (do_v5)
		AppendLevelLump(lump, lev_v5_magic, 4);
	else
		AppendLevelLump(lump, lev_v2_magic, 4);

	for (i = 0, count = 0; i < num_vertices; i++)
	{
		raw_v2_glbsp_vertex_t raw;
		glbsp_vertex_t *vert = lev_vertices[i];

		if (!(vert->index & IS_GL_VERTEX))
			continue;

		raw.x = SINT32((int)(vert->x * 65536.0));
		raw.y = SINT32((int)(vert->y * 65536.0));

		AppendLevelLump(lump, &raw, sizeof(raw));

		count++;
	}

	if (count != num_gl_vert)
		InternalError("PutV2Vertices miscounted (%d != %d)", count, num_gl_vert);

	if (count > 32767)
		MarkSoftFailure(LIMIT_GL_VERT);
}

static uint32_t VertexIndex32BitV5(const glbsp_vertex_t * v)
{
	if (v->index & IS_GL_VERTEX)
		return (uint32_t) ((v->index & ~IS_GL_VERTEX) | 0x80000000U);

	return (uint32_t) v->index;
}

void PutV3Segs(int do_v5)
{
	int i, count;
	lump_t *lump = CreateGLLump("GL_SEGS");

	if (!do_v5)
		AppendLevelLump(lump, lev_v3_magic, 4);

	DisplayTicker();

	// sort segs into ascending index
	qsort(segs, num_segs, sizeof(glbsp_seg_t *), SegCompare);

	for (i = 0, count = 0; i < num_segs; i++)
	{
		raw_v3_glbsp_seg_t raw;
		glbsp_seg_t *seg = segs[i];

		// ignore degenerate segs
		if (seg->degenerate)
			continue;

		if (do_v5)
		{
			raw.start = UINT32(VertexIndex32BitV5(seg->start));
			raw.end = UINT32(VertexIndex32BitV5(seg->end));
		}
		else
		{
			raw.start = UINT32(seg->start->index);
			raw.end = UINT32(seg->end->index);
		}

		raw.side = UINT16(seg->side);

		if (seg->linedef)
			raw.linedef = UINT16(seg->linedef->index);
		else
			raw.linedef = UINT16(0xFFFF);

		if (seg->partner)
			raw.partner = UINT32(seg->partner->index);
		else
			raw.partner = UINT32(0xFFFFFFFF);

		AppendLevelLump(lump, &raw, sizeof(raw));

		count++;

#if DEBUG_BSP
				   "(%1.1f,%1.1f) -> (%1.1f,%1.1f)\n", seg->index, UINT16(raw.linedef),
				   seg->side ? "L" : "R", UINT32(raw.partner), seg->start->x, seg->start->y, seg->end->x, seg->end->y);
#endif
	}

	if (count != num_complete_seg)
		InternalError("PutGLSegs miscounted (%d != %d)", count, num_complete_seg);
}

void PutV3Subsecs(int do_v5)
{
	int i;
	lump_t *lump;

	DisplayTicker();

	lump = CreateGLLump("GL_SSECT");

	if (!do_v5)
		AppendLevelLump(lump, lev_v3_magic, 4);

	for (i = 0; i < num_subsecs; i++)
	{
		raw_v3_glbsp_subsec_t raw;
		glbsp_subsec_t *sub = subsecs[i];

		raw.first = UINT32(sub->seg_list->index);
		raw.num = UINT32(sub->seg_count);

		AppendLevelLump(lump, &raw, sizeof(raw));

#if DEBUG_BSP
#endif
	}

	if (!do_v5 && num_subsecs > 32767)
		MarkHardFailure(LIMIT_GL_SSECT);
}

static int node_cur_index;

static void PutOneNode(glbsp_glbsp_node_t * node, lump_t * lump)
{
	raw_glbsp_glbsp_node_t raw;

	if (node->r.node)
		PutOneNode(node->r.node, lump);

	if (node->l.node)
		PutOneNode(node->l.node, lump);

	node->index = node_cur_index++;

	raw.x = SINT16(node->x);
	raw.y = SINT16(node->y);
	raw.dx = SINT16(node->dx / (node->too_long ? 2 : 1));
	raw.dy = SINT16(node->dy / (node->too_long ? 2 : 1));

	raw.b1.minx = SINT16(node->r.bounds.minx);
	raw.b1.miny = SINT16(node->r.bounds.miny);
	raw.b1.maxx = SINT16(node->r.bounds.maxx);
	raw.b1.maxy = SINT16(node->r.bounds.maxy);

	raw.b2.minx = SINT16(node->l.bounds.minx);
	raw.b2.miny = SINT16(node->l.bounds.miny);
	raw.b2.maxx = SINT16(node->l.bounds.maxx);
	raw.b2.maxy = SINT16(node->l.bounds.maxy);

	if (node->r.node)
		raw.right = UINT16(node->r.node->index);
	else if (node->r.subsec)
		raw.right = UINT16(node->r.subsec->index | 0x8000);
	else
		InternalError("Bad right child in node %d", node->index);

	if (node->l.node)
		raw.left = UINT16(node->l.node->index);
	else if (node->l.subsec)
		raw.left = UINT16(node->l.subsec->index | 0x8000);
	else
		InternalError("Bad left child in node %d", node->index);

	AppendLevelLump(lump, &raw, sizeof(raw));

#if DEBUG_BSP
			   "(%d,%d) -> (%d,%d)\n", node->index, UINT16(raw.left), UINT16(raw.right), node->x, node->y, node->x + node->dx, node->y + node->dy);
#endif
}

static void PutOneV5Node(glbsp_glbsp_node_t * node, lump_t * lump)
{
	raw_v5_glbsp_glbsp_node_t raw;

	if (node->r.node)
		PutOneV5Node(node->r.node, lump);

	if (node->l.node)
		PutOneV5Node(node->l.node, lump);

	node->index = node_cur_index++;

	raw.x = SINT16(node->x);
	raw.y = SINT16(node->y);
	raw.dx = SINT16(node->dx / (node->too_long ? 2 : 1));
	raw.dy = SINT16(node->dy / (node->too_long ? 2 : 1));

	raw.b1.minx = SINT16(node->r.bounds.minx);
	raw.b1.miny = SINT16(node->r.bounds.miny);
	raw.b1.maxx = SINT16(node->r.bounds.maxx);
	raw.b1.maxy = SINT16(node->r.bounds.maxy);

	raw.b2.minx = SINT16(node->l.bounds.minx);
	raw.b2.miny = SINT16(node->l.bounds.miny);
	raw.b2.maxx = SINT16(node->l.bounds.maxx);
	raw.b2.maxy = SINT16(node->l.bounds.maxy);

	if (node->r.node)
		raw.right = UINT32(node->r.node->index);
	else if (node->r.subsec)
		raw.right = UINT32(node->r.subsec->index | 0x80000000U);
	else
		InternalError("Bad right child in V5 node %d", node->index);

	if (node->l.node)
		raw.left = UINT32(node->l.node->index);
	else if (node->l.subsec)
		raw.left = UINT32(node->l.subsec->index | 0x80000000U);
	else
		InternalError("Bad left child in V5 node %d", node->index);

	AppendLevelLump(lump, &raw, sizeof(raw));

#if DEBUG_BSP
			   "(%d,%d) -> (%d,%d)\n", node->index, UINT32(raw.left), UINT32(raw.right), node->x, node->y, node->x + node->dx, node->y + node->dy);
#endif
}

void PutNodes(char *name, int do_gl, int do_v5, glbsp_glbsp_node_t * root)
{
	lump_t *lump;

	DisplayTicker();

	if (do_gl)
		lump = CreateGLLump(name);
	else
		lump = CreateLevelLump(name);

	node_cur_index = 0;

	if (root)
	{
		if (do_v5)
			PutOneV5Node(root, lump);
		else
			PutOneNode(root, lump);
	}

	if (node_cur_index != num_nodes)
		InternalError("PutNodes miscounted (%d != %d)", node_cur_index, num_nodes);

	if (!do_v5 && node_cur_index > 32767)
		MarkHardFailure(LIMIT_NODES);
}

/* ----- whole-level routines --------------------------- */

//
// LoadLevel
//
void LoadLevel(void)
{
	char *message;

	const char *level_name = GetLevelName();

	bool_t normal_exists = CheckForNormalNodes();

	// -JL- Identify Hexen mode by presence of BEHAVIOR lump
	lev_doing_hexen = (FindLevelLump("BEHAVIOR") != NULL);

	message = UtilFormat("Building GL nodes on %s%s", level_name, lev_doing_hexen ? " (Hexen)" : "");

	lev_doing_hexen |= cur_info->force_hexen;

	DisplaySetBarText(1, message);

	PrintVerbose("\n\n");
	PrintMsg("%s\n", message);
	PrintVerbose("\n");

	UtilFree(message);

	GetVertices();
	GetSectors();
	GetSidedefs();

	if (lev_doing_hexen)
	{
		GetLinedefsHexen();
		GetThingsHexen();
	}
	else
	{
		GetLinedefs();
		GetThings();
	}

	PrintVerbose("Loaded %d vertices, %d sectors, %d sides, %d lines, %d things\n", num_vertices, num_sectors, num_sidedefs, num_linedefs, num_things);

	if (cur_info->fast && normal_exists && num_sectors > 5 && num_linedefs > 100)
	{
		PrintVerbose("Using original nodes to speed things up\n");
		GetStaleNodes();
	}

	CalculateWallTips();

	if (lev_doing_hexen)
	{
		// -JL- Find sectors containing polyobjs
		DetectPolyobjSectors();
	}

	DetectOverlappingLines();

	if (cur_info->window_fx)
		DetectWindowEffects();
}

//
// FreeLevel
//
void FreeLevel(void)
{
	FreeVertices();
	FreeSidedefs();
	FreeLinedefs();
	FreeSectors();
	FreeThings();
	FreeSegs();
	FreeSubsecs();
	FreeNodes();
	FreeStaleNodes();
	FreeWallTips();
}

//
// PutGLOptions
//
void PutGLOptions(void)
{
	char option_buf[128];

	sprintf(option_buf, "-v%d -factor %d", 5, cur_info->factor);

	if (cur_info->fast)
		strcat(option_buf, " -f");
	if (cur_info->force_normal)
		strcat(option_buf, " -n");
	if (cur_info->merge_vert)
		strcat(option_buf, " -m");
	if (cur_info->pack_sides)
		strcat(option_buf, " -p");
	if (cur_info->prune_sect)
		strcat(option_buf, " -u");
	if (cur_info->skip_self_ref)
		strcat(option_buf, " -s");
	if (cur_info->window_fx)
		strcat(option_buf, " -y");

	if (cur_info->no_normal)
		strcat(option_buf, " -xn");
	if (cur_info->no_reject)
		strcat(option_buf, " -xr");
	if (cur_info->no_prune)
		strcat(option_buf, " -xu");

	AddGLTextLine("OPTIONS", option_buf);
}

//
// PutGLChecksum
//
void PutGLChecksum(void)
{
	uint32_t crc;
	lump_t *lump;
	char num_buf[64];

	Adler32_Begin(&crc);

	lump = FindLevelLump("VERTEXES");

	if (lump && lump->length > 0)
		Adler32_AddBlock(&crc, lump->data, lump->length);

	lump = FindLevelLump("LINEDEFS");

	if (lump && lump->length > 0)
		Adler32_AddBlock(&crc, lump->data, lump->length);

	Adler32_Finish(&crc);

	sprintf(num_buf, "0x%08x", crc);

	AddGLTextLine("CHECKSUM", num_buf);
}

//
// SaveLevel
//
void SaveLevel(glbsp_glbsp_node_t * root_node)
{
	lev_force_v3 = false;
	lev_force_v5 = true;

	// GL Nodes
	{
		PutV2Vertices(lev_force_v5);

		PutV3Segs(lev_force_v5);

		PutV3Subsecs(lev_force_v5);

		PutNodes("GL_NODES", true, lev_force_v5, root_node);

		// -JL- Add empty PVS lump
		CreateGLLump("GL_PVS");
	}

	// keyword support (v5.0 of the specs)
	AddGLTextLine("BUILDER", "glBSP " GLBSP_VER);
	PutGLOptions();
	{
		char *time_str = UtilTimeString();

		if (time_str)
		{
			AddGLTextLine("TIME", time_str);
			UtilFree(time_str);
		}
	}

	// this must be done _after_ the normal nodes have been built,
	// so that we use the new VERTEXES lump in the checksum.
	PutGLChecksum();
}
