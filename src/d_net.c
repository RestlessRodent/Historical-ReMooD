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
// Copyright (C) 2011-2013 GhostlyDeath <ghostlydeath@remood.org>
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

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "doomstat.h"
#include "d_net.h"
#include "m_argv.h"
#include "p_demcmp.h"
#include "r_main.h"
#include "p_info.h"

/****************
*** CONSTANTS ***
****************/

#define MAXGLOBALBUFSIZE					8	// Size of global buffer

/*************
*** LOCALS ***
*************/

static bool_t l_DedSv;							// Dedicated server
static bool_t l_Connected;						// Connected
static bool_t l_Server;							// We are server

static ticcmd_t l_GlobalBuf[MAXGLOBALBUFSIZE];	// Global buffer
static int32_t l_GlobalAt = -1;					// Position Global buf is at

/****************
*** FUNCTIONS ***
****************/

bool_t D_NetSetPlayerName(const int32_t a_PlayerID, const char* const a_Name)
{
	return true;
}

bool_t D_NetPlayerChangedPause(const int32_t a_PlayerID)
{
	return true;
}

/*** GLOBAL TICS ***/

/* D_SNExtCmdInGlobal() -- Grabs extended command in global command */
bool_t D_SNExtCmdInGlobal(const uint8_t a_ID, uint8_t** const a_Wp)
{
	/* Check */
	if (a_ID < 0 || a_ID >= NUMDTCT || !a_Wp)
		return false;
	
	/* Prevent global buffer overflow */
	if (l_GlobalAt >= MAXGLOBALBUFSIZE - 1)
		return false;
	
	/* Nothing grabbed? */
	if (l_GlobalAt < 0)
	{
		l_GlobalAt = 0;
		memset(&l_GlobalBuf[l_GlobalAt], 0, sizeof(&l_GlobalBuf[l_GlobalAt]));
	}
	
	/* First attempt to grab, from first set */
	l_GlobalBuf[l_GlobalAt].Ctrl.Type = 1;	// Set extended
	if (D_SNExtCmdInTicCmd(a_ID, a_Wp, &l_GlobalBuf[l_GlobalAt]))
		return true;
	
	/* Failed, increase global at and try next one */
	l_GlobalAt++;
	l_GlobalBuf[l_GlobalAt].Ctrl.Type = 1;	// Set extended
	if (D_SNExtCmdInTicCmd(a_ID, a_Wp, &l_GlobalBuf[l_GlobalAt]))
		return true;
	
	/* Completely failed */
	return false;
}

/* D_SNExtCmdInTicCmd() -- Grabs extended command in tic command */
bool_t D_SNExtCmdInTicCmd(const uint8_t a_ID, uint8_t** const a_Wp, ticcmd_t* const a_TicCmd)
{
	uint16_t* dsP;
	uint8_t* dbP;
	
	/* Check */
	if (a_ID < 0 || a_ID >= NUMDTCT || !a_Wp || !a_TicCmd)
		return false;
	
	/* Extended Tic */
	if (a_TicCmd->Ctrl.Type == 1)
	{
		dsP = &a_TicCmd->Ext.DataSize;
		dbP = a_TicCmd->Ext.DataBuf;
	}
		
	/* Standard Tic */
	else if (a_TicCmd->Ctrl.Type == 0)
	{
		dsP = &a_TicCmd->Std.DataSize;
		dbP = a_TicCmd->Std.DataBuf;
	}
	
	/* Bad Type */
	else
		return false;
	
	/* Not enough room to store extended command? */
	if (c_TCDataSize[a_ID] + 2 >= MAXTCDATABUF - *dsP)
		return false;
		
	/* Write Command at point */
	*a_Wp = &((dbP)[*dsP]);
	WriteUInt8((uint8_t**)a_Wp, a_ID);
	*dsP += c_TCDataSize[a_ID] + 1;
	
	// Was written OK
	return true;
}

/*** SERVER CONTROL ***/

/* D_SNDisconnect() -- Disconnects from server */
void D_SNDisconnect(const bool_t a_FromDemo)
{
	static bool_t InDis;	
	
	/* If disconnected already, stop */
	if (InDis)
		return;
	
	// Do not double disconnect
	InDis = true;
	
	/* Clear the global buffer */
	l_GlobalAt = -1;
	memset(l_GlobalBuf, 0, sizeof(l_GlobalBuf));
	
	/* Done disconnecting */
	InDis = true;
}

