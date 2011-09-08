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
#include "byteptr.h"
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

consvar_t cv_playdemospeed = { "playdemospeed", "0", 0, CV_Unsigned };

// -----------------------------------------------------------------
//  Some extra data function for handle textcmd buffer
// -----------------------------------------------------------------

static void D_Clearticcmd(int tic)
{
	int i;

	for (i = 0; i < MAXPLAYERS; i++)
		netcmds[tic % BACKUPTICS][i].angleturn = 0;	//&= ~TICCMD_RECEIVED;
}

CV_PossibleValue_t maxplayers_cons_t[] = { {1, "MIN"}
, {MAXPLAYERS, "MAX"}
, {0, NULL}
};

#define ___STRINGIZE(x) #x
#define __STRINGIZE(x) ___STRINGIZE(x)

consvar_t cv_allownewplayer = { "sv_allownewplayers", "1", 0, CV_OnOff };
consvar_t cv_maxplayers = { "sv_maxplayers", __STRINGIZE(MAXPLAYERS), CV_NETVAR, maxplayers_cons_t, NULL, 32 };

//
// D_QuitNetGame
// Called before quitting to leave a net game
// without hanging the other players
//
void D_QuitNetGame(void)
{
	MainDef.menuitems[1].status |= IT_DISABLED2;
}

// is there a game running
bool_t Playing(void)
{
	return ((playeringame[0] == 1) && !demoplayback);
}

// Copy an array of ticcmd_t, swapping between host and network uint8_t order.
//
static void TicCmdCopy(ticcmd_t * dst, ticcmd_t * src, int n)
{
	int i;
	for (i = 0; i < n; src++, dst++, i++)
	{
		dst->forwardmove = src->forwardmove;
		dst->sidemove = src->sidemove;
		dst->angleturn = readshort(&src->angleturn);
		dst->aiming = readshort(&src->aiming);
		dst->buttons = src->buttons;
	}
}

//
// TryRunTics
//
static void Local_Maketic(int realtics)
{
	int i;
	int Use;
	
	if (dedicated)
		return;

	I_OsPolling();				// i_getevent
	D_ProcessEvents();			// menu responder ???!!!
	// Cons responder
	// game responder call :
	//    HU_responder,St_responder, Am_responder
	//    F_responder (final)
	//    and G_MapEventsToControls

	rendergametic = gametic;
	
	//Use = maketic % BACKUPTICS;
	Use = D_SyncNetMapTime() % BACKUPTICS;
	
	for (i = 0; i < cv_splitscreen.value+1; i++)
		if (playeringame[consoleplayer[i]])
			G_BuildTiccmd(&netcmds[Use][consoleplayer[i]], realtics, i);
	maketic++;
	
	//netcmds[Use][consoleplayer[0]].angleturn |= TICCMD_RECEIVED;
}

extern bool_t advancedemo;
static int load;

void TryRunTics(tic_t realtics)
{
	tic_t LocalTic, TargetTic;
	
	// the machine have laged but is not so bad
	if (realtics > TICRATE / 7)	// FIXME: consistency failure!!
		realtics = TICRATE / 7;

	if (singletics)
		realtics = 1;

	if (realtics >= 1)
		COM_BufExecute();

	D_SyncNetUpdate();
	NetUpdate();

	if (demoplayback)
	{
		neededtic = gametic + realtics + cv_playdemospeed.value;
		// start a game after a demo
		maketic += realtics;
		firstticstosend = maketic;
		tictoclear = firstticstosend;
	}
	
	/* While the client is behind, update it to catch up */
	do
	{
		// Set local time and target time
		LocalTic = D_SyncNetMapTime();
		TargetTic = D_SyncNetAllReady();
		
		// Update the client if it is needed
		if (LocalTic < TargetTic)
		{
			// Run game ticker and increment the gametic
			G_Ticker();
			gametic++;
			
			// Set map time to be closer
			D_SyncNetSetMapTime(++LocalTic);
			
			// If the target was reached, update useless stuff
			if (LocalTic == TargetTic)
			{
				// Update music
				I_UpdateMusic();
			}
		}
		
		// Otherwise no updating is needed
		else
		{
			I_WaitVBL(20);
		}
	} while (D_SyncNetMapTime() < TargetTic);
	
#if 0
	if (neededtic > gametic)
	{
		if (advancedemo)
			D_DoAdvanceDemo();
		else
		{
			// run the count * tics
			while (neededtic > gametic)
			{

				//if (gametic % 5 == 0)
					I_UpdateMusic();
				G_Ticker();
				gametic++;
			
				// skip paused tic in a demo
				if (demoplayback)
				{
					if (paused)
					{
						neededtic++;
						//I_WaitVBL(20);
					}
				}
			}
		}
	}
	else if (neededtic == gametic)
		I_WaitVBL(20);
	
	/*else
		I_WaitVBL(20);*/
#endif
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

	nowtime = D_GetTics();//I_GetTime();
	realtics = nowtime - gametime;
	g_ProgramTic = I_GetTimeMS() / TICRATE;//nowtime;

	if (realtics <= 0)			// nothing new to update
		return;
	if (realtics > 5)
	{
		realtics = 5;
	}

	gametime = nowtime;

	Local_Maketic(realtics);	// make local tic, and call menu ?!
	
	if (!demoplayback)
		neededtic = maketic;

	M_Ticker();
	CONEx_Ticker();
	CON_Ticker();
}

