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
// Copyright (C) 1993-1996 by id Software, Inc.
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
//      Archiving: SaveGame I/O.

/***************
*** INCLUDES ***
***************/

#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "r_state.h"
#include "z_zone.h"
#include "w_wad.h"
#include "p_setup.h"
#include "t_vari.h"
#include "t_script.h"
#include "t_func.h"
#include "m_random.h"
#include "m_misc.h"
#include "p_saveg.h"
#include "console.h"
#include "p_demcmp.h"

/****************
*** FUNCTIONS ***
****************/

/* CLC_SaveGame() -- Saves the game */
static CONL_ExitCode_t CLC_SaveGame(const uint32_t a_ArgC, const char** const a_ArgV)
{
	/* Check */
	if (a_ArgC < 2)
	{
		CONL_OutputF("Usage: %s \"<filename>\"\n", a_ArgV[0]);
		return CLE_INVALIDARGUMENT;
	}
	
	/* Save the game */
	if (strcasecmp(a_ArgV[0], "save") == 0)
	{
		if (P_SaveGameEx(NULL, a_ArgV[1], strlen(a_ArgV[1]), NULL, NULL))
			return CLE_SUCCESS;
	}
	
	/* Load the game */
	else if (strcasecmp(a_ArgV[0], "load") == 0)
	{
		if (P_LoadGameEx(NULL, a_ArgV[1], strlen(a_ArgV[1]), NULL, NULL))
			return CLE_SUCCESS;
	}
	
	/* Return success always */
	return CLE_FAILURE;
}

/* P_InitSGConsole() -- Initialize save command */
void P_InitSGConsole(void)
{
	/* Add command */
	CONL_AddCommand("save", CLC_SaveGame);
	CONL_AddCommand("load", CLC_SaveGame);
}

/* P_SaveGameStream() -- Save game into specified stream */
bool_t P_SaveGameStream(D_TStreamSource_t* const a_Stream)
{
	D_TBlock_t* NB;
	void* d;
	
	/* Check */
	if (!a_Stream)
		return false;
	
	/* Print message */
	CONL_OutputF("{4Saving game...");
	
	/* "HELO" Block */
	NB = D_BlockNew(D_CharToMagic("HELO"), 0, 256, &d);
	snprintf(d, 256, "ReMooD %i.%i%c (%s)", REMOOD_MAJORVERSION, REMOOD_MINORVERSION, REMOOD_RELEASEVERSION, REMOOD_VERSIONCODESTRING);
	D_BlockSend(a_Stream, &NB);
	
	/* Print message */
	CONL_OutputF("done!{z\n");
	
	return true;
}

/* P_LoadGameStream() -- Load game into specified stream */
bool_t P_LoadGameStream(D_TStreamSource_t* const a_Stream)
{
	/* Check */
	if (!a_Stream)
		return false;
}


#define VERSIONSIZE 16
uint8_t* save_p = NULL;			// Pointer to the data

/*** REAL STUFF ***/
uint8_t* SaveBlock = NULL;
uint8_t* SaveStart = NULL;
size_t SaveLimit = 0;

/*** PROTOTYPES ***/
void P_SAVE_WadState(void);
void P_SAVE_Console(void);
void P_SAVE_LevelState(void);

void P_SAVE_Players(void);

void P_SAVE_MapObjects(void);

/*** SAVING AND LOADING ***/

/* P_CheckSizeEx() -- Resize buffer */
bool_t P_CheckSizeEx(size_t Need)
{
	return true;
}

/* P_SaveGameEx() -- Extended savegame */
bool_t P_SaveGameEx(const char* SaveName, const char* ExtFileName, size_t ExtFileNameLen, size_t* SaveLen, uint8_t** Origin)
{
	bool_t OK = false;
	D_RBlockStream_t* BS = D_RBSCreateFileStream(ExtFileName);
	
	if (BS)
		OK = P_SaveGameToBS(BS);
	
	return OK;
}

/* P_LoadGameEx() -- Load an extended save game */
bool_t P_LoadGameEx(const char* FileName, const char* ExtFileName, size_t ExtFileNameLen, size_t* SaveLen, uint8_t** Origin)
{
	bool_t OK = false;
	D_RBlockStream_t* BS = D_RBSCreateFileStream(ExtFileName);
	
	if (BS)
	{
		// Clear the level and set title screen (JUST IN CASE!)
		P_ExClearLevel();
		gamestate = GS_DEMOSCREEN;
		
		// Load the game
		OK = P_LoadGameFromBS(BS);
	}
	
	return OK;
}

/* PLG_FutureDeref_t -- Future derefence list */
typedef struct PLG_FutureDeref_s
{
	uint64_t UniqPtr;							// Unique Pointer
	void* SetVal;								// Set to this value (ptr)	
	
	void*** ChangePtr;							// Memory to change
	size_t NumChangePtr;						// Numbers to change
} PLG_FutureDeref_t;

// Deref Data
static PLG_FutureDeref_t* l_Derefs = NULL;
static size_t l_NumDerefs = 0;

/* PLGS_SetRef() -- Set reference */
// This allocates something is pointed to
static void PLGS_SetRef(const uint32_t a_UniqPtr, void* const a_SetVal)
{
}

/* PLGS_DeRef() -- Dereference addition */
static void PLGS_DeRef(const uint32_t a_UniqPtr, void** const a_PtrRef)
{
}

/*****************************************************************************/
/*****************************************************************************/
// Super Merged Save Handling -- This replaces any existing separate load/save
// formats and instead merges them into a single function.

// [05/10 01:25:27 AM] <@GhostlyDeath> Or really functions that move both ways
// [05/10 01:25:33 AM] <@GhostlyDeath> yeah that would work much better
// [05/10 01:25:39 AM] <@GhostlyDeath> Call the SAME function
// [05/10 01:25:40 AM] <@GhostlyDeath> BUT
// [05/10 01:25:48 AM] <@GhostlyDeath> depending on read/write a different action is performed
// [05/10 01:25:56 AM] <@GhostlyDeath> to in reality it is just a single piece of coe
// [05/10 01:25:57 AM] <@GhostlyDeath> but
// [05/10 01:26:05 AM] <@GhostlyDeath> It would be capable of read/writing to both at once
// [05/10 01:26:16 AM] <@GhostlyDeath> And I wouldn't need to worry about syncing at all
// [05/10 01:26:29 AM] <@GhostlyDeath> Since the function calls would be the same
// [05/10 01:26:31 AM] <@GhostlyDeath> cool

/*** CONSTANTS ***/

/* P_SGBWTypeC_t -- Type in C */
typedef enum P_SGBWTypeC_e
{
	PSTC_CHAR,									// char
	PSTC_SCHAR,									// signed char
	PSTC_SHORT,									// (signed) short
	PSTC_INT,									// (signed) int
	PSTC_LONG,									// (signed) long
	PSTC_UCHAR,									// unsigned char
	PSTC_USHORT,								// unsigned short
	PSTC_UINT,									// unsigned int
	PSTC_ULONG,									// unsigned long
	PSTC_FIXEDT,								// fixed_t
	PSTC_FLOAT,									// float
	PSTC_DOUBLE,								// double
	PSTC_POINTER,								// void*
	PSTC_STRING,								// char*
	PSTC_INT8,									// Int8
	PSTC_INT16,									// Int16
	PSTC_INT32,									// Int32
	PSTC_INT64,									// Int64
	PSTC_UINT8,									// UInt8
	PSTC_UINT16,								// UInt16
	PSTC_UINT32,								// UInt32
	PSTC_UINT64,								// UInt64
	PSTC_INTPTR,								// intptr_t
	PSTC_UINTPTR,								// uintptr_t
	PSTC_SIZET,									// size_t
	PSTC_SSIZET,								// ssize_t
	
	NUMPSTCS
} P_SGBWTypeC_t;

/* P_SGBWTypeRec_t -- Type in block */
typedef enum P_SGBWTypeRec_e
{
	PSRC_INT8,									// Int8
	PSRC_INT16,									// Int16
	PSRC_INT32,									// Int32
	PSRC_UINT8,									// UInt8
	PSRC_UINT16,								// UInt16
	PSRC_UINT32,								// UInt32
	PSRC_STRING,								// String
	PSRC_POINTER,								// Pointer
	
	NUMPSRCS
} P_SGBWTypeRec_t;

/*** STATICS ***/

// __REMOOD_PRWSBASE -- Handles basic integer types (they are all the same anyway)
#define __REMOOD_PRWSBASE(x,y) static bool_t PRWS_##y(D_RBlockStream_t* const a_Stream, const bool_t a_Load, const P_SGBWTypeRec_t a_RecType, void* a_Ptr)\
{\
	if (a_Load)\
		switch (a_RecType)\
		{\
			case PSRC_INT8: *((x*)a_Ptr) = D_RBSReadInt8(a_Stream);\
			case PSRC_INT16: *((x*)a_Ptr) = D_RBSReadInt16(a_Stream);\
			case PSRC_INT32: *((x*)a_Ptr) = D_RBSReadInt32(a_Stream);\
			case PSRC_UINT8: *((x*)a_Ptr) = D_RBSReadUInt8(a_Stream);\
			case PSRC_UINT16: *((x*)a_Ptr) = D_RBSReadUInt16(a_Stream);\
			case PSRC_UINT32: *((x*)a_Ptr) = D_RBSReadUInt32(a_Stream);\
			default: return false;\
		}\
	else\
		switch (a_RecType)\
		{\
			case PSRC_INT8: D_RBSWriteInt8(a_Stream, *((x*)a_Ptr));\
			case PSRC_INT16: D_RBSWriteInt16(a_Stream, *((x*)a_Ptr));\
			case PSRC_INT32: D_RBSWriteInt32(a_Stream, *((x*)a_Ptr));\
			case PSRC_UINT8: D_RBSWriteUInt8(a_Stream, *((x*)a_Ptr));\
			case PSRC_UINT16: D_RBSWriteUInt16(a_Stream, *((x*)a_Ptr));\
			case PSRC_UINT32: D_RBSWriteUInt32(a_Stream, *((x*)a_Ptr));\
			default: return false;\
		}\
	return true;\
}

__REMOOD_PRWSBASE(char,char);
__REMOOD_PRWSBASE(signed char,signedchar);
__REMOOD_PRWSBASE(signed short,signedshort);
__REMOOD_PRWSBASE(signed int,signedint);
__REMOOD_PRWSBASE(signed long,signedlong);
__REMOOD_PRWSBASE(unsigned char,unsignedchar);
__REMOOD_PRWSBASE(unsigned short,unsignedshort);
__REMOOD_PRWSBASE(unsigned int,unsignedint);
__REMOOD_PRWSBASE(unsigned long,unsignedlong);
__REMOOD_PRWSBASE(fixed_t,fixedt);

#undef __REMOOD_PRWSBASE

// l_NativeData -- Native data handlers
static const struct
{
	size_t Size;								// Size of data
	bool_t (*RWFunc)(D_RBlockStream_t* const a_Stream, const bool_t a_Load, const P_SGBWTypeRec_t a_RecType, void* a_Ptr);
} l_NativeData[NUMPSTCS] =
{
	{sizeof(char), PRWS_char},					// PSTC_CHAR
	{sizeof(signed char), PRWS_signedchar},		// PSTC_SCHAR
	{sizeof(signed short), PRWS_signedshort},	// PSTC_SHORT
	{sizeof(signed int), PRWS_signedint},		// PSTC_INT
	{sizeof(signed long), PRWS_signedlong},		// PSTC_LONG
	{sizeof(unsigned char), PRWS_unsignedchar},	// PSTC_UCHAR
	{sizeof(unsigned short), PRWS_unsignedshort},	// PSTC_USHORT
	{sizeof(unsigned int), PRWS_unsignedint},	// PSTC_UINT
	{sizeof(unsigned long), PRWS_unsignedlong},	// PSTC_ULONG
	{sizeof(fixed_t), PRWS_fixedt},				// PSTC_FIXEDT
	{sizeof(float),},								// PSTC_FLOAT
	{sizeof(double),},								// PSTC_DOUBLE
	{sizeof(void*),},								// PSTC_POINTER
	{sizeof(char*),},								// PSTC_STRING
	{sizeof(int8_t),},								// PSTC_INT8
	{sizeof(int16_t),},							// PSTC_INT16
	{sizeof(int32_t),},							// PSTC_INT32
	{sizeof(int64_t),},							// PSTC_INT64
	{sizeof(uint8_t),},							// PSTC_UINT8
	{sizeof(uint16_t),},							// PSTC_UINT16
	{sizeof(uint32_t),},							// PSTC_UINT32
	{sizeof(uint64_t),},							// PSTC_UINT64
	{sizeof(intptr_t),},							// PSTC_INTPTR
	{sizeof(uintptr_t),},							// PSTC_UINTPTR
	{sizeof(size_t),},								// PSTC_SIZET
	{sizeof(ssize_t),},							// PSTC_SSIZET
};

/*** FUNCTIONS ***/

/* P_SGBiWayReadOrWrite() -- Read or write data */
// This one should be dirty and ugly
bool_t P_SGBiWayReadOrWrite(
		D_RBlockStream_t* const a_Stream,
		const bool_t a_Load,
		void* const a_Ptr,
		const size_t a_Size,
		const P_SGBWTypeC_t a_NativeType,
		const P_SGBWTypeRec_t a_RecType
	)
{
	/* Sanity Checks */
	if (devparm)
	{
		// Size and native size don't match
		if (a_Size != l_NativeData[a_NativeType].Size)
			CONL_PrintF("WARNING: NTS Mismatch (%u vs %u)\n",
					(unsigned int)a_Size, (unsigned int)l_NativeData[a_NativeType].Size);
	}
	
	/* Handle Native Read/Write */
	if (l_NativeData[a_NativeType].RWFunc)
		return l_NativeData[a_NativeType].RWFunc(a_Stream, a_Load, a_RecType, a_Ptr);
	return false;
}

/* P_SGBiWayBS() -- Saving function that goes both ways */
// This one should be clean and neat
bool_t P_SGBiWayBS(D_RBlockStream_t* const a_Stream, const bool_t a_Load)
{
	// __HEADER -- Determines if header matches or starts a new one
#define __HEADER(s) (a_Load ? (strcasecmp((s), Header)) : (D_RBSBaseBlock(a_Stream, (s))))
	// __REC -- Only records block
#define __REC (a_Load ? JunkityJunk = 0 : D_RBSRecordBlock(a_Stream))
	// __BI -- Reads or loads data
#define __BI(x,nt,rc) P_SGBiWayReadOrWrite(a_Stream, a_Load, &x, sizeof(x), PSTC_##nt, PSRC_##rc)

	bool_t Continue;
	char Header[5];
	char JunkityJunk;
	
	/* Check */
	if (!a_Stream)
		return false;
	
	/* Debug */
	CONL_PrintF("%s the game...\n", (a_Load ? "Loading" : "Saving"));
	
	/* Clear */
	Continue = true;
	
	/* Infinite Loop */
	while (Continue)
	{
		//////////////////////////////
		// If loading, read block (play it back)
		memset(Header, 0, sizeof(Header));
		if (a_Load)
			if (!(Continue = D_RBSPlayBlock(a_Stream, Header)))
				break;
		
		//////////////////////////////
		// Game State
		if (__HEADER("SGGS"))
		{
			__BI(gamestate,INT,UINT8);
			
			__REC;
		}
		
		//////////////////////////////
		// If saving, don't continue
		if (!a_Load)
			Continue = false;
	}
	
	/* Success? */
	return true;
#undef __BI
#undef __HEADER
#undef __REC
}

/*****************************************************************************/
/*****************************************************************************/

