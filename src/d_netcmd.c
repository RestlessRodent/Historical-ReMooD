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
// DESCRIPTION:
//      host/client network commands
//      commands are executed through the command buffer
//      like console commands
//      other miscellaneous commands (at the end)

#include "doomdef.h"
#include "doomstat.h"

#include "console.h"
#include "command.h"

#include "d_netcmd.h"
#include "i_system.h"
#include "dstrings.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "g_input.h"
#include "m_menu.h"
#include "r_local.h"
#include "r_things.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_setup.h"
#include "s_sound.h"
#include "m_misc.h"
#include "am_map.h"
#include "p_spec.h"
#include "m_cheat.h"
#include "d_clisrv.h"
#include "v_video.h"
#include "d_main.h"
#include "p_demcmp.h"
#include "i_sound.h"
#include "i_util.h"
#include "i_video.h"
#include "am_map.h"

#include "b_bot.h"

// ------
// protos
// ------
void Command_Color_f(void);
void Command_Name_f(void);

void Command_WeaponPref(void);

void TeamPlay_OnChange(void);
void FragLimit_OnChange(void);
void Deahtmatch_OnChange(void);
void TimeLimit_OnChange(void);

void Command_Playdemo_f(void);
void Command_Timedemo_f(void);
void Command_Stopdemo_f(void);
void Command_Map_f(void);
void Command_Restart_f(void);

void Command_Pause(void);

void Command_Frags_f(void);
void Command_TeamFrags_f(void);
void Command_Version_f(void);
void Command_Quit_f(void);

void Command_Water_f(void);
void Command_ExitLevel_f(void);
void Command_Load_f(void);
void Command_Save_f(void);
void Command_ExitGame_f(void);

void Command_Kill(void);

// =========================================================================
//                           CLIENT VARIABLES
// =========================================================================

void CL_Player1Change(void);
void CL_Player2Change(void);
void CL_Player3Change(void);
void CL_Player4Change(void);

// =========================================================================
//                           CLIENT STARTUP
// =========================================================================

// register client and server commands
//
void D_RegisterClientCommands(void)
{
}

// =========================================================================
//                            CLIENT STUFF
// =========================================================================

/*****************************
*** EXTENDED NETWORK STUFF ***
*****************************/

/*** CONSTANTS ***/

static const fixed_t c_forwardmove[2] = { 25, 50 };
static const fixed_t c_sidemove[2] = { 24, 40 };
static const fixed_t c_angleturn[3] = { 640, 1280, 320 };	// + slow turn
#define MAXPLMOVE       (c_forwardmove[1])

#define MAXLOCALJOYS	MAXJOYSTICKS

const int32_t c_TCDataSize[NUMDTCT] =
{
	// NULL
	0,
	
	// JOIN
	4 + 2 + 4 + 4 + 4 + 4 + 1 + MAXPLAYERNAME + MAXPLAYERNAME,
		// uint32 HostID
		// uint16 players[] Spot
		// uint32 Symbols
		// uint32 Profile Instance
		// uint32 Process ID
		// uint32 Unique ID
		// uint8  Color
		// uint8* Name
		// uint8* Hexen Class
	
	// MAP CHANGE
	1 + 8,
		// uint8  Flags
		// uint8* Lump Name
	
	// DTCT_GAMEVAR, GAME VARIABLE
	4 + 4,
		// uint32 Code
		// int32  New Value
	
	// DTCT_PART, Player Leaves
	1 + 4,
		// uint8  Going to spectate
		// uint32 Viewport UUID
	
	// DTCT_ADDSPEC, Add Spectator
	4 + 4 + MAXPLAYERNAME,
		// uint32 Unique ID
		// uint32 Host ID
		// uint8* Account Name
	
	// DTCT_XKICKPLAYER, Kick Player
	2 + 4 + MAXTCCBUFSIZE,
		// uint16 In Game ID
		// uint32 Unique ID
		// uint8* Reason (MAXTCCBUFSIZE)
	
	// DTCT_XADDPLAYER, Add Player
	4 + 4 + 4 + 1 + 4 + 4 + MAXPLAYERNAME + MAXPLAYERNAME,
		// uint32 Unique ID
		// uint32 Host ID
		// uint32 Process ID
		// uint8  Screen ID
		// uint32 Reserved (set to zero)
		// uint32 Conveyed Flags
		// uint8* Account Name + Cookie
	
	// DTCT_XJOINPLAYER, Join Player
	4 + 4 + 4 + 4 + 1 + 1 + 4 + MAXPLAYERNAME + MAXPLAYERNAME,
		// uint32 Player ID
		// uint32 Process ID
		// uint32 Host ID
		// uint32 Flags
		// uint8  Color
		// uint8  CTF Team
		// uint32 Skin Name Hash
		// uint8* Display Name
		// uint8* Hexen Class
};

/*** GLOBALS ***/

bool_t g_NetDev = false;						// Network Debug
int g_SplitScreen = -1;							// Split screen players (-1 based)
D_SplitInfo_t g_Splits[MAXSPLITSCREEN];			// Split Information

/*** LOCALS ***/

static bool_t l_PermitMouse = false;			// Use mouse input
static int32_t l_MouseMove[2] = {0, 0};			// Mouse movement (x/y)
static uint32_t l_MouseButtons[2];				// Mouse buttons down (sng/dbl)
static tic_t l_MouseLastTime[32];				// Last time pressed
static bool_t l_KeyDown[NUMIKEYBOARDKEYS];		// Keys that are down
static uint32_t l_JoyButtons[MAXLOCALJOYS];		// Local Joysticks
static int16_t l_JoyAxis[MAXLOCALJOYS][MAXJOYAXIS];
static D_NetPlayer_t* l_FirstNetPlayer = NULL;	// First Net Player

/*** FUNCTIONS ***/

/* D_ScrSplitHasPlayer() -- Split-screen has player */
bool_t D_ScrSplitHasPlayer(const int8_t a_Player)
{
	/* Check */
	if (a_Player < 0 || a_Player >= MAXSPLITSCREEN)
		return false;
	
	/* Active (Non-Demo Only) */
	if (!demoplayback && g_Splits[a_Player].Active)
		return true;
	
	/* Waiting for player */
	if (g_Splits[a_Player].Waiting)
		return true;
	
	/* No player */
	return false;
}

/* D_ScrSplitVisible() -- Screen can be seen */
bool_t D_ScrSplitVisible(const int8_t a_Player)
{
	/* Check */
	if (a_Player < 0 || a_Player >= MAXSPLITSCREEN)
		return false;
	
	/* Visible if has a player */
	if (a_Player == 0 || D_ScrSplitHasPlayer(a_Player))
		return true;
	
	/* Not visible */
	return false;
}

