// -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
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
// DESCRIPTION: Wraps Memory Management

/***************
*** INCLUDES ***
***************/

#include "z_zone.h"
#include "i_system.h"
#include "m_misc.h"
#include "doomdef.h"
#include "m_argv.h"
#include "console.h"
#include "d_block.h"

/**************
*** GLOBALS ***
**************/

void* (*Z_MallocExWrappee)(const size_t a_Size, const Z_MemoryTag_t a_Tag, void** a_Ref, const uint32_t a_Flags _ZMGD_WRAPPEE) = NULL;
void (*Z_FreeWrappee)(void* const Ptr _ZMGD_WRAPPEE) = NULL;
size_t (*Z_FreeTagsWrappee)(const Z_MemoryTag_t LowTag, const Z_MemoryTag_t HighTag _ZMGD_WRAPPEE) = NULL;
Z_MemoryTag_t (*Z_GetTagFromPtrWrappee)(void* const Ptr _ZMGD_WRAPPEE) = NULL;
Z_MemoryTag_t (*Z_ChangeTagWrappee)(void* const Ptr, const Z_MemoryTag_t NewTag _ZMGD_WRAPPEE) = NULL;
void (*Z_RegisterCommands)(void) = NULL;

/****************
*** FUNCTIONS ***
****************/

/*** MHS (MALLOC HASH CHAINING) ***/

void Z_MHC_Init(void);
void* Z_MHC_MallocExWrappee(const size_t a_Size, const Z_MemoryTag_t a_Tag, void** a_Ref, const uint32_t a_Flags _ZMGD_WRAPPEE);
void Z_MHC_FreeWrappee(void* const Ptr _ZMGD_WRAPPEE);
size_t Z_MHC_FreeTagsWrappee(const Z_MemoryTag_t LowTag, const Z_MemoryTag_t HighTag _ZMGD_WRAPPEE);
Z_MemoryTag_t Z_MHC_GetTagFromPtrWrappee(void* const Ptr _ZMGD_WRAPPEE);
Z_MemoryTag_t Z_MHC_ChangeTagWrappee(void* const Ptr, const Z_MemoryTag_t NewTag _ZMGD_WRAPPEE);
void Z_MHC_RegisterCommands(void);

/*** DEBUG ***/

void Z_DEBUG_Init(void);
void* Z_DEBUG_MallocExWrappee(const size_t a_Size, const Z_MemoryTag_t a_Tag, void** a_Ref, const uint32_t a_Flags _ZMGD_WRAPPEE);
void Z_DEBUG_FreeWrappee(void* const Ptr _ZMGD_WRAPPEE);
size_t Z_DEBUG_FreeTagsWrappee(const Z_MemoryTag_t LowTag, const Z_MemoryTag_t HighTag _ZMGD_WRAPPEE);
Z_MemoryTag_t Z_DEBUG_GetTagFromPtrWrappee(void* const Ptr _ZMGD_WRAPPEE);
Z_MemoryTag_t Z_DEBUG_ChangeTagWrappee(void* const Ptr, const Z_MemoryTag_t NewTag _ZMGD_WRAPPEE);
void Z_DEBUG_RegisterCommands(void);

/*** FUNCTIONS ***/

/* Z_Init() -- Initializes a memory manager */
void Z_Init(void)
{
	bool DidSomething, IsDebug;
	uint8_t Run;
	
	/* Clear */
	DidSomething = false;
	
#if defined(_DEBUG)
	IsDebug = true;
#else
	IsDebug = false;
#endif
	
	/* Select which manager */
	for (Run = 0; Run < 2; Run++)
	{
		// Malloc Hash Chained Memory Manager
			// Fast but might cause problems in low memory or specific systems
			// such as DOS, PalmOS, and other mobile platforms.
		if (!DidSomething && (((!Run && M_CheckParm("-zmhc")) || (Run && !IsDebug))))
		{
			// Set pointers
			Z_MallocExWrappee = Z_MHC_MallocExWrappee;
			Z_FreeWrappee = Z_MHC_FreeWrappee;
			Z_FreeTagsWrappee = Z_MHC_FreeTagsWrappee;
			Z_GetTagFromPtrWrappee = Z_MHC_GetTagFromPtrWrappee;
			Z_ChangeTagWrappee = Z_MHC_ChangeTagWrappee;
			Z_RegisterCommands = Z_MHC_RegisterCommands;
		
			// Set
			DidSomething = true;
		
			// Init
			Z_MHC_Init();
		}
		
		// Debug Memory Manager (Perfect, but very slow)
		if (!DidSomething && (((!Run && M_CheckParm("-zdebug")) || (Run && IsDebug))))
		{
			// Set pointers
			Z_MallocExWrappee = Z_DEBUG_MallocExWrappee;
			Z_FreeWrappee = Z_DEBUG_FreeWrappee;
			Z_FreeTagsWrappee = Z_DEBUG_FreeTagsWrappee;
			Z_GetTagFromPtrWrappee = Z_DEBUG_GetTagFromPtrWrappee;
			Z_ChangeTagWrappee = Z_DEBUG_ChangeTagWrappee;
			Z_RegisterCommands = Z_DEBUG_RegisterCommands;
		
			// Set
			DidSomething = true;
		
			// Init
			Z_DEBUG_Init();
		}
	}
}

#define __REMOOD_MALCODE UINT32_C(0x2A1170C2)
#define __REMOOD_SYSCODE UINT32_C(0x5157C0DE)

/* operator new() -- Make allocation */
void* operator new(size_t n)// throw(std::bad_alloc)
{
	void* NewPtr;
	
	/* Allocate Pointer */
	if (Z_MallocExWrappee)
	{
		NewPtr = Z_Malloc(n + 4, PU_STATIC, NULL);
		*((uint32_t*)NewPtr) = __REMOOD_MALCODE;
	}
	else
	{
		NewPtr = I_SysAlloc(n + 4);
		memset(NewPtr, 0, n + 4);
		*((uint32_t*)NewPtr) = __REMOOD_SYSCODE;
	}
	
	/* Return allocated pointer */
	return (void*)(((uintptr_t)NewPtr) + 4);
}

/* operator delete() -- Delete allocation */
void operator delete(void* p)// throw()
{
	uint32_t* RawP;
	
	/* Get raw pointer */
	RawP = (uint32_t*)((uintptr_t)p - 4);
	
	/* Which pointer kind? */
	// Zone Allocated
	if (*RawP == __REMOOD_MALCODE)
		Z_Free(RawP);
		
	// System Allocated
	else if (*RawP == __REMOOD_SYSCODE)
		I_SysFree(RawP);
	
	// Unknown -- Go kaboom
	else
		I_Error("C++ delete of pointer of unknown kind!");
}

