// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.sourceforge.net/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           Demyx                  (demyx@endgameftw.com)
//                    Dragan                 (poliee13@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2009 The ReMooD Team..
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION:
//      DOOM selection menu, options, episode etc.
//      Sliders and icons. Kinda widget stuff.
//

#ifndef _WIN32
#include <unistd.h>
#endif
#include <fcntl.h>

#include "am_map.h"
#include "doomdef.h"
#include "dstrings.h"
#include "d_main.h"
#include "console.h"
#include "r_local.h"
#include "hu_stuff.h"
#include "g_game.h"
#include "g_input.h"
#include "m_argv.h"
#include "sounds.h"
#include "s_sound.h"
#include "i_system.h"
#include "m_menu.h"
#include "v_video.h"
#include "i_video.h"
#include "keys.h"
#include "z_zone.h"
#include "w_wad.h"
#include "p_local.h"
#include "p_fab.h"
#include "t_script.h"
#include "d_net.h"
#include "p_inter.h"
#include "dstrings.h"
#include "v_video.h"
#include "m_random.h"

void M_SetupGameOptionsNow(menu_t* Orig);
void M_WarpToMap(menu_t* Orig);

#define GAMESTNMOV (gamemode == heretic ? sfx_hstnmov : sfx_stnmov)
#define GAMESWITCHN (gamemode == heretic ? sfx_switch : sfx_swtchn)
#define GAMESWITCHX (gamemode == heretic ? sfx_switch : sfx_swtchx)
#define GAMEPSTOP (gamemode == heretic ? sfx_hpstop : sfx_pstop)
#define GAMEPISTOL (gamemode == heretic ? sfx_keyup : sfx_pstop)

// =============================================================================
//                                 Video Options
// =============================================================================

int vidMenu_itemOn = 0;

void M_DrawVideoOptions(void)
{
	int cursorx;
	int cursory;
	int i;
	int x;
	int y;
	char* text;
	int len;
	int NumModes = VID_NumModes();
	
	// Draw the initial menu
	M_DrawGenericMenu();
	
	// Where is our cursor?
	if (itemOn == 0)
	{
		cursorx = currentMenu->x + ((currentMenu->width / 3) * (vidMenu_itemOn % 3)) - 10;
		cursory = currentMenu->y + (STRINGHEIGHT << 1) + ((vidMenu_itemOn / 3) * STRINGHEIGHT);
	}
	
	// Draw video modes and stuff
	for (i = 0; i < NumModes; i++)
	{
		text = VID_GetModeName(i);
		x = currentMenu->x + ((currentMenu->width / 3) * (i % 3));
		y = currentMenu->y + (STRINGHEIGHT << 1) + ((i / 3) * STRINGHEIGHT);
		
		if (text)
			V_DrawString(x, y, (vid.modenum == i ? V_WHITEMAP : 0), text);
	}
	
	// Draw Cursor
	if (itemOn == 0)
	{
		if (skullAnimCounter < 4 * NEWTICRATERATIO)	//blink cursor
			V_DrawCharacter(cursorx, cursory, '*' | 0x80);
			
		if (!(skullAnimCounter < 4 * NEWTICRATERATIO))	//blink cursor
			V_DrawCharacter(currentMenu->x - 10, currentMenu->y + (itemOn * STRINGHEIGHT), '*' | 0x80);
	}
}

void M_HandleVideoKey(int choice)
{
	switch (choice)
	{
		// Handle Keys
		case KEY_UPARROW:
			vidMenu_itemOn -= 3;
			S_StartSound(NULL, sfx_pstop);
			break;
		case KEY_DOWNARROW:
			vidMenu_itemOn += 3;
			S_StartSound(NULL, sfx_pstop);
			break;
		case KEY_RIGHTARROW:
			vidMenu_itemOn++;
			S_StartSound(NULL, sfx_pstop);
			break;
		case KEY_LEFTARROW:
			vidMenu_itemOn--;
			S_StartSound(NULL, sfx_pstop);
			break;
			
		case KEY_ENTER:
			if (!setmodeneeded)
				setmodeneeded = vidMenu_itemOn + 1;
			break;
			
		case 'd':
		case 'D':
			SCR_SetDefaultMode();
			break;			
			
		// Don't ask me (GhostlyDeath)
		case KEY_BACKSPACE:
			S_StartSound(NULL, GAMESWITCHN);
		case KEY_ESCAPE:
			if (choice == KEY_ESCAPE)
				S_StartSound(NULL, GAMESWITCHX);

			if (currentMenu->prevMenu)
				M_SetupNextMenu(currentMenu->prevMenu);
			else
				M_ClearMenus(true);
			break;
		
		default:
			break;
	}
	
	// Handle video mode change
	if (vidMenu_itemOn < 0)
	{
		itemOn = 0;
		vidMenu_itemOn = VID_NumModes() - 1;
	}
	else if (vidMenu_itemOn > (VID_NumModes() - 1))
	{
		itemOn = 0;
		vidMenu_itemOn = 0;
	}
}

void M_StartVideoOptions(int choice)
{
	
	currentMenu->lastOn = itemOn;
	M_SetupNextMenu(&VideoDef);
}

// =============================================================================
//                                 Quit Game
// =============================================================================

int quitsounds[16] = {
	sfx_pldeth,
	sfx_dmpain,
	sfx_popain,
	sfx_slop,
	sfx_telept,
	sfx_posit1,
	sfx_posit3,
	sfx_sgtatk,
	sfx_vilact,
	sfx_getpow,
	sfx_boscub,
	sfx_slop,
	sfx_skeswg,
	sfx_kntdth,
	sfx_bspact,
	sfx_sgtatk
};

void M_QuitResponse(int ch)
{
	tic_t time;
	if (ch != 'y')
		return;
	
	if (gamemode != heretic)
	{
		if (gamemission != commercial)
			S_StartSound(NULL, quitsounds[(gametic >> 2) % 8]);
		else
			S_StartSound(NULL, quitsounds[(gametic >> 2) % 16]);

		if (!nosound)
		{
			time = I_GetTime() + TICRATE * 2;
			while (time > I_GetTime());
		}
	}

	I_Quit();
}

/* M_QuitDoom() -- Quit ReMooD */
void M_QuitDOOM(int choice)
{
	// We pick index 0 which is language sensitive,
	//  or one at random, between 1 and maximum number.
	static char s[200];
	sprintf(s, text[DOSY_NUM], text[QUITMSG_NUM + (gametic % NUM_QUITMESSAGES)]);
	M_StartMessage(s, M_QuitResponse, MM_YESNO);
}

void M_EndGameResponse(int ch)
{
	if (ch != 'y')
		return;

	currentMenu->lastOn = itemOn;
	M_ClearMenus(true);
	COM_BufAddText("exitgame\n");
}

void M_EndGame(int choice)
{
	if (demorecording || demoplayback)
		return;
	
	if ((gamestate >= GS_LEVEL) && (gamestate <= GS_FINALE))
		M_StartMessage("Would you like to end the current game?\npress y or n", M_EndGameResponse, MM_YESNO);
}

// =============================================================================
//                               CREATE GAME MENU
// =============================================================================

void M_ClassicGameOptions(int choice)
{
	NewGameOptionsDef.prevMenu = &NewGameClassicDef;
	currentMenu->lastOn = itemOn;
	M_SetupNextMenu(&NewGameOptionsDef);
}

void M_LocalGameOptions(int choice)
{
	NewGameOptionsDef.prevMenu = &CreateLocalGameDef;
	currentMenu->lastOn = itemOn;
	M_SetupNextMenu(&NewGameOptionsDef);
}


CV_PossibleValue_t NGSkill[] =
{
	{1, "I'm too young to die"},
	{2, "Hey, not too rough"},
	{3, "Hurt me plenty"},
	{4, "Ultra-Violence"},
	{5, "Nightmare!"},
	{6, "Random"},
	
	{0, NULL},
};

CV_PossibleValue_t NGSkillHeretic[] =
{
	{1, "Thou needeth a wet-nurse"},
	{2, "Yellowbellies-r-us"},
	{3, "Bringest them oneth"},
	{4, "Thou art a smite-meister"},
	{5, "Black plague possesses thee"},
	{6, "Random"},
	
	{0, NULL},
};

CV_PossibleValue_t NGOptions[] =
{
	{0, "Classic Doom"},	// As close as EXE as possible
	{1, "Legacy"},			// Like Legacy's Defaults (kinda)
	{2, "ReMooD"},			// ReMooD's Defaults
	{3, "Custom"},			// Custom (i.e. don't touch them)
	
	{0, NULL},
};

