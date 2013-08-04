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
// DESCRIPTION: Level Trianglulation

/***************
*** INCLUDES ***
***************/

#include "sn_polyg.h"
#include "p_info.h"
#include "p_demcmp.h"
#include "m_bbox.h"
#include "r_local.h"
#include "sn_main.h"
#include "vhw_wrap.h"
#include "b_bot.h"
#include "z_zone.h"
#include "i_util.h"
#include "p_info.h"
#include "p_local.h"

/*****************
*** STRUCTURES ***
*****************/

#define __REMOOD_DOUBLEPOLY

#if defined(__REMOOD_DOUBLEPOLY)
	typedef double polyf_t;
	
	#define POLYFT_C(x) ((double)(x))
	#define FIXEDTOPOLYF(x) ((polyf_t)(FIXED_TO_FLOAT(x)))
	#define POLYFTOFIXED(x) ((fixed_t)(FLOAT_TO_FIXED(x)))
	#define POLYFDIV(a,b) (((polyf_t)(a)) / ((polyf_t)(b)))
	#define POLYFMUL(a,b) (((polyf_t)(a)) * ((polyf_t)(b)))
#else
	typedef fixed_t polyf_t;
	
	#define FIXEDTOPOLYF(x) (x)
	#define POLYFTOFIXED(x) (x)
#endif

/* SN_Point_t -- Standard point */
typedef struct SN_Point_s
{
	polyf_t v[2];
} SN_Point_t;

/* SN_Poly_t -- Polygon */
typedef struct SN_Poly_s
{
	SN_Point_t** Pts;
	uint32_t NumPts;
} SN_Poly_t;

/**************
*** GLOBALS ***
**************/

/*************
*** LOCALS ***
*************/

/****************
*** FUNCTIONS ***
****************/

/* SN_DumpLevel() -- Dumps the level */
void SN_DumpLevel(void)
{
#define BUFSIZE 128
	int32_t i;
	I_File_t* File;
	char Buf[BUFSIZE];
	
	/* Build FileName */
	// Directory to dump
	snprintf(Buf, BUFSIZE - 1, "graph");
	I_mkdir(Buf, 0);
	
	// Level
	snprintf(Buf, BUFSIZE - 1, "graph/%s", g_CurrentLevelInfo->LumpName);
	I_mkdir(Buf, 0);
	
	// Actual Data
	snprintf(Buf, BUFSIZE - 1, "graph/%s/level", g_CurrentLevelInfo->LumpName);
	File = I_FileOpen(Buf, IFM_RWT);
	
	// Failed?
	if (!File)
		return;
	
	/* Dump all lines */
	for (i = 0; i < numlines; i++)
	{
		// Not a blocking line?
		if (!(lines[i].flags & ML_BLOCKING))
			continue;
		
		// Move to base position
		snprintf(Buf, BUFSIZE - 1, "move %i.0 %i.0\n",
				lines[i].v1->x >> 16, lines[i].v1->y >> 16
			);
		I_FileWrite(File, Buf, strlen(Buf));
		
		// Line to end position
		snprintf(Buf, BUFSIZE - 1, "%i.0 %i.0\n",
				lines[i].v2->x >> 16, lines[i].v2->y >> 16
			);
		I_FileWrite(File, Buf, strlen(Buf));
	}
	
	/* Close File */
	I_FileClose(File);

#undef BUFSIZE
}

