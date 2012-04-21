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
// DESCRIPTION: New New Memory Manager Written by GhostlyDeath
//              Stolen from my proprietary game and stripped

/***************
*** INCLUDES ***
***************/

#include <stdlib.h>
#include <string.h>
#include "z_zone.h"
#include "i_system.h"
#include "m_misc.h"
#include "doomdef.h"
#include "m_argv.h"
#include "console.h"

/****************
*** CONSTANTS ***
****************/

#define PARTSHIFT		5		// >>/<< for partition sizes
#define MINZONESIZE		(512 << 10)	// Minimum zone size: 512KiB
#define INITPARTCOUNT	256		// Initial partition count
#define RIGHTSIZE		8		// Minimum size to allocate on right
#define FORCEDMEMORYSIZE	(4U << 20)	// Minimum allowed auto memory
#define ZPARTEXTRASIZE 16		// Extra size to waste on allocation

/*****************
*** STRUCTURES ***
*****************/

/* Z_MemPartition_t -- A memory partition */
typedef struct Z_MemPartition_s
{
	/* Partition Data */
	struct
	{
		size_t Start;			// Partition start
		size_t End;				// Partition end
		size_t Size;			// Partition size
		bool_t Used;			// Partition used
		uint32_t* SelfRef;		// Self Reference Pointer
		bool_t AtEnd;			// Allocated at the end?
	} Part;						// Partition Info
	
	/* Block Data */
	struct
	{
		void* Ptr;				// Pointer to block
		void** Ref;				// Reference
		Z_MemoryTag_t Tag;		// Tag
		size_t Size;			// Allocation size
	} Block;
} Z_MemPartition_t;

/* Z_MemZone_t -- A memory zone */
typedef struct Z_MemZone_s
{
	/* Partitions */
	Z_MemPartition_t* PartitionList;	// List of partitions
	size_t NumPartitions;		// Number of partitions
	size_t MaxPartitions;		// Max partitions for list
	
	/* Data */
	uint8_t* DataChunk;			// Where all the data is stored
	size_t AllocSize;			// Allocated size
	size_t TotalMemory;			// Allocated memory (in shifts)
	size_t FreeMemory;			// Free memory (in shifts)
	
	/* Speed */
	size_t FirstFree;			// First free partition
	size_t LastFree;			// Last free partition
} Z_MemZone_t;

/*************
*** LOCALS ***
*************/

static Z_MemZone_t* l_MainZone = NULL;	// Memory zones

/************************
*** PRIVATE FUNCTIONS ***
************************/

#if defined(_DEBUG)
	#define __REMOOD_VALGRIND
#endif

#if defined(__REMOOD_VALGRIND)

/* Z_Allocation_t -- A malloc allocation (chunked) */
typedef struct Z_Allocation_s
{
	void* Data;									// Data to be allocated
	Z_MemoryTag_t Tag;							// Allocation Tag
	void** User;								// User
	
	struct Z_Allocation_s* Prev;				// Prev Block
	struct Z_Allocation_s* Next;				// Next Block
} Z_Allocation_t;

static Z_Allocation_t* l_AllocChain = NULL;		// Allocation Chain

/* ZS_AllocForPtr() -- Looks for pointer allocation */
static Z_Allocation_t* ZS_AllocForPtr(void* const a_Ptr)
{
	Z_Allocation_t* Rover;
	
	/* Check */
	if (!a_Ptr)
		return NULL;
	
	/* Look through list */
	for (Rover = l_AllocChain; Rover; Rover = Rover->Next)
		// Match?
		if (a_Ptr == Rover->Data)
			return Rover;
	
	/* Not found */
	return NULL;
}

/* Z_Init() -- Initialize the malloc() based memory manager */
void Z_Init(void)
{
	// Nothing to do here
}

/* Z_TagUsage() -- Does nothing */
size_t Z_TagUsage(const Z_MemoryTag_t TagNum)
{
	return 0;
}

/* Z_CheckHeap() -- Does nothing */
void Z_CheckHeap(const int Code)
{
}

/* Z_MallocWrappee() -- Allocate data */
void* Z_MallocWrappee(const size_t Size, const Z_MemoryTag_t Tag, void** Ref _ZMGD_WRAPPEE)
{
	Z_Allocation_t* New;
	static bool_t ReDo = false;
	void* Ret;
	
	/* Check */
	if (!Size)
		return NULL;
	
	/* Create new */
	New = malloc(sizeof(*New));
	
	// Check
	if (New)
	{
		memset(New, 0, sizeof(*New));
		New->Prev = New->Next = NULL;
		Ret = New->Data = malloc(Size);
		New->User = Ref;
		New->Tag = Tag;
	}
	
	// Check
	if (!New || (New && !New->Data))
	{
		// Redone?
		if (ReDo)
		{
			I_Error("Z_Malloc: No more memory!\n");
			return NULL;
		}
		
		// Free cache tag
		Z_FreeTags(PU_PURGELEVEL, NUMZTAGS);
		
		// Try again
		ReDo = true;
		Ret = Z_Malloc(Size, Tag, Ref);
		ReDo = false;
		
		// Return it
		return Ret;
	}
	
	/* Clear */
	memset(Ret, 0, Size);
	
	// Set
	New->Tag = Tag;
	
	/* Link */
	if (!l_AllocChain)
		l_AllocChain = New;
	else
	{
		New->Next = l_AllocChain;
		l_AllocChain->Prev = New;
		l_AllocChain = New;
	}
	
	/* Return Data */
	return Ret;
}

/* Z_FreeWrappee() -- Free pointer */
void Z_FreeWrappee(void* const Ptr _ZMGD_WRAPPEE)
{
	Z_Allocation_t* Alloc;
	
	/* Check */
	if (!Ptr)
		return;
	
	/* Find pointer */
	Alloc = ZS_AllocForPtr(Ptr);
	
	// Not found?
	if (!Alloc)
		return;
	
	/* Clear user */
	if (Alloc->User)
		*Alloc->User = NULL;
	
	/* Free everything else */
	free(Alloc->Data);
	
	/* Re-chain */
	if (Alloc == l_AllocChain)
	{
		if (Alloc->Next)
			l_AllocChain = Alloc->Next;
		else
			l_AllocChain = Alloc->Prev;
	}
	
	if (Alloc->Prev)
		Alloc->Prev->Next = Alloc->Next;
	if (Alloc->Next)
		Alloc->Next->Prev = Alloc->Prev;
	
	
	// Free allocation
	free(Alloc);
}

size_t Z_FreeTagsWrappee(const Z_MemoryTag_t LowTag, const Z_MemoryTag_t HighTag _ZMGD_WRAPPEE)
{
	Z_Allocation_t* Rover;
	Z_Allocation_t* Next;
	
	/* Look through list */
	for (Rover = l_AllocChain; Rover; Rover = Rover->Next)
	{
		// Match?
		if (Rover->Tag >= LowTag && Rover->Tag < HighTag)
		{
			Next = Rover->Next;
			Z_Free(Rover->Data);
			Rover = Next;
		}
	}
}

/* Z_GetTagFromPtrWrappee() -- Returns tag from this pointer */
Z_MemoryTag_t Z_GetTagFromPtrWrappee(void* const Ptr _ZMGD_WRAPPEE)
{
	Z_Allocation_t* Alloc;
	
	/* Check */
	if (!Ptr)
		return;
	
	/* Find pointer */
	Alloc = ZS_AllocForPtr(Ptr);
	
	// Not found?
	if (!Alloc)
		return;
	
	/* Return tag */
	return Alloc->Tag;
}

