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
// Copyright (C) 1993-1996 by id Software, Inc.
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
//      DOOM selection menu, options, episode etc.
//      Sliders and icons. Kinda widget stuff.
//
// NOTE:
//      All V_DrawPatchDirect () has been replaced by V_DrawScaledPatch ()
//      so that the menu is scaled to the screen size. The scaling is always
//      an integer multiple of the original size, so that the graphics look
//      good.

#include "am_map.h"

#include "doomdef.h"
#include "dstrings.h"
#include "d_main.h"

#include "console.h"

#include "r_local.h"
#include "hu_stuff.h"
#include "g_game.h"
#include "g_input.h"

#include "m_argv.h"

// Data.
#include "sounds.h"
#include "s_sound.h"
#include "i_system.h"
#include "i_sound.h"

#include "m_menu.h"
#include "v_video.h"
#include "i_video.h"

#include "keys.h"
#include "z_zone.h"
#include "w_wad.h"
#include "p_local.h"
#include "p_fab.h"
#include "t_script.h"

#include "d_net.h"
#include "p_inter.h"
#include "d_prof.h"
#include "v_widget.h"

#include "i_util.h"

/*******************************************************************************
********************************************************************************
*******************************************************************************/

/*** CONSTANTS ***/

#define MAXUIBUTTONS 32

/*** STRUCTURES ***/

/* M_UILocalBox_t -- Local message box */
typedef struct M_UILocalBox_s
{
	M_ExMBType_t Type;							// Message Box Type
	uint32_t MessageID;							// ID of message
	MBCallBackFunc_t CallBack;					// Callback when message pressed
	const char* Title;							// Title
	const char* Message;						// Message
	uint32_t SelButton;							// Selected Button
	uint32_t NumButtons;						// Number of buttons
	int32_t x, y, w, h;							// Box Size
	int32_t tX, tY, mX, mY;						// Message Positions
	uint32_t AppearTime;						// Time box appeared
	
	struct
	{
		int32_t x, y, w, h;						// Position and size
		const char* Text;						// Text on button
		M_ExMBType_t Response;					// Button response
	} Buttons[MAXUIBUTTONS];
} M_UILocalBox_t;

#define MUIBOXFONT VFONT_SMALL

/*** LOCALS ***/

static M_UILocalBox_t** l_UIBoxes = NULL;
static size_t l_NumUIBoxes = 0;

/*** FUNCTIONS ***/

/* M_ExUIActive() -- Returns true if UI is active */
bool_t M_ExUIActive(void)
{
	if (l_NumUIBoxes)
		return true;
	return false;
}

