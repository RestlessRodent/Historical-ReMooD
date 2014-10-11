// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Intermission.

#ifndef __WI_STUFF__
#define __WI_STUFF__

#include "doomtype.h"

/* Define wbstartstruct_t */
#if !defined(__REMOOD_WBSS_DEFINED)
	typedef struct wbstartstruct_s wbstartstruct_t;
	#define __REMOOD_WBSS_DEFINED
#endif

/* Define D_BS_t */
#if !defined(__REMOOD_DBSTDEFINED)
	typedef struct D_BS_s D_BS_t;
	#define __REMOOD_DBSTDEFINED
#endif

////#include "v_video.h"




//added:05-02-98:
typedef struct
{
	int count;
	int num;
	int color;
	char* name;
} fragsort_t;

// Called by main loop, animate the intermission.
void WI_Ticker(void);

// Called by main loop,
// draws the intermission directly into the screen buffer.
void WI_Drawer(void);

// Setup for an intermission screen.
void WI_Start(wbstartstruct_t* wbstartstruct);

bool_t teamingame(int teamnum);

void WI_BuildScoreBoard(wbstartstruct_t* const wbstartstruct, const bool_t a_IsInter);
void WI_DrawScoreBoard(const bool_t a_IsInter, const char* const a_Title, const char* const a_SubTitle);

bool_t WI_SaveGameHelper(D_BS_t* const a_BS);
bool_t WI_LoadGameHelper(D_BS_t* const a_BS);

#endif

