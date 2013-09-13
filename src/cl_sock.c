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
// Copyright (C) 2013-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Game Sockets

/***************
*** INCLUDES ***
***************/

#include "cl.h"
#include "i_util.h"
#include "z_zone.h"
#include "sn.h"
#include "g_state.h"

// Interface Friendly Stuff
#include "v_video.h"
#include "s_sound.h"
#include "console.h"
#include "m_menu.h"

/*****************
*** STRUCTURES ***
*****************/

/**************
*** GLOBALS ***
**************/

CL_Socket_t** g_CLSocks = NULL;					// Sockets
int32_t g_NumCLSocks = 0;						// Number of sockets

/****************
*** FUNCTIONS ***
****************/

/* CL_InitSocks() -- Initialize sockets */
int32_t CL_InitSocks(void)
{
	int32_t i;
	CL_Socket_t* This;
	
	/* Allocate sturctures */
	g_NumCLSocks = 1 + I_NumJoysticks();
	g_CLSocks = Z_Malloc(sizeof(*g_CLSocks) * g_NumCLSocks, PU_STATIC, NULL);
	
	/* Add Keyboard/Mouse Socket */
	This = g_CLSocks[0] = Z_Malloc(sizeof(*This), PU_STATIC, NULL);
	
	// Initialize
	This->JoyID = -1;
	
	/* Add sockets for every joystick */
	for (i = 0; i < I_NumJoysticks(); i++)
	{
		// Allocate
		This = g_CLSocks[1 + i] = Z_Malloc(sizeof(*This), PU_STATIC, NULL);
		
		// Initialize
		This->Flags |= CLSF_JOYSTICK;
		This->JoyID = i;
	}
	
	/* Return number of control sockets */
	return g_NumCLSocks;
}

/* CL_BindSocket() -- Binds socket to viewport */
CL_View_t* CL_BindSocket(CL_Socket_t* const a_Sock, const int8_t a_JoyID)
{
	int32_t i;
	CL_View_t* View;
	
	/* Check */
	if (!a_Sock)
		return NULL;
	
	/* If socket already owns a view, do not double bind */
	if (a_Sock->View)
		return a_Sock->View;
	
	/* Find a viewport that is not taken */
	for (i = 0; i < MAXSPLITS; i++)
	{
		View = &g_CLViews[i];
		
		// Already taken?
		if (View->Socket)
			continue;
		
		// Take over this view
		View->Socket = a_Sock;
		a_Sock->View = View;
		
		// No Joystick
		if (a_JoyID < 0)
			a_Sock->JoyID = -1;
		
		// Using joystick
		else
		{
			a_Sock->JoyID = a_JoyID;
			a_Sock->Flags |= CLSF_JOYSTICK;
		}
		
		// Increase bind count
		g_CLBinds++;
		
		// Play bind sound (a nice nice tone)
		S_StartSound(NULL, sfx_dialup);
		
		// Claimed
		return View;
	}
	
	/* No view found */
	return NULL;
}

/* CL_SockEvent() -- Handle events on sockets */
bool_t CL_SockEvent(const I_EventEx_t* const a_Event)
{
	CL_Socket_t* Sock;
	CL_View_t* View;
	int32_t i;
	bool_t DoBind;
	
	/* Check */
	if (!a_Event)
		return false;
	
	/* Determine who gets the event, for said socket */
	Sock = NULL;
	
	// Synthetic OSK
	if (a_Event->Type == IET_SYNTHOSK)
		Sock = g_CLSocks[a_Event->Data.SynthOSK.SNum];
	
	// Keyboard/Mouse == Always first
	else if (a_Event->Type == IET_KEYBOARD || a_Event->Type == IET_MOUSE)
		Sock = g_CLSocks[0];
	
	// Joystick
	else if (a_Event->Type == IET_JOYSTICK)
	{
		for (i = 1; i < g_NumCLSocks; i++, Sock = NULL)
			if ((Sock = g_CLSocks[i]))
			{
				// No joystick here
				if (!(Sock->Flags & CLSF_JOYSTICK))
					continue;
				
				// Wrong joy
				if (Sock->JoyID != a_Event->Data.Joystick.JoyID)
					continue;
				
				// This is it
				break;
			}
	}
	
	// Some other event
	else
		return false;
	
	/* No socket gets this */
	if (!Sock)
		return false;
	
	/* If socket is not bound, do bind code */
	if (!Sock->View)
	{
		DoBind = false;
		i = -1;
		
		// Use space on the keyboard
		if (a_Event->Type == IET_KEYBOARD)
		{
			if (a_Event->Data.Keyboard.KeyCode == IKBK_SPACE &&
					a_Event->Data.Keyboard.Down)
				DoBind = true;
		}
		
		// First mouse button takes control
		else if (a_Event->Type == IET_MOUSE)
		{
			if (a_Event->Data.Mouse.Button == 1 &&
					a_Event->Data.Mouse.Down)
				DoBind = true;
		}
		
		// Otherwise, use the first joystick button
		else if (a_Event->Type == IET_JOYSTICK)
		{
			if (a_Event->Data.Joystick.Button == 1 &&
					a_Event->Data.Joystick.Down)
				DoBind = true;
		}
		
		// If binding, do the bind
		if (DoBind)
			if ((View = CL_BindSocket(Sock, i)))
				return true;
	}
	
	/* Otherwise it is bound for some reason */
	else
	{
		// Always generate synthosk commands with joysticks
		if (a_Event->Type == IET_JOYSTICK)
		{
		}
		
		// Send control to port
		if (SN_HandleEvent(a_Event, Sock))
			return true;
	}
	
	/* Not Handled */
	return false;
}

