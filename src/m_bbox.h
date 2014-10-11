// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: bounding boxes.

#ifndef __M_BBOX__
#define __M_BBOX__



// Bounding box coordinate storage.
enum
{
	BOXTOP,
	BOXBOTTOM,
	BOXLEFT,
	BOXRIGHT
};								// bbox coordinates

// Bounding box functions.
void M_ClearBox(fixed_t* box);

void M_AddToBox(fixed_t* box, fixed_t x, fixed_t y);

bool_t M_CircleTouchBox(fixed_t* box, fixed_t circlex, fixed_t circley, fixed_t circleradius);

#endif
