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
// DESCRIPTION: Network transmission

/***************
*** INCLUDES ***
***************/

#include "d_net.h"
#include "i_util.h"
#include "doomstat.h"
#include "d_main.h"
#include "console.h"
#include "p_info.h"
#include "p_saveg.h"
#include "md5.h"

/*****************
*** STRUCTURES ***
*****************/

#define CHUNKSIZE 		1024
#define CHUNKSATONCE	64						// Chunks at a time

typedef struct D_File_s D_File_t;

/* D_Bit_t -- Small bit of file */
typedef struct D_Bit_s
{
	D_File_t* File;								// File master
	uint32_t ID;								// Host ID
	tic_t InitReq;								// Initialization request
	bool_t WasInit;								// Was initialized
	uint32_t LowAck;							// Acknowledged at
	tic_t LastTime;								// Time time used
} D_Bit_t;

/* D_File_t -- Networked file */
struct D_File_s
{
	int32_t Handle;
	uint32_t Modes;
	I_File_t* File;
	char PathName[PATH_MAX];
	char* BaseName;
	uint32_t Size;
	uint32_t Chunks;
	
	D_Bit_t** Bits;								// Files for hosts
	int32_t NumBits;
	
	uint32_t CheckSum[4];						// MD5 Sum
	char CheckSumChars[33];						// MD5 Sum (As hex characters)
	
	uint8_t GetChunks[CHUNKSATONCE][CHUNKSIZE];	// Local storage chunks
	uint32_t AtChunk;							// Chunk currently at
};

/*************
*** LOCALS ***
*************/

static D_File_t** l_Files;
static int32_t l_NumFiles;

/****************
*** FUNCTIONS ***
****************/

/* D_SNFileByRef() -- Finds file by reference */
D_File_t* D_SNFileByRef(const int32_t a_Ref)
{
	int32_t i;
	
	/* Check */
	if (a_Ref < 0)
		return NULL;
	
	/* Search */
	for (i = 0; i < l_NumFiles; i++)
		if (l_Files[i])
			if (l_Files[i]->Handle == a_Ref)
				return l_Files[i];
	
	/* Not found */
	return NULL;
}

/* D_SNClearFiles() -- Clear files being sent/recv */
void D_SNClearFiles(void)
{
	int32_t i;
	
	/* Close all handles */
	for (i = 0; i < l_NumFiles; i++)
		if (l_Files[i])
			D_SNCloseFile(l_Files[i]->Handle);
}

/* D_SNCloseFile() -- Closes file */
void D_SNCloseFile(const int32_t a_Handle)
{
	D_File_t* File;
	int32_t i;
	
	/* Check */
	if (a_Handle < 0)
		return;
	
	/* Get and remove from list */
	for (i = 0; i < l_NumFiles; i++)
		if ((File = l_Files[i]))
			if (File->Handle == a_Handle)
			{
				l_Files[i] = NULL;
				break;
			}
		
	/* Close master file */
	I_FileClose(File->File);
	
	/* Delete */
	Z_Free(File);
}

