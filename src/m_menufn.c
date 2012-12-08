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
// Copyright (C) 2012-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Menu Code

/***************
*** INCLUDES ***
***************/

#include "m_menu.h"
#include "p_demcmp.h"
#include "p_local.h"
#include "r_data.h"
#include "p_info.h"

/****************
*** FUNCTIONS ***
****************/


/* MS_TextureTestUnderDraw() -- Menu Under Drawer */
static bool_t MS_TextureTestUnderDraw(const int32_t a_Player, struct M_UIMenuHandler_s* const a_Handler, struct M_UIMenu_s* const a_Menu, const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H)
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
static bool_t MS_GameVar_LRValChange(const int32_t a_PlayerID, struct M_UIMenu_s* const a_Menu, struct M_UIItem_s* const a_Item, const bool_t a_More)
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
					if (strcasecmp(DS_GetString(SortedBits[k]->MenuTitle), DS_GetString(SortedBits[b]->MenuTitle)) < 0)
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
		New->Items[i].TextRef = DS_GetStringRef(Bit->MenuTitle);
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

/***********************
*** COMBINED MENUING ***
***********************/

/*** SUB-MENU HANDLER FUNCTIONS ***/

/* MS_NEWGAME_t -- NewGame Data */
typedef struct MS_NEWGAME_s
{
	const char** NullRef;
	const char** TypeRef;
	const char** LevelRef;
} MS_NEWGAME_t;

/* MS_NEWGAME_Init() -- New Game Initialized */
static void MS_NEWGAME_Init(struct M_UIMenuHandler_s* const a_Handler, struct M_UIMenu_s* const a_UIMenu)
{
	MS_NEWGAME_t* pd;
	
	/* Get Private */
	pd = a_Handler->PrivateData;
	
	/* Set type references */
	pd->TypeRef = DS_GetStringRef(DSTR_MENUGENERAL_NEWGAMEISLOCAL);
	
	/* Setup Initial Level */
	pd->LevelRef = DS_GetStringRef(DSTR_MENUGENERAL_NEWGAMEISLOCAL);
	pd->TypeRef = DS_GetStringRef(DSTR_MENUGENERAL_NEWGAMEISLOCAL);
}

/*** GENERATION ***/

/* MS_AddNewItem() -- Adds item to menu */
static M_UIItem_t* MS_AddNewItem(M_UIMenu_t* a_Menu, const M_UIItemType_t a_Type, const uint32_t a_Flags, const int32_t a_Bits, const char** const a_TextRef, const char** const a_ValRef, M_UIItemLRValChangeFuncType_t a_LRValFunc, M_UIItemPressFuncType_t a_PressFunc)
{
	M_UIItem_t* NewItem;
	
	/* Check */
	if (!a_Menu)
		return NULL;
	
	/* Resize items in menu */
	Z_ResizeArray((void**)&a_Menu->Items, sizeof(*a_Menu->Items),
		a_Menu->NumItems, a_Menu->NumItems + 1);
	NewItem = &a_Menu->Items[a_Menu->NumItems++];
	
	/* Set Stuff in it */
	NewItem->Type = a_Type;
	NewItem->Flags = a_Flags;
	NewItem->DataBits = a_Bits;
	NewItem->TextRef = a_TextRef;
	NewItem->ValueRef = a_ValRef;
	NewItem->LRValChangeFunc = a_LRValFunc;
	NewItem->ItemPressFunc = a_PressFunc;
	
	/* Return it */
	return NewItem;
}

static M_UIMenu_t** l_PreMenus;					// Pre-Created Menus
static size_t l_NumPreMenus;					// Number of them

/* M_ExMenuIDByName() -- Returns menu ID by name */
int32_t M_ExMenuIDByName(const char* const a_Name)
{
	int32_t MenuID;
	
	/* Check */
	if (!a_Name)
		return l_NumPreMenus;
		
	/* Go through menus */
	for (MenuID = 0; MenuID < l_NumPreMenus; MenuID++)
		if (l_PreMenus[MenuID])
			if (strcasecmp(l_PreMenus[MenuID]->ClassName, a_Name) == 0)
				return MenuID;
	
	/* Not found */
	return l_NumPreMenus;
}

