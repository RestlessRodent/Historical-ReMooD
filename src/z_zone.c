// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.sourceforge.net/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           Demyx                  (demyx@endgameftw.com)
//                    Dragan                 (poliee13@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2009 The ReMooD Team.
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
//      Modified Zone Memory Allocation Written by GhostlyDeath

#include "doomdef.h"
#include "z_zone.h"
#include "i_system.h"
#include "command.h"
#include "m_argv.h"
#include "i_video.h"
#include "doomstat.h"
#include "dstrings.h"

void Command_ClearCache_f(void);

// Experimental Segmentated Memory Manager, Created by GhostlyDeath
// Copyright (C) 2008 GhostlyDeath (ghostlydeath@gmail.com)

// This is an experimental segmented memory manager which will search via
// maps to find a place to put something, it appears to be alot faster than
// Doom's memory manager, ReMooD starts in a shorter time so I guess it would
// be more efficient.

// Small segment sizes are faster when there's not alot of memory available
// Big segment sizes are faster when there's alot of memory available

// Small Segments, Pros: Uses less memory
//                 Cons: If there is a buffer overrun, memory can be corrupted easily

// Large Segments, Pros: Blocks against memory corruption
//                 Cons: Wastes more memory

// A block size of 8 is efficient at all speeds, can compete with Zone

#define CAPNUMBER ((UInt32)0xDEADBEEF)
//#define SEGMENTSIZE (1 << 11)					// Lowest possible is (1 << 3) = 8
//#define SUBSEGMENTSIZE (SEGMENTSIZE >> 3)		// SEGMENTSIZE >> 3

#define DEFAULT_SEGMENTSIZE (1 << 11)
#define DEFAULT_SUBSEGMENTSIZE (DEFAULT_SEGMENTSIZE >> 3)

size_t SEGMENTSIZE = DEFAULT_SEGMENTSIZE;
size_t SUBSEGMENTSIZE = DEFAULT_SUBSEGMENTSIZE;

// 60 bytes w/ DEBUG + 64-bit
typedef struct MemBlock_s
{
	Int8 Tag;
	size_t Size;
	void* Ptr;
	void** User;
	
	// Segment Links (for free speed)
	size_t StartSeg;
	size_t EndSeg;
	UInt8 StartSub;
	UInt8 EndSub;

#ifdef ZDEBUG
	char File[32];
	UInt32 Line;
	
	UInt32* StartCap;
	UInt32* EndCap;
#endif
} MemBlock_t;

void* MemoryZone = NULL;
size_t AllocatedMemory = 0;
MemBlock_t* StartBlock = NULL;
size_t NumBlocks = 0;
UInt8* SegmentMap = NULL;
size_t NumSegments = 0;
size_t LastCheckedBlock = 0;
size_t LastUsedBlock = 0;
size_t FreeSegmentStart = 0;
MemBlock_t* Hashy[16];
size_t LastHashy[16];
size_t CheckedHashy[16];
size_t UsedHashy[16];

void Command_Memfree_f(void);

/* CapSizeT() -- Caps size_t to 32-bits */
#define CapSizeT(x) (x & 0xFFFFFFFF)

