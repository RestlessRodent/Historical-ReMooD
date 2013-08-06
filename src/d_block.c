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

/***************
*** INCLUDES ***
***************/

#include "d_block.h"
#include "z_zone.h"
#include "console.h"
#include "i_system.h"
#include "m_random.h"
#include "m_misc.h"
#include "c_lib.h"
#include "m_argv.h"
#include "z_miniz.h"

/******************
*** FILE STREAM ***
******************/

/* DS_RBSFile_DeleteF() -- Delete file stream */
static void DS_RBSFile_DeleteF(struct D_BS_s* const a_Stream)
{
	/* Check */
	if (!a_Stream)
		return;
	
	/* Close file? */
	if (a_Stream->Data)
	{
		I_FileClose((I_File_t*)a_Stream->Data);
		a_Stream->Data = NULL;
	}
}

/* DS_RBSFile_RecordF() -- Records the current block */
static size_t DS_RBSFile_RecordF(struct D_BS_s* const a_Stream)
{
	I_File_t* File;
	uint16_t Len, USLen;
	
	/* Check */
	if (!a_Stream)
		return 0;
	
	/* Obtain file */
	File = (I_File_t*)a_Stream->Data;
	
	// Not found?
	if (!File)
		return 0;
	
	/* Flush before write */
	I_FileFlush(File);
	
	/* Write Fields */
	// Header
	I_FileWrite(File, a_Stream->BlkHeader, 4);
	
	// Size
	if (a_Stream->BlkSize < UINT16_C(65535))
		Len = a_Stream->BlkSize; 
	else
		Len = UINT16_C(65535);
	
	USLen = Len;
	Len = LittleSwapUInt16(Len);
	I_FileWrite(File, &Len, 2);
	
	// Data
	return I_FileWrite(File, a_Stream->BlkData, USLen);
}

/* DS_RBSFile_PlayF() -- Play from file */
static bool_t DS_RBSFile_PlayF(struct D_BS_s* const a_Stream)
{
#define BUFSIZE 128
	uint8_t Buf[BUFSIZE];
	I_File_t* File;
	char Header[5];
	uint16_t Len, Left, Count;
	
	/* Check */
	if (!a_Stream)
		return 0;
	
	/* Obtain file */
	File = (I_File_t*)a_Stream->Data;
	
	// Not found?
	if (!File)
		return 0;
	
	/* Flush before read */
	// Do not flush the file, because on Windows it seems it seeks the file to an offset of the buffer
	// size (in my case 4096). So best not to do that so savegames and demos work.
	//I_FileFlush(File);
	
	/* Read Data */
	// Header
	Header[4] = 0;
	if (I_FileRead(File, &Header[0], 4) < 4)
		return false;
	
	// Base a new block
	D_BSBaseBlock(a_Stream, Header);
	
	// Length
	Len = 0;
	if (I_FileRead(File, &Len, 2) < 2)
		return false;
	
	Len = LittleSwapUInt16(Len);
	Left = Len;
	
	// Treat zero sized blocks as success!
	if (!Len)
		return true;
	
	// Data Read Loop
	do
	{
		// Big enough read?
		if (Left >= BUFSIZE)
			Count = BUFSIZE;
		
		// Too small
		else
			Count = Left;
		
		// Read associated data
		memset(Buf, 0, sizeof(Buf));
		if (I_FileRead(File, Buf, Count) < Count)
			return false;
		
		// Write count sized chunk
		D_BSWriteChunk(a_Stream, Buf, Count);
		
		// Remove count from left
		Left -= Count;
	} while (Left);

	return true;
#undef BUFSIZE
}

/* DS_RBSFile_FlushF() -- Flushes file */
static bool_t DS_RBSFile_FlushF(struct D_BS_s* const a_Stream)
{
	I_File_t* File;
	
	/* Check */
	if (!a_Stream)
		return false;
	
	/* Obtain file */
	File = (I_File_t*)a_Stream->Data;
	
	// Not found?
	if (!File)
		return false;
	
	/* Flush */
	I_FileFlush(File);
	return true;
}

/******************
*** FILE STREAM ***
******************/

/* DS_RBSWL_DeleteF() -- Delete file stream */
static void DS_RBSWL_DeleteF(struct D_BS_s* const a_Stream)
{
	/* Check */
	if (!a_Stream)
		return;
}

/* DS_RBSWL_RecordF() -- Records the current block */
static size_t DS_RBSWL_RecordF(struct D_BS_s* const a_Stream)
{
	// Recording blocks is not supported for WL Streams (read-only!)
	return 0;
}

/* DS_RBSWL_PlayF() -- Play from file */
bool_t DS_RBSWL_PlayF(struct D_BS_s* const a_Stream)
{
	WL_ES_t* Stream;
	char Header[5];
	uint16_t Len;
	uint32_t i;
	void* Data;
	
	/* Check */
	if (!a_Stream)
		return false;
		
	/* Get Data */
	Stream = (WL_ES_t*)a_Stream->Data;
	
	/* EOS? */
	if (WL_StreamEOF(Stream))
		return false;
	
	/* Read Header */
	// Clear
	memset(Header, 0, sizeof(Header));
	Len = 0;
	Data = NULL;
	
	// Read Header
	for (i = 0; i < 4; i++)
		Header[i] = WL_Src(Stream);
	
	// Read Length and Sum
	Len = WL_Srlu16(Stream);
	
	// Read data, if possible (Len could be zero (empty block?))
	if (Len > 0)
	{
		Data = Z_Malloc(Len, PU_STATIC, NULL);
		WL_StreamRawRead(Stream, WL_StreamTell(Stream), Data, Len);
		WL_StreamSeek(Stream, WL_StreamTell(Stream) + Len, false);
	}
	
	/* Initialize Block */
	D_BSBaseBlock(a_Stream, Header);
	
	/* Write Data to Block */
	D_BSWriteChunk(a_Stream, Data, Len);
	if (Data)
		Z_Free(Data);
	
	/* Success! */
	return true;
}

/****************
*** LOOP BACK ***
****************/

/* DS_RBSLoopBackHold_t -- Holding store */
typedef struct DS_RBSLoopBackHold_s
{
	uint64_t FlushID;							// Current flush ID
	char Header[5];								// Header
	uint8_t* Data;								// Data
	size_t Size;								// Size
	I_HostAddress_t Addr;						// Address
} DS_RBSLoopBackHold_t;

