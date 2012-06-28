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
#include "c_lib.h"

/******************
*** FILE STREAM ***
******************/

/* DS_RBSFile_DeleteF() -- Delete file stream */
static void DS_RBSFile_DeleteF(struct D_RBlockStream_s* const a_Stream)
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
static size_t DS_RBSFile_RecordF(struct D_RBlockStream_s* const a_Stream)
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
	BlockLen = LittleSwapUInt32(a_Stream->BlkSize);
	fwrite(&BlockLen, sizeof(BlockLen), 1, File);
	
	/* Determine Quicksum */
	for (i = 0; i < BlockLen; i++)
		if (i & 1)
			QuickSum ^= (((uint8_t*)a_Stream->BlkData)[i]) << ((i & 2) ? 4 : 0);
		else
			QuickSum ^= (~(((uint8_t*)a_Stream->BlkData)[i])) << ((i & 2) ? 6 : 2);
	QuickSum = LittleSwapUInt32(QuickSum);
	fwrite(&QuickSum, sizeof(QuickSum), 1, File);
	
	/* Write Data */
	RetVal = fwrite(a_Stream->BlkData, BlockLen, 1, File);
	
	/* Flush for write */
	fflush(File);
	
	return RetVal;
}

/* DS_RBSFile_PlayF() -- Play from file */
bool DS_RBSFile_PlayF(struct D_RBlockStream_s* const a_Stream)
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
	D_RBSBaseBlock(a_Stream, Header);
	
	/* Write Data to Block */
	D_RBSWriteChunk(a_Stream, Data, Len);
	if (Data)
		Z_Free(Data);
	
	/* Success! */
	return true;
}

/******************
*** FILE STREAM ***
******************/

/* DS_RBSWL_DeleteF() -- Delete file stream */
static void DS_RBSWL_DeleteF(struct D_RBlockStream_s* const a_Stream)
{
	/* Check */
	if (!a_Stream)
		return;
}

/* DS_RBSWL_RecordF() -- Records the current block */
static size_t DS_RBSWL_RecordF(struct D_RBlockStream_s* const a_Stream)
{
	// Recording blocks is not supported for WL Streams (read-only!)
	return 0;
}

/* DS_RBSWL_PlayF() -- Play from file */
bool DS_RBSWL_PlayF(struct D_RBlockStream_s* const a_Stream)
{
	WLEntryStream_c* Stream;
	char Header[5];
	uint32_t Len, Sum, i;
	void* Data;
	
	/* Check */
	if (!a_Stream)
		return false;
		
	/* Get Data */
	Stream = (WLEntryStream_c*)a_Stream->Data;
	
	/* Read Header */
	// Clear
	memset(Header, 0, sizeof(Header));
	Len = Sum = 0;
	Data = NULL;
	
	// Read Header
	for (i = 0; i < 4; i++)
		Header[i] = Stream->ReadChar();
	
	// Read Length and Sum
	Len = Stream->ReadLittleUInt32();
	Sum = Stream->ReadLittleUInt32();
	
	// Read data, if possible (Len could be zero (empty block?))
	if (Len > 0)
	{
		Data = Z_Malloc(Len, PU_STATIC, NULL);
		Stream->ReadChunk(Data, Len);
	}
	
	/* Initialize Block */
	D_RBSBaseBlock(a_Stream, Header);
	
	/* Write Data to Block */
	D_RBSWriteChunk(a_Stream, Data, Len);
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
	uint32_t FlushID;							// Current flush ID
	char Header[5];								// Header
	uint8_t* Data;								// Data
	size_t Size;								// Size
} DS_RBSLoopBackHold_t;

/* DS_RBSLoopBackData_t -- Loop back device */
typedef struct DS_RBSLoopBackData_s
{
	uint32_t FlushID;							// FlushID
	DS_RBSLoopBackHold_t** Q;					// Blocks in Q
	size_t SizeQ;								// Size of Q
} DS_RBSLoopBackData_t;

/* DS_RBSLoopBack_DeleteF() -- Delete loopback stream */
static void DS_RBSLoopBack_DeleteF(struct D_RBlockStream_s* const a_Stream)
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
			if (!LoopData->Q[i])
				Z_Free(LoopData->Q[i]);
		Z_Free(LoopData->Q);
	}
	
	/* Free Data */
	Z_Free(LoopData);
}

/* DS_RBSLoopBack_RecordF() -- Records a block */
size_t DS_RBSLoopBack_RecordF(struct D_RBlockStream_s* const a_Stream)
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
			Hold = LoopData->Q[i] = (DS_RBSLoopBackHold_t*)Z_Malloc(sizeof(DS_RBSLoopBackHold_t), PU_BLOCKSTREAM, NULL);
			break;
		}
	
	// No blank spots?
	if (!Hold)
	{
		Z_ResizeArray((void**)&LoopData->Q, sizeof(*LoopData->Q),
						LoopData->SizeQ, LoopData->SizeQ + 2);
		Hold = LoopData->Q[LoopData->SizeQ++] =
				(DS_RBSLoopBackHold_t*)Z_Malloc(sizeof(DS_RBSLoopBackHold_t), PU_BLOCKSTREAM, NULL);
		LoopData->SizeQ++;
	}
	
	/* Store info in hold */
	// Copy header
	memmove(Hold->Header, a_Stream->BlkHeader, 4);
	
	// Clone Data
	Hold->Size = a_Stream->BlkSize;
	Hold->Data = (uint8_t*)Z_Malloc(Hold->Size, PU_BLOCKSTREAM, NULL);
	memmove(Hold->Data, a_Stream->BlkData, Hold->Size);
	Hold->FlushID = LoopData->FlushID + 1;
	
	/* Return value does not matter */
	return 1;
}

/* DS_RBSLoopBack_PlayF() -- Backs a block back */
bool DS_RBSLoopBack_PlayF(struct D_RBlockStream_s* const a_Stream)
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
	D_RBSBaseBlock(a_Stream, Hold->Header);
	
	// Write all our data in it
	D_RBSWriteChunk(a_Stream, Hold->Data, Hold->Size);
	
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
bool DS_RBSLoopBack_FlushF(struct D_RBlockStream_s* const a_Stream)
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
} I_RBSNetSockData_t;

/* DS_RBSNet_DeleteF() -- Delete network stream */
static void DS_RBSNet_DeleteF(struct D_RBlockStream_s* const a_Stream)
{
	I_RBSNetSockData_t* NetData;
	
	/* Check */
	if (!a_Stream)
		return;
	
	/* Get Data */
	NetData = (I_RBSNetSockData_t*)a_Stream->Data;
	
	if (!NetData)
		return;
	
	/* Free Data */
	Z_Free(NetData);
}

/* DS_RBSNet_NetRecordF() -- Write block to network */
size_t DS_RBSNet_NetRecordF(struct D_RBlockStream_s* const a_Stream, I_HostAddress_t* const a_Host)
{
	I_RBSNetSockData_t* NetData;
	I_NetSocket_t* Socket;
	size_t RetVal, sz, i;
	uint8_t* TBuf = NULL;
	
	/* Check */
	if (!a_Stream)
		return 0;
	
	/* Get Data */
	NetData = (I_RBSNetSockData_t*)a_Stream->Data;
	
	if (!NetData)
		return 0;
	
	/* Get socket */
	Socket = NetData->Socket;
	
	/* Create temporary buffer for sending */
	sz = 4 + 4 + a_Stream->BlkSize;
	TBuf = (uint8_t*)Z_Malloc(sz, PU_BLOCKSTREAM, NULL);//alloca(sz);
	
	// Write to it
		// Header
	for (i = 0; i < 4; i++)
		TBuf[i] = a_Stream->BlkHeader[i];
	((uint32_t*)TBuf)[1] = LittleSwapUInt32((uint32_t)a_Stream->BlkSize);
	memmove(&(((uint32_t*)TBuf)[2]), a_Stream->BlkData, a_Stream->BlkSize);
	
	/* Send the entire buffer */
	// Sending 4 different packets alone could cause fragmentation and out of
	// order problems (which is REALLY not wanted). So this way, at least this
	// is not much of a problem.
	RetVal = I_NetSend(Socket, a_Host, TBuf, sz);
	
	/* Clean up and return */
	Z_Free(TBuf);
	return RetVal;
}

/* DS_RBSNet_NetPlayF() -- Play block from the network */
bool DS_RBSNet_NetPlayF(struct D_RBlockStream_s* const a_Stream, I_HostAddress_t* const a_Host)
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
	NetData = (I_RBSNetSockData_t*)a_Stream->Data;
	
	if (!NetData)
		return 0;
	
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
	D_RBSBaseBlock(a_Stream, Header);
	D_RBSWriteChunk(a_Stream, NetData->ReadBuf + 8, Len);
	
	/* Was played back */
	return true;
}

/*********************
*** PERFECT STREAM ***
*********************/

#define PERFECTKEYEXPIRETIME			300000	// Time until key expires (in MS, 5min)
#define PERFECTRETRANSTIME				1000	// If no ack recieved, retransmit (1s)
#define PERFECTMAXRETRIES				150		// Max before key revocation (1s * 150 = 150s/2.5m)

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
	bool ExpireLessThan;						// Expire time is in the past
	bool Revoke;								// Revoke key
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
	bool RackTimeIsLess;						// Less than
	bool ReTransmit;							// Retransmit block
	bool BlockAck;							// Block acknowledged
	I_HostAddress_t RemHost;					// Remote Host
	
	// Packet Key
	uint32_t Key[4];							// Key Holder
} DS_RBSPerfectHold_t;