/* CLC_ExMakeMenuCom() -- Makes menu for someone */
int CLC_ExMakeMenuCom(const uint32_t a_ArgC, const char** const a_ArgV)
{
	int32_t PlayerID, MenuID;
	
	/* No enough args? */
	if (a_ArgC < 2)
	{
		// Show Usage
		
		// Show menus available
		for (MenuID = 0; MenuID < l_NumPreMenus; MenuID++)
		{
			if (!l_PreMenus[MenuID])
				continue;
			
			if (l_PreMenus[MenuID]->ClassName)
				CONL_PrintF("{4%s", l_PreMenus[MenuID]->ClassName);
			else
				CONL_PrintF("{4%i", MenuID);
			
			if (MenuID < l_NumPreMenus - 1)
				CONL_PrintF("{z, ");
		}
		CONL_PrintF("{z\n");
		
		// Failed
		return 0;
	}
	
	/* Player specified? */
	PlayerID = 0;
	if (a_ArgC >= 3)
		PlayerID = strtol(a_ArgV[2], NULL, 10);
	
	// Illegal?
	if (PlayerID < 0 || PlayerID >= MAXSPLITSCREEN)
		return 0;
	
	/* Determine Menu */
	// By Number?
	if (isdigit(a_ArgV[1][0]))
		MenuID = strtol(a_ArgV[1], NULL, 10);
	
	// By Name
	else
		MenuID = M_ExMenuIDByName(a_ArgV[1]);
	
	/* Check Menu */
	if (MenuID < 0 || MenuID >= l_NumPreMenus)
		return 0;
	
	/* Create Menu */
	return !!M_ExPushMenu(PlayerID, M_ExMakeMenu(MenuID, NULL));
}

/* M_ExMakeMenu() -- Creates a new menu */
M_UIMenu_t* M_ExMakeMenu(const int32_t a_MenuID, void* const a_Data)
{
	M_UIMenu_t* NewMenu;
	
	/* Check */
	if (a_MenuID < 0 || a_MenuID >= l_NumPreMenus)
		return NULL;
	
	/* Return Created Menu */
	return l_PreMenus[a_MenuID];

#if 0
	/* Init Base */
	NewMenu = Z_Malloc(sizeof(*NewMenu), PU_STATIC, NULL);
	
	/* Which Menu? */
	switch (a_MenuID)
	{
			// Hello World
		case MNMID_HELLO:
			// Set Title
			NewMenu->TitleRef = DS_GetStringRef(DSTR_MENUGENERAL_HELLOWORLD);
			
			// Create Items
			MS_AddNewItem(NewMenu, MUIIT_NORMAL, 0, 0, DS_GetStringRef(DSTR_MENUGENERAL_NEWGAMETYPE), NULL, NULL, NULL);
			break;
			
			// Create New Game
		case MNMID_NEWGAME:
			// Set Title
			NewMenu->TitleRef = DS_GetStringRef(DSTR_MENUNEWGAME_TITLE);
			NewMenu->InitFunc = MS_NEWGAME_Init;
			NewMenu->PrivateSize = sizeof(MS_NEWGAME_t);
			
			// Create Items
#define __GEN_NG(bit, flags, title, ref) MS_AddNewItem(NewMenu, MUIIT_NORMAL, flags, bit, DS_GetStringRef(title), offsetof(MS_NEWGAME_t, ref), NULL, NULL)
			__GEN_NG(0, 0, DSTR_MENUGENERAL_NEWGAMETYPE, TypeRef);
			__GEN_NG(0, MUIIF_NOPARK, DSTR_MENU_NULLSPACE, NullRef);
			__GEN_NG(0, 0, DSTR_MENUGENERAL_NEWGAMELEVEL, LevelRef);
#undef __GEN_NG
			
			break;
			
			// Unknown
		default:
			Z_Free(NewMenu);
			return NULL;
	}
	
	/* Set */
	l_PreMenus[a_MenuID] = NewMenu;
	
	/* Return the created menu */
	return NewMenu;
#endif
}

/***************************************************
*** REMOODAT/RMD_MENU -- DYNAMIC MENU GENERATION ***
***************************************************/

/*** STRUCTURES ***/

/* M_RDATInfo_t -- ReMooD Data Info */
typedef struct M_RDATInfo_s
{
	M_UIMenu_t* AMenu;
	int32_t AItem;
} M_RDATInfo_t;