void Z_Init(void)
{
	size_t MemToUse;
	size_t FreeTotal;
	size_t MaxBlocks;
	size_t i, j;
	
	/* Get Some Information before we actually initialize stuff */
	I_GetFreeMem(&FreeTotal);
	CONS_Printf("Z_Init: Total free memory is %u.\n", CapSizeT(FreeTotal));
	
	/* Low memory mode? */
	if (M_CheckParm("-lowmem"))
	{
		SEGMENTSIZE = (1 << 8);
		SUBSEGMENTSIZE = (SEGMENTSIZE >> 3);
	}
	
	/* Now Determine how much memory ReMooD will REALLY use */
	if (M_CheckParm("-mb") && M_IsNextParm())
		MemToUse = atoi(M_GetNextParm());
	else
	{
		MemToUse = FreeTotal >> 4;	// Use 1/16 of free memory initialy
		
		// Low memory?
		if (M_CheckParm("-lowmem"))
		{
			if (MemToUse < (1 << 20))
				MemToUse = 1;
			else if (MemToUse > (4 << 20))
				MemToUse = 4;
			else
				MemToUse >>= 20;
		}
		
		// Now cap it between 8 and 32
		else
		{
			if (MemToUse < (8 << 20))
				MemToUse = 8;
			else if (MemToUse > (32 << 20))
				MemToUse = 32;
			else
				MemToUse >>= 20;
		}
	}

	/* Now Allocate the memory */
	MemoryZone = malloc(MemToUse << 20);
	
	if (!MemoryZone)
		I_Error("Z_Init: Failed to allocate %u bytes for Memory Zone!\n", CapSizeT(MemToUse << 20));
		
	AllocatedMemory = MemToUse << 20;
	memset(MemoryZone, 0, AllocatedMemory);
	
	/* Fill With Blocks */
	StartBlock = MemoryZone;
	if (M_CheckParm("-lowmem"))	// Low memory?
		MaxBlocks = ((size_t)((double)AllocatedMemory * 0.05)) / sizeof(MemBlock_t);
	else
		MaxBlocks = ((size_t)((double)AllocatedMemory * 0.10)) / sizeof(MemBlock_t);
	StartBlock[0].Tag = 1;
	NumBlocks = MaxBlocks;
	
	/* Hashy */
	// This will hash blocks depending on what block is used
	memset(LastHashy, 0, sizeof(LastHashy));
	memset(Hashy, 0, sizeof(Hashy));
	memset(CheckedHashy, 0, sizeof(CheckedHashy));
	memset(UsedHashy, 0, sizeof(UsedHashy));
	
	for (i = 0; i < 16; i++)
	{
		Hashy[i] = &StartBlock[((size_t)(MaxBlocks / 16)) * i];
		CheckedHashy[i] = ((size_t)(MaxBlocks / 16)) * i;
	}
	
	/* Map Segments */
	// Each segment represents an area of memory, then there are sub segments
	// which represent areas inside that part of memory.
	// Sub Segments are 16KB
	// Major Segments are 128KB
	
	NumSegments = AllocatedMemory / SEGMENTSIZE;
	SegmentMap = malloc(NumSegments);
	memset(SegmentMap, 0, NumSegments);
	
	FreeSegmentStart = (MaxBlocks * sizeof(MemBlock_t)) / SEGMENTSIZE;
	
	// Declare the Beginning segments locked completely
	for (i = 0; i <= (MaxBlocks * sizeof(MemBlock_t)) / SEGMENTSIZE; i++)
		SegmentMap[i] = 0xFF;
	
	/* Status Report */
	CONS_Printf("Z_Init: Allocated %u bytes of memory.\n", CapSizeT(AllocatedMemory));
	CONS_Printf("Z_Init: Blocks: %u\n", CapSizeT(MaxBlocks));
	if (SEGMENTSIZE >> 10)
		CONS_Printf("Z_Init: Segments: %u (%uKiB)\n", CapSizeT(NumSegments), SEGMENTSIZE >> 10);
	else
		CONS_Printf("Z_Init: Segments: %u (%uB)\n", CapSizeT(NumSegments), SEGMENTSIZE);
	CONS_Printf("Z_Init: Segments used by Blocks: %u\n", CapSizeT((MaxBlocks * sizeof(MemBlock_t)) / SEGMENTSIZE));
	CONS_Printf("Z_Init: Wasted sub-segments: %u (%uB)\n",
		((MaxBlocks * sizeof(MemBlock_t)) -
		((CapSizeT((MaxBlocks * sizeof(MemBlock_t)) / SEGMENTSIZE)) * SEGMENTSIZE)) / (SEGMENTSIZE >> 3),
		(((MaxBlocks * sizeof(MemBlock_t)) -
		((CapSizeT((MaxBlocks * sizeof(MemBlock_t)) / SEGMENTSIZE)) * SEGMENTSIZE)) / (SEGMENTSIZE >> 3)) * SEGMENTSIZE
		);
	CONS_Printf("Z_Init: Free Segments start at %u\n", CapSizeT(FreeSegmentStart));
	CONS_Printf("Z_Init: Segments left for allocation: %u\n", NumSegments - CapSizeT((MaxBlocks * sizeof(MemBlock_t)) / SEGMENTSIZE));
	
	/* Free Memory Command */
	COM_AddCommand("memfree", Command_Memfree_f);
	COM_AddCommand("clearcache", Command_ClearCache_f);
}

