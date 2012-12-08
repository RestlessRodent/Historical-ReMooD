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
// Copyright (C) 2011-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: ReMooD Scripting Virtual Machine (Core Machine Execution)


/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "t_dsvm.h"
#include "doomstat.h"
#include "console.h"

/****************
*** CONSTANTS ***
****************/

/* T_VMExprType_t -- Expression Types */
typedef enum T_VMExprType_e
{
	TVMET_NULL,									// NULL expression
	TVMET_DECLARETYPE,							// Declare variable type
	TVMET_DECLAREVAR,							// Declare variable	name
	TVMET_IDENT,								// Identifier of sorts
	TVMET_FUNCTION,								// Function
	TVMET_OPERATOR,								// Operator
	TVMET_OPENPAR,								// Open Parenthesis
	TVMET_CLOSEPAR,								// Close Parenthesis
	TVMET_NUMBER,								// A Number
	TVMET_STRING,								// A string
	TVMET_FUNCARGSPLIT,							// Splits function arguments
	TVMET_COMMA,								// Comma
	
	NUMTVMEXPRTYPES
} T_VMExprType_t;

/*****************
*** STRUCTURES ***
*****************/

struct T_VMFunc_s;

/* T_VMVarType_t -- Variable Type Handler */
typedef struct T_VMVarType_s
{
	const char* Name;							// Variable Type Name
} T_VMVarType_t;

/* T_VMVariable_t -- VM Variable */
typedef struct T_VMVariable_s
{
	char* Name;									// Variable Name
	const T_VMVarType_t* Handler;				// Type Handler
} T_VMVariable_t;

/* T_VMLabel_t -- VM Label */
typedef struct T_VMLabel_s
{
	char* Name;									// Name
	struct T_VMFunc_s* Func;					// Function that owns this
	uint32_t ExecPtr;							// Execution Point
} T_VMLabel_t;

/* T_VMFunc_t -- VM Function */
typedef struct T_VMFunc_s
{
	/* Function Properties */
	char* FuncName;								// Name of function
	struct T_VMFunc_s* Parent;					// Parent function (scope)
	
	/* Visible Variables */
	T_VMVariable_t** Vars;						// Variables
	size_t NumVars;								// Number of variables
	T_VMLabel_t** Labels;						// Function labels
	size_t NumLabels;							// Number of labels
	
	/* Links */
	struct T_VMFunc_s* Prev;					// Previous function
	struct T_VMFunc_s* Next;					// Next function
	
	/* Function Execution Data */
	uint32_t CodeSize;							// Size of code
} T_VMFunc_t;

/* T_VMExpr_t -- VM Expression */
typedef struct T_VMExpr_s
{
	char* Token;								// Token Value
	T_VMExprType_t Type;						// Type of expression
	const T_VMVarType_t* TypeNameHandle;		// Declaration hander
	int32_t ParNum;								// Parenthesis Number
	int32_t CommaCount;							// Comma number encountered
	T_VMVariable_t* ActualVar;					// Actual Variable
	T_VMVariable_t* DumpVar;					// Dump Variable
} T_VMExpr_t;

/* T_VMExprHold_t -- Expression holder */
typedef struct T_VMExprHold_s
{
	T_VMExpr_t* VMExpr;							// Expressions
	size_t NumVMExpr;							// Number of VM expressions
} T_VMExprHold_t;

/****************
*** CONSTANTS ***
****************/

// c_VMTypes -- Virtual machine data types
static const T_VMVarType_t c_VMTypes[] =
{
	{"int"},
	{"hub"},
	{"const"},
	{"fixed"},
	{"mobj"},
	{"string"},
	
	{NULL},
};

// c_VMReserved -- Reserved VM words
static const char* const c_VMReserved[] =
{
	"int", "hub", "const", "fixed", "mobj", "string", "script", "function",
	"include",
	
	NULL, NULL
};

/*************
*** LOCALS ***
*************/

static int32_t l_ScopeDepth = 0;				// Depth of scopes

static const WL_WADEntry_t** l_IncStack = NULL;	// Include Stack
static size_t l_NumIncStack = 0;				// Thing on the stack

static T_VMFunc_t* l_VMFirstFunc = NULL;		// First Function
static T_VMFunc_t* l_VMCurFunc = NULL;			// Current Virtual Function

/****************
*** FUNCTIONS ***
****************/

