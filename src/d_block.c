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
		fclose((FILE*)a_Stream->Data);
		a_Stream->Data = NULL;
	}
}

/* DS_RBSFile_RecordF() -- Records the current block */
static size_t DS_RBSFile_RecordF(struct D_BS_s* const a_Stream)
{
	FILE* File;
	size_t RetVal;
	uint32_t BlockLen, i;
	uint32_t QuickSum;
	
	/* Check */
	if (!a_Stream)
		return 0;
	
	/* Get Data */
	File = (FILE*)a_Stream->Data;
	
	/* Flush for write */
	fflush(File);
	
	/* Write Header */
	fwrite(a_Stream->BlkHeader, 4, 1, File);
	BlockLen = a_Stream->BlkSize;
	BlockLen = LittleSwapUInt32(BlockLen);
	fwrite(&BlockLen, sizeof(BlockLen), 1, File);
	
	/* Determine Quicksum */
	for (i = 0; i < a_Stream->BlkSize; i++)
		if (i & 1)
			QuickSum ^= (((uint8_t*)a_Stream->BlkData)[i]) << ((i & 2) ? 4 : 0);
		else
			QuickSum ^= (~(((uint8_t*)a_Stream->BlkData)[i])) << ((i & 2) ? 6 : 2);
	QuickSum = LittleSwapUInt32(QuickSum);
	fwrite(&QuickSum, sizeof(QuickSum), 1, File);
	
	/* Write Data */
	RetVal = fwrite(a_Stream->BlkData, a_Stream->BlkSize, 1, File);
	
	/* Flush for write */
	fflush(File);
	
	return RetVal;
}

/* DS_RBSFile_PlayF() -- Play from file */
bool_t DS_RBSFile_PlayF(struct D_BS_s* const a_Stream)
{
	FILE* File;
	char Header[5];
	uint32_t Len, Sum;
	void* Data;
	
	/* Check */
	if (!a_Stream)
		return false;
		
	/* Get Data */
	File = (FILE*)a_Stream->Data;
	
	/* Flush for read */
	fflush(File);
	
	/* Read Header */
	// Clear
	memset(Header, 0, sizeof(Header));
	Len = Sum = 0;
	Data = NULL;
	
	// Start reading and be sure to check if the read failed!!!
	if (fread(&Header[0], 4, 1, File) < 1)
		return false;
		
	if (fread(&Len, sizeof(Len), 1, File) < 1)
		return false;
	
	if (fread(&Sum, sizeof(Sum), 1, File) < 1)
		return false;
	
	// Endian Correct Values
	Len = LittleSwapUInt32(Len);
	Sum = LittleSwapUInt32(Sum);
	
	// Read data, if possible (Len could be zero (empty block?))
	if (Len > 0)
	{
		Data = Z_Malloc(Len, PU_STATIC, NULL);
		if (fread(Data, Len, 1, File) < 1)
		{
			Z_Free(Data);
			return false;
		}
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
	uint32_t Len, Sum, i;
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
	Len = Sum = 0;
	Data = NULL;
	
	// Read Header
	for (i = 0; i < 4; i++)
		Header[i] = WL_Src(Stream);
	
	// Read Length and Sum
	Len = WL_Srlu32(Stream);
	Sum = WL_Srlu32(Stream);
	
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

/* DS_RBSLoopBack_RecordF() -- Records a block */
size_t DS_RBSLoopBack_RecordF(struct D_BS_s* const a_Stream)
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
	Hold->FlushID = LoopData->FlushID + 1;
	
	/* Return value does not matter */
	return 1;
}

/* DS_RBSLoopBack_PlayF() -- Backs a block back */
bool_t DS_RBSLoopBack_PlayF(struct D_BS_s* const a_Stream)
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
#define IRBSNETSOCKBUFSIZE 2048
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
	uint32_t lenSize;
	
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
	sz = 4 + 4 + ChunkSize;
	
	//TBuf = Z_Malloc(sz, PU_BLOCKSTREAM, NULL);//alloca(sz);
	memset(NetData->WriteBuf, 0, sizeof(NetData->WriteBuf));
	TBuf = NetData->WriteBuf;
	
	// Write to it
		// Header
	for (i = 0; i < 4; i++)
		TBuf[i] = a_Stream->BlkHeader[i];
	lenSize = LittleSwapUInt32((uint32_t)ChunkSize);
	memmove(&(((uint32_t*)TBuf)[1]), &lenSize, sizeof(lenSize));
	memmove(&(((uint32_t*)TBuf)[2]), a_Stream->BlkData, ChunkSize);
	
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
	uint32_t Len;
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
	if ((RetVal = I_NetRecv(Socket, a_Host, NetData->ReadBuf, IRBSNETSOCKBUFSIZE)) < 8)
		return false;
	
	/* Extract information */
	memset(Header, 0, sizeof(Header));
	memmove(Header, NetData->ReadBuf, 4);
	memmove(&Len, NetData->ReadBuf + 4, 4);
	
	// Swap Len
	Len = LittleSwapUInt32(Len);
	
	// Limit
	if (Len >= IRBSNETSOCKBUFSIZE - 8)
		Len = IRBSNETSOCKBUFSIZE - 8 - 1;
	
	// Length Limit?
	if (!Len)
		return false;
	
	/* Write data into block */
	D_BSBaseBlock(a_Stream, Header);
	D_BSWriteChunk(a_Stream, NetData->ReadBuf + 8, Len);
	
	/* Was played back */
	return true;
}

/* DS_RBSNet_IOCtlF() -- IO Ctl */
static bool_t DS_RBSNet_IOCtlF(struct D_BS_s* const a_Stream, const D_BSStreamIOCtl_t a_IOCtl, intptr_t* a_DataP)
{
	/* Check */
	if (!a_DataP)
		return false;
	
	/* Which? */
	switch (a_IOCtl)
	{
			// Maximum transport
		case DRBSIOCTL_MAXTRANSPORT:
			*a_DataP = IRBSNETSOCKBUFSIZE - 16;
			return true;
			
			// Unknown
		default:
			return false;
	}
}

/*********************
*** PERFECT STREAM ***
*********************/

#if defined(__REMOOD_WELIKEBEINGDOSED)
	#define PERFECTKEYEXPIRETIME		300000	// Time until key expires (in MS, 5min)
	#define PERFECTRETRANSTIME			1000	// If no ack recieved, retransmit (1s)
	#define PERFECTMAXRETRIES			150		// Max before key revocation (1s * 150 = 150s/2.5m)
	
	#define PERFECTMAXENQ				512		// max packets before revoke
#else
	#define PERFECTKEYEXPIRETIME		30000	// Time until key expires (in MS, 5min)
	#define PERFECTRETRANSTIME			2000	// If no ack recieved, retransmit (1s)
	#define PERFECTMAXRETRIES			15		// Max before key revocation (2s * 150 = 150s/2.5m)
	
	#define PERFECTMAXENQ				256		// max packets before revoke
#endif

/* DS_RBSPerfectKey_t -- Key for perfect entry */
typedef struct DS_RBSPerfectKey_s
{
	uint32_t Key[4];
	I_HostAddress_t RemHost;					// Remote Host
	uint64_t NextReadNum;						// Next packet to read
	uint64_t NextWriteNum;						// Next packet to write
	
	uint32_t CreateTime;						// Time when key was created
	uint32_t LastTime;							// Last activity time
	uint32_t ExpireTime;						// Time when key expires
	bool_t ExpireLessThan;						// Expire time is in the past
	bool_t Revoke;								// Revoke key
} DS_RBSPerfectKey_t; 

/* DS_RBSPerfectHold_t -- Holding store */
typedef struct DS_RBSPerfectHold_s
{
	/* Standard */
	char Header[5];								// Header
	uint8_t* Data;								// Data
	size_t Size;								// Size
	
	/* Perfect Networking */
	uint32_t ReTryCount;						// Retransmit count
	uint64_t PacketNum;							// Packet Number
	uint32_t CheckSum;							// Simplified Checksum
	uint32_t ClockTime;							// Time packet was stored
	uint32_t AutoRackTime;						// time to retransmit
	bool_t RackTimeIsLess;						// Less than
	bool_t ReTransmit;							// Retransmit block
	bool_t BlockAck;							// Block acknowledged
	I_HostAddress_t RemHost;					// Remote Host
	
	// Packet Key
	uint32_t Key[4];							// Key Holder
} DS_RBSPerfectHold_t;

/* DS_RBSPerfectData_t -- Perfect Stream */
typedef struct DS_RBSPerfectData_s
{
	D_BS_t* WrapStream;				// Stream to wrap
	bool_t InFlush;								// In the middle of a flush
	
	DS_RBSPerfectHold_t** ReadQ;				// Blocks to read Q
	size_t SizeReadQ;							// Size of read Q
	
	DS_RBSPerfectHold_t** WriteQ;				// Blocks to read Q
	size_t SizeWriteQ;							// Size of read Q
	
	DS_RBSPerfectKey_t** Keys;					// Packet keys
	size_t NumKeys;								// Number of keys
	
	bool_t Debug;								// Debug it
} DS_RBSPerfectData_t;

