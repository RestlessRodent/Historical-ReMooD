// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Game Sockets

/***************
*** INCLUDES ***
***************/

#include "cl.h"
#include "i_util.h"
#include "z_zone.h"

/*****************
*** STRUCTURES ***
*****************/

/**************
*** GLOBALS ***
**************/

CL_View_t g_CLViews[MAXSPLITS];					// Viewports
int32_t g_CLBinds;								// Number of bound viewports

/****************
*** FUNCTIONS ***
****************/

/* CL_InitViews() -- Initializes viewports */
void CL_InitViews(void)
{
	memset(g_CLViews, 0, sizeof(g_CLViews));
	g_CLBinds = 0;
}