/* T_DSVM_ScriptError() -- Error script */
void T_DSVM_ScriptError(const char* const a_Str, const uint32_t a_Line)
{
	/* Play sound and print message */
	S_StartSound(NULL, sfx_lotime);
	CONL_PrintF("Script Error (%s%s%u): %s\n",
			(l_NumIncStack ? l_IncStack[l_NumIncStack - 1]->Name : ""),
			(l_NumIncStack ? ":" : ""),
			a_Line,
			a_Str
		);
}

/* TS_VMInjectLabel() -- Inject label at call point */
static T_VMLabel_t* TS_VMInjectLabel(const char* const a_Name, T_VMFunc_t* const a_Parent)
{
	T_VMLabel_t* New;
	
	/* Check */
	if (!a_Name)
		return NULL;
	
	/* Allocate New Variable */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Add to back of function variables */
	Z_ResizeArray((void**)&l_VMCurFunc->Labels, sizeof(*l_VMCurFunc->Labels),
		l_VMCurFunc->NumLabels, l_VMCurFunc->NumLabels + 1);
	l_VMCurFunc->Labels[l_VMCurFunc->NumLabels++] = New;
	
	/* Fill variable info */
	New->Name = Z_StrDup(a_Name, PU_STATIC, NULL);
	New->Func = a_Parent;
	New->ExecPtr = a_Parent->CodeSize;
	
	/* Debug */
	if (devparm)
		CONL_PrintF("SCRIPT: Created label \"%s\" (%s) @ %#06x\n", New->Name, l_VMCurFunc->FuncName, New->ExecPtr);
	
	/* Return it */
	return New;
}

/* TS_VMCreateFunc() -- Creates a function */
static T_VMFunc_t* TS_VMCreateFunc(const char* const a_Name, T_VMFunc_t* const a_Parent)
{
	T_VMFunc_t* New;
	
	/* Check */
	if (!a_Name)
		return NULL;
	
	/* Allocate */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Initialize */
	// Name and parent
	New->FuncName = Z_StrDup(a_Name, PU_STATIC, NULL);
	New->Parent = a_Parent;
	
	/* Chain */
	if (!l_VMFirstFunc)
		l_VMFirstFunc = New;
	else
	{
		l_VMFirstFunc->Prev = New;
		New->Next = l_VMFirstFunc;
		l_VMFirstFunc = New;
	}
	
	/* Debug */
	if (devparm)
		CONL_PrintF("SCRIPT: Created function \"%s\"\n", New->FuncName);
	
	/* Return it */
	return New;
}

/* TS_VMCreateVar() -- Creates a variable */
static T_VMVariable_t* TS_VMCreateVar(const char* const a_Name, const T_VMVarType_t* const a_Handler)
{
	T_VMVariable_t* New;
	
	/* Check */
	if (!a_Name)
		return NULL;
	
	/* Allocate New Variable */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Add to back of function variables */
	Z_ResizeArray((void**)&l_VMCurFunc->Vars, sizeof(*l_VMCurFunc->Vars),
		l_VMCurFunc->NumVars, l_VMCurFunc->NumVars + 1);
	l_VMCurFunc->Vars[l_VMCurFunc->NumVars++] = New;
	
	/* Fill variable info */
	New->Name = Z_StrDup(a_Name, PU_STATIC, NULL);
	New->Handler = a_Handler;
	
	/* Debug */
	if (devparm)
		CONL_PrintF("SCRIPT: Created variable \"%s\" (%s)\n", New->Name, l_VMCurFunc->FuncName);
	
	/* Return it */
	return New;
}

/* TS_VMClearExpr() -- Clears loaded expression */
static void TS_VMClearExpr(T_VMExprHold_t* const a_Hold)
{
	size_t i;
	
	/* Free Data Inside */
	for (i = 0; i < a_Hold->NumVMExpr; i++)
	{
		// Token
		if (a_Hold->VMExpr[i].Token)
			Z_Free(a_Hold->VMExpr[i].Token);
	}
	
	/* Free expr list */
	if (a_Hold->VMExpr)
		Z_Free(a_Hold->VMExpr);
	a_Hold->VMExpr = NULL;
	a_Hold->NumVMExpr = NULL;
	
	/* Debug */
	//if (devparm)
	//	CONL_PrintF("--\n");
}

