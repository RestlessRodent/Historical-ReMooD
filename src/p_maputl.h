// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: map utility functions

#ifndef __P_MAPUTL__
#define __P_MAPUTL__

#include "doomtype.h"
#include "r_defs.h"



//
// P_MAPUTL
//
typedef struct
{
	fixed_t x;
	fixed_t y;
	fixed_t dx;
	fixed_t dy;
	
} divline_t;

typedef struct
{
	fixed_t frac;				// along trace line
	bool_t isaline;
	union
	{
		mobj_t* thing;
		line_t* line;
	} d;
	
	struct
	{
		fixed_t x, y;							// X/Y?
		fixed_t dx, dy;							// Difference?
	} Trace;									// Trace Data
} intercept_t;

/*#define MAXINTERCEPTS   128

extern intercept_t      intercepts[MAXINTERCEPTS];
extern intercept_t*     intercept_p;*/

//SoM: 4/6/2000: Remove limit.
extern int max_intercepts;
extern intercept_t* intercepts;
extern intercept_t* intercept_p;

void P_CheckIntercepts();

typedef bool_t (*traverser_t) (intercept_t* in, void* const a_Data);

bool_t P_PathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2, int flags, traverser_t trav, void* const a_Data);

fixed_t P_AproxDistance(fixed_t dx, fixed_t dy);
int P_PointOnLineSide(fixed_t x, fixed_t y, line_t* line);
int P_PointOnDivlineSide(fixed_t x, fixed_t y, divline_t* line);
void P_MakeDivline(line_t* li, divline_t* dl);
fixed_t P_InterceptVector(divline_t* v2, divline_t* v1);
int P_BoxOnLineSide(fixed_t* tmbox, line_t* ld);

extern fixed_t opentop;
extern fixed_t openbottom;
extern fixed_t openrange;
extern fixed_t lowfloor;

void P_LineOpening(line_t* linedef);

bool_t P_BlockLinesIterator(int x, int y, bool_t (*func) (line_t*, void*), void* a_Arg);
bool_t P_BlockThingsIterator(int x, int y, bool_t (*func) (mobj_t*, void*), void* a_Arg);

#define PT_ADDLINES     1
#define PT_ADDTHINGS    2
#define PT_EARLYOUT     4

extern divline_t trace;

extern fixed_t tmbbox[4];		//p_map.c

// call your user function for each line of the blockmap in the bbox defined by the radius

/*bool_t P_RadiusLinesCheck (  fixed_t    radius,
                              fixed_t    x,
                              fixed_t    y,
                              bool_t   (*func)(line_t*));*/
#endif							// __P_MAPUTL__
