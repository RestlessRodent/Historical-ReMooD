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
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION:
//      parse and execute commands from console input/scripts.
//
//      handles console variables, which is a simplified version
//      of commands, each consvar can have a function called when
//      it is modified.. thus it acts nearly as commands.
//

/***************
*** INCLUDES ***
***************/

#include "doomdef.h"
#include "doomstat.h"

#include "console.h"
#include "z_zone.h"
#include "d_clisrv.h"
#include "d_netcmd.h"
#include "m_misc.h"
#include "m_fixed.h"
#include "p_saveg.h"
#include "p_demcmp.h"

/**********************
*** UPPER CONSTANTS ***
**********************/

/*** COMMANDS ***/

#define MAXCONLCOMMANDNAME		128							// Max name for console command

/*** VARIABLES ***/

/*****************
*** STRUCTURES ***
*****************/

/*** COMMANDS ***/

/* CONL_ConCommand_t -- Console command */
typedef struct CONL_ConCommand_s
{
	char Name[MAXCONLCOMMANDNAME];							// Name of command
	int (*ComFunc)(const uint32_t, const char** const);	// Function to call
	uint32_t Hash;											// Hash table hash
} CONL_ConCommand_t;

/*** VARIABLES ***/

/* CONL_ConVariable_s -- Console variable */
struct CONL_ConVariable_s
{
	/* Static */
	CONL_StaticVar_t* StaticLink;				// Link to static variable
	CONL_ConVariable_t* AliasTo;				// Alias to another variable
	
	/* Virtualization and Config */
	bool_t IsVirtual;							// A virtual variable (not regged)
	bool_t LoadedValue;							// Was set in the config file
	bool_t IsDeprecated;						// Is really a deprecated variable
	char* VirtualValue;							// Virtual Value
	
	/* Value */
	CONL_VarValue_t Value;						// Value of variable (actual)
	char* Name;									// Name
	uint32_t Hash;								// Hash of name
	
	/* Bar Graph */
	int32_t BarCaps[2];							// Bar caps
};

/**********************
*** LOWER CONSTANTS ***
**********************/

/*** COMMANDS ***/

/*** VARIABLES ***/

// g_CVPVClamp -- Clamped Integer
const CONL_VarPossibleValue_t c_CVPVClamp[] =
{
	// End
	{0, "MINVAL"},
	{1, "MAXVAL"},
	{0, NULL},
};

// g_CVPVInteger -- Signed Integer
const CONL_VarPossibleValue_t c_CVPVInteger[] =
{
	// End
	{-32767, "MINVAL"},
	{32767, "MAXVAL"},
	{0, NULL},
};

// g_CVPVPositive -- Positive Integer
const CONL_VarPossibleValue_t c_CVPVPositive[] =
{
	// End
	{0, "MINVAL"},
	{32767, "MAXVAL"},
	{0, NULL},
};

// g_CVPVNegative -- Negative Integer
const CONL_VarPossibleValue_t c_CVPVNegative[] =
{
	// End
	{-32767, "MINVAL"},
	{0, "MAXVAL"},
	{0, NULL},
};

// g_CVPVBoolean -- Boolean
const CONL_VarPossibleValue_t c_CVPVBoolean[] =
{
	// False
	{0, "no"},
	{0, "off"},
	{0, "false"},
	
	// True
	{1, "yes"},
	{1, "on"},
	{1, "true"},
	
	// End
	{0, "MINVAL"},
	{1, "MAXVAL"},
	{0, NULL},
};

// c_CVPVVexColor -- VEX_MAP_ Color
const CONL_VarPossibleValue_t c_CVPVVexColor[] =
{
	{0, "None"},								// VEX_MAP_NONE
	{1, "Red"},									// VEX_MAP_RED
	{2, "Orange"},								// VEX_MAP_ORANGE
	{3, "Yellow"},								// VEX_MAP_YELLOW
	{4, "Green"},								// VEX_MAP_GREEN
	{5, "Cyan"},								// VEX_MAP_CYAN
	{6, "Blue"},								// VEX_MAP_BLUE
	{7, "Magenta"},								// VEX_MAP_MAGENTA
	{8, "Brown"},								// VEX_MAP_BROWN
	{9, "BrightWhite"},							// VEX_MAP_BRIGHTWHITE
	{10, "White"},								// VEX_MAP_WHITE
	{11, "Gray"},								// VEX_MAP_GRAY
	{12, "Black"},								// VEX_MAP_BLACK
	{13, "Fuscia"},								// VEX_MAP_FUSCIA
	{14, "Gold"},								// VEX_MAP_GOLD
	{15, "TekGreen"},							// VEX_MAP_TEKGREEN
	
	// End
	{0, "MINVAL"},
	{15, "MAXVAL"},
	{0, NULL},
};

