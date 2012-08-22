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
	D_BS_t* PMPStream;				// Debug Stream
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
	{
		G_DemoProblem(true, DSTR_BADDEMO_LEVELNOTFOUND, "%s%i%i", LevelName, Episode, Map);
		return false;
	}
	
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
	P_XGSSetAllDefaults();
	P_XGSSetVersionLevel(true, VerMarker);
	
	// Options
	P_XGSSetValue(true, PGS_GAMESKILL, Data->Skill);
	P_XGSSetValue(true, PGS_GAMEDEATHMATCH, Data->Deathmatch);
	P_XGSSetValue(true, PGS_MONFASTMONSTERS, Data->Fast);
	P_XGSSetValue(true, PGS_MONRESPAWNMONSTERS, Data->Respawn);
	P_XGSSetValue(true, PGS_MONSPAWNMONSTERS, !Data->NoMonsters);
	
	// Based on Game Mode
		// Solo/Coop
	if (Data->Deathmatch == 0)
	{
		// Coop
		if (Data->MultiPlayer)
		{
			P_XGSSetValue(true, PGS_GAMESPAWNMULTIPLAYER, 1);
			P_XGSSetValue(true, PGS_ITEMSKEEPWEAPONS, 1);
		}
		
		// Solo
		else
		{
			P_XGSSetValue(true, PGS_GAMESPAWNMULTIPLAYER, 0);
			P_XGSSetValue(true, PGS_ITEMSKEEPWEAPONS, 0);
		}
	}
		// DM
	else
	{
		// Shared Flags
		P_XGSSetValue(true, PGS_GAMESPAWNMULTIPLAYER, 1);
		
		// DM
		if (Data->Deathmatch == 1)
		{
			P_XGSSetValue(true, PGS_ITEMSKEEPWEAPONS, 1);
			P_XGSSetValue(true, PGS_ITEMRESPAWNITEMS, 0);
		}
		
		// AltDM
		else
		{
			P_XGSSetValue(true, PGS_ITEMSKEEPWEAPONS, 0);
			P_XGSSetValue(true, PGS_ITEMRESPAWNITEMS, 1);
		}
	}
	
	if (Data->MultiPlayer)	// multiplayer/netgame
		P_XGSSetValue(true, PGS_COMULTIPLAYER, 1);
	else
		P_XGSSetValue(true, PGS_COMULTIPLAYER, 0);
	
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
		Data->PMPStream = D_BSCreateFileStream(M_GetNextParm(), DRBSSF_READONLY);
	
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
		if (D_BSPlayBlock(Data->PMPStream, Header))
		{
			// Read/Check gametic
			u32 = D_BSru32(Data->PMPStream);
			if (u32 > 0 && D_SyncNetMapTime() > 0)
				if (u32 != D_SyncNetMapTime())
					I_Error("PMP: gametic/MapTime Mismatch");
			
			if (u32 > 0 && D_SyncNetMapTime() > 0)
			{
				// Read/Check X Position
				i32 = D_BSri32(Data->PMPStream);
				if (abs(i32 - players[a_PlayerNum].mo->x) >= (8 << FRACBITS))
					I_Error("PMP: X Mismatch");
				
				// Read/Check Y Position
				i32 = D_BSri32(Data->PMPStream);
				if (abs(i32 - players[a_PlayerNum].mo->y) >= (8 << FRACBITS))
					I_Error("PMP: Y Mismatch");
				
				// Read/Check Z Position
				i32 = D_BSri32(Data->PMPStream);
				if (abs(i32 - players[a_PlayerNum].mo->z) >= (8 << FRACBITS))
					I_Error("PMP: Z Mismatch");
			}
		}
		
		// Ended?
		else
		{
			D_BSCloseStream(Data->PMPStream);
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
	
	// Special Action?
	if (ButtonCodes & 0x80)
	{
		// Pause
		if ((ButtonCodes & 3) == 1)
		{
			paused ^= 1;
			D_NetPlayerChangedPause(a_PlayerNum);
		}
		
		// Save Game
		else if ((ButtonCodes & 3) == 2)
			// Uh-oh!
			G_DemoProblem(true, DSTR_BADDEMO_SAVEGAMENOTSUPPORTED, "");
			
		// End of Demo
		else if (ButtonCodes == 0x80)
			Data->EndDemo = true;
	}
	
	// Normal Commands
	else
	{
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
	}
	
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
			Bits = P_XGSVal(PGS_GAMESKILL);
			fwrite(&Bits, 1, 1, a_Current->CFile);
			
			Bits = g_CurrentLevelInfo->EpisodeNum;
			fwrite(&Bits, 1, 1, a_Current->CFile);
			
			Bits = g_CurrentLevelInfo->LevelNum;
			fwrite(&Bits, 1, 1, a_Current->CFile);
			
			Bits = P_XGSVal(PGS_GAMEDEATHMATCH);
			fwrite(&Bits, 1, 1, a_Current->CFile);
			
			Bits = P_XGSVal(PGS_MONRESPAWNMONSTERS);
			fwrite(&Bits, 1, 1, a_Current->CFile);
			
			Bits = P_XGSVal(PGS_MONFASTMONSTERS);
			fwrite(&Bits, 1, 1, a_Current->CFile);
			
			Bits = !P_XGSVal(PGS_MONSPAWNMONSTERS);
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
		if (a_Cmd->buttons & BT_ATTACK)
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

/* G_LegacyExtraBuf_t -- Extra command buffer */
typedef struct G_LegacyExtraBuf_s
{
	uint8_t PlayerID;							// Player ID
	uint8_t Length;								// Length of data
	uint8_t Data[256];							// Actual Data
} G_LegacyExtraBuf_t;

/* G_LegacyDemoData_t -- Legacy Data */
typedef struct G_LegacyDemoData_s
{
	uint8_t VerMarker;							// Demo version
	bool_t EndDemo;								// End of demo
	
	uint8_t Skill, Episode, Map, DM, MultiPlayer;
	uint8_t Respawn, Fast, NoMonsters, DisplayP, TimeLimit;
	uint8_t Players[MAXPLAYERS];
	uint8_t SkinColors[MAXPLAYERS];
	
	bool_t OrigSwitch[MAXPLAYERS];
	uint8_t FavGuns[MAXPLAYERS][9];
	char Names[MAXPLAYERS][MAXPLAYERNAME];
	
	G_LegacyExtraBuf_t** ExtraBufs;				// Extra Buffers
	size_t NumExtraBufs;						// Number of extra buffers
	
	uint8_t DisplayPNode;						// Node of display player
	ticcmd_t OldCmd[MAXPLAYERS];				// Old Tic Command
} G_LegacyDemoData_t;

/*** FUNCTIONS ***/

/* G_DEMO_Legacy_StartPlaying() -- Start playing Demo */
bool_t G_DEMO_Legacy_StartPlaying(struct G_CurrentDemo_s* a_Current)
{
	int i, j, k, l, ss;
	char LevelName[9];
	G_LegacyDemoData_t* Data;
	uint8_t VerMarker, Skill, Episode, Map, DM, MultiPlayer;
	uint8_t Respawn, Fast, NoMonsters, DisplayP, TimeLimit;
	uint8_t Players[MAXPLAYERS];
	uint8_t SkinColors[MAXPLAYERS];
	bool_t OrigSwitch[MAXPLAYERS];
	uint8_t FavGuns[MAXPLAYERS][9];
	char Names[MAXPLAYERS][MAXPLAYERNAME];
	const P_LevelInfoEx_t* LevelInfo;
	const char* PlName;
	char c;
	
	/* Check */
	if (!a_Current)
		return false;
	
	/* Clear */
	memset(Players, 0, sizeof(Players));
	memset(SkinColors, 0, sizeof(SkinColors));
	memset(OrigSwitch, 0, sizeof(OrigSwitch));
	memset(FavGuns, 0, sizeof(FavGuns));
	memset(Names, 0, sizeof(Names));
	
	/* Reset Indexes */
	D_SyncNetSetMapTime(0);
	P_SetRandIndex(0);
	
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
		{
			G_DemoProblem(true, DSTR_BADDEMO_LEVELNOTFOUND, "%s%i%i", LevelName, Episode, Map);
			return false;
		}
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
		// 1.25+? Adds timelimit here
		if (VerMarker >= 125)
			TimeLimit = WL_StreamReadUInt8(a_Current->WLStream);
		
		// before 1.13, max 8 players
		if (VerMarker < 113)
		{
			// Read Player Info
			for (i = 0; i < 8; i++)
			{
				// 1.11 Encodes players in game with their skin color with
				// bit 1 set so...
				Players[i] = WL_StreamReadUInt8(a_Current->WLStream);
				
				// If it isn't set, then make them not in game
				if (!(Players[i] & 1))
					Players[i] = false;
				
				// Otherwise extract their skin color from it
				else
					SkinColors[i] = Players[i] >> 1;
			}
			
			// Read player preferences
			for (i = 0; i < 8; i++)
				if (Players[i])
				{
					// Name
					for (j = 0; j < 21; j++)
					{
						c = WL_StreamReadUInt8(a_Current->WLStream);
						
						if (j < MAXPLAYERNAME - 1)
							Names[i][j] = c;
					}
					
					// Original Switch
					OrigSwitch[i] = WL_StreamReadUInt8(a_Current->WLStream);
					
					// Favorite Guns
					for (j = 0; j < 9; j++)
					{
						FavGuns[i][j] = WL_StreamReadUInt8(a_Current->WLStream);
						
						// For some reason, the guns are zero character based!
						FavGuns[i][j] -= '0';
					}
				}
		}
		
		// Otherwise 32 players are supported here
		else
		{
			// 1.31+ saves multiplayer, but before it is implied
			if (VerMarker >= 131)
				MultiPlayer = WL_StreamReadUInt8(a_Current->WLStream);
			
			for (i = 0; i < 32; i++)
				Players[i] = WL_StreamReadUInt8(a_Current->WLStream);
		}
	}
	
	// Imply multiplayer?
	if (VerMarker < 131)
		MultiPlayer = Players[1];
	
	/* Setup Players */
	memset(playeringame, 0, sizeof(playeringame));
	memset(g_PlayerInSplit, 0, sizeof(g_PlayerInSplit));
	g_SplitScreen = -1;
	
	// Set them all up (split-screen)
	for (ss = 0, i = 0; i < MAXPLAYERS; i++)
		if (Players[i])
		{
			// Set as in game
			playeringame[i] = true;
			
			// Initialize Player
			G_AddPlayer(i);
			G_InitPlayer(&players[i]);
			
			// Before 1.13? has names and others provided by demos
			if (VerMarker < 113)
			{
				strncpy(player_names[i], Names[i], MAXPLAYERNAME);
				
				players[i].originalweaponswitch = OrigSwitch[i];
				players[i].skincolor = SkinColors[i];
				
				// Map Dehacked weapons to REMOODAT guns
				for (l = 0, j = 0; j < 9; j++)
					// Find first gun with ID and use that
					for (k = 0; k < NUMWEAPONS; k++)
						if (P_WeaponIsUnlocked(k))
							if (wpnlev1info[k]->DEHId >= 0)
								if (wpnlev1info[k]->DEHId == FavGuns[i][j])
									players[i].FavoriteWeapons[l++] = k;
			}
			
			// After that, it is provided by text commands
			else
			{
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
			}
			
			// Put in split screen
			// And before 1.30, only the display player (1.30 added ingame joining)
				// But just make 1.30 use the first 4 players
			if ((VerMarker < 130/* && ss == 0*/) || (VerMarker < 113))
				if (ss < 4)
				{
					g_SplitScreen++;
					g_PlayerInSplit[ss] = true;
					displayplayer[ss] = consoleplayer[ss] = i;
					ss++;
				}
		}
	
	/* Modify Settings required for level loading (as needed) */
	// Set version to the specified value
	P_XGSSetVersionLevel(true, VerMarker);
	P_XGSSetValue(true, PGS_GAMESKILL, Skill);
	
	// DM Before 1.27
	if (VerMarker < 127)
		P_XGSSetValue(true, PGS_GAMEDEATHMATCH, DM);
	
	// Respawn and Fast before 1.28
	if (VerMarker < 128)
	{
		P_XGSSetValue(true, PGS_MONRESPAWNMONSTERS, Respawn);
		P_XGSSetValue(true, PGS_MONFASTMONSTERS, Fast);
		
		// Time limit after 1.25
		if (VerMarker >= 125)
			P_XGSSetValue(true, PGS_GAMETIMELIMIT, TimeLimit);
	}
	
	// -nomonsters
	P_XGSSetValue(true, PGS_MONSPAWNMONSTERS, !NoMonsters);
	
	// Multiplayer
	P_XGSSetValue(true, PGS_COMULTIPLAYER, MultiPlayer);
	P_XGSSetValue(true, PGS_GAMESPAWNMULTIPLAYER, MultiPlayer);
	
	// Recalc Split-screen
	R_ExecuteSetViewSize();
	
	/* Load the level as per vanilla before 1.27 */
	if (VerMarker < 127)
		// Load the map, hopefully
		P_ExLoadLevel(LevelInfo, 0);
	
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
	{
		Data->Players[i] = Players[i];
		Data->SkinColors[i] = SkinColors[i];
		Data->OrigSwitch[i] = OrigSwitch[i];
		
		for (j = 0; j < MAXPLAYERNAME; j++)
			Data->Names[i][j] = Names[i][j];
		
		for (j = 0; j < 9; j++)
			Data->FavGuns[i][j] = FavGuns[i][j];
	}
	
	/* Display Player Warning */
	// If a demo was recorded on Legacy 1.30+ and the display (console) player
	// is not 0, then the game may have been recorded by a joining client. In
	// this case the demo cannot be played back because only the host can
	// successfully record a demo.
	if (Data->VerMarker >= 130)
		if (Data->DisplayP > 0)
			G_DemoProblem(true, DSTR_BADDEMO_NONHOSTDEMO, "");
	
	/* Success! */
	return true;
}