/* Z_ChangeTagWrappee() -- Changes this pointer's tag (returns old tag) */
Z_MemoryTag_t Z_ChangeTagWrappee(void* const Ptr, const Z_MemoryTag_t NewTag _ZMGD_WRAPPEE)
{
	Z_Allocation_t* Alloc;
	Z_MemoryTag_t OldTag;
	
	/* Check */
	if (!Ptr)
		return;
	
	/* Find pointer */
	Alloc = ZS_AllocForPtr(Ptr);
	
	// Not found?
	if (!Alloc)
		return;
	
	/* Return tag */
	OldTag = Alloc->Tag;
	
	// Modify
	Alloc->Tag = NewTag;
	
	// Return it
	return OldTag;
}

void Z_RegisterCommands(void)
{
}

#else

/* ZP_NewZone() -- Creates a new memory zone */
bool_t ZP_NewZone(size_t* const SizePtr, Z_MemZone_t** const ZoneRef)
{
	size_t ShiftSize;
	Z_MemZone_t* NewZone;
	Z_MemPartition_t* Part;
	
	/* Check */
	if (!SizePtr)
		return false;
		
	/* Obtain the shift size */
	ShiftSize = *SizePtr >> PARTSHIFT;
	
	/* Create an empty zone */
	NewZone = malloc(sizeof(*NewZone));
	
	// Failed?
	if (!NewZone)
		return false;
		
	// Clear it out
	memset(NewZone, 0, sizeof(*NewZone));
	
	/* Create huge chunk zone */
	NewZone->DataChunk = malloc(ShiftSize << PARTSHIFT);
	
	// Failed?
	if (!NewZone->DataChunk)
	{
		free(NewZone);
		return false;
	}
	
	// Clear it out
	memset(NewZone->DataChunk, 0, ShiftSize << PARTSHIFT);
	
	/* Create initial partitions */
	NewZone->PartitionList = malloc(sizeof(*NewZone->PartitionList) * INITPARTCOUNT);
	
	// Failed?
	if (!NewZone->PartitionList)
	{
		free(NewZone->DataChunk);
		free(NewZone);
		return false;
	}
	// Clear it out
	memset(NewZone->PartitionList, 0, sizeof(*NewZone->PartitionList) * INITPARTCOUNT);
	
	/* Initialize everything else in the zone stuff */
	NewZone->NumPartitions = 1;
	NewZone->MaxPartitions = INITPARTCOUNT;
	NewZone->TotalMemory = ShiftSize;
	NewZone->FreeMemory = ShiftSize;
	NewZone->AllocSize = ShiftSize << PARTSHIFT;
	
	// Initialize the first partition
	Part = NewZone->PartitionList;	// To save on typing
	
	Part->Part.Start = 0;
	Part->Part.End = NewZone->TotalMemory;
	Part->Part.Size = Part->Part.End - Part->Part.Start;
	Part->Part.Used = false;
	Part->Part.SelfRef = NewZone->DataChunk;
	
	memset(&Part->Block, 0, sizeof(Part->Block));	// Quick clear
	
	/* Set zone to last and increment the zone count */
	l_MainZone = NewZone;
	
	/* Set return values */
	if (ZoneRef)
		*ZoneRef = NewZone;
		
	*SizePtr = ShiftSize << PARTSHIFT;
	
	/* Return success */
	return true;
}

void* ZP_PointerForPartition(Z_MemPartition_t* const Part, void** const a_BasePtrPtr);

/* ZP_PartitionSplit() -- Splits a partition into two new pieces */
void ZP_PartitionSplit(Z_MemPartition_t* const ToSplit, Z_MemPartition_t** const ResLeftPtr, Z_MemPartition_t** const ResRightPtr, const size_t Pos,
                       const bool_t AtEnd)
{
	size_t pI, SplitPos, i;
	Z_MemPartition_t Copy;
	Z_MemPartition_t* NewList;
	void* Base;
	
	/* Check */
	if (!l_MainZone || !ToSplit || !ResLeftPtr || !ResRightPtr || !Pos)
		return;
		
	/* Partition cannot be used */
	if (ToSplit->Part.Used)
		return;
		
	/* Get location of partition in zone */
	pI = ToSplit - l_MainZone->PartitionList;
	
	// Check that
	if (pI >= l_MainZone->NumPartitions)
		return;
		
	/* Determine split point */
	// Base position is the partition start
	SplitPos = ToSplit->Part.Start;
	
	// Where do we attach?
	if (!AtEnd)
		SplitPos += Pos;
	else
		SplitPos += (ToSplit->Part.End - ToSplit->Part.Start) - Pos;
		
	/* Copy base partition before we lose it */
	Copy = *ToSplit;
	
	/* Depending if there is enough size or not */
	// Create new partitions if needed
	if (l_MainZone->NumPartitions >= (l_MainZone->MaxPartitions - (INITPARTCOUNT >> 2)))
	{
		// Allocate
		NewList = malloc(sizeof(Z_MemPartition_t) * (l_MainZone->MaxPartitions + INITPARTCOUNT));
		
		// Only if we successfully created a new list
		if (NewList)
		{
			// Clear the list
			memset(NewList, 0, sizeof(Z_MemPartition_t) * (l_MainZone->MaxPartitions + INITPARTCOUNT));
			
			// Copy all the old stuff over
			memmove(NewList, l_MainZone->PartitionList, sizeof(Z_MemPartition_t) * l_MainZone->MaxPartitions);
			
			// Free old stuff
			free(l_MainZone->PartitionList);
			
			// Set new list data
			l_MainZone->PartitionList = NewList;
			
			// Increment with the new max
			l_MainZone->MaxPartitions += INITPARTCOUNT;
		}
		// Failure
		else
		{
			// If we reached the breaking point, fail completely
			if (l_MainZone->NumPartitions >= (l_MainZone->MaxPartitions - 1))
				I_Error("ZP_PartitionSplit: Needed more partitions but ran out of memory.");
		}
	}
	// Move everything over
	memmove(&l_MainZone->PartitionList[pI + 1], &l_MainZone->PartitionList[pI], sizeof(Z_MemPartition_t) * (l_MainZone->NumPartitions - pI));
	l_MainZone->NumPartitions++;
	
	// Set left and right
	*ResLeftPtr = &l_MainZone->PartitionList[pI];
	*ResRightPtr = &l_MainZone->PartitionList[pI + 1];
	
	/* Clear result partitions */
	memset(*ResLeftPtr, 0, sizeof(**ResLeftPtr));
	memset(*ResRightPtr, 0, sizeof(**ResRightPtr));
	
	/* Set partition data of left side */
	(*ResLeftPtr)->Part.Start = Copy.Part.Start;
	(*ResLeftPtr)->Part.End = SplitPos;
	(*ResLeftPtr)->Part.Size = (*ResLeftPtr)->Part.End - (*ResLeftPtr)->Part.Start;
	
	(*ResRightPtr)->Part.Start = SplitPos;
	(*ResRightPtr)->Part.End = Copy.Part.End;
	(*ResRightPtr)->Part.Size = (*ResRightPtr)->Part.End - (*ResRightPtr)->Part.Start;
	
	/* Reset reference pointers */
	for (i = pI + 1; i < l_MainZone->NumPartitions; i++)
	{
		ZP_PointerForPartition(&l_MainZone->PartitionList[i], &Base);
		//fprintf(stderr, "ss %i -", *((uint32_t*)Base));
		l_MainZone->PartitionList[i].Part.SelfRef = Base;
		*((uint32_t*)Base) = i;
		//fprintf(stderr, " %i\n", *((uint32_t*)Base));
	}
}

