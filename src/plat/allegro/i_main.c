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

