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
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2009 The ReMooD Team..
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
// DESCRIPTION:

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

#include "endtxt.h"

#ifdef GAMECLIENT
#include "i_video.h"
#include "i_sound.h"
#include "i_joy.h"
JoyType_t Joystick;
#endif

extern void D_PostEvent(event_t *);

#ifdef _WIN32
typedef BOOL(WINAPI * MyFunc) (LPCSTR RootName, PULARGE_INTEGER pulA,
							   PULARGE_INTEGER pulB, PULARGE_INTEGER pulFreeBytes);
#endif

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

void I_ShutdownJoystick()
{
}

int joy_open(char *fname)
{
}

void I_InitJoystick(void)
{
}

//
// I_StartupMouse2
// 
void I_StartupMouse2(void)
{
}

byte mb_used = 6 + 2;			// 2 more for caching sound
static int quiting = 0;			/* prevent recursive I_Quit() */

//
// I_Tactile
//
void I_Tactile(int on, int off, int total)
{
	// UNUSED.
	on = off = total = 0;
}

ticcmd_t emptycmd;
ticcmd_t *I_BaseTiccmd(void)
{
	return &emptycmd;
}

//
// I_GetTime
// returns time in 1/TICRATE second tics
//

UInt32 LastTime = 0;

ULONG I_GetTime(void)
{
	UInt32 ticks = 0;
	static UInt32 basetime = 0;
	
	//ticks = ((float)clock() / (float)CLOCKS_PER_SEC) / 1000.0;
	ticks = clock() / (CLOCKS_PER_SEC / 1000);
	//ticks = clock();

	if (!basetime)
		basetime = ticks;

	return (ticks - basetime) * TICRATE / 1000;
}

//
// I_Init
//
void I_Init(void)
{
	quiting = 0;
}

//
// I_Quit
//
void I_Quit(void)
{
	/* prevent recursive I_Quit() */
	if (quiting)
		return;
	quiting = 1;
	//added:16-02-98: when recording a demo, should exit using 'q' key,
	//        but sometimes we forget and use 'F10'.. so save here too.
	if (demorecording)
		G_CheckDemoStatus();
	D_QuitNetGame();
	// use this for 1.28 19990220 by Kin
	M_SaveConfig(NULL);
	I_ShutdownSystem();
	printf("\r");
	exit(0);
}

void I_WaitVBL(int count)
{
#ifndef _WIN32
	usleep(count);
#else
	Sleep(count);
#endif
}

void I_BeginRead(void)
{
}

void I_EndRead(void)
{
}

byte *I_AllocLow(int length)
{
	byte *mem;

	mem = (byte *) malloc(length);
	memset(mem, 0, length);
	return mem;
}

//
// I_Error
//
extern boolean demorecording;

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

#ifdef _WIN32
	va_start(argptr, error);
	wvsprintf(txt, error, argptr);
	va_end(argptr);
	MessageBox(NULL, txt, "ReMooD Error", MB_OK | MB_ICONERROR);
#endif

	// Shutdown. Here might be other errors.
	if (demorecording)
		G_CheckDemoStatus();
	
	// shutdown everything else which was registered
	I_ShutdownSystem();

	exit(-1);
}
#define MAX_QUIT_FUNCS     16
typedef void (*quitfuncptr) ();
static quitfuncptr quit_funcs[MAX_QUIT_FUNCS] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
//
//  Adds a function to the list that need to be called by I_SystemShutdown().
//
void I_AddExitFunc(void (*func) ())
{
	int c;

	for (c = 0; c < MAX_QUIT_FUNCS; c++)
	{
		if (!quit_funcs[c])
		{
			quit_funcs[c] = func;
			break;
		}
	}
}

