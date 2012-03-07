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
// Copyright (C) 2011-2012 GhostlyDeath <ghostlydeath@remood.org>
// Copyright(C) 2000 Simon Howard
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
// DESCRIPTION: Level info.
// Under smmu, level info is stored in the level marker: ie. "mapxx"
// or "exmx" lump. This contains new info such as: the level name, music
// lump to be played, par time etc.

#include <stdio.h>
#include <stdlib.h>

#include "doomstat.h"
#include "doomdef.h"
#include "command.h"
#include "dehacked.h"
#include "dstrings.h"
#include "p_setup.h"
#include "p_info.h"
#include "p_mobj.h"
#include "t_script.h"
#include "w_wad.h"
#include "z_zone.h"
#include "p_local.h"

/*** CONSTANTS ***/

// c_LevelLumpNames -- Names of the level lumps
static const char* c_LevelLumpNames[MAXPLIEDS] =
{
	"",
	"THINGS",
	"LINEDEFS",
	"SIDEDEFS",
	"VERTEXES",
	"SEGS",
	"SSECTORS",
	"NODES",
	"SECTORS",
	"REJECT",
	"BLOCKMAP",
	"BEHAVIOR",
	"TEXTMAP",
	"ENDMAP",
	"RSCRIPTS",
	"LPREVIEW",
};

/* P_PMFIFieldType_t -- Type of thing this represents */
typedef enum P_PMFIFieldType_e
{
	PPMFIFT_STRING,								// char*
	PPMFIFT_INTEGER,							// int32_t
	PPMFIFT_WIDEBITFIELD,						// uint64_t (1 << n)
	PPMFIFT_FIXED,								// fixed_t
	
	MAXPPMFIFIELDTYPES
} P_PMFIFieldType_t;

/*** STRUCTURES ***/

/* P_LevelInfoHolder_t -- Holds level info */
typedef struct P_LevelInfoHolder_s
{
	P_LevelInfoEx_t** Infos;
	size_t NumInfos;
	
	bool_t IWADParsed;							// IWAD info parsed in
	const WL_WADFile_t* WAD;					// WAD File
} P_LevelInfoHolder_t;

/*** GLOBALS ***/
P_LevelInfoEx_t* g_CurrentLevelInfo = NULL;		// Current level being played

/*** LOCALS ***/
static P_LevelInfoEx_t** l_CompInfos = NULL;	// Composite infos
static size_t l_NumCompInfos = 0;				// Number of composites

/*** PRIVATE FUNCTIONS ***/

// c_PMIFields -- Map info fields
static const struct
{
	bool_t IsEnd;								// Parse no more
	P_PMFIFieldType_t Type;						// Type of variable
	const char* MapInfo;
	const char* NewInfo;
	uintptr_t Offset;
} c_PMIFields[MAXPINFOSETFLAGS] =
{
	{false, PPMFIFT_STRING, NULL, "levelname", offsetof(P_LevelInfoEx_t, Title)},
	{false, PPMFIFT_STRING, "author", "creator", offsetof(P_LevelInfoEx_t, Author)},
	{false, PPMFIFT_STRING, "music", "music", offsetof(P_LevelInfoEx_t, Music)},
	{false, PPMFIFT_STRING, "titlepatch", "levelpic", offsetof(P_LevelInfoEx_t, LevelPic)},
	{false, PPMFIFT_STRING, "sky1", "skyname", offsetof(P_LevelInfoEx_t, SkyTexture)},
	{false, PPMFIFT_INTEGER, "par", "partime", offsetof(P_LevelInfoEx_t, ParTime)},
	{false, PPMFIFT_STRING, "exitpic", "interpic", offsetof(P_LevelInfoEx_t, InterPic)},
	{false, PPMFIFT_STRING, "next", "nextlevel", offsetof(P_LevelInfoEx_t, NormalNext)},
	{false, PPMFIFT_STRING, "secretnext", "nextsecret", offsetof(P_LevelInfoEx_t, SecretNext)},
	{false, PPMFIFT_STRING, NULL, "consolecmd", offsetof(P_LevelInfoEx_t, BootCommand)},
	{false, PPMFIFT_WIDEBITFIELD, NULL, "defaultweapons", offsetof(P_LevelInfoEx_t, Weapons)},
	
	/*{false, PPMFIFT_STRING, "music", "music", offsetof(P_LevelInfoEx_t, Music)},
	{false, PPMFIFT_STRING, "music", "music", offsetof(P_LevelInfoEx_t, Music)},
	{false, PPMFIFT_STRING, "music", "music", offsetof(P_LevelInfoEx_t, Music)},
	{false, PPMFIFT_STRING, "music", "music", offsetof(P_LevelInfoEx_t, Music)},
	{false, PPMFIFT_STRING, "music", "music", offsetof(P_LevelInfoEx_t, Music)},
	{false, PPMFIFT_STRING, "music", "music", offsetof(P_LevelInfoEx_t, Music)},
	{false, PPMFIFT_STRING, "music", "music", offsetof(P_LevelInfoEx_t, Music)},*/
	
	// End
	{true, MAXPPMFIFIELDTYPES, NULL, NULL, 0},
};

