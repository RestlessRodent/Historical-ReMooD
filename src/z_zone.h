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

/*****************
*** PROTOTYPES ***
*****************/

/* Control */
void Z_Init(void);
size_t Z_TagUsage(const Z_MemoryTag_t TagNum);
void Z_CheckHeap(const int Code);

/* Misc */
char *Z_Strdup(const char* const String, const Z_MemoryTag_t Tag, void** Ref);
void Z_ChangeTag(void* const Ptr, const Z_MemoryTag_t NewTag);
void Z_DebugMarkBlock(void* const Ptr, const char* const String);
void Z_ResizeArray(void** const PtrPtr, const size_t ElemSize, const size_t OldSize, const size_t NewSize);

/* Memory */
#if defined(_DEBUG)		// DEBUG
	#define _ZMGD_WRAPPEE , const char* const File, const int Line
	
	#define Z_Malloc(s,t,r) Z_MallocWrappee((s),(t),(r),__FILE__,__LINE__)
	#define Z_MallocAlign(s,t,r,a) Z_MallocWrappee((s),(t),(r),__FILE__,__LINE__)
	#define Z_Free(p) Z_FreeWrappee((p),__FILE__,__LINE__)
	#define Z_FreeTags(l,h) Z_FreeTagsWrappee((l),(h),__FILE__,__LINE__)
#else					// NOT DEBUGGING
	#define _ZMGD_WRAPPEE
	
	#define Z_Malloc(s,t,r) Z_MallocWrappee((s),(t),(r))
	#define Z_MallocAlign(s,t,r,a) Z_MallocWrappee((s),(t),(r))
	#define Z_Free(p) Z_FreeWrappee((p))
	#define Z_FreeTags(l,h) Z_FreeTagsWrappee((l),(h))
#endif

// Prototypes
void *Z_MallocWrappee(const size_t Size, const Z_MemoryTag_t Tag, void** Ref _ZMGD_WRAPPEE);
void Z_FreeWrappee(void* const Ptr _ZMGD_WRAPPEE);
size_t Z_FreeTagsWrappee(const Z_MemoryTag_t LowTag, const Z_MemoryTag_t HighTag _ZMGD_WRAPPEE);

/*****************************************************************************/

#endif /* __Z_ZONE_H__ */

