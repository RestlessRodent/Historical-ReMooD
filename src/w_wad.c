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
// Copyright (C) 2008-2012 GhostlyDeath <ghostlydeath@remood.org>
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
#include "console.h"

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
static bool_t l_LockOCCB = false;				// OCCB is locked
static bool_t l_PendingOCCB = false;			// an OCCB is pending

static char l_SearchList[MAXSEARCHBUFFER][PATH_MAX];	// Places to look for WADs
static size_t l_SearchCount = 0;	// Number of places to look

/*** FUNCTIONS ***/

/* WLS_MultiCom() -- Multi command handler */
static int WLS_MultiCom(const uint32_t a_ArgC, const char** const a_ArgV)
{
#define POPARRAY 64
	WL_WADFile_t* WAD, *NewWAD, *ReMooDWAD;
	WL_WADFile_t* WArray[POPARRAY];
	int32_t i;
	
	/* Push Another WAD */
	if (strcasecmp(a_ArgV[0], "wadpush") == 0)
	{
		// Not enough args?
		if (a_ArgC < 2)
		{
			CONL_PrintF("%s <wad>\n", a_ArgV[0]);
			return CLE_FAILURE;
		}
		
		// Try opening the WAD
		WAD = WL_OpenWAD(a_ArgV[1]);
		
		// Failed?
		if (!WAD)
		{
			CONL_PrintF("WAD: Failed to open WAD\n");
			return CLE_FAILURE;
		}
		
		// Add to VWAD stack
		WL_PushWAD(WAD);
		
		// Success!
		return CLE_SUCCESS;
	}
	
	/* Pop Top-Most WAD */
	else if (strcasecmp(a_ArgV[0], "wadpop") == 0)
	{
		WAD = WL_IterateVWAD(NULL, false);
		
		// Find ReMooD.WAD
		ReMooDWAD = WL_IterateVWAD(NULL, true);
		ReMooDWAD = WL_IterateVWAD(ReMooDWAD, true);
		
		// Make sure it isn't remood.wad
		if (!WAD || (WAD && WAD == ReMooDWAD))
		{
			CONL_PrintF("WAD: Nothing to pop\n");
			return CLE_FAILURE;
		}
		
		// Pop it
		WL_PopWAD();
		
		// Success!
		return CLE_SUCCESS;
	}
	
	/* Change IWAD */
	else if (strcasecmp(a_ArgV[0], "wadiwad") == 0)
	{
		// Not enough args?
		if (a_ArgC < 2)
		{
			CONL_PrintF("%s <iwad>\n", a_ArgV[0]);
			return CLE_FAILURE;
		}
		
		// Open the specified IWAD (will need it)
		NewWAD = WL_OpenWAD(a_ArgV[1]);
		
		// Failed to open?
		if (!NewWAD)
		{
			CONL_PrintF("WAD: Failed to open WAD\n");
			return CLE_FAILURE;
		}
		
		// Lock OCCB (Massive changes)
		WL_LockOCCB(true);
		
		// Clear array
		memset(WArray, 0, sizeof(WArray));
		
		// First pop all WADs
		while ((WAD = WL_PopWAD()))
			if (i < POPARRAY - 1)
				WArray[i++] = WAD;
		
		// Push new IWAD
		WL_PushWAD(NewWAD);
		
		// Push remaining WADs
		for (i = i - 2; i >= 0; i--)
			WL_PushWAD(WArray[i]);
		
		// Unlock OCCB (Done with changes)
		WL_LockOCCB(false);
	}
	
	return CLE_FAILURE;
}

/* WL_Init() -- Initializes the WL Code */
void WL_Init(void)
{
	CONL_AddCommand("wadpush", WLS_MultiCom);
	CONL_AddCommand("wadpop", WLS_MultiCom);
	CONL_AddCommand("wadiwad", WLS_MultiCom);
}

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