/* SN_DumpPoly() -- Dumps polygon */
void SN_DumpPoly(SN_Poly_t* const a_Poly, const char a_Code, const int32_t a_Num)
{
#define BUFSIZE 128
	int32_t i;
	I_File_t* File;
	char Buf[BUFSIZE];
	
	/* Build FileName */
	// Directory to dump
	snprintf(Buf, BUFSIZE - 1, "graph");
	I_mkdir(Buf, 0);
	
	// Level
	snprintf(Buf, BUFSIZE - 1, "graph/%s", g_CurrentLevelInfo->LumpName);
	I_mkdir(Buf, 0);
	
	// Actual Data
	snprintf(Buf, BUFSIZE - 1, "graph/%s/%c-%04x", g_CurrentLevelInfo->LumpName, a_Code, a_Num);
	File = I_FileOpen(Buf, IFM_RWT);
	
	// Failed?
	if (!File)
		return;
	
	/* Dump all lines */
	for (i = 0; i < a_Poly->NumPts; i++)
	{
		// Move to base position
		snprintf(Buf, BUFSIZE - 1, "move %i.0 %i.0\n",
				POLYFTOFIXED(a_Poly->Pts[i]->v[0]) >> 16, POLYFTOFIXED(a_Poly->Pts[i]->v[1]) >> 16
			);
		I_FileWrite(File, Buf, strlen(Buf));
		
		// Line to end position
		snprintf(Buf, BUFSIZE - 1, "%i.0 %i.0\n",
				POLYFTOFIXED(a_Poly->Pts[(i + 1) % a_Poly->NumPts]->v[0]) >> 16, POLYFTOFIXED(a_Poly->Pts[(i + 1) % a_Poly->NumPts]->v[1]) >> 16
			);
		I_FileWrite(File, Buf, strlen(Buf));
	}
	
	/* Close File */
	I_FileClose(File);

#undef BUFSIZE
}

/* SN_PolyCenter() -- Finds center of polygon */
// Uses 64-bit due to number limitations
SN_Point_t SN_PolyCenter(SN_Poly_t* const a_Poly)
{
	SN_Point_t RetVal;
	int64_t bX, bY;
	int32_t i;
	
	/* Init */
	memset(&RetVal, 0, sizeof(RetVal));
	
	/* Add all points */
	bX = bY = 0;
	for (i = 0; i < a_Poly->NumPts; i++)
	{
		bX += POLYFTOFIXED(a_Poly->Pts[i]->v[0]);
		bY += POLYFTOFIXED(a_Poly->Pts[i]->v[1]);
	}
	
	/* Center is the average of all the points */
	RetVal.v[0] = FIXEDTOPOLYF(bX / (int64_t)a_Poly->NumPts);
	RetVal.v[1] = FIXEDTOPOLYF(bY / (int64_t)a_Poly->NumPts);
	
	/* Return Center */
	return RetVal;
}

/* SN_PolyAddPoint() -- Adds point to polygon */
SN_Poly_t* SN_PolyAddPoint(SN_Poly_t* const a_Poly, const polyf_t a_X, const polyf_t a_Y)
{
	SN_Poly_t* Poly;
	SN_Point_t* Point;
	
	/* Create new poly? */
	if (!a_Poly)
		Poly = Z_Malloc(sizeof(*Poly), PU_POLYGON, NULL);
	
	/* or use existing one */
	else
		Poly = a_Poly;
	
	/* Create New point */
	Point = Z_Malloc(sizeof(*Point), PU_POLYGON, NULL);
	
	Point->v[0] = a_X;
	Point->v[1] = a_Y;
	
	/* Add point to end of polygon */
	Z_ResizeArray((void**)&Poly->Pts, sizeof(*Poly->Pts),
		Poly->NumPts, Poly->NumPts + 1);
	Poly->Pts[Poly->NumPts++] = Point;
	
	/* Return polygon */
	return Poly;
}

/* SN_DiscardPoly() -- Discards a polygon */
void SN_DiscardPoly(SN_Poly_t* const a_Poly)
{
	int32_t i;
	
	/* Check */
	if (!a_Poly)
		return;
	
	/* Destroy All Points */
	if (a_Poly->Pts)
	{
		for (i = 0; i < a_Poly->NumPts; i++)
			if (a_Poly->Pts[i])
				Z_Free(a_Poly->Pts[i]);
		
		Z_Free(a_Poly->Pts);
	}
	
	// Clear final poly holder
	Z_Free(a_Poly);
}

