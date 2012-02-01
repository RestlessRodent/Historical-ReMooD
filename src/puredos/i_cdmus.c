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
//      cd music interface

#include "doomtype.h"
#include "i_sound.h"
#include "command.h"
#include "m_argv.h"

#ifdef GAMECLIENT
consvar_t cd_volume = { "cd_volume", "31", CV_SAVE};
consvar_t cdUpdate = { "cd_update", "1", CV_SAVE };
#endif

void I_StopCD(void)
{
}

void I_PauseCD(void)
{
}

void I_ResumeCD(void)
{
}

void I_ShutdownCD(void)
{
}

void I_InitCD(void)
{
}

void I_UpdateCD(void)
{
}

void I_PlayCD(int track, boolean looping)
{
}

int I_SetVolumeCD(int volume)
{
	return 0;
}

