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
// Replaces:
// * c_lib.h : Raw Byte Read/Write
// * w_wad.h : WL_ES_t, reader for WAD entries
// * i_util.h: FILE and Network wrappers

#ifndef __STR_H__
#define __STR_H__

/*****************************************************************************/

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"

/****************
*** CONSTANTS ***
****************/

/* STR_Type_t -- Stream Type */
typedef enum STR_Type_e
{
	STRT_MEM,									// Memory Stream
	STRT_ROFILE,								// File Stream (Read Only)
	STRT_RWFILE,								// File Stream (Read/Write Overwrite)
	STRT_WAD,									// WAD Stream
	STRT_NET,									// IPv4 Network Stream
	STRT_NETSIX,								// IPv6 Network Stream
	STRT_UNIX,									// UNIX Sockets
	
	NUMSTRT
} STR_Type_t;

/* STR_Class_t -- Stream Class */
typedef enum STR_Class_e
{
	STRC_STREAM,
	STRC_DATAGRAM,
	
	NUMSTRC
} STR_Class_t;

/*****************
*** STRUCTURES ***
*****************/

typedef struct STR_s STR_t;

/* STR_Addr_t -- Address */
typedef struct STR_Addr_s
{
	STR_Type_t Type;							// Type it belongs to
	
	union
	{
		struct
		{
			void* Addr;							// Address of buffer
			uint32_t Size;						// Size of buffer
			uint32_t Pos;						// Position in buffer
		} Raw;
		
		struct
		{
			char Path[PATH_MAX];				// Path of file
			uint32_t Pos;						// Position in file
		} File;
		
		struct
		{
			uint16_t Port;						// Port Number
			union
			{
				uint32_t u;						// Integer
				uint16_t s[2];					// Shorts
				uint8_t b[4];					// Bytes
			} Addr;								// IPv4 Address
		} Net;									// IPv4 Net Address
		
		struct
		{
			uint16_t Port;						// Port Number
			uint32_t Scope;						// Scope
			union
			{
				uint32_t ll[2];					// Long Longs
				uint32_t u[4];					// Integers
				uint16_t s[8];					// Shorts
				uint8_t b[16];					// Bytes
			} Addr;								// Address
		} Net6;									// IPv6 Net Address
	} Addr;										// Address Data
} STR_Addr_t;

/* STR_AddrInfo_t -- Address Info */
typedef struct STR_AddrInfo_s
{
	STR_Type_t Type;							// Type of address
	STR_Addr_t Addr;							// Address Info
	struct STR_AddrInfo_s* Next;				// Next info
} STR_AddrInfo_t;

/* Define WL_WADEntry_t */
#if !defined(__REMOOD_WLWADENT_DEFINED)
	typedef struct WL_WADEntry_s WL_WADEntry_t;
	#define __REMOOD_WLWADENT_DEFINED
#endif

/****************
*** FUNCTIONS ***
****************/

/* Basic */
STR_t* STR_Open(const STR_Type_t a_Type, const STR_Class_t a_Class);
bool_t STR_Bind(STR_t* const a_Str, STR_Addr_t* const a_Addr);
bool_t STR_Listen(STR_t* const a_Str, const uint32_t a_ConnCap);
STR_t* STR_Accept(STR_t* const a_Str, STR_Addr_t* const a_Addr);
bool_t STR_Connect(STR_t* const a_Str, STR_Addr_t* const a_Addr);
void STR_Close(STR_t* const a_Str);

/* Wrapped */
STR_t* STR_OpenFile(const char* const a_PathName, const char* const a_Mode);
STR_t* STR_OpenEntry(WL_WADEntry_t* const a_Entry);

/* Obtain generic info */
STR_Type_t STR_GetType(STR_t* const a_Str);
STR_Class_t STR_GetClass(STR_t* const a_Str);

