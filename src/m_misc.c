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

/*#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif*/

#include <fcntl.h>

#ifdef __APPLE__
#include <unistd.h>
#endif


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

#ifdef _WIN32
#include <shlobj.h>
#endif


// ==========================================================================
//                         FILE INPUT / OUTPUT
// ==========================================================================

//
// FIL_WriteFile
//
#ifndef O_BINARY
#define O_BINARY 0
#endif

bool_t FIL_WriteFile(char const* name, void* source, int length)
{
	int handle;
	int count;
	
	handle = open(name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);
	
	if (handle == -1)
		return false;
		
	count = write(handle, source, length);
	close(handle);
	
	if (count < length)
		return false;
		
	return true;
}

//
// FIL_ReadFile : return length, 0 on error
//
//Fab:26-04-98:
//  appends a zero uint8_t at the end
int FIL_ReadFile(char const* name, uint8_t** buffer)
{
	int handle, count, length;
	struct stat fileinfo;
	uint8_t* buf;
	
	handle = open(name, O_RDONLY | O_BINARY, 0666);
	if (handle == -1)
		return 0;
		
	if (fstat(handle, &fileinfo) == -1)
		return 0;
		
	length = fileinfo.st_size;
	buf = Z_Malloc(length + 1, PU_STATIC, 0);
	count = read(handle, buf, length);
	close(handle);
	
	if (count < length)
		return 0;
		
	//Fab:26-04-98:append 0 uint8_t for script text files
	buf[length] = 0;
	
	*buffer = buf;
	return length;
}

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
bool_t FIL_CheckExtension(char* in)
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
bool_t gameconfig_loaded = false;	// true once config.cfg loaded

//  AND executed

void Command_SaveConfig_f(void)
{
	char tmpstr[MAX_WADPATH];
	
	if (COM_Argc() != 2)
	{
		CONL_PrintF("saveconfig <filename[.cfg]> : save config to a file\n");
		return;
	}
	strcpy(tmpstr, COM_Argv(1));
	snprintf(SaveGameLocation, MAX_WADPATH, ".");
	FIL_DefaultExtension(tmpstr, ".cfg");
	
	M_SaveConfig(tmpstr);
	CONL_PrintF("config saved as %s\n", configfile);
}

void Command_LoadConfig_f(void)
{
	if (COM_Argc() != 2)
	{
		CONL_PrintF("loadconfig <filename[.cfg]> : load config from a file\n");
		return;
	}
	
	strcpy(configfile, COM_Argv(1));
	snprintf(SaveGameLocation, MAX_WADPATH, ".");
	FIL_DefaultExtension(configfile, ".cfg");
	
	/*  for create, don't check
	
	   if ( access (tmpstr,F_OK) )
	   {
	   CONL_PrintF("Error reading file %s (not exist ?)\n",tmpstr);
	   return;
	   }
	 */
	COM_BufInsertText(va("exec \"%s\"\n", configfile));
	
}

void Command_ChangeConfig_f(void)
{
	if (COM_Argc() != 2)
	{
		CONL_PrintF("changeconfig <filaname[.cfg]> : save current config and load another\n");
		return;
	}
	
	COM_BufAddText(va("saveconfig \"%s\"\n", configfile));
	COM_BufAddText(va("loadconfig \"%s\"\n", COM_Argv(1)));
}

