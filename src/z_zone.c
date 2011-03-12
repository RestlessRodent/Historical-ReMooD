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
// DESCRIPTION: New Memory Manager Written by GhostlyDeath

/***************
*** INCLUDES ***
***************/

#include <stdlib.h>
#include <string.h>
#include "z_zone.h"

/****************
*** CONSTANTS ***
****************/

#define ALIGNSIZE	8										// Size alignment
#define ALIGNBLOCK	((sizeof(Z_MemBlock_t) + (ALIGNSIZE - 1)) & ~(ALIGNSIZE - 1))	// Block alignment
#define ROOTID		0xDEADCAFEU								// Root ID
#define BLOCKID		0x7BEEEFE7U								// ID for blocks
#define BADBLOCK	0xDEADBEEFU								// Bad block

/*****************
*** STRUCTURES ***
*****************/

/* Z_MemBlock_t -- Memory block */
typedef struct Z_MemBlock_s
{
	/* Guard */
	uint32_t BlockIDStart;									// ID of start
	
	/* Normals */
	Z_MemoryTag_t Tag;										// Block tag
	void* Ptr;												// Pointer to data
	void** Ref;												// Reference
	size_t BlockSize;										// Size of the block related data
	size_t Size;											// Size of the data
	struct Z_MemBlock_s* Prev;								// Previous block
	struct Z_MemBlock_s* Next;								// Next block
	
	/* Guard */
	uint32_t BlockIDEnd;									// ID of end
} Z_MemBlock_t;

/* Z_RootMemChunk_t -- Root memory chunk */
typedef struct Z_RootMemChunk_s
{
	/* Guard */
	uint32_t RootIDStart;									// ID of start
	
	/* Normals */
	size_t TotalSize;										// Total memory available
	Z_MemBlock_t* Rover;									// Block rover
	Z_MemBlock_t* Head;										// Head block
	Z_MemBlock_t* Tail;										// Tail block
	
	/* Gaurd */
	uint32_t RootIDEnd;										// ID of end
} Z_RootMemChunk_t;

/*************
*** LOCALS ***
*************/

static void* l_CoreMemory = NULL;							// Local memory

/****************
*** FUNCTIONS ***
****************/

/* Z_AlignPlacePointer() -- Gives a nice pointer after n bytes */
static void* Z_AlignPlacePointer(void* const Ptr, const size_t Bytes)
{
	uintptr_t Base = (uintptr_t)Ptr;
	
	/* Move base ahead */
	Base += Bytes;
	
	/* Add alignment size */
	Base += ALIGNSIZE - 1;
	
	/* Clip base */
	Base &= ~(ALIGNSIZE - 1);
	
	/* Return base */
	return (void*)Base;
}

/* Z_Init() -- Initializes the memory manager */
void Z_Init(void)
{
	size_t l_CoreSize = 0;
	Z_RootMemChunk_t* Root = NULL;
	
	/* Determine core size */
	l_CoreSize = 64 << 20;			// Default 4 MiB
	
	/* Allocate core */
	l_CoreMemory = malloc(l_CoreSize);
	
	if (!l_CoreMemory)
	{
		I_Error("Z_Init: Failed to allocate core zone\n");
		return;
	}
	
	/* Initialize root */
	memset(l_CoreMemory, 0, l_CoreSize);
	Root = (Z_RootMemChunk_t*)l_CoreMemory;
	
	// Fields
	Root->RootIDStart = Root->RootIDEnd = ROOTID;
	Root->TotalSize = l_CoreSize;
	
	// First free block
	Root->Rover = Root->Head = Root->Tail = Z_AlignPlacePointer(Root, sizeof(Z_RootMemChunk_t));
	
	/* Initialize first block */
	Root->Head->BlockIDStart = Root->Head->BlockIDEnd = BLOCKID;
	Root->Head->BlockSize = l_CoreSize - ((uintptr_t)Root->Head - (uintptr_t)Root);
	Root->Head->Prev = Root->Head->Next = Root->Head;
}

/* Z_CheckBlockOK() -- Check to see if a block is OK */
static boolean Z_CheckBlockOK(Z_MemBlock_t* const Block)
{
	/* Non-fatal */
	// Bad pointer (oops!)
	if (!Block)
		return false;
	
	/* Fatal */
	// Start ID does not match end ID
	if (Block->BlockIDStart != Block->BlockIDEnd)
	{
		I_Error("Z_CheckBlockOK: Start != End");
		return false;
	}
	
	// Start ID is not BLOCKID
	if (Block->BlockIDStart != BLOCKID)
	{
		I_Error("Z_CheckBlockOK: Start != BLOCKID");
		return false;
	}
	
	/* All OK */
	return true;
}

