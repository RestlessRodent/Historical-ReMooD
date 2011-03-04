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
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           TetrisMaster512        (tetrismaster512@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2010 The ReMooD Team..
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
// DESCRIPTION: The not so system specific sound interface.

#ifndef __S_SOUND__
#define __S_SOUND__

#include "sounds.h"

// killough 4/25/98: mask used to indicate sound origin is player item pickup
#define PICKUP_SOUND (0x8000)

extern consvar_t stereoreverse;

extern consvar_t cv_soundvolume;
extern consvar_t cv_musicvolume;
extern consvar_t cv_numChannels;
extern consvar_t cv_rndsoundpitch;

extern consvar_t precachesound;

#ifdef SNDSERV
extern consvar_t sndserver_cmd;
extern consvar_t sndserver_arg;
#endif
#ifdef MUSSERV
extern consvar_t musserver_cmd;
extern consvar_t musserver_arg;
#endif

extern CV_PossibleValue_t soundvolume_cons_t[];
//part of i_cdmus.c
extern consvar_t cd_volume;
extern consvar_t cdUpdate;
#ifdef LINUX
extern consvar_t cv_jigglecdvol;
#endif

#ifdef __MACOS__
typedef enum
{
	music_normal,
	playlist_random,
	playlist_normal
} playmode_t;

extern consvar_t play_mode;
#endif

// register sound vars and commands at game startup
void S_RegisterSoundStuff(void);

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init(int sfxVolume, int musicVolume);

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
void S_StopSounds();
void S_Start(void);

//
// basicaly a wgetnumforname with adding "ds" at the begin of string
// return a lumpnum
//
int S_GetSfxLumpNum(sfxinfo_t * sfx);

//
// Start sound for thing at <origin>
//  using <sound_id> from sounds.h
//
void S_StartSound(void *origin, int sound_id);

// Will start a sound at a given volume.
void S_StartSoundAtVolume(void *origin, int sound_id, int volume);

// Stop sound for thing at <origin>
void S_StopSound(void *origin);

// Start music using <music_id> from sounds.h
void S_StartMusic(int music_id);

// Start music using <music_id> from sounds.h,
//  and set whether looping
void S_ChangeMusic(int music_num, int looping);
void S_ChangeMusicName(char *name, int looping);

// Stops the music fer sure.
void S_StopMusic(void);

// Stop and resume music, during game PAUSE.
void S_PauseSound(void);
void S_ResumeSound(void);

//
// Updates music & sounds
//
void S_UpdateSounds(void);

void S_SetMusicVolume(int volume);
void S_SetSfxVolume(int volume);

int S_SoundPlaying(void *origin, int id);
void S_StartSoundName(void *mo, char *soundname);

void Command_SoundReset_f(void);

struct mobj_s;

int S_AdjustSoundParamsEx(struct mobj_s* Listener, struct mobj_s* Source,
	int32_t* Volume,		// Volume of the sound (Distance) [Mono]
	int32_t* Balance,		// Balance of the sound (left/right) [Stereo + Surround + Full Surround]
	int32_t* Pitch,		// Change in pitch (Doppler!?!?) [All]
	int32_t* Orientation,	// Balance of the sound (front/back) [Surround + Full Surround]
	int32_t* FrontVolume	// How loud to play a sound for the front speaker [Full Surround]
	);

// GhostlyDeath -- Hw3sound.h had these functions, so I'm using a define instead
#define S_StartAttackSound(origin,id) S_StartSound((void*)origin, id)
#define S_StartScreamSound(origin,id) S_StartSound((void*)origin, id)
#define S_StartAmbientSound(id,volume) S_StartSoundAtVolume(NULL, id, volume)

#endif

