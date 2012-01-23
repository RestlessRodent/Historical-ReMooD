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
// DESCRIPTION: Profiles

#include "d_prof.h"
#include "console.h"
#include "r_data.h"
#include "z_zone.h"
#include "m_menu.h"
#include "r_things.h"
#include "r_draw.h"
#include "doomstat.h"
#include "v_video.h"
#include "keys.h"
#include "g_game.h"
#include "dstrings.h"
#include "s_sound.h"

ProfileInfo_t* Profiles = NULL;

// Player has no "true" profile (not even default) for demos and netplay
ProfileInfo_t NonLocalProfile;

ProfileInfo_t* PROF_GetNumber(size_t Num)
{
	ProfileInfo_t* Rover = Profiles;
	size_t i = 0;
	
	while (Rover && i != Num)
	{
		i++;
		Rover = Rover->next;
	}
	
	return Rover;
}

/* PROF_NumProfiles() -- Returns profile count */
size_t PROF_NumProfiles(void)
{
	ProfileInfo_t* Rover = Profiles;
	size_t i = 0;
	
	while (Rover)
	{
		i++;
		Rover = Rover->next;
	}
	
	return i;
}

/* PROF_CVARWasChanged() -- A cvar linked to a profile was changed */
// This isn't the greatest but it works
void PROF_CVARWasChanged(void)
{
	ProfileInfo_t* Rover = Profiles;
	player_t* Player = NULL;
	size_t i;
	
	if (!demoplayback && gamestate == GS_LEVEL)
	{
		while (Rover)
		{
			for (i = 0; i < cv_splitscreen.value + 1; i++)
			{
				if (playeringame[consoleplayer[i]] && Rover == players[consoleplayer[i]].profile)
				{
					// Set the player
					Player = &players[consoleplayer[i]];
					
					// Change Name
					strncpy(player_names[consoleplayer[i]], Rover->cvars[PC_NAME].string, MAXPLAYERNAME);
					player_names[consoleplayer[i]][MAXPLAYERNAME - 1] = 0;
					
					// Change Color
					Player->skincolor = Rover->cvars[PC_COLOR].value % MAXSKINCOLORS;
					
					if (Player->mo)
					{
						Player->mo->flags &= ~MF_TRANSLATION;
						Player->mo->flags |= (Player->skincolor) << MF_TRANSSHIFT;
					}
					// Change Skin
					SetPlayerSkin(consoleplayer[i], Rover->cvars[PC_SKIN].string);
					
					// Change autoaim
					Player->autoaim_toggle = Rover->cvars[PC_AUTOAIM].value;
				}
			}
			
			Rover = Rover->next;
		}
	}
}

consvar_t BaseProfileCVARS[MAXPROFILECVARS] =
{
	{"name", "Player", CV_SAVE | CV_CALL, NULL, PROF_CVARWasChanged}
	,
	{"color", "0", CV_SAVE | CV_CALL, Color_cons_t, PROF_CVARWasChanged}
	,
	{"skin", "marine", CV_SAVE | CV_CALL, NULL, PROF_CVARWasChanged}
	,
	{"autoaim", "1", CV_SAVE | CV_CALL, CV_YesNo, PROF_CVARWasChanged}
	,
};

void PROF_M_ChangeProfile(void);

CV_PossibleValue_t* DynPossible = NULL;
consvar_t DynProfileChooser = { "prof_dynchooser", "default", CV_HIDEN | CV_CALL, NULL, PROF_M_ChangeProfile };
consvar_t InGameProfileChooser = { "prof_ingamechooser", "default", CV_HIDEN, NULL, NULL };
consvar_t TempProfileName = { "prof_tempname", "", CV_HIDEN, NULL, NULL };

char* StrLwr(char* Str)
{
	char* b = Str;
	
	while (*b != 0)
	{
		if (*b >= 'A' && *b <= 'Z')
			*b += 32;
		b++;
	}
	
	return Str;
}

static ProfileInfo_t* PROF_FindProfile(char* Name)
{
	ProfileInfo_t* Rover = Profiles;
	
	if (Profiles)
	{
		while (Rover)
		{
			if (strcmp(Name, Rover->name) == 0)
				return Rover;
			Rover = Rover->next;
		}
		
		return NULL;
	}
	else
		return NULL;
}

