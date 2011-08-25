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
// Copyright (C) 2011 GhostlyDeath <ghostlydeath@gmail.com>
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

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "doomdef.h"
#include "command.h"
#include "s_sound.h"
#include "z_zone.h"
#include "console.h"
#include "doomstat.h"
#include "sounds.h"
#include "i_util.h"
#include "w_wad.h"

/*****************
*** PROTOTYPES ***
*****************/

void SetChannelsNum(void);
void S_UpdateCVARVolumes(void);

/*****************
*** STRUCTURES ***
*****************/

/* S_SoundChannel_t -- A playing sound in a channel */
typedef struct S_SoundChannel_s
{
	S_NoiseThinker_t* Origin;							// Source sound
	fixed_t Position;									// Stream position
	fixed_t Stop;										// When to stop
	fixed_t MoveRate;									// Rate at which to copy
	fixed_t Volume;										// Volume adjust
	WX_WADEntry_t* Entry;								// WAD entry being played
	void* Data;											// Sound data being played
	int SoundID;										// Sound being played
	bool_t Used;										// Channel is being used
} S_SoundChannel_t;

/**************
*** GLOBALS ***
**************/

consvar_t cv_snd_speakersetup = {"snd_speakersetup", "2", CV_SAVE};
consvar_t cv_snd_soundquality = {"snd_soundquality", "11025", CV_SAVE};
consvar_t cv_snd_sounddensity = {"snd_sounddensity", "1", CV_SAVE};
consvar_t cv_snd_pcspeakerwave = {"snd_pcspeakerwave", "1", CV_SAVE};
consvar_t cv_snd_channels = {"snd_numchannels", "16", CV_SAVE};
consvar_t cv_snd_reservedchannels = {"snd_reservedchannels", "2", CV_SAVE};
consvar_t cv_snd_multithreaded = {"snd_multithreaded", "1", CV_SAVE};
consvar_t cv_snd_output = {"snd_output", "Default", CV_SAVE};
consvar_t cv_snd_device = {"snd_device", "auto", CV_SAVE};

consvar_t stereoreverse = { "stereoreverse", "0", CV_SAVE, CV_OnOff };
consvar_t precachesound = { "precachesound", "0", CV_SAVE, CV_OnOff };
consvar_t cv_rndsoundpitch = { "rndsoundpitch", "Off", CV_SAVE, CV_OnOff };
consvar_t cv_numChannels = { "snd_channels", "16", CV_SAVE | CV_CALL, CV_Unsigned, SetChannelsNum };
consvar_t surround = { "surround", "0", CV_SAVE, CV_OnOff };

CV_PossibleValue_t soundvolume_cons_t[] = { {0, "MIN"}, {31, "MAX"}, {0, NULL} };
consvar_t cv_soundvolume = { "snd_soundvolume", "15", CV_SAVE | CV_CALL, soundvolume_cons_t, S_UpdateCVARVolumes };
consvar_t cv_musicvolume = { "snd_musicvolume", "15", CV_SAVE | CV_CALL, soundvolume_cons_t, S_UpdateCVARVolumes };

/*************
*** LOCALS ***
*************/

static bool_t l_SoundOK = false;					// Did the sound start OK?
static bool_t l_MusicOK = true;						// Same but for Music
static int l_CurrentSong = 0;						// Current playing song handle
static int l_Bits, l_Freq, l_Channels, l_Len;
static S_SoundChannel_t* l_DoomChannels;			// Sound channels
static size_t l_NumDoomChannels;					// Number of possible sound channels

/****************
*** FUNCTIONS ***
****************/

/* S_PlayEntryOnChannel() -- Plays a WAD entry on a channel */
S_SoundChannel_t* S_PlayEntryOnChannel(const uint32_t a_Channel, WX_WADEntry_t* const a_Entry)
{
	uint16_t* p;
	uint16_t Header, Freq, Length;
	
	/* Check */
	if (a_Channel >= l_NumDoomChannels || !a_Entry)
		return NULL;
	
	/* Use entry */
	WX_UseEntry(a_Entry, true);
	
	/* Set channel info */
	l_DoomChannels[a_Channel].Entry = a_Entry;
	l_DoomChannels[a_Channel].Data = WX_CacheEntry(a_Entry);
	l_DoomChannels[a_Channel].Used = true;
	l_DoomChannels[a_Channel].Position = 8 << FRACBITS;
	
	/* Read basic stuff */
	p = l_DoomChannels[a_Channel].Data;
	Header = ReadUInt16(&p);
	Freq = ReadUInt16(&p);
	Length = ReadUInt16(&p);
	
	// Set basic stuff
	l_DoomChannels[a_Channel].Stop = l_DoomChannels[a_Channel].Position + ((fixed_t)Length << FRACBITS);
	
	// Determine the play rate, which is by default the ratio of the sound freq and the card freq
	l_DoomChannels[a_Channel].MoveRate = FixedDiv((fixed_t)Freq << FRACBITS, (fixed_t)l_Freq << FRACBITS);
	fprintf(stderr, "%i %i %i (%i) %f\n", Header, Freq, Length, l_Freq, FIXED_TO_FLOAT(l_DoomChannels[a_Channel].MoveRate));
	
	/* Return channel */
	return &l_DoomChannels[a_Channel];
}

/* S_StopChannel() -- Stop channel from playing */
void S_StopChannel(const uint32_t a_Channel)
{
	/* Check */
	if (!l_SoundOK)
		return;
	
	/* Check */
	if (a_Channel >= l_NumDoomChannels)
		return;
	
	/* Clear channel out */
	// Check for entry
	if (l_DoomChannels[a_Channel].Entry)
		WX_UseEntry(l_DoomChannels[a_Channel].Entry, false);
	
	// Clear stuff
	l_DoomChannels[a_Channel].Origin = NULL;
	l_DoomChannels[a_Channel].MoveRate = 1 << FRACBITS;
	l_DoomChannels[a_Channel].Volume = 1 << FRACBITS;
	l_DoomChannels[a_Channel].Entry = NULL;
	l_DoomChannels[a_Channel].Data = NULL;
	l_DoomChannels[a_Channel].SoundID = 0;
	l_DoomChannels[a_Channel].Used = false;
	l_DoomChannels[a_Channel].Position = 0;
}

/* S_SoundPlaying() -- Checks whether a sound is being played by the object */
// Returns the current channel + 1
int S_SoundPlaying(S_NoiseThinker_t* a_Origin, int id)
{
	size_t i;
	
	/* Check */
	if (!l_SoundOK)
		return;
	
	/* Origin is set */
	// Return channel that mobj is emitting on
	if (a_Origin)
	{
		// Find it
		for (i = 0; i < l_NumDoomChannels; i++)
			if (l_DoomChannels[i].Origin == a_Origin)
				return i + 1;
	}
	
	/* Origin is not set */
	else
	{
		for (i = 0; i < l_NumDoomChannels; i++)
			if (!l_DoomChannels[i].Origin && l_DoomChannels[i].SoundID == id)
				return i + 1;
	}
	
	/* No sound found */
	return 0;
}

