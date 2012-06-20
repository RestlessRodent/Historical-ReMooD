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
// DESCRIPTION: Various Demo Support

/***************
*** INCLUDES ***
***************/

#include "g_game.h"
#include "p_info.h"
#include "p_demcmp.h"
#include "w_wad.h"
#include "m_random.h"
#include "m_misc.h"
#include "m_argv.h"
#include "p_setup.h"
#include "r_main.h"

/**********************
*** VANILLA FACTORY ***
**********************/
// Handles 1.0 through 1.9 demos

/* G_VanillaDemoData_t -- Vanilla Data */
typedef struct G_VanillaDemoData_s
{
	uint8_t VerMarker;
	uint8_t Skill, Episode, Map, Players[4];
	uint8_t Deathmatch, Respawn, Fast, NoMonsters, POV;
	bool_t MultiPlayer;
	bool_t LongTics;
	D_RBlockStream_t* PMPStream;				// Debug Stream
	bool_t WroteHeader;							// Wrote header
	bool_t EndDemo;								// EndDemo
} G_VanillaDemoData_t;

/* G_DEMO_Vanilla_StartPlaying() -- Starts playing Vanilla Demo */
bool_t G_DEMO_Vanilla_StartPlaying(struct G_CurrentDemo_s* a_Current)
{
	char LevelName[9];
	int32_t i, j, p;
	uint8_t VerMarker;
	uint8_t Skill, Episode, Map, Players[4];
	uint8_t Deathmatch, Respawn, Fast, NoMonsters, POV;
	G_VanillaDemoData_t* Data;
	const char* PlName;
	const P_LevelInfoEx_t* LevelInfo;
	
	/* Check */
	if (!a_Current)
		return false;
	
	/* Read Version Marker */
	VerMarker = WL_StreamReadUInt8(a_Current->WLStream);
	
	/* Which Demo Format? */
	// Doom 1.2 and Heretic
	if (VerMarker <= 4)
	{
		Skill = VerMarker;
		Episode = WL_StreamReadUInt8(a_Current->WLStream);
		Map = WL_StreamReadUInt8(a_Current->WLStream);
		
		// Read Players
		for (i = 0; i < 4; i++)
			Players[i] = WL_StreamReadUInt8(a_Current->WLStream);
		
		// Hack Version ID
		if (g_CoreGame == COREGAME_DOOM)
			VerMarker = 102;
		else
			VerMarker = 103;
	}
	
	// Doom 1.4 and up
	else
	{
		Skill = WL_StreamReadUInt8(a_Current->WLStream);
		Episode = WL_StreamReadUInt8(a_Current->WLStream);
		Map = WL_StreamReadUInt8(a_Current->WLStream);
		Deathmatch = WL_StreamReadUInt8(a_Current->WLStream);
		Respawn = WL_StreamReadUInt8(a_Current->WLStream);
		Fast = WL_StreamReadUInt8(a_Current->WLStream);
		NoMonsters = WL_StreamReadUInt8(a_Current->WLStream);
		POV = WL_StreamReadUInt8(a_Current->WLStream);
		
		// Read Players
		for (i = 0; i < 4; i++)
			Players[i] = WL_StreamReadUInt8(a_Current->WLStream);
	}
	
	/* Determine if the map is legal */
	// MAPxx
	if (g_IWADFlags & CIF_COMMERCIAL)
		snprintf(LevelName, 8, "MAP%02d", Map);
	
	// ExMx
	else
		snprintf(LevelName, 8, "E%1.1dM%1.1d", Episode, Map);
	
	// Attempt locate
	LevelInfo = P_FindLevelByNameEx(LevelName, 0);
	
	// Not found?
	if (!LevelInfo)
		return false;
	
	/* Fill Data */
	Data = a_Current->Data = Z_Malloc(sizeof(*Data), PU_STATIC, NULL);
	
	/* Clone */
	Data->VerMarker = VerMarker;
	Data->Skill = Skill;
	Data->Episode = Episode;
	Data->Map = Map;
	Data->Deathmatch = Deathmatch;
	Data->Respawn = Respawn;
	Data->Fast = Fast;
	Data->NoMonsters = NoMonsters;
	Data->POV = POV;
	
	for (i = 0; i < 4; i++)
	{
		Data->Players[i] = Players[i];
		
		// Multi-Player?
		if (Data->Players[i] && i > 0)
			Data->MultiPlayer = true;
	}
	
	/* Long Tics? */
	if (Data->VerMarker == 111)
	{
		// Set as 1.09 Demo with longtics
		Data->LongTics = true;
		VerMarker = Data->VerMarker = 109;
	}
	
	/* Setup Game Rules */
	P_EXGSSetAllDefaults();
	P_EXGSSetVersionLevel(true, VerMarker);
	
	// Options
	P_EXGSSetValue(true, PEXGSBID_GAMESKILL, Data->Skill);
	P_EXGSSetValue(true, PEXGSBID_GAMEDEATHMATCH, Data->Deathmatch);
	P_EXGSSetValue(true, PEXGSBID_MONFASTMONSTERS, Data->Fast);
	P_EXGSSetValue(true, PEXGSBID_MONRESPAWNMONSTERS, Data->Respawn);
	P_EXGSSetValue(true, PEXGSBID_MONSPAWNMONSTERS, !Data->NoMonsters);
	
	// Based on Game Mode
		// Solo/Coop
	if (Data->Deathmatch == 0)
	{
		// Coop
		if (Data->MultiPlayer)
		{
			P_EXGSSetValue(true, PEXGSBID_GAMESPAWNMULTIPLAYER, 1);
			P_EXGSSetValue(true, PEXGSBID_ITEMSKEEPWEAPONS, 1);
		}
		
		// Solo
		else
		{
			P_EXGSSetValue(true, PEXGSBID_GAMESPAWNMULTIPLAYER, 0);
			P_EXGSSetValue(true, PEXGSBID_ITEMSKEEPWEAPONS, 0);
		}
	}
		// DM
	else
	{
		// Shared Flags
		P_EXGSSetValue(true, PEXGSBID_GAMESPAWNMULTIPLAYER, 1);
		
		// DM
		if (Data->Deathmatch == 1)
		{
			P_EXGSSetValue(true, PEXGSBID_ITEMSKEEPWEAPONS, 1);
			P_EXGSSetValue(true, PEXGSBID_ITEMRESPAWNITEMS, 0);
		}
		
		// AltDM
		else
		{
			P_EXGSSetValue(true, PEXGSBID_ITEMSKEEPWEAPONS, 0);
			P_EXGSSetValue(true, PEXGSBID_ITEMRESPAWNITEMS, 1);
		}
	}
	
	if (Data->MultiPlayer)	// multiplayer/netgame
		P_EXGSSetValue(true, PEXGSBID_COMULTIPLAYER, 1);
	else
		P_EXGSSetValue(true, PEXGSBID_COMULTIPLAYER, 0);
	
	/* Reset Indexes */
	D_SyncNetSetMapTime(0);
	P_SetRandIndex(0);
	
	/* Setup Players */
	memset(playeringame, 0, sizeof(playeringame));
	memset(g_PlayerInSplit, 0, sizeof(g_PlayerInSplit));
	g_SplitScreen = -1;
	
	// Set them all up (split-screen)
	for (j = 0, i = 0; i < 4; i++)
		if (Data->Players[i])
		{
			// Set as in game
			playeringame[i] = true;
			
			// Initialize Player
			G_AddPlayer(i);
			G_InitPlayer(&players[i]);
			
			// Set Color
			players[i].skincolor = i;
			
			// Name it by player ID
			PlName = NULL;
			switch (i)
			{
				case 0: PlName = "{x70Green"; break;
				case 1: PlName = "{x71Indigo"; break;
				case 2: PlName = "{x72Brown"; break;
				case 3: PlName = "{x73Red"; break;
				default: break;
			}
			
			if (PlName)
				strncpy(player_names[i], PlName, MAXPLAYERNAME - 1);
			
			// Put in split screen
			if (j < 4)
			{
				g_SplitScreen++;
				g_PlayerInSplit[j] = true;
				consoleplayer[j] = displayplayer[j] = i;
				j++;
			}
		}
	
	// Recalc Split-screen
	R_ExecuteSetViewSize();
	
	/* Load The Map */
	// Load it, hopefully
	P_ExLoadLevel(LevelInfo, 0);
	
	/* PMP Debug */
	// for each player per tic: PMPL 16 ??? <x> <y> <z>
	if ((p = M_CheckParm("-vanillapmp")) && M_IsNextParm())
		Data->PMPStream = D_RBSCreateFileStream(M_GetNextParm(), DRBSSF_READONLY);
	
	/* Success! */
	return true;
}

