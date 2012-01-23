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
// DESCRIPTION:
//      Cheat sequence checking.

/***************
*** INCLUDES ***
***************/

#include "doomdef.h"
#include "dstrings.h"
#include "am_map.h"
#include "m_cheat.h"
#include "g_game.h"
#include "r_local.h"
#include "p_local.h"
#include "p_inter.h"
#include "m_cheat.h"
#include "i_sound.h"			// for I_PlayCD()
#include "s_sound.h"
#include "v_video.h"
#include "st_stuff.h"
#include "w_wad.h"

#if defined(NEWCHEATS)

/*****************
*** STRUCTURES ***
*****************/

/**************
*** GLOBALS ***
**************/

/****************
*** FUNCTIONS ***
****************/

#else
static bool_t HandleCheats(uint8_t key);

// ==========================================================================
//                             CHEAT Structures
// ==========================================================================

uint8_t cheat_mus_seq[] =
{
	0xb2, 0x26, 0xb6, 0xae, 0xea, 1, 0, 0, 0xff
};

//Fab:19-07-98: idcd xx : change cd track
uint8_t cheat_cd_seq[] =
{
	0xb2, 0x26, 0xe2, 0x26, 1, 0, 0, 0xff
};

uint8_t cheat_choppers_seq[] =
{
	0xb2, 0x26, 0xe2, 0x32, 0xf6, 0x2a, 0x2a, 0xa6, 0x6a, 0xea, 0xff	// id...
};

uint8_t cheat_god_seq[] =
{
	0xb2, 0x26, 0x26, 0xaa, 0x26, 0xff	// iddqd
};

uint8_t cheat_ammo_seq[] =
{
	0xb2, 0x26, 0xf2, 0x66, 0xa2, 0xff	// idkfa
};

uint8_t cheat_ammonokey_seq[] =
{
	0xb2, 0x26, 0x66, 0xa2, 0xff	// idfa
};

// Smashing Pumpkins Into Small Pieces Of Putrid Debris.
uint8_t cheat_noclip_seq[] =
{
	0xb2, 0x26, 0xea, 0x2a, 0xb2,	// idspispopd
	0xea, 0x2a, 0xf6, 0x2a, 0x26, 0xff
};

//
uint8_t cheat_commercial_noclip_seq[] =
{
	0xb2, 0x26, 0xe2, 0x36, 0xb2, 0x2a, 0xff	// idclip
};

//added:28-02-98: new cheat to fly around levels using jump !!
uint8_t cheat_fly_around_seq[] =
{
	0xb2, 0x26, SCRAMBLE('f'), SCRAMBLE('l'), SCRAMBLE('y'), 0xff	// idfly
};

uint8_t cheat_powerup_seq[7][10] =
{
	{0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0x6e, 0xff},	// beholdv
	{0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0xea, 0xff},	// beholds
	{0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0xb2, 0xff},	// beholdi
	{0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0x6a, 0xff},	// beholdr
	{0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0xa2, 0xff},	// beholda
	{0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0x36, 0xff},	// beholdl
	{0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0xff}	// behold
};

uint8_t cheat_clev_seq[] =
{
	0xb2, 0x26, 0xe2, 0x36, 0xa6, 0x6e, 1, 0, 0, 0xff	// idclev
};

// my position cheat
uint8_t cheat_mypos_seq[] =
{
	0xb2, 0x26, 0xb6, 0xba, 0x2a, 0xf6, 0xea, 0xff	// idmypos
};

uint8_t cheat_amap_seq[] = { 0xb2, 0x26, 0x26, 0x2e, 0xff };
cheatseq_t cheat_amap = { cheat_amap_seq, 0 };

// Now what?
cheatseq_t cheat_mus = { cheat_mus_seq, 0 };
cheatseq_t cheat_cd = { cheat_cd_seq, 0 };
cheatseq_t cheat_god = { cheat_god_seq, 0 };
cheatseq_t cheat_ammo = { cheat_ammo_seq, 0 };
cheatseq_t cheat_ammonokey = { cheat_ammonokey_seq, 0 };
cheatseq_t cheat_noclip = { cheat_noclip_seq, 0 };
cheatseq_t cheat_commercial_noclip = { cheat_commercial_noclip_seq, 0 };

//added:28-02-98:
cheatseq_t cheat_fly_around = { cheat_fly_around_seq, 0 };

cheatseq_t cheat_powerup[7] =
{
	{cheat_powerup_seq[0], 0},
	{cheat_powerup_seq[1], 0},
	{cheat_powerup_seq[2], 0},
	{cheat_powerup_seq[3], 0},
	{cheat_powerup_seq[4], 0},
	{cheat_powerup_seq[5], 0},
	{cheat_powerup_seq[6], 0}
};

