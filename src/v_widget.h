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
// Copyright (C) 2012-2012 GhostlyDeath <ghostlydeath@gmail.com>
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
// DESCRIPTION: Widget Code

#ifndef __V_WIDGET_H__
#define __V_WIDGET_H__

/***************
*** INCLUDES ***
***************/

#include "z_zone.h"
#include "w_wad.h"
#include "d_rmod.h"
#include "v_video.h"

/*****************
*** STRUCTURES ***
*****************/

typedef struct V_Widget_s V_Widget_t;

/****************
*** FUNCTIONS ***
****************/

/* RMOD */
bool_t V_WidgetRMODHandle(Z_Table_t* const a_Table, const WL_WADFile_t* const a_WAD, const D_RMODPrivates_t a_ID, D_RMODPrivate_t* const a_Private);
bool_t V_WidgetRMODOrder(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD, const D_RMODPrivates_t a_ID);

/* Widgets */
// Initial Stuff
bool_t V_InitWidgetSystem(void);

// Creation
V_Widget_t* V_WidgetCreate(V_Widget_t* const a_Parent, const char* const a_Type, const char* const a_ID);
void V_WidgetDestroy(V_Widget_t* const a_Widget);

// Drawing
void V_WidgetDraw(V_Widget_t* const a_Widget, const uint32_t a_Flags);

// Properties
bool_t V_WidgetSetValue(V_Widget_t* const a_Widget, const char* const a_Value);
bool_t V_WidgetSetSize(V_Widget_t* const a_Widget, const int32_t a_Width, const int32_t a_Height);
bool_t V_WidgetSetPosition(V_Widget_t* const a_Widget, const int32_t a_X, const int32_t a_Y);

bool_t V_WidgetSetPropertyStr(V_Widget_t* const a_Widget, const char* const a_Key, const char* const a_Value);
bool_t V_WidgetSetPropertyInt(V_Widget_t* const a_Widget, const char* const a_Key, const int32_t a_Value);

bool_t V_WidgetGetPropertyStr(V_Widget_t* const a_Widget, const char* const a_Key, char* const a_Dest, const size_t a_Size);
int32_t V_WidgetGetPropertyInt(V_Widget_t* const a_Widget, const char* const a_Key);

/******************************************************************************
*******************************************************************************
******************************************************************************/

/*****************
*** STRUCTURES ***
*****************/

typedef struct V_WidgetEx_s V_WidgetEx_t;

/* V_WidgetEx_t -- Extended widget */
struct V_WidgetEx_s
{
	/* Information */
	V_WidgetEx_t* wParent;						// Parent of widget
	size_t wNumKids;							// Number of kids
	V_WidgetEx_t** wKids;						// List of kids
	int32_t wPos[2];							// Widget position (320x200)
	int32_t wSize[2];							// Widget size (320x200)
	VEX_ColorList_t wColor[2];					// Widget Color [FG, BG]
	VEX_TransparencyList_t wTrans[2];			// Widget Transparency [FG, BG]
	VEX_ColorList_t wBorderColor;				// Widget Border Color
	VEX_TransparencyList_t wBorderTrans;		// Widget Border Transparency
	uint8_t wBorderSize;						// Size of border (in 320x200 pixels)
	VideoFont_t wFont;							// Widget Font
	
	/* Handler Functions */
		// fDelete() -- Delete widget
	void (*fDelete)(V_WidgetEx_t* const a_This);
		// fDraw() -- Draw widget
	void (*fDraw)(V_WidgetEx_t* const a_This);
		// fAddKid() -- Add kid widget
	void (*fAddKid)(V_WidgetEx_t* const a_This, V_WidgetEx_t* const a_Kid);
		// fDelKid() -- Delete kid widget
	void (*fDelKid)(V_WidgetEx_t* const a_This, V_WidgetEx_t* const a_Kid);
		// fSetSize() -- Set size of widget
	void (*fSetSize)(V_WidgetEx_t* const a_This, const int32_t a_W, const int32_t a_H);
		// fSetPos() -- Set position of widget
	void (*fSetPos)(V_WidgetEx_t* const a_This, const int32_t a_X, const int32_t a_Y);
};

/****************
*** FUNCTIONS ***
****************/

V_WidgetEx_t* V_WidgetExNewLabel(const VideoFont_t a_Font, const char* const a_Text);

#endif /* __V_WIDGET_H__ */

