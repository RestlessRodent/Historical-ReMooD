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
// Copyright (C) 2012-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Bot Node Related Code

/***************
*** INCLUDES ***
***************/

#include "b_priv.h"
#include "console.h"

/****************
*** FUNCTIONS ***
****************/

/* BS_PointsToAngleTurn() -- Convert points to angle turn */
uint16_t BS_PointsToAngleTurn(const fixed_t a_x1, const fixed_t a_y1, const fixed_t a_x2, const fixed_t a_y2)
{
	return R_PointToAngle2(a_x1, a_y1, a_x2, a_y2) >> 16;
}

/* BS_LinkAngleDir() -- Gets link angle for this angle */
void BS_LinkAngleDir(int32_t* const a_OutX, int32_t* const a_OutY, const angle_t a_Angle)
{
	/* Determine link angle */
	// East
	if (a_Angle >= ANGLEX(337) || a_Angle < ANGLEX(23))
	{
		*a_OutX = 1;
		*a_OutY = 0;
	}
	
	// North East
	else if (a_Angle >= ANGLEX(23) && a_Angle < ANGLEX(67))
	{
		*a_OutX = 1;
		*a_OutY = 1;
	}
	
	// North
	else if (a_Angle >= ANGLEX(67) && a_Angle < ANGLEX(113))
	{
		*a_OutX = 0;
		*a_OutY = 1;
	}
	
	// North West
	else if (a_Angle >= ANGLEX(113) && a_Angle < ANGLEX(158))
	{
		*a_OutX = -1;
		*a_OutY = 1;
	}
	
	// West
	else if (a_Angle >= ANGLEX(158) && a_Angle < ANGLEX(202))
	{
		*a_OutX = -1;
		*a_OutY = 0;
	}
	
	// South West
	else if (a_Angle >= ANGLEX(202) && a_Angle < ANGLEX(248))
	{
		*a_OutX = -1;
		*a_OutY = -1;
	}
	
	// South
	else if (a_Angle >= ANGLEX(248) && a_Angle < ANGLEX(293))
	{
		*a_OutX = 0;
		*a_OutY = -1;
	}
	
	// South East
	else if (a_Angle >= ANGLEX(293) && a_Angle < ANGLEX(337))
	{
		*a_OutX = 1;
		*a_OutY = -1;
	}
	
	// Oops
	else
	{
		*a_OutX = 0;
		*a_OutY = 0;
	}
}

/* BS_LinkDirection() -- Determines the link direction from these points */
void BS_LinkDirection(int32_t* const a_OutX, int32_t* const a_OutY, const fixed_t a_X1, const fixed_t a_Y1, const fixed_t a_X2, const fixed_t a_Y2)
{
	BS_LinkAngleDir(a_OutX, a_OutY, R_PointToAngle2(a_X1, a_Y1, a_X2, a_Y2));
}

/* BS_NearLinkDir() -- Determines if link directions are near enough */
// i.e north east is near to east, but not to south east
bool_t BS_NearLinkDir(const int32_t a_X1, const int32_t a_Y1, const int32_t a_X2, const int32_t a_Y2)
{
	static const struct
	{
		int32_t SetA[2];
		int32_t SetB[2];
		int32_t Pts[2];
	} c_NM[9] =
	{
		{{-1,  0}, { 0, -1}, {-1, -1}},
		{{-1, -1}, { 1, -1}, { 0, -1}},
		{{ 0, -1}, { 1,  0}, { 1, -1}},
		{{-1,  1}, {-1, -1}, {-1,  0}},
		{{ 0,  0}, { 0,  0}, { 0,  0}},
		{{ 1, -1}, { 1,  1}, { 1,  0}},
		{{-1,  0}, { 0,  1}, {-1,  1}},
		{{-1,  1}, { 1,  1}, { 0,  1}},
		{{ 0,  1}, { 1,  0}, { 1,  1}},
	};
	
	int32_t i;
	
	/* Do not allow (0,0) */
	if ((a_X1 == 0 && a_Y1 == 0) || (a_X2 == 0 && a_Y2 == 0))
		return false;
	
	/* Look in list */
	for (i = 0; i < 9; i++)
		if (a_X1 == c_NM[i].Pts[0] && a_Y1 == c_NM[i].Pts[1])
		{
			// Matches set a?
			if (a_X2 == c_NM[i].SetA[0] && a_Y2 == c_NM[i].SetA[1])
				return true;
			
			// Matches set b?
			if (a_X2 == c_NM[i].SetB[0] && a_Y2 == c_NM[i].SetB[1])
				return true;
		}
	
	/* Presume not a match */
	return false;
}

/* BS_MoveToAndAimAtFrom() -- Aim at target and move to one at the same time */
void BS_MoveToAndAimAtFrom(const fixed_t a_x1, const fixed_t a_y1, const fixed_t a_x2, const fixed_t a_y2, const fixed_t a_AimX, const fixed_t a_AimY, int16_t* const a_AngleTurn, int8_t* const a_Forward, int8_t* const a_Side)
{
	angle_t AimAng;
	angle_t MoveAng, DiffAng;
	
	/* Determine all angles */
	AimAng = R_PointToAngle2(a_x1, a_y1, a_AimX, a_AimY);
	MoveAng = R_PointToAngle2(a_x1, a_y1, a_x2, a_y2);
	DiffAng = MoveAng - AimAng;
	
	/* Do aiming first */
	*a_AngleTurn = AimAng >> 16;
	
	/* Then calculate side movements and such */
	*a_Forward = FixedMul(
		((fixed_t)c_forwardmove[1]) << FRACBITS,
		finecosine[DiffAng >> ANGLETOFINESHIFT]) >> FRACBITS;
	*a_Side = -(FixedMul(
		((fixed_t)c_forwardmove[1]) << FRACBITS,
		finesine[(DiffAng) >> ANGLETOFINESHIFT]) >> FRACBITS);
}

