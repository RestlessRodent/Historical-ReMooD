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

/*****************
*** STRUCTURES ***
*****************/

/* SN_Vert_t -- Vertex */
typedef struct SN_Vert_s
{
	fixed_t v[2];								// Points
} SN_Vert_t;

/* SN_Poly_t -- Polygon point */
typedef struct SN_Poly_s
{
	SN_Vert_t v;								// Polygon vertex
	struct SN_Poly_s* Next;						// Next
	struct SN_Poly_s* Prev;						// Previous
} SN_Poly_t;

/**************
*** GLOBALS ***
**************/

/*************
*** LOCALS ***
*************/

static P_LevelInfoEx_t* l_PLLevel = NULL;		// Last level loaded
static bool_t l_PLFlipped = false;				// Level was flipped

/****************
*** FUNCTIONS ***
****************/

/* SNS_DebugDrawPoly() -- Debugs drawing polygon */
static void SNS_DebugDrawPoly(SN_Poly_t* a_Poly, const char* const a_Name, const int a_Num)
{
#if defined(_DEBUG)
	FILE* f;
	char Buf[PATH_MAX];
	SN_Poly_t* Rover;
	
	// Directory Tree
	snprintf(Buf, PATH_MAX - 1, "graph");
	I_mkdir(Buf, 0);
	snprintf(Buf, PATH_MAX - 1, "graph/%s", WL_GetWADName(g_CurrentLevelInfo->WAD, false));
	I_mkdir(Buf, 0);
	snprintf(Buf, PATH_MAX - 1, "graph/%s/%s", WL_GetWADName(g_CurrentLevelInfo->WAD, false), g_CurrentLevelInfo->LumpName);
	I_mkdir(Buf, 0);
	
	snprintf(Buf, PATH_MAX - 1, "graph/%s/%s/%s-%04i", WL_GetWADName(g_CurrentLevelInfo->WAD, false), g_CurrentLevelInfo->LumpName, a_Name, a_Num);
	f = fopen(Buf, "w+");
	
	if (f)
	{
		Rover = a_Poly;
		
		do
		{
			fprintf(f, "%i %i\n", Rover->v.v[0] >> FRACBITS, Rover->v.v[1] >> FRACBITS);
			Rover = Rover->Next;
		} while (Rover != a_Poly);
		
		fprintf(f, "%i %i\n", Rover->v.v[0] >> FRACBITS, Rover->v.v[1] >> FRACBITS);
		
		fclose(f);
	}
#endif
}

/* SNS_AddVertex() -- Adds vertex to polygon */
static SN_Poly_t* SNS_AddVertex(SN_Poly_t* const a_Base, SN_Vert_t* const a_Vert)
{
	SN_Poly_t* Sel, *Bef;
	
	/* Initial Polygon */
	if (!a_Base)
	{
		Sel = Z_Malloc(sizeof(*Sel), PU_STATIC, NULL);
		Sel->Prev = Sel;
		Sel->Next = Sel;
		
		Sel->v = *a_Vert;
		
		return Sel;
	}
	
	/* Append to existing one */
	else
	{
		// Get last polygon
		Bef = a_Base->Prev;
		
		// Set next
		Bef->Next = Sel = Z_Malloc(sizeof(*Sel), PU_STATIC, NULL);
		Sel->Prev = Bef;
		Sel->Next = a_Base;
		a_Base->Prev = Sel;
		
		// Value
		Sel->v = *a_Vert;
		
		// Always return base
		return a_Base;
	}
}

/* SN_LineProp_t -- Properties of line */
typedef struct SN_LineProp_s
{
	bool_t Vert;
	fixed_t Slope;
	fixed_t YInter;
	
	SN_Vert_t* v[2];
} SN_LineProp_t;

/* SNS_CheckInter() -- Checks for intercept */
static bool_t SNS_CheckInter(SN_LineProp_t* const a_A, SN_LineProp_t* const a_BSN_Vert_t* const a_Out)
{
	fixed_t xx, yy;
	
	/* Both Lines are vertical */
	if (a_A->Vert && a_B->Vert)
		return false;
	
	/* Have same slope */
	else if (a_A->Slope == a_B->Slope)
		return false;
	
	/* Otherwise, calculate intersection */
	else
	{
		// y1 = m1x1 + b1
		// y2 = m2x2 + b2
		
		// m1x1 + b1 = m2x2 + b2
		
		xx = FixedDiv((a_B->YInter - a_A->YInter), (a_A->Slope - a_B->Slope));
		yy = FixedMul(a_A->Slope, xx) + a_A->YInter;
	}
	
	/* No interception */
	return false;
}

/* SNS_GetLineProps() -- Gets property of lines */
static void SNS_GetLineProps(SN_LineProp_t* const a_Out, SN_Vert_t* const a_A, SN_Vert_t* const a_B)
{
	/* Copy Verts */
	a_Out->v[0] = a_A;
	a_Out->v[1] = a_B;
	
	/* Get Slope of line */
	// Vertical?
	if (a_B->v[0] - a_A->v[0] == 0)
		a_Out->Vert = true;
	else
	{
		a_Out->Vert = false;
		a_Out->Slope = FixedDiv(a_B->v[1] - a_A->v[1], a_B->v[0] - a_A->v[0]);
		
		// Get Y Intercept
		YInter = ((-FixedMul(a_Out->Slope, a_A->v[0])) + a_A->v[1]);
	}
}