/* P_LoadGameFromBS() -- Load game from block stream */
bool_t P_LoadGameFromBS(D_RBlockStream_t* const a_Stream)
{
#define BUFSIZE 256
	char Buf[BUFSIZE];
	char BufB[BUFSIZE];
	char Header[5];
	uint8_t VerLeg, VerMaj, VerMin, VerRel;
	uint8_t CharBit, CheckBit;
	const WL_WADFile_t* WAD;
	uint8_t u8;
	uint32_t u32;
	int32_t i, j, k, l;
	
	/* Do bi-way */
	return P_SGBiWayBS(a_Stream, true);
	
	mapthing_t* MapThing;
	sector_t* Sector;
	
	/* Check */
	if (!a_Stream)
		return false;
	
	/* Clear Future Ref */
		
	/* Constantly Read Blocks */
	memset(Header, 0, sizeof(Header));
	memset(Buf, 0, sizeof(Buf));
	while (D_RBSPlayBlock(a_Stream, Header))
	{
		if (devparm)
			CONL_PrintF("LOAD: Read %s\n", Header);
		
		// SGVR -- Version
		if (strcasecmp(Header, "SGVR") == 0)
		{
			// Read version markers
			VerLeg = D_RBSReadUInt8(a_Stream);
			VerMaj = D_RBSReadUInt8(a_Stream);
			VerMin = D_RBSReadUInt8(a_Stream);
			VerRel = D_RBSReadUInt8(a_Stream);
			
			// Print Info
			CONL_PrintF("LOAD: Loading Version %i.%i%c (%i)\n",
					VerMaj, VerMin, VerRel,
					VerLeg
				);
			
			// Read Other Info
			D_RBSReadString(a_Stream, Buf, BUFSIZE - 1);
			CONL_PrintF("LOAD: Release \"%s\"\n", Buf);
			D_RBSReadString(a_Stream, Buf, BUFSIZE - 1);
			CONL_PrintF("LOAD: Fully known as %s\n", Buf);
			D_RBSReadString(a_Stream, Buf, BUFSIZE - 1);
			CONL_PrintF("LOAD: For more info, see %s\n", Buf);
		}
		
		// Current Loaded WADS
		else if (strcasecmp(Header, "SGVW") == 0)
		{
			// Lock OCCB and Pop all WADs
			WL_LockOCCB(true);
			while (WL_PopWAD())
				;
			
			// Keep reading WADs
			while ((CharBit = D_RBSReadUInt8(a_Stream)) != 'E')
			{
				// Read file and DOS Names
				memset(Buf, 0, sizeof(Buf));
				memset(BufB, 0, sizeof(BufB));
				
				D_RBSReadString(a_Stream, Buf, BUFSIZE - 1);
				D_RBSReadString(a_Stream, BufB, BUFSIZE - 1);
				
				// Open the WAD and if that failed, try the DOSNAME
				if (!(WAD = WL_OpenWAD(Buf)))
					WAD = WL_OpenWAD(BufB);
				
				// Failed?
				if (!WAD)
				{
					CONL_PrintF("LOAD: Savegame requires \"%s\" (%s) but you do not have that WAD.\n",
						Buf, BufB);
					return false;
				}
				
				// Check some bits
				CheckBit = D_RBSReadUInt8(a_Stream); // IP
				CheckBit = D_RBSReadUInt8(a_Stream); // WN
				CheckBit = D_RBSReadUInt8(a_Stream); // VI
				
				// Ignore index offset sizes and such
				u32 = D_RBSReadUInt32(a_Stream);
				u32 = D_RBSReadUInt32(a_Stream);
				u32 = D_RBSReadUInt32(a_Stream);
				
				// Ignore integer sums
				for (i = 0; i < 8; i++)
					u32 = D_RBSReadUInt32(a_Stream);
				
				// Compare MD5/SS against WAD
				for (i = 0; i < 64; i++)
					CheckBit = D_RBSReadUInt8(a_Stream);
				
				// Push WAD
				WL_PushWAD(WAD);
			}
			
			// Unlock OCCB
			WL_LockOCCB(false);
		}
		
		// Game state info
		else if (strcasecmp(Header, "SGZS") == 0)
		{
			// Read Timing Information
			gametic = D_RBSReadUInt32(a_Stream);
			D_SyncNetSetMapTime(D_RBSReadUInt32(a_Stream));
			D_RBSReadUInt32(a_Stream);	// Ignore real time, not that important
			
			// Read State Info
			gamestate = D_RBSReadUInt8(a_Stream);
			u8 = D_RBSReadUInt8(a_Stream);	// Ignore demorecording
			u8 = D_RBSReadUInt8(a_Stream);	// Ignore demoplayback
			multiplayer = D_RBSReadUInt8(a_Stream);
			
			// Read Map Name
			memset(Buf, 0, sizeof(Buf));
			D_RBSReadString(a_Stream, Buf, BUFSIZE - 1);
			
			// Find level
			g_CurrentLevelInfo = P_FindLevelByNameEx(Buf, NULL);
		}
		
		// SGMV -- Map Vertexes
		else if (strcasecmp(Header, "SGMV") == 0)
		{
			// Read Count
			numvertexes = D_RBSReadUInt32(a_Stream);
			vertexes = Z_Malloc(sizeof(*vertexes) * numvertexes, PU_LEVEL, NULL);
			
			// Read every vertex
			for (i = 0; i < numvertexes; i++)
			{
				vertexes[i].x = D_RBSReadInt32(a_Stream);
				vertexes[i].y = D_RBSReadInt32(a_Stream);
			}
		}
		
		// SGMS -- Map Sectors
		else if (strcasecmp(Header, "SGMS") == 0)
		{
			numsectors = D_RBSReadUInt32(a_Stream);
			sectors = Z_Malloc(sizeof(*sectors) * numsectors, PU_LEVEL, NULL);
			
			// Read every sector
			for (i = 0; i < numsectors; i++)
			{
				// Get Current
				Sector = &sectors[i];
				
				// Read in
			}
		}
		
		// SGMT -- Map Thing
		else if (strcasecmp(Header, "SGMT") == 0)
		{
			// Read Count
			nummapthings = D_RBSReadUInt32(a_Stream);
			mapthings = Z_Malloc(sizeof(*mapthings) * nummapthings, PU_LEVEL, NULL);
			
			// Read every map thing
			for (i = 0; i < nummapthings; i++)
			{
				// Set pointer reference
				MapThing = &mapthings[i];
				PLGS_SetRef(D_RBSReadPointer(a_Stream), MapThing);
				
				// Read the remainder
				MapThing->x = D_RBSReadInt16(a_Stream);
				MapThing->y = D_RBSReadInt16(a_Stream);
				MapThing->z = D_RBSReadInt16(a_Stream);
				MapThing->angle = D_RBSReadInt16(a_Stream);
				MapThing->type = D_RBSReadInt16(a_Stream);
				MapThing->options = D_RBSReadInt16(a_Stream);
				PLGS_DeRef(D_RBSReadPointer(a_Stream), (void**)&MapThing->mobj);
				MapThing->IsHexen = D_RBSReadUInt8(a_Stream);
				MapThing->HeightOffset = D_RBSReadInt16(a_Stream);
				MapThing->ID = D_RBSReadUInt16(a_Stream);
				MapThing->Special = D_RBSReadUInt8(a_Stream);
				for (j = 0; j < 5; j++)
					MapThing->Args[i] = D_RBSReadUInt8(a_Stream);
				MapThing->MoType = D_RBSReadUInt32(a_Stream);
				MapThing->MarkedWeapon = D_RBSReadUInt8(a_Stream);
			}
		}
		
		// SGMR -- Other Map Stuff
		else if (strcasecmp(Header, "SGMR") == 0)
		{
			// Read Player Starts
			j = D_RBSReadUInt32(a_Stream);
			for (k = 0; k < j; k++)
			{
				u32 = D_RBSReadUInt32(a_Stream);
				if (k < MAXPLAYERS && u32 >= 0 && u32 < nummapthings)
					playerstarts[k] = &mapthings[u32];
			}
		}
		
		// SGEE -- End Stream
		else if (strcasecmp(Header, "SGEE") == 0)
			break;
		
		// Clear Header for next block movement
		memset(Header, 0, sizeof(Header));
	}
	
	/* Success */
	return true;
#undef BUFSIZE
}

// Save Game Prototypes
void P_SGBS_Time(D_RBlockStream_t* const a_Stream);
void P_SGBS_Version(D_RBlockStream_t* const a_Stream);
void P_SGBS_WAD(D_RBlockStream_t* const a_Stream);
void P_SGBS_NetProfiles(D_RBlockStream_t* const a_Stream);
void P_SGBS_SplitPlayers(D_RBlockStream_t* const a_Stream);
void P_SGBS_Players(D_RBlockStream_t* const a_Stream);
void P_SGBS_MapData(D_RBlockStream_t* const a_Stream);
void P_SGBS_Thinkers(D_RBlockStream_t* const a_Stream);
void P_SGBS_State(D_RBlockStream_t* const a_Stream);

/* P_SaveGameToBS() -- Save game to block stream */
bool_t P_SaveGameToBS(D_RBlockStream_t* const a_Stream)
{
	const char* c;
	
	/* Check */
	if (!a_Stream)
		return false;
		
	/* Do bi-way */
	return P_SGBiWayBS(a_Stream, false);
		
	/* Create Header Block */
	// Save Game Save Stream
		// Base
	D_RBSBaseBlock(a_Stream, "SGSS");
		// Fill
	for (c = "ReMooD Save Game"; *c; c++)
		D_RBSWriteUInt8(a_Stream, *c);
		// Record
	D_RBSRecordBlock(a_Stream);
	
	/* Save Details of Game */
	P_SGBS_Time(a_Stream);
	P_SGBS_Version(a_Stream);
	P_SGBS_WAD(a_Stream);
	P_SGBS_State(a_Stream);
	P_SGBS_NetProfiles(a_Stream);
	P_SGBS_SplitPlayers(a_Stream);
	P_SGBS_Players(a_Stream);
	P_SGBS_MapData(a_Stream);
	P_SGBS_Thinkers(a_Stream);
	
	/* Write End Header */
	// Save Game End Exit
		// Base
	D_RBSBaseBlock(a_Stream, "SGEE");
		// Fill
	for (c = "End Stream"; *c; c++)
		D_RBSWriteUInt8(a_Stream, *c);
		// Record
	D_RBSRecordBlock(a_Stream);
	
	/* Success */
	return true;
}

/*** SAVING THE GAME STATE ***/

/* P_SGBS_Time() -- Print Time Information */
void P_SGBS_Time(D_RBlockStream_t* const a_Stream)
{
	/* Begin Header */
	D_RBSBaseBlock(a_Stream, "SGVT");
	
	/* Place Time Information (Save Related) */
	D_RBSWriteUInt32(a_Stream, time(NULL));
	D_RBSWriteUInt32(a_Stream, g_ProgramTic);
	
	/* Record Block */
	D_RBSRecordBlock(a_Stream);
}

/* P_SGBS_Version() -- Write version information */
void P_SGBS_Version(D_RBlockStream_t* const a_Stream)
{
	/* Begin Header */
	D_RBSBaseBlock(a_Stream, "SGVR");
	
	/* Fill With Versioning */
		// Legacy Version
	D_RBSWriteUInt8(a_Stream, VERSION);
		// ReMooD Version
	D_RBSWriteUInt8(a_Stream, REMOOD_MAJORVERSION);
	D_RBSWriteUInt8(a_Stream, REMOOD_MINORVERSION);
	D_RBSWriteUInt8(a_Stream, REMOOD_RELEASEVERSION);
		// Version Strings
	D_RBSWriteString(a_Stream, REMOOD_VERSIONCODESTRING);
	D_RBSWriteString(a_Stream, REMOOD_FULLVERSIONSTRING);
	D_RBSWriteString(a_Stream, REMOOD_URL);
		// Compilation Stuff
	D_RBSWriteString(a_Stream, __TIME__);
	D_RBSWriteString(a_Stream, __DATE__);
	
	/* Record Block */
	D_RBSRecordBlock(a_Stream);
}

/* P_SGBS_WAD() -- Write WAD State */
void P_SGBS_WAD(D_RBlockStream_t* const a_Stream)
{
	const WL_WADFile_t* CurVWAD;
	size_t i;
	
	/* Begin Header */
	D_RBSBaseBlock(a_Stream, "SGVW");
	
	/* Iterate all VWADs */	
	for (CurVWAD = WL_IterateVWAD(NULL, true); CurVWAD; CurVWAD = WL_IterateVWAD(CurVWAD, true))
	{
		D_RBSWriteUInt8(a_Stream, 'B');
		
		// Print WAD Names
		D_RBSWriteString(a_Stream, CurVWAD->__Private.__FileName);
		D_RBSWriteString(a_Stream, CurVWAD->__Private.__DOSName);
		
		// Print Some Flags
		D_RBSWriteUInt8(a_Stream, (CurVWAD->__Private.__IsIWAD ? 'I' : 'P'));
		D_RBSWriteUInt8(a_Stream, (CurVWAD->__Private.__IsWAD ? 'W' : 'N'));
		D_RBSWriteUInt8(a_Stream, (CurVWAD->__Private.__IsValid ? 'V' : 'I'));
		
		// Print Some WAD Identification
		D_RBSWriteUInt32(a_Stream, CurVWAD->NumEntries);
		D_RBSWriteUInt32(a_Stream, CurVWAD->__Private.__IndexOff);
		D_RBSWriteUInt32(a_Stream, CurVWAD->__Private.__Size);
		
		// Print WAD Sums
		for (i = 0; i < 4; i++)
			D_RBSWriteUInt32(a_Stream, CurVWAD->CheckSum[i]);
		for (i = 0; i < 4; i++)
			D_RBSWriteUInt32(a_Stream, CurVWAD->SimpleSum[i]);
		for (i = 0; i < 32; i++)
			D_RBSWriteUInt8(a_Stream, CurVWAD->CheckSumChars[i]);
		for (i = 0; i < 32; i++)
			D_RBSWriteUInt8(a_Stream, CurVWAD->SimpleSumChars[i]);
	}
	D_RBSWriteUInt8(a_Stream, 'E');
	
	/* Record Block */
	D_RBSRecordBlock(a_Stream);
}

/* P_SGBS_NetProfiles() -- Record network profiles */
void P_SGBS_NetProfiles(D_RBlockStream_t* const a_Stream)
{
	size_t i, j, k;
	D_ProfileEx_t* ProfileEx;
	D_NetPlayer_t* NetPlayer;
	ticcmd_t* TicCmd;
	
	/* Begin Header */
	D_RBSBaseBlock(a_Stream, "SGNP");
	
	/* Go through each player */
	for (i = 0; i < MAXPLAYERS; i++)
	{
		// Make sure he is in game
		if (!playeringame[i])
			continue;
		
		// Print Player ID
		D_RBSWriteUInt8(a_Stream, i);
		
		// Get Stuff
		NetPlayer = players[i].NetPlayer;
		ProfileEx = players[i].ProfileEx;
		
		// Parse Net Player
		if (NetPlayer)
		{
			// Write ticker
			D_RBSWriteUInt8(a_Stream, 'N');
			
			// Print UUID To Identify
			D_RBSWritePointer(a_Stream, NetPlayer);
			D_RBSWriteString(a_Stream, NetPlayer->UUID);
			
			// Print Profile And Player Link
			D_RBSWriteString(a_Stream, (NetPlayer->Profile ? NetPlayer->Profile->UUID : ""));
			D_RBSWriteUInt8(a_Stream, NetPlayer->Player - players);
			
			// Print Other NetInfo Stuff
			D_RBSWriteUInt8(a_Stream, NetPlayer->Type);
			D_RBSWriteString(a_Stream, NetPlayer->DisplayName);
			D_RBSWriteUInt8(a_Stream, NetPlayer->NetColor);
			
			// Print Tic Info
			for (k = 0; k < 2; k++)
			{
				// Write Tic Count
				D_RBSWriteUInt16(a_Stream, (!k ? NetPlayer->LocalTicTotal : NetPlayer->TicTotal));
				
				// Write each tic
				for (j = 0; j < (!k ? NetPlayer->LocalTicTotal : NetPlayer->TicTotal); j++)
				{
					// Current Tic
					TicCmd = (!k ? &NetPlayer->LocalTicCmd[j] : &NetPlayer->TicCmd[j]);
					
					// Write the commands out
					D_RBSWriteUInt8(a_Stream, TicCmd->forwardmove);
					D_RBSWriteUInt8(a_Stream, TicCmd->sidemove);
					D_RBSWriteUInt8(a_Stream, TicCmd->artifact);
					D_RBSWriteUInt8(a_Stream, TicCmd->XNewWeapon);
					D_RBSWriteUInt16(a_Stream, TicCmd->BaseAngleTurn);
					D_RBSWriteUInt16(a_Stream, TicCmd->angleturn);
					D_RBSWriteUInt16(a_Stream, TicCmd->aiming);
					D_RBSWriteUInt16(a_Stream, TicCmd->buttons);
				}
			}
		}
		
		// Missing net player, so ignore it
		else
			D_RBSWriteUInt8(a_Stream, 'X');
		
		// Parse Profile
		if (ProfileEx)
		{
			// Write ticker
			D_RBSWriteUInt8(a_Stream, 'P');
			
			// Print UUID To Identify
			D_RBSWritePointer(a_Stream, ProfileEx);
			D_RBSWriteString(a_Stream, ProfileEx->UUID);
			
			// Print NetPlayer Link
			D_RBSWriteString(a_Stream, (ProfileEx->NetPlayer ? ProfileEx->NetPlayer->UUID : ""));
			
			// Print Profile Info
			D_RBSWriteUInt8(a_Stream, ProfileEx->Type);
			D_RBSWriteString(a_Stream, ProfileEx->DisplayName);
			D_RBSWriteString(a_Stream, ProfileEx->AccountName);
			D_RBSWriteUInt8(a_Stream, ProfileEx->Color);
		}
		
		// Missing profile, so ignore it
		else
			D_RBSWriteUInt8(a_Stream, 'X');
	}
	
	/* Record Block */
	D_RBSRecordBlock(a_Stream);
}

/* P_SGBS_SplitPlayers() -- Split players */
void P_SGBS_SplitPlayers(D_RBlockStream_t* const a_Stream)
{
	size_t i;
	
	/* Begin Header */
	D_RBSBaseBlock(a_Stream, "SGSC");
	
	/* Print Local Players */
	D_RBSWriteUInt8(a_Stream, g_SplitScreen);
	
	// Go through Each
	for (i = 0; i < MAXSPLITSCREEN; i++)
	{
		D_RBSWriteUInt8(a_Stream, g_PlayerInSplit[i]);
		D_RBSWriteUInt8(a_Stream, consoleplayer[i]);
		D_RBSWriteUInt8(a_Stream, displayplayer[i]);
	}
	
	/* Record Block */
	D_RBSRecordBlock(a_Stream);
}

/* P_SGBS_Players() -- Dump Players */
void P_SGBS_Players(D_RBlockStream_t* const a_Stream)
{
	size_t i, j;
	player_t* Player;
	
	/* Begin Header */
	D_RBSBaseBlock(a_Stream, "SGPL");
	
	for (i = 0; i < MAXPLAYERS; i++)
	{
		Player = &players[i];
		
		// Print Identifier
		D_RBSWriteUInt8(a_Stream, i);
		D_RBSWritePointer(a_Stream, Player);
		
		// Not in game?
		if (!playeringame[i])
		{
			D_RBSWriteUInt8(a_Stream, 'V');
			continue;
		}
		
		// Write as In Game
		D_RBSWriteUInt8(a_Stream, 'P');
		
		// Write Links
		D_RBSWritePointer(a_Stream, Player->ProfileEx);
		D_RBSWriteString(a_Stream, (Player->ProfileEx ? Player->ProfileEx->UUID : ""));
		D_RBSWritePointer(a_Stream, Player->NetPlayer);
		D_RBSWriteString(a_Stream, (Player->NetPlayer ? Player->NetPlayer->UUID : ""));
		
		// Print Map Objects Connected To
		D_RBSWritePointer(a_Stream, Player->mo);
		D_RBSWritePointer(a_Stream, Player->rain1);
		D_RBSWritePointer(a_Stream, Player->rain2);
		D_RBSWritePointer(a_Stream, Player->attacker);
		
		// Write Player Info
		D_RBSWriteUInt8(a_Stream, Player->playerstate);
		D_RBSWriteInt32(a_Stream, Player->viewz);
		D_RBSWriteInt32(a_Stream, Player->viewheight);
		D_RBSWriteInt32(a_Stream, Player->deltaviewheight);
		D_RBSWriteInt32(a_Stream, Player->bob);
		D_RBSWriteUInt32(a_Stream, Player->aiming);
		D_RBSWriteInt32(a_Stream, Player->health);
		D_RBSWriteInt32(a_Stream, Player->armorpoints);
		D_RBSWriteInt32(a_Stream, Player->armortype);
		D_RBSWriteUInt32(a_Stream, Player->cards);
		D_RBSWriteUInt32(a_Stream, Player->backpack);
		D_RBSWriteUInt32(a_Stream, Player->addfrags);
		D_RBSWriteInt32(a_Stream, Player->readyweapon);
		D_RBSWriteString(a_Stream, Player->weaponinfo[Player->readyweapon]->ClassName);
		D_RBSWriteInt32(a_Stream, Player->pendingweapon);
		D_RBSWriteString(a_Stream, ((Player->pendingweapon < 0) ? "NoPending" :Player->weaponinfo[Player->pendingweapon]->ClassName));
		D_RBSWriteUInt8(a_Stream, Player->originalweaponswitch);
		D_RBSWriteUInt8(a_Stream, Player->autoaim_toggle);
		D_RBSWriteUInt8(a_Stream, Player->attackdown);
		D_RBSWriteUInt8(a_Stream, Player->usedown);
		D_RBSWriteUInt8(a_Stream, Player->jumpdown);
		D_RBSWriteUInt32(a_Stream, Player->cheats);
		D_RBSWriteUInt32(a_Stream, Player->refire);
		D_RBSWriteUInt32(a_Stream, Player->killcount);
		D_RBSWriteUInt32(a_Stream, Player->itemcount);
		D_RBSWriteUInt32(a_Stream, Player->secretcount);
		D_RBSWriteUInt32(a_Stream, Player->damagecount);
		D_RBSWriteUInt32(a_Stream, Player->bonuscount);
		D_RBSWriteUInt32(a_Stream, Player->specialsector);
		D_RBSWriteUInt32(a_Stream, Player->extralight);
		D_RBSWriteUInt32(a_Stream, Player->fixedcolormap);
		D_RBSWriteUInt32(a_Stream, Player->skincolor);
		D_RBSWriteUInt32(a_Stream, Player->skin);
		D_RBSWriteUInt8(a_Stream, Player->didsecret);
		D_RBSWriteInt32(a_Stream, Player->chickenTics);
		D_RBSWriteInt32(a_Stream, Player->chickenPeck);
		D_RBSWriteInt32(a_Stream, Player->flamecount);
		D_RBSWriteInt32(a_Stream, Player->flyheight);
		D_RBSWriteInt32(a_Stream, Player->inv_ptr);
		D_RBSWriteInt32(a_Stream, Player->st_curpos);
		D_RBSWriteInt32(a_Stream, Player->st_inventoryTics);
		D_RBSWriteInt32(a_Stream, Player->flushdelay);
		D_RBSWriteInt32(a_Stream, Player->MoveMom);
		D_RBSWriteInt32(a_Stream, Player->TargetViewZ);
		D_RBSWriteInt32(a_Stream, Player->FakeMom[0]);
		D_RBSWriteInt32(a_Stream, Player->FakeMom[1]);
		D_RBSWriteInt32(a_Stream, Player->FakeMom[2]);
		D_RBSWriteInt32(a_Stream, Player->MaxHealth[0]);
		D_RBSWriteInt32(a_Stream, Player->MaxHealth[1]);
		D_RBSWriteInt32(a_Stream, Player->MaxArmor[0]);
		D_RBSWriteInt32(a_Stream, Player->MaxArmor[1]);
		
		// Current Weapon Level
		D_RBSWriteUInt8(a_Stream, (Player->weaponinfo == wpnlev2info ? 1 : 0));
		
		// Write Variable Info
			// Powerups
		for (j = 0; j < NUMPOWERS; j++)
			D_RBSWriteInt32(a_Stream, Player->powers[j]);
			
			// Ammo
		for (j = 0; j < NUMAMMO; j++)
		{
			D_RBSWriteInt32(a_Stream, Player->ammo[j]);
			D_RBSWriteInt32(a_Stream, Player->maxammo[j]);
		}
			
			// Weapons
		for (j = 0; j < NUMWEAPONS; j++)
			D_RBSWriteUInt8(a_Stream, Player->weaponowned[j]);
			
			// Frags
		for (j = 0; j < MAXPLAYERS; j++)
			D_RBSWriteUInt32(a_Stream, Player->frags[j]);
			
			// Inventory Slots
		for (j = 0; j < NUMINVENTORYSLOTS; j++)
		{
			D_RBSWriteUInt8(a_Stream, Player->inventory[j].type);
			D_RBSWriteUInt8(a_Stream, Player->inventory[j].count);
		}
		
		// Save psprites
		for (j = 0; j < NUMPSPRITES; j++)
		{
			D_RBSWriteInt32(a_Stream, Player->psprites[j].tics);
			D_RBSWriteInt32(a_Stream, Player->psprites[j].sx);
			D_RBSWriteInt32(a_Stream, Player->psprites[j].sy);
			
			// Write current state
			if (!Player->psprites[j].state)
				D_RBSWriteUInt8(a_Stream, 'N');
			else
			{
				D_RBSWriteUInt8(a_Stream, 'S');
				D_RBSWriteUInt32(a_Stream, Player->psprites[j].state->FrameID);
				D_RBSWriteUInt32(a_Stream, Player->psprites[j].state->ObjectID);
				D_RBSWriteUInt32(a_Stream, Player->psprites[j].state->Marker);
				D_RBSWriteUInt32(a_Stream, Player->psprites[j].state->SpriteID);
				D_RBSWriteUInt32(a_Stream, Player->psprites[j].state->DehackEdID);
			}
		}
		
		// Save camera
		D_RBSWritePointer(a_Stream, Player->camera.mo);
		D_RBSWriteUInt8(a_Stream, Player->camera.chase);
		D_RBSWriteUInt32(a_Stream, Player->camera.aiming);
		D_RBSWriteUInt32(a_Stream, Player->camera.startangle);
		D_RBSWriteInt32(a_Stream, Player->camera.fixedcolormap);
		D_RBSWriteInt32(a_Stream, Player->camera.viewheight);
	}
	
	/* Record Block */
	D_RBSRecordBlock(a_Stream);
}

