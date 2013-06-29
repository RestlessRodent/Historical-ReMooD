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
// DESCRIPTION: host/client network commands
//              commands are executed through the command buffer
//              like console commands

#ifndef __D_NETCMD_H__
#define __D_NETCMD_H__

#include "doomdef.h"

#include "d_ticcmd.h"
#include "i_util.h"
#include "d_net.h"

// add game commands, needs cleanup
void D_RegisterClientCommands(void);
void D_SendPlayerConfig(void);
void Command_ExitGame_f(void);

/*****************************
*** EXTENDED NETWORK STUFF ***
*****************************/

/*** CONSTANTS ***/

#define MAXDNETTICCMDCOUNT					64	// Max allowed buffered tics

/*** STRUCTURES ***/

#if !defined(__REMOOD_DPROFTDEFINED)
	#define __REMOOD_DPROFTDEFINED
	typedef struct D_Prof_s D_Prof_t;
#endif

/* D_SplitInfo_t -- Split Screen Info */
typedef struct D_SplitInfo_s
{
	bool_t Active;								// Is Active
	bool_t Waiting;								// Waiting for fill
	int32_t Console;							// The console player
	int32_t Display;							// Display Player
	uint32_t ProcessID;							// Local Processing ID
	D_Prof_t* Profile;				// Player Profile
	struct D_SNPort_s* Port;					// Control Port
	bool_t DoNotSteal;							// Do not steal port
	
	bool_t JoyBound;							// Joystick Bound
	uint32_t JoyID;								// Joystick ID
	
	uint8_t RequestSent;						// Sent join request
	tic_t GiveUpAt;								// Give up joining at this time
	bool_t OverlayMap;							// Overlay automap
	bool_t MapKeyStillDown;						// Automap key still down
	
	int8_t ChatMode;							// All? Spec? Team?
	uint32_t ChatTargetID;						// Player to talk to
	tic_t ChatTimeOut;							// Chat timeout
	
	tic_t CoopSpyTime;							// Time to wait to respy
	tic_t TurnHeld;								// Time turning is held
	int32_t Scores;								// Scoreboard showing
	bool_t Turned180;							// Did 180 degre turn
	
	// Automap Stuff
	bool_t AutomapActive;						// Activated Automap
	fixed_t MapZoom;							// Zoom in the map
	bool_t MapFreeMode;							// Free movement mode
	fixed_t MapPos[2];							// Map position
	
	// Profile Select
	bool_t SelProfile;							// Selecting profile
	D_Prof_t* AtProf;							// At this profile
} D_SplitInfo_t;

/*** GLOBALS ***/

extern int g_SplitScreen;						// Players in splits
extern D_SplitInfo_t g_Splits[MAXSPLITSCREEN];	// Split Information

extern bool_t g_NetDev;

/*** FUNCTIONS ***/

bool_t D_ScrSplitHasPlayer(const int8_t a_Player);
bool_t D_ScrSplitVisible(const int8_t a_Player);

void D_NCRemoveSplit(const int32_t a_Split, const bool_t a_Demo);
void D_NCResetSplits(const bool_t a_Demo);
int8_t D_NCSFindSplitByProcess(const uint32_t a_ID);

void D_XNetMergeTics(ticcmd_t* const a_DestCmd, const ticcmd_t* const a_SrcList, const size_t a_NumSrc);

const char* D_NCSGetPlayerName(const uint32_t a_PlayerID);

uint32_t D_CMakePureRandom(void);
void D_CMakeUUID(char* const a_Buf);

#endif