/* Z_MallocAlign() -- Allocates Memory */
#ifndef ZDEBUG
void* Z_MallocAlign(size_t size, int tag, void** user, int alignbits)
#else
void* Z_MallocAlign2(size_t size, int tag, void** user, int alignbits, char* file, int line)
#endif
{
	size_t i;
	size_t j;
	size_t StartSeg = -1, EndSeg = -1;
	size_t StartSub = -1, EndSub = -1;
	size_t SizeInSegments;
	char HasSpace;
	MemBlock_t* Rover = NULL;
	void* CreationSpot = NULL;
	void* HashyStart = NULL;
	size_t HashyNum = 0;
	
#ifdef ZDEBUG
	size_t realsize;
#else
	#define realsize size
#endif
	
	/* Check to see if the memory manager has been started */
	if (!MemoryZone)
		I_Error("Z_MallocAlign: Allocate called before Z_Init!\n");
		
	/* Precalculations */
#ifdef ZDEBUG
	realsize = size;
#endif
	
	if (alignbits)
		realsize += realsize % (alignbits << 3);

#ifdef ZDEBUG
	realsize = realsize + (sizeof(UInt32) * 2);
#endif
		
	/* Find a Free Segment */
	if (realsize < SUBSEGMENTSIZE)
	{
		// Check the Segment Map
		for (i = FreeSegmentStart; i < NumSegments; i++)
			if (SegmentMap[i] != 0xFF)	// Ignore Fully used Segments
				for (j = 0; j < 8; j++)
					if (!(SegmentMap[i] & (1 << j)))
					{
						StartSeg = EndSeg = i;
						StartSub = EndSub = j;
						break;
					}
	}
	else if (realsize < SEGMENTSIZE)
	{
		// Check the Segment Map
		for (i = FreeSegmentStart; i < NumSegments; i++)
			if (!(SegmentMap[i] & 0xFF))
			{
				StartSeg = EndSeg = i;
				StartSub = 0;
				EndSub = realsize / (SEGMENTSIZE >> 3);
				break;
			}
	}
	else
	{
		// Check the Segment Map -- this method could probably be improved
		SizeInSegments = realsize / SEGMENTSIZE;
		
		for (i = FreeSegmentStart; i < NumSegments; i++)
			if (!(SegmentMap[i] & 0xFF))
			{
				HasSpace = 0;
				
				for (j = i + 1; j < i + 1 + SizeInSegments; j++)
				{
					if (!(SegmentMap[j] & 0xFF))
						HasSpace = 1;
					else
					{
						HasSpace = 0;
						break;
					}
				}
				
				if (HasSpace)
				{
					StartSeg = i;
					EndSeg = i + SizeInSegments;
					StartSub = 0;
					EndSub = ((realsize % SUBSEGMENTSIZE) >> 3);
					break;
				}
			}
	}
	
	// Do we crash?
	if (StartSeg == (size_t)-1)
		I_Error("Z_MallocAlign: Out of segments!\n");
		
	// Fill in used segments
	for (i = StartSeg; i <= EndSeg; i++)
		if (i == EndSeg)
			for (j = StartSub; j <= EndSub; j++)
				SegmentMap[i] |= (1 << j);
		else
			SegmentMap[i] = 0xFF;
			
	CreationSpot = ((size_t)MemoryZone) + (StartSeg * SEGMENTSIZE) + (StartSub * SUBSEGMENTSIZE);
	
	/* Find out what Hashy says */
	HashyNum = StartSeg - (StartSeg % ((size_t)(NumSegments / 16)));	// chop off to something that can be * 16
	HashyNum = HashyNum / ((size_t)(NumSegments / 16));
	
	/* Find a free block */
	i = CheckedHashy[HashyNum];
	for (;; i++)
	{
		if (i < CheckedHashy[HashyNum] && i == (CheckedHashy[HashyNum] - 1))
			break;
		else if (i >= CheckedHashy[HashyNum] && i == NumBlocks)
			i = 0;
		
		if (!StartBlock[i].Tag)
		{
			Rover = &StartBlock[i];
			CheckedHashy[HashyNum] = i + 1;
		
			if (i > UsedHashy[HashyNum])
				UsedHashy[HashyNum] = i;
			
			break;
		}
	}
	
	if (i == NumBlocks)
		I_Error("Z_MallocAlign: No more blocks!\n");
	
	/* Set the Block Parameters */
#ifdef ZDEBUG
	// Debug Parameters
	Rover->StartCap = CreationSpot;
	Rover->EndCap = (UInt8*)CreationSpot + (realsize - sizeof(UInt32));
	*(Rover->StartCap) = 0xDEADBEEF;
	*(Rover->EndCap) = 0xDEADBEEF;
	
	snprintf(Rover->File, 32, "%s", file);
	Rover->Line = line;
#endif

	// Normal Parameters
	Rover->Tag = tag;
	Rover->Size = realsize;
	Rover->Ptr = CreationSpot;
	Rover->User = user;
	
	// Segments for fast free
	Rover->StartSeg = StartSeg;
	Rover->EndSeg = EndSeg;
	Rover->StartSub = StartSub;
	Rover->EndSub = EndSub;
	
	/* Push FreeSegmentStart */
	for (i = FreeSegmentStart; i < NumSegments; i++)
		if (SegmentMap[i] != 0xFF)
		{
			FreeSegmentStart = i;
			break;
		}
		
#ifdef ZDEBUG
	return (UInt8*)CreationSpot + sizeof(UInt32);
#else
	return CreationSpot;
#endif
	
#ifndef ZDEBUG
	#undef realsize
#endif
}