/* BS_UnimatrixAtPos() -- Returns Unimatrix at this position */
B_Unimatrix_t* BS_UnimatrixAtPos(const fixed_t a_X, const fixed_t a_Y)
{
	fixed_t RealX, RealY;
	int32_t Location;
	
	/* Fix Coordinate */
	RealX = a_X - l_UMBase[0];
	RealY = a_Y - l_UMBase[1];
	
	/* Shrink to unimatrix base */
	RealX = FixedDiv(RealX, UNIMATRIXPHYSSIZE);
	RealY = FixedDiv(RealY, UNIMATRIXPHYSSIZE);
	
	// Shift Down
	RealX >>= FRACBITS;
	RealY >>= FRACBITS;
	
	/* Out of bounds? */
	if (RealX < 0 || RealX >= l_UMSize[0] || RealY < 0 || RealY >= l_UMSize[1])
		return NULL;
	
	/* Determine location */
	// Get location
	Location = (RealY * l_UMSize[0]) + RealX;
	
	/* Return it */
	return &l_UMGrid[Location];
}

/* BS_GHOSTCheckNodeTrav() -- Checks whether the node is in a "good" spot */
bool_t BS_GHOSTCheckNodeTrav(intercept_t* in, void* const a_Data)
{
	line_t* li;
	mobj_t* mo;
	
	/* Lines */
	if (in->isaline)
	{
		// Get line
		li = in->d.line;
		
		// Cannot cross two sided line
		if (!(li->flags & ML_TWOSIDED))
			return false;
		
		// Cannot cross impassible line
		if (li->flags & ML_BLOCKING)
			return false;
			
		// Get sector floor and ceiling z
		if (li->frontsector->floorheight > l_GFloorZ)
			l_GFloorZ = li->frontsector->floorheight;
		if (li->backsector->floorheight > l_GFloorZ)
			l_GFloorZ = li->backsector->floorheight;
		
		if (li->frontsector->ceilingheight > l_GCeilingZ)
			l_GCeilingZ = li->frontsector->ceilingheight;
		if (li->backsector->ceilingheight > l_GCeilingZ)
			l_GCeilingZ = li->backsector->ceilingheight;
		
		// Cannot fit inside of sector?
		if ((l_GCeilingZ - l_GFloorZ) < (56 << FRACBITS))
			return false;
		
		// Everything seems OK
		return true;
	}
	
	/* Things */
	else
	{
		// Get Object
		mo = in->d.thing;
		
		/* Don't hit solid things */
		if (mo->flags & (MF_SOLID))
			return false;
		
		return true;
	}
}

/* B_GHOST_NodeNearPos() -- Get node near position */
B_GhostNode_t* B_GHOST_NodeNearPos(const fixed_t a_X, const fixed_t a_Y, const fixed_t a_Z, const bool_t a_Any)
{
	B_Unimatrix_t* UniMatrix;
	B_GhostNode_t* CurrentNode, *Best;
	fixed_t Dist, ZDist, BestDist, BestZDist;
	size_t i;
	fixed_t Z;
	
	/* Get Unimatrix here */
	UniMatrix = BS_UnimatrixAtPos(a_X, a_Y);
	
	/* Nothing here? */
	if (!UniMatrix)
		return NULL;
	
	/* On Floor? */
	Z = a_Z;
	if (a_Z == ONFLOORZ)
		Z = R_PointInSubsector(a_X, a_Y)->sector->floorheight;
	
	/* Look through nodes there */
	// Clear best
	Best = NULL;
	BestDist = 0;
	
	// Go through all nodes here
	for (i = 0; i < UniMatrix->NumNodes; i++)
	{
		// Get current node
		CurrentNode = UniMatrix->Nodes[i];
		
		// Get distance
		Dist = P_AproxDistance(a_X - CurrentNode->x, a_Y - CurrentNode->y);
		
		// Too high?
		if (!a_Any)
			if (CurrentNode->FloorZ > (Z + (24 >> FRACBITS)))
				continue;
		
		// Distance is close (and is in stepping range)
		if (Dist < BOTMINNODEDIST)
			return CurrentNode;
		
		// Better?
		if (!Best || (Best && Dist < BestDist))
		{
			Best = CurrentNode;
			BestDist = Dist;
		}
	}
	
	/* None Found */
	// Return the best candidate
	if (a_Any)
		return Best;
	else
		return NULL;
}