/* G_DEMO_Vanilla_StopPlaying() -- Stop playing vanilla demo */
bool_t G_DEMO_Vanilla_StopPlaying(struct G_CurrentDemo_s* a_Current)
{
	G_VanillaDemoData_t* Data;
	
	/* Check */
	if (!a_Current)
		return false;
		
	/* Get */
	Data = a_Current->Data;
	
	// No data?
	if (!Data)
		return false;
	
	/* Delete used data */
	Z_Free(Data);
	
	/* Success! */
	return true;
}

/* G_DEMO_Vanilla_StartRecord() -- Starts recording vanilla demo */
bool_t G_DEMO_Vanilla_StartRecord(struct G_CurrentDemo_s* a_Current)
{
	uint8_t VerMarker;
	G_VanillaDemoData_t* Data;
	
	/* Check */
	if (!a_Current)
		return false;
		
	/* Fill Data */
	Data = a_Current->Data = Z_Malloc(sizeof(*Data), PU_STATIC, NULL);
	
	/* Write version marker */
	Data->VerMarker = VerMarker = 111;
	Data->LongTics = true;
	fwrite(&VerMarker, 1, 1, a_Current->CFile);
	fflush(a_Current->CFile);
	
	/* Success! */
	return true;
}

/* G_DEMO_Vanilla_StopRecord() -- Stops recording vanilla demo */
bool_t G_DEMO_Vanilla_StopRecord(struct G_CurrentDemo_s* a_Current)
{
	uint8_t Marker;
	G_VanillaDemoData_t* Data;
	
	/* Check */
	if (!a_Current)
		return false;
		
	/* Get */
	Data = a_Current->Data;
	
	// No data?
	if (!Data)
		return false;
	
	/* Write marker byte */
	Marker = 0x80;
	fwrite(&Marker, 1, 1, a_Current->CFile);
	fflush(a_Current->CFile);
	
	/* Delete used data */
	Z_Free(Data);
	
	/* Success! */
	return true;
}

