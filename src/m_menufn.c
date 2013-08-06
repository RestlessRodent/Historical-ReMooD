// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------
// Copyright (C) 2012-2013 GhostlyDeath <ghostlydeath@remood.org>
//                                      <ghostlydeath@gmail.com>
// ----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// ----------------------------------------------------------------------------
// DESCRIPTION: Menu Code

/***************
*** INCLUDES ***
***************/

#include "m_menu.h"
#include "p_demcmp.h"
#include "p_local.h"
#include "r_data.h"
#include "p_info.h"
#include "m_random.h"
#include "m_menupv.h"
#include "g_game.h"
#include "d_main.h"
#include "w_wad.h"
#include "d_net.h"
#include "i_util.h"

/****************
*** FUNCTIONS ***
****************/

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/* M_HelpInitIWADList() -- Generates IWAD list for selection */
int32_t M_HelpInitIWADList(CONL_VarPossibleValue_t** const a_PossibleOut)
{
	char Buf[PATH_MAX];
	static CONL_VarPossibleValue_t* IWADListPV;
	int32_t i, j, ThisWAD, n, k;
	D_IWADInfoEx_t* Info;
	const char* SubField;
	bool_t Found;
	
	const char** OKList;
	size_t OKn;
	
	/* If list exists, return that */
	if (IWADListPV)
	{
		// Return possible output
		if (a_PossibleOut)
			*a_PossibleOut = IWADListPV;
		
		// Return Currently selected WAD
		for (i = 0; IWADListPV[i].StrAlias; i++)
			if (D_GetIWADInfoByNum(IWADListPV[i].IntVal) == D_GetThisIWAD())
				return i;
		
		// Not found, presume zero
		return 0;
	}
	
	/* Otherwise, it needs generation */
	i = -1;
	Info = NULL;
	ThisWAD = 0;
	n = 0;
	OKList = NULL;
	OKn = 0;
	
	do
	{
		// Only if there is info
		if (Info)
		{
			// Not found
			Found = false;
			
			// Try and locate WAD File, to see if it exists in the standard locations
			for (j = 0; ; j++)
			{
				// Get sub field of IWAD Name
				SubField = D_FieldNumber(Info->BaseName, j);
				
				// End of fields?
				if (!SubField)
					break;
				
				// If it exists in the OK list ignore
				for (k = 0; k < OKn; k++)
					if (strcasecmp(SubField, OKList[k]) == 0)
					{
						k = -1;
						break;
					}
				
				// See if it exists
				if (k >= 0)
				{
					// Clear
					memset(Buf, 0, sizeof(Buf));
					
					// Try to find it
					if (WL_LocateWAD(SubField, Info->MD5Sum, Buf, PATH_MAX - 1))
					{
						// Add to OK list to cancel out name
							// this is so we do not bother checking other versions
							// of WADs like doom.wad
						Z_ResizeArray((void**)&OKList, sizeof(*OKList),
							OKn, OKn + 1);
						OKList[OKn++] = SubField;
				
						Found = true;
						break;	// No need to continue
					}
				}
			}
			
			// WAD was found somewhere
			if (Found)
			{
				// Matches this WAD?
				if (ThisWAD == 0 && Info == D_GetThisIWAD())
					ThisWAD = n;
				
				// Add to possible list
				Z_ResizeArray((void**)&IWADListPV, sizeof(*IWADListPV),
					n, n + 1);
				IWADListPV[n].IntVal = i;
				IWADListPV[n++].StrAlias = Info->NiceTitle;
			}
		}
		
		// Obtain info for next WAD
		Info = D_GetIWADInfoByNum(++i);
	} while (Info);
	
	// Nothing in list?
	if (n == 0)
	{
		Z_ResizeArray((void**)&IWADListPV, sizeof(*IWADListPV),
			n, n + 1);
		IWADListPV[n].IntVal = 0;
		IWADListPV[n++].StrAlias = "This IWAD";
	}
	
	// Add blank spot
	Z_ResizeArray((void**)&IWADListPV, sizeof(*IWADListPV),
		n, n + 1);
	
	// Return current WAD
	if (a_PossibleOut)
		*a_PossibleOut = IWADListPV;
	return ThisWAD;
}

/* --- NEW GAME MENU --- */

