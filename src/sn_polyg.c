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
		CONL_PrintF("A is vert\n");
		
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
		CONL_PrintF("B is vert\n");
		
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
			
		CONL_PrintF("Cross Detected (%f, %f)...\n", ICept.v[0], ICept.v[1]);
		
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
		
		CONL_PrintF("X/Y OK!\n");
		
		// Add to cross list
		if (s < 2)
		{
			CPts[s] = ICept;
			Crosses[s][0] = i;
			Crosses[s++][1] = (i + 1) % a_BasePoly->NumPts;
		}
		
		// Bug?
		else
		{
			s++;
			CONL_PrintF("Concave polygon detected!\n");
		}
	}
	
	/* Not enough crosses? */
	if (s < 2)
	{
		CONL_PrintF("Only %i crosses.\n", s);
		return;
	}
	
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

/* SN_PolySplitNode() -- Split Polygon By Node */
void SN_PolySplitNode(SN_Poly_t* const a_BasePoly, node_t* const a_Node)
{
	SN_Poly_t* A, *B, *C;
	SN_Point_t Str, End;
	int32_t i;
	
	/* Dump the polygon that needs splitting */
	if (devparm)
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
	{
		CONL_PrintF("Unable to split polygon\n");
		return;
	}
	
	/* Debug */
	if (devparm)
	{
		SN_DumpPoly(A, 'a', a_Node - nodes);
		SN_DumpPoly(B, 'b', a_Node - nodes);
	}
	
	/* Determine which polygon refers to which side of the node line */
	// A will be children[0]
	// B will be children[1]
	
	/* Continue splitting */
	for (i = 0; i < 2; i++)
	{
		// Is a subsector, split by segs
		if (a_Node->children[i] & NF_SUBSECTOR)
		{
			SN_DumpPoly(A, 's', a_Node->children[i] & NF_SUBSECTOR);
		}
		
		// Is another node, traverse
		else
			SN_PolySplitNode((!i ? A : B), &nodes[a_Node->children[i]]);
	}
}

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
	if (devparm)
		SN_DumpLevel();
	
	/* Split entire level by the nodes */
	SN_PolySplitNode(RootPoly, &nodes[numnodes - 1]);
}