/* SNS_SplitPoly() -- Splits polygon with line */
static void SNS_SplitPoly(SN_Poly_t* const a_Poly, SN_Vert_t* const a_Line, SN_Poly_t** const a_Left, SN_Poly_t** const a_Right)
{
	SN_LineProp_t SplitProp;
	SN_Poly_t* Rover;
	
	SN_LineProp_t CheckProp;
	
	SN_LineProp_t HitP[2];
	SN_Vert_t HitV[2];
	int HitCount;
	
	/* Get property of split line */
	memset(&SplitProp, 0, sizeof(SplitProp));
	SNS_GetLineProps(&SplitProp, &a_Line[0], &a_Line[1]);
	
	/* Init */
	memset(HitP, 0, sizeof(HitP));
	memset(HitV, 0, sizeof(HitV));
	HitCount = 0;
	
	/* Look through all vertexes */
	Rover = a_Poly;
	do
	{
		// Get properties of this line
		memset(&CheckProp, 0, sizeof(CheckProp));
		SNS_GetLineProps(&CheckProp, &Rover->v, &Rover->Next->v);
		
		// See if line intercepts
		if (SNS_CheckInter(&SplitProp, &CheckProp, &HitV[HitCount]))
		{
			memmove(&HitP[HitCount], &CheckProp, sizeof(CheckProp));
			HitCount++;
		}
		
		// Get next rover
		Rover = Rover->Next;
	} while (Rover != a_Poly);
	
	/* Debug */
	CONL_PrintF("Intercepts: %i\n", HitCount);
}

/* SNS_RunNode() -- Runs a single node */
static void SNS_RunNode(node_t* const a_Node, SN_Poly_t* const a_Poly)
{
	register int i;
	SN_Poly_t* Polies[2];
	SN_Vert_t SplitLine[2];
	
	/* No Polygon? */
	if (!a_Poly)
		return;
	
	/* Debug */
	if (g_SnowBug)
		SNS_DebugDrawPoly(a_Poly, "node", a_Node - nodes);
	
	/* Split Polygon by node line */
	// Rumor has it the node line is inaccurate and the seg line is more
	// accurate, so in the future use the seg line instead?
	SplitLine[0].v[0] = a_Node->x;
	SplitLine[0].v[1] = a_Node->y;
	SplitLine[1].v[0] = a_Node->x + a_Node->dx;
	SplitLine[1].v[1] = a_Node->y + a_Node->dy;
	
	Polies[0] = Polies[1] = NULL;
	SNS_SplitPoly(a_Poly, SplitLine, &Polies[1], &Polies[0]);
	
	/* Traverse Sides */
	// Start with left, go right
	for (i = 1; i >= 0; i--)
	{
		// Subsector?
		if (a_Node->children[i] & NF_SUBSECTOR)
		{
			// Finalize subsector
			//SNS_SubmitSub(&subsectors[(a_Node->children[i] & (~NF_SUBSECTOR))], Polies[i]);
		}
		
		// Node?
		else
		{
			// Triangulate this node further
			SNS_RunNode(&nodes[a_Node->children[i]], Polies[i]);
		}
	}
}

/* SN_PolygonizeLevel() -- Polygonizes the level */
void SN_PolygonizeLevel(void)
{
	int i;
	fixed_t Box[4];
	SN_Vert_t Vert;
	node_t* RootNode;
	SN_Poly_t* RootPoly;
	
	/* Check for change of level */
	if (l_PLLevel == g_CurrentLevelInfo && l_PLFlipped == P_XGSVal(PGS_FUNFLIPLEVELS))
		return;
	
	// Remember, to save time
	l_PLLevel = g_CurrentLevelInfo;
	l_PLFlipped = P_XGSVal(PGS_FUNFLIPLEVELS);
	
	/* Create initial polygon from root node */
	RootNode = &nodes[numnodes - 1];
	RootPoly = NULL;
	M_ClearBox(Box);
	
	// Add nodes to bounding box
	for (i = 0; i < 2; i++)
	{
		M_AddToBox(Box, RootNode->bbox[i][BOXLEFT], RootNode->bbox[i][BOXTOP]);
		M_AddToBox(Box, RootNode->bbox[i][BOXRIGHT], RootNode->bbox[i][BOXBOTTOM]);
	}
	
	// Setup root poly from box
		// Top left
	Vert.v[0] = Box[BOXLEFT];
	Vert.v[1] = Box[BOXTOP];
	RootPoly = SNS_AddVertex(RootPoly, &Vert);
		// Top right
	Vert.v[0] = Box[BOXRIGHT];
	Vert.v[1] = Box[BOXTOP];
	RootPoly = SNS_AddVertex(RootPoly, &Vert);
		// Bottom right
	Vert.v[0] = Box[BOXRIGHT];
	Vert.v[1] = Box[BOXBOTTOM];
	RootPoly = SNS_AddVertex(RootPoly, &Vert);
		// Bottom left
	Vert.v[0] = Box[BOXLEFT];
	Vert.v[1] = Box[BOXBOTTOM];
	RootPoly = SNS_AddVertex(RootPoly, &Vert);
	
	// Split level
	SNS_RunNode(RootNode, RootPoly);
}


