// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Lookup tables.

#ifndef __TABLES__
#define __TABLES__

#include "doomtype.h"

#define FINEANGLES              8192
#define FINEMASK                (FINEANGLES-1)
#define ANGLETOFINESHIFT        19	// 0x100000000 to 0x2000

// Effective size is 10240.
extern fixed_t finesine[5 * FINEANGLES / 4];

// Re-use data, is just PI/2 phase shift.
extern fixed_t* finecosine;

// Effective size is 4096.
extern fixed_t finetangent[FINEANGLES / 2];

// to get a global angle from cartesian coordinates, the coordinates are
// flipped until they are in the first octant of the coordinate system, then
// the y (<=x) is scaled and divided by x to get a tangent (slope) value
// which is looked up in the tantoangle[] table.
#define SLOPERANGE  2048
#define SLOPEBITS   11
#define DBITS       (FRACBITS-SLOPEBITS)

// The +1 size is to handle the case when x==y without additional checking.
extern angle_t tantoangle[SLOPERANGE + 1];

// Utility function, called by R_PointToAngle.
angle_t SlopeDiv(angle_t num, angle_t den);

fixed_t TBL_BAMToDeg(const angle_t a_Angle);

extern const int16_t c_AngLUT[8192];

#endif