cheatseq_t cheat_choppers = { cheat_choppers_seq, 0 };
cheatseq_t cheat_clev = { cheat_clev_seq, 0 };
cheatseq_t cheat_mypos = { cheat_mypos_seq, 0 };

// ==========================================================================
//                        CHEAT SEQUENCE PACKAGE
// ==========================================================================

static uint8_t cheat_xlate_table[256];

void cht_Init()
{
	int i;

	for (i = 0; i < 256; i++)
		cheat_xlate_table[i] = SCRAMBLE(i);
}

//
// Called in st_stuff module, which handles the input.
// Returns a 1 if the cheat was successful, 0 if failed.
//
int cht_CheckCheat(cheatseq_t* cht, char key)
{
	int rc = 0;

	if (!cht->p)
		cht->p = cht->sequence;	// initialize if first time

	if (*cht->p == 0)
		*(cht->p++) = key;
	else if (cheat_xlate_table[(uint8_t)key] == *cht->p)
		cht->p++;
	else
		cht->p = cht->sequence;

	if (*cht->p == 1)
		cht->p++;
	else if (*cht->p == 0xff)	// end of sequence character
	{
		cht->p = cht->sequence;
		rc = 1;
	}

	return rc;
}

void cht_GetParam(cheatseq_t* cht, char* buffer)
{

	uint8_t* p, c;

	p = cht->sequence;
	while (*(p++) != 1);

	do
	{
		c = *p;
		*(buffer++) = c;
		*(p++) = 0;
	}
	while (c && *p != 0xff);

	if (*p == 0xff)
		*buffer = 0;

}

// added 2-2-98 for compatibility with dehacked
int idfa_armor = 200;
int idfa_armor_class = 2;
int idkfa_armor = 200;
int idkfa_armor_class = 2;
int god_health = 100;

static player_t* plyr;

