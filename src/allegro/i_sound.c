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
// DJGPP's Allegro explodes if this isn't included first
#if defined(__DJGPP__)
	#include <stdint.h>
#endif

#include <allegro.h>

// Include winalleg on Windows since it conflicts!
#if defined(_WIN32)
	#include <winalleg.h>
#endif

/* Local */
#include "doomtype.h"
#include "i_sound.h"
#include "i_util.h"

/*****************
*** STRUCTURES ***
*****************/

/* I_AllegroSoundLocal_t -- Local sound stuff */
typedef struct I_AllegroSoundLocal_s
{
	AUDIOSTREAM* Stream;
	void* Buffer;
	
	size_t Len;
	uint8_t Bits;
	uint8_t Channels;
	uint16_t Freq;
} I_AllegroSoundLocal_t;

/**********************************
*** ALLEGRO SOUND STREAM DRIVER ***
**********************************/

/* I_AllegroSD_Init() -- Initializes a driver */
bool_t I_AllegroSD_Init(struct I_SoundDriver_s* const a_Driver)
{
	/* Check */
	if (!a_Driver)
		return false;
	
	/* Success */
	return true;
}

/* I_AllegroSD_Destroy() -- Destroys a driver */
bool_t I_AllegroSD_Destroy(struct I_SoundDriver_s* const a_Driver)
{
	/* Check */
	if (!a_Driver)
		return false;
	
	/* Success */
	return true;
}

/* I_AllegroSD_Success() -- Success */
void I_AllegroSD_Success(struct I_SoundDriver_s* const a_Driver)
{
	/* Check */
	if (!a_Driver)
		return;
		
	/* Allocate local data */
	a_Driver->Size = sizeof(I_AllegroSoundLocal_t);
	a_Driver->Data = Z_Malloc(a_Driver->Size, PU_STATIC, NULL);
}

/* I_AllegroSD_Request() -- Requests a buffer for this driver */
bool_t I_AllegroSD_Request(struct I_SoundDriver_s* const a_Driver, const uint8_t a_Bits, const uint16_t a_Freq, const uint8_t a_Channels, const uint32_t a_Samples)
{
	I_AllegroSoundLocal_t* Local;
	
	/* Check */
	if (!a_Driver || !a_Bits || !a_Freq || a_Channels > 2 || !a_Samples)
		return false;
	
	/* Get Local */
	Local = (I_AllegroSoundLocal_t*)a_Driver->Data;
	
	// Check
	if (!Local)
		return false;
	
	/* Try getting the buffer */
	Local->Stream = play_audio_stream(a_Samples, a_Bits, (a_Channels == 2 ? TRUE : FALSE), a_Freq, 255, 127);
	
	// Check
	if (!Local->Stream)
		return false;
	
	/* Set local parms */
	Local->Freq = a_Freq;//voice_get_frequency(Local->Stream);
	
	/* Success */
	return true;
}

/* I_AllegroSD_Obtain() -- Obtains a buffer that was requested */
void* I_AllegroSD_Obtain(struct I_SoundDriver_s* const a_Driver)
{
	I_AllegroSoundLocal_t* Local;
	
	/* Check */
	if (!a_Driver)
		return NULL;
	
	/* Get Local */
	Local = (I_AllegroSoundLocal_t*)a_Driver->Data;
	
	// Check
	if (!Local)
		return NULL;
	
	return Local->Buffer;
}

/* I_AllegroSD_IsFinished() -- Checks if the buffer is finished playing */
bool_t I_AllegroSD_IsFinished(struct I_SoundDriver_s* const a_Driver)
{
	I_AllegroSoundLocal_t* Local;
	
	/* Check */
	if (!a_Driver)
		return false;
	
	/* Get Local */
	Local = (I_AllegroSoundLocal_t*)a_Driver->Data;
	
	// Check
	if (!Local)
		return;
		
	// Check again
	if (!Local->Stream)
		return false;
	
	/* If there is no buffer pointer, ask Allegro for it */
	if (!Local->Buffer)
		Local->Buffer = get_audio_stream_buffer(Local->Stream);
	
	return !!Local->Buffer;
}

/* I_AllegroSD_WriteOut() -- Done streaming into buffer */
void I_AllegroSD_WriteOut(struct I_SoundDriver_s* const a_Driver)
{
	I_AllegroSoundLocal_t* Local;
	
	/* Check */
	if (!a_Driver)
		return;
	
	/* Get Local */
	Local = (I_AllegroSoundLocal_t*)a_Driver->Data;
	
	// Check
	if (!Local)
		return;
		
	/* Free the stream and set the buffer to NULL */
	free_audio_stream_buffer(Local->Stream);
	Local->Buffer = NULL;
}

/* I_AllegroSD_UnRequest() -- Unrequests a buffer previously obtained */
void I_AllegroSD_UnRequest(struct I_SoundDriver_s* const a_Driver)
{
	I_AllegroSoundLocal_t* Local;
	
	/* Check */
	if (!a_Driver)
		return;
	
	/* Get Local */
	Local = (I_AllegroSoundLocal_t*)a_Driver->Data;
	
	// Check
	if (!Local)
		return;
		
	/* Free the stream */
	// Destroy it
	if (Local->Stream)
	{
		voice_stop(Local->Stream);
		deallocate_voice(Local->Stream);
	}
	
	// Clear out
	Local->Stream = NULL;
	Local->Buffer = NULL;
}

/* I_AllegroSD_GetFreq() -- Get frequency of driver */
uint16_t I_AllegroSD_GetFreq(struct I_SoundDriver_s* const a_Driver)
{
	I_AllegroSoundLocal_t* Local;
	
	/* Check */
	if (!a_Driver)
		return 0;
	
	/* Get Local */
	Local = (I_AllegroSoundLocal_t*)a_Driver->Data;
	
	// Check
	if (!Local)
		return 0;
	
	/* Return frequency */
	return Local->Freq;
}

/* l_AllegroSound -- Allegro sound driver */
static I_SoundDriver_t l_AllegroSoundDriver =
{
	/* Info */
	"Allegro SoundStream",
	"allegrosnd",
	(1 << IST_WAVEFORM),
	127,
	
	/* Functions */
	I_AllegroSD_Init,
	I_AllegroSD_Destroy,
	I_AllegroSD_Success,
	I_AllegroSD_Request,
	I_AllegroSD_Obtain,
	I_AllegroSD_IsFinished,
	I_AllegroSD_WriteOut,
	I_AllegroSD_UnRequest,
	I_AllegroSD_GetFreq,
};

/****************
*** FUNCTIONS ***
****************/

/* I_SoundDriverInit() -- Initializes sound */
bool_t I_SoundDriverInit(void)
{
	/* Add Allegro MIDI Driver */
	if (!I_AddSoundDriver(&l_AllegroSoundDriver))
		CONS_Printf("I_SoundDriverInit: Failed to add Allegro Driver\n");
	
	/* Success */
	return true;
}

