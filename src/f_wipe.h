// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Mission start screen wipe/melt, special effects.

#ifndef __F_WIPE_H__
#define __F_WIPE_H__

#include "doomtype.h"

//--------------------------------------------------------------------------
//                        SCREEN WIPE PACKAGE
//--------------------------------------------------------------------------

enum
{
	// simple gradual pixel change for 8-bit only
	wipe_ColorXForm,
	
	// weird screen melt
	wipe_Melt,
	
	// GhostlyDeath <June 4, 2010> -- Blinds
	wipe_Blinds,
	
	wipe_NUMWIPES
};

int wipe_StartScreen(int x, int y, int width, int height);

int wipe_EndScreen(int x, int y, int width, int height);

int wipe_ScreenWipe(int wipeno, int x, int y, int width, int height, int ticks);

#endif