/* TS_VMPushExpr() -- Push to expression */
static T_VMExpr_t* TS_VMPushExpr(T_VMExprHold_t* const a_Hold, const char* const a_Token)
{
	size_t i;
	
	/* Check */
	if (!a_Token)
		return NULL;
	
	/* Resize */
	Z_ResizeArray((void**)&a_Hold->VMExpr, sizeof(*a_Hold->VMExpr), a_Hold->NumVMExpr, a_Hold->NumVMExpr + 1);
	i = a_Hold->NumVMExpr++;
	
	/* Fill Info */
	a_Hold->VMExpr[i].Token = Z_StrDup(a_Token, PU_STATIC, NULL);
	
	/* Debug */
	//if (devparm)
	//	CONL_PrintF("++\"%s\"\n", a_Hold->VMExpr[i].Token);
	
	/* Return it */
	return &a_Hold->VMExpr[i];
}

/* TS_VMSolveExpr() -- Attempts solving expression */
// This creates actual code, handles functions such as jumping, etc.
static bool_t TS_VMSolveExpr(T_VMExprHold_t* const a_Hold, const char* const a_ScopeFunc, T_VMFunc_t* const a_VMFunc)
{
#define BUFSIZE 512
	static uint32_t TempVarNum;
	char Buf[BUFSIZE];
	size_t i, t, r;
	T_VMExpr_t* This;
	int32_t ParStack, HighPar;
	bool_t DidDeclType;
	
	/* No Expression Loaded? */
	if (!a_Hold->NumVMExpr)
		return true;
	
	/* Pre-Process Expressions */
	DidDeclType = false;
	for (i = 0; i < a_Hold->NumVMExpr; i++)
	{
		// Get this
		This = &a_Hold->VMExpr[i];
		
		// Declaring Variable Type?
		if (This->Type == TVMET_NULL)
			for (t = 0; c_VMTypes[t].Name; t++)
				if (strcasecmp(c_VMTypes[t].Name, This->Token) == 0)
				{
					// Did not declare type
					if (!DidDeclType)
					{
						DidDeclType = true;
						This->Type = TVMET_DECLARETYPE;
						This->TypeNameHandle = &c_VMTypes[t];
					}
					
					break;
				}
		
		// Create variable?
		if (This->Type == TVMET_NULL)
			if (i > 0)
				if (a_Hold->VMExpr[i - 1].Type == TVMET_DECLARETYPE)
				{
					// Check reserved words
					for (r = 0; c_VMReserved[r]; r++)
						if (strcasecmp(This->Token, c_VMReserved[r]) == 0)
							break;
					
					// Reserved?
					if (c_VMReserved[r])
					{
						T_DSVM_ScriptError("Using reserved word as variable name.", 0);
						return false;
					}
					
					// Set as declaration
					This->Type = TVMET_DECLAREVAR;
				}
		
		// Alpha-numeric identifier
		if (This->Type == TVMET_NULL)
			if ((This->Token[0] >= 'a' && This->Token[0] <= 'z') ||
				(This->Token[0] >= 'A' && This->Token[0] <= 'Z') ||
				(This->Token[0] == '_'))
				This->Type = TVMET_IDENT;
		
		// Parenthesis?
		if (This->Type == TVMET_NULL)
			if (strcasecmp(This->Token, "(") == 0)
				This->Type = TVMET_OPENPAR;
			else if (strcasecmp(This->Token, ")") == 0)
				This->Type = TVMET_CLOSEPAR;
		
		// String?
		if (This->Type == TVMET_NULL)
			if (This->Token[0] == '\"')
				This->Type = TVMET_STRING;
		
		// Number
		if (This->Type == TVMET_NULL)
			if (This->Token[0] >= '0' && This->Token[0] <= '9')
				This->Type = TVMET_NUMBER;
		
		// Function split?
		if (This->Type == TVMET_NULL)
			if (strcasecmp(This->Token, ",") == 0)
				This->Type = TVMET_FUNCARGSPLIT;
		
		// Everything else is an operator
		if (This->Type == TVMET_NULL)
			This->Type = TVMET_OPERATOR;
	}
	
	/* Process Again */
	// Parenthesis
		// This groups parenthesis statements together in a fashion.
		// This helps on the order to operate things on in the future.
	HighPar = 0;
	for (i = 0; i < a_Hold->NumVMExpr; i++)
	{
		// Get this
		This = &a_Hold->VMExpr[i];
		
		// Modify parenthesis stack?
		if (This->Type == TVMET_OPENPAR)
			ParStack++;
		else if (This->Type == TVMET_CLOSEPAR)
			ParStack--;
		
		// Inside Parenthesis
		if (i > 0)
		{
			if (a_Hold->VMExpr[i - 1].Type == TVMET_OPENPAR)
				ParStack++;
			else if (a_Hold->VMExpr[i - 1].Type == TVMET_CLOSEPAR)
				ParStack--;
		}
		
		// Greater?
		if (ParStack + 1 >= HighPar)
			HighPar = ParStack + 1;
		
		// Set Parenthesis ID
		This->ParNum = ParStack;
		
		// Bad Parenthesis Stack?
		if (ParStack < 0)
		{
			T_DSVM_ScriptError("Too many closing parenthesis.", 0);
			return false;
		}
	}
	
	// Function arguments (commas)
		// This groups comma selectors together, this is used by the next parse
		// step to group function arguments into their own sections.
	ParStack = 0;
	for (i = 0; i < a_Hold->NumVMExpr; i++)
	{
		// Get this
		This = &a_Hold->VMExpr[i];
		
		if (a_Hold->VMExpr[i].Type == TVMET_FUNCARGSPLIT)
			ParStack++;
		
		// Comma operator
		if (i > 0 && a_Hold->VMExpr[i - 1].Type == TVMET_FUNCARGSPLIT)
			ParStack++;
		
		// Set comma count
		This->CommaCount = ParStack;
	}
	
	// Split parenthetical counts with commas
		// This runs through parenthesis lists and creates a similarly grouped
		// sections of which to evaluate expressions with.
	ParStack = 0;
	for (i = 0; i < a_Hold->NumVMExpr; i++)
	{
		// Get this
		This = &a_Hold->VMExpr[i];
		
		//
	}
	
#if 0
		// Declaring variable?
			// Create with this name
		if (This->Type == TVMET_DECLAREVAR)
			This->ActualVar = TS_VMCreateVar(This->Token, NULL);
		
		// Operator?
			// Create variable for temporary operation
		if (This->Type == TVMET_OPERATOR)
		{
			++TempVarNum;
			snprintf(Buf, BUFSIZE - 1, "#__tempvar_%u", TempVarNum);
			This->DumpVar = TS_VMCreateVar(Buf, a_Hold->VMExpr[i - 1].TypeNameHandle);
		}
#endif
	
	/* Debug */
	if (devparm)
	{
#define EXTRAROOM 4
		// Print Token
		for (i = 0; i < a_Hold->NumVMExpr; i++)
		{
			// Get this
			This = &a_Hold->VMExpr[i];
		
			// Token
			CONL_PrintF("%*s ", strlen(This->Token) + EXTRAROOM, This->Token);
		}
		CONL_PrintF("\n");
		
		// Other things
		for (i = 0; i < a_Hold->NumVMExpr; i++)
		{
			// Get this
			This = &a_Hold->VMExpr[i];
		
			// Token
			CONL_PrintF("%*i ", strlen(This->Token) + EXTRAROOM, This->ParNum);
		}
		CONL_PrintF("\n");
		
		// Type
		for (i = 0; i < a_Hold->NumVMExpr; i++)
		{
			// Get this
			This = &a_Hold->VMExpr[i];
		
			// Token
			CONL_PrintF("%*c ", strlen(This->Token) + EXTRAROOM, This->Type + 'A');
		}
		CONL_PrintF("\n");
		
		// Comma Count
		for (i = 0; i < a_Hold->NumVMExpr; i++)
		{
			// Get this
			This = &a_Hold->VMExpr[i];
		
			// Token
			CONL_PrintF("%*i ", strlen(This->Token) + EXTRAROOM, This->CommaCount);
		}
		CONL_PrintF("\n");
#undef EXTRAROOM
	}
	
	/* Start Solving */
	
	/* Success! */
	return true;
#undef BUFSIZE
}

