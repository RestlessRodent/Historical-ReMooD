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
// Copyright (C) 2012 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Menu Code

/***************
*** INCLUDES ***
***************/

#include "m_menu.h"
#include "p_demcmp.h"
#include "p_local.h"
#include "r_data.h"

/****************
*** FUNCTIONS ***
****************/


/* MS_TextureTestUnderDraw() -- Menu Under Drawer */
static bool_t MS_TextureTestUnderDraw(struct M_UIMenuHandler_s* const a_Handler, struct M_UIMenu_s* const a_Menu, const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H)
{
#define BUFSIZE 32
	texture_t* Texture;
	char Buf[BUFSIZE];
	int32_t i;
	
	/* Check */
	if (!a_Menu)
		return;
	
	/* Get Texture Reference */
	Texture = textures[a_Menu->Items[a_Handler->CurItem].DataBits];
	
	/* Draw Texture */
	// Flat
	if (Texture->IsFlat)
	{
		if (R_GetFlat(a_Menu->Items[a_Handler->CurItem].DataBits))
			V_ImageDraw(0, Texture->FlatImage, a_X + (a_W >> 1), a_Y + ((a_H - Texture->height) - 5), NULL);
	}
	
	// Wall
	else
	{
	}
	
	/* Draw Info */
	// Type
	V_DrawStringA(VFONT_SMALL, VFO_COLOR(VEX_MAP_WHITE), (Texture->IsFlat ? "Flat" : "Wall"), a_X + (a_W >> 1), a_Y + 10);
	
	// Other Stuff
	for (i = 0; i <= 7; i++)
	{
		// Clear
		memset(Buf, 0, sizeof(Buf));
		
		// Set
		switch (i)
		{
			case 1:
				snprintf(Buf, BUFSIZE - 1, "Cb: %u", Texture->CombinedOrder);
				break;
			
			case 2:
				snprintf(Buf, BUFSIZE - 1, "Io: %u", Texture->InternalOrder);
				break;
			
			case 3:
				snprintf(Buf, BUFSIZE - 1, "w&: %x", Texture->WidthMask);
				break;
				
			case 4:
				snprintf(Buf, BUFSIZE - 1, "tr: %i", Texture->Translation);
				break;
				
			case 5:
				snprintf(Buf, BUFSIZE - 1, "cs: %u", Texture->CacheSize);
				break;
				
			case 6:
				snprintf(Buf, BUFSIZE - 1, "om: %u", Texture->OrderMul);
				break;
			
			case 7:
				if (!Texture->FlatEntry)
					snprintf(Buf, BUFSIZE - 1, "PW: ---");
				else
					snprintf(Buf, BUFSIZE - 1, "PW: %s", (((const WL_WADEntry_t*)Texture->FlatEntry)->Owner->__Private.__DOSName));
				break;
				
			case 0:
			default:
				snprintf(Buf, BUFSIZE - 1, "Dx: %ix%i", Texture->width, Texture->height);
				break;
		}
		
		// Draw
		V_DrawStringA(VFONT_SMALL, VFO_COLOR(VEX_MAP_WHITE), Buf, a_X + (a_W >> 1), a_Y + (10 * (i + 2)));
	}
	
	/* Keep Drawing */
	return true;
#undef BUFSIZE
}

/* M_ExGeneralComm() -- Menu Commands */
int M_ExGeneralComm(const uint32_t a_ArgC, const char** const a_ArgV)
{
	M_UIMenu_t* New;
	int32_t i;
	
	static const char* const c_ColorTestStrs[33] =
	{
		"{zDefault (z)",
		
		"{0Default (0)",
		"{1Red (1)",
		"{2Orange (2)",
		"{3Yellow (3)",
		"{4Green (4)",
		"{5Cyan (5)",
		"{6Blue (6)",
		"{7Magenta (7)",
		"{8Brown (8)",
		"{9Bright White (9)",
		"{aWhite (a)",
		"{bGray (b)",
		"{cBlack (c)",
		"{dFuscia (d)",
		"{eGold (e)",
		"{fTek Green (f)",
		
		"{x70Green (x70)",
		"{x71Gray (x71)",
		"{x72Brown (x72)",
		"{x73Red (x73)",
		"{x74Light Gray (x74)",
		"{x75Light Brown (x75)",
		"{x76Light Red (x76)",
		"{x77Light Blue (x77)",
		"{x78Blue (x78)",
		"{x79Yellow (x79)",
		"{x7aBeige (x7a)",
		"{x7bWhite (x7b)",
		"{x7cOrange (x7c)",
		"{x7dTan (x7d)",
		"{x7eBlack (x7e)",
		"{x7fPink (x7f)",
	};
	
	/* Texture Test */
	if (strcasecmp(a_ArgV[0], "menutexturetest") == 0)
	{
		// Allocate
		New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
		// Quick
		New->NumItems = numtextures;
		New->Items = Z_Malloc(sizeof(*New->Items) * New->NumItems, PU_STATIC, NULL);
		New->CleanerFunc = M_GenericCleanerFunc;
		New->UnderDrawFunc = MS_TextureTestUnderDraw;
		
		// Set Strings
		for (i = 0; i < numtextures; i++)
		{
			New->Items[i].Menu = New;
			New->Items[i].Text = textures[i]->name;
			New->Items[i].DataBits = i;
		}
		
		M_ExPushMenu(0, New);
	}
	
	/* Color Test */
	else if (strcasecmp(a_ArgV[0], "menucolortest") == 0)
	{
		// Allocate
		New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
		// Quick
		New->NumItems = 33;
		New->Items = Z_Malloc(sizeof(*New->Items) * New->NumItems, PU_STATIC, NULL);
		New->CleanerFunc = M_GenericCleanerFunc;
		
		// Set Strings
		for (i = 0; i < 33; i++)
		{
			New->Items[i].Menu = New;
			New->Items[i].Text = c_ColorTestStrs[i];
		}
		
		M_ExPushMenu(0, New);
	}
}