/* M_ExUIMessageBox() -- Creates a new message box */
bool_t M_ExUIMessageBox(const M_ExMBType_t a_Type, const uint32_t a_MessageID, const char* const a_Title, const char* const a_Message, const MBCallBackFunc_t a_CallBack)
{
#define UISPACING 12
#define UIMIDSPACE (UISPACING >> 1)
	M_UILocalBox_t* New;
	int32_t tW, tH, mW, mH, bW, bX, bY;
	size_t i;
	
	/* Create at end of stack */
	Z_ResizeArray((void**)&l_UIBoxes, sizeof(*l_UIBoxes), l_NumUIBoxes, l_NumUIBoxes + 1);
	New = l_UIBoxes[l_NumUIBoxes++] = Z_Malloc(sizeof(M_UILocalBox_t), PU_STATIC, NULL);
	
	/* Fill with info */
	New->MessageID = a_MessageID;
	New->Title = a_Title;
	New->Message = a_Message;
	New->CallBack = a_CallBack;
	New->AppearTime = I_GetTimeMS();
	
	/* Create Buttons */
	if (a_Type & MEXMBT_DONTCARE)
	{
		// Get last button
		i = New->NumButtons++;
		New->Buttons[i].Response = MEXMBT_DONTCARE;
		New->Buttons[i].Text = "Don't Care";
		New->Buttons[i].w = V_StringWidthA(MUIBOXFONT, 0, New->Buttons[i].Text);
		New->Buttons[i].h = V_StringHeightA(MUIBOXFONT, 0, New->Buttons[i].Text);
	}
	
	/* Size everything down */
	// Get size of strings
	tW = V_StringWidthA(MUIBOXFONT, 0, New->Title);
	tH = V_StringHeightA(MUIBOXFONT, 0, New->Title);
	mW = V_StringWidthA(MUIBOXFONT, 0, New->Message);
	mH = V_StringHeightA(MUIBOXFONT, 0, New->Message);
	
	// Determine box size and position
	New->w = (mW > tW ? mW : tW) + UISPACING;
	New->h = mH + tH + (UIMIDSPACE * 3) + ((New->NumButtons) ? New->Buttons[0].h + UIMIDSPACE: 0);
	New->x = 160 - (New->w >> 1);
	New->y = 100 - (New->h >> 1);
	
	// Determine message locations
	New->tX = 160 - (tW >> 1);
	New->tY = New->y + UIMIDSPACE;
	New->mX = 160 - (mW >> 1);
	New->mY = New->tY + tH + UIMIDSPACE;
	
	if (New->NumButtons)
	{
		bY = New->mY + mH + UIMIDSPACE;
	}
	
	/* Go through and order buttons */
	// Determine button width
	bW = 0;
	for (i = 0; i < New->NumButtons; i++)
		bW += New->Buttons[i].w + UIMIDSPACE;
	bX = 160 - (bW >> 1);
	
	// Select positions
	for (i = 0; i < New->NumButtons; i++)
	{
		New->Buttons[i].x = bX;
		bX += New->Buttons[i].w + UIMIDSPACE;
		New->Buttons[i].y = bY;
	}
	
	/* Success */
	return true;
#undef UIMIDSPACE
#undef UISPACING
}

/* M_ExUIHandleEvent() -- Handle event */
bool_t M_ExUIHandleEvent(const I_EventEx_t* const a_Event)
{
	bool_t DidSomething, DidAClick;
	M_UILocalBox_t* Box;
	int32_t b, OldButton;
	int32_t x, y, BoxStackPos;
	uint32_t ThisTime;
	
	/* Check */
	if (!a_Event)
		return false;
		
	/* A message Box is on the screen? */
	if (l_NumUIBoxes)
	{
		DidSomething = DidAClick = false;
		
		// Get box at top of stack
		BoxStackPos = l_NumUIBoxes - 1;
		Box = l_UIBoxes[BoxStackPos];
		
		// No box?
		if (!Box)
			return false;
		
		// Keyboard Event
		if (a_Event->Type == IET_KEYBOARD)
		{
			// Enter is hit
			if (a_Event->Data.Keyboard.KeyCode == IKBK_RETURN)
			{
				// Get current time
				ThisTime = I_GetTimeMS();
				
				// Don't press enter too soon
				if (ThisTime > Box->AppearTime + 1500)
				{
					// Make noise
					S_StartSound(NULL, sfx_osktyp);
							
					// End
					DidSomething = true;
					DidAClick = true;
				}
			}
			
			// Moving cursor
			else if (a_Event->Data.Keyboard.KeyCode == IKBK_LEFT ||
				a_Event->Data.Keyboard.KeyCode == IKBK_RIGHT)
			{
				// Remember old button
				OldButton = Box->SelButton;
				
				// Move left or right?
				if (a_Event->Data.Keyboard.KeyCode == IKBK_LEFT)
					b = OldButton - 1;
				else
					b = OldButton + 1;
				
				// Cap
				if (b < 0)
					b = Box->NumButtons - 1;
				else if (b >= Box->NumButtons)
					b = 0;
				
				// Button changed?
				if (b != OldButton)
				{
					// Move to that button
					Box->SelButton = b;
					
					// Make noise
					S_StartSound(NULL, sfx_oskmov);
					
					// End
					DidSomething = true;
				}
			}
		}
		
		// Mouse Event
		else if (a_Event->Type == IET_MOUSE)
		{
			// Get mouse pos
			x = g_MousePos[0];
			y = g_MousePos[1];
			
			// Move to button under the cursor
			for (b = 0; b < Box->NumButtons; b++)
				if (x >= Box->Buttons[b].x && x <= Box->Buttons[b].x + Box->Buttons[b].w)
					if (y >= Box->Buttons[b].y && y <= Box->Buttons[b].y + Box->Buttons[b].h)
					{
						// Button isn't selected
						if (b != Box->SelButton)
						{
							// Move to that button
							Box->SelButton = b;
							
							// Make noise
							S_StartSound(NULL, sfx_oskmov);
							
							// End
							DidSomething = true;
							break;
						}
						
						// Mouse is down over THIS button
						else if (a_Event->Data.Mouse.Down && b == Box->SelButton)
						{
							// Make noise
							S_StartSound(NULL, sfx_osktyp);
							
							// End
							DidSomething = true;
							DidAClick = true;
							break;
						}
					}
		}
		
		// Did something?
		if (DidSomething)
		{
			// Chose this button?
			if (DidAClick)
			{
				// Call the callback
				if (Box->CallBack)
					Box->CallBack(
							Box->MessageID,
							Box->Buttons[Box->SelButton].Response,
							&Box->Title,
							&Box->Message
						);
				
				// Destroy the box
				Z_Free(Box);
				
				// Remove from the top of stack
				l_UIBoxes[BoxStackPos] = NULL;
				
				// Resize down
				Z_ResizeArray((void**)&l_UIBoxes, sizeof(*l_UIBoxes), l_NumUIBoxes, l_NumUIBoxes - 1);
				l_NumUIBoxes--;
			}
			
			return true;
		}
	}
	
	/* Un-Handled */
	return false;
}

