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
// Copyright (C) 2011-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Block Transport API

#ifndef __D_BLOCK_H__
#define __D_BLOCK_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "sn.h"
#include "w_wad.h"

/****************
*** CONSTANTS ***
****************/

#define RBLOCKBUFSIZE					128		// Block buffer size

/* D_BSStreamFlags_t -- Flags for streams */
typedef enum D_BSStreamFlags_s
{
	DRBSSF_OVERWRITE				= 0x0001,	// Overwrite existing file
	DRBSSF_READONLY					= 0x0002,	// Create Read Only Stream
} D_BSStreamFlags_t;

/* D_BSStreamIOCtl_t -- IOCtl Command for stream */
typedef enum D_BSStreamIOCtl_e
{
	DRBSIOCTL_ISPERFECT,						// Read: Is perfect?
	DRBSIOCTL_MAXTRANSPORT,						// Read/Write: Max transport size
	
	DRBSIOCTL_DROPHOST,							// Drops a host (reliable)
	DRBSIOCTL_RELRESET,							// Resets reliable state to normal
	DRBSIOCTL_ADDHOST,							// Adds a host (reliable)
	DRBSIOCTL_ISAUTH,							// Current packet authentic?
	DRBSIOCTL_CHECKISONLIST,					// Is on auth list?
	DRBSIOCTL_GETISONLIST,						// Is on auth list?
	
	NUMDRBSSTREAMIOCTL
} D_BSStreamIOCtl_t;

/*****************
*** STRUCTURES ***
*****************/

#if !defined(__REMOOD_DBSTDEFINED)
	typedef struct D_BS_s D_BS_t;
	#define __REMOOD_DBSTDEFINED
#endif

/* D_BS_t -- Remote block stream */
struct D_BS_s 
{
	/* Info */
	void* Data;									// Private Data
	uint32_t Flags;								// Flags (if ever needed)
	
	/* Current Block */
	char BlkHeader[4];							// Block identifier
	uint8_t* BlkData;							// Data
	size_t BlkSize;								// Block Size
	size_t BlkBufferSize;						// Size of buffer
	size_t ReadOff;								// Read Offset
	bool_t Marked;								// Marked?
	
	/* Functions */
	size_t (*RecordF)(struct D_BS_s* const a_Stream);
	bool_t (*PlayF)(struct D_BS_s* const a_Stream);
	bool_t (*FlushF)(struct D_BS_s* const a_Stream);
	
	size_t (*NetRecordF)(struct D_BS_s* const a_Stream, I_HostAddress_t* const a_Host);
	bool_t (*NetPlayF)(struct D_BS_s* const a_Stream, I_HostAddress_t* const a_Host);
	
	void (*DeleteF)(struct D_BS_s* const a_Stream);
	bool_t (*IOCtlF)(struct D_BS_s* const a_Stream, const D_BSStreamIOCtl_t a_IOCtl, intptr_t* a_DataP);
	
	/* Stream Stat */
	uint32_t StatBlock[2];						// Block stats
	uint32_t StatBytes[2];						// Byte stats
};

/****************
*** FUNCTIONS ***
****************/

D_BS_t* D_BSCreateLoopBackStream(void);
D_BS_t* D_BSCreateWLStream(WL_ES_t* const a_Stream);
D_BS_t* D_BSCreateFileStream(const char* const a_PathName, const uint32_t a_Flags);
D_BS_t* D_BSCreateNetStream(I_NetSocket_t* const a_NetSocket);

D_BS_t* D_BSCreatePackedStream(D_BS_t* const a_Wrapped);

void D_BSCloseStream(D_BS_t* const a_Stream);

bool_t D_BSStreamIOCtl(D_BS_t* const a_Stream, const D_BSStreamIOCtl_t a_IOCtl, const intptr_t a_DataP);

bool_t D_BSCompareHeader(const char* const a_A, const char* const a_B);
bool_t D_BSBaseBlock(D_BS_t* const a_Stream, const char* const a_Header);
bool_t D_BSRenameHeader(D_BS_t* const a_Stream, const char* const a_Header);

bool_t D_BSPlayBlock(D_BS_t* const a_Stream, char* const a_Header);
void D_BSRecordBlock(D_BS_t* const a_Stream);

bool_t D_BSPlayNetBlock(D_BS_t* const a_Stream, char* const a_Header, I_HostAddress_t* const a_Host);
void D_BSRecordNetBlock(D_BS_t* const a_Stream, I_HostAddress_t* const a_Host);

bool_t D_BSFlushStream(D_BS_t* const a_Stream);
void D_BSRewind(D_BS_t* const a_Stream);

size_t D_BSWriteChunk(D_BS_t* const a_Stream, const void* const a_Data, const size_t a_Size);

void D_BSwi8(D_BS_t* const a_Stream, const int8_t a_Val);
void D_BSwi16(D_BS_t* const a_Stream, const int16_t a_Val);
void D_BSwi32(D_BS_t* const a_Stream, const int32_t a_Val);
void D_BSwi64(D_BS_t* const a_Stream, const int64_t a_Val);
void D_BSwu8(D_BS_t* const a_Stream, const uint8_t a_Val);
void D_BSwu16(D_BS_t* const a_Stream, const uint16_t a_Val);
void D_BSwu32(D_BS_t* const a_Stream, const uint32_t a_Val);
void D_BSwu64(D_BS_t* const a_Stream, const uint64_t a_Val);

void D_BSws(D_BS_t* const a_Stream, const char* const a_Val);
void D_BSwsn(D_BS_t* const a_Stream, const char* const a_Val, const size_t a_N);
void D_BSwp(D_BS_t* const a_Stream, const void* const a_Ptr);

size_t D_BSReadChunk(D_BS_t* const a_Stream, void* const a_Data, const size_t a_Size);

int8_t D_BSri8(D_BS_t* const a_Stream);
int16_t D_BSri16(D_BS_t* const a_Stream);
int32_t D_BSri32(D_BS_t* const a_Stream);
int64_t D_BSri64(D_BS_t* const a_Stream);
uint8_t D_BSru8(D_BS_t* const a_Stream);
uint16_t D_BSru16(D_BS_t* const a_Stream);
uint32_t D_BSru32(D_BS_t* const a_Stream);
uint64_t D_BSru64(D_BS_t* const a_Stream);

uint64_t D_BSrcu64(D_BS_t* const a_Stream);
void D_BSwcu64(D_BS_t* const a_Stream, const uint64_t a_Val);

void D_BSrhost(D_BS_t* const a_Stream, I_HostAddress_t* const a_Out);
void D_BSwhost(D_BS_t* const a_Stream, const I_HostAddress_t* const a_In);

size_t D_BSrs(D_BS_t* const a_Stream, char* const a_Out, const size_t a_OutSize);
size_t D_BSrsn(D_BS_t* const a_Stream, char* const a_Out, const size_t a_OutSize);
uint64_t D_BSrp(D_BS_t* const a_Stream);

#endif /* __D_BLOCK_H__ */