/* SN_ClonePoly() -- Clones a polygon */
SN_Poly_t* SN_ClonePoly(SN_Poly_t* const a_Poly)
{
	int32_t i;
	SN_Poly_t* New;
	
	/* Check */
	if (!a_Poly)
		return NULL;
	
	/* Start clone */
	for (New = NULL, i = 0; i < a_Poly->NumPts; i++)
		New = SN_PolyAddPoint(New, a_Poly->Pts[i]->v[0], a_Poly->Pts[i]->v[1]);
	
	/* Return new polygon */
	return New;
}

/* SN_GetIntercept() -- Gets the interception point of two lines */
bool_t SN_GetIntercept(SN_Point_t* const a_Out, SN_Point_t* const a_Aa, SN_Point_t* const a_Ab, SN_Point_t* const a_Ba, SN_Point_t* const a_Bb)
{
	polyf_t mA, mB;
	polyf_t bA, bB;
	
	/* If both lines are vertical */
	if (a_Ab->v[0] - a_Aa->v[0] == POLYFT_C(0) && a_Bb->v[0] - a_Ba->v[0] == POLYFT_C(0))
	{
		// They will never meet
		return false;
	}
	
	/* If the first line is vertical */
	else if (a_Ab->v[0] - a_Aa->v[0] == POLYFT_C(0))
	{
		// Get slope of B
		mB = POLYFDIV(a_Bb->v[1] - a_Ba->v[1], a_Bb->v[0] - a_Ba->v[0]);
		
		// Get Y-Intercept of B
		bB = a_Ba->v[1] - POLYFMUL(mB, a_Ba->v[0]);
		
		// The point will always meet on the vertical line's X coord
		a_Out->v[0] = a_Ab->v[0];
		a_Out->v[1] = POLYFMUL(mB, a_Ab->v[0]) + bB;
		
		// They intercept, at some point
		return true;
	}
	
	/* If the second line is vertical */
	else if (a_Bb->v[0] - a_Ba->v[0] == POLYFT_C(0))
	{
		// Get slope of A
		mA = POLYFDIV(a_Ab->v[1] - a_Aa->v[1], a_Ab->v[0] - a_Aa->v[0]);
		
		// Get Y-Intercept of A
		bA = a_Aa->v[1] - POLYFMUL(mA, a_Aa->v[0]);
		
		// The point will always meet on the vertical line's X coord
		a_Out->v[0] = a_Bb->v[0];
		a_Out->v[1] = POLYFMUL(mA, a_Bb->v[0]) + bA;
		
		// They intercept, at some point
		return true;
	}
	
	/* If neither lines are vertical */
	else
	{
		// Get slopes of A and B
		mA = POLYFDIV(a_Ab->v[1] - a_Aa->v[1], a_Ab->v[0] - a_Aa->v[0]);
		mB = POLYFDIV(a_Bb->v[1] - a_Ba->v[1], a_Bb->v[0] - a_Ba->v[0]);
		
		// If the slopes are the same, the lines are parallel
		if (mA == mB)
			return false;
		
		// Get Y Intercepts of A and B
		bA = a_Aa->v[1] - POLYFMUL(mA, a_Aa->v[0]);
		bB = a_Ba->v[1] - POLYFMUL(mB, a_Ba->v[0]);
		
		// Calculate X of one line
		// The slope of a line might be zero, so...
			// ((y - b) / m) = x
			// y = mx + b
		if (mA == POLYFT_C(0))
		{
			a_Out->v[0] = POLYFDIV(a_Ba->v[1] - bB, mB);
			a_Out->v[1] = POLYFMUL(mA, a_Out->v[0]) + bA;
		}
		else
		{
			a_Out->v[0] = POLYFDIV(a_Aa->v[1] - bA, mA);
			a_Out->v[1] = POLYFMUL(mB, a_Out->v[0]) + bB;
		}
		
		if (isnan(a_Out->v[0]) || isnan(a_Out->v[1]))
			I_Error("Nan!");
		
		// They intercept, at some point
		return true;
	}
	
	/* Presume they never met? */
	return false;
}

