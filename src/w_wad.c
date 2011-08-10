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
#include "d_info.h"

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
bool_t W_FindWad(const char* Name, const char* MD5, char* OutPath, const size_t OutSize)
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
	bool_t Swapped = false;
	
	/* Send to extended code */
	WX_VirtualPushPop(WX_LoadWAD(filename), false, true);

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
						n->NumLumps = LittleSwapUInt32(NumLumps);
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
						IndexOffset = LittleSwapUInt32(IndexOffset);

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
	
	/* Initialize the extended WAD Code here */
	WX_Init();

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
			((pic_t*)we->Cache[WETYPE_PICT])->width = LittleSwapInt16(width);
			((pic_t*)we->Cache[WETYPE_PICT])->height = LittleSwapInt16(height);
			((pic_t*)we->Cache[WETYPE_PICT])->mode = PALETTE;
		}
		else
			Z_ChangeTag(we->Cache[WETYPE_PICT], tag);

		return we->Cache[WETYPE_PICT];
	}
	else
		return NULL;
}

/*******************************************************************************
***************************** EXTENDED WAD HANDLING ****************************
*******************************************************************************/

/****************
*** CONSTANTS ***
****************/

#define WXCHECKSUMSIZE					32		// Max characters allowed in checksum
#define MAXSEARCHBUFFER					32		// Max Paths to store

/* WX_EntryFlag_t -- Flag for an entry */
typedef enum WX_EntryFlag_e
{
	WXEF_COMPRESSED				= 0x00000001,	// Entry is compressed (to save space)
} WX_EntryFlag_t;

/* WX_WADType_t -- Type of WAD File this is */
typedef enum WX_WADType_e
{
	WXWT_DOOMWAD,								// Standard IWAD/PWAD
	WXWT_LUMP,									// A single lump (file)
	WXWT_DEHACKED,								// DeHackEd Patch wrapper
	
	NUMWXWADTYPES
} WX_WADType_t;

/*****************
*** STRUCTURES ***
*****************/

/* WX_WADFile_s -- An Extended WAD File */
struct WX_WADFile_s
{
	/* WAD Info */
	WX_WADType_t WADType;						// Type of WAD File (WAD)
	char* WADBaseName;							// Name of the WAD (DOOM2.WAD)
	char* WADPathName;							// Full path to WAD (/usr/share/games/doom/doom2.wad)
	char CheckSum[WXCHECKSUMSIZE];				// MD5 Sum of WAD
	size_t FileSize;							// Size of WAD File
	FILE* CFile;								// C File for WAD
	
	/* Entries */
	size_t IndexOffset;							// Offset to the index
	WadIndex_t NumLumps;						// Number of lumps in WAD
	WadIndex_t PreLumps;						// Number of lumps presized for
	WX_WADEntry_t* Entries;						// Lump data in WAD
	Z_HashTable_t* HashTable;					// Hash Table for entries
	
	/* Virtual Stuff */
	void* VirtPrivate[NUMWXDATAPRIVATEIDS];		// Virtual Private Data
	size_t VirtSize[NUMWXDATAPRIVATEIDS];		// VPD Size
	void* FormatPrivate;						// Format Private Data
	size_t FormatSize;							// Size of format
	
	/* Virtual Handler Functions */
		// FuncLoadWAD -- Load WAD File
	bool_t (*FuncLoadWAD)(WX_WADFile_t* const a_WAD);
		// FuncUnLoadWAD -- Unloads a WAD File
	bool_t (*FuncUnLoadWAD)(WX_WADFile_t* const a_WAD);
		// FuncReadEntryData -- Reads a single entry from the wad (for cache)
	bool_t (*FuncReadEntryData)(WX_WADFile_t* const a_WAD, WX_WADEntry_t* const a_Entry);
	
	/* Chains */
	WX_WADFile_t* PrevWAD;						// Previous WAD in link
	WX_WADFile_t* NextWAD;						// Next WAD in link
	WX_WADFile_t* VPrevWAD;						// Virtual previous WAD (re-order)
	WX_WADFile_t* VNextWAD;						// Virtual next WAD (re-order)
};

/* WX_WADEntry_s -- An Extended WAD Entry */
struct WX_WADEntry_s
{
	/* Entry Info */
	char* Name;									// Name of the entry
	uint32_t Hash;								// Hash name
	uint32_t Flags;								// Entry flags
	size_t Position;							// Position in file
	size_t Size;								// Size of entry
	WX_WADFile_t* ParentWAD;					// WAD that owns this entry
	WX_WADEntry_t* SymLink;						// Symbolic link to another entry
	
