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
// DESCRIPTION:
//      New Memory Manager Written by GhostlyDeath

/***************
*** INCLUDES ***
***************/

#include "doomdef.h"
#include "z_zone.h"
#include "i_system.h"
#include "command.h"
#include "m_argv.h"
#include "i_video.h"
#include "doomstat.h"
#include "dstrings.h"

/****************
*** CONSTANTS ***
****************/

#define FORCEDMEMORYSIZE (4U << 20)
#define SEGMENTSIZE 64								// Forced alignment of portions
#define MAXZONES 10									// Maximum zones before we crash

/*****************
*** STRUCTURES ***
*****************/

/* Z_MemoryBlock_t -- A block of memory */
typedef struct Z_MemoryBlock_s
{
	/* Info */
	void* Ptr;												// Pointer to data
	size_t Size;											// Size of data
	size_t OrigSize;										// Original data size
	void** Ref;												// Pointer reference
	Z_MemoryTag_t Tag;										// Memory Tag (static?)
	
	/* Partition */
	size_t DiffPos;											// Position from start
	
	/* Debug */
#if defined(_DEBUG)
	struct
	{
		char File[32];										// File malloced/freed in
		UInt32 Line;										// Line of file
		UInt32 Count;										// Times malloced/freed
	} DebugInfo[2];
	UInt32 Order;											// Order number
#endif
} Z_MemoryBlock_t;

/* Z_MemoryPartition_t -- A partition of memory */
typedef struct Z_MemoryPartition_s
{
	size_t Size;											// Size of partition
	
	struct
	{
		void* Ptr;											// Pointer to side
		size_t Offset;										// Size of partition
	} Sides[2];
} Z_MemoryPartition_t;

/* Z_MemoryZoneArea_t -- A portion of memory */
typedef struct Z_MemoryZoneArea_s
{
	/* Shared memory */
	void* Ptr;												// Pointer to area
	size_t Size;											// Size of area
	size_t UsedSize;										// Memory Used
	
	/* Blocks */
	Z_MemoryBlock_t* BlockList;								// List of blocks
	size_t BlockCount;										// Number of blocks
	size_t NumCacheBlocks;									// # of cache blocks
	size_t FirstFreeBlock;									// First available block
	size_t LastUsedBlock;									// Last used block
	
	/* Partitions */
	Z_MemoryPartition_t* PartitionList;						// List of partitions
	size_t PartitionCount;									// Number of partitions
	size_t UsedPartitions;									// Active partition count
		
	/* Links */
	struct Z_MemoryZoneArea_s* Prev;						// Previous area
	struct Z_MemoryZoneArea_s* Next;						// Next area
} Z_MemoryZoneArea_t;

/*************
*** LOCALS ***
*************/

Z_MemoryZoneArea_t* l_HeadZone = NULL;
size_t l_ZoneCount = 0;

/****************
*** FUNCTIONS ***
****************/

char *Z_Strdup(const char* String, const Z_MemoryTag_t Tag, void** Ref)
{
	return strcpy(Z_Malloc(strlen(String) + 1, Tag, Ref), String);
}

wchar_t* Z_StrdupW(const wchar_t* WString, const Z_MemoryTag_t Tag, void** Ref)
{
	return UNICODE_Copy(Z_Malloc((UNICODE_StringLength(WString) * sizeof(wchar_t)) + 2, Tag, Ref), WString);
}

/* Z_Sort() -- Sort data using selection sort */
void Z_Sort(void* const List, const size_t ElemSize, const size_t NumElem, const boolean Reverse, int (*CompFunc)(const void* const A, const void* const B))
{
#define SORTELEMOF(base,sz,i) (((UInt8*)(base)) + ((sz) * (i)))
	size_t i , j, l;
	void* Temp;
	
	/* Check */
	if (!List || !ElemSize || !NumElem || !CompFunc)
		return;
		
	/* Allocate temporary store */
	Temp = malloc(ElemSize);
	
	if (!Temp)	// uh-oh
		return;
	
	/* Now do selection sorting stuffer */
	for (i = 0; i < NumElem; i++)
	{
		// Look for smallest/largest in group
		if (!Reverse)
		{
			// Small
			for (l = i, j = i + 1; j < NumElem; j++)
				if (CompFunc(SORTELEMOF(List, ElemSize, j), SORTELEMOF(List, ElemSize, l)) < 0)
					l = j;
		}
		else
		{
			// Large
			for (l = i, j = i + 1; j < NumElem; j++)
				if (CompFunc(SORTELEMOF(List, ElemSize, j), SORTELEMOF(List, ElemSize, l)) > 0)
					l = j;
		}
		
		// Swap
		if (l != i)
		{
			memmove(Temp, SORTELEMOF(List, ElemSize, i), ElemSize);
			memmove(SORTELEMOF(List, ElemSize, i), SORTELEMOF(List, ElemSize, l), ElemSize);
			memmove(SORTELEMOF(List, ElemSize, l), Temp, ElemSize);
		}
	}
	
	/* Free temporary space */
	if (Temp)
		free(Temp);
#undef SORTELEMOF
}

