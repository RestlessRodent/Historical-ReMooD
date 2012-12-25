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
// DESCRIPTION: ReMooD Snow Renderer

/***************
*** INCLUDES ***
***************/

#include "sn_main.h"
#include "r_defs.h"
#include "r_state.h"
#include "m_bbox.h"
#include "p_info.h"

/*************
*** LOCALS ***
*************/

/* SN_Vert_t -- Vertex */
typedef struct SN_Vert_s
{
	raster_t x;
	raster_t y;
} SN_Vert_t;

/* SN_Line_t -- Line */
typedef struct SN_Line_s
{
	SN_Vert_t v[2];
} SN_Line_t;

/* SN_Poly_t -- Polygon, which is split */
typedef struct SN_Poly_s
{
	SN_Line_t* Lines;
	size_t NumLines;
} SN_Poly_t;

/****************
*** FUNCTIONS ***
****************/

/* SNS_CorrectPoly() -- Corrects polygon */
void SNS_CorrectPoly(SN_Poly_t* const a_Poly)
{
}

/* SNS_PolyAddLine() -- Adds line to polygon */
SN_Poly_t* SNS_PolyAddLine(SN_Poly_t* const a_Base, SN_Line_t* const a_Line)
{
	SN_Poly_t* Select;
	
	/* Base? */
	if (a_Base)
		Select = a_Base;
	else
		Select = Z_Malloc(sizeof(*Select), PU_STATIC, NULL);
	
	/* Append to end */
	Z_ResizeArray((void**)&Select->Lines, sizeof(*Select->Lines),
		Select->NumLines, Select->NumLines + 1);
	Select->Lines[Select->NumLines++] = *a_Line;
	
	/* Correct polygon */
	SNS_CorrectPoly(Select);
	
	/* Return polygon */
	return Select;
}


/* SNS_FreePoly() -- Frees polygon */
void SNS_FreePoly(SN_Poly_t* const a_Poly)
{
}

/* SNS_ClonePoly() -- Clones a new polygon */
SN_Poly_t* SNS_ClonePoly(SN_Poly_t* const a_Poly)
{
	register int i;
	SN_Poly_t* New;
	
	/* Clone each line */
	New = NULL;
	for (i = 0; i < a_Poly->NumLines; i++)
		New = SNS_PolyAddLine(New, &a_Poly->Lines[i]);
	
	return New;
}

/* SNS_BoxToPoly() -- Converts box to polygon */
SN_Poly_t* SNS_BoxToPoly(fixed_t* const a_Box)
{
	SN_Line_t NewLine;
	SN_Poly_t* Poly;
	
	/* Top */
	NewLine.v[0].x = a_Box[BOXLEFT];
	NewLine.v[0].y = a_Box[BOXTOP];
	NewLine.v[1].x = a_Box[BOXRIGHT];
	NewLine.v[1].y = a_Box[BOXTOP];
	Poly = SNS_PolyAddLine(NULL, &NewLine);
	
	/* Right */
	NewLine.v[0].x = a_Box[BOXRIGHT];
	NewLine.v[0].y = a_Box[BOXTOP];
	NewLine.v[1].x = a_Box[BOXRIGHT];
	NewLine.v[1].y = a_Box[BOXBOTTOM];
	Poly = SNS_PolyAddLine(Poly, &NewLine);
	
	/* Bottom */
	NewLine.v[0].x = a_Box[BOXRIGHT];
	NewLine.v[0].y = a_Box[BOXBOTTOM];
	NewLine.v[1].x = a_Box[BOXLEFT];
	NewLine.v[1].y = a_Box[BOXBOTTOM];
	Poly = SNS_PolyAddLine(Poly, &NewLine);
	
	/* Left */
	NewLine.v[0].x = a_Box[BOXLEFT];
	NewLine.v[0].y = a_Box[BOXBOTTOM];
	NewLine.v[1].x = a_Box[BOXLEFT];
	NewLine.v[1].y = a_Box[BOXTOP];
	Poly = SNS_PolyAddLine(Poly, &NewLine);
	
	/* Return resulting polygon */
	return Poly;
}