/* PS_SGBS_DumpMapThing() -- Dumps a map thing (occurs alot) */
void PS_SGBS_DumpMapThing(D_RBlockStream_t* const a_Stream, mapthing_t* const MapThing)
{
	D_RBSWritePointer(a_Stream, MapThing);
	D_RBSWriteInt16(a_Stream, MapThing->x);
	D_RBSWriteInt16(a_Stream, MapThing->y);
	D_RBSWriteInt16(a_Stream, MapThing->z);
	D_RBSWriteInt16(a_Stream, MapThing->angle);
	D_RBSWriteInt16(a_Stream, MapThing->type);
	D_RBSWriteInt16(a_Stream, MapThing->options);
	D_RBSWritePointer(a_Stream, MapThing->mobj);
	D_RBSWriteUInt8(a_Stream, MapThing->IsHexen);
	D_RBSWriteInt16(a_Stream, MapThing->HeightOffset);
	D_RBSWriteUInt16(a_Stream, MapThing->ID);
	D_RBSWriteUInt8(a_Stream, MapThing->Special);
	D_RBSWriteUInt8(a_Stream, MapThing->Args[0]);
	D_RBSWriteUInt8(a_Stream, MapThing->Args[1]);
	D_RBSWriteUInt8(a_Stream, MapThing->Args[2]);
	D_RBSWriteUInt8(a_Stream, MapThing->Args[3]);
	D_RBSWriteUInt8(a_Stream, MapThing->Args[4]);
	D_RBSWriteUInt32(a_Stream, MapThing->MoType);
	D_RBSWriteUInt8(a_Stream, MapThing->MarkedWeapon);
}

/* P_SGBS_MapData() -- Write All Map Data */
// Some map data cannot be moved
// Left: AFHKMQWXY
void P_SGBS_MapData(D_RBlockStream_t* const a_Stream)
{
	size_t i, j;
	mapthing_t* MapThing;
	sector_t* Sector;
	
	/* Vertexes */
	// Begin
	D_RBSBaseBlock(a_Stream, "SGMV");
	
	// Put vertexes
	D_RBSWriteUInt32(a_Stream, numvertexes);
	for (i = 0; i < numvertexes; i++)
	{
		D_RBSWriteInt32(a_Stream, vertexes[i].x);
		D_RBSWriteInt32(a_Stream, vertexes[i].y);
	}
	
	// End
	D_RBSRecordBlock(a_Stream);
	
	/* Sectors */
	// Begin
	D_RBSBaseBlock(a_Stream, "SGMS");
	
	// Put sectors
	D_RBSWriteUInt32(a_Stream, numsectors);
	for (i = 0; i < numsectors; i++)
	{
		// Get Current
		Sector = &sectors[i];
		
		// Dump
		D_RBSWriteInt32(a_Stream, Sector->floorheight);
		D_RBSWriteInt32(a_Stream, Sector->ceilingheight);
		D_RBSWriteInt32(a_Stream, Sector->nexttag);
		D_RBSWriteInt32(a_Stream, Sector->firsttag);
		D_RBSWriteInt32(a_Stream, Sector->validcount);
		D_RBSWriteInt32(a_Stream, Sector->stairlock);
		D_RBSWriteInt32(a_Stream, Sector->prevsec);
		D_RBSWriteInt32(a_Stream, Sector->nextsec);
		D_RBSWriteInt32(a_Stream, Sector->floor_xoffs);
		D_RBSWriteInt32(a_Stream, Sector->floor_yoffs);
		D_RBSWriteInt32(a_Stream, Sector->ceiling_xoffs);
		D_RBSWriteInt32(a_Stream, Sector->ceiling_yoffs);
		D_RBSWriteInt32(a_Stream, Sector->heightsec);
		D_RBSWriteInt32(a_Stream, Sector->altheightsec);
		D_RBSWriteInt32(a_Stream, Sector->floorlightsec);
		D_RBSWriteInt32(a_Stream, Sector->ceilinglightsec);
		D_RBSWriteInt32(a_Stream, Sector->teamstartsec);
		D_RBSWriteInt32(a_Stream, Sector->bottommap);
		D_RBSWriteInt32(a_Stream, Sector->midmap);
		D_RBSWriteInt32(a_Stream, Sector->topmap);
		D_RBSWriteInt32(a_Stream, Sector->validsort);
		D_RBSWriteInt32(a_Stream, FLOAT_TO_FIXED(Sector->lineoutLength));
		
		D_RBSWriteUInt32(a_Stream, Sector->special);
		D_RBSWriteUInt32(a_Stream, Sector->oldspecial);
		//D_RBSWriteUInt32(a_Stream, Sector->xxxxxxxxx);
		//D_RBSWriteUInt32(a_Stream, Sector->xxxxxxxxx);
		//D_RBSWriteUInt32(a_Stream, Sector->xxxxxxxxx);
		
		D_RBSWriteInt16(a_Stream, Sector->floorpic);
		D_RBSWriteInt16(a_Stream, Sector->ceilingpic);
		D_RBSWriteInt16(a_Stream, Sector->lightlevel);
		D_RBSWriteInt16(a_Stream, Sector->tag);
		D_RBSWriteInt16(a_Stream, Sector->soundtraversed);
		D_RBSWriteInt16(a_Stream, Sector->floortype);
		
		// Arrays
		for (j = 0; j < 4; j++)
		{
			D_RBSWriteInt32(a_Stream, Sector->blockbox[j]);
			D_RBSWriteInt32(a_Stream, Sector->BBox[j]);
		}
		
		// Variable
		
		// Pointers
		D_RBSWritePointer(a_Stream, Sector->soundtarget);

#if 0
// origin for any sounds played by the sector
S_NoiseThinker_t soundorg;


// list of mobjs in sector
mobj_t* thinglist;

//SoM: 3/6/2000: Start boom extra stuff
// thinker_t for reversable actions
void* floordata;			// make thinkers on
void* ceilingdata;			// floors, ceilings, lighting,
void* lightingdata;			// independent of one another

// list of mobjs that are at least partially in the sector
// thinglist is a subset of touching_thinglist
struct msecnode_s* touching_thinglist;	// phares 3/14/98
//SoM: 3/6/2000: end stuff...

int linecount;
struct line_s** lines;		// [linecount] size

//SoM: 2/23/2000: Improved fake floor hack
ffloor_t* ffloors;
int* attached;
int numattached;
lightlist_t* lightlist;
int numlights;
bool_t moved;

bool_t added;

// SoM: 4/3/2000: per-sector colormaps!
extracolormap_t* extra_colormap;

// ----- for special tricks with HW renderer -----
bool_t pseudoSector;
bool_t virtualFloor;
fixed_t virtualFloorheight;
bool_t virtualCeiling;
fixed_t virtualCeilingheight;
linechain_t* sectorLines;
struct sector_s** stackList;
// ----- end special tricks -----

// ReMooD Additions
char* FloorTexture;							// Name of floor texture
char* CeilingTexture;						// Name of ceiling texture
size_t SoundSecRef;							// Reference to sound sector
#endif
	}
	
	// End
	D_RBSRecordBlock(a_Stream);
	
	/* SideDefs */
	// Begin
	D_RBSBaseBlock(a_Stream, "SGMI");
	
	// End
	D_RBSRecordBlock(a_Stream);
	
	/* LineDefs */
	// Begin
	D_RBSBaseBlock(a_Stream, "SGML");
	
	// End
	D_RBSRecordBlock(a_Stream);
	
	/* SubSectors */
	// Begin
	D_RBSBaseBlock(a_Stream, "SGMU");
	
	// End
	D_RBSRecordBlock(a_Stream);
	
	/* Nodes */
	// Begin
	D_RBSBaseBlock(a_Stream, "SGMN");
	
	// End
	D_RBSRecordBlock(a_Stream);
	
	/* Segs */
	// Begin
	D_RBSBaseBlock(a_Stream, "SGMG");
	
	// End
	D_RBSRecordBlock(a_Stream);
	
	/* Block Map */
	// extern long* blockmaplump;		// offsets in blockmap are from here
	// extern long* blockmap;			// Big blockmap SSNTails
	// extern int bmapwidth;
	// extern int bmapheight;			// in mapblocks
	// extern fixed_t bmaporgx;
	// extern fixed_t bmaporgy;		// origin of block map
	// extern mobj_t** blocklinks;		// for thing chains
	
	// Begin
	D_RBSBaseBlock(a_Stream, "SGMB");
	
	// End
	D_RBSRecordBlock(a_Stream);
	
	/* Reject */
	// extern uint8_t* rejectmatrix;	// for fast sight rejection
	
	// Begin
	D_RBSBaseBlock(a_Stream, "SGMJ");
	
	// End
	D_RBSRecordBlock(a_Stream);
	
	/* Things */
	//extern int nummapthings;
	//extern mapthing_t* mapthings;
	
	// Begin
	D_RBSBaseBlock(a_Stream, "SGMT");
	
	// Record all things
	D_RBSWriteUInt32(a_Stream, nummapthings);
	for (i = 0; i < nummapthings; i++)
	{
		// Get Current
		MapThing = &mapthings[i];
		
		// Write Out
		PS_SGBS_DumpMapThing(a_Stream, MapThing);
	}
	
	// End
	D_RBSRecordBlock(a_Stream);
	
	/* Touching Sector Lists */
	//msecnode_t* sector_list = NULL;
	
	// Begin
	D_RBSBaseBlock(a_Stream, "SGMZ");
	
	// End
	D_RBSRecordBlock(a_Stream);
	
	/* Active Plats */
	// platlist_t* activeplats;
	
	// Begin
	D_RBSBaseBlock(a_Stream, "SGMP");
	
	// End
	D_RBSRecordBlock(a_Stream);
	
	/* Active Ceilings */
	// ceilinglist_t* activeceilings;
	
	// Begin
	D_RBSBaseBlock(a_Stream, "SGME");
	
	// End
	D_RBSRecordBlock(a_Stream);
	
	/* Others */
	// playerstarts[mthing->type - 1] = mthing;
	// deathmatchstarts[numdmstarts]
	// braintargets[numbraintargets]
	//extern mapthing_t* itemrespawnque[ITEMQUESIZE];
	//extern tic_t itemrespawntime[ITEMQUESIZE];
	//extern int iquehead;
	//extern int iquetail;
	//totalkills, totalitems
	//mobj_t* bodyque[BODYQUESIZE];
	//int bodyqueslot;
	
	// Begin
	D_RBSBaseBlock(a_Stream, "SGMR");
	
	// Player Starts
	D_RBSWriteUInt32(a_Stream, MAXPLAYERS);
	for (i = 0; i < MAXPLAYERS; i++)
		D_RBSWriteUInt32(a_Stream, playerstarts[i] - mapthings);
	
	// End
	D_RBSRecordBlock(a_Stream);
}

void P_MobjNullThinker(mobj_t* mobj);