/* Z_CreateNewZone() -- Adds a new zone */
Z_MemoryZoneArea_t* Z_CreateNewZone(const UInt32 Size)
{
	int i;
	Z_MemoryZoneArea_t* TempZone;
	Z_MemoryZoneArea_t* Rover;
	
	/* Will be more than max? */
	if (l_ZoneCount >= MAXZONES)
		return NULL;
	
	/* Check */
	if (Size < (3 << 20))
		return NULL;
	
	/* Allocate structure */
	// Alloc
	TempZone = malloc(sizeof(*TempZone));
	
	// Check
	if (!TempZone)
	{
		CONS_Printf("Z_CreateNewZone: Failed to allocate structure.\n");
		return NULL;
	}
	
	// Set
	memset(TempZone, 0, sizeof(*TempZone));
	
	/* Allocate Zone */
	// Alloc
	TempZone->Size = Size;
	TempZone->Ptr = malloc(TempZone->Size);
	
	// Check
	if (!TempZone->Ptr)
	{
		CONS_Printf("Z_CreateNewZone: Failed to allocate chunk.\n");
		free(TempZone);
		return NULL;
	}
	
	// Set
	memset(TempZone->Ptr, 0, TempZone->Size);
	
	/* Allocate Blocks */
	// Alloc
	TempZone->BlockCount = (Size / sizeof(Z_MemoryBlock_t)) / 10;
	TempZone->NumCacheBlocks = TempZone->FirstFreeBlock = TempZone->LastUsedBlock = 0;
	TempZone->BlockList = malloc(sizeof(Z_MemoryBlock_t) * TempZone->BlockCount);
	
	// Check
	if (!TempZone->BlockList)
	{
		CONS_Printf("Z_CreateNewZone: Failed to allocate blocks.\n");
		free(TempZone->Ptr);
		free(TempZone);
		return NULL;
	}
	
	// Set
	memset(TempZone->BlockList, 0, sizeof(Z_MemoryBlock_t) * TempZone->BlockCount);
	
	CONS_Printf("Z_CreateNewZone: Zone has %u blocks.\n", TempZone->BlockCount);
	
	/* Allocate Partitions */
	// Alloc
	TempZone->PartitionCount = 64;
	TempZone->UsedPartitions = 1;
	TempZone->PartitionList = malloc(sizeof(Z_MemoryPartition_t) * TempZone->PartitionCount);
	
	// Check
	if (!TempZone->PartitionList)
	{
		CONS_Printf("Z_CreateNewZone: Failed to allocate partitions.\n");
		free(TempZone->BlockList);
		free(TempZone->Ptr);
		free(TempZone);
		return NULL;
	}
	
	// Set
	memset(TempZone->PartitionList, 0, sizeof(Z_MemoryPartition_t) * TempZone->PartitionCount);
	TempZone->PartitionList[0].Sides[0].Ptr = TempZone->Ptr;
	TempZone->PartitionList[0].Sides[0].Offset = 0;
	TempZone->PartitionList[0].Sides[1].Ptr = ((UInt8*)TempZone->Ptr) + TempZone->Size;
	TempZone->PartitionList[0].Sides[1].Offset = TempZone->Size;
	TempZone->PartitionList[0].Size = TempZone->PartitionList[0].Sides[1].Offset;
	
	/* Link to end */
	Rover = l_HeadZone;
	
	// Fresh?
	if (!Rover)
		l_HeadZone = TempZone;
	
	// Otherwise link to end
	else
	{
		while (Rover->Next != NULL)
			Rover = Rover->Next;
		Rover->Next = TempZone;
		TempZone->Prev = Rover;
	}
	
	// Increment zone count
	l_ZoneCount++;
	
	return TempZone;
}

