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
	PPMFIFT_BOOL,								// bool_t
	
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

#define NUMPINFOIGNOREFIELDS 0

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
	{false, PPMFIFT_STRING, "creator", "creator", offsetof(P_LevelInfoEx_t, Author)},
	{false, PPMFIFT_STRING, "music", "music", offsetof(P_LevelInfoEx_t, Music)},
	{false, PPMFIFT_STRING, "titlepatch", "levelpic", offsetof(P_LevelInfoEx_t, LevelPic)},
	{false, PPMFIFT_STRING, "sky1", "skyname", offsetof(P_LevelInfoEx_t, SkyTexture)},
	{false, PPMFIFT_INTEGER, "par", "partime", offsetof(P_LevelInfoEx_t, ParTime)},
	{false, PPMFIFT_STRING, "exitpic", "interpic", offsetof(P_LevelInfoEx_t, InterPic)},
	{false, PPMFIFT_STRING, "intermusic", "intermusic", offsetof(P_LevelInfoEx_t, InterMus)},
	{false, PPMFIFT_STRING, "next", "nextlevel", offsetof(P_LevelInfoEx_t, NormalNext)},
	{false, PPMFIFT_STRING, "secretnext", "nextsecret", offsetof(P_LevelInfoEx_t, SecretNext)},
	{false, PPMFIFT_STRING, NULL, "consolecmd", offsetof(P_LevelInfoEx_t, BootCommand)},
	{false, PPMFIFT_WIDEBITFIELD, NULL, "defaultweapons", offsetof(P_LevelInfoEx_t, Weapons)},
	
	{false, PPMFIFT_INTEGER, "levelnum", "mapnumber", offsetof(P_LevelInfoEx_t, LevelNum)},
	{false, PPMFIFT_INTEGER, "cluster", "episodenumber", offsetof(P_LevelInfoEx_t, EpisodeNum)},
	
	// Level special endings
	{false, PPMFIFT_BOOL, "map07special", NULL, offsetof(P_LevelInfoEx_t, MapSevenSpecial)},
	{false, PPMFIFT_BOOL, "baronspecial", NULL, offsetof(P_LevelInfoEx_t, BaronSpecial)},
	{false, PPMFIFT_BOOL, "cyberdemonspecial", NULL, offsetof(P_LevelInfoEx_t, CyberSpecial)},
	{false, PPMFIFT_BOOL, "spidermastermindspecial", NULL, offsetof(P_LevelInfoEx_t, SpiderdemonSpecial)},
	{false, PPMFIFT_BOOL, "specialaction_exitlevel", NULL, offsetof(P_LevelInfoEx_t, ExitOnSpecial)},
	{false, PPMFIFT_BOOL, "specialaction_opendoor", NULL, offsetof(P_LevelInfoEx_t, OpenDoorOnSpecial)},
	{false, PPMFIFT_BOOL, "specialaction_lowerfloor", NULL, offsetof(P_LevelInfoEx_t, LowerFloorOnSpecial)},
	{false, PPMFIFT_BOOL, "specialaction_killmonsters", NULL, offsetof(P_LevelInfoEx_t, KillMonstersOnSpecial)},
	
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

