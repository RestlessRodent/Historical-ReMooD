// Emacs style mode select   -*- C++ -*- 
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

#include <math.h>

#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_mutex.h>
#include <SDL_byteorder.h>
#include <SDL_version.h>

#ifndef _WIN32
#include <unistd.h>
#endif

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

#define W_CacheLumpNum(num) (W_CacheLumpNum)((num),1)
#define W_CacheLumpName(name) W_CacheLumpNum (W_GetNumForName(name))
#define PIPE_CHECK(fh) if (broken_pipe) { fclose(fh); fh = NULL; broken_pipe = 0; }

#define MIDBUFFERSIZE   128*1024

// The number of internal mixing channels,
//  the samples calculated for each mixing step,
//  the size of the 16bit, 2 hardware channel (stereo)
//  mixing buffer, and the samplerate of the raw data.

// Needed for calling the actual sound output.
#define NUM_CHANNELS            8

#define SAMPLERATE              11025	// Hz

static int samplecount = 512;

static int lengths[NUMSFX];		// The actual lengths of all sound effects.
static unsigned int channelstep[NUM_CHANNELS];	// The channel step amount...
static unsigned int channelstepremainder[NUM_CHANNELS];	// ... and a 0.16 bit remainder of last step.

// The channel data pointers, start and end.
static unsigned char *channels[NUM_CHANNELS];
static unsigned char *channelsend[NUM_CHANNELS];

// Time/gametic that the channel started playing,
//  used to determine oldest, which automatically
//  has lowest priority.
// In case number of active sounds exceeds
//  available channels.
static int channelstart[NUM_CHANNELS];

// The sound in channel handles,
//  determined on registration,
//  might be used to unregister/stop/modify,
//  currently unused.
static int channelhandles[NUM_CHANNELS];

// SFX id of the playing sound effect.
// Used to catch duplicates (like chainsaw).
static int channelids[NUM_CHANNELS];

// GhostlyDeath -- Array for what channel is owned by what thing
static mobj_t *channelthing[NUM_CHANNELS];

// Pitch to stepping lookup, unused.
static int steptable[256];

// Volume lookups.
static int vol_lookup[128 * 256];

// Hardware left and right channel volume lookup.
static int *channelleftvol_lookup[NUM_CHANNELS];
static int *channelrightvol_lookup[NUM_CHANNELS];

// Buffer for MIDI
static char *musicbuffer;

// Flags for the -nosound and -nomusic options
extern boolean nosound;
extern boolean nomusic;

static boolean musicStarted = false;
static boolean soundStarted = false;

//
// This function loads the sound data from the WAD lump,
//  for single sound.
//
static void *getsfx(const char *sfxname, int *len)
{
	unsigned char *sfx;
	unsigned char *paddedsfx;
	int i;
	int size;
	int paddedsize;
	char name[20];
	int sfxlump;

	// Get the sound data from the WAD, allocate lump
	//  in zone memory.
	if (gamemode == heretic)
		sprintf(name, "%s", sfxname);
	else
		sprintf(name, "ds%s", sfxname);

	// Now, there is a severe problem with the
	//  sound handling, in it is not (yet/anymore)
	//  gamemode aware. That means, sounds from
	//  DOOM II will be requested even with DOOM
	//  shareware.
	// The sound list is wired into sounds.c,
	//  which sets the external variable.
	// I do not do runtime patches to that
	//  variable. Instead, we will use a
	//  default sound for replacement.

	if (W_CheckNumForName(name) == -1)
	{
		if (gamemode == heretic)
			sprintf(name, "ds%s", sfxname);
		else
			sprintf(name, "%s", sfxname);
		
		if (W_CheckNumForName(name) == -1)
		{
			if (gamemode == heretic)
				sfxlump = W_GetNumForName("gldhit");
			else
				sfxlump = W_GetNumForName("dspistol");
		}
		else
			sfxlump = W_GetNumForName(name);
	}
	else
		sfxlump = W_GetNumForName(name);

	size = W_LumpLength(sfxlump);

	sfx = (unsigned char *)W_CacheLumpNum(sfxlump);

	// If it's a 22Khz Sound
	if (((uint16_t *) sfx)[1] == 22050)
	{
		size /= 2;
		paddedsize = /*(*/(size - 8 + (samplecount - 1))/* / samplecount) * samplecount*/;
		paddedsfx = (unsigned char *)Z_Malloc(paddedsize + 8, PU_STATIC, 0);

		for (i = 0; i < size; i++)
			paddedsfx[i] = (sfx[i * 2] + (sfx[i * 2] - 1)) / 2;

		for (i = size; i < paddedsize + 8; i++)
			paddedsfx[i] = 128;
	}
	else
	{
		// Pads the sound effect out to the mixing buffer size.
		// The original realloc would interfere with zone memory.
		paddedsize = /*(*/(size - 8 + (samplecount - 1))/* / samplecount) * samplecount*/;

		// Allocate from zone memory.
		paddedsfx = (unsigned char *)Z_Malloc(paddedsize + 8, PU_STATIC, 0);
		// This should interfere with zone memory handling,
		//  which does not kick in in the soundserver.

		// Now copy and pad.
		memcpy(paddedsfx, sfx, size);
		for (i = size; i < paddedsize + 8; i++)
			paddedsfx[i] = 128;
	}

	// Remove the cached lump.
	Z_Free(sfx);

	// Preserve padded length.
	*len = paddedsize;

	// Return allocated padded data.
	return (void *)(paddedsfx + 8);
}