/* ZP_PointerForPartition() -- Returns the pointer to the start of the partition */
// GhostlyDeath <December 14, 2011> -- Account for wasted size
void* ZP_PointerForPartition(Z_MemPartition_t* const Part, void** const a_BasePtrPtr)
{
	uintptr_t Base;
	
	/* Check */
	if (!l_MainZone || !Part)
		return NULL;
		
	/* Return base zone plus the shifted size of partition */
	Base = (((uintptr_t) l_MainZone->DataChunk) + (Part->Part.Start << PARTSHIFT));
	
	if (a_BasePtrPtr)
		*a_BasePtrPtr = (void*)Base;
	return (void*)((uint8_t*)Base + ZPARTEXTRASIZE);
}

/* ZP_FindPointerForPartition() -- Finds a pointer that was found in a partition (if any) */
// GhostlyDeath <December 14, 2011> -- This function is slow!!!
bool_t ZP_FindPointerForPartition(void* const Ptr, Z_MemPartition_t** const PartPtr)
{
	size_t i;
	uintptr_t Back;
	uint32_t* BackP;
	Z_MemPartition_t* Part;
	
	/* Check */
	if (!Ptr || !l_MainZone || !PartPtr)
		return false;
		
	/* Search in main zone */
	// Back slap and see if it works
	Back = (uintptr_t)Ptr - ZPARTEXTRASIZE;
	BackP = (uint32_t*)Back;
	
	// Check number and see if it is this block
	if (*BackP < l_MainZone->NumPartitions)
		if (l_MainZone->PartitionList[*BackP].Part.SelfRef == BackP)
		{
			Part = &l_MainZone->PartitionList[*BackP];
			//fprintf(stderr, "Quickly %x\n", Part);
			*PartPtr = Part;
		
			// Return for success
			return true;
		}
	
	// Search the zone partition by partition for this pointer (SLOW!)
	for (i = 0; i < l_MainZone->NumPartitions; i++)
	{
		// Get local pointer
		Part = &l_MainZone->PartitionList[i];
		
		// Partition not used? (pointers only for freeed blocks)
		if (!Part->Part.Used)
			continue;
			
		// Partition pointer does not match?
		if (Part->Block.Ptr != Ptr)
			continue;
			
		// A match! So set the zone and part
		*PartPtr = Part;
		
		// Return for success
		return true;
	}
	
	/* Failure */
	return false;
}

/* ZP_MergePartitions() -- Merges partitions */
static size_t ZP_MergePartitions(Z_MemPartition_t* const Part)
{
	size_t i;
	size_t RetVal;
	Z_MemPartition_t* Mergee;
	Z_MemPartition_t LP, RP;
	
	/* Check */
	if (!Part)
		return 0;
	
	/* Get local partition id */
	i = Part - l_MainZone->PartitionList;
	
	/* As long as there is partition before us and it is free, go to that */
	Mergee = Part;
	RetVal = 0;
	
	while (i > 0 && !l_MainZone->PartitionList[i - 1].Part.Used)
	{
		Mergee--;				// Pointers are fun
		i--;
	}
	
	/* As long as a partition is to the right of us, merge into it */
	while (i + 1 < l_MainZone->NumPartitions && !l_MainZone->PartitionList[i + 1].Part.Used)
	{
		// Get original left and right, copy them
		LP = l_MainZone->PartitionList[i];	// also Mergee
		RP = l_MainZone->PartitionList[i + 1];
		
		// Slap back zones
		memmove(&l_MainZone->PartitionList[i], &l_MainZone->PartitionList[i + 1], sizeof(Z_MemPartition_t) * (l_MainZone->NumPartitions - i));
		
		// Decrement partition count
		l_MainZone->NumPartitions--;
		
		// Clear block data, it should be free anyway
		memset(Mergee, 0, sizeof(*Mergee));
		
		// Set data from copies
		Mergee->Part.Start = LP.Part.Start;
		Mergee->Part.End = RP.Part.End;
		Mergee->Part.Size = LP.Part.Size + RP.Part.Size;
		
		// Increment retval since we just lost a partition
		RetVal++;
	}
	
	/* Fix all partition self references */
	// Since the references are no longer valid for merged partitions
	for (; i < l_MainZone->NumPartitions; i++)
	{
		// Quick Ref
		Mergee = &l_MainZone->PartitionList[i];
		
		// Reset reference
		ZP_PointerForPartition(Mergee, &Mergee->Part.SelfRef);
		*Mergee->Part.SelfRef = i;
	}

	return RetVal;
}

/* ZP_FreePartitionInZone() -- Frees selected partition in selected zone */
// Returns (negative) result of partitions merged away (NewNP = OldNumPartitions - RetVal)
size_t ZP_FreePartitionInZone(Z_MemPartition_t* const Part)
{
	size_t i;
	size_t RetVal;
	Z_MemPartition_t* Mergee;
	Z_MemPartition_t LP, RP;
	
	/* Check */
	if (!Part)
		return 0;
		
	/* Partition free? */
	if (!Part->Part.Used)
	{
		I_Error("ZP_FreePartitionInZone: Freeing a partition already free?");
		return 0;
	}
	
	// Add free memory
	l_MainZone->FreeMemory += Part->Part.Size;
	
	/* Lazy fragment freeing */
	// Is there a reference set?
	if (Part->Block.Ref)
		*Part->Block.Ref = NULL;
		
	// Clear out data with a simple sweep
	memset(&Part->Block, 0, sizeof(Part->Block));
	
	// No longer used anymore
	Part->Part.Used = false;
	
	// Clear return value
	RetVal = 0;
	
	/* If the partition was at the end, merge */
	// Merging at the end has less cost than merging at the start, plus the
	// stuff at the end is smaller.
	if (Part->Part.AtEnd)
	{
		// Merge
		RetVal = ZP_MergePartitions(Part);
		
		// Clear at end
		Part->Part.AtEnd = false;
	}
	
	/* Return number of merged partitions */
	return RetVal;
}

/****************
*** FUNCTIONS ***
****************/

