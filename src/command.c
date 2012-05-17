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
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
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
#include "command.h"
#include "console.h"
#include "z_zone.h"
#include "d_clisrv.h"
#include "d_netcmd.h"
#include "m_misc.h"
#include "m_fixed.h"
#include "p_saveg.h"

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
	CONL_ExitCode_t (*ComFunc)(const uint32_t, const char** const);	// Function to call
	uint32_t Hash;											// Hash table hash
} CONL_ConCommand_t;

/*** VARIABLES ***/

/* CONL_ConVariable_s -- Console variable */
struct CONL_ConVariable_s
{
	/* Static */
	CONL_StaticVar_t* StaticLink;				// Link to static variable
	CONL_ConVariable_t* AliasTo;				// Alias to another variable
	consvar_t* DepVar;							// Deprecated variable
	
	/* Virtualization and Config */
	bool_t IsVirtual;							// A virtual variable (not regged)
	bool_t LoadedValue;							// Was set in the config file
	bool_t IsDeprecated;						// Is really a deprecated variable
	char* VirtualValue;							// Virtual Value
	
	/* Value */
	CONL_VarValue_t Value[MAXCONLVARSTATES];	// Value of variable (actual)
	char* Name;									// Name
	uint32_t Hash;								// Hash of name
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
	{3, "PrBoom"},								// VFONT_PRBOOMHUD
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
	
	// End
	{0, "MINVAL"},
	{13, "MAXVAL"},
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

static CONL_ExitCode_t g_CONLError = CLE_SUCCESS;			// '? command' Exit Code

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

/* CONLS_VarCoreLocate() -- Locate internal varaible by name */
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
		NewVar->StaticLink->Value = NewVar->Value;
		
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
const char* CONL_ExitCodeToStr(const CONL_ExitCode_t a_Code)
{
	static const char* const CodeStrs[NUMCONLEXITCODES] =
	{
		"Success",				// CLE_SUCCESS
		"Failure",				// CLE_FAILURE
		"Not An Error String",	// CLE_NOTANERRORSTRING
		"Critical Failure",		// CLE_CRITICALFAILURE
		"Unknown Command",		// CLE_UNKNOWNCOMMAND
		"Unknown Variable",		// CLE_UNKNOWNVARIABLE
		"Invalid Argument",		// CLE_INVALIDARGUMENT
		"Resource Not Found",	// CLE_RESOURCENOTFOUND
		"Connection Refused",	// CLE_CONNECTIONREFUSED
		"Read-only Medium",		// CLE_DISKREADONLY
		"Permission Denied",	// CLE_PERMISSIONDENIED
		"Unknown Sub-Command",	// CLE_UNKNOWNSUBCOMMAND
	};
	
	/* Return static char */
	if (a_Code >= NUMCONLEXITCODES || a_Code < 0)
		return CodeStrs[CLE_NOTANERRORSTRING];
	return CodeStrs[a_Code];
}

/* CONL_AddCommand() -- Add console command */
bool_t CONL_AddCommand(const char* const a_Name, CONL_ExitCode_t (*a_ComFunc)(const uint32_t, const char** const))
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
	Z_HashAddEntry(l_CONLCommandHashes, l_CONLCommands[l_CONLNumCommands].Hash, (uintptr_t)(l_CONLNumCommands + 1));
	
	/* Increment */
	l_CONLNumCommands++;
	
	/* Success */
	return true;
}

