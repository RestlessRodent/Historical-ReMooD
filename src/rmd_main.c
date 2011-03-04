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
// DESCRIPTION: ReMooD Script

#include "rmd_main.h"
#include "doomdef.h"
#include "doomstat.h"
#include "g_game.h"
#include "z_zone.h"

/* Variable */

/* Instructions */
RMD_Instruction_t AssemblyInstructions[] =
{
	//xxxxxxxxxxxxxxxxxxxx	nv	v							func
	{"NULL",				0,	NULL, 						NULL},		// Does nothing, very useful
	{"REGISTERVAR",			0,	NULL,						NULL},		// Adds a variable
	{"UNREGISTERVAR",		0,	NULL,						NULL},		// Removes a variable
};


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


int RMD_ClearOldLevelCode(void)
{
	return 0;
}

void Command_DumpAssembly_f(void)
{
}

int RMD_CompileLegacyScript(WadIndex_t Lump)
{
	return 0;
}

#if 0

#define RCL RMDTID_LINKED

rmdbyte_t* ByteCode = NULL;
size_t ExecPos[MAXDEEPCALLS];
size_t ByteCodeZone = 0;
size_t CurrentExec = 0;

uint16_t* GroupParent = NULL;
size_t NumGroups = 0;
RMD_Symbol_t* Symbols = NULL;
size_t NumSymbols = 0;
uint8_t* UserZone = NULL;
size_t UserZoneSize = NULL;

size_t TypeSizes[MAXRMDTYPEID] =
{
	0,
	4,
	8,
	4,
	1,
	1,
	RMD_VARIABLESIZE,
	4,
	sizeof(size_t)
};

/* ########################################################################## */
/* ############################# Global Scripting ########################### */
/* ########################################################################## */

char** TokenBuf = NULL;
char** TokenTemp = NULL;
char** TokenChunks = NULL;
size_t NumTokens = 0;
size_t NumTokenChunks = 0;
size_t CurrentToken = 0;
size_t CurrentTokenChunk = 0;
size_t CurrentTokenChunkPos = 0;

int RMD_CreateTokens(void)
{
	// Allocate Space for Tokens
	TokenBuf = Z_Malloc(sizeof(char*) * TOKENCOUNT, PU_STATIC, NULL);
	memset(TokenBuf, 0, sizeof(char*) * TOKENCOUNT);
	NumTokens = TOKENCOUNT;
	CurrentToken = 0;
	
	// Create Token Chunks
	TokenChunks = Z_Malloc(sizeof(char*) * MAXTOKENCHUNKS, PU_STATIC, NULL);
	memset(TokenChunks, 0, sizeof(char*) * MAXTOKENCHUNKS);
	NumTokenChunks = MAXTOKENCHUNKS;
	CurrentTokenChunk = 0;
	CurrentTokenChunkPos = 0;
	
	return 1;
}

int RMD_RemoveTokens(void)
{
	if (TokenBuf)
	{
		Z_Free(TokenBuf);
		TokenBuf = NULL;
		NumTokens = 0;
	}
	
	if (TokenChunks)
	{
		for (CurrentTokenChunk = 0; CurrentTokenChunk < NumTokenChunks; CurrentTokenChunk++)
		{
			if (TokenChunks[CurrentTokenChunk])
			{
				Z_Free(TokenChunks[CurrentTokenChunk]);
				TokenChunks[CurrentTokenChunk] = NULL;
			}
		}
		
		Z_Free(TokenChunks);
		TokenChunks = NULL;
		CurrentTokenChunk = 0;
		NumTokenChunks = 0;
		CurrentTokenChunkPos = 0;
	}
	
	return 1;
}

/* RMD_ClearOldLevelCode() -- Clears anything that may be lingering from old levels */
int RMD_ClearOldLevelCode(void)
{
	RMD_RemoveTokens();
	RMD_ClearCompiler();
	
	if (Symbols)
	{
		Z_Free(Symbols);
		Symbols = NULL;
		NumSymbols = 0;
	}
}

int RMD_ClearCompiler(void)
{
	/* Remove old byte code and variables */
	if (ByteCode)
	{
		Z_Free(ByteCode);
		ByteCode = NULL;
		ByteCodeZone = 0;
	}
	
	if (UserZone)
	{
		Z_Free(UserZone);
		UserZone = 0;
		UserZoneSize = 0;
	}
}

