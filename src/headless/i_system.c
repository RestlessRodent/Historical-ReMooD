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
// DESCRIPTION:

/***************
*** INCLUDES ***
***************/

/* System */
#include <stdint.h>

#if !defined(__REMOOD_SYSTEM_WINDOWS)
	#include <sys/stat.h>
#endif

/* Local */
#include "doomtype.h"

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

#include "doomdef.h"
#include "m_misc.h"
#include "i_system.h"

#include "d_net.h"
#include "g_game.h"

#include "i_video.h"
#include "i_sound.h"

extern void D_PostEvent(event_t *);

//
//I_OutputMsg
//
void I_OutputMsg(char *fmt, ...)
{
	va_list argptr;

	va_start(argptr, fmt);
	vfprintf(stderr, fmt, argptr);
	va_end(argptr);
}

void I_StartupKeyboard(void)
{
}

/* I_StartupTimer() -- Timer startup */
void I_StartupTimer(void)
{
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

int joy_open(char *fname)
{
}

uint8_t mb_used = 6 + 2;			// 2 more for caching sound

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
	static clock_t FirstClock;
	clock_t ThisClock = 0;
	
	/* Get current clock */
	ThisClock = clock();
	
	// FirstClock not set?
	if (!FirstClock)
		FirstClock = ThisClock;
	
	/* Return time passed */
	return ((ThisClock - FirstClock) * 1000) / CLOCKS_PER_SEC;
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
}

uint8_t *I_AllocLow(int length)
{
	uint8_t *mem;

	mem = (uint8_t *) malloc(length);
	memset(mem, 0, length);
	return mem;
}

//
// I_Error
//
void I_Error(char *error, ...)
{
	va_list argptr;
	char txt[512];

	if (devparm)
		abort();

	// Message first.
	va_start(argptr, error);
	fprintf(stderr, "Error: ");
	vfprintf(stderr, error, argptr);
	fprintf(stderr, "\n");
	va_end(argptr);

	fflush(stderr);

	// Shutdown. Here might be other errors.
	if (demorecording)
		G_CheckDemoStatus();
	
	// shutdown everything else which was registered
	I_ShutdownSystem();

	exit(-1);
}

void I_LocateWad(void)
{
	// relict from the Linux version
	return;
}

#ifdef LINUX
#define MEMINFO_FILE "/proc/meminfo"
#define MEMTOTAL "MemTotal:"
#define MEMFREE "MemFree:"
#endif

// quick fix for compil
size_t I_GetFreeMem(size_t * total)
{
#ifdef LINUX
	/* LINUX covers all the unix OS's.
	 */
#ifdef FREEBSD
	struct vmmeter sum;
	kvm_t *kd;
	struct nlist namelist[] = {
#define X_SUM   0
		{"_cnt"},
		{NULL}
	};
	if ((kd = kvm_open(NULL, NULL, NULL, O_RDONLY, "kvm_open")) == NULL)
	{
		*total = 0L;
		return 0;
	}
	if (kvm_nlist(kd, namelist) != 0)
	{
		kvm_close(kd);
		*total = 0L;
		return 0;
	}
	if (kvm_read(kd, namelist[X_SUM].n_value, &sum, sizeof(sum)) != sizeof(sum))
	{
		kvm_close(kd);
		*total = 0L;
		return 0;
	}
	kvm_close(kd);

	*total = sum.v_page_count * sum.v_page_size;
	return sum.v_free_count * sum.v_page_size;
#else
#ifdef SOLARIS
	/* Just guess */
	*total = 32 << 20;
	return 32 << 20;
#else
	/* Linux */
	char buf[1024];
	char *memTag;
	size_t freeKBytes;
	size_t totalKBytes;
	int n;
	int meminfo_fd = -1;

	meminfo_fd = open(MEMINFO_FILE, O_RDONLY);
	n = read(meminfo_fd, buf, 1023);
	close(meminfo_fd);

	if (n < 0)
	{
		// Error
		*total = 0L;
		return 0;
	}

	buf[n] = '\0';
	if (NULL == (memTag = strstr(buf, MEMTOTAL)))
	{
		// Error
		*total = 0L;
		return 0;
	}

	memTag += sizeof(MEMTOTAL);
	totalKBytes = atoi(memTag);

	if (NULL == (memTag = strstr(buf, MEMFREE)))
	{
		// Error
		*total = 0L;
		return 0;
	}

	memTag += sizeof(MEMFREE);
	freeKBytes = atoi(memTag);

	*total = totalKBytes << 10;
	return freeKBytes << 10;
#endif							/* SOLARIS */
#endif							/* FREEBSD */
#else
	/*  Not Linux.
	 */
#ifdef _WIN32
    MEMORYSTATUS info;

    info.dwLength = sizeof(MEMORYSTATUS);
    GlobalMemoryStatus( &info );
    if( total )
        *total = info.dwTotalPhys;
    return info.dwAvailPhys;
#else
	return 16 << 20;
#endif
#endif							/* LINUX */
}

/****************
*** FUNCTIONS ***
****************/

/* I_mkdir() -- Creates a new directory */
int I_mkdir(const char* a_Path, int a_UNIXPowers)
{
#if defined(__REMOOD_SYSTEM_WINDOWS)
	mkdir(a_Path);
#else
	// Ignore UNIX Powers
	mkdir(a_Path, S_IWUSR);
#endif
}

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

