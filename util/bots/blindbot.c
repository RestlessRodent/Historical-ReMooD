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
// Copyright (C) 2013-2013 ReMooD       <http://remood.org/>
// Copyright (C) 2013-2013 GhostlyDeath <ghostlydeath@remood.org>
//                                      <ghostlydeath@gmail.com>
// ----------------------------------------------------------------------------
// THERE ARE TWO LICENSES AVAILABLE FOR THIS FILE ONLY WITH ADDITIONAL TERMS:
//
// * THIS HEADER MUST NOT BE REMOVED, REGARDLESS OF WHICH LICENSE YOU CHOOSE.
// * KEEPING THESE LICENSE DESCRIPTIONS IN THE HEADER WILL NOT AFFECT NOR FORCE
// * THE CHOICE OF THE LICENSE AS LONG AS IT IS ONE OF THE FOLLOWING LISTED.
//
//  GNU GENERAL PUBLIC LICENSE 3 OR LATER
//  * This program is free software; you can redistribute it and/or
//  * modify it under the terms of the GNU General Public License
//  * as published by the Free Software Foundation; either version 3
//  * of the License, or (at your option) any later version.
//  * 
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
// 
//  SIMPLIFIED BSD LICENSE:
//  * Redistribution and use in source and binary forms, with or without
//  * modification, are permitted provided that the following conditions are
//  * met: 
//  * 
//  * 1. Redistributions of source code must retain the above copyright notice,
//  *    this list of conditions and the following disclaimer. 
//  * 2. Redistributions in binary form must reproduce the above copyright
//  *    notice, this list of conditions and the following disclaimer in the
//  *    documentation and/or other materials provided with the distribution. 
//  * 
//  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
//  * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
//  * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
//  * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  * 
//  * The views and conclusions contained in the software and documentation are
//  * those of the authors and should not be interpreted as representing
//  * official policies, either expressed or implied, of the ReMooD Project.
// ----------------------------------------------------------------------------
// DESCRIPTION: This bot is blind and will attempt level navigation to an
// extent by moving in a direction. If moving in direction is proving pointless
// then it will change direction.

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
	
	/* Infinite Loop */
	// Execute once every 2 tics
	for (Then = 0, Mode = 0;; BSleep(1))
	{
		// If bot is dead, press use
		if (g_Info.Health <= 0)
		{
			GeneralChat("Respawning");
			
			g_TicCmd.Buttons |= BLT_USE;
			g_TicCmd.ForwardMove = 0;
			
			// Change time
			Now = Then;
			continue;
		}
		
		GeneralChat("Enter loop");
		
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
				{
					g_TicCmd.LookAngle = ANG45;
					GeneralChat("Heading North East");
				}
				else if (YDir == 0)
				{
					g_TicCmd.LookAngle = 0;
					GeneralChat("Heading East");
				}
				else
				{
					g_TicCmd.LookAngle = ANG315;
					GeneralChat("Heading South East");
				}
			}
			
			else if (XDir == 0)
			{
				if (YDir == 1)
				{
					g_TicCmd.LookAngle = ANG90;
					GeneralChat("Heading North");
				}
				else if (YDir == 0)
				{
					g_TicCmd.LookAngle = 0;
					GeneralChat("Heading Nowhere");
				}
				else
				{
					g_TicCmd.LookAngle = ANG270;
					GeneralChat("Heading South");
				}
			}
			
			else
			{
				if (YDir == 1)
				{
					g_TicCmd.LookAngle = ANG135;
					GeneralChat("Heading North West");
				}
				else if (YDir == 0)
				{
					g_TicCmd.LookAngle = ANG180;
					GeneralChat("Heading West");
				}
				else
				{
					g_TicCmd.LookAngle = ANG225;
					GeneralChat("Heading South West");
				}
			}
			
			// Set destination point
			dx = g_Info.x + ((1024 << FRACBITS) * XDir);
			dy = g_Info.y + ((1024 << FRACBITS) * YDir);
			
			// Reset point distance
			ld = DistTo(dx, dy) >> FRACBITS;
			
			// Set time until mode recheck target
			Mode = 1;
			Then = Now + TICRATE;
		}
		
		// Move to direction
		else
		{
			// Walk to this point
			g_TicCmd.ForwardMove = MAXRUNSPEED;
			
			// Caluclate Distance always
			nd = DistTo(dx, dy) >> FRACBITS;
			
			// Check to see if we are getting closer to our destination
			if (Now >= Then)
			{
				// If we are not getting any closer... change direction
				if (nd >= ld)
				{
					GeneralChat("Moving away!?!?");
					
					// Loop prevents 0,0 target
					do
					{
						YDir++;
					
						if (YDir >= 2)
						{
							YDir = -1;
							XDir++;
						
							if (XDir >= 2)
								XDir = -1;
						}
					}
					while (XDir == 0 && YDir == 0);
					
					// Recalculate target
					Mode = 0;
					g_TicCmd.ForwardMove = 0;	// stop moving
				}
				
				// Remember this distance (it will get smaller)
				else
				{
					GeneralChat("In range");
					
					// Set destination point
					dx = g_Info.x + ((1024 << FRACBITS) * XDir);
					dy = g_Info.y + ((1024 << FRACBITS) * YDir);
					
					// Calculate new distance
					ld = DistTo(dx, dy) >> FRACBITS;
				}
			}
		}
	}
}