/* DS_NCSNetCommand() -- Network commands */
static int DS_NCSNetCommand(const uint32_t a_ArgC, const char** const a_ArgV)
{
	struct player_s* p;
	int i;
	
	/* Check */
	if (a_ArgC < 2)
		return 1;
	
	/* Which Sub Command? */
	// Internal add Player
	else if (strcasecmp(a_ArgV[1], "addplayer") == 0)
	{
		if (devparm)
			CONL_PrintF("NET: Requesting the server add local player.\n");
		D_NCLocalPlayerAdd((a_ArgC >= 3 ? a_ArgV[2] : NULL), false, g_SplitScreen, INT_MAX, false);
		return 0;
	}
	
	// Internal add bot
	else if (strcasecmp(a_ArgV[1], "addbot") == 0)
	{
		if (devparm)
			CONL_PrintF("NET: Requesting the server add local bot.\n");
		D_NCLocalPlayerAdd((a_ArgC >= 3 ? a_ArgV[2] : NULL), true, g_SplitScreen, INT_MAX, false);
		return 0;
	}
	
	// Internal spectate
	else if (strcasecmp(a_ArgV[1], "spectate") == 0)
	{
	}
	
	/* Success */
	return 0;
}

/* D_NCSInit() -- Initialize network client/server */
void D_NCSInit(void)
{
	/* Debug? */
	if (M_CheckParm("-netdev"))
		g_NetDev = true;
	
	/* Register "net" command */
	CONL_AddCommand("net", DS_NCSNetCommand);
}

/* D_NCSHandleEvent() -- Handle advanced events */
bool_t D_NCSHandleEvent(const I_EventEx_t* const a_Event)
{
	int32_t ButtonNum, LocalJoy;
	uint32_t Bit;
	
	/* Check */
	if (!a_Event)
		return false;
	
	/* Which kind of event? */
	switch (a_Event->Type)
	{
			// Mouse
		case IET_MOUSE:
			// Add position to movement
			l_MouseMove[0] += a_Event->Data.Mouse.Move[0];
			l_MouseMove[1] += a_Event->Data.Mouse.Move[1];
			
			// Handling of buttons (with double click)
			if (a_Event->Data.Mouse.Button > 0 && a_Event->Data.Mouse.Button < 32)
			{
				// Determine bit
				ButtonNum = a_Event->Data.Mouse.Button - 1U;
				Bit = 1U << (ButtonNum);
				
				// Unpressed?
				if (!a_Event->Data.Mouse.Down)
				{
					l_MouseButtons[0] &= ~Bit;
					l_MouseButtons[1] &= ~Bit;
				}
				else
				{
					// Always set single bit
					l_MouseButtons[0] |= Bit;
					
					// Double Click?
						// TODO make this a CVAR of sorts
					if (g_ProgramTic - l_MouseLastTime[ButtonNum] < 17)
					{
						l_MouseButtons[1] |= Bit;
						l_MouseLastTime[ButtonNum] = 0;
					}
				
					// Single Click (set last time for double)
					else
						l_MouseLastTime[ButtonNum] = g_ProgramTic;
				}
			}
			break;
			
			// Keyboard
		case IET_KEYBOARD:
			if (a_Event->Data.Keyboard.KeyCode >= 0 && a_Event->Data.Keyboard.KeyCode < NUMIKEYBOARDKEYS)
				l_KeyDown[a_Event->Data.Keyboard.KeyCode] = a_Event->Data.Keyboard.Down;
			break;
			
			// Joystick
		case IET_JOYSTICK:
			// Get local joystick
			LocalJoy = a_Event->Data.Joystick.JoyID;
			
			// Now determine which action
			if (LocalJoy >= 0 && LocalJoy < MAXLOCALJOYS)
			{
				// Button Pressed Down
				if (a_Event->Data.Joystick.Button)
				{
					// Get Number
					ButtonNum = a_Event->Data.Joystick.Button;
					ButtonNum--;
					
					// Limited to 32 buttons =(
					if (ButtonNum >= 0 && ButtonNum < 32)
					{
						// Was it pressed?
						if (a_Event->Data.Joystick.Down)
							l_JoyButtons[LocalJoy] |= (1 << ButtonNum);
						else
							l_JoyButtons[LocalJoy] &= ~(1 << ButtonNum);
					}
				}
				
				// Axis Moved
				else if (a_Event->Data.Joystick.Axis)
				{
					ButtonNum = a_Event->Data.Joystick.Axis;
					ButtonNum--;
					
					if (ButtonNum >= 0 && ButtonNum < MAXJOYAXIS)
						l_JoyAxis[LocalJoy][ButtonNum] = a_Event->Data.Joystick.Value;
				}
			}
			break;
		
			// Unknown
		default:
			break;
	}
	
	/* Un-Handled */
	return false;
}

/* NextWeapon() -- Finds the next weapon in the chain */
// This is for PrevWeapon and NextWeapon
// Rewritten for RMOD Support!
// This uses the fields in weaponinfo_t for ordering info
static uint8_t DS_NCSNextWeapon(player_t* player, int step)
{
	size_t g, w, fw, BestNum;
	int32_t s, StepsLeft, StepsAdd, BestDiff, ThisDiff;
	size_t MostOrder, LeastOrder;
	bool_t Neg;
	weaponinfo_t** weapons;
	
	/* Get current weapon info */
	weapons = player->weaponinfo;
	
	/* Get the weapon with the lowest and highest order */
	// Find first gun the player has (so order is correct)
	MostOrder = LeastOrder = 0;
	for (w = 0; w < NUMWEAPONS; w++)
		if (P_CanUseWeapon(player, w))
		{
			// Got the first available gun
			MostOrder = LeastOrder = w;
			break;
		}
	
	// Now go through
	for (w = 0; w < NUMWEAPONS; w++)
	{
		// Can't use this gun?
		if (!P_CanUseWeapon(player, w))
			continue;
		
		// Least
		if (weapons[w]->SwitchOrder < weapons[LeastOrder]->SwitchOrder)
			LeastOrder = w;
		
		// Most
		if (weapons[w]->SwitchOrder > weapons[MostOrder]->SwitchOrder)
			MostOrder = w;
	}
	
	/* Look for the current weapon in the weapon list */
	// Well that was easy
	fw = s = g = player->readyweapon;
	
	/* Constantly change the weapon */
	// Prepare variables
	Neg = (step < 0 ? true : false);
	StepsAdd = (Neg ? -1 : 1);
	StepsLeft = step * StepsAdd;
	
	// Go through the weapon list, step times
	while (StepsLeft > 0)
	{
		// Clear variables
		BestDiff = 9999999;		// The worst weapon difference ever
		BestNum = NUMWEAPONS;
		
		// Go through every weapon and find the next in the order
		for (w = 0; w < NUMWEAPONS; w++)
		{
			// Ignore the current weapon (don't want to switch back to it)
			if (w == fw)		// Otherwise BestDiff is zero!
				continue;
			
			// Can't use this gun?
			if (!P_CanUseWeapon(player, w))
				continue;
			
			// Only consider worse/better weapons?
			if ((Neg && weapons[w]->SwitchOrder > weapons[fw]->SwitchOrder) || (!Neg && weapons[w]->SwitchOrder < weapons[fw]->SwitchOrder))
				continue;
			
			// Get current diff
			ThisDiff = abs(weapons[fw]->SwitchOrder - weapons[w]->SwitchOrder);
			
			// Closer weapon?
			if (ThisDiff < BestDiff)
			{
				BestDiff = ThisDiff;
				BestNum = w;
			}
		}
		
		// Found no weapon? Then "loop" around
		if (BestNum == NUMWEAPONS)
		{
			// Switch to the highest gun if going down
			if (Neg)
				fw = MostOrder;
			
			// And if going up, go to the lowest
			else
				fw = LeastOrder;
		}
		
		// Found a weapon
		else
		{
			// Switch to this gun
			fw = BestNum;
		}
		
		// Next step
		StepsLeft--;
	}
	
	/* Return the weapon we want */
	return fw;
}

