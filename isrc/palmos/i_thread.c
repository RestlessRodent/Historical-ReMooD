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

#include "i_thread.h"
#include "z_zone.h"

/* THREAD_CheckSupport() -- Checks to see if the system supports threading */
int THREAD_CheckSupport(void)
{
	return THREADERROR_NOTSUPPORTED;
}

/* THREAD_CreateThread() -- Creates a new thread */
int THREAD_CreateThread(void* (*ThreadFunc)(void*))
{
	return THREADERROR_NOTSUPPORTED;
}

/* THREAD_GetThreadHandle() -- Get a handle to the just created thread, if there's an error a negative is returned */
int THREAD_GetThreadHandle(void)
{
	return THREADERROR_NOTSUPPORTED;
}

/* THREAD_KillThread() -- Kills a thread based on it's handle or -1 for all threads */
int THREAD_KillThread(int handle)
{
	return THREADERROR_NOTSUPPORTED;
}

