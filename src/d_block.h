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
// Copyright (C) 2011 GhostlyDeath <ghostlydeath@remood.org>.
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

/****************
*** CONSTANTS ***
****************/

/* D_TBlockErr_t -- Block error */
typedef enum D_TBlockErr_e
{
	DTBE_SUCCESS,								// -+ Transport OK
	DTBE_NODATA,								// -  No Data to recieve
	DTBE_GENERALFAIL,							// -+ General Failure
	DTBE_RESETBYPEER,							// -+ Connection reset on one side
	DTBE_CONNECTIONREFUSED,						//  + Connection refused (close port?)
	DTBE_OUTOFMEMORY,							// -+ No more memory available
	DTBE_INVALIDARGUMENT,						// -+ Invalid argument passed
	DTBE_NOTTHISDIRECTION,						// -+ Wrong direction of stream
	DTBE_PREMATUREEND,							// -  Premature end of stream
} D_TBlockErr_t;

/* D_TBlockFlags_t -- Block flags */
typedef enum D_TBlockFlags_e
{
	// Compression (Make data smaller)
	DTBF_COMPRESSMASK			= 0x00000003,	// Compression Mask
	DTBF_COMPRESSSHIFT			= 0,			// Compression Shift
	
	DTBF_COMPRESS_NEVER			= 0,			// Never Compress
	DTBF_COMPRESS_DEFAULT		= 1,			// Auto-determine compression
	DTBF_COMPRESS_SPEED			= 2,			// Compress for speed (faster comp/decomp)
	DTBF_COMPRESS_SIZE			= 3,			// Compress for size (smaller)
	
	// Acknowledge (Make sure block reaches other end, etc.)
	DTBF_ACKMASK				= 0x000000038,	// Acknoledge Mask
	DTBF_ACKSHIFT				= 3,			// Shift to mask
	
	DTBF_ACK_NONE				= 0,			// No acknowledge of block
	DTBF_ACK_QUICK				= 1,			// Try to ack, but if failed ignore
	DTBF_ACK_ASYNC				= 2,			// Move forward but keep trying to get data
	DTBF_ACK_SYNC				= 3,			// Do not move until block is fully synced
} D_TBlockFlags_t;

/*****************
*** STRUCTURES ***
*****************/

/* D_TBlock_t -- Transport block */
typedef struct D_TBlock_s D_TBlock_t;

/* D_TStreamStat_t -- Stream status */
// 0 = Send, 1 = Receive
typedef struct D_TStreamStat_s
{
	// Time
	uint32_t LastTime[2];						// Last activity time
	
	// Block
	uint32_t BlkXMit[2];						// Blocks transmitted
	uint32_t BlkPS[2];							// Blocks per second
	uint32_t BlkSplit[2];						// Blocks split to fit MTU
	uint32_t BlkMerge[2];						// Blocks merged to fit MTU (after split)
	uint32_t BlkMWait[2];						// Blocks waiting to be merged
	uint32_t BlkBufferCount[2];					// Blocks waiting in buffer
	
	// Byte
	uint32_t ByteXMit[2];						// Bytes transmitted
	uint32_t BytePS[2];							// Bytes per second
	uint32_t ByteMTU[2];						// Max transmission unit
	
	// Synchronization
	uint32_t AckFail[2];						// Acknowledge failures
	uint32_t AckPass[2];						// Acknowledge successes
	
	// Compression
	uint32_t CompressCount[2];					// Compressed blocks
	uint32_t CompressGain[2];					// Bytes gained with compression
	uint32_t CompressLoss[2];					// Bytes lost with compression
} D_TStreamStat_t;

/* D_TStreamSource_t -- Stream source for blocks */
typedef struct D_TStreamSource_s
{
	void* Data;									// Extra data
	D_TStreamStat_t Stat;						// Stream status
	uint32_t BlockDefFlags;						// Flags (Block match)
	
	void (*FuncDeleteStream)(struct D_TStreamSource_s* const a_Stream);
	D_TBlockErr_t (*FuncSend)(struct D_TStreamSource_s* const a_Stream, D_TBlock_t** const a_BlkPtrIn, const uint32_t a_TFlags, D_TStreamStat_t* const a_StatPtr, void** const a_DataPtr);
	D_TBlockErr_t (*FuncRecv)(struct D_TStreamSource_s* const a_Stream, D_TBlock_t** const a_BlkPtrOut, const uint32_t a_TFlags, D_TStreamStat_t* const a_StatPtr, void** const a_DataPtr);
} D_TStreamSource_t;

/*****************
*** PROTOTYPES ***
*****************/

/* Generic File I/O */
D_TStreamSource_t* D_CreateFileInStream(const char* const a_PathName);
D_TStreamSource_t* D_CreateFileOutStream(const char* const a_PathName);

/* Generic Stream */
void D_BlockStreamDelete(D_TStreamSource_t* const a_Stream);

/* Generic Block Info */
D_TBlock_t* D_BlockNew(const uint32_t a_Magic, const uint32_t a_Flags, const uint32_t a_Size, void** const a_DataPtr);
uint32_t D_CharToMagic(const char* const a_CharMagic);
void D_BlockFree(D_TBlock_t* const a_Block);

/* Generic Block Send/Recv */
D_TBlockErr_t D_BlockRecv(D_TStreamSource_t* const a_Stream, D_TBlock_t** const a_BlkPtr);
D_TBlockErr_t D_BlockSend(D_TStreamSource_t* const a_Stream, D_TBlock_t** const a_BlkPtr);

#endif /* __D_BLOCK_H__ */