	/* Cache Data */
	void* Cache[NUMWXCONVTYPES];				// Cached Data
	int32_t UsageCount[NUMWXCONVTYPES];			// Times this entry is being used
};

/***************************
*** VIRTUAL WAD HANDLERS ***
***************************/

/*** LMP ***/
/* WX_P_LMP_LoadWAD() -- Load a format */
bool_t WX_P_LMP_LoadWAD(WX_WADFile_t* const a_WAD)
{
	WX_WADEntry_t* SoloEntry;
	char* p;
	
	/* Check */
	if (!a_WAD)
		return false;
	
	/* Add a single entry */
	SoloEntry = WX_AddEntry(a_WAD);
	
	/* Set Data */
	SoloEntry->Name = Z_StrDup(a_WAD->WADBaseName, PU_STATIC, NULL);
	SoloEntry->Position = 0;
	SoloEntry->Size = a_WAD->FileSize;
	
	/* Format name to normal */
	// Uppercase
	C_strupr(SoloEntry->Name);
	
	// Strip . and after
	p = strrchr(SoloEntry->Name, '.');
	
	if (p)
		*p = '\0';
	
	/* Success! */
	return true;
}

/* WX_P_LMP_UnLoadWAD() -- Unload format */
bool_t WX_P_LMP_UnLoadWAD(WX_WADFile_t* const a_WAD)
{
	/* Check */
	if (!a_WAD)
		return false;
	
	// Nothing to do here really
	
	/* Success! */
	return true;
}

/* WX_P_LMP_ReadEntryData() -- Reads a single entry in format */
bool_t WX_P_LMP_ReadEntryData(WX_WADFile_t* const a_WAD, WX_WADEntry_t* const a_Entry)
{
	/* Check */
	if (!a_WAD || !a_Entry)
		return false;
	
	/* Use C Function */
	if (a_WAD->CFile)
	{
		// Set position in lump
		fseek(a_WAD->CFile, a_Entry->Position, SEEK_SET);
		
		// Direct read
		if (a_Entry->Cache[WXCT_RAW])
			fread(a_Entry->Cache[WXCT_RAW], a_Entry->Size, 1, a_WAD->CFile);
	}
	
	/* Success! */
	return true;
}

/*** DEH ***/
/* WX_P_DEH_LoadWAD() -- Load a format */
// Same as LMP, but use DEHACKED instead
bool_t WX_P_DEH_LoadWAD(WX_WADFile_t* const a_WAD)
{
	/* LMP Call */
	if (!WX_P_LMP_LoadWAD(a_WAD))
		return false;
	
	/* Free and set to DEHACKED */
	Z_Free(a_WAD->Entries[0].Name);
	a_WAD->Entries[0].Name = Z_StrDup("DEHACKED", PU_STATIC, NULL);
	
	/* Success! */
	return true;
}

/* WX_P_DEH_UnLoadWAD() -- Unload format */
bool_t WX_P_DEH_UnLoadWAD(WX_WADFile_t* const a_WAD)
{
	/* LMP Call */
	return WX_P_LMP_UnLoadWAD(a_WAD);
}

/* WX_P_DEH_ReadEntryData() -- Reads a single entry in format */
bool_t WX_P_DEH_ReadEntryData(WX_WADFile_t* const a_WAD, WX_WADEntry_t* const a_Entry)
{
	/* LMP Call */
	return WX_P_LMP_ReadEntryData(a_WAD, a_Entry);
}

