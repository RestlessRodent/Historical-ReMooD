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
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
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
// DESCRIPTION: Network drawing stuff

/***************
*** INCLUDES ***
***************/

#include "sn.h"
#include "dstrings.h"
#include "v_video.h"
#include "console.h"

/****************
*** CONSTANTS ***
****************/

/*****************
*** STRUCTURES ***
*****************/

/*************
*** LOCALS ***
*************/

static tic_t l_PDWarnTic;						// partial disconnect warn

/*****************
*** PROTOTYPES ***
*****************/

/* SN_DrawLobby() -- Draws the lobby */
void SN_DrawLobby(void)
{
	static V_Image_t* BGImage;
	
	/* Draw a nice picture */
	// Load it first
	if (!BGImage)
		BGImage = V_ImageFindA("RMD_LLOA", VCP_DOOM);
	
	// Draw it
	V_ImageDraw(0, BGImage, 0, 0, NULL);
	
	/* Draw Text */
	// Notice
	V_DrawStringA(VFONT_LARGE, 0, DS_GetString(DSTR_WFGS_TITLE), 10, 10);
	
	/* Draw Mouse */
	CONL_DrawMouse();
}

/* SN_SetServerLagWarn() -- Server is lagging, set warning time */
void SN_SetServerLagWarn(const tic_t a_EstPD)
{
	l_PDWarnTic = a_EstPD;
}

/* SN_Drawer() -- Networking drawer */
void SN_Drawer(void)
{
#define BUFSIZE 32
	char Buf[BUFSIZE];
	tic_t Left;
	int32_t Mins, Secs;
	
	/* Do not draw if not connected */
	if (!SN_IsConnected())
		return;
	
	/* Partial disconnect at this tic */
	if (l_PDWarnTic)
	{
		// Calculate time left
		Left = l_PDWarnTic - g_ProgramTic;
		
		// Overflowed, not yet set to zero
		if (Left > l_PDWarnTic)
			Left = 0;
		
		// Draw a giant message
		V_DrawStringA(VFONT_LARGE, 0, DS_GetString(DSTR_DNETDRAWC_PDWARN), 10, 10);
		
		// Calculate Time
		Secs = Left / TICRATE;
		Mins = Secs / 60;
		Secs = Secs % 60;
		
		// Draw time in numbers
		if (Mins)
			snprintf(Buf, BUFSIZE - 1, "%i:%02i", Mins, Secs);
		else
			snprintf(Buf, BUFSIZE - 1, "%i seconds", Secs);
		Buf[BUFSIZE - 1] = 0;
		
		// Draw it
		V_DrawStringA(VFONT_SMALL, 0, Buf, 10, 12 + V_FontHeight(VFONT_LARGE));
	}
#undef BUFSIZE
}