/* Z_Init() -- Initialize the memory manager */
void Z_Init(void)
{
	size_t FreeMem = 0;
	size_t TotalMem;
	UInt32 ToAlloc;
	Int32 ZoneCount;
	
	/* Check Free Memory */
	if (devparm)
		CONS_Printf("Z_Init: Checking free memory...\n");
	
	// Do it now
	FreeMem = I_GetFreeMem(&TotalMem);
	
	// Less than 20MB?
	if (FreeMem < FORCEDMEMORYSIZE)
	{
		if (devparm)
			CONS_Printf("Z_Init: Reported value below %iMiB, capping.\n", FORCEDMEMORYSIZE >> 20);
		
		FreeMem = FORCEDMEMORYSIZE;
	}
	
	/* Allocate */
	// Get memory from command line (if possible)
	if (M_CheckParm("-mb") && M_IsNextParm())
		ToAlloc = atoi(M_GetNextParm());
	else
		ToAlloc = (FreeMem < (32 << 20) ? FreeMem : (32 << 20));
	
	// Allocation loop
	do
	{
		// Run zone creation
		Z_CreateNewZone(ToAlloc);
		
		// CreateNewZone links in so check that
		if (!l_HeadZone)
		{
			CONS_Printf("Z_Init: Failed to allocate %iKiB.\n", ToAlloc >> 10);
			ToAlloc >>= 1;
		}
	} while (!l_HeadZone && ToAlloc > (1024 << 10));
	
	// Failed?
	if (!l_HeadZone)
	{
		I_Error("No main memory!\n");
		return;
	}
	
	CONS_Printf("Z_Init: Allocation of %iKiB worked!\n", ToAlloc >> 10);
	
	/* Create more zones based on --numzones */
	if (M_CheckParm("-numzones") && M_IsNextParm())
		ZoneCount = atoi(M_GetNextParm());
	else
		ZoneCount = 0;
	
	while (ZoneCount > 0)
	{
		if (!Z_CreateNewZone(ToAlloc))
			CONS_Printf("Z_Init: Failed to allocate extra zone.\n");
		ZoneCount--;	// Don't forget this!
	}
}

/* Z_PartitionCheckSize() -- */
boolean Z_PartitionCheckSize(Z_MemoryZoneArea_t* const SelectedZone, size_t Size, Z_MemoryPartition_t** const GotPartition)
{
	size_t i;
	
	/* Check */
	if (!SelectedZone)
		return false;
	
	/* Run through */
	for (i = 0; i < SelectedZone->UsedPartitions; i++)
		if (SelectedZone->PartitionList[i].Size >= Size)
		{
			if (GotPartition)	// For remembering so we don't have to do this again
				*GotPartition = &SelectedZone->PartitionList[i];
			return true;	// Offset sizes states that we can fit this
		}
	
	/* Fail if not returned before */
	return false;
}

/* Z_ExtendBlockList() -- Reallocates block list */
// Meant to be called from locked function
void Z_ExtendBlockList(Z_MemoryZoneArea_t* const SelectedZone)
{
	size_t ExtendCount;
	
	/* Check */
	if (!SelectedZone)
		return;
	
	/* Extend list */
	// Count
	ExtendCount = SelectedZone->BlockCount >> 3;
	
	if (ExtendCount < 32)
		ExtendCount = 32;
	
	// Do it
	SelectedZone->BlockList = realloc(SelectedZone->BlockList, sizeof(Z_MemoryBlock_t) * (SelectedZone->BlockCount + ExtendCount));
	SelectedZone->BlockCount += ExtendCount;
}

/* Z_MemPartComp() -- For generic sorting, compare two partitions */
int Z_MemPartComp(const void* const A, const void* const B)
{
	/* Check */
	if (!A || !B)
		return 0;
	
	/* Less than? */
	if ((((Z_MemoryPartition_t*)A)->Sides[0].Offset) < (((Z_MemoryPartition_t*)B)->Sides[0].Offset))
		return -1;
	
	/* Greater than? */
	else if ((((Z_MemoryPartition_t*)A)->Sides[0].Offset) > (((Z_MemoryPartition_t*)B)->Sides[0].Offset))
		return 1;
	
	/* Must be equal */
	else
		return 0;
}

