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
// Copyright (C) 2012-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Bot Code

#ifndef __B_BOT_H__
#define __B_BOT_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "doomdef.h"
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

/*****************
*** STRUCTURES ***
*****************/

typedef struct B_GhostBot_s B_GhostBot_t;

/* B_BotTemplate_t -- Bot Template */
typedef struct B_BotTemplate_s
{
	uint32_t BotIDNum;							// Bot ID Number
	char AccountName[MAXPLAYERNAME];			// Account Name
	char DisplayName[MAXPLAYERNAME];			// Display Name
	uint8_t SkinColor;							// Skin Color
	uint8_t RGBSkinColor[3];					// Skin Color in RGB
	const char* WeaponOrder;					// Weapon Order
	char HexenClass[MAXPLAYERNAME];				// Favorite Hexen Class
	
	uint32_t Count;								// Usage Count
	uint32_t Flags;								// Flags for bot
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
B_GhostBot_t* B_InitBot(const B_BotTemplate_t* a_Template);
B_BotTemplate_t* B_BotGetTemplate(const int32_t a_Player);

B_BotTemplate_t* B_BotGetTemplateDataPtr(B_GhostBot_t* const a_BotData);
struct D_XPlayer_s* B_BotGetXPlayer(B_GhostBot_t* const a_BotData);

void B_InitNodes(void);
void B_ClearNodes(void);

/*** B_GHOST.C ***/
void B_InitBotCodes(void);

void B_GHOST_Ticker(void);
void B_ClearNodes(void);
void B_InitNodes(void);

void B_BuildBotTicCmd(struct D_XPlayer_s* const a_XPlayer, B_GhostBot_t* const a_BotData, ticcmd_t* const a_TicCmd);
void B_GHOST_Think(B_GhostBot_t* const a_GhostBot, ticcmd_t* const a_TicCmd);

B_BotTemplate_t* B_GHOST_FindTemplate(const char* const a_Name);
B_BotTemplate_t* B_GHOST_RandomTemplate(void);

void B_XDestroyBot(B_GhostBot_t* const a_BotData);
void B_RemoveThinker(thinker_t* const a_Thinker);

#endif /* __B_BOT_H__ */