/* CONL_Exec() -- Execute command */
CONL_ExitCode_t CONL_Exec(const uint32_t a_ArgC, const char** const a_ArgV)
{
	uint32_t Hash;
	uintptr_t ComNum;
	
	/* Check */
	if (!a_ArgC || !a_ArgV)
		return CLE_CRITICALFAILURE;
	
	/* Find hash for command */
	Hash = Z_Hash(a_ArgV[0]);
	
	/* Find command for this hash */
	ComNum = (uintptr_t)Z_HashFindEntry(l_CONLCommandHashes, Hash, a_ArgV[0], false);
	ComNum -= 1;
	
	// Check, if not found try variables
	if (ComNum >= l_CONLNumCommands)
	{
		return CLE_UNKNOWNCOMMAND;	// not found
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
	CONL_VariableState_t vs;
	
	/* Check */
	if (!a_StaticVar)
		return NULL;
	
	/* Sanity */
	if (a_StaticVar->Type < 0 || a_StaticVar->Type >= NUMCONLVARIABLETYPES || !a_StaticVar->VarName || !a_StaticVar->DefaultValue)
		return NULL;
	
	// String types do not require possible values, but other types do
	if (a_StaticVar->Type != CLVT_STRING && !a_StaticVar->Possible)
		return NULL;
	
	/* Locate variable to see if it is virtualized or registered */
	if ((NewVar = CONLS_VarCoreLocate(a_StaticVar->VarName)))
	{
		// De-virtualize
		NewVar->IsVirtual = false;
		
		// Set static stuff
		NewVar->StaticLink = a_StaticVar;
		NewVar->StaticLink->Value = NewVar->Value;
		NewVar->StaticLink->RealLink = NewVar;
		
		// If the value is loaded
		if (NewVar->LoadedValue && NewVar->VirtualValue)
		{
			// Set with it
			CONL_VarSetStr(NewVar->StaticLink, CLVS_NORMAL, NewVar->VirtualValue);
			
			// Clear
			Z_Free(NewVar->VirtualValue);
			NewVar->VirtualValue = NULL;
			NewVar->LoadedValue = false;	// In case the var is not saved
		}
		
		// Otherwise, set it with the default value
		else
		{
			// Since it was never loaded with a config, we want the default
			CONL_VarSetStr(NewVar->StaticLink, CLVS_NORMAL, NewVar->StaticLink->DefaultValue);
		}
		
		// Set other states to no string
		if (a_StaticVar->Flags & CLVF_TRIPLESTATE)
			for (vs = CLVS_NORMAL + 1; vs < MAXCONLVARSTATES; vs++)
				NewVar->Value[vs].String = Z_StrDup("", PU_STATIC, NULL);
		
		// Return the same variable
		return NewVar;
	}
	
	/* Obtain a new variable */
	NewVar = CONLS_PushVar(a_StaticVar->VarName, a_StaticVar);
	
	// Set with default value
	CONL_VarSetStr(NewVar->StaticLink, CLVS_NORMAL, NewVar->StaticLink->DefaultValue);
	
	// Set other states to no string
	if (a_StaticVar->Flags & CLVF_TRIPLESTATE)
		for (vs = CLVS_NORMAL + 1; vs < MAXCONLVARSTATES; vs++)
			NewVar->Value[vs].String = Z_StrDup("", PU_STATIC, NULL);
	
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

/* CONL_VarSetStrByName() -- Set variable by its name */
const char* CONL_VarSetStrByName(const char* const a_Var, const CONL_VariableState_t a_State, const char* const a_NewVal)
{
#define BUFSIZE 512
	char Buf[BUFSIZE];
	CONL_ConVariable_t* FoundVar;
	
	/* Check */
	if (!a_Var || a_State < 0 || a_State >= MAXCONLVARSTATES || !a_NewVal)
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
			// Replace the loaded value
			if (FoundVar->VirtualValue)
				Z_Free(FoundVar->VirtualValue);
			FoundVar->VirtualValue = NULL;
			
			// Put it there
			FoundVar->VirtualValue = Z_StrDup(Buf, PU_STATIC, NULL);
			
			// Return the loaded value
			return (const char*)FoundVar->VirtualValue;
		}
		
		// Otherwise return the linked value
		else if (FoundVar->StaticLink)
		{
			// Use the real variable set (the one you are supposed to use)
			// It returns the corrected value
			return CONL_VarSetStr(FoundVar->StaticLink, a_State, a_NewVal);
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
const char* CONL_VarSetStr(CONL_StaticVar_t* a_Var, const CONL_VariableState_t a_State, const char* const a_NewVal)
{
#define BUFSIZE 256
	char Buf[BUFSIZE];
	CONL_ConVariable_t* RealVar;
	size_t PossibleNum;
	bool_t GotPossible, Whole;
	int32_t IntVal, MinVal, MaxVal;
	double DblVal;
	fixed_t FixVal;
	const char* StoppedAt;
	
	/* Check */
	if (!a_Var || a_State < 0 || a_State >= MAXCONLVARSTATES || !a_NewVal)
		return NULL;
	
	/* Multi/Single State Collision? */
	if (a_State != 0 && !(a_Var->Flags & CLVF_TRIPLESTATE))
		return NULL;
	
	/* Get real variable */
	RealVar = a_Var->RealLink;
	
	// None of that?
	if (!RealVar)
		return NULL;
		
	/* Free old string in this spot */
	if (RealVar->Value[a_State].String)
		Z_Free(RealVar->Value[a_State].String);
	RealVar->Value[a_State].String = NULL;
	
	/* Variable is a string */
	if (a_Var->Type == CLVT_STRING)
	{
		// Unescape the string
		memset(Buf, 0, sizeof(Buf));
		CONL_UnEscapeString(Buf, BUFSIZE, a_NewVal);
		
		// Set string (a copy)
		RealVar->Value[a_State].String = Z_StrDup(Buf, PU_STATIC, NULL);
	}
	
	/* Variable is a number type */
	else
	{
		// Set whole to true, will be reset later
		Whole = true;
		
		// Convert string to integer (using strtol() to detect errors)
		IntVal = strtol(a_NewVal, &StoppedAt, 10);
		
		// No numbers?
		if (a_Var->Type == CLVT_INTEGER)
			if (StoppedAt == a_NewVal || (*StoppedAt != '\n' && *StoppedAt != '\0' && !isspace(*StoppedAt)))
			{
				IntVal = 0;
				Whole = false;
			}
		
		// Convert to double
		DblVal = atof(a_NewVal);
		
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
			RealVar->Value[a_State].Int = a_Var->Possible[PossibleNum].IntVal;
			RealVar->Value[a_State].Fixed = RealVar->Value[a_State].Int << FRACBITS;
			
			// Set string to possible value that was found
			RealVar->Value[a_State].String = Z_StrDup(a_Var->Possible[PossibleNum].StrAlias, PU_STATIC, NULL);
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
				RealVar->Value[a_State].Int = IntVal;
				RealVar->Value[a_State].Fixed = IntVal << FRACBITS;
				
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
				RealVar->Value[a_State].Int = (int32_t)DblVal;
				RealVar->Value[a_State].Fixed = FLOAT_TO_FIXED(DblVal);
				
				// Convert to string
				snprintf(Buf, BUFSIZE, "%f", DblVal);
				GotPossible = true;
			}
			
			// Set string value
			RealVar->Value[a_State].String = Z_StrDup((GotPossible ? Buf : ""), PU_STATIC, NULL);
		}
	}
	
	/* Call callback function, if possible */
	if (a_Var->ChangeFunc)
		a_Var->ChangeFunc(RealVar, a_Var);
	
	/* Return new value */
	return a_Var->Value[a_State].String;
#undef BUFSIZE
}

/* CONL_VarSetInt() -- Sets variable value, and returns actual set value */
int32_t CONL_VarSetInt(CONL_StaticVar_t* a_Var, const CONL_VariableState_t a_State, const int32_t a_NewVal)
{
#define BUFSIZE 128
	char Buf[BUFSIZE];
	
	/* Check */
	if (!a_Var || a_State < 0 || a_State >= MAXCONLVARSTATES)
		return 0;
	
	/* Send to string handler */
	// Convert
	snprintf(Buf, BUFSIZE, "%i", a_NewVal);
	
	// Send and return
	if (CONL_VarSetStr(a_Var, a_State, Buf))
		return a_Var->Value[a_State].Int;
	
	/* Returned NULL? */
	return 0;
#undef BUFSIZE
}

/* CONL_VarSetFixed() -- Sets variable value, and returns actual set value */
fixed_t CONL_VarSetFixed(CONL_StaticVar_t* a_Var, const CONL_VariableState_t a_State, const fixed_t a_NewVal)
{
#define BUFSIZE 128
	char Buf[BUFSIZE];
	
	/* Check */
	if (!a_Var || a_State < 0 || a_State >= MAXCONLVARSTATES)
		return 0;
	
	/* Send to string handler */
	// Convert
	snprintf(Buf, BUFSIZE, "%f", FIXED_TO_FLOAT(a_NewVal));
	
	// Send and return
	if (CONL_VarSetStr(a_Var, a_State, Buf))
		return a_Var->Value[a_State].Fixed;
	
	/* Returned NULL? */
	return 0;
#undef BUFSIZE
}

/***********************
*** CONSOLE COMMANDS ***
***********************/

/* CLC_Version() -- ReMooD version info */
CONL_ExitCode_t CLC_Version(const uint32_t a_ArgC, const char** const a_ArgV)
{
	CONL_OutputF("ReMooD %i.%i%c \"%s\"\n", REMOOD_MAJORVERSION, REMOOD_MINORVERSION, REMOOD_RELEASEVERSION, REMOOD_VERSIONCODESTRING);
	CONL_OutputF("  Please visit %s for more information.\n", REMOOD_URL);
	
	/* Return success always */
	return CLE_SUCCESS;
}

/* CLC_Dep() -- Access to deprecated console */
CONL_ExitCode_t CLC_Dep(const uint32_t a_ArgC, const char** const a_ArgV)
{
#define BUFSIZE 512
	char Buf[BUFSIZE];
	uint32_t i, j, n;
	
	/* Clear */
	memset(Buf, 0, sizeof(Buf));
	
	/* Place command in buffer */
	// The old console code uses a single line
	// Also, ignore the actual "dep" part
	for (i = 1; i < a_ArgC; i++)
	{	
		// Always quote everything
		strncat(Buf, "\"", BUFSIZE);
		strncat(Buf, a_ArgV[i], BUFSIZE);
		strncat(Buf, "\"", BUFSIZE);
	}
	
	// Add newline
	strncat(Buf, "\n", BUFSIZE);
	
	/* Send command to be executed */
	COM_BufAddText(Buf);

	/* Flush command (to execute it) */
	COM_BufExecute();
	
	/* Always return success, no real way to get it to work */
	return CLE_SUCCESS;
#undef BUFSIZE
}

/* CLC_Exec() -- Execute command */
CONL_ExitCode_t CLC_Exec(const uint32_t a_ArgC, const char** const a_ArgV)
{
	return CONL_Exec(a_ArgC - 1, a_ArgV + 1);
}

/* CLC_ExecFile() -- Executes a file */
CONL_ExitCode_t CLC_ExecFile(const uint32_t a_ArgC, const char** const a_ArgV)
{
#define BUFSIZE 512
	FILE* File;
	char Buf[BUFSIZE];
	char* p;
	
	/* Check */
	if (a_ArgC < 2)
		return CLE_INVALIDARGUMENT;
		
	/* Message */
	CONL_PrintF("Executing \"%s\"...\n", a_ArgV[1]);
	
	/* Attempt open of file */
	File = fopen(a_ArgV[1], "r");
	
	// Failed?
	if (!File)
		return CLE_RESOURCENOTFOUND;
	
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
	return CLE_SUCCESS;
#undef BUFSIZE
}

/* CLC_Exclamation() -- Runs command and reverses error status */
CONL_ExitCode_t CLC_Exclamation(const uint32_t a_ArgC, const char** const a_ArgV)
{
	CONL_ExitCode_t Code;
	
	/* Execute */
	Code = CONL_Exec(a_ArgC - 1, a_ArgV + 1);
	
	/* Which do we return? */
	if (!Code)
		return CLE_FAILURE;
	else
		return CLE_SUCCESS;
}

/* CLC_Question() -- Runs command and sets error number */
CONL_ExitCode_t CLC_Question(const uint32_t a_ArgC, const char** const a_ArgV)
{
	CONL_ExitCode_t Code;
	
	/* Execute */
	Code = CONL_Exec(a_ArgC - 1, a_ArgV + 1);
	
	/* Set error */
	g_CONLError = Code;
	
	/* Always succeed */
	return CLE_SUCCESS;
}

/* CLC_CVarList() -- List console variables */
CONL_ExitCode_t CLC_CVarList(const uint32_t a_ArgC, const char** const a_ArgV)
{
#define BUFSIZE 512
	size_t i, j;
	CONL_ConVariable_t* CurVar;
	uint32_t Flags;
	char Buf[BUFSIZE];
	
	/* Check */
	if (!a_ArgC || !a_ArgV)
		return CLE_INVALIDARGUMENT;
	
	/* No Variables? */
	if (!l_CONLNumVariables)
		return CLE_RESOURCENOTFOUND;
	
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
			CONL_PrintF("{e%c", (Flags & CLVF_TRIPLESTATE	? 'T' : ' '));
		}
		
		// Print nothing, if otherwise
		else
			CONL_PrintF("{z      ");
		
		// Print variable name
		CONL_PrintF("{z\"{4%-20.20s{z\" = {z", CurVar->Name);
		
		// A registered variable
		if (CurVar->StaticLink)
		{
			// Multi-state
			if (CurVar->StaticLink->Flags & CLVF_TRIPLESTATE)
			{
				// Open brace
				CONL_PrintF("{b{{");
				
				for (j = 0; j < MAXCONLVARSTATES; j++)
				{
					// Escape string
					memset(Buf, 0, sizeof(Buf));
					CONL_EscapeString(Buf, BUFSIZE, CurVar->StaticLink->Value[j].String);
				
					// Value in quotes
					CONL_PrintF("{z\"{2%s{z\"{z", CurVar->StaticLink->Value[j].String);
					
					// Comma?
					if (j < MAXCONLVARSTATES - 1)
						CONL_PrintF("{z, ");
				}
				
				// Close brace
				CONL_PrintF("{b}{z\n");
			}
			
			// Single state
			else
			{
				// Escape string
				memset(Buf, 0, sizeof(Buf));
				CONL_EscapeString(Buf, BUFSIZE, CurVar->StaticLink->Value[0].String);
				
				// Print
				CONL_PrintF("{z\"{2%s{z\"{z\n", Buf);
			}
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
	return CLE_SUCCESS;
#undef BUFSIZE
}

/* CLC_CVarSet() -- Set variable to value */
CONL_ExitCode_t CLC_CVarSet(const uint32_t a_ArgC, const char** const a_ArgV)
{
#define BUFSIZE 512
	const char* RealValue;
	char Buf[BUFSIZE];
	
	/* Requires three arguments */
	if (a_ArgC < 3)
		return CLE_INVALIDARGUMENT;
	
	/* Set value */
	RealValue = CONL_VarSetStrByName(a_ArgV[1], CLVS_NORMAL, a_ArgV[2]);
	
	// Escape string
	memset(Buf, 0, sizeof(Buf));
	CONL_EscapeString(Buf, BUFSIZE, RealValue);
	
	// Message
	if (devparm)
		CONL_PrintF("{z{3%s{z was set to \"{5%s{z\".\n", a_ArgV[1], Buf);
	
	/* Success */
	return CLE_SUCCESS;
#undef BUFSIZE
}

/*******************************************************************************
********************************************************************************
*******************************************************************************/

//      code stolen from the QuakeC sources, thanks Id :)

//========
// protos.
//========
static bool_t COM_Exists(char* com_name);
static void COM_ExecuteString(char* text);

static void COM_Alias_f(void);
static void COM_Echo_f(void);
static void COM_Exec_f(void);
static void COM_Wait_f(void);
static void COM_Help_f(void);
static void COM_commandlist_f(void);
static void COM_cvarlist_f(void);
static void COM_Toggle_f(void);

static bool_t CV_Command(void);
static char* CV_StringValue(char* var_name);
static consvar_t* consvar_vars;	// list of registered console variables

static char com_token[1024];
static char* COM_Parse(char* data);

CV_PossibleValue_t CV_OnOff[] = { {0, "Off"}, {1, "On"}, {0, NULL} };
CV_PossibleValue_t CV_YesNo[] = { {0, "No"}, {1, "Yes"}, {0, NULL} };
CV_PossibleValue_t CV_Unsigned[] = { {0, "MIN"}, {999999999, "MAX"}, {0, NULL} };

#define COM_BUF_SIZE    8192	// command buffer size

int com_wait;					// one command per frame (for cmd sequences)

// command aliases
//
typedef struct cmdalias_s
{
	struct cmdalias_s* next;
	char* name;
	char* value;				// the command string to replace the alias
} cmdalias_t;

cmdalias_t* com_alias;			// aliases list

consvar_t* CV_Export(void)
{
	return consvar_vars;
}

// =========================================================================
//                            COMMAND BUFFER
// =========================================================================

vsbuf_t com_text;				// variable sized buffer

//  Add text in the command buffer (for later execution)
//
void COM_BufAddText(char* text)
{
	int l;
	
	l = strlen(text);
	
	if (com_text.cursize + l >= com_text.maxsize)
	{
		CONL_PrintF("Command buffer full!\n");
		return;
	}
	VS_Write(&com_text, text, l);
}

// Adds command text immediately after the current command
// Adds a \n to the text
//
void COM_BufInsertText(char* text)
{
	char* temp;
	int templen;
	
	// copy off any commands still remaining in the exec buffer
	templen = com_text.cursize;
	if (templen)
	{
		temp = Z_Malloc(templen, PU_STATIC, NULL);
		memcpy(temp, com_text.data, templen);
		VS_Clear(&com_text);
	}
	else
		temp = NULL;			// shut up compiler
		
	// add the entire text of the file (or alias)
	COM_BufAddText(text);
	
	// add the copied off data
	if (templen)
	{
		VS_Write(&com_text, temp, templen);
		Z_Free(temp);
	}
}

//  Flush (execute) console commands in buffer
//   does only one if com_wait
//
void COM_BufExecute(void)
{
	int i;
	char* text;
	char line[1024];
	int quotes;
	
	if (com_wait)
	{
		com_wait--;
		return;
	}
	
	while (com_text.cursize)
	{
		// find a '\n' or ; line break
		text = (char*)com_text.data;
		
		quotes = 0;
		for (i = 0; i < com_text.cursize; i++)
		{
			if (text[i] == '"')
				quotes++;
			if (!(quotes & 1) && text[i] == ';')
				break;			// don't break if inside a quoted string
			if (text[i] == '\n' || text[i] == '\r')
				break;
		}
		
		memcpy(line, text, i);
		line[i] = 0;
		
		// flush the command text from the command buffer, _BEFORE_
		// executing, to avoid that 'recursive' aliases overflow the
		// command text buffer, in that case, new commands are inserted
		// at the beginning, in place of the actual, so it doesn't
		// overflow
		if (i == com_text.cursize)
			// the last command was just flushed
			com_text.cursize = 0;
		else
		{
			i++;
			com_text.cursize -= i;
			memmove(text, text + i, com_text.cursize);
		}
		
		// execute the command line
		COM_ExecuteString(line);
		
		// delay following commands if a wait was encountered
		if (com_wait)
		{
			com_wait--;
			break;
		}
	}
}

// =========================================================================
//                            COMMAND EXECUTION
// =========================================================================

typedef struct xcommand_s
{
	char* name;
	struct xcommand_s* next;
	com_func_t function;
} xcommand_t;

static xcommand_t* com_commands = NULL;	// current commands

#define MAX_ARGS        80
static int com_argc;
static char* com_argv[MAX_ARGS];
static char* com_null_string = "";
static char* com_args = NULL;	// current command args or NULL

//  Initialise command buffer and add basic commands
//
void COM_Init(void)
{
	// allocate command buffer
	VS_Alloc(&com_text, COM_BUF_SIZE);
	
	// add standard commands
	COM_AddCommand("alias", COM_Alias_f);
	COM_AddCommand("echo", COM_Echo_f);
	COM_AddCommand("exec", COM_Exec_f);
	COM_AddCommand("wait", COM_Wait_f);
	COM_AddCommand("help", COM_Help_f);
	COM_AddCommand("commandlist", COM_commandlist_f);
	COM_AddCommand("commands", COM_commandlist_f);
	COM_AddCommand("cvarlist", COM_cvarlist_f);
	COM_AddCommand("cvars", COM_cvarlist_f);
	COM_AddCommand("toggle", COM_Toggle_f);
}

// Returns how many args for last command
//
int COM_Argc(void)
{
	return com_argc;
}

// Returns string pointer for given argument number
//
char* COM_Argv(int arg)
{
	if (arg >= com_argc || arg < 0)
		return com_null_string;
	return com_argv[arg];
}

// Returns string pointer of all command args
//
char* COM_Args(void)
{
	return com_args;
}

int COM_CheckParm(char* check)
{
	int i;
	
	for (i = 1; i < com_argc; i++)
	{
		if (!strcasecmp(check, com_argv[i]))
			return i;
	}
	return 0;
}

// Parses the given string into command line tokens.
//
// Takes a null terminated string.  Does not need to be /n terminated.
// breaks the string up into arg tokens.
static void COM_TokenizeString(char* text)
{
	int i, n;
	
// clear the args from the last string
	for (i = 0; i < com_argc; i++)
		Z_Free(com_argv[i]);
		
	com_argc = 0;
	com_args = NULL;
	
	for (;;)
	{
// skip whitespace up to a /n
		while (*text && *text <= ' ' && *text != '\n')
			text++;
			
		if (*text == '\n')
		{
			// a newline means end of command in buffer,
			// thus end of this command's args too
			text++;
			break;
		}
		
		if (!*text)
			return;
			
		if (com_argc == 1)
			com_args = text;
			
		text = COM_Parse(text);
		if (!text)
			return;
			
		if (com_argc < MAX_ARGS)
		{
			n = strlen(com_token) + 1;
			com_argv[com_argc] = Z_Malloc(n, PU_STATIC, NULL);
			strncpy(com_argv[com_argc], com_token, n);
			com_argc++;
		}
	}
	
}

// Add a command before existing ones.
//
void COM_AddCommand(char* name, com_func_t func)
{
	xcommand_t* cmd;
	
	//CONEx_Command_t Command;
	
	/* Check */
	if (!name || !func)
		return;
		
	/* Clear */
	//memset(&Command, 0, sizeof(Command));
	
	/* Set */
	//strncpy(Command.Name, name, CONEX_MAXVARIABLENAME);
	//Command.Func = COMEx_DeprCommandFunc;
	//Command.DepFunc = func;
	
	/* Send */
	//CONEx_AddCommand(CONEx_GetRootConsole(), &Command);
	
	// GhostlyDeath <November 10, 2010> -- Deprecated code follows
	
	// fail if the command is a variable name
	if (CV_StringValue(name)[0])
	{
		CONL_PrintF("%s is a variable name\n", name);
		return;
	}
	// fail if the command already exists
	for (cmd = com_commands; cmd; cmd = cmd->next)
	{
		if (!strcmp(name, cmd->name))
		{
			CONL_PrintF("Command %s already exists\n", name);
			return;
		}
	}
	
	cmd = Z_Malloc(sizeof(xcommand_t), PU_STATIC, NULL);
	cmd->name = name;
	cmd->function = func;
	cmd->next = com_commands;
	com_commands = cmd;
}

//  Returns true if a command by the name given exists
//
static bool_t COM_Exists(char* com_name)
{
	xcommand_t* cmd;
	
	for (cmd = com_commands; cmd; cmd = cmd->next)
	{
		if (!strcmp(com_name, cmd->name))
			return true;
	}
	
	return false;
}

//  Command completion using TAB key like '4dos'
//  Will skip 'skips' commands
//
char* COM_CompleteCommand(char* partial, int skips)
{
	xcommand_t* cmd;
	int len;
	
	len = strlen(partial);
	
	if (!len)
		return NULL;
		
// check functions
	for (cmd = com_commands; cmd; cmd = cmd->next)
		if (!strncmp(partial, cmd->name, len))
			if (!skips--)
				return cmd->name;
				
	return NULL;
}

// Parses a single line of text into arguments and tries to execute it.
// The text can come from the command buffer, a remote client, or stdin.
//
static void COM_ExecuteString(char* text)
{
	xcommand_t* cmd;
	cmdalias_t* a;
	
	COM_TokenizeString(text);
	
// execute the command line
	if (!COM_Argc())
		return;					// no tokens
		
// check functions
	for (cmd = com_commands; cmd; cmd = cmd->next)
	{
		if (!strcmp(com_argv[0], cmd->name))
		{
			cmd->function();
			return;
		}
	}
	
// check aliases
	for (a = com_alias; a; a = a->next)
	{
		if (!strcmp(com_argv[0], a->name))
		{
			COM_BufInsertText(a->value);
			return;
		}
	}
	
	// GhostlyDeath <July 8, 2008> -- If it starts with profile have the profile manager handle it
	{
		// check cvars
		// Hurdler: added at Ebola's request ;)
		// (don't flood the console in software mode with bad gr_xxx command)
		if (!CV_Command() && con_destlines)
			CONL_PrintF("Unknown command '%s'\n", COM_Argv(0));
	}
}

// =========================================================================
//                            SCRIPT COMMANDS
// =========================================================================

// alias command : a command name that replaces another command
//
static void COM_Alias_f(void)
{
	cmdalias_t* a;
	char cmd[1024];
	int i, c;
	
	if (COM_Argc() < 3)
	{
		CONL_PrintF("alias <name> <command>\n");
		return;
	}
	
	a = Z_Malloc(sizeof(cmdalias_t), PU_STATIC, NULL);
	a->next = com_alias;
	com_alias = a;
	
	a->name = Z_StrDup(COM_Argv(1), PU_STATIC, NULL);
	
// copy the rest of the command line
	cmd[0] = 0;					// start out with a null string
	c = COM_Argc();
	for (i = 2; i < c; i++)
	{
		strcat(cmd, COM_Argv(i));
		if (i != c)
			strcat(cmd, " ");
	}
	strcat(cmd, "\n");
	
	a->value = Z_StrDup(cmd, PU_STATIC, NULL);
}

// Echo a line of text to console
//
static void COM_Echo_f(void)
{
	int i;
	
	for (i = 1; i < COM_Argc(); i++)
		CONL_PrintF("%s ", COM_Argv(i));
	CONL_PrintF("\n");
}

// Execute a script file
//
static void COM_Exec_f(void)
{
	int length;
	uint8_t* buf = NULL;
	
	if (COM_Argc() != 2)
	{
		CONL_PrintF("exec <filename> : run a script file\n");
		return;
	}
// load file

	length = FIL_ReadFile(COM_Argv(1), &buf);
	//CONL_PrintF ("debug file length : %d\n",length);
	
	if (!buf)
	{
		CONL_PrintF("couldn't execute file %s\n", COM_Argv(1));
		return;
	}
	
	CONL_PrintF("executing %s\n", COM_Argv(1));
	
// insert text file into the command buffer

	COM_BufInsertText(buf);
	
// free buffer

	Z_Free(buf);
	
}

// Delay execution of the rest of the commands to the next frame,
// allows sequences of commands like "jump; fire; backward"
//
static void COM_Wait_f(void)
{
	if (COM_Argc() > 1)
		com_wait = atoi(COM_Argv(1));
	else
		com_wait = 1;			// 1 frame
}

static void COM_Help_f(void)
{
	xcommand_t* cmd;
	consvar_t* cvar;
	int i = 0;
	
	if (COM_Argc() > 1)
	{
		cvar = CV_FindVar(COM_Argv(1));
		if (cvar)
		{
			CONL_PrintF("Variable %s:\n", cvar->name);
			CONL_PrintF("  flags :");
			if (cvar->flags & CV_SAVE)
				CONL_PrintF("AUTOSAVE ");
			if (cvar->flags & CV_FLOAT)
				CONL_PrintF("FLOAT ");
			if (cvar->flags & CV_NETVAR)
				CONL_PrintF("NETVAR ");
			if (cvar->flags & CV_CALL)
				CONL_PrintF("ACTION ");
			CONL_PrintF("\n");
			if (cvar->PossibleValue)
			{
				if (strcasecmp(cvar->PossibleValue[0].strvalue, "MIN") == 0)
				{
					for (i = 1; cvar->PossibleValue[i].strvalue != NULL; i++)
						if (!strcasecmp(cvar->PossibleValue[i].strvalue, "MAX"))
							break;
					CONL_PrintF("  range from %d to %d\n", cvar->PossibleValue[0].value, cvar->PossibleValue[i].value);
				}
				else
				{
					CONL_PrintF("  possible value :\n", cvar->name);
					while (cvar->PossibleValue[i].strvalue)
					{
						CONL_PrintF("    %-2d : %s\n", cvar->PossibleValue[i].value, cvar->PossibleValue[i].strvalue);
						i++;
					}
				}
			}
		}
		else
			CONL_PrintF("No Help for this command/variable\n");
	}
	else
	{
		// commands
		/*CONL_PrintF("\2Commands\n");
		   for (cmd = com_commands; cmd; cmd = cmd->next)
		   {
		   CONL_PrintF("%s ", cmd->name);
		   i++;
		   }
		
		   // varibale
		   CONL_PrintF("\2\nVariable\n");
		   for (cvar = consvar_vars; cvar; cvar = cvar->next)
		   {
		   CONL_PrintF("%s ", cvar->name);
		   i++;
		   } */
		
		CONL_PrintF("\2\nType \"commandlist\" or \"cvarlist\" for more or type help <command or variable>\n");
		
		//if (devparm)
		//  CONL_PrintF("\2Total : %d\n", i);
	}
}

static void COM_commandlist_f(void)
{
	xcommand_t* cmd;
	int i = 0;
	
	if (COM_Argc() > 1 && strncasecmp(COM_Argv(1) + 1, "short", 5))
		for (cmd = com_commands; cmd; cmd = cmd->next)
		{
			CONL_PrintF("%s ", cmd->name);
			i++;
		}
	else
		for (cmd = com_commands; cmd; cmd = cmd->next)
		{
			CONL_PrintF("%s\n", cmd->name);
			i++;
		}
		
	CONL_PrintF("\2\nTotal of %i Commands\n", i);
}

static void COM_cvarlist_f(void)
{
	consvar_t* cvar;
	int i = 0;
	int j;
	char chars[8];
	
	if (COM_Argc() > 1 && strncasecmp(COM_Argv(1) + 1, "short", 5))
		for (cvar = consvar_vars; cvar; cvar = cvar->next)
		{
			CONL_PrintF("%s ", cvar->name);
			i++;
		}
	else
		for (cvar = consvar_vars; cvar; cvar = cvar->next)
		{
			memset(chars, ' ', sizeof(chars));
			
			if (cvar->flags & CV_SAVE)
				chars[0] = 'S';
			if (cvar->flags & CV_CALL)
				chars[1] = 'C';
			if (cvar->flags & CV_NETVAR)
				chars[2] = 'N';
			if (cvar->flags & CV_NOINIT)
				chars[3] = '-';
			if (cvar->flags & CV_FLOAT)
				chars[4] = 'F';
			if (cvar->flags & CV_ALIAS)
				chars[5] = 'A';
			if (cvar->flags & CV_DEPRECATED)
				chars[6] = 'D';
			if (cvar->flags & CV_INVERTEDALIAS)
				chars[7] = 'I';
				
			for (j = 0; j < sizeof(chars); j++)
				CONL_PrintF("%c", chars[j]);
				
			if (cvar->flags & CV_ALIAS)
				CONL_PrintF("\t%s >>> %s\n", cvar->name, cvar->aliasto);
			else
				CONL_PrintF("\t%s\n", cvar->name);
			i++;
		}
		
	CONL_PrintF("\2\nTotal of %i Variables\n", i);
}

static void COM_Toggle_f(void)
{
	consvar_t* cvar;
	
	if (COM_Argc() != 2 && COM_Argc() != 3)
	{
		CONL_PrintF("Toggle <cvar_name> [-1]\n" "Toggle the value of a cvar\n");
		return;
	}
	cvar = CV_FindVar(COM_Argv(1));
	if (!cvar)
	{
		CONL_PrintF("%s is not a cvar\n", COM_Argv(1));
		return;
	}
	// netcvar don't change imediately
	cvar->flags |= CV_SHOWMODIFONETIME;
	if (COM_Argc() == 3)
		CV_AddValue(cvar, atol(COM_Argv(2)));
	else
		CV_AddValue(cvar, +1);
}

// =========================================================================
//                      VARIABLE SIZE BUFFERS
// =========================================================================

#define VSBUFMINSIZE   256

void VS_Alloc(vsbuf_t* buf, int initsize)
{
	if (initsize < VSBUFMINSIZE)
		initsize = VSBUFMINSIZE;
	buf->data = Z_Malloc(initsize, PU_STATIC, NULL);
	buf->maxsize = initsize;
	buf->cursize = 0;
}

void VS_Free(vsbuf_t* buf)
{
//  Z_Free (buf->data);
	buf->cursize = 0;
}

void VS_Clear(vsbuf_t* buf)
{
	buf->cursize = 0;
}

void* VS_GetSpace(vsbuf_t* buf, int length)
{
	void* data;
	
	if (buf->cursize + length > buf->maxsize)
	{
		if (!buf->allowoverflow)
			I_Error("overflow 111");
			
		if (length > buf->maxsize)
			I_Error("overflow l%i 112", length);
			
		buf->overflowed = true;
		CONL_PrintF("VS buffer overflow");
		VS_Clear(buf);
	}
	
	data = buf->data + buf->cursize;
	buf->cursize += length;
	
	return data;
}

//  Copy data at end of variable sized buffer
//
void VS_Write(vsbuf_t* buf, void* data, int length)
{
	memcpy(VS_GetSpace(buf, length), data, length);
}

//  Print text in variable size buffer, like VS_Write + trailing 0
//
void VS_Print(vsbuf_t* buf, char* data)
{
	int len;
	
	len = strlen(data) + 1;
	
	if (buf->data[buf->cursize - 1])
		memcpy((uint8_t*)VS_GetSpace(buf, len), data, len);	// no trailing 0
	else
		memcpy((uint8_t*)VS_GetSpace(buf, len - 1) - 1, data, len);	// write over trailing 0
}

// =========================================================================
//
//                           CONSOLE VARIABLES
//
//   console variables are a simple way of changing variables of the game
//   through the console or code, at run time.
//
//   console vars acts like simplified commands, because a function can be
//   attached to them, and called whenever a console var is modified
//
// =========================================================================

static char* cv_null_string = "";

//  Search if a variable has been registered
//  returns true if given variable has been registered
//
consvar_t* CV_FindVar(char* name)
{
	consvar_t* cvar;
	
	for (cvar = consvar_vars; cvar; cvar = cvar->next)
		if (!strcmp(name, cvar->name))
			return cvar;
			
	return NULL;
}

//  Build a unique Net Variable identifier number, that is used
//  in network packets instead of the fullname
//
unsigned short CV_ComputeNetid(char* s)
{
	unsigned short ret;
	static int premiers[16] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53 };
	int i;
	
	ret = 0;
	i = 0;
	while (*s)
	{
		ret += (*s) * premiers[i];
		s++;
		i = (i + 1) % 16;
	}
	return ret;
}