/* Z_CorrectPartitions() -- Corrects partition list */
// Meant to be called from locked function
void Z_CorrectPartitions(Z_MemoryZoneArea_t* const SelectedZone)
{
	size_t i,j, k;
	Z_MemoryPartition_t Temp;
	
	/* Check */
	if (!SelectedZone)
		return;
	
	/* Speedups */
	// No more than one partition?
	if (SelectedZone->UsedPartitions <= 1)
		return;
	
	/* Sort Partitions */
	// Only need to sort if more than 1 partition
	if (SelectedZone->UsedPartitions > 1)
		Z_Sort(SelectedZone->PartitionList, sizeof(Z_MemoryPartition_t), SelectedZone->UsedPartitions, false, Z_MemPartComp);
	
	/* Merge Partitions */
	for (i = 0; i < SelectedZone->UsedPartitions; i++)
	{
		/* Remove Empty partitions */
		// Previously used a while loop...
		// But now it's basically
		// [200] [0] [0] [100]
		// [200] [100]
		// in a single step really
		
		// Count adjacent empties
		for (j = 0, k = i; !SelectedZone->PartitionList[k].Size && k < SelectedZone->UsedPartitions; j++, k++)
			;
		
		// Empties exist?
		if (j)
		{
			memmove(
					&SelectedZone->PartitionList[i],	// move to current spot
					&SelectedZone->PartitionList[k],	// Source is first non-zero size (which will be k, since j and k are relative to each other)
					sizeof(Z_MemoryPartition_t) * (SelectedZone->UsedPartitions - k)	// k from edge (k is zero based, while up is 1 based)
				);
			SelectedZone->UsedPartitions -= j;		// Lost partitions is counted partitions
		}
		
		/* Touching */
		// Might have changed since empty removal
		if (SelectedZone->UsedPartitions > 1)
			while (i < SelectedZone->UsedPartitions - 1 &&
				SelectedZone->PartitionList[i].Sides[1].Offset == SelectedZone->PartitionList[i + 1].Sides[0].Offset)
			{
				// Build Temporary
				Temp.Size = SelectedZone->PartitionList[i].Size + SelectedZone->PartitionList[i + 1].Size;
				Temp.Sides[0] = SelectedZone->PartitionList[i].Sides[0];
				Temp.Sides[1] = SelectedZone->PartitionList[i + 1].Sides[1];
				
				// Clip off
				memmove(
						&SelectedZone->PartitionList[i],			// Current
						&SelectedZone->PartitionList[i + 1],		// 1 away
						sizeof(Z_MemoryPartition_t) * (SelectedZone->UsedPartitions - (i + 1))	// move all from after current to end of list
					);
				
				// Set i to temp and such
				SelectedZone->PartitionList[i] = Temp;
				SelectedZone->UsedPartitions--;
			}
	}
}