/* PS_ParseMapInfo() -- Parses map info */
static bool_t PS_ParseMapInfo(P_LevelInfoHolder_t* const a_Holder, const WL_WADEntry_t* a_MIEntry)
{
#define BUFSIZE 512
	char Buf[BUFSIZE];
	char Token[BUFSIZE];
	P_LevelInfoEx_t* CurrentInfo;
	WL_EntryStream_t* Stream;
	size_t i, FNum;
	char* p;
	char* TokenP;
	
	char** StrValP;
	char** StrValS;
	
	int32_t* IntValP;
	int32_t* IntValS;
	
	fixed_t* FixedValP;
	fixed_t* FixedValS;
	
	void* vP;
	void* vS;
	
	// GhostlyDeath <March 6, 2012> -- Non-standard default storage (cool!)
	P_LevelInfoEx_t DefaultStore;
	
	/* Check */
	if (!a_Holder || !a_MIEntry)
		return false;
	
	/* Clear defualt area */
	memset(&DefaultStore, 0, sizeof(DefaultStore));
	
	/* Open Stream */
	Stream = WL_StreamOpen(a_MIEntry);
	
	// Failed to open
	if (!Stream)
		return false;
	
	// Debug
	if (devparm)
		CONL_PrintF("PS_ParseMapInfo: Parsing MAPINFO \"%s\" for \"%s\".\n", a_MIEntry->Name, WL_GetWADName(a_Holder->WAD, false));
	
	// Check unicode
	WL_StreamCheckUnicode(Stream);
	
	/* Parse till the end */
	// Not working on any map
	CurrentInfo = NULL;
	
	// Read constantly
	while (!WL_StreamEOF(Stream))
	{
		// Read line into buffer
		memset(Buf, 0, sizeof(Buf));
		memset(Token, 0, sizeof(Token));
		WL_StreamReadLine(Stream, Buf, BUFSIZE - 1);
		
		// Debug
		CONL_PrintF(">> %s\n", Buf);
		
		// Skip whitespace
		for (p = Buf; *p && (*p == ' ' || *p == '\t'); p++)
			;
		
		// Read first word into token
		for (i = 0; *p && *p != ' ' && *p != '\t'; p++)
			if (i < BUFSIZE)
				Token[i++] = *p;
		
		// Skip whitespace
		for (; *p && (*p == ' ' || *p == '\t'); p++)
			;
		
		// If it is "defaultsettings", it defines the base for everything
		if (strcasecmp(Token, "defaultsettings") == 0)
		{
			// Just set current into to the default
			CurrentInfo = &DefaultStore;
		}
		
		// If it is "map" it is defining a new map
		else if (strcasecmp(Token, "map") == 0)
		{
			// Find out which map to look for
			memset(Token, 0, sizeof(Token));
			for (i = 0; *p && *p != ' ' && *p != '\t'; p++)
				if (i < BUFSIZE)
					Token[i++] = *p;
			
			// Look in holder
			for (i = 0; i < a_Holder->NumInfos; i++)
				if (a_Holder->Infos[i])
					if (strcasecmp(a_Holder->Infos[i]->LumpName, Token) == 0)
					{
						// Debug
						if (devparm)
							CONL_PrintF("PS_ParseMapInfo: Found map \"%s\".\n", Token);
						
						CurrentInfo = a_Holder->Infos[i];
						break;
					}
			
			// No map found?
			if (i >= a_Holder->NumInfos)
			{
				// Debug
				if (devparm)
					CONL_PrintF("PS_ParseMapInfo: Map \"%s\" not found.\n", Token);
				
				CurrentInfo = NULL;
			}
			
			// Copy default settings?
			if (CurrentInfo)
			{
				// For every field...
				for (i = 0; i < !c_PMIFields[i].IsEnd; i++)
				{
					// Check to see if already set or the default is unset
						// But have MAPINFO levels replace each other
					if (CurrentInfo->SetBits[i] > DefaultStore.SetBits[i])
						continue;	// Skip it then
					
					// Set the bit flag then (with MAPINFO level)
					CurrentInfo->SetBits[i] = PLIBL_MAPINFO;
					
					// Get source and dest
					vP = (void*)(((uintptr_t)CurrentInfo) + c_PMIFields[i].Offset);
					vS = (void*)(((uintptr_t)&DefaultStore) + c_PMIFields[i].Offset);
					
					// Which type now?
					switch (c_PMIFields[i].Type)
					{
							// Copy String
						case PPMFIFT_STRING:
							// Get source and dest
							StrValP = (char**)vP;
							StrValS = (char**)vS;
							
							// Duplicate string
							*StrValP = Z_StrDup(*StrValS, PU_STATIC, NULL);
							break;
							
							// Copy Integer
						case PPMFIFT_INTEGER:
							// Get source and dest
							IntValP = (int32_t*)vP;
							IntValS = (int32_t*)vS;
							
							// Copy value
							*IntValP = *IntValS;
							break;
						
							// Unknown
						default:
							break;
					}
				}
			}
		}
		
		// Otherwise look up in a table (above)
		else
		{
			// Look through list for a match
			for (FNum = 0; !c_PMIFields[FNum].IsEnd; FNum++)
				if (c_PMIFields[FNum].MapInfo)
					if (strcasecmp(Token, c_PMIFields[FNum].MapInfo) == 0)
						break;
			
			// Reached end?
			if (c_PMIFields[FNum].IsEnd)
			{
				if (devparm)
					CONL_PrintF("PS_ParseMapInfo: Unknown \"%s\".\n", Token);
				continue;
			}
			
			// No map to modify
			if (!CurrentInfo)
			{
				if (devparm)
					CONL_PrintF("PS_ParseMapInfo: No map yet defined.\n");
				continue;
			}
			
			// Check to see if already set or the default is unset
				// But have MAPINFO levels replace each other
			if (CurrentInfo->SetBits[i] > PLIBL_MAPINFO)
				continue;	// Skip it then
			
			// Set the bit flag then (with MAPINFO level)
			CurrentInfo->SetBits[i] = PLIBL_MAPINFO;
			
			// Get field location
			vP = (void*)(((uintptr_t)CurrentInfo) + c_PMIFields[FNum].Offset);
			
			// Depending on the type, set the value
				// String
			if (c_PMIFields[FNum].Type == PPMFIFT_STRING)
			{
				// Get actual pointer
				StrValP = (char**)vP;
				
				// Set value
				*StrValP = Z_StrDup(p, PU_STATIC, NULL);
				
				// Debug
				if (devparm)
					CONL_PrintF("PS_ParseMapInfo: \"%s\" set to \"%s\".\n", c_PMIFields[FNum].MapInfo, *StrValP);
			}
				// Integer
			else if (c_PMIFields[FNum].Type == PPMFIFT_INTEGER)
			{
				// Get actual pointer
				IntValP = (char**)vP;
				
				// Set value
				*IntValP = atoi(p);
				
				// Debug
				if (devparm)
					CONL_PrintF("PS_ParseMapInfo: \"%s\" set to \'%i\'.\n", c_PMIFields[FNum].MapInfo, *IntValP);
			}
		}
	}
	
	/* Close Stream */
	WL_StreamClose(Stream);
	
	/* Clear default settings */
	for (FNum = 0; !c_PMIFields[FNum].IsEnd; FNum++)
		switch (c_PMIFields[FNum].Type)
		{
				// Delete String
			case PPMFIFT_STRING:
				// Get source
				StrValP = (char**)(((uintptr_t)&DefaultStore) + c_PMIFields[FNum].Offset);
				
				// Is set?
				if (*StrValP)
					Z_Free(*StrValP);
				*StrValP = NULL;
				break;
				
				// Everything else is safe to not worry about
			default:
				break;
		}
	
	/* Success! */
	return true;
#undef BUFSIZE
}