/* GAMEKEYDOWN() -- Checks if a key is down */
static bool_t GAMEKEYDOWN(D_ProfileEx_t* const a_Profile, const uint8_t a_SID, const uint8_t a_Key)
{
	static bool_t Recoursed;
	bool_t MoreDown;
	size_t i;
	uint32_t CurrentButton;
	
	/* Determine if more key is down */
	// But do not infinite loop here
	MoreDown = false;
	if (!Recoursed)
	{
		Recoursed = true;
		MoreDown = GAMEKEYDOWN(a_Profile, a_SID, DPEXIC_MORESTUFF);
		Recoursed = false;
	}
	
	/* Check Keyboard */
	for (i = 0; i < 4; i++)
		if (a_Profile->Ctrls[a_Key][i] >= 0 && a_Profile->Ctrls[a_Key][i] < NUMIKEYBOARDKEYS)
			if (l_KeyDown[a_Profile->Ctrls[a_Key][i]])
				return true;
	
	/* Check Joysticks */
	//if (a_Profile->Flags & DPEXF_GOTJOY)
		//if (a_Profile->JoyControl >= 0 && a_Profile->JoyControl < 4)
	if (a_SID >= 0 && a_SID < MAXSPLITSCREEN && g_Splits[a_SID].JoyBound)
		if (g_Splits[a_SID].JoyID >= 1 && g_Splits[a_SID].JoyID <= MAXLOCALJOYS)
			if (l_JoyButtons[g_Splits[a_SID].JoyID - 1])
				for (i = 0; i < 4; i++)
					if ((a_Profile->Ctrls[a_Key][i] & 0xF000) == (MoreDown ? 0x4000 : 0x1000))
					{
						// Get current button
						CurrentButton = (a_Profile->Ctrls[a_Key][i] & 0x00FF);
				
						// Button pressed?
						if (CurrentButton >= 0 && CurrentButton < 32)
							if (l_JoyButtons[g_Splits[a_SID].JoyID - 1] & (1 << CurrentButton))
								return true;
					}
				
	/* Check Mice */
	if (a_Profile->Flags & DPEXF_GOTMOUSE)
		if (l_MouseButtons[0] || l_MouseButtons[1])
			for (i = 0; i < 4; i++)
			{
				// Single
				if ((a_Profile->Ctrls[a_Key][i] & 0xF000) == (MoreDown ? 0x5000 : 0x2000))
				{
					// Get current button
					CurrentButton = (a_Profile->Ctrls[a_Key][i] & 0x00FF);
		
					// Button pressed?
					if (CurrentButton >= 0 && CurrentButton < 32)
						if (l_MouseButtons[0] & (1 << CurrentButton))
							return true;
				}
		
				// Double
				if ((a_Profile->Ctrls[a_Key][i] & 0xF000) == (MoreDown ? 0x6000 : 0x3000))
				{
					// Get current button
					CurrentButton = (a_Profile->Ctrls[a_Key][i] & 0x00FF);
		
					// Button pressed?
					if (CurrentButton >= 0 && CurrentButton < 32)
						if (l_MouseButtons[1] & (1 << CurrentButton))
							return true;
				}
			}
	
	/* Not pressed */
	return false;
}

short G_ClipAimingPitch(int* aiming);