/* M_MainMenu_DCursor() -- Draws cursor over item */
void M_MainMenu_DCursor(M_SWidget_t* const a_Widget, M_SWidget_t* const a_Sub)
{
	bool_t SkullNum;
	
	/* Which skull? */
	SkullNum = !!(g_ProgramTic & 0x8);
	
	/* Load Skull? */
	if (!a_Widget->Data.MainMenu.Skulls[SkullNum])
		a_Widget->Data.MainMenu.Skulls[SkullNum] = V_ImageFindA((SkullNum ? "M_SKULL2" : "M_SKULL1"), VCP_DOOM);
	
	/* Draw it */
	V_ImageDraw(
			0,
			a_Widget->Data.MainMenu.Skulls[SkullNum],
			a_Sub->dx - 32,
			a_Sub->dy - 5,
			NULL
		);
}

/* M_SubMenu_FSelect() -- Opens submenu */
bool_t M_SubMenu_FSelect(M_SWidget_t* const a_Widget)
{
	M_SMSpawn(a_Widget->Screen, a_Widget->SubMenu);
	return true;
}

/* M_NewGameClassic_FSelect() -- Classic is selected at the top menu */
bool_t M_NewGameClassic_FSelect(M_SWidget_t* const a_Widget)
{
	/* Clear next game vars */
	NG_ResetVars();
	
	/* Doom */
	if (g_CoreGame == CG_DOOM)
	{
		// Doom II
		if ((g_IWADFlags & (CIF_COMMERCIAL | CIF_EXTENDED)) == (CIF_COMMERCIAL))
		{
			NG_SetNextMap("map01");
			M_SMSpawn(a_Widget->Screen, MSM_SKILLSELECTDOOM);
			return true;
		}
		
		// Doom II: BFG Edition
		else if ((g_IWADFlags & (CIF_COMMERCIAL | CIF_EXTENDED)) == (CIF_COMMERCIAL | CIF_EXTENDED))
		{
		}
		
		// Shareware Doom
		else if ((g_IWADFlags & (CIF_SHAREWARE)) == (CIF_SHAREWARE))
		{
			NG_SetNextMap("e1m1");
			M_SMSpawn(a_Widget->Screen, MSM_SKILLSELECTDOOM);
			return true;
		}
		
		// Registered Doom
		else if ((g_IWADFlags & (CIF_REGISTERED | CIF_EXTENDED)) == (CIF_REGISTERED))
		{
			M_SMSpawn(a_Widget->Screen, MSM_EPISELECTDOOM);
			return true;
		}
		
		// Ultimate Doom
		else if ((g_IWADFlags & (CIF_REGISTERED | CIF_EXTENDED)) == (CIF_REGISTERED | CIF_EXTENDED))
		{
			M_SMSpawn(a_Widget->Screen, MSM_EPISELECTUDOOM);
			return true;
		}
		
		// Unknown
		else
			return false;
	}
	
	/* Heretic */
	else if (g_CoreGame == CG_HERETIC)
	{
	}
	
	/* Unknown!? */
	else
		return false;
	
	/* Success! */
	return true;
}

#define MAXFSIZE 64
typedef struct M_SvIndex_s
{
	bool_t OK;
	char Left[MAXSERVERNAME];
	char Right[MAXFSIZE];
	char* LR, *RR;
	I_HostAddress_t Addr;
	tic_t Timer;
} M_SvIndex_t;

static int32_t l_SvPage;						// Current server page
static M_SvIndex_t l_E[MAXSERVERSONLIST];

/* M_NewGameClassic_ServerFSelect() -- Server select */
bool_t M_NewGameClassic_ServerFSelect(M_SWidget_t* const a_Widget)
{
#define BUFSIZE 128
	char Buf[BUFSIZE];
	M_SvIndex_t* Idx;
	int32_t SlotRef, MasterRef;
	
	/* Determine reference */
	SlotRef = a_Widget->Option - 10;
	MasterRef = (l_SvPage * MAXSERVERSONLIST) + SlotRef;
	Idx = &l_E[SlotRef];
	
	/* Make sure entry is OK */
	if (!Idx->OK)
		return true;
		
	/* Pop all menus */
	M_StackPopAll();
	
	/* The "connect" command handles everything */
	I_NetHostToString(&Idx->Addr, Buf, BUFSIZE);
	CONL_InputF("connect \"%s\" --\n", Buf);
	
	/* Success !*/
	return true;
#undef BUFSIZE
}

