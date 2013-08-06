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
// Legacy Script compiler

/***************
*** INCLUDES ***
***************/

//#include "t_comp.h"
//#include "console.h"

/****************
*** CONSTANTS ***
****************/

/* TLD_BlockType_t -- Type of block */
typedef enum TLS_BlockType_e
{
	BT_STANDARD,				// Global or local
	BT_SCRIPT,					// script ## { }
	BT_IF,						// if() { }
	BT_ELSEIF,					// elseif() { }
	BT_ELSE,					// else() { }
	BT_WHILE,					// while() { }
	BT_FOR,						// for() { }
} TLS_BlockType_t;

/*************
*** LOCALS ***
*************/

bool_t l_DoCompile = false;
bool_t l_ScriptDebug = false;	// Debug scripts

/****************
*** FUNCTIONS ***
****************/

static int Deepness;			// Deepness brace wise
static TLS_BlockType_t PushBlockType;	// Block type to push next
static bool_t HubVar;			// Make hub variable?
static TLS_VariableType_t NextVar;	// next variable type
static bool_t Skip = false;		// Skip token
static char Blocks[512];
static char LastBlock[2] = { 'A', 'A' };

static int Deeps[64];

/* TLS_ClearScripts() -- Clear old scripts */
bool_t TLS_ClearScripts(void)
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
	bool_t Break;
	bool_t Escaped;
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
	Escaped = true;				// skip first "
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
			if (**Seek == ' ' ||** Seek == '\t' ||** Seek == '\r' ||** Seek == '\n')
			{
				(*Seek)++;
				continue;
			}
			// Alpha-numeric identifier ('[A-Za-z][A-Za-z0-9_]*')
			if ((**Seek >= 'a' &&** Seek <= 'z') || (**Seek >= 'A' &&** Seek <= 'Z'))
				Mode = 1;
				
			// String ('".*[^\\]"')
			else if (**Seek == '\"')
				Mode = 2;
				
			// Number
			else if (**Seek >= '0' &&** Seek <= '9')
				Mode = 3;
				
			// Double type
			else if (**Seek == '=' ||** Seek == '<' ||** Seek == '>' ||** Seek == '-' ||** Seek == '+' || *Seek == '!' || *Seek == '&' || *Seek == '|')
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
					if ((**Seek >= 'a' &&** Seek <= 'z') || (**Seek >= 'A' &&** Seek <= 'Z') || (**Seek >= '0' &&** Seek <= '9') ||** Seek == '_')
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
					if (!Escaped &&** Seek == '\"')
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
					if ((**Seek >= '0' &&** Seek <= '9') ||** Seek == '.')
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
bool_t TLS_ValidIdent(char* Ident)
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
	        strcasecmp(Ident, "mobj") == 0 || strcasecmp(Ident, "script") == 0 || strcasecmp(Ident, "string") == 0 || strcasecmp(Ident, "while") == 0)
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
bool_t TLS_ValidUInt(char* Ident)
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
		   } */
		LastBlock[0] = 'A';
		LastBlock[1] = 'A';
		
		Deeps[Deepness] = Deep;
		
		CONL_PrintF("%%%% PUSHBLOCK \"%s\"\n", Blocks);
		
		Skip = false;
		return 0;
	}
	
	else if (Dir < 0)
	{
		Deepness--;
		
		if (Deepness < 0)
			return 0;
			
		LastBlock[1] = Blocks[strlen(Blocks) - 1] + 1;
		LastBlock[0] = Blocks[strlen(Blocks) - 2];
		Blocks[strlen(Blocks) - 1] = 0;
		Blocks[strlen(Blocks) - 1] = 0;
		
		if (LastBlock[1] > 'Z')
		{
			LastBlock[1] = 'A';
			LastBlock[0]++;
		}
		
		CONL_PrintF("%%%% POPBLOCK\n");
		
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
		CONL_PrintF("> %i: %s\n", i, p);
}

/* TLS_IncrementalCompile() -- Compile recursive */
bool_t TLS_IncrementalCompile(const WadIndex_t Index)
{
	return false;
}

/* TLS_CompileLump() -- Compile lump script */
bool_t TLS_CompileLump(const WadIndex_t Index)
{
	return false;
}
