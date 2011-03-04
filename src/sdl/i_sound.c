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
// DESCRIPTION:
//      System interface for sound.

#include <SDL.h>
#include <math.h>

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

int DigFreq[96] =
{
	0, 175, 180, 185, 190, 196, 202, 208, 214, 220, 226, 233, 240, 247, 254, 262,
	269, 277, 285, 294, 302, 311, 320, 330, 339, 349, 359, 370, 381, 392, 403, 415, 
	427, 440, 453, 466, 480, 494, 508, 523, 539, 554, 571, 587, 604, 622, 640, 659, 
	679, 698, 719, 740, 762, 784, 807, 831, 855, 881, 907, 932, 961, 989, 1017, 1047, 
	1078, 1110, 1142, 1176, 1210, 1244, 1282, 1318, 1357, 1397, 1439, 1480, 1524, 
	1570, 1615, 1662, 1712, 1762, 1813, 1864, 1921, 1975, 2036, 2093, 2158, 2218, 
	2286, 2353, 2420, 2491, 2566, 2640
};

typedef struct SoundChannel_s
{
	uint32_t Age;					// Tic of the sound
	
	uint8_t Volume;				// Volume of the sound
	int8_t Balance;				// Balance to the left and right
	int8_t Orientation;			// Balance to the front and back
	int8_t Pitch;					// Pitch of the sound (playback speed maybe?)
	
	mobj_t* LinkedMobj;			// Object the sound is linked to
	
	int32_t SoundID;				// Identification of the playing sound
	void* Data;					// Pointer to the data
	size_t Offset;				// Offset of the data
} SoundChannel_t;

static boolean SoundStarted = false;
static boolean SoundEnabled = false;
static SDL_AudioSpec AudioSpec;

typedef struct SoundCache_s
{
	uint16_t Freq;
	uint8_t Density;
	void* Data;
	size_t End;
} SoundCache_t;

uint16_t OurFreq = 0;
uint8_t OurDensity = 0;
SoundCache_t* SoundCache = NULL;
size_t NumChannels = 0;
size_t NumSpeakers = 0;
size_t NumReservedChannels = 0;
SoundChannel_t* Channels = NULL;
boolean DigitalAnalog = false;
int AnalogWaveform = 0;
boolean NoSDLLock = false;

CV_PossibleValue_t SpeakerSetup_cons_t[] =
{
	{1, "Monaural"},
	{2, "Stereo"},
	{4, "Surround"},
	{6, "Full Surround"},
	
	{0, NULL}
};

consvar_t cv_snd_speakersetup = {"snd_speakersetup", "2", CV_SAVE, SpeakerSetup_cons_t};

CV_PossibleValue_t SoundQuality_cons_t[] =
{
	{5512, "5 KHz"},
	{11025, "11 KHz"},
	{22050, "22 KHz"},
	{44100, "44 KHz"},
	
	{0, NULL}
};

consvar_t cv_snd_soundquality = {"snd_soundquality", "11025", CV_SAVE, SoundQuality_cons_t};

CV_PossibleValue_t SoundDensity_cons_t[] =
{
	{0, "8-bit"},
	{1, "16-bit"},
	
	{0, NULL}
};

consvar_t cv_snd_sounddensity = {"snd_sounddensity", "1", CV_SAVE, SoundDensity_cons_t};

CV_PossibleValue_t PCSpeakerWaveform_cons_t[] =
{
	{0, "Sine"},
	{1, "Square"},
	//{2, "Triangle"},
	{3, "Sawtooth"},
	{4, "Rectangle"},
	{5, "SineSquare"},
	{6, "AbsoluteSine"},
	
	{0, NULL}
};

consvar_t cv_snd_pcspeakerwave = {"snd_pcspeakerwave", "1", CV_SAVE, PCSpeakerWaveform_cons_t};

CV_PossibleValue_t Channels_cons_t[] =
{
	{1, "MIN"},
	{32, "MAX"},
	
	{0, NULL}
};

consvar_t cv_snd_channels = {"snd_numchannels", "16", CV_SAVE, Channels_cons_t};
consvar_t cv_snd_reservedchannels = {"snd_reservedchannels", "4", CV_SAVE, Channels_cons_t};
consvar_t cv_snd_multithreaded = {"snd_multithreaded", "1", CV_SAVE, CV_YesNo};

/*************************
*** HARDWARE STREAMING ***
*************************/

/*** DEFAULT ***/
boolean AD_DEFAULT_InitDriver(void);
boolean AD_DEFAULT_StopDriver(void);
boolean AD_DEFAULT_PostSound(void);
boolean AD_SIMULATEDPCSPEAKER_InitDriver(void);
boolean AD_SIMULATEDPCSPEAKER_StopDriver(void);
boolean AD_SIMULATEDPCSPEAKER_PostSound(void);
boolean AD_ALTSIMULATEDPCSPEAKER_InitDriver(void);
boolean AD_ALTSIMULATEDPCSPEAKER_StopDriver(void);
boolean AD_ALTSIMULATEDPCSPEAKER_PostSound(void);

