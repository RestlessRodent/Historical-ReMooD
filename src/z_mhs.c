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
// Copyright (C) 2012-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Malloc Hash Chaining

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

/****************
*** CONSTANTS ***
****************/

#define ZSTART			UINT32_C(0xDEADBEEF)	// Start of Block
#define ZEND			UINT32_C(0xCAFEBABE)	// End of Block

#define ZALIGNMENT						16		// Align to 
#define ZBLOCKBASE	((sizeof(Z_MHCInfo_t) + ZALIGNMENT) & ~(ZALIGNMENT - 1))

/*****************
*** STRUCTURES ***
*****************/

/* Z_MHCInfo_t -- Malloc Chain Info */
typedef struct Z_MHCInfo_s
{
	uint32_t Start;								// Marker Start
	
	size_t Size;								// Allocation Size
	Z_MemoryTag_t Tag;							// Allocation Tag
	void** RefPtr;								// Reference
	
	struct Z_MHCInfo_s* Prev;					// Previous
	struct Z_MHCInfo_s* Next;					// Next
	
	uint32_t End;								// Marker End
} Z_MHCInfo_t;

/*************
*** LOCALS ***
*************/

static Z_MHCInfo_t* l_ZChains[NUMZTAGS + 1];

/****************
*** FUNCTIONS ***
****************/

/* Z_MHC_Init() -- Chain initialization */
void Z_MHC_Init(void)
{
	/* Clear chain list */
	memset(l_ZChains, 0, sizeof(l_ZChains));
}

/* Z_MHC_MallocExWrappee() -- Allocates space */
void* Z_MHC_MallocExWrappee(const size_t a_Size, const Z_MemoryTag_t a_Tag, void** a_Ref, const uint32_t a_Flags _ZMGD_WRAPPEE)
{
	Z_MHCInfo_t* AllocBlock;
	uint8_t Try;
	
	/* Allocate space */
	// Try Twice
	for (Try = 0; Try < 2; Try++)
	{
		// Attempt system allocation
		AllocBlock = I_SysAlloc(ZBLOCKBASE + a_Size);
		
		// Failed?
		if (!AllocBlock)
		{
			// Free all tags and try again
			Z_FreeTags(PU_PURGELEVEL, NUMZTAGS);
			continue;
		}
		
		// Worked! Clear Block
		memset(AllocBlock, 0, ZBLOCKBASE + a_Size);
		
		// Set Info
		AllocBlock->Start = ZSTART;
		AllocBlock->End = ZEND;
		
		AllocBlock->Size = a_Size;
		AllocBlock->Tag = a_Tag;
		AllocBlock->RefPtr = a_Ref;
		
		// Link into tag chain
		if (!l_ZChains[AllocBlock->Tag])
			l_ZChains[AllocBlock->Tag] = AllocBlock;
		else
		{
			AllocBlock->Next = l_ZChains[AllocBlock->Tag];
			l_ZChains[AllocBlock->Tag]->Prev = AllocBlock;
			l_ZChains[AllocBlock->Tag] = AllocBlock;
		}
		
		// Return offset of
		return (void*)((uintptr_t)AllocBlock + ZBLOCKBASE);
	}
	
	/* Failed to allocate */
	I_Error("Out of system memory.");
	return NULL;
}

/* Z_MHC_FreeWrappee() -- Frees a memory block */
void Z_MHC_FreeWrappee(void* const Ptr _ZMGD_WRAPPEE)
{
	Z_MHCInfo_t* AllocBlock;
	
	/* Get allocation block */
	AllocBlock = (Z_MHCInfo_t*)(((uintptr_t)Ptr) - ZBLOCKBASE);
	
	/* Corrupted? */
	if (AllocBlock->Start != ZSTART || AllocBlock->End != ZEND)
	{
		I_Error("Corrupted Memory Block");
		return;
	}
	
	/* Un-Ref */
	if (AllocBlock->RefPtr)
		*AllocBlock->RefPtr = NULL;
	
	/* Invalidate */
	AllocBlock->Start = ZEND;
	AllocBlock->End = ZSTART;
	
	/* Un-Chain */
	if (AllocBlock->Prev)
		AllocBlock->Prev->Next = AllocBlock->Next;
	if (AllocBlock->Next)
		AllocBlock->Next->Prev = AllocBlock->Prev;
		
	// Root?
	if (l_ZChains[AllocBlock->Tag] == AllocBlock)
		l_ZChains[AllocBlock->Tag] = AllocBlock->Next;
	
	/* Free */
	I_SysFree(AllocBlock);
}

