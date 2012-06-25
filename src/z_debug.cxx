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
// DESCRIPTION: New New Memory Manager Written by GhostlyDeath
//              Stolen from my proprietary game and stripped

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

#define PARTSHIFT		5		// >>/<< for partition sizes
#define MINZONESIZE		(512 << 10)	// Minimum zone size: 512KiB
#define INITPARTCOUNT	256		// Initial partition count
#define RIGHTSIZE		8		// Minimum size to allocate on right
#define FORCEDMEMORYSIZE	(4U << 20)	// Minimum allowed auto memory
#define ZPARTEXTRASIZE 16		// Extra size to waste on allocation

/*****************
*** STRUCTURES ***
*****************/

/* Z_DEBUG_MemPartition_t -- A memory partition */
typedef struct Z_DEBUG_MemPartition_s
{
	/* Partition Data */
	struct
	{
		size_t Start;			// Partition start
		size_t End;				// Partition end
		size_t Size;			// Partition size
		bool Used;			// Partition used
		uint32_t* SelfRef;		// Self Reference Pointer
		bool AtEnd;			// Allocated at the end?
	} Part;						// Partition Info
	
	/* Block Data */
	struct
	{
		void* Ptr;				// Pointer to block
		void** Ref;				// Reference
		Z_MemoryTag_t Tag;		// Tag
		size_t Size;			// Allocation size
#if defined(_DEBUG)
		const char* File;		// File
		int32_t Line;			// Line
		uint32_t Time;			// Allocation Time
#endif
	} Block;
} Z_DEBUG_MemPartition_t;

/* Z_DEBUG_MemZone_t -- A memory zone */
typedef struct Z_DEBUG_MemZone_s
{
	/* Partitions */
	Z_DEBUG_MemPartition_t* PartitionList;	// List of partitions
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
} Z_DEBUG_MemZone_t;

/*************
*** LOCALS ***
*************/

static Z_DEBUG_MemZone_t* l_MainZone = NULL;	// Memory zones

/************************
*** PRIVATE FUNCTIONS ***
************************/

/* ZP_DEBUG_NewZone() -- Creates a new memory zone */
bool ZP_DEBUG_NewZone(size_t* const SizePtr, Z_DEBUG_MemZone_t** const ZoneRef)
{
	size_t ShiftSize;
	Z_DEBUG_MemZone_t* NewZone;
	Z_DEBUG_MemPartition_t* Part;
	
	/* Check */
	if (!SizePtr)
		return false;
		
	/* Obtain the shift size */
	ShiftSize = *SizePtr >> PARTSHIFT;
	
	/* Create an empty zone */
	NewZone = (Z_DEBUG_MemZone_t*)I_SysAlloc(sizeof(*NewZone));
	
	// Failed?
	if (!NewZone)
		return false;
		
	// Clear it out
	memset(NewZone, 0, sizeof(*NewZone));
	
	/* Create huge chunk zone */
	NewZone->DataChunk = (uint8_t*)I_SysAlloc(ShiftSize << PARTSHIFT);
	
	// Failed?
	if (!NewZone->DataChunk)
	{
		I_SysFree(NewZone);
		return false;
	}
	
	// Clear it out
	memset(NewZone->DataChunk, 0, ShiftSize << PARTSHIFT);
	
	/* Create initial partitions */
	NewZone->PartitionList = (Z_DEBUG_MemPartition_t*)I_SysAlloc(sizeof(*NewZone->PartitionList) * INITPARTCOUNT);
	
	// Failed?
	if (!NewZone->PartitionList)
	{
		I_SysFree(NewZone->DataChunk);
		I_SysFree(NewZone);
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
	Part->Part.SelfRef = (uint32_t*)NewZone->DataChunk;
	
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

void* ZP_DEBUG_PointerForPartition(Z_DEBUG_MemPartition_t* const Part, void** const a_BasePtrPtr);

/* ZP_DEBUG_PartitionSplit() -- Splits a partition into two new pieces */
void ZP_DEBUG_PartitionSplit(Z_DEBUG_MemPartition_t* const ToSplit, Z_DEBUG_MemPartition_t** const ResLeftPtr, Z_DEBUG_MemPartition_t** const ResRightPtr, const size_t Pos,
                       const bool AtEnd)
{
	size_t pI, SplitPos, i;
	Z_DEBUG_MemPartition_t Copy;
	Z_DEBUG_MemPartition_t* NewList;
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
		NewList = (Z_DEBUG_MemPartition_t*)I_SysAlloc(sizeof(Z_DEBUG_MemPartition_t) * (l_MainZone->MaxPartitions + INITPARTCOUNT));
		
		// Only if we successfully created a new list
		if (NewList)
		{
			// Clear the list
			memset(NewList, 0, sizeof(Z_DEBUG_MemPartition_t) * (l_MainZone->MaxPartitions + INITPARTCOUNT));
			
			// Copy all the old stuff over
			memmove(NewList, l_MainZone->PartitionList, sizeof(Z_DEBUG_MemPartition_t) * l_MainZone->MaxPartitions);
			
			// Free old stuff
			I_SysFree(l_MainZone->PartitionList);
			
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
				I_Error("ZP_DEBUG_PartitionSplit: Needed more partitions but ran out of memory.");
		}
	}
	// Move everything over
	memmove(&l_MainZone->PartitionList[pI + 1], &l_MainZone->PartitionList[pI], sizeof(Z_DEBUG_MemPartition_t) * (l_MainZone->NumPartitions - pI));
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
		ZP_DEBUG_PointerForPartition(&l_MainZone->PartitionList[i], &Base);
		//fprintf(stderr, "ss %i -", *((uint32_t*)Base));
		l_MainZone->PartitionList[i].Part.SelfRef = (uint32_t*)Base;
		*((uint32_t*)Base) = i;
		//fprintf(stderr, " %i\n", *((uint32_t*)Base));
	}
}