//
//  Removes a function from the list that need to be called by
//   I_SystemShutdown().
//
void I_RemoveExitFunc(void (*func) ())
{
	int c;

	for (c = 0; c < MAX_QUIT_FUNCS; c++)
	{
		if (quit_funcs[c] == func)
		{
			while (c < MAX_QUIT_FUNCS - 1)
			{
				quit_funcs[c] = quit_funcs[c + 1];
				c++;
			}
			quit_funcs[MAX_QUIT_FUNCS - 1] = NULL;
			break;
		}
	}
}

//
//  Closes down everything. This includes restoring the initial
//  pallete and video mode, and removing whatever mouse, keyboard, and
//  timer routines have been installed.
//
//  NOTE : Shutdown user funcs. are effectively called in reverse order.
//
void I_ShutdownSystem()
{
	int c;

	for (c = MAX_QUIT_FUNCS - 1; c >= 0; c--)
		if (quit_funcs[c])
			(*quit_funcs[c]) ();

}

void I_GetDiskFreeSpace(INT64 * freespace)
{

#ifdef LINUX
#ifdef SOLARIS
	*freespace = MAXINT64;
	return;
#else
	struct statfs stfs;
	if (statfs(".", &stfs) == -1)
	{
		*freespace = MAXINT64;
		return;
	}
	*freespace = stfs.f_bavail * stfs.f_bsize;
#endif
#endif

#ifdef _WIN32

	static MyFunc pfnGetDiskFreeSpaceEx = NULL;
	static boolean testwin95 = false;

	INT64 usedbytes;

	if (!testwin95)
	{
		HINSTANCE h = LoadLibraryA("kernel32.dll");

		if (h)
		{
			pfnGetDiskFreeSpaceEx = (MyFunc) GetProcAddress(h, "GetDiskFreeSpaceExA");
			FreeLibrary(h);
		}
		testwin95 = true;
	}
	if (pfnGetDiskFreeSpaceEx)
	{
		if (!pfnGetDiskFreeSpaceEx
			(NULL, (PULARGE_INTEGER) freespace, (PULARGE_INTEGER) & usedbytes, NULL))
			*freespace = MAXINT;
	}
	else
	{
		ULONG SectorsPerCluster, BytesPerSector, NumberOfFreeClusters;
		ULONG TotalNumberOfClusters;
		GetDiskFreeSpace(NULL, &SectorsPerCluster, &BytesPerSector,
						 &NumberOfFreeClusters, &TotalNumberOfClusters);
		*freespace = BytesPerSector * SectorsPerCluster * NumberOfFreeClusters;
	}

#endif

#if !defined (LINUX) && !defined (_WIN32)
	// Dummy for platform independent; 1GB should be enough
	*freespace = 1024 * 1024 * 1024;
#endif
}

char *I_GetUserName(void)
{
#ifdef LINUX

	static char username[MAXPLAYERNAME];
	char *p;
	if ((p = getenv("USER")) == NULL)
		if ((p = getenv("user")) == NULL)
			if ((p = getenv("USERNAME")) == NULL)
				if ((p = getenv("username")) == NULL)
					return NULL;
	strncpy(username, p, MAXPLAYERNAME);
	if (strcmp(username, "") == 0)
		return NULL;
	return username;

#endif

#ifdef _WIN32

	static char username[MAXPLAYERNAME];
	char *p;
	int ret;
	ULONG i = MAXPLAYERNAME;

	ret = GetUserName(username, &i);
	if (!ret)
	{
		if ((p = getenv("USER")) == NULL)
			if ((p = getenv("user")) == NULL)
				if ((p = getenv("USERNAME")) == NULL)
					if ((p = getenv("username")) == NULL)
						return NULL;
		strncpy(username, p, MAXPLAYERNAME);
	}
	if (strcmp(username, "") == 0)
		return NULL;
	return username;

#endif

#if !defined (LINUX) && !defined (_WIN32)

	// dummy for platform independent version
	return NULL;
#endif
}

int I_mkdir(const char *dirname, int unixright)
{
#ifdef LINUX
	return mkdir(dirname, unixright);
#else
	return mkdir(dirname);
#endif
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

