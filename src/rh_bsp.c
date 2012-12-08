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
// Copyright (C) 1996-1998 Activision and Raven Software
// Copyright (C) 2012-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Heretic Renderer

/***************
*** INCLUDES ***
***************/

#include "doomdef.h"
#include "d_player.h"
#include "rh_main.h"
#include "m_bbox.h"
#include "r_state.h"
#include "r_sky.h"
#include "r_main.h"

/****************
*** CONSTANTS ***
****************/

// c_checkcoord -- Bounding box coordinate check
static const int c_checkcoord[12][4] =
{
	{3,0, 2,1},
	{3,0, 2,0},
	{3,1, 2,0},
	{0,0, 0,0},
	{2,0, 2,1},
	{0,0, 0,0},
	{3,1, 3,0},
	{0,0, 0,0},
	{2,0, 3,1},
	{2,1, 3,1},
	{2,1, 3,0}
};

/*****************
*** PROTOTYPES ***
*****************/

/****************
*** FUNCTIONS ***
****************/

/* RHS_CheckBBox() -- Checks whether a box is visible (to be drawn) */
static bool_t RHS_CheckBBox(RH_RenderStat_t* const a_Stat, fixed_t* const a_BSPBox)
{
	int BoxX, BoxY, BoxPos;
	fixed_t x1, y1, x2, y2;
	angle_t angle1, angle2;
	
	/* Find box edge */
	// X Plane
	if (a_Stat->viewx <= a_BSPBox[BOXLEFT])
		BoxX = 0;
	else if (a_Stat->viewx < a_BSPBox[BOXRIGHT])
		BoxX = 1;
	else
		BoxX = 2;
	
	// Y Plane
	if (a_Stat->viewy <= a_BSPBox[BOXTOP])
		BoxY = 0;
	else if (a_Stat->viewy < a_BSPBox[BOXBOTTOM])
		BoxY = 1;
	else
		BoxY = 2;
	
	/* Check Box Position */
	BoxPos = (BoxY << 2) + BoxX;
	
	// Visible?
	if (BoxPos == 5)
		return true;
	
	/* Check box more */
	x1 = a_BSPBox[c_checkcoord[BoxPos][0]];
	y1 = a_BSPBox[c_checkcoord[BoxPos][1]];
	x2 = a_BSPBox[c_checkcoord[BoxPos][2]];
	y2 = a_BSPBox[c_checkcoord[BoxPos][3]];
	
	/* Check for open space */
	
	/* Visible */
	return true;
	
#if defined(__DUPECODEHERE__)
	int			boxx, boxy, boxpos;
	fixed_t		x1, y1, x2, y2;
	angle_t		angle1, angle2, span, tspan;
	cliprange_t	*start;
	int			sx1, sx2;

//
// check clip list for an open space
//	
	angle1 = R_PointToAngle (x1, y1) - viewangle;
	angle2 = R_PointToAngle (x2, y2) - viewangle;
	
	span = angle1 - angle2;
	if (span >= ANG180)
		return true;	// sitting on a line
	tspan = angle1 + clipangle;
	if (tspan > 2*clipangle)
	{
		tspan -= 2*clipangle;
		if (tspan >= span)
			return false;	// totally off the left edge
		angle1 = clipangle;
	}
	tspan = clipangle - angle2;
	if (tspan > 2*clipangle)
	{
		tspan -= 2*clipangle;
		if (tspan >= span)
			return false;	// totally off the left edge
		angle2 = -clipangle;
	}


// find the first clippost that touches the source post (adjacent pixels are touching)
	angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
	angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;
	sx1 = viewangletox[angle1];
	sx2 = viewangletox[angle2];
	if (sx1 == sx2)
		return false;				// does not cross a pixel
	sx2--;
	
	start = solidsegs;
	while (start->last < sx2)
		start++;
	if (sx1 >= start->first && sx2 <= start->last)
		return false;	// the clippost contains the new span

	return true;
#endif
}

