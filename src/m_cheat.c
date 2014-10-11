// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION:
#if 0
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
// Copyright (C) 1993-1996 by id Software, Inc.
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
// DESCRIPTION:
//      Cheat sequence checking.

/***************
*** INCLUDES ***
***************/

















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

	plyr = &players[g_Splits[0].Console];

	plyr->cheats ^= CF_NOCLIP;

	//if (plyr->cheats & CF_NOCLIP)
	//	CONL_PrintF(STSTR_NCON);
	//else
	//	CONL_PrintF(STSTR_NCOFF);

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

	plyr = &players[g_Splits[0].Console];

	plyr->cheats ^= CF_GODMODE;
	if (plyr->cheats & CF_GODMODE)
	{
		if (plyr->mo)
			plyr->mo->health = god_health;

		plyr->health = god_health;
		//CONL_PrintF("%s\n", STSTR_DQDON);
	}
	//else
	//	CONL_PrintF("%s\n", STSTR_DQDOFF);
}

void Command_CheatGimme_f(void)
{
#if 0
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

	plyr = &players[g_Splits[0].Console];

	for (k = 0; k < (g_SplitScreen ? 2 : 1); k++)
	{
		if (k == 1)
		{
			plyr = &players[g_Splits[1].Display];
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
#endif
}

//
// PTR_SummonTraverse
// Summons an object if possible
//
bool_t PTR_SummonTraverse(intercept_t* in, void* a_Data)
{
	return false;
}

void Command_CheatSummon_f(void)
{
#if 0
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

	plyr = &players[g_Splits[0].Console];

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
#endif
}

void Command_CheatSummonFriend_f(void)
{
	if (multiplayer)
		return;
}

// heretic cheat

/***********************
*** NEW CHEAT SYSTEM ***
***********************/

/*** STRUCTURES ***/

/* M_SingleCheat_t -- A single cheat */
typedef struct M_SingleCheat_s
{
	const char* const Name;						// Cheat name
	void (*Command)(player_t* const a_Player, const uint32_t a_ArgC, const char** const a_ArgV);
} M_SingleCheat_t;

/*** GLOBALS ***/

uint32_t g_CheatFlags = 0;						// Global cheat flags

/*** LOCALS ***/

void MS_CHEAT_FreezeTime(player_t* const a_Player, const uint32_t a_ArgC, const char** const a_ArgV);
void MS_CHEAT_Give(player_t* const a_Player, const uint32_t a_ArgC, const char** const a_ArgV);
void MS_CHEAT_Summon(player_t* const a_Player, const uint32_t a_ArgC, const char** const a_ArgV);
void MS_CHEAT_SummonFriend(player_t* const a_Player, const uint32_t a_ArgC, const char** const a_ArgV);
void MS_CHEAT_SummonTeam(player_t* const a_Player, const uint32_t a_ArgC, const char** const a_ArgV);
void MS_CHEAT_God(player_t* const a_Player, const uint32_t a_ArgC, const char** const a_ArgV);
void MS_CHEAT_Morph(player_t* const a_Player, const uint32_t a_ArgC, const char** const a_ArgV);

// l_LocalCheats -- Locally obtained cheats
static M_SingleCheat_t l_LocalCheats[] =
{
	{"freezetime", MS_CHEAT_FreezeTime},
	{"give", MS_CHEAT_Give},
	{"summon", MS_CHEAT_Summon},
	{"summonfriend", MS_CHEAT_SummonFriend},
	{"summonteam", MS_CHEAT_SummonTeam},
	{"god", MS_CHEAT_God},
	{"morph", MS_CHEAT_Morph},
	
	{NULL},
};

/*** FUNCTIONS ***/

/* MS_CHEAT_FreezeTime() -- Freeze time! Cool! */
void MS_CHEAT_FreezeTime(player_t* const a_Player, const uint32_t a_ArgC, const char** const a_ArgV)
{
	/* Toggle Freeze Time */
	g_CheatFlags ^= MCF_FREEZETIME;
	
	/* Message */
	CONL_PrintF("Time is now %s.\n", (g_CheatFlags & MCF_FREEZETIME ? "Frozen" : "Flowing"));
}

/* MS_CHEAT_Give() -- Give something */
void MS_CHEAT_Give(player_t* const a_Player, const uint32_t a_ArgC, const char** const a_ArgV)
{
	PI_wepid_t Weapon;
	PI_ammoid_t Ammo;
	size_t CompLen, i;
	char* AstPtr;
	bool_t Wild;
	
	/* Check Arguments */
	if (!a_Player || a_ArgC < 1)
		return;
	
	/* Find asterisk? */
	Wild = false;
	AstPtr = strchr(a_ArgV[0], '*');
	
	if (!AstPtr)
		CompLen = strlen(a_ArgV[0]);
	else
	{
		Wild = true;
		CompLen = AstPtr - a_ArgV[0];
	}
	
	/* Give what? */
	// Weapon?
	for (i = 0; i < NUMWEAPONS; i++)
		if ((!Wild && strcasecmp(a_ArgV[0], a_Player->weaponinfo[i]->ClassName) == 0) || 
			(Wild && strncasecmp(a_ArgV[0], a_Player->weaponinfo[i]->ClassName, CompLen) == 0))
			// Make it owned
			a_Player->weaponowned[i] = true;
	
	// Ammo?
	for (i = 0; i < NUMAMMO; i++)
		if ((!Wild && strcasecmp(a_ArgV[0], ammoinfo[i]->ClassName) == 0) || 
			(Wild && strncasecmp(a_ArgV[0], ammoinfo[i]->ClassName, CompLen) == 0))
			// Give max ammo
			a_Player->ammo[i] = a_Player->maxammo[i];
}

/* MS_CHEAT_Summon() -- Spawn an object in front of the player */
void MS_CHEAT_Summon(player_t* const a_Player, const uint32_t a_ArgC, const char** const a_ArgV)
{
	fixed_t Distance, BaseDist;
	PI_mobjid_t Obj;
	mobj_t* Mo;
	size_t Count, i;
	bool_t Friend;
	
	/* Check Arguments */
	if (!a_Player || a_ArgC < 1)
		return;
	
	/* Get Count */
	if (a_ArgC > 1)
		Count = atoi(a_ArgV[1]);
	else
		Count = 1;
	
	/* Look it up by name */
	Obj = INFO_GetTypeByName(a_ArgV[0]);
	
	// Invalid?
	if (Obj == NUMMOBJTYPES)
		return;
		
	/* Spawn it away from the player */
	BaseDist = Distance = (a_Player->mo->info->radius * 2) + (mobjinfo[Obj]->radius * 2);

	// Spawn it
	for (i = 0; i < Count; i++)
	{
		Mo = P_SpawnMobj(
				a_Player->mo->x +
					FixedMul(Distance,
						finecosine[a_Player->mo->angle >> ANGLETOFINESHIFT]),
				a_Player->mo->y + FixedMul(Distance, finesine[a_Player->mo->angle >> ANGLETOFINESHIFT]),
				a_Player->mo->z,
				Obj
			);
		
		// Failed to spawn?
		if (!Mo)
			break;
		
		// If respawning more than 1 and the rest cannot be seen, respawn
		if (Count > 1 && !P_CheckSight(a_Player->mo, Mo))
		{
			P_RemoveMobj(Mo);
			continue;
		}
				
		// Modify angle
		Mo->angle = a_Player->mo->angle;
		
		// Add distance
		Distance += BaseDist;
	}
}

/* MS_CHEAT_SummonFriend() -- Spawn an object in front of the player */
void MS_CHEAT_SummonFriend(player_t* const a_Player, const uint32_t a_ArgC, const char** const a_ArgV)
{
	fixed_t Distance, BaseDist;
	PI_mobjid_t Obj;
	mobj_t* Mo;
	size_t Count, i;
	bool_t Friend;
	
	/* Check Arguments */
	if (!a_Player || a_ArgC < 1)
		return;
	
	/* Get Count */
	if (a_ArgC > 1)
		Count = atoi(a_ArgV[1]);
	else
		Count = 1;
	
	/* Look it up by name */
	Obj = INFO_GetTypeByName(a_ArgV[0]);
	
	// Invalid?
	if (Obj == NUMMOBJTYPES)
		return;
		
	/* Spawn it away from the player */
	BaseDist = Distance = (a_Player->mo->info->radius * 2) + (mobjinfo[Obj]->radius * 2);

	// Spawn it
	for (i = 0; i < Count; i++)
	{
		Mo = P_SpawnMobj(
				a_Player->mo->x +
					FixedMul(Distance,
						finecosine[a_Player->mo->angle >> ANGLETOFINESHIFT]),
				a_Player->mo->y + FixedMul(Distance, finesine[a_Player->mo->angle >> ANGLETOFINESHIFT]),
				a_Player->mo->z,
				Obj
			);
		
		// Failed to spawn?
		if (!Mo)
			break;
		
		// If respawning more than 1 and the rest cannot be seen, respawn
		if (Count > 1 && !P_CheckSight(a_Player->mo, Mo))
		{
			P_RemoveMobj(Mo);
			continue;
		}
				
		// Modify angle
		Mo->angle = a_Player->mo->angle;
		
		// Make friendly
		Mo->flags2 |= MF2_FRIENDLY;
		
		// Add distance
		Distance += BaseDist;
	}
}


/* MS_CHEAT_SummonTeam() -- Spawn an object in front of the player */
void MS_CHEAT_SummonTeam(player_t* const a_Player, const uint32_t a_ArgC, const char** const a_ArgV)
{
	fixed_t Distance, BaseDist;
	PI_mobjid_t Obj;
	mobj_t* Mo;
	int32_t Count, i;
	int32_t TeamNum;
	bool_t Friend;
	
	/* Check Arguments */
	if (!a_Player || a_ArgC < 2)
		return;
	
	/* Get Team number */
	TeamNum = atoi(a_ArgV[0]);
	
	// bad?
	if (TeamNum < 0 || TeamNum > MAXSKINCOLORS)
		return;
	
	/* Get Count */
	if (a_ArgC > 2)
		Count = atoi(a_ArgV[2]);
	else
		Count = 1;
	
	// Cap?
	if (Count < 1)
		Count = 1;
	else if (Count > 256)
		Count = 256;
	
	/* Look it up by name */
	Obj = INFO_GetTypeByName(a_ArgV[1]);
	
	// Invalid?
	if (Obj == NUMMOBJTYPES)
		return;
		
	/* Spawn it away from the player */
	BaseDist = Distance = (a_Player->mo->info->radius * 2) + (mobjinfo[Obj]->radius * 2);

	// Spawn it
	for (i = 0; i < Count; i++)
	{
		Mo = P_SpawnMobj(
				a_Player->mo->x +
					FixedMul(Distance,
						finecosine[a_Player->mo->angle >> ANGLETOFINESHIFT]),
				a_Player->mo->y + FixedMul(Distance, finesine[a_Player->mo->angle >> ANGLETOFINESHIFT]),
				a_Player->mo->z,
				Obj
			);
		
		// Failed to spawn?
		if (!Mo)
			break;
		
		// If respawning more than 1 and the rest cannot be seen, respawn
		if (Count > 1 && !P_CheckSight(a_Player->mo, Mo))
		{
			P_RemoveMobj(Mo);
			continue;
		}
				
		// Modify angle
		Mo->angle = a_Player->mo->angle;
		
		// Make friendly
		Mo->SkinTeamColor = TeamNum;
		
		// Add distance
		Distance += BaseDist;
	}
}

/* MS_CHEAT_God() -- Make invincible */
void MS_CHEAT_God(player_t* const a_Player, const uint32_t a_ArgC, const char** const a_ArgV)
{
	/* Check */
	if (!a_Player)
		return;
	
	/* Toggle God Mode */
	a_Player->cheats ^= CF_GODMODE;
}

/* MS_CHEAT_Morph() -- Morph player into another object */
void MS_CHEAT_Morph(player_t* const a_Player, const uint32_t a_ArgC, const char** const a_ArgV)
{
	/* Check */
	if (!a_Player || a_ArgC < 1)
		return;
	
	/* Change base class */
	P_MorphObjectClass(a_Player->mo, INFO_GetTypeByName(a_ArgV[0]));
}

/* MS_MultiCheatCommand() -- Multi-cheat command */
static int MS_MultiCheatCommand(const uint32_t a_ArgC, const char** const a_ArgV)
{
	size_t i;
	
	/* Check */
	if (a_ArgC < 2)
		return 1;
	
	/* See if cheating is permitted in this game */
	
	/* Loop */
	for (i = 0; l_LocalCheats[i].Name; i++)
		// Compare Name
		if (strcasecmp(a_ArgV[1], l_LocalCheats[i].Name) == 0)
			// Call cheat handler
			l_LocalCheats[i].Command(&players[g_Splits[0].Console], a_ArgC - 2, a_ArgV + 2);
	
	/* Return */
	return 0;
}

/* M_CheatInit() -- Initialize Cheating */
void M_CheatInit(void)
{
	/* Add multi-cheat command */
	CONL_AddCommand("cheat", MS_MultiCheatCommand);
}

#endif
#endif