/* CL_SockDrawer() -- Draws socket interface */
void CL_SockDrawer(void)
{
#define BUFSIZE 9
	int32_t i, j, h, x, y, la, wa, Lots;
	CL_Socket_t* Sock;
	char Color, *Text;
	char Buf[BUFSIZE];
	bool_t Flash, Ready;
	
	/* Only draw if a menu or console is active */
	if (UI_Visible())
		return;
	
	/* Height of font */
	h = V_FontHeight(VFONT_SMALL);
	
	/* Flash? */
	Flash = false;
	if ((g_ProgramTic >> 4) & 1)
		Flash = true;
	
	/* Go through all sockets */
	for (i = 0; i < g_NumCLSocks; i++)
	{
		// Get socket here
		if (!(Sock = g_CLSocks[i]))
			continue;
		
		// If a viewport is set, then it is active
		if (Sock->View)
		{
			// Ready!
			Ready = true;
			Text = "{1READY!";
		}
		
		// Otherwise, press some buttons
		else
		{
			// All players inside, who cares about those who cannot join?
			if (g_CLBinds >= MAXSPLITS)
				continue;
			
			// Not ready
			Ready = false;
			
			// Flash message every half second
			if (Flash)
				Text = "";
			else
			{
				if (Sock->Flags & CLSF_JOYSTICK)
					Text = "{5PRESS 1ST BUTTON!";
				else
					Text = "{5PRESS SPACE!";
			}
		}
		
		// Determine their color
		Color = i & 15;
		
		if (Color >= 10)
			Color = (Color - 10) + 'a';
		else
			Color += '0';
		
		// Setup string to display (just P whatever)
		Buf[0] = '{';
		Buf[1] = 'x';
		Buf[2] = '7';
		Buf[3] = Color;
		Buf[4] = 'P';
		
		if (i < 9)
		{
			Buf[5] = '1' + i;
			Buf[6] = 0;
		}
		else
		{
			Buf[5] = '1' + ((i - 9) / 10);
			Buf[6] = '0' + ((i - 9) % 10);
			Buf[7] = 0;
		}
		
		// Determine lots count
		Lots = j / 4;	// so many joysticks!
		
		// Same size
		la = V_StringWidthA(VFONT_SMALL, 0, Buf);
		
		// Determine draw location
		switch (j & 3)
		{
			case 0:
				x = 2;
				y = 2 + ((h + 2) * Lots);
				
				if (Ready)
				{
					wa = la + 2;
					la = 0;
				}
				else
					wa = la = 0;
				break;
				
			case 1:
				x = 318;
				y = 2 + ((h + 2) * Lots);
				la = -la;
				wa = -V_StringWidthA(VFONT_SMALL, 0, Text);
				
				if (Ready)
					wa -= 2;
				break;
				
			case 2:
				x = 2;
				y = (198 - ((h + 2) * Lots)) - h;
				
				if (Ready)
				{
					wa = la + 2;
					la = 0;
				}
				else
					wa = la = 0;
				break;
				
			default:
				x = 318;
				y = (198 - ((h + 2) * Lots)) - h;
				la = -la;
				wa = -V_StringWidthA(VFONT_SMALL, 0, Text);
				
				if (Ready)
					wa -= 2;
				break;
		}
		
		// Draw ready next to player
		if (Ready)
		{
			V_DrawStringA(VFONT_SMALL, 0, Buf, x + la, y);
			V_DrawStringA(VFONT_SMALL, 0, Text, x + la + wa, y);
		}
		
		// Flash between
		else
		{
			if (Flash)
				V_DrawStringA(VFONT_SMALL, 0, Buf, x + la, y);
			else
				V_DrawStringA(VFONT_SMALL, 0, Text, x + wa, y);
		}
		
		// Was drawn, increase j
		j++;
	}
#undef BUFSIZE
}

/* CL_InitLevelSocks() -- Ininitialize Sockets (and spectators) for a new level */
void CL_InitLevelSocks(void)
{
	// TODO FIXME: Implement
}

/* CL_ClearLevelSocks() -- Clears Sockets (and spectators) for a cleared level */
void CL_ClearLevelSocks(void)
{
	// TODO FIXME: Implement
}

/* CL_DoResetMapZoom() -- Resets zoom for all sockets */
void CL_DoResetMapZoom(void)
{
	// TODO FIXME: Clear Zooms
}

/* CL_DoAngleSync() -- Syncrhonize all player angles with their true angles */
void CL_DoAngleSync(void)
{
	// TODO FIXME: Implement
}

/* CL_DoSetYawP() -- Sets yaw for player */
void CL_DoSetYawP(player_t* const a_Player, const angle_t a_Yaw)
{
	/* Check */
	if (!a_Player)
		return;
	
	// TODO FIXME: Set localangle
}

/* CL_DoSetAnglesP() -- Set local angles for player */
void CL_DoSetAnglesP(player_t* const a_Player, const angle_t a_Yaw, const angle_t a_Pitch)
{
	/* Check */
	if (!a_Player)
		return;
	
	// TODO FIXME: Set Angles
}

/* CL_DoDeathViewP() -- Initializes Death View */
void CL_DoDeathViewP(player_t* const a_Player)
{
	/* Check */
	if (!a_Player)
		return;
	
	// TODO FIXME: Set aiming angle to zero
}

/* CL_DoTactileP() -- Force feedback */
void CL_DoTactileP(player_t* const a_Player, const int32_t a_On, const int32_t a_Off, const int32_t a_Total)
{
	/* Check */
	if (!a_Player)
		return;
	
	// TODO FIXME: Electrocute real player
}

void CL_SpecTicker(void) {}// Move to cl_spec.c

