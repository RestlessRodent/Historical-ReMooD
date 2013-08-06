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
// DESCRIPTION:
//      Archiving: SaveGame I/O.

/***************
*** INCLUDES ***
***************/

#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "r_state.h"
#include "z_zone.h"
#include "w_wad.h"
#include "p_setup.h"
#include "t_vari.h"
#include "t_script.h"
#include "t_func.h"
#include "m_random.h"
#include "m_misc.h"
#include "p_saveg.h"
#include "console.h"
#include "p_demcmp.h"
#include "m_menu.h"
#include "b_bot.h"
#include "d_net.h"
#include "d_netcmd.h"
#include "r_main.h"
#include "r_sky.h"
#include "p_inter.h"
#include "wi_stuff.h"

/**************
*** GLOBALS ***
**************/

extern mobj_t* g_LFPRover;

/*************
*** LOCALS ***
*************/

static P_SaveSubVersion_t l_SSV;				// Sub save version
static bool_t l_SoloLoad;						// Single player load game

/****************
*** FUNCTIONS ***
****************/

/* CLC_SaveGame() -- Saves the game */
static int CLC_SaveGame(const uint32_t a_ArgC, const char** const a_ArgV)
{
	/* Check */
	if (a_ArgC < 2)
	{
		CONL_OutputF("Usage: %s \"<filename>\"\n", a_ArgV[0]);
		return 1;
	}
	
	/* Save the game */
	if (strcasecmp(a_ArgV[0], "save") == 0)
	{
		if (P_SaveGameEx(NULL, a_ArgV[1], strlen(a_ArgV[1]), NULL, NULL))
			return 0;
	}
	
	/* Load the game */
	else if (strcasecmp(a_ArgV[0], "load") == 0)
	{
		if (P_LoadGameEx(NULL, a_ArgV[1], strlen(a_ArgV[1]), NULL, NULL))
			return 0;
	}
	
	/* Return failure otherwise */
	return 1;
}

/* P_InitSGConsole() -- Initialize save command */
void P_InitSGConsole(void)
{
	/* Add command */
	CONL_AddCommand("save", CLC_SaveGame);
	CONL_AddCommand("load", CLC_SaveGame);
}

/* P_SaveGameEx() -- Extended savegame */
bool_t P_SaveGameEx(const char* SaveName, const char* ExtFileName, size_t ExtFileNameLen, size_t* SaveLen, uint8_t** Origin)
{
	bool_t OK = false;
	D_BS_t* BS = D_BSCreateFileStream(ExtFileName, DRBSSF_OVERWRITE);
	D_BS_t* CS;
	
	/* Failed? */
	if (!BS)
		return false;
	
	/* Create Compressed Stream */
	CS = D_BSCreatePackedStream(BS);
	
	// Failed?
	if (!CS)
	{
		D_BSCloseStream(BS);
		return false;
	}
		
	/* Save */
	OK = P_SaveToStream(CS, BS);
	
	// Close
	D_BSCloseStream(CS);
	D_BSCloseStream(BS);
	
	return OK;
}

/* P_LoadGameEx() -- Load an extended save game */
bool_t P_LoadGameEx(const char* FileName, const char* ExtFileName, size_t ExtFileNameLen, size_t* SaveLen, uint8_t** Origin)
{
	bool_t OK = false;
	D_BS_t* BS = D_BSCreateFileStream(ExtFileName, DRBSSF_READONLY);
	D_BS_t* CS;
	
	/* Failed? */
	if (!BS)
		return false;
	
	/* Create Compressed Stream */
	CS = D_BSCreatePackedStream(BS);
	
	// Failed?
	if (!CS)
	{
		D_BSCloseStream(BS);
		return false;
	}
		
	/* Load */
	OK = P_LoadFromStream(CS, false);
	
	// Close
	D_BSCloseStream(CS);
	D_BSCloseStream(BS);
	
	return OK;
}

/*****************************************************************************/

/* PS_IllegalSave() -- Savegame is illegal */
static bool_t PS_IllegalSave(const int32_t a_Reason)
{
	CONL_PrintF("Illegal savegame: %s\n", DS_GetString(a_Reason));
	return false;
}

/* PS_Expect() -- Expect header */
static bool_t PS_Expect(D_BS_t* const a_Str, const char* const a_Header)
{
	char Header[5] = {0, 0, 0, 0, 0};
	
	/* Play Block */
	if (!D_BSPlayBlock(a_Str, Header))
		return PS_IllegalSave(DSTR_PSAVEGC_ENDOFSTREAM);
	
	/* Compare */
	if (!D_BSCompareHeader(a_Header, Header))
		return PS_IllegalSave(DSTR_PSAVEGC_WRONGHEADER);
	
	// Was OK
	return true;
}

/* PS_GetCeilingListID() -- Get ID from Ceiling list */
static int32_t PS_GetCeilingListID(ceilinglist_t* const a_CList)
{
	int32_t RetVal;
	ceilinglist_t* Rover;
	
	/* Check */
	if (!a_CList)
		return -1;
	
	/* Rove */
	for (RetVal = 1, Rover = activeceilings; Rover; Rover = Rover->next, RetVal++)
		if (a_CList == Rover)
			return RetVal;
	
	/* Not Found??? */
	return 0;
}


/* PS_GetPlatListID() -- Get ID from Plat list */
static int32_t PS_GetPlatListID(platlist_t* const a_PList)
{
	int32_t RetVal;
	platlist_t* Rover;
	
	/* Check */
	if (!a_PList)
		return -1;
	
	/* Rove */
	for (RetVal = 1, Rover = activeplats; Rover; Rover = Rover->next, RetVal++)
		if (a_PList == Rover)
			return RetVal;
	
	/* Not Found??? */
	return 0;
}

/* PS_GetThinkerID() -- Returns ID of thinker */
static int32_t PS_GetThinkerID(thinker_t* const a_Thinker)
{
	int32_t RetVal;
	thinker_t* Rover;
	
	/* Check */
	if (!a_Thinker)
		return -1;
	
	/* Is cap? */
	if (a_Thinker == &thinkercap)
		return 0;
	
	/* Thinker Loop */
	for (RetVal = 0, Rover = thinkercap.next; Rover != &thinkercap; Rover = Rover->next, RetVal++)
		// Is this one?
		if (Rover == a_Thinker)
			return RetVal + 1;
	
	/* Not in chain? */
	return -2;
}

/* PS_GetThinkerFromID() -- Returns thinker_t of ID */
static thinker_t* PS_GetThinkerFromID(const int32_t a_ID)
{
	thinker_t* Rover;
	int32_t i;
	
	/* Invalid or not in chain */
	if (a_ID < 0)
		return NULL;
	
	/* Is cap? */
	if (a_ID == 0)
		return &thinkercap;
	
	/* Thinker Loop */
	for (i = 0, Rover = thinkercap.next; Rover != &thinkercap; Rover = Rover->next, i++)
		// Is this one?
		if (i + 1 == a_ID)
			return Rover;
	
	/* Not found */
	return NULL;
}

/* PS_LoadUnloadStateP() -- Saves/Restores Camera */
static void PS_StateP(D_BS_t* const a_Str, const bool_t a_Write, PI_state_t** const a_StateP)
{
#define BUFSIZE 64
	char Buf[BUFSIZE];
	uint8_t nn;
	uint32_t mk;
	uint32_t i, ID;
	
	/* Save */
	if (a_Write)
	{
		// No state?
		if (!*a_StateP)
		{
			D_BSwu8(a_Str, 0);
		}
		
		// Is a state
		else
		{
			D_BSwu8(a_Str, 1);
			
			// Write Origin Thing ID
			D_BSwu8(a_Str, (*a_StateP)->Origin.Type);
			
			switch ((*a_StateP)->Origin.Type)
			{
					// Thing
				case 0:
					D_BSws(a_Str, mobjinfo[(*a_StateP)->Origin.ID]->RClassName);
					break;
					
					// Weapon
				case 1:
					D_BSws(a_Str, wpnlev1info[(*a_StateP)->Origin.ID]->ClassName);
					break;
					
					// S_NULL?
				default:
					D_BSwu8(a_Str, 0);
					break;
			}
			
			// Write Marker
			D_BSwu32(a_Str, (*a_StateP)->Marker);
		}
	}
	
	/* Load */
	else
	{
		// NULL State?
		nn = D_BSru8(a_Str);
		
		// None
		if (!nn)
		{
			*a_StateP = NULL;
		}
		
		// Is set
		else if (nn == 1)
		{
			// Read Type, String, Marker
			nn = D_BSru8(a_Str);
			D_BSrs(a_Str, Buf, BUFSIZE);
			mk = D_BSru32(a_Str);
			
			// Reference ID
			switch (nn)
			{
					// Thing
				case 0:
					ID = INFO_GetTypeByName(Buf);
					break;
					
					// Weapon
				case 1:
					ID = INFO_GetWeaponByName(Buf);
					break;
					
				default:
					ID = UINT32_C(0xFFFFFFFF);
					break;
			}
			
			// Search all states for origin and marker match
			for (i = 0; i < NUMSTATES; i++)
				if (states[i]->Origin.Type == nn &&
					states[i]->Origin.ID == ID &&
					states[i]->Marker == mk)
				{
					*a_StateP = states[i];
					return;
				}
		}
	}
#undef BUFSIZE
}

/* PS_LoadUnloadCamera() -- Saves/Restores Camera */
static void PS_LoadUnloadCamera(D_BS_t* const a_Str, const bool_t a_Write, camera_t* const a_Cam)
{
}

/* PS_LoadUnloadInvenT() -- Saves/Restores Inventory */
static void PS_LoadUnloadInvenT(D_BS_t* const a_Str, const bool_t a_Write, inventory_t* const a_Inven)
{
}

/* PS_LoadUnloadPSPDef() -- Saves/Restores PSP Definition */
static void PS_LoadUnloadPSPDef(D_BS_t* const a_Str, const bool_t a_Write, pspdef_t* const a_PSP)
{
	/* Save */
	if (a_Write)
	{
		PS_StateP(a_Str, a_Write, &a_PSP->state);
		D_BSwi32(a_Str, a_PSP->tics);
		D_BSwi32(a_Str, a_PSP->sx);
		D_BSwi32(a_Str, a_PSP->sy);
	}
	
	/* Load */
	else
	{
		PS_StateP(a_Str, a_Write, &a_PSP->state);
		a_PSP->tics = D_BSri32(a_Str);
		a_PSP->sx = D_BSri32(a_Str);
		a_PSP->sy = D_BSri32(a_Str);
	}
}

/* PS_LoadUnloadTicCmd() -- Saves/Restores Tic Command */
static void PS_LoadUnloadTicCmd(D_BS_t* const a_Str, const bool_t a_Write, ticcmd_t* const a_TC)
{
	int32_t i, n;
	uint8_t v;
	
	/* Saving */
	if (a_Write)
	{
		// Save control
		D_BSwu8(a_Str, a_TC->Ctrl.Type);
		D_BSwu16(a_Str, a_TC->Ctrl.Ping);
		
		// Players
		if (a_TC->Ctrl.Type == 0)
		{
			// Standard Commands
			D_BSwi8(a_Str, a_TC->Std.forwardmove);
			D_BSwi8(a_Str, a_TC->Std.sidemove);
			D_BSwi16(a_Str, a_TC->Std.angleturn);
			D_BSwu16(a_Str, a_TC->Std.aiming);
			D_BSwu32(a_Str, a_TC->Std.buttons);
			D_BSwu8(a_Str, a_TC->Std.artifact);
			D_BSwi16(a_Str, a_TC->Std.BaseAngleTurn);
			D_BSwi16(a_Str, a_TC->Std.BaseAiming);
			D_BSwu8(a_Str, a_TC->Std.InventoryBits);
			D_BSwu32(a_Str, a_TC->Std.StatFlags);
			
			// Extended Command Buffer
			D_BSwu16(a_Str, a_TC->Std.DataSize);
			D_BSwu16(a_Str, MAXTCDATABUF);
			
			for (i = 0; i < MAXTCDATABUF; i++)
				D_BSwu8(a_Str, a_TC->Std.DataBuf[i]);
		}
		
		// Commands Only
		else
		{
			D_BSwu16(a_Str, a_TC->Ext.DataSize);
			D_BSwu16(a_Str, MAXTCDATABUF);
			
			for (i = 0; i < MAXTCDATABUF; i++)
				D_BSwu8(a_Str, a_TC->Ext.DataBuf[i]);
		}
	}
	
	/* Loading */
	else
	{
		// Read control
		a_TC->Ctrl.Type = D_BSru8(a_Str);
		a_TC->Ctrl.Ping = D_BSru16(a_Str);
		
		// Players
		if (a_TC->Ctrl.Type == 0)
		{
			// Standard Commands
			a_TC->Std.forwardmove = D_BSri8(a_Str);
			a_TC->Std.sidemove = D_BSri8(a_Str);
			a_TC->Std.angleturn = D_BSri16(a_Str);
			a_TC->Std.aiming = D_BSru16(a_Str);
			a_TC->Std.buttons = D_BSru32(a_Str);
			a_TC->Std.artifact = D_BSru8(a_Str);
			a_TC->Std.BaseAngleTurn = D_BSri16(a_Str);
			a_TC->Std.BaseAiming = D_BSri16(a_Str);
			a_TC->Std.InventoryBits = D_BSru8(a_Str);
			a_TC->Std.StatFlags = D_BSru32(a_Str);
			
			// Extended Command Buffer
			a_TC->Std.DataSize = D_BSru16(a_Str);
			n = D_BSru16(a_Str);
			
			for (i = 0; i < n; i++)
			{
				v = D_BSru8(a_Str);
				
				if (i < MAXTCDATABUF)
					a_TC->Std.DataBuf[i] = v;
			}
		}
		
		// Commands Only
		else
		{
			a_TC->Ext.DataSize = D_BSru16(a_Str);
			n = D_BSru16(a_Str);
			
			for (i = 0; i < n; i++)
			{
				v = D_BSru8(a_Str);
				
				if (i < MAXTCDATABUF)
					a_TC->Ext.DataBuf[i] = v;
			}
		}
	}
}

/* PS_LoadUnloadNoiseThinker() -- Saves/Restores Noise Thinker (sound origin) */
static void PS_LoadUnloadNoiseThinker(D_BS_t* const a_Str, const bool_t a_Write, S_NoiseThinker_t* const a_NT)
{
	/* Saving */
	if (a_Write)
	{
		D_BSwu32(a_Str, a_NT->Flags);
		D_BSwi32(a_Str, a_NT->x);
		D_BSwi32(a_Str, a_NT->y);
		D_BSwi32(a_Str, a_NT->z);
		D_BSwi32(a_Str, a_NT->momx);
		D_BSwi32(a_Str, a_NT->momy);
		D_BSwi32(a_Str, a_NT->momz);
		D_BSwi32(a_Str, a_NT->Pitch);
		D_BSwi32(a_Str, a_NT->Volume);
		D_BSwu32(a_Str, a_NT->Angle);
	}
	
	/* Loading */
	else
	{
		a_NT->Flags = D_BSru32(a_Str);
		a_NT->x = D_BSri32(a_Str);
		a_NT->y = D_BSri32(a_Str);
		a_NT->z = D_BSri32(a_Str);
		a_NT->momx = D_BSri32(a_Str);
		a_NT->momy = D_BSri32(a_Str);
		a_NT->momz = D_BSri32(a_Str);
		a_NT->Pitch = D_BSri32(a_Str);
		a_NT->Volume = D_BSri32(a_Str);
		a_NT->Angle = D_BSru32(a_Str);
	}
}