/* Z_InternalMalloc() -- Internal handler for Z_Malloc */
// Meant to be called from a locked function
void* Z_InternalMalloc(Z_MemoryZoneArea_t* SelectedZone, Z_MemoryPartition_t* SelectedPartition, const size_t Size, const Z_MemoryTag_t Tag, void** const Ref, const size_t OrigSize
#if defined(_DEBUG)
		, const char* const File, const int Line
#endif
		)
{
	Z_MemoryBlock_t* SelectedBlock = NULL;
	void* BasePtr = NULL;
	void* RefPtr = NULL;
	size_t BaseOffset = 0;
	Z_MemoryTag_t BackTag;
	int i;
#if defined(_DEBUG)
	static UInt32 LastOrder = 0;
	Z_MemoryBlock_t OldBlock;
#endif
	
	/* Check */
	if (!SelectedZone || !Size)
		return NULL;
	
	/* Correct */
	// Tag mistake?
	BackTag = Tag;
	if (BackTag <= PU_FREE)
		BackTag = PU_STATIC;
	
	/* Do allocation stuff */
	// Find a block to use
	do
	{
		// Search
		for (i = SelectedZone->FirstFreeBlock; i < SelectedZone->BlockCount; i++)
			if (SelectedZone->BlockList[i].Tag == PU_FREE)
			{
				SelectedBlock = &SelectedZone->BlockList[i];
				break;
			}
		
		// Nothing hit? So extend blocklist (realloc blocklist, etc.)
		if (!SelectedBlock)
			Z_ExtendBlockList(SelectedZone);
	} while (!SelectedBlock);
	
	// Add to end of partition
	BaseOffset = SelectedPartition->Sides[0].Offset;
	BasePtr = SelectedPartition->Sides[0].Ptr;
	
	SelectedPartition->Sides[0].Offset += Size;
	SelectedPartition->Sides[0].Ptr = ((UInt8*)SelectedZone->Ptr) + SelectedPartition->Sides[0].Offset;
	SelectedPartition->Size -= Size;

#if defined(_DEBUG)
	OldBlock = *SelectedBlock;
#endif
	
	// Setup block info
	SelectedBlock->Ptr = BasePtr;
	SelectedBlock->Size = Size;
	SelectedBlock->OrigSize = OrigSize;
	SelectedBlock->Ref = Ref;
	SelectedBlock->Tag = BackTag;
	SelectedBlock->DiffPos = BaseOffset;

#if defined(_DEBUG)
	SelectedBlock->Order = ++LastOrder;
	strncpy(SelectedBlock->DebugInfo[0].File, File, 32);
	SelectedBlock->DebugInfo[0].Line = Line;
	SelectedBlock->DebugInfo[0].Count++;
	
	// Detect double malloc
	if (SelectedBlock->DebugInfo[0].Count != SelectedBlock->DebugInfo[1].Count + 1)
		I_Error("Free count != Malloc count, double malloc detected check OldBlock.");
#endif
	
	// Clear block memory
	memset(SelectedBlock->Ptr, 0, SelectedBlock->Size);
	
	// Setup first debug border
#if defined(_DEBUG)
	*((UInt32*)BasePtr) = 0xDEADBEEF;
	BasePtr = ((UInt8*)BasePtr) + sizeof(UInt32);
#endif
	
	// Setup block ref
	*((UInt32*)BasePtr) = SelectedBlock - SelectedZone->BlockList;
	BasePtr = ((UInt8*)BasePtr) + sizeof(UInt32);
	RefPtr = BasePtr;
	
	// Setup second debug border
#if defined(_DEBUG)
	BasePtr = ((UInt8*)SelectedBlock->Ptr) + (SelectedBlock->Size - sizeof(UInt32));
	*((UInt32*)BasePtr) = 0xDEADBEEF;
#endif
	
	// Reference
	if (SelectedBlock->Ref)
		*SelectedBlock->Ref = RefPtr;

	// Speed handling (last used and first free)
	i = SelectedBlock - SelectedZone->BlockList;
	if (i >= SelectedZone->LastUsedBlock)
		SelectedZone->LastUsedBlock = i;
	if (i == SelectedZone->FirstFreeBlock)
		SelectedZone->FirstFreeBlock++;
	
	// Manage partition changes
	Z_CorrectPartitions(SelectedZone);
	
	// Add to size
	SelectedZone->UsedSize += SelectedBlock->Size;
	
	/* Return */
	return RefPtr;
}

