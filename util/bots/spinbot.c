// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: This bot spins around in circles.

/***************
*** INCLUDES ***
***************/

#include "bot_lib.h"

/****************
*** FUNCTIONS ***
****************/

/* main() -- Main entry for bot */
void main(void)
{
	volatile tic_t NowTic, LastTic;		// Timing
	
	for (NowTic = LastTic = 0;; BSleep(1))
	{
		// If bot is dead, respawn
		if (g_BotInfo.Flags & BLBIF_DEAD)
		{
			g_TicCmd.Buttons = BLT_USE;
			continue;
		}
		
		// Clear use button
		else
			g_TicCmd.Buttons &= ~BLT_USE;
		
		// Always set to max momentum (so bot runs like crazy)
		g_TicCmd.ForwardMove = MAXRUNSPEED;
		g_TicCmd.SideMove = -MAXRUNSPEED;
		
		// Obtain current time
		NowTic = g_GameInfo.GameTic;
		
		// A new tic was entered
		if (NowTic >= LastTic)
		{
			g_TicCmd.LookAngle += ANGLEX(45);
			g_TicCmd.Buttons |= BLT_JUMP;
			
			// Last last time to now (for the next second)
			LastTic = NowTic + TICRATE;
		}
	}
}

