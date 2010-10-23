// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.org/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           TetrisMaster512        (tetrismaster512@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 2008 ReMooD Team.
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION: ReMooD Demo Format

#include "d_rdf.h"
#include "byteptr.h"
#include "cmp_rle.h"
#include <stdio.h>
#include <stdlib.h>

#define DPF_C_MASK			7
#define DPF_C_OFFSET		0

#define DPF_C_RLE			1
#define DPF_C_RESERVEDA		2
#define DPF_C_RESERVEDB		3
#define DPF_C_RESERVEDC		4
#define DPF_C_RESERVEDD		5
#define DPF_C_RESERVEDE		6
#define DPF_C_RESERVEDF		7

typedef struct DEMOPack_s
{
	union
	{
		UInt32 Num;
		UInt8 Char[4];
	} IDs;
	
	UInt16 Flags;
	
	union
	{
		UInt16 ZSize[2];
		UInt32 Size;
	} Sizes;
} DEMOPack_t;

#define TOID(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))

typedef struct DEMOHeader_s
{	
	// Compiler Info
	UInt32 DeadBeef;
	UInt32 SizeShort;
	UInt32 SizeInt;
	UInt32 SizeLong;
	UInt32 SizeSizeT;
	UInt32 SizeFloat;
	UInt32 SizeDouble;
	UInt32 SizeEightS;
	UInt32 SizeSixteenS;
	UInt32 SizeThirtyS;
	UInt32 SizeSixtyS;
	Int8 Compiler[32];
	
	// Port Info
	UInt8 LegacyVersion;
	UInt8 Major;
	UInt8 Minor;
	UInt8 Release;
	Int8 CodeName[32];
	Int8 URL[32];
	
	// RDF Info
	Int8 RDFDate[32];
	Int8 RDFTime[32];
} DEMOHeader_t;

typedef struct EightStruct_s
{
	UInt8 a;
	UInt8 b;
} EightStruct_t;

typedef struct SixteenStruct_s
{
	UInt16 a;
	UInt16 b;
} SixteenStruct_t;

typedef struct ThirtyStruct_s
{
	UInt32 a;
	UInt32 b;
} ThirtyStruct_t;

typedef struct SixtyStruct_s
{
	UInt64 a;
	UInt64 b;
} SixtyStruct_t;

FILE* DemoFile = NULL;
DEMOPack_t PackStore;

/******************************************************************************/
/*********************************** GLOBAL ***********************************/
/******************************************************************************/

DemoStatus_t RDF_DEMO_PrepareRecording(char* name)
{
	char FullName[64];
	int i;
	
	if (DemoFile)
		RDF_DEMO_EndRecording();
		
	memset(FullName, 0, sizeof(FullName));
	snprintf(FullName, sizeof(FullName), "%s.rdf", name);
	
	// Attempt to open it
	DemoFile = fopen(FullName, "rb");
	i = 0;
	
	while (DemoFile)
	{
		fclose(DemoFile);
		DemoFile = NULL;
		memset(FullName, 0, sizeof(FullName));
		snprintf(FullName, sizeof(FullName), "%s-%03i.rdf", name, i);
		i++;
		DemoFile = fopen(FullName, "rb");
	}
	
	DemoFile = fopen(FullName, "wb");
	
	return DST_SUCCESS;
}

DemoStatus_t RDF_DEMO_StartRecording(void)
{
	DEMOHeader_t Header;
	
	if (!DemoFile)
		return DST_NOTRECORDING;
	
	/*** WRITE DATA ***/
	// Actual Data
	memset(&Header, 0, sizeof(Header));
	Header.LegacyVersion = VERSION;
	Header.Major = REMOOD_MAJORVERSION;
	Header.Minor = REMOOD_MINORVERSION;
	Header.Release = REMOOD_RELEASEVERSION;
	snprintf(Header.CodeName, 32, "%s", REMOOD_VERSIONCODESTRING);
	snprintf(Header.URL, 32, "%s", REMOOD_URL);
	snprintf(Header.RDFDate, 32, "%s", __DATE__);
	snprintf(Header.RDFTime, 32, "%s", __TIME__);
	
	// More compiler related
#if defined(_MSC_VER)
	snprintf(Header.Compiler, 32, "MSVC %i", _MSC_VER);
#elif defined(__GNUC__) || defined(__GNUG__) || defined(_GNUC_) || defined(_GNUG_)
	snprintf(Header.Compiler, 32, "GCC %i.%i", __GNUC__, __GNUC_MINOR__);
#else
	snprintf(Header.Compiler, 32, "Unknown");
#endif
	Header.DeadBeef = 0xDEADBEEF;
	Header.SizeShort = sizeof(short);
	Header.SizeInt = sizeof(int);
	Header.SizeLong = sizeof(long);
	Header.SizeSizeT = sizeof(size_t);
	Header.SizeFloat = sizeof(float);
	Header.SizeDouble = sizeof(double);
	Header.SizeEightS = sizeof(EightStruct_t);
	Header.SizeSixteenS = sizeof(SixteenStruct_t);	
	Header.SizeThirtyS = sizeof(ThirtyStruct_t);
	Header.SizeSixtyS = sizeof(SixtyStruct_t);
	
	// Demo Pack
	PackStore.IDs.Num = TOID('R', 'D', 'F', '8');
	PackStore.Flags = 0;
	PackStore.Sizes.Size = sizeof(DEMOHeader_t);
	
	// Stream Out
	fwrite(&PackStore, sizeof(PackStore), 1, DemoFile);
	fwrite(&Header, sizeof(Header), 1, DemoFile);
	
	return DST_SUCCESS;
}

DemoStatus_t RDF_DEMO_EndRecording(void)
{
	if (DemoFile)
	{
		fclose(DemoFile);
		return DST_SUCCESS;
	}
	
	return DST_NOTRECORDING;
}

/******************************************************************************/
/************************************ WRITE ***********************************/
/******************************************************************************/

/******************************************************************************/
/************************************* READ ***********************************/
/******************************************************************************/