/* DS_RBSPerfect_DeleteF() -- Delete perfect stream */
static void DS_RBSPerfect_DeleteF(struct D_BS_s* const a_Stream)
{
	size_t i;
	DS_RBSPerfectData_t* PerfectData;
	
	/* Check */
	if (!a_Stream)
		return;
	
	/* Get Data */
	PerfectData = (DS_RBSPerfectData_t*)a_Stream->Data;
	
	// Check
	if (!PerfectData)
		return;
	
	/* Delete blocks */
	// Read Queue
	if (PerfectData->ReadQ)
	{
		for (i = 0; i < PerfectData->SizeReadQ; i++)
			if (PerfectData->ReadQ[i])
				Z_Free(PerfectData->ReadQ[i]);
		Z_Free(PerfectData->ReadQ);
	}
			
	// Write Queue
	if (PerfectData->WriteQ)
	{
		for (i = 0; i < PerfectData->SizeWriteQ; i++)
			if (PerfectData->WriteQ[i])
				Z_Free(PerfectData->WriteQ[i]);
		Z_Free(PerfectData->WriteQ);
	}
	
	/* Delete keys */
	if (PerfectData->Keys)
	{
		for (i = 0; i < PerfectData->NumKeys; i++)
			if (PerfectData->Keys[i])
				Z_Free(PerfectData->Keys[i]);
		Z_Free(PerfectData->Keys);
	}
	
	/* Delete Data */
	Z_Free(PerfectData);
}

/* DS_RBSPerfect_IntFindKey() -- Finds the correct key this belongs to */
static DS_RBSPerfectKey_t* DS_RBSPerfect_IntFindKey(struct D_BS_s* const a_Stream, DS_RBSPerfectData_t* const a_PerfectData, const uint32_t* const a_InKey, I_HostAddress_t* const a_Host)
{
	size_t k, FbK, j, i, b, z;
	DS_RBSPerfectKey_t* Key;
	uint32_t ThisTime;
	DS_RBSPerfectHold_t* Hold;
	
	/* Check */
	if (!a_Stream || !a_PerfectData || !a_Host)
		return NULL;
	
	/* Get Current Time */
	ThisTime = I_GetTimeMS();
	
	/* Look in existing key list */
	for (FbK = 0, k = 0; k < a_PerfectData->NumKeys; k++)
	{
		// Get Key
		Key = a_PerfectData->Keys[k];
		
		// No key here?
		if (!Key)
		{
			if (!FbK)
				FbK = k;
			continue;
		}
		
		// Key expires? Revoke it
		if ((!Key->ExpireLessThan && ThisTime >= Key->ExpireTime) ||
			(Key->ExpireLessThan && Key->ExpireTime >= ThisTime))
		{
			Key->Revoke = true;
		}
		
		// Revoke Key?
		if (Key->Revoke)
		{
			// Go through all read/write blocks and removed all matching keys
				// Read Q
			for (b = 0; b < a_PerfectData->SizeReadQ; b++)
				if (a_PerfectData->ReadQ[b])
				{
					// Get Block
					Hold = a_PerfectData->ReadQ[b];
					
					// Key match?
					for (z = 0; z < 4; z++)
						if (Hold->Key[z] != Key->Key[z])
							break;
					
					// Matches? Then delete it
					if (z >= 4)
					{
						if (Hold->Data)
							Z_Free(Hold->Data);
						Hold->Data = NULL;
						Z_Free(Hold);
						a_PerfectData->ReadQ[b] = NULL;
					}
				}
				
				// Write Q
			for (b = 0; b < a_PerfectData->SizeWriteQ; b++)
				if (a_PerfectData->WriteQ[b])
				{
					// Get Block
					Hold = a_PerfectData->WriteQ[b];
					
					// Key match?
					for (z = 0; z < 4; z++)
						if (Hold->Key[z] != Key->Key[z])
							break;
					
					// Matches? Then delete it
					if (z >= 4)
					{
						if (Hold->Data)
							Z_Free(Hold->Data);
						Hold->Data = NULL;
						Z_Free(Hold);
						a_PerfectData->WriteQ[b] = NULL;
					}
				}
			
			// Delete self key
			Z_Free(Key);
			Key = a_PerfectData->Keys[k] = NULL;
			return NULL;	// Was revoked
		}
		
		// If a key was passed, compare it
		if (a_InKey)
		{
			// Match each
			for (i = 0; i < 4; i++)
				if (a_InKey[i] != Key->Key[i])
					break;
			
			// No match?
			if (i < 4)
				continue;
		}
		
		// Otherwise do host based authentication
		else
		{
			// Match IP
			if (!I_NetCompareHost(a_Host, &Key->RemHost))
				continue;
		}
	
		// Bump expiration
		Key->LastTime = ThisTime;
		Key->ExpireTime = Key->LastTime + PERFECTKEYEXPIRETIME;

		Key->ExpireLessThan = false;
		if (Key->ExpireTime < Key->LastTime)
			Key->ExpireLessThan = true;
		
		// Return matched key
		return Key;
	}
	
	/* If this point was reached then the key does not exist */
	// Use pre-existing blank spot
	if (FbK > 0 && FbK < a_PerfectData->NumKeys)
		Key = a_PerfectData->Keys[FbK] = Z_Malloc(sizeof(*Key), PU_STATIC, NULL);
	
	// Resize the key array
	else
	{
		Z_ResizeArray((void**)&a_PerfectData->Keys, sizeof(*a_PerfectData->Keys),
			a_PerfectData->NumKeys, a_PerfectData->NumKeys + 1);
		Key = a_PerfectData->Keys[a_PerfectData->NumKeys++] = Z_Malloc(sizeof(*Key), PU_STATIC, NULL);
	}
	
	// Setup key info
	memmove(&Key->RemHost, a_Host, sizeof(*a_Host));
	Key->CreateTime = ThisTime;
	Key->LastTime = Key->CreateTime;
	Key->ExpireTime = Key->LastTime + PERFECTKEYEXPIRETIME;
	
	Key->ExpireLessThan = false;
	if (Key->ExpireTime < Key->LastTime)
		Key->ExpireLessThan = true;
	
	/* Generate Hopefully Random Key Data */
	if (a_InKey)
		for (i = 0; i < 4; i++)
			Key->Key[i] = a_InKey[i];
	else
		for (i = 0; i < 4; i++)
		{
			// Base
			Key->Key[i] = ThisTime * (i + 1);
		
			// Cycle
			for (j = 0; j < 16; j++)
				Key->Key[i] ^= (j + (M_Random() * M_Random()) + (((uintptr_t)a_PerfectData) - ((uintptr_t)a_InKey))) << ((j + i) & 24);
		}
	
	/* Place in empty spot */
	// First known
	if (FbK > 0 && FbK < a_PerfectData->NumKeys)
		a_PerfectData->Keys[FbK] = Key;
	
	/* Return the new key */
	return Key;
}

