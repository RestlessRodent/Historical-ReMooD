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

/****************
*** CONSTANTS ***
****************/

#define CHUNKSIZE 1024							// Size of chunks

/*****************
*** STRUCTURES ***
*****************/

/* D_XFile_t -- XFile */
typedef struct D_XFile_s
{
	int32_t Ref;								// Reference
	int32_t* RefPtr;							// Pointer to reference holder
	
	bool_t SendAway;							// File being sent away
	I_File_t* File;								// File pointer
	char Path[PATH_MAX];						// Path to file
	uint32_t Size;								// Size of files
	uint32_t NumChunks;							// Number of chunks
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

/* DS_FindByRef() -- Find existing file by reference */
static D_XFile_t* DS_FindByRef(const int32_t a_Ref)
{
	int32_t i;
	
	/* Check */
	if (!a_Ref)
		return NULL;
	
	/* Look in list */
	for (i = 0; i < l_NumXF; i++)
		if (l_XF[i])
			if (l_XF[i]->Ref == a_Ref)
				return l_XF[i];
	
	/* Not found */
	return NULL;
}

/* D_XFPrepFile() -- Prepare file for sending */
bool_t D_XFPrepFile(const char* const a_File, int32_t* const a_FileRef)
{
	D_XFile_t* New;
	I_File_t* File;
	int32_t i;
	
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
		return;
	
	/* Setup file reference */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	// Init
	New->SendAway = true;
	*a_FileRef = New->Ref = ++l_XFLastRef;
	New->RefPtr = a_FileRef;
	New->File = File;
	strncpy(New->Path, a_File, PATH_MAX - 1);
	
	// Determine size of file
	I_FileSeek(File, 0, true);
	New->Size = I_FileTell(File);
	New->NumChunks = (New->Size / CHUNKSIZE) + 1;
	
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
	
	/* Success! */
	return true;
}

/* D_XFSendFile() -- Send file to somewhere */
bool_t D_XFSendFile(const int32_t a_FileRef, I_HostAddress_t* const a_Addr, D_BS_t* const a_RelBS, D_BS_t* const a_StdBS)
{
	D_XFile_t* File;
	
	/* Check */
	if (!a_FileRef || !a_Addr || !a_RelBS || !a_StdBS)
		return false;
	
	/* Locate file by ref */
	File = DS_FindByRef(a_FileRef);
	
	// Not found?
	if (!File)
		return false;
	
	/* Presume it worked */
	return true;
}