/* Z_FreeBlock() -- Frees a specific block */
static void Z_FreeBlock(Z_MemBlock_t* const Block)
{
	Z_RootMemChunk_t* Root = NULL;
	Z_MemBlock_t* BackBlock = NULL;
	Z_MemBlock_t* PurgeBlock = NULL;
	
	/* Check */
	if (!Block)
		return;
	
	/* Valid block */
	if (!Z_CheckBlockOK(Block))
		return;
	
	/* Obtain root */
	Root = (Z_RootMemChunk_t*)l_CoreMemory;
	
	/* Clear block info */
	// Deref
	if (Block->Ref)
		*Block->Ref = NULL;
	
	// Clear others
	Block->Size = 0;
	Block->Ptr = NULL;
	Block->Ref = NULL;
	Block->Tag = 0;
	
	/* Forward merging (ONLY IF WE ARE NOT TAIL!) */
	// Don't merge if head is the tail
	if (Root->Tail != Root->Head && !Block->Next->Ptr && Block != Root->Tail)
	{
		// If the tail is the block we merge with set tail to this
		if (Root->Tail == Block->Next)
			Root->Tail = Block;
		
		// Merge
		PurgeBlock = Block->Next;
		Block->BlockSize += PurgeBlock->BlockSize;
		Block->Next = PurgeBlock->Next;
		Block->Next->Prev = Block;
		
		// Wipe any trace of block
		memset(PurgeBlock, 0, sizeof(*PurgeBlock));
		PurgeBlock->BlockIDStart = PurgeBlock->BlockIDEnd = BADBLOCK;
		
		// Is rover the merged block?
		if (Root->Rover == PurgeBlock)
			Root->Rover = Block;
	}
	
	/* Backward merging (ONLY IF WE ARE NOT HEAD!) */
	// Don't merge if head is the tail
	if (Root->Tail != Root->Head && !Block->Prev->Ptr && Block != Root->Head)
	{
		BackBlock = Block->Prev;
		
		// We don't have to worry about head in this case since we backstep
	
		// Merge
		PurgeBlock = BackBlock->Next;
		
		BackBlock->BlockSize += PurgeBlock->BlockSize;
		BackBlock->Next = PurgeBlock->Next;
		BackBlock->Next->Prev = BackBlock;
		
		// Wipe any trace of block
		memset(PurgeBlock, 0, sizeof(*PurgeBlock));
		PurgeBlock->BlockIDStart = PurgeBlock->BlockIDEnd = BADBLOCK;
		
		// Is rover the merged block?
		if (Root->Rover == PurgeBlock)
			Root->Rover = BackBlock;
	}
}

/* Z_MallocWrappee() -- Allocate memory */
void* Z_MallocWrappee(const size_t Size, const Z_MemoryTag_t Tag, void** const Ref _ZMGD_WRAPPEE)
{
	size_t GoodSize;
	void* PlacePtr;
	Z_MemBlock_t* Start;
	Z_MemBlock_t* Next;
	Z_RootMemChunk_t* Root = NULL;
	int32_t i;
	
	/* Obtain root */
	Root = (Z_RootMemChunk_t*)l_CoreMemory;
	
	/* Calculate the good size */
	// The good size of the size of the block plus size of the data we want
	GoodSize = ((sizeof(Z_MemBlock_t) + (Size ? Size : 1)) + (ALIGNSIZE - 1)) & ~(ALIGNSIZE - 1);
	//GoodSize = ((sizeof(Z_MemBlock_t) + (ALIGNSIZE - 1)) & ~(ALIGNSIZE - 1)) + 
		//((Size + (ALIGNSIZE - 1)) & ~(ALIGNSIZE - 1));
	
	/* Rove list for suitable find */
	for (i = 0; i < 5; i++, Root->Rover = Root->Rover->Next)
	{
		// Prepare
		Start = Root->Rover->Prev;
		//Root->Rover = Root->Rover->Next;
		
		// NULL-ify Start if it's Root->Rover or the next after
		if (Start == Start->Next || Start == Root->Rover->Next || Start == Root->Rover->Next->Next)
			Start = NULL;
		
		// Only make a single loop around and allocate if we can
		for (; Root->Rover != Start; Root->Rover = Root->Rover->Next)
		{
			// Valid block?
			if (!Z_CheckBlockOK(Root->Rover))
				continue;
		
			// Is this a cached block?
			if (Root->Rover->Tag >= PU_PURGELEVEL)
			{
				// Free the block if it is so., Rover will be renormalized
				Z_FreeBlock(Root->Rover);
			}
		
			// Is this block actually free?
			if (Root->Rover->Ptr)
				continue;
		
			// Is there enough space in this block to allocate this and the next block?
			if (GoodSize > Root->Rover->BlockSize - ALIGNBLOCK)
				continue;
		
			// If we are good, place the pointer here and set next after
			PlacePtr = Z_AlignPlacePointer(Root->Rover, sizeof(Z_MemBlock_t));
			Next = Z_AlignPlacePointer(Root->Rover, GoodSize);
		
			// Relink now and next
			Next->Next = Root->Rover->Next;		// Place between old next [a new b]
			Root->Rover->Next = Next;			// Rover next is now new
			Next->Prev = Root->Rover;			// the block before new block is the rover
		
			// If the tail is rover, change it so next is the tail
			if (Root->Tail == Root->Rover)
				Root->Tail = Next;
		
			// Initialize rover and next to match allocation
			Next->BlockSize = Root->Rover->BlockSize - GoodSize;	// next is shrunken this
			Root->Rover->BlockSize = GoodSize;
		
			Root->Rover->Ptr = PlacePtr;
			Root->Rover->Tag = Tag;
			Root->Rover->Ref = Ref;
			Root->Rover->Size = Size;		// includes size
		
			Next->BlockIDStart = Next->BlockIDEnd = BLOCKID;
			Next->Ptr = NULL;
			Next->Ref = NULL;
			Next->Size = 0;
			Next->Tag = 0;
		
			// Clear memory
			memset(Root->Rover->Ptr, 0, Root->Rover->Size);
		
			// Place rover at the next block
			Root->Rover = Next;
		
			// Return pointer
			return PlacePtr;
		}
		
		// If we reached this point, allocation failed
		Z_FreeTags(PU_PURGELEVEL, NUMZTAGS);	// Try and free some tags
	}
	
	// Error
	I_Error("Z_Malloc: Nore more space left to allocate inside!\n");
	
	return (void*)0xDEADBEEFL;	// Segfault, hopefully!
}

