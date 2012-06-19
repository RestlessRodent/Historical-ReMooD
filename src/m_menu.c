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
#include "p_demcmp.h"

#include "r_data.h"

/*******************************************************************************
********************************************************************************
*******************************************************************************/

/*** CONSTANTS ***/

#define MAXUIBUTTONS 32

/* M_UIItemType_t -- Type of UI Item */
typedef enum M_UIItemType_e
{
	MUIIT_NORMAL,								// Normal Item
	MUIIT_HEADER,								// Header Item	
	
	NUMUIITEMTYPES
} M_UIItemType_t;

/* M_UIItemFlags_t -- Item Flags */
typedef enum M_UIItemFlags_e
{
	MUIIF_NOPARK		= UINT32_C(0x0000001),	// No parking
} M_UIItemFlags_t;

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

struct M_UIMenu_s;

/* M_UIItem_t -- Menu Item */
typedef struct M_UIItem_s
{
	struct M_UIMenu_s* Menu;					// Menu Owner
	uint32_t Flags;								// Flags
	M_UIItemType_t Type;						// Type of item
	const char* Text;							// Item Text
	const char* Value;							// Value
	const char** TextRef;						// Item Text (i18n)
	const char** ValueRef;						// Value (i18n)
	
	int32_t DataBits;							// Anything needed for data
	bool_t (*LRValChangeFunc)(struct M_UIMenu_s* const a_Menu, struct M_UIItem_s* const a_Item, const bool_t a_More);
} M_UIItem_t;

struct M_UIMenuHandler_s;

/* M_UIMenu_t -- Interface Menu */
typedef struct M_UIMenu_s
{
	uint8_t Junk;								// Junk
	
	M_UIItem_t* Items;							// Items
	size_t NumItems;							// Number of Items
	
	void (*CleanerFunc)(struct M_UIMenuHandler_s* const a_Handler, struct M_UIMenu_s* const a_UIMenu);
	bool_t (*UnderDrawFunc)(struct M_UIMenuHandler_s* const a_Handler, struct M_UIMenu_s* const a_Menu, const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H);
	bool_t (*OverDrawFunc)(struct M_UIMenuHandler_s* const a_Handler, struct M_UIMenu_s* const a_Menu, const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H);
} M_UIMenu_t;

/* M_UIMenuHandler_t -- Menu Handler */
typedef struct M_UIMenuHandler_s
{
	M_UIMenu_t* UIMenu;							// Defined UI Menu
	int32_t CurItem;							// Current Selected Item
	int32_t OldCurItem;							// Old Selected Item
	int32_t StartOff;							// Starting Offset
	int32_t IPS;								// Known IPS
	int32_t OSKWait;							// OSK Wait
} M_UIMenuHandler_t;

#define MUIBOXFONT VFONT_SMALL

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

M_UIMenuHandler_t* M_ExPushMenu(const uint8_t a_Player, M_UIMenu_t* const a_UI);

/*** MENU FUNCTIONS ***/