//
// Load the default config file
//
void M_FirstLoadConfig(void)
{
	/* GhostlyDeath <December 11, 2008> -- New configuration file location code */
	int ConfigMode = 0;
	char* DashConfig = NULL;
	char ReMooDHome[MAX_WADPATH];
	
#ifdef _WIN32
	char Check[MAX_PATH];
#endif
	
	memset(ReMooDHome, 0, sizeof(ReMooDHome));
	
	// 1. Check -config
	if (!ConfigMode && M_CheckParm("-config") && M_IsNextParm())
	{
		DashConfig = M_GetNextParm();
		
		if (DashConfig)
		{
			// Does the file exist and can we write to it?
			if (!access(DashConfig, R_OK | W_OK))
			{
				snprintf(configfile, MAX_WADPATH, "%s", DashConfig);
				snprintf(SaveGameLocation, MAX_WADPATH, ".");
				CONL_PrintF("D_DoomMain: Using %s as the master configuration.\n", DashConfig);
				ConfigMode = 1;
			}
			// Does the file exist but we can't write to it? if so, get the settings
			else if (!access(DashConfig, R_OK))
			{
				CONL_PrintF("D_DoomMain: %s is read-only, copying settings.\n", DashConfig);
				COM_BufInsertText(va("exec \"%s\"\n", configfile));
			}
		}
	}
	// 2. Check CDROM
#if defined(_WIN32) || defined(__MSDOS__)
	if (!ConfigMode && M_CheckParm("-cdrom"))
	{
		I_mkdir("c:\\doomdata", 700);
		strcpy(configfile, "c:/doomdata/" CONFIGFILENAME);
		snprintf(SaveGameLocation, MAX_WADPATH, "c:\\doomdata");
		
		ConfigMode = 1;
		
		CONL_PrintF("D_DoomMain: Using c:\\doomdata\\%s as the master configuration.\n", CONFIGFILENAME);
	}
#endif
	
	// 3. Check ./
	if (!ConfigMode)
	{
		// Does the file exist and can we write to it?
		if (!access(va("./%s", CONFIGFILENAME), R_OK | W_OK))
		{
			snprintf(configfile, MAX_WADPATH, "./%s", CONFIGFILENAME);
			CONL_PrintF("D_DoomMain: Using ./%s as the master configuration.\n", CONFIGFILENAME);
			snprintf(SaveGameLocation, MAX_WADPATH, ".");
			ConfigMode = 1;
		}
		// Does the file exist but we can't write to it? if so, get the settings
		else if (!access(va("./%s", CONFIGFILENAME), R_OK))
		{
			CONL_PrintF("D_DoomMain: ./%s is read-only, copying settings.\n", CONFIGFILENAME);
			COM_BufInsertText(va("exec \"./%s\"\n", CONFIGFILENAME));
		}
	}
	// 4. Check ~
	if (!ConfigMode)
	{
#ifdef _WIN32					// Windows
		// NT has this environment variable
		if (strncasecmp(getenv("OS"), "Windows_NT", 10) == 0)
			if (getenv("HOMEDRIVE") && getenv("HOMEPATH"))
				snprintf(ReMooDHome, MAX_WADPATH, "%s%s\\ReMooD", getenv("HOMEDRIVE"), getenv("HOMEPATH"));
				
		if (ReMooDHome[0])
			if (!access(va("%s\\%s", ReMooDHome, CONFIGFILENAME), R_OK | W_OK))
			{
				snprintf(configfile, MAX_WADPATH, "%s\\%s", ReMooDHome, CONFIGFILENAME);
				CONL_PrintF("D_DoomMain: Using %s\\%s as the master configuration.\n", ReMooDHome, CONFIGFILENAME);
				snprintf(SaveGameLocation, MAX_WADPATH, "%s", ReMooDHome);
				ConfigMode = 1;
			}
#else							// Other OSes
		if (getenv("HOME"))
			snprintf(ReMooDHome, MAX_WADPATH, "%s/.remood", getenv("HOME"));
			
		if (ReMooDHome[0])
		{
			I_mkdir(ReMooDHome, 0700);
			snprintf(configfile, MAX_WADPATH, "%s/%s", ReMooDHome, CONFIGFILENAME);
			snprintf(SaveGameLocation, MAX_WADPATH, "%s", ReMooDHome);
			CONL_PrintF("D_DoomMain: Using %s as the master configuration.\n", configfile);
			ConfigMode = 1;
		}
#endif
	}
	// 5. Check Application Data (Windows Only)
#ifdef _WIN32
	if (!ConfigMode)
	{
		memset(ReMooDHome, 0, sizeof(ReMooDHome));
		
		if (strncasecmp(getenv("OS"), "Windows_NT", 10) == 0)	// All of NT has this
		{
			// It is possible that one may upgrade from XP to Vista...
			if (getenv("APPDATA"))
			{
				snprintf(ReMooDHome, MAX_WADPATH, "%s\\ReMooD", getenv("APPDATA"));
				
				if (!access(va("%s\\%s", ReMooDHome, CONFIGFILENAME), R_OK | W_OK))
				{
					snprintf(configfile, MAX_WADPATH, "%s\\%s", ReMooDHome, CONFIGFILENAME);
					snprintf(SaveGameLocation, MAX_WADPATH, "%s", ReMooDHome);
					CONL_PrintF("D_DoomMain: Using %s\\%s as the master configuration.\n", ReMooDHome, CONFIGFILENAME);
					ConfigMode = 1;	// set this for later use
				}
				else
					memset(ReMooDHome, 0, sizeof(ReMooDHome));
			}
			// No config exists there, so we go back to LOCALAPPDATA and APPDATA
			if (!ReMooDHome[0])
			{
				if (getenv("LOCALAPPDATA"))
					snprintf(ReMooDHome, MAX_WADPATH, "%s\\ReMooD", getenv("LOCALAPPDATA"));
				else if (getenv("APPDATA"))
					snprintf(ReMooDHome, MAX_WADPATH, "%s\\ReMooD", getenv("APPDATA"));
			}
		}
		else					// Use SHGetSpecialFolderLocation w/ CSIDL_APPDATA for 9x
		{
			memset(Check, 0, sizeof(Check));
			
			if (SHGetSpecialFolderPath(NULL, Check, CSIDL_APPDATA, FALSE))
				//if (SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, Check) == S_OK)
				snprintf(ReMooDHome, MAX_WADPATH, "%s\\ReMooD", Check);
		}
		
		// Common between the two
		if (ReMooDHome[0] && !ConfigMode)	// if ConfigMode was set then a config was found
		{
			I_mkdir(ReMooDHome, 0700);
			snprintf(configfile, MAX_WADPATH, "%s\\%s", ReMooDHome, CONFIGFILENAME);
			snprintf(SaveGameLocation, MAX_WADPATH, "%s", ReMooDHome);
			CONL_PrintF("D_DoomMain: Using %s as the master configuration.\n", configfile);
			ConfigMode = 1;
		}
	}
#endif
	
	// If All else fails
	if (!ConfigMode)
	{
		snprintf(configfile, MAX_WADPATH, "./%s", CONFIGFILENAME);
		snprintf(SaveGameLocation, MAX_WADPATH, ".");
	}
	else
	{
		// load config, make sure those commands doesnt require the screen..
		CONL_PrintF("\n");
		COM_BufInsertText(va("exec \"%s\"\n", configfile));
		COM_BufExecute();		// make sure initial settings are done
		
		// make sure I_Quit() will write back the correct config
		// (do not write back the config if it crash before)
		gameconfig_loaded = true;
	}
	
	CONL_PrintF("M_FirstLoadConfig: Using configuration file \"%s\"\n", configfile);
}

