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
// Copyright (C) 1993-1996 by id Software, Inc.
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
// DESCRIPTION:
//      bounding boxes

#include "doomtype.h"
#include "m_bbox.h"

// faB: getting sick of windows includes errors,
//     I'm supposed to clean that up later.. sure
#ifdef _WIN32
#ifndef MAXINT
#define MAXINT    ((int)0x7fffffff)
#endif
#ifndef MININT
#define MININT    ((int)0x80000000)
#endif
#endif

void M_ClearBox(fixed_t * box)
{
	box[BOXTOP] = box[BOXRIGHT] = MININT;
	box[BOXBOTTOM] = box[BOXLEFT] = MAXINT;
}

void M_AddToBox(fixed_t * box, fixed_t x, fixed_t y)
{
	if (x < box[BOXLEFT])
		box[BOXLEFT] = x;
	if (x > box[BOXRIGHT])
		box[BOXRIGHT] = x;

	if (y < box[BOXBOTTOM])
		box[BOXBOTTOM] = y;
	if (y > box[BOXTOP])
		box[BOXTOP] = y;
}

boolean M_PointInBox(fixed_t * box, fixed_t x, fixed_t y)
{
	if (x < box[BOXLEFT])
		return false;
	if (x > box[BOXRIGHT])
		return false;
	if (y < box[BOXBOTTOM])
		return false;
	if (y > box[BOXTOP])
		return false;

	return true;
}

boolean M_CircleTouchBox(fixed_t * box, fixed_t circlex, fixed_t circley, fixed_t circleradius)
{
	if (box[BOXLEFT] - circleradius > circlex)
		return false;
	if (box[BOXRIGHT] + circleradius < circlex)
		return false;
	if (box[BOXBOTTOM] - circleradius > circley)
		return false;
	if (box[BOXTOP] + circleradius < circley)
		return false;
	return true;
}