/* M_MappedVars_t -- Multi-Variable Map */
typedef struct M_MappedVars_s
{
	const char* const Name;						// Mapping Name
	
	uint32_t Hash;								// Hash Code
	uint32_t HashMap[MAXSPLITSCREEN];			// Mapped Hashes
} M_MappedVars_t;

/*** LOCALS ***/

/* l_MappedVars -- Available mapped variables */
static M_MappedVars_t l_MappedVars[] =
{
	{"mvm_curprof1"},
	
	// End
	{NULL},
};

bool_t MS_CurProfBack(CONL_ConVariable_t* const, CONL_StaticVar_t* const);
bool_t MS_CurProfSlide(CONL_ConVariable_t* const, CONL_StaticVar_t* const, const int32_t);

CONL_StaticVar_t l_StaticMapVars[][MAXSPLITSCREEN] =
{
		// mvm_curprof -- Current Profile
	{{
			CLVT_STRING, NULL, 0/*CLVF_SAVE*/,
			"mvm_curprof1", 0, CLVVT_STRING, "guest",
			MS_CurProfBack, MS_CurProfSlide
	}},
	
	// END
	{{
			CLVT_STRING, NULL, 0,
			NULL
	}}
};

/*** FUNCTIONS ***/

/* MS_CurProfBack() -- Value changed */
bool_t MS_CurProfBack(CONL_ConVariable_t* const a_Var, CONL_StaticVar_t* const a_SVar)
{
	static bool_t Rec;
	D_ProfileEx_t* Prof;
	
	/* Prevent infinite recursion */
	// Say if guest does not exist!
	if (Rec)
		return false;
	Rec = true;
	
	/* Get current profile */
	Prof = D_FindProfileEx(a_SVar->Value->String);
	
	/* Does not exist? */
	if (!Prof)
		CONL_VarSetStr(a_SVar, "guest");
	
	/* Allow function to be called */
	Rec = false;
	return true;
}

/* MS_CurProfSlide() -- Value Slid */
bool_t MS_CurProfSlide(CONL_ConVariable_t* const a_Var, CONL_StaticVar_t* const a_SVar, const int32_t a_Right)
{
	D_ProfileEx_t* Prof, *NewProf;
	
	/* Get current profile */
	Prof = D_FindProfileEx(a_SVar->Value->String);
	
	// Not found?
	if (!Prof)
		Prof = D_FindProfileEx("guest");
	
	// Still nothing?
	if (!Prof)
		return false;
	
	/* Change based on direction (a simple rove) */
	if (a_Right > 0)
		NewProf = Prof->Next;
	else if (a_Right < 0)
		NewProf = Prof->Prev;
	
	/* If there is no next/prev, then calculate a loop */
	// Profiles are not a circularly linked list.
	if (!NewProf)
	{
		NewProf = Prof;
		
		// If moving left, find the one all the way to the right
		if (a_Right < 0)
			while (NewProf->Next)
				NewProf = NewProf->Next;
		
		// If moving right, find the one all the way to the left
		else if (a_Right > 0)
			while (NewProf->Prev)
				NewProf = NewProf->Prev;
	}
	
	/* No next/prev? No Change? */
	if (!NewProf || Prof == NewProf)
		return false;
	
	/* Set profile to new name */
	CONL_VarSetStr(a_SVar, NewProf->AccountName);
	return false;
}

/* M_DoMappedVars() -- Register menu variables */
void M_DoMappedVars(void)
{
#define BUFSIZE 64
	char Buf[BUFSIZE];
	char* End;
	int32_t i, j;
	
	/* Go through all variables listed */
	for (i = 0; l_StaticMapVars[i][0].VarName; i++)
	{
		// Copy name
		memset(Buf, 0, sizeof(Buf));
		strncpy(Buf, l_StaticMapVars[i][0].VarName, BUFSIZE - 1);
		
		// Get end of string
		End = Buf + (strlen(Buf) - 1);
		
		// Clone and setup variables
		for (j = MAXSPLITSCREEN; j > 0; j--)
		{
			// Copy from last
			if (j > 1)
			{
				// All the data
				memmove(&l_StaticMapVars[i][j - 1], &l_StaticMapVars[i][0],
					sizeof(l_StaticMapVars[i][0]));
				
				// Then clone the name
				*End = '0' + j;
				*((char**)&l_StaticMapVars[i][j - 1].VarName) = Z_StrDup(Buf, PU_STATIC, NULL);
			}
			
			// Register
			CONL_VarRegister(&l_StaticMapVars[i][j - 1]);
		}
	}
#undef BUFSIZE
}