/* SN_PolySplit() -- Splits polygon by line */
void SN_PolySplit(SN_Poly_t* const a_BasePoly, SN_Poly_t** const a_SideA, SN_Poly_t** const a_SideB, SN_Point_t* const a_A, SN_Point_t* const a_B)
{
	int32_t s, i;
	SN_Point_t ICept;
	bool_t OK;
	SN_Point_t* lA, *lB;
	int32_t Crosses[2][2];
	SN_Point_t CPts[2];
	
	/* Initialize */
	s = 0;
	memset(Crosses, 0, sizeof(Crosses));
	memset(CPts, 0, sizeof(CPts));
	
	/* Go through all lines of the polygon */
	for (i = 0; i < a_BasePoly->NumPts; i++)
	{
		// Get points to check
		lA = a_BasePoly->Pts[i];
		lB = a_BasePoly->Pts[(i + 1) % a_BasePoly->NumPts];
		
		// Find interception of this point
		memset(&ICept, 0, sizeof(ICept));
		OK = SN_GetIntercept(&ICept, lA, lB, a_A, a_B);
		
		// No intercept detected
		if (!OK)
			continue;
		
		// See if the interception point exceeds the line bounds
			// Only one of these should meet
		if ((!((lA->v[0] < lB->v[0] &&
					ICept.v[0] > lA->v[0] && ICept.v[0] < lB->v[0]) ||
			(lA->v[0] > lB->v[0] &&
					ICept.v[0] < lA->v[0] && ICept.v[0] > lB->v[0]))) &&
			(!((lA->v[1] < lB->v[1] &&
					ICept.v[1] > lA->v[1] && ICept.v[1] < lB->v[1]) ||
			(lA->v[1] > lB->v[1] &&
					ICept.v[1] < lA->v[1] && ICept.v[1] > lB->v[1]))))
			continue;
		
		// Add to cross list
		if (s < 2)
		{
			CPts[s] = ICept;
			Crosses[s][0] = i;
			Crosses[s++][1] = (i + 1) % a_BasePoly->NumPts;
		}
		
		// Bug?
		else
			s++;
	}
	
	/* Not enough crosses? */
	if (s < 2)
		return;
	
	/* Rebuild the first polygon */
	// Add all points before the first interception, including the start point
	// on the crossed line
	for (i = 0; i <= Crosses[0][0]; i++)
		*a_SideA = SN_PolyAddPoint(*a_SideA,
					a_BasePoly->Pts[i]->v[0], a_BasePoly->Pts[i]->v[1]);
	
	// Now add the division line
		// First point
	*a_SideA = SN_PolyAddPoint(*a_SideA, CPts[0].v[0], CPts[0].v[1]);
		// Second point
	*a_SideA = SN_PolyAddPoint(*a_SideA, CPts[1].v[0], CPts[1].v[1]);
	
	// Now start after the second interception, and include any remaining points
		// If the division line is on the last line of the polygon, then no lines
		// will be added here.
	for (i = Crosses[1][0] + 1; i < a_BasePoly->NumPts; i++)
		*a_SideA = SN_PolyAddPoint(*a_SideA,
					a_BasePoly->Pts[i]->v[0], a_BasePoly->Pts[i]->v[1]);
	
	/* Now rebuild the second polygon */
	// Add all points after the first cross line until the second cross line
	for (i = Crosses[0][0] + 1; i <= Crosses[1][0]; i++)
		*a_SideB = SN_PolyAddPoint(*a_SideB,
					a_BasePoly->Pts[i]->v[0], a_BasePoly->Pts[i]->v[1]);
	
	// Now the final line is the division line, but unlike the first polygon
	// the far second crossing is the first point.
		// First point
	*a_SideB = SN_PolyAddPoint(*a_SideB, CPts[1].v[0], CPts[1].v[1]);
		// Second point
	*a_SideB = SN_PolyAddPoint(*a_SideB, CPts[0].v[0], CPts[0].v[1]);
}

