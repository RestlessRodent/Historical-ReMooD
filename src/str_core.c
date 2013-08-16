// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------
// Copyright (C) 2013-2013 GhostlyDeath <ghostlydeath@remood.org>
//                                      <ghostlydeath@gmail.com>
// ----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// ----------------------------------------------------------------------------
// DESCRIPTION: Stream Reader/Writer

/***************
*** INCLUDES ***
***************/

#include "str.h"
#include "w_wad.h"

/*****************
*** STRUCTURES ***
*****************/

/* STR_t -- Stream */
struct STR_s
{
	STR_Type_t Type;							// Type of stream
	STR_Class_t Class;							// Class of stream
};

/****************
*** FUNCTIONS ***
****************/

/* STR_Open() -- Opens stream */
STR_t* STR_Open(const STR_Type_t a_Type, const STR_Class_t a_Class)
{
	/* Check */
	if (a_Type < 0 || a_Type >= NUMSTRT || a_Class < 0 || a_Class >= NUMSTRC)
		return NULL;
	
	/* Failed */
	return NULL;
}

/* STR_Bind() -- Binds to address */
bool_t STR_Bind(STR_t* const a_Str, STR_Addr_t* const a_Addr)
{
	/* Check */
	if (!a_Str || !a_Addr)
		return false;
	
	return true;
}

bool_t STR_Listen(STR_t* const a_Str, const uint32_t a_ConnCap);
STR_t* STR_Accept(STR_t* const a_Str, STR_Addr_t* const a_Addr);
bool_t STR_Connect(STR_t* const a_Str, STR_Addr_t* const a_Addr);

/* STR_Close() -- Closes stream */
void STR_Close(STR_t* const a_Str)
{
	/* Check */
	if (!a_Str)
		return;
}

/* STR_OpenFile() -- Wraps opening of a file */
STR_t* STR_OpenFile(const char* const a_PathName, const char* const a_Mode)
{
	return NULL;
}

/* STR_OpenEntry() -- Wraps opening of a WAD entry */
STR_t* STR_OpenEntry(WL_WADEntry_t* const a_Entry)
{
	STR_t* New;
	STR_Addr_t Addr;
	
	/* Check */
	if (!a_Entry)
		return NULL;
	
	/* Create a new stream, which is a memory stream */
	if (!(New = STR_Open(STRT_MEM, STRC_STREAM)))
		return NULL;	// Failed for some reason
	
	/* Setup initial settings for bind */
	memset(&Addr, 0, sizeof(Addr));
	Addr.Type = STRT_MEM;
	Addr.Addr.Raw.Addr = WL_MapEntry(a_Entry);
	Addr.Addr.Raw.Size = a_Entry->Size;
	
	// Attempt to bind to address
	if (!STR_Bind(New, &Addr))
	{
		STR_Close(New);
		return NULL;
	}
	
	/* Success, so return the new stream */
	return New;
}

STR_Type_t STR_GetType(STR_t* const a_Str);
STR_Class_t STR_GetClass(STR_t* const a_Str);
bool_t STR_GetAddrInfo(const char* const a_Name, STR_AddrInfo_t** const a_Out);
void STR_FreeAddrInfo(STR_AddrInfo_t* const a_Addresses);
bool_t STR_GetNameInfo(STR_AddrInfo_t* const a_In, char* const a_Name, const size_t a_Len);
bool_t STR_GetAddr(const char* const a_Name, STR_Addr_t* const a_Addr);
uint32_t STR_Tell(STR_t* const a_Str);
uint32_t STR_Seek(STR_t* const a_Str, const uint32_t a_Where, const bool_t a_End);
uint32_t STR_Wait(STR_t* const a_Str);

/* STR_ReadFrom() -- Reads from stream */
int32_t STR_ReadFrom(STR_t* const a_Str, void* const a_In, const uint32_t a_Len, STR_Addr_t* const a_SrcAddr)
{
}

/* STR_WriteTo() -- Writes to stream */
int32_t STR_WriteTo(STR_t* const a_Str, const void* const a_Out, const uint32_t a_Len, STR_Addr_t* const a_DestAddr)
{
}

/****************************
*** SHORT NAME READ/WRITE ***
****************************/

/* Read */
// Macros
#define READ(t,n) \
t STR_r##n(STR_t* const a_Str)\
{\
	t v = 0;\
	STR_Read(a_Str, &v, sizeof(v));\
	return v;\
}

#define READXSWAP(t,n,px,x,s) \
t STR_r##px##n(STR_t* const a_Str)\
{\
	t v = 0;\
	STR_Read(a_Str, &v, sizeof(v));\
	return x##Swap##s(v);\
}

#define READLSWAP(t,n,x) READXSWAP(t,n,l,Little,x)
#define READBSWAP(t,n,x) READXSWAP(t,n,b,Big,x)

// Signed
READ(int8_t,xi8);
READ(int16_t,xi16);
READ(int32_t,xi32);
READ(int64_t,xi64);
READLSWAP(int16_t,i16,Int16)
READLSWAP(int32_t,i32,Int32)
READLSWAP(int64_t,i64,Int64)
READBSWAP(int16_t,i16,Int16)
READBSWAP(int32_t,i32,Int32)
READBSWAP(int64_t,i64,Int64)

// Unsigned
READ(uint8_t,xu8);
READ(uint16_t,xu16);
READ(uint32_t,xu32);
READ(uint64_t,xu64);
READLSWAP(uint16_t,u16,UInt16)
READLSWAP(uint32_t,u32,UInt32)
READLSWAP(uint64_t,u64,UInt64)
READBSWAP(uint16_t,u16,UInt16)
READBSWAP(uint32_t,u32,UInt32)
READBSWAP(uint64_t,u64,UInt64)

/* Write */

