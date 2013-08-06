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
// DESCRIPTION: Virtual Machine

/***************
*** INCLUDES ***
***************/

#include "t_vm.h"
#include "dstrings.h"
#include "console.h"
#include "z_zone.h"

/****************
*** CONSTANTS ***
****************/

#define TVMPP_COMMENT		UINT16_C(0x0001)	// In Comment
#define TVMPP_ESCAPE		UINT16_C(0x0002)	// Escaped Sequence
#define TVMPP_QUOTE			UINT16_C(0x0004)	// Quoted String

/* TVM_TokenType_t -- Type of token thing is */
typedef enum TVM_TokenType_e
{
	TVMTT_INVALID,								// Invalid
	TVMTT_SYMBOL,								// Symbol
	TVMTT_IDENTIFIER,							// Identifier
	TVMTT_STRING,								// String
	TVMTT_INTEGER,								// Integer
	TVMTT_FIXED,								// Fixed
	TVMTT_DECLARE,								// Declare Word
	
	NUMTVMTOKENTYPES
} TVM_TokenType_t;

/* TVM_ScopeFlags_t -- Flags for scopes */
typedef enum TVM_ScopeFlags_e
{
	TVMSF_EXTERN		= UINT32_C(0x00000001),	// External Visible Scope
} TVM_ScopeFlags_t;

/*****************
*** STRUCTURES ***
*****************/

typedef bool_t (*TVM_ParseMode_t)(struct TVM_State_s* const a_State, char* const a_Buf, const size_t a_Len);

/* TVM_TokenChain_t -- Chains of tokens */
typedef struct TVM_TokenChain_s
{
	TVM_TokenType_t Type;						// Type of token
	char* Text;									// Token text
	
	struct TVM_TokenChain_s* Prev;				// Previous Link
	struct TVM_TokenChain_s* Next;				// Next Link
	
	struct TVM_TokenChain_s* First;				// First link
} TVM_TokenChain_t;

/* TVM_State_t -- State */
typedef struct TVM_State_s
{
	TVM_Namespace_t NameSpace;					// Namespace
	char* ErrorStr;								// Error string
	int32_t ScopeDepth;							// Depth of Scope
	TVM_ParseMode_t ParseMode;					// Current Parse Mode
	TVM_ParseMode_t Confused;					// Confused mode
	int8_t ReadInclude;							// Include read count
	int32_t ReadCond;							// Conditionals Read
	int32_t CondStack;							// Conditional parenthesis stack
	const WL_WADEntry_t* IncludeThis;			// Includes this file
	const WL_WADEntry_t** Incs;					// Stack of includes
	size_t NumIncs;								// Number of them
	TVM_TokenChain_t* TokChain;					// Token Chain
	char CondType;								// Type of condition
	uint32_t ScopeIdent;						// Scope Identity
	int32_t ReadScript;							// Script Reading
	uint8_t ScriptID;							// ID of script
} TVM_State_t;

/****************
*** CONSTANTS ***
****************/

static const struct
{
	const char* Token;							// Token for operator
	bool_t Backwards;							// Backwards expression
} c_VMOps[] =
{
	{"=", true},
	{"||", false},
	{"&&", false},
	{"|", false},
	{"&", false},
	{"==", false},
	{"!=", false},
	{"<", false},
	{">", false},
	{"<=", false},
	{">=", false},
	{"+", false},
	{"-", false},
	{"*", false},
	{"/", false},
	{"%", false},
	{"!", false},
	{"++", false},
	{"--", false},
	{".", false},
	
	{NULL}
};

/*************
*** LOCALS ***
*************/

//static TVM_State_t l_VMState[NUMTVMNAMESPACES];	// Current Map State

/****************
*** FUNCTIONS ***
****************/

/* TVMS_IsSpace() -- Returns true if character is space */
static bool_t TVMS_IsSpace(const char a_Char)
{
	switch (a_Char)
	{
		case ' ':
		case '\r':
		case '\t':
			return true;
		
		default:
			return false;
	}
}