/* DS_RBSLoopBackData_t -- Loop back device */
typedef struct DS_RBSLoopBackData_s
{
	uint64_t FlushID;							// FlushID
	DS_RBSLoopBackHold_t** Q;					// Blocks in Q
	size_t SizeQ;								// Size of Q
} DS_RBSLoopBackData_t;

/* DS_RBSLoopBack_DeleteF() -- Delete loopback stream */
static void DS_RBSLoopBack_DeleteF(struct D_BS_s* const a_Stream)
{
	size_t i;
	DS_RBSLoopBackData_t* LoopData;
	
	/* Check */
	if (!a_Stream)
		return;
	
	/* Get Data */
	LoopData = (DS_RBSLoopBackData_t*)a_Stream->Data;
	
	// Check
	if (!LoopData)
		return;
	
	/* Destroy all blocks */
	if (LoopData->Q)
	{
		for (i = 0; i < LoopData->SizeQ; i++)
			if (LoopData->Q[i])
				Z_Free(LoopData->Q[i]);
		Z_Free(LoopData->Q);
	}
	
	/* Free Data */
	Z_Free(LoopData);
}

/* DS_RBSLoopBack_NetRecordF() -- Records a block */
size_t DS_RBSLoopBack_NetRecordF(struct D_BS_s* const a_Stream, I_HostAddress_t* const a_Host)
{
	size_t i;
	DS_RBSLoopBackData_t* LoopData;
	DS_RBSLoopBackHold_t* Hold;
	
	/* Check */
	if (!a_Stream)
		return 0;
	
	/* Get Data */
	LoopData = (DS_RBSLoopBackData_t*)a_Stream->Data;
	
	// Check
	if (!LoopData)
		return 0;
	
	/* Find spot to record at */
	Hold = NULL;
	for (i = 0; i < LoopData->SizeQ; i++)
		if (!LoopData->Q[i])
		{
			Hold = LoopData->Q[i] = Z_Malloc(sizeof(DS_RBSLoopBackHold_t), PU_BLOCKSTREAM, NULL);
			break;
		}
	
	// No blank spots?
	if (!Hold)
	{
		Z_ResizeArray((void**)&LoopData->Q, sizeof(*LoopData->Q),
						LoopData->SizeQ, LoopData->SizeQ + 2);
		Hold = LoopData->Q[LoopData->SizeQ++] =
				Z_Malloc(sizeof(DS_RBSLoopBackHold_t), PU_BLOCKSTREAM, NULL);
		LoopData->SizeQ++;
	}
	
	/* Store info in hold */
	// Copy header
	memmove(Hold->Header, a_Stream->BlkHeader, 4);
	
	// Clone Data
	Hold->Size = a_Stream->BlkSize;
	Hold->Data = Z_Malloc(Hold->Size, PU_BLOCKSTREAM, NULL);
	memmove(Hold->Data, a_Stream->BlkData, Hold->Size);
	if (a_Host)
		memmove(&Hold->Addr, a_Host, sizeof(Hold->Addr));
	Hold->FlushID = LoopData->FlushID + 1;
	
	/* Return value does not matter */
	return 1;
}

/* DS_RBSLoopBack_NetPlayF() -- Backs a block back */
bool_t DS_RBSLoopBack_NetPlayF(struct D_BS_s* const a_Stream, I_HostAddress_t* const a_Host)
{
	DS_RBSLoopBackData_t* LoopData;
	DS_RBSLoopBackHold_t* Hold;
	size_t i;
	
	/* Check */
	if (!a_Stream)
		return false;
	
	/* Get Data */
	LoopData = (DS_RBSLoopBackData_t*)a_Stream->Data;
	
	// Check
	if (!LoopData)
		return false;
	
	/* Nothing recorded? */
	if (!LoopData->SizeQ)
		return false;
	
	/* Nothing in the Q? */
	if (!LoopData->Q[0])
		return false;
	
	/* Get current hold */
	Hold = LoopData->Q[0];
	
	// See if it after the current flush level
	if (LoopData->FlushID <= Hold->FlushID)
		return false;
	
	/* Create Base Block */
	D_BSBaseBlock(a_Stream, Hold->Header);
	
	// Write all our data in it
	D_BSWriteChunk(a_Stream, Hold->Data, Hold->Size);
	
	// Copy addr
	if (a_Host)
		memmove(a_Host, &Hold->Addr, sizeof(Hold->Addr));
	
	/* Free Hold */
	if (Hold->Data)
		Z_Free(Hold->Data);
	Hold->Data = NULL;
	Z_Free(Hold);
	LoopData->Q[0] = NULL;
	
	// Move everything down
	if (LoopData->SizeQ >= 1)
	{
		for (i = 0; i < LoopData->SizeQ - 1; i++)
			LoopData->Q[i] = LoopData->Q[i + 1];
		LoopData->Q[i] = NULL;
	}
	
	// Nothing left?
	if (!LoopData->Q[0])
		LoopData->FlushID = 1;
	
	/* Something read */
	return true;
}

/* DS_RBSLoopBack_FlushF() -- Flush block stream */
bool_t DS_RBSLoopBack_FlushF(struct D_BS_s* const a_Stream)
{
	DS_RBSLoopBackData_t* LoopData;
	
	/* Check */
	if (!a_Stream)
		return false;
	
	/* Get Data */
	LoopData = (DS_RBSLoopBackData_t*)a_Stream->Data;
	
	// Check
	if (!LoopData)
		return false;
	
	/* No data in Q */
	if (!LoopData->SizeQ)
		return false;
	
	/* Q is empty */
	if (!LoopData->Q[0])
		return false;
		
	/* Increase flush ID */
	LoopData->FlushID++;
	
	/* Nothing done */
	return true;
}

/**************
*** NETWORK ***
**************/

/* I_RBSNetSockData_t -- Network Socket Data */
typedef struct I_RBSNetSockData_s
{
#define IRBSNETSOCKBUFSIZE 1300
	I_NetSocket_t* Socket;						// Socket
	uint8_t ReadBuf[IRBSNETSOCKBUFSIZE];		// Read Buffer Fill
	uint8_t WriteBuf[IRBSNETSOCKBUFSIZE];		// Output Buffer Fill
} I_RBSNetSockData_t;

