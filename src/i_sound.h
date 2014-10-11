// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: System interface, sound.

#ifndef __I_SOUND__
#define __I_SOUND__

#include "z_zone.h"








/* Sound Stuff */
bool_t I_SoundDriverInit(void);

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

#endif

