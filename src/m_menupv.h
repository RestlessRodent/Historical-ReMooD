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
// DESCRIPTION: Menu Private Stuff

#if 0

#ifndef __M_MENUPV_H__
#define __M_MENUPV_H__

/***************
*** INCLUDES ***
***************/

#include "d_prof.h"
#include "v_video.h"
#include "console.h"
#include "p_demcmp.h"

/****************
*** CONSTANTS ***
****************/

/* M_SWidFlags_t -- Widget flags */
typedef enum M_SWidFlags_e
{
	MSWF_NOSELECT		= UINT32_C(0x00000001),	// Cannot be selected
	MSWF_NOHANDLEEVT	= UINT32_C(0x00000002),	// Do not handle events
	MSWF_DISABLED		= UINT32_C(0x00000004),	// Disabled
} M_SWidFlags_t;

/* M_CreateGameOpts_e -- Create game options */
enum M_CreateGameOpts_e
{
	MCGO_NULL,
	
	MCGO_IWAD,
	
	NUMMCGO
};

#define MAXSERVERSONLIST 10

/*****************
*** STRUCTURES ***
*****************/

typedef struct M_SWidget_s M_SWidget_t;

/* M_SWidget_t -- Simple menu widget */
struct M_SWidget_s
{
	uint32_t Flags;								// Widget Flags
	int32_t Screen;								// Screen
	
	int32_t x, y;								// X/Y position
	int32_t w, h;								// Width/Height
	int32_t offx, offy;							// X/Y offset
	
	int32_t dx, dy;								// Draw X/Y
	int32_t dw, dh;								// Draw W/H
	
	M_SWidget_t* Parent;						// Parent Widget
	
	M_SWidget_t** Kids;							// Kid Widgets
	int32_t NumKids;							// Number of kids
	
	int32_t CursorOn;							// Curson on which kid?
	M_SMMenus_t SubMenu;						// SubMenu to open
	int32_t Option;								// Option
	D_Prof_t* Prof;								// Profile to modify
	
	// Drawing
	void (*DCursor)(M_SWidget_t* const, M_SWidget_t* const);
	void (*DDraw)(M_SWidget_t* const);
	
	// Functions
	void (*FDestroy)(M_SWidget_t* const);
	bool_t (*FEvent)(M_SWidget_t* const, const I_EventEx_t* const);
	bool_t (*FLeftRight)(M_SWidget_t* const, const int32_t);
	bool_t (*FUpDown)(M_SWidget_t* const, const int32_t);
	bool_t (*FSelect)(M_SWidget_t* const);
	bool_t (*FCancel)(M_SWidget_t* const);
	void (*FTicker)(M_SWidget_t* const);
	
	union
	{
		struct
		{
			V_Image_t* Pic;						// Picture to draw
		} Image;								// Simple Image
		
		struct
		{
			V_Image_t* Skulls[2];				// Skulls
		} MainMenu;								// Main Menu Stuff
		
		struct
		{
			VideoFont_t Font;					// Font to draw in
			uint32_t Flags;						// Draw flags
			const char** Ref;					// Reference to text
			const char** ValRef;				// Value Reference
			int32_t Pivot;						// Current pivot location
			CONL_StaticVar_t* CVar;				// Console Variables
			CONL_VarPossibleValue_t* Possible;	// Possible Values
			P_XGSVariable_t* Var;				// Variable
			P_XGSBitID_t NextVar;				// Next Variable
			uint32_t ValFlags;					// Value Draw Flags
		} Label;
		
		struct
		{
			VideoFont_t Font;					// Font to draw in
			uint32_t Flags;						// Draw flags
			CONCTI_Inputter_t* Inputter;		// Text Inputter
			bool_t StealInput;					// Input Stolen
			bool_t OldSteal;					// Old Steal Input
		} TextBox;
	} Data;										// Widget Data
};

/**************
*** GLOBALS ***
**************/

extern D_Prof_t* g_DoProf;

/****************
*** FUNCTIONS ***
****************/

void M_StackPop(const int32_t a_ScreenID);
void M_StackPopAllScreen(const int32_t a_Screen);
void M_StackPopAll(void);

int32_t M_HelpInitIWADList(CONL_VarPossibleValue_t** const a_PossibleOut);

void M_MainMenu_DCursor(M_SWidget_t* const a_Widget, M_SWidget_t* const a_Sub);
bool_t M_SubMenu_FSelect(M_SWidget_t* const a_Widget);

bool_t M_NewGameClassic_FSelect(M_SWidget_t* const a_Widget);
bool_t M_NewGameClassic_ServerFSelect(M_SWidget_t* const a_Widget);
bool_t M_NewGameClassic_ServerFLeftRight(M_SWidget_t* const a_Widget, const int32_t a_Right);
bool_t M_NewGameClassic_ServerFTicker(M_SWidget_t* const a_Widget);

bool_t M_NewGameEpi_FSelect(M_SWidget_t* const a_Widget);
bool_t M_NewGameSkill_FSelect(M_SWidget_t* const a_Widget);
bool_t M_QuitGame_DisconFSelect(M_SWidget_t* const a_Widget);
bool_t M_QuitGame_PDisconFSelect(M_SWidget_t* const a_Widget);
bool_t M_QuitGame_StopWatchFSelect(M_SWidget_t* const a_Widget);
bool_t M_QuitGame_StopRecordFSelect(M_SWidget_t* const a_Widget);
bool_t M_QuitGame_LogOffFSelect(M_SWidget_t* const a_Widget);
bool_t M_QuitGame_ExitFSelect(M_SWidget_t* const a_Widget);
void M_QuitGame_FTicker(M_SWidget_t* const a_Widget);
void M_ACG_CreateFSelect(M_SWidget_t* const a_Widget);
bool_t M_CTUS_BoxCallBack(struct CONCTI_Inputter_s*, const char* const);
void M_CTUS_ConnectFSelect(M_SWidget_t* const a_Widget);

void M_ProfMan_FTicker(M_SWidget_t* const a_Widget);
bool_t M_ProfMan_CreateProf(M_SWidget_t* const a_Widget);
bool_t M_ProfMan_IndvFSel(M_SWidget_t* const a_Widget);
bool_t M_ProfMan_AcctBCB(struct CONCTI_Inputter_s*, const char* const);

#endif /* __M_MENUPV_H__ */

#endif