/* PS_LevelInfoGetBlockPoints() -- Locates block points within a file */
static bool_t PS_LevelInfoGetBlockPoints(P_LevelInfoEx_t* const a_Info, const WL_WADEntry_t* a_Entry, WL_EntryStream_t* const a_Stream)
{
#define BUFSIZE 256
	char Buf[BUFSIZE];
	uint16_t Char;
	size_t i, j, k, LineStartPos;
	P_InfoBlockType_t CurrentSpec = NUMPINFOBLOCKTYPES;
	P_InfoBlockType_t NextLineIs = NUMPINFOBLOCKTYPES;
	
	/* Check */
	if (!a_Info || !a_Entry || !a_Stream)
		return false;
	
	/* While there is no stream end */
	while (!WL_StreamEOF(a_Stream))
	{
		// Reset variables
		i = 0;
		memset(Buf, 0, sizeof(Buf));
		LineStartPos = WL_StreamTell(a_Stream);
		
		// Read line into buffer
		do
		{
			// Read it
			Char = WL_StreamReadChar(a_Stream);
			
			// Place it
			if (i < BUFSIZE - 1)
				if (Char != '\n' && Char != '\r')
					Buf[i++] = Char;
		} while (Char != '\n');
		
		// Determine if this is a block specifier
		if (Buf[0] == '[')
		{
			// Ignore any spaces
			for (j = 1; Buf[j] && (Buf[j] == ' ' || Buf[j] == '\t'); j++);
			
			// Find terminating ']'
			for (k = strlen(Buf) - 1; k > 1; k--)
				if (Buf[k] == ']')
				{
					// Delete this and any white space before
					for (; k > 1 && (Buf[k] == ']' || Buf[k] == ' ' || Buf[k] == '\t'); k--)
						Buf[k] = '\0';
					break;
				}
			
			// Never found?
			if (k == 1)
				continue;
			
			// Terminate previous block
			if (CurrentSpec != NUMPINFOBLOCKTYPES)
			{
				// Set end position
				a_Info->BlockPos[CurrentSpec][1] = LineStartPos - 1;
				
				// No longer at this
				CurrentSpec = NUMPINFOBLOCKTYPES;
			}
			
			// Level info?
			if (strcasecmp(&Buf[j], "level info") == 0)
				NextLineIs = PIBT_LEVELINFO;
			
			// Scripts?
			else if (strcasecmp(&Buf[j], "scripts") == 0)
				NextLineIs = PIBT_SCRIPTS;
			
			// Intertext?
			else if (strcasecmp(&Buf[j], "intertext") == 0)
				NextLineIs = PIBT_INTERTEXT;
		}
		
		// If the next line (was then) is a specifier, set it.
		// Process here in case of block specs adjacent to each other, so if
		// they are, they aren't specified at all.
		else if (NextLineIs != NUMPINFOBLOCKTYPES)
		{
			// The position of this block is at
			a_Info->BlockPos[NextLineIs][0] = LineStartPos;
			
			// Set current and clear next
			CurrentSpec = NextLineIs;
			NextLineIs = NUMPINFOBLOCKTYPES;
		}
	}
	
	/* Check for unterminated block */
	if (CurrentSpec != NUMPINFOBLOCKTYPES)
		// Set end position to entry size
		a_Info->BlockPos[CurrentSpec][1] = a_Entry->Size;
	
	/* Debug */
	if (devparm)
	{
		if (a_Info->BlockPos[PIBT_LEVELINFO][1] - a_Info->BlockPos[PIBT_LEVELINFO][0] > 0)
			CONL_PrintF("PS_LevelInfoGetBlockPoints: Level Info at %i - %i (size %i)\n",
					a_Info->BlockPos[PIBT_LEVELINFO][0],
					a_Info->BlockPos[PIBT_LEVELINFO][1],
					a_Info->BlockPos[PIBT_LEVELINFO][1] - a_Info->BlockPos[PIBT_LEVELINFO][0]
				);
		
		if (a_Info->BlockPos[PIBT_SCRIPTS][1] - a_Info->BlockPos[PIBT_SCRIPTS][0] > 0)
			CONL_PrintF("PS_LevelInfoGetBlockPoints: Scripts at %i - %i (size %i)\n",
					a_Info->BlockPos[PIBT_SCRIPTS][0],
					a_Info->BlockPos[PIBT_SCRIPTS][1],
					a_Info->BlockPos[PIBT_SCRIPTS][1] - a_Info->BlockPos[PIBT_SCRIPTS][0]
				);
				
		if (a_Info->BlockPos[PIBT_INTERTEXT][1] - a_Info->BlockPos[PIBT_INTERTEXT][0] > 0)
			CONL_PrintF("PS_LevelInfoGetBlockPoints: Intertext at %i - %i (size %i)\n",
					a_Info->BlockPos[PIBT_INTERTEXT][0],
					a_Info->BlockPos[PIBT_INTERTEXT][1],
					a_Info->BlockPos[PIBT_INTERTEXT][1] - a_Info->BlockPos[PIBT_INTERTEXT][0]
				);
	}
	
	/* Success! */
	return true;
#undef BUFSIZE
}