bool_t cht_Responder(event_t* ev)
{
	int i;
	char* msg;

	if (ev->type == ev_keydown)
	{
		msg = NULL;

		if (gamestate != GS_LEVEL)
			return false;

		// added 17-5-98
		plyr = &players[consoleplayer[0]];
		// b. - enabled for more debug fun.
		// if (gameskill != sk_nightmare) {

		if (cht_CheckCheat(&cheat_amap, ev->data1))
			am_cheating = (am_cheating + 1) % 3;
		else
			// 'dqd' cheat for toggleable god mode
			if (cht_CheckCheat(&cheat_god, ev->data1))
			{
				plyr->cheats ^= CF_GODMODE;
				if (plyr->cheats & CF_GODMODE)
				{
					if (plyr->mo)
						plyr->mo->health = god_health;

					plyr->health = god_health;
					//plyr->message = STSTR_DQDON;
					msg = STSTR_DQDON;
				}
				else
					//plyr->message = STSTR_DQDOFF;
					msg = STSTR_DQDOFF;
			}
		// 'fa' cheat for killer fucking arsenal
			else if (cht_CheckCheat(&cheat_ammonokey, ev->data1))
			{
				plyr->armorpoints = idfa_armor;
				plyr->armortype = idfa_armor_class;

				for (i = 0; i < NUMWEAPONS; i++)
					plyr->weaponowned[i] = true;

				for (i = 0; i < NUMAMMO; i++)
					plyr->ammo[i] = plyr->maxammo[i];

				if (gamemode != commercial)
					plyr->weaponowned[wp_supershotgun] = false;

				if (gamemode == shareware)
				{
					plyr->weaponowned[wp_plasma] = false;
					plyr->weaponowned[wp_bfg] = false;
				}
				//plyr->message = STSTR_FAADDED;
				msg = STSTR_FAADDED;
			}
		// 'kfa' cheat for key full ammo
			else if (cht_CheckCheat(&cheat_ammo, ev->data1))
			{
				plyr->armorpoints = idkfa_armor;
				plyr->armortype = idkfa_armor_class;

				for (i = 0; i < NUMWEAPONS; i++)
					plyr->weaponowned[i] = true;

				for (i = 0; i < NUMAMMO; i++)
					plyr->ammo[i] = plyr->maxammo[i];

				if (gamemode != commercial)
					plyr->weaponowned[wp_supershotgun] = false;

				if (gamemode == shareware)
				{
					plyr->weaponowned[wp_plasma] = false;
					plyr->weaponowned[wp_bfg] = false;
				}

				plyr->cards = it_allkeys;

				//plyr->message = STSTR_KFAADDED;
				msg = STSTR_KFAADDED;
			}
		// 'mus' cheat for changing music
			else if (cht_CheckCheat(&cheat_mus, ev->data1))
			{
				char buf[3];
				int musnum;

				plyr->message = STSTR_MUS;
				cht_GetParam(&cheat_mus, buf);

				if (gamemode == commercial)
				{
					musnum = mus_runnin + (buf[0] - '0') * 10 + buf[1] - '0' - 1;

					if (((buf[0] - '0') * 10 + buf[1] - '0') > 35)
						//plyr->message = STSTR_NOMUS;
						msg = STSTR_NOMUS;
					else
						S_ChangeMusic(musnum, 1);
				}
				else
				{
					musnum = mus_e1m1 + (buf[0] - '1') * 9 + (buf[1] - '1');

					if (((buf[0] - '1') * 9 + buf[1] - '1') > 31)
						//plyr->message = STSTR_NOMUS;
						msg = STSTR_NOMUS;
					else
						S_ChangeMusic(musnum, 1);
				}
			}
		// 'cd' for changing cd track quickly
		//NOTE: the cheat uses the REAL track numbers, not remapped ones
			else if (cht_CheckCheat(&cheat_cd, ev->data1))
			{
				char buf[3];

				cht_GetParam(&cheat_cd, buf);

				plyr->message = "Changing cd track...\n";
				I_PlayCD((buf[0] - '0') * 10 + (buf[1] - '0'), true);
			}
		// Simplified, accepting both "noclip" and "idspispopd".
		// no clipping mode ch      eat
			else if (cht_CheckCheat(&cheat_noclip, ev->data1) || cht_CheckCheat(&cheat_commercial_noclip, ev->data1))
			{
				plyr->cheats ^= CF_NOCLIP;

				if (plyr->cheats & CF_NOCLIP)
					//plyr->message = STSTR_NCON;
					msg = STSTR_NCON;
				else
					//plyr->message = STSTR_NCOFF;
					msg = STSTR_NCOFF;
			}
		// 'behold?' power-up cheats
		for (i = 0; i < 6; i++)
		{
			if (cht_CheckCheat(&cheat_powerup[i], ev->data1))
			{
				if (!plyr->powers[i])
					P_GivePower(plyr, i);
				else if (i != pw_strength)
					plyr->powers[i] = 1;
				else
					plyr->powers[i] = 0;

				//plyr->message = STSTR_BEHOLDX;
				msg = STSTR_BEHOLDX;
			}
		}

		// 'behold' power-up menu
		if (cht_CheckCheat(&cheat_powerup[6], ev->data1))
		{
			//plyr->message = STSTR_BEHOLD;
			msg = STSTR_BEHOLD;
		}
		// 'choppers' invulnerability & chainsaw
		else if (cht_CheckCheat(&cheat_choppers, ev->data1))
		{
			plyr->weaponowned[wp_chainsaw] = true;
			plyr->powers[pw_invulnerability] = true;

			//plyr->message = STSTR_CHOPPERS;
			msg = STSTR_CHOPPERS;
		}
		// 'mypos' for player position
		else if (cht_CheckCheat(&cheat_mypos, ev->data1))
		{
			//plyr->message = buf;
			CONL_PrintF(va
			            ("ang=%i;x,y=(%i,%i)\n",
			             players[statusbarplayer].mo->angle / ANGLE_1, players[statusbarplayer].mo->x >> FRACBITS, players[statusbarplayer].mo->y >> FRACBITS));

		}
		else
			//added:28-02-98: new fly cheat using jump key
			if (cht_CheckCheat(&cheat_fly_around, ev->data1))
			{
				plyr->cheats ^= CF_FLYAROUND;
				if (plyr->cheats & CF_FLYAROUND)
					//plyr->message = "FLY MODE ON : USE JUMP KEY";
					msg = "FLY MODE ON : USE JUMP KEY\n";
				else
					//plyr->message = "FLY MODE OFF";
					msg = "FLY MODE OFF\n";
			}
		// 'clev' change-level cheat
		if (cht_CheckCheat(&cheat_clev, ev->data1))
		{
			char buf[3];
			int epsd;
			int map;

			cht_GetParam(&cheat_clev, buf);

			if (gamemode == commercial)
			{
				epsd = 0;
				map = (buf[0] - '0') * 10 + buf[1] - '0';
			}
			else
			{
				epsd = buf[0] - '0';
				map = buf[1] - '0';
				// added 3-1-98
				if (epsd < 1)
					return false;
			}

			// Catch invalid maps.
			//added:08-01-98:moved if (epsd<1)...  up
			if (map < 1)
				return false;

			// Ohmygod - this is not going to work.
			if ((gamemode == retail) && ((epsd > 4) || (map > 9)))
				return false;

			if ((gamemode == registered) && ((epsd > 3) || (map > 9)))
				return false;

			if ((gamemode == shareware) && ((epsd > 1) || (map > 9)))
				return false;

			if ((gamemode == commercial) && ((epsd > 1) || (map > 34)))
				return false;

			// So be it.
			//plyr->message = STSTR_CLEV;
			msg = STSTR_CLEV;
			G_DeferedInitNew(gameskill, G_BuildMapName(epsd, map), false);
		}
		// append a newline to the original doom messages
		if (msg)
			CONL_PrintF("%s\n", msg);
	}
	return false;
}

