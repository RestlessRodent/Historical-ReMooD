// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.org/
// -----------------------------------------------------------------------------
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2011 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION: flat sprites & blood splats effects

#ifndef __R_SPLATS_H__
#define __R_SPLATS_H__

#include "r_defs.h"
#include "w_wad.h"

#define WALLSPLATS				// comment this out to compile without splat effects

#define MAXLEVELSPLATS      1024

// splat flags
#define SPLATDRAWMODE_MASK   0x03	// mask to get drawmode from flags
#define SPLATDRAWMODE_OPAQUE 0x00
#define SPLATDRAWMODE_SHADE  0x01
#define SPLATDRAWMODE_TRANS  0x02
/*
#define SPLATUPPER           0x04
#define SPLATLOWER           0x08
*/
// ==========================================================================
// DEFINITIONS
// ==========================================================================

// WALL SPLATS are patches drawn on top of wall segs
struct wallsplat_s
{
	WadIndex_t patch;					// lump id.
	vertex_t v1;				// vertices along the linedef
	vertex_t v2;
	fixed_t top;
	fixed_t offset;				// offset in columns<<FRACBITS from start of linedef to start of splat
	int flags;
	int *yoffset;
	//short       xofs, yofs;
	//int         tictime;
	line_t *line;				// the parent line of the splat seg
	struct wallsplat_s *next;
};
typedef struct wallsplat_s wallsplat_t;

//p_setup.c
extern float P_SegLength(seg_t * seg);

// call at P_SetupLevel()
void R_ClearLevelSplats(void);

void R_AddWallSplat(line_t * wallline, int sectorside, char *patchname,
					fixed_t top, fixed_t wallfrac, int flags);

#endif /*__R_SPLATS_H__*/

