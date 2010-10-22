// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.sourceforge.net/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           Demyx                  (demyx@endgameftw.com)
//                    Dragan                 (poliee13@hotmail.com)
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

#ifndef __RMD_MAIN_H__
#define __RMD_MAIN_H__

#include "doomtype.h"
#include "w_wad.h"

#if 0

#define MAXDEEPCALLS		10
#define MAXREGISTERS		100
#define TOKENCOUNT			512
#define NEWTOKENSIZE		512
#define TOKENCHUNKSIZE		1024
#define MAXTOKENCHUNKS		16
#define RMD_POINTERSIZE TypeSizes[RMDTID_INTEGER]
#define RMD_VARIABLESIZE ((size_t)-1)
#define RMD_CORRECTSIZE(t) ((t) & RMDTID_POINTER ? RMD_POINTERSIZE : (t) & RMDTID_MASK)
#define RMDTID_LINKED 1073741824

typedef enum
{
	RMDBID_NULL,

	/* Flow Control */
	RMDBID_TRAP,
	RMDBID_CALL,
	RMDBID_JUMP,
	RMDBID_RETURN,
	RMDBID_PAUSE,
	RMDBID_WAIT,
	
	/* Return Value */
	RMDBID_SETRETURNVALUE,
	RMDBID_GETRETURNVALUE,
	
	/* Memory */
	RMDBID_COPY,
	RMDBID_SWAP,
	
	/* Setting Values */
	RMDBID_SETVALUE_VOID,
	RMDBID_SETVALUE_INTEGER,
	RMDBID_SETVALUE_FIXED,
	RMDBID_SETVALUE_MOBJ,
	RMDBID_SETVALUE_STRING,
	
	/* Casting */
	RMDBID_CAST,

	/* Arithmetic */
	RMDBID_ADD,
	RMDBID_SUBTRACT,
	RMDBID_MULTIPLY,
	RMDBID_DIVIDE,
	RMDBID_MODULUS,
	RMDBID_BINARYAND,
	RMDBID_BINARYOR,
	RMDBID_BINARYXOR,
	RMDBID_BINARYNOT,
	RMDBID_BINARYXNOR,
	RMDBID_BINARYSHIFTLEFT,
	RMDBID_BINARYSHIFTRIGHT,
	
	MAXRMDBYTEID
} rmdbyteid_t;

typedef enum
{
	RMDTID_VOID,				// void
	RMDTID_INTEGER,				// integer
	RMDTID_REAL,				// real
	RMDTID_FIXED,				// fixed
	RMDTID_BOOLEAN,				// boolean
	RMDTID_CHAR,				// char (-128 to 127)
	RMDTID_BYTE,				// byte (0 to 255)
	RMDTID_STRING,				// string
	RMDTID_MOBJ,				// Map Object
	RMDTID_NATIVE,				// native
	
	RMDTID_POINTER =    0x1000,	// Pointer
	RMDTID_FUNCTION =   0x2000,	// Function
	RMDTID_CONST =      0x4000,	// Constant
	RMDTID_HUB =        0x8000,	// Hub
	RMDTID_STATIC =    0x10000,	// Static
	RMDTID_EXTERN =    0x20000,	// Extern
	RMDTID_SYMLINK =   0x40000,	// SymLink
	RMDTID_PTRISINT =  0x80000,	// Pointer is an Int
	RMDTID_PTRTOINT = 0x100000,	// Pointer points to an Int
	
	RMDTID_MASK = RMDTID_POINTER | RMDTID_FUNCTION | RMDTID_CONST | RMDTID_HUB | RMDTID_STATIC | RMDTID_EXTERN | RMDTID_SYMLINK | RMDTID_PTRISINT | RMDTID_PTRTOINT,
	
	MAXRMDTYPEID = RMDTID_NATIVE + 1,
} rmdtypeid_t;

typedef struct rmdbyte_s
{
	rmdbyteid_t Type;
	UInt32 Size;
} rmdbyte_t;

// Symbol
typedef struct RMD_Symbol_s
{
	char ID[32];
	rmdtypeid_t Type;
	UInt32 Group;
	UInt32 PhysicalLocation;
	UInt32 Size;
} RMD_Symbol_t;

