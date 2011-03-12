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
// Copyright (C) 2011 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION: All globals are here

/***************
*** INCLUDES ***
***************/

#include "globals.h"
#include "z_zone.h"

/**************
*** GLOBALS ***
**************/

ReMooDGlobals_t* g_Globals = NULL;

/*****************
*** PROTOTYPES ***
*****************/

/* ReMooD_GlobalsNew() -- Create new globals and set as current */
void ReMooD_GlobalsNew(void)
{
	/* Create */
	if (g_Globals)
	{
		g_Globals->Next = Z_Malloc(sizeof(*g_Globals), PU_STATIC, NULL);
		g_Globals->Next->Prev = g_Globals;
		g_Globals = g_Globals->Next;
	}
	else
		g_Globals = Z_Malloc(sizeof(*g_Globals), PU_STATIC, NULL);
	
	/* Initialize some */
	g_Globals->pagename = Z_Strdup("TITLEPIC", PU_STATIC);
	g_Globals->novideo = false;
}

/* ReMooD_GlobalsDelete() -- Delete current globals */
void ReMooD_GlobalsDelete(void)
{
}

/* ReMooD_GlobalsPrev() -- Go to the previous set of globals */
void ReMooD_GlobalsPrev(void)
{
}

/* ReMooD_GlobalsNext() -- Go to the next set of globals */
void ReMooD_GlobalsNext(void)
{
}

