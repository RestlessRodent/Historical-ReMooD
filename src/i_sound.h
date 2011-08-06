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
// DESCRIPTION: System interface, sound.

#ifndef __I_SOUND__
#define __I_SOUND__

#include "doomdef.h"
#include "sounds.h"
#include "command.h"

#include "p_mobj.h"
#include "g_game.h"

void *I_GetSfx(sfxinfo_t * sfx);
void I_FreeSfx(sfxinfo_t * sfx);

// Init at program start...
void I_StartupSound();

// ... update sound buffer and audio device at runtime...
void I_UpdateSound(void);
void I_SubmitSound(void);

// ... shut down and relase at program termination.
void I_ShutdownSound(void);

//
//  SFX I/O
//

// Starts a sound in a particular sound channel.
int I_StartSound(int id, int vol, int sep, int pitch, int priority, mobj_t * origin);

// Stops a sound channel.
void I_StopSound(int handle);
void I_CutOrigonator(void *origin);	// Stops a sound originating from something

// Called by S_*() functions
//  to see if a channel is still playing.
// Returns 0 if no longer playing, 1 if playing.
int I_SoundIsPlaying(int handle);

// Updates the volume, separation,
//  and pitch of a sound channel.
void I_UpdateSoundParams(int handle, int vol, int sep, int pitch);

//
//  MUSIC I/O
//
bool_t I_MusicDriverInit(void);

void I_InitCD(void);
void I_StopCD(void);
void I_PauseCD(void);
void I_ResumeCD(void);
void I_ShutdownCD(void);
void I_UpdateCD(void);
void I_PlayCD(int track, bool_t looping);
int I_SetVolumeCD(int volume);	// return 0 on failure

extern uint8_t cdaudio_started;
extern consvar_t cv_snd_speakersetup;
extern consvar_t cv_snd_soundquality;
extern consvar_t cv_snd_pcspeakerwave;
extern consvar_t cv_snd_output;
extern consvar_t cv_snd_device;
extern consvar_t cv_snd_channels;
extern consvar_t cv_snd_reservedchannels;
extern consvar_t cv_snd_multithreaded;
extern consvar_t cv_snd_sounddensity;

int I_StartSoundEx(int id, int vol, int sep, int pitch, int priority, mobj_t * origin, int orientation, int front, int center);

#endif