/*** FUNCTIONS ***/

/* P_WLInfoRemove() -- Function that removes private data from a WAD */
static void P_WLInfoRemove(const WL_WADFile_t* a_WAD)
{
	P_LevelInfoHolder_t* Holder;
	
	/* Check */
	if (!a_WAD)
		return;
	
	/* Get Data */
}


/* P_WLInfoCreator() -- Function that creates private data for a WAD */
static bool_t P_WLInfoCreator(const WL_WADFile_t* const a_WAD, const uint32_t a_Key, void** const a_DataPtr, size_t* const a_SizePtr, WL_RemoveFunc_t* const a_RemoveFuncPtr)
{
	size_t i, j;
	const WL_WADEntry_t* Entry;
	const WL_WADEntry_t* Base;
	const WL_WADEntry_t* MapInfo;
	bool_t LoadLumps;
	bool_t IsDoomHexen;
	P_LevelInfoHolder_t* Holder;
	P_LevelInfoEx_t* CurrentInfo;
	WL_EntryStream_t* ReadStream;
	uint16_t Char;
	
	/* Check */
	if (!a_WAD || !a_DataPtr || !a_SizePtr)
		return false;
	
	/* Create base structures */
	*a_SizePtr = sizeof(P_LevelInfoHolder_t);
	Holder = *a_DataPtr = Z_Malloc(*a_SizePtr, PU_STATIC, NULL);
	
	/* Set holder info */
	Holder->WAD = a_WAD;
	
	/* Debug */
	if (devparm)
		CONL_PrintF("P_WLInfoCreator: Loading level info for \"%s\".\n", WL_GetWADName(a_WAD, false));
	
	/* Seek through lumps looking for levels */
	Base = NULL;
	LoadLumps = false;
	for (i = 0; i < a_WAD->NumEntries; i++)
	{
		// Get current entry
		Entry = &a_WAD->Entries[i];
		
		// Not loading level info lumps
		if (!LoadLumps)
		{
			// Remember it as the base
			if (!Base)
			{
				Base = Entry;
				continue;
			}
		
			// Check to see if this entry is called "THINGS" or "TEXTMAP"
			if (strcasecmp(Entry->Name, c_LevelLumpNames[PLIEDS_THINGS]) == 0)
			{
				// Set Doom/Hexen type
				IsDoomHexen = true;
			}
			else if (strcasecmp(Entry->Name, c_LevelLumpNames[PLIEDS_TEXTMAP]) == 0)
			{
				// Set textual format
				IsDoomHexen = false;
			}
		
			// Not of a known format, so just ignore it
			else
			{
				// Clear base, reset i back, and continue
				Base = NULL;
				i--;
				continue;
			}
			
			// Add to end of list
			Z_ResizeArray((void**)&Holder->Infos, sizeof(*Holder->Infos), Holder->NumInfos, Holder->NumInfos + 1);
			CurrentInfo = Holder->Infos[Holder->NumInfos++] = Z_Malloc(sizeof(*CurrentInfo), PU_STATIC, NULL);
			
			// Initialize new info with base stuff
			strncat(CurrentInfo->LumpName, Base->Name, MAXPLIEXFIELDWIDTH);
			
			CurrentInfo->WAD = a_WAD;
			CurrentInfo->EntryPtr[PLIEDS_HEADER] = Base;
			
			if (IsDoomHexen)
				CurrentInfo->EntryPtr[PLIEDS_THINGS] = Entry;
			else
			{
				CurrentInfo->Type.Text = true;
				CurrentInfo->EntryPtr[PLIEDS_TEXTMAP] = Entry;
			}
			
			// Clear base and start loading lumps
			LoadLumps = true;
			Base = NULL;
		}
		
		// Loading infos
		else
		{
			// If this is a textual format map, stop at ENDMAP
			if (strcasecmp(Entry->Name, c_LevelLumpNames[PLIEDS_ENDMAP]) == 0)
			{
				// Set end map
				CurrentInfo->EntryPtr[PLIEDS_ENDMAP] = Entry;
				
				// Reset variables and read a new map
				LoadLumps = false;
				CurrentInfo = NULL;
				continue;
			}
			
			// Find name match to correlate entries to pointers
			for (j = 1; j < MAXPLIEDS; j++)
				if (strcasecmp(Entry->Name, c_LevelLumpNames[j]) == 0)
				{
					// Place in info
					CurrentInfo->EntryPtr[j] = Entry;
					
					// If it is a behavior lump, set Hexen
					if (j == PLIEDS_BEHAVIOR)
						CurrentInfo->Type.Hexen = true;
					
					// Found something so it is done
					break;
				}
			
			// If this isn't a textual map and a match was not found, back off
			if (!CurrentInfo->Type.Text)
				if (j == MAXPLIEDS)
				{
					// Reset and back off
					LoadLumps = false;
					CurrentInfo = NULL;
					i--;
					continue;
				}
		}
	}
	
	/* Parse MAPINFO (Hexen/ZDoom) */
	// This is done now so the stuff in the lump headers takes precedence.
	if ((MapInfo = WL_FindEntry(a_WAD, 0, "MAPINFO")))
		if (!PS_ParseMapInfo(Holder, MapInfo))
			CONL_PrintF("P_WLInfoCreator: Failed to parse MAPINFO.\n");
	
	/* Parse loaded infos */
	for (i = 0; i < Holder->NumInfos; i++)
	{
		// Set current
		CurrentInfo = Holder->Infos[i];
		
		// Check
		if (!CurrentInfo)
			continue;
		
		// Determine textual playability
		if (CurrentInfo->Type.Text)
		{
			// ReMooD requires node information in textual maps (it does not
			// build them), thus if they do not exist, you cannot play the
			// level at all. This is until node building is implemented.
			if (CurrentInfo->EntryPtr[PLIEDS_SEGS] &&
				CurrentInfo->EntryPtr[PLIEDS_SSECTORS] &&
				CurrentInfo->EntryPtr[PLIEDS_NODES] &&
				CurrentInfo->EntryPtr[PLIEDS_REJECT] &&
				CurrentInfo->EntryPtr[PLIEDS_BLOCKMAP])
			{
				CurrentInfo->Playable = true;
			}
			
			// Not playable
			else
				CONL_PrintF("P_WLInfoCreator: ReMooD requires textual format maps to contain node builder information to be played.\n");
		}
		
		// Determine Doom/Hexen playability
		else
		{
			// Require all required lumps
			if (CurrentInfo->EntryPtr[PLIEDS_THINGS] &&
				CurrentInfo->EntryPtr[PLIEDS_LINEDEFS] &&
				CurrentInfo->EntryPtr[PLIEDS_SIDEDEFS] &&
				CurrentInfo->EntryPtr[PLIEDS_VERTEXES] &&
				CurrentInfo->EntryPtr[PLIEDS_SEGS] &&
				CurrentInfo->EntryPtr[PLIEDS_SSECTORS] &&
				CurrentInfo->EntryPtr[PLIEDS_NODES] &&
				CurrentInfo->EntryPtr[PLIEDS_SECTORS] &&
				CurrentInfo->EntryPtr[PLIEDS_REJECT] &&
				CurrentInfo->EntryPtr[PLIEDS_BLOCKMAP])
			{
				CurrentInfo->Playable = true;
			}
		}
		
		// Read header lump for Legacy map information
		ReadStream = WL_StreamOpen(CurrentInfo->EntryPtr[PLIEDS_HEADER]);
		
		// Worked?
		if (ReadStream)
		{
			// Determine unicode level
			WL_StreamCheckUnicode(ReadStream);
			
			// Determine block locations
			if (PS_LevelInfoGetBlockPoints(CurrentInfo, CurrentInfo->EntryPtr[PLIEDS_HEADER], ReadStream))
			{
				// Parse [level info]
			}
		}
		
		// Destroy stream
		WL_StreamClose(ReadStream);
		
		// Debug
		if (devparm)
			CONL_PrintF("P_WLInfoCreator: \"%s\" is %s and %s.\n",
					CurrentInfo->LumpName,
					(CurrentInfo->Type.Text ? "textual" : (CurrentInfo->Type.Hexen ? "Hexen" : "Doom")),
					(CurrentInfo->Playable ? "is playable" : "cannot be played")
				);
	}
	
	/* Success! */
	return true;
}

