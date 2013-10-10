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
// DESCRIPTION: Interface Drawing Control

/* NOT IN DEDICATED SERVER */
#if !defined(__REMOOD_DEDICATED)
/***************************/

/***************
*** INCLUDES ***
***************/

#include "ui.h"
#include "i_video.h"
#include "ui_dloc.h"
#include "w_wad.h"

/****************
*** CONSTANTS ***
****************/

#define UIPROT(d) \
void UI_##d##_DrawImg(UI_BufferSpec_t* const a_Spec, UI_Img_t* const a_Img, const int32_t a_X, const int32_t a_Y);\
void UI_##d##_DrawImgScale(UI_BufferSpec_t* const a_Spec, UI_Img_t* const a_Img, const int32_t a_X, const int32_t a_Y, const fixed_t a_sW, const fixed_t a_sH);

#define UISET(d) \
UI_DrawImg = UI_##d##_DrawImg;\
UI_DrawImgScale = UI_##d##_DrawImgScale

UIPROT(d8)
UIPROT(d16)
UIPROT(d32)
UIPROT(dgl)

/**************
*** GLOBALS ***
**************/

void (*UI_DrawImg)(UI_BufferSpec_t* const a_Spec, UI_Img_t* const a_Img, const int32_t a_X, const int32_t a_Y) = NULL;
void (*UI_DrawImgScale)(UI_BufferSpec_t* const a_Spec, UI_Img_t* const a_Img, const int32_t a_X, const int32_t a_Y, const fixed_t a_sW, const fixed_t a_sH) = NULL;

/*****************
*** PROTOTYPES ***
*****************/

void D_UILoadTitles(void);

/****************
*** FUNCTIONS ***
****************/

/* UI_WLOrder() -- WAD order list */
bool_t UI_WLOrder(const bool_t a_Pushed, const WL_WADFile_t* const a_WAD)
{
	/* Clear image list */
	UI_ImgClearList();
	
	/* Load the default palette (PLAYPAL) */
	UI_ImgLoadPalette();
	
	/* Initialize the title screen */
	D_UILoadTitles();
	
	/* Always works */
	return true;
}

/* UI_Init() -- Initializes the UI */
void UI_Init(void)
{
	if (!WL_RegisterOCCB(UI_WLOrder, WLDCO_UIIMAGES))
		I_Error("Failed to register UI_WLOrder");
}

/* UI_SetBitDepth() -- Sets bit depth of rendering */
void UI_SetBitDepth(const uint32_t a_Depth)
{
	static bool_t FirstDepth;
	
	/* Remove boot logo */
	if (FirstDepth)
		UI_ConBootClear();
	
	/* Clear currently loaded images */
	UI_ImgClearList();
	
	/* Initialize the boot logo */
	if (!FirstDepth)
	{
		FirstDepth = true;
		
		UI_ConBootInit();
	}
	
	/* Set drawer functions */
	switch (a_Depth)
	{
			// 8-bit
		case 1:
			UISET(d8);
			break;
			
			// 16-bit
		case 2:
			UISET(d16);
			break;
			
			// 32-bit
		case 4:
			UISET(d32);
			break;
			
			// OpenGL
		case I_VIDEOGLMODECONST:
			UISET(dgl);
			break;
	}
}

/* NOT IN DEDICATED SERVER */
#endif
/***************************/