/* Z_GetBlockFromPtr() -- Gets the assigned block from a Ptr */
static Z_MemBlock_t* Z_GetBlockFromPtr(void* const Ptr)
{
	Z_MemBlock_t* Block;
	
	/* Check */
	if (!Ptr)
		return NULL;
	
	/* Do some math */
	Block = (Z_MemBlock_t*)((uintptr_t)Ptr - (uintptr_t)Z_AlignPlacePointer(NULL, sizeof(Z_MemBlock_t)));
	
	/* Are we insane? */
	if (Block->Ptr == Ptr)
		return Block;
	
	// we are
	return NULL;
}

/* Z_FreeWrappee() -- Free memory */
void Z_FreeWrappee(void* const Ptr _ZMGD_WRAPPEE)
{
	Z_MemBlock_t* Block = NULL;
	
	/* Get Block */
	Block = Z_GetBlockFromPtr(Ptr);
	
	// Check
	if (!Block)
		return;
	
	/* Match? */
	Z_FreeBlock(Block);
}

/* Z_ChangeTag() -- Change a pointer's tag */
void Z_ChangeTag(void* const Ptr, const Z_MemoryTag_t NewTag)
{
	Z_MemBlock_t* Block = NULL;
	Z_MemoryTag_t OldTag;
	
	/* Get Block */
	Block = Z_GetBlockFromPtr(Ptr);
	
	// Check
	if (!Block)
		return ((Z_MemoryTag_t)-1);
	
	/* Match? */
	OldTag = Block->Tag;
	Block->Tag = NewTag;
	return OldTag;
}

/* Z_GetTagFromPtr() -- Return the memory tag associated with a pointer */
Z_MemoryTag_t Z_GetTagFromPtr(void* const Ptr)
{
	Z_MemBlock_t* Block = NULL;
	
	/* Get Block */
	Block = Z_GetBlockFromPtr(Ptr);
	
	// Check
	if (!Block)
		return ((Z_MemoryTag_t)-1);
		
	/* Match? */
	return Block->Tag;
}

/* Z_DebugMarkBlock() -- Not implemented */
void Z_DebugMarkBlock(void* const Ptr, const char* const String)
{
}

/* Z_FreeTagsWrappee() -- Clear tags from low to high */
size_t Z_FreeTagsWrappee(const Z_MemoryTag_t LowTag, const Z_MemoryTag_t HighTag _ZMGD_WRAPPEE)
{
	size_t Count;
	Z_MemBlock_t* Start;
	Z_RootMemChunk_t* Root = NULL;
	
	/* Obtain root */
	Root = (Z_RootMemChunk_t*)l_CoreMemory;
	
	/* Free loop */
	// Prepare
	Start = Root->Rover;
	Root->Rover = Root->Rover->Next;
	
	// Only make a single loop around and clear what we can kinda
	for (; Root->Rover != Start; Root->Rover = Root->Rover->Next)
	{
		// Valid block?
		if (!Z_CheckBlockOK(Root->Rover))
			continue;
		
		// Is this a cached block?
		if (Root->Rover->Tag >= LowTag && Root->Rover->Tag < HighTag)
		{
			// Free the block if it is so., Rover will be renormalized
			Z_FreeBlock(Root->Rover);
			Count++;
		}
	}
	
	return Count;
}

/* Z_CheckHeap() -- Not implemented */
void Z_CheckHeap(const int Code)
{
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

/* Z_StrDup() -- Duplicate a string */
char* Z_Strdup(const char* const String, const Z_MemoryTag_t Tag, void** Ref)
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

/* Z_StrDupW() -- Duplicate a wide string */
wchar_t* Z_StrdupW(const wchar_t* const WString, const Z_MemoryTag_t Tag, void** Ref)
{
	size_t n;
	wchar_t* Ptr;

	/* Check */
	if (!WString)
		return NULL;

	/* Copy */
	n = wcslen(WString);
	Ptr = Z_Malloc(sizeof(wchar_t) * (n + 1), Tag, Ref);
	wcscpy(Ptr, WString);

	/* Return */
	return Ptr;
}

