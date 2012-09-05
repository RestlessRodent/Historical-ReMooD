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
// Copyright (C) 2011-2012 GhostlyDeath <ghostlydeath@gmail.com>
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
#include "m_argv.h"
#include "r_main.h"
#include "p_maputl.h"

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
	S_NoiseThinker_t* Origin;	// Source sound
	fixed_t MoveRate;			// Rate at which to copy
	fixed_t RateAdjust;			// Adjust the rate, somewhat
	fixed_t Volume;				// Volume adjust
	fixed_t ChanVolume[16];		// Per channel volume
	WX_WADEntry_t* Entry;		// WAD entry being played
	void* Data;					// Sound data being played
	int SoundID;				// Sound being played
	int Priority;				// Priority of the sound being played
	int BasePriority;			// Priority of the original sound
	bool_t Used;				// Channel is being used
	
	// GhostlyDeath <September 21, 2011> -- Any sound length!
	fixed_t RoveByte;			// Byte movement
	uint32_t CurrentByte;		// Current byte playing
	uint32_t StopByte;			// Byte to stop at
} S_SoundChannel_t;

/**************
*** GLOBALS ***
**************/

// c_CVPVSpeakerSetup -- Speaker Setup
const CONL_VarPossibleValue_t c_CVPVSpeakerSetup[] =
{
	{1, "Monaural"},
	{1, "Mono"},
	
	{2, "Stereo"},
	{2, "Daul"},
	
	{4, "Surround"},
	
	{6, "Full Surround"},
	{6, "FullSurround"},
	
	// End
	{1, "MINVAL"},
	{6, "MAXVAL"},
	{0, NULL},
};

// snd_speakersetup -- Speakers to use
CONL_StaticVar_t l_SNDSpeakerSetup =
{
	CLVT_INTEGER, c_CVPVSpeakerSetup, CLVF_SAVE,
	"snd_speakersetup", DSTR_CVHINT_SNDSPEAKERSETUP, CLVVT_STRING, "Stereo",
	NULL
};

// c_CVPVQuality -- Speaker Setup
const CONL_VarPossibleValue_t c_CVPVQuality[] =
{
	{5512, "5 KHz"},
	{11025, "11 KHz"},
	{22050, "22 KHz"},
	{44100, "44 KHz"},
	
	// End
	{5512, "MINVAL"},
	{44100, "MAXVAL"},
	{0, NULL},
};

// snd_quality -- Sound Quality
CONL_StaticVar_t l_SNDQuality =
{
	CLVT_INTEGER, c_CVPVQuality, CLVF_SAVE,
	"snd_quality", DSTR_CVHINT_SNDQUALITY, CLVVT_STRING, "11025",
	NULL
};

// c_CVPVDensity -- Speaker Setup
const CONL_VarPossibleValue_t c_CVPVDensity[] =
{
	{8, "8-bit"},
	{8, "8bit"},
	{8, "Single"},
	
	{16, "16-bit"},
	{16, "16bit"},
	{16, "Double"},
	
	// End
	{8, "MINVAL"},
	{16, "MAXVAL"},
	{0, NULL},
};

// snd_density -- Sound Density
CONL_StaticVar_t l_SNDDensity =
{
	CLVT_INTEGER, c_CVPVDensity, CLVF_SAVE,
	"snd_density", DSTR_CVHINT_SNDDENSITY, CLVVT_STRING, "8",
	NULL
};

// snd_buffersize -- Buffer Size
CONL_StaticVar_t l_SNDBufferSize =
{
	CLVT_INTEGER, c_CVPVPositive, CLVF_SAVE,
	"snd_buffersize", DSTR_CVHINT_SNDBUFFERSIZE, CLVVT_STRING, "512",
	NULL
};

// snd_randompitch -- Randomized sound pitch
CONL_StaticVar_t l_SNDRandomPitch =
{
	CLVT_INTEGER, c_CVPVBoolean, CLVF_SAVE,
	"snd_randompitch", DSTR_CVHINT_SNDRANDOMPITCH, CLVVT_STRING, "false",
	NULL
};

// snd_channels -- Number of channels
CONL_StaticVar_t l_SNDChannels =
{
	CLVT_INTEGER, c_CVPVPositive, CLVF_SAVE,
	"snd_channels", DSTR_CVHINT_SNDCHANNELS, CLVVT_STRING, "12",
	NULL
};