/* MS_MapToSV() -- Gets map name to variable set */
static CONL_StaticVar_t* MS_MapToSV(const int32_t a_PlayerID, const char* const a_Name, const int32_t a_ID, M_MappedVars_t** const a_MapOut)
{
#define BUFSIZE 64
	char Buf[BUFSIZE];
	char* End;
	int32_t i, j;
	int32_t CheckID;
	
	static bool_t MTSVDone;
	static int32_t MTSVCount;
	
	/* Initialize Mappings? */
	if (!MTSVDone)
	{
		// Hash them all
		for (i = 0; l_MappedVars[i].Name; i++)
		{
			MTSVCount++;
			l_MappedVars[i].Hash = Z_Hash(l_MappedVars[i].Name);
			
			// Setup buffer
			for (j = 1; j <= MAXSPLITSCREEN; j++)
			{
				memset(Buf, 0, sizeof(Buf));
				strncpy(Buf, l_MappedVars[i].Name, BUFSIZE - 2);
				End = Buf + (strlen(Buf) - 1);
				*End = '0' + j;
				l_MappedVars[i].HashMap[j - 1] = Z_Hash(Buf);
			}
		}
		
		// Set as done
		MTSVDone = true;
	}
	
	/* Need ID? */
	if (a_Name || a_ID < 0)
	{
		// Look in mappings list
		CheckID = -1;
		for (i = 0; l_MappedVars[i].Name; i++)
			if (strcasecmp(a_Name, l_MappedVars[i].Name) == 0)
			{
				CheckID = i;
				break;
			}
	}
	
	// Don't need it
	else
		CheckID = a_ID;
	
	/* Illegal? */
	if (CheckID < 0 || CheckID >= MTSVCount || a_PlayerID < 0 || a_PlayerID >= MAXSPLITSCREEN)
		return NULL;
	
	/* Return explicit find */
	return CONL_VarLocateHash(l_MappedVars[CheckID].HashMap[a_PlayerID]);
#undef BUFSIZE
} 

/* MS_QuitResp() -- Quit the game */
void MS_QuitResp(const uint32_t a_MessageID, const M_ExMBType_t a_Response, const char** const a_TitleP, const char** const a_MessageP)
{
	/* Quitting? */
	if (a_Response & MEXMBT_YES)
		I_Quit();
}

