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

#include <SDL.h>

#include "doomdef.h"
#include "m_argv.h"
#include "d_main.h"
#include "i_util.h"

/****************
*** FUNCTIONS ***
****************/

/* main() -- Main entry point */
int main(int argc, char** argv)
{
	/* Set command line */
	I_CommonCommandLine(&argc, &argv, NULL);
	
	/* Initial SDL Start */
	if (SDL_Init(SDL_INIT_NOPARACHUTE) == -1)
		I_Error("main: Failed to initialize SDL.");
		
	/* Run the game */
	D_DoomMain();
	D_DoomLoop();
	
	/* Success! */
	return EXIT_SUCCESS;
}

