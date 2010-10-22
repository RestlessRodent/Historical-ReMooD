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

#include "i_thread.h"
#include "z_zone.h"

#ifdef ENABLEMULTITHREADING
#ifdef _WIN32
#define THREADING_WIN32
#else
#define THREADING_PTHREAD
#endif
#else
#endif

#if defined(THREADING_PTHREAD)
#include <pthread.h>
#elif defined(THREADING_WIN32)
HANDLE* ThreadList = NULL;
size_t NumThreads = 0;
#endif

/* THREAD_CheckSupport() -- Checks to see if the system supports threading */
int THREAD_CheckSupport(void)
{
#ifndef ENABLEMULTITHREADING
	return THREADERROR_NOTSUPPORTED;
#else

#if defined(THREADING_WIN32)
	return THREADERROR_SUCCESS;
#else
	return THREADERROR_DEADFUNCTION;
#endif

#endif
}

/* THREAD_CreateThread() -- Creates a new thread */
int THREAD_CreateThread(void* (*ThreadFunc)(void*))
{
#ifndef ENABLEMULTITHREADING
	return THREADERROR_NOTSUPPORTED;
#else

#if defined(THREADING_WIN32)
	// Create the thread
	HANDLE* nList = NULL;
	HANDLE tHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFunc, NULL, 0, NULL);
	
	if (!tHandle)
		return THREADERROR_CREATEFAILED;
		
	// Add it to the list
	if (ThreadList)
	{
		nList = Z_Malloc(sizeof(HANDLE) * (NumThreads + 1), PU_STATIC, NULL);
		memcpy(nList, ThreadList, sizeof(HANDLE) * NumThreads);
		nList[NumThreads] = tHandle;
		Z_Free(ThreadList);
		ThreadList = nList;
	}
	else
	{
		ThreadList = Z_Malloc(sizeof(HANDLE), PU_STATIC, NULL);
		NumThreads = 1;
		*ThreadList = tHandle;
	}
	
	return THREADERROR_SUCCESS;
	
#else
	return THREADERROR_DEADFUNCTION;
#endif
#endif
}

/* THREAD_GetThreadHandle() -- Get a handle to the just created thread, if there's an error a negative is returned */
int THREAD_GetThreadHandle(void)
{
#ifndef ENABLEMULTITHREADING
	return -THREADERROR_NOTSUPPORTED;
#else
	return -THREADERROR_DEADFUNCTION;
#endif
}

/* THREAD_KillThread() -- Kills a thread based on it's handle or -1 for all threads */
int THREAD_KillThread(int handle)
{
	size_t i;
	
#ifndef ENABLEMULTITHREADING
	return THREADERROR_NOTSUPPORTED;
#else

#if defined(THREADING_WIN32)
	if (!ThreadList || handle < -1)
		return THREADERROR_BADHANDLE;
	
	if (handle == -1)
	{
		for (i = 0; i < NumThreads; i++)
			TerminateThread(ThreadList[i], 0);
			
		Z_Free(ThreadList);
		ThreadList = 0;
		NumThreads = 0;
			
		return THREADERROR_SUCCESS;
	}
	
	return THREADERROR_BADHANDLE;
#else
	return THREADERROR_DEADFUNCTION;
#endif
#endif
}