/* M_ExMultiMenuCom() -- Multi-Menu Command */
int M_ExMultiMenuCom(const uint32_t a_ArgC, const char** const a_ArgV)
{
	int32_t i, j;
	P_LevelInfoEx_t* LInfo;
	CONL_StaticVar_t* SVar;
	D_ProfileEx_t* Prof;
	
	/* Quit Prompt */
	if (strcasecmp(a_ArgV[0], "m_quitprompt") == 0)
	{
		i = DSTR_DEP_QUITMSG + (M_Random() % (DSTR_DEP_QUIT2MSG6 - DSTR_DEP_QUITMSG));
		M_ExUIMessageBox(MEXMBT_YES | MEXMBT_NO, 1, DS_GetString(DSTR_MENUGENERAL_QUIT), DS_GetString(i), MS_QuitResp);
		return 0;
	}
	
	/* Start Classic Game selection */
	else if (strcasecmp(a_ArgV[0], "m_startclassic") == 0)
	{
		// Which player?
		if (a_ArgC > 1)
			i = strtol(a_ArgV[1], NULL, 10);
		else
			i = 0;
		
		// Doom
		if (g_CoreGame == COREGAME_DOOM)
			return !!M_ExPushMenu(i,
				M_ExMakeMenu(M_ExMenuIDByName("classicskilldoom"), NULL));
		
		// Heretic
		else if (g_CoreGame == COREGAME_HERETIC)
			return !!M_ExPushMenu(i,
				M_ExMakeMenu(M_ExMenuIDByName("classicskillheretic"), NULL));
		
		// Unknown
		else
			return 0;
	}
	
	/* After Skill Selection */
	else if (strcasecmp(a_ArgV[0], "m_startclassic2") == 0)
	{
		i = j = 0;
		
		// Which player?
		if (a_ArgC > 1)
			i = strtol(a_ArgV[1], NULL, 10);
		
		// Which Skill?
		if (a_ArgC > 2)
			j = strtol(a_ArgV[2], NULL, 10);
		
		// Doom, Episodes or straight game?
		if (g_CoreGame == COREGAME_DOOM)
			// Doom II
			if (g_IWADFlags & CIF_COMMERCIAL)
			{
				CONL_InputF("m_classicmap map01 %i\n", j);
				return 0;
			}
			
			// Ultimate Doom
			else if (g_IWADFlags & CIF_EXTENDED)
				return !!M_ExPushMenu(i,
					M_ExMakeMenu(M_ExMenuIDByName("episdoomud"), NULL));
			
			// Registered Doom	
			else if (g_IWADFlags & CIF_EXTENDED)
				return !!M_ExPushMenu(i,
					M_ExMakeMenu(M_ExMenuIDByName("episdoomrg"), NULL));
			
			// Shareware Doom	
			else
				return !!M_ExPushMenu(i,
					M_ExMakeMenu(M_ExMenuIDByName("episdoomsw"), NULL));
		
		// Heretic
		else if (g_CoreGame == COREGAME_HERETIC)
			// Shadow of the Serpent Riders
			if (g_IWADFlags & CIF_EXTENDED)
				return !!M_ExPushMenu(i,
					M_ExMakeMenu(M_ExMenuIDByName("episheresr"), NULL));
			
			// Registered Heretic	
			else if (g_IWADFlags & CIF_EXTENDED)
				return !!M_ExPushMenu(i,
					M_ExMakeMenu(M_ExMenuIDByName("epishererg"), NULL));
			
			// Shareware Heretic	
			else
				return !!M_ExPushMenu(i,
					M_ExMakeMenu(M_ExMenuIDByName("episheresw"), NULL));
		
		// Unknown
		else
			return 0;
	}
	
	/* Start Classic Map */
	else if (strcasecmp(a_ArgV[0], "m_classicmap") == 0)
	{
		i = 0;
		
		// Skill Specified?
		if (a_ArgC > 2)
			i = strtol(a_ArgV[2], NULL, 10);
		
		// No map?
		if (a_ArgC < 2)
			return 1;
		
		// Locate Level
		LInfo = P_FindLevelByNameEx(a_ArgV[1], NULL);
		
		// Not found?
		if (!LInfo)
			return 1;
		
		// Do the runs for a local game
		for (j = 0; j < MAXSPLITSCREEN; j++)
			M_ExPopAllMenus(j);
		D_XNetDisconnect(false);
		P_XGSSetAllDefaults();
		D_XNetMakeServer(false, 0);
		D_XNetChangeMap(a_ArgV[1]);
		
		// It worked, hopefully
		return 0;
	}
	
	/* Use currently selected profile */
	else if (strcasecmp(a_ArgV[0], "m_usethisprof") == 0)
	{
		i = -1;
		
		// Player Specified?
		if (a_ArgC > 1)
			i = strtol(a_ArgV[1], NULL, 10);
		
		// Bad player?
		if (i < 0 || i >= MAXSPLITSCREEN)
			return 1;
			
		// Find profile name to use
		SVar = MS_MapToSV(i, "mvm_curprof1", -1, NULL);
		
		// Set player to use this profile, if found
		if (SVar)
		{
			Prof = D_FindProfileEx(SVar->Value->String);
			
			if (Prof)
			{
				D_XNetChangeLocalProf(i, Prof);
				M_ExPopAllMenus(i);
			}
			return 0;
		}
		
		return 1;
	}
	
	/* Unknown */
	else
		return 1;
	
	/* Fall through? */
	return 2;
}

/* MS_Gen_SubMenu_Press() -- Generic Sub-Menu Press */
static bool_t MS_Gen_SubMenu_Press(const int32_t a_PlayerID, struct M_UIMenu_s* const a_Menu, struct M_UIItem_s* const a_Item)
{
	int32_t NewMenu;
	
	/* Find new menu, possibly */
	NewMenu = M_ExMenuIDByName(a_Item->PressVal);
	
	// Invalid?
	if (NewMenu < 0 || NewMenu >= l_NumPreMenus)
		return false;
	
	return !!M_ExPushMenu(a_PlayerID, M_ExMakeMenu(NewMenu, NULL));
}