/* S_StartSoundAtVolume() -- Starts playing a sound */
void S_StartSoundAtVolume(S_NoiseThinker_t* a_Origin, int sound_id, int volume)
{
#define BUFSIZE 24
	char Buf[BUFSIZE];
	int OnChannel, i;
	WX_WADEntry_t* Entry;
	S_SoundChannel_t* Target;
	
	/* Check */
	if (!l_SoundOK)
		return;
	
	/* Check if sound is already on a channel */
	OnChannel = S_SoundPlaying(a_Origin, sound_id);
	
	// Not playing on a channel
	if (!OnChannel)
	{
		// Find first free channel
		for (OnChannel = 0; OnChannel < l_NumDoomChannels; OnChannel++)
			if (!l_DoomChannels[OnChannel].Used)
				break;
		
		// No channel found
		if (OnChannel == l_NumDoomChannels)
		{
			// Choose a random channel
			OnChannel = M_Random() % l_NumDoomChannels;
			
			// Stop sound on this channel
			S_StopChannel(OnChannel);
		}
	}
	else
		OnChannel--;
	
	/* Obtain entry then play on said channel */
	// Prefix with ds
	snprintf(Buf, BUFSIZE, "ds%.6s", S_sfx[sound_id].name);
	Entry = WX_EntryForName(NULL, Buf, false);
		
	// Try direct name
	if (!Entry)
		Entry = WX_EntryForName(NULL, S_sfx[sound_id].name, false);
	Target = S_PlayEntryOnChannel(OnChannel, Entry);
	
	// Failed?
	if (!Target)
		return;
	
	/* Set extra stuff */
	Target->Origin = a_Origin;
	Target->SoundID = sound_id;
	Target->MoveRate = 1 << FRACBITS;
	Target->Volume = 1 << FRACBITS;
#undef BUFSIZE
}

/* S_StartSound() -- Play a sound at full volume */
void S_StartSound(S_NoiseThinker_t* a_Origin, int sound_id)
{
	/* Check */
	if (!l_SoundOK)
		return;
	
	/* Just call other function */
	S_StartSoundAtVolume(a_Origin, sound_id, 255);
}

/* S_StartSoundName() -- Start a sound based on name */
void S_StartSoundName(S_NoiseThinker_t* a_Origin, char *soundname)
{
	/* Check */
	if (!l_SoundOK)
		return;
}

/* S_StopSound() -- Stop sound being played by this object */
void S_StopSound(S_NoiseThinker_t* a_Origin)
{
	/* Check */
	if (!l_SoundOK)
		return;
	
	
}

int S_AdjustSoundParamsEx(struct mobj_s* Listener, struct mobj_s* Source,
	int32_t* Volume,		// Volume of the sound (Distance) [Mono]
	int32_t* Balance,		// Balance of the sound (left/right) [Stereo + Surround + Full Surround]
	int32_t* Pitch,		// Change in pitch (Doppler!?!?) [All]
	int32_t* Orientation,	// Balance of the sound (front/back) [Surround + Full Surround]
	int32_t* FrontVolume	// How loud to play a sound for the front speaker [Full Surround]
	)
{
	/* Check */
	if (!l_SoundOK)
		return -1;
	
	return -1;
}

/* S_RepositionSounds() -- Repositions all sounds */
void S_RepositionSounds(void)
{
	/* Check */
	if (!l_SoundOK)
		return;
}

void SetChannelsNum(void)
{
}

/* S_RegisterSoundStuff() -- Register the sound console variables */
void S_RegisterSoundStuff(void)
{
	static bool_t cvRegged = false;
	
	/* Check */
	if (cvRegged)
		return;
	
	/* Register Variables */
	CV_RegisterVar(&cv_snd_speakersetup);
	CV_RegisterVar(&cv_snd_soundquality);
	CV_RegisterVar(&cv_snd_sounddensity);
	CV_RegisterVar(&cv_snd_pcspeakerwave);
	CV_RegisterVar(&cv_snd_channels);
	CV_RegisterVar(&cv_snd_reservedchannels);
	CV_RegisterVar(&cv_snd_multithreaded);
	CV_RegisterVar(&cv_snd_output);
	CV_RegisterVar(&cv_snd_device);
	
	// Everything was registered
	cvRegged = true;
}

/* S_Init() -- Initializes the sound subsystem */
void S_Init(int sfxVolume, int musicVolume)
{
	/* Always register sound stuff */
	// So the menu doesn't crash on us
	S_RegisterSoundStuff();
	
	// No sound at all?
	if (M_CheckParm("-nosound"))
		return;
	
	/* Initialize both sound and music */
	// Sound
	l_SoundOK = false;
	if (!M_CheckParm("-nosfx"))
		if (I_StartupSound())
			l_SoundOK = true;
	
	// Music
	l_MusicOK = false;
	if (!M_CheckParm("-nomusic"))
		if (I_InitMusic())
			l_MusicOK = true;
	
	// Set volumes based on CVARs
	S_UpdateCVARVolumes();
	
	/* Try getting a buffer */
	if (l_SoundOK)
	{
		if (!I_SoundBufferRequest(IST_WAVEFORM, cv_snd_sounddensity.value * 8, cv_snd_soundquality.value, cv_snd_speakersetup.value, 512))
		{
			l_Bits = l_Freq = l_Channels = l_Len = 0;
			CONS_Printf("S_Init: Failed to obtain a sound buffer.\n");
		}
		
		// Setup buffer
		else
		{
			// Remember settings
			l_Bits = cv_snd_sounddensity.value * 8;
			l_Freq = I_SoundGetFreq();
			l_Channels = cv_snd_speakersetup.value;
			l_Len = 512;
			
			// Frequency did not match
			if (l_Freq != cv_snd_soundquality.value)
				CONS_Printf("S_Init: Requested %iHz but got %iHz\n", cv_snd_soundquality.value, l_Freq);
			
			// Create channels
			l_NumDoomChannels = cv_numChannels.value;
			
			if (!l_NumDoomChannels)
				l_NumDoomChannels = 1;
			
			l_DoomChannels = Z_Malloc(sizeof(*l_DoomChannels) * l_NumDoomChannels, PU_STATIC, NULL);
		}
	}
}

void S_StopSounds(void)
{
}