/* G_DEMO_Legacy_StopPlaying() -- Stop playing demo */
bool_t G_DEMO_Legacy_StopPlaying(struct G_CurrentDemo_s* a_Current)
{
	size_t i;
	G_LegacyDemoData_t* Data;
	
	/* Check */
	if (!a_Current)
		return true;
		
	/* Get */
	Data = a_Current->Data;
	
	// No data?
	if (!Data)
		return true;
	
	/* Cleanup */
	if (Data->ExtraBufs)
	{
		for (i = 0; i < Data->NumExtraBufs; i++)
			if (Data->ExtraBufs[i])
				Z_Free(Data->ExtraBufs[i]);
		Z_Free(Data->ExtraBufs);
	}
	
	// Data itself
	Z_Free(Data);

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
	G_LegacyDemoData_t* Data;
	
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

#define ZT_FWD 0x01
#define ZT_SIDE 0x02
#define ZT_ANGLE 0x04
#define ZT_BUTTONS 0x08
#define ZT_AIMING 0x10
#define ZT_CHAT 0x20
#define ZT_EXTRADATA 0x40

/* LegacyNetXCommand_t -- Legacy next commands */
typedef enum LegacyNetXCommand_s
{
	XD_NAMEANDCOLOR = 1,
	XD_WEAPONPREF,
	XD_EXIT,
	XD_QUIT,
	XD_KICK,
	XD_NETVAR,
	XD_SAY,
	XD_MAP,
	XD_EXITLEVEL,
	XD_LOADGAME,
	XD_SAVEGAME,
	XD_PAUSE,
	XD_ADDPLAYER,
	XD_ADDBOT,
	XD_USEARTEFACT,
	MAXNETXCMD
} LegacyNetXCommand_t;

/* c_LegacyNetVars -- Legacy net variable mappings */
static const struct
{
	uint16_t ID;								// Variable ID
	const char* Name;							// Name of variable
	P_XGSBitID_t DirectBitMap;					// Direct bit mapping
} c_LegacyNetVars[] =
{
	{0x6661, "sv_maxplayers"},
	{0x20af, "teamplay", PGS_GAMETEAMPLAY},
	{0x3352, "teamdamage", PGS_GAMETEAMDAMAGE},
	{0x2a45, "fraglimit", PGS_GAMEFRAGLIMIT},
	{0x2a74, "timelimit", PGS_GAMETIMELIMIT},	// breaks
	{0x34c3, "deathmatch", PGS_GAMEDEATHMATCH},
	{0x776e, "allowexitlevel", PGS_GAMEALLOWLEVELEXIT},
	{0x37c8, "allowturbo"},
	{0x2b96, "allowjump", PGS_PLENABLEJUMPING},
	{0x5304, "allowautoaim", PGS_PLALLOWAUTOAIM},
	{0x8cd2, "allowrocketjump", PGS_GAMEALLOWROCKETJUMP},
	{0x43a8, "solidcorpse", PGS_GAMESOLIDCORPSES},
	{0x55cd, "fastmonsters", PGS_MONFASTMONSTERS},
	{0xa420, "predictingmonsters", PGS_MONPREDICTMISSILES},
	{0x298f, "bloodtime", PGS_GAMEBLOODTIME},
	{0x9fbe, "fragsweaponfalling", PGS_PLDROPWEAPONS},
	{0x19b3, "gravity", PGS_GAMEGRAVITY},
	{0x8e8d, "respawnmonsters", PGS_MONRESPAWNMONSTERS},
	{0xaaa3, "respawnmonsterstime", PGS_MONRESPAWNMONSTERSTIME},
	{0x8a30, "respawnitemtime", PGS_ITEMRESPAWNITEMSTIME},
	{0x43c1, "respawnitem", PGS_ITEMRESPAWNITEMS},
	{0x3752, "allowmlook", PGS_COUSEMOUSEAIMING},	// Check this!
	{0, NULL},
};

/* GS_DEMO_Legacy_ComputeNetID() -- Computes Legacy Variable NetID */
static uint16_t GS_DEMO_Legacy_ComputeNetID(const char* s)
{
	const static int premiers[16] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53 };
	int i;
	uint16_t ret;

	ret = 0;
	i = 0;
	
	while (*s)
	{
		ret += (*s) * premiers[i];
		s++;
		i = (i + 1) % 16;
	}
	
	return ret;
}

