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
	BGCM_DONTCARE,								// Don't care
	BGCM_MAXKILLS,								// 100% Kills
	BGCM_UVMAX,									// 100% Kills and Secrets
	BGCM_UVALLMAX,								// 100% Kills, Items, Secrets
	BGCM_MAXSECRETS,							// 100% Secrets
	BGCM_MAXITEMS,								// 100% Items
	BGCM_EXITRUN,								// Exit Running
	BGCM_MAXKILLSITEMS,							// 100% Kills, Items
	
	NUMBGHOSTCOOPMODES
} B_GhostCoopMode_t;

/* B_GhostAtkPosture_t -- Attack posture for bot */
typedef enum B_GhostAtkPosture_e
{
	BGAP_DONTCARE,								// Don't Care
	BGAP_DEFENSE,								// Lean twords Defense
	BGAP_MIDDLE,								// Neither offensive or defensive
	BGAP_OFFENSE,								// Lean twords Offence	
		
	NUMBGHOSTATKPOSTURE
} B_GhostAtkPosture_t;

/*****************
*** STRUCTURES ***
*****************/

typedef struct B_BotData_s B_BotData_t;
struct B_BotTemplate_s;

/* B_GhostBot_t -- GhostlyBots information */
typedef struct B_GhostBot_s
{
	uint8_t Junk;								// Junk Data
	ticcmd_t* TicCmdPtr;						// Pointer to tic command
	bool_t Initted;								// Initialized
	void* AtNode;								// At node
	void* OldNode;								// Old node
	B_BotData_t* BotData;						// Data
	player_t* Player;							// Player
	mobj_t* Mo;									// Mo
	struct B_BotTemplate_s* BotTemplate;		// Template
	
	int32_t RoamX, RoamY;						// Roaming X/Y
	
	struct
	{
		bool_t IsSet;							// Target Set
		bool_t MoveTarget;						// Movement target
		uint32_t ExpireTic;						// Action expires at this time
		int32_t Priority;						// Priority
		fixed_t x, y;							// X/Y Target
		uintptr_t Key;							// Key
	} Targets[MAXBOTTARGETS];					// Bot target
	
	struct
	{
		bool_t JobHere;							// A Job is here
		bool_t (*JobFunc)(struct B_GhostBot_s* a_GhostBot, const size_t a_JobID);
		int32_t Priority;						// Job Priority
		uint32_t Sleep;							// Job Sleeping (wait until tic happens)
	} Jobs[MAXBOTJOBS];							// Bot's Jobs
	
	struct
	{
		B_GhostAtkPosture_t Posture;			// Bot Posture
		B_GhostCoopMode_t CoopMode;				// Coop Mode
	} AISpec;									// AI Specification
} B_GhostBot_t;

/* B_BotTemplate_t -- Bot Template */
typedef struct B_BotTemplate_s
{
	uint32_t BotIDNum;							// Bot ID Number
	const char* AccountName;					// Account Name
	const char* DisplayName;					// Display Name
	uint8_t SkinColor;							// Skin Color
	uint8_t RGBSkinColor[3];					// Skin Color in RGB
	const char* WeaponOrder;					// Weapon Order
	B_GhostAtkPosture_t Posture;				// Posture
	B_GhostCoopMode_t CoopMode;					// Coop Mode
	const char* HexenClass;						// Favorite Weapon Class
} B_BotTemplate_t;

/**************
*** GLOBALS ***
**************/

extern bool_t g_BotDebug;						// Debugging Bots

/*** B_GHOST.C ***/
extern fixed_t g_GlobalBoundBox[4];				// Global bounding box
extern bool_t g_GotBots;						// Got a bot?

/****************
*** FUNCTIONS ***
****************/

/*** B_BOT.C ***/
B_BotData_t* B_InitBot(D_NetPlayer_t* const a_NPp, const B_BotTemplate_t* a_Template);
const B_BotTemplate_t* B_BotGetTemplate(const int32_t a_Player);
void B_InitNodes(void);
void B_ClearNodes(void);

void B_BuildBotTicCmd(B_BotData_t* const a_BotData, ticcmd_t* const a_TicCmd);

void B_RemoveMobj(void* const a_Mo);

/*** B_GHOST.C ***/
void B_GHOST_Ticker(void);
void B_GHOST_ClearLevel(void);
void B_GHOST_InitLevel(void);

void B_GHOST_Think(B_GhostBot_t* const a_GhostBot, ticcmd_t* const a_TicCmd);

const B_BotTemplate_t* B_GHOST_FindTemplate(const char* const a_Name);
const B_BotTemplate_t* B_GHOST_RandomTemplate(void);
const B_BotTemplate_t* B_GHOST_TemplateByID(const uint32_t a_ID);

void B_XDestroyBot(B_BotData_t* const a_BotData);

#endif /* __B_BOT_H__ */