/*** WAD ***/
/* WX_P_WAD_LoadWAD() -- Load a format */
bool_t WX_P_WAD_LoadWAD(WX_WADFile_t* const a_WAD)
{
	uint32_t HeaderSet[3];
	uint32_t Temp;
	WX_WADEntry_t* NewEntry;
	size_t i, j, k;
	char* p;
	
	/* Check */
	if (!a_WAD)
		return false;

	/* Read WAD Header */
	// Seek to start (just in case)
	fseek(a_WAD->CFile, 0, SEEK_SET);
	
	// Start reading data
	if (fread(&HeaderSet, 12, 1, a_WAD->CFile) < 1)
		return false;
	
	/* Byte swap as needed */
	for (i = 0; i < 3; i++)
		HeaderSet[i] = LittleSwapUInt32(HeaderSet[i]);
		
	/* Perform some checks */
	// Check magic number to see if it is valid
	if (HeaderSet[0] != 1145132873 /* IWAD */ && HeaderSet[0] != 1145132880 /* PWAD */)
		return false;
	
	// Check to see if there are more than zero lumps
	if (HeaderSet[1] == 0)
		return false;
	
	// Check to see that the index offset is within file bounds
	if (HeaderSet[2] >= a_WAD->FileSize)
		return false;
	
	/* Make some corrections */
	// If the index runs off the WAD prevent it from doing as such
	if (HeaderSet[2] + (HeaderSet[1] * 16) >= a_WAD->FileSize)
		HeaderSet[1] = (a_WAD->FileSize - HeaderSet[2]) / 16;
	
	/* Set parent info */
	a_WAD->IndexOffset = HeaderSet[2];
	
	/* Allocate entries */
	WX_PreEntryTable(a_WAD, HeaderSet[1]);
	
	/* Jump to the index and start the read */
	// Seek
	fseek(a_WAD->CFile, a_WAD->IndexOffset, SEEK_SET);
	
	// Create WAD Name Hack
	a_WAD->FormatSize = (sizeof(char) * 9) * (HeaderSet[1] + 1);
	a_WAD->FormatPrivate = Z_Malloc(a_WAD->FormatSize, PU_STATIC, NULL);
	
	// Read
	for (i = 0; i < HeaderSet[1]; i++)
	{
		// Create a new entry
		NewEntry = WX_AddEntry(a_WAD);
		
		// Read in lump position and send to entry
		if (fread(&Temp, 4, 1, a_WAD->CFile) < 1)
			return false;
		Temp = LittleSwapUInt32(Temp);
		NewEntry->Position = Temp;
		
		// Read in lump size and send to entry
		if (fread(&Temp, 4, 1, a_WAD->CFile) < 1)
			return false;
		Temp = LittleSwapUInt32(Temp);
		NewEntry->Size = Temp;
		
		// Read name and slap it into name hack
		p = &((char*)a_WAD->FormatPrivate)[i * 9];
		if (fread(p, 8, 1, a_WAD->CFile) < 1)
			return false;
		NewEntry->Name = p;
		
		// Correct name
		for (j = 0, k = 0; j < 8; j++)
			if (k)
				NewEntry->Name[j] = '\0';
			else
			{
				// Is NUL?
				if (NewEntry->Name[j] == '\0')
				{
					k = 1;
					continue;
				}
				
				// Compression?
				if (j == 0 && NewEntry->Name[j] & 0x80)
				{
					NewEntry->Flags |= WXEF_COMPRESSED;
					NewEntry->Name[j] &= 0x7F;
				}
				
				// Make uppercase
				NewEntry->Name[j] = toupper(NewEntry->Name[j]);
			}
		
		// Offset out of bounds?
		if (NewEntry->Position >= a_WAD->FileSize)
			NewEntry->Position = NewEntry->Size = 0;
		
		// Correct size and offset
		if (NewEntry->Position + NewEntry->Size >= a_WAD->FileSize)
			NewEntry->Size = a_WAD->FileSize - NewEntry->Position;
	}
	
	/* Success! */
	return true;
}

/* WX_P_WAD_UnLoadWAD() -- Unload format */
bool_t WX_P_WAD_UnLoadWAD(WX_WADFile_t* const a_WAD)
{
	size_t i;
	
	/* Check */
	if (!a_WAD)
		return false;
		
	/* Clear Names as they are not controlled by Z_ */
	for (i = 0; i < a_WAD->NumLumps; i++)
		a_WAD->Entries[i].Name = NULL;
	
	/* Delete private data */
	Z_Free(a_WAD->FormatPrivate);
	
	/* Success! */
	return true;
}

/* WX_P_WAD_ReadEntryData() -- Reads a single entry in format */
bool_t WX_P_WAD_ReadEntryData(WX_WADFile_t* const a_WAD, WX_WADEntry_t* const a_Entry)
{
	/* Check */
	if (!a_WAD || !a_Entry)
		return false;
	
	/* Use C Function */
	if (a_WAD->CFile)
	{
		// Set position in lump
		fseek(a_WAD->CFile, a_Entry->Position, SEEK_SET);
		
		// Direct read
		if (a_Entry->Cache[WXCT_RAW])
			fread(a_Entry->Cache[WXCT_RAW], a_Entry->Size, 1, a_WAD->CFile);
	}
	
	/* Success! */
	return true;
}

/*** REMOOD INTERNAL VIRTUAL WAD ***/
/* WX_Handlers_t -- List of function pointers */
typedef struct WX_Handlers_s
{
	bool_t (*FuncLoadWAD)(WX_WADFile_t* const a_WAD);
	bool_t (*FuncUnLoadWAD)(WX_WADFile_t* const a_WAD);
	bool_t (*FuncReadEntryData)(WX_WADFile_t* const a_WAD, WX_WADEntry_t* const a_Entry);
} WX_Handlers_t;