/* ZP_DEBUG_PointerForPartition() -- Returns the pointer to the start of the partition */
// GhostlyDeath <December 14, 2011> -- Account for wasted size
void* ZP_DEBUG_PointerForPartition(Z_DEBUG_MemPartition_t* const Part, void** const a_BasePtrPtr)
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

/* ZP_DEBUG_FindPointerForPartition() -- Finds a pointer that was found in a partition (if any) */
// GhostlyDeath <December 14, 2011> -- This function is slow!!!
bool ZP_DEBUG_FindPointerForPartition(void* const Ptr, Z_DEBUG_MemPartition_t** const PartPtr)
{
	size_t i;
	uintptr_t Back;
	uint32_t* BackP;
	Z_DEBUG_MemPartition_t* Part;
	
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

/* ZP_DEBUG_MergePartitions() -- Merges partitions */
static size_t ZP_DEBUG_MergePartitions(Z_DEBUG_MemPartition_t* const Part)
{
	size_t i;
	size_t RetVal;
	Z_DEBUG_MemPartition_t* Mergee;
	Z_DEBUG_MemPartition_t LP, RP;
	
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
		memmove(&l_MainZone->PartitionList[i], &l_MainZone->PartitionList[i + 1], sizeof(Z_DEBUG_MemPartition_t) * (l_MainZone->NumPartitions - i));
		
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
		ZP_DEBUG_PointerForPartition(Mergee, (void**)&Mergee->Part.SelfRef);
		*Mergee->Part.SelfRef = i;
	}

	return RetVal;
}

/* ZP_DEBUG_FreePartitionInZone() -- Frees selected partition in selected zone */
// Returns (negative) result of partitions merged away (NewNP = OldNumPartitions - RetVal)
size_t ZP_DEBUG_FreePartitionInZone(Z_DEBUG_MemPartition_t* const Part)
{
	size_t i;
	size_t RetVal;
	Z_DEBUG_MemPartition_t* Mergee;
	Z_DEBUG_MemPartition_t LP, RP;
	
	/* Check */
	if (!Part)
		return 0;
		
	/* Partition free? */
	if (!Part->Part.Used)
	{
		I_Error("ZP_DEBUG_FreePartitionInZone: Freeing a partition already free?");
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
		RetVal = ZP_DEBUG_MergePartitions(Part);
		
		// Clear at end
		Part->Part.AtEnd = false;
	}
	
	/* Return number of merged partitions */
	return RetVal;
}

#if defined(_DEBUG)
/* Z_DEBUG_DupFileLine() -- Duplicate line and file into another block */
void Z_DEBUG_DupFileLine(void* const a_Dest, void* const a_Src)
{
	Z_DEBUG_MemPartition_t* PartD;
	Z_DEBUG_MemPartition_t* PartS;
	
	/* Check */
	if (!a_Dest || !a_Src)
		return;
		
	/* Find zone and partition relating to pointer */
	// Clear
	PartD = PartS = NULL;
	
	// Now call
	if (!ZP_DEBUG_FindPointerForPartition(a_Dest, &PartD))
		return;
	if (!ZP_DEBUG_FindPointerForPartition(a_Src, &PartS))
		return;
	
	/* Copy File:line over */
	PartD->Block.File = PartS->Block.File;
	PartD->Block.Line = PartS->Block.Line;
}
#endif

/****************
*** FUNCTIONS ***
****************/

