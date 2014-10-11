// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: DOOM Network game communication and protocol,

#include "doomtype.h"
#include "g_state.h"
#include "g_game.h"
#include "i_system.h"
#include "console.h"
#include "d_main.h"
#include "i_video.h"
#include "m_menu.h"
#include "p_local.h"
#include "sn.h"
#include "bot.h"

//
// NETWORKING
//
// gametic is the tic about to (or currently being) run
// maketic is the tic that hasn't had control made for it yet
// client:
//   neededtic is the tic needed by the client for run the game
//   firstticstosend is used to optimize a condition
// normaly maketic>=gametic>0,

// server specific vars
static tic_t firstticstosend;	// min of the nettics
static tic_t tictoclear = 0;	// optimize d_clearticcmd
static tic_t maketic;
static tic_t neededtic;

int32_t g_IgnoreWipeTics;						// Demo playback, ignore this many wipe tics

// -----------------------------------------------------------------
//  Some extra data function for handle textcmd buffer
// -----------------------------------------------------------------

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

//
// TryRunTics
//

extern bool_t advancedemo;
static int load;

/* D_RunSingleTic() -- Single tic is run */
void D_RunSingleTic(void)
{
	// Do not do join windows in the middle of a tic!
		// Otherwise they will miss the tic and lag out! Not to mention
		// have a malformed save of sorts.
		
	// Legacy Demo Stuff
	if (demoplayback)
		G_DemoPreGTicker();
		
	// Execute Bots
		// This is so bots run every gametic
		// And they do not over process themselves
	BOT_Ticker();
	
	// Run game ticker and increment the gametic
	G_Ticker();
	++gametic;
	
	// Legacy Demo Stuff
	if (demoplayback)
		G_DemoPostGTicker();
	
	// People can join now
}

/* TryRunTics() -- Attempts to run a single tic */
void TryRunTics(tic_t realtics, tic_t* const a_TicRunCount)
{
	static tic_t LastTic;
	int64_t ThisMS;
	tic_t LocalTic;
	static tic_t LastPT;
	tic_t XXSNAR;
	
	/* Basic loop stuff */
	// Update time
	if (singletics)
		g_ProgramTic = gametic;
	else
		g_ProgramTic = I_GetTimeMS() / TICRATE;
		
	// This stuff is only important once a program tic
	if (LastPT != g_ProgramTic)
	{
		CONL_Ticker();
		M_SMTicker();				// Simple Menu Ticker
		
		LastPT = g_ProgramTic;
	}
	
	// Update events
	I_OsPolling();
	D_ProcessEvents();
	
	// Update network
	SN_Update();
	
	/* Init */
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
	
	if (demoplayback)
	{
		neededtic = gametic + realtics;
		// start a game after a demo
		maketic += realtics;
		firstticstosend = maketic;
		tictoclear = firstticstosend;
	}
	
	// Get current time
	LocalTic = I_GetTime();
	
	/* Run spectators independent of game timing */
	// So they move around during lag and other events
	if ((gamestate == GS_INTERMISSION || gamestate == GS_LEVEL) && LocalTic > LastTic)
		//for (XXSNAR = LocalTic - LastTic; XXSNAR > 0; XXSNAR--)
			P_SpecTicker();
	
	/* Title screen? */
	if (gamestate == GS_DEMOSCREEN)
	{
		// No update needed?
		if (LocalTic <= LastTic)
		{
			I_WaitVBL(20);
			return;
		}

#if !defined(__REMOOD_DEDICATED)
		// If demo needs advancing
		D_UITitleNext();
		
		// Tic the title screen
		D_UITitleTick();
#endif
		
		// Set last time
		LastTic = LocalTic;
		return;
	}
	
	/* Connecting? */
	else if (gamestate == GS_WAITFORJOINWINDOW)
	{
		// No update needed?
		if (LocalTic <= LastTic)
		{
			I_WaitVBL(20);
			return;
		}
		
		// Set last time
		LastTic = LocalTic;
		return;
	}
	
	/* While the client is behind, update it to catch up */
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
		XXSNAR = SN_OkTics(&LocalTic, &LastTic);
	
	/* Set tics that were run */
	if (a_TicRunCount)
		*a_TicRunCount = XXSNAR;
	
	/* Count Down Loops */
	if (XXSNAR > 0)
	{
		// Run tick loops
		while ((XXSNAR--) > 0)
		{
			// Run single tic
			D_RunSingleTic();
			
			// Single tics? -timedemo
			if (singletics)
				break;
		}
	}
	
	// Not behind so sleep
	else if (!singletics)
	{
	}
}

