// -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
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
// DESCRIPTION: WAD I/O functions, wad resource definitions (some).

#ifndef __W_WAD_H__
#define __W_WAD_H__

#include "doomtype.h"
#include "z_zone.h"
#include "d_block.h"

/*************
*** GLOBAL ***
*************/

#define INVALIDLUMP -1
typedef int WadIndex_t;

/************************
*** LITE WAD HANDLING ***
************************/

/*** CONSTANTS ***/
#define WLMAXENTRYNAME			24	// Max characters in entry name
#define WLMAXPRIVATEWADSTUFF	32	// Maximum number of private WAD Stuffs
#define WLMAXDOSNAME			13	// nnnnnnnn.xxx\0

/* WL_DataPDCOrder_t -- Orders for data handling (PDCs) */
typedef enum WL_DataPDCOrder_e
{
	WLDPO_VIMAGES = 20,				// Image_t
	WLDPO_RMOD = 30,				// ReMooD Map Object Data
	WLDPO_VFONTS = 75,				// Fonts
	WLDPO_TEXTURES = 100,			// Textures
	WLDPO_MAPINFO = 125,			// MAPINFO
} WL_DataPDCOrder_t;

/* WL_DataOCCBOrder_t -- Orders for data handling (OCCBs) */
typedef enum WL_DataOCCBOrder_e
{
	WLDCO_IWADDETECT = 1,			// IWAD Detection
	WLDCO_RWADDETECT = 2,			// ReMooD.WAD Detection
	WLDCO_RMOD = 30,				// ReMooD Map Object Data
	WLDCO_VFONTS = 75,				// Fonts
	WLDCO_TEXTURES = 75,			// Textures
	WLDCO_EXTRASPECIALS = 100,		// Extra Special Stuff
} WL_DataOCCBOrder_t;

/* WL_DataKeys_t -- Keys for data handling, shortcuts */
// Helps prevent magic
typedef enum WL_DataKeys_e
{
	WLDK_RMOD = 0x524D4F44U,		// ReMooD Map Object Data
	WLDK_VIMAGES = 0x8C064303U,		// Image_t
	WLDK_VFONTS = 0x464F4E54U,		// Fonts
	WLDK_MAPINFO = 0x4F464E49U,		// Map Information
	WLDK_TEXTURES = 0x72547854U,	// Textures
} WL_DataKeys_t;

/* WL_FindFlags_t -- Flags when finding things */
typedef enum WL_FindFlags_e
{
	WLFF_DEFAULTS = 0,			// Default stuff
	WLFF_FORWARDS = 0x0001,		// Search forwards instead of backwards
	WLFF_IMPORTANT = 0x0010,	// I_Error if not found (not recommended!)
} WL_FindFlags_t;

/*** STRUCTURES ***/
struct WL_WADEntry_s;
struct WL_WADFile_s;

// WAD Data loading
typedef void (*WL_RemoveFunc_t)(const struct WL_WADFile_s* a_WAD);
typedef bool (*WL_PCCreatorFunc_t)(const struct WL_WADFile_s* const a_WAD, const uint32_t a_Key, void** const a_DataPtr, size_t* const a_SizePtr, WL_RemoveFunc_t* const a_RemoveFuncPtr);

// Order Change
typedef bool (*WL_OrderCBFunc_t)(const bool a_Pushed, const struct WL_WADFile_s* const a_WAD);

/* WL_WADEntry_t -- A lite WAD entry */
typedef struct WL_WADEntry_s
{
	/* Private Stuff You Don't Touch */
	struct
	{
		bool __Compressed;	// Entry is compressed
		int32_t __UsageCount;	// Times entry used
		void* __Data;			// Loaded Data (cached)
		size_t __Offset;		// Offset of the internal data
		size_t __InternalSize;	// Internal size (could be compressed)
		
		void* __WXClone;		// Cloned data for WX
	} __Private;				// Don't mess with me
	
	/* Public Stuff You Read From */
	// Normal Stuff
	char Name[WLMAXENTRYNAME];	// Name of entry
	uint32_t Hash;				// Hash for this entry
	WadIndex_t Index;			// Index for this entry (WAD Local)
	WadIndex_t GlobalIndex;		// Global Index for this entry (of all WADs)
	
	// Data Related
	size_t Size;				// Size of the data
	
	// Owner
	struct WL_WADFile_s* Owner;	// WAD that owns this
	
	// Linkage
	struct WL_WADEntry_s* PrevEntry;	// Previous Entry
	struct WL_WADEntry_s* NextEntry;	// Next Entry
} WL_WADEntry_t;