/* SNS_LineCross() -- Gets the crossing of two lines */
// See http://en.wikipedia.org/wiki/Line-line_intersection#Mathematics
bool_t SNS_LineCross(SN_Vert_t* a_Out, SN_Line_t* const a_A, SN_Line_t* const a_B)
{
	raster_t x, y;
	
#if 0
	/* Calculate center point */
	// X Position
	x = rDi(rSu(rMu(rSu(rMu(a_A->v[0].x, a_A->v[1].y), rMu(a_A->v[0].y, a_A->v[1].x)), rSu(a_B->v[0].x, a_B->v[1].x)), rMu(rSu(a_A->v[0].x, a_A->v[1].x), rSu(rMu(a_B->v[0].x, a_B->v[1].y), rMu(a_B->v[0].y, a_B->v[1].x)))), rSu(rMu(rSu(a_A->v[0].x, a_A->v[1].x), rSu(a_B->v[0].y, a_B->v[1].y)), rMu(rSu(a_A->v[0].y, a_A->v[1].y), rSu(a_B->v[0].x, a_B->v[1].x))));
	
	// Y position
	y = rDi(rSu(rMu(rSu(rMu(a_A->v[0].x, a_A->v[1].y), rMu(a_A->v[0].y, a_A->v[1].x)), rSu(a_B->v[0].y, a_B->v[1].y)), rMu(rSu(a_A->v[0].y, a_A->v[1].y), rSu(rMu(a_B->v[0].x, a_B->v[1].y), rMu(a_B->v[0].y, a_B->v[1].x)))), rSu(rMu(rSu(a_A->v[0].x, a_A->v[1].x), rSu(a_B->v[0].y, a_B->v[1].y)), rMu(rSu(a_A->v[0].y, a_A->v[1].y), rSu(a_B->v[0].x, a_B->v[1].x))));
#else
	/* Calculate center point */
	
#endif

	return false;
}

/* SNS_SplitPolyByLine() -- Splits polygon by line */
void SNS_SplitPolyByLine(SN_Poly_t* const a_In, SN_Line_t* const a_Line, SN_Poly_t** const a_LeftPtr, SN_Poly_t** const a_RightPtr)
{
	register int i;
	int h;
	SN_Line_t* HitL[2];
	SN_Vert_t HitV[2];
	
	/* Init */
	memset(HitL, 0, sizeof(HitL));
	memset(HitV, 0, sizeof(HitV));
	h = 0;
	
	/* Cross on all sides of the polygon */
	for (i = 0; i < a_In->NumLines; i++)
		if (SNS_LineCross(&HitV[h], a_Line, &a_In->Lines[i]))
		{
			HitL[h] = &a_In->Lines[i];
			h++;
		}
	
	/* Hit polygon */
	if (h >= 2)
	{
	}
	
	/* Missed polygon */
	else
	{
		*a_LeftPtr = SNS_ClonePoly(a_In);
		*a_RightPtr = SNS_ClonePoly(a_In);
	}
	
	/* Correct both sides */
	SNS_CorrectPoly(*a_LeftPtr);
	SNS_CorrectPoly(*a_RightPtr);
}

/* SNS_SubmitSub() -- Submits subsector */
void SNS_SubmitSub(subsector_t* const a_SubS, SN_Poly_t* const a_Poly)
{
	register int i;
#if defined(_DEBUG)
	FILE* f;
	char Buf[PATH_MAX];
#endif

	/* Debug */
	if (g_SnowBug)
	{
		CONL_PrintF("SubS %u\n", ((unsigned int)(a_SubS - subsectors)));
		
#if defined(_DEBUG)
		// Directory Tree
		snprintf(Buf, PATH_MAX - 1, "graph");
		I_mkdir(Buf, 0);
		snprintf(Buf, PATH_MAX - 1, "graph/%s", WL_GetWADName(g_CurrentLevelInfo->WAD, false));
		I_mkdir(Buf, 0);
		snprintf(Buf, PATH_MAX - 1, "graph/%s/%s", WL_GetWADName(g_CurrentLevelInfo->WAD, false), g_CurrentLevelInfo->LumpName);
		I_mkdir(Buf, 0);
		
		snprintf(Buf, PATH_MAX - 1, "graph/%s/%s/sub-%04i", WL_GetWADName(g_CurrentLevelInfo->WAD, false), g_CurrentLevelInfo->LumpName, a_SubS - subsectors);
		f = fopen(Buf, "w+");
		
		if (f)
		{
			for (i = 0; i < a_Poly->NumLines; i++)
			{
				fprintf(f, "%i %i\n",
						SN_RasterToInt(a_Poly->Lines[i].v[0].x),
						SN_RasterToInt(a_Poly->Lines[i].v[1].x)
						);
						
				fprintf(f, "%i %i\n",
						SN_RasterToInt(a_Poly->Lines[(i + 1) % a_Poly->NumLines].v[0].x),
						SN_RasterToInt(a_Poly->Lines[(i + 1) % a_Poly->NumLines].v[1].x)
						);
			}
			
			fclose(f);
		}
#endif
	}
}