/* DS_RBSPerfect_NetRecordF() -- Write block to perfect stream */
static size_t DS_RBSPerfect_NetRecordF(struct D_BS_s* const a_Stream, I_HostAddress_t* const a_Host)
{
	size_t i, b, z;
	DS_RBSPerfectData_t* PerfectData;
	DS_RBSPerfectHold_t* Hold;
	DS_RBSPerfectKey_t* Key;
	bool_t KeepSending;
	uint32_t ThisTime, MaskEnc;
	D_BS_t* NormStream;
	uint32_t PerfPkNumLow, PerfPkNumHi;
	
	/* Check */
	if (!a_Stream)
		return 0;
	
	/* Get Data */
	PerfectData = (DS_RBSPerfectData_t*)a_Stream->Data;
	
	// Check
	if (!PerfectData)
		return 0;
	
	/* Get Time */
	ThisTime = I_GetTimeMS();
	
	/* First find the key to use */
	// Don't do this in the middle of a flush (since there is no data to record)
	if (!PerfectData->InFlush)
	{
		Key = DS_RBSPerfect_IntFindKey(a_Stream, PerfectData, NULL, a_Host);
	
		// Second record the packet into the write Q
		if (Key)
		{
			// Find blank spot
			Hold = NULL;
			for (i = 0; i < PerfectData->SizeWriteQ; i++)
				if (!PerfectData->WriteQ[i])
				{
					Hold = PerfectData->WriteQ[i] = Z_Malloc(sizeof(*Hold), PU_BLOCKSTREAM, NULL);
					break;
				}
	
			// No blank spots?
			if (!Hold)
			{
				Z_ResizeArray((void**)&PerfectData->WriteQ, sizeof(*PerfectData->WriteQ),
								PerfectData->SizeWriteQ, PerfectData->SizeWriteQ + 2);
				Hold = PerfectData->WriteQ[PerfectData->SizeWriteQ++] =
						Z_Malloc(sizeof(*Hold), PU_BLOCKSTREAM, NULL);
				PerfectData->SizeWriteQ++;
				
				if (PerfectData->SizeWriteQ >= PERFECTMAXENQ)
				{
					Key->Revoke = true;
				}
			}
	
			// Store block in the hold
				// Header
			memmove(Hold->Header, a_Stream->BlkHeader, 4);
	
				// Data
			Hold->Size = a_Stream->BlkSize;
			Hold->Data = Z_Malloc(Hold->Size, PU_BLOCKSTREAM, NULL);
			memmove(Hold->Data, a_Stream->BlkData, Hold->Size);
	
				// Remote Host
			if (a_Host)
				memmove(&Hold->RemHost, a_Host, sizeof(*a_Host));
		
				// Determine some kind of sum
			for (i = 0; i < Hold->Size; i++)
				Hold->CheckSum ^= ((uint32_t)Hold->Data[i]) << (i & 23);
		
				// Time packet appeared
			Hold->ClockTime = ThisTime;
			Hold->AutoRackTime = Hold->ClockTime + PERFECTRETRANSTIME;
			Hold->RackTimeIsLess = false;
			if (Hold->AutoRackTime < Hold->AutoRackTime)
				Hold->RackTimeIsLess = true;
	
				// I/O Order
			Hold->PacketNum = Key->NextWriteNum++;
			Hold->ReTransmit = true;
		
				// Copy Key
			for (i = 0; i < 4; i++)
				Hold->Key[i] = Key->Key[i];
		}
	}
	
	/* Third, write any queued writes to the network */
	KeepSending = true;
	do
	{
		// Find block in Q to write to the network
		Hold = NULL;
		Key = NULL;
		
		// Go through each block in the Q
		for (b = 0; b < PerfectData->SizeWriteQ; b++)
		{
			// Get Current Hold
			Hold = PerfectData->WriteQ[b];
			
			// Nothing here?
			if (!Hold)
				continue;
			
			// Obtain key from block
			Key = DS_RBSPerfect_IntFindKey(a_Stream, PerfectData, Hold->Key, &Hold->RemHost);
			
			// No key? Must have expired
			if (!Key)
			{
				// Delete block, only IF this spot is blank
					// because if a key was revoked as we tried to grab it, then
					// this could be invalid!
				if (PerfectData->WriteQ[b])
				{
					if (Hold->Data)
						Z_Free(Hold->Data);
					Hold->Data = NULL;
					Z_Free(Hold);
				}
				
				PerfectData->WriteQ[b] = NULL;
				continue;
			}
			
			// Packet was ACKed!
			if (Hold->BlockAck)
			{
				// Delete block
				if (Hold->Data)
					Z_Free(Hold->Data);
				Hold->Data = NULL;
				Z_Free(Hold);
				PerfectData->WriteQ[b] = NULL;
				
				// Increase count
				//Key->NextWriteNum++;
				continue;
			}
			
			// Retry count exceeded, revoke keys
			if (Hold->ReTryCount >= PERFECTMAXRETRIES)
			{
				// Delete block
				if (Hold->Data)
					Z_Free(Hold->Data);
				Hold->Data = NULL;
				Z_Free(Hold);
				PerfectData->WriteQ[b] = NULL;
				
				// Revoke key
				Key->Revoke = true;
				continue;
			}
			
			// Retransmit request?
			if ((!Hold->RackTimeIsLess && ThisTime >= Hold->AutoRackTime) ||
				(Hold->RackTimeIsLess && Hold->AutoRackTime >= ThisTime))
			{
				// Re-transmit
				Hold->ReTransmit = true;
				Hold->ReTryCount++;
				
				// Update Time
				Hold->AutoRackTime = ThisTime + PERFECTRETRANSTIME;
				Hold->RackTimeIsLess = false;
				if (Hold->AutoRackTime < Hold->AutoRackTime)
					Hold->RackTimeIsLess = true;
			}
			
			// Re-transmitting block?
			if (Hold->ReTransmit)
			{
				//CONL_PrintF("retran!\n");
				// Don't retransmit constantly
				Hold->ReTransmit = false;
				
				// Get encapsulated stream out
				NormStream = PerfectData->WrapStream;
				
				// Write to remote host
				D_BSBaseBlock(NormStream, "PERF");
				
				// Write Send, Key, Num, Sum, Header, Size, MaskEnc
				MaskEnc = 0;
					
					// Send
				D_BSwu8(NormStream, 'S');
				MaskEnc ^= 'S';
					// Key
				for (z = 0; z < 4; z++)
				{
					D_BSwu32(NormStream, Hold->Key[z]);
					MaskEnc ^= Hold->Key[z];
				}
					// Num
				PerfPkNumLow = Hold->PacketNum & (uint64_t)((~UINT32_C(0)));
				PerfPkNumHi = Hold->PacketNum >> UINT64_C(32);
				
				D_BSwu32(NormStream, PerfPkNumLow);
				D_BSwu32(NormStream, PerfPkNumHi);
				
				MaskEnc ^= PerfPkNumLow;
				MaskEnc ^= PerfPkNumHi;
					// Sum
				D_BSwu32(NormStream, Hold->CheckSum);
				MaskEnc ^= Hold->CheckSum;
					// Header
				for (z = 0; z < 4; z++)
				{
					D_BSwu8(NormStream, Hold->Header[z]);
					MaskEnc ^= Hold->Header[z];
				}
					// Size
				D_BSwu32(NormStream, Hold->Size);
				MaskEnc ^= Hold->Size;
					// MaskEnc
				D_BSwu32(NormStream, MaskEnc);
				
				// Write actual block data
				D_BSWriteChunk(NormStream, Hold->Data, Hold->Size);
				
				// Record to stream
				D_BSRecordNetBlock(NormStream, &Hold->RemHost);
			}
		}
		
		// Stop sending packets like crazy;
		KeepSending = false;
	} while (KeepSending);

	/* Return value does not matter */
	return 1;
}

