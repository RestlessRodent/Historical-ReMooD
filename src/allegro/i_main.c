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

/* System */
// DJGPP's Allegro explodes if this isn't included first
#if defined(__DJGPP__)
#include <stdint.h>
#endif

#include <allegro.h>

// Include winalleg on Windows since it conflicts!
#if defined(_WIN32)
#define ALLEGRO_NO_MAGIC_MAIN	// Breaks with mingw-w64

#include <winalleg.h>
#endif

#define __REMOOD_IGNORE_FIXEDTYPES
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
	
	/* Initialize Allegro */
	if (allegro_init())
		return EXIT_FAILURE;
		
	/* Run the game */
	D_DoomMain();
	D_DoomLoop();
	
	/* Success! */
	return EXIT_SUCCESS;
}

#if !defined(ALLEGRO_NO_MAGIC_MAIN)

// Non-mangled main
END_OF_MAIN()

#else

/* GhostlyDeath <October 13, 2011> -- Mangle ourself */
// On mingw-w64 (i686-w64-mingw32-gcc (GCC) 4.7.0 20110831 (experimental)) due
// to Allegro using generic (probably a GCC 4.7 thing) it causes the build to
// fail. So instead, this is handled directly in this case. It works for
// i586-mingw32msvc-gcc (GCC) 4.4.4 so far.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	return _WinMain(main, hInstance, hPrevInstance, lpCmdLine, nShowCmd);
}

#endif