//
// This function adds a sound to the
//  list of currently active sounds,
//  which is maintained as a given number
//  (eight, usually) of internal channels.
// Returns a handle.
//
static int addsfx(int sfxid, int volume, int step, int seperation, mobj_t * origin)
{
	static unsigned short handlenums = 0;

	int i;
	int j;
	int rc = -1;

	int oldest = gametic;
	int oldestnum = 0;
	int slot;

	int rightvol;
	int leftvol;

	// Chainsaw troubles.
	// Play these sound effects only one at a time.
	/*if (sfxid == sfx_sawup || sfxid == sfx_sawidl || sfxid == sfx_sawful || sfxid == sfx_sawhit || sfxid == sfx_stnmov || sfxid == sfx_pistol)
	   {
	   // Loop all channels, check.
	   for (i = 0; i < NUM_CHANNELS; i++)
	   {
	   // Active, and using the same SFX?
	   if ((channels[i]) && (channelids[i] == sfxid))
	   {
	   // Reset.
	   channels[i] = 0;
	   // We are sure that iff,
	   //  there will only be one.
	   break;
	   }
	   }
	   } */

	// GhostlyDeath -- Loop all channels to find if a new sound is being made
	j = NUM_CHANNELS;
	if (origin != NULL)
	{
		for (j = 0; (j < NUM_CHANNELS) && (channels[j]); j++)
		{
			if (channelthing[j] == origin)
				channels[j] = 0;
		}
	}

	if (j == NUM_CHANNELS)
	{
		// Loop all channels to find oldest SFX.
		for (i = 0; (i < NUM_CHANNELS) && (channels[i]); i++)
		{
			if (channelstart[i] < oldest)
			{
				oldestnum = i;
				oldest = channelstart[i];
			}
		}
	}
	else
		i = j;

	// Tales from the cryptic.
	// If we found a channel, fine.
	// If not, we simply overwrite the first one, 0.
	// Probably only happens at startup.
	if (i == NUM_CHANNELS)
		slot = oldestnum;
	else
		slot = i;

	// Okay, in the less recent channel,
	//  we will handle the new SFX.
	// Set pointer to raw data.
	channels[slot] = (unsigned char *)S_sfx[sfxid].data;
	// Set pointer to end of raw data.
	channelsend[slot] = channels[slot] + lengths[sfxid];
	channelthing[slot] = origin;

	// Reset current handle number, limited to 0..100.
	if (!handlenums)
		handlenums = 100;

	// Assign current handle number.
	// Preserved so sounds could be stopped (unused).
	channelhandles[slot] = rc = handlenums++;

	// Set stepping???
	// Kinda getting the impression this is never used.
	channelstep[slot] = step;
	// ???
	channelstepremainder[slot] = 0;
	// Should be gametic, I presume.
	channelstart[slot] = gametic;

	// Separation, that is, orientation/stereo.
	//  range is: 1 - 256
	seperation += 1;

	// Per left/right channel.
	//  x^2 seperation,
	//  adjust volume properly.
	//    volume *= 8;

	// Volume arrives in range 0..255 and it must be in 0..cv_soundvolume...
	volume = (volume * cv_soundvolume.value) >> 7;
	// Notice : sdldoom replaced all the calls to avoid this conversion

	leftvol = volume - ((volume * seperation * seperation) >> 16);	///(256*256);
	seperation = seperation - 257;
	rightvol = volume - ((volume * seperation * seperation) >> 16);

	// Sanity check, clamp volume.
	if (rightvol < 0)
		rightvol = 0;
	else if (rightvol > 127)
		rightvol = 127;

	if (leftvol < 0)
		leftvol = 0;
	else if (leftvol > 127)
		leftvol = 127;

	// Get the proper lookup table piece
	//  for this volume level???
	channelleftvol_lookup[slot] = &vol_lookup[leftvol * 256];
	channelrightvol_lookup[slot] = &vol_lookup[rightvol * 256];

	// Preserve sound SFX id,
	//  e.g. for avoiding duplicates of chainsaw.
	channelids[slot] = sfxid;

	// You tell me.
	return rc;
}