/* D_SNPrepFile() -- Prepare sending file */
int32_t D_SNPrepFile(const char* const a_PathName, const uint32_t a_Modes)
{
#define SUMBUF 4096
	D_File_t Temp, *New;
	static int32_t RefCount;
	int32_t i;
	
	static uint8_t* l_SumBuf;
	MD5_CTX MD5Sum;
	uint64_t n;
	uint8_t RawSum[16];
	
	/* Check */
	if (!a_PathName)
		return -1;
	
	/* Clear */
	memset(&Temp, 0, sizeof(Temp));
	
	/* Try opening file */
	if (!(Temp.File = I_FileOpen(a_PathName, a_Modes)))
		return -1;
	
	/* Create new reference */
	do
	{
		Temp.Handle = ++RefCount;
	} while (D_SNFileByRef(Temp.Handle));
	Temp.Modes = a_Modes;
	
	strncpy(Temp.PathName, a_PathName, PATH_MAX - 1);
	Temp.BaseName = WL_BaseNameEx(Temp.PathName);
	
	/* Find size */
	I_FileSeek(Temp.File, 0, true);
	Temp.Size = I_FileTell(Temp.File);
	Temp.Chunks = (Temp.Size / CHUNKSIZE) + 1;
	I_FileSeek(Temp.File, 0, false);
	
	/* Checksum (MD5) */
	// Input buffer
	if (!l_SumBuf)
		l_SumBuf = Z_Malloc(sizeof(*l_SumBuf) * SUMBUF, PU_STATIC, NULL);
	
	// Init MD5
	memset(&MD5Sum, 0, sizeof(MD5Sum));
	MD5_Init(&MD5Sum);
	
	// MD5
	for (i = 0; i < Temp.Size; i += SUMBUF)
	{
		// Read bytes
		n = I_FileRead(Temp.File, l_SumBuf, SUMBUF);
		
		// MD5
		MD5_Update(&MD5Sum, l_SumBuf, n);
	}
	
	// Finalize MD5
	memset(RawSum, 0, sizeof(RawSum));
	MD5_Final(RawSum, &MD5Sum);
	
	// Setup characters
	for (i = 0; i < 16; i++)
		Temp.CheckSum[i >> 2] |= ((uint32_t)RawSum[i]) << (((3 - i) & 3) << 3);
	
	snprintf(Temp.CheckSumChars, 33, "%08x%08x%08x%08x", Temp.CheckSum[0], Temp.CheckSum[1], Temp.CheckSum[2], Temp.CheckSum[3]);
	
	// Back to start
	I_FileSeek(Temp.File, 0, false);
	
	/* Link in */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	memmove(New, &Temp, sizeof(*New));
	
	// Find free spot
	for (i = 0; i < l_NumFiles; i++)
		if (!l_Files[i])
			break;
	
	// No room?
	if (i >= l_NumFiles)
	{
		Z_ResizeArray((void**)&l_Files, sizeof(*l_Files), l_NumFiles, l_NumFiles + 1);
		i = l_NumFiles++;
	}
	
	// Set here
	l_Files[i] = New;
	
	/* Failed */
	return New->Handle;
#undef SUMBUF
}

/* D_SNPrepSave() -- Prepare save game */
int32_t D_SNPrepSave(void)
{
#define BUFSIZE 128
#define MINISIZE 16
	char Buf[BUFSIZE];
	char Mini[MINISIZE];
	
	/* Prepare name */
	memset(Buf, 0, sizeof(Buf));
	memset(Mini, 0, sizeof(Mini));
	I_GetStorageDir(Buf, BUFSIZE - 24, DST_TEMP);
	strncat(Buf, "/", BUFSIZE);
	snprintf(Mini, MINISIZE - 1, "netg%04x.rsv", I_GetCurrentPID() & 0xFFFF);
	strncat(Buf, Mini, BUFSIZE);
	
	/* Save to this place */
	if (!P_SaveGameEx("Network Save", Buf, strlen(Buf), NULL, NULL))
		return -1;
	
	/* Prepare file for transfer */
	return D_SNPrepFile(Buf, IFM_READ);
#undef BUFSIZE
#undef MINISIZE
}

/* D_SNSendFile() -- Send file to remote side */
void D_SNSendFile(const int32_t a_Handle, D_SNHost_t* const a_Host)
{
#define BUFSIZE 128
	char Buf[BUFSIZE];
	D_File_t* File;
	D_Bit_t* Bit;
	int32_t i;
	
	/* Check */
	if (a_Handle < 0 || !a_Host)
		return;
	
	/* Find file */
	if (!(File = D_SNFileByRef(a_Handle)))
		return;
	
	/* See if sending file to this host already */
	for (i = 0; i < File->NumBits; i++)
		if ((Bit = File->Bits[i]))
			if (Bit->ID == a_Host->ID)
				return;
	
	/* Otherwise, setup a new bit */
	Bit = Z_Malloc(sizeof(*Bit), PU_STATIC, NULL);
	
	// Add to file
	for (i = 0; i < File->NumBits; i++)
		if (!File->Bits[i])
			break;
	
	if (i >= File->NumBits)
	{
		Z_ResizeArray((void**)&File->Bits, sizeof(*File->Bits),
			File->NumBits, File->NumBits + 1);
		i = File->NumBits++;
	}
	
	File->Bits[i] = Bit;
	
	// Fill
	Bit->File = File;
	Bit->ID = a_Host->ID;
	
	// Message
	memset(Buf, 0, sizeof(Buf));
	I_NetHostToString(&a_Host->Addr, Buf, BUFSIZE);
	CONL_OutputUT(CT_NETWORK, DSTR_DXP_SENDFILE, "%s%s\n",
		File->BaseName, Buf);
#undef BUFSIZE
}