/* Z_MallocWrappee() -- Allocate memory */
void* Z_MallocWrappee(const size_t Size, const Z_MemoryTag_t Tag, void** const Ref
#if defined(_DEBUG)
		, const char* const File, const int Line
#endif
		)
{
	Z_MemoryZoneArea_t* Rover = NULL;
	Z_MemoryZoneArea_t* SelectedZone = NULL;
	Z_MemoryPartition_t* SelectedPartition = NULL;
	int FailCount = 0;
	void* RefPtr = NULL;
	size_t FixedSize = Size;
	
	/* Correct Size */
	// Block Ref
	FixedSize += sizeof(UInt32);
	
	// Debug Marks
#if defined(_DEBUG)
	FixedSize += sizeof(UInt32) << 1;
#endif
	
	// Segment alignment
	FixedSize = FixedSize + (SEGMENTSIZE - (FixedSize % SEGMENTSIZE));
	
	if (FixedSize < SEGMENTSIZE)
		FixedSize = SEGMENTSIZE;
	
	/* Look for a zone to lock */
	// Loop until a zone is selected
	do
	{
		/* Clear */
		SelectedZone = NULL;
		
		/* Rover not selected? */
		if (!Rover)
			Rover = l_HeadZone;
		
		// Select
		SelectedZone = Rover;
		
		/* Search through partitions to see if we can fit this */
		if (!Z_PartitionCheckSize(SelectedZone, FixedSize, &SelectedPartition))
		{
			// Free cached blocks
			FailCount++;
			
			// Unselect zone
			SelectedZone = NULL;
		}
		
		/* We have enough size to fit the new memory */
		if (SelectedZone)
		{
			// Try allocation of memory
			RefPtr = Z_InternalMalloc(SelectedZone, SelectedPartition, FixedSize, Tag, Ref, Size
#if defined(_DEBUG)
							, File, Line
#endif
						);
			
			if (RefPtr)
				return RefPtr;
			else
			{
				SelectedZone = NULL;
				FailCount++;
			}
		}

		/* Failures */
		// Failcount equal to two times the zone count?
		if (FailCount >= (l_ZoneCount << 1))
		{
			// Create a new zone possibly
			if (!Z_CreateNewZone(Rover->Size >> 2))
				CONS_Printf("Z_Malloc: Seems a new zone could not be created!\n");
			else
				FailCount = 0;
			
			// More than four times the zone count?
			if (FailCount >= (l_ZoneCount << 2))
			{
				I_Error("Appears there is no more memory left.");
				return NULL;
			}
		}
		
		/* Rover needs to be cycled */
		Rover = Rover->Next;
	} while (!SelectedZone);
	
	/* Failed if reached here */
	I_Error("Memory failed to allocate");
	return NULL;
}

/* Z_BlockForPtr() -- Get block from ptr */
Z_MemoryBlock_t* Z_BlockForPtr(Z_MemoryZoneArea_t* SelectedZone, void* const Ptr)
{
	UInt32* Scanner = NULL;
	UInt32 BlockRef = 0;
	void* Base = NULL;
	size_t i;
	
	/* Check */
	if (!SelectedZone || !Ptr)
		return NULL;
	
	/* Get base pointer */
	Scanner = Ptr;
	BlockRef = *(--Scanner);
#if defined(_DEBUG)
	Base = (void*)(--Scanner);
#else
	Base = (void*)Scanner;
#endif
	
	/* Try quick block reference */
	if (BlockRef < SelectedZone->BlockCount)
		if (SelectedZone->BlockList[BlockRef].Ptr == Base)
			return &SelectedZone->BlockList[BlockRef];
	
	/* Scan each block for reference */
	for (i = 0; i <= SelectedZone->LastUsedBlock; i++)
		if (SelectedZone->BlockList[i].Ptr == Base)
			return &SelectedZone->BlockList[i];
	
	/* Failure */
	return NULL;
}

