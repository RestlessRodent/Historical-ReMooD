// Emacs style mode select   -*- C++ -*- 
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
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           Demyx                  (demyx@endgameftw.com)
//                    Dragan                 (poliee13@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2009 The ReMooD Team..
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
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
//#include "r_defs.h"

typedef void GlidePatch_t;

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

typedef struct WadEntry_s
{
	/* Basic */
	char *Name;					// Pointer to a string (for Pk3s later on)
	size_t Size;
	size_t Position;

	/* Cache Info */
	void *CachePtr;
	void *Picture;

	/* Parent */
	struct WadFile_s *Host;

} WadEntry_t;

typedef struct WadFile_s
{
	/* Filesystem */
	char *FileName;				// Filename
	size_t NumLumps;			// Number of Lumps
	FILE *File;					// File pointer
	size_t Size;				// Size of the file

	/* Data */
	WadEntry_t *Index;			// WAD's Index

	/* WAD Hack */
	void *WADNameHack;

	/* Special */
	UInt32 Method;				// File Method

	/* Multiplayer Verification */
	UInt8 MD5Sum[16];			// MD5 Sum

	/* Links */
	struct WadFile_s *Prev;
	struct WadFile_s *Next;
} WadFile_t;

#define INVALIDLUMP -1
typedef int WadIndex_t;
#define MAXLUMPS 2147483647
#define INDEXSIZET(x) ((int)(x))

size_t W_NumWadFiles(void);
WadFile_t *W_GetWadForNum(size_t Num);
WadFile_t *W_GetWadForName(char *Name);
size_t W_GetNumForWad(WadFile_t * WAD);
WadEntry_t *W_GetEntry(WadIndex_t lump);
WadIndex_t W_LumpsSoFar(WadFile_t * wadid);
WadIndex_t W_InitMultipleFiles(char **filenames);
void W_Shutdown(void);
int W_LoadWadFile(char *filename);
void W_Reload(void);
WadIndex_t W_CheckNumForName(char *name);
WadIndex_t W_CheckNumForNamePwad(char *name, size_t wadid, WadIndex_t startlump);
WadIndex_t W_CheckNumForNamePwadPtr(char *name, WadFile_t * wadid, WadIndex_t startlump);
WadIndex_t W_GetNumForName(char *name);
WadIndex_t W_CheckNumForNameFirst(char *name);
WadIndex_t W_GetNumForNameFirst(char *name);
size_t W_LumpLength(WadIndex_t lump);
size_t W_ReadLumpHeader(WadIndex_t lump, void *dest, size_t size);
void W_ReadLump(WadIndex_t lump, void *dest);
void *W_CacheLumpNum(WadIndex_t lump, size_t PU);
void *W_CacheLumpName(char *name, size_t PU);
void *W_CachePatchName(char *name, size_t PU);
//void* W_CacheEntry(WadEntry_t* Entry, size_t PU);
#define W_CachePatchNum(lump,tag)    W_CacheLumpNum(lump,tag)
void *W_CacheRawAsPic(WadIndex_t lump, int width, int height, size_t tag);	// return a pic_t
WadIndex_t W_GetNumForEntry(WadEntry_t* Entry);

typedef struct
{
	WadFile_t *WadFile;
	size_t firstlump;
	size_t numlumps;
} lumplist_t;

extern WadFile_t *WADFiles;

#endif							/* __W_WAD__ */