/* S_Start() -- Change song based on level */
// YUCK! When RMOD comes fix level music stuff
void S_Start(void)
{
	int mnum;
	
	if (gamemode == commercial)
		mnum = mus_runnin + gamemap - 1;
	else
	{
		int spmus[] = {
			// Song - Who? - Where?

			mus_e3m4,			// American     e4m1
			mus_e3m2,			// Romero       e4m2
			mus_e3m3,			// Shawn        e4m3
			mus_e1m5,			// American     e4m4
			mus_e2m7,			// Tim  e4m5
			mus_e2m4,			// Romero       e4m6
			mus_e2m6,			// J.Anderson   e4m7 CHIRON.WAD
			mus_e2m5,			// Shawn        e4m8
			mus_e1m9			// Tim          e4m9
		};

		if (gameepisode < 4)
			mnum = mus_e1m1 + (gameepisode - 1) * 9 + gamemap - 1;
		else
			mnum = spmus[gamemap - 1];
	}

	// HACK FOR COMMERCIAL
	//  if (commercial && mnum > mus_e3m9)
	//      mnum -= mus_e3m9;

	/*if (info_music && *info_music)
		S_ChangeMusicName(info_music, true);
	else*/
		S_ChangeMusic(mnum, true);

	//nextcleanup = 15;
}

/* S_ChangeMusic() -- Changes the current song that is playing */
void S_ChangeMusic(int music_num, int looping)
{
	/* Check */
	if (!l_MusicOK || music_num < 0 || music_num >= NUMMUSIC)
		return;
	
	/* Short circuit to change music name */
	S_ChangeMusicName(S_music[music_num].name, looping);
}

/* S_ChangeMusicName() -- Change song by its name */
void S_ChangeMusicName(char *name, int looping)
{
#define BUFSIZE 12
	char NameBuf[BUFSIZE];
	
	/* Check */
	if (!l_MusicOK || !name)
		return;
	
	/* Prepend the D_ prefix */
	snprintf(NameBuf, BUFSIZE, "D_%s", name);
	C_strupr(NameBuf);
	
	/* If a song is already playing */
	if (l_CurrentSong)
		S_StopMusic();
	
	/* Call the interface */
	l_CurrentSong = I_RegisterSong(NameBuf);
	
	// Failed?
	if (!l_CurrentSong)
		return;
	
	/* Start playing the song */
	I_PlaySong(l_CurrentSong, looping);
	
	// Change volume (in case of new driver, volume will be lost)
	S_UpdateCVARVolumes();
#undef BUFSIZE
}

/* S_StopMusic() -- Stops playing music */
void S_StopMusic(void)
{
	/* Check */
	if (!l_MusicOK || !l_CurrentSong)
		return;
	
	/* Call interface code */
	I_StopSong(l_CurrentSong);
	I_UnRegisterSong(l_CurrentSong);
	l_CurrentSong = 0;
}

/* S_PauseMusic() -- Pause playing music */
void S_PauseMusic(void)
{
	/* Check */
	if (!l_MusicOK || !l_CurrentSong)
		return;
	
	/* Call interface code */
	I_PauseSong(l_CurrentSong);
}

/* S_ResumeMusic() -- Resumes paused music */
void S_ResumeMusic(void)
{
	/* Check */
	if (!l_MusicOK || !l_CurrentSong)
		return;
	
	/* Call interface code */
	I_ResumeSong(l_CurrentSong);
}

/* S_WriteMixSample() -- Writes a single sample */
static void S_WriteMixSample(void** Buf, uint8_t Value)
{
	int32_t v, s;
	
	/* Get shift */
	s = (int32_t)Value - 127;
	
	/* Write Wide */
	if (l_Bits == 16)
	{
		// Read current value (I hope the buffer is aligned!)
#if defined(__REMOOD_SIGNEDSOUND)
		v = **((int16_t**)Buf);
		v = (v) + (s * 256);
		
		if (v > 32767)
			v = 32767;
		else if (v < -32768)
			v = -32768;
		
		// Mix into buffer
		WriteInt16(Buf, v);
#else
		v = **((uint16_t**)Buf);
		v = (v - 32768) + (s * 256);
		
		if (v > 32767)
			v = 32767;
		else if (v < -32768)
			v = -32768;
		v += 32768;
		
		// Mix into buffer
		WriteUInt16(Buf, v);
#endif
	}
	
	/* Write Narrow */
	else
	{
#if defined(__REMOOD_SIGNEDSOUND)
		v = **((int8_t**)Buf);
		v = (v) + (s);
		
		if (v > 127)
			v = 127;
		else if (v < -128)
			v = -128;
		
		// Mix into buffer
		WriteInt8(Buf, v);
#else
		// Read current value and add to it
		v = **((uint8_t**)Buf);
		v = (v - 128) + s;
		
		if (v > 127)
			v = 127;
		else if (v < -128)
			v = -128;
		v += 128;
		
		// Mix into buffer
		WriteUInt8(Buf, v);
#endif
	}
}

/* S_UpdateSounds() -- Updates all playing sounds */
void S_UpdateSounds(void)
{
	void* SoundBuf, *p, *End;
	size_t SoundLen, i, j;
	uint8_t ReadSample;
	
	/* Check */
	if (!l_Bits || !l_SoundOK)
		return;
	
	/* Is the buffer finished? */
	if (!I_SoundBufferIsFinished())
		return;
	
	/* Update all playing sounds */
	S_RepositionSounds();
	
	/* Obtain Buffer */
	SoundBuf = I_SoundBufferObtain();
	SoundLen = l_Len * (l_Bits / 8) * l_Channels;
	End = (uint8_t*)SoundBuf + SoundLen;
	
	// Check
	if (!SoundBuf)
		return;
	
	/* Clear buffer completely */
	memset(SoundBuf, 0, SoundLen);
	
	/* Write Sound Data */
	for (i = 0; i < l_NumDoomChannels; i++)
	{
		// Only play channels being used
		if (!l_DoomChannels[i].Used)
			continue;
		
		// Set p to start of buffer
		p = SoundBuf;
		
		// Keep reading and mixing
		for (; p < End && l_DoomChannels[i].Position < l_DoomChannels[i].Stop; l_DoomChannels[i].Position += l_DoomChannels[i].MoveRate)
		{
			// Read sample bit from data
			ReadSample = ((uint8_t*)l_DoomChannels[i].Data)[l_DoomChannels[i].Position >> FRACBITS];
			
			// Write in
			for (j = 0; j < l_Channels; j++)
				S_WriteMixSample(&p, ReadSample);
		}
		
		// Did the sound stop?
		if (l_DoomChannels[i].Position >= l_DoomChannels[i].Stop)
			S_StopChannel(i);
	}
	
#if 0
S_NoiseThinker_t* Origin;							// Source sound
fixed_t Position;									// Stream position
fixed_t Stop;										// When to stop
fixed_t MoveRate;									// Rate at which to copy
fixed_t Volume;										// Volume adjust
WX_WADEntry_t* Entry;								// WAD entry being played
void* Data;											// Sound data being played
int SoundID;										// Sound being played
bool_t Used;										// Channel is being used
#endif
	
	/*for (i = 0; i < l_Len >> 1; i++)
		S_WriteSample(&p, 65535);
	for (i = 0; i < l_Len; i++)
		S_WriteSample(&p, 0);*/
	
	/* Write to driver */
	I_SoundBufferWriteOut();
}