/**************
*** DRIVERS ***
**************/

typedef enum
{
	AD_NONE,
	AD_DEFAULT,
	AD_SIMULATEDPCSPEAKER,
	AD_ALTSIMULATEDPCSPEAKER,
	
	NUMAUDIODRIVERS
} AUDIODRIVER_e;

typedef struct AUDIODRIVER_s
{
	char Name[32];
	struct AUDIODRIVER_s* Dependency;
	
	boolean (*InitDriver)(void);
	boolean (*StopDriver)(void);
	boolean (*PostSound)(void);
	
	boolean Digital;
	boolean Active;
} AUDIODRIVER_t;

AUDIODRIVER_t AUDIODRIVER_Drivers[NUMAUDIODRIVERS] =
{
	// NONE
	{
		"None",
		NULL,
		NULL,
		NULL,
		NULL,
		false,
		false
	},
	
	// DEFAULT
	{
		"Default",
		NULL,
		AD_DEFAULT_InitDriver,
		AD_DEFAULT_StopDriver,
		AD_DEFAULT_PostSound,
		true,
		false
	},
	
	// SIMULATED PC SPEAKER
	{
		"Simulated PC Speaker",
		&AUDIODRIVER_Drivers[AD_DEFAULT],
		AD_SIMULATEDPCSPEAKER_InitDriver,
		AD_SIMULATEDPCSPEAKER_StopDriver,
		AD_SIMULATEDPCSPEAKER_PostSound,
		false,
		false
	},
	
	// ALTERNATE SIMULATED PC SPEAKER
	{
		"Alt. Simulated PC Speaker",
		&AUDIODRIVER_Drivers[AD_DEFAULT],
		AD_ALTSIMULATEDPCSPEAKER_InitDriver,
		AD_ALTSIMULATEDPCSPEAKER_StopDriver,
		AD_ALTSIMULATEDPCSPEAKER_PostSound,
		false,
		false
	},
};

CV_PossibleValue_t SoundOutput_cons_t[] =
{
	{0, AUDIODRIVER_Drivers[0].Name},
	{1, AUDIODRIVER_Drivers[1].Name},
	{2, AUDIODRIVER_Drivers[2].Name},
	{3, AUDIODRIVER_Drivers[3].Name},
	
	{0, NULL}
};

consvar_t cv_snd_output = {"snd_output", "Default", CV_SAVE, SoundOutput_cons_t};
consvar_t cv_snd_device = {"snd_device", "auto", CV_SAVE};

AUDIODRIVER_t* SelectedAudioDriver = NULL;

/* GetAudioDriver() -- returns an audio driver by name */
AUDIODRIVER_t* GetAudioDriver(char* Driver)
{
	int8_t i;
	
	if (Driver)
		for (i = 0; i < NUMAUDIODRIVERS; i++)
			if (strcasecmp(Driver, AUDIODRIVER_Drivers[i].Name) == 0)
				return &AUDIODRIVER_Drivers[i];
	
	return NULL;
}

/*************************
*** HARDWARE STREAMING ***
*************************/

