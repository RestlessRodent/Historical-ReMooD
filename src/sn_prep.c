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

/*************
*** LOCALS ***
*************/

/* SN_Vert_t -- Vertex */
typedef struct SN_Vert_s
{
	raster_t x;
	raster_t y;
	raster_t z;
} SN_Vert_t;

/* SN_Poly_t -- Polygon, which is split */
typedef struct SN_Poly_s
{
	SN_Vert_t* Verts;
	size_t NumVerts;
} SN_Poly_t;

/****************
*** FUNCTIONS ***
****************/

/* SNS_SubmitSub() -- Submits subsector */
void SNS_SubmitSub(subsector_t* const a_SubS)
{
	/* Debug */
	if (g_SnowBug)
		CONL_PrintF("SubS %u\n", ((unsigned int)(a_SubS - subsectors)));
}

/* SNS_Triangulate() -- Triangulates Nodes */
void SNS_Triangulate(node_t* const a_Node)
{
	register int i;
	
	/* Debug */
	if (g_SnowBug)
		CONL_PrintF("Node %u\n", ((unsigned int)(a_Node - nodes)));
	
	/* Split polygon by the node line */
	// And get the left and right resulting polygons
	
	/* Traverse Sides */
	// Start with left, go right
	for (i = 1; i >= 0; i--)
	{
		// Subsector?
		if (a_Node->children[i] & NF_SUBSECTOR)
		{
			// Finalize subsector
			SNS_SubmitSub(&subsectors[(a_Node->children[i] & (~NF_SUBSECTOR))]);
		}
		
		// Node?
		else
		{
			// Triangulate this node further
			SNS_Triangulate(&nodes[a_Node->children[i]]);
		}
	}
}

/* SN_ClearLevel() -- Clear the level */
void SN_ClearLevel(void)
{
}

/* SN_InitLevel() -- Initializes the level for rendering */
bool_t SN_InitLevel(void)
{
	/* Debug */
	if (M_CheckParm("-devsnow"))
		g_SnowBug = true;
	
	/* First clear the level */
	SN_ClearLevel();
	
	/* Triangulate */
	// Create initial box
	
	// Now do the hard work
	SNS_Triangulate(&nodes[numnodes - 1]);
	
	/* Success? */
	return true;
}