static ProfileInfo_t* PROF_CreateProfile(char* Name)
{
	ProfileInfo_t* Temp = NULL;
	ProfileInfo_t* Rover = NULL;
	CV_PossibleValue_t* cleaner = NULL;
	int i;
	int count;
	char buf[128];
	
	if (Temp = PROF_FindProfile(Name))
		return Temp;
		
	if (!Profiles)
	{
		Profiles = Z_Malloc(sizeof(ProfileInfo_t), PU_STATIC, NULL);
		Temp = Profiles;
		memset(Temp, 0, sizeof(ProfileInfo_t));
	}
	else
	{
		Rover = Profiles;
		while (Rover->next)
			Rover = Rover->next;
		Rover->next = Z_Malloc(sizeof(ProfileInfo_t), PU_STATIC, NULL);
		Temp = Rover->next;
		memset(Temp, 0, sizeof(ProfileInfo_t));
		Temp->prev = Rover;
	}
	
	// Copy Name
	strncpy(Temp->name, Name, MAXPLAYERNAME);
	
	// Create Base CVARs
	for (i = 0; i < MAXPROFILECVARS; i++)
	{
		memset(buf, 0, sizeof(buf));
		
		// Set Name
		sprintf(buf, "profile_%s_%s", Name, BaseProfileCVARS[i].name);
		Temp->cvars[i].name = Z_Malloc(strlen(buf) + 1, PU_STATIC, NULL);
		strcpy(Temp->cvars[i].name, buf);
		
		// Others can be directly copied since they are very similar
		Temp->cvars[i].defaultvalue = BaseProfileCVARS[i].defaultvalue;
		Temp->cvars[i].flags = BaseProfileCVARS[i].flags;
		Temp->cvars[i].PossibleValue = BaseProfileCVARS[i].PossibleValue;
		Temp->cvars[i].func = BaseProfileCVARS[i].func;
		
		// And finally, we register it
		CV_RegisterVar(&Temp->cvars[i]);
	}
	
	if (DynPossible)
	{
		cleaner = DynPossible;
		while (cleaner->strvalue)
		{
			Z_Free(cleaner->strvalue);
			cleaner++;
		}
		
		Z_Free(DynPossible);
		DynPossible = NULL;
	}
	// Recreate possible value
	count = 0;
	Rover = Profiles;
	while (Rover)
	{
		count++;
		Rover = Rover->next;
	}
	
	DynPossible = Z_Malloc(sizeof(CV_PossibleValue_t) * (count + 1), PU_STATIC, NULL);
	
	i = 0;
	Rover = Profiles;
	
	while (Rover)
	{
		DynPossible[i].value = i;
		DynPossible[i].strvalue = Z_Malloc(strlen(Rover->name) + 1, PU_STATIC, NULL);
		strcpy(DynPossible[i].strvalue, Rover->name);
		
		i++;
		Rover = Rover->next;
	}
	
	DynPossible[i].value = 0;
	DynPossible[i].strvalue = NULL;
	
	DynProfileChooser.PossibleValue = DynPossible;
	DynProfileChooser.value = 0;
	DynProfileChooser.string = DynPossible[0].strvalue;
	InGameProfileChooser.PossibleValue = DynPossible;
	InGameProfileChooser.value = 0;
	InGameProfileChooser.string = DynPossible[0].strvalue;
	
	CONL_PrintF("PROF_Init: Created profile \"%s\"!\n", Name);
	
	return Temp;
}

void PROF_Init(void)
{
	CONL_PrintF("PROF_Init: Initializing profile subsystem...\n");
	
	// Create Default profile
	PROF_CreateProfile("default");
	CV_RegisterVar(&DynProfileChooser);
	CV_RegisterVar(&TempProfileName);
}

void PROF_Shutdown(void)
{
	CONL_PrintF("PROF_Init: Stopping profile subsystem...\n");
}

