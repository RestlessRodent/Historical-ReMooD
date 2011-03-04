// Emacs style mode select   -*- C++ -*- 
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
// DESCRIPTION: Internally used data structures for virtually everything,
//              key definitions, lots of other stuff.

#ifndef __DOOMDEF__
#define __DOOMDEF__

#ifdef _WIN32

/* Set the system to Windows 98/NT */
//#undef NTDDI_VERSION
#undef WINVER
#undef _WIN32_WINDOWS
#undef _WIN32_IE

#define WINVER 0x0410
#define _WIN32_WINDOWS 0x0410
#define _WIN32_IE 0x0400

/* Now include windows.h */
#include <windows.h>
#define ASMCALL __cdecl

#else

#define ASMCALL
#define min(x,y) ( ((x)<(y)) ? (x) : (y) )
#define max(x,y) ( ((x)>(y)) ? (x) : (y) )

#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

#if defined( _WIN32)
#include <io.h>
#endif

#ifdef PC_DOS
#include <conio.h>
#endif

#include "doomtype.h"

// Uncheck this to compile debugging code
//#define RANGECHECK
#ifndef PARANOIA
#define PARANOIA				// do some test that never happens but maybe
#endif
#if defined( _WIN32) || defined(LINUX)
#define LOGMESSAGES				// write message in log.txt (win32 and Linux only for the moment)
#endif

#define REMOOD_MAJORVERSION 1
#define REMOOD_MINORVERSION 0
#define REMOOD_RELEASEVERSION 'a'
#define REMOOD_VERSIONSTRING "1.0a"
#define REMOOD_VERSIONCODESTRING "Final"
#define REMOOD_FULLVERSIONSTRING ""REMOOD_VERSIONSTRING" \""REMOOD_VERSIONCODESTRING"\""
#define REMOOD_URL "http://remood.org/"

#define VERSION		((((REMOOD_MAJORVERSION % 10) * 100) + ((REMOOD_MINORVERSION % 10) * 10) + (((REMOOD_RELEASEVERSION - 'a') % 10))) + 100)
#define VERSIONSTRING  " \""REMOOD_VERSIONCODESTRING"\""

#if defined (LOGMESSAGES) && defined(LINUX)
#define INVALID_HANDLE_VALUE -1
extern int logstream;
#endif

// some tests, enable or desable it if it run or not
#define SPLITSCREEN
#define ABSOLUTEANGLE			// work fine, soon #ifdef and old code remove
#define NEWLIGHT				// compute lighting with bsp (in construction)
#define FRAGGLESCRIPT			// SoM: Activate FraggleScript
#define FIXROVERBUGS			// Fix some 3dfloor bugs. SSNTails 06-13-2002

// =========================================================================

// demo version when playback demo, or the current VERSION
// used to enable/disable selected features for backward compatibility
// (where possible)
extern byte demoversion;

// The maximum number of players, multiplayer/networking.
// NOTE: it needs more than this to increase the number of players...

#define MAXPLAYERS              32	// TODO: ... more!!!
#define MAXSPLITSCREENPLAYERS	4
#define MAXSKINS                MAXPLAYERS
#define PLAYERSMASK             (MAXPLAYERS-1)
#define MAXPLAYERNAME           21
#define MAXSKINCOLORS           16

#define SAVESTRINGSIZE          24

// State updates, number of tics / second.
// NOTE: used to setup the timer rate, see I_StartupTimer().
#define OLDTICRATE       35
#define NEWTICRATERATIO   1		// try 4 for 140 fps :)
#define TICRATE         (OLDTICRATE*NEWTICRATERATIO)

// Name of local directory for config files and savegames
#ifdef LINUX
#define DEFAULTDIR ".remood"
#else
#define DEFAULTDIR "remood"
#endif

#include "g_state.h"

// commonly used routines - moved here for include convenience

// i_system.h
void I_Error(char *error, ...);

// console.h
void CONS_Printf(char *fmt, ...);

#include "m_swap.h"

// m_misc.h
char *va(char *format, ...);
char *Z_StrDup(const char *in);
wchar_t *Z_StrDupW(const wchar_t *in);
wchar_t* Z_StrDupWfromA(const char* in);

// g_game.h
extern boolean devparm;			// development mode (-devparm)

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

// =======================
// Misc stuff for later...
// =======================

// if we ever make our alloc stuff...
#define ZZ_Alloc(x) Z_Malloc((x),PU_STATIC,NULL)

// debug me in color (v_video.c)
void IO_Color(unsigned char color, unsigned char r, unsigned char g, unsigned char b);

// i_system.c, replace getchar() once the keyboard has been appropriated
int I_GetKey(void);

#ifdef _WIN32
#ifndef R_OK
#define R_OK    0				//faB: win32 does not have R_OK in includes..
#endif
#ifndef W_OK
#define W_OK	02
#endif
#elif !defined( __OS2__)
#define _MAX_PATH   MAX_WADPATH
#endif

#endif							// __DOOMDEF__