/* M_NewGameClassic_ServerFSelect() -- Left/Right on server list */
bool_t M_NewGameClassic_ServerFLeftRight(M_SWidget_t* const a_Widget, const int32_t a_Right)
{
	/* Success !*/
	return true;
}

/* M_NewGameClassic_ServerFTicker() -- Ticker for server list */
bool_t M_NewGameClassic_ServerFTicker(M_SWidget_t* const a_Widget)
{
	M_SvIndex_t* Idx;
	SN_Server_t* Server;
	int32_t SlotRef, MasterRef;
	M_SWidget_t* Parent;
	
	/* Determine reference */
	SlotRef = a_Widget->Option - 10;
	MasterRef = (l_SvPage * MAXSERVERSONLIST) + SlotRef;
	Idx = &l_E[SlotRef];
	
	/* Check timer on this one */
	if (g_ProgramTic < Idx->Timer)
		return true;
	
	// Up the timer
	Idx->Timer = g_ProgramTic + TICRATE;
	
	/* Always set widget pointers */
	Idx->LR = Idx->Left;
	Idx->RR = Idx->Right;
	a_Widget->Data.Label.Ref = &Idx->LR;
	a_Widget->Data.Label.ValRef = &Idx->RR;
	
	/* Obtain server here */
	Server = SN_FindServerByIndex(MasterRef);
	
	// No server?
	if (!Server)
	{
		Idx->Left[0] = '-';
		Idx->Left[1] = '-';
		Idx->Left[2] = 0;
		Idx->Right[0] = 0;
		Idx->OK = false;
		return;
	}
	
	// Entry is OK
	Idx->OK = true;
	
	// Copy address
	memmove(&Idx->Addr, &Server->Addr, sizeof(Idx->Addr));
	
	// Otherwise, use server name
	strncpy(Idx->Left, Server->Name, MAXSERVERNAME - 1);
	Idx->Left[MAXSERVERNAME - 1] = 0;
	
	/* Success !*/
	return true;
}

/* M_NewGameEpi_FSelect() -- Episode Selected */
bool_t M_NewGameEpi_FSelect(M_SWidget_t* const a_Widget)
{
#define BUFSIZE 8
	char Buf[BUFSIZE];
	snprintf(Buf, BUFSIZE, "e%dm1", a_Widget->Option);
	NG_SetNextMap(Buf);
	M_SMSpawn(a_Widget->Screen, MSM_SKILLSELECTDOOM);
	return true;
#undef BUFSIZE
}

/* M_NewGameSkill_FSelect() -- Selects skill */
bool_t M_NewGameSkill_FSelect(M_SWidget_t* const a_Widget)
{
	NG_SetVarValue(PGS_GAMESKILL, a_Widget->Option);
	
	// I'm Too Young To Die
	if (a_Widget->Option == 0)
		NG_SetVarValue(PGS_PLHALFDAMAGE, 1);
	
	// I'm Too Young To Die/Nightmare
	if (a_Widget->Option == 0 || a_Widget->Option == 4)
		NG_SetVarValue(PGS_PLDOUBLEAMMO, 1);
	
	// Nightmare
	if (a_Widget->Option == 4)
	{
		NG_SetVarValue(PGS_MONFASTMONSTERS, 1);
		NG_SetVarValue(PGS_MONRESPAWNMONSTERS, 1);
	}
	
	/* Make Game Now */
	SN_StartLocalServer(0, NULL, true, true);
	
	/* Kill all menus */
	M_StackPopAll();
	return true;
}

/* --------------------- */

/* --- QUIT GAME MENU --- */

/* M_QuitGame_DisconFSelect() -- Disconnect from server */
bool_t M_QuitGame_DisconFSelect(M_SWidget_t* const a_Widget)
{
	/* Disconnect from Netgame */
	SN_Disconnect(false, "Disconnected");
	M_StackPopAll();
	return true;
}

/* M_QuitGame_PDisconFSelect() -- Disconnect from server */
bool_t M_QuitGame_PDisconFSelect(M_SWidget_t* const a_Widget)
{
	/* Disconnect from Netgame */
	SN_PartialDisconnect("Disconnected");
	M_StackPopAll();
	return true;
}

/* M_QuitGame_StopWatchFSelect() -- Stop watching demo */
bool_t M_QuitGame_StopWatchFSelect(M_SWidget_t* const a_Widget)
{
	/* Stop Demo from Playing */
	if (demoplayback)
	{
		G_StopDemoPlay();
		M_StackPopAll();
	}
	return true;
}

