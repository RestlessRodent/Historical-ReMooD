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
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION: DOOM Network game communication and protocol,
//              all OS independend parts.

#include "d_clisrv.h"
#include "doomdef.h"
#include "command.h"
#include "doomstat.h"
#include "console.h"
#include "m_menu.h"
#include "m_argv.h"
#include "g_game.h"
#include "d_main.h"
#include "p_tick.h"
#include "i_system.h"
#include "i_sound.h"
#include "i_video.h"

//
// NETWORKING
//
// gametic is the tic about to (or currently being) run
// maketic is the tic that hasn't had control made for it yet
// client:
//   neededtic is the tic needed by the client for run the game
//   firstticstosend is used to optimize a condition
// normaly maketic>=gametic>0,

bool_t server = false;			// true or false but !server=client

// server specific vars
static tic_t firstticstosend;	// min of the nettics
static tic_t tictoclear = 0;	// optimize d_clearticcmd
static tic_t maketic;
static tic_t neededtic;

// engine
ticcmd_t netcmds[BACKUPTICS][MAXPLAYERS];

int32_t g_IgnoreWipeTics;						// Demo playback, ignore this many wipe tics

// -----------------------------------------------------------------
//  Some extra data function for handle textcmd buffer
// -----------------------------------------------------------------

static void D_Clearticcmd(int tic)
{
	int i;
	
	for (i = 0; i < MAXPLAYERS; i++)
		netcmds[tic % BACKUPTICS][i].Std.angleturn = 0;	//&= ~TICCMD_RECEIVED;
}

#define ___STRINGIZE(x) #x
#define __STRINGIZE(x) ___STRINGIZE(x)

//
// D_QuitNetGame
// Called before quitting to leave a net game
// without hanging the other players
//
void D_QuitNetGame(void)
{
}

// is there a game running
bool_t Playing(void)
{
	return ((playeringame[0] == 1) && !demoplayback);
}

// Copy an array of ticcmd_t, swapping between host and network uint8_t order.
//
static void TicCmdCopy(ticcmd_t* dst, ticcmd_t* src, int n)
{
	int i;
	
	for (i = 0; i < n; src++, dst++, i++)
	{
		dst->Std.forwardmove = src->Std.forwardmove;
		dst->Std.sidemove = src->Std.sidemove;
		dst->Std.angleturn = src->Std.angleturn;
		dst->Std.aiming = src->Std.aiming;
		dst->Std.buttons = src->Std.buttons;
	}
}

//
// TryRunTics
//

extern bool_t advancedemo;
static int load;

/* TryRunTics() -- Attempts to run a single tic */
void TryRunTics(tic_t realtics)
{
	static tic_t LastTic;
	static int64_t LastMS;
	int64_t ThisMS, DiffMS;
	static bool_t ToggleUp;
	tic_t LocalTic, TargetTic;
	int STRuns;
	
	tic_t XXLocalTic, XXSNAR;

	// Init
	LocalTic = 0;
	ThisMS = I_GetTimeMS();
	
	// Last tic not set?
	if (!LastTic)
		LastTic = I_GetTime();
		
	// the machine have laged but is not so bad
	if (realtics > TICRATE / 7)	// FIXME: consistency failure!!
		realtics = TICRATE / 7;
		
	if (singletics)
		realtics = 1;
		
	//D_SyncNetUpdate();
	
	if (demoplayback)
	{
		neededtic = gametic + realtics;
		// start a game after a demo
		maketic += realtics;
		firstticstosend = maketic;
		tictoclear = firstticstosend;
	}
	
	// Title screen?
	if (gamestate == GS_DEMOSCREEN)
	{
		LocalTic = I_GetTime();
		
		// No update needed?
		if (LocalTic <= LastTic)
		{
			I_WaitVBL(20);
			return;
		}
		
		// If demo needs advancing
		if (advancedemo)
			D_DoAdvanceDemo();
		
		// Tic the title screen
		D_PageTicker();
		D_SyncNetUpdate();
		
		// Set last time
		LastTic = LocalTic;
		return;
	}
	
	// Connecting?
	else if (gamestate == GS_WAITFORJOINWINDOW)
	{
		LocalTic = I_GetTime();
		
		// No update needed?
		if (LocalTic <= LastTic)
		{
			I_WaitVBL(20);
			return;
		}
		
		// Update Network
		D_SyncNetUpdate();
		
		// Set last time
		LastTic = LocalTic;
		return;
	}
	
	/* While the client is behind, update it to catch up */
	// Update music
	I_UpdateMusic();
	
	// If the game is paused, don't do anything
	if (D_SyncNetIsPaused())
		return;
	
	// Get current time
	LocalTic = I_GetTime();
	
	// While the game is behind, update it
	if (demoplayback)
	{
		// Play enough tics to keep it synced to real percieved time
		if (!singletics)
		{
			if (LocalTic <= LastTic)
			{
				//I_WaitVBL(20);
				return;
			}
			
			if (g_IgnoreWipeTics)
			{
				LastTic = LocalTic;
				g_IgnoreWipeTics = 0;
			}
			
			XXSNAR = LocalTic - LastTic;
			LastTic = LocalTic;
		}
		
		// Force playback of every tic
		else
			XXSNAR = 1;
	}
	else
		XXSNAR = D_SyncNetAllReady();
	
	// Update Network
	D_SyncNetUpdate();
	
	if (XXSNAR > 0)
	{
		// Run tick loops
		while ((XXSNAR--) > 0)
		{
			// Run game ticker and increment the gametic
			G_Ticker();
			gametic++;
			
			// Single tics? -timedemo
			if (singletics)
				break;
		}
		
		// Set last MS time
		LastMS = ThisMS;
	}
	
	// Not behind so sleep
	else if (!singletics)
	{
		// Get time difference
		DiffMS = ThisMS - LastMS;
		
		// Cap
		if (DiffMS > 100)
			DiffMS = 100;
		
		if (DiffMS > (TICSPERMS >> 1))
			I_WaitVBL(DiffMS - (TICSPERMS >> 2));
		
		//I_WaitVBL(20);
	}
}

/* D_GetTics() -- Returns wrap capable time in tics */
static tic_t D_GetTics(void)
{
	register uint32_t ThisTime;
	static uint32_t FirstTime;
	static tic_t ShiftTime;
	
	/* Get the current time */
	ThisTime = I_GetTime();
	
	/* Last time not set? */
	if (!FirstTime)
		FirstTime = ThisTime;
		
	/* This time less than last time? */
	// An overflow occured, so we shift
	if (ThisTime < FirstTime)
	{
		// Add to shift time the lost time
		ShiftTime += FirstTime;
		
		// Reset last (since it will be the new base)
		FirstTime = ThisTime;
	}
	
	/* Return shift + (this - first) */
	return ShiftTime + (ThisTime - FirstTime);
}

void NetUpdate(void)
{
	static tic_t gametime = 0;
	tic_t nowtime;
	int i;
	int realtics;
	
	nowtime = D_GetTics();		//I_GetTime();
	realtics = nowtime - gametime;
	g_ProgramTic = I_GetTimeMS() / TICRATE;	//nowtime;
	
	if (singletics)
		realtics = 1;
	
	if (realtics <= 0)			// nothing new to update
		return;
	if (realtics > 5)
	{
		realtics = 5;
	}
	
	gametime = nowtime;
	
	I_OsPolling();				// i_getevent
	D_ProcessEvents();			// menu responder ???!!!
	
	if (!demoplayback)
		neededtic = maketic;
		
	CONL_Ticker();
	//CON_Ticker();
}
