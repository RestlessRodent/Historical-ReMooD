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
// Copyright (C) 2011-2012 GhostlyDeath <ghostlydeath@remood.org>.
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
// DESCRIPTION: Block Transport API

#ifndef __D_BLOCK_H__
#define __D_BLOCK_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "d_net.h"

/****************
*** CONSTANTS ***
****************/

#define RBLOCKBUFSIZE					128		// Block buffer size

/*****************
*** STRUCTURES ***
*****************/

/* D_RBlockStream_t -- Remote block stream */
typedef struct D_RBlockStream_s
{
	/* Info */
	void* Data;									// Private Data
	
	/* Current Block */
	char BlkHeader[4];							// Block identifier
	uint8_t* BlkData;							// Data
	size_t BlkSize;								// Block Size
	size_t BlkBufferSize;						// Size of buffer
	size_t ReadOff;								// Read Offset
	bool_t Marked;								// Marked?
	
	/* Functions */
	size_t (*RecordF)(struct D_RBlockStream_s* const a_Stream);
	bool_t (*PlayF)(struct D_RBlockStream_s* const a_Stream);
	bool_t (*FlushF)(struct D_RBlockStream_s* const a_Stream);
	
	size_t (*NetRecordF)(struct D_RBlockStream_s* const a_Stream, I_HostAddress_t* const a_Host);
	bool_t (*NetPlayF)(struct D_RBlockStream_s* const a_Stream, I_HostAddress_t* const a_Host);
	
	void (*DeleteF)(struct D_RBlockStream_s* const a_Stream);
	
	/* Stream Stat */
	uint32_t StatBlock[2];						// Block stats
	uint32_t StatBytes[2];						// Byte stats
} D_RBlockStream_t;

/****************
*** FUNCTIONS ***
****************/

D_RBlockStream_t* D_RBSCreateLoopBackStream(void);
D_RBlockStream_t* D_RBSCreateFileStream(const char* const a_PathName, const bool_t a_Overwrite);
D_RBlockStream_t* D_RBSCreateNetStream(I_NetSocket_t* const a_NetSocket);
D_RBlockStream_t* D_RBSCreatePerfectStream(D_RBlockStream_t* const a_Wrapped);
void D_RBSCloseStream(D_RBlockStream_t* const a_Stream);

void D_RBSStatStream(D_RBlockStream_t* const a_Stream, uint32_t* const a_ReadBk, uint32_t* const a_WriteBk, uint32_t* const a_ReadBy, uint32_t* const a_WriteBy);
void D_RBSUnStatStream(D_RBlockStream_t* const a_Stream);
bool_t D_RBSMarkedStream(D_RBlockStream_t* const a_Stream);

bool_t D_RBSCompareHeader(const char* const a_A, const char* const a_B);
bool_t D_RBSBaseBlock(D_RBlockStream_t* const a_Stream, const char* const a_Header);
bool_t D_RBSRenameHeader(D_RBlockStream_t* const a_Stream, const char* const a_Header);

bool_t D_RBSPlayBlock(D_RBlockStream_t* const a_Stream, char* const a_Header);
void D_RBSRecordBlock(D_RBlockStream_t* const a_Stream);

bool_t D_RBSPlayNetBlock(D_RBlockStream_t* const a_Stream, char* const a_Header, I_HostAddress_t* const a_Host);
void D_RBSRecordNetBlock(D_RBlockStream_t* const a_Stream, I_HostAddress_t* const a_Host);

bool_t D_RBSFlushStream(D_RBlockStream_t* const a_Stream);

size_t D_RBSWriteChunk(D_RBlockStream_t* const a_Stream, const void* const a_Data, const size_t a_Size);

void D_RBSWriteInt8(D_RBlockStream_t* const a_Stream, const int8_t a_Val);
void D_RBSWriteInt16(D_RBlockStream_t* const a_Stream, const int16_t a_Val);
void D_RBSWriteInt32(D_RBlockStream_t* const a_Stream, const int32_t a_Val);
void D_RBSWriteUInt8(D_RBlockStream_t* const a_Stream, const uint8_t a_Val);
void D_RBSWriteUInt16(D_RBlockStream_t* const a_Stream, const uint16_t a_Val);
void D_RBSWriteUInt32(D_RBlockStream_t* const a_Stream, const uint32_t a_Val);

void D_RBSWriteString(D_RBlockStream_t* const a_Stream, const char* const a_Val);
void D_RBSWritePointer(D_RBlockStream_t* const a_Stream, const void* const a_Ptr);

size_t D_RBSReadChunk(D_RBlockStream_t* const a_Stream, void* const a_Data, const size_t a_Size);

int8_t D_RBSReadInt8(D_RBlockStream_t* const a_Stream);
int16_t D_RBSReadInt16(D_RBlockStream_t* const a_Stream);
int32_t D_RBSReadInt32(D_RBlockStream_t* const a_Stream);
uint8_t D_RBSReadUInt8(D_RBlockStream_t* const a_Stream);
uint16_t D_RBSReadUInt16(D_RBlockStream_t* const a_Stream);
uint32_t D_RBSReadUInt32(D_RBlockStream_t* const a_Stream);

size_t D_RBSReadString(D_RBlockStream_t* const a_Stream, char* const a_Out, const size_t a_OutSize);
uint64_t D_RBSReadPointer(D_RBlockStream_t* const a_Stream);

#endif /* __D_BLOCK_H__ */