/*** DEFAULT ***/
void I_UpdateSound_sdl(void *unused, int8_t* stream, int len)
{
	/*static*/ size_t i, j, k;
	/*static*/ size_t NewLen;
	/*static*/ size_t WavLen;
	/*static*/ int8_t* i8;
	/*static*/ int16_t* i16;
	/*static*/ int32_t TData;
	/*static*/ int32_t TData2;
	/*static*/ int8_t* Out = NULL;
	/*static*/ float Mul[6];
	/*static*/ float Vol;
	/*static*/ float T;
	/*static*/ float SysVol;
	
	if (!Channels || !SoundCache)
		return;

	//memset(stream, 0, len);
	
	NewLen = (len - (NumSpeakers * (OurDensity / 8))) / NumSpeakers / (OurDensity / 8);
		
	SysVol = (float)cv_soundvolume.value / 15.0;
		
	for (i = 0; i < NumChannels; i++)
	{
		if (!Channels[i].Data)
			continue;
			
		/* Set Multipliers */
		Vol = ((float)Channels[i].Volume / 256.0) * SysVol;
		// Mono
		if (NumSpeakers == 1)
			Mul[0] = Vol;
		else if (NumSpeakers == 2)
		{
			// Base
			Mul[0] = Mul[1] = 1.0;
			
			/* More left oriented */
			if (Channels[i].Balance < 0)
			{
				T = Channels[i].Balance;
				
				if (T < 0)
				{
					T += 128;
					T /= 128;
					
					Mul[1] *= T;
				}
			}
			
			/* More right oriented */
			else if (Channels[i].Balance > 0)
			{
				T = -Channels[i].Balance;
				
				if (T < 0)
				{
					T += 128;
					T /= 128;
					
					Mul[0] *= T;
				}
			}
			
			Mul[0] *= Vol;
			Mul[1] *= Vol;
		}
		else if (NumSpeakers == 4)
		{
			Mul[0] = Vol;
			Mul[1] = Vol;
			Mul[2] = Vol;
			Mul[3] = Vol;
		}
		else if (NumSpeakers == 6)
		{
			Mul[0] = Vol;
			Mul[1] = Vol;
			Mul[2] = Vol;
			Mul[3] = Vol;
			Mul[4] = Vol;
			Mul[5] = Vol;
		}
		
		/* Reset stream for channel */
		Out = stream;
		
		WavLen = SoundCache[Channels[i].SoundID].End - Channels[i].Offset;
		
		if (WavLen > NewLen)
			WavLen = NewLen;
		
		for (j = 0; j < WavLen && Channels[i].Data; j++)
		{
			for (k = 0; k < NumSpeakers; k++)
			{
				if (OurDensity == 8)
				{
					TData = *Out;
					TData += (*((int8_t*)Channels[i].Data + Channels[i].Offset)) * Mul[k];
				
					if (TData > 127)
						TData = 127;
					else if (TData < -128)
						TData = -128;
					
					*Out = TData;
				}
				else
				{
					TData = *((int16_t*)Out);
					TData += (*((int16_t*)Channels[i].Data + Channels[i].Offset)) * Mul[k];
				
					if (TData > 32767)
						TData = 32767;
					else if (TData < -32768)
						TData = -32768;
					
					*((int16_t*)Out) = TData;
				}
			
				Out += OurDensity >> 3;
			}
			
			// End of channel?
			Channels[i].Offset++;
			
			// Stop playing?
			if (Channels[i].Offset >= SoundCache[Channels[i].SoundID].End)
			{
				Channels[i].Age = 0;
				Channels[i].Volume = 0;
				Channels[i].Balance = 0;
				Channels[i].Orientation = 0;
				Channels[i].Pitch = 0;
				Channels[i].LinkedMobj = NULL;
				Channels[i].SoundID = -1;
				Channels[i].Data = NULL;
				Channels[i].Offset = 0;
				j = 9999999;
				break;
			}
		}
	}
}

boolean AD_DEFAULT_InitDriver(void)
{
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1)
	{
		CONS_Printf("AD_DEFAULT_InitDriver: Failed to initialize SDL (%s).\n", SDL_GetError());
		return false;
	}
	
	// Clear
	memset(&AudioSpec, 0, sizeof(AudioSpec));
	
	// Initialize
	if (cv_snd_sounddensity.value)
		AudioSpec.format = AUDIO_S16;
	else
		AudioSpec.format = AUDIO_S8;
	AudioSpec.freq = cv_snd_soundquality.value;
	AudioSpec.channels = cv_snd_speakersetup.value;
	AudioSpec.callback = I_UpdateSound_sdl;
	
	//// Windows has problems with samples
	// 256 is lowest
	AudioSpec.samples = 256 * AudioSpec.channels * (AudioSpec.freq / 5,512) * (cv_snd_sounddensity.value + 1);
	
	if (M_CheckParm("-nosdlbufferlock"))
		NoSDLLock = true;
	
	if (SDL_OpenAudio(&AudioSpec, NULL))
	{
		CONS_Printf("AD_DEFAULT_InitDriver: Failed to open audio stream (%s).\n", SDL_GetError());
		return false;
	}
	
	SDL_PauseAudio(0);
	
	return true;
}

boolean AD_DEFAULT_StopDriver(void)
{
	SDL_CloseAudio();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	
	return true;
}

boolean AD_DEFAULT_PostSound(void)
{
	return false;
}

/*** SIMULATED PC SPEAKER ***/
boolean AD_SIMULATEDPCSPEAKER_InitDriver(void)
{
	DigitalAnalog = true;
	AnalogWaveform = cv_snd_pcspeakerwave.value;
	return true;
}

boolean AD_SIMULATEDPCSPEAKER_StopDriver(void)
{
	DigitalAnalog = false;
	AnalogWaveform = 0;
	return true;
}

boolean AD_SIMULATEDPCSPEAKER_PostSound(void)
{
	return false;
}

/*** ALTERNATE SIMULATED PC SPEAKER ***/
boolean AD_ALTSIMULATEDPCSPEAKER_InitDriver(void)
{
	if (!AD_SIMULATEDPCSPEAKER_InitDriver())
		return false;
	
	DigitalAnalog = 2;
	return true;
}