/* RH_ClipSolidWallSegment() -- Clip a solid wall */
void RH_ClipSolidWallSegment(RH_RenderStat_t* const a_Stat, seg_t* const a_Seg, const int a_First, const int a_Last)
{

#if defined(__DUPECODEHERE__)
	cliprange_t	*next, *start;

// find the first range that touches the range (adjacent pixels are touching)
	start = solidsegs;
	while (start->last < first-1)
		start++;

	if (first < start->first)
	{
		if (last < start->first-1)
		{	// post is entirely visible (above start), so insert a new clippost
			R_StoreWallRange (first, last);
			next = newend;
			newend++;
			while (next != start)
			{
				*next = *(next-1);
				next--;
			}
			next->first = first;
			next->last = last;
			return;
		}
		
	  // there is a fragment above *start
		R_StoreWallRange (first, start->first - 1);
		start->first = first;		// adjust the clip size
	}
	
	if (last <= start->last)
		return;			// bottom contained in start
		
	next = start;
	while (last >= (next+1)->first-1)
	{
		// there is a fragment between two posts
		R_StoreWallRange (next->last + 1, (next+1)->first - 1);
		next++;
		if (last <= next->last)
		{	// bottom is contained in next
			start->last = next->last;	// adjust the clip size
			goto crunch;
		}
	}
	
	// there is a fragment after *next
	R_StoreWallRange (next->last + 1, last);
	start->last = last;		// adjust the clip size
	
	
// remove start+1 to next from the clip list,
// because start now covers their area
crunch:
	if (next == start)
		return;			// post just extended past the bottom of one post

	while (next++ != newend)	// remove a post
		*++start = *next;
	newend = start+1;
#endif
}

/* RH_ClipPassWallSegment() -- Clip see-thru wall */
void RH_ClipPassWallSegment(RH_RenderStat_t* const a_Stat, seg_t* const a_Seg, const int a_First, const int a_Last)
{
}

/* RH_AddLine() -- Adds seg to be drawn */
void RH_AddLine(RH_RenderStat_t* const a_Stat, seg_t* const a_Seg)
{
	int x1, x2;
	angle_t angle1, angle2, span, tspan;
	
	/* Determine culling */
	// Get line angles
	angle1 = R_PointToAngle(a_Seg->v1->x, a_Seg->v1->y);
	angle2 = R_PointToAngle(a_Seg->v2->x, a_Seg->v2->y);
	
	// Clip to edges
	span = angle1 - angle2;
	
	// Backside
	if (span >= ANG180)
		return;
	
	// Angles
	a_Stat->rw_angle1 = angle1;
	angle1 -= a_Stat->viewangle;
	angle2 -= a_Stat->viewangle;
	
	// Clip some more
	tspan = angle1 + a_Stat->clipangle;
	if (tspan > 2 * a_Stat->clipangle)
	{
		tspan -= 2 * a_Stat->clipangle;
		
		if (tspan >= span)
			return;	// totally off the left edge
		
		angle1 = a_Stat->clipangle;
	}
	
	tspan = a_Stat->clipangle - angle2;
	if (tspan > 2 * a_Stat->clipangle)
	{
		tspan -= 2 * a_Stat->clipangle;
		
		if (tspan >= span)
			return;	// totally off the left edge
		
		angle2 = -a_Stat->clipangle;
	}
	
	/* Seg in view range but might not be visible */
	angle1 = (angle1 + ANG90) >> ANGLETOFINESHIFT;
	angle2 = (angle2 + ANG90) >> ANGLETOFINESHIFT;
	x1 = a_Stat->viewangletox[angle1];
	x2 = a_Stat->viewangletox[angle2];
	
	// Does not cross any pixels
	if (x1 == x2)
		return;
	
	// Get back sector
	a_Stat->backsector = a_Seg->backsector;
	
	// Back sector missing or floors higher
	if ((!a_Stat->backsector) || (a_Stat->backsector && (a_Stat->backsector->ceilingheight <= a_Stat->frontsector->floorheight || a_Stat->backsector->floorheight >= a_Stat->frontsector->ceilingheight)))
	{
		RH_ClipSolidWallSegment(a_Stat, a_Seg, x1, x2 - 1);
		return;
	}
	
	/* Clip as pass through wall */
	if (!(a_Stat->backsector->ceilingheight != a_Stat->frontsector->ceilingheight || a_Stat->backsector->floorheight != a_Stat->frontsector->floorheight))
	{
		// reject empty lines used for triggers and special events
		if (a_Stat->backsector->ceilingpic == a_Stat->frontsector->ceilingpic &&
			a_Stat->backsector->floorpic == a_Stat->frontsector->floorpic &&
			a_Stat->backsector->lightlevel == a_Stat->frontsector->lightlevel &&
			a_Seg->sidedef->midtexture == 0)
			return;
	}
	
	RH_ClipPassWallSegment(a_Stat, a_Seg, x1, x2 - 1);
}

