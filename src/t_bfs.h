// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Binary FraggleScript (LegacyScript)

#ifndef __T_BFS_H__
#define __T_BFS_H__

/*****************************************************************************/

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"

/****************
*** CONSTANTS ***
****************/

/* TBFS_Vis_t() -- Script visibility */
typedef enum TBFS_Vis_e
{
	TBFSV_LEVEL,								// Only to this level
	TBFSV_HUB,									// Only to this hub
	TBFSV_GAME,									// Only to this game session
} TBFS_Vis_t;

/*****************
*** STRUCTURES ***
*****************/

/* Define WL_ES_t */
#if !defined(__REMOOD_WLEST_DEFINED)
	typedef struct WL_ES_s WL_ES_t;
	#define __REMOOD_WLEST_DEFINED
#endif

/* Define WL_WADEntry_t */
#if !defined(__REMOOD_WLWADENT_DEFINED)
	typedef struct WL_WADEntry_s WL_WADEntry_t;
	#define __REMOOD_WLWADENT_DEFINED
#endif

/****************
*** FUNCTIONS ***
****************/

void TBFS_ClearScripts(const TBFS_Vis_t a_Vis);
bool_t TBFS_LoadScript(const TBFS_Vis_t a_Vis, WL_ES_t* const a_WL, const uint32_t a_Start, const uint32_t a_End);
bool_t TBFS_LoadEntry(const TBFS_Vis_t a_Vis, const WL_WADEntry_t* const a_Ent);

/*****************************************************************************/

#endif /* __T_BFS_H__ */

