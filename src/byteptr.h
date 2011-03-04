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
// Copyright (C) 2008-2011 GhostlyDeath (ghostlydeath@gmail.com)
// Copyright (C) 2007-2011 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION:  Macro to read/write from/to a char*, used for packet cration and such...

#ifndef __BYTEPTR_H__
#define __BYTEPTR_H__

/*****************************************************************************/

/***************
*** INCLUDES ***
****************/

#include "doomtype.h"
#include "doomdef.h"

// Everything moved to doomtype.h!

/**************************************
*** REMOVE THIS GARBAGE, SERIOUSLY! ***
**************************************/

#if defined(_REMOOD_BIG_ENDIAN)
	#define writeshort(p,b)     *(int16_t*)  (p)   = LITTLESWAP32(b)
	#define writelong(p,b)      *(int32_t *)  (p)   = LITTLESWAP32(b)

	#define WRITECHAR(p,b)		WriteInt8((int8_t**)&(p), (int8_t)(b))
	#define WRITEBYTE(p,b)		WriteUInt8((uint8_t**)&(p), (uint8_t)(b))
	#define WRITESHORT(p,b)		WriteInt16((int16_t**)&(p), (int16_t)(LITTLESWAP16(b)))
	#define WRITEUSHORT(p,b)	WriteUInt16((uint16_t**)&(p), (uint16_t)(LITTLESWAP16(b)))
	#define WRITELONG(p,b)		WriteInt32((int32_t**)&(p), (int32_t)(LITTLESWAP32(b)))
	#define WRITEULONG(p,b)		WriteUInt32((uint32_t**)&(p), (uint32_t)(LITTLESWAP32(b)))
	#define WRITEFIXED(p,b)		WriteInt32((int32_t**)&(p), (int32_t)(LITTLESWAP32(b)))
	#define WRITEANGLE(p,b)		WriteUInt32((uint32_t**)&(p), (uint32_t)(LITTLESWAP32(b)))

	#define readshort(p)	    LITTLESWAP32(*((int16_t  *)(p)))
	#define readlong(p)	  		LITTLESWAP32(*((int32_t   *)(p)))

	#define READCHAR(p)			ReadInt8((int8_t**)(&(p)))
	#define READBYTE(p)			ReadUInt8((uint8_t**)(&(p)))
	#define READSHORT(p)		LITTLESWAP16(ReadInt16((int16_t**)(&(p))))
	#define READUSHORT(p)		LITTLESWAP16(ReadUInt16((uint16_t**)(&(p))))
	#define READLONG(p)			LITTLESWAP32(ReadInt32((int32_t**)(&(p))))
	#define	READULONG(p)		LITTLESWAP32(ReadUInt32((uint32_t**)(&(p))))
	#define READFIXED(p)		LITTLESWAP32(ReadInt32((int32_t**)(&(p))))
	#define READANGLE(p)		LITTLESWAP32(ReadUInt32((uint32_t**)(&(p))))
#else
	#define writeshort(p,b)     *(int16_t*)  (p)   = b
	#define writelong(p,b)      *(int32_t *)  (p)   = b

	#define WRITECHAR(p,b)		WriteInt8((int8_t**)&(p), (int8_t)(b))
	#define WRITEBYTE(p,b)		WriteUInt8((uint8_t**)&(p), (uint8_t)(b))
	#define WRITESHORT(p,b)		WriteInt16((int16_t**)&(p), (int16_t)(b))
	#define WRITEUSHORT(p,b)	WriteUInt16((uint16_t**)&(p), (uint16_t)(b))
	#define WRITELONG(p,b)		WriteInt32((int32_t**)&(p), (int32_t)(b))
	#define WRITEULONG(p,b)		WriteUInt32((uint32_t**)&(p), (uint32_t)(b))
	#define WRITEFIXED(p,b)		WriteInt32((int32_t**)&(p), (int32_t)(b))
	#define WRITEANGLE(p,b)		WriteUInt32((uint32_t**)&(p), (uint32_t)(b))

	#define readshort(p)	    *((int16_t  *)(p))
	#define readlong(p)	  		  *((int32_t   *)(p))

	#define READCHAR(p)			ReadInt8((int8_t**)(&(p)))
	#define READBYTE(p)			ReadUInt8((uint8_t**)(&(p)))
	#define READSHORT(p)		ReadInt16((int16_t**)(&(p)))
	#define READUSHORT(p)		ReadUInt16((uint16_t**)(&(p)))
	#define READLONG(p)			ReadInt32((int32_t**)(&(p)))
	#define	READULONG(p)		ReadUInt32((uint32_t**)(&(p)))
	#define READFIXED(p)		ReadInt32((int32_t**)(&(p)))
	#define READANGLE(p)		ReadUInt32((uint32_t**)(&(p)))
#endif