/* RH_SubSector() -- Draws subsector */
void RH_SubSector(RH_RenderStat_t* const a_Stat, const int a_SubSNum)
{
	seg_t* Seg;
	subsector_t* SubS;
	int32_t i, n;
	RH_VisPlane_t Temp;
	
	/* Get subsector info */
	SubS = &subsectors[a_SubSNum];
	a_Stat->frontsector = SubS->sector;
	n = SubS->numlines;
	Seg = &segs[SubS->firstline];
	
	/* Render plane for subsector */
	// Floor
	a_Stat->floorplane = NULL;
	if (a_Stat->frontsector->floorheight < a_Stat->viewz)
	{
		// Fill info
		memset(&Temp, 0, sizeof(Temp));
		Temp.Height = a_Stat->frontsector->floorheight;
		Temp.PicNum = a_Stat->frontsector->floorpic;
		Temp.LightLevel = a_Stat->frontsector->lightlevel;
		Temp.Special = a_Stat->frontsector->special;
		
		// Find it
		a_Stat->floorplane = RH_FindPlane(a_Stat, &Temp);
	}
	
	// Ceiling
	a_Stat->ceilingplane = NULL;
	if (a_Stat->frontsector->ceilingheight > a_Stat->viewz ||
		a_Stat->frontsector->ceilingpic == skyflatnum)
	{
		// Fill info
		memset(&Temp, 0, sizeof(Temp));
		Temp.Height = a_Stat->frontsector->ceilingheight;
		Temp.PicNum = a_Stat->frontsector->ceilingpic;
		Temp.LightLevel = a_Stat->frontsector->lightlevel;
		Temp.Special = a_Stat->frontsector->special;
		
		// Find it
		a_Stat->ceilingplane = RH_FindPlane(a_Stat, &Temp);
	}
	
	/* Render Sprites */
	RH_AddSprites(a_Stat, SubS->sector);
	
	/* Render Segs */
	while (n-- > 0)
		RH_AddLine(a_Stat, Seg++);
}

/* RH_RenderBSPNode() -- Renders BSP Nodes */
void RH_RenderBSPNode(RH_RenderStat_t* const a_Stat, const int a_NodeNum)
{
	int Side;
	node_t* NodeP;
	
	/* Get Pointer */
	NodeP = &nodes[a_NodeNum];
	
	/* SubSector */
	if (a_NodeNum & NF_SUBSECTOR)
	{
		// NULL?
		if (a_NodeNum == -1)
			RH_SubSector(a_Stat, 0);
		
		// Otherwise
		else
			RH_SubSector(a_Stat, a_NodeNum & (~NF_SUBSECTOR));
	}
	
	/* Node */
	else
	{
		// Determine node side
		Side = R_PointOnSide(a_Stat->viewx, a_Stat->viewy, NodeP);
		
		// Divide the front space
		RH_RenderBSPNode(a_Stat, NodeP->children[Side]);
		
		// Possibly divide the back space
		Side ^= 1;
		if (RHS_CheckBBox(a_Stat, NodeP->bbox[Side]))
			RH_RenderBSPNode(a_Stat, NodeP->children[Side]);
	}
}