// c_CVPVFont -- A Font
const CONL_VarPossibleValue_t c_CVPVFont[] =
{
	{0, "Small"},								// VFONT_SMALL
	{1, "Large"},								// VFONT_LARGE
	{2, "StatusBarSmall"},						// VFONT_STATUSBARSMALL
	{3, "Boom"},								// VFONT_BOOMHUD
	{4, "OEM"},									// VFONT_OEM
	{5, "UserA"},								// VFONT_USERSPACEA
	{6, "UserB"},								// VFONT_USERSPACEB
	{7, "UserC"},								// VFONT_USERSPACEC
	{8, "UserD"},								// VFONT_USERSPACED
	{9, "DoomSmall"},							// VFONT_SMALL_DOOM
	{10, "DoomLarge"},							// VFONT_LARGE_DOOM
	{11, "HereticSmall"},						// VFONT_SMALL_HERETIC
	{12, "HereticLarge"},						// VFONT_LARGE_HERETIC
	{13, "StatusBarLarge"},						// VFONT_STATUSBARLARGE
	{14, "NewUI"},								// VFONT_NEWUI
	
	// End
	{0, "MINVAL"},
	{14, "MAXVAL"},
	{0, NULL},
};

/**************
*** GLOBALS ***
**************/

/*** COMMANDS ***/

/*** VARIABLES ***/

/*************
*** LOCALS ***
*************/

/*** COMMANDS ***/

static int g_CONLError = 0;			// '? command' Exit Code

static CONL_ConCommand_t* l_CONLCommands = NULL;			// Console commands
static size_t l_CONLNumCommands = 0;						// Number of commands
static Z_HashTable_t* l_CONLCommandHashes = NULL;			// Speed lookup

/*** VARIABLES ***/

static CONL_ConVariable_t** l_CONLVariables = NULL;			// Console variables
static size_t l_CONLNumVariables = 0;						// Number of variables
static Z_HashTable_t* l_CONLVariableHashes = NULL;			// Speed lookup

static bool_t l_CONLVarLoaded = false;						// Loaded config

/**************
*** STATICS ***
**************/

/*** COMMANDS ***/

/* CONL_CommandHashCompare() -- Compares entry hash */
// A = const char*
// B = uintptr_t
static bool_t CONL_CommandHashCompare(void* const a_A, void* const a_B)
{
	/* Check */
	if (!a_A || (((uintptr_t)a_B) - 1) >= l_CONLNumCommands)
		return false;
	
	/* Caseless compare */
	if (strcasecmp(a_A, l_CONLCommands[((uintptr_t)a_B) - 1].Name) == 0)
		return true;
	
	/* No match */
	return false;
};

/*** VARIABLES ***/

/* CONL_VariableHashCompare() -- Compares entry hash */
// A = const char*
// B = uintptr_t
static bool_t CONL_VariableHashCompare(void* const a_A, void* const a_B)
{
	const char* A;
	CONL_ConVariable_t* B;
	
	/* Check */
	if (!a_A || !a_B)
		return false;
	
	/* Get normals */
	A = a_A;
	B = a_B;
	
	/* Caseless compare */
	if (strcasecmp(A, B->Name) == 0)
		return true;
	
	/* Not a match */
	return false;
}

/* CONLS_VarCoreLocate() -- Locate internal variable by name */
static CONL_ConVariable_t* CONLS_VarCoreLocate(const char* const a_Name)
{
	uint32_t Hash;
	CONL_ConVariable_t* Found;
	
	/* Does the hash table need initialization? */
	if (!l_CONLVariableHashes)
	{
		// Create table
		l_CONLVariableHashes = Z_HashCreateTable(CONL_VariableHashCompare);
		
		// Register variable commands
		CONL_AddCommand("cvlist", CLC_CVarList);
		CONL_AddCommand("cvset", CLC_CVarSet);
	}
	
	/* Check */
	if (!a_Name)
		return NULL;
	
	/* Hash name */
	Hash = Z_Hash(a_Name);
	
	/* Find in table */
	Found = Z_HashFindEntry(l_CONLVariableHashes, Hash, (void*)a_Name, false);
	
	/* Return result (if found) */
	return Found;
}

/* CONLS_VarCoreLocateHash() -- Locates internal variable by hash */
static CONL_ConVariable_t* CONLS_VarCoreLocateHash(const uint32_t a_Hash, const bool_t a_OnlyRegged)
{
	size_t i;
	
	/* Long loop */
	for (i = 0; i < l_CONLNumVariables; i++)
		if (l_CONLVariables[i])
			if (l_CONLVariables[i]->Hash == a_Hash)
			{
				// If we want only registered ones, but this one is not...
				if (a_OnlyRegged)
					if (l_CONLVariables[i]->IsVirtual)
						continue;
				
				// Otherwise, return this one
				return l_CONLVariables[i];
			}	
	
	/* Not found */
	return NULL;
}

/* CONLS_PushVar() -- Pushes a new variable */
static CONL_ConVariable_t* CONLS_PushVar(const char* const a_Name, CONL_StaticVar_t* const a_StaticVar)
{
	uint32_t Hash;
	CONL_ConVariable_t* NewVar;
	
	/* Check */
	if (!a_Name)
		return NULL;
	
	/* Hash Name */
	Hash = Z_Hash(a_Name);
	
	/* Create space for variable and push to end of list */
	// Create
	NewVar = Z_Malloc(sizeof(*NewVar), PU_STATIC, NULL);
	
	// Push
	Z_ResizeArray((void**)&l_CONLVariables, sizeof(*l_CONLVariables), l_CONLNumVariables, l_CONLNumVariables + 1);
	l_CONLVariables[l_CONLNumVariables++] = NewVar;
	
	// Add to hash table
	Z_HashAddEntry(l_CONLVariableHashes, Hash, (void*)NewVar);
	
	/* Set variable info */
	NewVar->Name = Z_StrDup(a_Name, PU_STATIC, NULL);
	NewVar->Hash = Hash;
	NewVar->StaticLink = a_StaticVar;
	NewVar->LoadedValue = l_CONLVarLoaded;
	
	// If there is a static link...
	if (NewVar->StaticLink)
	{
		// Set value stuff
		NewVar->StaticLink->Value = &NewVar->Value;
		
		// And real link
		NewVar->StaticLink->RealLink = NewVar;
	}
	
	/* Return it */
	return NewVar;
}

