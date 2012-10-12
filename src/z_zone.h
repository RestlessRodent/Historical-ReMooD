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
// Copyright (C) 2011-2012 GhostlyDeath <ghostlydeath@remood.org>.
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
// DESCRIPTION: Modified Zone Memory Allocation Written by GhostlyDeath

#ifndef __Z_ZONE_H__
#define __Z_ZONE_H__

/*****************************************************************************/

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"

/****************
*** CONSTANTS ***
****************/

/* Z_Tag_t -- Memory tag type */
typedef enum Z_MemoryTag_e
{
	PU_FREE = 0,				// Not used
	
	/* Static */
	PU_STATIC = 1,				// Never freed
	
	PU_BLOCKSTREAM,				// Block Streams
	PU_SGPTRREF,								// Save Game Pointer References
	PU_NETWORK,									// Networking Stuff
	PU_BOTS,									// Bot Stuff
	
	PU_REMOODAT,								// ReMooD Data
	PU_FONTCHARS,								// Font Characters
	
	PU_WLDKRMOD,								// WAD Level
	PU_SOUND,
	PU_MUSIC,
	PU_DAVE,
	PU_HWRPATCHINFO,
	PU_HWRPATCHCOLMIPMAP,
	
	PU_LEVEL,
	PU_LEVSPEC,
	PU_HWRPLANE,
	PU_ENDLEVELTAGS,
	
	PU_MENUDAT,					// Menu Data
	
	/* Cache */
	PU_PURGELEVEL = 100,		// Freed when needed
	PU_CACHE,
	PU_HWRCACHE,
	
	NUMZTAGS
} Z_MemoryTag_t;

/* Z_LockBackAction_t -- Action being performed */
typedef enum Z_LockBackAction_e
{
	ZLBA_CHANGETAG,				// Tag is being changed
	ZLBA_FREE,					// Being freed
	
	NUMZLOCKBACKACTIONS
} Z_LockBackAction_t;

/*****************
*** PROTOTYPES ***
*****************/

/* Control */
void Z_Init(void);
size_t Z_TagUsage(const Z_MemoryTag_t TagNum);
void Z_CheckHeap(const int Code);

/* Misc */
char __REMOOD_DEPRECATED* Z_Strdup(const char* const String, const Z_MemoryTag_t Tag, void** Ref);
void Z_DebugMarkBlock(void* const Ptr, const char* const String);
void Z_SetLockBack(void* const Ptr, bool_t (*LockBack) (void* const, const Z_LockBackAction_t, const uintptr_t, const uintptr_t));

extern void (*Z_RegisterCommands)(void);

/* Memory */
#if defined(_DEBUG)				// DEBUG
#define _ZMGD_WRAPPEE , const char* const File, const int Line

#define Z_MallocEx(s,t,r,f) Z_MallocExWrappee((s),(t),(r),(f),__FILE__,__LINE__)
#define Z_Malloc(s,t,r) Z_MallocExWrappee((s),(t),(r),0,__FILE__,__LINE__)
#define Z_MallocAlign(s,t,r,a) Z_MallocExWrappee((s),(t),(r),0,__FILE__,__LINE__)
#define Z_Free(p) Z_FreeWrappee((p),__FILE__,__LINE__)
#define Z_FreeTags(l,h) Z_FreeTagsWrappee((l),(h),__FILE__,__LINE__)
#define Z_ChangeTag(p,t) Z_ChangeTagWrappee((p),(t),__FILE__,__LINE__)
#define Z_GetTagFromPtr(p) Z_GetTagFromPtrWrappee((p),__FILE__,__LINE__)

#define Z_StrDup(p,t,r) Z_StrDupWrappee((p),(t),(r),__FILE__,__LINE__)
#define Z_ResizeArray(p,e,o,n) Z_ResizeArrayWrappee((p),(e),(o),(n),__FILE__,__LINE__)

void Z_DupFileLine(void* const a_Dest, void* const a_Src);

#else							// NOT DEBUGGING
#define _ZMGD_WRAPPEE

#define Z_MallocEx(s,t,r,f) Z_MallocExWrappee((s),(t),(r),(f))
#define Z_Malloc(s,t,r) Z_MallocExWrappee((s),(t),(r),0)
#define Z_MallocAlign(s,t,r,a) Z_MallocExWrappee((s),(t),(r),0)
#define Z_Free(p) Z_FreeWrappee((p))
#define Z_FreeTags(l,h) Z_FreeTagsWrappee((l),(h))
#define Z_ChangeTag(p,t) Z_ChangeTagWrappee((p),(t))
#define Z_GetTagFromPtr(p) Z_GetTagFromPtrWrappee((p))