/* G_DEMO_Vanilla_CheckDemo() -- See if vanilla demo is over */
bool_t G_DEMO_Vanilla_CheckDemo(struct G_CurrentDemo_s* a_Current)
{
	G_VanillaDemoData_t* Data;
	
	/* Check */
	if (!a_Current)
		return true;
		
	/* Get */
	Data = a_Current->Data;
	
	// No data?
	if (!Data)
		return true;
	
	/* Force Demo End */
	if (Data->EndDemo)
		return true;
	
	/* Playing Demo */
	if (!a_Current->Out)
	{
		// Stream ended?
		if (WL_StreamEOF(a_Current->WLStream))
			return true;
	}
	
	/* Recording Demo */
	else
	{
	}
	
	/* Still playing/recording */
	return false;
}

/* G_DEMO_Vanilla_ReadTicCmd() -- Reads tic command from demo */
bool_t G_DEMO_Vanilla_ReadTicCmd(struct G_CurrentDemo_s* a_Current, ticcmd_t* const a_Cmd, const int32_t a_PlayerNum)
{
	G_VanillaDemoData_t* Data;
	uint8_t ButtonCodes;
	
	uint32_t u32;
	int32_t i32;
	char Header[5];
	
	/* Check */
	if (!a_Current || !a_Cmd || a_PlayerNum < 0 || a_PlayerNum >= 4)
		return false;
	
	/* Get */
	Data = a_Current->Data;
	
	/* PMP */
	if (Data->PMPStream)
		if (D_RBSPlayBlock(Data->PMPStream, Header))
		{
			// Read/Check gametic
			u32 = D_RBSReadUInt32(Data->PMPStream);
			if (u32 > 0 && D_SyncNetMapTime() > 0)
				if (u32 != D_SyncNetMapTime())
					I_Error("PMP: gametic/MapTime Mismatch");
			
			if (u32 > 0 && D_SyncNetMapTime() > 0)
			{
				// Read/Check X Position
				i32 = D_RBSReadInt32(Data->PMPStream);
				if (abs(i32 - players[a_PlayerNum].mo->x) >= (8 << FRACBITS))
					I_Error("PMP: X Mismatch");
				
				// Read/Check Y Position
				i32 = D_RBSReadInt32(Data->PMPStream);
				if (abs(i32 - players[a_PlayerNum].mo->y) >= (8 << FRACBITS))
					I_Error("PMP: Y Mismatch");
				
				// Read/Check Z Position
				i32 = D_RBSReadInt32(Data->PMPStream);
				if (abs(i32 - players[a_PlayerNum].mo->z) >= (8 << FRACBITS))
					I_Error("PMP: Z Mismatch");
			}
		}
		
		// Ended?
		else
		{
			D_RBSCloseStream(Data->PMPStream);
			Data->PMPStream = NULL;
		}
	
	/* Clear Command */
	memset(a_Cmd, 0, sizeof(*a_Cmd));
	
	/* Read player's command */
	a_Cmd->forwardmove = WL_StreamReadInt8(a_Current->WLStream);
	a_Cmd->sidemove = WL_StreamReadInt8(a_Current->WLStream);
	
	// 1.91?
	if (Data->LongTics)
	{
		a_Cmd->angleturn = WL_StreamReadInt8(a_Current->WLStream);
		a_Cmd->angleturn |= ((int16_t)WL_StreamReadInt8(a_Current->WLStream)) << 8;
	}
	else
		a_Cmd->angleturn = ((int16_t)WL_StreamReadInt8(a_Current->WLStream)) << 8;
	
	/* Button codes require re-handling */
	// They are different in Vanilla Demos
	ButtonCodes = WL_StreamReadUInt8(a_Current->WLStream);
	
	// Fire Weapon?
	if (ButtonCodes & 1)
		a_Cmd->buttons |= BT_ATTACK;
	
	// Use?
	if (ButtonCodes & 2)
		a_Cmd->buttons |= BT_USE;
	
	// Change gun?
	if (ButtonCodes & 4)
		a_Cmd->buttons |= BT_CHANGE | BT_EXTRAWEAPON;	// Slot based change
	
	// Resort weapon over
	a_Cmd->buttons |= ((((ButtonCodes & 0x38) >> 3)) << BT_SLOTSHIFT) & BT_SLOTMASK;

	/* End of demo? */
	if (a_Cmd->forwardmove == 0x80)
		Data->EndDemo = true;
	
	/* Success! */
	return true;
}

