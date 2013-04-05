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

/*******************************************************************************
********************************************************************************
*******************************************************************************/

/*** CONSTANTS ***/

//#define __REMOOD_FORCESINGLE (demoplayback || gamestate == GS_DEMOSCREEN)
//#define __REMOOD_NUMSPLITS (__REMOOD_FORCESINGLE ? 1 : (g_SplitScreen > 0 ? g_SplitScreen + 1 : 1))

#define __REMOOD_NUMSPLITS (MS_NumScrSplits())

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

/*** GLOBALS ***/

int32_t g_ResumeMenu = 0;						// Resume menu for n tics

/*** LOCALS ***/

static M_UILocalBox_t** l_UIBoxes = NULL;
static size_t l_NumUIBoxes = 0;

static M_UIMenuHandler_t** l_UIMenus[MAXSPLITSCREEN];
static size_t l_NumUIMenus[MAXSPLITSCREEN];

// menu_font -- Menu Font
CONL_StaticVar_t l_MenuFont =
{
	CLVT_INTEGER, c_CVPVFont, CLVF_SAVE,
	"menu_font", DSTR_CVHINT_MENUFONT, CLVVT_STRING, "SMALL",
	NULL
};

// menu_headercolor -- Color of menu items
CONL_StaticVar_t l_MenuHeaderColor =
{
	CLVT_INTEGER, c_CVPVVexColor, CLVF_SAVE,
	"menu_headercolor", DSTR_CVHINT_MENUHEADERCOLOR, CLVVT_STRING, "Green",
	NULL
};

// menu_itemcolor -- Color of menu items
CONL_StaticVar_t l_MenuItemColor =
{
	CLVT_INTEGER, c_CVPVVexColor, CLVF_SAVE,
	"menu_itemcolor", DSTR_CVHINT_MENUITEMCOLOR, CLVVT_STRING, "Red",
	NULL
};

// menu_valcolor -- Color of menu items
CONL_StaticVar_t l_MenuValColor =
{
	CLVT_INTEGER, c_CVPVVexColor, CLVF_SAVE,
	"menu_valcolor", DSTR_CVHINT_MENUVALSCOLOR, CLVVT_STRING, "BrightWhite",
	NULL
};


// menu_compact -- Compact Menu
CONL_StaticVar_t l_MenuCompact =
{
	CLVT_INTEGER, c_CVPVBoolean, CLVF_SAVE,
	"menu_compact", DSTR_CVHINT_CONMONOSPACE, CLVVT_STRING, "true",
	NULL
};

/*** MENU FUNCTIONS ***/

/* MS_NumScrSplits() -- Calculate True Split Count */
static int8_t MS_NumScrSplits(void)
{
	int8_t Ret, i;
	
	/* Go through players */
	for (i = 0; i < MAXSPLITSCREEN; i++)
		if (!D_ScrSplitHasPlayer(i))
		{
			// Always return something
			if (i == 0)
				return 0;
			return i - 1;
		}
	
	/* Return the max */
	return 3;
}

int M_ExGeneralComm(const uint32_t a_ArgC, const char** const a_ArgV);

int M_ExMultiMenuCom(const uint32_t a_ArgC, const char** const a_ArgV);

/* M_MenuExInit() -- Init Menu */
void M_MenuExInit(void)
{
	CONL_VarRegister(&l_MenuFont);
	CONL_VarRegister(&l_MenuCompact);
	CONL_VarRegister(&l_MenuHeaderColor);
	CONL_VarRegister(&l_MenuItemColor);
	CONL_VarRegister(&l_MenuValColor);
	
	CONL_AddCommand("menucolortest", M_ExGeneralComm);
	CONL_AddCommand("menutexturetest", M_ExGeneralComm);
	
	/* Normal Commands */
	CONL_AddCommand("m_quitprompt", M_ExMultiMenuCom);
	CONL_AddCommand("m_startclassic", M_ExMultiMenuCom);
	CONL_AddCommand("m_startclassic2", M_ExMultiMenuCom);
	CONL_AddCommand("m_classicmap", M_ExMultiMenuCom);
	CONL_AddCommand("m_usethisprof", M_ExMultiMenuCom);
	CONL_AddCommand("m_createprof", M_ExMultiMenuCom);
}

