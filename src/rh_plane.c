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

/****************
*** CONSTANTS ***
****************/

/*****************
*** PROTOTYPES ***
*****************/

/****************
*** FUNCTIONS ***
****************/

/* RH_MapPlane() -- Maps a plane */
void RH_MapPlane(RH_RenderStat_t* const a_Stat, RH_VisPlane_t* const a_VisPlane, const int16_t a_Y, const int16_t a_X1, const int16_t a_X2)
{
	fixed_t planeheight;
	angle_t angle;
	fixed_t distance, length;
	unsigned index;
	int ds_y;
	int ds_x1;
	int ds_x2;
	lighttable_t *ds_colormap;
	fixed_t ds_xfrac;
	fixed_t ds_yfrac;
	fixed_t ds_xstep;
	fixed_t ds_ystep;
	uint8_t *ds_source;
	
	/* Exceeds Bounds? */
	if (a_X2 < a_X1 || a_X1 < 0 || a_X2 >= vid.width || a_Y > a_Stat->viewheight)
		return;
	
	/* Get plane height */
	planeheight = a_VisPlane->Height - a_Stat->viewz;
	
	/* Draw */
	// Not Cached?
	if (planeheight != a_Stat->cachedheight[a_Y])
	{
		a_Stat->cachedheight[a_Y] = planeheight;
		distance = a_Stat->cacheddistance[a_Y] = FixedMul(planeheight, a_Stat->yslope[a_Y]);

		ds_xstep = a_Stat->cachedxstep[a_Y] = FixedMul(distance, a_Stat->basexscale);
		ds_ystep = a_Stat->cachedystep[a_Y] = FixedMul(distance, a_Stat->baseyscale);
	}
	
	// Cached
	else
	{
		distance = a_Stat->cacheddistance[a_Y];
		ds_xstep = a_Stat->cachedxstep[a_Y];
		ds_ystep = a_Stat->cachedystep[a_Y];
	}
	
	/* Prepare to draw */
	length = FixedMul(distance, a_Stat->distscale[a_X1]);
	angle = (a_Stat->viewangle + a_Stat->xtoviewangle[a_X1]) >> ANGLETOFINESHIFT;
	ds_xfrac = a_Stat->viewx + FixedMul(finecosine[angle], length);
	ds_yfrac = -a_Stat->viewy - FixedMul(finesine[angle], length);

#if 0
	if (fixedcolormap)
		ds_colormap = fixedcolormap;
	else
	{
		index = distance >> LIGHTZSHIFT;
		if (index >= MAXLIGHTZ )
			index = MAXLIGHTZ-1;
		ds_colormap = planezlight[index];
	}
#endif

	/* Draw It */
	// Locations
	ds_y = a_Y;
	ds_x1 = a_X1;
	ds_x2 = a_X2;
	
	// Call handler
	RH_DrawSpan(a_Stat, a_VisPlane, ds_y, ds_x1, ds_x2, ds_colormap, ds_xfrac, ds_yfrac, ds_xstep, ds_ystep, ds_source);
}

/* RH_MakeSpans() -- Make visplane spans to be drawn */
void RH_MakeSpans(RH_RenderStat_t* const a_Stat, RH_VisPlane_t* const a_VisPlane, const int16_t a_X, const int16_t a_T1, const int16_t a_B1, const int16_t a_T2, const int16_t a_B2)
{
	int16_t x, t1, t2, b1, b2;
	
	/* Copy */
	x = a_X;
	t1 = a_T1;
	t2 = a_T2;
	b1 = a_B1;
	b2 = a_B2;
	
	/* Map Plane */
	// Top
	for (; t1 < t2 && t1 <= b1; t1++)
		RH_MapPlane(a_Stat, a_VisPlane, t1, a_Stat->spanstart[t1], x - 1);
		
	// Bottom
	for (; b1 > b2 && t1 >= t1; b1--)
		RH_MapPlane(a_Stat, a_VisPlane, b1, a_Stat->spanstart[b1], x - 1);
	
	/* Change Span positions */
	// Start
	for (; t2 < t1 && t2 <= b2; t2++)
		a_Stat->spanstart[t2] = x;
	
	// End
	for (; b2 > b1 && b2 >= t2; b2--)
		a_Stat->spanstart[b2] = x;
}

