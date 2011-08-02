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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include <direct.h>				// for mkdir
#endif
#include <fcntl.h>

#if defined(FREEBSD) || defined(_WIN32)
#include <SDL.h>
#else
#include <SDL/SDL.h>
#endif

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

#ifdef LMOUSE2
#include <termios.h>
#endif

#ifdef LJOYSTICK				// linux joystick 1.x
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/joystick.h>
#endif

#include "doomdef.h"
#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"
#include "i_system.h"

#include "d_net.h"
#include "g_game.h"

#include "endtxt.h"

#include "i_joy.h"

extern void D_PostEvent(event_t *);

#ifdef LJOYSTICK
int joyfd = -1;
int joyaxes = 0;
int joystick_started = 0;
int joy_scale = 1;
#endif
JoyType_t Joystick;

#ifdef LMOUSE2
int fdmouse2 = -1;
int mouse2_started = 0;
#endif

#ifdef _WIN32
typedef BOOL(WINAPI * MyFunc) (LPCSTR RootName, PULARGE_INTEGER pulA,
							   PULARGE_INTEGER pulB, PULARGE_INTEGER pulFreeBytes);
#endif

//
// StartupKeyboard
//
void I_StartupKeyboard(void)
{
}

//
//I_StartupTimer
//
void I_StartupTimer(void)
{
}

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

//
// I_GetKey
//
int I_GetKey(void)
{
	// Warning: I_GetKey emties the event queue till next keypress
	event_t *ev;
	int rc = 0;

	// return the first keypress from the event queue
	for (; eventtail != eventhead; eventtail = (++eventtail) & (MAXEVENTS - 1))
	{
		ev = &events[eventtail];
		if (ev->type == ev_keydown)
		{
			rc = ev->data1;
			continue;
		}
	}

	return rc;
}

#ifdef LJOYSTICK
//
// I_JoyScale
//
void I_JoyScale()
{
	joy_scale = (cv_joyscale.value == 0) ? 1 : cv_joyscale.value;
}

//
// I_GetJoyEvent
//
void I_GetJoyEvent()
{
	struct js_event jdata;
	static event_t event = { 0, 0, 0, 0 };
	static int buttons = 0;
	if (!joystick_started)
		return;
	while (read(joyfd, &jdata, sizeof(jdata)) != -1)
	{
		switch (jdata.type)
		{
			case JS_EVENT_AXIS:
				event.type = ev_joystick;
				event.data1 = 0;
				switch (jdata.number)
				{
					case 0:
						event.data2 = ((jdata.value >> 5) / joy_scale) * joy_scale;
						D_PostEvent(&event);
						break;
					case 1:
						event.data3 = ((jdata.value >> 5) / joy_scale) * joy_scale;
						D_PostEvent(&event);
					default:
						break;
				}
				break;
			case JS_EVENT_BUTTON:
				if (jdata.number < JOYBUTTONS)
				{
					if (jdata.value)
					{
						if (!((buttons >> jdata.number) & 1))
						{
							buttons |= 1 << jdata.number;
							event.type = ev_keydown;
							event.data1 = KEY_JOY1 + jdata.number;
							D_PostEvent(&event);
						}
					}
					else
					{
						if ((buttons >> jdata.number) & 1)
						{
							buttons ^= 1 << jdata.number;
							event.type = ev_keyup;
							event.data1 = KEY_JOY1 + jdata.number;
							D_PostEvent(&event);
						}
					}
				}
				break;
		}
	}
}

//
// I_ShutdownJoystick
//
void I_ShutdownJoystick()
{
	if (joyfd != -1)
	{
		close(joyfd);
		joyfd = -1;
	}
	joyaxes = 0;
	joystick_started = 0;
}

//
// joy_open
//
int joy_open(char *fname)
{
	joyfd = open(fname, O_RDONLY | O_NONBLOCK);
	if (joyfd == -1)
	{
		CONS_Printf("Error opening %s!\n", fname);
		return 0;
	}
	ioctl(joyfd, JSIOCGAXES, &joyaxes);
	if (joyaxes < 2)
	{
		CONS_Printf("Not enought axes?\n");
		joyaxes = 0;
		joyfd = -1;
		close(joyfd);
		return 0;
	}
	return joyaxes;
}
/*int joy_waitb(int fd, int *xpos,int *ypos,int *hxpos,int *hypos) {
  int i,xps,yps,hxps,hyps;
  struct js_event jdata;
  for(i=0;i<1000;i++) {
    while(read(fd,&jdata,sizeof(jdata))!=-1) {
      switch(jdata.type) {
      case JS_EVENT_AXIS:
        switch(jdata.number) {
        case 0: // x
          xps = jdata.value;
          break;
        case 1: // y
          yps = jdata.value;
          break;
        case 3: // hat x
          hxps = jdata.value;
          break;
        case 4: // hat y
          hyps = jdata.value;
        default:
          break;
        }
        break;
      case JS_EVENT_BUTTON:
        break;
      }
    }
  }
  }*/
#endif							// LJOYSTICK

//
// I_InitJoystick
//
void I_InitJoystick(void)
{
#ifdef LJOYSTICK
	I_ShutdownJoystick();
	if (!strcmp(cv_usejoystick.string, "0"))
		return;
	if (!joy_open(cv_joyport.string))
		return;
	joystick_started = 1;
	return;
#endif
}

