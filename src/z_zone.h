// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
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
// Copyright (C) 2010 GhostlyDeath.
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
	PU_FREE											= 0,	// Not used
	
	/* Static */
	PU_STATIC										= 1,	// Never freed
	PU_SOUND,
	PU_MUSIC,
	PU_DAVE,
	PU_HWRPATCHINFO,
	PU_HWRPATCHCOLMIPMAP,
	PU_LEVEL,
	PU_LEVSPEC,
	PU_HWRPLANE,
	
	/* Cache */
	PU_PURGELEVEL									= 100,	// Freed when needed
	PU_CACHE,
	PU_HWRCACHE,
	
	NUMZTAGS
} Z_MemoryTag_t;

/* Z_LockBackAction_t -- Action being performed */
typedef enum Z_LockBackAction_e
{
	ZLBA_CHANGETAG,								// Tag is being changed
	ZLBA_FREE,									// Being freed
	
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
char __REMOOD_DEPRECATED *Z_Strdup(const char* const String, const Z_MemoryTag_t Tag, void** Ref);
char *Z_StrDup(const char* const String, const Z_MemoryTag_t Tag, void** Ref);
void Z_DebugMarkBlock(void* const Ptr, const char* const String);
void Z_ResizeArray(void** const PtrPtr, const size_t ElemSize, const size_t OldSize, const size_t NewSize);
void Z_SetLockBack(void* const Ptr, boolean (*LockBack)(void* const, const Z_LockBackAction_t, const uintptr_t, const uintptr_t));

/* Memory */
#if defined(_DEBUG)		// DEBUG
	#define _ZMGD_WRAPPEE , const char* const File, const int Line
	
	#define Z_Malloc(s,t,r) Z_MallocWrappee((s),(t),(r),__FILE__,__LINE__)
	#define Z_MallocAlign(s,t,r,a) Z_MallocWrappee((s),(t),(r),__FILE__,__LINE__)
	#define Z_Free(p) Z_FreeWrappee((p),__FILE__,__LINE__)
	#define Z_FreeTags(l,h) Z_FreeTagsWrappee((l),(h),__FILE__,__LINE__)
	#define Z_ChangeTag(p,t) Z_ChangeTagWrappee((p),(t),__FILE__,__LINE__)
	#define Z_GetTagFromPtr(p) Z_GetTagFromPtrWrappee((p),__FILE__,__LINE__)
#else					// NOT DEBUGGING
	#define _ZMGD_WRAPPEE
	
	#define Z_Malloc(s,t,r) Z_MallocWrappee((s),(t),(r))
	#define Z_MallocAlign(s,t,r,a) Z_MallocWrappee((s),(t),(r))
	#define Z_Free(p) Z_FreeWrappee((p))
	#define Z_FreeTags(l,h) Z_FreeTagsWrappee((l),(h))
	#define Z_ChangeTag(p,t) Z_ChangeTagWrappee((p),(t))
	#define Z_GetTagFromPtr(p) Z_GetTagFromPtrWrappee((p))
#endif

// Prototypes
void *Z_MallocWrappee(const size_t Size, const Z_MemoryTag_t Tag, void** Ref _ZMGD_WRAPPEE);
void Z_FreeWrappee(void* const Ptr _ZMGD_WRAPPEE);
size_t Z_FreeTagsWrappee(const Z_MemoryTag_t LowTag, const Z_MemoryTag_t HighTag _ZMGD_WRAPPEE);
Z_MemoryTag_t Z_GetTagFromPtrWrappee(void* const Ptr _ZMGD_WRAPPEE);
Z_MemoryTag_t Z_ChangeTagWrappee(void* const Ptr, const Z_MemoryTag_t NewTag _ZMGD_WRAPPEE);

/*** Hash Utility ***/

/* Structures */
typedef struct Z_HashTable_s Z_HashTable_t;

/* Prototypes */
uint32_t Z_Hash(const char* const a_Str);
Z_HashTable_t* Z_HashCreateTable(boolean (*a_CompareFunc)(void* const a_A, void* const a_B));
void Z_HashDeleteTable(Z_HashTable_t* const a_HashTable);
boolean Z_HashAddEntry(Z_HashTable_t* const a_HashTable, const uint32_t a_Key, void* const a_Data);
void* Z_HashFindEntry(Z_HashTable_t* const a_HashTable, const uint32_t a_Key, void* const a_DataSim, const boolean a_BackRun);

/*** Table Utility ***/

/* Structures */
typedef struct Z_Table_s Z_Table_t;

/* Prototypes */
Z_Table_t* Z_TableCreate(const char* const a_Key);
void Z_TableDestroy(Z_Table_t* const a_Table);
Z_Table_t* Z_TableUp(Z_Table_t* const a_Table);
Z_Table_t* Z_FindSubTable(Z_Table_t* const a_Table, const char* const a_Key, const boolean a_Create);
const char* Z_TableGetValue(Z_Table_t* const a_Table, const char* const a_SubKey);
boolean Z_TableSetValue(Z_Table_t* const a_Table, const char* const a_SubKey, const char* const a_NewValue);
void Z_TableClearValue(Z_Table_t* const a_Table, const char* const a_SubKey);
void Z_TablePrint(Z_Table_t* const a_Table, const char* const a_Prefix);
boolean Z_TableMergeInto(Z_Table_t* const a_Target, const Z_Table_t* const a_Source);

/*****************************************************************************/

#endif /* __Z_ZONE_H__ */