void PROF_HandleVAR(char* arg0, char* arg1)
{
	char UserBuf[MAXPLAYERNAME];
	char ModBuf[24];
	char CVARBuf[128];
	int i, j, k, l;
	char* b;
	ProfileInfo_t* Prof;
	consvar_t* Tmp = NULL;
	
	memset(UserBuf, 0, sizeof(UserBuf));
	memset(ModBuf, 0, sizeof(ModBuf));
	memset(CVARBuf, 0, sizeof(CVARBuf));
	
	/* Skip the first part of the argument */
	arg0 += strlen("profile_");
	
	/* Get the profile name */
	b = arg0;
	i = 0;
	while (*b != '_')
	{
		if (i < MAXPLAYERNAME)
			UserBuf[i] = *b;
		i++;
		b++;
		arg0++;
	}
	UserBuf[MAXPLAYERNAME - 1] = 0;
	
	StrLwr(UserBuf);
	
	/* Create profile (or if it exists, return it's ptr) and get the ptr */
	Prof = PROF_CreateProfile(UserBuf);
	
	/* Set value */
	arg0++;
	i = 0;
	while (*arg0 != ' ' && *arg0 != 0)
	{
		if (i < 24)
			ModBuf[i] = *arg0;
		i++;
		arg0++;
	}
	
	ModBuf[24 - 1] = 0;
	
	/* Create Variable to change */
	sprintf(CVARBuf, "profile_%s_%s", UserBuf, ModBuf);
	if (Tmp = CV_FindVar(CVARBuf))
		CV_Set(Tmp, arg1);
}

void M_DrawProfileMenu(void);
void M_HandleSkinChanger(int choice);
void M_CreateProfile(int choice);

menuitem_t ProfileItems[] =
{
	{IT_WHITESTRING | IT_CALL, NULL, PTROFUNICODESTRING(DSTR_MENUPROFILES_CREATEPROFILE), M_CreateProfile},
	{IT_STRING | IT_CVAR, NULL, PTROFUNICODESTRING(DSTR_MENUPROFILES_CURRENTPROFILE), &DynProfileChooser},
	{IT_STRING | IT_SPACE, NULL, PTROFUNICODESTRING(DSTR_MENUNULLSPACE), NULL},
	{IT_STRING | IT_CVAR | IT_CV_STRING, NULL, PTROFUNICODESTRING(DSTR_MENUPROFILES_NAME), NULL},
	{IT_STRING | IT_CVAR, NULL, PTROFUNICODESTRING(DSTR_MENUPROFILES_COLOR), NULL},
	{IT_STRING | IT_KEYHANDLER, NULL, PTROFUNICODESTRING(DSTR_MENUPROFILES_SKIN), M_HandleSkinChanger},
	{IT_STRING | IT_CVAR, NULL, PTROFUNICODESTRING(DSTR_MENUPROFILES_AUTOAIM), NULL},
};

menu_t ProfileDef =
{
	MENUFLAG_OPTIMALSPACE,
	"M_OPTION",
	PTROFUNICODESTRING(DSTR_MENUPROFILES_TITLE),
	sizeof(ProfileItems) / sizeof(menuitem_t),
	ProfileItems,
	&MainDef,
	M_DrawProfileMenu,
	0,
	0,
	0,
	0,
	1,
};

void M_AcceptNewProfile(int choice);

menuitem_t CreateProfileItems[] =
{
	{IT_SPACE | IT_STRING | IT_CENTERSTRING, NULL, PTROFUNICODESTRING(DSTR_MENUCREATEPROFILE_PLEASENAME), NULL},
	{IT_STRING | IT_CVAR | IT_CV_STRING, NULL, PTROFUNICODESTRING(DSTR_MENUCREATEPROFILE_NAME), &TempProfileName},
	{IT_WHITESTRING | IT_CALL, NULL, PTROFUNICODESTRING(DSTR_MENUCREATEPROFILE_ACCEPT), M_AcceptNewProfile},
};

menu_t CreateProfileDef =
{
	0,
	NULL,
	PTROFUNICODESTRING(DSTR_MENUCREATEPROFILE_TITLE),
	sizeof(CreateProfileItems) / sizeof(menuitem_t),
	CreateProfileItems,
	&ProfileDef,
	M_DrawGenericMenu,
	BASEVIDWIDTH >> 2,			// X
	(BASEVIDHEIGHT >> 1) - (STRINGHEIGHT << 1),	// Y
	BASEVIDWIDTH >> 1,			// W
	STRINGHEIGHT << 2,			// H
	1
};

