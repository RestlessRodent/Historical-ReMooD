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
// Copyright (C) 2008-2011 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION:
//      Handles WAD file header, directory, lump I/O.

// added for linux 19990220 by Kin
#ifdef LINUX
#define O_BINARY 0
#endif

#ifndef __APPLE_CC__
#ifndef FREEBSD
#include <malloc.h>
#endif
#endif
#include <fcntl.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include <string.h>
#include <errno.h>

#include "doomdef.h"
#include "doomtype.h"
#include "w_wad.h"
#include "z_zone.h"
#include "i_video.h"
#include "dehacked.h"
#include "r_defs.h"
#include "i_system.h"
#include "md5.h"
#include "v_video.h"

// WAD DATA
#include "m_argv.h"
#include "hu_stuff.h"
#include "d_info.h"

/************************
*** LITE WAD HANDLING ***
************************/

/*** CONSTANTS ***/
#if defined(_WIN32)
#define WLPATHDIRSEP ';'
#else
#define WLPATHDIRSEP ':'
#endif

#define MAXSEARCHBUFFER					32	// Max Paths to store

/*** STRUCTURES ***/
/* WL_PDC_t -- Private data creator */
typedef struct WL_PDC_s
{
	uint32_t Key;								// Key for this registrar
	uint8_t Order;								// Registration Order
	WL_PCCreatorFunc_t CreatorFunc;				// Function to create data
	WL_RemoveFunc_t RemoveFunc;					// Function to remove data
	
	bool_t Mark;								// Marker
} WL_PDC_t;

/* WL_OCCB_t -- Order Changed Callback */
typedef struct WL_OCCB_s
{
	WL_OrderCBFunc_t Func;						// Function to call
	uint8_t Order;								// Callback order
	
	/* Used as a linked list */
	struct WL_OCCB_s* Prev;						// Previous
	struct WL_OCCB_s* Next;						// Next
} WL_OCCB_t;

/*** LOCALS ***/
static WL_WADFile_t* l_LFirstWAD = NULL;		// First WAD in the chain
static WL_WADFile_t* l_LFirstVWAD = NULL;		// First Virtual WAD in the chain
static WL_WADFile_t* l_LLastVWAD = NULL;		// First Virtual WAD in the chain
static WL_PDC_t l_PDC[WLMAXPRIVATEWADSTUFF];	// Private WAD Stuff
static WL_PDC_t* l_PDCp[WLMAXPRIVATEWADSTUFF];	// Sorted variant
static size_t l_NumPDC = 0;						// Number of PDC
static WL_OCCB_t* l_OCCBHead = NULL;			// Head of registrars

static char l_SearchList[MAXSEARCHBUFFER][PATH_MAX];	// Places to look for WADs
static size_t l_SearchCount = 0;	// Number of places to look

/*** FUNCTIONS ***/

/* WL_BaseName() -- Returns the base name of the passed filename */
static const char* WL_BaseName(const char* const a_File)
{
	const char* TempX = NULL;
	const char* OldTempX = NULL;
	
	/* Check */
	if (!a_File)
		return NULL;
		
	/* Get last occurence */
	// Find last slash
	TempX = strrchr(a_File, '/');
	
#if defined(_WIN32)
	// Find last backslash if we found no slashes
	if (!TempX)
		TempX = strrchr(a_File, '\\');
		
	// Otherwise find the last backslash but revert to last slash if not found
	else
	{
		OldTempX = TempX;
		TempX = strrchr(OldTempX, '\\');
		
		if (!TempX)
			TempX = OldTempX;
	}
#endif
	
	// Found nothing at all
	if (!TempX)
		TempX = a_File;
		
	/* Check if we landed on a slash */
	while (*TempX == '/'
#if defined(_WIN32)
	        || *TempX == '\\'
#endif
	      )
		TempX++;
		
	/* Return pointer */
	return TempX;
}

/* ZLP_EntryHashCheck() -- Compares entry hash */
// A = const char*
// B = WL_WADEntry_t*
static bool_t ZLP_EntryHashCheck(void* const a_A, void* const a_B)
{
	const char* A;
	WL_WADEntry_t* B;
	
	/* Check */
	if (!a_A || !a_B)
		return false;
		
	/* Clone */
	A = a_A;
	B = a_B;
	
	/* Compare name */
	if (strcasecmp(A, B->Name) == 0)
		return true;
		
	// Not matched
	return false;
}