/* M_ExMenuHandleEvent() -- Handles Menu Events */
bool_t M_ExMenuHandleEvent(const I_EventEx_t* const a_Event)
{
	int32_t i, RowPos, RowEnd, DoDown, DoRight, z;
	M_UIMenuHandler_t* TopMenu;
	M_UIMenu_t* UI;
	bool_t Up, DoCancel, DoPress;
	
	/* Player 1 has no menu? */
	if (!l_NumUIMenus[0])
		if (a_Event->Type == IET_KEYBOARD)
			if (a_Event->Data.Keyboard.Down &&
				a_Event->Data.Keyboard.KeyCode == IKBK_ESCAPE)
			{
				M_ExPushMenu(0, M_ExMakeMenu(M_ExMenuIDByName("mainmenu"), NULL));
				return true;
			}
	
	/* Control for each player */
	for (i = 0; i < __REMOOD_NUMSPLITS + 1; i++)
	{
		// No menus for this player?
		if (!l_NumUIMenus[i])
			continue;
		
		// Get top menu
		TopMenu = l_UIMenus[i][l_NumUIMenus[i] - 1];
		
		// Menu Here?
		if (!TopMenu)
			continue;
		
		// Get UI
		UI = TopMenu->UIMenu;
		Up = false;
		DoRight = DoDown = 0;
		DoCancel = DoPress = false;
		
		// Which key command?
		if (a_Event->Type == IET_SYNTHOSK)
		{
			// Not this player?
			if (a_Event->Data.SynthOSK.PNum != i)
				continue;
			
			// Waiting on OSK?
			if (g_ProgramTic < TopMenu->OSKWait)
				continue;
			
			// Move menu up/down?
			if (a_Event->Data.SynthOSK.Down != 0)
				DoDown = a_Event->Data.SynthOSK.Down;
			
			// Change value left/right?
			else if (a_Event->Data.SynthOSK.Right != 0)
				DoRight = a_Event->Data.SynthOSK.Right;
			
			// Cancel
			else if (a_Event->Data.SynthOSK.Cancel)
				DoCancel = true;
			
			// Press
			else if (a_Event->Data.SynthOSK.Press)
				DoPress = true;
			
			// Un-handled
			else
				continue;
			
			// Handled somewhere
			TopMenu->OSKWait = g_ProgramTic + (TICRATE >> 3);
		}
		
		// Normal Menu access (Only by Player 1)
		else if (i == 0 && a_Event->Type == IET_KEYBOARD && a_Event->Data.Keyboard.Down)
		{
			switch (a_Event->Data.Keyboard.KeyCode)
			{
				case IKBK_LEFT:
					DoRight = -1;
					break;
				
				case IKBK_RIGHT:
					DoRight = 1;
					break;
				
				case IKBK_DOWN:
					DoDown = 1;
					break;
				
				case IKBK_UP:
					DoDown = -1;
					break;
				
				case IKBK_ESCAPE:
					DoCancel = true;
					break;
				
				case IKBK_RETURN:
					DoPress = true;
					break;
				
				default:
					continue;
			}
		}
		else
			continue;
			
		// Cancel Menu
		if (DoCancel)
		{
			// Pop Menu
			S_StartSound(NULL, sfx_generic_switchoff);
			M_ExPopMenu(i);
			return true;	// always handled
		}
		
		// No menu items?
		if (!UI->NumItems)
			continue;
		
		// Up/Down?
		if (DoDown != 0)
		{
			TopMenu->CurItem += DoDown;
			Up = DoDown > 0;
		}
		
		// Non-parkable Item?
		for (;;)
		{
			// Passed items?
			if (TopMenu->CurItem < 0)
				TopMenu->StartOff = TopMenu->CurItem = TopMenu->UIMenu->NumItems - 1;
			else if (TopMenu->CurItem >= TopMenu->UIMenu->NumItems)
				TopMenu->StartOff = TopMenu->CurItem = 0;
			
			// Non-parkable item?
			if (TopMenu->UIMenu->Items[TopMenu->CurItem].Flags & MUIIF_NOPARK)
				if (Up)
					TopMenu->CurItem++;
				else
					TopMenu->CurItem--;
			else
				break;
		}
		
		// Item changed? Play sound
		if (TopMenu->CurItem != TopMenu->OldCurItem)
			S_StartSound(NULL, sfx_generic_menumove);
		
		// Change view?
		for (z = 0; z < 10; z++)
		{
			// Really small menu?
			if (UI->NumItems <= 5)
			{
				TopMenu->StartOff = 0;
				break;
			}
			
			// Try to bound
			RowPos = TopMenu->CurItem - TopMenu->StartOff;
			RowEnd = TopMenu->StartOff + TopMenu->IPS;
		
			// Absolute At Start?
			if (TopMenu->CurItem < 2)
			{
				TopMenu->StartOff = 0;
				break;
			}
			
			// First 2 items
			else if (RowPos < 2)
				TopMenu->StartOff--;
		
			// Last 2 items
			else if (RowPos > TopMenu->IPS - 2)
				TopMenu->StartOff++;
			
			// In the middle
			else
				break;
		
			// Capped?
			if (TopMenu->StartOff < 0)
				TopMenu->StartOff = 0;
			else if (TopMenu->StartOff + TopMenu->IPS >= UI->NumItems)
				TopMenu->StartOff = UI->NumItems - TopMenu->IPS;
		}

		//TopMenu->StartOff = TopMenu->CurItem;
		TopMenu->OldCurItem = TopMenu->CurItem;
		
		// Disabled Item?
		if (TopMenu->UIMenu->Items[TopMenu->CurItem].Flags & MUIIF_DISABLED)
		{
			// Make sound?
			if ((DoRight && UI->Items[TopMenu->CurItem].LRValChangeFunc) ||
				(DoPress && UI->Items[TopMenu->CurItem].ItemPressFunc))
				S_StartSound(NULL, sfx_generic_menufail);
		}
		
		// Perform action on it
		else
		{
			// Left/Right?
			if (DoRight != 0)
				if (UI->Items[TopMenu->CurItem].LRValChangeFunc)
					if (UI->Items[TopMenu->CurItem].LRValChangeFunc(i, UI, &UI->Items[TopMenu->CurItem], DoRight > 0))
						S_StartSound(NULL, sfx_generic_menuslide);
		
			// Press
			if (DoPress != 0)
				if (UI->Items[TopMenu->CurItem].ItemPressFunc)
					if (UI->Items[TopMenu->CurItem].ItemPressFunc(i, UI, &UI->Items[TopMenu->CurItem]))
						S_StartSound(NULL, sfx_generic_menupress);
		}
		
		// Was handled
		return true;
	}
	
	/* Un-Handled */
	return false;
}

