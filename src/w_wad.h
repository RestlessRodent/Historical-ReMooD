// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
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

#ifndef __W_WAD__
#define __W_WAD__

#include "doomtype.h"
#include <stdio.h>
#include <stdlib.h>
#include "z_zone.h"

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
	char *Name;					// Pointer to a string (for Pk3s later on)
	size_t Size;
	size_t Position;
	WadEntryType_t Type;		// File type

	/* Cache Info */
	void* Cache[NUMWADENTRYTYPES];

	/* Parent */
	struct WadFile_s *Host;

} WadEntry_t;

typedef struct WadFile_s
{
	/* Filesystem */
	char *FileName;					// Filename
	size_t NumLumps;				// Number of Lumps
	FILE *File;						// File pointer
	size_t Size;					// Size of the file

	/* Data */
	WadEntry_t *Index;				// WAD's Index

	/* WAD Hack */
	void *WADNameHack;

	/* Special */
	uint32_t Method;				// File Method

	/* Multiplayer Verification */
	uint8_t MD5Sum[16];				// MD5 Sum
	
	/* WAD Specific Data */
	void* Specific;				// Specific Data for said WAD

	/* Links */
	struct WadFile_s *Prev;
	struct WadFile_s *Next;
} WadFile_t;

#define INVALIDLUMP -1
typedef int WadIndex_t;
#define MAXLUMPS 2147483647
#define INDEXSIZET(x) ((int)(x))

size_t		__REMOOD_DEPRECATED W_NumWadFiles(void);
WadFile_t*	__REMOOD_DEPRECATED W_GetWadForNum(size_t Num);
WadFile_t*	__REMOOD_DEPRECATED W_GetWadForName(char *Name);
size_t		__REMOOD_DEPRECATED W_GetNumForWad(WadFile_t * WAD);
WadEntry_t*	__REMOOD_DEPRECATED W_GetEntry(WadIndex_t lump);
WadIndex_t	__REMOOD_DEPRECATED W_LumpsSoFar(WadFile_t * wadid);
WadIndex_t	__REMOOD_DEPRECATED W_InitMultipleFiles(char **filenames);
void		__REMOOD_DEPRECATED W_Shutdown(void);
int			__REMOOD_DEPRECATED W_LoadWadFile(char *filename);
void		__REMOOD_DEPRECATED W_Reload(void);
WadIndex_t	__REMOOD_DEPRECATED W_BiCheckNumForName(char *name, int forwards);
WadIndex_t	__REMOOD_DEPRECATED W_CheckNumForName(char *name);
WadIndex_t	__REMOOD_DEPRECATED W_CheckNumForNamePwad(char *name, size_t wadid, WadIndex_t startlump);
WadIndex_t	__REMOOD_DEPRECATED W_CheckNumForNamePwadPtr(char *name, WadFile_t * wadid, WadIndex_t startlump);
WadIndex_t	__REMOOD_DEPRECATED W_GetNumForName(char *name);
WadIndex_t	__REMOOD_DEPRECATED W_CheckNumForNameFirst(char *name);
WadIndex_t	__REMOOD_DEPRECATED W_GetNumForNameFirst(char *name);
size_t		__REMOOD_DEPRECATED W_LumpLength(WadIndex_t lump);
size_t		__REMOOD_DEPRECATED W_ReadLumpHeader(WadIndex_t lump, void *dest, size_t size);
void		__REMOOD_DEPRECATED W_ReadLump(WadIndex_t lump, void *dest);
void*		__REMOOD_DEPRECATED W_CacheLumpNum(WadIndex_t lump, size_t PU);
void*		__REMOOD_DEPRECATED W_CacheLumpName(char *name, size_t PU);
void*		__REMOOD_DEPRECATED W_CachePatchName(char *name, size_t PU);
void*		__REMOOD_DEPRECATED W_CacheRawAsPic(WadIndex_t lump, int width, int height, size_t tag);	// return a pic_t
void*		__REMOOD_DEPRECATED W_CacheAsConvertableType(WadIndex_t Lump, size_t PU, WadEntryType_t Type, WadEntryType_t From);
void*		__REMOOD_DEPRECATED W_CacheAsConvertableTypeName(char* Name, size_t PU, WadEntryType_t Type, WadEntryType_t From);
WadIndex_t	__REMOOD_DEPRECATED W_GetNumForEntry(WadEntry_t* Entry);