/* WL_OpenWAD() -- Opens a WAD File */
const WL_WADFile_t* WL_OpenWAD(const char* const a_PathName)
{
	static uint32_t BaseTop;
	char FoundWAD[PATH_MAX];
	FILE* CFile;
	WL_WADFile_t* NewWAD;
	uint32_t Magic;
	bool_t IsWAD;
	char* FileName;
	char* Chunk;
	size_t i, j, k, n;
	char c;
	bool_t Dot;
	uint8_t u8;
	uint32_t u32;
	WL_WADEntry_t* ThisEntry;
	
	/* Check */
	if (!a_PathName)
		return NULL;
		
	/* First locate the WAD */
	if (!WL_LocateWAD(a_PathName, NULL, FoundWAD, PATH_MAX))
		return NULL;
		
	// Debug
	if (devparm)
		CONS_Printf("WL_OpenWAD: Opening \"%s\".\n", FoundWAD);
		
	/* Attempt opening of the file */
	CFile = fopen(FoundWAD, "rb");
	
	// Failed?
	if (!CFile)
		return NULL;
		
	/* Read the first 4 bytes, it is the magic number */
	Magic = 0;
	if (!fread(&Magic, 4, 1, CFile))
	{
		// Failed to read magic number
		fclose(CFile);
		return NULL;
	}
	Magic = LittleSwapUInt32(Magic);	// Swap for BE
	
	// Is it a WAD file? If not threat it as a lump instead
	IsWAD = false;
	if (Magic == 0x44415749U || Magic == 0x44415750U)
		IsWAD = true;
		
	/* Allocate Fresh WAD and pump it to the chain */
	NewWAD = Z_Malloc(sizeof(*NewWAD), PU_STATIC, NULL);
	
	// Checkers
	NewWAD->__Private.__IsWAD = IsWAD;
	
	if (Magic == 0x44415749U)
		NewWAD->__Private.__IsIWAD = true;
	
	// Add to the beginning of the chain (faster)
	if (!l_LFirstWAD)
		l_LFirstWAD = NewWAD;
	else
	{
		NewWAD->__Private.__PrevWAD = l_LFirstWAD;
		l_LFirstWAD->__Private.__NextWAD = NewWAD;
		l_LFirstWAD = NewWAD;
	}
	
	/* Set initial data */
	strncpy(NewWAD->__Private.__PathName, FoundWAD, PATH_MAX);
	strncpy(NewWAD->__Private.__FileName, WL_BaseName(NewWAD->__Private.__PathName), PATH_MAX);
	NewWAD->__Private.__CFile = CFile;
	
	// Determine the DOS name (kinda)
	n = strlen(NewWAD->__Private.__FileName);
	for (Dot = false, i = 0, j = 0, k = 0; i < n; i++)
	{
		c = NewWAD->__Private.__FileName[i];
		
		// A valid enough character
		if (isalnum(c) || c == '_' || c == '~')
		{
			// Normal Name
			if (!Dot)
			{
				if (j < 8)
					NewWAD->__Private.__DOSName[j++] = toupper(c);
			}
			
			// Extension
			else
			{
				if (k < 3)
					NewWAD->__Private.__DOSName[j + (k++)] = toupper(c);
			}
		}
		
		// Is the character a dot?
		if (c == '.')
		{
			Dot = true;
			NewWAD->__Private.__DOSName[j++] = '.';
		}
	}
	
	// Find size
	fseek(CFile, 0, SEEK_END);
	NewWAD->__Private.__Size = ftell(CFile);
	fseek(CFile, 0, SEEK_SET);
	
	/* Determine checksums */
	// Simple Sum
	for (i = 0, j = 0, k = 0; i < NewWAD->__Private.__Size; i++)
	{
		// Read single byte
		fread(&u8, 1, 1, CFile);
		
		// If even, do XOR
		if (!(i & 0))
			NewWAD->SimpleSum[k] ^= (uint32_t)u8 << ((uint32_t)j * 8);
		
		// Otherwise, do XNOR
		else
			NewWAD->SimpleSum[k] ^= ((uint32_t)(~u8)) << ((uint32_t)j * 8);
		
		// Cycle?
		if (++j >= 4)
		{
			j = 0;
			
			if (++k >= 4)
				k = 0;
		}
	}
	
	// MD5
	
	/* Top hash ID */
	BaseTop += (8 << 16);
	NewWAD->__Private.__TopHash = BaseTop;
	
	/* Read type specific data */
	// Create hash table for fast access
	NewWAD->__Private.__EntryHashes = Z_HashCreateTable(ZLP_EntryHashCheck);
	
	// This is a WAD File (Load entries from WAD)
	if (IsWAD)
	{
		// Seek to start of info
		fseek(CFile, 4, SEEK_SET);
		
		// Read lump count and swap it
		fread(&u32, 4, 1, CFile);
		u32 = LittleSwapUInt32(u32);
		NewWAD->NumEntries = u32;
		
		// Read index offset and swap it
		fread(&u32, 4, 1, CFile);
		u32 = LittleSwapUInt32(u32);
		NewWAD->__Private.__IndexOff = u32;
		
		// Index rolls off file? If so, clip number of them
		if (NewWAD->__Private.__IndexOff + (NewWAD->NumEntries * 16) > NewWAD->__Private.__Size)
			NewWAD->NumEntries = (NewWAD->__Private.__Size - NewWAD->__Private.__IndexOff) / 16;
		
		// Error correction (corrupt WAD?)
		if (NewWAD->__Private.__IndexOff >= NewWAD->__Private.__Size)
		{
			// Clear so it is an empty WAD
			NewWAD->NumEntries = 0;
			NewWAD->__Private.__IndexOff = 0;
		}
		
		// Seek to index location
		fseek(CFile, NewWAD->__Private.__IndexOff, SEEK_SET);
		
		// Allocate entries
		NewWAD->Entries = Z_Malloc(sizeof(WL_WADEntry_t) * NewWAD->NumEntries, PU_STATIC, NULL);
		
		// Start reading the index
		for (i = 0; i < NewWAD->NumEntries; i++)
		{
			// Get current entry
			ThisEntry = &NewWAD->Entries[i];
			
			// Initialize Links
			ThisEntry->Owner = NewWAD;
			if (i > 1 && i < NewWAD->NumEntries - 1)	// Forward
				ThisEntry->NextEntry = &NewWAD->Entries[i + 1];
			if (i > 0)									// Back
				ThisEntry->PrevEntry = &NewWAD->Entries[i - 1];
			
			// Initialize index
			ThisEntry->Index = i;
			ThisEntry->GlobalIndex = NewWAD->__Private.__TopHash + ThisEntry->Index;
			
			// Read lump position
			fread(&u32, 4, 1, CFile);
			u32 = LittleSwapUInt32(u32);
			ThisEntry->__Private.__Offset = u32;
			
			// Read lump internal size
			fread(&u32, 4, 1, CFile);
			u32 = LittleSwapUInt32(u32);
			ThisEntry->__Private.__InternalSize = u32;
			
			// Read name
			for (Dot = false, j = 0; j < 8; j++)
			{
				fread(&c, 1, 1, CFile);	
				
				if (!Dot)
					if (c == '\0')
						Dot = true;
					else
						ThisEntry->Name[j] = tolower(c);
			}
			
			// Check for PSX/N64 Compression
			if (ThisEntry->Name[0] & 0x80)
			{
				// Remove bit and set compression flag
				ThisEntry->Name[0] &= ~0x80;
				ThisEntry->__Private.__Compressed = true;
				
				// TODO: Implement compression
				ThisEntry->Size = 0;
			}
			
			// Not compressed
			else
				ThisEntry->Size = ThisEntry->__Private.__InternalSize;
		}
	}
	// This is a standard lump (make WAD a single entry containing the entire file)
	else
	{
		// Seek to start of file
		fseek(CFile, 0, SEEK_SET);
		
		// Create single entry
		NewWAD->NumEntries = 1;
		ThisEntry = NewWAD->Entries = Z_Malloc(sizeof(WL_WADEntry_t), PU_STATIC, NULL);
		
		// Set size and such
		ThisEntry->__Private.__Offset = 0;
		ThisEntry->Size = ThisEntry->__Private.__InternalSize = NewWAD->__Private.__Size;
		
		// Name is the DOS name
		for (Dot = false, i = 0; i < WLMAXDOSNAME && i < WLMAXENTRYNAME; i++)
			if (!Dot)
				if (NewWAD->__Private.__DOSName[i] == '.')
					Dot = true;
				else
					ThisEntry->Name[i] = NewWAD->__Private.__DOSName[i];
	}
	
	/* Hash everything */
	// This is done here to allow all formats to use the hash table, it also
	// reduces code wastage.
	for (i = 0; i < NewWAD->NumEntries; i++)
	{
		// Get current entry
		ThisEntry = &NewWAD->Entries[i];
		
		// Determine hash
		ThisEntry->Hash = Z_Hash(ThisEntry->Name);
		
		// Add to table
		Z_HashAddEntry(NewWAD->__Private.__EntryHashes, ThisEntry->Hash, ThisEntry);
	}
	
	/* Run Data Registration */
	for (i = 0; i < l_NumPDC; i++)
		WL_GetPrivateData(NewWAD, l_PDCp[i]->Key, NULL);
	
	/* Success */
	NewWAD->__Private.__IsValid = true;
	
	// Debug
	if (devparm)
	{
		u32 = NewWAD->NumEntries;
		CONS_Printf("WL_OpenWAD: Loaded \"%s\"%s%s [%u Entries, %08x%08x%08x%08x]\n", NewWAD->__Private.__DOSName, (NewWAD->__Private.__IsIWAD ? "*" : ""), (NewWAD->__Private.__IsWAD ? "" : "+"), u32, NewWAD->SimpleSum[0], NewWAD->SimpleSum[1], NewWAD->SimpleSum[2], NewWAD->SimpleSum[3]);
	}
		
	/* Return the generated WAD */
	return NewWAD;
}