//  Save all game config here
//
void M_SaveConfig(char* filename)
{
	FILE* f;
	
	// make sure not to write back the config until
	//  it's been correctly loaded
	/*if (!gameconfig_loaded)
	   {
	   CONL_PrintF("M_SaveConfig: Configuration never loaded\n");
	   return;
	   } */
	
	// can change the file name
	if (filename)
	{
		f = fopen(filename, "w");
		// change it only if valide
		if (f)
			strcpy(configfile, filename);
		else
		{
			CONL_PrintF("Couldn't save game config file %s\n", filename);
			return;
		}
	}
	else
	{
		f = fopen(configfile, "w");
		if (!f)
		{
			CONL_PrintF("Couldn't save game config file %s\n", configfile);
			return;
		}
	}
	
	// header message
	fprintf(f, "// ReMooD Configuration File (Version %i.%i%c \"%s\"; Legacy Version %i.%i)\n",
	        REMOOD_MAJORVERSION, REMOOD_MINORVERSION, REMOOD_RELEASEVERSION, REMOOD_VERSIONCODESTRING, VERSION / 100, VERSION % 100);
	fprintf(f, "// See %s for more details.\n", REMOOD_URL);
	
	//FIXME: save key aliases if ever implemented..
	
	CV_SaveVariables(f);
	
	fclose(f);
	
	CONL_PrintF("M_SaveConfig: Saved configuration to \"%s\"\n", configfile);
}

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
bool_t WritePCXfile(char* filename, uint8_t* data, int width, int height, uint8_t* palette)
{
	int i;
	int length;
	pcx_t* pcx;
	uint8_t* pack;
	
	pcx = Z_Malloc(width * height * 2 + 1000, PU_STATIC, NULL);
	
	pcx->manufacturer = 0x0a;	// PCX id
	pcx->version = 5;			// 256 color
	pcx->encoding = 1;			// uncompressed
	pcx->bits_per_pixel = 8;	// 256 color
	pcx->xmin = 0;
	pcx->ymin = 0;
	pcx->xmax = LittleSwapInt16(width - 1);
	pcx->ymax = LittleSwapInt16(height - 1);
	pcx->hres = LittleSwapInt16(width);
	pcx->vres = LittleSwapInt16(height);
	memset(pcx->palette, 0, sizeof(pcx->palette));
	pcx->color_planes = 1;		// chunky image
	pcx->bytes_per_line = LittleSwapInt16(width);
	pcx->palette_type = LittleSwapInt16(1);	// not a grey scale
	memset(pcx->filler, 0, sizeof(pcx->filler));
	
	// pack the image
	pack = &pcx->data;
	
	for (i = 0; i < width * height; i++)
	{
		if ((*data & 0xc0) != 0xc0)
			*pack++ = *data++;
		else
		{
			*pack++ = 0xc1;
			*pack++ = *data++;
		}
	}
	
	// write the palette
	*pack++ = 0x0c;				// palette ID byte
	for (i = 0; i < 768; i++)
		*pack++ = *palette++;
		
	// write output file
	length = pack - (uint8_t*)pcx;
	i = FIL_WriteFile(filename, pcx, length);
	
	Z_Free(pcx);
	return i;
}

//
// M_ScreenShot
//
void M_ScreenShot(void)
{
	int i;
	uint8_t* linear;
	char lbmname[MAX_WADPATH];
	bool_t ret = false;
	
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
