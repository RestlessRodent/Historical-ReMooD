// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.org/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           TetrisMaster512        (tetrismaster512@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 2008 ReMooD Team.
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// Legacy Script compiler

/***************
*** INCLUDES ***
***************/

#include "t_comp.h"

/****************
*** CONSTANTS ***
****************/

/* TLD_BlockType_t -- Type of block */
typedef enum TLS_BlockType_e
{
	BT_STANDARD,												// Global or local
	BT_SCRIPT,													// script ## { }
	BT_IF,														// if() { }
	BT_ELSEIF,													// elseif() { }
	BT_ELSE,													// else() { }
	BT_WHILE,													// while() { }
	BT_FOR,														// for() { }
} TLS_BlockType_t;

/*************
*** LOCALS ***
*************/

boolean l_DoCompile = false;
boolean l_ScriptDebug = false;									// Debug scripts

/****************
*** FUNCTIONS ***
****************/


static int Deepness;	// Deepness brace wise
static TLS_BlockType_t PushBlockType;	// Block type to push next
static boolean HubVar;		// Make hub variable?
static boolean BoostHubVar;	// Boosted hub variable
static TLS_VariableType_t NextVar;	// next variable type
static boolean Skip = false;		// Skip token
static char Blocks[512];
static char LastBlock[2] = {'A', 'A'};
static int Deeps[64];

/* TLS_ClearScripts() -- Clear old scripts */
boolean TLS_ClearScripts(void)
{
	/* Clear flags */
	// Compiling a [scripts] section
	l_DoCompile = false;
	
	Deepness = 0;
	memset(Blocks, 0, sizeof(Blocks));
	LastBlock[0] = 'A';
	LastBlock[1] = 'A';
	Skip = false;
	
	return true;
}

