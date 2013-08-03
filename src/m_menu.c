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
// Copyright (C) 2012-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Menu Code

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

#include "z_zone.h"
#include "w_wad.h"
#include "p_local.h"
#include "t_script.h"

#include "d_net.h"
#include "p_inter.h"
#include "d_prof.h"

#include "i_util.h"
#include "p_demcmp.h"

#include "r_data.h"
#include "vhw_wrap.h"

#include "g_game.h"

#include "m_menupv.h"

/*******************
*** SIMPLE MENUS ***
*******************/

/*** LOCALS ***/

static M_SWidget_t** l_SMWi[MAXSPLITSCREEN];
static size_t l_NumSMWi[MAXSPLITSCREEN];

/*** HELPERS ***/

/* MS_SMDestroy() -- Destroys Widget */
static void MS_SMDestroy(M_SWidget_t* const a_Widget)
{
	int32_t i;
	
	/* Check */
	if (!a_Widget)
		return;
	
	/* Kill all kids */
	for (i = 0; i < a_Widget->NumKids; i++)
		if (a_Widget->Kids[i])
			MS_SMDestroy(a_Widget->Kids[i]);
	
	/* Call destruction function */
	if (a_Widget->FDestroy)
		a_Widget->FDestroy(a_Widget);
		
	/* Free self */
	Z_Free(a_Widget);
}

/* MS_SMStackPush() -- Pushes menu onto player stack */
static void MS_SMStackPush(const int32_t a_ScreenID, M_SWidget_t* const a_Widget)
{
	/* Check */
	if (a_ScreenID < 0 || a_ScreenID >= MAXSPLITSCREEN || !a_Widget)
		return;
	
	/* Resize array */
	Z_ResizeArray((void**)&l_SMWi[a_ScreenID], sizeof(*l_SMWi[a_ScreenID]),
		l_NumSMWi[a_ScreenID], l_NumSMWi[a_ScreenID] + 1);
	l_SMWi[a_ScreenID][l_NumSMWi[a_ScreenID]++] = a_Widget;
	
	/* Play a nice sound */
	// First menu
	if (l_NumSMWi[a_ScreenID] == 1)
		S_StartSound(NULL, sfx_generic_switchon);
	
	// Any other menu
	else
		S_StartSound(NULL, sfx_generic_menupress);
}

/* M_StackPop() -- Pops Widget */
void M_StackPop(const int32_t a_ScreenID)
{
	M_SWidget_t* Popped;
	
	/* Check */
	if (a_ScreenID < 0 || a_ScreenID >= MAXSPLITSCREEN)
		return;
	
	/* No items? */
	if (!l_NumSMWi[a_ScreenID])
		return;
	
	/* Destroy last pushed widget */
	Popped = l_SMWi[a_ScreenID][--l_NumSMWi[a_ScreenID]];
	
	// Kill it
	MS_SMDestroy(Popped);
	
	// Play sound
	S_StartSound(NULL, sfx_generic_switchoff);
}

/* M_StackPopAllScreen() -- Pops all from stack for screen */
void M_StackPopAllScreen(const int32_t a_Screen)
{
	/* Check */
	if (a_Screen < 0 || a_Screen >= MAXSPLITSCREEN)
		return;
	
	/* Pop */
	while (l_NumSMWi[a_Screen])
		M_StackPop(a_Screen);
}

/* M_StackPopAll() -- Pops all from stack */
void M_StackPopAll(void)
{
	int32_t i;
	
	/* Pop */
	for (i = 0; i < MAXSPLITSCREEN; i++)
		while (l_NumSMWi[i])
			M_StackPop(i);
}

/* MS_SMCreateBase() -- Creates base widget */
static M_SWidget_t* MS_SMCreateBase(M_SWidget_t* const a_Parent)
{
	M_SWidget_t* New;
	
	/* Setup new widget */
	New = Z_Malloc(sizeof(*New), PU_SIMPLEMENU, NULL);
	
	// Parent
	New->Parent = a_Parent;
	
	// Copy Screen Too
	if (New->Parent)
		New->Screen = New->Parent->Screen;
	
	// Add to parents kids
	if (New->Parent)
	{
		Z_ResizeArray((void**)&New->Parent->Kids, sizeof(*New->Parent->Kids),
			New->Parent->NumKids, New->Parent->NumKids + 1);
		New->Parent->Kids[New->Parent->NumKids++] = New;
	}
	
	/* Return it */
	return New;
}

/* MS_SMCreateBox() -- Creates simple box */
static M_SWidget_t* MS_SMCreateBox(M_SWidget_t* const a_Parent, const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H)
{
	M_SWidget_t* New;
	
	/* First create base */
	New = MS_SMCreateBase(a_Parent);
	
	// Set box shape
	New->x = a_X;
	New->y = a_Y;
	New->w = a_W;
	New->h = a_H;
	
	/* Return it */
	return New;
}

/* MS_SMTBFDDraw() -- Draw Text Box */
static void MS_SMTBFDDraw(M_SWidget_t* const a_Widget)
{
	uint32_t Flags;
	
	/* Base flags */
	Flags = a_Widget->Data.TextBox.Flags;
	a_Widget->Data.TextBox.Inputter->Font = a_Widget->Data.TextBox.Font;
	
	/* Change cursor draw status */
	a_Widget->Data.TextBox.Inputter->DontDrawCursor = !a_Widget->Data.TextBox.StealInput;
	
	/* Draw Inputter */
	CONCTI_DrawInput(a_Widget->Data.TextBox.Inputter, Flags, a_Widget->dx, a_Widget->dy, a_Widget->Parent->dx + a_Widget->Parent->dw);
	
	/* Draw OSK? */
	if (a_Widget->Data.TextBox.StealInput)
	{
		CONL_OSKSetVisible(a_Widget->Screen, true);
		CONL_OSKDrawForPlayer(a_Widget->Screen);
		CONL_OSKSetVisible(a_Widget->Screen, false);
	}
}

