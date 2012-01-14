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
// DESCRIPTION: WAD I/O functions, wad resource definitions (some).

#ifndef __W_WAD_H__
#define __W_WAD_H__

#include "doomtype.h"
#include <stdio.h>
#include <stdlib.h>
#include "z_zone.h"

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
	WLDPO_MAPINFO = 125,			// MAPINFO
} WL_DataPDCOrder_t;

/* WL_DataOCCBOrder_t -- Orders for data handling (OCCBs) */
typedef enum WL_DataOCCBOrder_e
{
	WLDCO_RMOD = 30,				// ReMooD Map Object Data
	WLDCO_VFONTS = 75,				// Fonts
} WL_DataOCCBOrder_t;

/* WL_DataKeys_t -- Keys for data handling, shortcuts */
// Helps prevent magic
typedef enum WL_DataKeys_e
{
	WLDK_RMOD = 0x524D4F44U,		// ReMooD Map Object Data
	WLDK_VIMAGES = 0x8C064303U,		// Image_t
	WLDK_VFONTS = 0x464F4E54U,		// Fonts
	WLDK_MAPINFO = 0x4F464E49U,		// Map Information
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
typedef bool_t (*WL_PCCreatorFunc_t)(const struct WL_WADFile_s* const a_WAD, const uint32_t a_Key, void** const a_DataPtr, size_t* const a_SizePtr, WL_RemoveFunc_t* const a_RemoveFuncPtr);

// Order Change
typedef bool_t (*WL_OrderCBFunc_t)(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD);

/* WL_WADEntry_t -- A lite WAD entry */
typedef struct WL_WADEntry_s
{
	/* Private Stuff You Don't Touch */
	struct
	{
		bool_t __Compressed;	// Entry is compressed
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
		bool_t __IsValid;		// Is this WAD valid?
		bool_t __IsWAD;			// Is this a WAD?
		bool_t __IsIWAD;		// Is this an IWAD?
		
		// File related stuff
		char __PathName[PATH_MAX];	// Path to WAD File
		char __FileName[PATH_MAX];	// File name of WAD
		char __DOSName[WLMAXDOSNAME];	// DOS Name of the WAD
		void* __CFile;			// File on disk
		size_t __IndexOff;		// Offset of index
		size_t __Size;			// Size of WAD
		
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
		bool_t __Linked;		// Is the WAD virtually linked?
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
const WL_WADFile_t* WL_OpenWAD(const char* const a_PathName);
void WL_CloseWAD(const WL_WADFile_t* const a_WAD);
bool_t WL_LocateWAD(const char* const a_Name, const char* const a_MD5, char* const a_OutPath, const size_t a_OutSize);

const char* const WL_GetWADName(const WL_WADFile_t* const a_WAD, const bool_t a_NonDOSName);
const WL_WADFile_t* WL_IterateVWAD(const WL_WADFile_t* const a_WAD, const bool_t a_Forwards);

void WL_PushWAD(const WL_WADFile_t* const a_WAD);
const WL_WADFile_t* WL_PopWAD(void);

bool_t WL_LockOCCB(const bool_t a_DoLock);
bool_t WL_RegisterOCCB(WL_OrderCBFunc_t const a_Func, const uint8_t a_Order);

bool_t WL_RegisterPDC(const uint32_t a_Key, const uint8_t a_Order, WL_PCCreatorFunc_t const a_CreatorFunc, WL_RemoveFunc_t const a_RemoveFunc);
void* WL_GetPrivateData(const WL_WADFile_t* const a_WAD, const uint32_t a_Key, size_t* const a_SizePtr);

// Entry Handling
const WL_WADEntry_t* WL_FindEntry(const WL_WADFile_t* const a_BaseSearch, const uint32_t a_Flags, const char* const a_Name);
uintptr_t WL_TranslateEntry(const WadIndex_t a_GlobalIndex, const WL_WADFile_t* const a_Entry);
size_t WL_ReadData(const WL_WADEntry_t* const a_Entry, const size_t a_Offset, void* const a_Out, const size_t a_OutSize);

// WAD Stream Buffer
WL_EntryStream_t* WL_StreamOpen(const WL_WADEntry_t* const a_Entry);
void WL_StreamClose(WL_EntryStream_t* const a_Stream);

uint32_t WL_StreamTell(WL_EntryStream_t* const a_Stream);
uint32_t WL_StreamSeek(WL_EntryStream_t* const a_Stream, const uint32_t a_NewPos, const bool_t a_End);

size_t WL_StreamRawRead(WL_EntryStream_t* const a_Stream, const size_t a_Offset, void* const a_Out, const size_t a_OutSize);

int8_t WL_StreamReadInt8(WL_EntryStream_t* const a_Stream);
int16_t WL_StreamReadInt16(WL_EntryStream_t* const a_Stream);
int32_t WL_StreamReadInt32(WL_EntryStream_t* const a_Stream);
uint8_t WL_StreamReadUInt8(WL_EntryStream_t* const a_Stream);
uint16_t WL_StreamReadUInt16(WL_EntryStream_t* const a_Stream);
uint32_t WL_StreamReadUInt32(WL_EntryStream_t* const a_Stream);

int16_t WL_StreamReadLittleInt16(WL_EntryStream_t* const a_Stream);
int32_t WL_StreamReadLittleInt32(WL_EntryStream_t* const a_Stream);
uint16_t WL_StreamReadLittleUInt16(WL_EntryStream_t* const a_Stream);
uint32_t WL_StreamReadLittleUInt32(WL_EntryStream_t* const a_Stream);

/******************************
*** OLD REMOOD WAD HANDLING ***
******************************/

#define MAX_WADPATH   128
#define MAX_WADFILES  32

enum
{
	METHOD_DEHACKED,			// DeHackEd!
	METHOD_WAD,					// WAD!
	
	// TODO METHODS
	METHOD_RLEWAD,				// RWAD
	METHOD_PKZIP,				// ZIP ARCHIVE
	METHOD_SEVENZIP,			// 7Z ARCHIVE
	METHOD_TAR,					// TAR ARCHIVE
	
	MAXMETHODS
};

struct WadFile_s;

typedef enum
{
	WETYPE_RAW,
	
	/* Image */
	WETYPE_PICTURETYPESTART,
	
	WETYPE_PATCHT = WETYPE_PICTURETYPESTART,
	WETYPE_PICT,
	WETYPE_FLAT,
	
	WETYPE_PICTURETYPEEND = WETYPE_FLAT,
	
	/* Text */
	WETYPE_TEXTTYPESTART,
	
	WETYPE_AUTOTEXT = WETYPE_TEXTTYPESTART,
	WETYPE_WCHART,
	
	WETYPE_TEXTTYPEEND = WETYPE_WCHART,
	
	NUMWADENTRYTYPES
} WadEntryType_t;

typedef struct WadEntry_s
{
	/* Basic */
	char* Name;					// Pointer to a string (for Pk3s later on)
	size_t Size;
	size_t Position;
	WadEntryType_t Type;		// File type
	
	/* Cache Info */
	void* Cache[NUMWADENTRYTYPES];
	
	/* Parent */
	struct WadFile_s* Host;
	
	/* Deprecation */
	WL_WADEntry_t* DepEntry;	// Deprecated Entry
	WadIndex_t DepIndex;		// ID of this entry
	
} WadEntry_t;

typedef struct WadFile_s
{
	/* Filesystem */
	char* FileName;				// Filename
	size_t NumLumps;			// Number of Lumps
	FILE* File;					// File pointer
	size_t Size;				// Size of the file
	
	/* Data */
	WadEntry_t* Index;			// WAD's Index
	
	/* WAD Hack */
	void* WADNameHack;
	
	/* Special */
	uint32_t Method;			// File Method
	
	/* Multiplayer Verification */
	uint8_t MD5Sum[16];			// MD5 Sum
	
	/* WAD Specific Data */
	void* Specific;				// Specific Data for said WAD
	
	/* Links */
	struct WadFile_s* Prev;
	struct WadFile_s* Next;
	
	/* Deprecation */
	WL_WADFile_t* DepWAD;		// Deprecated WAD
} WadFile_t;

#define MAXLUMPS 2147483647
#define INDEXSIZET(x) ((int)(x))

size_t __REMOOD_DEPRECATED W_NumWadFiles(void);
WadFile_t* __REMOOD_DEPRECATED W_GetWadForNum(size_t Num);
WadFile_t* __REMOOD_DEPRECATED W_GetWadForName(char* Name);
size_t __REMOOD_DEPRECATED W_GetNumForWad(WadFile_t* WAD);
WadEntry_t* __REMOOD_DEPRECATED W_GetEntry(WadIndex_t lump);
WadIndex_t __REMOOD_DEPRECATED W_LumpsSoFar(WadFile_t* wadid);
WadIndex_t __REMOOD_DEPRECATED W_InitMultipleFiles(char** filenames);
void __REMOOD_DEPRECATED W_Shutdown(void);
int __REMOOD_DEPRECATED W_LoadWadFile(char* filename);
void __REMOOD_DEPRECATED W_Reload(void);
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
bool_t __REMOOD_DEPRECATED W_FindWad(const char* Name, const char* MD5, char* OutPath, const size_t OutSize);
const char* __REMOOD_DEPRECATED W_BaseName(const char* Name);

void* __REMOOD_DEPRECATED W_CachePatchNum(const WadIndex_t Lump, size_t PU);

typedef struct
{
	WadFile_t* WadFile;
	WadIndex_t LumpNum;
	WadIndex_t firstlump;
	WadIndex_t numlumps;
} lumplist_t;

extern WadFile_t* WADFiles;

/****************************
*** EXTENDED WAD HANDLING ***
****************************/

/*** CONSTANTS ***/

/* WX_BuildAction_t -- Build action for WAD */
typedef enum WX_BuildAction_e
{
	WXBA_BUILDWAD,				// Build single WAD
	WXBA_CLEARWAD,				// Clear single WAD
	WXBA_BUILDCOMPOSITE,		// Build WAD composite
	WXBA_CLEARCOMPOSITE,		// Clear WAD composite
	
	NUMWXBUILDACTIONS
} WX_BuildAction_t;

/* WX_DataPrivateID_t -- Private DATA ID Tag */
typedef enum WX_DataPrivateID_e
{
	WXDPID_GCHARS,				// Graphic characters
	WXDPID_MENU,				// Menu Junk
	WXDPID_RMOD,				// RMOD Table
	
	NUMWXDATAPRIVATEIDS
} WX_DataPrivateID_t;

/*** STRUCTURES ***/
// Since I hid WX_ from the start, they can easily adapt to WL!
typedef WL_WADFile_t WX_WADFile_t;
typedef WL_WADEntry_t WX_WADEntry_t;

/*** PROTOTYPES ***/
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
size_t __REMOOD_DEPRECATED WX_GetEntryName(WX_WADEntry_t* const a_Entry, char* const a_OutBuf, const size_t a_OutSize);
size_t __REMOOD_DEPRECATED WX_GetEntrySize(WX_WADEntry_t* const a_Entry);
size_t __REMOOD_DEPRECATED WX_ClearUnused(void);

#endif							/* __W_WAD_H__ */