/* TLS_TokenData() -- Tokenize something */
char* TLS_TokenData(const char* const Data, const size_t Size, void** const TokData)
{
#define BUFSIZE 512
	static char BeefBuf[BUFSIZE];
	char** Seek;
	size_t i;
	int Mode;
	boolean Break;
	boolean Escaped;
	char c;
	
	/* Check */
	if (!Data || !TokData)
		return NULL;
	
	/* No token data? */
	if (!(*TokData))
	{
		// Create it
		*TokData = Z_Malloc(sizeof(char*), PU_STATIC, NULL);
		*TokData = Data;
	}
	
	// Set seek
	Seek = TokData;
	
	/* Reached end? */
	if (*Seek - Data >= Size)
		return NULL;
	
	/* Clear data */
	i = 0;
	Mode = 0;
	Break = false;
	Escaped = true;	// skip first "
	memset(BeefBuf, 0, sizeof(BeefBuf));
	
	/* Read in some characters */
	// What we put in the buffer depends on what we find
	while (!Break && *Seek - Data < Size)
	{
		// Single line comments '//.*$', skip to \n
		if (**Seek == '/' && *(*Seek + 1) == '/')
			while (**Seek != '\n')
				(*Seek)++;
		
		// No mode set
		if (!Mode)
		{
			// Skip whitespace
			if (**Seek == ' ' || **Seek == '\t' || **Seek == '\r' || **Seek == '\n')
			{
				(*Seek)++;
				continue;
			}
			
			// Alpha-numeric identifier ('[A-Za-z][A-Za-z0-9_]*')
			if ((**Seek >= 'a' && **Seek <= 'z') || (**Seek >= 'A' && **Seek <= 'Z'))
				Mode = 1;
			
			// String ('".*[^\\]"')
			else if (**Seek == '\"')
				Mode = 2;
			
			// Number
			else if (**Seek >= '0' && **Seek <= '9')
				Mode = 3;
			
			// Double type
			else if (**Seek == '=' || **Seek == '<' || **Seek == '>' || **Seek == '-' || **Seek == '+' ||
				*Seek == '!' || *Seek == '&' || *Seek == '|')
				Mode = 4;
		
			// Other single symbol
			else
				Mode = 999;
		}
		
		// Mode is set
		else
		{
			switch (Mode)
			{
					// Alpha-numeric identifier ('[A-Za-z][A-Za-z0-9_]*')
				case 1:
					if ((**Seek >= 'a' && **Seek <= 'z') || (**Seek >= 'A' && **Seek <= 'Z') ||
						(**Seek >= '0' && **Seek <= '9') || **Seek == '_')
					{
						// Copy to buffer
						if (i < BUFSIZE - 1)
							BeefBuf[i++] = **Seek;
						(*Seek)++;
					}
					else
					{
						// Breakout
						BeefBuf[i++] = 0;
						Break = true;
					}
					break;
					
					// String ('".*[^\\]"')
				case 2:
					// End of string and no escaped
					if (!Escaped && **Seek == '\"')
					{
						if (i < BUFSIZE - 1)
							BeefBuf[i++] = **Seek;
						(*Seek)++;
						Break = true;
						Escaped = false;
					}
					
					// Normal
					else
					{
						// Unescape always
						Escaped = false;
						
						// Set escape?
						if (**Seek == '\\')
							Escaped = true;
						
						// Copy
						if (i < BUFSIZE - 1)
							BeefBuf[i++] = **Seek;
						(*Seek)++;
					}
					break;
					
					// Number
				case 3:
					if ((**Seek >= '0' && **Seek <= '9') || **Seek == '.')
					{
						// Copy to buffer
						if (i < BUFSIZE - 1)
							BeefBuf[i++] = **Seek;
						(*Seek)++;
					}
					else
					{
						// Breakout
						BeefBuf[i++] = 0;
						Break = true;
					}
					break;
					
					// Double type
				case 4:
					// Clone
					switch (**Seek)
					{
							// Double
						case '=':
						case '&':
						case '|':
						case '+':
						case '-':
							// Get first
							BeefBuf[i++] = **Seek;
							c = **Seek;
							(*Seek)++;
							
							// If second matches, merge
							if (**Seek == c)
							{
								BeefBuf[i++] = **Seek;
								(*Seek)++;
							}
							
							// End off
							BeefBuf[i++] = 0;
							Break = true;
							break;
						
							// Non-Double
						default:
							switch (**Seek)
							{
									// Followed by =
								case '>':
								case '<':
								case '!':
									// Get first
									BeefBuf[i++] = **Seek;
									(*Seek)++;
							
									// If second is a =, merge
									if (**Seek == '=')
									{
										BeefBuf[i++] = **Seek;
										(*Seek)++;
									}
							
									// End off
									BeefBuf[i++] = 0;
									Break = true;
									break;
									
									// Not followed by =
								default:
									BeefBuf[0] = **Seek;
									BeefBuf[1] = 0;
									(*Seek)++;
									Break = true;
									break;
							}
							break;
					}
					break;
				
					// Single-character
				default:
					BeefBuf[0] = **Seek;
					BeefBuf[1] = 0;
					(*Seek)++;
					Break = true;
					break;
			}
		}
	}
	
	// Force clean chop
	BeefBuf[BUFSIZE - 1] = 0;
	
	return BeefBuf;
#undef BUFSIZE
}

/* TLS_ValidIdent() -- Valid identifier? */
boolean TLS_ValidIdent(char* Ident)
{
	int i;
	
	/* Check */
	if (!Ident)
		return false;
	
	/* Reserved? */
	if (strcasecmp(Ident, "const") == 0 || 
		strcasecmp(Ident, "else") == 0 || 
		strcasecmp(Ident, "elseif") == 0 || 
		strcasecmp(Ident, "fixed") == 0 || 
		strcasecmp(Ident, "float") == 0 || 
		strcasecmp(Ident, "for") == 0 || 
		strcasecmp(Ident, "hub") == 0 || 
		strcasecmp(Ident, "if") == 0 || 
		strcasecmp(Ident, "int") == 0 || 
		strcasecmp(Ident, "mobj") == 0 || 
		strcasecmp(Ident, "script") == 0 || 
		strcasecmp(Ident, "string") == 0 || 
		strcasecmp(Ident, "while") == 0)
		return false;
	
	/* Valid characters */
	for (i = 0; i < strlen(Ident); i++)
		if (!i)
		{
			if (!((Ident[i] >= 'a' && Ident[i] <= 'z') || (Ident[i] >= 'A' && Ident[i] <= 'Z')))
				return false;
		}
		else
		{
			if (!((Ident[i] >= 'a' && Ident[i] <= 'z') || (Ident[i] >= 'A' && Ident[i] <= 'Z') || (Ident[i] >= '0' && Ident[i] <= '9') || Ident[i] == '_'))
				return false;
		}
	
	/* Otherwise */
	return true;
}

