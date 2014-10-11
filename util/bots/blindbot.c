// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: This bot is blind and will attempt level navigation to an

/***************
*** INCLUDES ***
***************/

#include "bot_lib.h"

/****************
*** FUNCTIONS ***
****************/

/* main() -- Bot main entry */
void main(void)
{
	int XDir, YDir;
	int Mode;
	fixed_t dx, dy;
	fixed_t nd, ld;
	tic_t Now, Then;
	
	/* Head in default direction */
	XDir = 0;
	YDir = 1;
	Mode = 0;
	dx = dy = nd = ld = 0;
	Now = Then = 0;
	
	/* Infinite Loop */
	// Execute once every 2 tics
	for (;; BSleep(1))
	{
		// If bot is dead, press use
		if (g_Info.Health <= 0)
		{
			g_TicCmd.Buttons |= BLT_USE;
			g_TicCmd.ForwardMove = 0;
			
			// Change time
			Mode = 0;
			Now = Then;
			continue;
		}
		
		// Let go of use
		g_TicCmd.Buttons &= ~BLT_USE;
		
		// Current time
		Now = g_Real.GameTic;
		
		// Set new direction
		if (!Mode)
		{
			// Face new angle based on direction
				//             ANG90
				//              ^
				//              |
				// ANG180 <-----*-----> ANG0 (EAST)
				//              |
				//              v
				//            ANG270
				// NORTH = +y SOUTH = -y
				// EAST  = +x WEST  = -x
				//	-1,1	0,1		1,1
				//	-1,0	0,0		1,0
				//	-1,-1	0,-1	1,-1
			if (XDir == 1)
			{
				if (YDir == 1)
					g_TicCmd.LookAngle = ANG45;
				else if (YDir == 0)
					g_TicCmd.LookAngle = 0;
				else
					g_TicCmd.LookAngle = ANG315;
			}
			
			else if (XDir == 0)
			{
				if (YDir == 1)
					g_TicCmd.LookAngle = ANG90;
				else if (YDir == 0)
					g_TicCmd.LookAngle = 0;
				else
					g_TicCmd.LookAngle = ANG270;
			}
			
			else
			{
				if (YDir == 1)
					g_TicCmd.LookAngle = ANG135;
				else if (YDir == 0)
					g_TicCmd.LookAngle = ANG180;
				else
					g_TicCmd.LookAngle = ANG225;
			}
			
			// Set destination point
			dx = g_Info.x + ((256 << FRACBITS) * XDir);
			dy = g_Info.y + ((256 << FRACBITS) * YDir);
			
			// Reset point distance
			ld = DistTo(dx, dy) >> FRACBITS;
			
			// Set time until mode recheck target
			Mode = 1;
			Then = Now + 2;
			
			// Walk to this point
			g_TicCmd.ForwardMove = MAXRUNSPEED;
		}
		
		// Move to direction
		else
		{
			// Calculate Distance always
			nd = DistTo(dx, dy) >> FRACBITS;
			
			// Check to see if we are getting closer to our destination
			if (Now >= Then)
			{
				// If we are not getting any closer... change direction
				if (nd >= ld)
				{
					// Loop prevents 0,0 target
					do
					{
						XDir = RandomLTZGT();
						YDir = RandomLTZGT();
					}
					while (XDir == 0 && YDir == 0);
					
					// Recalculate target
					Mode = 0;
					g_TicCmd.ForwardMove = 0;	// stop moving
				}
				
				// Remember this distance (it will get smaller)
				else
				{
					// Set destination point
					dx = g_Info.x + ((256 << FRACBITS) * XDir);
					dy = g_Info.y + ((256 << FRACBITS) * YDir);
					
					// Calculate new distance
					ld = DistTo(dx, dy) >> FRACBITS;
				}
			}
		}
	}
}