/* Z_Init() -- Initializes the memory manager */
void Z_Init(void)
{
	uint64_t FreeMem = 0, TotalMem;
	size_t LoopSize, i;
	bool_t Ok = false;
	uint32_t Sanity = 0;
	
	/* Sanity check */
	// Place in byte
	*((uint8_t*)&Sanity) = 1;
	
	// Check
#if !defined(__REMOOD_BIG_ENDIAN)
	if (Sanity != 1)
#else
	if (Sanity != 0x01000000)
#endif
		CONL_PrintF("Z_Init: ReMooD was compiled with the wrong byte order!\n");
	
	/* Create new zone in a loop */
	// Check free memory
	FreeMem = I_GetFreeMemory(&TotalMem);
	
	// Lots of memory?
	if (FreeMem >= (uint64_t)0x7FFFFFFFUL)
		FreeMem = (uint64_t)0x7FFFFFFFUL;
		
	if (TotalMem >= (uint64_t)0x7FFFFFFFUL)
		TotalMem = (uint64_t)0x7FFFFFFFUL;
	
	// Print memory that is free
	if (devparm)
		CONL_PrintF("Z_Init: There is %iMiB of memory available (Total: %iMiB).\n", (int)(FreeMem >> 20), (int)(TotalMem >> 20));
	
	// Less than 20MB?
	if (FreeMem < FORCEDMEMORYSIZE)
	{
		if (devparm)
			CONL_PrintF("Z_Init: Reported value below %iMiB, capping.\n", FORCEDMEMORYSIZE >> 20);
			
		FreeMem = FORCEDMEMORYSIZE;
	}
	// Initial size
	if (M_CheckParm("-mb") && M_IsNextParm())
		LoopSize = atoi(M_GetNextParm()) << 20;
	else
		LoopSize = (FreeMem < (32 << 20) ? FreeMem : (32 << 20));
		
	// Size cap?
	if (LoopSize < MINZONESIZE)
		LoopSize = MINZONESIZE;
		
	// Cap to power of two
	for (i = (sizeof(size_t) * 8) - 1; i > 0; i--)
		if (LoopSize & (1 << i))
		{
			LoopSize = 1 << i;
			break;
		}
	// Creation loop
	while (LoopSize > MINZONESIZE && !(Ok = ZP_NewZone(&LoopSize, NULL)))
		LoopSize -= LoopSize >> 2;	// Cut down by 1/4th
}

size_t WX_ClearUnused(void);

/* Z_MallocWrappee() -- Allocate memory */
void* Z_MallocWrappee(const size_t Size, const Z_MemoryTag_t Tag, void** const Ref _ZMGD_WRAPPEE)
{
	size_t i, ShiftSize, iBase, iEnd, j, k;
	Z_MemPartition_t* ResLeft, *ResRight, *New, *Free;
	void* RetVal;
	bool_t AtEnd;
	static int DeferLevel;
	void* BasePtr;
	
	/* Clear some */
	RetVal = NULL;
	ResLeft = ResRight = New = Free = NULL;
	
	/* Size for shift */
	ShiftSize = ((Size + ZPARTEXTRASIZE) >> PARTSHIFT) + 1;
	
	// Do we allocate at the end?
	if (ShiftSize < RIGHTSIZE)
		AtEnd = true;
	else
		AtEnd = false;
		
	// Determine the base and end
	// At the start
	if (!AtEnd)
	{
		iBase = 0;
		iEnd = l_MainZone->NumPartitions;
	}
	// At the end
	else
	{
		iBase = l_MainZone->NumPartitions - 1;
		iEnd = (size_t) - 1;
	}
	
	// Search every partition
	for (i = iBase; i != iEnd; (AtEnd ? i-- : i++))
	{
		// Partition used?
		if (l_MainZone->PartitionList[i].Part.Used)
			continue;
			
		// Partition too small?
		if (l_MainZone->PartitionList[i].Part.Size < ShiftSize)
		{
			// Partition is free and the next partition is free
			if (!AtEnd)	// Never do this at the end
				if (!l_MainZone->PartitionList[(AtEnd ? (i - 1) : (i + 1))].Part.Used)
					// Merge partitions
					ZP_MergePartitions(&l_MainZone->PartitionList[i].Part);
			
			// Recheck to see if the partition is too small
			if (l_MainZone->PartitionList[i].Part.Size < ShiftSize)
				continue;
		}
			
		// Not a perfect fit? Then we need to split
		if (l_MainZone->PartitionList[i].Part.Size != ShiftSize)
		{
			// Clear results
			ResLeft = ResRight = NULL;
			
			// Split partition
			ZP_PartitionSplit(&l_MainZone->PartitionList[i], &ResLeft, &ResRight, ShiftSize, AtEnd);
			
			// Big block?
			if (!AtEnd)
			{
				New = ResLeft;
				Free = ResRight;
			}
			// Tiny Block?
			else
			{
				New = ResRight;
				Free = ResLeft;
			}
		}
		// A perfect fit so just use the entire block
		else
		{
			New = &l_MainZone->PartitionList[i];
			Free = NULL;
		}
		
		// Set used in new
		New->Part.Used = true;
		New->Part.AtEnd = AtEnd;
		
		// Set block info
		New->Block.Ptr = ZP_PointerForPartition(New, &BasePtr);
		New->Block.Ref = Ref;
		New->Block.Size = Size;
		
		// Block Tag (if it is invalid, just make it static to be sure)
		if ((size_t) Tag >= NUMZTAGS)
			New->Block.Tag = PU_STATIC;
		else
			New->Block.Tag = Tag;
			
		// Set return value
		RetVal = New->Block.Ptr;
		
		// Complementary memset
		memset(RetVal, 0, Size);
		
		// Subtract free memory
		l_MainZone->FreeMemory -= New->Part.Size;
		
		// Set Ref -- For some reason this causes sprites to not draw!
		//if (Ref)
			//*Ref = RetVal;
		
		// Return pointer
		return RetVal;
	}
	
	/* Failure? */
	// If we double looped, fail
	if (DeferLevel)
	{
		I_Error("Z_MallocReal: Failed to allocate %zu bytes.\n", Size);
		return NULL;
	}
	// Run another loop
	else
	{
		// Clear cache blocks and unused lumps
		k = l_MainZone->FreeMemory;
		i = Z_FreeTags(PU_PURGELEVEL, NUMZTAGS);
		j = WX_ClearUnused();
		
		if (devparm)
			CONL_PrintF("Z_MallocReal: Nearly out of memory, freed %u lumps, %u blocks (Freed %u bytes).\n", j, i, l_MainZone->FreeMemory - k);
			
		// Now try re-allocation
		DeferLevel = 1;
		RetVal = Z_Malloc(Size, Tag, Ref);
		DeferLevel = 0;
		return RetVal;
	}
}

/* Z_FreeWrappee() -- Free memory */
void Z_FreeWrappee(void* const Ptr _ZMGD_WRAPPEE)
{
	Z_MemPartition_t* Part;
	
	/* Check */
	if (!Ptr)
		return;
		
	/* Find zone and partition relating to pointer */
	// Clear
	Part = NULL;
	
	// Now call
	if (!ZP_FindPointerForPartition(Ptr, &Part))
		return;					// not found
		
	/* Send zone along with partition to my free function */
	ZP_FreePartitionInZone(Part);
}

/* Z_ChangeTagWrappee() -- Change memory tag returning the old one */
Z_MemoryTag_t Z_ChangeTagWrappee(void* const Ptr, const Z_MemoryTag_t NewTag _ZMGD_WRAPPEE)
{
	Z_MemoryTag_t OldTag;
	Z_MemPartition_t* Part;
	
	/* Check */
	if (!Ptr)
		return ((Z_MemoryTag_t) - 1);
		
	/* Find zone and partition relating to pointer */
	// Clear
	Part = NULL;
	
	// Now call
	if (!ZP_FindPointerForPartition(Ptr, &Part))
		return ((Z_MemoryTag_t) - 1);	// not found
		
	/* Remember old tag */
	OldTag = Part->Block.Tag;
	
	// Validate new tag (if it is invalid, just make it static)
	if ((size_t) NewTag >= NUMZTAGS)
		Part->Block.Tag = PU_STATIC;
	else
		Part->Block.Tag = NewTag;
		
	// Return the old tag
	return OldTag;
}