/****************
*** FUNCTIONS ***
****************/

/*** COMMANDS ***/

/* CONL_ExitCodeToStr() -- Converts exit code to string */
const char* CONL_ExitCodeToStr(const int a_Code)
{
	static const char* const CodeStrs[2] =
	{
		"Success",				// 0
		"Failure",				// 1
	};
	
	/* Return static char */
	if (a_Code >= 2 || a_Code < 0)
		return CodeStrs[1];
	return CodeStrs[a_Code];
}

/* CONL_AddCommand() -- Add console command */
bool_t CONL_AddCommand(const char* const a_Name, int (*a_ComFunc)(const uint32_t, const char** const))
{
	uintptr_t x;
	
	/* Check */
	if (!a_Name || !a_ComFunc)
		return false;
	
	/* Check if already registered */
	if (l_CONLCommands)
		for (x = 0; x < l_CONLNumCommands; x++)
			if (strcasecmp(l_CONLCommands[x].Name, a_Name) == 0)
				return false;	// Whoops!
	
	/* Resize command structure */
	Z_ResizeArray((void**)&l_CONLCommands, sizeof(*l_CONLCommands), l_CONLNumCommands, l_CONLNumCommands + 1);
	
	/* Set in command structure */
	strncpy(l_CONLCommands[l_CONLNumCommands].Name, a_Name, MAXCONLCOMMANDNAME);
	l_CONLCommands[l_CONLNumCommands].ComFunc = a_ComFunc;
	l_CONLCommands[l_CONLNumCommands].Hash = Z_Hash(l_CONLCommands[l_CONLNumCommands].Name);
	
	/* Create hash table */
	// No table yet created?
	if (!l_CONLCommandHashes)
		l_CONLCommandHashes = Z_HashCreateTable(CONL_CommandHashCompare);
	
	// Add to hash
	Z_HashAddEntry(l_CONLCommandHashes, l_CONLCommands[l_CONLNumCommands].Hash, (void*)(l_CONLNumCommands + 1));
	
	/* Increment */
	l_CONLNumCommands++;
	
	/* Success */
	return true;
}

/* CONL_Exec() -- Execute command */
int CONL_Exec(const uint32_t a_ArgC, const char** const a_ArgV)
{
	uint32_t Hash;
	uintptr_t ComNum;
	CONL_ConVariable_t* VarFind;
	P_XGSVariable_t* GameVar;
	char* GiantString;
	int32_t i, n, w, RetVal;
	char OldVarVal[PEXGSSTRBUFSIZE];
	
	/* Check */
	if (!a_ArgC || !a_ArgV)
		return 1;
	
	/* Find hash for command */
	Hash = Z_Hash(a_ArgV[0]);
	
	/* Find command for this hash */
	ComNum = (uintptr_t)Z_HashFindEntry(l_CONLCommandHashes, Hash, (const void*)a_ArgV[0], false);
	ComNum -= 1;
	
	// Check, if not found try variables
	if (ComNum >= l_CONLNumCommands)
	{
		// Return failure by default
		RetVal = 1;
		
		// Variable and game vars only really accept a single input, so concat all argvs
		// together into one giant string
		GiantString = NULL;
		for (i = 1, n = 0; i < a_ArgC; i++)
		{
			// Get Length
			w = strlen(a_ArgV[i]) + 3;
			
			// Resize
			Z_ResizeArray((void**)&GiantString, sizeof(*GiantString),
				n, n + w);
			
			// Concat
			n += w;
			strncat(GiantString, a_ArgV[i], n - 1);
			
			// Add extra space?
			if (i + 1 < a_ArgC)
				strncat(GiantString, " ", n - 1);
		}
		
		// Try finding variable
		VarFind = (CONL_ConVariable_t*)Z_HashFindEntry(l_CONLVariableHashes, Hash, (const void*)a_ArgV[0], false);
		
		// Found variable (and only registered ones)
		if (VarFind && VarFind->StaticLink)
		{
			// Print value if name was just said
			if (a_ArgC == 1)
				CONL_PrintF("%s: %s\n", VarFind->StaticLink->VarName, VarFind->StaticLink->Value->String);
			
			// Otherwise, set value
			else
				CONL_VarSetStr(VarFind->StaticLink, GiantString);
			
			// Always exit as successful
			RetVal = 0;
		}
		
		// Try game variable instead
		else
		{
			GameVar = P_XGSVarForName(a_ArgV[0]);
		
			// Found
			if (GameVar)
			{
				// Print value of variable if only variable said
				if (a_ArgC == 1)
					CONL_PrintF("%s: %s\n", GameVar->Name, GameVar->StrVal);
				
				// Otherwise set variable value
				else
				{
					strncpy(OldVarVal, GameVar->StrVal, PEXGSSTRBUFSIZE - 1);
					OldVarVal[PEXGSSTRBUFSIZE - 1] = 0;
					
					P_XGSSetValueStr(false, GameVar->BitID, GiantString);
					
					CONL_PrintF("%s changed from \"%s\" to \"%s\".\n",
							GameVar->Name,
							OldVarVal,
							GiantString//GameVar->StrVal
						);
				}
					
				// Always exit as successful
				RetVal = 0;
			}
		}
		
		// Cleanup and quit
		if (GiantString)
			Z_Free(GiantString);
		return RetVal;	// not found
	}
	
	/* Execute */
	return l_CONLCommands[ComNum].ComFunc(a_ArgC, a_ArgV);
}