/* Z_Free() -- Frees memory */
#ifndef ZDEBUG
void Z_Free2(void* ptr, void* Block)
#else
void Z_Free2(void* ptr, void* Block, char* file, int line)
#endif
{
	size_t i = 0, j = 0, k = 0;
	size_t StartSeg = 0;
	size_t ChoppedPtr = 0;
	size_t HashyNum = 0;
	
	/* Check to see if the memory manager has been started */
	if (!MemoryZone)
		I_Error("Z_Free: Free called before Z_Init!\n");
	
	/* Direct Block Free */
	if (Block && Block >= MemoryZone && Block <= ((UInt8*)MemoryZone + AllocatedMemory))
	{
		// Clear Segments
		for (j = ((MemBlock_t*)Block)->StartSeg; j <= ((MemBlock_t*)Block)->EndSeg; j++)
			if (j == ((MemBlock_t*)Block)->EndSeg)
				for (k = ((MemBlock_t*)Block)->StartSub; k <= ((MemBlock_t*)Block)->EndSub; k++)
					SegmentMap[j] &= ~(1 << k);
			else
				SegmentMap[j] = 0;
			
#ifdef ZDEBUG
		if (*(((MemBlock_t*)Block)->StartCap) != 0xDEADBEEF)
			if (devparm)
				CONS_Printf("Z_Free: Block %i's StartCap is corrupt (0x%08x) [Alloc: %s:%i; Free: %s:%i]\n",
					((size_t)Block - (size_t)MemoryZone) / sizeof(MemBlock_t), *(((MemBlock_t*)Block)->StartCap), ((MemBlock_t*)Block)->File, ((MemBlock_t*)Block)->Line, file, line);
				
		if (*(((MemBlock_t*)Block)->EndCap) != 0xDEADBEEF)
			if (devparm)
				CONS_Printf("Z_Free: Block %i's EndCap is corrupt (0x%08x) [Alloc: %s:%i; Free: %s:%i]\n",
					((size_t)Block - (size_t)MemoryZone) / sizeof(MemBlock_t), *(((MemBlock_t*)Block)->EndCap), ((MemBlock_t*)Block)->File, ((MemBlock_t*)Block)->Line, file, line);
			
		memset(((MemBlock_t*)Block)->File, 0, sizeof(((MemBlock_t*)Block)->File));
		((MemBlock_t*)Block)->Line = 0;

		((MemBlock_t*)Block)->StartCap = NULL;
		((MemBlock_t*)Block)->EndCap = NULL;
#endif
	
		// Clear Block
		memset(((MemBlock_t*)Block)->Ptr, 0, ((MemBlock_t*)Block)->Size);
		((MemBlock_t*)Block)->Tag = 0;
		((MemBlock_t*)Block)->Ptr = NULL;
		if (((MemBlock_t*)Block)->User)
			*(((MemBlock_t*)Block)->User) = NULL;
		((MemBlock_t*)Block)->Size = 0;
	}
	
	/* Block search */
	else
	{
		if (ptr < MemoryZone || ptr > (size_t)MemoryZone + AllocatedMemory)
		{
#ifdef ZDEBUG
			if (devparm)
				CONS_Printf("Z_Free: Free outside bounds from %s:%i.\n", file, line);
#endif
			return;
		}
		
#ifdef ZDEBUG
		ptr = ((size_t)ptr) - sizeof(UInt32);
#endif
		
		/* Get Estimated Segment based on Pointer */
		// CreationSpot = ((UInt8*)MemoryZone) + (StartSeg * SEGMENTSIZE) + (StartSub * SUBSEGMENTSIZE);
		// NumSegments = AllocatedMemory / SEGMENTSIZE;
		ChoppedPtr = (size_t)ptr - (size_t)MemoryZone;
		StartSeg = (double)NumSegments * ((double)ChoppedPtr / (double)AllocatedMemory);
		
		/* Find out what Hashy Says */
		HashyNum = StartSeg - (StartSeg % ((size_t)(NumSegments / 16)));	// chop off to something that can be * 16
		HashyNum = HashyNum / ((size_t)(NumSegments / 16));
		
		/* Search for our pointer match */
		i = UsedHashy[HashyNum];
		for (;; i--)
		{
			if (i > UsedHashy[HashyNum] && i == (UsedHashy[HashyNum] + 1))
				break;
			else if (i <= UsedHashy[HashyNum] && i == 0)
				i = NumBlocks - 1;
		
			if (StartBlock[i].Ptr == ptr)
			{
				// Clear Segments
				for (j = StartBlock[i].StartSeg; j <= StartBlock[i].EndSeg; j++)
					if (j == StartBlock[i].EndSeg)
						for (k = StartBlock[i].StartSub; k <= StartBlock[i].EndSub; k++)
							SegmentMap[j] &= ~(1 << k);
					else
						SegmentMap[j] = 0;
					
#ifdef ZDEBUG
				if (*(StartBlock[i].StartCap) != 0xDEADBEEF)
					if (devparm)
						CONS_Printf("Z_Free: Block %i's StartCap is corrupt (0x%08x) [Alloc: %s:%i; Free: %s:%i]\n",
							i, *(StartBlock[i].StartCap), StartBlock[i].File, StartBlock[i].Line, file, line);
						
				if (*(StartBlock[i].EndCap) != 0xDEADBEEF)
					if (devparm)
						CONS_Printf("Z_Free: Block %i's EndCap is corrupt (0x%08x) [Alloc: %s:%i; Free: %s:%i]\n",
							i, *(StartBlock[i].EndCap), StartBlock[i].File, StartBlock[i].Line, file, line);
					
				memset(StartBlock[i].File, 0, sizeof(StartBlock[i].File));
				StartBlock[i].Line = 0;
	
				StartBlock[i].StartCap = NULL;
				StartBlock[i].EndCap = NULL;
#endif
			
				// Clear Block
				memset(StartBlock[i].Ptr, 0, StartBlock[i].Size);
				StartBlock[i].Tag = 0;
				StartBlock[i].Ptr = NULL;
				if (StartBlock[i].User)
					*StartBlock[i].User = NULL;
				StartBlock[i].Size = 0;

				// If this is the last block, let's make it so and if we just freed it...
				if (i == UsedHashy[HashyNum])
					UsedHashy[HashyNum]--;
			
				if (CheckedHashy[HashyNum] > i)
					CheckedHashy[HashyNum] = i;
				return;
			}
		}
	}
}