/* MS_Gen_Console_Press() -- Executes console command */
static bool_t MS_Gen_Console_Press(const int32_t a_PlayerID, struct M_UIMenu_s* const a_Menu, struct M_UIItem_s* const a_Item)
{
#define BUFSIZE 128
	char Buf[BUFSIZE];
	char* o, *i;
	int32_t n;
	
	/* Generate String */
	memset(Buf, 0, sizeof(Buf));
	for (o = Buf, i = a_Item->PressVal, n = 0; i && *i && n < BUFSIZE - 1; n++, i++, o++)
	{
		// Special Variable
		if (*i == '@')
		{
		}
		
		// Player ID
		else if (*i == '#')
			*o = '0' + a_PlayerID;
		
		// Normal Text
		else
			*o = *i;
	}
	
	/* Send to console */
	CONL_InputF("%s\n", Buf);
	
	/* Success */
	return true;
#undef BUFSIZE
}

/* MS_SVarForValue() -- Gets static var for value */
static CONL_StaticVar_t* MS_SVarForValue(const int32_t a_PlayerID, const char* const a_Value, const intptr_t a_ValInt)
{
	// Is mapped variable
	if (*a_Value == '~')
		return MS_MapToSV(a_PlayerID, NULL, a_ValInt, NULL);
	
	// Is variable?
	else if (*a_Value == '$')
		return CONL_VarLocateHash(a_ValInt);
	
	// Nothing
	else
		return NULL;
}

/* MS_Gen_Variable_Slide() -- Slide variable, left/right */
static bool_t MS_Gen_Variable_Slide(const int32_t a_PlayerID, struct M_UIMenu_s* const a_Menu, struct M_UIItem_s* const a_Item, const bool_t a_More)
{
	CONL_StaticVar_t* SVar;
	
	/* Attempt finding variable */
	SVar = MS_SVarForValue(a_PlayerID, a_Item->DrawVal, a_Item->DrawValInt);
	
	// If not found, do nothing
	if (!SVar)
		return false;
	
	// Slide value
	return CONL_VarSlideValue(SVar, (a_More ? 1 : -1));
}

/* MS_Gen_Variable_Value() -- Obtain variable string */
static bool_t MS_Gen_Variable_Value(const int32_t a_PlayerID, struct M_UIMenu_s* const a_Menu, struct M_UIItem_s* const a_Item, const char** const a_ValOut)
{
#define BUFSIZE 8
	char Buf[BUFSIZE];
	CONL_StaticVar_t* SVar;
	
	/* Missing draw value */
	if (!a_Item->DrawVal)
	{
		snprintf(Buf, BUFSIZE, "%i", a_Item->DrawValInt);
		*a_ValOut = Buf;
	}
	
	/* Has a draw value */
	else
	{
		SVar = MS_SVarForValue(a_PlayerID, a_Item->DrawVal, a_Item->DrawValInt);
		
		// Found variable?
		if (SVar)
			*a_ValOut = SVar->Value->String;
		
		// Not found
		else
			*a_ValOut = a_Item->DrawVal;
	}
	
	/* Success! */
	return true;
#undef BUFSIZE
}

/* MS_ValToInt() -- Returns value from Int */
static intptr_t MS_ValToInt(const char* const a_Str)
{
	int32_t i;
	
	/* Check */
	if (!a_Str)
		return 0;
	
	/* If it starts with ~, it is mapped to a mapped set */
	if (*a_Str == '~')
	{
		for (i = 0; l_MappedVars[i].Name; i++)
			if (strcasecmp(a_Str + 1, l_MappedVars[i].Name) == 0)
				return i;
		return -1;
	}
	
	/* If it starts with $, it is mapped to a single var */
	else if (*a_Str == '$')
		return Z_Hash(a_Str + 1);
	
	/* Otherwise, it is an integer */
	else
		return strtol(a_Str, NULL, 10);
}

