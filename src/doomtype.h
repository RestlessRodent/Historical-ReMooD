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
// DESCRIPTION: doom games standard types
//              Simple basic typedefs, isolated here to make it easier
//              separating modules.
//    
//-----------------------------------------------------------------------------

#ifndef __DOOMTYPE_H__
#define __DOOMTYPE_H__

#ifdef _WIN32
#include <windows.h>
#endif

#include <stddef.h>

#include <wchar.h>

/***********************
*** FIXED SIZE TYPES ***
***********************/

/* Microsoft Visual C++ */
#if defined(_MSC_VER)
	typedef signed __int8 int8_t;
	typedef signed __int16 int16_t;
	typedef signed __int32 int32_t;
	typedef signed __int64 int64_t;
	typedef unsigned __int8 uint8_t;
	typedef unsigned __int16 uint16_t;
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int64 uint64_t;

/* Everything else */
#elif (__STDC_VERSION__ >= 199901L) || defined(__GNUC__) || defined(__WATCOMC__)
	#include <stdint.h>
#endif

/*****************
*** C KEYWORDS ***
*****************/

// Keywords
#if defined(__GNUC__)
	#define __REMOOD_INLINE inline
	#define __REMOOD_FORCEINLINE __attribute__((always_inline))
	#define __REMOOD_UNUSED __attribute__((unused))
#elif defined(_MSC_VER)
	#define __REMOOD_INLINE _inline
	#define __REMOOD_FORCEINLINE __forceinline
	#define __REMOOD_UNUSED
#else
	#define __REMOOD_INLINE inline
	#define __REMOOD_FORCEINLINE
	#define __REMOOD_UNUSED
#endif

// Visual C++ does not like inline in C
#if defined(_MSC_VER)
	#define inline _inline
#endif

// String comparison
#if defined(_MSC_VER)
	#define strcasecmp stricmp
	#define strncasecmp strnicmp
	#define snprintf _snprintf
	#define alloca _alloca
#else
	#define stricmp strcasecmp
	#define strnicmp strncasecmp
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

#define BP_MERGE(a,b) a##b

#if defined(__arm__) || defined(_M_ARM) || defined(__sparc__) || defined(__sparc)
	/* Access to pointer data for system that can't handle unaligned access */
	// Lets say we have the following data:
	// { 01  23  45  67  |  89  AB  CD  EF }
	//      [*DEREF           ]
	// On normal systems we can just dereference as normal, but on some systems
	// such as ARM, we cannot do this. Instead we have to dereference both sides
	// then merge the data together.
	// Or we could just read byte by byte.

	#define BP_READ(w,x) static inline x __REMOOD_FORCEINLINE BP_MERGE(Read,w)(const x** const Ptr)\
	{\
		x Ret = 0;\
		uint8_t* p8;\
		size_t i;\
		\
		if (!Ptr || !(*Ptr))\
			return 0;\
		\
		p8 = (uint8_t*)*Ptr;\
		for (i = 0; i < sizeof(x); i++)\
			((uint8_t*)&Ret)[i] = p8[i];\
		\
		(*Ptr)++;\
		return Ret;\
	}
#else
	/* Normal Pointer Access */
	#define BP_READ(w,x) static inline x __REMOOD_FORCEINLINE BP_MERGE(Read,w)(const x** const Ptr)\
	{\
		x Ret;\
		\
		if (!Ptr || !(*Ptr))\
			return 0;\
		\
		Ret = **Ptr;\
		(*Ptr)++;\
		return Ret;\
	}
#endif

BP_READ(Int8,int8_t)
BP_READ(Int16,int16_t)
BP_READ(Int32,int32_t)
BP_READ(Int64,int64_t)
BP_READ(UInt8,uint8_t)
BP_READ(UInt16,uint16_t)
BP_READ(UInt32,uint32_t)
BP_READ(UInt64,uint64_t)

#define BP_WRITE(w,x) static inline void __REMOOD_FORCEINLINE BP_MERGE(Write,w)(x** const Ptr, const x Val)\
{\
	if (!Ptr || !(*Ptr))\
		return;\
	**Ptr = Val;\
	(*Ptr)++;\
}

BP_WRITE(Int8,int8_t)
BP_WRITE(Int16,int16_t)
BP_WRITE(Int32,int32_t)
BP_WRITE(Int64,int64_t)
BP_WRITE(UInt8,uint8_t)
BP_WRITE(UInt16,uint16_t)
BP_WRITE(UInt32,uint32_t)
BP_WRITE(UInt64,uint64_t)

#undef BP_READ
#undef BP_WRITE

/* WriteString() -- Write a string of any length (bad) */
static inline void __REMOOD_FORCEINLINE WriteString(uint8_t** const Out, uint8_t* const String)
{
	size_t i;
	
	// Loop
	for (i = 0; String[i]; i++)
		WriteUInt8(Out, (uint8_t)String[i]);
	WriteUInt8(Out, 0);
}

/* WriteStringN() -- Write a string of n length */
static inline void __REMOOD_FORCEINLINE WriteStringN(uint8_t** const Out, uint8_t* const String, const size_t Count)
{
	size_t i;
	
	// Loop
	for (i = 0; i < Count && String[i]; i++)
		WriteUInt8(Out, (uint8_t)String[i]);
	for (; i < Count; i++)
		WriteUInt8(Out, 0);
}

/********************
*** BYTE SWAPPING ***
********************/

/* SwapUInt16() -- Swap 16-bits */
static inline uint16_t __REMOOD_FORCEINLINE SwapUInt16(const uint16_t In)
{
	return ((In & 0xFFU) << 8) | ((In & 0xFF00U) >> 8);
}