/* SNS_Triangulate() -- Triangulates Nodes */
void SNS_Triangulate(node_t* const a_Node, SN_Poly_t* const a_Poly)
{
	register int i;
	SN_Poly_t* Polies[2];
	SN_Line_t SplitLine;
#if defined(_DEBUG)
	FILE* f;
	char Buf[PATH_MAX];
#endif
	
	/* Debug */
	if (g_SnowBug)
	{
		CONL_PrintF("Node %u\n", ((unsigned int)(a_Node - nodes)));
		
#if defined(_DEBUG)
		// Directory Tree
		snprintf(Buf, PATH_MAX - 1, "graph");
		I_mkdir(Buf, 0);
		snprintf(Buf, PATH_MAX - 1, "graph/%s", WL_GetWADName(g_CurrentLevelInfo->WAD, false));
		I_mkdir(Buf, 0);
		snprintf(Buf, PATH_MAX - 1, "graph/%s/%s", WL_GetWADName(g_CurrentLevelInfo->WAD, false), g_CurrentLevelInfo->LumpName);
		I_mkdir(Buf, 0);
		
		snprintf(Buf, PATH_MAX - 1, "graph/%s/%s/node-%04i", WL_GetWADName(g_CurrentLevelInfo->WAD, false), g_CurrentLevelInfo->LumpName, a_Node - nodes);
		f = fopen(Buf, "w+");
		
		if (f)
		{
			for (i = 0; i < a_Poly->NumLines; i++)
			{
				fprintf(f, "%i %i\n",
						SN_RasterToInt(a_Poly->Lines[i].v[0].x),
						SN_RasterToInt(a_Poly->Lines[i].v[1].x)
						);
						
				fprintf(f, "%i %i\n",
						SN_RasterToInt(a_Poly->Lines[(i + 1) % a_Poly->NumLines].v[0].x),
						SN_RasterToInt(a_Poly->Lines[(i + 1) % a_Poly->NumLines].v[1].x)
						);
			}
			
			fclose(f);
		}
#endif
	}
	
	/* Split polygon by the node line */
	// Get split line
	SplitLine.v[0].x = SN_FixedToRaster(a_Node->x);
	SplitLine.v[0].y = SN_FixedToRaster(a_Node->y);
	SplitLine.v[1].x = SN_FixedToRaster(a_Node->x + a_Node->dx);
	SplitLine.v[1].y = SN_FixedToRaster(a_Node->y + a_Node->dy);
	
	// And get the left and right resulting polygons
	SNS_SplitPolyByLine(a_Poly, &SplitLine, &Polies[1], &Polies[0]);
	SNS_FreePoly(a_Poly);
	
	/* Traverse Sides */
	// Start with left, go right
	for (i = 1; i >= 0; i--)
	{
		// Subsector?
		if (a_Node->children[i] & NF_SUBSECTOR)
		{
			// Finalize subsector
			SNS_SubmitSub(&subsectors[(a_Node->children[i] & (~NF_SUBSECTOR))], Polies[i]);
		}
		
		// Node?
		else
		{
			// Triangulate this node further
			SNS_Triangulate(&nodes[a_Node->children[i]], Polies[i]);
		}
		
		// Free polygon
		SNS_FreePoly(Polies[i]);
	}
}

/* SN_ClearLevel() -- Clear the level */
void SN_ClearLevel(void)
{
}

/* SN_InitLevel() -- Initializes the level for rendering */
bool_t SN_InitLevel(void)
{
	SN_Poly_t* BigPoly;
	fixed_t Box[4];
	node_t* LastNode;
	int i;
	
	/* Debug */
	if (M_CheckParm("-devsnow"))
		g_SnowBug = true;
	
	/* First clear the level */
	SN_ClearLevel();
	
	/* Triangulate */
	// Create initial box
	memset(Box, 0, sizeof(Box));
	LastNode = &nodes[numnodes - 1];
	
	for (i = 0; i < 2; i++)
	{
		M_AddToBox(Box, LastNode->bbox[i][BOXLEFT], LastNode->bbox[i][BOXTOP]);
		M_AddToBox(Box, LastNode->bbox[i][BOXRIGHT], LastNode->bbox[i][BOXBOTTOM]);
	}
	
	BigPoly = SNS_BoxToPoly(Box);
	
	// Now do the hard work
	SNS_Triangulate(LastNode, BigPoly);
	
	/* Success? */
	return true;
}

