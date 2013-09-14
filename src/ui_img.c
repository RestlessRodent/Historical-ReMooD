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
// Copyright (C) 2013-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Image Handling

/* NOT IN DEDICATED SERVER */
#if !defined(__REMOOD_DEDICATED)
/***************************/

/***************
*** INCLUDES ***
***************/

#include "ui.h"
#include "ui_dloc.h"

/*****************
*** STRUCTURES ***
*****************/

/*************
*** LOCALS ***
*************/

/****************
*** FUNCTIONS ***
****************/

/* UI_ImgLoadEntC() -- Lodas image with specified mapping */
UI_Img_t* UI_ImgLoadEntC(const WL_WADEntry_t* const a_Entry, const UI_ColorMap_t a_Map)
{
}

/* UI_ImgLoadEnt() -- Loads image with assumed native mapping */
UI_Img_t* UI_ImgLoadEnt(const WL_WADEntry_t* const a_Entry)
{
	return UI_ImgLoadEntC(a_Entry, UICM_NATIVE);
}

/* UI_ImgLoadEntSC() -- Finds entry by name, then uses a map */
UI_Img_t* UI_ImgLoadEntSC(const char* const a_Name, const UI_ColorMap_t a_Map)
{
	/* Check */
	if (!a_Name)
		return NULL;
}

/* UI_ImgLoadEntS() -- Loads image by name */
UI_Img_t* UI_ImgLoadEntS(const char* const a_Name)
{
	return UI_ImgLoadEntSC(a_Name, UICM_NATIVE);
}

/* UI_ImgLoadBootLogo() -- Loads the boot logo as an image */
UI_Img_t* UI_ImgLoadBootLogo(const uint8_t* const a_Data, const size_t a_Len)
{
}

/* UI_ImgCount() -- Counts image usage */
int32_t UI_ImgCount(UI_Img_t* const a_Img, const int32_t a_Count)
{
	/* Check */
	if (!a_Img || !a_Count)
		return 0;
}

/* NOT IN DEDICATED SERVER */
#endif
/***************************/