void		__REMOOD_DEPRECATED W_UnloadData(void);
void		__REMOOD_DEPRECATED W_LoadData(void);
bool_t		__REMOOD_DEPRECATED W_FindWad(const char* Name, const char* MD5, char* OutPath, const size_t OutSize);
const char*	__REMOOD_DEPRECATED W_BaseName(const char* Name);

void*		__REMOOD_DEPRECATED W_CachePatchNum(const WadIndex_t Lump, size_t PU);

typedef struct
{
	WadFile_t *WadFile;
	WadIndex_t LumpNum;
	WadIndex_t firstlump;
	WadIndex_t numlumps;
} lumplist_t;

extern WadFile_t *WADFiles;

/****************************
*** EXTENDED WAD HANDLING ***
****************************/

/*** CONSTANTS ***/
/* WX_BuildAction_t -- Build action for WAD */
typedef enum WX_BuildAction_e
{
	WXBA_BUILDWAD,									// Build single WAD
	WXBA_CLEARWAD,									// Clear single WAD
	WXBA_BUILDCOMPOSITE,							// Build WAD composite
	WXBA_CLEARCOMPOSITE,							// Clear WAD composite
	
	NUMWXBUILDACTIONS
} WX_BuildAction_t;

/* WX_DataPrivateID_t -- Private DATA ID Tag */
typedef enum WX_DataPrivateID_e
{
	WXDPID_GCHARS,									// Graphic characters
	WXDPID_MENU,									// Menu Junk
	WXDPID_RMOD,									// RMOD Table
	
	NUMWXDATAPRIVATEIDS
}  WX_DataPrivateID_t;

/*** STRUCTURES ***/
typedef struct WX_WADFile_s WX_WADFile_t;
typedef struct WX_WADEntry_s WX_WADEntry_t;

/*** PROTOTYPES ***/
uint32_t			__REMOOD_DEPRECATED WX_Hash(const char* const a_Str);
const char*			__REMOOD_DEPRECATED WX_BaseName(const char* const a_File);
const char*			__REMOOD_DEPRECATED WX_BaseExtension(const char* const a_File);
bool_t				__REMOOD_DEPRECATED WX_Init(void);
bool_t				__REMOOD_DEPRECATED WX_LocateWAD(const char* const a_Name, const char* const a_MD5, char* const a_OutPath, const size_t a_OutSize);
WX_WADFile_t*		__REMOOD_DEPRECATED WX_LoadWAD(const char* const a_AutoPath);
void				__REMOOD_DEPRECATED WX_UnLoadWAD(WX_WADFile_t* const a_WAD);
WX_WADFile_t*		__REMOOD_DEPRECATED WX_RoveWAD(WX_WADFile_t* const a_WAD, const bool_t a_Virtual, const int32_t a_Next);
void				__REMOOD_DEPRECATED WX_PreEntryTable(WX_WADFile_t* const a_WAD, const size_t a_Count);
WX_WADEntry_t*		__REMOOD_DEPRECATED WX_AddEntry(WX_WADFile_t* const a_WAD);
void				__REMOOD_DEPRECATED WX_WipeEntryTable(WX_WADFile_t* const a_WAD);
void				__REMOOD_DEPRECATED WX_LoadWADStuff(WX_WADFile_t* const a_WAD);
void				__REMOOD_DEPRECATED WX_ClearWADStuff(WX_WADFile_t* const a_WAD);
void				__REMOOD_DEPRECATED WX_CompileComposite(void);
void				__REMOOD_DEPRECATED WX_ClearComposite(void);
WX_WADEntry_t*		__REMOOD_DEPRECATED WX_GetNumEntry(WX_WADFile_t* const a_WAD, const size_t a_Index);
WX_WADEntry_t*		__REMOOD_DEPRECATED WX_EntryForName(WX_WADFile_t* const a_WAD, const char* const a_Name, const bool_t a_Forwards);
void*				__REMOOD_DEPRECATED WX_CacheEntry(WX_WADEntry_t* const a_Entry);
size_t				__REMOOD_DEPRECATED WX_UseEntry(WX_WADEntry_t* const a_Entry, const bool_t a_Use);
bool_t				__REMOOD_DEPRECATED WX_VirtualPushPop(WX_WADFile_t* const a_WAD, const bool_t a_Pop, const bool_t a_Back);
bool_t				__REMOOD_DEPRECATED WX_GetVirtualPrivateData(WX_WADFile_t* const a_WAD, const WX_DataPrivateID_t a_ID, void*** const a_PPPtr, size_t** const a_PPSize);
WX_WADEntry_t*		__REMOOD_DEPRECATED WX_RoveEntry(WX_WADEntry_t* const a_Entry, const int32_t a_Next);
size_t				__REMOOD_DEPRECATED WX_GetEntryName(WX_WADEntry_t* const a_Entry, char* const a_OutBuf, const size_t a_OutSize);
size_t				__REMOOD_DEPRECATED WX_GetEntrySize(WX_WADEntry_t* const a_Entry);
size_t				__REMOOD_DEPRECATED WX_ClearUnused(void);

