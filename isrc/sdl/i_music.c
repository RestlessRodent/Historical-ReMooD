// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.sourceforge.net/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           Demyx                  (demyx@endgameftw.com)
//                    Dragan                 (poliee13@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 2008 ReMooD Team.
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION: Music

#include "i_sound.h"
#include "m_argv.h"

static boolean MusicStarted = false;
static boolean MusicEnabled = false;

#ifdef _WIN32
#else
#endif

/*************
*** SYSTEM ***
*************/

void I_InitMusic(void)
{
	if (M_CheckParm("-nosound") || M_CheckParm("-nomusic"))
		return;

#ifdef _WIN32
#else
	/* Linux and such does not really support wavetable MIDI */
#endif
}

void I_ShutdownMusic(void)
{
	if (!MusicStarted)
		return;
}

void I_UpdateMusic(void)
{
	if (!MusicStarted || !MusicEnabled)
		return;
}

void I_SetMusicVolume(int volume)
{
	if (!MusicStarted || !MusicEnabled)
		return;
}

void I_SetFMODVolume(int volume)
{
	if (!MusicStarted || !MusicEnabled)
		return;
}

/******************************
*** LOADING/UNLOADING SONGS ***
******************************/

int I_RegisterSong(char* lumpname)
{
	if (!MusicStarted || !MusicEnabled)
		return 0;
	
	return 1;
}

void I_UnRegisterSong(int handle)
{
	if (!MusicStarted || !MusicEnabled)
		return 0;
	
	return 1;
}

/**************
*** CONTROL ***
**************/

void I_PauseSong(int handle)
{
	if (!MusicStarted || !MusicEnabled)
		return;
}

void I_ResumeSong(int handle)
{
	if (!MusicStarted || !MusicEnabled)
		return;
}

void I_PlaySong(int handle, int looping)
{
	if (!MusicStarted || !MusicEnabled)
		return;
}

void I_StopSong(int handle)
{
	if (!MusicStarted || !MusicEnabled)
		return;
}