/* PS_LUMapObjRef() -- Loads/Unloads a map object reference */
static void PS_LUMapObjRef(D_BS_t* const a_Str, const bool_t a_Write, void** const a_Ref)
{
	uint8_t IDType;
	uint32_t RefNum;	
	
	/* Saving */
	if (a_Write)
	{
#define __DEF(id,ref) {IDType = (id); RefNum = (ref);}

		// Vertex
		if (*a_Ref >= (void*)vertexes && *a_Ref < (void*)&vertexes[numvertexes])
			__DEF(1, ((vertex_t*)*a_Ref) - vertexes)
			
		// Segs
		else if (*a_Ref >= (void*)segs && *a_Ref < (void*)&segs[numsegs])
			__DEF(2, ((seg_t*)*a_Ref) - segs)
			
		// Sectors
		else if (*a_Ref >= (void*)sectors && *a_Ref < (void*)&sectors[numsectors])
			__DEF(3, ((sector_t*)*a_Ref) - sectors)
			
		// SubSectors
		else if (*a_Ref >= (void*)subsectors && *a_Ref < (void*)&subsectors[numsubsectors])
			__DEF(4, ((subsector_t*)*a_Ref) - subsectors)
			
		// Nodes
		else if (*a_Ref >= (void*)nodes && *a_Ref < (void*)&nodes[numnodes])
			__DEF(5, ((node_t*)*a_Ref) - nodes)
			
		// Lines
		else if (*a_Ref >= (void*)lines && *a_Ref < (void*)&lines[numlines])
			__DEF(6, ((line_t*)*a_Ref) - lines)
			
		// Sides
		else if (*a_Ref >= (void*)sides && *a_Ref < (void*)&sides[numsides])
			__DEF(7, ((side_t*)*a_Ref) - sides)
		
		// Map Things
		else if (*a_Ref >= (void*)mapthings && *a_Ref < (void*)&mapthings[nummapthings])
			__DEF(8, ((mapthing_t*)*a_Ref) - mapthings)
		
		// NULL or invalid
		else
		{
			IDType = 0;
			RefNum = 0;
		}
		
		// Write it
		D_BSwu8(a_Str, IDType);
		D_BSwu32(a_Str, RefNum);
#undef __DEF
	}
	
	/* Loading */
	else
	{
		// Read Type and reference number
		IDType = D_BSru8(a_Str);
		RefNum = D_BSru32(a_Str);
		
		// Which Type?
		switch (IDType)
		{
#define __CHK(b,rng) if (a_Ref) *a_Ref = ((RefNum >= 0 && RefNum < (rng)) ? (&((b)[RefNum])) : NULL)
			case 1: __CHK(vertexes, numvertexes); break;
			case 2: __CHK(segs, numsegs); break;
			case 3: __CHK(sectors, numsectors); break;
			case 4: __CHK(subsectors, numsubsectors); break;
			case 5: __CHK(nodes, numnodes); break;
			case 6: __CHK(lines, numlines); break;
			case 7: __CHK(sides, numsides); break;
			case 8: __CHK(mapthings, nummapthings); break;
			
				// NULL or invalid
			default:
				*a_Ref = NULL;
				break;
		}
#undef __CHK
	}
}

/*****************************************************************************/

/* PS_SaveDummy() -- Saves dummy data */
// Save Header that is
static void PS_SaveDummy(D_BS_t* const a_Str, const bool_t a_Tail)
{
	/* Base */
	D_BSBaseBlock(a_Str, "SAVG");
	
	/* Encode */
	// Version
	D_BSwu8(a_Str, VERSION);
	D_BSwu8(a_Str, REMOOD_MAJORVERSION);
	D_BSwu8(a_Str, REMOOD_MINORVERSION);
	D_BSwu8(a_Str, REMOOD_RELEASEVERSION);
	D_BSws(a_Str, REMOOD_VERSIONCODESTRING);
	D_BSws(a_Str, REMOOD_URL);
	
	// Version
	D_BSwu32(a_Str, PSSV_LATEST);
	
	// Sync
	D_BSwu32(a_Str,  G_CalcSyncCode(false));
	
	/* Record */
	D_BSRecordBlock(a_Str);
}

/* PS_LoadDummy() -- Loads dummy data */
// Save Header that is
static bool_t PS_LoadDummy(D_BS_t* const a_Str, const bool_t a_Tail)
{
#define BUFSIZE 64
	uint8_t Ver[4];
	char Buf[BUFSIZE];
	uint32_t u32;
	int i;
	
	/* Expect "SAVG" */
	if (!PS_Expect(a_Str, "SAVG"))
		return false;
	
	/* Decode */
	// Only bother if not the tail
	if (!a_Tail)
	{
		// Read Version
		for (i = 0; i < 4; i++)
			Ver[i] = D_BSru8(a_Str);
		D_BSrs(a_Str, Buf, BUFSIZE);
		
		// Print Version
		CONL_PrintF("Loading %hhu.%hhu%c \"%s\" savegame.\n",
				Ver[1], Ver[2], Ver[3], Buf
			);
		
		// Read URL
		D_BSrs(a_Str, Buf, BUFSIZE);
		CONL_PrintF("See: %s\n", Buf);
		
		// Sub save version
		l_SSV = D_BSru32(a_Str);
		
		// Sync Code
		u32 = D_BSru32(a_Str);
	}
	
	// Print Notice
	else
	{
		CONL_PrintF("Loaded savegame.\n");
	}
	
	/* Success! */
	return true;
#undef BUFSIZE
}

/*---------------------------------------------------------------------------*/


extern SN_Host_t*** g_HostsP;
extern int32_t* g_NumHostsP;

#define GHOSTS (*g_HostsP)
#define GNUMHOSTS (*g_NumHostsP)

/* PS_SaveNetState() -- Saves networked state */
static void PS_SaveNetState(D_BS_t* const a_Str)
{
	int32_t i, j;
	SN_Host_t* Host;
	SN_Port_t* Port;
	
	/* Save Hosts */
	D_BSBaseBlock(a_Str, "HOST");
	
	// Current Host ID
	Host = SN_MyHost();
	if (Host)
		D_BSwu32(a_Str, Host->ID);
	else
		D_BSwu32(a_Str, 0);
	
	// Go through all hosts
	for (i = 0; i < GNUMHOSTS; i++)
	{
		// Get Host
		if (!(Host = GHOSTS[i]))
			continue;
		
		// Something is here
		D_BSwu8(a_Str, 1);
		
		// Dump host data
		D_BSwu32(a_Str, Host->ID);
		D_BSwu8(a_Str, Host->Local);
		D_BSwu8(a_Str, Host->Cleanup);
		D_BSws(a_Str, Host->QuitReason);
	}
	
	// End
	D_BSwu8(a_Str, 0);
	
	// Encode
	D_BSRecordBlock(a_Str);
	
	/* Save Ports */
	D_BSBaseBlock(a_Str, "PORT");
	
	// Go through all hosts again
	for (i = 0; i < GNUMHOSTS; i++)
	{
		// Get Host
		if (!(Host = GHOSTS[i]))
			continue;
		
		// Go through ports
		for (j = 0; j < Host->NumPorts; j++)
		{
			// Get Port
			if (!(Port = Host->Ports[j]))
				continue;
			
			// Something is here
			D_BSwu8(a_Str, 1);
			
			// Write data
			D_BSwu32(a_Str, Port->ID);
			D_BSwu32(a_Str, Port->Host->ID);
			D_BSws(a_Str, Port->Name);
			D_BSwu8(a_Str, Port->Bot);
			D_BSwi32(a_Str, ((Port->Player) ? Port->Player - players : -1));
			D_BSwi32(a_Str, Port->Screen);
			D_BSwu32(a_Str, Port->ProcessID);
			
			// Profile
			if (Port->Profile)
			{
				D_BSws(a_Str, Port->Profile->UUID);
				D_BSws(a_Str, Port->Profile->AccountName);
			}
			else
			{
				D_BSwu8(a_Str, 0);
				D_BSwu8(a_Str, 0);
			}
			
			// Fields reserved for bot
			if (Port->Bot)
			{
				D_BSws(a_Str, "BotUUID");
				D_BSws(a_Str, "BotName");
				D_BSwu32(a_Str, 0);
				D_BSwu32(a_Str, 0);
				D_BSwu32(a_Str, 0);
				D_BSwu32(a_Str, 0);
				D_BSwu32(a_Str, 0);
			}
		}
	}
	
	// End
	D_BSwu8(a_Str, 0);
	
	// Encode
	D_BSRecordBlock(a_Str);
}

/* PS_LoadNetState() -- Loads networked state */
static bool_t PS_LoadNetState(D_BS_t* const a_Str)
{
#define UUIDLEN (MAXUUIDLENGTH + 2)
	char Buf[UUIDLEN];
	SN_Host_t* MyHost;
	SN_Host_t* Host;
	SN_Port_t* Port;
	uint32_t ID, ReadID, PortID, LocalID;
	int32_t TempI, j;
	bool_t Local;
	D_SplitInfo_t* Split;
	D_Prof_t* Prof;
	
	/* Get current host, if connected */
	// Or if in single player game
	MyHost = NULL;
	if (SN_IsConnected() || l_SoloLoad)
		MyHost = SN_MyHost();
	
	/* Expect "HOST" */
	if (!PS_Expect(a_Str, "HOST"))
		return false;
	
	// Read my ID
	ID = D_BSru32(a_Str);
	
	LocalID = 0;
	if (MyHost)
		LocalID = MyHost->ID;
	
	// Host reading loop
	for (;;)
	{
		// End?
		if (!D_BSru8(a_Str))
			break;
		
		// Read the ID here
		ReadID = D_BSru32(a_Str);
		Local = D_BSru8(a_Str);
		
		// This is OUR host?
		if (MyHost && MyHost->ID == ReadID)
			Local = true;	// Connected
		else if ((l_SoloLoad || !MyHost) && ID == ReadID)
			Local = true;	// Loading save game (take control of ourself)
		else
			Local = false;	// Do not possess this host
		
		// Claim this host
		if (MyHost && Local)
		{
			Host = MyHost;
			
			// Use this ID in single player
				// Since we take over an existing host structure
			if (l_SoloLoad)
				LocalID = ReadID;
		}
		
		// Create host, if this is not ours
		else
		{
			if (!(Host = SN_HostByID(ReadID)))	// maybe it already exists?
				Host = SN_CreateHost();
			
			// Make my host
			if (Local)
				LocalID = ReadID;
		}
		
		// Fill in fields
		Host->ID = ReadID;	// Solo replaces host
		Host->Local = Local;
		Host->Cleanup = D_BSru8(a_Str);
		D_BSrs(a_Str, Host->QuitReason, MAXQUITREASON);
		
		// If single player, drop non-locals (but keep local)
		if (l_SoloLoad)
		{
			Host->Cleanup = !Host->Local;
			
			// If cleaning up, change message when they all die
			if (Host->Cleanup)
			{
				strncpy(Host->QuitReason, "Non-local player", MAXQUITREASON - 1);
				Host->QuitReason[MAXQUITREASON - 1] = 0;
			}
		}
	}
	
	// Set local host
	MyHost = SN_HostByID(LocalID);
	SN_SetMyHost(MyHost);
		
	/* Expect "PORT" */
	if (!PS_Expect(a_Str, "PORT"))
		return false;
	
	// Port reading loop
	for (;;)
	{
		// End?
		if (!D_BSru8(a_Str))
			break;
		
		// Read Port IDs
		PortID = D_BSru32(a_Str);
		ReadID = D_BSru32(a_Str);
		
		// Find host that owns this port
		Host = SN_HostByID(ReadID);
		
		// Something bad happened
		if (!Host)
			return PS_IllegalSave(DSTR_PSAVEGC_ILLEGALHOST);
		
		// Create Port for this host
		Port = SN_AddPort(Host);
		
		// Set fields
		Port->ID = PortID;
		D_BSrs(a_Str, Port->Name, MAXPLAYERNAME);
		Port->Bot = D_BSru8(a_Str);
		
		// Read player ID
		TempI = D_BSri32(a_Str);
		
		if (TempI >= 0 && TempI < MAXPLAYERS)
		{
			Port->Player = &players[TempI];
			players[TempI].Port = Port;
		}
		
		// Screen
		Port->Screen = D_BSri32(a_Str);
		Port->ProcessID = D_BSru32(a_Str);
		
		// Profile
		for (TempI = 0; TempI < 2; TempI++)
		{
			// Read string
			D_BSrs(a_Str, Buf, UUIDLEN);
			
			// If profile not set, try setting
			if (!Port->Profile)
				if ((Prof = D_FindProfileEx(Buf)))
					SN_SetPortProfile(Port, Prof);
		}
		
		// Reserved for bot
		if (Port->Bot)
		{
			D_BSrs(a_Str, NULL, 0);
			D_BSrs(a_Str, NULL, 0);
			D_BSru32(a_Str);
			D_BSru32(a_Str);
			D_BSru32(a_Str);
			D_BSru32(a_Str);
			D_BSru32(a_Str);
		}
	}
	
	/* Reset all splits */
	if (l_SoloLoad)
	{
		D_NCResetSplits(demoplayback);
		g_SplitScreen = -1;
	}
	
	/* Re-allocate local screens to determine who is who */
	for (TempI = 0; TempI < MyHost->NumPorts; TempI++)
	{
		// Get current port
		if (!(Port = MyHost->Ports[TempI]))
			continue;
		
		// Ignore bot or non-screened players
		if (Port->Bot || Port->Screen < 0 || Port->Screen >= MAXSPLITSCREEN)
			continue;
		
		// Get this split
		Split = &g_Splits[Port->Screen];
		
		// Two players own the same split!?
		if (Split->DoNotSteal)
			continue;
		
		// Initialize split for this port
		if (Port->Player)
		{
			Split->Active = true;
			Split->Console = Split->Display = Port->Player - players;
		}
		
		else
			Split->Console = Split->Display = -1;
		
		if (!demoplayback)
			Split->Waiting = true;
		
		// Do not steal
		Split->DoNotSteal = true;
		Split->Port = Port;
		Split->Profile = Port->Profile;
		
		// Increase split count
		g_SplitScreen++;
	}
	
	// Fixup splits
	R_ExecuteSetViewSize();
	
#if 0
	uint32_t OurHost, SaveHost;
	int32_t i;
	uint8_t Code;
	D_XPlayer_t *XPlay;
	char NameBuf[MAXPLAYERNAME];
	B_BotTemplate_t* BotTemp;
	
	/* Expect "NSTA" */
	if (!PS_Expect(a_Str, "NSTA"))
		return false;
	
	/* Get associated host IDs */
	OurHost = D_XNetGetHostID();
	SaveHost = D_BSru32(a_Str);
	
	/* Read XPlayers */
	for (;;)
	{
		// Read Code
		Code = D_BSru8(a_Str);
		
		// End of list?
		if (!Code)
			break;
		
		// Clear fake player
		XPlay = Z_Malloc(sizeof(*XPlay), PU_STATIC, NULL);
		
		// Link into list
		for (i = 0; i < g_NumXPlays; i++)
			if (!g_XPlays[i])
			{
				g_XPlays[i] = XPlay;
				break;
			}
		
		// No room? Add to end
		if (i >= g_NumXPlays)
		{
			Z_ResizeArray((void**)&g_XPlays, sizeof(*g_XPlays),
				g_NumXPlays, g_NumXPlays + 1);
			g_XPlays[g_NumXPlays++] = XPlay;
		}
		
		// Read into fake player
		XPlay->ID = D_BSru32(a_Str);
		XPlay->HostID = D_BSru32(a_Str);
		XPlay->ClProcessID = D_BSru32(a_Str);
		
		XPlay->Flags = D_BSru32(a_Str);
		
		D_BSrs(a_Str, XPlay->AccountName, MAXPLAYERNAME);
		D_BSrs(a_Str, XPlay->DisplayName, MAXPLAYERNAME);
		D_BSrs(a_Str, XPlay->ProfileUUID, MAXUUIDLENGTH);
		D_BSrs(a_Str, XPlay->LoginUUID, MAXUUIDLENGTH);
		D_BSrs(a_Str, XPlay->AccountServer, MAXXSOCKTEXTSIZE);
		D_BSrs(a_Str, XPlay->AccountCookie, MAXPLAYERNAME);
		
		XPlay->ScreenID = D_BSri8(a_Str);
		XPlay->InGameID = D_BSri32(a_Str);
		XPlay->Ping = D_BSri32(a_Str);
		XPlay->StatusBits = D_BSru32(a_Str);
		
		D_BSrs(a_Str, NameBuf, MAXPLAYERNAME);
		
		// Playing a demo back, do not assign to screen
		if (demoplayback)
		{
			XPlay->Flags &= ~(DXPF_SERVER | DXPF_LOCAL);
			XPlay->Flags |= DXPF_DEMO;
		}
		
		// Client, and this is our XPlayer
		else if (OurHost != 0 && OurHost == XPlay->HostID)
		{
			XPlay->Flags &= ~DXPF_SERVER;
			XPlay->Flags |= DXPF_LOCAL;
			
			// As long as it is not a bot, bind to a screen!
			if (!(XPlay->Flags & DXPF_BOT))
			{
				// Find local process split (if any)
				i = D_NCSFindSplitByProcess(XPlay->ClProcessID);
				
				// Not found
				if (i < 0)
				{
					// Find first free spot (hopefuly)
					for (i = 0; i < MAXSPLITSCREEN; i++)
						if (!D_ScrSplitHasPlayer(i))
							break;
					
					// There is room
					if (i < MAXSPLITSCREEN)
					{
						g_Splits[i].Waiting = true;
						g_Splits[i].ProcessID = XPlay->ClProcessID;
						
						// Resize all screens
						g_SplitScreen++;
						R_ExecuteSetViewSize();
					}
				}
				
				// Found it
				else
				{
					// Use said profile
					if (g_Splits[i].Profile)
						D_XNetChangeLocalProf(i, g_Splits[i].Profile);
					
					// Screens need increasing?
					if (i >= g_SplitScreen)
					{
						g_SplitScreen = i;
						R_ExecuteSetViewSize();
					}
				}
				
				// Set common stuff
				if (i < MAXSPLITSCREEN)
				{
					XPlay->ScreenID = i;
					g_Splits[i].ProcessID = XPlay->ClProcessID;
					g_Splits[i].Console = 0;
					g_Splits[i].Display = -1;
					g_Splits[i].XPlayer = XPlay;
				}
			}
		}
		
		// Client, and this is server host
		else if (OurHost != 0 && SaveHost == XPlay->HostID)
		{
			// Make non-local but a server
			XPlay->Flags &= ~DXPF_LOCAL;
			XPlay->Flags |= DXPF_SERVER;
		}
		
		// Client, and this is someone else
		else if (OurHost != 0)
		{
			// Make non-local
			XPlay->Flags &= ~DXPF_LOCAL;
		}
		
		// Server load
		else if (OurHost == 0)
		{
			// Bot?
			if (XPlay->Flags & DXPF_BOT)
			{
				// Try and find the bot template
				BotTemp = B_GHOST_FindTemplate(NameBuf);
				
				// Not found? use random then
				if (!BotTemp)
					BotTemp = B_GHOST_RandomTemplate();
				
				// Initialize bot at this player
				XPlay->BotData = B_InitBot(BotTemp);
				//XPlay->BotData->XPlayer = XPlay;
			}
			
			// Server player
			else if (XPlay->Flags & DXPF_SERVER)
			{
				XPlay->Flags |= DXPF_LOCAL;
				
				// Re-assign to current splits, if possible
				if (XPlay->ScreenID >= 0 && XPlay->ScreenID < MAXSPLITSCREEN)
				{
					// Playing the game
					if (XPlay->InGameID >= 0)
					{
						g_Splits[XPlay->ScreenID].Waiting = false;
						g_Splits[XPlay->ScreenID].Active = true;
						g_Splits[XPlay->ScreenID].Console = XPlay->InGameID;
						g_Splits[XPlay->ScreenID].Display = g_Splits[XPlay->ScreenID].Console;
					}
					
					// Not active
					else
					{
						g_Splits[XPlay->ScreenID].Waiting = true;
						g_Splits[XPlay->ScreenID].Active = false;
						g_Splits[XPlay->ScreenID].Console = -1;
						g_Splits[XPlay->ScreenID].Display = -1;
					}
					
					// Find profile
					D_XNetChangeLocalProf(XPlay->ScreenID, D_FindProfileEx(XPlay->ProfileUUID));
					
					// Set profile and XPlayer
					g_Splits[XPlay->ScreenID].ProcessID = XPlay->ClProcessID;
					g_Splits[XPlay->ScreenID].Profile = XPlay->Profile;
					g_Splits[XPlay->ScreenID].XPlayer = XPlay;
					
					// Split screen count needs resize?
					if (XPlay->ScreenID >= g_SplitScreen)
					{
						g_SplitScreen = XPlay->ScreenID;
						R_ExecuteSetViewSize();
					}
				}
			}
			
			// Non-Server Player (delete next tic)
			else
			{
				XPlay->Flags &= ~(DXPF_LOCAL);
				XPlay->Flags |= DXPF_DEFUNCT;
			}
		}
		
		// See if we can obtain the profile used by the player
		if (XPlay->Flags & DXPF_LOCAL)
		{
			// Assign profile, if any
			//if (!XPlay->Profile)
			//	XPlay->Profile = D_FindProfileEx(XPlay->ProfileUUID);
			
			// Set screen profile
			if (XPlay->ScreenID >= 0 && XPlay->ScreenID < MAXSPLITSCREEN)
				if (!g_Splits[XPlay->ScreenID].Profile)
					g_Splits[XPlay->ScreenID].Profile = XPlay->Profile;
		}
		
		// If the player is in game, assign it
		if (XPlay->InGameID >= 0 && XPlay->InGameID < MAXPLAYERS)
		{
			XPlay->Player = &players[XPlay->InGameID];
			XPlay->Player->XPlayer = XPlay;
			XPlay->Player->ProfileEx = XPlay->Profile;
		}
	}
	
	/* Success! */
	return true;
#endif
	return true;
#undef UUIDLEN
}

