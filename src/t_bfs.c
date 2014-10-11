// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Binary FraggleScript (LegacyScript)

/***************
*** INCLUDES ***
***************/

#include "t_bfs.h"
#include "t_bprim.h"

/****************
*** CONSTANTS ***
****************/

/*****************
*** STRUCTURES ***
*****************/

/*************
*** LOCALS ***
*************/

/****************
*** FUNCTIONS ***
****************/

/* TBFS_ClearScripts() -- Clears loaded scripts */
void TBFS_ClearScripts(const TBFS_Vis_t a_Vis)
{
}

/* TBFS_LoadScript() -- Loads script at specified visibility */
bool_t TBFS_LoadScript(const TBFS_Vis_t a_Vis, WL_ES_t* const a_WL, const uint32_t a_Start, const uint32_t a_End)
{
	/* Check */
	if (!a_WL || a_Start <= a_End)
		return false;
}

/* TBFS_LoadEntry() -- Loads entire entry */
bool_t TBFS_LoadEntry(const TBFS_Vis_t a_Vis, const WL_WADEntry_t* const a_Ent)
{
}