/* D_NCSLocalBuildTicCmd() -- Build local tic command */
static void D_NCSLocalBuildTicCmd(D_NetPlayer_t* const a_NPp, ticcmd_t* const a_TicCmd, size_t* const a_SIDp)
{
#define MAXWEAPONSLOTS 12
	D_ProfileEx_t* Profile;
	player_t* Player;
	int32_t TargetMove;
	size_t i, PID, SID;
	int8_t SensMod, MoveMod, MouseMod, MoveSpeed, TurnSpeed;
	int32_t SideMove, ForwardMove, BaseAT, BaseAM;
	bool_t IsTurning, GunInSlot, ResetAim;
	int slot, j, l, k;
	weapontype_t newweapon;
	weapontype_t SlotList[MAXWEAPONSLOTS];
	
	/* Check */
	if (!a_NPp || !a_TicCmd)
		return;
	
	/* Obtain profile */
	Profile = a_NPp->Profile;
	Player = a_NPp->Player;
	
	// No profile?
	if (!Profile)
		return;
	
	/* Find Player ID */
	PID = a_NPp->Player - players;
	
	// Illegal player?
	if (PID < 0 || PID >= MAXPLAYERS)
		return;
	
	/* Find Screen ID */
	for (SID = 0; SID < MAXSPLITSCREEN; SID++)
		if (g_Splits[SID].Active)
			if (g_Splits[SID].Console == PID)
				break;
	
	// No player here?
	if (!g_Splits[SID].Active)
		return;
	
	// Not found?
	if (SID >= MAXSPLITSCREEN)
		return;
	
	// Return pointer
	if (a_SIDp)
		*a_SIDp = SID;
	
	/* Reset Some Things */
	SideMove = ForwardMove = BaseAT = BaseAM = 0;
	IsTurning = ResetAim = false;
	
	/* Modifiers */
	// Mouse Sensitivity
	SensMod = 0;
	
	// Movement Modifier
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_MOVEMENT))
		MoveMod = 1;
	else
		MoveMod = 0;
	
	// Mouse Modifier
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_LOOKING))
		MouseMod = 2;
	else if (MoveMod)
		MouseMod = 1;
	else 
		MouseMod = 0;
	
	// Moving Speed
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_SPEED))
		MoveSpeed = 1;
	else
		MoveSpeed = 0;
	
	// Turn Speed
	if ((Profile->Flags & DPEXF_SLOWTURNING) &&
			gametic < (a_NPp->TurnHeld + Profile->SlowTurnTime))
		TurnSpeed = 2;
	else if (MoveSpeed)
		TurnSpeed = 1;
	else
		TurnSpeed = 0;
	
	/* Player has joystick input? */
	// Read input for all axis
	if (SID >= 0 && SID < MAXSPLITSCREEN && g_Splits[SID].JoyBound)
		if (g_Splits[SID].JoyID >= 1 && g_Splits[SID].JoyID <= MAXLOCALJOYS)
			for (i = 0; i < MAXJOYAXIS; i++)
			{
				// Modify with sensitivity
				TargetMove = ((float)l_JoyAxis[g_Splits[SID].JoyID - 1][i]) * (((float)Profile->JoySens[SensMod]) / 100.0);
			
				// Which movement to perform?
				switch (Profile->JoyAxis[MouseMod][i])
				{
						// Movement
					case DPEXCMA_MOVEX:
					case DPEXCMA_MOVEY:
						// Movement is fractionally based
						TargetMove = (((float)TargetMove) / ((float)32767.0)) * ((float)c_forwardmove[MoveSpeed]);
					
						// Now which action really?
						if (Profile->JoyAxis[MouseMod][i] == DPEXCMA_MOVEX)
							SideMove += TargetMove;
						else
							ForwardMove -= TargetMove;
						break;
					
						// Looking Left/Right
					case DPEXCMA_LOOKX:
						TargetMove = (((float)TargetMove) / ((float)32767.0)) * ((float)c_angleturn[TurnSpeed]);
						IsTurning = true;
						BaseAT -= TargetMove;
						break;
					
						// Looking Up/Down
					case DPEXCMA_LOOKY:
						break;
				
					default:
						break;
				}
			}
	
	/* Player has mouse input? */
	if (l_PermitMouse && (Profile->Flags & DPEXF_GOTMOUSE))
	{
		// Read mouse input for both axis
		for (i = 0; i < 2; i++)
		{
			// Modify with sensitivity
			TargetMove = l_MouseMove[i] * ((((float)(Profile->MouseSens[SensMod] * Profile->MouseSens[SensMod])) / 110.0) + 0.1);
			
			// Do action for which movement type?
			switch (Profile->MouseAxis[MouseMod][i])
			{
					// Strafe Left/Right
				case DPEXCMA_MOVEX:
					SideMove += TargetMove;
					break;
					
					// Move Forward/Back
				case DPEXCMA_MOVEY:
					ForwardMove += TargetMove;
					break;
					
					// Left/Right Look
				case DPEXCMA_LOOKX:
					BaseAT -= TargetMove * 8;
					break;
					
					// Up/Down Look
				case DPEXCMA_LOOKY:
					BaseAM += TargetMove << 3;
					//localaiming[SID] += TargetMove << 19;
					break;
				
					// Unknown
				default:
					break;
			}
		}
		
		// Clear mouse permission
		l_PermitMouse = false;
		
		// Clear mouse input
		l_MouseMove[0] = l_MouseMove[1] = 0;
	}
	
	/* Handle Player Control Keyboard Stuff */
	// Weapon Attacks
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_ATTACK))
		a_TicCmd->Std.buttons |= BT_ATTACK;
	
	// Use
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_USE))
		a_TicCmd->Std.buttons |= BT_USE;
	
	// Jump
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_JUMP))
		a_TicCmd->Std.buttons |= BT_JUMP;
	
	// Suicide
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_SUICIDE))
		a_TicCmd->Std.buttons |= BT_SUICIDE;
	
	// Keyboard Turning
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_TURNLEFT))
	{
		// Strafe
		if (MoveMod)
			SideMove -= c_sidemove[MoveSpeed];
		
		// Turn
		else
		{
			BaseAT += c_angleturn[TurnSpeed];
			IsTurning = true;
		}
	}
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_TURNRIGHT))
	{
		// Strafe
		if (MoveMod)
			SideMove += c_sidemove[MoveSpeed];
		
		// Turn
		else
		{
			BaseAT -= c_angleturn[TurnSpeed];
			IsTurning = true;
		}
	}
	
	// Keyboard Moving
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_STRAFELEFT))
		SideMove -= c_sidemove[MoveSpeed];
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_STRAFERIGHT))
		SideMove += c_sidemove[MoveSpeed];
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_FORWARDS))
		ForwardMove += c_forwardmove[MoveSpeed];
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_BACKWARDS))
		ForwardMove -= c_forwardmove[MoveSpeed];
		
	// Looking
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_LOOKCENTER))
		ResetAim = true;
		//localaiming[SID] = 0;
	else
	{
		if (GAMEKEYDOWN(Profile, SID, DPEXIC_LOOKUP))
			BaseAM += Profile->LookUpDownSpeed >> 16;
			//localaiming[SID] += Profile->LookUpDownSpeed;
		
		if (GAMEKEYDOWN(Profile, SID, DPEXIC_LOOKDOWN))
			BaseAM -= Profile->LookUpDownSpeed >> 16;
			//localaiming[SID] -= Profile->LookUpDownSpeed;
	}
	
	// Weapons
		// Next
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_NEXTWEAPON))
	{
		// Set switch
		a_TicCmd->Std.buttons |= BT_CHANGE;
		D_TicCmdFillWeapon(a_TicCmd, DS_NCSNextWeapon(Player, 1));
	}
		// Prev
	else if (GAMEKEYDOWN(Profile, SID, DPEXIC_PREVWEAPON))
	{
		// Set switch
		a_TicCmd->Std.buttons |= BT_CHANGE;
		D_TicCmdFillWeapon(a_TicCmd, DS_NCSNextWeapon(Player, -1));
	}
		// Best Gun
	else if (GAMEKEYDOWN(Profile, SID, DPEXIC_BESTWEAPON))
	{
		newweapon = P_PlayerBestWeapon(Player, true);
		
		if (newweapon != Player->readyweapon)
		{
			a_TicCmd->Std.buttons |= BT_CHANGE;
			D_TicCmdFillWeapon(a_TicCmd, newweapon);
		}
	}
		// Worst Gun
	else if (GAMEKEYDOWN(Profile, SID, DPEXIC_WORSTWEAPON))
	{
		newweapon = P_PlayerBestWeapon(Player, false);
		
		if (newweapon != Player->readyweapon)
		{
			a_TicCmd->Std.buttons |= BT_CHANGE;
			D_TicCmdFillWeapon(a_TicCmd, newweapon);
		}
	}
		// Slots
	else
	{
		// Which slot?
		slot = -1;
		
		// Look for keys
		for (i = DPEXIC_SLOT1; i <= DPEXIC_SLOT10; i++)
			if (GAMEKEYDOWN(Profile, SID, i))
			{
				slot = (i - DPEXIC_SLOT1) + 1;
				break;
			}
		
		// Hit slot?
		if (slot != -1)
		{
			// Clear flag
			GunInSlot = false;
			l = 0;
		
			// Figure out weapons that belong in this slot
			for (j = 0, i = 0; i < NUMWEAPONS; i++)
				if (P_CanUseWeapon(Player, i))
				{
					// Weapon not in this slot?
					if (Player->weaponinfo[i]->SlotNum != slot)
						continue;
				
					// Place in slot list before the highest
					if (j < (MAXWEAPONSLOTS - 1))
					{
						// Just place here
						if (j == 0)
						{
							// Current weapon is in this slot?
							if (Player->readyweapon == i)
							{
								GunInSlot = true;
								l = j;
							}
						
							// Place in last spot
							SlotList[j++] = i;
						}
					
						// Otherwise more work is needed
						else
						{
							// Start from high to low
								// When the order is lower, we know to insert now
							for (k = 0; k < j; k++)
								if (Player->weaponinfo[i]->SwitchOrder < Player->weaponinfo[SlotList[k]]->SwitchOrder)
								{
									// Current gun may need shifting
									if (!GunInSlot)
									{
										// Current weapon is in this slot?
										if (Player->readyweapon == i)
										{
											GunInSlot = true;
											l = k;
										}
									}
								
									// Possibly shift gun
									else
									{
										// If the current gun is higher then this gun
										// then it will be off by whatever is more
										if (Player->weaponinfo[SlotList[l]]->SwitchOrder > Player->weaponinfo[i]->SwitchOrder)
											l++;
									}
								
									// move up
									memmove(&SlotList[k + 1], &SlotList[k], sizeof(SlotList[k]) * (MAXWEAPONSLOTS - k - 1));
								
									// Place in slightly upper spot
									SlotList[k] = i;
									j++;
								
									// Don't add it anymore
									break;
								}
						
							// Can't put it anywhere? Goes at end then
							if (k == j)
							{
								// Current weapon is in this slot?
								if (Player->readyweapon == i)
								{
									GunInSlot = true;
									l = k;
								}
							
								// Put
								SlotList[j++] = i;
							}
						}
					}
				}
		
			// No guns in this slot? Then don't switch to anything
			if (j == 0)
				newweapon = Player->readyweapon;
		
			// If the current gun is in this slot, go to the next in the slot
			else if (GunInSlot)		// from [best - worst]
				newweapon = SlotList[((l - 1) + j) % j];
		
			// Otherwise, switch to the best gun there
			else
				// Set it to the highest valued gun
				newweapon = SlotList[j - 1];
		
			// Did it work?
			if (newweapon != Player->readyweapon)
			{
				a_TicCmd->Std.buttons |= BT_CHANGE;
				D_TicCmdFillWeapon(a_TicCmd, newweapon);
			}
		}
	}
	
	// Inventory
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_NEXTINVENTORY))
		a_TicCmd->Std.InventoryBits = TICCMD_INVRIGHT;
	else if (GAMEKEYDOWN(Profile, SID, DPEXIC_PREVINVENTORY))
		a_TicCmd->Std.InventoryBits = TICCMD_INVLEFT;
	else if (GAMEKEYDOWN(Profile, SID, DPEXIC_USEINVENTORY))
		a_TicCmd->Std.InventoryBits = TICCMD_INVUSE;
	
	/* Handle special functions */
	// Show Scores
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_TOPSCORES))
		a_NPp->Scores = 1;
	else if (GAMEKEYDOWN(Profile, SID, DPEXIC_BOTTOMSCORES))
		a_NPp->Scores = -1;
	else
		a_NPp->Scores = 0;
	
	// Coop Spy
	if (GAMEKEYDOWN(Profile, SID, DPEXIC_COOPSPY))
	{
		// Only every half second
		if (gametic > (a_NPp->CoopSpyTime + (TICRATE >> 1)))
		{
			do
			{
				g_Splits[SID].Display = (g_Splits[SID].Display + 1) % MAXPLAYERS;
			} while (!playeringame[g_Splits[SID].Display] || !P_PlayerOnSameTeam(&players[g_Splits[SID].Console], &players[g_Splits[SID].Display]));
			
			// Print Message
			CONL_PrintF("%sYou are now watching %s.\n",
					(SID == 3 ? "\x6" : (SID == 2 ? "\x5" : (SID == 1 ? "\x4" : ""))),
					(g_Splits[SID].Display == g_Splits[SID].Console ? "Yourself" : D_NCSGetPlayerName(g_Splits[SID].Display))
				);
			
			// Reset timeout
			a_NPp->CoopSpyTime = gametic + (TICRATE >> 1);
		}
	}
	
	// Key is unpressed to reduce time
	else
		a_NPp->CoopSpyTime = 0;
	
	/* Set Movement Now */
	// Cap
	if (SideMove > MAXPLMOVE)
		SideMove = MAXPLMOVE;
	else if (SideMove < -MAXPLMOVE)
		SideMove = -MAXPLMOVE;
	
	if (ForwardMove > MAXPLMOVE)
		ForwardMove = MAXPLMOVE;
	else if (ForwardMove < -MAXPLMOVE)
		ForwardMove = -MAXPLMOVE;
	
	// Set
	a_TicCmd->Std.sidemove = SideMove;
	a_TicCmd->Std.forwardmove = ForwardMove;
	
	/* Slow turning? */
	if (!IsTurning)
		a_NPp->TurnHeld = gametic;
	
	/* Turning */
	a_TicCmd->Std.BaseAngleTurn = BaseAT;
	a_TicCmd->Std.BaseAiming = BaseAM;
	a_TicCmd->Std.ResetAim = ResetAim;
	
	/* Set from localaiming and such */
	// Don't change angle when teleporting