/*---------------------------------------------------------------------------*/

/* PS_SavePlayers() -- Saves player data */
static void PS_SavePlayers(D_BS_t* const a_Str)
{
	int32_t i, j, k;
	player_t* This;
		
	/* Encode */
	// Base
	D_BSBaseBlock(a_Str, "PLAY");
	
	// Basic Info
	for (i = 0; i < MAXPLAYERS; i++)
	{
		D_BSwu8(a_Str, playeringame[i]);
		D_BSws(a_Str, player_names[i]);
		D_BSws(a_Str, team_names[i]);
	}
	
	// Record
	D_BSRecordBlock(a_Str);
	
	// Complex Info
	for (i = 0; i < MAXPLAYERS; i++)
	{
		// Not in game?
		if (i != 0 && !playeringame[i])
			continue;
		
		// Base
		D_BSBaseBlock(a_Str, "PINF");
		
		// Get Current
		This = &players[i];
		
		// Encode Data
		D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)This->mo));
		D_BSwi32(a_Str, This->playerstate);
		PS_LoadUnloadTicCmd(a_Str, true, &This->cmd);
		D_BSwi32(a_Str, This->viewz);
		D_BSwi32(a_Str, This->viewheight);
		D_BSwi32(a_Str, This->deltaviewheight);
		D_BSwi32(a_Str, This->bob);
		D_BSwi32(a_Str, This->FlatBob);
		D_BSwu32(a_Str, This->aiming);
		D_BSwi32(a_Str, This->health);
		D_BSwi32(a_Str, This->armorpoints);
		D_BSwu8(a_Str, This->armortype);
		D_BSwu8(a_Str, This->backpack);
		D_BSwu16(a_Str, This->addfrags);
		for (j = 0; j < MAXPLAYERS; j++)
			D_BSwu16(a_Str, This->frags[j]);
		D_BSwi32(a_Str, This->readyweapon);
		if (This->readyweapon >= 0)
			D_BSws(a_Str, wpnlev1info[This->readyweapon]->ClassName);
		else
			D_BSwu8(a_Str, 0);
		D_BSwi32(a_Str, This->pendingweapon);
		if (This->pendingweapon >= 0)
			D_BSws(a_Str, wpnlev1info[This->pendingweapon]->ClassName);
		else
			D_BSwu8(a_Str, 0);
		D_BSwi32(a_Str, This->DeadWeapon);
		if (This->DeadWeapon >= 0)
			D_BSws(a_Str, wpnlev1info[This->DeadWeapon]->ClassName);
		else
			D_BSwu8(a_Str, 0);
		D_BSwu32(a_Str, NUMWEAPONS);
		D_BSwu32(a_Str, NUMAMMO);
		for (j = 0; j < NUMWEAPONS; j++)
		{
			D_BSws(a_Str, wpnlev1info[This->FavoriteWeapons[j]]->ClassName);
			
			if (This->weaponowned[j])
				D_BSws(a_Str, wpnlev1info[j]->ClassName);
			else
				D_BSwu8(a_Str, 0);
		}
		for (j = 0; j < NUMAMMO; j++)
		{
			D_BSws(a_Str, ammoinfo[j]->ClassName);
			D_BSwi32(a_Str, This->ammo[j]);
			D_BSwi32(a_Str, This->maxammo[j]);
		}
		D_BSwu8(a_Str, This->originalweaponswitch);
		D_BSwu8(a_Str, This->autoaim_toggle);
		D_BSwu8(a_Str, This->attackdown);
		D_BSwu8(a_Str, This->usedown);
		D_BSwu8(a_Str, This->jumpdown);
		D_BSwi32(a_Str, This->cheats);
		D_BSwi32(a_Str, This->refire);
		D_BSwi32(a_Str, This->killcount);
		D_BSwi32(a_Str, This->itemcount);
		D_BSwi32(a_Str, This->secretcount);
		D_BSwi32(a_Str, This->damagecount);
		D_BSwi32(a_Str, This->bonuscount);
		D_BSwu8(a_Str, This->PalChoice);
		D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)This->attacker));
		D_BSwi32(a_Str, This->specialsector);
		D_BSwi32(a_Str, This->extralight);
		D_BSwi32(a_Str, This->fixedcolormap);
		D_BSwi32(a_Str, This->skincolor);
		D_BSwi32(a_Str, This->skin);
		D_BSwu32(a_Str, NUMPSPRITES);
		for (j = 0; j < NUMPSPRITES; j++)
			PS_LoadUnloadPSPDef(a_Str, true, &This->psprites[j]);
		D_BSwu8(a_Str, This->didsecret);
		D_BSwi32(a_Str, This->chickenTics);
		D_BSwi32(a_Str, This->chickenPeck);
		D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)This->rain1));
		D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)This->rain2));
		D_BSwi32(a_Str, This->flamecount);
		D_BSwi32(a_Str, This->flyheight);
		D_BSwu32(a_Str, NUMINVENTORYSLOTS);
		for (j = 0; j < NUMINVENTORYSLOTS; j++)
			PS_LoadUnloadInvenT(a_Str, true, &This->inventory[j]);
		D_BSwi32(a_Str, This->inventorySlotNum);
		D_BSwi32(a_Str, This->inv_ptr);
		D_BSwi32(a_Str, This->st_curpos);
		D_BSwi32(a_Str, This->st_inventoryTics);
		if (This->weaponinfo == wpnlev1info)
			D_BSwu8(a_Str, 1);
		else if (This->weaponinfo == wpnlev2info)
			D_BSwu8(a_Str, 2);
		else
			D_BSwu8(a_Str, 0);
		D_BSwi32(a_Str, This->flushdelay);
		D_BSwi32(a_Str, This->TargetViewZ);
		for (j = 0; j < 3; j++)
			D_BSwi32(a_Str, This->FakeMom[j]);
		PS_LoadUnloadCamera(a_Str, true, &This->camera);
		D_BSwi32(a_Str, This->CamDist);
		D_BSwi32(a_Str, This->CamHeight);
		D_BSwi32(a_Str, This->CamSpeed);
		D_BSwu8(a_Str, This->ChaseCam);
		D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)This->LastBFGBall));
		D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)This->Attackee));
		for (j = 0; j < 2; j++)
		{
			D_BSwi32(a_Str, This->MaxHealth[j]);
			D_BSwi32(a_Str, This->MaxArmor[j]);
		}
		D_BSwu8(a_Str, This->CounterOpPlayer);
		D_BSwi32(a_Str, This->TotalFrags);
		D_BSwi32(a_Str, This->TotalDeaths);
		D_BSwu32(a_Str, This->FraggerID);
		D_BSwcu64(a_Str, This->SuicideDelay);
		for (j = 0; j < 2; j++)
		{
			D_BSwu32(a_Str, This->KeyCards[j]);
			
			for (k = 0; k < 32; k++)
				D_BSwu8(a_Str, This->KeyFlash[j][k]);
		}
		
		// Record
		D_BSRecordBlock(a_Str);
	}
}

/* PS_LoadPlayers() -- Loads player data */
static bool_t PS_LoadPlayers(D_BS_t* const a_Str)
{
#define BUFSIZE 128
	int32_t i, j, n, mw, ma, a, b;
	player_t* This;
	pspdef_t pspjunk;
	inventory_t invenjunk;
	PI_wepid_t wi;
	PI_ammoid_t ai;
	char Buf[BUFSIZE];
	
	/* Expect "PLAY" */
	if (!PS_Expect(a_Str, "PLAY"))
		return false;
	
	/* Decode */
	// Basic Info
	for (i = 0; i < MAXPLAYERS; i++)
	{
		playeringame[i] = D_BSru8(a_Str);
		D_BSrs(a_Str, player_names[i], MAXPLAYERNAME);
		D_BSrs(a_Str, team_names[i], MAXPLAYERNAME * 2);
	}
	
	// Complex Info
	for (i = 0; i < MAXPLAYERS; i++)
	{
		// Not in game?
		if (i != 0 && !playeringame[i])
			continue;
		
		// Expect "PINF" for each individual player
		if (!PS_Expect(a_Str, "PINF"))
			return false;
		
		// Get Current
		This = &players[i];
		
		// Init player base
		G_ResetPlayer(This);
		
		// Decode Info
		This->mo = (void*)((intptr_t)D_BSri32(a_Str));
		This->playerstate = D_BSri32(a_Str);
		PS_LoadUnloadTicCmd(a_Str, false, &This->cmd);
		This->viewz = D_BSri32(a_Str);
		This->viewheight = D_BSri32(a_Str);
		This->deltaviewheight = D_BSri32(a_Str);
		This->bob = D_BSri32(a_Str);
		This->FlatBob = D_BSri32(a_Str);
		This->aiming = D_BSru32(a_Str);
		This->health = D_BSri32(a_Str);
		This->armorpoints = D_BSri32(a_Str);
		This->armortype = D_BSru8(a_Str);
		This->backpack = D_BSru8(a_Str);
		This->addfrags = D_BSru16(a_Str);
		for (j = 0; j < MAXPLAYERS; j++)
			This->frags[j] = D_BSru16(a_Str);
			
		// Ready Weapon
		This->readyweapon = D_BSri32(a_Str);
		D_BSrs(a_Str, Buf, BUFSIZE);
		wi = INFO_GetWeaponByName(Buf);
		if (wi < NUMWEAPONS)
			This->readyweapon = wi;
			
		// Pending weapon
		This->pendingweapon = D_BSri32(a_Str);
		D_BSrs(a_Str, Buf, BUFSIZE);
		wi = INFO_GetWeaponByName(Buf);
		if (This->pendingweapon < 0)
			This->pendingweapon = wp_nochange;
		else if (wi < NUMWEAPONS)
			This->pendingweapon = wi;
		
		// Dead weapon
		This->DeadWeapon = D_BSri32(a_Str);
		D_BSrs(a_Str, Buf, BUFSIZE);
		wi = INFO_GetWeaponByName(Buf);
		if (wi < NUMWEAPONS)
			This->DeadWeapon = wi;
		
		mw = D_BSru32(a_Str);
		ma = D_BSru32(a_Str);
		for (j = 0; j < mw; j++)
		{
			// Favorite Weapon
			D_BSrs(a_Str, Buf, BUFSIZE);
			wi = INFO_GetWeaponByName(Buf);
			if (wi < NUMWEAPONS)
				This->FavoriteWeapons[j] = wi;
			
			// Owned Weapon
			D_BSrs(a_Str, Buf, BUFSIZE);
			if (Buf[0])
			{
				wi = INFO_GetWeaponByName(Buf);
				if (wi >= 0 && wi < NUMWEAPONS)
					This->weaponowned[wi] = true;
			}
		}
		for (j = 0; j < ma; j++)
		{
			// Ammo ID and such
			D_BSrs(a_Str, Buf, BUFSIZE);
			ai = INFO_GetAmmoByName(Buf);
			a = D_BSri32(a_Str);
			b = D_BSri32(a_Str);
			
			if (ai < NUMAMMO)
			{
				This->ammo[ai] = a;
				This->maxammo[ai] = b;
			}
		}
		This->originalweaponswitch = D_BSru8(a_Str);
		This->autoaim_toggle = D_BSru8(a_Str);
		This->attackdown = D_BSru8(a_Str);
		This->usedown = D_BSru8(a_Str);
		This->jumpdown = D_BSru8(a_Str);
		This->cheats = D_BSri32(a_Str);
		This->refire = D_BSri32(a_Str);
		This->killcount = D_BSri32(a_Str);
		This->itemcount = D_BSri32(a_Str);
		This->secretcount = D_BSri32(a_Str);
		This->damagecount = D_BSri32(a_Str);
		This->bonuscount = D_BSri32(a_Str);
		This->PalChoice = D_BSru8(a_Str);
		This->attacker = (void*)((intptr_t)D_BSri32(a_Str));
		This->specialsector = D_BSri32(a_Str);
		This->extralight = D_BSri32(a_Str);
		This->fixedcolormap = D_BSri32(a_Str);
		This->skincolor = D_BSri32(a_Str);
		This->skin = D_BSri32(a_Str);
		n = D_BSru32(a_Str);
		for (j = 0; j < n; j++)
			PS_LoadUnloadPSPDef(a_Str, false, (j < NUMPSPRITES ? &This->psprites[j] : &pspjunk));
		This->didsecret = D_BSru8(a_Str);
		This->chickenTics = D_BSri32(a_Str);
		This->chickenPeck = D_BSri32(a_Str);
		This->rain1 = (void*)((intptr_t)D_BSri32(a_Str));
		This->rain2 = (void*)((intptr_t)D_BSri32(a_Str));
		This->flamecount = D_BSri32(a_Str);
		This->flyheight = D_BSri32(a_Str);
		n = D_BSru32(a_Str);
		for (j = 0; j < n; j++)
			PS_LoadUnloadInvenT(a_Str, false, (j < NUMINVENTORYSLOTS ? &This->inventory[j] : &invenjunk));
		This->inventorySlotNum = D_BSri32(a_Str);
		This->inv_ptr = D_BSri32(a_Str);
		This->st_curpos = D_BSri32(a_Str);
		This->st_inventoryTics = D_BSri32(a_Str);
		n = D_BSru8(a_Str);
		if (n == 2)
			This->weaponinfo = wpnlev2info;
		else
			This->weaponinfo = wpnlev1info;
		This->flushdelay = D_BSri32(a_Str);
		This->TargetViewZ = D_BSri32(a_Str);
		for (j = 0; j < 3; j++)
			This->FakeMom[j] = D_BSri32(a_Str);
		PS_LoadUnloadCamera(a_Str, false, &This->camera);
		This->CamDist = D_BSri32(a_Str);
		This->CamHeight = D_BSri32(a_Str);
		This->CamSpeed = D_BSri32(a_Str);
		This->ChaseCam = D_BSru8(a_Str);
		This->LastBFGBall = (void*)((intptr_t)D_BSri32(a_Str));
		This->Attackee = (void*)((intptr_t)D_BSri32(a_Str));
		for (j = 0; j < 2; j++)
		{
			This->MaxHealth[j] = D_BSri32(a_Str);
			This->MaxArmor[j] = D_BSri32(a_Str);
		}
		This->CounterOpPlayer = D_BSru8(a_Str);
		This->TotalFrags = D_BSri32(a_Str);
		This->TotalDeaths = D_BSri32(a_Str);
		This->FraggerID = D_BSru32(a_Str);
		This->SuicideDelay = D_BSrcu64(a_Str);
		for (j = 0; j < 2; j++)
		{
			This->KeyCards[j] = D_BSru32(a_Str);
			for (n = 0; n < 32; n++)
				This->KeyFlash[j][n] = D_BSru8(a_Str);
		}
	}
	
	/* Success! */
	return true;
#undef BUFSIZE
}