/* Z_ChangeTag2() -- Changes a ptr's tag */
void Z_ChangeTag2(void *ptr, int tag)
{
	size_t i = 0;
	size_t StartSeg = 0;
	size_t ChoppedPtr = 0;
	size_t HashyNum = 0;
	
	/* Check to see if the memory manager has been started */
	if (!MemoryZone)
		I_Error("Z_ChangeTag2: ChangeTag called before Z_Init!\n");
		
	if (ptr < MemoryZone || ptr > (size_t)MemoryZone + AllocatedMemory)
	{
#ifdef ZDEBUG
		if (devparm)
			CONS_Printf("Z_ChangeTag2: Pointer outside bounds.\n");
#endif
		return;
	}
		
#ifdef ZDEBUG
	ptr = ((size_t)ptr) - sizeof(UInt32);
#endif
		
	/* Get Estimated Segment based on Pointer */
	// CreationSpot = ((UInt8*)MemoryZone) + (StartSeg * SEGMENTSIZE) + (StartSub * SUBSEGMENTSIZE);
	// NumSegments = AllocatedMemory / SEGMENTSIZE;
	ChoppedPtr = (size_t)ptr - (size_t)MemoryZone;
	StartSeg = (double)NumSegments * ((double)ChoppedPtr / (double)AllocatedMemory);
		
	/* Find out what Hashy Says */
	HashyNum = StartSeg - (StartSeg % ((size_t)(NumSegments / 16)));	// chop off to something that can be * 16
	HashyNum = HashyNum / ((size_t)(NumSegments / 16));
		
	/* Search for our pointer match */
	i = UsedHashy[HashyNum];
	for (;; i--)
	{
		if (i > UsedHashy[HashyNum] && i == (UsedHashy[HashyNum] + 1))
			break;
		else if (i <= UsedHashy[HashyNum] && i == 0)
			i = NumBlocks - 1;
		
		if (StartBlock[i].Ptr == ptr)
		{
			StartBlock[i].Tag = tag;
			return;
		}
	}
}

