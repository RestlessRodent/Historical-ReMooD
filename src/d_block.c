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
	
	/* Check */
	if (!a_Stream)
		return 0;
	
	/* Get Data */
	File = (FILE*)a_Stream->Data;
}

/****************
*** FUNCTIONS ***
****************/

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
	
	/* Return Stream */
	return New;
}

/* D_RBSCreateNetStream() -- Create network stream */
D_RBlockStream_t* D_RBSCreateNetStream(I_NetSocket_t* const a_NetSocket)
{
	return NULL;
}

/* D_RBSCloseStream() -- Closes File Stream */
void D_RBSCloseStream(D_RBlockStream_t* const a_Stream)
{
	/* Check */
	if (!a_Stream)
		return;
}

/* D_RBSBaseBlock() -- Base block */
void D_RBSBaseBlock(D_RBlockStream_t* const a_Stream, const char* const a_Header)
{
	/* Check */
	if (!a_Stream || !a_Header)
		return;
	
	/* Clear Everything */
	memset(a_Stream->BlkHeader, 0, sizeof(a_Stream->BlkHeader));
	
	if (a_Stream->BlkData)
		Z_Free(a_Stream->BlkData);
	a_Stream->BlkSize = a_Stream->BlkBufferSize = 0;
	
	/* Create a fresh block */
	a_Stream->BlkBufferSize = RBLOCKBUFSIZE;
	a_Stream->BlkData = Z_Malloc(sizeof(a_Stream->BlkBufferSize), PU_BLOCKSTREAM, NULL);
	
	// Copy header
	memmove(a_Stream->BlkHeader, a_Header, (strlen(a_Header) >= 4 ? 4 : strlen(a_Header)));
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
}

/* D_RBSWriteChunk() -- Write data chunk into block */
size_t D_RBSWriteChunk(D_RBlockStream_t* const a_Stream, const uint8_t* const a_Data, const size_t a_Size)
{
	/* Check */
	if (!a_Stream || !a_Data || !a_Size)
		return 0;
}