#if 0
	if ((!Player->mo) || (Player->mo && Player->mo->reactiontime <= 0))
	{
		// Local angle (x look)
		localangle[SID] += (a_TicCmd->angleturn << 16);
		a_TicCmd->angleturn = localangle[SID] >> 16;
	}
#endif
	
	// Local aiming (y look)
	//a_TicCmd->aiming = G_ClipAimingPitch(&localaiming[SID]);
#undef MAXWEAPONSLOTS
}

/* D_NCSNetUpdateSingle() -- Update single player */
void D_NCSNetUpdateSingle(struct player_s* a_Player)
{
	size_t PID, SID, i;
	D_NetPlayer_t* NPp;
	ticcmd_t* TicCmd, *LocalTicCmd;
	
	/* Check */
	if (!a_Player)
		return;
	
	// Get player ID
	PID = a_Player - players;
	
	// Get Screen ID
	for (SID = 0; SID < MAXSPLITSCREEN; SID++)
		if (g_Splits[SID].Active)
			if (PID == g_Splits[SID].Console)
				break;
	
	// More checks
	if (PID < 0 || PID >= MAXPLAYERS || !playeringame[PID])
		return;
	
	/* Get player's netplayer */
	NPp = players[PID].NetPlayer;
	
	// No net player?
	if (!NPp)
		return;
	
	/* Generate Commands */
	// Use last free spot
	if (NPp->LocalTicTotal < MAXDNETTICCMDCOUNT - 1)
		i = NPp->LocalTicTotal++;
	else
		i = MAXDNETTICCMDCOUNT - 1;
	TicCmd = &NPp->LocalTicCmd[i];
	
	// Set Time
	NPp->LastLocalTic = g_ProgramTic;
	
	// Clear command
	memset(TicCmd, 0, sizeof(*TicCmd));
	
	// Now what to do with this?
	switch (NPp->Type)
	{
			// Local player on this computer
		case DNPT_LOCAL:
			// Fill command based on controller input
			D_NCSLocalBuildTicCmd(NPp, TicCmd, NULL);
			break;
		
			// Networked player on another system
		case DNPT_NETWORK:
			break;
			
			// Bot, a simulated player
		case DNPT_BOT:
			// No bot data?
			if (!NPp->BotData)
				NPp->BotData = B_InitBot(NULL);
			
			// Build bot tic command
			//TicCmd = &NPp->TicCmd[i];
			B_BuildBotTicCmd(NULL, NPp->BotData, TicCmd);
			break;
			
			// Unknown
		default:
			break;
	}
	
	/* Place time in tics */
	TicCmd->Ctrl.ProgramTic = g_ProgramTic;
	TicCmd->Ctrl.GameTic = gametic;
}

