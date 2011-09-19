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
#define _WIN32_IE 0x0400
#endif

// Lean and mean!
#define WIN32_LEAN_AND_MEAN

// Now include
#include <windows.h>
#include <io.h>
#endif

/* UNIX */
#if defined(__unix__)
#include <unistd.h>
#endif

/* DJGPP */
#if defined(__REMOOD_SYSTEM_DOS) && defined(__DJGPP__)
#define snprintf g_snprintf
#define vsnprintf g_vsnprintf
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
#define __REMOOD_BIG_ENDIAN
#elif defined(__BYTE_ORDER) && (__BYTE_ORDER == __BIG_ENDIAN)
#define __REMOOD_BIG_ENDIAN
#endif
#endif

// Known Big endian systems
#if !defined(__REMOOD_BIG_ENDIAN)
#if defined(__MIPSEB__)
#define __REMOOD_BIG_ENDIAN
#endif
#endif

// Otherwise it is little
#if !defined(__REMOOD_LITTLE_ENDIAN)
#define __REMOOD_LITTLE_ENDIAN
#endif
#endif

/***********************
*** FIXED SIZE TYPES ***
***********************/

#if !defined(__REMOOD_IGNORE_FIXEDTYPES)

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
#endif

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

BP_READ(Int8, int8_t)
BP_READ(Int16, int16_t)
BP_READ(Int32, int32_t) BP_READ(Int64, int64_t) BP_READ(UInt8, uint8_t) BP_READ(UInt16, uint16_t) BP_READ(UInt32, uint32_t) BP_READ(UInt64, uint64_t)
#define BP_WRITE(w,x) static inline void __REMOOD_FORCEINLINE BP_MERGE(Write,w)(x** const Ptr, const x Val)\
{\
	if (!Ptr || !(*Ptr))\
		return;\
	**Ptr = Val;\
	(*Ptr)++;\
}
BP_WRITE(Int8, int8_t)
BP_WRITE(Int16, int16_t)
BP_WRITE(Int32, int32_t) BP_WRITE(Int64, int64_t) BP_WRITE(UInt8, uint8_t) BP_WRITE(UInt16, uint16_t) BP_WRITE(UInt32, uint32_t) BP_WRITE(UInt64, uint64_t)
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
static void __REMOOD_FORCEINLINE WriteStringN(uint8_t** const Out, uint8_t* const String, const size_t Count)
{
	size_t i;
	
	// Loop
	for (i = 0; i < Count && String[i]; i++)
		WriteUInt8(Out, (uint8_t)String[i]);
	for (; i < Count; i++)
		WriteUInt8(Out, 0);
}

/* ReadCompressedUInt16() -- Reads a "compressed" uint16_t */
// Numerical Range: 0 - 32767
static __REMOOD_FORCEINLINE uint16_t ReadCompressedUInt16(const void** const p)
{
	uint16_t ReadVal;
	
	/* Check */
	if (!p || !*p)
		return 0;
		
	/* Read in first value */
	ReadVal = ReadUInt8((const uint8_t** const)p);
	
	// Compressed?
	if (ReadVal & 0x80U)
	{
		ReadVal &= 0x7FU;		// Don't remember upper bit
		ReadVal <<= 8;
		ReadVal |= ReadUInt8((const uint8_t** const)p);
	}
	
	/* Return result */
	return ReadVal;
}