/* DS_RBSPerfect_NetPlayF() -- Play block from the perfect stream */
static bool_t DS_RBSPerfect_NetPlayF(struct D_BS_s* const a_Stream, I_HostAddress_t* const a_Host)
{
	char Header[5];
	DS_RBSPerfectData_t* PerfectData;
	bool_t OrigRetVal;
	D_BS_t* NormStream;
	DS_RBSPerfectHold_t* Hold;
	
	size_t i, z, b, BlankSpot;
	
	DS_RBSPerfectKey_t* Key;
	
	uint8_t PerfResp;
	uint32_t PerfKey[4];
	uint64_t PerfPkNum;
	uint32_t PerfPkSum, PerfPkSize, PerfMaskEnc, PerfPkNumLow, PerfPkNumHi;
	uint8_t PerfHeader[5];
	uint32_t ConfirmMask, MaskEnc;
	uint32_t BlockSum;
	
	uint32_t ThisTime;
	
	/* Check */
	if (!a_Stream)
		return false;
	
	/* Get Data */
	PerfectData = a_Stream->Data;
	
	// Nothing there?
	if (!PerfectData)
		return false;
	
	/* Clear perfection mark */
	a_Stream->Marked = false;
	ThisTime = I_GetTimeMS();
	
	/* Go through ordered stream */
	// And determine if there are any blocks that are OK and queued
	for (b = 0; b < PerfectData->SizeReadQ; b++)
	{
		// Get Current Hold
		Hold = PerfectData->ReadQ[b];
	
		// Nothing here?
		if (!Hold)
			continue;
		
		// Get key associated with block
		Key = DS_RBSPerfect_IntFindKey(a_Stream, PerfectData, Hold->Key, &Hold->RemHost);
		
		// No key found? probably revoked, delete block
		if (!Key)
		{
			if (Hold->Data)
				Z_Free(Hold->Data);
			Hold->Data = NULL;
			Z_Free(Hold);
			PerfectData->ReadQ[b] = NULL;
			continue;
		}
		
		// If the block does not match the current read pos, skip it
		// This would mean that we got blocks out of order
		if (Hold->PacketNum != Key->NextReadNum)
			continue;	// Still want to keep it though!
		
		// Build block
		D_BSBaseBlock(a_Stream, Hold->Header);
		D_BSWriteChunk(a_Stream, Hold->Data, Hold->Size);
		
		// Copy IP Host -- This is very important
		if (a_Host)
			memmove(a_Host, &Hold->RemHost, sizeof(*a_Host));
		
		// Was sent away, so delete, increment, return
		if (Hold->Data)
			Z_Free(Hold->Data);
		Hold->Data = NULL;
		Z_Free(Hold);
		PerfectData->ReadQ[b] = NULL;
		Key->NextReadNum++;
		a_Stream->Marked = true;	// and set perfection!
		return true;
	}
	
	/* Constantly read blocks */
	for (;;)
	{
		// Clear
		memset(Header, 0, sizeof(Header));
		OrigRetVal = D_BSPlayNetBlock(PerfectData->WrapStream, Header, a_Host);
	
		// If no block was read, return
		if (!OrigRetVal)
			return false;
	
		// Perfect Block
		if (D_BSCompareHeader("PERF", Header))
		{
			// Read this block and check to see if it is ordered enough and is
			// fully checksummed and correct. If the block is good return it,
			// otherwise add it to a queue to wait on ordered blocks. Also do
			// handling of retransmission and such.
			
			// Read PERFECT Header
			PerfResp = D_BSru8(PerfectData->WrapStream);
			
			for (i = 0; i < 4; i++)
				PerfKey[i] = D_BSru32(PerfectData->WrapStream);
			
			PerfPkNumLow = D_BSru32(PerfectData->WrapStream);
			PerfPkNumHi = D_BSru32(PerfectData->WrapStream);
			
			PerfPkNum = PerfPkNumHi;
			PerfPkNum <<= UINT64_C(32);
			PerfPkNum |= PerfPkNumLow;
			
			PerfPkSum = D_BSru32(PerfectData->WrapStream);
			
			PerfHeader[4] = 0;
			for (i = 0; i < 4; i++)
				PerfHeader[i] = D_BSru8(PerfectData->WrapStream);
			
			PerfPkSize = D_BSru32(PerfectData->WrapStream);
			PerfMaskEnc = D_BSru32(PerfectData->WrapStream);
			
			// Confirm the value
			ConfirmMask = 0;
			ConfirmMask ^= PerfResp;
			ConfirmMask ^= PerfPkNumLow;
			ConfirmMask ^= PerfPkNumHi;
			ConfirmMask ^= PerfPkSum;
			ConfirmMask ^= PerfPkSize;
			ConfirmMask ^= PerfKey[0];
			ConfirmMask ^= PerfKey[1];
			ConfirmMask ^= PerfKey[2];
			ConfirmMask ^= PerfKey[3];
			ConfirmMask ^= PerfHeader[0];
			ConfirmMask ^= PerfHeader[1];
			ConfirmMask ^= PerfHeader[2];
			ConfirmMask ^= PerfHeader[3];
			
			//CONL_PrintF("%08x ?= %08x conf!\n", ConfirmMask, PerfMaskEnc);
			
			// Does not match?
			if (ConfirmMask != PerfMaskEnc)
				// It is lost to the sands of time
				continue;
			
			// Find the key that owns this packet
			Key = DS_RBSPerfect_IntFindKey(a_Stream, PerfectData, PerfKey, a_Host);
			
			// Compare the input key to the found key
			for (i = 0; Key && i < 4; i++)
				if (Key->Key[i] != PerfKey[i])
					Key = NULL;
			
			// No Key?
			if (!Key)
				continue;
			
			// Compare packet size
			if (PerfPkSize != PerfectData->WrapStream->BlkSize - 41)
			{
				//CONL_PrintF("Not 41!\n");
				continue;
			}
			
			// Send Block
			if (PerfResp == 'S')
			{
				//CONL_PrintF("send!\n");
				
				// Get the checksum of the data (to make sure it got through)
				BlockSum = 0;
				for (i = 0; i < PerfPkSize; i++)
					BlockSum ^= ((uint32_t)(PerfectData->WrapStream->BlkData[i + 41])) << (i & 23);
				
				// Bad sum?
				if (BlockSum != PerfPkSum)
					// Ignore this block
					continue;
				
				// Add block to the read queue
					// As long as the following are met:
					// - The block isn't from the past
					// - The block isn't already stored
				if (PerfPkNum >= Key->NextReadNum)
				{
					// Find blocks already in queue that match this key
					// and see if the packet number matches.
					for (BlankSpot = 0, b = 0; b < PerfectData->SizeReadQ; b++)
					{
						// Get Current Hold
						Hold = PerfectData->ReadQ[b];
						
						// Nothing here?
						if (!Hold)
						{
							if (!BlankSpot)
								BlankSpot = b;
							Hold = NULL;
							continue;
						}
						
						// Compare key
						for (z = 0; z < 4; z++)
							if (Hold->Key[z] != PerfKey[z])
								break;
						
						// Compare packet num and check z also
						if (z < 4 || Hold->PacketNum != PerfPkNum)
						{
							Hold = NULL;
							continue;
						}
						
						// Found the block
						break;
					}
					
					// Ran out?
					if (b >= PerfectData->SizeReadQ)
						Hold = NULL;
					
					// Pre-existing block not found
					if (!Hold)
					{
						// Use blank spot?
						if (BlankSpot > 0 && BlankSpot < PerfectData->SizeReadQ)
							Hold = PerfectData->ReadQ[BlankSpot] = Z_Malloc(sizeof(*Hold), PU_BLOCKSTREAM, NULL);
					
						// Otherwise resize
						else
						{
							//Hold
							Z_ResizeArray((void**)&PerfectData->ReadQ, sizeof(*PerfectData->ReadQ),
											PerfectData->SizeReadQ, PerfectData->SizeReadQ + 2);
							Hold = PerfectData->ReadQ[PerfectData->SizeReadQ++] =
									Z_Malloc(sizeof(*Hold), PU_BLOCKSTREAM, NULL);
							PerfectData->SizeReadQ++;
						}
						
						// Fill in information
						memmove(&Hold->RemHost, a_Host, sizeof(Hold->RemHost));
						Hold->PacketNum = PerfPkNum;
						Hold->CheckSum = PerfPkSum;
						
						for (z = 0; z < 4; z++)
						{
							Hold->Header[z] = PerfHeader[z];
							Hold->Key[z] = PerfKey[z];
						}
						
							// Time packet appeared
						Hold->ClockTime = ThisTime;
						Hold->AutoRackTime = Hold->ClockTime + PERFECTRETRANSTIME;
						Hold->RackTimeIsLess = false;
						if (Hold->AutoRackTime < Hold->AutoRackTime)
							Hold->RackTimeIsLess = true;
						
						// Copy Data
						Hold->Size = PerfPkSize;
						Hold->Data = Z_Malloc(Hold->Size, PU_BLOCKSTREAM, NULL);
						memmove(Hold->Data, &PerfectData->WrapStream->BlkData[41], Hold->Size);
					}
				}
				
				// ACK it now
				NormStream = PerfectData->WrapStream;
				
				// Write to remote host
				D_BSBaseBlock(NormStream, "PERF");
				
				// Write Send, Key, Num, Sum, Header, Size, MaskEnc
				MaskEnc = 0;
					
					// Send
				D_BSwu8(NormStream, 'A');
				MaskEnc ^= 'A';
					// Key
				for (z = 0; z < 4; z++)
				{
					D_BSwu32(NormStream, PerfKey[z]);
					MaskEnc ^= PerfKey[z];
				}
					// Num
				PerfPkNumLow = PerfPkNum & (uint64_t)((~UINT32_C(0)));
				PerfPkNumHi = PerfPkNum >> UINT64_C(32);
				
				D_BSwu32(NormStream, PerfPkNumLow);
				D_BSwu32(NormStream, PerfPkNumHi);
				
				MaskEnc ^= PerfPkNumLow;
				MaskEnc ^= PerfPkNumHi;
					// Sum
				D_BSwu32(NormStream, PerfPkSum);
				MaskEnc ^= PerfPkSum;
					// Header
				for (z = 0; z < 4; z++)
				{
					D_BSwu8(NormStream, PerfHeader[z]);
					MaskEnc ^= PerfHeader[z];
				}
					// Size -- This is ignored, due to future payload
				D_BSwu32(NormStream, 0);
				MaskEnc ^= 0;
					// MaskEnc
				D_BSwu32(NormStream, MaskEnc);
				
				// Record it
				D_BSRecordNetBlock(NormStream, a_Host);
				
				// Was added to Q so deal with later
				continue;
			}
			
			// ACK Block
			else if (PerfResp == 'A')
			{
				// Find matching write block and set as acknowledged
				for (b = 0; b < PerfectData->SizeWriteQ; b++)
				{
					//CONL_PrintF("ack!\n");
					
					// Get Current Hold
					Hold = PerfectData->WriteQ[b];
					
					// Nothing here?
					if (!Hold)
					{
						Hold = NULL;
						continue;
					}
					
					// Compare key
					for (z = 0; z < 4; z++)
						if (Hold->Key[z] != PerfKey[z])
							break;
					
					// Compare packet num and check z also
					if (z < 4 || Hold->PacketNum != PerfPkNum)
					{
						Hold = NULL;
						continue;
					}
					
					// Found the block, mark as acknowledged
					Hold->BlockAck = true;
					break;
				}
				
				// Modified with response
				continue;
			}
		}
	
		// Normal Block
		else
		{
			// Duplicate the data as needed
				// Don't worry about host because that was copied already (ptrs)
			D_BSBaseBlock(a_Stream, Header);
			D_BSWriteChunk(a_Stream, PerfectData->WrapStream->BlkData, PerfectData->WrapStream->BlkSize);
		
			// Return the original value returned
			return OrigRetVal;
		}
	}
	
	/* Nothing was returned */
	// If any blocks were streamed, they would be played on the next run.
	return false;
}

/* DS_RBSPerfect_FlushF() -- Flushes perfect data */
static bool_t DS_RBSPerfect_FlushF(struct D_BS_s* const a_Stream)
{
	DS_RBSPerfectData_t* PerfectData;
	I_HostAddress_t FakeHost;
	
	/* Check */
	if (!a_Stream)
		return false;
	
	/* Get Data */
	PerfectData = a_Stream->Data;
	
	// Nothing there?
	if (!PerfectData)
		return false;
	
	/* Sync? */
	PerfectData->InFlush = true;
	DS_RBSPerfect_NetRecordF(a_Stream, &FakeHost);
	PerfectData->InFlush = false;
	
	return true;
}

/* DS_RBSPerfect_IOCtlF() -- IOCtl Controller */
bool_t DS_RBSPerfect_IOCtlF(struct D_BS_s* const a_Stream, const D_BSStreamIOCtl_t a_IOCtl, intptr_t* a_DataP)
{
	/* Which IOCTL now? */
	switch (a_IOCtl)
	{
			// Just read a perfect packet
		case DRBSIOCTL_ISPERFECT:
			*a_DataP = a_Stream->Marked;
			return true;
			
			// Un-Handled
		default:
			return false;
	}
}

/**********************
*** RELIABLE STREAM ***
*********************/

#define RBSMAXRELIABLEKEEP				128		// Max reliable keeps
#define RBSRELIABLERETRAN				500		// Retransmit reliables after this
#define RBSRELDECAYADD					50		// Retransmit decay rate
#define RBSRELMAXDECAY					5000	// Maximum decay retransmission
#define RBSRELIABLETIMEOUT				120000	// Remove connection after 2 min

/* DS_ReliablePk_t -- Reliable packet */
typedef struct DS_ReliablePk_s
{
	uint32_t Key;								// Key for packet
	int32_t Time;								// Time wrote
	int32_t FirstTime;							// First time
	int32_t Decay;								// Decay Time
	bool_t Transmit;							// Transmit data
	char Header[5];								// Data Header
	uintptr_t Size;								// Data Size
	void* Data;									// Actual Data
} DS_ReliablePk_t;

