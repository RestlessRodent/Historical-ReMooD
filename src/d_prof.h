// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION: Profiles

#ifndef __D_PROF_H__
#define __D_PROF_H__

#include "doomtype.h"
#include "doomdef.h"
#include "command.h"
#include "d_netcmd.h"

enum
{
	PC_NAME,
	PC_COLOR,
	PC_SKIN,
	PC_AUTOAIM,
	
	MAXPROFILECVARS
};

typedef struct ProfileInfo_s
{
	char name[MAXPLAYERNAME];
	consvar_t cvars[MAXPROFILECVARS];
	
	struct ProfileInfo_s* prev;
	struct ProfileInfo_s* next;
} ProfileInfo_t;

extern ProfileInfo_t NonLocalProfile;

void PROF_Init(void);
void PROF_Shutdown(void);
void PROF_HandleVAR(char* arg0, char* arg1);

void M_StartProfiler(int choice);
void M_ProfilePrompt(int player);

/************************
*** EXTENDED PROFILES ***
************************/

/*** CONSTANTS ***/

/* D_ProfileExType_t -- Profile Type */
typedef enum D_ProfileExType_e
{
	DPEXT_LOCAL,								// Local player
	DPEXT_NETWORK,								// Network player
	DPEXT_BOT,									// Bot player
	
	NUMDPROFILEEXTYPES
} D_ProfileExType_t;

/* D_ProfileExFlags_t -- Extended profile flags */
typedef enum D_ProfileExFlags_e
{
	DPEXF_GOTMOUSE				= 0x00000001,	// Has control of the mouse
	DPEXF_GOTJOY				= 0x00000002,	// Controls a joystick
	DPEXF_PLAYING				= 0x00000004,	// Is playing the game
} D_ProfileExFlags_t;

/* D_ProfileExBotFlags_t -- Bot Flags */
typedef enum D_ProfileExBotFlags_e
{
	DPEXBOTF_MOUSE				= 0x00000001,	// Bot "uses" a mouse
	DPEXBOTF_TURNAROUND			= 0x00000002,	// Bot can do 180 degree turns
} D_ProfileExBotFlags_t;

/*** STRUCTURES ***/

struct mobj_s;

/* D_ProfileEx_t -- Extended Profile */
typedef struct D_ProfileEx_s
{
	/* Profile Related */
	D_ProfileExType_t Type;						// Type of profile
	uint32_t Flags;								// Flags for profile controller
	char DisplayName[MAXPLAYERNAME];			// Name to show in network games
	char AccountName[MAXPLAYERNAME];			// Local account name (selection limited)
	uint8_t Color;								// Color
	uint8_t JoyControl;							// Which joystick player controls
	char UUID[MAXPLAYERNAME * 2];				// Player Unique ID
	
	/* Network Related */
	D_NetPlayer_t* NetPlayer;					// Network Player
} D_ProfileEx_t;

#endif							/* __D_PROF_H__ */