/*** VARIABLES ***/

/* CONL_VarSetLoaded() -- Sets loaded value */
bool_t CONL_VarSetLoaded(const bool_t a_Loaded)
{
	bool_t Old = l_CONLVarLoaded;
	
	/* Set */
	l_CONLVarLoaded = a_Loaded;
	
	/* Return Old */
	return Old;
}

/* CONL_VarRegister() -- Registers a variable */
CONL_ConVariable_t* CONL_VarRegister(CONL_StaticVar_t* const a_StaticVar)
{
	CONL_ConVariable_t* NewVar;
	CONL_ConVariable_t* FoundVar;
	uint32_t Hash;
	
	/* Check */
	if (!a_StaticVar)
		return NULL;
	
	/* Sanity */
	if (a_StaticVar->Type < 0 || a_StaticVar->Type >= NUMCONLVARIABLETYPES || !a_StaticVar->VarName || !a_StaticVar->DefaultValue)
		return NULL;
	
	// String types do not require possible values, but other types do
	if (a_StaticVar->Type != CLVT_STRING && !a_StaticVar->Possible)
		return NULL;
	
	/* See if varaible is already taken, by hash */
	// Hash name
	Hash = Z_Hash(a_StaticVar->VarName);
	
	// Try to find it
	FoundVar = CONLS_VarCoreLocateHash(Hash, true);
	
	// Already found?
	if (FoundVar)
	{
		CONL_OutputUT(CT_CONSOLE, DSTR_COMMANDC_WOULDHASHCOLLIDE, "%s%s\n",
				a_StaticVar->VarName, FoundVar->Name
			);
		return NULL;
	}
	
	/* Locate variable to see if it is virtualized or registered */
	if ((NewVar = CONLS_VarCoreLocate(a_StaticVar->VarName)))
	{
		// De-virtualize
		NewVar->IsVirtual = false;
		
		// Set static stuff
		NewVar->StaticLink = a_StaticVar;
		NewVar->StaticLink->Value = &NewVar->Value;
		NewVar->StaticLink->RealLink = NewVar;
		
		// Value was set to something before config was loaded "+something"?
		// If the value is loaded from config
		if ((!NewVar->LoadedValue && NewVar->VirtualValue) || (NewVar->LoadedValue && NewVar->VirtualValue))
		{
			// Set with it
			CONL_VarSetStr(NewVar->StaticLink, NewVar->VirtualValue);
			
			// Clear
			Z_Free(NewVar->VirtualValue);
			NewVar->VirtualValue = NULL;
			NewVar->LoadedValue = false;	// In case the var is not saved
		}
		
		// Otherwise, set it with the default value
		else
		{
			// Since it was never loaded with a config, we want the default
			CONL_VarSetStr(NewVar->StaticLink, NewVar->StaticLink->DefaultValue);
		}
		
		
		// Return the same variable
		return NewVar;
	}
	
	/* Obtain a new variable */
	NewVar = CONLS_PushVar(a_StaticVar->VarName, a_StaticVar);
	
	// Set with default value
	CONL_VarSetStr(NewVar->StaticLink, NewVar->StaticLink->DefaultValue);
	
	/* Success */
	return NewVar;
}

/* CONL_StaticVarByNum() -- Return variable by number */
bool_t CONL_StaticVarByNum(const size_t a_Num, CONL_StaticVar_t** const a_VarP)
{
	/* Check */
	if (a_Num < 0 || a_Num >= l_CONLNumVariables)
		return false;
	
	/* Return static variable link of variable */
	if (l_CONLVariables[a_Num])
	{
		if (a_VarP)
			*a_VarP = l_CONLVariables[a_Num]->StaticLink;
		
		// Variable in this spot
		return true;
	}
	
	/* Failed */
	return false;
}

/* CONL_VarLocate() -- Locates a variable by name */
CONL_StaticVar_t* CONL_VarLocate(const char* const a_Name)
{
	CONL_ConVariable_t* FoundVar;
	
	/* Check */
	if (!a_Name)
		return NULL;
	
	/* Locate by name */
	if ((FoundVar = CONLS_VarCoreLocate(a_Name)))
		// Only if it actually is registered
		if (FoundVar->StaticLink)
			return FoundVar->StaticLink;
	
	/* Failed */
	return NULL;
}

/* CONL_VarLocateHash() -- Locates variable by hash */
CONL_StaticVar_t* CONL_VarLocateHash(const uint32_t a_Hash)
{
	CONL_ConVariable_t* FoundVar;
	
	/* Check */
	if (!a_Hash)
		return NULL;
	
	/* Locate by hash */
	if ((FoundVar = CONLS_VarCoreLocateHash(a_Hash, true)))
		// Only if it actually is registered
		if (FoundVar->StaticLink)
			return FoundVar->StaticLink;
	
	/* Failed */
	return NULL;
}

