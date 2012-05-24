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
// Copyright (C) 2012 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Bot Code

/***************
*** INCLUDES ***
***************/

#include "b_bot.h"
#include "z_zone.h"
#include "m_bbox.h"
#include "p_mobj.h"
#include "p_local.h"
#include "r_main.h"
#include "doomstat.h"

/****************
*** CONSTANTS ***
****************/

#define BOTMINNODEDIST		(32 << FRACBITS)	// Minimum node distance
#define MAXBGADJDEPTH					32		// Max Adjaceny Depth
#define UNIMATRIXDIV		(32 << FRACBITS)	// Division of the unimatrix
#define UNIMATRIXSIZE					(128)	// Size of the unimatrix

#define UNIMATRIXPHYSSIZE	(FixedMul(UNIMATRIXDIV, UNIMATRIXSIZE << FRACBITS))

static const int8_t c_LinkOp[3] = {2, 1, 0};

/*****************
*** STRUCTURES ***
*****************/

/* B_GhostNode_t -- A Node */
typedef struct B_GhostNode_s
{
	fixed_t x;									// X Position
	fixed_t y;									// Y Position
	
	fixed_t FloorZ;								// Z of floor
	fixed_t CeilingZ;							// Z of ceiling
	
	struct
	{
		struct B_GhostNode_s* Node;				// Node connected to
		fixed_t Dist;							// Distance to node
	} Links[3][3];								// Chain Links
} B_GhostNode_t;

/* B_Unimatrix_t -- A large grid portion of the map */
typedef struct B_Unimatrix_s
{
	fixed_t StartPos[2];						// Start Positions
	fixed_t EndPos[2];							// End Positions
	
	subsector_t** SubSecs;						// Subsectors in unimatrix
	size_t NumSubSecs;							// Number of subsectors
	
	sector_t** Sectors;							// Sectors inside unimatrix
	size_t NumSectors;							// Number of sectors in it
	
	B_GhostNode_t** Nodes;						// Nodes in unimatrix
	size_t NumNodes;							// Number of those nodes
} B_Unimatrix_t;

/* B_LineSet_t -- A set of lines */
typedef struct B_LineSet_s
{
	fixed_t vs[2];								// Start
	fixed_t ve[2];								// End
	
	struct B_LineSet_s* Next;					// Next line
	struct B_LineSet_s* Prev;					// Previous line
} B_LineSet_t;

/**************
*** GLOBALS ***
**************/

fixed_t g_GlobalBoundBox[4];					// Global bounding box

/*************
*** LOCALS ***
*************/

// Adjacent Sectors
static sector_t* (*l_BAdj)[MAXBGADJDEPTH] = NULL;	// Adjacent sector list
static size_t* l_BNumAdj = NULL;				// Number of adjacent sectors
static size_t l_BNumSecs = 0;					// Number of sectors
static size_t l_BBuildAdj = 0;					// Current stage

// Unimatrix
static fixed_t l_UMBase[2];						// Base unimatrix position
static int32_t l_UMSize[2];						// Size of the unimatrix
static B_Unimatrix_t* l_UMGrid;					// Unimatrix Grid
static size_t l_UMBuild = 0;					// Umimatrix Build Number

// SubSector Mesh
static bool_t l_SSMCreated = false;				// Mesh created?
static fixed_t l_GFloorZ, l_GCeilingZ;			// Scanned floor and ceiling position
static int32_t l_SSBuildChain = 0;				// Final Stage Chaining