/* Z_GetTagFromPtrWrappee() -- Return the memory tag associated with a pointer */
Z_MemoryTag_t Z_GetTagFromPtrWrappee(void* const Ptr _ZMGD_WRAPPEE)
{
	Z_MemoryTag_t RetVal;
	Z_MemPartition_t* Part;
	
	/* Check */
	if (!Ptr)
		return ((Z_MemoryTag_t) - 1);
		
	/* Find zone and partition relating to pointer */
	// Clear
	Part = NULL;
	
	// Now call
	if (!ZP_FindPointerForPartition(Ptr, &Part))
		return ((Z_MemoryTag_t) - 1);	// not found
		
	/* Remember the tag */
	RetVal = Part->Block.Tag;
	
	// Return the tag
	return RetVal;
}

/* Z_GetRefFromPtrWrappee() -- Returns the reference of a block */
void** Z_GetRefFromPtrWrappee(void* const Ptr _ZMGD_WRAPPEE)
{
	void** RetVal;
	Z_MemPartition_t* Part;
	
	/* Check */
	if (!Ptr)
		return NULL;
		
	/* Find zone and partition relating to pointer */
	// Clear
	Part = NULL;
	
	// Now call
	if (!ZP_FindPointerForPartition(Ptr, &Part))
		return NULL;			// not found
		
	/* Remember the reference */
	RetVal = Part->Block.Ref;
	
	// Return the tag
	return RetVal;
}

/* Z_FreeTagsWrappee() -- Free all tags between low inclusive and high exclusive */
size_t Z_FreeTagsWrappee(const Z_MemoryTag_t LowTag, const Z_MemoryTag_t HighTag _ZMGD_WRAPPEE)
{
	size_t i, j, FreeCount, Lost;
	Z_MemPartition_t* Part;
	
	/* Clear free count */
	FreeCount = 0;
	
	// Zone does not exist?
	if (!l_MainZone)
		return 0;
		
	// Go through each partition in the zone
	for (j = 0; j < l_MainZone->NumPartitions; j++)
	{
		// Copy partition
		Part = &l_MainZone->PartitionList[j];
		
		// Partition not used?
		if (!Part->Part.Used)
			continue;
			
		// Tags not in bounds?
		if (Part->Block.Tag < LowTag || Part->Block.Tag > HighTag)
			continue;
			
		// Free this partition
		Lost = ZP_FreePartitionInZone(Part);
		
		// Increment free count
		FreeCount++;
		
		// Subtract lost partitions
		if (j - Lost > j)		// Negative
			j = 0;
		else					// Backtrace to said partition (leftmost free)
			j -= Lost;
	}
	
	/* Return freed stuff */
	return FreeCount;
}

/* ZS_MemInfo() -- "meminfo" -- Print memory information */
static CONL_ExitCode_t ZS_MemInfo(const uint32_t a_ArgC, const char** const a_ArgV)
{
	size_t i;
	Z_MemPartition_t* Cur;
	
	uint64_t SysTotalLL;
	uint32_t SysUsed, SysTotal;
	
	uint32_t ZAlloc, ZTotal, ZUsed;
	uint32_t ATotal, AUsed;
	uint32_t DTotal, DUsed;
	uint32_t WTotal;
	
	uint32_t PUsed, PTotal, PMax, PBytes;
	
	/* Get system totals */
	SysUsed = I_GetFreeMemory(&SysTotalLL);
	SysTotal = SysTotalLL;
	SysUsed = SysTotal - SysUsed;
	
	/* Go through the memory manager */
	// Get zone reports
	ZAlloc = l_MainZone->AllocSize;
	ZTotal = l_MainZone->TotalMemory << PARTSHIFT;
	ZUsed = ZTotal - (l_MainZone->FreeMemory << PARTSHIFT);
	
	// Find actual memory
	PUsed = ATotal = AUsed = DTotal = DUsed = WTotal = 0;
	PTotal = l_MainZone->NumPartitions;
	PMax = l_MainZone->MaxPartitions;
	PBytes = l_MainZone->MaxPartitions * sizeof(Z_MemPartition_t);
	
	for (i = 0; i < l_MainZone->NumPartitions; i++)
	{
		// Get current partition
		Cur = &l_MainZone->PartitionList[i];
		
		// Add actual size
		ATotal += Cur->Part.Size << PARTSHIFT;
		DTotal += Cur->Part.Size << PARTSHIFT;
		
		// If this block is used, add to used totals
		if (Cur->Part.Used)
		{
			AUsed += Cur->Part.Size << PARTSHIFT;
			DUsed += Cur->Block.Size;
			PUsed++;
		}
	}
	
	// Wasted is the AUsed - DUsed;
	WTotal = AUsed - DUsed;
	
	/* Print Report */
	CONL_PrintF("{9*** MEMORY REPORT ***{z\n");
	CONL_PrintF("{zSystem : {3%8u{zKiB/{4%8u{zKiB\n", SysUsed >> 10, SysTotal >> 10);
	CONL_PrintF("{zClaimed: {3%8u{zKiB\n", ZAlloc >> 10);
	CONL_PrintF("{zReport : {3%8u{zKiB/{4%8u{zKiB\n", ZUsed >> 10, ZTotal >> 10);
	CONL_PrintF("{zActual : {3%8u{zKiB/{4%8u{zKiB\n", AUsed >> 10, ATotal >> 10);
	CONL_PrintF("{zData   : {3%8u{zKiB/{4%8u{zKiB\n", DUsed >> 10, DTotal >> 10);
	CONL_PrintF("{zWasted : {3%8u{zKiB\n", WTotal >> 10);
	CONL_PrintF("{zParts  : {3%8u{z/{4%8u\n", PUsed, PTotal);
	CONL_PrintF("{zPartUtl: {3%8u{z/{4%8u\n", PTotal, PMax);
	CONL_PrintF("{zPartSiz: {3%8u{zKiB\n", PBytes >> 10);
	
	/* Success */
	return CLE_SUCCESS;
}

/* ZS_MemCacheFree() -- "memcachefree" -- Free all cached memory */
static CONL_ExitCode_t ZS_MemCacheFree(const uint32_t a_ArgC, const char** const a_ArgV)
{
	uint32_t FreeCount;
	
	/* Free all cache tags */
	FreeCount = Z_FreeTags(PU_PURGELEVEL, NUMZTAGS);
	
	/* Print Info */
	CONL_PrintF("Freed %u blocks.\n", FreeCount);
	
	/* Success */
	return CLE_SUCCESS;
}

