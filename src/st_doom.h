// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Doom Status Bar

#ifndef __ST_DOOM_H__
#define __ST_DOOM_H__

/*****************************************************************************/

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "st_stuff.h"

/****************
*** FUNCTIONS ***
****************/

void ST_DoomBar(const size_t a_PID, const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H, player_t* const a_ConsoleP, player_t* const a_DisplayP, D_Prof_t* a_Profile);
 void ST_DoomModShape(const size_t a_PID, int32_t* const a_X, int32_t* const a_Y, int32_t* const a_W, int32_t* const a_H, player_t* const a_ConsoleP, player_t* const a_DisplayP, D_Prof_t* a_Profile);

/*****************************************************************************/

#endif /* __ST_DOOM_H__ */