/* P_SGBS_Thinkers() -- Thinkers */
// extern thinker_t thinkercap;
void P_SGBS_Thinkers(D_RBlockStream_t* const a_Stream)
{
	size_t i, j;
	thinker_t* CurThinker;
	bool_t Capped;
	
	mobj_t* Mobj;
	vldoor_t* VLDoor;
	strobe_t* Strobe;
	scroll_t* Scroll;
	pusher_t* Pusher;
	mapthing_t* MapThing;
	fireflicker_t* FireFlicker;
	friction_t* Friction;
	plat_t* Plat;
	floormove_t* FloorMove;
	glow_t* Glow;
	lightlevel_t* LightFade;
	elevator_t* Elevator;
	ceiling_t* Ceiling;
	lightflash_t* LightFlash;
	
	/* Run through all thinkers */
	// Thinkercap MUST be first
	Capped = false;
	for (CurThinker = &thinkercap;
		!Capped || (Capped && CurThinker != &thinkercap);
		CurThinker = CurThinker->next, Capped = true)
	{
		// Begin
		D_RBSBaseBlock(a_Stream, "SGTH");
		
		// Print Thinker Current Pointer ID
		D_RBSWritePointer(a_Stream, CurThinker);
		
		// P_MobjNullThinker(mobj_t* mobj) || P_MobjThinker(mobj_t* mobj)
		if (CurThinker->function.acv == P_MobjNullThinker ||
				CurThinker->function.acv == P_MobjThinker)
		{
			// Thinker Header
			D_RBSWriteUInt8(a_Stream, 'M');
			if (CurThinker->function.acv == P_MobjNullThinker)
				D_RBSWriteUInt8(a_Stream, 'N');
			else
				D_RBSWriteUInt8(a_Stream, 'O');
			
			// Get Thinker
			Mobj = (mobj_t*)CurThinker;
			
			// Dump Info
			D_RBSWriteInt32(a_Stream, Mobj->x);
			D_RBSWriteInt32(a_Stream, Mobj->y);
			D_RBSWriteInt32(a_Stream, Mobj->z);
			D_RBSWriteUInt32(a_Stream, Mobj->angle);
			D_RBSWriteInt32(a_Stream, Mobj->sprite);
			D_RBSWriteInt32(a_Stream, Mobj->frame);
			D_RBSWriteInt32(a_Stream, Mobj->skin);
			D_RBSWriteInt32(a_Stream, Mobj->sprite);
			D_RBSWriteInt32(a_Stream, Mobj->floorz);
			D_RBSWriteInt32(a_Stream, Mobj->ceilingz);
			D_RBSWriteInt32(a_Stream, Mobj->height);
			D_RBSWriteInt32(a_Stream, Mobj->radius);
			D_RBSWriteInt32(a_Stream, Mobj->momx);
			D_RBSWriteInt32(a_Stream, Mobj->momy);
			D_RBSWriteInt32(a_Stream, Mobj->momz);
			D_RBSWriteInt32(a_Stream, Mobj->type);
			D_RBSWriteInt32(a_Stream, Mobj->tics);
			D_RBSWriteInt32(a_Stream, Mobj->flags);
			D_RBSWriteInt32(a_Stream, Mobj->eflags);
			D_RBSWriteInt32(a_Stream, Mobj->flags2);
			D_RBSWriteInt32(a_Stream, Mobj->special1);
			D_RBSWriteInt32(a_Stream, Mobj->special2);
			D_RBSWriteInt32(a_Stream, Mobj->health);
			D_RBSWriteInt32(a_Stream, Mobj->movedir);
			D_RBSWriteInt32(a_Stream, Mobj->movecount);
			D_RBSWriteInt32(a_Stream, Mobj->reactiontime);
			D_RBSWriteInt32(a_Stream, Mobj->threshold);
			D_RBSWriteInt32(a_Stream, Mobj->lastlook);
			D_RBSWriteInt32(a_Stream, Mobj->friction);
			D_RBSWriteInt32(a_Stream, Mobj->movefactor);
			D_RBSWriteInt32(a_Stream, Mobj->dropped_ammo_count);
			D_RBSWriteUInt32(a_Stream, Mobj->XFlagsA);
			D_RBSWriteUInt32(a_Stream, Mobj->XFlagsB);
			D_RBSWriteUInt32(a_Stream, Mobj->XFlagsC);
			D_RBSWriteUInt32(a_Stream, Mobj->XFlagsD);
			D_RBSWriteUInt32(a_Stream, Mobj->RXAttackAttackType);
			D_RBSWriteUInt32(a_Stream, Mobj->RXShotWithWeapon);
			D_RBSWriteUInt8(a_Stream, Mobj->RemoveMo);
			D_RBSWriteUInt32(a_Stream, Mobj->RemType);
			D_RBSWriteInt32(a_Stream, Mobj->MaxZObtained);
			D_RBSWriteInt32(a_Stream, Mobj->SkinTeamColor);
			D_RBSWriteInt32(a_Stream, Mobj->NoiseThinker.Pitch);
			D_RBSWriteInt32(a_Stream, Mobj->NoiseThinker.Volume);
			
			// Map Data Related
			D_RBSWriteInt32(a_Stream, Mobj->player - players);
			D_RBSWriteUInt32(a_Stream, Mobj->subsector - subsectors);
			D_RBSWriteUInt32(a_Stream, Mobj->spawnpoint - mapthings);
			
			// Spawn Point is probably virtualzed
			if (Mobj->spawnpoint)
			{
				MapThing = Mobj->spawnpoint;
				D_RBSWriteUInt8(a_Stream, 'T');
				
				PS_SGBS_DumpMapThing(a_Stream, MapThing);
			}
			else
				D_RBSWriteUInt8(a_Stream, 'X');
			
			// Pointer Links
			D_RBSWritePointer(a_Stream, Mobj->snext);
			D_RBSWritePointer(a_Stream, Mobj->sprev);
			D_RBSWritePointer(a_Stream, Mobj->bnext);
			D_RBSWritePointer(a_Stream, Mobj->bprev);
			D_RBSWritePointer(a_Stream, Mobj->target);
			D_RBSWritePointer(a_Stream, Mobj->player);
			D_RBSWritePointer(a_Stream, Mobj->tracer);
			D_RBSWritePointer(a_Stream, Mobj->ChildFloor);
			D_RBSWritePointer(a_Stream, Mobj->touching_sectorlist);
			D_RBSWritePointer(a_Stream, Mobj->info);
			
			// Info
			D_RBSWriteString(a_Stream, (Mobj->info ? Mobj->info->RClassName : "NoRCN"));
			
			// State
			if (Mobj->state)
			{
				D_RBSWriteUInt8(a_Stream, 'S');
				D_RBSWriteUInt32(a_Stream, Mobj->state->FrameID);
				D_RBSWriteUInt32(a_Stream, Mobj->state->ObjectID);
				D_RBSWriteUInt32(a_Stream, Mobj->state->Marker);
				D_RBSWriteUInt32(a_Stream, Mobj->state->SpriteID);
				D_RBSWriteUInt32(a_Stream, Mobj->state->DehackEdID);
			}
			else
				D_RBSWriteUInt8(a_Stream, 'X');
			
			// Variable Info
				// ReMooD Extended Flags
			for (i = 0; i < NUMINFORXFIELDS; i++)
				D_RBSWriteUInt32(a_Stream, Mobj->RXFlags[i]);
			
				// Map Objects On
			for (i = 0; i < 2; i++)
			{
				D_RBSWriteUInt32(a_Stream, Mobj->MoOnCount[i]);
				for (j = 0; j < Mobj->MoOnCount[i]; j++)
					D_RBSWritePointer(a_Stream, Mobj->MoOn[i][j]);
			}
		}
		
		// T_FireFlicker(fireflicker_t* flick)
		else if (CurThinker->function.acv == T_FireFlicker)
		{
			// Thinker Header
			D_RBSWriteUInt8(a_Stream, 'F');
			D_RBSWriteUInt8(a_Stream, 'F');
			
			// Get Thinker
			FireFlicker = (fireflicker_t*)CurThinker;
			
			// Dump Info
			D_RBSWriteUInt32(a_Stream, FireFlicker->sector - sectors);
			D_RBSWriteInt32(a_Stream, FireFlicker->count);
			D_RBSWriteInt32(a_Stream, FireFlicker->maxlight);
			D_RBSWriteInt32(a_Stream, FireFlicker->minlight);
		}
		
		// T_Friction(friction_t* f)
		else if (CurThinker->function.acv == T_Friction)
		{
			// Thinker Header
			D_RBSWriteUInt8(a_Stream, 'F');
			D_RBSWriteUInt8(a_Stream, 'R');
			
			// Get Thinker
			Friction = (friction_t*)CurThinker;
			
			// Dump Info
			D_RBSWriteInt32(a_Stream, Friction->friction);
			D_RBSWriteInt32(a_Stream, Friction->movefactor);
			D_RBSWriteInt32(a_Stream, Friction->affectee);
		}
		
		// T_Glow(glow_t* g)
		else if (CurThinker->function.acv == T_Glow)
		{
			// Thinker Header
			D_RBSWriteUInt8(a_Stream, 'G');
			D_RBSWriteUInt8(a_Stream, 'L');
			
			// Get Thinker
			Glow = (glow_t*)CurThinker;
			
			// Dump Info
			D_RBSWriteUInt32(a_Stream, Glow->sector - sectors);
			D_RBSWriteInt32(a_Stream, Glow->minlight);
			D_RBSWriteInt32(a_Stream, Glow->maxlight);
			D_RBSWriteInt32(a_Stream, Glow->direction);
		}
		
		// T_LightFade(lightlevel_t* ll)
		else if (CurThinker->function.acv == T_LightFade)
		{
			// Thinker Header
			D_RBSWriteUInt8(a_Stream, 'L');
			D_RBSWriteUInt8(a_Stream, 'A');
			
			// Get Thinker
			LightFade = (lightlevel_t*)CurThinker;
			
			// Dump Info
			D_RBSWriteUInt32(a_Stream, LightFade->sector - sectors);
			D_RBSWriteInt32(a_Stream, LightFade->destlevel);
			D_RBSWriteInt32(a_Stream, LightFade->speed);
		}
		
		// T_LightFlash(lightflash_t* flash)
		else if (CurThinker->function.acv == T_LightFlash)
		{
			// Thinker Header
			D_RBSWriteUInt8(a_Stream, 'L');
			D_RBSWriteUInt8(a_Stream, 'F');
			
			// Get Thinker
			LightFlash = (lightflash_t*)CurThinker;
			
			// Dump Data
			D_RBSWriteUInt32(a_Stream, LightFlash->sector - sectors);
			D_RBSWriteInt32(a_Stream, LightFlash->count);
			D_RBSWriteInt32(a_Stream, LightFlash->maxlight);
			D_RBSWriteInt32(a_Stream, LightFlash->minlight);
			D_RBSWriteInt32(a_Stream, LightFlash->maxtime);
			D_RBSWriteInt32(a_Stream, LightFlash->mintime);
		}
		
		// T_MoveCeiling(ceiling_t* ceiling)
		else if (CurThinker->function.acv == T_MoveCeiling)
		{
			// Thinker Header
			D_RBSWriteUInt8(a_Stream, 'M');
			D_RBSWriteUInt8(a_Stream, 'C');
			
			// Get Thinker
			Ceiling = (ceiling_t*)CurThinker;
			
			// Dump Info
			D_RBSWriteUInt8(a_Stream, Ceiling->type);
			D_RBSWriteUInt8(a_Stream, Ceiling->crush);
			D_RBSWriteInt32(a_Stream, Ceiling->bottomheight);
			D_RBSWriteInt32(a_Stream, Ceiling->topheight);
			D_RBSWriteInt32(a_Stream, Ceiling->speed);
			D_RBSWriteInt32(a_Stream, Ceiling->oldspeed);
			D_RBSWriteInt32(a_Stream, Ceiling->newspecial);
			D_RBSWriteInt32(a_Stream, Ceiling->oldspecial);
			D_RBSWriteInt32(a_Stream, Ceiling->texture);
			D_RBSWriteInt32(a_Stream, Ceiling->direction);
			D_RBSWriteInt32(a_Stream, Ceiling->tag);
			D_RBSWriteInt32(a_Stream, Ceiling->olddirection);
			D_RBSWriteUInt32(a_Stream, Ceiling->sector - sectors);
			D_RBSWritePointer(a_Stream, Ceiling->list);
		}
		
		// T_MoveElevator(elevator_t* elevator)
		else if (CurThinker->function.acv == T_MoveElevator)
		{
			// Thinker Header
			D_RBSWriteUInt8(a_Stream, 'M');
			D_RBSWriteUInt8(a_Stream, 'E');
			
			// Get Thinker
			Elevator = (elevator_t*)CurThinker;
			
			// Dump Info
			D_RBSWriteUInt8(a_Stream, Elevator->type);
			D_RBSWriteUInt32(a_Stream, Elevator->sector - sectors);
			D_RBSWriteInt32(a_Stream, Elevator->direction);
			D_RBSWriteInt32(a_Stream, Elevator->floordestheight);
			D_RBSWriteInt32(a_Stream, Elevator->ceilingdestheight);
			D_RBSWriteInt32(a_Stream, Elevator->speed);
		}
		
		// T_MoveFloor(floormove_t* floor)
		else if (CurThinker->function.acv == T_MoveFloor)
		{
			// Thinker Header
			D_RBSWriteUInt8(a_Stream, 'M');
			D_RBSWriteUInt8(a_Stream, 'F');
			
			// Get Thinker
			FloorMove = (floormove_t*)CurThinker;
			
			// DumpInfo
			D_RBSWriteUInt8(a_Stream, FloorMove->type);
			D_RBSWriteUInt8(a_Stream, FloorMove->crush);
			D_RBSWriteUInt32(a_Stream, FloorMove->sector - sectors);
			D_RBSWriteInt32(a_Stream, FloorMove->direction);
			D_RBSWriteInt32(a_Stream, FloorMove->newspecial);
			D_RBSWriteInt32(a_Stream, FloorMove->oldspecial);
			D_RBSWriteInt32(a_Stream, FloorMove->texture);
			D_RBSWriteInt32(a_Stream, FloorMove->floordestheight);
			D_RBSWriteInt32(a_Stream, FloorMove->speed);
		}
		
		// T_PlatRaise(plat_t* plat)
		else if (CurThinker->function.acv == T_PlatRaise)
		{
			// Thinker Header
			D_RBSWriteUInt8(a_Stream, 'P');
			D_RBSWriteUInt8(a_Stream, 'R');
			
			// Get Thinker
			Plat = (plat_t*)CurThinker;
			
			// Dump Info
			D_RBSWriteUInt32(a_Stream, Plat->sector - sectors);
			D_RBSWriteInt32(a_Stream, Plat->speed);
			D_RBSWriteInt32(a_Stream, Plat->low);
			D_RBSWriteInt32(a_Stream, Plat->high);
			D_RBSWriteInt32(a_Stream, Plat->wait);
			D_RBSWriteInt32(a_Stream, Plat->count);
			D_RBSWriteInt32(a_Stream, Plat->tag);
			D_RBSWriteUInt8(a_Stream, Plat->status);
			D_RBSWriteUInt8(a_Stream, Plat->oldstatus);
			D_RBSWriteUInt8(a_Stream, Plat->crush);
			D_RBSWriteUInt8(a_Stream, Plat->type);
			D_RBSWritePointer(a_Stream, Plat->list);
		}
		
		// T_Pusher(pusher_t* p)
		else if (CurThinker->function.acv == T_Pusher)
		{
			// Thinker Header
			D_RBSWriteUInt8(a_Stream, 'P');
			D_RBSWriteUInt8(a_Stream, 'U');
			
			// Get Thinker
			Pusher = (pusher_t*)CurThinker;
			
			// Dump Info
			D_RBSWriteUInt8(a_Stream, Pusher->type);
			D_RBSWritePointer(a_Stream, Pusher->source);
			D_RBSWriteInt32(a_Stream, Pusher->x_mag);
			D_RBSWriteInt32(a_Stream, Pusher->y_mag);
			D_RBSWriteInt32(a_Stream, Pusher->magnitude);
			D_RBSWriteInt32(a_Stream, Pusher->radius);
			D_RBSWriteInt32(a_Stream, Pusher->x);
			D_RBSWriteInt32(a_Stream, Pusher->y);
			D_RBSWriteInt32(a_Stream, Pusher->affectee);
		}
		
		// T_Scroll(scroll_t* s)
		else if (CurThinker->function.acv == T_Scroll)
		{
			// Thinker Header
			D_RBSWriteUInt8(a_Stream, 'S');
			D_RBSWriteUInt8(a_Stream, 'C');
			
			// Get Thinker
			Scroll = (scroll_t*)CurThinker;
			
			// Dump Info
			D_RBSWriteInt32(a_Stream, Scroll->dx);
			D_RBSWriteInt32(a_Stream, Scroll->dy);
			D_RBSWriteInt32(a_Stream, Scroll->affectee);
			D_RBSWriteInt32(a_Stream, Scroll->control);
			D_RBSWriteInt32(a_Stream, Scroll->last_height);
			D_RBSWriteInt32(a_Stream, Scroll->vdx);
			D_RBSWriteInt32(a_Stream, Scroll->vdy);
			D_RBSWriteInt32(a_Stream, Scroll->accel);
			D_RBSWriteUInt8(a_Stream, Scroll->type);
		}
		
		// T_StrobeFlash(strobe_t* flash)
		else if (CurThinker->function.acv == T_StrobeFlash)
		{
			// Thinker Header
			D_RBSWriteUInt8(a_Stream, 'S');
			D_RBSWriteUInt8(a_Stream, 'F');
			
			// Get Thinker
			Strobe = (strobe_t*)CurThinker;
			
			// Dump Info
			D_RBSWriteUInt32(a_Stream, Strobe->sector - sectors);
			D_RBSWriteInt32(a_Stream, Strobe->count);
			D_RBSWriteInt32(a_Stream, Strobe->minlight);
			D_RBSWriteInt32(a_Stream, Strobe->maxlight);
			D_RBSWriteInt32(a_Stream, Strobe->darktime);
			D_RBSWriteInt32(a_Stream, Strobe->brighttime);
		}
		
		// T_VerticalDoor(vldoor_t* door)
		else if (CurThinker->function.acv == T_VerticalDoor)
		{
			// Thinker Header
			D_RBSWriteUInt8(a_Stream, 'V');
			D_RBSWriteUInt8(a_Stream, 'D');
			
			// Get Thinker
			VLDoor = (vldoor_t*)CurThinker;
			
			// Dump Info
			D_RBSWriteUInt8(a_Stream, VLDoor->type);
			D_RBSWriteUInt32(a_Stream, VLDoor->sector - sectors);
			D_RBSWriteInt32(a_Stream, VLDoor->topheight);
			D_RBSWriteInt32(a_Stream, VLDoor->speed);
			D_RBSWriteUInt8(a_Stream, VLDoor->direction);
			D_RBSWriteInt32(a_Stream, VLDoor->topwait);
			D_RBSWriteInt32(a_Stream, VLDoor->topcountdown);
			D_RBSWriteUInt32(a_Stream, VLDoor->line - lines);
		}
		
		// End
		D_RBSRecordBlock(a_Stream);
	}
}

/* P_SGBS_State() -- Game State */
void P_SGBS_State(D_RBlockStream_t* const a_Stream)
{
	size_t i;
	P_EXGSVariable_t* Vars;
	
	/* Something */
	// Begin
	D_RBSBaseBlock(a_Stream, "SGZA");
	
	// End
	D_RBSRecordBlock(a_Stream);
	
	/* Game State Info */
	// Begin
	D_RBSBaseBlock(a_Stream, "SGZS");
	
	// Write Times
	D_RBSWriteUInt32(a_Stream, gametic);
	D_RBSWriteUInt32(a_Stream, D_SyncNetMapTime());
	D_RBSWriteUInt32(a_Stream, D_SyncNetRealTime());
	
	// Write Game state
	D_RBSWriteUInt8(a_Stream, gamestate);
	D_RBSWriteUInt8(a_Stream, demorecording);
	D_RBSWriteUInt8(a_Stream, demoplayback);
	D_RBSWriteUInt8(a_Stream, multiplayer);
	
	// Write Current Level name
	D_RBSWriteString(a_Stream, g_CurrentLevelInfo->LumpName);
	
	// End
	D_RBSRecordBlock(a_Stream);
	
	/* Game Setting Variables */
	// Begin
	D_RBSBaseBlock(a_Stream, "SGZV");
	
	// Go through all variables
	for (i = 0; i < PEXGSNUMBITIDS; i++)
	{
		// Get Variable
		Vars = P_EXGSVarForBit(i);
		
		// No Var?
		if (!Vars)
			continue;
		
		// Write ID and Name
		D_RBSWriteUInt32(a_Stream, Vars->BitID);
		D_RBSWriteString(a_Stream, Vars->Name);
		
		// Write Value
		if (Vars->WasSet)
			D_RBSWriteInt32(a_Stream, Vars->ActualVal);
		else
			D_RBSWriteInt32(a_Stream, Vars->DefaultVal);
	}
	
	// End
	D_RBSRecordBlock(a_Stream);
}

/*** WRITING THE GAME STATE ***/



/*** MISC ***/

/* P_SAVE_WadState() -- Saves WAD Information */
void P_SAVE_WadState(void)
{
#if 0
	WadFile_t* WAD = NULL;
	size_t i, j;
	size_t NumWADs = W_NumWadFiles();
	char Base[8];
	char MD5Sum[16];
	
	P_CheckSizeEx(4 + 4 + ((12 + 16) * (NumWADs - 1)));	// Header + Size + Wad Stuff (8.3 + MD5Sum)
	
	/* WAD State Header */
	WriteStringN(&SaveBlock, "gWAD", 4);
	WriteUInt32(&SaveBlock, ((12 + 16) * (NumWADs - 1)));
	
	// Wad Info Loop
	for (i = 0; i < NumWADs; i++)
	{
		// Skip remood.wad
		if (i == 1)
			continue;
			
		WAD = W_GetWadForNum(i);
		
		// Print the WAD's name in snipped 8.3
		memset(Base, 0, sizeof(Base));
		FIL_ExtractFileBase(WAD->FileName, Base);
		
		// Print the info
		WriteStringN(&SaveBlock, Base, 8);
		switch (WAD->Method)
		{
			case METHOD_DEHACKED:
				WriteStringN(&SaveBlock, ".deh", 4);
				break;
			case METHOD_WAD:
				WriteStringN(&SaveBlock, ".wad", 4);
				break;
			case METHOD_RLEWAD:
				WriteStringN(&SaveBlock, ".wad", 4);
				break;
			case METHOD_PKZIP:
				WriteStringN(&SaveBlock, ".pk3", 4);
				break;
			case METHOD_SEVENZIP:
				WriteStringN(&SaveBlock, ".7z", 4);
				break;
			case METHOD_TAR:
				WriteStringN(&SaveBlock, ".tar", 4);
				break;
			default:
				WriteStringN(&SaveBlock, ".", 4);
				break;
		}
		WriteStringN(&SaveBlock, WAD->MD5Sum, 16);
	}
#endif
}

/* P_SAVE_Console() -- Saves CVARs */
void P_SAVE_Console(void)
{
#if 0
	size_t Off = 0;
	size_t TrueLen = 0;
	consvar_t* TheVars = CV_Export();
	consvar_t* cvar = NULL;
	uint8_t* T;
	int x, y;
	
	// Check size
	P_CheckSizeEx(8);
	WriteStringN(&SaveBlock, "cVAR", 4);
	Off = SaveBlock - SaveStart;
	WriteUInt32(&SaveBlock, 0);
	
	for (cvar = TheVars; cvar; cvar = cvar->next)
	{
		if ((cvar->flags & CV_NETVAR) || (cvar->flags & CV_SAVEDGAMEVAR))
		{
			// Length?
			x = strlen(cvar->name);
			y = strlen(cvar->string);
			P_CheckSizeEx(x + y + 2);
			TrueLen += x + y + 2;
			
			// Print stuff
			WriteString(&SaveBlock, cvar->name);
			WriteUInt8(&SaveBlock, 0);
			WriteString(&SaveBlock, cvar->string);
			WriteUInt8(&SaveBlock, 0);
		}
	}
	
	// Hack!
	T = SaveStart + Off;
	WriteUInt32(&T, TrueLen);
#endif
}