/* WL_GetWADName() -- Get the name of a WAD */
const char* const WL_GetWADName(const WL_WADFile_t* const a_WAD, const bool_t a_NonDOSName)
{
	/* Check */
	if (!a_WAD)
		return NULL;
	
	/* Return the correct name */
	if (a_NonDOSName)
		return a_WAD->__Private.__FileName;
	return a_WAD->__Private.__DOSName;
}

/* WL_Iterate() -- Iterates (Virtual) WAD Files */
const WL_WADFile_t* WL_IterateVWAD(const WL_WADFile_t* const a_WAD, const bool_t a_Forwards)
{
	WL_WADFile_t* Rover;
	
	/* NULL WAD means the last/first */
	if (!a_WAD)
	{
		// Forwards needs no roving
		if (a_Forwards)
			return l_LFirstVWAD;
		
		// Backwards used to need roving, not anymore (speed!)
		else
			return l_LLastVWAD;
	}
	
	/* Non-null means iterate to the next */
	else
	{
		if (a_Forwards)
			return a_WAD->NextVWAD;
		else
			return a_WAD->PrevVWAD;
	}
	
	/* ??? */
	return NULL;
}

/* WL_CloseWAD() -- Closes a WAD File */
void WL_CloseWAD(const WL_WADFile_t* const a_WAD)
{
}