/* S_SetMusicVolume() -- Sets music volume */
void S_SetMusicVolume(int volume)
{
	/* Set variable */
	if (l_MusicOK)
		CV_SetValue(&cv_musicvolume, volume);
}

/* S_SetSfxVolume() -- Sets sound volume */
void S_SetSfxVolume(int volume)
{
	/* Set variable */
	if (l_SoundOK)
		CV_SetValue(&cv_soundvolume, volume);
}

/* S_UpdateCVARVolumes() -- CVAR Volumes changed */
void S_UpdateCVARVolumes(void)
{
	/* Send values to interfaces */
	// Music
	if (l_MusicOK)
		I_SetMusicVolume(cv_musicvolume.value * 8);
	
	// Sound is not sent because it is dynamically modified by ReMooD.
	// So instead of messing with mixer values, I can just lower the amplitude
	// of sounds being mixed.
}

void Command_SoundReset_f(void)
{
}

void S_FreeSfx(sfxinfo_t * sfx)
{
}


#if 0
#include "doomdef.h"
#include "doomstat.h"
#include "command.h"
#include "g_game.h"
#include "m_argv.h"
#include "r_main.h"				//R_PointToAngle2() used to calc stereo sep.
#include "r_things.h"						// for skins
#include "p_info.h"

#include "i_sound.h"
#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"
#include "d_main.h"

#include "m_random.h"
#include "m_menu.h"

void Command_SoundReset_f(void)
{
	I_ShutdownSound();
	I_StartupSound();
}

#if defined (_WIN32) && !defined (SURROUND)
#define SURROUND
#endif

#ifdef __MACOS__
consvar_t play_mode = { "play_mode", "0", CV_SAVE, CV_Unsigned };
#endif

void I_StartFMODSong(char *musicname, int looping);

consvar_t stereoreverse = { "stereoreverse", "0", CV_SAVE, CV_OnOff };
consvar_t precachesound = { "precachesound", "0", CV_SAVE, CV_OnOff };
CV_PossibleValue_t soundvolume_cons_t[] = { {0, "MIN"}, {31, "MAX"}, {0, NULL} };
consvar_t cv_soundvolume = { "soundvolume", "15", CV_SAVE, soundvolume_cons_t };
consvar_t cv_musicvolume = { "musicvolume", "15", CV_SAVE, soundvolume_cons_t };
consvar_t cv_rndsoundpitch = { "rndsoundpitch", "Off", CV_SAVE, CV_OnOff };
void SetChannelsNum(void);
consvar_t cv_numChannels = { "snd_channels", "16", CV_SAVE | CV_CALL, CV_Unsigned, SetChannelsNum };
consvar_t surround = { "surround", "0", CV_SAVE, CV_OnOff };

#define S_MAX_VOLUME            127

// when to clip out sounds
// Does not fit the large outdoor areas.
// added 2-2-98 in 8 bit volume control (befort  (1200*0x10000))
#define S_CLIPPING_DIST         (1200*0x10000)

// Distance tp origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// Originally: (200*0x10000).
// added 2-2-98 in 8 bit volume control (befort  (160*0x10000))
#define S_CLOSE_DIST            (160*0x10000)

// added 2-2-98 in 8 bit volume control (befort  remove the +4)
#define S_ATTENUATOR            ((S_CLIPPING_DIST-S_CLOSE_DIST)>>(FRACBITS+4))

// Adjustable by menu.
#define NORM_VOLUME             snd_MaxVolume

#define NORM_PITCH              128
#define NORM_PRIORITY           64
#define NORM_SEP                128

#define S_PITCH_PERTURB         1
#define S_STEREO_SWING          (96*0x10000)

#ifdef SURROUND
#define SURROUND_SEP            -128
#endif

// percent attenuation from front to back
#define S_IFRACVOL              30

typedef struct
{
	// sound information (if null, channel avail.)
	sfxinfo_t *sfxinfo;

	// origin of sound
	void *origin;

	// handle of the sound being played
	int handle;

} channel_t;

// the set of channels available
static channel_t *channels;

// whether songs are mus_paused
static bool_t mus_paused;

// music currently being played
static musicinfo_t *mus_playing = 0;

static int nextcleanup;

//
// Internals.
//
int S_getChannel(void *origin, sfxinfo_t * sfxinfo);

int S_AdjustSoundParams(mobj_t * listener, mobj_t * source, int *vol, int *sep, int *pitch);

static void S_StopChannel(int cnum);

void S_RegisterSoundStuff(void)
{
	if (dedicated)
		return;

	//added:11-04-98: stereoreverse
	CV_RegisterVar(&stereoreverse);
	CV_RegisterVar(&precachesound);

#ifdef SURROUND
	CV_RegisterVar(&surround);
#endif

#ifdef __MACOS__				//mp3 playlist stuff
	{
		int i;
		for (i = 0; i < PLAYLIST_LENGTH; i++)
		{
			user_songs[i].name = malloc(7);
			sprintf(user_songs[i].name, "song%i%i", i / 10, i % 10);
			user_songs[i].defaultvalue = malloc(1);
			*user_songs[i].defaultvalue = 0;
			user_songs[i].flags = CV_SAVE;
			user_songs[i].PossibleValue = NULL;
			CV_RegisterVar(&user_songs[i]);
		}
		CV_RegisterVar(&play_mode);
	}
#endif
}

void SetChannelsNum(void)
{
	int i;

	// Allocating the internal channels for mixing
	// (the maximum number of sounds rendered
	// simultaneously) within zone memory.
	if (channels)
		Z_Free(channels);

	channels = (channel_t *) Z_Malloc(cv_numChannels.value * sizeof(channel_t), PU_STATIC, 0);

	// Free all channels for use
	for (i = 0; i < cv_numChannels.value; i++)
		channels[i].sfxinfo = 0;

}

void S_InitRuntimeMusic()
{
	int i;

	for (i = mus_firstfreeslot; i < mus_lastfreeslot; i++)
		S_music[i].name = NULL;
}

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init(int sfxVolume, int musicVolume)
{
	int i;

	if (dedicated)
		return;

	//CONS_Printf( "S_Init: default sfx volume %d\n", sfxVolume);

	S_SetSfxVolume(sfxVolume);
	S_SetMusicVolume(musicVolume);

	SetChannelsNum();

	// no sounds are playing, and they are not mus_paused
	mus_paused = 0;

	// Note that sounds have not been cached (yet).
	for (i = 1; i < NUMSFX; i++)
		S_sfx[i].lumpnum = S_sfx[i].usefulness = -1;	// for I_GetSfx()

	//
	//  precache sounds if requested by cmdline, or precachesound var true
	//
	if (!nosound && (M_CheckParm("-precachesound") || precachesound.value))
	{
		// Initialize external data (all sounds) at start, keep static.
		CONS_Printf("Loading sounds... ");

		for (i = 1; i < NUMSFX; i++)
		{
			// NOTE: linked sounds use the link's data at StartSound time
			if (S_sfx[i].name && !S_sfx[i].link)
				S_sfx[i].data = I_GetSfx(&S_sfx[i]);

		}

		CONS_Printf(" pre-cached all sound data\n");
	}

	S_InitRuntimeMusic();
}