/* DS_RBSPerfectData_t -- Perfect Stream */
typedef struct DS_RBSPerfectData_s
{
	D_RBlockStream_t* WrapStream;				// Stream to wrap
	bool InFlush;								// In the middle of a flush
	
	DS_RBSPerfectHold_t** ReadQ;				// Blocks to read Q
	size_t SizeReadQ;							// Size of read Q
	
	DS_RBSPerfectHold_t** WriteQ;				// Blocks to read Q
	size_t SizeWriteQ;							// Size of read Q
	
	DS_RBSPerfectKey_t** Keys;					// Packet keys
	size_t NumKeys;								// Number of keys
	
	bool Debug;								// Debug it
} DS_RBSPerfectData_t;

/* DS_RBSPerfect_DeleteF() -- Delete perfect stream */
static void DS_RBSPerfect_DeleteF(struct D_RBlockStream_s* const a_Stream)
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
static DS_RBSPerfectKey_t* DS_RBSPerfect_IntFindKey(struct D_RBlockStream_s* const a_Stream, DS_RBSPerfectData_t* const a_PerfectData, const uint32_t* const a_InKey, I_HostAddress_t* const a_Host)
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
		Key = a_PerfectData->Keys[FbK] = (DS_RBSPerfectKey_t*)Z_Malloc(sizeof(*Key), PU_STATIC, NULL);
	
	// Resize the key array
	else
	{
		Z_ResizeArray((void**)&a_PerfectData->Keys, sizeof(*a_PerfectData->Keys),
			a_PerfectData->NumKeys, a_PerfectData->NumKeys + 1);
		Key = a_PerfectData->Keys[a_PerfectData->NumKeys++] = (DS_RBSPerfectKey_t*)Z_Malloc(sizeof(*Key), PU_STATIC, NULL);
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
static size_t DS_RBSPerfect_NetRecordF(struct D_RBlockStream_s* const a_Stream, I_HostAddress_t* const a_Host)
{
	size_t i, b, z;
	DS_RBSPerfectData_t* PerfectData;
	DS_RBSPerfectHold_t* Hold;
	DS_RBSPerfectKey_t* Key;
	bool KeepSending;
	uint32_t ThisTime, MaskEnc;
	D_RBlockStream_t* NormStream;
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
					Hold = PerfectData->WriteQ[i] = (DS_RBSPerfectHold_t*)Z_Malloc(sizeof(*Hold), PU_BLOCKSTREAM, NULL);
					break;
				}
	
			// No blank spots?
			if (!Hold)
			{
				Z_ResizeArray((void**)&PerfectData->WriteQ, sizeof(*PerfectData->WriteQ),
								PerfectData->SizeWriteQ, PerfectData->SizeWriteQ + 2);
				Hold = PerfectData->WriteQ[PerfectData->SizeWriteQ++] =
						(DS_RBSPerfectHold_t*)Z_Malloc(sizeof(*Hold), PU_BLOCKSTREAM, NULL);
				PerfectData->SizeWriteQ++;
			}
	
			// Store block in the hold
				// Header
			memmove(Hold->Header, a_Stream->BlkHeader, 4);
	
				// Data
			Hold->Size = a_Stream->BlkSize;
			Hold->Data = (uint8_t*)Z_Malloc(Hold->Size, PU_BLOCKSTREAM, NULL);
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
				D_RBSBaseBlock(NormStream, "PERF");
				
				// Write Send, Key, Num, Sum, Header, Size, MaskEnc
				MaskEnc = 0;
					
					// Send
				D_RBSWriteUInt8(NormStream, 'S');
				MaskEnc ^= 'S';
					// Key
				for (z = 0; z < 4; z++)
				{
					D_RBSWriteUInt32(NormStream, Hold->Key[z]);
					MaskEnc ^= Hold->Key[z];
				}
					// Num
				PerfPkNumLow = Hold->PacketNum & (uint64_t)((~UINT32_C(0)));
				PerfPkNumHi = Hold->PacketNum >> UINT64_C(32);
				
				D_RBSWriteUInt32(NormStream, PerfPkNumLow);
				D_RBSWriteUInt32(NormStream, PerfPkNumHi);
				
				MaskEnc ^= PerfPkNumLow;
				MaskEnc ^= PerfPkNumHi;
					// Sum
				D_RBSWriteUInt32(NormStream, Hold->CheckSum);
				MaskEnc ^= Hold->CheckSum;
					// Header
				for (z = 0; z < 4; z++)
				{
					D_RBSWriteUInt8(NormStream, Hold->Header[z]);
					MaskEnc ^= Hold->Header[z];
				}
					// Size
				D_RBSWriteUInt32(NormStream, Hold->Size);
				MaskEnc ^= Hold->Size;
					// MaskEnc
				D_RBSWriteUInt32(NormStream, MaskEnc);
				
				// Write actual block data
				D_RBSWriteChunk(NormStream, Hold->Data, Hold->Size);
				
				// Record to stream
				D_RBSRecordNetBlock(NormStream, &Hold->RemHost);
			}
		}
		
		// Stop sending packets like crazy;
		KeepSending = false;
	} while (KeepSending);

	/* Return value does not matter */
	return 1;
}

/* DS_RBSPerfect_NetPlayF() -- Play block from the perfect stream */
static bool DS_RBSPerfect_NetPlayF(struct D_RBlockStream_s* const a_Stream, I_HostAddress_t* const a_Host)
{
	char Header[5];
	DS_RBSPerfectData_t* PerfectData;
	bool OrigRetVal;
	D_RBlockStream_t* NormStream;
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
	PerfectData = (DS_RBSPerfectData_t*)a_Stream->Data;
	
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
		D_RBSBaseBlock(a_Stream, Hold->Header);
		D_RBSWriteChunk(a_Stream, Hold->Data, Hold->Size);
		
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
		OrigRetVal = D_RBSPlayNetBlock(PerfectData->WrapStream, Header, a_Host);
	
		// If no block was read, return
		if (!OrigRetVal)
			return false;
	
		// Perfect Block
		if (D_RBSCompareHeader("PERF", Header))
		{
			// Read this block and check to see if it is ordered enough and is
			// fully checksummed and correct. If the block is good return it,
			// otherwise add it to a queue to wait on ordered blocks. Also do
			// handling of retransmission and such.
			
			// Read PERFECT Header
			PerfResp = D_RBSReadUInt8(PerfectData->WrapStream);
			
			for (i = 0; i < 4; i++)
				PerfKey[i] = D_RBSReadUInt32(PerfectData->WrapStream);
			
			PerfPkNumLow = D_RBSReadUInt32(PerfectData->WrapStream);
			PerfPkNumHi = D_RBSReadUInt32(PerfectData->WrapStream);
			
			PerfPkNum = PerfPkNumHi;
			PerfPkNum <<= UINT64_C(32);
			PerfPkNum |= PerfPkNumLow;
			
			PerfPkSum = D_RBSReadUInt32(PerfectData->WrapStream);
			
			PerfHeader[4] = 0;
			for (i = 0; i < 4; i++)
				PerfHeader[i] = D_RBSReadUInt8(PerfectData->WrapStream);
			
			PerfPkSize = D_RBSReadUInt32(PerfectData->WrapStream);
			PerfMaskEnc = D_RBSReadUInt32(PerfectData->WrapStream);
			
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
							Hold = PerfectData->ReadQ[BlankSpot] = (DS_RBSPerfectHold_t*)Z_Malloc(sizeof(*Hold), PU_BLOCKSTREAM, NULL);
					
						// Otherwise resize
						else
						{
							//Hold
							Z_ResizeArray((void**)&PerfectData->ReadQ, sizeof(*PerfectData->ReadQ),
											PerfectData->SizeReadQ, PerfectData->SizeReadQ + 2);
							Hold = PerfectData->ReadQ[PerfectData->SizeReadQ++] =
									(DS_RBSPerfectHold_t*)Z_Malloc(sizeof(*Hold), PU_BLOCKSTREAM, NULL);
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
						Hold->Data = (uint8_t*)Z_Malloc(Hold->Size, PU_BLOCKSTREAM, NULL);
						memmove(Hold->Data, &PerfectData->WrapStream->BlkData[41], Hold->Size);
					}
				}
				
				// ACK it now
				NormStream = (D_RBlockStream_t*)PerfectData->WrapStream;
				
				// Write to remote host
				D_RBSBaseBlock(NormStream, "PERF");
				
				// Write Send, Key, Num, Sum, Header, Size, MaskEnc
				MaskEnc = 0;
					
					// Send
				D_RBSWriteUInt8(NormStream, 'A');
				MaskEnc ^= 'A';
					// Key
				for (z = 0; z < 4; z++)
				{
					D_RBSWriteUInt32(NormStream, PerfKey[z]);
					MaskEnc ^= PerfKey[z];
				}
					// Num
				PerfPkNumLow = PerfPkNum & (uint64_t)((~UINT32_C(0)));
				PerfPkNumHi = PerfPkNum >> UINT64_C(32);
				
				D_RBSWriteUInt32(NormStream, PerfPkNumLow);
				D_RBSWriteUInt32(NormStream, PerfPkNumHi);
				
				MaskEnc ^= PerfPkNumLow;
				MaskEnc ^= PerfPkNumHi;
					// Sum
				D_RBSWriteUInt32(NormStream, PerfPkSum);
				MaskEnc ^= PerfPkSum;
					// Header
				for (z = 0; z < 4; z++)
				{
					D_RBSWriteUInt8(NormStream, PerfHeader[z]);
					MaskEnc ^= PerfHeader[z];
				}
					// Size -- This is ignored, due to future payload
				D_RBSWriteUInt32(NormStream, 0);
				MaskEnc ^= 0;
					// MaskEnc
				D_RBSWriteUInt32(NormStream, MaskEnc);
				
				// Record it
				D_RBSRecordNetBlock(NormStream, a_Host);
				
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
			D_RBSBaseBlock(a_Stream, Header);
			D_RBSWriteChunk(a_Stream, PerfectData->WrapStream->BlkData, PerfectData->WrapStream->BlkSize);
		
			// Return the original value returned
			return OrigRetVal;
		}
	}
	
	/* Nothing was returned */
	// If any blocks were streamed, they would be played on the next run.
	return false;
}

