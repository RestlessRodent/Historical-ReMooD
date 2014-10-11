// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Malloc Hash Chaining

/***************
*** INCLUDES ***
***************/

#include "z_zone.h"
#include "i_system.h"
#include "i_util.h"
#include "console.h"

/****************
*** CONSTANTS ***
****************/

#define ZSTART			UINT32_C(0xDEADBEEF)	// Start of Block
#define ZEND			UINT32_C(0xCAFEBABE)	// End of Block

#define ZMBSTART		UINT32_C(0x1337D00D)
#define ZMBEND			UINT32_C(0x5CA1AB1E)

#define ZALIGNMENT						16		// Align to 
#define ZBLOCKBASE	((sizeof(Z_MHCInfo_t) + ZALIGNMENT) & ~(ZALIGNMENT - 1))
#define ZMINIBASE	((sizeof(Z_MHCMiniBlock_t) + ZALIGNMENT) & ~(ZALIGNMENT - 1))

#define ZMINIALLOCMAX	UINT32(32768)			// Max allocation in mini block
#define ZMINIBLOCKSZ	UINT32(65536)			// Block size of mini block

/*****************
*** STRUCTURES ***
*****************/

struct Z_MHCInfo_s;

/* Z_MHCMiniBlock_t -- Mini block for mini allocations */
// This will reduce the number of mallocs(), but add slightly more overhead but
// it will keep small blocks together. Probably might be seen as pre-mature
// optimization as some operating systems might be smart about allocating small
// bits of memory. However some other systems might just allocate in chunks of
// the min page size and just waste away.
typedef struct Z_MHCMiniBlock_s
{
	uint32_t Start;								// Start of mini block
	
	struct Z_MHCMiniBlock_s* Prev;				// Previous mini block (another chunk)
	struct Z_MHCMiniBlock_s* Next;				// Next mini block
	
	struct Z_MHCInfo_t* Info;					// First in info
	uint32_t AllocSize;							// Allocation size
	uint32_t FreeSize;							// Free space inside
	
	uint32_t End;								// End of mini block
} Z_MHCMiniBlock_t;

/* Z_MHCInfo_t -- Malloc Chain Info */
typedef struct Z_MHCInfo_s
{
	uint32_t Start;								// Marker Start
	
	size_t Size;								// Allocation Size
	Z_MemoryTag_t Tag;							// Allocation Tag
	void** RefPtr;								// Reference
	
	Z_MHCMiniBlock_t* MiniBlock;				// Mini block if inside one
	struct Z_MHCInfo_s* Prev;					// Previous
	struct Z_MHCInfo_s* Next;					// Next
	
#if defined(_DEBUG)
	const char* File;							// File used by
	int Line;									// Line location
#endif
	
	uint32_t End;								// Marker End
} Z_MHCInfo_t;

/*************
*** LOCALS ***
*************/

static Z_MHCMiniBlock_t* l_ZMinis[NUMZTAGS + 1];
static Z_MHCInfo_t* l_ZChains[NUMZTAGS + 1];
static uint64_t l_ZAllocSize[2] = {0, 0};

/****************
*** FUNCTIONS ***
****************/