/* WL_BaseNameEx() -- Externalized */
const char* WL_BaseNameEx(const char* const a_File)
{
	return WL_BaseName(a_File);
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
#define SUMBUF 4096
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
	WL_WADFile_t* WAD;
	uint8_t ShortBuf[16];
	static uint8_t* l_SumBuf;
	void* p;
	
	/* Check */
	if (!a_PathName)
		return NULL;
		
	/* First locate the WAD */
	if (!WL_LocateWAD(a_PathName, NULL, FoundWAD, PATH_MAX))
		return NULL;
		
#if defined(_WIN32)
	#define __REMOOD_PATHCOMPFUNC strcasecmp
#else
	#define __REMOOD_PATHCOMPFUNC strcmp
#endif
	
	// Determine if the WAD is already open (don't load WADs twice!)
		// This is important in networked situations and save games
	for (WAD = l_LFirstWAD; WAD; WAD = WAD->__Private.__PrevWAD)
	{
		// Compare name (path and base name)
		if (__REMOOD_PATHCOMPFUNC(FoundWAD, WAD->__Private.__PathName) == 0 ||
			strcasecmp(WL_BaseNameEx(FoundWAD), WAD->__Private.__FileName) == 0)
		{
			CONL_PrintF("WL_OpenWAD: \"%s\" is already open!\n", FoundWAD);
			return WAD;
		}
	}
#undef __REMOOD_PATHCOMPFUNC
		
	// Debug
	if (devparm)
		CONL_PrintF("WL_OpenWAD: Opening \"%s\".\n", FoundWAD);
		
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
		if (isalnum(c) || c == '-' || c == '_' || c == '~')
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
	
	// GhostlyDeath <May 7, 2012> -- Boot Status
	CONL_EarlyBootTic(NewWAD->__Private.__DOSName, true);
	
	// Find size
	fseek(CFile, 0, SEEK_END);
	NewWAD->__Private.__Size = ftell(CFile);
	fseek(CFile, 0, SEEK_SET);
	
	/* Determine checksums */
	// Create buffer?
	if (!l_SumBuf)
		l_SumBuf = Z_Malloc(SUMBUF, PU_STATIC, NULL);
	
	// Simple Sum
	for (i = 0, j = 0, k = 0; i < NewWAD->__Private.__Size; i += SUMBUF)
	{
		// Read bytes
		n = fread(l_SumBuf, 1, SUMBUF, CFile);
		
		// Process
		for (u32 = 0; u32 < n && u32 < SUMBUF; u32++)
		{
			// If even, do XOR
			if (!(i & 0))
				NewWAD->SimpleSum[k] ^= (uint32_t)l_SumBuf[u32] << ((uint32_t)j * 8);
		
			// Otherwise, do XNOR
			else
				NewWAD->SimpleSum[k] ^= ((uint32_t)(~l_SumBuf[u32])) << ((uint32_t)j * 8);
		
			// Cycle?
			if (++j >= 4)
			{
				j = 0;
			
				if (++k >= 4)
					k = 0;
			}
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
			
			// Read in bulk
			memset(ShortBuf, 0, sizeof(*ShortBuf) * 16);
			if (fread(ShortBuf, 16, 1, CFile) < 1)
				break;
				
			p = ShortBuf;
			
			// Read lump position
			ThisEntry->__Private.__Offset = LittleReadUInt32((uint32_t**)&p);
			
			// Read lump internal size
			ThisEntry->__Private.__InternalSize = LittleReadUInt32((uint32_t**)&p);
			
			// Read name
			for (Dot = false, j = 0; j < 8; j++)
			{
				c = ShortBuf[8 + j];
				
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
		ThisEntry->Owner = NewWAD;
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
	
	/* Success */
	NewWAD->__Private.__IsValid = true;
	
	// Print simple sum
	snprintf(NewWAD->SimpleSumChars, 33, "%08x%08x%08x%08x", NewWAD->SimpleSum[0], NewWAD->SimpleSum[1], NewWAD->SimpleSum[2], NewWAD->SimpleSum[3]);
	
	// Debug
	if (devparm)
	{
		u32 = NewWAD->NumEntries;
		CONL_PrintF("WL_OpenWAD: Added \"%s\"%s%s [%u Entries, %s]\n", NewWAD->__Private.__DOSName, (NewWAD->__Private.__IsIWAD ? "*" : ""), (NewWAD->__Private.__IsWAD ? "" : "+"), u32, NewWAD->SimpleSumChars);
	}
	
	/* Run Data Registration */
	for (i = 0; i < l_NumPDC; i++)
		WL_GetPrivateData(NewWAD, l_PDCp[i]->Key, NULL);
		
	/* Return the generated WAD */
	return NewWAD;
#undef SUMBUF
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
		{
			// Anti-Shareware
			if (g_IWADFlags & CIF_SHAREWARE)
			{
				Rover = l_LFirstVWAD;
				
				// Use the WAD after the IWAD
				if (Rover)
					return Rover->NextVWAD;
				else
					return Rover;
			}
			
			// Return the last WAD
			else
				return l_LLastVWAD;
		}
	}
	
	/* Non-null means iterate to the next */
	else
	{
		// Anti-Shareware
		if (g_IWADFlags & CIF_SHAREWARE)
		{
			if (a_Forwards)
			{
				// Return the next one if this is the first
				if (a_WAD == l_LFirstVWAD)
					return a_WAD->NextVWAD;
				else
					return NULL;
			}
			else
			{
				// Return the first WAD if this is the next after the first
				if (a_WAD == l_LFirstVWAD->NextVWAD)
					return l_LFirstVWAD;
				else
					return NULL;
			}
		}
		
		// Normal
		else
		{
			if (a_Forwards)
				return a_WAD->NextVWAD;
			else
				return a_WAD->PrevVWAD;
		}
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
			CONL_PrintF("WL_PushWAD: WAD already linked!\n");
		
		return;
	}
	
	/* Debug */
	if (devparm)
		CONL_PrintF("WL_PushWAD: Pushing \"%s\".\n", a_WAD->__Private.__DOSName);
	
	/* Re-link into chain */
	// No chain at all?
	if (!l_LFirstVWAD)
		l_LLastVWAD = l_LFirstVWAD = (WL_WADFile_t*)a_WAD;
	
	// Chain needs linking
	else
	{
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
			
				// Break out for OCCB
				break;
			}
		
			// Go to next
			Rover = Rover->NextVWAD;
		}
	}
	
	/* Call order change callback */
	// If it is unlocked, then call the callbacks
	if (!l_LockOCCB)
	{
		CB = l_OCCBHead;

		while (CB)
		{
			if (CB->Func)
				if (!CB->Func(true, l_LLastVWAD))
					if (devparm)
						CONL_PrintF("WL_PushWAD: Order callback failed.\n");
			CB = CB->Next;
		}
	}
	
	// Otherwise, set pending
	else
	{
		if (devparm)
			CONL_PrintF("WL_PushWAD: OCCB locked, pending notification.\n");
		
		l_PendingOCCB = true;
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
	// If it is not locked, call them
	if (!l_LockOCCB)
	{
		CB = l_OCCBHead;
	
		while (CB)
		{
			if (CB->Func)
				if (!CB->Func(false, Rover))
					if (devparm)
						CONL_PrintF("WL_PopWAD: Order callback failed.\n");
			CB = CB->Next;
		}
	}
	
	// Otherwise, set pending
	else
	{
		if (devparm)
			CONL_PrintF("WL_PopWAD: OCCB locked, pending notification.\n");
		
		l_PendingOCCB = true;
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
		strncpy(l_SearchList[l_SearchCount++], ".", PATH_MAX);
		strncpy(l_SearchList[l_SearchCount++], "bin", PATH_MAX);
		I_GetStorageDir(l_SearchList[l_SearchCount++], PATH_MAX - 1, DST_DATA);
		
		// -waddir argument
		if (M_CheckParm("-waddir"))
			while (M_IsNextParm())
			{
				// Get directory argument
				DirArg = M_GetNextParm();
				
				// Add to Search
				if (l_SearchCount < MAXSEARCHBUFFER)
					// Copy
					strncpy(l_SearchList[l_SearchCount++], DirArg, PATH_MAX);
			}
		
		// $DOOMWADDIR
		if ((DirArg = I_GetEnvironment("DOOMWADDIR")))
		{
			// Add to search
			if (l_SearchCount < MAXSEARCHBUFFER)
				// Copy
				strncpy(l_SearchList[l_SearchCount++], DirArg, PATH_MAX);
		}
		
		// $DOOMWADPATH
		if ((DirArg = I_GetEnvironment("DOOMWADPATH")))
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
					strncpy(l_SearchList[l_SearchCount++], DirArg, (j < PATH_MAX ? j : PATH_MAX));
					
				// Go to end
				if (End)
					DirArg = End + 1;
				else
					DirArg = NULL;
			}
			while (DirArg);
		}
		
		// Add trailing / to the end of all the searches (except for the first)
		for (i = 1; i < l_SearchCount; i++)
			strncat(l_SearchList[i], "/", PATH_MAX);
			
		// Debug
		if (devparm)
			for (i = 0; i < l_SearchCount; i++)
				CONL_PrintF("WL_LocateWAD: Searching \"%s\".\n", l_SearchList[i]);
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

/* WL_LockOCCB() -- Prevents OCCB from running */
bool_t WL_LockOCCB(const bool_t a_DoLock)
{
	WL_OCCB_t* CB;
	
	/* Locking? */
	if (a_DoLock)
	{
		// Set lock
		l_LockOCCB = true;
		
		// Debug
		if (devparm)
			CONL_PrintF("WL_LockOCCB: Locked.\n");
	}
	
	/* Unlocking */
	else
	{
		// Debug
		if (devparm)
			if (l_PendingOCCB)
				CONL_PrintF("WL_LockOCCB: Unlocked with pending.\n");
			else
				CONL_PrintF("WL_LockOCCB: Unlocked.\n");
		
		// Unset lock
		l_LockOCCB = false;
		
		// Is there a pending OCCB?
		if (l_PendingOCCB)
		{
			// Unset pending
			l_PendingOCCB = false;
			
			// Call all callbacks
			CB = l_OCCBHead;

			while (CB)
			{
				if (CB->Func)
					if (!CB->Func(true, l_LLastVWAD))
						if (devparm)
							CONL_PrintF("WL_LockOCCB: Order callback failed.\n");
				CB = CB->Next;
			}
		}
	}
	
	/* Success! */
	return true;
}

/* WL_RegisterOCCB() -- This is called whenever the WAD order changes */
bool_t WL_RegisterOCCB(WL_OrderCBFunc_t const a_Func, const uint8_t a_Order)
{
	WL_OCCB_t* New;
	WL_OCCB_t* Temp;
	WL_WADFile_t* Rover;
	
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
		New = NULL;
		Temp = l_OCCBHead;
		
		// While there is a current
		while (Temp)
		{
			// Order is lower?
			if (a_Order < Temp->Order)
			{
				// Create and link here
				New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
				
				// If Temp is head
				if (Temp == l_OCCBHead)
				{
					// A simple link before
					New->Next = Temp;
					Temp->Prev = New;
					l_OCCBHead = New;
				}
				
				// Otherwise, re-link inside a chain
				else
				{
					// Re-chain
					New->Prev = Temp->Prev;
					New->Next = Temp;
					if (Temp->Prev)
						Temp->Prev->Next = New;
					Temp->Prev = New;
				}
				
				// break out
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
			Temp = Temp->Next;
		}
	}
	
	/* No new? */
	if (!New)
		return false;	// just in case
	
	/* Set info */
	New->Func = a_Func;
	New->Order = a_Order;
	
	// Call order for all WADs (to simulate order changes)
	Rover = NULL;
	while ((Rover = WL_IterateVWAD(Rover, true)))
		if (!New->Func(true, Rover))
			if (devparm)
				CONL_PrintF("WL_RegisterOCCB: Initial push simulate failed for \"%s\".\n", Rover->__Private.__DOSName);
	
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
		l_PDC[j].Mark = true;
	}
	
	/* Create data for WADs */
	WAD = l_LFirstWAD;
	
	while (WAD)
	{
		// Getting private data means creating it
		WL_GetPrivateData(WAD, a_Key, NULL);
	
		// Go to the previous WAD (since new WADs are prelinked)
		WAD = WAD->__Private.__PrevWAD;
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
			CONL_PrintF("WL_GetPrivateData: Stuff overflow.\n");
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
			CONL_PrintF("WL_GetPrivateData: Private data creation failed!\n");
		
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

/*** ENTRY STREAM ***/
#define WLSTREAMCACHESIZE				4096	// Default size of cache stream

/* WL_ES_s -- Stream for an entry */
struct WL_ES_s
{
	const WL_WADEntry_t* Entry;					// Entry for stream
	
	uint8_t* Cache;								// Stream cache
	uint32_t CacheSize;							// Size of cache
	uint32_t CacheOffset;						// Offset of cache data
	uint32_t CacheRealBase;						// Real offset base
	
	uint32_t StreamOffset;						// Offset of stream
	uint32_t StreamSize;						// Size of stream
	
	bool_t IsUnicode;							// Is UTF16 stream
	bool_t IsSwapped;							// Is byte swapped
	
	char MBBuf[5];								// Multi-byte buffer
	size_t MBLeft;								// Bytes Left
};

/* WL_StreamOpen() -- Opens a stream */
WL_ES_t* WL_StreamOpen(const WL_WADEntry_t* const a_Entry)
{
	WL_ES_t* New;
	
	/* Check */
	if (!a_Entry)
		return NULL;
	
	/* Create a new stream */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Set info */
	New->Entry = a_Entry;
	New->StreamSize = a_Entry->Size;
	New->CacheSize = WLSTREAMCACHESIZE;
	New->Cache = Z_Malloc(New->CacheSize, PU_STATIC, NULL);
	
	/* Read the first bits into the cache */
	WL_ReadData(New->Entry, New->CacheOffset, New->Cache, New->CacheSize);
	
	/* Return stream */
	return New;
}

/* WL_StreamClose() -- Closes a stream */
void WL_StreamClose(WL_ES_t* const a_Stream)
{
	/* Check */
	if (!a_Stream)
		return;
	
	/* Free */
	// Free cache
	if (a_Stream->Cache)
		Z_Free(a_Stream->Cache);
	
	// Free actual stream
	Z_Free(a_Stream);
}

/* WL_StreamGetEntry() -- Get stream's entry */
const WL_WADEntry_t* WL_StreamGetEntry(WL_ES_t* const a_Stream)
{
	/* Check */
	if (!a_Stream)
		return NULL;
	
	/* Return it */
	return a_Stream->Entry;
}

/* WL_StreamTell() -- Return current position of stream */
uint32_t WL_StreamTell(WL_ES_t* const a_Stream)
{
	/* Check */
	if (!a_Stream)
		return 0;
	return a_Stream->StreamOffset;
}

/* WL_StreamSeek() -- Relocate stream */
uint32_t WL_StreamSeek(WL_ES_t* const a_Stream, const uint32_t a_NewPos, const bool_t a_End)
{
	/* Check */
	if (!a_Stream)
		return 0;
	
	/* Which endpoint? */
	// Start
	if (!a_End)
	{
		// After end?
		if (a_NewPos >= a_Stream->StreamSize)
			a_Stream->StreamOffset = a_Stream->StreamSize - 1;
		else
			a_Stream->StreamOffset = a_NewPos;
	}
	
	// End
	else
	{
		// After start?
		if (a_NewPos >= a_Stream->StreamSize)
			a_Stream->StreamOffset = 0;
		else
			a_Stream->StreamOffset = a_Stream->StreamSize - a_NewPos - 1;
	}
	
	/* Return current position */
	return WL_StreamTell(a_Stream);
}

/* WL_StreamEOF() -- Checks end of file */
bool_t WL_StreamEOF(WL_ES_t* const a_Stream)
{
	/* Check */
	if (!a_Stream)
		return true;	// return EOF in this case
	
	/* At the end? */
	if (a_Stream->StreamOffset >= a_Stream->StreamSize)
		return true;
	
	/* Not at end */
	return false;
}

/* WLS_StreamBufferChunk() -- Buffer this offset */
static bool_t WLS_StreamBufferChunk(WL_ES_t* const a_Stream, const size_t a_Offset)
{
	uint32_t WantedChunk;
	
	/* Check */
	if (!a_Stream)
		return false;
	
	/* Determine the chunk we want */
	WantedChunk = a_Offset / a_Stream->CacheSize;
	
	/* Chunk differs? */
	if (WantedChunk != a_Stream->CacheOffset)
	{
		// Clean the old cache
		memset(a_Stream->Cache, 0, a_Stream->CacheSize);
		
		// Set new location chunk
		a_Stream->CacheOffset = WantedChunk;
		
		// Determine the position of where to start reading
		a_Stream->CacheRealBase = WantedChunk * a_Stream->CacheSize;
		
		// Read it in
		WL_ReadData(a_Stream->Entry, a_Stream->CacheRealBase, a_Stream->Cache, a_Stream->CacheSize);
		
		// Success!
		return true;
	}
	
	/* Otherwise if it is the same, do nothing */
	else
		return false;
}

/* WL_StreamRawRead() -- Reads a raw stream */
size_t WL_StreamRawRead(WL_ES_t* const a_Stream, const size_t a_Offset, void* const a_Out, const size_t a_OutSize)
{
#if 0
	size_t Expected;
	
	/* Check */
	if (!a_Stream || !a_Out || !a_OutSize)
		return 0;
	
	return WL_ReadData(a_Stream->Entry, a_Offset, a_Out, a_OutSize);
#elif 1
	uint32_t Left, CurOff, ChunkOff, ChunkCount;
	
	/* Check */
	if (!a_Stream || !a_Out || !a_OutSize)
		return 0;
	
	/* Initialize */
	Left = a_OutSize;
	CurOff = 0;
	
	/* Keep reading until nothing is left */
	while (Left > 0)
	{
		// Buffer in area for stream
		WLS_StreamBufferChunk(a_Stream,  a_Offset + CurOff);
		
		// Determine base chunk to read and how much of it to read
		ChunkOff = (a_Offset + CurOff)	 - a_Stream->CacheRealBase;
		ChunkCount = a_Stream->CacheSize - ChunkOff;
		
		// Exceeds?
		if (ChunkCount > Left)
			ChunkCount = Left;
		
		// Copy
		memmove((void*)(((uintptr_t)a_Out) + CurOff), a_Stream->Cache + ChunkOff, ChunkCount);
		
		// Add offset and remove left
		Left -= ChunkCount;
		CurOff += ChunkCount;
	}
	
	/* Return read count */
	return CurOff;
#else
	size_t Left, Done, ToCopy;
	
	/* Check */
	if (!a_Stream || !a_Out || !a_OutSize)
		return 0;
	
	/* Place data amount that is remaining */
	Done = 0;
	Left = a_OutSize;
	
	/* Determine if the cache needs re-alignment */
	if (Left < a_Stream->CacheOffset || Left >= (a_Stream->CacheOffset + a_Stream->CacheSize))
	{
		// Reset cache offset
		a_Stream->CacheOffset = a_Offset;
		
		// Read in data
		WL_ReadData(a_Stream->Entry, a_Stream->CacheOffset, a_Stream->Cache, a_Stream->CacheSize);
	}
	
	/* Copy current chunk to output */
	// Determine amount to copy
	if (Left > a_Stream->CacheSize)
		ToCopy = a_Stream->CacheSize;
	else
		ToCopy = Left;
	
	// memmove
	memmove(a_Out, a_Stream->Cache, ToCopy);
	
	// Modify numbers
	Done = ToCopy;
	Left -= ToCopy;
	
	/* If there is anything left, recursively call self */
	if (Left > 0)
		Done += WL_StreamRawRead(a_Stream, a_Offset + Done, (void*)(((uintptr_t)a_Out) + Done), Left);
	
	/* Return done */
	return Done;
#endif
}

#define __REMOOD_MACROMERGE(a,b) a##b

#define __REMOOD_WLSTREAMREAD(ww,w,x) x __REMOOD_MACROMERGE(WL_Sr,ww)(WL_ES_t* const a_Stream)\
{\
	x Out = 0;\
	\
	if (!a_Stream)\
		return 0;\
	\
	a_Stream->StreamOffset += WL_StreamRawRead(a_Stream, a_Stream->StreamOffset, &Out, sizeof(Out));\
	\
	return Out;\
}

#define __REMOOD_WLSTREAMLITTLEREAD(ww,w,x) x __REMOOD_MACROMERGE(WL_Srl,ww)(WL_ES_t* const a_Stream)\
{\
	return __REMOOD_MACROMERGE(LittleSwap,w)(__REMOOD_MACROMERGE(WL_Sr,ww)(a_Stream));\
}

__REMOOD_WLSTREAMREAD(i8,Int8,int8_t);
__REMOOD_WLSTREAMREAD(i16,Int16,int16_t);
__REMOOD_WLSTREAMREAD(i32,Int32,int32_t);
__REMOOD_WLSTREAMREAD(u8,UInt8,uint8_t);
__REMOOD_WLSTREAMREAD(u16,UInt16,uint16_t);
__REMOOD_WLSTREAMREAD(u32,UInt32,uint32_t);

__REMOOD_WLSTREAMLITTLEREAD(i16,Int16,int16_t);
__REMOOD_WLSTREAMLITTLEREAD(i32,Int32,int32_t);
__REMOOD_WLSTREAMLITTLEREAD(u16,UInt16,uint16_t);
__REMOOD_WLSTREAMLITTLEREAD(u32,UInt32,uint32_t);

#undef __REMOOD_WLSTREAMREAD
#undef __REMOOD_WLSTREAMLITTLEREAD

#undef __REMOOD_MACROMERGE

/* WL_StreamCheckUnicode() -- Checks if the stream is unicode */
bool_t WL_StreamCheckUnicode(WL_ES_t* const a_Stream)
{
	uint16_t FirstBits;
	
	/* Check */
	if (!a_Stream)
		return false;
		
	/* Reposition */
	WL_StreamSeek(a_Stream, 0, false);
	
	/* Read the first bytes */
	FirstBits = WL_Sru16(a_Stream);
	
	/* Determine if it is Unicode */
	if (FirstBits == 0xFFFE)
		a_Stream->IsUnicode = true;
	else if (FirstBits == 0xFEFF)
		a_Stream->IsUnicode = a_Stream->IsSwapped = true;
	
	/* Reposition to start of text */
	if (!a_Stream->IsUnicode)
		WL_StreamSeek(a_Stream, 0, false);
	else
		WL_StreamSeek(a_Stream, 2, false);
	
	/* Success */
	return true;
}

/* WL_Src() -- Read character from stream */
char WL_Src(WL_ES_t* const a_Stream)
{
	char RetVal;
	uint16_t wcTemp;
	
	/* Check */
	if (!a_Stream)
		return 0;
	
	/* Depending on stream type */
	if (!a_Stream->IsUnicode)
		RetVal = WL_Sru8(a_Stream);
	else
	{
		// If UTF-8 buffer contains data, flush it
		if (a_Stream->MBLeft)
		{
			// Return this character
			RetVal = a_Stream->MBBuf[0];
			
			// Decrease
			memmove(&a_Stream->MBBuf[0], &a_Stream->MBBuf[1], sizeof(*a_Stream->MBBuf) * 4);
			a_Stream->MBLeft--;
			
			// Return
			return RetVal;
		}
		
		// Read next UTF-16 character
		wcTemp = WL_Sru16(a_Stream);
		
		// Swap the bits?
		if (a_Stream->IsSwapped)
			wcTemp = SwapUInt16(RetVal);
		
		// If character is > 127, convert to UTF-8 and flush first char
		if (wcTemp > 127)
		{
			// Convert
			a_Stream->MBLeft = V_ExtWCharToMB(wcTemp, a_Stream->MBBuf);
			
			// Return first character
			if (a_Stream->MBLeft)
			{
				RetVal = a_Stream->MBBuf[0];
			
				// Decrease
				memmove(&a_Stream->MBBuf[0], &a_Stream->MBBuf[1], sizeof(*a_Stream->MBBuf) * 4);
				a_Stream->MBLeft--;
			
				// Return
				return RetVal;
			}
		}
		
		// A normal character
		else
			RetVal = wcTemp;
	}
	
	/* Return */
	return RetVal;
}

/* WL_Srl() -- Read single line from stream */
size_t WL_Srl(WL_ES_t* const a_Stream, char* const a_Buf, const size_t a_Size)
{
	size_t RetVal;
	char Char;
	
	/* Check */
	if (!a_Stream || !a_Buf || !a_Size)
		return 0;
	
	/* Read characters until \n */
	for (RetVal = 0, Char = WL_Src(a_Stream); !WL_StreamEOF(a_Stream); Char = WL_Src(a_Stream))
	{
		// If character is \n, the stream has ended
		if (Char == '\n')
			break;
		
		// Ignore '\r'
		if (Char == '\r')
			continue;
		
		// Place in buffer
		if (RetVal < a_Size - 1)
			a_Buf[RetVal++] = Char;
	}
	
	/* Return retval */
	return RetVal;
}

/**********************
*** WX_ DEPRECATION ***
**********************/

/* WX_EntryForName() -- Returns an entry for a name */
WX_WADEntry_t* __REMOOD_DEPRECATED WX_EntryForName(WX_WADFile_t* const a_WAD, const char* const a_Name, const bool_t a_Forwards)
{
	return WL_FindEntry(a_WAD, (a_Forwards ? WLFF_FORWARDS : 0), a_Name);
}

/* WX_CacheEntry() -- Caches a WX Entry */
void* __REMOOD_DEPRECATED WX_CacheEntry(WX_WADEntry_t* const a_Entry)
{
	/* Check */
	if (!a_Entry)
		return NULL;
	
	/* Does WX Data exist? */
	if (a_Entry->__Private.__WXClone)
		return a_Entry->__Private.__WXClone;
	
	/* Allocate Space in WX area */
	a_Entry->__Private.__WXClone = Z_Malloc(a_Entry->Size, PU_STATIC, &a_Entry->__Private.__WXClone);
	
	// Read data to cloned area
	WL_ReadData(a_Entry, 0, a_Entry->__Private.__WXClone, a_Entry->Size);
	
	/* Return cloned data */
	return a_Entry->__Private.__WXClone;
}

/* WX_GetEntrySize() -- Return size of entry */
size_t __REMOOD_DEPRECATED WX_GetEntrySize(WX_WADEntry_t* const a_Entry)
{
	if (!a_Entry)
		return 0;
	return a_Entry->Size;
}


/* W_InitMultipleFiles() -- Initializes multiple file inputs */
// This is the main code for loading WADs
size_t W_InitMultipleFiles(char** filenames)
{
	size_t i;
	size_t OK;
	const WL_WADFile_t* File;
	
	/* Check */
	if (!filenames)
		return 0;
	
	/* Lock OCCB */
	WL_LockOCCB(true);
	
	/* Run through filename list */
	for (OK = 0, i = 0; filenames[i]; i++)
		if ((File = WL_OpenWAD(filenames[i])))
		{
			// This one worked
			OK++;
			
			// Push to stack
			WL_PushWAD(File);
		}
		
	/* Unlock OCCB */
	WL_LockOCCB(false);
	
	/* Return OK count */
	return OK;
}