/* WL_PushWAD() -- Pushes a WAD to the end of the virtual stack */
void WL_PushWAD(const WL_WADFile_t* const a_WAD)
{
	WL_WADFile_t* Rover;
	WL_OCCB_t* CB;
	
	/* Check */
	if (!a_WAD)
		return;
	
	/* Already attached in the chain? */
	if (a_WAD->PrevVWAD || a_WAD->NextVWAD)
	{
		if (devparm)
			CONS_Printf("WL_PushWAD: WAD already linked!\n");
		
		return;
	}
	
	/* Debug */
	if (devparm)
		CONS_Printf("WL_PushWAD: Pushing \"%s\".\n", a_WAD->__Private.__DOSName);
	
	/* No chain at all? */
	if (!l_LFirstVWAD)
	{
		l_LLastVWAD = l_LFirstVWAD = (WL_WADFile_t*)a_WAD;
		return;
	}
	
	/* Chain needs linking */
	Rover = l_LFirstVWAD;
	
	while (Rover)
	{
		// No next?
		if (!Rover->NextVWAD)
		{
			// Pair and return
			Rover->NextVWAD = (WL_WADFile_t*)a_WAD;
			((WL_WADFile_t*)a_WAD)->PrevVWAD = Rover;
			
			// Last virtual WAD is always the justly pushed WAD! simple
			l_LLastVWAD = ((WL_WADFile_t*)a_WAD);
			
			// Call order change callbacks
			CB = l_OCCBHead;
			
			while (CB)
			{
				if (CB->Func)
					CB->Func(true, l_LLastVWAD);
				CB = CB->Next;
			}
			
			// Return now
			return;
		}
		
		// Go to next
		Rover = Rover->NextVWAD;
	}
}

/* WL_PopWAD() -- Pops a WAD from the end of the virtual stack */
const WL_WADFile_t* WL_PopWAD(void)
{
	WL_WADFile_t* Rover;
	WL_OCCB_t* CB;
	
	/* Get first */
	Rover = l_LFirstVWAD;
	
	// Seek to end
	while (Rover && Rover->NextVWAD)
		Rover = Rover->NextVWAD;
	
	/* Unlink */
	if (Rover)
	{
		// Set last WAD to the previous
		l_LLastVWAD = Rover->PrevVWAD;
		
		// Remove from chain
		if (Rover->PrevVWAD)
			Rover->PrevVWAD->NextVWAD = NULL;
		Rover->PrevVWAD = Rover->NextVWAD = NULL;
	}
	
	/* Top of chain? */
	if (Rover == l_LFirstVWAD)
		l_LFirstVWAD = NULL;
		
	/* Call order change callbacks */
	CB = l_OCCBHead;
	
	while (CB)
	{
		if (CB->Func)
			CB->Func(false, Rover);
		CB = CB->Next;
	}
	
	/* Return the popped off WAD */
	return Rover;
}

/* WLP_BaseName() -- Returns the base name of the WAD */
static const char* WLP_BaseName(const char* const a_File)
{
	const char* TempX = NULL;
	const char* OldTempX = NULL;
	
	/* Check */
	if (!a_File)
		return NULL;
		
	/* Get last occurence */
	// Find last slash
	TempX = strrchr(a_File, '/');
	
#if defined(_WIN32)
	// Find last backslash if we found no slashes
	if (!TempX)
		TempX = strrchr(a_File, '\\');
		
	// Otherwise find the last backslash but revert to last slash if not found
	else
	{
		OldTempX = TempX;
		TempX = strrchr(OldTempX, '\\');
		
		if (!TempX)
			TempX = OldTempX;
	}
#endif
	
	// Found nothing at all
	if (!TempX)
		TempX = a_File;
		
	/* Check if we landed on a slash */
	while (*TempX == '/'
#if defined(_WIN32)
	        || *TempX == '\\'
#endif
	      )
		TempX++;
		
	/* Return pointer */
	return TempX;
}