/************************
*** LITE WAD HANDLING ***
************************/

/*** CONSTANTS ***/
#define WLMAXENTRYNAME			24				// Max characters in entry name 
#define WLMAXPRIVATEWADSTUFF	64				// Maximum number of private WAD Stuffs
#define WLMAXDOSNAME			13				// nnnnnnnn.xxx\0

/* WL_DataKeys_t -- Keys for data handling, shortcuts */
// Helps prevent magic
typedef enum WL_DataKeys_e
{
	WLDK_RMOD					= 0x00000000U,	// ReMooD Map Object Data
	WLDK_FLATS					= 0x00000000U,	// Floor Textures (Flats)
	WLDK_TEXTURES				= 0x00000000U,	// Wall Textures
	WLDK_PATCHES				= 0x00000000U,	// Wall Patches
	WLDK_SPRITES				= 0x00000000U,	// Sprites
	WLDK_SKINS					= 0x00000000U,	// Skins
	WLDK_SOUNDS					= 0x00000000U,	// Sounds
	WLDK_MUSIC					= 0x00000000U,	// Music
	WLDK_UNICODE				= 0x00000000U,	// Unicode Related Data
	WLDK_LANGUAGE				= 0x00000000U,	// Language Translations
	WLDK_SCRIPTS				= 0x00000000U,	// Virtual Machine Scripts
	WLDK_MAPINFO				= 0x00000000U,	// Map Info
} WL_DataKeys_t;

/* WL_FindFlags_t -- Flags when finding things */
typedef enum WL_FindFlags_e
{
	WLFF_DEFAULTS				= 0,			// Default stuff
	WLFF_FORWARDS				= 0x0001,		// Search forwards instead of backwards
	WLFF_SINGLEWAD				= 0x0002,		// Only search in this WAD (instead of bleeding)
	WLFF_REALCHAINS				= 0x0004,		// Use real chains (DEBUG USE ONLY!)
	WLFF_UNLINKED				= 0x0008,		// Search non-linked WADs, i.e. not in virtual chain (DEBUG USE ONLY!)
	WLFF_IMPORTANT				= 0x0010,		// I_Error if not found (not recommended!)
} WL_FindFlags_t;

/*** STRUCTURES ***/
struct WL_WADEntry_s;
struct WL_WADFile_s;

typedef void (*WL_RemoveFunc_t)(const struct WL_WADFile_s* a_WAD);
typedef bool_t (*WL_PCCreatorFunc_t)(const struct WL_WADFile_s* const a_WAD, const uint32_t a_Key, void** const a_DataPtr, size_t** const a_SizePtr, WL_RemoveFunc_t* const a_RemoveFuncPtr);

