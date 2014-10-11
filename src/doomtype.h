// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: doom games standard types

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
	#include <shlobj.h>
	#include <limits.h>
	#include <float.h>
#endif

/* Palm OS Ugly Includes */
#if defined(__palmos__)
	//#include "pealstub.h"
#endif

/* Required Stuff */
#if defined(__REMOOD_USECCSTUB)
	//#include "ccstub.h"
#else
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
	#include <time.h>

	// UNIX
	#if defined(__unix__)
		#include <unistd.h>
	#endif
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
	
	// Mac OS X has machine/endian.h
	#if defined(__APPLE__) && defined(__MACH__)
		#include <machine/endian.h>
		
		// Big endian? PowerPC system?
		#if __DARWIN_BYTE_ORDER == __DARWIN_BIG_ENDIAN
			#define __REMOOD_BIG_ENDIAN
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

/******************************************
*** SYSTEMS THAT REQUIRE ALIGNED ACCESS ***
******************************************/

#if defined(__arm__) || defined(_M_ARM) || defined(__sparc__) || defined(__sparc)
	#define __REMOOD_FORCEALIGN
#endif

/***********************
*** FIXED SIZE TYPES ***
***********************/

#if !defined(__REMOOD_IGNORE_FIXEDTYPES)

/* Palm OS */
#if defined(__palmos__)
	#include <PalmTypes.h>
	
	typedef Int8 int8_t;
	typedef Int16 int16_t;
	typedef Int32 int32_t;
	typedef UInt8 uint8_t;
	typedef UInt16 uint16_t;
	typedef UInt32 uint32_t;
	
	typedef signed long long int64_t;
	typedef unsigned long long uint64_t;
	
	typedef int32_t intptr_t;
	typedef uint32_t uintptr_t;
	
	typedef intptr_t ssize_t;
	typedef __SIZE_TYPE__ size_t;
	
	#define UINT32_C(x) x
	#define INT64_C(x) x##ll
	#define UINT64_C(x) x##ull

/* C99 Complaint Compilers */
#elif (__STDC_VERSION__ >= 199901L) || defined(__GNUC__) || defined(__WATCOMC__) || (defined(_MSC_VER) && _MSC_VER >= 1600)
	#include <stdint.h>
	
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
	
	#define INT8_C(x) x
	#define UINT8_C(x) x
	#define INT16_C(x) x
	#define UINT16_C(x) x
	#define INT32_C(x) x
	#define UINT32_C(x) x
	#define INT64_C(x) x##i64
	#define UINT64_C(x) x##ui64
#endif

#endif /* __REMOOD_IGNORE_FIXEDTYPES */

/***********
*** BOOL ***
***********/

/* bool_t -- Boolean, true or false */
#if defined(__palmos__)
	typedef Boolean bool_t;
#else
	typedef enum bool_e
	{
		false,
		true
	} bool_t;
#endif

/*****************
*** C KEYWORDS ***
*****************/

// Keywords

#if defined(__palmos__)
	#if defined(__NO_INLINE__)
		#define __REMOOD_INLINE
	#else
		#define __REMOOD_INLINE inline
	#endif
	#define __REMOOD_FORCEINLINE
	#define __REMOOD_UNUSED
	#define __REMOOD_DEPRECATED
#elif defined(__GNUC__)
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

/***************************
*** TARGET COMPATIBILITY ***
***************************/

/* Palm OS */
#if defined(__REMOOD_SYSTEM_PALMOS)
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
	#define isnan _isnan

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

// Max WAD Path
#ifndef MAX_WADPATH
	#define MAX_WADPATH PATH_MAX
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

/**************
*** VA_COPY ***
**************/

#if defined(__GNUC__)
	#define __REMOOD_VA_COPY(a,b) va_copy((a),(b))
	#define __REMOOD_VA_COPYEND(a) va_end((a))
#elif defined(_MSC_VER)
	#define __REMOOD_VA_COPY(a,b) ((a) = (b))
	#define __REMOOD_VA_COPYEND(a)
#else
	#define __REMOOD_VA_COPY(a,b) ((a) = (b))
	#define __REMOOD_VA_COPYEND(a)
#endif

/*************
*** LIMITS ***
*************/

#if defined(_MSC_VER)
	#if !defined(INT8_MAX)
		#define INT8_MAX _I8_MAX
	#endif

	#if !defined(INT8_MIN)
		#define INT8_MIN _I8_MIN
	#endif

	#if !defined(UINT8_MAX)
		#define UINT8_MAX _UI8_MAX
	#endif

	#if !defined(INT16_MAX)
		#define INT16_MAX _I16_MAX
	#endif

	#if !defined(INT16_MIN)
		#define INT16_MIN _I16_MIN
	#endif

	#if !defined(UINT16_MAX)
		#define UINT16_MAX _UI16_MAX
	#endif

	#if !defined(INT32_MAX)
		#define INT32_MAX _I32_MAX
	#endif

	#if !defined(INT32_MIN)
		#define INT32_MIN _I32_MIN
	#endif

	#if !defined(UINT32_MAX)
		#define UINT32_MAX _UI32_MAX
	#endif
#endif

/****************************
*** INCLUDE LIBRARY STUFF ***
****************************/

#include "c_lib.h"
#include "m_fixed.h"

/**************
*** VERSION ***
**************/

#define REMOOD_MAJORVERSION 1
#define REMOOD_MINORVERSION 0
#define REMOOD_RELEASEVERSION 'a'
#define REMOOD_VERSIONSTRING "1.0a"
#define REMOOD_VERSIONCODESTRING "Stuffed Cabbage"
#define REMOOD_FULLVERSIONSTRING ""REMOOD_VERSIONSTRING" \""REMOOD_VERSIONCODESTRING"\""
#define REMOOD_URL "http://remood.org/"

#define VERSION		((((REMOOD_MAJORVERSION % 10) * 100) + ((REMOOD_MINORVERSION % 10) * 10) + (((REMOOD_RELEASEVERSION - 'a') % 10))) + 100)
#define VERSIONSTRING  " \""REMOOD_VERSIONCODESTRING"\""

/******************
*** LIMITATIONS ***
******************/

#define MAXPLAYERS              32
#define MAXSPLITS	4
#define MAXSKINS                MAXPLAYERS
#define MAXPLAYERNAME           32
#define MAXSKINCOLORS           16
#define TICRATE				UINT64_C(35)
#define TICSPERMS			29	// 28.5, but the benefit of the doubt

// Should not be here, but here for convenience
#define NUMINFORXFIELDS 4		// Max sub fields in object flags
#define VIEWHEIGHT               41
#define VIEWHEIGHTS             "41"

/******************************
*** GLOBALS USED EVERYWHERE ***
******************************/

extern bool_t devparm;

#endif							/* __DOOMTYPE_H__ */

