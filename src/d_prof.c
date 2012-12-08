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
#include "g_game.h"
#include "dstrings.h"
#include "s_sound.h"
#include "p_local.h"
#include "d_main.h"

/************************
*** EXTENDED PROFILES ***
************************/

/*** CONSTANTS ***/
static const struct
{
	const char ShortName[16];					// Short Name
	const char LongName[32];					// Long Name (menus)
	D_ProfileExInputCtrl_t ID;					// For Reference
} c_ControlMapper[NUMDPROFILEEXINPUTCTRLS] =
{
	{"null", "Nothing", DPEXIC_NULL},
	
	{"modspeed", "Speed Modifier", DPEXIC_SPEED},
	{"modmove", "Movement Modifier", DPEXIC_MOVEMENT},
	{"modlook", "Look Modifier", DPEXIC_LOOKING},
	
	{"forwards", "Move Forwards", DPEXIC_FORWARDS},
	{"backwards", "Move Backwards", DPEXIC_BACKWARDS},
	{"strafeleft", "Strafe Left", DPEXIC_STRAFELEFT},
	{"straferight", "Strafe Right", DPEXIC_STRAFERIGHT},
	{"flyup", "Fly Up", DPEXIC_FLYUP},
	{"flydown", "Fly Down", DPEXIC_FLYDOWN},
	{"land", "Land", DPEXIC_LAND},
	{"jump", "Jump", DPEXIC_JUMP},
	
	/* Looking */
	{"turnleft", "Turn Left", DPEXIC_TURNLEFT},
	{"turnright", "Turn Right", DPEXIC_TURNRIGHT},
	{"turn180", "Turn 180\xC2\xb0", DPEXIC_TURNSEMICIRCLE},
	{"lookup", "Look Up", DPEXIC_LOOKUP},
	{"lookdown", "Look Down", DPEXIC_LOOKDOWN},
	{"lookcenter", "Center View", DPEXIC_LOOKCENTER},
	
	/* Actions */
	{"use", "Use Action", DPEXIC_USE},
	{"suicide", "Commit Suicide", DPEXIC_SUICIDE},
	{"taunt", "Taunt", DPEXIC_TAUNT},
	{"chat", "Chat", DPEXIC_CHAT},
	{"teamchat", "Chat With Team", DPEXIC_TEAMCHAT},
	
	/* Weapons */
	{"attack", "Attack", DPEXIC_ATTACK},
	{"altattack", "Secondary Attack", DPEXIC_ALTATTACK},
	{"reload", "Reload Weapon", DPEXIC_RELOAD},
	{"switchfire", "Switch Firing Mode", DPEXIC_SWITCHFIREMODE},
	{"slot1", "Weapon Slot 1", DPEXIC_SLOT1},
	{"slot2", "Weapon Slot 2", DPEXIC_SLOT2},
	{"slot3", "Weapon Slot 3", DPEXIC_SLOT3},
	{"slot4", "Weapon Slot 4", DPEXIC_SLOT4},
	{"slot5", "Weapon Slot 5", DPEXIC_SLOT5},
	{"slot6", "Weapon Slot 6", DPEXIC_SLOT6},
	{"slot7", "Weapon Slot 7", DPEXIC_SLOT7},
	{"slot8", "Weapon Slot 8", DPEXIC_SLOT8},
	{"slot9", "Weapon Slot 9", DPEXIC_SLOT9},
	{"slot10", "Weapon Slot 10", DPEXIC_SLOT10},
	{"nextweapon", "Next Weapon", DPEXIC_NEXTWEAPON},
	{"prevweapon", "Previous Weapon", DPEXIC_PREVWEAPON},
	{"bestweapon", "Best Weapon", DPEXIC_BESTWEAPON},
	{"worstweapon", "Worst Weapon", DPEXIC_WORSTWEAPON},
	
	/* Inventory */
	{"nextinventory", "Inventory Cursor Next", DPEXIC_NEXTINVENTORY},
	{"previnventory", "Inventory Cursor Previous", DPEXIC_PREVINVENTORY},
	{"useinventory", "Use Inventory Item", DPEXIC_USEINVENTORY},
	{"cancelinventory", "Cancel Inventory Selection", DPEXIC_CANCELINVENTORY},
	
	/* General */
	{"topscores", "Show Top Scores", DPEXIC_TOPSCORES},
	{"worstscores", "Show Worst Scores", DPEXIC_BOTTOMSCORES},
	{"coopspy", "Switch Cooperative Spy Player", DPEXIC_COOPSPY},
	{"automap",	"Toggle the Automap", DPEXIC_AUTOMAP},
	{"chatmode", "Enter Chat Mode", DPEXIC_CHATMODE},
	{"popupmenu", "Show the Menu", DPEXIC_POPUPMENU},
	{"morestuff", "More Commands Modifier", DPEXIC_MORESTUFF},
};