/* ZS_MemFrag() -- "memfrag" -- Show memory fragmentation */
static CONL_ExitCode_t ZS_MemFrag(const uint32_t a_ArgC, const char** const a_ArgV)
{
#define MAXCHARS (30 * 10)
	uint32_t BytesPerChar;
	uint32_t CurByte, Diff, Left;
	size_t i, j, CurCol, c;
	Z_MemPartition_t* Cur;
	bool_t LastUsed;
	
	static const struct
	{
		char Char;
		char Color;
	} CharBits[8] =
	{
		{'.', '4'},
		{',', '3'},
		{'-', '3'},
		{'-', '2'},
		{'=', '2'},
		{'=', '2'},
		{'#', '2'},
		{'#', '1'},
	};
	
	uint32_t Used, Free, Total;
	
	/* Obtain bytes per character */
	BytesPerChar = l_MainZone->AllocSize / MAXCHARS;
	
	/* Message */
	CONL_PrintF("{zPrinting memory map: '#' = %uB\n", BytesPerChar);
	
	/* Print characters */
	for (Total = Used = Free = Left = 0, CurCol = 0, i = 0, c = 0; c < MAXCHARS; c++)
	{
		// Calculate totals until memory cap is reached
		while (Total < BytesPerChar)
		{
			// No more partitions?
			if (i >= l_MainZone->NumPartitions)
				break;
			
			// Get current partition
			Cur = &l_MainZone->PartitionList[i];
			
			// Add to totals
			Total += Cur->Part.Size << PARTSHIFT;
			
			// Used or free?
			if (Cur->Part.Used)
				Used += Cur->Part.Size << PARTSHIFT;
			else
				Free += Cur->Part.Size << PARTSHIFT;
			
			// Go to next partition
			i++;
		}
		
		// Find out which character to use
		for (j = 0; j < 7; j++)
			if (Used <= (((Total >> 3) * j) + 1))
				break;
		
		// Subtract totals for the next
		Total -= BytesPerChar;
		
		if (Used < BytesPerChar)
			Used = 0;
		else
			Used -= BytesPerChar;
		
		if (Free < BytesPerChar)
			Free = 0;
		else
			Free -= BytesPerChar;
		
		// Print char and go to next column
		CONL_PrintF("{%c%c", CharBits[j].Color, CharBits[j].Char);
		CurCol++;
		
		// Next line?
		if (CurCol == 30)
		{
			CurCol = 0;
			CONL_PrintF("\n");
		}
	}

	// So it flushes
	CONL_PrintF("{z\n");
	
	/* Success */
	return CLE_SUCCESS;
}

/* Z_RegisterCommands() -- Register commands to the console */
void Z_RegisterCommands(void)
{
	CONL_AddCommand("meminfo", ZS_MemInfo);
	CONL_AddCommand("memcachefree", ZS_MemCacheFree);
	CONL_AddCommand("memfrag", ZS_MemFrag);
}

#endif

/***********************
*** COMMON FUNCTIONS ***
***********************/

/* Z_ResizeArray() -- Resizes an array */
void Z_ResizeArray(void** const PtrPtr, const size_t ElemSize, const size_t OldSize, const size_t NewSize)
{
	void* Temp;
	Z_MemoryTag_t OldTag;
	
	/* Check */
	if (!PtrPtr || !ElemSize || OldSize == NewSize)
		return;
		
	/* Free? */
	// Only if the newsize is zero
	if (!NewSize)
	{
		Z_Free(*PtrPtr);
		*PtrPtr = NULL;
		return;
	}
	
	/* Change info from PtrPtr */
	// Find the old tag
	if (*PtrPtr)
	{
		OldTag = Z_GetTagFromPtr(*PtrPtr);
		
		// Force tag to be static
		Z_ChangeTag(*PtrPtr, PU_STATIC);
	}
	else
		OldTag = PU_STATIC;
		
	/* Allocate temp for new size */
	Temp = Z_Malloc(ElemSize * NewSize, PU_STATIC, NULL);
	
	/* If *PtrPtr is set then manage it */
	if (*PtrPtr)
	{
		// Copy all the old data
		memmove(Temp, *PtrPtr, ElemSize * (NewSize < OldSize ? NewSize : OldSize));
		
		// Free it
		Z_Free(*PtrPtr);
	}
	
	/* Set *PtrPtr to temp */
	*PtrPtr = Temp;
	
	// Restore the old tag
	Z_ChangeTag(*PtrPtr, OldTag);
}

char* Z_StrDup(const char* const String, const Z_MemoryTag_t Tag, void** Ref)
{
	size_t n;
	char* Ptr;
	
	/* Check */
	if (!String)
		return NULL;
		
	/* Copy */
	n = strlen(String);
	Ptr = Z_Malloc(sizeof(char) * (n + 1), Tag, Ref);
	strcpy(Ptr, String);
	
	/* Return */
	return Ptr;
}

/* Z_StrDup() -- Duplicate a string */
char* Z_Strdup(const char* const String, const Z_MemoryTag_t Tag, void** Ref)
{
	return Z_StrDup(String, Tag, Ref);
}

/*******************
*** HASH UTILITY ***
*******************/

/*** STRUCTURES ***/

/* Z_HashKey_t -- A single key in a hash table */
typedef struct Z_HashKey_s
{
	uint32_t Key;
	void* Data;
} Z_HashKey_t;

/* Z_HashTable_s -- A hash table */
struct Z_HashTable_s
{
	Z_HashKey_t* KeyList[256];	// Key list
	size_t KeySize[256];		// Keys in list
	bool_t (*CompareFunc) (void* const a_A, void* const a_B);
};

/*** FUNCTIONS ***/

/* Z_Hash() -- Hashes a string */
uint32_t Z_Hash(const char* const a_Str)
{
	uint32_t Ret = 0;
	size_t i;
	
	/* Check */
	if (!a_Str)
		return 0;
		
	/* Hash loop */
	for (i = 0; a_Str[i]; i++)
		Ret ^= (uint32_t)((toupper(a_Str[i]) - 32) & 0x3F) << (6 * (i % 5));
		
	/* Return */
	return Ret;
}

/* Z_HashCreateTable() -- Creates a new hashing table */
Z_HashTable_t* Z_HashCreateTable(bool_t (*a_CompareFunc) (void* const a_A, void* const a_B))
{
	Z_HashTable_t* New;
	
	/* Allocate and function */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	New->CompareFunc = a_CompareFunc;
	
	/* Return new table */
	return New;
}

/* Z_HashDeleteTable() -- Deletes a hash table */
void Z_HashDeleteTable(Z_HashTable_t* const a_HashTable)
{
	size_t i;
	
	/* Check */
	if (!a_HashTable)
		return;
		
	/* Free it */
	for (i = 0; i < 256; i++)
		if (a_HashTable->KeyList[i])
			Z_Free(a_HashTable->KeyList[i]);
	Z_Free(a_HashTable);
}

/* Z_HashAddEntry() -- Adds a single entry to the hash table */
bool_t Z_HashAddEntry(Z_HashTable_t* const a_HashTable, const uint32_t a_Key, void* const a_Data)
{
	uint32_t Nub;
	size_t i;
	
	/* Check */
	if (!a_HashTable || !a_Data)
		return false;
		
	/* Get nub of key */
	Nub = a_Key & 0xFF;
	
	/* Resize list */
	Z_ResizeArray((void**)&a_HashTable->KeyList[Nub], sizeof(Z_HashKey_t), a_HashTable->KeySize[Nub], a_HashTable->KeySize[Nub] + 1);
	
	// Slap at end
	i = a_HashTable->KeySize[Nub]++;
	a_HashTable->KeyList[Nub][i].Key = a_Key;
	a_HashTable->KeyList[Nub][i].Data = a_Data;
	
	/* Success! */
	return true;
}