/* GS_DEMO_Legacy_HandleExtraCmd() -- Handles legacy extra data */
static bool_t GS_DEMO_Legacy_HandleExtraCmd(struct G_CurrentDemo_s* a_Current, const G_LegacyExtraBuf_t* const a_ExtraBuf)
{
#define BUFSIZE 256
	char Buf[BUFSIZE];
	uint8_t XID;
	const uint8_t* d, *e;
	G_LegacyDemoData_t* Data;
	P_LevelInfoEx_t* Level;
	player_t* Player;
	weapontype_t FavGuns[9];
	
	int32_t i, j, k, l;
	
	uint16_t u16a;
	uint8_t u8a, u8b, u8c;
	
	/* Check */
	if (!a_Current || !a_ExtraBuf)
		return false;
	
	/* Obtain */
	Data = a_Current->Data;
	
	// Check
	if (!Data)
		return false;
	
	/* Read ID */
	d = a_ExtraBuf->Data;
	e = d + a_ExtraBuf->Length;
	Player = &players[a_ExtraBuf->PlayerID];
	
	/* Constant Loop */
	while (d < e)
	{
		// Get command code
		XID = *(d++);
	
		// Debug
		if (devparm)
			CONL_PrintF("Got Legacy XCmd %i.\n", XID);
	
		// Which command?
		switch (XID)
		{
				// 1
				// u8a = skin color
				// Buf = Name
			case XD_NAMEANDCOLOR:
				u8a = ReadUInt8(&d);
				
				// Set skin color
				Player->skincolor = u8a;
				if (Player->mo)
					Player->mo->flags = (Player->mo->flags & ~MF_TRANSLATION) | ((Player->skincolor) << MF_TRANSSHIFT);
				
				// Read name
				memset(Buf, 0, sizeof(Buf));
					// Any Size
				if (Data->VerMarker >= 128)
				{
					for (i = 0;;)
					{
						u8a = ReadUInt8(&d);
					
						// End?
						if (!u8a)
							break;
					
						// Slap into buffer
						if (i < BUFSIZE - 1)
							Buf[i++] = u8a;
					}
				}
					// MAXPLAYERNAME size
				else
				{
					for (i = 0; i < 21; i++)
						Buf[i] = ReadUInt8(&d);
					Buf[MAXPLAYERNAME - 1] = 0;
				}
				
				// Change Name
				D_NetSetPlayerName(a_ExtraBuf->PlayerID, Buf);
				
				// TODO: SetPlayerSkin
				if (Data->VerMarker < 120 || Data->VerMarker >= 125)
				{
					// Any Size
					if (Data->VerMarker >= 128)
					{
						memset(Buf, 0, sizeof(Buf));
						for (i = 0;;)
						{
							u8a = ReadUInt8(&d);
					
							// End?
							if (!u8a)
								break;
					
							// Slap into buffer
							if (i < BUFSIZE - 1)
								Buf[i++] = u8a;
						}
					}
					
					// MAXSKINNAME size
					else
					{
						for (i = 0; i < 16; i++)
							Buf[i] = ReadUInt8(&d);
						Buf[MAXPLAYERNAME - 1] = 0;
					}
				}
				break;
				
				// 2
			case XD_WEAPONPREF:
				// Original Weapon Switch
				players[a_ExtraBuf->PlayerID].originalweaponswitch = ReadUInt8(&d);
				
				// Favorite Guns
				for (j = 0; j < 9; j++)
					FavGuns[j] = ReadUInt8(&d);
				
				// Auto aim control
				players[a_ExtraBuf->PlayerID].autoaim_toggle = ReadUInt8(&d);
				
				// Map Dehacked weapons to REMOODAT guns
				for (l = 0, j = 0; j < 9; j++)
					// Find first gun with ID and use that
					for (k = 0; k < NUMWEAPONS; k++)
						if (P_WeaponIsUnlocked(k))
							if (wpnlev1info[k]->DEHId >= 0)
								if (wpnlev1info[k]->DEHId == FavGuns[j])
									players[a_ExtraBuf->PlayerID].FavoriteWeapons[l++] = k;
				break;
	
			case XD_EXIT:
				break;
	
			case XD_QUIT:
				break;
	
			case XD_KICK:
				break;
	
				// 6
				// u16a = NetVar ID
				// Buf = Value to set to
			case XD_NETVAR:
				u16a = ReadUInt16(&d);
				
				// Read value to change to
				memset(Buf, 0, sizeof(Buf));
				for (i = 0;;)
				{
					u8a = ReadUInt8(&d);
					
					// End?
					if (!u8a)
						break;
					
					// Slap into buffer
					if (i < BUFSIZE - 1)
						Buf[i++] = u8a;
				}
				
				// Seek through table and virtual command
				for (i = 0; c_LegacyNetVars[i].Name; i++)
					if (u16a == c_LegacyNetVars[i].ID)
					{
						// Message
						CONL_PrintF("Simulated change of \"%s\" to \"%s\".\n",
								c_LegacyNetVars[i].Name,
								Buf
							);
						
						// If there is a bit here, change it
						if (c_LegacyNetVars[i].DirectBitMap)
							P_XGSSetValueStr(true, c_LegacyNetVars[i].DirectBitMap, Buf);
						break;
					}
				
				// Not found
				if (!c_LegacyNetVars[i].Name)
					G_DemoProblem(true, DSTR_BADDEMO_NETVARNOTSUPPORTED, "%#04x", u16a);
				break;
	
			case XD_SAY:
				break;
	
				// 8
				// u8a = Skill
				// u8b = Extra (>= 1.28) , Nomonsters
				// u8c = Reset Players
			case XD_MAP:
				// Read info
				u8a = ReadUInt8(&d);
				
				if (Data->VerMarker >= 128)
					u8b = ReadUInt8(&d);
				else
					u8b = 0;
				
				// Extra bits in there
				if (Data->VerMarker >= 129)
				{
					u8c = !(u8b & 2);
					u8b &= 1;
				}
				else
					u8c = 0;
				
				// Read map
				memset(Buf, 0, sizeof(Buf));
				for (i = 0;;)
				{
					u8a = ReadUInt8(&d);
					
					// End?
					if (!u8a)
						break;
					
					// Slap into buffer
					if (i < BUFSIZE - 1)
						Buf[i++] = u8a;
				}
				
				// Find level
				Level = P_FindLevelByNameEx(Buf, NULL);
				
				// It was not found =(
				if (!Level)
					G_DemoProblem(true, DSTR_BADDEMO_LEVELNOTFOUND, "%s%i%i", Buf, 0, 0);
				
				// Otherwise it was found
				else
				{
					// Reborn dead players or forced reset?
					for (i = 0; i < MAXPLAYERS; i++)
						if (playeringame[i])
							if (players[i].playerstate == PST_DEAD || (Data->VerMarker >= 129 && u8c))
								players[i].playerstate = PST_REBORN;
					
					// Change monster spawning?
					if (Data->VerMarker >= 128)
						if (u8b)
							P_XGSSetValue(true, PGS_MONSPAWNMONSTERS, (u8b ? 0 : 1));
					
					// Load The level
					P_ExLoadLevel(Level, 0);
				}
				break;
	
			case XD_EXITLEVEL:
				break;
	
			case XD_LOADGAME:
				break;
	
			case XD_SAVEGAME:
				break;
	
				// 12
				// u8a = Pause State
				// u8b = Old pause state (messages)
			case XD_PAUSE:
				u8b = paused;
				
				// Before 1.31 it is just a toggle
				if (Data->VerMarker < 131)
					paused ^= 1;
				
				// However, onwards it is an actual set value
				else
				{
					u8a = ReadUInt8(&d);
					paused = !!u8a;
				}
				
				// State changed?
				if (u8b != paused)
					D_NetPlayerChangedPause(a_ExtraBuf->PlayerID);
				break;
			
				// 13
				// u8a = Node (Client ID)
				// u8b = Player Number
			case XD_ADDPLAYER:
				u8a = ReadUInt8(&d);
				u8b = ReadUInt8(&d);
				
				// This is the local demo player? If so then remember the local
				// node for future splitscreen recapture (provided player 2
				// appears later on in the game)
				if (u8b == Data->DisplayP)
					Data->DisplayPNode = u8a;
				
				// Add the player normally
				playeringame[u8b] = true;
				G_AddPlayer(u8b);
				
				// Splitscreen the player?
				if (u8a == Data->DisplayPNode)
					if (g_SplitScreen < 3)	// 0 = 1p, 1 = 2p, 2 = 3p, 3 = 4p
					{
						g_PlayerInSplit[g_SplitScreen + 1] = true;
						consoleplayer[g_SplitScreen + 1] = displayplayer[g_SplitScreen + 1] = u8b;
						g_SplitScreen++;
						
						// Recalc Split-screen
						R_ExecuteSetViewSize();
					}
				
				// Players who join a game (during a session) and record a demo
				// do not have valid playbackable demos. Only the host can record
				// a demo. Legacy joins the non-host players but never adds the
				// first host player.
				if (!playeringame[0])
					G_DemoProblem(false, DSTR_BADDEMO_NONHOSTDEMO, "");
				break;
	
			case XD_ADDBOT:
				break;
	
			case XD_USEARTEFACT:
				break;
	
				// Unknown!
			default:
				G_DemoProblem(true, DSTR_BADDEMO_UNKNOWNXDCMD, "%#02x", XID);
				break;
		}
	}
	
	/* Success! */
	return true;
#undef BUFSIZE
}