/* c_AxisMap -- Map of axis names */
static const char* const c_AxisMap[NUMDPROFILEEXCTRLMAS] =
{
	"null",										// DPEXCMA_NULL
	"movex",									// DPEXCMA_MOVEX
	"movey",									// DPEXCMA_MOVEY,
	"lookx",									// DPEXCMA_LOOKX
	"looky",									// DPEXCMA_LOOKY,
	
	"negmovex",									// DPEXCMA_NEGMOVEX
	"negmovey",									// DPEXCMA_NEGMOVEY,
	"neglookx",									// DPEXCMA_NEGLOOKX,
	"neglooky",									// DPEXCMA_NEGLOOKY,
};

/* c_ProfDataStat -- Simplified config space */
static const struct
{
	const char ArgName[16];
	size_t Offset;
	uint16_t Size;
} c_ProfDataStat[] =
{
#define QUICKDS(x,s) {#x, offsetof(D_ProfileEx_t, x), s}
	QUICKDS(Color, 8),
	QUICKDS(JoyControl, 8),
	QUICKDS(SlowTurnTime, 717),
	QUICKDS(MouseSens[0], 32),
	QUICKDS(MouseSens[1], 32),
	QUICKDS(JoySens[0], 32),
	QUICKDS(JoySens[1], 32),
	QUICKDS(LookUpDownSpeed, 32),
	QUICKDS(AutoGrabJoy, 32),
	QUICKDS(ColorPickup, 8),
	QUICKDS(ColorSecret, 8),
	QUICKDS(SoundSecret, 5555),
	QUICKDS(DrawPSprites, 1010),
	QUICKDS(BobMode, 8),
	QUICKDS(ViewHeight, 3232),
	QUICKDS(CamDist, 3232),
	QUICKDS(CamHeight, 3232),
	QUICKDS(CamSpeed, 3232),
	QUICKDS(ChaseCam, 1010),
	QUICKDS(TransSBar, 1010),
	QUICKDS(ScaledSBar, 1010),
	
	{"", 0, 0},
#undef QUICKDS
};

/*** GLOBALS ***/

D_ProfileEx_t* g_KeyDefaultProfile = NULL;		// Profile with our key defaults

/*** LOCALS ***/

static D_ProfileEx_t* l_FirstProfile = NULL;	// First in chain

static bool_t l_DefaultCtrlsMapped = false;
static D_ProfileExCtrlMA_t l_DefaultMouseAxis[MAXALTAXIS][MAXMOUSEAXIS];		// Mouse Axis Movement
static D_ProfileExCtrlMA_t l_DefaultJoyAxis[MAXALTAXIS][MAXJOYAXIS];	// Joy Axis Movement
static uint32_t l_DefaultCtrls[NUMDPROFILEEXINPUTCTRLS][4];

/*** FUNCTIONS ***/

