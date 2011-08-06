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
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2011 GhostlyDeath (ghostlydeath@gmail.com)
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

#ifndef __M_ARGV__
#define __M_ARGV__

//
// MISC
//
extern int myargc;
extern char **myargv;

// Returns the position of the given parameter
// in the arg list (0 if not found).
int M_CheckParm(char *check);

// push all parameters bigining by a +, ex : +map map01
void M_PushSpecialParameters(void);

// return true if there is available parameters
// use it befor M_GetNext 
bool_t M_IsNextParm(void);

// return the next parameter after a M_CheckParm
// NULL if not found use M_IsNext to find if there is a parameter
char *M_GetNextParm(void);

// Find a Response File
void M_FindResponseFile(void);

#endif							//__M_ARGV__
