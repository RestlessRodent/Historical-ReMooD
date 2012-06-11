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

/************************
*** EXTENDED PROFILES ***
************************/

/*** LOCALS ***/

static D_ProfileEx_t* l_FirstProfile = NULL;	// First in chain

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
	
	/* Copy Name */
	strncpy(New->AccountName, a_Name, MAXPLAYERNAME - 1);
	strncpy(New->DisplayName, a_Name, MAXPLAYERNAME - 1);
	
	/* Set Default Options */
	New->Flags |= DPEXF_GOTMOUSE | DPEXF_GOTJOY | DPEXF_SLOWTURNING;
	New->SlowTurnTime = 6;
	
	// Default Controls
#define SETKEY_M(a,b) a##b
#define SETKEY(c,k) New->Ctrls[SETKEY_M(DPEXIC_,c)][0] = (SETKEY_M(IKBK_,k))
#define SETJOY(c,b) New->Ctrls[SETKEY_M(DPEXIC_,c)][3] = 0x1000 | ((b) - 1)

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

#undef SETJOY
#undef SETKEY_M
#undef SETKEY
	
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

