// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Status bar code.

#ifndef __STSTUFF_H__
#define __STSTUFF_H__

#include "doomtype.h"

/* Define D_Prof_t */
#if !defined(__REMOOD_DPROFTDEFINED)
	#define __REMOOD_DPROFTDEFINED
	typedef struct D_Prof_s D_Prof_t;
#endif

/* Define player_t */
#if !defined(__REMOOD_PLAYERT_DEFINED)
	typedef struct player_s player_t;
	#define __REMOOD_PLAYERT_DEFINED
#endif

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
void SB_Init(void);

/*****************************************************************************/

/*** STRUCTURES ***/

typedef void (*ST_BarFunc_t)(const size_t a_PID, const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H, player_t* const a_ConsoleP, player_t* const a_DisplayP, D_Prof_t* a_Profile);
typedef void (*ST_ModShapeFunc_t)(const size_t a_PID, int32_t* const a_X, int32_t* const a_Y, int32_t* const a_W, int32_t* const a_H, player_t* const a_ConsoleP, player_t* const a_DisplayP, D_Prof_t* a_Profile);

/*** FUNCTIONS ***/

void ST_GetScreenCDP(const int32_t a_Split, player_t** const a_ConsolePP, player_t** const a_DisplayPP, D_Prof_t** const a_ProfP);
void ST_CalcScreen(const int32_t a_ThisPlayer, int32_t* const a_X, int32_t* const a_Y, int32_t* const a_W, int32_t* const a_H);
void ST_DrawPlayerBarsEx(void);
void ST_InitEx(void);
void ST_TickerEx(void);

int32_t ST_ExViewBarHeight(void);
bool_t ST_ExSoloViewTransSBar(void);
bool_t ST_ExSoloViewScaledSBar(void);

bool_t ST_CheckDrawGameView(const int32_t a_Screen);

#endif