/*---------------------------------------------------------------------------*/

/* PS_SaveGameState() -- Saves game state */
static bool_t PS_SaveGameState(D_BS_t* const a_Str)
{
	int32_t i;
	P_XGSVariable_t* Var;
	
	/* Encode */
	// Base
	D_BSBaseBlock(a_Str, "GSTT");
	
	// Write
	D_BSwi32(a_Str, gamestate);
	D_BSwi32(a_Str, gameaction);
	D_BSwu64(a_Str, gametic);
	D_BSwu64(a_Str, leveltime);
	D_BSwu32(a_Str, g_CoreGame);
	D_BSwu32(a_Str, g_IWADFlags);
	D_BSwu8(a_Str, P_GetRandIndex());
	D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)g_LFPRover));
	
	// Record
	D_BSRecordBlock(a_Str);
	
	/* Write Vars */
	for (i = 0; i <= PEXGSNUMBITIDS; i++)
	{
		D_BSBaseBlock(a_Str, "GVAR");
		
		// End of list
		if (i == PEXGSNUMBITIDS)
			D_BSwu8(a_Str, 0);
		
		// Encode single variable
		else
		{
			Var = P_XGSVarForBit(i);
			
			// Illegal Var? Woops!
			if (!Var)
				D_BSwu8(a_Str, 2);
			
			// Write variable data
			else
			{
				D_BSwu8(a_Str, 1);
				D_BSwu32(a_Str, Var->BitID);
				D_BSwi32(a_Str, (Var->WasSet ? Var->ActualVal : Var->DefaultVal));
			}
		}
		
		D_BSRecordBlock(a_Str);
	}
	
	/* Success! */
	return true;
}

/* PS_LoadGameState() -- Loads the game state */
static bool_t PS_LoadGameState(D_BS_t* const a_Str)
{
	int32_t i, n;
	P_XGSVariable_t* Var;
	
	/* Expect "GSTT" */
	if (!PS_Expect(a_Str, "GSTT"))
		return false;
	
	/* Read */
	gamestate = D_BSri32(a_Str);
	gameaction = D_BSri32(a_Str);
	gametic = D_BSru64(a_Str);
	leveltime = D_BSru64(a_Str);
	g_CoreGame = D_BSru32(a_Str);
	g_IWADFlags = D_BSru32(a_Str);
	P_SetRandIndex(D_BSru8(a_Str));
	g_LFPRover = (intptr_t)D_BSri32(a_Str);
	
	/* Read Vars */
	for (;;)
	{
		// Expect GVAR
		if (!PS_Expect(a_Str, "GVAR"))
			return false;
		
		// Read Var Code
		i = D_BSru8(a_Str);
		
		// End of vars?
		if (i == 0)
			break;
		
		// Illegal?
		else if (i == 2)
			continue;
		
		// Standard var
		else
		{
			i = D_BSru32(a_Str);
			n = D_BSri32(a_Str);
			
			// Get var
			Var = P_XGSVarForBit(n);
			P_XGSRootSetValue(i, n);
		}
	}
	
	/* Success! */
	return true;
}

/*---------------------------------------------------------------------------*/

#define SECNODECOUNT 64
#define SECTORCOUNT 4
#define LINECOUNT 4
#define MAPTHINGCOUNT 16
#define BMAPCOUNT 256