/* PS_ParseLumpHeader() -- Parses lump header */
static bool_t PS_ParseLumpHeader(P_LevelInfoEx_t* const a_CurrentInfo, WL_ES_t* const a_Stream)
{
#define BUFSIZE 512
	char Buf[BUFSIZE];
	char* SideA, *SideB;
	int32_t n;
	size_t i, FNum;
	
	char* p, *q;
	char* TokenP;
	
	char** StrValP;
	char** StrValS;
	
	int32_t* IntValP;
	int32_t* IntValS;
	
	fixed_t* FixedValP;
	fixed_t* FixedValS;
	
	bool_t* BoolValP;
	bool_t* BoolValS;
	
	void* vP;
	void* vS;
	
	/* Check */
	if (!a_CurrentInfo || !a_Stream)
		return 0;
	
	/* Go back to start of info */
	WL_StreamSeek(a_Stream, a_CurrentInfo->BlockPos[PIBT_LEVELINFO][0], false);
	
	/* Keep reading lines */
	for (;;)
	{
		// EOS?
		if (WL_StreamEOF(a_Stream))
			break;
		
		// Read a fresh line from the buffer
		memset(Buf, 0, sizeof(Buf));
		WL_Srl(a_Stream, Buf, BUFSIZE - 1);
		
		// Now past the end?
		if (WL_StreamTell(a_Stream) > a_CurrentInfo->BlockPos[PIBT_LEVELINFO][1])
			break;
		
		// Debug
		if (devparm)
			CONL_PrintF("PS_ParseLumpHeader: \"%s\"\n", Buf);
		
		// Remove leading whitespace
		while (Buf[0] == ' ' || Buf[0] == '\t' || Buf[0] == '\r' || Buf[0] == '\n')
			memmove(&Buf[0], &Buf[1], BUFSIZE - 1);
		
		// Completely blank line?
		if (!Buf[0])
			continue;
		
		// Comment?
		if (Buf[0] == '/' && Buf[1] == '/')
			continue;
		
		// Determine sides
		SideA = Buf;				// Always the start
		SideB = strchr(Buf, '=');	// Find equal sign
		
		// No equal sign?
		if (!SideB)
			continue;
		
		// Set the start of side b to NULL
		*(SideB++) = 0;
		
		// Remove beginning white space on b side
		while (SideB[0] == ' ' || SideB[0] == '\t' || SideB[0] == '\r' || SideB[0] == '\n')
			SideB++;
		
		// Remove whitespace at end of sides
		// A
		for (n = strlen(SideA);
			n >= 1 && (SideA[n - 1] == ' ' || SideA[n - 1] == '\t' ||
			SideA[n - 1] == '\r' || SideA[n - 1] == '\n'); n--)
			SideA[n - 1] = 0;
		
		// B
		for (n = strlen(SideB);
			n >= 1 && (SideB[n - 1] == ' ' || SideB[n - 1] == '\t' ||
			SideB[n - 1] == '\r' || SideB[n - 1] == '\n'); n--)
			SideB[n - 1] = 0;
		
		/* Debug */
		if (devparm)
			CONL_PrintF("PS_ParseLumpHeader: \"%s\" == \"%s\"\n", SideA, SideB);
			
		// Look through list for a match
		for (FNum = 0; !c_PMIFields[FNum].IsEnd; FNum++)
			if (c_PMIFields[FNum].NewInfo)
				if (strcasecmp(SideA, c_PMIFields[FNum].NewInfo) == 0)
					break;
		
		// Reached end?
		if (c_PMIFields[FNum].IsEnd)
		{
			if (devparm)
				CONL_PrintF("PS_ParseLumpHeader: Unknown \"%s\".\n", SideA);
			continue;
		}
		
		// Check to see if already set or the default is unset
			// But have MAPINFO levels replace each other
		if (a_CurrentInfo->SetBits[FNum] > PLIBL_LUMPHEADER)
		{
			if (devparm)
				CONL_PrintF("PS_ParseLumpHeader: Already defined at higher level (this: %i, at: %i).\n", PLIBL_MAPINFO, a_CurrentInfo->SetBits[FNum]);
			continue;	// Skip it then
		}
		
		// Setup
		p = SideB;
		
		// GhostlyDeath <May 5, 2012> -- Ignore any quotes
		if (*p == '\"')
		{
			// Null the quote away
			*(p++) = '\0';
			
			// Find the next quote, null that then die
			for (q = p; *q; q++)
				if (*q == '\"')
				{
					*q = '\0';
					break;
				}
		}
		
		// Set the bit flag then (with LUMPHEADER level)
		a_CurrentInfo->SetBits[FNum] = PLIBL_LUMPHEADER;
		
		// Get field location
		vP = (void*)(((uintptr_t)a_CurrentInfo) + c_PMIFields[FNum].Offset);
		
		// Depending on the type, set the value
			// String
		if (c_PMIFields[FNum].Type == PPMFIFT_STRING)
		{
			// Get actual pointer
			StrValP = (char**)vP;
			
			// Delete string?
			if (*StrValP)
				Z_Free(*StrValP);
			
			// Set value
			*StrValP = Z_StrDup(p, PU_WLDKRMOD, NULL);
			
			// Debug
			if (devparm)
				CONL_PrintF("PS_ParseLumpHeader: \"%s\" set to \"%s\".\n", c_PMIFields[FNum].NewInfo, *StrValP);
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
				CONL_PrintF("PS_ParseLumpHeader: \"%s\" set to \'%i\'.\n", c_PMIFields[FNum].NewInfo, *IntValP);
		}
			
			// Bool
		else if (c_PMIFields[FNum].Type == PPMFIFT_BOOL)
		{
			// Get source and dest
			BoolValP = (bool_t*)vP;
		
			// Set to true!
			*BoolValP = true;
			
			// Debug
			if (devparm)
				CONL_PrintF("PS_ParseLumpHeader: \"%s\" flagged.\n", c_PMIFields[FNum].MapInfo);
		}
	}
	
	/* Proper Title? */
	if (a_CurrentInfo->Title)
		snprintf(a_CurrentInfo->ProperTitle, MAXPINFOPROPERNAME, "%s - %s", a_CurrentInfo->LumpName, a_CurrentInfo->Title);
	
	/* Success! */
	return true;
#undef BUFSIZE
}

