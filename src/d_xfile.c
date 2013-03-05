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
// DESCRIPTION: Extended Network Protocol -- Files

/***************
*** INCLUDES ***
***************/

#include "d_xpro.h"
#include "i_util.h"
#include "md5.h"
#include "console.h"
#include "z_mlzo.h"
#include "d_main.h"

/****************
*** CONSTANTS ***
****************/

#define CHUNKSIZE 1024							// Size of chunks

#define MAXHOLDCHUNKS	64						// Chunks to hold at once

#define REQUESTDELAY TICRATE					// Request delay for more chunks

/*****************
*** STRUCTURES ***
*****************/

/* D_XFile_t -- XFile */
typedef struct D_XFile_s
{
	int32_t Ref;								// Reference
	int32_t* RefPtr;							// Pointer to reference holder
	
	int32_t FromRef;							// From reference
	I_HostAddress_t FromAddr;					// From Address
	
	D_BS_t* SendRelBS;							// Reliable BS
	D_BS_t* SendStdBS;							// Standard BS
	
	I_File_t* File;								// File pointer
	char Path[PATH_MAX];						// Path to file
	uint32_t Size;								// Size of files
	uint32_t NumChunks;							// Number of chunks
	uint32_t ChunkSize;							// Size of chunks
	
	uint32_t CheckSum[4];						// MD5 Sum
	char CheckSumChars[33];						// MD5 Sum (As hex characters)
	
	D_XDesc_t* RecvDesc;						// Recieve descriptor
	uint32_t RecvBase;							// Base chunk received
	uint8_t* RecvHold;							// Receiving hold
	tic_t RecvLastReq;							// Last chunk request
	bool_t RecvGot[MAXHOLDCHUNKS];				// Chunks got in hold
	
	tic_t StartTime, EndTime;					// Transfer times
} D_XFile_t;

/*************
*** LOCALS ***
*************/

static int32_t l_XFLastRef = 0;					// Last file ref

static D_XFile_t** l_XF = NULL;					// XFiles
static size_t l_NumXF = 0;						// Number of XFiles

/****************
*** FUNCTIONS ***
****************/

/* DS_XFDeleteFile() -- Deletes file */
static void DS_XFDeleteFile(D_XFile_t* const a_File)
{
	size_t i;
	
	/* Find in list and clear */
	for (i = 0; i < l_NumXF; i++)
		if (l_XF[i] == a_File)
			l_XF[i] = NULL;
	
	/* Clear ref */
	if (a_File->RefPtr)
		*a_File->RefPtr = 0;
	
	/* Clear hold */
	if (a_File->RecvHold)
		Z_Free(a_File->RecvHold);
	
	/* Close file */
	if (a_File->File)
		I_FileClose(a_File->File);
	a_File->File = NULL;
	
	/* Clear */
	Z_Free(a_File);
}

/* DS_FindByRef() -- Find existing file by reference */
static D_XFile_t* DS_FindByRef(const int32_t a_Ref, const bool_t a_Remote)
{
	int32_t i;
	
	/* Check */
	if (!a_Ref)
		return NULL;
	
	/* Look in list */
	for (i = 0; i < l_NumXF; i++)
		if (l_XF[i])
			if ((!a_Remote && l_XF[i]->Ref == a_Ref) || (a_Remote && l_XF[i]->FromRef == a_Ref))
				return l_XF[i];
	
	/* Not found */
	return NULL;
}

/* DS_BlankFile() -- Initializes blank file */
static D_XFile_t* DS_BlankFile(void)
{
	size_t i;
	D_XFile_t* New;
	
	/* Allocate */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Init */
	New->Ref = ++l_XFLastRef;
	
	/* Add more files */
	for (i = 0; i < l_NumXF; i++)
		if (!l_XF[i])
		{
			l_XF[i] = New;
			break;
		}
	
	// No free slot
	if (i >= l_NumXF)
	{
		Z_ResizeArray((void**)&l_XF, sizeof(*l_XF),
			l_NumXF, l_NumXF + 1);
		l_XF[l_NumXF++] = New;
	}
	
	/* Return */
	return New;
}

