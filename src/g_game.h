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
// DESCRIPTION:

#ifndef __G_GAME__
#define __G_GAME__

#include "doomdef.h"
#include "doomstat.h"
#include "d_event.h"

//added:11-02-98: yeah now you can change it!
// changed to 2d array 19990220 by Kin
extern char player_names[MAXPLAYERS][MAXPLAYERNAME];
extern char team_names[MAXPLAYERS][MAXPLAYERNAME * 2];

extern bool_t nomonsters;		// checkparm of -nomonsters

#define GAMEMAPNAMESIZE 128
extern char gamemapname[GAMEMAPNAMESIZE];

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

extern angle_t localangle[MAXSPLITSCREENPLAYERS];
extern int localaiming[MAXSPLITSCREENPLAYERS];	// should be a angle_t but signed

/* Prototypes */
char* G_BuildMapName(int episode, int map);
short G_ClipAimingPitch(int* aiming);
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
void G_ExitLevel(void);
void G_SecretExitLevel(void);
void G_NextLevel(void);
void G_Ticker(void);
bool_t G_Responder(event_t* ev);
bool_t G_Downgrade(int version);
void G_PrepareDemoStuff(void);

player_t* G_AddPlayer(int playernum);
void G_InitPlayer(player_t* const a_Player);

void G_DoCompleted(void);
void G_DoVictory(void);
void G_DoWorldDone(void);

extern uint8_t* demo_p;
extern uint8_t* demoend;

/*******************
*** DEMO FACTORY ***
*******************/

struct G_CurrentDemo_s;

typedef bool_t (*G_DEMO_StartPlayingType_t)(struct G_CurrentDemo_s* a_Current);
typedef bool_t (*G_DEMO_StopPlayingType_t)(struct G_CurrentDemo_s* a_Current);
typedef bool_t (*G_DEMO_StartRecordType_t)(struct G_CurrentDemo_s* a_Current);
typedef bool_t (*G_DEMO_StopRecordType_t)(struct G_CurrentDemo_s* a_Current);
typedef bool_t (*G_DEMO_CheckDemoType_t)(struct G_CurrentDemo_s* a_Current);
typedef bool_t (*G_DEMO_ReadTicCmdType_t)(struct G_CurrentDemo_s* a_Current, ticcmd_t* const a_Cmd, const int32_t a_PlayerNum);
typedef bool_t (*G_DEMO_WriteTicCmdType_t)(struct G_CurrentDemo_s* a_Current, const ticcmd_t* const a_Cmd, const int32_t a_PlayerNum);

typedef bool_t (*G_DEMO_PreGTickCmdType_t)(struct G_CurrentDemo_s* a_Current);
typedef bool_t (*G_DEMO_PostGTickCmdType_t)(struct G_CurrentDemo_s* a_Current);

/* G_DemoFactory_t -- Demo Factory */
typedef struct G_DemoFactory_s
{
	const char* FactoryName;					// Name of factory
	bool_t DoesRBS;								// Doe RBS Stream
	G_DEMO_StartPlayingType_t StartPlayingFunc;	// Starts playing demo
	G_DEMO_StopPlayingType_t StopPlayingFunc;	// Stops playing demo
	G_DEMO_StartRecordType_t StartRecordFunc;	// Starts recording demo
	G_DEMO_StopRecordType_t StopRecordFunc;		// Stops recording demo
	G_DEMO_CheckDemoType_t CheckDemoFunc;		// Check Demo's Status (quit)
	G_DEMO_ReadTicCmdType_t ReadTicCmdFunc;		// Reads tic command
	G_DEMO_WriteTicCmdType_t WriteTicCmdFunc;	// Writes tic command
	G_DEMO_PreGTickCmdType_t PreGTickCmdFunc;	// Pre G_Ticker() Command
	G_DEMO_PreGTickCmdType_t PostGTickCmdFunc;	// Post G_Ticker() Command
} G_DemoFactory_t;

/* G_CurrentDemo_t -- Current Demo Info */
typedef struct G_CurrentDemo_s
{
	bool_t Out;									// Demo is out (being written)
	const G_DemoFactory_t* Factory;				// Factory for demo
	void* CFile;								// CFile
	WL_EntryStream_t* WLStream;					// Demo Streamer (Raw)
	D_BS_t* RBSStream;				// Block Streamer
	void* Data;									// Internal Data
} G_CurrentDemo_t;

extern tic_t g_DemoTime;

const G_DemoFactory_t* G_DemoFactoryByName(const char* const a_Name);
void G_DemoQueue(const char* const a_Name);
bool_t G_PlayNextQ(void);
G_CurrentDemo_t* G_DemoPlay(WL_EntryStream_t* const a_Stream, const G_DemoFactory_t* const a_Factory);

void G_RecordDemo(char* name);	// Only called by startup code.
void G_StopDemo(void);
void G_StopDemoRecord(void);
void G_StopDemoPlay(void);
void G_BeginRecording(const char* const a_Output, const char* const a_FactoryName);
void G_DoPlayDemo(char* defdemoname);
void G_TimeDemo(char* name);
void G_DeferedPlayDemo(char* demo);
bool_t G_CheckDemoStatus(void);

void G_ReadDemoTiccmd(ticcmd_t* cmd, int playernum);
void G_WriteDemoTiccmd(ticcmd_t* cmd, int playernum);
void G_DemoPreGTicker(void);
void G_DemoPostGTicker(void);

void G_DemoProblem(const bool_t a_IsError, const UnicodeStringID_t a_StrID, const char* const a_Format, ...);

#endif							/* __G_GAME_H__ */