void M_NGOptionChange(void);

CV_PossibleValue_t NGSplitScreenValue[] =
{
	{1, "Two"},
	{2, "Three"},
	{3, "Four"},
	
	{0, NULL},
};

CV_PossibleValue_t NGSplitScreenValue2[] =
{
	{0, "One"},
	{1, "Two"},
	{2, "Three"},
	{3, "Four"},
	
	{0, NULL},
};

#define __ASDF(x) #x
#define _ASDF(x) __ASDF(x)

CV_PossibleValue_t NGMaxPlayers[] =
{
	{0, "MIN"},
	{MAXPLAYERS, "MAX"},
	
	{0, NULL}
};

consvar_t cv_ng_map = {"ng_map", "0", CV_SAVE, CV_YesNo};
consvar_t cv_ng_skill = {"ng_skill", "3", CV_SAVE, NGSkill};
consvar_t cv_ng_options = {"ng_options", "2", CV_SAVE | CV_CALL, NGOptions, M_NGOptionChange};
consvar_t cv_ng_splitscreen = {"ng_splitscreen", "1", CV_SAVE, NGSplitScreenValue};
consvar_t cv_ng_splitscreen2 = {"ng_splitscreen2", "1", CV_SAVE, NGSplitScreenValue2};
consvar_t cv_ng_maxplayers = {"ng_maxplayers", _ASDF(MAXPLAYERS), CV_SAVE, NGMaxPlayers};
consvar_t cv_ng_maxclients = {"ng_maxclients", _ASDF(MAXPLAYERS), CV_SAVE, NGMaxPlayers};
consvar_t cv_ng_deathmatch = {"ng_deathmatch", "0", CV_SAVE, deathmatch_cons_t, NULL};
consvar_t cv_ng_teamplay = {"ng_teamplay", "0", CV_SAVE, teamplay_cons_t, NULL};
consvar_t cv_ng_teamdamage = {"ng_teamdamage", "0", CV_SAVE, CV_OnOff};
consvar_t cv_ng_fraglimit = {"ng_fraglimit", "0", CV_SAVE, fraglimit_cons_t, NULL};
consvar_t cv_ng_timelimit = {"ng_timelimit", "0", CV_SAVE, CV_Unsigned, NULL};
consvar_t cv_ng_allowexitlevel = { "ng_allowexitlevel", "1", CV_SAVE, CV_YesNo, NULL };
consvar_t cv_ng_allowjump = { "ng_allowjump", "1", CV_SAVE, CV_YesNo };
consvar_t cv_ng_allowautoaim = { "ng_allowautoaim", "1", CV_SAVE, CV_YesNo };
consvar_t cv_ng_forceautoaim = { "ng_forceautoaim", "1", CV_SAVE, CV_YesNo };
consvar_t cv_ng_allowrocketjump = { "ng_allowrocketjump", "0", CV_SAVE, CV_YesNo };
consvar_t cv_ng_classicrocketblast = { "ng_classicrocketblast", "0", CV_SAVE, CV_YesNo };
consvar_t cv_ng_allowturbo = { "ng_allowturbo", "0", CV_SAVE, CV_YesNo, NULL };
consvar_t cv_ng_itemrespawntime = { "ng_respawnitemtime", "30", CV_SAVE, CV_Unsigned };
consvar_t cv_ng_itemrespawn = { "ng_respawnitem", "0", CV_SAVE, CV_OnOff };
consvar_t cv_ng_spawnmonsters = { "ng_spawnmonsters", "1", CV_SAVE, CV_YesNo };
consvar_t cv_ng_respawnmonsters = { "ng_respawnmonsters", "0", CV_SAVE, CV_OnOff };
consvar_t cv_ng_respawnmonsterstime = { "ng_respawnmonsterstime", "12", CV_SAVE, CV_Unsigned };
consvar_t cv_ng_solidcorpse = { "ng_solidcorpse", "0", CV_SAVE, CV_OnOff };
consvar_t cv_ng_fastmonsters = { "ng_fastmonsters", "0", CV_SAVE, CV_OnOff, NULL};
consvar_t cv_ng_predictingmonsters = { "ng_predictingmonsters", "0", CV_SAVE, CV_OnOff };
consvar_t cv_ng_classicmonsterlogic = { "ng_classicmonsterlogic", "0", CV_SAVE, CV_YesNo };
consvar_t cv_ng_gravity = { "ng_gravity", "1", CV_SAVE | CV_FLOAT};
consvar_t cv_ng_classicblood = {"ng_classicblood", "0", CV_SAVE, CV_YesNo, NULL};
consvar_t cv_ng_classicmeleerange = { "ng_classicmeleerange", "0", CV_SAVE, CV_YesNo };
consvar_t cv_ng_fragsweaponfalling = { "ng_fragsweaponfalling", "0", CV_SAVE, CV_YesNo };
consvar_t cv_ng_bloodtime = { "ng_bloodtime", "20", CV_SAVE, bloodtime_cons_t,};
consvar_t cv_ng_infiniteammo = { "ng_infiniteammo", "0", CV_SAVE, CV_YesNo };

void M_NGOptionChange(void)
{
	if (cv_ng_options.value == 3)
	{
		NewGameClassicDef.menuitems[5].status &= ~IT_DISABLED2;
		CreateLocalGameDef.menuitems[9].status &= ~IT_DISABLED2;
	}
	else
	{
		NewGameClassicDef.menuitems[5].status |= IT_DISABLED2;
		CreateLocalGameDef.menuitems[9].status |= IT_DISABLED2;
	}
}

CV_PossibleValue_t* DynMaps = NULL;
int lastgamemode = -1, lastgamemission = -1;

int NumMaps[numgamemodes] =
{
	9,	// shareware
	27,	// registered
	32,	// commercial
	36,	// retail
	1,	// indetermined
	5,	// chexquest1
	45,	// heretic
};

int MissionTXTStart[numgamemissions] =
{
	HUSTR_E1M1_NUM, // doom
	HUSTR_1_NUM,	//doom2
	THUSTR_1_NUM,	// pack_tnt
	PHUSTR_1_NUM,	// pack_plut
	HUSTR_E1M1_NUM,	// pack_chex -- TODO: Use Chex Quest level names
	HERETIC_E1M1_NUM,	// pack_hereticsw
	HERETIC_E1M1_NUM,	// pack_heretic
	HERETIC_E1M1_NUM,	// pack_heretic13
};