//  Return the Net Variable, from it's identifier number
//
static consvar_t* CV_FindNetVar(unsigned short netid)
{
	consvar_t* cvar;
	
	for (cvar = consvar_vars; cvar; cvar = cvar->next)
		if (cvar->netid == netid)
			return cvar;
			
	return NULL;
}

void Setvalue(consvar_t* var, char* valstr);

//  Register a variable, that can be used later at the console
//
void CV_RegisterVar(consvar_t* variable)
{
	consvar_t* cvar = NULL;
	int i, j;
	consvar_t** temp = NULL;
	
	// first check to see if it has allready been defined
	if (CV_FindVar(variable->name))
	{
		CONL_PrintF("Variable %s is already defined\n", variable->name);
		return;
	}
	// check for overlap with a command
	if (COM_Exists(variable->name))
	{
		CONL_PrintF("%s is a command name\n", variable->name);
		return;
	}
	// check net variables
	if (variable->flags & CV_NETVAR)
	{
		variable->netid = CV_ComputeNetid(variable->name);
		if (CV_FindNetVar(variable->netid))
			I_Error("Variable %s have same netid\n", variable->name);
	}
	// link the variable in
	if (!(variable->flags & CV_HIDEN))
	{
		variable->next = consvar_vars;
		consvar_vars = variable;
	}
	variable->string = NULL;
	
	// copy the value off, because future sets will Z_Free it
	//variable->string = Z_StrDup (variable->string);
	
#ifdef PARANOIA
	if ((variable->flags & CV_NOINIT) && !(variable->flags & CV_CALL))
		I_Error("variable %s have CV_NOINIT without CV_CALL\n");
	if ((variable->flags & CV_CALL) && !variable->func)
		I_Error("variable %s have cv_call flags whitout func");
#endif
	if (variable->flags & CV_NOINIT)
		variable->flags &= ~CV_CALL;
		
	Setvalue(variable, variable->defaultvalue);
	
	if (variable->flags & CV_NOINIT)
		variable->flags |= CV_CALL;
		
	// the SetValue will set this bit
	variable->flags &= ~CV_MODIFIED;
	
	/* Setup Alias Links */
	for (cvar = consvar_vars; cvar; cvar = cvar->next)
	{
		if (cvar == variable)
			continue;
			
		if (cvar->flags & CV_ALIAS && cvar->aliasto)
			if (!strcmp(variable->name, cvar->aliasto))
			{
				/* Link US to THEM */
				if (!variable->ACount)
				{
					variable->ACount = 1;
					variable->ALink = Z_Malloc(sizeof(consvar_t*), PU_STATIC, NULL);
					variable->ALink[0] = cvar;
				}
				else
				{
					j = 0;
					for (i = 0; i < variable->ACount; i++)
						if (variable->ALink[i] == cvar)
							j = 1;
							
					// Didn't find it
					if (!j)
					{
						variable->ACount = 2;
						temp = variable->ALink;
						variable->ALink = Z_Malloc(sizeof(consvar_t*) * variable->ACount, PU_STATIC, NULL);
						memcpy(variable->ALink, temp, sizeof(consvar_t*) * (variable->ACount - 1));
						Z_Free(temp);
						variable->ALink[variable->ACount - 1] = cvar;
					}
				}
				
				/* Link THEM to US */
				if (!cvar->ACount)
				{
					cvar->ACount = 1;
					cvar->ALink = Z_Malloc(sizeof(consvar_t*), PU_STATIC, NULL);
					cvar->ALink[0] = variable;
				}
				else
				{
					j = 0;
					for (i = 0; i < cvar->ACount; i++)
						if (cvar->ALink[i] == variable)
							j = 1;
							
					// Didn't find it
					if (!j)
					{
						cvar->ACount = 2;
						temp = cvar->ALink;
						cvar->ALink = Z_Malloc(sizeof(consvar_t*) * cvar->ACount, PU_STATIC, NULL);
						memcpy(cvar->ALink, temp, sizeof(consvar_t*) * (cvar->ACount - 1));
						Z_Free(temp);
						cvar->ALink[cvar->ACount - 1] = variable;
					}
				}
			}
	}
}

