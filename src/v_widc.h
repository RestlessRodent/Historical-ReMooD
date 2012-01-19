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
	},
};

/***************
*** NEATMENU ***
***************/

/* VS_WH_NeatMenu_DrawFunc() -- Draw */
static bool_t VS_WH_NeatMenu_DrawFunc(V_Widget_t* const a_Widget, const uint32_t a_Flags, const int32_t a_X, const int32_t a_Y, const int32_t a_Width, const int32_t a_Height)
{
	size_t i;
	int32_t y;
	
	/* Check */
	if (!a_Widget)
		return false;
		
	/* Always make first kid in big text */
	a_Widget->Children[0]->Font = VFONT_LARGE;
	
	/* Draw children consecutively */
	for (y = 0, i = 0; i < a_Widget->NumChildren; i++)
	{
		// No kid here?
		if (!a_Widget->Children[i])
			continue;
		
		// Auto size
		((V_WidgetHandlerAutoSizeFunc_t)(VS_WTMI(a_Widget->Children[i], VWHFID_AUTOSIZE)))(a_Widget->Children[i]);
		
		// Set position of child
		V_WidgetSetPosition(a_Widget->Children[i], a_X, a_Y + y);
		
		// Draw child
		V_WidgetDraw(a_Widget->Children[i], a_Flags);
		
		// Move around
		y += a_Widget->Children[i]->Height;
	}
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
	/* Check */
	if (!a_Widget || !a_KidToAdd)
		return false;
		
	/* Always make first kid in big text */
	a_Widget->Children[0]->Font = VFONT_LARGE;
	
	/* Auto size the kid */
	((V_WidgetHandlerAutoSizeFunc_t)(VS_WTMI(a_KidToAdd, VWHFID_AUTOSIZE)))(a_KidToAdd);
	
	/* Success */	
	return true;
}

/* VS_WH_NeatMenu_KidChangedValueFunc() -- Kid changed value */
static bool_t VS_WH_NeatMenu_KidChangedValueFunc(V_Widget_t* const a_Widget, V_Widget_t* const a_Kid, const char* const a_Value)
{
	/* Check */
	if (!a_Widget || !a_Kid)
		return false;
	
	/* Determine kid size */
	((V_WidgetHandlerAutoSizeFunc_t)(VS_WTMI(a_Kid, VWHFID_AUTOSIZE)))(a_Kid);
	
	/* Success! */
	return true;
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
	},
};

#endif /* __V_WIDGET_C__ */
#endif /* __V_WIDC_H__ */