/* DS_RBSPerfect_FlushF() -- Flushes perfect data */
static bool DS_RBSPerfect_FlushF(struct D_RBlockStream_s* const a_Stream)
{
	DS_RBSPerfectData_t* PerfectData;
	I_HostAddress_t FakeHost;
	
	/* Check */
	if (!a_Stream)
		return false;
	
	/* Get Data */
	PerfectData = (DS_RBSPerfectData_t*)a_Stream->Data;
	
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
bool DS_RBSPerfect_IOCtlF(struct D_RBlockStream_s* const a_Stream, const D_RBSStreamIOCtl_t a_IOCtl, int32_t* a_DataP)
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

/****************
*** FUNCTIONS ***
****************/

/* D_RBSCreateLoopBackStream() -- Creates loop back stream */
D_RBlockStream_t* D_RBSCreateLoopBackStream(void)
{
	D_RBlockStream_t* New;
	
	/* Create block stream */
	New = (D_RBlockStream_t*)Z_Malloc(sizeof(*New), PU_BLOCKSTREAM, NULL);
	
	/* Set Functions */
	New->Data = Z_Malloc(sizeof(DS_RBSLoopBackData_t), PU_BLOCKSTREAM, NULL);
	New->RecordF = DS_RBSLoopBack_RecordF;
	New->PlayF = DS_RBSLoopBack_PlayF;
	New->FlushF = DS_RBSLoopBack_FlushF;
	New->DeleteF = DS_RBSLoopBack_DeleteF;
	
	/* Return */
	return New;
}

/* D_RBSCreateWLStream() -- Creates a stream that wraps an entry stream */
D_RBlockStream_t* D_RBSCreateWLStream(WLEntryStream_c* const a_Stream)
{
	D_RBlockStream_t* New;
	
	/* Check */
	if (!a_Stream)
		return NULL;
	
	/* Create block stream */
	New = (D_RBlockStream_t*)Z_Malloc(sizeof(*New), PU_BLOCKSTREAM, NULL);
	
	/* Setup Data */
	New->Data = a_Stream;
	New->RecordF = DS_RBSWL_RecordF;
	New->PlayF = DS_RBSWL_PlayF;
	New->DeleteF = DS_RBSWL_DeleteF;
	
	/* Return Stream */
	return New;
}

/* D_RBSCreateFileStream() -- Create file stream */
D_RBlockStream_t* D_RBSCreateFileStream(const char* const a_PathName, const uint32_t a_Flags)
{
	FILE* File;
	D_RBlockStream_t* New;
	
	/* Check */
	if (!a_PathName)
		return NULL;
	
	/* Open r or r/w file */
	File = fopen(a_PathName, (((a_Flags & DRBSSF_READONLY) ? "r" : ((a_Flags & DRBSSF_OVERWRITE) ? "w+b" : "a+b"))));
	
	// Failed?
	if (!File)
		return NULL;
	
	/* Create block stream */
	New = (D_RBlockStream_t*)Z_Malloc(sizeof(*New), PU_BLOCKSTREAM, NULL);
	
	/* Setup Data */
	New->Flags = a_Flags;
	New->Data = File;
	New->RecordF = DS_RBSFile_RecordF;
	New->PlayF = DS_RBSFile_PlayF;
	New->DeleteF = DS_RBSFile_DeleteF;
	
	/* Return Stream */
	return New;
}

/* D_RBSCreateNetStream() -- Create network stream */
D_RBlockStream_t* D_RBSCreateNetStream(I_NetSocket_t* const a_NetSocket)
{
	D_RBlockStream_t* New;
	I_RBSNetSockData_t* NetData;
	
	/* Check */
	if (!a_NetSocket)
		return NULL;
	
	/* Create block stream */
	New = (D_RBlockStream_t*)Z_Malloc(sizeof(*New), PU_BLOCKSTREAM, NULL);
	
	/* Setup Data */
	New->Data = NetData = (I_RBSNetSockData_t*)Z_Malloc(sizeof(*NetData), PU_BLOCKSTREAM, NULL);
	New->NetRecordF = DS_RBSNet_NetRecordF;
	New->NetPlayF = DS_RBSNet_NetPlayF;
	New->DeleteF = DS_RBSNet_DeleteF;
	
	/* Load into data */
	NetData->Socket = a_NetSocket;
	
	/* Return Stream */
	return New;
}

/* D_RBSCreatePerfectStream() -- Creates a wrapped "Perfect" Stream */
D_RBlockStream_t* D_RBSCreatePerfectStream(D_RBlockStream_t* const a_Wrapped)
{
	D_RBlockStream_t* New;
	DS_RBSPerfectData_t* Data;
	
	/* Check */
	if (!a_Wrapped)
		return NULL;
	
	/* Create block stream */
	New = (D_RBlockStream_t*)Z_Malloc(sizeof(*New), PU_BLOCKSTREAM, NULL);
	
	/* Setup Data */
	New->Data = Data = (DS_RBSPerfectData_t*)Z_Malloc(sizeof(*Data), PU_BLOCKSTREAM, NULL);
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

/* D_RBSCloseStream() -- Closes File Stream */
void D_RBSCloseStream(D_RBlockStream_t* const a_Stream)
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

/* D_RBSStreamIOCtl() -- Call special IOCtl Handler on stream */
bool D_RBSStreamIOCtl(D_RBlockStream_t* const a_Stream, const D_RBSStreamIOCtl_t a_IOCtl, int32_t* a_DataP)
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

/* D_RBSStatStream() -- Obtain stream stats */
void D_RBSStatStream(D_RBlockStream_t* const a_Stream, uint32_t* const a_ReadBk, uint32_t* const a_WriteBk, uint32_t* const a_ReadBy, uint32_t* const a_WriteBy)
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

/* D_RBSUnStatStream() -- Clear stream stats */
void D_RBSUnStatStream(D_RBlockStream_t* const a_Stream)
{
	size_t i;
	
	/* Check */
	if (!a_Stream)
		return;
	
	/* Reset */
	for (i = 0; i < 2; i++)
		a_Stream->StatBlock[i] = a_Stream->StatBytes[i] = 0;
}

/* D_RBSMarkedStream() -- Returns true if stream is marked */
bool D_RBSMarkedStream(D_RBlockStream_t* const a_Stream)
{
	/* Check */
	if (!a_Stream)
		return false;
	
	return a_Stream->Marked;
}

/* D_RBSCompareHeader() -- Compares headers */
bool D_RBSCompareHeader(const char* const a_A, const char* const a_B)
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

/* D_RBSBaseBlock() -- Base block */
// This used to return void, but I need it to return true for the SaveGame code
bool D_RBSBaseBlock(D_RBlockStream_t* const a_Stream, const char* const a_Header)
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
	a_Stream->BlkData = (uint8_t*)Z_Malloc(a_Stream->BlkBufferSize, PU_BLOCKSTREAM, NULL);
	a_Stream->ReadOff = 0;
	
	// Copy header
	memmove(a_Stream->BlkHeader, a_Header, (strlen(a_Header) >= 4 ? 4 : strlen(a_Header)));
	
	/* Success */
	return true;
}

/* D_RBSRenameHeader() -- Rename block */
bool D_RBSRenameHeader(D_RBlockStream_t* const a_Stream, const char* const a_Header)
{
	/* Check */
	if (!a_Stream || !a_Header)
		return false;
	
	/* Copy header */
	memmove(a_Stream->BlkHeader, a_Header, (strlen(a_Header) >= 4 ? 4 : strlen(a_Header)));
	
	/* Success */
	return true;
}

/* D_RBSRecordBlock() -- Records the current block to the stream */
void D_RBSRecordBlock(D_RBlockStream_t* const a_Stream)
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

/* D_RBSRecordBlock() -- Plays the current block from the stream */
bool D_RBSPlayBlock(D_RBlockStream_t* const a_Stream, char* const a_Header)
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

/* D_RBSPlayNetBlock() -- Plays net block */
bool D_RBSPlayNetBlock(D_RBlockStream_t* const a_Stream, char* const a_Header, I_HostAddress_t* const a_Host)
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

/* D_RBSRecordNetBlock() -- Record net block */
void D_RBSRecordNetBlock(D_RBlockStream_t* const a_Stream, I_HostAddress_t* const a_Host)
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

/* D_RBSFlushStream() -- Flush stream */
bool D_RBSFlushStream(D_RBlockStream_t* const a_Stream)
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

/* D_RBSWriteChunk() -- Write data chunk into block */
size_t D_RBSWriteChunk(D_RBlockStream_t* const a_Stream, const void* const a_Data, const size_t a_Size)
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
#define __REMOOD_RBSQUICK(w,x) void BP_MERGE(D_RBSWrite,w)(D_RBlockStream_t* const a_Stream, const x a_Val)\
{\
	D_RBSWriteChunk(a_Stream, &a_Val, sizeof(a_Val));\
}

__REMOOD_RBSQUICK(Int8,int8_t);
__REMOOD_RBSQUICK(Int16,int16_t);
__REMOOD_RBSQUICK(Int32,int32_t);
__REMOOD_RBSQUICK(Int64,int64_t);
__REMOOD_RBSQUICK(UInt8,uint8_t);
__REMOOD_RBSQUICK(UInt16,uint16_t);
__REMOOD_RBSQUICK(UInt32,uint32_t);
__REMOOD_RBSQUICK(UInt64,uint64_t);

/* D_RBSWriteString() -- Write Version String */
void D_RBSWriteString(D_RBlockStream_t* const a_Stream, const char* const a_Val)
{
	const char* c;
	
	/* Check */
	if (!a_Stream)
		return;
	
	/* Constant Write */
	for (c = a_Val; *c; c++)
		D_RBSWriteUInt8(a_Stream, *c);
	D_RBSWriteUInt8(a_Stream, 0);	// NUL
}

/* D_RBSWritePointer() -- Write Pointer */
void D_RBSWritePointer(D_RBlockStream_t* const a_Stream, const void* const a_Ptr)
{
	size_t i;
	uint64_t XP;
	
	/* Write sizeof() */
	D_RBSWriteUInt8(a_Stream, sizeof(a_Ptr));
	
	/* Write all the pointer bits (for UUID in a way) */
	XP = (uint64_t)((uintptr_t)a_Ptr);
	for (i = 0; i < sizeof(a_Ptr); i++)
		D_RBSWriteUInt8(a_Stream, (uint8_t)(((XP >> ((UINT64_C(8) * ((uint64_t)i)))) & UINT64_C(0xFF))));
		//OP |= ((uint64_t)D_RBSReadUInt8(a_Stream)) << ((UINT64_C(8) * ((uint64_t)i)));
}

/* D_RBSReadChunk() -- Reads Chunk */
size_t D_RBSReadChunk(D_RBlockStream_t* const a_Stream, void* const a_Data, const size_t a_Size)
{
	ssize_t Count;
	size_t Read;
	
	/* Check */
	if (!a_Stream || !a_Data || !a_Size)
		return 0;
	
	/* Counting Read */
	Read = 0;
	for (Count = a_Size; Count > 0; Count--)
		if (a_Stream->ReadOff < a_Stream->BlkSize)
			((uint8_t*)a_Data)[Read++] = a_Stream->BlkData[a_Stream->ReadOff++];
	
	/* Return read amount */
	return Read;
}

#define BP_MERGE(a,b) a##b
#define __REMOOD_RBSQUICKREAD(w,x) x BP_MERGE(D_RBSRead,w)(D_RBlockStream_t* const a_Stream)\
{\
	x Ret;\
	D_RBSReadChunk(a_Stream, &Ret, sizeof(Ret));\
	return Ret;\
}

__REMOOD_RBSQUICKREAD(Int8,int8_t);
__REMOOD_RBSQUICKREAD(Int16,int16_t);
__REMOOD_RBSQUICKREAD(Int32,int32_t);
__REMOOD_RBSQUICKREAD(Int64,int64_t);
__REMOOD_RBSQUICKREAD(UInt8,uint8_t);
__REMOOD_RBSQUICKREAD(UInt16,uint16_t);
__REMOOD_RBSQUICKREAD(UInt32,uint32_t);
__REMOOD_RBSQUICKREAD(UInt64,uint64_t);

/* D_RBSReadString() -- Read String */
size_t D_RBSReadString(D_RBlockStream_t* const a_Stream, char* const a_Out, const size_t a_OutSize)
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
		Char = D_RBSReadUInt8(a_Stream);
		
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

/* D_RBSReadPointer() -- Reads pointer */
uint64_t D_RBSReadPointer(D_RBlockStream_t* const a_Stream)
{
	size_t i;
	uint8_t SizeOf;
	uint64_t OP;
	
	/* Read sizeof() */
	SizeOf = D_RBSReadUInt8(a_Stream);
	
	/* Read in pointer bits */
	OP = 0;
	for (i = 0; i < SizeOf; i++)
		OP |= ((uint64_t)D_RBSReadUInt8(a_Stream)) << ((UINT64_C(8) * ((uint64_t)i)));
	
	/* Return the number */
	return OP;
}

/**************************
*** GENERIC BYTE STREAM ***
**************************/

/* GenericByteStream_c::GenericByteStream_c() -- Constructor */
GenericByteStream_c::GenericByteStream_c()
{
	/* Init */
	p_IsUnicode = false;
}

/* GenericByteStream_c::~GenericByteStream_c() -- Deconstructor */
GenericByteStream_c::~GenericByteStream_c()
{
}

/* GenericByteStream_c::CheckUnicode() -- Checks whether a stream is a Unicode stream */
// ReadChar() and ReadWChar() become translated
bool GenericByteStream_c::CheckUnicode(void)
{
	uint16_t FirstBits;
	
	/* Reposition */
	Seek(0);
	
	/* Read the first bytes */
	FirstBits = ReadUInt16();
	
	/* Determine if it is Unicode */
	if (FirstBits == 0xFFFE)
		p_IsUnicode = true;
	else if (FirstBits == 0xFEFF)
		p_IsUnicode = p_IsSwapped = true;
	
	/* Reposition to start of text */
	if (!p_IsUnicode)
		Seek(0);
	else
		Seek(2);
	
	/* Success */
	return true;
}

/* GenericByteStream_c::ReadChar() -- Reads a single character */
char GenericByteStream_c::ReadChar(void)
{
	char RetVal;
	uint16_t wcTemp;
	
	/* Depending on stream type */
	// Non Unicode or UTF-8
	if (!p_IsUnicode)
		RetVal = ReadUInt8();
	
	// UTF-16/32
	else
	{
		// If UTF-8 buffer contains data, flush it
		if (p_MBLeft)
		{
			// Return this character
			RetVal = p_MBBuf[0];
			
			// Decrease
			memmove(&p_MBBuf[0], &p_MBBuf[1], sizeof(*p_MBBuf) * 4);
			p_MBLeft--;
			
			// Return
			return RetVal;
		}
		
		// Read next UTF-16 character
		if (p_IsSwapped)
			wcTemp = ReadUInt16();
		else
			wcTemp = ReadLittleUInt16();
		
		// If character is > 127, convert to UTF-8 and flush first char
		if (wcTemp > 127)
		{
			// Convert
			p_MBLeft = V_ExtWCharToMB(wcTemp, p_MBBuf);
			
			// Return first character
			if (p_MBLeft)
			{
				RetVal = p_MBBuf[0];
			
				// Decrease
				memmove(&p_MBBuf[0], &p_MBBuf[1], sizeof(*p_MBBuf) * 4);
				p_MBLeft--;
			
				// Return
				return RetVal;
			}
		}
		
		// A normal character
		else
			RetVal = wcTemp;
	}
	
	/* Return */
	return RetVal;
}

/* GenericByteStream_c::ReadLine() -- Reads Unicode Line from file */
size_t GenericByteStream_c::ReadLine(char* const a_Buf, const size_t a_Size)
{
	size_t RetVal;
	char Char;
	
	/* Check */
	if (!a_Buf || !a_Size)
		return 0;
	
	/* Read characters until \n */
	for (RetVal = 0, Char = ReadChar(); !EndOfStream(); Char = ReadChar())
	{
		// If character is \n, the stream has ended
		if (Char == '\n')
			break;
		
		// Ignore '\r'
		if (Char == '\r')
			continue;
		
		// Place in buffer
		if (RetVal < a_Size - 1)
			a_Buf[RetVal++] = Char;
	}
	
	/* Return retval */
	return RetVal;
}

/* GenericByteStream_c::ReadString() -- Reads string (\0 term) from stream */
size_t GenericByteStream_c::ReadString(char* const a_Buf, const size_t a_Size)
{
	size_t i;
	uint8_t Char;
	
	/* Check */
	if (!a_Buf || !a_Size)
		return 0;
	
	/* Read variable string data */
	for (i = 0; ; i++)
	{
		// Read Character
		Char = ReadUInt8();
		
		// End of string?
		if (!Char)
		{
			// Append NUL, if possible
			if (i < a_Size)
				a_Buf[i] = 0;
			break;
		}
		
		// Otherwise add to the output
		if (i < a_Size)
			a_Buf[i] = Char;
	}
	
	/* Always put NUL at very end */
	a_Buf[a_Size - 1] = 0;
	
	/* Return read count */
	return i;
}

/* GenericByteStream_c::WriteString() -- Writes string */
void GenericByteStream_c::WriteString(const char* const a_Buf)
{
	const char* c;
	
	/* Check */
	if (!a_Buf)
		return;
	
	/* Constant Write */
	for (c = a_Buf; *c; c++)
		WriteUInt8(*c);
	WriteUInt8(0);	// NUL
}

#define __REMOOD_GBSMERGE(a,b) a##b

#define __REMOOD_GBSREADINT(N,T) T GenericByteStream_c::__REMOOD_GBSMERGE(Read,N)(void)\
{\
	T Value;\
	uint64_t OffBase;\
	OffBase = ReadChunk(&Value, sizeof(Value));\
	if (Seekable())\
		Seek(Tell() + OffBase);\
	return Value;\
}

__REMOOD_GBSREADINT(Int8,int8_t);
__REMOOD_GBSREADINT(Int16,int16_t);
__REMOOD_GBSREADINT(Int32,int32_t);
__REMOOD_GBSREADINT(Int64,int64_t);
__REMOOD_GBSREADINT(UInt8,uint8_t);
__REMOOD_GBSREADINT(UInt16,uint16_t);
__REMOOD_GBSREADINT(UInt32,uint32_t);
__REMOOD_GBSREADINT(UInt64,uint64_t);

#define __REMOOD_GBSWRITEINT(N,T) void GenericByteStream_c::__REMOOD_GBSMERGE(Write,N)(const T a_Value)\
{\
	uint64_t OffBase;\
	OffBase = WriteChunk(&a_Value, sizeof(a_Value));\
	if (Seekable())\
		Seek(Tell() + OffBase);\
}

__REMOOD_GBSWRITEINT(Int8,int8_t);
__REMOOD_GBSWRITEINT(Int16,int16_t);
__REMOOD_GBSWRITEINT(Int32,int32_t);
__REMOOD_GBSWRITEINT(Int64,int64_t);
__REMOOD_GBSWRITEINT(UInt8,uint8_t);
__REMOOD_GBSWRITEINT(UInt16,uint16_t);
__REMOOD_GBSWRITEINT(UInt32,uint32_t);
__REMOOD_GBSWRITEINT(UInt64,uint64_t);

#define __REMOOD_RBDLITTLEREAD(N,T) T GenericByteStream_c::__REMOOD_GBSMERGE(ReadLittle,N)(void)\
{\
	T Val;\
	Val = __REMOOD_GBSMERGE(Read,N)();\
	return __REMOOD_GBSMERGE(LittleSwap,N)(Val);\
}

__REMOOD_RBDLITTLEREAD(Int16,int16_t);
__REMOOD_RBDLITTLEREAD(Int32,int32_t);
__REMOOD_RBDLITTLEREAD(Int64,int64_t);
__REMOOD_RBDLITTLEREAD(UInt16,uint16_t);
__REMOOD_RBDLITTLEREAD(UInt32,uint32_t);
__REMOOD_RBDLITTLEREAD(UInt64,uint64_t);

#define __REMOOD_RBDBIGREAD(N,T) T GenericByteStream_c::__REMOOD_GBSMERGE(ReadBig,N)(void)\
{\
	T Val;\
	Val = __REMOOD_GBSMERGE(Read,N)();\
	return __REMOOD_GBSMERGE(BigSwap,N)(Val);\
}

__REMOOD_RBDBIGREAD(Int16,int16_t);
__REMOOD_RBDBIGREAD(Int32,int32_t);
__REMOOD_RBDBIGREAD(Int64,int64_t);
__REMOOD_RBDBIGREAD(UInt16,uint16_t);
__REMOOD_RBDBIGREAD(UInt32,uint32_t);
__REMOOD_RBDBIGREAD(UInt64,uint64_t);

#define __REMOOD_RBDLITTLEWRITE(N,T) void GenericByteStream_c::__REMOOD_GBSMERGE(WriteLittle,N)(const T a_Value)\
{\
	__REMOOD_GBSMERGE(Write,N)(__REMOOD_GBSMERGE(LittleSwap,N)(a_Value));\
}

__REMOOD_RBDLITTLEWRITE(Int16,int16_t);
__REMOOD_RBDLITTLEWRITE(Int32,int32_t);
__REMOOD_RBDLITTLEWRITE(Int64,int64_t);
__REMOOD_RBDLITTLEWRITE(UInt16,uint16_t);
__REMOOD_RBDLITTLEWRITE(UInt32,uint32_t);
__REMOOD_RBDLITTLEWRITE(UInt64,uint64_t);

/*******************************
*** CLASS BASED BLOCK STREAM ***
*******************************/

/*** ADDRESS DESCRIPTOR ***/

/* RBAddress_c::RBAddress_c() -- Constructor */
RBAddress_c::RBAddress_c()
{
}

/* RBAddress_c::~RBAddress_c() -- Destructor */
RBAddress_c::~RBAddress_c()
{
}

/* RBAddress_c::SetFrom() -- Copies one address to another */
void RBAddress_c::SetFrom(const RBAddress_c& a_B)
{
	p_Type = a_B.p_Type;
}

/* RBAddress_c::CompareAddress() -- Compares two addresses */
bool RBAddress_c::CompareAddress(const RBAddress_c& a_B) const
{
	return CompareAddress(*this, a_B);
}

/* RBAddress_c::CompareAddress() -- Compares two addresses */
bool RBAddress_c::CompareAddress(const RBAddress_c& a_A, const RBAddress_c& a_B)
{
	/* Different Address Types? */
	if (a_A.p_Type != a_B.p_Type)
		return false;
	
	/* Un-Initialized? */
	switch (a_A.p_Type)
	{
			// Local Address
		case AT_LOCAL:
			return true;
			
			// Un-Initialized or unknown
		case AT_NOTINIT:
		default:
			return false;
	}
}

/* RBAddress_c::ChooseLocal() -- Choose local address */
void RBAddress_c::ChooseLocal(void)
{
	p_Type = AT_LOCAL;
}

/*** BASE STREAM ***/

/* RBStream_c::RBStream_c -- Constructor */
RBStream_c::RBStream_c()
{
	/* Init */
	memset(p_BlkHeader, 0, sizeof(p_BlkHeader));
	p_Data = NULL;
	p_Flags = 0;
	p_BlkData = 0;
	p_BlkSize = 0;
	p_BlkBufferSize = 0;
	p_ReadOff = 0;
	p_Marked = 0;
}

/* RBStream_c::~RBStream_c() -- Destructor */
RBStream_c::~RBStream_c()
{
	/* Free data if it exists */
	if (p_BlkData)
		Z_Free(p_BlkData);
}

/* RBStream_c::Seekable() -- Block streams are not seekable */
bool RBStream_c::Seekable(void)
{
	return false;
}

/* RBStream_c::EndOfStream() -- Returns true if at end of data stream */
bool RBStream_c::EndOfStream(void)
{
	if (p_ReadOff >= p_BlkSize)
		return true;
	return false;
}

/* RBStream_c::Tell() -- Current read position */
uint64_t RBStream_c::Tell(void)
{
	return p_ReadOff;
}

/* RBStream_c::Seek() -- Seek to new location */
uint64_t RBStream_c::Seek(const uint64_t a_NewPos, const bool a_AtEnd)
{
	// Always returns current position
	return Tell();
}

/* RBStream_c::ReadChunk() -- Reads chunk from block */
size_t RBStream_c::ReadChunk(void* const a_Data, const size_t a_Size)
{
	ssize_t Count;
	size_t Read;
	
	/* Check */
	if (!a_Data || !a_Size)
		return 0;
	
	/* Counting Read */
	Read = 0;
	for (Count = a_Size; Count > 0; Count--)
		if (p_ReadOff < p_BlkSize)
			((uint8_t*)a_Data)[Read++] = p_BlkData[p_ReadOff++];
	
	/* Return read amount */
	return Read;
}

/* RBStream_c::WriteChunk() -- Writes piece of information to block chunk */
size_t RBStream_c::WriteChunk(const void* const a_Data, const size_t a_Size)
{
	size_t DestSize;
	
	/* Check */
	if (!a_Data || !a_Size)
		return 0;
	
	/* Determine Block Resize */
	DestSize = p_BlkSize + a_Size;
	
	// Too big?
	while (DestSize >= p_BlkBufferSize)
	{
		Z_ResizeArray((void**)&p_BlkData, sizeof(*p_BlkData), p_BlkBufferSize, p_BlkBufferSize + RBLOCKBUFSIZE);
		p_BlkBufferSize += RBLOCKBUFSIZE;
	}
	
	/* Slap data there */
	memmove(&p_BlkData[p_BlkSize], a_Data, a_Size);
	p_BlkSize += a_Size;
	
	/* Return size */
	return a_Size;
}

/* RBStream_c::CompareHeader() -- Compares block header */
bool RBStream_c::CompareHeader(const char* const a_A, const char* const a_B)
{
	/* Check */
	if (!a_A || !a_B)
		return false;
	
	/* See if it matches */
	if (strcasecmp(a_A, a_B) == 0)
		return true;
	
	/* No match */
	return false;
}

/* RBStream_c::BlockBase() -- Creates a new empty block */
bool RBStream_c::BlockBase(const char* const a_Header)
{
	/* Check */
	if (!a_Header)
		return false;
	
	/* Clear Everything */
	memset(p_BlkHeader, 0, sizeof(p_BlkHeader));
	
	if (p_BlkData)
		Z_Free(p_BlkData);
	p_BlkSize = p_BlkBufferSize = 0;
	
	/* Create a fresh block */
	p_BlkBufferSize = RBLOCKBUFSIZE;
	p_BlkData = (uint8_t*)Z_Malloc(p_BlkBufferSize, PU_BLOCKSTREAM, NULL);
	p_ReadOff = 0;
	
	// Copy header
	memmove(p_BlkHeader, a_Header, (strlen(a_Header) >= 4 ? 4 : strlen(a_Header)));
	
	/* Success */
	return true;
}

/* RBStream_c::BlockRename() -- Renames the header of a block */
bool RBStream_c::BlockRename(const char* const a_Header)
{
	/* Check */
	if (!a_Header)
		return false;
	
	/* Copy header */
	memset(p_BlkHeader, 0, sizeof(p_BlkHeader));
	memmove(p_BlkHeader, a_Header, (strlen(a_Header) >= 4 ? 4 : strlen(a_Header)));
	
	/* Success */
	return true;
}

/* RBStream_c::HeaderCopy() -- Copy Header */
bool RBStream_c::HeaderCopy(char* const a_Dest)
{
	strcpy(a_Dest, p_BlkHeader);
	return true;
}

/* RBStream_c::BlockSize() -- Return size of block */
size_t RBStream_c::BlockSize(void)
{
	return p_BlkSize;
}

/* RBStream_c::BlockData() -- Return data of block */
uint8_t* RBStream_c::BlockData(void)
{
	return p_BlkData;
}

/*** LOOP BACK STREAM ***/

/* RBLoopBackStream_c::RBLoopBackStream_c() -- Creates loopback */
RBLoopBackStream_c::RBLoopBackStream_c()
{
	/* Init */
	p_FlushID = 0;
	p_Q = 0;
	p_SizeQ = 0;
}

/* RBLoopBackStream_c::~RBLoopBackStream_c() -- Destroys loopback */
RBLoopBackStream_c::~RBLoopBackStream_c()
{
	size_t i;
	
	/* Destroy all blocks */
	if (p_Q)
	{
		for (i = 0; i < p_SizeQ; i++)
			if (!p_Q[i])
				Z_Free(p_Q[i]);
		Z_Free(p_Q);
	}
}

/* RBLoopBackStream_c::BlockPlay() -- Plays block from loopback */
bool RBLoopBackStream_c::BlockPlay(RBAddress_c* const a_Address)
{
	RBLoopBackHold_s* Hold;
	size_t i;
	
	/* Always set address to local */
	if (a_Address)
		a_Address->ChooseLocal();
	
	/* Nothing recorded? */
	if (!p_SizeQ)
		return false;
	
	/* Nothing in the Q? */
	if (!p_Q[0])
		return false;
	
	/* Get current hold */
	Hold = p_Q[0];
	
	// See if it after the current flush level
	if (p_FlushID <= Hold->FlushID)
		return false;
	
	/* Create Base Block */
	BlockBase(Hold->Header);
	
	// Write all our data in it
	WriteChunk(Hold->Data, Hold->Size);
	
	/* Free Hold */
	if (Hold->Data)
		Z_Free(Hold->Data);
	Hold->Data = NULL;
	Z_Free(Hold);
	p_Q[0] = NULL;
	
	// Move everything down
	if (p_SizeQ >= 1)
	{
		for (i = 0; i < p_SizeQ - 1; i++)
			p_Q[i] = p_Q[i + 1];
		p_Q[i] = NULL;
	}
	
	// Nothing left?
	if (!p_Q[0])
		p_FlushID = 1;
	
	/* Something read */
	return true;
}

/* RBLoopBackStream_c::BlockRecord() -- Records block to loopback */
bool RBLoopBackStream_c::BlockRecord(RBAddress_c* const a_Address)
{
	size_t i;
	RBLoopBackHold_s* Hold;
	
	/* Always set address to local */
	if (a_Address)
		a_Address->ChooseLocal();
	
	/* Find spot to record at */
	Hold = NULL;
	for (i = 0; i < p_SizeQ; i++)
		if (!p_Q[i])
		{
			Hold = p_Q[i] = (RBLoopBackHold_s*)Z_Malloc(sizeof(RBLoopBackHold_s), PU_BLOCKSTREAM, NULL);
			break;
		}
	
	// No blank spots?
	if (!Hold)
	{
		Z_ResizeArray((void**)&p_Q, sizeof(*p_Q),
						p_SizeQ, p_SizeQ + 2);
		Hold = p_Q[p_SizeQ++] =
				(RBLoopBackHold_s*)Z_Malloc(sizeof(RBLoopBackHold_s), PU_BLOCKSTREAM, NULL);
		p_SizeQ++;
	}
	
	/* Store info in hold */
	// Copy header
	memmove(Hold->Header, p_BlkHeader, 4);
	
	// Clone Data
	Hold->Size = p_BlkSize;
	Hold->Data = (uint8_t*)Z_Malloc(Hold->Size, PU_BLOCKSTREAM, NULL);
	memmove(Hold->Data, p_BlkData, Hold->Size);
	Hold->FlushID = p_FlushID + 1;
	
	/* Block was recorded */
	return true;
}

/* RBLoopBackStream_c::BlockFlush() -- Flushes read and recorded blocks */
bool RBLoopBackStream_c::BlockFlush(void)
{
	/* No data in Q */
	if (!p_SizeQ)
		return false;
	
	/* Q is empty */
	if (!p_Q[0])
		return false;
		
	/* Increase flush ID */
	p_FlushID++;
	
	/* Something done */
	return true;
}

/*** WL ENTRY STREAMS ***/

/* RBWLEStream_c::RBWLEStream_c() -- Creates entry stream */
RBWLEStream_c::RBWLEStream_c(WLEntryStream_c* const a_Stream)
{
	/* Init */
	p_WLStream = a_Stream;
}

/* RBWLEStream_c::~RBWLEStream_c() -- Deletes entry stream */
RBWLEStream_c::~RBWLEStream_c()
{
}

/* RBWLEStream_c::BlockPlay() -- Plays back a block */
bool RBWLEStream_c::BlockPlay(RBAddress_c* const a_Address)
{
	char Header[5];
	uint32_t Len, Sum, i;
	void* Data;
	
	/* Read Header */
	// Clear
	memset(Header, 0, sizeof(Header));
	Len = Sum = 0;
	Data = NULL;
	
	// Read Header
	for (i = 0; i < 4; i++)
		Header[i] = p_WLStream->ReadChar();
	
	// Read Length and Sum
	Len = p_WLStream->ReadLittleUInt32();
	Sum = p_WLStream->ReadLittleUInt32();
	
	// Read data, if possible (Len could be zero (empty block?))
	if (Len > 0)
	{
		Data = Z_Malloc(Len, PU_STATIC, NULL);
		p_WLStream->ReadChunk(Data, Len);
	}
	
	/* Initialize Block */
	BlockBase(Header);
	
	/* Write Data to Block */
	WriteChunk(Data, Len);
	if (Data)
		Z_Free(Data);
	
	/* Success! */
	return true;
}

/* RBWLEStream_c::BlockRecord() -- WLStreams are read only */
bool RBWLEStream_c::BlockRecord(RBAddress_c* const a_Address)
{
	return false;
}

/* RBWLEStream_c::BlockFlush() -- Flushing not needed */
bool RBWLEStream_c::BlockFlush(void)
{
	return true;	// Always flushed
}

/*** PERFECT STREAM ***/

/* RBPerfectStream_c::RBPerfectStream_c() -- Creates perfect stream */
RBPerfectStream_c::RBPerfectStream_c(RBStream_c* const a_WrapStream)
{
	/* Init */
	// Load stuff into data
	p_WrapStream = a_WrapStream;
	
	// Debug?
	if (M_CheckParm("-perfdev"))
		p_Debug = true;
}

/* RBPerfectStream_c::~RBPerfectStream_c() -- Destroys a perfect stream */
RBPerfectStream_c::~RBPerfectStream_c()
{
	size_t i;
	
	/* Delete blocks */
	// Read Queue
	if (p_ReadQ)
	{
		for (i = 0; i < p_SizeReadQ; i++)
			if (p_ReadQ[i])
				Z_Free(p_ReadQ[i]);
		Z_Free(p_ReadQ);
	}
			
	// Write Queue
	if (p_WriteQ)
	{
		for (i = 0; i < p_SizeWriteQ; i++)
			if (p_WriteQ[i])
				Z_Free(p_WriteQ[i]);
		Z_Free(p_WriteQ);
	}
	
	/* Delete keys */
	if (p_Keys)
	{
		for (i = 0; i < p_NumKeys; i++)
			if (p_Keys[i])
				Z_Free(p_Keys[i]);
		Z_Free(p_Keys);
	}
}

/* RBPerfectStream_c::IsPerfect() -- Packet read is perfect */
bool RBPerfectStream_c::IsPerfect(void)
{
	return p_Marked;
}

/* RBPerfectStream_c::IntFindKey() -- Finds a key internally */
RBPerfectStream_c::RBPerfectKey_s* RBPerfectStream_c::IntFindKey(const uint32_t* const a_InKey, RBAddress_c* const a_Address)
{
	size_t k, FbK, j, i, b, z;
	RBPerfectKey_s* Key;
	uint32_t ThisTime;
	RBPerfectHold_s* Hold;
	
	/* Get Current Time */
	ThisTime = I_GetTimeMS();
	
	/* Look in existing key list */
	for (FbK = 0, k = 0; k < p_NumKeys; k++)
	{
		// Get Key
		Key = p_Keys[k];
		
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
			for (b = 0; b < p_SizeReadQ; b++)
				if (p_ReadQ[b])
				{
					// Get Block
					Hold = p_ReadQ[b];
					
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
						p_ReadQ[b] = NULL;
					}
				}
				
				// Write Q
			for (b = 0; b < p_SizeWriteQ; b++)
				if (p_WriteQ[b])
				{
					// Get Block
					Hold = p_WriteQ[b];
					
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
						p_WriteQ[b] = NULL;
					}
				}
			
			// Delete self key
			Z_Free(Key);
			Key = p_Keys[k] = NULL;
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
			if (!a_Address->CompareAddress(Key->RemHost))
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
	if (FbK > 0 && FbK < p_NumKeys)
		Key = p_Keys[FbK] = new RBPerfectKey_s;
	
	// Resize the key array
	else
	{
		Z_ResizeArray((void**)&p_Keys, sizeof(*p_Keys),
			p_NumKeys, p_NumKeys + 1);
		Key = p_Keys[p_NumKeys++] = new RBPerfectKey_s;
	}
	
	// Setup key info
	Key->RemHost.SetFrom(*a_Address);
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
				Key->Key[i] ^= (j + (M_Random() * M_Random()) + (((uintptr_t)this) - ((uintptr_t)a_InKey))) << ((j + i) & 24);
		}
	
	/* Place in empty spot */
	// First known
	if (FbK > 0 && FbK < p_NumKeys)
		p_Keys[FbK] = Key;
	
	/* Return the new key */
	return Key;
}

