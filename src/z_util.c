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
// Copyright (C) 2011-2013 GhostlyDeath <ghostlydeath@remood.org>.
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
#if defined(__REMOOD_OLDHASHFUNC)
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
#else
	register int i;
	
	uint32_t Val;
	uint32_t Ret = 0;
	static const uint32_t c_Primes[16] =
	{
#if 1
		0, 1, 2, 3, 5, 7, 11, 13, 17, 19,	// Base
		17, 13, 11, 7, 5, 3 // For Masking
#else
		0, 1, 2, 3, 5, 7, 11, 13, 17, 19, 29, 31,	// Base
		29, 19, 17, 13,	// For Masking
#endif
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
		// Current Value (Remove caps, hopefully)
#if 1
		Val = toupper(a_Str[i]) & 0x7F;
		
		if ((i & 1) == 0)
			Val = ~Val;
#else
		Val = /*toupper*/(a_Str[i]) & (~0x20);
#endif
		
		// Modify return value
		Ret ^= (Val << c_Primes[i & 15]);// | (Val << c_Primes[31 - (Val & 7)]);
	}
		
	/* Return */
	return Ret;
#endif
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
	int32_t IntValue;			// Integer Value
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

/* Z_TableGetValueInt() -- Returns pre-determined key integer value */
int32_t Z_TableGetValueInt(Z_Table_t* const a_Table, const char* const a_SubKey, bool_t* const a_Found)
{
	Z_TableEntry_t* Entry;
	
	/* Clear Found */
	if (a_Found)
		*a_Found = false;
	
	/* Check */
	if (!a_Table || !a_SubKey)
		return 0;
		
	/* Find entry */
	Entry = Z_FindTableEntry(a_Table, a_SubKey, false, NULL);
	
	// If we found the entry it must be an entry
	if (Entry)
		if (!Entry->IsTable)
		{
			// Set Found
			if (a_Found)
				*a_Found = true;
			
			// Get Value
			return Entry->Data.Index.IntValue;
		}
			
	/* Was either not found or was a table */
	return 0;
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
		Entry->Data.Index.IntValue = C_strtoi32(Entry->Data.Index.Value, NULL, 0);
		
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

/* Z_TableStoreToStream() -- Dumps table to stream */
void Z_TableStoreToStream(Z_Table_t* const a_Table, struct D_BS_s* const a_Stream)
{
	size_t i;
	Z_TableEntry_t* ThisEnt;
	
	/* Check */
	if (!a_Table || !a_Stream)
		return;
	
	/* Write Table Marker Header */
	// Write Table Key
	D_BSBaseBlock(a_Stream, "TABL");
	D_BSws(a_Stream, a_Table->Key);
	D_BSRecordBlock(a_Stream);
	
	// Go through all entries
	for (i = 0; i < a_Table->NumEntries; i++)
	{
		// Get
		ThisEnt = a_Table->Entries[i];
		if (!ThisEnt)
			continue;
		
		// Is a table?
		if (ThisEnt->IsTable)	// Recourse
			Z_TableStoreToStream(ThisEnt->Data.TableLink, a_Stream);
		
		// Is normal entry
		else
		{
			// Begin Entry Information
			D_BSBaseBlock(a_Stream, "DATA");
			
			// Write it out
			D_BSws(a_Stream, ThisEnt->Data.Index.SubKey);
			D_BSws(a_Stream, ThisEnt->Data.Index.Value);
			
			// End
			D_BSRecordBlock(a_Stream);
		}
	}
	
	/* End Table */
	D_BSBaseBlock(a_Stream, "ENDT");
	D_BSRecordBlock(a_Stream);
}

/* Z_TableStreamToStore() -- Restores table that was previously dumped */
Z_Table_t* Z_TableStreamToStore(struct D_BS_s* const a_Stream)
{
#define BUFSIZE 256
	char Header[5];
	char Buf[BUFSIZE];
	char Buf2[BUFSIZE];
	Z_Table_t* RootTable, *CurrentTable;
	int32_t Depth;
	
	/* Check */
	if (!a_Stream)
		return NULL;
	
	/* Hopefully read table header */
	// Init
	Depth = 0;
	memset(Header, 0, sizeof(Header));
	RootTable = CurrentTable = NULL;
	
	// Constant Read
	while (D_BSPlayBlock(a_Stream, Header))
	{
		// End of Table?
		if (D_BSCompareHeader("ENDT", Header))
		{
			// Go back up
			Depth--;
			if (CurrentTable)
				CurrentTable = CurrentTable->ParentTable;
			
			// Break out?
			if (Depth <= 0)
				break;
		}
		
		// Start of Table?
		else if (D_BSCompareHeader("TABL", Header))
		{
			// Read Table Key
			//memset(Buf, 0, sizeof(Buf));
			D_BSrs(a_Stream, Buf, BUFSIZE - 1);
			
			// Creating root table?
			if (!RootTable)
				RootTable = CurrentTable = Z_TableCreate(Buf);
			
			// Sub-Table
			else
				// Find the sub table and set current
				CurrentTable = Z_FindSubTable(CurrentTable, Buf, true);
			
			// Increase table depth
			Depth++;
		}
		
		// Single Entry?
		else if (D_BSCompareHeader("DATA", Header))
		{
			// Clear, just in case
			//memset(Buf, 0, sizeof(Buf));
			//memset(Buf2, 0, sizeof(Buf2));
			
			// Read strings
			D_BSrs(a_Stream, Buf, BUFSIZE);	// Key
			D_BSrs(a_Stream, Buf2, BUFSIZE);	// Value
			
			// Create sub entry
			Z_TableSetValue(CurrentTable, Buf, Buf2);
		}
	}
	
	/* Return the root table */
	return RootTable;
	
	//Z_Table_t* Z_FindSubTable(Z_Table_t* const a_Table, const char* const a_Key, const bool_t a_Create);
	//bool_t Z_TableSetValue(Z_Table_t* const a_Table, const char* const a_SubKey, const char* const a_NewValue);
#undef BUFSIZE
}