/* SN_PolyFigureOutSide() -- Figures out the side of the polygon */
void SN_PolyFigureOutSide(SN_Poly_t** const a_A, SN_Poly_t** const a_B, int32_t (*a_SideFunc)(SN_Point_t* const a_Point, void* const a_Data), void* const a_Data)
{
#define MAXSIDES 32
	SN_Point_t Mids[2][MAXSIDES];
	SN_Point_t Work;
	SN_Poly_t* CheckPoly;
	int32_t i, j, k, Sides[2][2], Weight[2];
	
	/* Check */
	if (!a_A || !a_B || !a_SideFunc)
		return;
	
	/* Init */
	memset(Mids, 0, sizeof(Mids));
	memset(Sides, 0, sizeof(Sides));
	memset(Weight, 0, sizeof(Weight));
	
	/* Get mid points of both polygons */
	// This will help determine the side the polygon belongs to
	for (i = 0; i < 2; i++)
	{
		// Get which polygon to check
		CheckPoly = (!i ? *a_A : *a_B);
		
		// Get mid points
		for (j = 0; j < CheckPoly->NumPts && j < MAXSIDES; j++)
		{
			Mids[i][j].v[0] = POLYFDIV(CheckPoly->Pts[j]->v[0] + CheckPoly->Pts[(j + 1) % CheckPoly->NumPts]->v[0], POLYFT_C(2));
			Mids[i][j].v[1] = POLYFDIV(CheckPoly->Pts[j]->v[1] + CheckPoly->Pts[(j + 1) % CheckPoly->NumPts]->v[1], POLYFT_C(2));
		}
	}
	
	/* Get mid points of the midpoints against the other midpoints */
	for (i = 0; i < 2; i++)
	{
		// Get which polygon to check
		CheckPoly = (!i ? *a_A : *a_B);
		
		// Check midpoint vs other midpoint
		for (j = 0; j < CheckPoly->NumPts && j < MAXSIDES; j++)
			for (k = 0; k < CheckPoly->NumPts && k < MAXSIDES; k++)
			{
				// Already checked this side, or is this side?
				if (k <= j)
					continue;
				
				// Calculate midpoint from these midpoints
				Work.v[0] = POLYFDIV(Mids[i][j].v[0] + Mids[i][k].v[0], POLYFT_C(2));
				Work.v[1] = POLYFDIV(Mids[i][j].v[1] + Mids[i][k].v[1], POLYFT_C(2));
				
				// Calculate side it is on and place in array
				Sides[i][a_SideFunc(&Work, a_Data)]++;
			}
	}
	
	
	/* Get weight of both polygons, to determine the side they are on */
	for (i = 0; i < 2; i++)
	{
		// Front sides move positive, back sides move negative
		Weight[i] += Sides[i][0];
		Weight[i] -= Sides[i][1];
	}
		
	/*CONL_PrintF("[%i, %i], [%i, %i]; w = %i, %i\n",
			Sides[0][0], Sides[0][1],
			Sides[1][0], Sides[1][1],
			Weight[0], Weight[1]
		);*/
	
	/* Move polygons based on weight */
	// A heavier than B
	if (Weight[0] > Weight[1])
	{
		// Nothing to be done here as side 0 relates to A already
	}
	
	// B heavier than A
		// This means that B is on the front side really (side 0)
	else if (Weight[1] > Weight[0])
	{
		CheckPoly = *a_B;
		*a_B = *a_A;
		*a_A = CheckPoly;
	}
#undef MAXSIDES
}

//typedef struct B_GhostNode_s* B_GhostNode_t;
//B_GhostNode_t* B_NodeCreate(const fixed_t a_X, const fixed_t a_Y, const fixed_t a_Z);

/* SNS_SubSPointOnSide() -- which side point is on */
static int32_t SNS_SubSPointOnSide(SN_Point_t* const a_Point, void* const a_Data)
{
	return R_PointOnSegSide(POLYFTOFIXED(a_Point->v[0]), POLYFTOFIXED(a_Point->v[1]), a_Data);
}