/* PS_WLInfoOCCB() -- Handles OCCB for the Level Info Code */
static bool_t PS_WLInfoOCCB(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD)
{
	const WL_WADFile_t* Rover;
	P_LevelInfoHolder_t* Holder;
	const WL_WADEntry_t* Entry;
	P_LevelInfoEx_t* CurrentInfo;
	P_LevelInfoEx_t* CompInfo;
	P_LevelInfoEx_t** CompSpot;
	size_t Sz, i;
	
	/* Debug */
	if (devparm)
		CONL_PrintF("PS_WLInfoOCCB: Building composite...\n");
	
	/* First, The IWAD must get their MAPINFOs loaded from remood.wad */
	// IWAD is always first
	Rover = WL_IterateVWAD(NULL, true);
	
	// Get holder
	Holder = WL_GetPrivateData(Rover, WLDK_MAPINFO, &Sz);
	
	// Is this an IWAD? And is the data not loaded?
	if (Holder && Rover->__Private.__IsIWAD && !Holder->IWADParsed)
	{
		// Find MAPINFO used for this IWAD
		Entry = WL_FindEntry(g_ReMooDPtr, 0, g_IWADMapInfoName);
		
		// Parse MAPINFO for this IWAD
		if (Entry)
			if (!PS_ParseMapInfo(Holder, Entry))
				CONL_PrintF("PS_WLInfoOCCB: Failed to load IWAD MAPINFO!\n");
		
		// Set as parsed
		Holder->IWADParsed = true;
	}
	
	/* Clear level info composite */
	if (l_CompInfos)
		Z_Free(l_CompInfos);
	l_CompInfos = NULL;
	l_NumCompInfos = 0;
	
	/* Now merge all the level information into a composite form */
	for (Rover = WL_IterateVWAD(NULL, true); Rover; Rover = WL_IterateVWAD(Rover, true))
	{
		// Get holder for this WAD
		Holder = WL_GetPrivateData(Rover, WLDK_MAPINFO, &Sz);
		
		// No holder?
		if (!Holder)
			continue;
		
		// Go through each map
		for (i = 0; i < Holder->NumInfos; i++)
		{
			// Get current info
			CurrentInfo = Holder->Infos[i];
			
			// No info?
			if (!CurrentInfo)
				continue;
			
			// Level not playable?
			if (!CurrentInfo->Playable)
				continue;
			
			// See if it exists in the composite
			CompInfo = P_FindLevelByNameEx(CurrentInfo->LumpName, &CompSpot);
			
			// It does
			if (CompInfo)
			{
				// Replace spot with this map
				*CompSpot = CurrentInfo;
			}
			
			// Does not
			else
			{
				// Resize array
				Z_ResizeArray((void**)&l_CompInfos, sizeof(*l_CompInfos), l_NumCompInfos, l_NumCompInfos + 1);
				
				// Place at end
				l_CompInfos[l_NumCompInfos++] = CurrentInfo;
			}
		}
	}
	
	/* Success! */
	return true;
}

/* PCLC_Maps() -- Print maps available */
static CONL_ExitCode_t PCLC_Maps(const uint32_t a_ArgC, const char** const a_ArgV)
{
	size_t i;
	
	/* No maps? */
	if (!l_NumCompInfos)
		return CLE_RESOURCENOTFOUND;
	
	/* Go through list */
	for (i = 0; i < l_NumCompInfos; i++)
		if (l_CompInfos[i])
		{
			// Print map name and the WAD it is in
			CONL_PrintF("{9%s{z ({4%s{z)", l_CompInfos[i]->LumpName, WL_GetWADName(l_CompInfos[i]->WAD, false));
			
			// Next line? (two per line)
			if (((i + 1) & 1) == 0)
				CONL_PrintF("{z\n");
			
			// Comma?
			else if (i < (l_NumCompInfos - 1))
				CONL_PrintF("{z, ");
		}
	
	/* Print end line */
	CONL_PrintF("\n");
	
	/* Always success */
	return CLE_SUCCESS;
}

