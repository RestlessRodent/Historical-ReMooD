// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION:

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "sn.h"
#include "d_netcmd.h"
#include "p_demcmp.h"
#include "z_zone.h"
#include "d_prof.h"
#include "info.h"
#include "g_state.h"
#include "d_player.h"
#include "v_video.h"
#include "console.h"
#include "tables.h"
#include "d_main.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_mobj.h"
#include "st_stuff.h"
#include "cl.h"

/****************
*** CONSTANTS ***
****************/

static const fixed_t c_forwardmove[2] = { 25, 50 };
static const fixed_t c_sidemove[2] = { 24, 40 };
static const fixed_t c_angleturn[3] = { 640, 1280, 320 };	// + slow turn
#define MAXPLMOVE       (c_forwardmove[1])
#define MAXLOCALJOYS	MAXJOYSTICKS			// Max joysticks handled

/*************
*** LOCALS ***
*************/

static bool_t l_PermitMouse = false;			// Use mouse input
static int32_t l_MouseMove[2] = {0, 0};			// Mouse movement (x/y)
static uint32_t l_MouseButtons[2];				// Mouse buttons down (sng/dbl)
static tic_t l_MouseLastTime[32];				// Last time pressed
static bool_t l_KeyDown[NUMIKEYBOARDKEYS];		// Keys that are down
static uint32_t l_JoyButtons[MAXLOCALJOYS];		// Local Joysticks
static int16_t l_JoyAxis[MAXLOCALJOYS][MAXJOYAXIS];
static CONCTI_Inputter_t* l_ChatBox[MAXSPLITS];	// Splitscreen chat

/****************
*** FUNCTIONS ***
****************/

/* SN_ChatDrawer() -- Draws chat line */
void SN_ChatDrawer(const int8_t a_Screen, const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H)
{
	uint32_t Flags;
	
	/* Check */
	if (a_Screen < 0 || a_Screen >= MAXSPLITS)
		return;
	
	/* Not chatting? */
	if (!g_Splits[a_Screen].ChatMode)
		return;
	
	/* Draw box */
	if (l_ChatBox[a_Screen])
	{	
		// Base flags
		Flags = 0;
		
		// If team chat, draw colorized!
		if (g_Splits[a_Screen].ChatMode == 2 || g_Splits[a_Screen].ChatMode == 3)
			if (!g_Splits[a_Screen].Port || !g_Splits[a_Screen].Port->Player)		// Spec
				Flags = VFO_COLOR(VEX_MAP_WHITE);
			else		// Team
			{
				Flags = P_GetPlayerTeam(&players[g_Splits[a_Screen].Console]);
				P_GetTeamInfo(Flags, &Flags, NULL);
				Flags = VFO_PCOL(Flags);
			}
		
		// Draw it
		CONCTI_DrawInput(l_ChatBox[a_Screen], Flags, a_X, a_H - V_FontHeight(l_ChatBox[a_Screen]->Font), a_W);
	}
}

/* SN_ClearChat() -- Clears player chat */
void SN_ClearChat(const int32_t a_Screen)
{
	/* Check */
	if (a_Screen < 0 || a_Screen >= MAXSPLITS)
		return;
	
	/* Get out of chat mode */
	g_Splits[a_Screen].ChatMode = 0;
	g_Splits[a_Screen].ChatTargetID = 0;
	g_Splits[a_Screen].ChatTimeOut = g_ProgramTic + (TICRATE >> 1);
	
	/* Remove line */
	if (l_ChatBox[a_Screen])
		CONCTI_SetText(l_ChatBox[a_Screen], "");
}

/* SN_CONCTIChatLine() -- For when enter is pressed */
static bool_t SN_CONCTIChatLine(struct CONCTI_Inputter_s* a_Input, const char* const a_Text)
{
	/* Handle chat string and send to server (or local) */
	SN_SendChat(g_Splits[a_Input->Screen].Port, (g_Splits[a_Input->Screen].ChatMode == 2 || g_Splits[a_Input->Screen].ChatMode == 3), a_Text);
	
	/* Leave Chat Mode */
	// Done with it, no use
	SN_ClearChat(a_Input->Screen);
	
	/* always keep box */
	return true;
}