/* SN_PolySplitSubS() -- Split polygon by subsector */
void SN_PolySplitSubS(SN_Poly_t* const a_BasePoly, subsector_t* const a_SubS)
{
	int32_t i;
	SN_Poly_t* A, *B;
	SN_Poly_t* Keeper;
	SN_Point_t Str, End, cA;
	
	/* Setup base polygon to keep */
	Keeper = SN_ClonePoly(a_BasePoly);
	
	/* Split subsector by all the seg lines */
	for (i = a_SubS->firstline; i < (a_SubS->firstline + a_SubS->numlines); i++)
	{
		// Init
		A = B = NULL;
		memset(&Str, 0, sizeof(Str));
		memset(&End, 0, sizeof(End));
		
		// Get seg line to split with
		Str.v[0] = FIXEDTOPOLYF(segs[i].v1->x);
		Str.v[1] = FIXEDTOPOLYF(segs[i].v1->y);
		End.v[0] = FIXEDTOPOLYF(segs[i].v2->x);
		End.v[1] = FIXEDTOPOLYF(segs[i].v2->y);
		
		// Split
		SN_PolySplit(Keeper, &A, &B, &Str, &End);
		
		// Bad split?
		if (!A || !B)
		{
			if (A)
				SN_DiscardPoly(A);
			if (B)
				SN_DiscardPoly(B);
			continue;
		}
		
		// Find out which polygons belong to which side
		SN_PolyFigureOutSide(&A, &B, SNS_SubSPointOnSide, &segs[i]);
		
		// Only keep the front side and trash the old poly
		SN_DiscardPoly(B);
		SN_DiscardPoly(Keeper);
		Keeper = A;
	}
	
	/* Dump the polygon that was split, hopefully correctly */
	if (g_SnowBug)
		SN_DumpPoly(Keeper, 's', a_SubS - subsectors);
	
	/* Do bot stuff */
	// Get Center
	cA = SN_PolyCenter(Keeper);
	
	// Set subsector center
	a_SubS->PolyValid = true;
	a_SubS->CenterX = POLYFTOFIXED(cA.v[0]);
	a_SubS->CenterY = POLYFTOFIXED(cA.v[1]);
	
	// Add node there
	//B_NodeCreate(POLYFTOFIXED(cA.v[0]), POLYFTOFIXED(cA.v[1]), ONFLOORZ);
	
	// Set polygon
	a_SubS->Poly = Keeper;
}

/* SNS_NodePointOnSide() -- which side point is on */
static int32_t SNS_NodePointOnSide(SN_Point_t* const a_Point, void* const a_Data)
{
	return R_PointOnSide(POLYFTOFIXED(a_Point->v[0]), POLYFTOFIXED(a_Point->v[1]), a_Data);
}

/* SN_PolySplitNode() -- Split Polygon By Node */
void SN_PolySplitNode(SN_Poly_t* const a_BasePoly, node_t* const a_Node)
{
	SN_Poly_t* A, *B;
	SN_Point_t Str, End;
	int32_t i;
	
	/* Dump the polygon that needs splitting */
	if (g_SnowBug)
		SN_DumpPoly(a_BasePoly, 'n', a_Node - nodes);
	
	/* Init */
	A = B = NULL;
	memset(&Str, 0, sizeof(Str));
	memset(&End, 0, sizeof(End));
	
	// Get the node division line
	Str.v[0] = FIXEDTOPOLYF(a_Node->x);
	Str.v[1] = FIXEDTOPOLYF(a_Node->y);
	End.v[0] = FIXEDTOPOLYF(a_Node->x + a_Node->dx);
	End.v[1] = FIXEDTOPOLYF(a_Node->y + a_Node->dy);
	
	/* Split by the node line getting two polygons */
	SN_PolySplit(a_BasePoly, &A, &B, &Str, &End);
	
	// Failed to split?
	if (!A || !B)
		return;
	
	/* Debug */
	if (g_SnowBug)
	{
		SN_DumpPoly(A, 'a', a_Node - nodes);
		SN_DumpPoly(B, 'b', a_Node - nodes);
	}
	
	/* Determine which polygon refers to which side of the node line */
	// A will be children[0]
	// B will be children[1]
	
	// Find out which polygons belong to which side
	SN_PolyFigureOutSide(&A, &B, SNS_NodePointOnSide, a_Node);
	
	/* Continue splitting */
	for (i = 0; i < 2; i++)
	{
		// Is a subsector, split by segs
		if (a_Node->children[i] & NF_SUBSECTOR)
			SN_PolySplitSubS((!i ? A : B), &subsectors[(a_Node->children[i] & ~NF_SUBSECTOR)]);
		
		// Is another node, traverse
		else
			SN_PolySplitNode((!i ? A : B), &nodes[a_Node->children[i]]);
		
		// Discard polygon side
		SN_DiscardPoly((!i ? A : B));
	}
}