/* PS_SaveMapState() -- Saves map to savegame */
static bool_t PS_SaveMapState(D_BS_t* const a_Str)
{
	uint32_t i, j;
	thinker_t* Thinker;
	sector_t* sect;
	line_t* line;
	
	mobj_t* mo;
	fireflicker_t* flicker;
	lightflash_t* lightflash;
	strobe_t* strobe;
	glow_t* glow;
	lightlevel_t* lightfade;
	vldoor_t* vldoor;
	floormove_t* floormove;
	pusher_t* pusher;
	friction_t* friction;
	ceiling_t* ceiling;
	plat_t* plat;
	elevator_t* elevator;
	scroll_t* scroll;
	
	ceilinglist_t* clist;
	platlist_t* plist;
	
	button_t* button;
	
	/* If not in a level, then do not continue */
	if (gamestate != GS_LEVEL && gamestate != GS_INTERMISSION)
		return true;
	
	/* Save the current map that is being played */
	D_BSBaseBlock(a_Str, "MLMP");
	
	// Write WAD and lump being played
	D_BSws(a_Str, WL_GetWADName(g_CurrentLevelInfo->WAD, false));
	D_BSws(a_Str, g_CurrentLevelInfo->LumpName);

	// Record
	D_BSRecordBlock(a_Str);
	
	/* Save Sector Nodes */
	// Save Count
	D_BSBaseBlock(a_Str, "MSCC");
	D_BSwi32(a_Str, g_NumMSecNodes);
	D_BSRecordBlock(a_Str);
	
	// Save actual sector nodes
	for (i = 0; i < g_NumMSecNodes; i++)
	{
		// New?
		if ((i & (SECNODECOUNT - 1)) == 0)
		{
			if (i > 0)
				D_BSRecordBlock(a_Str);
			D_BSBaseBlock(a_Str, "MSCN");
		}
		
		// Write Info here
		PS_LUMapObjRef(a_Str, true, (void**)&g_MSecNodes[i]->m_sector);
		D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)g_MSecNodes[i]->m_thing));
		D_BSwi32(a_Str, P_GetIDFromSecNode(g_MSecNodes[i]->m_tprev));
		D_BSwi32(a_Str, P_GetIDFromSecNode(g_MSecNodes[i]->m_tnext));
		D_BSwi32(a_Str, P_GetIDFromSecNode(g_MSecNodes[i]->m_sprev));
		D_BSwi32(a_Str, P_GetIDFromSecNode(g_MSecNodes[i]->m_snext));
		D_BSwu8(a_Str, g_MSecNodes[i]->visited);
	}
	
	// Record any remaining node blocks
	if (g_NumMSecNodes > 0)
		D_BSRecordBlock(a_Str);
		
	/* Save Ceiling List */
	D_BSBaseBlock(a_Str, "CEIL");
	
	j = 0;
	for (clist = activeceilings; clist; clist = clist->next)
		j++;
	D_BSwi32(a_Str, j);
	
	D_BSwi32(a_Str, PS_GetCeilingListID(activeceilings));
	for (clist = activeceilings; clist; clist = clist->next)
	{
		D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)clist->ceiling));
		D_BSwi32(a_Str, PS_GetCeilingListID(clist->next));
		
		if (clist->prev == &activeceilings)
			D_BSwi32(a_Str, -2);
		else
			D_BSwi32(a_Str, PS_GetCeilingListID((ceilinglist_t*)(((intptr_t)(*clist->prev)) - offsetof(ceilinglist_t, next))));
	}
	
	D_BSwi32(a_Str, -3);
	
	D_BSRecordBlock(a_Str);
	
	/* Save Plat List */
	D_BSBaseBlock(a_Str, "PLAT");
	
	j = 0;
	for (plist = activeplats; plist; plist = plist->next)
		j++;
	D_BSwi32(a_Str, j);
	
	D_BSwi32(a_Str, PS_GetPlatListID(activeplats));
	for (plist = activeplats; plist; plist = plist->next)
	{
		D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)plist->plat));
		D_BSwi32(a_Str, PS_GetPlatListID(plist->next));
		
		if (plist->prev == &activeplats)
			D_BSwi32(a_Str, -2);
		else
			D_BSwi32(a_Str, PS_GetPlatListID((platlist_t*)(((intptr_t)(*plist->prev)) - offsetof(platlist_t, next))));
	}
	
	D_BSwi32(a_Str, -3);
	
	D_BSRecordBlock(a_Str);
	
	/* Save Thinkers */
	for (Thinker = thinkercap.next; Thinker != &thinkercap; Thinker = Thinker->next)
	{
		// Base thinker
		D_BSBaseBlock(a_Str, "THNK");
		
		// Write thinker type/function
		D_BSwi8(a_Str, Thinker->Type);
		D_BSwi8(a_Str, G_ThinkFuncToType(Thinker->function));
		
		// Encode specific data
		switch (Thinker->Type)
		{
				// Map Object
			case PTT_MOBJ:
				mo = (void*)Thinker;
				
				PS_LoadUnloadNoiseThinker(a_Str, true, &mo->NoiseThinker);
				D_BSwi32(a_Str, mo->x);
				D_BSwi32(a_Str, mo->y);
				D_BSwi32(a_Str, mo->z);
				D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)mo->snext));
				D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)mo->sprev));
				D_BSwu32(a_Str, mo->angle);
				D_BSwi32(a_Str, mo->sprite);
				D_BSwi32(a_Str, mo->frame);
				D_BSwi32(a_Str, mo->skin);
				D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)mo->bnext));
				D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)mo->bprev));
				PS_LUMapObjRef(a_Str, true, (void**)&mo->subsector);
				D_BSwi32(a_Str, mo->floorz);
				D_BSwi32(a_Str, mo->ceilingz);
				D_BSwi32(a_Str, mo->radius);
				D_BSwi32(a_Str, mo->height);
				D_BSwi32(a_Str, mo->momx);
				D_BSwi32(a_Str, mo->momy);
				D_BSwi32(a_Str, mo->momz);
				D_BSws(a_Str, mo->info->RClassName);
				D_BSwi32(a_Str, mo->tics);
				PS_StateP(a_Str, true, &mo->state);
				D_BSwi32(a_Str, mo->flags);
				D_BSwi32(a_Str, mo->eflags);
				D_BSwi32(a_Str, mo->flags2);
				D_BSwi32(a_Str, mo->special1);
				D_BSwi32(a_Str, mo->special2);
				D_BSwi32(a_Str, mo->health);
				D_BSwi32(a_Str, mo->movedir);
				D_BSwi32(a_Str, mo->movecount);
				D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)mo->target));
				D_BSwi32(a_Str, mo->reactiontime);
				D_BSwi32(a_Str, mo->threshold);
				if (mo->player)
					D_BSwi32(a_Str, mo->player - players);
				else
					D_BSwi32(a_Str, -1);
				D_BSwi32(a_Str, mo->lastlook);
				
				// Spawn point (might be script created)
				PS_LUMapObjRef(a_Str, true, (void**)&mo->spawnpoint);
				D_BSwu8(a_Str, !!mo->spawnpoint);
				if (mo->spawnpoint)
				{
					D_BSwi16(a_Str, mo->spawnpoint->x);
					D_BSwi16(a_Str, mo->spawnpoint->y);
					D_BSwi16(a_Str, mo->spawnpoint->z);
					D_BSwi16(a_Str, mo->spawnpoint->angle);
					D_BSwi16(a_Str, mo->spawnpoint->type);
					D_BSwi16(a_Str, mo->spawnpoint->options);
					D_BSwu8(a_Str, mo->spawnpoint->IsHexen);
					D_BSwi16(a_Str, mo->spawnpoint->HeightOffset);
					D_BSwu16(a_Str, mo->spawnpoint->ID);
					D_BSwu8(a_Str, mo->spawnpoint->Special);
					for (i = 0; i < 5; i++)
						D_BSwu8(a_Str, mo->spawnpoint->Args[i]);
					if (mo->spawnpoint->MoType >= 0 && mo->spawnpoint->MoType < NUMMOBJTYPES)
						D_BSws(a_Str, mobjinfo[mo->spawnpoint->MoType]->RClassName);
					else
						D_BSwu8(a_Str, 0);
					D_BSwu8(a_Str, mo->spawnpoint->MarkedWeapon);
					D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)mo->spawnpoint->mobj));
				}
				
				D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)mo->tracer));
				D_BSwi32(a_Str, mo->friction);
				D_BSwi32(a_Str, mo->movefactor);
				D_BSwi32(a_Str, P_GetIDFromSecNode(mo->touching_sectorlist));
				D_BSwi32(a_Str, mo->dropped_ammo_count);
				
				for (i = 0; i < NUMINFORXFIELDS; i++)
					D_BSwu32(a_Str, mo->RXFlags[i]);
				
				if (mo->RXShotWithWeapon >= 0 && mo->RXShotWithWeapon < NUMWEAPONS)
					D_BSws(a_Str, wpnlev1info[mo->RXShotWithWeapon]->ClassName);
				else
					D_BSwu8(a_Str, 0);
				D_BSwi32(a_Str, mo->RXAttackAttackType);
				D_BSwu8(a_Str, mo->RemoveMo);
				D_BSws(a_Str, mobjinfo[mo->RemType]->RClassName);
				D_BSwi32(a_Str, mo->MaxZObtained);
				D_BSwu32(a_Str, mo->SpawnOrder);
				D_BSwi32(a_Str, mo->SkinTeamColor);
				D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)mo->FollowPlayer));
				
				for (i = 0; i < 2; i++)
				{
					D_BSwu32(a_Str, mo->TimeThinking[i]);
					D_BSwu32(a_Str, mo->TimeFromDead[i]);
				};
				
				D_BSwi32(a_Str, mo->KillerPlayer);
				D_BSwu32(a_Str, mo->FraggerID);
				
				for (i = 0; i < 3; i++)
					D_BSwi32(a_Str, mo->DrawPos[i]);
				
				// Object on object count
				for (i = 0; i < 2; i++)
				{
					D_BSwu32(a_Str, mo->MoOnCount[i]);
					
					for (j = 0; j < mo->MoOnCount[i]; j++)
						D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)mo->MoOn[i][j]));
				}
				
				// References
				for (i = 0; i < NUMPMOBJREFTYPES; i++)
				{
					D_BSwu32(a_Str, mo->RefCount[i]);
					D_BSwu32(a_Str, mo->RefListSz[i]);
					
					for (j = 0; j < mo->RefListSz[i]; j++)
						D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)mo->RefList[i][j]));
				}
				
				// CTF Stuff
				D_BSwi32(a_Str, mo->FakeColor);
				D_BSwu8(a_Str, mo->CTFTeam);
				D_BSwu32(a_Str, P_TouchFuncToID(mo->AltTouchFunc));
				break;

				// Vertical Door
			case PTT_VERTICALDOOR:
				vldoor = (void*)Thinker;
				
				D_BSwi32(a_Str, vldoor->type);
				PS_LUMapObjRef(a_Str, true, (void**)&vldoor->sector);
				D_BSwi32(a_Str, vldoor->topheight);
				D_BSwi32(a_Str, vldoor->speed);
				D_BSwi32(a_Str, vldoor->direction);
				D_BSwi32(a_Str, vldoor->topwait);
				D_BSwi32(a_Str, vldoor->topcountdown);
				PS_LUMapObjRef(a_Str, true, (void**)&vldoor->line);
				break;
			
				// Light Source
			case PTT_FIREFLICKER:
				flicker = (void*)Thinker;
				
				PS_LUMapObjRef(a_Str, true, (void**)&flicker->sector);
				D_BSwi32(a_Str, flicker->count);
				D_BSwi32(a_Str, flicker->maxlight);
				D_BSwi32(a_Str, flicker->minlight);
				break;
	
				// Light Source
			case PTT_LIGHTFLASH:
				lightflash = (void*)Thinker;
				
				PS_LUMapObjRef(a_Str, true, (void**)&lightflash->sector);
				D_BSwi32(a_Str, lightflash->count);
				D_BSwi32(a_Str, lightflash->maxlight);
				D_BSwi32(a_Str, lightflash->minlight);
				D_BSwi32(a_Str, lightflash->maxtime);
				D_BSwi32(a_Str, lightflash->mintime);
				break;
	
				// Light Source
			case PTT_STROBEFLASH:
				strobe = (void*)Thinker;
				
				PS_LUMapObjRef(a_Str, true, (void**)&strobe->sector);
				D_BSwi32(a_Str, strobe->count);
				D_BSwi32(a_Str, strobe->maxlight);
				D_BSwi32(a_Str, strobe->minlight);
				D_BSwi32(a_Str, strobe->darktime);
				D_BSwi32(a_Str, strobe->brighttime);
				break;
	
				// Light Source
			case PTT_GLOW:
				glow = (void*)Thinker;
				
				PS_LUMapObjRef(a_Str, true, (void**)&glow->sector);
				D_BSwi32(a_Str, glow->maxlight);
				D_BSwi32(a_Str, glow->minlight);
				D_BSwi32(a_Str, glow->direction);
				break;
	
				// Light Source
			case PTT_LIGHTFADE:
				lightfade = (void*)Thinker;
				
				PS_LUMapObjRef(a_Str, true, (void**)&lightfade->sector);
				D_BSwi32(a_Str, lightfade->destlevel);
				D_BSwi32(a_Str, lightfade->speed);
				break;
	
				// Moving Surface
			case PTT_MOVEFLOOR:
				floormove = (void*)Thinker;
				
				D_BSwi32(a_Str, floormove->type);
				D_BSwu32(a_Str, floormove->crush);
				PS_LUMapObjRef(a_Str, true, (void**)&floormove->sector);
				D_BSwi32(a_Str, floormove->direction);
				D_BSwu32(a_Str, floormove->newspecial);
				D_BSwu32(a_Str, floormove->oldspecial);
				D_BSwi32(a_Str, floormove->texture);
				D_BSwi32(a_Str, floormove->floordestheight);
				D_BSwi32(a_Str, floormove->speed);
				break;
	
				// Moving Surface
			case PTT_MOVECEILING:
				ceiling = (void*)Thinker;
				
				D_BSwi32(a_Str, ceiling->type);
				PS_LUMapObjRef(a_Str, true, (void**)&ceiling->sector);
				D_BSwi32(a_Str, ceiling->bottomheight);
				D_BSwi32(a_Str, ceiling->topheight);
				D_BSwi32(a_Str, ceiling->speed);
				D_BSwi32(a_Str, ceiling->oldspeed);
				D_BSwu8(a_Str, ceiling->crush);
				D_BSwu32(a_Str, ceiling->newspecial);
				D_BSwu32(a_Str, ceiling->oldspecial);
				D_BSwi32(a_Str, ceiling->texture);
				D_BSwi32(a_Str, ceiling->direction);
				D_BSwi32(a_Str, ceiling->tag);
				D_BSwi32(a_Str, ceiling->olddirection);
				D_BSwi32(a_Str, PS_GetCeilingListID(ceiling->list));
				break;
	
				// Moving Surface
			case PTT_PLATRAISE:
				plat = (plat_t*)Thinker;
				
				PS_LUMapObjRef(a_Str, true, (void**)&plat->sector);
				D_BSwi32(a_Str, plat->speed);
				D_BSwi32(a_Str, plat->low);
				D_BSwi32(a_Str, plat->high);
				D_BSwi32(a_Str, plat->wait);
				D_BSwi32(a_Str, plat->count);
				D_BSwi32(a_Str, plat->status);
				D_BSwi32(a_Str, plat->oldstatus);
				D_BSwu8(a_Str, plat->crush);
				D_BSwi32(a_Str, plat->tag);
				D_BSwi32(a_Str, plat->type);
				D_BSwi32(a_Str, PS_GetPlatListID(plat->list));
				break;
	
				// Moving Surface
			case PTT_MOVEELEVATOR:
				elevator = (elevator_t*)Thinker;
				
				D_BSwu32(a_Str, elevator->type);
				PS_LUMapObjRef(a_Str, true, (void**)&elevator->sector);
				D_BSwi32(a_Str, elevator->direction);
				D_BSwi32(a_Str, elevator->floordestheight);
				D_BSwi32(a_Str, elevator->ceilingdestheight);
				D_BSwi32(a_Str, elevator->speed);
				D_BSwu8(a_Str, elevator->Silent);
				D_BSwi32(a_Str, elevator->PerpWait);
				D_BSwi32(a_Str, elevator->PerpTicsLeft);
				PS_LUMapObjRef(a_Str, true, (void**)&elevator->CallLine);
				D_BSwi32(a_Str, elevator->PDoorSpeed);
				D_BSwi32(a_Str, elevator->OldDirection);
				D_BSwu8(a_Str, elevator->Dinged);
				break;
	
				// Scrolling Line
			case PTT_SCROLL:
				scroll = (scroll_t*)Thinker;
				
				scroll->dx = D_BSri32(a_Str);
				scroll->dy = D_BSri32(a_Str);
				scroll->affectee = D_BSri32(a_Str);
				scroll->control = D_BSri32(a_Str);
				scroll->last_height = D_BSri32(a_Str);
				scroll->vdx = D_BSri32(a_Str);
				scroll->vdy = D_BSri32(a_Str);
				scroll->accel = D_BSri32(a_Str);
				scroll->type = D_BSri32(a_Str);
				break;
	
				// Friction
			case PTT_FRICTION:
				friction = (void*)Thinker;
				
				D_BSwi32(a_Str, friction->friction);
				D_BSwi32(a_Str, friction->movefactor);
				D_BSwi32(a_Str, friction->affectee);
				break;
	
				// Pusher/Puller
			case PTT_PUSHER:
				pusher = (void*)Thinker;
				
				D_BSwu32(a_Str, pusher->type);
				D_BSwi32(a_Str, PS_GetThinkerID(pusher->source));
				D_BSwi32(a_Str, pusher->x_mag);
				D_BSwi32(a_Str, pusher->y_mag);
				D_BSwi32(a_Str, pusher->magnitude);
				D_BSwi32(a_Str, pusher->radius);
				D_BSwi32(a_Str, pusher->x);
				D_BSwi32(a_Str, pusher->y);
				D_BSwi32(a_Str, pusher->affectee);
				break;
	
				// Unknown
			default:
				break;
		}
		
		// Record
		D_BSRecordBlock(a_Str);
	}
	
	// Write End of Thinkers
	D_BSBaseBlock(a_Str, "THNK");
	D_BSwi8(a_Str, -1);
	D_BSRecordBlock(a_Str);
	
	/* Save BlockLinks */
	// Write links
	for (i = 0; i < (bmapwidth * bmapheight); i++)
	{
		// New?
		if ((i & (BMAPCOUNT - 1)) == 0)
		{
			if (i > 0)
				D_BSRecordBlock(a_Str);
			D_BSBaseBlock(a_Str, "BKLN");
			
			if (i == 0)
				D_BSwu32(a_Str, (bmapwidth * bmapheight));
		}
		
		D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)blocklinks[i]));
	}
	
	// Record
	if (i > 0)
		D_BSRecordBlock(a_Str);
	
	/* Save FFloors */
	// TODO
	
	/* Save Line Data */
	for (i = 0 ; i < numlines; i++)
	{
		line = &lines[i];
		
		// New?
		if ((i & (LINECOUNT - 1)) == 0)
		{
			if (i > 0)
				D_BSRecordBlock(a_Str);
			D_BSBaseBlock(a_Str, "LINE");
		}
		
		// Dump Info
	}
	
	if (numlines > 0)
		D_BSRecordBlock(a_Str);
	
	/* Save Sector Data */
	for (i = 0 ; i < numsectors; i++)
	{
		sect = &sectors[i];
		
		// New?
		if ((i & (SECTORCOUNT - 1)) == 0)
		{
			if (i > 0)
				D_BSRecordBlock(a_Str);
			D_BSBaseBlock(a_Str, "SECT");
		}
		
		// Dump Info
		D_BSwi32(a_Str, sect->floorheight);
		D_BSwi32(a_Str, sect->ceilingheight);
		D_BSwi16(a_Str, sect->floorpic);
		D_BSwi16(a_Str, sect->ceilingpic);
		D_BSwi16(a_Str, sect->lightlevel);
		D_BSwu32(a_Str, sect->special);
		D_BSwu32(a_Str, sect->oldspecial);
		D_BSwi16(a_Str, sect->tag);
		D_BSwi32(a_Str, sect->nexttag);
		D_BSwi32(a_Str, sect->firsttag);
		D_BSwi16(a_Str, sect->soundtraversed);
		D_BSwi16(a_Str, sect->floortype);
		D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)sect->soundtarget));
		
		for (j = 0; j < 4; j++)
			D_BSwi32(a_Str, sect->blockbox[j]);
		
		PS_LoadUnloadNoiseThinker(a_Str, true, &sect->soundorg);
		
		D_BSwi32(a_Str, sect->validcount);
		D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)sect->thinglist));
		D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)sect->floordata));
		D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)sect->ceilingdata));
		D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)sect->lightingdata));
		
		D_BSwi32(a_Str, sect->stairlock);
		D_BSwi32(a_Str, sect->prevsec);
		D_BSwi32(a_Str, sect->nextsec);
		D_BSwi32(a_Str, sect->floor_xoffs);
		D_BSwi32(a_Str, sect->floor_yoffs);
		D_BSwi32(a_Str, sect->ceiling_xoffs);
		D_BSwi32(a_Str, sect->ceiling_yoffs);
		D_BSwi32(a_Str, sect->heightsec);
		D_BSwi32(a_Str, sect->altheightsec);
		D_BSwi32(a_Str, sect->floorlightsec);
		D_BSwi32(a_Str, sect->ceilinglightsec);
		D_BSwi32(a_Str, sect->teamstartsec);
		D_BSwi32(a_Str, sect->bottommap);
		D_BSwi32(a_Str, sect->midmap);
		D_BSwi32(a_Str, sect->topmap);
		D_BSwi32(a_Str, P_GetIDFromSecNode(sect->touching_thinglist));
		
		D_BSwi32(a_Str, P_GetIDFromFFloor(sect->ffloors));
		
		D_BSwu32(a_Str, sect->linecount);
		for (j = 0; j < sect->linecount; j++)
			PS_LUMapObjRef(a_Str, true, (void**)&sect->lines[j]);
		
		D_BSwu32(a_Str, sect->numattached);
		for (j = 0; j < sect->numattached; j++)
			D_BSwi32(a_Str, sect->attached[j]);
		
		D_BSwu8(a_Str, sect->LLSelf);
		// TODO: Light Lists
		// lightlist_t* lightlist;
		// int32_t numlights;
		
		D_BSwu8(a_Str, sect->moved);
		D_BSwi32(a_Str, sect->validsort);
		D_BSwu8(a_Str, sect->added);
		
		// TODO: Colormap
		// extracolormap_t* extra_colormap;
		
		D_BSws(a_Str, sect->FloorTexture);
		D_BSws(a_Str, sect->CeilingTexture);
		
		for (j = 0; j < 4; j++)
			D_BSwi32(a_Str, sect->BBox[j]);
		
		D_BSwu32(a_Str, sect->SoundSecRef);
		D_BSwi32(a_Str, sect->AltSkyTexture);
		D_BSwu8(a_Str, sect->AltSkyFlipped);
		
		D_BSwu32(a_Str, sect->NumAdj);
		for (j = 0; j < sect->NumAdj; j++)
			PS_LUMapObjRef(a_Str, true, (void**)&sect->Adj[j]);
	}
	
	if (numsectors > 0)
		D_BSRecordBlock(a_Str);
	
	/* Save Map Thing References */
	// Write it all
	for (i = 0; i < nummapthings; i++)
	{
		if ((i & (MAPTHINGCOUNT - 1)) == 0)
		{
			if (i > 0)
				D_BSRecordBlock(a_Str);
			D_BSBaseBlock(a_Str, "MTRF");
		}
		
		D_BSwu8(a_Str, mapthings[i].MarkedWeapon);
		if (mapthings[i].MoType >= 0 && mapthings[i].MoType < NUMMOBJTYPES)
			D_BSws(a_Str, mobjinfo[mapthings[i].MoType]->RClassName);
		else
			D_BSwu8(a_Str, 0);
		D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)mapthings[i].mobj));
	}
	
	// Record
	if (nummapthings > 0)
		D_BSRecordBlock(a_Str);
	
	/* Save Respawn Queue */
	D_BSBaseBlock(a_Str, "IQUE");
	
	// Save entire queue
	D_BSwi32(a_Str, ITEMQUESIZE);
	D_BSwi32(a_Str, iquehead);
	D_BSwi32(a_Str, iquetail);
	
	for (i = 0; i < ITEMQUESIZE; i++)
	{
		D_BSwu8(a_Str, !!itemrespawnque[i]);
		PS_LUMapObjRef(a_Str, true, (void**)&itemrespawnque[i]);
		D_BSwcu64(a_Str, itemrespawntime[i]);
		
		// In case of script spawned things
		if (itemrespawnque[i])
		{
			D_BSwi16(a_Str, itemrespawnque[i]->x);
			D_BSwi16(a_Str, itemrespawnque[i]->y);
			D_BSwi16(a_Str, itemrespawnque[i]->z);
			D_BSwi16(a_Str, itemrespawnque[i]->angle);
			D_BSwi16(a_Str, itemrespawnque[i]->type);
			D_BSwi16(a_Str, itemrespawnque[i]->options);
			D_BSwu8(a_Str, itemrespawnque[i]->IsHexen);
			D_BSwi16(a_Str, itemrespawnque[i]->HeightOffset);
			D_BSwu16(a_Str, itemrespawnque[i]->ID);
			D_BSwu8(a_Str, itemrespawnque[i]->Special);
			for (j = 0; j < 5; j++)
				D_BSwu8(a_Str, itemrespawnque[i]->Args[j]);
			if (itemrespawnque[i]->MoType >= 0 && itemrespawnque[i]->MoType < NUMMOBJTYPES)
				D_BSws(a_Str, mobjinfo[itemrespawnque[i]->MoType]->RClassName);
			else
				D_BSwu8(a_Str, 0);
			D_BSwu8(a_Str, itemrespawnque[i]->MarkedWeapon);
			D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)itemrespawnque[i]->mobj));
		}
	}
	
	// Record
	D_BSRecordBlock(a_Str);
	
	/* Save buttons */
	D_BSBaseBlock(a_Str, "BUTN");
	
	for (i = 0; i < MAXBUTTONS; i++)
	{
		button = &buttonlist[i];
		
		PS_LUMapObjRef(a_Str, true, (void**)&button->line);
		D_BSwi32(a_Str, button->where);
		D_BSwi32(a_Str, button->btexture);
		D_BSwi32(a_Str, button->btimer);
		
		if (!button->soundorg)
			D_BSwi32(a_Str, -1);
		else
			D_BSwi32(a_Str, ((sector_t*)(((intptr_t)button->soundorg) - offsetof(sector_t, soundorg))) - sectors);
	}
	
	// Record
	D_BSRecordBlock(a_Str);
	
	/* Save CTF Flag Info */
	D_BSBaseBlock(a_Str, "CTFF");
	
	// Dump Flags
	D_BSwu8(a_Str, MAXSKINCOLORS);
	for (i = 0; i < MAXSKINCOLORS + 1; i++)
		D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)g_CTFFlags[i]));
	
	// Record
	D_BSRecordBlock(a_Str);
	
	/* Save Other related variables */
	D_BSBaseBlock(a_Str, "LMSC");
	
	// Body Queue
	D_BSwu8(a_Str, BODYQUESIZE);
	D_BSwi32(a_Str, bodyqueslot);
	for (i = 0; i < BODYQUESIZE; i++)
		D_BSwi32(a_Str, PS_GetThinkerID((thinker_t*)bodyque[i]));
	
	// Coop Starts
	D_BSwu8(a_Str, MAXPLAYERS);
	for (i = 0; i < MAXPLAYERS; i++)
		if (playerstarts[i])
			D_BSwi32(a_Str, playerstarts[i] - mapthings);
		else
			D_BSwi32(a_Str, -1);
	
	// DM Starts
	D_BSwi32(a_Str, numdmstarts);
	for (i = 0; i < numdmstarts; i++)
		if (i < MAX_DM_STARTS && deathmatchstarts[i])
			D_BSwi32(a_Str, deathmatchstarts[i] - mapthings);
		else
			D_BSwi32(a_Str, -1);
	
	// Team Starts
	D_BSwu8(a_Str, MAXSKINCOLORS);
	D_BSwu8(a_Str, MAXPLAYERS);
	for (i = 0; i < MAXSKINCOLORS; i++)
		for (j = 0; j < MAXPLAYERS; j++)
			if (g_TeamStarts[i][j])
				D_BSwi32(a_Str, g_TeamStarts[i][j] - mapthings);
			else
				D_BSwi32(a_Str, -1);
	
	// Map Totals
	for (i = 0; i < 5; i++)
		D_BSwi32(a_Str, g_MapKIS[i]);
	D_BSwi32(a_Str, totalkills);
	D_BSwi32(a_Str, totalitems);
	D_BSwi32(a_Str, totalsecret);
	
	// Record
	D_BSRecordBlock(a_Str);
	
	/* Success! */
	return true;	
}

