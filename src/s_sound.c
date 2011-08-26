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
#include "g_game.h"

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
	fixed_t RateAdjust;									// Adjust the rate, somewhat
	fixed_t Volume;										// Volume adjust
	fixed_t ChanVolume[16];								// Per channel volume
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
static fixed_t l_GlobalSoundVolume;					// Global sound volume

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
	l_DoomChannels[a_Channel].RateAdjust = 1 << FRACBITS;
	
	/* Read basic stuff */
	p = l_DoomChannels[a_Channel].Data;
	Header = ReadUInt16(&p);
	Freq = ReadUInt16(&p);
	Length = ReadUInt16(&p);
	
	// Set basic stuff
	l_DoomChannels[a_Channel].Stop = l_DoomChannels[a_Channel].Position + ((fixed_t)Length << FRACBITS);
	
	// Determine the play rate, which is by default the ratio of the sound freq and the card freq
	l_DoomChannels[a_Channel].MoveRate = FixedDiv((fixed_t)Freq << FRACBITS, (fixed_t)l_Freq << FRACBITS);
	
	/* Return channel */
	return &l_DoomChannels[a_Channel];
}

/* S_StopChannel() -- Stop channel from playing */
void S_StopChannel(const uint32_t a_Channel)
{
	size_t i;
	
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
	l_DoomChannels[a_Channel].RateAdjust = 1 << FRACBITS;
	
	for (i = 0; i < 16; i++)
		l_DoomChannels[a_Channel].ChanVolume[i] = 1 << FRACBITS;
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

void S_UpdateSingleChannel(S_SoundChannel_t* const a_Channel);

/* S_StartSoundAtVolume() -- Starts playing a sound */
void S_StartSoundAtVolume(S_NoiseThinker_t* a_Origin, int sound_id, int volume)
{
#define BUFSIZE 24
	char Buf[BUFSIZE];
	int OnChannel, i;
	fixed_t RPA;
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
	Target->Volume = 1 << FRACBITS;
	
	// Original volumes
	for (i = 0; i < 16; i++)
		Target->ChanVolume[i] = FixedDiv(volume << FRACBITS, 255 << FRACBITS);
	
	// Random sound pitch?
	if (cv_rndsoundpitch.value)
	{
		// Get value to adjust
		RPA = FixedDiv((fixed_t)M_Random() << FRACBITS, 127 << FRACBITS);
		
		// Cap to 0.75 .. 1.25
		if (RPA <= 49152)
			RPA = 49152;
		else if (RPA >= 81920)
			RPA = 81920;
		
		// Modify move rate to random pitch change
		Target->MoveRate = FixedMul(Target->MoveRate, RPA);
	}
	
	/* Update channel the sound is playing on */
	S_UpdateSingleChannel(Target);
	
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
	int Found;
	
	/* Check */
	if (!l_SoundOK || !a_Origin)
		return;
	
	/* Find channel playing sound */
	if (!(Found = S_SoundPlaying(a_Origin, 0)))
		return;
	
	/* Stop channel */
	S_StopChannel(Found - 1);
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

fixed_t P_AproxDistance(fixed_t dx, fixed_t dy);

/* S_UpdateSingleChannel() -- Updates a single channel */
void S_UpdateSingleChannel(S_SoundChannel_t* const a_Channel)
{
	S_NoiseThinker_t* Listener;
	S_NoiseThinker_t* Emitter;
	fixed_t ApproxDist, DistVol, Fine;
	angle_t Angle;
	size_t i;
	
	/* Check */
	if (!a_Channel)
		return;
	
	/* The emitting thing is the channel */
	Emitter = a_Channel->Origin;
	
	// No emitter? This is the case for HUD sounds
	if (!Emitter)
		return NULL;
	
	/* Find the thing that is listening */
	Listener = NULL;
	
	// Just use displayplayer for now!
	if (players[displayplayer[0]].mo)
		Listener = &players[displayplayer[0]].mo->NoiseThinker;
	
	// Listener not set (so can't hear sounds much really)
	if (!Listener)
		return;
	
	/* Reset Channel volumes, since all of them will be wiped */
	for (i = 0; i < l_Channels; i++)
		a_Channel->ChanVolume[i] = 1 << FRACBITS;
	
	/* If the listener is the emitter, play all sounds normally */
	// Since the channel volumes are already maxed, lower volume here
	if (Listener == Emitter)
		return;
	
	/* Approximate the distance between the map object and the listener */
	ApproxDist = P_AproxDistance(Listener->x - Emitter->x, Listener->y - Emitter->y);
	
	// Close sounds are always full blast
	ApproxDist -= 120 << FRACBITS;
	
	// Too far to hear?
	/*if (ApproxDist >= (1080 << FRACBITS))
		ApproxDist = (1080 << FRACBITS);*/
	
	// Very close?
	if (ApproxDist < 0)
		ApproxDist = 0;
	
	// The volume of the sound is somewhere within 120-1,200 map units
	DistVol = (1 << FRACBITS) - FixedMul(ApproxDist, 60);
	
	if (DistVol < 0)
		DistVol = 0;
	
	/* Get balance swing between left and right */
	Angle = R_PointToAngle2(Listener->x, Listener->y, Emitter->x, Emitter->y);
	
	// Correct Listener Angle
	Angle -= Listener->Angle;
	Fine = finesine[Angle >> ANGLETOFINESHIFT];
	
	/* Set final parameters based on the channel count */
	switch (l_Channels)
	{
			// Mono -- Distance only
		case 1:
			a_Channel->ChanVolume[0] = FixedMul(a_Channel->ChanVolume[0], DistVol);
			break;
			
			// Stereo -- Left/right based on angle
		case 2:
			// Left
			if (Fine > 0)
			{
				a_Channel->ChanVolume[0] = FixedMul(a_Channel->ChanVolume[0], DistVol);
				a_Channel->ChanVolume[1] = FixedMul(FixedMul(a_Channel->ChanVolume[1], DistVol), (1 << FRACBITS) - Fine);
			}
			
			// Right
			else
			{
				a_Channel->ChanVolume[0] = FixedMul(FixedMul(a_Channel->ChanVolume[0], DistVol), (1 << FRACBITS) + Fine);
				a_Channel->ChanVolume[1] = FixedMul(a_Channel->ChanVolume[1], DistVol);
			}
			
			//a_Channel->ChanVolume[0] = FixedMul(FixedMul(a_Channel->ChanVolume[0], DistVol), Fine);
			//a_Channel->ChanVolume[1] = FixedMul(FixedMul(a_Channel->ChanVolume[1], DistVol), -Fine);
			break;
	}
}

/* S_RepositionSounds() -- Repositions all sounds */
void S_RepositionSounds(void)
{
	size_t i;
	
	/* Check */
	if (!l_SoundOK)
		return;
	
	/* Go through all playing sounds */
	for (i = 0; i < l_NumDoomChannels; i++)
	{
		// Skip unused channels
		if (!l_DoomChannels[i].Used)
			continue;
		
		// Update single channel
		S_UpdateSingleChannel(&l_DoomChannels[i]);
	}
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

/* S_ApplySampleVol() -- Apply volume to sample */
static uint8_t S_ApplySampleVol(const uint8_t a_Byte, const fixed_t a_ModVolume)
{
	fixed_t Normed;
	
	/* Normalize a bit */
	Normed = ((fixed_t)a_Byte - 128) << FRACBITS;
	
	/* Apply volume */
	return (FixedMul(Normed, a_ModVolume) >> FRACBITS) + 128;
}

/* S_WriteMixSample() -- Writes a single sample */
static void S_WriteMixSample(void** Buf, uint8_t Value)
{
	int32_t v, s;
	
	/* Get shift */
	s = (int32_t)Value - 128;
	
	/* Write Wide */
	if (l_Bits == 16)
	{
		// Read current value (I hope the buffer is aligned!)
		v = **((uint16_t**)Buf);
		v = (v - 32768) + (s * 256);
		
		if (v > 32767)
			v = 32767;
		else if (v < -32768)
			v = -32768;
		v += 32768;
		
		// Mix into buffer
		WriteUInt16(Buf, v);
	}
	
	/* Write Narrow */
	else
	{
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
	}
}

/* S_UpdateSounds() -- Updates all playing sounds */
void S_UpdateSounds(void)
{
	void* SoundBuf, *p, *End;
	size_t SoundLen, i, j;
	uint8_t ReadSample;
	fixed_t ActualRate, ModVolume[16];
	
	static FILE* f;
	
	if (!f)
		f = fopen("/tmp/rawsound.raw", "wb"); 
	
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
		
		// Determine volume for all channels
		for (j = 0; j < l_Channels; j++)
			ModVolume[j] = FixedMul(FixedMul(l_DoomChannels[i].Volume, l_DoomChannels[i].ChanVolume[j]), l_GlobalSoundVolume);
		
		// Keep reading and mixing
		ActualRate = FixedMul(l_DoomChannels[i].MoveRate, l_DoomChannels[i].RateAdjust);
		for (; p < End && l_DoomChannels[i].Position < l_DoomChannels[i].Stop; l_DoomChannels[i].Position += ActualRate)
		{
			// Read sample bit from data
			ReadSample = ((uint8_t*)l_DoomChannels[i].Data)[l_DoomChannels[i].Position >> FRACBITS];
			
			// Write in
			for (j = 0; j < l_Channels; j++)
				S_WriteMixSample(&p, S_ApplySampleVol(ReadSample, ModVolume[j]));
		}
		
		// Did the sound stop?
		if (l_DoomChannels[i].Position >= l_DoomChannels[i].Stop)
			S_StopChannel(i);
	}
	
	for (p = SoundBuf; p < End; p)
	{
		ReadSample = ReadUInt8(&p);
		fwrite(&ReadSample, 1, 1, f);
	}
	
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
	if (l_SoundOK)
		l_GlobalSoundVolume = FixedDiv(cv_soundvolume.value << FRACBITS, 31 << FRACBITS);
}

void Command_SoundReset_f(void)
{
}

void S_FreeSfx(sfxinfo_t * sfx)
{
}