//  Retrieve the lump number of sfx
//
int S_GetSfxLumpNum(sfxinfo_t * sfx)
{
	char namebuf[9];
	int sfxlump;

	sprintf(namebuf, "ds%s", sfx->name);

	sfxlump = W_CheckNumForName(namebuf);
	if (sfxlump > 0)
	{
		sfx->lumpnum = sfxlump;
		return sfxlump;
	}
	
	sprintf(namebuf, "%s", sfx->name);;

	sfxlump = W_CheckNumForName(namebuf);
	
	if (sfxlump > 0)
	{
		sfx->lumpnum = sfxlump;
		return sfxlump;
	}

	sfx->lumpnum = W_GetNumForName("dspistol");
	
	return sfx->lumpnum;
}

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//

//SoM: Stop all sounds, load level info, THEN start sounds.
void S_StopSounds()
{
	int cnum;

	// kill all playing sounds at start of level
	//  (trust me - a good idea)
	for (cnum = 0; cnum < cv_numChannels.value; cnum++)
		if (channels[cnum].sfxinfo)
			S_StopChannel(cnum);
}

void S_Start(void)
{
	int mnum;

	// start new music for the level
	mus_paused = 0;

	if (gamemode == commercial)
		mnum = mus_runnin + gamemap - 1;
	else
	{
		int spmus[] = {
			// Song - Who? - Where?

			mus_e3m4,			// American     e4m1
			mus_e3m2,			// Romero       e4m2
			mus_e3m3,			// Shawn        e4m3
			mus_e1m5,			// American     e4m4
			mus_e2m7,			// Tim  e4m5
			mus_e2m4,			// Romero       e4m6
			mus_e2m6,			// J.Anderson   e4m7 CHIRON.WAD
			mus_e2m5,			// Shawn        e4m8
			mus_e1m9			// Tim          e4m9
		};

		if (gameepisode < 4)
			mnum = mus_e1m1 + (gameepisode - 1) * 9 + gamemap - 1;
		else
			mnum = spmus[gamemap - 1];
	}

	// HACK FOR COMMERCIAL
	//  if (commercial && mnum > mus_e3m9)
	//      mnum -= mus_e3m9;

	if (info_music && *info_music)
		S_ChangeMusicName(info_music, true);
	else
		S_ChangeMusic(mnum, true);

	nextcleanup = 15;
}

void S_StartSoundAtVolume(void *origin_p, int sfx_id, int volume)
{

	int sep;
	int pitch;
	int priority;
	int orientation;
	int front = 0;
	int center;
	sfxinfo_t *sfx;
	int cnum;
	int i;

	mobj_t *origin = (mobj_t *) origin_p;

	if (nosound || (origin && origin->type == MT_SPIRIT))
		return;

	// Debug.
	/*
	   fprintf( stderr,
	   "S_StartSoundAtVolume: playing sound %d (%s)\n",
	   sfx_id, S_sfx[sfx_id].name );
	 */

#ifdef PARANOIA
	// check for bogus sound #
	if (sfx_id < 1 || sfx_id > NUMSFX)
		I_Error("Bad sfx #: %d\n", sfx_id);
#endif

	sfx = &S_sfx[sfx_id];

	if (sfx->skinsound != -1 && origin && origin->skin)
	{
		// it redirect player sound to the sound in the skin table
		sfx_id = ((skin_t *) origin->skin)->soundsid[sfx->skinsound];
		sfx = &S_sfx[sfx_id];
	}

	// Initialize sound parameters
	if (sfx->link)
	{
		pitch = sfx->pitch;
		priority = sfx->priority;
		volume += sfx->volume;

		if (volume < 1)
			return;

		// added 2-2-98 SfxVolume is now the hardware volume, don't mix up
		//    if (volume > SfxVolume)
		//      volume = SfxVolume;
	}
	else
	{
		pitch = NORM_PITCH;
		priority = NORM_PRIORITY;
	}

	// Check to see if it is audible,
	//  and if not, modify the params

	//added:16-01-98:changed consoleplayer to displayplayer
	if (origin)
	{
		int breakout = 0;
		int rc, rc2;
		int rcx[MAXSPLITSCREENPLAYERS];
		int volumex[MAXSPLITSCREENPLAYERS];
		int sepx[MAXSPLITSCREENPLAYERS];
		int pitchx[MAXSPLITSCREENPLAYERS];
		int orientationx[MAXSPLITSCREENPLAYERS];
		int frontx[MAXSPLITSCREENPLAYERS];
		int centerx[MAXSPLITSCREENPLAYERS];
		int volume2 = volume, sep2 /*=sep*/ , pitch2 = pitch;
		
		//for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
		//{
			memset(rcx, 0, sizeof(rcx));
			memset(volumex, 0, sizeof(volumex));
			memset(sepx, 0, sizeof(sepx));
			memset(pitchx, 0, sizeof(pitchx));
			memset(orientationx, 0, sizeof(orientationx));
			memset(frontx, 0, sizeof(frontx));
			memset(centerx, 0, sizeof(frontx));
		//}
		
		for (i = 0; i < cv_splitscreen.value+1; i++)
			if (origin == &players[displayplayer[i]].mo)
				breakout = 1;
				
		if (!breakout)
		{
			for (i = 0; i < cv_splitscreen.value+1; i++)
				if (playeringame[displayplayer[i]])
					rcx[i] = S_AdjustSoundParamsEx(
						players[displayplayer[i]].mo, origin, &volumex[i], &sepx[i], &pitchx[i],
						&orientationx[i], &frontx[i]/*, &centerx[i]*/
						);
				else
					rcx[i] = 0;
			
			for (i = cv_splitscreen.value; i >= 0; i--)
				if (!rcx[i] && i == 0)
					return;
				else if (!rcx[i])
					volumex[i] = -1;
				
			// Whoever hears the sound the loudest makes it played from their POV
			for (i = 0; i < cv_splitscreen.value+1; i++)
				if (volumex[i] > volume)
				{
					volume = volumex[i];
					sep = sepx[i];
					pitch = pitchx[i];
					orientation = orientationx[i];
					center = centerx[i];
					if (playeringame[displayplayer[i]])
						if (origin->x == players[displayplayer[i]].mo->x &&
							origin->y == players[displayplayer[i]].mo->y)
							sep = NORM_SEP;
				}
				
			for (i = 0; i < cv_splitscreen.value+1; i++)
				if (playeringame[displayplayer[i]])
					if (origin->x == players[displayplayer[i]].mo->x &&
						origin->y == players[displayplayer[i]].mo->y)
						sep = NORM_SEP;
		}
		else
			sep = NORM_SEP;
	}
	else
		sep = NORM_SEP;

	// hacks to vary the sfx pitches

	//added:16-02-98: removed by Fab, because it used M_Random() and it
	//                was a big bug, and then it doesnt change anything
	//                dont hear any diff. maybe I'll put it back later
	//                but of course not using M_Random().
	//added 16-08-02: added back by Judgecutor
	//Sound pitching for both Doom and Heretic
	if (cv_rndsoundpitch.value)
	{
		if (sfx_id >= sfx_sawup && sfx_id <= sfx_sawhit)
			pitch += 8 - (M_Random() & 15);
		else if (sfx_id != sfx_itemup && sfx_id != sfx_tink)
			pitch += 16 - (M_Random() & 31);
	}

	if (pitch < 0)
		pitch = NORM_PITCH;
	if (pitch > 255)
		pitch = 255;

	// kill old sound
	S_StopSound(origin);

	// try to find a channel
	cnum = S_getChannel(origin, sfx);

	if (cnum < 0)
		return;

	//
	// This is supposed to handle the loading/caching.
	// For some odd reason, the caching is done nearly
	//  each time the sound is needed?
	//

	// cache data if necessary
	// NOTE : set sfx->data NULL sfx->lump -1 to force a reload
	if (sfx->link)
		sfx->data = sfx->link->data;

	if (!sfx->data)
	{
		//CONS_Printf ("cached sound %s\n", sfx->name);
		if (!sfx->link)
			sfx->data = I_GetSfx(sfx);
		else
		{
			sfx->data = I_GetSfx(sfx->link);
			sfx->link->data = sfx->data;
		}
	}
	
	if (!sfx->data)
		return;

	// increase the usefulness
	if (sfx->usefulness++ < 0)
		sfx->usefulness = -1;

#ifdef SURROUND
	// judgecutor:
	// Avoid channel reverse if surround
	if (stereoreverse.value && sep != SURROUND_SEP)
		sep = (~sep) & 255;
#else
	//added:11-04-98:
	if (stereoreverse.value)
		sep = (~sep) & 255;
#endif

	//CONS_Printf("stereo %d reverse %d\n", sep, stereoreverse.value);

	// Assigns the handle to one of the channels in the
	//  mix/output buffer.
	channels[cnum].handle = I_StartSoundEx(sfx_id,
										 /*
										    sfx->data,
										  */
										 volume, sep, pitch, priority, (mobj_t *) origin_p,
										 orientation, front, center);
}