/* CONL_VarSetStrByName() -- Set variable by its name */
const char* CONL_VarSetStrByName(const char* const a_Var, const char* const a_NewVal)
{
#define BUFSIZE 512
	char Buf[BUFSIZE];
	CONL_ConVariable_t* FoundVar;
	
	/* Check */
	if (!a_Var || !a_NewVal)
		return NULL;
	
	/* Unescape value */
	memset(Buf, 0, sizeof(Buf));
	CONL_UnEscapeString(Buf, BUFSIZE, a_NewVal);
	
	/* Locate by name */
	if ((FoundVar = CONLS_VarCoreLocate(a_Var)))
	{
		// If the variable is virtualized, return that
		if (FoundVar->IsVirtual)
		{
			// But if it is virtualized it was set before config load
			// i.e. + params
			if (!l_CONLVarLoaded)
			{
				// Replace the loaded value
				if (FoundVar->VirtualValue)
					Z_Free(FoundVar->VirtualValue);
				FoundVar->VirtualValue = NULL;
			
				// Put it there
				FoundVar->VirtualValue = Z_StrDup(Buf, PU_STATIC, NULL);
			}
			
			// Return the loaded value
			return (const char*)FoundVar->VirtualValue;
		}
		
		// Otherwise return the linked value
		else if (FoundVar->StaticLink)
		{
			// Use the real variable set (the one you are supposed to use)
			// It returns the corrected value
			return CONL_VarSetStr(FoundVar->StaticLink, a_NewVal);
		}
		
		// No value for this variable?
		return NULL;
	}
	
	/* Not found, so create a virtual variable */
	FoundVar = CONLS_PushVar(a_Var, NULL);
	
	// Set virtual and load the value we want there
	FoundVar->IsVirtual = true;
	FoundVar->VirtualValue = Z_StrDup(Buf, PU_STATIC, NULL);
	
	/* Return the virtualized value */
	return (const char*)FoundVar->VirtualValue;
#undef BUFSIZE
}

/* CONL_VarSetStr() -- Sets variable value, and returns actual set value */
const char* CONL_VarSetStr(CONL_StaticVar_t* a_Var, const char* const a_NewVal)
{
#define BUFSIZE 256
	char Buf[BUFSIZE];
	CONL_ConVariable_t* RealVar;
	size_t PossibleNum;
	bool_t GotPossible, Whole;
	int32_t IntVal, MinVal, MaxVal;
	double DblVal;
	fixed_t FixVal;
	char* StoppedAt;
	
	/* Check */
	if (!a_Var || !a_NewVal)
		return NULL;
	
	/* Get real variable */
	RealVar = a_Var->RealLink;
	
	// None of that?
	if (!RealVar)
		return NULL;
		
	/* Free old string in this spot */
	if (RealVar->Value.String)
		Z_Free(RealVar->Value.String);
	RealVar->Value.String = NULL;
	
	/* Variable is a string */
	if (a_Var->Type == CLVT_STRING)
	{
		// Unescape the string
		memset(Buf, 0, sizeof(Buf));
		CONL_UnEscapeString(Buf, BUFSIZE, a_NewVal);
		
		// Set string (a copy)
		RealVar->Value.String = Z_StrDup(Buf, PU_STATIC, NULL);
	}
	
	/* Variable is a number type */
	else
	{
		// Set whole to true, will be reset later
		Whole = true;
		
		// Convert string to integer (using strtol() to detect errors)
		IntVal = C_strtoi32(a_NewVal, &StoppedAt, 10);
		
		// No numbers?
		if (a_Var->Type == CLVT_INTEGER)
			if (StoppedAt == a_NewVal || (*StoppedAt != '\n' && *StoppedAt != '\0' && !isspace(*StoppedAt)))
			{
				IntVal = 0;
				Whole = false;
			}
		
		// Convert to double
		DblVal = strtod(a_NewVal, NULL);
		
		// Other Stuff
		FixVal = FLOAT_TO_FIXED(DblVal);
		MinVal = MaxVal = 0;
		Whole = !Whole;
		
		// Whole number? Only if converted successfully
		if (!Whole)
		{
			// Flip back
			Whole = false;
			
			// Check
			if (a_Var->Type == CLVT_INTEGER)
				Whole = true;
			else if (a_Var->Type == CLVT_FIXED)
				if ((FixVal - ((FixVal >> FRACBITS) << FRACBITS)) == 0)
					Whole = true;
		}
		
		// Failed on conversion
		else
			Whole = false;
	
		// Look in possible array
		for (GotPossible = false, PossibleNum = 0; a_Var->Possible[PossibleNum].StrAlias; PossibleNum++)
		{
			// String or number? match
			if ((strcasecmp(a_NewVal, a_Var->Possible[PossibleNum].StrAlias) == 0) ||
				(Whole && IntVal == a_Var->Possible[PossibleNum].IntVal))
			{
				GotPossible = true;
				break;
			}
			
			// Matches MINVAL?
			if (strcasecmp("MINVAL", a_Var->Possible[PossibleNum].StrAlias) == 0)
				MinVal = a_Var->Possible[PossibleNum].IntVal;
			
			// Matches MAXVAL?
			if (strcasecmp("MAXVAL", a_Var->Possible[PossibleNum].StrAlias) == 0)
				MaxVal = a_Var->Possible[PossibleNum].IntVal;
		}
		
		// Found possible value
		if (GotPossible)
		{
			// Set integer and fixed with the found possible value
			RealVar->Value.Int = a_Var->Possible[PossibleNum].IntVal;
			RealVar->Value.Fixed = RealVar->Value.Int << FRACBITS;
			
			// Set string to possible value that was found
			RealVar->Value.String = Z_StrDup(a_Var->Possible[PossibleNum].StrAlias, PU_STATIC, NULL);
		}
		
		// Not found, so cap to min/max
		else
		{
			// Is an integer
			if (a_Var->Type == CLVT_INTEGER)
			{
				// Capped?
				if (IntVal < MinVal)
					IntVal = MinVal;
				else if (IntVal > MaxVal)
					IntVal = MaxVal;
				
				// Set values
				RealVar->Value.Int = IntVal;
				RealVar->Value.Fixed = IntVal << FRACBITS;
				
				// Convert to string
				snprintf(Buf, BUFSIZE, "%i", IntVal);
				GotPossible = true;
			}
			
			// Is a fixed
			else if (a_Var->Type == CLVT_FIXED)
			{
				// Capped?
				if (FLOAT_TO_FIXED(DblVal) < (MinVal << FRACBITS))
					DblVal = (double)MinVal;
				else if (FLOAT_TO_FIXED(DblVal) > (MaxVal << FRACBITS))
					DblVal = (double)MaxVal;
				
				// Set values
				RealVar->Value.Int = (int32_t)DblVal;
				RealVar->Value.Fixed = FLOAT_TO_FIXED(DblVal);
				
				// Convert to string
				snprintf(Buf, BUFSIZE, "%f", DblVal);
				GotPossible = true;
			}
			
			// Set string value
			RealVar->Value.String = Z_StrDup((GotPossible ? Buf : ""), PU_STATIC, NULL);
		}
	}
	
	/* Call callback function, if possible */
	if (a_Var->ChangeFunc)
		a_Var->ChangeFunc(RealVar, a_Var);
	
	/* Return new value */
	return a_Var->Value->String;
#undef BUFSIZE
}

