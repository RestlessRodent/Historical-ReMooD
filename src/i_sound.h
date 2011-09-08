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
extern consvar_t cv_snd_speakersetup;
extern consvar_t cv_snd_soundquality;
extern consvar_t cv_snd_pcspeakerwave;
extern consvar_t cv_snd_output;
extern consvar_t cv_snd_device;
extern consvar_t cv_snd_channels;
extern consvar_t cv_snd_reservedchannels;
extern consvar_t cv_snd_multithreaded;
extern consvar_t cv_snd_sounddensity;

#endif