//
// SFX API
// Note: this was called by S_Init.
// However, whatever they did in the
// old DPMS based DOS version, this
// were simply dummies in the Linux
// version.
// See soundserver initdata().
//
// Well... To keep compatibility with legacy doom, I have to call this in
// I_InitSound since it is not called in S_Init... (emanne@absysteme.fr)

void I_SetChannels()
{
	// Init internal lookups (raw data, mixing buffer, channels).
	// This function sets up internal lookups used during
	//  the mixing process.
	int i;
	int j;

	int *steptablemid = steptable + 128;

	if (nosound)
		return;

	// This table provides step widths for pitch parameters.
	// I fail to see that this is currently used.
	for (i = -128; i < 128; i++)
		steptablemid[i] = (int)(pow(2.0, (i / 64.0)) * 65536.0);

	// Generates volume lookup tables
	//  which also turn the unsigned samples
	//  into signed samples.
	for (i = 0; i < 128; i++)
		for (j = 0; j < 256; j++)
		{
			vol_lookup[i * 256 + j] = (i * (j - 128) * 256) / 127;
		}
}

void I_SetSfxVolume(int volume)
{
	// Identical to DOS.
	// Basically, this should propagate
	//  the menu/config file setting
	//  to the state variable used in
	//  the mixing.

	CV_SetValue(&cv_soundvolume, volume);

}

//
// Retrieve the raw data lump index
//  for a given SFX name.
//
int I_GetSfxLumpNum(sfxinfo_t * sfx)
{
	char namebuf[9];
	
	if (gamemode == heretic)
		sprintf(namebuf, "%s", sfx->name);
	else
		sprintf(namebuf, "ds%s", sfx->name);
		
	return W_GetNumForName(namebuf);
}

void *I_GetSfx(sfxinfo_t * sfx)
{
	int len;
	return getsfx(sfx->name, &len);
}

// FIXME: dummy for now Apr.9 2001 by Rob
void I_FreeSfx(sfxinfo_t * sfx)
{
}

//
// Starting a sound means adding it
//  to the current list of active sounds
//  in the internal channels.
// As the SFX info struct contains
//  e.g. a pointer to the raw data,
//  it is ignored.
// As our sound handling does not handle
//  priority, it is ignored.
// Pitching (that is, increased speed of playback)
//  is set, but currently not used by mixing.
//
int I_StartSound(int id, int vol, int sep, int pitch, int priority, mobj_t * origin)
{

	// UNUSED
	priority = 0;

	if (nosound)
		return 0;

	// Returns a handle (not used).
	SDL_LockAudio();
	id = addsfx(id, vol, steptable[pitch], sep, origin);
	SDL_UnlockAudio();

	return id;
}