/* CONL_VarSetInt() -- Sets variable value, and returns actual set value */
int32_t CONL_VarSetInt(CONL_StaticVar_t* a_Var, const int32_t a_NewVal)
{
#define BUFSIZE 128
	char Buf[BUFSIZE];
	
	/* Check */
	if (!a_Var)
		return 0;
	
	/* Send to string handler */
	// Convert
	snprintf(Buf, BUFSIZE, "%i", a_NewVal);
	
	// Send and return
	if (CONL_VarSetStr(a_Var, Buf))
		return a_Var->Value->Int;
	
	/* Returned NULL? */
	return 0;
#undef BUFSIZE
}

/* CONL_VarSetFixed() -- Sets variable value, and returns actual set value */
fixed_t CONL_VarSetFixed(CONL_StaticVar_t* a_Var, const fixed_t a_NewVal)
{
#define BUFSIZE 128
	char Buf[BUFSIZE];
	
	/* Check */
	if (!a_Var)
		return 0;
	
	/* Send to string handler */
	// Convert
	snprintf(Buf, BUFSIZE, "%f", FIXED_TO_FLOAT(a_NewVal));
	
	// Send and return
	if (CONL_VarSetStr(a_Var, Buf))
		return a_Var->Value->Fixed;
	
	/* Returned NULL? */
	return 0;
#undef BUFSIZE
}