// snd_reservedchannels -- Number of reserved channels
CONL_StaticVar_t l_SNDReservedChannels =
{
	CLVT_INTEGER, c_CVPVPositive, CLVF_SAVE,
	"snd_reservedchannels", DSTR_CVHINT_SNDRESERVEDCHANNELS, CLVVT_STRING, "2",
	NULL
};

bool_t S_VolumeVarsChanged(CONL_ConVariable_t* const a_Var, CONL_StaticVar_t* const a_StaticVar);

// c_CVPVVolume -- Volume
const CONL_VarPossibleValue_t c_CVPVVolume[] =
{
	// End
	{0, "MINVAL"},
	{31, "MAXVAL"},
	{0, NULL},
};

// snd_soundvolume -- Volume of sound
CONL_StaticVar_t l_SNDSoundVolume =
{
	CLVT_INTEGER, c_CVPVVolume, CLVF_SAVE,
	"snd_soundvolume", DSTR_CVHINT_SNDSOUNDVOLUME, CLVVT_STRING, "15",
	S_VolumeVarsChanged
};

// snd_musicvolume -- Volume of sound
CONL_StaticVar_t l_SNDMusicVolume =
{
	CLVT_INTEGER, c_CVPVVolume, CLVF_SAVE,
	"snd_musicvolume", DSTR_CVHINT_SNDMUSICVOLUME, CLVVT_STRING, "15",
	S_VolumeVarsChanged
};

/*************
*** LOCALS ***
*************/

static bool_t l_SoundOK = false;	// Did the sound start OK?
static bool_t l_MusicOK = true;	// Same but for Music
static int l_CurrentSong = 0;	// Current playing song handle
static int l_Bits, l_Freq, l_Channels, l_Len;
static S_SoundChannel_t* l_DoomChannels;	// Sound channels
static size_t l_NumDoomChannels;	// Number of possible sound channels
static size_t l_ReservedChannels;	// Reserved Channels
static fixed_t l_GlobalSoundVolume;	// Global sound volume
static bool_t l_ThreadedSound = false;	// Threaded sound

/****************
*** FUNCTIONS ***
****************/

/* S_GetListenerEmitterWithDist() -- Gets the listener for the sound, the emiter and the distance */
fixed_t S_GetListenerEmitterWithDist(S_SoundChannel_t* const a_Channel, S_NoiseThinker_t* const a_Origin, S_NoiseThinker_t** const a_Listen, S_NoiseThinker_t** const a_Emit)
{
	fixed_t ApproxDist, NewDist = 0;
	int i;
	S_NoiseThinker_t* Attempt;
	
	/* Check */
	if (!a_Listen || !a_Emit)
		return 0;
	
	/* Clear listener */
	*a_Listen = NULL;
		
	/* Find emitter of object */
	if (a_Channel)
		*a_Emit = a_Channel->Origin;	// Emitter is the channel origin
	else
		*a_Emit = a_Origin;		// Emitter is the object origin
		
	/* If there is no emitter, don't bother going on */
	if (!*a_Emit)
		return 0;
	
	/* No local players */
	if (g_SplitScreen < 0)
		return 0;
		
	/* Find the closest listener */
	*a_Listen = NULL;
	ApproxDist = 32000 << FRACBITS;
	for (i = 0; i <= g_SplitScreen; i++)
	{
		// Check to see if the player is in game (if not ignore)
		if (displayplayer[i] < 0 || displayplayer[i] >= MAXPLAYERS || !playeringame[displayplayer[i]])
			continue;
			
		// Attempt getting listener
		if (players[displayplayer[i]].mo)
			Attempt = &players[displayplayer[i]].mo->NoiseThinker;
		else
			continue;
			
		// Get distance
		NewDist = P_AproxDistance(Attempt->x - (*a_Emit)->x, Attempt->y - (*a_Emit)->y);
		
		// Better?
		if (NewDist < ApproxDist)
		{
			// Set distance over
			ApproxDist = NewDist;
			
			// Set listener
			*a_Listen = Attempt;
		}
	}
	
	/* Normalize distance */
	ApproxDist -= 120 << FRACBITS;
	
	// Very close?
	if (ApproxDist < 0)
		ApproxDist = 0;
		
	// Return
	return ApproxDist;
}

