// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.org/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           TetrisMaster512        (tetrismaster512@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 2008-2010 The ReMooD Team.
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

// New WAD Code by GhostlyDeath
WadFile_t *WADFiles = NULL;

size_t W_NumWadFiles(void)
{
	size_t i = 0;
	WadFile_t *x = WADFiles;

	while (x)
	{
		i++;
		x = x->Next;
	}

	return i;
}

WadFile_t *W_GetWadForNum(size_t Num)
{
	size_t i = 0;
	WadFile_t *x = WADFiles;

	while (x)
	{
		if (i == Num)
			return x;

		i++;
		x = x->Next;
	}

	return NULL;
}

WadFile_t *W_GetWadForName(char *Name)
{
	WadFile_t *x = WADFiles;

	while (x)
	{
		if (strcasecmp(x->FileName, Name) == 0)
			return x;

		x = x->Next;
	}

	return NULL;
}

size_t W_GetNumForWad(WadFile_t * WAD)
{
	size_t i = 0;
	WadFile_t *x = WADFiles;

	while (x)
	{
		i++;

		if (x == WAD)
			return i;

		x = x->Next;
	}

	return INVALIDLUMP;
}

/* W_LoadWadFile() -- Loads a WAD File */
int W_LoadWadFile(char *filename)
{
	FILE *tFile = NULL;			// this file
	WadFile_t *tWAD = NULL;		// this WAD
	WadFile_t *n = NULL;		// Scanning (Next/This)
	WadFile_t *o = NULL;		// Oh...
	WadFile_t *p = NULL;		// Scanning (Prev)
	UInt32 OpenMethod = MAXMETHODS;	// Method of opening the file
	int readcount;
	int err;

	/* Scan! -- Don't open a WAD twice yknow! */
	n = WADFiles;

	while (n)
	{
		if (strcasecmp(filename, n->FileName) == 0)
			return W_NumWadFiles() - 1;

		n = n->Next;
	}

	// Open it
	tFile = fopen(filename, "rb");

	// it open?
	if (tFile == NULL)
	{
		err = -errno;
		
		if (devparm)
			CONS_Printf("W_LoadWadFile: tFile is NULL!\n");
		
		return -1;
	}
	else
	{
		/* Determine Opening Method or return 0 */
		if (stricmp(&filename[strlen(filename) - 3], "deh") == 0)
			OpenMethod = METHOD_DEHACKED;
		else
		{
			char Temp[4];

			if (!(readcount = fread(Temp, sizeof(UInt32), 1, tFile)))
			{
				if (devparm)
					CONS_Printf("W_LoadWadFile: Failed to read header!\n");
				fclose(tFile);
				return -2;
			}
			else
			{
				if ((strncmp(Temp, "IWAD", 4) == 0) || (strncmp(Temp, "PWAD", 4) == 0))
					OpenMethod = METHOD_WAD;
				else
				{
					if (devparm)
						CONS_Printf("W_LoadWadFile: \"%s\" is not a PWAD!\n", filename);
					fclose(tFile);
					return -3;
				}
			}
		}

		// TODO
		if (OpenMethod == METHOD_DEHACKED)
		{
			if (devparm)
				CONS_Printf("W_LoadWadFile: DeHackEd file is being loaded elsewhere.");
			
			fclose(tFile);
			
			DEH_LoadDehackedFile(filename);
			
			return -4;
		}

		/* Alloc Looping */
		n = NULL;

		if (WADFiles == NULL)
		{
			int z;
			WADFiles = Z_Malloc(sizeof(WadFile_t), PU_STATIC, NULL);
			n = WADFiles;

			// Initialize
			n->Prev = NULL;
			n->Next = NULL;
			n->NumLumps = 0;
			n->File = NULL;
			n->Size = 0;
			n->Index = NULL;
			n->Method = 0;
			n->WADNameHack = NULL;
			for (z = 0; z < 16; z++)
				n->MD5Sum[z] = 0;
		}
		else					// Seek to the end
		{
			int z;
			p = NULL;
			o = WADFiles;

			for (;;)
			{
				if (o != NULL)
				{
					p = o;
					o = o->Next;
				}
				else
				{
					o = Z_Malloc(sizeof(WadFile_t), PU_STATIC, NULL);
					p->Next = o;
					o->Prev = p;
					o->Next = NULL;

					// Initialize
					o->NumLumps = 0;
					o->File = NULL;
					o->Size = 0;
					o->Index = NULL;
					o->Method = 0;
					o->WADNameHack = NULL;
					for (z = 0; z < 16; z++)
						o->MD5Sum[z] = 0;
					break;
				}
			}

			n = o;
		}

		// OK! Now the fun part!
		if (n)
		{
			UInt32 NumLumps = 0;
			UInt32 IndexOffset = 0;
			UInt32 j = 0;

			n->Method = OpenMethod;
			n->File = tFile;

			/* FileName */
			n->FileName = Z_Malloc(strlen(filename) + 1, PU_STATIC, NULL);
			strcpy(n->FileName, filename);
			n->FileName[strlen(filename)] = 0;

			switch (n->Method)
			{
				case METHOD_WAD:	// WAD File
					/* NumLumps */
					if ((readcount = fread(&NumLumps, sizeof(NumLumps), 1, tFile)))
						n->NumLumps = NumLumps;
					else
					{
						if (devparm)
							CONS_Printf
								("W_LoadWadFile: Failed! NumLumps ?= %i (read %i bytes)\n",
								 NumLumps, readcount);
						fclose(tFile);
						return -5;
					}

					/* MD5 Sum -- TODO */
					for (j = 0; j < 16; j++)
						n->MD5Sum[j] = 127;

					/* Indexes */
					if (readcount = fread(&IndexOffset, sizeof(IndexOffset), 1, tFile))
					{
						UInt32 l;
						UInt32 sz = 0;
						UInt32 pos = 0;

						n->WADNameHack = Z_Malloc((NumLumps * 9) + 1, PU_STATIC, NULL);
						n->Index = Z_Malloc(sizeof(WadEntry_t) * NumLumps, PU_STATIC, NULL);

						fseek(tFile, IndexOffset, SEEK_SET);

						for (l = 0; l < NumLumps; l++)
						{
							fread(&pos, sizeof(UInt32), 1, tFile);
							n->Index[l].Position = pos;

							fread(&sz, sizeof(UInt32), 1, tFile);
							n->Index[l].Size = sz;

							/*n->Index[l].Name = Z_Malloc(9, PU_STATIC, NULL);
							   fread(n->Index[l].Name, 8, 1, tFile);
							   n->Index[l].Name[8] = 0; */

							n->Index[l].Name = &(((char *)(n->WADNameHack))[l * 9]);
							fread(n->Index[l].Name, 8, 1, tFile);
							n->Index[l].Name[8] = 0;

							n->Index[l].CachePtr = NULL;
							n->Index[l].Picture = NULL;
							n->Index[l].Host = n;
						}
						
						// We will close the file later
						n->File = tFile;
					}
					else
					{
						if (devparm)
							CONS_Printf
								("W_LoadWadFile: Failed! IndexOffset ?= %i (read %i bytes)\n",
								 IndexOffset, readcount);
						fclose(tFile);
						return -6;
					}

					if (devparm)
						CONS_Printf("W_LoadWadFile: Successfully loaded \"%s\"!\n", filename);
					return W_NumWadFiles() - 1;
					break;

				default:
					if (devparm)
						CONS_Printf("W_LoadWadFile: No Method!\n");
					fclose(tFile);
					return -3;
			}
		}
		else
		{
			if (devparm)
				CONS_Printf("W_LoadWadFile: n is NULL!\n");
			return -1;
		}
	}
}