/* SN_HandleEvent() -- Handle advanced events */
bool_t SN_HandleEvent(const I_EventEx_t* const a_Event, CL_Socket_t* const a_Sock)
{
	int32_t ButtonNum, LocalJoy;
	uint32_t Bit;
	
	/* Check */
	if (!a_Event)
		return false;
	
	/* Handle chatting for players */
	// Keyboard for P1 only
	if (a_Event->Type == IET_KEYBOARD)
		Bit = 1;
	
	// Synth OSK always work
	else if (a_Event->Type == IET_SYNTHOSK)
		Bit = a_Event->Data.SynthOSK.SNum + 1;
	
	// No player specified (mouse, joy, etc.)
	else
		Bit = 0;	// Non-chat stuff
	
	// Doing chat?
	if (Bit && !demoplayback)
	{
		// Bit is off by one
		Bit -= 1;
		
		// If they are chatting
		if (g_Splits[Bit].ChatMode && D_ScrSplitHasPlayer(Bit))
		{
			// Cancel chat?
			if ((a_Event->Type == IET_KEYBOARD && a_Event->Data.Keyboard.KeyCode == IKBK_ESCAPE && a_Event->Data.Keyboard.Down) || (a_Event->Type == IET_SYNTHOSK && a_Event->Data.SynthOSK.Cancel))
			{
				SN_ClearChat(Bit);
				return true;
			}
			
			// Need to create chat box?
			if (!l_ChatBox[Bit])
			{
				l_ChatBox[Bit] = CONCTI_CreateInput(16, SN_CONCTIChatLine, &l_ChatBox[Bit]);
				l_ChatBox[Bit]->Screen = Bit;
				l_ChatBox[Bit]->Font = VFONT_SMALL;
			}
			
			// Handle event for inputter
			return CONCTI_HandleEvent(l_ChatBox[Bit], a_Event);
		}
		
		// Profile select
	}
	
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
			return true;
			
			// Keyboard
		case IET_KEYBOARD:
			if (a_Event->Data.Keyboard.KeyCode >= 0 && a_Event->Data.Keyboard.KeyCode < NUMIKEYBOARDKEYS)
				l_KeyDown[a_Event->Data.Keyboard.KeyCode] = a_Event->Data.Keyboard.Down;
			return true;
			
#if 0
			// Joystick
		case IET_JOYSTICK:
			// Get local joystick
			LocalJoy = a_Event->Data.Joystick.JoyID;
			
			// Now determine which action
			if (LocalJoy >= 0 && LocalJoy < MAXLOCALJOYS)
			{
				// Not bound? Then remove anything remembered (prevents stuckness)
				if (!D_JoyToPort(LocalJoy + 1))
				{
					l_JoyButtons[LocalJoy] = 0;
					memset(&l_JoyAxis[LocalJoy], 0, sizeof(l_JoyAxis[LocalJoy]));
					break;
				}
				
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
			return true;
#endif
		
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
// This uses the fields in PI_wep_t for ordering info
static uint8_t DS_XNetNextWeapon(player_t* player, int step)
{
	size_t g, w, fw, BestNum;
	int32_t s, StepsLeft, StepsAdd, BestDiff, ThisDiff;
	size_t MostOrder, LeastOrder;
	bool_t Neg;
	PI_wep_t** weapons;
	
	/* No player? */
	if (!player)
		return 0;
	
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
static bool_t GAMEKEYDOWN(D_Prof_t* const a_Profile, const uint8_t a_SID, const uint8_t a_Key)
{
	static bool_t Recoursed;
	int8_t MoreDown;
	size_t i;
	uint32_t CurrentButton;
	uint32_t KeyBit, JoyBit, MouseBit, DMouseBit;
	
	/* Determine if more key is down */
	// But do not infinite loop here
	MoreDown = 0;
	if (!Recoursed)
	{
		// More Down?
		Recoursed = true;
		if (GAMEKEYDOWN(a_Profile, a_SID, DPEXIC_MORESTUFF))
			MoreDown = 1;
		Recoursed = false;
		
		// Even more down?
		if (MoreDown == 1)
		{
			Recoursed = true;
			if (GAMEKEYDOWN(a_Profile, a_SID, DPEXIC_MOREMORESTUFF))
				MoreDown = 2;
			Recoursed = false;
		}
	}
	
	// If key is more more stuff, check more stuff bind
		// This is for multi-key combo buttons
	if (a_Key == DPEXIC_MOREMORESTUFF)
		MoreDown = 1;
	
	/* Determine check shifts */
	// Standard
	if (MoreDown <= 0 || MoreDown > 2)
	{
		KeyBit = PRFKBIT_KEY;
		JoyBit = PRFKBIT_JOY;
		MouseBit = PRFKBIT_MOUSE;
		DMouseBit = PRFKBIT_DMOUSE;
	}
	
	// More
	else if (MoreDown == 1)
	{
		KeyBit = PRFKBIT_KEYP;
		JoyBit = PRFKBIT_JOYP;
		MouseBit = PRFKBIT_MOUSEP;
		DMouseBit = PRFKBIT_DMOUSEP;
	}
	
	// Extra
	else if (MoreDown == 2)
	{
		KeyBit = PRFKBIT_KEYX;
		JoyBit = PRFKBIT_JOYX;
		MouseBit = PRFKBIT_MOUSEX;
		DMouseBit = PRFKBIT_DMOUSEX;
	}
	
	/* Check Keyboard */
	for (i = 0; i < 4; i++)
		if ((D_ProfileRawControl(a_Profile, a_Key, i) & PRFKBIT_MASK) == KeyBit)
		{
			// Get current key
			CurrentButton = (D_ProfileRawControl(a_Profile, a_Key, i) & PRFKBIT_VMASK);
			
			// Check if key is down
			if (CurrentButton >= 0 && CurrentButton < NUMIKEYBOARDKEYS)
				if (l_KeyDown[CurrentButton])
					return true;
		}
	
	/* Check Joysticks */
	//if (a_Profile->Flags & DPEXF_GOTJOY)
		//if (a_Profile->JoyControl >= 0 && a_Profile->JoyControl < 4)
	if (a_SID >= 0 && a_SID < MAXSPLITS && g_Splits[a_SID].JoyBound)
		if (g_Splits[a_SID].JoyID >= 1 && g_Splits[a_SID].JoyID <= MAXLOCALJOYS)
			if (l_JoyButtons[g_Splits[a_SID].JoyID - 1])
				for (i = 0; i < 4; i++)
					if ((D_ProfileRawControl(a_Profile, a_Key, i) & PRFKBIT_MASK) == JoyBit)
					{
						// Get current button
						CurrentButton = (D_ProfileRawControl(a_Profile, a_Key, i) & PRFKBIT_VMASK);
				
						// Button pressed?
						if (CurrentButton >= 0 && CurrentButton < 32)
							if (l_JoyButtons[g_Splits[a_SID].JoyID - 1] & (1 << CurrentButton))
								return true;
					}
				
	/* Check Mice */
	if (D_ProfileFlags(a_Profile) & DPEXF_GOTMOUSE)
		if (l_MouseButtons[0] || l_MouseButtons[1])
			for (i = 0; i < 4; i++)
			{
				// Single
				if ((D_ProfileRawControl(a_Profile, a_Key, i) & PRFKBIT_MASK) == MouseBit)
				{
					// Get current button
					CurrentButton = (D_ProfileRawControl(a_Profile, a_Key, i) & PRFKBIT_VMASK);
		
					// Button pressed?
					if (CurrentButton >= 0 && CurrentButton < 32)
						if (l_MouseButtons[0] & (1 << CurrentButton))
							return true;
				}
		
				// Double
				if ((D_ProfileRawControl(a_Profile, a_Key, i) & PRFKBIT_MASK) == DMouseBit)
				{
					// Get current button
					CurrentButton = (D_ProfileRawControl(a_Profile, a_Key, i) & PRFKBIT_VMASK);
		
					// Button pressed?
					if (CurrentButton >= 0 && CurrentButton < 32)
						if (l_MouseButtons[1] & (1 << CurrentButton))
							return true;
				}
			}
	
	/* Not pressed */
	return false;
}

#define GKD(k) GAMEKEYDOWN(Profile, SID, (k))

/* SN_PortTicCmd() -- Builds tic command for port */
void SN_PortTicCmd(SN_Port_t* const a_Port, ticcmd_t* const a_TicCmd)
{
#define MAXWEAPONSLOTS 12
	D_Prof_t* Profile;
	player_t* Player, *SpyCon, *SpyPOV, *SpyFake;
	int32_t TargetMove;
	size_t i, PID, SID;
	int8_t SensMod, MoveMod, MouseMod, MoveSpeed, TurnSpeed;
	int32_t SideMove, ForwardMove, BaseAT, BaseAM, NegMod, FlyMove;
	bool_t IsTurning, GunInSlot, ResetAim;
	int slot, j, l, k;
	PI_wepid_t newweapon;
	PI_wepid_t SlotList[MAXWEAPONSLOTS];
	D_SplitInfo_t* SSplit;
	
	/* Check */
	if (!a_Port || !a_TicCmd)
		return;
	
	/* Obtain profile */
	Profile = a_Port->Profile;
	Player = a_Port->Player;
	
	/* Find Player ID */
	PID = a_Port->Player - players;
	
	/* Find Screen ID */
	for (SID = 0; SID < MAXSPLITS; SID++)
		if (D_ScrSplitVisible(SID))
			if (g_Splits[SID].Port == a_Port)
				break;
	
	// Not found?
	if (SID >= MAXSPLITS)
	{
		// Force first screen in demo?
		if (demoplayback)
			SID = 0;
		
		// Otherwise don't make any commands
		else
			return;
	}
	
	// Quick ref
	SSplit = &g_Splits[SID];
	
	// No profile? Try from split, then from player
	if (!Profile)
		if (!(Profile = SSplit->Profile))
			if (!(Profile = Player->ProfileEx))
				return;
	
	/* Chatting? */
	// Not in chat mode
	if (!SSplit->ChatMode)
	{
		i = 0;

		// To All
		if (GKD(DPEXIC_CHAT))
			i = 1;

		// To Team
		else if (GKD(DPEXIC_TEAMCHAT))
			i = 2;

		// Starting chat?
		if (i)
		{
			// Initiate Chat Mode
			SSplit->ChatMode = i;
			SSplit->ChatTargetID = 0;
			
			// If player 1 is chatting, let go of all keyboard keys
				// Otherwise, stuck keys and re-chatting when chat is done!
			if (SID == 0)
				for (i = 0; i < NUMIKEYBOARDKEYS; i++)
					l_KeyDown[i] = false;
			
			return;		// Do not continue processing events
		}
	}
	
	// In Chat mode
	else
	{
		// Do not handle player events, make them not move as if all keys are down
		return;
	}
	
	/* Reset Some Things */
	SideMove = ForwardMove = BaseAT = BaseAM = FlyMove = 0;
	IsTurning = ResetAim = false;
	
	/* Modifiers */
	// Mouse Sensitivity
	SensMod = 0;
	
	// Movement Modifier
	if (GKD(DPEXIC_MOVEMENT))
		MoveMod = 1;
	else
		MoveMod = 0;
	
	// Mouse Modifier
	if (GKD(DPEXIC_LOOKING))
		MouseMod = 2;
	else if (MoveMod)
		MouseMod = 1;
	else 
		MouseMod = 0;
	
	// Moving Speed
	if (GKD(DPEXIC_SPEED))
		MoveSpeed = 1;
	else
		MoveSpeed = 0;
	
	// Turn Speed
	if ((D_ProfileUseSlowTurn(Profile)) && g_ProgramTic < (SSplit->TurnHeld + D_ProfileSlowTurnTime(Profile)))
		TurnSpeed = 2;
	else if (MoveSpeed)
		TurnSpeed = 1;
	else
		TurnSpeed = 0;
	
	// Auto-run? If so, invert speeds
	if (D_ProfileUseAutoRun(Profile))
	{
		MoveSpeed = !MoveSpeed;
		
		if (TurnSpeed != 2)
			TurnSpeed = !TurnSpeed;
	}
	
	// TODO: Redo interface input system and profile for this
#if 0
	/* Player has joystick input? */
	// Read input for all axis
	if (SID >= 0 && SID < MAXSPLITS && SSplit->JoyBound)
		if (SSplit->JoyID >= 1 && SSplit->JoyID <= MAXLOCALJOYS)
			for (i = 0; i < MAXJOYAXIS; i++)
			{
				// Modify with sensitivity
				TargetMove = ((float)l_JoyAxis[SSplit->JoyID - 1][i]) * (((float)Profile->JoySens[SensMod]) / 100.0);
			
				// Which movement to perform?
				NegMod = 1;
				switch (Profile->JoyAxis[MouseMod][i])
				{
						// Movement
					case DPEXCMA_NEGMOVEX:
					case DPEXCMA_NEGMOVEY:
						NegMod = -1;
						
					case DPEXCMA_MOVEX:
					case DPEXCMA_MOVEY:
						// Movement is fractionally based
						TargetMove = (((float)TargetMove) / ((float)32767.0)) * ((float)c_forwardmove[(Profile->JoyAutoRun ? 1 : MoveSpeed)]);
						TargetMove *= NegMod;
					
						// Now which action really?
						if (Profile->JoyAxis[MouseMod][i] == DPEXCMA_MOVEX ||
								Profile->JoyAxis[MouseMod][i] == DPEXCMA_NEGMOVEX)
							SideMove += TargetMove;
						else
							ForwardMove -= TargetMove;
						break;
					
						// Looking Left/Right
					case DPEXCMA_NEGLOOKX:
						NegMod = -1;
						
					case DPEXCMA_LOOKX:
						TargetMove = (((float)TargetMove) / ((float)32767.0)) * ((float)c_angleturn[(Profile->JoyAutoRun ? 1 : TurnSpeed)]);
						TargetMove *= NegMod;
						IsTurning = true;
						BaseAT -= TargetMove;
						break;
					
						// Looking Up/Down
					case DPEXCMA_NEGLOOKY:
						NegMod = -1;
						
					case DPEXCMA_LOOKY:
#if 0
						TargetMove = (((double)TargetMove) / ((double)32767.0)) *
						
						((double)Profile->LookUpDownSpeed) / (double;
						TargetMove *= NegMod;
						BaseAM += (TargetMove * NegMod);
#endif
						break;
						
						// Panning Up/Down (Linear)
					case DPEXCMA_NEGPANY:
						NegMod = -1;
						
					case DPEXCMA_PANY:
						BaseAM = (((double)l_JoyAxis[SSplit->JoyID - 1][i]) / ((double)32767.0)) * ((double)5856.0);
						BaseAM *= -1 * NegMod;
						
						// Make sure panning look is set
						a_TicCmd->Std.buttons |= BT_PANLOOK;
						break;
						
						// Panning Up/Down (Angular)
					case DPEXCMA_NEGANGPANY:
						NegMod = -1;
						
					case DPEXCMA_ANGPANY:
						// Get angle to extract
						TargetMove = abs(l_JoyAxis[SSplit->JoyID - 1][i]) >> 2;
						
						// Cap to valid precision in LUT
						if (TargetMove >= 8192)
							TargetMove = 8191;
						else if (TargetMove < 0)
							TargetMove = 0;
						
						// Extract from LUT
						BaseAM = (((double)c_AngLUT[TargetMove]) / ((double)32767.0)) * ((double)5856.0);
						BaseAM *= -1 * NegMod;
						
						// Negative?
						if (l_JoyAxis[SSplit->JoyID - 1][i] < 0)
							BaseAM *= -1;
						
						// Make sure panning look is set
						a_TicCmd->Std.buttons |= BT_PANLOOK;
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
			NegMod = 1;
			switch (Profile->MouseAxis[MouseMod][i])
			{
					// Strafe Left/Right
				case DPEXCMA_NEGMOVEX:
					NegMod = -1;
						
				case DPEXCMA_MOVEX:
					SideMove += TargetMove * NegMod;
					break;
					
					// Move Forward/Back
				case DPEXCMA_NEGMOVEY:
					NegMod = -1;
					
				case DPEXCMA_MOVEY:
					ForwardMove += TargetMove * NegMod;
					break;
					
					// Left/Right Look
				case DPEXCMA_NEGLOOKX:
					NegMod = -1;
				
				case DPEXCMA_LOOKX:
					BaseAT -= TargetMove * 8 * NegMod;
					break;
					
					// Up/Down Look
				case DPEXCMA_NEGLOOKY:
					NegMod = -1;
					
				case DPEXCMA_LOOKY:
					BaseAM += (TargetMove * NegMod) << 3;
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
#endif
	
	/* Handle Player Control Keyboard Stuff */
	// Weapon Attacks
	if (GKD(DPEXIC_ATTACK))
		a_TicCmd->Std.buttons |= BT_ATTACK;
	
	// Use
	if (GKD(DPEXIC_USE))
		a_TicCmd->Std.buttons |= BT_USE;
	
	// Jump
	if (GKD(DPEXIC_JUMP))
		a_TicCmd->Std.buttons |= BT_JUMP;
	
	// Suicide
	if (GKD(DPEXIC_SUICIDE))
		a_TicCmd->Std.buttons |= BT_SUICIDE;
	
	// Keyboard Turning
	if (GKD(DPEXIC_TURNLEFT))
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
	
	if (GKD(DPEXIC_TURNRIGHT))
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
	
	// 180 Degree Turn (don't allow repeat on it, otherwise it is useless)
	slot = GKD(DPEXIC_TURNSEMICIRCLE);
	
	if (!SSplit->Turned180 && slot)
	{
		BaseAT = 0x7FFF;
		IsTurning = true;
		SSplit->Turned180 = true;
	}
	else if (SSplit->Turned180 && !slot)
		SSplit->Turned180 = false;
	
	// Keyboard Moving
	if (GKD(DPEXIC_STRAFELEFT))
		SideMove -= c_sidemove[MoveSpeed];
	if (GKD(DPEXIC_STRAFERIGHT))
		SideMove += c_sidemove[MoveSpeed];
	if (GKD(DPEXIC_FORWARDS))
		ForwardMove += c_forwardmove[MoveSpeed];
	if (GKD(DPEXIC_BACKWARDS))
		ForwardMove -= c_forwardmove[MoveSpeed];
		
	// Looking
	if (GKD(DPEXIC_LOOKCENTER))
		ResetAim = true;
		//localaiming[SID] = 0;
	else
	{
		if (GKD(DPEXIC_LOOKUP))
			BaseAM += D_ProfileUseLookSpring(Profile) >> 16;
		
		if (GKD(DPEXIC_LOOKDOWN))
			BaseAM -= D_ProfileUseLookSpring(Profile) >> 16;
	}
	
	// Flying
		// Up
	if (GKD(DPEXIC_FLYUP))
		FlyMove += 5;
		
		// Down
	if (GKD(DPEXIC_FLYDOWN))
		FlyMove -= 5;
		
		// Land
	if (GKD(DPEXIC_LAND))
		a_TicCmd->Std.buttons |= BT_FLYLAND;
	
	// Weapons
	if (Player)
	{
		// Next
		if (GKD(DPEXIC_NEXTWEAPON))
		{
			// Set switch
			a_TicCmd->Std.buttons |= BT_CHANGE;
			D_TicCmdFillWeapon(a_TicCmd, DS_XNetNextWeapon(Player, 1));
		}
		
		// Prev
		else if (GKD(DPEXIC_PREVWEAPON))
		{
			// Set switch
			a_TicCmd->Std.buttons |= BT_CHANGE;
			D_TicCmdFillWeapon(a_TicCmd, DS_XNetNextWeapon(Player, -1));
		}
		
		// Best Gun
		else if (GKD(DPEXIC_BESTWEAPON))
		{
			newweapon = P_PlayerBestWeapon(Player, true);
		
			if (newweapon != Player->readyweapon)
			{
				a_TicCmd->Std.buttons |= BT_CHANGE;
				D_TicCmdFillWeapon(a_TicCmd, newweapon);
			}
		}
		
		// Worst Gun
		else if (GKD(DPEXIC_WORSTWEAPON))
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
				if (GKD(i))
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
	}
	
	// Inventory
	if (GKD(DPEXIC_NEXTINVENTORY))
		a_TicCmd->Std.InventoryBits = TICCMD_INVRIGHT;
	else if (GKD(DPEXIC_PREVINVENTORY))
		a_TicCmd->Std.InventoryBits = TICCMD_INVLEFT;
	else if (GKD(DPEXIC_USEINVENTORY))
		a_TicCmd->Std.InventoryBits = TICCMD_INVUSE;
	
	/* Handle special functions */
	// Show Scores
	if (GKD(DPEXIC_TOPSCORES))
		SSplit->Scores = 1;
	else if (GKD(DPEXIC_BOTTOMSCORES))
		SSplit->Scores = -1;
	else
		SSplit->Scores = 0;
	
	// Automap
	if (GKD(DPEXIC_AUTOMAP))
	{
		// Don't flash the automap like crazy
		if (!SSplit->MapKeyStillDown)
		{
			// Map not active, activate
			if (!SSplit->AutomapActive)
			{
				SSplit->AutomapActive = true;
				SSplit->OverlayMap = false;
			}
			
			// Is active
			else
			{
				// Overlay now active, activate
				if (!SSplit->OverlayMap)
					SSplit->OverlayMap = true;
				
				// Otherwise, stop the map
				else
					SSplit->AutomapActive = false;
			}
			
			// Place key down to prevent massive flashing
			SSplit->MapKeyStillDown = true;
		}
	}
	else
		SSplit->MapKeyStillDown = false;
	
	// Coop Spy
	if (GKD(DPEXIC_COOPSPY))
	{
		// Only every half second
		if (g_ProgramTic > (SSplit->CoopSpyTime + (TICRATE >> 1)))
		{
			// Get current POV
			SpyPOV = P_SpecGetPOV(SID);
			SpyFake = P_SpecGet(SID);
			
			// Get current player
			SpyCon = a_Port->Player;
			
			if (!SpyCon)
				SpyCon = SpyFake;
			
			// In spectator mode
			if (!a_Port->Player)
			{
				// Go through all players
					// If watching self, find first player
					// If watching someone, find next player
				for (j = ((SpyPOV == SpyFake) ? 0 : SSplit->Display + 1); j < MAXPLAYERS; j++)
					if (playeringame[j])
					{
						SSplit->Display = j;
						SpyPOV = &players[SSplit->Display];
						break;
					}
				
				// Nobody?
				if (j >= MAXPLAYERS)
				{
					SSplit->Display = -1;
					SpyPOV = SpyFake;
				}
				
				else
					SpyPOV = &players[SSplit->Display];
			}
			
			// Normal Game Mode
			else
			{
				j = 0;
				do
				{
					SSplit->Display = (SSplit->Display + 1) % MAXPLAYERS;
					j++;
				} while (j < MAXPLAYERS && (!playeringame[SSplit->Display] || (!P_SameTeam(&players[SSplit->Console], &players[SSplit->Display]))));
				
				// Change POV
				SpyPOV = &players[SSplit->Display];
			}
			
			// Print Message
			CONL_PrintF("%sYou are now watching %s.\n",
					(SID == 3 ? "\x6" : (SID == 2 ? "\x5" : (SID == 1 ? "\x4" : ""))),
					(SpyCon == SpyPOV ? "Yourself" : D_NCSGetPlayerName(SSplit->Display))
				);
			
			// Reset timeout
			SSplit->CoopSpyTime = g_ProgramTic + (TICRATE >> 1);
		}
	}
	
	// Key is unpressed to reduce time
	else
		SSplit->CoopSpyTime = 0;
	
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
	a_TicCmd->Std.FlySwim = FlyMove;
	
	/* Slow turning? */
	if (!IsTurning)
		SSplit->TurnHeld = g_ProgramTic;
	
	/* Turning */
	a_TicCmd->Std.BaseAngleTurn = BaseAT;
	a_TicCmd->Std.BaseAiming = BaseAM;
	
	if (ResetAim)
		a_TicCmd->Std.buttons |= BT_RESETAIM;
	
	/* Handle Look Spring */
	// This resets aim to center once you move
	if (D_ProfileUseLookSpring(Profile))
		if (!BaseAM && (abs(ForwardMove) >= c_forwardmove[0] || abs(SideMove) >= c_sidemove[0]))
		{
			a_TicCmd->Std.BaseAiming = 0;
			a_TicCmd->Std.buttons |= BT_RESETAIM;
		}
	
#undef MAXWEAPONSLOTS
}


/* SN_TicBufSum() -- Calculates checksum of Tic Buffer */
uint32_t SN_TicBufSum(SN_TicBuf_t* const a_TicBuf,  const SN_TicBufVersion_t a_VersionNum, const uint32_t a_Players)
{
	int32_t i;
	uint32_t RetVal = UINT32_C(0xDEADBEEF);
	ticcmd_t* TicCmd;
	
	/* Go through players */
	for (i = 0; i < MAXPLAYERS; i++)
	{
		// Commands
		TicCmd = &a_TicBuf->Tics[i];
		
		// Not playing?
		if (!(a_Players & (1 << i)))
		{
			if (a_VersionNum >= DXNTBV_VER20130731)
				RetVal ^= UINT32_C(0xBEEF1337);
			continue;
		}
		
		// Standard constant
		if (a_VersionNum >= DXNTBV_VER20130731)
			RetVal ^= UINT32_C(0xCAFEBABE);
		
		// XOR in buttons
		RetVal ^= TicCmd->Std.buttons;
	}
	
	/* Return calculated code */
	return RetVal;
}

/* SN_EncodeTicBuf() -- Encodes tic buffer into more compact method */
void SN_EncodeTicBuf(SN_TicBuf_t* const a_TicBuf, uint8_t** const a_OutD, uint32_t* const a_OutSz, const SN_TicBufVersion_t a_VersionNum)
{
	// Size for a single player
	// 384 + 2 + 2 + 4 + 1 + 2 + 2 + 32 + 1 + 4 + 2 + 2 + 1 + 1 + 1 + 8 = 449
	// For 32+global: 449 * 33 = 14817
#define BUFSIZE 16384
	static uint8_t* Buf;
	SN_TicBuf_t* TicBuf;
	uint8_t* p;
	uint64_t Left;
	uint16_t u16;
	uint32_t u32;
	int32_t i, j, z;
	ticcmd_t* Cmd;
	uint16_t* dsP;
	uint8_t* dbP, u8;
	
	/* Check */
	if (!a_TicBuf || !a_OutD || !a_OutSz)
		return;
	
	/* Init */
	TicBuf = a_TicBuf;
	
	// Allocate buffer and start there
	if (!Buf)
		Buf = Z_Malloc(sizeof(*Buf) * BUFSIZE, PU_STATIC, NULL);
	p = Buf;
	
	/* Write Version Number */
	WriteUInt8(&p, a_VersionNum);
	
	/* Write Data */
	// Encode gametic in multiple parts
	Left = TicBuf->GameTic;
	do
	{
		// Write lower bits
		u16 = Left & UINT16_C(0x7FFF);
		Left >>= UINT64_C(15);
		
		// More data?
		if (Left)
			u16 |= UINT16_C(0x8000);
		
		// Encode
		LittleWriteUInt16((uint16_t**)&p, u16);
	} while (Left);
	
	/* Player Counts */
	for (u32 = 0, i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
			u32 |= UINT32_C(1) << i;
	
	// Reverse mask
	u32 &= ~a_TicBuf->PIGRevMask;
	
	// Write counts
	LittleWriteUInt32((uint32_t**)&p, u32);
	
	// Do not use u32 any longer
	
	/* Encode player tics */
	for (i = 0; i < MAXPLAYERS + 1; i++)
	{
		// Not in game?
		if (!(u32 & (1 << i)) && i != MAXPLAYERS)
			continue;
		
		// Get Command for this player
		Cmd = &TicBuf->Tics[i];
		
		// Encode Type
		WriteUInt8(&p, Cmd->Ctrl.Type);
		
		// Encode Ping
		LittleWriteUInt16((uint16_t**)&p, Cmd->Ctrl.Ping);
		
		// Standard player stuff
		if (!Cmd->Ctrl.Type)
		{	
			// Set Data to encode
			u16 = 0;

#define __DIFFY(x,y) if (Cmd->Std.x) u16 |= y;
#define __WRITE(x,y) if (u16 & x) y
			
			__DIFFY(forwardmove, DDB_FORWARD);
			__DIFFY(sidemove, DDB_SIDE);
			__DIFFY(aiming, DDB_AIMING);
			__DIFFY(buttons, DDB_BUTTONS);
			__DIFFY(angleturn, DDB_ANGLE);
			__DIFFY(InventoryBits, DDB_INVENTORY);
			__DIFFY(StatFlags, DDB_STATFLAGS);
			__DIFFY(artifact, DDB_ARTIFACT);
			__DIFFY(FlySwim, DDB_FLYSWIM);
			__DIFFY(XSNewWeapon[0], DDB_WEAPON);
			
			// Save a byte by encoding the more important commands first
			Left = u16;
			u8 = u16 & UINT16_C(0x7F);
			Left >>= 7;
			if (Left)
				u8 |= UINT8_C(0x80);
			WriteUInt8(&p, u8);
			if (Left)
			{
				u8 = Left & UINT16_C(0xFF);
				WriteUInt8(&p, u8);
			}
			
			__WRITE(DDB_FORWARD, WriteInt8((int8_t**)&p, Cmd->Std.forwardmove));
			__WRITE(DDB_SIDE, WriteInt8((int8_t**)&p, Cmd->Std.sidemove));
			__WRITE(DDB_AIMING, LittleWriteUInt16((uint16_t**)&p, Cmd->Std.aiming));
			__WRITE(DDB_ANGLE, LittleWriteInt16((int16_t**)&p, Cmd->Std.angleturn));
			__WRITE(DDB_INVENTORY, WriteUInt8(&p, Cmd->Std.InventoryBits));
			__WRITE(DDB_ARTIFACT, WriteUInt8(&p, Cmd->Std.artifact));
			__WRITE(DDB_FLYSWIM, LittleWriteInt16((int16_t**)&p, Cmd->Std.FlySwim));
			
			if (u16 & DDB_WEAPON)
				WriteString((uint8_t**)&p, Cmd->Std.XSNewWeapon);
			
			// Variable encode buttons and status flags
			for (z = 0; z < 2; z++)
			{
				// Check if flag is not set, if not do not write anything
				if (!(u16 & (z ? DDB_BUTTONS : DDB_STATFLAGS)))
					continue;
				
				// Encode in variable length
				Left = (z ? Cmd->Std.buttons : Cmd->Std.StatFlags);
				do
				{
					u8 = Left & UINT32_C(0x7F);
					Left >>= 7;
					
					if (Left)
						u8 |= UINT8_C(0x80);
					
					WriteUInt8(&p, u8);
				} while (Left);
			}
			
			// Data pointers
			dsP = &Cmd->Std.DataSize;
			dbP = Cmd->Std.DataBuf;
#undef __WRITE	
#undef __DIFFY
		}
		
		// Extended data
		else
		{
			// Data pointers
			dsP = &Cmd->Ext.DataSize;
			dbP = Cmd->Ext.DataBuf;
		}
		
		// Write pointer data
		Left = *dsP;
		do
		{
			// Get value and shift down
			u16 = Left & UINT8_C(0x7F);
			Left >>= 7;
			
			// More Data?
			if (Left)
				u16 |= UINT8_C(0x80);
			
			// Write Value
			WriteUInt8(&p, u16);
		} while (Left);
		
		// Write buffer
		for (j = 0; j < *dsP; j++)
			WriteUInt8(&p, dbP[j]);
	}
	
	/* Checksum */
	LittleWriteUInt32((uint32_t**)&p, SN_TicBufSum(a_TicBuf, a_VersionNum, u32));
	
	/* Done */
	*a_OutD = Buf;
	*a_OutSz = p - Buf;
#undef BUFSIZE
}

/* SN_DecodeTicBuf() -- Decodes tic buffer */
bool_t SN_DecodeTicBuf(SN_TicBuf_t* const a_TicBuf, const uint8_t* const a_InD, const uint32_t a_InSz)
{
#define BUFSIZE 16384
	static uint8_t* Buf;
	uint8_t* p, u8;
	uint16_t u16, Mask;
	uint32_t PIG, u32;
	ticcmd_t* Cmd;
	uint16_t* dsP;
	uint8_t* dbP, VersionNum;
	int32_t i, j, z, ShiftMul;
	
	/* Check */
	if (!a_TicBuf || !a_InD || !a_InSz)
		return false;
	
	/* Init */
	if (!Buf)
		Buf = Z_Malloc(BUFSIZE, PU_STATIC, NULL);
	
	// Setup buffer
	memset(Buf, 0, BUFSIZE);
	memmove(Buf, a_InD, (a_InSz < BUFSIZE ? a_InSz : BUFSIZE));
	p = Buf;
	
	// Clear tic buffer
	memset(a_TicBuf, 0, sizeof(*a_TicBuf));
	
	/* Read Data */
	// Read Version
	VersionNum = ReadUInt8(&p);
	
	// Read Gametic
	ShiftMul = 0;
	do
	{
		u16 = LittleReadUInt16((const uint16_t**)&p);
		a_TicBuf->GameTic |= ((tic_t)(u16 & UINT16_C(0x7FFF))) << (15 * ShiftMul++);
	} while (u16 & UINT16_C(0x8000));
	
	// Player in game
	PIG = LittleReadUInt32((const uint32_t**)&p);
	
	// Set reverse mask for notices
	a_TicBuf->PIGRevMask = ~PIG;
	
	// Player Commands
	for (i = 0; i < MAXPLAYERS + 1; i++)
	{
		// Get tic command
		Cmd = &a_TicBuf->Tics[i];
		
		// not in game?
		if (!(PIG & (1 << i)) && i != MAXPLAYERS)
			continue;
		
		// Read type
		Cmd->Ctrl.Type = ReadUInt8(&p);
		
		// Read Ping
		Cmd->Ctrl.Ping = LittleReadUInt16((const uint16_t**)&p);
		
		// Standard Player
		if (!Cmd->Ctrl.Type)
		{
			// Read config mask
			u8 = ReadUInt8(&p);
			Mask = u8 & UINT8_C(0x7F);
		
			if (u8 & UINT8_C(0x80))
			{
				u8 = ReadUInt8(&p);
				Mask |= ((uint16_t)u8) << 7;
			}
			
			if (Mask & DDB_FORWARD)
				Cmd->Std.forwardmove = ReadInt8((const int8_t**)&p);
			if (Mask & DDB_SIDE)
				Cmd->Std.sidemove = ReadInt8((const int8_t**)&p);
			if (Mask & DDB_AIMING)
				Cmd->Std.aiming = LittleReadUInt16((const uint16_t**)&p);
			if (Mask & DDB_ANGLE)
				Cmd->Std.angleturn = LittleReadInt16((const int16_t**)&p);
			if (Mask & DDB_INVENTORY)
				Cmd->Std.InventoryBits = ReadUInt8(&p);
			if (Mask & DDB_ARTIFACT)
				Cmd->Std.artifact = ReadUInt8(&p);
			if (Mask & DDB_FLYSWIM)
				Cmd->Std.FlySwim = LittleReadInt16((const int16_t**)&p);
			
			if (Mask & DDB_WEAPON)
			{
				j = 0;
				do
				{
					u8 = ReadUInt8(&p);
					
					if (j < MAXTCWEAPNAME)
						Cmd->Std.XSNewWeapon[j++] = u8;
				} while (u8);
			}
			
			// Variable decode buttons and status flags
			for (z = 0; z < 2; z++)
			{
				// Check if flag is set
				if (!(Mask & (z ? DDB_BUTTONS : DDB_STATFLAGS)))
					continue;
				
				// Read variable length
				u32 = ShiftMul = 0;
				do
				{
					u8 = ReadUInt8(&p);
					u32 |= ((uint32_t)(u8 & UINT8_C(0x7F))) << (7 * ShiftMul++);
				} while (u8 & UINT8_C(0x80));
				
				// Set value
				if (z)
					Cmd->Std.buttons = u32;
				else
					Cmd->Std.StatFlags = u32;
			}
			
			// Data pointers
			dsP = &Cmd->Std.DataSize;
			dbP = Cmd->Std.DataBuf;
		}
		
		// Extended
		else
		{
			// Data pointers
			dsP = &Cmd->Ext.DataSize;
			dbP = Cmd->Ext.DataBuf;
		}
		
		// Read Size
		*dsP = ShiftMul = 0;
		do
		{
			u8 = ReadUInt8(&p);
			*dsP |= (tic_t)(u8 & UINT16_C(0x7F)) << (7 * ShiftMul++);
		} while (u8 & UINT16_C(0x80));
		
		// Read data buffer
		for (j = 0; j < *dsP; j++)
		{
			u8 = ReadUInt8(&p);
			
			if (j < MAXTCDATABUF)
				dbP[j] = u8;
		}
	}
	
	/* Confirm checksum */
	u32 = LittleReadUInt32((const uint32_t**)&p);
	return (u32 == SN_TicBufSum(a_TicBuf, VersionNum, PIG));
#undef BUFSIZE
}

