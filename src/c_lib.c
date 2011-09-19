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
// DESCRIPTION: Missing C Stuff

/***************
*** INCLUDES ***
***************/

#include "c_lib.h"

/****************
*** FUNCTIONS ***
****************/

/* C_strupr() -- Uppercase characters */
char* C_strupr(char* s)
{
	char* x;
	
	/* Check */
	if (!s)
		return NULL;
		
	/* Run */
	x = s;
	while (*x)
	{
		*x = toupper(*x);
		x++;
	}
	
	return s;
}

/* C_strlwr() -- Lowercase characters */
char* C_strlwr(char* s)
{
	char* x;
	
	/* Check */
	if (!s)
		return NULL;
		
	/* Run */
	x = s;
	while (*x)
	{
		*x = tolower(*x);
		x++;
	}
	
	return s;
}