/* P_SAVE_LevelState() -- Saves the state of the level */
void P_SAVE_LevelState(void)
{
#if 0
	size_t Off = 0;
	size_t TrueLen = 0;
	uint8_t* T;
	
	// Check size
	P_CheckSizeEx(8);
	WriteStringN(&SaveBlock, "gLVL", 4);
	Off = SaveBlock - SaveStart;
	WriteUInt32(&SaveBlock, 0);
	
	/* Save The Map List and stuff */
	// Current Map
	if (levelmapname)
	{
		P_CheckSizeEx(16);
		TrueLen += 16;
		WriteStringN(&SaveBlock, "mCMP", 4);
		WriteUInt32(&SaveBlock, 8);
		WriteStringN(&SaveBlock, levelmapname, 8);
	}
	// The Map List
#if 0
	P_CheckSizeEx(8);
	TrueLen += 8;
	WriteStringN(&SaveBlock, "mMPL", 4);
	/* ... */
#endif
	
	// Game tic and such
	P_CheckSizeEx(12);
	TrueLen += 12;
	WriteStringN(&SaveBlock, "mGTK", 4);
	WriteUInt32(&SaveBlock, 4);
	WriteUInt32(&SaveBlock, gametic);
	
	// Game Skill
	P_CheckSizeEx(12);
	TrueLen += 12;
	WriteStringN(&SaveBlock, "mSKL", 4);
	WriteUInt32(&SaveBlock, 4);
	WriteUInt32(&SaveBlock, gameskill);
	
	// Game Episode and map
	P_CheckSizeEx(16);
	TrueLen += 16;
	WriteStringN(&SaveBlock, "mEMP", 4);
	WriteUInt32(&SaveBlock, 8);
	WriteUInt32(&SaveBlock, gameepisode);
	WriteUInt32(&SaveBlock, gamemap);
	
	// Random Number Index
	P_CheckSizeEx(9);
	TrueLen += 9;
	WriteStringN(&SaveBlock, "mRND", 4);
	WriteUInt32(&SaveBlock, 1);
	WriteUInt8(&SaveBlock, P_GetRandIndex());
	
	/* Normal Level */
	if (gamestate == GS_LEVEL)
	{
	}
	
	/* Intermission */
	else if (gamestate == GS_INTERMISSION)
	{
	}
	// Hack!
	T = SaveStart + Off;
	WriteUInt32(&T, TrueLen);
#endif
}

/*** PLAYERS ***/
void P_SAVE_Players(void)
{
#if 0
	size_t i, j;
	
	/*** LOCAL PLAYERS ***/
	// Header
	P_CheckSizeEx((0 * MAXPLAYERS) + 2 + 4);
	WriteStringN(&SaveBlock, "pLCL", 4);
	WriteUInt32(&SaveBlock, (0 * MAXPLAYERS));
	WriteUInt16(&SaveBlock, MAXPLAYERS);
	
	for (i = 0; i < MAXPLAYERS; i++)
	{
		// Misc
		WriteUInt64(&SaveBlock, players[i].mo);
		WriteUInt64(&SaveBlock, players[i].attacker);
		WriteUInt64(&SaveBlock, players[i].rain1);
		WriteUInt64(&SaveBlock, players[i].rain2);
		WriteUInt32(&SaveBlock, players[i].playerstate);
		WriteUInt8(&SaveBlock, players[i].originalweaponswitch);
		WriteUInt8(&SaveBlock, players[i].autoaim_toggle);
		WriteUInt8(&SaveBlock, players[i].attackdown);
		WriteUInt8(&SaveBlock, players[i].usedown);
		WriteUInt8(&SaveBlock, players[i].jumpdown);
		WriteUInt8(&SaveBlock, players[i].refire);
		WriteUInt8(&SaveBlock, players[i].damagecount);
		WriteUInt8(&SaveBlock, players[i].bonuscount);
		WriteUInt8(&SaveBlock, players[i].extralight);
		WriteUInt8(&SaveBlock, players[i].fixedcolormap);
		WriteUInt8(&SaveBlock, players[i].skincolor);
		WriteUInt8(&SaveBlock, players[i].skin);
		WriteUInt8(&SaveBlock, players[i].didsecret);
		
		// Stats
		WriteInt32(&SaveBlock, players[i].health);
		WriteUInt8(&SaveBlock, players[i].armorpoints);
		WriteInt32(&SaveBlock, players[i].armortype);
		WriteInt32(&SaveBlock, players[i].killcount);
		WriteInt32(&SaveBlock, players[i].itemcount);
		WriteInt32(&SaveBlock, players[i].secretcount);
		
		// Inventory
		WriteStringN(&SaveBlock, va("#%i", players[i].readyweapon), 24);
		WriteStringN(&SaveBlock, va("#%i", players[i].pendingweapon), 24);
		WriteUInt8(&SaveBlock, players[i].cards);
		WriteUInt8(&SaveBlock, players[i].backpack);
		
		WriteUInt8(&SaveBlock, 32);
		for (j = 0; j < NUMPOWERS; j++)
			WriteInt32(&SaveBlock, players[i].powers[j]);
		for (; j < 32; j++)
			WriteInt32(&SaveBlock, 0);
			
		WriteUInt8(&SaveBlock, 32);
		for (j = 0; j < NUMAMMO; j++)
			WriteInt32(&SaveBlock, players[i].ammo[j]);
		for (; j < 32; j++)
			WriteInt32(&SaveBlock, 0);
			
		WriteUInt8(&SaveBlock, 32);
		for (j = 0; j < NUMAMMO; j++)
			WriteInt32(&SaveBlock, players[i].maxammo[j]);
		for (; j < 32; j++)
			WriteInt32(&SaveBlock, 0);
			
		WriteUInt8(&SaveBlock, 32);
		for (j = 0; j < NUMWEAPONS; j++)
			WriteUInt8(&SaveBlock, players[i].weaponowned[j]);
		for (; j < 32; j++)
			WriteUInt8(&SaveBlock, 0);
			
		WriteUInt8(&SaveBlock, 32);
		for (j = 0; j < NUMWEAPONS; j++)
			WriteUInt8(&SaveBlock, players[i].favoritweapon[j]);
		for (; j < 32; j++)
			WriteUInt8(&SaveBlock, 0);
			
		// Orientation
		WriteInt32(&SaveBlock, players[i].viewz);
		WriteInt32(&SaveBlock, players[i].viewheight);
		WriteInt32(&SaveBlock, players[i].deltaviewheight);
		WriteInt32(&SaveBlock, players[i].bob);
		WriteInt32(&SaveBlock, players[i].aiming);
	}
	
	/*** NETWORK PLAYERS ***/
	
	/*** CAPTURED PROFILES ***/
#endif
}

/*** SECTORS ***/

/*** LINES ***/

/*** THINGS ***/
void P_SAVE_MapObjects(void)
{
#if 0
	size_t Off = 0;
	size_t TrueLen = 0;
	uint8_t* T;
	thinker_t* currentthinker;
	mobj_t* mobj;
	int y;
	
	// Header
	P_CheckSizeEx(8);
	WriteStringN(&SaveBlock, "mOBJ", 4);
	Off = SaveBlock - SaveStart;
	WriteUInt32(&SaveBlock, 0);
	
	/* Place down everything about every MObj */
	for (currentthinker = thinkercap.next; currentthinker != &thinkercap; currentthinker = currentthinker->next)
		// Is it a MObj?
		if (currentthinker->function.acp1 == (actionf_p1) P_MobjThinker)
		{
			mobj = (mobj_t*)currentthinker;
			
#define MOPZ(n) P_CheckSizeEx((n)); TrueLen += (n)
			
			/* Save it's information */
			// Local Pointer!
			MOPZ(8);
			WriteUInt64(&SaveBlock, mobj);	// This will be used to determine stuff when relinking, etc.
			
			// Class Information
			MOPZ(24 * 3);
			WriteStringN(&SaveBlock, MT2ReMooDClass[mobj->type], 24);	// RMOD Class Name
			WriteStringN(&SaveBlock, MT2MTString[mobj->type], 24);	// MT_ Compatible Name
			if (mobj->skin)		// Skin Name
				WriteStringN(&SaveBlock, mobj->skin, 24);
			else
				WriteStringN(&SaveBlock, "", 24);
				
			// Misc
			MOPZ(6 * 4);
			WriteInt32(&SaveBlock, mobj->health);
			WriteInt32(&SaveBlock, mobj->special1);
			WriteInt32(&SaveBlock, mobj->special2);
			WriteInt32(&SaveBlock, 0);	// Reserved for 3
			WriteInt32(&SaveBlock, 0);	// Reserved for 4
			WriteInt32(&SaveBlock, mobj->player - players);
			
			// States and sprites, etc.
			MOPZ(24 + 4 + 8);
			WriteStringN(&SaveBlock, va("#%i", mobj->state - states), 24);
			WriteStringN(&SaveBlock, sprnames[mobj->sprite], 4);
			WriteInt32(&SaveBlock, mobj->frame);
			WriteInt32(&SaveBlock, mobj->tics);
			
			// Old Flags (DEPRECATED) (3)
			MOPZ(12);
			WriteUInt32(&SaveBlock, mobj->flags);
			WriteUInt32(&SaveBlock, mobj->flags2);
			WriteUInt32(&SaveBlock, mobj->eflags);
			
			// X Flags
			MOPZ(20);
			P_MobjFlagsNaturalToExtended(mobj);
			WriteUInt32(&SaveBlock, mobj->XFlagsA);
			WriteUInt32(&SaveBlock, mobj->XFlagsB);
			WriteUInt32(&SaveBlock, mobj->XFlagsC);
			WriteUInt32(&SaveBlock, mobj->XFlagsD);
			WriteUInt32(&SaveBlock, 0);	// RESERVED FOR E
			P_MobjFlagsExtendedToNatural(mobj);
			
			// Position and Orientation (6)
			MOPZ(28);
			WriteInt32(&SaveBlock, mobj->x);
			WriteInt32(&SaveBlock, mobj->y);
			WriteInt32(&SaveBlock, mobj->z);
			WriteInt32(&SaveBlock, mobj->angle);
			WriteInt32(&SaveBlock, mobj->floorz);
			WriteInt32(&SaveBlock, mobj->ceilingz);
			WriteUInt32(&SaveBlock, mobj->subsector - subsectors);
			
			// Momentum (6)
			MOPZ(20);
			WriteInt32(&SaveBlock, mobj->momx);
			WriteInt32(&SaveBlock, mobj->momy);
			WriteInt32(&SaveBlock, mobj->momz);
			WriteInt32(&SaveBlock, mobj->friction);
			WriteInt32(&SaveBlock, mobj->movefactor);
			
			// Boundaries (2)
			MOPZ(8);
			WriteInt32(&SaveBlock, mobj->radius);
			WriteInt32(&SaveBlock, mobj->height);
			
			// AI (9)
			MOPZ(36);
			WriteUInt64(&SaveBlock, mobj->tracer);
			WriteUInt64(&SaveBlock, mobj->target);
			WriteInt32(&SaveBlock, mobj->lastlook);
			WriteInt32(&SaveBlock, mobj->threshold);
			WriteInt32(&SaveBlock, mobj->reactiontime);
			WriteInt32(&SaveBlock, mobj->movedir);
			WriteInt32(&SaveBlock, mobj->movecount);
			
			// Map Thing translation
			MOPZ(16);
			if (mobj->spawnpoint)
			{
				WriteInt32(&SaveBlock, mobj->spawnpoint - mapthings);
				WriteInt16(&SaveBlock, mobj->spawnpoint->x);
				WriteInt16(&SaveBlock, mobj->spawnpoint->y);
				WriteInt16(&SaveBlock, mobj->spawnpoint->z);
				WriteInt16(&SaveBlock, mobj->spawnpoint->angle);
				WriteInt16(&SaveBlock, mobj->spawnpoint->type);
				WriteInt16(&SaveBlock, mobj->spawnpoint->options);
			}
			else
			{
				WriteInt32(&SaveBlock, -1);
				WriteUInt64(&SaveBlock, 0);
				WriteUInt32(&SaveBlock, 0);
			}
			
			// Reserved
			MOPZ(20);
			WriteUInt32(&SaveBlock, 0);
			WriteUInt32(&SaveBlock, 0);
			WriteUInt32(&SaveBlock, 0);
			WriteUInt32(&SaveBlock, 0);
			WriteUInt32(&SaveBlock, 0);
		}
	// Hack!
	T = SaveStart + Off;
	WriteUInt32(&T, TrueLen);
#endif
}

/*** SCRIPTS ***/

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* P_SaveGame() -- Saves a game */
void P_SaveGame(void)
{
}

/* P_LoadGame() -- Loads a game */
bool_t P_LoadGame(void)
{
	return false;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#if 0

// Pads save_p to a 4-uint8_t boundary
//  so that the load/save works on SGI&Gecko.
#ifdef SGI
// BP: this stuff isn't be removed but i think it will no more work
//     anyway what processor can't read/write unaligned data ?
#define PADSAVEP()      save_p += (4 - ((int) save_p & 3)) & 3
#else
#define PADSAVEP()
#endif

// BP: damned this #if don't work ! why ?
// GhostlyDeath <April 23, 2008> -- Because #if is a preprocessor thing, NUMWEAPONS is a source thing!
// NUMWEAPONS is 0 to the compiler unless you #define NUMWEAPONS 8
#if NUMWEAPONS > 8
#error please update the player_saveflags enum
#endif

typedef enum
{
	// weapons   = 0x01ff,
	BACKPACK = 0x0200,
	ORIGNWEAP = 0x0400,
	AUTOAIM = 0x0800,
	ATTACKDWN = 0x1000,
	USEDWN = 0x2000,
	JMPDWN = 0x4000,
	DIDSECRET = 0x8000,
} player_saveflags;

typedef enum
{
	// powers      = 0x00ff
	PD_REFIRE = 0x0100,
	PD_KILLCOUNT = 0x0200,
	PD_ITEMCOUNT = 0x0400,
	PD_SECRETCOUNT = 0x0800,
	PD_DAMAGECOUNT = 0x1000,
	PD_BONUSCOUNT = 0x2000,
	PD_CHICKENTICS = 0x4000,
	PD_CHICKEPECK = 0x8000,
	PD_FLAMECOUNT = 0x10000,
	PD_FLYHEIGHT = 0x20000,
} player_diff;

//
// P_ArchivePlayers
//
void P_ArchivePlayers(void)
{
	int i, j;
	int flags;
	uint32_t diff;
	
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i])
			continue;
			
		PADSAVEP();
		
		flags = 0;
		diff = 0;
		for (j = 0; j < NUMPOWERS; j++)
			if (players[i].powers[j])
				diff |= 1 << j;
		if (players[i].refire)
			diff |= PD_REFIRE;
		if (players[i].killcount)
			diff |= PD_KILLCOUNT;
		if (players[i].itemcount)
			diff |= PD_ITEMCOUNT;
		if (players[i].secretcount)
			diff |= PD_SECRETCOUNT;
		if (players[i].damagecount)
			diff |= PD_DAMAGECOUNT;
		if (players[i].bonuscount)
			diff |= PD_BONUSCOUNT;
			
		WRITEULONG(save_p, diff);
		
		WRITEANGLE(save_p, players[i].aiming);
		WRITEUSHORT(save_p, players[i].health);
		WRITEUSHORT(save_p, players[i].armorpoints);
		WRITEBYTE(save_p, players[i].armortype);
		
		for (j = 0; j < NUMPOWERS; j++)
			if (diff & (1 << j))
				WRITELONG(save_p, players[i].powers[j]);
		WRITEBYTE(save_p, players[i].cards);
		WRITEBYTE(save_p, players[i].readyweapon);
		WRITEBYTE(save_p, players[i].pendingweapon);
		WRITEBYTE(save_p, players[i].playerstate);
		
		WRITEUSHORT(save_p, players[i].addfrags);
		for (j = 0; j < MAXPLAYERS; j++)
			if (playeringame[i])
				WRITEUSHORT(save_p, players[i].frags[j]);
				
		for (j = 0; j < NUMWEAPONS; j++)
		{
			WRITEBYTE(save_p, players[i].favoritweapon[j]);
			if (players[i].weaponowned[j])
				flags |= 1 << j;
		}
		for (j = 0; j < NUMAMMO; j++)
		{
			WRITEUSHORT(save_p, players[i].ammo[j]);
			WRITEUSHORT(save_p, players[i].maxammo[j]);
		}
		if (players[i].backpack)
			flags |= BACKPACK;
		if (players[i].originalweaponswitch)
			flags |= ORIGNWEAP;
		if (players[i].autoaim_toggle)
			flags |= AUTOAIM;
		if (players[i].attackdown)
			flags |= ATTACKDWN;
		if (players[i].usedown)
			flags |= USEDWN;
		if (players[i].jumpdown)
			flags |= JMPDWN;
		if (players[i].didsecret)
			flags |= DIDSECRET;
			
		if (diff & PD_REFIRE)
			WRITELONG(save_p, players[i].refire);
		if (diff & PD_KILLCOUNT)
			WRITELONG(save_p, players[i].killcount);
		if (diff & PD_ITEMCOUNT)
			WRITELONG(save_p, players[i].itemcount);
		if (diff & PD_SECRETCOUNT)
			WRITELONG(save_p, players[i].secretcount);
		if (diff & PD_DAMAGECOUNT)
			WRITELONG(save_p, players[i].damagecount);
		if (diff & PD_BONUSCOUNT)
			WRITELONG(save_p, players[i].bonuscount);
			
		WRITEBYTE(save_p, players[i].skincolor);
		
		for (j = 0; j < NUMPSPRITES; j++)
		{
			if (players[i].psprites[j].state)
				WRITEUSHORT(save_p, (players[i].psprites[j].state - states) + 1);
			else
				WRITEUSHORT(save_p, 0);
			WRITELONG(save_p, players[i].psprites[j].tics);
			WRITEFIXED(save_p, players[i].psprites[j].sx);
			WRITEFIXED(save_p, players[i].psprites[j].sy);
		}
		WRITEUSHORT(save_p, flags);
	}
}

