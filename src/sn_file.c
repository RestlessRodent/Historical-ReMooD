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

#include "sn.h"
#include "md5.h"
//#include "i_util.h"
//#include "doomstat.h"
//#include "d_main.h"
//#include "console.h"
//#include "p_info.h"
//#include "p_saveg.h"


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
	
	struct
	{
		bool_t OK;								// Chunk is OK
		uint8_t Data[CHUNKSIZE];				// Local storage chunks
	} GetChunk[CHUNKSATONCE];
	uint32_t AtChunk;							// Chunk currently at
	tic_t ChunkTime;							// Time of last chunk
	
	uint32_t TotalRead;							// Total bytes read
	uint32_t BytesRead;							// Bytes Read
	tic_t LastRate;								// Last Rate
};

/*************
*** LOCALS ***
*************/

static D_File_t** l_Files;
static int32_t l_NumFiles;

/****************
*** FUNCTIONS ***
****************/

/* SN_FileByRef() -- Finds file by reference */
D_File_t* SN_FileByRef(const int32_t a_Ref)
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

/* SN_ClearFiles() -- Clear files being sent/recv */
void SN_ClearFiles(void)
{
	int32_t i;
	
	/* Close all handles */
	for (i = 0; i < l_NumFiles; i++)
		if (l_Files[i])
			SN_CloseFile(l_Files[i]->Handle);
}

/* SN_CloseFile() -- Closes file */
void SN_CloseFile(const int32_t a_Handle)
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

/* SN_PrepFile() -- Prepare sending file */
int32_t SN_PrepFile(const char* const a_PathName, const uint32_t a_Modes)
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
	} while (SN_FileByRef(Temp.Handle));
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

/* SN_PrepSave() -- Prepare save game */
int32_t SN_PrepSave(void)
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
	return SN_PrepFile(Buf, IFM_READ);
#undef BUFSIZE
#undef MINISIZE
}

/* SN_SendFile() -- Send file to remote side */
void SN_SendFile(const int32_t a_Handle, SN_Host_t* const a_Host)
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
	if (!(File = SN_FileByRef(a_Handle)))
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

/* SN_MakeFileValid() -- Makes legal filename */
void SN_MakeFileValid(char* const a_Name)
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

