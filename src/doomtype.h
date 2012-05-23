// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
// Copyright (C) 2011-2012 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: doom games standard types
//              Simple basic typedefs, isolated here to make it easier
//              separating modules.
//
//-----------------------------------------------------------------------------

#ifndef __DOOMTYPE_H__
#define __DOOMTYPE_H__

/***************
*** INCLUDES ***
***************/

/* Windows */
// windows.h is huge as hell and slows down the compile, maybe it can be removed?
#ifdef _WIN32
// Provided we aren't on Win64, target Windows 98!
#if !defined(_WIN64)
#undef WINVER
#undef _WIN32_WINDOWS
#undef _WIN32_IE

#define WINVER 0x0410
#define _WIN32_WINDOWS 0x0410
#define _WIN32_IE 0x0500
#endif

// Lean and mean!
#define WIN32_LEAN_AND_MEAN

// Now include
#include <windows.h>
#include <io.h>
#endif

/* Required Stuff */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <math.h>

/* UNIX */
#if defined(__unix__)
#include <unistd.h>
#endif

/* DJGPP */
#if defined(__REMOOD_SYSTEM_DOS) && defined(__DJGPP__)
	#define snprintf g_snprintf
	#define vsnprintf g_vsnprintf
#endif

/* Microsoft Visual C++ */
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
	#define vsnprintf _vsnprintf
#endif

/******************
*** BIG ENDIAN? ***
******************/

/* Just check for big endian */
#if !defined(__REMOOD_BIG_ENDIAN) && !defined(__REMOOD_LITTLE_ENDIAN)
// GCC has endian.h (but only on linux)
#if defined(__GNUC__) && defined(__linux__)
#include <endian.h>

#if defined(BYTE_ORDER) && (BYTE_ORDER == BIG_ENDIAN)
#define __REMOOD_BIG_ENDIAN 1
#elif defined(__BYTE_ORDER) && (__BYTE_ORDER == __BIG_ENDIAN)
#define __REMOOD_BIG_ENDIAN 1
#endif
#endif

// Known Big endian systems
#if !defined(__REMOOD_BIG_ENDIAN)
#if defined(__MIPSEB__) || defined(__BIG_ENDIAN__)
#define __REMOOD_BIG_ENDIAN 1
#endif
#endif

// Otherwise it is little
#if !defined(__REMOOD_BIG_ENDIAN) && !defined(__REMOOD_LITTLE_ENDIAN)
#define __REMOOD_LITTLE_ENDIAN 1
#endif
#endif

// Doubly be sure
#if !defined(__REMOOD_BIG_ENDIAN) && !defined(__REMOOD_LITTLE_ENDIAN)
#error Error No endian set
#elif defined(__REMOOD_BIG_ENDIAN) && defined(__REMOOD_LITTLE_ENDIAN)
#error Error Both endians set
#endif

/***********************
*** FIXED SIZE TYPES ***
***********************/

#if !defined(__REMOOD_IGNORE_FIXEDTYPES)

/* C99 Complaint Compilers */
#if (__STDC_VERSION__ >= 199901L) || defined(__GNUC__) || defined(__WATCOMC__)
	#include <stdint.h>

	#define __REMOOD_LL_SUFFIX(a) a##LL
	#define __REMOOD_ULL_SUFFIX(a) a##ULL

/* Microsoft Visual C++ */
#elif defined(_MSC_VER)
	typedef signed __int8 int8_t;
	typedef signed __int16 int16_t;
	typedef signed __int32 int32_t;
	typedef signed __int64 int64_t;
	typedef unsigned __int8 uint8_t;
	typedef unsigned __int16 uint16_t;
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int64 uint64_t;

	// Stuff MSVC <= 6 Lacks
	#if _MSC_VER <= 1400
		typedef int32_t ssize_t;
		typedef size_t uintptr_t;
		typedef ssize_t intptr_t;
	#endif
		
	#define UINT32_C(x) x
	#define INT64_C(x) x##i64
	#define UINT64_C(x) x##ui64
	
	#define __REMOOD_LL_SUFFIX(a) a##i64
	#define __REMOOD_ULL_SUFFIX(a) a##ui64
#endif

#endif /* __REMOOD_IGNORE_FIXEDTYPES */

/***********
*** BOOL ***
***********/

/* bool_t -- Boolean, true or false */
typedef enum bool_e
{
	false,
	true
} bool_t;

/*****************
*** C KEYWORDS ***
*****************/

// Keywords
#if defined(__GNUC__)
#define __REMOOD_INLINE inline
#define __REMOOD_FORCEINLINE __attribute__((always_inline))
#define __REMOOD_UNUSED __attribute__((unused))
#define __REMOOD_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define __REMOOD_INLINE _inline
#define __REMOOD_FORCEINLINE __forceinline
#define __REMOOD_UNUSED
#define __REMOOD_DEPRECATED
#else
#define __REMOOD_INLINE inline
#define __REMOOD_FORCEINLINE
#define __REMOOD_UNUSED
#define __REMOOD_DEPRECATED
#endif

/*****************************
*** COMPILER COMPATIBILITY ***
*****************************/

/* Microsoft Visual C++ */
#if defined(_MSC_VER)
#define strncasecmp strnicmp
#define strcasecmp stricmp
#define snprintf _snprintf
#define alloca _alloca

// Visual C++ does not like inline in C
#define inline _inline
#endif

// PATH_MAX/MAX_PATH
#ifndef PATH_MAX
#ifdef MAX_PATH
#define PATH_MAX MAX_PATH
#else
#define PATH_MAX 4096
#endif
#endif


/***************************
*** DATA READING/WRITING ***
***************************/

/************
*** TIC_T ***
************/

// A valid tic
typedef uint64_t tic_t;

/******************************************
*** REMOVE ALL THIS GARBAGE, SERIOUSLY! ***
******************************************/

union FColorRGBA
{
	uint32_t rgba;
	
	struct
	{
		uint8_t red;
		uint8_t green;
		uint8_t blue;
		uint8_t alpha;
	} s;
};
typedef union FColorRGBA RGBA_t;

#ifdef __REMOOD_BIG_ENDIAN
#define UINT2RGBA(a) a
#else
#define UINT2RGBA(a) ((a&0xff)<<24)|((a&0xff00)<<8)|((a&0xff0000)>>8)|(((ULONG)a&0xff000000)>>24)
#endif

/****************************
*** INCLUDE LIBRARY STUFF ***
****************************/

#include "c_lib.h"


#endif							/* __DOOMTYPE_H__ */