//
// P_UnArchivePlayers
//
void P_UnArchivePlayers(void)
{
	int i, j;
	int flags;
	uint32_t diff;
	
	for (i = 0; i < MAXPLAYERS; i++)
	{
		memset(&players[i], 0, sizeof(player_t));
		if (!playeringame[i])
			continue;
			
		PADSAVEP();
		diff = READULONG(save_p);
		
		players[i].aiming = READANGLE(save_p);
		players[i].health = READUSHORT(save_p);
		players[i].armorpoints = READUSHORT(save_p);
		players[i].armortype = READBYTE(save_p);
		
		for (j = 0; j < NUMPOWERS; j++)
			if (diff & (1 << j))
				players[i].powers[j] = READLONG(save_p);
				
		players[i].cards = READBYTE(save_p);
		players[i].readyweapon = READBYTE(save_p);
		players[i].pendingweapon = READBYTE(save_p);
		players[i].playerstate = READBYTE(save_p);
		
		players[i].addfrags = READUSHORT(save_p);
		for (j = 0; j < MAXPLAYERS; j++)
			if (playeringame[i])
				players[i].frags[j] = READUSHORT(save_p);
				
		for (j = 0; j < NUMWEAPONS; j++)
			players[i].favoritweapon[j] = READBYTE(save_p);
		for (j = 0; j < NUMAMMO; j++)
		{
			players[i].ammo[j] = READUSHORT(save_p);
			players[i].maxammo[j] = READUSHORT(save_p);
		}
		if (diff & PD_REFIRE)
			players[i].refire = READLONG(save_p);
		if (diff & PD_KILLCOUNT)
			players[i].killcount = READLONG(save_p);
		if (diff & PD_ITEMCOUNT)
			players[i].itemcount = READLONG(save_p);
		if (diff & PD_SECRETCOUNT)
			players[i].secretcount = READLONG(save_p);
		if (diff & PD_DAMAGECOUNT)
			players[i].damagecount = READLONG(save_p);
		if (diff & PD_BONUSCOUNT)
			players[i].bonuscount = READLONG(save_p);
			
		players[i].skincolor = READBYTE(save_p);
		
		for (j = 0; j < NUMPSPRITES; j++)
		{
			flags = READUSHORT(save_p);
			if (flags)
				players[i].psprites[j].state = &states[flags - 1];
				
			players[i].psprites[j].tics = READLONG(save_p);
			players[i].psprites[j].sx = READFIXED(save_p);
			players[i].psprites[j].sy = READFIXED(save_p);
		}
		
		flags = READUSHORT(save_p);
		
		for (j = 0; j < NUMWEAPONS; j++)
			players[i].weaponowned[j] = (flags & (1 << j)) != 0;
			
		players[i].backpack = (flags & BACKPACK) != 0;
		players[i].originalweaponswitch = (flags & ORIGNWEAP) != 0;
		players[i].autoaim_toggle = (flags & AUTOAIM) != 0;
		players[i].attackdown = (flags & ATTACKDWN) != 0;
		players[i].usedown = (flags & USEDWN) != 0;
		players[i].jumpdown = (flags & JMPDWN) != 0;
		players[i].didsecret = (flags & DIDSECRET) != 0;
		
		players[i].viewheight = cv_viewheight.value << FRACBITS;
		players[i].weaponinfo = wpnlev1info;
	}
}

#define SD_FLOORHT     0x01
#define SD_CEILHT      0x02
#define SD_FLOORPIC    0x04
#define SD_CEILPIC     0x08
#define SD_LIGHT       0x10
#define SD_SPECIAL     0x20
#define SD_DIFF2       0x40

//SoM: 4/10/2000: Fix sector related savegame bugs
// diff2 flags
#define SD_FXOFFS     0x01
#define SD_FYOFFS     0x02
#define SD_CXOFFS     0x04
#define SD_CYOFFS     0x08
#define SD_STAIRLOCK  0x10
#define SD_PREVSEC    0x20
#define SD_NEXTSEC    0x40

#define LD_FLAG     0x01
#define LD_SPECIAL  0x02
//#define LD_TAG      0x04
#define LD_S1TEXOFF 0x08
#define LD_S1TOPTEX 0x10
#define LD_S1BOTTEX 0x20
#define LD_S1MIDTEX 0x40
#define LD_DIFF2    0x80

// diff2 flags
#define LD_S2TEXOFF 0x01
#define LD_S2TOPTEX 0x02
#define LD_S2BOTTEX 0x04
#define LD_S2MIDTEX 0x08

//
// P_ArchiveWorld
//
void P_ArchiveWorld(void)
{
	int i;
	int statsec = 0, statline = 0;
	line_t* li;
	side_t* si;
	uint8_t* put;
	
	// reload the map just to see difference
	mapsector_t* ms;
	mapsidedef_t* msd;
	maplinedef_t* mld;
	sector_t* ss;
	uint8_t diff;
	uint8_t diff2;
	
	ms = W_CacheLumpNum(lastloadedmaplumpnum + ML_SECTORS, PU_CACHE);
	ss = sectors;
	put = save_p;
	
	for (i = 0; i < numsectors; i++, ss++, ms++)
	{
		diff = 0;
		diff2 = 0;
		if (ss->floorheight != LittleSwapInt16(ms->floorheight) << FRACBITS)
			diff |= SD_FLOORHT;
		if (ss->ceilingheight != LittleSwapInt16(ms->ceilingheight) << FRACBITS)
			diff |= SD_CEILHT;
		//
		//  flats
		//
		// P_AddLevelFlat should not add but just return the number
		if (ss->floorpic != P_AddLevelFlat(ms->floorpic, levelflats))
			diff |= SD_FLOORPIC;
		if (ss->ceilingpic != P_AddLevelFlat(ms->ceilingpic, levelflats))
			diff |= SD_CEILPIC;
			
		if (ss->lightlevel != LittleSwapInt16(ms->lightlevel))
			diff |= SD_LIGHT;
		if (ss->special != LittleSwapInt16(ms->special))
			diff |= SD_SPECIAL;
			
		if (ss->floor_xoffs != 0)
			diff2 |= SD_FXOFFS;
		if (ss->floor_yoffs != 0)
			diff2 |= SD_FYOFFS;
		if (ss->ceiling_xoffs != 0)
			diff2 |= SD_CXOFFS;
		if (ss->ceiling_yoffs != 0)
			diff2 |= SD_CYOFFS;
		if (ss->stairlock < 0)
			diff2 |= SD_STAIRLOCK;
		if (ss->nextsec != -1)
			diff2 |= SD_NEXTSEC;
		if (ss->prevsec != -1)
			diff2 |= SD_PREVSEC;
		if (diff2)
			diff |= SD_DIFF2;
			
		if (diff)
		{
			statsec++;
			
			WRITESHORT(put, i);
			WRITEBYTE(put, diff);
			if (diff & SD_DIFF2)
				WRITEBYTE(put, diff2);
			if (diff & SD_FLOORHT)
				WRITEFIXED(put, ss->floorheight);
			if (diff & SD_CEILHT)
				WRITEFIXED(put, ss->ceilingheight);
			if (diff & SD_FLOORPIC)
			{
				memcpy(put, levelflats[ss->floorpic].name, 8);
				put += 8;
			}
			if (diff & SD_CEILPIC)
			{
				memcpy(put, levelflats[ss->ceilingpic].name, 8);
				put += 8;
			}
			if (diff & SD_LIGHT)
				WRITESHORT(put, (short)ss->lightlevel);
			if (diff & SD_SPECIAL)
				WRITESHORT(put, (short)ss->special);
				
			if (diff2 & SD_FXOFFS)
				WRITEFIXED(put, ss->floor_xoffs);
			if (diff2 & SD_FYOFFS)
				WRITEFIXED(put, ss->floor_yoffs);
			if (diff2 & SD_CXOFFS)
				WRITEFIXED(put, ss->ceiling_xoffs);
			if (diff2 & SD_CYOFFS)
				WRITEFIXED(put, ss->ceiling_yoffs);
			if (diff2 & SD_STAIRLOCK)
				WRITELONG(put, ss->stairlock);
			if (diff2 & SD_NEXTSEC)
				WRITELONG(put, ss->nextsec);
			if (diff2 & SD_PREVSEC)
				WRITELONG(put, ss->prevsec);
		}
	}
	WRITEUSHORT(put, 0xffff);
	
	mld = W_CacheLumpNum(lastloadedmaplumpnum + ML_LINEDEFS, PU_CACHE);
	msd = W_CacheLumpNum(lastloadedmaplumpnum + ML_SIDEDEFS, PU_CACHE);
	li = lines;
	// do lines
	for (i = 0; i < numlines; i++, mld++, li++)
	{
		diff = 0;
		diff2 = 0;
		
		// we don't care of map in deathmatch !
		if (((cv_deathmatch.value == 0) && (li->flags != LittleSwapInt16(mld->flags))) ||
		        ((cv_deathmatch.value != 0) && ((li->flags & ~ML_MAPPED) != LittleSwapInt16(mld->flags))))
			diff |= LD_FLAG;
		if (li->special != LittleSwapInt16(mld->special))
			diff |= LD_SPECIAL;
			
		if (li->sidenum[0] != -1)
		{
			si = &sides[li->sidenum[0]];
			if (si->textureoffset != LittleSwapInt16(msd[li->sidenum[0]].textureoffset) << FRACBITS)
				diff |= LD_S1TEXOFF;
			//SoM: 4/1/2000: Some textures are colormaps. Don't worry about invalid textures.
			if (R_CheckTextureNumForName(msd[li->sidenum[0]].toptexture) != -1)
				if (si->toptexture != R_TextureNumForName(msd[li->sidenum[0]].toptexture))
					diff |= LD_S1TOPTEX;
			if (R_CheckTextureNumForName(msd[li->sidenum[0]].bottomtexture) != -1)
				if (si->bottomtexture != R_TextureNumForName(msd[li->sidenum[0]].bottomtexture))
					diff |= LD_S1BOTTEX;
			if (R_CheckTextureNumForName(msd[li->sidenum[0]].midtexture) != -1)
				if (si->midtexture != R_TextureNumForName(msd[li->sidenum[0]].midtexture))
					diff |= LD_S1MIDTEX;
		}
		if (li->sidenum[1] != -1)
		{
			si = &sides[li->sidenum[1]];
			if (si->textureoffset != LittleSwapInt16(msd[li->sidenum[1]].textureoffset) << FRACBITS)
				diff2 |= LD_S2TEXOFF;
			if (R_CheckTextureNumForName(msd[li->sidenum[1]].toptexture) != -1)
				if (si->toptexture != R_TextureNumForName(msd[li->sidenum[1]].toptexture))
					diff2 |= LD_S2TOPTEX;
			if (R_CheckTextureNumForName(msd[li->sidenum[1]].bottomtexture) != -1)
				if (si->bottomtexture != R_TextureNumForName(msd[li->sidenum[1]].bottomtexture))
					diff2 |= LD_S2BOTTEX;
			if (R_CheckTextureNumForName(msd[li->sidenum[1]].midtexture) != -1)
				if (si->midtexture != R_TextureNumForName(msd[li->sidenum[1]].midtexture))
					diff2 |= LD_S2MIDTEX;
			if (diff2)
				diff |= LD_DIFF2;
				
		}
		
		if (diff)
		{
			statline++;
			WRITESHORT(put, (short)i);
			WRITEBYTE(put, diff);
			if (diff & LD_DIFF2)
				WRITEBYTE(put, diff2);
			if (diff & LD_FLAG)
				WRITESHORT(put, li->flags);
			if (diff & LD_SPECIAL)
				WRITESHORT(put, li->special);
				
			si = &sides[li->sidenum[0]];
			if (diff & LD_S1TEXOFF)
				WRITEFIXED(put, si->textureoffset);
			if (diff & LD_S1TOPTEX)
				WRITESHORT(put, si->toptexture);
			if (diff & LD_S1BOTTEX)
				WRITESHORT(put, si->bottomtexture);
			if (diff & LD_S1MIDTEX)
				WRITESHORT(put, si->midtexture);
				
			si = &sides[li->sidenum[1]];
			if (diff2 & LD_S2TEXOFF)
				WRITEFIXED(put, si->textureoffset);
			if (diff2 & LD_S2TOPTEX)
				WRITESHORT(put, si->toptexture);
			if (diff2 & LD_S2BOTTEX)
				WRITESHORT(put, si->bottomtexture);
			if (diff2 & LD_S2MIDTEX)
				WRITESHORT(put, si->midtexture);
		}
	}
	WRITEUSHORT(put, 0xffff);
	
	//CONL_PrintF("sector saved %d/%d, line saved %d/%d\n",statsec,numsectors,statline,numlines);
	save_p = put;
}

//
// P_UnArchiveWorld
//
void P_UnArchiveWorld(void)
{
	int i;
	line_t* li;
	side_t* si;
	uint8_t* get;
	uint8_t diff, diff2;
	
	get = save_p;
	
	for (;;)
	{
		i = READUSHORT(get);
		
		if (i == 0xffff)
			break;
			
		diff = READBYTE(get);
		if (diff & SD_DIFF2)
			diff2 = READBYTE(get);
		else
			diff2 = 0;
		if (diff & SD_FLOORHT)
			sectors[i].floorheight = READFIXED(get);
		if (diff & SD_CEILHT)
			sectors[i].ceilingheight = READFIXED(get);
		if (diff & SD_FLOORPIC)
		{
			sectors[i].floorpic = P_AddLevelFlat(get, levelflats);
			get += 8;
		}
		if (diff & SD_CEILPIC)
		{
			sectors[i].ceilingpic = P_AddLevelFlat(get, levelflats);
			get += 8;
		}
		if (diff & SD_LIGHT)
			sectors[i].lightlevel = READSHORT(get);
		if (diff & SD_SPECIAL)
			sectors[i].special = READSHORT(get);
			
		if (diff2 & SD_FXOFFS)
			sectors[i].floor_xoffs = READFIXED(get);
		if (diff2 & SD_FYOFFS)
			sectors[i].floor_yoffs = READFIXED(get);
		if (diff2 & SD_CXOFFS)
			sectors[i].ceiling_xoffs = READFIXED(get);
		if (diff2 & SD_CYOFFS)
			sectors[i].ceiling_yoffs = READFIXED(get);
		if (diff2 & SD_STAIRLOCK)
			sectors[i].stairlock = READLONG(get);
		else
			sectors[i].stairlock = 0;
		if (diff2 & SD_NEXTSEC)
			sectors[i].nextsec = READLONG(get);
		else
			sectors[i].nextsec = -1;
		if (diff2 & SD_PREVSEC)
			sectors[i].prevsec = READLONG(get);
		else
			sectors[i].prevsec = -1;
	}
	
	for (;;)
	{
		i = READUSHORT(get);
		
		if (i == 0xffff)
			break;
		diff = READBYTE(get);
		li = &lines[i];
		
		if (diff & LD_DIFF2)
			diff2 = READBYTE(get);
		else
			diff2 = 0;
		if (diff & LD_FLAG)
			li->flags = READSHORT(get);
		if (diff & LD_SPECIAL)
			li->special = READSHORT(get);
			
		si = &sides[li->sidenum[0]];
		if (diff & LD_S1TEXOFF)
			si->textureoffset = READFIXED(get);
		if (diff & LD_S1TOPTEX)
			si->toptexture = READSHORT(get);
		if (diff & LD_S1BOTTEX)
			si->bottomtexture = READSHORT(get);
		if (diff & LD_S1MIDTEX)
			si->midtexture = READSHORT(get);
			
		si = &sides[li->sidenum[1]];
		if (diff2 & LD_S2TEXOFF)
			si->textureoffset = READFIXED(get);
		if (diff2 & LD_S2TOPTEX)
			si->toptexture = READSHORT(get);
		if (diff2 & LD_S2BOTTEX)
			si->bottomtexture = READSHORT(get);
		if (diff2 & LD_S2MIDTEX)
			si->midtexture = READSHORT(get);
	}
	
	save_p = get;
}

//
// Thinkers
//

typedef enum
{
	MD_SPAWNPOINT = 0x000001,
	MD_POS = 0x000002,
	MD_TYPE = 0x000004,
// Eliminated MD_Z to prevent 3dfloor hiccups SSNTails 03-17-2002
	MD_MOM = 0x000008,
	MD_RADIUS = 0x000010,
	MD_HEIGHT = 0x000020,
	MD_FLAGS = 0x000040,
	MD_HEALTH = 0x000080,
	MD_RTIME = 0x000100,
	MD_STATE = 0x000200,
	MD_TICS = 0x000400,
	MD_SPRITE = 0x000800,
	MD_FRAME = 0x001000,
	MD_EFLAGS = 0x002000,
	MD_PLAYER = 0x004000,
	MD_MOVEDIR = 0x008000,
	MD_MOVECOUNT = 0x010000,
	MD_THRESHOLD = 0x020000,
	MD_LASTLOOK = 0x040000,
	MD_TARGET = 0x080000,
	MD_TRACER = 0x100000,
	MD_FRICTION = 0x200000,
	MD_MOVEFACTOR = 0x400000,
	MD_FLAGS2 = 0x800000,
	MD_SPECIAL1 = 0x1000000,
	MD_SPECIAL2 = 0x2000000,
	MD_AMMO = 0x4000000,
} mobj_diff_t;

enum
{
	tc_mobj,
	tc_ceiling,
	tc_door,
	tc_floor,
	tc_plat,
	tc_flash,
	tc_strobe,
	tc_glow,
	tc_fireflicker,
	tc_elevator,				//SoM: 3/15/2000: Add extra boom types.
	tc_scroll,
	tc_friction,
	tc_pusher,
	tc_end
} specials_e;

