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
#include "v_video.h"
#include "sn.h"

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
}

