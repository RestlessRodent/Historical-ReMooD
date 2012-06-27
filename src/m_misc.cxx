// -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
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
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION:
//      Default Config File.
//      PCX Screenshots.
//      File i/o
//      Common used routines

#include "doomdef.h"
#include "g_game.h"
#include "m_misc.h"
#include "hu_stuff.h"
#include "v_video.h"
#include "z_zone.h"
#include "g_input.h"
#include "i_video.h"
#include "d_main.h"
#include "m_argv.h"
#include "dstrings.h"
#include "i_system.h"
#include "console.h"

// ==========================================================================
//                         FILE INPUT / OUTPUT
// ==========================================================================

//
// checks if needed, and add default extension to filename
//
void FIL_DefaultExtension(char* path, char* extension)
{
	char* src;
	
	// search for '.' from end to begin, add .EXT only when not found
	src = path + strlen(path) - 1;
	
	while (*src != '/' && src != path)
	{
		if (*src == '.')
			return;				// it has an extension
		src--;
	}
	
	strcat(path, extension);
}

//  Creates a resource name (max 8 chars 0 padded) from a file path
//
void FIL_ExtractFileBase(char* path, char* dest)
{
	char* src;
	int length;
	
	src = path + strlen(path) - 1;
	
	// back up until a \ or the start
	while (src != path && *(src - 1) != '\\' && *(src - 1) != '/')
	{
		src--;
	}
	
	// copy up to eight characters
	memset(dest, 0, 8);
	length = 0;
	
	while (*src && *src != '.')
	{
		if (++length == 9)
			I_Error("Filename base of %s >8 chars", path);
			
		*dest++ = toupper((int)*src++);
	}
}

//  Returns true if a filename extension is found
//  There are no '.' in wad resource name
//
bool FIL_CheckExtension(char* in)
{
	while (*in++)
		if (*in == '.')
			return true;
			
	return false;
}

// ==========================================================================
//                        CONFIGURATION FILE
// ==========================================================================

//
// DEFAULTS
//

char configfile[MAX_WADPATH];
char SaveGameLocation[MAX_WADPATH];

// ==========================================================================
//                          CONFIGURATION
// ==========================================================================
bool gameconfig_loaded = false;	// true once config.cfg loaded

//  AND executed

// ==========================================================================
//                            SCREEN SHOTS
// ==========================================================================

typedef struct
{
	char manufacturer;
	char version;
	char encoding;
	char bits_per_pixel;
	
	unsigned short xmin;
	unsigned short ymin;
	unsigned short xmax;
	unsigned short ymax;
	
	unsigned short hres;
	unsigned short vres;
	
	unsigned char palette[48];
	
	char reserved;
	char color_planes;
	unsigned short bytes_per_line;
	unsigned short palette_type;
	
	char filler[58];
	unsigned char data;			// unbounded
} pcx_t;

//
// WritePCXfile
//
bool WritePCXfile(char* filename, uint8_t* data, int width, int height, uint8_t* palette)
{
	return false;
}

//
// M_ScreenShot
//
void M_ScreenShot(void)
{
	int i;
	uint8_t* linear;
	char lbmname[MAX_WADPATH];
	bool ret = false;
	
	// munge planar buffer to linear
	linear = screens[2];
	I_ReadScreen(linear);
	
	// find a file name to save it to
	strcpy(lbmname, "DOOM0000.pcx");
	for (i = 0; i < 10000; i++)
	{
		lbmname[4] = '0' + ((i / 1000) % 10);
		lbmname[5] = '0' + ((i / 100) % 10);
		lbmname[6] = '0' + ((i / 10) % 10);
		lbmname[7] = '0' + ((i / 1) % 10);
		if (access(lbmname, 0) == -1)
			break;				// file doesn't exist
	}
	if (i < 10000)
	{
		// save the pcx file
		//ret = WritePCXfile(lbmname, linear, vid.width, vid.height, W_CacheLumpName("PLAYPAL", PU_CACHE));
	}
	
	if (ret)
		CONL_PrintF("screen shot %s saved\n", lbmname);
	else
		//CONL_PrintF("Couldn't create screen shot\n");
		CONL_PrintF("%s\n", lbmname);
}

// ==========================================================================
//                        MISC STRING FUNCTIONS
// ==========================================================================

//  Temporary varargs CONL_PrintF
//
char* va(char* format, ...)
{
	va_list argptr;
	static char string[1024];
	
	va_start(argptr, format);
	vsprintf(string, format, argptr);
	va_end(argptr);
	
	return string;
}

// s1=s2+s3+s1
void strcatbf(char* s1, char* s2, char* s3)
{
	char tmp[1024];
	
	strcpy(tmp, s1);
	strcpy(s1, s2);
	strcat(s1, s3);
	strcat(s1, tmp);
}
