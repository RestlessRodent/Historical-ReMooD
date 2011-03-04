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

//#define NEWMEM
#ifdef NEWMEM

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
#define SEGMENTSIZE 8								// Forced alignment of portions
#define MAXZONES 10									// Maximum zones before we crash
#define BEEFPOINT Size

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
	void* RefPtr;
	char DebugMark[128];
	struct
	{
		char File[32];										// File malloced/freed in
		uint32_t Line;										// Line of file
		uint32_t Count;										// Times malloced/freed
	} DebugInfo[2];
	uint32_t Order;											// Order number
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

/* Command_z_dump_f() -- Dump all memory */
void Command_z_dump_f(void)
{
#define ASCIISIZE 20
	Z_MemoryZoneArea_t* Rover = NULL;
	Z_MemoryBlock_t* SelectedBlock = NULL;
	Z_MemoryPartition_t* SelectedPartition = NULL;
	size_t i, j, k, l;
	uint8_t* x, y;
	uint8_t Line[ASCIISIZE];
	uint8_t LastLine[ASCIISIZE];
	boolean WasLast;
	
	FILE* f;
	
	/* Open */
	f = fopen("memdump", "wt");
	
	if (!f)
		return;
	
	/* Start Dumping */
	// Rove list
	for (Rover = l_HeadZone; Rover; Rover = Rover->Next)
	{
		// Print zone info
		fprintf(f, "###### Zone %p ######\n", Rover);
		fprintf(f, "\n## Header\n");
		fprintf(f, "\tPtr = %p\n", Rover->Ptr);
		fprintf(f, "\tSize = %u\n", Rover->Size);
		fprintf(f, "\tUsedSize = %u\n", Rover->UsedSize);
		fprintf(f, "\tBlockList = %p\n", Rover->BlockList);
		fprintf(f, "\tBlockCount = %u\n", Rover->BlockCount);
		fprintf(f, "\tNumCacheBlocks = %u\n", Rover->NumCacheBlocks);
		fprintf(f, "\tFirstFreeBlock = %u\n", Rover->FirstFreeBlock);
		fprintf(f, "\tLastUsedBlock = %u\n", Rover->LastUsedBlock);
		fprintf(f, "\tPartitionList = %p\n", Rover->PartitionList);
		fprintf(f, "\tPartitionCount = %u\n", Rover->PartitionCount);
		fprintf(f, "\tUsedPartitions = %u\n", Rover->UsedPartitions);
		fprintf(f, "\tPrev = %p\n", Rover->Prev);
		fprintf(f, "\tNext = %p\n", Rover->Next);
		
		// Ride every partition
		fprintf(f, "\n## Partitions\n");
		for (i = 0; i < Rover->PartitionCount; i++)
		{
			// Select
			SelectedPartition = &Rover->PartitionList[i];
			
			fprintf(f, "%i:\n", i);
			fprintf(f, "\t[%i].Size = %u\n", i, SelectedPartition->Size);
			fprintf(f, "\t[%i].Sides[0].Ptr = %p\n", i, SelectedPartition->Sides[0].Ptr);
			fprintf(f, "\t[%i].Sides[0].Offset = %u\n", i, SelectedPartition->Sides[0].Offset);
			fprintf(f, "\t[%i].Sides[1].Ptr = %p\n", i, SelectedPartition->Sides[1].Ptr);
			fprintf(f, "\t[%i].Sides[1].Offset = %u\n", i, SelectedPartition->Sides[1].Offset);
		}
		
		// Ride every block
		fprintf(f, "\n## Blocks\n");
		for (i = 0; i < Rover->BlockCount; i++)
		{
			// Select
			SelectedBlock = &Rover->BlockList[i];
			
			fprintf(f, "%i:\n", i);
			
			// Info
			if (SelectedBlock->Tag != PU_FREE)
			{
				fprintf(f, "\t[%i].Ptr = %p\n", i, SelectedBlock->Ptr);
#if defined(_DEBUG)
				fprintf(f, "\t[%i].RefPtr = %p\n", i, SelectedBlock->RefPtr);
#endif
				fprintf(f, "\t[%i].Size = %u\n", i, SelectedBlock->Size);
				fprintf(f, "\t[%i].OrigSize = %u\n", i, SelectedBlock->OrigSize);
				fprintf(f, "\t[%i].Ref = %p -> %p\n", i,
					SelectedBlock->Ref, (SelectedBlock->Ref ? *SelectedBlock->Ref : NULL));
				fprintf(f, "\t[%i].Tag = %u\n", i, SelectedBlock->Tag);
				fprintf(f, "\t[%i].DiffPos = %u\n", i, SelectedBlock->DiffPos);
			
#if defined(_DEBUG)
				fprintf(f, "\t[%i].DebugMark = \"%s\"\n", i, SelectedBlock->DebugMark);
				fprintf(f, "\t[%i].Order = %u\n", i, SelectedBlock->Order);
				fprintf(f, "\t[%i].DebugInfo[0].File = \"%s\"\n", i, SelectedBlock->DebugInfo[0].File);
				fprintf(f, "\t[%i].DebugInfo[0].Line = %u\n", i, SelectedBlock->DebugInfo[0].Line);
				fprintf(f, "\t[%i].DebugInfo[0].Count = %u\n", i, SelectedBlock->DebugInfo[0].Count);
				fprintf(f, "\t[%i].DebugInfo[1].File = \"%s\"\n", i, SelectedBlock->DebugInfo[1].File);
				fprintf(f, "\t[%i].DebugInfo[1].Line = %u\n", i, SelectedBlock->DebugInfo[1].Line);
				fprintf(f, "\t[%i].DebugInfo[1].Count = %u\n", i, SelectedBlock->DebugInfo[1].Count);
#endif
			
				// ASCII display
				if (SelectedBlock->Ptr)
				{
					// Clear bytes
					memset(LastLine, 0, sizeof(LastLine));
					memset(Line, 0, sizeof(Line));
					WasLast = false;
					
					// Run each line
					for (l = 0, j = 0; j < SelectedBlock->Size; j += ASCIISIZE, l++)
					{
						// Copy line to last line
						memmove(LastLine, Line, sizeof(LastLine));
						
						// Copy now to Line
						memmove(Line, &((uint8_t*)SelectedBlock->Ptr)[j], sizeof(Line));
						
						// Print data
						if (l == 0 || j + ASCIISIZE >= SelectedBlock->Size || memcmp(Line, LastLine, sizeof(Line)))
						{
							// Base
							fprintf(f, "%06X: ", j);
							
							// Hex
							for (k = j; k < j + ASCIISIZE; k++)
							{
								if (k < SelectedBlock->Size)
								{
									fprintf(f, "%02x", ((uint8_t*)SelectedBlock->Ptr)[k]);
#if !defined(_DEBUG)
									fprintf(f, " ");
#else
									if (k >= 8 && k < 8 + SelectedBlock->OrigSize)
										fprintf(f, "+");
									else
										fprintf(f, " ");
#endif
								}
								else
									fprintf(f, "__ ");
							}
							
							// Chars
							for (k = j; k < j + ASCIISIZE; k++)
								if (k < SelectedBlock->Size)
									fprintf(f, "%c", (((uint8_t*)SelectedBlock->Ptr)[k] >= ' ' && ((uint8_t*)SelectedBlock->Ptr)[k] < 0x7F ? ((uint8_t*)SelectedBlock->Ptr)[k] : '.'));
								else
									fprintf(f, "_");
							
							WasLast = false;
							fprintf(f, "\n");
						}
						else
						{
							if (!WasLast)
							{
								fprintf(f, "*");
								fprintf(f, "\n");
								WasLast = true;
							}
						}
					}
					
#if 0
					for (x = SelectedBlock->Ptr, j = 0; j < SelectedBlock->Size;)
					{
						// Print base address:
						fprintf(f, "%06X: ", (uint8_t*)x - (uint8_t*)SelectedBlock->Ptr);
					
						// Print hex
						for (l = 0, k = j; k < j + ASCIISIZE; k++, l++)
						{
							
							
							// Always print on first/last or non-match
							if (l == 0 || k + ASCIISIZE >= SelectedBlock->Size ||
								memcmp
							{
								if (k < SelectedBlock->Size)
									fprintf(f, "%02x ", ((uint8_t*)SelectedBlock->Ptr)[k]);
								else
									fprintf(f, "__ ");
							}
						}
					
						// Print chars
						/*for (k = j; k < j + ASCIISIZE; k++)
							if (k < SelectedBlock->Size)
								fprintf(f, "%c", (((uint8_t*)SelectedBlock->Ptr)[k] >= ' ' && ((uint8_t*)SelectedBlock->Ptr)[k] < 0x7F ? ((uint8_t*)SelectedBlock->Ptr)[k] : '.'));
							else
								fprintf(f, "_");*/
					
						fprintf(f, "\n");
					
						// Skip
						x += ASCIISIZE;
						j += ASCIISIZE;
					}
#endif
				}
			}
			else
			{
#if defined(_DEBUG)
				fprintf(f, "\t[%i].Order = %u\n", i, SelectedBlock->Order);
				fprintf(f, "\t[%i].DebugInfo[0].File = \"%s\"\n", i, SelectedBlock->DebugInfo[0].File);
				fprintf(f, "\t[%i].DebugInfo[0].Line = %u\n", i, SelectedBlock->DebugInfo[0].Line);
				fprintf(f, "\t[%i].DebugInfo[0].Count = %u\n", i, SelectedBlock->DebugInfo[0].Count);
				fprintf(f, "\t[%i].DebugInfo[1].File = \"%s\"\n", i, SelectedBlock->DebugInfo[1].File);
				fprintf(f, "\t[%i].DebugInfo[1].Line = %u\n", i, SelectedBlock->DebugInfo[1].Line);
				fprintf(f, "\t[%i].DebugInfo[1].Count = %u\n", i, SelectedBlock->DebugInfo[1].Count);
#endif
			}
				
			fprintf(f, "\n");
		}
		
		fprintf(f, "\n");
	}
	
	/* Close */
	fclose(f);
#undef ASCIISIZE
}

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
#define SORTELEMOF(base,sz,i) (((uint8_t*)(base)) + ((sz) * (i)))
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
Z_MemoryZoneArea_t* Z_CreateNewZone(const uint32_t Size)
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
	TempZone->BlockCount = 32;//(Size / sizeof(Z_MemoryBlock_t)) / 50;
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
	TempZone->PartitionList[0].Sides[1].Ptr = ((uint8_t*)TempZone->Ptr) + TempZone->Size;
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
	uint32_t ToAlloc;
	int32_t ZoneCount;
	
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
		ToAlloc = atoi(M_GetNextParm()) << 20;
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
		ZoneCount = atoi(M_GetNextParm()) - 1;
	else
		ZoneCount = 0;
	
	while (ZoneCount > 0)
	{
		if (!Z_CreateNewZone(ToAlloc))
			CONS_Printf("Z_Init: Failed to allocate extra zone.\n");
		ZoneCount--;	// Don't forget this!
	}
	
	COM_AddCommand("zmemdump", Command_z_dump_f);
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
	static uint32_t LastOrder = 0;
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
	SelectedPartition->Sides[0].Ptr = ((uint8_t*)SelectedZone->Ptr) + SelectedPartition->Sides[0].Offset;
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
	memset(SelectedBlock->DebugMark, 0, sizeof(SelectedBlock->DebugMark));
	SelectedBlock->Order = ++LastOrder;
	strncpy(SelectedBlock->DebugInfo[0].File, File, 32);
	SelectedBlock->DebugInfo[0].Line = Line;
	SelectedBlock->DebugInfo[0].Count++;
	
	// Detect double malloc
	if (SelectedBlock->DebugInfo[0].Count != SelectedBlock->DebugInfo[1].Count + 1)
		I_Error("Free count != Malloc count, double malloc detected check OldBlock.");
#endif
	
	// Clear block memory
	memset(SelectedBlock->Ptr, 0xFF, SelectedBlock->Size);		// Off-zone
	
	// Setup first debug border
#if defined(_DEBUG)
	*((uint32_t*)BasePtr) = 0xDEADBEEF;
	BasePtr = ((uint8_t*)BasePtr) + sizeof(uint32_t);
#endif
	
	// Setup block ref
	*((uint32_t*)BasePtr) = SelectedBlock - SelectedZone->BlockList;
	BasePtr = ((uint8_t*)BasePtr) + sizeof(uint32_t);
	RefPtr = BasePtr;
	
	// Setup second debug border
#if defined(_DEBUG)
	BasePtr = ((uint8_t*)SelectedBlock->Ptr) + (SelectedBlock->BEEFPOINT - sizeof(uint32_t));
	*((uint32_t*)BasePtr) = 0xDEADBEEF;
	
	SelectedBlock->RefPtr = RefPtr;
#endif

	// Block-useable
	memset(RefPtr, 0x00, SelectedBlock->OrigSize);
	
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
	
	//printf("%p %p\n", SelectedBlock->Ptr, RefPtr);
	
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
	FixedSize += sizeof(uint32_t);
	
	// Debug Marks
#if defined(_DEBUG)
	FixedSize += sizeof(uint32_t) << 1;
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
	uint32_t* Scanner = NULL;
	uint32_t BlockRef = 0;
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
	int32_t i;
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
		BasedStart = (uint8_t*)SelectedBlock->Ptr - (uint8_t*)SelectedZone->Ptr;
		BasedOffset = BasedStart + SelectedBlock->Size;
		
		// If Z_CorrectPartitions sorts and merges partitions and we run it anyway,
		// wouldn't it be better to just make a new partition?
		i = SelectedZone->UsedPartitions++;
		SelectedZone->PartitionList[i].Size = SelectedBlock->Size;
		SelectedZone->PartitionList[i].Sides[0].Ptr = SelectedBlock->Ptr;
		SelectedZone->PartitionList[i].Sides[0].Offset = BasedStart;
		SelectedZone->PartitionList[i].Sides[1].Ptr = (uint8_t*)SelectedBlock->Ptr + SelectedBlock->Size;
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
		if (*((uint32_t*)(SelectedBlock->Ptr)) != 0xDEADBEEF || *((uint32_t*)((void*)SelectedBlock->Ptr + (SelectedBlock->BEEFPOINT - sizeof(uint32_t)))) != 0xDEADBEEF)
		{
			CONS_Printf("Z_Free: %s%s %hs:%i (%i) / %hs:%i (%i) [%08x %08x]\n",
					(*((uint32_t*)(SelectedBlock->Ptr)) != 0xDEADBEEF ? "under" : ""),
					(*((uint32_t*)((void*)SelectedBlock->Ptr + (SelectedBlock->BEEFPOINT - sizeof(uint32_t)))) != 0xDEADBEEF ? "over" : ""),
					
					// Malloc info
					SelectedBlock->DebugInfo[0].File,
					SelectedBlock->DebugInfo[0].Line,
					SelectedBlock->DebugInfo[0].Count,
					
					// Free info
					SelectedBlock->DebugInfo[1].File,
					SelectedBlock->DebugInfo[1].Line,
					SelectedBlock->DebugInfo[1].Count,
					
					*((uint32_t*)(SelectedBlock->Ptr)),
					*((uint32_t*)((void*)SelectedBlock->Ptr + (SelectedBlock->BEEFPOINT - sizeof(uint32_t))))
				);
		}
#endif

		// Clear
		memset(SelectedBlock->Ptr, 0, SelectedBlock->Size);
		
		// Clean Block up
		SelectedBlock->Ptr = NULL;
		SelectedBlock->Size = 0;
		SelectedBlock->OrigSize = 0;
		SelectedBlock->Tag = 0;
		SelectedBlock->DiffPos = 0;
		SelectedBlock->Ref = 0;

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
#if defined(_DEBUG)
	else
	{
		CONS_Printf("Z_InternalFree: Passed bogus pointer %p.\n", Ptr);
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
		if (!(Ptr >= Rover->Ptr && Ptr < (void*)(((uint8_t*)Rover->Ptr) + Rover->Size)))
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
					Z_InternalFree(Rover, (uint8_t*)Rover->BlockList[i].Ptr + (sizeof(uint32_t)
#if defined(_DEBUG)
							<< 1), File, Line
#else
							)
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
		if (!(Ptr >= Rover->Ptr && Ptr < (void*)(((uint8_t*)Rover->Ptr) + Rover->Size)))
			continue;
		
		// Find block
		SelectedBlock = Z_BlockForPtr(Rover, Ptr);
		
		if (SelectedBlock)
			// Mess around
			SelectedBlock->Tag = (Tag > PU_FREE ? Tag : PU_STATIC);
		
		break;	// Neat
	}
}