/* M_MenuDataKeyer() -- Handles menus */
bool_t M_MenuDataKeyer(void** a_DataPtr, const int32_t a_Stack, const D_RMODCommand_t a_Command, const char* const a_Field, const char* const a_Value)
{
#define THISMENU ((*InfoPP)->AMenu)
#define THISITEM (&THISMENU->Items[(*InfoPP)->AItem])
	M_RDATInfo_t** InfoPP;
	int32_t i;
	const char** SRef;
	
	/* Get Active Menu */
	InfoPP = NULL;
	if (a_DataPtr)
		InfoPP = a_DataPtr;
	
	/* Which Command? */
	switch (a_Command)
	{
			// Opening {
		case DRC_OPEN:
			// Creating Fresh Menu?
			if (a_Stack == 1 && strcasecmp(a_Field, "Menu") == 0)
			{
				// Switch to menu
				i = M_ExMenuIDByName(a_Value);
				
				// Legal?
				if (i >= 0 && i < l_NumPreMenus)
					THISMENU = l_PreMenus[i];
				
				// Otherwise, append to the list
				else
				{
					Z_ResizeArray((void**)&l_PreMenus, sizeof(*l_PreMenus),
						l_NumPreMenus, l_NumPreMenus + 1);
					THISMENU = l_PreMenus[l_NumPreMenus++] =
						Z_Malloc(sizeof(*THISMENU), PU_MENUDAT, NULL);
						
					// Set class name
					THISMENU->ClassName = Z_StrDup(a_Value, PU_MENUDAT, NULL);
				}
			}
			
			// Creating Menu Item?
			else if (a_Stack == 2 && strcasecmp(a_Field, "Item") == 0)
			{
				// See if item already exists
				for (i = 0; i < THISMENU->NumItems; i++)
					if (strcasecmp(a_Value, THISMENU->Items[i].ClassName) == 0)
						break;
				
				// Not found?
				if (i >= THISMENU->NumItems)
				{
					Z_ResizeArray((void**)&THISMENU->Items,
						sizeof(*THISMENU->Items),
						THISMENU->NumItems, THISMENU->NumItems + 1);
					
					// Select this one now
					(*InfoPP)->AItem = THISMENU->NumItems++;
					
					// Set class name
					THISITEM->ClassName = Z_StrDup(a_Value, PU_MENUDAT, NULL);
				}
				
				// Select here
				else
					(*InfoPP)->AItem = &THISMENU->Items[i];
			}
			
			return true;
			
			// Closing }
		case DRC_CLOSE:
			// Close Item
			if (a_Stack == 2)
			{
				(*InfoPP)->AItem = -1;
			}
			
			// Close Menu
			else if (a_Stack == 1)
			{
				THISMENU = NULL;
			}
			return true;
			
			// Data Entry
		case DRC_DATA:
			// Menu Properties
			if (a_Stack == 1 && THISMENU)
			{
				// Menu Picture?
				if (strcasecmp("TitlePic", a_Field) == 0)
				{
					THISMENU->TitlePic = V_ImageFindA(a_Value, 0);
				}
				
				// Menu Title?
				else if (strcasecmp("Title", a_Field) == 0)
				{
					SRef = DS_FindStringRef(a_Value);
					
					// Reference found?
					if (SRef)
						THISMENU->TitleRef = SRef;
					
					// Otherwise, direct string
					else
					{
						if (THISMENU->Title)
							Z_Free(THISMENU->Title);
						THISMENU->Title = Z_StrDup(a_Value, PU_MENUDAT, NULL);
					}
				}
			}
			
			// Item Properties
			else if (a_Stack == 2 && (*InfoPP)->AItem != -1)
			{
				// Item Text?
				if (strcasecmp("Text", a_Field) == 0)
				{
					SRef = DS_FindStringRef(a_Value);
					
					// Reference found?
					if (SRef)
						THISITEM->TextRef = SRef;
					
					// Otherwise, direct string
					else
					{
						if (THISITEM->Text)
							Z_Free(THISITEM->Text);
						THISITEM->Text = Z_StrDup(a_Value, PU_MENUDAT, NULL);
					}
				}
				
				// Omni Function?
				else if (strcasecmp("OmniFunc", a_Field) == 0)
				{
					M_MenuDataKeyer(a_DataPtr, a_Stack, a_Command,
							"SelectFunc", a_Value);
					M_MenuDataKeyer(a_DataPtr, a_Stack, a_Command,
							"DrawValueFunc", a_Value);
					M_MenuDataKeyer(a_DataPtr, a_Stack, a_Command,
							"SlideFunc", a_Value);
				}
				
				// Omni Value?
				else if (strcasecmp("OmniVal", a_Field) == 0)
				{
					M_MenuDataKeyer(a_DataPtr, a_Stack, a_Command,
							"SelectVal", a_Value);
					M_MenuDataKeyer(a_DataPtr, a_Stack, a_Command,
							"DrawValueVal", a_Value);
					M_MenuDataKeyer(a_DataPtr, a_Stack, a_Command,
							"SlideVal", a_Value);
				}
				
				// Sliding?
				else if (strcasecmp("SlideFunc", a_Field) == 0)
				{
					if (strcasecmp("Variable", a_Value) == 0)
						THISITEM->LRValChangeFunc = MS_Gen_Variable_Slide;
					
					// Illegal
					else
						THISITEM->LRValChangeFunc = NULL;
				}
				
				// Slide Value?
				else if (strcasecmp("SlideVal", a_Field) == 0)
				{
					if (THISITEM->SlideVal)
						Z_Free(THISITEM->SlideVal);
					
					// Copy string
					THISITEM->SlideVal = Z_StrDup(a_Value, PU_MENUDAT, NULL);
					THISITEM->SlideValInt = MS_ValToInt(a_Value);
				}
				
				// Item Function?
				else if (strcasecmp("SelectFunc", a_Field) == 0)
				{
					// Call sub-menu
					if (strcasecmp("SubMenu", a_Value) == 0)
						THISITEM->ItemPressFunc = MS_Gen_SubMenu_Press;
						
					// Execute Console Command
					else if (strcasecmp("Console", a_Value) == 0)
						THISITEM->ItemPressFunc = MS_Gen_Console_Press;
					
					// Illegal
					else
						THISITEM->ItemPressFunc = NULL;
				}
				
				// Item Value?
				else if (strcasecmp("SelectVal", a_Field) == 0)
				{
					if (THISITEM->PressVal)
						Z_Free(THISITEM->PressVal);
					
					// Copy string
					THISITEM->PressVal = Z_StrDup(a_Value, PU_MENUDAT, NULL);
					THISITEM->PressValInt = MS_ValToInt(a_Value);
				}
				
				// Draw Value Function?
				else if (strcasecmp("DrawValueFunc", a_Field) == 0)
				{
					// Menu Variable
					if (strcasecmp("Variable", a_Value) == 0)
						THISITEM->ValueFunc = MS_Gen_Variable_Value;
					
					// Illegal
					else
						THISITEM->ValueFunc = NULL;
				}
				
				// Draw Value?
				else if (strcasecmp("DrawValueVal", a_Field) == 0)
				{
					if (THISITEM->DrawVal)
						Z_Free(THISITEM->DrawVal);
					
					// Copy string
					THISITEM->DrawVal = Z_StrDup(a_Value, PU_MENUDAT, NULL);
					THISITEM->DrawValInt = MS_ValToInt(a_Value);
				}
				
				// Disabled?
				else if (strcasecmp("Disabled", a_Field) == 0)
				{
					THISITEM->Flags &= ~MUIIF_DISABLED;
					
					if (INFO_BoolFromString(a_Value))
						THISITEM->Flags |= MUIIF_DISABLED;
				}
				
				// No Parking?
				else if (strcasecmp("NoPark", a_Field) == 0)
				{
					THISITEM->Flags &= ~MUIIF_NOPARK;
					
					if (INFO_BoolFromString(a_Value))
						THISITEM->Flags |= MUIIF_NOPARK;
				}
			}
			
			return true;
			
			// Initialize
		case DRC_INIT:
			(*InfoPP) = Z_Malloc(sizeof(*(*InfoPP)), PU_MENUDAT, NULL);
			return true;
			
			// Finalize
		case DRC_FINAL:
			Z_Free((*InfoPP));
			return true;
			
			// First Time
		case DRC_FIRST:
			// Delete all menus
			if (l_PreMenus)
			{
				// Delete inner defined menus
				for (i = 0; i < l_NumPreMenus; i++)
					if (l_PreMenus[i])
						Z_Free(l_PreMenus[i]);
				
				// Free
				Z_Free(l_PreMenus);
				l_PreMenus = NULL;
				l_NumPreMenus = 0;
			}
			
			// Free Tags
			Z_FreeTags(PU_MENUDAT, PU_MENUDAT);
			return true;
			
			// Last Time
		case DRC_LAST:
			return true;
			
			// Unknown
		default:
			return false;
	}
}