/* M_QuitGame_StopRecordFSelect() -- Stop recording demo */
bool_t M_QuitGame_StopRecordFSelect(M_SWidget_t* const a_Widget)
{
	/* Stop Demo from Recording */
	if (demorecording)
	{
		G_StopDemoRecord();
		M_StackPopAll();
	}
	return true;
}

/* M_QuitGame_LogOffFSelect() -- Stop recording demo */
bool_t M_QuitGame_LogOffFSelect(M_SWidget_t* const a_Widget)
{
	return true;
}

/* M_QuitGame_ExitFSelect() -- Stop recording demo */
bool_t M_QuitGame_ExitFSelect(M_SWidget_t* const a_Widget)
{
	I_Quit();
	return true;
}

/* M_QuitGame_FTicker() -- Ticker for quit game */
void M_QuitGame_FTicker(M_SWidget_t* const a_Widget)
{
	size_t i;
	M_SWidget_t* Kid;
	
	/* Go through all options */
	for (i = 0; i < a_Widget->NumKids; i++)
	{
		// Get kid
		Kid = a_Widget->Kids[i];
		
		if (!Kid)
			continue;
		
		// Depends on the function
			// Disconnect
		if (Kid->FSelect == M_QuitGame_DisconFSelect)
		{
			if (gamestate != GS_DEMOSCREEN && !demoplayback)
				Kid->Flags &= ~MSWF_DISABLED;
			else
				Kid->Flags |= MSWF_DISABLED;
		}
			
			// Partial Disconnect
		else if (Kid->FSelect == M_QuitGame_PDisconFSelect)
		{
			if (gamestate != GS_DEMOSCREEN && !demoplayback)
				Kid->Flags &= ~MSWF_DISABLED;
			else
				Kid->Flags |= MSWF_DISABLED;
		}
				
			// Stop watching demo
		else if (Kid->FSelect == M_QuitGame_StopWatchFSelect)
		{
			if (demoplayback)
				Kid->Flags &= ~MSWF_DISABLED;
			else
				Kid->Flags |= MSWF_DISABLED;
		}
				
			// Stop Recording
		else if (Kid->FSelect == M_QuitGame_StopRecordFSelect)
		{
			if (demorecording)
				Kid->Flags &= ~MSWF_DISABLED;
			else
				Kid->Flags |= MSWF_DISABLED;
		}
			
			// Log Off
		else if (Kid->FSelect == M_QuitGame_LogOffFSelect)
		{
			Kid->Flags |= MSWF_DISABLED;
		}
		
		// If disabled, do not select
		if (Kid->Flags & MSWF_DISABLED)
		{
			// Do not select
			Kid->Flags |= MSWF_NOSELECT;
			
			if (i == a_Widget->CursorOn)
				a_Widget->CursorOn = (a_Widget->CursorOn + 1) % a_Widget->NumKids;
		}
		
		// Not disabled
		else
			Kid->Flags &= ~MSWF_NOSELECT;
	}
	
	return;
}

/* ---------------------- */

/* --- ADVANCED CREATE GAME MENU --- */

extern const void* g_ReMooDPtr;

