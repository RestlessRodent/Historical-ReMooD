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
// DESCRIPTION: Legacy Script

#include "rmd_main.h"
#include "doomdef.h"
#include "doomstat.h"
#include "g_game.h"
#include "z_zone.h"

#if 0

/* ########################################################################## */
/* ######################### Legacy Script Compilers ######################## */
/* ########################################################################## */
// Compared to the #if 0 code below this, this is smaller, although problems may
// occur, it saves time and does not reproduce code, which is one of the main
// causes for bugs. In short, stages 1 and 2 are merged. The difference is that
// Copy is no longer needed since it's being tokenized on the go, this means
// that if the src lump is for example 128KB, a copy would cost 128KB and
// especially with the tokenizers going on too, may cost alot more memory!

uint32_t LegacyCheck = 0;

/* RMD_LSParseAndTokenize() -- Parses and tokenizes, all in one */
int RMD_LSParseAndTokenize(WadIndex_t Lump, uint8_t Recursive)
{
	char* Src = NULL;
	size_t Size = 0;
	char* x;
	char* a;
	char* b;
	uint8_t ReachedScriptZone = 0;
	uint8_t CopyMode = 0;
	uint8_t LowerCase = 0;
	char InQuotes = 0;
	size_t i;
	WadIndex_t IncludeLump = INVALIDLUMP;
	char IncludeName[9];
	
	/* Don't recourse after 4 */
	if (Recursive >= 4)
		return 0;
	
	/* Allocate and copy completely */
	Src = W_CacheLumpNum(Lump, PU_STATIC);
	
	// If recursive is 0 then fail, else we ignore the bad include
	if (!Src && !Recursive)
	{
		CONS_Printf("Compile Error: Legacy Script -- Lump does not exist\n");
		return 0;
	}
	else if (!Src && Recursive)
	{
		CONS_Printf("Compiler Warning: Legacy Script -- Include is invalid (Does not exist)\n");
		return 1;
	}
	
	// Check Size
	Size = W_LumpLength(Lump) + 1;
	
	// If recursive is 0 then fail, else we ignore the bad include
	if (Size < 2 && !Recursive)
	{
		CONS_Printf("Compile Error: Legacy Script -- Lump is 0 bytes\n");
		return 0;
	}
	else if (Size < 2 && Recursive)
	{
		CONS_Printf("Compiler Warning: Legacy Script -- Include is invalid (Size is 0 bytes)\n");
		return 1;
	}
	
	/* Now to start parsing and tokenizing at the same time */
	x = Src;
	a = NULL;
	b = NULL;
	
#define XPARSECHECK (*x && x < (Src + Size))
	
	CONS_Printf("Compiler: Compiling Script...\n");
	
	if (devparm)
		CONS_Printf("Compiler: XPARSECHECK is %i\n", XPARSECHECK);
		
	while (XPARSECHECK)
	{
		/* First Script starts a [script], includes don't have such blocks */
		if (!Recursive && !ReachedScriptZone)
		{
			while (XPARSECHECK && strncasecmp(x, "[scripts]", 9))
				x++;
				
			if (strncasecmp(x, "[scripts]", 9) == 0)
			{	
				x += 10;
				ReachedScriptZone = 1;
				continue;
			}
			else
			{
				CONS_Printf("Compile Error: Legacy Script -- There is no [scripts] block.\n");
				break;
			}
		}
		
		/* There are no '[' or ']' in Legacy Scripts, so we stop preprocessing */
		if (*x == '[' || *x == ']')
			break;
			
		/* Ignore Single line comments */
		else if (*x == '/' && (*(x+1) && (x+1) < (Src + Size)) && *(x+1) == '/')
			while (XPARSECHECK && *x != '\r' && *x != '\n')
				x++;
			
		/* Copy quotes exactly */
		// A lot more condensed too!
		else if (*x == '\"')
		{
			InQuotes = *x;
			
			a = x++;	// and they said to use ++x... ;)
			
			// Any \t \r or \n will be changed into spaces in the tokenizer step!
			while (XPARSECHECK && (*x != InQuotes || (*x == InQuotes && *(x-1) == '\\')))
				x++;
			
			// without this, it would be: "asuydgauyshfnasf
			if (XPARSECHECK)
				b = ++x;
			else
				break;
				
			CopyMode = 1;
		}
				
		/* Skip newlines, spaces and tabs */
		// I had a case where anything after spaces would be considered as a
		// statement:
		//   print("pow ( 2, 0.5 ) /2      : ", pow(2,0.5)/2,       "\n");
		// After the double space and stuff check, it landed (which I JUST
		// commented) at the " in "\n" and it got confused and since there was
		// no possible match, it just skipped it! And then later on (which is
		// now I just removed :)! )
		else if (*x == '\r' || *x == '\n' || *x == '\t' || *x == ' ')
			x++;
		
		/* Everything else is the fun part */
		// This is a merger between the preprocessor and the tokenizer, and MUST
		// separate the parts such as script 1{stuff("");}
		// to : "script" "1" "{" "stuff" "(" "\"\"" ")" ";" "}"
		// not: "script" "1{stuff("");}"
		else
		{		
			// Anything starting with a letter or underscore is an identifier!
			if ((*x >= 'A' && *x <= 'Z') || (*x >= 'a' && *x <= 'z') || (*x == '_'))
			{
				CopyMode = 1;
				a = x++;
				b = x;
				
				while (XPARSECHECK && ((*x >= 'A' && *x <= 'Z') || (*x >= 'a' && *x <= 'z') ||
					(*x >= '0' && *x <= '9') || *x == '_'))
				{
					b++;
					x++;
				}
				
				LowerCase = 0;
			}
			
			// Anything starting with a number is a number or floating point number
			// NOTE: Minus signs will be treated by the compiler specially, in the
			// end they still will be negative but in this case it is treated as
			// an operator!
			else if (*x >= '0' && *x <= '9')
			{
				CopyMode = 1;
				a = x++;
				b = x;
				
				while (XPARSECHECK && ((*x >= '0' && *x <= '9') || *x == '.'))
				{
					b++;
					x++;
				}
			}
			
			// Everything else (usually operators and such)
			else
			{
				if (
					((*x == '=' || *x == '|' || *x == '&' || *x == '+' || *x == '-') && *(x+1) == *x) ||
					((*x == '>' || *x == '<' || *x == '!' || *x == '=') && *(x+1) == '=')
					)
				{
					a = x;
					b = ++x;
					
					CopyMode = 2;
				}
				else
				{
					a = x;
					b = NULL;
					
					CopyMode = 2;
				}
				
				x++;
			}
		}
		
		// ###############
		// CODE DIRECTLY FROM THE TOKENIZER
		// ###############
		
		/* Ran out of room in one chunk */
		if (CurrentTokenChunkPos >= TOKENCHUNKSIZE - 24)
		{
			CurrentTokenChunk++;
			CurrentTokenChunkPos = 0;
		}
		
		/* No more token chunks */
		if (CurrentTokenChunk >= MAXTOKENCHUNKS)
			break;
		
		/* Create a new token chunk if needed */
		if (!TokenChunks[CurrentTokenChunk])
		{
			TokenChunks[CurrentTokenChunk] = Z_Malloc(sizeof(char) * TOKENCHUNKSIZE, PU_STATIC, NULL);
			memset(TokenChunks[CurrentTokenChunk], 0, sizeof(char) * TOKENCHUNKSIZE);
		}
		
		/* Resize Tokens if needed */
		if (CurrentToken >= NumTokens)
		{
			TokenTemp = Z_Malloc(sizeof(char*) * (NumTokens + TOKENCOUNT), PU_STATIC, NULL);
			memset(TokenTemp, 0, sizeof(char*) * (NumTokens + TOKENCOUNT));
			memcpy(TokenTemp, TokenBuf, sizeof(char*) * NumTokens);
			Z_Free(TokenBuf);
			TokenBuf = TokenTemp;
			TokenTemp = NULL;
			NumTokens += TOKENCOUNT;
		}
		
		/* Copy */
		if (CopyMode == 1)
		{
			
			TokenBuf[CurrentToken] = &TokenChunks[CurrentTokenChunk][CurrentTokenChunkPos];
			memcpy(TokenBuf[CurrentToken], a, (b - a));
			TokenChunks[CurrentTokenChunk][CurrentTokenChunkPos + (b - a)] = 0;
			CurrentTokenChunkPos += (b - a) + 1;
			
			// Check for include (although checking 'i' and 'n' will waste a
			// cycle or two, how much would a strcasecmp waste if it did not
			// match?
			if (CurrentToken > 1)
				if (TokenBuf[CurrentToken - 2][0] == 'i' &&
					TokenBuf[CurrentToken - 2][1] == 'n' &&
					TokenBuf[CurrentToken][0] == '\"' &&
					strlen(TokenBuf[CurrentToken - 2]) == 7 &&
					strncmp(TokenBuf[CurrentToken - 2], "include", 7) == 0)
				{
					memset(IncludeName, 0, 9);
					memcpy(IncludeName, TokenBuf[CurrentToken] + 1, 8);
					
					for (i = 0; i < 8; i++)
						if (IncludeName[i] == '\"')
							IncludeName[i] = 0;
					
					IncludeLump = W_CheckNumForName(IncludeName);
					
					if (IncludeLump == INVALIDLUMP)
					{
						CONS_Printf("Compile Warning: Legacy Script -- include \"%s\" not found!\n", IncludeName);
						
						for (i = 0; i < 8; i++)
							if (IncludeName[i] == '.')
								IncludeName[i] = '_';
								
						IncludeLump = W_CheckNumForName(IncludeName);
					}
					
					// Don't bother removing the include ( ) ; tokens, the
					// compiler can just ignore it and move on, except the (
					// and ) will be treated as special cases
					// -- err, no! just ignore it!
					TokenBuf[CurrentToken - 2] = NULL;
					TokenBuf[CurrentToken - 1] = NULL;
					CurrentToken -= 2;
					
					if (IncludeLump != INVALIDLUMP)
					{
						RMD_LSParseAndTokenize(IncludeLump, Recursive + 1);
						TokenBuf[CurrentToken] = NULL;
						CurrentToken--;
					}
					else
						CONS_Printf("Compile Warning: Legacy Script -- include \"%s\" not found! (backup method)\n", IncludeName);
				}
			
			CurrentToken++;
			CopyMode = 0;
		}
		else if (CopyMode == 2)
		{
			if (a && !b)
			{
				TokenBuf[CurrentToken] = &TokenChunks[CurrentTokenChunk][CurrentTokenChunkPos];
				TokenBuf[CurrentToken][0] = *a;
				TokenBuf[CurrentToken][1] = 0;
				CurrentTokenChunkPos += 2;
			}
			else
			{
				TokenBuf[CurrentToken] = &TokenChunks[CurrentTokenChunk][CurrentTokenChunkPos];
				TokenBuf[CurrentToken][0] = *a;
				TokenBuf[CurrentToken][1] = *b;
				TokenBuf[CurrentToken][2] = 0;
				CurrentTokenChunkPos += 3;
			}
			
			CurrentToken++;
			CopyMode = 0;
		}
	}
	
	Z_Free(Src);
	
	if (!Recursive && !ReachedScriptZone)
		return 0;
	
	return 1;
}