/* D_NCSNetUpdateAll() -- Update all players */
void D_NCSNetUpdateAll(void)
{
	static tic_t LastProgram, LastTic;
	size_t i, j, SID;
	D_NetPlayer_t* NetPlayer;
	ticcmd_t TicMerge;
	
	// Extended tic command stuff
	uint8_t XNewWeapon;							// New weapon to switch to
	
	/* Same tic? */
	if (LastProgram == g_ProgramTic && LastTic == gametic)
		return;
	
	// Set time
	LastProgram = g_ProgramTic;
	LastTic = gametic;
	
	/* Enable Mouse Input */
	l_PermitMouse = true;
	
	/* Update All Players */
	for (i = 0; i < MAXPLAYERS; i++)
		D_NCSNetUpdateSingle(&players[i]);
	
	// Transmit Commands
	D_NCSNetUpdateSingleTic();
	
	// Update Networking
	D_NCUpdate();
	D_XNetUpdate();
}

/* D_NCSNetUpdateSingleTic() -- Single tic update */
void D_NCSNetUpdateSingleTic(void)
{
	size_t i, j, SID;
	D_NetPlayer_t* NetPlayer;
	ticcmd_t TicMerge;
	
	/* Transmit local tic command */
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
		{
			// Get and check netplayer
			NetPlayer = players[i].NetPlayer;
		
			if (!NetPlayer)
				continue;
		
			// Non-local player?
			if (NetPlayer->Type != DNPT_LOCAL && NetPlayer->Type != DNPT_BOT)
				continue;
		
			// Merge all local tics into a single source
			memset(&TicMerge, 0, sizeof(TicMerge));
			
			// Merge
			D_NCSNetMergeTics(&TicMerge, NetPlayer->LocalTicCmd, NetPlayer->LocalTicTotal);
			
			// Clear away
			NetPlayer->LocalTicTotal = 0;
			
			// Transmit this tic command
			D_NCSNetTicTransmit(NetPlayer, &TicMerge);
		}
}

/* D_NCSNetSetState() -- Set state of local players */
void D_NCSNetSetState(const D_NetState_t a_State)
{
	size_t i;
	
	/* Check */
	if (a_State < 0 || a_State >= NUMDNETSTATES)
		return;
	
	/* Set it for all local players */
	for (i = 0; i < MAXSPLITSCREEN; i++)
		if (g_Splits[i].Active && playeringame[g_Splits[i].Console])
			if (players[g_Splits[i].Console].NetPlayer)
				players[g_Splits[i].Console].NetPlayer->NetState = a_State;
}

/* D_NCSNetTicTransmit() -- Transmit tic command to server */
void D_NCSNetTicTransmit(D_NetPlayer_t* const a_NPp, ticcmd_t* const a_TicCmd)
{
	size_t i, SID;
	ticcmd_t* DestTic;
	ticcmd_t Merge;
	I_EventEx_t OSKEvent;
	D_NetClient_t* Server;
	D_BS_t* Stream;
	int32_t PNum;
	bool_t DidAngle;
	
	/* Check */
	if (!a_NPp || !a_TicCmd)
		return;
	
	/* Only transmit local commands */
	if (a_NPp->Type == DNPT_NETWORK)
		return;
	
	/* Determine Local Screen */
	PNum = (a_NPp->Player - players);
	for (SID = 0; SID < MAXSPLITSCREEN; SID++)
		if (g_Splits[SID].Active && (a_NPp->Player - players) == g_Splits[SID].Console)
			break;
		
	/* Create Synthetic OSK Events */
	// Moved high up in the event chain since this really does not belong here.
	// And this was a cheap hack to begin with anyway.
	// But never do it for bots, and only for those without joys
	if (a_NPp->Type == DNPT_LOCAL && !g_Splits[SID].JoyBound)
		if (CONL_OSKIsActive(SID) || M_ExPlayerUIActive(SID))
		{
			// These are player movement based
			// Right/Left Movement
			memset(&OSKEvent, 0, sizeof(OSKEvent));
	
			// Set type
			OSKEvent.Type = IET_SYNTHOSK;
			OSKEvent.Data.SynthOSK.PNum = SID;
	
			// Right/Left
			if ((a_TicCmd->Std.sidemove) >= (c_sidemove[0] >> 1) || (a_TicCmd->Std.BaseAngleTurn) <= -(c_angleturn[2] >> 1))
				OSKEvent.Data.SynthOSK.Right = 1;
			else if ((a_TicCmd->Std.sidemove) <= -(c_sidemove[0] >> 1) || (a_TicCmd->Std.BaseAngleTurn) >= (c_angleturn[2] >> 1))
				OSKEvent.Data.SynthOSK.Right = -1;
	
			// Up/Down
			if ((a_TicCmd->Std.forwardmove) <= -(c_forwardmove[0] >> 1))
				OSKEvent.Data.SynthOSK.Down = 1;
			else if ((a_TicCmd->Std.forwardmove) >= (c_forwardmove[0] >> 1))
				OSKEvent.Data.SynthOSK.Down = -1;
	
			// Press
			if (a_TicCmd->Std.buttons & BT_ATTACK)
				OSKEvent.Data.SynthOSK.Press = 1;
			
			// Cancel
			if (a_TicCmd->Std.buttons & BT_JUMP)
				OSKEvent.Data.SynthOSK.Cancel = 1;
	
			// Push Event
			if (OSKEvent.Data.SynthOSK.Right || OSKEvent.Data.SynthOSK.Down || OSKEvent.Data.SynthOSK.Press || OSKEvent.Data.SynthOSK.Cancel)	
				I_EventExPush(&OSKEvent);
			
			// OSK is active, so don't continue any further
			return;
		}
	
	// Add local command to end
	a_NPp->TicCmd[a_NPp->TicTotal++] = *a_TicCmd;
	
	// Merge it All
	memset(&Merge, 0, sizeof(Merge));
	D_NCSNetMergeTics(&Merge, a_NPp->TicCmd, a_NPp->TicTotal);
	
	// Set local view angle
	if (SID < MAXSPLITSCREEN)
		if (!a_NPp->Player || (a_NPp->Player && a_NPp->Player->mo && !a_NPp->Player->mo->reactiontime))
		{
			// Absolute Angles
			if (P_XGSVal(PGS_COABSOLUTEANGLE))
			{
				localangle[SID] += Merge.Std.BaseAngleTurn << 16;
				Merge.Std.angleturn = localangle[SID] >> 16;
			}
			
			// Doom Angles
			else
				Merge.Std.angleturn = Merge.Std.BaseAngleTurn;
		}
	
	// Set local aiming angle
	if (SID < MAXSPLITSCREEN)
	{
		if (Merge.Std.ResetAim)
			localaiming[SID] = 0;
		else
			localaiming[SID] += Merge.Std.BaseAiming << 16;
		Merge.Std.aiming = G_ClipAimingPitch(&localaiming[SID]);
		//Merge.aiming = localaiming[SID] >> 16;
	}
	
	// Only use this tic (single player game)
	a_NPp->TicCmd[0] = Merge;
	a_NPp->TicTotal = 1;
	a_NPp->XMitCount++;
}

