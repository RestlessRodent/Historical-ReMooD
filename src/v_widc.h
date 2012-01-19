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
	},
};

#endif /* __V_WIDGET_C__ */
#endif /* __V_WIDC_H__ */

