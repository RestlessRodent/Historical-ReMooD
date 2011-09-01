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
// Copyright (C) 2011 GhostlyDeath <ghostlydeath@remood.org>
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

/***************
*** INCLUDES ***
***************/

/* System */
#include <SDL.h>

/* Local */
#include "doomtype.h"
#include "i_sound.h"
#include "i_util.h"

/*****************
*** STRUCTURES ***
*****************/

/* I_SDLSoundLocal_t -- Local sound stuff */
typedef struct I_SDLSoundLocal_s
{
	int RetVal;
	SDL_AudioSpec Spec;
	void* DoubleBuffer;
	volatile bool_t DoWrite, IsFinished;
	volatile size_t CopyStreamCount;
} I_SDLSoundLocal_t;

/**********************************
*** ALLEGRO SOUND STREAM DRIVER ***
**********************************/

/* I_SDLSD_Callback() -- Callback for audio processing */
static void I_SDLSD_Callback(struct I_SoundDriver_s* a_Driver, uint8_t* a_Stream, int a_Length)
{
	I_SDLSoundLocal_t* Local;
	
	/* Check */
	if (!a_Driver || !a_Stream || !a_Length)
		return;
	
	/* Get Local */
	Local = (I_SDLSoundLocal_t*)a_Driver->Data;
	
	// Check
	if (!Local)
		return;
		
	/* Copy the buffer? */
	if (!Local->DoWrite)
		return;
	
	// Do actual copy
	memmove(a_Stream, Local->DoubleBuffer, (a_Length < Local->Spec.size ? a_Length : Local->Spec.size));
	
	/* Finished writing */
	Local->IsFinished = true;
	SDL_PauseAudio(1);
}

/* I_SDLSD_Init() -- Initializes a driver */
bool_t I_SDLSD_Init(struct I_SoundDriver_s* const a_Driver)
{
	/* Check */
	if (!a_Driver)
		return false;
	
	/* Initialize SDL Audio */
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1)
		return false;
	
	/* Success */
	return true;
}

/* I_SDLSD_Destroy() -- Destroys a driver */
bool_t I_SDLSD_Destroy(struct I_SoundDriver_s* const a_Driver)
{
	/* Check */
	if (!a_Driver)
		return false;
	
	/* Quit SDL */
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	
	/* Success */
	return true;
}

/* I_SDLSD_Success() -- Success */
void I_SDLSD_Success(struct I_SoundDriver_s* const a_Driver)
{
	I_SDLSoundLocal_t* Local;
	
	/* Check */
	if (!a_Driver)
		return;
		
	/* Allocate local data */
	a_Driver->Size = sizeof(I_SDLSoundLocal_t);
	a_Driver->Data = Z_Malloc(a_Driver->Size, PU_STATIC, NULL);
	
	/* Get Local */
	Local = (I_SDLSoundLocal_t*)a_Driver->Data;
	
	// Check
	if (!Local)
		return;
	
	/* Set return value */
	Local->RetVal = -1;
}

/* I_SDLSD_Request() -- Requests a buffer for this driver */
size_t I_SDLSD_Request(struct I_SoundDriver_s* const a_Driver, const uint8_t a_Bits, const uint16_t a_Freq, const uint8_t a_Channels, const uint32_t a_Samples)
{
	I_SDLSoundLocal_t* Local;
	
	/* Check */
	if (!a_Driver || !a_Bits || !a_Freq || a_Channels > 2 || !a_Samples)
		return 0;
	
	/* Get Local */
	Local = (I_SDLSoundLocal_t*)a_Driver->Data;
	
	// Check
	if (!Local)
		return 0;
	
	/* Ask SDL for a buffer */
	// Say what I want
	memset(&Local->Spec, 0, sizeof(Local->Spec));
	Local->Spec.freq = a_Freq;
	Local->Spec.format = (a_Bits == 16 ? AUDIO_U16 : AUDIO_U8);
	Local->Spec.channels = a_Channels;
	Local->Spec.samples = a_Samples;
	Local->Spec.userdata = a_Driver;
	Local->Spec.callback = I_SDLSD_Callback;
	
	// Get what I want (hopefully anyway)
	Local->RetVal = SDL_OpenAudio(&Local->Spec, NULL);
	
	// Failed?
	if (Local->RetVal != 0)
		return 0;
	
	/* Allocate double buffer */
	Local->DoubleBuffer = Z_Malloc(Local->Spec.size, PU_STATIC, NULL);
	Local->IsFinished = true;
	Local->DoWrite = false;
	
	/* Unpause audio, it is ready for playing */
	SDL_PauseAudio(0);
	
	/* Success */
	return Local->Spec.samples;
}