/* Z_HashFindEntry() -- Finds an entry in the hash table */
void* Z_HashFindEntry(Z_HashTable_t* const a_HashTable, const uint32_t a_Key, void* const a_DataSim, const bool_t a_BackRun)
{
	uint32_t Nub;
	size_t i;
	
	/* Check */
	if (!a_HashTable)
		return NULL;
		
	/* Get nub */
	Nub = a_Key & 0xFF;
	
	/* Seek based on direction */
	for (i = (a_BackRun ? a_HashTable->KeySize[Nub] - 1 : 0); i != (a_BackRun ? (size_t) - 1 : a_HashTable->KeySize[Nub]); i = (a_BackRun ? i - 1 : i + 1))
	{
		// Compare for key equality
		if (a_Key == a_HashTable->KeyList[Nub][i].Key)
		{
			// If there is a compare function and data is being passed
			if (a_HashTable->CompareFunc && a_DataSim)
				if (!a_HashTable->CompareFunc(a_DataSim, a_HashTable->KeyList[Nub][i].Data))
					continue;
					
			// Found it!
			return a_HashTable->KeyList[Nub][i].Data;
		}
	}
	
	/* Found nothing */
	return NULL;
}

/* Z_HashDeleteEntry() -- Deletes the found entry from the hash table */
bool_t Z_HashDeleteEntry(Z_HashTable_t* const a_HashTable, const uint32_t a_Key, void* const a_DataSim, const bool_t a_BackRun)
{
	uint32_t Nub;
	size_t i;
	
	/* Check */
	if (!a_HashTable)
		return true;
		
	/* Get nub */
	Nub = a_Key & 0xFF;
	
	/* Seek based on direction */
	for (i = (a_BackRun ? a_HashTable->KeySize[Nub] - 1 : 0); i != (a_BackRun ? (size_t) - 1 : a_HashTable->KeySize[Nub]); i = (a_BackRun ? i - 1 : i + 1))
	{
		// Compare for key equality
		if (a_Key == a_HashTable->KeyList[Nub][i].Key)
		{
			// If there is a compare function and data is being passed
			if (a_HashTable->CompareFunc && a_DataSim)
				if (!a_HashTable->CompareFunc(a_DataSim, a_HashTable->KeyList[Nub][i].Data))
					continue;
					
			// Smash the table
			memmove(&a_HashTable->KeyList[Nub][i], &a_HashTable->KeyList[Nub][i + 1], a_HashTable->KeySize[Nub] - i - 1);
			
			// Shrink array
			Z_ResizeArray((void**)&a_HashTable->KeyList[Nub], sizeof(Z_HashKey_t), a_HashTable->KeySize[Nub], a_HashTable->KeySize[Nub] - 1);
			
			// Shrink count
			a_HashTable->KeySize[Nub]--;
			
			return true;
		}
	}
	
	/* Found nothing */
	return false;
}

/********************
*** TABLE UTILITY ***
********************/

/*** STRUCTURES ***/

/* Z_TableIndex_t -- Table properties */
typedef struct Z_TableIndex_s
{
	char* SubKey;				// Key for index
	uint32_t SubHash;			// Hash for subkey
	char* Value;				// Value for index
} Z_TableIndex_t;

/* Z_TableEntry_t -- A single entry in a table */
typedef struct Z_TableEntry_s
{
	bool_t IsTable;				// Is a table link
	union
	{
		Z_TableIndex_t Index;	// Key index
		Z_Table_t* TableLink;	// Link to another table
	} Data;						// Data for entry
} Z_TableEntry_t;

/* Z_Table_t -- A table */
struct Z_Table_s
{
	char* Key;					// Table key
	uint32_t Hash;				// Hash for table key
	Z_TableEntry_t** Entries;	// Entries as pointers to
	size_t NumEntries;			// Number of entries
	Z_HashTable_t* EntryHashes;	// Subentry hash lookup
	Z_Table_t* ParentTable;		// Owner table
};

/*** FUNCTIONS ***/

/* Z_TableCompareFunc() -- Compares passed data */
static bool_t Z_TableCompareFunc(void* const a_A, void* const a_B)
{
	Z_TableEntry_t* B;
	
	/* Check */
	if (!a_A || !a_B)
		return false;
		
	/* Get B */
	B = a_B;
	
	/* Is a table? */
	if (B->IsTable)
	{
		if (strcasecmp(a_A, B->Data.TableLink->Key) == 0)
			return true;
	}
	
	/* Is an index */
	else
	{
		if (strcasecmp(a_A, B->Data.Index.SubKey) == 0)
			return true;
	}
	
	/* No match */
	return false;
}

/* Z_TableCreate() -- Creates a new table */
Z_Table_t* Z_TableCreate(const char* const a_Key)
{
	Z_Table_t* NewTable;
	
	/* Allocate */
	NewTable = Z_Malloc(sizeof(*NewTable), PU_STATIC, NULL);
	
	// Set key
	NewTable->Key = Z_StrDup(a_Key, PU_STATIC, NULL);
	NewTable->Hash = Z_Hash(NewTable->Key);
	
	// Create hashing table
	NewTable->EntryHashes = Z_HashCreateTable(Z_TableCompareFunc);
	
	/* Return table */
	return NewTable;
}

/* Z_TableDestroy() -- Destroys an existing table */
void Z_TableDestroy(Z_Table_t* const a_Table)
{
	size_t i;
	Z_TableEntry_t* Ref;
	Z_Table_t* SubTable, *UpTable;
	
	/* Check */
	if (!a_Table)
		return;
		
	/* If we have a parent, unlink us from them */
	if (a_Table->ParentTable)
	{
		UpTable = a_Table->ParentTable;
	}
	
	/* Go through everything */
	for (i = 0; i < a_Table->NumEntries; i++)
	{
		// Get reference
		Ref = a_Table->Entries[i];
		
		// Is this a table?
		if (Ref->IsTable)
		{
			// Obtain subtable
			SubTable = Ref->Data.TableLink;
			
			// Clear parent
			SubTable->ParentTable = NULL;
			
			// Now delete that table
			Z_TableDestroy(SubTable);
		}
		
		// Otherwise it is an index
		else
		{
			Z_Free(Ref->Data.Index.SubKey);
			Z_Free(Ref->Data.Index.Value);
		}
		
		// Finished with entry, so destroy it
		Z_Free(Ref);
	}
	
	/* Destroy the remainder of the stuff */
	Z_Free(a_Table->Entries);
	Z_HashDeleteTable(a_Table->EntryHashes);
	
	// Now self
	Z_Free(a_Table);
}

/* Z_TableUp() -- Returns the parent of the table */
Z_Table_t* Z_TableUp(Z_Table_t* const a_Table)
{
	/* Check */
	if (!a_Table)
		return NULL;
		
	/* Easy */
	return a_Table->ParentTable;
}

/* Z_TableName() -- Returns the name of the table */
const char* Z_TableName(Z_Table_t* const a_Table)
{
	/* Check */
	if (!a_Table)
		return NULL;
		
	/* Easy */
	return a_Table->Key;
}