/* B_GHOST_CreateNodeAtPos() -- Creates node at point */
B_GhostNode_t* B_GHOST_CreateNodeAtPos(const fixed_t a_X, const fixed_t a_Y)
{
	B_GhostNode_t* New;
	subsector_t* SubS;
	B_GhostNode_t* Rover;
	size_t i;
	fixed_t RealX, RealY, Dist;
	bool_t Failed;
	B_Unimatrix_t* ThisMatrix;
	B_GhostNode_t* CurrentNode, *NearNode;
	
	/* Init */
	RealX = a_X;
	RealY = a_Y;
	
	/* Truncate the coordinates to a 32-grid */
	//RealX &= ~2097151;//~1048575;
	//RealY &= ~2097151;//~1048575;
	RealX &= ~0xFFFF;
	RealY &= ~0xFFFF;
	
	/* See if there is a subsector here */
	// Because this spot could be in the middle of nowhere
	SubS = R_IsPointInSubsector(RealX, RealY);
	
	// If nothing is there, forget it
	if (!SubS)
		return NULL;
	
	// Don't add any nodes that are close to this spot
	NearNode = B_GHOST_NodeNearPos(RealX, RealY, SubS->sector->floorheight, false);
	
	// There was something close by
	if (NearNode)
		return NULL;
	
	/* Get Unimatrix */
	ThisMatrix = BS_UnimatrixAtPos(RealX, RealY);
	
	// No matrix?
	if (!ThisMatrix)
		return NULL;
	
	/* Determine Node Validity */
	// Init
	l_GFloorZ = SubS->sector->floorheight;
	l_GCeilingZ = SubS->sector->ceilingheight;
	Failed = false;
	
	// See if it is possible to even create a node here
	// The bots will blindly run to a node it cannot reach because
	// there happens to be a wall where the node is directly attached
	// to it.
	if (!Failed && !P_PathTraverse(
				RealX - (BOTMINNODEDIST >> 1),
				RealY - (BOTMINNODEDIST >> 1),
				RealX + (BOTMINNODEDIST >> 1),
				RealY + (BOTMINNODEDIST >> 1),
				PT_ADDLINES,
				BS_GHOSTCheckNodeTrav,
				NULL
			))
		Failed = true;
	
	// Draw line from thing corner (corss section TL to BR)
	if (!Failed && !P_PathTraverse(
				RealX - (BOTMINNODEDIST >> 1),
				RealY + (BOTMINNODEDIST >> 1),
				RealX + (BOTMINNODEDIST >> 1),
				RealY - (BOTMINNODEDIST >> 1),
				PT_ADDLINES,
				BS_GHOSTCheckNodeTrav,
				NULL
			))
		Failed = true;
	
	/* Failed to worked? */
	if (Failed)
		return NULL;
	
	/* Create new node */
	New = Z_Malloc(sizeof(*New), PU_BOTS, NULL);
	
	// Init
	New->x = RealX;
	New->y = RealY;
	New->FloorZ = l_GFloorZ;
	New->CeilingZ = l_GCeilingZ;
	New->SubS = SubS;
	
	/* Add to subsector links */
	Z_ResizeArray((void**)&SubS->GhostNodes, sizeof(*SubS->GhostNodes),
		SubS->NumGhostNodes, SubS->NumGhostNodes + 1);
	SubS->GhostNodes[SubS->NumGhostNodes++] = New;
	
	/* Add everything to the unimatrix */
	// Add subsector to unimatrix
	for (i = 0; i < ThisMatrix->NumSubSecs; i++)
		if (ThisMatrix->SubSecs[i] == SubS)
			break;
	
	if (i >= ThisMatrix->NumSubSecs)
	{
		Z_ResizeArray((void**)&ThisMatrix->SubSecs, sizeof(*ThisMatrix->SubSecs),
				ThisMatrix->NumSubSecs, ThisMatrix->NumSubSecs + 1);
		ThisMatrix->SubSecs[ThisMatrix->NumSubSecs++] = SubS;
	}
	
	// Add sector to unimatrix
	for (i = 0; i < ThisMatrix->NumSectors; i++)
		if (ThisMatrix->Sectors[i] == SubS->sector)
			break;
	
	if (i >= ThisMatrix->NumSectors)
	{
		Z_ResizeArray((void**)&ThisMatrix->Sectors, sizeof(*ThisMatrix->Sectors),
				ThisMatrix->NumSectors, ThisMatrix->NumSectors + 1);
		ThisMatrix->Sectors[ThisMatrix->NumSectors++] = SubS->sector;
	}
	
	// Add node to unimatrix
	Z_ResizeArray((void**)&ThisMatrix->Nodes, sizeof(*ThisMatrix->Nodes),
			ThisMatrix->NumNodes, ThisMatrix->NumNodes + 1);
	ThisMatrix->Nodes[ThisMatrix->NumNodes++] = New;
	
	/* Add to master chain */
	if (!l_HeadNode)
		l_HeadNode = New;
	else
	{
		New->Next = l_HeadNode;
		l_HeadNode = New;
	}
	
	/* Return the new node */
	return New;
}

/* B_GHOST_PolygonSplitLevel() -- Splits level into polygons */
bool_t B_GHOST_PolygonSplitLevel(const node_t* const a_Node)
{
	return false;
}

/* B_GHOST_RecursiveSplitMap() -- Recursively split the map */
// Normally this would have been for subsector shapes, but that is too much
// of a pain in the ass to do. I cannot seem to get it working at all, so i'm
// giving up on that and instead doing bounding boxes for navigation. It is
// shittier and less precise but it is hell of alot easier to implement!
// Although this is shitter, it is alot faster since people will probably not
// want to wait 5 minutes for polygonal meshes to build. If the map isn't that
// square it will be less accurate. So on levels such as MAP02 it can work good
// but on some very curvy levels, maybe not so much.
bool_t B_GHOST_RecursiveSplitMap(const node_t* const a_Node)
{
	int32_t Side, i, j, k;
	fixed_t x1, y1, x2, y2;
	
	fixed_t* px, *py;
	fixed_t cx, cy, dx, dy, ex, ey;
	subsector_t* SubS;
	
	/* Check */
	if (!a_Node)
		return false;
	
	/* Go through sides */
	for (i = 0; i < 2; i++)
	{
		// Side is a subsector
		if (a_Node->children[i] & NF_SUBSECTOR)
		{
			// Get bounds for subsector
				// A bounding box consists of four short values (top, bottom,
				// left and right) giving the upper and lower bounds of the y
				// coordinate and the lower and upper bounds of the x coordinate
				// (in that order).
			x1 = a_Node->bbox[i][2];
			y1 = a_Node->bbox[i][1];
			x2 = a_Node->bbox[i][3];
			y2 = a_Node->bbox[i][0];
			
			// Get the center of that box (easy to get)
			cx = FixedDiv(x1 + x2, 2 << FRACBITS);
			cy = FixedDiv(y1 + y2, 2 << FRACBITS);
			
			// Create a node there
			B_GHOST_CreateNodeAtPos(cx, cy);
			
#define MESHNESS 4
			// Create nodes in a 4x4 pattern scaled within the bo
			cx = (x2 - x1) / (MESHNESS + 2);
			cy = (y2 - y1) / (MESHNESS + 2);
			
			for (j = 1; j <= MESHNESS; j++)
			{
				dx = x1 + (cx * j);
				
				for (k = 1; k <= MESHNESS; k++)
				{
					dy = y1 + (cy * j);
					
					B_GHOST_CreateNodeAtPos(dx, dy);
				}
			}
			
			// Go through each 4 corners and midpoint that and the center
			// this creates a web of sorts
			for (j = 0; j < 4; j++)
			{
				// Which now?
				if ((j & 1) == 0)
					px = &x1;
				else
					px = &x2;
				if (j < 2)
					py = &y1;
				else
					py = &y2;
				
				// Get the midpoint of those between the center
				dx = FixedDiv(*px + cx, 2 << FRACBITS);
				dy = FixedDiv(*py + cy, 2 << FRACBITS);
				
				// Create node there
				B_GHOST_CreateNodeAtPos(dx, dy);
			}
		}
		
		// Side is another node
		else
		{
			// Just recourse into it
			B_GHOST_RecursiveSplitMap(&nodes[a_Node->children[i]]);
		}
	}
	
	/* Success */
	return true;
}