/* l_WXHandlers -- Handlers for file formats */
WX_Handlers_t l_WXHandlers[NUMWXWADTYPES] =
{
	{WX_P_WAD_LoadWAD, WX_P_WAD_UnLoadWAD, WX_P_WAD_ReadEntryData},
	{WX_P_LMP_LoadWAD, WX_P_LMP_UnLoadWAD, WX_P_LMP_ReadEntryData},
	{WX_P_DEH_LoadWAD, WX_P_DEH_UnLoadWAD, WX_P_DEH_ReadEntryData},
};

/*************
*** LOCALS ***
*************/

static char l_SearchList[MAXSEARCHBUFFER][PATH_MAX];	// Places to look for WADs
static size_t l_SearchCount = 0;						// Number of places to look
static WX_WADFile_t* l_FirstWAD = NULL;					// First WAD File
static WX_WADFile_t* l_FirstVWAD = NULL;				// First WAD File seen by game (re-order)
static WX_WADFile_t* l_LastVWAD = NULL;					// Last WAD File seen by game (re-order)

/****************
*** FUNCTIONS ***
****************/

/* WX_Hash() -- Hashes a string */
uint32_t			WX_Hash(const char* const a_Str)
{
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
}

/* WX_BaseName() -- Returns the base name of the WAD */
const char*			WX_BaseName(const char* const a_File)
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

/* WX_BaseExtension() -- Return extension of filename */
const char*			WX_BaseExtension(const char* const a_File)
{
	const char* TempX = NULL;
	
	/* Check */
	if (!a_File)
		return NULL;
		
	/* Get last occurence */
	// Find last period
	TempX = strrchr(a_File, '.');
	
	// Found nothing at all
	if (!TempX)
		TempX = a_File;
	
	/* Check if we landed on a period */
	while (*TempX == '.')
		TempX++;
	
	/* Return pointer */
	return TempX;
}

/* WX_Init() -- Initializes the extended WAD Code */
bool_t				WX_Init(void)
{
	size_t i;
	char* DirArg;

	/* Initialize the search list */
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
			{
				// Copy
				strncpy(l_SearchList[l_SearchCount], DirArg, PATH_MAX);
				
				// Add trailing slash and increment the count
				strncat(l_SearchList[l_SearchCount++], "/", PATH_MAX);
			}
		}
	
	// Debug: Print order of search locations
	if (devparm)
	{
		CONS_Printf("WX_Init: Searching in: ");
		for (i = 0; i < l_SearchCount; i++)
			CONS_Printf("\"%s\"%s", l_SearchList[i], (i < l_SearchCount - 1 ? ", " : ""));
		CONS_Printf("\n");
	}
	
	/* Success! */
	return true;
}