/* Z_DEBUG_Init() -- Initializes the memory manager */
void Z_DEBUG_Init(void)
{
	uint64_t FreeMem = 0, TotalMem;
	size_t LoopSize, i;
	bool Ok = false;
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
		CONL_PrintF("Z_DEBUG_Init: ReMooD was compiled with the wrong byte order!\n");
	
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
		CONL_PrintF("Z_DEBUG_Init: There is %iMiB of memory available (Total: %iMiB).\n", (int)(FreeMem >> 20), (int)(TotalMem >> 20));
	
	// Less than 20MB?
	if (FreeMem < FORCEDMEMORYSIZE)
	{
		if (devparm)
			CONL_PrintF("Z_DEBUG_Init: Reported value below %iMiB, capping.\n", FORCEDMEMORYSIZE >> 20);
			
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
	while (LoopSize > MINZONESIZE && !(Ok = ZP_DEBUG_NewZone(&LoopSize, NULL)))
		LoopSize -= LoopSize >> 2;	// Cut down by 1/4th
}

/* Z_DEBUG_MallocExWrappee() -- Allocate memory */
void* Z_DEBUG_MallocExWrappee(const size_t Size, const Z_MemoryTag_t Tag, void** const Ref, const uint32_t a_Flags _ZMGD_WRAPPEE)
{
	size_t i, ShiftSize, iBase, iEnd, j, k;
	Z_DEBUG_MemPartition_t* ResLeft, *ResRight, *New, *Free;
	void* RetVal;
	bool AtEnd;
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
					ZP_DEBUG_MergePartitions(&l_MainZone->PartitionList[i]);
			
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
			ZP_DEBUG_PartitionSplit(&l_MainZone->PartitionList[i], &ResLeft, &ResRight, ShiftSize, AtEnd);
			
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
		New->Block.Ptr = ZP_DEBUG_PointerForPartition(New, &BasePtr);
		New->Block.Ref = Ref;
		New->Block.Size = Size;

#ifdef _DEBUG
		New->Block.File = File;
		New->Block.Line = Line;
		New->Block.Time = I_GetTimeMS();
#endif
		
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
		I_Error("Z_DEBUG_MallocReal: Failed to allocate %zu bytes.\n", Size);
		return NULL;
	}
	// Run another loop
	else
	{
		// Clear cache blocks and unused lumps
		k = l_MainZone->FreeMemory;
		i = Z_FreeTags(PU_PURGELEVEL, NUMZTAGS);
		j = 0;
		
		if (devparm)
			CONL_PrintF("Z_DEBUG_MallocReal: Nearly out of memory, freed %u lumps, %u blocks (Freed %u bytes).\n", j, i, l_MainZone->FreeMemory - k);
			
		// Now try re-allocation
		DeferLevel = 1;
		RetVal = Z_Malloc(Size, Tag, Ref);
		DeferLevel = 0;
		return RetVal;
	}
}

/* Z_DEBUG_FreeWrappee() -- Free memory */
void Z_DEBUG_FreeWrappee(void* const Ptr _ZMGD_WRAPPEE)
{
	Z_DEBUG_MemPartition_t* Part;
	
	/* Check */
	if (!Ptr)
		return;
		
	/* Find zone and partition relating to pointer */
	// Clear
	Part = NULL;
	
	// Now call
	if (!ZP_DEBUG_FindPointerForPartition(Ptr, &Part))
		return;					// not found
		
	/* Send zone along with partition to my free function */
	ZP_DEBUG_FreePartitionInZone(Part);
}