/* Z_InternalFree() -- Free memory, internally */
void Z_InternalFree(Z_MemoryZoneArea_t* SelectedZone, void* const Ptr
#if defined(_DEBUG)
		, const char* const File, const int Line
#endif
		)
{
	Z_MemoryBlock_t* SelectedBlock = NULL;
	Int32 i;
	size_t BasedOffset, BasedStart;
	
	/* Check */
	if (!SelectedZone || !Ptr)
		return;

	/* Free memory */
	// Find Block
	SelectedBlock = Z_BlockForPtr(SelectedZone, Ptr);
	
	// In case we got passed a bogus pointer
	if (SelectedBlock)
	{
		// Fix partitions (you never know)
		Z_CorrectPartitions(SelectedZone);
		
		// Offset for partitioning
		BasedStart = (UInt8*)SelectedBlock->Ptr - (UInt8*)SelectedZone->Ptr;
		BasedOffset = BasedStart + SelectedBlock->Size;
		
		// If Z_CorrectPartitions sorts and merges partitions and we run it anyway,
		// wouldn't it be better to just make a new partition?
		i = SelectedZone->UsedPartitions++;
		SelectedZone->PartitionList[i].Size = SelectedBlock->Size;
		SelectedZone->PartitionList[i].Sides[0].Ptr = SelectedBlock->Ptr;
		SelectedZone->PartitionList[i].Sides[0].Offset = BasedStart;
		SelectedZone->PartitionList[i].Sides[1].Ptr = (UInt8*)SelectedBlock->Ptr + SelectedBlock->Size;
		SelectedZone->PartitionList[i].Sides[1].Offset = BasedOffset;
		
		// May have broken something, hopefully not!
		Z_CorrectPartitions(SelectedZone);
		
		// Speed Stuff
		SelectedZone->UsedSize -= SelectedBlock->Size;
		
		if (SelectedBlock < &SelectedZone->BlockList[SelectedZone->FirstFreeBlock])
			SelectedZone->FirstFreeBlock = SelectedBlock - SelectedZone->BlockList;
		
		if (SelectedZone->LastUsedBlock && SelectedBlock == &SelectedZone->BlockList[SelectedZone->LastUsedBlock])
			SelectedZone->LastUsedBlock--;	// last block, so no need! (just don't try it at zero)
			
		// Clear Reference
		if (SelectedBlock->Ref)
			*SelectedBlock->Ref = NULL;
		
		// Check */
#if defined(_DEBUG)
		if (*((UInt32*)(SelectedBlock->Ptr)) != 0xDEADBEEF || *((UInt32*)((void*)SelectedBlock->Ptr + (SelectedBlock->Size - sizeof(UInt32)))) != 0xDEADBEEF)
		{
			CONS_Printf("Z_Free: %s%s %hs:%i (%i) / %hs:%i (%i) [%08x %08x]\n",
					(*((UInt32*)(SelectedBlock->Ptr)) != 0xDEADBEEF ? "under" : ""),
					(*((UInt32*)((void*)SelectedBlock->Ptr + (SelectedBlock->Size - sizeof(UInt32)))) != 0xDEADBEEF ? "over" : ""),
					
					// Malloc info
					SelectedBlock->DebugInfo[0].File,
					SelectedBlock->DebugInfo[0].Line,
					SelectedBlock->DebugInfo[0].Count,
					
					// Free info
					SelectedBlock->DebugInfo[1].File,
					SelectedBlock->DebugInfo[1].Line,
					SelectedBlock->DebugInfo[1].Count,
					
					*((UInt32*)(SelectedBlock->Ptr)),
					*((UInt32*)((void*)SelectedBlock->Ptr + (SelectedBlock->Size - sizeof(UInt32))))
				);
		}
#endif
		
		// Clean Block up
		SelectedBlock->Ptr = NULL;
		SelectedBlock->Size = 0;
		SelectedBlock->OrigSize = 0;
		SelectedBlock->Tag = 0;
		SelectedBlock->DiffPos = 0;

		// Debug info (free info)
#if defined(_DEBUG)
		strncpy(SelectedBlock->DebugInfo[1].File, File, 32);
		SelectedBlock->DebugInfo[1].Line = Line;
		SelectedBlock->DebugInfo[1].Count++;
		
		// Detect count mismatch ([1].Count != [0].Count)
		if (SelectedBlock->DebugInfo[0].Count != SelectedBlock->DebugInfo[1].Count)
			I_Error("Free count != Malloc count, double malloc/free.");
#endif
	}
#if defined(QUICKDEBUG)
	else
	{
		CONS_Printf("Z_InternalFree: Passed bogus pointer %p.\n", Ptr);
		
		if (l_MemTrace)
			I_Error("Z_InternalFree: Bogus pointer %p.\n", Ptr);
	}
#endif
}

/* Z_FreeWrappee() -- Free memory */
// Hopefully I got these memmoves right!!
void Z_FreeWrappee(void* const Ptr
#if defined(_DEBUG)
		, const char* const File, const int Line
#endif
		)
{
	Z_MemoryZoneArea_t* Rover = NULL;
	
	/* Check */
	// No pointer?
	if (!Ptr)
		return;
	
	/* Look for zone containing ptr */
	// Rove list
	for (Rover = l_HeadZone; Rover; Rover = Rover->Next)
	{
		/* Not in this zone? */
		if (!(Ptr >= Rover->Ptr && Ptr < (void*)(((UInt8*)Rover->Ptr) + Rover->Size)))
			continue;
		
		// Pass to internal free
		Z_InternalFree(Rover, Ptr
#if defined(_DEBUG)
				, File, Line
#endif
			);
		
		break;	// Neat
	}
}

