// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.sourceforge.net/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           Demyx                  (demyx@endgameftw.com)
//                    Dragan                 (poliee13@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 2008 ReMooD Team.
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION: MultiThreading

#ifndef __I_THREAD_H__
#define __I_THREAD_H__

#include "doomdef.h"

enum
{
	THREADERROR_SUCCESS,		// Operation worked!
	THREADERROR_NOTSUPPORTED,	// Threading is not supported on this system
	THREADERROR_USERDISABLED,	// User passed -nothread, -nothreading and/or -nothreads
	THREADERROR_DEADFUNCTION,	// Function is not implemented
	THREADERROR_BADHANDLE,		// The handle passed is invalid
	THREADERROR_CREATEFAILED,	// Thread creation failed

	MAXTHREADERROR
};

int THREAD_CheckSupport(void);
int THREAD_GetThreadHandle(void);
int THREAD_CreateThread(void* (*ThreadFunc)(void*));
int THREAD_KillThread(int handle);

#endif /* __I_THREAD_H__ */

