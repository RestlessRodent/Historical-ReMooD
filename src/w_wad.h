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
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2011 GhostlyDeath (ghostlydeath@gmail.com)
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

#ifdef __GNUG__
#pragma interface
#endif

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
boolean		__REMOOD_DEPRECATED W_FindWad(const char* Name, const char* MD5, char* OutPath, const size_t OutSize);
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
/* WX_ConvType_t -- Convertable type */
typedef enum WX_ConvType_e
{
	WXCT_RAW,										// Raw Data
	WXCT_PATCH,										// patch_t
	WXCT_PIC,										// pic_t
	
	NUMWXCONVTYPES
} WX_ConvType_t;

/* WX_DataPrivateID_t -- Private DATA ID Tag */
typedef enum WX_DataPrivateID_e
{
	WXDPID_GCHARS,									// Graphic characters
	
	NUMWXDATAPRIVATEIDS
}  WX_DataPrivateID_t;

/*** STRUCTURES ***/
typedef struct WX_WADFile_s WX_WADFile_t;
typedef struct WX_WADEntry_s WX_WADEntry_t;

/*** PROTOTYPES ***/
uint32_t			WX_Hash(const char* const a_Str);
const char*			WX_BaseName(const char* const a_File);
const char*			WX_BaseExtension(const char* const a_File);
boolean				WX_Init(void);
boolean				WX_LocateWAD(const char* const a_Name, const char* const a_MD5, char* const a_OutPath, const size_t a_OutSize);
WX_WADFile_t*		WX_LoadWAD(const char* const a_AutoPath);
void				WX_UnLoadWAD(WX_WADFile_t* const a_WAD);
void				WX_PreEntryTable(WX_WADFile_t* const a_WAD, const size_t a_Count);
WX_WADEntry_t*		WX_AddEntry(WX_WADFile_t* const a_WAD);
void				WX_WipeEntryTable(WX_WADFile_t* const a_WAD);
void				WX_LoadWADStuff(WX_WADFile_t* const a_WAD);
void				WX_ClearWADStuff(WX_WADFile_t* const a_WAD);
void				WX_CompileComposite(void);
void				WX_ClearComposite(void);
WX_WADEntry_t*		WX_GetNumEntry(WX_WADFile_t* const a_WAD, const size_t a_Index);
WX_WADEntry_t*		WX_EntryForName(WX_WADFile_t* const a_WAD, const char* const a_Name, const boolean a_Forwards);
void*				WX_CacheEntry(WX_WADEntry_t* const a_Entry, const WX_ConvType_t a_From, const WX_ConvType_t a_To);
void*				WX_UseEntry(WX_WADEntry_t* const a_Entry, const WX_ConvType_t a_Type, const boolean a_Use);
boolean				WX_VirtualPushPop(WX_WADFile_t* const a_WAD, const boolean a_Pop, const boolean a_Back);
boolean				WX_GetVirtualPrivateData(WX_WADFile_t* const a_WAD, const WX_DataPrivateID_t a_ID, void*** const a_PPPtr, size_t** const a_PPSize);
WX_WADEntry_t*		WX_RoveEntry(WX_WADEntry_t* const a_Entry, const ssize_t a_Next);
size_t				WX_GetEntryName(WX_WADEntry_t* const a_Entry, char* const a_OutBuf, const size_t a_OutSize);
size_t				WX_GetEntrySize(WX_WADEntry_t* const a_Entry);



#endif							/* __W_WAD__ */