/* Z_MHC_Init() -- Chain initialization */
void Z_MHC_Init(void)
{
	/* Clear chain list */
	memset(l_ZChains, 0, sizeof(l_ZChains));
	memset(l_ZMinis, 0, sizeof(l_ZMinis));
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
		
		// Add to allocation size
		l_ZAllocSize[0] += a_Size;
		l_ZAllocSize[1] += ZBLOCKBASE + a_Size;
		
		// Worked! Clear Block
		memset(AllocBlock, 0, ZBLOCKBASE + a_Size);
		
		// Set Info
		AllocBlock->Start = ZSTART;
		AllocBlock->End = ZEND;
		
		AllocBlock->Size = a_Size;
		
		if (a_Tag < PU_STATIC)	
			AllocBlock->Tag = PU_STATIC;
		else if (a_Tag >= NUMZTAGS)
			AllocBlock->Tag = NUMZTAGS - 1;
		else
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
		
		// File and line
#if defined(_DEBUG)
		AllocBlock->File = File;
		AllocBlock->Line = Line;
#endif
		
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
	
	/* Remove allocation size */
	l_ZAllocSize[0] -= AllocBlock->Size;
	l_ZAllocSize[1] -= ZBLOCKBASE + AllocBlock->Size;
	
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

#if defined(_DEBUG)
/* Z_MHZDbgRef_t -- Debug reference */
typedef struct Z_MHZDbgRef_s
{
	uint32_t Hash;								// Hash or zero
	const char* File;							// Filename
	uint32_t Blocks;							// Owned blocks
	uint32_t Bytes;								// Bytes allocated
	uint32_t Cache;								// Cached bytes used
	uint32_t EstWaste;							// Estimated wasted space
} Z_MHZDbgRef_t;
#endif

/* Z_MHC_MemUsage() -- Memory usage */
static int Z_MHC_MemUsage(const uint32_t a_ArgC, const char** const a_ArgV)
{
#if defined(_DEBUG)
#define MAXCOUNT 256
	int32_t i, j, k;
	Z_MHCInfo_t* Alloc;
	uint32_t HashF;
	Z_MHZDbgRef_t Refs[MAXCOUNT];
	Z_MHZDbgRef_t* Ref, Temp;
	const char* FName;
	
	uint32_t Bu[4];
	char Bc[4];
#endif
	
	/* Totals */
	CONL_PrintF("Total: %u KiB\n", l_ZAllocSize[1] / 1024);
	CONL_PrintF("Avail: %u KiB\n", l_ZAllocSize[0] / 1024);
	CONL_PrintF("Waste: %u KiB\n", (l_ZAllocSize[1] - l_ZAllocSize[0]) / 1024);

	/* Deep memory debug */
#if defined(_DEBUG)
	// Clear references
	memset(Refs, 0, sizeof(Refs));	
	
	// Need to go through each tag, being unique
	for (i = 0; i < NUMZTAGS; i++)
		for (Alloc = l_ZChains[i]; Alloc; Alloc = Alloc->Next)
		{
			// Find reference from file
			if (Alloc->File)
			{
				// Hashfile name
				HashF = Z_Hash(Alloc->File);
			
				// See if it exists in refs already
				for (j = 1; j < MAXCOUNT; j++)
					if ((Ref = &Refs[j])->Hash == HashF)
						break;
			
				// Does not
				if (j >= MAXCOUNT)
				{
					// Try to find a free spot
					for (j = 1; j < MAXCOUNT; j++)
						if (!Refs[j].Hash)
						{
							Ref = &Refs[j];
							Ref->Hash = HashF;
							Ref->File = Alloc->File;
							break;
						}
				
					// Nothing
					if (j >= MAXCOUNT)
						continue;
				}
			}
			
			// Unknown (no file passed!?!?)
			else
				Ref = &Refs[0];
			
			// Increase reference totals
			Ref->Blocks++;
			
			if (Alloc->Tag >= PU_PURGELEVEL)
				Ref->Cache += Alloc->Size;
			else
				Ref->Bytes += Alloc->Size;
			
			// Add wastage of space
			Ref->EstWaste += ZBLOCKBASE;
		}
	
	/* Sort by total */
	for (i = 1; i < MAXCOUNT; i++)
	{
		// Find highest
		for (k = i, j = i; j < MAXCOUNT; j++)
			if ((Refs[j].Bytes + Refs[j].Cache) > (Refs[k].Bytes + Refs[k].Cache))
				k = j;
		
		// Can swap
		if (k != i)
		{
			Temp = Refs[k];
			Refs[k] = Refs[i];
			Refs[i] = Temp;
		}
	}
	
	/* Print into on all references */
	CONL_PrintF("%-13s: %-5s|%-5s?|%-5s?|%-5s?|%-5s?\n",
			"File", "Blcks", "Used", "Cache", "Total", "Waste"
		);
	for (j = 0; j < MAXCOUNT; j++)
	{
		Ref = &Refs[j];
		
		if (!Ref->Hash && j != 0)
			continue;
		
		// Get last characters of file
		FName = (j == 0 ? "Unknown" : Ref->File);
		
		// If too long, reduce characters (so it fits better)
		// And removes any useless prefix like src/ or C:\Long Directory\...
		i = strlen(FName);
		if (i >= 12)
			FName += i - 12;
			
		// Shorten for Ks
		Bu[0] = Ref->Bytes;
		Bu[1] = Ref->Cache;
		Bu[2] = Bu[0] + Bu[1];
		Bu[3] = Ref->EstWaste;
		
		// Init letters to c
		for (i = 0; i < 4; i++)
			Bc[i] = 'B';
		
		// Go through, and lower
		for (i = 0; i < 4; i++)
			while (Bu[i] >= 10240)
			{
				// Decrease amount
				Bu[i] /= 1024;
				
				// And increase the intensity
				switch (Bc[i])
				{
					case 'B': Bc[i] = 'K'; break;
					case 'K': Bc[i] = 'M'; break;
					case 'M': Bc[i] = 'G'; break;
					case 'G': Bc[i] = 'T'; break;
					default: Bc[i] = 'P'; break;
				}
			}
		
		// A nice message
		CONL_PrintF("%-13s: %5u|%5u%c|%5u%c|%5u%c|%5u%c\n",
				FName,
				Ref->Blocks,
				Bu[0], Bc[0],
				Bu[1], Bc[1],
				Bu[2], Bc[2],
				Bu[3], Bc[3]
			);
	}
#undef MAXCOUNT
#endif

	/* Done */
	return 0;
}

/* Z_MHC_RegisterCommands() -- Regsters memory manager commands and variables */
void Z_MHC_RegisterCommands(void)
{
	CONL_AddCommand("memusage", Z_MHC_MemUsage);
}