bool_t G_DEMO_Vanilla_WriteTicCmd(struct G_CurrentDemo_s* a_Current, const ticcmd_t* const a_Cmd, const int32_t a_PlayerNum)
{
	uint8_t Bits;
	int8_t IntV, i;
	G_VanillaDemoData_t* Data;
	
	/* Check */
	if (!a_Current || !a_Cmd || a_PlayerNum < 0 || a_PlayerNum >= 4)
		return false;
	
	/* Get */
	Data = a_Current->Data;
	
	// Missing?
	if (!Data)
		return false;
	
	/* Remaining header not yet written? */
	if (!Data->WroteHeader)
		if (g_CurrentLevelInfo)
		{
			Bits = P_EXGSGetValue(PEXGSBID_GAMESKILL);
			fwrite(&Bits, 1, 1, a_Current->CFile);
			
			Bits = g_CurrentLevelInfo->EpisodeNum;
			fwrite(&Bits, 1, 1, a_Current->CFile);
			
			Bits = g_CurrentLevelInfo->LevelNum;
			fwrite(&Bits, 1, 1, a_Current->CFile);
			
			Bits = P_EXGSGetValue(PEXGSBID_GAMEDEATHMATCH);
			fwrite(&Bits, 1, 1, a_Current->CFile);
			
			Bits = P_EXGSGetValue(PEXGSBID_MONRESPAWNMONSTERS);
			fwrite(&Bits, 1, 1, a_Current->CFile);
			
			Bits = P_EXGSGetValue(PEXGSBID_MONFASTMONSTERS);
			fwrite(&Bits, 1, 1, a_Current->CFile);
			
			Bits = !P_EXGSGetValue(PEXGSBID_MONSPAWNMONSTERS);
			fwrite(&Bits, 1, 1, a_Current->CFile);
			
			Bits = 0;
			fwrite(&Bits, 1, 1, a_Current->CFile);
			
			// Read Players
			for (i = 0; i < 4; i++)
			{
				Bits = playeringame[i];
				fwrite(&Bits, 1, 1, a_Current->CFile);
			}
			
			// Wrote header
			Data->WroteHeader = true;
		}
	
	/* Write tic commands as long as header was written */
	if (Data->WroteHeader)
	{
		// Movement
		IntV = a_Cmd->forwardmove;
		fwrite(&IntV, 1, 1, a_Current->CFile);
		
		IntV = a_Cmd->sidemove;
		fwrite(&IntV, 1, 1, a_Current->CFile);
		
		// Turning
			// Long tics
		if (Data->LongTics)
		{
			IntV = (a_Cmd->angleturn & 0x00FF);
			fwrite(&IntV, 1, 1, a_Current->CFile);
			
			IntV = (a_Cmd->angleturn & 0xFF00) >> 8;
			fwrite(&IntV, 1, 1, a_Current->CFile);
		}
			// Normal
		else
		{
			IntV = (a_Cmd->angleturn & 0xFF00) >> 8;
			fwrite(&IntV, 1, 1, a_Current->CFile);
		}
		
		// Buttons
		Bits = 0;
		
		// Fire Weapon?
		if (a_Cmd->buttons &  BT_ATTACK)
			Bits |= 1;
	
		// Use?
		if (a_Cmd->buttons & BT_USE)
			Bits |= 2;
	
		// Change gun?
		if ((a_Cmd->buttons & (BT_CHANGE | BT_EXTRAWEAPON)) == (BT_CHANGE | BT_EXTRAWEAPON))
			Bits |= 4;
		
		// Resort weapon over
		Bits |= (((a_Cmd->buttons & BT_SLOTMASK) >> BT_SLOTSHIFT) << 3) & 0x38;
		
		// Write
		fwrite(&Bits, 1, 1, a_Current->CFile);
	}
	
	/* Flush */
	fflush(a_Current->CFile);
	
	/* Success! */
	return true;
}

/*********************
*** LEGACY FACTORY ***
*********************/
// Handles 1.11 through 1.43 demos

/*** STRUCTURES ***/

/* G_LegacyDemoData_t -- Legacy Data */
typedef struct G_LegacyDemoData_s
{
	uint8_t VerMarker;							// Demo version
	bool_t EndDemo;								// End of demo
	
	uint8_t Skill, Episode, Map, DM, MultiPlayer;
	uint8_t Respawn, Fast, NoMonsters, DisplayP, TimeLimit;
	bool_t Players[MAXPLAYERS];
} G_LegacyDemoData_t;

/*** FUNCTIONS ***/