/* PS_LoadMapState() -- Loads map from savegame */
static bool_t PS_LoadMapState(D_BS_t* const a_Str)
{
#define BUFSIZE 128
	char Buf[BUFSIZE];
	P_LevelInfoEx_t* pli;
	int32_t i, j, k, n, x;
	thinker_t* Thinker;
	sector_t* sect;
	line_t* line;
	tic_t Tic;
	
	mobj_t* mo;
	fireflicker_t* flicker;
	lightflash_t* lightflash;
	strobe_t* strobe;
	glow_t* glow;
	lightlevel_t* lightfade;
	vldoor_t* vldoor;
	floormove_t* floormove;
	pusher_t* pusher;
	friction_t* friction;
	ceiling_t* ceiling;
	plat_t* plat;
	elevator_t* elevator;
	scroll_t* scroll;
	
	ceilinglist_t* clist, *rootclist, *nextclist, *clistrover;
	platlist_t* plist, *rootplist, *nextplist, *plistrover;
	
	button_t* button;
	
	/* If not in a level, then do not continue */
	if (gamestate != GS_LEVEL && gamestate != GS_INTERMISSION)
		return true;
	
	/* Expect "MLMP" */
	if (!PS_Expect(a_Str, "MLMP"))
		return false;
	
	// Ignore WAD Name
	D_BSrs(a_Str, Buf, BUFSIZE);
	
	// Attempt to locate level being played
	D_BSrs(a_Str, Buf, BUFSIZE);
	pli = P_FindLevelByNameEx(Buf, NULL);
	
	// If not found, then fail
	if (!pli)
		return PS_IllegalSave(DSTR_PSAVEGC_UNKNOWNLEVEL);
	
	// Load level then
	if (!P_ExLoadLevel(pli, PEXLL_FROMSAVE))
		return PS_IllegalSave(DSTR_PSAVEGC_LEVELLOADFAIL);
	
	/* Load Sector Nodes */
	// Load Count
	if (!PS_Expect(a_Str, "MSCC"))
		return false;
	
	g_NumMSecNodes = D_BSri32(a_Str);
	
	// Allocate Buffers
	g_MSecNodes = Z_Malloc(sizeof(*g_MSecNodes) * g_NumMSecNodes, PU_LEVEL, NULL);
	for (i = 0; i < g_NumMSecNodes; i++)
		g_MSecNodes[i] = Z_Malloc(sizeof(*g_MSecNodes[i]), PU_LEVEL, NULL);
	
	// Load individual nodes
	for (i = 0; i < g_NumMSecNodes; i++)
	{
		// Load new data in
		if ((i & (SECNODECOUNT - 1)) == 0)
			if (!PS_Expect(a_Str, "MSCN"))
				return false;
		
		// Load Data
		PS_LUMapObjRef(a_Str, false, (void**)&g_MSecNodes[i]->m_sector);
		g_MSecNodes[i]->m_thing = (void*)((intptr_t)D_BSri32(a_Str));
		g_MSecNodes[i]->m_tprev = P_GetSecNodeFromID(D_BSri32(a_Str));
		g_MSecNodes[i]->m_tnext = P_GetSecNodeFromID(D_BSri32(a_Str));
		g_MSecNodes[i]->m_sprev = P_GetSecNodeFromID(D_BSri32(a_Str));
		g_MSecNodes[i]->m_snext = P_GetSecNodeFromID(D_BSri32(a_Str));
		g_MSecNodes[i]->visited = D_BSru8(a_Str);
	}
	
	/* Load Ceiling List */
	if (!PS_Expect(a_Str, "CEIL"))
		return false;
	
	// Clear ceilings
	activeceilings = NULL;
	rootclist = NULL;
	
	// Re-allocate basic ceiling structure
	j = D_BSri32(a_Str);
	
	for (clist = NULL, i = 0; i < j; i++)
	{
		if (!clist)
			rootclist = clist = Z_Malloc(sizeof(*clist), PU_STATIC, NULL);
		else
		{
			clist->SaveLink = Z_Malloc(sizeof(*clist), PU_STATIC, NULL);
			clist = clist->SaveLink;
		}
	}
	
	// Find activeceilings list
	x = D_BSri32(a_Str);
	for (i = 1, clist = rootclist; clist; clist = clist->SaveLink, i++)
		if (i == x)
			break;
	
	// Active ceiling is this list
	activeceilings = clist;
	
	// Read consecutive list
	for (clist = rootclist; clist; clist = nextclist)
	{
		// Next may get kludged
		nextclist = clist->SaveLink;
		
		// Read, end if -3, otherwise a thinker
		x = D_BSri32(a_Str);
		if (x == -3)
			break;	// End
		
		// Thinker
		clist->ceiling = (ceiling_t*)x;
		
		// Reref real next
		x = D_BSri32(a_Str);
		for (i = 1, clistrover = rootclist; clistrover; clistrover = clistrover->SaveLink, i++)
			if (i == x)
				break;
		
		clist->next = clistrover;
		
		// This is a pointer of a pointer
		x = D_BSri32(a_Str);
		if (x == -2)
			clist->prev = &activeceilings;
		else
		{
			for (i = 1, clistrover = rootclist; clistrover; clistrover = clistrover->SaveLink, i++)
				if (i == x)
					break;
		
			if (clistrover)	// there might not be a next
				clist->prev = &clistrover->next;
		}
	}
	
	/* Load Plat List */
	if (!PS_Expect(a_Str, "PLAT"))
		return false;
	
	// Clear plats
	activeplats = NULL;
	rootplist = NULL;
	
	// Re-allocate basic plat structure
	j = D_BSri32(a_Str);
	
	for (plist = NULL, i = 0; i < j; i++)
	{
		if (!plist)
			rootplist = plist = Z_Malloc(sizeof(*plist), PU_STATIC, NULL);
		else
		{
			plist->SaveLink = Z_Malloc(sizeof(*plist), PU_STATIC, NULL);
			plist = plist->SaveLink;
		}
	}
	
	// Find activeplats list
	x = D_BSri32(a_Str);
	for (i = 1, plist = rootplist; plist; plist = plist->SaveLink, i++)
		if (i == x)
			break;
	
	// Active plat is this list
	activeplats = plist;
	
	// Read consecutive list
	for (plist = rootplist; plist; plist = nextplist)
	{
		// Next may get kludged
		nextplist = plist->SaveLink;
		
		// Read, end if -3, otherwise a thinker
		x = D_BSri32(a_Str);
		if (x == -3)
			break;	// End
		
		// Thinker
		plist->plat = (plat_t*)x;
		
		// Reref real next
		x = D_BSri32(a_Str);
		for (i = 1, plistrover = rootplist; plistrover; plistrover = plistrover->SaveLink, i++)
			if (i == x)
				break;
		
		plist->next = plistrover;
		
		// This is a pointer of a pointer
		x = D_BSri32(a_Str);
		if (x == -2)
			plist->prev = &activeplats;
		else
		{
			for (i = 1, plistrover = rootplist; plistrover; plistrover = plistrover->SaveLink, i++)
				if (i == x)
					break;
		
			if (plistrover)	// there might not be a next
				plist->prev = &plistrover->next;
		}
	}
	
	/* Load Thinkers */
	for (;;)
	{
		// Expect Thinker
		if (!PS_Expect(a_Str, "THNK"))
			return false;
		
		// Read Type
		i = D_BSri8(a_Str);
		
		// End of thinkers?
		if (i < 0)
			break;
		
		x = D_BSri8(a_Str);
		
		// Illegal Type?
		if (i >= NUMPTHINKERTYPES || x < 0 || x >= NUMPTHINKERTYPES)
			return PS_IllegalSave(DSTR_PSAVEGC_ILLEGALTHINKER);
		
		// Allocate New Thinker
		Thinker = Z_Malloc(g_ThinkerData[i].Size, PU_LEVEL, NULL);
		Thinker->function = G_ThinkTypeToFunc(x);
		P_AddThinker(Thinker, i);
		
		// Load data based on thinker type
		switch (x)
		{
				// Junk
			case PTT_CAP:
			case PTT_DEFUNCT:
			case PTT_DELETEME:
				break;
				
				// Map Object
			case PTT_MOBJ:
				mo = (void*)Thinker;
				
				PS_LoadUnloadNoiseThinker(a_Str, false, &mo->NoiseThinker);
				mo->x = D_BSri32(a_Str);
				mo->y = D_BSri32(a_Str);
				mo->z = D_BSri32(a_Str);
				mo->snext = (void*)((intptr_t)D_BSri32(a_Str));
				mo->sprev = (void*)((intptr_t)D_BSri32(a_Str));
				mo->angle = D_BSru32(a_Str);
				mo->sprite = D_BSri32(a_Str);
				mo->frame = D_BSri32(a_Str);
				mo->skin = D_BSri32(a_Str);
				mo->bnext = (void*)((intptr_t)D_BSri32(a_Str));
				mo->bprev = (void*)((intptr_t)D_BSri32(a_Str));
				PS_LUMapObjRef(a_Str, false, (void**)&mo->subsector);
				mo->floorz = D_BSri32(a_Str);
				mo->ceilingz = D_BSri32(a_Str);
				mo->radius = D_BSri32(a_Str);
				mo->height = D_BSri32(a_Str);
				mo->momx = D_BSri32(a_Str);
				mo->momy = D_BSri32(a_Str);
				mo->momz = D_BSri32(a_Str);
				
				D_BSrs(a_Str, Buf, BUFSIZE);
				mo->type = INFO_GetTypeByName(Buf);
				mo->info = mobjinfo[mo->type];
				
				mo->tics = D_BSri32(a_Str);
				PS_StateP(a_Str, false, &mo->state);
				mo->flags = D_BSri32(a_Str);
				mo->eflags = D_BSri32(a_Str);
				mo->flags2 = D_BSri32(a_Str);
				mo->special1 = D_BSri32(a_Str);
				mo->special2 = D_BSri32(a_Str);
				mo->health = D_BSri32(a_Str);
				mo->movedir = D_BSri32(a_Str);
				mo->movecount = D_BSri32(a_Str);
				mo->target = (void*)((intptr_t)D_BSri32(a_Str));
				mo->reactiontime = D_BSri32(a_Str);
				mo->threshold = D_BSri32(a_Str);
				
				x = D_BSri32(a_Str);
				if (x < 0 || x >= MAXPLAYERS)
					mo->player = NULL;
				else
					mo->player = &players[x];
				
				mo->lastlook = D_BSri32(a_Str);

				// Spawn point (might be script created)
				PS_LUMapObjRef(a_Str, false, (void**)&mo->spawnpoint);
				n = !!mo->spawnpoint;
				x = D_BSru8(a_Str);
				
				if (x)
					if (!n)
					{
						mo->spawnpoint = Z_Malloc(sizeof(*mo->spawnpoint), PU_LEVEL, NULL);
						
						mo->spawnpoint->x = D_BSri16(a_Str);
						mo->spawnpoint->y = D_BSri16(a_Str);
						mo->spawnpoint->z = D_BSri16(a_Str);
						mo->spawnpoint->angle = D_BSri16(a_Str);
						mo->spawnpoint->type = D_BSri16(a_Str);
						mo->spawnpoint->options = D_BSri16(a_Str);
						mo->spawnpoint->IsHexen = D_BSru8(a_Str);
						mo->spawnpoint->HeightOffset = D_BSri16(a_Str);
						mo->spawnpoint->ID = D_BSru16(a_Str);
						mo->spawnpoint->Special = D_BSru8(a_Str);
						for (i = 0; i < 5; i++)
							mo->spawnpoint->Args[i] = D_BSru8(a_Str);
						
						D_BSrs(a_Str, Buf, BUFSIZE);
						mo->spawnpoint->MoType = INFO_GetTypeByName(Buf);
						
						mo->spawnpoint->MarkedWeapon = D_BSru8(a_Str);
						mo->spawnpoint->mobj = (void*)((intptr_t)D_BSri32(a_Str));
					}
					
					else
					{
						D_BSri16(a_Str);
						D_BSri16(a_Str);
						D_BSri16(a_Str);
						D_BSri16(a_Str);
						D_BSri16(a_Str);
						D_BSri16(a_Str);
						D_BSru8(a_Str);
						D_BSri16(a_Str);
						D_BSru16(a_Str);
						D_BSru8(a_Str);
						for (i = 0; i < 5; i++)
							D_BSru8(a_Str);
						D_BSrs(a_Str, Buf, BUFSIZE);
						D_BSru8(a_Str);
						D_BSri32(a_Str);
					}

				mo->tracer = (void*)((intptr_t)D_BSri32(a_Str));
				mo->friction = D_BSri32(a_Str);
				mo->movefactor = D_BSri32(a_Str);
				mo->touching_sectorlist = P_GetSecNodeFromID(D_BSri32(a_Str));
				mo->dropped_ammo_count = D_BSri32(a_Str);

				for (i = 0; i < NUMINFORXFIELDS; i++)
					mo->RXFlags[i] = D_BSru32(a_Str);
				
				D_BSrs(a_Str, Buf, BUFSIZE);
				mo->RXShotWithWeapon = INFO_GetWeaponByName(Buf);
				
				mo->RXAttackAttackType = D_BSri32(a_Str);
				mo->RemoveMo = D_BSru8(a_Str);
				
				D_BSrs(a_Str, Buf, BUFSIZE);
				mo->RemType = INFO_GetTypeByName(Buf);
				
				mo->MaxZObtained = D_BSri32(a_Str);
				mo->SpawnOrder = D_BSru32(a_Str);
				mo->SkinTeamColor = D_BSri32(a_Str);
				mo->FollowPlayer = (void*)((intptr_t)D_BSri32(a_Str));

				for (i = 0; i < 2; i++)
				{
					mo->TimeThinking[i] = D_BSru32(a_Str);
					mo->TimeFromDead[i] = D_BSru32(a_Str);
				};

				mo->KillerPlayer = D_BSri32(a_Str);
				mo->FraggerID = D_BSru32(a_Str);

				for (i = 0; i < 3; i++)
					mo->DrawPos[i] = D_BSri32(a_Str);

				// Object on object count
				for (i = 0; i < 2; i++)
				{
					mo->MoOnCount[i] = D_BSru32(a_Str);
					
					mo->MoOn[i] = Z_Malloc(sizeof(*mo->MoOn[i]) * mo->MoOnCount[i], PU_LEVEL, NULL);
	
					for (j = 0; j < mo->MoOnCount[i]; j++)
						mo->MoOn[i][j] = (void*)((intptr_t)D_BSri32(a_Str));
				}

				// References
				for (i = 0; i < NUMPMOBJREFTYPES; i++)
				{
					mo->RefCount[i] = D_BSru32(a_Str);
					mo->RefListSz[i] = D_BSru32(a_Str);
					
					mo->RefList[i] = Z_Malloc(sizeof(*mo->RefList[i]) * mo->RefListSz[i], PU_LEVEL, NULL);
	
					for (j = 0; j < mo->RefListSz[i]; j++)
						mo->RefList[i][j] = (void*)((intptr_t)D_BSri32(a_Str));
				}
				
				// CTF Stuff
				mo->FakeColor = D_BSri32(a_Str);
				mo->CTFTeam = D_BSru8(a_Str);
				mo->AltTouchFunc = P_TouchIDToFunc(D_BSru32(a_Str));
				break;
				
				// Vertical Door
			case PTT_VERTICALDOOR:
				vldoor = (void*)Thinker;
				
				vldoor->type = D_BSri32(a_Str);
				PS_LUMapObjRef(a_Str, false, (void**)&vldoor->sector);
				vldoor->topheight = D_BSri32(a_Str);
				vldoor->speed = D_BSri32(a_Str);
				vldoor->direction = D_BSri32(a_Str);
				vldoor->topwait = D_BSri32(a_Str);
				vldoor->topcountdown = D_BSri32(a_Str);
				PS_LUMapObjRef(a_Str, false, (void**)&vldoor->line);
				break;
	
				// Light Source
			case PTT_FIREFLICKER:
				flicker = (void*)Thinker;
				
				PS_LUMapObjRef(a_Str, false, (void**)&flicker->sector);
				flicker->count = D_BSri32(a_Str);
				flicker->maxlight = D_BSri32(a_Str);
				flicker->minlight = D_BSri32(a_Str);
				break;
	
				// Light Source
			case PTT_LIGHTFLASH:
				lightflash = (void*)Thinker;
				
				PS_LUMapObjRef(a_Str, false, (void**)&lightflash->sector);
				lightflash->count = D_BSri32(a_Str);
				lightflash->maxlight = D_BSri32(a_Str);
				lightflash->minlight = D_BSri32(a_Str);
				lightflash->maxtime = D_BSri32(a_Str);
				lightflash->mintime = D_BSri32(a_Str);
				break;
	
				// Light Source
			case PTT_STROBEFLASH:
				strobe = (void*)Thinker;
				
				PS_LUMapObjRef(a_Str, false, (void**)&strobe->sector);
				strobe->count = D_BSri32(a_Str);
				strobe->maxlight = D_BSri32(a_Str);
				strobe->minlight = D_BSri32(a_Str);
				strobe->darktime = D_BSri32(a_Str);
				strobe->brighttime = D_BSri32(a_Str);
				break;
	
				// Light Source
			case PTT_GLOW:
				glow = (void*)Thinker;
				
				PS_LUMapObjRef(a_Str, false, (void**)&glow->sector);
				glow->maxlight = D_BSri32(a_Str);
				glow->minlight = D_BSri32(a_Str);
				glow->direction = D_BSri32(a_Str);
				break;
	
				// Light Source
			case PTT_LIGHTFADE:
				lightfade = (void*)Thinker;
				
				PS_LUMapObjRef(a_Str, false, (void**)&lightfade->sector);
				lightfade->destlevel = D_BSri32(a_Str);
				lightfade->speed = D_BSri32(a_Str);
				break;
	
				// Moving Surface
			case PTT_MOVEFLOOR:
				floormove = (void*)Thinker;
				
				floormove->type = D_BSri32(a_Str);
				floormove->crush = D_BSru32(a_Str);
				PS_LUMapObjRef(a_Str, false, (void**)&floormove->sector);
				floormove->direction = D_BSri32(a_Str);
				floormove->newspecial = D_BSru32(a_Str);
				floormove->oldspecial = D_BSru32(a_Str);
				floormove->texture = D_BSri32(a_Str);
				floormove->floordestheight = D_BSri32(a_Str);
				floormove->speed = D_BSri32(a_Str);
				break;
	
				// Moving Surface
			case PTT_MOVECEILING:
				ceiling = (void*)Thinker;
				
				ceiling->type = D_BSri32(a_Str);
				PS_LUMapObjRef(a_Str, false, (void**)&ceiling->sector);
				ceiling->bottomheight = D_BSri32(a_Str);
				ceiling->topheight = D_BSri32(a_Str);
				ceiling->speed = D_BSri32(a_Str);
				ceiling->oldspeed = D_BSri32(a_Str);
				ceiling->crush = D_BSru8(a_Str);
				ceiling->newspecial = D_BSru32(a_Str);
				ceiling->oldspecial = D_BSru32(a_Str);
				ceiling->texture = D_BSri32(a_Str);
				ceiling->direction = D_BSri32(a_Str);
				ceiling->tag = D_BSri32(a_Str);
				ceiling->olddirection = D_BSri32(a_Str);
				 
				// Find ceilinglist reference
				x = D_BSri32(a_Str);
				for (j = 1, clistrover = rootclist; clistrover; clistrover = clistrover->SaveLink, j++)
					if (j == x)
						break;
				ceiling->list = clistrover;
				break;
	
				// Moving Surface
			case PTT_PLATRAISE:
				plat = (plat_t*)Thinker;
				
				PS_LUMapObjRef(a_Str, false, (void**)&plat->sector);
				plat->speed = D_BSri32(a_Str);
				plat->low = D_BSri32(a_Str);
				plat->high = D_BSri32(a_Str);
				plat->wait = D_BSri32(a_Str);
				plat->count = D_BSri32(a_Str);
				plat->status = D_BSri32(a_Str);
				plat->oldstatus = D_BSri32(a_Str);
				plat->crush = D_BSru8(a_Str);
				plat->tag = D_BSri32(a_Str);
				plat->type = D_BSri32(a_Str);
				
				// Find platlist reference
				x = D_BSri32(a_Str);
				for (j = 1, plistrover = rootplist; plistrover; plistrover = plistrover->SaveLink, j++)
					if (j == x)
						break;
				plat->list = plistrover;
				break;
	
				// Moving Surface
			case PTT_MOVEELEVATOR:
				elevator = (elevator_t*)Thinker;
				
				elevator->type = D_BSru32(a_Str);
				PS_LUMapObjRef(a_Str, false, (void**)&elevator->sector);
				elevator->direction = D_BSri32(a_Str);
				elevator->floordestheight = D_BSri32(a_Str);
				elevator->ceilingdestheight = D_BSri32(a_Str);
				elevator->speed = D_BSri32(a_Str);
				elevator->Silent = D_BSru8(a_Str);
				elevator->PerpWait = D_BSri32(a_Str);
				elevator->PerpTicsLeft = D_BSri32(a_Str);
				PS_LUMapObjRef(a_Str, false, (void**)&elevator->CallLine);
				elevator->PDoorSpeed = D_BSri32(a_Str);
				elevator->OldDirection = D_BSri32(a_Str);
				elevator->Dinged = D_BSru8(a_Str);
				break;
	
				// Scrolling Line
			case PTT_SCROLL:
				scroll = (scroll_t*)Thinker;
				
				D_BSwi32(a_Str, scroll->dx);
				D_BSwi32(a_Str, scroll->dy);
				D_BSwi32(a_Str, scroll->affectee);
				D_BSwi32(a_Str, scroll->control);
				D_BSwi32(a_Str, scroll->last_height);
				D_BSwi32(a_Str, scroll->vdx);
				D_BSwi32(a_Str, scroll->vdy);
				D_BSwi32(a_Str, scroll->accel);
				D_BSwi32(a_Str, scroll->type);
				break;
	
				// Friction
			case PTT_FRICTION:
				friction = (void*)Thinker;
				
				friction->friction = D_BSri32(a_Str);
				friction->movefactor = D_BSri32(a_Str);
				friction->affectee = D_BSri32(a_Str);
				break;
	
				// Pusher/Puller
			case PTT_PUSHER:
				pusher = Thinker;
				
				pusher->type = D_BSru32(a_Str);
				pusher->source = (intptr_t)D_BSri32(a_Str);
				pusher->x_mag = D_BSri32(a_Str);
				pusher->y_mag = D_BSri32(a_Str);
				pusher->magnitude = D_BSri32(a_Str);
				pusher->radius = D_BSri32(a_Str);
				pusher->x = D_BSri32(a_Str);
				pusher->y = D_BSri32(a_Str);
				pusher->affectee = D_BSri32(a_Str);
				break;
			
				// Unknown
			default:
				return PS_IllegalSave(DSTR_PSAVEGC_UNHANDLEDTHINKER);
		}
	}
	
	// Restore ceiling list thinkers
	for (clistrover = rootclist; clistrover; clistrover = clistrover->SaveLink)
		clistrover->ceiling = (void*)PS_GetThinkerFromID((intptr_t)clistrover->ceiling);
	
	// Restore plat list thinkers
	for (plistrover = rootplist; plistrover; plistrover = plistrover->SaveLink)
		plistrover->plat = (void*)PS_GetThinkerFromID((intptr_t)plistrover->plat);
	
	// Restore sector node thinker IDs
	for (i = 0; i < g_NumMSecNodes; i++)
		g_MSecNodes[i]->m_thing = (void*)PS_GetThinkerFromID((intptr_t)g_MSecNodes[i]->m_thing);
		
	/* Restore BlockLinks */
	// Read Links
	j = (bmapwidth * bmapheight);
	n = 0;
	for (i = 0; i < j; i++)
	{
		// New block group?
		if ((i & (BMAPCOUNT - 1)) == 0)
		{
			// Expect start of group
			if (!PS_Expect(a_Str, "BKLN"))
				return false;
			
			// If this is the first block, then read the count
			if (i == 0)
				n = D_BSru32(a_Str);
		}
		
		// Read only if it is valid
		if (i < n)
		{
			x = D_BSri32(a_Str);
			
			if (i < j)
				blocklinks[i] = (void*)PS_GetThinkerFromID(((intptr_t)x));
		}
	}
	
	/* Restore FFloors */
	// TODO
	
	/* Restore Line Data */
	for (i = 0 ; i < numlines; i++)
	{
		line = &lines[i];
		
		// New?
		if ((i & (LINECOUNT - 1)) == 0)
			if (!PS_Expect(a_Str, "LINE"))
				return false;
		
		// Load Info
	}
	
	/* Restore Sector Data */
	for (i = 0 ; i < numsectors; i++)
	{
		sect = &sectors[i];
		
		// New?
		if ((i & (SECTORCOUNT - 1)) == 0)
			if (!PS_Expect(a_Str, "SECT"))
				return false;
		
		// Load Info
		sect->floorheight = D_BSri32(a_Str);
		sect->ceilingheight = D_BSri32(a_Str);
		sect->floorpic = D_BSri16(a_Str);
		sect->ceilingpic = D_BSri16(a_Str);
		sect->lightlevel = D_BSri16(a_Str);
		sect->special = D_BSru32(a_Str);
		sect->oldspecial = D_BSru32(a_Str);
		sect->tag = D_BSri16(a_Str);
		sect->nexttag = D_BSri32(a_Str);
		sect->firsttag = D_BSri32(a_Str);
		sect->soundtraversed = D_BSri16(a_Str);
		sect->floortype = D_BSri16(a_Str);
		
		sect->soundtarget = (void*)PS_GetThinkerFromID(((intptr_t)D_BSri32(a_Str)));
		
		for (j = 0; j < 4; j++)
			sect->blockbox[j] = D_BSri32(a_Str);

		PS_LoadUnloadNoiseThinker(a_Str, false, &sect->soundorg);

		sect->validcount = D_BSri32(a_Str);
		sect->thinglist = (void*)PS_GetThinkerFromID(((intptr_t)D_BSri32(a_Str)));
		sect->floordata = (void*)PS_GetThinkerFromID(((intptr_t)D_BSri32(a_Str)));
		sect->ceilingdata = (void*)PS_GetThinkerFromID(((intptr_t)D_BSri32(a_Str)));
		sect->lightingdata = (void*)PS_GetThinkerFromID(((intptr_t)D_BSri32(a_Str)));

		sect->stairlock = D_BSri32(a_Str);
		sect->prevsec = D_BSri32(a_Str);
		sect->nextsec = D_BSri32(a_Str);
		sect->floor_xoffs = D_BSri32(a_Str);
		sect->floor_yoffs = D_BSri32(a_Str);
		sect->ceiling_xoffs = D_BSri32(a_Str);
		sect->ceiling_yoffs = D_BSri32(a_Str);
		sect->heightsec = D_BSri32(a_Str);
		sect->altheightsec = D_BSri32(a_Str);
		sect->floorlightsec = D_BSri32(a_Str);
		sect->ceilinglightsec = D_BSri32(a_Str);
		sect->teamstartsec = D_BSri32(a_Str);
		sect->bottommap = D_BSri32(a_Str);
		sect->midmap = D_BSri32(a_Str);
		sect->topmap = D_BSri32(a_Str);
		sect->touching_thinglist = P_GetSecNodeFromID(D_BSri32(a_Str));

		sect->ffloors = P_GetFFloorFromID(D_BSri32(a_Str));

		sect->linecount = D_BSru32(a_Str);
		sect->lines = Z_Malloc(sizeof(*sect->lines) * sect->linecount, PU_LEVEL, NULL);
		for (j = 0; j < sect->linecount; j++)
			PS_LUMapObjRef(a_Str, false, (void**)&sect->lines[j]);

		sect->numattached = D_BSru32(a_Str);
		sect->attached = Z_Malloc(sizeof(*sect->attached) * sect->numattached, PU_LEVEL, NULL);
		for (j = 0; j < sect->numattached; j++)
			sect->attached[j] = D_BSri32(a_Str);

		sect->LLSelf = D_BSru8(a_Str);
		// TODO: Light Lists
		// lightlist_t* lightlist;
		// int32_t numlights;

		sect->moved = D_BSru8(a_Str);
		sect->validsort = D_BSri32(a_Str);
		sect->added = D_BSru8(a_Str);

		// TODO: Colormap
		// extracolormap_t* extra_colormap;
		
		D_BSrs(a_Str, Buf, BUFSIZE);
		sect->FloorTexture = Z_StrDup(Buf, PU_LEVEL, NULL);
		
		D_BSrs(a_Str, Buf, BUFSIZE);
		sect->CeilingTexture = Z_StrDup(Buf, PU_LEVEL, NULL);
		
		for (j = 0; j < 4; j++)
			sect->BBox[j] = D_BSri32(a_Str);

		sect->SoundSecRef = D_BSru32(a_Str);
		sect->AltSkyTexture = D_BSri32(a_Str);
		sect->AltSkyFlipped = D_BSru8(a_Str);

		sect->NumAdj = D_BSru32(a_Str);
		sect->Adj = Z_Malloc(sizeof(*sect->Adj) * sect->NumAdj, PU_LEVEL, NULL);
		for (j = 0; j < sect->NumAdj; j++)
			PS_LUMapObjRef(a_Str, false, (void**)&sect->Adj[j]);
	}
	
	/* Restore Map Thing References */
	for (i = 0; i < nummapthings; i++)
	{
		if ((i & (MAPTHINGCOUNT - 1)) == 0)
			if (!PS_Expect(a_Str, "MTRF"))
				return false;
			
		mapthings[i].MarkedWeapon = D_BSru8(a_Str);
		
		D_BSrs(a_Str, Buf, BUFSIZE);
		mapthings[i].MoType = INFO_GetTypeByName(Buf);
		
		mapthings[i].mobj = (void*)PS_GetThinkerFromID(((intptr_t)D_BSri32(a_Str)));
	}
	
	// Restore thinker references from thinkers
	for (Thinker = thinkercap.next; Thinker != &thinkercap; Thinker = Thinker->next)
		switch (Thinker->Type)
		{
				// Map Object
			case PTT_MOBJ:
				mo = Thinker;
				
				mo->snext = (void*)PS_GetThinkerFromID((intptr_t)mo->snext);
				mo->sprev = (void*)PS_GetThinkerFromID((intptr_t)mo->sprev);
				mo->bnext = (void*)PS_GetThinkerFromID((intptr_t)mo->bnext);
				mo->bprev = (void*)PS_GetThinkerFromID((intptr_t)mo->bprev);
				mo->target = (void*)PS_GetThinkerFromID((intptr_t)mo->target);
				mo->tracer = (void*)PS_GetThinkerFromID((intptr_t)mo->tracer);
				mo->FollowPlayer = (void*)PS_GetThinkerFromID((intptr_t)mo->FollowPlayer);
				
				for (i = 0; i < NUMPMOBJREFTYPES; i++)
					for (x = 0; x < mo->RefListSz[i]; x++)
						mo->RefList[i][x] = (void*)PS_GetThinkerFromID((intptr_t)mo->RefList[i][x]);
				
				for (i = 0; i < 2; i++)
					for (x = 0; x < mo->MoOnCount[i]; x++)
						mo->MoOn[i][x] = (void*)PS_GetThinkerFromID((intptr_t)mo->MoOn[i][x]);
				
				if (mo->spawnpoint)
					if (mo->spawnpoint < &mapthings[0] || mo->spawnpoint >= &mapthings[nummapthings])
						mo->spawnpoint->mobj = (void*)PS_GetThinkerFromID(((intptr_t)D_BSri32(mo->spawnpoint->mobj)));
				break;
				
				// Pusher
			case PTT_PUSHER:
				pusher = Thinker;
				
				pusher->source = (void*)PS_GetThinkerFromID((intptr_t)pusher->source);
				
				// Unknown
			default:
				break;
		}
	
	/* Restore Respawn Queue */
	if (!PS_Expect(a_Str, "IQUE"))
		return false;
	
	// Save entire queue
	n = D_BSri32(a_Str);
	iquehead = D_BSri32(a_Str);
	iquetail = D_BSri32(a_Str);
	
	for (i = 0; i < n; i++)
	{
		x = D_BSru8(a_Str);
		PS_LUMapObjRef(a_Str, false, (void**)(i < ITEMQUESIZE ? &itemrespawnque[i] : NULL));
		Tic = D_BSrcu64(a_Str);
		
		if (i < ITEMQUESIZE)
			itemrespawntime[i] = Tic;
		
		if (x)
			// References script spawned object
			if (!itemrespawnque[i])
			{
				itemrespawnque[i] = Z_Malloc(sizeof(*itemrespawnque[i]), PU_LEVEL, NULL);
				
				itemrespawnque[i]->x = D_BSri16(a_Str);
				itemrespawnque[i]->y = D_BSri16(a_Str);
				itemrespawnque[i]->z = D_BSri16(a_Str);
				itemrespawnque[i]->angle = D_BSri16(a_Str);
				itemrespawnque[i]->type = D_BSri16(a_Str);
				itemrespawnque[i]->options = D_BSri16(a_Str);
				itemrespawnque[i]->IsHexen = D_BSru8(a_Str);
				itemrespawnque[i]->HeightOffset = D_BSri16(a_Str);
				itemrespawnque[i]->ID = D_BSru16(a_Str);
				itemrespawnque[i]->Special = D_BSru8(a_Str);
				for (j = 0; j < 5; j++)
					itemrespawnque[i]->Args[j] = D_BSru8(a_Str);
				
				D_BSrs(a_Str, Buf, BUFSIZE);
				itemrespawnque[i]->MoType = INFO_GetTypeByName(Buf);
				
				itemrespawnque[i]->MarkedWeapon = D_BSru8(a_Str);
				itemrespawnque[i]->mobj = (void*)PS_GetThinkerFromID(((intptr_t)D_BSri32(a_Str)));
			}
			
			// Saved anyway, but discard due to thing match
			else
			{
				D_BSri16(a_Str);
				D_BSri16(a_Str);
				D_BSri16(a_Str);
				D_BSri16(a_Str);
				D_BSri16(a_Str);
				D_BSri16(a_Str);
				D_BSru8(a_Str);
				D_BSri16(a_Str);
				D_BSru16(a_Str);
				D_BSru8(a_Str);
				for (j = 0; j < 5; j++)
					D_BSru8(a_Str);
				D_BSrs(a_Str, Buf, BUFSIZE);
				D_BSru8(a_Str);
				D_BSri32(a_Str);
			}
	}
	
	
	/* Save buttons */
	if (!PS_Expect(a_Str, "BUTN"))
		return false;
	
	for (i = 0; i < MAXBUTTONS; i++)
	{
		button = &buttonlist[i];
		
		PS_LUMapObjRef(a_Str, false, (void**)&button->line);
		button->where = D_BSri32(a_Str);
		button->btexture = D_BSri32(a_Str);
		button->btimer = D_BSri32(a_Str);
		
		x = D_BSri32(a_Str);
		
		if (x < 0 || x >= numsectors)
			button->soundorg = NULL;
		else
			button->soundorg = &sectors[x].soundorg;
	}
	
	/* Load CTF Flag Info */
	if (!PS_Expect(a_Str, "CTFF"))
		return false;
	
	// Read Flags
	n = D_BSru8(a_Str);
	for (i = 0; i < n; i++)
		g_CTFFlags[i] = (void*)PS_GetThinkerFromID(((intptr_t)D_BSri32(a_Str)));
	g_CTFFlags[MAXSKINCOLORS] = (void*)PS_GetThinkerFromID(((intptr_t)D_BSri32(a_Str)));
		
	/* Expect "LMSC" */
	if (!PS_Expect(a_Str, "LMSC"))
		return false;
	
	// Body queue
	n = D_BSru8(a_Str);
	bodyqueslot = D_BSri32(a_Str);
	for (i = 0; i < n; i++)
		bodyque[i] = (void*)PS_GetThinkerFromID(((intptr_t)D_BSri32(a_Str)));
		
	// Coop Starts
	n = D_BSru8(a_Str);
	for (i = 0; i < n; i++)
	{
		x = D_BSri32(a_Str);
		
		if (i >= MAXPLAYERS || x < 0 || x >= nummapthings)
			playerstarts[i] = NULL;
		else
			playerstarts[i] = &mapthings[x];
	}
	
	// DM Starts
	n = D_BSri32(a_Str);
	
	if (n < MAX_DM_STARTS)
		numdmstarts = n;
	else
		numdmstarts = MAX_DM_STARTS;
	
	for (i = 0; i < n; i++)
	{
		x = D_BSri32(a_Str);
		
		if (i >= MAX_DM_STARTS || x < 0 || x >= nummapthings)
			deathmatchstarts[i] = NULL;
		else
			deathmatchstarts[i] = &mapthings[x];
	}
	
	// Team Starts
	n = D_BSru8(a_Str);
	k = D_BSru8(a_Str);
	
	for (i = 0; i < n; i++)
		for (j = 0; j < k; j++)
		{
			// Read start ID
			x = D_BSri32(a_Str);
			
			if (i >= MAXSKINCOLORS || j >= MAXPLAYERS || x < 0 || x >= nummapthings)
				g_TeamStarts[i][j] = NULL;
			else
				g_TeamStarts[i][j] = &mapthings[x];
		}
	
	// Map Totals
	for (i = 0; i < 5; i++)
		g_MapKIS[i] = D_BSri32(a_Str);
	totalkills = D_BSri32(a_Str);
	totalitems = D_BSri32(a_Str);
	totalsecret = D_BSri32(a_Str);
	
	/* Success! */
	return true;
#undef BUFSIZE
}