/* Z_PushNewEntry() -- Adds new entry to end of entries (prevents dup code) */
static Z_TableEntry_t* Z_PushNewEntry(Z_Table_t* const a_Table, const char* const a_Key)
{
	size_t Num;
	Z_TableEntry_t* Ref;
	
	/* Check */
	if (!a_Table || !a_Key)
		return NULL;
		
	/* Resize ptr list */
	Num = a_Table->NumEntries;
	Z_ResizeArray((void**)&a_Table->Entries, sizeof(*a_Table->Entries), Num, Num + 1);
	a_Table->Entries[Num] = Z_Malloc(sizeof(*a_Table->Entries[Num]), PU_STATIC, NULL);
	Ref = a_Table->Entries[Num];
	a_Table->NumEntries = Num + 1;
	
	/* Push to hashes */
	Z_HashAddEntry(a_Table->EntryHashes, Z_Hash(a_Key), Ref);
	
	/* Return reference */
	return Ref;
}

/* Z_FindTableEntry() -- Finds a table entry */
static Z_TableEntry_t* Z_FindTableEntry(Z_Table_t* const a_Table, const char* const a_Key, const bool_t a_Create, bool_t* const a_Created)
{
	uint32_t Hash;
	Z_TableEntry_t* Entry;
	
	/* Check */
	if (!a_Table || !a_Key)
		return NULL;
		
	/* Get hash from key */
	Hash = Z_Hash(a_Key);
	
	/* Find actual entry */
	Entry = Z_HashFindEntry(a_Table->EntryHashes, Hash, (void*)a_Key, false);
	
	// Found? return the entry
	if (Entry)
	{
		if (a_Created)
			*a_Created = false;
		return Entry;
	}
	// Create entry?
	if (!a_Create)
		return NULL;
		
	// It was not found to push it
	Entry = Z_PushNewEntry(a_Table, a_Key);
	
	if (a_Created)
		*a_Created = true;
		
	/* Return new entry */
	return Entry;
}

/* Z_FindSubTable() -- Finds subtable (and quite possibly creates it) */
Z_Table_t* Z_FindSubTable(Z_Table_t* const a_Table, const char* const a_Key, const bool_t a_Create)
{
	Z_TableEntry_t* Entry;
	bool_t Created;
	
	/* Check */
	if (!a_Table || !a_Key)
		return NULL;
		
	/* Find entry */
	Entry = Z_FindTableEntry(a_Table, a_Key, a_Create, &Created);
	
	// If we found the entry, it must be a table
	if (Entry && !Created)
		if (Entry->IsTable)
			return Entry->Data.TableLink;
		else
			return NULL;		// Entry so return NULL here
			
	// Otherwise if we are creating something
	if (a_Create && Created)
	{
		// Mess around with fresh entry
		Entry->IsTable = true;
		Entry->Data.TableLink = Z_TableCreate(a_Key);
		Entry->Data.TableLink->ParentTable = a_Table;
		
		// Return new table
		return Entry->Data.TableLink;
	}
	
	/* Failure */
	return NULL;
}

/* Z_TableGetValue() -- Get a value from a subkey */
const char* Z_TableGetValue(Z_Table_t* const a_Table, const char* const a_SubKey)
{
	Z_TableEntry_t* Entry;
	
	/* Check */
	if (!a_Table || !a_SubKey)
		return NULL;
		
	/* Find entry */
	Entry = Z_FindTableEntry(a_Table, a_SubKey, false, NULL);
	
	// If we found the entry it must be an entry
	if (Entry)
		if (!Entry->IsTable)
			// Get Value
			return Entry->Data.Index.Value;
			
	/* Was either not found or was a table */
	return NULL;
}

/* Z_TableSetValue() -- Set a value for a subkey */
bool_t Z_TableSetValue(Z_Table_t* const a_Table, const char* const a_SubKey, const char* const a_NewValue)
{
	Z_TableEntry_t* Entry;
	bool_t Created;
	
	/* Check */
	if (!a_Table || !a_SubKey)
		return false;
		
	/* Find entry */
	Entry = Z_FindTableEntry(a_Table, a_SubKey, true, &Created);
	
	/* Key not in table, or is in table and isn't a table */
	if (Created || (Entry && !Created && !Entry->IsTable))
	{
		// As long as the entry was created, does memory get allocated
		if (Created)
		{
			Entry->Data.Index.SubKey = Z_StrDup(a_SubKey, PU_STATIC, NULL);
			Entry->Data.Index.SubHash = Z_Hash(Entry->Data.Index.SubKey);
		}
		// if an old value exists, clear it
		if (Entry->Data.Index.Value)
			Z_Free(Entry->Data.Index.Value);
			
		Entry->Data.Index.Value = Z_StrDup(a_NewValue, PU_STATIC, NULL);
		
		// Success!
		return true;
	}
	
	/* Failure */
	return false;
}

/* Z_TablePrint() -- Prints table to console */
void Z_TablePrint(Z_Table_t* const a_Table, const char* const a_Prefix)
{
#define BUFSIZE 32
	size_t i;
	char Prefix[BUFSIZE];
	Z_TableEntry_t* Entry;
	
	/* Check */
	if (!a_Table)
		return;
		
	/* Slap on prefix */
	memset(Prefix, 0, sizeof(Prefix));
	if (a_Prefix)
		strncpy(Prefix, a_Prefix, BUFSIZE);
		
	/* Print Table key */
	CONL_PrintF("%s \"%s\" (%i)\n", Prefix, a_Table->Key, (int)a_Table->NumEntries);
	
	/* For every entry */
	// Slap a > on it
	strncat(Prefix, ">", BUFSIZE);
	
	// Rove
	for (i = 0; i < a_Table->NumEntries; i++)
	{
		// Reference
		Entry = a_Table->Entries[i];
		
		// If it is a table, go in it
		if (Entry->IsTable)
			Z_TablePrint(Entry->Data.TableLink, Prefix);
			
		// Otherwise it is an entry, print it's value
		else
			CONL_PrintF("%s @\"%s\" = \"%s\"\n", Prefix, Entry->Data.Index.SubKey, Entry->Data.Index.Value);
	}
#undef BUFSIZE
}

/* Z_TableMergeInto() -- Merges one table into another table */
bool_t Z_TableMergeInto(Z_Table_t* const a_Target, const Z_Table_t* const a_Source)
{
	/* Check */
	if (!a_Target || !a_Source)
		return false;
		
	return true;
}

/* Z_TableSuperCallback() -- Goes through a table and sends a found table to a callback */
bool_t Z_TableSuperCallback(Z_Table_t* const a_Table, bool_t (*a_Callback) (Z_Table_t* const a_Sub, void* const a_Data), void* const a_Data)
{
	size_t i;
	
	/* Check */
	if (!a_Table || !a_Callback)
		return false;
		
	/* Rove */
	for (i = 0; i < a_Table->NumEntries; i++)
		// Exists and is a table?
		if (a_Table->Entries[i] && a_Table->Entries[i]->IsTable)
			if (!a_Callback(a_Table->Entries[i]->Data.TableLink, a_Data))
				// Callback function returned false, so return false here
				return false;
				
	/* Success */
	return true;
}

/* Z_TableGetValueOrElse() -- Gets value from table or returns a_ElseOr */
const char* Z_TableGetValueOrElse(Z_Table_t* const a_Table, const char* const a_SubKey, const char* a_ElseOr)
{
	const char* Value;
	
	/* Try getting the value */
	// It has no value
	if (!(Value = Z_TableGetValue(a_Table, a_SubKey)))
		return a_ElseOr;
	
	// There is a value
	else
		return Value;
}