/* G_DEMO_Legacy_StartPlaying() -- Start playing Demo */
bool_t G_DEMO_Legacy_StartPlaying(struct G_CurrentDemo_s* a_Current)
{
	int i, j, k;
	char LevelName[9];
	G_LegacyDemoData_t* Data;
	uint8_t VerMarker, Skill, Episode, Map, DM, MultiPlayer;
	uint8_t Respawn, Fast, NoMonsters, DisplayP, TimeLimit;
	bool_t Players[MAXPLAYERS];
	const P_LevelInfoEx_t* LevelInfo;
	const char* PlName;
	
	/* Check */
	if (!a_Current)
		return false;
	
	/* Clear */
	memset(Players, 0, sizeof(Players));
	
	/* Read Demo Info */
	VerMarker = WL_StreamReadUInt8(a_Current->WLStream);
	Skill = WL_StreamReadUInt8(a_Current->WLStream);
	Episode = WL_StreamReadUInt8(a_Current->WLStream);
	Map = WL_StreamReadUInt8(a_Current->WLStream);
	
	/* Locate Map (Only before 1.27) */
	if (VerMarker < 127)
	{
		// MAPxx
		if (g_IWADFlags & CIF_COMMERCIAL)
			snprintf(LevelName, 8, "MAP%02d", Map);
	
		// ExMx
		else
			snprintf(LevelName, 8, "E%1.1dM%1.1d", Episode, Map);
	
		// Attempt locate
		LevelInfo = P_FindLevelByNameEx(LevelName, 0);
	
		// Not found?
		if (!LevelInfo)
			return false;
	}
	else
		LevelInfo = NULL;
	
	// Only Before 1.27
	DM = WL_StreamReadUInt8(a_Current->WLStream);
	
	// Only Before 1.28
	Respawn = WL_StreamReadUInt8(a_Current->WLStream);
	Fast = WL_StreamReadUInt8(a_Current->WLStream);
	
	NoMonsters = WL_StreamReadUInt8(a_Current->WLStream);
	DisplayP = WL_StreamReadUInt8(a_Current->WLStream);
	
	// 1.09
	if (VerMarker <= 109)
		for (i = 0; i < 4; i++)
			Players[i] = WL_StreamReadUInt8(a_Current->WLStream);
	
	// 1.11+
	else
	{
		TimeLimit = WL_StreamReadUInt8(a_Current->WLStream);
		
		if (VerMarker < 113)
		{
			for (i = 0; i < 8; i++)
				Players[i] = WL_StreamReadUInt8(a_Current->WLStream);
			
			MultiPlayer = WL_StreamReadUInt8(a_Current->WLStream);
		}
		else
		{
			MultiPlayer = WL_StreamReadUInt8(a_Current->WLStream);
			
			for (i = 0; i < 32; i++)
				Players[i] = WL_StreamReadUInt8(a_Current->WLStream);
		}	
	}
	
	/* Setup Players */
	memset(playeringame, 0, sizeof(playeringame));
	memset(g_PlayerInSplit, 0, sizeof(g_PlayerInSplit));
	g_SplitScreen = -1;
	
	// Set them all up (split-screen)
	for (j = 0, i = 0; i < 4; i++)
		if (Players[i])
		{
			// Set as in game
			playeringame[i] = true;
			
			// Initialize Player
			G_AddPlayer(i);
			G_InitPlayer(&players[i]);
			
			// Set Color
			k = i % 11;
			players[i].skincolor = k;
			
			// Name it by player ID
			PlName = NULL;
			switch (k)
			{
				case 0: PlName = "{x70Green"; break;
				case 1: PlName = "{x71Indigo"; break;
				case 2: PlName = "{x72Brown"; break;
				case 3: PlName = "{x73Red"; break;
				case 4: PlName = "{x74Light Gray"; break;
				case 5: PlName = "{x75Light Brown"; break;
				case 6: PlName = "{x76Light Red"; break;
				case 7: PlName = "{x77Light Blue"; break;
				case 8: PlName = "{x78Blue"; break;
				case 9: PlName = "{x79Yellow"; break;
				case 10: PlName = "{x7aBeige"; break;
				default: break;
			}
			
			if (PlName)
				strncpy(player_names[i], PlName, MAXPLAYERNAME - 1);
			
			// Put in split screen
			if (j < 4)
			{
				g_SplitScreen++;
				g_PlayerInSplit[j] = true;
				consoleplayer[j] = displayplayer[j] = i;
				j++;
			}
		}
	
	/* Modify Settings required for level loading (as needed) */
	// Set version to the specified value
	P_EXGSSetVersionLevel(true, VerMarker);
	
	// DM Before 1.27
	if (VerMarker < 127)
		P_EXGSSetValue(true, PEXGSBID_GAMEDEATHMATCH, DM);
	
	// Respawn and Fast before 1.28
	if (VerMarker < 128)
	{
		P_EXGSSetValue(true, PEXGSBID_MONRESPAWNMONSTERS, Respawn);
		P_EXGSSetValue(true, PEXGSBID_MONFASTMONSTERS, Fast);
		P_EXGSSetValue(true, PEXGSBID_GAMETIMELIMIT, TimeLimit);
	}
	
	// Multiplayer
	P_EXGSSetValue(true, PEXGSBID_COMULTIPLAYER, MultiPlayer);
	
	/* Load the level as per vanilla before 1.27 */
	if (VerMarker < 127)
	{
		// Recalc Split-screen
		R_ExecuteSetViewSize();
		
		// Load the map, hopefully
		P_ExLoadLevel(LevelInfo, 0);
	}
	
	/* Otherwise start waiting for players */
	else
		gamestate = wipegamestate = GS_WAITINGPLAYERS;
	
	/* Create Demo Info */
	a_Current->Data = Data = Z_Malloc(sizeof(*Data), PU_STATIC, NULL);
	
	Data->VerMarker = VerMarker;
	Data->Skill = Skill;
	Data->Episode = Episode;
	Data->Map = Map;
	Data->DM = DM;
	Data->MultiPlayer = MultiPlayer;
	Data->Respawn = Respawn;
	Data->Fast = Fast;
	Data->NoMonsters = NoMonsters;
	Data->DisplayP = DisplayP;
	Data->TimeLimit = TimeLimit;
	
	for (i = 0; i < MAXPLAYERS; i++)
		Data->Players[i] = Players[i];
	
	/* Success! */
	return true;
}

/* G_DEMO_Legacy_StopPlaying() -- Stop playing demo */
bool_t G_DEMO_Legacy_StopPlaying(struct G_CurrentDemo_s* a_Current)
{
	return false;
}

/* G_DEMO_Legacy_StartRecord() -- Start recording demo */
bool_t G_DEMO_Legacy_StartRecord(struct G_CurrentDemo_s* a_Current)
{
	return false;
}

/* G_DEMO_Legacy_StopRecord() -- Stop recording demo */
bool_t G_DEMO_Legacy_StopRecord(struct G_CurrentDemo_s* a_Current)
{
	return false;
}

/* G_DEMO_Legacy_CheckDemo() -- Check Status */
bool_t G_DEMO_Legacy_CheckDemo(struct G_CurrentDemo_s* a_Current)
{
	return false;
}