/* MS_SMTBFEvent() -- Text Box Event */
static bool_t MS_SMTBFEvent(M_SWidget_t* const a_Widget, const I_EventEx_t* const a_Event)
{
	bool_t RetVal;
	
	/* If not stealing input, ignore */
	if (!a_Widget->Data.TextBox.StealInput)
		return false;
	
	/* Unsteal Focus */
	// Keyboard
	if (a_Event->Type == IET_KEYBOARD)
	{
		// Ignore keyboard events on other screens
		if (a_Widget->Screen != 0)
			return false;
		
		// Only check if escape is down
		if (a_Event->Data.Keyboard.Down)
			if (a_Event->Data.Keyboard.KeyCode == IKBK_ESCAPE)
			{
				a_Widget->Data.TextBox.StealInput = false;
				return true;
			}
	}
	
	// Synthetic
	else if (a_Event->Type == IET_SYNTHOSK)
	{
		// Ignore events belonging to another screen
		if (a_Event->Data.SynthOSK.PNum != a_Widget->Screen)
			return false;
		
		// Check if cancel is pressed
		if (a_Event->Data.SynthOSK.Cancel)
		{
			a_Widget->Data.TextBox.StealInput = false;
			return true;
		}
	}
	
	/* Otherwise it is stolen */
	// Handle OSK Event (Virtual board?)
	CONL_OSKSetVisible(a_Widget->Screen, true);
	RetVal = CONL_OSKHandleEvent(a_Event, a_Widget->Screen);
	CONL_OSKSetVisible(a_Widget->Screen, false);
	
	if (RetVal)
		return true;
	
	// Accept Only Keyboard and Synth
	if (a_Event->Type != IET_KEYBOARD && a_Event->Type != IET_SYNTHOSK)
		return false;
	
	// Handle inputter event
	if (CONCTI_HandleEvent(a_Widget->Data.TextBox.Inputter, a_Event))
		return true;
	
	// Always return true to prevent menu stealing
	return true;
}

/* MS_SMTBFSelect() -- Text Box Selected */
static void MS_SMTBFSelect(M_SWidget_t* const a_Widget)
{
	/* Steal input if not selected */
	if (!a_Widget->Data.TextBox.StealInput)
		a_Widget->Data.TextBox.StealInput = true;
}

/* MS_SMTBFTicker() -- Text Box Ticker */
static void MS_SMTBFTicker(M_SWidget_t* const a_Widget)
{
	/* Change of stolen input? */
	if (a_Widget->Data.TextBox.StealInput != a_Widget->Data.TextBox.OldSteal)
	{
		// Set
		a_Widget->Data.TextBox.OldSteal = a_Widget->Data.TextBox.StealInput;
	}
}

/* MS_SMTBFDestroy() -- Box destroyed */
static void MS_SMTBFDestroy(M_SWidget_t* const a_Widget)
{
	/* Destroy Inputter */
	CONCTI_DestroyInput(a_Widget->Data.TextBox.Inputter);
	a_Widget->Data.TextBox.Inputter = NULL;
}

/* MS_SMCreateTextBox() -- Creates editable text box */
static M_SWidget_t* MS_SMCreateTextBox(M_SWidget_t* const a_Parent, const VideoFont_t a_Font, const uint32_t a_Flags, CONCTI_OutBack_t a_CallBack)
{
	M_SWidget_t* New;
	
	/* First create base */
	New = MS_SMCreateBase(a_Parent);
	
	// Parent has no kids, start from base position
	if (a_Parent->NumKids == 1)
	{
		New->x = 12;
		New->y = 5;
	}
	
	// Otherwise add to it
	else
	{
		New->x = a_Parent->Kids[a_Parent->NumKids - 2]->x;
		New->y = a_Parent->Kids[a_Parent->NumKids - 2]->y + a_Parent->Kids[a_Parent->NumKids - 2]->h + 4;
	}
	
	// Width and height matches font
	New->w = V_FontWidth(a_Font);
	New->h = V_FontHeight(a_Font);
	
	New->FDestroy = MS_SMTBFDestroy;
	New->DDraw = MS_SMTBFDDraw;
	New->FSelect = MS_SMTBFSelect;
	New->FEvent = MS_SMTBFEvent;
	New->FTicker = MS_SMTBFTicker;
	
	/* Setup Inputter */
	New->Data.TextBox.Font = a_Font;
	New->Data.TextBox.Flags = a_Flags;
	New->Data.TextBox.Inputter = CONCTI_CreateInput(1, a_CallBack, &New->Data.TextBox.Inputter);
	New->Data.TextBox.Inputter->DataRef = New;
	New->Data.TextBox.Inputter->Screen = New->Screen;
	
	/* Return */
	return New;
}

