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

#ifndef __V_WIDC_H__
#define __V_WIDC_H__
#if defined(__V_WIDGET_C__)

/***********
*** NONE ***
***********/

/* VS_WH_None_CreateFunc() -- Create */
static bool_t VS_WH_None_CreateFunc(V_Widget_t* const a_Widget, const char* const a_Type)
{
	return true;
}

/* VS_WH_None_DeleteFunc() -- Delete */
static bool_t VS_WH_None_DeleteFunc(V_Widget_t* const a_Widget)
{
	return true;
}

/* VS_WH_None_DrawFunc() -- Draw */
static bool_t VS_WH_None_DrawFunc(V_Widget_t* const a_Widget, const uint32_t a_Flags, const int32_t a_X, const int32_t a_Y, const int32_t a_Width, const int32_t a_Height)
{
	return true;
}

/* VS_WH_None_SetValueFunc() -- Set value */
static bool_t VS_WH_None_SetValueFunc(V_Widget_t* const a_Widget, const char* const a_Value)
{
	return true;
}

/* VS_WH_None_SetDimensionsFunc() -- Set dimensions */
static bool_t VS_WH_None_SetDimensionsFunc(V_Widget_t* const a_Widget, const int32_t a_X, const int32_t a_Y, const int32_t a_Width, const int32_t a_Height)
{
	return true;
}

/* VS_WH_None_CanAddKidFunc() -- Can add kids? */
static bool_t VS_WH_None_CanAddKidFunc(V_Widget_t* const a_Widget, V_Widget_t* const a_KidToAdd)
{
	return false;
}

/* VS_WH_None_AddKidFunc() -- Add kids */
static bool_t VS_WH_None_AddKidFunc(V_Widget_t* const a_Widget, V_Widget_t* const a_KidToAdd)
{
	return false;
}

/* VS_WH_None_KidChangedValueFunc() -- Kid changed value */
static bool_t VS_WH_None_KidChangedValueFunc(V_Widget_t* const a_Widget, V_Widget_t* const a_Kid, const char* const a_Value)
{
	return false;
}

/* VS_WH_None_AutoSizeFunc() -- Autosize widget */
static bool_t VS_WH_None_AutoSizeFunc(V_Widget_t* const a_Widget)
{
	return false;
}

/* VS_WH_None_SetPropFunc() -- Set property */
static bool_t VS_WH_None_SetPropFunc(V_Widget_t* const a_Widget, const char* const a_Property, const uint32_t a_PropHash, const char* const a_Value)
{
	return false;
}

/* VS_WH_None_GetPropFunc() -- Get property */
static bool_t VS_WH_None_GetPropFunc(V_Widget_t* const a_Widget, const char* const a_Property, const uint32_t a_PropHash, const char* const* a_ValuePtr)
{
	return false;
}

/* VS_WH_None_PropChangedFunc() -- Property changed */
static bool_t VS_WH_None_PropChangedFunc(V_Widget_t* const a_Widget, const char* const a_Property, const uint32_t a_PropHash, const char* const a_Value)
{
	return false;
}

/* Handler Struct */
static V_WidgetHandler_t l_WH_None =
{
	/* Base */
	"none",
	NULL,
	
	/* Handler Chain */
	NULL,
	NULL,
	
	/* Functions */
	{
		(V_WidgetHandlerAbstractFunc_t)VS_WH_None_CreateFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_None_DeleteFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_None_DrawFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_None_SetValueFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_None_SetDimensionsFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_None_CanAddKidFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_None_AddKidFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_None_KidChangedValueFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_None_AutoSizeFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_None_SetPropFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_None_GetPropFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_None_PropChangedFunc,
	},
};

/****************
*** COLOR BOX ***
****************/

/* VS_WH_ColorBox_DrawFunc() -- Draw */
static bool_t VS_WH_ColorBox_DrawFunc(V_Widget_t* const a_Widget, const uint32_t a_Flags, const int32_t a_X, const int32_t a_Y, const int32_t a_Width, const int32_t a_Height)
{
	/* Check */
	if (!a_Widget)
		return false;
	
	/* Draw a nice box */
	V_DrawColorBoxEx(a_Flags, 127, a_X, a_Y, a_X + a_Width, a_Y + a_Height);
	
	/* Success! */
	return true;
}

/* Handler Struct */
static V_WidgetHandler_t l_WH_ColorBox =
{
	/* Base */
	"colorbox",
	&l_WH_None,
	
	/* Handler Chain */
	NULL,
	NULL,
	
	/* Functions */
	{
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_ColorBox_CreateFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_ColorBox_DeleteFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_ColorBox_DrawFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_ColorBox_SetValueFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_ColorBox_SetDimensionsFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_ColorBox_CanAddKidFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_ColorBox_AddKidFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_ColorBox_KidChangedValueFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_ColorBox_AutoSizeFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_ColorBox_SetPropFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_ColorBox_GetPropFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_ColorBox_PropChangedFunc,
	},
};

/************
*** LABEL ***
************/