/*** MB FUNCTIONS ***/

/* M_ExAllUIActive() -- Returns true if all player UIs are active */
bool_t M_ExAllUIActive(void)
{
	int i, Count, Total;
	
	/* Go through players */
	Count = Total = 0;
	for (i = 0; i < MAXSPLITSCREEN; i++)
		if (D_ScrSplitHasPlayer(i))
		{
			Total++;
			if (M_ExPlayerUIActive(i))
				Count++;
		}
	
	/* If total is the same as count */
	if (Total > 0 && Total == Count)
		return true;
	return false;
}

/* M_ExPlayerUIActive() -- This player's menu is active */
bool_t M_ExPlayerUIActive(const uint32_t a_Player)
{
	/* Check */
	if (a_Player < 0 || a_Player >= MAXSPLITSCREEN)
		return false;
	
	/* Out of bounds? */
	if (a_Player >= __REMOOD_NUMSPLITS + 1)
		return false;
	
	/* Return active state */
	return l_NumUIMenus[a_Player];
}

/* M_ExUIActive() -- Returns true if UI is active */
bool_t M_ExUIActive(void)
{
	int32_t i;
	
	/* UI Boxes Open */
	if (l_NumUIBoxes)
		return true;
	
	/* Check menu count as per drawer/handler rules */
	for (i = 0; i < __REMOOD_NUMSPLITS + 1; i++)
		if (l_NumUIMenus[i])
			return true;
	
	return false;
}

