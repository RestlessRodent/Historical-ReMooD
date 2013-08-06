// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
//         :oCCCCOCoc.
//     .cCO8OOOOOOOOO8Oo:
//   .oOO8OOOOOOOOOOOOOOOCc
//  cO8888:         .:oOOOOC.                                                TM
// :888888:   :CCCc   .oOOOOC.     ###      ###                    #########
// C888888:   .ooo:   .C########   #####  #####  ######    ######  ##########
// O888888:         .oO###    ###  #####  ##### ########  ######## ####    ###
// C888888:   :8O.   .C##########  ### #### ### ##    ##  ##    ## ####    ###
// :8@@@@8:   :888c   o###         ### #### ### ########  ######## ##########
//  :8@@@@C   C@@@@   oo########   ###  ##  ###  ######    ######  #########
//    cO@@@@@@@@@@@@@@@@@Oc0
//      :oO8@@@@@@@@@@Oo.
//         .oCOOOOOCc.                                      http://remood.org/
// ----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2013 GhostlyDeath <ghostlydeath@remood.org>
//                                      <ghostlydeath@gmail.com>
// ----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// ----------------------------------------------------------------------------
// DESCRIPTION: Internally used data structures for virtually everything,
//              key definitions, lots of other stuff.

#ifndef __DOOMDEF__
#define __DOOMDEF__

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
#define REMOOD_VERSIONCODESTRING "Stuffed Cabbage"
#define REMOOD_FULLVERSIONSTRING ""REMOOD_VERSIONSTRING" \""REMOOD_VERSIONCODESTRING"\""
#define REMOOD_URL "http://remood.org/"

#define VERSION		((((REMOOD_MAJORVERSION % 10) * 100) + ((REMOOD_MINORVERSION % 10) * 10) + (((REMOOD_RELEASEVERSION - 'a') % 10))) + 100)
#define VERSIONSTRING  " \""REMOOD_VERSIONCODESTRING"\""

// some tests, enable or desable it if it run or not
#define SPLITSCREEN
#define ABSOLUTEANGLE			// work fine, soon #ifdef and old code remove
#define FRAGGLESCRIPT			// SoM: Activate FraggleScript

// =========================================================================

// demo version when playback demo, or the current VERSION
// used to enable/disable selected features for backward compatibility
// (where possible)
extern uint8_t demoversion;

// The maximum number of players, multiplayer/networking.
// NOTE: it needs more than this to increase the number of players...

#define MAXPLAYERS              32	// TODO: ... more!!!
#define MAXSPLITSCREENPLAYERS	4
#define MAXSPLITSCREEN			MAXSPLITSCREENPLAYERS
#define MAXSKINS                MAXPLAYERS
#define MAXPLAYERNAME           32
#define MAXSKINCOLORS           16

#define SAVESTRINGSIZE          24

// State updates, number of tics / second.
#define TICRATE				UINT64_C(35)
#define TICSPERMS			29	// 28.5, but the benefit of the doubt

// Name of local directory for config files and savegames
#ifdef LINUX
#define DEFAULTDIR ".remood"
#else
#define DEFAULTDIR "remood"
#endif

//#include "g_state.h"

// commonly used routines - moved here for include convenience

// i_system.h
void I_Error(char* error, ...);

// console.h
void CONS_Printf(char* fmt, ...);

// m_misc.h
char* va(char* format, ...);

// g_game.h
extern bool_t devparm;			// development mode (-devparm)

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

// =======================
// Misc stuff for later...
// =======================

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