/* VS_WH_Label_DeleteFunc() -- Delete */
static bool_t VS_WH_Label_DeleteFunc(V_Widget_t* const a_Widget)
{
	/* Check */
	if (!a_Widget)
		return false;
	
	/* Delete value if any */
	if (a_Widget->ValueP)
		Z_Free(a_Widget->ValueP);
	a_Widget->ValueP = NULL;
	
	/* Return */
	return true;
}

/* VS_WH_Label_DrawFunc() -- Draw */
static bool_t VS_WH_Label_DrawFunc(V_Widget_t* const a_Widget, const uint32_t a_Flags, const int32_t a_X, const int32_t a_Y, const int32_t a_Width, const int32_t a_Height)
{
	/* Check */
	if (!a_Widget)
		return false;
	
	/* Draw string */
	if (a_Widget->ValueP)
		V_DrawStringA(a_Widget->Font, a_Flags, a_Widget->ValueP, a_X, a_Y);
	
	/* Success */
	return true;
}

/* VS_WH_Label_SetValueFunc() -- Set value */
static bool_t VS_WH_Label_SetValueFunc(V_Widget_t* const a_Widget, const char* const a_Value)
{
	/* Check */
	if (!a_Widget || !a_Value)
		return false;
		
	/* Clear old value */
	if (a_Widget->ValueP)
		Z_Free(a_Widget->ValueP);
	a_Widget->ValueP = NULL;
	
	/* Duplicate */
	a_Widget->ValueP = Z_StrDup(a_Value, PU_STATIC, NULL);
	
	/* Set true */
	return true;
}

/* VS_WH_Label_AutoSizeFunc() -- Set size automatically */
static bool_t VS_WH_Label_AutoSizeFunc(V_Widget_t* const a_Widget)
{
	int w, h;
	
	/* Check */
	if (!a_Widget)
		return false;
	
	/* Is there something here? */
	if (a_Widget->ValueP)
	{
		// Determine size
		V_StringDimensionsA(a_Widget->Font, 0, a_Widget->ValueP, &w, &h);
		
		// Set
		a_Widget->Width = w;
		a_Widget->Height = h;
	}
	
	/* Make it sizeless */
	else
	{
		a_Widget->Width = 0;
		a_Widget->Height = 0;
	}
	
	/* Success */
	return true;
}

/* Handler Struct */
static V_WidgetHandler_t l_WH_Label =
{
	/* Base */
	"label",
	&l_WH_None,
	
	/* Handler Chain */
	NULL,
	NULL,
	
	/* Functions */
	{
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_Label_CreateFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_Label_DeleteFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_Label_DrawFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_Label_SetValueFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_Label_SetDimensionsFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_Label_CanAddKidFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_Label_AddKidFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_Label_KidChangedValueFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_Label_AutoSizeFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_Label_SetPropFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_Label_GetPropFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_Label_PropChangedFunc,
	},
};

/***************
*** NEATMENU ***
***************/

/* VS_WH_NeatMenu_DrawFunc() -- Draw */
static bool_t VS_WH_NeatMenu_DrawFunc(V_Widget_t* const a_Widget, const uint32_t a_Flags, const int32_t a_X, const int32_t a_Y, const int32_t a_Width, const int32_t a_Height)
{
#define MENUSPACER 2
#define MENULEFTSPACE 10
	size_t i, SelectedItem;
	int32_t y;
	uint32_t Flags;
	
	/* Check */
	if (!a_Widget)
		return false;
		
	/* Always make first kid in big text */
	a_Widget->Children[0]->Font = VFONT_LARGE;
	
	/* Get the currently selected item */
	SelectedItem = V_WidgetGetPropertyInt(a_Widget, "selected");
	
	// Offset by 2 (title and hint)
	SelectedItem += 2;
	
	/* Draw menu background */
	// Blur pixel effect
	y = 0;
	
	// Top title bar
	if (a_Widget->NumChildren)
		if (a_Widget->Children[0])
		{
			// Some height
			y += a_Widget->Children[0]->Height << 1;
			
			// Draw background
			V_DrawFadeConsBackEx(VEX_COLORMAP(VEX_MAP_BLACK), a_X, a_Y, a_Width, a_Y + y);
		}
	
	// Bottom black
	V_DrawFadeConsBackEx(VEX_COLORMAP(VEX_MAP_NONE), a_X, a_Y + y, a_Width, a_Y + (a_Height - y));
	
	/* Draw children consecutively */
	for (y = 0, i = 0; i < a_Widget->NumChildren; i++)
	{
		// No kid here?
		if (!a_Widget->Children[i])
			continue;
		
		// If this is the first widget (it is a title), center it
		if (i == 0)
		{
			// Space on top
			y += a_Widget->Children[i]->Height >> 1;
			
			// Center in the middle
			V_WidgetSetPosition(a_Widget->Children[i], a_X + (a_Widget->Width >> 1) - (a_Widget->Children[i]->Width >> 1), a_Y + y);
			
			// Space on the bottom
			y += a_Widget->Children[i]->Height >> 1;
			
			// Bright white title
			Flags = VFO_COLOR(VEX_MAP_BRIGHTWHITE);
		}
		
		// If this is the second widget, it is the help string
		else if (i == 1)
		{
			// Set to bright white
			Flags = VFO_COLOR(VEX_MAP_MAGENTA);
			
			// Set position of child
			V_WidgetSetPosition(a_Widget->Children[i], a_X + (MENULEFTSPACE >> 1), a_Y + (a_Height - (MENUSPACER << 1) - a_Widget->Children[i]->Height));
		}
		
		// Selected Widget?
		else if (i == SelectedItem)
		{
			// Set to bright white
			Flags = VFO_COLOR(VEX_MAP_BRIGHTWHITE);
			
			// Draw a star
			if (gametic & 8)
				V_DrawCharacterA(VFONT_SMALL, Flags | a_Flags, '*', a_X + 2, a_Y + y);
			
			// Set position of child
			V_WidgetSetPosition(a_Widget->Children[i], a_X + MENULEFTSPACE, a_Y + y);
		}
		
		// Not selected
		else
		{
			// Use default flags
			Flags = 0;
			
			// Set position of child
			V_WidgetSetPosition(a_Widget->Children[i], a_X + MENULEFTSPACE, a_Y + y);
		}
		
		// Draw child
		V_WidgetDraw(a_Widget->Children[i], Flags | a_Flags);
		
		// Move around
		if (i != 1)
			y += a_Widget->Children[i]->Height + (MENUSPACER << 1);
	}
#undef MENULEFTSPACE
#undef MENUSPACER
}