/* PCLC_MapInfo() -- Prints all available information on a map */
static CONL_ExitCode_t PCLC_MapInfo(const uint32_t a_ArgC, const char** const a_ArgV)
{
	P_LevelInfoEx_t* Map;
	
	/* Check */
	if (a_ArgC < 2)
		return CLE_INVALIDARGUMENT;
	
	/* Locate map */
	Map = P_FindLevelByNameEx(a_ArgV[1], NULL);
	
	// Not found?
	if (!Map)
		return CLE_RESOURCENOTFOUND;
	
	/* Print information on map */
	CONL_PrintF("{zFormat of map is {3%s{z.\n", (Map->Type.Text ? "Textual" : (Map->Type.Hexen ? "Hexen" : "Doom")));
	
	CONL_PrintF("{1Title   {z: {9%s{z\n", Map->Title);
	CONL_PrintF("{1Author  {z: {9%s{z\n", Map->Author);
	CONL_PrintF("{1Date    {z: {9%2i{z/{9%2i{z/{9%4i{z\n", Map->Date.Month, Map->Date.Day, Map->Date.Year);
	
	/*CONL_PrintF("{1  {z: {9%s{z\n", Map->);
	CONL_PrintF("{1  {z: {9%s{z\n", Map->);
	CONL_PrintF("{1  {z: {9%s{z\n", Map->);
	CONL_PrintF("{1  {z: {9%s{z\n", Map->);
	CONL_PrintF("{1  {z: {9%s{z\n", Map->);
	CONL_PrintF("{1  {z: {9%s{z\n", Map->);
	CONL_PrintF("{1  {z: {9%s{z\n", Map->);
	CONL_PrintF("{1  {z: {9%s{z\n", Map->);
	CONL_PrintF("{1  {z: {9%s{z\n", Map->);
	CONL_PrintF("{1  {z: {9%s{z\n", Map->);
	CONL_PrintF("{1  {z: {9%s{z\n", Map->);
	CONL_PrintF("{1  {z: {9%s{z\n", Map->);
	CONL_PrintF("{1  {z: {9%s{z\n", Map->);
	CONL_PrintF("{1  {z: {9%s{z\n", Map->);
	CONL_PrintF("{1  {z: {9%s{z\n", Map->);
	CONL_PrintF("{1  {z: {9%s{z\n", Map->);
	CONL_PrintF("{1  {z: {9%s{z\n", Map->);
	CONL_PrintF("{1  {z: {9%s{z\n", Map->);
	CONL_PrintF("{1  {z: {9%s{z\n", Map->);
	CONL_PrintF("{1  {z: {9%s{z\n", Map->);*/
	
	/* Success! */
	return CLE_SUCCESS;
}

/* P_PrepareLevelInfoEx() -- Prepare extended level info */
void P_PrepareLevelInfoEx(void)
{
	/* Register handler into the light WAD code */
	// This loads all the levels for each WAD
	if (!WL_RegisterPDC(WLDK_MAPINFO, WLDPO_MAPINFO, P_WLInfoCreator, P_WLInfoRemove))
		I_Error("P_PrepareLevelInfoEx: Failed to register info creator!");
	
	// This merges together the info (and attempts to load IWAD MAPINFOs into IWADs)
	if (!WL_RegisterOCCB(PS_WLInfoOCCB, 60))
		I_Error("P_PrepareLevelInfoEx: Failed to register info OCCB!");
	
	/* Add console commands */
	CONL_AddCommand("maps", PCLC_Maps);
	CONL_AddCommand("mapinfo", PCLC_MapInfo);
}

/* P_FindLevelByNameEx() -- Finds level by its lump name */
P_LevelInfoEx_t* P_FindLevelByNameEx(const char* const a_Name, P_LevelInfoEx_t*** const a_LocPtr)
{
	size_t i;
	
	/* Check */
	if (!a_Name)
		return NULL;
	
	/* Clear location */
	if (a_LocPtr)
		*a_LocPtr = NULL;
	
	/* Look into composite */
	for (i = 0; i < l_NumCompInfos; i++)
		if (l_CompInfos[i])
			if (strcasecmp(l_CompInfos[i]->LumpName, a_Name) == 0)
			{
				// Set location
				if (a_LocPtr)
					*a_LocPtr =	&l_CompInfos[i];
				
				return l_CompInfos[i];	// Found it!
			}
	
	/* Not found */
	return NULL;
}

/*******************************************************************************
********************************************************************************
*******************************************************************************/

//----------------------------------------------------------------------------
//
// Helper functions
//

void P_LowerCase(char* line)
{
	char* temp;
	
	for (temp = line; *temp; temp++)
		*temp = tolower(*temp);
}

static void P_RemoveComments(char* line)
{
	char* temp = line;
	
	while (*temp)
	{
		if (*temp == '/' && *(temp + 1) == '/')
		{
			*temp = '\0';
			return;
		}
		temp++;
	}
}

static void P_RemoveEqualses(char* line)
{
	char* temp;
	
	temp = line;
	
	while (*temp)
	{
		if (*temp == '=')
		{
			*temp = ' ';
		}
		temp++;
	}
}

//----------------------------------------------------------------------------
//
//  Level vars: level variables in the [level info] section.
//
//  Takes the form:
//     [variable name] = [value]
//
//  '=' sign is optional: all equals signs are internally turned to spaces
//

char* info_interpic;
char* info_levelname;
int info_partime;
char* info_music;
char* info_skyname;
char* info_creator;
char* info_levelpic;
char* info_nextlevel;
char* info_nextsecret;
char* info_intertext = NULL;
char* info_backdrop;
char* info_weapons;
int info_scripts;				// has the current level got scripts?
int gravity;

