// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
//         :oCCCCOCoc.
//     .cCO8OOOOOOOOO8Oo:
//   .oOO8OOOOOOOOOOOOOOOCc
//  cO8888:         .:oOOOOC.                                                TM
// :888888:   :CCCc   .oOOOOC.     ###      ###                    #########
// C888888:   .ooo:   .C########   #####  #####  ######    ######  ##########
// O888888:         .oO###    ###  #####  ##### ########  ######## ####    ###
// C888888:   :8O.   .C##########  ### #### ### ##    ##  ##    ## ####    ###
// :8@@@@8:   :888c   o###         ### #### ### ########  ######## ##########
//  :8@@@@C   C@@@@   oo########   ###  ##  ###  ######    ######  #########
//    cO@@@@@@@@@@@@@@@@@Oc0
//      :oO8@@@@@@@@@@Oo.
//         .oCOOOOOCc.                                      http://remood.org/
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: The not so system specific sound interface.

#ifndef __S_SOUND_H__
#define __S_SOUND_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "sounds.h"

#include "m_fixed.h"
#include "tables.h"

/****************
*** CONSTANTS ***
****************/

/*****************
*** STRUCTURES ***
*****************/

typedef int32_t sfxid_t;

struct mobj_s;

/* S_NoiseThinker_t -- A thinker that makes noise */
typedef struct S_NoiseThinker_s
{
	uint32_t Flags;				// Sound flags
	
	/* World Position */
	fixed_t x;
	fixed_t y;
	fixed_t z;
	
	/* Momenntum */
	// This is for doppler and such
	fixed_t momx;
	fixed_t momy;
	fixed_t momz;
	
	/* Other things */
	fixed_t Pitch;				// Pitch modification
	fixed_t Volume;				// Volume modification
	angle_t Angle;				// Angle
} S_NoiseThinker_t;

/**************
*** GLOBALS ***
**************/

/****************
*** FUNCTIONS ***
****************/

void S_RegisterSoundStuff(void);
void S_Init(int sfxVolume, int musicVolume);
void S_StopSounds(void);
void S_Start(void);
int S_GetSfxLumpNum(sfxinfo_t* sfx);
void S_FreeSfx(sfxinfo_t* sfx);

void S_StartSound(S_NoiseThinker_t* a_Origin, sfxid_t sound_id);
void S_StartSoundAtVolume(S_NoiseThinker_t* a_Origin, sfxid_t sound_id, int volume);
void S_StartSoundName(S_NoiseThinker_t* a_Origin, char* soundname);
void S_StopSound(S_NoiseThinker_t* a_Origin);
int S_SoundPlaying(S_NoiseThinker_t* a_Origin, int id);

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