// command that can be typed at the console !

void Command_CheatNoClip_f(void)
{
	player_t* plyr;

	if (gamestate != GS_LEVEL)
	{
		CONL_PrintF("Cheats may only be used during a game!\n");
		return;
	}

	if (multiplayer)
		return;

	plyr = &players[consoleplayer[0]];

	plyr->cheats ^= CF_NOCLIP;

	if (plyr->cheats & CF_NOCLIP)
		CONL_PrintF(STSTR_NCON);
	else
		CONL_PrintF(STSTR_NCOFF);

}

void Command_CheatGod_f(void)
{
	player_t* plyr;

	if (gamestate != GS_LEVEL)
	{
		CONL_PrintF("Cheats may only be used during a game!\n");
		return;
	}

	if (multiplayer)
		return;

	plyr = &players[consoleplayer[0]];

	plyr->cheats ^= CF_GODMODE;
	if (plyr->cheats & CF_GODMODE)
	{
		if (plyr->mo)
			plyr->mo->health = god_health;

		plyr->health = god_health;
		CONL_PrintF("%s\n", STSTR_DQDON);
	}
	else
		CONL_PrintF("%s\n", STSTR_DQDOFF);
}

void Command_CheatGimme_f(void)
{
	char* s;
	int i, j, k;
	player_t* plyr;
	int all = 0;

	if (gamestate != GS_LEVEL)
	{
		CONL_PrintF("Cheats may only be used during a game!\n");
		return;
	}

	if (multiplayer)
		return;

	if (COM_Argc() < 2)
	{
		CONL_PrintF("gimme [all, health, ammo, armor, weapons, ...]\n");
		return;
	}

	plyr = &players[consoleplayer[0]];

	for (k = 0; k < (cv_splitscreen.value ? 2 : 1); k++)
	{
		if (k == 1)
		{
			plyr = &players[displayplayer[1]];
		}

		for (i = 1; i < COM_Argc(); i++)
		{
			s = COM_Argv(i);

			if (!strncmp(s, "all", 3))
				all = 1;

			if (all || !strncmp(s, "health", 6))
			{
				if (plyr->mo)
					plyr->mo->health = god_health;

				plyr->health = god_health;

				CONL_PrintF("got health\n");
			}
			if (all || !strncmp(s, "ammo", 4))
			{
				for (j = 0; j < NUMAMMO; j++)
					plyr->ammo[j] = plyr->maxammo[j];

				CONL_PrintF("got ammo\n");
			}
			if (all || !strncmp(s, "armor", 5))
			{
				plyr->armorpoints = idfa_armor;
				plyr->armortype = idfa_armor_class;

				CONL_PrintF("got armor\n");
			}
			if (all || !strncmp(s, "keys", 4))
			{
				plyr->cards = it_allkeys;

				CONL_PrintF("got keys\n");
			}
			if (all || !strncmp(s, "weapons", 7))
			{
				for (j = 0; j < NUMWEAPONS; j++)
					if ((gamemission == doom && j != wp_supershotgun) || (gamemission != doom))
						plyr->weaponowned[j] = true;

				for (j = 0; j < NUMAMMO; j++)
					plyr->ammo[j] = plyr->maxammo[j];

				CONL_PrintF("got weapons\n");
			}
			//
			// WEAPONS
			//
			if (all || !strncmp(s, "chainsaw", 8))
			{
				plyr->weaponowned[wp_chainsaw] = true;

				CONL_PrintF("got chainsaw\n");
			}
			if (all || !strncmp(s, "shotgun", 7))
			{
				plyr->weaponowned[wp_shotgun] = true;
				plyr->ammo[am_shell] = plyr->maxammo[am_shell];

				CONL_PrintF("got shotgun\n");
			}
			if (all || !strncmp(s, "supershotgun", 12))
			{
				if (gamemode == commercial)	// only in Doom2
				{
					plyr->weaponowned[wp_supershotgun] = true;
					plyr->ammo[am_shell] = plyr->maxammo[am_shell];

					CONL_PrintF("got super shotgun\n");
				}
			}
			if (all || !strncmp(s, "rocket", 6))
			{
				plyr->weaponowned[wp_missile] = true;
				plyr->ammo[am_misl] = plyr->maxammo[am_misl];

				CONL_PrintF("got rocket launcher\n");
			}
			if (all || !strncmp(s, "plasma", 6))
			{
				plyr->weaponowned[wp_plasma] = true;
				plyr->ammo[am_cell] = plyr->maxammo[am_cell];

				CONL_PrintF("got plasma\n");
			}
			if (all || !strncmp(s, "bfg", 3))
			{
				plyr->weaponowned[wp_bfg] = true;
				plyr->ammo[am_cell] = plyr->maxammo[am_cell];

				CONL_PrintF("got bfg\n");
			}
			if (all || !strncmp(s, "chaingun", 8))
			{
				plyr->weaponowned[wp_chaingun] = true;
				plyr->ammo[am_clip] = plyr->maxammo[am_clip];

				CONL_PrintF("got chaingun\n");
			}

			if (!strncmp(s, "level2weapons", 13))
			{
				plyr->weaponinfo = wpnlev2info;
				CONL_PrintF("got level 2 weapons\n");
			}
			else if (!strncmp(s, "level1weapons", 13))
			{
				plyr->weaponinfo = wpnlev1info;
				CONL_PrintF("got level 1 weapons\n");
			}
			//
			// SPECIAL ITEMS
			//
			if (all || !strncmp(s, "berserk", 7))
			{
				if (!plyr->powers[pw_strength])
					P_GivePower(plyr, pw_strength);
				CONL_PrintF("got berserk strength\n");
			}
			//22/08/99: added by Hurdler
			if (all || !strncmp(s, "map", 3))
			{
				am_cheating = 1;
				CONL_PrintF("got map\n");
			}
			//
			if (all || !strncmp(s, "fullmap", 7))
			{
				am_cheating = 2;
				CONL_PrintF("got map and things\n");
			}
		}
	}
}