void M_AcceptNewProfile(int choice)
{
	char* a;
	char* b;
	
	// Check some things
	if (!TempProfileName.string)
		return;
		
	if (strlen(TempProfileName.string) < 1)
		return;
		
	// always lowercase
	StrLwr(TempProfileName.string);
	
	// Remove Spaces and underscores
	if (strcasecmp(TempProfileName.string, "bananacreampie") != 0)
	{
		a = TempProfileName.string;
		b = a;
		while (*b)
		{
			if (!(((*b >= 'a') && (*b <= 'z')) || ((*b >= '0') && (*b <= '9'))))
				b++;
			else
			{
				*a = *b;
				a++;
				b++;
			}
		}
		
		*a = 0;
	}
	// If someone decides they want an empty name, which they can't have...
	if (*TempProfileName.string == 0 || strlen(TempProfileName.string) < 1)
		CV_Set(&TempProfileName, "bananacreampie");	//TempProfileName.string = BananaCreamPie;
		
	// Profile can't already exist
	if (PROF_FindProfile(TempProfileName.string))
		return;
		
	// Otherwise
	PROF_CreateProfile(TempProfileName.string);
	
	currentMenu->lastOn = itemOn;
	M_SetupNextMenu(&ProfileDef);
}

void M_CreateProfile(int choice)
{
	CV_Set(&TempProfileName, "");
	currentMenu->lastOn = itemOn;
	CreateProfileDef.firstdraw = 0;
	M_SetupNextMenu(&CreateProfileDef);
}

void M_DrawProfileMenu(void)
{
	spritedef_t* sprdef;
	spriteframe_t* sprframe;
	int lump;
	patch_t* patch;
	uint8_t* colormap;
	char* skinchar;
	
	// Draw player sprite
	sprdef = &skins[R_SkinAvailable(PROF_GetNumber(((consvar_t*) (currentMenu->menuitems[1].itemaction))->value)->cvars[PC_SKIN].string)].spritedef;
	sprframe = &sprdef->spriteframes[((gametic % 16) >> 2) & FF_FRAMEMASK];
	lump = sprframe->lumppat[0];
	patch = W_CachePatchNum(lump, PU_CACHE);
	
	if (((consvar_t*) (currentMenu->menuitems[4].itemaction))->value == 0)
		colormap = colormaps;
	else
		colormap = (uint8_t*)translationtables - 256 + (((consvar_t*) (currentMenu->menuitems[4].itemaction))->value << 8);
		
	V_DrawMappedPatch(((currentMenu->x + currentMenu->width) - patch->width) + patch->leftoffset,
	                  ((currentMenu->y + currentMenu->height) - patch->height) + patch->topoffset, 0, patch, colormap);
	                  
	M_DrawGenericMenu();
	
	// Draw Skin Value
	skinchar = PROF_GetNumber(((consvar_t*) (currentMenu->menuitems[1].itemaction))->value)->cvars[PC_SKIN].string;
	
	V_DrawStringA(VFONT_SMALL, VEX_MAP_WHITE, skinchar,
	              (currentMenu->x + currentMenu->width) - V_StringWidthA(VFONT_SMALL, 0, skinchar), (currentMenu->y + (5 * STRINGHEIGHT)));
}

void M_StartProfiler(int choice)
{
	CV_Set(&DynProfileChooser, "default");
	currentMenu->lastOn = itemOn;
	M_SetupNextMenu(&ProfileDef);
	
	if (!R_SkinAvailable(PROF_GetNumber(((consvar_t*) (currentMenu->menuitems[1].itemaction))->value)->cvars[PC_SKIN].string))
	{
		COM_BufAddText(va("%s \"%s\"", PROF_GetNumber(((consvar_t*) (currentMenu->menuitems[1].itemaction))->value)->cvars[PC_SKIN].name, "marine"));
	}
}