/* MS_TextureTestUnderDraw() -- Menu Under Drawer */
static bool_t MS_TextureTestUnderDraw(struct M_UIMenuHandler_s* const a_Handler, struct M_UIMenu_s* const a_Menu, const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H)
{
#define BUFSIZE 32
	texture_t* Texture;
	char Buf[BUFSIZE];
	int32_t i;
	
	/* Check */
	if (!a_Menu)
		return;
	
	/* Get Texture Reference */
	Texture = textures[a_Menu->Items[a_Handler->CurItem].DataBits];
	
	/* Draw Texture */
	// Flat
	if (Texture->IsFlat)
	{
		if (R_GetFlat(a_Menu->Items[a_Handler->CurItem].DataBits))
			V_ImageDraw(0, Texture->FlatImage, a_X + (a_W >> 1), a_Y + ((a_H - Texture->height) - 5), NULL);
	}
	
	// Wall
	else
	{
	}
	
	/* Draw Info */
	// Type
	V_DrawStringA(VFONT_SMALL, VFO_COLOR(VEX_MAP_WHITE), (Texture->IsFlat ? "Flat" : "Wall"), a_X + (a_W >> 1), a_Y + 10);
	
	// Other Stuff
	for (i = 0; i <= 7; i++)
	{
		// Clear
		memset(Buf, 0, sizeof(Buf));
		
		// Set
		switch (i)
		{
			case 1:
				snprintf(Buf, BUFSIZE - 1, "Cb: %u", Texture->CombinedOrder);
				break;
			
			case 2:
				snprintf(Buf, BUFSIZE - 1, "Io: %u", Texture->InternalOrder);
				break;
			
			case 3:
				snprintf(Buf, BUFSIZE - 1, "w&: %x", Texture->WidthMask);
				break;
				
			case 4:
				snprintf(Buf, BUFSIZE - 1, "tr: %i", Texture->Translation);
				break;
				
			case 5:
				snprintf(Buf, BUFSIZE - 1, "cs: %u", Texture->CacheSize);
				break;
				
			case 6:
				snprintf(Buf, BUFSIZE - 1, "om: %u", Texture->OrderMul);
				break;
			
			case 7:
				if (!Texture->FlatEntry)
					snprintf(Buf, BUFSIZE - 1, "PW: ---");
				else
					snprintf(Buf, BUFSIZE - 1, "PW: %s", (((const WL_WADEntry_t*)Texture->FlatEntry)->Owner->__Private.__DOSName));
				break;
				
			case 0:
			default:
				snprintf(Buf, BUFSIZE - 1, "Dx: %ix%i", Texture->width, Texture->height);
				break;
		}
		
		// Draw
		V_DrawStringA(VFONT_SMALL, VFO_COLOR(VEX_MAP_WHITE), Buf, a_X + (a_W >> 1), a_Y + (10 * (i + 2)));
	}
	
	/* Keep Drawing */
	return true;
#undef BUFSIZE
}

/* MS_ExGeneralComm() -- Menu Commands */
static CONL_ExitCode_t MS_ExGeneralComm(const uint32_t a_ArgC, const char** const a_ArgV)
{
	M_UIMenu_t* New;
	int32_t i;
	
	static const char* const c_ColorTestStrs[33] =
	{
		"{zDefault (z)",
		
		"{0Default (0)",
		"{1Red (1)",
		"{2Orange (2)",
		"{3Yellow (3)",
		"{4Green (4)",
		"{5Cyan (5)",
		"{6Blue (6)",
		"{7Magenta (7)",
		"{8Brown (8)",
		"{9Bright White (9)",
		"{aWhite (a)",
		"{bGray (b)",
		"{cBlack (c)",
		"{dFuscia (d)",
		"{eGold (e)",
		"{fTek Green (f)",
		
		"{x70Green (x70)",
		"{x71Gray (x71)",
		"{x72Brown (x72)",
		"{x73Red (x73)",
		"{x74Light Gray (x74)",
		"{x75Light Brown (x75)",
		"{x76Light Red (x76)",
		"{x77Light Blue (x77)",
		"{x78Blue (x78)",
		"{x79Yellow (x79)",
		"{x7aBeige (x7a)",
		"{x7bWhite (x7b)",
		"{x7cOrange (x7c)",
		"{x7dTan (x7d)",
		"{x7eBlack (x7e)",
		"{x7fPink (x7f)",
	};
	
	/* Texture Test */
	if (strcasecmp(a_ArgV[0], "menutexturetest") == 0)
	{
		// Allocate
		New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
		// Quick
		New->NumItems = numtextures;
		New->Items = Z_Malloc(sizeof(*New->Items) * New->NumItems, PU_STATIC, NULL);
		New->CleanerFunc = NULL;
		New->UnderDrawFunc = MS_TextureTestUnderDraw;
		
		// Set Strings
		for (i = 0; i < numtextures; i++)
		{
			New->Items[i].Menu = New;
			New->Items[i].Text = textures[i]->name;
			New->Items[i].DataBits = i;
		}
		
		M_ExPushMenu(0, New);
	}
	
	/* Color Test */
	else if (strcasecmp(a_ArgV[0], "menucolortest") == 0)
	{
		// Allocate
		New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
		// Quick
		New->NumItems = 33;
		New->Items = Z_Malloc(sizeof(*New->Items) * New->NumItems, PU_STATIC, NULL);
		New->CleanerFunc = NULL;
		
		// Set Strings
		for (i = 0; i < 33; i++)
		{
			New->Items[i].Menu = New;
			New->Items[i].Text = c_ColorTestStrs[i];
		}
		
		M_ExPushMenu(0, New);
	}
}