/* BS_GhostNTNPFirst() -- Helps determine whether point is reachable */
// This is done for the initial node creation and as such it also determines
// if a switch or line trigger is needed for activation.
bool_t BS_GhostNTNPFirst(intercept_t* in, void* const a_Data)
{
	line_t* li;
	
	/* A Line */
	if (in->isaline)
	{
		// Get line
		li = in->d.line;
		
		// Cannot cross two sided line
		if (!(li->flags & ML_TWOSIDED))
			return false;
		
		// Cannot cross impassible line
		if (li->flags & ML_BLOCKING)
			return false;
		
		// Cannot fit inside of sector
			// Front Side
		if ((li->frontsector->ceilingheight - li->frontsector->floorheight) < (56 << FRACBITS))
			return false;
			// Back Side
		if ((li->backsector->ceilingheight - li->backsector->floorheight) < (56 << FRACBITS))
			return false;
		
		// Everything seems OK
		return true;
	}
	
	/* A Thing */
	else
	{
		// Things are checked later
		return false;
	}
}

/* BS_CheckNodeToNode() -- Checks whether a node can be traveled to */
bool_t BS_CheckNodeToNode(B_GhostBot_t* const a_Bot, B_GhostNode_t* const a_Start, B_GhostNode_t* const a_End, const bool_t a_FirstTime)
{
	/* Check */
	if (!a_Start || !a_End || (!a_FirstTime && !a_Bot))
		return false;
	
	/* First Time Determination? */
	// This one takes awhile
	if (a_FirstTime)
	{
		// Quick Height Determination
			// Too big a step up
		if (a_End->FloorZ > a_Start->FloorZ + (24 << FRACBITS))
			return false;
			
			// Ceiling height incompatible
		else if ((a_Start->CeilingZ - a_End->FloorZ < (56 << FRACBITS)) ||
				(a_End->CeilingZ - a_Start->FloorZ < (56 << FRACBITS)))
			return false;
		
		// Draw line to node (slower)
		if (!P_PathTraverse(
				a_Start->x,
				a_Start->y,
				a_End->x,
				a_End->y,
				PT_ADDLINES,
				BS_GhostNTNPFirst,
				NULL
			))
			return false;
		
		// Reachable
		return true;
	}
	
	/* Other times */
	else
	{
		return true;
	}
}

/* BS_GHOST_BuildLinks() -- Builds links for the specified subsector */
void BS_GHOST_BuildLinks(const int32_t a_SubSNum)
{
	int32_t s, dX, dY, aR;
	B_GhostNode_t* ThisNode, *OtherNode;
	subsector_t* SubS, *OtherSS;
	fixed_t Dist;
	
	struct
	{
		B_GhostNode_t* Node;
		fixed_t Dist;
	} Best[9];
	
	/* Debug */
	if (g_BotDebug)
		CONL_PrintF("Building links for subs %i (total runs %i)\n", a_SubSNum, numsubsectors * numsubsectors);
	
	/* Get info for this subsector */
	// Refernece
	SubS = &subsectors[a_SubSNum];
	
	// Invalid polygon?
	if (!SubS->PolyValid)
		return;
	
	// Obtain the remaining info
	ThisNode = B_GHOST_NodeNearPos(SubS->CenterX, SubS->CenterY, ONFLOORZ, true);
	
	// No node?
	if (!ThisNode)
		return;
	
	/* Initialize */
	memset(Best, 0, sizeof(Best));
	
	/* Go through other subsectors */
	for (s = 0; s < numsubsectors; s++)
	{
		OtherSS = &subsectors[s];
		
		// Ignore the same subsector
		if (s == a_SubSNum)
			continue;
		
		// Ignore invalid polygons
		if (!OtherSS->PolyValid)
			continue;
		
		// Get node for that subsector
		OtherNode = B_GHOST_NodeNearPos(OtherSS->CenterX, OtherSS->CenterY, ONFLOORZ, true);
		
		// No Node?
		if (!OtherNode)
			continue;
		
		// See if traversal is possible
		if (!BS_CheckNodeToNode(NULL, ThisNode, OtherNode, true))
			continue;
		
		// It is, so obtain the logical direction to that node
		dX = dY = 0;
		BS_LinkDirection(&dX, &dY, ThisNode->x, ThisNode->y, OtherNode->x, OtherNode->y);
		
		// Ignore (0,0)
		if (dX == 0 && dY == 0)
			continue;
		
		// Make it array referenceable
		dX += 1;
		dY += 1;
		aR = (dY * 3) + dX;
		
		// Get distance to target
		Dist = P_AproxDistance(ThisNode->x - OtherNode->x, ThisNode->y - OtherNode->y);
		
		// Better connection?
		if (!Best[aR].Node || (Best[aR].Node && Dist < Best[aR].Dist))
		{
			Best[aR].Node = OtherNode;
			Best[aR].Dist = Dist;
		}
	}
	
	/* Hopefully obtained all the best links */
	for (s = 0; s < 9; s++)
	{
		// Ignore if no node set
		if (!Best[s].Node)
			continue;
		
		// Convert from array base back to double ref
		dX = s % 3;
		dY = s / 3;
		
		// Set link to that node
		ThisNode->Links[dX][dY].Node = Best[s].Node;
		ThisNode->Links[dX][dY].Dist = Best[s].Dist;
	}
}