/* DS_ReliableFlat_t -- Reliable storage area */
typedef struct DS_ReliableFlat_s
{
	uint32_t HostHash;							// Hash of host
	I_HostAddress_t Address;					// Address of host
	D_BS_t* outStore;							// Loopback storage (sent data)
	DS_ReliablePk_t OutPks[RBSMAXRELIABLEKEEP];	// Output packets
	//uint32_t KeepKeys[RBSMAXRELIABLEKEEP];	// Maximum Keep keys
	uint64_t GoCount;							// Go count (sent stuff)
	D_BS_t* inStore;							// Input storage
	uint64_t LastTime[2];						// Last IO time (stale connection)
} DS_ReliableFlat_t;

/* DS_RBSReliableData_t -- Reliable transport stream data */
typedef struct DS_RBSReliableData_s
{
	D_BS_t* Self;								// Self
	bool_t Debug;								// Debug Enabled
	D_BS_t* WrapStream;							// Stream being wrapped
	uint32_t TransSize;							// Transport Size
	bool_t IsPerf;								// Is perfect
	
	DS_ReliableFlat_t** Flats;					// Flats (per-host data)
	size_t NumFlats;							// Number of flats
	uint32_t RR;						// Round rover
} DS_RBSReliableData_t;

/* DS_RBSReliable_KillFlat() -- Kills a flat */
static void DS_RBSReliable_KillFlat(DS_RBSReliableData_t* const a_Data, DS_ReliableFlat_t* const a_Flat)
{
	int i, j;
	DS_ReliableFlat_t* CurFlat;
	
	/* Check */
	if (!a_Flat)
		return;
		
	/* Find out spot */
	for (i = 0; i < a_Data->NumFlats; i++)
	{
		// Get current
		CurFlat = a_Data->Flats[i];
		
		// Missing?
		if (!CurFlat)
			continue;
		
		// Not the flat we want to clear
		if (a_Flat != CurFlat)
			continue;
		
		// Remove from list
		a_Data->Flats[i] = NULL;
		
		// Destroy loopback stores
		D_BSCloseStream(CurFlat->inStore);
		D_BSCloseStream(CurFlat->outStore);
		
		// Destroy and active holds
		for (j = 0; j < RBSMAXRELIABLEKEEP; j++)
		{
			// Free data if it existed
			if (CurFlat->OutPks[j].Data)
				Z_Free(CurFlat->OutPks[j].Data);
			CurFlat->OutPks[j].Data = NULL;
		}
		
		// Free self
		Z_Free(CurFlat);
		
		// Done
		break;
	}
}

/* DS_RBSReliable_Reset() -- Resets reliable stream */
static void DS_RBSReliable_Reset(DS_RBSReliableData_t* const a_Data)
{
	int i;
	DS_ReliableFlat_t* CurFlat;
	
	/* Kill off all flats */
	for (i = 0; i < a_Data->NumFlats; i++)
	{
		// Get current
		CurFlat = a_Data->Flats[i];
		
		// Missing?
		if (!CurFlat)
			continue;
		
		// Kill it
		DS_RBSReliable_KillFlat(a_Data, CurFlat);
	}	
}

/* DS_RBSReliable_FlatByHost() -- Get flat by host address */
static DS_ReliableFlat_t* DS_RBSReliable_FlatByHost(DS_RBSReliableData_t* const a_Data, const I_HostAddress_t* const a_Address, const bool_t a_Create)
{
	uint32_t CurHash;
	DS_ReliableFlat_t* CurFlat;
	register int i;
	
	/* Hash current address */
	CurHash = I_NetHashHost(a_Address);
	
	/* Go through data entries */
	for (i = 0; i < a_Data->NumFlats; i++)
	{
		// Get current
		CurFlat = a_Data->Flats[i];
		
		// Check
		if (!CurFlat)
			continue;
		
		// Compare hash
		if (CurHash == CurFlat->HostHash)
			if (I_NetCompareHost(a_Address, &CurFlat->Address))
				return CurFlat;
	}
	
	/* Create it if wanted */
	if (a_Create)
	{
		// Find blank spot
		for (i = 0; i < a_Data->NumFlats; i++)
			if (!a_Data->Flats[i])
				break;
		
		// No room?
		if (i >= a_Data->NumFlats)
		{
			Z_ResizeArray((void**)&a_Data->Flats, sizeof(*a_Data->Flats),
				a_Data->NumFlats, a_Data->NumFlats + 1);
			i = a_Data->NumFlats++;
		}
		
		// Create
		a_Data->Flats[i] = CurFlat = Z_Malloc(sizeof(*CurFlat), PU_STATIC, NULL);
		
		// Setup
		CurFlat->HostHash = CurHash;
		memmove(&CurFlat->Address, a_Address, sizeof(I_HostAddress_t));
		CurFlat->outStore = D_BSCreateLoopBackStream();
		CurFlat->inStore = D_BSCreateLoopBackStream();
		
		// Return it
		return CurFlat;	
	}
	
	/* Do nothing */
	else
		return NULL;
}