/* G_DEMO_Legacy_ReadTicCmd() -- Read Tic Command */
bool_t G_DEMO_Legacy_ReadTicCmd(struct G_CurrentDemo_s* a_Current, ticcmd_t* const a_Cmd, const int32_t a_PlayerNum)
{
	G_LegacyDemoData_t* Data;
	
	/* Check */
	if (!a_Current)
		return false;
	
	/* Obtain */
	Data = a_Current->Data;
	
	// Check
	if (!Data)
		return false;
	
	/* Old Demo Format */
	if (Data->VerMarker < 112)
	{
	}
	
	/* New Compact Demo Format */
	else
	{
	}
	
	/* Success */
	return true;
}

/* G_DEMO_Legacy_WriteTicCmd() -- Write Tic Commnd */
bool_t G_DEMO_Legacy_WriteTicCmd(struct G_CurrentDemo_s* a_Current, const ticcmd_t* const a_Cmd, const int32_t a_PlayerNum)
{
	return false;
}

/*********************
*** REMOOD FACTORY ***
*********************/
// Handles ReMooD 1.0a+ Demos

/*** STRUCTURES ***/

/*** FUNCTIONS ***/

/* G_DEMO_ReMooD_StartPlaying() -- Start playing Demo */
bool_t G_DEMO_ReMooD_StartPlaying(struct G_CurrentDemo_s* a_Current)
{
	return false;
}

/* G_DEMO_ReMooD_StopPlaying() -- Stop playing demo */
bool_t G_DEMO_ReMooD_StopPlaying(struct G_CurrentDemo_s* a_Current)
{
	return false;
}

/* G_DEMO_ReMooD_StartRecord() -- Start recording demo */
bool_t G_DEMO_ReMooD_StartRecord(struct G_CurrentDemo_s* a_Current)
{
	return false;
}

/* G_DEMO_ReMooD_StopRecord() -- Stop recording demo */
bool_t G_DEMO_ReMooD_StopRecord(struct G_CurrentDemo_s* a_Current)
{
	return false;
}

/* G_DEMO_ReMooD_CheckDemo() -- Check Status */
bool_t G_DEMO_ReMooD_CheckDemo(struct G_CurrentDemo_s* a_Current)
{
	return false;
}

/* G_DEMO_ReMooD_ReadTicCmd() -- Read Tic Command */
bool_t G_DEMO_ReMooD_ReadTicCmd(struct G_CurrentDemo_s* a_Current, ticcmd_t* const a_Cmd, const int32_t a_PlayerNum)
{
	return false;
}

/* G_DEMO_ReMooD_WriteTicCmd() -- Write Tic Commnd */
bool_t G_DEMO_ReMooD_WriteTicCmd(struct G_CurrentDemo_s* a_Current, const ticcmd_t* const a_Cmd, const int32_t a_PlayerNum)
{
	return false;
}

/*******************
*** DEMO FACTORY ***
*******************/

/*** STRUCTURES ***/

/* G_DemoLink_t -- Demo chain link */
typedef struct G_DemoLink_s
{
	char* Name;									// Demo Name
	
	struct G_DemoLink_s* Next;					// Next in queue
} G_DemoLink_t;

/*** GLOBALS ***/

tic_t g_DemoTime = 0;							// Current demo read time

/*** LOCALS ***/

static G_CurrentDemo_t* l_PlayDemo = NULL;		// Demo being played
static G_CurrentDemo_t* l_RecDemo = NULL;		// Demo being recorded
static G_DemoLink_t* l_DemoQ = NULL;			// Demo Queue

/*** FACTORIES ***/

static const G_DemoFactory_t c_DemoFactories[] =
{
	// Vanilla Factory
	{
		"vanilla",
		false,
		G_DEMO_Vanilla_StartPlaying,
		G_DEMO_Vanilla_StopPlaying,
		G_DEMO_Vanilla_StartRecord,
		G_DEMO_Vanilla_StopRecord,
		G_DEMO_Vanilla_CheckDemo,
		G_DEMO_Vanilla_ReadTicCmd,
		G_DEMO_Vanilla_WriteTicCmd
	},
	
	// Legacy Factory
	{
		"legacy",
		false,
		G_DEMO_Legacy_StartPlaying,
		G_DEMO_Legacy_StopPlaying,
		G_DEMO_Legacy_StartRecord,
		G_DEMO_Legacy_StopRecord,
		G_DEMO_Legacy_CheckDemo,
		G_DEMO_Legacy_ReadTicCmd,
		G_DEMO_Legacy_WriteTicCmd
	},
	
	// ReMooD Factory
	{
		"remood",
		true,
		G_DEMO_ReMooD_StartPlaying,
		G_DEMO_ReMooD_StopPlaying,
		G_DEMO_ReMooD_StartRecord,
		G_DEMO_ReMooD_StopRecord,
		G_DEMO_ReMooD_CheckDemo,
		G_DEMO_ReMooD_ReadTicCmd,
		G_DEMO_ReMooD_WriteTicCmd
	},
	
	// End
	{NULL},
};

/*** FUNCTIONS ***/

/* G_DemoFactoryByName() -- Get factory by name */
const G_DemoFactory_t* G_DemoFactoryByName(const char* const a_Name)
{
	size_t i;
	
	/* Check */
	if (!a_Name)
		return NULL;
	
	/* Find */
	for (i = 0; c_DemoFactories[i].FactoryName; i++)
		if (strcasecmp(a_Name, c_DemoFactories[i].FactoryName) == 0)
			return &c_DemoFactories[i];
	
	/* Not found */
	return NULL;
}

