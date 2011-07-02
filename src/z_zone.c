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
// Copyright (C) 2011 GhostlyDeath (ghostlydeath@gmail.com).
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

/****************
*** CONSTANTS ***
****************/

#define PARTSHIFT		5						// >>/<< for partition sizes
#define MINZONESIZE		(512 << 10)				// Minimum zone size: 512KiB
#define INITPARTCOUNT	256						// Initial partition count
#define RIGHTSIZE		8						// Minimum size to allocate on right
#define FORCEDMEMORYSIZE	(4U << 20)			// Minimum allowed auto memory

/*****************
*** STRUCTURES ***
*****************/

/* Z_MemPartition_t -- A memory partition */
typedef struct Z_MemPartition_s
{
	/* Partition Data */
	struct
	{
		size_t Start;							// Partition start
		size_t End;								// Partition end
		size_t Size;							// Partition size
		boolean Used;							// Partition used
	} Part;										// Partition Info
	
	/* Block Data */
	struct
	{
		void* Ptr;								// Pointer to block
		void** Ref;								// Reference
		Z_MemoryTag_t Tag;						// Tag
		size_t Size;							// Allocation size
	} Block;
} Z_MemPartition_t;

/* Z_MemZone_t -- A memory zone */
typedef struct Z_MemZone_s
{
	/* Partitions */
	Z_MemPartition_t* PartitionList;			// List of partitions
	size_t NumPartitions;						// Number of partitions
	size_t MaxPartitions;						// Max partitions for list
	
	/* Data */
	uint8_t* DataChunk;							// Where all the data is stored
	size_t AllocSize;							// Allocated size
	size_t TotalMemory;							// Allocated memory (in shifts)
	size_t FreeMemory;							// Free memory (in shifts)
	
	/* Speed */
	size_t FirstFree;							// First free partition
	size_t LastFree;							// Last free partition
} Z_MemZone_t;

/*************
*** LOCALS ***
*************/

static Z_MemZone_t* l_MainZone = NULL;			// Memory zones

/************************
*** PRIVATE FUNCTIONS ***
************************/

/* ZP_NewZone() -- Creates a new memory zone */
boolean ZP_NewZone(size_t* const SizePtr, Z_MemZone_t** const ZoneRef)
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

/* ZP_PartitionSplit() -- Splits a partition into two new pieces */
void ZP_PartitionSplit(Z_MemPartition_t* const ToSplit, Z_MemPartition_t** const ResLeftPtr, Z_MemPartition_t** const ResRightPtr, const size_t Pos, const boolean AtEnd)
{
	size_t pI, SplitPos;
	Z_MemPartition_t Copy;
	Z_MemPartition_t* NewList;
	
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
	memmove(
			&l_MainZone->PartitionList[pI + 1],
			&l_MainZone->PartitionList[pI],
			sizeof(Z_MemPartition_t) * (l_MainZone->NumPartitions - pI)
		);
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
}

/* ZP_PointerForPartition() -- Returns the pointer to the start of the partition */
void* ZP_PointerForPartition(Z_MemPartition_t* const Part)
{
	/* Check */
	if (!l_MainZone || !Part)
		return NULL;
	
	/* Return base zone plus the shifted size of partition */
	return (void*)(((uintptr_t)l_MainZone->DataChunk) + (Part->Part.Start << PARTSHIFT));
}