#ifdef LMOUSE2
//
// I_GetMouse2Event
//
void I_GetMouse2Event()
{
	static unsigned char mdata[5];
	static int i = 0, om2b = 0;
	int di, j, mlp, button;
	event_t event;
	const int mswap[8] = { 0, 4, 1, 5, 2, 6, 3, 7 };
	if (!mouse2_started)
		return;
	for (mlp = 0; mlp < 20; mlp++)
	{
		for (; i < 5; i++)
		{
			di = read(fdmouse2, mdata + i, 1);
			if (di == -1)
				return;
		}
		if ((mdata[0] & 0xf8) != 0x80)
		{
			for (j = 1; j < 5; j++)
			{
				if ((mdata[j] & 0xf8) == 0x80)
				{
					for (i = 0; i < 5 - j; i++)
					{			// shift
						mdata[i] = mdata[i + j];
					}
				}
			}
			if (i < 5)
				continue;
		}
		else
		{
			button = mswap[~mdata[0] & 0x07];
			for (j = 0; j < MOUSEBUTTONS; j++)
			{
				if (om2b & (1 << j))
				{
					if (!(button & (1 << j)))
					{			//keyup
						event.type = ev_keyup;
						event.data1 = KEY_2MOUSE1 + j;
						D_PostEvent(&event);
						om2b ^= 1 << j;
					}
				}
				else
				{
					if (button & (1 << j))
					{
						event.type = ev_keydown;
						event.data1 = KEY_2MOUSE1 + j;
						D_PostEvent(&event);
						om2b ^= 1 << j;
					}
				}
			}
			event.data2 = ((signed char)mdata[1]) + ((signed char)mdata[3]);
			event.data3 = ((signed char)mdata[2]) + ((signed char)mdata[4]);
			if (event.data2 && event.data3)
			{
				event.type = ev_mouse2;
				event.data1 = 0;
				D_PostEvent(&event);
			}
		}
		i = 0;
	}
}

//
// I_ShutdownMouse2
//
void I_ShutdownMouse2()
{
	if (fdmouse2 != -1)
		close(fdmouse2);
	mouse2_started = 0;
}

#endif

//
// I_StartupMouse2
// 
void I_StartupMouse2(void)
{
#ifdef LMOUSE2
	struct termios m2tio;
	int i, dtr, rts;
	I_ShutdownMouse2();
	if (cv_usemouse2.value == 0)
		return;
	if ((fdmouse2 = open(cv_mouse2port.string, O_RDONLY | O_NONBLOCK | O_NOCTTY)) == -1)
	{
		CONS_Printf("Error opening %s!\n", cv_mouse2port.string);
		return;
	}
	tcflush(fdmouse2, TCIOFLUSH);
	m2tio.c_iflag = IGNBRK;
	m2tio.c_oflag = 0;
	m2tio.c_cflag = CREAD | CLOCAL | HUPCL | CS8 | CSTOPB | B1200;
	m2tio.c_lflag = 0;
	m2tio.c_cc[VTIME] = 0;
	m2tio.c_cc[VMIN] = 1;
	tcsetattr(fdmouse2, TCSANOW, &m2tio);
	strupr(cv_mouse2opt.string);
	for (i = 0, rts = dtr = -1; i < strlen(cv_mouse2opt.string); i++)
	{
		if (cv_mouse2opt.string[i] == 'D')
		{
			if (cv_mouse2opt.string[i + 1] == '-')
			{
				dtr = 0;
			}
			else
			{
				dtr = 1;
			}
		}
		if (cv_mouse2opt.string[i] == 'R')
		{
			if (cv_mouse2opt.string[i + 1] == '-')
			{
				rts = 0;
			}
			else
			{
				rts = 1;
			}
		}
	}
	if ((dtr != -1) || (rts != -1))
	{
		if (!ioctl(fdmouse2, TIOCMGET, &i))
		{
			if (!dtr)
			{
				i &= ~TIOCM_DTR;
			}
			else
			{
				if (dtr > 0)
					i |= TIOCM_DTR;
			}
			if (!rts)
			{
				i &= ~TIOCM_RTS;
			}
			else
			{
				if (rts > 0)
					i |= TIOCM_RTS;
			}
			ioctl(fdmouse2, TIOCMSET, &i);
		}
	}
	mouse2_started = 1;
#endif
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

uint32_t LastTime = 0;

ULONG I_GetTime(void)
{
	Uint32 ticks = 0;
	static Uint32 basetime = 0;
	
	// milliseconds since SDL initialization
	ticks = SDL_GetTicks();

	if (!basetime)
		basetime = ticks;

	return (ticks - basetime) * TICRATE / 1000;
}

//
// I_Init
//
void I_Init(void)
{
	I_StartupSound();
	I_InitMusic();
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
	I_ShutdownMusic();
	I_ShutdownSound();
	I_ShutdownCD();
	// use this for 1.28 19990220 by Kin
	M_SaveConfig(NULL);
	I_ShutdownGraphics();
	I_ShutdownSystem();
	printf("\r");
	ShowEndTxt();
	exit(0);
}

void I_WaitVBL(int count)
{
	SDL_Delay(count);
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

	D_QuitNetGame();
	I_ShutdownMusic();
	I_ShutdownSound();
	I_ShutdownGraphics();
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
#if defined(__REMOOD_SYSTEM_WINDOWS)
	return mkdir(dirname);
#else
	return mkdir(dirname, unixright);
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