/* WX_LocateWAD() -- Locates an IWAD that matches the name (and optionally) its sum */
// This is alot better than the former W_FindWad() which wasted many CPU cycles calculating the same thing for
// every single WAD we want to find.
bool_t				WX_LocateWAD(const char* const a_Name, const char* const a_MD5, char* const a_OutPath, const size_t a_OutSize)
{
	char CheckBuffer[PATH_MAX];
	char BaseWAD[PATH_MAX];
	const char* p;
	size_t i, j;
	
	/* Name is required and a size must be given if a_OutPath is set */
	if (!a_Name || (a_OutPath && !a_OutSize))
		return false;
	
	/* Look in the search buffer for said WADs */
	for (j = 0; j < 2; j++)
	{
		// Search the exact given name, then the basename
		if (!j)
			p = a_Name;
		else
		{
			memset(BaseWAD, 0, sizeof(BaseWAD));
			strncpy(BaseWAD, WX_BaseName(a_Name), PATH_MAX);
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

/* ZP_EntryHashCompare() -- Compares entry hash */
// A = const char*
// B = WX_WADEntry_t*
static bool_t ZP_EntryHashCompare(void* const a_A, void* const a_B)
{
	const char* A;
	WX_WADEntry_t* B;
	
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

/* WX_LoadWAD() -- Loads a single WAD file */
WX_WADFile_t*		WX_LoadWAD(const char* const a_AutoPath)
{
	char FoundWAD[PATH_MAX];
	WX_WADFile_t* NewWAD, *Rover;
	size_t i;
	
	/* Check */
	if (!a_AutoPath)
		return NULL;
	
	/* Attempt location of WAD */
	memset(FoundWAD, 0, sizeof(FoundWAD));
	if (!WX_LocateWAD(a_AutoPath, 0, FoundWAD, PATH_MAX))
		return NULL;
	
	// Debug, which file we are going to load
	if (devparm)
		CONS_Printf("WX_LoadWAD: Loading \"%s\".\n", FoundWAD);
	
	/* Allocate blank WAD File */
	NewWAD = Z_Malloc(sizeof(*NewWAD), PU_STATIC, NULL);
	
	// Copy fullname and basename
	NewWAD->WADPathName = Z_StrDup(FoundWAD, PU_STATIC, NULL);
	NewWAD->WADBaseName = WX_BaseName(NewWAD->WADPathName);
	
	/* Load the base file and gain some more info */
	NewWAD->CFile = fopen(FoundWAD, "rb");
	
	// worked?
	if (!NewWAD->CFile)
	{
		WX_UnLoadWAD(NewWAD);
		return NULL;
	}
	
	// Obtain file size
	fseek(NewWAD->CFile, 0, SEEK_END);
	NewWAD->FileSize = ftell(NewWAD->CFile);
	fseek(NewWAD->CFile, 0, SEEK_SET);
	
	/* Determine how the WAD should be handled */
	// Explicit WAD
	if (strncasecmp("wad", WX_BaseExtension(NewWAD->WADBaseName), 3) == 0)
		NewWAD->WADType = WXWT_DOOMWAD;
	
	// Explicit DEHACKED
	else if (strncasecmp("deh", WX_BaseExtension(NewWAD->WADBaseName), 3) == 0)
		NewWAD->WADType = WXWT_DEHACKED;
	
	// Try to detect it based on the format
	else
	{
		// Check for WAD
		
		// Failed so treat it as a lump
		NewWAD->WADType = WXWT_LUMP;
	}
	
	/* Loader setup and call */
	// Based on type
	NewWAD->FuncLoadWAD = l_WXHandlers[NewWAD->WADType].FuncLoadWAD;
	NewWAD->FuncUnLoadWAD = l_WXHandlers[NewWAD->WADType].FuncUnLoadWAD;
	NewWAD->FuncReadEntryData = l_WXHandlers[NewWAD->WADType].FuncReadEntryData;
	
	// Call loader
	if (NewWAD->FuncLoadWAD)
		if (!NewWAD->FuncLoadWAD(NewWAD))
		{
			// Loader failed
			WX_UnLoadWAD(NewWAD);
			return NULL;
		}
	
	// Hash all entries
	NewWAD->HashTable = Z_HashCreateTable(ZP_EntryHashCompare);
	for (i = 0; i < NewWAD->NumLumps; i++)
	{
		// Create
		NewWAD->Entries[i].Hash = WX_Hash(NewWAD->Entries[i].Name);
		
		// Add to WAD hash list
		Z_HashAddEntry(NewWAD->HashTable, NewWAD->Entries[i].Hash, &NewWAD->Entries[i]);
	}
	
	/* Load data in WAD that will soon be composited */
	WX_LoadWADStuff(NewWAD);
	
	/* Link into WAD list */
	if (!l_FirstWAD)
		l_FirstWAD = NewWAD;
	else
	{
		Rover = l_FirstWAD;
		
		// Go to last
		while (Rover->NextWAD)
			Rover = Rover->NextWAD;
			
		// Set after last
		Rover->NextWAD = NewWAD;
	}
	
	/* Success */
	// A little message
	if (devparm)
		CONS_Printf("WX_LoadWAD: \"%s\" loaded!\n", NewWAD->WADBaseName);
	
	// Return the freshly made WAD
	return NewWAD;
}

/* WX_UnLoadWAD() -- Unloads a WAD file */
void				WX_UnLoadWAD(WX_WADFile_t* const a_WAD)
{
	/* Check */
	if (!a_WAD)
		return;
		
	// Debug, which file we are going to unload
	if (devparm)
		CONS_Printf("WX_UnLoadWAD: Unloading \"%s\".\n", a_WAD->WADBaseName);
	
	/* If this is a virtually linked WAD, a composite depends on it */
	if (a_WAD->VPrevWAD || a_WAD->VNextWAD)
		WX_ClearComposite();
		
	/* Free anything special for this WAD */
	WX_ClearWADStuff(a_WAD);
	
	/* Unload format specifics */
	if (a_WAD->FuncUnLoadWAD)
		a_WAD->FuncUnLoadWAD(a_WAD);
	
	/* Wipe the entry table */
	WX_WipeEntryTable(a_WAD);
	
	/* Clear normals */
	Z_Free(a_WAD->WADBaseName);
	Z_Free(a_WAD->WADPathName);
	
	/* Unlink WAD */
	// From real chain
	if (a_WAD->PrevWAD)
		a_WAD->PrevWAD->NextWAD = a_WAD->NextWAD;
	if (a_WAD->NextWAD)
		a_WAD->NextWAD->PrevWAD = a_WAD->PrevWAD;
		
	// From virtual chain
	if (a_WAD->VPrevWAD)
		a_WAD->VPrevWAD->VNextWAD = a_WAD->VNextWAD;
	if (a_WAD->VNextWAD)
		a_WAD->VNextWAD->VPrevWAD = a_WAD->VPrevWAD;
	
	// If this WAD is the first real WAD
	if (l_FirstWAD == a_WAD)
		l_FirstWAD = l_FirstWAD->NextWAD;
	
	// If this WAD is the first virtual WAD
	if (l_FirstVWAD == a_WAD)
		l_FirstVWAD = l_FirstVWAD->VNextWAD;
	
	// If this WAD is the last virtual WAD
	if (l_LastVWAD == a_WAD)
		l_LastVWAD = l_LastVWAD->VPrevWAD;
	
	/* Free the current WAD */
	Z_Free(a_WAD);
}

/* WX_PreEntryTable() -- Preallocate entry table */
void				WX_PreEntryTable(WX_WADFile_t* const a_WAD, const size_t a_Count)
{
	/* Check */
	if (!a_WAD || !a_Count)
		return;
	
	/* Check if the entry table actually needs resizing */
	if (a_WAD->PreLumps >= a_Count)
		return;
	
	/* It does, so resize it all */
	Z_ResizeArray(&a_WAD->Entries, sizeof(*a_WAD->Entries), a_WAD->PreLumps, a_Count);
	a_WAD->PreLumps = a_Count;
}

/* WX_AddEntry() -- Adds a single entry and returns a pointer to it */
WX_WADEntry_t*		WX_AddEntry(WX_WADFile_t* const a_WAD)
{
	WX_WADEntry_t* Entry;
	
	/* Check */
	if (!a_WAD)
		return NULL;
		
	/* If there is no more room, then allocate some more */
	if (a_WAD->NumLumps + 1 >= a_WAD->PreLumps)
		WX_PreEntryTable(a_WAD, a_WAD->NumLumps + 1);
	
	/* Set entry */
	Entry = &a_WAD->Entries[a_WAD->NumLumps++];
	
	// Set parent
	Entry->ParentWAD = a_WAD;
	
	/* Success! */
	return Entry;
}

/* WX_WipeEntryTable() -- Delete all entries within a WAD */
void				WX_WipeEntryTable(WX_WADFile_t* const a_WAD)
{
	size_t i, j;
	
	/* Check */
	if (!a_WAD)
		return;
	
	/* Clear table */
	if (a_WAD->Entries)
	{
		// Go through every entry
		for (i = 0; i < a_WAD->NumLumps; i++)
		{
			// Clear data
			for (j = 0; j < NUMWXCONVTYPES; j++)
			{
				// Warn if cache still in use
				if (devparm && a_WAD->Entries[i].UsageCount[j])
					CONS_Printf("WX_WipeEntryTable: Warning, \"%s\" still in use!\n", a_WAD->Entries[i].Name);
			
				// Clear cache
				if (a_WAD->Entries[i].Cache[j])
					Z_Free(a_WAD->Entries[i].Cache[j]);
			}
			
			// Delete Name
			Z_Free(a_WAD->Entries[i].Name);
		}
		
		// Delete table
		Z_Free(a_WAD->Entries);
		a_WAD->PreLumps = a_WAD->NumLumps = 0;
	}
}

/* WX_LoadWADStuff() -- Load stuff from WAD that will soon be part of a composite */
void				WX_LoadWADStuff(WX_WADFile_t* const a_WAD)
{
	/* Check */
	if (!a_WAD)
		return;
	
	V_WXMapGraphicCharsWAD(a_WAD);
	D_WXBuildInfos(DILT_BUILDONE, a_WAD);
}

/* WX_ClearWADStuff() -- Clear stuff from WAD that will soon be part of a composite */
void				WX_ClearWADStuff(WX_WADFile_t* const a_WAD)
{
	/* Check */
	if (!a_WAD)
		return;
	
	V_WXClearGraphicCharsWAD(a_WAD);
	D_WXBuildInfos(DILT_CLEARONE, a_WAD);
}

/* WX_CompileComposite() -- Merge all the loaded WAD Data and create a composite of it */
void				WX_CompileComposite(void)
{
	V_WXMapGraphicCharsComposite(l_FirstVWAD);
	D_WXBuildInfos(DILT_BUILDALL, l_FirstVWAD);
}

/* WX_ClearComposite() -- Clear all of the compositied WAD data */
void				WX_ClearComposite(void)
{
	V_WXClearGraphicCharsComposite();
	D_WXBuildInfos(DILT_CLEARALL, NULL);
}

/* WX_GetNumEntry() -- Gets entry in WAD by lump number */
WX_WADEntry_t*		WX_GetNumEntry(WX_WADFile_t* const a_WAD, const size_t a_Index)
{
	size_t CorrectedIndex;
	
	/* Check */
	if (!a_WAD)
		return NULL;
	
	/* Otherwise */
	// After last (forced overflow)
	if (a_Index == (size_t)-2)
		CorrectedIndex = a_WAD->NumLumps;
	// Do not overflow
	else if (a_Index >= a_WAD->NumLumps - 1)
		CorrectedIndex = a_WAD->NumLumps - 1;
	else
		CorrectedIndex = a_Index;
	
	return &a_WAD->Entries[CorrectedIndex];
}

/* WX_EntryForName() -- Finds an entry based on name */
// If a_WAD is NULL, then all virtual wads are checked in said order
WX_WADEntry_t*		WX_EntryForName(WX_WADFile_t* const a_WAD, const char* const a_Name, const bool_t a_Forwards)
{
	uint32_t SeekHash;
	WX_WADFile_t* Rover;
	WX_WADEntry_t* Found;
	
	/* Check */
	if (!a_Name)
		return NULL;
	
	/* Hash Name */
	SeekHash = WX_Hash(a_Name);
	
	/* Rove WADs */
	if (a_WAD)
		Rover = a_WAD;
	else
		Rover = (a_Forwards ? l_FirstVWAD : l_LastVWAD);
	
	// find it!
	while (Rover)
	{
		// Check hash
		Found = Z_HashFindEntry(Rover->HashTable, SeekHash, a_Name, !a_Forwards);
		
		// If we found it, return
		if (Found)
			return Found;
		
		// Go to the next list
		if (!a_WAD)
			Rover = (a_Forwards ? Rover->VNextWAD : Rover->VPrevWAD);
		
		// Nothing else to do here
		else
			break;
	}
	
	/* Failed */
	return NULL;
}

/* WX_CacheEntry() -- Caches a single entry */
void*				WX_CacheEntry(WX_WADEntry_t* const a_Entry, const WX_ConvType_t a_From, const WX_ConvType_t a_To)
{
	void* RawData;
	void* RetVal;
	size_t From, To;
	
	/* Check */
	if (!a_Entry)
		return NULL;
	
	/* Map From/To */
	From = a_From;
	To = a_To;
	
	if (From >= NUMWXCONVTYPES)
		From = WXCT_RAW;
	if (To >= NUMWXCONVTYPES)
		To = WXCT_RAW;
	
	/* Cache the raw data */
	// Already cached
	if (a_Entry->Cache[WXCT_RAW])
		RawData = a_Entry->Cache[WXCT_RAW];
	
	// Not cached
	else
	{
		// Allocate size needed for cache
		a_Entry->Cache[WXCT_RAW] = Z_Malloc(a_Entry->Size, PU_STATIC, NULL);
		
		// Load in data
		if (!a_Entry->ParentWAD->FuncReadEntryData(a_Entry->ParentWAD, a_Entry))
			return NULL;
		RawData = a_Entry->Cache[WXCT_RAW];
	}
	
	/* Conversion Matrix */
	switch (From)
	{
		/* From RAW */
		case WXCT_RAW:
			switch (To)
			{
				// To RAW
				case WXCT_RAW:
					RetVal = RawData;
					break;
	
				// To patch_t
				case WXCT_PATCH:
	
				// To pic_t
				case WXCT_PIC:
	
				// To Unhandled
				default:
					RetVal = NULL;
					break;
			}
			break;
		
		/* From patch_t */
		case WXCT_PATCH:
			switch (To)
			{
				// To RAW
				case WXCT_RAW:
					RetVal = RawData;
					break;
	
				// To patch_t -- TODO: Implement Propers
				case WXCT_PATCH:
					RetVal = RawData;
					break;
	
				// To pic_t
				case WXCT_PIC:
	
				// To Unhandled
				default:
					RetVal = NULL;
					break;
			}
			break;
		
		/* From pic_t */
		case WXCT_PIC:
			switch (To)
			{
				// To RAW
				case WXCT_RAW:
					RetVal = RawData;
					break;
	
				// To patch_t
				case WXCT_PATCH:
	
				// To pic_t
				case WXCT_PIC:
	
				// To Unhandled
				default:
					RetVal = NULL;
					break;
			}
			break;
		
		/* Unhandled */
		default:
			break;
	}
	
	/* Use it? */
	if (RetVal)
		WX_UseEntry(a_Entry, a_To, true);
	
	return RetVal;
}

/* WX_UseEntry() -- Uses an entry to prevent its free */
size_t				WX_UseEntry(WX_WADEntry_t* const a_Entry, const WX_ConvType_t a_Type, const bool_t a_Use)
{
	/* Check */
	if (!a_Entry || (size_t)a_Type >= (size_t)NUMWXCONVTYPES)
		return 0;
	
	/* Do we use it or not? */
	if (a_Use)
		a_Entry->UsageCount[a_Type]++;
	else
		a_Entry->UsageCount[a_Type]--;
	
	/* Now return the count */
	return a_Entry->UsageCount[a_Type];
}

/* WX_VirtualPushPop() -- Pushes or pops a WAD on the virtual stack */
bool_t				WX_VirtualPushPop(WX_WADFile_t* const a_WAD, const bool_t a_Pop, const bool_t a_Back)
{
	/* A WAD, and pushing -- Add to front or back */
	if (a_WAD && !a_Pop)
	{
		// Do not push if we are linked
		if (a_WAD->VPrevWAD || a_WAD->VNextWAD)
			return false;
		
		// Nothing exists
		if (!l_FirstVWAD)
		{
			l_FirstVWAD = l_LastVWAD = a_WAD;
			return true;
		}
		
		// Pushing to front
		if (!a_Back)
		{
			a_WAD->VNextWAD = l_FirstVWAD;
			l_FirstVWAD->VPrevWAD = a_WAD;
			l_FirstVWAD = a_WAD;
		}
		
		// Pushing to back
		else
		{
			a_WAD->VPrevWAD = l_LastVWAD;
			l_LastVWAD->VNextWAD = a_WAD;
			l_LastVWAD = a_WAD;
		}
		
		return true;
	}
	
	/* A WAD, and popping -- Remove from chain */
	else if (a_WAD && a_Pop)
	{
		// Do not pop if we are unlinked
		if (!a_WAD->VPrevWAD && !a_WAD->VNextWAD)
			return false;
		
		// Fix chain if this is first or last
		if (a_WAD == l_FirstVWAD)
			l_FirstVWAD = l_FirstVWAD->VNextWAD;
		if (a_WAD == l_LastVWAD)
			l_LastVWAD = l_LastVWAD->VPrevWAD;
		
		// Just remove links
		if (a_WAD->VPrevWAD)
			a_WAD->VPrevWAD->VNextWAD = a_WAD->VNextWAD;
		if (a_WAD->VNextWAD)
			a_WAD->VNextWAD->VPrevWAD = a_WAD->VPrevWAD;
		
		// Clear
		a_WAD->VPrevWAD = a_WAD->VNextWAD = NULL;
		
		return true;
	}
	
	/* No WAD, and popping -- Remove front or back */
	else if (!a_WAD && a_Pop)
	{
		// Just call self!
		return WX_VirtualPushPop((!a_Back ? l_FirstVWAD : l_LastVWAD), true, false);
	}
	
	/* Nothing */
	else
		return false;
}

/* WX_GetVirtualPrivateData() -- Return private data in a WAD */
bool_t				WX_GetVirtualPrivateData(WX_WADFile_t* const a_WAD, const WX_DataPrivateID_t a_ID, void*** const a_PPPtr, size_t** const a_PPSize)
{
	/* Check */
	if (!a_WAD || !a_PPPtr || !a_PPSize || (size_t)a_ID >= (size_t)NUMWXDATAPRIVATEIDS)
		return false;
	
	/* Send */
	*a_PPPtr = &a_WAD->VirtPrivate[a_ID];
	*a_PPSize = &a_WAD->VirtSize[a_ID];
	
	return true;
	
}

/* WX_RoveEntry() -- Changes entries */
WX_WADEntry_t*		WX_RoveEntry(WX_WADEntry_t* const a_Entry, const ssize_t a_Next)
{
	/* Check */
	if (!a_Entry)
		return NULL;
	
	/* Return next */
	return a_Entry + a_Next;
}

/* WX_GetEntryName() -- Gets name of entry */
size_t				WX_GetEntryName(WX_WADEntry_t* const a_Entry, char* const a_OutBuf, const size_t a_OutSize)
{
	/* Check */
	if (!a_Entry || !a_OutBuf || !a_OutSize)
		return NULL;
	
	/* Slap into buffer */
	return strncpy(a_OutBuf, a_Entry->Name, a_OutSize);
}

/* WX_GetEntrySize() -- Returns the size of an entry */
size_t				WX_GetEntrySize(WX_WADEntry_t* const a_Entry)
{
	/* Check */
	if (!a_Entry)
		return NULL;
	
	/* Return size here */
	return a_Entry->Size;
}

/* WX_ClearUnused() -- Clears unused lump data */
size_t				WX_ClearUnused(void)
{
	return 0;
}