void CV_UnRegisterVar(consvar_t* var)
{
	consvar_t* rover = NULL;
	consvar_t* brover = NULL;
	
	// Return if NULL
	if (!var)
		return;
		
	for (rover = consvar_vars; rover; brover = rover, rover = rover->next)
		if (rover == var)
		{
			if (brover)			// might be the first CVAR!
				brover->next = rover->next;
			if (rover == consvar_vars)
				consvar_vars = rover->next;
			rover->next = NULL;
			return;
		}
		
	CONL_PrintF("Variable \"%s\" is not registered!\n", var->name);
}

//  Returns the string value of a console var
//
static char* CV_StringValue(char* var_name)
{
	consvar_t* var;
	
	var = CV_FindVar(var_name);
	if (!var)
		return cv_null_string;
	return var->string;
}

//  Completes the name of a console var
//
char* CV_CompleteVar(char* partial, int skips)
{
	consvar_t* cvar;
	int len;
	
	len = strlen(partial);
	
	if (!len)
		return NULL;
		
	// check functions
	for (cvar = consvar_vars; cvar; cvar = cvar->next)
		if (!strncmp(partial, cvar->name, len))
			if (!skips--)
				return cvar->name;
				
	return NULL;
}

// set value to the variable, no check only for internal use
//
void Setvalue(consvar_t* var, char* valstr)
{
	int i;
	
	// Alias Handling
	consvar_t* ActualVar = var;
	
	if (var->flags & CV_ALIAS && var->aliasto)
	{
		ActualVar = CV_FindVar(var->aliasto);
		
		if (!ActualVar)
			return;
	}
	// All normal
	if (ActualVar->PossibleValue)
	{
		int v = atoi(valstr);
		
		if (!strcasecmp(ActualVar->PossibleValue[0].strvalue, "MIN"))
		{
			// bounded cvar
			int i;
			
			// search for maximum
			for (i = 1; ActualVar->PossibleValue[i].strvalue != NULL; i++)
				if (!strcasecmp(ActualVar->PossibleValue[i].strvalue, "MAX"))
					break;
#ifdef PARANOIA
			if (ActualVar->PossibleValue[i].strvalue == NULL)
				I_Error("Bounded cvar \"%s\" without Maximum !", ActualVar->name);
#endif
			if (v < ActualVar->PossibleValue[0].value)
			{
				v = ActualVar->PossibleValue[0].value;
				sprintf(valstr, "%d", v);
			}
			if (v > ActualVar->PossibleValue[i].value)
			{
				v = ActualVar->PossibleValue[i].value;
				sprintf(valstr, "%d", v);
			}
		}
		else
		{
			// waw spaghetti programming ! :)
			
			// check first strings
			for (i = 0; ActualVar->PossibleValue[i].strvalue != NULL; i++)
				if (!strcasecmp(ActualVar->PossibleValue[i].strvalue, valstr))
					goto found;
			if (!v)
				if (strcmp(valstr, "0") != 0)	// !=0 if valstr!="0"
					goto error;
			// check int now
			for (i = 0; ActualVar->PossibleValue[i].strvalue != NULL; i++)
				if (v == ActualVar->PossibleValue[i].value)
					goto found;
					
error:							// not found
			CONL_PrintF("\"%s\" is not a possible value for \"%s\"\n", valstr, ActualVar->name);
			if (ActualVar->defaultvalue == valstr)
				I_Error("Variable %s default value \"%s\" is not a possible value\n", ActualVar->name, ActualVar->defaultvalue);
			return;
found:
			ActualVar->value = ActualVar->PossibleValue[i].value;
			ActualVar->string = ActualVar->PossibleValue[i].strvalue;
			
			goto finish;
		}
	}
	// free the old value string
	if (var->string)
		Z_Free(ActualVar->string);
		
	ActualVar->string = Z_StrDup(valstr, PU_STATIC, NULL);
	
	if (ActualVar->flags & CV_FLOAT)
	{
		double d;
		
		d = atof(ActualVar->string);
		ActualVar->value = d * FRACUNIT;
	}
	else
		ActualVar->value = atoi(ActualVar->string);
		
finish:
	/* Match Aliases to value */
	if (ActualVar->ALink && ActualVar->ACount)
	{
		for (i = 0; i < ActualVar->ACount; i++)
		{
			ActualVar->ALink[i]->flags |= CV_MODIFIED;
			ActualVar->ALink[i]->value = ActualVar->value;
			ActualVar->ALink[i]->string = ActualVar->string;
		}
	}
	
	if (ActualVar->flags & CV_SHOWMODIFONETIME || ActualVar->flags & CV_SHOWMODIF || (var->flags & CV_ALIAS && var->flags & CV_DEPRECATED) ||
	var->flags & CV_DEPRECATED)
	{
		if (var->flags & CV_ALIAS && var->flags & CV_DEPRECATED)
			CONL_PrintF("Please use \"%s\" instead of \"%s\", as the latter will be removed.\n", var->aliasto, var->name);
		else if (var->flags & CV_DEPRECATED)
			CONL_PrintF("%s is deprecated and will be removed in a future version.\n", var->name);
			
		CONL_PrintF("%s set to %s\n", ActualVar->name, ActualVar->string);
		ActualVar->flags &= ~CV_SHOWMODIFONETIME;
	}
	ActualVar->flags |= CV_MODIFIED;
	// raise 'on change' code
	if (ActualVar->flags & CV_CALL)
		ActualVar->func();
}