/* TLS_ValidUInt() -- Valid unsigned number */
boolean TLS_ValidUInt(char* Ident)
{
	int i;
	
	/* Check */
	if (!Ident || !strlen(Ident))
		return false;
	
	/* Loop */
	for (i = 0; i < strlen(Ident); i++)
		if (!(Ident[i] >= '0' && Ident[i] <= '9'))
			return false;
	
	return true;
}

/* TLS_Deepen() -- Change deepness */
int TLS_Deepen(int Dir, int Deep)
{
	if (Dir > 0)
	{
		Deepness++;
		Blocks[strlen(Blocks)] = LastBlock[0];
		Blocks[strlen(Blocks)] = LastBlock[1]++;
		
		/*if (LastBlock[1] > 'Z')
		{
			LastBlock[0]++;
			LastBlock[1] = 'A';
		}*/
		LastBlock[0] = 'A';
		LastBlock[1] = 'A';
		
		Deeps[Deepness] = Deep;
		
		CONS_Printf("%%%% PUSHBLOCK \"%s\"\n", Blocks);
		
		Skip = false;
		return 0;
	}
	
	else if (Dir < 0)
	{
		Deepness--;
		
		if (Deepness < 0)
			return;
			
		LastBlock[1] = Blocks[strlen(Blocks) - 1] + 1;
		LastBlock[0] = Blocks[strlen(Blocks) - 2];
		Blocks[strlen(Blocks) - 1] = 0;
		Blocks[strlen(Blocks) - 1] = 0;
		
		if (LastBlock[1] > 'Z')
		{
			LastBlock[1] = 'A';
			LastBlock[0]++;
		}
		
		CONS_Printf("%%%% POPBLOCK\n");
		
		Skip = false;
		
		return Deeps[Deepness + 1];
	}
}

/* TLS_FillStatement() -- Expand a statement */
void TLD_FillStatement(char* Tokens, char* FinalRes)
{
	char* p;
	int i;
	
	for (i = 0, p = Tokens; *p; p += strlen(p) + 1, i++)
		CONS_Printf("> %i: %s\n", i, p);
}

