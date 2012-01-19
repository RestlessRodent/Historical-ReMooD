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

#define __V_WIDGET_C__

/****************
*** CONSTANTS ***
****************/

/* V_WidgetHandlerFuncId_t -- Function ID for the handler */
typedef enum V_WidgetHandlerFuncId_e
{
	VWHFID_CREATE,
	VWHFID_DELETE,
	VWHFID_DRAW,
	VWHFID_SETVALUE,
	VWHFID_SETSIZE,
	VWHFID_KIDSOK,
	VWHFID_ADDKID,
	VWHFID_KIDCHANGEDVAL,
	VWHFID_AUTOSIZE,
	
	NUMWIDGETHANDLERFUNCS
} V_WidgetHandlerFuncId_t;

/*****************
*** STRUCTURES ***
*****************/

typedef bool_t (*V_WidgetHandlerAbstractFunc_t)();

typedef bool_t (*V_WidgetHandlerCreateFunc_t)(V_Widget_t* const a_Widget, const char* const a_Type);
typedef bool_t (*V_WidgetHandlerDeleteFunc_t)(V_Widget_t* const a_Widget);
typedef bool_t (*V_WidgetHandlerDrawFunc_t)(V_Widget_t* const a_Widget, const uint32_t a_Flags, const int32_t a_X, const int32_t a_Y, const int32_t a_Width, const int32_t a_Height);
typedef bool_t (*V_WidgetHandlerSetValueFunc_t)(V_Widget_t* const a_Widget, const char* const a_Value);
typedef bool_t (*V_WidgetHandlerSetDimensionFunc_t)(V_Widget_t* const a_Widget, const int32_t a_X, const int32_t a_Y, const int32_t a_Width, const int32_t a_Height);
typedef bool_t (*V_WidgetHandlerCanAddKidFunc_t)(V_Widget_t* const a_Widget, V_Widget_t* const a_KidToAdd);
typedef bool_t (*V_WidgetHandlerAddKidFunc_t)(V_Widget_t* const a_Widget, V_Widget_t* const a_KidToAdd);
typedef bool_t (*V_WidgetHandlerKidChangedValueFunc_t)(V_Widget_t* const a_Widget, V_Widget_t* const a_Kid, const char* const a_Value);
typedef bool_t (*V_WidgetHandlerAutoSizeFunc_t)(V_Widget_t* const a_Widget);

/* V_WidgetHandler_t -- Handles a widget */
typedef struct V_WidgetHandler_s
{
	/* Base */
	const char* TypeName;						// Widget Type
	struct V_WidgetHandler_s* ParentHandler;	// Derived From
	
	/* Handler Chain */
	struct V_WidgetHandler_s* Prev;
	struct V_WidgetHandler_s* Next;
	
	/* Functions */
	V_WidgetHandlerAbstractFunc_t Handlers[NUMWIDGETHANDLERFUNCS];
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
	VideoFont_t Font;							// Font to use
	
	/* Drawing */
	int32_t XPos;								// X position
	int32_t YPos;								// Y position
	int32_t Width;								// Width of widget
	int32_t Height;								// Height of widget
	
	/* Handlers */
	V_WidgetHandler_t* Handler;					// Handler to use
};

/****************
*** CONSTANTS ***
****************/

/*************
*** LOCALS ***
*************/

static V_WidgetHandler_t* l_WidgetHandlers = NULL;

/****************
*** FUNCTIONS ***
****************/

/* VS_WidgetTopMostImpl() -- Top most implementor */
static V_WidgetHandlerAbstractFunc_t VS_WTMI(V_Widget_t* const a_Widget, const V_WidgetHandlerFuncId_t a_ID)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return NULL;
	
	/*** STANDARD CLIENT ***/
#else
	V_WidgetHandler_t* HandleRover;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return NULL;
	
	/* Check */
	if (!a_Widget || a_ID < 0 || a_ID >= NUMWIDGETHANDLERFUNCS)
		return NULL;
	
	/* Start at current widget */
	HandleRover = a_Widget->Handler;
	
	// While we can rove
	while (HandleRover)
	{
		// Check if implemented
		if (HandleRover->Handlers[a_ID])
			return HandleRover->Handlers[a_ID];
		
		// Go up more
		HandleRover = HandleRover->ParentHandler;
	}
	
	/* Never hit? */
	return NULL;
#endif /* __REMOOD_DEDICATED */
}

/* Never in Dedicated */
#if !defined(__REMOOD_DEDICATED)
#include "v_widc.h"
#endif /* __REMOOD_DEDICATED */

/* VS_RegisterWidgetHandler() -- Registers a widget handler */
static bool_t VS_RegisterWidgetHandler(V_WidgetHandler_t* const a_Handler)
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
	if (!a_Handler)
		return false;
	
	/* If there are no handlers, set first */
	if (!l_WidgetHandlers)
		l_WidgetHandlers = a_Handler;
	
	/* If there are, then just set them at the start */
	else
	{
		l_WidgetHandlers->Prev = a_Handler;
		a_Handler->Next = l_WidgetHandlers;
		l_WidgetHandlers = a_Handler;
	}
	
	/* Success! */
	return true;