/* DS_SendChunk() -- Sends file chunk */
static void DS_SendChunk(D_XFile_t* const a_File, I_HostAddress_t* const a_Addr, D_BS_t* const a_BS, const uint32_t a_Chunk)
{
	static uint8_t* ChunkBuf;
	static size_t ChunkSize;
	static uint8_t* LZOBuf;
	lzo_uint LZOSize;
	static uint8_t* WorkBuf;
	
	/* Check */
	if (!a_File)
		return;
	
	/* No File? Out of bounds? Remote file? */
	if (!a_File->File || a_File->FromRef || a_Chunk >= a_File->NumChunks)
		return;
	
	/* Re-allocate? */
	if (!ChunkBuf || !ChunkSize || a_File->ChunkSize > ChunkSize)
	{
		// Clear old
		if (ChunkBuf)
			Z_Free(ChunkBuf);
		if (LZOBuf)
			Z_Free(LZOBuf);
		
		// Allocate
		ChunkSize = a_File->ChunkSize;
		ChunkBuf = Z_Malloc(ChunkSize, PU_STATIC, NULL);
		LZOBuf = Z_Malloc(ChunkSize + 256, PU_STATIC, NULL);
	}
	
	/* Allocate work buffer? */
	if (!WorkBuf)
		WorkBuf = Z_Malloc(LZO1X_1_MEM_COMPRESS, PU_STATIC, NULL);
	
	/* Clear buffer */
	memset(ChunkBuf, 0, ChunkSize);
	memset(LZOBuf, 0, ChunkSize + 256);
	memset(WorkBuf, 0, LZO1X_1_MEM_COMPRESS);
	
	/* Read piece of file */
	I_FileSeek(a_File->File, a_File->ChunkSize * a_Chunk, false);
	I_FileRead(a_File->File, ChunkBuf, a_File->ChunkSize);
	
	/* Compress with LZO */
	LZOSize = ChunkSize + 256;
	lzo1x_1_compress(
			ChunkBuf, a_File->ChunkSize,
			LZOBuf, &LZOSize,
			WorkBuf
		);
	
	/* Build packet and send */
	D_BSBaseBlock(a_BS, "FILE");
	
	D_BSwu8(a_BS, 'c');
	D_BSwi32(a_BS, a_File->Ref);
	D_BSwu32(a_BS, a_Chunk);
	D_BSwu32(a_BS, LZOSize);
	
	D_BSWriteChunk(a_BS, LZOBuf, ((uint32_t)LZOSize));
	
	// Send
	D_BSRecordNetBlock(a_BS, a_Addr);
}

/* DS_XFReadChunk() -- Reads Chunk */
static void DS_XFReadChunk(D_XFile_t* const a_File, D_BS_t* const a_BS, const uint32_t a_Chunk, const uint32_t a_Size)
{
	static uint8_t* ChunkBuf;
	static size_t ChunkSize;
	static uint8_t* LZOBuf;
	lzo_uint LZOSize;
	uint32_t RelChunk;
	
	/* Check */
	if (!a_File)
		return;
	
	/* No File? Out of bounds? Local File? */
	if (!a_File->File || !a_File->FromRef || a_Chunk >= a_File->NumChunks)
		return;
	
	/* Chunk is outside of requested range */
	if (a_Chunk < a_File->RecvBase || a_Chunk >= a_File->RecvBase + MAXHOLDCHUNKS)
		return;
	
	// Get relative chunk
	RelChunk = a_Chunk - a_File->RecvBase;
	
	/* Already got chunk? */
	if (a_File->RecvGot[RelChunk])
		return;
		
	/* Re-allocate? */
	if (!ChunkBuf || !ChunkSize || a_File->ChunkSize > ChunkSize)
	{
		// Clear old
		if (ChunkBuf)
			Z_Free(ChunkBuf);
		if (LZOBuf)
			Z_Free(LZOBuf);
		
		// Allocate
		ChunkSize = a_File->ChunkSize;
		ChunkBuf = Z_Malloc(ChunkSize, PU_STATIC, NULL);
		LZOBuf = Z_Malloc(ChunkSize + 256, PU_STATIC, NULL);
	}
	
	/* Clear buffer */
	memset(ChunkBuf, 0, ChunkSize);
	memset(LZOBuf, 0, ChunkSize + 256);
	
	/* Load LZO data from buffer */
	D_BSReadChunk(a_BS, LZOBuf, ChunkSize + 256);
	LZOSize = ChunkSize;
	
	/* Decompress */
	lzo1x_decompress(
			LZOBuf, ChunkSize + 256,
			ChunkBuf, &LZOSize,
			NULL
		);
	
	/* Place into chunk buffer */
	memmove(&a_File->RecvHold[RelChunk * a_File->ChunkSize], ChunkBuf, a_File->ChunkSize);
	a_File->RecvGot[RelChunk] = true;
}