boolean AD_ALTSIMULATEDPCSPEAKER_StopDriver(void)
{
	if (!AD_SIMULATEDPCSPEAKER_StopDriver())
		return false;
	
	return true;
}

boolean AD_ALTSIMULATEDPCSPEAKER_PostSound(void)
{
	return false;
}

/*************
*** SYSTEM ***
*************/

void I_SOUND_DestroyDigital(void);
void I_SOUND_DestroyAnalog(void);

void I_SOUND_MakeDigital(void)
{
	int i;
	
	if (Channels)
		I_SOUND_DestroyDigital();
	
	if (!SoundCache)
	{
		SoundCache = Z_Malloc(sizeof(SoundCache_t) * NUMSFX, PU_STATIC, NULL);
		
		// Sound precache?
		if (precachesound.value)
			for (i = 0; i < NUMSFX; i++)
				I_GetSfx(S_sfx[i].link);
	}
	
	// Set channels up
	Channels = Z_Malloc(sizeof(SoundChannel_t) * cv_snd_channels.value, PU_STATIC, NULL);
	NumChannels = cv_snd_channels.value;
	NumReservedChannels = cv_snd_reservedchannels.value;
	
	if (NumReservedChannels >= NumChannels)
		NumReservedChannels = NumChannels - 1;
		
	OurFreq = cv_snd_soundquality.value;
	OurDensity = (cv_snd_sounddensity.value + 1) * 8;
	NumSpeakers = cv_snd_speakersetup.value;
}

void I_SOUND_MakeAnalog(void)
{
}

void I_SOUND_DestroyDigital(void)
{
	int i;
	
	if (Channels)
	{
		Z_Free(Channels);
		Channels = NULL;
		NumChannels = 0;
		NumReservedChannels = 0;
	}
	
	if (SoundCache)
	{
		for (i = 0; i < NUMSFX; i++)
			if (SoundCache[i].Data)
			{
				Z_Free(SoundCache[i].Data);
				SoundCache[i].Data = NULL;
			}
		
		Z_Free(SoundCache);
		SoundCache = NULL;
	}
	
	OurFreq = 0;
	OurDensity = 0;
	NumSpeakers = 0;
}

void I_SOUND_DestroyAnalog(void)
{
}

void I_StartupSound()
{
	int8_t i;
	AUDIODRIVER_t* WantedDriver = NULL;
	int8_t LoadLoop[NUMAUDIODRIVERS];
	
	if (M_CheckParm("-nosound") || M_CheckParm("-nosfx"))
		return;
		
	/* Check which driver we want */
	WantedDriver = GetAudioDriver(cv_snd_output.string);
	
	if (!WantedDriver)
	{
		CONS_Printf("I_StartupSound: Unknown driver!\n");
		return;
	}
	
	/* Is our driver already loaded? */
	if (SelectedAudioDriver == WantedDriver)
	{
		CONS_Printf("I_StartupSound: Driver already loaded!\n");
		return;
	}
	
	/* Does this driver depend on something and it isn't loaded or currently active? */
	if (WantedDriver->Dependency)
		if (SelectedAudioDriver != WantedDriver->Dependency || !WantedDriver->Dependency->Active)
			if (WantedDriver->Dependency->InitDriver)
			{
				if (!(WantedDriver->Dependency->InitDriver()))
				{
					CONS_Printf("I_StartupSound: Dependent driver failed to load!\n");
					return;
				}
				else
				{
					WantedDriver->Dependency->Active = true;
					if (WantedDriver->Dependency->Digital)
						I_SOUND_MakeDigital();
					else
						I_SOUND_MakeAnalog();
				}
			}
			else
			{
				CONS_Printf("I_StartupSound: Dependent driver has no InitDriver function!\n");
				return;
			}
				
	/* Check if there is an init function */
	if (!(WantedDriver->InitDriver))
	{
		CONS_Printf("I_StartupSound: Driver has no InitDriver function!\n");
		return;
	}
	
	/* Now start our driver */
	if (!WantedDriver->InitDriver())
	{
		CONS_Printf("I_StartupSound: Driver failed to load!\n");
		return;
	}
	
	if (WantedDriver->Digital)
		I_SOUND_MakeDigital();
	else
		I_SOUND_MakeAnalog();
	
	WantedDriver->Active = true;
	SelectedAudioDriver = WantedDriver;
	SoundStarted = true;
	SoundEnabled = true;
	
	CONS_Printf("I_StartupSound: Driver loaded successfully!\n");
}