static void M_GenerateLevelPossible(void)
{
	CV_PossibleValue_t* Rover;
	int j;
	char* bx;
	int NewCount;
	
	// Clear old list if it exists
	if (DynMaps)
	{
		Rover = DynMaps;
		
		for (;;)
		{
			if (Rover->strvalue)
			{
				Z_Free(Rover->strvalue);
				Rover->strvalue = NULL;
				
				Rover++;
			}
			else
				break;
		}
		
		Z_Free(DynMaps);
		DynMaps = NULL;
	}
	
	// Heretic Hack
	if (gamemode == heretic)
	{
		if (gamemission == pack_hereticsw)
			NumMaps[gamemode] = 9;
		else if (gamemission == pack_heretic)
			NumMaps[gamemode] = 9 * 3;
		else
			NumMaps[gamemode] = (9 * 5) + 3;	// includes secret levels :}
	}
	
	// Create map list
	NewCount = (NumMaps[gamemode] + 2);
	
	// Random Map (or Episode 1, 2, 3, 4)
	NewCount++;		// Random Map (All Maps)
	if (gamemode != shareware && gamemode != chexquest1 && gamemode != commercial)
	{
		if (gamemode == registered || gamemode == retail ||
			(gamemode == heretic && (gamemission == pack_heretic || gamemission == pack_heretic13)))
			NewCount += 2;	// TSOH + I
		if (gamemode == retail || (gamemode == heretic && gamemission == pack_heretic13))
			NewCount++;		// TFC
		if (gamemode == heretic && gamemission == pack_heretic13)
			NewCount += 2;
	}
	
	DynMaps = Z_Malloc(NewCount * sizeof(CV_PossibleValue_t), PU_STATIC, 0);
	
	// always Random
	DynMaps[NumMaps[gamemode]].value = 90;
	DynMaps[NumMaps[gamemode]].strvalue = "Random";
	
	// Episode Based
	if (gamemode != shareware && gamemode != chexquest1 && gamemode != commercial)
	{
		if (gamemode == registered || gamemode == retail ||
			(gamemode == heretic && (gamemission == pack_heretic || gamemission == pack_heretic13)))
		{
			DynMaps[NumMaps[gamemode] + 1].value = 91;
			DynMaps[NumMaps[gamemode] + 1].strvalue = "Random (Episode 1)";
			DynMaps[NumMaps[gamemode] + 2].value = 92;
			DynMaps[NumMaps[gamemode] + 2].strvalue = "Random (Episode 2)";
			DynMaps[NumMaps[gamemode] + 3].value = 93;
			DynMaps[NumMaps[gamemode] + 3].strvalue = "Random (Episode 3)";
		}
		
		if (gamemode == retail || (gamemode == heretic && gamemission == pack_heretic13))
		{
			DynMaps[NumMaps[gamemode] + 4].value = 94;
			DynMaps[NumMaps[gamemode] + 4].strvalue = "Random (Episode 4)";
		}
		
		if (gamemode == heretic && gamemission == pack_heretic13)
		{
			DynMaps[NumMaps[gamemode] + 5].value = 95;
			DynMaps[NumMaps[gamemode] + 5].strvalue = "Random (Episode 5)";
			DynMaps[NumMaps[gamemode] + 6].value = 96;
			DynMaps[NumMaps[gamemode] + 6].strvalue = "Random (Episode 6)";
		}
	}
	
	switch (gamemode)
	{
		case chexquest1:	/* DOOM */
		case shareware:
		case registered:
		case retail:
			for (j = 0; j < NumMaps[gamemode]; j++)
			{
				DynMaps[j].value = (((j / 9) + 1) * 10) + ((j % 9) + 1);
				bx = text[MissionTXTStart[gamemission]+j];
				while (*bx && *bx != ':')
					bx++;
				if (*bx == ':')
				{
					bx++;
					while (*bx != ' ')
						bx++;
					bx++;
				}
				else
					bx = text[MissionTXTStart[gamemission]+j];
				DynMaps[j].strvalue = Z_Malloc(8 + strlen(bx) + 1, PU_STATIC, 0);
				sprintf(DynMaps[j].strvalue, "E%iM%i - %s", (j / 9) + 1, (j % 9) + 1, bx);
			}
			break;
			
		case heretic:		/* Heretic */
			for (j = 0; j < NumMaps[gamemode]; j++)
			{
				DynMaps[j].value = (((j / 9) + 1) * 10) + ((j % 9) + 1);
				bx = text[HERETIC_E1M1_NUM+j];
				while (*bx && *bx != ':')
					bx++;
				if (*bx == ':')
				{
					bx++;
					while (*bx != ' ')
						bx++;
					bx++;
				}
				else
					bx = text[HERETIC_E1M1_NUM+j];
				DynMaps[j].strvalue = Z_Malloc(8 + strlen(bx) + 1, PU_STATIC, 0);
				sprintf(DynMaps[j].strvalue, "E%iM%i - %s", (j / 9) + 1, (j % 9) + 1, bx);
			}
			break;
		
		case commercial:	/* DOOM 2 */
			for (j = 0; j < NumMaps[gamemode]; j++)
			{
				DynMaps[j].value = j + 1;
				bx = text[MissionTXTStart[gamemission]+j];
				while (*bx && *bx != ':')
					bx++;
				if (*bx == ':')
				{
					bx++;
					while (*bx != ' ')
						bx++;
					bx++;
				}
				else
					bx = text[MissionTXTStart[gamemission]+j];
				DynMaps[j].strvalue = Z_Malloc(9 + strlen(bx) + 1, PU_STATIC, 0);
				sprintf(DynMaps[j].strvalue, "MAP%02i - %s", j + 1, bx);
			}
			break;
	}
	
	DynMaps[NewCount - 1].value = 0;
	DynMaps[NewCount - 1].strvalue = NULL;
	
	// Set Last Modes
	lastgamemode = gamemode;
	lastgamemission = gamemission;
	
	// For Security
	cv_ng_map.PossibleValue = DynMaps;
	CV_Set(&cv_ng_map, DynMaps[0].strvalue);
}

void M_DoNewGameClassicClassic(int choice)
{
	currentMenu->lastOn = itemOn;
	
	// Heretic?
	if (gamemode == heretic)
		cv_ng_skill.PossibleValue = NGSkillHeretic;
	else
		cv_ng_skill.PossibleValue = NGSkill;
		
	// Skill?
	if (gamemode == heretic)
	{
		NewGameCCSkillDef.menuitems[0].WItemTextPtr = PTROFUNICODESTRING(MENU_CLASSICGAME_HERETICSKILLA);
		NewGameCCSkillDef.menuitems[1].WItemTextPtr = PTROFUNICODESTRING(MENU_CLASSICGAME_HERETICSKILLB);
		NewGameCCSkillDef.menuitems[2].WItemTextPtr = PTROFUNICODESTRING(MENU_CLASSICGAME_HERETICSKILLC);
		NewGameCCSkillDef.menuitems[3].WItemTextPtr = PTROFUNICODESTRING(MENU_CLASSICGAME_HERETICSKILLD);
		NewGameCCSkillDef.menuitems[4].WItemTextPtr = PTROFUNICODESTRING(MENU_CLASSICGAME_HERETICSKILLE);
	}
	else
	{
		NewGameCCSkillDef.menuitems[0].WItemTextPtr = PTROFUNICODESTRING(MENU_CLASSICGAME_DOOMSKILLA);
		NewGameCCSkillDef.menuitems[1].WItemTextPtr = PTROFUNICODESTRING(MENU_CLASSICGAME_DOOMSKILLB);
		NewGameCCSkillDef.menuitems[2].WItemTextPtr = PTROFUNICODESTRING(MENU_CLASSICGAME_DOOMSKILLC);
		NewGameCCSkillDef.menuitems[3].WItemTextPtr = PTROFUNICODESTRING(MENU_CLASSICGAME_DOOMSKILLD);
		NewGameCCSkillDef.menuitems[4].WItemTextPtr = PTROFUNICODESTRING(MENU_CLASSICGAME_DOOMSKILLE);
	}
	
	// Episode?
	if (gamemode == commercial)
	{
		NewGameCCSkillDef.prevMenu = &NewGameDef;
		M_SetupNextMenu(&NewGameCCSkillDef);
	}
	else
	{
		NewGameCCEpiDef.prevMenu = &NewGameDef;
		NewGameCCSkillDef.prevMenu = &NewGameCCEpiDef;
		
		if (gamemode == heretic)
		{
			NewGameCCEpiDef.menuitems[0].WItemTextPtr = PTROFUNICODESTRING(MENU_CLASSICGAME_HERETICEPISODEA);
			NewGameCCEpiDef.menuitems[1].WItemTextPtr = PTROFUNICODESTRING(MENU_CLASSICGAME_HERETICEPISODEB);
			NewGameCCEpiDef.menuitems[2].WItemTextPtr = PTROFUNICODESTRING(MENU_CLASSICGAME_HERETICEPISODEC);
			NewGameCCEpiDef.menuitems[3].WItemTextPtr = PTROFUNICODESTRING(MENU_CLASSICGAME_HERETICEPISODED);
			NewGameCCEpiDef.menuitems[4].WItemTextPtr = PTROFUNICODESTRING(MENU_CLASSICGAME_HERETICEPISODEE);
			NewGameCCEpiDef.menuitems[5].WItemTextPtr = PTROFUNICODESTRING(MENU_CLASSICGAME_HERETICEPISODEF);
			
			if (gamemission == pack_hereticsw)
			{
				NewGameCCEpiDef.menuitems[1].status |= IT_DISABLED2;
				NewGameCCEpiDef.menuitems[2].status |= IT_DISABLED2;
				NewGameCCEpiDef.menuitems[3].status |= IT_DISABLED2;
				NewGameCCEpiDef.menuitems[4].status |= IT_DISABLED2;
				NewGameCCEpiDef.menuitems[5].status |= IT_DISABLED2;
			}
			else if (gamemission == pack_heretic)
			{
				NewGameCCEpiDef.menuitems[1].status &= ~IT_DISABLED2;
				NewGameCCEpiDef.menuitems[2].status &= ~IT_DISABLED2;
				NewGameCCEpiDef.menuitems[3].status |= IT_DISABLED2;
				NewGameCCEpiDef.menuitems[4].status |= IT_DISABLED2;
				NewGameCCEpiDef.menuitems[5].status |= IT_DISABLED2;
			}
			else
			{
				NewGameCCEpiDef.menuitems[1].status &= ~IT_DISABLED2;
				NewGameCCEpiDef.menuitems[2].status &= ~IT_DISABLED2;
				NewGameCCEpiDef.menuitems[3].status &= ~IT_DISABLED2;
				NewGameCCEpiDef.menuitems[4].status &= ~IT_DISABLED2;
				NewGameCCEpiDef.menuitems[5].status &= ~IT_DISABLED2;
			}
		}
		else
		{
			NewGameCCEpiDef.menuitems[0].WItemTextPtr = PTROFUNICODESTRING(MENU_CLASSICGAME_DOOMEPISODEA);
			NewGameCCEpiDef.menuitems[1].WItemTextPtr = PTROFUNICODESTRING(MENU_CLASSICGAME_DOOMEPISODEB);
			NewGameCCEpiDef.menuitems[2].WItemTextPtr = PTROFUNICODESTRING(MENU_CLASSICGAME_DOOMEPISODEC);
			NewGameCCEpiDef.menuitems[3].WItemTextPtr = PTROFUNICODESTRING(MENU_CLASSICGAME_DOOMEPISODED);
			NewGameCCEpiDef.menuitems[4].WItemTextPtr = PTROFUNICODESTRING(MENU_NULLSPACE);
			NewGameCCEpiDef.menuitems[5].WItemTextPtr = PTROFUNICODESTRING(MENU_NULLSPACE);
			
			NewGameCCEpiDef.menuitems[4].status |= IT_DISABLED2;
			NewGameCCEpiDef.menuitems[5].status |= IT_DISABLED2;
		
			if (gamemode != retail)
				NewGameCCEpiDef.menuitems[3].status |= IT_DISABLED2;
			else
				NewGameCCEpiDef.menuitems[3].status &= ~IT_DISABLED2;
			
			if (gamemode == shareware)
			{
				NewGameCCEpiDef.menuitems[1].status |= IT_DISABLED2;
				NewGameCCEpiDef.menuitems[2].status |= IT_DISABLED2;
			}
			else
			{
				NewGameCCEpiDef.menuitems[1].status &= ~IT_DISABLED2;
				NewGameCCEpiDef.menuitems[2].status &= ~IT_DISABLED2;
			}
		}
		
		M_SetupNextMenu(&NewGameCCEpiDef);
	}
}

