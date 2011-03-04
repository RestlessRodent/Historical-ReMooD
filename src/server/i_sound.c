// Emacs style mode select   -*- C++ -*- 
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
// DESCRIPTION:
//      System interface for sound.

#include "i_thread.h"
#include "i_sound.h"
#include "z_zone.h"
#include "m_swap.h"
#include "i_system.h"
#include "i_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"
#include "doomdef.h"
#include "doomstat.h"
#include "s_sound.h"
#include "doomtype.h"
#include "d_main.h"

#ifdef GAMECLIENT
consvar_t cv_snd_speakersetup = {"snd_speakersetup", "2", CV_SAVE};
consvar_t cv_snd_soundquality = {"snd_soundquality", "11025", CV_SAVE};
consvar_t cv_snd_sounddensity = {"snd_sounddensity", "1", CV_SAVE};
consvar_t cv_snd_pcspeakerwave = {"snd_pcspeakerwave", "1", CV_SAVE};
consvar_t cv_snd_channels = {"snd_numchannels", "16", CV_SAVE};
consvar_t cv_snd_reservedchannels = {"snd_reservedchannels", "4", CV_SAVE};
consvar_t cv_snd_multithreaded = {"snd_multithreaded", "1", CV_SAVE};
consvar_t cv_snd_output = {"snd_output", "Default", CV_SAVE};
consvar_t cv_snd_device = {"snd_device", "auto", CV_SAVE};
#endif

void I_UpdateSound_sdl(void *unused, int8_t* stream, int len)
{
}

void I_SOUND_MakeDigital(void)
{
}

void I_SOUND_MakeAnalog(void)
{
}

void I_SOUND_DestroyDigital(void)
{
}

void I_SOUND_DestroyAnalog(void)
{
}

void I_StartupSound()
{
}

void I_ShutdownSound(void)
{
}

boolean I_UpdateSoundCache(void)
{
}

void *I_GetSfx(sfxinfo_t* sfx)
{
	return NULL;
}

void I_FreeSfx(sfxinfo_t* sfx)
{
}

void I_SetSfxVolume(int volume)
{
}

void I_UpdateSound(void)
{
}

void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
}

int I_StartSoundEx(int id, int vol, int sep, int pitch, int priority, mobj_t * origin, int orientation, int front, int center)
{
	return -1;
}

int I_StartSound(int id, int vol, int sep, int pitch, int priority, mobj_t * origin)
{
	return I_StartSoundEx(id, vol, sep, pitch, priority, origin, 128, 0, 0);
}

void I_StopSound(int handle)
{
}

void I_CutOrigonator(void *origin)
{
}

int I_SoundIsPlaying(int handle)
{
	return 0;
}

void I_SubmitSound(void)
{
}

