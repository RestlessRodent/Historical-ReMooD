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
// DESCRIPTION: New New Memory Manager Written by GhostlyDeath
//              Stolen from my proprietary game and stripped

/***************
*** INCLUDES ***
***************/

#include "z_zone.h"
#include "i_system.h"







/***********************
*** COMMON FUNCTIONS ***
***********************/

/* Z_ResizeArrayWrappee() -- Resizes an array */
void Z_ResizeArrayWrappee(void** const PtrPtr, const size_t ElemSize, const size_t OldSize, const size_t NewSize _ZMGD_WRAPPEE)
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
	Temp = Z_MallocExWrappee(ElemSize * NewSize, PU_STATIC, NULL, 0
#if defined(_DEBUG)
		, File, Line
#endif
		);
	
	/* If *PtrPtr is set then manage it */
	if (*PtrPtr)
	{
		// Copy file:line
#if defined(_DEBUG)
//		Z_DupFileLine(Temp, *PtrPtr);
#endif
		
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

/* Z_StrDupWrappee() -- Duplicate String */
char* Z_StrDupWrappee(const char* const String, const Z_MemoryTag_t Tag, void** Ref _ZMGD_WRAPPEE)
{
	size_t n;
	char* Ptr;
	
	/* Check */
	if (!String)
		return NULL;
		
	/* Copy */
	n = strlen(String);
#if defined(_DEBUG)
	Ptr = Z_MallocExWrappee(sizeof(char) * (n + 1), Tag, Ref, 0, File, -Line);
#else
	Ptr = Z_Malloc(sizeof(char) * (n + 1), Tag, Ref);
#endif
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
// The input string is case insensitive
uint32_t Z_Hash(const char* const a_Str)
{
	register int i;
	
	uint32_t Val;
	uint32_t Ret = 0;
	static const uint32_t c_Primes[16] =
	{
		0, 1, 2, 3, 5, 7, 11, 13, 17, 19,	// Base
		17, 13, 11, 7, 5, 3 // For Masking
	};
	
	/* Check */
	if (!a_Str)
		return 0;
		
	/* Hash loop */
	// A = 0x41 Z = 0x5A
	// a = 0x61 z = 0x7A
	// Diff is 32, or 0x20
	for (i = 0; a_Str[i]; i++)
	{
		Val = toupper(a_Str[i]) & 0x7F;
		
		if ((i & 1) == 0)
			Val = ~Val;
		
		// Modify return value
		Ret ^= (Val << c_Primes[i & 15]);// | (Val << c_Primes[31 - (Val & 7)]);
	}
		
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