/* RMD_LSEvaluateExpression() -- LS, evaluates an expression */
// This turns the...
// foo = 4 + ((banana + bar(12)) * 2); into the right byte code
// Returns the result of the resulting expression avaluation

typedef struct EvalEntry_s
{
	rmdtypeid_t Type;
	char VariableID[32];
	uint32_t Variable;
	char* Op;
	char** Tokens;
	size_t NumTokens;
} EvalEntry_t;

uint32_t RMD_LSEvaluateExpression(uint32_t* PosPtr, uint8_t* EndedInErrorPtr, uint16_t* BlockPtr, uint16_t* BlockParentPtr, size_t Len, char** TokenBufPtr)
{
	/* Local */
	// Counters
	size_t i = 0, j = 0, k = 0, l = 0, m = 0, n = 0, o = 0, p = 0;
	static char* OpOrder[] =
	{
		"(", ".", "--", "++", "!", "~", "%", "/", "*", "-", "+", ">=", "<=",
		">", "<", "!=", "==", "&", "|", "&&", "||", "="
	};
	static boolean OpRTL[] =
	{
		false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, true
	};
	static boolean OpBi[] =
	{
		false, false, true, true, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false
	};
	
	// Tokens
	char** tb_Real = NULL;
	char** tb_Fake = NULL;
	char** tb_Backup = NULL;
	char** tb_NewFake = NULL;
	size_t tb_FakeLen = 0;
	
	// Size Determinations
	size_t sz_StartToken = 0, sz_EndToken = 0;
	size_t sz_Len = 0;
	
	// Evaluations
	EvalEntry_t* ev_Eval = NULL;
	EvalEntry_t* ev_NewEval = NULL;
	size_t ev_NumEval = 0;
	boolean ev_Do = false;
	boolean ev_ReDo = false;
	char ev_TempName[32];
	char* ev_TempString = NULL;
	rmdtypeid_t ev_Type = RMDTID_VOID;
	
	// Tests
	uint32_t ts_Result = 0;
	uint32_t ts_Result2 = 0;
	
	//////////////// INITIALIZATION ////////////////
	/* Determine Length */
	// Set Real
	if (TokenBufPtr)	// Using a recursive token buffer or the real one except directly referenced?
		tb_Real = TokenBufPtr;
	else				// Determine from the CurrentToken and such
		tb_Real = TokenBuf;
		
	// If Len is not passed... find ;
	if (Len)
	{
		sz_StartToken = 0;
		sz_EndToken = Len - 1;
		sz_Len = Len;
	}
	else
	{
		if (TokenBufPtr)
			sz_StartToken = 0;
		else
			sz_StartToken = CurrentToken;
		
		sz_EndToken = sz_StartToken;
		
		while (strcmp(tb_Real[sz_EndToken], ";"))
			sz_EndToken++;
		sz_Len = sz_EndToken - sz_StartToken;
	}
	
	// For REAL tokenbufs
	tb_Real = &tb_Real[sz_StartToken];
	
	if (!sz_Len)	// Invalid length?
		return 0;
	
	/* Now create fake tokens */
	tb_Fake = Z_Malloc(sizeof(char*) * sz_Len, PU_STATIC, NULL);
	memset(tb_Fake, 0, sizeof(char*) * sz_Len);
	
	for (i = 0; i < sz_Len; i++)
	{
		tb_Fake[i] = Z_Malloc(sizeof(char) * (strlen(tb_Real[sz_StartToken + i]) + 1), PU_STATIC, NULL);
		memset(tb_Fake[i], 0, sizeof(char) * (strlen(tb_Real[sz_StartToken + i]) + 1));
		strcpy(tb_Fake[i], tb_Real[sz_StartToken + i]);
	}
	
	/* Create our temporary variable */
	snprintf(ev_TempName, sizeof(ev_TempName), "_t_%08x", LegacyCheck++);
	ts_Result = RMD_CreateSymbol(*BlockParentPtr, *BlockPtr, ev_TempName, RMDTID_VOID);
	
	/* Split Tokens into evaluation chains */
	// Create the first chain
	ev_Eval = Z_Malloc(sizeof(EvalEntry_t), PU_STATIC, NULL);
	memset(ev_Eval, 0, sizeof(EvalEntry_t));
	ev_NumEval = 1;
	
	snprintf(ev_Eval[0].VariableID, sizeof(ev_Eval[0].VariableID), "_t_%08x", LegacyCheck++);
	
	ev_Eval[0].Type = RMDTID_VOID;
	ev_Eval[0].Variable = RMD_CreateSymbol(*BlockParentPtr, *BlockPtr, ev_Eval[0].VariableID, RMDTID_VOID);
	ev_Eval[0].Op = NULL;
	ev_Eval[0].Tokens = Z_Malloc(sizeof(char*) * sz_Len, PU_STATIC, NULL);
	ev_Eval[0].NumTokens = sz_Len;
	
	for (i = 0; i < sz_Len; i++)
		ev_Eval[0].Tokens[i] = tb_Fake[i];
		
	/* Translate constants to symbols so our assembly runner can take care of them */
	// `apple = banana + 3` >> `apple = banana + _t_deadbeef`
	for (i = 0; i < ev_Eval[0].NumTokens; i++)
	{
		/*if (strcmp(ev_Eval[0].Tokens[i], "(") == 0)
		{
			k = 1;
			j = i + 1;
			
			while (k > 0)
			{
				if (strcmp(ev_Eval[0].Tokens[j], "(") == 0)
					k++;
				else if (strcmp(ev_Eval[0].Tokens[j], ")") == 0)
					k--;
				
				j++;
			}
			
			i = j - 1;
			continue;
		}
		else*/ if (ev_Eval[0].Tokens[i][0] == '\"')
		{
			snprintf(ev_TempName, sizeof(ev_TempName), "_t_%08x", LegacyCheck++);
			ts_Result2 = RMD_CreateSymbol(*BlockParentPtr, *BlockPtr, ev_TempName, RMDTID_STRING);
			
			ev_TempString = Z_Malloc(sizeof(char) * (strlen(ev_Eval[0].Tokens[i]) - 2), PU_STATIC, NULL);
			strncpy(ev_TempString, ev_Eval[0].Tokens[i] + 1, strlen(ev_Eval[0].Tokens[i]) - 1);
			
			RMD_CreateByte(PosPtr, RMDBID_SETVALUE_STRING, "ts", ts_Result2, ev_Eval[0].Tokens[i]);
			
			Z_Free(ev_TempString);
			ev_TempString = NULL;
			
			Z_Free(ev_Eval[0].Tokens[i]);
			ev_Eval[0].Tokens[i] = Z_Malloc(sizeof(char) * (strlen(Symbols[NumSymbols - 1].ID) + 1), PU_STATIC, NULL);
			memset(ev_Eval[0].Tokens[i], 0, sizeof(char) * (strlen(Symbols[NumSymbols - 1].ID) + 1));
			strcpy(ev_Eval[0].Tokens[i], Symbols[NumSymbols - 1].ID);
		}
		else if (ev_Eval[0].Tokens[i][0] >= '0' && ev_Eval[0].Tokens[i][0] <= '9')
		{
			ev_Type = RMDTID_INTEGER;
			
			// Check for fixed
			for (j = 0; j < strlen(ev_Eval[0].Tokens[i]); j++)
				if (ev_Eval[0].Tokens[i][j] == '.')
				{
					ev_Type = RMDTID_FIXED;
					break;
				}
			
			snprintf(ev_TempName, sizeof(ev_TempName), "_t_%08x", LegacyCheck++);
			ts_Result2 = RMD_CreateSymbol(*BlockParentPtr, *BlockPtr, ev_TempName, ev_Type);
			
			if (ev_Type == RMDTID_INTEGER)
				RMD_CreateByte(PosPtr, RMDBID_SETVALUE_INTEGER, "ti", ts_Result2, atoi(ev_Eval[0].Tokens[i]));
			else
				RMD_CreateByte(PosPtr, RMDBID_SETVALUE_FIXED, "ti", ts_Result2,
					(int32_t)(atof(ev_Eval[0].Tokens[i]) * 65535.0));
			
			Z_Free(ev_Eval[0].Tokens[i]);
			ev_Eval[0].Tokens[i] = Z_Malloc(sizeof(char) * (strlen(Symbols[NumSymbols - 1].ID) + 1), PU_STATIC, NULL);
			memset(ev_Eval[0].Tokens[i], 0, sizeof(char) * (strlen(Symbols[NumSymbols - 1].ID) + 1));
			strcpy(ev_Eval[0].Tokens[i], Symbols[NumSymbols - 1].ID);
		}
	}
	
	for (i = 0; i < ev_NumEval; i++)
		for (j = 0; j < ev_Eval[i].NumTokens; j++)
		{
			
		}
	
	// Split chains
#if 0
	ev_Do = true;
	ev_ReDo = false;
	while (ev_Do)
	{
		// We constantly split chains until they cannot be split any further
		// Example: User puts in `banana = (4 * foo) - bar % 600 + apple++`
		// >> {banana = (4 * foo) - bar % 600 + apple++}
		// >> {(4 * foo) - bar % 600 + apple++, banana = $$1$$}
		// >> {(4 * foo), $$1$$ - bar % 600 + apple++, banana = $$2$$}
		// >> {(4 * foo), bar % 600, $$1$$ - $$2$$ + apple++, banana = $$4$$}
		// >> {(4 * foo), bar % 600, $$1$$ - $$2$$, $$3$$ + apple++, banana = $$4$$}
		// >> {(4 * foo), bar % 600, $$1$$ - $$2$$, $$3$$ + apple, banana = $$4$$, apple = apple + 1}
		//     $$ 1 $$     $$ 2 $$     $$ 3 $$       $$ 4 $$         $$ 5 $$         $$6$$
		// .. Evaluate 1 :: (4 * foo)
		// .. Evaluate 2 :: bar % 600
		// .. Evaluate 3 :: $$1$$ - $$2$$ or (4 * foo) - (bar % 600)
		// .. Evaluate 4 :: $$3$$ + $$4$$ or $$3$$ + apple or (((4 * foo) - (bar % 600)) + (apple))
		// .. Evaluate 5 :: banana = $$5$$ or banana = (((4 * foo) - (bar % 600)) + (apple))
		// .. Evaluate 6 :: apple = apple + 1
		
		ev_ReDo = false;
		
		ev_Do = false;
#if 0
		for (i = 0; i < ev_NumEval; i++)
		{
			for (j = 0; j < ev_Eval[i].NumTokens; j++)
			{
				// Check Function
				if ((ev_Eval[i].Tokens[j][0] >= 'a' && ev_Eval[i].Tokens[j][0] <= 'z') || (ev_Eval[i].Tokens[j][0] >= 'A' && ev_Eval[i].Tokens[j][0] <= 'Z') || ev_Eval[i].Tokens[j][0] == '_')
				{
				}
				
				// Check Parenthesis
				else if (strcmp(ev_Eval[i].Tokens[j], "(") == 0)
				{
					m = j + 1;
					n = 1;
					
					while (n > 0)
					{
						if (strcmp(ev_Eval[i].Tokens[m], "(") == 0)
							n++;
						else if (strcmp(ev_Eval[i].Tokens[m], ")") == 0)
							n--;
						
						if (n > 0)
							m++;
						else
							break;
					}
					
					/* Split Here */
					// Create New One
					ev_NewEval = Z_Malloc(sizeof(EvalEntry_t) * (ev_NumEval + 1), PU_STATIC, NULL);
					memset(ev_NewEval, 0, sizeof(EvalEntry_t) * (ev_NumEval + 1));
					
					// Copy old until i
					for (l = 0; l < i; l++)
						ev_NewEval[l] = ev_Eval[i];
						
					// Create parenthetical statement before
					snprintf(ev_TempName, sizeof(ev_TempName), "_t_%08x", LegacyCheck++);
					
					ev_NewEval[l].Type = RMDTID_VOID;
					ev_NewEval[l].Variable = RMD_CreateSymbol(*BlockParentPtr, *BlockPtr, ev_TempName, RMDTID_VOID, RMD_CORRECTSIZE(RMDTID_VOID), NULL);
					ev_NewEval[l].Tokens = Z_Malloc(sizeof(char*) * (m - j), PU_STATIC, NULL);
					ev_NewEval[l].Op = "(";
					ev_NewEval[l].NumTokens = m - j;
					
					// Second
					snprintf(ev_TempName, sizeof(ev_TempName), "_t_%08x", LegacyCheck++);
					
					ev_NewEval[l + 1].Type = RMDTID_VOID;
					ev_NewEval[l + 1].Variable = RMD_CreateSymbol(*BlockParentPtr, *BlockPtr, ev_TempName, RMDTID_VOID, RMD_CORRECTSIZE(RMDTID_VOID), NULL);
					ev_NewEval[l + 1].Tokens = Z_Malloc(sizeof(char*) * (ev_Eval[i].NumTokens - (m - j) + 1), PU_STATIC, NULL);
					ev_NewEval[l + 1].Op = NULL;
					ev_NewEval[l + 1].NumTokens = (ev_Eval[i].NumTokens - (m - j) + 1);
					
					// Place parenthesis
					for (n = 0; n < (m - j); n++)
						ev_NewEval[l].Tokens[n] = ev_Eval[i].Tokens[j + n];
					
					// Replace
					ev_NewEval[l + 1].Tokens[0]	= RMD_GetSymbolByID(*BlockPtr, ev_NewEval[l].Variable)->ID;
					
					// Copy After
					for (o = 1; n < ev_Eval[i].NumTokens; o++, n++)
						ev_NewEval[l + 1].Tokens[o] = ev_Eval[i].Tokens[j + n];
						
					Z_Free(ev_Eval);
					ev_Eval = ev_NewEval;
					ev_NewEval = 0;
						
					ev_NumEval++;
					
					ev_ReDo = true;
				}
				
				// Check Other orders
				else
				{
				}
				
				if (ev_ReDo)
					break;
			}
		
			if (ev_ReDo)
				break;
		}
	
		if (!ev_ReDo && i == ev_NumEval)
			ev_Do = false;
#endif
	}
#endif
	
	//////////////// EVALUATION ////////////////
#ifdef _DEBUG
	if (devparm)
	{
		CONS_Printf("RMD_LSEvaluateExpression: Pre-Evaluations Start\n");
		
		for (i = 0; i < ev_NumEval; i++)
		{
			CONS_Printf("Eval #%i: ", i);
			CONS_Printf("Type = %i; ", ev_Eval[i].Type);
			CONS_Printf("Variable = 0x%08x; ", ev_Eval[i].Variable);
			CONS_Printf("Op = %s; ", (ev_Eval[i].Op ? ev_Eval[i].Op : "NULL"));
			CONS_Printf("NumTokens = %i\n", ev_Eval[i].NumTokens);
			
			CONS_Printf("<< ");
			for (j = 0; j < ev_Eval[i].NumTokens; j++)
				CONS_Printf("\"%s\" ", ev_Eval[i].Tokens[j]);
			CONS_Printf(">>\n");
		}
		
		CONS_Printf("RMD_LSEvaluateExpression: Pre-Evaluations Finish\n");
	}
#endif
	
	for (i = 0; i < ev_NumEval; i++)
	{
	}
	
	//////////////// UN-INITIALIZATION ////////////////
	/* Clean Evaluations */
	if (ev_Eval)
	{
		Z_Free(ev_Eval);
		ev_Eval = NULL;
	}
	
	/* Clean Token Copy */
	if (tb_Fake)
	{
		for (i = 0; i < sz_Len; i++)
			if (tb_Fake[i])
			{
				Z_Free(tb_Fake[i]);
				tb_Fake[i] = NULL;
			}
		Z_Free(tb_Fake);
		tb_Fake = NULL;
	}
	
	return ts_Result;
}