void I_ShutdownSound(void)
{
	size_t i = 0;
	
	if (!SoundStarted)
		return;
	
	/* Is there a selected audio driver? */
	if (!SelectedAudioDriver)
		return;
		
	/* Remove sfxlump and data pointers */
	for (i = 0; i < NUMSFX; i++)
	{
		S_sfx[i].data = NULL;
		S_sfx[i].lumpnum = -1;
	}
		
	/* Does this driver have a dependency? */
	if (SelectedAudioDriver->Dependency)
		if (SelectedAudioDriver->Dependency->Active)
			if (SelectedAudioDriver->Dependency->StopDriver)
			{
				if (!(SelectedAudioDriver->Dependency->StopDriver()))
				{
					CONS_Printf("I_ShutdownSound: Dependent driver failed to stop!\n");
					return;
				}
				else
				{
					SelectedAudioDriver->Dependency->Active = false;
					
					if (SelectedAudioDriver->Dependency->Digital)
						I_SOUND_DestroyDigital();
					else
						I_SOUND_DestroyAnalog();
				}
			}
			else
			{
				CONS_Printf("I_ShutdownSound: Dependent driver has no StopDriver function!\n");
				return;
			}
				
	/* Check if there is a stop function */
	if (!(SelectedAudioDriver->StopDriver))
	{
		CONS_Printf("I_ShutdownSound: Driver has no StopDriver function!\n");
		return;
	}
	
	/* SelectedAudioDriver start our driver */
	if (!SelectedAudioDriver->StopDriver())
	{
		CONS_Printf("I_StartupSound: Driver failed to stop!\n");
		return;
	}
	
	if (SelectedAudioDriver->Digital)
		I_SOUND_DestroyDigital();
	else
		I_SOUND_DestroyAnalog();
	
	SelectedAudioDriver->Active = false;
	SelectedAudioDriver = NULL;
	SoundStarted = false;
	SoundEnabled = false;
	
	CONS_Printf("I_ShutdownSound: Driver unloaded successfully!\n");
}

/*************
*** SOUNDS ***
*************/

boolean I_UpdateSoundCache(void)
{
}

