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
// DESCRIPTION: User Interface
//
// In terms of old code, replaces:
//   * Console
//   * Menu
//   * HUD
//   * Status Bar

#ifndef __UI_H__
#define __UI_H__

/* NOT IN DEDICATED SERVER */
#if !defined(__REMOOD_DEDICATED)
/***************************/

/*****************************************************************************/

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "i_util.h"

/****************
*** CONSTANTS ***
****************/

/* UI_ImgRef_t -- Type of image reference */
typedef enum UI_ImgRef_e
{
	UIIR_WAD,									// Reference by WAD
	UIIR_RAW,									// Reference by Raw Data
	UIIR_BOOTLOGO,								// Boot Logo
} UI_ImgRef_t;

/* UI_ColorMap_t -- Color Mapping of Image */
typedef enum UI_ColorMap_e
{
	UICM_NATIVE,								// Native Mapping
	UICM_DOOM,									// Doom Colors
	UICM_HERETIC,								// Heretic Colors
	UICM_HEXEN,									// Hexen Colors
	UICM_STRIFE,								// Strife Colors
} UI_ColorMap_t;

/*****************
*** STRUCTURES ***
*****************/

/* Define UI_Img_t */
#if !defined(__REMOOD_UIIMG_DEFINED)
	typedef struct UI_Img_s UI_Img_t;
	#define __REMOOD_UIIMG_DEFINED
#endif

/* Define WL_WADEntry_t */
#if !defined(__REMOOD_WLWADENT_DEFINED)
	typedef struct WL_WADEntry_s WL_WADEntry_t;
	#define __REMOOD_WLWADENT_DEFINED
#endif

/* Define WL_WADFile_t */
#if !defined(__REMOOD_WLWADFILE_DEFINED)
	typedef struct WL_WADFile_s WL_WADFile_t;
	#define __REMOOD_WLWADFILE_DEFINED
#endif

/* UI_Img_t -- User Interface Image */
struct UI_Img_s
{
	UI_Img_t* Prev;								// Previous Image
	UI_Img_t* Next;								// Next Image
	
	UI_ImgRef_t RefType;						// Type of reference
	union
	{
		struct
		{
			WL_WADFile_t* WAD;					// WAD This belongs to
			WL_WADEntry_t* Entry;				// Entry this belongs to
		} WAD;									// WAD Storage
		
		struct
		{
			uint8_t* Data;						// Data Reference
			size_t Len;							// Size of data
		} Raw;									// Raw data
	} Ref;										// Reference
	
	int32_t o[2];								// Offset
	int32_t l[2];								// Size
	uint32_t Depth;								// Depth image exists for
	
	UI_ColorMap_t Map;							// Color Mapping
	
	uint8_t* Data;								// Image Data (Native)
	uint8_t* Mask;								// Image Mask
	uint32_t GLRef;								// OpenGL Reference
	
	int32_t Count;								// Reference Count
};

/* UI_RGB_t -- RGB Table Entry */
typedef struct UI_RGB_s
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} UI_RGB_t;

/* UI_BufferSpec_t -- Buffer specification */
typedef struct UI_BufferSpec_s
{
	uint8_t* Data;								// Buffer Data
	uint32_t w, h, d, p;						// Width, Height, Depth, Pitch
} UI_BufferSpec_t;

/****************
*** FUNCTIONS ***
****************/

/*** UI_VIS.C ***/

bool_t UI_Visible(void);
bool_t UI_GrabMouse(void);
bool_t UI_ShouldFreezeGame(void);
void UI_Ticker(void);
void UI_Drawer(void);	// Move to UI_DRAW.C
bool_t UI_HandleEvent(I_EventEx_t* const a_Event, const bool_t a_Early);	// Move to UI_EVENT.C

/*** UI_CON.C ***/

void UI_ConBootInit(void);
void UI_ConBootClear(void);
void UI_ConPassLine(const char* const a_Line);

/*** UI_CTRL.C ***/

void UI_Init(void);
void UI_SetBitDepth(const uint32_t a_Depth);

void UI_GetVideo(UI_BufferSpec_t* const a_Spec);

/*** UI_DRAW.C ***/

void UI_DrawLoop(void);

/*** UI_D*.C ***/

extern void (*UI_DrawImg)(UI_BufferSpec_t* const a_Spec, UI_Img_t* const a_Img, const int32_t a_X, const int32_t a_Y);

/*** UI_IMG.C ***/

void UI_ImgClearList(void);

UI_Img_t* UI_ImgLoadEntC(const WL_WADEntry_t* const a_Entry, const UI_ColorMap_t a_Map);
UI_Img_t* UI_ImgLoadEnt(const WL_WADEntry_t* const a_Entry);

UI_Img_t* UI_ImgLoadEntSC(const char* const a_Name, const UI_ColorMap_t a_Map);
UI_Img_t* UI_ImgLoadEntS(const char* const a_Name);

UI_Img_t* UI_ImgLoadBootLogo(const uint8_t* const a_Data, const size_t a_Len);

int32_t UI_ImgCount(UI_Img_t* const a_Img, const int32_t a_Count);

/*****************************************************************************/

/* NOT IN DEDICATED SERVER */
#endif
/***************************/

#endif /* __UI_H__ */