/* D_XFPrepFile() -- Prepare file for sending */
bool_t D_XFPrepFile(const char* const a_File, int32_t* const a_FileRef)
{
#define SUMBUF 4096
	static uint8_t* l_SumBuf;
	D_XFile_t* New;
	I_File_t* File;
	int32_t i;
	MD5_CTX MD5Sum;
	uint64_t n;
	uint8_t RawSum[16];
	
	/* Check */
	if (!a_File || !a_FileRef)
		return false;
	
	/* See if it exists first */
	if (!I_CheckFileAccess(a_File, false))
		return false;
	
	/* Try opening file */
	File = I_FileOpen(a_File, IFM_READ);
	
	// Failed?
	if (!File)
		return false;
	
	/* Setup file reference */
	New = DS_BlankFile();
	
	// Init
	*a_FileRef = New->Ref;
	New->RefPtr = a_FileRef;
	New->File = File;
	strncpy(New->Path, a_File, PATH_MAX - 1);
	
	// Determine size of file
	I_FileSeek(File, 0, true);
	New->Size = I_FileTell(File);
	New->ChunkSize = CHUNKSIZE;
	New->NumChunks = (New->Size / New->ChunkSize) + 1;
	I_FileSeek(File, 0, false);
	
	/* Calculate MD5 */
	// Init MD5
	memset(&MD5Sum, 0, sizeof(MD5Sum));
	MD5_Init(&MD5Sum);
	
	// MD5
	for (i = 0; i < New->Size; i += SUMBUF)
	{
		// Read bytes
		n = I_FileRead(File, l_SumBuf, SUMBUF);
		
		// MD5
		MD5_Update(&MD5Sum, l_SumBuf, n);
	}
	
	// Finalize MD5
	memset(RawSum, 0, sizeof(RawSum));
	MD5_Final(RawSum, &MD5Sum);
	
	// Setup characters
	for (i = 0; i < 16; i++)
		New->CheckSum[i >> 2] |= ((uint32_t)RawSum[i]) << (((3 - i) & 3) << 3);
	
	snprintf(New->CheckSumChars, 33, "%08x%08x%08x%08x", New->CheckSum[0], New->CheckSum[1], New->CheckSum[2], New->CheckSum[3]);
	
	/* Check WAD blacklist */
	if (D_CheckWADBlacklist(New->CheckSumChars))
	{
		DS_XFDeleteFile(New);
		return false;
	}
	
	/* Success! */
	return true;
#undef SUMBUF
}

/* D_XFSendFile() -- Send file to somewhere */
bool_t D_XFSendFile(const int32_t a_FileRef, I_HostAddress_t* const a_Addr, D_BS_t* const a_RelBS, D_BS_t* const a_StdBS)
{
#define BUFSIZE 64
	char Buf[BUFSIZE];
	D_XFile_t* File;
	
	/* Check */
	if (!a_FileRef || !a_Addr || !a_RelBS || !a_StdBS)
		return false;
	
	/* Locate file by ref */
	File = DS_FindByRef(a_FileRef, false);
	
	// Not found?
	if (!File)
		return false;
	
	/* Check Blacklist */
	if (D_CheckWADBlacklist(File->CheckSumChars))
		return false;
	
	/* Build Packet */
	D_BSBaseBlock(a_RelBS, "FILE");
	
	// Encode information
	D_BSwu8(a_RelBS, 'u');
	D_BSws(a_RelBS, File->CheckSumChars);
	D_BSwi32(a_RelBS, File->Ref);
	D_BSws(a_RelBS, WL_BaseNameEx(File->Path));
	D_BSwu32(a_RelBS, File->Size);
	D_BSwu32(a_RelBS, File->ChunkSize);
	D_BSwu32(a_RelBS, File->NumChunks);
	
	// Send away
	D_BSRecordNetBlock(a_RelBS, a_Addr);
	
	/* Message */
	I_NetHostToString(a_Addr, Buf, BUFSIZE - 1);
	CONL_OutputUT(CT_NETWORK, DSTR_DXP_SENDFILE, "%s%s\n", File->Path, Buf);
	
	/* Set output */
	File->SendRelBS = a_RelBS;
	File->SendStdBS = a_StdBS;
	
	/* Presume it worked */
	return true;
#undef BUFSIZE
}