/* Z_FreeTags() -- Frees all tags in the tags specified */
void Z_FreeTags(int lowtag, int hightag)
{
	size_t i, j;
	size_t last;
#ifdef ZDEBUG
	size_t Count = 0;
	size_t Bytes = 0;
#endif
	
	if (devparm)
		CONS_Printf("Z_FreeTags: Clearing %i to %i.\n", lowtag, hightag);
	
	for (i = 0; i < 16; i++)
	{
		last = UsedHashy[i];
		
		for (j = ((size_t)(NumBlocks / 16)) * i; j < last; j++)
			if ((StartBlock[j].Tag >= lowtag) && (StartBlock[j].Tag <= hightag))
			{
#ifdef ZDEBUG
				Bytes += StartBlock[j].Size;
				Count++;
				Z_Free2(StartBlock[j].Ptr, &StartBlock[j], __FILE__, __LINE__);
#else
				Z_Free2(StartBlock[j].Ptr, &StartBlock[j]);
#endif
			}
	}
		
#ifdef ZDEBUG
	if (devparm)
		CONS_Printf("Z_FreeTags: Freed %i blocks (%lu bytes).\n", Count, Bytes);
#endif
}

/* Z_DumpHeap() -- Prints Heap Information */
void Z_DumpHeap(int lowtag, int hightag)
{
}

/* Z_FileDumpHeap() -- Dumps all the memory to a file */
void Z_FileDumpHeap(FILE * f)
{
}

/* Z_CheckHeap() -- Checks the heap for corruption */
void Z_CheckHeap(int k)
{
}

/* Z_TagUsage() -- Prints the space tags are using */
size_t Z_TagUsage(int tagnum)
{
	return 0;
}

/* Z_FreeMemory() -- Returns memory that is free */
void Z_FreeMemory(int *realfree, int *cachemem, int *usedmem, int *largefreeblock)
{
}

/* Command_Memfree_f() -- Returns memory that is free in a console command */
void Command_Memfree_f(void)
{
	size_t i, j;
	size_t UsedBlocks = 0;
	size_t UsedSegments = 0;
	size_t UsedSubSegments = 0;
	size_t UsedSize = 0;
	
	CONS_Printf("*** Memory Usage Report***\n");
	
	// Used Blocks
	for (i = 0; i < NumBlocks; i++)
		if (StartBlock[i].Tag)
		{
			//printf("Block %i has %20lu\n", i, StartBlock[i].Size);
			UsedBlocks++;
			UsedSize += StartBlock[i].Size;
		}
		
	// Used Segments
	for (i = 0; i < NumSegments; i++)
		if (SegmentMap[i] & 0xFF)
			UsedSegments++;
	
	// Used SubSegments
	for (i = 0; i < NumSegments; i++)
		if (SegmentMap[i] & 0xFF)
			for (j = 0; j < 8; j++)
				if (SegmentMap[i] & (1 << j))
					UsedSubSegments++;
	
	CONS_Printf("BLK Used Bytes  : %10lu / %10lu (%3i%%)\n",
		NumBlocks * sizeof(MemBlock_t), AllocatedMemory,
		(int)(((double)(NumBlocks * sizeof(MemBlock_t)) / (double)AllocatedMemory) * 100));
		
	CONS_Printf("User Used Bytes : %10lu / %10lu (%3i%%)\n",
		UsedSize, AllocatedMemory,
		(int)(((double)(UsedSize) / (double)AllocatedMemory) * 100));
		
	CONS_Printf("Total Used Bytes: %10lu / %10lu (%3i%%)\n",
		(NumBlocks * sizeof(MemBlock_t)) + UsedSize, AllocatedMemory,
		(int)(((double)((NumBlocks * sizeof(MemBlock_t)) + UsedSize) / (double)AllocatedMemory) * 100));
	
	CONS_Printf("Blocks          : %10lu / %10lu (%3i%%)\n",
		UsedBlocks, NumBlocks,
		(int)(((double)UsedBlocks / (double)NumBlocks) * 100));
		
	CONS_Printf("Segments        : %10lu / %10lu (%3i%%)\n",
		UsedSegments, NumSegments,
		(int)(((double)UsedSegments / (double)NumSegments) * 100));
		
	CONS_Printf("Sub-Segments    : %10lu / %10lu (%3i%%)\n",
		UsedSubSegments, NumSegments * 8,
		(int)(((double)UsedSubSegments / (double)(NumSegments * 8)) * 100));
}