void S_StartSound(void *origin, int sfx_id)
{
	// the volume is handled 8 bits
	S_StartSoundAtVolume(origin, sfx_id, 255);
}

void S_StopSound(void *origin)
{
	int cnum;

	// SoM: Sounds without origion can have multiple sources, they shouldn't
	// be stoped by new sounds.
	if (!origin)
		return;

	for (cnum = 0; cnum < cv_numChannels.value; cnum++)
	{
		if (channels[cnum].sfxinfo && channels[cnum].origin == origin)
		{
			S_StopChannel(cnum);
			break;
		}
	}

	I_CutOrigonator(origin);
}

//
// Stop and resume music, during game PAUSE.
//
void S_PauseSound(void)
{
	if (digmusic)
		I_PauseSong(0);
	else if (mus_playing && !mus_paused)
	{
		I_PauseSong(mus_playing->handle);
		mus_paused = true;
	}

	// pause cd music
#ifdef LINUX
	I_PauseCD();
#else
	I_StopCD();
#endif
}

void S_ResumeSound(void)
{
	if (digmusic)
		I_ResumeSong(0);
	else if (mus_playing && mus_paused)
	{
		I_ResumeSong(mus_playing->handle);
		mus_paused = false;
	}

	// resume cd music
	I_ResumeCD();
}

//
// Updates music & sounds
//
static int actualsfxvolume;		//check for change through console
static int actualmusicvolume;

void S_UpdateSounds(void)
{
	int audible;
	int cnum;
	int volume;
	int sep;
	int pitch;
	sfxinfo_t *sfx;
	channel_t *c;

	mobj_t *listener = players[displayplayer[0]].mo;

	if (dedicated)
		return;

	// Update sound/music volumes, if changed manually at console
	if (actualsfxvolume != cv_soundvolume.value)
		S_SetSfxVolume(cv_soundvolume.value);
	if (actualmusicvolume != cv_musicvolume.value)
		S_SetMusicVolume(cv_musicvolume.value);

	/*
	   Clean up unused data.
	   if (gametic > nextcleanup)
	   {
	   for (i=1 ; i<NUMSFX ; i++)
	   {
	   if (S_sfx[i].usefulness==0)
	   {
	   //S_sfx[i].usefulness--;

	   // don't forget to unlock it !!!
	   // __dmpi_unlock_....
	   //Z_ChangeTag(S_sfx[i].data, PU_CACHE);
	   //S_sfx[i].data = 0;

	   CONS_Printf ("\2flushed sfx %.6s\n", S_sfx[i].name);
	   }
	   }
	   nextcleanup = gametic + 15;
	   }
	 */

	for (cnum = 0; cnum < cv_numChannels.value; cnum++)
	{
		c = &channels[cnum];
		sfx = c->sfxinfo;

		if (c->sfxinfo)
		{
			if (I_SoundIsPlaying(c->handle))
			{
				// initialize parameters
				volume = 255;	//8 bits internal volume precision
				pitch = NORM_PITCH;
				sep = NORM_SEP;

				if (sfx->link)	// strange (BP)
				{
					pitch = sfx->pitch;
					volume += sfx->volume;
					if (volume < 1)
					{
						S_StopChannel(cnum);
						continue;
					}
				}

				// check non-local sounds for distance clipping
				//  or modify their params
				if (c->origin && listener != c->origin &&
					!(cv_splitscreen.value && c->origin == players[displayplayer[1]].mo))
				{
					int audible2;
					int volume2 = volume, sep2 = sep, pitch2 = pitch;
					audible = S_AdjustSoundParams(listener, c->origin, &volume, &sep, &pitch);

					if (cv_splitscreen.value)
					{
						audible2 =
							S_AdjustSoundParams(players[displayplayer[1]].
												mo, c->origin, &volume2, &sep2, &pitch2);
						if (audible2 && (!audible || (audible && volume2 > volume)))
						{
							audible = true;
							volume = volume2;
							sep = sep2;
							pitch = pitch2;
						}
					}

					if (!audible)
					{
						S_StopChannel(cnum);
					}
					else
						I_UpdateSoundParams(c->handle, volume, sep, pitch);
				}
			}
			else
			{
				// if channel is allocated but sound has stopped,
				//  free it
				S_StopChannel(cnum);
			}
		}
	}
	// kill music if it is a single-play && finished
	// if (     mus_playing
	//      && !I_QrySongPlaying(mus_playing->handle)
	//      && !mus_paused )
	// S_StopMusic();

}

