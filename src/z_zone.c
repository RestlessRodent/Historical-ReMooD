// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Wraps Memory Management

/***************
*** INCLUDES ***
***************/

#include "z_zone.h"
#include "i_system.h"
#include "m_argv.h"

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
	bool_t DidSomething, IsDebug;
	uint8_t Run;
	
	/* Clear */
	DidSomething = false;
	IsDebug = false;
	
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

