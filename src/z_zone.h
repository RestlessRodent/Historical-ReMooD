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
// Copyright (C) 2010 GhostlyDeath.
// Copyright (C) 2010 The ReMooD Team.
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
// DESCRIPTION: Modified Zone Memory Allocation Written by GhostlyDeath

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

/* Memory */
#if defined(_DEBUG)
	// DEBUG
void *Z_MallocWrappee(const size_t Size, const Z_MemoryTag_t Tag, void** Ref,
		const char* const File, const int Line);
void Z_FreeWrappee(void* const Ptr, const char* const File, const int Line);


#define Z_Malloc(s,t,r) Z_MallocWrappee(s,t,r,__FILE__,__LINE__)
#define Z_MallocAlign(s,t,r,a) Z_MallocWrappee(s,t,r,__FILE__,__LINE__)
#define Z_Free(p) Z_FreeWrappee(p,__FILE__,__LINE__)
#define Z_FreeTags(l,h) Z_FreeTagsWrappee(l,h,__FILE__,__LINE__)
#else
	// NOT DEBUG
void *Z_MallocWrappee(const size_t Size, const Z_MemoryTag_t Tag, void** Ref);
void Z_FreeWrappee(void* const Ptr);

#define Z_Malloc(s,t,r) Z_MallocWrappee(s,t,r)
#define Z_MallocAlign(s,t,r,a) Z_MallocWrappee(s,t,r)
#define Z_Free(p) Z_FreeWrappee(p)
#define Z_FreeTags(l,h) Z_FreeTagsWrappee(l)
#endif

#endif /* __Z_ZONE_H__ */