int ccskill;
int ccepisode;
int epi;

void M_VerifyNightmare(int ch);

void M_SelectSkill(int choice)
{
	/* Demyx -- For nightmare verification. */
	if (choice == sk_nightmare)
    {
        M_StartMessage(NIGHTMARE,M_VerifyNightmare,MM_YESNO);
        return;
    }

	ccskill = choice;

	currentMenu->lastOn = itemOn;
	
	M_SetupGameOptionsNow(&NewGameCCSkillDef);
	M_WarpToMap(&NewGameCCSkillDef);
}

void M_SelectEpisode(int choice)
{
	ccepisode = choice;
	epi = choice;
	
	currentMenu->lastOn = itemOn;
	M_SetupNextMenu(&NewGameCCSkillDef);
}

void M_VerifyNightmare(int ch)
{
	if(ch != 'y')
		return;

	G_DeferedInitNew (sk_nightmare, G_BuildMapName(epi+1,1),0);

	M_ClearMenus(true);	//Whoops forgot about this one.
}

void M_DoNewGameClassic(int choice)
{
	if (!DynMaps || lastgamemode != gamemode || lastgamemission != gamemission)
		M_GenerateLevelPossible();
	
	M_NGOptionChange();
		
	currentMenu->lastOn = itemOn;
	M_SetupNextMenu(&NewGameClassicDef);
}

void M_DoNewGameLocal(int choice)
{
	if (!DynMaps || lastgamemode != gamemode || lastgamemission != gamemission)
		M_GenerateLevelPossible();
	
	M_NGOptionChange();
		
	currentMenu->lastOn = itemOn;
	M_SetupNextMenu(&CreateLocalGameDef);
}

void M_StartClassicGame(int choice)
{
	M_SetupGameOptionsNow(&NewGameClassicDef);
	M_WarpToMap(&NewGameClassicDef);
}

void M_StartLocalGame(int choice)
{
	M_SetupGameOptionsNow(&CreateLocalGameDef);
	M_WarpToMap(&CreateLocalGameDef);
}


void M_SetupGameOptionsNow(menu_t* Orig)
{
	// GLOBAL
	if (Orig == &CreateLocalGameDef || Orig == &NewGameClassicDef)
	{
		if (Orig == &NewGameClassicDef)
			CV_Set(&cv_deathmatch, "0");
		else
			CV_Set(&cv_deathmatch, va("%d", cv_ng_deathmatch.value));
		
		CV_Set(&cv_teamplay, cv_ng_teamplay.string);
		CV_Set(&cv_fraglimit, cv_ng_fraglimit.string);
		CV_Set(&cv_timelimit, cv_ng_timelimit.string);
		if (Orig == &NewGameCCSkillDef)
			CV_Set(&cv_spawnmonsters, "1");
		else
			CV_Set(&cv_spawnmonsters, cv_ng_spawnmonsters.string);
		
		switch (cv_ng_deathmatch.value)
		{
			case 1:
				break;
			case 2:
				CV_Set(&cv_itemrespawntime, cv_ng_itemrespawntime.string);
				CV_Set(&cv_itemrespawn, cv_ng_itemrespawn.string);
				break;
			case 3:
				break;
			default:
				break;
		}
		
		if (cv_ng_skill.value == 5)
		{
			CV_Set(&cv_respawnmonsters, "1");
			CV_Set(&cv_respawnmonsterstime, "12");
			CV_Set(&cv_fastmonsters, "1");
		}
		else
		{
			CV_Set(&cv_respawnmonsters, cv_ng_respawnmonsters.string);
			CV_Set(&cv_respawnmonsterstime, cv_ng_respawnmonsterstime.string);
			CV_Set(&cv_fastmonsters, cv_ng_fastmonsters.string);
		}
	}
	
	// CLASSIC
	if ((Orig == &CreateLocalGameDef || Orig == &NewGameClassicDef) &&
		cv_ng_options.value == 0)
	{
		CV_Set(&cv_teamdamage, "1");
		CV_Set(&cv_allowexitlevel, "1");
		CV_Set(&cv_allowjump, "0");
		CV_Set(&cv_allowautoaim, "1");
		CV_Set(&cv_forceautoaim, "1");
		CV_Set(&cv_allowrocketjump, "0");
		CV_Set(&cv_classicrocketblast, "1");
		CV_Set(&cv_allowturbo, "0");
		CV_Set(&cv_solidcorpse, "0");
		CV_Set(&cv_predictingmonsters, "0");
		CV_Set(&cv_classicmonsterlogic, "1");
		CV_Set(&cv_gravity, "1");
		CV_Set(&cv_classicblood, "1");
		CV_Set(&cv_classicmeleerange, "1");
		CV_Set(&cv_fragsweaponfalling, "0");
		CV_Set(&cv_bloodtime, "5");
	}
	
	// LEGACY
	else if ((Orig == &CreateLocalGameDef || Orig == &NewGameClassicDef) &&
		cv_ng_options.value == 1)
	{
		CV_Set(&cv_teamdamage, "0");
		CV_Set(&cv_allowexitlevel, "1");
		CV_Set(&cv_allowjump, "1");
		CV_Set(&cv_allowautoaim, "1");
		CV_Set(&cv_forceautoaim, "0");
		CV_Set(&cv_allowrocketjump, "0");
		CV_Set(&cv_classicrocketblast, "0");
		CV_Set(&cv_allowturbo, "0");
		CV_Set(&cv_solidcorpse, "0");
		CV_Set(&cv_predictingmonsters, "0");
		CV_Set(&cv_classicmonsterlogic, "0");
		CV_Set(&cv_gravity, "1");
		CV_Set(&cv_classicblood, "0");
		CV_Set(&cv_classicmeleerange, "0");
		CV_Set(&cv_fragsweaponfalling, "0");
		CV_Set(&cv_bloodtime, "20");
	}
	
	// REMOOD
	else if (((Orig == &CreateLocalGameDef || Orig == &NewGameClassicDef) &&
		cv_ng_options.value == 2) || (Orig == &NewGameCCSkillDef))
	{
		CV_Set(&cv_teamdamage, "0");
		CV_Set(&cv_allowexitlevel, "1");
		CV_Set(&cv_allowjump, "1");
		CV_Set(&cv_allowautoaim, "1");
		CV_Set(&cv_forceautoaim, "0");
		CV_Set(&cv_allowrocketjump, "0");
		CV_Set(&cv_classicrocketblast, "0");
		CV_Set(&cv_allowturbo, "0");
		CV_Set(&cv_solidcorpse, "0");
		CV_Set(&cv_predictingmonsters, "0");
		CV_Set(&cv_classicmonsterlogic, "0");
		CV_Set(&cv_gravity, "1");
		CV_Set(&cv_classicblood, "0");
		CV_Set(&cv_classicmeleerange, "1");
		CV_Set(&cv_fragsweaponfalling, "1");
		CV_Set(&cv_bloodtime, "10");
	}
		
	// CUSTOM
	else/*if ((Orig == &CreateLocalGameDef || Orig == &NewGameClassicDef) &&
		cv_ng_options.value == 3)*/
	{
		CV_Set(&cv_teamdamage, cv_ng_teamdamage.string);
		CV_Set(&cv_allowexitlevel, cv_ng_allowexitlevel.string);
		CV_Set(&cv_allowjump, cv_ng_allowjump.string);
		CV_Set(&cv_allowautoaim, cv_ng_allowautoaim.string);
		CV_Set(&cv_forceautoaim, cv_ng_forceautoaim.string);
		CV_Set(&cv_allowrocketjump, cv_ng_allowrocketjump.string);
		CV_Set(&cv_classicrocketblast, cv_ng_classicrocketblast.string);
		CV_Set(&cv_allowturbo, cv_ng_allowturbo.string);
		CV_Set(&cv_solidcorpse, cv_ng_solidcorpse.string);
		CV_Set(&cv_predictingmonsters, cv_ng_predictingmonsters.string);
		CV_Set(&cv_classicmonsterlogic, cv_ng_classicmonsterlogic.string);
		CV_Set(&cv_gravity, cv_ng_gravity.string);
		CV_Set(&cv_classicblood, cv_ng_classicblood.string);
		CV_Set(&cv_classicmeleerange, cv_ng_classicmeleerange.string);
		CV_Set(&cv_fragsweaponfalling, cv_ng_fragsweaponfalling.string);
		CV_Set(&cv_bloodtime, cv_ng_bloodtime.string);
	}
}