#define Z_StrDup(p,t,r) Z_StrDupWrappee((p),(t),(r))
#define Z_ResizeArray(p,e,o,n) Z_ResizeArrayWrappee((p),(e),(o),(n))
#endif

// Prototypes
extern void* (*Z_MallocExWrappee)(const size_t a_Size, const Z_MemoryTag_t a_Tag, void** a_Ref, const uint32_t a_Flags _ZMGD_WRAPPEE);
extern void (*Z_FreeWrappee)(void* const Ptr _ZMGD_WRAPPEE);
extern size_t (*Z_FreeTagsWrappee)(const Z_MemoryTag_t LowTag, const Z_MemoryTag_t HighTag _ZMGD_WRAPPEE);
extern Z_MemoryTag_t (*Z_GetTagFromPtrWrappee)(void* const Ptr _ZMGD_WRAPPEE);
extern Z_MemoryTag_t (*Z_ChangeTagWrappee)(void* const Ptr, const Z_MemoryTag_t NewTag _ZMGD_WRAPPEE);

char* Z_StrDupWrappee(const char* const String, const Z_MemoryTag_t Tag, void** Ref _ZMGD_WRAPPEE);
void Z_ResizeArrayWrappee(void** const PtrPtr, const size_t ElemSize, const size_t OldSize, const size_t NewSize _ZMGD_WRAPPEE);

/*** Hash Utility ***/

/* Structures */
typedef struct Z_HashTable_s Z_HashTable_t;

/* Prototypes */
uint32_t Z_Hash(const char* const a_Str);
Z_HashTable_t* Z_HashCreateTable(bool_t (*a_CompareFunc) (void* const a_A, void* const a_B));
void Z_HashDeleteTable(Z_HashTable_t* const a_HashTable);
bool_t Z_HashAddEntry(Z_HashTable_t* const a_HashTable, const uint32_t a_Key, void* const a_Data);
void* Z_HashFindEntry(Z_HashTable_t* const a_HashTable, const uint32_t a_Key, void* const a_DataSim, const bool_t a_BackRun);

bool_t Z_HashDeleteEntry(Z_HashTable_t* const a_HashTable, const uint32_t a_Key, void* const a_DataSim, const bool_t a_BackRun);

/*** Table Utility ***/

/* Structures */
typedef struct Z_Table_s Z_Table_t;

/* Prototypes */
Z_Table_t* Z_TableCreate(const char* const a_Key);
void Z_TableDestroy(Z_Table_t* const a_Table);
Z_Table_t* Z_TableUp(Z_Table_t* const a_Table);
const char* Z_TableName(Z_Table_t* const a_Table);
Z_Table_t* Z_FindSubTable(Z_Table_t* const a_Table, const char* const a_Key, const bool_t a_Create);
const char* Z_TableGetValue(Z_Table_t* const a_Table, const char* const a_SubKey);
int32_t Z_TableGetValueInt(Z_Table_t* const a_Table, const char* const a_SubKey, bool_t* const a_Found);
bool_t Z_TableSetValue(Z_Table_t* const a_Table, const char* const a_SubKey, const char* const a_NewValue);
void Z_TableClearValue(Z_Table_t* const a_Table, const char* const a_SubKey);
void Z_TablePrint(Z_Table_t* const a_Table, const char* const a_Prefix);
bool_t Z_TableMergeInto(Z_Table_t* const a_Target, const Z_Table_t* const a_Source);
bool_t Z_TableSuperCallback(Z_Table_t* const a_Table, bool_t (*a_Callback) (Z_Table_t* const a_Sub, void* const a_Data), void* const a_Data);

const char* Z_TableGetValueOrElse(Z_Table_t* const a_Table, const char* const a_SubKey, const char* a_ElseOr);

struct D_BS_s;
void Z_TableStoreToStream(Z_Table_t* const a_Table, struct D_BS_s* const a_Stream);
Z_Table_t* Z_TableStreamToStore(struct D_BS_s* const a_Stream);

/*****************************************************************************/

#endif							/* __Z_ZONE_H__ */