void S_SetMusicVolume(int volume)
{
	if (volume < 0 || volume > 31)
		CONS_Printf("musicvolume should be between 0-31\n");

	CV_SetValue(&cv_musicvolume, volume & 31);
	actualmusicvolume = cv_musicvolume.value;	//check for change of var

	I_SetMusicVolume(volume & 31);
}

void S_SetSfxVolume(int volume)
{
	if (volume < 0 || volume > 31)
		CONS_Printf("sfxvolume should be between 0-31\n");

	CV_SetValue(&cv_soundvolume, volume & 31);
	actualsfxvolume = cv_soundvolume.value;	//check for change of var

	// now hardware volume
	I_SetSfxVolume(volume & 31);

}

//
// Starts some music with the music id found in sounds.h.
//
void S_StartMusic(int m_id)
{
	S_ChangeMusic(m_id, false);
}

//
// S_ChangeMusicName
// Changes music by name
void S_ChangeMusicName(char *name, int looping)
{
	int music;

	if (!strncmp(name, "-", 6))
	{
		S_StopMusic();
		return;
	}

	music = S_FindMusic(name);

	if (music > mus_None && music < NUMMUSIC)
		S_ChangeMusic(music, looping);
	else
	{
		CONS_Printf("music not found: %s\n", name);
		S_StopMusic();			// stop music anyway
	}
}

void S_ChangeMusic(int music_num, int looping)
{
	musicinfo_t *music;
	
	if (dedicated)
		return;
		
	if (nomusic)
		return;
		
	if ((music_num <= mus_None) || (music_num >= NUMMUSIC))
	{
		CONS_Printf("ERROR: Bad music number %d\n", music_num);
		return;
	}
	else
		music = &S_music[music_num];
		
	if (mus_playing == music)
		return;
	
	I_PlaySong(I_RegisterSong(va("d_%s", music->name)));
	
	mus_playing = music;
	
/*
	musicinfo_t *music;

	if (dedicated)
		return;

	if (digmusic)
	{
		if ((music_num <= mus_None) || (music_num >= NUMMUSIC))
		{
			CONS_Printf("ERROR: Bad music number %d\n", music_num);
			return;
		}
		else
			music = &S_music[music_num];

		if (mus_playing == music)
			return;

		I_StartFMODSong(music->name, looping);
		mus_playing = music;
		return;
	}

	if (nomusic)
		return;

	if ((music_num <= mus_None) || (music_num >= NUMMUSIC))
	{
		CONS_Printf("ERROR: Bad music number %d\n", music_num);
		return;
	}
	else
		music = &S_music[music_num];

	if (mus_playing == music)
		return;

	// shutdown old music
	S_StopMusic();

	// get lumpnum if neccessary
	if (!music->lumpnum)
	{
		music->lumpnum = W_GetNumForName(va("d_%s", music->name));
	}
	// load & register it
	music->data = (void *)W_CacheLumpNum(music->lumpnum, PU_MUSIC);
#ifdef __MACOS__
	music->handle = I_RegisterSong(music_num);
#else
	music->handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));
#endif


	// play it
	I_PlaySong(music->handle, looping);

	mus_playing = music;*/
}

void I_StopFMODSong(void);

void S_StopMusic(void)
{
/*	if (mus_playing)
	{
		if (mus_paused)
			I_ResumeSong(mus_playing->handle);

		if (digmusic)
			I_StopFMODSong();
		I_StopSong(mus_playing->handle);
		I_UnRegisterSong(mus_playing->handle);
		if (!digmusic)
			Z_ChangeTag(mus_playing->data, PU_CACHE);

		mus_playing->data = 0;
		mus_playing = 0;
	}*/
}

static void S_StopChannel(int cnum)
{

	int i;
	channel_t *c = &channels[cnum];

	if (c->sfxinfo)
	{
		// stop the sound playing
		if (I_SoundIsPlaying(c->handle))
		{
			I_StopSound(c->handle);
		}

		// check to see
		//  if other channels are playing the sound
		for (i = 0; i < cv_numChannels.value; i++)
		{
			if (cnum != i && c->sfxinfo == channels[i].sfxinfo)
			{
				break;
			}
		}

		// degrade usefulness of sound data
		c->sfxinfo->usefulness--;

		c->sfxinfo = 0;
	}
}

/* S_AdjustSoundParamsEx() -- Enhanced for new sound code */
// by GhostlyDeath <February 20, 2009>
// Sets parameters...
// Returns 0 if sound cannot be heard
// Returns 1 if sound is heard (but is not centered)
// Returns 2 if sound is heard (but is centered)
int S_AdjustSoundParamsEx(mobj_t* Listener, mobj_t* Source,
	int32_t* Volume,		// Volume of the sound (Distance) [Mono]
	int32_t* Balance,		// Balance of the sound (left/right) [Stereo + Surround + Full Surround]
	int32_t* Pitch,		// Change in pitch (Doppler!?!?) [All]
	int32_t* Orientation,	// Balance of the sound (front/back) [Surround + Full Surround]
	int32_t* FrontVolume	// How loud to play a sound for the front speaker [Full Surround]
	)
{
	fixed_t approx_dist;
	fixed_t adx;
	fixed_t ady;
	fixed_t momxd;
	fixed_t momyd;
	fixed_t momzd;
	fixed_t momt;
	angle_t angle;
	
	/* Check for valid parms */
	if (!Listener || !Source || !Volume)
		return 0;
		
	/* Center speaker quick verify stuff */
	if (Listener == Source)
	{
		// always use these values
		if (Pitch)
			*Pitch = NORM_PITCH;
		if (Volume)
			*Volume = 255;
		if (Balance)
			*Balance = NORM_SEP;
		if (Orientation)
			*Orientation = NORM_SEP;
		if (FrontVolume)
			*FrontVolume = 0;
		
		return 2;			// play on the center speaker
	}
	
	/* Quick Distance */
	// calculate the distance to sound origin
	//  and clip it if necessary
	adx = abs(Listener->x - Source->x);
	ady = abs(Listener->y - Source->y);

	// From _GG1_ p.428. Appox. eucledian distance fast.
	approx_dist = adx + ady - ((adx < ady ? adx : ady) >> 1);

	if (approx_dist > S_CLIPPING_DIST)
		return 0;
		
	/* Doppler effect */
#if 0
	if (Pitch)
	{
		// Get difference
		momxd = listener->momx - source->momx;
		momyd = listener->momy - source->momy;
		momzd = listener->momz - source->momz;
	
		// Average values
		momt = momxd + momyd + momzd;
		momt = FixedDiv(momt, 3 << FRACBITS);
		
		*Pitch = NORM_PITCH + (FixedMul((NORM_PITCH << 1) << FRACBITS, momt) >> FRACBITS);
	}
#endif
	
	/* Volume */
	if (approx_dist < S_CLOSE_DIST)
		*Volume = 255;
	else
		*Volume = (15 * ((S_CLIPPING_DIST - approx_dist) >> FRACBITS)) / S_ATTENUATOR;
	
	/* Balance */
	if (Balance)
	{
		// angle of source to listener
		angle = R_PointToAngle2(Listener->x, Listener->y, Source->x, Source->y);

		if (angle > Listener->angle)
			angle = angle - Listener->angle;
		else
			angle = angle + (0xffffffff - Listener->angle);
		
		angle >>= ANGLETOFINESHIFT;
		*Balance = NORM_SEP - (FixedMul(S_STEREO_SWING, finesine[angle]) >> FRACBITS);
	}
	
	/* Orientation */
	if (Orientation)
	{
		// angle of source to listener
		angle = R_PointToAngle2(Listener->y, Listener->x, Source->y, Source->x);	// I really don't know
		// swapped x and y is equiv to rotating clockwise 90 degrees then flipping vertically

		if (angle > Listener->angle)
			angle = angle - Listener->angle;
		else
			angle = angle + (0xffffffff - Listener->angle);
		
		angle >>= ANGLETOFINESHIFT;
		*Orientation = NORM_SEP - (FixedMul(S_STEREO_SWING, finesine[angle]) >> FRACBITS);	// might be cosine?
		// it might also be + instead of -
	}
	
	return (*Volume > 0);
}

