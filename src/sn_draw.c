// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Network drawing stuff

/***************
*** INCLUDES ***
***************/

#include "sn.h"
#include "dstrings.h"
#include "v_video.h"
#include "console.h"
#include "g_state.h"

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

