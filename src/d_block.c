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

/**********************
*** RELIABLE STREAM ***
*********************/

#define RBSMAXRELIABLEKEEP				128		// Max reliable keeps
#define RBSRELIABLERETRAN				500		// Retransmit reliables after this
#define RBSRELDECAYADD					50		// Retransmit decay rate
#define RBSRELMAXDECAY					5000	// Maximum decay retransmission
#define RBSRELIABLETIMEOUT				120000	// Remove connection after 2 min
#define RBSRELINPUTQUEUE				128		// Packets in input queue

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

/* DS_RelInputPk_t -- Reliable input packet */
typedef struct DS_RelInputPk_s
{
	// Storage Information
	char Header[5];								// Packer Header
	I_HostAddress_t Address;					// Address
	uintptr_t Size;								// Size
	void* Data;									// Data
	
	// Reliable Info
	bool_t IsAuth;								// Is Authentic
	bool_t IsEaten;								// Being used
} DS_RelInputPk_t;

struct DS_RBSReliableData_s;

/* DS_ReliableFlat_t -- Reliable storage area */
typedef struct DS_ReliableFlat_s
{
	struct DS_RBSReliableData_s* Owner;			// Owner	
	
	uint32_t HostHash;							// Hash of host
	I_HostAddress_t Address;					// Address of host
	
	DS_ReliablePk_t OutPks[RBSMAXRELIABLEKEEP];	// Output packets
	
	uint64_t GoCount;							// Go count (sent stuff)
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
	uint32_t RR;								// Round rover
	
	DS_RelInputPk_t InPks[RBSRELINPUTQUEUE];	// Packets read from wrapped stream
	uint32_t InWait;							// Packets waiting
	
	D_BS_t* UnAuthBuf;							// Unauthorized buffer
	
	bool_t IsAuth;								// Is packet authorized
	bool_t IsOnList;							// Is on auth list
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
	
	/* Kill all contained data */
	for (i = 0; i < RBSRELINPUTQUEUE; i++)
		if (a_Data->InPks[i].Data)
			Z_Free(a_Data->InPks[i].Data);
	memset(a_Data->InPks, 0, sizeof(a_Data->InPks));
	a_Data->InWait = 0;
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
		CurFlat->Owner = a_Data;
		CurFlat->HostHash = CurHash;
		memmove(&CurFlat->Address, a_Address, sizeof(I_HostAddress_t));
		
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
	
	int32_t FirstSlot;
	
	DS_RelInputPk_t *IPKp;
	
	/* Get Data */
	RelData = a_Stream->Data;
	
	// Check
	if (!RelData)
		return false;
	
	/* Init */
	memset(Header, 0, sizeof(Header));
	memset(&Addr, 0, sizeof(Addr));
	memset(NotActive, 0xFF, sizeof(NotActive));
	
	// Current time
	ThisTime = I_GetTimeMS();
	
	/* Find first slot input can be read from */
	for (FirstSlot = 0; FirstSlot < RBSRELINPUTQUEUE; FirstSlot++)
		if (!RelData->InPks[FirstSlot].IsEaten)
			break;
	
	/* Play blocks from the wrapped stream */
	while (FirstSlot < RBSRELINPUTQUEUE)
	{
		// Bounds here
		IPKp = &RelData->InPks[FirstSlot];
		
		// If this spot is taken, skip ahead
		if (IPKp->IsEaten)
		{
			FirstSlot++;
			continue;
		}
		
		// Play single block (was while, but this is less intensive)
		if (D_BSPlayNetBlock(RelData->WrapStream, Header, &Addr))
		{
			// Get flat for host
			CurFlat = DS_RBSReliable_FlatByHost(RelData, &Addr, false);
			
			// Reliable Data?
			if (D_BSCompareHeader(Header, "RELY"))
			{
				// "Spoofed" or previously dropped, ignore them
				if (!CurFlat)
					continue;
				
				// Read reliable header
				inCode = D_BSru8(RelData->WrapStream);
				inSlot = D_BSru8(RelData->WrapStream);
				inKey = D_BSru16(RelData->WrapStream);
				inTime = D_BSru32(RelData->WrapStream);
				inFTime = D_BSru32(RelData->WrapStream);
				inSize = D_BSru16(RelData->WrapStream);
				
				// Invalid Code?
				if (inCode != 'P' && inCode != 'A')
					continue;	// Drop packet, who cares
				
				// Acknowledge Packet
				else if (inCode == 'A')
				{
					// Slot out of bounds?
					if (inSlot < 0 || inSlot >= RBSMAXRELIABLEKEEP)
						continue;	// Drop
					
					// Get packet data
					PkP = &CurFlat->OutPks[inSlot];
					
					// Wrong key?
					if ((inKey & UINT16_C(0xFFFF)) != (PkP->Key & UINT16_C(0xFFFF)))
						continue;
					
					// Clear packet info
					PkP->Key = 0;
					PkP->Time = PkP->FirstTime = PkP->Decay = 0;
					PkP->Transmit = 0;
					PkP->Header[0] = PkP->Header[1] = PkP->Header[2] = PkP->Header[3] = 0;
					PkP->Size = 0;
					
					if (PkP->Data)
						Z_Free(PkP->Data);
					PkP->Data = NULL;
				}
				
				// Data Packet
				else if (inCode == 'P')
				{
					// Read Header and size
					for (i = 0; i < 4; i++)
						IPKp->Header[i] = D_BSru8(RelData->WrapStream);
					IPKp->Header[i] = 0;
					
					IPKp->Size = inSize;
					
					// Set other info
					memmove(&IPKp->Address, &Addr, sizeof(IPKp->Address));
					IPKp->IsAuth = true;
					IPKp->IsEaten = true;
					FirstSlot++;	// Slot up for next run
					
					// Read data
					IPKp->Data = Z_Malloc(IPKp->Size, PU_STATIC, NULL);
					D_BSReadChunk(RelData->WrapStream, IPKp->Data, IPKp->Size);
					
					// Reply with ACK (to say we got it)
					D_BSBaseBlock(RelData->WrapStream, "RELY");
					
					D_BSwu8(RelData->WrapStream, 'A');	// A for ack
					D_BSwu8(RelData->WrapStream, inSlot);	// Current slot
					D_BSwu16(RelData->WrapStream, inKey);
					D_BSwu32(RelData->WrapStream, inTime);
					D_BSwu32(RelData->WrapStream, inFTime);
					D_BSwu16(RelData->WrapStream, inSize);
					
					D_BSRecordNetBlock(RelData->WrapStream, &CurFlat->Address);
					
					// Flush wrapped stream
					D_BSFlushStream(RelData->WrapStream);
					
					// Some waiting
					RelData->InWait++;
				}
			}
			
			// Un-Reliable
			else
			{
				// Copy header
				for (i = 0; i < 4; i++)
					IPKp->Header[i] = Header[i];
				IPKp->Header[i] = 0;
				
				IPKp->Size = RelData->WrapStream->BlkSize;
				
				// Set other info
				memmove(&IPKp->Address, &Addr, sizeof(IPKp->Address));
				IPKp->IsAuth = false;
				IPKp->IsEaten = true;
				FirstSlot++;	// Slot up for next run
				
				// Read data
				IPKp->Data = Z_Malloc(IPKp->Size, PU_STATIC, NULL);
				D_BSReadChunk(RelData->WrapStream, IPKp->Data, IPKp->Size);
					
				// Some waiting
				RelData->InWait++;
			}
		}
		
		// No packets read, so no need to continue
		else
			break;
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
			D_BSwu8(RelData->WrapStream, j);	// Current slot
			D_BSwu16(RelData->WrapStream, PkP->Key);
			D_BSwu32(RelData->WrapStream, PkP->Time);
			D_BSwu32(RelData->WrapStream, PkP->FirstTime);
			D_BSwu16(RelData->WrapStream, PkP->Size);
			
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
	DS_ReliablePk_t* Pack, *FirstFree, *Oldest;
	
	int32_t i;
	
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

	/* Go through output store */
	// The previous code wrote to an output store, how terrible, and the same
	// stream was never even flushed, so there would be insane spikes. Also,
	// these spikes were viral, so you would spike out of control.
	// So instead of just writing to a loop back block queue, send it directly
	// but record the information for retransmissions.
	for (FirstFree = Oldest = NULL, i = 0; i < RBSMAXRELIABLEKEEP; i++)
	{
		// Get
		Pack = &Flat->OutPks[i];
		
		// No packet here?
		if (!Pack->Key)
		{
			if (!FirstFree)
				FirstFree = Pack;
			break;	// For speed, since we found one, no sense!
		}
		
		// Older? Bug every 49 days, but no too much of a worry
		if (!Oldest || Pack->FirstTime < Oldest->FirstTime)
			Oldest = Pack;
	}
	
	/* Choose a packet spot to place in */
	if (FirstFree)
		Pack = FirstFree;
	else
		Pack = Oldest;
	
	/* Place data with key and such */
	// Wipe packet first
	if (Pack->Data)
		Z_Free(Pack->Data);
	memset(Pack, 0, sizeof(*Pack));
	
	// Setup fields
	for (Pack->Key = 0; !Pack->Key; Pack->Key = D_CMakePureRandom() & UINT16_C(0xFFFF));
	Pack->Time = Pack->FirstTime = I_GetTimeMS();
	Pack->Decay = 0;
	Pack->Transmit = true;
	Pack->Header[0] = a_Stream->BlkHeader[0];
	Pack->Header[1] = a_Stream->BlkHeader[1];
	Pack->Header[2] = a_Stream->BlkHeader[2];
	Pack->Header[3] = a_Stream->BlkHeader[3];
	Pack->Size = a_Stream->BlkSize;
	Pack->Data = Z_Malloc(a_Stream->BlkSize, PU_STATIC, NULL);
	
	// Place data there
	memmove(Pack->Data, a_Stream->BlkData, Pack->Size);
	
	/* Flush reliable stream to transmit */
	D_BSFlushStream(a_Stream);
	
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
	bool_t SkipFlats;
	DS_RelInputPk_t *IPKp;
	
	/* Get Data */
	RelData = a_Stream->Data;
	
	// Check
	if (!RelData)
		return false;
	
	/* Flush ourself */
	if (RelData->InWait == 0)
		D_BSFlushStream(a_Stream);
	
	/* Go through the input buffer, round robin style */
	Stop = ((RelData->RR - 1) + RBSRELINPUTQUEUE) % RBSRELINPUTQUEUE;
	if (RelData->InWait > 0)
		while ((RelData->RR = (RelData->RR + 1) % RBSRELINPUTQUEUE) != Stop)
		{
			// Ref
			IPKp = &RelData->InPks[RelData->RR];
		
			// Nothing here
			if (!IPKp->IsEaten)
				continue;
		
			// Authentic?
			RelData->IsAuth = IPKp->IsAuth;
		
			// Clone data
			D_BSBaseBlock(a_Stream, IPKp->Header);
			D_BSWriteChunk(a_Stream, IPKp->Data, IPKp->Size);
		
			// Set address
			if (a_Host)
				memmove(a_Host, &IPKp->Address, sizeof(*a_Host));
		
			// Free packet data
			if (IPKp->Data)
				Z_Free(IPKp->Data);
		
			memset(IPKp, 0, sizeof(*IPKp));
			
			// Less waiting
			RelData->InWait--;
		
			// Read something
			return true;
		}
	
	/* No packets read at all! */
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
	
	/* Close unauth lo */
	D_BSCloseStream(RelData->UnAuthBuf);
}

/* DS_RBSReliable_IOCtlF() -- Advanced I/O Control */
bool_t DS_RBSReliable_IOCtlF(struct D_BS_s* const a_Stream, const D_BSStreamIOCtl_t a_IOCtl, const intptr_t a_DataP)
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
			*((intptr_t*)a_DataP) = RelData->IsPerf;
			return true;
			
			// Max supported transport
		case DRBSIOCTL_MAXTRANSPORT:
			*((intptr_t*)a_DataP) = RelData->TransSize - 32;
			return true;
			
			// Adds a single host
		case DRBSIOCTL_ADDHOST:
			DS_RBSReliable_FlatByHost(RelData, a_DataP, true);
			return true;
			
			// Drop a single host
		case DRBSIOCTL_DROPHOST:
			DS_RBSReliable_KillFlat(RelData, DS_RBSReliable_FlatByHost(RelData, a_DataP, false));
			return true;
			
			// Reset reliable streams
		case DRBSIOCTL_RELRESET:
			DS_RBSReliable_Reset(RelData);
			break;
			
			// Authenticated Packet?
		case DRBSIOCTL_ISAUTH:
			*((bool_t*)a_DataP) = RelData->IsAuth;
			break;
			
			// Check if on auth list
		case DRBSIOCTL_CHECKISONLIST:
			RelData->IsOnList = !!DS_RBSReliable_FlatByHost(RelData, a_DataP, false);
			break;
			
			// return state of last cehck
		case DRBSIOCTL_GETISONLIST:
			*((bool_t*)a_DataP) = RelData->IsOnList;
			break;
			
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
		Data->TransSize = RBSPACKEDMAXWAITBYTES;
	
	// Unauth buffer
	Data->UnAuthBuf = D_BSCreateLoopBackStream();
	
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


