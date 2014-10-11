// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: ReMooD `deutex` Clone, for what ReMooD uses and bonus stuff

/***************
*** INCLUDES ***
***************/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

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

typedef enum
{
	false,
	true
} bool_t;

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
#define BUFSIZE 128
	char Buf[BUFSIZE];
	int en, len, i;
	bool_t Regen, WADExists;
	char* TXTName, *WADName, *d, *s;
	struct stat StatBuf;
	FILE* TXTFile, *WADFile;
	
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
	
	/* Attempt opening the text file */
	TXTFile = fopen(TXTName, "rt");
	en = errno;
	
	// Failed?
	if (!TXTFile)
	{
		fprintf(stderr, "Failed to open %s: %s.\n", TXTName, strerror(en));
		return EXIT_FAILURE;
	}
	
	/* See if WAD Exists */
	WADExists = !!(access(WADName, R_OK) == 0);
	
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
	
	/* Read INFO text */
	while (!feof(TXTFile))
	{
		// Read line
		memset(Buf, 0, sizeof(Buf));
		fgets(Buf, BUFSIZE - 1, TXTFile);
		
		// Go through string and convert all whitespace to standard space
		len = strlen(Buf);
		for (i = 0; i < len; i++)
			if (isspace(Buf[i]))
				Buf[i] = ' ';
		
		// Skip whitespace at start
		while (Buf[0] && isspace(Buf[0]))
			memmove(&Buf[0], &Buf[1], sizeof(*Buf) * (BUFSIZE - 1));
		
		// Comment?
		if (Buf[0] == '#')
			continue;
		
		// Remove whitespace at end of line
		for (;;)
		{
			len = strlen(Buf);
			
			if (len > 0 && Buf[len - 1] == ' ')
				Buf[len - 1] = 0;
			else
				break;
		}
		
		// Empty Line
		if (!Buf[0])
			continue;
		
		// Debug
		fprintf(stdout, ">> `%s`\n", Buf);
	}
	
	/* Not regenerating */
	if (!Regen)
	{
		fprintf(stdout, "%s does not need updating.\n", WADName);
	}
	
	/* Regenerate WAD? */
	else
	{
		// Message
		fprintf(stdout, "%senerating %s...\n", (!WADExists ? "G" : "Reg"), WADName);
		fprintf(stdout, "Because ");
		
		if (!WADExists)
			fprintf(stdout, "%s does not exist", WADName);
		else if (TXTDate > WADDate)
			fprintf(stdout, "%s is newer than %s", TXTName, WADName);
		else
			fprintf(stdout, "of an unknown reason"); 
		
		fprintf(stdout, ".\n");
		
		// Attempt creation of target WAD
		
		// Start Regeneration Cycle
	}

#undef BUFSIZE
}