/* DS_RBSReliable_FlushF() -- Flushes stream */
bool_t DS_RBSReliable_FlushF(struct D_BS_s* const a_Stream)
{
	DS_RBSReliableData_t* RelData;
	DS_ReliableFlat_t* CurFlat;
	I_HostAddress_t Addr;
	char Header[5], inHeader[5];
	int i, j, nai, pkc, z;
	int NotActive[RBSMAXRELIABLEKEEP];
	uint32_t ThisTime;
	DS_ReliablePk_t* PkP;
	uintptr_t CopyCount;
	bool_t Kill;
	
	uint8_t inCode;
	uint16_t inSlot;
	uint32_t inKey, inTime, inFTime, inSize;
	
	/* Get Data */
	RelData = a_Stream->Data;
	
	// Check
	if (!RelData)
		return false;
	
	/* Init */
	memset(Header, 0, sizeof(Header));
	memset(&Addr, 0, sizeof(Addr));
	memset(NotActive, 0xFF, sizeof(NotActive));
	
	ThisTime = I_GetTimeMS();
	
	/* Read input from the upper stream first */
	// Any reliable data sent to us will have an ACK back.
	// If we recieve an ACK, then clear the out packet specified.
	while (D_BSPlayNetBlock(RelData->WrapStream, Header, &Addr))
	{
		// Clear input header
		memset(inHeader, 0, sizeof(inHeader));
		
		// Get current flat
		CurFlat = DS_RBSReliable_FlatByHost(RelData, &Addr, true);
		
		// Reliable Packet?
		if (D_BSCompareHeader(Header, "RELY"))
		{
			// Read Header
			inCode = D_BSru8(RelData->WrapStream);
			inSlot = D_BSru16(RelData->WrapStream);
			inKey = D_BSru32(RelData->WrapStream);
			inTime = D_BSru32(RelData->WrapStream);
			inFTime = D_BSru32(RelData->WrapStream);
			inSize = D_BSru32(RelData->WrapStream);
			
			// Reserved
			D_BSru32(RelData->WrapStream);
			D_BSru32(RelData->WrapStream);
			D_BSru32(RelData->WrapStream);
			
			// Which Code?
			switch (inCode)
			{
					// Packet
				case 'P':
					// Contained header
					for (z = 0; z < 4; z++)
						inHeader[z] = D_BSru8(RelData->WrapStream);
			
					// Base Block
					D_BSBaseBlock(CurFlat->inStore, inHeader);
			
					// Write direct chunk
					D_BSWriteChunk(CurFlat->inStore, &RelData->WrapStream->BlkData[RelData->WrapStream->ReadOff], inSize);
			
					// Record it
					D_BSRecordBlock(CurFlat->inStore);
		
					// Flush
					D_BSFlushStream(CurFlat->inStore);
					
					// Reply with ACK
					D_BSBaseBlock(RelData->WrapStream, "RELY");
			
					// Write protocol head
					D_BSwu8(RelData->WrapStream, 'A');	// A for ack
					D_BSwu16(RelData->WrapStream, inSlot);	// Current slot
					D_BSwu32(RelData->WrapStream, inKey);
					D_BSwu32(RelData->WrapStream, inTime);
					D_BSwu32(RelData->WrapStream, inFTime);
					D_BSwu32(RelData->WrapStream, inSize);
			
					// Reserved
					D_BSwu32(RelData->WrapStream, 0xDEADBEEFU);
					D_BSwu32(RelData->WrapStream, 0xDEADBEEFU);
					D_BSwu32(RelData->WrapStream, 0xDEADBEEFU);
					
					D_BSRecordNetBlock(RelData->WrapStream, &CurFlat->Address);
					D_BSFlushStream(RelData->WrapStream);
					break;
					
					// Acknowledge
				case 'A':
					// Slot bounds check
					if (inSlot >= 0 && inSlot < RBSMAXRELIABLEKEEP)
					{
						// Get current output
						PkP = &CurFlat->OutPks[inSlot];
						
						// Same key?
						if (inKey == PkP->Key)
						{
							// Clear Data
							PkP->Key = 0;
							PkP->Time = 0;
							PkP->FirstTime = 0;
							PkP->Decay = 0;
							PkP->Transmit = false;
							memset(PkP->Header, 0, sizeof(PkP->Header));
							PkP->Size = 0;
						}
					}
					break;
					
					// Unknown
				default:
					break;
			}
		}
		
		// Un-Reliable Packet
		else
		{
			// Base Block
			D_BSBaseBlock(CurFlat->inStore, RelData->WrapStream->BlkHeader);
			
			// Write Data 1:1
			D_BSWriteChunk(CurFlat->inStore, RelData->WrapStream->BlkData, RelData->WrapStream->BlkSize);
			
			// Record it
			D_BSRecordBlock(CurFlat->inStore);
		
			// Flush
			D_BSFlushStream(CurFlat->inStore);
		}
		
		// Clear address for another cycle
		memset(&Addr, 0, sizeof(Addr));
	}
	
	/* Handle all data that needs outputting */
	for (i = 0; i < RelData->NumFlats; i++)
	{
		// Get current
		CurFlat = RelData->Flats[i];
		
		// Missing?
		if (!CurFlat)
			continue;
		
		// Go through the active chain
		nai = -1;
		for (Kill = false, j = 0; j < RBSMAXRELIABLEKEEP; j++)
		{
			// Not active?
			if (!CurFlat->OutPks[j].Key)
			{
				if (nai < (RBSMAXRELIABLEKEEP >> 1))
					NotActive[++nai] = j;
				continue;
			}
			
			// Retransmit?
			if (ThisTime - CurFlat->OutPks[j].Time > (RBSRELIABLERETRAN + CurFlat->OutPks[j].Decay))
				CurFlat->OutPks[j].Transmit = true;
			
			// Packet timeout? Possibly a now headless connection
			if (ThisTime - CurFlat->OutPks[j].FirstTime > RBSRELIABLETIMEOUT)
			{
				Kill = true;
				DS_RBSReliable_KillFlat(RelData, CurFlat);
				break;
			}
		}
		
		// Connection was killed
		if (Kill)
			continue;
		
		// Something was not active? free spot!
		while (nai >= 0)
		{
			// Read location
			pkc = NotActive[nai--];
			
			// Out of bounds?
			if (pkc < 0 || pkc >= RBSMAXRELIABLEKEEP)
				continue;
			
			// Pointer to output
			PkP = &CurFlat->OutPks[pkc];
			
			// Read input from output stream
			memset(PkP->Header, 0, sizeof(PkP->Header));
			D_BSFlushStream(CurFlat->outStore);
			if (!D_BSPlayBlock(CurFlat->outStore, PkP->Header))
				break;	// Means nothing was in the loopback
			
			// Generate key and time
			for (PkP->Key = 0; PkP->Key == 0;)	// cannot be zero!
				PkP->Key = D_CMakePureRandom();
			PkP->Time = PkP->FirstTime = ThisTime;
			PkP->Transmit = true;
			
			// Set data
			PkP->Size = CurFlat->outStore->BlkSize;
			
			// Allocate Data Buffer?
			if (!PkP->Data)
				PkP->Data = Z_Malloc(RelData->TransSize, PU_STATIC, NULL);
			else
				memset(PkP->Data, 0, RelData->TransSize);
			
			// Determine bytes to copy
			if (PkP->Size < RelData->TransSize)
				CopyCount = PkP->Size;
			else
				CopyCount = RelData->TransSize;
			
			// Read entire chunk
			D_BSReadChunk(CurFlat->outStore, PkP->Data, CopyCount);
		}
		
		// Transmit all packets needing transmission
		for (j = 0; j < RBSMAXRELIABLEKEEP; j++)
		{
			// Not active?
			if (!CurFlat->OutPks[j].Key)
				continue;
			
			// Not being transmitted
			if (!CurFlat->OutPks[j].Transmit)
				continue;
				
			// Pointer to output
			PkP = &CurFlat->OutPks[j];
			
			// Do not transmit again
			PkP->Transmit = false;
			PkP->Time = ThisTime;	// Set current time for better retrans
			PkP->Decay += RBSRELDECAYADD;	// Decay transmission
			
			if (PkP->Decay >= RBSRELMAXDECAY)
				PkP->Decay = RBSRELMAXDECAY;
			
			// Write base block for wrapped stream
			D_BSBaseBlock(RelData->WrapStream, "RELY");
			
			// Write protocol head
			D_BSwu8(RelData->WrapStream, 'P');	// P for packet
			D_BSwu16(RelData->WrapStream, j);	// Current slot
			D_BSwu32(RelData->WrapStream, PkP->Key);
			D_BSwu32(RelData->WrapStream, PkP->Time);
			D_BSwu32(RelData->WrapStream, PkP->FirstTime);
			D_BSwu32(RelData->WrapStream, PkP->Size);
			
			// Reserved
			D_BSwu32(RelData->WrapStream, 0xDEADBEEFU);
			D_BSwu32(RelData->WrapStream, 0xDEADBEEFU);
			D_BSwu32(RelData->WrapStream, 0xDEADBEEFU);
			
			// Block Header
			for (z = 0; z < 4; z++)
				D_BSwu8(RelData->WrapStream, PkP->Header[z]);
			
			// Actual Data
			D_BSWriteChunk(RelData->WrapStream, PkP->Data, PkP->Size);
			
			// Transmit to remote host
			D_BSRecordNetBlock(RelData->WrapStream, &CurFlat->Address);
		}
	}
	
	/* Flush the wrapped stream */
	// This is so any data that was written is sent
	D_BSFlushStream(RelData->WrapStream);
	
	/* Flush always successful */
	return true;
}

/* DS_RBSReliable_NetRecordF() -- Records to stream */
size_t DS_RBSReliable_NetRecordF(struct D_BS_s* const a_Stream, I_HostAddress_t* const a_Host)
{
	DS_RBSReliableData_t* RelData;
	DS_ReliableFlat_t* Flat;
	
	/* Get Data */
	RelData = a_Stream->Data;
	
	// Check
	if (!RelData)
		return 0;
	
	/* Get flat for address */
	Flat = DS_RBSReliable_FlatByHost(RelData, a_Host, true);
	
	// Not found? Whoops!
	if (!Flat)
		return 0;
	
	/* Write to output queue, stuff to be done, eventually... */
	D_BSBaseBlock(Flat->outStore, a_Stream->BlkHeader);
	
	// Write entire data chunk
	D_BSWriteChunk(Flat->outStore, a_Stream->BlkData, a_Stream->BlkSize);
	
	// Record it
	D_BSRecordBlock(Flat->outStore);
	
	// Flush it
	D_BSFlushStream(Flat->outStore);
	
	/* Always successful */
	return 1;
}

/* DS_RBSReliable_NetPlayF() -- Plays from stream */
bool_t DS_RBSReliable_NetPlayF(struct D_BS_s* const a_Stream, I_HostAddress_t* const a_Host)
{
	DS_RBSReliableData_t* RelData;
	uint32_t Stop;
	bool_t Blipped;
	DS_ReliableFlat_t* CurFlat;
	char Header[5];
	
	/* Get Data */
	RelData = a_Stream->Data;
	
	// Check
	if (!RelData)
		return false;
	
	/* No communications performed, ever */
	if (!RelData->NumFlats)
		return false;
	
	/* Setup stop point */
	Stop = RelData->RR % RelData->NumFlats;
	memset(Header, 0, sizeof(Header));
	
	/* Go through all the rounds */
	Blipped = false;
	for (RelData->RR = RelData->RR % RelData->NumFlats;
			!Blipped || (Blipped && Stop != RelData->RR);
			RelData->RR = (RelData->RR + 1) % RelData->NumFlats, Blipped = true)
	{
		// Get current flat
		CurFlat = RelData->Flats[RelData->RR];
		
		// Nothing here?
		if (!CurFlat)
			continue;
		
		// Flush
		D_BSFlushStream(CurFlat->inStore);
		
		// Try playing back something
		if (D_BSPlayBlock(CurFlat->inStore, Header))
		{
			// Current base
			D_BSBaseBlock(a_Stream, Header);
			
			// Write chunk, the entire lo data
			D_BSWriteChunk(a_Stream, CurFlat->inStore->BlkData, CurFlat->inStore->BlkSize);
			
			// Copy Address
			memmove(a_Host, &CurFlat->Address, sizeof(I_HostAddress_t));
			
			// Something was played, so return
			return true;
		}
	}
	
	/* Nothing was played back */
	return false;
}

/* DS_RBSReliable_DeleteF() -- Deletes stream */
void DS_RBSReliable_DeleteF(struct D_BS_s* const a_Stream)
{
	DS_RBSReliableData_t* RelData;
	
	/* Get Data */
	RelData = a_Stream->Data;
	
	// Check
	if (!RelData)
		return;
	
	/* Reset takes care of most of the work */
	DS_RBSReliable_Reset(RelData);
	
	/* Finish it off */
	if (RelData->Flats)
		Z_Free(RelData->Flats);
}

/* DS_RBSReliable_IOCtlF() -- Advanced I/O Control */
bool_t DS_RBSReliable_IOCtlF(struct D_BS_s* const a_Stream, const D_BSStreamIOCtl_t a_IOCtl, intptr_t* a_DataP)
{
	DS_RBSReliableData_t* RelData;
	
	/* Get Data */
	RelData = a_Stream->Data;
	
	// Check
	if (!RelData)
		return false;
	
	/* Which IOCtl? */
	switch (a_IOCtl)
	{
			// Perfect Packet
		case DRBSIOCTL_ISPERFECT:
			*a_DataP = RelData->IsPerf;
			return true;
			
			// Max supported transport
		case DRBSIOCTL_MAXTRANSPORT:
			*a_DataP = RelData->TransSize - 32;
			return true;
			
			// Drop a single host
		case DRBSIOCTL_DROPHOST:
			DS_RBSReliable_KillFlat(RelData, DS_RBSReliable_FlatByHost(RelData, a_DataP, false));
			return true;
			
			// Reset reliable streams
		case DRBSIOCTL_RELRESET:
			DS_RBSReliable_Reset(RelData);
			break;
			
			// Unknown
		default:
			return false;
	}
}

