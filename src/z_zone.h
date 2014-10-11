// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
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
	
	PU_LEVEL,
	PU_LEVSPEC,
	PU_HWRPLANE,
	PU_ENDLEVELTAGS = PU_HWRPLANE,
	
	PU_MENUDAT,					// Menu Data
	PU_SIMPLEMENU,								// Simple Menus
	
	/* Cache */
	PU_PURGELEVEL,		// Freed when needed
	PU_CACHE,
	
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

/* Define Z_HashTable_t */
#if !defined(__REMOOD_ZHT_DEFINED)
	typedef struct Z_HashTable_s Z_HashTable_t;
	#define __REMOOD_ZHT_DEFINED
#endif

/* Prototypes */
uint32_t Z_Hash(const char* const a_Str);
Z_HashTable_t* Z_HashCreateTable(bool_t (*a_CompareFunc) (void* const a_A, void* const a_B));
void Z_HashDeleteTable(Z_HashTable_t* const a_HashTable);
bool_t Z_HashAddEntry(Z_HashTable_t* const a_HashTable, const uint32_t a_Key, void* const a_Data);
void* Z_HashFindEntry(Z_HashTable_t* const a_HashTable, const uint32_t a_Key, void* const a_DataSim, const bool_t a_BackRun);

bool_t Z_HashDeleteEntry(Z_HashTable_t* const a_HashTable, const uint32_t a_Key, void* const a_DataSim, const bool_t a_BackRun);

/*****************************************************************************/

#endif							/* __Z_ZONE_H__ */