/* DS_RBSNet_DeleteF() -- Delete network stream */
static void DS_RBSNet_DeleteF(struct D_BS_s* const a_Stream)
{
	I_RBSNetSockData_t* NetData;
	
	/* Check */
	if (!a_Stream)
		return;
	
	/* Get Data */
	NetData = a_Stream->Data;
	
	if (!NetData)
		return;
	
	/* Free Data */
	Z_Free(NetData);
}

/* DS_RBSNet_NetRecordF() -- Write block to network */
size_t DS_RBSNet_NetRecordF(struct D_BS_s* const a_Stream, I_HostAddress_t* const a_Host)
{
	I_RBSNetSockData_t* NetData;
	I_NetSocket_t* Socket;
	size_t RetVal, sz, i, ChunkSize;
	uint8_t* TBuf = NULL;
	uint16_t lenSize;
	
	/* Check */
	if (!a_Stream)
		return 0;
	
	/* Get Data */
	NetData = a_Stream->Data;
	
	if (!NetData)
		return 0;
	
	/* Get socket */
	Socket = NetData->Socket;
	ChunkSize = a_Stream->BlkSize;
	
	if (ChunkSize > IRBSNETSOCKBUFSIZE - 16)
		ChunkSize = IRBSNETSOCKBUFSIZE - 16;
	
	/* Create temporary buffer for sending */
	sz = 4 + 2 + ChunkSize;
	
	//TBuf = Z_Malloc(sz, PU_BLOCKSTREAM, NULL);//alloca(sz);
	memset(NetData->WriteBuf, 0, sizeof(NetData->WriteBuf));
	TBuf = NetData->WriteBuf;
	
	// Write to it
		// Header
	for (i = 0; i < 4; i++)
		TBuf[i] = a_Stream->BlkHeader[i];
	lenSize = LittleSwapUInt16((uint16_t)ChunkSize);
	memmove((void*)(((uintptr_t)TBuf) + 4), &lenSize, sizeof(lenSize));
	memmove((void*)(((uintptr_t)TBuf) + 6), a_Stream->BlkData, ChunkSize);
	
	/* Send the entire buffer */
	// Sending 4 different packets alone could cause fragmentation and out of
	// order problems (which is REALLY not wanted). So this way, at least this
	// is not much of a problem.
	RetVal = I_NetSend(Socket, a_Host, TBuf, sz);
	
	/* Clean up and return */
	//Z_Free(TBuf);
	return RetVal;
}

/* DS_RBSNet_NetPlayF() -- Play block from the network */
bool_t DS_RBSNet_NetPlayF(struct D_BS_s* const a_Stream, I_HostAddress_t* const a_Host)
{
	I_RBSNetSockData_t* NetData;
	I_NetSocket_t* Socket;
	char Header[5];
	uint16_t Len;
	size_t RetVal;
	
	/* Check */
	if (!a_Stream)
		return false;
	
	/* Get Data */
	NetData = a_Stream->Data;
	
	if (!NetData)
		return false;
	
	/* Get socket */
	Socket = NetData->Socket;
	
	// Check
	if (!Socket)
		return false;
	
	/* Read data from socket */
	memset(NetData->ReadBuf, 0, IRBSNETSOCKBUFSIZE);
	if ((RetVal = I_NetRecv(Socket, a_Host, NetData->ReadBuf, IRBSNETSOCKBUFSIZE)) < 6)
		return false;
	
	/* Extract information */
	memset(Header, 0, sizeof(Header));
	memmove(Header, NetData->ReadBuf, 4);
	memmove(&Len, NetData->ReadBuf + 4, 2);
	
	// Swap Len
	Len = LittleSwapUInt16(Len);
	
	// Limit
	if (Len >= IRBSNETSOCKBUFSIZE - 6)
		Len = IRBSNETSOCKBUFSIZE - 6 - 1;
	
	// Length Limit?
	//if (!Len)
	//	return false;
	
	/* Write data into block */
	D_BSBaseBlock(a_Stream, Header);
	D_BSWriteChunk(a_Stream, NetData->ReadBuf + 6, Len);
	
	/* Was played back */
	return true;
}

/* DS_RBSNet_IOCtlF() -- IO Ctl */
static bool_t DS_RBSNet_IOCtlF(struct D_BS_s* const a_Stream, const D_BSStreamIOCtl_t a_IOCtl, const intptr_t a_DataP)
{
	/* Check */
	if (!a_DataP)
		return false;
	
	/* Which? */
	switch (a_IOCtl)
	{
			// Maximum transport
		case DRBSIOCTL_MAXTRANSPORT:
			*((intptr_t*)a_DataP) = IRBSNETSOCKBUFSIZE - 16;
			return true;
			
			// Unknown
		default:
			return false;
	}
}

/********************
*** PACKED STREAM ***
********************/

#define RBSPACKEDMAXWAITBLOCK			128		// Max blocks in queue
#define RBSPACKEDMAXWAITBYTES			10240	// Max bytes in queue
#define RBSPACKEDZLIBLEVEL				9		// Zlib compression level
#define RBSPACKEDZLIBCHUNK		RBSPACKEDMAXWAITBYTES + 1024

/* DS_RBSPackedData_t -- Reliable transport stream data */
typedef struct DS_RBSPackedData_s
{
	D_BS_t* Self;								// Self
	D_BS_t* WrapStream;							// Stream being wrapped
	uint32_t TransSize;							// Transport Size
	
	bool_t InPack;								// Currently Packing
	bool_t HasAddr;								// Has Address
	I_HostAddress_t CurAddr;					// Current Address
	
	uint32_t WaitingBlocks;						// Blocks waiting
	uint32_t WaitingBytes;						// Bytes waiting
	
	uint8_t InBuf[RBSPACKEDMAXWAITBYTES];		// Input Data
	uint8_t* InAt;								// Input is at...
	uint8_t* InEnd;								// End of input
	
	uint8_t OutBuf[RBSPACKEDMAXWAITBYTES];		// Output Data
	uint8_t* OutAt;								// Output is at...
} DS_RBSPackedData_t;

