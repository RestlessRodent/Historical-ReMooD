// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION:

/***************
*** INCLUDES ***
***************/

/* System */
#include <SDL.h>

/* Local */
#include "doomtype.h"
#include "i_sound.h"
#include "i_util.h"
#include "console.h"

/****************
*** CONSTANTS ***
****************/

#define SDLSLIPCOUNT 2

/*****************
*** STRUCTURES ***
*****************/

/* I_SDLSoundLocal_t -- Local sound stuff */
typedef struct I_SDLSoundLocal_s
{
	int RetVal;
	SDL_AudioSpec Spec;
	void (*ThreadFunc) (const bool_t a_Threaded);
	void* Buffer;
	SDL_mutex* Mutex;
} I_SDLSoundLocal_t;

/**********************************
*** ALLEGRO SOUND STREAM DRIVER ***
**********************************/

/* I_SDLSD_Callback() -- Callback for audio processing */
static void I_SDLSD_Callback(struct I_SoundDriver_s* a_Driver, uint8_t* a_Stream, int a_Length)
{
	I_SDLSoundLocal_t* Local;
	size_t Src;
	
	/* Check */
	if (!a_Driver || !a_Stream || !a_Length)
		return;
		
	/* Get Local */
	Local = (I_SDLSoundLocal_t*) a_Driver->Data;
	
	// Check
	if (!Local)
		return;
		
	/* Set local stuff for threaded operation */
	Local->Buffer = a_Stream;
	Local->ThreadFunc(true);
	Local->Buffer = NULL;
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
	I_SDLSoundLocal_t* Local;
	
	/* Check */
	if (!a_Driver)
		return false;
		
	/* Get Local */
	Local = (I_SDLSoundLocal_t*) a_Driver->Data;
	
	// Check
	if (Local)
	{
		// Destroy mutex
		SDL_DestroyMutex(Local->Mutex);
		Local->Mutex = NULL;
	}
	
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
	Local = (I_SDLSoundLocal_t*) a_Driver->Data;
	
	// Check
	if (!Local)
		return;
		
	/* Create mutex */
	Local->Mutex = SDL_CreateMutex();
}

/* I_SDLSD_Request() -- Requests a buffer for this driver */
size_t I_SDLSD_Request(struct I_SoundDriver_s* const a_Driver, const uint8_t a_Bits, const uint16_t a_Freq, const uint8_t a_Channels, const uint32_t a_Samples)
{
	I_SDLSoundLocal_t* Local;
	
	/* Check */
	if (!a_Driver || !a_Bits || !a_Freq || a_Channels > 2 || !a_Samples)
		return 0;
		
	/* Get Local */
	Local = (I_SDLSoundLocal_t*) a_Driver->Data;
	
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
	
	// It worked but the frequency is zero?
	if (!Local->Spec.freq)
	{
		SDL_CloseAudio();
		return 0;
	}
		
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
	Local = (I_SDLSoundLocal_t*) a_Driver->Data;
	
	// Check
	if (!Local)
		return NULL;
		
	/* Return buffer */
	return Local->Buffer;
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
	Local = (I_SDLSoundLocal_t*) a_Driver->Data;
	
	// Check
	if (!Local)
		return false;
		
	/* Always returns true for threaded enable */
	return true;
}

/* I_SDLSD_WriteOut() -- Done streaming into buffer */
void I_SDLSD_WriteOut(struct I_SoundDriver_s* const a_Driver)
{
	// Does nothing, due to threaded nature
}

/* I_SDLSD_UnRequest() -- Unrequests a buffer previously obtained */
void I_SDLSD_UnRequest(struct I_SoundDriver_s* const a_Driver)
{
	I_SDLSoundLocal_t* Local;
	
	/* Check */
	if (!a_Driver)
		return;
		
	/* Get Local */
	Local = (I_SDLSoundLocal_t*) a_Driver->Data;
	
	// Check
	if (!Local)
		return;
		
	/* Check if RetVal is non-zero (failure) */
	if (Local->RetVal != 0)
		return;
		
	/* Free the stream */
	SDL_CloseAudio();
}

/* I_SDLSD_GetFreq() -- Get frequency of driver */
uint16_t I_SDLSD_GetFreq(struct I_SoundDriver_s* const a_Driver)
{
	I_SDLSoundLocal_t* Local;
	
	/* Check */
	if (!a_Driver)
		return 0;
		
	/* Get Local */
	Local = (I_SDLSoundLocal_t*) a_Driver->Data;
	
	// Check
	if (!Local)
		return 0;
		
	/* Return frequency */
	return Local->Spec.freq;
}

/* I_SDLSD_Thread() -- Thread SDL music */
bool_t I_SDLSD_Thread(struct I_SoundDriver_s* const a_Driver, void (*a_ThreadFunc) (const bool_t a_Threaded))
{
	I_SDLSoundLocal_t* Local;
	
	/* Check */
	if (!a_Driver || !a_ThreadFunc)
		return false;
		
	/* Get Local */
	Local = (I_SDLSoundLocal_t*) a_Driver->Data;
	
	// Check
	if (!Local)
		return false;
		
	/* Set function */
	Local->ThreadFunc = a_ThreadFunc;
	SDL_PauseAudio(0);
	
	/* Success */
	return true;
}

/* I_SDLSD_LockThread() -- Stops the sound thread from running */
void I_SDLSD_LockThread(struct I_SoundDriver_s* const a_Driver, const bool_t a_Lock)
{
	I_SDLSoundLocal_t* Local;
	
	/* Check */
	if (!a_Driver)
		return;
		
	/* Get Local */
	Local = (I_SDLSoundLocal_t*) a_Driver->Data;
	
	// Check
	if (!Local)
		return;
		
	/* Lock or unlock? */
	if (a_Lock)
	{
		//SDL_LockAudio();
		// GhostlyDeath <September 20, 2011> -- Use mutexes instead
		// Can be called from audio update thread
		SDL_LockMutex(Local->Mutex);
	}
	else
	{
		//SDL_UnlockAudio();
		// GhostlyDeath <September 20, 2011> -- Use mutexes instead
		// Can be called from audio update thread
		SDL_UnlockMutex(Local->Mutex);
	}
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
	I_SDLSD_Thread,
	I_SDLSD_LockThread,
};

/****************
*** FUNCTIONS ***
****************/

/* I_SoundDriverInit() -- Initializes sound */
bool_t I_SoundDriverInit(void)
{
	/* Add SDL Sound Driver */
	if (!I_AddSoundDriver(&l_SDLSoundDriver))
		CONL_PrintF("I_SoundDriverInit: Failed to add SDL Driver\n");
		
	/* Success */
	return true;
}
