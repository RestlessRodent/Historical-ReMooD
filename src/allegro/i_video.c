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
// Copyright (C) 2011 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: DOOM graphics stuff for Allegro

/***************
*** INCLUDES ***
***************/

/* System */
#include <stdlib.h>
#include <allegro.h>

/* Local */
#include "doomtype.h"
#include "i_video.h"
#include "i_util.h"

void I_StartFrame(void)
{
}

void I_GetEvent(void)
{
}

void I_StartupMouse(void)
{
}

void I_UpdateJoysticks(void)
{
}

void I_OsPolling(void)
{
}

void I_UpdateNoBlit(void)
{
}

void I_FinishUpdate(void)
{
}

void I_ReadScreen(byte* scr)
{
}

void I_SetPalette(RGBA_t* palette)
{
}

/* VID_PrepareModeList() -- Adds video modes to the mode list */
void VID_PrepareModeList(void)
{
}

/* I_SetVideoMode() -- Sets the current video mode */
boolean I_SetVideoMode(const uint32_t a_Width, const uint32_t a_Height, const boolean a_Fullscreen)
{
}

/* I_StartupGraphics() -- Initializes graphics */
void I_StartupGraphics(void)
{
}

/* I_ShutdownGraphics() -- Turns off graphics */
void I_ShutdownGraphics(void)
{
}