/* M_ExMenuDrawer() -- Draws Menu */
void M_ExMenuDrawer(void)
{
	int32_t ScrX, ScrY, ScrW, ScrH;
	int ValW, ValH;
	int32_t i, j, x, y, xa, ya, xb, yb;
	M_UIMenuHandler_t* TopMenu;
	M_UIMenu_t* UI;
	M_UIItem_t* Item;
	const char* TitleStr;
	const char* ValStr;
	uint32_t DrawFlags;
	bool_t DrewAMenu;
	
	/* Draw for each player */
	DrewAMenu = false;
	for (i = 0; i < __REMOOD_NUMSPLITS + 1; i++)
	{
		// No menus for this player?
		if (!l_NumUIMenus[i])
			continue;
		
		// Menu was drawn!
		DrewAMenu = true;
		
		// Get base screen size
		ScrX = 0;
		ScrY = 0;
		ScrW = 320;
		ScrH = 200;
		
		// Cover full menu in playback
		if (!(demoplayback || gamestate == GS_DEMOSCREEN))
		{
			// 2+ Split = Half Height
			if (__REMOOD_NUMSPLITS > 0)
				ScrH >>= 1;
		
			// 3+ Split = Half Width
			if (__REMOOD_NUMSPLITS > 1)
				ScrW >>= 1;
		
			// Modify h/w?
				// 2 player
			if (__REMOOD_NUMSPLITS == 1)
				ScrY = ScrH * (i & 1);
		
				// 3/4 player
			else if (__REMOOD_NUMSPLITS > 1)
			{
				ScrX = ScrW * (i & 1);
				ScrY = ScrH * ((i >> 1) & 1);
			}
		}
		
		// Get top menu
		TopMenu = l_UIMenus[i][l_NumUIMenus[i] - 1];
		
		// Menu Here?
		if (!TopMenu)
			continue;
			
		// Draw Faded Background
		//VHW_HUDBlurBack(
		//		VEX_COLORMAP(VEX_MAP_BLACK),
		//		ScrX, ScrY, ScrX + ScrW, ScrY + ScrH
		//	);
		
		// Get UI
		UI = TopMenu->UIMenu;
		
		// Draw underneath
		if (UI->UnderDrawFunc)
			if (!UI->UnderDrawFunc(i, TopMenu, UI, ScrX, ScrY, ScrW, ScrH))
				continue;
			
		// Base
		y = ScrY + 4;
		
		// Draw Title Picture?
		if (UI->TitlePic)
		{
			V_ImageDraw(0, UI->TitlePic, 0, y, NULL);
			y += UI->TitlePic->Height;
		}
		
		// Draw Title Text
		else
		{
			if (UI->TitleRef)
				TitleStr = *UI->TitleRef;
			else
				TitleStr = UI->Title;
			
			// Draw if set
			if (TitleStr)
			{
				V_DrawStringA(VFONT_LARGE, 0, TitleStr, ScrX, ScrY + 2);
				y += V_FontHeight(VFONT_LARGE) + 4;
			}
		}
		
		// Base Position
		x = ScrX + 10;
		ya = V_FontHeight(l_MenuFont.Value[0].Int) + 2;
		TopMenu->IPS = ((ScrH - 20) - y) / ya;
		
		// Draw each option
		for (j = TopMenu->StartOff; j < UI->NumItems && y < ((ScrY + ScrH) - 20); j++, y += ya)
		{
			// Get Item
			Item = &UI->Items[j];
			
			// Get Strings
				// Text
			if (Item->TextRef)
				TitleStr = *Item->TextRef;
			else
				TitleStr = Item->Text;
				
				// Value
			ValStr = NULL;
			if (Item->ValueFunc)
				Item->ValueFunc(i, TopMenu, Item, &ValStr);
			else if (Item->ValueRef)
				if (TopMenu->PrivateData)
					ValStr = *((const char**)(((uintptr_t)TopMenu->PrivateData) + (uintptr_t)Item->ValueRef));
				else
					ValStr = *Item->ValueRef;
			else
				ValStr = Item->Value;
			
			// Draw Cursor
			if (j == TopMenu->CurItem)
				if (g_ProgramTic & 0x4)
					V_DrawCharacterMB(
						l_MenuFont.Value[0].Int,
						VFO_COLOR(VEX_MAP_BRIGHTWHITE),
						"*", x - 10, y, NULL, NULL
					);
			
			// Draw Flags
			DrawFlags = 0;
			
			if (Item->Type == MUIIT_HEADER)
				DrawFlags |= VFO_COLOR(l_MenuHeaderColor.Value[0].Int);
			else
			{
				// Disabled
				if (Item->Flags & MUIIF_DISABLED)
				{
					// Currently Selected?
					if (j == TopMenu->CurItem && !(g_ProgramTic & 0x8))
						DrawFlags |= VFO_COLOR(VEX_MAP_RED);
					else
						DrawFlags |= VFO_COLOR(VEX_MAP_GRAY);
				}
				
				// Selected (Show indicator)
				else if (j == TopMenu->CurItem)
				{
					if (g_ProgramTic & 0x8)
						DrawFlags |= VFO_COLOR(VEX_MAP_YELLOW);
					else
						DrawFlags |= VFO_COLOR(VEX_MAP_RED);
				}
				
				// Un-Selected
				else
					DrawFlags |= VFO_COLOR(l_MenuItemColor.Value[0].Int);
			}
			
			// Draw Item Text
			xa = V_DrawStringA(
					l_MenuFont.Value[0].Int,
					DrawFlags,
					TitleStr, x, y
				);
			
			// Value to draw?
			if (ValStr)
			{
				// Get Dimensions of value
				V_StringDimensionsA(l_MenuFont.Value[0].Int, 0, ValStr, &ValW, &ValH);
				
				// X position is given
				xb = (ScrX + (ScrW - 10)) - ValW;
				
				// If menu is not compact add some more stuff
				if (!l_MenuCompact.Value[0].Int)
					y += ya;
				yb = y;
				
				// Draw
				V_DrawStringA(
						l_MenuFont.Value[0].Int,
						VFO_COLOR(l_MenuValColor.Value[0].Int),
						ValStr, xb, yb
					);
			}
		}
		
		// Draw over
		if (UI->OverDrawFunc)
			if (!UI->OverDrawFunc(i, TopMenu, UI, ScrX, ScrY, ScrW, ScrH))
				continue;
	}
	
	/* Draw Mouse */
	if (DrewAMenu)
		CONL_DrawMouse();
}

