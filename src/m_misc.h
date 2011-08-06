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
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2011 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION: Default Config File.
//              PCX Screenshots.
//              File i/o
//              Common used routines

#ifndef __M_MISC__
#define __M_MISC__

#include "doomtype.h"
#include "w_wad.h"

// the file where all game vars and settings are saved
#define CONFIGFILENAME   "remood.cfg"

//
// MISC
//
//===========================================================================

bool_t FIL_WriteFile(char const *name, void *source, int length);

int FIL_ReadFile(char const *name, uint8_t ** buffer);

void FIL_DefaultExtension(char *path, char *extension);

//added:11-01-98:now declared here for use by G_DoPlayDemo(), see there...
void FIL_ExtractFileBase(char *path, char *dest);

bool_t FIL_CheckExtension(char *in);

//===========================================================================

void M_ScreenShot(void);

//===========================================================================

extern char configfile[MAX_WADPATH];

void Command_SaveConfig_f(void);
void Command_LoadConfig_f(void);
void Command_ChangeConfig_f(void);

void M_FirstLoadConfig(void);
//Fab:26-04-98: save game config : cvars, aliases..
void M_SaveConfig(char *filename);

//===========================================================================

// s1=s2+s3+s1 (1024 lenghtmax)
void strcatbf(char *s1, char *s2, char *s3);

extern char SaveGameLocation[MAX_WADPATH];

#endif