/* RBPerfectStream_c::BlockPlay() -- Plays a block back */
bool RBPerfectStream_c::BlockPlay(RBAddress_c* const a_Address)
{
	char Header[5];
	bool OrigRetVal;
	RBStream_c* NormStream;
	RBPerfectHold_s* Hold;
	
	size_t i, z, b, BlankSpot;
	
	RBPerfectKey_s* Key;
	
	uint8_t PerfResp;
	uint32_t PerfKey[4];
	uint64_t PerfPkNum;
	uint32_t PerfPkSum, PerfPkSize, PerfMaskEnc, PerfPkNumLow, PerfPkNumHi;
	uint8_t PerfHeader[5];
	uint32_t ConfirmMask, MaskEnc;
	uint32_t BlockSum;
	
	uint32_t ThisTime;
	
	/* Clear perfection mark */
	p_Marked = false;
	ThisTime = I_GetTimeMS();
	
	/* Go through ordered stream */
	// And determine if there are any blocks that are OK and queued
	for (b = 0; b < p_SizeReadQ; b++)
	{
		// Get Current Hold
		Hold = p_ReadQ[b];
	
		// Nothing here?
		if (!Hold)
			continue;
		
		// Get key associated with block
		Key = IntFindKey(Hold->Key, &Hold->RemHost);
		
		// No key found? probably revoked, delete block
		if (!Key)
		{
			if (Hold->Data)
				Z_Free(Hold->Data);
			Hold->Data = NULL;
			delete Hold;
			p_ReadQ[b] = NULL;
			continue;
		}
		
		// If the block does not match the current read pos, skip it
		// This would mean that we got blocks out of order
		if (Hold->PacketNum != Key->NextReadNum)
			continue;	// Still want to keep it though!
		
		// Build block
		BlockBase(Hold->Header);
		WriteChunk(Hold->Data, Hold->Size);
		
		// Copy IP Host -- This is very important
		if (a_Address)
			a_Address->SetFrom(Hold->RemHost);
		
		// Was sent away, so delete, increment, return
		if (Hold->Data)
			Z_Free(Hold->Data);
		Hold->Data = NULL;
		delete Hold;
		p_ReadQ[b] = NULL;
		Key->NextReadNum++;
		p_Marked = true;	// and set perfection!
		return true;
	}
	
	/* Constantly read blocks */
	for (;;)
	{
		// Clear
		memset(Header, 0, sizeof(Header));
		OrigRetVal = p_WrapStream->BlockPlay(a_Address);
		p_WrapStream->HeaderCopy(Header);
	
		// If no block was read, return
		if (!OrigRetVal)
			return false;
	
		// Perfect Block
		if (D_RBSCompareHeader("PERF", Header))
		{
			// Read this block and check to see if it is ordered enough and is
			// fully checksummed and correct. If the block is good return it,
			// otherwise add it to a queue to wait on ordered blocks. Also do
			// handling of retransmission and such.
			
			// Read PERFECT Header
			PerfResp = p_WrapStream->ReadUInt8();
			
			for (i = 0; i < 4; i++)
				PerfKey[i] = p_WrapStream->ReadLittleUInt32();
			
			PerfPkNumLow = p_WrapStream->ReadLittleUInt32();
			PerfPkNumHi = p_WrapStream->ReadLittleUInt32();
			
			PerfPkNum = PerfPkNumHi;
			PerfPkNum <<= UINT64_C(32);
			PerfPkNum |= PerfPkNumLow;
			
			PerfPkSum = p_WrapStream->ReadLittleUInt32();
			
			PerfHeader[4] = 0;
			for (i = 0; i < 4; i++)
				PerfHeader[i] = p_WrapStream->ReadUInt8();
			
			PerfPkSize = p_WrapStream->ReadLittleUInt32();
			PerfMaskEnc = p_WrapStream->ReadLittleUInt32();
			
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
			Key = IntFindKey(PerfKey, a_Address);
			
			// Compare the input key to the found key
			for (i = 0; Key && i < 4; i++)
				if (Key->Key[i] != PerfKey[i])
					Key = NULL;
			
			// No Key?
			if (!Key)
				continue;
			
			// Compare packet size
			if (PerfPkSize != p_WrapStream->BlockSize() - 41)
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
					BlockSum ^= ((uint32_t)(p_WrapStream->BlockData()[i + 41])) << (i & 23);
				
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
					for (BlankSpot = 0, b = 0; b < p_SizeReadQ; b++)
					{
						// Get Current Hold
						Hold = p_ReadQ[b];
						
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
					if (b >= p_SizeReadQ)
						Hold = NULL;
					
					// Pre-existing block not found
					if (!Hold)
					{
						// Use blank spot?
						if (BlankSpot > 0 && BlankSpot < p_SizeReadQ)
							Hold = p_ReadQ[BlankSpot] = new RBPerfectHold_s;
					
						// Otherwise resize
						else
						{
							//Hold
							Z_ResizeArray((void**)&p_ReadQ, sizeof(*p_ReadQ),
											p_SizeReadQ, p_SizeReadQ + 2);
							Hold = p_ReadQ[p_SizeReadQ++] = new RBPerfectHold_s;
							p_SizeReadQ++;
						}
						
						// Fill in information
						Hold->RemHost.SetFrom(*a_Address);
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
						Hold->Data = (uint8_t*)Z_Malloc(Hold->Size, PU_BLOCKSTREAM, NULL);
						memmove(Hold->Data, &p_WrapStream->BlockData()[41], Hold->Size);
					}
				}
				
				// ACK it now
				NormStream = (RBStream_c*)p_WrapStream;
				
				// Write to remote host
				NormStream->BlockBase("PERF");
				
				// Write Send, Key, Num, Sum, Header, Size, MaskEnc
				MaskEnc = 0;
					
					// Send
				NormStream->WriteUInt8('A');
				MaskEnc ^= 'A';
					// Key
				for (z = 0; z < 4; z++)
				{
					NormStream->WriteLittleUInt32(PerfKey[z]);
					MaskEnc ^= PerfKey[z];
				}
					// Num
				PerfPkNumLow = PerfPkNum & (uint64_t)((~UINT32_C(0)));
				PerfPkNumHi = PerfPkNum >> UINT64_C(32);
				
				NormStream->WriteLittleUInt32(PerfPkNumLow);
				NormStream->WriteLittleUInt32(PerfPkNumHi);
				
				MaskEnc ^= PerfPkNumLow;
				MaskEnc ^= PerfPkNumHi;
					// Sum
				NormStream->WriteLittleUInt32(PerfPkSum);
				MaskEnc ^= PerfPkSum;
					// Header
				for (z = 0; z < 4; z++)
				{
					NormStream->WriteUInt8(PerfHeader[z]);
					MaskEnc ^= PerfHeader[z];
				}
					// Size -- This is ignored, due to future payload
				NormStream->WriteLittleUInt32(0);
				MaskEnc ^= 0;
					// MaskEnc
				NormStream->WriteLittleUInt32(MaskEnc);
				
				// Record it
				NormStream->BlockRecord(a_Address);
				
				// Was added to Q so deal with later
				continue;
			}
			
			// ACK Block
			else if (PerfResp == 'A')
			{
				// Find matching write block and set as acknowledged
				for (b = 0; b < p_SizeWriteQ; b++)
				{
					//CONL_PrintF("ack!\n");
					
					// Get Current Hold
					Hold = p_WriteQ[b];
					
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
			BlockBase(Header);
			WriteChunk(p_WrapStream->BlockData(), p_WrapStream->BlockSize());
		
			// Return the original value returned
			return OrigRetVal;
		}
	}
	
	/* Nothing was returned */
	// If any blocks were streamed, they would be played on the next run.
	return false;
}

/* RBPerfectStream_c::BlockRecord() -- Records block */
bool RBPerfectStream_c::BlockRecord(RBAddress_c* const a_Address)
{
	size_t i, b, z;
	RBPerfectHold_s* Hold;
	RBPerfectKey_s* Key;
	bool KeepSending;
	uint32_t ThisTime, MaskEnc;
	RBStream_c* NormStream;
	uint32_t PerfPkNumLow, PerfPkNumHi;
	
	/* Get Time */
	ThisTime = I_GetTimeMS();
	
	/* First find the key to use */
	// Don't do this in the middle of a flush (since there is no data to record)
	if (!p_InFlush)
	{
		Key = IntFindKey(NULL, a_Address);
	
		// Second record the packet into the write Q
		if (Key)
		{
			// Find blank spot
			Hold = NULL;
			for (i = 0; i < p_SizeWriteQ; i++)
				if (!p_WriteQ[i])
				{
					Hold = p_WriteQ[i] = new RBPerfectHold_s;
					break;
				}
	
			// No blank spots?
			if (!Hold)
			{
				Z_ResizeArray((void**)&p_WriteQ, sizeof(*p_WriteQ),
								p_SizeWriteQ, p_SizeWriteQ + 2);
				Hold = p_WriteQ[p_SizeWriteQ++] = new RBPerfectHold_s;
				p_SizeWriteQ++;
			}
	
			// Store block in the hold
				// Header
			memmove(Hold->Header, p_BlkHeader, 4);
	
				// Data
			Hold->Size = p_BlkSize;
			Hold->Data = (uint8_t*)Z_Malloc(Hold->Size, PU_BLOCKSTREAM, NULL);
			memmove(Hold->Data, p_BlkData, Hold->Size);
	
				// Remote Host
			if (a_Address)
				Hold->RemHost.SetFrom(*a_Address);
		
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
		for (b = 0; b < p_SizeWriteQ; b++)
		{
			// Get Current Hold
			Hold = p_WriteQ[b];
			
			// Nothing here?
			if (!Hold)
				continue;
			
			// Obtain key from block
			Key = IntFindKey(Hold->Key, &Hold->RemHost);
			
			// No key? Must have expired
			if (!Key)
			{
				// Delete block, only IF this spot is blank
					// because if a key was revoked as we tried to grab it, then
					// this could be invalid!
				if (p_WriteQ[b])
				{
					if (Hold->Data)
						Z_Free(Hold->Data);
					Hold->Data = NULL;
					Z_Free(Hold);
				}
				
				p_WriteQ[b] = NULL;
				continue;
			}
			
			// Packet was ACKed!
			if (Hold->BlockAck)
			{
				// Delete block
				if (Hold->Data)
					Z_Free(Hold->Data);
				Hold->Data = NULL;
				delete Hold;
				p_WriteQ[b] = NULL;
				
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
				delete Hold;
				p_WriteQ[b] = NULL;
				
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
				NormStream = p_WrapStream;
				
				// Write to remote host
				NormStream->BlockBase("PERF");
				
				// Write Send, Key, Num, Sum, Header, Size, MaskEnc
				MaskEnc = 0;
					
					// Send
				NormStream->WriteUInt8('S');
				MaskEnc ^= 'S';
					// Key
				for (z = 0; z < 4; z++)
				{
					NormStream->WriteLittleUInt32(Hold->Key[z]);
					MaskEnc ^= Hold->Key[z];
				}
					// Num
				PerfPkNumLow = Hold->PacketNum & (uint64_t)((~UINT32_C(0)));
				PerfPkNumHi = Hold->PacketNum >> UINT64_C(32);
				
				NormStream->WriteLittleUInt32(PerfPkNumLow);
				NormStream->WriteLittleUInt32(PerfPkNumHi);
				
				MaskEnc ^= PerfPkNumLow;
				MaskEnc ^= PerfPkNumHi;
					// Sum
				NormStream->WriteLittleUInt32(Hold->CheckSum);
				MaskEnc ^= Hold->CheckSum;
					// Header
				for (z = 0; z < 4; z++)
				{
					NormStream->WriteUInt8(Hold->Header[z]);
					MaskEnc ^= Hold->Header[z];
				}
					// Size
				NormStream->WriteLittleUInt32(Hold->Size);
				MaskEnc ^= Hold->Size;
					// MaskEnc
				NormStream->WriteLittleUInt32(MaskEnc);
				
				// Write actual block data
				NormStream->WriteChunk(Hold->Data, Hold->Size);
				
				// Record to stream
				NormStream->BlockRecord(&Hold->RemHost);
			}
		}
		
		// Stop sending packets like crazy;
		KeepSending = false;
	} while (KeepSending);

	/* Return value does not matter */
	return true;
}

/* RBPerfectStream_c::BlockFlush() -- Flushes the block stream */
bool RBPerfectStream_c::BlockFlush(void)
{
	RBAddress_c FakeHost;
	
	/* Sync? */
	p_InFlush = true;
	BlockRecord(&FakeHost);
	p_InFlush = false;
	
	return true;
}

/*** MULTICAST STREAM ***/

/* RBMultiCastStream_c::RBMultiCastStream_c() -- Constructor */
RBMultiCastStream_c::RBMultiCastStream_c()
{
}

/* RBMultiCastStream_c::~RBMultiCastStream_c() -- Destructor */
RBMultiCastStream_c::~RBMultiCastStream_c()
{
}

/* RBMultiCastStream_c::AddMultiCast() -- Adds target */
void RBMultiCastStream_c::AddMultiCast(RBStream_c* const a_OtherStream, RBAddress_c* const a_Address)
{
	size_t i;
	
	/* Check */
	if (!a_OtherStream || !a_Address)
		return;
	
	/* Make sure it doesn't exist already */
	for (i = 0; i < p_NumCasts; i++)
		if (p_Casts[i])
			if (p_Casts[i]->OtherStream == a_OtherStream && a_Address->CompareAddress(*p_Casts[i]->Address))
				return;
	
	/* Find free spot */
	for (i = 0; i < p_NumCasts; i++)
		if (!p_Casts[i])
			break;
	
	// No room?
	if (i >= p_NumCasts)
	{
		Z_ResizeArray((void**)&p_Casts, sizeof(*p_Casts), p_NumCasts, p_NumCasts + 1);
		i = p_NumCasts++;
	}
	
	/* Place here */
	p_Casts[i] = new MultiCastInfo_s;
	p_Casts[i]->OtherStream = a_OtherStream;
	p_Casts[i]->Address = a_Address;
}

/* RBMultiCastStream_c::DelMultiCast() -- Removes target */
void RBMultiCastStream_c::DelMultiCast(RBStream_c* const a_OtherStream, RBAddress_c* const a_Address)
{
	size_t i;
	
	/* Check */
	if (!a_OtherStream || !a_Address)
		return;
		
	/* Remove from list */
	for (i = 0; i < p_NumCasts; i++)
		if (p_Casts[i])
			if (p_Casts[i]->OtherStream == a_OtherStream && a_Address->CompareAddress(*p_Casts[i]->Address))
			{
				delete p_Casts[i];
				p_Casts[i] = NULL;
				return;
			}
}

/* RBMultiCastStream_c::BlockPlay() -- Does Nothing */
bool RBMultiCastStream_c::BlockPlay(RBAddress_c* const a_Address)
{
	// You cannot read from multicast
	return false;
}

/* RBMultiCastStream_c::BlockRecord() -- Sends block away en masse */
bool RBMultiCastStream_c::BlockRecord(RBAddress_c* const a_Address)
{
	size_t i;
	
	if (devparm)
		CONL_PrintF("Multicast record (%s %i)!\n", p_BlkHeader, p_BlkSize);
	
	/* Send to everyone in the group */
	for (i = 0; i < p_NumCasts; i++)
		if (p_Casts[i])
		{
			// Create block base
			p_Casts[i]->OtherStream->BlockBase(p_BlkHeader);
			
			// Copy chunk in whole pieces
			p_Casts[i]->OtherStream->WriteChunk(p_BlkData, p_BlkSize);
			
			// Record on stream
			p_Casts[i]->OtherStream->BlockRecord(p_Casts[i]->Address);
		}
	
	/* Always works! */
	return true;
}

/* RBMultiCastStream_c::BlockPlay() -- Does Nothing */
bool RBMultiCastStream_c::BlockFlush(void)
{
	// Flushing multicasts is not needed
	return false;
}