enum
{
	IVT_STRING,
	IVT_INT,
	IVT_CONSOLECMD,				// SoM: W00t I RULE
	IVT_END
};

typedef struct
{
	int type;
	char* name;
	void* variable;
} levelvar_t;

levelvar_t levelvars[] =
{
	{IVT_STRING, "levelpic", &info_levelpic},
	{IVT_STRING, "levelname", &info_levelname},
	{IVT_INT, "partime", &info_partime},
	{IVT_STRING, "music", &info_music},
	{IVT_STRING, "skyname", &info_skyname},
	{IVT_STRING, "creator", &info_creator},
	{IVT_STRING, "interpic", &info_interpic},
	{IVT_STRING, "nextlevel", &info_nextlevel},
	{IVT_STRING, "nextsecret", &info_nextsecret},
	{IVT_INT, "gravity", &gravity},
	{IVT_STRING, "inter-backdrop", &info_backdrop},
	{IVT_STRING, "defaultweapons", &info_weapons},
	{IVT_CONSOLECMD, "consolecmd", NULL},
	{IVT_END, 0, 0}
};

void P_ParseLevelVar(char* cmd)
{
	char varname[50];
	char* equals;
	levelvar_t* current;
	
	if (!*cmd)
		return;
		
	P_RemoveEqualses(cmd);
	
	// right, first find the variable name
	
	sscanf(cmd, "%s", varname);
	
	// find what it equals
	equals = cmd + strlen(varname);
	while (*equals == ' ')
		equals++;				// cut off the leading spaces
		
	current = levelvars;
	
	while (current->type != IVT_END)
	{
		if (!strcasecmp(current->name, varname))
		{
			switch (current->type)
			{
				case IVT_STRING:
					*(char**)current->variable	// +5 for safety
					= Z_Malloc(strlen(equals) + 5, PU_LEVEL, NULL);
					strcpy(*(char**)current->variable, equals);
					break;
					
				case IVT_INT:
					*(int*)current->variable = atoi(equals);
					break;
				case IVT_CONSOLECMD:
					{
						char t[256];
						
						sprintf(t, "%s\n", equals);
						COM_BufAddText(t);
					}
					break;
			}
		}
		current++;
	}
}

// clear all the level variables so that none are left over from a
// previous level

int isExMy(char* name)
{
	if (strlen(name) != 4)
		return 0;
		
	if (toupper(name[0]) != 'E' || toupper(name[2]) != 'M' || !isnumchar(name[1]) || !isnumchar(name[3]) || name[4] != '\0')
		return 0;
	return 1;
}

int isMAPxy(char* name)
{
	if (strlen(name) != 5)
		return 0;
		
	if (toupper(name[0]) != 'M' || toupper(name[1]) != 'A' || toupper(name[2]) != 'P' || !isnumchar(name[3]) || !isnumchar(name[4]) || name[5] != '\0')
		return 0;
	return 1;
}

void P_ClearLevelVars()
{
	info_levelname = info_skyname = info_levelpic = info_interpic = "";
	info_music = "";
	info_creator = "unknown";
	info_partime = -1;
	
	if (gamemode == commercial && isExMy(levelmapname))
	{
		static char nextlevel[10];
		
		info_nextlevel = nextlevel;
		
		// set the next episode
		strcpy(nextlevel, levelmapname);
		nextlevel[3]++;
		if (nextlevel[3] > '9')	// next episode
		{
			nextlevel[3] = '1';
			nextlevel[1]++;
		}
		
		info_music = levelmapname;
	}
	else
		info_nextlevel = "";
		
	info_nextsecret = "";
	
	info_weapons = "";
	gravity = FRACUNIT;			// default gravity
	
	if (info_intertext)
	{
		Z_Free(info_intertext);
		info_intertext = NULL;
	}
	
	info_backdrop = NULL;
	
	T_ClearScripts();
	info_scripts = false;
}

//----------------------------------------------------------------------------
//
// P_ParseScriptLine
//
// FraggleScript: if we are reading in script lines, we add the new lines
// into the levelscript
//

//SoM: Dynamic limit set by lumpsize...
int maxscriptsize = -1;

void P_ParseScriptLine(char* line)
{
	if (levelscript.data[0] == 0)
	{
		Z_Free(levelscript.data);
		levelscript.data = Z_Malloc(maxscriptsize, PU_LEVEL, 0);
		levelscript.data[0] = '\0';
	}
	
	if ((int)(strlen(levelscript.data) + strlen(line)) > maxscriptsize)
		I_Error("Script larger than script lump???\n");
		
	// add the new line to the current data using sprintf (ugh)
	sprintf(levelscript.data, "%s%s\n", levelscript.data, line);
}

//-------------------------------------------------------------------------
//
// P_ParseInterText
//
// Add line to the custom intertext
//

void P_ParseInterText(char* line)
{
	while (*line == ' ')
		line++;
	if (!*line)
		return;
		
	if (info_intertext)
	{
		int textlen = strlen(info_intertext);
		
		if (textlen + 1 > maxscriptsize)
			I_Error("Intermission text bigger than LUMP?\n");
			
		// newline
		info_intertext[textlen] = '\n';
		
		// add line to end
		sprintf(info_intertext, "%s\n%s", info_intertext, line);
	}
	else
	{
		info_intertext = Z_Malloc(maxscriptsize, PU_STATIC, 0);
		strcpy(info_intertext, line);
	}
}

//---------------------------------------------------------------------------
//
// Setup/Misc. Functions
//

bool_t default_weaponowned[NUMWEAPONS];

void P_InitWeapons()
{
	char* s;
	
	memset(default_weaponowned, 0, sizeof(default_weaponowned));
	
	s = info_weapons;
	
	while (*s)
	{
		switch (*s)
		{
			case '3':
				default_weaponowned[wp_shotgun] = true;
				break;
			case '4':
				default_weaponowned[wp_chaingun] = true;
				break;
			case '5':
				default_weaponowned[wp_missile] = true;
				break;
			case '6':
				default_weaponowned[wp_plasma] = true;
				break;
			case '7':
				default_weaponowned[wp_bfg] = true;
				break;
			case '8':
				default_weaponowned[wp_supershotgun] = true;
				break;
			default:
				break;
		}
		s++;
	}
}