/* S_PlayEntryOnChannel() -- Plays a WAD entry on a channel */
S_SoundChannel_t* S_PlayEntryOnChannel(const uint32_t a_Channel, WX_WADEntry_t* const a_Entry)
{
	uint16_t* p;
	void* Data;
	int32_t Header, Freq, Length;
	size_t LumpLen;
	
	/* Check */
	if (a_Channel >= l_NumDoomChannels || !a_Entry)
		return NULL;
		
	/* Check length */
	LumpLen = WX_GetEntrySize(a_Entry);
	
	if (LumpLen < 9)
		return NULL;
		
	/* Use entry */
	Data = WX_CacheEntry(a_Entry);
	
	/* Read basic stuff */
	p = Data;
	Header = ReadUInt16(&p);
	Freq = ReadUInt16(&p);
	Length = ReadUInt16(&p);
	
	if (!Freq || !Length || Header != 3)
		return NULL;
	
	/* Set channel info */
	l_DoomChannels[a_Channel].Entry = a_Entry;
	l_DoomChannels[a_Channel].Data = Data;
	l_DoomChannels[a_Channel].Used = true;
	l_DoomChannels[a_Channel].RateAdjust = 1 << FRACBITS;
	l_DoomChannels[a_Channel].Priority = 127;
	l_DoomChannels[a_Channel].BasePriority = 127;
	
	// Determine the play rate, which is by default the ratio of the sound freq and the card freq
	//l_DoomChannels[a_Channel].MoveRate = FixedDiv((fixed_t) Freq << FRACBITS, (fixed_t) l_Freq << FRACBITS);
	l_DoomChannels[a_Channel].MoveRate = ((Freq << 15) / (l_Freq >> 1));
	
	/* Long sounds */
	l_DoomChannels[a_Channel].RoveByte = 0;
	l_DoomChannels[a_Channel].CurrentByte = 8;
	l_DoomChannels[a_Channel].StopByte = LumpLen - 9;
	
	/* Return channel */
	return &l_DoomChannels[a_Channel];
}