/* DS_RBSPacked_FlushF() -- Flushes stream */
bool_t DS_RBSPacked_FlushF(struct D_BS_s* const a_Stream)
{
#define BUFSIZE RBSPACKEDZLIBCHUNK//63
	DS_RBSPackedData_t* PackData;
	uint8_t* Seek;
	uint8_t Header[5];
	int32_t i;
	uint32_t Size;
	bool_t RetVal;
	mz_stream ZStream;
	uint8_t ZBuf[BUFSIZE];
	int ZRet;
	
	/* Get Data */
	RetVal = false;
	PackData = a_Stream->Data;
	
	/* Nothing to write? */
	if (PackData->OutAt <= &PackData->OutBuf[0])
		return true;
	
	/* Initialize Compression */
	// Initialize Stream
	memset(&ZStream, 0, sizeof(ZStream));
	ZStream.avail_in = PackData->OutAt - &PackData->OutBuf[0];
	ZStream.next_in = PackData->OutBuf;
	
	// Attempt to initialize deflation
	if ((ZRet = mz_deflateInit(&ZStream, RBSPACKEDZLIBLEVEL)) != MZ_OK)
	{
		// Well this REALLY sucks, need to write all the blocks all over again!
		Seek = PackData->OutBuf;
		while (Seek < PackData->OutAt)
		{
			// Read header
			memset(Header, 0, sizeof(Header));
			for (i = 0; i < 4; i++)
				Header[i] = ReadUInt8(&Seek);
			D_BSBaseBlock(PackData->WrapStream, Header);
			
			// Read Size
			Size = BigReadUInt16(&Seek);
			
			// Write direct chunk
			D_BSWriteChunk(PackData->WrapStream, Seek, Size);
			Seek += Size;
			
			// Record
			D_BSRecordNetBlock(PackData->WrapStream,
				(PackData->HasAddr ? &PackData->CurAddr : NULL));
		}
		
		// Flush destination stream
		D_BSFlushStream(PackData->WrapStream);
		
		// Full reset
		memset(PackData->OutBuf, 0, sizeof(PackData->OutBuf));
		PackData->OutAt = PackData->OutBuf;
		PackData->HasAddr = false;
		
		// Compress init failed, so say as such
		return false;
	}
	
	/* Deflation Loop */
	// Initialize
	D_BSBaseBlock(PackData->WrapStream, "ZLIB");
	D_BSwu16(PackData->WrapStream, ZStream.avail_in);
	
	// Finish off whatever is possible
	memset(ZBuf, 0, sizeof(ZBuf));
	ZStream.avail_out = BUFSIZE;
	ZStream.next_out = ZBuf;
	
	// Deflate into stream and hope for the best
	if ((ZRet = mz_deflate(&ZStream, MZ_FINISH)) == MZ_STREAM_END)
		D_BSWriteChunk(PackData->WrapStream, ZBuf, BUFSIZE - ZStream.avail_out);
	
	// End deflation
	mz_deflateEnd(&ZStream);
	
	// Record
	D_BSRecordNetBlock(PackData->WrapStream, (PackData->HasAddr ? &PackData->CurAddr : NULL));
	
	// Flush destination stream
	D_BSFlushStream(PackData->WrapStream);
	
	// Full reset
	memset(PackData->OutBuf, 0, sizeof(PackData->OutBuf));
	PackData->OutAt = PackData->OutBuf;
	PackData->HasAddr = false;
	
	// Return success!
	return true;
#undef BUFSIZE
}

/* DS_RBSPacked_NetRecordF() -- Records to stream */
size_t DS_RBSPacked_NetRecordF(struct D_BS_s* const a_Stream, I_HostAddress_t* const a_Host)
{
	DS_RBSPackedData_t* PackData;
	int32_t SizeNeeded;
	int32_t SizeLeft, i;
	uint32_t BSize;
	
	/* Get Data */
	PackData = a_Stream->Data;
	
	// Check
	if (!PackData)
		return false;
	
	/* Calculate needed size to write */
	SizeNeeded = 4 + 2 + a_Stream->BlkSize;	// header + size + data
	SizeLeft = &PackData->OutBuf[PackData->TransSize] - PackData->OutAt;
	
	/* Change of address? Too small? */
	if ((SizeNeeded >= SizeLeft) ||				// Too small
		(SizeLeft < 6) ||						// No Space Left
		(a_Host && !PackData->HasAddr) ||		// Adds address
		(!a_Host && PackData->HasAddr) ||		// Loses address
		(a_Host && PackData->HasAddr &&			// Address mismatch
			!I_NetCompareHost(a_Host, &PackData->CurAddr)) ||
		(PackData->WaitingBlocks >= RBSPACKEDMAXWAITBLOCK) ||	// Too many blocks?
		(PackData->WaitingBytes >= RBSPACKEDMAXWAITBYTES))	// Too many bytes?
		DS_RBSPacked_FlushF(a_Stream);	// Send stream away
	
	/* Recalculate needed size (in case of flush) */
	SizeLeft = &PackData->OutBuf[PackData->TransSize] - PackData->OutAt;
	
	/* WAY too big to fit? */
	if (SizeNeeded >= SizeLeft)
	{
		// Clone block into destination
		D_BSBaseBlock(PackData->WrapStream, a_Stream->BlkHeader);
		D_BSWriteChunk(PackData->WrapStream, a_Stream->BlkData, a_Stream->BlkSize);
		D_BSRecordNetBlock(PackData->WrapStream, a_Host);
		
		return a_Stream->BlkSize;
	}
	
	/* Concat into the output buffer */
	// Header
	for (i = 0; i < 4; i++)
		WriteUInt8(&PackData->OutAt, a_Stream->BlkHeader[i]);
	
	// Size
	BSize = a_Stream->BlkSize;
	BigWriteUInt16(&PackData->OutAt, BSize);
	
	// Copy Data
	memmove(PackData->OutAt, a_Stream->BlkData, a_Stream->BlkSize);
	PackData->OutAt += a_Stream->BlkSize;
	
	// Copy Address
	if (a_Host)
	{
		PackData->HasAddr = true;
		memmove(&PackData->CurAddr, a_Host, sizeof(*a_Host));
	}
	else
		PackData->HasAddr = false;
	
	/* Let us say that it worked! */
	return true;
}