void M_WarpToMap(menu_t* Orig)
{
	int GameEp, GameMap;
	int Skill;
	int split;
	char buf[3];
	
	// Unset splitscreen, we play alone here
	if (Orig == &CreateLocalGameDef)
		split = cv_ng_splitscreen.value;
	else
		split = 0;
	
	// Skill
	if (Orig == &NewGameCCSkillDef)
		Skill = ccskill;
	else
	{
		if (cv_ng_skill.value == 6)
			Skill = M_Random() % 5;
		else
			Skill = cv_ng_skill.value - 1;
	}
	
	// One of the random options
	if (Orig == &NewGameCCSkillDef)
	{
		GameEp = ccepisode + 1;
		GameMap = 1;
	}
	else
		switch (cv_ng_map.value)
		{
			case 90:	// All Maps
				switch (gamemode)
				{
					case heretic:
						switch (gamemission)
						{
							case pack_heretic:
								GameEp = (M_Random() % 3) + 1;
								GameMap = (M_Random() % 9) + 1;
								break;
							case pack_heretic13:
								GameEp = (M_Random() % 6) + 1;
								GameMap = (M_Random() % 9) + 1;
								break;
							case pack_hereticsw:
							default:
								GameEp = 1;
								GameMap = (M_Random() % 9) + 1;
								break;
						}
						break;
					case chexquest1:
						GameEp = 1;
						GameMap = (M_Random() % 5) + 1;
						break;
					case shareware:
						GameEp = 1;
						GameMap = (M_Random() % 9) + 1;
						break;
					case registered:
						GameEp = (M_Random() % 3) + 1;
						GameMap = (M_Random() % 9) + 1;
						break;
					case retail:
						GameEp = (M_Random() % 4) + 1;
						GameMap = (M_Random() % 9) + 1;
						break;
					case commercial:	/* DOOM 2 */
						GameEp = 1;
						if (W_CheckNumForName("MAP31") != INVALIDLUMP)	// For German Doom 2 =/
							GameMap = (M_Random() % 32) + 1;
						else
							GameMap = (M_Random() % 30) + 1;
						break;
				}
				break;
			case 91:	// Episode 1 Maps
				GameEp = 1;
				GameMap = (M_Random() % 9) + 1;
				break;
			case 92:	// Episode 2 Maps
				GameEp = 2;
				GameMap = (M_Random() % 9) + 1;
				break;
			case 93:	// Episode 3 Maps
				GameEp = 3;
				GameMap = (M_Random() % 9) + 1;
				break;
			case 94:	// Episode 4 Maps
				GameEp = 4;
				GameMap = (M_Random() % 9) + 1;
				break;
			case 95:	// Episode 5
				GameEp = 5;
				GameMap = (M_Random() % 9) + 1;
				break;
			case 96:	// Episode 6
				GameEp = 6;
				GameMap = (M_Random() % 9) + 1;
				break;
			default:	// Not Random
				if (gamemode == commercial)
				{
					GameEp = 1;
					GameMap = cv_ng_map.value;
				}
				else
				{
					GameEp = cv_ng_map.value / 10;
					GameMap = cv_ng_map.value % 10;
				}
				break;
		}
	
	// Start the map
	G_DeferedInitNew(Skill, G_BuildMapName(GameEp, GameMap), split);
	
	// Close the menu
	M_ClearMenus(true);
}

// =============================================================================
//                                     MENU
// =============================================================================