/* D_CreateProfileEx() -- Create Profile */
D_ProfileEx_t* D_CreateProfileEx(const char* const a_Name)
{
	D_ProfileEx_t* New;
	size_t i;
	char Char;
	
	/* Check */
	if (!a_Name)
		return NULL;
	
	/* Allocate */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Set properties */
	// UUID (hopefully random)
	D_CMakeUUID(New->UUID);
	
	// First character is never random
	New->UUID[0] = a_Name[0];
	
	// Instance ID (multiplayer)
	New->InstanceID = D_CMakePureRandom();
	
	/* Copy Name */
	strncpy(New->AccountName, a_Name, MAXPLAYERNAME - 1);
	strncpy(New->DisplayName, a_Name, MAXPLAYERNAME - 1);
	
	/* Set Default Options */
	New->Flags |= DPEXF_GOTMOUSE | DPEXF_GOTJOY | DPEXF_SLOWTURNING;
	New->SlowTurnTime = 6;
	
	// Default Controls (First Time)
	if (!l_DefaultCtrlsMapped)
	{
#define SETKEY_M(a,b) a##b
#define SETKEY(c,k) l_DefaultCtrls[SETKEY_M(DPEXIC_,c)][0] = (SETKEY_M(IKBK_,k))
#define SETJOY(c,b) l_DefaultCtrls[SETKEY_M(DPEXIC_,c)][3] = 0x1000 | ((b) - 1)
#define SETJOYMORE(c,b) l_DefaultCtrls[SETKEY_M(DPEXIC_,c)][3] = 0x4000 | ((b) - 1)
#define SETMOUSE(c,b) l_DefaultCtrls[SETKEY_M(DPEXIC_,c)][1] = 0x2000 | ((b) - 1)
#define SETDBLMOUSE(c,b) l_DefaultCtrls[SETKEY_M(DPEXIC_,c)][2] = 0x3000 | ((b) - 1)
		
		// Default Controls
		if (g_ModelMode == DMM_DEFAULT)
		{
			SETMOUSE(ATTACK, 1);
			SETMOUSE(MOVEMENT, 2);
			SETMOUSE(PREVWEAPON, 5);
			SETMOUSE(NEXTWEAPON, 4);
			SETDBLMOUSE(USE, 2);
			
			SETKEY(SPEED, SHIFT);
			SETKEY(MOVEMENT, ALT);
			SETKEY(LOOKING, S);
			SETKEY(FORWARDS, UP);
			SETKEY(BACKWARDS, DOWN);
			SETKEY(STRAFELEFT, COMMA);
			SETKEY(STRAFERIGHT, PERIOD);
			SETKEY(JUMP, FORWARDSLASH);
			SETKEY(LAND, HOME);
			SETKEY(TURNSEMICIRCLE, BACKSLASH);
			SETKEY(TURNLEFT, LEFT);
			SETKEY(TURNRIGHT, RIGHT);
			SETKEY(LOOKUP, PAGEUP);
			SETKEY(LOOKDOWN, PAGEDOWN);
			SETKEY(LOOKCENTER, END);
			SETKEY(USE, SPACE);
			SETKEY(TAUNT, U);
			SETKEY(CHAT, T);
			SETKEY(TEAMCHAT, Y);
			SETKEY(ATTACK, CTRL);
			SETKEY(RELOAD, R);
			SETKEY(SLOT1, 1);
			SETKEY(SLOT2, 2);
			SETKEY(SLOT3, 3);
			SETKEY(SLOT4, 4);
			SETKEY(SLOT5, 5);
			SETKEY(SLOT6, 6);
			SETKEY(SLOT7, 7);
			SETKEY(SLOT8, 8);
			SETKEY(SLOT9, 9);
			SETKEY(SLOT10, 0);
			SETKEY(PREVWEAPON, LEFTBRACKET);
			SETKEY(NEXTWEAPON, RIGHTBRACKET);
			SETKEY(PREVINVENTORY, SEMICOLON);
			SETKEY(NEXTINVENTORY, COLON);
			SETKEY(USEINVENTORY, RETURN);
			SETKEY(FLYUP, INSERT);
			SETKEY(FLYDOWN, KDELETE);
			SETKEY(TOPSCORES, F);
			SETKEY(COOPSPY, F12);
			SETKEY(AUTOMAP, TAB);
	
			// Joystick Buttons
			SETJOY(ATTACK, 1);
			SETJOY(USE, 2);
			SETJOY(MOVEMENT, 3);
			SETJOY(SPEED, 4);
		
			SETJOY(STRAFELEFT, 5);
			SETJOY(STRAFERIGHT, 6);
		
			SETJOY(PREVWEAPON, 7);
			SETJOY(NEXTWEAPON, 8);
		
			SETJOY(MORESTUFF, 9);
			SETJOY(POPUPMENU, 10);
		
			SETJOY(JUMP, 11);
		
			// More Joystick Buttons (with more key)
			SETJOYMORE(TOPSCORES, 1);
			SETJOYMORE(COOPSPY, 2);
			SETJOYMORE(AUTOMAP, 3);
			SETJOYMORE(USEINVENTORY, 4);
		
			SETJOYMORE(RELOAD, 5);
			SETJOYMORE(SWITCHFIREMODE, 6);
		
			SETJOYMORE(PREVINVENTORY, 7);
			SETJOYMORE(NEXTINVENTORY, 8);
		
			SETJOYMORE(CHATMODE, 10);
			SETJOYMORE(SUICIDE, 11);
		}
		
		// GCW Zero
		else if (g_ModelMode == DMM_GCW)
		{
			SETKEY(FORWARDS, UP);
			SETKEY(BACKWARDS, DOWN);
			SETKEY(TURNLEFT, LEFT);
			SETKEY(TURNRIGHT, RIGHT);
			
			SETKEY(ATTACK, SPACE);
			SETKEY(USE, CTRL);
			SETKEY(MOVEMENT, ALT);
			SETKEY(SPEED, SHIFT);
		
			SETKEY(STRAFELEFT, TAB);
			SETKEY(STRAFERIGHT, BACKSPACE);
			
			SETKEY(NEXTWEAPON, PAUSE);
			
			SETKEY(QUICKMENU, RETURN);
		}
		
#undef SETJOY
#undef SETKEY_M
#undef SETKEY
		
		// Mouse Axis
			// Not ALT
		l_DefaultMouseAxis[0][0] = DPEXCMA_LOOKX;
		l_DefaultMouseAxis[0][1] = DPEXCMA_MOVEY;
			// ALT
		l_DefaultMouseAxis[1][0] = DPEXCMA_MOVEX;
		l_DefaultMouseAxis[1][1] = DPEXCMA_MOVEY;
			// Mouse Look (Default 'S')
		l_DefaultMouseAxis[2][0] = DPEXCMA_LOOKX;
		l_DefaultMouseAxis[2][1] = DPEXCMA_LOOKY;
	
		// Joystick Axis
			// Not ALT
		l_DefaultJoyAxis[0][0] = DPEXCMA_LOOKX;
		l_DefaultJoyAxis[0][1] = DPEXCMA_MOVEY;
			// ALT
		l_DefaultJoyAxis[1][0] = DPEXCMA_MOVEX;
		l_DefaultJoyAxis[1][1] = DPEXCMA_MOVEY;
			// Mouse Look (Default 'S')
		l_DefaultJoyAxis[2][0] = DPEXCMA_LOOKX;
		l_DefaultJoyAxis[2][1] = DPEXCMA_NEGLOOKY;
	
		// Now set
		l_DefaultCtrlsMapped = true;
	}
	
	// Copy directly from defaults
	memmove(New->Ctrls, l_DefaultCtrls, sizeof(l_DefaultCtrls));
	memmove(New->MouseAxis, l_DefaultMouseAxis, sizeof(l_DefaultMouseAxis));
	memmove(New->JoyAxis, l_DefaultJoyAxis, sizeof(l_DefaultJoyAxis));

	// Default Sensitivities
	New->MouseSens[0] = New->MouseSens[1] = 10;
	New->JoySens[0] = New->JoySens[1] = 100;
	New->LookUpDownSpeed = (1 << 25);
	
	// Default Colors
	New->ColorPickup = VEX_MAP_WHITE;
	New->ColorSecret = VEX_MAP_BRIGHTWHITE;
	
	// Default Sounds
	New->SoundSecret = sfx_secret;
	
	// Default other options
	New->DrawPSprites = true;
	New->BobMode = 1;							// Middle bobbing mode
	New->ViewHeight = VIEWHEIGHT << FRACBITS;	// Player View Height
	New->ChaseCam = false;						// Enable chase cam
	New->CamDist = 128 << FRACBITS;				// Camera Distance (default)
	New->CamHeight = 20 << FRACBITS;			// Camera Height
	New->CamSpeed = 16384;						// Camera Speed
	New->TransSBar = false;						// Transparent status bar
	New->ScaledSBar = false;					// Scaled status bar
	
	/* Link */
	if (!l_FirstProfile)
		l_FirstProfile = New;
	else
	{
		New->Next = l_FirstProfile;
		l_FirstProfile->Prev = New;
		l_FirstProfile = New;
	}
	
	/* Key defaults unset? */
	if (!g_KeyDefaultProfile)
		g_KeyDefaultProfile = New;
	
	/* Return the new one */
	return New;
}

