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
// Copyright (C) 2012 GhostlyDeath <ghostlydeath@remood.org>
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
#include "m_menu.h"

/****************
*** FUNCTIONS ***
****************/

/* CLC_SaveGame() -- Saves the game */
static int CLC_SaveGame(const uint32_t a_ArgC, const char** const a_ArgV)
{
	/* Check */
	if (a_ArgC < 2)
	{
		CONL_OutputF("Usage: %s \"<filename>\"\n", a_ArgV[0]);
		return 1;
	}
	
	/* Save the game */
	if (strcasecmp(a_ArgV[0], "save") == 0)
	{
		if (P_SaveGameEx(NULL, a_ArgV[1], strlen(a_ArgV[1]), NULL, NULL))
			return 0;
	}
	
	/* Load the game */
	else if (strcasecmp(a_ArgV[0], "load") == 0)
	{
		if (P_LoadGameEx(NULL, a_ArgV[1], strlen(a_ArgV[1]), NULL, NULL))
			return 0;
	}
	
	/* Return success always */
	return 1;
}

/* P_InitSGConsole() -- Initialize save command */
void P_InitSGConsole(void)
{
	/* Add command */
	CONL_AddCommand("save", CLC_SaveGame);
	CONL_AddCommand("load", CLC_SaveGame);
}

/* P_SaveGameEx() -- Extended savegame */
bool_t P_SaveGameEx(const char* SaveName, const char* ExtFileName, size_t ExtFileNameLen, size_t* SaveLen, uint8_t** Origin)
{
	return false;
}

/* P_LoadGameEx() -- Load an extended save game */
bool_t P_LoadGameEx(const char* FileName, const char* ExtFileName, size_t ExtFileNameLen, size_t* SaveLen, uint8_t** Origin)
{
	return false;
}












































#if 0

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
	D_BS_t* BS = D_BSCreateFileStream(ExtFileName, DRBSSF_OVERWRITE);
	
	if (BS)
		OK = P_SaveGameToBS(BS, NULL);
	
	return OK;
}