/* M_ExUIDrawer() -- UI Drawer */
void M_ExUIDrawer(void)
{
	M_UILocalBox_t* Box;
	size_t i;
	uint32_t a_Flags;
	bool_t DisableDraw;
	
	/* Draw Messages? */
	if (l_NumUIBoxes)
	{
		// Get box at top of stack
		Box = l_UIBoxes[l_NumUIBoxes - 1];
		
		// Box exists?
		if (Box)
		{
			// Message is appeared but cannot actually click it
			DisableDraw = false;
			if (CONL_IsActive())
				DisableDraw = true;
			
			// Draw Faded Message Background
			V_DrawFadeConsBackEx(
					VEX_COLORMAP((DisableDraw ? VEX_MAP_BLACK : VEX_MAP_GRAY)),
					Box->x, Box->y, Box->x + Box->w, Box->y + Box->h
				);
			
			// Draw Title/Message Pair
			V_DrawStringA(
					MUIBOXFONT, VFO_COLOR((DisableDraw ? VEX_MAP_WHITE : VEX_MAP_GREEN)), Box->Title,
					Box->tX, Box->tY
				);
			V_DrawStringA(
					MUIBOXFONT, VFO_COLOR(VEX_MAP_BRIGHTWHITE), Box->Message,
					Box->mX, Box->mY
				);
			
			// Draw Buttons
			for (i = 0; i < Box->NumButtons; i++)
			{
				if (i == Box->SelButton)
				{
					if (DisableDraw)
					{
						if (gametic & 0x8)
							a_Flags = VFO_COLOR(VEX_MAP_BLACK);
						else
							a_Flags = VFO_COLOR(VEX_MAP_WHITE);
					}
					else
					{
						if (gametic & 0x8)
							a_Flags = VFO_COLOR(VEX_MAP_RED);
						else
							a_Flags = VFO_COLOR(VEX_MAP_YELLOW);
					}
				}
				else
					a_Flags = VFO_COLOR((DisableDraw ? VEX_MAP_BLACK : VEX_MAP_GRAY));
				V_DrawStringA(
						MUIBOXFONT, a_Flags, Box->Buttons[i].Text,
						Box->Buttons[i].x, Box->Buttons[i].y
					);
			}
		}
		
		// Draw Mouse
		CONL_DrawMouse();
	}
}


