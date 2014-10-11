// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: game startup, and main loop code, system specific interface stuff.

#ifndef __D_MAIN__
#define __D_MAIN__

#include "doomtype.h"




#include "i_util.h"

/*****************
*** STRUCTURES ***
*****************/

/**************
*** GLOBALS ***
**************/

/************
*** OTHER ***
************/

// the infinite loop of D_DoomLoop() called from win_main for windows version
void D_DoomLoop(void);

//
// D_DoomMain()
// Not a globally visible function, just included for source reference,
// calls all startup code, parses command line options.
// If not overrided by user input, calls N_AdvanceDemo.
//
void D_DoomMain(void);

void D_ProcessEvents(void);
void D_DoAdvanceDemo(void);

//
// BASE LEVEL
//
void D_PageTicker(void);

// pagename is lumpname of a 320x200 patch to fill the screen
void D_WaitingPlayersDrawer(void);
void D_PageDrawer(const char* const a_LumpName);
void D_WFJWDrawer(void);
void D_AdvanceDemo(void);
void D_StartTitle(void);

void D_BuildMapName(char* const a_Dest, const size_t a_Len, const int32_t a_Epi, const int32_t a_Map);
bool_t D_CheckWADBlacklist(const char* const a_Sum);

void D_UITitleBump(void);

#endif							//__D_MAIN__

