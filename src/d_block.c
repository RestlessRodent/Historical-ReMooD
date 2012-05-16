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

#include <stdio.h>

#include "d_block.h"
#include "z_zone.h"

/*****************
*** STRUCTURES ***
*****************/

/* D_TBlock_s -- Transport block */
struct D_TBlock_s
{
	uint32_t Magic;								// Block magic
	uint32_t Flags;								// Flags for blokc
	uint32_t DataSize;							// Size of block
	void* Data;									// Block Data
};

/**************************
*** GENERIC FILE STREAM ***
**************************/

/* D_GFS_DeleteStream() -- Delete file stream */
void D_GFS_DeleteStream(struct D_TStreamSource_s* const a_Stream)
{
	/* Check */
	if (!a_Stream)
		return;
	
	/* Close file */
	if (a_Stream->Data)
		fclose(a_Stream->Data);
}

/* D_GFS_Send() -- Write data to file */
D_TBlockErr_t D_GFS_Send(struct D_TStreamSource_s* const a_Stream, D_TBlock_t** const a_BlkPtrIn, const uint32_t a_TFlags, D_TStreamStat_t* const a_StatPtr, void** const a_DataPtr)
{
	FILE* f;
	size_t Total;
	
	/* Check */
	if (!a_Stream || !a_BlkPtrIn || !*a_BlkPtrIn || !a_DataPtr)
		return DTBE_INVALIDARGUMENT;
		
	/* Get origin file */
	f = (FILE*)a_Stream->Data;
	
	/* Write data */
	Total = 0;
	
	if (fwrite(&(*a_BlkPtrIn)->Magic, 4, 1, f) > 0)
		Total += 4;
	if (fwrite(&(*a_BlkPtrIn)->DataSize, 4, 1, f) > 0)
		Total += 4;
	if (fwrite((*a_BlkPtrIn)->Data, (*a_BlkPtrIn)->DataSize, 1, f) > 0)
		Total += (*a_BlkPtrIn)->DataSize;
	
	/* Stat */
	if (a_StatPtr)
	{
		a_StatPtr->LastTime[0] = I_GetTimeMS();
		a_StatPtr->BlkXMit[0]++;
		a_StatPtr->ByteXMit[0] += Total;
	}
	
	/* Success */
	return DTBE_SUCCESS;
}

/* D_GFS_Recv() -- Read data from file */
D_TBlockErr_t D_GFS_Recv(struct D_TStreamSource_s* const a_Stream, D_TBlock_t** const a_BlkPtrOut, const uint32_t a_TFlags, D_TStreamStat_t* const a_StatPtr, void** const a_DataPtr)
{
}

/* D_CreateFileInStream() -- Create file input (read) stream */
D_TStreamSource_t* D_CreateFileInStream(const char* const a_PathName)
{
	return NULL;
}

/* D_CreateFileOutStream() -- Create file output (write) stream */
D_TStreamSource_t* D_CreateFileOutStream(const char* const a_PathName)
{
	D_TStreamSource_t* New;
	FILE* f;
	
	/* Check */
	if (!a_PathName)
		return NULL;
	
	/* Try opening the file */
	f = fopen(a_PathName, "w+b");
	
	// Check
	if (!f)
		return NULL;
	
	/* Create */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Fill */
	New->Data = f;
	New->FuncSend = D_GFS_Send;
	New->FuncDeleteStream = D_GFS_DeleteStream;
	
	/* Return */
	return New;
}

/****************
*** FUNCTIONS ***
****************/

/* D_BlockStreamDelete() -- Delete stream */
void D_BlockStreamDelete(D_TStreamSource_t* const a_Stream)
{
	if (!a_Stream)
		return;
	
	/* Call delete func */
	if (a_Stream->FuncDeleteStream)
		a_Stream->FuncDeleteStream(a_Stream);
	
	/* Free */
	Z_Free(a_Stream);
}

/* D_BlockNew() -- Creates a new block */
D_TBlock_t* D_BlockNew(const uint32_t a_Magic, const uint32_t a_Flags, const uint32_t a_Size, void** const a_DataPtr)
{
	D_TBlock_t* New;
	
	/* Create */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Fill */
	New->Magic = a_Magic;
	New->Flags = a_Flags;
	
	// Is there possible data?
	if (a_DataPtr)
	{
		New->DataSize = a_Size;
		New->Data = Z_Malloc(New->DataSize, PU_STATIC, NULL);
		
		*a_DataPtr = New->Data;
	}
	
	/* Return */
	return New;
}

/* D_CharToMagic() -- Character stream to magic */
uint32_t D_CharToMagic(const char* const a_CharMagic)
{
	uint32_t Ret;
	const char* p;
	
	/* Check */
	if (!a_CharMagic)
		return 0;
	
	/* Run */
	for (Ret = 0, p = a_CharMagic; *p; p++)
	{
		Ret <<= 8;
		Ret |= *p;
	}
	
	return Ret;
}