/* I_SDLSD_Obtain() -- Obtains a buffer that was requested */
void* I_SDLSD_Obtain(struct I_SoundDriver_s* const a_Driver)
{
	I_SDLSoundLocal_t* Local;
	
	/* Check */
	if (!a_Driver)
		return NULL;
	
	/* Get Local */
	Local = (I_SDLSoundLocal_t*)a_Driver->Data;
	
	// Check
	if (!Local)
		return NULL;
	
	/* Return double buffer */
	return Local->DoubleBuffer;
}

/* I_SDLSD_IsFinished() -- Checks if the buffer is finished playing */
bool_t I_SDLSD_IsFinished(struct I_SoundDriver_s* const a_Driver)
{
	I_SDLSoundLocal_t* Local;
	bool_t RetVal;
	
	/* Check */
	if (!a_Driver)
		return false;
	
	/* Get Local */
	Local = (I_SDLSoundLocal_t*)a_Driver->Data;
	
	// Check
	if (!Local)
		return;
	
	/* Return finished status */
	// Do audio locking, this should prevent echos from occuring
	SDL_LockAudio();
	RetVal = Local->IsFinished;
	SDL_UnlockAudio();
	
	return RetVal;
}

/* I_SDLSD_WriteOut() -- Done streaming into buffer */
void I_SDLSD_WriteOut(struct I_SoundDriver_s* const a_Driver)
{
	I_SDLSoundLocal_t* Local;
	
	/* Check */
	if (!a_Driver)
		return;
	
	/* Get Local */
	Local = (I_SDLSoundLocal_t*)a_Driver->Data;
	
	// Check
	if (!Local)
		return;
	
	/* Clear finished, and set DoWrite */
	SDL_LockAudio();
	Local->DoWrite = true;
	Local->IsFinished = false;
	SDL_PauseAudio(0);
	SDL_UnlockAudio();
}

/* I_SDLSD_UnRequest() -- Unrequests a buffer previously obtained */
void I_SDLSD_UnRequest(struct I_SoundDriver_s* const a_Driver)
{
	I_SDLSoundLocal_t* Local;
	
	/* Check */
	if (!a_Driver)
		return;
	
	/* Get Local */
	Local = (I_SDLSoundLocal_t*)a_Driver->Data;
	
	// Check
	if (!Local)
		return;
	
	/* Check if RetVal is non-zero (failure) */
	if (Local->RetVal != 0)
		return;
		
	/* Free the stream */
	SDL_CloseAudio();
	
	// Clear out
	Local->RetVal = -1;
	if (Local->DoubleBuffer)
		Z_Free(Local->DoubleBuffer);
	Local->DoubleBuffer = NULL;
}

/* I_SDLSD_GetFreq() -- Get frequency of driver */
uint16_t I_SDLSD_GetFreq(struct I_SoundDriver_s* const a_Driver)
{
	I_SDLSoundLocal_t* Local;
	
	/* Check */
	if (!a_Driver)
		return 0;
	
	/* Get Local */
	Local = (I_SDLSoundLocal_t*)a_Driver->Data;
	
	// Check
	if (!Local)
		return 0;
	
	/* Return frequency */
	return Local->Spec.freq;
}

/* l_SDLSound -- SDL sound driver */
static I_SoundDriver_t l_SDLSoundDriver =
{
	/* Info */
	"SDL SoundStream",
	"sdlsnd",
	(1 << IST_WAVEFORM),
	127,
	
	/* Functions */
	I_SDLSD_Init,
	I_SDLSD_Destroy,
	I_SDLSD_Success,
	I_SDLSD_Request,
	I_SDLSD_Obtain,
	I_SDLSD_IsFinished,
	I_SDLSD_WriteOut,
	I_SDLSD_UnRequest,
	I_SDLSD_GetFreq,
};

/****************
*** FUNCTIONS ***
****************/

/* I_SoundDriverInit() -- Initializes sound */
bool_t I_SoundDriverInit(void)
{
	/* Add SDL Sound Driver */
	if (!I_AddSoundDriver(&l_SDLSoundDriver))
		CONS_Printf("I_SoundDriverInit: Failed to add SDL Driver\n");
	
	/* Success */
	return true;
}