/* D_SNIsConnected() -- Connected to server */
bool_t D_SNIsConnected(void)
{
	return l_Server || l_Connected;
}

/* D_SNStartServer() -- Starts local server */
bool_t D_SNStartServer(const int32_t a_NumLocal, const char** const a_Profs)
{
	/* Disconnect first */
	D_SNDisconnect(false);
	
	/* Set flags */
	l_Server = l_Connected = true;
	
	/* Set the proper gamestate */
	gamestate = wipegamestate = GS_WAITINGPLAYERS;
	S_ChangeMusicName("D_WAITIN", 1);			// A nice tune
	
	/* Set game settings */
	NG_ApplyVars();
	
	/* Calculate Split-screen */
	R_ExecuteSetViewSize();
	
	/* Created */
	return true;
}

/* D_SNServerInit() -- Initializes Server Mode */
bool_t D_SNServerInit(void)
{
	const char* PProfs[MAXPLAYERS];
	int32_t np, i;
	
	/* Clear initial profiles */
	memset(PProfs, 0, sizeof(PProfs));
	
	/* Dedicated server? */
	if (M_CheckParm("-dedicated"))
	{
		l_DedSv = true;
		
		// No players inside
		np = 0;
	}
	
	// Otherwise, set 1 player
	else
	{
		l_DedSv = false;
		
		// 1 player inside
		np = 1;
		
		// If -p1 is specified, use the indicated profile there
		if (M_CheckParm("-p1"))
			if (M_IsNextParm())
				PProfs[0] = M_GetNextParm();
	}
	
	/* Networked or local? */
	// Command line local game
	if (NG_IsAutoStart())
	{
		// Start Server
		D_SNStartServer(np, PProfs);
		return true;
	}
	
	/* No server started */
	return false;
}

/*** LOOP ***/

/* D_SNUpdate() -- Updates network state */
void D_SNUpdate(void)
{
}

/*** PORT CONTROL ***/

/* D_SNTics() -- Handles tic commands */
void D_SNTics(ticcmd_t* const a_TicCmd, const bool_t a_Write, const int32_t a_Player)
{
	/* If not writing, clear tic command */
	if (!a_Write)
		memset(a_TicCmd, 0, sizeof(a_TicCmd));
	
	/* If server, we dictate commands */
	if (l_Server)
	{
		// Write Commands to clients
		if (a_Write)
		{
		}
		
		// Read command from tic queues
		else
		{
			// Global
			if (a_Player < 0)
			{
				// Something is in the buffer
				if (l_GlobalAt >= 0)
				{
					// Move the first item inside
					memmove(a_TicCmd, &l_GlobalBuf[0], sizeof(*a_TicCmd));
					
					// Move everything down
					memmove(&l_GlobalBuf[0], &l_GlobalBuf[1], sizeof(ticcmd_t) * (MAXGLOBALBUFSIZE - 1));
					memset(&l_GlobalBuf[MAXGLOBALBUFSIZE - 1], 0, sizeof(l_GlobalBuf[MAXGLOBALBUFSIZE - 1]));
					l_GlobalAt--;
				}
				
				// Nothing
				else
					memset(a_TicCmd, 0, sizeof(*a_TicCmd));
			}
			
			// Player
			else
			{
			}
		}
	}
	
	/* Otherwise, server dictates to us */
	else
	{
		// Client cannot write commands
		if (a_Write)
			return;
	}
}


/*** GAME CONTROL ***/

/* D_SNChangeMap() -- Changes the map */
void D_SNChangeMap(const char* const a_NewMap, const bool_t a_Reset)
{
	uint8_t* Wp;
	P_LevelInfoEx_t* Info;
	int32_t i, j;	
	
	/* Check */
	if (!a_NewMap)
		return;
	
	/* See if the map exists first */
	if (!(Info = P_FindLevelByNameEx(a_NewMap, NULL)))
		return;
	
	/* Server */
	if (l_Server)
	{
		// Attempt global grab
		if (D_SNExtCmdInGlobal(DTCT_MAPCHANGE, &Wp))
		{
			// Resetting players?
			WriteUInt8((uint8_t**)&Wp, a_Reset);
			
			// Map name
			for (i = 0, j = 0; i < 8; i++)
				if (!j)
				{
					WriteUInt8((uint8_t**)&Wp, Info->LumpName[i]);
				
					if (!Info->LumpName[i])
						j = 1;
				}
				else
					WriteUInt8((uint8_t**)&Wp, 0);
		}
	}
	
	/* Client */
	else
	{
	}
}


