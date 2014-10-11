// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Cheat code checking.

#ifndef __M_CHEAT_H__
#define __M_CHEAT_H__

/***************
*** INCLUDES ***
***************/



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