#define WRITESTRING(p,b)    { int tmp_i=0; do { WRITECHAR(p,b[tmp_i]); } while(b[tmp_i++]); }
#define WRITESTRINGN(p,b,n) { int tmp_i=0; do { WRITECHAR(p,b[tmp_i]); if(!b[tmp_i]) break;tmp_i++; } while(tmp_i<n); }
#define WRITEMEM(p,s,n)     memcpy(p, s, n);p+=n

#define READSTRING(p,s)     { int tmp_i=0; do { s[tmp_i]=READBYTE(p);  } while(s[tmp_i++]); }
#define SKIPSTRING(p)       while(READBYTE(p))
#define READMEM(p,s,n)      memcpy(s, p, n);p+=n

/*********************************
*** BIG ENDIAN, LITTLE ENDIAN? ***
*********************************/

#if defined(__GNUC__)
	#if defined(__linux__)
		#include <endian.h>
	#endif

	/* MUST ALWAYS DO THIS! */
	// Confusing GCC Stuff...
	#if !defined(_REMOOD_BIG_ENDIAN) && !defined(_REMOOD_LITTLE_ENDIAN)
		#if defined(__BYTE_ORDER) && (__BYTE_ORDER == __BIG_ENDIAN)
			#define _REMOOD_BIG_ENDIAN 1
		#elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __BIG_ENDIAN__)
			#define _REMOOD_BIG_ENDIAN 1
		#endif
	#endif
#endif

/**************************
*** BYTE SWAPPING STUFF ***
**************************/

/*** NEW BYTE SWAPPING ***/
#define SWAP16(n) (((((n) & 0xFFFF) << 8) | (((n) & 0xFFFF) >> 8)) & 0xFFFF)
#define SWAP32(n) ((((((n) & 0xFFFFFFFF) >> 24)) | \
			((((n) & 0xFFFFFFFF) >> 8)  & 0x0000FF00) | \
			((((n) & 0xFFFFFFFF) << 8)  & 0x00FF0000) | \
			((((n) & 0xFFFFFFFF) << 24) & 0xFF000000)) & 0xFFFFFFFF)
#if !defined(_REMOOD_NOINT64)
	#define SWAP64(n) ((((((n) & 0xFFFFFFFFFFFFFFFFLL) >> 56)) | \
				((((n) & 0xFFFFFFFFFFFFFFFFLL) >> 40) & 0x000000000000FF00LL) | \
				((((n) & 0xFFFFFFFFFFFFFFFFLL) >> 24) & 0x0000000000FF0000LL) | \
				((((n) & 0xFFFFFFFFFFFFFFFFLL) >> 8 ) & 0x00000000FF000000LL) | \
				((((n) & 0xFFFFFFFFFFFFFFFFLL) << 8 ) & 0x000000FF00000000LL) | \
				((((n) & 0xFFFFFFFFFFFFFFFFLL) << 24) & 0x0000FF0000000000LL) | \
				((((n) & 0xFFFFFFFFFFFFFFFFLL) << 40) & 0x00FF000000000000LL) | \
				((((n) & 0xFFFFFFFFFFFFFFFFLL) << 56) & 0xFF00000000000000LL)) & 0xFFFFFFFFFFFFFFFFLL)
#endif

#if defined(_REMOOD_BIG_ENDIAN)
	#define BIGSWAP16(n) (n)
	#define BIGSWAP32(n) (n)
	#define LITTLESWAP16(n) SWAP16((n))
	#define LITTLESWAP32(n) SWAP32((n))
	
	#if !defined(_REMOOD_NOINT64)
		#define BIGSWAP64(n) (n)
		#define LITTLESWAP64(n) SWAP64((n))
	#endif
#else
	#define BIGSWAP16(n) SWAP16((n))
	#define BIGSWAP32(n) SWAP32((n))
	#define LITTLESWAP16(n) (n)
	#define LITTLESWAP32(n) (n)
	
	#if !defined(_REMOOD_NOINT64)
		#define BIGSWAP64(n) SWAP64((n))
		#define LITTLESWAP64(n) (n)
	#endif
#endif

/*** DEPRECATED BYTE SWAPPING ***/
#define SHORT(x)	(x)//((int16_t)LITTLESWAP16((int16_t)(x)))
#define LONG(x)		(x)//((int32_t)LITTLESWAP32((int32_t)(x)))
#define SHORTU(x)	(x)//((uint16_t)LITTLESWAP16((uint16_t)(x)))
#define LONGU(x)	(x)//((uint32_t)LITTLESWAP32((uint32_t)(x)))
#define SIZET(x)	(x)//((size_t)((sizeof(size_t) == 8 ? (LITTLESWAP64((size_t)(x))) : (LITTLESWAP32((size_t)(x))))))

#if defined(_REMOOD_BIG_ENDIAN)
	#define SwapSHORT(n)	(n)//SWAP16((uint16_t)(n))
	#define SwapLONG(n)		(n)//SWAP32((uint32_t)(n))
#endif

/*****************************************************************************/

#endif							/* __BYTEPTR_H__ */