/* Name/Address Resolution */
bool_t STR_GetAddrInfo(const char* const a_Name, STR_AddrInfo_t** const a_Out);
void STR_FreeAddrInfo(STR_AddrInfo_t* const a_Addresses);
bool_t STR_GetNameInfo(STR_AddrInfo_t* const a_In, char* const a_Name, const size_t a_Len);
bool_t STR_GetAddr(const char* const a_Name, STR_Addr_t* const a_Addr);

/* Location (if supported) */
uint32_t STR_Tell(STR_t* const a_Str);
uint32_t STR_Seek(STR_t* const a_Str, const uint32_t a_Where, const bool_t a_End);
uint32_t STR_Wait(STR_t* const a_Str);

/* Basic Read/Write */
int32_t STR_ReadFrom(STR_t* const a_Str, void* const a_In, const uint32_t a_Len, STR_Addr_t* const a_SrcAddr);
int32_t STR_WriteTo(STR_t* const a_Str, const void* const a_Out, const uint32_t a_Len, STR_Addr_t* const a_DestAddr);

#define STR_Read(s,i,l) STR_ReadFrom((s), (i), (l), NULL)
#define STR_Write(s,o,l) STR_WriteTo((s), (o), (l), NULL)

/* Read */
// Non Swapped
int8_t STR_rxi8(STR_t* const a_Str);
int16_t STR_rxi16(STR_t* const a_Str);
int32_t STR_rxi32(STR_t* const a_Str);
int64_t STR_rxi64(STR_t* const a_Str);
uint8_t STR_rxu8(STR_t* const a_Str);
uint16_t STR_rxu16(STR_t* const a_Str);
uint32_t STR_rxu32(STR_t* const a_Str);
uint64_t STR_rxu64(STR_t* const a_Str);

// Little Swapped
int16_t STR_rli16(STR_t* const a_Str);
int32_t STR_rli32(STR_t* const a_Str);
int64_t STR_rli64(STR_t* const a_Str);
uint16_t STR_rlu16(STR_t* const a_Str);
uint32_t STR_rlu32(STR_t* const a_Str);
uint64_t STR_rlu64(STR_t* const a_Str);

// Big Swapped
int16_t STR_rbi16(STR_t* const a_Str);
int32_t STR_rbi32(STR_t* const a_Str);
int64_t STR_rbi64(STR_t* const a_Str);
uint16_t STR_rbu16(STR_t* const a_Str);
uint32_t STR_rbu32(STR_t* const a_Str);
uint64_t STR_rbu64(STR_t* const a_Str);

/* Write */
void STR_wxi8(STR_t* const a_Str, const int8_t a_Val);
void STR_wxi16(STR_t* const a_Str, const int16_t a_Val);
void STR_wxi32(STR_t* const a_Str, const int32_t a_Val);
void STR_wxi64(STR_t* const a_Str, const int64_t a_Val);
void STR_wxu8(STR_t* const a_Str, const uint8_t a_Val);
void STR_wxu16(STR_t* const a_Str, const uint16_t a_Val);
void STR_wxu32(STR_t* const a_Str, const uint32_t a_Val);
void STR_wxu64(STR_t* const a_Str, const uint64_t a_Val);

// Little Swapped
void STR_wli16(STR_t* const a_Str, const int16_t a_Val);
void STR_wli32(STR_t* const a_Str, const int32_t a_Val);
void STR_wli64(STR_t* const a_Str, const int64_t a_Val);
void STR_wlu16(STR_t* const a_Str, const uint16_t a_Val);
void STR_wlu32(STR_t* const a_Str, const uint32_t a_Val);
void STR_wlu64(STR_t* const a_Str, const uint64_t a_Val);

// Big Swapped
void STR_wbi16(STR_t* const a_Str, const int16_t a_Val);
void STR_wbi32(STR_t* const a_Str, const int32_t a_Val);
void STR_wbi64(STR_t* const a_Str, const int64_t a_Val);
void STR_wbu16(STR_t* const a_Str, const uint16_t a_Val);
void STR_wbu32(STR_t* const a_Str, const uint32_t a_Val);
void STR_wbu64(STR_t* const a_Str, const uint64_t a_Val);

/*****************************************************************************/

#endif /* __STR_H__ */