/*---------------------------------------------------------------------------*/

extern wbstartstruct_t wminfo;

/* PS_SaveInterState() -- Saves intermission data */
static bool_t PS_SaveInterState(D_BS_t* const a_Str)
{
	int32_t i, j;
	wbplayerstruct_t* wbps;	
	
	/* Save */
	D_BSBaseBlock(a_Str, "INTR");
	
	// Write fields
	D_BSwi32(a_Str, wminfo.epsd);
	D_BSwu8(a_Str, wminfo.didsecret);
	D_BSwi32(a_Str, wminfo.last);
	D_BSwi32(a_Str, wminfo.next);
	D_BSwi32(a_Str, wminfo.maxkills);
	D_BSwi32(a_Str, wminfo.maxitems);
	D_BSwi32(a_Str, wminfo.maxsecret);
	D_BSwi32(a_Str, wminfo.maxfrags);
	D_BSwi32(a_Str, wminfo.partime);
	D_BSwi32(a_Str, wminfo.pnum);
	
	// Next level
	if (wminfo.NextInfo)
	{
		D_BSwu8(a_Str, 1);
		D_BSws(a_Str, WL_GetWADName(wminfo.NextInfo->WAD, false));
		D_BSws(a_Str, wminfo.NextInfo->LumpName);
	}
	else
		D_BSwu8(a_Str, 0);
	
	// Record
	D_BSRecordBlock(a_Str);
	
	/* Write start structures for each player */
	for (i = 0; i < MAXPLAYERS; i++)
	{
		// Get structure
		wbps = &wminfo.plyr[i];
		
		// Start Block
		D_BSBaseBlock(a_Str, "WBPS");
		
		// Write Data
		D_BSwu8(a_Str, wbps->in);
		D_BSwi32(a_Str, wbps->skills);
		D_BSwi32(a_Str, wbps->sitems);
		D_BSwi32(a_Str, wbps->ssecret);
		D_BSwi32(a_Str, wbps->stime);
		D_BSwi32(a_Str, wbps->score);
		D_BSwu16(a_Str, wbps->addfrags);
		
		for (j = 0; j < MAXPLAYERS; j++)
			D_BSwu16(a_Str, wbps->frags[j]);
		
		// Record
		D_BSRecordBlock(a_Str);
	}
	
	/* Helper */
	D_BSBaseBlock(a_Str, "WILP");
	
	if (!WI_SaveGameHelper(a_Str))
		return false;
	
	D_BSRecordBlock(a_Str);

	/* Success! */
	return true;
}