/* DS_RBSPacked_NetPlayF() -- Plays from stream */
bool_t DS_RBSPacked_NetPlayF(struct D_BS_s* const a_Stream, I_HostAddress_t* const a_Host)
{
	DS_RBSPackedData_t* PackData;
	I_HostAddress_t Addr;
	char Header[5];
	mz_stream ZStream;
	int32_t ZRet, i;
	int32_t Size, PackSize;
	bool_t zl;
	
	/* Get Data */
	PackData = a_Stream->Data;
	
	/* No input data? Read more! */
	while (PackData->InAt + 6 > PackData->InEnd)
	{
		// Init
		memset(Header, 0, sizeof(Header));
		memset(&Addr, 0, sizeof(Addr));
		
		// Try reading from wrapped stream
		if (!D_BSPlayNetBlock(PackData->WrapStream, Header, &Addr))
			return false;	// No data at all!
		
		// Non compressed block data?
		if (!D_BSCompareHeader(Header, "ZLIB"))
		{
			zl = false;
			
			// Clone it
			D_BSBaseBlock(a_Stream, Header);
			D_BSWriteChunk(a_Stream, PackData->WrapStream->BlkData, PackData->WrapStream->BlkSize);
			
			// Send host, if any
			if (a_Host)
				memmove(a_Host, &Addr, sizeof(Addr));
		}
		
		// Compressed block data
		else
		{
			zl = true;
			
			// Read Packed Size
			PackSize = D_BSru16(PackData->WrapStream);
			if (PackSize > RBSPACKEDMAXWAITBYTES)
				PackSize = RBSPACKEDMAXWAITBYTES;
			
			// Nothing packed?
			if (!PackSize)
				continue;
			
			// Initialize decompression
			memset(&ZStream, 0, sizeof(ZStream));
			ZStream.avail_out = PackSize;
			ZStream.next_out = PackData->InBuf;
			
			ZStream.avail_in = PackData->WrapStream->BlkSize - 2;
			ZStream.next_in = PackData->WrapStream->BlkData + 2;
			
			if ((ZRet = mz_inflateInit(&ZStream)) != MZ_OK)
			{
				if (devparm)
					CONL_OutputUT(CT_NETWORK, DSTR_DBLOCKC_ZLIBINFLATEERR,
						"%i", ZRet * -1);
				return false;
			}
			
			// Inflate as much as possible
			ZRet = mz_inflate(&ZStream, MZ_FINISH);
			
			// Failed?
			if (!(ZRet == MZ_OK || ZRet == MZ_BUF_ERROR || ZRet == MZ_STREAM_END))
			{
				if (devparm)
					CONL_OutputUT(CT_NETWORK, DSTR_DBLOCKC_ZLIBINFLATEERR,
						"%i", ZRet);
				return false;
			}
			
			// End stream write and calculate input pointers
			Size = PackSize;//RBSPACKEDMAXWAITBYTES - ZStream.avail_out;
			PackData->InAt = &PackData->InBuf[0];
			PackData->InEnd = &PackData->InBuf[Size];
			
			// Cleanup ZLib Stuff
			mz_inflateEnd(&ZStream);
		}
		
#if 0
		// If we are about to trash our address, flush it (to prevent multiple
		// host mishaps with compression streams).
		if (PackData->HasAddr && !I_NetCompareHost(&PackData->CurAddr, &Addr))
			DS_RBSPacked_FlushF(a_Stream);
#endif
		
		// Replace new host
		PackData->HasAddr = true;
		memmove(&PackData->CurAddr, &Addr, sizeof(Addr));
		
		if (!zl)
			return true;
	}
	
	/* Maybe try reading again */
	if (PackData->InAt + 6 <= PackData->InEnd)
	{
		// Read Header
		for (i = 0; i < 4; i++)
			Header[i] = *(PackData->InAt++);
		Header[i] = 0;
		
		// Read Size
		Size = BigReadUInt16(&PackData->InAt);

		// Size exceeds bounds?
		if (PackData->InAt + Size > PackData->InEnd)
			Size = PackData->InEnd - PackData->InAt;
		
		// Base a block
		D_BSBaseBlock(a_Stream, Header);
		D_BSWriteChunk(a_Stream, PackData->InAt, Size);
		PackData->InAt += Size;
		
		// Copy address, if any
		if (a_Host)
			if (PackData->HasAddr)
				memmove(a_Host, &PackData->HasAddr, sizeof(*a_Host));
			else
				memset(a_Host, 0, sizeof(*a_Host));
		
		// Something was read
		return true;
	}
	
	return false;
}

/* DS_RBSPacked_DeleteF() -- Deletes stream */
void DS_RBSPacked_DeleteF(struct D_BS_s* const a_Stream)
{
	DS_RBSPackedData_t* PackData;
	
	/* Get Data */
	PackData = a_Stream->Data;
	
	// Check
	if (!PackData)
		return;
	
	/* Flush */
	// This is so any unwritten compressed data gets written
	DS_RBSPacked_FlushF(a_Stream);
	D_BSFlushStream(a_Stream);
		
	/* Destroy input */
	//D_BSCloseStream(PackData->InputBuf);
	
	/* Destroy Data */
	Z_Free(PackData);
}

/* DS_RBSPacked_IOCtlF() -- Advanced I/O Control */
bool_t DS_RBSPacked_IOCtlF(struct D_BS_s* const a_Stream, const D_BSStreamIOCtl_t a_IOCtl, const intptr_t a_DataP)
{
	DS_RBSPackedData_t* PackData;
	
	/* Get Data */
	PackData = a_Stream->Data;
	
	// Check
	if (!PackData)
		return false;
	
	/* Forward IOCTls */
	return D_BSStreamIOCtl(PackData->WrapStream, a_IOCtl, a_DataP);
}

/****************
*** FUNCTIONS ***
****************/

/* D_BSCreateLoopBackStream() -- Creates loop back stream */
D_BS_t* D_BSCreateLoopBackStream(void)
{
	D_BS_t* New;
	
	/* Create block stream */
	New = Z_Malloc(sizeof(*New), PU_BLOCKSTREAM, NULL);
	
	/* Set Functions */
	New->Data = Z_Malloc(sizeof(DS_RBSLoopBackData_t), PU_BLOCKSTREAM, NULL);
	New->NetRecordF = DS_RBSLoopBack_NetRecordF;
	New->NetPlayF = DS_RBSLoopBack_NetPlayF;
	New->FlushF = DS_RBSLoopBack_FlushF;
	New->DeleteF = DS_RBSLoopBack_DeleteF;
	
	/* Return */
	return New;
}

/* D_BSCreateWLStream() -- Creates a stream that wraps an entry stream */
D_BS_t* D_BSCreateWLStream(WL_ES_t* const a_Stream)
{
	D_BS_t* New;
	
	/* Check */
	if (!a_Stream)
		return NULL;
	
	/* Create block stream */
	New = Z_Malloc(sizeof(*New), PU_BLOCKSTREAM, NULL);
	
	/* Setup Data */
	New->Data = a_Stream;
	New->RecordF = DS_RBSWL_RecordF;
	New->PlayF = DS_RBSWL_PlayF;
	New->DeleteF = DS_RBSWL_DeleteF;
	
	/* Return Stream */
	return New;
}

