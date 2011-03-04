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

//#define NEWMEM
#ifdef NEWMEM

#ifndef __Z_ZONE_H__
#define __Z_ZONE_H__

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
char *Z_Strdup(const char* String, const Z_MemoryTag_t Tag, void** Ref);
wchar_t* Z_StrdupW(const wchar_t* WString, const Z_MemoryTag_t Tag, void** Ref);
void Z_ChangeTag(void* const Ptr, const Z_MemoryTag_t Tag);
void Z_DebugMarkBlock(void* const Ptr, const char* const String);

/* Memory */
#if defined(_DEBUG)
	// DEBUG
void *Z_MallocWrappee(const size_t Size, const Z_MemoryTag_t Tag, void** Ref,
		const char* const File, const int Line);
void Z_FreeWrappee(void* const Ptr, const char* const File, const int Line);
size_t Z_FreeTagsWrappee(const Z_MemoryTag_t LowTag, const Z_MemoryTag_t HighTag, const char* const File, const int Line);

#define Z_Malloc(s,t,r) Z_MallocWrappee(s,t,r,__FILE__,__LINE__)
#define Z_MallocAlign(s,t,r,a) Z_MallocWrappee(s,t,r,__FILE__,__LINE__)
#define Z_Free(p) Z_FreeWrappee(p,__FILE__,__LINE__)
#define Z_FreeTags(l,h) Z_FreeTagsWrappee(l,h,__FILE__,__LINE__)
#else
	// NOT DEBUG
void *Z_MallocWrappee(const size_t Size, const Z_MemoryTag_t Tag, void** Ref);
void Z_FreeWrappee(void* const Ptr);
size_t Z_FreeTagsWrappee(const Z_MemoryTag_t LowTag, const Z_MemoryTag_t HighTag);

#define Z_Malloc(s,t,r) Z_MallocWrappee(s,t,r)
#define Z_MallocAlign(s,t,r,a) Z_MallocWrappee(s,t,r)
#define Z_Free(p) Z_FreeWrappee(p)
#define Z_FreeTags(l,h) Z_FreeTagsWrappee(l,h)
#endif

#endif /* __Z_ZONE_H__ */

#else

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
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2011 GhostlyDeath (ghostlydeath@gmail.com)
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

#ifndef __Z_ZONE__
#define __Z_ZONE__

#include "doomtype.h"

#include <stdio.h>

//
// ZONE MEMORY
// PU - purge tags.
// Tags < 100 are not overwritten until freed.
#define PU_STATIC               1	// static entire execution time
#define PU_SOUND                2	// static while playing
#define PU_MUSIC                3	// static while playing
#define PU_DAVE                 4	// anything else Dave wants static

#define PU_HWRPATCHINFO         5	// Hardware GlidePatch_t struct for OpenGl/Glide texture cache
#define PU_HWRPATCHCOLMIPMAP    6	// Hardware GlideMipmap_t struct colromap variation of patch

#define PU_LEVEL               50	// static until level exited
#define PU_LEVSPEC             51	// a special thinker in a level
#define PU_HWRPLANE            52
// Tags >= PU_PURGELEVEL are purgable whenever needed.
#define PU_PURGELEVEL         100
#define PU_CACHE              101
#define PU_HWRCACHE           102	// 'second-level' cache for graphics
									   // stored in hardware format and downloaded as needed

#define PU_MAXPURGELEVEL	200

//#define ZDEBUG

void Z_Init(void);
void Z_FreeTags(int lowtag, int hightag);
void Z_DumpHeap(int lowtag, int hightag);
void Z_FileDumpHeap(FILE * f);
void Z_CheckHeap(int i);
void Z_ChangeTag2(void *ptr, int tag);

// returns number of bytes allocated for one tag type
size_t Z_TagUsage(int tagnum);

void Z_FreeMemory(int *realfree, int *cachemem, int *usedmem, int *largefreeblock);

#ifdef _DEBUG
#define ZDEBUG
#endif

#ifdef ZDEBUG
#define Z_Free(p) Z_Free2(p, NULL, __FILE__,__LINE__)
void Z_Free2(void *ptr, void* Block, char *file, int line);
#define Z_Malloc(s,t,p) Z_MallocAlign2(s,t,p,0,__FILE__,__LINE__)
#define Z_MallocAlign(s,t,p,a) Z_MallocAlign2(s,t,p,a,__FILE__,__LINE__)
void *Z_MallocAlign2(size_t size, int tag, void **ptr, int alignbits, char *file, int line);
#else
#define Z_Free(p) Z_Free2(p, NULL)
void Z_Free2(void* ptr, void* Block);
void *Z_MallocAlign2(size_t size, int tag, void **user, int alignbits);
#define Z_Malloc(s,t,p) Z_MallocAlign2(s,t,p,0)
#define Z_MallocAlign(s,t,p,a) Z_MallocAlign2(s,t,p,a)
#endif

char *Z_Strdup(const char *s, int tag, void **user);
wchar_t* Z_StrdupW(const wchar_t* w, int tag, void** user);

#ifdef ZDEBUG
int Z_CheckCollision2(void* Area, void* Start, size_t Length, char* file, int line);
#define Z_CheckCollison(a,s,l) Z_CheckCollision2(a,s,l,__FILE__,__LINE__)
#else
#define Z_CheckCollison(a,s,l) (0)		// Use Zero for no collision
#endif

#define Z_ChangeTag(p,t)  Z_ChangeTag2(p,t)
#define Z_DebugMarkBlock(p,s)

#endif

#endif

