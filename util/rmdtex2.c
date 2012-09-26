// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
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
// Copyright (C) 2012 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: ReMooD `deutex` Clone, for what ReMooD uses and bonus stuff

/***************
*** INCLUDES ***
***************/

/* Standard Includes Everyone Has */

/***************
*** INCLUDES ***
***************/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>

/* C99 Complaint Compilers */
#if (__STDC_VERSION__ >= 199901L) || defined(__GNUC__) || defined(__WATCOMC__)
	#include <stdint.h>

/* Microsoft Visual C++ */
#elif defined(_MSC_VER)
	typedef signed __int8 int8_t;
	typedef signed __int16 int16_t;
	typedef signed __int32 int32_t;
	typedef signed __int64 int64_t;
	typedef unsigned __int8 uint8_t;
	typedef unsigned __int16 uint16_t;
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int64 uint64_t;
	
	#define strncasecmp strnicmp
	#define strcasecmp stricmp
	#define snprintf _snprintf
#endif

#ifndef PATH_MAX
	#ifdef MAX_PATH
		#define PATH_MAX MAX_PATH
	#else
		#define PATH_MAX 4096
	#endif
#endif

/********************
*** BYTE SWAPPING ***
********************/

/* SwapUInt32() -- Swap 32-bits */
static uint32_t SwapUInt32(const uint32_t In)
{
	return ((In & 0xFFU) << 24) | ((In & 0xFF00U) << 8) | ((In & 0xFF0000U) >> 8) | ((In & 0xFF000000U) >> 24);
}

/* LittleSwapuInt16() -- Swap 16-bits for little endien */
static uint16_t LittleSwapUInt16(const uint16_t In)
{
#if defined(__BIG_ENDIAN__)
	return ((In & 0xFFU) << 8) | ((In & 0xFF00U) >> 8);
#else
	return In;
#endif
}

/* LittleSwapUInt32() -- Swap 32-bits for little endien */
static uint32_t LittleSwapUInt32(const uint32_t In)
{
#if defined(__BIG_ENDIAN__)
	return SwapUInt32(In);
#else
	return In;
#endif
}

/****************
*** FUNCTIONS ***
****************/

/* main() -- Main entry point */
int main(int argc, char** argv)
{
	int Regen, WADExists;
	char* TXTName, *WADName;
	struct stat StatBuf;
	
	time_t WADDate, TXTDate;
	
	/* Check */
	if (argc < 3)
	{
		fprintf(stderr, "Usage: %s <wadinfo.txt> <output.wad> <prefix>\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	/* Get file names */
	TXTName = argv[1];
	WADName = argv[2];
	
	/* Get the date of the text file */
	memset(&StatBuf, 0, sizeof(StatBuf));
	stat(TXTName, &StatBuf);
	TXTDate = StatBuf.st_mtime;
	
	/* See if WAD Exists */
	WADExists = !!(access(WADName, R_OK) != 0);
	
	// If it does exist, get the date
	WADDate = 0;
	if (WADExists)
	{
		// Obtain Date
		memset(&StatBuf, 0, sizeof(StatBuf));
		stat(WADName, &StatBuf);
		WADDate = StatBuf.st_mtime;
	}
	
	/* Determine if WAD needs regeneration */
	// Clear for checks
	Regen = 0;
	
	// WAD Does not exist
	if (!Regen && !WADExists)
		Regen = true;
	
	// TXT newer than WAD
	if (!Regen && TXTDate > WADDate)
		Regen = true;
}