/* Z_DebugMarkBlock() -- Mark a block for debug */
void Z_DebugMarkBlock(void* const Ptr, const char* const String)
{
#if defined(_DEBUG)
	Z_MemoryZoneArea_t* Rover = NULL;
	Z_MemoryBlock_t* SelectedBlock = NULL;
	
	/* Check */
	// No pointer or string?
	if (!Ptr || !String)
		return;
	
	/* Look for zone containing ptr */
	// Rove list
	for (Rover = l_HeadZone; Rover; Rover = Rover->Next)
	{
		/* Not in this zone? */
		if (!(Ptr >= Rover->Ptr && Ptr < (void*)(((uint8_t*)Rover->Ptr) + Rover->Size)))
			continue;
		
		// Find block
		SelectedBlock = Z_BlockForPtr(Rover, Ptr);
		
		if (SelectedBlock)
			// Mess around
			strncpy(SelectedBlock->DebugMark, String, 128);
		
		break;	// Neat
	}
#endif
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
				
				if (*((uint32_t*)(SelectedBlock->Ptr)) != 0xDEADBEEF || *((uint32_t*)((void*)SelectedBlock->Ptr + (SelectedBlock->BEEFPOINT - sizeof(uint32_t)))) != 0xDEADBEEF)
				{
					CONS_Printf("Z_Free: %s%s %hs:%i (%i) / %hs:%i (%i) [%08x %08x]\n",
							(*((uint32_t*)(SelectedBlock->Ptr)) != 0xDEADBEEF ? "under" : ""),
							(*((uint32_t*)((void*)SelectedBlock->Ptr + (SelectedBlock->BEEFPOINT - sizeof(uint32_t)))) != 0xDEADBEEF ? "over" : ""),
					
							// Malloc info
							SelectedBlock->DebugInfo[0].File,
							SelectedBlock->DebugInfo[0].Line,
							SelectedBlock->DebugInfo[0].Count,
					
							// Free info
							SelectedBlock->DebugInfo[1].File,
							SelectedBlock->DebugInfo[1].Line,
							SelectedBlock->DebugInfo[1].Count,
					
							*((uint32_t*)(SelectedBlock->Ptr)),
							*((uint32_t*)((void*)SelectedBlock->Ptr + (SelectedBlock->BEEFPOINT - sizeof(uint32_t))))
						);
				}
			}
