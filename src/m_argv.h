// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION:

#ifndef __M_ARGV__
#define __M_ARGV__

//
// MISC
//
extern int myargc;
extern char** myargv;

// Returns the position of the given parameter
// in the arg list (0 if not found).
int M_CheckParm(char* check);

// GhostlyDeath <August 27, 2011> -- + and ++ via command line
void M_PushSpecialParameters(void);
void M_PushSpecialPlusParameters(void);

// return true if there is available parameters
// use it befor M_GetNext
bool_t M_IsNextParm(void);

// return the next parameter after a M_CheckParm
// NULL if not found use M_IsNext to find if there is a parameter
char* M_GetNextParm(void);

// Find a Response File
void M_FindResponseFile(void);

#endif							//__M_ARGV__