/* D_FindProfileEx() -- Locates a profile */
D_ProfileEx_t* D_FindProfileEx(const char* const a_Name)
{
	D_ProfileEx_t* Rover;
	
	/* Check */
	if (!a_Name)
		return NULL;
	
	/* Rove */
	for (Rover = l_FirstProfile; Rover; Rover = Rover->Next)
	{
		// UUID Match?
		if (strcmp(Rover->UUID, a_Name) == 0)
			return Rover;
		
		// Account Match?
		else if (strcasecmp(Rover->AccountName, a_Name) == 0)
			return Rover;
	}
	
	/* Not found */
	return NULL;
}

/* D_FindProfileExByInstance() -- Find profile instance ID */
D_ProfileEx_t* D_FindProfileExByInstance(const uint32_t a_ID)
{
	D_ProfileEx_t* Rover;
	
	/* Rove */
	for (Rover = l_FirstProfile; Rover; Rover = Rover->Next)
		if (Rover->InstanceID == a_ID)
			return Rover;
	
	/* Not found */
	return NULL;
}

/* DS_KeyCodeToStr() -- Converts a key code to a string */
static void DS_KeyCodeToStr(char* const a_Dest, const size_t a_Size, const uint32_t a_Code)
{
	/* Check */
	if (!a_Dest || !a_Size)
		return;
	
	/* Nothing */
	if (!a_Code)
		snprintf(a_Dest, a_Size, "---");
	
	/* Joystick */
	else if (a_Code & 0x1000)
		snprintf(a_Dest, a_Size, "joyb%02i", (int)((a_Code & 0xFFF) - 1));
	
	/* Mouse */
	else if (a_Code & 0x2000)
		snprintf(a_Dest, a_Size, "mouseb%02i", (int)((a_Code & 0xFFF) - 1));
	
	/* Double Mouse */
	else if (a_Code & 0x3000)
		snprintf(a_Dest, a_Size, "dblmouseb%02i", (int)((a_Code & 0xFFF) - 1));
		
	/* More Joystick */
	else if (a_Code & 0x4000)
		snprintf(a_Dest, a_Size, "morejoyb%02i", (int)((a_Code & 0xFFF) - 1));
	
	/* More Mouse */
	else if (a_Code & 0x5000)
		snprintf(a_Dest, a_Size, "moremouseb%02i", (int)((a_Code & 0xFFF) - 1));
	
	/* More Double Mouse */
	else if (a_Code & 0x6000)
		snprintf(a_Dest, a_Size, "moredblmouseb%02i", (int)((a_Code & 0xFFF) - 1));
	
	/* Keyboard */
	else if (a_Code >= 0 && a_Code < NUMIKEYBOARDKEYS)
		snprintf(a_Dest, a_Size, "%s", c_KeyNames[a_Code][0]);
	
	/* Illegal */
	else
		snprintf(a_Dest, a_Size, "---");
}