//  does as if "<varname> <value>" is entered at the console
//
void CV_Set(consvar_t* var, char* value)
{
	//changed = strcmp(var->string, value);
	
	/*#ifdef PARANOIA
	   if (!var)
	   I_Error("CV_Set : no variable\n");
	   if (!var->string)
	   I_Error("cv_Set : %s no string set ?!\n", var->name);
	   #endif
	   if (strcasecmp(var->string, value) == 0)
	   return;                  // no changes */
	
	Setvalue(var, value);
}

//  Expands value to string before calling CV_Set ()
//
void CV_SetValue(consvar_t* var, int value)
{
	char val[32];
	
	sprintf(val, "%d", value);
	CV_Set(var, val);
}

void CV_AddValue(consvar_t* var, int increment)
{
	int newvalue = var->value + increment;
	
	if (var->PossibleValue)
	{
#define MIN 0
	
		if (strcmp(var->PossibleValue[MIN].strvalue, "MIN") == 0)
		{
			int max;
			
			// seach the next to last
			for (max = 0; var->PossibleValue[max + 1].strvalue != NULL; max++)
				;
				
			if (newvalue < var->PossibleValue[MIN].value)
				newvalue += var->PossibleValue[max].value - var->PossibleValue[MIN].value + 1;	// add the max+1
			newvalue = var->PossibleValue[MIN].value +
			(newvalue - var->PossibleValue[MIN].value) % (var->PossibleValue[max].value - var->PossibleValue[MIN].value + 1);
			
			CV_SetValue(var, newvalue);
		}
		else
		{
			int max, currentindice = -1, newindice;
			
			// this code do not support more than same value for differant PossibleValue
			for (max = 0; var->PossibleValue[max].strvalue != NULL; max++)
				if (var->PossibleValue[max].value == var->value)
					currentindice = max;
			max--;
#ifdef PARANOIA
			if (currentindice == -1)
				I_Error("CV_AddValue : current value %d not found in possible value\n", var->value);
#endif
			newindice = (currentindice + increment + max + 1) % (max + 1);
			CV_Set(var, var->PossibleValue[newindice].strvalue);
		}
	}
	else
		CV_SetValue(var, newvalue);
}