//
// PTR_SummonTraverse
// Summons an object if possible
//
bool_t PTR_SummonTraverse(intercept_t* in)
{
	return false;
}

void Command_CheatSummon_f(void)
{
	char* s;
	int i, j, k;
	player_t* plyr;
	int distance;
	fixed_t x2;
	fixed_t y2;
	fixed_t cosangle;

	if (gamestate != GS_LEVEL)
	{
		CONL_PrintF("Cheats may only be used during a game!\n");
		return;
	}

	if (multiplayer)
		return;

	if (COM_Argc() < 2)
	{
		CONL_PrintF("summon [Eternity Class or MT_ Number] ...\n");
		return;
	}

	plyr = &players[consoleplayer[0]];

	s = COM_Argv(1);

	j = -1;

	if ((s[0] == '0') ||
	        (s[0] == '1') || (s[0] == '2') || (s[0] == '3') || (s[0] == '4') || (s[0] == '5') || (s[0] == '6') || (s[0] == '7') || (s[0] == '8') || (s[0] == '9'))
	{
		j = atoi(s);

		if (!((j >= 0) && (j < NUMMOBJTYPES)))
		{
			CONL_PrintF("Invalid MT_ Number, must be >= 0 and < %i.\n", NUMMOBJTYPES);
			return;
		}
	}
	else
	{
		for (k = 0; k < NUMMOBJTYPES; k++)
		{
			if (MT2ReMooDClass[k] == NULL)
				continue;

			if (strcasecmp(s, MT2ReMooDClass[k]) == 0)
			{
				j = k;
				break;
			}
		}

		if (j == -1)
		{
			for (k = 0; k < NUMMOBJTYPES; k++)
			{
				if (MT2MTString[k] == NULL)
					continue;

				if (strcasecmp(s, MT2MTString[k]) == 0)
				{
					j = k;
					break;
				}
			}

			CONL_PrintF("Invalid Object Name \"%s\".\n", s);
			return;
		}
	}

	// How far should this thing be?
	distance = (plyr->mo->info->radius * 2) + (mobjinfo[k].radius * 2);

	P_SpawnMobj(plyr->mo->x +
	            FixedMul(distance,
	                     finecosine[plyr->mo->angle >> ANGLETOFINESHIFT]),
	            plyr->mo->y + FixedMul(distance, finesine[plyr->mo->angle >> ANGLETOFINESHIFT]), plyr->mo->z, j);
}

void Command_CheatSummonFriend_f(void)
{
	if (multiplayer)
		return;
}

// heretic cheat

#endif