/* CONL_VarSlideValue() -- Slide variable */
bool_t CONL_VarSlideValue(CONL_StaticVar_t* const a_Var, const int32_t a_Right)
{
	int32_t i, DirMov, Hit, ixMi, ixMa, NextVal;
	
	/* Check */
	if (!a_Var || !a_Right)
		return false;
	
	/* Overloaded sliding? */
	if (a_Var->SlideFunc)
		return a_Var->SlideFunc(a_Var->RealLink, a_Var, a_Right);
	
	/* Cannot slide strings */
	if (a_Var->Type == CLVT_STRING)
		return false;
	
	/* If there is no possible value */
	if (!a_Var->Possible)
	{
		// Integer
		if (a_Var->Type == CLVT_INTEGER)
		{
			NextVal = a_Var->Value->Int + (1 * a_Right);
			return (CONL_VarSetInt(a_Var, NextVal) == NextVal);
		}
		
		// Fixed point
		else if (a_Var->Type == CLVT_FIXED)
		{
			NextVal = a_Var->Value->Fixed + ((fixed_t)4096 * (fixed_t)a_Right);
			return (CONL_VarSetFixed(a_Var, NextVal) == NextVal);
		}
		
		// Oops!
		else
			return false;
	}
	
	/* Change value based on direction */
	// Determine min/max and such
	ixMi = ixMa = Hit = -1;
	
	// Go through options
	for (i = 0; a_Var->Possible[i].StrAlias; i++)
	{
		// Is this value?
		if (Hit == -1)
			if (a_Var->Value->Int == a_Var->Possible[i].IntVal)
				Hit = i;
		
		// Min?
		if (strcasecmp(a_Var->Possible[i].StrAlias, "MINVAL") == 0)
			ixMi = i;
		
		// Max?
		if (strcasecmp(a_Var->Possible[i].StrAlias, "MAXVAL") == 0)
			ixMa = i;
	}
	
	// If found match and MIN/MAX are not the first values
	if (Hit != -1 && !(ixMi == 0 || ixMi == 1) && !(ixMa == 0 || ixMa == 1))
	{
		// Get moving direction
		if (a_Right > 0)
			DirMov = 1;
		else
			DirMov = -1;
		
		// Next value to check
		NextVal = Hit + DirMov;
		
		// Off the negative end?
		if (NextVal < 0)
			return false;
		
		// Off the top end?
		if (!a_Var->Possible[NextVal].StrAlias)
			return false;
		
		// Find the first non-match in direction
		for (; NextVal >= 0 && a_Var->Possible[NextVal].StrAlias; NextVal += DirMov)
		{
			// The min/max are usually placed at the end, so if they are bumped
			// do not set (this would prevent right direction looping while the left
			// does not loop).
			if ((ixMi != -1 && NextVal == ixMi) || (ixMa != -1 && NextVal == ixMa))
				return false;
			
			// Compare Integer Values
			if (a_Var->Possible[NextVal].IntVal != a_Var->Possible[Hit].IntVal)
			{
				NextVal = a_Var->Possible[NextVal].IntVal;
				return (CONL_VarSetInt(a_Var, NextVal) == NextVal);
			}
		}
	}
	
	// No sliding match found, or no value was even hit, slide it
		// Integer
	if (a_Var->Type == CLVT_INTEGER)
	{
		// Get next value
		if (a_Right < 0)
			NextVal = a_Var->Value->Int - 1;
		else if (a_Right > 0)
			NextVal = a_Var->Value->Int + 1;
		
		// Cap to minimum
		if (ixMi != -1)
			if (NextVal < a_Var->Possible[ixMi].IntVal)
				NextVal = a_Var->Possible[ixMi].IntVal;
			
		// Cap to maximum
		if (ixMa != -1)
			if (NextVal > a_Var->Possible[ixMa].IntVal)
				NextVal = a_Var->Possible[ixMa].IntVal;
		
		// Set integer value
		return (CONL_VarSetInt(a_Var, NextVal) == NextVal);
	}
	
		// Fixed
	else if (a_Var->Type == CLVT_FIXED)
	{
		// Get next value
		if (a_Right < 0)
			NextVal = a_Var->Value->Fixed - (fixed_t)4096;
		else if (a_Right > 0)
			NextVal = a_Var->Value->Fixed + (fixed_t)4096;
		
		// Cap to minimum
		if (ixMi != -1)
			if (NextVal < a_Var->Possible[ixMi].IntVal << (fixed_t)FRACBITS)
				NextVal = a_Var->Possible[ixMi].IntVal << (fixed_t)FRACBITS;
			
		// Cap to maximum
		if (ixMa != -1)
			if (NextVal > a_Var->Possible[ixMa].IntVal << (fixed_t)FRACBITS)
				NextVal = a_Var->Possible[ixMa].IntVal << (fixed_t)FRACBITS;
				
		// Set fixed value
		return (CONL_VarSetFixed(a_Var, NextVal) == NextVal);
	}
	
	/* No value changed? */
	return false;
}

/***********************
*** CONSOLE COMMANDS ***
***********************/

/* CLC_Version() -- ReMooD version info */
int CLC_Version(const uint32_t a_ArgC, const char** const a_ArgV)
{
	CONL_OutputF("ReMooD %i.%i%c \"%s\"\n", REMOOD_MAJORVERSION, REMOOD_MINORVERSION, REMOOD_RELEASEVERSION, REMOOD_VERSIONCODESTRING);
	CONL_OutputF("  Please visit %s for more information.\n", REMOOD_URL);
	
	/* Return success always */
	return 0;
}

/* CLC_Exec() -- Execute command */
int CLC_Exec(const uint32_t a_ArgC, const char** const a_ArgV)
{
	return CONL_Exec(a_ArgC - 1, a_ArgV + 1);
}

/* CLC_ExecFile() -- Executes a file */
int CLC_ExecFile(const uint32_t a_ArgC, const char** const a_ArgV)
{
#define BUFSIZE 512
	FILE* File;
	char Buf[BUFSIZE];
	char* p;
	
	/* Check */
	if (a_ArgC < 2)
		return 1;
		
	/* Message */
	CONL_PrintF("Executing \"%s\"...\n", a_ArgV[1]);
	
	/* Attempt open of file */
	File = fopen(a_ArgV[1], "r");
	
	// Failed?
	if (!File)
		return 1;
	
	/* Constantly read file */
	while (!feof(File))
	{
		// Read into buffer
		memset(Buf, 0, sizeof(Buf));
		fgets(Buf, BUFSIZE, File);
		
		// Skip white space at start
		for (p = Buf; *p && isspace(*p); p++);
		
		// Check for comment
		if (*p == '/' && *(p + 1) == '/')
			continue;
		
		// Execute command by sending it to the input buffer
		CONL_InputF("%s\n", Buf);
	}
	
	/* Close File */
	fclose(File);
	
	/* Success */
	return 0;
#undef BUFSIZE
}

