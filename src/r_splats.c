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
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION:
//      floor and wall splats

#include "r_draw.h"
#include "r_main.h"
#include "r_plane.h"
#include "r_splats.h"
#include "w_wad.h"
#include "z_zone.h"
#include "d_netcmd.h"
#include "p_demcmp.h"

#ifdef WALLSPLATS
static wallsplat_t wallsplats[MAXLEVELSPLATS];	// WALL splats
static int freewallsplat;
#endif

// g_CVPVMaxSplats -- Max splats to allow
const CONL_VarPossibleValue_t c_CVPVMaxSplats[] =
{
	// End
	{1, "MINVAL"},
	{MAXLEVELSPLATS, "MAXVAL"},
	{0, NULL},
};

// r_maxsplats -- Max splats to allow
CONL_StaticVar_t l_RMaxSplats =
{
	CLVT_INTEGER, c_CVPVMaxSplats, CLVF_SAVE,
	"r_maxsplats", DSTR_CVHINT_RMAXSPLATS, CLVVT_STRING, "true",
	NULL
};

// r_drawsplats -- Enables Drawing of splats
CONL_StaticVar_t l_RDrawSplats =
{
	CLVT_INTEGER, c_CVPVBoolean, CLVF_SAVE,
	"r_drawsplats", DSTR_CVHINT_RDRAWSPLATS, CLVVT_STRING, "true",
	NULL
};

// for floorsplats, accessed by asm code
struct rastery_s* prastertab;

// --------------------------------------------------------------------------
// setup splat cache
// --------------------------------------------------------------------------
void R_ClearLevelSplats(void)
{
#ifdef WALLSPLATS
	freewallsplat = 0;
	memset(wallsplats, 0, sizeof(wallsplats));
#endif
}

// ==========================================================================
//                                                                WALL SPLATS
// ==========================================================================
#ifdef WALLSPLATS
// --------------------------------------------------------------------------
// Return a pointer to a splat free for use, or NULL if no more splats are
// available
// --------------------------------------------------------------------------
static wallsplat_t* R_AllocWallSplat(void)
{
	wallsplat_t* splat;
	wallsplat_t* p_splat;
	line_t* li;
	
	// clear the splat from the line if it was in use
	splat = &wallsplats[freewallsplat];
	li = splat->line;
	if (li)
	{
		// remove splat from line splats list
		if (li->splats == splat)
		{
			li->splats = splat->next;	//remove from head
			splat->line = NULL;	// Remove reference here
		}
		else
		{
			// GhostlyDeath <November 3, 2010> -- PARANOIA removal
			if (!li->splats)
			{
				if (devparm)
					CONL_PrintF("WARNING - R_AllocWallSplat: Line has no splats (%s:%i).\n", __FILE__, __LINE__);
				return NULL;
			}
			
			// GhostlyDeath <November 3, 2010> -- Remove NULL dereference
			for (p_splat = li->splats; p_splat && p_splat->next; p_splat = p_splat->next)
				if (p_splat->next == splat)
				{
					p_splat->next = splat->next;
					break;
				}
		}
	}
	
	memset(splat, 0, sizeof(wallsplat_t));
	
	// for next allocation
	freewallsplat++;
	if ((freewallsplat >= l_RMaxSplats.Value->Int) || (freewallsplat >= MAXLEVELSPLATS))
		freewallsplat = 0;
		
	return splat;
}

/* P_SegLength() -- Length of seg */
float P_SegLength(seg_t* seg)
{
#define crapmul (1.0f / 65536.0f)
	double dx, dy;
	
	// make a vector (start at origin)
	dx = (seg->v2->x - seg->v1->x) * crapmul;
	dy = (seg->v2->y - seg->v1->y) * crapmul;
	
	return sqrt(dx * dx + dy * dy) * FRACUNIT;
#undef crapmul
}