//
// P_ArchiveThinkers
//
// Things to handle:
//
// P_MobjsThinker (all mobj)
// T_MoveCeiling, (ceiling_t: sector_t * swizzle), - active list
// T_VerticalDoor, (vldoor_t: sector_t * swizzle),
// T_MoveFloor, (floormove_t: sector_t * swizzle),
// T_LightFlash, (lightflash_t: sector_t * swizzle),
// T_StrobeFlash, (strobe_t: sector_t *),
// T_Glow, (glow_t: sector_t *),
// T_PlatRaise, (plat_t: sector_t *), - active list
// BP: added missing : T_FireFlicker
//
void P_ArchiveThinkers(void)
{
	thinker_t* th;
	mobj_t* mobj;
	uint32_t diff;
	
//    int                 i; //SoM: 3/16/2000: Removed. Not used any more.

	// save off the current thinkers
	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if (th->function.acp1 == (actionf_p1) P_MobjThinker)
		{
			mobj = (mobj_t*)th;
			
			/*
			   // not a monster nor a picable item so don't save it
			   if( (((mobj->flags & (MF_COUNTKILL | MF_PICKUP | MF_SHOOTABLE )) == 0)
			   && (mobj->flags & MF_MISSILE)
			   && (mobj->info->doomednum !=-1) )
			   || (mobj->type == MT_BLOOD) )
			   continue;
			 */
			if (mobj->spawnpoint && (!(mobj->spawnpoint->options & MTF_FS_SPAWNED)) && (mobj->info->doomednum != -1))
			{
				// spawnpoint is not moddified but we must save it since it is a indentifier
				diff = MD_SPAWNPOINT;
				
				if ((mobj->x != mobj->spawnpoint->x << FRACBITS) ||
				        (mobj->y != mobj->spawnpoint->y << FRACBITS) || (mobj->angle != (unsigned)(ANG45 * (mobj->spawnpoint->angle / 45))))
					diff |= MD_POS;
				if (mobj->info->doomednum != mobj->spawnpoint->type)
					diff |= MD_TYPE;
			}
			else
			{
				// not a map spawned thing so make it from scratch
				diff = MD_POS | MD_TYPE;
			}
			
			// not the default but the most probable
			if ((mobj->momx != 0) || (mobj->momy != 0) || (mobj->momz != 0))
				diff |= MD_MOM;
			if (mobj->radius != mobj->info->radius)
				diff |= MD_RADIUS;
			if (mobj->height != mobj->info->height)
				diff |= MD_HEIGHT;
			if (mobj->flags != mobj->info->flags)
				diff |= MD_FLAGS;
			if (mobj->flags2 != mobj->info->flags2)
				diff |= MD_FLAGS2;
			if (mobj->health != mobj->info->spawnhealth)
				diff |= MD_HEALTH;
			if (mobj->reactiontime != mobj->info->reactiontime)
				diff |= MD_RTIME;
			if (mobj->state - states != mobj->info->spawnstate)
				diff |= MD_STATE;
			if (mobj->tics != mobj->state->tics)
				diff |= MD_TICS;
			if (mobj->sprite != mobj->state->sprite)
				diff |= MD_SPRITE;
			if (mobj->frame != mobj->state->frame)
				diff |= MD_FRAME;
			if (mobj->eflags)
				diff |= MD_EFLAGS;
			if (mobj->player)
				diff |= MD_PLAYER;
				
			if (mobj->movedir)
				diff |= MD_MOVEDIR;
			if (mobj->movecount)
				diff |= MD_MOVECOUNT;
			if (mobj->threshold)
				diff |= MD_THRESHOLD;
			if (mobj->lastlook != -1)
				diff |= MD_LASTLOOK;
			if (mobj->target)
				diff |= MD_TARGET;
			if (mobj->tracer)
				diff |= MD_TRACER;
			if (mobj->friction != ORIG_FRICTION)
				diff |= MD_FRICTION;
			if (mobj->movefactor != ORIG_FRICTION_FACTOR)
				diff |= MD_MOVEFACTOR;
			if (mobj->special1)
				diff |= MD_SPECIAL1;
			if (mobj->special2)
				diff |= MD_SPECIAL2;
			if (mobj->dropped_ammo_count)
				diff |= MD_AMMO;
				
			PADSAVEP();
			WRITEBYTE(save_p, tc_mobj);
			WRITEULONG(save_p, diff);
			// save pointer, at load time we will search this pointer to reinitilize pointers
			WRITEULONG(save_p, (uint32_t)mobj);
			
			WRITEFIXED(save_p, mobj->z);	// Force this so 3dfloor problems don't arise. SSNTails 03-17-2002
			WRITEFIXED(save_p, mobj->floorz);
			
			if (diff & MD_SPAWNPOINT)
				WRITESHORT(save_p, mobj->spawnpoint - mapthings);
			if (diff & MD_TYPE)
				WRITEULONG(save_p, mobj->type);
			if (diff & MD_POS)
			{
				WRITEFIXED(save_p, mobj->x);
				WRITEFIXED(save_p, mobj->y);
				WRITEANGLE(save_p, mobj->angle);
			}
			if (diff & MD_MOM)
			{
				WRITEFIXED(save_p, mobj->momx);
				WRITEFIXED(save_p, mobj->momy);
				WRITEFIXED(save_p, mobj->momz);
			}
			if (diff & MD_RADIUS)
				WRITEFIXED(save_p, mobj->radius);
			if (diff & MD_HEIGHT)
				WRITEFIXED(save_p, mobj->height);
			if (diff & MD_FLAGS)
				WRITELONG(save_p, mobj->flags);
			if (diff & MD_FLAGS2)
				WRITELONG(save_p, mobj->flags2);
			if (diff & MD_HEALTH)
				WRITELONG(save_p, mobj->health);
			if (diff & MD_RTIME)
				WRITELONG(save_p, mobj->reactiontime);
			if (diff & MD_STATE)
				WRITEUSHORT(save_p, mobj->state - states);
			if (diff & MD_TICS)
				WRITELONG(save_p, mobj->tics);
			if (diff & MD_SPRITE)
				WRITEUSHORT(save_p, mobj->sprite);
			if (diff & MD_FRAME)
				WRITEULONG(save_p, mobj->frame);
			if (diff & MD_EFLAGS)
				WRITEULONG(save_p, mobj->eflags);
			if (diff & MD_PLAYER)
				WRITEBYTE(save_p, mobj->player - players);
			if (diff & MD_MOVEDIR)
				WRITELONG(save_p, mobj->movedir);
			if (diff & MD_MOVECOUNT)
				WRITELONG(save_p, mobj->movecount);
			if (diff & MD_THRESHOLD)
				WRITELONG(save_p, mobj->threshold);
			if (diff & MD_LASTLOOK)
				WRITELONG(save_p, mobj->lastlook);
			if (diff & MD_TARGET)
				WRITEULONG(save_p, (uint32_t)mobj->target);
			if (diff & MD_TRACER)
				WRITEULONG(save_p, (uint32_t)mobj->tracer);
			if (diff & MD_FRICTION)
				WRITELONG(save_p, mobj->friction);
			if (diff & MD_MOVEFACTOR)
				WRITELONG(save_p, mobj->movefactor);
			if (diff & MD_SPECIAL1)
				WRITELONG(save_p, mobj->special1);
			if (diff & MD_SPECIAL2)
				WRITELONG(save_p, mobj->special2);
			if (diff & MD_AMMO)
				WRITELONG(save_p, mobj->dropped_ammo_count);
		}
		else if (th->function.acv == (actionf_v) NULL)
		{
			//SoM: 3/15/2000: Boom stuff...
			ceilinglist_t* cl;
			
			for (cl = activeceilings; cl; cl = cl->next)
				if (cl->ceiling == (ceiling_t*) th)
				{
					ceiling_t* ceiling;
					
					WRITEBYTE(save_p, tc_ceiling);
					PADSAVEP();
					ceiling = (ceiling_t*) save_p;
					memcpy(save_p, th, sizeof(*ceiling));
					save_p += sizeof(*ceiling);
					writelong(&ceiling->sector, ((ceiling_t*) th)->sector - sectors);
				}
				
			continue;
		}
		else if (th->function.acp1 == (actionf_p1) T_MoveCeiling)
		{
			ceiling_t* ceiling;
			
			WRITEBYTE(save_p, tc_ceiling);
			PADSAVEP();
			ceiling = (ceiling_t*) save_p;
			memcpy(save_p, th, sizeof(*ceiling));
			save_p += sizeof(*ceiling);
			writelong(&ceiling->sector, ((ceiling_t*) th)->sector - sectors);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1) T_VerticalDoor)
		{
			vldoor_t* door;
			
			WRITEBYTE(save_p, tc_door);
			PADSAVEP();
			door = (vldoor_t*) save_p;
			memcpy(save_p, th, sizeof(*door));
			save_p += sizeof(*door);
			writelong(&door->sector, ((vldoor_t*) th)->sector - sectors);
			writelong(&door->line, ((vldoor_t*) th)->line - lines);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1) T_MoveFloor)
		{
			floormove_t* floor;
			
			WRITEBYTE(save_p, tc_floor);
			PADSAVEP();
			floor = (floormove_t*) save_p;
			memcpy(save_p, th, sizeof(*floor));
			save_p += sizeof(*floor);
			writelong(&floor->sector, ((floormove_t*) th)->sector - sectors);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1) T_PlatRaise)
		{
			plat_t* plat;
			
			WRITEBYTE(save_p, tc_plat);
			PADSAVEP();
			plat = (plat_t*) save_p;
			memcpy(save_p, th, sizeof(*plat));
			save_p += sizeof(*plat);
			writelong(&plat->sector, ((plat_t*) th)->sector - sectors);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1) T_LightFlash)
		{
			lightflash_t* flash;
			
			WRITEBYTE(save_p, tc_flash);
			PADSAVEP();
			flash = (lightflash_t*) save_p;
			memcpy(save_p, th, sizeof(*flash));
			save_p += sizeof(*flash);
			writelong(&flash->sector, ((lightflash_t*) th)->sector - sectors);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1) T_StrobeFlash)
		{
			strobe_t* strobe;
			
			WRITEBYTE(save_p, tc_strobe);
			PADSAVEP();
			strobe = (strobe_t*) save_p;
			memcpy(save_p, th, sizeof(*strobe));
			save_p += sizeof(*strobe);
			writelong(&strobe->sector, ((strobe_t*) th)->sector - sectors);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1) T_Glow)
		{
			glow_t* glow;
			
			WRITEBYTE(save_p, tc_glow);
			PADSAVEP();
			glow = (glow_t*) save_p;
			memcpy(save_p, th, sizeof(*glow));
			save_p += sizeof(*glow);
			writelong(&glow->sector, ((glow_t*) th)->sector - sectors);
			continue;
		}
		else
			// BP added T_FireFlicker
			if (th->function.acp1 == (actionf_p1) T_FireFlicker)
			{
				fireflicker_t* fireflicker;
				
				WRITEBYTE(save_p, tc_fireflicker);
				PADSAVEP();
				fireflicker = (fireflicker_t*) save_p;
				memcpy(save_p, th, sizeof(*fireflicker));
				save_p += sizeof(*fireflicker);
				writelong(&fireflicker->sector, ((fireflicker_t*) th)->sector - sectors);
				continue;
			}
			else
				//SoM: 3/15/2000: Added extra Boom thinker types.
				if (th->function.acp1 == (actionf_p1) T_MoveElevator)
				{
					elevator_t* elevator;
					
					WRITEBYTE(save_p, tc_elevator);
					PADSAVEP();
					elevator = (elevator_t*) save_p;
					memcpy(save_p, th, sizeof(*elevator));
					save_p += sizeof(*elevator);
					writelong(&elevator->sector, ((elevator_t*) th)->sector - sectors);
					continue;
				}
				else if (th->function.acp1 == (actionf_p1) T_Scroll)
				{
					WRITEBYTE(save_p, tc_scroll);
					memcpy(save_p, th, sizeof(scroll_t));
					save_p += sizeof(scroll_t);
					continue;
				}
				else if (th->function.acp1 == (actionf_p1) T_Friction)
				{
					WRITEBYTE(save_p, tc_friction);
					memcpy(save_p, th, sizeof(friction_t));
					save_p += sizeof(friction_t);
					continue;
				}
				else if (th->function.acp1 == (actionf_p1) T_Pusher)
				{
					WRITEBYTE(save_p, tc_pusher);
					memcpy(save_p, th, sizeof(pusher_t));
					save_p += sizeof(pusher_t);
					continue;
				}
#ifdef PARANOIA
				else if ((int)th->function.acp1 != -1)	// wait garbage colection
					I_Error("unknow thinker type 0x%X", th->function.acp1);
#endif
					
	}
	
	WRITEBYTE(save_p, tc_end);
}

// Now save the pointers, tracer and target, but at load time we must
// relink to this, the savegame contain the old position in the pointer
// field copyed in the info field temporarely, but finaly we just search
// for to old postion and relink to
static mobj_t* FindNewPosition(void* oldposition)
{
	thinker_t* th;
	mobj_t* mobj;
	
	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		mobj = (mobj_t*)th;
		if ((void*)mobj->info == oldposition)
			return mobj;
	}
	if (devparm)
		CONL_PrintF("\2not found\n");
	DEBFILE("not found\n");
	return NULL;
}

//
// P_UnArchiveThinkers
//
void P_UnArchiveThinkers(void)
{
	thinker_t* currentthinker;
	thinker_t* next;
	mobj_t* mobj;
	uint32_t diff;
	int i;
	uint8_t tclass;
	ceiling_t* ceiling;
	vldoor_t* door;
	floormove_t* floor;
	plat_t* plat;
	lightflash_t* flash;
	strobe_t* strobe;
	glow_t* glow;
	fireflicker_t* fireflicker;
	elevator_t* elevator;		//SoM: 3/15/2000
	scroll_t* scroll;
	friction_t* friction;
	pusher_t* pusher;
	int j;
	
	// remove all the current thinkers
	currentthinker = thinkercap.next;
	while (currentthinker != &thinkercap)
	{
		next = currentthinker->next;
		
		mobj = (mobj_t*)currentthinker;
		if (currentthinker->function.acp1 == (actionf_p1) P_MobjThinker)
			// since this item isn't save don't remove it
			
			/*            if( !((((mobj->flags & (MF_COUNTKILL | MF_PICKUP | MF_SHOOTABLE )) == 0)
			   && (mobj->flags & MF_MISSILE)
			   && (mobj->info->doomednum !=-1) )
			   || (mobj->type == MT_BLOOD) ) )
			 */
			P_RemoveMobj((mobj_t*)currentthinker);
		else
			Z_Free(currentthinker);
			
		currentthinker = next;
	}
	// BP: we don't want the removed mobj come back !!!
	iquetail = iquehead = 0;
	P_InitThinkers();
	
	// read in saved thinkers
	for (;;)
	{
		tclass = READBYTE(save_p);
		if (tclass == tc_end)
			break;				// leave the while
		switch (tclass)
		{
			case tc_mobj:
				PADSAVEP();
				
				diff = READULONG(save_p);
				next = (void*)READULONG(save_p);	// &mobj in the old system
				
				mobj = Z_Malloc(sizeof(mobj_t), PU_LEVEL, NULL);
				memset(mobj, 0, sizeof(mobj_t));
				
				mobj->z = READFIXED(save_p);	// Force this so 3dfloor problems don't arise. SSNTails 03-17-2002
				mobj->floorz = READFIXED(save_p);
				
				if (diff & MD_SPAWNPOINT)
				{
					short spawnpointnum = READSHORT(save_p);
					
					mobj->spawnpoint = &mapthings[spawnpointnum];
					mapthings[spawnpointnum].mobj = mobj;
				}
				if (diff & MD_TYPE)
				{
					mobj->type = READULONG(save_p);
				}
				else			//if (diff & MD_SPAWNPOINT) //Hurdler: I think we must add that test ?
				{
					for (i = 0; i < NUMMOBJTYPES; i++)
						if (mobj->spawnpoint->type == mobjinfo[i].doomednum)
							break;
					if (i == NUMMOBJTYPES)
					{
						I_Error("Savegame corrupted\n");
					}
					mobj->type = i;
				}
				mobj->info = &mobjinfo[mobj->type];
				if (diff & MD_POS)
				{
					mobj->x = READFIXED(save_p);
					mobj->y = READFIXED(save_p);
					mobj->angle = READANGLE(save_p);
				}
				else
				{
					mobj->x = mobj->spawnpoint->x << FRACBITS;
					mobj->y = mobj->spawnpoint->y << FRACBITS;
					mobj->angle = ANG45 * (mobj->spawnpoint->angle / 45);
				}
				if (diff & MD_MOM)
				{
					mobj->momx = READFIXED(save_p);
					mobj->momy = READFIXED(save_p);
					mobj->momz = READFIXED(save_p);
				}				// else null (memset)
				
				if (diff & MD_RADIUS)
					mobj->radius = READFIXED(save_p);
				else
					mobj->radius = mobj->info->radius;
				if (diff & MD_HEIGHT)
					mobj->height = READFIXED(save_p);
				else
					mobj->height = mobj->info->height;
				if (diff & MD_FLAGS)
					mobj->flags = READLONG(save_p);
				else
					mobj->flags = mobj->info->flags;
				if (diff & MD_FLAGS2)
					mobj->flags2 = READLONG(save_p);
				else
					mobj->flags2 = mobj->info->flags2;
				if (diff & MD_HEALTH)
					mobj->health = READLONG(save_p);
				else
					mobj->health = mobj->info->spawnhealth;
				if (diff & MD_RTIME)
					mobj->reactiontime = READLONG(save_p);
				else
					mobj->reactiontime = mobj->info->reactiontime;
					
				if (diff & MD_STATE)
					mobj->state = &states[READUSHORT(save_p)];
				else
					mobj->state = &states[mobj->info->spawnstate];
				if (diff & MD_TICS)
					mobj->tics = READLONG(save_p);
				else
					mobj->tics = mobj->state->tics;
				if (diff & MD_SPRITE)
					mobj->sprite = READUSHORT(save_p);
				else
					mobj->sprite = mobj->state->sprite;
				if (diff & MD_FRAME)
					mobj->frame = READULONG(save_p);
				else
					mobj->frame = mobj->state->frame;
				if (diff & MD_EFLAGS)
					mobj->eflags = READULONG(save_p);
				if (diff & MD_PLAYER)
				{
					i = READBYTE(save_p);
					mobj->player = &players[i];
					mobj->player->mo = mobj;
					for (j = 0; j < MAXSPLITSCREENPLAYERS; j++)
						if (consoleplayer[i] == i)
							localangle[i] = mobj->angle;
				}
				if (diff & MD_MOVEDIR)
					mobj->movedir = READLONG(save_p);
				if (diff & MD_MOVECOUNT)
					mobj->movecount = READLONG(save_p);
				if (diff & MD_THRESHOLD)
					mobj->threshold = READLONG(save_p);
				if (diff & MD_LASTLOOK)
					mobj->lastlook = READLONG(save_p);
				else
					mobj->lastlook = -1;
				if (diff & MD_TARGET)
					mobj->target = (mobj_t*)READULONG(save_p);
				if (diff & MD_TRACER)
					mobj->tracer = (mobj_t*)READULONG(save_p);
				if (diff & MD_FRICTION)
					mobj->friction = READLONG(save_p);
				else
					mobj->friction = ORIG_FRICTION;
				if (diff & MD_MOVEFACTOR)
					mobj->movefactor = READLONG(save_p);
				else
					mobj->movefactor = ORIG_FRICTION_FACTOR;
				if (diff & MD_SPECIAL1)
					mobj->special1 = READLONG(save_p);
				if (diff & MD_SPECIAL2)
					mobj->special2 = READLONG(save_p);
				if (diff & MD_AMMO)
					mobj->dropped_ammo_count = READLONG(save_p);
					
				// now set deductable field
				// TODO : save this too
				mobj->skin = NULL;
				
				// set sprev, snext, bprev, bnext, subsector
				P_SetThingPosition(mobj);
				
				/*
				   mobj->floorz = mobj->subsector->sector->floorheight;
				   if( (diff & MD_Z) == 0 )
				   mobj->z = mobj->floorz;
				 */// This causes 3dfloor problems! SSNTails 03-17-2002
				if (mobj->player)
				{
					mobj->player->viewz = mobj->player->mo->z + mobj->player->viewheight;
					//CONL_PrintF("viewz = %f\n",FIXED_TO_FLOAT(mobj->player->viewz));
				}
				mobj->ceilingz = mobj->subsector->sector->ceilingheight;
				mobj->thinker.function.acp1 = (actionf_p1) P_MobjThinker;
				P_AddThinker(&mobj->thinker);
				
				mobj->info = (mobjinfo_t*) next;	// temporarely, set when leave this function
				break;
				
			case tc_ceiling:
				PADSAVEP();
				ceiling = Z_Malloc(sizeof(*ceiling), PU_LEVEL, NULL);
				memcpy(ceiling, save_p, sizeof(*ceiling));
				save_p += sizeof(*ceiling);
				ceiling->sector = &sectors[LittleSwapInt32(ceiling->sector)];
				ceiling->sector->ceilingdata = ceiling;
				
				if (ceiling->thinker.function.acp1)
					ceiling->thinker.function.acp1 = (actionf_p1) T_MoveCeiling;
					
				P_AddThinker(&ceiling->thinker);
				P_AddActiveCeiling(ceiling);
				break;
				
			case tc_door:
				PADSAVEP();
				door = Z_Malloc(sizeof(*door), PU_LEVEL, NULL);
				memcpy(door, save_p, sizeof(*door));
				save_p += sizeof(*door);
				door->sector = &sectors[LittleSwapInt32(door->sector)];
				door->sector->ceilingdata = door;
				door->line = &lines[LittleSwapInt32(door->line)];
				door->thinker.function.acp1 = (actionf_p1) T_VerticalDoor;
				P_AddThinker(&door->thinker);
				break;
				
			case tc_floor:
				PADSAVEP();
				floor = Z_Malloc(sizeof(*floor), PU_LEVEL, NULL);
				memcpy(floor, save_p, sizeof(*floor));
				save_p += sizeof(*floor);
				floor->sector = &sectors[LittleSwapInt32(floor->sector)];
				floor->sector->floordata = floor;
				floor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
				P_AddThinker(&floor->thinker);
				break;
				
			case tc_plat:
				PADSAVEP();
				plat = Z_Malloc(sizeof(*plat), PU_LEVEL, NULL);
				memcpy(plat, save_p, sizeof(*plat));
				save_p += sizeof(*plat);
				plat->sector = &sectors[LittleSwapInt32(plat->sector)];
				plat->sector->floordata = plat;
				
				if (plat->thinker.function.acp1)
					plat->thinker.function.acp1 = (actionf_p1) T_PlatRaise;
					
				P_AddThinker(&plat->thinker);
				P_AddActivePlat(plat);
				break;
				
			case tc_flash:
				PADSAVEP();
				flash = Z_Malloc(sizeof(*flash), PU_LEVEL, NULL);
				memcpy(flash, save_p, sizeof(*flash));
				save_p += sizeof(*flash);
				flash->sector = &sectors[LittleSwapInt32(flash->sector)];
				flash->thinker.function.acp1 = (actionf_p1) T_LightFlash;
				P_AddThinker(&flash->thinker);
				break;
				
			case tc_strobe:
				PADSAVEP();
				strobe = Z_Malloc(sizeof(*strobe), PU_LEVEL, NULL);
				memcpy(strobe, save_p, sizeof(*strobe));
				save_p += sizeof(*strobe);
				strobe->sector = &sectors[LittleSwapInt32(strobe->sector)];
				strobe->thinker.function.acp1 = (actionf_p1) T_StrobeFlash;
				P_AddThinker(&strobe->thinker);
				break;
				
			case tc_glow:
				PADSAVEP();
				glow = Z_Malloc(sizeof(*glow), PU_LEVEL, NULL);
				memcpy(glow, save_p, sizeof(*glow));
				save_p += sizeof(*glow);
				glow->sector = &sectors[LittleSwapInt32(glow->sector)];
				glow->thinker.function.acp1 = (actionf_p1) T_Glow;
				P_AddThinker(&glow->thinker);
				break;
				
			case tc_fireflicker:
				PADSAVEP();
				fireflicker = Z_Malloc(sizeof(*fireflicker), PU_LEVEL, NULL);
				memcpy(fireflicker, save_p, sizeof(*fireflicker));
				save_p += sizeof(*fireflicker);
				fireflicker->sector = &sectors[LittleSwapInt32(fireflicker->sector)];
				fireflicker->thinker.function.acp1 = (actionf_p1) T_FireFlicker;
				P_AddThinker(&fireflicker->thinker);
				break;
				
			case tc_elevator:
				PADSAVEP();
				elevator = Z_Malloc(sizeof(elevator_t), PU_LEVEL, NULL);
				memcpy(elevator, save_p, sizeof(elevator_t));
				save_p += sizeof(elevator_t);
				elevator->sector = &sectors[LittleSwapInt32(elevator->sector)];
				elevator->sector->floordata = elevator;	//jff 2/22/98
				elevator->sector->ceilingdata = elevator;	//jff 2/22/98
				elevator->thinker.function.acp1 = (actionf_p1) T_MoveElevator;
				P_AddThinker(&elevator->thinker);
				break;
				
			case tc_scroll:
				scroll = Z_Malloc(sizeof(scroll_t), PU_LEVEL, NULL);
				memcpy(scroll, save_p, sizeof(scroll_t));
				save_p += sizeof(scroll_t);
				scroll->thinker.function.acp1 = (actionf_p1) T_Scroll;
				P_AddThinker(&scroll->thinker);
				break;
				
			case tc_friction:
				friction = Z_Malloc(sizeof(friction_t), PU_LEVEL, NULL);
				memcpy(friction, save_p, sizeof(friction_t));
				save_p += sizeof(friction_t);
				friction->thinker.function.acp1 = (actionf_p1) T_Friction;
				P_AddThinker(&friction->thinker);
				break;
				
			case tc_pusher:
				pusher = Z_Malloc(sizeof(pusher_t), PU_LEVEL, NULL);
				memcpy(pusher, save_p, sizeof(pusher_t));
				save_p += sizeof(pusher_t);
				pusher->thinker.function.acp1 = (actionf_p1) T_Pusher;
				pusher->source = P_GetPushThing(pusher->affectee);
				P_AddThinker(&pusher->thinker);
				break;
				
			default:
				I_Error("P_UnarchiveSpecials:Unknown tclass %i " "in savegame", tclass);
		}
	}
	
	// use info field (value = oldposition) to relink mobjs
	for (currentthinker = thinkercap.next; currentthinker != &thinkercap; currentthinker = currentthinker->next)
	{
		if (currentthinker->function.acp1 == (actionf_p1) P_MobjThinker)
		{
			mobj = (mobj_t*)currentthinker;
			if (mobj->tracer)
			{
				mobj->tracer = FindNewPosition(mobj->tracer);
				if (!mobj->tracer)
					DEBFILE(va("tracer not found on %d\n", mobj->type));
			}
			if (mobj->target)
			{
				mobj->target = FindNewPosition(mobj->target);
				if (!mobj->target)
					DEBFILE(va("target not found on %d\n", mobj->target));
					
			}
		}
	}
	
}