/****************
*** FUNCTIONS ***
****************/

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
static bool_t BS_GHOSTCheckNodeTrav(intercept_t* in, void* const a_Data)
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
B_GhostNode_t* B_GHOST_NodeNearPos(const fixed_t a_X, const fixed_t a_Y)
{
	B_Unimatrix_t* UniMatrix;
	B_GhostNode_t* CurrentNode;
	fixed_t Dist;
	size_t i;
	
	/* Get Unimatrix here */
	UniMatrix = BS_UnimatrixAtPos(a_X, a_Y);
	
	/* Nothing here? */
	if (!UniMatrix)
		return NULL;
	
	/* Look through nodes there */
	// Go through all nodes here
	for (i = 0; i < UniMatrix->NumNodes; i++)
	{
		// Get current node
		CurrentNode = UniMatrix->Nodes[i];
		
		// Get distance
		Dist = P_AproxDistance(a_X - CurrentNode->x, a_Y - CurrentNode->y);
		
		// Distance is close
		if (Dist < BOTMINNODEDIST)
			return CurrentNode;
	}
	
	/* None Found */
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
	
	/* Truncate the coordinates to a 16-grid */
	RealX &= ~1048575;
	RealY &= ~1048575;
	
	/* See if there is a subsector here */
	// Because this spot could be in the middle of nowhere
	SubS = R_IsPointInSubsector(RealX, RealY);
	
	// If nothing is there, forget it
	if (!SubS)
		return NULL;
	
	// Don't add any nodes that are close to this spot
	NearNode = B_GHOST_NodeNearPos(RealX, RealY);
	
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
	
	/* Debug */
	P_SpawnMobj(
			RealX,
			RealY,
			l_GFloorZ,
			INFO_GetTypeByName("ReMooDBotDebugNode")
		);
		
	/* Return the new node */
	return New;
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
	int8_t Side, i, j;
	fixed_t x1, y1, x2, y2;
	
	fixed_t* px, *py;
	fixed_t cx, cy, dx, dy;
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
	B_LineSet_t* InitSet, *OldSet, *FirstSet;
	B_GhostNode_t* CurrentNode, *NearNode;
	
	fixed_t x, y, z, dx, dy;
	int32_t uX, uY;
	int8_t lox, loy;
	
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
		// Recursive map generation
		B_GHOST_RecursiveSplitMap(&nodes[numnodes - 1]);
		
		// Don't do anything else
		l_SSMCreated = true;
		return;
	}
	
	/* Build node links */
	if (l_SSBuildChain < (l_UMSize[0] * l_UMSize[1]))
	{
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
								
							// Project
							dx = CurrentNode->x + (BOTMINNODEDIST * x);
							dy = CurrentNode->y + (BOTMINNODEDIST * y);
							
							// Try and locate nearby nodes
							NearNode = B_GHOST_NodeNearPos(dx, dy);
					
							// No node?
							if (!NearNode)
							{
								// Try a deeper reach
								dx = dx + (BOTMINNODEDIST * x);
								dy = dx + (BOTMINNODEDIST * y);
								
								// Try again
								NearNode = B_GHOST_NodeNearPos(dx, dy);
								
								// Still nothing
								if (!NearNode)
									continue;
							}
							
							// Check to see if path can be traversed
							
							// Movement to node is possible, link it
							lox = c_LinkOp[x + 1];
							loy = c_LinkOp[y + 1];
							CurrentNode->Links[lox][loy].Node = NearNode;
							CurrentNode->Links[lox][loy].Dist = P_AproxDistance(
									dx - CurrentNode->x, dy - CurrentNode->y);
						}
				}
			}
		
		// For next time
		return;
	}
}

/* B_GHOST_ClearLevel() -- Clears level */
void B_GHOST_ClearLevel(void)
{
	size_t i, j;
	
	/* Wipe adjacent sector data */
	if (l_BAdj && l_BNumAdj)
	{
		// Destroy lists
		for (i = 0; i < l_BNumSecs; i++)
			if (l_BAdj[i])
				Z_Free(l_BAdj[i]);
		
		Z_Free(l_BAdj);
		Z_Free(l_BNumAdj);
		l_BAdj = l_BNumAdj = NULL;
		l_BNumSecs = l_BBuildAdj = 0;
	}
	
	/* Destroy the unimatrix */
	if (l_UMGrid)
		Z_Free(l_UMGrid);
	l_UMSize[0] = l_UMSize[1] = 0;
	l_UMBase[0] = l_UMBase[1] = 0;
	
	/* Clear mesh map */
	l_SSMCreated = false;
	l_SSBuildChain = 0;
	
	/* Clear anything extra there may be */
	Z_FreeTags(PU_BOTS, PU_BOTS);
}

/* B_GHOST_InitLevel() -- Initializes level */
void B_GHOST_InitLevel(void)
{
	/* Clear old data */
	B_GHOST_ClearLevel();
	
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
}

/* B_GHOST_Think() -- Bot thinker routine */
void B_GHOST_Think(B_GhostBot_t* const a_GhostBot, ticcmd_t* const a_TicCmd)
{
	size_t J;
	
	/* Check */
	if (!a_GhostBot || !a_TicCmd)
		return;
	
	/* Clear tic command */
	memset(a_TicCmd, 0, sizeof(*a_TicCmd));
	
	/* Init */
	a_GhostBot->TicCmdPtr = a_TicCmd;
	
	/* Go through jobs and execute them */
	for (J = 0; J < MAXBOTJOBS; J++)
		if (a_GhostBot->Jobs[J].JobHere)
		{
			// Sleeping job?
			if (gametic < a_GhostBot->Jobs[J].Sleep)
				continue;
			
			// Execute
			if (a_GhostBot->Jobs[J].JobFunc)
				if (!a_GhostBot->Jobs[J].JobFunc(a_GhostBot, J))
				{
					// Delete job
					a_GhostBot->Jobs[J].JobHere = false;
					a_GhostBot->Jobs[J].Priority = 0;
					a_GhostBot->Jobs[J].Sleep = 0;
					a_GhostBot->Jobs[J].JobFunc = NULL;
				}
		}
}

