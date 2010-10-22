// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.sourceforge.net/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           Demyx                  (demyx@endgameftw.com)
//                    Dragan                 (poliee13@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2009 The ReMooD Team..
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
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
extern boolean st_statusbaron;
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
boolean ST_Responder(event_t * ev);

// Called by main loop.
void ST_Ticker(void);

// Called by main loop.
void ST_Drawer(boolean refresh);

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
extern boolean st_recalc;

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

boolean ST_Responder(event_t * ev);

// face load/unload graphics, called when skin changes
void ST_loadFaceGraphics(char *facestr);
void ST_unloadFaceGraphics(void);

// return if player a is in the same team of the player b
boolean ST_SameTeam(player_t * a, player_t * b);

// get the frags of the player
// only one function for calculation : more simple code
int ST_PlayerFrags(int playernum);

//--------------------
// status bar overlay
//--------------------
extern boolean st_overlay;		// sb overlay on or off when fullscreen

#define TRANSPARENTSTATUSBAR (cv_transparentstatusbar.value && cv_viewsize.value > 9 && (!automapactive || (automapactive && automapoverlay)))
#define STTRANSPARENTSCREEN \
	((cv_scalestatusbar.value || cv_viewsize.value >= 11) ?\
	((TRANSPARENTSTATUSBAR ? BG : FG) | V_SCALESTART | V_TRANSLUCENTPATCH) :\
	((TRANSPARENTSTATUSBAR ? BG : FG) | V_NOSCALEPATCH | V_NOSCALESTART | V_TRANSLUCENTPATCH))
	
// heretic status bar
void SB_Ticker(void);
void SB_Drawer(boolean refresh);
boolean SB_Responder(event_t * event);
void SB_Init(void);

#endif