void PROF_M_ChangeProfile(void)
{
	ProfileInfo_t* Prof = PROF_FindProfile(DynProfileChooser.string);
	
	if (!Prof)
		Prof = Profiles;
		
	ProfileItems[3].itemaction = &(Prof->cvars[PC_NAME]);
	ProfileItems[4].itemaction = &(Prof->cvars[PC_COLOR]);
	ProfileItems[6].itemaction = &(Prof->cvars[PC_AUTOAIM]);
	//ProfileItems[5].itemaction = &(Prof->cvars[PC_SKIN]);
}

void M_HandleSkinChanger(int choice)
{
	int l;
	bool_t exitmenu = false;	// exit to previous menu and send name change
	int myskin;
	
	myskin = R_SkinAvailable(PROF_GetNumber(((consvar_t*) (currentMenu->menuitems[1].itemaction))->value)->cvars[PC_SKIN].string);
	
	switch (choice)
	{
		case KEY_DOWNARROW:
			S_StartSound(NULL, sfx_pstop);
			if (itemOn + 1 >= ProfileDef.numitems)
				itemOn = 0;
			else
				itemOn++;
			break;
		case KEY_UPARROW:
			S_StartSound(NULL, sfx_pstop);
			if (!itemOn)
				itemOn = ProfileDef.numitems - 1;
			else
				itemOn--;
			break;
		case KEY_LEFTARROW:
			S_StartSound(NULL, sfx_stnmov);
			myskin--;
			break;
		case KEY_RIGHTARROW:
			S_StartSound(NULL, sfx_stnmov);
			myskin++;
			break;
		case KEY_ENTER:
			S_StartSound(NULL, sfx_stnmov);
			myskin++;
			break;
		case KEY_BACKSPACE:
			S_StartSound(NULL, sfx_swtchn);
			exitmenu = true;
			break;
		case KEY_ESCAPE:
			S_StartSound(NULL, sfx_swtchx);
			exitmenu = true;
			break;
		default:
			return;
	}
	
	// Wrap Skin Value
	if (myskin < 0)
		myskin = numskins - 1;
	else
	{
		if (myskin > numskins - 1)
			myskin = 0;
	}
	
	if (myskin != R_SkinAvailable(PROF_GetNumber(((consvar_t*) (currentMenu->menuitems[1].itemaction))->value)->cvars[PC_SKIN].string))
	{
		COM_BufAddText(va("%s \"%s\"", PROF_GetNumber(((consvar_t*) (currentMenu->menuitems[1].itemaction))->value)->cvars[PC_SKIN].name, skins[myskin].name));
	}
	
	if (exitmenu)
	{
		if (currentMenu->prevMenu)
			M_SetupNextMenu(currentMenu->prevMenu);
		else
			M_ClearMenus(true);
	}
}

/******************************************************************************/

/*                               PROFILE PROMPT                               */

/******************************************************************************/

//InGameProfileChooser

void M_ProfAccept(int choice);

char* PP_Solo = "for you.";
char* PP_P1of2 = "for Player 1 (Top Screen).";
char* PP_P2of2 = "for Player 2 (Bottom Screen).";
char* PP_P1of4 = "for Player 1 (Top-Left Screen).";
char* PP_P2of4 = "for Player 2 (Top-Right Screen).";
char* PP_P3of4 = "for Player 3 (Bottom-Left Screen).";
char* PP_P4of4 = "for Player 4 (Bottom-Right Screen).";

menuitem_t ProfileChooserItems[] =
{
	{
		IT_SPACE | IT_STRING | IT_CENTERSTRING, NULL,
		PTROFUNICODESTRING(DSTR_MENUSELECTPROFILE_PLEASESELECT), NULL
	},
	{
		IT_SPACE | IT_STRING | IT_CENTERSTRING, NULL,
		PTROFUNICODESTRING(DSTR_MENUSELECTPROFILE_PLACEHOLDER), NULL
	},
	{IT_STRING | IT_CVAR, NULL, PTROFUNICODESTRING(DSTR_MENUSELECTPROFILE_PROFILE), &InGameProfileChooser},
	{IT_WHITESTRING | IT_CALL, NULL, PTROFUNICODESTRING(DSTR_MENUSELECTPROFILE_ACCEPT), M_ProfAccept},
};