/* B_GHOST_Ticker() -- Bot Ticker */
void B_GHOST_Ticker(void)
{
	int32_t i, j, k, zz;
	sector_t* CurSec;
	bool_t DoOK, DoStop;
	sector_t* (*BAdj)[MAXBGADJDEPTH] = NULL;
	sector_t* SecRoverA, *SecRoverB;
	size_t* BNumAdj = NULL;
	B_Unimatrix_t* UniMatrix;
	B_GhostNode_t* CurrentNode, *NearNode;
	
	fixed_t x, y, z, dx, dy;
	int32_t uX, uY;
	int8_t lox, loy;
	
	/* No Bots? */
	if (!g_GotBots)
		return;
		
	/* Do not build bot data if not the server */
	if (!D_XNetIsServer())
		return;
		
	/* If everything is done, don't bother */
	if (l_SSAllDone)
		return;
	
	/* Adjanceny chains not yet complete? */
	if (l_BBuildAdj < l_BNumSecs)
	{
		// Debug
		if (g_BotDebug)
			CONL_PrintF("GHOSTBOT: Building chains for %u\n", (unsigned)l_BBuildAdj);
		
		// Build chains
		CurSec = &sectors[l_BBuildAdj];
		BAdj = &l_BAdj[l_BBuildAdj];
		BNumAdj = &l_BNumAdj[l_BBuildAdj];
		
		// Init
		(*BAdj)[0] = CurSec;
		(*BNumAdj) = 1;
		DoStop = false;
		DoOK = true;
		k = 0;
		
		// Loop
		while (k < (*BNumAdj))
		{
			// Go through the "last" check sector
			SecRoverA = (*BAdj)[k++];
	
			// Reset
			DoStop = true;
	
			// Look in list
			if (SecRoverA)
				for (i = 0; i < SecRoverA->NumAdj; i++)
				{
					// Get rover at this pos
					SecRoverB = SecRoverA->Adj[i];
	
					// Make sure it isn't already in our queue
					for (j = 0; j < (*BNumAdj); j++)
						if (SecRoverB == (*BAdj)[j])
							break;
	
					// it wasn't so add it
					if (j >= (*BNumAdj))
					{
						if ((*BNumAdj) < MAXBGADJDEPTH - 1)
							(*BAdj)[(*BNumAdj)++] = SecRoverB;
					}
				}
		}
		
		// Go to the next one, next time
		l_BBuildAdj++;
		return;
	}
	
	/* Build unimatrix */
	// Do 20 at a time (since there are lots of unimatrixes)
	if (l_UMBuild < (l_UMSize[0] * l_UMSize[1]))
	{
		// Debug
		if (g_BotDebug)
			CONL_PrintF("GHOSTBOT: Building unimatrix %u\n", (unsigned)l_UMBuild);
		
		for (zz = 0; zz < 20; zz++)
			// Build them
			if (l_UMBuild < (l_UMSize[0] * l_UMSize[1]))
			{
				// Get Current
				UniMatrix = &l_UMGrid[l_UMBuild];
				
				// Determine which unimatrix this is locationally speaking
				uX = l_UMBuild % l_UMSize[0];
				uY = l_UMBuild / l_UMSize[0];
				
				// Determine Unimatrix Position on the map
				UniMatrix->StartPos[0] = FixedMul(uX << FRACBITS, UNIMATRIXPHYSSIZE) + l_UMBase[0];
				UniMatrix->StartPos[1] = FixedMul(uY << FRACBITS, UNIMATRIXPHYSSIZE) + l_UMBase[1];
				
				// Determine end position
				UniMatrix->EndPos[0] = UniMatrix->StartPos[0] + UNIMATRIXPHYSSIZE;
				UniMatrix->EndPos[1] = UniMatrix->StartPos[1] + UNIMATRIXPHYSSIZE;
				
				// Go to the next one, next time
				l_UMBuild++;
			}
			
		// Return for later processing
		return;
	}
	
	/* Build SubSector Mesh Map */
	if (!l_SSMCreated)
	{
#if 1
		// Polygonize the level
		SN_PolygonizeLevel();
		
		// Map nodes from polygons
#elif 0
		// Brute force
		for (x = g_GlobalBoundBox[BOXLEFT]; x < g_GlobalBoundBox[BOXRIGHT]; x += FIXEDT_C(32))
		{
			CONL_PrintF("%i of %i\n", (x - g_GlobalBoundBox[BOXLEFT]) >> 16, (g_GlobalBoundBox[BOXRIGHT] - g_GlobalBoundBox[BOXLEFT]) >> 16);	
			
			for (y = g_GlobalBoundBox[BOXBOTTOM]; y < g_GlobalBoundBox[BOXTOP]; y += FIXEDT_C(32))
				B_GHOST_CreateNodeAtPos(x, y);
		}
#else
		// Recursive map generation
		B_GHOST_RecursiveSplitMap(&nodes[numnodes - 1]);
#endif
		
		// Don't do anything else
		l_SSMCreated = true;
		return;
	}
	
	/* Build node links */
	if (l_SSBuildChain < numsubsectors)
	{
		// Build links for 5 subsectors at a time
		zz = l_SSBuildChain + 5;
		
		if (zz >= numsubsectors)
			zz = numsubsectors;
		
		for (; l_SSBuildChain < zz; l_SSBuildChain++)
			BS_GHOST_BuildLinks(l_SSBuildChain);
		
		// Continue next time
		return;
	}
	
#if 0
	if (l_SSBuildChain < (l_UMSize[0] * l_UMSize[1]))
	{
		// Debug
		if (g_BotDebug)
			CONL_PrintF("GHOSTBOT: Building links for %u\n", (unsigned)l_SSBuildChain);
		
		// Go through unimatrixes chain
		for (zz = 0; zz < 20; zz++)
			if (l_SSBuildChain < (l_UMSize[0] * l_UMSize[1]))
			{
				// Get Current
				UniMatrix = &l_UMGrid[l_SSBuildChain++];
				
				// Go through all nodes here
				for (i = 0; i < UniMatrix->NumNodes; i++)
				{
					// Get current node
					CurrentNode = UniMatrix->Nodes[i];
					
					// Project all around node to find other nodes
					for (x = -1; x <= 1; x++)
						for (y = -1; y <= 1; y++)
						{
							// Zero?
							if (x == 0 || y == 0)
								continue;
							
							// Determine lox
							lox = x + 1;
							loy = y + 1;
							
							// If a node is already here, ignore
							if (CurrentNode->Links[lox][loy].Node)
								continue;
							
							// Setup loop
							NearNode = NULL;
							dx = CurrentNode->x;
							dy = CurrentNode->y;
							j = 0;
							
							// Node searching loop
							while (!NearNode && ++j < 10)
							{
								// Add coordinate
								dx += (BOTMINNODEDIST * x);
								dy += (BOTMINNODEDIST * y);
								
								// Try and locate nearby nodes
								NearNode = B_GHOST_NodeNearPos(dx, dy, CurrentNode->FloorZ, !!j);
								
								// No Node found?
								if (!NearNode)
									continue;
									
								// If node is self, ignore
								if (NearNode == CurrentNode)
								{
									NearNode = NULL;
									continue;
								}
							
								// Get link angle between targets
									// This will determine whether or not this node
									// in said direction is compatible for this dir.
									// If this check is not done, then a node that
									// should face west might really be east.
									// This fixes problems where the bot wants to
									// move east, but moves west instead.
								BS_LinkDirection(&uX, &uY, CurrentNode->x, CurrentNode->y, NearNode->x, NearNode->y);
							
								if (!BS_NearLinkDir(x, y, uX, uY))
								{
									NearNode = NULL;
									continue;
								}
							
								// Check to see if path can be traversed
								if (!BS_CheckNodeToNode(NULL, CurrentNode, NearNode, true))
								{
									NearNode = NULL;
									continue;
								}
							}
							
							// Completely failed?
							if (!NearNode || j >= 10)
								continue;
							
							// Movement to node is possible, link it
							CurrentNode->Links[lox][loy].Node = NearNode;
							CurrentNode->Links[lox][loy].Dist = P_AproxDistance(
									dx - CurrentNode->x, dy - CurrentNode->y);
							
							// Set back reference, but only if the near node has
							// nothing already
							if (!NearNode->Links[c_LinkOp[lox]][c_LinkOp[loy]].Node)
							{
								NearNode->Links[c_LinkOp[lox]][c_LinkOp[loy]].Node = CurrentNode;
								NearNode->Links[c_LinkOp[lox]][c_LinkOp[loy]].Dist = CurrentNode->Links[lox][loy].Dist;
							}
						}
				}
			}
		
		// For next time
		return;
	}
#endif
	
	// All done!
	l_SSAllDone = true;
}

