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
// DESCRIPTION: System specific interface stuff.

#ifndef __I_VIDEO__
#define __I_VIDEO__

#include "doomtype.h"

void I_StartupGraphics(void);	//setup video mode
void I_ShutdownGraphics(void);	//restore old video mode

// Takes full 8 bit values.
void I_SetPalette(RGBA_t * palette);

#ifdef __MACOS__
void macConfigureInput(void);

void VID_Pause(int pause);
#endif

void I_UpdateNoBlit(void);
void I_FinishUpdate(void);

// Wait for vertical retrace or pause a bit.
void I_WaitVBL(int count);

void I_ReadScreen(byte * scr);

void I_BeginRead(void);
void I_EndRead(void);

/****************
*** FUNCTIONS ***
****************/

void VID_PrepareModeList(void);
boolean I_SetVideoMode(const uint32_t a_Width, const uint32_t a_Height, const boolean a_Fullscreen);

#endif