void *I_GetSfx(sfxinfo_t* sfx)
{
	int i, j, k, l;
	int SoundID = -1;
	int DoCache = 0;
	uint16_t InFreq = 0;
	void* InData = NULL;
	size_t InLen = 0;
	size_t InSamples = 0;
	int8_t* y8;
	int16_t* y16;
	int8_t* x8;
	size_t YAdd = 0;
	uint16_t Mask = 0;
	int Multiply = 0;
	int RMultiply = 0;
	int32_t tVal = 0;
	size_t sz = 0;
	size_t AnLen;
	size_t PerSample;
	
	if (!SoundStarted || !SoundEnabled || !sfx || !SoundCache)
		return NULL;
		
	/* We have to find out which sound we are really getting */
	SoundID = sfx - S_sfx;
	
	if (SoundID < 0 || SoundID >= NUMSFX)
		return NULL;
	
	/* Is this sound cached? */
	if (SoundCache[SoundID].Freq)
	{
		// Does not match the output?
		if (SoundCache[SoundID].Freq != OurFreq || SoundCache[SoundID].Density != OurDensity)
		{
			I_FreeSfx(sfx);
			DoCache = 1;
		}
		else
			return sfx->data;
	}
	else		// If not, we cache it
		DoCache = 1;
	
	/* Do we cache? */
	if (DoCache)
	{
		/**** SIMULATED ANALOG ***/
		if (DigitalAnalog)
		{
			/* Find DPxxxxxx lump */
			if (sfx->lumpnum == -1)
			{
				char namebuf[9];
				
				sprintf(namebuf, "dp%s", sfx->name);
				sfx->lumpnum = W_CheckNumForName(namebuf);
				
				// Some other method
				if (sfx->lumpnum == -1)
				{
					sprintf(namebuf, "%s", sfx->name);
					i = strlen(namebuf);
					
					if (i < 7)
						i += 2;
					
					namebuf[i] = 0;
					namebuf[i - 1] = 'p';
					namebuf[i - 2] = '~';
					
					sfx->lumpnum = W_CheckNumForName(namebuf);
				}
			}
			
			InData = W_CacheLumpNum(sfx->lumpnum, PU_CACHE);
			
			if (!InData)
				return NULL;
			
			/* Create Fake sound */
			// Lock
			if (!NoSDLLock)
				SDL_LockAudio();
			
			InLen = W_LumpLength(sfx->lumpnum);
			
			// Get fake len
			AnLen = ((uint16_t*)InData)[1];
			
			if (AnLen > InLen - 4)
				AnLen = InLen - 4;
			
			sz = (size_t)(((float)(((uint16_t*)InData)[1]) / 140.0) * OurFreq) * (OurDensity / 8);
			SoundCache[SoundID].Data = Z_Malloc(sz, PU_CACHE, &SoundCache[SoundID].Data);
			SoundCache[SoundID].End = sz / (OurDensity / 8);
			
			PerSample = (int)((float)OurFreq * (1.0/140.0));
			
			x8 = &(((int16_t*)InData)[3]);
			y8 = SoundCache[SoundID].Data;
			y16 = SoundCache[SoundID].Data;
			
			{
				int PowVal = pow(2, OurDensity - 1) - 1;
				int HalfPowVal = PowVal >> 1;
				double junk;
				
				if (OurDensity == 8)
					Mask = 0x00FF;
				else
					Mask = 0xFFFF;
				
				for (j = 0; j < AnLen * PerSample; j++)
				{
					// Wave
					/*if (DigFreq[*x8] == 0)
						tVal = 0;
					else
					{*/
						if (!AnalogWaveform)
						{
							tVal = sin(
								6.283185308 *		// 2pi
								(float)j *					// Current pos
								((float)DigFreq[*x8] / (float)OurFreq)) *		// Freq
								PowVal;				// Value
						}
						else if (AnalogWaveform == 1)		/* SQUARE */
						{
							tVal = modf(
								j *
								((float)DigFreq[*x8] / (float)OurFreq)
								, &junk) *
								PowVal;
							
							if (tVal > HalfPowVal)
								tVal = PowVal;
							else if (tVal < -(HalfPowVal + 1))
								tVal = -(PowVal + 1);
							else
								tVal = 0;
						}
						//else if (AnalogWaveform == 2)		/* TRIANGLE? */
						//{
						//}
						else if (AnalogWaveform == 3)		/* SAWTOOTH */
						{
							if (DigFreq[*x8] != 0 && ((int)((float)OurFreq / (float)DigFreq[*x8])) != 0)
								tVal =
									2 *
									(j % (int)((float)OurFreq / (float)DigFreq[*x8])) / ((float)OurFreq / (float)DigFreq[*x8]) *
									PowVal;
						}
						else if (AnalogWaveform == 4)		/* RECTANGLE? Maybe not */
						{
							tVal = modf(
								j *
								((float)DigFreq[*x8] / (float)OurFreq)
								, &junk) *
								PowVal;
						
							if (tVal >= (HalfPowVal - (HalfPowVal >> 1)) &&
									tVal < (HalfPowVal + (HalfPowVal >> 1)))
								tVal = HalfPowVal;
							else if (tVal >= (HalfPowVal + (HalfPowVal >> 1)))
								tVal = PowVal;
							else if (tVal <= -(HalfPowVal + 1) + (HalfPowVal >> 1) &&
									tVal > -(HalfPowVal + 1) - (HalfPowVal >> 1))
								tVal = -(HalfPowVal + 1);
							else if (tVal <= -(HalfPowVal + 1) - (HalfPowVal >> 1))
								tVal = -(PowVal + 1);
							else
								tVal = 0;
						}
						else if (AnalogWaveform == 5)			/* SINE SQUARE */
						{
							tVal = sin(
								6.283185308 *		// 2pi
								j *					// Current pos
								((float)DigFreq[*x8] / (float)OurFreq)) *		// Freq
								PowVal;				// Value
							
							if (tVal > HalfPowVal)
								tVal = PowVal;
							else if (tVal < -(HalfPowVal + 1))
								tVal = -(PowVal + 1);
							else
								tVal = 0;
						}
						else if (AnalogWaveform == 6)		/* ABS SINE */
						{
							tVal = abs(sin(
								6.283185308 *		// 2pi
								j *					// Current pos
								((float)DigFreq[*x8] / (float)OurFreq)) *		// Freq
								PowVal);				// Value
						}
					/*}*/
					
					// Cap
					if (tVal > PowVal)
						tVal = PowVal;
					else if (tVal < -(PowVal + 1))
						tVal = -(PowVal + 1);
					
					// Set
					if (OurDensity == 8)
						*y8 = tVal;
					else
						*y16 = tVal;
					
					// Boost y
					y8++;
					y16++;
					
					// Boost x
					if ((j % PerSample) == (PerSample - 1))
						x8++;
				}
			}
			
			// Finalize
			SoundCache[SoundID].Freq = OurFreq;
			SoundCache[SoundID].Density = OurDensity;
		
			// Unlock
			if (!NoSDLLock)
				SDL_UnlockAudio();
		}
		
		/**** DIGITAL ****/
		else
		{
			if (sfx->lumpnum == -1)
				sfx->lumpnum = S_GetSfxLumpNum(sfx);
		
			InData = W_CacheLumpNum(sfx->lumpnum, PU_CACHE);
		
			if (!InData)
				return NULL;
			
			// Be sure the sound stuff isn't running
			if (!NoSDLLock)
				SDL_LockAudio();
		
			InLen = W_LumpLength(sfx->lumpnum);
		
			InFreq = ((uint16_t*)InData)[1];
			InSamples = ((uint16_t*)InData)[2] - 1;
		
			/* Figure out how much space we need to use */
			Multiply = ((float)OurFreq / (float)InFreq);
			RMultiply = ((float)InFreq / (float)OurFreq);
			if (Multiply)
				sz = (InSamples * Multiply) * (OurDensity / 8);
			else
				sz = (InSamples / RMultiply) * (OurDensity / 8);
			SoundCache[SoundID].Data = Z_Malloc(sz, PU_CACHE, &SoundCache[SoundID].Data);
		
			SoundCache[SoundID].End = sz / (OurDensity / 8);
		
			/* Matches doom and stuff, so just copy and paste! kinda... */
			x8 = &(((int16_t*)InData)[4]);
			y8 = SoundCache[SoundID].Data;
			y16 = SoundCache[SoundID].Data;
		
			if (InFreq == OurFreq)
			{
				if (OurDensity == 8)
				{
					for (j = 0; j < SoundCache[SoundID].End; j++)
					{
						tVal = *((uint8_t*)x8);
						tVal -= 128;
						*y8 = tVal;
				
						x8++;
						y8++;
					}
				}
				else
				{
					for (j = 0; j < SoundCache[SoundID].End; j++)
					{
						// I think a site on PCM states that the range was equal to 16 bits instead of 8 so maybe...
						tVal = *((uint8_t*)x8) << 8;
						tVal -= 32768;
						*y16 = tVal;
				
						x8++;
						y16++;
					}
				}
			}
		
			/* Does not match, so we need to convert it */
			else
			{
				/* 8-bit */
				if (OurDensity == 8)
				{
					/* Multiplication */
					if (Multiply)
					{
						for (j = 0; j < SoundCache[SoundID].End; j++)
						{
							tVal = (*((uint8_t*)x8)) - 128;
						
							for (k = 0; k < Multiply; k++)
							{
								*y8 = tVal;
								*y8++;
							}
						
							x8++;
						}
					}
				
					/* Merge */
					else
					{
						for (j = 0; j < SoundCache[SoundID].End; j++)
						{
							tVal = 0;
					
							for (k = 0; k < RMultiply; k++)
							{
								tVal += (*((uint8_t*)x8)) - 128;
								x8++;
							}
					
							*y8 = tVal / RMultiply;
							*y8++;
						}
					}
				}
			
				/* 16-bit */
				else
				{
					/* Multiplication */
					if (Multiply)
					{
						for (j = 0; j < SoundCache[SoundID].End; j++)
						{
							tVal = (*((uint8_t*)x8) << 8) - 32768;
						
							for (k = 0; k < Multiply; k++)
							{
								*y16 = tVal;
								*y16++;
							}
						
							x8++;
						}
					}
				
					/* Merge */
					else
					{
						for (j = 0; j < SoundCache[SoundID].End; j++)
						{
							tVal = 0;
					
							for (k = 0; k < RMultiply; k++)
							{
								tVal += (*((uint8_t*)x8) << 8) - 32768;
								x8++;
							}
					
							*y16 = tVal / RMultiply;
							*y16++;
						}
					}
				}
			}
		
			SoundCache[SoundID].Freq = OurFreq;
			SoundCache[SoundID].Density = OurDensity;
		
			// Unlock
			if (!NoSDLLock)
				SDL_UnlockAudio();
		}
	}
	
	/* Back link data */
	sfx->data = SoundID + 1;	// hopefully only the sound engine uses the data parameter
	
	return sfx->data;
}