#endif /* __REMOOD_DEDICATED */
}

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
	V_WidgetHandler_t* NewHandler;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return false;
	
	/* Register base widgets */
	// None
	VS_RegisterWidgetHandler(&l_WH_None);
	
	// ColorBox
	VS_RegisterWidgetHandler(&l_WH_ColorBox);
	
	// Label
	VS_RegisterWidgetHandler(&l_WH_Label);
	
	// NeatMenu
	VS_RegisterWidgetHandler(&l_WH_NeatMenu);
	
	/* Success! */
	return true;
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
	V_Widget_t* NewWidget;
	V_WidgetHandler_t* Handler;
	size_t i;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return NULL;
	
	/* Check */
	if (!a_Type || !a_ID)
		return NULL;
	
	/* Before trying anything, see if the parent can accept more kids */
	if (a_Parent)
	{
		// See if the parent can add more kids
		if (!((V_WidgetHandlerCanAddKidFunc_t)(VS_WTMI(a_Parent, VWHFID_KIDSOK)))(a_Parent, NULL))
		{
			if (devparm)
				CONS_Printf("V_WidgetCreate: Wanted to add to parent, but parent does not want more kids.\n");
			
			return NULL;
		}
	}
	
	/* See if the widget type exists */
	Handler = l_WidgetHandlers;
	while (Handler)
	{
		// Check name
		if (strcasecmp(a_Type, Handler->TypeName) == 0)
			break;
		
		// Next
		Handler = Handler->Next;
	}
	
	// No handler?
	if (!Handler)
		return NULL;
	
	/* Create */
	NewWidget = Z_Malloc(sizeof(*NewWidget), PU_STATIC, NULL);
	
	/* Set handler */
	NewWidget->Handler = Handler;
	
	/* Call creation handler */
	((V_WidgetHandlerCreateFunc_t)(VS_WTMI(NewWidget, VWHFID_CREATE)))(NewWidget, a_Type);
	
	/* Add new widget to parent */
	if (a_Parent)
	{
		// See if there is a blank spot
		for (i = 0; i < a_Parent->NumChildren; i++)
			if (!a_Parent->Children[i])
			{
				a_Parent->Children[i] = NewWidget;
				break;
			}
		
		// No room, so add to end
		if (i == a_Parent->NumChildren)
		{
			// Resize array
			Z_ResizeArray((void**)&a_Parent->Children, sizeof(*a_Parent->Children), a_Parent->NumChildren, a_Parent->NumChildren + 1);
			
			// Set last child and increment at the same time
			a_Parent->Children[a_Parent->NumChildren++] = NewWidget;
		}
		
		// Call Informer (that we added a new kid)
		((V_WidgetHandlerAddKidFunc_t)(VS_WTMI(NewWidget, VWHFID_ADDKID)))(a_Parent, NewWidget);
	}
	
	/* Success! */
	return NewWidget;
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
	size_t i;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return;
	
	/* Check */
	if (!a_Widget)
		return;
	
	/* Delete kids first */
	for (i = 0; i < a_Widget->NumChildren; i++)
	{
		V_WidgetDestroy(a_Widget->Children[i]);
		a_Widget->Children[i] = NULL;
	}
	
	/* Call delete */
	((V_WidgetHandlerDeleteFunc_t)(VS_WTMI(a_Widget, VWHFID_DELETE)))(a_Widget);
	
	/* Free memory */
	Z_Free(a_Widget);
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
	
	/* Check */
	if (!a_Widget)
		return;
	
	/* Call callback */
	((V_WidgetHandlerDrawFunc_t)(VS_WTMI(a_Widget, VWHFID_DRAW)))(a_Widget, a_Flags, a_Widget->XPos, a_Widget->YPos, a_Widget->Width, a_Widget->Height);
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
	bool_t RetVal;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return false;
	
	/* Check */
	if (!a_Widget)
		return false;
		
	/* Set new value */
	RetVal = ((V_WidgetHandlerSetValueFunc_t)(VS_WTMI(a_Widget, VWHFID_SETVALUE)))(a_Widget, a_Value);
	
	/* Inform parent of value chane */
	if (a_Widget->Parent)
		((V_WidgetHandlerKidChangedValueFunc_t)(VS_WTMI(a_Widget, VWHFID_KIDCHANGEDVAL)))(a_Widget->Parent, a_Widget, a_Value);	
	
	/* Success! */
	return RetVal;
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
	
	/* Check */
	if (!a_Widget)
		return false;
	
	/* Set Current Size */
	a_Widget->Width = a_Width;
	a_Widget->Height = a_Height;
	
	/* Call callback for this widget */
	((V_WidgetHandlerSetDimensionFunc_t)(VS_WTMI(a_Widget, VWHFID_SETSIZE)))(a_Widget, a_Widget->XPos, a_Widget->YPos, a_Widget->Width, a_Widget->Height);
	
	/* Success! */
	return true;
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
		
	/* Check */
	if (!a_Widget)
		return false;
	
	/* Set Current Position */
	a_Widget->XPos = a_X;
	a_Widget->YPos = a_Y;
	
	/* Call callback for this widget */
	((V_WidgetHandlerSetDimensionFunc_t)(VS_WTMI(a_Widget, VWHFID_SETSIZE)))(a_Widget, a_Widget->XPos, a_Widget->YPos, a_Widget->Width, a_Widget->Height);
	
	/* Success! */
	return true;
#endif /* __REMOOD_DEDICATED */
}

