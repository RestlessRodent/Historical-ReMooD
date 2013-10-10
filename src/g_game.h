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
// DESCRIPTION:

#ifndef __G_GAME__
#define __G_GAME__

#include "doomtype.h"

/* Define player_t */
#if !defined(__REMOOD_PLAYERT_DEFINED)
	typedef struct player_s player_t;
	#define __REMOOD_PLAYERT_DEFINED
#endif

/* Define mobj_t */
#if !defined(__REMOOD_MOBJT_DEFINED)
	typedef struct mobj_s mobj_t;
	#define __REMOOD_MOBJT_DEFINED
#endif

/* Define ticcmd_t */
#if !defined(__REMOOD_TICCMDT_DEFINED)
	typedef union ticcmd_u ticcmd_t;
	#define __REMOOD_TICCMDT_DEFINED
#endif

/* Define WL_ES_t */
#if !defined(__REMOOD_WLEST_DEFINED)
	typedef struct WL_ES_s WL_ES_t;
	#define __REMOOD_WLEST_DEFINED
#endif

/* Define UnicodeStringID_t */
#if !defined(__REMOOD_UNICSTRID_DEFINED)
	typedef int UnicodeStringID_t;
	#define __REMOOD_UNICSTRID_DEFINED
#endif







//added:11-02-98: yeah now you can change it!
// changed to 2d array 19990220 by Kin

extern bool_t nomonsters;		// checkparm of -nomonsters

#define GAMEMAPNAMESIZE 128
extern char gamemapname[GAMEMAPNAMESIZE];


// ======================================
// DEMO playback/recording related stuff.
// ======================================

// demoplaying back and demo recording


// Quit after playing a demo from cmdline.


// gametic at level start
extern tic_t levelstarttic;

extern uint32_t g_CheatFlags;

/* Prototypes */
char* G_BuildMapName(int episode, int map);
int16_t G_ClipAimingPitch(int32_t* aiming);
void G_DoReborn(int playernum);
bool_t G_DeathMatchSpawnPlayer(int playernum);
void G_CoopSpawnPlayer(int playernum);
void G_PlayerReborn(int player);
void G_DoLoadLevel(bool_t resetplayer);
void G_LoadGame(int slot);		// Can be called by the startup code or M_Responder
void G_DoLoadGame(int slot);	// Can be called by the startup code or M_Responder
void G_DoSaveGame(int slot, char* description);	// Called by M_Responder.
void G_SaveGame(int slot, char* description);	// Called by M_Responder.
void G_DoneLevelLoad(void);

void G_ExitLevel(const bool_t a_Secret, mobj_t* const a_Activator, const char* const a_Message);

void G_NextLevel(void);

uint32_t G_CalcSyncCode(const bool_t a_Debug);
void G_Ticker(void);
bool_t G_Downgrade(int version);
void G_PrepareDemoStuff(void);

player_t* G_AddPlayer(int playernum);
void G_InitPlayer(player_t* const a_Player);
void G_ResetPlayer(player_t* const a_Player);

void G_DoCompleted(void);
void G_DoVictory(void);
void G_DoWorldDone(void);

extern uint8_t* demo_p;
extern uint8_t* demoend;

/*******************
*** DEMO FACTORY ***
*******************/

typedef struct G_CDemo_s G_CDemo_t;
typedef struct G_DemoFactory_s G_DemoFactory_t;

extern tic_t g_DemoTime;

const G_DemoFactory_t* G_DemoFactoryByName(const char* const a_Name);
void G_DemoQueue(const char* const a_Name);
bool_t G_PlayNextQ(void);
G_CDemo_t* G_DemoPlay(WL_ES_t* const a_Stream, const G_DemoFactory_t* const a_Factory);

void G_RecordDemo(char* name);	// Only called by startup code.
void G_StopDemo(void);
void G_StopDemoRecord(void);
void G_StopDemoPlay(void);
void G_BeginRecording(const char* const a_Output, const char* const a_FactoryName);
void G_DoPlayDemo(char* defdemoname, const bool_t a_TitleScreen);
void G_TimeDemo(char* name);
void G_DeferedPlayDemo(char* demo);
bool_t G_CheckDemoStatus(void);
bool_t G_GetDemoExplicit(void);
void G_EncodeSaveGame(void);
bool_t G_UseDemoSyncCode(void);

void G_ReadStartTic(uint32_t* const a_Code);
void G_WriteStartTic(const uint32_t a_Code);
void G_ReadEndTic(uint32_t* const a_Code);
void G_WriteEndTic(const uint32_t a_Code);

void G_ReadDemoGlobalTicCmd(ticcmd_t* const a_TicCmd);
void G_WriteDemoGlobalTicCmd(ticcmd_t* const a_TicCmd);
void G_ReadDemoTiccmd(ticcmd_t* cmd, int playernum);
void G_WriteDemoTiccmd(ticcmd_t* cmd, int playernum);

void G_DemoPreGTicker(void);
void G_DemoPostGTicker(void);
uint32_t G_GetDemoHostID(void);

void G_DemoProblem(const bool_t a_IsError, const UnicodeStringID_t a_StrID, const char* const a_Format, ...);

#endif							/* __G_GAME_H__ */

