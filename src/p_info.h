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
// Copyright (C) 2008-2011 GhostlyDeath (ghostlydeath@gmail.com)
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

/*** NEW LEVEL INFO CODE ***/

void P_PrepareLevelInfoEx(void);

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