/* DS_KeyStrToCode() -- Converts string to key code */
static uint32_t DS_KeyStrToCode(const char* const a_Str)
{
	int i;
	
	/* Check */
	if (!a_Str)
		return 0;
	
	/* Illegal/NULL Key */
	if (strcasecmp(a_Str, "---") == 0)
		return 0;
	
	/* Joystick Buttons */
	else if (strncasecmp(a_Str, "joyb", 4) == 0)
		return 0x1000 | (C_strtou32(a_Str + 4, NULL, 10) + 1);
	
	/* Mouse Buttons */
	else if (strncasecmp(a_Str, "mouseb", 6) == 0)
		return 0x2000 | (C_strtou32(a_Str + 6, NULL, 10) + 1);
	
	/* Double Mouse Buttons */
	else if (strncasecmp(a_Str, "dblmouseb", 9) == 0)
		return 0x3000 | (C_strtou32(a_Str + 9, NULL, 10) + 1);
		
	/* More Joystick Buttons */
	else if (strncasecmp(a_Str, "morejoyb", 8) == 0)
		return 0x4000 | (C_strtou32(a_Str + 4, NULL, 10) + 1);
	
	/* More Mouse Buttons */
	else if (strncasecmp(a_Str, "moremouseb", 10) == 0)
		return 0x5000 | (C_strtou32(a_Str + 6, NULL, 10) + 1);
	
	/* More Double Mouse Buttons */
	else if (strncasecmp(a_Str, "moredblmouseb", 13) == 0)
		return 0x6000 | (C_strtou32(a_Str + 9, NULL, 10) + 1);
	
	/* Keyboard Keys */
	else
	{
		for (i = 0; i < NUMIKEYBOARDKEYS; i++)
			if (strcasecmp(a_Str, c_KeyNames[i][0]) == 0)
				return i;
	}
}