/* PS_ParseMapInfo() -- Parses map info */
static bool_t PS_ParseMapInfo(P_LevelInfoHolder_t* const a_Holder, const WL_WADEntry_t* a_MIEntry)
{
#define BUFSIZE 512
	char Buf[BUFSIZE];
	char Token[BUFSIZE];
	P_LevelInfoEx_t* CurrentInfo;
	WL_ES_t* Stream;
	size_t i, FNum;
	char* p, *q;
	char* TokenP;
	
	char** StrValP;
	char** StrValS;
	
	int32_t* IntValP;
	int32_t* IntValS;
	
	fixed_t* FixedValP;
	fixed_t* FixedValS;
	
	bool_t* BoolValP;
	bool_t* BoolValS;
	
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
		WL_Srl(Stream, Buf, BUFSIZE - 1);
		
		// Skip whitespace
		for (p = Buf; *p && (*p == ' ' || *p == '\t'); p++)
			;
		
		// Read first word into token
		for (i = 0; *p && *p != ' ' && *p != '\t'; p++)
			if (i < BUFSIZE)
				Token[i++] = *p;
		
		// Comment?
		if (Token[0] == ';')
			continue;
		
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
			CurrentInfo = NULL;
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
			
			// Setup Defaults
			if (CurrentInfo)
			{
				// Setup level title?
				if (!CurrentInfo->Title)
				{
					// Set as MAPINFO level
					CurrentInfo->SetBits[0] = PLIBL_MAPINFO;
				
					// Ignore any whitespace
					for (; *p && (*p == ' ' || *p == '\t'); p++)
						;
				
					// Ignore any quotes
					if (*p == '\"')
					{
						// Null the quote away
						*(p++) = '\0';
				
						// Find the next quote, null that then die
						for (q = p; *q; q++)
							if (*q == '\"')
							{
								*q = '\0';
								break;
							}
					}
				
					// Place level name here
					CurrentInfo->Title = Z_StrDup(p, PU_WLDKRMOD, NULL);
					
					// Properize
					snprintf(CurrentInfo->ProperTitle, MAXPINFOPROPERNAME, "%s - %s", CurrentInfo->LumpName, CurrentInfo->Title);
				}
			}
			
			// Copy default settings?
				// Also never copy from default to default!
			if (CurrentInfo && CurrentInfo != &DefaultStore)
			{
				// For every field...
				for (i = 0; !c_PMIFields[i].IsEnd; i++)
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
							
							// Delete string?
							if (*StrValP)
								Z_Free(*StrValP);
							
							// Duplicate string
							if (*StrValS)
								*StrValP = Z_StrDup(*StrValS, PU_WLDKRMOD, NULL);
							break;
							
							// Copy Integer
						case PPMFIFT_INTEGER:
							// Get source and dest
							IntValP = (int32_t*)vP;
							IntValS = (int32_t*)vS;
							
							// Copy value
							*IntValP = *IntValS;
							break;
							
							// Copy Bool
						case PPMFIFT_BOOL:
							// Get source and dest
							BoolValP = (bool_t*)vP;
							BoolValS = (bool_t*)vS;
							
							// Copy value
							*BoolValP = *BoolValS;
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
			if (CurrentInfo->SetBits[FNum] > PLIBL_MAPINFO)
			{
				if (devparm)
					CONL_PrintF("PS_ParseMapInfo: Already defined at higher level (this: %i, at: %i).\n", PLIBL_MAPINFO, CurrentInfo->SetBits[FNum]);
				continue;	// Skip it then
			}
			
			// GhostlyDeath <May 5, 2012> -- Ignore any quotes
			if (*p == '\"')
			{
				// Null the quote away
				*(p++) = '\0';
				
				// Find the next quote, null that then die
				for (q = p; *q; q++)
					if (*q == '\"')
					{
						*q = '\0';
						break;
					}
			}
			
			// However find the first space afterwards and nuke that
			else
			{
				// Hunt it down
				for (q = p; *q; q++)
					if (*q == ' ')
					{
						*q = '\0';
						break;
					}
			}
			
			// Set the bit flag then (with MAPINFO level)
			CurrentInfo->SetBits[FNum] = PLIBL_MAPINFO;
			
			// Get field location
			vP = (void*)(((uintptr_t)CurrentInfo) + c_PMIFields[FNum].Offset);
			
			// Depending on the type, set the value
				// String
			if (c_PMIFields[FNum].Type == PPMFIFT_STRING)
			{
				// Get actual pointer
				StrValP = (char**)vP;
				
				// Delete string?
				if (*StrValP)
					Z_Free(*StrValP);
				
				// Set value
				*StrValP = Z_StrDup(p, PU_WLDKRMOD, NULL);
				
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
				
				// Bool
			else if (c_PMIFields[FNum].Type == PPMFIFT_BOOL)
			{
				// Get source and dest
				BoolValP = (bool_t*)vP;
			
				// Set to true!
				*BoolValP = true;
				
				// Debug
				if (devparm)
					CONL_PrintF("PS_ParseMapInfo: \"%s\" flagged.\n", c_PMIFields[FNum].MapInfo);
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
static bool_t PS_LevelInfoGetBlockPoints(P_LevelInfoEx_t* const a_Info, const WL_WADEntry_t* a_Entry, WL_ES_t* const a_Stream)
{
#define BUFSIZE 256
	char Buf[BUFSIZE];
	uint16_t Char;
	bool_t FullRead;
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
		FullRead = false;
		
		// Read line into buffer
		do
		{
			// No more characters?
			if (WL_StreamEOF(a_Stream))
				break;
			
			// Read it
			Char = WL_Src(a_Stream);
			
			// Place it
			if (i < BUFSIZE - 1)
				if (Char != '\n' && Char != '\r')
					Buf[i++] = Char;
				else
					FullRead = true;
		} while (Char != '\n');
		
		// Determine if this is a block specifier
		if (FullRead && Buf[0] == '[')
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
	WL_ES_t* ReadStream;
	uint16_t Char;
	
	/* Check */
	if (!a_WAD || !a_DataPtr || !a_SizePtr)
		return false;
	
	/* Create base structures */
	*a_SizePtr = sizeof(P_LevelInfoHolder_t);
	Holder = *a_DataPtr = Z_Malloc(*a_SizePtr, PU_WLDKRMOD, NULL);
	
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
			CurrentInfo = Holder->Infos[Holder->NumInfos++] = Z_Malloc(sizeof(*CurrentInfo), PU_WLDKRMOD, NULL);
			
			// Initialize new info with base stuff
			strncat(CurrentInfo->LumpName, Base->Name, MAXPLIEXFIELDWIDTH);
			
			// Basic Proper Title
			snprintf(CurrentInfo->ProperTitle, MAXPINFOPROPERNAME, "%s %s\n", a_WAD->__Private.__DOSName, CurrentInfo->LumpName);
			
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
		
		// Info
		CONL_EarlyBootTic(CurrentInfo->LumpName, true);
		
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
				// Check if PIBT_LEVELINFO exists
				if (CurrentInfo->BlockPos[PIBT_LEVELINFO][1] - CurrentInfo->BlockPos[PIBT_LEVELINFO][0] > 0)
					PS_ParseLumpHeader(CurrentInfo, ReadStream);
			}
		}
		
		// Destroy stream
		WL_StreamClose(ReadStream);
		
		// If no level name exists, fake one (as long as this isn't an IWAD)
		if (!CurrentInfo->WAD->__Private.__IsIWAD)
		{
			// Title
			if (!CurrentInfo->Title)
			{
				CurrentInfo->Title = Z_StrDup(CurrentInfo->LumpName, PU_WLDKRMOD, NULL);
				CurrentInfo->SetBits[0] = PLIBL_GENERIC;
			}
			
			// Author
			if (!CurrentInfo->Author)
			{
				CurrentInfo->Author = Z_StrDup("Unknown", PU_WLDKRMOD, NULL);
				CurrentInfo->SetBits[1] = PLIBL_GENERIC;
			}
		}
		
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
	P_LevelInfoEx_t* Composite;
	P_LevelInfoEx_t* CopyFrom;
	P_LevelInfoEx_t** CompSpot;
	size_t Sz, i, Field, j, k;
	bool_t BootFields;
	
	char** StrValP;
	char** StrValS;
	
	int32_t* IntValP;
	int32_t* IntValS;
	
	fixed_t* FixedValP;
	fixed_t* FixedValS;
	
	void* vP;
	void* vS;
	
	/* Debug */
	if (devparm)
		CONL_PrintF("PS_WLInfoOCCB: Building composite...\n");
	
	/* Clear level info composite */
	if (l_CompInfos)
	{
		// Clear subs
		for (i = 0; i < l_NumCompInfos; i++)
			if (l_CompInfos[i])
				if (l_CompInfos[i]->IsComposite)
				{
					// Clear out strings
					// Copy fields over (all but authors)
					for (Field = 0; !c_PMIFields[Field].IsEnd; Field++)
					{
						// Get source
						vP = (void*)(((uintptr_t)l_CompInfos[i]) + c_PMIFields[Field].Offset);
					
						// Which type now?
						switch (c_PMIFields[Field].Type)
						{
								// Copy String
							case PPMFIFT_STRING:
								if (*((char**)vP))
									Z_Free(*((char**)vP));
								break;
								
								// Nothing to be freed
							default:
								break;
						}
					}
					
					// Clear self away
					Z_Free(l_CompInfos[i]);
				}
		
		// Clear master
		Z_Free(l_CompInfos);
	}
	l_CompInfos = NULL;
	l_NumCompInfos = 0;
	
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
				// Clear for future field booting
				BootFields = false;
				
				// If the existing info is a compsite then use that
				if (CompInfo->IsComposite)
					// Set as this
					Composite = CompInfo;
				
				// Otherwise allocate a new composite
				else
				{
					// Allocate
					Composite = Z_Malloc(sizeof(*Composite), PU_WLDKRMOD, NULL);
					
					// Set as composite and boot the fields
					Composite->IsComposite = true;
					BootFields = true;
					
					// Use the latest map (entry related) stuff
					if (CurrentInfo->WAD)
					{
						// Copy WAD
						Composite->WAD = CurrentInfo->WAD;
						
						// Copy lumps over
						for (Field = 0; Field < MAXPLIEDS; Field++)
							Composite->EntryPtr[Field] = CurrentInfo->EntryPtr[Field];
						
						// Copy type
						Composite->Type = CurrentInfo->Type;
						
						// Copy script positions
						for (Field = 0; Field < NUMPINFOBLOCKTYPES; Field++)
							for (j = 0; j < 2; j++)
								Composite->BlockPos[Field][j] = CurrentInfo->BlockPos[Field][j];
						
						// Copy name of lump
						memmove(Composite->LumpName, CurrentInfo->LumpName, sizeof(char) * MAXPLIEXFIELDWIDTH);
					}
				}
				
				// Copy fields from CurrentInfo to Composite
				// For every field...
				for (k = 0; k < 2; k++)
				{
					// Fields don't need booting?
					if (!k && !BootFields)
						continue;
					
					// Which fields to copy from?
					if (!k)
						CopyFrom = CompInfo;
					else
						CopyFrom = CurrentInfo;
					
					// Copy fields over (all but authors)
					for (Field = NUMPINFOIGNOREFIELDS; !c_PMIFields[Field].IsEnd; Field++)
					{
						// Not set here?
						if (!CopyFrom->SetBits[Field])
							continue;
					
						// Check to see if already set or the default is unset
							// Compare level, if it is the same or better, replace
							// Less than because we always want CopyFrom to always
							// be better!
						if (CopyFrom->SetBits[Field] < Composite->SetBits[Field])
							continue;	// Skip it then
					
						// Set the bit flag then (with most level)
						Composite->SetBits[Field] = CopyFrom->SetBits[Field];
					
						// Get source and dest
						vP = (void*)(((uintptr_t)Composite) + c_PMIFields[Field].Offset);
						vS = (void*)(((uintptr_t)CopyFrom) + c_PMIFields[Field].Offset);
					
						// Which type now?
						switch (c_PMIFields[Field].Type)
						{
								// Copy String
							case PPMFIFT_STRING:
								// Get source and dest
								StrValP = (char**)vP;
								StrValS = (char**)vS;
							
								// Delete string?
								if (*StrValP)
									Z_Free(*StrValP);
							
								// Duplicate string
								*StrValP = Z_StrDup(*StrValS, PU_WLDKRMOD, NULL);
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
				
				// Set current spot with composite
				*CompSpot = Composite;
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
static int PCLC_Maps(const uint32_t a_ArgC, const char** const a_ArgV)
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
static int PCLC_MapInfo(const uint32_t a_ArgC, const char** const a_ArgV)
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

/* P_LevelNameEx() -- Returns name of level */
const char* P_LevelNameEx(void)
{
	/* No level loaded? */
	if (!g_CurrentLevelInfo)
		return "None";
	
	/* Use title? */
	if (g_CurrentLevelInfo->Title)
		return g_CurrentLevelInfo->Title;
	
	/* Otherwise use the lump name */
	return g_CurrentLevelInfo->LumpName;
}

