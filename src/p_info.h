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
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
// Copyright(C) 2000 Simon Howard
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

#include "command.h"

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
	PLIBL_LUMPHEADER,							// Set by lump header
} P_LevelInfoBitLevels_t;

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
	
	/* Info */
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
	char* NormalNext;							// Next level to play after this
	char* SecretNext;							// Secret level to play after this
	fixed_t Gravity;							// Level gravity
	char* StoryFlat;							// Flat to use in story mode
	uint64_t Weapons;							// Weapons
	char* BootCommand;							// Command to execute on map start
} P_LevelInfoEx_t;

/*** GLOBALS ***/
extern P_LevelInfoEx_t* g_CurrentLevelInfo;		// Current level being played

/*** FUNCTIONS ***/
void P_PrepareLevelInfoEx(void);
P_LevelInfoEx_t* P_FindLevelByNameEx(const char* const a_Name, P_LevelInfoEx_t*** const a_LocPtr);

/*** OLD JUNKY DEPRECATED JUNK ***/

void P_LoadLevelInfo(int lumpnum);

void P_CleanLine(char* line);

extern char* info_interpic;
extern char* info_levelname;
extern char* info_levelpic;
extern char* info_music;
extern int info_partime;
extern char* info_levelcmd[128];
extern char* info_skyname;
extern char* info_creator;
extern char* info_nextlevel;
extern char* info_nextsecret;
extern char* info_intertext;
extern char* info_backdrop;
extern int info_scripts;		// whether the current level has scripts

extern bool_t default_weaponowned[NUMWEAPONS];

// level menu
// level authors can include a menu in their level to
// activate special features

typedef struct
{
	char* description;
	int scriptnum;
} levelmenuitem_t;

#define isnumchar(c) ( (c) >= '0' && (c) <= '9')
int isExMy(char* name);
int isMAPxy(char* name);

/*#define isExMy(s) ( (tolower((s)[0]) == 'e') && \
                    (isnumchar((s)[1])) &&      \
                    (tolower((s)[2]) == 'm') && \
                    (isnumchar((s)[3])) &&      \
                    ((s)[4] == '\0') )
#define isMAPxy(s) ( (strlen(s) == 5) && \
                     (tolower((s)[0]) == 'm') && \
                     (tolower((s)[1]) == 'a') && \
                     (tolower((s)[2]) == 'p') && \
                     (isnumchar((s)[3])) &&      \
                     (isnumchar((s)[4])) &&      \
                     ((s)[5] == '\0'))*/

void P_Info_AddCommands();
char* P_LevelName();
char* P_LevelNameByNum(int episode, int map);

#endif