/********************
*** PACKED STREAM ***
********************/

#define RBSPACKEDMAXWAITBLOCK				64	// Max blocks in queue
#define RBSPACKEDMAXWAITBYTES			10240	// Max bytes in queue
#define RBSPACKEDZLIBLEVEL				5		// Zlib compression level
#define RBSPACKEDZLIBCHUNK				5120	// Compress 2K bytes at a time

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
	
	mz_stream ZStream;							// Z Stream
	mz_stream InfZStream;						// Z Stream
	uint8_t ZChunk[RBSPACKEDZLIBCHUNK];			// Z Chunk
	
	D_BS_t* InputBuf;							// Input block buffers
} DS_RBSPackedData_t;

/* DS_RBSPacked_FlushF() -- Flushes stream */
bool_t DS_RBSPacked_FlushF(struct D_BS_s* const a_Stream)
{
	DS_RBSPackedData_t* PackData;
	int Ret;
	
	/* Get Data */
	PackData = a_Stream->Data;
	
	// Check
	if (!PackData)
		return false;
	
	/* Finish output compressed data */
	if (PackData->InPack)
	{
		// Init
		PackData->ZStream.avail_in = 0;
		PackData->ZStream.next_in = NULL;
		
		// Deflate some data
		do
		{
			// Finish Compression
			memset(PackData->ZChunk, 0, sizeof(PackData->ZChunk));
			PackData->ZStream.avail_out = RBSPACKEDZLIBCHUNK;
			PackData->ZStream.next_out = PackData->ZChunk;
			
			if ((Ret = mz_deflate(&PackData->ZStream, MZ_FINISH)) != MZ_STREAM_ERROR)
				D_BSWriteChunk(PackData->WrapStream, PackData->ZChunk, RBSPACKEDZLIBCHUNK - PackData->ZStream.avail_out);
		}
		while (Ret != MZ_STREAM_END);
		
		// Send to remote host
		D_BSRecordNetBlock(PackData->WrapStream, (PackData->HasAddr ? &PackData->CurAddr : NULL));
		
		// End Compression
		mz_deflateEnd(&PackData->ZStream);
		memset(&PackData->ZStream, 0, sizeof(PackData->ZStream));
	}
	
	/* Reset info */
	PackData->InPack = false;
	PackData->HasAddr = false;
	PackData->WaitingBlocks = 0;
	PackData->WaitingBytes = 0;
	
	/* Success! */
	return true;
}

/* DS_RBSPacked_NetRecordF() -- Records to stream */
size_t DS_RBSPacked_NetRecordF(struct D_BS_s* const a_Stream, I_HostAddress_t* const a_Host)
{
	DS_RBSPackedData_t* PackData;
	int i, Ret;
	uint32_t u32;
	
	/* Get Data */
	PackData = a_Stream->Data;
	
	// Check
	if (!PackData)
		return false;
	
	/* Different target address? */
	// Check if packing anything
	if (PackData->InPack)
		if ((a_Host && !PackData->HasAddr) || (!a_Host && PackData->HasAddr) ||
			(a_Host && PackData->HasAddr &&
				!I_NetCompareHost(a_Host, &PackData->CurAddr)) ||
			PackData->WaitingBlocks >= RBSPACKEDMAXWAITBLOCK ||
			PackData->WaitingBytes >= RBSPACKEDMAXWAITBYTES)
			DS_RBSPacked_FlushF(a_Stream);
	
	/* Not packing? */
	if (!PackData->InPack)
	{
		// Setup Compression
		memset(&PackData->ZStream, 0, sizeof(PackData->ZStream));
		if (mz_deflateInit(&PackData->ZStream, RBSPACKEDZLIBLEVEL) != MZ_OK)
		{
			// Initialization failed, so write block 1:1 to host stream
			D_BSBaseBlock(PackData->WrapStream, a_Stream->BlkHeader);
			D_BSWriteChunk(PackData->WrapStream, a_Stream->BlkData, a_Stream->BlkSize);
			D_BSRecordNetBlock(PackData->WrapStream, a_Host);
			return 1;
		}
		
		// Now in pack
		PackData->InPack = true;
		
		// Create Block for contained compressed data
		D_BSBaseBlock(PackData->WrapStream, "ZLIB");
	}
	
	/* Create Block Header */
	// Increase wait queue
	PackData->WaitingBlocks += 1;
	PackData->WaitingBytes += a_Stream->BlkSize;
	
	/* Compress Data */
	// Clear stream state
	//memset(&PackData->ZStream, 0, sizeof(PackData->ZStream));
	
	// Write all needed data
	for (i = 0; i < 3; i++)
	{
		// Header
		if (i == 0)
		{
			PackData->ZStream.avail_in = 4;
			PackData->ZStream.next_in = a_Stream->BlkHeader;
		}
		
		// Size
		else if (i == 1)
		{
			u32 = a_Stream->BlkSize;
			u32 = LittleSwapUInt32(u32);
			PackData->ZStream.avail_in = 4;
			PackData->ZStream.next_in = &u32;
		}
		
		// Actual Data
		else
		{
			PackData->ZStream.avail_in = a_Stream->BlkSize;
			PackData->ZStream.next_in = a_Stream->BlkData;
		}

		do
		{
			// Clear
			memset(PackData->ZChunk, 0, sizeof(PackData->ZChunk));
			PackData->ZStream.avail_out = RBSPACKEDZLIBCHUNK;
			PackData->ZStream.next_out = PackData->ZChunk;
			
			// Deflate some data
			if ((Ret = mz_deflate(&PackData->ZStream, MZ_NO_FLUSH)) != MZ_STREAM_ERROR)
				D_BSWriteChunk(PackData->WrapStream, PackData->ZChunk, RBSPACKEDZLIBCHUNK - PackData->ZStream.avail_out);
		}
		while (PackData->ZStream.avail_in > 0);
	}
	
	/* Change Host */
	if (!a_Host)
		PackData->HasAddr = false;
	else
	{
		PackData->HasAddr = true;
		memmove(&PackData->CurAddr, a_Host, sizeof(*a_Host));
	}
	
	/* Success! */
	return true;
}

/* DS_RBSPacked_NetPlayF() -- Plays from stream */
bool_t DS_RBSPacked_NetPlayF(struct D_BS_s* const a_Stream, I_HostAddress_t* const a_Host)
{
	DS_RBSPackedData_t* PackData;
	int i, ReadCount, ReadMode, j, Ret;
	I_HostAddress_t Addr;
	char Header[5];
	uint8_t* p, *e, u8;
	int32_t BkSize, ReadOK;
	bool_t BasedBlock;
	
	/* Get Data */
	PackData = a_Stream->Data;
	
	// Check
	if (!PackData)
		return false;
	
	/* Try again and again */
	for (ReadCount = 0;;)
	{
		// Clear
		memset(Header, 0, sizeof(Header));
		memset(&Addr, 0, sizeof(Addr));
		
		// See if there is a block in the input buffer
		D_BSFlushStream(PackData->InputBuf);
		if (D_BSPlayNetBlock(PackData->InputBuf, Header, a_Host))
		{
			// Clone Block
			D_BSBaseBlock(a_Stream, Header);
			D_BSWriteChunk(a_Stream, PackData->InputBuf->BlkData, PackData->InputBuf->BlkSize);
			
			// A block was read
			return true;
		}
		
		// Load input blocks to be processed
		if (D_BSPlayNetBlock(PackData->WrapStream, Header, &Addr))
		{
			// Increase read count
			ReadMode = 0;
			BasedBlock = false;
			BkSize = 0;
			
			// Compressed?
			if (D_BSCompareHeader(Header, "ZLIB"))
			{
				// Constantly read inflated data
				memset(&PackData->InfZStream, 0, sizeof(PackData->InfZStream));
				
				// Initialize
				if (mz_inflateInit(&PackData->InfZStream) != MZ_OK)
					continue;
					
				// Setup input
				PackData->InfZStream.avail_in = PackData->WrapStream->BlkSize;
				PackData->InfZStream.next_in = PackData->WrapStream->BlkData;
				
				// Read data
				do
				{
					// Clear
					memset(PackData->ZChunk, 0, sizeof(PackData->ZChunk));
					PackData->InfZStream.avail_out = RBSPACKEDZLIBCHUNK;
					PackData->InfZStream.next_out = PackData->ZChunk;
						
					// Deflate some data
					Ret = mz_inflate(&PackData->InfZStream, MZ_FINISH);
					
					if (Ret == MZ_OK || Ret == MZ_BUF_ERROR || Ret == MZ_STREAM_END)
					{
						// Get p base
						p = &PackData->ZChunk;
						e = p + (RBSPACKEDZLIBCHUNK - PackData->InfZStream.avail_out);
						
						// Loop constantly
						while (p < e)
						{
							// Reading Header
							if (ReadMode >= 0 && ReadMode < 4)
							{
								BkSize = 0;
								Header[ReadMode++] = ReadUInt8((uint8_t**)&p);
							}
							
							// Size
							else if (ReadMode >= 4 && ReadMode < 8)
							{
								BkSize <<= 8;
								BkSize |= ReadUInt8((uint8_t**)&p);
								ReadMode++;
							}
							
							// Start block
							else if (ReadMode == 8)
							{
								BkSize = BigSwapUInt32(BkSize);
								
								// Base block
								BasedBlock = true;
								D_BSBaseBlock(PackData->InputBuf, Header);
								ReadMode++;
								ReadCount++;
								
								//if (devparm)
								//	CONL_PrintF("ENQ << %s (%u)\n", Header, BkSize);
							}
						
							// Reading Packet Data
							else if (ReadMode >= 9)
							{
								// Determine amount to read
								ReadOK = (e - p);
								if (ReadOK > BkSize)
									ReadOK = BkSize;
								D_BSWriteChunk(PackData->InputBuf, p, ReadOK);
								p += ReadOK;
								BkSize -= ReadOK;
								
								//if (ReadOK >= 4)
								//	CONL_PrintF("## %llu\n", ((uint64_t*)(PackData->InputBuf->BlkData))[0]);
								
								// Block end reached
								if (BkSize <= 0)
								{
									ReadMode = 0;
									
									// Record it
									D_BSRecordNetBlock(PackData->InputBuf, &Addr);
									BasedBlock = false;
								}
							}
						}
					}
					
					// Force end of stream
					else
					{
						if (devparm)
							CONL_PrintF("Inflation error %i\n", Ret);
						break;
					}
				}
				while (PackData->InfZStream.avail_in > 0 || Ret != MZ_STREAM_END);
				
				// End inflation
				mz_inflateEnd(&PackData->ZStream);
				
				// Record it
				if (BasedBlock)
				{
					D_BSRecordNetBlock(PackData->InputBuf, &Addr);
					D_BSFlushStream(PackData->InputBuf);
					D_BSFlushStream(PackData->InputBuf);
				}
			}
			
			// Uncompressed, pass as is
			else
			{
				// Clone data into input
				D_BSBaseBlock(PackData->InputBuf, Header);
				D_BSWriteChunk(PackData->InputBuf, PackData->WrapStream->BlkData, PackData->WrapStream->BlkSize);
				D_BSRecordNetBlock(PackData->InputBuf, &Addr);
				ReadCount++;
				
				// Flush
				D_BSFlushStream(PackData->InputBuf);
				D_BSFlushStream(PackData->InputBuf);
			}
			
			// Clear for next round
			memset(Header, 0, sizeof(Header));
			memset(&Addr, 0, sizeof(Addr));
			
			if (devparm)
				CONL_PrintF("Read %i compressed blocks.\n", ReadCount);
			
			// Nothing read?
			if (!ReadCount)
				return false;
			
			continue;
		}
		
		// Nothing read?
		if (!ReadCount)
			return false;
	}
	
	/* Failed */
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
		
	/* Destroy input */
	D_BSCloseStream(PackData->InputBuf);
	
	/* Destroy Data */
	Z_Free(PackData);
}