/* VS_WH_NeatMenu_CanAddKidFunc() -- Can add kids? */
static bool_t VS_WH_NeatMenu_CanAddKidFunc(V_Widget_t* const a_Widget, V_Widget_t* const a_KidToAdd)
{
	/* Check */
	if (!a_Widget)
		return false;
	
	/* Can always add kids */
	return true;
}

/* VS_WH_NeatMenu_AddKidFunc() -- Add kids */
static bool_t VS_WH_NeatMenu_AddKidFunc(V_Widget_t* const a_Widget, V_Widget_t* const a_KidToAdd)
{
	size_t i;
	
	/* Check */
	if (!a_Widget || !a_KidToAdd)
		return false;
		
	/* Always make first kid in big text */
	a_Widget->Children[0]->Font = VFONT_LARGE;
	
	/* Autosize widgets */
	for (i = 0; i < a_Widget->NumChildren; i++)
		if (a_Widget->Children[i])
			((V_WidgetHandlerAutoSizeFunc_t)(VS_WTMI(a_Widget->Children[i], VWHFID_AUTOSIZE)))(a_Widget->Children[i]);
	
	/* Success */	
	return true;
}

/* VS_WH_NeatMenu_KidChangedValueFunc() -- Kid changed value */
static bool_t VS_WH_NeatMenu_KidChangedValueFunc(V_Widget_t* const a_Widget, V_Widget_t* const a_Kid, const char* const a_Value)
{
	size_t i;
	
	/* Check */
	if (!a_Widget || !a_Kid)
		return false;
	
	/* Autosize widgets */
	for (i = 0; i < a_Widget->NumChildren; i++)
		if (a_Widget->Children[i])
			((V_WidgetHandlerAutoSizeFunc_t)(VS_WTMI(a_Widget->Children[i], VWHFID_AUTOSIZE)))(a_Widget->Children[i]);
	
	/* Success! */
	return true;
}

/* VS_WH_NeatMenu_SetPropFunc() -- Set property */
static bool_t VS_WH_NeatMenu_SetPropFunc(V_Widget_t* const a_Widget, const char* const a_Property, const uint32_t a_PropHash, const char* const a_Value)
{
	static bool_t PreHashed;
	static uint32_t Hashes[1];
	
	/* Check */
	if (!a_Widget || !a_Property)
		return false;
	
	/* Pre-hash? */
	if (!PreHashed)
	{
		Hashes[0] = Z_Hash("selected");
		
		// Set
		PreHashed = true;
	}
	
	/* Compare */
	if (
		(a_PropHash == Hashes[0] && strcasecmp(a_Property, "selected") == 0)
		)
		return true;
	
	/* Not what we want */
	return false;
}

/* Handler Struct */
static V_WidgetHandler_t l_WH_NeatMenu =
{
	/* Base */
	"neatmenu",
	&l_WH_None,
	
	/* Handler Chain */
	NULL,
	NULL,
	
	/* Functions */
	{
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_NeatMenu_CreateFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_NeatMenu_DeleteFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_NeatMenu_DrawFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_NeatMenu_SetValueFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_NeatMenu_SetDimensionsFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_NeatMenu_CanAddKidFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_NeatMenu_AddKidFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_NeatMenu_KidChangedValueFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_NeatMenu_AutoSizeFunc,
		(V_WidgetHandlerAbstractFunc_t)VS_WH_NeatMenu_SetPropFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_NeatMenu_GetPropFunc,
		NULL,//(V_WidgetHandlerAbstractFunc_t)VS_WH_NeatMenu_PropChangedFunc,
	},
};

#endif /* __V_WIDGET_C__ */
#endif /* __V_WIDC_H__ */