/* B_ClearNodes() -- Clears level */
void B_ClearNodes(void)
{
	size_t i, j;
	
	/* Free all bot shore nodes */
	for (i = 0; i < l_NumLocalBots; i++)
		if (l_LocalBots[i])
		{
			BS_GHOST_ClearShore(l_LocalBots[i], false);
			BS_GHOST_ClearShore(l_LocalBots[i], true);
		}
	
	/* Wipe adjacent sector data */
	if (l_BAdj && l_BNumAdj)
	{
		Z_Free(l_BAdj);
		Z_Free(l_BNumAdj);
		l_BAdj = NULL;
		l_BNumAdj = l_BNumSecs = l_BBuildAdj = 0;
	}
	
	/* Destroy the unimatrix */
	if (l_UMGrid)
		Z_Free(l_UMGrid);
	l_UMGrid = NULL;
	l_UMSize[0] = l_UMSize[1] = 0;
	l_UMBase[0] = l_UMBase[1] = 0;
	
	/* Clear mesh map */
	l_SSMCreated = false;
	l_SSBuildChain = 0;
	l_SSAllDone = false;
	
	/* Clear node list */
	l_HeadNode = 0;
	
	/* Clear anything extra there may be */
	Z_FreeTags(PU_BOTS, PU_BOTS);
	
	/* Unset that we have bots */
	g_GotBots = false;
}

int CLC_DumpNodes(const uint32_t a_ArgC, const char** const a_ArgV);

/* B_InitNodes() -- Initializes level */
void B_InitNodes(void)
{
	int i;
	static bool_t AddedCLC;
	
	/* Need to add command? */
	if (!AddedCLC)
	{
		CONL_AddCommand("dumpnodes", CLC_DumpNodes);
		AddedCLC = true;
	}
	
	/* Clear old data */
	B_ClearNodes();
	
	/* Prepare for sector initialization */
	l_BNumSecs = numsectors;
	l_BAdj = Z_Malloc(sizeof(*l_BAdj) * l_BNumSecs, PU_BOTS, NULL);
	l_BNumAdj = Z_Malloc(sizeof(*l_BNumAdj) * l_BNumSecs, PU_BOTS, NULL);
	
	/* Prepare the unimatrix */
	// Initialize Base
	l_UMBase[0] = g_GlobalBoundBox[BOXLEFT] & ~(UNIMATRIXDIV - 1);
	l_UMBase[1] = g_GlobalBoundBox[BOXBOTTOM] & ~(UNIMATRIXDIV - 1);
	
	// Determine size
	l_UMSize[0] = ((g_GlobalBoundBox[BOXRIGHT] & ~(UNIMATRIXDIV - 1)) - l_UMBase[0]) / UNIMATRIXSIZE;
	l_UMSize[1] = ((g_GlobalBoundBox[BOXTOP] & ~(UNIMATRIXDIV - 1)) - l_UMBase[1]) / UNIMATRIXSIZE;
	
	// Convert down
	l_UMSize[0] >>= FRACBITS;
	l_UMSize[1] >>= FRACBITS;
	
	// Allocate
	l_UMGrid = Z_Malloc(sizeof(*l_UMGrid) * (l_UMSize[0] * l_UMSize[1]), PU_BOTS, NULL);
	
	/* Debug */
	if (g_BotDebug)
		CONL_PrintF("GHOSTBOT: Unimatrix is [%u, %u] @ (%i, %i)\n",
				l_UMSize[0], l_UMSize[1],
				l_UMBase[0] >> FRACBITS, l_UMBase[1] >> FRACBITS
			);
	
	/* Determine if bots are already inside */
	g_GotBots = false;
	if (D_XNetIsServer())
		for (i = 0; i < g_NumXPlays; i++)
			if (g_XPlays[i])
				if ((g_XPlays[i]->Flags & (DXPF_LOCAL | DXPF_BOT)) == (DXPF_LOCAL | DXPF_BOT))
					g_GotBots = true;
}