/* MS_Label_DDraw() -- Draws Label */
void MS_Label_DDraw(M_SWidget_t* const a_Widget)
{
	uint32_t Flags, ValFlags;
	int32_t Width;
	
	/* Base flags */
	Flags = a_Widget->Data.Label.Flags;
	ValFlags = a_Widget->Data.Label.ValFlags;
	
	/* Disabled? */
	if (a_Widget->Flags & MSWF_DISABLED)
	{
		Flags &= VFO_COLORMASK;
		Flags |= VFO_COLOR(VEX_MAP_GRAY);
		
		ValFlags &= VFO_COLORMASK;
		ValFlags |= VFO_COLOR(VEX_MAP_GRAY);
	}
	
	/* Not Disabled */
	else
	{
	}
	
	/* Draw String */
	if (a_Widget->Data.Label.Ref)
		a_Widget->dw = V_DrawStringA(a_Widget->Data.Label.Font, Flags, *a_Widget->Data.Label.Ref, a_Widget->dx, a_Widget->dy);
	
	/* Draw value */
	// Possible Value
	if (a_Widget->Data.Label.Possible)
	{
		// Get width of possible
		Width = V_StringWidthA(a_Widget->Data.Label.Font, ValFlags, a_Widget->Data.Label.Possible[a_Widget->Data.Label.Pivot].StrAlias) + 8;
		
		// Draw it
		V_DrawStringA(a_Widget->Data.Label.Font, ValFlags, a_Widget->Data.Label.Possible[a_Widget->Data.Label.Pivot].StrAlias, a_Widget->Parent->dx + (a_Widget->Parent->dw - Width), a_Widget->dy);
	}
}

/* MS_SMCreateLabel() -- Creates label */
static M_SWidget_t* MS_SMCreateLabel(M_SWidget_t* const a_Parent, const VideoFont_t a_Font, const uint32_t a_Flags, const char** const a_Ref)
{
	M_SWidget_t* New;
	
	/* First create base */
	New = MS_SMCreateBase(a_Parent);
	
	// Parent has no kids, start from base position
	if (a_Parent->NumKids == 1)
	{
		New->x = 12;
		New->y = 5;
	}
	
	// Otherwise add to it
	else
	{
		New->x = a_Parent->Kids[a_Parent->NumKids - 2]->x;
		New->y = a_Parent->Kids[a_Parent->NumKids - 2]->y + a_Parent->Kids[a_Parent->NumKids - 2]->h + 4;
	}
	
	// Width and height matches font
	New->w = V_FontWidth(a_Font);
	New->h = V_FontHeight(a_Font);
	
	// Drawer
	New->DDraw = MS_Label_DDraw;
	
	// Setup
	New->Data.Label.Font = a_Font;
	New->Data.Label.Flags = a_Flags;
	New->Data.Label.Ref = a_Ref;
	
	/* Done */
	return New;
}

/* MS_SMCreateCVarSlide() -- Console Variable Slider */
static M_SWidget_t* MS_SMCreateCVarSlide(M_SWidget_t* const a_Parent, const VideoFont_t a_Font, const uint32_t a_Flags, const uint32_t a_ValFlags, const char** const a_Ref, CONL_StaticVar_t* const a_CVar)
{
	M_SWidget_t* New;
	
	/* Create base label widget first */
	New = MS_SMCreateLabel(a_Parent, a_Font, a_Flags, a_Ref);
	
	/* Setup */
	New->Data.Label.ValFlags = a_ValFlags;
	New->Data.Label.CVar = a_CVar;
	
	/* Return */
	return New;
}

/* MS_PossSlideFLeftRight() -- L/R Possible Slide */
static bool_t MS_PossSlideFLeftRight(M_SWidget_t* const a_Widget, const int32_t a_Right)
{	
	int32_t n;	
	
	/* Find max value of possible value */	
	for (n = 0; a_Widget->Data.Label.Possible[n].StrAlias; n++)
		;
	
	/* Change Pivot */
	a_Widget->Data.Label.Pivot += a_Right;
	
	// Cap
	while (a_Widget->Data.Label.Pivot < 0)
		a_Widget->Data.Label.Pivot += n;
	while (n > 0 && a_Widget->Data.Label.Pivot >= n)
		a_Widget->Data.Label.Pivot -= n;
	
	/* Always emit slide sound */
	S_StartSound(NULL, sfx_generic_menuslide);
}

/* MS_SMCreatePossSlide() -- Possible Value Slider */
static M_SWidget_t* MS_SMCreatePossSlide(M_SWidget_t* const a_Parent, const VideoFont_t a_Font, const uint32_t a_Flags, const uint32_t a_ValFlags, const char** const a_Ref, CONL_VarPossibleValue_t* const a_PossibleList)
{
	M_SWidget_t* New;
	
	/* Create base label widget first */
	New = MS_SMCreateLabel(a_Parent, a_Font, a_Flags, a_Ref);
	
	/* Setup */
	New->Data.Label.ValFlags = a_ValFlags;
	New->Data.Label.Possible = a_PossibleList;
	New->FLeftRight = MS_PossSlideFLeftRight;
	
	/* Return */
	return New;
}

/* MS_Image_DDraw() -- Draws Image */
void MS_Image_DDraw(M_SWidget_t* const a_Widget)
{
	V_ImageDraw(0, a_Widget->Data.Image.Pic, a_Widget->dx, a_Widget->dy, NULL);
}