/* G_DEMO_Legacy_ReadTicCmd() -- Read Tic Command */
bool_t G_DEMO_Legacy_ReadTicCmd(struct G_CurrentDemo_s* a_Current, ticcmd_t* const a_Cmd, const int32_t a_PlayerNum)
{
#define TXTCMDBUFSIZE 256
	char Buf[TXTCMDBUFSIZE];
	G_LegacyDemoData_t* Data;
	uint8_t ButtonCodes, ZipTic, ExtraCount, CmdID;
	int32_t i;
	G_LegacyExtraBuf_t* NewBuf;
	uint8_t u8a, u8b;
	
	/* Check */
	if (!a_Current)
		return false;
	
	/* Obtain */
	Data = a_Current->Data;
	
	// Check
	if (!Data)
		return false;

	/* Clear Command */
	memset(a_Cmd, 0, sizeof(*a_Cmd));
	
	/* Old Demo Format */
	if (Data->VerMarker < 112)
	{
		// Special Action?
		if (ButtonCodes & 0x80)
		{
			// Pause
			if ((ButtonCodes & 3) == 1)
			{
				paused ^= 1;
				D_NetPlayerChangedPause(a_PlayerNum);
			}
		
			// Save Game
			else if ((ButtonCodes & 3) == 2)
				// Uh-oh!
				G_DemoProblem(true, DSTR_BADDEMO_SAVEGAMENOTSUPPORTED, "");
			
			// End of Demo
			else if (ButtonCodes == 0x80)
				Data->EndDemo = true;
		}
		
		else
		{
			// Read player's command
			a_Cmd->forwardmove = WL_StreamReadInt8(a_Current->WLStream);
			a_Cmd->sidemove = WL_StreamReadInt8(a_Current->WLStream);
			a_Cmd->angleturn = ((int16_t)WL_StreamReadInt8(a_Current->WLStream)) << 8;

			// Button codes are different in old Legacy
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
		}
	}
	
	/* New Compact Demo Format */
	else
	{
		// Read the Zip tic
		ZipTic = WL_StreamReadUInt8(a_Current->WLStream);
		
		// End of demo?
		if (ZipTic == 0x80)
		{
			Data->EndDemo = true;
			return true;
		}
		
		// Forward movement
		if (ZipTic & ZT_FWD)
			Data->OldCmd[a_PlayerNum].forwardmove = WL_StreamReadInt8(a_Current->WLStream);
		
		// Side movement
		if (ZipTic & ZT_SIDE)
			Data->OldCmd[a_PlayerNum].sidemove = WL_StreamReadInt8(a_Current->WLStream);
		
		// Angle turn
		if (ZipTic & ZT_ANGLE)
			if (Data->VerMarker < 125)
			{
				Data->OldCmd[a_PlayerNum].angleturn = WL_StreamReadInt8(a_Current->WLStream);
				Data->OldCmd[a_PlayerNum].angleturn <<= 8;
			}
			else
				Data->OldCmd[a_PlayerNum].angleturn = WL_StreamReadInt16(a_Current->WLStream);
		
		// Buttons
		if (ZipTic & ZT_BUTTONS)
		{
			// Read Base Codes
			Data->OldCmd[a_PlayerNum].buttons = 0;	// Clear!
			ButtonCodes =  WL_StreamReadUInt8(a_Current->WLStream);
			
			// Attack
			if (ButtonCodes & 1)
				Data->OldCmd[a_PlayerNum].buttons |= BT_ATTACK;
			
			// Use
			if (ButtonCodes & 2)
				Data->OldCmd[a_PlayerNum].buttons |= BT_USE;
			
			// Jump
			if (ButtonCodes & 64)
				Data->OldCmd[a_PlayerNum].buttons |= BT_JUMP;
				
			// Change gun?
			if (ButtonCodes & 4)
				// Slot based switching (BT_EXTRAWEAPON)
				if (ButtonCodes & 128)
				{
					a_Cmd->buttons |= BT_CHANGE | BT_EXTRAWEAPON;	// Slot based change
					a_Cmd->buttons |= ((((ButtonCodes & 0x38) >> 3)) << BT_SLOTSHIFT) & BT_SLOTMASK;
				}
				
				// Specific Gun (!BT_EXTRAWEAPON)
				else
				{
					// Extract Gun
					u8a = (((ButtonCodes & 0x38) >> 3));
					
					// Locate DeHackEd Weapon
					u8b = 0;
					for (i = 0; i < NUMWEAPONS; i++)
						if (P_WeaponIsUnlocked(i))
							if (wpnlev1info[i]->DEHId == u8a)
							{
								u8b = i;
								break;
							}
					
					// Use new command rather than shifties
					a_Cmd->buttons |= BT_CHANGE;
					a_Cmd->XNewWeapon |= u8b;
				}
		}
		
		// Aiming
		if (ZipTic & ZT_AIMING)
			Data->OldCmd[a_PlayerNum].aiming = WL_StreamReadInt16(a_Current->WLStream);
		
		// Chat -- Not actually used?
		if (ZipTic & ZT_CHAT)
			ButtonCodes = WL_StreamReadUInt8(a_Current->WLStream);
		
		// Extra Data
		if (ZipTic & ZT_EXTRADATA)
		{
			// Read Extra Count
			ExtraCount = WL_StreamReadUInt8(a_Current->WLStream);
			memset(Buf, 0, sizeof(Buf));
			
			// Old 1.12 Method
			if (Data->VerMarker == 112)
			{
			}
			
			// New Text Commands
			else
			{
				// Read String
				for (i = 0; i < ExtraCount; i++)
					Buf[i] = WL_StreamReadUInt8(a_Current->WLStream);
				
				// Enqueue it
				NewBuf = Z_Malloc(sizeof(*NewBuf), PU_STATIC, NULL);
				NewBuf->PlayerID = a_PlayerNum;
				NewBuf->Length = ExtraCount;
				memmove(NewBuf->Data, Buf, 256);
				
				// Find blank spot
				for (i = 0; i < Data->NumExtraBufs; i++)
					if (!Data->ExtraBufs[i])
					{
						Data->ExtraBufs[i] = NewBuf;
						break;
					}
				
				// No blank spot?
				if (i >= Data->NumExtraBufs)
				{
					Z_ResizeArray((void**)&Data->ExtraBufs, sizeof(*Data->ExtraBufs),
						Data->NumExtraBufs, Data->NumExtraBufs + 1);
					Data->ExtraBufs[Data->NumExtraBufs++] = NewBuf;
				}
			}
		}
		
		// Use old command
		memmove(a_Cmd, &Data->OldCmd[a_PlayerNum], sizeof(Data->OldCmd[a_PlayerNum]));
	}
	
	/* Success */
	return true;
#undef TXTCMDBUFSIZE
}