/* G_DemoQueue() -- Add Demo to Queue */
void G_DemoQueue(const char* const a_Name)
{
	G_DemoLink_t* New;
	G_DemoLink_t* Rover;
	
	/* Check */
	if (!a_Name)
		return;
	
	/* Enqueue */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	// Clone
	New->Name = Z_StrDup(a_Name, PU_STATIC, NULL);
	
	/* Chain to end */
	if (!l_DemoQ)
		l_DemoQ = New;
	else
	{
		for (Rover = l_DemoQ; Rover->Next; Rover = Rover->Next);
		
		Rover->Next = New;
	}
}

/* G_PlayNextQ() -- Play next in Q */
bool_t G_PlayNextQ(void)
{
	char DOSName[32];
	G_DemoLink_t* QNext;
	size_t i, j, k, n;
	char c;
	char* At;
	
	/* Check */
	if (!l_DemoQ)
		return false;
		
	/* Play */
	// Get next
	QNext = l_DemoQ->Next;
	
	// Formalize the name (DOS it!)
	memset(DOSName, 0, sizeof(DOSName));
	n = strlen(l_DemoQ->Name);
	for (i = 0, j = 0, k = 0; i < n; i++)
	{
		c = l_DemoQ->Name[i];

		// A valid enough character
		if (isalnum(c) || c == '-' || c == '_' || c == '~')
		{
			if (j < 8)
				DOSName[j++] = toupper(c);
		}
		
		// Special specifier
		if (c == '@')
			break;

		// Is the character a dot?
		if (c == '.')
			break;
	}
	
	// At sign in original name?
	At = strchr(l_DemoQ->Name, '@');
	
	// Free current, Set Next
	Z_Free(l_DemoQ->Name);
	Z_Free(l_DemoQ);
	l_DemoQ = QNext;
	
	// Append anything that was after the @ sign
	if (At)
	{
		strncat(DOSName, "@", 32);
		strncat(DOSName, ++At, 32);
	}
	
	// Use specified name
	G_DeferedPlayDemo(DOSName);
	return true;
}

/* G_DemoPlay() -- Plays demo with factory */
G_CurrentDemo_t* G_DemoPlay(WL_EntryStream_t* const a_Stream, const G_DemoFactory_t* const a_Factory)
{
	G_CurrentDemo_t* New;
	uint8_t Marker, MarkerB;
	
	/* Allocate Demo */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Use factory */
	// Given Factory
	if (a_Factory)
		New->Factory = a_Factory;
	
	// Auto-Detect Factory
	else
	{
		// Read first byte
		Marker = WL_StreamReadUInt8(a_Stream);
		MarkerB = WL_StreamReadUInt8(a_Stream);
		
		// Based on marker Value
			// ReMooD
		if (Marker == 'R' && MarkerB == 'E')
			New->Factory = G_DemoFactoryByName("remood");
			
			// Old 1.2 or Heretic Demo, Doom Demos (Including longtics)
		else if ((Marker <= 4) || (Marker >= 104 && Marker <= 111))
			New->Factory = G_DemoFactoryByName("vanilla");
			
			// Legacy Demo
		else if (Marker >= 112 && Marker <= 143)
			New->Factory = G_DemoFactoryByName("legacy");
		
		// Seek back to start
		WL_StreamSeek(a_Stream, 0, false);
	}
	
	/* Notice */
	CONL_PrintF("Playing \"%s\" Demo\n", New->Factory->FactoryName);
	
	/* Set Other Stuff */
	New->WLStream = a_Stream;
	
	/* Internally play it */
	if (New->Factory->StartPlayingFunc)
		if (!New->Factory->StartPlayingFunc(New))
		{
			Z_Free(New);
			return NULL;
		}
	
	/* Return it */
	return New;
}

void G_RecordDemo(char* name)
{
}

/* G_StopDemoRecord() -- Stops recording demo */
void G_StopDemoRecord(void)
{
	/* Not recording */
	if (!demorecording)
		return;
		
	/* Call handled stop demo */
	if (l_RecDemo->Factory->StopRecordFunc)
		l_RecDemo->Factory->StopRecordFunc(l_RecDemo);
	
	/* Close any files and streams */
	if (l_RecDemo->CFile)
		fclose(l_RecDemo->CFile);
	if (l_RecDemo->RBSStream)
		D_RBSCloseStream(l_RecDemo->RBSStream);
	
	/* Free current */
	Z_Free(l_RecDemo);
	l_RecDemo = NULL;
	demorecording = false;
}

/* G_StopDemoPlay() -- Stops playing demo */
void G_StopDemoPlay(void)
{
	bool_t QuitDoom, Advance;
	
	/* Not playing? */
	if (!demoplayback)
		return;
	
	/* Call handled stop demo */
	if (l_PlayDemo->Factory->StopPlayingFunc)
		l_PlayDemo->Factory->StopPlayingFunc(l_PlayDemo);
		
	// Clear reference
	Z_Free(l_PlayDemo);
	
	/* No longer playing */
	l_PlayDemo = NULL;
	demoplayback = false;
	gamestate = wipegamestate = GS_NULL;
	
	/* What to do? */
	QuitDoom = Advance = false;
	
	if (singledemo)
	{
		// Playing another demo?
		if (G_PlayNextQ())
			QuitDoom = false;
		
		// No demos left to play
		else
			QuitDoom = true;
	}
	else
		Advance = true;
	
	/* Stop recording if advancing/quitting */
	if ((QuitDoom || Advance) && demorecording)
		G_StopDemoRecord();
	
	/* Quitting? */
	if (QuitDoom)
		I_Quit();
	
	// Advance
	else if (Advance)
		D_AdvanceDemo();
}