void I_StopSound(int handle)
{
	// You need the handle returned by StartSound.
	// Would be looping all channels,
	//  tracking down the handle,
	//  an setting the channel to zero.

	handle = 0;
}

void I_CutOrigonator(void *origin)
{
	int i;

	if (!origin)
		return;

	for (i = 0; i < NUM_CHANNELS && channels[i]; i++)
	{
		if (channelthing[i] == origin)
		{
			channels[i] = 0;
		}
	}
}

int I_SoundIsPlaying(int handle)
{
	// Ouch.
	return gametic < handle;
}

//
// Not used by SDL version
//
void I_SubmitSound(void)
{
}

//
// This function loops all active (internal) sound
//  channels, retrieves a given number of samples
//  from the raw sound data, modifies it according
//  to the current (internal) channel parameters,
//  mixes the per channel samples into the given
//  mixing buffer, and clamping it to the allowed
//  range.
//
// This function currently supports only 16bit.
//
void I_UpdateSound()
{
	/*
	   Pour une raison que j'ignore, la version SDL n'appelle jamais
	   ce truc directement. Fonction vide pour garder une compatibilite
	   avec le point de vue de legacy...
	 */

	// Himmel, Arsch und Zwirn
}

void I_UpdateSound_sdl(void *unused, Uint8 * stream, int len)
{
	// Mix current sound data.
	// Data, from raw sound, for right and left.
	register unsigned int sample;
	register int dl;
	register int dr;

	// Pointers in audio stream, left, right, end.
	signed short *leftout;
	signed short *rightout;
	signed short *leftend;
	// Step in stream, left and right, thus two.
	int step;

	// Mixing channel index.
	int chan;
	int i, j, k;

	if (nosound)
		return;

	// Left and right channel
	//  are in audio stream, alternating.
	leftout = (signed short *)stream + 1;
	rightout = ((signed short *)stream);
	step = 2;

	// Determine end, for left channel only
	//  (right channel is implicit).
	leftend = leftout + samplecount * step;

	// Mix sounds into the mixing buffer.
	// Loop over step*samplecount,
	//  that is 512 values for two channels.
	while (leftout != leftend)
	{
		// Reset left/right value.
		dl = *leftout;
		dr = *rightout;

		// Love thy L2 chache - made this a loop.
		// Now more channels could be set at compile time
		//  as well. Thus loop those  channels.
		for (chan = 0; chan < NUM_CHANNELS; chan++)
		{
			// Check channel, if active.
			if (channels[chan] > 100)	// GhostlyDeath -- I got crashes due to channels[0] being 0x1
			{		
				// Get the raw data from the channel.
				sample = *channels[chan];
				// Add left and right part
				//  for this channel (sound)
				//  to the current data.
				// Adjust volume accordingly.
				if (channelthing[chan] != NULL)
				{
					int dontgo = 0;
					int avg = 0;
					
					for (i = 0; i < cv_splitscreen.value + 1; i++)
						if (players[displayplayer[i]].mo &&
							players[displayplayer[i]].mo == channelthing[chan])
						dontgo = 1;
					
					if (!dontgo)
					{
						int volume = 0;
						int seperation = 0;
						int volumet = 0;
						int seperationt = 0;
						int pitch = 0;
						int volumex[MAXSPLITSCREENPLAYERS];
						int sepx[MAXSPLITSCREENPLAYERS];
						int pitchx[MAXSPLITSCREENPLAYERS];
						int leftvol = 0;
						int rightvol = 0;

						for (i = 0; i < cv_splitscreen.value + 1; i++)
						{
							S_AdjustSoundParams(players[displayplayer[i]].mo, channelthing[chan], &volumex[i],	// volume
												&sepx[i],	// sep
												&pitchx[i]	//pitch
												);
							
							if (volumex[i] > 0)
							{
								volumet += volumex[i];
								seperationt += sepx[i];
								avg++;
							}
						}
						
						if (avg)
						{
							volume = volumet / avg;
							seperation = seperationt / avg;
						}
						
						if (stereoreverse.value)
							seperation = (~seperation) & 255;

						volume = (volume * cv_soundvolume.value) >> 7;
						leftvol = volume - ((volume * seperation * seperation) >> 16);	///(256*256);
						seperation = seperation - 257;
						rightvol = volume - ((volume * seperation * seperation) >> 16);
						if (rightvol < 0 || rightvol > 127)
							return;
						if (leftvol < 0 || leftvol > 127)
							return;
						channelleftvol_lookup[chan] = &vol_lookup[leftvol * 256];
						channelrightvol_lookup[chan] = &vol_lookup[rightvol * 256];
					}
				}

				dl += channelleftvol_lookup[chan][sample];
				dr += channelrightvol_lookup[chan][sample];

				// Increment index ???
				channelstepremainder[chan] += channelstep[chan];
				// MSB is next sample???
				channels[chan] += channelstepremainder[chan] >> 16;
				// Limit to LSB???
				channelstepremainder[chan] &= 65536 - 1;

				// Check whether we are done.
				if (channels[chan] >= channelsend[chan])
				{
					channels[chan] = 0;
					channelthing[chan] = NULL;
				}
			}
		}

		// Clamp to range. Left hardware channel.
		// Has been char instead of short.

		if (dl > 0x7fff)
			*leftout = 0x7fff;
		else if (dl < -0x8000)
			*leftout = -0x8000;
		else
			*leftout = dl;

		// Same for right hardware channel.
		if (dr > 0x7fff)
			*rightout = 0x7fff;
		else if (dr < -0x8000)
			*rightout = -0x8000;
		else
			*rightout = dr;

		// Increment current pointers in stream
		leftout += step;
		rightout += step;
	}
}