/* R_AddWallSplat() -- Adds a splat to a wall */
// GhostlyDeath <Octover 23, 2010> -- Rewritten. If a splat is near another splat replace that splat.
void R_AddWallSplat(line_t* wallline, int sectorside, char* patchname, fixed_t top, fixed_t wallfrac, int flags)
{
	fixed_t LineLength;
	fixed_t FracSplat;
	fixed_t Offset;
	wallsplat_t* Rover;
	wallsplat_t* Next;
	sector_t* BackSector = NULL;
	int* yOffset = NULL;
	wallsplat_t Temp;
	V_Image_t* SplatImage;
	
	/* Check */
	// Proper arguments
	if (!wallline || !patchname)
		return;
		
	// Splats are actually enabled
	if (!l_RDrawSplats.Value->Int)
		return;
		
	// Demo version
	if (!P_XGSVal(PGS_COENABLESPLATS))
		return;
		
	/* Pre-init some variables */
	// Find picture
	SplatImage = V_ImageFindA(patchname, VCP_NONE);
	
	if (!SplatImage)
		return;
	
	// Get side of sector
	sectorside ^= 1;
	if (wallline->sidenum[sectorside] != -1)
	{
		BackSector = sides[wallline->sidenum[sectorside]].sector;
		
		if (top < BackSector->floorheight)
		{
			yOffset = &BackSector->floorheight;
			top -= BackSector->floorheight;
		}
		else if (top > BackSector->ceilingheight)
		{
			yOffset = &BackSector->ceilingheight;
			top -= BackSector->ceilingheight;
		}
	}
	// Get offset of splat along the line
	LineLength = P_SegLength((seg_t*) wallline);
	Offset = FixedMul(wallfrac, LineLength) - (SplatImage->Width << (FRACBITS - 1));
	FracSplat = FixedDiv(((SplatImage->Width << FRACBITS) >> 1), LineLength);
	wallfrac -= FracSplat;
	
	// Splat off wall?
	if (wallfrac > LineLength)
		return;
		
	/* Replace an existing splat */
	// Start on line
	Rover = wallline->splats;
	
	// Rove line
	while (Rover)
	{
		// Next
		Rover = Rover->next;
	}
	
	// Ran out of Rover
	if (!Rover)
		Rover = R_AllocWallSplat();
		
	// Double-check
	if (!Rover)
		return;
		
	/* Set splat properties */
	// GhostlyDeath <September 10, 2011> -- Clear temporary splat (this should fix SIGSEGVs)
	memset(&Temp, 0, sizeof(Temp));
	
	// Basic
	Temp.Image = SplatImage;
	Temp.top = top;
	Temp.flags = flags;
	Temp.offset = Offset;
	Temp.yoffset = yOffset;
	Temp.line = wallline;
	
	// Get position
	Temp.v1.x = wallline->v1->x + FixedMul(wallline->dx, wallfrac);
	Temp.v1.y = wallline->v1->y + FixedMul(wallline->dy, wallfrac);
	wallfrac += FracSplat + FracSplat;
	
	// Off left of wall?
	if (wallfrac < 0)
		return;
		
	Temp.v2.x = wallline->v1->x + FixedMul(wallline->dx, wallfrac);
	Temp.v2.y = wallline->v1->y + FixedMul(wallline->dy, wallfrac);
	
	if (wallline->frontsector && wallline->frontsector == BackSector)
		return;
		
	/* Insert splat into wall */
	// Wall has no splats
	if (!wallline->splats)
	{
		// Just set into wall
		wallline->splats = R_AllocWallSplat();
		Rover = wallline->splats;
		
		// Clone Data
		*Rover = Temp;
		Rover->next = NULL;
	}
	// Wall has splats
	else
	{
		// Get head
		Rover = wallline->splats;
		
		// Check each entry
		while (Rover)
		{
			// Same patch id?
			if (Rover->Image && Rover->Image->wData == SplatImage->wData)
				// Near offset?
				if ((abs((Rover->offset >> FRACBITS) - (Offset >> FRACBITS)) < 8) && (abs((Rover->top >> FRACBITS) - (top >> FRACBITS)) < 8))
				{
					// Copy
					Next = Rover->next;
					*Rover = Temp;
					Rover->next = Next;
					
					// Break out
					break;
				}
			
			// Next?
			if (Rover->next)
				Rover = Rover->next;
				
			// Create next and break out (new)
			else
			{
				Rover->next = R_AllocWallSplat();
				
				// Clone Data -- R_AWS() should return something
				if (Rover->next)	// Probably always false
				{
					*(Rover->next) = Temp;
					if (Rover->next)	// Could have been NULLed!
						Rover->next->next = NULL;
				}
				break;
			}
		}
	}
}
#endif							// WALLSPLATS