/* W_Shutdown() -- Close down WAD Handling system and be sure */
void W_Shutdown(void)
{
	WadFile_t* Rover = WADFiles;
	WadFile_t* Next = NULL;
	size_t i, j;
	
	while (Rover)
	{
		// Next will be eradicated
		Next = Rover->Next;
		
		// Un-cache all lumps
		for (i = 0; i < Rover->NumLumps; i++)
		{
			// Normal data cache
			if (Rover->Index[i].CachePtr)
			{
				Z_Free(Rover->Index[i].CachePtr);
				Rover->Index[i].CachePtr = NULL;
			}
			
			// pic_t or patch_t cache
			if (Rover->Index[i].Picture)
			{
				Z_Free(Rover->Index[i].Picture);
				Rover->Index[i].Picture = NULL;
			}
		}
		
		// Free the Index
		if (Rover->Index)
		{
			Z_Free(Rover->Index);
			Rover->Index = NULL;
		}
		
		// Free WADNameHack
		if (Rover->WADNameHack)
		{
			Z_Free(Rover->WADNameHack);
			Rover->WADNameHack = NULL;
		}
		
		// Free FileName
		if (Rover->FileName)
		{
			Z_Free(Rover->FileName);
			Rover->FileName = NULL;
		}
		
		// Close the file
		if (Rover->File)
		{
			fclose(Rover->File);
			Rover->File = NULL;
		}
		
		// Remove the rover
		Z_Free(Rover);
		
		// Move to next WAD
		Rover = Next;
	}
	
	WADFiles = NULL;
}