/* D_XFStopTransferRef() -- Stop reference transfers */
void D_XFStopTransferRef(const int32_t a_FileRef)
{
	DS_XFDeleteFile(DS_FindByRef(a_FileRef, false));
}

/* D_XFStopTransferHost() -- Stop host transfers */
void D_XFStopTransferHost(I_HostAddress_t* const a_Addr)
{
}

/* D_XFStopAll() -- Stop all transfers */
void D_XFStopAll(void)
{
	size_t i;
	
	/* Go through all */
	for (i = 0; i < l_NumXF; i++)
		if (l_XF[i])
			DS_XFDeleteFile(l_XF[i]);
}

/* D_XFFilePacket() -- Handle file packet */
bool_t D_XFFilePacket(D_XDesc_t* const a_Desc, const char* const a_Header, const uint32_t a_Flags, I_HostAddress_t* const a_Addr, D_XEndPoint_t* const a_EP)
{
#define NAMESIZE 64
	char CheckSum[33];
	char Name[NAMESIZE];
	uint8_t Ctrl;
	D_XFile_t* XFile;
	int32_t Ref, i;
	uint32_t Chunk, Size;
	
	/* Read control code */
	Ctrl = D_BSru8(a_Desc->RelBS);
	
	/* Handle based on control code */
	switch (Ctrl)
	{
			// File being uploaded to us
		case 'u':
			// Read Checksum
			D_BSrs(a_Desc->RelBS, CheckSum, 33);
			
			// See if sum is valid
			if (D_CheckWADBlacklist(CheckSum))
				return false;
			
			// Read Remote Ref and Name
			Ref = D_BSri32(a_Desc->RelBS);
			D_BSrs(a_Desc->RelBS, Name, NAMESIZE);
			
			// Only accept certain file extentions
			if (!WL_ValidExt(Name))
				return false;
			
			// See if file already being downloaded
			XFile = DS_FindByRef(Ref, true);
			
			if (XFile)
				return false;
			
			// Otherwise build file
			XFile = DS_BlankFile();
			
			XFile->FromRef = Ref;
			memmove(&XFile->FromAddr, a_Addr, sizeof(XFile->FromAddr));
			I_GetStorageDir(XFile->Path, PATH_MAX - 1, DST_DATA);
			strncat(XFile->Path, "/", PATH_MAX - 1);
			strncat(XFile->Path, WL_BaseNameEx(Name), PATH_MAX - 1);
			
			XFile->Size = D_BSru32(a_Desc->RelBS);
			XFile->ChunkSize = D_BSru32(a_Desc->RelBS);
			XFile->NumChunks = D_BSru32(a_Desc->RelBS);
			
			XFile->RecvDesc = a_Desc;
			
			// Invalid?
			if (!XFile->ChunkSize)
				XFile->ChunkSize = 1;
			if (!XFile->NumChunks)
				XFile->NumChunks = 1;
			
			strncpy(XFile->CheckSumChars, CheckSum, 33);
			
			// Open File
			XFile->File = I_FileOpen(XFile->Path, IFM_RWT);
			
			// Failed?
			if (!XFile->File)
			{
				DS_XFDeleteFile(XFile);
				return false;
			}
			
			// Create hold buffer
			XFile->RecvHold = Z_Malloc(XFile->ChunkSize * MAXHOLDCHUNKS, PU_STATIC, NULL);
			
			// Set time recieved
			XFile->StartTime = g_ProgramTic;
			
			// Message
			I_NetHostToString(a_Addr, Name, NAMESIZE - 1);
			CONL_OutputUT(CT_NETWORK, DSTR_DXP_RECVFILE, "%s%s\n", XFile->Path, Name);
			return true;
			
			// Request a file to download (WADs)
		case 'r':
			return true;
			
			// File Chunk
		case 'c':
			// Which chunk for which file was recieved?
			Ref = D_BSri32(a_Desc->RelBS);
			Chunk = D_BSru32(a_Desc->RelBS);
			Size = D_BSru32(a_Desc->RelBS);
			XFile = DS_FindByRef(Ref, true);
			
			// No file?
			if (!XFile)
				return false;
			
			// Info
			DS_XFReadChunk(XFile, a_Desc->RelBS, Chunk, Size);
			return true;
			
			// Want chunk(s)
		case 'w':
			// Find which file they want
			Ref = D_BSri32(a_Desc->RelBS);
			XFile = DS_FindByRef(Ref, false);
			
			// Not found?
			if (!XFile)
				return false;
			
			// File is being sent remote?
			if (XFile->FromRef)
				return false;
			
			// Read chunks they want
			for (i = 0; i < MAXHOLDCHUNKS; i++)
			{
				Chunk = D_BSru32(a_Desc->RelBS);
				
				// End?
				if (Chunk == UINT32_C(0xFFFFFFFF))
					break;
				
				// Send chunk to them
				DS_SendChunk(XFile, a_Addr, a_Desc->StdBS, Chunk);
			}
			return true;
			
			// Stopped Transfer
		case 'x':
			return true;
					
			// Un-handled
		default:
			return false;
	}

#undef NAMESIZE
}