void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
	// I fail too see that this is used.
	// Would be using the handle to identify
	//  on which channel the sound might be active,
	//  and resetting the channel parameters.

	// UNUSED.
	handle = vol = sep = pitch = 0;
}

void I_ShutdownSound(void)
{

	if (nosound)
		return;

	if (!soundStarted)
		return;

	CONS_Printf("I_ShutdownSound: ");
	SDL_CloseAudio();
	CONS_Printf("shut down\n");
	soundStarted = false;
}

static SDL_AudioSpec audio;

void I_StartupSound()
{
	int i;

	if (nosound)
		return;

	// Configure sound device
	CONS_Printf("I_InitSound: ");

	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		CONS_Printf("Couldn't initialize SDL Audio: %s\n", SDL_GetError());
		nosound = true;
		return;
	}
	// Open the audio device
	audio.freq = SAMPLERATE;
#if ( SDL_BYTEORDER == SDL_BIG_ENDIAN )
	audio.format = AUDIO_S16MSB;
#else
	audio.format = AUDIO_S16LSB;
#endif
	audio.channels = 2;
	audio.samples = samplecount;
	audio.callback = I_UpdateSound_sdl;
	if (SDL_OpenAudio(&audio, NULL) < 0)
	{
		CONS_Printf("couldn't open audio with desired format\n");
		SDL_CloseAudio();
		nosound = true;
		return;
	}
	samplecount = audio.samples;
	CONS_Printf(" configured audio device with %d samples/slice\n", samplecount);

	// Initialize external data (all sounds) at start, keep static.
	CONS_Printf("I_InitSound: (%d sfx)", NUMSFX);

	for (i = 1; i < NUMSFX; i++)
	{
		// Alias? Example is the chaingun sound linked to pistol.
		if (S_sfx[i].name)
		{
			if (!S_sfx[i].link)
			{
				// Load data from WAD file.
				S_sfx[i].data = getsfx(S_sfx[i].name, &lengths[i]);
			}
			else
			{
				// Previously loaded already?
				S_sfx[i].data = S_sfx[i].link->data;
				lengths[i] = lengths[(S_sfx[i].link - S_sfx) / sizeof(sfxinfo_t)];
			}
		}
	}

	CONS_Printf(" pre-cached all sound data\n");

	// Finished initialization.
	CONS_Printf("I_InitSound: sound module ready\n");
	SDL_PauseAudio(0);
	I_SetChannels();
	soundStarted = true;
}

