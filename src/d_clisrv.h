// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: high level networking stuff

#ifndef __D_CLISRV__
#define __D_CLISRV__





// Networking and tick handling related.
#define BACKUPTICS            32

/* Externals */
extern bool_t server;

/* Functions */
void NetUpdate(void);			// Create any new ticcmds and broadcast to other players.
bool_t Playing(void);			// is there a game running
void D_QuitNetGame(void);		// Broadcasts special packets to other players to notify of game exit
void TryRunTics(tic_t realtic, tic_t* const a_TicRunCount);	// how many ticks to run

void D_RunSingleTic(void);

#endif