void I_FreeSfx(sfxinfo_t* sfx)
{
	int SoundID = -1;
	
	if (!SoundStarted || !SoundEnabled || !sfx || !SoundCache)
		return;
		
	/* We have to find out which sound we are really freeing */
	SoundID = sfx - S_sfx;
	
	if (SoundID < 0 || SoundID >= NUMSFX)
		return;
	
	/* Is this sound cached? */
	if (SoundCache[SoundID].Freq)
	{
		if (!NoSDLLock)
			SDL_LockAudio();
		
		if (SoundCache[SoundID].Data)
			Z_Free(SoundCache[SoundID].Data);
		SoundCache[SoundID].Data = NULL;
		SoundCache[SoundID].Freq = 0;
		SoundCache[SoundID].Density = 0;
		SoundCache[SoundID].End = 0;
		
		if (!NoSDLLock)
			SDL_UnlockAudio();
	}
}

/*************
*** VOLUME ***
*************/

void I_SetSfxVolume(int volume)
{
	CV_SetValue(&cv_soundvolume, volume);
}

/*********************
*** SOUND PLAYBACK ***
*********************/

void I_UpdateSound(void)
{
	if (!SoundStarted || !SoundEnabled)
		return;
}

void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
	if (!SoundStarted || !SoundEnabled || !Channels || handle == -1 || handle >= NumChannels)
		return;
	
	if (DigitalAnalog == 1)
		return;
	
	Channels[handle].Volume = vol;
	Channels[handle].Pitch = ((int32_t)pitch) - 128;
	Channels[handle].Balance = ((int32_t)sep) - 128;
	//Channels[handle].Orientation = ((int32_t)orientation) - 128;
}