/* Z_FreeTagsWrappee() -- Free memory based on tags */
// Returns tags actually freed
size_t Z_FreeTagsWrappee(const Z_MemoryTag_t LowTag, const Z_MemoryTag_t HighTag
#if defined(_DEBUG)
		, const char* const File, const int Line
#endif
		)
{
	Z_MemoryZoneArea_t* Rover = NULL;
	size_t i;
	size_t c = 0;
	
	/* Check */
	// Always false?
	if (HighTag < LowTag || LowTag > HighTag)
		return 0;
	
	/* Seek blocks in zone */
	// Rove list
	for (Rover = l_HeadZone; Rover; Rover = Rover->Next)
		// Ride every block
		for (i = 0; i < Rover->LastUsedBlock; i++)
			if (Rover->BlockList[i].Tag != PU_FREE)
				if (Rover->BlockList[i].Tag >= LowTag && Rover->BlockList[i].Tag <= HighTag)
				{
					// Pass to internal free
					Z_InternalFree(Rover, Rover->BlockList[i].Ptr
#if defined(_DEBUG)
							, File, Line
#endif
						);
					
					c++;	// Increment
				}
	
	return c;
}

/* Z_ChangeTag() -- Change tag of a block */
void Z_ChangeTag(void* const Ptr, const Z_MemoryTag_t Tag)
{
	Z_MemoryZoneArea_t* Rover = NULL;
	Z_MemoryBlock_t* SelectedBlock = NULL;
	
	/* Check */
	// No pointer?
	if (!Ptr)
		return;
	
	/* Look for zone containing ptr */
	// Rove list
	for (Rover = l_HeadZone; Rover; Rover = Rover->Next)
	{
		/* Not in this zone? */
		if (!(Ptr >= Rover->Ptr && Ptr < (void*)(((UInt8*)Rover->Ptr) + Rover->Size)))
			continue;
		
		// Find block
		SelectedBlock = Z_BlockForPtr(Rover, Ptr);
		
		if (SelectedBlock)
			// Mess around
			SelectedBlock->Tag = (Tag > PU_FREE ? Tag : PU_STATIC);
		
		break;	// Neat
	}
}

void Z_CheckHeap(const int Code)
{
#if defined(_DEBUG)
	Z_MemoryZoneArea_t* Rover = NULL;
	Z_MemoryBlock_t* SelectedBlock = NULL;
	size_t i;
	
	/* Look for zone containing ptr */
	// Rove list
	for (Rover = l_HeadZone; Rover; Rover = Rover->Next)
		// Ride every block
		for (i = 0; i < Rover->LastUsedBlock; i++)
			if (Rover->BlockList[i].Tag != PU_FREE)
			{
				// Select
				SelectedBlock = &Rover->BlockList[i];
				
				if (*((UInt32*)(SelectedBlock->Ptr)) != 0xDEADBEEF || *((UInt32*)((void*)SelectedBlock->Ptr + (SelectedBlock->Size - sizeof(UInt32)))) != 0xDEADBEEF)
				{
					CONS_Printf("Z_Free: %s%s %hs:%i (%i) / %hs:%i (%i) [%08x %08x]\n",
							(*((UInt32*)(SelectedBlock->Ptr)) != 0xDEADBEEF ? "under" : ""),
							(*((UInt32*)((void*)SelectedBlock->Ptr + (SelectedBlock->Size - sizeof(UInt32)))) != 0xDEADBEEF ? "over" : ""),
					
							// Malloc info
							SelectedBlock->DebugInfo[0].File,
							SelectedBlock->DebugInfo[0].Line,
							SelectedBlock->DebugInfo[0].Count,
					
							// Free info
							SelectedBlock->DebugInfo[1].File,
							SelectedBlock->DebugInfo[1].Line,
							SelectedBlock->DebugInfo[1].Count,
					
							*((UInt32*)(SelectedBlock->Ptr)),
							*((UInt32*)((void*)SelectedBlock->Ptr + (SelectedBlock->Size - sizeof(UInt32))))
						);
				}
			}
#endif
}