/* MS_SMCreateImage() -- Creates image widget */
static M_SWidget_t* MS_SMCreateImage(M_SWidget_t* const a_Parent, const int32_t a_X, const int32_t a_Y, V_Image_t* const a_Image)
{
	M_SWidget_t* New;
	
	/* Check */
	if (!a_Image)
		return NULL;
	
	/* First create base */
	New = MS_SMCreateBase(a_Parent);
	
	// Set image props
	New->Data.Image.Pic = a_Image;
	New->x = a_X;
	New->y = a_Y;
	New->w = a_Image->Width;
	New->h = a_Image->Height;
	New->DDraw = MS_Image_DDraw;
	
	/* Return it */
	return New;
}

/* MS_SMDrawWidget() -- Recursively draws widgets */
static void MS_SMDrawWidget(M_SWidget_t* const a_Widget)
{
	int32_t i;
	M_SWidget_t* Sub;
	bool_t Blink;
	
	/* Set drawing location */
	// No parent? First to draw
	if (!a_Widget->Parent)
	{
		a_Widget->dx = a_Widget->x;
		a_Widget->dy = a_Widget->y;
		a_Widget->dw = a_Widget->w;
		a_Widget->dh = a_Widget->h;
	}
	
	// Otherwise base it on parent
	else
	{
		// Offsets are easy
		a_Widget->dx = a_Widget->Parent->dx + a_Widget->x;
		a_Widget->dy = a_Widget->Parent->dy + a_Widget->y;
		
		// Cap sizes
		a_Widget->dw = a_Widget->w;
		if (a_Widget->dw > a_Widget->Parent->dw - a_Widget->x)
			a_Widget->dw = a_Widget->Parent->dw - a_Widget->x;
			
		a_Widget->dh = a_Widget->h;
		if (a_Widget->dh > a_Widget->Parent->dh - a_Widget->y)
			a_Widget->dh = a_Widget->Parent->dh - a_Widget->y;
	}
	
	/* Debug Box */
	if (devparm)
		VHW_HUDDrawBox(VEX_HOLLOW, 0xFF, ((g_ProgramTic & 0x100) ? (g_ProgramTic & 0xFF) : ~(g_ProgramTic & 0xFF)), 0, a_Widget->dx, a_Widget->dy, a_Widget->dx + a_Widget->dw, a_Widget->dy + a_Widget->dh);
	
	/* Draw Kids First */
	for (i = 0; i < a_Widget->NumKids; i++)
	{
		Sub = a_Widget->Kids[i];
		
		// Nothing?
		if (!Sub)
			continue;
		
		// Draw it
		MS_SMDrawWidget(Sub);
		
		// This thing is selected
		if (i == a_Widget->CursorOn)
		{
			// Special cursor drawing function
			if (a_Widget->DCursor)
				a_Widget->DCursor(a_Widget, Sub);
			
			// Generic Cursor
			else
			{
				// Blinked?
				Blink = !!(g_ProgramTic & 0x4);
				
				if (Blink)
					V_DrawCharacterMB(
						VFONT_SMALL,
						VFO_COLOR(VEX_MAP_BRIGHTWHITE),
						"*",
						Sub->dx - (V_FontWidth(VFONT_SMALL) + 4),
						Sub->dy,
						NULL, NULL
					);
			}
		}
	}
	
	/* Draw this widget ontop */
	if (a_Widget->DDraw)
		a_Widget->DDraw(a_Widget);
}

/* MS_GenUDEvt() -- Generic up/down event */
static bool_t MS_GenUDEvt(M_SWidget_t* const a_Widget, const int32_t a_Down)
{
	int32_t OldItem, This;
	
	/* Check */
	if (!a_Widget || !a_Down)
		return false;
	
	/* Widget only has one or none kids */
	if (a_Widget->NumKids <= 1)
		return false;
	
	/* Remember old item */
	This = OldItem = a_Widget->CursorOn;
	
	/* Loop */
	for (;;)
	{
		// Modify movement
		This += a_Down;
		
		// Correct this within bounds
		if (This < 0)
			This = a_Widget->NumKids - 1;
		
		else if (This >= a_Widget->NumKids)
			This = 0;
		
		// Same?
		if (This == OldItem)
			break;
		
		// Illegal kid here
		if (!a_Widget->Kids[This])
			continue;
		
		// Cannot park on kid
		if (a_Widget->Kids[This]->Flags & MSWF_NOSELECT)
			continue;
		
		// Otherwise, done
		break;
	}
	
	/* Set this */
	a_Widget->CursorOn = This;
	
	/* Return */
	if (This != OldItem)
	{
		// Play a sound
		S_StartSound(NULL, sfx_generic_menumove);
		return true;
	}
	
	// Did not move at all
	else
		return false;
}