int RMD_ResetCompiler(void)
{
	RMD_ClearCompiler();
	
	/* Create 100 initial instructions */
	ByteCode = Z_Malloc(sizeof(rmdbyte_t) * 100, PU_STATIC, NULL);
	memset(ByteCode, 0, sizeof(rmdbyte_t) * 100);
	ByteCodeZone = sizeof(rmdbyte_t) * 100;
	
	// the first bytecode will jump to code that will call the start functions
	// if any
	ExecPos[0] = sizeof(rmdbyte_t);
	CurrentExec = 0;
	
	return 1;
}

char* ValidCodes[MAXRMDBYTEID] =
{
	"",			// RMDBID_NULL

	/* Flow Control */
	"u",		// RMDBID_TRAP
	"u",		// RMDBID_CALL
	"u",		// RMDBID_JUMP
	"",			// RMDBID_RETURN
	"u",		// RMDBID_PAUSE
	"u",		// RMDBID_WAIT
	
	/* Return Value */
	"t",		// RMDBID_SETRETURNVALUE
	"t",		// RMDBID_GETRETURNVALUE
	
	/* Memory */
	"uu",		// RMDBID_COPY
	"uu",		// RMDBID_SWAP
	
	/* Getting and Setting */
	"t",		// RMDBID_SETVALUE_VOID
	"ti",		// RMDBID_SETVALUE_INTEGER
	"ti",		// RMDBID_SETVALUE_FIXED
	"ti",		// RMDBID_SETVALUE_MOBJ
	"ts",		// RMDBID_SETVALUE_STRING
	
	/* Casting */
	"tt",		// RMDBID_CAST

	/* Arithmetic */
	"tt",		// RMDBID_ADD
	"tt",		// RMDBID_SUBTRACT
	"tt",		// RMDBID_MULTIPLY
	"tt",		// RMDBID_DIVIDE
	"tt",		// RMDBID_MODULUS
	"tt",		// RMDBID_BINARYAND
	"tt",		// RMDBID_BINARYOR
	"tt",		// RMDBID_BINARYXOR
	"tt",		// RMDBID_BINARYNOT
	"tt",		// RMDBID_BINARYXNOR
	"tt",		// RMDBID_BINARYSHIFTLEFT
	"tt",		// RMDBID_BINARYSHIFTRIGHT
};

char* NameCodes[MAXRMDBYTEID] =
{
	"NULL",			// RMDBID_NULL

	/* Flow Control */
	"TRAP",		// RMDBID_TRAP
	"CALL",		// RMDBID_CALL
	"JUMP",		// RMDBID_JUMP
	"RETURN",			// RMDBID_RETURN
	"PAUSE",		// RMDBID_PAUSE
	"WAIT",		// RMDBID_WAIT
	
	/* Return Value */
	"SETRETURNVALUE",		// RMDBID_SETRETURNVALUE
	"GETRETURNVALUE",		// RMDBID_GETRETURNVALUE
	
	/* Memory */
	"COPY",		// RMDBID_COPY
	"SWAP",		// RMDBID_SWAP
	
	/* Getting and Setting */
	"SETVALUE_VOID",		// RMDBID_SETVALUE_VOID
	"SETVALUE_INTEGER",		// RMDBID_SETVALUE_INTEGER
	"SETVALUE_FIXED",		// RMDBID_SETVALUE_FIXED
	"SETVALUE_MOBJ",		// RMDBID_SETVALUE_MOBJ
	"SETVALUE_STRING",		// RMDBID_SETVALUE_STRING
	
	/* Casting */
	"CAST",		// RMDBID_CAST

	/* Arithmetic */
	"ADD",		// RMDBID_ADD
	"SUBTRACT",		// RMDBID_SUBTRACT
	"MULTIPLY",		// RMDBID_MULTIPLY
	"DIVIDE",		// RMDBID_DIVIDE
	"MODULUS",		// RMDBID_MODULUS
	"BINARYAND",		// RMDBID_BINARYAND
	"BINARYOR",		// RMDBID_BINARYOR
	"BINARYXOR",		// RMDBID_BINARYXOR
	"BINARYNOT",		// RMDBID_BINARYNOT
	"BINARYXNOR",		// RMDBID_BINARYXNOR
	"BINARYSHIFTLEFT",		// RMDBID_BINARYSHIFTLEFT
	"BINARYSHIFTRIGHT",		// RMDBID_BINARYSHIFTRIGHT
};

