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
// Copyright (C) 2008-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Cheat code checking.

#ifndef __M_CHEAT_H__
#define __M_CHEAT_H__

/***************
*** INCLUDES ***
***************/

#include "d_event.h"

#if defined(NEWCHEATS)

/****************
*** CONSTANTS ***
****************/

/*****************
*** STRUCTURES ***
*****************/

/* M_Cheat_t -- A cheat */
typedef struct M_Cheat_s
{
	/* Static */
	char* OldSeq;				// Letters to activate cheat
	uint8_t Games;				// Doom, Doom 2, Heretic?
	
	/* Dynamic */
	char* Seq;					// Sequence set by dehacked
} M_Cheat_t;

/**************
*** GLOBALS ***
**************/

/*****************
*** PROTOTYPES ***
*****************/

#else
//
// CHEAT SEQUENCE PACKAGE
//

#define SCRAMBLE(a) \
((((a)&1)<<7) + (((a)&2)<<5) + ((a)&4) + (((a)&8)<<1) \
 + (((a)&16)>>1) + ((a)&32) + (((a)&64)>>5) + (((a)&128)>>7))

typedef struct
{
	uint8_t* sequence;
	uint8_t* p;

} cheatseq_t;

int cht_CheckCheat(cheatseq_t* cht, char key);

void cht_GetParam(cheatseq_t* cht, char* buffer);

bool_t cht_Responder(event_t* ev);
void cht_Init();

void Command_CheatNoClip_f(void);
void Command_CheatGod_f(void);
void Command_CheatGimme_f(void);

// Summoning
void Command_CheatSummon_f(void);
void Command_CheatSummonFriend_f(void);
#endif

/*** CONSTANTS ***/

typedef enum M_CheatFlag_s
{
	MCF_FREEZETIME					= 0x00000001, // Freeze Time
} M_CheatFlag_t;

/*** GLOBALS ***/

extern uint32_t g_CheatFlags;					// Global cheat flags

/*** FUNCTIONS ***/

void M_CheatInit(void);

#endif							/* __M_CHEAT_H__ */