//  Allow display of variable content or change from the console
//
//  Returns false if the passed command was not recognised as
//  console variable.
//
static bool_t CV_Command(void)
{
	consvar_t* v;
	
	// check variables
	v = CV_FindVar(COM_Argv(0));
	if (!v)
		return false;
		
	// perform a variable print or set
	if (COM_Argc() == 1)
	{
		CONL_PrintF("\"%s\" is \"%s\" default is \"%s\"\n", v->name, v->string, v->defaultvalue);
		return true;
	}
	
	CV_Set(v, COM_Argv(1));
	return true;
}

//  Save console variables that have the CV_SAVE flag set but not CV_ALIAS
//
void CV_SaveVariables(FILE* f)
{
	consvar_t* cvar;
	
	for (cvar = consvar_vars; cvar; cvar = cvar->next)
		if (cvar->flags & CV_SAVE & !(cvar->flags & CV_ALIAS))
			fprintf(f, "%s \"%s\"\n", cvar->name, cvar->string);
}

//============================================================================
//                            SCRIPT PARSE
//============================================================================

//  Parse a token out of a string, handles script files too
//  returns the data pointer after the token
static char* COM_Parse(char* data)
{
	int c;
	int len;
	
	len = 0;
	com_token[0] = 0;
	
	if (!data)
		return NULL;
		
// skip whitespace
skipwhite:
	while ((c = *data) <= ' ')
	{
		if (c == 0)
			return NULL;		// end of file;
		data++;
	}
	
// skip // comments
	if (c == '/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
	}
// handle quoted strings specially
	if (c == '\"')
	{
		data++;
		for (;;)
		{
			c = *data++;
			if (c == '\"' || !c)
			{
				com_token[len] = 0;
				return data;
			}
			com_token[len] = c;
			len++;
		}
	}
// parse single characters
	if (c == '{' || c == '}' || c == ')' || c == '(' || c == '\'' || c == ':')
	{
		com_token[len] = c;
		len++;
		com_token[len] = 0;
		return data + 1;
	}
// parse a regular word
	do
	{
		com_token[len] = c;
		data++;
		len++;
		c = *data;
		if (c == '{' || c == '}' || c == ')' || c == '(' || c == '\'' || c == ':')
			break;
	}
	while (c > 32);
	
	com_token[len] = 0;
	return data;
}
