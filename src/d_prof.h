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

#ifndef __D_PROF_H__
#define __D_PROF_H__

#include "doomtype.h"
#include "doomdef.h"
#include "command.h"
#include "d_netcmd.h"

/************************
*** EXTENDED PROFILES ***
************************/

/*** CONSTANTS ***/

/* D_ProfileExType_t -- Profile Type */
typedef enum D_ProfileExType_e
{
	DPEXT_LOCAL,								// Local player
	DPEXT_NETWORK,								// Network player
	DPEXT_BOT,									// Bot player
	
	NUMDPROFILEEXTYPES
} D_ProfileExType_t;

/* D_ProfileExFlags_t -- Extended profile flags */
typedef enum D_ProfileExFlags_e
{
	DPEXF_GOTMOUSE				= 0x00000001,	// Has control of the mouse
	DPEXF_GOTJOY				= 0x00000002,	// Controls a joystick
	DPEXF_PLAYING				= 0x00000004,	// Is playing the game
	DPEXF_SLOWTURNING			= 0x00000008,	// Enables Slow Turning
	DPEXF_DONTSAVE				= 0x00000010,	// Don't save in configs
	DPEXF_DEFAULTKEYS			= 0x00000020,	// Default Keys here!
} D_ProfileExFlags_t;

/* D_ProfileExBotFlags_t -- Bot Flags */
typedef enum D_ProfileExBotFlags_e
{
	DPEXBOTF_MOUSE				= 0x00000001,	// Bot "uses" a mouse
	DPEXBOTF_TURNAROUND			= 0x00000002,	// Bot can do 180 degree turns
} D_ProfileExBotFlags_t;

/* D_ProfileExInputCtrl_t -- Input control */
typedef enum D_ProfileExInputCtrl_e
{
	DPEXIC_NULL,
	
	/* Modifiers */
	DPEXIC_SPEED,
	DPEXIC_MOVEMENT,
	DPEXIC_LOOKING,
	
	/* Movement */
	DPEXIC_FORWARDS,
	DPEXIC_BACKWARDS,
	DPEXIC_STRAFELEFT,
	DPEXIC_STRAFERIGHT,
	DPEXIC_FLYUP,
	DPEXIC_FLYDOWN,
	DPEXIC_LAND,
	DPEXIC_JUMP,
	
	/* Looking */
	DPEXIC_TURNLEFT,
	DPEXIC_TURNRIGHT,
	DPEXIC_TURNSEMICIRCLE,
	DPEXIC_LOOKUP,
	DPEXIC_LOOKDOWN,
	DPEXIC_LOOKCENTER,
	
	/* Actions */
	DPEXIC_USE,
	DPEXIC_SUICIDE,
	DPEXIC_TAUNT,
	DPEXIC_CHAT,
	DPEXIC_TEAMCHAT,
	
	/* Weapons */
	DPEXIC_ATTACK,
	DPEXIC_ALTATTACK,
	DPEXIC_RELOAD,
	DPEXIC_SWITCHFIREMODE,
	DPEXIC_SLOT1,
	DPEXIC_SLOT2,
	DPEXIC_SLOT3,
	DPEXIC_SLOT4,
	DPEXIC_SLOT5,
	DPEXIC_SLOT6,
	DPEXIC_SLOT7,
	DPEXIC_SLOT8,
	DPEXIC_SLOT9,
	DPEXIC_SLOT10,
	DPEXIC_NEXTWEAPON,
	DPEXIC_PREVWEAPON,
	DPEXIC_BESTWEAPON,
	DPEXIC_WORSTWEAPON,
	
	/* Inventory */
	DPEXIC_NEXTINVENTORY,
	DPEXIC_PREVINVENTORY,
	DPEXIC_USEINVENTORY,
	DPEXIC_CANCELINVENTORY,
	
	/* General */
	DPEXIC_TOPSCORES,							// Show the best players
	DPEXIC_BOTTOMSCORES,						// Show the worst players
	DPEXIC_COOPSPY,								// Coop Spy
	DPEXIC_AUTOMAP,								// Toggle Automap
	DPEXIC_CHATMODE,							// Chat Mode
	DPEXIC_POPUPMENU,							// Popup the menu
	DPEXIC_MORESTUFF,							// Access to more stuff
	
	NUMDPROFILEEXINPUTCTRLS
} D_ProfileExInputCtrl_t;