/* MS_SMWidEvent() -- Handle Widget Events */
static bool_t MS_SMWidEvent(M_SWidget_t* const a_Widget, const I_EventEx_t* const a_Event)
{
	bool_t Handled;
	int32_t Right, Down;
	bool_t Select, Cancel;
	
	/* Init */
	Handled = false;
	
	/* Handle event on selected child */
	if (a_Widget->CursorOn < a_Widget->NumKids)
		if (a_Widget->Kids[a_Widget->CursorOn])
			if (!(a_Widget->Kids[a_Widget->CursorOn]->Flags & MSWF_NOHANDLEEVT))
			{
				Handled = MS_SMWidEvent(a_Widget->Kids[a_Widget->CursorOn], a_Event);
				
				if (Handled)
					return true;
			}
	
	/* Advanced handle event? */
	if (a_Widget->FEvent)
	{
		Handled = a_Widget->FEvent(a_Widget, a_Event);
		
		if (Handled)
			return true;
	}
	
	/* Generic Event Handling */
	// Clear
	Cancel = Select = false;
	Right = Down = 0;
	
	// Keyboard (P1)
	if (a_Event->Type == IET_KEYBOARD)
	{
		switch (a_Event->Data.Keyboard.KeyCode)
		{
			case IKBK_RIGHT: Right = 1; break;
			case IKBK_LEFT: Right = -1; break;
			case IKBK_UP: Down = -1; break;
			case IKBK_DOWN: Down = 1; break;
			case IKBK_ESCAPE: Cancel = true; break;
			case IKBK_RETURN: Select = true; break;
			default: break;
		}
	}
	
	// Synth (Any)
	else if (a_Event->Type == IET_SYNTHOSK)
	{
		Right = a_Event->Data.SynthOSK.Right;
		Down = a_Event->Data.SynthOSK.Down;
		Select = a_Event->Data.SynthOSK.Press;
		Cancel = a_Event->Data.SynthOSK.Cancel;
	}
	
	// Nothing?
	if (!Right && !Down && !Select && !Cancel)
		return false;
	
	// Select
	if (Select)
		if (a_Widget->FSelect)
			return a_Widget->FSelect(a_Widget);
	
	// Cancel
	if (Cancel)
		if (a_Widget->FCancel)
			return a_Widget->FCancel(a_Widget);
	
	// Left/Right?
	if (Right)
		if (a_Widget->FLeftRight)
			return a_Widget->FLeftRight(a_Widget, Right);
	
	// Up/Down?
	if (Down)
	{
		if (a_Widget->FUpDown)
			Handled = a_Widget->FUpDown(a_Widget, Down);
		
		if (Handled)
			return true;
		
		// Generic cursor up/down movement
		return MS_GenUDEvt(a_Widget, Down);
	}
	
	/* Not handled */
	return false;
}

/*** FUNCTIONS ***/

/* M_SMInit() -- Simple Menus */
void M_SMInit(void)
{
}

/* M_SMHandleEvent() -- Handles Event */
bool_t M_SMHandleEvent(const I_EventEx_t* const a_Event)
{
	int32_t Screen;
	
	/* Ignore Up events */
	if (a_Event->Type == IET_KEYBOARD && !a_Event->Data.Keyboard.Down)
		return false;
	
	/* Ignore joysticks */
	if (a_Event->Type == IET_JOYSTICK)
		return false;
	
	/* Get screen event is for, otherwise it is for player 1 */
	Screen = 0;
	if (a_Event->Type == IET_SYNTHOSK)
		Screen = a_Event->Data.SynthOSK.PNum;
	
	/* No menus for this player */
	if (!l_NumSMWi[Screen])
	{
		// If hitting ESC, spawn main menu (P1 Only)
		if (Screen == 0 && a_Event->Type == IET_KEYBOARD &&
			a_Event->Data.Keyboard.KeyCode == IKBK_ESCAPE)
		{
			// Cheat, if P1 is chatting, cancel chat rather than popping menu up
			if (g_Splits[Screen].ChatMode && D_ScrSplitHasPlayer(Screen))
				;//D_XNetClearChat(Screen);
			
			// Otherwise, spawn main menu
			else
				M_SMSpawn(Screen, MSM_MAIN);
			
			return true;
		}
	}
	
	/* Menus for this player */
	else
	{
		// Handle Widget events
		if (MS_SMWidEvent(l_SMWi[Screen][l_NumSMWi[Screen] - 1], a_Event))
			return true;
			
		// Generic ESC pop
		if ((Screen == 0 && a_Event->Type == IET_KEYBOARD &&
			a_Event->Data.Keyboard.KeyCode == IKBK_ESCAPE) ||
			(a_Event->Type == IET_SYNTHOSK && a_Event->Data.SynthOSK.Cancel))
		{
			M_StackPop(Screen);
			return true;
		}
		
		// Eat all unhandled events for the first screen
		if (Screen == 0)
			return true;
	}
	
	/* Not handled */
	return false;
}

/* M_SMDoGrab() -- Grabbing mouse */
bool_t M_SMDoGrab(void)
{
	/* Only for player 1 */
	return !!l_NumSMWi[0];
}

/* M_SMPlayerMenuVisible() -- Player has visible menu */
bool_t M_SMPlayerMenuVisible(const int32_t a_ScreenID)
{
	/* Check */
	if (a_ScreenID < 0 || a_ScreenID >= MAXSPLITSCREEN)
		return false;	
	
	return !!l_NumSMWi[a_ScreenID];
}

/* M_SMMenuVisible() -- Menu is visible */
bool_t M_SMMenuVisible(void)
{
	int32_t i;
	
	/* If any player has a visible menu */
	for (i = 0; i < MAXSPLITSCREEN; i++)
		if (M_SMPlayerMenuVisible(i))
			return true;
	
	/* Not visible */
	return false;
}

/* M_SMGenSynth() -- Generate synth events? */
bool_t M_SMGenSynth(const int32_t a_ScreenID)
{
	if (a_ScreenID < 0 || a_ScreenID >= MAXSPLITSCREEN)
		return false;
	return !!l_NumSMWi[a_ScreenID];
}

