// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Refresh module, BSP traversal and handling.

#ifndef __R_BSP__
#define __R_BSP__

#include "r_defs.h"

extern seg_t* curline;
extern side_t* sidedef;
extern line_t* linedef;
extern sector_t* frontsector;
extern sector_t* backsector;

extern bool_t skymap;

// faB: drawsegs are now allocated on the fly ... see r_segs.c
// extern drawseg_t*       drawsegs;
//SoM: 3/26/2000: Use boom code.
extern drawseg_t* drawsegs;
extern size_t maxdrawsegs;
extern drawseg_t* ds_p;
extern drawseg_t* firstnewseg;

extern lighttable_t** hscalelight;
extern lighttable_t** vscalelight;
extern lighttable_t** dscalelight;

typedef void (*drawfunc_t) (int start, int stop);

// BSP?
void R_ClearClipSegs(void);
void R_SetupClipSegs();
void R_ClearDrawSegs(void);

void R_RenderBSPNode(int bspnum);

sector_t* R_FakeFlat(sector_t* sec, sector_t* tempsec, int* floorlightlevel, int* ceilinglightlevel, bool_t back);

int R_GetPlaneLight(sector_t* sector, fixed_t planeheight, bool_t underside);
void R_Prep3DFloors(sector_t* sector);
#endif