/* W_InitMultipleFiles() -- This gets based a list of C Strings for multiple WAD loading */
int W_InitMultipleFiles(char **filenames)
{
	int worked = 0;
	int ret = -1;
	char* p = NULL;

	// will be realloced as lumps are added
	for (; *filenames; filenames++)
	{
		// Try loading it
		if ((ret = W_LoadWadFile(*filenames)) > -1)
			worked++;
		
		switch (ret)
		{
			case -1: p = strerror(-ret); break;
			case -2: p = "Invalid WAD File"; break;
			case -3: p = "Unsupported format"; break;
			case -4: p = "File rerouted"; break;
			case -5: p = "Index past EOF"; break;
			case -6: p = "Index offset past EOF"; break;
			default: p = "Success"; break;
		}
		
		// Print message
		CONS_Printf("W_InitMultipleFiles: %s: %s\n", *filenames, p);
	}
	
	/* Load Data */
	V_MapGraphicalCharacters();
	//RMD_LoadDefinitions(W_NumWadFiles());

	if (W_NumWadFiles() == 0)
		I_Error("W_InitMultipleFiles: no files found\n");

	return W_NumWadFiles();
}

/* W_Reload() -- Reloads a WAD file? */
void W_Reload(void)
{
}

/* W_BiCheckNumForName() -- Checks for an existance of a lump (forward and reverse) */
WadIndex_t W_BiCheckNumForName(char *name, int forwards)
{
	// Go the the last WAD
	WadFile_t *rover = WADFiles;
	Int32 i;
	Int32 num;
	Int32 actual = 0;

	if (rover)
	{
		if (forwards)
		{
			while (rover)
			{
				if (rover->NumLumps > MAXLUMPS)
					num = MAXLUMPS;
				else
					num = rover->NumLumps;

				for (i = 0; i < num; i++, actual++)
					if (strncasecmp(rover->Index[i].Name, name, 8) == 0)
						return actual;

				rover = rover->Next;
			}

			return INVALIDLUMP;
		}
		else
		{
			// Rove around the stuff
			while (rover->Next != NULL)
			{
				actual += rover->NumLumps;
				rover = rover->Next;
			}

			actual += rover->NumLumps - 1;

			while (rover)
			{
				if (rover->NumLumps > MAXLUMPS)
					num = MAXLUMPS;
				else
					num = rover->NumLumps;

				for (i = num; i > 0; i--, actual--)
					if (strncasecmp(rover->Index[i - 1].Name, name, 8) == 0)
						return actual;

				rover = rover->Prev;
			}

			return INVALIDLUMP;
		}
	}
	else
		return INVALIDLUMP;
}