/* SN_FileInit() -- Initialize sending of file */
void SN_FileInit(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
#define BUFSIZE PATH_MAX
#define MINISIZE PATH_MAX
	char Buf[BUFSIZE];
	char Mini[MINISIZE];
	D_File_t Temp, *New;
	int32_t i;
	char* x;
	
	/* Client Only */
	if (SN_IsServer())
		return;
	
	/* Init */
	memset(&Temp, 0, sizeof(Temp));
	
	/* Read packet data */
	Temp.Handle = D_BSri32(a_BS);
	
	// Already used or bad?
	if (Temp.Handle < 0)
		return;
		
	if (!(New = SN_FileByRef(Temp.Handle)))
	{
		// Load size and other things
		Temp.Size = D_BSru32(a_BS);
		Temp.Chunks = D_BSru32(a_BS);
		
		memset(Mini, 0, sizeof(Mini));
		D_BSrs(a_BS, Mini, PATH_MAX - 1);
		
		Temp.BaseName = WL_BaseNameEx(Mini);
		D_BSrs(a_BS, Temp.CheckSumChars, 33);
		
		// Validate name
		SN_MakeFileValid(Temp.BaseName);
		
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
		
		// If file exists, rename (xyz rename)
		x = Temp.BaseName;
		while (I_CheckFileAccess(Temp.PathName, false, NULL))
		{
			// No more names
			if (!*x || *x == '.')
				return;
			
			// Change current character to x
			if (*x != 'x' && *x != 'y' && *x != 'z')
				*(x) = 'x';
			else if (*x == 'x')
				*(x) = 'y';
			else if (*x == 'y')
				*(x++) = 'z';
		}
		
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

/* SN_FileReady() -- File is ready to transmit */
void SN_FileReady(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	D_File_t* File;
	D_Bit_t* Bit;
	int32_t Handle, i;
	
	/* Server only */
	if (!SN_IsServer() || !a_Host || a_Host->Local)
		return;
	
	/* Find reference file */
	Handle = D_BSri32(a_BS);
	
	if (Handle < 0 || !(File = SN_FileByRef(Handle)))
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

/* SN_FileRecv() -- Received file piece */
void SN_FileRecv(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	D_File_t* File;
	int32_t Handle;
	uint32_t Chunk;
	
	/* Client Only */
	if (SN_IsServer())
		return;
	
	/* Read input packet */
	Handle = D_BSri32(a_BS);
	
	// Get file
	if (Handle < 0 || !(File = SN_FileByRef(Handle)))
		return;
	
	// Get chunk
	Chunk = D_BSru32(a_BS);
	
	// Passed this chunk
	if (Chunk < File->AtChunk)
		return;
	
	// Base chunk
	Chunk -= File->AtChunk;
	
	// Out of range or have it already
	if (Chunk >= CHUNKSATONCE || File->GetChunk[Chunk].OK)
		return;
	
	/* Put chunk here */
	File->GetChunk[Chunk].OK = true;
	D_BSReadChunk(a_BS, File->GetChunk[Chunk].Data, CHUNKSIZE);
	
	if (Chunk == 0)
		File->ChunkTime = g_ProgramTic;
}

/* SN_ChunkReq() -- Request Chunk */
void SN_ChunkReq(D_BS_t* const a_BS, SN_Host_t* const a_Host, I_HostAddress_t* const a_Addr)
{
	/* Server Only */
	if (!SN_IsServer() || !a_Host)
		return;
}

/* SN_KillBit() -- Kills bit */
void SN_KillBit(D_Bit_t* const a_Bit)
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

/* SN_DoSendPiece() --  Do sending of a piece of a file */
void SN_DoSendPiece(D_BS_t* const a_BS, SN_Host_t* const a_Host, D_File_t* const a_File, D_Bit_t* const a_Bit)
{
	uint8_t InChunk[CHUNKSIZE];	
	
	/* Do not send any pieces if not enough time has passed */
	if (g_ProgramTic <= a_Bit->LastTime)
		return;
	
	/* Ack is above file size */
	if (a_Bit->LowAck > a_File->Chunks)
		return;
	
	/* Seek to chunk in file and read that */
	I_FileSeek(a_File->File, CHUNKSIZE * a_Bit->LowAck, false);
	I_FileRead(a_File->File, InChunk, CHUNKSIZE);
	
	// Compress with LZW
		// maybe later
	
	/* Build packet to send */
	D_BSBaseBlock(a_BS, "FPUT");
	
	D_BSwi32(a_BS, a_File->Handle);
	D_BSwu32(a_BS, a_Bit->LowAck);
	
	D_BSWriteChunk(a_BS, InChunk, CHUNKSIZE);
	
	D_BSRecordNetBlock(a_BS, &a_Host->Addr);
	
	/* Go to send the next block */
	a_Bit->LowAck++;
}

/* SN_SquashChunks() -- Squashes chunks together */
void SN_SquashChunks(D_BS_t* const a_BS, D_File_t* const a_File)
{
	uint32_t LogicalPos;
	uint32_t WriteSize;
	bool_t Delete;
	char* Temp;
	
	/* While the first chunk is OK */
	while (a_File->GetChunk[0].OK)
	{
		// Determine amount to write
		LogicalPos = CHUNKSIZE * a_File->AtChunk;
		
		if (LogicalPos >= a_File->Size)
			break;	// chunk after EOF
		else if (LogicalPos + CHUNKSIZE >= a_File->Size)
			WriteSize = a_File->Size - LogicalPos;
		else
			WriteSize = CHUNKSIZE;
		
		// Write to file
		I_FileWrite(a_File->File, a_File->GetChunk[0].Data, WriteSize);
		a_File->BytesRead += WriteSize;
		a_File->TotalRead += WriteSize;
		
		// Move all other chunks down
		memmove(&a_File->GetChunk[0], &a_File->GetChunk[1], sizeof(a_File->GetChunk[0]) * (CHUNKSATONCE - 1));
		
		// Clear last chunk
		memset(&a_File->GetChunk[CHUNKSATONCE - 1], 0, sizeof(a_File->GetChunk[0]));
		
		// Reset time
		a_File->ChunkTime = g_ProgramTic;
		
		// Work on next chunk
		a_File->AtChunk++;
	}
	
	/* Show download ticker */
	if (g_ProgramTic >= a_File->LastRate)
	{
		CONL_PrintF("%i KiB/s (%u KiB of %u KiB)\n",
			(a_File->BytesRead >> 10),
			a_File->TotalRead,
			a_File->Size
			);
		a_File->LastRate = g_ProgramTic + TICRATE;
		a_File->BytesRead = 0;
	}
	
	/* Got the file completely */
	if (a_File->TotalRead >= a_File->Size)
	{
		CONL_PrintF("Download complete!\n");
		
		// Handle file
		Temp = Z_StrDup(a_File->PathName, PU_STATIC, NULL);
		SN_CloseFile(a_File->Handle);	// Do not need file handle anymore
		Delete = SN_GotFile(Temp);
		
		// Delete file?
		if (Delete)
			I_FileDeletePath(Temp);
		
		// Clear temp buffer
		Z_Free(Temp);
	}
}

/* SN_FileLoop() -- File send/recv loop */
void SN_FileLoop(void)
{
	int32_t i, j, k, BitCount;
	D_File_t* File;
	D_Bit_t* Bit;
	SN_Host_t* Host;
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
				if (!(Host = SN_HostByID(Bit->ID)))
				{
					SN_KillBit(Bit);
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
				
				// Send pieces of files
				SN_DoSendPiece(BS, Host, File, Bit);
			}
		
			// No more bits
			if (!BitCount)
				SN_CloseFile(File->Handle);
		}
		
		// Receiving file
		else
		{
			SN_SquashChunks(BS, File);
		}
	}
}

