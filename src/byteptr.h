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
// Project Leader:    GhostlyDeath           (ghostlydeath@remood.org)
// Project Co-Leader: RedZTag                (jostol@remood.org)
// Members:           TetrisMaster512        (tetrismaster512@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 2008-2010 The ReMooD Team..
// Copyright (C) 2007-2010 GhostlyDeath (ghostlydeath@remood.org)
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
// DESCRIPTION:  Macro to read/write from/to a char*, used for packet cration and such...

#ifndef __BYTEPTR_H__
#define __BYTEPTR_H__

#include "doomtype.h"
#include "doomdef.h"

Int8 ReadInt8(Int8** const Ptr);
Int16 ReadInt16(Int16** const Ptr);
Int32 ReadInt32(Int32** const Ptr);
UInt8 ReadUInt8(UInt8** const Ptr);
UInt16 ReadUInt16(UInt16** const Ptr);
UInt32 ReadUInt32(UInt32** const Ptr);
wchar_t Readwchar_t(wchar_t** const Ptr);
void ReadStringN(char** const Ptr, char* Dest, size_t n);

void WriteInt8(Int8** const Ptr, const Int8 Val);
void WriteInt16(Int16** const Ptr, const Int16 Val);
void WriteInt32(Int32** const Ptr, const Int32 Val);
void WriteUInt8(UInt8** const Ptr, const UInt8 Val);
void WriteUInt16(UInt16** const Ptr, const UInt16 Val);
void WriteUInt32(UInt32** const Ptr, const UInt32 Val);
void Writewchar_t(wchar_t** const Ptr, const wchar_t Val);
void WriteString(char** const Ptr, const char* Val);
void WriteStringN(char** const Ptr, const char* Val, const size_t n);


#if !defined(_REMOOD_NOINT64)
	Int64 ReadInt64(Int64** const Ptr);
	UInt64 ReadUInt64(UInt64** const Ptr);
	void WriteInt64(Int64** const Ptr, const Int64 Val);
	void WriteUInt64(UInt64** const Ptr, const UInt64 Val);
#endif

#if defined(_REMOOD_BIG_ENDIAN)
	#define writeshort(p,b)     *(Int16*)  (p)   = LITTLESWAP32(b)
	#define writelong(p,b)      *(Int32 *)  (p)   = LITTLESWAP32(b)

	#define WRITECHAR(p,b)		WriteInt8((Int8**)&(p), (Int8)(b))
	#define WRITEBYTE(p,b)		WriteUInt8((UInt8**)&(p), (UInt8)(b))
	#define WRITESHORT(p,b)		WriteInt16((Int16**)&(p), (Int16)(LITTLESWAP16(b)))
	#define WRITEUSHORT(p,b)	WriteUInt16((UInt16**)&(p), (UInt16)(LITTLESWAP16(b)))
	#define WRITELONG(p,b)		WriteInt32((Int32**)&(p), (Int32)(LITTLESWAP32(b)))
	#define WRITEULONG(p,b)		WriteUInt32((UInt32**)&(p), (UInt32)(LITTLESWAP32(b)))
	#define WRITEFIXED(p,b)		WriteInt32((Int32**)&(p), (Int32)(LITTLESWAP32(b)))
	#define WRITEANGLE(p,b)		WriteUInt32((UInt32**)&(p), (UInt32)(LITTLESWAP32(b)))

	#define readshort(p)	    LITTLESWAP32(*((Int16  *)(p)))
	#define readlong(p)	  		LITTLESWAP32(*((Int32   *)(p)))

	#define READCHAR(p)			ReadInt8((Int8**)(&(p)))
	#define READBYTE(p)			ReadUInt8((UInt8**)(&(p)))
	#define READSHORT(p)		LITTLESWAP16(ReadInt16((Int16**)(&(p))))
	#define READUSHORT(p)		LITTLESWAP16(ReadUInt16((UInt16**)(&(p))))
	#define READLONG(p)			LITTLESWAP32(ReadInt32((Int32**)(&(p))))
	#define	READULONG(p)		LITTLESWAP32(ReadUInt32((UInt32**)(&(p))))
	#define READFIXED(p)		LITTLESWAP32(ReadInt32((Int32**)(&(p))))
	#define READANGLE(p)		LITTLESWAP32(ReadUInt32((UInt32**)(&(p))))
#else
	#define writeshort(p,b)     *(Int16*)  (p)   = b
	#define writelong(p,b)      *(Int32 *)  (p)   = b

	#define WRITECHAR(p,b)		WriteInt8((Int8**)&(p), (Int8)(b))
	#define WRITEBYTE(p,b)		WriteUInt8((UInt8**)&(p), (UInt8)(b))
	#define WRITESHORT(p,b)		WriteInt16((Int16**)&(p), (Int16)(b))
	#define WRITEUSHORT(p,b)	WriteUInt16((UInt16**)&(p), (UInt16)(b))
	#define WRITELONG(p,b)		WriteInt32((Int32**)&(p), (Int32)(b))
	#define WRITEULONG(p,b)		WriteUInt32((UInt32**)&(p), (UInt32)(b))
	#define WRITEFIXED(p,b)		WriteInt32((Int32**)&(p), (Int32)(b))
	#define WRITEANGLE(p,b)		WriteUInt32((UInt32**)&(p), (UInt32)(b))

	#define readshort(p)	    *((Int16  *)(p))
	#define readlong(p)	  		  *((Int32   *)(p))

	#define READCHAR(p)			ReadInt8((Int8**)(&(p)))
	#define READBYTE(p)			ReadUInt8((UInt8**)(&(p)))
	#define READSHORT(p)		ReadInt16((Int16**)(&(p)))
	#define READUSHORT(p)		ReadUInt16((UInt16**)(&(p)))
	#define READLONG(p)			ReadInt32((Int32**)(&(p)))
	#define	READULONG(p)		ReadUInt32((UInt32**)(&(p)))
	#define READFIXED(p)		ReadInt32((Int32**)(&(p)))
	#define READANGLE(p)		ReadUInt32((UInt32**)(&(p)))
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
#define SHORT(x)	((Int16)LITTLESWAP16((Int16)(x)))
#define LONG(x)		((Int32)LITTLESWAP32((Int32)(x)))
#define SHORTU(x)	((UInt16)LITTLESWAP16((UInt16)(x)))
#define LONGU(x)	((UInt32)LITTLESWAP32((UInt32)(x)))
#define SIZET(x)	((size_t)((sizeof(size_t) == 8 ? (LITTLESWAP64((size_t)(x))) : (LITTLESWAP32((size_t)(x))))))

#if defined(_REMOOD_BIG_ENDIAN)
	#define SwapSHORT(n)	SWAP16((UInt16)(n))
	#define SwapLONG(n)		SWAP32((UInt32)(n))
#endif

#endif							/* __BYTEPTR_H__ */