/* D_XFHandleFiles() -- Handles file transfers */
void D_XFHandleFiles(void)
{
	size_t i, c;
	D_XFile_t* XFile, Clone;
	bool_t Built;
	
	/* Go through files and handle downloading */
	for (i = 0; i < l_NumXF; i++)
	{
		XFile = l_XF[i];
		
		// No file here?
		if (!XFile)
			continue;
		
		// Not downloading?
		if (!XFile->FromRef)
			continue;
			
		// Clear built
		Built = false;
		
		// Request more chunks?
		if (XFile->RecvBase < XFile->NumChunks && g_ProgramTic >= XFile->RecvLastReq)
		{
			// Go through chunks waiting
			for (c = 0; c < MAXHOLDCHUNKS; c++)
			{
				// Already got? More than we need?
				if (XFile->RecvGot[c] || (c + XFile->RecvBase) >= XFile->NumChunks)
					continue;
				
				// Need to build header?
				if (!Built)
				{
					D_BSBaseBlock(XFile->RecvDesc->StdBS, "FILE");
					D_BSwu8(XFile->RecvDesc->StdBS, 'w');
					D_BSwi32(XFile->RecvDesc->StdBS, XFile->FromRef);
					Built = true;
				}
				
				// Put in want list
				D_BSwu32(XFile->RecvDesc->StdBS, c + XFile->RecvBase);
			}
			
			// Send away?
			if (Built)
			{
				D_BSwu32(XFile->RecvDesc->StdBS, UINT32_C(0xFFFFFFFF));
				D_BSRecordNetBlock(XFile->RecvDesc->StdBS, &XFile->FromAddr);
			}
			
			// Request more in the future
			XFile->RecvLastReq = g_ProgramTic + REQUESTDELAY;
		}
		
		// Place chunks into file?
		while (XFile->RecvGot[0])
		{
			// Write chunk to file
			if (XFile->RecvBase >= XFile->NumChunks - 1)
				I_FileWrite(XFile->File, XFile->RecvHold, XFile->Size % XFile->ChunkSize);
			else
				I_FileWrite(XFile->File, XFile->RecvHold, XFile->ChunkSize);
			
			// Move everything over
			XFile->RecvBase++;
			memmove(XFile->RecvHold, XFile->RecvHold + XFile->ChunkSize, XFile->ChunkSize * (MAXHOLDCHUNKS - 1));
			memmove(XFile->RecvGot, XFile->RecvGot + 1, sizeof(bool_t) * (MAXHOLDCHUNKS - 1));
			
			// Clear last spot
			memset(&XFile->RecvHold[XFile->ChunkSize * (MAXHOLDCHUNKS - 1)], 0, XFile->ChunkSize);
			XFile->RecvGot[MAXHOLDCHUNKS - 1] = false;
		}
		
		// Got file completely
		if (XFile->RecvBase >= XFile->NumChunks)
		{
			// Set time ended
			XFile->EndTime = g_ProgramTic;
			
			// Clone before deletion
			memmove(&Clone, XFile, sizeof(Clone));
			
			// Now Delete
			for (c = 0; c < l_NumXF; c++)
				if (l_XF[c] == XFile)
					DS_XFDeleteFile(XFile);
					
			// Handle receive of a file (load save, continue WAD, etc.)
			D_XPGotFile(Clone.RecvDesc, Clone.Path, Clone.CheckSumChars, Clone.Size, &Clone.FromAddr, Clone.StartTime, Clone.EndTime);
		}
	}
}