/* D_SNMakeFileValid() -- Makes legal filename */
void D_SNMakeFileValid(char* const a_Name)
{
	char* c;
	
	/* Loop */
	for (c = a_Name; *c; c++)
		if (!((*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z') ||
			(*c >= '0' && *c <= '9') || *c == '.' || *c == '_' ||
			*c == '-'
#if defined(__MS_DOS__) || defined(__DOS__)	// do not make spaces in DOS!
			|| *c == ' '
#endif
			))
			*c = '_';
}

/* D_SNFileInit() -- Initialize sending of file */
void D_SNFileInit(D_BS_t* const a_BS, D_SNHost_t* const a_Host, I_HostAddress_t* const a_Addr)
{
#define BUFSIZE PATH_MAX
#define MINISIZE PATH_MAX
	char Buf[BUFSIZE];
	char Mini[MINISIZE];
	D_File_t Temp, *New;
	int32_t i;
	
	/* Client Only */
	if (D_SNIsServer())
		return;
	
	/* Init */
	memset(&Temp, 0, sizeof(Temp));
	
	/* Read packet data */
	Temp.Handle = D_BSri32(a_BS);
	
	// Already used or bad?
	if (Temp.Handle < 0)
		return;
		
	if (!(New = D_SNFileByRef(Temp.Handle)))
	{
		// Load size and other things
		Temp.Size = D_BSru32(a_BS);
		Temp.Chunks = D_BSru32(a_BS);
		
		memset(Mini, 0, sizeof(Mini));
		D_BSrs(a_BS, Mini, PATH_MAX - 1);
		
		Temp.BaseName = WL_BaseNameEx(Mini);
		D_BSrs(a_BS, Temp.CheckSumChars, 33);
		
		// Validate name
		D_SNMakeFileValid(Temp.BaseName);
		
		CONL_PrintF("%s\n", Temp.BaseName);
		
		// Check extension
		if (!WL_ValidExt(Temp.BaseName))
			return;
			
		// Name to save to
		memset(Buf, 0, sizeof(Buf));
		I_GetStorageDir(Buf, BUFSIZE - 24, DST_TEMP);
		strncat(Buf, "/", BUFSIZE);
		strncat(Buf, Temp.BaseName, BUFSIZE);
		
		// Correct base
		strncpy(Temp.PathName, Buf, PATH_MAX - 1);
		Temp.BaseName = WL_BaseNameEx(Temp.PathName);
		
		// Message
		CONL_OutputUT(CT_NETWORK, DSTR_DXP_RECVFILE, "%s%s\n",
			Temp.BaseName, Temp.PathName);
		
		// Open File
		Temp.Modes = IFM_WRITE | IFM_TRUNCATE;
		if (!(Temp.File = I_FileOpen(Temp.PathName, Temp.Modes)))
			return;
	
		// Clone 
		New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
		memmove(New, &Temp, sizeof(*New));
	
		// Add to local files
		for (i = 0; i < l_NumFiles; i++)
			if (!l_Files[i])
				break;
	
		// No room?
		if (i >= l_NumFiles)
		{
			Z_ResizeArray((void**)&l_Files, sizeof(*l_Files), l_NumFiles, l_NumFiles + 1);
			i = l_NumFiles++;
		}
	
		// Set here
		l_Files[i] = New;
	}
	
	/* No file created? */
	if (!New)
		return;
	
	/* Acknowlegde to server */
	D_BSBaseBlock(a_BS, "FRDY");
	D_BSwi32(a_BS, New->Handle);
	D_BSRecordNetBlock(a_BS, a_Addr);
#undef BUFSIZE
#undef MINISIZE
}

/* D_SNFileReady() -- File is ready to transmit */
void D_SNFileReady(D_BS_t* const a_BS, D_SNHost_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	D_File_t* File;
	D_Bit_t* Bit;
	int32_t Handle, i;
	
	/* Server only */
	if (!D_SNIsServer() || !a_Host || a_Host->Local)
		return;
	
	/* Find reference file */
	Handle = D_BSri32(a_BS);
	
	if (Handle < 0 || !(File = D_SNFileByRef(Handle)))
		return;
	
	/* Make sure this host has a bit */
	for (i = 0; i < File->NumBits; i++)
	{
		if ((Bit = File->Bits[i]))
			if (Bit->ID == a_Host->ID)
				break;
		
		// Clear bit
		Bit = NULL;
	}
	
	// No host was sent this request
	if (!Bit)
		return;
	
	/* Set as ready */
	Bit->WasInit = true;
}

/* D_SNFileRecv() -- Received file piece */
void D_SNFileRecv(D_BS_t* const a_BS, D_SNHost_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	/* Client Only */
	if (D_SNIsServer())
		return;
}

/* D_SNChunkReq() -- Request Chunk */
void D_SNChunkReq(D_BS_t* const a_BS, D_SNHost_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	/* Server Only */
	if (!D_SNIsServer() || !a_Host)
		return;
}

/* D_SNKillBit() -- Kills bit */
void D_SNKillBit(D_Bit_t* const a_Bit)
{
	D_File_t* File;
	int32_t k;
	
	/* Check */
	if (!a_Bit)
		return;
	
	/* Get file */
	File = a_Bit->File;
	
	/* Remove from top */
	for (k = 0; k < File->NumBits; k++)
		if (File->Bits[k] == a_Bit)
		{
			File->Bits[k] = NULL;
			break;
		}
	
	/* Free */
	Z_Free(a_Bit);
}

/* D_SNFileLoop() -- File send/recv loop */
void D_SNFileLoop(void)
{
	int32_t i, j, k, BitCount;
	D_File_t* File;
	D_Bit_t* Bit;
	D_SNHost_t* Host;
	D_BS_t* BS;
	
	/* Go through files */
	for (i = 0; i < l_NumFiles; i++)
	{
		if (!(File = l_Files[i]))
			continue;
		
		// Sending file
		if (File->Modes & IFM_READ)
		{
			// Perform individual transfers
			for (BitCount = 0, j = 0; j < File->NumBits; j++)
			{
				if (!(Bit = File->Bits[j]))
					continue;
			
				// If host is dead, destroy transfer
				if (!(Host = D_SNHostByID(Bit->ID)))
				{
					D_SNKillBit(Bit);
					continue;
				}
		
				// Get BS
				BS = Host->BS;
		
				// Add to count
				BitCount++;
				
				// If not initialized, initialize
				if (!Bit->WasInit)
				{
					// Time delay
					if (g_ProgramTic >= Bit->InitReq)
					{
						Bit->InitReq = g_ProgramTic + (TICRATE >> 1);
				
						D_BSBaseBlock(BS, "FOPN");
				
						D_BSwi32(BS, File->Handle);
						D_BSwu32(BS, File->Size);
						D_BSwu32(BS, File->Chunks);
						D_BSws(BS, File->BaseName);
						D_BSws(BS, File->CheckSumChars);
				
						D_BSRecordNetBlock(BS, &Host->Addr);
					}
			
					continue;
				}
				
			}
		
			// No more bits
			if (!BitCount)
				D_SNCloseFile(File->Handle);
		}
		
		// Receiving file
		else
		{
		}
	}
}