/* RMD_CompileLegacyScript() -- Legacy Script compatibility layer */
int RMD_CompileLegacyScript(WadIndex_t Lump)
{
	size_t NumParenthesis = 0;
	uint8_t Area = 0;
	int32_t Note = 0;
	int32_t Note2 = 0;
	uint8_t EndedInError = 0;
	int ReturnVal = 1;
	char* t_charstar;
	uint32_t pos = 0;
	uint32_t origvar;
	size_t i, j, k;
	uint8_t ParserClass = 0;
	uint16_t Block = 0;
	uint16_t LastBlock = 0xFFFF;
	uint32_t Var_Result = 0;
	uint32_t Var_Sym = 0;
	
	uint32_t Var_CP, Var_DP, Var_Zoom, Var_Trigger;
	
	RMD_CreateTokens();
	
	// Start parsing!
	if (!RMD_LSParseAndTokenize(Lump, 0))
		return 0;
		
	/*** STAGE 3: COMPILE ***/
	RMD_ResetCompiler();
	
#define LSTOKENCHECK(x) (CurrentToken <= NumTokens && TokenBuf[CurrentToken] && strcmp(TokenBuf[CurrentToken], (x)) == 0)
	
	// Every Legacy Script has these global variables
	Var_CP = RMD_CreateSymbol(LastBlock, Block, "consoleplayer", RMDTID_INTEGER);
	Var_DP = RMD_CreateSymbol(LastBlock, Block, "displayplayer", RMDTID_INTEGER);
	Var_Zoom = RMD_CreateSymbol(LastBlock, Block, "zoom", RMDTID_INTEGER);
	Var_Trigger = RMD_CreateSymbol(LastBlock, Block, "trigger", RMDTID_INTEGER);
	
	for (CurrentToken = 0; CurrentToken <= NumTokens; CurrentToken++)
	{
		/* Skip Null Tokens */
		if (!TokenBuf[CurrentToken])
			continue;
			
		/* Extra Semi-Colon */
		else if (LSTOKENCHECK(";"))
			continue;
		
		/* Script or Function */
		else if (LSTOKENCHECK("script") || LSTOKENCHECK("function"))
		{
			if (Block)
			{
				CONS_Printf("Compile Error: Legacy Script -- Function block inside function block.\n", TokenBuf[CurrentToken]);
				EndedInError = 1;
				break;
			}
			else
			{
				/* Script */
				if (LSTOKENCHECK("script"))
				{
				}
				
				/* Function */
				else if (LSTOKENCHECK("function"))
				{
				}
			}
		}
		
		/* Variable */
		else if (LSTOKENCHECK("const") || LSTOKENCHECK("hub") || LSTOKENCHECK("int") ||
			LSTOKENCHECK("fixed") || LSTOKENCHECK("string") || LSTOKENCHECK("mobj"))
		{
			rmdtypeid_t VarType = RMDTID_VOID;
			char* VarName;
			
			/* Determine Type to Use */
			if (LSTOKENCHECK("const"))
				VarType = RMDTID_INTEGER | RMDTID_CONST;
			else if (LSTOKENCHECK("hub"))
				VarType = RMDTID_INTEGER | RMDTID_HUB;
			else if (LSTOKENCHECK("int"))
				VarType = RMDTID_INTEGER;
			else if (LSTOKENCHECK("fixed"))
				VarType = RMDTID_FIXED;
			else if (LSTOKENCHECK("string"))
				VarType = RMDTID_STRING;
			else if (LSTOKENCHECK("mobj"))
				VarType = RMDTID_MOBJ;
			else
			{
				CONS_Printf("Compile Error: Legacy Script -- Unknown type \"%s\".\n", TokenBuf[CurrentToken]);
				EndedInError = 1;
				break;
			}
			
			/* Get the name */
			CurrentToken++;
			while (!TokenBuf[CurrentToken])
				CurrentToken++;
			VarName = TokenBuf[CurrentToken];
			
			// Check Validity
			if (!((*VarName >= 'A' && *VarName <= 'Z') || (*VarName >= 'a' && *VarName <= 'z') || *VarName == '_'))
			{
				CONS_Printf("Compile Error: Legacy Script -- Invalid name for an identifier: \"%s\".\n", TokenBuf[CurrentToken]);
				EndedInError = 1;
				break;
			}
			
			/* Check to see if it's being set to something */
			CurrentToken++;
			while (!TokenBuf[CurrentToken])
				CurrentToken++;
				
			if (LSTOKENCHECK(";"))
			{
				// Add the symbol with an initial value of Zero
				RMD_CreateSymbol(LastBlock, Block, VarName, VarType);
			}
			else if (LSTOKENCHECK("="))
			{
				// Boost token
				CurrentToken++;
				
				// Create the symbol in memory
				Var_Sym = RMD_CreateSymbol(LastBlock, Block, VarName, VarType);
				
				/* Test Short Circuit */
				if (TokenBuf[CurrentToken + 1][0] == ';')
				{
					// INTEGER
					if (TokenBuf[CurrentToken][0] >= '0' && TokenBuf[CurrentToken][0] <= '9')
					{
						k = 0;
						
						for (j = 0; j < strlen(TokenBuf[CurrentToken]); j++)
							if (TokenBuf[CurrentToken][j] == '.')
							{
								k = 1;
								break;
							}
							
						if (k)
							RMD_CreateByte(&pos, RMDBID_SETVALUE_FIXED, "ti", Var_Sym,
								(int32_t)(atof(TokenBuf[CurrentToken]) * 65535.0));
						else
							RMD_CreateByte(&pos, RMDBID_SETVALUE_INTEGER, "ti", Var_Sym,
								atoi(TokenBuf[CurrentToken]));
					}
					
					// STRING
					else if (TokenBuf[CurrentToken][0] == '\"')
					{
					}
					
					// VARIABLE
					else
					{
						Var_Result = 0xFFFFFFFF;
						
						for (j = 0; j < NumSymbols; j++)
							if (strcmp(Symbols[j].ID, TokenBuf[CurrentToken]) == 0)
							{
								Var_Result = Symbols[j].PhysicalLocation;
								break;
							}
						
						if (Var_Result == 0xFFFFFFFF)
						{
							CONS_Printf("Compile Error: Legacy Script -- Undefined reference to `%s`\n", TokenBuf[CurrentToken]);
							EndedInError = 1;
							break;
						}
						
						RMD_CreateByte(&pos, RMDBID_CAST, "tt", Var_Sym, Var_Result);
					}
				}
				else
				{	
					// Evaluate
					Var_Result = RMD_LSEvaluateExpression(&pos, &EndedInError, &Block, &LastBlock, 0, &TokenBuf[CurrentToken - 2]);
				
					if (EndedInError || !Var_Result)	// Copy resulting value into symbol
					{
						CONS_Printf("Compile Error: Legacy Script -- Bad expression\n", TokenBuf[CurrentToken]);
						EndedInError = 1;
						break;
					}
				
					if (Var_Result != 0xFFFFFFFF)
						RMD_CreateByte(&pos, RMDBID_COPY, "uu", Var_Sym, Var_Result);
				}
				
				while (!LSTOKENCHECK(";"))
					CurrentToken++;
			}
			else
			{
				CONS_Printf("Compile Error: Legacy Script -- Unknown symbol \"%s\" following declaration.\n", TokenBuf[CurrentToken]);
				EndedInError = 1;
				break;
			}
		}
		
		/* Something inside of function block */
		else if (Block)
		{
			/* Conditional -- if*/
			if (LSTOKENCHECK("if"))
			{
			}
			
			/* Conditional -- elseif */
			else if (LSTOKENCHECK("elseif"))
			{
			}
			
			/* Conditional -- else */
			else if (LSTOKENCHECK("else"))
			{
			}
			
			/* Loop -- while */
			else if (LSTOKENCHECK("while"))
			{
			}
			
			/* Loop -- for */
			else if (LSTOKENCHECK("for"))
			{
			}
			
			/* Ending Brace */
			else if (LSTOKENCHECK("}"))
			{
			}
			
			/* Normal Expression? */
			else
			{
			}
		}
		
		/* Something that doesn't belong */
		else
		{
			if (LSTOKENCHECK("startscript"))
			{
			}
			else
			{
				CONS_Printf("Compile Error: Legacy Script -- \"%s\" invalid outside function block.\n", TokenBuf[CurrentToken]);
				EndedInError = 1;
				break;
			}
		}
	}
	
	if (EndedInError)
	{
		// Compilation Failed so we throw everything away and return 0
		//RMD_ClearCompiler();
	
		CONS_Printf("Compile Error: Legacy Script -- Stopping compilation.\n");
	
		ReturnVal = 0;
	}

	/*** STAGE 4: POST-COMPILATION ***/
	// Remove Tokens
	RMD_RemoveTokens();
	
	return ReturnVal;
}

#endif