//
// M_Init
//
void M_Init(void)
{
	int i;
	int j;
	menu_t* glob = NULL;
	patch_t *tpatch = NULL;

	if (dedicated)
		return;

	currentMenu = &MainDef;
	menuactive = 0;
	itemOn = currentMenu->lastOn;

	whichSkull = 0;
	skullAnimCounter = 10;

	quickSaveSlot = -1;

	switch (gamemode)
	{
		case commercial:
			break;
		case shareware:
		case registered:
		case retail:
			break;
		default:
			break;
	}
	CV_RegisterVar(&cv_skill);
	CV_RegisterVar(&cv_monsters);
	CV_RegisterVar(&cv_nextmap);
	CV_RegisterVar(&cv_newdeathmatch);
	
	// New Game Variables
	CV_RegisterVar(&cv_ng_map);
	CV_RegisterVar(&cv_ng_skill);
	CV_RegisterVar(&cv_ng_options);
	CV_RegisterVar(&cv_ng_splitscreen);
	CV_RegisterVar(&cv_ng_splitscreen2);
	CV_RegisterVar(&cv_ng_maxplayers);
	CV_RegisterVar(&cv_ng_maxclients);
	CV_RegisterVar(&cv_ng_map);
	CV_RegisterVar(&cv_ng_skill);
	CV_RegisterVar(&cv_ng_options);
	CV_RegisterVar(&cv_ng_deathmatch);
	CV_RegisterVar(&cv_ng_teamplay);
	CV_RegisterVar(&cv_ng_teamdamage);
	CV_RegisterVar(&cv_ng_fraglimit);
	CV_RegisterVar(&cv_ng_timelimit);
	CV_RegisterVar(&cv_ng_allowexitlevel);
	CV_RegisterVar(&cv_ng_allowjump);
	CV_RegisterVar(&cv_ng_allowautoaim);
	CV_RegisterVar(&cv_ng_forceautoaim);
	CV_RegisterVar(&cv_ng_allowrocketjump);
	CV_RegisterVar(&cv_ng_classicrocketblast);
	CV_RegisterVar(&cv_ng_allowturbo);
	CV_RegisterVar(&cv_ng_itemrespawntime);
	CV_RegisterVar(&cv_ng_itemrespawn);
	CV_RegisterVar(&cv_ng_spawnmonsters);
	CV_RegisterVar(&cv_ng_respawnmonsters);
	CV_RegisterVar(&cv_ng_respawnmonsterstime);
	CV_RegisterVar(&cv_ng_solidcorpse);
	CV_RegisterVar(&cv_ng_fastmonsters);
	CV_RegisterVar(&cv_ng_predictingmonsters);
	CV_RegisterVar(&cv_ng_classicmonsterlogic);
	CV_RegisterVar(&cv_ng_gravity);
	CV_RegisterVar(&cv_ng_classicblood);
	CV_RegisterVar(&cv_ng_classicmeleerange);
	CV_RegisterVar(&cv_ng_fragsweaponfalling);
	CV_RegisterVar(&cv_ng_bloodtime);
	
	/* GhostlyDeath <July 5, 2008> -- Dynamic menu stuff (So easy!) */
	i = 0;
	glob = MenuPtrList[0];
	while (glob)
	{
		glob = MenuPtrList[i];
		
		if (!glob)
		{
			i++;
			continue;
		}
		
		/* GhostlyDeath <July 6, 2008> -- Initialize some things */
		glob->lastOn = 0;
		
		/* GhostlyDeath <July 5, 2008> -- OPTIMAL SPACING */
		// This spaces everything for us!
		if (glob->extraflags & MENUFLAG_OPTIMALSPACE)
		{
			if (glob->menutitlepic && glob->menutitlepic[0] == '*')
			{
				if (gamemode == heretic)
					glob->menutitlepic = "M_HTIC";
				else
					glob->menutitlepic = "M_DOOM";
			}
			
			// First determine how big our title will be
			if (CharacterGroups[VFONT_LARGE] && glob->WMenuTitlePtr && UNICODE_StringLength(*(glob->WMenuTitlePtr)) > 0)
			{
				// Use height of the patch
				glob->menutitlex = (BASEVIDWIDTH / 2) - (V_StringWidthW(VFONT_LARGE, 0, *(glob->WMenuTitlePtr)) / 2);
				glob->menutitley = (MENUPADDING / 2);
				
				glob->x = MENUPADDING;
				glob->y = MENUPADDING + glob->menutitley + V_StringHeightW(VFONT_LARGE, 0, *(glob->WMenuTitlePtr));
				glob->width = BASEVIDWIDTH - (MENUPADDING * 2);
				glob->height = (BASEVIDHEIGHT - glob->menutitley - V_StringHeightW(VFONT_LARGE, 0, *(glob->WMenuTitlePtr))) - (MENUPADDING * 2);
			}
			else if (glob->menutitlepic)
			{
				tpatch = W_CachePatchName(glob->menutitlepic, PU_CACHE);
				
				if (tpatch)
				{
					// Center Title
					glob->menutitlex = (BASEVIDWIDTH / 2) - (tpatch->width / 2);
					glob->menutitley = (MENUPADDING / 2);
					
					// Box Menu Items
					glob->x = MENUPADDING;
					glob->y = MENUPADDING + glob->menutitley + tpatch->height;
					glob->width = BASEVIDWIDTH - (MENUPADDING * 2);
					glob->height = (BASEVIDHEIGHT - glob->menutitley - tpatch->height) - (MENUPADDING * 2);
				}
				else
				{
					// Box Menu Items
					glob->menutitlepic = NULL;
					glob->x = MENUPADDING;
					glob->y = MENUPADDING;
					glob->width = BASEVIDWIDTH - (MENUPADDING * 2);
					glob->height = BASEVIDHEIGHT - (MENUPADDING * 2);
				}
			}
			
			// Get Items per page
			glob->itemsperpage = 0;
			glob->firstdraw = 0;
			
			j = glob->y;
			while (j < glob->y + glob->height)
			{
				glob->itemsperpage++;
				j += STRINGHEIGHT;
			}
		}
		else
		{
			if (glob->menutitlex <= -1)
				glob->menutitlex = 0;
			if (glob->menutitley <= -1)
				glob->menutitley = 0;
				
			glob->itemsperpage = 500;
		}
		
		// Go to the next menu
		i++;
	}
}

//
// M_Ticker
//
void M_Ticker(void)
{
	if (dedicated)
		return;

	if (--skullAnimCounter <= 0)
	{
		whichSkull ^= 1;
		skullAnimCounter = 8 * NEWTICRATERATIO;
	}
}

//
// M_SetupNextMenu
//
void M_SetupNextMenu(menu_t * menudef)
{
	int status = 0;
	int cont = 1;
	
	if (currentMenu->quitroutine)
	{
		if (!currentMenu->quitroutine())
			return;				// we can't quit this menu (also used to set parameter from the menu)
	}
	
	currentMenu = menudef;
	itemOn = currentMenu->lastOn;
	
	// GhostlyDeath <July 11, 2008> -- Never park on an item that can't be selected
	status = currentMenu->menuitems[itemOn].status;
	while (((status & IT_TYPE) == IT_SPACE) ||
		(((status & IT_TYPE) != IT_SPACE) && (status & IT_DISABLED2)))
	{
		itemOn++;
	
		if (itemOn >= currentMenu->numitems)
			itemOn = 0;
	
		status = currentMenu->menuitems[itemOn].status;
	}
}

static int controltochange;
extern int (*setupcontrols)[2];

void M_ChangecontrolResponse(event_t * ev)
{
	int control;
	int found;
	int ch = ev->data1;

	// ESCAPE cancels
	if (ch != KEY_ESCAPE && ch != KEY_PAUSE)
	{

		switch (ev->type)
		{
				// ignore mouse/joy movements, just get buttons
			case ev_mouse:
				ch = KEY_NULL;	// no key
				break;
			case ev_joystick:
				ch = KEY_NULL;	// no key
				break;

				// keypad arrows are converted for the menu in cursor arrows
				// so use the event instead of ch
			case ev_keydown:
				ch = ev->data1;
				break;

			default:
				break;
		}

		control = controltochange;

		// check if we already entered this key
		found = -1;
		if (setupcontrols[control][0] == ch)
			found = 0;
		else if (setupcontrols[control][1] == ch)
			found = 1;
		if (found >= 0)
		{
			// replace mouse and joy clicks by double clicks
			if (ch >= KEY_MOUSE1B1 && ch <= KEY_MOUSE1B1 + MOUSEBUTTONS)
				setupcontrols[control][found] = ch - KEY_MOUSE1B1 + KEY_MOUSE1DBL1;
			else if (ch >= KEY_JOY1B1 && ch <= KEY_JOY1B1 + JOYBUTTONS)
				setupcontrols[control][found] = ch - KEY_JOY1B1 + KEY_JOY1DBL1;
		}
		else
		{
			// check if change key1 or key2, or replace the two by the new
			found = 0;
			if (setupcontrols[control][0] == KEY_NULL)
				found++;
			if (setupcontrols[control][1] == KEY_NULL)
				found++;
			if (found == 2)
			{
				found = 0;
				setupcontrols[control][1] = KEY_NULL;	//replace key 1 ,clear key2
			}
			G_CheckDoubleUsage(ch);
			setupcontrols[control][found] = ch;
		}

	}

	M_StopMessage(0);
}

void M_ChangeControl(int choice)
{
	static wchar_t tmp[55];

	controltochange = currentMenu->menuitems[choice].alphaKey;
	swprintf(tmp, L"Hit the new key for\n%s\nESC for Cancel", *(currentMenu->menuitems[choice].WItemTextPtr));

	M_StartMessageW(tmp, M_ChangecontrolResponse, MM_EVENTHANDLER);
}

void M_ControlsDoPlayer1(int choice)
{
	setupcontrols = gamecontrol[0];
	currentMenu->lastOn = itemOn;
	DefaultKeyBindDef.WMenuTitlePtr = PTROFUNICODESTRING(MENU_OTHER_PLAYERACONTROLS);
	M_SetupNextMenu(&DefaultKeyBindDef);
}
void M_ControlsDoPlayer2(int choice)
{
	setupcontrols = gamecontrol[1];
	currentMenu->lastOn = itemOn;
	DefaultKeyBindDef.WMenuTitlePtr = PTROFUNICODESTRING(MENU_OTHER_PLAYERBCONTROLS);
	M_SetupNextMenu(&DefaultKeyBindDef);
}
void M_ControlsDoPlayer3(int choice)
{
	setupcontrols = gamecontrol[2];
	currentMenu->lastOn = itemOn;
	DefaultKeyBindDef.WMenuTitlePtr = PTROFUNICODESTRING(MENU_OTHER_PLAYERCCONTROLS);
	M_SetupNextMenu(&DefaultKeyBindDef);
}
void M_ControlsDoPlayer4(int choice)
{
	setupcontrols = gamecontrol[3];
	currentMenu->lastOn = itemOn;
	DefaultKeyBindDef.WMenuTitlePtr = PTROFUNICODESTRING(MENU_OTHER_PLAYERDCONTROLS);
	M_SetupNextMenu(&DefaultKeyBindDef);
}


//
// CONTROL PANEL
//