//
// P_FinishMobjs
// SoM: Delay this until AFTER we load fragglescript because FS needs this
// data!
void P_FinishMobjs()
{
	thinker_t* currentthinker;
	mobj_t* mobj;
	
	// put info field there real value
	for (currentthinker = thinkercap.next; currentthinker != &thinkercap; currentthinker = currentthinker->next)
	{
		if (currentthinker->function.acp1 == (actionf_p1) P_MobjThinker)
		{
			mobj = (mobj_t*)currentthinker;
			mobj->info = &mobjinfo[mobj->type];
		}
	}
}

//
// P_ArchiveSpecials
//

// BP: added : itemrespawnqueue
//
void P_ArchiveSpecials(void)
{
	int i;
	
	// BP: added save itemrespawn queue for deathmatch
	i = iquetail;
	while (iquehead != i)
	{
		WRITELONG(save_p, itemrespawnque[i] - mapthings);
		WRITELONG(save_p, itemrespawntime[i]);
		i = (i + 1) & (ITEMQUESIZE - 1);
	}
	
	// end delimiter
	WRITELONG(save_p, 0xffffffff);
}

//
// P_UnArchiveSpecials
//
void P_UnArchiveSpecials(void)
{
	int i;
	
	// BP: added save itemrespawn queue for deathmatch
	iquetail = iquehead = 0;
	while ((i = READLONG(save_p)) != 0xffffffff)
	{
		itemrespawnque[iquehead] = &mapthings[i];
		itemrespawntime[iquehead++] = READLONG(save_p);
	}
}

/////////////////////////////////////////////////////////////////////////////
// BIG NOTE FROM SOM!
//
// SMMU/MBF use the CheckSaveGame function to dynamically expand the savegame
// buffer which would eliminate all limits on savegames... Could/Should we
// use this method?
/////////////////////////////////////////////////////////////////////////////

// SoM: Added FraggleScript stuff.
// Save all the neccesary FraggleScript data.
// we need to save the levelscript(all global
// variables) and the runningscripts (scripts
// currently suspended)

/***************** save the levelscript *************/
// make sure we remember all the global
// variables.

void P_ArchiveLevelScript()
{
	int num_variables = 0;
	int i;
	
	// all we really need to do is save the variables
	// count the variables first
	
	// count number of variables
	num_variables = 0;
	for (i = 0; i < VARIABLESLOTS; i++)
	{
		svariable_t* sv = levelscript.variables[i];
		
		while (sv && sv->type != svt_label)
		{
			num_variables++;
			sv = sv->next;
		}
	}
	
	//CheckSaveGame(sizeof(short));
	WRITESHORT(save_p, num_variables);	// write num_variables
	
	// go thru hash chains, store each variable
	for (i = 0; i < VARIABLESLOTS; i++)
	{
		// go thru this hashchain
		svariable_t* sv = levelscript.variables[i];
		
		// once we get to a label there can be no more actual
		// variables in the list to store
		while (sv && sv->type != svt_label)
		{
		
			//CheckSaveGame(strlen(sv->name)+10); // 10 for type and safety
			
			// write svariable: name
			
			strcpy(save_p, sv->name);
			save_p += strlen(sv->name) + 1;	// 1 extra for ending NULL
			
			// type
			*save_p++ = sv->type;	// store type;
			
			switch (sv->type)	// store depending on type
			{
				case svt_string:
					{
						//CheckSaveGame(strlen(sv->value.s)+5); // 5 for safety
						strcpy(save_p, sv->value.s);
						save_p += strlen(sv->value.s) + 1;
						break;
					}
				case svt_int:
					{
						//CheckSaveGame(sizeof(long));
						WRITELONG(save_p, sv->value.i);
						break;
					}
				case svt_mobj:
					{
						//CheckSaveGame(sizeof(long));
						WRITEULONG(save_p, sv->value.mobj);
						break;
					}
				case svt_fixed:
					{
						WRITEFIXED(save_p, sv->value.fixed);
						break;
					}
			}
			sv = sv->next;
		}
	}
}

void P_UnArchiveLevelScript()
{
	int i;
	int num_variables;
	
	// free all the variables in the current levelscript first
	
	for (i = 0; i < VARIABLESLOTS; i++)
	{
		svariable_t* sv = levelscript.variables[i];
		
		while (sv && sv->type != svt_label)
		{
			svariable_t* next = sv->next;
			
			Z_Free(sv);
			sv = next;
		}
		levelscript.variables[i] = sv;	// null or label
	}
	
	// now read the number of variables from the savegame file
	num_variables = READSHORT(save_p);
	
	for (i = 0; i < num_variables; i++)
	{
		svariable_t* sv = Z_Malloc(sizeof(svariable_t), PU_LEVEL, 0);
		int hashkey;
		
		// name
		sv->name = Z_Strdup(save_p, PU_LEVEL, 0);
		save_p += strlen(sv->name) + 1;
		
		sv->type = *save_p++;
		
		switch (sv->type)		// read depending on type
		{
			case svt_string:
				{
					sv->value.s = Z_Strdup(save_p, PU_LEVEL, 0);
					save_p += strlen(sv->value.s) + 1;
					break;
				}
			case svt_int:
				{
					sv->value.i = READLONG(save_p);
					break;
				}
			case svt_mobj:
				{
					uint32_t* long_p = (uint32_t*)save_p;
					
					sv->value.mobj = FindNewPosition((mobj_t*)long_p);
					long_p++;
					save_p = (char*)long_p;
					break;
				}
			case svt_fixed:
				{
					sv->value.fixed = READFIXED(save_p);
					break;
				}
			default:
				break;
		}
		
		// link in the new variable
		hashkey = variable_hash(sv->name);
		sv->next = levelscript.variables[hashkey];
		levelscript.variables[hashkey] = sv;
	}
	
}

/**************** save the runningscripts ***************/

extern runningscript_t runningscripts;	// t_script.c
runningscript_t* new_runningscript();	// t_script.c
void clear_runningscripts();	// t_script.c

// save a given runningscript
void P_ArchiveRunningScript(runningscript_t* rs)
{
	int i;
	int num_variables;
	
	//CheckSaveGame(sizeof(short) * 8); // room for 8 shorts
	WRITESHORT(save_p, rs->script->scriptnum);	// save scriptnum
	WRITESHORT(save_p, rs->savepoint - rs->script->data);	// offset
	WRITESHORT(save_p, rs->wait_type);
	WRITESHORT(save_p, rs->wait_data);
	
	// save pointer to trigger using prev
	WRITEULONG(save_p, (uint32_t)rs->trigger);
	
	// count number of variables
	num_variables = 0;
	for (i = 0; i < VARIABLESLOTS; i++)
	{
		svariable_t* sv = rs->variables[i];
		
		while (sv && sv->type != svt_label)
		{
			num_variables++;
			sv = sv->next;
		}
	}
	WRITESHORT(save_p, num_variables);
	
	// save num_variables
	
	// store variables
	// go thru hash chains, store each variable
	for (i = 0; i < VARIABLESLOTS; i++)
	{
		// go thru this hashchain
		svariable_t* sv = rs->variables[i];
		
		// once we get to a label there can be no more actual
		// variables in the list to store
		while (sv && sv->type != svt_label)
		{
		
			//CheckSaveGame(strlen(sv->name)+10); // 10 for type and safety
			
			// write svariable: name
			
			strcpy(save_p, sv->name);
			save_p += strlen(sv->name) + 1;	// 1 extra for ending NULL
			
			// type
			*save_p++ = sv->type;	// store type;
			
			switch (sv->type)	// store depending on type
			{
				case svt_string:
					{
						//CheckSaveGame(strlen(sv->value.s)+5); // 5 for safety
						strcpy(save_p, sv->value.s);
						save_p += strlen(sv->value.s) + 1;
						break;
					}
				case svt_int:
					{
						//CheckSaveGame(sizeof(long)+4);
						WRITELONG(save_p, sv->value.i);
						break;
					}
				case svt_mobj:
					{
						//CheckSaveGame(sizeof(long)+4);
						WRITEULONG(save_p, (uint32_t)sv->value.mobj);
						break;
					}
				case svt_fixed:
					{
						WRITEFIXED(save_p, sv->value.fixed);
						break;
					}
					// others do not appear in user scripts
					
				default:
					break;
			}
			
			sv = sv->next;
		}
	}
}

// get the next runningscript
runningscript_t* P_UnArchiveRunningScript()
{
	int i;
	int scriptnum;
	int num_variables;
	runningscript_t* rs;
	
	// create a new runningscript
	rs = new_runningscript();
	
	scriptnum = READSHORT(save_p);	// get scriptnum
	
	// levelscript?
	
	if (scriptnum == -1)
		rs->script = &levelscript;
	else
		rs->script = levelscript.children[scriptnum];
		
	// read out offset from save
	rs->savepoint = rs->script->data + READSHORT(save_p);
	rs->wait_type = READSHORT(save_p);
	rs->wait_data = READSHORT(save_p);
	
	// read out trigger thing
	rs->trigger = FindNewPosition((mobj_t*)(READULONG(save_p)));
	
	// get number of variables
	num_variables = READSHORT(save_p);
	
	// read out the variables now (fun!)
	
	// start with basic script slots/labels
	
	for (i = 0; i < VARIABLESLOTS; i++)
		rs->variables[i] = rs->script->variables[i];
		
	for (i = 0; i < num_variables; i++)
	{
		svariable_t* sv = Z_Malloc(sizeof(svariable_t), PU_LEVEL, 0);
		int hashkey;
		
		// name
		sv->name = Z_Strdup(save_p, PU_LEVEL, 0);
		save_p += strlen(sv->name) + 1;
		
		sv->type = *save_p++;
		
		switch (sv->type)		// read depending on type
		{
			case svt_string:
				{
					sv->value.s = Z_Strdup(save_p, PU_LEVEL, 0);
					save_p += strlen(sv->value.s) + 1;
					break;
				}
			case svt_int:
				{
					sv->value.i = READLONG(save_p);
					break;
				}
			case svt_mobj:
				{
					sv->value.mobj = FindNewPosition((mobj_t*)READULONG(save_p));
					break;
				}
			case svt_fixed:
				{
					sv->value.fixed = READFIXED(save_p);
					break;
				}
			default:
				break;
		}
		
		// link in the new variable
		hashkey = variable_hash(sv->name);
		sv->next = rs->variables[hashkey];
		rs->variables[hashkey] = sv;
	}
	
	return rs;
}

// archive all runningscripts in chain
void P_ArchiveRunningScripts()
{
	runningscript_t* rs;
	int num_runningscripts = 0;
	
	// count runningscripts
	for (rs = runningscripts.next; rs; rs = rs->next)
		num_runningscripts++;
		
	//CheckSaveGame(sizeof(long));
	
	// store num_runningscripts
	WRITEULONG(save_p, num_runningscripts);
	
	// now archive them
	rs = runningscripts.next;
	while (rs)
	{
		P_ArchiveRunningScript(rs);
		rs = rs->next;
	}
}

// restore all runningscripts from save_p
void P_UnArchiveRunningScripts()
{
	runningscript_t* rs;
	int num_runningscripts;
	int i;
	
	// remove all runningscripts first : may have been started
	// by levelscript on level load
	
	clear_runningscripts();
	
	// get num_runningscripts
	num_runningscripts = READULONG(save_p);
	
	for (i = 0; i < num_runningscripts; i++)
	{
		// get next runningscript
		rs = P_UnArchiveRunningScript();
		
		// hook into chain
		rs->next = runningscripts.next;
		rs->prev = &runningscripts;
		rs->prev->next = rs;
		if (rs->next)
			rs->next->prev = rs;
	}
}

void P_ArchiveScripts()
{
#ifdef FRAGGLESCRIPT
	// save levelscript
	P_ArchiveLevelScript();
	
	// save runningscripts
	P_ArchiveRunningScripts();
	
	// Archive the script camera.
	WRITELONG(save_p, (long)script_camera_on);
	WRITEULONG(save_p, (uint32_t)script_camera.mo);
	WRITEANGLE(save_p, script_camera.aiming);
	WRITEFIXED(save_p, script_camera.viewheight);
	WRITEANGLE(save_p, script_camera.startangle);
#endif
}

void P_UnArchiveScripts()
{
#ifdef FRAGGLESCRIPT
	// restore levelscript
	P_UnArchiveLevelScript();
	
	// restore runningscripts
	P_UnArchiveRunningScripts();
	
	// Unarchive the script camera
	script_camera_on = (bool_t)READLONG(save_p);
	script_camera.mo = FindNewPosition((mobj_t*)(READULONG(save_p)));
	script_camera.aiming = READANGLE(save_p);
	script_camera.viewheight = READFIXED(save_p);
	script_camera.startangle = READANGLE(save_p);
#endif
	P_FinishMobjs();
}

// =======================================================================
//          Misc
// =======================================================================
void P_ArchiveMisc()
{
	uint32_t pig = 0;
	int i;
	
	WRITEBYTE(save_p, gameskill);
	WRITEBYTE(save_p, gameepisode);
	WRITEBYTE(save_p, gamemap);
	
	for (i = 0; i < MAXPLAYERS; i++)
		pig |= (playeringame[i] != 0) << i;
		
	WRITEULONG(save_p, pig);
	
	WRITEULONG(save_p, leveltime);
	WRITEBYTE(save_p, P_GetRandIndex());
}

bool_t P_UnArchiveMisc()
{
	uint32_t pig;
	int i;
	
	gameskill = READBYTE(save_p);
	gameepisode = READBYTE(save_p);
	gamemap = READBYTE(save_p);
	
	pig = READULONG(save_p);
	
	for (i = 0; i < MAXPLAYERS; i++)
	{
		playeringame[i] = (pig & (1 << i)) != 0;
		players[i].playerstate = PST_REBORN;
	}
	
	if (!P_SetupLevel(gameepisode, gamemap, gameskill, NULL))
		return false;
		
	// get the time
	leveltime = READULONG(save_p);
	P_SetRandIndex(READBYTE(save_p));
	
	return true;
}

void P_SaveGame(void)
{
	P_ArchiveMisc();
	P_ArchivePlayers();
	P_ArchiveWorld();
	P_ArchiveThinkers();
	P_ArchiveSpecials();
	P_ArchiveScripts();
	
	WRITEBYTE(save_p, 0x1d);	// consistancy marker
}

bool_t P_LoadGame(void)
{
	if (!P_UnArchiveMisc())
		return false;
	P_UnArchivePlayers();
	P_UnArchiveWorld();
	P_UnArchiveThinkers();
	P_UnArchiveSpecials();
	P_UnArchiveScripts();
	
	return READBYTE(save_p) == 0x1d;
}

#endif
