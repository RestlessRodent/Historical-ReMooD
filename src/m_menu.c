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
// Copyright (C) 2012 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Menu Code

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
#include "p_demcmp.h"

#include "r_data.h"

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
		V_DrawFadeConsBackEx(
				VEX_COLORMAP(VEX_MAP_BLACK),
				ScrX, ScrY, ScrX + ScrW, ScrY + ScrH
			);
		
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
			if (Item->ValueRef)
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
			if (Item->Value)
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


