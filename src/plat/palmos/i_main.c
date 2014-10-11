// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION:

/***************
*** INCLUDES ***
***************/

#include "doomdef.h"
#include "m_argv.h"
#include "d_main.h"

/****************
*** FUNCTIONS ***
****************/

/* PilotMain() -- Main Entry Point */
UInt32 PilotMain(UInt16 cmd, void* cmdPBP, UInt16 launchFlags)
{
	/* Set command line */
	I_CommonCommandLine(NULL, NULL, NULL);

	/* Run the game */
	D_DoomMain();
	D_DoomLoop();
	
	return 0;
}

