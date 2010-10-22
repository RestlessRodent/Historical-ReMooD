// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.sourceforge.net/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           Demyx                  (demyx@endgameftw.com)
//                    Dragan                 (poliee13@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2009 The ReMooD Team..
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION:  

#ifdef MUSSERV
#include <sys/msg.h>
struct musmsg
{
	long msg_type;
	char msg_text[12];
};
extern int msg_id;
#endif

#include "doomdef.h"
#include "doomstat.h"
#include "command.h"
#include "g_game.h"
#include "m_argv.h"
#include "../src_cl/r_main.h"				//R_PointToAngle2() used to calc stereo sep.
#include "../src_cl/r_things.h"						// for skins
#include "p_info.h"

#include "i_sound.h"
#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"
#include "d_main.h"

#include "m_random.h"
#include "../src_cl/m_menu.h"

void Command_SoundReset_f(void)
{
	I_ShutdownSound();
	I_StartupSound();
}

// commands for music and sound servers
#ifdef MUSSERV
consvar_t musserver_cmd = { "musserver_cmd", "musserver", CV_SAVE };
consvar_t musserver_arg = { "musserver_arg", "-t 20 -f -u 0", CV_SAVE };
#endif
#ifdef SNDSERV
consvar_t sndserver_cmd = { "sndserver_cmd", "llsndserv", CV_SAVE };
consvar_t sndserver_arg = { "sndserver_arg", "-quiet", CV_SAVE };
#endif

#if defined (_WIN32) && !defined (SURROUND)
#define SURROUND
#endif

#ifdef __MACOS__
consvar_t play_mode = { "play_mode", "0", CV_SAVE, CV_Unsigned };
#endif

void I_StartFMODSong(char *musicname, int looping);

// stereo reverse 1=true, 0=false
consvar_t stereoreverse = { "stereoreverse", "0", CV_SAVE, CV_OnOff };

// if true, all sounds are loaded at game startup
consvar_t precachesound = { "precachesound", "0", CV_SAVE, CV_OnOff };

CV_PossibleValue_t soundvolume_cons_t[] = { {0, "MIN"}, {31, "MAX"}, {0, NULL} };

// actual general (maximum) sound & music volume, saved into the config
consvar_t cv_soundvolume = { "soundvolume", "15", CV_SAVE, soundvolume_cons_t };
consvar_t cv_musicvolume = { "musicvolume", "15", CV_SAVE, soundvolume_cons_t };
consvar_t cv_rndsoundpitch = { "rndsoundpitch", "Off", CV_SAVE, CV_OnOff };

// number of channels available
void SetChannelsNum(void);
consvar_t cv_numChannels = { "snd_channels", "16", CV_SAVE | CV_CALL, CV_Unsigned, SetChannelsNum };

#ifdef SURROUND
consvar_t surround = { "surround", "0", CV_SAVE, CV_OnOff };
#endif

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
static boolean mus_paused;

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

#ifdef SNDSERV
	CV_RegisterVar(&sndserver_cmd);
	CV_RegisterVar(&sndserver_arg);
#endif
#ifdef MUSSERV
	CV_RegisterVar(&musserver_cmd);
	CV_RegisterVar(&musserver_arg);
#endif
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

	if (gamemode == heretic)
		sprintf(namebuf, "%s", sfx->name);
	else
		sprintf(namebuf, "ds%s", sfx->name);

	sfxlump = W_CheckNumForName(namebuf);
	if (sfxlump > 0)
	{
		sfx->lumpnum = sfxlump;
		return sfxlump;
	}

	if (gamemode != heretic)
		sprintf(namebuf, "%s", sfx->name);
	else
		sprintf(namebuf, "ds%s", sfx->name);

	sfxlump = W_CheckNumForName(namebuf);
	
	if (sfxlump > 0)
	{
		sfx->lumpnum = sfxlump;
		return sfxlump;
	}

	if (gamemode == heretic)
		sfx->lumpnum = W_GetNumForName("keyup");
	else
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
	else if (gamemode == heretic)
		mnum = mus_he1m1 + (gameepisode - 1) * 9 + gamemap - 1;
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
		if (gamemode != heretic)
		{
			if (sfx_id >= sfx_sawup && sfx_id <= sfx_sawhit)
				pitch += 8 - (M_Random() & 15);
			else if (sfx_id != sfx_itemup && sfx_id != sfx_tink)
				pitch += 16 - (M_Random() & 31);
		}
		else
			pitch = 128 + (M_Random() & 7) - (M_Random() & 7);
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

	/*if (digmusic)
		I_SetFMODVolume(volume);
	else*/
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
	
	if (gamemode == heretic)
		I_RegisterSong(music->name);
	else
		I_RegisterSong(va("d_%s", music->name));
	
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

#ifdef MUSSERV

	if (msg_id != -1)
	{
		struct musmsg msg_buffer;

		msg_buffer.msg_type = 6;
		memset(msg_buffer.msg_text, 0, sizeof(msg_buffer.msg_text));
		sprintf(msg_buffer.msg_text, "d_%s", music->name);
		msgsnd(msg_id, (struct msgbuf *)&msg_buffer, sizeof(msg_buffer.msg_text), IPC_NOWAIT);
	}

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
	Int32* Volume,		// Volume of the sound (Distance) [Mono]
	Int32* Balance,		// Balance of the sound (left/right) [Stereo + Surround + Full Surround]
	Int32* Pitch,		// Change in pitch (Doppler!?!?) [All]
	Int32* Orientation,	// Balance of the sound (front/back) [Surround + Full Surround]
	Int32* FrontVolume	// How loud to play a sound for the front speaker [Full Surround]
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
		if (!stricmp(S_sfx[i].name, soundname))
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