/* D_NCSNetMergeTics() -- Merges all tic commands */
void D_NCSNetMergeTics(ticcmd_t* const a_DestCmd, const ticcmd_t* const a_SrcList, const size_t a_NumSrc)
{
//#define __REMOOD_SWIRVYANGLE
	size_t i, j;
	int32_t FM, SM, AT, AM, RANG;
	fixed_t xDiv;
	
	/* Check */
	if (!a_DestCmd || !a_SrcList || !a_NumSrc)
		return;
	
	/* Merge Variadic Stuff */
	// Super merging
	FM = SM = AT = AM = RANG = 0;
	for (j = 0; j < a_NumSrc; j++)
	{
		FM += a_SrcList[j].Std.forwardmove;
		SM += a_SrcList[j].Std.sidemove;
		RANG += a_SrcList[j].Std.angleturn;

#if defined(__REMOOD_SWIRVYANGLE)
		AT += a_SrcList[j].Std.BaseAngleTurn;
		AM += a_SrcList[j].Std.BaseAiming;
#else
		// Use the furthest aiming angle
		if (abs(a_SrcList[j].Std.BaseAngleTurn) > abs(AT))
			AT = a_SrcList[j].Std.BaseAngleTurn;
		if (abs(a_SrcList[j].Std.BaseAiming) > abs(AM))
			AM = a_SrcList[j].Std.BaseAiming;
#endif
		
		// Reset aim?
		a_DestCmd->Std.ResetAim |= a_SrcList[j].Std.ResetAim;
			
		// Merge weapon here
		if (!a_DestCmd->Std.XSNewWeapon[0])
			strncpy(a_DestCmd->Std.XSNewWeapon, a_SrcList[j].Std.XSNewWeapon, MAXTCWEAPNAME);
		
		// Clear slot and weapon masks (they OR badly)
		a_DestCmd->Std.buttons &= ~(BT_WEAPONMASK | BT_SLOTMASK);
		
		// Merge Buttons
		a_DestCmd->Std.buttons |= a_SrcList[j].Std.buttons;
		
		a_DestCmd->Std.aiming = a_SrcList[j].Std.aiming;
		
		// Use latest time codes
		if (a_SrcList[j].Ctrl.ProgramTic > a_DestCmd->Ctrl.ProgramTic)
			a_DestCmd->Ctrl.ProgramTic = a_SrcList[j].Ctrl.ProgramTic;
		if (a_SrcList[j].Ctrl.GameTic > a_DestCmd->Ctrl.GameTic)
			a_DestCmd->Ctrl.GameTic = a_SrcList[j].Ctrl.GameTic;
	}

	// Do some math
	xDiv = ((fixed_t)a_NumSrc) << FRACBITS;
	a_DestCmd->Std.forwardmove = FixedDiv(FM << FRACBITS, xDiv) >> FRACBITS;
	a_DestCmd->Std.sidemove = FixedDiv(SM << FRACBITS, xDiv) >> FRACBITS;
	a_DestCmd->Std.angleturn = a_SrcList[0].Std.angleturn;//FixedDiv(RANG << FRACBITS, xDiv) >> FRACBITS;
	
	/* Aiming is slightly different */
#if defined(__REMOOD_SWIRVYANGLE)
	// Divide some
	//AT /= ((int32_t)(a_NumSrc));
	
	// Cap
	if (AT > 32000)
		AT = 32000;
	else if (AT < -32000)
		AT = -32000;
	
	// Cap
	if (AM > 32000)
		AM = 32000;
	else if (AM < -32000)
		AM = -32000;
	
	// Try now
	a_DestCmd->Std.BaseAngleTurn = FixedDiv(AT << FRACBITS, xDiv) >> FRACBITS;
	a_DestCmd->Std.BaseAiming = FixedDiv(AM << FRACBITS, xDiv) >> FRACBITS;
#else
	// Use furthest angle
	//AT /= ((int32_t)(a_NumSrc));
	a_DestCmd->Std.BaseAngleTurn = AT;
	a_DestCmd->Std.BaseAiming = AM;
#endif
}

/* D_NCSAllocNetPlayer() -- Allocates a network player */
D_NetPlayer_t* D_NCSAllocNetPlayer(void)
{
	size_t i;
	uint8_t Char;
	D_NetPlayer_t* New;
	
	/* Allocate */
	New = Z_Malloc(sizeof(D_NetPlayer_t), PU_STATIC, NULL);
	
	/* Link */
	if (!l_FirstNetPlayer)
		l_FirstNetPlayer = New;
	else
	{
		l_FirstNetPlayer->ChainPrev = l_FirstNetPlayer;
		New->ChainNext = l_FirstNetPlayer;
		l_FirstNetPlayer = New;
	}
	
	/* Set properties */
	D_CMakeUUID(New->UUID);
	
	/* Return New */
	return New;
}

/* D_NCSFreeNetPlayer() -- Frees a net player */
void D_NCSFreeNetPlayer(D_NetPlayer_t* const a_NetPlayer)
{
	/* Check */
	if (!a_NetPlayer)
		return;
}

/* D_NCSFindNetPlayer() -- Finds a net player */
D_NetPlayer_t* D_NCSFindNetPlayer(const char* const a_Name)
{
	D_NetPlayer_t* Rover;
	
	/* Check */
	if (!a_Name)
		return NULL;
	
	/* Rove */
	for (Rover = l_FirstNetPlayer; Rover; Rover = Rover->ChainPrev)
	{
		// Match?
		if (strcmp(a_Name, Rover->UUID) == 0)
			return Rover;
			
		// Match?
		if (strcmp(a_Name, Rover->AccountName) == 0)
			return Rover;
	}
	
	/* Not Found */
	return NULL;
}

/* D_NCSFindNetPlayerByProcess() -- Find net player by process ID */
D_NetPlayer_t* D_NCSFindNetPlayerByProcess(const uint32_t a_ID)
{
	D_NetPlayer_t* Rover;
	
	/* Rove */
	for (Rover = l_FirstNetPlayer; Rover; Rover = Rover->ChainPrev)
	{
		// Match?
		if (Rover->ProcessID == a_ID)
			return Rover;
	}
	
	/* Not Found */
	return NULL;
}

/* D_NCSFindNetPlayerByUnique() -- Finds unique net player */
D_NetPlayer_t* D_NCSFindNetPlayerByUnique(const uint32_t a_ID)
{
	D_NetPlayer_t* Rover;
	
	/* Check */
	if (!a_ID)
		return NULL;
	
	/* Rove */
	for (Rover = l_FirstNetPlayer; Rover; Rover = Rover->ChainPrev)
	{
		// Match?
		if (Rover->UniqueID == a_ID)
			return Rover;
	}
	
	/* Not Found */
	return NULL;
}

/* D_NCSFindSplitByProcess() -- Finds split screen by process */
int8_t D_NCSFindSplitByProcess(const uint32_t a_ID)
{
	int i;
	
	/* Check */
	if (!a_ID)
		return -1;
	
	/* Loop */
	for (i = 0; i < MAXSPLITSCREEN; i++)
		if (D_ScrSplitHasPlayer(i))
			if (g_Splits[i].ProcessID == a_ID)
				return i;
	
	/* Not found */
	return -1;
}