/* D_BlockFree() -- Frees an allocated block */
void D_BlockFree(D_TBlock_t* const a_Block)
{
	/* Check */
	if (!a_Block)
		return;
	
	/* Free data if it exists */
	if (a_Block->Data)
		Z_Free(a_Block->Data);
	a_Block->Data = NULL;
	
	/* Clear */
	a_Block->Magic = 0;
	a_Block->Flags = 0;
	a_Block->DataSize = 0;
	
	/* Free */
	Z_Free(a_Block);
}

/* D_BlockRecv() -- Receive block */
D_TBlockErr_t D_BlockRecv(D_TStreamSource_t* const a_Stream, D_TBlock_t** const a_BlkPtr)
{
	/* Check */
	if (!a_Stream || !a_BlkPtr)
		return DTBE_INVALIDARGUMENT;
	
	/* Check if there is a recv func */
	if (!a_Stream->FuncRecv)
		return DTBE_NOTTHISDIRECTION;
	
	/* Use function */
	return a_Stream->FuncRecv(a_Stream, a_BlkPtr, a_Stream->BlockDefFlags, &a_Stream->Stat, &a_Stream->Data);
}

/* D_BlockSend() -- Send block */
D_TBlockErr_t D_BlockSend(D_TStreamSource_t* const a_Stream, D_TBlock_t** const a_BlkPtr)
{
	if (!a_Stream || !a_BlkPtr)
		return DTBE_INVALIDARGUMENT;
		
	/* Check if there is a send func */
	if (!a_Stream->FuncSend)
		return DTBE_NOTTHISDIRECTION;
	
	/* Use function */
	return a_Stream->FuncSend(a_Stream, a_BlkPtr, a_Stream->BlockDefFlags, &a_Stream->Stat, &a_Stream->Data);
}

/*****************************************************************************/

/******************
*** FILE STREAM ***
******************/

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
bool_t DS_RBSFile_PlayF(struct D_RBlockStream_s* const a_Stream)
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
	if (fread(&Header, 4, 1, File) < 1)
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
bool_t DS_RBSLoopBack_PlayF(struct D_RBlockStream_s* const a_Stream)
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
bool_t DS_RBSLoopBack_FlushF(struct D_RBlockStream_s* const a_Stream)
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