/* M_ACG_CreateFSelect() -- Start Game is selected */
void M_ACG_CreateFSelect(M_SWidget_t* const a_Widget)
{
	M_SWidget_t* Parent;
	M_SWidget_t* Map[NUMMCGO];
	int32_t i;
	D_IWADInfoEx_t* IWADInfo, *ThisIWAD;
	char Path[PATH_MAX];
	const char* Field;
	bool_t LatchedIWAD;
	const WL_WADFile_t* WAD, *OldIWAD;
	
	/* Map options to widgets */
	memset(Map, 0, sizeof(Map));
	Parent = a_Widget->Parent;
	for (i = 0; i < Parent->NumKids; i++)
		if (Parent->Kids[i])
			if (Parent->Kids[i]->Option > 0 && Parent->Kids[i]->Option < NUMMCGO)
				Map[Parent->Kids[i]->Option] = Parent->Kids[i];
	
	/* Disconnect */
	SN_Disconnect(false, "Starting new game");
	
	/* Lock OCCB */
	WL_LockOCCB(true);
	
	/* Change WADS, if need be */
	// IWAD
	if (Map[MCGO_IWAD])
	{
		// Get IWADs in use
		IWADInfo = D_GetIWADInfoByNum(Map[MCGO_IWAD]->Data.Label.Possible[Map[MCGO_IWAD]->Data.Label.Pivot].IntVal);
		ThisIWAD = D_GetThisIWAD();
		
		// Different IWADs?
		if (IWADInfo != ThisIWAD)
		{
			// First get the existing IWAD
			OldIWAD = WL_IterateVWAD(NULL, true);
			
			// Pop all wads possible
			while (WL_PopWAD())
				;
			
			// Push IWAD
			for (i = 0, LatchedIWAD = false; ; i++)
			{
				// Get field?
				Field = D_FieldNumber(IWADInfo->BaseName, i);
				
				// End?
				if (!Field)
					break;
				
				// Determine path
				memset(Path, 0, sizeof(Path));
				if ((LatchedIWAD = WL_LocateWAD(Field, IWADInfo->MD5Sum, Path, PATH_MAX - 1)))
				{
					// Try opening
					WAD = WL_OpenWAD(Path, NULL);
					
					if (!WAD)
						LatchedIWAD = false;
					else
						break;
				}
			}
			
			// Got IWAD?
			if (LatchedIWAD && WAD)
				WL_PushWAD(WAD);
			
			// Otherwise, push old IWAD
			else
				WL_PushWAD(OldIWAD);
			
			// Push ReMooD.WAD			
			WL_PushWAD(g_ReMooDPtr);
		}
	}
	
	// PWADs
	
	/* UnLock OCCB */
	WL_LockOCCB(false);
	
	// Clean up any unused WADs
	WL_CloseNotStacked();
	
	/* Set new game options */
	
	/* Pop All Menus */
	M_StackPopAll();
	
	/* Add any local players */
	
	/* Setup Server */
	//D_XNetMakeServer(false /*TODO*/, NULL, 0, false);
	
	/* Add any bots */
}

/* --------------------------------- */

/* --- CONNECT TO UNLISTED SERVER --- */

#define CONNECTIPSIZE 72
static char l_ConnectIP[CONNECTIPSIZE];

/* M_CTUS_ConnectFSelect() -- Connect Selected */
void M_CTUS_ConnectFSelect(M_SWidget_t* const a_Widget)
{
	int32_t i;
	M_SWidget_t* Wid;
	I_EventEx_t Evt;
	char* p;
	
	/* Obtain string buffer by sending \n to inputter */
	// Widget is NULL if called from callback!
	if (a_Widget)
	{
		for (i = 0; i < a_Widget->Parent->NumKids; i++)
			if (a_Widget->Parent->Kids[i])
				if (a_Widget->Parent->Kids[i]->Option == 1337)
				{
					Wid = a_Widget->Parent->Kids[i];
					
					// Send enter
					memset(&Evt, 0, sizeof(Evt));
					
					Evt.Type = IET_SYNTHOSK;
					Evt.Data.SynthOSK.PNum = Wid->Screen;
					Evt.Data.SynthOSK.KeyCode = IKBK_RETURN;
					
					// Fake a handle event
					CONCTI_HandleEvent(Wid->Data.TextBox.Inputter, &Evt);
					//I_EventExPush(&Evt);
					return;
				}
		
		// Return from now
		return;
	}
	
	/* Do standard connect, if there is a string there */
	// Check for no string
	if (!l_ConnectIP[0])
		return;
	
	/* Pop all menus */
	M_StackPopAll();
	
	/* The "connect" command handles everything */
	CONL_InputF("connect \"%s\" --\n", l_ConnectIP);
}

/* M_CTUS_BoxCallBack() -- Callback for enter on the box */
bool_t M_CTUS_BoxCallBack(struct CONCTI_Inputter_s* a_Inputter, const char* const a_Str)
{
	M_SWidget_t* Widget;
	
	/* Empty string? */
	if (!a_Str || !a_Str[0])
		return false;
	
	/* No longer steal focus */
	Widget = (M_SWidget_t*)a_Inputter->DataRef;
	Widget->Data.TextBox.StealInput = false;
	
	/* Copy string to buffer */
	memset(l_ConnectIP, 0, sizeof(l_ConnectIP));
	strncpy(l_ConnectIP, a_Str, CONNECTIPSIZE - 1);
	
	/* Set inputter to that text */
	CONCTI_SetText(a_Inputter, l_ConnectIP);
	
	/* Call Connect Func */
	M_CTUS_ConnectFSelect(NULL);
	
	/* Return false, do not want box destroyed */
	return false;
}