// Fucntions
int RMD_CompileScript(WadIndex_t Lump);
int RMD_ExecuteScript(UInt16 Number);
int RMD_PauseExecution(void);
int RMD_ResumeExecution(void);
int RMD_ResetCompiler(void);
int RMD_ClearCompiler(void);
int RMD_CreateTokens(void);
int RMD_RemoveTokens(void);
UInt32 RMD_CreateSymbol(UInt16 ParentGroup, UInt16 Group, char* ID, rmdtypeid_t Type);
int RMD_CreateByte(UInt32* Loc, rmdbyteid_t code, char* types, ...);

// Tables
extern size_t TypeSizes[MAXRMDTYPEID];
extern UInt16* GroupParent;
extern size_t NumGroups;
extern RMD_Symbol_t* Symbols;
extern size_t NumSymbols;
extern UInt8* UserZone;
extern size_t UserZoneSize;
extern rmdbyte_t* ByteCode;
extern size_t ByteCodeZone;
extern char** TokenBuf;
extern char** TokenTemp;
extern char** TokenChunks;
extern size_t NumTokens;
extern size_t NumTokenChunks;
extern size_t CurrentToken;
extern size_t CurrentTokenChunk;
extern size_t CurrentTokenChunkPos;

#endif

int RMD_ClearOldLevelCode(void);
void Command_DumpAssembly_f(void);
int RMD_CompileLegacyScript(WadIndex_t Lump);

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/* RMD_Type_t -- Type of parameter or object */
typedef enum
{
	/* Types */
	RMT_VOID				= 0x00000000,
	RMT_SIGNEDINTEGER		= 0x00000001,	// -2,147,483,648 to 2,147,483,647
	RMT_UNSIGNEDINTEGER		= 0x00000002,	// 0 to 4,294,967,295
	RMT_DOOMFIXED			= 0x00000003,	// Doom Fixed Point
	RMT_CFLOAT				= 0x00000004,	// C Floating Point
	RMT_SIGNEDCHAR			= 0x00000005,	// -128 to 127
	RMT_UNSIGNEDCHAR		= 0x00000006,	// 0 to 255
	RMT_BOOLEAN				= 0x00000007,	// true or false
	RMT_STRING				= 0x00000008,	// String
	RMT_FUNCTION			= 0x00000009,	// Function
	RMT_MOBJ				= 0x0000000A,	// Map Object
	RMT_MAPTHING			= 0x0000000B,	// Map Thing
	RMT_VERTEX				= 0x0000000C,	// Vertex (on the map)
	RMT_LINE				= 0x0000000D,	// Line (on the map)
	RMT_SIDE				= 0x0000000E,	// Side (on the map)
	RMT_SECTOR				= 0x0000000F,	// Sector (on the map)
	RMT_POINTER				= 0x00000010,	// Pointer
	RMT_STARTDOTDOTDOT		= 0x00000011,	// Start of a ...
	RMT_ENDDOTDOTDOT		= 0x00000012,	// End of a ...
	RMT_PLAYER				= 0x00000013,	// Player
	RMT_NETPLAYER			= 0x00000014,	// Network Player
	RMT_ONETWENTYEIGHT		= 0x00000015,	// Unsigned 128-bit Number
	
	/* Flags */
	RMT_FLAGSMASK			= 0xFFFF0000,	// Mask for below
	
	RMT_POINTEROBJECT		= 0x00010000,	// Pointer to an object
	RMT_POINTERPOINTER		= 0x00020000,	// Pointer to a pointer
	RMT_IGNOREDOTDOTDOT		= 0x00040000,	// Do not treat as ...
	RMT_STATIC				= 0x00080000,	// Static (Allocated at start and freed at end)
	RMT_NOTTHREADED			= 0x00100000,	// Do not thread, wait until execution stops to execute this
	RMT_ARRAY				= 0x00200000,	// Treat as an array
	
	RMT_ENDTYPELIST			= 0xFFFFFFFF,	// End of list
} RMD_Type_t;

/* RMD_Data_t -- Data */
typedef struct RMD_Data_s
{
	UInt32 Type;
	UInt32 Data;
} RMD_Data_t;

/* RMD_InstructionFunction -- Type for an instruction */
struct RMD_Instruction_s;
typedef boolean (*RMD_InstructionFunction)(struct RMD_Instruction_s*, RMD_Data_t*);

/* RMD_Instruction_t -- Assembly Instruction */
typedef struct RMD_Instruction_s
{
	CONST char* Name;
	CONST size_t NumParameters;
	CONST UInt32* Parameters;
	CONST RMD_InstructionFunction Function;
} RMD_Instruction_t;

#endif /* __RMD_MAIN_H__ */