/* WL_LocateWAD() -- Finds WAD on the disk */
bool_t WL_LocateWAD(const char* const a_Name, const char* const a_MD5, char* const a_OutPath, const size_t a_OutSize)
{
	char CheckBuffer[PATH_MAX];
	char BaseWAD[PATH_MAX];
	const char* p;
	size_t i, j;
	char* DirArg;
	char* End;
	
	/* Initialize the search list */
	if (!l_SearchCount)
	{
		// Clear it
		memset(l_SearchList, 0, sizeof(l_SearchList));
		
		// Add implicit nothing, current dir, bin/
		strncpy(l_SearchList[l_SearchCount++], "", PATH_MAX);
		strncpy(l_SearchList[l_SearchCount++], "./", PATH_MAX);
		strncpy(l_SearchList[l_SearchCount++], "bin/", PATH_MAX);
		
		// -waddir argument
		if (M_CheckParm("-waddir"))
			while (M_IsNextParm())
			{
				// Get directory argument
				DirArg = M_GetNextParm();
				
				// Add to Search
				if (l_SearchCount < MAXSEARCHBUFFER)
					// Copy
					strncpy(l_SearchList[l_SearchCount], DirArg, PATH_MAX);
			}
		// $DOOMWADDIR
		if ((DirArg = getenv("DOOMWADDIR")))
		{
			// Add to search
			if (l_SearchCount < MAXSEARCHBUFFER)
				// Copy
				strncpy(l_SearchList[l_SearchCount], DirArg, PATH_MAX);
		}
		// $DOOMWADPATH
		if ((DirArg = getenv("DOOMWADPATH")))
		{
			do
			{
				// Find target (the next : or ;)
				End = strchr(DirArg, WLPATHDIRSEP);
				
				// Get size, from End to DirArg or just strlen if this is the end
				if (End)
					j = End - DirArg;
				else
					j = strlen(DirArg);
					
				// Add to search
				if (l_SearchCount < MAXSEARCHBUFFER)
					// Copy only up to j characters (excludes : or ;)
					strncpy(l_SearchList[l_SearchCount], DirArg, (j < PATH_MAX ? j : PATH_MAX));
					
				// Go to end
				if (End)
					DirArg = End + 1;
				else
					DirArg = NULL;
			}
			while (DirArg);
		}
		// Add trailing / to the end of all the searches
		for (i = 0; i < l_SearchCount; i++)
			strncat(l_SearchList[i], "/", PATH_MAX);
			
		// Debug
		if (devparm)
			for (i = 0; i < l_SearchCount; i++)
				CONS_Printf("WL_LocateWAD: Searching \"%s\".\n", l_SearchList[i]);
	}
	
	/* Name is required and a size must be given if a_OutPath is set */
	if (!a_Name || (a_OutPath && !a_OutSize))
		return false;
		
	/* Look in the search buffer for said WADs */
	for (j = 0; j < 2; j++)
	{
		// Search the exact given name
		if (!j)
			p = a_Name;
		
		// If this point is reached, then try the basename (always try to succeed)
		else
		{
			memset(BaseWAD, 0, sizeof(BaseWAD));
			strncpy(BaseWAD, WLP_BaseName(a_Name), PATH_MAX);
			p = BaseWAD;
		}
		
		// Now search
		for (i = 0; i < l_SearchCount; i++)
		{
			// Clear the check buffer
			memset(CheckBuffer, 0, sizeof(CheckBuffer));
			
			// Append the current search path along with the name of the WAD
			// Do not add trailing slash here since we already do it as needed in WX_Init()
			strncpy(CheckBuffer, l_SearchList[i], PATH_MAX);
			strncat(CheckBuffer, p, PATH_MAX);
			
			// Check whether we can read it
			if (!access(CheckBuffer, R_OK))
			{
				// TODO: Check MD5
				
				// Send to output buffer (if it was passed)
				if (a_OutPath)
					strncpy(a_OutPath, CheckBuffer, a_OutSize);
					
				// Success
				return true;
			}
		}
	}
	
	/* Failed */
	return false;
}

/* WL_RegisterOCCB() -- This is called whenever the WAD order changes */
bool_t WL_RegisterOCCB(WL_OrderCBFunc_t const a_Func, const uint8_t a_Order)
{
	WL_OCCB_t* New;
	WL_OCCB_t* Temp;
	
	/* Check */
	if (!a_Func)
		return false;
	
	/* Attach to tail, in the correct order */
	// First?
	if (!l_OCCBHead)
		l_OCCBHead = New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
		
	// Rover
	else
	{
		// Head
		Temp = l_OCCBHead;
		
		// While there is a current
		while (Temp)
		{
			// Order is lower?
			if (a_Order < Temp->Order)
			{
				// Create and link here
				New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
				
				// Re-chain
				New->Prev = Temp->Prev;
				New->Next = Temp;
				if (Temp->Prev)
					Temp->Prev->Next = New;
				Temp->Prev = New;
				break;
			}
			
			// There is no next?
			if (!Temp->Next)
			{
				// Create next and break
				New = Temp->Next = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
				New->Prev = Temp;
				break;
			}
			
			// Go to next
			Temp = New->Next;
		}
	}
	
	/* No new? */
	if (!New)
		return false;	// just in case
	
	/* Set info */
	New->Func = a_Func;
	New->Order = a_Order;
	
	/* Success */
	return true;
}

/* WL_RegisterPDC() -- Registers a data handler */
bool_t WL_RegisterPDC(const uint32_t a_Key, const uint8_t a_Order, WL_PCCreatorFunc_t const a_CreatorFunc, WL_RemoveFunc_t const a_RemoveFunc)
{
	size_t i, j, k, z;
	WL_PDC_t* Slot;
	WL_WADFile_t* WAD;
	
	/* Check */
	if (!a_Key || !a_CreatorFunc)
		return false;
	
	/* No more room? */
	if (l_NumPDC + 1 >= (WLMAXPRIVATEWADSTUFF - 1))
		return false;
	
	/* See if the key is used already */
	for (i = 0; i < l_NumPDC; i++)
		if (a_Key == l_PDC[i].Key)
			return false;
	
	/* Add it at the end */
	Slot = &l_PDC[l_NumPDC++];
	Slot->Key = a_Key;
	Slot->Order = a_Order;
	Slot->CreatorFunc = a_CreatorFunc;
	Slot->RemoveFunc = a_RemoveFunc;
	
	/* Sort */
	// Clear all markers first
	for (i = 0; i < l_NumPDC; i++)
		l_PDC[i].Mark = false;
	
	// Use presorted pointers, easy
	memset(l_PDCp, 0, sizeof(l_PDCp));
	
	for (k = 0, z = 0; z < l_NumPDC; z++)
	{
		// Get first unmarked
		for (i = 0; i < l_NumPDC; i++)
			if (!l_PDC[i].Mark)
			{
				j = i;
				break;
			}
		
		// Find lowest
		for (i = 0; i < l_NumPDC; i++)
			if (!l_PDC[i].Mark)
				if (l_PDC[i].Order < l_PDC[j].Order)
					j = i;
		
		// Put pointer in sort
		l_PDCp[k++] = &l_PDC[j];
	}
	
	/* Create data for WADs */
	WAD = l_LFirstWAD;
	
	while (WAD)
	{
		// Getting private data means creating it
		WL_GetPrivateData(WAD, a_Key, NULL);
	
		// Next
		WAD = WAD->__Private.__NextWAD;
	}
	
	/* Success! */
	return true;
}