/* T_DSVM_ReadToken() -- Reads Token */
bool_t T_DSVM_ReadToken(WL_ES_t* const a_Stream, const size_t a_End, uint32_t* const a_Row, char* const a_Buf, const size_t a_BufSize)
{
	bool_t ReadSomething;
	char Char;
	size_t o;
	int8_t Mode;
	bool_t ReadString, Ident, Number, Symbol;
	bool_t DidPeriod, EscapeQuote;
	
	/* Clear Buffer */
	memset(a_Buf, 0, sizeof(*a_Buf) * a_BufSize);
	ReadSomething = false;		// Read something at least
	o = 0;
	
	/* Read Loop */
	Mode = 0;
	ReadString = Ident = Number = Symbol = DidPeriod = EscapeQuote = false;
	while ((Char = WL_Src(a_Stream)))
	{
		// EOS?
		if (!Char || WL_StreamEOF(a_Stream) || WL_StreamTell(a_Stream) >= a_End)
			return ReadSomething;
		
		// Ignore Whitespace
		if ((!ReadString && (Char == ' ' || Char == '\t')) || (Char == '\r' || Char == '\n'))
		{
			// New Row?
			if (Char == '\n')
				if (a_Row)
					*a_Row += 1;
			
			// If something was read, return it
			if (ReadSomething)
				return true;
			
			// Otherwise continue
			continue;
		}
		
		// Ignore Comments
		if (!ReadString && (Char == '/'))
		{
			// Read Next Char
			Char = WL_Src(a_Stream);
			
			// End of read?
			if (!Char || WL_StreamEOF(a_Stream) || WL_StreamTell(a_Stream) >= a_End)
				return ReadSomething;
			
			// If it is a '/' it is a comment, otherwise it is division
			if (Char == '/')
			{
				// Continue until '\n'
				while (Char = WL_Src(a_Stream))
				{
					// End of read?
					if (!Char || WL_StreamEOF(a_Stream) || WL_StreamTell(a_Stream) >= a_End)
						return ReadSomething;
					
					// New line?
					if (Char == '\n')
					{
						if (a_Row)
							*a_Row += 1;
						break;
					}
				}
				
				// Return if we read something
				if (ReadSomething)
					return ReadSomething;
				
				// Otherwise continue
				continue;	// A full comment line
			}
			
			// It isn't, so it is division
			else
			{
				// If it is not a symbol, go back one
				// This is for `foo+2/5-1` like cases
				if ((Ident || Number) && !Symbol)
				{
					WL_StreamSeek(a_Stream, WL_StreamTell(a_Stream) - 1, false);
					return ReadSomething;
				}
				
				// Otherwise it is a full blown free operator
				else
				{
					if (o < a_BufSize)
						a_Buf[o++] = Char;
					ReadSomething = true;
					Symbol = true;
					return true;
				}
			}
		}
		
		// Reading something, but not the same something?
		if (!ReadString &&
			((Ident && !((Char >= 'a' && Char <= 'z') || (Char >= 'A' && Char <= 'Z') || (Char >= '0' && Char <= '9') || (Char == '_'))) ||
			(Number && !((Char >= '0' && Char <= '9') || (Char == '.' && !DidPeriod))) ||
			(Symbol)))
		{
			// Backup and return what was read
			WL_StreamSeek(a_Stream, WL_StreamTell(a_Stream) - 1, false);
			return ReadSomething;
		}
		
		// Copy Character to Buffer
		if (o < a_BufSize)
			a_Buf[o++] = Char;
		ReadSomething = true;
		
		// Number period?
		if (Number && Char == '.')
			DidPeriod = true;
			
		// End of string?
		if (ReadString && !EscapeQuote && Char == '\"')
			return ReadSomething;
		
		// Check for string escape
		if (ReadString)
		{
			if (EscapeQuote)
				EscapeQuote = false;
			else if (Char == '\\')
				EscapeQuote = true;
		}
		
		// Not reading anything, set associated flags
		if (!Ident && !Number && !Symbol && !ReadString)
		{
			// Identifier
			if ((Char >= 'a' && Char <= 'z') || (Char >= 'A' && Char <= 'Z') || (Char == '_'))
				Ident = true;
			
			// Number
			else if (Char >= '0' && Char <= '9')
				Number = true;
			
			// String
			else if (Char == '\"')
				ReadString = true;
			
			// Otherwise it is a symbol
			else
				Symbol = true;
		}
		
		// Return symbols always
		if (Symbol)
			return ReadSomething;
	}
	
	/* No more tokens */
	return false;
}