/* S_ReservedChannelPlay() -- Play sound on reserved channel */
void S_ReservedChannelPlay(int sound_id, int volume, fixed_t MoveVal)
{
#define BUFSIZE 24
	char Buf[BUFSIZE];
	WX_WADEntry_t* Entry;
	S_SoundChannel_t* Target;
	
	/* Obtain entry then play on said channel */
	// Prefix with ds
	snprintf(Buf, BUFSIZE, "ds%.6s", S_sfx[sound_id].name);
	Entry = WX_EntryForName(NULL, Buf, false);
	
	// Try direct name
	if (!Entry)
		Entry = WX_EntryForName(NULL, S_sfx[sound_id].name, false);
	Target = S_PlayEntryOnChannel(l_NumDoomChannels - 1, Entry);
	
	/* Modify Target */
	if (Target)
	{
		Target->MoveRate = FixedMul(Target->MoveRate, FixedDiv(MoveVal, ((fixed_t)l_Freq) << FRACBITS));
	}
#undef BUFSIZE
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
	// Clear stuff
	l_DoomChannels[a_Channel].Origin = NULL;
	l_DoomChannels[a_Channel].MoveRate = 1 << FRACBITS;
	l_DoomChannels[a_Channel].Volume = 1 << FRACBITS;
	l_DoomChannels[a_Channel].Entry = NULL;
	l_DoomChannels[a_Channel].Data = NULL;
	l_DoomChannels[a_Channel].SoundID = 0;
	l_DoomChannels[a_Channel].Used = false;
	l_DoomChannels[a_Channel].RateAdjust = 1 << FRACBITS;
	l_DoomChannels[a_Channel].Priority = 0;
	l_DoomChannels[a_Channel].BasePriority = 0;
	l_DoomChannels[a_Channel].RoveByte = 0;
	l_DoomChannels[a_Channel].CurrentByte = 0;
	l_DoomChannels[a_Channel].StopByte = 0;
	
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
		return 0;
		
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

void S_UpdateSingleChannel(S_SoundChannel_t* const a_Channel, S_NoiseThinker_t* const a_Listen, S_NoiseThinker_t* const a_Emit, const fixed_t a_Dist);

/* S_StartSoundAtVolume() -- Starts playing a sound */
void S_StartSoundAtVolume(S_NoiseThinker_t* a_Origin, int sound_id, int volume)
{
#define BUFSIZE 24
	char Buf[BUFSIZE];
	WX_WADEntry_t* Entry;
	S_SoundChannel_t* Target;
	int OnChannel, i, LowestP, MyP;
	fixed_t RPA, Dist, GS;
	S_NoiseThinker_t* Listener;
	S_NoiseThinker_t* Emitter;
	
	/* Check */
	if (!l_SoundOK || sound_id < 0 || sound_id >= NUMSFX)
		return;
		
	/* Get gamespeed */
	// For slow motioning, cap to 0.25
	GS = 1 << FRACBITS;
	if (GS < 16384)
		GS = 16384;
		
	/* Get closest listener, emitter, and distance */
	Dist = S_GetListenerEmitterWithDist(NULL, a_Origin, &Listener, &Emitter);
	if (Dist < 0)
		Dist = -Dist;
	
	// The further the sound is the lower the priority
	MyP = S_sfx[sound_id].priority;
	MyP = FixedMul(MyP << FRACBITS, (1 << FRACBITS) - FixedMul(Dist, 60)) >> FRACBITS;
	
	/* Lock sound thread */
	I_SoundLockThread(true);
	
	/* Check if sound is already on a channel */
	OnChannel = S_SoundPlaying(a_Origin, sound_id);
	
	// Not playing on a channel
	if (!OnChannel)
	{
		// Find first free channel
		for (OnChannel = 0; OnChannel < l_NumDoomChannels - l_ReservedChannels; OnChannel++)
			if (!l_DoomChannels[OnChannel].Used)
				break;
				
		// No channel found
		if (OnChannel == l_NumDoomChannels)
		{
			// Find the channel with the lowest priority
			LowestP = -1;
			
			for (OnChannel = 0; OnChannel < l_NumDoomChannels - l_ReservedChannels; OnChannel++)
				// Only care about used channels (check anyway, despite always being true)
				if (l_DoomChannels[OnChannel].Used)
					// Replace a channel with a lower priority
					if (l_DoomChannels[OnChannel].Priority <= MyP &&
					((LowestP == -1) || (LowestP >= 0 && l_DoomChannels[OnChannel].Priority <= l_DoomChannels[LowestP].Priority)))
						LowestP = OnChannel;
						
			// Don't play the sound, not worth it!
			if (LowestP == -1)
			{
				// Unlock sound thread, if this ever returns
				I_SoundLockThread(false);
				return;
			}
			
			// Choose a random channel
			OnChannel = LowestP;
			
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
	{
		// If this fails, always unlock just in case
		I_SoundLockThread(false);
		return;
	}
	
	/* Set extra stuff */
	Target->Origin = a_Origin;
	Target->SoundID = sound_id;
	Target->Volume = 1 << FRACBITS;
	Target->Priority = MyP;
	Target->Priority = S_sfx[sound_id].priority;
	
	// Original volumes
	for (i = 0; i < 16; i++)
		Target->ChanVolume[i] = FixedDiv(volume << FRACBITS, 255 << FRACBITS);
		
	// Random sound pitch?
	if (l_SNDRandomPitch.Value->Int)
	{
		// Get value to adjust
		RPA = FixedDiv((fixed_t) M_Random() << FRACBITS, 127 << FRACBITS);
		
		// Cap to 0.75 .. 1.25
		if (RPA <= 49152)
			RPA = 49152;
		else if (RPA >= 81920)
			RPA = 81920;
			
		// Modify move rate to random pitch change
		Target->MoveRate = FixedMul(Target->MoveRate, RPA);
	}
	
	// Game Speed modifier
	Target->MoveRate = FixedMul(Target->MoveRate, GS);
	
	/* Unlock sound */
	I_SoundLockThread(false);
	
	/* Update channel the sound is playing on */
	S_UpdateSingleChannel(Target, Listener, Emitter, Dist);
	
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
void S_StartSoundName(S_NoiseThinker_t* a_Origin, char* soundname)
{
	/* Check */
	if (!l_SoundOK)
		return;
	
	S_StartSound(a_Origin, S_SoundIDForName(soundname));
}

/* S_StopSound() -- Stop sound being played by this object */
void S_StopSound(S_NoiseThinker_t* a_Origin)
{
	int Found;
	
	/* Check */
	if (!l_SoundOK || !a_Origin)
		return;
		
	/* Lock */
	I_SoundLockThread(true);
	
	/* Find channel playing sound */
	if ((Found = S_SoundPlaying(a_Origin, 0)))
		// Stop channel
		S_StopChannel(Found - 1);
		
	/* Unlock */
	I_SoundLockThread(false);
}

fixed_t P_AproxDistance(fixed_t dx, fixed_t dy);

/* S_UpdateSingleChannel() -- Updates a single channel */
void S_UpdateSingleChannel(S_SoundChannel_t* const a_Channel, S_NoiseThinker_t* const a_Listen, S_NoiseThinker_t* const a_Emit, const fixed_t a_Dist)
{
	S_NoiseThinker_t* Listener;
	S_NoiseThinker_t* Emitter;
	fixed_t ApproxDist, DistVol, Fine;
	angle_t Angle;
	size_t i;
	fixed_t Dist;
	
	/* Check */
	if (!a_Channel)
		return;
		
	/* Need to get listeners? */
	if (!a_Listen || !a_Emit)
		Dist = S_GetListenerEmitterWithDist(a_Channel, NULL, &Listener, &Emitter);
		
	// Use passed variables
	else
	{
		Listener = a_Listen;
		Emitter = a_Emit;
		Dist = a_Dist;
	}
	
	// Listener not set (so can't hear sounds much really)
	if (!Listener || !Emitter)
		return;
		
	/* Reset Channel volumes, since all of them will be wiped */
	for (i = 0; i < l_Channels; i++)
		a_Channel->ChanVolume[i] = 1 << FRACBITS;
		
	/* If the listener is the emitter, play all sounds normally */
	// Since the channel volumes are already maxed, lower volume here
	if (Listener == Emitter)
		return;
		
	/* Approximate the distance between the map object and the listener */
#if 1
	ApproxDist = Dist;
#else
	ApproxDist = P_AproxDistance(Listener->x - Emitter->x, Listener->y - Emitter->y);
	
	// Close sounds are always full blast
	ApproxDist -= 120 << FRACBITS;
	
	// Very close?
	if (ApproxDist < 0)
		ApproxDist = 0;
#endif
	
	// The volume of the sound is somewhere within 120-1,200 map units
	DistVol = (1 << FRACBITS) - FixedMul(ApproxDist, 60);
	
	// GhostlyDeath <August 27, 2011> -- Modify sound priority (before I trash DistVol)
	a_Channel->Priority = FixedMul(a_Channel->BasePriority << FRACBITS, DistVol) >> FRACBITS;
	
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
		
	/* Lock sound */
	I_SoundLockThread(true);
	
	/* Go through all playing sounds */
	for (i = 0; i < l_NumDoomChannels; i++)
	{
		// Skip unused channels
		if (!l_DoomChannels[i].Used)
			continue;
			
		// Update single channel
		S_UpdateSingleChannel(&l_DoomChannels[i], NULL, NULL, 0);
	}
	
	/* Unlock sound */
	I_SoundLockThread(false);
}

void SetChannelsNum(void)
{
}

/* SCLC_SoundMulti() -- Sound multi-handler */
static int SCLC_SoundMulti(const uint32_t a_ArgC, const char** const a_ArgV)
{
	/* Check */
	if (a_ArgC < 2)
		return CLE_FAILURE;
	
	/* Play Song */
	if (strcasecmp(a_ArgV[0], "soundchangemus") == 0)
	{
		S_ChangeMusicName(a_ArgV[1], true);
	}
	
	/* Play Sound */
	else if (strcasecmp(a_ArgV[0], "soundplay") == 0)
	{
	}
	
	/* Success */
	return CLE_SUCCESS;
}

/* S_RegisterSoundStuff() -- Register the sound console variables */
void S_RegisterSoundStuff(void)
{
	static bool_t cvRegged = false;
	
	/* Check */
	if (cvRegged)
		return;
	
	/* Register new commands */
	CONL_AddCommand("soundchangemus", SCLC_SoundMulti);
	CONL_AddCommand("soundplay", SCLC_SoundMulti);
		
	/* Register Variables */
	CONL_VarRegister(&l_SNDChannels);
	CONL_VarRegister(&l_SNDReservedChannels);
	CONL_VarRegister(&l_SNDMusicVolume);
	CONL_VarRegister(&l_SNDSoundVolume);
	CONL_VarRegister(&l_SNDDensity);
	CONL_VarRegister(&l_SNDQuality);
	CONL_VarRegister(&l_SNDSpeakerSetup);
	CONL_VarRegister(&l_SNDBufferSize);
	CONL_VarRegister(&l_SNDRandomPitch);
	
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
	{
		// GhostlyDeath <January 20, 2012> -- To prevent a race, init channels here
		l_NumDoomChannels = l_SNDChannels.Value->Int;
		l_ReservedChannels = l_SNDReservedChannels.Value->Int;
	
		if (!l_NumDoomChannels)
			l_NumDoomChannels = 1;
		
		l_DoomChannels = Z_Malloc(sizeof(*l_DoomChannels) * l_NumDoomChannels, PU_STATIC, NULL);
		
		// Now startup sound
		if (I_StartupSound())
			l_SoundOK = true;
		
		// If sound failed to start, free sound channels
		if (!l_SoundOK)
		{
			Z_Free(l_DoomChannels);
			l_DoomChannels = NULL;
		}
	}
			
	// Set volumes based on CVARs
	S_UpdateCVARVolumes();
	
	/* Try getting a buffer */
	if (l_SoundOK)
	{
		if (!(l_Len = I_SoundBufferRequest(IST_WAVEFORM, l_SNDDensity.Value->Int, l_SNDQuality.Value->Int, l_SNDSpeakerSetup.Value->Int, l_SNDBufferSize.Value->Int)))
		{
			l_Bits = l_Freq = l_Channels = l_Len = 0;
			CONL_PrintF("S_Init: Failed to obtain a sound buffer.\n");
		}
		// Setup buffer
		else
		{
			// Set threaded function (if possible)
			l_ThreadedSound = I_SoundSetThreaded(S_UpdateSounds);
			
			// Is multi-thread?
			if (l_ThreadedSound)
				CONL_PrintF("S_Init: Sound is multi-threaded!\n");
				
			// Remember settings
			l_Bits = l_SNDDensity.Value->Int;
			l_Freq = I_SoundGetFreq();
			l_Channels = l_SNDSpeakerSetup.Value->Int;
			
			// Frequency did not match
			if (l_Freq != l_SNDQuality.Value->Int)
				CONL_PrintF("S_Init: Requested %iHz but got %iHz\n", l_SNDQuality.Value->Int, l_Freq);
		}
	}
	
	// Music
	l_MusicOK = false;
	if (!M_CheckParm("-nomusic"))
		if (I_InitMusic())
			l_MusicOK = true;
}

/* S_StopSounds() -- Stops all playing sounds */
void S_StopSounds(void)
{
	size_t i;
	
	/* Check */
	if (!l_SoundOK)
		return;
		
	/* Lock sound */
	I_SoundLockThread(true);
	
	/* Stop all sounds on every channel */
	for (i = 0; i < l_NumDoomChannels; i++)
		if (l_DoomChannels[i].Used)
			S_StopChannel(i);
			
	/* Unlock sound */
	I_SoundLockThread(false);
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
		int spmus[] =
		{
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
void S_ChangeMusicName(char* name, int looping)
{
#define BUFSIZE 24
	size_t i;
	char NameBuf[BUFSIZE];
	
	/* Check */
	if (!l_MusicOK || !name)
		return;
		
	/* If a song is already playing */
	if (l_CurrentSong)
		S_StopMusic();
	
	/* Double check */
	for (i = 0; i < 2; i++)
	{
		// Prepend the D_ prefix?
		memset(NameBuf, 0, sizeof(NameBuf));
		snprintf(NameBuf, BUFSIZE - 1, "%s%s", (!i ? "D_" : ""), name);
		C_strupr(NameBuf);
		
		// Call the interface
		l_CurrentSong = I_RegisterSong(NameBuf);
	
		// Failed?
		if (!l_CurrentSong)
		{
			if (!i)
				continue;
			else
				return;
		}
	}
		
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
	Normed = ((fixed_t) a_Byte - 128) << FRACBITS;
	
	/* Apply volume */
	return (FixedMul(Normed, a_ModVolume) >> FRACBITS) + 128;
}

/* S_WriteMixSample() -- Writes a single sample */
static void S_WriteMixSample(void** Buf, uint8_t Value)
{
	int32_t v, s;
	
	/* Get shift */
	s = ((int32_t)Value) - 128;
	
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
void S_UpdateSounds(const bool_t a_Threaded)
{
	void* SoundBuf, *p, *End;
	size_t SoundLen, i, j;
	uint8_t ReadSample;
	uint16_t Mod;
	fixed_t ActualRate, ModVolume[16];
	
	/* Check */
	if (!l_Bits || !l_SoundOK)
		return;
		
	/* Check for thread match */
	if (a_Threaded != l_ThreadedSound)
		return;
		
	/* Is the buffer finished? */
	if (!I_SoundBufferIsFinished())
		return;
		
	/* If this is threaded, lock! */
	I_SoundLockThread(true);
	
	/* Update all playing sounds */
	//S_RepositionSounds();
	
	/* Obtain Buffer */
	SoundBuf = I_SoundBufferObtain();
	SoundLen = l_Len * (l_Bits / 8) * l_Channels;
	End = (uint8_t*)SoundBuf + SoundLen;
	
	// Check
	if (!SoundBuf)
		return;
		
	/* Clear buffer completely */
	if (l_Bits == 16)
		for (Mod = 0x8000, i = 0, p = SoundBuf; i < (SoundLen >> 1); i++)
			WriteUInt16(&p, Mod);
	else
		memset(SoundBuf, 0x80, SoundLen);
		
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
		for (; p < End && l_DoomChannels[i].CurrentByte < l_DoomChannels[i].StopByte; l_DoomChannels[i].RoveByte += ActualRate)
		{
			// Move byte ahead?
			l_DoomChannels[i].CurrentByte += l_DoomChannels[i].RoveByte >> FRACBITS;
			l_DoomChannels[i].RoveByte &= 0xFFFF;	// keep lower
			
			// Read sample bit from data
			ReadSample = ((uint8_t*)l_DoomChannels[i].Data)[(l_DoomChannels[i].CurrentByte)];
			
			// Write in
			for (j = 0; j < l_Channels; j++)
				S_WriteMixSample(&p, S_ApplySampleVol(ReadSample, ModVolume[j]));
		}
		
		// Did the sound stop?
		if (l_DoomChannels[i].CurrentByte >= l_DoomChannels[i].StopByte)
			S_StopChannel(i);
	}
	
	/* Write to driver */
	I_SoundBufferWriteOut(SoundBuf, SoundLen, l_Freq, l_Bits, l_Channels);
	
	/* Unlock thread */
	I_SoundLockThread(false);
}

/* S_SetMusicVolume() -- Sets music volume */
void S_SetMusicVolume(int volume)
{
	/* Set variable */
	if (l_MusicOK)
		CONL_VarSetInt(&l_SNDMusicVolume, volume);
}

/* S_SetSfxVolume() -- Sets sound volume */
void S_SetSfxVolume(int volume)
{
	/* Set variable */
	if (l_SoundOK)
		CONL_VarSetInt(&l_SNDSoundVolume, volume);
}

/* S_UpdateCVARVolumes() -- CVAR Volumes changed */
void S_UpdateCVARVolumes(void)
{
	/* Send values to interfaces */
	// Music
	if (l_MusicOK)
		I_SetMusicVolume(l_SNDMusicVolume.Value->Int * 8);
		
	// Sound is not sent because it is dynamically modified by ReMooD.
	// So instead of messing with mixer values, I can just lower the amplitude
	// of sounds being mixed.
	if (l_SoundOK)
		l_GlobalSoundVolume = FixedDiv(l_SNDSoundVolume.Value->Int << FRACBITS, 31 << FRACBITS);
}

/* S_VolumeVarsChanged() -- Volume variables changed */
bool_t S_VolumeVarsChanged(CONL_ConVariable_t* const a_Var, CONL_StaticVar_t* const a_StaticVar)
{
	S_UpdateCVARVolumes();
	return true;
}

void Command_SoundReset_f(void)
{
}

void S_FreeSfx(sfxinfo_t* sfx)
{
}

/* S_SoundIDForName() -- Return sound ID for sound name */
// TODO FIXME: Speed this up
int S_SoundIDForName(const char* const a_Name)
{
	size_t i;
	
	/* Check */
	if (!a_Name)
		return 0;
	
	/* Find sounds */
	for (i = 0; i < NUMSFX; i++)
		if (S_sfx[i].name)
			if (strcasecmp(a_Name, S_sfx[i].name) == 0)
				return i;
	
	/* Return default sound */
	return 0;
}

