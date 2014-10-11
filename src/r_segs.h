// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Refresh module, drawing LineSegs from BSP.

#ifndef __R_SEGS__
#define __R_SEGS__

#include "doomtype.h"
#include "r_bsp.h"

/* Define lighttable_t */
#if !defined(__REMOOD_LITETABLE_DEFINED)
	typedef uint8_t lighttable_t;
	#define __REMOOD_LITETABLE_DEFINED
#endif

extern lighttable_t** walllights;

void R_RenderMaskedSegRange(drawseg_t* ds, int x1, int x2);

void R_RenderThickSideRange(drawseg_t* ds, int x1, int x2, ffloor_t* ffloor);

void R_StoreWallRange(int start, int stop);
#endif