/* D_NCSIterSpec() -- Iterate spectators */
D_NetPlayer_t* D_NCSIterSpec(D_NetPlayer_t* const a_At)
{
	D_NetPlayer_t* Rover;
	
	/* Rove */
	for (Rover = (!a_At ? l_FirstNetPlayer : a_At->ChainPrev); Rover; Rover = Rover->ChainPrev)
	{
		// Is a spectator?
		if (Rover->Type == DNPT_SPECTATOR)
			return Rover;
	}
	
	/* Not Found */
	return NULL;
}

/* D_NCRemoveSplit() -- Removes Split */
void D_NCRemoveSplit(const int32_t a_Split, const bool_t a_Demo)
{
	int i;
	
	/* Check */
	if (a_Split < 0 || a_Split >= MAXSPLITSCREEN)
		return;	
	
	/* Not in demo */
	if (!a_Demo)
	{
		// Move splits down, to replace this split
		for (i = a_Split; i < MAXSPLITSCREEN; i++)
			// Last spot?
			if (i == MAXSPLITSCREEN - 1)
				memset(&g_Splits[i], 0, sizeof(g_Splits[i]));
			
			// Move the stuff from the next spot over this one
			else
				memmove(&g_Splits[i], &g_Splits[i + 1], sizeof(g_Splits[i]));
	}
	
	/* In demo */
	else
	{
		// Move splits down, to replace this split
		for (i = a_Split; i < MAXSPLITSCREEN; i++)
			// Last spot?
			if (i == MAXSPLITSCREEN - 1)
			{
				// Make inactive
				g_Splits[i].Active = false;
			}
			
			// Move the stuff from the next spot over this one
			else
			{
				// Grab non breaking stuff from demos over
				g_Splits[i].Active = g_Splits[i + 1].Active;
				g_Splits[i].Console = g_Splits[i + 1].Console;
				g_Splits[i].Display = g_Splits[i + 1].Display;
			}
	}
	
	/* Correctsplit screen */
	// Subtract the removed player
	if (g_SplitScreen >= 0)
		g_SplitScreen--;
	
	// Correct visual display
	R_ExecuteSetViewSize();
}

/* D_NCResetSplits() -- Resets all splits */
void D_NCResetSplits(const bool_t a_Demo)
{
	int i;
	
	/* Wipe all splits */
	for (i = MAXSPLITSCREEN; i > 0; i--)
		D_NCRemoveSplit(i - 1, a_Demo);
}

/* D_NCSGetPlayerName() -- Get player name */
const char* D_NCSGetPlayerName(const uint32_t a_PlayerID)
{
	/* Check */
	if (a_PlayerID < 0 || a_PlayerID >= MAXPLAYERS)
		return NULL;
	
	/* Player is in game */
	if (playeringame[a_PlayerID])
	{
		// Network Player
		if (players[a_PlayerID].NetPlayer)
			if (players[a_PlayerID].NetPlayer->DisplayName[0])
				return players[a_PlayerID].NetPlayer->DisplayName;
		
		// Try from profiles
		if (players[a_PlayerID].ProfileEx)
			if (players[a_PlayerID].ProfileEx->DisplayName[0])
				return players[a_PlayerID].ProfileEx->DisplayName;
	}
	
	/* Return default */
	if (player_names[a_PlayerID][0])
		return player_names[a_PlayerID];
	
	/* Return Unknown */
	return "Unnamed Player";
}

/* D_TicCmdFillWeapon() -- Fills weapon for tic command */
void D_TicCmdFillWeapon(ticcmd_t* const a_Target, const int32_t a_ID)
{
	strncpy(a_Target->Std.XSNewWeapon, wpnlev1info[a_ID]->ClassName, MAXTCWEAPNAME);
}

/* D_CMakePureRandom() -- Create a pure random number */
uint32_t D_CMakePureRandom(void)
{
//#define __REMOOD_RANDRAWBITS
	uint32_t Garbage, i;
	uint32_t* RawBits;
	void* StackP;
	
	/* Allocate Raw Bits */
#ifdef __REMOOD_RANDRAWBITS
	RawBits = (uint32_t*)I_SysAlloc(sizeof(*RawBits) * 16);
#endif
	
	/* Attempt number generation */
	// Init
	Garbage = M_Random();
	Garbage <<= 8;
	Garbage |= M_Random();
	Garbage <<= 8;
	Garbage |= M_Random();
	Garbage <<= 8;
	Garbage |= M_Random();
	
	// Current Time
	Garbage ^= ((int)I_GetTime() * (int)I_GetTime());
	
	// Address of this function
	Garbage ^= (uint32_t)(((uintptr_t)D_CMakePureRandom) * ((uintptr_t)D_CMakePureRandom));
	
	// Address of garbage
	Garbage ^= (uint32_t)(((uintptr_t)&Garbage) * ((uintptr_t)&Garbage));
	
	// Current PID
	Garbage ^= ((uint32_t)I_GetCurrentPID() * (uint32_t)I_GetCurrentPID());
	
	// Stack Pointer
	StackP = alloca(1);
	Garbage ^= (uint32_t)((uintptr_t)StackP ^ ((uintptr_t)StackP >> UINT64_C(32)));
	
#ifdef __REMOOD_RANDRAWBITS
	// Allocated Data
	if (RawBits)
	{
		// Raw bits address
		Garbage ^= (uint32_t)(((uintptr_t)RawBits) * ((uintptr_t)RawBits));
	
		// Raw bits data (unitialized memory)
		for (i = 0; i < 16; i++)
			Garbage ^= RawBits[i];
	
		// Cleanup
		I_SysFree(RawBits);
	}
#endif
	
	/* Return the garbage number */
	return Garbage;
}

/* D_CMakeUUID() -- Makes a UUID */
void D_CMakeUUID(char* const a_Buf)
{
	size_t i, FailCount;
	uint8_t Char;
	uint32_t Garbage;
	
	/* Generate a hopefully random ID */
	for (i = 0; i < (MAXPLAYERNAME * 2) - 1; i++)
	{
		// Hopefully random enough
		Garbage = D_CMakePureRandom();
		Char = (((int)(M_Random())) + Garbage);
		FailCount = 0;
		
		// Limit Char
		while (!((Char >= '0' && Char <= '9') || (Char >= 'a' && Char <= 'z') || (Char >= 'A' && Char <= 'Z')))
		{
			if (Char <= 'A')
				Char += 15;
			else if (Char >= 'z')
				Char -= 15;
			else
				Char ^= D_CMakePureRandom();
			
			if (++FailCount >= 10)
				if (M_Random() & 1)
					Char = 'A' + (M_Random() % ('Y' - 'A'));
				else
					Char = 'a' + (M_Random() % ('y' - 'a'));
		}
		
		// Last character is the same as this?
		if (i > 0 && Char == a_Buf[i - 1])
		{
			i--;
			continue;
		}
		
		// Set as
		a_Buf[i] = Char;
		
		// Sleep for some unknown time
		I_WaitVBL(M_Random() & 1);
	}
}