/* SwapUInt32() -- Swap 32-bits */
static inline uint32_t __REMOOD_FORCEINLINE SwapUInt32(const uint32_t In)
{
	return ((In & 0xFFU) << 24) | ((In & 0xFF00U) << 8) | ((In & 0xFF0000U) >> 8) | ((In & 0xFF000000U) >> 24);
}

/* SwapUInt64() -- Swap 64-bits */
static inline uint64_t __REMOOD_FORCEINLINE SwapUInt64(const uint64_t In)
{
	return (((In >> 56)) |
		((In >> 40) & 0x000000000000FF00LL) |
		((In >> 24) & 0x0000000000FF0000LL) |
		((In >> 8 ) & 0x00000000FF000000LL) |
		((In << 8 ) & 0x000000FF00000000LL) |
		((In << 24) & 0x0000FF0000000000LL) |
		((In << 40) & 0x00FF000000000000LL) |
		((In << 56) & 0xFF00000000000000LL));
}

/* SwapInt16() -- Swap 16-bits */
static inline int16_t __REMOOD_FORCEINLINE SwapInt16(const int16_t In)
{
	return (int16_t)SwapUInt16((uint16_t)In);
}

/* SwapInt32() -- Swap 32-bits */
static inline int32_t __REMOOD_FORCEINLINE SwapInt32(const int32_t In)
{
	return (int32_t)SwapUInt32((uint32_t)In);
}

/* SwapInt64() -- Swap 64-bits */
static inline int32_t __REMOOD_FORCEINLINE SwapInt64(const int64_t In)
{
	return (int64_t)SwapUInt64((uint64_t)In);
}

/* Little swapping */
#if defined(__BIG_ENDIAN__)
	#define LS_x(w,x) static inline x __REMOOD_FORCEINLINE BP_MERGE(LittleSwap,w)(const x In)\
	{\
		return BP_MERGE(Swap,w)(In);\
	}
#else
	#define LS_x(w,x) static inline x __REMOOD_FORCEINLINE BP_MERGE(LittleSwap,w)(const x In)\
	{\
		return In;\
	}
#endif

LS_x(Int16,int16_t)
LS_x(UInt16,uint16_t)
LS_x(Int32,int32_t)
LS_x(UInt32,uint32_t)
LS_x(Int64,int64_t)
LS_x(UInt64,uint64_t)

#undef LS_x

/* Big swapping */
#if defined(__BIG_ENDIAN__)
	#define BS_x(w,x) static inline x __REMOOD_FORCEINLINE BP_MERGE(BigSwap,w)(const x In)\
	{\
		return In;\
	}
#else
	#define BS_x(w,x) static inline x __REMOOD_FORCEINLINE BP_MERGE(BigSwap,w)(const x In)\
	{\
		return BP_MERGE(Swap,w)(In);\
	}
#endif

BS_x(Int16,int16_t)
BS_x(UInt16,uint16_t)
BS_x(Int32,int32_t)
BS_x(UInt32,uint32_t)
BS_x(Int64,int64_t)
BS_x(UInt64,uint64_t)

#undef BS_x

/* End */
#undef BP_MERGE

/******************************************
*** REMOVE ALL THIS GARBAGE, SERIOUSLY! ***
******************************************/

#ifdef _MSC_VER
#ifndef __ssize_t_defined
#if defined(_M_IA64) || defined(_M_X64) || defined(_WIN64)
typedef int64_t ssize_t;
#else
typedef int32_t ssize_t;
#endif
#define __ssize_t_defined
#endif
#endif

#if (_MSC_VER > 1200) || !defined(_MSC_VER)
typedef uint8_t byte;
#endif

typedef uint32_t tic_t;
#ifndef _WIN32
typedef int32_t boolean;
#else
#ifndef boolean
#define boolean BOOL
#endif
#endif

// Win32 :(
#define ULONG uint32_t
#define USHORT uint16_t

// Legacy Compat
#define INT64 uint64_t

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
#define MAXCHAR   ((int8_t)0x7f)
#endif

#ifndef MAXSHORT
#define MAXSHORT  ((int16_t)0x7fff)
#endif

#ifndef MAXINT
#define MAXINT    ((int32_t)0x7fffffff)
#endif

#ifndef MAXLONG
#define MAXLONG	MAXINT			// ((long)0x7fffffff)
#endif

#ifndef MAXINT64
#define MAXINT64   ((int64_t)0x7fffffffffffffffL)
#endif

#ifndef MINCHAR
#define MINCHAR   ((int8_t)0x80)
#endif

#ifndef MINSHORT
#define MINSHORT  ((int16_t)0x8000)
#endif

#ifndef MININT
#define MININT    ((int32_t)0x80000000)
#endif

#ifndef MINLONG
#define MINLONG   MININT		//((long)0x80000000)
#endif

#ifndef MININT64
#define MININT64   ((int64_t)0x8000000000000000L)
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

#if defined(__GNUC__)
	#define ATTRIB_FORCEINLINE	__attribute__((always_inline))
	#define ATTRIB_HOT			__attribute__((hot))
	#define ATTRIB_COLD			__attribute__((cold))
	#define ATTRIB_UNUSED		__attribute__((unused))
#else
	#define ATTRIB_FORCEINLINE
	#define ATTRIB_HOT
	#define ATTRIB_COLD
	#define ATTRIB_UNUSED
#endif

#endif /* __DOOMTYPE_H__ */