/* RMD_CreateByte() -- Creates an instruction at a location with passed parameters */
int RMD_CreateByte(uint32_t* Loc, rmdbyteid_t code, char* types, ...)
{
	va_list vars;
	rmdbyte_t* TempByte = NULL;
	uint32_t Offset = 0;
	char* VData = NULL;
	char* IData = NULL;
	void* WrappedPtr = NULL;
	ssize_t WrappedDiff = 0;
	uint32_t UsedSize = 0;
	
	int32_t Ti32 = 0;
	uint32_t Tui32 = 0;
	char* Tc = 0;
	
	/* Check Validity */
	if (!Loc || !types)
		return 0;
		
	if (!(code >= 0 && code < MAXRMDBYTEID))
	{
		CONS_Printf("Compiler Warning: Unknown instruction \"%s\"\n", code);
		return 0;
	}
	
	va_start(vars, types);
	
	/* Check start bytes */
	if (*Loc >= (ByteCodeZone - (sizeof(uint32_t) * 4)))
	{
		TempByte = Z_Malloc(sizeof(rmdbyte_t) * ((ByteCodeZone / sizeof(rmdbyte_t)) + 100), PU_STATIC, NULL);
		memcpy(TempByte, ByteCode, ByteCodeZone);
		Z_Free(ByteCode);
		ByteCode = TempByte;
		ByteCodeZone = ByteCodeZone + (sizeof(rmdbyte_t) * 100);
	}
	
	WrappedPtr = (size_t)ByteCode + *Loc;
	
	/* Write the marker byte */
	WriteUInt32(&WrappedPtr, code);
	WriteUInt32(&WrappedPtr, 0);
	
	/* Write Our Data */
	VData = ValidCodes[code];
	IData = types;
	
	while (*VData)
	{
		if (WrappedPtr >= (size_t)ByteCode + (ByteCodeZone - (sizeof(uint32_t) * 4)))
		{	
			// Now Resize
			TempByte = Z_Malloc(sizeof(rmdbyte_t) * ((ByteCodeZone / sizeof(rmdbyte_t)) + 100), PU_STATIC, NULL);
			memcpy(TempByte, ByteCode, ByteCodeZone);
			
			// Get the difference
			WrappedDiff = (size_t)WrappedPtr - (ssize_t)ByteCode;
			
			// Move over
			Z_Free(ByteCode);
			ByteCode = TempByte;
			ByteCodeZone = ByteCodeZone + (sizeof(rmdbyte_t) * 100);
			
			// Restore
			WrappedPtr = (size_t)ByteCode + WrappedDiff;
		}
	
		switch (*VData)
		{
			case 'i':
				if (*IData)
				{
					switch (*IData)
					{
						case 'i':
							Ti32 = va_arg(vars, int32_t);
							WriteInt32(&WrappedPtr, Ti32);
							break;
						case 't':
						case 'u':
							Tui32 = va_arg(vars, uint32_t);
							if (Tui32 >= 2147483647)
								Ti32 = 2147483647;
							else
								Ti32 = Tui32;
							WriteInt32(&WrappedPtr, Ti32);
							break;
						case 's':
							break;
						default:
							break;
					}
				}
				else
					WriteInt32(&WrappedPtr, 0);
				
				UsedSize += 4;
				break;
			
			case 't':
			case 'u':
				if (*IData)
				{
					switch (*IData)
					{
						case 'i':
							Ti32 = va_arg(vars, int32_t);
							if (Ti32 <= 0)
								Tui32 = 0;
							else
								Tui32 = Ti32;
							WriteUInt32(&WrappedPtr, Tui32);
							break;
						case 't':
						case 'u':
							Tui32 = va_arg(vars, uint32_t);
							WriteUInt32(&WrappedPtr, Tui32);
							break;
						case 's':
							break;
						default:
							break;
					}
				}
				else
					WriteUInt32(WrappedPtr, 0);
				
				UsedSize += 4;
				break;
				
			case 's':
				/*if (*IData)
				{
					switch (*IData)
					{
						case 'i':
							break;
						case 't':
						case 'u':
							break;
						case 's':
							break;
						default:
							break;
					}
				}
				else
				{*/
					WriteInt8(&WrappedPtr, 'n');
					WriteInt8(&WrappedPtr, 'i');
					WriteInt8(&WrappedPtr, 'l');
					WriteInt8(&WrappedPtr, 0);
					UsedSize += 4;
				/*}*/
				break;
		}
		
		VData++;
		
		if (*IData)
			IData++;
	}
	
	va_end(vars);
	
	WrappedPtr = (size_t)ByteCode + *Loc + sizeof(uint32_t);
	WriteUInt32(&WrappedPtr, UsedSize);
	*Loc += UsedSize + (sizeof(uint32_t) * 2);
	
	if (devparm)
		CONS_Printf("Created byte code %i (%s) w/ size %i\n", code, NameCodes[code], UsedSize);

	return 1;
}

