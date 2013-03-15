// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------
// Copyright (C) 2011-2013 GhostlyDeath <ghostlydeath@remood.org>
//                                      <ghostlydeath@gmail.com>
// ----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// ----------------------------------------------------------------------------
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