/* M_SMFreezeGame() -- Freeze game? */
bool_t M_SMFreezeGame(void)
{
	int i, OpenCount;
	
	/* Never on demo */
	if (demoplayback)	
		return false;
	
	/* Only for player 1, if playing alone */
	if (g_SplitScreen == 0)
		return !!l_NumSMWi[0];
	
	/* Otherwise, see if all players inside have menus open */
	for (i = OpenCount = 0; i < g_SplitScreen + 1; i++)
		if (g_Splits[i].Active || g_Splits[i].Waiting)
			if (l_NumSMWi[i])
				OpenCount++;
	
	// If all menus are open
	return !!(OpenCount == (g_SplitScreen + 1));
}

/* M_SMDrawer() -- Draws menu */
void M_SMDrawer(void)
{
	int32_t Screen;
	bool_t DrawCursor;
	
	/* For all screens */
	DrawCursor = false;
	for (Screen = 0; Screen < MAXSPLITSCREEN; Screen++)
	{
		// No Menus?
		if (!l_NumSMWi[Screen])
			continue;
		
		// Draw recursive menu stuff
		MS_SMDrawWidget(l_SMWi[Screen][l_NumSMWi[Screen] - 1]);
		
		// Cursor?
		if (Screen == 0)
			DrawCursor = true;
	}
	
	/* Draw Mouse? */
	if (DrawCursor)
		CONL_DrawMouse();
}

/* M_SMTicker() -- Ticks simple menu */
void M_SMTicker(void)
{
	int32_t Screen, i;
	M_SWidget_t* TopMenu;
	
	/* For all screens */
	for (Screen = 0; Screen < MAXSPLITSCREEN; Screen++)
	{
		// No Menus?
		if (!l_NumSMWi[Screen])
			continue;
		
		// Tick menu stuff in this menu
		TopMenu = l_SMWi[Screen][l_NumSMWi[Screen] - 1];
		
		// No menu?
		if (!TopMenu)
			continue;
		
		// Tick it
		if (TopMenu->FTicker)
			TopMenu->FTicker(TopMenu);
		
		// Tick Kids
		for (i = 0; i < TopMenu->NumKids; i++)
			if (TopMenu->Kids[i])
				if (TopMenu->Kids[i]->FTicker)
					TopMenu->Kids[i]->FTicker(TopMenu->Kids[i]);
	}
}