//SoM: Moved from hu_stuff.c
#define HU_TITLE  (text[HUSTR_E1M1_NUM + (gameepisode-1)*9+gamemap-1])
#define HU_TITLE2 (text[HUSTR_1_NUM + gamemap-1])
#define HU_TITLEP (text[PHUSTR_1_NUM + gamemap-1])
#define HU_TITLET (text[THUSTR_1_NUM + gamemap-1])
#define HU_TITLEH (text[HERETIC_E1M1_NUM + (gameepisode-1)*9+gamemap-1])

unsigned char* levelname;

void P_FindLevelName()
{
	extern char* maplumpname;
	
	// determine the level name
	// there are a number of sources from which it can come from,
	// getting the right one is the tricky bit =)
	// info level name from level lump (p_info.c) ?
	
	if (*info_levelname)
		levelname = info_levelname;
	// not a new level or dehacked level names ?
	else if (!newlevel || deh_loaded)
	{
		if (isMAPxy(maplumpname))
			levelname = gamemission == pack_tnt ? HU_TITLET : gamemission == pack_plut ? HU_TITLEP : HU_TITLE2;
		else if (isExMy(maplumpname))
			levelname = HU_TITLE;
		else
			levelname = maplumpname;
	}
	else						//  otherwise just put "new level"
	{
		static char newlevelstr[50];
		
		sprintf(newlevelstr, "%s: new level", maplumpname);
		levelname = newlevelstr;
	}
}

//-------------------------------------------------------------------------
//
// P_ParseInfoCmd
//
// We call the relevant function to deal with the line we are given,
// based on readtype. If we get a section divider ([] bracketed) we
// change readtype.
//

enum
{
	RT_LEVELINFO,
	RT_SCRIPT,
	RT_OTHER,
	RT_INTERTEXT
} readtype;

void P_ParseInfoCmd(char* line)
{
	if (!*line)
		return;
		
	if (readtype != RT_SCRIPT)	// not for scripts
	{
		//      P_LowerCase(line);
		while (*line == ' ')
			line++;
		if (!*line)
			return;
		if ((line[0] == '/' && line[1] == '/') ||	// comment
		        line[0] == '#' || line[0] == ';')
			return;
	}
	
	if (*line == '[')			// a new section seperator
	{
		line++;
		if (!strncasecmp(line, "level info", 10))
			readtype = RT_LEVELINFO;
		if (!strncasecmp(line, "scripts", 7))
		{
			readtype = RT_SCRIPT;
			info_scripts = true;	// has scripts
		}
		if (!strncasecmp(line, "intertext", 9))
			readtype = RT_INTERTEXT;
		return;
	}
	
	switch (readtype)
	{
		case RT_LEVELINFO:
			P_ParseLevelVar(line);
			break;
			
		case RT_SCRIPT:
			P_ParseScriptLine(line);
			break;
			
		case RT_INTERTEXT:
			P_ParseInterText(line);
			break;
			
		case RT_OTHER:
			break;
	}
}

//-------------------------------------------------------------------------
//
// P_LoadLevelInfo
//
// Load the info lump for a level. Call P_ParseInfoCmd for each
// line of the lump.

void P_LoadLevelInfo(int lumpnum)
{
	char* lump;
	char* readline;
	int lumpsize;
	
	readtype = RT_OTHER;
	P_ClearLevelVars();
	
	lumpsize = maxscriptsize = W_LumpLength(lumpnum);
	readline = Z_Malloc(lumpsize + 1, PU_STATIC, 0);
	readline[0] = '\0';
	
	if (lumpsize > 0)
	{
		rover = lump = W_CacheLumpNum(lumpnum, PU_STATIC);
		while (rover < lump + lumpsize)
		{
			if (*rover == '\n')	// end of line
			{
				P_ParseInfoCmd(readline);	// parse line
				readline[0] = '\0';
			}
			else
				// add to line if valid char
				if (isprint(*rover) || *rover == '{' || *rover == '}')
				{
					// add char
					readline[strlen(readline) + 1] = '\0';
					readline[strlen(readline)] = *rover;
				}
				
			rover++;
		}
		
		// parse last line
		P_ParseInfoCmd(readline);
		Z_Free(lump);
	}
	Z_Free(readline);
	
	P_InitWeapons();
	P_FindLevelName();
	
	//Set the gravity for the level!
	if (cv_gravity.value != gravity)
	{
		if (gravity != FRACUNIT)
			COM_BufAddText(va("gravity %f\n", ((double)gravity) / 100));
		else
			COM_BufAddText(va("gravity %f\n", ((double)gravity) / FRACUNIT));
	}
	
	COM_BufExecute();			//Hurdler: flush the command buffer
	return;
}

//-------------------------------------------------------------------------
//
// Console Commands
//

void COM_Creator_f(void)
{
	CONL_PrintF("%s\n", info_creator);
}

void COM_Levelname_f(void)
{
	CONL_PrintF("%s\n", levelname);
}

void P_Info_AddCommands()
{
	COM_AddCommand("creator", COM_Creator_f);
	COM_AddCommand("levelname", COM_Levelname_f);
}

char* P_LevelName()
{
	return levelname;
}

// todo : make this use mapinfo lump
char* P_LevelNameByNum(int episode, int map)
{
	switch (gamemode)
	{
		case shareware:
		case registered:
		case retail:
			return text[HUSTR_E1M1_NUM + (episode - 1) * 9 + map - 1];
		case commercial:
			switch (gamemission)
			{
				case pack_tnt:
					return text[THUSTR_1_NUM + map - 1];
				case pack_plut:
					return text[PHUSTR_1_NUM + map - 1];
				default:
					return text[HUSTR_1_NUM + map - 1];
			}
		default:
			break;
	}
	return "New map";
}