/* MS_GameVar_LRValChange() -- Game variable value changer callback */
static bool_t MS_GameVar_LRValChange(struct M_UIMenu_s* const a_Menu, struct M_UIItem_s* const a_Item, const bool_t a_More)
{
	P_XGSVariable_t* Bit;
	int32_t OldVal, NewVal, ModVal;
	
	/* Get bit */
	Bit = P_XGSVarForBit(a_Item->DataBits);
	
	// Failed?
	if (!Bit)
		return false;
		
	/* Value change amount? */
	ModVal = P_XGSGetNextValue(Bit->BitID, a_More);
	
	/* Get values and attempt change */
	OldVal = P_XGSVal(Bit->BitID);
	NewVal = P_XGSSetValue(false, Bit->BitID, ModVal);
	g_ResumeMenu++;
	
	// Success? Only if actually changed
	if (NewVal != OldVal)
		return true;
	return true;	// Always play sound
}

/* M_ExTemplateMakeGameVars() -- Make Game Variable Template */
M_UIMenu_t* M_ExTemplateMakeGameVars(const int32_t a_Mode)
{
	M_UIMenu_t* New;
	int32_t i, j, k, b, w, z;
	P_XGSVariable_t* Bit;
	P_XGSVariable_t** SortedBits;
	P_XGSMenuCategory_t LastCat;
	
	/* Sort bits */
	SortedBits = Z_Malloc(sizeof(*SortedBits) * (PEXGSNUMBITIDS - 1), PU_STATIC, NULL);
	
	// Init
	for (i = 1; i < PEXGSNUMBITIDS; i++)
		SortedBits[i - 1] = P_XGSVarForBit(i);
	
	// Sort by category first
	for (i = 0; i < (PEXGSNUMBITIDS - 1); i++)
	{
		// Initial
		b = i;
		
		// Find next lowest
		for (j = i + 1; j < (PEXGSNUMBITIDS - 1); j++)
			if (SortedBits[j]->Category < SortedBits[b]->Category)
				b = j;
		
		// Swap
		Bit = SortedBits[i];
		SortedBits[i] = SortedBits[b];
		SortedBits[b] = Bit;
	}
	
	// Sort by name now
	LastCat = 0;
	for (w = z = i = 0; i <= (PEXGSNUMBITIDS - 1); i++)
	{
		// Change of category? or ran out!
		if (i == (PEXGSNUMBITIDS - 1) || LastCat != SortedBits[i]->Category)
		{
			// Sort sub items
			for (j = w; j <= z; j++)
			{
				// Initial
				b = j;
		
				// Find next lowest
				for (k = j + 1; k <= z; k++)
					if (strcasecmp(SortedBits[k]->MenuTitle, SortedBits[b]->MenuTitle) < 0)
						b = k;
		
				// Swap
				Bit = SortedBits[j];
				SortedBits[j] = SortedBits[b];
				SortedBits[b] = Bit;
			}
			
			// Setup for future sort
			if (i < (PEXGSNUMBITIDS - 1))
				LastCat = SortedBits[i]->Category;
			w = z = i;
		}
		
		// Un-changed
		else
		{
			z++;	// Increase end spot
		}
	}
	
	/* Allocate */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Quick */
	New->NumItems = (PEXGSNUMBITIDS - 1) + NUMPEXGSMENUCATEGORIES;
	New->Items = Z_Malloc(sizeof(*New->Items) * New->NumItems, PU_STATIC, NULL);
	New->CleanerFunc = M_GenericCleanerFunc;
	
	/* Hack Up Variables */
	LastCat = 0;
	for (j = 0, i = 0; i < New->NumItems; i++)
	{
		// Don't park by default
		New->Items[i].Flags |= MUIIF_NOPARK;
		New->Items[i].Menu = New;
		
		// Overflow?
		if (j >= (PEXGSNUMBITIDS - 1))
			continue;
		
		// Get Bit
		Bit = SortedBits[j++];
		
		// No Category?
		if (Bit->Category == PEXGSMC_NONE)
			continue;
		
		// Category change?
		if (LastCat != Bit->Category)
		{
			// We want to put the header for the next category always
			LastCat = Bit->Category;
			
			// Print Category Header
			New->Items[i].Type = MUIIT_HEADER;
			//New->Items[i].Text = "*** CATEGORY ***";
			New->Items[i].TextRef = PTROFUNICODESTRING((DSTR_MENUGAMEVAR_CATNONE + LastCat));
			
			// Write on next item
			LastCat = Bit->Category;
			i++;
		}
		
		// Set
		New->Items[i].Text = Bit->MenuTitle;
		New->Items[i].Value = Bit->StrVal;
		New->Items[i].Flags &= ~MUIIF_NOPARK;	// Park here
		New->Items[i].LRValChangeFunc = MS_GameVar_LRValChange;
		New->Items[i].DataBits = Bit->BitID;
	}
	
	/* Cleanup */
	Z_Free(SortedBits);
	
	/* Return */
	return New;
}


