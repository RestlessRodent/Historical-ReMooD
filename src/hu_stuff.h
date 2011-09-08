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
// DESCRIPTION: Head up display

#ifndef __HU_STUFF_H__
#define __HU_STUFF_H__

#include "d_event.h"
#include "w_wad.h"
#include "wi_stuff.h"
#include "r_defs.h"

//------------------------------------
//           heads up font
//------------------------------------
#define HU_FONTSTART    '!'		// the first font characters
#define HU_FONTEND      '_'		// the last font characters

#define HU_FONTSIZE     (HU_FONTEND - HU_FONTSTART + 1)

#define HU_CROSSHAIRS   3		// maximum 9 see HU_Init();

extern char *shiftxform;		// french/english translation shift table

extern char english_shiftxform[];
extern char french_shiftxform[];

//------------------------------------
//           chat stuff
//------------------------------------
#define HU_BROADCAST    5		// first char in chat message

#define HU_MAXMSGLEN    80

extern patch_t *hu_font[HU_FONTSIZE];

//set true by hu_ when entering a chat message
extern bool_t chat_on;

// P_DeathThink set this true to show scores while dead, in dmatch
extern bool_t hu_showscores;
extern bool_t playerdeadview;

// init heads up data at game startup.
void HU_Init(void);

// reset heads up when consoleplayer respawns.
void HU_Start(void);

//
bool_t HU_Responder(event_t * ev);

//
void HU_Ticker(void);
void HU_Drawer(void);
char HU_dequeueChatChar(void);
void HU_Erase(void);

// used by console input
char ForeignTranslation(unsigned char ch);

// set chatmacros cvars point the original or dehacked texts, before
// config.cfg is executed !!
void HU_HackChatmacros(void);

// chatmacro <0-9> "message" console command
void Command_Chatmacro_f(void);

int HU_CreateTeamFragTbl(fragsort_t * fragtab, int dmtotals[], int fragtbl[MAXPLAYERS][MAXPLAYERS]);

void HU_SetTip(char *tip, int displaytics);
void HU_ClearTips();
int HU_GetFSPic(int lumpnum, int xpos, int ypos);
int HU_DeleteFSPic(int handle);
int HU_ModifyFSPic(int handle, int lumpnum, int xpos, int ypos);
void HU_DrawFSPics();
void HU_ClearFSPics();

// GhostlyDeath <October 24, 2010> -- Dynamic WAD support
void HU_UnloadWadData(void);
void HU_LoadWadData(void);

#endif