/* D_BSCreateFileStream() -- Create file stream */
D_BS_t* D_BSCreateFileStream(const char* const a_PathName, const uint32_t a_Flags)
{
	I_File_t* File;
	D_BS_t* New;
	uint32_t Modes;
	
	/* Check */
	if (!a_PathName)
		return NULL;
	
	/* Open r or r/w file */
	if (a_Flags & DRBSSF_READONLY)
		Modes = IFM_READ;
	else
	{
		Modes = IFM_WRITE;
		
		if (a_Flags & DRBSSF_OVERWRITE)
			Modes |= IFM_TRUNCATE;
	}
	
	File = I_FileOpen(a_PathName, Modes);
	
	// Failed?
	if (!File)
		return NULL;
	
	/* Create block stream */
	New = Z_Malloc(sizeof(*New), PU_BLOCKSTREAM, NULL);
	
	/* Setup Data */
	New->Flags = a_Flags;
	New->Data = File;
	New->RecordF = DS_RBSFile_RecordF;
	New->PlayF = DS_RBSFile_PlayF;
	New->DeleteF = DS_RBSFile_DeleteF;
	New->FlushF = DS_RBSFile_FlushF;
	
	/* Return Stream */
	return New;
}

/* D_BSCreateNetStream() -- Create network stream */
D_BS_t* D_BSCreateNetStream(I_NetSocket_t* const a_NetSocket)
{
	D_BS_t* New;
	I_RBSNetSockData_t* NetData;
	
	/* Check */
	if (!a_NetSocket)
		return NULL;
	
	/* Create block stream */
	New = Z_Malloc(sizeof(*New), PU_BLOCKSTREAM, NULL);
	
	/* Setup Data */
	New->Data = NetData = Z_Malloc(sizeof(*NetData), PU_BLOCKSTREAM, NULL);
	New->NetRecordF = DS_RBSNet_NetRecordF;
	New->NetPlayF = DS_RBSNet_NetPlayF;
	New->DeleteF = DS_RBSNet_DeleteF;
	New->IOCtlF = DS_RBSNet_IOCtlF;
	
	/* Load into data */
	NetData->Socket = a_NetSocket;
	
	/* Return Stream */
	return New;
}

/* D_BSCreatePackedStream() -- Creates a packed stream */
D_BS_t* D_BSCreatePackedStream(D_BS_t* const a_Wrapped)
{
	D_BS_t* New;
	DS_RBSPackedData_t* Data;
	intptr_t Trans;
	
	/* Check */
	if (!a_Wrapped)
		return NULL;
	
	/* Create block stream */
	New = Z_Malloc(sizeof(*New), PU_BLOCKSTREAM, NULL);
	
	/* Setup Data */
	New->Data = Data = Z_Malloc(sizeof(*Data), PU_BLOCKSTREAM, NULL);
	New->NetRecordF = DS_RBSPacked_NetRecordF;
	New->NetPlayF = DS_RBSPacked_NetPlayF;
	New->FlushF = DS_RBSPacked_FlushF;
	New->DeleteF = DS_RBSPacked_DeleteF;
	New->IOCtlF = DS_RBSPacked_IOCtlF;
	
	// Load stuff into data
	Data->Self = New;
	Data->WrapStream = a_Wrapped;
	if (D_BSStreamIOCtl(a_Wrapped, DRBSIOCTL_MAXTRANSPORT, &Trans))
		Data->TransSize = Trans;
	else
		Data->TransSize = 2048;
	Data->OutAt = Data->OutBuf;
	Data->InAt = Data->InBuf;
	Data->InEnd = Data->InBuf;
	//Data->InputBuf = D_BSCreateLoopBackStream();
	
	/* Return Stream */
	return New;
}

/* D_BSCloseStream() -- Closes File Stream */
void D_BSCloseStream(D_BS_t* const a_Stream)
{
	/* Check */
	if (!a_Stream)
		return;
	
	/* Call deletion */
	if (a_Stream->DeleteF)
		a_Stream->DeleteF(a_Stream);
	
	/* Free it */
	if (a_Stream->BlkData)
		Z_Free(a_Stream->BlkData);
	Z_Free(a_Stream);
}

/* D_BSStreamIOCtl() -- Call special IOCtl Handler on stream */
bool_t D_BSStreamIOCtl(D_BS_t* const a_Stream, const D_BSStreamIOCtl_t a_IOCtl, const intptr_t a_DataP)
{
	/* Check */
	if (!a_Stream || a_IOCtl < 0 || a_IOCtl >= NUMDRBSSTREAMIOCTL)
		return false;
	
	/* Call IOCtl */
	if (a_Stream->IOCtlF)
		return a_Stream->IOCtlF(a_Stream, a_IOCtl, a_DataP);
	
	/* There is no IOCtl */
	return false;
}

/* D_BSCompareHeader() -- Compares headers */
bool_t D_BSCompareHeader(const char* const a_A, const char* const a_B)
{
	/* Check */
	if (!a_A || !a_B)
		return false;
	
	/* Compare */
	if (tolower(a_A[0]) == tolower(a_B[0]) &&
		tolower(a_A[1]) == tolower(a_B[1]) &&
		tolower(a_A[2]) == tolower(a_B[2]) &&
		tolower(a_A[3]) == tolower(a_B[3]))
		return true;
	
	/* No match */
	return false;
}

/* D_BSBaseBlock() -- Base block */
// This used to return void, but I need it to return true for the SaveGame code
bool_t D_BSBaseBlock(D_BS_t* const a_Stream, const char* const a_Header)
{
	int32_t i;
	
	/* Check */
	if (!a_Stream || !a_Header)
		return false;
	
	/* Clear Everything */
	memset(a_Stream->BlkHeader, 0, sizeof(a_Stream->BlkHeader));
	
	if (a_Stream->BlkData)
		Z_Free(a_Stream->BlkData);
	a_Stream->BlkSize = a_Stream->BlkBufferSize = 0;
	
	/* Create a fresh block */
	a_Stream->BlkBufferSize = RBLOCKBUFSIZE;
	a_Stream->BlkData = Z_Malloc(a_Stream->BlkBufferSize, PU_BLOCKSTREAM, NULL);
	a_Stream->ReadOff = 0;
	
	// Copy header
	for (i = 0; i < 4; i++)
		a_Stream->BlkHeader[i] = a_Header[i];
	
	/* Success */
	return true;
}

