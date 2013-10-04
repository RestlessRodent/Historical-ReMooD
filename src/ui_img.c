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
#include "z_zone.h"
#include "screen.h"

/****************
*** CONSTANTS ***
****************/

// l_BootPal -- Boot palette
static const UI_RGB_t l_BootPal[256] =
{
	{0x00, 0x00, 0x00},
	{0x05, 0x05, 0x00},
	{0x4c, 0x01, 0x00},
	{0x9b, 0x00, 0x00},
	{0xfd, 0xff, 0xfc},
	{0xff, 0xff, 0xff},
	{0xff, 0x00, 0x00},
};

/*****************
*** STRUCTURES ***
*****************/

/*************
*** LOCALS ***
*************/

static UI_Img_t* l_ImgList = NULL;				// List of images

/****************
*** FUNCTIONS ***
****************/

/* UI_ImgClearList() -- Clears the image list */
void UI_ImgClearList(void)
{
	UI_Img_t* Rover, *Next;
	
	/* Go through all images */
	for (Rover = l_ImgList, Next = NULL; Rover; Rover = Next)
	{
		// Get next, may be lost
		Next = Rover->Next;
		
		// If still referenced, I_Error
		if (Rover->Count > 0)
			I_Error("Image still referenced!");
		
		// Free associated data
		if (Rover->Data)
			Z_Free(Rover->Data);
		
		if (Rover->Mask)
			Z_Free(Rover->Mask);
		
		// Unlink
		if (Rover->Prev)
			Rover->Prev->Next = Rover->Next;
		if (Rover->Next)
			Rover->Next->Prev = Rover->Prev;
		
		if (l_ImgList == Rover)
			if (Rover->Prev)
				l_ImgList = Rover->Prev;
			else
				l_ImgList = Rover->Next;
		
		// Delete image
		Z_Free(Rover);
	}
}

/* UI_ImgPutI() -- Puts single pixel in image */
static void UI_ImgPutI(UI_Img_t* const a_Img, const int32_t a_X, const int32_t a_Y, const uint32_t a_Index)
{
	uint32_t Color;
	UI_RGB_t *OrigPal;
	
	/* Check */
	if (!a_Img)
		return;
	
	/* Out of bounds? */
	if (a_X < 0 || a_Y < 0 || a_X >= a_Img->l[0] || a_Y >= a_Img->l[1] || a_Index >= 256)
		return;
	
	/* Translate resulting color */
	// Boot logo?
	if (a_Img->RefType == UIIR_BOOTLOGO)
		OrigPal = l_BootPal;
	
	// Other palette, either native or remapped (ex: Doom images on Heretic)
	else
	{
		// TODO FIXME
		return;
	}
	
	// If using native pallete, direct copy
	if (a_Img->Map == UICM_NATIVE)
		Color = a_Index;
	
	// Otherwise, use the referenced index from the mapping
	else
		Color = 0;
	
	// Translate color to real palette color
	if (a_Img->Depth > 1)
		Color = SVRGB(OrigPal[Color].r, OrigPal[Color].b, OrigPal[Color].g);
	
	/* Draw real color */
	// Indexed Mode
	if (a_Img->Depth == 1)
		a_Img->Data[(a_Y * a_Img->p) + a_X] = Color;
	
	// High Color
	else if (a_Img->Depth == 2)
		((uint16_t*)a_Img->Data)[(a_Y * a_Img->p) + a_X] = Color;
	
	// True Color
	else if (a_Img->Depth == 4)
		((uint32_t*)a_Img->Data)[(a_Y * a_Img->p) + a_X] = D_CMakePureRandom();//Color;
	
	// Set Mask
	a_Img->Mask[(a_Y * a_Img->l[0]) + a_X] = 1;
}

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
	UI_Img_t* New;
	int i, x, y, j;
	int Count;
	uint8_t px[2];
	
	/* Check */
	if (!a_Data || !a_Len)
		return NULL;
	
	/* Go through images to find boot logo */
	for (New = l_ImgList; New; New = New->Next)
		if (New->RefType == UIIR_BOOTLOGO)
			break;
	
	// Does not exist?
	if (!New)
	{
		New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
		
		if (l_ImgList)
			l_ImgList->Prev = New;
		New->Next = l_ImgList;
		l_ImgList = New;
		
		New->RefType = UIIR_BOOTLOGO;
	}
	
	/* Image Data not loaded? */
	if (!New->Data)
	{
		New->l[0] = 198;
		New->l[1] = 198;
		New->Depth = vid.bpp;
		New->p = 198;
		New->pd = New->p * New->Depth;
		
		// Allocate buffer
		New->Data = Z_Malloc((New->l[0] * New->l[1]) * New->Depth, PU_STATIC, NULL);
		New->Mask = Z_Malloc((New->l[0] * New->l[1]), PU_STATIC, NULL);
		
		// Set the mask to all 1s
		memset(New->Mask, 1, (New->l[0] * New->l[1]));
		
		// Run decoder loop
		for (x = 0, y = 0, i = 0; i < a_Len;)
		{
			// Read RLE count
			Count = a_Data[i++];
			
			// Read pixels to set
			while (Count--)
			{
				// Get bits
				px[0] = ((a_Data[i] & 0x0F)) + 1;
				px[1] = ((a_Data[i++] & 0xF0) >> 4) + 1;
				
				// Draw into image
				UI_ImgPutI(New, x, y, px[0]);
				UI_ImgPutI(New, x + 1, y, px[1]);
				
				// Move x up
				x += 2;
				
				// Reached length of image
				if (x >= 198)
				{
					x = 0;
					y++;
				}
			}
		}
	}
	
	/* New or existing image */
	return New;
}

/* UI_ImgCount() -- Counts image usage */
int32_t UI_ImgCount(UI_Img_t* const a_Img, const int32_t a_Count)
{
	/* Check */
	if (!a_Img || !a_Count)
		return 0;
	
	/* Return count */
	return a_Img->Count;
}

/* NOT IN DEDICATED SERVER */
#endif
/***************************/