extern fixed_t g_GlobalBoundBox[4];

/* SN_PolygonizeLevel() -- Polygonizes the level */
void SN_PolygonizeLevel(void)
{
	SN_Poly_t* RootPoly;
	
	/* Clear old polygon stuff */
	Z_FreeTags(PU_POLYGON, PU_POLYGON);
	
	/* No level? */
	if (gamestate != GS_LEVEL)
		return;
	
	/* Create box around entire level */
	RootPoly = SN_PolyAddPoint(NULL,
				FIXEDTOPOLYF(g_GlobalBoundBox[BOXLEFT]), FIXEDTOPOLYF(g_GlobalBoundBox[BOXTOP]));
	RootPoly = SN_PolyAddPoint(RootPoly,
				FIXEDTOPOLYF(g_GlobalBoundBox[BOXRIGHT]), FIXEDTOPOLYF(g_GlobalBoundBox[BOXTOP]));
	RootPoly = SN_PolyAddPoint(RootPoly,
				FIXEDTOPOLYF(g_GlobalBoundBox[BOXRIGHT]), FIXEDTOPOLYF(g_GlobalBoundBox[BOXBOTTOM]));
	RootPoly = SN_PolyAddPoint(RootPoly,
				FIXEDTOPOLYF(g_GlobalBoundBox[BOXLEFT]), FIXEDTOPOLYF(g_GlobalBoundBox[BOXBOTTOM]));
	
	// Debug
	if (g_SnowBug)
		SN_DumpLevel();
	
	/* Split entire level by the nodes */
	SN_PolySplitNode(RootPoly, &nodes[numnodes - 1]);
	SN_DiscardPoly(RootPoly);
}

/* SN_DrawPolyLines() -- Debug drawing polygons */
void SN_DrawPolyLines(void* const a_Data, void (*a_DrawFunc)(void* const, const fixed_t, const fixed_t, const fixed_t, const fixed_t, const uint8_t, const uint8_t, const uint8_t))
{
	int32_t i, j, k;
	SN_Poly_t* Poly;
	uint32_t RGB[3] = {63, 0, 0};
	SN_Point_t* PtA, *PtB;
	
	for (i = 0; i < numsubsectors; i++)
	{
		// Get poly
		Poly = subsectors[i].Poly;
		
		// None?
		if (!Poly)
			continue;
		
		// Move color ahead
		RGB[0] += 63;
		
		if (RGB[0] > 255)
		{
			RGB[1] += 63;
			RGB[0] = 0;
			
			if (RGB[1] > 255)
			{
				RGB[2] += 63;
				RGB[1] = 0;
				
				if (RGB[2] > 255)
				{
					RGB[0] = 63;
					RGB[1] = 0;
					RGB[2] = 0;
				}
			}
		}
		
		// Draw each point
		for (j = 0; j < Poly->NumPts; j++)
		{
			PtA = Poly->Pts[j];
			PtB = Poly->Pts[(j + 1) % Poly->NumPts];
			
			a_DrawFunc(a_Data, POLYFTOFIXED(PtA->v[0]), POLYFTOFIXED(PtA->v[1]), POLYFTOFIXED(PtB->v[0]), POLYFTOFIXED(PtB->v[1]), RGB[0], RGB[1], RGB[2]);
		}
	}
}