/*****************************************************************************/

/* BS_GHOST_PopNode() -- Pops a shore node */
B_ShoreNode_t* BS_GHOST_PopNode(struct B_GhostBot_s* a_Bot, const bool_t a_Work)
{
	/* Work? */
	if (a_Work)
	{
		// No more to pop
		if (a_Bot->NumWork == 0)
			return NULL;
		
		// Remove top
		Z_Free(a_Bot->Work[a_Bot->NumWork - 1]);
		a_Bot->Work[a_Bot->NumWork - 1] = NULL;
		a_Bot->NumWork--;
		
		// Return topmost node
		return a_Bot->Work[a_Bot->NumWork - 1];
	}
	
	/* Standard */
	else
	{
		return NULL;
	}
}

/* BS_GHOST_AddNode() -- Adds a shore node */
B_ShoreNode_t* BS_GHOST_AddNode(struct B_GhostBot_s* a_Bot, const bool_t a_Work, const B_ShoreType_t a_Type, const fixed_t a_X, const fixed_t a_Y, const fixed_t a_Z)
{
	B_ShoreNode_t* New;
	
	/* Allocate */
	New = Z_Malloc(sizeof(*New), PU_BOTS, NULL);
	
	/* Initialize */
	New->Type = a_Type;
	New->Pos[0] = a_X;
	New->Pos[1] = a_Y;
	New->Pos[2] = a_Z;
	New->SubS = R_IsPointInSubsector(a_X, a_Y);
	New->UniM = BS_UnimatrixAtPos(a_X, a_Y);
	
	// On the floor?
	if (a_Z == ONFLOORZ)
		if (New->SubS)
			New->Pos[2] = New->SubS->sector->floorheight;
		else
			New->Pos[2] = a_Z;
	
	New->BotNode = B_GHOST_NodeNearPos(a_X, a_Y, New->Pos[2], true);
	
	/* Add to list */
	if (a_Work)
	{
		Z_ResizeArray((void**)&a_Bot->Work, sizeof(*a_Bot->Work),
			a_Bot->NumWork, a_Bot->NumWork + 1);
		a_Bot->Work[a_Bot->NumWork++] = New;
	}
	
	else
	{
		Z_ResizeArray((void**)&a_Bot->Shore, sizeof(*a_Bot->Shore),
			a_Bot->NumShore, a_Bot->NumShore + 1);
		a_Bot->Shore[a_Bot->NumShore++] = New;
	}
	
	/* Return New node */
	return New;
}

/* BS_GHOST_ClearShore() -- Clears shore path */
void BS_GHOST_ClearShore(struct B_GhostBot_s* a_Bot, const bool_t a_Work)
{
	uint32_t i;
	
	/* Remove old shore/work nodes if they exist */
	if (a_Work)
	{
		if (a_Bot->Work)
		{
			for (i = 0; i < a_Bot->NumWork; i++)
				if (a_Bot->Work[i])
					Z_Free(a_Bot->Work[i]);
		
			Z_Free(a_Bot->Work);
		}
	
		a_Bot->Work = NULL;
		a_Bot->NumWork = a_Bot->WorkIt = 0;
	}
	
	else
	{
		if (a_Bot->Shore)
		{
			for (i = 0; i < a_Bot->NumShore; i++)
				if (a_Bot->Shore[i])
					Z_Free(a_Bot->Shore[i]);
		
			Z_Free(a_Bot->Shore);
		}
	
		a_Bot->Shore = NULL;
		a_Bot->NumShore = a_Bot->ShoreIt = 0;
	}
	
	/* Remove any targets that are set */
	if (!a_Work)
		for (i = 0; i < MAXBOTTARGETS; i++)
			if (a_Bot->Targets[i].IsSet && a_Bot->Targets[i].MoveTarget)
				if (a_Bot->Targets[i].Key == SHOREKEY)
					memset(&a_Bot->Targets[i], 0, sizeof(a_Bot->Targets[i]));
}

/* BS_GHOST_WorkToShore() -- Moves work to shore */
void BS_GHOST_WorkToShore(struct B_GhostBot_s* a_Bot)
{
	/* Free shore, if any */
	BS_GHOST_ClearShore(a_Bot, false);
	
	/* Move pointers over */
	a_Bot->Shore = a_Bot->Work;
	a_Bot->NumShore = a_Bot->NumWork;
	a_Bot->ShoreIt = a_Bot->WorkIt;
	
	/* Clear work pointers */
	a_Bot->Work = NULL;
	a_Bot->NumWork = a_Bot->WorkIt = 0;
}