void Command_DumpAssembly_f(void)
{
	uint8_t* Code;
	char* x;
	uint32_t OpCode = 0;
	char String[128];
	char Comment[128];
	int32_t i;
	uint32_t u;
	uint32_t* p;
	size_t j;
	size_t k = 0;
	
	if (!ByteCode)
	{
		CONS_Printf("Failed to dump byte code!\n");
	}
	else
	{
		Code = ByteCode;
		
		while (Code < (size_t)ByteCode + ByteCodeZone)
		{
			OpCode = ReadUInt32(&Code);
			ReadUInt32(&Code);
			x = ValidCodes[OpCode];
			memset(String, 0, sizeof(String));
			memset(Comment, 0, sizeof(Comment));
			
			if (OpCode == 0)
				break;
			
			CONS_Printf("%-20s ", NameCodes[OpCode]);
			
			while (*x)
			{
				i = 0;
				u = 0;
				
				switch (*x)
				{
					case 'i':
						i = ReadInt32(&Code);
						CONS_Printf("i:%i ", i);
						break;
					case 'u':
					case 't':
						u = ReadUInt32(&Code);
						CONS_Printf("u:%08xh ", u);
						break;
					case 's':
						break;
					default:
						break;
				}
				
				if (u || i)
				{
					if (u)
						p = &u;
					else
						p = &i;
					
					if (*p >= 0x80000000 && *p <= 0x87FFFFFF)
					{
						for (j = 0; j < NumSymbols; j++)
							if (Symbols[j].PhysicalLocation == *p)
							{
								snprintf(Comment, sizeof(Comment), "%s %i == `%s`,", Comment, x - ValidCodes[OpCode], Symbols[j].ID);
								break;
							}
					}
					else if (*p == 0xFFFFFFFF)
						snprintf(Comment, sizeof(Comment), "%s par %i is invalid,", Comment, x - ValidCodes[OpCode]);
				}
				
				x++;
			}
			
			if (Comment[0])
				CONS_Printf(";%s\n", Comment);
			else
				CONS_Printf("\n");
			
			k++;
		}
	}
}

/* RMD_CreateSymbol() -- */
typedef struct UserChunk_s
{
	uint32_t Variable;
	uint32_t Size;
} UserChunk_t;

uint32_t RMD_CreateSymbol(uint16_t ParentGroup, uint16_t Group, char* ID, rmdtypeid_t Type)
{
	RMD_Symbol_t* NewSymbol;
	
	if (!UserZone)
	{
		UserZone = Z_Malloc(1024, PU_STATIC, NULL);
		memset(UserZone, 0, 1024);
		UserZoneSize = 1024;
	}
	
	if (!Symbols)
	{
		Symbols = Z_Malloc(sizeof(RMD_Symbol_t), PU_STATIC, NULL);
		memset(Symbols, 0, sizeof(RMD_Symbol_t));
		NumSymbols = 1;
	}
	else
	{
		NewSymbol = Z_Malloc(sizeof(RMD_Symbol_t) * (NumSymbols + 1), PU_STATIC, NULL);
		memset(NewSymbol, 0, sizeof(RMD_Symbol_t) * (NumSymbols + 1));
		memcpy(NewSymbol, Symbols, sizeof(RMD_Symbol_t) * NumSymbols);
		Z_Free(Symbols);
		Symbols = NewSymbol;
		NumSymbols++;
	}
	
	strncpy(Symbols[NumSymbols - 1].ID, ID, 32);
	Symbols[NumSymbols - 1].Type = Type;
	Symbols[NumSymbols - 1].Group = Group;
	Symbols[NumSymbols - 1].PhysicalLocation = (uint32_t)0x80000000 + (NumSymbols - 1);
	Symbols[NumSymbols - 1].Size = 0;
	
	if (devparm)
		CONS_Printf("Created symbol \"%s\" of type %xh.\n", Symbols[NumSymbols - 1].ID, Type);
	
	return Symbols[NumSymbols - 1].PhysicalLocation;
}

/* ########################################################################## */
/* ######################### ReMooD Script Compilers ######################## */
/* ########################################################################## */

/* RMD_CompileScript() -- ReMooD Script Compiler */
int RMD_CompileScript(WadIndex_t Lump)
{
	return 0;
}

#endif