/* M_ExPopMenu() -- Pops menu from stack */
int32_t M_ExPopMenu(const uint8_t a_Player)
{
	M_UIMenuHandler_t* Handler;
	
	/* Check */
	if (a_Player < 0 || a_Player >= MAXSPLITSCREEN)
		return 0;
		
	/* No menus for player? */
	if (l_NumUIMenus[a_Player] <= 0)
		return 0;
	
	/* Get top item */
	Handler = l_UIMenus[a_Player][l_NumUIMenus[a_Player] - 1];
	
	// Call cleaners
	if (Handler->UIMenu->CleanerFunc)
		Handler->UIMenu->CleanerFunc(a_Player, Handler, Handler->UIMenu);
	
	// Delete private data
	if (Handler->PrivateData)
		Z_Free(Handler->PrivateData);
	
	// Free handler
	Z_Free(Handler);
	
	/* Downsize array */
	l_UIMenus[a_Player][l_NumUIMenus[a_Player] - 1] = NULL;
	Z_ResizeArray((void**)&l_UIMenus[a_Player], sizeof(*l_UIMenus[a_Player]), l_NumUIMenus[a_Player], l_NumUIMenus[a_Player] - 1);
	l_NumUIMenus[a_Player]--;
	
	/* Return amount of menus left */
	return l_NumUIMenus[a_Player];
}

