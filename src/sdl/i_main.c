// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.org/
// -----------------------------------------------------------------------------
// Copyright (C) 2011 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION:
//      Main program, simply calls D_DoomMain high level loop.

/***************
*** INCLUDES ***
***************/

#include <SDL.h>
#include "doomdef.h"
#include "m_argv.h"
#include "d_main.h"

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