/* TLS_IncrementalCompile() -- Compile recursive */
boolean TLS_IncrementalCompile(const WadIndex_t Index)
{
#define BUFSIZE 512
	char* Data = NULL;
	char* Token = NULL;
	void* TokData = NULL;
	char Buf[BUFSIZE];
	char Buf2[BUFSIZE];
	size_t Len = 0;
	size_t RecCount = 0;
	boolean Ok;
	int i, j, k;
	
	/* Cache lump data */
	Data = W_CacheLumpNum(Index, PU_STATIC);
	
	// Check
	if (!Data)
		return false;
		
	/* Debug Message */
	if (l_ScriptDebug)
		CONS_Printf("TLSD: Incremental compile %i.\n", Index);
		
	/* Recursive */
	// Too much?
	if (RecCount >= 8)
		return false;
	
	RecCount++;
		
	/* Token loop */
	Ok = true;
	Len = W_LumpLength(Index);
	while (Skip || (Token = TLS_TokenData(Data, Len, &TokData)))
	{
		/* Not OK */
		Ok = false;
		
		/* Block reference */
		if (strcasecmp(Token, "[") == 0)
		{
			// Get next token
			Token = TLS_TokenData(Data, Len, &TokData);
			
			if (!Token)
				break;
			
			// "scripts"?
			if (strcasecmp(Token, "scripts") == 0)
				l_DoCompile = true;
			else
				l_DoCompile = false;
			
			// Find until ]
			while ((Token = TLS_TokenData(Data, Len, &TokData)))
				if (strcasecmp(Token, "]") == 0)
				{
					// Feed Token
					Token = TLS_TokenData(Data, Len, &TokData);
					break;
				}
			
			if (!Token)
				break;
		}
		
		/* Not compiling scripts block (so ignore statement) */
		if (!l_DoCompile)
		{
			Ok = true;
			continue;
		}
		
		/* Check for include */
		if (strcasecmp(Token, "include") == 0)
		{
			// Get next token
			Token = TLS_TokenData(Data, Len, &TokData);
			
			// Expect (
			if (!Token || Token[0] != '(')
			{
				if (l_ScriptDebug)
					CONS_Printf("TLSD: Expected \'(\' after include (got \"%s\").\n", Token);
				break;
			}
			
			// Get next token
			Token = TLS_TokenData(Data, Len, &TokData);
			
			// Check if a string
			if (!Token || Token[0] != '\"')
			{
				if (l_ScriptDebug)
					CONS_Printf("TLSD: Expected string constant in include (got \"%s\").\n", Token);
				break;
			}
			
			// Copy
			strncpy(Buf, Token, BUFSIZE);
			
			// Strip quotes
			Buf[strlen(Buf) - 1] = 0;
			memmove(&Buf[0], &Buf[1], sizeof(char) * (strlen(Buf)));
			
			if (l_ScriptDebug)
				CONS_Printf("TLSD: Including \"%s\".\n", Buf);
			
			// Check if lump exists and if not, 's/\./_/g'
			if (W_CheckNumForName(Buf) == INVALIDLUMP)
			{
				if (l_ScriptDebug)
					CONS_Printf("TLSD: Lump does not exist, performing \'s/\\./_/g\'\n");
					
				// Now do it
				for (i = 0; i < strlen(Buf); i++)
					if (Buf[i] == '.')
						Buf[i] = '_';
			}
			
			// Run recursive
			if (!TLS_IncrementalCompile(W_CheckNumForName(Buf)))
				if (l_ScriptDebug)
					CONS_Printf("TLSD: Include failed.\n");
			
			// Get next token
			Token = TLS_TokenData(Data, Len, &TokData);
			
			// Expect )
			if (!Token || Token[0] != ')')
			{
				if (l_ScriptDebug)
					CONS_Printf("TLSD: Expected \')\' after include (got \"%s\").\n", Token);
				break;
			}
			
			// Get next token
			Token = TLS_TokenData(Data, Len, &TokData);
			
			// Expect ;
			if (!Token || Token[0] != ';')
			{
				if (l_ScriptDebug)
					CONS_Printf("TLSD: Expected \';\' after include (got \"%s\").\n", Token);
				break;
			}
				
			// Continue
			Ok = true;
			continue;
		}
		
		/* Long statement expansion */
		// Script
		else if (strcasecmp(Token, "script") == 0)
		{
			// Unset skip
			Skip = false;
			
			// Get script id
			Token = TLS_TokenData(Data, Len, &TokData);
			
			if (!Token || !TLS_ValidUInt(Token))
			{
				if (l_ScriptDebug)
					CONS_Printf("TLSD: Invalid script id \"%s\".\n", Token);
				break;
			}
			
			// Print label
			CONS_Printf("%%%% LABEL \"_SCRIPT_%04X\"\n", atoi(Token));
		}
		
		// Variable
		else if (strcasecmp(Token, "int") == 0 || strcasecmp(Token, "fixed") == 0 ||
			strcasecmp(Token, "string") == 0 || strcasecmp(Token, "mobj") == 0 ||
			strcasecmp(Token, "const") == 0 || strcasecmp(Token, "float") == 0)
		{
			// Copy type
			strcpy(Buf, Token);
			
			// Get identifier name
			Token = TLS_TokenData(Data, Len, &TokData);
			
			if (!Token || !TLS_ValidIdent(Token))
			{
				if (l_ScriptDebug)
					CONS_Printf("TLSD: Invalid identifier name \"%s\".\n", Token);
				break;
			}
			
			strcpy(Buf2, Token);
			
			// Stay on
			Skip = true;
			
			// Instruction
			CONS_Printf("%%%% DECL \"%s\", \"_%s_%s\"\n", Buf, Blocks, Buf2);
		}
		
		// Begin block
		else if (strcasecmp(Token, "{") == 0)
		{
			// Unset skip
			Skip = false;
			
			TLS_Deepen(1, 0);
		}
		
		// End block
		else if (strcasecmp(Token, "}") == 0)
		{
			// Unset skip
			Skip = false;
			
			i = TLS_Deepen(-1, 0);
		}
		
		// Conditional
		else if (strcasecmp(Token, "if") == 0 || strcasecmp(Token, "elseif") == 0 || strcasecmp(Token, "else") == 0)
		{
			// Unset skip
			Skip = false;
			
			// Deepen
			TLS_Deepen(1, 0);
			
			// Label start
			CONS_Printf("%%%% LABEL \"_%s_BEGINIF\"\n", Blocks);
			
			// Find (
			while (Token && strcasecmp(Token, "(") != 0)
				Token = TLS_TokenData(Data, Len, &TokData);
			Token = TLS_TokenData(Data, Len, &TokData);
			
			// Clear buf
			memset(Buf, 0, sizeof(Buf));
			memset(Buf2, 0, sizeof(Buf2));
			i = k = 0;
			
			// Until we reach a ), read tokens into buffer
			while (Token && ((k == 0 && strcasecmp(Token, ")") != 0) || (k > 0)))
			{
				// Is a (?
				if (strcasecmp(Token, "("))
					k++;
				if (strcasecmp(Token, ")"))
					k--;
				
				// Copy token over
				for (j = 0; j < strlen(Token) + 1; j++)
					Buf[i++] = Token[j];
				
				// Next token
				Token = TLS_TokenData(Data, Len, &TokData);
			}
			
			// Send buf over
			TLD_FillStatement(Buf, Buf2);
			
			CONS_Printf("? %s\n", Token);
			CONS_Printf("%%%% LABEL \"_%s_BEGINIF\"\n", Blocks);
			CONS_Printf("%%%% IF \"_%s\"\n", Buf2);
			Skip = true;
		}
		
		// Loop
		else if (strcasecmp(Token, "for") == 0 || strcasecmp(Token, "while") == 0)
		{
			// Unset skip
			Skip = false;
			
			// Deepen
			//TLS_Deepen(1, 0);
			
			// Label start
			CONS_Printf("%%%% LABEL \"_%s\"\n", Blocks);
			
			// Parse loop
			
			// Undeep
			//TLS_Deepen(-1, 0);
		}
		
		// Standard Statement
		else
		{
			// Unset skip
			Skip = false;
			
			// Clear buf
			memset(Buf, 0, sizeof(Buf));
			memset(Buf2, 0, sizeof(Buf2));
			i = 0;
			
			// Until we reach a ;, read tokens into buffer
			while (Token && strcasecmp(Token, ";") != 0)
			{
				// Copy token over
				for (j = 0; j < strlen(Token) + 1; j++)
					Buf[i++] = Token[j];
				
				// Next token
				Token = TLS_TokenData(Data, Len, &TokData);
			}
			
			// Send buf over
			TLD_FillStatement(Buf, Buf2);
		}
		
#if 0
		/* Check for script */
		else if (strcasecmp(Token, "script") == 0)
		{
			// Check variable
			if (HubVar || NextVar)
			{
				if (l_ScriptDebug)
					CONS_Printf("TLSD: Attempted to use script as a variable.\n");
				break;
			}
			
			// Get next token
			Token = TLS_TokenData(Data, Len, &TokData);
			
			// Expect a standard number
			if (!Token || strchr(Token, '.'))
			{
				if (l_ScriptDebug)
					CONS_Printf("TLSD: Expected positive integer constant after script (got \"%s\").\n", Token);
				break;
			}
			
			// Push script block
			PushBlockType = BT_SCRIPT;
		}
		
		/* Increase deepness */
		else if (strcasecmp(Token, "{") == 0)
		{
			// Check variable
			if (HubVar || NextVar)
			{
				if (l_ScriptDebug)
					CONS_Printf("TLSD: Attempted to brace as a variable.\n");
				break;
			}
			
			// Increment
			Deepness++;
			
			// Print
			if (l_ScriptDebug)
				CONS_Printf("TLSD: Now %i deep.\n", Deepness);
			
			// Reset block type
			PushBlockType = 0;
		}
		
		/* Decrease deepness */
		else if (strcasecmp(Token, "}") == 0)
		{
			// Check hub
			if (HubVar || NextVar)
			{
				if (l_ScriptDebug)
					CONS_Printf("TLSD: Attempted to brace as a variable.\n");
				break;
			}
			
			// Decrement
			Deepness--;
			
			// Print
			if (l_ScriptDebug)
				CONS_Printf("TLSD: Now %i deep.\n", Deepness);
			
			// Check
			if (Deepness < 0)
			{
				if (l_ScriptDebug)
					CONS_Printf("TLSD: Too many closing braces.\n");
				break;
			}
		}
		
		/* Hub variable */
		else if (strcasecmp(Token, "hub") == 0)
		{
			// Check hub
			if (HubVar)
			{
				if (l_ScriptDebug)
					CONS_Printf("TLSD: hub hub is too hubby for ReMooD.\n");
				break;
			}
			
			// Check var
			if (NextVar)
			{
				if (l_ScriptDebug)
					CONS_Printf("TLSD: hub is a reserved word.\n");
				break;
			}
			
			// Now set
			HubVar = true;
			
			if (l_ScriptDebug)
				CONS_Printf("TLSD: Next variable is a hub.\n");
		}
		
		/* Declare variable */
		else if (strcasecmp(Token, "int") == 0 || strcasecmp(Token, "fixed") == 0 ||
			strcasecmp(Token, "string") == 0 || strcasecmp(Token, "mobj") == 0 ||
			strcasecmp(Token, "const") == 0 || strcasecmp(Token, "float") == 0)
		{
			// Check var
			if (NextVar)
			{
				if (l_ScriptDebug)
					CONS_Printf("TLSD: %s is a reserved word.\n", Token);
				break;
			}
			
			// Unset hub
			BoostHubVar = HubVar;
			HubVar = false;
			
			// Which variable kind?
			if (strcasecmp(Token, "int") == 0)
				NextVar = TLSVT_INT;
			else if (strcasecmp(Token, "const") == 0)
				NextVar = TLSVT_CONST;
			else if (strcasecmp(Token, "mobj") == 0)
				NextVar = TLSVT_MOBJ;
			else if (strcasecmp(Token, "string") == 0)
				NextVar = TLSVT_STRING;
			else if (strcasecmp(Token, "fixed") == 0 || strcasecmp(Token, "float") == 0)
				NextVar = TLSVT_FIXED;
			
			if (l_ScriptDebug)
				CONS_Printf("TLSD: Next statement declares a variable (%i).\n", NextVar);
		}
		
		/* Loops and branches */
		else if (strcasecmp(Token, "if") == 0 || strcasecmp(Token, "elseif") == 0 ||
			strcasecmp(Token, "else") == 0 || strcasecmp(Token, "while") == 0 ||
			strcasecmp(Token, "for") == 0)
		{
		}
		
		/* Standard statement */
		else
		{
			// Check hub
			if (HubVar)
			{
				if (l_ScriptDebug)
					CONS_Printf("TLSD: Attempted to hub a non variable.\n");
				break;
			}
			
			// Wait until ;
			while (Token && strcasecmp(Token, ";") != 0)
			{
				CONS_Printf("# \'%s\'\n", Token);
				Token = TLS_TokenData(Data, Len, &TokData);
			}
			CONS_Printf(";\n");
			
			// Lose variable stuff
			BoostHubVar = false;
			NextVar = false;
		}
#endif
		
		/* Ok */
		Ok = true;
	}
	
	if (l_ScriptDebug)
		CONS_Printf("\n");
	
	/* Someone forget braces or whatever? */
	if (RecCount == 0 && Deepness > 0)
	{
		Ok = false;
		
		if (l_ScriptDebug)
			CONS_Printf("TLSD: Script contains unclosed brace.\n");
	}
	
	/* Free token data */
	if (TokData)
		Z_Free(TokData);
	
	/* Unrecursive */
	RecCount--;

	return Ok;
#undef BUFSIZE
}

/* TLS_CompileLump() -- Compile lump script */
boolean TLS_CompileLump(const WadIndex_t Index)
{
	/* Debug? */
	l_ScriptDebug = !!M_CheckParm("-tlsdebug");
	
	if (l_ScriptDebug)
		CONS_Printf("TLS_CompileLump: Debugging %i.\n", Index);
	
	return TLS_IncrementalCompile(Index);
}

