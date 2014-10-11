// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: flat sprites & blood splats effects

#ifndef __R_SPLATS_H__
#define __R_SPLATS_H__

#include "r_defs.h"

/* Define V_Image_t */
#if !defined(__REMOOD_VIMAGET_DEFINED)
	typedef struct V_Image_s V_Image_t;
	#define __REMOOD_VIMAGET_DEFINED
#endif



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
	vertex_t v1;				// vertices along the linedef
	vertex_t v2;
	fixed_t top;
	fixed_t offset;				// offset in columns<<FRACBITS from start of linedef to start of splat
	int flags;
	int* yoffset;
	//short       xofs, yofs;
	//int         tictime;
	line_t* line;				// the parent line of the splat seg
	V_Image_t* Image;							// What the splat looks like
	struct wallsplat_s* next;
};
typedef struct wallsplat_s wallsplat_t;

//p_setup.c
extern float P_SegLength(seg_t* seg);

// call at P_SetupLevel()
void R_ClearLevelSplats(void);

void R_AddWallSplat(line_t* wallline, int sectorside, char* patchname, fixed_t top, fixed_t wallfrac, int flags);

#endif /*__R_SPLATS_H__*/