/* WL_GetPrivateData() -- Retrieves existing private data */
void* WL_GetPrivateData(const WL_WADFile_t* const a_WAD, const uint32_t a_Key, size_t* const a_SizePtr)
{
	size_t i;
	void* p = NULL;
	WL_PDC_t* PDC;
	
	/* Check */
	if (!a_WAD)
		return NULL;
	
	/* Find data in the chain */
	for (i = 0; i < a_WAD->__Private.__PublicData.__NumStuffs; i++)
		if (a_WAD->__Private.__PublicData.__Stuff[i].__Key == a_Key)
			return a_WAD->__Private.__PublicData.__Stuff[i].__DataPtr;
	
	/* If this point was reached, then it does not exist */
	// Check for overflow
	if (a_WAD->__Private.__PublicData.__NumStuffs + 1 >= (WLMAXPRIVATEWADSTUFF - 1))
	{
		if (devparm)
			CONS_Printf("WL_GetPrivateData: Stuff overflow.\n");
		return NULL;	// Overflow
	}
	
	// Find which creator to use
	for (PDC = NULL, i = 0; i < l_NumPDC; i++)
		if (l_PDCp[i]->Key == a_Key)
		{
			PDC = l_PDCp[i];
			break;
		}
	
	// Not found?
	if (!PDC)
		return NULL;
	
	// Create data here
	((WL_WADFile_t*)a_WAD)->__Private.__PublicData.__Stuff[a_WAD->__Private.__PublicData.__NumStuffs].__Key = a_Key;
	((WL_WADFile_t*)a_WAD)->__Private.__PublicData.__Stuff[a_WAD->__Private.__PublicData.__NumStuffs].__RemoveFunc = PDC->RemoveFunc;
	
	if (!PDC->CreatorFunc(
			a_WAD,
			a_Key,
			&((WL_WADFile_t*)a_WAD)->__Private.__PublicData.__Stuff[a_WAD->__Private.__PublicData.__NumStuffs].__DataPtr,
			&((WL_WADFile_t*)a_WAD)->__Private.__PublicData.__Stuff[a_WAD->__Private.__PublicData.__NumStuffs].__Size,
			&((WL_WADFile_t*)a_WAD)->__Private.__PublicData.__Stuff[a_WAD->__Private.__PublicData.__NumStuffs].__RemoveFunc
		))
	{	// Failed?
		if (devparm)
			CONS_Printf("WL_GetPrivateData: Private data creation failed!\n");
		
		memset(&((WL_WADFile_t*)a_WAD)->__Private.__PublicData.__Stuff[a_WAD->__Private.__PublicData.__NumStuffs], 0, sizeof(a_WAD->__Private.__PublicData.__Stuff[a_WAD->__Private.__PublicData.__NumStuffs]));
		return NULL;
	}
	
	p = ((WL_WADFile_t*)a_WAD)->__Private.__PublicData.__Stuff[a_WAD->__Private.__PublicData.__NumStuffs].__DataPtr;
	
	((WL_WADFile_t*)a_WAD)->__Private.__PublicData.__NumStuffs++;
	
	/* Success? */
	return p;
}

/* WL_FindEntry() -- Find an entry by name */
const WL_WADEntry_t* WL_FindEntry(const WL_WADFile_t* const a_BaseSearch, const uint32_t a_Flags, const char* const a_Name)
{
	WL_WADFile_t* Rover;
	const WL_WADEntry_t* Entry;
	uint32_t Hash;
	
	/* Global virtual search */
	if (!a_BaseSearch)
	{
		// To simplify the implementation of this, do recursive calls!
		for (Rover = (WL_WADFile_t*)WL_IterateVWAD(NULL, ((a_Flags & WLFF_FORWARDS) ? true : false)); Rover; Rover = (WL_WADFile_t*)WL_IterateVWAD((const WL_WADFile_t*)Rover, ((a_Flags & WLFF_FORWARDS) ? true : false)))
			// Recursive call (simplifies the implementation), make sure IMPORTANT is NOT SET!
			if ((Entry = WL_FindEntry((const WL_WADFile_t*)Rover, a_Flags & (~(WLFF_IMPORTANT)), a_Name)))
				return Entry;	// It was found here
	}
	
	/* Specific to this WAD */
	else
	{
		// Look in hash table
		Hash = Z_Hash(a_Name);
		Entry = (const WL_WADEntry_t*)Z_HashFindEntry(a_BaseSearch->__Private.__EntryHashes, Hash, (void*)a_Name, ((a_Flags & WLFF_FORWARDS) ? false : true));
		
		// If found, return (for IMPORTANT case later on)
		if (Entry)
			return Entry;
	}
	
	/* I_Error? */
	if (a_Flags & WLFF_IMPORTANT)
		I_Error("WL_FindEntry: Failed to find \"%s\".", a_Name);
	
	/* Not found, so return nothing */
	return NULL;
}