/* ---------------------------------- */

/* --- PROFILE MANAGER --- */

/* M_ProfMan_FTicker() -- Ticker for profile list */
// This function dynamically recreates the menu to list all profiles
void M_ProfMan_FTicker(M_SWidget_t* const a_Widget)
{
	int32_t i;
	M_SWidget_t* Wid;
	D_Prof_t* Prof;
	
	/* Go through sub options */
	for (i = 0; i < MAXPROFCONST; i++)
	{
		// Get widget
		Wid = a_Widget->Kids[i + 1];
		
		// Get profile
		Prof = g_ProfList[i];
		
		// If not profile, disable selection
		if (!Prof)
		{
			Wid->Data.Label.Ref = NULL;
			Wid->Flags |= MSWF_NOSELECT | MSWF_DISABLED;
			
			if (Wid->CursorOn == i + 1)
				Wid->CursorOn = 0;
			continue;
		}
		
		// Allow selection
		Wid->Flags &= ~(MSWF_NOSELECT | MSWF_DISABLED);
		
		// Set profile to name
		Prof->AccountRef = Prof->AccountName;
		Prof->DisplayRef = Prof->DisplayName;
		Wid->Data.Label.Ref = &Prof->AccountRef;
	}
}

D_Prof_t* g_DoProf = NULL;

/* M_ProfMan_CreateProf() -- Create profile */
bool_t M_ProfMan_CreateProf(M_SWidget_t* const a_Widget)
{
	char Buf[MAXPLAYERNAME];
	int32_t i;
	D_Prof_t* Prof;
	M_SWidget_t* Sub;
	
	/* Add basic profile with generic no name */
	// Find player to setup
	for (Prof = NULL, i = 0; i < 9999; i++)
	{
		// Make temp name
		snprintf(Buf, MAXPLAYERNAME - 1, "player%i", i + 1);
		Buf[MAXPLAYERNAME - 1] = 0;
		
		// See if profile exists
		if ((Prof = D_FindProfileEx(Buf)))
		{
			Prof = NULL;
			continue;
		}
		
		// Does not exist
		break;
	}
	
	/* Create profile with said name */
	Prof = D_CreateProfileEx(Buf);
	
	// Failed for some reason?
	if (!Prof)
		return true;
	
	/* See if profile is in slot */
	for (i = 0; i < MAXPROFCONST; i++)
		if (g_ProfList[i] == Prof)
			break;
	
	// Not in first n profiles
	if (i >= MAXPROFCONST)
		return true;
	
	/* Go straight to modification setup */
	g_DoProf = Prof;
	Sub = M_SMSpawn(a_Widget->Screen, MSM_PROFMOD);
	
	if (Sub)
		Sub->Prof = Prof;
	
	/* Success! */
	return true;
}

/* M_ProfMan_IndvFSel() -- Going to modify profile */
bool_t M_ProfMan_IndvFSel(M_SWidget_t* const a_Widget)
{
	M_SWidget_t* Sub;
	
	/* Create submenu to modify profile */
	g_DoProf = g_ProfList[a_Widget->Option];
	Sub = M_SMSpawn(a_Widget->Screen, MSM_PROFMOD);
	
	if (Sub)
		Sub->Prof = g_ProfList[a_Widget->Option];
	
	/* Success? */
	return true;
}

/* M_ProfMan_AcctBCB() -- Callback for account name */
bool_t M_ProfMan_AcctBCB(struct CONCTI_Inputter_s* a_Inputter, const char* const a_Str)
{
	M_SWidget_t* Widget;
	
	/* Empty string? */
	if (!a_Str || !a_Str[0])
		return false;
	
	/* No longer steal focus */
	Widget = (M_SWidget_t*)a_Inputter->DataRef;
	Widget->Data.TextBox.StealInput = false;
	
	/* Attempt rename of profile */
	D_ProfRename(Widget->Parent->Prof, a_Str);
	
	// Set text to account name regardless if change failed
	CONCTI_SetText(a_Inputter, Widget->Parent->Prof->AccountName);
	
	/* Return false, do not want box destroyed */
	return false;
}

/* ----------------------- */