WadIndex_t W_LumpsSoFar(WadFile_t * wadid)
{
	WadIndex_t actual = 0;
	WadFile_t *rover = WADFiles;

	if (rover)
	{
		if (!rover)
			return INVALIDLUMP;

		while (rover && (rover != wadid))
		{
			actual += rover->NumLumps;
			rover = rover->Next;
		}

		return actual;
	}
	return INVALIDLUMP;
}

/* W_CheckNumForName() -- Checks for the existance of a lump by name */
WadIndex_t W_CheckNumForName(char *name)
{
	return W_BiCheckNumForName(name, 0);
}

/* W_CheckNumForNamePwad() -- */
// This is as compatible as it gets!
WadIndex_t W_CheckNumForNamePwad(char *name, size_t wadid, WadIndex_t startlump)
{
	/*WadFile_t *rover = W_GetWadForName(name);
	   WadIndex_t i;
	   WadIndex_t num;
	   WadIndex_t startlumpx;
	   WadIndex_t actual = 0;
	   UInt32 j;

	   if (rover)
	   {
	   for (j = 0; j < wadid; j++)
	   actual += W_GetWadForNum(j)->NumLumps - 1;

	   if (rover->NumLumps > MAXLUMPS)
	   num = MAXLUMPS;
	   else
	   num = rover->NumLumps;

	   if (startlump > MAXLUMPS)
	   startlumpx = MAXLUMPS;
	   else
	   startlumpx = startlump;

	   for (i = startlumpx; i < num; i++, actual++)
	   if (strcasecmp(rover->Index[i].Name, name) == 0)
	   return actual;

	   return INVALIDLUMP;
	   }
	   else
	   return INVALIDLUMP; */
	return W_CheckNumForNamePwadPtr(name, W_GetWadForNum(wadid), startlump);
}

WadIndex_t W_CheckNumForNamePwadPtr(char *name, WadFile_t * wadid, WadIndex_t startlump)
{
	WadIndex_t i;
	WadIndex_t num;
	WadIndex_t startlumpx;
	WadIndex_t actual = 0;
	UInt32 j;

	if (wadid)
	{
		for (j = 0; j < W_GetNumForWad(wadid); j++)
			actual += W_GetWadForNum(j)->NumLumps - 1;

		if (wadid->NumLumps > MAXLUMPS)
			num = MAXLUMPS;
		else
			num = wadid->NumLumps;

		if (startlump > MAXLUMPS)
			startlumpx = MAXLUMPS;
		else
			startlumpx = startlump;

		for (i = startlumpx; i < num; i++, actual++)
			if (strncasecmp(wadid->Index[i].Name, name, 8) == 0)
				return actual;

		return INVALIDLUMP;
	}
	else
		return INVALIDLUMP;
}

WadIndex_t W_CheckNumForNameFirst(char *name)
{
	return W_BiCheckNumForName(name, 1);
}

WadIndex_t W_GetNumForName(char *name)
{
	WadIndex_t i = W_CheckNumForName(name);

	if (i == INVALIDLUMP)
	{
		I_Error("W_GetNumForName: \"%s\" not found!\n", name);
		return INVALIDLUMP;
	}
	else
		return i;
}

WadIndex_t W_GetNumForNameFirst(char *name)
{
	WadIndex_t i = W_CheckNumForNameFirst(name);

	if (i == INVALIDLUMP)
	{
		I_Error("W_GetNumForName: \"%s\" not found!\n", name);
		return INVALIDLUMP;
	}
	else
		return i;
}

/* W_GetEntry() -- Returns a pointer to a WadEntry_t */
WadEntry_t *W_GetEntry(WadIndex_t lump)
{
	WadFile_t *rover = WADFiles;
	WadIndex_t val = 0;

	if (lump == INVALIDLUMP)
		return NULL;

	while (rover)
	{
		// in this WAD?
		if ((lump >= val) && (lump <= (val + (rover->NumLumps - 1))))
			return &(rover->Index[lump - val]);
		else
		{
			val += rover->NumLumps;
			rover = rover->Next;
		}
	}

	return NULL;
}

