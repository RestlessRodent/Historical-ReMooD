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
// DESCRIPTION: Doom/Hexen game states

#ifndef __G_STATE_H__
#define __G_STATE_H__

/*****************************************************************************/

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"

/****************
*** CONSTANTS ***
****************/

/* G_State_t -- Current game state */
typedef enum
{
	GS_NULL = 0,				// at begin
	GS_LEVEL,					// we are playing
	GS_INTERMISSION,			// gazing at the intermission screen
	GS_FINALE,					// game final animation
	GS_DEMOSCREEN,				// looking at a demo
	//legacy
	GS_DEDICATEDSERVER,			// added 27-4-98 : new state for dedicated server
	GS_WAITINGPLAYERS,			// added 3-9-98 : waiting player in net game
	
	// GhostlyDeath <May 17, 2012> -- Waiting for join window
	GS_WAITFORJOINWINDOW,						// Player must wait to join
	
} G_State_t;

/* G_Action_t -- Current Action */
typedef enum
{
	ga_nothing,
	ga_completed,
	ga_worlddone,
} G_Action_t;

typedef enum
{
	shareware,					// DOOM 1 shareware, E1, M9
	registered,					// DOOM 1 registered, E3, M27
	commercial,					// DOOM 2 retail, E1 M34
	// DOOM 2 german edition not handled
	retail,						// DOOM 1 retail, E4, M36
	indetermined,				// Well, no IWAD found.
	chexquest1,					// DarkWolf95:July 14, 2003: Chex Quest Support
	
	numgamemodes,
	heretic
} gamemode_t;

// Mission packs - might be useful for TC stuff?
typedef enum
{
	doom,						// DOOM 1
	doom2,						// DOOM 2
	pack_tnt,					// TNT mission pack
	pack_plut,					// Plutonia pack
	pack_chex,					// Chex Quest
	none,
	
	numgamemissions
} gamemission_t;

/* CoreIWADFlags_t -- IWAD flags */
typedef enum CoreIWADFlags_e
{
	CIF_SHAREWARE				= 0x0000001,	// Shareware Mode
	CIF_COMMERCIAL				= 0x0000002,	// Commercial
	CIF_REGISTERED				= 0x0000004,	// Registered Mode
	CIF_CANFILE					= 0x0000008,	// Can -file -deh, etc.
	CIF_EXTENDED				= 0x0000010,	// Extended Mode
	CIF_DOWNLOADABLE			= 0x0000020,	// Can be downloaded
	CIF_DOUBLEWARP				= 0x0000040,	// Warp Takes two arguments
	CIF_FREEDOOM				= 0x0000080,	// Free content!
} CoreIWADFlags_t;

/* CoreGame_t -- Game being played... */
typedef enum CoreGame_e
{
	CG_UNKNOWN,									// Unknown game
	CG_DOOM		= UINT32_C(0x01),				// Doom is being played
	CG_HERETIC	= UINT32_C(0x02),				// Heretic is being played
	CG_HEXEN	= UINT32_C(0x04),				// Hexen is being played
	CG_STRIFE	= UINT32_C(0x08),				// Strife is being played
	
	CG_DOOMHER	= CG_DOOM | CG_HERETIC,
	CG_ALL = CG_DOOM | CG_HERETIC | CG_HEXEN | CG_STRIFE,
	
	NUMCOREGAMES
} CoreGame_t;

// skill levels
typedef enum
{
	sk_baby,
	sk_easy,
	sk_medium,
	sk_hard,
	sk_nightmare
} G_Skill_t;

/* P_GameMode_t -- Game Modes */
typedef enum P_GameMode_e
{
	PGM_COOP,									// Cooperative
	PGM_COUNTEROP,								// Counteroperative
	PGM_DM,										// Deathmatch
	PGM_CTF,									// CTF
	PGM_FLAGTAG,								// Hold the Flag
	PGM_KOTH,									// King of the Hill
	PGM_POPACAP,								// Pop a Cap
	PGM_LMS,									// Last Man Standing
	PGM_SURVIVAL,								// Survival
	
	PGM_CUSTOM,									// Custom Game Mode
	
	NUMPGAMEMODES
} P_GameMode_t;

typedef enum D_ModelMode_s
{
	DMM_DEFAULT,								// Default PC
	DMM_GCW,									// GCW Zero
	DMM_WII,									// Nintendo Wii
} D_ModelMode_t;

/*****************
*** STRUCTURES ***
*****************/

/* D_IWADInfoEx_t -- Extended IWAD Info */
typedef struct D_IWADInfoEx_s
{
	/* Base Info */
	const char* NiceTitle;						// Nice IWAD Title name
	const char* NetName;						// Network Name
	const char* ForceNames;						// Names for forcing [conf = 500]
	const char* BaseName;						// WAD Basename [conf = 5]
	const char* SimpleSum;						// Simple sum of WAD [conf = 40]
	const char* MD5Sum;							// MD5 Sum of WAD [conf = 50]
	const char* SHA1Sum;						// SHA-1 Sum of WAD [conf = 60]
	const char* Lumps;							// Identifying lumps [conf = 1]
	const char* BonusLumps;						// Unique Lumps [conf = 35]
	uint32_t Size;								// Size of WAD [conf = 15]
	uint32_t NumLumps;							// Number of lumps in WAD [conf = 15]
	
	/* Game Info */
	CoreGame_t CoreGame;						// Core Game
	bool_t CanDistrib;							// Distributable? (Not illegal to give away)
	const char* MapInfoLump;					// Map Info Lump
	uint32_t Flags;								// Flags for game
	const char* MapNameFormat;					// Format of map names
	
	gamemission_t mission;						// Deprecated mission
	gamemode_t mode;							// Deprecated mode
} D_IWADInfoEx_t;

/**************
*** GLOBALS ***
**************/

extern G_State_t gamestate;
extern G_State_t wipegamestate;

extern gamemode_t gamemode;
extern gamemission_t gamemission;

extern G_Action_t gameaction;

extern CoreGame_t g_CoreGame;					// Core game mode
extern const void* g_ReMooDPtr;					// Pointer to remood.wad
extern const char* g_IWADMapInfoName;			// Name of IWAD MAPINFO
extern uint32_t g_IWADFlags;					// IWAD Flags

extern bool_t g_DedicatedServer;				// Dedicated Server

extern tic_t gametic;
extern tic_t g_ProgramTic;
extern tic_t pagetic;
extern int demosequence;

extern bool_t singletics;

extern bool_t demoplayback;
extern bool_t demorecording;
extern bool_t timingdemo;
extern bool_t singledemo;
extern bool_t g_TitleScreenDemo;

extern D_ModelMode_t g_ModelMode;				// Model to use

extern bool_t paused;			// Game Pause?

extern bool_t noblit;

extern tic_t leveltime;

extern int totalkills;
extern int totalitems;
extern int totalsecret;

extern int32_t g_MapKIS[5];

/****************
*** FUNCTIONS ***
****************/

D_IWADInfoEx_t* D_GetThisIWAD(void);
D_IWADInfoEx_t* D_GetIWADInfoByNum(const uint32_t a_Num);
const char* D_FieldNumber(const char* const a_Str, const size_t a_Num);

void D_InitModelMode(void);

// Was the only prototype in p_tick.h
void P_Ticker(void);

#endif /* __G_STATE_H__ */