/* D_ProfileExCtrlMA_t -- Controlled mouse axis */
typedef enum D_ProfileExCtrlMA_e
{
	DPEXCMA_NULL,								// Do Nothing
	DPEXCMA_MOVEX,								// Move on axis
	DPEXCMA_MOVEY,
	DPEXCMA_LOOKX,								// Look on axis
	DPEXCMA_LOOKY,
	
	DPEXCMA_NEGMOVEX,							// Negative Move on axis
	DPEXCMA_NEGMOVEY,
	DPEXCMA_NEGLOOKX,							// Negative Look on axis
	DPEXCMA_NEGLOOKY,
	
	NUMDPROFILEEXCTRLMAS
} D_ProfileExCtrlMA_t;

#define MAXALTAXIS		3
#define MAXMOUSEAXIS	2
#define MAXJOYAXIS		8

#define MAXPROFILEUUID (MAXPLAYERNAME * 2)

/*** STRUCTURES ***/

struct mobj_s;

/* D_ProfileEx_t -- Extended Profile */
typedef struct D_ProfileEx_s
{
	/* Profile Related */
	D_ProfileExType_t Type;						// Type of profile
	uint32_t Flags;								// Flags for profile controller
	char DisplayName[MAXPLAYERNAME];			// Name to show in network games
	char AccountName[MAXPLAYERNAME];			// Local account name (selection limited)
	uint8_t Color;								// Color
	uint8_t JoyControl;							// Which joystick player controls
	char UUID[MAXPROFILEUUID + 1];				// Player Unique ID
	tic_t SlowTurnTime;							// Time to slow turn
	uint32_t InstanceID;						// Instance ID
	
	/* Controls For Player */
	uint32_t Ctrls[NUMDPROFILEEXINPUTCTRLS][4];	// Player Controls
	D_ProfileExCtrlMA_t MouseAxis[MAXALTAXIS][MAXMOUSEAXIS];	// Mouse Axis Movement
	D_ProfileExCtrlMA_t JoyAxis[MAXALTAXIS][MAXJOYAXIS];		// Joy Axis Movement
	int32_t MouseSens[2];						// Mouse Sensitivity
	int32_t JoySens[2];							// Joystick Sensitivity
	int32_t LookUpDownSpeed;					// Looking Up/Down Speed
	int32_t AutoGrabJoy;						// Auto-Grab Joystick
	
	/* Profile Chains */
	struct D_ProfileEx_s* Prev;					// Previous link
	struct D_ProfileEx_s* Next;					// Next link
	
	/* Other stuff */
	uint8_t ColorPickup;						// Color for pickups
	uint8_t ColorSecret;						// Secret Found Color
	int32_t SoundSecret;						// Sound to play when Secret Found
	bool_t DrawPSprites;						// Draw Player Sprites
	int8_t BobMode;								// Bobbing Mode (Doom, Mid, Effort)
	fixed_t ViewHeight;							// View Height
	fixed_t CamDist, CamHeight, CamSpeed;		// Camera Properties
	bool_t ChaseCam;							// Enable chase camera
	bool_t TransSBar;							// Transparent Status Bar
	bool_t ScaledSBar;							// Scaled Status Bar
	char HexenClass[MAXPLAYERNAME];				// Hexen Class
} D_ProfileEx_t;

/*** GLOBALS ***/

extern D_ProfileEx_t* g_KeyDefaultProfile;

/*** FUNCTIONS ***/

D_ProfileEx_t* D_CreateProfileEx(const char* const a_Name);

D_ProfileEx_t* D_FindProfileEx(const char* const a_Name);
D_ProfileEx_t* D_FindProfileExByInstance(const uint32_t a_ID);

void D_SaveProfileData(void (*a_WriteBack)(const char* const a_Buf, void* const a_Data), void* const a_Data);
int CLC_Profile(const uint32_t a_ArgC, const char** const a_ArgV);

#endif							/* __D_PROF_H__ */