int I_StartSoundEx(int id, int vol, int sep, int pitch, int priority, mobj_t * origin, int orientation, int front, int center)
{
	// Static since we call this alot I guess...
	int i = 0;
	int Oldest = -1;
	int ChanToUse = -1;
	static int last = 0;
	
	if (!SoundStarted || !SoundEnabled || !SoundCache || !Channels)
		return -1;
	
	/*** ANALOG ***/
	if (DigitalAnalog == 1)
	{
		ChanToUse = last+1;
		I_StopSound(last);
		last++;
		
		if (last == (NumChannels - NumReservedChannels) - 1)
			last = 0;
		
		if (!NoSDLLock)
			SDL_LockAudio();
		
		Channels[ChanToUse].Age = gametic;
		Channels[ChanToUse].SoundID = id;
		Channels[ChanToUse].Volume = 255;
		Channels[ChanToUse].LinkedMobj = origin;
		Channels[ChanToUse].Pitch = 0;
		Channels[ChanToUse].Balance = 0;
		Channels[ChanToUse].Orientation = 0;
		Channels[ChanToUse].Data = SoundCache[id].Data;
		Channels[ChanToUse].Offset = 0;

		if (!NoSDLLock)	
			SDL_UnlockAudio();
			
		return ChanToUse;
	}
	
	/*** DIGITAL ***/
	else
	{
		// Search sounds
		for (i = 0; i < NumChannels - NumReservedChannels; i++)
		{
			/* Sound already playing by mobj? */
			if (origin && Channels[i].LinkedMobj == origin)
			{
				ChanToUse = i;
				break;
			}
		
			/* Free channel? */
			else if (Channels[i].SoundID == -1)
			{
				ChanToUse = i;
			
				// In case of mobj
				if (!origin)
					break;
			}
		
			/* Unfree? */
			else if (Channels[i].SoundID > -1)
			{
				if (Oldest == -1)
					Oldest = i;
				else if (Channels[i].Age < Channels[Oldest].Age)
					Oldest = i;
			}
		}
	
		/* Replace an old sound? */
		if (ChanToUse == -1 && Oldest > -1)
			ChanToUse = Oldest;
	
		/* Set the channel data */
		if (ChanToUse != -1)
		{
			if (!NoSDLLock)
				SDL_LockAudio();
		
			Channels[ChanToUse].Age = gametic;
			Channels[ChanToUse].SoundID = id;
			Channels[ChanToUse].Volume = vol;
			Channels[ChanToUse].LinkedMobj = origin;
			Channels[ChanToUse].Pitch = ((int32_t)pitch) - 128;
			Channels[ChanToUse].Balance = ((int32_t)sep) - 128;
			Channels[ChanToUse].Orientation = ((int32_t)orientation) - 128;
			Channels[ChanToUse].Data = SoundCache[id].Data;
			Channels[ChanToUse].Offset = 0;
		
			if (!NoSDLLock)
				SDL_UnlockAudio();
		}
		
		return ChanToUse;
	}
}

int I_StartSound(int id, int vol, int sep, int pitch, int priority, mobj_t * origin)
{
	return I_StartSoundEx(id, vol, sep, pitch, priority, origin, 128, 0, 0);
}

void I_StopSound(int handle)
{
	if (!SoundStarted || !SoundEnabled || !Channels || handle == -1 || handle >= NumChannels)
		return;
	
	if (Channels[handle].SoundID > -1)
	{
		if (!NoSDLLock)
			SDL_LockAudio();
		
		Channels[handle].Age = 0;
		Channels[handle].Volume = 0;
		Channels[handle].Balance = 0;
		Channels[handle].Orientation = 0;
		Channels[handle].Pitch = 0;
		Channels[handle].LinkedMobj = NULL;
		Channels[handle].SoundID = -1;
		Channels[handle].Data = NULL;
		Channels[handle].Offset = 0;
		
		if (!NoSDLLock)
			SDL_UnlockAudio();
	}
}

void I_CutOrigonator(void *origin)
{
	if (!SoundStarted || !SoundEnabled)
		return;
}

int I_SoundIsPlaying(int handle)
{
	if (!SoundStarted || !SoundEnabled || handle == -1 || handle >= NumChannels)
		return 0;
		
	if (Channels[handle].SoundID == -1)
		return 0;
	else
		return 1;
}

void I_SubmitSound(void)
{
	if (!SoundStarted || !SoundEnabled)
		return;
}

