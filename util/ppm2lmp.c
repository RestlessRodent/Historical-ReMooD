// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: ReMooD `deutex` Clone, for what ReMooD uses and bonus stuff

/***************
*** INCLUDES ***
***************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/****************
*** CONSTANTS ***
****************/

/*****************
*** STRUCTURES ***
*****************/

/****************
*** FUNCTIONS ***
****************/

/* main() -- Main entry point */
int main(int argc, char** argv)
{
	/* Check */
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s <input.ppm>\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	/* Success! */
	return EXIT_SUCCESS;
}