/* M_MenuExInit() -- Init Menu */
void M_MenuExInit(void)
{
	CONL_VarRegister(&l_MenuFont);
	CONL_VarRegister(&l_MenuCompact);
	CONL_VarRegister(&l_MenuHeaderColor);
	CONL_VarRegister(&l_MenuItemColor);
	CONL_VarRegister(&l_MenuValColor);
	
	CONL_AddCommand("menucolortest", MS_ExGeneralComm);
	CONL_AddCommand("menutexturetest", MS_ExGeneralComm);
}

/* M_ExMenuHandleEvent() -- Handles Menu Events */
bool_t M_ExMenuHandleEvent(const I_EventEx_t* const a_Event)
{
	int32_t i, RowPos, RowEnd, DoDown, DoRight, z;
	M_UIMenuHandler_t* TopMenu;
	M_UIMenu_t* UI;
	bool_t Up, DoCancel;
	
	/* Control for each player */
	for (i = 0; i < (g_SplitScreen > 0 ? g_SplitScreen + 1 : 1); i++)
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
		DoCancel = false;
		
		// Which key command?
		if (a_Event->Type == IET_SYNTHOSK)
		{
			// Not this player?
			if (a_Event->Data.SynthOSK.PNum != i)
				continue;
			
			// Waiting on OSK?
			if (gametic < TopMenu->OSKWait)
				continue;
			
			// Move menu up/down?
			if (a_Event->Data.SynthOSK.Down != 0)
				DoDown = a_Event->Data.SynthOSK.Down;
			
			// Change value left/right?
			else if (a_Event->Data.SynthOSK.Right != 0)
				DoRight = a_Event->Data.SynthOSK.Right;
			
			// Un-handled
			else
				continue;
			
			// Handled somewhere
			TopMenu->OSKWait = gametic + (TICRATE >> 2);
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
			M_ExPopMenu(i);
			return true;	// always handled
		}
		
		// Up/Down?
		if (DoDown != 0)
		{
			TopMenu->CurItem += DoDown;
			Up = DoDown > 0;
		}
		
		// Left/Right?
		if (DoRight != 0)
			if (UI->Items[TopMenu->CurItem].LRValChangeFunc)
				if (UI->Items[TopMenu->CurItem].LRValChangeFunc(UI, &UI->Items[TopMenu->CurItem], DoRight > 0))
					S_StartSound(NULL, sfx_stnmov);
		
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
			S_StartSound(NULL, sfx_pstop);
		
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
		
		// Was handled
		return true;
	}
	
	/* Un-Handled */
	return false;
}

/*** MB FUNCTIONS ***/