/* WL_TranslateEntry() -- Translate a Global Index to/from Entry pointer */
uintptr_t WL_TranslateEntry(const WadIndex_t a_GlobalIndex, const WL_WADFile_t* const a_Entry)
{
	return 0;
}

/* WL_ReadData() -- Read WAD data into memory */
size_t WL_ReadData(const WL_WADEntry_t* const a_Entry, const size_t a_Offset, void* const a_Out, const size_t a_OutSize)
{
	size_t CorrectedSize;
	
	/* Check */
	if (!a_Entry || !a_Out || !a_OutSize)
		return 0;
	
	/* Get corrected size of read */
	if (a_Offset >= a_Entry->Size)
		CorrectedSize = 0;
	else if (a_Offset + a_OutSize > a_Entry->Size)
		CorrectedSize = a_Entry->Size - a_Offset;
	else
		CorrectedSize = a_OutSize;
	
	/* Nothing to read? */
	if (!CorrectedSize)
		return 0;
	
	/* Does the data need cache? */
	// Load cache
	if (!a_Entry->__Private.__Data)
	{
		// Allocate for all data
		((WL_WADEntry_t*)a_Entry)->__Private.__Data = Z_Malloc(a_Entry->Size, PU_STATIC, (void**)&a_Entry->__Private.__Data);
		
		// Seek to WAD location
		fseek(
				((FILE*)((WL_WADEntry_t*)a_Entry)->Owner->__Private.__CFile),
				a_Entry->__Private.__Offset,
				SEEK_SET
			);
		
		// Read data
		fread(
				((WL_WADEntry_t*)a_Entry)->__Private.__Data,
				a_Entry->Size,
				1,
				((FILE*)((WL_WADEntry_t*)a_Entry)->Owner->__Private.__CFile)
			);
	}
	
	// Change tag to prevent random free?
	else
		Z_ChangeTag(a_Entry->__Private.__Data, PU_STATIC);
	
	/* Copy into output area */
	memmove(a_Out, (void*)(((uintptr_t)a_Entry->__Private.__Data) + a_Offset), CorrectedSize);
	
	/* Return former tag */
	Z_ChangeTag(a_Entry->__Private.__Data, PU_CACHE);
	
	return CorrectedSize;
}

/*********************
*** W_ DEPRECATION ***
*********************/

#define OLDWPDCKEY		0x77444C4FU

/* WP_DepCreate() -- Creates old form WadFile_t and WadEntry_t */
static bool_t WP_DepCreate(const struct WL_WADFile_s* const a_WAD, const uint32_t a_Key, void** const a_DataPtr, size_t* const a_SizePtr, WL_RemoveFunc_t* const a_RemoveFuncPtr)
{
	WadFile_t* WAD;
	
	/* Check */
	if (!a_WAD || !a_Key || !a_DataPtr || !a_SizePtr)
		return;
		
	/* Create WAD info on data */
	*a_SizePtr = sizeof(WadFile_t);
	WAD = *a_DataPtr = Z_Malloc(*a_SizePtr, PU_STATIC, NULL);
	
	/* Fill in WAD Info */
	WAD->DepWAD = a_WAD;
	WAD->FileName = a_WAD->__Private.__DOSName;
	WAD->NumLumps = a_WAD->NumEntries;
	WAD->Size = a_WAD->__Private.__Size;
	
	/* Success */
	return true;
}

/* WP_DepRemove() -- Removes the old deprecated WAD Code stuff */
void WP_DepRemove(const struct WL_WADFile_s* a_WAD)
{
	/* Check */
	if (!a_WAD)
		return;
}


size_t __REMOOD_DEPRECATED W_NumWadFiles(void);
WadFile_t* __REMOOD_DEPRECATED W_GetWadForNum(size_t Num);
WadFile_t* __REMOOD_DEPRECATED W_GetWadForName(char* Name);
size_t __REMOOD_DEPRECATED W_GetNumForWad(WadFile_t* WAD);
WadEntry_t* __REMOOD_DEPRECATED W_GetEntry(WadIndex_t lump);
WadIndex_t __REMOOD_DEPRECATED W_LumpsSoFar(WadFile_t* wadid);

/* W_InitMultipleFiles() -- Initializes multiple file inputs */
// This is the main code for loading WADs
WadIndex_t __REMOOD_DEPRECATED W_InitMultipleFiles(char** filenames)
{
	/* Register private data for the old WAD Code */
	if (!WL_RegisterPDC(OLDWPDCKEY, 25, WP_DepCreate, WP_DepRemove))
		I_Error("W_InitMultipleFiles: Failed to register PDC.");
	
	/* Failure */
	return 0;
}