void M_ChangeCvar(int choise)
{
	consvar_t *cv = (consvar_t *) currentMenu->menuitems[itemOn].itemaction;
	
	if (currentMenu->menuitems[itemOn].status & IT_CVARREADONLY)
		return;

	if (((currentMenu->menuitems[itemOn].status & IT_CVARTYPE) == IT_CV_SLIDER)
		|| ((currentMenu->menuitems[itemOn].status & IT_CVARTYPE) == IT_CV_NOMOD))
	{
		CV_SetValue(cv, cv->value + choise * 2 - 1);
	}
	else if (cv->flags & CV_FLOAT)
	{
		char s[20];
		sprintf(s, "%f", (float)cv->value / FRACUNIT + (choise * 2 - 1) * (1.0 / 16.0));
		CV_Set(cv, s);
	}
	else
		CV_AddValue(cv, choise * 2 - 1);
}

boolean M_ChangeStringCvar(int choise)
{
	consvar_t *cv = (consvar_t *) currentMenu->menuitems[itemOn].itemaction;
	char buf[255];
	int len;
	
	if (currentMenu->menuitems[itemOn].status & IT_CVARREADONLY)
		return;

	switch (choise)
	{
		case KEY_BACKSPACE:
			len = strlen(cv->string);
			if (len > 0)
			{
				memcpy(buf, cv->string, len);
				buf[len - 1] = 0;
				CV_Set(cv, buf);
			}
			return true;
		default:
			if (choise >= 32 && choise <= 127)
			{
				len = strlen(cv->string);
				if (len < MAXSTRINGLENGTH - 1)
				{
					if (shiftdown)
					{
						switch (choise)
						{
							case '-': choise = '_'; break;
							case '=': choise = '+'; break;
							case ';': choise = ':'; break;
							case '\"': choise = '\''; break;
							case ',': choise = '<'; break;
							case '.': choise = '>'; break;
							case '/': choise = '?'; break;
							case '1': choise = '!'; break;
							case '2': choise = '@'; break;
							case '3': choise = '#'; break;
							case '4': choise = '$'; break;
							case '5': choise = '%'; break;
							case '6': choise = '^'; break;
							case '7': choise = '&'; break;
							case '8': choise = '*'; break;
							case '9': choise = '('; break;
							case '0': choise = ')'; break;
						}
					}
					memcpy(buf, cv->string, len);
					buf[len++] = choise;
					buf[len] = 0;
					CV_Set(cv, buf);
				}
				return true;
			}
			break;
	}
	return false;
}

