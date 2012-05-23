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

/****************
*** CONSTANTS ***
****************/

#define MAXBGADJDEPTH					32		// Max Adjaceny Depth
#define UNIMATRIXDIV		(32 << FRACBITS)	// Division of the unimatrix
#define UNIMATRIXSIZE					(128)	// Size of the unimatrix

#define UNIMATRIXPHYSSIZE	(FixedMul(UNIMATRIXDIV, UNIMATRIXSIZE << FRACBITS))

/*****************
*** STRUCTURES ***
*****************/

/* B_Unimatrix_t -- A large grid portion of the map */
typedef struct B_Unimatrix_s
{
	fixed_t StartPos[2];						// Start Positions
	fixed_t EndPos[2];							// End Positions
	
	subsector_t** SubSecs;						// Subsectors in unimatrix
	size_t NumSubSecs;							// Number of subsectors
	
	sector_t** Sectors;							// Sectors inside unimatrix
	size_t NumSectors;							// Number of sectors in it
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

/****************
*** FUNCTIONS ***
****************/

/* B_GHOST_SplitPoly() -- Splits polygon */
bool_t B_GHOST_SplitPoly(B_LineSet_t* const a_LineSet, B_LineSet_t** const a_A, B_LineSet_t** const a_B, const fixed_t a_x1, const fixed_t a_y1, const fixed_t a_x2, const fixed_t a_y2)
{
	fixed_t SplitSlope, SplitB;
	fixed_t SegSlope, SegB;
	fixed_t IntX, IntY;
	B_LineSet_t* Rover;
	B_LineSet_t* MarkA, *MarkB;
	fixed_t IntAMx, IntAMy, IntBMx, IntBMy;
	B_LineSet_t* MarkAa, *MarkAb, *MarkBa, *MarkBb;
	B_LineSet_t* TempSet, *ASeg, *BSeg;
	bool_t Looped;
	int32_t i, vNum;
	
	B_LineSet_t** WorkMark;
	B_LineSet_t** WorkA, **WorkB;
	fixed_t WorkIx, WorkIy;
	
	/* Check */
	if (!a_LineSet || !a_A || !a_B)
		return false;
	
	/* Debug */
	fprintf(stderr, "With (%f, %f) -> (%f, %f)\n",
		FIXED_TO_FLOAT(a_x1), FIXED_TO_FLOAT(a_y1),
		FIXED_TO_FLOAT(a_x2), FIXED_TO_FLOAT(a_y2));
	
	/* Get slope info of line */
	// Division
	if (a_x2 - a_x1)
		SplitSlope = FixedDiv(a_y2 - a_y1, a_x2 - a_x1);
	else
		SplitSlope = ~((fixed_t)0);
	
	// Base positions (y = mx + b, solve for b)
	// So: b = y - mx	
	SplitB = a_y1 - FixedMul(SplitSlope, a_x1);
	
	/* Begin dividing */
	Looped = false;
	MarkA = MarkB = NULL;
	for (vNum = 0, Rover = a_LineSet; !Looped || (Looped && Rover != a_LineSet); Rover = Rover->Next, vNum++)
	{
		// Looped
		Looped = true;
		
		// Find slope and b of this current line
		if (Rover->ve[0] - Rover->vs[0])
			SegSlope = FixedDiv(Rover->ve[1] - Rover->vs[1], Rover->ve[0] - Rover->vs[0]);
		else
			SegSlope = ~((fixed_t)0);
		SegB = Rover->vs[1] - FixedMul(SegSlope, Rover->vs[0]);
		
		// Parallel? Never intercepts
		if (SplitSlope == SegSlope)
			continue;
		
		// Find interception point
			// Split is vertical
		if (SplitSlope == ~((fixed_t)0))
		{
			fprintf(stderr, "%i sv\n", vNum);
			
			// X is already known
			IntX = a_x1;
			
			// But the X in y = mx + b
			IntY = FixedMul(SegSlope, IntX) + SegB;
		}
		
			// Segment is vertical
		else if (SegSlope == ~((fixed_t)0))
		{
			fprintf(stderr, "%i gv\n", vNum);
			
			// X is already known
			IntX = Rover->vs[0];
			
			// But the X in y = mx + b
			IntY = FixedMul(SplitSlope, IntX) + SplitB;
		}
		
			// Both non-vertical
		else
		{
			fprintf(stderr, "%i, nv\n", vNum);
			
			// 0 = (m1 - m2)x + (b1 - b2), solve for x
			// So: (b1 - b2) / (m1 - m2) = x
			IntX = FixedDiv(SplitB - SegB, SplitSlope - SegSlope);
		
			// Then put it in the y = mx + b
			IntY = FixedMul(SplitSlope, IntX) + SplitB;
		}
		
		// Determine if the interception point is off the line
		if (((Rover->ve[0] >= Rover->vs[0]) && (IntX < Rover->vs[0] || IntX > Rover->ve[0])) ||
			((Rover->ve[0] <= Rover->vs[0]) && (IntX > Rover->vs[0] || IntX < Rover->ve[0])))
		{
			fprintf(stderr, "Int (%f, %f) outside [(%f, %f), (%f, %f)]\n",
					FIXED_TO_FLOAT(IntX), FIXED_TO_FLOAT(IntY),
					FIXED_TO_FLOAT(Rover->vs[0]), FIXED_TO_FLOAT(Rover->vs[1]),
					FIXED_TO_FLOAT(Rover->ve[0]), FIXED_TO_FLOAT(Rover->ve[1])
				);
			continue;
		}
		
		fprintf(stderr, "Split! %i\n", vNum);
		
		// Mark lines
		if (!MarkA)
		{
			MarkA = Rover;
			IntAMx = IntX;
			IntAMy = IntY;
		}
		else
		{
			MarkB = Rover;
			IntBMx = IntX;
			IntBMy = IntY;
			break;	// No more processing needs to be done
		}
	}
	
	/* No Interception at all? */
	if (!MarkA || !MarkB)
	{
		fprintf(stderr, "No marks: %p %p\n", MarkA, MarkB);
		return false;
	}
	
	/* Split the polygon at the marks */
	fprintf(stderr, "Double marked!\n");
	
	// Split the lines
	for (i = 0; i < 2; i++)
	{
		// Line A
		if (!i)
		{
			WorkIx = IntAMx;
			WorkIy = IntAMy;
			WorkMark = &MarkA;
			WorkA = &MarkAa;
			WorkB = &MarkAb;
		}
		
		// Line B
		else
		{
			WorkIx = IntBMx;
			WorkIy = IntBMy;
			WorkMark = &MarkB;
			WorkA = &MarkBa;
			WorkB = &MarkBb;
		}
		
		// Create new segment
		(*WorkB) = Z_Malloc(sizeof(*(*WorkB)), PU_BOTS, NULL);
		(*WorkB)->vs[0] = WorkIx;
		(*WorkB)->vs[1] = WorkIy;
		(*WorkB)->ve[0] = (*WorkMark)->ve[0];
		(*WorkB)->ve[1] = (*WorkMark)->ve[1];
	
		// Reduce the original line
		(*WorkA) = (*WorkMark);
		(*WorkA)->ve[0] = WorkIx;
		(*WorkA)->ve[1] = WorkIy;
		
		// Relink chains
			// Start from the next back
		(*WorkMark)->Next->Prev = (*WorkB);
		(*WorkB)->Next = (*WorkMark)->Next;
			// Link in second segment
		(*WorkB)->Prev = (*WorkA);
		(*WorkA)->Next = (*WorkB);
	}
	
	// Create two new line segments for the split line itself
	// Remember that B goes in the opposite direction
		// Allocate new segments
	ASeg = Z_Malloc(sizeof(*ASeg), PU_BOTS, NULL);
	BSeg = Z_Malloc(sizeof(*BSeg), PU_BOTS, NULL);
	
	// Chain the segments correctly first
	ASeg->Next = MarkBb;
	ASeg->Prev = MarkAa;
	BSeg->Next = MarkAb;
	BSeg->Prev = MarkBa;
	
	// Correct chains of new line
	ASeg->Prev->Next = ASeg;
	ASeg->Next->Prev = ASeg;
	BSeg->Prev->Next = BSeg;
	BSeg->Next->Prev = BSeg;
	
	// Set position of lines
		// First
	ASeg->vs[0] = ASeg->Prev->ve[0];
	ASeg->vs[1] = ASeg->Prev->ve[1];
	ASeg->ve[0] = ASeg->Next->vs[0];
	ASeg->ve[1] = ASeg->Next->vs[1];
		// Second
	BSeg->vs[0] = BSeg->Prev->ve[0];
	BSeg->vs[1] = BSeg->Prev->ve[1];
	BSeg->ve[0] = BSeg->Next->vs[0];
	BSeg->ve[1] = BSeg->Next->vs[1];
	
	/* Return the split line as the new "base" */
	*a_A = ASeg;
	*a_B = BSeg;
	
	/* Success! */
	return true;
}

/* BS_GHOST_PolyCenter() -- Obtains polygon center */
void BS_GHOST_PolyCenter(B_LineSet_t* const a_LineSet, fixed_t* const a_Xp, fixed_t* const a_Yp)
{
#if 1
#define MAXPOLYSIDES 128
	fixed_t px[MAXPOLYSIDES];
	fixed_t py[MAXPOLYSIDES];
	fixed_t cx[MAXPOLYSIDES];
	fixed_t cy[MAXPOLYSIDES];
	bool_t Did[MAXPOLYSIDES][MAXPOLYSIDES];
	fixed_t BaseX, BaseY;
	fixed_t TotalX, TotalY;
	size_t nSides, nCenters;
	int32_t i, j, k;
	B_LineSet_t* Rover, *Next;
	bool_t OK;
	
	/* Clear */
	memset(px, 0, sizeof(px));
	memset(py, 0, sizeof(py));
	memset(cx, 0, sizeof(cx));
	memset(cy, 0, sizeof(cy));
	
	/* Obtain mid points of all lines on the polygons */
	nSides = 0;
	for (OK = false, Rover = a_LineSet; !OK || (OK && Rover != a_LineSet); Rover = Rover->Next)
	{
		OK = true;
		Next = Rover->Next;
		
		if (nSides < MAXPOLYSIDES - 1)
		{
			px[nSides] = FixedDiv(Next->vs[0] + Rover->vs[0], 2 << FRACBITS);
			py[nSides++] = FixedDiv(Next->vs[1] + Rover->vs[1], 2 << FRACBITS);
		}
	};
	
	/* Obtain the mid points of all mid points, that are not adjacent */
	// This loop shrinks the input polygon
	// Do this four times
	for (k = 0; k < 4; k++)
	{
		// Clear dids
		memset(Did, 0, sizeof(Did));
		
		// Prepare centering
		nCenters = 0;
		for (i = 0; i < nSides; i++)
			for (j = 0; j < nSides; j++)
			{
				// Adjacent?
				if (j == i || j == i - 1 || j == i + 1 ||
					(i == 0 && j == nSides - 1) ||
					(i == nSides - 1 && j == 0))
					continue;
				
				// Did already?
				if (Did[i][j])
					continue;
			
				if (nCenters < MAXPOLYSIDES - 1)
				{
					cx[nCenters] = FixedDiv(px[j] + px[i], 2 << FRACBITS);
					cy[nCenters++] = FixedDiv(py[j] + py[i], 2 << FRACBITS);
					Did[i][j] = true;	// Did it
				}
			}
		
		// Move over old stuff
		memmove(px, cx, sizeof(px));
		memmove(py, cy, sizeof(py));
		nSides = nCenters;
	}
	
	/* Calculate the base of all the points */
	BaseX = BaseY = 31000 << FRACBITS;
	for (i = 0; i < nSides; i++)
	{
		if (px[i] < BaseX)
			BaseX = px[i];
		if (py[i] < BaseY)
			BaseY = py[i];
	}
	
	/* Subtract all the midpoints by the base */
	for (i = 0; i < nSides; i++)
	{
		px[i] -= BaseX;
		py[i] -= BaseY;
	}
	
	/* Average all of the midpoints */
	TotalX = TotalY = 0;
	for (i = 0; i < nSides; i++)
	{
		TotalX += px[i];
		TotalY += py[i];
	}
	
	/* Dive the total by the point count */
	TotalX = FixedDiv(TotalX, nSides << FRACBITS);
	TotalY = FixedDiv(TotalY, nSides << FRACBITS);
	
	/* The base is probably here */
	*a_Xp = TotalX + BaseX;
	*a_Yp = TotalY + BaseY;

#undef MAXPOLYSIDES
#else
#define MULCUT		16384
	fixed_t AreaTotal, AreaTotalB, gx, gy;
	B_LineSet_t* Rover, *Next;
	bool_t OK;
	
	/* Check */
	if (!a_LineSet)
		return;
		
	/* Clear */
	*a_Xp = 0;
	*a_Yp = 0;
	
	/* Calculate the area */
	// Vertex Totals
	AreaTotal = 0;
	for (OK = false, Rover = a_LineSet; !OK || (OK && Rover != a_LineSet); Rover = Rover->Next)
	{
		OK = true;
		Next = Rover->Next;
		
		AreaTotal += 
			FixedMul(
					FixedMul(Rover->vs[0], MULCUT),
					FixedMul(Next->vs[1], MULCUT)
				) -
			FixedMul(
					FixedMul(Next->vs[0], MULCUT),
					FixedMul(Rover->vs[1], MULCUT)
				);
	}
	
	// Divide it
	AreaTotal = FixedDiv(AreaTotal, 2 << FRACBITS);
	
	/* Find centroid now */
	AreaTotalB = 0;
	gx = gy = 0;
	for (OK = false, Rover = a_LineSet; !OK || (OK && Rover != a_LineSet); Rover = Rover->Next)
	{
		OK = true;
		Next = Rover->Next;
		
		AreaTotalB = 
			FixedMul(
					FixedMul(Rover->vs[0], MULCUT),
					FixedMul(Next->vs[1], MULCUT)
				) -
			FixedMul(
					FixedMul(Next->vs[0], MULCUT),
					FixedMul(Rover->vs[1], MULCUT)
				);
		
		gx += FixedMul(FixedMul(Rover->vs[0], MULCUT) + FixedMul(Next->vs[0], MULCUT), AreaTotalB);
		gy += FixedMul(FixedMul(Rover->vs[1], MULCUT) + FixedMul(Next->vs[1], MULCUT), AreaTotalB);
	}
	
	/* Divide */
	*a_Xp = FixedDiv(FixedDiv(gx, FixedMul(6 << FRACBITS, AreaTotal)), MULCUT);
	*a_Yp = FixedDiv(FixedDiv(gy, FixedMul(6 << FRACBITS, AreaTotal)), MULCUT);
#endif
}

/* B_GHOST_RecursiveSplitMap() -- Recursively split the map */
void B_GHOST_RecursiveSplitMap(const node_t* const a_Node, B_LineSet_t* const a_LineSet)
{
	size_t i, Side;
	B_LineSet_t* Rover;
	bool_t OK;
	B_LineSet_t* PolyA, *PolyB, *Temp, *ExtraA, *ExtraB;
	static FILE* DebugFile;
	static uint32_t DebugSet;
	fixed_t x, y;
	subsector_t* SubS, *PointSub;
	seg_t* Seg;
	
	/* Check */
	if (!a_Node || !a_LineSet)
		return;
	
	/* Debug */
	// Create?
	if (!DebugFile)
		DebugFile = fopen("PolyLog", "wt");
	
	/* Split along node */
	PolyA = PolyB = NULL;
	if (!B_GHOST_SplitPoly(a_LineSet, &PolyA, &PolyB,
				a_Node->x, a_Node->y,
				a_Node->x + a_Node->dx, a_Node->y + a_Node->dy))
		return;		// Oops!
	
	// Get center of one of the polygons
	BS_GHOST_PolyCenter(PolyA, &x, &y);
	
	fprintf(stderr, "PolyA center is (%f, %f)\n", FIXED_TO_FLOAT(x), FIXED_TO_FLOAT(y));

	// Determine sides the split polygons are on
		// ret 0 = front, 1 = back
	if (R_PointOnSide(x, y, a_Node) != 0)
	{
		// Polygon A is not on the front side of the node, so swap
		Temp = PolyA;
		PolyA = PolyB;
		PolyB = Temp;
	}
	
	/* Handle Each Side */
	for (Side = 0; Side < 2; Side++)
	{
		// Subsector?
		if (a_Node->children[Side] & NF_SUBSECTOR)
		{
			// Get current polygon and subsector
			SubS = &subsectors[a_Node->children[Side] & (~NF_SUBSECTOR)];
			Temp = (!Side ? PolyA : PolyB);
			
			// Split them all by the segs
			for (i = 0; i < SubS->numlines; i++)
			{
				// Get current seg
				Seg = &segs[SubS->firstline + i];
				
				// Split by this seg line
				if (!B_GHOST_SplitPoly(Temp, &ExtraA, &ExtraB,
						Seg->v1->x, Seg->v1->y,
						Seg->v2->x, Seg->v2->y))
					continue;	// Oops!
				
				// Determine which polygon is in this subsector
				BS_GHOST_PolyCenter(ExtraA, &x, &y);
				PointSub = R_IsPointInSubsector(x, y);
				
				// Which side to take?
				if (PointSub == SubS)
					Temp = ExtraA;
				else
					Temp = ExtraB;
			}
			
			// Get center of this polygon
			BS_GHOST_PolyCenter(Temp, &x, &y);
			
			// Spawn something there to see it
			P_SpawnMobj(x, y, ONFLOORZ, INFO_GetTypeByName("ReMooDBotDebugNode"));

			// Write output
			fprintf(stderr, "SubSector:\n");
			fprintf(DebugFile, "\"Set %u\"\n", DebugSet);
			for (OK = false, Rover = Temp; !OK || (OK && Rover != Temp); Rover = Rover->Next)
			{
				OK = true;
				fprintf(DebugFile, "%g %g\n", FIXED_TO_FLOAT(Rover->vs[0]), FIXED_TO_FLOAT(Rover->vs[1]));
				fflush(DebugFile);
		
				fprintf(stderr, "\t(%g, %g) -> (%g, %g)\n",
						FIXED_TO_FLOAT(Rover->vs[0]), FIXED_TO_FLOAT(Rover->vs[1]),
						FIXED_TO_FLOAT(Rover->ve[0]), FIXED_TO_FLOAT(Rover->ve[1])
					);
			}
			fprintf(DebugFile, "%g %g\n", FIXED_TO_FLOAT(Rover->vs[0]), FIXED_TO_FLOAT(Rover->vs[1]));
			fflush(DebugFile);
			
			fprintf(DebugFile, "\n");
			fprintf(stderr, "\n");
			DebugSet++;
		}
		
		// Node
		else
		{
			// Split again
			B_GHOST_RecursiveSplitMap(&nodes[a_Node->children[Side]], (!Side ? PolyA : PolyB));
		}
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
	B_LineSet_t* InitSet, *OldSet, *FirstSet;
	
	fixed_t x, y, z;
	int32_t uX, uY;
	
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
		{
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
		}
			
		// Return for later processing
		return;
	}
	
	/* Build SubSector Mesh Map */
	if (!l_SSMCreated)
	{
		// Create initial set
			// Top
		FirstSet = InitSet = Z_Malloc(sizeof(*InitSet), PU_BOTS, NULL);
		InitSet->vs[0] = g_GlobalBoundBox[BOXBOTTOM];
		InitSet->vs[1] = g_GlobalBoundBox[BOXLEFT];
		InitSet->ve[0] = g_GlobalBoundBox[BOXBOTTOM];
		InitSet->ve[1] = g_GlobalBoundBox[BOXRIGHT];
			// Right
		OldSet = InitSet;
		InitSet->Next = Z_Malloc(sizeof(*InitSet), PU_BOTS, NULL);
		InitSet = InitSet->Next;
		InitSet->Prev = OldSet;
		InitSet->vs[0] = OldSet->ve[0];
		InitSet->vs[1] = OldSet->ve[1];
		InitSet->ve[0] = g_GlobalBoundBox[BOXTOP];
		InitSet->ve[1] = g_GlobalBoundBox[BOXRIGHT];
			// Bottom
		OldSet = InitSet;
		InitSet->Next = Z_Malloc(sizeof(*InitSet), PU_BOTS, NULL);
		InitSet = InitSet->Next;
		InitSet->Prev = OldSet;
		InitSet->vs[0] = OldSet->ve[0];
		InitSet->vs[1] = OldSet->ve[1];
		InitSet->ve[0] = g_GlobalBoundBox[BOXTOP];
		InitSet->ve[1] = g_GlobalBoundBox[BOXLEFT];
			// Left
		OldSet = InitSet;
		InitSet->Next = Z_Malloc(sizeof(*InitSet), PU_BOTS, NULL);
		InitSet = InitSet->Next;
		InitSet->Prev = OldSet;
		InitSet->vs[0] = OldSet->ve[0];
		InitSet->vs[1] = OldSet->ve[1];
		InitSet->ve[0] = g_GlobalBoundBox[BOXBOTTOM];
		InitSet->ve[1] = g_GlobalBoundBox[BOXLEFT];
		InitSet->Next = FirstSet;
		FirstSet->Prev = InitSet;
		
		// Recursive map generation
		B_GHOST_RecursiveSplitMap(&nodes[numnodes - 1], FirstSet);
		
		// Don't do anything else
		l_SSMCreated = true;
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