/* T_DSVM_CompileStream() -- Compiles a stream */
bool_t T_DSVM_CompileStream(WL_ES_t* const a_Stream, const size_t a_End)
{
#define MAXSCOPES 32
#define BUFSIZE 512
	char Buf[BUFSIZE];
	char ExtraBuf[BUFSIZE];
	uint32_t Row;
	bool_t ScriptProblem, QuickRet, TraversedScope;
	const WL_WADEntry_t* Entry;
	WL_ES_t* IncStream;
	int32_t i, j, n, s;
	
	uint32_t ScopeStack[MAXSCOPES], u32;
	uint8_t ScopePos;
	static uint32_t SScopeID;
	
	T_VMExprHold_t BootExpr;

	/* Check */
	if (!a_Stream)
		return false;
	
	/* Initialize some things */
	memset(&BootExpr, 0, sizeof(BootExpr));
	memset(ScopeStack, 0, sizeof(ScopeStack));
	ScopePos = 0;
	
	/* Debug */
	if (devparm)
		CONL_PrintF("T_DSVM_CompileStream: Compiling...\n");
	
	/* Always push to the include stack */
	// This helps prevent `include("MAP01");`
	Z_ResizeArray((void**)&l_IncStack, sizeof(*l_IncStack), l_NumIncStack, l_NumIncStack + 1);
	l_IncStack[l_NumIncStack++] = WL_StreamGetEntry(a_Stream);
	
	/* Constantly Read Tokens */
	ScriptProblem = false;
	while (T_DSVM_ReadToken(a_Stream, a_End, &Row, Buf, BUFSIZE - 1))
	{
		// Debug
		if (devparm)
			CONL_PrintF("Tok: `%s`\n", Buf);
		
		// Handle Includes
		if (strcasecmp(Buf, "include") == 0)
		{
			// Out of scope?
			if (l_ScopeDepth > 0)
			{
				T_DSVM_ScriptError("Include inside function or script.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Read token, and expect a '('
			QuickRet = T_DSVM_ReadToken(a_Stream, a_End, &Row, Buf, BUFSIZE - 1);
			if (!QuickRet || (QuickRet && strcasecmp(Buf, "(") != 0))
			{
				T_DSVM_ScriptError("Expected `(` to follow include.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Read String
			QuickRet = T_DSVM_ReadToken(a_Stream, a_End, &Row, Buf, BUFSIZE - 1);
			n = strlen(Buf);
			if (!QuickRet || (QuickRet && (Buf[0] != '\"' || n <= 1 || (n > 1 && Buf[n - 1] != '\"'))))
			{
				T_DSVM_ScriptError("Includes must be double quoted as strings.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Trim Quotes
			memset(ExtraBuf, 0, sizeof(ExtraBuf));
			strncpy(ExtraBuf, Buf + 1, strlen(Buf) - 2);
			
			// Read token, and expect a ')'
			QuickRet = T_DSVM_ReadToken(a_Stream, a_End, &Row, Buf, BUFSIZE - 1);
			if (!QuickRet || (QuickRet && strcasecmp(Buf, ")") != 0))
			{
				T_DSVM_ScriptError("Expected `)` to follow include.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Read token, and expect a ';'
			QuickRet = T_DSVM_ReadToken(a_Stream, a_End, &Row, Buf, BUFSIZE - 1);
			if (!QuickRet || (QuickRet && strcasecmp(Buf, ";") != 0))
			{
				T_DSVM_ScriptError("Expected `;` to follow include.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Attempt location of include
			Entry = WL_FindEntry(NULL, 0, ExtraBuf);
			
			// Not found?
			if (!Entry)
			{
				// Change all periods to underscores
				n = strlen(ExtraBuf);
				for (i = 0; i < n; i++)
					if (ExtraBuf[i] == '.')
						ExtraBuf[i] = '_';
				
				// Try again
				Entry = WL_FindEntry(NULL, 0, ExtraBuf);
				
				// Did it fail twice?
				if (!Entry)
				{
					T_DSVM_ScriptError("Include not found.", Row);
					T_DSVM_ScriptError(ExtraBuf, Row);
					ScriptProblem = true;
					break;
				}
			}
			
			// Make sure it isn't on the stack
			for (i = 0; i < l_NumIncStack; i++)
				if (l_IncStack[i] == Entry)
					break;
			
			// It IS on there
			if (i < l_NumIncStack)
			{
				T_DSVM_ScriptError("Recursive include detected.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Open stream on it
			IncStream = WL_StreamOpen(Entry);
			
			// Failed to open?
			if (!IncStream)
			{
				T_DSVM_ScriptError("Failed to open include stream.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Call script parser on it
			QuickRet = T_DSVM_CompileStream(IncStream, Entry->Size);
			
			// Close Stream
			WL_StreamClose(IncStream);
			
			// Something bad happened when including?
			if (!QuickRet)
			{
				T_DSVM_ScriptError("Script error while including.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Continue onto the next token
			continue;
		}
		
		// Script Number
		else if (strcasecmp(Buf, "script") == 0)
		{
			// Script inside a scope?
			if (l_ScopeDepth > 0)
			{
				T_DSVM_ScriptError("Script inside another script or function.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Read script number, and expect digits
			QuickRet = T_DSVM_ReadToken(a_Stream, a_End, &Row, Buf, BUFSIZE - 1);
			n = strlen(Buf);
			for (i = 0; i < n; i++)
				if (Buf[i] < '0' || Buf[i] > '9')
					break;
			if (!QuickRet || i < n)
			{
				T_DSVM_ScriptError("Expected a number to follow script.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Get the script number
			s = strtol(Buf, NULL, 10);
			
			// Negative?
			if (s < 0)
			{
				T_DSVM_ScriptError("Script identifiers cannot be negative.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Expect a '{' now
			QuickRet = T_DSVM_ReadToken(a_Stream, a_End, &Row, Buf, BUFSIZE - 1);
			if (!QuickRet || (QuickRet && strcasecmp(Buf, "{") != 0))
			{
				T_DSVM_ScriptError("Expected `{` to follow script identifier.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Create function for script
			snprintf(ExtraBuf, BUFSIZE - 1, "@__script_%i", s);
			l_VMCurFunc = TS_VMCreateFunc(ExtraBuf, l_VMCurFunc);
			l_ScopeDepth++;
			
			// Set into stack
			ScopeStack[ScopePos++] = ~s;
			
			// Exceeded scope depth?
			if (ScopePos >= MAXSCOPES)
			{
				T_DSVM_ScriptError("Exceeded maximum scope depth.", Row);
				ScriptProblem = true;
				break;
			}
		}
		
		// Custom Function
		else if (strcasecmp(Buf, "function") == 0)
		{
			// Script inside a scope?
			if (l_ScopeDepth > 0)
			{
				T_DSVM_ScriptError("Function inside another script or function.", Row);
				ScriptProblem = true;
				break;
			}
		}
		
		// Scope Termination
		else if (strcasecmp(Buf, "}") == 0)
		{
			// Pop from stack (place function end marker here?)
			u32 = ScopeStack[ScopePos - 1];
			snprintf(ExtraBuf, BUFSIZE - 1, "@__endscope_pre_%i", u32);
			TS_VMInjectLabel(ExtraBuf, l_VMCurFunc);
			
			ScopePos--;
			
			// Go Up
			l_VMCurFunc = l_VMCurFunc->Parent;
			l_ScopeDepth--;
			
			// Went way outside scope?
			if (l_ScopeDepth < 0 || !l_VMCurFunc)
			{
				T_DSVM_ScriptError("Went outside the global scope.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Exceeded scope depth?
			if (ScopePos >= MAXSCOPES)
			{
				T_DSVM_ScriptError("Exceeded maximum scope depth.", Row);
				ScriptProblem = true;
				break;
			}
			
			// Place label after in standard function area
			snprintf(ExtraBuf, BUFSIZE - 1, "@__endscope_%i", u32);
			TS_VMInjectLabel(ExtraBuf, l_VMCurFunc);
		}
		
		// Standard Expression
		else
		{
			// Init expression
			TS_VMClearExpr(&BootExpr);
			TraversedScope = false;
			
			// Expression Loop
			do
			{
				// ; or { is end of expression
				if (strcasecmp(Buf, ";") == 0 || strcasecmp(Buf, "{") == 0)
				{
					// Sub-block?
					if (strcasecmp(Buf, "{") == 0)
					{
						// Create function for scope
						// Expressions should be evaluation before scopes but they
						// need to know which scope to jump to. So an if parsed
						// will either call @__scope_ or just jump to @__endscope_
						u32 = ++SScopeID;
						snprintf(ExtraBuf, BUFSIZE - 1, "@__scope_%i", u32);
						TraversedScope = true;
					}
					
					QuickRet = true;
					break;
				}
				
				// Push to stack
				TS_VMPushExpr(&BootExpr, Buf);
			} while ((QuickRet = T_DSVM_ReadToken(a_Stream, a_End, &Row, Buf, BUFSIZE - 1)));
			
			// Failed?
			if (!QuickRet)
			{
				TS_VMClearExpr(&BootExpr);
				T_DSVM_ScriptError("Expected end of expression.", Row);
				ScriptProblem = true;
				return;
			}
			
			// Try solving it
			QuickRet = TS_VMSolveExpr(&BootExpr, ExtraBuf, l_VMCurFunc);
			
			// Failed to solve?
			if (!QuickRet)
			{
				TS_VMClearExpr(&BootExpr);
				T_DSVM_ScriptError("Could not solve expression.", Row);
				ScriptProblem = true;
				return;
			}
			
			// Call traverses a scope `if (something) {`
			if (TraversedScope)
			{ 
				l_VMCurFunc = TS_VMCreateFunc(ExtraBuf, l_VMCurFunc);
				l_ScopeDepth++;
			
				// Set into stack
				ScopeStack[ScopePos++] = u32;

				// Exceeded scope depth?
				if (ScopePos >= MAXSCOPES)
				{
					T_DSVM_ScriptError("Exceeded maximum scope depth.", Row);
					ScriptProblem = true;
					break;
				}
				
				// Place label after in standard function area
				snprintf(ExtraBuf, BUFSIZE - 1, "@__scope_init_%i", u32);
				TS_VMInjectLabel(ExtraBuf, l_VMCurFunc);
			}
			
			// Clear it
			TS_VMClearExpr(&BootExpr);
		}
	}
	
	/* Pop from stack */
	// Stack will be empty? Delete it instead
	if (l_NumIncStack == 1)
	{
		if (l_IncStack)
			Z_Free(l_IncStack);
		l_IncStack = NULL;
	}
	
	// Otherwise, resize down
	else
		Z_ResizeArray((void**)&l_IncStack, sizeof(*l_IncStack), l_NumIncStack, l_NumIncStack - 1);
	
	// Decrement
	l_NumIncStack--;
	
	/* Success? */
	return !ScriptProblem;
#undef BUFSIZE
}

/* T_DSVM_Cleanup() -- Cleanup */
bool_t T_DSVM_Cleanup(void)
{
	size_t v;
	T_VMFunc_t* Rover, *Next;
	T_VMVariable_t* ThisVar;
	T_VMLabel_t* ThisLabel;
	
	/* Reset Vars */
	l_ScopeDepth = 0;
	
	/* Free up things */
	// Stack
	if (l_IncStack)
		Z_Free(l_IncStack);
	l_IncStack = NULL;
	l_NumIncStack = 0;
	
	// Functions
	l_VMCurFunc = NULL;
	
	for (Rover = l_VMFirstFunc; Rover; Rover = Next)
	{
		// Get Next
		Next = Rover->Next;
		
		// Clear function info
		Z_Free(Rover->FuncName);
		
		// Clear variables
		if (Rover->Vars)
		{
			// Free stuff in variables
			for (v = 0; v < Rover->NumVars; v++)
			{
				// Get current
				ThisVar = Rover->Vars[v];
				
				// Free info
				if (ThisVar->Name)
					Z_Free(ThisVar->Name);
				
				// Free the actual variable
				Z_Free(ThisVar);
			}
			
			// Free list
			Z_Free(Rover->Vars);
		}
		
		// Clear labels
		if (Rover->Labels)
		{
			// Free stuff in variables
			for (v = 0; v < Rover->NumLabels; v++)
			{
				// Get current
				ThisLabel = Rover->Labels[v];
				
				// Free info
				if (ThisLabel->Name)
					Z_Free(ThisLabel->Name);
				
				// Free the actual variable
				Z_Free(ThisLabel);
			}
			
			// Free list
			Z_Free(Rover->Labels);
		}
		
		// Clear function itself
		Z_Free(Rover);
	}
	
	// Expression
	//TS_VMClearExpr(&BootExpr);
	
	/* Initialize for new script */
	// Create global function
	l_VMCurFunc = l_VMFirstFunc = NULL;
	l_VMCurFunc = TS_VMCreateFunc("@__global", NULL);
	
	/* Success! */
	return true;
}

