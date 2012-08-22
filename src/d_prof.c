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
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
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
#include "p_local.h"

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

/*** LOCALS ***/

static D_ProfileEx_t* l_FirstProfile = NULL;	// First in chain

static bool_t l_DefaultCtrlsMapped = false;
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
	// First character is never random
	New->UUID[0] = a_Name[0];
	
	// UUID (hopefully random)
	for (i = 1; i < (MAXPLAYERNAME * 2) - 1; i++)
	{
		// Hopefully random enough
		Char = (((int)(M_Random())) + ((int)I_GetTime() * (int)I_GetTime()));
		
		// Limit Char
		if (!((Char >= '0' && Char <= '9') || (Char >= 'a' && Char <= 'z') || (Char >= 'A' && Char <= 'Z')))
		{
			i--;
			continue;
		}
		
		// Set as
		New->UUID[i] = Char;
		
		// Sleep for some unknown time
		I_WaitVBL(M_Random() & 1);
	}
	
	// Set last as NUL (for easy printf)
	New->UUID[i] = 0;
	
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
#define SETMOUSE(c,b) l_DefaultCtrls[SETKEY_M(DPEXIC_,c)][1] = 0x2000 | ((b) - 1)
#define SETDBLMOUSE(c,b) l_DefaultCtrls[SETKEY_M(DPEXIC_,c)][2] = 0x4000 | ((b) - 1)
		
		SETMOUSE(ATTACK, 1);
		SETMOUSE(MOVEMENT, 3);
		SETMOUSE(PREVWEAPON, 5);
		SETMOUSE(NEXTWEAPON, 4);
		SETDBLMOUSE(USE, 3);
	
		SETKEY(SPEED, SHIFT);
		SETKEY(MOVEMENT, ALT);
		SETKEY(LOOKING, S);
		SETKEY(FORWARDS, UP);
		SETKEY(BACKWARDS, DOWN);
		SETKEY(STRAFELEFT, COMMA);
		SETKEY(STRAFERIGHT, PERIOD);
		SETKEY(JUMP, FORWARDSLASH);
		SETKEY(LAND, HOME);
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
	
		// Joystick Buttons
		SETJOY(ATTACK, 1);
		SETJOY(USE, 2);
		SETJOY(MOVEMENT, 3);
		SETJOY(SPEED, 4);
		
		// Now set
		l_DefaultCtrlsMapped = true;

#undef SETJOY
#undef SETKEY_M
#undef SETKEY
	}
	
	// Copy directly from defaults
	memmove(New->Ctrls, l_DefaultCtrls, sizeof(l_DefaultCtrls));
	
	// Mouse Axis
		// Not ALT
	New->MouseAxis[0][0] = DPEXCMA_LOOKX;
	New->MouseAxis[0][1] = DPEXCMA_MOVEY;
		// ALT
	New->MouseAxis[1][0] = DPEXCMA_MOVEX;
	New->MouseAxis[1][1] = DPEXCMA_MOVEY;
		// Mouse Look (Default 'S')
	New->MouseAxis[2][0] = DPEXCMA_LOOKX;
	New->MouseAxis[2][1] = DPEXCMA_LOOKY;
	
	// Joystick Axis
		// Not ALT
	New->JoyAxis[0][0] = DPEXCMA_LOOKX;
	New->JoyAxis[0][1] = DPEXCMA_MOVEY;
		// ALT
	New->JoyAxis[1][0] = DPEXCMA_MOVEX;
	New->JoyAxis[1][1] = DPEXCMA_MOVEY;
		// Mouse Look (Default 'S')
	New->JoyAxis[2][0] = DPEXCMA_LOOKX;
	New->JoyAxis[2][1] = DPEXCMA_LOOKY;

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
	else if (a_Code & 0x4000)
		snprintf(a_Dest, a_Size, "dblmouseb%02i", (int)((a_Code & 0xFFF) - 1));
	
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
		return 0x1000 | (strtol(a_Str + 4, NULL, 10) + 1);
	
	/* Mouse Buttons */
	else if (strncasecmp(a_Str, "mouseb", 6) == 0)
		return 0x2000 | (strtol(a_Str + 6, NULL, 10) + 1);
	
	/* Double Mouse Buttons */
	else if (strncasecmp(a_Str, "dblmouseb", 9) == 0)
		return 0x4000 | (strtol(a_Str + 9, NULL, 10) + 1);
	
	/* Keyboard Keys */
	else
	{
		for (i = 0; i < NUMIKEYBOARDKEYS; i++)
			if (strcasecmp(a_Str, c_KeyNames[i][0]) == 0)
				return i;
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
		//if (Rover->Flags & DPEXF_DONTSAVE)
		//	continue;
		
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
	int i, k;
	
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
	}
	
	// Change Value
	else if (strcasecmp(a_ArgV[1], "value") == 0)
	{
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
		
		// Load Index
		i = strtol(a_ArgV[4], NULL, 10);
		
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
	
	return 0;
#undef BUFSIZE
}