/* PS_LoadInterState() -- Loads intermission data */
static bool_t PS_LoadInterState(D_BS_t* const a_Str)
{
#define BUFSIZE 128
	char Buf[BUFSIZE];
	int32_t i, j;
	wbplayerstruct_t* wbps;
	P_LevelInfoEx_t* pli;
	
	/* Expect "INTR" */
	if (!PS_Expect(a_Str, "INTR"))
		return false;
	
	// Write fields
	wminfo.epsd = D_BSri32(a_Str);
	wminfo.didsecret = D_BSru8(a_Str);
	wminfo.last = D_BSri32(a_Str);
	wminfo.next = D_BSri32(a_Str);
	wminfo.maxkills = D_BSri32(a_Str);
	wminfo.maxitems = D_BSri32(a_Str);
	wminfo.maxsecret = D_BSri32(a_Str);
	wminfo.maxfrags = D_BSri32(a_Str);
	wminfo.partime = D_BSri32(a_Str);
	wminfo.pnum = D_BSri32(a_Str);
	
	// Next level
	if (D_BSru8(a_Str))
	{
		// Ignore WAD Name
		D_BSrs(a_Str, Buf, BUFSIZE);
	
		// Attempt to locate level being played
		D_BSrs(a_Str, Buf, BUFSIZE);
		pli = P_FindLevelByNameEx(Buf, NULL);
	
		// If not found, then fail
		if (!pli)
			return PS_IllegalSave(DSTR_PSAVEGC_UNKNOWNLEVEL);
		
		// Set next
		wminfo.NextInfo = pli;
	}
	
	// No next level set
	else
		wminfo.NextInfo = NULL;
		
	
	/* Write start structures for each player */
	for (i = 0; i < MAXPLAYERS; i++)
	{
		// Get structure
		wbps = &wminfo.plyr[i];
		
		// Expect "WBPS"
		if (!PS_Expect(a_Str, "WBPS"))
			return false;
		
		// Write Data
		wbps->in = D_BSru8(a_Str);
		wbps->skills = D_BSri32(a_Str);
		wbps->sitems = D_BSri32(a_Str);
		wbps->ssecret = D_BSri32(a_Str);
		wbps->stime = D_BSri32(a_Str);
		wbps->score = D_BSri32(a_Str);
		wbps->addfrags = D_BSru16(a_Str);
		
		for (j = 0; j < MAXPLAYERS; j++)
			wbps->frags[j] = D_BSru16(a_Str);
	}
	
	/* Helper */
	if (!PS_Expect(a_Str, "WILP"))
		return false;
	
	if (!WI_LoadGameHelper(a_Str))
		return false;

	/* Success! */
	return true;
#undef BUFSIZE
}


/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

/*****************************************************************************/

/* P_SaveToStream() -- Save to stream */
bool_t P_SaveToStream(D_BS_t* const a_Str, D_BS_t* const a_OrigStr)
{
	/* Force Lag */
	//D_XNetForceLag();

	/* If on title screen, or demo, die. */
	if (gamestate == GS_DEMOSCREEN || demoplayback)
		return false;
	
	/* Network State */
	PS_SaveDummy(a_OrigStr, false);
	PS_SaveNetState(a_Str);
	PS_SavePlayers(a_Str);
	PS_SaveGameState(a_Str);
	PS_SaveMapState(a_Str);
	PS_SaveInterState(a_Str);
	PS_SaveDummy(a_Str, true);
	
	// All done
	return true;
}

/*****************************************************************************/

/* P_LoadFromStream() -- Load from stream */
bool_t P_LoadFromStream(D_BS_t* const a_Str, const bool_t a_DemoPlay)
{
	bool_t OK;
	int32_t i;
	
#if 0
	/* Force Lag */
	D_XNetForceLag();
#endif
	
	/* Disconnect if not playing or if we are a server loading a game */
	// If we are the server or playing solo, we want to disconnect dropping all
	// other players from the game. However, if we are a connecting client we
	// do not want to disconnect.
	// However, an option passed to the game can say to not disconnect, i.e.
	// such as when playing a demo or joining a netgame.
	l_SoloLoad = false;
	if (!a_DemoPlay)
		if (!SN_IsConnected() || SN_IsServer() || (demoplayback || gamestate == GS_DEMOSCREEN) || (SN_IsConnected() && !SN_WaitingForSave()))
		{
			// Disconnect
			SN_Disconnect(false, "Loading a save game.");
			
			// Start local server
				// Do not force add a first port
			SN_StartLocalServer(0, NULL, false, false);
			l_SoloLoad = true;
		}
	
	// Switch to the WFGS screen
	gamestate = GS_WAITINGPLAYERS;
	
	/* Clear level before loading */
	P_ExClearLevel();
	
	/* Set OK */
	OK = true;
	
	/* Network State */
	if (OK)
		OK = PS_LoadDummy(a_Str, false);
	
	if (OK)
		OK = PS_LoadNetState(a_Str);
		
	if (OK)
		OK = PS_LoadPlayers(a_Str);
		
	if (OK)
		OK = PS_LoadGameState(a_Str);
		
	if (OK)
		OK = PS_LoadMapState(a_Str);
	
	if (OK)
		OK = PS_LoadInterState(a_Str);
		
	if (OK)
		OK = PS_LoadDummy(a_Str, true);
	
	// Did not work
	if (!OK)
	{
		P_ExClearLevel();
		SN_Disconnect(false, "Failed to load savegame");
		return false;
	}
	
	/* Handle Reference Links (if any) */
	// Player look last
	g_LFPRover = (void*)PS_GetThinkerFromID((intptr_t)g_LFPRover);
	
	// Players
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
		{
			players[i].mo = (void*)PS_GetThinkerFromID((intptr_t)players[i].mo);
			players[i].attacker = (void*)PS_GetThinkerFromID((intptr_t)players[i].attacker);
			players[i].rain1 = (void*)PS_GetThinkerFromID((intptr_t)players[i].rain1);
			players[i].rain2 = (void*)PS_GetThinkerFromID((intptr_t)players[i].rain2);
			players[i].LastBFGBall = (void*)PS_GetThinkerFromID((intptr_t)players[i].LastBFGBall);
			players[i].Attackee = (void*)PS_GetThinkerFromID((intptr_t)players[i].Attackee);
		}
		
	// Initialize Level Based Info
	if (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION)
	{
		// Initialize Spectators
		P_SpecInit(-2);
		
		// Music
		if (g_CurrentLevelInfo)
			if (gamestate == GS_INTERMISSION)
			{
				if (g_CurrentLevelInfo->InterMus)
					S_ChangeMusicName(g_CurrentLevelInfo->InterMus, 1);
			}
			else
			{
				if (g_CurrentLevelInfo->Music)
					S_ChangeMusicName(g_CurrentLevelInfo->Music, 1);
			}
		
		// Sky
		P_SetupLevelSky();
		skyflatnum = R_GetFlatNumForName("F_SKY1");
		
		// Bot Nodes
		//B_InitNodes();
		
		// Correct player angles
			// This is so you are still facing the desired angle when you load
			// the game and not some random angle previously used.
		for (i = 0; i < MAXSPLITSCREEN; i++)
		{
			if (!D_ScrSplitHasPlayer(i))
				continue;
			
			if (!(g_Splits[i].Console >= 0 && g_Splits[i].Console < MAXPLAYERS))
				continue;
			
			if (!playeringame[g_Splits[i].Console])
				continue;
			
			localaiming[i] = players[g_Splits[i].Console].aiming;
			
			if (players[g_Splits[i].Console].mo)
				localangle[i] = players[g_Splits[i].Console].mo->angle;
		}
		
		// Update Scores
		P_UpdateScores();
	}
	
	/* Done! */
	return OK;
}

