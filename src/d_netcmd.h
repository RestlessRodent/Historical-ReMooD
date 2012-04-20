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
// Copyright (C) 1998-2000 by DooM Legacy Team.
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
// DESCRIPTION: host/client network commands
//              commands are executed through the command buffer
//              like console commands

#ifndef __D_NETCMD__
#define __D_NETCMD__

#include "doomdef.h"
#include "command.h"
#include "d_ticcmd.h"

// console vars
extern consvar_t cv_playername;
extern consvar_t cv_playercolor;
extern consvar_t cv_use_mouse;
extern consvar_t cv_use_joystick;
extern consvar_t cv_autoaim;
extern consvar_t cv_controlperkey;

// splitscreen with seconde mouse
extern consvar_t cv_mouse2port;
extern consvar_t cv_use_mouse2;

#ifdef LMOUSE2
extern consvar_t cv_mouse2opt;
#endif
extern consvar_t cv_invertmouse2;
extern consvar_t cv_alwaysfreelook2;
extern consvar_t cv_mousemove2;
extern consvar_t cv_mousesens2;
extern consvar_t cv_mlooksens2;

// normaly in p_mobj but the .h in not read !
extern consvar_t cv_itemrespawntime;
extern consvar_t cv_itemrespawn;
extern consvar_t cv_respawnmonsters;
extern consvar_t cv_respawnmonsterstime;


// 02-08-98      : r_things.c
extern consvar_t cv_skin;

// secondary splitscreen player
extern consvar_t cv_playername2;
extern consvar_t cv_playercolor2;
extern consvar_t cv_skin2;

extern consvar_t cv_teamplay;
extern consvar_t cv_teamdamage;
extern consvar_t cv_fraglimit;
extern consvar_t cv_timelimit;
extern uint32_t timelimitintics;
extern consvar_t cv_allowturbo;
extern consvar_t cv_allowexitlevel;

extern consvar_t cv_netstat;
extern consvar_t cv_translucency;
extern consvar_t cv_splats;
extern consvar_t cv_maxsplats;
extern consvar_t cv_screenslink;

// add game commands, needs cleanup
void D_RegisterClientCommands(void);
void D_SendPlayerConfig(void);
void Command_ExitGame_f(void);

extern CV_PossibleValue_t fraglimit_cons_t[];
extern CV_PossibleValue_t teamplay_cons_t[];
extern CV_PossibleValue_t deathmatch_cons_t[];

/*****************************
*** EXTENDED NETWORK STUFF ***
*****************************/

/*** CONSTANTS ***/

#define MAXDNETTICCMDCOUNT					64	// Max allowed buffered tics

/* D_NetPlayerType_t -- Profile Type */
typedef enum D_NetPlayerType_e
{
	DNPT_LOCAL,									// Local player
	DNPT_NETWORK,								// Network player
	DNPT_BOT,									// Bot player
	
	NUMDNETPLAYERTYPES
} D_NetPlayerType_t;

/*** STRUCTURES ***/

struct D_ProfileEx_s;
struct player_s;

/* D_NetPlayer_t() -- Network Player */
typedef struct D_NetPlayer_s
{
	/* Generic */
	D_NetPlayerType_t Type;						// Type of network player
	struct D_ProfileEx_s* Profile;				// Linked Profile
	struct player_s* Player;					// Attached Player
	
	/* Player Control */
	// Sync
	int TicTotal;								// Total number of tic commands
	ticcmd_t TicCmd[MAXDNETTICCMDCOUNT];		// Tic Command to execute
	
	// Desync
	
	/* Specifics */
} D_NetPlayer_t;

/*** GLOBALS ***/

extern int g_SplitScreen;						// Players in splits
extern bool_t g_PlayerInSplit[MAXSPLITSCREEN];	// Players that belong in splits

/*** FUNCTIONS ***/

struct player_s* D_NCSAddLocalPlayer(const char* const a_ProfileID);
struct player_s* D_NCSAddBotPlayer(const char* const a_ProfileID);

void D_NCSInit(void);
void D_NCSNetUpdateSingle(struct player_s* a_Player);
void D_NCSNetUpdateAll(void);

D_NetPlayer_t* D_NCSAllocNetPlayer(void);

#endif