/* Z_CheckCollision() -- Checks for Data Collision */
#ifdef ZDEBUG
int Z_CheckCollision2(void* ptr, void* Start, size_t Length, char* file, int line)
#else
int Z_CheckCollision(void* ptr, void* Start, size_t Length)
#endif
{
#ifdef ZDEBUG
	size_t i = 0;
	size_t StartSeg = 0;
	size_t ChoppedPtr = 0;
	size_t HashyNum = 0;
	
	/* Check to see if the memory manager has been started */
	if (!MemoryZone)
		I_Error("Z_CheckCollision2: CheckCollision called before Z_Init!\n");
		
	if (ptr < MemoryZone || ptr > (size_t)MemoryZone + AllocatedMemory)
	{
		if (devparm)
			CONS_Printf("Z_CheckCollision2: ptr outside bounds.\n");
		return 0;
	}
	
	ptr = ((size_t)ptr) - sizeof(UInt32);
		
	/* Get Estimated Segment based on Pointer */
	// CreationSpot = ((UInt8*)MemoryZone) + (StartSeg * SEGMENTSIZE) + (StartSub * SUBSEGMENTSIZE);
	// NumSegments = AllocatedMemory / SEGMENTSIZE;
	ChoppedPtr = (size_t)ptr - (size_t)MemoryZone;
	StartSeg = (double)NumSegments * ((double)ChoppedPtr / (double)AllocatedMemory);
		
	/* Find out what Hashy Says */
	HashyNum = StartSeg - (StartSeg % ((size_t)(NumSegments / 16)));	// chop off to something that can be * 16
	HashyNum = HashyNum / ((size_t)(NumSegments / 16));
		
	/* Search for our pointer match */
	i = UsedHashy[HashyNum];
	for (;; i--)
	{
		if (i > UsedHashy[HashyNum] && i == (UsedHashy[HashyNum] + 1))
			break;
		else if (i <= UsedHashy[HashyNum] && i == 0)
			i = NumBlocks - 1;
		
		if (StartBlock[i].Ptr == ptr)
		{
			if (!((Start >= StartBlock[i].Ptr && Start <= ((size_t)StartBlock[i].Ptr + StartBlock[i].Size)) &&
				(((size_t)Start + Length) >= StartBlock[i].Ptr && ((size_t)Start + Length) <= ((size_t)StartBlock[i].Ptr + StartBlock[i].Size))))
			{
				CONS_Printf("Z_CheckCollision2: Collision detected: Write to %p to %p will %s by %ul bytes [Checker: %s:%i]\n",
					Start, (size_t)Start + Length,
					(Start < StartBlock[i].Ptr ? "underun" : "overrun"),
					(Start < StartBlock[i].Ptr ? (ssize_t)StartBlock[i].Ptr - (ssize_t)Start : ((size_t)Start + Length) - ((size_t)StartBlock[i].Ptr + StartBlock[i].Size)),
					file, line);
				return 1;
			}
			else
				return 0;
		}
	}

#endif	
	return 0;
}

/* CommandClearCache_f() -- Clears un-important data */
void Command_ClearCache_f(void)
{
	CONS_Printf("Clearing cache...\n");
	Z_FreeTags(PU_PURGELEVEL, PU_MAXPURGELEVEL);
}

char *Z_Strdup(const char *s, int tag, void **user)
{
	return strcpy(Z_Malloc(strlen(s) + 1, tag, user), s);
}

wchar_t* Z_StrdupW(const wchar_t* w, int tag, void** user)
{
	return UNICODE_Copy(Z_Malloc((UNICODE_StringLength(w) * sizeof(wchar_t)) + 2, tag, user), w);
}