/* BS_GHOST_ShoreFromTo() -- Builds a path from point 1 to point 2 */
// Returns false, if not possible
bool_t BS_GHOST_ShoreFromTo(struct B_GhostBot_s* a_Bot, const fixed_t a_FromX, const fixed_t a_FromY, const fixed_t a_ToX, const fixed_t a_ToY)
{
#define BUFSIZE 128
#define MAXFAILS 128
	int32_t i, x, b, Fails, Tries;
	B_ShoreNode_t* SNode;
	B_GhostNode_t* RoverNode, *DestNode, *Near;
	I_File_t* File;
	char Buf[BUFSIZE];
	int32_t DirX, DirY, ArrX, ArrY;
	static uint32_t CheckID;
	
	struct
	{
		bool_t OK;								// OK
		fixed_t DistToGoal;						// Distance to goal
		int32_t LoX, LoY;						// Link type
		B_GhostNode_t* Node;					// Node Here
	} DirChoice[9];
	
	/* Increase CheckID */
	CheckID += 1;
	
	/* Clear old path */
	BS_GHOST_ClearShore(a_Bot, true);
	
	/* Add Head Position */
	SNode = BS_GHOST_AddNode(a_Bot, true, BST_HEAD, a_FromX, a_FromY, ONFLOORZ);
	
	/* Generate Path to Target */
	// This is A* like, I do not have the internet currently but I do have
	// a general idea of how the algorithm works.
	
	// Initialize
	RoverNode = SNode->BotNode;		// Start at the starting point
	DestNode = B_GHOST_NodeNearPos(a_ToX, a_ToY, ONFLOORZ, true);
	Fails = 0;
	
	// Check initial node we start at
	RoverNode->CheckID = CheckID;
	
	// Traversal loop
	while (RoverNode != DestNode && Fails < MAXFAILS)
	{
		//CONL_PrintF("@%p, F=%i\n", RoverNode, Fails);
		
		// Get direction from current node position to destination
		DirX = DirY = 0;
		BS_LinkDirection(&DirX, &DirY, RoverNode->x, RoverNode->y, DestNode->x, DestNode->y);
		
		// Get array directions
		ArrX = DirX + 1;
		ArrY = DirY + 1;
		
		// Initialize
		memset(DirChoice, 0, sizeof(DirChoice));
		x = 0;
		
		// Try all directions
		for (i = 0; i < 9; i++)
		{
			// Get node at this position
			Near = RoverNode->Links[ArrX][ArrY].Node;
			
			// Only if it exists and has never been checked
				// also, if it is traversable to
			if (Near && Near->CheckID != CheckID)
				// See if traversal is possible
				if (BS_CheckNodeToNode(a_Bot, RoverNode, Near, false))
				{
					DirChoice[x].OK = true;
					DirChoice[x].DistToGoal =
						P_AproxDistance(
								Near->x - DestNode->x,
								Near->y - DestNode->y
							);
					DirChoice[x].LoX = ArrX;
					DirChoice[x].LoY = ArrY;
					DirChoice[x].Node = Near;
					x++;
				}
			
			// Check more directions
			ArrX++;
		
			if (ArrX == 3)
			{
				ArrY++;
				ArrX = 0;
			
				if (ArrY == 3)
					ArrY = 0;
			}
			
			// Never stop on (0,0)
			if (ArrX == 1 && ArrY == 1)
				ArrX++;
		}
		
		// Get the target that is closest to the goal
		for (b = -1, i = 0; i < x; i++)
			if (b == -1 || (b >= 0 && DirChoice[i].DistToGoal < DirChoice[b].DistToGoal))
				b = i;
		
		// If the best path has been determined
		if (b != -1)
		{
			//CONL_PrintF(">%p [%i/%i]\n", DirChoice[b].Node, b, x);
			
			// Drop a shore node here
			RoverNode = DirChoice[b].Node;
			SNode = BS_GHOST_AddNode(a_Bot, true, BST_NODE, RoverNode->x, RoverNode->y, ONFLOORZ);
			
			// Set the node as checked
			RoverNode->CheckID = CheckID;
		}
		
		// Otherwise pop the point and try another route
		else
		{
			// A failure
			Fails++;
			
			// If there is only one node, then traversal failed
			if (a_Bot->NumWork == 1)
				break;
			
			// Otherwise, try the node before
			else
			{
				SNode = BS_GHOST_PopNode(a_Bot, true);
			
				// Go back there then
				RoverNode = SNode->BotNode;
			}
			
			//CONL_PrintF("<%p [%i/%i]\n", RoverNode, b, x);
		}
	}
	
	/* Add Tail Position */
	SNode = BS_GHOST_AddNode(a_Bot, true, BST_TAIL, a_ToX, a_ToY, ONFLOORZ);
	
	/* Debug Dump */
	if (g_BotDebug)
	{
		// Open File
		File = I_FileOpen("bshore", IFM_RWT);
		
		// If it worked, dump data
		if (File)
		{
			// Dump path of shore nodes
			for (x = 0; x < a_Bot->NumWork; x++)
			{
				// Get node
				SNode = a_Bot->Work[x];
				
				// Enter point of this node
				snprintf(Buf, BUFSIZE - 1, "%i.0 %i.0\n",
						SNode->Pos[0] >> 16, SNode->Pos[1] >> 16
					);
				I_FileWrite(File, Buf, strlen(Buf));
			}
			
			// Dump map lines that are solid
			// This is to help determine the contour of the level
			snprintf(Buf, BUFSIZE - 1, "\n\"lines\"");
			I_FileWrite(File, Buf, strlen(Buf));
	
			for (x = 0; x < numlines; x++)
			{
				// Not a blocking line?
				if (!(lines[x].flags & ML_BLOCKING))
					continue;
		
				// Move to base position
				snprintf(Buf, BUFSIZE - 1, "move %i.0 %i.0\n",
						lines[x].v1->x >> 16, lines[x].v1->y >> 16
					);
				I_FileWrite(File, Buf, strlen(Buf));
		
				// Line to end position
				snprintf(Buf, BUFSIZE - 1, "%i.0 %i.0\n",
						lines[x].v2->x >> 16, lines[x].v2->y >> 16
					);
				I_FileWrite(File, Buf, strlen(Buf));
			}
	
			/* Close File */
			I_FileClose(File);
		}
	}
	
	/* Path mapped */
	if (RoverNode != DestNode)
		return false;
	return true;
#undef BUFSIZE
#undef MAXDEPTH
}