/* M_ExUIActive() -- Returns true if UI is active */
bool_t M_ExUIActive(void)
{
	if (l_NumUIBoxes)
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
	
	/* Draw for each player */
	for (i = 0; i < (g_SplitScreen > 0 ? g_SplitScreen + 1 : 1); i++)
	{
		// No menus for this player?
		if (!l_NumUIMenus[i])
			continue;
		
		// Get base screen size
		ScrX = 0;
		ScrY = 0;
		ScrW = 320;
		ScrH = 200;
		
		// 2+ Split = Half Height
		if (g_SplitScreen > 0)
			ScrH >>= 1;
		
		// 3+ Split = Half Width
		if (g_SplitScreen > 1)
			ScrW >>= 1;
		
		// Modify h/w?
			// 2 player
		if (g_SplitScreen == 1)
			ScrY = ScrH * (i & 1);
		
			// 3/4 player
		else if (g_SplitScreen > 1)
		{
			ScrX = ScrW * (i & 1);
			ScrY = ScrH * ((i >> 1) & 1);
		}
		
		// Get top menu
		TopMenu = l_UIMenus[i][l_NumUIMenus[i] - 1];
		
		// Menu Here?
		if (!TopMenu)
			continue;
		
		// Get UI
		UI = TopMenu->UIMenu;
		
		// Draw underneath
		if (UI->UnderDrawFunc)
			if (!UI->UnderDrawFunc(TopMenu, UI, ScrX, ScrY, ScrW, ScrH))
				continue;
		
		// Draw Title
		V_DrawStringA(VFONT_LARGE, 0, "Title", 0, 2);
		
		// Base Position
		x = ScrX + 10;
		y = ScrY + V_FontHeight(VFONT_LARGE) + 4;
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
				ValStr = *Item->ValueRef;
			else
				ValStr = Item->Value;
			
			// Draw Cursor
			if (j == TopMenu->CurItem)
				if (gametic & 0x4)
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
				// Selected (Show indicator)
				if (j == TopMenu->CurItem)
				{
					if (gametic & 0x8)
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
			if (!UI->OverDrawFunc(TopMenu, UI, ScrX, ScrY, ScrW, ScrH))
				continue;
	}
}

/* M_ExPopMenu() -- Pops menu from stack */
void M_ExPopMenu(const uint8_t a_Player)
{
	M_UIMenuHandler_t* Handler;
	
	/* Check */
	if (a_Player < 0 || a_Player >= MAXSPLITSCREEN)
		return;
		
	/* No menus for player? */
	if (l_NumUIMenus[a_Player] <= 0)
		return;
	
	/* Get top item */
	Handler = l_UIMenus[a_Player][l_NumUIMenus[a_Player] - 1];
	
	// Call cleaners
	if (Handler->UIMenu->CleanerFunc)
		Handler->UIMenu->CleanerFunc(Handler, Handler->UIMenu);
	
	// Free handler
	Z_Free(Handler);
	
	/* Downsize array */
	l_UIMenus[a_Player][l_NumUIMenus[a_Player] - 1] = NULL;
	Z_ResizeArray((void**)&l_UIMenus[a_Player], sizeof(*l_UIMenus[a_Player]), l_NumUIMenus[a_Player], l_NumUIMenus[a_Player] - 1);
	l_NumUIMenus[a_Player]--;
}

/* M_ExPushMenu() -- Pushes menu to handle stack */
M_UIMenuHandler_t* M_ExPushMenu(const uint8_t a_Player, M_UIMenu_t* const a_UI)
{
	M_UIMenuHandler_t* New;
	
	/* Check */
	if (!a_UI || a_Player < 0 || a_Player >= MAXSPLITSCREEN)
		return NULL;
		
	/* Allocate */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Setup */
	New->UIMenu = a_UI;
	
	/* Don't park on unparkable items */
	while (New->CurItem < a_UI->NumItems && (a_UI->Items[New->CurItem].Flags & MUIIF_NOPARK))
		New->CurItem++;
	
	/* Add to end of stack */
	Z_ResizeArray((void**)&l_UIMenus[a_Player], sizeof(*l_UIMenus[a_Player]), l_NumUIMenus[a_Player], l_NumUIMenus[a_Player] + 1);
	l_UIMenus[a_Player][l_NumUIMenus[a_Player]++] = New;
}

/* MS_GenericCleanerFunc() -- Generic Menu Cleaner */
static void MS_GenericCleanerFunc(struct M_UIMenuHandler_s* const a_Handler, struct M_UIMenu_s* const a_UIMenu)
{
	Z_Free(a_Handler->UIMenu->Items);
	Z_Free(a_Handler->UIMenu);
}

/* MS_GameVar_LRValChange() -- Game variable value changer callback */
static bool_t MS_GameVar_LRValChange(struct M_UIMenu_s* const a_Menu, struct M_UIItem_s* const a_Item, const bool_t a_More)
{
	P_EXGSVariable_t* Bit;
	int32_t OldVal, NewVal, ModVal;
	
	/* Get bit */
	Bit = P_EXGSVarForBit(a_Item->DataBits);
	
	// Failed?
	if (!Bit)
		return false;
		
	/* Value change amount? */
	ModVal = P_EXGSGetNextValue(Bit->BitID, a_More);
	
	/* Get values and attempt change */
	OldVal = P_EXGSGetValue(Bit->BitID);
	NewVal = P_EXGSSetValue(false, Bit->BitID, ModVal);
	
	// Success? Only if actually changed
	if (NewVal != OldVal)
		return true;
	return false;
}

/* M_ExTemplateMakeGameVars() -- Make Game Variable Template */
M_UIMenu_t* M_ExTemplateMakeGameVars(const int32_t a_Mode)
{
	M_UIMenu_t* New;
	int32_t i, j, k, b, w, z;
	P_EXGSVariable_t* Bit;
	P_EXGSVariable_t** SortedBits;
	P_EXGSMenuCategory_t LastCat;
	
	/* Sort bits */
	SortedBits = Z_Malloc(sizeof(*SortedBits) * (PEXGSNUMBITIDS - 1), PU_STATIC, NULL);
	
	// Init
	for (i = 1; i < PEXGSNUMBITIDS; i++)
		SortedBits[i - 1] = P_EXGSVarForBit(i);
	
	// Sort by category first
	for (i = 0; i < (PEXGSNUMBITIDS - 1); i++)
	{
		// Initial
		b = i;
		
		// Find next lowest
		for (j = i + 1; j < (PEXGSNUMBITIDS - 1); j++)
			if (SortedBits[j]->Category < SortedBits[b]->Category)
				b = j;
		
		// Swap
		Bit = SortedBits[i];
		SortedBits[i] = SortedBits[b];
		SortedBits[b] = Bit;
	}
	
	// Sort by name now
	LastCat = 0;
	for (w = z = i = 0; i <= (PEXGSNUMBITIDS - 1); i++)
	{
		// Change of category? or ran out!
		if (i == (PEXGSNUMBITIDS - 1) || LastCat != SortedBits[i]->Category)
		{
			// Sort sub items
			for (j = w; j <= z; j++)
			{
				// Initial
				b = j;
		
				// Find next lowest
				for (k = j + 1; k <= z; k++)
					if (strcasecmp(SortedBits[k]->MenuTitle, SortedBits[b]->MenuTitle) < 0)
						b = k;
		
				// Swap
				Bit = SortedBits[j];
				SortedBits[j] = SortedBits[b];
				SortedBits[b] = Bit;
			}
			
			// Setup for future sort
			if (i < (PEXGSNUMBITIDS - 1))
				LastCat = SortedBits[i]->Category;
			w = z = i;
		}
		
		// Un-changed
		else
		{
			z++;	// Increase end spot
		}
	}
	
	/* Allocate */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Quick */
	New->NumItems = (PEXGSNUMBITIDS - 1) + NUMPEXGSMENUCATEGORIES;
	New->Items = Z_Malloc(sizeof(*New->Items) * New->NumItems, PU_STATIC, NULL);
	New->CleanerFunc = MS_GenericCleanerFunc;
	
	/* Hack Up Variables */
	LastCat = 0;
	for (j = 0, i = 0; i < New->NumItems; i++)
	{
		// Don't park by default
		New->Items[i].Flags |= MUIIF_NOPARK;
		New->Items[i].Menu = New;
		
		// Overflow?
		if (j >= (PEXGSNUMBITIDS - 1))
			continue;
		
		// Get Bit
		Bit = SortedBits[j++];
		
		// No Category?
		if (Bit->Category == PEXGSMC_NONE)
			continue;
		
		// Category change?
		if (LastCat != Bit->Category)
		{
			// We want to put the header for the next category always
			LastCat = Bit->Category;
			
			// Print Category Header
			New->Items[i].Type = MUIIT_HEADER;
			//New->Items[i].Text = "*** CATEGORY ***";
			New->Items[i].TextRef = PTROFUNICODESTRING((DSTR_MENUGAMEVAR_CATNONE + LastCat));
			
			// Write on next item
			LastCat = Bit->Category;
			i++;
		}
		
		// Set
		New->Items[i].Text = Bit->MenuTitle;
		New->Items[i].Value = Bit->StrVal;
		New->Items[i].Flags &= ~MUIIF_NOPARK;	// Park here
		New->Items[i].LRValChangeFunc = MS_GameVar_LRValChange;
		New->Items[i].DataBits = Bit->BitID;
	}
	
	/* Cleanup */
	Z_Free(SortedBits);
	
	/* Return */
	return New;
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
	
	/* Draw Menus? */
	else
		M_ExMenuDrawer();
}