/* M_SMSpawn() -- Spawns menu for player */
void* M_SMSpawn(const int32_t a_ScreenID, const M_SMMenus_t a_MenuID)
{
#define SUBMENUFLAGS (VFO_COLOR(VEX_MAP_BRIGHTWHITE))
#define SORTFLAGS (VFO_COLOR(VEX_MAP_ORANGE))
#define VALUEFLAGS (VFO_COLOR(VEX_MAP_BRIGHTWHITE))
	M_SWidget_t* Root, *Work;
	CONL_VarPossibleValue_t* Possible;
	int32_t i;
	
	/* Check */
	if (a_ScreenID < 0 || a_ScreenID >= MAXSPLITSCREEN || a_MenuID < 0 || a_MenuID >= NUMMSMMENUS)
		return NULL;
	
	/* Init */
	Root = NULL;
	
	/* Which menu to spawn? */
	switch (a_MenuID)
	{
			// Main Menu Wrapper
		case MSM_MAIN:
			if (g_CoreGame == CG_DOOM)
				M_SMSpawn(a_ScreenID, MSM_MAINDOOM);
			else if (g_CoreGame == CG_HERETIC)
				M_SMSpawn(a_ScreenID, MSM_MAINHERETIC);
			break;
		
			// Main Menu (Doom)
		case MSM_MAINDOOM:
			// Create initial box
			Root = MS_SMCreateBox(NULL, 0, 0, 320, 200);
			
			// Use skull cursor instead
			Root->DCursor = M_MainMenu_DCursor;
			
			// Nice Doom picture
			Work = MS_SMCreateImage(Root, 94, 2, V_ImageFindA("M_DOOM", VCP_DOOM));
			Work->Flags |= MSWF_NOSELECT;
			
			// Main Menu Stuff
			Work = MS_SMCreateImage(Root, 97, 64, V_ImageFindA("M_NGAME", VCP_DOOM));
			Work->FSelect = M_SubMenu_FSelect;
			Work->SubMenu = MSM_NEWGAME;
			
			Work = MS_SMCreateImage(Root, 97, 80, V_ImageFindA("M_OPTION", VCP_DOOM));
			Work->FSelect = M_SubMenu_FSelect;
			Work->SubMenu = MSM_OPTIONS;
			
			Work = MS_SMCreateImage(Root, 97, 96, V_ImageFindA("M_LOADG", VCP_DOOM));
			Work = MS_SMCreateImage(Root, 97, 112, V_ImageFindA("M_SAVEG", VCP_DOOM));
			
			Work = MS_SMCreateImage(Root, 97, 128, V_ImageFindA("M_QUITG", VCP_DOOM));
			Work->FSelect = M_SubMenu_FSelect;
			Work->SubMenu = MSM_QUITGAME;
			
			// Start on new game
			Root->CursorOn = 1;
			break;
			
			// Heretic Main Menu
		case MSM_MAINHERETIC:
			break;
			
			// New Game
		case MSM_NEWGAME:
			// Create initial box
			Root = MS_SMCreateBox(NULL, 0, 0, 320, 200);
			
			// Add sub options
				// New Game
			Work = MS_SMCreateLabel(Root, VFONT_SMALL, SUBMENUFLAGS, DS_GetStringRef(DSTR_MENUNEWGAME_CLASSIC));
			Work->FSelect = M_NewGameClassic_FSelect;
			
				// More advanced settings
			Work = MS_SMCreateLabel(Root, VFONT_SMALL, SUBMENUFLAGS, DS_GetStringRef(DSTR_MENUNEWGAME_CREATEGAME));
			Work->FSelect = M_SubMenu_FSelect;
			Work->SubMenu = MSM_ADVANCEDCREATEGAME;
				
				// Connect to unlisted server
			Work = MS_SMCreateLabel(Root, VFONT_SMALL, SUBMENUFLAGS, DS_GetStringRef(DSTR_MENUNEWGAME_UNLISTEDIP));
			Work->FSelect = M_SubMenu_FSelect;
			Work->SubMenu = MSM_JOINUNLISTEDSERVER;
			
				// Server List: Name
			Work = MS_SMCreateLabel(Root, VFONT_SMALL, SORTFLAGS, DS_GetStringRef(DSTR_MENUGENERAL_SVNAME));
			break;
			
			// Select Skill
		case MSM_SKILLSELECTDOOM:
			// Create initial box
			Root = MS_SMCreateBox(NULL, 0, 0, 320, 200);
			
			// Use skull cursor instead
			Root->DCursor = M_MainMenu_DCursor;
			
			// Nice new game picture
			Work = MS_SMCreateImage(Root, 96, 14, V_ImageFindA("M_NEWG", VCP_DOOM));
			Work->Flags |= MSWF_NOSELECT;
			
			// Nice skill picture
			Work = MS_SMCreateImage(Root, 54, 38, V_ImageFindA("M_SKILL", VCP_DOOM));
			Work->Flags |= MSWF_NOSELECT;
			
			// Skill Select
			Work = MS_SMCreateImage(Root, 48, 63, V_ImageFindA("M_JKILL", VCP_DOOM));
			Work->Option = 0;
			Work->FSelect = M_NewGameSkill_FSelect;
			
			Work = MS_SMCreateImage(Root, 48, 79, V_ImageFindA("M_ROUGH", VCP_DOOM));
			Work->Option = 1;
			Work->FSelect = M_NewGameSkill_FSelect;
			
			Work = MS_SMCreateImage(Root, 48, 95, V_ImageFindA("M_HURT", VCP_DOOM));
			Work->Option = 2;
			Work->FSelect = M_NewGameSkill_FSelect;
			
			Work = MS_SMCreateImage(Root, 48, 111, V_ImageFindA("M_ULTRA", VCP_DOOM));
			Work->Option = 3;
			Work->FSelect = M_NewGameSkill_FSelect;
			
			Work = MS_SMCreateImage(Root, 48, 127, V_ImageFindA("M_NMARE", VCP_DOOM));
			Work->Option = 4;
			Work->FSelect = M_NewGameSkill_FSelect;
			
			// Start on HMP
			Root->CursorOn = 4;
			break;
		
			// Episode Select
		case MSM_EPISELECTUDOOM:
		case MSM_EPISELECTDOOM:
			// Create initial box
			Root = MS_SMCreateBox(NULL, 0, 0, 320, 200);
			
			// Use skull cursor instead
			Root->DCursor = M_MainMenu_DCursor;
			
			// Nice episode choosing picture
			Work = MS_SMCreateImage(Root, 54, 38, V_ImageFindA("M_EPISOD", VCP_DOOM));
			Work->Flags |= MSWF_NOSELECT;
			
			// Episode Select
			Work = MS_SMCreateImage(Root, 48, 63, V_ImageFindA("M_EPI1", VCP_DOOM));
			Work->Option = 1;
			Work->FSelect = M_NewGameEpi_FSelect;
			
			Work = MS_SMCreateImage(Root, 48, 79, V_ImageFindA("M_EPI2", VCP_DOOM));
			Work->Option = 2;
			Work->FSelect = M_NewGameEpi_FSelect;
			
			Work = MS_SMCreateImage(Root, 48, 95, V_ImageFindA("M_EPI3", VCP_DOOM));
			Work->Option = 3;
			Work->FSelect = M_NewGameEpi_FSelect;
			
			if (a_MenuID == MSM_EPISELECTUDOOM)
			{
				Work = MS_SMCreateImage(Root, 48, 111, V_ImageFindA("M_EPI4", VCP_DOOM));
				Work->Option = 4;
				Work->FSelect = M_NewGameEpi_FSelect;
			}
			
			// Start on Episode
			Root->CursorOn = 1;
			break;
			
			// Advanced Game Creation Menu
		case MSM_ADVANCEDCREATEGAME:
			// Create initial box
			Root = MS_SMCreateBox(NULL, 0, 0, 320, 200);
			
			// Connection Type
			//Work = MS_SMCreateLabel(Root, VFONT_SMALL, SUBMENUFLAGS, DS_GetStringRef(DSTR_MENUQUIT_DISCONNECT));
			
			// IWAD to play with (UDoom, HOE, etc.)
			i = M_HelpInitIWADList(&Possible);
			Work = MS_SMCreatePossSlide(Root, VFONT_SMALL, 0, VALUEFLAGS, DS_GetStringRef(DSTR_MENUCREATEGAME_IWADTITLE), Possible);
			Work->Data.Label.Pivot = i;
			Work->Option = MCGO_IWAD;
			
			// Start Game
			Work = MS_SMCreateLabel(Root, VFONT_SMALL, SUBMENUFLAGS, DS_GetStringRef(DSTR_MENUCREATEGAME_STARTGAME));
			Work->FSelect = M_ACG_CreateFSelect;
			break;
		
			// Join Unlisted Server
		case MSM_JOINUNLISTEDSERVER:
			// Create initial box
			Root = MS_SMCreateBox(NULL, 0, 0, 320, 200);
			
			// Address Label
			Work = MS_SMCreateLabel(Root, VFONT_SMALL, SORTFLAGS, DS_GetStringRef(DSTR_MENUUNLISTED_ADDRESS));
			Work->Flags |= MSWF_NOSELECT;
			
			// Edit Box
			Work = MS_SMCreateTextBox(Root, VFONT_SMALL, 0, M_CTUS_BoxCallBack);
			Work->Option = 1337;
			
			// Connect
			Work = MS_SMCreateLabel(Root, VFONT_SMALL, SUBMENUFLAGS, DS_GetStringRef(DSTR_MENUUNLISTED_CONNECT));
			Work->FSelect = M_CTUS_ConnectFSelect;
			
			// Start on text box
			Root->CursorOn = 1;
			break;
			
			// Quit Game
		case MSM_QUITGAME:
			// Create initial box
			Root = MS_SMCreateBox(NULL, 0, 0, 320, 200);
			
			// Options
			Root->FTicker = M_QuitGame_FTicker;
			
			// Add sub options
				// Disconnect
			Work = MS_SMCreateLabel(Root, VFONT_SMALL, SUBMENUFLAGS, DS_GetStringRef(DSTR_MENUQUIT_DISCONNECT));
			Work->FSelect = M_QuitGame_DisconFSelect;
			
				// Partial Disconnect
			Work = MS_SMCreateLabel(Root, VFONT_SMALL, SUBMENUFLAGS, DS_GetStringRef(DSTR_MENUQUIT_PDISCONNECT));
			Work->FSelect = M_QuitGame_PDisconFSelect;
			
				// Stop Watching
			Work = MS_SMCreateLabel(Root, VFONT_SMALL, SUBMENUFLAGS, DS_GetStringRef(DSTR_MENUQUIT_STOPWATCHING));
			Work->FSelect = M_QuitGame_StopWatchFSelect;
			
				// Stop Recording
			Work = MS_SMCreateLabel(Root, VFONT_SMALL, SUBMENUFLAGS, DS_GetStringRef(DSTR_MENUQUIT_STOPRECORDING));
			Work->FSelect = M_QuitGame_StopRecordFSelect;
			
				// Log Off
			Work = MS_SMCreateLabel(Root, VFONT_SMALL, SUBMENUFLAGS, DS_GetStringRef(DSTR_MENUQUIT_LOGOFF));
			Work->FSelect = M_QuitGame_LogOffFSelect;
			
				// Exit ReMooD
			Work = MS_SMCreateLabel(Root, VFONT_SMALL, SUBMENUFLAGS, DS_GetStringRef(DSTR_MENUQUIT_EXITREMOOD));
			Work->FSelect = M_QuitGame_ExitFSelect;
			
			// Call ticker once, to gray out
			M_QuitGame_FTicker(Root);
			break;
		
			// Options Menu
		case MSM_OPTIONS:
			// Create initial box
			Root = MS_SMCreateBox(NULL, 0, 0, 320, 200);
			
			// Create profile manager
				// Disconnect
			Work = MS_SMCreateLabel(Root, VFONT_SMALL, SUBMENUFLAGS, DS_GetStringRef(DSTR_MENUOPTION_MANAGEPROF));
			Work->FSelect = M_SubMenu_FSelect;
			Work->SubMenu = MSM_PROFMAN;
			
			break;
			
			// Profile Manager
		case MSM_PROFMAN:
			// Create initial box
			Root = MS_SMCreateBox(NULL, 0, 0, 320, 200);
			
			// Add menu ticker
			Root->FTicker = M_ProfMan_FTicker;
			
			// Add create profile option
			Work = MS_SMCreateLabel(Root, VFONT_SMALL, SUBMENUFLAGS, DS_GetStringRef(DSTR_MENUOPTION_CREATEPROF));
			Work->FSelect = M_ProfMan_CreateProf;
			
			// Create MAXPROFCONST menu slots for profiles
			for (i = 0; i < MAXPROFCONST; i++)
			{
				Work = MS_SMCreateLabel(Root, VFONT_SMALL, 0, DS_GetStringRef(DSTR_MENU_NULLSPACE));
				Work->Option = i;
				Work->FSelect = M_ProfMan_IndvFSel;
			}
			break;
		
			// Modify profile
		case MSM_PROFMOD:
			// Create initial box
			Root = MS_SMCreateBox(NULL, 0, 0, 320, 200);
			
			// Create profile name thing
			Work = MS_SMCreateTextBox(Root, VFONT_SMALL, 0, M_ProfMan_AcctBCB);
			
			if (Work)
			{
				if (g_DoProf)
					CONCTI_SetText(Work->Data.TextBox.Inputter, g_DoProf->AccountName);
			}
			break;
		
			// Unknown
		default:
			break;
	}
	
	/* Pushing? */
	if (Root)
	{
		Root->Screen = a_ScreenID;
		MS_SMStackPush(a_ScreenID, Root);
	}
	
	/* Return created menu */
	return Root;
#undef SUBMENUFLAGS
#undef SORTFLAGS
}

