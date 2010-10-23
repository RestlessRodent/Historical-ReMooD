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
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           TetrisMaster512        (tetrismaster512@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2010 The ReMooD Team..
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
// DESCRIPTION: doom games standard types
//              Simple basic typedefs, isolated here to make it easier
//              separating modules.
//    
//-----------------------------------------------------------------------------

#ifndef __DOOMTYPE__
#define __DOOMTYPE__

#ifdef _WIN32
#include <windows.h>
#endif

#include <wchar.h>

#ifdef _MSC_VER
typedef signed __int8 Int8;
typedef signed __int16 Int16;
typedef signed __int32 Int32;
typedef signed __int64 Int64;
typedef unsigned __int8 UInt8;
typedef unsigned __int16 UInt16;
typedef unsigned __int32 UInt32;
typedef unsigned __int64 UInt64;
#else
#include <stdint.h>
typedef int8_t Int8;
typedef int16_t Int16;
typedef int32_t Int32;
typedef int64_t Int64;
typedef uint8_t UInt8;
typedef uint16_t UInt16;
typedef uint32_t UInt32;
typedef uint64_t UInt64;
#endif

#ifdef _MSC_VER
#ifndef __ssize_t_defined
#if defined(_M_IA64) || defined(_M_X64) || defined(_WIN64)
typedef Int64 ssize_t;
#else
typedef Int32 ssize_t;
#endif
#define __ssize_t_defined
#endif
#endif

#if (_MSC_VER > 1200) || !defined(_MSC_VER)
typedef UInt8 byte;
#endif

typedef UInt32 tic_t;
#ifndef _WIN32
typedef Int32 boolean;
#else
#ifndef boolean
#define boolean BOOL
#endif
#endif

// Win32 :(
#define ULONG UInt32
#define USHORT UInt16

// Legacy Compat
#define INT64 Int64

#ifdef _MSC_VER
#define strncasecmp strnicmp
#define strcasecmp stricmp
#define inline __inline
#else
#define stricmp strcasecmp
#define strnicmp strncasecmp
#ifndef __MINGW32__
#define lstrlen(x) strlen(x)
#endif
#endif

// GhostlyDeath <April 23, 2009> -- Inline, Static and Const Messes
#if defined(_MSC_VER)
	#define INLINE __forceinline
	#define EXTERNINLINE
	#define STATIC static
	#define CONST const
#elif defined(__GNUC__) || defined(_GNUC_)
	#define INLINE inline
	#define EXTERNINLINE INLINE
	#define STATIC static
	#define CONST const
#else
	#define INLINE
	#define EXTERNINLINE
	#define STATIC
	#define CONST
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef false
#define false FALSE
#endif

#ifndef true
#define true TRUE
#endif

#ifndef MAXCHAR
#define MAXCHAR   ((Int8)0x7f)
#endif

#ifndef MAXSHORT
#define MAXSHORT  ((Int16)0x7fff)
#endif

#ifndef MAXINT
#define MAXINT    ((Int32)0x7fffffff)
#endif

#ifndef MAXLONG
#define MAXLONG	MAXINT			// ((long)0x7fffffff)
#endif

#ifndef MAXINT64
#define MAXINT64   ((Int64)0x7fffffffffffffffL)
#endif

#ifndef MINCHAR
#define MINCHAR   ((Int8)0x80)
#endif

#ifndef MINSHORT
#define MINSHORT  ((Int16)0x8000)
#endif

#ifndef MININT
#define MININT    ((Int32)0x80000000)
#endif

#ifndef MINLONG
#define MINLONG   MININT		//((long)0x80000000)
#endif

#ifndef MININT64
#define MININT64   ((Int64)0x8000000000000000L)
#endif

union FColorRGBA
{
	ULONG rgba;
	struct
	{
		byte red;
		byte green;
		byte blue;
		byte alpha;
	} s;
};
typedef union FColorRGBA RGBA_t;

#ifdef __BIG_ENDIAN__
#define UINT2RGBA(a) a
#else
#define UINT2RGBA(a) ((a&0xff)<<24)|((a&0xff00)<<8)|((a&0xff0000)>>8)|(((ULONG)a&0xff000000)>>24)
#endif

#endif							//__DOOMTYPE__

