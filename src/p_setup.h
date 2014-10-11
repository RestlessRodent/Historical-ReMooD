// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Setup a game, startup stuff.

#ifndef __P_SETUP__
#define __P_SETUP__

#include "g_state.h"
#include "p_info.h"
#include "r_defs.h"

/* Define mapthing_t */
#if !defined(__REMOOD_MAPTHINGT_DEFINED)
	typedef struct mapthing_s mapthing_t;
	#define __REMOOD_MAPTHINGT_DEFINED
#endif






//extern  mapthing_t**    deathmatch_p;

extern int lastloadedmaplumpnum;	// for comparative savegame

char* P_FlatNameForNum(int num);

extern int nummapthings;
extern mapthing_t* mapthings;

// NOT called by W_Ticker. Fixme.
bool_t P_SetupLevel(int episode, int map, G_Skill_t skill, char* mapname);

subsector_t* R_PointInSubsector(fixed_t x, fixed_t y);

extern bool_t newlevel;
extern bool_t doom1level;
extern char* levelmapname;

void P_SetupLevelSky(void);

/*******************************************************************************
********************************************************************************
*******************************************************************************/

/*** CONSTANTS ***/

/* P_ExLLFlags_t -- Load level flags */
typedef enum P_ExLLFlags_e
{
	PEXLL_NOPLREVIVE			= 0x0000001,	// Do not revive players
	PEXLL_NOSPAWNMAPTHING		= 0x0000002,	// Do not spawn map things
	PEXLL_NOSPAWNSPECIALS		= 0x0000004,	// Do not spawn specials
	PEXLL_NOINITBRAIN			= 0x0000008,	// Do not initialize brain targets
	PEXLL_NOFINALIZE			= 0x0000010,	// Do not finalize the level
	PEXLL_NOCLEARLEVEL			= 0x0000020,	// Does not clear the level
	
	PEXLL_FROMSAVE = PEXLL_NOPLREVIVE | PEXLL_NOSPAWNMAPTHING | PEXLL_NOSPAWNSPECIALS | PEXLL_NOINITBRAIN | PEXLL_NOFINALIZE | PEXLL_NOCLEARLEVEL,
} P_ExLLFlags_t;

/*** STRUCTURES ***/

/*** PROTOTYPES ***/
void P_InitSetupEx(void);

bool_t P_ExClearLevel(void);

bool_t P_ExLoadLevel(P_LevelInfoEx_t* const a_Info, const uint32_t a_Flags);
bool_t P_ExFinalizeLevel(void);

#endif

