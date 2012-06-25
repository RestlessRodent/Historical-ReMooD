// -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
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
// DESCRIPTION: Missing C Stuff

#ifndef __C_LIB_H__
#define __C_LIB_H__

/******************************************************************************/

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"

/*************
*** MACROS ***
*************/

#define OLDReadInt8(w) ReadInt8R((const int8_t** const)(w))
#define OLDReadInt16(w) ReadInt16R((const int16_t** const)(w))
#define OLDReadInt32(w) ReadInt32R((const int32_t** const)(w))
#define OLDReadInt64(w) ReadInt64R((const int64_t** const)(w))
#define OLDReadUInt8(w) ReadUInt8R((const uint8_t** const)(w))
#define OLDReadUInt16(w) ReadUInt16R((const uint16_t** const)(w))
#define OLDReadUInt32(w) ReadUInt32R((const uint32_t** const)(w))
#define OLDReadUInt64(w) ReadUInt64R((const uint64_t** const)(w))

#define OLDWriteInt8(w,x) WriteInt8R((int8_t** const)(w), (x))
#define OLDWriteInt16(w,x) WriteInt16R((int16_t** const)(w), (x))
#define OLDWriteInt32(w,x) WriteInt32R((int32_t** const)(w), (x))
#define OLDWriteInt64(w,x) WriteInt64R((int64_t** const)(w), (x))
#define OLDWriteUInt8(w,x) WriteUInt8R((uint8_t** const)(w), (x))
#define OLDWriteUInt16(w,x) WriteUInt16R((uint16_t** const)(w), (x))
#define OLDWriteUInt32(w,x) WriteUInt32R((uint32_t** const)(w), (x))
#define OLDWriteUInt64(w,x) WriteUInt64R((uint64_t** const)(w), (x))

/*****************
*** PROTOTYPES ***
*****************/

char* C_strupr(char* s);
char* C_strlwr(char* s);

/*** Byte Operations ***/
#define __REMOOD_MACROMERGE(a,b) a##b

#define __REMOOD_READMACRO(w,x) x __REMOOD_MACROMERGE(Read,w)(const x** const Ptr)
#define __REMOOD_WRITEMACRO(w,x) void __REMOOD_MACROMERGE(Write,w)(x** const Ptr, const x Val)

#define __REMOOD_LITTLESWAPMACRO(w,x) x __REMOOD_MACROMERGE(LittleSwap,w)(const x In)
#define __REMOOD_BIGSWAPMACRO(w,x) x __REMOOD_MACROMERGE(BigSwap,w)(const x In)

#define __REMOOD_LITTLEREADMACRO(w,x) x __REMOOD_MACROMERGE(LittleRead,w)(const x** const Ptr)
#define __REMOOD_LITTLEWRITEMACRO(w,x) void __REMOOD_MACROMERGE(LittleWrite,w)(x** const Ptr, const x Val)

// Read
__REMOOD_READMACRO(Int8R, int8_t);
__REMOOD_READMACRO(Int16R, int16_t);
__REMOOD_READMACRO(Int32R, int32_t);
__REMOOD_READMACRO(Int64R, int64_t);
__REMOOD_READMACRO(UInt8R, uint8_t);
__REMOOD_READMACRO(UInt16R, uint16_t);
__REMOOD_READMACRO(UInt32R, uint32_t);
__REMOOD_READMACRO(UInt64R, uint64_t);

// Write
__REMOOD_WRITEMACRO(Int8R, int8_t);
__REMOOD_WRITEMACRO(Int16R, int16_t);
__REMOOD_WRITEMACRO(Int32R, int32_t);
__REMOOD_WRITEMACRO(Int64R, int64_t);
__REMOOD_WRITEMACRO(UInt8R, uint8_t);
__REMOOD_WRITEMACRO(UInt16R, uint16_t);
__REMOOD_WRITEMACRO(UInt32R, uint32_t);
__REMOOD_WRITEMACRO(UInt64R, uint64_t);

void WriteString(uint8_t** const Out, uint8_t* const String);
void WriteStringN(uint8_t** const Out, uint8_t* const String, const size_t Count);

// Compressed Hacks
uint16_t ReadCompressedUInt16(const void** const p);
void WriteCompressedUInt16(void** const p, const uint16_t Value);

// Swapping
uint16_t SwapUInt16(const uint16_t In);
uint32_t SwapUInt32(const uint32_t In);
uint64_t SwapUInt64(const uint64_t In);
int16_t SwapInt16(const int16_t In);
int32_t SwapInt32(const int32_t In);
int64_t SwapInt64(const int64_t In);


__REMOOD_LITTLESWAPMACRO(Int16, int16_t);
__REMOOD_LITTLESWAPMACRO(UInt16, uint16_t);
__REMOOD_LITTLESWAPMACRO(Int32, int32_t);
__REMOOD_LITTLESWAPMACRO(UInt32, uint32_t);
__REMOOD_LITTLESWAPMACRO(Int64, int64_t);
__REMOOD_LITTLESWAPMACRO(UInt64, uint64_t);

__REMOOD_BIGSWAPMACRO(Int16, int16_t);
__REMOOD_BIGSWAPMACRO(UInt16, uint16_t);
__REMOOD_BIGSWAPMACRO(Int32, int32_t);
__REMOOD_BIGSWAPMACRO(UInt32, uint32_t);
__REMOOD_BIGSWAPMACRO(Int64, int64_t);
__REMOOD_BIGSWAPMACRO(UInt64, uint64_t);


__REMOOD_LITTLEREADMACRO(Int16, int16_t);
__REMOOD_LITTLEREADMACRO(UInt16, uint16_t);
__REMOOD_LITTLEREADMACRO(Int32, int32_t);
__REMOOD_LITTLEREADMACRO(UInt32, uint32_t);
__REMOOD_LITTLEREADMACRO(Int64, int64_t);
__REMOOD_LITTLEREADMACRO(UInt64, uint64_t);
__REMOOD_LITTLEWRITEMACRO(Int16, int16_t);
__REMOOD_LITTLEWRITEMACRO(UInt16, uint16_t);
__REMOOD_LITTLEWRITEMACRO(Int32, int32_t);
__REMOOD_LITTLEWRITEMACRO(UInt32, uint32_t);
__REMOOD_LITTLEWRITEMACRO(Int64, int64_t);
__REMOOD_LITTLEWRITEMACRO(UInt64, uint64_t);

// Remove Macros
#undef __REMOOD_MACROMERGE
#undef __REMOOD_READMACRO
#undef __REMOOD_WRITEMACRO
#undef __REMOOD_LITTLESWAPMACRO
#undef __REMOOD_BIGSWAPMACRO
#undef __REMOOD_LITTLEREADMACRO
#undef __REMOOD_LITTLEWRITEMACRO

/******************************************************************************/

#endif							/* __C_LIB_H__ */