/* G_DEMO_Legacy_WriteTicCmd() -- Write Tic Commnd */
bool_t G_DEMO_Legacy_WriteTicCmd(struct G_CurrentDemo_s* a_Current, const ticcmd_t* const a_Cmd, const int32_t a_PlayerNum)
{
	return false;
}

/* G_DEMO_Legacy_PostGTickCmd() -- Post tic command */
bool_t G_DEMO_Legacy_PostGTickCmd(struct G_CurrentDemo_s* a_Current)
{
	G_LegacyDemoData_t* Data;
	size_t i;
	
	/* Check */
	if (!a_Current)
		return false;
	
	/* Obtain */
	Data = a_Current->Data;
	
	// Check
	if (!Data)
		return false;
	
	/* Handle all the commands */
	// No commands to execute
	if (!Data->ExtraBufs)
		return true;
	
	// While there is a command
	while (Data->ExtraBufs[0])
	{
		// Handle the command
		GS_DEMO_Legacy_HandleExtraCmd(a_Current, Data->ExtraBufs[0]);
		
		// Free the current
		Z_Free(Data->ExtraBufs[0]);
		
		// Move everything down
		for (i = 0; i < Data->NumExtraBufs - 1; i++)
			Data->ExtraBufs[i] = Data->ExtraBufs[i + 1];
		Data->ExtraBufs[Data->NumExtraBufs - 1] = NULL;
	}
	
	/* Success */
	return true;
}

