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
// DESCRIPTION: Menu widget stuff, episode selection and such.

#ifndef __M_MENU__
#define __M_MENU__

#include "d_event.h"
#include "command.h"
#include "d_prof.h"

#include "w_wad.h"
#include "z_zone.h"

#include "d_rmod.h"

#include "i_util.h"

/*******************************************************************************
********************************************************************************
*******************************************************************************/

/*** CONSTANTS ***/

/* M_ExMBType_t -- Message Box Type */
typedef enum M_ExMBType_e
{
	MEXMBT_OK						= 0x0001,	// OK
	MEXMBT_YES						= 0x0002,	// Yes
	MEXMBT_NO						= 0x0004,	// No
	MEXMBT_CANCEL					= 0x0008,	// Cancel
	MEXMBT_DONTCARE					= 0x0010,	// Don't Care
} M_ExMBType_t;

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
	MUIIF_DISABLED		= UINT32_C(0x0000002),	// Disabled
} M_UIItemFlags_t;

/*** STRUCTURES ***/

typedef void (*MBCallBackFunc_t)(const uint32_t a_MessageID, const M_ExMBType_t a_Response, const char** const a_TitleP, const char** const a_MessageP);

struct M_UIMenu_s;
struct M_UIItem_s;

typedef bool_t (*M_UIItemLRValChangeFuncType_t)(const int32_t a_PlayerID, struct M_UIMenu_s* const a_Menu, struct M_UIItem_s* const a_Item, const bool_t a_More);
typedef bool_t (*M_UIItemPressFuncType_t)(const int32_t a_PlayerID, struct M_UIMenu_s* const a_Menu, struct M_UIItem_s* const a_Item);

/* M_UIItem_t -- Menu Item */
typedef struct M_UIItem_s
{
	const char* ClassName;						// Class Name for Item
	struct M_UIMenu_s* Menu;					// Menu Owner
	uint32_t Flags;								// Flags
	M_UIItemType_t Type;						// Type of item
	const char* Text;							// Item Text
	const char* Value;							// Value
	const char** TextRef;						// Item Text (i18n)
	const char** ValueRef;						// Value (i18n)
	
	const char* SubVal;							// SubValue
	
	intptr_t DataBits;							// Anything needed for data
	M_UIItemLRValChangeFuncType_t LRValChangeFunc;
	M_UIItemPressFuncType_t ItemPressFunc;
} M_UIItem_t;

struct M_UIMenuHandler_s;
struct V_Image_s;

/* M_UIMenu_t -- Interface Menu */
typedef struct M_UIMenu_s
{
	uint8_t Junk;								// Junk
	const char* ClassName;						// Menu Class Name
	
	struct V_Image_s* TitlePic;					// Title Picture
	const char* Title;							// Menu Title
	const char** TitleRef;						// Title Reference
	
	M_UIItem_t* Items;							// Items
	size_t NumItems;							// Number of Items
	
	void (*InitFunc)(const int32_t a_Player, struct M_UIMenuHandler_s* const a_Handler, struct M_UIMenu_s* const a_UIMenu);
	void (*CleanerFunc)(const int32_t a_Player, struct M_UIMenuHandler_s* const a_Handler, struct M_UIMenu_s* const a_UIMenu);
	bool_t (*UnderDrawFunc)(const int32_t a_Player, struct M_UIMenuHandler_s* const a_Handler, struct M_UIMenu_s* const a_Menu, const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H);
	bool_t (*OverDrawFunc)(const int32_t a_Player, struct M_UIMenuHandler_s* const a_Handler, struct M_UIMenu_s* const a_Menu, const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H);
	
	size_t PrivateSize;							// Size of private data
} M_UIMenu_t;

/* M_UIMenuHandler_t -- Menu Handler */
typedef struct M_UIMenuHandler_s
{
	const M_UIMenu_t* UIMenu;					// Defined UI Menu
	int32_t CurItem;							// Current Selected Item
	int32_t OldCurItem;							// Old Selected Item
	int32_t StartOff;							// Starting Offset
	int32_t IPS;								// Known IPS
	int32_t OSKWait;							// OSK Wait
	int32_t ScreenID;							// Screen
	void* PrivateData;							// Private Menu Data
} M_UIMenuHandler_t;

/*** GLOBALS ***/
extern int32_t g_ResumeMenu;					// Resume menu for n tics

/*** FUNCTIONS ***/

void M_MenuExInit(void);

bool_t M_ExAllUIActive(void);
bool_t M_ExPlayerUIActive(const uint32_t a_Player);
bool_t M_ExUIActive(void);

bool_t M_ExUIMessageBox(const M_ExMBType_t a_Type, const uint32_t a_MessageID, const char* const a_Title, const char* const a_Message, const MBCallBackFunc_t a_CallBack);

bool_t M_ExUIHandleEvent(const I_EventEx_t* const a_Event);
void M_ExUIDrawer(void);

M_UIMenuHandler_t* M_ExPushMenu(const uint8_t a_Player, M_UIMenu_t* const a_UI);
int32_t M_ExPopMenu(const uint8_t a_Player);
void M_ExPopAllMenus(const uint8_t a_Player);
int32_t M_ExFirstMenuSpot(const uint8_t a_Player, M_UIMenu_t* const a_UIMenu);

void M_GenericCleanerFunc(const int32_t a_Player, struct M_UIMenuHandler_s* const a_Handler, struct M_UIMenu_s* const a_UIMenu);

int32_t M_ExMenuIDByName(const char* const a_Name);
M_UIMenu_t* M_ExMakeMenu(const int32_t a_MenuID, void* const a_Data);

int CLC_ExMakeMenuCom(const uint32_t a_ArgC, const char** const a_ArgV);
M_UIMenu_t* M_ExTemplateMakeGameVars(const int32_t a_Mode);

#endif