void __REMOOD_DEPRECATED W_Shutdown(void);
int __REMOOD_DEPRECATED W_LoadWadFile(char* filename);

/* W_Reload() -- Reloads the WAD for level update */
void __REMOOD_DEPRECATED W_Reload(void)
{
	// Not used at all
}

WadIndex_t __REMOOD_DEPRECATED W_BiCheckNumForName(char* name, int forwards);
WadIndex_t __REMOOD_DEPRECATED W_CheckNumForName(char* name);
WadIndex_t __REMOOD_DEPRECATED W_CheckNumForNamePwad(char* name, size_t wadid, WadIndex_t startlump);
WadIndex_t __REMOOD_DEPRECATED W_CheckNumForNamePwadPtr(char* name, WadFile_t* wadid, WadIndex_t startlump);
WadIndex_t __REMOOD_DEPRECATED W_GetNumForName(char* name);
WadIndex_t __REMOOD_DEPRECATED W_CheckNumForNameFirst(char* name);
WadIndex_t __REMOOD_DEPRECATED W_GetNumForNameFirst(char* name);
size_t __REMOOD_DEPRECATED W_LumpLength(WadIndex_t lump);
size_t __REMOOD_DEPRECATED W_ReadLumpHeader(WadIndex_t lump, void* dest, size_t size);
void __REMOOD_DEPRECATED W_ReadLump(WadIndex_t lump, void* dest);
void* __REMOOD_DEPRECATED W_CacheLumpNum(WadIndex_t lump, size_t PU);
void* __REMOOD_DEPRECATED W_CacheLumpName(char* name, size_t PU);
void* __REMOOD_DEPRECATED W_CachePatchName(char* name, size_t PU);
void* __REMOOD_DEPRECATED W_CacheRawAsPic(WadIndex_t lump, int width, int height, size_t tag);	// return a pic_t
WadIndex_t __REMOOD_DEPRECATED W_GetNumForEntry(WadEntry_t* Entry);
void __REMOOD_DEPRECATED W_LoadData(void);

/* W_FindWad() -- Locates a WAD File */
bool_t __REMOOD_DEPRECATED W_FindWad(const char* Name, const char* MD5, char* OutPath, const size_t OutSize)
{
	/* This function is exactly the same */
	return WL_LocateWAD(Name, MD5, OutPath, OutSize);
}

/* W_BaseName() -- Basename of file */
const char* __REMOOD_DEPRECATED W_BaseName(const char* Name)
{
	// The original version
	return WLP_BaseName(Name);
}

void* __REMOOD_DEPRECATED W_CachePatchNum(const WadIndex_t Lump, size_t PU);

/**********************
*** WX_ DEPRECATION ***
**********************/

bool_t __REMOOD_DEPRECATED WX_Init(void);
bool_t __REMOOD_DEPRECATED WX_LocateWAD(const char* const a_Name, const char* const a_MD5, char* const a_OutPath, const size_t a_OutSize);
WX_WADFile_t* __REMOOD_DEPRECATED WX_RoveWAD(WX_WADFile_t* const a_WAD, const bool_t a_Virtual, const int32_t a_Next);
WX_WADEntry_t* __REMOOD_DEPRECATED WX_GetNumEntry(WX_WADFile_t* const a_WAD, const size_t a_Index);
WX_WADEntry_t* __REMOOD_DEPRECATED WX_EntryForName(WX_WADFile_t* const a_WAD, const char* const a_Name, const bool_t a_Forwards);
void* __REMOOD_DEPRECATED WX_CacheEntry(WX_WADEntry_t* const a_Entry);
size_t __REMOOD_DEPRECATED WX_UseEntry(WX_WADEntry_t* const a_Entry, const bool_t a_Use);
bool_t __REMOOD_DEPRECATED WX_GetVirtualPrivateData(WX_WADFile_t* const a_WAD, const WX_DataPrivateID_t a_ID, void** *const a_PPPtr,
                                                    size_t** const a_PPSize);
WX_WADEntry_t* __REMOOD_DEPRECATED WX_RoveEntry(WX_WADEntry_t* const a_Entry, const int32_t a_Next);

/* WX_GetEntryName() -- Return name of entry */
size_t __REMOOD_DEPRECATED WX_GetEntryName(WX_WADEntry_t* const a_Entry, char* const a_OutBuf, const size_t a_OutSize)
{
	size_t s;
	
	if (!a_Entry || !a_OutBuf || !a_OutSize)
		return 0;
	
	/* Copy */
	strncpy(a_OutBuf, a_Entry->Name, a_OutSize);
	
	/* Return */
	s = strlen(a_Entry->Name);
	
	if (s < a_OutSize)
		return s;
	else
		return a_OutSize;
}

/* WX_GetEntrySize() -- Return size of entry */
size_t __REMOOD_DEPRECATED WX_GetEntrySize(WX_WADEntry_t* const a_Entry)
{
	if (!a_Entry)
		return 0;
	return a_Entry->Size;
}

/* WX_ClearUnused() -- Clear unused data */
size_t __REMOOD_DEPRECATED WX_ClearUnused(void)
{
	// Does nothing
	return 0;
}

