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
// Copyright (C) 2012 GhostlyDeath <ghostlydeath@gmail.com>
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

/***************
*** INCLUDES ***
***************/

#include "v_widget.h"
#include "doomstat.h"

/*****************
*** STRUCTURES ***
*****************/

typedef bool_t (*V_WidgetHandlerCreateFunc_t)(V_Widget_t* const a_Widget, const char* const a_Type);
typedef bool_t (*V_WidgetHandlerDeleteFunc_t)(V_Widget_t* const a_Widget);
typedef bool_t (*V_WidgetHandlerDrawFunc_t)(V_Widget_t* const a_Widget);
typedef bool_t (*V_WidgetHandlerSetDimensionFunc_t)(V_Widget_t* const a_Widget, const int32_t a_X, const int32_t a_Y, const int32_t a_Width, const int32_t a_Height);

/* V_WidgetHandler_t -- Handles a widget */
typedef struct V_WidgetHandler_s
{
} V_WidgetHandler_t;

/* V_Widget_s -- A GUI Widget */
struct V_Widget_s
{
	/* Ancestry */
	struct V_Widget_s* Parent;					// Parent Widget
	struct V_Widget_s** Children;				// Kid Widgets
	size_t NumChildren;							// Number of kids
	
	/* Value Stuff */
	void* ValueP;								// Pointer type value
	
	/* Drawing */
	int32_t XPos;								// X position
	int32_t YPos;								// Y position
	int32_t Width;								// Width of widget
	int32_t Height;								// Height of widget
	
	/* Handlers */
	const V_WidgetHandler_t* Handler;			// Handler to use
};

/****************
*** FUNCTIONS ***
****************/

/* V_WidgetRMODHandle() -- Handle RMOD Widget Data */
bool_t V_WidgetRMODHandle(Z_Table_t* const a_Table, const WL_WADFile_t* const a_WAD, const D_RMODPrivates_t a_ID, D_RMODPrivate_t* const a_Private)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return false;
	
	/*** STANDARD CLIENT ***/
#else

	/* Not for dedicated server */
	if (g_DedicatedServer)
		return false;	
	
	/* Check */
	if (!a_Table || !a_WAD || !a_ID || !a_Private)
		return false;
	
	/* Success! */
	return false;
#endif /* __REMOOD_DEDICATED */
}

/* V_WidgetRMODOrder() -- WAD order changed */
bool_t V_WidgetRMODOrder(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD, const D_RMODPrivates_t a_ID)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return false;
	
	/*** STANDARD CLIENT ***/
#else
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return false;	
	
	/* Success! */
	return false;
#endif /* __REMOOD_DEDICATED */
}

/* V_InitWidgetSystem() -- Initialize the widget subsystem */
bool_t V_InitWidgetSystem(void)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return false;
	
	/*** STANDARD CLIENT ***/
#else
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return false;	
	
	/* Success! */
	return false;
#endif /* __REMOOD_DEDICATED */
}

/* V_WidgetCreate() -- Create new widget */
V_Widget_t* V_WidgetCreate(V_Widget_t* const a_Parent, const char* const a_Type, const char* const a_ID)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return NULL;
	
	/*** STANDARD CLIENT ***/
#else
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return NULL;	
	
	/* Success! */
	return NULL;
#endif /* __REMOOD_DEDICATED */
}

/* V_WidgetDestroy() -- Destroy widget */
void V_WidgetDestroy(V_Widget_t* const a_Widget)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return;
	
	/*** STANDARD CLIENT ***/
#else
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return;
#endif /* __REMOOD_DEDICATED */
}

/* V_WidgetDraw() -- Draw Widget */
void V_WidgetDraw(V_Widget_t* const a_Widget, const uint32_t a_Flags)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return;
	
	/*** STANDARD CLIENT ***/
#else
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return;	
#endif /* __REMOOD_DEDICATED */
}

/* V_WidgetSetValue() -- Set widget value */
bool_t V_WidgetSetValue(V_Widget_t* const a_Widget, const char* const a_Value)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return false;
	
	/*** STANDARD CLIENT ***/
#else
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return false;	
	
	/* Success! */
	return false;
#endif /* __REMOOD_DEDICATED */
}

/* V_WidgetSetSize() -- Set size of widget */
bool_t V_WidgetSetSize(V_Widget_t* const a_Widget, const int32_t a_Width, const int32_t a_Height)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return false;
	
	/*** STANDARD CLIENT ***/
#else
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return false;	
	
	/* Success! */
	return false;
#endif /* __REMOOD_DEDICATED */
}

/* V_WidgetSetPosition() -- Set position of widget */
bool_t V_WidgetSetPosition(V_Widget_t* const a_Widget, const int32_t a_X, const int32_t a_Y)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return false;
	
	/*** STANDARD CLIENT ***/
#else
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return false;	
	
	/* Success! */
	return false;
#endif /* __REMOOD_DEDICATED */
}