#endif
}

#else

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
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2010 The ReMooD Team.
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

#define CAPNUMBER ((uint32_t)0xDEADBEEF)
//#define SEGMENTSIZE (1 << 11)					// Lowest possible is (1 << 3) = 8
//#define SUBSEGMENTSIZE (SEGMENTSIZE >> 3)		// SEGMENTSIZE >> 3

#define DEFAULT_SEGMENTSIZE (1 << 11)
#define DEFAULT_SUBSEGMENTSIZE (DEFAULT_SEGMENTSIZE >> 3)

size_t SEGMENTSIZE = DEFAULT_SEGMENTSIZE;
size_t SUBSEGMENTSIZE = DEFAULT_SUBSEGMENTSIZE;

// 60 bytes w/ DEBUG + 64-bit
typedef struct MemBlock_s
{
	int8_t Tag;
	size_t Size;
	void* Ptr;
	void** User;
	
	// Segment Links (for free speed)
	size_t StartSeg;
	size_t EndSeg;
	uint8_t StartSub;
	uint8_t EndSub;

#ifdef ZDEBUG
	char File[32];
	uint32_t Line;
	
	uint32_t* StartCap;
	uint32_t* EndCap;
#endif
} MemBlock_t;

void* MemoryZone = NULL;
size_t AllocatedMemory = 0;
MemBlock_t* StartBlock = NULL;
size_t NumBlocks = 0;
uint8_t* SegmentMap = NULL;
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
	realsize = realsize + (sizeof(uint32_t) * 2);
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
	Rover->EndCap = (uint8_t*)CreationSpot + (realsize - sizeof(uint32_t));
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
	return (uint8_t*)CreationSpot + sizeof(uint32_t);
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
	if (Block && Block >= MemoryZone && Block <= ((uint8_t*)MemoryZone + AllocatedMemory))
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
		ptr = ((size_t)ptr) - sizeof(uint32_t);
#endif
		
		/* Get Estimated Segment based on Pointer */
		// CreationSpot = ((uint8_t*)MemoryZone) + (StartSeg * SEGMENTSIZE) + (StartSub * SUBSEGMENTSIZE);
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
	ptr = ((size_t)ptr) - sizeof(uint32_t);
#endif
		
	/* Get Estimated Segment based on Pointer */
	// CreationSpot = ((uint8_t*)MemoryZone) + (StartSeg * SEGMENTSIZE) + (StartSub * SUBSEGMENTSIZE);
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
	
	ptr = ((size_t)ptr) - sizeof(uint32_t);
		
	/* Get Estimated Segment based on Pointer */
	// CreationSpot = ((uint8_t*)MemoryZone) + (StartSeg * SEGMENTSIZE) + (StartSub * SUBSEGMENTSIZE);
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


#endif

