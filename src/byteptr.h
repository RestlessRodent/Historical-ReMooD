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

#if defined(__REMOOD_BIG_ENDIAN)
	#define writeshort(p,b)     *(int16_t*)  (p)   = LittleSwapUInt32(b)
	#define writelong(p,b)      *(int32_t *)  (p)   = LittleSwapUInt32(b)

	#define WRITECHAR(p,b)		WriteInt8((int8_t**)&(p), (int8_t)(b))
	#define WRITEBYTE(p,b)		WriteUInt8((uint8_t**)&(p), (uint8_t)(b))
	#define WRITESHORT(p,b)		WriteInt16((int16_t**)&(p), (int16_t)(LittleSwapUInt16(b)))
	#define WRITEUSHORT(p,b)	WriteUInt16((uint16_t**)&(p), (uint16_t)(LittleSwapUInt16(b)))
	#define WRITELONG(p,b)		WriteInt32((int32_t**)&(p), (int32_t)(LittleSwapUInt32(b)))
	#define WRITEULONG(p,b)		WriteUInt32((uint32_t**)&(p), (uint32_t)(LittleSwapUInt32(b)))
	#define WRITEFIXED(p,b)		WriteInt32((int32_t**)&(p), (int32_t)(LittleSwapUInt32(b)))
	#define WRITEANGLE(p,b)		WriteUInt32((uint32_t**)&(p), (uint32_t)(LittleSwapUInt32(b)))

	#define readshort(p)	    LittleSwapUInt32(*((int16_t  *)(p)))
	#define readlong(p)	  		LittleSwapUInt32(*((int32_t   *)(p)))

	#define READCHAR(p)			ReadInt8((int8_t**)(&(p)))
	#define READBYTE(p)			ReadUInt8((uint8_t**)(&(p)))
	#define READSHORT(p)		LittleSwapUInt16(ReadInt16((int16_t**)(&(p))))
	#define READUSHORT(p)		LittleSwapUInt16(ReadUInt16((uint16_t**)(&(p))))
	#define READLONG(p)			LittleSwapUInt32(ReadInt32((int32_t**)(&(p))))
	#define	READULONG(p)		LittleSwapUInt32(ReadUInt32((uint32_t**)(&(p))))
	#define READFIXED(p)		LittleSwapUInt32(ReadInt32((int32_t**)(&(p))))
	#define READANGLE(p)		LittleSwapUInt32(ReadUInt32((uint32_t**)(&(p))))
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

/*****************************************************************************/

#endif							/* __BYTEPTR_H__ */