/* WL_WADFile_t -- A lite WAD file */
typedef struct WL_WADFile_s
{
	/* Private Stuff You Don't Touch */
	struct
	{
		// Validity
		bool __IsValid;							// Is this WAD valid?
		bool __IsWAD;							// Is this a WAD?
		bool __IsIWAD;							// Is this an IWAD?
		
		// File related stuff
		char __PathName[PATH_MAX];				// Path to WAD File
		char __FileName[PATH_MAX];				// File name of WAD
		char __DOSName[WLMAXDOSNAME];			// DOS Name of the WAD
		FileStream_c* __CFile;					// File on disk
		size_t __IndexOff;						// Offset of index
		size_t __Size;							// Size of WAD
		
		// Entries
		uint32_t __TopHash;		// Top 8-bits of hash (indexes)
		Z_HashTable_t* __EntryHashes;	// Hash table for find speed
		
		// Public Data (per WAD basis)
		struct
		{
			size_t __NumStuffs;	// Amount of stuff there is
			struct
			{
				uint32_t __Key;	// Key to the data
				void* __DataPtr;	// Pointer to data
				size_t __Size;	// Size of data
				WL_RemoveFunc_t __RemoveFunc;	// Function that removes the data loaded (when WAD unloaded)
			} __Stuff[WLMAXPRIVATEWADSTUFF];	// Private Stuff
		} __PublicData;			// Public data for perWAD data storage
		
		// Linkage
		bool __Linked;		// Is the WAD virtually linked?
		struct WL_WADFile_s* __PrevWAD;	// Previous WAD
		struct WL_WADFile_s* __NextWAD;	// Next WAD
	} __Private;				// Don't mess with me
	
	/* Public Stuff You Read From */
	// Data Related Stuff
	size_t TotalSize;			// Size of all entries added together
	uint32_t CheckSum[4];		// MD5 Sum
	char CheckSumChars[33];		// MD5 Sum (As hex characters)
	uint32_t SimpleSum[4];		// Simple Sum
	char SimpleSumChars[33];		// Simple Sum (As hex characters)
	
	// Entries
	WL_WADEntry_t* Entries;		// Entries in the WAD
	WadIndex_t NumEntries;		// Number of entries
	
	// Virtual Linkage
	struct WL_WADFile_s* PrevVWAD;	// Previous virtual WAD
	struct WL_WADFile_s* NextVWAD;	// Next virtual WAD
} WL_WADFile_t;

// Streamer
typedef struct WL_EntryStream_s WL_EntryStream_t;

/*** PROTOTYPES ***/
const char* WL_BaseNameEx(const char* const a_File);

// WAD Handling
void WL_Init(void);
const WL_WADFile_t* WL_OpenWAD(const char* const a_PathName);
void WL_CloseWAD(const WL_WADFile_t* const a_WAD);
bool WL_LocateWAD(const char* const a_Name, const char* const a_MD5, char* const a_OutPath, const size_t a_OutSize);

const char* const WL_GetWADName(const WL_WADFile_t* const a_WAD, const bool a_NonDOSName);
const WL_WADFile_t* WL_IterateVWAD(const WL_WADFile_t* const a_WAD, const bool a_Forwards);

void WL_PushWAD(const WL_WADFile_t* const a_WAD);
const WL_WADFile_t* WL_PopWAD(void);

bool WL_LockOCCB(const bool a_DoLock);
bool WL_RegisterOCCB(WL_OrderCBFunc_t const a_Func, const uint8_t a_Order);

bool WL_RegisterPDC(const uint32_t a_Key, const uint8_t a_Order, WL_PCCreatorFunc_t const a_CreatorFunc, WL_RemoveFunc_t const a_RemoveFunc);
void* WL_GetPrivateData(const WL_WADFile_t* const a_WAD, const uint32_t a_Key, size_t* const a_SizePtr);

// Entry Handling
const WL_WADEntry_t* WL_FindEntry(const WL_WADFile_t* const a_BaseSearch, const uint32_t a_Flags, const char* const a_Name);
uintptr_t WL_TranslateEntry(const WadIndex_t a_GlobalIndex, const WL_WADFile_t* const a_Entry);
size_t WL_ReadData(const WL_WADEntry_t* const a_Entry, const size_t a_Offset, void* const a_Out, const size_t a_OutSize);

/* WLEntryStream_c -- Stream that handles WL WAD Entries */
class WLEntryStream_c : public GenericByteStream_c
{
	private:
		const WL_WADEntry_t* p_Entry;			// Entry for stream
	
		uint8_t* p_Cache;						// Stream cache
		uint32_t p_CacheSize;					// Size of cache
		uint32_t p_CacheOffset;					// Offset of cache data
		uint32_t p_CacheRealBase;				// Real offset base
	
		uint32_t p_StreamOffset;				// Offset of stream
		uint32_t p_StreamSize;					// Size of stream
		
		bool BufferChunk(const size_t a_Offset);
		
	public:
		WLEntryStream_c(const WL_WADEntry_t* a_Entry);
		~WLEntryStream_c();
		
		bool Seekable(void);
		bool AutoSeek(void);
		bool EndOfStream(void);
		uint64_t Tell(void);
		uint64_t Seek(const uint64_t a_NewPos, const bool a_AtEnd = false);
		size_t ReadChunk(void* const a_Data, const size_t a_Size);
		size_t WriteChunk(const void* const a_Data, const size_t a_Size);
		
		const WL_WADEntry_t* GetEntry(void);
};

/****************************
*** EXTENDED WAD HANDLING ***
****************************/

/*** STRUCTURES ***/
// Since I hid WX_ from the start, they can easily adapt to WL!
typedef WL_WADFile_t WX_WADFile_t;
typedef WL_WADEntry_t WX_WADEntry_t;

/*** PROTOTYPES ***/
WX_WADEntry_t* __REMOOD_DEPRECATED WX_EntryForName(WX_WADFile_t* const a_WAD, const char* const a_Name, const bool a_Forwards);
void* __REMOOD_DEPRECATED WX_CacheEntry(WX_WADEntry_t* const a_Entry);
size_t __REMOOD_DEPRECATED WX_GetEntrySize(WX_WADEntry_t* const a_Entry);

/*** THE LAST REMAINING FUNCTION ***/
size_t W_InitMultipleFiles(char** filenames);

#endif							/* __W_WAD_H__ */