/* WL_WADEntry_t -- A lite WAD entry */
typedef struct WL_WADEntry_s
{
	/* Private Stuff You Don't Touch */
	struct
	{
		bool_t __Compressed;					// Entry is compressed
		int32_t __UsageCount;					// Times entry used
		void* __Data;							// Loaded Data (cached)
		size_t __Offset;						// Offset of the internal data
		size_t __InternalSize;					// Internal size (could be compressed)
	} __Private;								// Don't mess with me
	
	/* Public Stuff You Read From */
	// Normal Stuff
	char Name[WLMAXENTRYNAME];					// Name of entry
	uint32_t Hash;								// Hash for this entry
	WadIndex_t Index;							// Index for this entry (WAD Local)
	WadIndex_t GlobalIndex;						// Global Index for this entry (of all WADs)
	
	// Data Related
	size_t Size;								// Size of the data
	
	// Owner
	struct WL_WADFile_s* Owner;					// WAD that owns this
	
	// Linkage
	struct WL_WADEntry_s* PrevEntry;			// Previous Entry
	struct WL_WADEntry_s* NextEntry;			// Next Entry
} WL_WADEntry_t;

/* WL_WADFile_t -- A lite WAD file */
typedef struct WL_WADFile_s
{
	/* Private Stuff You Don't Touch */
	struct
	{
		// Validity
		bool_t __IsValid;						// Is this WAD valid?
		
		// File related stuff
		char __PathName[PATH_MAX];				// Path to WAD File
		char __FileName[PATH_MAX];				// File name of WAD
		char __DOSName[WLMAXDOSNAME];			// DOS Name of the WAD
		void* __CFile;							// File on disk
		size_t __IndexOff;						// Offset of index
		size_t __Size;							// Size of WAD
		
		// Entries
		uint32_t __TopHash;						// Top 8-bits of hash (indexes)
		Z_HashTable_t* __EntryHashes;			// Hash table for find speed
		
		// Public Data (per WAD basis)
		struct
		{
			size_t __NumStuffs;					// Amount of stuff there is
			struct
			{
				uint32_t __Key;					// Key to the data
				void* __DataPtr;				// Pointer to data
				size_t __Size;					// Size of data
				WL_RemoveFunc_t __RemoveFunc;	// Function that removes the data loaded (when WAD unloaded)
			} __Stuff[WLMAXPRIVATEWADSTUFF];	// Private Stuff
		} __PublicData;							// Public data for perWAD data storage
		
		// Linkage
		bool_t __Linked;						// Is the WAD virtually linked?
		struct WL_WADFile_s* __PrevWAD;			// Previous WAD
		struct WL_WADFile_s* __NextWAD;			// Next WAD
	} __Private;								// Don't mess with me
	
	/* Public Stuff You Read From */
	// Data Related Stuff
	size_t TotalSize;							// Size of all entries added together
	uint32_t CheckSum[4];						// MD5 Sum
	uint8_t CheckSumChars[33];					// MD5 Sum (As hex characters)
	
	// Entries
	WL_WADEntry_t* Entries;						// Entries in the WAD
	WadIndex_t NumEntries;						// Number of entries
	
	// Virtual Linkage
	struct WL_WADFile_s* PrevVWAD;				// Previous virtual WAD
	struct WL_WADFile_s* NextVWAD;				// Next virtual WAD
} WL_WADFile_t;

/*** PROTOTYPES ***/
// WAD Handling
const WL_WADFile_t*		WL_OpenWAD(const char* const a_PathName);
void					WL_CloseWAD(const WL_WADFile_t* const a_WAD);
bool_t					WL_LocateWAD(const char* const a_Name, const char* const a_MD5, char* const a_OutPath, const size_t a_OutSize);

void					WL_PushWAD(const WL_WADFile_t* const a_WAD);
const WL_WADFile_t*		WL_PopWAD(void);

bool_t					WL_RegisterPDC(const uint32_t a_Key, const uint8_t a_Order, WL_PCCreatorFunc_t const a_CreatorFunc);
void*					WL_GetPrivateData(const WL_WADFile_t* const a_WAD, const uint32_t a_Key, size_t* const a_SizePtr);

// Entry Handling
const WL_WADEntry_t*	WL_FindEntry(const WL_WADFile_t* const a_BaseSearch, const uint32_t a_Flags, const char* const a_Name);
uintptr_t				WL_TranslateEntry(const WadIndex_t a_GlobalIndex, const WL_WADFile_t* const a_Entry);
size_t					WL_ReadData(const WL_WADEntry_t* const a_Entry, const size_t a_Offset, void* const a_Out, const size_t a_OutSize);

#endif							/* __W_WAD__ */