/* M_ExPopAllMenus() -- Pops all menus for a player */
void M_ExPopAllMenus(const uint8_t a_Player)
{
	while (M_ExPopMenu(a_Player))
		;
}

/* M_ExFirstMenuSpot() -- Finds first spot containing menu */
int32_t M_ExFirstMenuSpot(const uint8_t a_Player, M_UIMenu_t* const a_UIMenu)
{
	int32_t i;
	
	/* Check */
	if (a_Player < 0 || a_Player >= MAXSPLITSCREEN)
		return -1;
	
	/* Go through menus */
	for (i = 0; i < l_NumUIMenus[a_Player]; i++)
		if (l_UIMenus[a_Player][i]->UIMenu == a_UIMenu)
			return i;
	
	/* Not Found */
	return -1;
}

/* M_ExPushMenu() -- Pushes menu to handle stack */
M_UIMenuHandler_t* M_ExPushMenu(const uint8_t a_Player, M_UIMenu_t* const a_UI)
{
	M_UIMenuHandler_t* New;
	bool_t NoMenu;
	
	/* Check */
	if (!a_UI || a_Player < 0 || a_Player >= MAXSPLITSCREEN)
		return NULL;
	
	/* Player not active? */
	if (a_Player != 0 && !D_ScrSplitHasPlayer(a_Player))
		return NULL;
	
	/* No menu active? */
	NoMenu = !l_NumUIMenus[a_Player];
		
	/* Allocate */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Setup */
	New->UIMenu = a_UI;
	New->ScreenID = a_Player;
	
	/* Don't park on unparkable items */
	while (New->CurItem < a_UI->NumItems && (a_UI->Items[New->CurItem].Flags & MUIIF_NOPARK))
		New->CurItem++;
	
	/* Add to end of stack */
	Z_ResizeArray((void**)&l_UIMenus[a_Player], sizeof(*l_UIMenus[a_Player]), l_NumUIMenus[a_Player], l_NumUIMenus[a_Player] + 1);
	l_UIMenus[a_Player][l_NumUIMenus[a_Player]++] = New;
	
	/* Initialize */
	// Allocate private area?
	if (New->UIMenu->PrivateSize)
		New->PrivateData = Z_Malloc(New->UIMenu->PrivateSize, PU_STATIC, NULL);
	
	// Call Initializer
	if (New->UIMenu->InitFunc)
		New->UIMenu->InitFunc(a_Player, New, New->UIMenu);
	
	/* Play Sound */
	if (NoMenu)
		S_StartSound(NULL, sfx_generic_switchon);
	else
		S_StartSound(NULL, sfx_generic_menupress);
}

/* M_GenericCleanerFunc() -- Generic Menu Cleaner */
void M_GenericCleanerFunc(const int32_t a_Player, struct M_UIMenuHandler_s* const a_Handler, struct M_UIMenu_s* const a_UIMenu)
{
	Z_Free(a_Handler->UIMenu->Items);
	Z_Free(a_Handler->UIMenu);
}