/* DS_RBSNet_NetRecordF() -- Write block to network */
size_t DS_RBSNet_NetRecordF(struct D_RBlockStream_s* const a_Stream, I_HostAddress_t* const a_Host)
{
	I_RBSNetSockData_t* NetData;
	I_NetSocket_t* Socket;
	size_t RetVal, sz, i;
	uint32_t BlockSize;
	uint8_t* TBuf = NULL;
	
	/* Check */
	if (!a_Stream)
		return 0;
	
	/* Get Data */
	NetData = a_Stream->Data;
	
	if (!NetData)
		return 0;
	
	/* Get socket */
	Socket = NetData->Socket;
	
	/* Create temporary buffer for sending */
	sz = 4 + 4 + a_Stream->BlkSize;
	TBuf = Z_Malloc(sz, PU_BLOCKSTREAM, NULL);//alloca(sz);
	
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
bool_t DS_RBSNet_NetPlayF(struct D_RBlockStream_s* const a_Stream, I_HostAddress_t* const a_Host)
{
	I_RBSNetSockData_t* NetData;
	I_NetSocket_t* Socket;
	char Header[5];
	uint32_t Len;
	void* Data;
	size_t RetVal;
	
	/* Check */
	if (!a_Stream)
		return false;
	
	/* Get Data */
	NetData = a_Stream->Data;
	
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

/* DS_RBSPerfectData_t -- Perfect Stream */
typedef struct DS_RBSPerfectData_s
{
	D_RBlockStream_t* WrapStream;				// Stream to wrap
} DS_RBSPerfectData_t;

/* DS_RBSPerfect_NetRecordF() -- Write block to perfect stream */
size_t DS_RBSPerfect_NetRecordF(struct D_RBlockStream_s* const a_Stream, I_HostAddress_t* const a_Host)
{
	return 0;
}

/* DS_RBSPerfect_NetPlayF() -- Play block from the perfect stream */
bool_t DS_RBSPerfect_NetPlayF(struct D_RBlockStream_s* const a_Stream, I_HostAddress_t* const a_Host)
{
	char Header[5];
	DS_RBSPerfectData_t* Data;
	bool_t OrigRetVal;
	
	/* Check */
	if (!a_Stream)
		return false;
	
	/* Get Data */
	Data = a_Stream->Data;
	
	// Nothing there?
	if (!Data)
		return false;
	
	/* Go through ordered stream */
	// And determine if there are any blocks that are OK and queued
	
	/* Constantly read blocks */
	for (;;)
	{
		// Clear
		memset(Header, 0, sizeof(Header));
		OrigRetVal = D_RBSPlayNetBlock(Data->WrapStream, Header, a_Host);
	
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
		}
	
		// Normal Block
		else
		{
			// Duplicate the data as needed
				// Don't worry about host because that was copied already (ptrs)
			D_RBSBaseBlock(a_Stream, Header);
			D_RBSWriteChunk(a_Stream, Data->WrapStream->BlkData, Data->WrapStream->BlkSize);
		
			// Return the original value returned
			return OrigRetVal;
		}
	}
	
	/* Nothing was returned */
	return false;
}

/****************
*** FUNCTIONS ***
****************/

/* D_RBSCreateLoopBackStream() -- Creates loop back stream */
D_RBlockStream_t* D_RBSCreateLoopBackStream(void)
{
	D_RBlockStream_t* New;
	
	/* Create block stream */
	New = Z_Malloc(sizeof(*New), PU_BLOCKSTREAM, NULL);
	
	/* Set Functions */
	New->Data = Z_Malloc(sizeof(DS_RBSLoopBackData_t), PU_BLOCKSTREAM, NULL);
	New->RecordF = DS_RBSLoopBack_RecordF;
	New->PlayF = DS_RBSLoopBack_PlayF;
	New->FlushF = DS_RBSLoopBack_FlushF;
	
	/* Return */
	return New;
}

/* D_RBSCreateFileStream() -- Create file stream */
D_RBlockStream_t* D_RBSCreateFileStream(const char* const a_PathName)
{
	FILE* File;
	D_RBlockStream_t* New;
	
	/* Check */
	if (!a_PathName)
		return NULL;
	
	/* Open r/w file */
	File = fopen(a_PathName, "a+b");
	
	// Failed?
	if (!File)
		return;
	
	/* Create block stream */
	New = Z_Malloc(sizeof(*New), PU_BLOCKSTREAM, NULL);
	
	/* Setup Data */
	New->Data = File;
	New->RecordF = DS_RBSFile_RecordF;
	New->PlayF = DS_RBSFile_PlayF;
	
	/* Return Stream */
	return New;
}

/* D_RBSCreateNetStream() -- Create network stream */
D_RBlockStream_t* D_RBSCreateNetStream(I_NetSocket_t* const a_NetSocket)
{
	FILE* File;
	D_RBlockStream_t* New;
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
	New = Z_Malloc(sizeof(*New), PU_BLOCKSTREAM, NULL);
	
	/* Setup Data */
	New->Data = Data = Z_Malloc(sizeof(*Data), PU_BLOCKSTREAM, NULL);
	New->NetRecordF = DS_RBSPerfect_NetRecordF;
	New->NetPlayF = DS_RBSPerfect_NetPlayF;
	
	// Load stuff into data
	Data->WrapStream = a_Wrapped;
	
	/* Return Stream */
	return New;
}

/* D_RBSCloseStream() -- Closes File Stream */
void D_RBSCloseStream(D_RBlockStream_t* const a_Stream)
{
	/* Check */
	if (!a_Stream)
		return;
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

/* D_RBSCompareHeader() -- Compares headers */
bool_t D_RBSCompareHeader(const char* const a_A, const char* const a_B)
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
bool_t D_RBSBaseBlock(D_RBlockStream_t* const a_Stream, const char* const a_Header)
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

/* D_RBSRenameHeader() -- Rename block */
bool_t D_RBSRenameHeader(D_RBlockStream_t* const a_Stream, const char* const a_Header)
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
bool_t D_RBSPlayBlock(D_RBlockStream_t* const a_Stream, char* const a_Header)
{
	/* Check */
	if (!a_Stream)
		return;
	
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
bool_t D_RBSPlayNetBlock(D_RBlockStream_t* const a_Stream, char* const a_Header, I_HostAddress_t* const a_Host)
{
	/* Check */
	if (!a_Stream)
		return;
	
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
bool_t D_RBSFlushStream(D_RBlockStream_t* const a_Stream)
{
	/* Check */
	if (!a_Stream)
		return;
	
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
__REMOOD_RBSQUICK(UInt8,uint8_t);
__REMOOD_RBSQUICK(UInt16,uint16_t);
__REMOOD_RBSQUICK(UInt32,uint32_t);

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
		D_RBSWriteUInt8(a_Stream, ((XP >> ((UINT64_C(8) * ((uint64_t)i)))) & UINT64_C(0xFF)));
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
__REMOOD_RBSQUICKREAD(UInt8,uint8_t);
__REMOOD_RBSQUICKREAD(UInt16,uint16_t);
__REMOOD_RBSQUICKREAD(UInt32,uint32_t);

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