/* DS_ReloadValue() -- Reloads value into profile */
static void DS_ReloadValue(D_ProfileEx_t* const a_Profile, const char* const a_Option, const char* const a_Value)
{
	int i;
	void* Ptr;
	
	/* Check */
	if (!a_Profile || !a_Option || !a_Value)
		return;
	
	/* Find Named Option */
	for (i = 0; c_ProfDataStat[i].ArgName[0]; i++)
		if (strcasecmp(a_Option, c_ProfDataStat[i].ArgName) == 0)
			break;
	
	// Not found?
	if (!c_ProfDataStat[i].ArgName[0])
		return;
	
	/* Get offset */
	Ptr = (void*)((uintptr_t)a_Profile + c_ProfDataStat[i].Offset);
	
	/* Based on Size */
	switch (c_ProfDataStat[i].Size)
	{
		case 8: *((uint8_t*)Ptr) = C_strtou32(a_Value, NULL, 10); break;
		case 16: *((uint16_t*)Ptr) = C_strtou32(a_Value, NULL, 10); break;
		case 32: *((uint32_t*)Ptr) = C_strtou32(a_Value, NULL, 10); break;
		case 717: *((tic_t*)Ptr) = C_strtou32(a_Value, NULL, 10); break;
		case 3232: *((fixed_t*)Ptr) = FLOAT_TO_FIXED(atof(a_Value)); break;
		
		case 1010:
			if (strcasecmp(a_Value, "true") == 0 || strcasecmp(a_Value, "yes") == 0)
				*((bool_t*)Ptr) = true;
			else
				*((bool_t*)Ptr) = false;
			break;
		
		case 5555:
			for (i = 0; i < NUMSFX; i++)
				if (strcasecmp(a_Value, S_sfx[i].name) == 0)
				{
					*((int32_t*)Ptr) = i;
					break;
				}
			break;
	}
}

/* DS_SizeToStr() -- Converts sized argument to a string */
static void DS_SizeToStr(void* const a_Ptr, const uint16_t a_Size, char* const a_Buf, const size_t a_BufSize)
{
	switch (a_Size)
	{
		case 8: snprintf(a_Buf, a_BufSize, "%i", *((uint8_t*)a_Ptr)); break;
		case 16: snprintf(a_Buf, a_BufSize, "%i", *((uint16_t*)a_Ptr)); break;
		case 32: snprintf(a_Buf, a_BufSize, "%i", *((uint32_t*)a_Ptr)); break;
		
		case 3232: snprintf(a_Buf, a_BufSize, "%f", FIXED_TO_FLOAT(*((fixed_t*)a_Ptr))); break;
		case 717: snprintf(a_Buf, a_BufSize, "%li", *((tic_t*)a_Ptr)); break;
		case 1010: snprintf(a_Buf, a_BufSize, "%s", (*((bool_t*)a_Ptr) ? "true" : "false")); break;
		case 5555: snprintf(a_Buf, a_BufSize, "%s", S_sfx[*((int32_t*)a_Ptr)].name); break;
		
		default: snprintf(a_Buf, a_BufSize, "0"); break;
	}
}

