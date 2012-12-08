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
// Copyright (C) 2008-2013 GhostlyDeath (ghostlydeath@gmail.com)
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

#ifndef __P_INFO_H__
#define __P_INFO_H__



#include "w_wad.h"

/**************************
*** NEW LEVEL INFO CODE ***
**************************/

/*** CONSTANTS ***/

#define MAXPLIEXFIELDWIDTH			32
#define MAXCONSOLECOMMANDWIDTH		128

/* P_InfoBlockType_t -- Type of info block */
typedef enum P_InfoBlockType_e
{
	PIBT_LEVELINFO,								// [level info]
	PIBT_SCRIPTS,								// [scripts]
	PIBT_INTERTEXT,								// [intertext]	
	
	NUMPINFOBLOCKTYPES
} P_InfoBlockType_t;

/* P_LevelInfoExDataStuff_t -- Stuff needed to hold level information */
typedef enum P_LevelInfoExDataStuff_e
{
	PLIEDS_HEADER,								// MAPxx, ExMx
	PLIEDS_THINGS,								// THINGS
	PLIEDS_LINEDEFS,							// LINEDEFS
	PLIEDS_SIDEDEFS,							// SIDEDEFS
	PLIEDS_VERTEXES,							// VERTEXES
	PLIEDS_SEGS,								// SEGS
	PLIEDS_SSECTORS,							// SSECTORS
	PLIEDS_NODES,								// NODES
	PLIEDS_SECTORS,								// SECTORS
	PLIEDS_REJECT,								// REJECT
	PLIEDS_BLOCKMAP,							// BLOCKMAP
	PLIEDS_BEHAVIOR,							// BEHAVIOR (Hexen)
	PLIEDS_TEXTMAP,								// TEXTMAP (UDMF)
	PLIEDS_ENDMAP,								// ENDMAP (UDMF)
	PLIEDS_RSCRIPTS,							// ReMooD Scripts (for this level)
	PLIEDS_LPREVIEW,							// Level preview image
	
	MAXPLIEDS
} P_LevelInfoExDataStuff_t;

/* P_LevelInfoBitLevels_t -- Level info bit levels */
typedef enum P_LevelInfoBitLevels_e
{
	PLIBL_NONE,									// Nothing
	PLIBL_MAPINFO,								// Set by MAPINFO
	PLIBL_GENERIC,								// Generic Set
	PLIBL_LUMPHEADER,							// Set by lump header
} P_LevelInfoBitLevels_t;

#define MAXPINFOPROPERNAME	64					// Max Level Proper Name
#define MAXPINFOSETFLAGS 24						// Max stuff to set flags for

/*** STRUCTURES ***/
/* P_LevelInfoEx_t -- Extended level info */
typedef struct P_LevelInfoEx_s
{
	/* WAD Related */
	const WL_WADFile_t* WAD;					// WAD for this level
	const WL_WADEntry_t* EntryPtr[MAXPLIEDS];	// Pointer to entry
	struct
	{
		bool_t Hexen;							// Hexen level (false = Doom)
		bool_t Text;							// Text Level
	} Type;										// Level Type
	
	/* Script Related */
	uint32_t BlockPos[NUMPINFOBLOCKTYPES][2];	// Block positions ([xxx] stuff)
	
	/* Block related */
	bool_t IsComposite;							// Is a composite of information
	
	/* Info */
	char ProperTitle[MAXPINFOPROPERNAME];		// Proper Name of Level
	char LumpName[MAXPLIEXFIELDWIDTH];			// Name of the lump
	char* Title;								// Level Title
	char* Author;								// Creator of level
	struct
	{
		uint8_t Day;							// Day (of month)
		uint8_t Month;							// Month
		uint16_t Year;							// Year
	} Date;										// Date Created
	
	/* Compatibility */
	bool_t Playable;							// Actually playable
	
	/* Settings */
	int8_t SetBits[MAXPINFOSETFLAGS];			// Bits that were set
	char* LevelPic;								// Picture on intermission screen
	int32_t ParTime;							// Par Time
	char* Music;								// Background music to play
	char* SkyTexture;							// Sky texture
	char* InterPic;								// Intermission background
	char* InterMus;								// Intermission music
	char* NormalNext;							// Next level to play after this
	char* SecretNext;							// Secret level to play after this
	fixed_t Gravity;							// Level gravity
	char* StoryFlat;							// Flat to use in story mode
	uint64_t Weapons;							// Weapons
	char* BootCommand;							// Command to execute on map start
	bool_t MapSevenSpecial;						// Map can use DOOM.WAD:MAP07 666/667
	bool_t BaronSpecial;						// Baron of hell special
	bool_t CyberSpecial;						// Cyberdemon special
	bool_t SpiderdemonSpecial;					// Spider Mastermind special
	bool_t ExitOnSpecial;						// Exit on special action
	bool_t OpenDoorOnSpecial;					// Open door on special action
	bool_t LowerFloorOnSpecial;					// Floor lowers on special
	bool_t KillMonstersOnSpecial;				// Kills other monsters on special
	int32_t LevelNum;							// Level number
	int32_t EpisodeNum;							// Episode Number
} P_LevelInfoEx_t;

/*** GLOBALS ***/
extern P_LevelInfoEx_t* g_CurrentLevelInfo;		// Current level being played

/*** FUNCTIONS ***/
void P_PrepareLevelInfoEx(void);
P_LevelInfoEx_t* P_FindLevelByNameEx(const char* const a_Name, P_LevelInfoEx_t*** const a_LocPtr);
const char* P_LevelNameEx(void);

#endif