//
// Changes volume, stereo-separation, and pitch variables
//  from the norm of a sound effect to be played.
// If the sound is not audible, returns a 0.
// Otherwise, modifies parameters and returns 1.
//
int S_AdjustSoundParams(mobj_t * listener, mobj_t * source, int *vol, int *sep, int *pitch)
{
	fixed_t approx_dist;
	fixed_t adx;
	fixed_t ady;
	angle_t angle;
	
	if (!listener || !source)
		return 0;

	// calculate the distance to sound origin
	//  and clip it if necessary
	adx = abs(listener->x - source->x);
	ady = abs(listener->y - source->y);

	// From _GG1_ p.428. Appox. eucledian distance fast.
	approx_dist = adx + ady - ((adx < ady ? adx : ady) >> 1);

	if (gamemap != 8 && approx_dist > S_CLIPPING_DIST)
	{
		return 0;
	}

	// angle of source to listener
	angle = R_PointToAngle2(listener->x, listener->y, source->x, source->y);

	if (angle > listener->angle)
		angle = angle - listener->angle;
	else
		angle = angle + (0xffffffff - listener->angle);

#ifdef SURROUND

	// Produce a surround sound for angle from 105 till 255
	if (surround.value == 1 && (angle > (ANG90 + (ANG45 / 3)) && angle < (ANG270 - (ANG45 / 3))))
		*sep = SURROUND_SEP;
	else
	{
#endif

		angle >>= ANGLETOFINESHIFT;

		// stereo separation
		*sep = 128 - (FixedMul(S_STEREO_SWING, finesine[angle]) >> FRACBITS);

#ifdef SURROUND
	}
#endif

	// volume calculation
	if (approx_dist < S_CLOSE_DIST)
	{
		// added 2-2-98 SfxVolume is now hardware volume
		*vol = 255;				//snd_SfxVolume;
	}
	// removed hack here for gamemap==8 (it made far sound still present)
	else
	{
		// distance effect
		*vol = (15 * ((S_CLIPPING_DIST - approx_dist) >> FRACBITS)) / S_ATTENUATOR;
	}

	return (*vol > 0);
}

//
// S_getChannel :
//   If none available, return -1.  Otherwise channel #.
//
int S_getChannel(void *origin, sfxinfo_t * sfxinfo)
{
	// channel number to use
	int cnum;

	channel_t *c;

	// Find an open channel
	for (cnum = 0; cnum < cv_numChannels.value; cnum++)
	{
		if (!channels[cnum].sfxinfo)
			break;
		else if (origin && channels[cnum].origin == origin)
		{
			S_StopChannel(cnum);
			break;
		}
	}

	// None available
	if (cnum == cv_numChannels.value)
	{
		// Look for lower priority
		for (cnum = 0; cnum < cv_numChannels.value; cnum++)
			if (channels[cnum].sfxinfo->priority >= sfxinfo->priority)
				break;

		if (cnum == cv_numChannels.value)
		{
			// FUCK!  No lower priority.  Sorry, Charlie.
			return -1;
		}
		else
		{
			// Otherwise, kick out lower priority.
			S_StopChannel(cnum);
		}
	}

	c = &channels[cnum];

	// channel is decided to be cnum.
	c->sfxinfo = sfxinfo;
	c->origin = origin;

	return cnum;
}

// SoM: Searches through the channels and checks for origin or id.
// returns 0 of not found, returns 1 if found.
// if id == -1, the don't check it...
int S_SoundPlaying(void *origin, int id)
{
	int cnum;

	for (cnum = 0; cnum < cv_numChannels.value; cnum++)
	{
		if (origin && channels[cnum].origin == origin)
			return 1;
		if (id != -1 && channels[cnum].sfxinfo - S_sfx == id)
			return 1;
	}
	return 0;
}

//
// S_StartSoundName
// Starts a sound using the given name.
#define MAXNEWSOUNDS 10
int newsounds[MAXNEWSOUNDS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void S_StartSoundName(void *mo, char *soundname)
{
	int i;
	int soundnum = 0;
	//Search existing sounds...
	for (i = sfx_None + 1; i < NUMSFX; i++)
	{
		if (!S_sfx[i].name)
			continue;
		if (!strcasecmp(S_sfx[i].name, soundname))
		{
			soundnum = i;
			break;
		}
	}

	if (!soundnum)
	{
		for (i = 0; i < MAXNEWSOUNDS; i++)
		{
			if (newsounds[i] == 0)
				break;
			if (!S_SoundPlaying(NULL, newsounds[i]))
			{
				S_RemoveSoundFx(newsounds[i]);
				break;
			}
		}

		if (i == MAXNEWSOUNDS)
		{
			CONS_Printf("Cannot load another extra sound!\n");
			return;
		}

		soundnum = S_AddSoundFx(soundname, false);
		newsounds[i] = soundnum;
	}

	S_StartSound(mo, soundnum);
}

#endif
