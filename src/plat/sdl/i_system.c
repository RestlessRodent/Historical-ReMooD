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
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include <direct.h>				// for mkdir
#include <windows.h>
#endif
#include <fcntl.h>
#include <time.h>

#ifdef LINUX
#ifndef FREEBSD
#include <sys/vfs.h>
#else
#include <sys/param.h>
#include <sys/mount.h>

/*For meminfo*/
#include <sys/types.h>
#include <kvm.h>
#include <nlist.h>
#include <sys/vmmeter.h>
#include <fcntl.h>
#endif
#endif

#if !defined(_MSC_VER)
#include <stdint.h>
#endif

#if !defined(__REMOOD_SYSTEM_WINDOWS)
#include <sys/stat.h>
#endif

/* Local */
#include "doomtype.h"
#include "i_util.h"

#include "doomdef.h"
#include "m_misc.h"
#include "i_system.h"


#include "g_game.h"

#include "i_video.h"
#include "i_sound.h"

//
//I_OutputMsg
//
void I_OutputMsg(char* fmt, ...)
{
	va_list argptr;
	
	va_start(argptr, fmt);
	vfprintf(stderr, fmt, argptr);
	va_end(argptr);
}

/* I_OutputText() -- Output text to console */
void I_OutputText(const char* const a_Text)
{
	fputs(a_Text, stderr);
}

void I_StartupKeyboard(void)
{
}

/* I_StartupTimer() -- Timer startup */
void I_StartupTimer(void)
{
	if (SDL_InitSubSystem(SDL_INIT_TIMER) == -1)
		I_Error("I_StartupTimer: Failed to initialize timer.\n");
}

int I_GetKey(void)
{
	return 0;
}

void I_JoyScale()
{
}

void I_GetJoyEvent()
{
}

//
// I_Tactile
//
void I_Tactile(int on, int off, int total)
{
	// UNUSED.
	on = off = total = 0;
}

/* I_GetTimeMS() -- Returns time since the game started (in MS) */
uint32_t I_GetTimeMS(void)
{
	static uint32_t FirstTick;
	uint32_t ThisTick = 0;
	
	/* Get current tick */
	ThisTick = SDL_GetTicks();
	
	// FirstTick not set?
	if (!FirstTick)
		FirstTick = ThisTick;
		
	/* Return time passed */
	return ThisTick - FirstTick;
}

//
// I_Init
//
void I_Init(void)
{
}

/* I_WaitVBL() -- Wait for vertical blank */
void I_WaitVBL(int count)
{
	SDL_Delay(count);
}

/****************
*** FUNCTIONS ***
****************/

/* I_SysAlloc() -- Allocate system memory */
void* I_SysAlloc(const size_t a_Size)
{
	return malloc(a_Size);
}

/* I_SysRealloc() -- Reallocate system memory */
void* I_SysRealloc(void* const a_Ptr, const size_t a_NewSize)
{
	return realloc(a_Ptr, a_NewSize);
}

/* I_SysFree() -- Free memory */
void I_SysFree(void* const a_Ptr)
{
	free(a_Ptr);
}

/* I_SystemPreExit() -- Called before functions are exited */
void I_SystemPreExit(void)
{
}

/* I_SystemPostExit() -- Called after functions are exited */
void I_SystemPostExit(void)
{
}

