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
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2011 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION: 

#ifndef __G_GAME__
#define __G_GAME__

#include "doomdef.h"
#include "doomstat.h"
#include "d_event.h"

//added:11-02-98: yeah now you can change it!
// changed to 2d array 19990220 by Kin
extern char player_names[MAXPLAYERS][MAXPLAYERNAME];
extern char team_names[MAXPLAYERS][MAXPLAYERNAME*2];

extern bool_t nomonsters;		// checkparm of -nomonsters
extern char gamemapname[128];

extern player_t players[MAXPLAYERS];
extern bool_t playeringame[MAXPLAYERS];

// ======================================
// DEMO playback/recording related stuff.
// ======================================

// demoplaying back and demo recording
extern bool_t demoplayback;
extern bool_t demorecording;
extern bool_t timingdemo;

// Quit after playing a demo from cmdline.
extern bool_t singledemo;

// gametic at level start
extern tic_t levelstarttic;

// used in game menu
extern consvar_t cv_crosshair;
//extern consvar_t  cv_crosshairscale;
extern consvar_t cv_autorun;
extern consvar_t cv_autorun2;
extern consvar_t cv_invertmouse;
extern consvar_t cv_alwaysfreelook;
extern consvar_t cv_mousemove;
extern consvar_t cv_showmessages;
extern consvar_t cv_disabledemos;
extern consvar_t cv_fastmonsters;
extern consvar_t cv_joystickfreelook;
extern consvar_t cv_predictingmonsters;	//added by AC for predmonsters
extern angle_t localangle[MAXSPLITSCREENPLAYERS];
extern int localaiming[MAXSPLITSCREENPLAYERS];	// should be a angle_t but signed

extern CV_PossibleValue_t map_cons_t[];
extern CV_PossibleValue_t skill_cons_t[];
extern CV_PossibleValue_t exmy_cons_t[];

/* Prototypes */
void Command_Turbo_f(void);
char *G_BuildMapName(int episode, int map);
void G_BuildTiccmd(ticcmd_t * cmd, int realtics, int player);
short G_ClipAimingPitch(int *aiming);
void G_DoReborn(int playernum);
bool_t G_DeathMatchSpawnPlayer(int playernum);
void G_CoopSpawnPlayer(int playernum);
void G_PlayerReborn(int player);
void G_InitNew(skill_t skill, char *mapname, bool_t resetplayer);
void G_DeferedInitNew(skill_t skill, char *mapname, int StartSplitScreenGame);
void G_DoLoadLevel(bool_t resetplayer);
void G_DeferedPlayDemo(char *demo);
void G_LoadGame(int slot);		// Can be called by the startup code or M_Responder
void G_DoLoadGame(int slot);	// Can be called by the startup code or M_Responder
void G_DoSaveGame(int slot, char *description);	// Called by M_Responder.
void G_SaveGame(int slot, char *description);	// Called by M_Responder.
void G_RecordDemo(char *name);	// Only called by startup code.
void G_BeginRecording(void);
void G_DoPlayDemo(char *defdemoname);
void G_TimeDemo(char *name);
void G_DoneLevelLoad(void);
void G_StopDemo(void);
bool_t G_CheckDemoStatus(void);
void G_ExitLevel(void);
void G_SecretExitLevel(void);
void G_NextLevel(void);
void G_Ticker(void);
bool_t G_Responder(event_t * ev);
bool_t G_Downgrade(int version);
void G_AddPlayer(int playernum);
bool_t G_CheckDemoStatus(void);
void G_ReadDemoTiccmd(ticcmd_t * cmd, int playernum);
void G_WriteDemoTiccmd(ticcmd_t * cmd, int playernum);
void G_InitNew(skill_t skill, char *mapname, bool_t resetplayer);
void G_DoCompleted(void);
void G_DoVictory(void);
void G_DoWorldDone(void);

extern uint8_t *demo_p;
extern uint8_t *demoend;

#endif							/* __G_GAME_H__ */

