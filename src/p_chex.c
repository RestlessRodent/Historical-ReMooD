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
// Copyright (C) 1998-2000 by DooM Legacy Team.
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

#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "r_main.h"
#include "r_state.h"
#include "s_sound.h"
#include "m_random.h"
#include "m_cheat.h"
#include "dstrings.h"
#include "p_chex.h"

extern uint8_t cheat_mus_seq[];
extern uint8_t cheat_choppers_seq[];
extern uint8_t cheat_god_seq[];
extern uint8_t cheat_ammo_seq[];
extern uint8_t cheat_ammonokey_seq[];
extern uint8_t cheat_noclip_seq[];
extern uint8_t cheat_commercial_noclip_seq[];
extern uint8_t cheat_powerup_seq[7][10];
extern uint8_t cheat_clev_seq[];
extern uint8_t cheat_mypos_seq[];
extern uint8_t cheat_amap_seq[];

void Chex1PatchEngine(void)
{

	//patch new text
	char *NEW_QUIT1 = "Don't give up now...do\nyou still wish to quit?";
	char *NEW_QUIT2 = "please don't leave we\nneed your help!";

	text[QUITMSG_NUM] = NEW_QUIT1;
	text[QUITMSG1_NUM] = NEW_QUIT2;
	text[QUITMSG2_NUM] = NEW_QUIT2;
	text[QUITMSG3_NUM] = NEW_QUIT2;
	text[QUITMSG4_NUM] = NEW_QUIT2;
	text[QUITMSG5_NUM] = NEW_QUIT2;
	text[QUITMSG6_NUM] = NEW_QUIT2;
	text[QUITMSG7_NUM] = NEW_QUIT2;

	text[QUIT2MSG_NUM] = NEW_QUIT1;
	text[QUIT2MSG1_NUM] = NEW_QUIT2;
	text[QUIT2MSG2_NUM] = NEW_QUIT2;
	text[QUIT2MSG3_NUM] = NEW_QUIT2;
	text[QUIT2MSG4_NUM] = NEW_QUIT2;
	text[QUIT2MSG5_NUM] = NEW_QUIT2;
	text[QUIT2MSG6_NUM] = NEW_QUIT2;

	text[HUSTR_E1M1_NUM] = "E1M1: Landing Zone";
	text[HUSTR_E1M2_NUM] = "E1M2: Storage Facility";
	text[HUSTR_E1M3_NUM] = "E1M3: Experimental Lab";
	text[HUSTR_E1M4_NUM] = "E1M4: Arboretum";
	text[HUSTR_E1M5_NUM] = "E1M5: Caverns of Bazoik";

	text[GOTCLIP_NUM] = "picked up mini zorch recharge.";
	text[GOTCLIPBOX_NUM] = "Picked up a mini zorch pack.";
	text[GOTARMBONUS_NUM] = "picked up slime repellant.";
	text[GOTSTIM_NUM] = "picked up bowl of fruit.";
	text[GOTHTHBONUS_NUM] = "picked up glass of water.";
	text[GOTMEDIKIT_NUM] = "picked up bowl of vegetables.";
	text[GOTMEDINEED_NUM] = "Picked up some needed vegetables!";
	text[GOTARMOR_NUM] = "Picked up the Chex(R) Armor.";
	text[GOTMEGA_NUM] = "Picked up the Super Chex(R) Armor!";
	text[GOTSUPER_NUM] = "Supercharge Breakfast!";
	text[GOTSUIT_NUM] = "Slimeproof Suit";

	//patch monster changes
	mobjinfo[MT_POSSESSED].missilestate = 0;
	mobjinfo[MT_POSSESSED].meleestate = S_POSS_ATK1;

	mobjinfo[MT_SHOTGUY].missilestate = 0;
	mobjinfo[MT_SHOTGUY].meleestate = S_SPOS_ATK1;
}