/* D_BSRenameHeader() -- Rename block */
bool_t D_BSRenameHeader(D_BS_t* const a_Stream, const char* const a_Header)
{
	/* Check */
	if (!a_Stream || !a_Header)
		return false;
	
	/* Copy header */
	memmove(a_Stream->BlkHeader, a_Header, (strlen(a_Header) >= 4 ? 4 : strlen(a_Header)));
	
	/* Success */
	return true;
}

/* D_BSRecordBlock() -- Records the current block to the stream */
void D_BSRecordBlock(D_BS_t* const a_Stream)
{
	/* Check */
	if (!a_Stream)
		return;
	
	/* Call recorder */
	if (a_Stream->RecordF)
		a_Stream->RecordF(a_Stream);
	else if (a_Stream->NetRecordF)
		a_Stream->NetRecordF(a_Stream, NULL);
		
	/* Increase stats */
	a_Stream->StatBlock[1]++;
	a_Stream->StatBytes[1] += a_Stream->BlkSize;
}

/* D_BSRecordBlock() -- Plays the current block from the stream */
bool_t D_BSPlayBlock(D_BS_t* const a_Stream, char* const a_Header)
{
	/* Check */
	if (!a_Stream)
		return false;
	
	/* Call recorder */
	// Local
	if (a_Stream->PlayF)
	{
		if (a_Stream->PlayF(a_Stream))
		{
			// Stats
			a_Stream->StatBlock[0]++;
			a_Stream->StatBytes[0] += a_Stream->BlkSize;
			
			// Copy header
			if (a_Header)
				memmove(a_Header, a_Stream->BlkHeader, 4);
				
			// Read a block
			return true;
		}
	}
	
	// Networked
	else if (a_Stream->NetPlayF)
	{
		if (a_Stream->NetPlayF(a_Stream, NULL))
		{
			// Stats
			a_Stream->StatBlock[0]++;
			a_Stream->StatBytes[0] += a_Stream->BlkSize;
			
			// Copy header
			if (a_Header)
				memmove(a_Header, a_Stream->BlkHeader, 4);
				
			// Read a block
			return true;
		}
	}
	
	return false;
}

/* D_BSPlayNetBlock() -- Plays net block */
bool_t D_BSPlayNetBlock(D_BS_t* const a_Stream, char* const a_Header, I_HostAddress_t* const a_Host)
{
	/* Check */
	if (!a_Stream)
		return false;
		
	/* Clear */
	if (a_Host)
		memset(a_Host, 0, sizeof(*a_Host));
	
	/* Call recorder */
	// Networked
	if (a_Stream->NetPlayF)
	{
		if (a_Stream->NetPlayF(a_Stream, a_Host))
		{
			// Stats
			a_Stream->StatBlock[0]++;
			a_Stream->StatBytes[0] += a_Stream->BlkSize;
			
			// Copy header
			if (a_Header)
				memmove(a_Header, a_Stream->BlkHeader, 4);
				
			// Read a block
			return true;
		}
	}
	
	// Local
	else if (a_Stream->PlayF)
	{
		if (a_Stream->PlayF(a_Stream))
		{
			// Stats
			a_Stream->StatBlock[0]++;
			a_Stream->StatBytes[0] += a_Stream->BlkSize;
			
			// Copy header
			if (a_Header)
				memmove(a_Header, a_Stream->BlkHeader, sizeof(a_Stream->BlkHeader));
				
			// Read a block
			return true;
		}
	}
	
	return false;
}

/* D_BSRecordNetBlock() -- Record net block */
void D_BSRecordNetBlock(D_BS_t* const a_Stream, I_HostAddress_t* const a_Host)
{
	/* Check */
	if (!a_Stream)
		return;
	
	/* Call recorder */
	if (a_Stream->NetRecordF)
		a_Stream->NetRecordF(a_Stream, a_Host);
	else if (a_Stream->RecordF)
		a_Stream->RecordF(a_Stream);
		
	/* Increase stats */
	a_Stream->StatBlock[1]++;
	a_Stream->StatBytes[1] += a_Stream->BlkSize;
}

/* D_BSFlushStream() -- Flush stream */
bool_t D_BSFlushStream(D_BS_t* const a_Stream)
{
	/* Check */
	if (!a_Stream)
		return false;
	
	/* Call flusher */
	if (a_Stream->FlushF)
		return a_Stream->FlushF(a_Stream);
	
	/* Flusher not called */
	return false;
}

/* D_BSRewind() -- Rewinds the selected stream */
void D_BSRewind(D_BS_t* const a_Stream)
{
	/* Check */
	if (!a_Stream)
		return;
	
	/* Reset read position */
	a_Stream->ReadOff = 0;
}

/* D_BSWriteChunk() -- Write data chunk into block */
size_t D_BSWriteChunk(D_BS_t* const a_Stream, const void* const a_Data, const size_t a_Size)
{
	size_t DestSize;
	
	/* Check */
	if (!a_Stream || !a_Data || !a_Size)
		return 0;
	
	/* Determine Block Resize */
	DestSize = a_Stream->BlkSize + a_Size;
	
	// Too big?
	while (DestSize >= a_Stream->BlkBufferSize)
	{
		Z_ResizeArray((void**)&a_Stream->BlkData, sizeof(*a_Stream->BlkData), a_Stream->BlkBufferSize, a_Stream->BlkBufferSize + RBLOCKBUFSIZE);
		a_Stream->BlkBufferSize += RBLOCKBUFSIZE;
	}
	
	/* Slap data there */
	memmove(&a_Stream->BlkData[a_Stream->BlkSize], a_Data, a_Size);
	a_Stream->BlkSize += a_Size;
	
	/* Return size */
	return a_Size;
}

#define BP_MERGE(a,b) a##b
#define __REMOOD_BSQUICK(w,x,ww) void BP_MERGE(D_BSw,ww)(D_BS_t* const a_Stream, const x a_Val)\
{\
	x Val = BP_MERGE(LittleSwap,w)(a_Val);\
	D_BSWriteChunk(a_Stream, &Val, sizeof(Val));\
}

