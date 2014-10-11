// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Default Config File.

#ifndef __M_MISC__
#define __M_MISC__

#include "doomtype.h"



// the file where all game vars and settings are saved
#define CONFIGFILENAME   "remood.cfg"

//
// MISC
//
//===========================================================================

bool_t FIL_WriteFile(char const* name, void* source, int length);

int FIL_ReadFile(char const* name, uint8_t** buffer);

void FIL_DefaultExtension(char* path, char* extension);

//added:11-01-98:now declared here for use by G_DoPlayDemo(), see there...
void FIL_ExtractFileBase(char* path, char* dest);

bool_t FIL_CheckExtension(char* in);

//===========================================================================

void M_ScreenShot(void);

//===========================================================================

extern char configfile[MAX_WADPATH];

void Command_SaveConfig_f(void);
void Command_LoadConfig_f(void);
void Command_ChangeConfig_f(void);

void M_FirstLoadConfig(void);

//Fab:26-04-98: save game config : cvars, aliases..
void M_SaveConfig(char* filename);

//===========================================================================

// s1=s2+s3+s1 (1024 lenghtmax)
void strcatbf(char* s1, char* s2, char* s3);

extern char SaveGameLocation[MAX_WADPATH];

/* M_SSFormat_t -- Screenshot format */
typedef enum M_SSFormat_e
{
	MSSF_PCX,									// Standard PCX
	MSSF_PNG,									// PNGs
	MSSF_FASTPNG,								// Fast PNGs
	MSSF_PPM,									// Portable Pixmap
	MSSF_FASTPPM,								// Fast PPM
	
	NUMMSSFORMATS
} M_SSFormat_t;

void M_ScreenShotEx(const M_SSFormat_t a_Format, const char* const a_PathName, void* const a_CFile);

#endif