/* D_SaveProfileData() -- Saves profile data */
void D_SaveProfileData(void (*a_WriteBack)(const char* const a_Buf, void* const a_Data), void* const a_Data)
{
#define BUFSIZE 256
	char Buf[BUFSIZE];
	char BufB[BUFSIZE];
	char EscapeUUID[BUFSIZE];
	D_ProfileEx_t* Rover;
	int i, j, k;
	
	/* Check */
	if (!a_WriteBack)
		return;
	
	/* Start Header */
	a_WriteBack("\n// Begin Profiles (edit at your own risk)\n", a_Data);
	
	/* Go through every profile */
	for (Rover = l_FirstProfile; Rover; Rover = Rover->Next)
	{
		// Skip ones marked DO NOT SAVE
		if (Rover->Flags & DPEXF_DONTSAVE)
			continue;
		
		// Escape the Profile Name
		memset(EscapeUUID, 0, sizeof(EscapeUUID));
		CONL_EscapeString(EscapeUUID, BUFSIZE, Rover->AccountName);
		
		// Mark profile creation
		memset(BufB, 0, sizeof(BufB));
		CONL_EscapeString(BufB, BUFSIZE, Rover->UUID);
		memset(Buf, 0, sizeof(Buf));
		snprintf(Buf, BUFSIZE, "profile create \"%s\" \"%s\"\n", EscapeUUID, BufB);
		a_WriteBack(Buf, a_Data);
		
		// Write Profile Data
		for (i = 0; c_ProfDataStat[i].ArgName[0]; i++)
		{
			// Value
			memset(BufB, 0, sizeof(BufB));
			DS_SizeToStr((void*)((uintptr_t)Rover + c_ProfDataStat[i].Offset), c_ProfDataStat[i].Size, BufB, BUFSIZE);
			
			// Write
			snprintf(Buf, BUFSIZE, "profile value \"%s\" \"%s\" \"%s\"\n", EscapeUUID, c_ProfDataStat[i].ArgName, BufB);
			a_WriteBack(Buf, a_Data);
		}
		
		// Write Mouse/Joy Axis
		for (i = 0; i < MAXALTAXIS; i++)
		{
			// Mouse
			for (j = 0; j < MAXMOUSEAXIS; j++)
			{
				// If not the default, change
				if (Rover->MouseAxis[i][j] == l_DefaultMouseAxis[i][j])
					continue;
					
				// Write Axis
				snprintf(Buf, BUFSIZE, "profile maxis \"%s\" %i %i \"%s\"\n",
						EscapeUUID,
						i,
						j,
						c_AxisMap[Rover->MouseAxis[i][j]]
					);
				a_WriteBack(Buf, a_Data);
			}
			
			// Joystick
			for (j = 0; j < MAXJOYAXIS; j++)
			{
				// If not the default, change
				if (Rover->JoyAxis[i][j] == l_DefaultJoyAxis[i][j])
					continue;
					
				// Write Axis
				snprintf(Buf, BUFSIZE, "profile jaxis \"%s\" %i %i \"%s\"\n",
						EscapeUUID,
						i,
						j,
						c_AxisMap[Rover->JoyAxis[i][j]]
					);
				a_WriteBack(Buf, a_Data);
			}
		}
		
		// Write Controls
		for (i = 0; i < NUMDPROFILEEXINPUTCTRLS; i++)
			for (j = 0; j < 4; j++)
			{
				// If the key does not match the default then save it.
				// Otherwise don't save it (since this fills the config
				// file up to insane proportions.
				if (Rover->Ctrls[i][j] == l_DefaultCtrls[i][j])
					continue;
				
				// Convert Key to String
				DS_KeyCodeToStr(BufB, BUFSIZE, Rover->Ctrls[i][j]);
				
				// Write Key
				snprintf(Buf, BUFSIZE, "profile control \"%s\" \"%s\" %i \"%s\"\n",
						EscapeUUID,
						c_ControlMapper[i].ShortName,
						j,
						BufB
					);
				a_WriteBack(Buf, a_Data);
			}
		
		// Spacer
		a_WriteBack("\n", a_Data);
	}
	
	/* End Header */
	a_WriteBack("// End Profiles\n", a_Data);
#undef BUFSIZE
}