/* TVMS_GetTokenType() -- Returns the type of the token passed */
static TVM_TokenType_t TVMS_GetTokenType(const char* const a_Buf)
{
	char* p;
	int32_t i;
	
	/* Setup */
	p = a_Buf;
	
	/* End? */
	if (!*p)
		return TVMTT_INVALID;
	
	/* Perform checks */
	// Special
	if (!strcmp(a_Buf, "hub") || !strcmp(a_Buf, "const") ||
		!strcmp(a_Buf, "string") || !strcmp(a_Buf, "int") ||
		!strcmp(a_Buf, "mobj") || !strcmp(a_Buf, "float") ||
		!strcmp(a_Buf, "fixed"))
		return TVMTT_DECLARE;
	
	// Some kind of integer
	else if (*p >= '0' && *p <= '9')
	{
		// Count number of decimal points
		for (i = 0, p += 1; *p; p++)
			if (*p == '.')
				i++;
			
			// Contains non-numbers
			else if (*p < '0' || *p > '9')
				return TVMTT_INVALID;
		
		// If 1, return fixed
		if (i == 1)
			return TVMTT_FIXED;
		
		// More than 1, is illegal
		else if (i > 1)
			return TVMTT_INVALID;
		
		// Otherwise, is an integer
		return TVMTT_INTEGER;
	}
	
	// Identifier
	else if (*p == '_' || (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z'))
	{
		// Make sure the rest is valid
		for (p += 1; *p; p++)
			if (!(*p == '_' || (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9')))
				return TVMTT_INVALID;
		
		// Return as identifier
		return TVMTT_IDENTIFIER;
	}
	
	// String
	else if (*p == '\"')
		return TVMTT_STRING;
	
	// Symbol
	else
		return TVMTT_SYMBOL;
	
	/* Failure */
	return TVMTT_INVALID;
}

/* TVMS_AppendToken() -- Appends token to chain */
static TVM_TokenChain_t* TVMS_AppendToken(TVM_TokenChain_t* const a_Chain, const char* const a_Token)
{
	TVM_TokenChain_t* New, *Last;
	
	/* No chain */
	if (!a_Chain)
		Last = NULL;
	else
		// Go to last bit
		for (Last = a_Chain; Last->Next; Last = Last->Next)
			;
	
	/* Allocate New */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Setup based on last */
	// It is around
	if (Last)
	{
		Last->Next = New;
		New->Prev = Last;
		New->First = Last->First;
	}
	
	// Not around
	else
	{
		New->First = New;
	}
	
	/* Setup Word */
	New->Text = Z_StrDup(a_Token, PU_STATIC, NULL);
	New->Type = TVMS_GetTokenType(New->Text);
	
	/* Always return the first chain */
	return New->First;
}

/* TVMS_ClearChain() -- Clears token chain */
static void TVMS_ClearChain(TVM_TokenChain_t* const a_Chain)
{
	TVM_TokenChain_t* Rover, *Next;
	
	/* Check */
	if (!a_Chain)
		return;
	
	/* Clear everything */
	Next = NULL;
	for (Rover = a_Chain->First; Rover; Rover = Next)
	{
		// Remember Next
		Next = Rover->Next;
		
		// Delete
		Z_Free(Rover->Text);
		Z_Free(Rover);
	}
}

/* TVMS_PutBuffer() -- Puts text into buffer */
static void TVMS_PutBuffer(char** const a_BufRef, size_t* const a_SizeRef, size_t* const a_MaxRef, const char a_Char)
{
#define GROWSIZE 32
	/* No buffer? */
	if (!*a_BufRef)
	{
		*a_BufRef = Z_Malloc(sizeof(**a_BufRef) * GROWSIZE, PU_STATIC, NULL);
		*a_MaxRef = GROWSIZE;
	}
	
	/* Too small? */
	else if (*a_SizeRef >= *a_MaxRef - 2)
	{
		Z_ResizeArray((void**)a_BufRef, sizeof(**a_BufRef),
			*a_MaxRef, *a_MaxRef + GROWSIZE);
		*a_MaxRef += GROWSIZE;
	}
	
	/* Place */
	(*a_BufRef)[(*a_SizeRef)++] = a_Char;
#undef GROWSIZE
}

/* TVMS_PushScope() -- Pushes new scope to the VM */
static bool_t TVMS_PushScope(struct TVM_State_s* const a_State, const char* const a_Name, const uint32_t a_Flags)
{
	CONL_PrintF(">> PUSH %s (%08x)\n", a_Name, a_Flags);
	a_State->ScopeDepth++;
	return true;
}

/* TVMS_PopScope() -- Pops the last scope from the VM */
static bool_t TVMS_PopScope(struct TVM_State_s* const a_State, char* const a_Dest, const size_t a_Len)
{
	CONL_PrintF("<< POP\n");
	a_State->ScopeDepth--;
	return true;
}

/* TVMS_CreateVar() -- Creates a variable */
static bool_t TVMS_CreateVar(struct TVM_State_s* const a_State, char* const a_Type, const char* const a_Name, TVM_CCVarInfo_t** const a_Info)
{
	/* Check */
	if (!a_Info)
	{
		a_State->ErrorStr = "No info passed";
		return false;
	}	
	
	CONL_PrintF("?? Declare `%s` as type `%s`\n", a_Name, a_Type);
	return true;
}

/* TVMS_LSEval() -- Evaluation of LegacyScript expression */
static bool_t TVMS_LSEval(struct TVM_State_s* const a_State, TVM_TokenChain_t** a_List, int32_t a_Count, int32_t a_Start, int32_t a_End, TVM_CCVarInfo_t* const a_PlaceHere)
{
	int32_t i;
	
	/* Debug */
	for (i = a_Start; i <= a_End; i++)
		CONL_PrintF("~%s ", a_List[i]->Text);
	CONL_PrintF("\n");
	
	/* Possible Parenthesis that are not needed */
	// TODO FIXME
	
	
	
	return false;
}

/* TVMS_SolveExpr() -- Solves an expression */
// This is a mostly direct port of t_parse.c evaluate_expression() and friends.
// This is just for script compatibility (since we do not want all the Legacy mods
// to break into pieces).
static bool_t TVMS_SolveExpr(struct TVM_State_s* const a_State, TVM_TokenChain_t* const a_Chain)
{
	TVM_TokenChain_t* Rover;
	TVM_TokenChain_t** List;
	TVM_TokenChain_t* DeclareAs;
	int32_t Count;
	bool_t RetVal;
	TVM_CCVarInfo_t* Var;
	
	/* Convert expression to list */
	// Init
	List = NULL;
	Count = 0;
	DeclareAs = NULL;
	Var = NULL;
	
	// Rove and append to list
	for (Rover = a_Chain; Rover; Rover = Rover->Next)
	{
		// Current Token is a reserved variable word
		if (Rover->Type == TVMTT_DECLARE)
		{
			// Doubly declared?
			if (DeclareAs)
			{
				if (List)
					Z_Free(List);
				
				a_State->ErrorStr = "It is illegal to declare a variable named after a reserved word.";
				return false;
			}	
			
			DeclareAs = Rover;
		}
		
		// Grab declare
		else
		{
			// Declaring?
			if (DeclareAs)
			{
				// Not an indentifier
				if (Rover->Type != TVMTT_IDENTIFIER)
				{
					if (List)
						Z_Free(List);
				
					a_State->ErrorStr = "Variable name is not a valid identifier.";
					return false;
				}
				
				// Create new variable
				if (!TVMS_CreateVar(a_State, DeclareAs->Text, Rover->Text, &Var))
				{
					if (List)
						Z_Free(List);
					return false;
				}
				
				// Not declaring anymore
				DeclareAs = NULL;
			}
			
			// Reallocate List
			Z_ResizeArray((void**)&List, sizeof(*List), Count, Count + 1);
		
			// Set current
			List[Count++] = Rover;
		}
	}
	
	/* Call sub handler */
	RetVal = TVMS_LSEval(a_State, List, Count, 0, Count - 1, Var);
	
	/* Cleanup */
	if (List)
		Z_Free(List);
	return RetVal;
}

bool_t TVMS_CompileWLESIntExt(TVM_State_t* const a_State, WL_ES_t* const a_Stream, const uint32_t a_End);

/* TVMS_PM_Include() -- Include directive */
static bool_t TVMS_PM_Include(struct TVM_State_s* const a_State, char* const a_Buf, const size_t a_Len)
{
	WL_ES_t* IncStr;
	bool_t RetVal;
	size_t i;
	
	/* Based on inclusion state... */
	switch (a_State->ReadInclude)
	{
			// Expect '('
		case 0:
			if (strcmp(a_Buf, "("))
			{
				a_State->ErrorStr = "Expected \'(\' after include.";
				return false;
			}
			
			a_State->ReadInclude++;
			return true;
			
			// Expect string, containing said include
		case 1:
			// Check for ", which deems an include
			if (a_Buf[0] != '\"')
			{
				a_State->ErrorStr = "Expected quoted string in include.";
				return false;
			}
			
			// Cheat Buffer (remove quotes)
			memmove(&a_Buf[0], &a_Buf[1], a_Len);
			a_Buf[a_Len - 2] = 0;
			
			// Locate Include
			a_State->IncludeThis = WL_FindEntry(NULL, 0, a_Buf);
			
			// Check if it exists
			if (!a_State->IncludeThis)
			{
				a_State->ErrorStr = "Include lump not found.";
				return false;
			}
			
			// Move on
			a_State->ReadInclude++;
			return true;
			
			// Expect ')'
		case 2:
			if (strcmp(a_Buf, ")"))
			{
				a_State->ErrorStr = "Expected \')\' after include.";
				return false;
			}
			
			a_State->ReadInclude++;
			return true;
			
			// Expect ';'
		case 3:
			if (strcmp(a_Buf, ";"))
			{
				a_State->ErrorStr = "Expected \';\' at end of include statement.";
				return false;
			}
			
			// Make sure it is not on the stack
			for (i = 0; i < a_State->NumIncs; i++)
				if (a_State->Incs[i] == a_State->IncludeThis)
				{
					a_State->ErrorStr = "Recursive include detected.";
					return false;
				}
			
			// Push to stack
			Z_ResizeArray((void**)&a_State->Incs, sizeof(*a_State->Incs),
				a_State->NumIncs, a_State->NumIncs + 1);
			a_State->Incs[a_State->NumIncs++] = a_State->IncludeThis;
			
			// Return state to normal
			a_State->ParseMode = a_State->Confused;
			
			// Parse said include now, using whatever it returns
			IncStr = WL_StreamOpen(a_State->IncludeThis);
			RetVal = TVMS_CompileWLESIntExt(a_State, IncStr, a_State->IncludeThis->Size);
			WL_StreamClose(IncStr);
			
			// Remove from stack
			a_State->Incs[a_State->NumIncs - 1] = NULL;
			Z_ResizeArray((void**)&a_State->Incs, sizeof(*a_State->Incs),
				a_State->NumIncs, a_State->NumIncs - 1);
			a_State->NumIncs--;
			
			// Change message?
			if (!RetVal)
				a_State->ErrorStr = "Error in include file.";
			return RetVal;
	}
	
	/* Oops? */
	return false;
}

/* TVMS_PM_Cond() -- Conditional */
static bool_t TVMS_PM_Cond(struct TVM_State_s* const a_State, char* const a_Buf, const size_t a_Len)
{
	int32_t OldStack;
	
	/* First read */
	if (a_State->ReadCond++ == 0)
	{
		// Not a '('?
		if (strcmp(a_Buf, "("))
		{
			a_State->ErrorStr = "Expected \'(\' after conditional statement.";
			return false;
		}
		
		// Increase parenthesis stack
		a_State->CondStack++;
		return true;
	}
	
	/* Possibly modify stack count */
	OldStack = a_State->CondStack;
	
	// Increase in stack
	if (!strcmp(a_Buf, "("))
		a_State->CondStack++;
	
	// Decrease in stack
	else if (!strcmp(a_Buf, ")"))
	{
		a_State->CondStack--;
		
		// End of if statement?
		if (a_State->CondStack <= 0)
		{
			// TODO FIXME
			a_State->ErrorStr = "End-cond TODO FIXME";
			return false;
		}
	}
	
	// TODO FIXME
	a_State->ErrorStr = "Conditional TODO FIXME";
	return false;
}

/* TVMS_PM_Script() -- Script */
static bool_t TVMS_PM_Script(struct TVM_State_s* const a_State, char* const a_Buf, const size_t a_Len)
{
#define BUFSIZE 32
	char Buf[BUFSIZE];	
	TVM_TokenType_t TokType;
	int32_t IntVal;
	
	/* Reading first, integer */
	if (a_State->ReadScript == 0)
	{
		// Obtain type of token
		TokType = TVMS_GetTokenType(a_Buf);
	
		// Not an integer?
		if (TokType != TVMTT_INTEGER)
		{
			a_State->ErrorStr = "Script IDs are integers only.";
			return false;
		}
	
		// Obtain integer value
		IntVal = C_strtoi32(a_Buf, NULL, 10);
	
		// Out of range?
		if (IntVal < 0 || IntVal >= 255)
		{
			a_State->ErrorStr = "Script IDs may only be within 0 - 255 inclusive.";
			return false;
		}
		
		// Set ID, and wait for next token
		a_State->ScriptID = IntVal;
		a_State->ReadScript++;
		return true;
	}
	
	/* Read curly brace */
	else if (a_State->ReadScript == 1)
	{
		// Expect '{'
		if (strcmp("{", a_Buf))
		{
			a_State->ErrorStr = "Expected '{' after script ID.";
			return false;
		}
		
		// Emit warnings in global namespace
		if (a_State->NameSpace == TVMNS_GLOBAL)
			CONL_OutputUT(CT_SCRIPTING, DSTR_TVMC_GLOBALSCRIPTNUM, "%u\n", (uint8_t)a_State->ScriptID);
	
		// Setup script name
		memset(Buf, 0, sizeof(Buf));
		snprintf(Buf, BUFSIZE, "@___script_%02x", (uint8_t)a_State->ScriptID);
	
		// Push it
		TVMS_PushScope(a_State, Buf, TVMSF_EXTERN);
	
		// Return to confused state
		a_State->ParseMode = a_State->Confused;
	
		// Success!
		return true;
	}
	
	/* Failure */
	return false;
#undef BUFSIZE
}

/* TVMS_PM_Confused() -- Confused state (default) */
static bool_t TVMS_PM_Confused(struct TVM_State_s* const a_State, char* const a_Buf, const size_t a_Len)
{
#define BUFSIZE 32
	char Buf[BUFSIZE];	
	bool_t SolveOK;
	
	/* Special keywords */
		// Include
	if (!strcmp("include", a_Buf))
	{
		// Start parsing include
		a_State->ReadInclude = 0;
		a_State->ParseMode = TVMS_PM_Include;
		return true;
	}
	
		// Script
	else if (!strcmp("script", a_Buf))
	{
		// Make sure no tokens were read
		if (a_State->TokChain)
		{
			a_State->ErrorStr = "Missing \';\' before \'}\'.";
			return false;
		}
		
		// Disallow scripts inside of a scope
		if (a_State->ScopeDepth > 0)
		{
			a_State->ErrorStr = "Scripts may only exist at the top level block.";
			return false;
		}
		
		// Start parsing script
		a_State->ReadScript = 0;
		a_State->ParseMode = TVMS_PM_Script;
		
		// Define a new code point
		return true;
	}
		
		// if/while/for/else
	else if (!strcmp("if", a_Buf) || !strcmp("while", a_Buf) || !strcmp("for", a_Buf) || !strcmp("else", a_Buf))
	{
		// Make sure no tokens were read
		if (a_State->TokChain)
		{
			a_State->ErrorStr = "Missing \';\' before \'}\'.";
			return false;
		}
		
		// Switch to conditional
		a_State->CondType = a_Buf[0];
		a_State->ReadCond = 0;
		a_State->CondStack = 0;
		a_State->ParseMode = TVMS_PM_Cond;
		
		// Continue
		return true;
	}
	
		// Start of scope
	else if (!strcmp("{", a_Buf))
	{
		// Make sure no tokens were read
		if (a_State->TokChain)
		{
			a_State->ErrorStr = "Missing \';\' before \'}\'.";
			return false;
		}
		
		// Setup scope name
		memset(Buf, 0, sizeof(Buf));
		snprintf(Buf, BUFSIZE, "@___scope_%08x", (uint32_t)(++a_State->ScopeIdent));

		// Push it -- but it is NOT external (not jumpable from remote)
		TVMS_PushScope(a_State, Buf, 0);
	}
	
		// End of scope
	else if (!strcmp("}", a_Buf))
	{
		// Make sure no tokens were read
		if (a_State->TokChain)
		{
			a_State->ErrorStr = "Missing \';\' before \'}\'.";
			return false;
		}
		
		// Already at zero scope?
		if (a_State->ScopeDepth == 0)
		{
			a_State->ErrorStr = "Extra \'}\', causes negative scope depth.";
			return false;
		}
		
		// Fall out of scope
		TVMS_PopScope(a_State, NULL, 0);
	}
	
	/* Parse statements into code */
	// If ; was read, this is an end of a statement
	else if (!strcmp(";", a_Buf))
	{
		// Solve expression if tokens waiting
		if (a_State->TokChain)
		{
			// Attempt it
			a_State->ErrorStr = NULL;
			SolveOK = TVMS_SolveExpr(a_State, a_State->TokChain);
			
			// Clear chain
			TVMS_ClearChain(a_State->TokChain);
			a_State->TokChain = NULL;
			
			// Failed?
			if (!SolveOK)
			{
				// Change string if not any
				if (!a_State->ErrorStr)
					a_State->ErrorStr = "Invalid expression.";
				return false;
			}
		}
	}
	
	// Otherwise, append to the chain
	else
	{
		// Append
		a_State->TokChain = TVMS_AppendToken(a_State->TokChain, a_Buf);
	}
	
	/* Success! */
	return true;
#undef BUFSIZE
}

/* TVMS_ParseToken() -- Parses token into the state */
static bool_t TVMS_ParseToken(TVM_State_t* const a_State, char* const a_Buf, const size_t a_Len)
{
	//CONL_PrintF("(%i)`%s`, ", (int)a_Len, a_Buf);
	return a_State->ParseMode(a_State, a_Buf, a_Len);
}

/* TVMS_CompileLine() -- Compiles a line (tokenizes it) */
// In Legacy Script, tokens are either identifiers, strings, numbers, or symbols.
static bool_t TVMS_CompileLine(TVM_State_t* const a_State, char* const a_Buf, const size_t a_Len)
{
	char* Token, *p, *ep;
	size_t TokAt, TokSz;
	int8_t Type;
	bool_t SendAway, Escape;
	bool_t RetVal;
	
	/* Init */
	Token = NULL;
	TokAt = TokSz = 0;
	Type = -1;
	SendAway = false;
	Escape = false;
	RetVal = true;
	
	/* Read characters */
	for (p = a_Buf, ep = a_Buf + a_Len; p < ep;)
	{
		// Initialize Token State
		if (!Token)
		{
			// Ignore whitespace
			if (TVMS_IsSpace(*p))
			{
				p++;
				continue;
			}
			
			// Number
			if ((*p >= '0' && *p <= '9') || *p == '.')
				Type = 1;
			
			// Identifier
			else if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '_')
				Type = 2;
			
			// String
			else if (*p == '\"')
				Type = 3;
			
			// Symbol
			else
				Type = 4;
			
			// Add to token
			TVMS_PutBuffer(&Token, &TokAt, &TokSz, *(p++));
			continue;
		}
		
		// Determne if token ends
			// Number
		if (Type == 1)
		{
			if (!((*p >= '0' && *p <= '9') || *p == '.'))
				SendAway = true;
		}
		
			// Identifier
		else if (Type == 2)
		{
			if (!((*p >= 'a' && *p <= 'z') ||
				(*p >= 'A' && *p <= 'Z') ||
				(*p >= '0' && *p <= '9') || *p == '_'))
				SendAway = true;
		}
		
			// String
		else if (Type == 3)
		{
			// Not escaped and quoted?
			if (!Escape && *p == '\"')
			{
				TVMS_PutBuffer(&Token, &TokAt, &TokSz, *(p++));
				SendAway = true;
			}
			
			// Escape?
			Escape = false;
			if (*p == '\\')
				Escape = true;
		}
		
			// Symbol
		else
		{
			// Double symbol?
			if (*p != '(' && *p != ')')
				if (*p == '=' || *p == Token[TokAt - 1])
					TVMS_PutBuffer(&Token, &TokAt, &TokSz, *(p++));
			
			// Always send away
			SendAway = true;
		}
		
		// Send away?
		if (SendAway)
		{
			// Call token handler thing
			if (!TVMS_ParseToken(a_State, Token, TokAt))
				RetVal = false;
			
			// Re-initialize token bits
			Z_Free(Token);
			Token = NULL;
			TokAt = TokSz = 0;
			Type = -1;
			SendAway = false;
			
			// Failed?
			if (!RetVal)
				break;
		}
		
		// Otherwise add to token
		else
			TVMS_PutBuffer(&Token, &TokAt, &TokSz, *(p++));
	}
	
	/* A token remains */
	if (Token)
	{
		if (!TVMS_ParseToken(a_State, Token, TokAt))
			RetVal = false;
		
		// Free it
		Z_Free(Token);
	}
	
	/* It worked? */
	return RetVal;
}

/* TVMS_CompileWLESInt() -- Compiles stream segment */
// This is based off my C99 preprocessor, at least the tokenization part.
// However compared to C, this is alot smaller and alot faster too.
// It already is fast, but since Legacy Script lacks so many things there's no
// code to execute.
// So compared to C, there is no...
//  * Trigraphs
//  * Multi-line Comments
//  * Escaped Newlines
static bool_t TVMS_CompileWLESInt(TVM_State_t* const a_State, WL_ES_t* const a_Stream, const uint32_t a_End)
{
#define EATCHAR {ReadLeft--; memmove(&ReadBuf[0], &ReadBuf[1], sizeof(*ReadBuf) * (READSIZE - 1));}
#define READSIZE 32
	char ReadBuf[READSIZE];
	uint32_t ReadLeft;
	uint16_t Mode;
	uint32_t LineNum;
	
	char* Token;
	size_t TokAt, TokSz;
	
	/* Check */
	if (!a_Stream || !a_End)
		return false;
	
	/* Message */
	CONL_OutputUT(CT_SCRIPTING, DSTR_TVMC_COMPILING, "%s\n", WL_StreamGetEntry(a_Stream)->Name);
	
	/* Initialize */
	ReadLeft = Mode = TokAt = 0;
	memset(ReadBuf, 0, sizeof(ReadBuf));
	Token = NULL;
	LineNum = 1;
	
	/* Tokenization Loop */
	while ((!WL_StreamEOF(a_Stream) && WL_StreamTell(a_Stream) < a_End) || ReadLeft > 0)
	{
		// Read Into Buffer
		while (ReadLeft < READSIZE - 1)
			if (!WL_StreamEOF(a_Stream) && WL_StreamTell(a_Stream) < a_End)
				ReadBuf[ReadLeft++] = WL_Src(a_Stream);
			else
			{
				ReadBuf[ReadLeft] = 0;
				break;
			}
		
		// Nothing read?
		if (ReadLeft == 0)
			break;
		
		// If not in quotes, transform whitespace to space
		if ((Mode & TVMPP_QUOTE) == 0)
			if (TVMS_IsSpace(ReadBuf[0]))
				ReadBuf[0] = ' ';
		
		// If not in quotes and multiple spaces...
		if ((Mode & TVMPP_QUOTE) == 0)
			while (ReadBuf[0] == ' ' && ReadBuf[1] == ' ')
				EATCHAR;
				
		// Detect Comment
		if ((Mode & TVMPP_QUOTE) == 0)
			if (ReadBuf[0] == '/' && ReadBuf[1] == '/')
			{
				// Set as comment mode
				Mode |= TVMPP_COMMENT;
				
				// Eat two characters and move on
				EATCHAR;
				EATCHAR;
				continue;
			}
		
		// Detect Quotes
		if ((Mode & TVMPP_COMMENT) == 0)
			if ((Mode & TVMPP_ESCAPE) == 0)
				if (ReadBuf[0] == '\"')
					Mode ^= TVMPP_QUOTE;
		
		// Handle Newline
		if (ReadBuf[0] == '\n')
		{
			// Eat newline
			EATCHAR;
			
			// Increase Line Number
			LineNum++;
			
			// Clear comment and escapedness, if any
			Mode &= ~(TVMPP_COMMENT | TVMPP_ESCAPE);
			
			// Send to tokenizer
			if (Token && TokAt > 0)
				if (!TVMS_CompileLine(a_State, Token, TokAt))
				{
					CONL_OutputUT(CT_SCRIPTING, DSTR_TVMC_ERROR, "%u%s\n", LineNum, a_State->ErrorStr);
					
					// Free token before death
					if (Token)
						Z_Free(Token);
					return false;
				}
			
			// Clear Token
			if (Token)
				Z_Free(Token);
			Token = NULL;
			TokAt = TokSz = 0;
			
			// Continue on
			continue;
		}
		
		// Escape sequence
		Mode &= ~TVMPP_ESCAPE;
		if (ReadBuf[0] == '\\')
			Mode |= TVMPP_ESCAPE;
				
		// Single-line comments, ignore the remaining
		if (Mode & TVMPP_COMMENT)
		{
			EATCHAR;
			continue;
		}
		
		// Otherwise append to token buffer
		TVMS_PutBuffer(&Token, &TokAt, &TokSz, ReadBuf[0]);
		
		// Eat character
		EATCHAR;
	}
	
	/* Clear Token */
	if (Token)
		Z_Free(Token);
	
#undef READSIZE
#undef EATCHAR
	
	/* Success! */
	return true;
}

/* TVMS_CompileWLESIntExt() -- Externalized internal call */
bool_t TVMS_CompileWLESIntExt(TVM_State_t* const a_State, WL_ES_t* const a_Stream, const uint32_t a_End)
{
	return TVMS_CompileWLESInt(a_State, a_Stream, a_End);
}

/* TVMS_Cleanup() -- Cleans up */
static void TVMS_Cleanup(TVM_State_t* const a_State)
{
	// Left over includes?
	if (a_State->Incs)
	{
		Z_Free(a_State->Incs);
		a_State->Incs = NULL;
		a_State->NumIncs = 0;
	}
	
	// Left over tokens?
	if (a_State->TokChain)
		TVMS_ClearChain(a_State->TokChain);
}

/* TVM_CompileWLES() -- Wraps internal */
void TVM_CompileWLES(const TVM_Namespace_t a_NameSpace, WL_ES_t* const a_Stream, const uint32_t a_End)
{
	TVM_State_t State;
	
	/* Check valid namespace */
	if (a_NameSpace < 0 || a_NameSpace >= NUMTVMNAMESPACES)
		return;
	
	/* Clear */
	memset(&State, 0, sizeof(State));
	
	/* Init for new compile stage */
	State.NameSpace = a_NameSpace;
	State.ParseMode = TVMS_PM_Confused;
	State.Confused = TVMS_PM_Confused;
	State.ScopeDepth = 0;
	
	/* Start compilation */
	if (TVMS_CompileWLESInt(&State, a_Stream, a_End))
		CONL_OutputUT(CT_SCRIPTING, DSTR_TVMC_SUCCESS, "%s\n", WL_StreamGetEntry(a_Stream)->Name);
	
	/* Cleanup */
	TVMS_Cleanup(&State);
}

/* TVM_Clear() -- Clears the VM */
void TVM_Clear(const TVM_Namespace_t a_NameSpace)
{
#if 0
	/* Check namespace */
	if (a_NameSpace < 0 || a_NameSpace >= NUMTVMNAMESPACES)
		return;
	
	/* Reset scope identity */
	l_VMState[a_NameSpace].ScopeIdent = 1;
	
	/* Cleanup */
	TVMS_Cleanup(&l_VMState[a_NameSpace]);
#endif
}

/* TVM_CompileEntry() -- Compiles entry */
void TVM_CompileEntry(const TVM_Namespace_t a_NameSpace, const WL_WADEntry_t* const a_Entry)
{
	WL_ES_t* ScriptStream;
	
	/* Open stream */
	ScriptStream = WL_StreamOpen(a_Entry);
	
	/* If it was created */
	if (ScriptStream)
	{
		// Check Unicode viability
		WL_StreamCheckUnicode(ScriptStream);
		
		// Compile it
		TVM_CompileWLES(a_NameSpace, ScriptStream, a_Entry->Size);
		
		// Close it
		WL_StreamClose(ScriptStream);
	}
}