/* W_GetNumForEntry() -- Converts entry to number */
WadIndex_t W_GetNumForEntry(WadEntry_t* Entry)
{
	if (!Entry)
		return INVALIDLUMP;
	
	if (!Entry->Host)
		return INVALIDLUMP;
	
	if (!Entry->Host->Index)
		return INVALIDLUMP;
	
	return W_LumpsSoFar(Entry->Host) + (Entry - Entry->Host->Index);
}

/* W_LumpLength() -- Returns the length of the lump */
size_t W_LumpLength(WadIndex_t lump)
{
	WadEntry_t *we = W_GetEntry(lump);

	if (we)
		return we->Size;
	else
		return 0;
}

size_t W_ReadLumpHeader(WadIndex_t lump, void *dest, size_t size)
{
	size_t BytesRead = 0;
	size_t BytesToRead = 0;
	WadEntry_t *we = W_GetEntry(lump);

	if (!dest)
	{
		I_Error("W_ReadLumpHeader: Dest is NULL!\n");
		return 0;
	}

	if (we)
	{
		if (size == 0)
			BytesToRead = we->Size;
		else
			BytesToRead = size;

		fseek(we->Host->File, we->Position, SEEK_SET);
		BytesRead = fread(dest, BytesToRead, 1, we->Host->File);
		return BytesRead;
	}
	else
		return 0;
}

void W_ReadLump(WadIndex_t lump, void *dest)
{
	W_ReadLumpHeader(lump, dest, 0);
}

void *W_CacheLumpNum(WadIndex_t lump, size_t PU)
{
	WadEntry_t *Entry = W_GetEntry(lump);

	if (Entry)
	{
		if (Entry->Size < 1)
			return NULL;

		// Is it already cached?
		if (Entry->CachePtr == NULL)
		{
			Entry->CachePtr = Z_Malloc(Entry->Size, PU, &(Entry->CachePtr));

			if (!Entry->CachePtr)
				I_Error("W_CacheLumpNum: Failed to allocate space for lump!\n");

			W_ReadLump(lump, Entry->CachePtr);
		}

		return Entry->CachePtr;
	}
	else
		return NULL;
}

void *W_CacheLumpName(char *name, size_t PU)
{
	return W_CacheLumpNum(W_GetNumForName(name), PU);
}

/* W_CacheEntry() -- Caches an Entry based on pointer */
/*void* W_CacheEntry(WadEntry_t* Entry, size_t PU)
{
	if (Entry)
	{
		// Is it already cached?
		if (Entry->CachePtr == NULL)
		{
			Entry->CachePtr = Z_Malloc(Entry->Size, PU_CACHE, &(Entry->CachePtr));
			W_ReadLump(lump, Entry->CachePtr);
		}
		
		return Entry->CachePtr;
	}
	else
		return NULL;
																																   																   								   				   		   	   	}*/// Dont use this yet...

/* W_CachePatchName() -- Cache's a patch as a patch_t */
void *W_CachePatchName(char *name, size_t PU)
{
	if (W_CheckNumForName(name) == INVALIDLUMP)
		return W_CachePatchNum(W_GetNumForName("BRDR_MM"), PU);
	return W_CachePatchNum(W_GetNumForName(name), PU);
}

/* W_CacheRawAsPic() -- I have no clue what or what this does! */
// but I think whatever they were doing before was kinda like this!
void *W_CacheRawAsPic(WadIndex_t lump, int width, int height, size_t tag)
{
	WadEntry_t *we = NULL;
	pic_t *pic = NULL;

	we = W_GetEntry(lump);

	if (we)
	{
		if (!we->Picture)
		{
			we->Picture = Z_Malloc(W_LumpLength(lump) + sizeof(pic_t), tag, &we->Picture);
			W_ReadLumpHeader(lump, ((pic_t*)we->Picture)->data, 0);
			((pic_t*)we->Picture)->width = SHORT(width);
			((pic_t*)we->Picture)->height = SHORT(height);
			((pic_t*)we->Picture)->mode = PALETTE;
		}
		else
			Z_ChangeTag(we->Picture, tag);

		return we->Picture;
	}
	else
		return NULL;
}

