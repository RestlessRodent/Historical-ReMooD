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
// DESCRIPTION: Status bar code.
//              Does the face/direction indicator animatin.
//              Does palette indicators as well (red pain/berserk, bright pickup)

#ifndef __STSTUFF_H__
#define __STSTUFF_H__

#include "doomtype.h"
#include "d_event.h"
#include "d_player.h"

extern int ST_Y;
extern int st_x;
extern bool_t st_statusbaron;
extern float st_scalex;
extern float st_scaley;
extern int stbarheight;
extern int st_borderpatchnum;

// update all global position variables (just above)
void ST_CalcPos(void);

//
// STATUS BAR
//

void ST_ExternrefreshBackground(void);

// Called by main loop.
bool_t ST_Responder(event_t* ev);

// Called by main loop.
void ST_Ticker(void);

// Called by main loop.
void ST_Drawer(bool_t refresh);

// Called when the console player is spawned on each level.
void ST_Start(void);

// Called by startup code.
void ST_Init(void);

// Called by G_Responder() when pressing F12 while viewing a demo.
void ST_changeDemoView(void);

// Add status bar related commands & vars
void ST_AddCommands(void);

// force redraw
void ST_Invalidate(void);

// need this for SCR_Recalc() coz widgets coords change with resolutions
extern bool_t st_recalc;

// States for status bar code.
typedef enum
{
	AutomapState,
	FirstPersonState
} st_stateenum_t;

// States for the chat code.
typedef enum
{
	StartChatState,
	WaitDestState,
	GetChatState
} st_chatstateenum_t;

bool_t ST_Responder(event_t* ev);

// face load/unload graphics, called when skin changes
void ST_loadFaceGraphics(char* facestr);
void ST_unloadFaceGraphics(void);

// return if player a is in the same team of the player b
bool_t ST_SameTeam(player_t* a, player_t* b);

// get the frags of the player
// only one function for calculation : more simple code
int ST_PlayerFrags(int playernum);

//--------------------
// status bar overlay
//--------------------
extern bool_t st_overlay;		// sb overlay on or off when fullscreen

#define TRANSPARENTSTATUSBAR (cv_transparentstatusbar.value && cv_viewsize.value > 9 && (!automapactive || (automapactive && automapoverlay)))
#define STTRANSPARENTSCREEN \
	((cv_scalestatusbar.value || cv_viewsize.value >= 11) ?\
	((TRANSPARENTSTATUSBAR ? BG : FG) | V_SCALESTART | V_TRANSLUCENTPATCH) :\
	((TRANSPARENTSTATUSBAR ? BG : FG) | V_NOSCALEPATCH | V_NOSCALESTART | V_TRANSLUCENTPATCH))

// heretic status bar
void SB_Ticker(void);
void SB_Drawer(bool_t refresh);
bool_t SB_Responder(event_t* event);
void SB_Init(void);

/*****************************************************************************/

/*** FUNCTIONS ***/

void ST_DrawPlayerBarsEx(void);
void ST_InitEx(void);
void ST_TickerEx(void);

int32_t ST_ExViewBarHeight(void);
bool_t ST_ExSoloViewTransSBar(void);
bool_t ST_ExSoloViewScaledSBar(void);

bool_t ST_CheckDrawGameView(const int32_t a_Screen);

#endif