/* Z_DEBUG_ChangeTagWrappee() -- Change memory tag returning the old one */
Z_MemoryTag_t Z_DEBUG_ChangeTagWrappee(void* const Ptr, const Z_MemoryTag_t NewTag _ZMGD_WRAPPEE)
{
	Z_MemoryTag_t OldTag;
	Z_DEBUG_MemPartition_t* Part;
	
	/* Check */
	if (!Ptr)
		return ((Z_MemoryTag_t) - 1);
		
	/* Find zone and partition relating to pointer */
	// Clear
	Part = NULL;
	
	// Now call
	if (!ZP_DEBUG_FindPointerForPartition(Ptr, &Part))
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

/* Z_DEBUG_GetTagFromPtrWrappee() -- Return the memory tag associated with a pointer */
Z_MemoryTag_t Z_DEBUG_GetTagFromPtrWrappee(void* const Ptr _ZMGD_WRAPPEE)
{
	Z_MemoryTag_t RetVal;
	Z_DEBUG_MemPartition_t* Part;
	
	/* Check */
	if (!Ptr)
		return ((Z_MemoryTag_t) - 1);
		
	/* Find zone and partition relating to pointer */
	// Clear
	Part = NULL;
	
	// Now call
	if (!ZP_DEBUG_FindPointerForPartition(Ptr, &Part))
		return ((Z_MemoryTag_t) - 1);	// not found
		
	/* Remember the tag */
	RetVal = Part->Block.Tag;
	
	// Return the tag
	return RetVal;
}

/* Z_DEBUG_GetRefFromPtrWrappee() -- Returns the reference of a block */
void** Z_DEBUG_GetRefFromPtrWrappee(void* const Ptr _ZMGD_WRAPPEE)
{
	void** RetVal;
	Z_DEBUG_MemPartition_t* Part;
	
	/* Check */
	if (!Ptr)
		return NULL;
		
	/* Find zone and partition relating to pointer */
	// Clear
	Part = NULL;
	
	// Now call
	if (!ZP_DEBUG_FindPointerForPartition(Ptr, &Part))
		return NULL;			// not found
		
	/* Remember the reference */
	RetVal = Part->Block.Ref;
	
	// Return the tag
	return RetVal;
}

/* Z_DEBUG_FreeTagsWrappee() -- Free all tags between low inclusive and high exclusive */
size_t Z_DEBUG_FreeTagsWrappee(const Z_MemoryTag_t LowTag, const Z_MemoryTag_t HighTag _ZMGD_WRAPPEE)
{
	size_t i, j, FreeCount, Lost;
	Z_DEBUG_MemPartition_t* Part;
	
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
		Lost = ZP_DEBUG_FreePartitionInZone(Part);
		
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

/* ZS_DEBUG_MemInfo() -- "meminfo" -- Print memory information */
static CONL_ExitCode_t ZS_DEBUG_MemInfo(const uint32_t a_ArgC, const char** const a_ArgV)
{
	size_t i;
	Z_DEBUG_MemPartition_t* Cur;
	
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
	PBytes = l_MainZone->MaxPartitions * sizeof(Z_DEBUG_MemPartition_t);
	
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

/* ZS_DEBUG_MemCacheFree() -- "memcachefree" -- Free all cached memory */
static CONL_ExitCode_t ZS_DEBUG_MemCacheFree(const uint32_t a_ArgC, const char** const a_ArgV)
{
	uint32_t FreeCount;
	
	/* Free all cache tags */
	FreeCount = Z_FreeTags(PU_PURGELEVEL, NUMZTAGS);
	
	/* Print Info */
	CONL_PrintF("Freed %u blocks.\n", FreeCount);
	
	/* Success */
	return CLE_SUCCESS;
}

/* ZS_DEBUG_MemFrag() -- "memfrag" -- Show memory fragmentation */
static CONL_ExitCode_t ZS_DEBUG_MemFrag(const uint32_t a_ArgC, const char** const a_ArgV)
{
#define MAXCHARS (30 * 10)
	uint32_t BytesPerChar;
	uint32_t CurByte, Diff, Left;
	size_t i, j, CurCol, c;
	Z_DEBUG_MemPartition_t* Cur;
	bool LastUsed;
	
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

#if defined(_DEBUG)
/* ZS_DEBUG_MemTrash() -- "memtrash" -- Print hardcore information */
static CONL_ExitCode_t ZS_DEBUG_MemTrash(const uint32_t a_ArgC, const char** const a_ArgV)
{
	size_t i;
	FILE* f;
	Z_DEBUG_MemPartition_t* Cur;
	
	/* Open */
	f = fopen("Trash", "wt");
	
	/* Go through each partition */
	for (i = 0; i < l_MainZone->NumPartitions; i++)
	{
		// Get current partition
		Cur = &l_MainZone->PartitionList[i];
		
		// Used?
		if (Cur->Part.Used)
		{
			fprintf(f, "Used [%16u+%8u]; tg %2u sz %8u %12.12s:%4i (t+%8u))\n",
					(uint32_t)Cur->Part.Start,
					(uint32_t)Cur->Part.Size,
					(uint32_t)Cur->Block.Tag,
					(uint32_t)Cur->Block.Size,
					Cur->Block.File,
					(int32_t)Cur->Block.Line,
					(uint32_t)Cur->Block.Time
				);
		}
		
		// Free?
		else
		{
			fprintf(f, "Free [%16u+%8u]\n",
					(uint32_t)Cur->Part.Start,
					(uint32_t)Cur->Part.Size
				);
		}
	}
	
	fclose(f);
	
	/* Success */
	return CLE_SUCCESS;
}
#endif

/* Z_DEBUG_RegisterCommands() -- Register commands to the console */
void Z_DEBUG_RegisterCommands(void)
{
	CONL_AddCommand("meminfo", ZS_DEBUG_MemInfo);
	CONL_AddCommand("memcachefree", ZS_DEBUG_MemCacheFree);
	CONL_AddCommand("memfrag", ZS_DEBUG_MemFrag);

#if defined(_DEBUG)
	CONL_AddCommand("memtrash", ZS_DEBUG_MemTrash);
#endif
}