/* CLC_Profile() -- Profile command handler */
int CLC_Profile(const uint32_t a_ArgC, const char** const a_ArgV)
{
#define BUFSIZE 256
	char BufA[BUFSIZE];
	char BufB[BUFSIZE];
	D_ProfileEx_t* New;
	int i, j, k;
	D_ProfileExCtrlMA_t* TMA;
	
	/* Not enough arguments? */
	if (a_ArgC < 3)
		return 1;
		
	/* Clear Buffers */
	memset(BufA, 0, sizeof(BufA));
	memset(BufB, 0, sizeof(BufB));
	
	/* Which Sub Command */
	// Create Profile
	if (strcasecmp(a_ArgV[1], "create") == 0)
	{
		// Read Name
		CONL_UnEscapeString(BufA, BUFSIZE, a_ArgV[2]);
		
		// Possibly read UUID
		New = NULL;
		if (a_ArgC >= 4)
		{
			CONL_UnEscapeString(BufB, BUFSIZE, a_ArgV[3]);
			
			// Check UUID existence
			New = D_FindProfileEx(BufB);
		}
		
		// See if it already exists
		if (!New)
			New = D_FindProfileEx(BufA);
		
		// Exists?
		if (New)
		{
			CONL_OutputU(DSTR_DPROFC_ALREADYEXIST, "%s\n", BufA);
			return 1;
		}
		
		// Create Profile
		New = D_CreateProfileEx(BufA);
		
		// Failed??
		if (!New)
		{
			CONL_OutputU(DSTR_DPROFC_FAILEDCREATE, "\n");
			return 1;
		}
		
		// Set UUID if preformed
		if (BufB[0])
			strncpy(New->UUID, BufB, MAXPROFILEUUID);
		
		// Done
		return true;
	}
	
	/* After this all values are mostly the same */
	// Read Name
	CONL_UnEscapeString(BufA, BUFSIZE, a_ArgV[2]);
	
	// Find profile
	New = D_FindProfileEx(BufA);
	
	// Not found?
	if (!New)
	{
		CONL_OutputU(DSTR_DPROFC_NOTFOUND, "%s\n", BufA);
		return 1;
	}
	
	// Change Value
	if (strcasecmp(a_ArgV[1], "value") == 0)
	{
		// Usage?
		if (a_ArgC < 5)
		{
			CONL_OutputU(DSTR_DPROFC_VALUEUSAGE, "%s\n", a_ArgV[0]);
			return 1;
		}
		
		DS_ReloadValue(New, a_ArgV[3], a_ArgV[4]);
	}
	
	// Control
	else if (strcasecmp(a_ArgV[1], "control") == 0)
	{
		// Usage?
		if (a_ArgC < 6)
		{
			CONL_OutputU(DSTR_DPROFC_CONTROLUSAGE, "%s\n", a_ArgV[0]);
			return 1;
		}
		
		// Load Index
		i = C_strtou32(a_ArgV[4], NULL, 10);
		
		// Out of bounds?
		if (i < 0 || i >= 4)
		{
			CONL_OutputU(DSTR_DPROFC_INDEXOUTOFRANGE, "%i\n", i);
			return 1;
		}
		
		// Load Control Name
		for (k = 0; k < NUMDPROFILEEXINPUTCTRLS; k++)
			if (strcasecmp(a_ArgV[3], c_ControlMapper[k].ShortName) == 0)
				break;
		
		// Not found?
		if (k >= NUMDPROFILEEXINPUTCTRLS)
		{
			CONL_OutputU(DSTR_DPROFC_NOTCONTROLNAME, "%s\n", a_ArgV[3]);
			return 1;
		}
		
		// Back convert string to ID
		New->Ctrls[k][i] = DS_KeyStrToCode(a_ArgV[5]);
	}
	
	// Mouse/Joy Axis
	else if ((strcasecmp(a_ArgV[1], "maxis") == 0) || (strcasecmp(a_ArgV[1], "jaxis") == 0))
	{
		// Usage?
		if (a_ArgC < 6)
		{
			CONL_OutputU(DSTR_DPROFC_MAXISUSAGE, "%s\n", a_ArgV[0]);
			return 1;
		}
		
		// Obtain current grouping and control set
		i = C_strtou32(a_ArgV[3], NULL, 10);
		j = C_strtou32(a_ArgV[4], NULL, 10);
		
		// Alternate overflow?
		if (i < 0 || i >= MAXALTAXIS)
			return 1;
		
		// Clear
		TMA = NULL;
		
		// Modding Joy?
		if (a_ArgV[1][0] == 'j')
		{
			// Out of range?
			if (j < 0 || j >= MAXJOYAXIS)
				return 1;
			
			// Set
			TMA = &New->JoyAxis[i][j];
		}
		
		// Modding Mouse?
		else
		{
			// Out of range?
			if (j < 0 || j >= MAXMOUSEAXIS)
				return 1;
			
			// Set
			TMA = &New->MouseAxis[i][j];
		}
		
		// Find in list
		for (k = 0; k < NUMDPROFILEEXCTRLMAS; k++)
			if (strcasecmp(a_ArgV[5], c_AxisMap[k]) == 0)
			{
				*TMA = k;
				return 0;
			}
			
		// Failed?
		return 1;
	}
	
	return 0;
#undef BUFSIZE
}