/* DS_RBSPacked_IOCtlF() -- Advanced I/O Control */
bool_t DS_RBSPacked_IOCtlF(struct D_BS_s* const a_Stream, const D_BSStreamIOCtl_t a_IOCtl, intptr_t* a_DataP)
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
	New->RecordF = DS_RBSLoopBack_RecordF;
	New->PlayF = DS_RBSLoopBack_PlayF;
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
	FILE* File;
	D_BS_t* New;
	
	/* Check */
	if (!a_PathName)
		return NULL;
	
	/* Open r or r/w file */
	File = fopen(a_PathName, (((a_Flags & DRBSSF_READONLY) ? "r" : ((a_Flags & DRBSSF_OVERWRITE) ? "w+b" : "a+b"))));
	
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

/* D_BSCreatePerfectStream() -- Creates a wrapped "Perfect" Stream */
D_BS_t* D_BSCreatePerfectStream(D_BS_t* const a_Wrapped)
{
	D_BS_t* New;
	DS_RBSPerfectData_t* Data;
	
	/* Check */
	if (!a_Wrapped)
		return NULL;
	
	/* Create block stream */
	New = Z_Malloc(sizeof(*New), PU_BLOCKSTREAM, NULL);
	
	/* Setup Data */
	New->Data = Data = Z_Malloc(sizeof(*Data), PU_BLOCKSTREAM, NULL);
	New->NetRecordF = DS_RBSPerfect_NetRecordF;
	New->NetPlayF = DS_RBSPerfect_NetPlayF;
	New->FlushF = DS_RBSPerfect_FlushF;
	New->DeleteF = DS_RBSPerfect_DeleteF;
	New->IOCtlF = DS_RBSPerfect_IOCtlF;
	
	// Load stuff into data
	Data->WrapStream = a_Wrapped;
	
	// Debug?
	if (M_CheckParm("-perfdev"))
		Data->Debug = true;
	
	/* Return Stream */
	return New;
}

/* D_BSCreateReliableStream() -- Creates a reliable stream */
D_BS_t* D_BSCreateReliableStream(D_BS_t* const a_Wrapped)
{
	D_BS_t* New;
	DS_RBSReliableData_t* Data;
	intptr_t Trans;
	
	/* Check */
	if (!a_Wrapped)
		return NULL;
	
	/* Create block stream */
	New = Z_Malloc(sizeof(*New), PU_BLOCKSTREAM, NULL);
	
	/* Setup Data */
	New->Data = Data = Z_Malloc(sizeof(*Data), PU_BLOCKSTREAM, NULL);
	New->NetRecordF = DS_RBSReliable_NetRecordF;
	New->NetPlayF = DS_RBSReliable_NetPlayF;
	New->FlushF = DS_RBSReliable_FlushF;
	New->DeleteF = DS_RBSReliable_DeleteF;
	New->IOCtlF = DS_RBSReliable_IOCtlF;
	
	// Load stuff into data
	Data->Self = New;
	Data->WrapStream = a_Wrapped;
	if (D_BSStreamIOCtl(a_Wrapped, DRBSIOCTL_MAXTRANSPORT, &Trans))
		Data->TransSize = Trans;
	else
		Data->TransSize = 2048;
	
	// Debug?
	if (M_CheckParm("-reliabledev"))
		Data->Debug = true;
	
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
	Data->InputBuf = D_BSCreateLoopBackStream();
	
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
	Z_Free(a_Stream);
}

/* D_BSStreamIOCtl() -- Call special IOCtl Handler on stream */
bool_t D_BSStreamIOCtl(D_BS_t* const a_Stream, const D_BSStreamIOCtl_t a_IOCtl, intptr_t* a_DataP)
{
	/* Check */
	if (!a_Stream || !a_DataP || a_IOCtl < 0 || a_IOCtl >= NUMDRBSSTREAMIOCTL)
		return false;
	
	/* Call IOCtl */
	if (a_Stream->IOCtlF)
		return a_Stream->IOCtlF(a_Stream, a_IOCtl, a_DataP);
	
	/* There is no IOCtl */
	return false;
}

/* D_BSStatStream() -- Obtain stream stats */
void D_BSStatStream(D_BS_t* const a_Stream, uint32_t* const a_ReadBk, uint32_t* const a_WriteBk, uint32_t* const a_ReadBy, uint32_t* const a_WriteBy)
{
	/* Check */
	if (!a_Stream)
		return;
	
	/* Return Stats */
	if (a_ReadBk)
		*a_ReadBk = a_Stream->StatBlock[0];
	if (a_WriteBk)
		*a_WriteBk = a_Stream->StatBlock[1];
		
	if (a_ReadBy)
		*a_ReadBy = a_Stream->StatBytes[0];
	if (a_WriteBy)
		*a_WriteBy = a_Stream->StatBytes[1];
}

/* D_BSUnStatStream() -- Clear stream stats */
void D_BSUnStatStream(D_BS_t* const a_Stream)
{
	size_t i;
	
	/* Check */
	if (!a_Stream)
		return;
	
	/* Reset */
	for (i = 0; i < 2; i++)
		a_Stream->StatBlock[i] = a_Stream->StatBytes[i] = 0;
}

/* D_BSMarkedStream() -- Returns true if stream is marked */
bool_t D_BSMarkedStream(D_BS_t* const a_Stream)
{
	/* Check */
	if (!a_Stream)
		return false;
	
	return a_Stream->Marked;
}

/* D_BSCompareHeader() -- Compares headers */
bool_t D_BSCompareHeader(const char* const a_A, const char* const a_B)
{
	/* Check */
	if (!a_A || !a_B)
		return false;
	
	/* Compare */
	if (strcasecmp(a_A, a_B) == 0)
		return true;
	
	/* No match */
	return false;
}

/* D_BSBaseBlock() -- Base block */
// This used to return void, but I need it to return true for the SaveGame code
bool_t D_BSBaseBlock(D_BS_t* const a_Stream, const char* const a_Header)
{
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
	memmove(a_Stream->BlkHeader, a_Header, (strlen(a_Header) >= 4 ? 4 : strlen(a_Header)));
	
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
	ssize_t Count;
	size_t Read;
	uint8_t Bit;
	
	/* Check */
	if (!a_Stream || !a_Data || !a_Size)
		return 0;
	
	/* Counting Read */
	Read = 0;
	for (Count = a_Size; Count > 0; Count--)
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
	if (!a_Stream || !a_Out || !a_OutSize)
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
			if (i < a_OutSize)
				a_Out[i] = 0;
			break;
		}
		
		// Otherwise add to the output
		if (i < a_OutSize)
			a_Out[i] = Char;
	}
	
	/* Always put NUL at very end */
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