//
// M_Responder
//
boolean M_Responder(event_t * ev)
{
	int ch;
	int i;
	int adjustfirstdraw = 0;
	static tic_t joywait = 0;
	static tic_t mousewait = 0;
	static int mousey = 0;
	static int lasty = 0;
	static int mousex = 0;
	static int lastx = 0;
	void (*routine) (int choice);	// for some casting problem

	ch = -1;

	if (ev->type == ev_keydown)
	{
		ch = ev->data1;

		// added 5-2-98 remap virtual keys (mouse & joystick buttons)
		switch (ch)
		{
			case KEY_MOUSE1B1:
				ch = KEY_ENTER;
				break;
			case KEY_MOUSE1B1 + 1:
				ch = KEY_BACKSPACE;
				break;
			case KEY_JOY1B1:
			case KEY_JOY1B1 + 2:
			case KEY_JOY1B1 + 3:
				ch = KEY_ENTER;
				break;
			case KEY_JOY1B1 + 1:
				ch = KEY_BACKSPACE;
				break;
		}
	}
	else if (menuactive)
	{
		if (ev->type == ev_joystick && joywait < I_GetTime())
		{
			if (ev->data3 == -1)
			{
				ch = KEY_UPARROW;
				joywait = I_GetTime() + TICRATE / 7;
			}
			else if (ev->data3 == 1)
			{
				ch = KEY_DOWNARROW;
				joywait = I_GetTime() + TICRATE / 7;
			}

			if (ev->data2 == -1)
			{
				ch = KEY_LEFTARROW;
				joywait = I_GetTime() + TICRATE / 17;
			}
			else if (ev->data2 == 1)
			{
				ch = KEY_RIGHTARROW;
				joywait = I_GetTime() + TICRATE / 17;
			}
		}
		else
		{
			if (ev->type == ev_mouse && mousewait < I_GetTime())
			{
				mousey += ev->data3;
				if (mousey < lasty - 30)
				{
					ch = KEY_DOWNARROW;
					mousewait = I_GetTime() + TICRATE / 7;
					mousey = lasty -= 30;
				}
				else if (mousey > lasty + 30)
				{
					ch = KEY_UPARROW;
					mousewait = I_GetTime() + TICRATE / 7;
					mousey = lasty += 30;
				}

				mousex += ev->data2;
				if (mousex < lastx - 30)
				{
					ch = KEY_LEFTARROW;
					mousewait = I_GetTime() + TICRATE / 7;
					mousex = lastx -= 30;
				}
				else if (mousex > lastx + 30)
				{
					ch = KEY_RIGHTARROW;
					mousewait = I_GetTime() + TICRATE / 7;
					mousex = lastx += 30;
				}
			}
		}
	}

	if (ch == -1)
		return false;

	// Save Game string input
	/*if (saveStringEnter)
	{
		switch (ch)
		{
			case KEY_BACKSPACE:
				if (saveCharIndex > 0)
				{
					saveCharIndex--;
					savegamestrings[saveSlot][saveCharIndex] = 0;
				}
				break;

			case KEY_ESCAPE:
				saveStringEnter = 0;
				strcpy(&savegamestrings[saveSlot][0], saveOldString);
				break;

			case KEY_ENTER:
				saveStringEnter = 0;
				if (savegamestrings[saveSlot][0])
					M_DoSave(saveSlot);
				break;

			default:
				ch = toupper(ch);
				if (ch != 32)
					if (ch - HU_FONTSTART < 0 || ch - HU_FONTSTART >= HU_FONTSIZE)
						break;
				if (ch >= 32 && ch <= 127 &&
					saveCharIndex < SAVESTRINGSIZE - 1 &&
					V_StringWidth(savegamestrings[saveSlot]) < (SAVESTRINGSIZE - 2) * 8)
				{
					savegamestrings[saveSlot][saveCharIndex++] = ch;
					savegamestrings[saveSlot][saveCharIndex] = 0;
				}
				break;
		}
		return true;
	}*/

	if (devparm && ch == KEY_F1)
	{
		COM_BufAddText("screenshot\n");
		return true;
	}

	// F-Keys
	if (!menuactive)
	{
		switch (ch)
		{
			case KEY_MINUS:	// Screen size down
				if (automapactive || chat_on || con_destlines)	// DIRTY !!!
					return false;
				CV_SetValue(&cv_viewsize, cv_viewsize.value - 1);
				S_StartSound(NULL, GAMESTNMOV);
				return true;

			case KEY_EQUALS:	// Screen size up
				if (automapactive || chat_on || con_destlines)	// DIRTY !!!
					return false;
				CV_SetValue(&cv_viewsize, cv_viewsize.value + 1);
				S_StartSound(NULL, GAMESTNMOV);
				return true;

/*			case KEY_F1:		// Help key
				M_StartControlPanel();

				if (gamemode == retail)
					currentMenu = &ReadDef2;
				else
					currentMenu = &ReadDef1;

				itemOn = 0;
				S_StartSound(NULL, sfx_swtchn);
				return true;

			case KEY_F2:		// Save
				M_StartControlPanel();
				S_StartSound(NULL, sfx_swtchn);
				M_SaveGame(0);
				return true;

			case KEY_F3:		// Load
				M_StartControlPanel();
				S_StartSound(NULL, sfx_swtchn);
				M_LoadGame(0);
				return true;

			case KEY_F4:		// Sound Volume
				M_StartControlPanel();
				currentMenu = &SoundDef;
				itemOn = sfx_vol;
				S_StartSound(NULL, sfx_swtchn);
				return true;

				//added:26-02-98: now F5 calls the Video Menu
			case KEY_F5:
				S_StartSound(NULL, sfx_swtchn);
				M_StartControlPanel();
				M_SetupNextMenu(&VidModeDef);
				//M_ChangeDetail(0);
				return true;

			case KEY_F6:		// Quicksave
				S_StartSound(NULL, sfx_swtchn);
				M_QuickSave();
				return true;

				//added:26-02-98: F7 changed to Options menu
			case KEY_F7:		// End game
				S_StartSound(NULL, sfx_swtchn);
				M_StartControlPanel();
				M_SetupNextMenu(&OptionsDef);
				//M_EndGame(0);
				return true;
*/
			case KEY_F8:		// Toggle messages
				CV_AddValue(&cv_showmessages, +1);
				S_StartSound(NULL, GAMESWITCHN);
				return true;
/*
			case KEY_F9:		// Quickload
				S_StartSound(NULL, GAMESWITCHN);
				M_QuickLoad();
				return true;*/

			case KEY_F10:		// Quit DOOM
				S_StartSound(NULL, GAMESWITCHN);
				M_QuitDOOM(0);
				return true;

				//added:10-02-98: the gamma toggle is now also in the Options menu
			case KEY_F11:
				S_StartSound(NULL, GAMESWITCHN);
				CV_AddValue(&cv_usegamma, +1);
				return true;

				// Pop-up menu
			case KEY_ESCAPE:
				M_StartControlPanel();
				S_StartSound(NULL, GAMESWITCHN);
				return true;
		}
		return false;
	}

	routine = currentMenu->menuitems[itemOn].itemaction;

	//added:30-01-98:
	// Handle menuitems which need a specific key handling
	if (routine && (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_KEYHANDLER)
	{
		routine(ch);
		return true;
	}

	if (currentMenu->menuitems[itemOn].status == IT_MSGHANDLER)
	{
		if (currentMenu->menuitems[itemOn].alphaKey == true)
		{
			if (ch == ' ' || ch == 'n' || ch == 'y' || ch == KEY_ESCAPE)
			{
				if (routine)
					routine(ch);
				M_StopMessage(0);
				return true;
			}
			return true;
		}
		else
		{
			//added:07-02-98:dirty hak:for the customise controls, I want only
			//      buttons/keys, not moves
			if (ev->type == ev_mouse || ev->type == ev_joystick)
				return true;
			if (routine)
				routine((int)ev);
			return true;
		}
	}

	// BP: one of the more big hack i have never made
	if (routine && (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_CVAR)
	{
		if ((currentMenu->menuitems[itemOn].status & IT_CVARTYPE) == IT_CV_STRING)
		{
			if (M_ChangeStringCvar(ch))
				return true;
			else
				routine = NULL;
		}
		else
			routine = M_ChangeCvar;
	}
	// Keys usable within menu
	switch (ch)
	{
		case KEY_DOWNARROW:
			do
			{
				if (itemOn + 1 > currentMenu->numitems - 1)
					itemOn = 0;
				else
					itemOn++;
			}
			while (((currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_SPACE) ||
					(currentMenu->menuitems[itemOn].status & IT_DISABLED2));
			S_StartSound(NULL, sfx_pstop);
			goto adjustfirstdraw;;

		case KEY_UPARROW:
			do
			{
				if (!itemOn)
					itemOn = currentMenu->numitems - 1;
				else
					itemOn--;
			}
			while (((currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_SPACE) ||
					(currentMenu->menuitems[itemOn].status & IT_DISABLED2));
			S_StartSound(NULL, sfx_pstop);
			goto adjustfirstdraw;

		case KEY_LEFTARROW:
			if (routine &&
				((currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_ARROWS
				 || (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_CVAR))
			{
				S_StartSound(NULL, GAMESTNMOV);
				routine(0);
			}
			adjustfirstdraw = 1;
			break;

		case KEY_RIGHTARROW:
			if (routine &&
				((currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_ARROWS
				 || (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_CVAR))
			{
				S_StartSound(NULL, GAMESTNMOV);
				routine(1);
			}
			return true;

		case KEY_ENTER:
			currentMenu->lastOn = itemOn;
			if (routine)
			{
				switch (currentMenu->menuitems[itemOn].status & IT_TYPE)
				{
					case IT_CVAR:
					case IT_ARROWS:
						routine(1);	// right arrow
						S_StartSound(NULL, GAMESTNMOV);
						break;
					case IT_CALL:
						routine(itemOn);
						S_StartSound(NULL, GAMEPISTOL);
						break;
					case IT_SUBMENU:
						currentMenu->lastOn = itemOn;
						M_SetupNextMenu((menu_t *) currentMenu->menuitems[itemOn].itemaction);
						S_StartSound(NULL, GAMEPISTOL);
						break;
				}
			}
			return true;

		case KEY_ESCAPE:
			currentMenu->lastOn = itemOn;
			if (currentMenu->prevMenu)
			{
				currentMenu = currentMenu->prevMenu;
				itemOn = currentMenu->lastOn;
				S_StartSound(NULL, GAMESWITCHX);	// its a matter of taste which sound to choose
				//S_StartSound(NULL,GAMESWITCHN);
			}
			else
			{
				M_ClearMenus(true);
				S_StartSound(NULL, GAMESWITCHX);
			}

			return true;
		case KEY_BACKSPACE:
			if ((currentMenu->menuitems[itemOn].status) == IT_CONTROL)
			{
				S_StartSound(NULL, GAMESTNMOV);
				// detach any keys associated to the game control
				G_ClearControlKeys(setupcontrols, currentMenu->menuitems[itemOn].alphaKey);
				return true;
			}
			currentMenu->lastOn = itemOn;
			if (currentMenu->prevMenu)
			{
				currentMenu = currentMenu->prevMenu;
				itemOn = currentMenu->lastOn;
				S_StartSound(NULL, GAMESWITCHN);
			}
			return true;

		default:
			for (i = itemOn + 1; i < currentMenu->numitems; i++)
				if (currentMenu->menuitems[i].alphaKey == ch)
				{
					itemOn = i;
					S_StartSound(NULL, sfx_pstop);
					goto adjustfirstdraw;
				}
			for (i = 0; i <= itemOn; i++)
				if (currentMenu->menuitems[i].alphaKey == ch)
				{
					itemOn = i;
					S_StartSound(NULL, sfx_pstop);
					goto adjustfirstdraw;
				}
			break;

	}
	
adjustfirstdraw:
	/* GhostlyDeath <June 6, 2008> -- Scrolling menus */
	if (currentMenu->numitems > 5)
	{
		// itemOn < first
		if (itemOn < currentMenu->firstdraw + 2)
			currentMenu->firstdraw = itemOn - 2;
		
		// More than a page
		if (itemOn > (currentMenu->firstdraw + currentMenu->itemsperpage) - 3)
			currentMenu->firstdraw += itemOn - ((currentMenu->firstdraw + currentMenu->itemsperpage) - 3);
		
		// More than numitems
		if (currentMenu->firstdraw > currentMenu->numitems - 3)
			currentMenu->firstdraw = currentMenu->numitems - 3;
	
		// Never leave a gap from the bottom to the items	
		if (currentMenu->numitems - currentMenu->itemsperpage > 0 &&
			currentMenu->firstdraw > currentMenu->numitems - currentMenu->itemsperpage)
			currentMenu->firstdraw = currentMenu->numitems - currentMenu->itemsperpage;
	}
	else
	{
		// itemOn < first
		if (itemOn < currentMenu->firstdraw)
			currentMenu->firstdraw = itemOn;
		
		// More than a page
		if (itemOn > (currentMenu->firstdraw + currentMenu->itemsperpage) - 1)
			currentMenu->firstdraw += itemOn - ((currentMenu->firstdraw + currentMenu->itemsperpage) - 1);
		
		// More than numitems
		if (currentMenu->firstdraw > currentMenu->numitems - 1)
			currentMenu->firstdraw = currentMenu->numitems - 1;
	
		// Never leave a gap from the bottom to the items	
		if (currentMenu->numitems - currentMenu->itemsperpage > 0 &&
			currentMenu->firstdraw > currentMenu->numitems - currentMenu->itemsperpage)
			currentMenu->firstdraw = currentMenu->numitems - currentMenu->itemsperpage;
	}
	
	// Less than 0 (should never happen)
	if (currentMenu->firstdraw < 0)
		currentMenu->firstdraw = 0;
		
	return true;
}

void M_LockGameCVARS(void)
{
	int i;
	
	for (i = 0; i < GameOptionsDef.numitems; i++)
		if (GameOptionsDef.menuitems[i].status != IT_SPACE)
			GameOptionsDef.menuitems[i].status |= IT_CVARREADONLY;
}

void M_UnLockGameCVARS(void)
{
	int i;
	
	for (i = 0; i < GameOptionsDef.numitems; i++)
		if (GameOptionsDef.menuitems[i].status != IT_SPACE)
			GameOptionsDef.menuitems[i].status &= ~IT_CVARREADONLY;
}

void M_ResetSound(int choice)
{
	I_ShutdownSound();
	I_StartupSound();
}

/*** MOUSE OPTIONS ***/
void M_MouseModeChange(void)
{
}