/* Z_MHC_FreeTagsWrappee() -- Frees tags */
size_t Z_MHC_FreeTagsWrappee(const Z_MemoryTag_t LowTag, const Z_MemoryTag_t HighTag _ZMGD_WRAPPEE)
{
	Z_MemoryTag_t Rover;
	size_t Count;
	
	/* Free tag chains */
	Count = 0;
	for (Rover = LowTag; Rover <= HighTag; Rover++)
	{
		// Free tags there
		while (l_ZChains[Rover])
		{
			Z_Free((void*)(((uintptr_t)l_ZChains[Rover]) + ZBLOCKBASE));
			Count++;
		}
		
		// Sanity
		if (l_ZChains[Rover])
			I_Error("Chain still referenced");
	}
	
	/* Return free count */
	return Count;
}

/* Z_MHC_GetTagFromPtrWrappee() -- Returns block tag */
Z_MemoryTag_t Z_MHC_GetTagFromPtrWrappee(void* const Ptr _ZMGD_WRAPPEE)
{
	Z_MHCInfo_t* AllocBlock;
	
	/* Get allocation block */
	AllocBlock = (Z_MHCInfo_t*)(((uintptr_t)Ptr) - ZBLOCKBASE);
	
	/* Corrupted? */
	if (AllocBlock->Start != ZSTART || AllocBlock->End != ZEND)
	{
		I_Error("Corrupted Memory Block");
		return 0;
	}
	
	/* Return it */
	return AllocBlock->Tag;
}

/* Z_MHC_ChangeTagWrappee() -- Changes block's tag */
Z_MemoryTag_t Z_MHC_ChangeTagWrappee(void* const Ptr, const Z_MemoryTag_t NewTag _ZMGD_WRAPPEE)
{
	Z_MHCInfo_t* AllocBlock;
	Z_MemoryTag_t OldTag;
	
	/* Get allocation block */
	AllocBlock = (Z_MHCInfo_t*)(((uintptr_t)Ptr) - ZBLOCKBASE);
	
	/* Corrupted? */
	if (AllocBlock->Start != ZSTART || AllocBlock->End != ZEND)
	{
		I_Error("Corrupted Memory Block");
		return 0;
	}
	
	/* Same Tag? Ignore */
	if (NewTag == AllocBlock->Tag)
		return AllocBlock->Tag;
	
	/* Remember */
	OldTag = AllocBlock->Tag;
	
	/* Un-Chain */
	// Root?
	if (l_ZChains[AllocBlock->Tag] == AllocBlock)
		l_ZChains[AllocBlock->Tag] = AllocBlock->Next;
	
	// Re-chain remainders
	if (AllocBlock->Prev)
		AllocBlock->Prev->Next = AllocBlock->Next;
	if (AllocBlock->Next)
		AllocBlock->Next->Prev = AllocBlock->Prev;
	AllocBlock->Prev = AllocBlock->Next = NULL;
	
	/* Re-tag */
	AllocBlock->Tag = NewTag;
	
	/* Re-chain */
	if (!l_ZChains[AllocBlock->Tag])
		l_ZChains[AllocBlock->Tag] = AllocBlock;
	else
	{
		AllocBlock->Next = l_ZChains[AllocBlock->Tag];
		l_ZChains[AllocBlock->Tag]->Prev = AllocBlock;
		l_ZChains[AllocBlock->Tag] = AllocBlock;
	}
	
	/* Return old tag */
	return OldTag;
}

void Z_MHC_RegisterCommands(void)
{
}

