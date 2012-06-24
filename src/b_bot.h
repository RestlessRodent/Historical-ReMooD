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
// Copyright (C) 2012 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Bot Code

#ifndef __B_BOT_H__
#define __B_BOT_H__

/***************
*** INCLUDES ***
***************/

#include "d_netcmd.h"
#include "d_prof.h"
#include "d_ticcmd.h"

#include "r_defs.h"
#include "r_state.h"
#include "p_mobj.h"
#include "d_player.h"

/****************
*** CONSTANTS ***
****************/

#define MAXBOTJOBS							32	// Maximum Jobs

#define MAXBOTTARGETS						16	// Max target designated

/* B_GhostCoopMode_t -- Coop Mode */
typedef enum B_GhostCoopMode_e
{
	BGCM_MAXKILLS,								// 100% Kills
	BGCM_UVMAX,									// 100% Kills and Secrets
	BGCM_UVALLMAX,								// 100% Kills, Items, Secrets
	BGCM_MAXSECRETS,							// 100% Secrets
	BGCM_MAXITEMS,								// 100% Items
	BGCM_EXITRUN,								// Exit Running
	
	NUMBGHOSTCOOPMODES
} B_GhostCoopMode_t;

/* B_GhostAtkPosture_t -- Attack posture for bot */
typedef enum B_GhostAtkPosture_e
{
	BGAP_DEFENSE,								// Lean twords Defense
	BGAP_MIDDLE,								// Neither offensive or defensive
	BGAP_OFFENSE,								// Lean twords Offence	
		
	NUMBGHOSTATKPOSTURE
} B_GhostAtkPosture_t;

/*****************
*** STRUCTURES ***
*****************/

typedef struct B_BotData_s B_BotData_t;

/* B_GhostBot_t -- GhostlyBots information */
typedef struct B_GhostBot_s
{
	uint8_t Junk;								// Junk Data
	ticcmd_t* TicCmdPtr;						// Pointer to tic command
	bool Initted;								// Initialized
	void* AtNode;								// At node
	void* OldNode;								// Old node
	B_BotData_t* BotData;						// Data
	player_t* Player;							// Player
	mobj_t* Mo;									// Mo
	
	int32_t RoamX, RoamY;						// Roaming X/Y
	
	struct
	{
		bool IsSet;							// Target Set
		bool MoveTarget;						// Movement target
		uint32_t ExpireTic;						// Action expires at this time
		int32_t Priority;						// Priority
		fixed_t x, y;							// X/Y Target
		uintptr_t Key;							// Key
	} Targets[MAXBOTTARGETS];					// Bot target
	
	struct
	{
		bool JobHere;							// A Job is here
		bool (*JobFunc)(struct B_GhostBot_s* a_GhostBot, const size_t a_JobID);
		int32_t Priority;						// Job Priority
		uint32_t Sleep;							// Job Sleeping (wait until tic happens)
	} Jobs[MAXBOTJOBS];							// Bot's Jobs
	
	struct
	{
		B_GhostAtkPosture_t Posture;			// Bot Posture
		B_GhostCoopMode_t CoopMode;				// Coop Mode
	} AISpec;									// AI Specification
} B_GhostBot_t;

/**************
*** GLOBALS ***
**************/

extern bool g_BotDebug;						// Debugging Bots

/*** B_GHOST.C ***/
extern fixed_t g_GlobalBoundBox[4];				// Global bounding box

/****************
*** FUNCTIONS ***
****************/

/*** B_BOT.C ***/
B_BotData_t* B_InitBot(D_NetPlayer_t* const a_NPp);
void B_InitNodes(void);
void B_ClearNodes(void);

void B_BuildBotTicCmd(B_BotData_t* const a_BotData, ticcmd_t* const a_TicCmd);

void B_RemoveMobj(void* const a_Mo);

/*** B_GHOST.C ***/
void B_GHOST_Ticker(void);
void B_GHOST_ClearLevel(void);
void B_GHOST_InitLevel(void);

void B_GHOST_Think(B_GhostBot_t* const a_GhostBot, ticcmd_t* const a_TicCmd);

#endif /* __B_BOT_H__ */