#undef ZT_FWD
#undef ZT_SIDE
#undef ZT_ANGLE
#undef ZT_BUTTONS
#undef ZT_AIMING
#undef ZT_CHAT
#undef ZT_EXTRADATA

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
static bool_t l_CommandedDemo = false;			// Commanded demo
static bool_t l_DemoServer = false;				// Server playing demos

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
		G_DEMO_Vanilla_WriteTicCmd,
		NULL,
		NULL,
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
		G_DEMO_Legacy_WriteTicCmd,
		NULL,
		G_DEMO_Legacy_PostGTickCmd,
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
		G_DEMO_ReMooD_WriteTicCmd,
		NULL,
		NULL,
	},
	
	// End
	{NULL},
};

/*** FUNCTIONS ***/

/* CLC_PlayDemo() -- Plays a demo */
static int CLC_PlayDemo(const uint32_t a_ArgC, const char** const a_ArgV)
{
	/* Check */
	if (a_ArgC < 2)
	{
		CONL_PrintF("Usage: %s <Demo Name>\n", a_ArgV[0]);
		return CLE_FAILURE;
	}
	
	/* Stop old demo from playing */
	if (demoplayback)
		G_StopDemo();
	
	/* Play Demo */
	l_CommandedDemo = true;
	G_DoPlayDemo(a_ArgV[1]);
}

