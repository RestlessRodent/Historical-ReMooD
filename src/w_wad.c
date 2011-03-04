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
//                    Dragan                 (poliee13@hotmail.com)
// -----------------------------------------------------------------------------
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

/* W_UnloadData() -- Unload all WAD attached data */
void W_UnloadData(void)
{
	HU_UnloadWadData();
}

/* W_LoadData() -- Load all WAD attached data */
void W_LoadData(void)
{
	HU_LoadWadData();
}

/* W_BaseName() -- Find basename of file */
const char* W_BaseName(const char* Name)
{
	const char* TempX = NULL;
	const char* OldTempX = NULL;
	
	/* Check */
	if (!Name)
		return NULL;
		
	/* Get last occurence */
	// Find last slash
	TempX = strrchr(Name, '/');
	
#if defined(_WIN32)
	// Find last backslash if we found no slashes
	if (!TempX)
		TempX = strrchr(Name, '\\');
	
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
		TempX = Name;
	
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

/* W_FindWad() -- Finds a WAD File */
boolean W_FindWad(const char* Name, const char* MD5, char* OutPath, const size_t OutSize)
{
	#define MAXSEARCHPATH 32
	#define PATHSIZE 512//PATH_MAX
#ifdef _WIN32
	#define PATHDELIM '\\'
#else
	#define PATHDELIM '/'
#endif
	char* WADPath;											// DOOMWADPATH
	char* WADDir;											// DOOMWADDIR
	char* DirArg;											// -waddir argument
	char SearchDirs[MAXSEARCHPATH][PATHSIZE];				// Current searching directories
	char Buf[PATHSIZE];
	size_t NumDirs;											// Number of search directories
	int i, n, j;
	const char* BaseName;
	
	/* Check */
	if (!Name)
		return false;
	
	/* Clear */
	BaseName = NULL;
	WADPath = NULL;
	WADDir = NULL;
	memset(SearchDirs, 0, sizeof(SearchDirs));
	NumDirs = 0;
	
	/* Find base name */
	BaseName = W_BaseName(Name);
	
	/* Add Paths */
	// Nothing
	strncpy(SearchDirs[NumDirs++], "", PATHSIZE);
	
	// Current working dir
	strncpy(SearchDirs[NumDirs++], "./", PATHSIZE);
	
	// bin
	strncpy(SearchDirs[NumDirs++], "./bin/", PATHSIZE);
	
	// -waddir
	if (M_CheckParm("-waddir"))
		while (M_IsNextParm())
		{
			// Get directory argument
			DirArg = M_GetNextParm();
			
			// Add to Search
			if (NumDirs < MAXSEARCHPATH)
			{
				strncpy(SearchDirs[NumDirs], DirArg, PATHSIZE);
			
				// Remove trailing slashes
				while (strlen(SearchDirs[NumDirs]) &&
					(SearchDirs[NumDirs][strlen(SearchDirs[NumDirs]) - 1] == '/'
#if defined(_WIN32)
					|| SearchDirs[NumDirs][strlen(SearchDirs[NumDirs]) - 1] == '\\'
#endif
					))
					SearchDirs[NumDirs][strlen(SearchDirs[NumDirs]) - 1] = 0;
				
				// Add trailing slash
				SearchDirs[NumDirs][strlen(SearchDirs[NumDirs])] = '/';
				
				// Increment
				NumDirs++;
			}
		}
	
	// DOOMWADDIR
	WADDir = getenv("DOOMWADDIR");
	
	if (WADDir)
		if (NumDirs < MAXSEARCHPATH)
		{
			// Add to Search
			strncpy(SearchDirs[NumDirs], WADDir, PATHSIZE);
			
			// Remove trailing slashes
			while (strlen(SearchDirs[NumDirs]) &&
				(SearchDirs[NumDirs][strlen(SearchDirs[NumDirs]) - 1] == '/'
#if defined(_WIN32)
				|| SearchDirs[NumDirs][strlen(SearchDirs[NumDirs]) - 1] == '\\'
#endif
				))
				SearchDirs[NumDirs][strlen(SearchDirs[NumDirs]) - 1] = 0;
			
			// Add trailing slash
			SearchDirs[NumDirs][strlen(SearchDirs[NumDirs])] = '/';
			
			// Increment
			NumDirs++;
		}
	
	// DOOMWADPATH
	WADPath = getenv("DOOMWADPATH");
	
	if (WADPath)
	{
		// Get length
		n = strlen(WADPath);
		
		if (n)
			for (j = 0, i = 0; i < n + 1; i++)
				if (WADPath[i] == 0 ||
#ifdef _WIN32
					WADPath[i] == ';'
#else
					WADPath[i] == ':'
#endif
					)
				{
					// Clip off
					if (j < PATHSIZE)
						SearchDirs[NumDirs][j] = 0;
					j = 0;
					
					// Remove trailing slashes
					while (strlen(SearchDirs[NumDirs]) &&
						(SearchDirs[NumDirs][strlen(SearchDirs[NumDirs]) - 1] == '/'
#if defined(_WIN32)
						|| SearchDirs[NumDirs][strlen(SearchDirs[NumDirs]) - 1] == '\\'
#endif
						))
						SearchDirs[NumDirs][strlen(SearchDirs[NumDirs]) - 1] = 0;
			
					// Add trailing slash
					SearchDirs[NumDirs][strlen(SearchDirs[NumDirs])] = '/';
					
					// Increment
					NumDirs++;
				}
				else
				{
					if (j < PATHSIZE - 1)
						SearchDirs[NumDirs][j++] = WADPath[i];
				}
	}
	
	/* Do actual WAD Searching */
	for (i = 0; i < NumDirs; i++)
		for (j = 0; j < 2; j++)
		{
			// Skip base
			if (j && Name == BaseName)
				continue;
			
			// Copy to buffer
			strncpy(Buf, SearchDirs[i], PATHSIZE);
			strncat(Buf, (j ? BaseName : Name), PATHSIZE);
		
			// Message
			if (devparm)
				CONS_Printf("W_FindWad: Searching \"%s\"...\n", Buf);
		
			// Found it?
			if (!access(Buf, R_OK))
			{
				// Check MD5
				if (MD5)
					;
			
				// Copy to buffer
				if (OutPath && OutSize)
					strncpy(OutPath, Buf, OutSize);
			
				// Success!
				return true;
			}
		}
	
	/* Failure */
	return false;
#undef PATHDELIM
}

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
	uint32_t OpenMethod = MAXMETHODS;	// Method of opening the file
	int readcount;
	int err;
	int y, z;
	char tName[9];
	boolean Swapped = false;

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

			if (!(readcount = fread(Temp, sizeof(uint32_t), 1, tFile)))
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
			uint32_t NumLumps = 0;
			uint32_t IndexOffset = 0;
			uint32_t j = 0;

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
						n->NumLumps = LITTLESWAP32(NumLumps);
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
						uint32_t l;
						uint32_t sz = 0;
						uint32_t pos = 0;
						
						// GhostlyDeath <November 5, 2010> -- Swap for BE
						IndexOffset = LITTLESWAP32(IndexOffset);

						n->WADNameHack = Z_Malloc((NumLumps * 9) + 1, PU_STATIC, NULL);
						memset(n->WADNameHack, 0, (NumLumps * 9) + 1);
						
						n->Index = Z_Malloc(sizeof(WadEntry_t) * NumLumps, PU_STATIC, NULL);
						memset(n->Index, 0, sizeof(WadEntry_t) * NumLumps);

						fseek(tFile, IndexOffset, SEEK_SET);

						for (l = 0; l < NumLumps; l++)
						{
							fread(&pos, sizeof(uint32_t), 1, tFile);
							n->Index[l].Position = pos;

							fread(&sz, sizeof(uint32_t), 1, tFile);
							n->Index[l].Size = sz;

							/*n->Index[l].Name = Z_Malloc(9, PU_STATIC, NULL);
							   fread(n->Index[l].Name, 8, 1, tFile);
							   n->Index[l].Name[8] = 0; */

							n->Index[l].Name = &(((char *)(n->WADNameHack))[l * 9]);
							
							fread(tName, 8, 1, tFile);
							strncpy(n->Index[l].Name, tName, 9);
							
							n->Index[l].Name[8] = 0;

							/*n->Index[l].CachePtr = NULL;
							n->Index[l].Picture = NULL;*/
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
			for (j = 0; j < NUMWADENTRYTYPES; j++)
				if (Rover->Index[i].Cache[j])
				{
					Z_Free(Rover->Index[i].Cache[j]);
					Rover->Index[i].Cache[j] = NULL;
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
	int32_t i;
	int32_t num;
	int32_t actual = 0;
	char NewName[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	
	// Turn into a UInt64
	memcpy(NewName, name, strlen(name));
	for (i = 0; i < 8; i++)
		if (NewName[i] >= 'A' && NewName[i] <= 'Z')
			NewName[i] += 32;
	
	// Now find it
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
					//if (memcmp(NewName, rover->Index[i].Name, 8) == 0)
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
				{
					//if (memcmp(NewName, rover->Index[i - 1].Name, 8) == 0)
					if (strncasecmp(rover->Index[i - 1].Name, name, 8) == 0)
						return actual;
				}

				rover = rover->Prev;
			}

			return INVALIDLUMP;
		}
	}
	else
		return INVALIDLUMP;
}