/* ZP_FindPointerForPartition() -- Finds a pointer that was found in a partition (if any) */
boolean ZP_FindPointerForPartition(void* const Ptr, Z_MemPartition_t** const PartPtr)
{
	size_t i;
	Z_MemPartition_t* Part;
	
	/* Check */
	if (!Ptr || !l_MainZone || !PartPtr)
		return false;
	
	/* Did we grab a zone? */
	// Search the zone partition by partition for this pointer
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

/* ZP_FreePartitionInZone() -- Frees selected partition in selected zone */
// Returns (negative) result of partitions merged away (NewNP = OldNumPartitions - RetVal)
size_t ZP_FreePartitionInZone(Z_MemPartition_t* const Part)
{
	size_t i;
	size_t RetVal;
	Z_MemPartition_t* Mergee;
	Z_MemPartition_t LP, RP;
	
	/* Check */
	if (!!Part)
		return 0;
	
	/* Partition free? */
	if (!Part->Part.Used)
	{
		I_Error("ZP_FreePartitionInZone: Freeing a partition already free?");
		return 0;
	}
	
	/* Lazy fragment freeing */
	// Is there a reference set?
	if (Part->Block.Ref)
		*Part->Block.Ref = NULL;
	
	// Clear out data with a simple sweep
	memset(&Part->Block, 0, sizeof(Part->Block));
	
	// No longer used anymore
	Part->Part.Used = false;
	
	/* Get local partition id */
	i = Part - l_MainZone->PartitionList;
	
	/* As long as there is partition before us and it is free, go to that */
	Mergee = Part;
	RetVal = 0;
	
	while (i > 0 && !l_MainZone->PartitionList[i - 1].Part.Used)
	{
		Mergee--;	// Pointers are fun
		i--;
	}
	
	/* As long as a partition is to the right of us, merge into it */
	while (i + 1 < l_MainZone->NumPartitions && !l_MainZone->PartitionList[i + 1].Part.Used)
	{
		// Get original left and right, copy them
		LP = l_MainZone->PartitionList[i];	// also Mergee
		RP = l_MainZone->PartitionList[i + 1];
		
		// Slap back zones
		memmove(
				&l_MainZone->PartitionList[i],
				&l_MainZone->PartitionList[i + 1],
				sizeof(Z_MemPartition_t) * (l_MainZone->NumPartitions - i)
			);
		
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
	
	/* Return number of merged partitions */
	return RetVal;
}

/****************
*** FUNCTIONS ***
****************/

/* Z_Init() -- Initializes the memory manager */
void Z_Init(void)
{
	size_t FreeMem = 0, TotalMem;
	size_t LoopSize, i;
	boolean Ok = false;
	
	/* Create new zone in a loop */
	// Check free memory
	FreeMem = I_GetFreeMem(&TotalMem);
	
	// Less than 20MB?
	if (FreeMem < FORCEDMEMORYSIZE)
	{
		if (devparm)
			CONS_Printf("Z_Init: Reported value below %iMiB, capping.\n", FORCEDMEMORYSIZE >> 20);
		
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

/* Z_MallocWrappee() -- Allocate memory */
void* Z_MallocWrappee(const size_t Size, const Z_MemoryTag_t Tag, void** const Ref _ZMGD_WRAPPEE)
{
	size_t i, ShiftSize, iBase, iEnd;
	uint32_t DidMask, FullMask;
	Z_MemPartition_t* ResLeft, *ResRight, *New, *Free;
	void* RetVal;
	boolean AtEnd;
	
	/* Clear some */
	RetVal = NULL;
	ResLeft = ResRight = New = Free = NULL;
	
	/* Size for shift */
	ShiftSize = (Size >> PARTSHIFT) + 1;
	
	// Do we allocate at the end?
	if (ShiftSize < RIGHTSIZE)
		AtEnd = true;
	else
		AtEnd = false;
	
	// Figure out full mask
	FullMask = ~0;
	
	// Set the mask for this zone
		// Do it here since if we fail to lock a zone we don't use it
		// which means a zone that was locked could have the space we
		// need to allocate.
	DidMask |= 1 << i;
	
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
		iEnd = (size_t)-1;
	}	
	
	// Search every partition
	for (i = iBase; i != iEnd; (AtEnd ? i-- : i++))
	{
		// Partition used?
		if (l_MainZone->PartitionList[i].Part.Used)
			continue;

		// Paritition too small?
		if (l_MainZone->PartitionList[i].Part.Size < ShiftSize)
			continue;
		
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
		
		// Set block info
		New->Block.Ptr = ZP_PointerForPartition(New);
		New->Block.Ref = Ref;
		New->Block.Size = Size;
		
		// Block Tag (if it is invalid, just make it static to be sure)
		if ((size_t)Tag >= NUMZTAGS)
			New->Block.Tag = PU_STATIC;
		else
			New->Block.Tag = Tag;
		
		// Set return value
		RetVal = New->Block.Ptr;
		
		// Complementary memset
		memset(RetVal, 0, Size);
		
		// Return pointer
		return RetVal;
	}
	
	/* Failure? */
	I_Error("Z_MallocReal: Failed to allocate %zu bytes.\n", Size);
	return NULL;
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
		return;	// not found
	
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
		return ((Z_MemoryTag_t)-1);
	
	/* Find zone and partition relating to pointer */
	// Clear
	Part = NULL;
	
	// Now call
	if (!ZP_FindPointerForPartition(Ptr, &Part))
		return ((Z_MemoryTag_t)-1);	// not found
	
	/* Remember old tag */
	OldTag = Part->Block.Tag;
	
	// Validate new tag (if it is invalid, just make it static)
	if ((size_t)NewTag >= NUMZTAGS)
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
		return ((Z_MemoryTag_t)-1);
	
	/* Find zone and partition relating to pointer */
	// Clear
	Part = NULL;
	
	// Now call
	if (!ZP_FindPointerForPartition(Ptr, &Part))
		return ((Z_MemoryTag_t)-1);	// not found
	
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
		return NULL;	// not found
	
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
		if (j - Lost > j)	// Negative
			j = 0;
		else				// Backtrace to said partition (leftmost free)
			j -= Lost;
	}
	
	/* Return freed stuff */
	return FreeCount;
}

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

char *Z_StrDup(const char* const String, const Z_MemoryTag_t Tag, void** Ref)
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
	Z_HashKey_t* KeyList[256];								// Key list
	size_t KeySize[256];							// Keys in list
	boolean (*CompareFunc)(void* const a_A, void* const a_B);
};

/*** FUNCTIONS ***/
/* Z_HashCreateTable() -- Creates a new hashing table */
Z_HashTable_t* Z_HashCreateTable(boolean (*a_CompareFunc)(void* const a_A, void* const a_B))
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
boolean Z_HashAddEntry(Z_HashTable_t* const a_HashTable, const uint32_t a_Key, void* const a_Data)
{
	uint32_t Nub;
	size_t i;
	
	/* Check */
	if (!a_HashTable || !a_Data)
		return false;
	
	/* Get nub of key */
	Nub = a_Key & 0xFF;
	
	/* Resize list */
	Z_ResizeArray(&a_HashTable->KeyList[Nub], sizeof(Z_HashKey_t), a_HashTable->KeySize[Nub], a_HashTable->KeySize[Nub] + 1);
	
	// Slap at end
	i = a_HashTable->KeySize[Nub]++;
	a_HashTable->KeyList[Nub][i].Key = a_Key;
	a_HashTable->KeyList[Nub][i].Data = a_Data;
	
	/* Success! */
	return true;
}

/* Z_HashFindEntry() -- Finds an entry in the hash table */
void* Z_HashFindEntry(Z_HashTable_t* const a_HashTable, const uint32_t a_Key, void* const a_DataSim, const boolean a_BackRun)
{
	uint32_t Nub;
	size_t i;
	
	/* Check */
	if (!a_HashTable)
		return NULL;
	
	/* Get nub */
	Nub = a_Key & 0xFF;
	
	/* Seek based on direction */
	for (i = (a_BackRun ? a_HashTable->KeySize[Nub] - 1 : 0);
		i != (a_BackRun ? (size_t)-1 : a_HashTable->KeySize[Nub]);
		i = (a_BackRun ? i - 1 : i + 1))
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