/* G_PrepareDemoStuff() -- Registers any demo stuff */
void G_PrepareDemoStuff(void)
{
	CONL_AddCommand("playdemo", CLC_PlayDemo);
}

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
	
	// No factory?
	if (!New->Factory)
	{
		Z_Free(New);
		G_DemoProblem(true, DSTR_BADDEMO_UNKNOWNFACTORY, "");
		return NULL;
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
		D_BSCloseStream(l_RecDemo->RBSStream);
	
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
	else if (!l_CommandedDemo)
		Advance = true;
	else if (l_CommandedDemo)
	{
		l_CommandedDemo = false;
		D_StartTitle();
	}
	
	/* If not a server playing demos (demoplayback server) */
	// Disconnect from "ourself"
	if (!l_DemoServer)
		D_NCDisconnect();
	
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
	D_BS_t* RBSStream;				// Block Streamer
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
		RBSStream = D_BSCreateFileStream(a_Output, DRBSSF_OVERWRITE);
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
	
	/* If not a server playing demos (demoplayback server) */
	// We need to switch to a server state before servers can be played.
	if (!l_DemoServer)
	{
		D_NCDisconnect();
		D_NCServize();
	}
	
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
	CONL_InputF("playdemo \"%s\"\n", name);
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

/* G_DemoPreGTicker() -- Pre demo ticker */
void G_DemoPreGTicker(void)
{
	/* Playing Demo? */
	if (demoplayback)
		if (l_PlayDemo)
			if (l_PlayDemo->Factory->PreGTickCmdFunc)
				l_PlayDemo->Factory->PreGTickCmdFunc(l_PlayDemo);
	
	/* Recording Demo? */
	if (demorecording)
		if (l_RecDemo)
			if (l_RecDemo->Factory->PreGTickCmdFunc)
				l_RecDemo->Factory->PreGTickCmdFunc(l_RecDemo);
}

/* G_DemoPostGTicker() -- Post demo ticker */
void G_DemoPostGTicker(void)
{
	/* Playing Demo? */
	if (demoplayback)
		if (l_PlayDemo)
			if (l_PlayDemo->Factory->PostGTickCmdFunc)
				l_PlayDemo->Factory->PostGTickCmdFunc(l_PlayDemo);
	
	/* Recording Demo? */
	if (demorecording)
		if (l_RecDemo)
			if (l_RecDemo->Factory->PostGTickCmdFunc)
				l_RecDemo->Factory->PostGTickCmdFunc(l_RecDemo);
}

/* G_DemoProblem() -- Problem with demo! Uh oh! */
void G_DemoProblem(const bool_t a_IsError, const UnicodeStringID_t a_StrID, const char* const a_Format, ...)
{
	va_list ArgPtr;
	
	/* Check */
	if (!a_Format)
		return;
	
	/* Which sound to play? */
	//if (a_IsError)
	//	S_StartSound(NULL, sfx_lotime);
	//else
		S_StartSound(NULL, sfx_gerror);	
	
	/* Send to PrintV() */
	va_start(ArgPtr, a_Format);
	CONL_PrintF("{%c", (a_IsError ? '1' : '3'));
	CONL_UnicodePrintV(false, a_StrID, a_Format, ArgPtr);
	CONL_PrintF("\n");
	va_end(ArgPtr);
}