/* RH_RenderPlanes() -- Renders visplanes */
void RH_RenderPlanes(RH_RenderStat_t* const a_Stat)
{
	size_t h, i;
	int16_t Stop, x;
	RH_VisPlane_t* Rover;
	
	/* Draw all visplanes */
	for (Rover = a_Stat->FirstPlaneLink; Rover; Rover = Rover->Next)
	{
		// Bad plane?
		if (Rover->MinX > Rover->MaxX)
			continue;
		
		// Is sky texture?
		if (Rover->PicNum == skyflatnum)
		{
		}
		
		// Normal texture
		else
		{
			// Limit
			Rover->Top[Rover->MaxX + 1] = -1;
			Rover->Top[Rover->MinX - 1] = -1;
			
			// Make spans
			Stop = Rover->MaxX + 1;
			for (x = Rover->MinX; x <= Stop; x++)
				RH_MakeSpans(a_Stat, Rover,
						x,
						Rover->Top[x - 1],
						Rover->Bottom[x - 1],
						Rover->Top[x],
						Rover->Bottom[x]
					);
		}
	}
}

/* RHS_HashPlane() -- Hashes a visplane */
static uint32_t RHS_HashPlane(RH_RenderStat_t* const a_Stat, RH_VisPlane_t* const a_Input)
{
	uint32_t RetVal;
	
	/* Check */
	if (!a_Input)
		return 0;
	
	/* Clear return value */
	RetVal = 0;
	
	/* Hash it up */
	RetVal += (a_Input->Height >> FRACBITS) * 9;
	RetVal += (a_Input->PicNum) * 7;
	RetVal += (a_Input->LightLevel) * 3;
	RetVal += (a_Input->Special);
	
	/* Return it */
	return RetVal & (MAXRHVISPLANES - 1);
}

/* RH_FindPlane() -- Finds another visplane or a visplane similar to this one */
RH_VisPlane_t* RH_FindPlane(RH_RenderStat_t* const a_Stat, RH_VisPlane_t* const a_Input)
{
	int32_t i;
	uint32_t Hash;
	RH_VisPlane_t* New;
	
	/* Obtain hash of visplane */
	Hash = RHS_HashPlane(a_Stat, a_Input);
	
	/* See if any planes exist already */
	for (i = 0; i < a_Stat->NumVisPlanes[Hash]; i++)
		if (RH_ComparePlane(a_Stat, a_Input, a_Stat->VisPlanes[Hash][i]))
			return a_Stat->VisPlanes[Hash][i];
	
	/* Otherwise, allocate a new visplane */
	Z_ResizeArray((void**)&a_Stat->VisPlanes[Hash], sizeof(*a_Stat->VisPlanes[Hash]),
		a_Stat->NumVisPlanes[Hash], a_Stat->NumVisPlanes[Hash] + 1);
	a_Stat->VisPlanes[Hash][a_Stat->NumVisPlanes[Hash]++] = New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Fill in info */
	New->Height = a_Input->Height;
	New->PicNum = a_Input->PicNum;
	New->LightLevel = a_Input->LightLevel;
	New->Special = a_Input->Special;
	New->MinX = vid.width;
	New->MaxX = -1;
	
	/* Make buffers */
	// Top
	New->Top = Z_Malloc(sizeof(*New->Top) * (vid.width + 2), PU_STATIC, NULL);
	New->Top = &New->Top[1];
	memset(New->Top, 0xff, sizeof(*New->Top) * (vid.width + 2));
	
	// Bottom
	New->Bottom = Z_Malloc(sizeof(*New->Bottom) * (vid.width + 2), PU_STATIC, NULL);
	New->Bottom = &New->Bottom[1];
	
	/* Link into chain */
	if (!a_Stat->FirstPlaneLink)
		a_Stat->FirstPlaneLink = New;
	else
	{
		a_Stat->FirstPlaneLink->Prev = New;
		New->Next = a_Stat->FirstPlaneLink;
		a_Stat->FirstPlaneLink = New;
	}
	
	/* Return it */
	return New;
}

/* RH_ComparePlane() -- Checks whether the planes are the same */
bool_t RH_ComparePlane(RH_RenderStat_t* const a_Stat, RH_VisPlane_t* const a_A, RH_VisPlane_t* const a_B)
{
	/* Missing? */
	if (!a_A || !a_B)
		return false;
	
	/* Same var? */
	if (a_A == a_B)
		return true;
	
	/* Compare stats */
	if (a_A->Height != a_B->Height)
		return false;
	if (a_A->PicNum != a_B->PicNum)
		return false;
	if (a_A->LightLevel != a_B->LightLevel)
		return false;
	if (a_A->Special != a_B->Special)
		return false;
	
	/* They match */
	return true;
}

