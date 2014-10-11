// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Internally used data structures for virtually everything,

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

#define SAVESTRINGSIZE          24


// Name of local directory for config files and savegames
#ifdef LINUX
#define DEFAULTDIR ".remood"
#else
#define DEFAULTDIR "remood"
#endif



// commonly used routines - moved here for include convenience

// i_system.h
void I_Error(char* error, ...);

// console.h
void CONS_Printf(char* fmt, ...);

// m_misc.h
char* va(char* format, ...);

// g_game.h
			// development mode (-devparm)

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

