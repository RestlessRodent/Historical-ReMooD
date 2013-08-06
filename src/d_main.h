// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
//         :oCCCCOCoc.
//     .cCO8OOOOOOOOO8Oo:
//   .oOO8OOOOOOOOOOOOOOOCc
//  cO8888:         .:oOOOOC.                                                TM
// :888888:   :CCCc   .oOOOOC.     ###      ###                    #########
// C888888:   .ooo:   .C########   #####  #####  ######    ######  ##########
// O888888:         .oO###    ###  #####  ##### ########  ######## ####    ###
// C888888:   :8O.   .C##########  ### #### ### ##    ##  ##    ## ####    ###
// :8@@@@8:   :888c   o###         ### #### ### ########  ######## ##########
//  :8@@@@C   C@@@@   oo########   ###  ##  ###  ######    ######  #########
//    cO@@@@@@@@@@@@@@@@@Oc0
//      :oO8@@@@@@@@@@Oo.
//         .oCOOOOOCc.                                      http://remood.org/
// ----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2013 GhostlyDeath <ghostlydeath@remood.org>
//                                      <ghostlydeath@gmail.com>
// ----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// ----------------------------------------------------------------------------
// DESCRIPTION: game startup, and main loop code, system specific interface stuff.

#ifndef __D_MAIN__
#define __D_MAIN__

#include "doomtype.h"
//#include "doomstat.h"

//#include "w_wad.h"				// for MAX_WADFILES
//#include "i_util.h"

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

bool_t D_JoyPortsEmpty(void);
uint32_t D_PortToJoy(const uint8_t a_PortID);
uint8_t D_JoyToPort(const uint32_t a_JoyID);
void D_JoySpecialTicker(void);
void D_JoySpecialDrawer(void);
bool_t D_JoySpecialEvent(const I_EventEx_t* const a_Event);


#endif							//__D_MAIN__

