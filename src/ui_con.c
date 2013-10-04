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
// DESCRIPTION: Console Drawer

/* NOT IN DEDICATED SERVER */
#if !defined(__REMOOD_DEDICATED)
/***************************/

/***************
*** INCLUDES ***
***************/

#include "ui.h"

#include "bootdata.h"

/*************
*** LOCALS ***
*************/

static UI_Img_t* l_BootLogo = NULL;

/****************
*** FUNCTIONS ***
****************/

/* UI_ConBootInit() -- Initialization of the boot console */
void UI_ConBootInit(void)
{
	/* Load boot logo */
	if (!(l_BootLogo = UI_ImgLoadBootLogo(c_BootLogo, CBOOTLOGOSIZE)))
		I_Error("Failed to load boot logo!");
	
	/* Reference it */
	UI_ImgCount(l_BootLogo, 1);
}

/* UI_ConBootClear() -- Clear the boot logo */
void UI_ConBootClear(void)
{
	return;
	
	/* If image does not exist, do not bother */
	if (!l_BootLogo)
		return;	
	
	/* Dereference logo */
	UI_ImgCount(l_BootLogo, -1);
	
	// No longer reference it
	l_BootLogo = NULL;
}

/* UI_ConPassLine() -- Passes line of text to the console */
void UI_ConPassLine(const char* const a_Line)
{
	/* Boot Console? */
	if (l_BootLogo)
	{
	}
	
	/* Normal Console */
	else
	{
	}
}

/* UI_ConDebug() -- Debug only */
void UI_ConDebug(UI_BufferSpec_t* const a_Spec)
{
	UI_DrawImg(a_Spec, l_BootLogo, 1, 1);
}

/* NOT IN DEDICATED SERVER */
#endif
/***************************/

