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
//      DOOM graphics stuff for SDL

#include <stdlib.h>

#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "m_argv.h"
#include "d_main.h"
#include "s_sound.h"
#include "g_input.h"
#include "g_game.h"
#include "i_video.h"
#include "console.h"
#include "command.h"

#ifdef GAMECLIENT
consvar_t cv_vidwait = {"vid_wait","1",CV_SAVE,CV_OnOff};
byte graphics_started = 0;
boolean allow_fullscreen = false;
#endif

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

int VID_NumModes(void)
{
   return 0;
}

char* VID_GetModeName(int modeNum)
{
	return NULL;
}

int VID_GetModeForSize(int w, int h)
{
	return 0;
}

void VID_PrepareModeList(void)
{
}

int VID_SetMode(int modeNum)
{
	return 1;
}

void I_StartupGraphics(void)
{
}

void I_ShutdownGraphics(void)
{
}