__REMOOD_BSQUICK(Int8,int8_t,i8);
__REMOOD_BSQUICK(Int16,int16_t,i16);
__REMOOD_BSQUICK(Int32,int32_t,i32);
__REMOOD_BSQUICK(Int64,int64_t,i64);
__REMOOD_BSQUICK(UInt8,uint8_t,u8);
__REMOOD_BSQUICK(UInt16,uint16_t,u16);
__REMOOD_BSQUICK(UInt32,uint32_t,u32);
__REMOOD_BSQUICK(UInt64,uint64_t,u64);

/* D_BSws() -- Write Version String */
void D_BSws(D_BS_t* const a_Stream, const char* const a_Val)
{
	const char* c;
	
	/* Check */
	if (!a_Stream)
		return;
	
	/* Constant Write */
	if (a_Val)	// So NULL can be sent as just \0
		for (c = a_Val; *c; c++)
			D_BSwu8(a_Stream, *c);
	D_BSwu8(a_Stream, 0);	// NUL
}

/* D_BSwp() -- Write Pointer */
void D_BSwp(D_BS_t* const a_Stream, const void* const a_Ptr)
{
	size_t i;
	uint64_t XP;
	
	/* Write sizeof() */
	D_BSwu8(a_Stream, sizeof(a_Ptr));
	
	/* Write all the pointer bits (for UUID in a way) */
	XP = (uint64_t)((uintptr_t)a_Ptr);
	for (i = 0; i < sizeof(a_Ptr); i++)
		D_BSwu8(a_Stream, (uint8_t)(((XP >> ((UINT64_C(8) * ((uint64_t)i)))) & UINT64_C(0xFF))));
		//OP |= ((uint64_t)D_BSru8(a_Stream)) << ((UINT64_C(8) * ((uint64_t)i)));
}

/* D_BSReadChunk() -- Reads Chunk */
size_t D_BSReadChunk(D_BS_t* const a_Stream, void* const a_Data, const size_t a_Size)
{
	size_t Count;
	size_t Read;
	uint8_t Bit;
	
	/* Check */
	if (!a_Stream || !a_Data || !a_Size)
		return 0;
	
	/* Counting Read */
	Read = 0;
	for (Count = 0; Count < a_Size; Count++)
	{
		Bit = 0;
		if (a_Stream->ReadOff < a_Stream->BlkSize)
			Bit = a_Stream->BlkData[a_Stream->ReadOff++];
		
		((uint8_t*)a_Data)[Read++] = Bit;
	}
	
	/* Return read amount */
	return Read;
}

#define BP_MERGE(a,b) a##b
#define __REMOOD_BSQUICKREAD(w,x,ww) x BP_MERGE(D_BSr,ww)(D_BS_t* const a_Stream)\
{\
	x Ret;\
	D_BSReadChunk(a_Stream, &Ret, sizeof(Ret));\
	return BP_MERGE(LittleSwap,w)(Ret);\
}

__REMOOD_BSQUICKREAD(Int8,int8_t,i8);
__REMOOD_BSQUICKREAD(Int16,int16_t,i16);
__REMOOD_BSQUICKREAD(Int32,int32_t,i32);
__REMOOD_BSQUICKREAD(Int64,int64_t,i64);
__REMOOD_BSQUICKREAD(UInt8,uint8_t,u8);
__REMOOD_BSQUICKREAD(UInt16,uint16_t,u16);
__REMOOD_BSQUICKREAD(UInt32,uint32_t,u32);
__REMOOD_BSQUICKREAD(UInt64,uint64_t,u64);

/* D_BSrs() -- Read String */
size_t D_BSrs(D_BS_t* const a_Stream, char* const a_Out, const size_t a_OutSize)
{
	size_t i;
	uint8_t Char;
	
	/* Check */
	if (!a_Stream)
		return 0;
	
	/* Read variable string data */
	for (i = 0; ; i++)
	{
		// Read Character
		Char = D_BSru8(a_Stream);
		
		// End of string?
		if (!Char)
		{
			// Append NUL, if possible
			if (a_Out && i < a_OutSize)
				a_Out[i] = 0;
			break;
		}
		
		// Otherwise add to the output
		if (a_Out && i < a_OutSize)
			a_Out[i] = Char;
	}
	
	/* Always put NUL at very end */
	if (a_Out)
		a_Out[a_OutSize - 1] = 0;
	
	/* Return read count */
	return i;	
}

/* D_BSrp() -- Reads pointer */
uint64_t D_BSrp(D_BS_t* const a_Stream)
{
	size_t i;
	uint8_t SizeOf;
	uint64_t OP;
	
	/* Read sizeof() */
	SizeOf = D_BSru8(a_Stream);
	
	/* Read in pointer bits */
	OP = 0;
	for (i = 0; i < SizeOf; i++)
		OP |= ((uint64_t)D_BSru8(a_Stream)) << ((UINT64_C(8) * ((uint64_t)i)));
	
	/* Return the number */
	return OP;
}

/* D_BSrcu64() -- Reads compressed uint64_t */
uint64_t D_BSrcu64(D_BS_t* const a_Stream)
{
	uint64_t RetVal, Shift;
	uint8_t Read;
	
	/* Constantly read */
	RetVal = Shift = 0;
	do
	{
		// Read single byte
		Read = D_BSru8(a_Stream);
		
		// Shift it in
		RetVal |= ((uint64_t)(Read & UINT8_C(0x7F))) << Shift;
		Shift += 7;
	} while (Read & UINT8_C(0x80));
	
	/* Return it */
	return RetVal;
}

/* D_BSwcu64() -- Writes compressed uint64_t */
void D_BSwcu64(D_BS_t* const a_Stream, const uint64_t a_Val)
{
	uint64_t Left;
	uint8_t Write;
	
	/* Write Loop */
	Left = a_Val;
	
	// While there is something left
	do
	{
		Write = Left & 0x7F;
		Left >>= 7;
		if (Left)
			Write |= 0x80;
		D_BSwu8(a_Stream, Write);
	} while (Write & 0x80);
}

/* D_BSrhost() -- Reads host name from stream */
void D_BSrhost(D_BS_t* const a_Stream, I_HostAddress_t* const a_Out)
{
}

/* D_BSwhost() -- Writes host name to stream */
void D_BSwhost(D_BS_t* const a_Stream, const I_HostAddress_t* const a_In)
{
}