/* W_LumpsSoFar() -- Returns number of lumps passed to get to this WAD */
WadIndex_t W_LumpsSoFar(WadFile_t * wadid)
{
	WadIndex_t actual = 0;
	WadFile_t* OldWAD = NULL;
	WadFile_t* rover = WADFiles;
	
	// GhostlyDeath <June 21, 2009> -- Not sure if we should really return INVALIDLUMP
	if (!rover || !wadid)
		return INVALIDLUMP;
		
	// Shortcut to first WAD..
	if (rover == wadid)
		return 0;
	
	// Otherwise
	while (rover)
	{
		OldWAD = rover;
		rover = rover->Next;
	
		if (rover)
		{
			actual += OldWAD->NumLumps;
			
			if (rover == wadid)
				break;
		}
	}
	
	return actual;
	
	/*
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
	*/
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
	   uint32_t j;

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
	int32_t j;

	if (wadid)
	{
		//for (j = 0; j < W_GetNumForWad(wadid) - 1; j++)
		//	actual += W_GetWadForNum(j)->NumLumps - 1;
		//actual += W_LumpsSoFar(wadid);

		if (wadid->NumLumps > MAXLUMPS)
			num = MAXLUMPS;
		else
			num = wadid->NumLumps;

		if (startlump > MAXLUMPS)
			startlumpx = MAXLUMPS;
		else
			startlumpx = startlump;
			
		actual = startlumpx;

		for (i = startlumpx; i < num; i++, actual++)
			if (strncasecmp(wadid->Index[i].Name, name, 8) == 0)
				return W_LumpsSoFar(wadid) + actual;

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

void* W_CacheAsConvertableType(WadIndex_t Lump, size_t PU, WadEntryType_t Type, WadEntryType_t From)
{
	WadEntry_t *Entry = W_GetEntry(Lump);
	void* RawData = NULL;
	
	/* Bunch of Checks */
	if (!Entry)
		return NULL;
		
	if (Type < 0 || Type >= NUMWADENTRYTYPES)
		return NULL;
		
	if (From < 0 || From >= NUMWADENTRYTYPES)
		return NULL;
		
	/* What converts into what? */
	if (Entry->Cache[Type] != NULL)
		return Entry->Cache[Type];
	
	/* Get the raw data */
	RawData = W_CacheLumpNum(Lump, PU);
	
	/* From and to the same? */
	if (From == Type)
		return RawData;
	
	/* From/To Mess */
	switch (From)
	{
		case WETYPE_RAW:
			return NULL;
			break;
		
		/********* PICTURE FORMATS *********/
		case WETYPE_PATCHT:
			if (Type >= WETYPE_TEXTTYPESTART && Type <= WETYPE_TEXTTYPEEND)
				return NULL;
			
			break;
			
		case WETYPE_PICT:
			if (Type >= WETYPE_TEXTTYPESTART && Type <= WETYPE_TEXTTYPEEND)
				return NULL;
			
			break;
			
		case WETYPE_FLAT:
			if (Type >= WETYPE_TEXTTYPESTART && Type <= WETYPE_TEXTTYPEEND)
				return NULL;
			
			break;
		
		/********* TEXT FORMATS *********/
		case WETYPE_AUTOTEXT:
			if (Type >= WETYPE_PICTURETYPESTART && Type <= WETYPE_PICTURETYPEEND)
				return NULL;
			
			break;
			
		case WETYPE_WCHART:
			if (Type >= WETYPE_PICTURETYPESTART && Type <= WETYPE_PICTURETYPEEND)
				return NULL;
			
			break;
		
		default:
			break;
	}
}

void* W_CacheAsConvertableTypeName(char* Name, size_t PU, WadEntryType_t Type, WadEntryType_t From)
{
	return W_CacheAsConvertableType(W_GetNumForName(Name), PU, Type, From);
}

/* W_CacheAs() -- */
void *W_CacheLumpNum(WadIndex_t lump, size_t PU)
{
#if defined(_DEBUG)
	char DbgString[128];
#endif
	WadEntry_t *Entry = W_GetEntry(lump);

	if (Entry)
	{
		if (Entry->Size < 1)
			return NULL;

		// Is it already cached?
		if (Entry->Cache[WETYPE_RAW] == NULL)
		{
			Entry->Cache[WETYPE_RAW] = Z_Malloc(Entry->Size, PU, &(Entry->Cache[WETYPE_RAW]));

#if defined(_DEBUG)
			snprintf(DbgString, 128, "E=\"%s\";S=%u;P=%u;H=\"%s\"", Entry->Name, Entry->Size, Entry->Position, Entry->Host->FileName);
			
			Z_DebugMarkBlock(Entry->Cache[WETYPE_RAW], DbgString);
#endif

			if (!Entry->Cache[WETYPE_RAW])
				I_Error("W_CacheLumpNum: Failed to allocate space for lump!\n");

			W_ReadLump(lump, Entry->Cache[WETYPE_RAW]);
		}

		return Entry->Cache[WETYPE_RAW];
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

/* W_CachePatchNum() -- Cache's a patch from a raw patch */
void* W_CachePatchNum(const WadIndex_t Lump, size_t PU)
{
	return W_CacheLumpNum(Lump, PU);
}

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
		if (!we->Cache[WETYPE_PICT])
		{
			we->Cache[WETYPE_PICT] = Z_Malloc(W_LumpLength(lump) + sizeof(pic_t), tag, &we->Cache[WETYPE_PICT]);
			W_ReadLumpHeader(lump, ((pic_t*)we->Cache[WETYPE_PICT])->data, 0);
			((pic_t*)we->Cache[WETYPE_PICT])->width = SHORT(width);
			((pic_t*)we->Cache[WETYPE_PICT])->height = SHORT(height);
			((pic_t*)we->Cache[WETYPE_PICT])->mode = PALETTE;
		}
		else
			Z_ChangeTag(we->Cache[WETYPE_PICT], tag);

		return we->Cache[WETYPE_PICT];
	}
	else
		return NULL;
}