/* CLC_Echo() -- Echo text */
int CLC_Echo(const uint32_t a_ArgC, const char** const a_ArgV)
{
	int i;
	
	/* Print all args */
	for (i = 1; i < a_ArgC; i++)
	{
		CONL_PrintF("%s", a_ArgV[i]);
		
		if (i < a_ArgC - 1)
			CONL_PrintF(" ");
	}
	CONL_PrintF("\n");
	
	/* Always works */
	return 0;
}

/* CLC_Exclamation() -- Runs command and reverses error status */
int CLC_Exclamation(const uint32_t a_ArgC, const char** const a_ArgV)
{
	int Code;
	
	/* Execute */
	Code = CONL_Exec(a_ArgC - 1, a_ArgV + 1);
	
	/* Which do we return? */
	if (!Code)
		return 1;
	else
		return 0;
}

/* CLC_Question() -- Runs command and sets error number */
int CLC_Question(const uint32_t a_ArgC, const char** const a_ArgV)
{
	int Code;
	
	/* Execute */
	Code = CONL_Exec(a_ArgC - 1, a_ArgV + 1);
	
	/* Set error */
	g_CONLError = Code;
	
	/* Always succeed */
	return 0;
}

/* CLC_CVarList() -- List console variables */
int CLC_CVarList(const uint32_t a_ArgC, const char** const a_ArgV)
{
#define BUFSIZE 512
	size_t i, j;
	CONL_ConVariable_t* CurVar;
	uint32_t Flags;
	char Buf[BUFSIZE];
	
	/* Check */
	if (!a_ArgC || !a_ArgV)
		return 1;
	
	/* No Variables? */
	if (!l_CONLNumVariables)
		return 1;
	
	/* Go through variable list */
	for (i = 0; i < l_CONLNumVariables; i++)
	{
		// Get variable
		CurVar = l_CONLVariables[i];
		
		// Missing?
		if (!CurVar)
			continue;
		
		// Print variable type
		if (CurVar->StaticLink)
		{
			if (CurVar->StaticLink->Type == CLVT_INTEGER)
				CONL_PrintF("{9INT {z");
			else if (CurVar->StaticLink->Type == CLVT_FIXED)
				CONL_PrintF("{9FIX {z");
			else if (CurVar->StaticLink->Type == CLVT_STRING)
				CONL_PrintF("{9STR {z");
			else
				CONL_PrintF("{0??? {z");
		}
		
		// Type-less
		else
			CONL_PrintF("{0??? {z");
		
		// Print variable flags
		if (CurVar->StaticLink)
		{
			// Get flags for less characters
			Flags = CurVar->StaticLink->Flags;
			
			// Print each bit
			CONL_PrintF("{7%c", (Flags & CLVF_SAVE			? 'S' : ' '));
			CONL_PrintF("{4%c", (Flags & CLVF_GAMESTATE		? 'g' : ' '));
			CONL_PrintF("{1%c", (Flags & CLVF_SERVERSTATE	? 'v' : ' '));
			CONL_PrintF("{3%c", (Flags & CLVF_CLIENTSTATE	? 'c' : ' '));
			CONL_PrintF("{b%c", (Flags & CLVF_READONLY		? 'R' : ' '));
			CONL_PrintF("{e%c", (Flags & CLVF_NOISY			? 'N' : ' '));
		}
		
		// Print nothing, if otherwise
		else
			CONL_PrintF("{z      ");
		
		// Print variable name
		CONL_PrintF("{z\"{4%-20.20s{z\" = {z", CurVar->Name);
		
		// A registered variable
		if (CurVar->StaticLink)
		{
			// Escape string
			memset(Buf, 0, sizeof(Buf));
			CONL_EscapeString(Buf, BUFSIZE, CurVar->StaticLink->Value[0].String);
			
			// Print
			CONL_PrintF("{z\"{2%s{z\"{z\n", Buf);
		}
		
		// Non-registered variable
		else
		{
			memset(Buf, 0, sizeof(Buf));
			CONL_EscapeString(Buf, BUFSIZE, CurVar->VirtualValue);
			CONL_PrintF("{z\"{2%s{z\"{z\n", Buf);
		}
	}
	
	/* Success! */
	return 0;
#undef BUFSIZE
}

/* CLC_CVarSet() -- Set variable to value */
int CLC_CVarSet(const uint32_t a_ArgC, const char** const a_ArgV)
{
#define BUFSIZE 512
	const char* RealValue;
	char Buf[BUFSIZE];
	
	/* Requires three arguments */
	if (a_ArgC < 3)
		return 1;
	
	/* Set value */
	RealValue = CONL_VarSetStrByName(a_ArgV[1], a_ArgV[2]);
	
	// Escape string
	memset(Buf, 0, sizeof(Buf));
	CONL_EscapeString(Buf, BUFSIZE, RealValue);
	
	// Message
	if (devparm)
		CONL_PrintF("{z{3%s{z was set to \"{5%s{z\".\n", a_ArgV[1], Buf);
	
	/* Success */
	return 0;
#undef BUFSIZE
}

/* CLC_Quit() -- Quit the game */
int CLC_Quit(const uint32_t a_ArgC, const char** const a_ArgV)
{
	/* Noooooooo! */
	I_Quit();
	
	/* Success */
	return 0;
}

/* CLC_CloseConsole() -- Close the console */
int CLC_CloseConsole(const uint32_t a_ArgC, const char** const a_ArgV)
{
	/* Close Console */
	CONL_SetActive(false);
	
	/* Success */
	return 0;
}