/* G_StopDemo() -- Stops recording demo */
void G_StopDemo(void)
{
	/* Stop Playing Demo */
	if (demoplayback)
		G_StopDemoPlay();
	
	/* Stop Recording Demo */
	if (demorecording)
		G_StopDemoRecord();
}

/* G_BeginRecording() -- Begin recording demo */
void G_BeginRecording(const char* const a_Output, const char* const a_FactoryName)
{
	const G_DemoFactory_t* Factory;
	G_CurrentDemo_t* New;
	D_RBlockStream_t* RBSStream;				// Block Streamer
	void* CFile;								// CFile
	
	/* Check */
	if (!a_Output || !a_FactoryName)
		return;
	
	/* Get Factory */
	Factory = G_DemoFactoryByName(a_FactoryName);
	
	// Failed?
	if (!Factory)
		return;
		
	/* Setup file */
	RBSStream = CFile = NULL;
	if (Factory->DoesRBS)
		RBSStream = D_RBSCreateFileStream(a_Output, DRBSSF_OVERWRITE);
	else
		CFile = fopen(a_Output, "wb");
		
	// Failed?
	if (!CFile && !RBSStream)
		return;
	
	/* Create Demo */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	// Set Stuff
	New->Out = true;
	New->Factory = Factory;
	New->CFile = CFile;
	New->RBSStream = RBSStream;
	
	/* Call recorder start */
	if (New->Factory->StartRecordFunc)
		New->Factory->StartRecordFunc(New);
	
	/* Set as playing demo */
	l_RecDemo = New;
	demorecording = true;
}

/* G_DoPlayDemo() -- Plays demo */
void G_DoPlayDemo(char* defdemoname)
{
	char Base[12];
	const WL_WADEntry_t* Entry;
	WL_EntryStream_t* Stream;
	G_CurrentDemo_t* Demo;
	char* At;
	const G_DemoFactory_t* Factory;
	
	/* Check */
	if (!defdemoname)
		return;
	
	/* At sign in name? */
	// Get Base name
	memset(Base, 0, sizeof(Base));
	strncpy(Base, defdemoname, 11);
	
	// Find it
	At = strchr(Base, '@');
	
	if (At)
	{
		*(At++) = 0;
	
		// Find Factory
		Factory = G_DemoFactoryByName(defdemoname + (At - Base));
	}
	
	// No Factory specified
	else
		Factory = NULL;
	
	/* Find entry of demo */
	Entry = WL_FindEntry(NULL, 0, Base);
	
	// Not found?
	if (!Entry)
		return;
	
	/* Open stream */
	Stream = WL_StreamOpen(Entry);
	
	// Failed?
	if (!Stream)
		return;
	
	/* Stop currently playing demo */
	G_StopDemoPlay();
	
	/* Play demo in any factory */
	Demo = G_DemoPlay(Stream, Factory);
	
	// Failed?
	if (!Demo)
		return;
	
	/* Set as playing */
	demoplayback = true;
	l_PlayDemo = Demo;
	g_DemoTime = 0;
}

void G_TimeDemo(char* name)
{
}

/* G_DeferedPlayDemo() -- Defers playing back demo */
void G_DeferedPlayDemo(char* name)
{
	COM_BufAddText("playdemo \"");
	COM_BufAddText(name);
	COM_BufAddText("\"\n");
}

/* G_CheckDemoStatus() -- Sees if a demo should end */
bool_t G_CheckDemoStatus(void)
{
	bool_t RetVal = false;
	
	/* Playing Demo? */
	if (demoplayback)
		if (l_PlayDemo->Factory->CheckDemoFunc)
			if (l_PlayDemo->Factory->CheckDemoFunc(l_PlayDemo))
			{
				G_StopDemoPlay();
				RetVal = true;
			}
	
	/* Playing Demo? */
	if (demorecording)
		if (l_RecDemo->Factory->CheckDemoFunc)
			if (l_RecDemo->Factory->CheckDemoFunc(l_RecDemo))
			{
				G_StopDemoRecord();
				RetVal = true;
			}
	
	/* Return checked status, if any */
	return RetVal;
}

/* G_ReadDemoTiccmd() -- Reads demo tic command */
void G_ReadDemoTiccmd(ticcmd_t* cmd, int playernum)
{
	/* Not Playing Demo? */
	if (!demoplayback)
		return;
	
	/* Read tic command */
	if (l_PlayDemo->Factory->ReadTicCmdFunc)
		l_PlayDemo->Factory->ReadTicCmdFunc(l_PlayDemo, cmd, playernum);
}

/* G_WriteDemoTiccmd() -- Writes demo tic command */
void G_WriteDemoTiccmd(ticcmd_t* cmd, int playernum)
{
	/* Not Recording Demo? */
	if (!demorecording)
		return;
	
	/* Write tic command */
	if (l_RecDemo->Factory->WriteTicCmdFunc)
		l_RecDemo->Factory->WriteTicCmdFunc(l_RecDemo, cmd, playernum);
}