/* M_ExUIMessageBox() -- Creates a new message box */
bool_t M_ExUIMessageBox(const M_ExMBType_t a_Type, const uint32_t a_MessageID, const char* const a_Title, const char* const a_Message, const MBCallBackFunc_t a_CallBack)
{
#define UISPACING 12
#define UIMIDSPACE (UISPACING >> 1)
	M_UILocalBox_t* New;
	int32_t tW, tH, mW, mH, bW, bX, bY;
	size_t i, j;
	
	static const struct
	{
		uint32_t Bit;							// Bit
		uint32_t String;						// String
	} c_ButFlags[] =
	{
		{MEXMBT_DONTCARE, DSTR_UIGENERAL_DONTCARE},
		{MEXMBT_YES, DSTR_UIGENERAL_YES},
		{MEXMBT_NO, DSTR_UIGENERAL_NO},
		{0},
	};
	
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
	for (j = 0; c_ButFlags[j].Bit; j++)
		if ((a_Type & c_ButFlags[j].Bit) == c_ButFlags[j].Bit)
		{
			// Get last button
			i = New->NumButtons++;
			New->Buttons[i].Response = c_ButFlags[j].Bit;
			New->Buttons[i].Text = DS_GetString(c_ButFlags[j].String);
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
			// Up?
			if (!a_Event->Data.Keyboard.Down)
				return true;
			
			// Enter is hit
			if (a_Event->Data.Keyboard.KeyCode == IKBK_RETURN)
			{
				// Get current time
				ThisTime = I_GetTimeMS();
				
				// Don't press enter too soon
				if (ThisTime > Box->AppearTime + 1000)
				{
					// Make noise
					S_StartSound(NULL, sfx_generic_menupress);
							
					// End
					DidAClick = true;
				}
				
				DidSomething = true;
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
					S_StartSound(NULL, sfx_generic_menumove);
					
					// End
				}
				
				DidSomething = true;
			}
			
			// Always did something, for the keyboard
			DidSomething = true;
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
							S_StartSound(NULL, sfx_generic_menumove);
							
							// End
							DidSomething = true;
							break;
						}
						
						// Mouse is down over THIS button
						else if (a_Event->Data.Mouse.Down && b == Box->SelButton)
						{
							// Make noise
							S_StartSound(NULL, sfx_generic_menupress);
							
							// End
							DidSomething = true;
							DidAClick = true;
							break;
						}
					}
		}
		
		// Did something?
		if (DidSomething || DidAClick)
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
	
	/* Un-Handled Possibly? */
	return M_ExMenuHandleEvent(a_Event);
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
			VHW_HUDBlurBack(
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
						if (g_ProgramTic & 0x8)
							a_Flags = VFO_COLOR(VEX_MAP_BLACK);
						else
							a_Flags = VFO_COLOR(VEX_MAP_WHITE);
					}
					else
					{
						if (g_ProgramTic & 0x8)
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
	
	/* Draw Menus? */
	else
		M_ExMenuDrawer();
}

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
static void MS_SMTBFDDraw(struct M_SWidget_s* const a_Widget)
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
static bool_t MS_SMTBFEvent(struct M_SWidget_s* const a_Widget, const I_EventEx_t* const a_Event)
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
static void MS_SMTBFSelect(struct M_SWidget_s* const a_Widget)
{
	/* Steal input if not selected */
	if (!a_Widget->Data.TextBox.StealInput)
		a_Widget->Data.TextBox.StealInput = true;
}

/* MS_SMTBFTicker() -- Text Box Ticker */
static void MS_SMTBFTicker(struct M_SWidget_s* const a_Widget)
{
	/* Change of stolen input? */
	if (a_Widget->Data.TextBox.StealInput != a_Widget->Data.TextBox.OldSteal)
	{
		// Set
		a_Widget->Data.TextBox.OldSteal = a_Widget->Data.TextBox.StealInput;
	}
}

/* MS_SMTBFDestroy() -- Box destroyed */
static void MS_SMTBFDestroy(struct M_SWidget_s* const a_Widget)
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
void MS_Label_DDraw(struct M_SWidget_s* const a_Widget)
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
static bool_t MS_PossSlideFLeftRight(struct M_SWidget_s* const a_Widget, const int32_t a_Right)
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
void MS_Image_DDraw(struct M_SWidget_s* const a_Widget)
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
				D_XNetClearChat(Screen);
			
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
void M_SMSpawn(const int32_t a_ScreenID, const M_SMMenus_t a_MenuID)
{
#define SUBMENUFLAGS (VFO_COLOR(VEX_MAP_BRIGHTWHITE))
#define SORTFLAGS (VFO_COLOR(VEX_MAP_ORANGE))
#define VALUEFLAGS (VFO_COLOR(VEX_MAP_BRIGHTWHITE))
	M_SWidget_t* Root, *Work;
	CONL_VarPossibleValue_t* Possible;
	int32_t i;
	
	/* Check */
	if (a_ScreenID < 0 || a_ScreenID >= MAXSPLITSCREEN || a_MenuID < 0 || a_MenuID >= NUMMSMMENUS)
		return;
	
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
#undef SUBMENUFLAGS
#undef SORTFLAGS
}