menu_t ProfileChooser =
{
	0,
	NULL,
	PTROFUNICODESTRING(DSTR_MENUSELECTPROFILE_TITLE),
	sizeof(ProfileChooserItems) / sizeof(menuitem_t),
	ProfileChooserItems,
	NULL,
	M_DrawGenericMenu,
	BASEVIDWIDTH >> 2,			// X
	(BASEVIDHEIGHT >> 1) - (STRINGHEIGHT << 1),	// Y
	BASEVIDWIDTH >> 1,			// W
	STRINGHEIGHT << 2,			// H
	1,
};

player_t* ModPlayer = NULL;

/* M_ProfilePrompt() -- When adding new local players, they have to have a
                        profile linked to them */
void M_ProfilePrompt(int player)
{
	player_t* Player = NULL;
	int i;
	int OK = 0;
	
	// Check to see if our player is a split screen player and that they are playing
	for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
		if (playeringame[consoleplayer[i]] && consoleplayer[i] == player)
		{
			OK = 1;
			Player = &players[consoleplayer[i]];
			ModPlayer = Player;
			break;
		}
	// Nope
	if (!OK || !Player)
		return;
		
	// Variable Message
	if (cv_splitscreen.value == 1)
	{
		if (i == 0)
			ProfileChooserItems[1].WItemTextPtr = PTROFUNICODESTRING(DSTR_MENUSELECTPROFILE_TWOSPLITA);
		else
			ProfileChooserItems[1].WItemTextPtr = PTROFUNICODESTRING(DSTR_MENUSELECTPROFILE_TWOSPLITB);
	}
	else if (cv_splitscreen.value > 1)
	{
		if (i == 0)
			ProfileChooserItems[1].WItemTextPtr = PTROFUNICODESTRING(DSTR_MENUSELECTPROFILE_FOURSPLITA);
		else if (i == 1)
			ProfileChooserItems[1].WItemTextPtr = PTROFUNICODESTRING(DSTR_MENUSELECTPROFILE_FOURSPLITB);
		else if (i == 2)
			ProfileChooserItems[1].WItemTextPtr = PTROFUNICODESTRING(DSTR_MENUSELECTPROFILE_FOURSPLITC);
		else
			ProfileChooserItems[1].WItemTextPtr = PTROFUNICODESTRING(DSTR_MENUSELECTPROFILE_FOURSPLITD);
	}
	else
		ProfileChooserItems[1].WItemTextPtr = PTROFUNICODESTRING(DSTR_MENUSELECTPROFILE_FORYOU);
		
	// GhostlyDeath <August 3, 2011> -- if there is only one profile, autochoose that
	if (PROF_NumProfiles() == 1)
	{
		M_ProfAccept(1);
		return;
	}
	
	M_StartControlPanel();
	if (currentMenu)
		currentMenu->lastOn = itemOn;
	itemOn = 1;
	M_SetupNextMenu(&ProfileChooser);
}

void M_ProfAccept(int choice)
{
	ProfileInfo_t* Rover = Profiles;
	int i;
	
	if (!ModPlayer)
		return;
		
	while (Rover)
	{
		if (strcasecmp(InGameProfileChooser.string, Rover->name) == 0)
		{
			ModPlayer->profile = Rover;
			
			// Change Name
			for (i = 0; i < cv_splitscreen.value + 1; i++)
				if (playeringame[consoleplayer[i]] && ModPlayer == &players[consoleplayer[i]])
				{
					strncpy(player_names[consoleplayer[i]], Rover->cvars[PC_NAME].string, MAXPLAYERNAME);
					break;
				}
			// Change Color
			ModPlayer->skincolor = Rover->cvars[PC_COLOR].value % MAXSKINCOLORS;
			
			if (ModPlayer->mo)
			{
				ModPlayer->mo->flags &= ~MF_TRANSLATION;
				ModPlayer->mo->flags |= (ModPlayer->skincolor) << MF_TRANSSHIFT;
			}
			// Change Skin
			SetPlayerSkin(consoleplayer[i], Rover->cvars[PC_SKIN].string);
			
			// Change autoaim
			ModPlayer->autoaim_toggle = Rover->cvars[PC_AUTOAIM].value;
			break;
		}
		else
			Rover = Rover->next;
	}
	
	M_ClearMenus(false);
}
