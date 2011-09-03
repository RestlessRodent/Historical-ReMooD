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
// Copyright (C) 2011 GhostlyDeath <ghostlydeath@gmail.com>
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
// DESCRIPTION: ReMooD `deutex` Clone, for what ReMooD uses and bonus stuff

/***************
*** INCLUDES ***
***************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/****************
*** FUNCTIONS ***
****************/

/* main() -- Main entry point */
int main(int argc, char** argv)
{
	/* Check */
	if (argc < 3)
	{
		fprintf(stderr, "Usage: %s <wadinfo.txt> <output.wad>\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	/* Success! */
	return EXIT_SUCCESS;
}

