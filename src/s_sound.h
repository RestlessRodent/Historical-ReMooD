// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: The not so system specific sound interface.

#ifndef __S_SOUND_H__
#define __S_SOUND_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "sounds.h"




/****************
*** CONSTANTS ***
****************/

/*****************
*** STRUCTURES ***
*****************/

typedef int32_t sfxid_t;

/* Define S_NoiseThinker_t */
#if !defined(__REMOOD_SNOISETHNK_DEFINED)
	typedef struct S_NoiseThinker_s S_NoiseThinker_t;
	#define __REMOOD_SNOISETHNK_DEFINED
#endif

/**************
*** GLOBALS ***
**************/

/****************
*** FUNCTIONS ***
****************/

void S_RegisterSoundStuff(void);
void S_Init(int sfxVolume, int musicVolume);
void S_StopSounds(void);
int S_GetSfxLumpNum(sfxinfo_t* sfx);
void S_FreeSfx(sfxinfo_t* sfx);

void S_StartSoundAtVolume(S_NoiseThinker_t* a_Origin, sfxid_t sound_id, int volume, const bool_t a_Reverse);
int S_SoundPlaying(S_NoiseThinker_t* a_Origin, int id);

void S_XStartSound(S_NoiseThinker_t* a_Origin, sfxid_t sound_id);
void S_XStartSoundRev(S_NoiseThinker_t* a_Origin, sfxid_t sound_id);
void S_XStartSoundName(S_NoiseThinker_t* a_Origin, char* soundname);
void S_XStartSoundNameRev(S_NoiseThinker_t* a_Origin, char* soundname);
void S_XStopSound(S_NoiseThinker_t* a_Origin);

// Start sound but emit sound only for a specific player
#define S_StartSoundScreen(o,i,p) S_XStartSound(((S_NoiseThinker_t*)(o)), (i))

// To cancel out those annoying (expected S_NoiseThinker_t* but not mobj_t*)
#define S_StartSound(o,i) S_XStartSound(((S_NoiseThinker_t*)(o)), (i))
#define S_StartSoundRev(o,i) S_XStartSoundRev(((S_NoiseThinker_t*)(o)), (i))
#define S_StartSoundName(o,i) S_XStartSoundName(((S_NoiseThinker_t*)(o)), (i))
#define S_StartSoundNameRev(o,i) S_XStartSoundNameRev(((S_NoiseThinker_t*)(o)), (i))
#define S_StopSound(o) S_XStopSound(((S_NoiseThinker_t*)(o)))

void S_RepositionSounds(void);

void S_ChangeMusic(int music_num, int looping);
void S_ChangeMusicName(char* name, int looping);
void S_StopMusic(void);
void S_PauseMusic(void);
void S_ResumeMusic(void);
void S_UpdateSounds(const bool_t a_Threaded);
void S_SetMusicVolume(int volume);
void S_SetSfxVolume(int volume);
void Command_SoundReset_f(void);

sfxid_t S_SoundIDForName(const char* const a_Name);

#endif							/* __S_SOUND_H__ */