/* WriteCompressedUInt16() -- Writes a "compressed" uint16_t */
// Numerical Range: 0 - 32767
static __REMOOD_FORCEINLINE void WriteCompressedUInt16(void** const p, const uint16_t Value)
{
	/* Check */
	if (!p || !*p)
		return;
		
	/* Write in high value */
	if (Value > 0x7FU)
		WriteUInt8((uint8_t** const)p, ((Value & 0x7F00) >> 8) | 0x80U);
		
	/* Write in low value */
	WriteUInt8((uint8_t** const)p, Value & 0xFFU);
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
	        ((In >> 8) & 0x00000000FF000000LL) |
	        ((In << 8) & 0x000000FF00000000LL) |
	        ((In << 24) & 0x0000FF0000000000LL) | ((In << 40) & 0x00FF000000000000LL) | ((In << 56) & 0xFF00000000000000LL));
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
#if defined(__REMOOD_BIG_ENDIAN)
#define LS_x(w,x) static __REMOOD_INLINE x __REMOOD_FORCEINLINE BP_MERGE(LittleSwap,w)(const x In)\
	{\
		return BP_MERGE(Swap,w)(In);\
	}
#else
#define LS_x(w,x) static __REMOOD_INLINE x __REMOOD_FORCEINLINE BP_MERGE(LittleSwap,w)(const x In)\
	{\
		return In;\
	}
#endif

LS_x(Int16, int16_t) LS_x(UInt16, uint16_t) LS_x(Int32, int32_t) LS_x(UInt32, uint32_t) LS_x(Int64, int64_t) LS_x(UInt64, uint64_t)
#undef LS_x

/* Big swapping */
#if defined(__REMOOD_BIG_ENDIAN)
#define BS_x(w,x) static __REMOOD_INLINE x __REMOOD_FORCEINLINE BP_MERGE(BigSwap,w)(const x In)\
	{\
		return In;\
	}
#else
#define BS_x(w,x) static __REMOOD_INLINE x __REMOOD_FORCEINLINE BP_MERGE(BigSwap,w)(const x In)\
	{\
		return BP_MERGE(Swap,w)(In);\
	}
#endif
BS_x(Int16, int16_t) BS_x(UInt16, uint16_t) BS_x(Int32, int32_t) BS_x(UInt32, uint32_t) BS_x(Int64, int64_t) BS_x(UInt64, uint64_t)
#undef BS_x

/*** Reading/Writing Little Endian Data ***/
#if defined(__REMOOD_BIG_ENDIAN)
#define BPLREAD_x(w,x) static __REMOOD_INLINE x __REMOOD_FORCEINLINE BP_MERGE(LittleRead,w)(const x** const Ptr)\
	{\
		return BP_MERGE(Swap,w)(BP_MERGE(Read,w)(Ptr));\
	}
#else
#define BPLREAD_x(w,x) static __REMOOD_INLINE x __REMOOD_FORCEINLINE BP_MERGE(LittleRead,w)(const x** const Ptr)\
	{\
		return BP_MERGE(Read,w)(Ptr);\
	}
#endif
#if defined(__REMOOD_BIG_ENDIAN)
#define BPLWRITE_x(w,x) static __REMOOD_INLINE void __REMOOD_FORCEINLINE BP_MERGE(LittleWrite,w)(x** const Ptr, const x Val)\
	{\
		BP_MERGE(Write,w)(Ptr, BP_MERGE(Swap,w)(Val));\
	}
#else
#define BPLWRITE_x(w,x) static __REMOOD_INLINE void __REMOOD_FORCEINLINE BP_MERGE(LittleWrite,w)(x** const Ptr, const x Val)\
	{\
		BP_MERGE(Write,w)(Ptr, Val);\
	}
#endif
BPLREAD_x(Int16, int16_t)
BPLREAD_x(UInt16, uint16_t)
BPLREAD_x(Int32, int32_t)
BPLREAD_x(UInt32, uint32_t)
BPLREAD_x(Int64, int64_t)
BPLREAD_x(UInt64, uint64_t)
BPLWRITE_x(Int16, int16_t)
BPLWRITE_x(UInt16, uint16_t) BPLWRITE_x(Int32, int32_t) BPLWRITE_x(UInt32, uint32_t) BPLWRITE_x(Int64, int64_t) BPLWRITE_x(UInt64, uint64_t)
#undef BPLREAD_x
#undef BPLWRITE_x

/* End */
#undef BP_MERGE

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

#endif							/* __DOOMTYPE_H__ */