/* P_LoadGameEx() -- Load an extended save game */
bool_t P_LoadGameEx(const char* FileName, const char* ExtFileName, size_t ExtFileNameLen, size_t* SaveLen, uint8_t** Origin)
{
	bool_t OK = false;
	D_BS_t* BS = D_BSCreateFileStream(ExtFileName, DRBSSF_READONLY);
	
	if (BS)
	{
		// Clear the level and set title screen (JUST IN CASE!)
		P_ExClearLevel();
		gamestate = GS_DEMOSCREEN;
		
		// Load the game
		OK = P_LoadGameFromBS(BS, NULL);
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
static void PLGS_SetRef(const uint64_t a_UniqPtr, void* const a_SetVal)
{
	size_t i;
	PLG_FutureDeref_t* Def;
	
	/* If pointer is zero, it points to nowhere */
	if (!a_UniqPtr)
		return;
	
	/* Look in dereference list */
	Def = NULL;
	// If it already is in there, add to things to deref
	for (i = 0; i < l_NumDerefs; i++)
		if (l_Derefs[i].UniqPtr == a_UniqPtr)
		{
			Def = &l_Derefs[i];
			break;
		}
	
	/* Otherwise add to the back */
	if (!Def)
	{
		Z_ResizeArray((void**)&l_Derefs, sizeof(*l_Derefs), l_NumDerefs, l_NumDerefs + 1);
		Def = &l_Derefs[l_NumDerefs++];
		Z_ChangeTag(l_Derefs, PU_SGPTRREF);
	}
	
	/* Set */
	Def->UniqPtr = a_UniqPtr;
	if (!Def->SetVal || Def->SetVal == a_SetVal)
		Def->SetVal = a_SetVal;
	else
	{
		if (devparm)
			CONL_PrintF("SAVE WARNING: Pointer already set? (%p >> %p)\n",
					Def->SetVal, a_SetVal);
	}
}

/* PLGS_DeRef() -- Dereference addition */
static void PLGS_DeRef(const uint64_t a_UniqPtr, void** const a_PtrRef)
{
	size_t i;
	PLG_FutureDeref_t* Def;
	
	/* If pointer is zero, it points to nowhere */
	if (!a_UniqPtr)
		return;
	
	/* Look in dereference list */
	Def = NULL;
	// If it already is in there, add to things to deref
	for (i = 0; i < l_NumDerefs; i++)
		if (l_Derefs[i].UniqPtr == a_UniqPtr)
		{
			Def = &l_Derefs[i];
			break;
		}
	
	/* Otherwise add to the back */
	if (!Def)
	{
		Z_ResizeArray((void**)&l_Derefs, sizeof(*l_Derefs), l_NumDerefs, l_NumDerefs + 1);
		Def = &l_Derefs[l_NumDerefs++];
		Z_ChangeTag(l_Derefs, PU_SGPTRREF);
	}
	
	/* Set */
	if (!Def->UniqPtr)
		Def->UniqPtr = a_UniqPtr;
	
	Z_ResizeArray(
			(void**)&Def->ChangePtr, sizeof(*Def->ChangePtr), 
			Def->NumChangePtr, Def->NumChangePtr + 1);
	Def->ChangePtr[Def->NumChangePtr++] = a_PtrRef;
	Z_ChangeTag(Def->ChangePtr, PU_SGPTRREF);
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
	PSTC_BOOLT,									// bool_t
	PSTC_TICT,									// tic_t
	PSTC_ANGLET,								// angle_t
	PSTC_FLOAT,									// float
	PSTC_DOUBLE,								// double
	PSTC_POINTERTO,								// void* -- Points to something
	PSTC_POINTERIS,								// void* -- Is pointed at (arrays)
	PSTC_POINTERISDIRECT,						// void* -- Is pointed at (local)
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
	PSRC_INT64,									// Int64
	PSRC_UINT8,									// UInt8
	PSRC_UINT16,								// UInt16
	PSRC_UINT32,								// UInt32
	PSRC_UINT64,								// UInt64
	PSRC_STRING,								// String
	PSRC_POINTER,								// Pointer
	
	NUMPSRCS
} P_SGBWTypeRec_t;

/*** STATICS ***/

/* PRWS_DRPointer() -- Pointer to something */
static bool_t PRWS_DRPointer(D_BS_t* const a_Stream, const bool_t a_Load, const P_SGBWTypeRec_t a_RecType, void* a_Ptr, const P_SGBWTypeC_t a_CType)
{
	uint64_t pID;
	
	/* Only accept pointer inputs/outputs */
	if (a_RecType != PSRC_POINTER)
	{
		if (devparm)
			CONL_PrintF("WARNING: RPointer not as ptr (%c vs %c)\n", a_RecType, PSRC_POINTER);
		return false;
	}
	
	/* If Saving, Dump Pointer */
	if (!a_Load)
		D_BSwp(a_Stream, *((void**)a_Ptr));
	
	/* If Loading, Get pointer and mark ref */
	else
	{
		pID = D_BSrp(a_Stream);
		PLGS_DeRef(pID, ((void**)a_Ptr));
		*((void**)a_Ptr) = NULL;	// FIXME: See if this causes problems?
	}
	
	/* Success */
	return true;
}

/* PRWS_SRPointer() -- This is a pointer (that is pointed to) */
static bool_t PRWS_SRPointer(D_BS_t* const a_Stream, const bool_t a_Load, const P_SGBWTypeRec_t a_RecType, void* a_Ptr, const P_SGBWTypeC_t a_CType)
{
	uint64_t pID;
	
	/* Only accept pointer inputs/outputs */
	if (a_RecType != PSRC_POINTER)
	{
		if (devparm)
			CONL_PrintF("WARNING: SPointer not as ptr (%c vs %c)\n", a_RecType, PSRC_POINTER);
		return false;
	}
	
	/* If Saving, Dump Pointer */
	if (!a_Load)
	{
		if (a_CType == PSTC_POINTERISDIRECT)	// Locals pointing to stuff
			D_BSwp(a_Stream, ((void**)a_Ptr));
		else
			D_BSwp(a_Stream, *((void**)a_Ptr));
	}
	
	/* If Loading, Get pointer and mark ref */
	else
	{
		pID = D_BSrp(a_Stream);
		if (a_CType == PSTC_POINTERISDIRECT)	// Locals pointing to stuff
			PLGS_SetRef(pID, ((void**)a_Ptr));
		else		// Arrays
			PLGS_SetRef(pID, *((void**)a_Ptr));
	}
	
	/* Success */
	return true;
}

// __REMOOD_PRWSBASE -- Handles basic integer types (they are all the same anyway)
#define __REMOOD_PRWSBASE(x,y) static bool_t PRWS_##y(D_BS_t* const a_Stream, const bool_t a_Load, const P_SGBWTypeRec_t a_RecType, void* a_Ptr, const P_SGBWTypeC_t a_CType)\
{\
	if (a_Load)\
		switch (a_RecType)\
		{\
			case PSRC_INT8: *((x*)a_Ptr) = D_BSri8(a_Stream); break;\
			case PSRC_INT16: *((x*)a_Ptr) = D_BSri16(a_Stream); break;\
			case PSRC_INT32: *((x*)a_Ptr) = D_BSri32(a_Stream); break;\
			case PSRC_INT64: *((x*)a_Ptr) = D_BSri64(a_Stream); break;\
			case PSRC_UINT8: *((x*)a_Ptr) = D_BSru8(a_Stream); break;\
			case PSRC_UINT16: *((x*)a_Ptr) = D_BSru16(a_Stream); break;\
			case PSRC_UINT32: *((x*)a_Ptr) = D_BSru32(a_Stream); break;\
			case PSRC_UINT64: *((x*)a_Ptr) = D_BSru64(a_Stream); break;\
			default: return false;\
		}\
	else\
		switch (a_RecType)\
		{\
			case PSRC_INT8: D_BSwi8(a_Stream, *((x*)a_Ptr)); break;\
			case PSRC_INT16: D_BSwi16(a_Stream, *((x*)a_Ptr)); break;\
			case PSRC_INT32: D_BSwi32(a_Stream, *((x*)a_Ptr)); break;\
			case PSRC_INT64: D_BSwi64(a_Stream, *((x*)a_Ptr)); break;\
			case PSRC_UINT8: D_BSwu8(a_Stream, *((x*)a_Ptr)); break;\
			case PSRC_UINT16: D_BSwu16(a_Stream, *((x*)a_Ptr)); break;\
			case PSRC_UINT32: D_BSwu32(a_Stream, *((x*)a_Ptr)); break;\
			case PSRC_UINT64: D_BSwu64(a_Stream, *((x*)a_Ptr)); break;\
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
__REMOOD_PRWSBASE(bool_t,boolt);
__REMOOD_PRWSBASE(tic_t,tict);
__REMOOD_PRWSBASE(angle_t,anglet);
__REMOOD_PRWSBASE(int8_t,int8);
__REMOOD_PRWSBASE(int16_t,int16);
__REMOOD_PRWSBASE(int32_t,int32);
__REMOOD_PRWSBASE(int64_t,int64);
__REMOOD_PRWSBASE(uint8_t,uint8);
__REMOOD_PRWSBASE(uint16_t,uint16);
__REMOOD_PRWSBASE(uint32_t,uint32);
__REMOOD_PRWSBASE(uint64_t,uint64);
__REMOOD_PRWSBASE(size_t,sizet);

#undef __REMOOD_PRWSBASE

// l_NativeData -- Native data handlers
static const struct
{
	size_t Size;								// Size of data
	bool_t (*RWFunc)(D_BS_t* const a_Stream, const bool_t a_Load, const P_SGBWTypeRec_t a_RecType, void* a_Ptr, const P_SGBWTypeC_t a_CType);
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
	{sizeof(bool_t), PRWS_boolt},				// PSTC_BOOLT
	{sizeof(tic_t), PRWS_tict},				// PSTC_TICT
	{sizeof(angle_t), PRWS_anglet},				// PSTC_ANGLET
	{sizeof(float),},							// PSTC_FLOAT
	{sizeof(double),},							// PSTC_DOUBLE
	{sizeof(void*), PRWS_DRPointer},			// PSTC_POINTERTO
	{sizeof(void*), PRWS_SRPointer},			// PSTC_POINTERIS
	{sizeof(void*), PRWS_SRPointer},			// PSTC_POINTERISDIRECT
	{sizeof(char*),},							// PSTC_STRING
	{sizeof(int8_t), PRWS_int8},								// PSTC_INT8
	{sizeof(int16_t), PRWS_int16},							// PSTC_INT16
	{sizeof(int32_t), PRWS_int32},							// PSTC_INT32
	{sizeof(int64_t), PRWS_int64},							// PSTC_INT64
	{sizeof(uint8_t), PRWS_uint8},							// PSTC_UINT8
	{sizeof(uint16_t), PRWS_uint16},							// PSTC_UINT16
	{sizeof(uint32_t), PRWS_uint32},							// PSTC_UINT32
	{sizeof(uint64_t), PRWS_uint64},							// PSTC_UINT64
	{sizeof(intptr_t),},							// PSTC_INTPTR
	{sizeof(uintptr_t),},							// PSTC_UINTPTR
	{sizeof(size_t), PRWS_sizet},								// PSTC_SIZET
	{sizeof(ssize_t),},							// PSTC_SSIZET
};

// l_RecChars -- Record characters
static const char l_RecChars[NUMPSRCS] =
{
	'c', 's', 'i', 'l', 'C', 'S', 'I', 'L', 's', 'p'
};

/*** FUNCTIONS ***/

/* P_SGBiWayReadOrWrite() -- Read or write data */
// This one should be dirty and ugly
bool_t P_SGBiWayReadOrWrite(
		D_BS_t* const a_Stream,
		const bool_t a_Load,
		void* const a_Ptr,
		const size_t a_Size,
		const P_SGBWTypeC_t a_NativeType,
		const P_SGBWTypeRec_t a_RecType,
		const char* const a_File, const int a_Line
	)
{
	uint8_t Marker;
	
	/* Read Data Marker */
	if (a_Load)
		Marker = D_BSru8(a_Stream);
	
	/* Sanity Checks */
	if (devparm)
	{
		// I/O Type mismatch
		if (a_Load)
			if (Marker != l_RecChars[a_RecType])
			{
				CONL_PrintF("WARNING: IOT Mismatch (%c vs %c) [@ %s:%i]\n",
						(unsigned int)Marker, (unsigned int)l_RecChars[a_RecType], a_File, a_Line);
				
				I_Error("IOT mismatch");
			}
		
		// Size and native size don't match and it isn't a reference to pointer
		if (a_Size != l_NativeData[a_NativeType].Size && a_NativeType != PSTC_POINTERIS && a_NativeType != PSTC_POINTERISDIRECT)
			CONL_PrintF("WARNING: NTS Mismatch (%u vs %u) [@ %s:%i]\n",
					(unsigned int)a_Size, (unsigned int)l_NativeData[a_NativeType].Size, a_File, a_Line);
	}
	
	/* Write Data Marker */
	if (!a_Load)
		D_BSwu8(a_Stream, l_RecChars[a_RecType]);
	
	/* Handle Native Read/Write */
	if (l_NativeData[a_NativeType].RWFunc)
		return l_NativeData[a_NativeType].RWFunc(a_Stream, a_Load, a_RecType, a_Ptr, a_NativeType);
	return false;
}

/* P_SGBiWayReadStr() -- Read string and Z_StrDup it */
bool_t P_SGBiWayReadStr(D_BS_t* const a_Stream, char** const a_Ptr, char* const a_Buf, const size_t a_BufSize)
{
	/* Clear */
	//memset(a_Buf, 0, sizeof(a_Buf) * a_BufSize);
	
	/* Read String */
	D_BSrs(a_Stream, a_Buf, a_BufSize - 1);
	
	/* Dupe it */
	if (a_Ptr)
	{
		if (*a_Ptr)
			Z_Free(*a_Ptr);
		*a_Ptr = Z_StrDup(a_Buf, PU_STATIC, NULL);
	}
	
	return true;
}

/* PS_SGBiWayDH() -- Debug Header */
static char* PS_SGBiWayDH(char* const a_Str, I_HostAddress_t* const a_NetAddr, I_HostAddress_t* const a_InAddr)
{
	/* Compare netaddr and inaddr */
	if (a_NetAddr && a_InAddr)
		if (!I_NetCompareHost(a_NetAddr, a_InAddr))
			return "";	// Drop header
	
	if (devparm)
		CONL_PrintF("SAVE DEBUG: Begin \"%s\"\n", a_Str);
	return a_Str;
}

void P_MobjNullThinker(mobj_t* mobj);

/* PS_IDThinkerType() -- Identifies the thinker type */
static uint8_t PS_IDThinkerType(thinker_t* const a_Thinker)
{
	/* Check */
	if (!a_Thinker)
		return '?';
	
	/* Go through each one */
	if (a_Thinker->function.acv == P_MobjNullThinker)	return 'a';
	else if (a_Thinker->function.acv == P_MobjThinker)	return 'b';
	else if (a_Thinker->function.acv == T_FireFlicker)	return 'c';
	else if (a_Thinker->function.acv == T_Friction)		return 'd';
	else if (a_Thinker->function.acv == T_Glow)			return 'e';
	else if (a_Thinker->function.acv == T_LightFade)	return 'f';
	else if (a_Thinker->function.acv == T_LightFlash)	return 'g';
	else if (a_Thinker->function.acv == T_MoveCeiling)	return 'h';
	else if (a_Thinker->function.acv == T_MoveElevator)	return 'i';
	else if (a_Thinker->function.acv == T_MoveFloor)	return 'j';
	else if (a_Thinker->function.acv == T_PlatRaise)	return 'k';
	else if (a_Thinker->function.acv == T_Pusher)		return 'l';
	else if (a_Thinker->function.acv == T_Scroll)		return 'm';
	else if (a_Thinker->function.acv == T_StrobeFlash)	return 'n';
	else if (a_Thinker->function.acv == T_VerticalDoor)	return 'o';
	
	/* Unknown */
	return '?';
}

/* PS_WRAPPED_D_BSws() -- Wrapped for MSVC */
bool_t PS_WRAPPED_D_BSws(D_BS_t* const a_Stream, const char* const a_Val)
{
	D_BSws(a_Stream, a_Val);
	return true;
}

/* P_SGBiWayBS() -- Saving function that goes both ways */
// This one should be clean and neat
bool_t P_SGBiWayBS(D_BS_t* const a_Stream, const bool_t a_Load)
{
	// __HEADER -- Determines if header matches or starts a new one
#define __HEADER(s) (a_Load ? (strcasecmp((s), PS_SGBiWayDH(Header, a_NetAddr, &InAddr)) == 0) : (D_BSBaseBlock(a_Stream, PS_SGBiWayDH(s, NULL, NULL))))
	// __REC -- Write: Continues (done with block so who cares); Read: Record block
#define __REC if (a_Load) HitBlock = true; else D_BSRecordNetBlock(a_Stream, a_NetAddr)
	// __BI -- Reads or loads data
#define __BI(x,nt,rc) P_SGBiWayReadOrWrite(a_Stream, a_Load, &x, sizeof(x), PSTC_##nt, PSRC_##rc, __FILE__, __LINE__)
	// __BISTRZ -- Z_Malloced String
#define __BISTRZ(x) (a_Load ? P_SGBiWayReadStr(a_Stream,&x,Buf,BUFSIZE) : PS_WRAPPED_D_BSws(a_Stream, x))
	// __BISTRB -- Buffered String
#define __BISTRB(buf,bs) (a_Load ? P_SGBiWayReadStr(a_Stream,NULL,buf,bs) : PS_WRAPPED_D_BSws(a_Stream, buf))
	
	//D_BSws(D_BS_t* const a_Stream, const char* const a_Val);
	//D_BSrs(D_BS_t* const a_Stream, char* const a_Out, const size_t a_OutSize);

#define BUFSIZE 512
#if 0
	bool_t Continue;
	char Header[5];
	char Buf[512];
	
	uint8_t SaveMaj, SaveMin, SaveRel, SaveLeg;
	
	ssize_t i, j, k, l;
	
	int8_t i8;
	int16_t i16;
	int32_t i32;
	uint8_t u8;
	uint16_t u16;
	uint32_t u32, u32b;
	uint64_t u64;
	tic_t tt;
	fixed_t fxt;
	void* vp;
	bool_t HitBlock;
	
	ffloor_t* FFLRover;
	msecnode_t* MSNode;
	
	thinker_t* Thinker, *OldThinker, *ThinkRove;
	mobj_t* Mobj;
	
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
			if (!(Continue = D_BSPlayBlock(a_Stream, Header)))
				break;
		
		//////////////////////////////
		// Game Version
		if (__HEADER("SGVR"))
		{
			// Legacy Version
			u8 = VERSION;
			__BI(u8,UINT8,UINT8);
			SaveLeg = u8;
			
			// ReMooD Version
			u8 = REMOOD_MAJORVERSION;
			__BI(u8,UINT8,UINT8);
			SaveMaj = u8;
			u8 = REMOOD_MINORVERSION;
			__BI(u8,UINT8,UINT8);
			SaveMin = u8;
			u8 = REMOOD_RELEASEVERSION;
			__BI(u8,UINT8,UINT8);
			SaveRel = u8;
			
			// Print Version Info
			CONL_PrintF("SAVE: Version %hhu.%hhu%c (%hhu).\n", SaveMaj, SaveMin, SaveRel, SaveLeg);
			
			// Record
			__REC;
		}
		
		// Game State -- Before Map Data Is Loaded
		if (__HEADER("SGGS"))
		{
			// doomstat.h
			__BI(gamestate,INT,UINT8);
			__BI(gameskill,INT,UINT8);
			__BI(gameepisode,UINT8,UINT8);
			__BI(gamemap,UINT8,UINT8);
			__BI(gametic,TICT,UINT32);
			__BI(paused,BOOLT,UINT8);
			__BI(multiplayer,BOOLT,UINT8);
			__BI(totalkills,INT,UINT32);
			__BI(totalitems,INT,UINT32);
			__BI(totalsecret,INT,UINT32);
			__BI(bodyqueslot,INT,UINT32);
			__BI(g_CoreGame,INT,UINT8);			// Just in case!
			__BI(g_IWADFlags,UINT32,UINT32);
			
			u32 = MAXSPLITSCREENPLAYERS;
			__BI(u32,UINT32,UINT32);
			for (i = 0; i < u32; i++)
			{
				__BI(g_Splits[i].Console,INT,UINT8);
				__BI(g_Splits[i].Display,INT,UINT8);
				
				// g_game.h
				__BI(localangle[i],ANGLET,UINT32);
				__BI(localaiming[i],INT,UINT32);
				
				// g_netcmd.h
				__BI(g_Splits[i].Active,BOOLT,UINT8);
			}
			
			u32 = BODYQUESIZE;
			__BI(u32,UINT32,UINT32);
			for (i = 0; i < u32; i++)
				__BI(bodyque[i],POINTERTO,POINTER);
			
			// d_net.h
			tt = D_SyncNetMapTime();
			__BI(tt,TICT,UINT32);
			D_SyncNetSetMapTime(tt);
			
			// m_random.h
			u8 = P_GetRandIndex();
			__BI(u8,UINT8,UINT8);
			P_SetRandIndex(u8);
			
			// g_game.h
			u32 = MAXPLAYERS;
			__BI(u32,UINT32,UINT32);
			for (i = 0; i < u32; i++)
			{
				__BISTRB(player_names[i],MAXPLAYERNAME);
				__BISTRB(team_names[i],MAXPLAYERNAME * 2);
				__BI(playeringame[i],BOOLT,UINT8);
			}
			
			__BISTRB(gamemapname,GAMEMAPNAMESIZE);
			__BI(nomonsters,BOOLT,UINT8);
			__BI(levelstarttic,TICT,UINT32);
			
			// d_netcmd.h
			__BI(g_SplitScreen,INT,INT32);
			
			// p_info.h
			memset(Buf, 0, sizeof(Buf));
			if (g_CurrentLevelInfo)
				strncpy(Buf, g_CurrentLevelInfo->LumpName, BUFSIZE - 1);
			__BISTRB(Buf,BUFSIZE);
			if (a_Load)
				g_CurrentLevelInfo = P_FindLevelByNameEx(Buf, NULL);
			
			// Record
			__REC;
		}
		
		// Players -- Players in the game
		if (__HEADER("SGPL"))
		{
			// If Loading, memset player info
			if (a_Load)
				memset(players, 0, sizeof(players));
			
			// g_game.h/d_player.h
			u32 = MAXPLAYERS;
			__BI(u32,UINT32,UINT32);
			for (i = 0; i < u32; i++)
			{
				__BI(players[i],POINTERISDIRECT,POINTER);
				
				__BI(players[i].mo,POINTERTO,POINTER);
				__BI(players[i].rain1,POINTERTO,POINTER);
				__BI(players[i].rain2,POINTERTO,POINTER);
				__BI(players[i].attacker,POINTERTO,POINTER);
				
				__BI(players[i].aiming,ANGLET,UINT32);
				
				__BI(players[i].viewz,FIXEDT,INT32);
				__BI(players[i].viewheight,FIXEDT,INT32);
				__BI(players[i].deltaviewheight,FIXEDT,INT32);
				__BI(players[i].bob,FIXEDT,INT32);
				__BI(players[i].MoveMom,FIXEDT,INT32);
				__BI(players[i].TargetViewZ,FIXEDT,INT32);
				
				__BI(players[i].playerstate,INT,UINT8);
				__BI(players[i].health,INT,INT32);
				__BI(players[i].armorpoints,INT,INT32);
				__BI(players[i].cheats,INT,INT32);
				__BI(players[i].refire,INT,INT32);
				__BI(players[i].killcount,INT,INT32);
				__BI(players[i].itemcount,INT,INT32);
				__BI(players[i].secretcount,INT,INT32);
				__BI(players[i].damagecount,INT,INT32);
				__BI(players[i].bonuscount,INT,INT32);
				__BI(players[i].specialsector,INT,INT32);
				__BI(players[i].extralight,INT,INT32);
				__BI(players[i].fixedcolormap,INT,INT32);
				__BI(players[i].skincolor,INT,INT32);
				__BI(players[i].skin,INT,INT32);
				__BI(players[i].chickenTics,INT,INT32);
				__BI(players[i].chickenPeck,INT,INT32);
				__BI(players[i].flamecount,INT,INT32);
				__BI(players[i].flyheight,INT,INT32);
				__BI(players[i].inventorySlotNum,INT,INT32);
				__BI(players[i].inv_ptr,INT,INT32);
				__BI(players[i].st_curpos,INT,INT32);
				__BI(players[i].st_inventoryTics,INT,INT32);
				__BI(players[i].flushdelay,INT,INT32);
				__BI(players[i].readyweapon,INT,INT32);
				__BI(players[i].pendingweapon,INT,INT32);
				
				__BI(players[i].backpack,BOOLT,UINT8);
				__BI(players[i].originalweaponswitch,BOOLT,UINT8);
				__BI(players[i].autoaim_toggle,BOOLT,UINT8);
				__BI(players[i].attackdown,BOOLT,UINT8);
				__BI(players[i].usedown,BOOLT,UINT8);
				__BI(players[i].jumpdown,BOOLT,UINT8);
				__BI(players[i].didsecret,BOOLT,UINT8);
				
				__BI(players[i].armortype,UINT8,UINT8);
				
				__BI(players[i].addfrags,UINT16,UINT16);
				
				__BI(players[i].cards,UINT32,UINT32);
				
				// Structures
					// Tic Command
				__BI(players[i].cmd.forwardmove,INT8,INT8);
				__BI(players[i].cmd.sidemove,INT8,INT8);
				__BI(players[i].cmd.angleturn,INT16,INT16);
				__BI(players[i].cmd.aiming,UINT16,UINT16);
				__BI(players[i].cmd.buttons,UINT16,UINT16);
				__BI(players[i].cmd.artifact,UINT8,UINT8);
				__BI(players[i].cmd.XNewWeapon,UINT8,UINT8);
				__BI(players[i].cmd.BaseAngleTurn,INT16,INT16);
				
					// Camera
				__BI(players[i].camera.chase,BOOLT,UINT8);
				__BI(players[i].camera.aiming,ANGLET,UINT32);
				__BI(players[i].camera.fixedcolormap,INT,UINT32);
				__BI(players[i].camera.viewheight,FIXEDT,INT32);
				__BI(players[i].camera.startangle,ANGLET,UINT32);
				__BI(players[i].camera.mo,POINTERTO,POINTER);
					
					// Weapon Info
				u8 = (players[i].weaponinfo == wpnlev2info ? 1 : 0);
				__BI(u8,UINT8,UINT8);
				players[i].weaponinfo = (u8 == 1 ? wpnlev2info : wpnlev1info);
				
				// Arrays
					// Health/Armor Limitations
				for (j = 0; j < 2; j++)
				{
					__BI(players[i].MaxHealth[j],INT32,INT32);
					__BI(players[i].MaxArmor[j],INT32,INT32);
				}
				
					// Fake momentum
				for (j = 0; j < 3; j++)
					__BI(players[i].FakeMom[j],FIXEDT,INT32);
				
					// Frags
				u32 = MAXPLAYERS;
				__BI(u32b,UINT32,UINT32);
				for (j = 0; j < u32b; j++)
					__BI(players[i].frags[j],UINT16,UINT16);
					
					// Weapons
				if (!players[i].weaponowned)
					players[i].weaponowned = Z_Malloc(sizeof(*players[i].weaponowned) * NUMWEAPONS, PU_STATIC, NULL);
				
				u32b = NUMWEAPONS;
				__BI(u32b,UINT32,UINT32);
				for (j = 0; j < u32b; j++)
					__BI(players[i].weaponowned[j],BOOLT,UINT8);
				
					// Ammo
				if (!players[i].ammo)
					players[i].ammo = Z_Malloc(sizeof(*players[i].ammo) * NUMAMMO, PU_STATIC, NULL);
				if (!players[i].maxammo)
					players[i].maxammo = Z_Malloc(sizeof(*players[i].maxammo) * NUMAMMO, PU_STATIC, NULL);
				u32b = NUMAMMO;
				__BI(u32b,UINT32,UINT32);
				for (j = 0; j < u32b; j++)
				{
					__BI(players[i].ammo[j],INT,INT32);
					__BI(players[i].maxammo[j],INT,INT32);
				}
				
					// Powers
				u32b = NUMPOWERS;
				__BI(u32b,UINT32,UINT32);
				for (j = 0; j < u32b; j++)
					__BI(players[i].powers[j],INT,INT32);
					
					// Inventory
				u32b = NUMINVENTORYSLOTS;
				__BI(u32b,UINT32,UINT32);
				for (j = 0; j < u32b; j++)
				{
					__BI(players[i].inventory[j].type,UINT8,UINT8);
					__BI(players[i].inventory[j].count,UINT8,UINT8);
				}
				
					// PSprites
				u32b = NUMPSPRITES;
				__BI(u32b,UINT32,UINT32);
				for (j = 0; j < u32b; j++)
				{
					u32b = 0;
					if (players[i].psprites[j].state)
						u32b = players[i].psprites[j].state->StateNum;
					else
						u32b = ~0;
						
					__BI(u32b,UINT32,UINT32);
					
					if (u32b != (~0))
						players[i].psprites[j].state = states[u32b];
					else
						u32b = NULL;
					
					__BI(players[i].psprites[j].tics,INT,INT32);
					__BI(players[i].psprites[j].sx,FIXEDT,INT32);
					__BI(players[i].psprites[j].sy,FIXEDT,INT32);
				}
				
				// Profile Information
				memset(Buf, 0, sizeof(Buf));
				if (!a_Load && players[i].ProfileEx)
					strncpy(Buf, players[i].ProfileEx->UUID, BUFSIZE - 1);
				__BISTRB(Buf,BUFSIZE);
				if (a_Load)
					players[i].ProfileEx = D_FindProfileEx(Buf);
				
				// NetPlayer Information
				memset(Buf, 0, sizeof(Buf));
				if (!a_Load && players[i].NetPlayer)
					strncpy(Buf, players[i].NetPlayer->UUID, BUFSIZE - 1);
				__BISTRB(Buf,BUFSIZE);
				if (a_Load)
					players[i].NetPlayer = D_NCSFindNetPlayer(Buf);
			}
			
			// Record
			__REC;
		}
		
		// Map Vertexes
		if (__HEADER("SGMA"))
		{
			// Dump
			u32 = numvertexes;
			__BI(u32,UINT32,UINT32);
			numvertexes = u32;
			if (a_Load)
				vertexes = Z_Malloc(sizeof(*vertexes) * numvertexes, PU_LEVEL, (void**)&vertexes);
			for (i = 0; i < u32; i++)
			{
				__BI(vertexes[i],POINTERISDIRECT,POINTER);
				__BI(vertexes[i].x,FIXEDT,INT32);
				__BI(vertexes[i].y,FIXEDT,INT32);
			}
			
			// Record
			__REC;
		}
		
		// Map Segs
		if (__HEADER("SGMB"))
		{
			// Dump
			u32 = numsegs;
			__BI(u32,UINT32,UINT32);
			numsegs = u32;
			if (a_Load)
				segs = Z_Malloc(sizeof(*segs) * numsegs, PU_LEVEL, (void**)&segs);
			for (i = 0; i < u32; i++)
			{
				__BI(segs[i],POINTERISDIRECT,POINTER);
				
				u32b = segs[i].v1 - vertexes;
				__BI(u32b,UINT32,UINT32);
				segs[i].v1 = &vertexes[u32b];
				
				u32b = segs[i].v2 - vertexes;
				__BI(u32b,UINT32,UINT32);
				segs[i].v2 = &vertexes[u32b];
				
				__BI(segs[i].side,INT,INT8);
				__BI(segs[i].offset,FIXEDT,INT32);
				__BI(segs[i].angle,ANGLET,UINT32);
				__BI(segs[i].sidedef,POINTERTO,POINTER);
				__BI(segs[i].linedef,POINTERTO,POINTER);
				__BI(segs[i].frontsector,POINTERTO,POINTER);
				__BI(segs[i].backsector,POINTERTO,POINTER);
				__BI(segs[i].lightmaps,POINTERTO,POINTER);
				for (j = 0; j < 2; j++)
					__BI(segs[i].VertexID[j],UINT32,UINT32);
				__BI(segs[i].LineID,UINT32,UINT32);
				
				fxt = FLOAT_TO_FIXED(segs[i].length);
				__BI(fxt,FIXEDT,INT32);
				segs[i].length = FIXED_TO_FLOAT(fxt);
				
				u32b = segs[i].numlights;
				__BI(u32b,UINT32,UINT32);
				segs[i].numlights = u32b;
				if (a_Load)
					segs[i].rlights = Z_Malloc(sizeof(*segs[i].rlights) * segs[i].numlights, PU_LEVEL, NULL);
				for (j = 0; j < u32b; j++)
				{
					__BI(segs[i].rlights[j].height,FIXEDT,INT32);
					__BI(segs[i].rlights[j].heightstep,FIXEDT,INT32);
					__BI(segs[i].rlights[j].botheight,FIXEDT,INT32);
					__BI(segs[i].rlights[j].botheightstep,FIXEDT,INT32);
					__BI(segs[i].rlights[j].lightlevel,INT16,INT16);
					__BI(segs[i].rlights[j].flags,INT,INT32);
					__BI(segs[i].rlights[j].lightnum,INT,INT32);
					__BI(segs[i].rlights[j].extra_colormap,POINTERTO,POINTER);
					__BI(segs[i].rlights[j].rcolormap,POINTERTO,POINTER);
				}
			}
			
			// Record
			__REC;
		}
		
		// Map Sectors
		if (__HEADER("SGMC"))
		{
			// Dump
			u32 = numsectors;
			__BI(u32,UINT32,UINT32);
			numsectors = u32;
			if (a_Load)
				sectors = Z_Malloc(sizeof(*sectors) * numsectors, PU_LEVEL, (void**)&sectors);
			for (i = 0; i < u32; i++)
			{
				// Pointer to this sector
				__BI(sectors[i],POINTERISDIRECT,POINTER);
				
				//// Pointers Needed By 3D Floors ////
				__BI(sectors[i].lightlevel,POINTERISDIRECT,POINTER);
					// Floor Relation
				__BI(sectors[i].floorheight,POINTERISDIRECT,POINTER);
				__BI(sectors[i].floorpic,POINTERISDIRECT,POINTER);
				__BI(sectors[i].floor_xoffs,POINTERISDIRECT,POINTER);
				__BI(sectors[i].floor_yoffs,POINTERISDIRECT,POINTER);
					// Ceiling Relation
				__BI(sectors[i].ceilingheight,POINTERISDIRECT,POINTER);
				__BI(sectors[i].ceilingpic,POINTERISDIRECT,POINTER);
				__BI(sectors[i].ceiling_xoffs,POINTERISDIRECT,POINTER);
				__BI(sectors[i].ceiling_yoffs,POINTERISDIRECT,POINTER);
				//////////////////////////////////////
				
				__BI(sectors[i].floorheight,FIXEDT,INT32);
				__BI(sectors[i].ceilingheight,FIXEDT,INT32);
				__BI(sectors[i].floor_xoffs,FIXEDT,INT32);
				__BI(sectors[i].floor_yoffs,FIXEDT,INT32);
				__BI(sectors[i].ceiling_xoffs,FIXEDT,INT32);
				__BI(sectors[i].ceiling_yoffs,FIXEDT,INT32);
				__BI(sectors[i].virtualFloorheight,FIXEDT,INT32);
				__BI(sectors[i].virtualCeilingheight,FIXEDT,INT32);
				for (j = 0; j < 4; j++)
					__BI(sectors[i].BBox[j],FIXEDT,INT32);
				
				__BI(sectors[i].floorpic,SHORT,INT16);
				__BI(sectors[i].ceilingpic,SHORT,INT16);
				__BI(sectors[i].lightlevel,SHORT,INT16);
				__BI(sectors[i].tag,SHORT,INT16);
				__BI(sectors[i].soundtraversed,SHORT,INT16);
				__BI(sectors[i].floortype,SHORT,INT16);
				
				__BI(sectors[i].nexttag,INT,INT32);
				__BI(sectors[i].firsttag,INT,INT32);
				__BI(sectors[i].validcount,INT,INT32);
				__BI(sectors[i].stairlock,INT,INT32);
				__BI(sectors[i].prevsec,INT,INT32);
				__BI(sectors[i].nextsec,INT,INT32);
				__BI(sectors[i].heightsec,INT,INT32);
				__BI(sectors[i].altheightsec,INT,INT32);
				__BI(sectors[i].floorlightsec,INT,INT32);
				__BI(sectors[i].ceilinglightsec,INT,INT32);
				__BI(sectors[i].teamstartsec,INT,INT32);
				__BI(sectors[i].bottommap,INT,INT32);
				__BI(sectors[i].midmap,INT,INT32);
				__BI(sectors[i].topmap,INT,INT32);
				__BI(sectors[i].validsort,INT,INT32);
				for (j = 0; j < 4; j++)
					__BI(sectors[i].blockbox[j],INT,INT32);
					
				__BI(sectors[i].moved,BOOLT,UINT8);
				__BI(sectors[i].added,BOOLT,UINT8);
				__BI(sectors[i].pseudoSector,BOOLT,UINT8);
				__BI(sectors[i].virtualFloor,BOOLT,UINT8);
				__BI(sectors[i].virtualCeiling,BOOLT,UINT8);
				
				__BI(sectors[i].special,UINT32,UINT32);
				__BI(sectors[i].oldspecial,UINT32,UINT32);
				
				__BI(sectors[i].SoundSecRef,SIZET,UINT32);
				
				__BI(sectors[i].soundtarget,POINTERTO,POINTER);
				__BI(sectors[i].floordata,POINTERTO,POINTER);
				__BI(sectors[i].ceilingdata,POINTERTO,POINTER);
				__BI(sectors[i].lightingdata,POINTERTO,POINTER);
				__BI(sectors[i].thinglist,POINTERTO,POINTER);
				__BI(sectors[i].extra_colormap,POINTERTO,POINTER);
				__BI(sectors[i].touching_thinglist,POINTERTO,POINTER);
				
				// Strings
				__BISTRZ(sectors[i].FloorTexture);
				__BISTRZ(sectors[i].CeilingTexture);
				
				// Floating Points
				fxt = FLOAT_TO_FIXED(sectors[i].lineoutLength);
				__BI(fxt,FIXEDT,INT32);
				sectors[i].lineoutLength = FIXED_TO_FLOAT(fxt);
				
				// Structures
					// Sound Originator
				__BI(sectors[i].soundorg.Flags,UINT32,UINT32);
				__BI(sectors[i].soundorg.x,FIXEDT,INT32);
				__BI(sectors[i].soundorg.y,FIXEDT,INT32);
				__BI(sectors[i].soundorg.z,FIXEDT,INT32);
				__BI(sectors[i].soundorg.momx,FIXEDT,INT32);
				__BI(sectors[i].soundorg.momy,FIXEDT,INT32);
				__BI(sectors[i].soundorg.momz,FIXEDT,INT32);
				__BI(sectors[i].soundorg.Pitch,FIXEDT,INT32);
				__BI(sectors[i].soundorg.Volume,FIXEDT,INT32);
				__BI(sectors[i].soundorg.Angle,ANGLET,UINT32);
				
				// Arrays
					// Lines that make up sector
				u32b = sectors[i].linecount;
				__BI(u32b,UINT32,UINT32);
				sectors[i].linecount = u32b;
				if (a_Load)
					sectors[i].lines =
						Z_Malloc(sizeof(*sectors[i].lines) * sectors[i].linecount, PU_LEVEL, NULL);
				for (j = 0; j < u32b; j++)
					__BI(sectors[i].lines[j],POINTERTO,POINTER);
					
					// Attached Sectors
				u32b = sectors[i].numattached;
				__BI(u32b,UINT32,UINT32);
				sectors[i].numattached = u32b;
				if (a_Load)
					sectors[i].attached =
						Z_Malloc(sizeof(*sectors[i].attached) * sectors[i].numattached,
							PU_LEVEL, NULL);
				for (j = 0; j < u32b; j++)
					__BI(sectors[i].attached[j],INT,INT32);
				
				// Light List -- Either owned by itself or a reference to another
				__BI(sectors[i].numlights,INT,INT32);
				__BI(sectors[i].LLSelf,BOOLT,UINT8);
					// Lightself is self (sector malloc)
				if (true/*sectors[i].LLSelf*/)
				{
					// Allocate
					if (a_Load)
						sectors[i].lightlist = Z_Malloc(sizeof(*sectors[i].lightlist) * sectors[i].numlights, PU_LEVEL, NULL);
					__BI((*sectors[i].lightlist),POINTERISDIRECT,POINTER);
					
					// Go through each light
					for (j = 0; j < sectors[i].numlights; j++)
					{
						// Dump light data
						__BI(sectors[i].lightlist[j].height,FIXEDT,INT32);
						__BI(sectors[i].lightlist[j].flags,INT,INT32);
						__BI(sectors[i].lightlist[j].lightlevel,POINTERTO,POINTER);
						__BI(sectors[i].lightlist[j].extra_colormap,POINTERTO,POINTER);
						__BI(sectors[i].lightlist[j].caster,POINTERTO,POINTER);
					}
				}
				
					// Owned by another sector
				else
					__BI(sectors[i].lightlist,POINTERTO,POINTER);
				
				//// 3D Floors in Sector (Exists as linked list) ////
				// This used to be a big ordeal a revision before this one.
				// However A solution I devised for a different structure works
				// perfectly here too.
				__BI(sectors[i].ffloors,POINTERTO,POINTER);
				/////////////////////////////////////////////////////
				
				// Untouched
				//linechain_t* sectorLines;
				//struct sector_s** stackList;
			}
			
			// Record
			__REC;
		}
		
		// Records all msecnode_t*, ffloor_t*
		if (__HEADER("SGMD"))
		{
			//// Fake Floors ////
			// Dump
			__BI(g_NumPFakeFloors,SIZET,UINT32);
			
			// Loading? Allocate
			if (a_Load)
				g_PFakeFloors = Z_Malloc(sizeof(*g_PFakeFloors) * g_NumPFakeFloors, PU_LEVEL, NULL);
			
			// Go through each one
			for (i = 0; i < g_NumPFakeFloors; i++)
			{
				// Get Current
				if (a_Load)		// Needs allocation
					g_PFakeFloors[i] = Z_Malloc(sizeof(*g_PFakeFloors[i]), PU_LEVEL, NULL);
				FFLRover = g_PFakeFloors[i];
				
				// Dump IsPointer of current floor
					// Don't use FFLRover because it is a local!
				__BI((*g_PFakeFloors[i]),POINTERISDIRECT,POINTER);
	
				// Dump Floor Data
				__BI(FFLRover->topheight,POINTERTO,POINTER);
				__BI(FFLRover->toppic,POINTERTO,POINTER);
				__BI(FFLRover->toplightlevel,POINTERTO,POINTER);
				__BI(FFLRover->topxoffs,POINTERTO,POINTER);
				__BI(FFLRover->topyoffs,POINTERTO,POINTER);
				__BI(FFLRover->bottomheight,POINTERTO,POINTER);
				__BI(FFLRover->bottompic,POINTERTO,POINTER);
				__BI(FFLRover->bottomxoffs,POINTERTO,POINTER);
				__BI(FFLRover->bottomyoffs,POINTERTO,POINTER);
				__BI(FFLRover->master,POINTERTO,POINTER);
				__BI(FFLRover->target,POINTERTO,POINTER);
				__BI(FFLRover->next,POINTERTO,POINTER);
				__BI(FFLRover->prev,POINTERTO,POINTER);
				__BI(FFLRover->OwnerMobj,POINTERTO,POINTER);
				__BI(FFLRover->delta,FIXEDT,INT32);
				__BI(FFLRover->secnum,INT,INT32);
				__BI(FFLRover->lastlight,INT,INT32);
				__BI(FFLRover->alpha,INT,INT32);
				__BI(FFLRover->flags,INT,UINT32);
			}
			/////////////////////
			
			//// Touching Sector Nodes ////
			// Dump
			__BI(g_NumMSecNodes,SIZET,UINT32);
			
			// Loading? Allocate
			if (a_Load)
				g_MSecNodes = Z_Malloc(sizeof(*g_MSecNodes) * g_NumMSecNodes, PU_LEVEL, NULL);
			
			// Go through each one
			for (i = 0; i < g_NumMSecNodes; i++)
			{
				// Get Current
				if (a_Load)		// Needs allocation
					g_MSecNodes[i] = Z_Malloc(sizeof(*g_MSecNodes[i]), PU_LEVEL, NULL);
				MSNode = g_MSecNodes[i];
				
				// Dump pointer to this node
					// Don't use MSNode because that is a local
				__BI((*g_MSecNodes[i]),POINTERISDIRECT,POINTER);
				
				// Dump information (mostly pointers)
				__BI(MSNode->m_sector,POINTERTO,POINTER);
				__BI(MSNode->m_thing,POINTERTO,POINTER);
				__BI(MSNode->m_tprev,POINTERTO,POINTER);
				__BI(MSNode->m_tnext,POINTERTO,POINTER);
				__BI(MSNode->m_sprev,POINTERTO,POINTER);
				__BI(MSNode->m_snext,POINTERTO,POINTER);
				__BI(MSNode->visited,BOOLT,UINT8);
			}
			///////////////////////////////
			
			// Record
			__REC;
		}
		
		// Map SubSectors (subsector_t)
		if (__HEADER("SGME"))
		{
			// Dump
			u32 = numsubsectors;
			__BI(u32,UINT32,UINT32);
			numsubsectors = u32;
			if (a_Load)
				subsectors = Z_Malloc(sizeof(*subsectors) * numsubsectors, PU_LEVEL, (void**)&subsectors);
			for (i = 0; i < u32; i++)
			{
				// Self
				__BI(subsectors[i],POINTERISDIRECT,POINTER);
				
				// Dump
				__BI(subsectors[i].sector,POINTERTO,POINTER);
				__BI(subsectors[i].splats,POINTERTO,POINTER);
				__BI(subsectors[i].numlines,SHORT,INT16);
				__BI(subsectors[i].firstline,SHORT,INT16);
				__BI(subsectors[i].validcount,INT,INT32);
			}
			
			// Record
			__REC;
		}
		
		// Map Nodes (node_t)
		if (__HEADER("SGMF"))
		{
			// Dump
			u32 = numnodes;
			__BI(u32,UINT32,UINT32);
			numnodes = u32;
			if (a_Load)
				nodes = Z_Malloc(sizeof(*nodes) * numnodes, PU_LEVEL, (void**)&nodes);
			for (i = 0; i < u32; i++)
			{
				// Self
				__BI(nodes[i],POINTERISDIRECT,POINTER);
				
				// Dump
				__BI(nodes[i].x,FIXEDT,INT32);
				__BI(nodes[i].y,FIXEDT,INT32);
				__BI(nodes[i].dx,FIXEDT,INT32);
				__BI(nodes[i].dy,FIXEDT,INT32);
				
				for (j = 0; j < 2; j++)
				{
					for (k = 0; k < 4; k++)
						__BI(nodes[i].bbox[j][k],FIXEDT,INT32);
					__BI(nodes[i].children[j],USHORT,UINT16);
				}
			}
			
			// Record
			__REC;
		}
		
		// Map Lines (line_t)
		if (__HEADER("SGMG"))
		{
			// Dump
			u32 = numlines;
			__BI(u32,UINT32,UINT32);
			numlines = u32;
			if (a_Load)
				lines = Z_Malloc(sizeof(*lines) * numlines, PU_LEVEL, (void**)&lines);
			for (i = 0; i < u32; i++)
			{
				// Self
				__BI(lines[i],POINTERISDIRECT,POINTER);
				
				// Dump
				u32b = lines[i].v1 - vertexes;
				__BI(u32b,UINT32,UINT32);
				lines[i].v1 = &vertexes[u32b];
				
				u32b = lines[i].v2 - vertexes;
				__BI(u32b,UINT32,UINT32);
				lines[i].v2 = &vertexes[u32b];
				
				__BI(lines[i].frontsector,POINTERTO,POINTER);
				__BI(lines[i].backsector,POINTERTO,POINTER);
				__BI(lines[i].specialdata,POINTERTO,POINTER);
				__BI(lines[i].splats,POINTERTO,POINTER);
				__BI(lines[i].dx,FIXEDT,INT32);
				__BI(lines[i].dy,FIXEDT,INT32);
				__BI(lines[i].flags,SHORT,INT16);
				__BI(lines[i].special,UINT32,UINT32);
				__BI(lines[i].tag,SHORT,INT16);
				__BI(lines[i].slopetype,INT,UINT8);
				__BI(lines[i].validcount,INT,INT32);
				__BI(lines[i].tranlump,INT,INT32);
				__BI(lines[i].firsttag,INT,INT32);
				__BI(lines[i].nexttag,INT,INT32);
				__BI(lines[i].ecolormap,INT,INT32);
				__BI(lines[i].HexenSpecial,UINT8,UINT8);
				
				for (j = 0; j < 2; j++)
				{
					__BI(lines[i].sidenum[j],SHORT,INT16);
					__BI(lines[i].VertexNum[j],SIZET,UINT32);
				}
					
				for (j = 0; j < 4; j++)
					__BI(lines[i].bbox[j],FIXEDT,INT32);
				
				for (j = 0; j < 5; j++)
					__BI(lines[i].ACSArgs[j],UINT8,UINT8);
			}
			
			// Record
			__REC;
		}
		
		// Map Sides (side_t)
		if (__HEADER("SGMH"))
		{
			// Dump
			u32 = numsides;
			__BI(u32,UINT32,UINT32);
			numsides = u32;
			if (a_Load)
				sides = Z_Malloc(sizeof(*sides) * numsides, PU_LEVEL, (void**)&sides);
			for (i = 0; i < u32; i++)
			{
				// Locator Pointer
				__BI(sides[i],POINTERISDIRECT,POINTER);
				
				// Dump
				__BI(sides[i].textureoffset,FIXEDT,INT32);
				__BI(sides[i].rowoffset,FIXEDT,INT32);
				__BI(sides[i].ScaleX,FIXEDT,INT32);
				__BI(sides[i].ScaleY,FIXEDT,INT32);
				__BI(sides[i].toptexture,SHORT,INT16);
				__BI(sides[i].bottomtexture,SHORT,INT16);
				__BI(sides[i].midtexture,SHORT,INT16);
				__BI(sides[i].sector,POINTERTO,POINTER);
				__BI(sides[i].VFlip,BOOLT,UINT8);
				__BI(sides[i].special,UINT32,UINT32);
				__BI(sides[i].SectorNum,SIZET,UINT32);
				
				for (j = 0; j < 3; j++)
					__BISTRZ(sides[i].WallTextures[j]);
			}
			
			// Record
			__REC;
		}
		
		// Map Things (mapthing_t)
		if (__HEADER("SGMI"))
		{
			// Dump
			u32 = nummapthings;
			__BI(u32,UINT32,UINT32);
			nummapthings = u32;
			if (a_Load)
				mapthings = Z_Malloc(sizeof(*mapthings) * nummapthings, PU_LEVEL, (void**)&mapthings);
			for (i = 0; i < u32; i++)
			{
				// Current Pointer
				__BI(mapthings[i],POINTERISDIRECT,POINTER);
				
				// Dump
				__BI(mapthings[i].x,SHORT,INT16);
				__BI(mapthings[i].y,SHORT,INT16);
				__BI(mapthings[i].z,SHORT,INT16);
				__BI(mapthings[i].angle,SHORT,INT16);
				__BI(mapthings[i].type,SHORT,INT16);
				__BI(mapthings[i].options,SHORT,INT16);
				__BI(mapthings[i].IsHexen,BOOLT,UINT8);
				__BI(mapthings[i].MarkedWeapon,BOOLT,UINT8);
				__BI(mapthings[i].MoType,INT,UINT32);
				__BI(mapthings[i].Special,UINT8,UINT8);
				__BI(mapthings[i].ID,UINT16,UINT16);
				__BI(mapthings[i].HeightOffset,INT16,INT16);
				__BI(mapthings[i].mobj,POINTERTO,POINTER);
				
				for (j = 0; j < 5; j++)
					__BI(mapthings[i].Args[j],UINT8,UINT8);
			}
			
			// Record
			__REC;
		}
		
		// REJECT and BLOCKMAP
		if (__HEADER("SGMJ"))
		{
			// Save the reject (will take up TONS of room)
				// g_RJMSize * 2
			__BI(g_RJMSize,SIZET,UINT32);
			if (a_Load)
				rejectmatrix = Z_Malloc(sizeof(*rejectmatrix) * g_RJMSize, PU_LEVEL, NULL);
			for (i = 0; i < g_RJMSize; i++)
				__BI(rejectmatrix[i],UINT8,UINT8);
			
			// Save the blockmap
				// As above, can get pretty big
			__BI(g_BMLSize,SIZET,UINT32);
			if (a_Load)
			{
				blockmaplump = Z_Malloc(sizeof(*blockmaplump) * g_BMLSize, PU_LEVEL, NULL);
				blockmap = blockmaplump + 4;	// Needed for compat
			}
			for (i = 0; i < g_BMLSize; i++)
				__BI(blockmaplump[i],LONG,INT32);
			
				// Block Map origins
			__BI(bmapwidth,INT,INT32);
			__BI(bmapheight,INT,INT32);
			__BI(bmaporgx,FIXEDT,INT32);
			__BI(bmaporgy,FIXEDT,INT32);
				
				// Blocklinks
			if (a_Load)
				blocklinks = Z_Malloc(sizeof(*blocklinks) * (bmapwidth * bmapheight), PU_LEVEL, NULL);
			for (i = 0; i < (bmapwidth * bmapheight); i++)
				__BI(blocklinks[i],POINTERTO,POINTER);
			
			// Record
			__REC;
		}
		
		// Game State -- After Map Data Is Loaded
		if (__HEADER("SGGT"))
		{
			// doomstat.h
				// Player Starts
			u32 = MAXPLAYERS;
			__BI(u32,UINT32,UINT32);
			for (i = 0; i < u32; i++)
			{
				u32b = playerstarts[i] - mapthings;
				__BI(u32b,UINT32,UINT32);
				playerstarts[i] = NULL;
				if (u32b >= 0 && u32b < nummapthings)
					playerstarts[i] = &mapthings[u32b];
			}
			
			// p_setup.h
				// Deathmatch Starts
			u32 = numdmstarts;
			__BI(u32,UINT32,UINT32);
			for (i = 0; i < u32; i++)
			{
				u32b = deathmatchstarts[i] - mapthings;
				__BI(u32b,UINT32,UINT32);
				deathmatchstarts[i] = NULL;
				if (u32b >= 0 && u32b < nummapthings)
					deathmatchstarts[i] = &mapthings[u32b];
			}
			
			// p_map.c
				// Spechits
			__BI(spechit_max,INT,INT32);
			__BI(numspechit,INT,INT32);
			if (a_Load)
				spechit = Z_Malloc(sizeof(*spechit) * spechit_max, PU_LEVEL, NULL);
			for (i = 0; i < numspechit; i++)
				__BI(spechit[i],INT,INT32);
			
			// Record
			__REC;
		}
		
		// Thinkers -- The whole mass of them!
			// This dual load/save code does not work well with thinkers. So I
			// have to hack around it to get it to work.
		if (__HEADER("SGTH"))
		{
			// Save/Restore Thinkercap
				// Be sure to store the is of it, for references
			__BI(thinkercap,POINTERISDIRECT,POINTER);
			__BI(thinkercap.Type,INT,UINT8);
			__BI(thinkercap.next,POINTERTO,POINTER);
			__BI(thinkercap.prev,POINTERTO,POINTER);
			
			// Save thinker list
			__BI(g_NumThinkerList,SIZET,UINT32);
			if (a_Load)
				g_ThinkerList = Z_Malloc(sizeof(*g_ThinkerList) * g_NumThinkerList, PU_LEVEL, NULL);
			// Go through each one
			for (i = 0; i < g_NumThinkerList; i++)
			{
				// Dump Thinker if Saving
				if (!a_Load)
				{
					// No Thinker Here?
					if (!g_ThinkerList[i])
						u8 = 'N';
					else
						u8 = 'K';
					
					// Save
					__BI(u8,UINT8,UINT8);
					
					// Skip?
					if (u8 == 'N')
						continue;
					
					Thinker = g_ThinkerList[i];
					__BI(Thinker->Type,INT,UINT8);
				}
				
				// Load Thinker when loading
				else
				{
					// Determie if thinker is NULL, or not
					u8 = 'N';
					__BI(u8,UINT8,UINT8);
					
					// If it is NULL continue on
					if (u8 == 'N')
						continue;
					
					// Load the type
					u8 = 0;
					__BI(u8,UINT8,UINT8);
					
					// Check bounds (making sure it is valid)
					if (u8 < 0 || u8 >= NUMPTHINKERTYPES)
						u8 = PTT_CAP;
					
					// Allocate and place in spot
					Thinker = g_ThinkerList[i] = Z_Malloc(g_ThinkerData[u8].Size, PU_LEVEL, NULL);
					
					// Set Type and function
					Thinker->Type = u8;
					Thinker->function = g_ThinkerData[u8].Func;
				}
				
				// Dump IS Pointer
				__BI((*g_ThinkerList[i]),POINTERISDIRECT,POINTER);
				
				// Dump thinker chain
				__BI(Thinker->next,POINTERTO,POINTER);
				__BI(Thinker->prev,POINTERTO,POINTER);
				
				// Dump info about object
				switch (Thinker->Type)
				{
						// Map Object
					case PTT_MOBJ:
						// Clone
						Mobj = (mobj_t*)Thinker;
						
						// Start Dumping
						__BI(Mobj->x,FIXEDT,INT32);
						__BI(Mobj->y,FIXEDT,INT32);
						__BI(Mobj->z,FIXEDT,INT32);
						__BI(Mobj->floorz,FIXEDT,INT32);
						__BI(Mobj->ceilingz,FIXEDT,INT32);
						__BI(Mobj->radius,FIXEDT,INT32);
						__BI(Mobj->height,FIXEDT,INT32);
						__BI(Mobj->momx,FIXEDT,INT32);
						__BI(Mobj->momy,FIXEDT,INT32);
						__BI(Mobj->momz,FIXEDT,INT32);
						__BI(Mobj->MaxZObtained,FIXEDT,INT32);

						__BI(Mobj->snext,POINTERTO,POINTER);
						__BI(Mobj->sprev,POINTERTO,POINTER);
						__BI(Mobj->bnext,POINTERTO,POINTER);
						__BI(Mobj->bprev,POINTERTO,POINTER);
						__BI(Mobj->subsector,POINTERTO,POINTER);
						__BI(Mobj->target,POINTERTO,POINTER);
						__BI(Mobj->player,POINTERTO,POINTER);
						__BI(Mobj->spawnpoint,POINTERTO,POINTER);
						__BI(Mobj->tracer,POINTERTO,POINTER);
						__BI(Mobj->touching_sectorlist,POINTERTO,POINTER);
						__BI(Mobj->ChildFloor,POINTERTO,POINTER);

						__BI(Mobj->angle,ANGLET,UINT32);

						__BI(Mobj->RemoveMo,BOOLT,UINT8);

						__BI(Mobj->sprite,INT,UINT32);
						__BI(Mobj->frame,INT,INT32);
						__BI(Mobj->skin,INT,INT32);
						__BI(Mobj->tics,INT,INT32);
						__BI(Mobj->type,INT,INT32);
						__BI(Mobj->flags,INT,INT32);
						__BI(Mobj->eflags,INT,INT32);
						__BI(Mobj->flags2,INT,INT32);
						__BI(Mobj->special1,INT,INT32);
						__BI(Mobj->special2,INT,INT32);
						__BI(Mobj->health,INT,INT32);
						__BI(Mobj->movedir,INT,INT32);
						__BI(Mobj->movecount,INT,INT32);
						__BI(Mobj->reactiontime,INT,INT32);
						__BI(Mobj->threshold,INT,INT32);
						__BI(Mobj->lastlook,INT,INT32);
						__BI(Mobj->friction,INT,INT32);
						__BI(Mobj->movefactor,INT,INT32);
						__BI(Mobj->dropped_ammo_count,INT,INT32);
						__BI(Mobj->RXShotWithWeapon,INT,INT32);
						__BI(Mobj->RXAttackAttackType,INT,INT32);
						__BI(Mobj->RemType,INT,INT32);
						__BI(Mobj->SkinTeamColor,INT,INT32);

						__BI(Mobj->XFlagsA,UINT32,UINT32);
						__BI(Mobj->XFlagsB,UINT32,UINT32);
						__BI(Mobj->XFlagsC,UINT32,UINT32);
						__BI(Mobj->XFlagsD,UINT32,UINT32);

						for (l = 0; l < NUMINFORXFIELDS; l++)
							__BI(Mobj->RXFlags[l],UINT32,UINT32);

						// State -- Can be tricky
						if (!a_Load && Mobj->state)
							__BI(Mobj->state->StateNum,INT,UINT32);
						else
						{
							u32 = 0;
							__BI(u32,UINT32,UINT32);
							if (u32 >= 0 && u32 < NUMSTATES)
								Mobj->state = states[u32];
						}

						// Map Object OnTop Ref
						for (l = 0; l < 2; l++)
						{
							// Count
							__BI(Mobj->MoOnCount[l],SIZET,UINT32);
	
							// Allocate?
							if (a_Load && Mobj->MoOnCount[l])
								Mobj->MoOn[l] = Z_Malloc(sizeof(*Mobj->MoOn[l]) * Mobj->MoOnCount[l],
													PU_STATIC, NULL);
	
							// Dump Refs
							for (j = 0; j < Mobj->MoOnCount[l]; j++)
								__BI(Mobj->MoOn[l][j],POINTERTO,POINTER);
						}

						// Dump Reference Counts
						u32 = NUMPMOBJREFTYPES;
						__BI(u32,UINT32,UINT32);
						for (l = 0; l < u32; l++)
						{
							// Base Counts
							__BI(Mobj->RefCount[l],INT32,INT32);
							__BI(Mobj->RefListSz[l],SIZET,UINT32);
	
							// Allocate?
							if (a_Load)
								Mobj->RefList[l] = Z_Malloc(sizeof(*Mobj->RefList[l]) * Mobj->RefListSz[l],
														PU_LEVEL, NULL);
	
							// Reference List
							for (j = 0; j < Mobj->RefListSz[l]; j++)
								__BI(Mobj->RefList[l][j],POINTERTO,POINTER);
						}

						// Noise Thinker
						__BI(Mobj->NoiseThinker.Flags,UINT32,UINT32);
						__BI(Mobj->NoiseThinker.x,FIXEDT,INT32);
						__BI(Mobj->NoiseThinker.y,FIXEDT,INT32);
						__BI(Mobj->NoiseThinker.z,FIXEDT,INT32);
						__BI(Mobj->NoiseThinker.momx,FIXEDT,INT32);
						__BI(Mobj->NoiseThinker.momy,FIXEDT,INT32);
						__BI(Mobj->NoiseThinker.momz,FIXEDT,INT32);
						__BI(Mobj->NoiseThinker.Pitch,FIXEDT,INT32);
						__BI(Mobj->NoiseThinker.Volume,FIXEDT,INT32);
						__BI(Mobj->NoiseThinker.Angle,ANGLET,UINT32);

						// Reference info based on type
						Mobj->info = mobjinfo[Mobj->type];
						break;
						
						// Unknown!?
					default:
						break;
				}
			}
			
			// Store/Recover thinkercap linked list
			//for (Thinker = &thinkercap;
			
#if 0
			// If Saving, start at head thinker
			Thinker = NULL;
			OldThinker = &thinkercap;
			if (!a_Load)
				Thinker = OldThinker->next;
				
			// Dump Base Cap pointers
			__BI(OldThinker->next,POINTERTO,POINTER);
			__BI(OldThinker->prev,POINTERTO,POINTER);
			
			// Go through the list
			do
			{
				// Determine what to do
					// Loading?
				if (a_Load)
					u8 = '?';	// Unknown Thinker
					// Saving?
				else
					u8 = PS_IDThinkerType(Thinker);
				
				// Load/Save Type
				__BI(u8,UINT8,UINT8);
				
				// Dump Pointer
				__BI(OldThinker->next,POINTERISDIRECT,POINTER);
				
				// Save thinker data
				switch (u8)
				{
						// P_MobjNullThinker, P_MobjThinker
					case 'a':
					case 'b':
						// Reference it
							// Load
						if (a_Load)
						{
							Mobj = Thinker = Z_Malloc(sizeof(*Mobj), PU_LEVEL, NULL);
							Thinker->function.acv = (u8 == 'a' ? P_MobjNullThinker : P_MobjThinker);
						}
							// Save
						else
							Mobj = Thinker;
						
						// Dump Data
						__BI(Mobj->x,FIXEDT,INT32);
						__BI(Mobj->y,FIXEDT,INT32);
						__BI(Mobj->z,FIXEDT,INT32);
						__BI(Mobj->floorz,FIXEDT,INT32);
						__BI(Mobj->ceilingz,FIXEDT,INT32);
						__BI(Mobj->radius,FIXEDT,INT32);
						__BI(Mobj->height,FIXEDT,INT32);
						__BI(Mobj->momx,FIXEDT,INT32);
						__BI(Mobj->momy,FIXEDT,INT32);
						__BI(Mobj->momz,FIXEDT,INT32);
						__BI(Mobj->MaxZObtained,FIXEDT,INT32);
						
						__BI(Mobj->snext,POINTERTO,POINTER);
						__BI(Mobj->sprev,POINTERTO,POINTER);
						__BI(Mobj->bnext,POINTERTO,POINTER);
						__BI(Mobj->bprev,POINTERTO,POINTER);
						__BI(Mobj->subsector,POINTERTO,POINTER);
						__BI(Mobj->target,POINTERTO,POINTER);
						__BI(Mobj->player,POINTERTO,POINTER);
						__BI(Mobj->spawnpoint,POINTERTO,POINTER);
						__BI(Mobj->tracer,POINTERTO,POINTER);
						__BI(Mobj->touching_sectorlist,POINTERTO,POINTER);
						__BI(Mobj->ChildFloor,POINTERTO,POINTER);
						
						__BI(Mobj->angle,ANGLET,UINT32);
						
						__BI(Mobj->RemoveMo,BOOLT,UINT8);
						
						__BI(Mobj->sprite,INT,UINT32);
						__BI(Mobj->frame,INT,INT32);
						__BI(Mobj->skin,INT,INT32);
						__BI(Mobj->tics,INT,INT32);
						__BI(Mobj->type,INT,INT32);
						__BI(Mobj->flags,INT,INT32);
						__BI(Mobj->eflags,INT,INT32);
						__BI(Mobj->flags2,INT,INT32);
						__BI(Mobj->special1,INT,INT32);
						__BI(Mobj->special2,INT,INT32);
						__BI(Mobj->health,INT,INT32);
						__BI(Mobj->movedir,INT,INT32);
						__BI(Mobj->movecount,INT,INT32);
						__BI(Mobj->reactiontime,INT,INT32);
						__BI(Mobj->threshold,INT,INT32);
						__BI(Mobj->lastlook,INT,INT32);
						__BI(Mobj->friction,INT,INT32);
						__BI(Mobj->movefactor,INT,INT32);
						__BI(Mobj->dropped_ammo_count,INT,INT32);
						__BI(Mobj->RXShotWithWeapon,INT,INT32);
						__BI(Mobj->RXAttackAttackType,INT,INT32);
						__BI(Mobj->RemType,INT,INT32);
						__BI(Mobj->SkinTeamColor,INT,INT32);
						
						__BI(Mobj->XFlagsA,UINT32,UINT32);
						__BI(Mobj->XFlagsB,UINT32,UINT32);
						__BI(Mobj->XFlagsC,UINT32,UINT32);
						__BI(Mobj->XFlagsD,UINT32,UINT32);
						
						for (i = 0; i < NUMINFORXFIELDS; i++)
							__BI(Mobj->RXFlags[i],UINT32,UINT32);
						
						// State -- Can be tricky
						if (!a_Load && Mobj->state)
							__BI(Mobj->state->StateNum,INT,UINT32);
						else
						{
							u32 = 0;
							__BI(u32,UINT32,UINT32);
							if (u32 >= 0 && u32 < NUMSTATES)
								Mobj->state = states[u32];
						}
						
						// Map Object OnTop Ref
						for (i = 0; i < 2; i++)
						{
							// Count
							__BI(Mobj->MoOnCount[i],SIZET,UINT32);
							
							// Allocate?
							if (a_Load)
								Mobj->MoOn[i] = Z_Malloc(sizeof(*Mobj->MoOn[i]) * Mobj->MoOnCount[i],
													PU_STATIC, NULL);
							
							// Dump Refs
							for (j = 0; j < Mobj->MoOnCount[i]; j++)
								__BI(Mobj->MoOn[i][j],POINTERTO,POINTER);
						}
						
						// Dump Reference Counts
						u32 = NUMPMOBJREFTYPES;
						__BI(u32,UINT32,UINT32);
						for (i = 0; i < u32; i++)
						{
							// Base Counts
							__BI(Mobj->RefCount[i],INT32,INT32);
							__BI(Mobj->RefListSz[i],SIZET,UINT32);
							
							// Allocate?
							if (a_Load)
								Mobj->RefList[i] = Z_Malloc(sizeof(*Mobj->RefList[i]) * Mobj->RefListSz[i],
														PU_LEVEL, NULL);
							
							// Reference List
							for (j = 0; j < Mobj->RefListSz[i]; j++)
								__BI(Mobj->RefList[i][j],POINTERTO,POINTER);
						}
						
						// Noise Thinker
						__BI(Mobj->NoiseThinker.Flags,UINT32,UINT32);
						__BI(Mobj->NoiseThinker.x,FIXEDT,INT32);
						__BI(Mobj->NoiseThinker.y,FIXEDT,INT32);
						__BI(Mobj->NoiseThinker.z,FIXEDT,INT32);
						__BI(Mobj->NoiseThinker.momx,FIXEDT,INT32);
						__BI(Mobj->NoiseThinker.momy,FIXEDT,INT32);
						__BI(Mobj->NoiseThinker.momz,FIXEDT,INT32);
						__BI(Mobj->NoiseThinker.Pitch,FIXEDT,INT32);
						__BI(Mobj->NoiseThinker.Volume,FIXEDT,INT32);
						__BI(Mobj->NoiseThinker.Angle,ANGLET,UINT32);
						
						// Reference info based on type
						Mobj->info = mobjinfo[Mobj->type];
						break;
						
						// Unknown
					default:
						Thinker = Z_Malloc(sizeof(*Thinker), PU_LEVEL, NULL);
						break;
				}
				
				// Dump Thinker Chains
				__BI(Thinker->prev,POINTERTO,POINTER);
				__BI(Thinker->next,POINTERTO,POINTER);
#if 0		
if (a_Thinker->function.acv == P_MobjNullThinker)		return 'a';
else if (a_Thinker->function.acv == P_MobjThinker)	return 'b';
else if (a_Thinker->function.acv == T_FireFlicker)	return 'c';
else if (a_Thinker->function.acv == T_Friction)	return 'd';
else if (a_Thinker->function.acv == T_Glow)	return 'e';
else if (a_Thinker->function.acv == T_LightFade)	return 'f';
else if (a_Thinker->function.acv == T_LightFlash)	return 'g';
else if (a_Thinker->function.acv == T_MoveCeiling)	return 'h';
else if (a_Thinker->function.acv == T_MoveElevator)	return 'i';
else if (a_Thinker->function.acv == T_MoveFloor)	return 'j';
else if (a_Thinker->function.acv == T_PlatRaise)	return 'k';
else if (a_Thinker->function.acv == T_Pusher)	return 'l';
else if (a_Thinker->function.acv == T_Scroll)	return 'm';
else if (a_Thinker->function.acv == T_StrobeFlash)	return 'n';
else if (a_Thinker->function.acv == T_VerticalDoor)	return 'o';
#endif
				// Remember Old
				OldThinker = Thinker;
				
				// Action to perform
				if (!a_Load)	// If saving, go to next thinker
					Thinker = Thinker->next;
				
				// At thinkercap?
				if (!a_Load && Thinker == &thinkercap)
				{
					// Save ? to end reading it
					u8 = '?';
					__BI(u8,UINT8,UINT8);
					break;
				}
			} while (u8 != '?');
#endif

			// Record
			__REC;
		}
		
		//////////////////////////////
		// If saving, don't continue
		if (!a_Load)
			Continue = false;
	}
	
	/* Handle Pointer References */
	if (a_Load)
	{
		// Go through each one and Reference everything
		for (i = 0; i < l_NumDerefs; i++)
		{
			// Missing value to set?
			if (!l_Derefs[i].SetVal)
				continue;
			
			// Go through change list
			for (j = 0; j < l_Derefs[i].NumChangePtr; j++)
			{
				vp = *(l_Derefs[i].ChangePtr[j]);
				*(l_Derefs[i].ChangePtr[j]) = l_Derefs[i].SetVal;
				
#if 0
				if (devparm)
					CONL_PrintF("SAVE DEBUG: %p set to %p (was %p)\n",
							(l_Derefs[i].ChangePtr[j]),
							l_Derefs[i].SetVal, vp
						);
#endif
				
				l_Derefs[i].ChangePtr[j] = NULL;	// Clear for future checking
			}
		}
		
		// If debugging go through again
#if 0
		if (devparm)
			for (i = 0; i < l_NumDerefs; i++)
				for (j = 0; j < l_Derefs[i].NumChangePtr; j++)
					if (l_Derefs[i].ChangePtr[j])
					{
						CONL_PrintF("SAVE DEBUG: Missed %p (by %p, sv = %p)\n",
									l_Derefs[i].ChangePtr[j],
									((void*)((uintptr_t)l_Derefs[i].UniqPtr)),
									l_Derefs[i].SetVal
								);
					}
#endif

		// Free all references
		Z_FreeTags(PU_SGPTRREF, PU_SGPTRREF);
		l_Derefs = NULL;
		l_NumDerefs = 0;
	}
	
	/* Final Cleaning Up */
	// Repair thinkercap chain (it appears to get unlinked)
	if (a_Load)
	{
		thinkercap.next->prev = &thinkercap;
		thinkercap.prev->next = &thinkercap;
	}
	
	/* Success? */
	return true;
#undef BUFSIZE
#endif
}

/*****************************************************************************/
/*****************************************************************************/

/* P_LoadGameFromBS() -- Load game from block stream */
bool_t P_LoadGameFromBS(D_BS_t* const a_Stream, I_HostAddress_t* const a_NetAddr)
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
	mapthing_t* MapThing;
	sector_t* Sector;
	
	/* Do bi-way */
	return P_SGDXSpec(a_Stream, a_NetAddr, true);
	return P_SGBiWayBS(a_Stream, true);
	
	/* Check */
	if (!a_Stream)
		return false;
	
	/* Clear Future Ref */
		
	/* Constantly Read Blocks */
	memset(Header, 0, sizeof(Header));
	memset(Buf, 0, sizeof(Buf));
	while (D_BSPlayBlock(a_Stream, Header))
	{
		if (devparm)
			CONL_PrintF("LOAD: Read %s\n", Header);
		
		// SGVR -- Version
		if (strcasecmp(Header, "SGVR") == 0)
		{
			// Read version markers
			VerLeg = D_BSru8(a_Stream);
			VerMaj = D_BSru8(a_Stream);
			VerMin = D_BSru8(a_Stream);
			VerRel = D_BSru8(a_Stream);
			
			// Print Info
			CONL_PrintF("LOAD: Loading Version %i.%i%c (%i)\n",
					VerMaj, VerMin, VerRel,
					VerLeg
				);
			
			// Read Other Info
			D_BSrs(a_Stream, Buf, BUFSIZE - 1);
			CONL_PrintF("LOAD: Release \"%s\"\n", Buf);
			D_BSrs(a_Stream, Buf, BUFSIZE - 1);
			CONL_PrintF("LOAD: Fully known as %s\n", Buf);
			D_BSrs(a_Stream, Buf, BUFSIZE - 1);
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
			while ((CharBit = D_BSru8(a_Stream)) != 'E')
			{
				// Read file and DOS Names
				memset(Buf, 0, sizeof(Buf));
				memset(BufB, 0, sizeof(BufB));
				
				D_BSrs(a_Stream, Buf, BUFSIZE - 1);
				D_BSrs(a_Stream, BufB, BUFSIZE - 1);
				
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
				CheckBit = D_BSru8(a_Stream); // IP
				CheckBit = D_BSru8(a_Stream); // WN
				CheckBit = D_BSru8(a_Stream); // VI
				
				// Ignore index offset sizes and such
				u32 = D_BSru32(a_Stream);
				u32 = D_BSru32(a_Stream);
				u32 = D_BSru32(a_Stream);
				
				// Ignore integer sums
				for (i = 0; i < 8; i++)
					u32 = D_BSru32(a_Stream);
				
				// Compare MD5/SS against WAD
				for (i = 0; i < 64; i++)
					CheckBit = D_BSru8(a_Stream);
				
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
			gametic = D_BSru32(a_Stream);
			D_SyncNetSetMapTime(D_BSru32(a_Stream));
			D_BSru32(a_Stream);	// Ignore real time, not that important
			
			// Read State Info
			gamestate = D_BSru8(a_Stream);
			u8 = D_BSru8(a_Stream);	// Ignore demorecording
			u8 = D_BSru8(a_Stream);	// Ignore demoplayback
			multiplayer = D_BSru8(a_Stream);
			
			// Read Map Name
			memset(Buf, 0, sizeof(Buf));
			D_BSrs(a_Stream, Buf, BUFSIZE - 1);
			
			// Find level
			g_CurrentLevelInfo = P_FindLevelByNameEx(Buf, NULL);
		}
		
		// SGMV -- Map Vertexes
		else if (strcasecmp(Header, "SGMV") == 0)
		{
			// Read Count
			numvertexes = D_BSru32(a_Stream);
			vertexes = Z_Malloc(sizeof(*vertexes) * numvertexes, PU_LEVEL, NULL);
			
			// Read every vertex
			for (i = 0; i < numvertexes; i++)
			{
				vertexes[i].x = D_BSri32(a_Stream);
				vertexes[i].y = D_BSri32(a_Stream);
			}
		}
		
		// SGMS -- Map Sectors
		else if (strcasecmp(Header, "SGMS") == 0)
		{
			numsectors = D_BSru32(a_Stream);
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
			nummapthings = D_BSru32(a_Stream);
			mapthings = Z_Malloc(sizeof(*mapthings) * nummapthings, PU_LEVEL, NULL);
			
			// Read every map thing
			for (i = 0; i < nummapthings; i++)
			{
				// Set pointer reference
				MapThing = &mapthings[i];
				PLGS_SetRef(D_BSrp(a_Stream), MapThing);
				
				// Read the remainder
				MapThing->x = D_BSri16(a_Stream);
				MapThing->y = D_BSri16(a_Stream);
				MapThing->z = D_BSri16(a_Stream);
				MapThing->angle = D_BSri16(a_Stream);
				MapThing->type = D_BSri16(a_Stream);
				MapThing->options = D_BSri16(a_Stream);
				PLGS_DeRef(D_BSrp(a_Stream), (void**)&MapThing->mobj);
				MapThing->IsHexen = D_BSru8(a_Stream);
				MapThing->HeightOffset = D_BSri16(a_Stream);
				MapThing->ID = D_BSru16(a_Stream);
				MapThing->Special = D_BSru8(a_Stream);
				for (j = 0; j < 5; j++)
					MapThing->Args[i] = D_BSru8(a_Stream);
				MapThing->MoType = D_BSru32(a_Stream);
				MapThing->MarkedWeapon = D_BSru8(a_Stream);
			}
		}
		
		// SGMR -- Other Map Stuff
		else if (strcasecmp(Header, "SGMR") == 0)
		{
			// Read Player Starts
			j = D_BSru32(a_Stream);
			for (k = 0; k < j; k++)
			{
				u32 = D_BSru32(a_Stream);
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
void P_SGBS_Time(D_BS_t* const a_Stream);
void P_SGBS_Version(D_BS_t* const a_Stream);
void P_SGBS_WAD(D_BS_t* const a_Stream);
void P_SGBS_NetProfiles(D_BS_t* const a_Stream);
void P_SGBS_SplitPlayers(D_BS_t* const a_Stream);
void P_SGBS_Players(D_BS_t* const a_Stream);
void P_SGBS_MapData(D_BS_t* const a_Stream);
void P_SGBS_Thinkers(D_BS_t* const a_Stream);
void P_SGBS_State(D_BS_t* const a_Stream);

/* P_SaveGameToBS() -- Save game to block stream */
bool_t P_SaveGameToBS(D_BS_t* const a_Stream, I_HostAddress_t* const a_NetAddr)
{
	const char* c;
	
	/* Check */
	if (!a_Stream)
		return false;
		
	/* Do bi-way */
	return P_SGDXSpec(a_Stream, a_NetAddr, false);
	return P_SGBiWayBS(a_Stream, false);
		
	/* Create Header Block */
	// Save Game Save Stream
		// Base
	D_BSBaseBlock(a_Stream, "SGSS");
		// Fill
	for (c = "ReMooD Save Game"; *c; c++)
		D_BSwu8(a_Stream, *c);
		// Record
	D_BSRecordBlock(a_Stream);
	
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
	D_BSBaseBlock(a_Stream, "SGEE");
		// Fill
	for (c = "End Stream"; *c; c++)
		D_BSwu8(a_Stream, *c);
		// Record
	D_BSRecordBlock(a_Stream);
	
	/* Success */
	return true;
}

/*** SAVING THE GAME STATE ***/

/* P_SGBS_Time() -- Print Time Information */
void P_SGBS_Time(D_BS_t* const a_Stream)
{
	/* Begin Header */
	D_BSBaseBlock(a_Stream, "SGVT");
	
	/* Place Time Information (Save Related) */
	D_BSwu32(a_Stream, time(NULL));
	D_BSwu32(a_Stream, g_ProgramTic);
	
	/* Record Block */
	D_BSRecordBlock(a_Stream);
}

/* P_SGBS_Version() -- Write version information */
void P_SGBS_Version(D_BS_t* const a_Stream)
{
	/* Begin Header */
	D_BSBaseBlock(a_Stream, "SGVR");
	
	/* Fill With Versioning */
		// Legacy Version
	D_BSwu8(a_Stream, VERSION);
		// ReMooD Version
	D_BSwu8(a_Stream, REMOOD_MAJORVERSION);
	D_BSwu8(a_Stream, REMOOD_MINORVERSION);
	D_BSwu8(a_Stream, REMOOD_RELEASEVERSION);
		// Version Strings
	D_BSws(a_Stream, REMOOD_VERSIONCODESTRING);
	D_BSws(a_Stream, REMOOD_FULLVERSIONSTRING);
	D_BSws(a_Stream, REMOOD_URL);
		// Compilation Stuff
	D_BSws(a_Stream, __TIME__);
	D_BSws(a_Stream, __DATE__);
	
	/* Record Block */
	D_BSRecordBlock(a_Stream);
}

/* P_SGBS_WAD() -- Write WAD State */
void P_SGBS_WAD(D_BS_t* const a_Stream)
{
	const WL_WADFile_t* CurVWAD;
	size_t i;
	
	/* Begin Header */
	D_BSBaseBlock(a_Stream, "SGVW");
	
	/* Iterate all VWADs */	
	for (CurVWAD = WL_IterateVWAD(NULL, true); CurVWAD; CurVWAD = WL_IterateVWAD(CurVWAD, true))
	{
		D_BSwu8(a_Stream, 'B');
		
		// Print WAD Names
		D_BSws(a_Stream, CurVWAD->__Private.__FileName);
		D_BSws(a_Stream, CurVWAD->__Private.__DOSName);
		
		// Print Some Flags
		D_BSwu8(a_Stream, (CurVWAD->__Private.__IsIWAD ? 'I' : 'P'));
		D_BSwu8(a_Stream, (CurVWAD->__Private.__IsWAD ? 'W' : 'N'));
		D_BSwu8(a_Stream, (CurVWAD->__Private.__IsValid ? 'V' : 'I'));
		
		// Print Some WAD Identification
		D_BSwu32(a_Stream, CurVWAD->NumEntries);
		D_BSwu32(a_Stream, CurVWAD->__Private.__IndexOff);
		D_BSwu32(a_Stream, CurVWAD->__Private.__Size);
		
		// Print WAD Sums
		for (i = 0; i < 4; i++)
			D_BSwu32(a_Stream, CurVWAD->CheckSum[i]);
		for (i = 0; i < 4; i++)
			D_BSwu32(a_Stream, CurVWAD->SimpleSum[i]);
		for (i = 0; i < 32; i++)
			D_BSwu8(a_Stream, CurVWAD->CheckSumChars[i]);
		for (i = 0; i < 32; i++)
			D_BSwu8(a_Stream, CurVWAD->SimpleSumChars[i]);
	}
	D_BSwu8(a_Stream, 'E');
	
	/* Record Block */
	D_BSRecordBlock(a_Stream);
}

/* P_SGBS_NetProfiles() -- Record network profiles */
void P_SGBS_NetProfiles(D_BS_t* const a_Stream)
{
}

/* P_SGBS_SplitPlayers() -- Split players */
void P_SGBS_SplitPlayers(D_BS_t* const a_Stream)
{
	size_t i;
	
	/* Begin Header */
	D_BSBaseBlock(a_Stream, "SGSC");
	
	/* Print Local Players */
	D_BSwu8(a_Stream, g_SplitScreen);
	
	// Go through Each
	for (i = 0; i < MAXSPLITSCREEN; i++)
	{
		D_BSwu8(a_Stream, g_Splits[i].Active);
		D_BSwu8(a_Stream, g_Splits[i].Console);
		D_BSwu8(a_Stream, g_Splits[i].Display);
	}
	
	/* Record Block */
	D_BSRecordBlock(a_Stream);
}

/* P_SGBS_Players() -- Dump Players */
void P_SGBS_Players(D_BS_t* const a_Stream)
{
	size_t i, j;
	player_t* Player;
	
	/* Begin Header */
	D_BSBaseBlock(a_Stream, "SGPL");
	
	for (i = 0; i < MAXPLAYERS; i++)
	{
		Player = &players[i];
		
		// Print Identifier
		D_BSwu8(a_Stream, i);
		D_BSwp(a_Stream, Player);
		
		// Not in game?
		if (!playeringame[i])
		{
			D_BSwu8(a_Stream, 'V');
			continue;
		}
		
		// Write as In Game
		D_BSwu8(a_Stream, 'P');
		
		// Write Links
		D_BSwp(a_Stream, Player->ProfileEx);
		D_BSws(a_Stream, (Player->ProfileEx ? Player->ProfileEx->UUID : ""));
		
		// Print Map Objects Connected To
		D_BSwp(a_Stream, Player->mo);
		D_BSwp(a_Stream, Player->rain1);
		D_BSwp(a_Stream, Player->rain2);
		D_BSwp(a_Stream, Player->attacker);
		
		// Write Player Info
		D_BSwu8(a_Stream, Player->playerstate);
		D_BSwi32(a_Stream, Player->viewz);
		D_BSwi32(a_Stream, Player->viewheight);
		D_BSwi32(a_Stream, Player->deltaviewheight);
		D_BSwi32(a_Stream, Player->bob);
		D_BSwu32(a_Stream, Player->aiming);
		D_BSwi32(a_Stream, Player->health);
		D_BSwi32(a_Stream, Player->armorpoints);
		D_BSwi32(a_Stream, Player->armortype);
		D_BSwu32(a_Stream, Player->cards);
		D_BSwu32(a_Stream, Player->backpack);
		D_BSwu32(a_Stream, Player->addfrags);
		D_BSwi32(a_Stream, Player->readyweapon);
		D_BSws(a_Stream, Player->weaponinfo[Player->readyweapon]->ClassName);
		D_BSwi32(a_Stream, Player->pendingweapon);
		D_BSws(a_Stream, ((Player->pendingweapon < 0) ? "NoPending" :Player->weaponinfo[Player->pendingweapon]->ClassName));
		D_BSwu8(a_Stream, Player->originalweaponswitch);
		D_BSwu8(a_Stream, Player->autoaim_toggle);
		D_BSwu8(a_Stream, Player->attackdown);
		D_BSwu8(a_Stream, Player->usedown);
		D_BSwu8(a_Stream, Player->jumpdown);
		D_BSwu32(a_Stream, Player->cheats);
		D_BSwu32(a_Stream, Player->refire);
		D_BSwu32(a_Stream, Player->killcount);
		D_BSwu32(a_Stream, Player->itemcount);
		D_BSwu32(a_Stream, Player->secretcount);
		D_BSwu32(a_Stream, Player->damagecount);
		D_BSwu32(a_Stream, Player->bonuscount);
		D_BSwu32(a_Stream, Player->specialsector);
		D_BSwu32(a_Stream, Player->extralight);
		D_BSwu32(a_Stream, Player->fixedcolormap);
		D_BSwu32(a_Stream, Player->skincolor);
		D_BSwu32(a_Stream, Player->skin);
		D_BSwu8(a_Stream, Player->didsecret);
		D_BSwi32(a_Stream, Player->chickenTics);
		D_BSwi32(a_Stream, Player->chickenPeck);
		D_BSwi32(a_Stream, Player->flamecount);
		D_BSwi32(a_Stream, Player->flyheight);
		D_BSwi32(a_Stream, Player->inv_ptr);
		D_BSwi32(a_Stream, Player->st_curpos);
		D_BSwi32(a_Stream, Player->st_inventoryTics);
		D_BSwi32(a_Stream, Player->flushdelay);
		D_BSwi32(a_Stream, Player->MoveMom);
		D_BSwi32(a_Stream, Player->TargetViewZ);
		D_BSwi32(a_Stream, Player->FakeMom[0]);
		D_BSwi32(a_Stream, Player->FakeMom[1]);
		D_BSwi32(a_Stream, Player->FakeMom[2]);
		D_BSwi32(a_Stream, Player->MaxHealth[0]);
		D_BSwi32(a_Stream, Player->MaxHealth[1]);
		D_BSwi32(a_Stream, Player->MaxArmor[0]);
		D_BSwi32(a_Stream, Player->MaxArmor[1]);
		
		// Current Weapon Level
		D_BSwu8(a_Stream, (Player->weaponinfo == wpnlev2info ? 1 : 0));
		
		// Write Variable Info
			// Powerups
		for (j = 0; j < NUMPOWERS; j++)
			D_BSwi32(a_Stream, Player->powers[j]);
			
			// Ammo
		for (j = 0; j < NUMAMMO; j++)
		{
			D_BSwi32(a_Stream, Player->ammo[j]);
			D_BSwi32(a_Stream, Player->maxammo[j]);
		}
			
			// Weapons
		for (j = 0; j < NUMWEAPONS; j++)
			D_BSwu8(a_Stream, Player->weaponowned[j]);
			
			// Frags
		for (j = 0; j < MAXPLAYERS; j++)
			D_BSwu32(a_Stream, Player->frags[j]);
			
			// Inventory Slots
		for (j = 0; j < NUMINVENTORYSLOTS; j++)
		{
			D_BSwu8(a_Stream, Player->inventory[j].type);
			D_BSwu8(a_Stream, Player->inventory[j].count);
		}
		
		// Save psprites
		for (j = 0; j < NUMPSPRITES; j++)
		{
			D_BSwi32(a_Stream, Player->psprites[j].tics);
			D_BSwi32(a_Stream, Player->psprites[j].sx);
			D_BSwi32(a_Stream, Player->psprites[j].sy);
			
			// Write current state
			if (!Player->psprites[j].state)
				D_BSwu8(a_Stream, 'N');
			else
			{
				D_BSwu8(a_Stream, 'S');
				D_BSwu32(a_Stream, Player->psprites[j].state->FrameID);
				D_BSwu32(a_Stream, Player->psprites[j].state->ObjectID);
				D_BSwu32(a_Stream, Player->psprites[j].state->Marker);
				D_BSwu32(a_Stream, Player->psprites[j].state->SpriteID);
				D_BSwu32(a_Stream, Player->psprites[j].state->DehackEdID);
			}
		}
		
		// Save camera
		D_BSwp(a_Stream, Player->camera.mo);
		D_BSwu8(a_Stream, Player->camera.chase);
		D_BSwu32(a_Stream, Player->camera.aiming);
		D_BSwu32(a_Stream, Player->camera.startangle);
		D_BSwi32(a_Stream, Player->camera.fixedcolormap);
		D_BSwi32(a_Stream, Player->camera.viewheight);
	}
	
	/* Record Block */
	D_BSRecordBlock(a_Stream);
}

/* PS_SGBS_DumpMapThing() -- Dumps a map thing (occurs alot) */
void PS_SGBS_DumpMapThing(D_BS_t* const a_Stream, mapthing_t* const MapThing)
{
	D_BSwp(a_Stream, MapThing);
	D_BSwi16(a_Stream, MapThing->x);
	D_BSwi16(a_Stream, MapThing->y);
	D_BSwi16(a_Stream, MapThing->z);
	D_BSwi16(a_Stream, MapThing->angle);
	D_BSwi16(a_Stream, MapThing->type);
	D_BSwi16(a_Stream, MapThing->options);
	D_BSwp(a_Stream, MapThing->mobj);
	D_BSwu8(a_Stream, MapThing->IsHexen);
	D_BSwi16(a_Stream, MapThing->HeightOffset);
	D_BSwu16(a_Stream, MapThing->ID);
	D_BSwu8(a_Stream, MapThing->Special);
	D_BSwu8(a_Stream, MapThing->Args[0]);
	D_BSwu8(a_Stream, MapThing->Args[1]);
	D_BSwu8(a_Stream, MapThing->Args[2]);
	D_BSwu8(a_Stream, MapThing->Args[3]);
	D_BSwu8(a_Stream, MapThing->Args[4]);
	D_BSwu32(a_Stream, MapThing->MoType);
	D_BSwu8(a_Stream, MapThing->MarkedWeapon);
}

/* P_SGBS_MapData() -- Write All Map Data */
// Some map data cannot be moved
// Left: AFHKMQWXY
void P_SGBS_MapData(D_BS_t* const a_Stream)
{
	size_t i, j;
	mapthing_t* MapThing;
	sector_t* Sector;
	
	/* Vertexes */
	// Begin
	D_BSBaseBlock(a_Stream, "SGMV");
	
	// Put vertexes
	D_BSwu32(a_Stream, numvertexes);
	for (i = 0; i < numvertexes; i++)
	{
		D_BSwi32(a_Stream, vertexes[i].x);
		D_BSwi32(a_Stream, vertexes[i].y);
	}
	
	// End
	D_BSRecordBlock(a_Stream);
	
	/* Sectors */
	// Begin
	D_BSBaseBlock(a_Stream, "SGMS");
	
	// Put sectors
	D_BSwu32(a_Stream, numsectors);
	for (i = 0; i < numsectors; i++)
	{
		// Get Current
		Sector = &sectors[i];
		
		// Dump
		D_BSwi32(a_Stream, Sector->floorheight);
		D_BSwi32(a_Stream, Sector->ceilingheight);
		D_BSwi32(a_Stream, Sector->nexttag);
		D_BSwi32(a_Stream, Sector->firsttag);
		D_BSwi32(a_Stream, Sector->validcount);
		D_BSwi32(a_Stream, Sector->stairlock);
		D_BSwi32(a_Stream, Sector->prevsec);
		D_BSwi32(a_Stream, Sector->nextsec);
		D_BSwi32(a_Stream, Sector->floor_xoffs);
		D_BSwi32(a_Stream, Sector->floor_yoffs);
		D_BSwi32(a_Stream, Sector->ceiling_xoffs);
		D_BSwi32(a_Stream, Sector->ceiling_yoffs);
		D_BSwi32(a_Stream, Sector->heightsec);
		D_BSwi32(a_Stream, Sector->altheightsec);
		D_BSwi32(a_Stream, Sector->floorlightsec);
		D_BSwi32(a_Stream, Sector->ceilinglightsec);
		D_BSwi32(a_Stream, Sector->teamstartsec);
		D_BSwi32(a_Stream, Sector->bottommap);
		D_BSwi32(a_Stream, Sector->midmap);
		D_BSwi32(a_Stream, Sector->topmap);
		D_BSwi32(a_Stream, Sector->validsort);
		D_BSwi32(a_Stream, FLOAT_TO_FIXED(Sector->lineoutLength));
		
		D_BSwu32(a_Stream, Sector->special);
		D_BSwu32(a_Stream, Sector->oldspecial);
		//D_BSwu32(a_Stream, Sector->xxxxxxxxx);
		//D_BSwu32(a_Stream, Sector->xxxxxxxxx);
		//D_BSwu32(a_Stream, Sector->xxxxxxxxx);
		
		D_BSwi16(a_Stream, Sector->floorpic);
		D_BSwi16(a_Stream, Sector->ceilingpic);
		D_BSwi16(a_Stream, Sector->lightlevel);
		D_BSwi16(a_Stream, Sector->tag);
		D_BSwi16(a_Stream, Sector->soundtraversed);
		D_BSwi16(a_Stream, Sector->floortype);
		
		// Arrays
		for (j = 0; j < 4; j++)
		{
			D_BSwi32(a_Stream, Sector->blockbox[j]);
			D_BSwi32(a_Stream, Sector->BBox[j]);
		}
		
		// Variable
		
		// Pointers
		D_BSwp(a_Stream, Sector->soundtarget);

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
	D_BSRecordBlock(a_Stream);
	
	/* SideDefs */
	// Begin
	D_BSBaseBlock(a_Stream, "SGMI");
	
	// End
	D_BSRecordBlock(a_Stream);
	
	/* LineDefs */
	// Begin
	D_BSBaseBlock(a_Stream, "SGML");
	
	// End
	D_BSRecordBlock(a_Stream);
	
	/* SubSectors */
	// Begin
	D_BSBaseBlock(a_Stream, "SGMU");
	
	// End
	D_BSRecordBlock(a_Stream);
	
	/* Nodes */
	// Begin
	D_BSBaseBlock(a_Stream, "SGMN");
	
	// End
	D_BSRecordBlock(a_Stream);
	
	/* Segs */
	// Begin
	D_BSBaseBlock(a_Stream, "SGMG");
	
	// End
	D_BSRecordBlock(a_Stream);
	
	/* Block Map */
	// extern long* blockmaplump;		// offsets in blockmap are from here
	// extern long* blockmap;			// Big blockmap SSNTails
	// extern int bmapwidth;
	// extern int bmapheight;			// in mapblocks
	// extern fixed_t bmaporgx;
	// extern fixed_t bmaporgy;		// origin of block map
	// extern mobj_t** blocklinks;		// for thing chains
	
	// Begin
	D_BSBaseBlock(a_Stream, "SGMB");
	
	// End
	D_BSRecordBlock(a_Stream);
	
	/* Reject */
	// extern uint8_t* rejectmatrix;	// for fast sight rejection
	
	// Begin
	D_BSBaseBlock(a_Stream, "SGMJ");
	
	// End
	D_BSRecordBlock(a_Stream);
	
	/* Things */
	//extern int nummapthings;
	//extern mapthing_t* mapthings;
	
	// Begin
	D_BSBaseBlock(a_Stream, "SGMT");
	
	// Record all things
	D_BSwu32(a_Stream, nummapthings);
	for (i = 0; i < nummapthings; i++)
	{
		// Get Current
		MapThing = &mapthings[i];
		
		// Write Out
		PS_SGBS_DumpMapThing(a_Stream, MapThing);
	}
	
	// End
	D_BSRecordBlock(a_Stream);
	
	/* Touching Sector Lists */
	//msecnode_t* sector_list = NULL;
	
	// Begin
	D_BSBaseBlock(a_Stream, "SGMZ");
	
	// End
	D_BSRecordBlock(a_Stream);
	
	/* Active Plats */
	// platlist_t* activeplats;
	
	// Begin
	D_BSBaseBlock(a_Stream, "SGMP");
	
	// End
	D_BSRecordBlock(a_Stream);
	
	/* Active Ceilings */
	// ceilinglist_t* activeceilings;
	
	// Begin
	D_BSBaseBlock(a_Stream, "SGME");
	
	// End
	D_BSRecordBlock(a_Stream);
	
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
	D_BSBaseBlock(a_Stream, "SGMR");
	
	// Player Starts
	D_BSwu32(a_Stream, MAXPLAYERS);
	for (i = 0; i < MAXPLAYERS; i++)
		D_BSwu32(a_Stream, playerstarts[i] - mapthings);
	
	// End
	D_BSRecordBlock(a_Stream);
}

void P_MobjNullThinker(mobj_t* mobj);

/* P_SGBS_Thinkers() -- Thinkers */
// extern thinker_t thinkercap;
void P_SGBS_Thinkers(D_BS_t* const a_Stream)
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
		D_BSBaseBlock(a_Stream, "SGTH");
		
		// Print Thinker Current Pointer ID
		D_BSwp(a_Stream, CurThinker);
		
		// P_MobjNullThinker(mobj_t* mobj) || P_MobjThinker(mobj_t* mobj)
		if (CurThinker->function.acv == P_MobjNullThinker ||
				CurThinker->function.acv == P_MobjThinker)
		{
			// Thinker Header
			D_BSwu8(a_Stream, 'M');
			if (CurThinker->function.acv == P_MobjNullThinker)
				D_BSwu8(a_Stream, 'N');
			else
				D_BSwu8(a_Stream, 'O');
			
			// Get Thinker
			Mobj = (mobj_t*)CurThinker;
			
			// Dump Info
			D_BSwi32(a_Stream, Mobj->x);
			D_BSwi32(a_Stream, Mobj->y);
			D_BSwi32(a_Stream, Mobj->z);
			D_BSwu32(a_Stream, Mobj->angle);
			D_BSwi32(a_Stream, Mobj->sprite);
			D_BSwi32(a_Stream, Mobj->frame);
			D_BSwi32(a_Stream, Mobj->skin);
			D_BSwi32(a_Stream, Mobj->sprite);
			D_BSwi32(a_Stream, Mobj->floorz);
			D_BSwi32(a_Stream, Mobj->ceilingz);
			D_BSwi32(a_Stream, Mobj->height);
			D_BSwi32(a_Stream, Mobj->radius);
			D_BSwi32(a_Stream, Mobj->momx);
			D_BSwi32(a_Stream, Mobj->momy);
			D_BSwi32(a_Stream, Mobj->momz);
			D_BSwi32(a_Stream, Mobj->type);
			D_BSwi32(a_Stream, Mobj->tics);
			D_BSwi32(a_Stream, Mobj->flags);
			D_BSwi32(a_Stream, Mobj->eflags);
			D_BSwi32(a_Stream, Mobj->flags2);
			D_BSwi32(a_Stream, Mobj->special1);
			D_BSwi32(a_Stream, Mobj->special2);
			D_BSwi32(a_Stream, Mobj->health);
			D_BSwi32(a_Stream, Mobj->movedir);
			D_BSwi32(a_Stream, Mobj->movecount);
			D_BSwi32(a_Stream, Mobj->reactiontime);
			D_BSwi32(a_Stream, Mobj->threshold);
			D_BSwi32(a_Stream, Mobj->lastlook);
			D_BSwi32(a_Stream, Mobj->friction);
			D_BSwi32(a_Stream, Mobj->movefactor);
			D_BSwi32(a_Stream, Mobj->dropped_ammo_count);
			D_BSwu32(a_Stream, Mobj->XFlagsA);
			D_BSwu32(a_Stream, Mobj->XFlagsB);
			D_BSwu32(a_Stream, Mobj->XFlagsC);
			D_BSwu32(a_Stream, Mobj->XFlagsD);
			D_BSwu32(a_Stream, Mobj->RXAttackAttackType);
			D_BSwu32(a_Stream, Mobj->RXShotWithWeapon);
			D_BSwu8(a_Stream, Mobj->RemoveMo);
			D_BSwu32(a_Stream, Mobj->RemType);
			D_BSwi32(a_Stream, Mobj->MaxZObtained);
			D_BSwi32(a_Stream, Mobj->SkinTeamColor);
			D_BSwi32(a_Stream, Mobj->NoiseThinker.Pitch);
			D_BSwi32(a_Stream, Mobj->NoiseThinker.Volume);
			
			// Map Data Related
			D_BSwi32(a_Stream, Mobj->player - players);
			D_BSwu32(a_Stream, Mobj->subsector - subsectors);
			D_BSwu32(a_Stream, Mobj->spawnpoint - mapthings);
			
			// Spawn Point is probably virtualzed
			if (Mobj->spawnpoint)
			{
				MapThing = Mobj->spawnpoint;
				D_BSwu8(a_Stream, 'T');
				
				PS_SGBS_DumpMapThing(a_Stream, MapThing);
			}
			else
				D_BSwu8(a_Stream, 'X');
			
			// Pointer Links
			D_BSwp(a_Stream, Mobj->snext);
			D_BSwp(a_Stream, Mobj->sprev);
			D_BSwp(a_Stream, Mobj->bnext);
			D_BSwp(a_Stream, Mobj->bprev);
			D_BSwp(a_Stream, Mobj->target);
			D_BSwp(a_Stream, Mobj->player);
			D_BSwp(a_Stream, Mobj->tracer);
			D_BSwp(a_Stream, Mobj->ChildFloor);
			D_BSwp(a_Stream, Mobj->touching_sectorlist);
			D_BSwp(a_Stream, Mobj->info);
			
			// Info
			D_BSws(a_Stream, (Mobj->info ? Mobj->info->RClassName : "NoRCN"));
			
			// State
			if (Mobj->state)
			{
				D_BSwu8(a_Stream, 'S');
				D_BSwu32(a_Stream, Mobj->state->FrameID);
				D_BSwu32(a_Stream, Mobj->state->ObjectID);
				D_BSwu32(a_Stream, Mobj->state->Marker);
				D_BSwu32(a_Stream, Mobj->state->SpriteID);
				D_BSwu32(a_Stream, Mobj->state->DehackEdID);
			}
			else
				D_BSwu8(a_Stream, 'X');
			
			// Variable Info
				// ReMooD Extended Flags
			for (i = 0; i < NUMINFORXFIELDS; i++)
				D_BSwu32(a_Stream, Mobj->RXFlags[i]);
			
				// Map Objects On
			for (i = 0; i < 2; i++)
			{
				D_BSwu32(a_Stream, Mobj->MoOnCount[i]);
				for (j = 0; j < Mobj->MoOnCount[i]; j++)
					D_BSwp(a_Stream, Mobj->MoOn[i][j]);
			}
		}
		
		// T_FireFlicker(fireflicker_t* flick)
		else if (CurThinker->function.acv == T_FireFlicker)
		{
			// Thinker Header
			D_BSwu8(a_Stream, 'F');
			D_BSwu8(a_Stream, 'F');
			
			// Get Thinker
			FireFlicker = (fireflicker_t*)CurThinker;
			
			// Dump Info
			D_BSwu32(a_Stream, FireFlicker->sector - sectors);
			D_BSwi32(a_Stream, FireFlicker->count);
			D_BSwi32(a_Stream, FireFlicker->maxlight);
			D_BSwi32(a_Stream, FireFlicker->minlight);
		}
		
		// T_Friction(friction_t* f)
		else if (CurThinker->function.acv == T_Friction)
		{
			// Thinker Header
			D_BSwu8(a_Stream, 'F');
			D_BSwu8(a_Stream, 'R');
			
			// Get Thinker
			Friction = (friction_t*)CurThinker;
			
			// Dump Info
			D_BSwi32(a_Stream, Friction->friction);
			D_BSwi32(a_Stream, Friction->movefactor);
			D_BSwi32(a_Stream, Friction->affectee);
		}
		
		// T_Glow(glow_t* g)
		else if (CurThinker->function.acv == T_Glow)
		{
			// Thinker Header
			D_BSwu8(a_Stream, 'G');
			D_BSwu8(a_Stream, 'L');
			
			// Get Thinker
			Glow = (glow_t*)CurThinker;
			
			// Dump Info
			D_BSwu32(a_Stream, Glow->sector - sectors);
			D_BSwi32(a_Stream, Glow->minlight);
			D_BSwi32(a_Stream, Glow->maxlight);
			D_BSwi32(a_Stream, Glow->direction);
		}
		
		// T_LightFade(lightlevel_t* ll)
		else if (CurThinker->function.acv == T_LightFade)
		{
			// Thinker Header
			D_BSwu8(a_Stream, 'L');
			D_BSwu8(a_Stream, 'A');
			
			// Get Thinker
			LightFade = (lightlevel_t*)CurThinker;
			
			// Dump Info
			D_BSwu32(a_Stream, LightFade->sector - sectors);
			D_BSwi32(a_Stream, LightFade->destlevel);
			D_BSwi32(a_Stream, LightFade->speed);
		}
		
		// T_LightFlash(lightflash_t* flash)
		else if (CurThinker->function.acv == T_LightFlash)
		{
			// Thinker Header
			D_BSwu8(a_Stream, 'L');
			D_BSwu8(a_Stream, 'F');
			
			// Get Thinker
			LightFlash = (lightflash_t*)CurThinker;
			
			// Dump Data
			D_BSwu32(a_Stream, LightFlash->sector - sectors);
			D_BSwi32(a_Stream, LightFlash->count);
			D_BSwi32(a_Stream, LightFlash->maxlight);
			D_BSwi32(a_Stream, LightFlash->minlight);
			D_BSwi32(a_Stream, LightFlash->maxtime);
			D_BSwi32(a_Stream, LightFlash->mintime);
		}
		
		// T_MoveCeiling(ceiling_t* ceiling)
		else if (CurThinker->function.acv == T_MoveCeiling)
		{
			// Thinker Header
			D_BSwu8(a_Stream, 'M');
			D_BSwu8(a_Stream, 'C');
			
			// Get Thinker
			Ceiling = (ceiling_t*)CurThinker;
			
			// Dump Info
			D_BSwu8(a_Stream, Ceiling->type);
			D_BSwu8(a_Stream, Ceiling->crush);
			D_BSwi32(a_Stream, Ceiling->bottomheight);
			D_BSwi32(a_Stream, Ceiling->topheight);
			D_BSwi32(a_Stream, Ceiling->speed);
			D_BSwi32(a_Stream, Ceiling->oldspeed);
			D_BSwi32(a_Stream, Ceiling->newspecial);
			D_BSwi32(a_Stream, Ceiling->oldspecial);
			D_BSwi32(a_Stream, Ceiling->texture);
			D_BSwi32(a_Stream, Ceiling->direction);
			D_BSwi32(a_Stream, Ceiling->tag);
			D_BSwi32(a_Stream, Ceiling->olddirection);
			D_BSwu32(a_Stream, Ceiling->sector - sectors);
			D_BSwp(a_Stream, Ceiling->list);
		}
		
		// T_MoveElevator(elevator_t* elevator)
		else if (CurThinker->function.acv == T_MoveElevator)
		{
			// Thinker Header
			D_BSwu8(a_Stream, 'M');
			D_BSwu8(a_Stream, 'E');
			
			// Get Thinker
			Elevator = (elevator_t*)CurThinker;
			
			// Dump Info
			D_BSwu8(a_Stream, Elevator->type);
			D_BSwu32(a_Stream, Elevator->sector - sectors);
			D_BSwi32(a_Stream, Elevator->direction);
			D_BSwi32(a_Stream, Elevator->floordestheight);
			D_BSwi32(a_Stream, Elevator->ceilingdestheight);
			D_BSwi32(a_Stream, Elevator->speed);
		}
		
		// T_MoveFloor(floormove_t* floor)
		else if (CurThinker->function.acv == T_MoveFloor)
		{
			// Thinker Header
			D_BSwu8(a_Stream, 'M');
			D_BSwu8(a_Stream, 'F');
			
			// Get Thinker
			FloorMove = (floormove_t*)CurThinker;
			
			// DumpInfo
			D_BSwu8(a_Stream, FloorMove->type);
			D_BSwu8(a_Stream, FloorMove->crush);
			D_BSwu32(a_Stream, FloorMove->sector - sectors);
			D_BSwi32(a_Stream, FloorMove->direction);
			D_BSwi32(a_Stream, FloorMove->newspecial);
			D_BSwi32(a_Stream, FloorMove->oldspecial);
			D_BSwi32(a_Stream, FloorMove->texture);
			D_BSwi32(a_Stream, FloorMove->floordestheight);
			D_BSwi32(a_Stream, FloorMove->speed);
		}
		
		// T_PlatRaise(plat_t* plat)
		else if (CurThinker->function.acv == T_PlatRaise)
		{
			// Thinker Header
			D_BSwu8(a_Stream, 'P');
			D_BSwu8(a_Stream, 'R');
			
			// Get Thinker
			Plat = (plat_t*)CurThinker;
			
			// Dump Info
			D_BSwu32(a_Stream, Plat->sector - sectors);
			D_BSwi32(a_Stream, Plat->speed);
			D_BSwi32(a_Stream, Plat->low);
			D_BSwi32(a_Stream, Plat->high);
			D_BSwi32(a_Stream, Plat->wait);
			D_BSwi32(a_Stream, Plat->count);
			D_BSwi32(a_Stream, Plat->tag);
			D_BSwu8(a_Stream, Plat->status);
			D_BSwu8(a_Stream, Plat->oldstatus);
			D_BSwu8(a_Stream, Plat->crush);
			D_BSwu8(a_Stream, Plat->type);
			D_BSwp(a_Stream, Plat->list);
		}
		
		// T_Pusher(pusher_t* p)
		else if (CurThinker->function.acv == T_Pusher)
		{
			// Thinker Header
			D_BSwu8(a_Stream, 'P');
			D_BSwu8(a_Stream, 'U');
			
			// Get Thinker
			Pusher = (pusher_t*)CurThinker;
			
			// Dump Info
			D_BSwu8(a_Stream, Pusher->type);
			D_BSwp(a_Stream, Pusher->source);
			D_BSwi32(a_Stream, Pusher->x_mag);
			D_BSwi32(a_Stream, Pusher->y_mag);
			D_BSwi32(a_Stream, Pusher->magnitude);
			D_BSwi32(a_Stream, Pusher->radius);
			D_BSwi32(a_Stream, Pusher->x);
			D_BSwi32(a_Stream, Pusher->y);
			D_BSwi32(a_Stream, Pusher->affectee);
		}
		
		// T_Scroll(scroll_t* s)
		else if (CurThinker->function.acv == T_Scroll)
		{
			// Thinker Header
			D_BSwu8(a_Stream, 'S');
			D_BSwu8(a_Stream, 'C');
			
			// Get Thinker
			Scroll = (scroll_t*)CurThinker;
			
			// Dump Info
			D_BSwi32(a_Stream, Scroll->dx);
			D_BSwi32(a_Stream, Scroll->dy);
			D_BSwi32(a_Stream, Scroll->affectee);
			D_BSwi32(a_Stream, Scroll->control);
			D_BSwi32(a_Stream, Scroll->last_height);
			D_BSwi32(a_Stream, Scroll->vdx);
			D_BSwi32(a_Stream, Scroll->vdy);
			D_BSwi32(a_Stream, Scroll->accel);
			D_BSwu8(a_Stream, Scroll->type);
		}
		
		// T_StrobeFlash(strobe_t* flash)
		else if (CurThinker->function.acv == T_StrobeFlash)
		{
			// Thinker Header
			D_BSwu8(a_Stream, 'S');
			D_BSwu8(a_Stream, 'F');
			
			// Get Thinker
			Strobe = (strobe_t*)CurThinker;
			
			// Dump Info
			D_BSwu32(a_Stream, Strobe->sector - sectors);
			D_BSwi32(a_Stream, Strobe->count);
			D_BSwi32(a_Stream, Strobe->minlight);
			D_BSwi32(a_Stream, Strobe->maxlight);
			D_BSwi32(a_Stream, Strobe->darktime);
			D_BSwi32(a_Stream, Strobe->brighttime);
		}
		
		// T_VerticalDoor(vldoor_t* door)
		else if (CurThinker->function.acv == T_VerticalDoor)
		{
			// Thinker Header
			D_BSwu8(a_Stream, 'V');
			D_BSwu8(a_Stream, 'D');
			
			// Get Thinker
			VLDoor = (vldoor_t*)CurThinker;
			
			// Dump Info
			D_BSwu8(a_Stream, VLDoor->type);
			D_BSwu32(a_Stream, VLDoor->sector - sectors);
			D_BSwi32(a_Stream, VLDoor->topheight);
			D_BSwi32(a_Stream, VLDoor->speed);
			D_BSwu8(a_Stream, VLDoor->direction);
			D_BSwi32(a_Stream, VLDoor->topwait);
			D_BSwi32(a_Stream, VLDoor->topcountdown);
			D_BSwu32(a_Stream, VLDoor->line - lines);
		}
		
		// End
		D_BSRecordBlock(a_Stream);
	}
}

/* P_SGBS_State() -- Game State */
void P_SGBS_State(D_BS_t* const a_Stream)
{
	size_t i;
	P_XGSVariable_t* Vars;
	
	/* Something */
	// Begin
	D_BSBaseBlock(a_Stream, "SGZA");
	
	// End
	D_BSRecordBlock(a_Stream);
	
	/* Game State Info */
	// Begin
	D_BSBaseBlock(a_Stream, "SGZS");
	
	// Write Times
	D_BSwu32(a_Stream, gametic);
	//D_BSwu32(a_Stream, D_SyncNetMapTime());
	//D_BSwu32(a_Stream, D_SyncNetRealTime());
	
	// Write Game state
	D_BSwu8(a_Stream, gamestate);
	D_BSwu8(a_Stream, demorecording);
	D_BSwu8(a_Stream, demoplayback);
	D_BSwu8(a_Stream, multiplayer);
	
	// Write Current Level name
	D_BSws(a_Stream, g_CurrentLevelInfo->LumpName);
	
	// End
	D_BSRecordBlock(a_Stream);
	
	/* Game Setting Variables */
	// Begin
	D_BSBaseBlock(a_Stream, "SGZV");
	
	// Go through all variables
	for (i = 0; i < PEXGSNUMBITIDS; i++)
	{
		// Get Variable
		Vars = P_XGSVarForBit(i);
		
		// No Var?
		if (!Vars)
			continue;
		
		// Write ID and Name
		D_BSwu32(a_Stream, Vars->BitID);
		D_BSws(a_Stream, Vars->Name);
		
		// Write Value
		if (Vars->WasSet)
			D_BSwi32(a_Stream, Vars->ActualVal);
		else
			D_BSwi32(a_Stream, Vars->DefaultVal);
	}
	
	// End
	D_BSRecordBlock(a_Stream);
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
						if (g_Splits[i].Console == i)
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
				
				mobj->info = (PI_mobj_t*) next;	// temporarely, set when leave this function
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

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


/* P_SGDXDataSpec_t -- Save Game Data Extended Specification */
typedef struct P_SGDXDataSpec_s
{
	bool_t AtEnd;								// Is At End?
	ptrdiff_t OffSet;							// Offset to native
	size_t SizeOf;								// Size of native
	P_SGBWTypeC_t CType;						// C Type
	P_SGBWTypeRec_t RType;						// Record Type
} P_SGDXDataSpec_t;

/* P_SGDXTypeIO_t -- Type I/O */
typedef struct P_SGDXTypeIO_s
{
	const char* VarName;						// Name of variable
	P_SGBWTypeC_t CType;						// C Type
	bool_t (*IOFunc)(D_BS_t* const a_Stream, const bool_t a_Load, const P_SGBWTypeRec_t a_RecType, const P_SGBWTypeC_t a_CType, void* const a_ValPtr, const size_t a_ValSize, int32_t* const a_Copy);
} P_SGDXTypeIO_t;

// __SPEC -- Member Info
#define __SPEC(s,m,ct,rt) {false, offsetof(s,m), sizeof((*(((s*)0))).m), PSTC_##ct, PSRC_##rt}
#define __ENDSPEC {true}

/*****************************************************************************/

/*** I/O ***/

#if 0
	PSRC_STRING,								// String
	PSRC_POINTER,								// Pointer
	PSRC_INT8,									// Int8
	PSRC_INT16,									// Int16
	PSRC_INT32,									// Int32
	PSRC_UINT8,									// UInt8
	PSRC_UINT16,								// UInt16
	PSRC_UINT32,								// UInt32
#endif


/* PRWS_DRPointer() -- Pointer to something */
static bool_t PS_SGDXPointerHandler(D_BS_t* const a_Stream, const bool_t a_Load, const P_SGBWTypeRec_t a_RecType, const P_SGBWTypeC_t a_CType, void* const a_ValPtr, const size_t a_ValSize, int32_t* const a_Copy)
{
	uint64_t pID;
	
	/* Only accept pointer inputs/outputs */
	if (a_RecType != PSRC_POINTER)
	{
		if (devparm)
			CONL_PrintF("WARNING: RPointer not as ptr (%c vs %c)\n", a_RecType, PSRC_POINTER);
		return false;
	}
	
	/* If Saving, Dump Pointer */
	if (!a_Load)
		D_BSwp(a_Stream, *((void**)a_ValPtr));
	
	/* If Loading, Get pointer and mark ref */
	else
	{
		pID = D_BSrp(a_Stream);
		PLGS_DeRef(pID, ((void**)a_ValPtr));
		*((void**)a_ValPtr) = NULL;	// FIXME: See if this causes problems?
	}
	
	/* Success */
	return true;
}

#define __REMOOD_SGDXIHM(xxnamexx) PS_SGDXIntHandler_##xxnamexx
#define __REMOOD_SGDXIH(xxnamexx,xxnativexx) static bool_t PS_SGDXIntHandler_##xxnamexx(D_BS_t* const a_Stream, const bool_t a_Load, const P_SGBWTypeRec_t a_RecType, const P_SGBWTypeC_t a_CType, void* const a_ValPtr, const size_t a_ValSize, int32_t* const a_Copy)\
{\
	if (a_Load)\
	{\
		switch (a_RecType)\
		{\
			case PSRC_INT8: *((xxnativexx*)a_ValPtr) = D_BSri8(a_Stream); break;\
			case PSRC_INT16: *((xxnativexx*)a_ValPtr) = D_BSri16(a_Stream); break;\
			case PSRC_INT32: *((xxnativexx*)a_ValPtr) = D_BSri32(a_Stream); break;\
			case PSRC_UINT8: *((xxnativexx*)a_ValPtr) = D_BSru8(a_Stream); break;\
			case PSRC_UINT16: *((xxnativexx*)a_ValPtr) = D_BSru16(a_Stream); break;\
			case PSRC_UINT32: *((xxnativexx*)a_ValPtr) = D_BSru32(a_Stream); break;\
			default: return false;\
		}\
		\
		if (a_Copy)\
			*a_Copy = *((xxnativexx*)a_ValPtr);\
	}\
	else\
	{\
		switch (a_RecType)\
		{\
			case PSRC_INT8: D_BSwi8(a_Stream, *((xxnativexx*)a_ValPtr)); break;\
			case PSRC_INT16: D_BSwi16(a_Stream, *((xxnativexx*)a_ValPtr)); break;\
			case PSRC_INT32: D_BSwi32(a_Stream, *((xxnativexx*)a_ValPtr)); break;\
			case PSRC_UINT8: D_BSwu8(a_Stream, *((xxnativexx*)a_ValPtr)); break;\
			case PSRC_UINT16: D_BSwu16(a_Stream, *((xxnativexx*)a_ValPtr)); break;\
			case PSRC_UINT32: D_BSwu32(a_Stream, *((xxnativexx*)a_ValPtr)); break;\
			default: return false;\
		}\
		\
		if (a_Copy)\
			*a_Copy = *((xxnativexx*)a_ValPtr);\
	}\
	return true;\
}

__REMOOD_SGDXIH(char,char);
__REMOOD_SGDXIH(schar,signed char);
__REMOOD_SGDXIH(uchar,unsigned char);

__REMOOD_SGDXIH(ssi,signed short int);
__REMOOD_SGDXIH(si,signed int);
__REMOOD_SGDXIH(sli,signed long int);

__REMOOD_SGDXIH(usi,unsigned short int);
__REMOOD_SGDXIH(ui,unsigned int);
__REMOOD_SGDXIH(uli,unsigned long int);

__REMOOD_SGDXIH(fixedt,fixed_t);
__REMOOD_SGDXIH(boolt,bool_t);
__REMOOD_SGDXIH(tict,tic_t);
__REMOOD_SGDXIH(anglet,angle_t);

__REMOOD_SGDXIH(int8,int8_t);
__REMOOD_SGDXIH(int16,int16_t);
__REMOOD_SGDXIH(int32,int32_t);
__REMOOD_SGDXIH(int64,int64_t);
__REMOOD_SGDXIH(uint8,uint8_t);
__REMOOD_SGDXIH(uint16,uint16_t);
__REMOOD_SGDXIH(uint32,uint32_t);
__REMOOD_SGDXIH(uint64,uint64_t);

#undef __REMOOD_SGDXIH

#define __IODEF(nt,ctc,func) {#ctc, nt, func}

// c_IOTable -- I/O Table
static const P_SGDXTypeIO_t c_IOTable[NUMPSTCS] =
{
	__IODEF(PSTC_CHAR,char,__REMOOD_SGDXIHM(char)),
	__IODEF(PSTC_SCHAR,signed char,__REMOOD_SGDXIHM(schar)),
	__IODEF(PSTC_SHORT,signed short int,__REMOOD_SGDXIHM(ssi)),
	__IODEF(PSTC_INT,signed int,__REMOOD_SGDXIHM(si)),
	__IODEF(PSTC_LONG,signed long int,__REMOOD_SGDXIHM(sli)),
	__IODEF(PSTC_UCHAR,unsigned char,__REMOOD_SGDXIHM(uchar)),
	__IODEF(PSTC_USHORT,unsigned short int,__REMOOD_SGDXIHM(usi)),
	__IODEF(PSTC_UINT,unsigned int,__REMOOD_SGDXIHM(ui)),
	__IODEF(PSTC_ULONG,unsigned long int,__REMOOD_SGDXIHM(uli)),
	__IODEF(PSTC_FIXEDT,fixed_t,__REMOOD_SGDXIHM(fixedt)),
	__IODEF(PSTC_BOOLT,bool_t,__REMOOD_SGDXIHM(boolt)),
	__IODEF(PSTC_TICT,tic_t,__REMOOD_SGDXIHM(tict)),
	__IODEF(PSTC_ANGLET,angle_t,__REMOOD_SGDXIHM(anglet)),
	__IODEF(PSTC_FLOAT,float,NULL),
	__IODEF(PSTC_DOUBLE,double,NULL),
	__IODEF(PSTC_POINTERTO,void*,PS_SGDXPointerHandler),
	__IODEF(PSTC_POINTERIS,void*,NULL),
	__IODEF(PSTC_POINTERISDIRECT,void*,NULL),
	__IODEF(PSTC_STRING,char*,NULL),
	__IODEF(PSTC_INT8,int8_t,__REMOOD_SGDXIHM(int8)),
	__IODEF(PSTC_INT16,int16_t,__REMOOD_SGDXIHM(int16)),
	__IODEF(PSTC_INT32,int32_t,__REMOOD_SGDXIHM(int32)),
	__IODEF(PSTC_INT64,int64_t,__REMOOD_SGDXIHM(int64)),
	__IODEF(PSTC_UINT8,uint8_t,__REMOOD_SGDXIHM(uint8)),
	__IODEF(PSTC_UINT16,uint16_t,__REMOOD_SGDXIHM(uint16)),
	__IODEF(PSTC_UINT32,uint32_t,__REMOOD_SGDXIHM(uint32)),
	__IODEF(PSTC_UINT64,uint64_t,__REMOOD_SGDXIHM(uint64)),
	__IODEF(PSTC_INTPTR,intptr_t,NULL),
	__IODEF(PSTC_UINTPTR,uintptr_t,NULL),
	__IODEF(PSTC_SIZET,size_t,NULL),
	__IODEF(PSTC_SSIZET,ssize_t,NULL),
};

#undef __IODEF
#undef __REMOOD_SGDXIHM

/*** SPECIFICATIONS ***/

// c_MapThingSpec -- Map thing specification
static const P_SGDXDataSpec_t c_MapThingSpec[] =
{
	__SPEC(mapthing_t,x,SHORT,INT32),
	__SPEC(mapthing_t,y,SHORT,INT32),
	__SPEC(mapthing_t,z,SHORT,INT32),
	__SPEC(mapthing_t,angle,SHORT,INT32),
	__SPEC(mapthing_t,type,SHORT,INT32),
	__SPEC(mapthing_t,options,SHORT,INT32),
	__SPEC(mapthing_t,mobj,POINTERTO,POINTER),
	__SPEC(mapthing_t,IsHexen,BOOLT,UINT8),
	__SPEC(mapthing_t,HeightOffset,INT16,INT16),
	__SPEC(mapthing_t,ID,UINT16,UINT16),
	__SPEC(mapthing_t,Special,UINT8,UINT8),
	__SPEC(mapthing_t,Args[0],UINT8,UINT8),
	__SPEC(mapthing_t,Args[1],UINT8,UINT8),
	__SPEC(mapthing_t,Args[2],UINT8,UINT8),
	__SPEC(mapthing_t,Args[3],UINT8,UINT8),
	__SPEC(mapthing_t,Args[4],UINT8,UINT8),
	__SPEC(mapthing_t,MoType,INT,UINT32),
	__SPEC(mapthing_t,MarkedWeapon,BOOLT,UINT8),
	
	__ENDSPEC,
};

// c_VertexSpec -- vertex_t info
static const P_SGDXDataSpec_t c_VertexSpec[] =
{
	__SPEC(vertex_t,x,FIXEDT,INT32),
	__SPEC(vertex_t,y,FIXEDT,INT32),
	
	__ENDSPEC,
};

// c_LineSpec -- line_t info
static const P_SGDXDataSpec_t c_LineSpec[] =
{
	
	__ENDSPEC,
};

// c_SideSpec -- side_t info
static const P_SGDXDataSpec_t c_SideSpec[] =
{
	
	__ENDSPEC,
};

// c_SectorSpec -- sector_t Info
static const P_SGDXDataSpec_t c_SectorSpec[] =
{
	__SPEC(sector_t,floorheight,FIXEDT,INT32),
	__SPEC(sector_t,ceilingheight,FIXEDT,INT32),
	__SPEC(sector_t,floorpic,SHORT,INT16),
	__SPEC(sector_t,ceilingpic,SHORT,INT16),
	__SPEC(sector_t,lightlevel,SHORT,INT16),
	__SPEC(sector_t,special,UINT32,UINT32),
	__SPEC(sector_t,oldspecial,UINT32,UINT32),
	__SPEC(sector_t,tag,SHORT,INT16),
	__SPEC(sector_t,nexttag,INT,INT32),
	__SPEC(sector_t,firsttag,INT,INT32),
	__SPEC(sector_t,soundtraversed,SHORT,INT16),
	__SPEC(sector_t,floortype,SHORT,INT16),
	__SPEC(sector_t,blockbox[0],INT,INT32),
	__SPEC(sector_t,blockbox[1],INT,INT32),
	__SPEC(sector_t,blockbox[2],INT,INT32),
	__SPEC(sector_t,blockbox[3],INT,INT32),
	__SPEC(sector_t,validcount,INT,INT32),
	__SPEC(sector_t,stairlock,INT,INT32),
	__SPEC(sector_t,prevsec,INT,INT32),
	__SPEC(sector_t,nextsec,INT,INT32),
	__SPEC(sector_t,floor_xoffs,FIXEDT,INT32),
	__SPEC(sector_t,floor_yoffs,FIXEDT,INT32),
	__SPEC(sector_t,ceiling_xoffs,FIXEDT,INT32),
	__SPEC(sector_t,ceiling_yoffs,FIXEDT,INT32),
	__SPEC(sector_t,heightsec,INT,INT32),
	__SPEC(sector_t,altheightsec,INT,INT32),
	__SPEC(sector_t,floorlightsec,INT,INT32),
	__SPEC(sector_t,ceilinglightsec,INT,INT32),
	__SPEC(sector_t,teamstartsec,INT,INT32),
	__SPEC(sector_t,bottommap,INT,INT32),
	__SPEC(sector_t,midmap,INT,INT32),
	__SPEC(sector_t,topmap,INT,INT32),

#if 0
	struct msecnode_s* touching_thinglist;	// phares 3/14/98
	int linecount;
	struct line_s** lines;		// [linecount] size
	ffloor_t* ffloors;
	int* attached;
	int numattached;
	bool_t LLSelf;								// True if lightlist is Z_Malloc()
	lightlist_t* lightlist;
	int numlights;
	bool_t moved;
	int validsort;				//if == validsort allready been sorted
	bool_t added;
	extracolormap_t* extra_colormap;
	bool_t pseudoSector;
	bool_t virtualFloor;
	fixed_t virtualFloorheight;
	bool_t virtualCeiling;
	fixed_t virtualCeilingheight;
	linechain_t* sectorLines;
	struct sector_s** stackList;
	double lineoutLength;
	char* FloorTexture;							// Name of floor texture
	char* CeilingTexture;						// Name of ceiling texture
	fixed_t BBox[4];							// Sector bounding box
	size_t SoundSecRef;							// Reference to sound sector
	struct sector_s** Adj;						// Adjacent sectors
	size_t NumAdj;								// Number of adjacent sectors
	mobj_t* thinglist;
	mobj_t* soundtarget;
	S_NoiseThinker_t soundorg;
	void* floordata;			// make thinkers on
	void* ceilingdata;			// floors, ceilings, lighting,
	void* lightingdata;			// independent of one another
#endif
	
	__ENDSPEC,
};

/*** FUNCTIONS ***/

/* PS_SGDXReadWriteData() -- Read/Write Data */
static bool_t PS_SGDXReadWriteData(D_BS_t* const a_Stream, const bool_t a_Load, void* const a_Ptr, const size_t a_Size, const P_SGBWTypeC_t a_CType, const P_SGBWTypeRec_t a_RType, int32_t* const a_Copy)
{
	P_SGDXTypeIO_t* IO;
	
	/* Get IO */
	IO = &c_IOTable[a_CType];
	
	//if (devparm)
	//	fprintf(stderr, "%s %2u %2u (%2u)\n", (a_Load ? "Read" : "Write"), (unsigned)a_CType, (unsigned)a_RType, (unsigned)a_Size);
	
	/* Use Function */
	if (IO->IOFunc)
		return IO->IOFunc(a_Stream, a_Load, a_RType, a_CType, a_Ptr, a_Size, a_Copy);
	
	/* Nothing */
	return false;
}


/* P_SGBSGDXReadStr() -- Read string and possibly Z_StrDup it */
bool_t P_SGBSGDXReadStr(D_BS_t* const a_Stream, char** const a_Ptr, char* const a_Buf, const size_t a_BufSize)
{
	/* Clear */
	//memset(a_Buf, 0, sizeof(a_Buf) * a_BufSize);
	
	/* Read String */
	D_BSrs(a_Stream, a_Buf, a_BufSize - 1);
	
	/* Dupe it */
	if (a_Ptr)
	{
		if (*a_Ptr)
			Z_Free(*a_Ptr);
		*a_Ptr = Z_StrDup(a_Buf, PU_STATIC, NULL);
	}
	
	return true;
}

/* PS_SGDXDoStruct() -- Saves/Loads structure specification */
static bool_t PS_SGDXDoStruct(D_BS_t* const a_Stream, const bool_t a_Load, void* const a_Base, const P_SGDXDataSpec_t* const a_Spec)
{
	size_t i;
	uint64_t pMark;
	
	/* Check */
	if (!a_Stream || !a_Base || !a_Spec)
		return false;
	
	/* Dump/Identify Pointer to Struct */
	// If loading, read pointer and remember for future ref. Something most
		// likely points to this structure.
	if (a_Load)
	{
		pMark = D_BSrp(a_Stream);
		PLGS_SetRef(pMark, a_Base);
	}
	
	// If saving, write pointer.
	else
		D_BSwp(a_Stream, a_Base);
	
	/* Go through each spec */
	for (i = 0; !a_Spec[i].AtEnd; i++)
	{
		// Debug
		if (devparm)
			CONL_PrintF("Sim: %p+%4u (%2u,%2u)\n",
					((uint8_t*)a_Base) + a_Spec[i].OffSet,
					(unsigned)a_Spec[i].SizeOf, a_Spec[i].CType, a_Spec[i].RType
				);
		
		// Save/load each member
		PS_SGDXReadWriteData(a_Stream, a_Load, (void*)(((uint8_t*)a_Base) + a_Spec[i].OffSet), a_Spec[i].SizeOf, a_Spec[i].CType, a_Spec[i].RType, NULL);
	}
	
	/* Success? */
	return true;
}

/* P_SGDXDoArray() -- Do array of some type */
	// a_ArrayP -- Pointer to array
	// a_MemSz -- Size of array members
	// a_SizeP -- Pointer to array count (&numwhatever)
	// a_SizeSz -- Size of array count (sizeof(numwhatever))
	// a_SizeType -- Native type for array count (INT,SIZE,UINT32,etc.)
bool_t P_SGDXDoArray(D_BS_t* const a_Stream, const bool_t a_Load, void** const a_ArrayP, const size_t a_MemSz, void* const a_SizeP, const size_t a_SizeSz, const P_SGBWTypeC_t a_SizeType, const Z_MemoryTag_t a_PUTag)
{
	int32_t i32;
	
	/* Check */
	if (!a_Stream || !a_ArrayP || !a_MemSz || !a_SizeP || !a_SizeSz)
		return false;
	
	/* Read/Write Array Size */
	// Use the preexisting handler!
	PS_SGDXReadWriteData(a_Stream, a_Load, a_SizeP, a_SizeSz, a_SizeType, PSRC_UINT32, &i32);
	
	/* Loading */
	if (a_Load)
	{
		// Allocate
		*a_ArrayP = Z_Malloc(a_MemSz * i32, a_PUTag, NULL);
	}
	
	/* Saving */
	else
	{
		// Do nothing
		if (devparm)
			CONL_PrintF("Sim: Array %p[%4u] (ElemSz %u)\n",
					a_ArrayP, i32, (unsigned)a_MemSz
				);
	}
	
	/* Success? */
	return true;
}

#ifdef __BI
	#undef __BI
#endif
#ifdef __BISTRZ
	#undef __BISTRZ
#endif
#ifdef __BISTRB
	#undef __BISTRB
#endif	

#define __INITARRAY(ar,co,ct,pu) P_SGDXDoArray(a_Stream, a_Load, &ar, sizeof(*ar), &co, sizeof(co), PSTC_##ct,pu)
#define __BI(x,nt,rc) PS_SGDXReadWriteData(a_Stream, a_Load, &x, sizeof(x), PSTC_##nt, PSRC_##rc, NULL)

	// __BISTRZ -- Z_Malloced String
#define __BISTRZ(x) (a_Load ? P_SGDXReadStr(a_Stream,&x,Buf,BUFSIZE) : PS_WRAPPED_D_BSws(a_Stream, x))
	// __BISTRB -- Buffered String
#define __BISTRB(buf,bs) (a_Load ? P_SGBSGDXReadStr(a_Stream,NULL,buf,bs) : PS_WRAPPED_D_BSws(a_Stream, buf))

/* P_SGDXSpec() -- Save/Load Game via specification */
bool_t P_SGDXSpec(D_BS_t* const a_Stream, I_HostAddress_t* const a_NetAddr, const bool_t a_Load)
{
#define BUFSIZE 128
	char Buf[BUFSIZE];
	bool_t Continue;
	char Header[5];
	size_t i, j;
	void* vp;
	int32_t i32;
	bool_t HitBlock;
	I_HostAddress_t InAddr;
	
	/* Check */
	if (!a_Stream)
		return false;
		
	//M_ExUIMessageBox(MEXMBT_DONTCARE, 123, "Hello", "You just saved the game!", NULL);
	
	/* Read/Dump Everything */
	Continue = true;
	while (Continue)
	{
		//////////////////////////////
		// If loading, read block (play it back)
		memset(Header, 0, sizeof(Header));
		if (a_Load)
		{
			memset(&InAddr, 0, sizeof(InAddr));
			if (!(Continue = D_BSPlayNetBlock(a_Stream, Header, &InAddr)))
				break;
			HitBlock = false;	// Check when a valid block was read
		}
		
		//////////////////////////////
		// Save specific data
		
		// SGGV -- Game Variables
		if (__HEADER("SGGV"))
		{
			
			
			// Record
			__REC;
		}
		
		// SGPW -- PWADs
		if (__HEADER("SGPW"))
		{
			// Record
			__REC;
		}
		
		// SGMP -- Map Level
		if (__HEADER("SGMP"))
		{
			// Save/Restore Map Level
				// Save
			if (!a_Load)
				strncpy(Buf, g_CurrentLevelInfo->LumpName, BUFSIZE - 1);
			__BISTRB(Buf,BUFSIZE - 1);
			__BI(gamestate,INT,INT32);
			__BI(gameaction,INT,INT32);
			
			// Load Level Base
			if (a_Load)
				P_ExLoadLevel(P_FindLevelByNameEx(Buf, 0), PEXLL_NOSPAWNSPECIALS | PEXLL_NOSPAWNMAPTHING | PEXLL_NOINITBRAIN | PEXLL_NOFINALIZE | PEXLL_NOCLEARLEVEL);
				
			// Record
			__REC;	
		}
		
		// SGMI -- Misc.
		if (__HEADER("SGMI"))
		{
			// Map Things
			__INITARRAY(mapthings,nummapthings,INT,PU_LEVEL);
			for (i = 0; i < nummapthings; i++)
				PS_SGDXDoStruct(a_Stream, a_Load, &mapthings[i], c_MapThingSpec);
			
			// Player Starts
			for (i = 0; i < MAXPLAYERS; i++)
				__BI(playerstarts[i],POINTERTO,POINTER);
			
			// Record
			__REC;
		}
		
		// SGTH -- Thinkers
		if (__HEADER("SGTH"))
		{
			// Record
			__REC;
		}
		
#if 0
		// SGVX -- Vertexes
		if (__HEADER("SGVX"))
		{
			__INITARRAY(vertexes,numvertexes,INT,PU_LEVEL);
			for (i = 0; i < numvertexes; i++)
				PS_SGDXDoStruct(a_Stream, a_Load, &vertexes[i], c_VertexSpec);
			
			// Record
			__REC;
		}
		
		// SGLD -- LineDefs
		if (__HEADER("SGLD"))
		{
			__INITARRAY(lines,numlines,INT,PU_LEVEL);
			for (i = 0; i < numlines; i++)
				PS_SGDXDoStruct(a_Stream, a_Load, &lines[i], c_LineSpec);
			
			// Record
			__REC;
		}
		
		
		// SGSD -- SideDefs
		if (__HEADER("SGSD"))
		{
			__INITARRAY(sides,numsides,INT,PU_LEVEL);
			for (i = 0; i < numsides; i++)
				PS_SGDXDoStruct(a_Stream, a_Load, &sides[i], c_SideSpec);
			
			// Record
			__REC;
		}
		
		// SGSC -- Sectors
		if (__HEADER("SGSC"))
		{
			__INITARRAY(sectors,numsectors,INT,PU_LEVEL);
			for (i = 0; i < numsectors; i++)
				PS_SGDXDoStruct(a_Stream, a_Load, &sectors[i], c_SectorSpec);
			
			// Record
			__REC;
		}
		
		// SGST -- State
		if (__HEADER("SGST"))
		{
			__BI(gamestate,INT,INT32);
			
			// Record
			__REC;
		}
#endif
		
		// If Saving, Terminate
			// Or hit an invalid block when loading
		if (!a_Load || (a_Load && !HitBlock))
			break;
	}
	
	/* Parse Pointer Reference Tables */
	if (a_Load)
	{
		// Go through each one and Reference everything
		for (i = 0; i < l_NumDerefs; i++)
		{
			// Missing value to set?
			if (!l_Derefs[i].SetVal)
				continue;
			
			// Go through change list
			for (j = 0; j < l_Derefs[i].NumChangePtr; j++)
			{
				vp = *(l_Derefs[i].ChangePtr[j]);
				*(l_Derefs[i].ChangePtr[j]) = l_Derefs[i].SetVal;
				
				l_Derefs[i].ChangePtr[j] = NULL;	// Clear for future checking
			}
		}
		
		// Free all references
		Z_FreeTags(PU_SGPTRREF, PU_SGPTRREF);
		l_Derefs = NULL;
		l_NumDerefs = 0;
	}
	
	/* Success? */
	return true;
#undef BUFSIZE
}

#endif

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

