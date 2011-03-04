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
// -----------------------------------------------------------------------------
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
// DESCRIPTION: Music

#include "i_thread.h"
#include "i_sound.h"
#include "w_wad.h"
#include "z_zone.h"

typedef enum
{
	MUS_STOP,
	MUS_PLAY,
	MUS_PAUSE,
	MUS_REWIND,
	MUS_PREVTRACK,
	MUS_NEXTTRACK,
	MUS_TERMINATE,
	MUS_SLEEP,
	
	MAXMUSICCONTROL
} musiccontrol_t;

#define MUS_PUSHSTOPALLNOTES 0x40

#ifdef _WIN32
#define PLATFORMSUPPORTSMUSIC 1
#else
#define PLATFORMSUPPORTSMUSIC 0
#endif

static MusicReady = 0;
extern boolean nomusic;
int threadedmusic = 0;
uint8_t* MusicData = NULL;
size_t MusicLen = 0;
uint8_t* MusicPtr = NULL;
int MusicControl = MUS_STOP;
int MusicThread = 0;
int ThreadHandle = -2;

/********************************* MUSIC SETUP *********************************/
void I_InitMusic(void)
{
	CONS_Printf("I_InitMusic: Initializing music subsystem...\n");
	
	if (!PLATFORMSUPPORTSMUSIC)
	{
		CONS_Printf("I_InitMusic: Platform does not support music!\n");
		return;
	}
	
	// Do we use multi-threaded music?
	if (!THREAD_CheckSupport())
		threadedmusic = 1;
		
	if (threadedmusic)
	{
		CONS_Printf("I_InitMusic: Music will be multi-threaded.\n");
		
		if (THREAD_CreateThread(I_UpdateMusicThreaded))
		{
			CONS_Printf("I_InitMusic: Thread creation failed, falling back to single-threaded music.\n");
			threadedmusic = 0;
			MusicControl = MUS_SLEEP;
		}		
		else
			ThreadHandle = THREAD_GetThreadHandle();
	}
	
	if (!threadedmusic)
		CONS_Printf("I_InitMusic: Music will be single-threaded (may be choppy).\n");
}

void I_ShutdownMusic(void)
{
	CONS_Printf("I_ShutdownMusic: Shutting down music subsystem...\n");
	
	if (!PLATFORMSUPPORTSMUSIC)
	{
		CONS_Printf("I_ShutdownMusic: Platform does not support music!\n");
		return;
	}
	
	MusicControl = MUS_TERMINATE;
	
	if (threadedmusic)
		THREAD_KillThread(ThreadHandle);
}

int I_RegisterSong(char* lumpname)
{
	uint8_t* ptr = NULL;
	WadIndex_t lump = INVALIDLUMP;
	
	if (!PLATFORMSUPPORTSMUSIC)
	{
		CONS_Printf("I_RegisterSong: Platform does not support music!\n");
		return;
	}
	
	// See if it really exists
	lump = W_CheckNumForName(lumpname);
	
	if (lump == INVALIDLUMP)
	{
		CONS_Printf("I_RegisterSong: \"%s\" is an invalid lump.\n", lumpname);
		return 0;
	}
	
	// Cache it
	ptr = W_CacheLumpNum(lump, PU_MUSIC);
	
	if (!ptr)
	{
		CONS_Printf("I_RegisterSong: Could not cache lump \"%s\".\n", lumpname);
		return 0;
	}
	
	// Check the header
	switch (*((uint32_t*)ptr))
	{
		case 441668941:	// MUS 0x1A
			// if we are using multi-threaded music, we must wait
			if (threadedmusic && ThreadHandle > -1)
			{
				MusicThread = 1;
				while (MusicThread == 1);
			}
			
			// Remove old music first
			if (MusicData)
			{
				Z_Free(MusicData);
				MusicData = NULL;
				MusicControl = MUS_SLEEP;
			}
			
			// Copy in new music
			MusicLen = W_LumpLength(lump);
			MusicData = Z_Malloc(MusicLen, PU_MUSIC, &MusicData);
			MusicPtr = MusicData + (((uint16_t*)ptr)[3]);
			
			// GhostlyDeath says <October 19, 2008>:
			//     It took me a good 2 hours to find out why the MIDI code kept
			//     saying all zeros. Turns out I never copied the data over =/
			memcpy(MusicData, ptr, MusicLen);
			
			MusicControl |= MUS_PUSHSTOPALLNOTES;
			MusicControl = MUS_PLAY;
			
			MusicThread = 0;
			
			return 1;
			
		default:
			CONS_Printf("I_RegisterSong: Song format not known \"%04x\"\n", (*((uint32_t*)ptr)));
			return 0;
	}

	return 0;
}

void I_UnRegisterSong(int handle)
{
}

/******************************** MUSIC UPDATE *********************************/
#ifdef _WIN32
typedef union
{
	uint32_t word;
	uint8_t bytes[4];
} MIDIMsg_t;
#endif

void I_UpdateMusic(void)
{
	if (threadedmusic)
		return;
		
	if (MusicData)
	{
#ifdef _WIN32
#else
#endif
	}
}

#ifdef _WIN32
HMIDIOUT* hMidiOutPtr = NULL;
#endif

void* I_UpdateMusicThreaded(void* unused)
{
	size_t TimeDelay = 0;
	uint8_t Channel = 0;
	uint8_t Last = 0;
	uint8_t Event = 0;
	uint8_t ShortCircuit = 0;
	size_t i = 0;
	uint8_t ChanVol[16];
	uint8_t IgnoreNote = 0;
#ifdef _WIN32
	HMIDIOUT hMidiOut;
	MIDIMsg_t mData;
	
	midiOutOpen(&hMidiOut, MIDI_MAPPER, NULL, NULL, CALLBACK_NULL);
	
	hMidiOutPtr = &hMidiOut;
#endif

	CONS_Printf("I_UpdateMusicThreaded: Thread loaded!\n");

	for (; MusicControl != MUS_TERMINATE;)
	{
#ifdef _WIN32
		/* Normal Playback */
		if (!MusicThread)
		{
			if (MusicControl & MUS_PUSHSTOPALLNOTES)
			{
				memset(&mData, 0, sizeof(mData));
					
				for (i = 0; i < 16; i++)
				{
					mData.bytes[0] = 0xB0 | i;
					mData.bytes[1] = 0x78;
					midiOutShortMsg(hMidiOut, mData.word);
				}
					
				MusicControl &= ~MUS_PUSHSTOPALLNOTES;
				I_WaitVBL(100);
				continue;
			}
			
			switch (MusicControl)
			{
				case MUS_SLEEP:		// Fall asleep
				case MUS_STOP:		// Pause the music and go to the beginning
				case MUS_PAUSE:		// Pause the music
					memset(&mData, 0, sizeof(mData));
					
					for (i = 0; i < 16; i++)
					{
						mData.bytes[0] = 0xB0 | i;
						mData.bytes[1] = 0x78;
						midiOutShortMsg(hMidiOut, mData.word);
					}
					
					TimeDelay = 140;
					break;
			
				case MUS_PLAY:		// Play or continue to play the music
					if (MusicData && (MusicPtr < MusicData || MusicPtr >= MusicData + MusicLen))
						MusicPtr = MusicData + (((uint16_t*)MusicData)[3]);
						
					if (MusicData && MusicPtr)
					{
						memset(&mData, 0, sizeof(mData));
						
						for (;;)
						{
							Channel = (*MusicPtr) & 15;
							
							if (Channel == 9)
								Channel = 15;
							else if (Channel == 15)
								Channel = 9;
							
							Last = *MusicPtr & 128;
							Event = ((*MusicPtr & ~128) & 0xF0) >> 4;
							
							MusicPtr++;
							
							/*if (devparm)
								CONS_Printf("Debug :: Event: %i; Chan: %i; After [%02x %02x %02x]\n",
									Event, Channel,
									*MusicPtr, *(MusicPtr+1), *(MusicPtr+2));*/
							
							switch (Event)
							{
								case 0:		// Release Note
									mData.bytes[0] = 0x80 | Channel;
									mData.bytes[1] = (*MusicPtr) & 127;
									
									MusicPtr++;
									break;
									
								case 1:		// Play Note
									mData.bytes[0] = 0x90 | Channel;
									mData.bytes[1] = (*MusicPtr) & 0x7F;
									
									if (*MusicPtr & 0x80)
									{
										MusicPtr++;
										mData.bytes[2] = (*MusicPtr) & 127;
										ChanVol[Channel] = (*MusicPtr) & 127;
									}
									else
										mData.bytes[2] = ChanVol[Channel];
									
									MusicPtr++;
									break;
									
								case 2:		// Pitch Wheel
									mData.bytes[0] = 0xE0 | Channel;
									mData.bytes[1] = *MusicPtr;
									MusicPtr++;
									break;
									
								case 3:		// System Event
									mData.bytes[0] = 0xB0 | Channel;
									
									switch (*MusicPtr & 127)
									{
										case 10:
											mData.bytes[1] = 0x78;
											break;
										case 11:
											mData.bytes[1] = 0x7B;
											break;
										case 12:
											mData.bytes[1] = 0x7E;
											mData.bytes[2] = Channel;
											break;
										case 13:
											mData.bytes[1] = 0x7F;
											break;
										case 14:
											mData.bytes[1] = 0x79;
											break;
										default:
											if (devparm)
												CONS_Printf("System Unknown: %i [%02x %02x >%02x< %02x %02x]\n",
													*MusicPtr & 127,
													*(MusicPtr-2), *(MusicPtr-1), *MusicPtr, *(MusicPtr+1), *(MusicPtr+2));
											IgnoreNote = 1;
											break;
									}
									
									MusicPtr++;
									break;
									
								case 4:		// Change Controller
									i = 1;
									
									switch (*MusicPtr & 127)
									{
										case 0:	// Change Program
											mData.bytes[0] = 0xC0 | Channel;
											if (devparm)
												CONS_Printf("Program Change: %02X %02X [Chan %i to Prog %i]\n",
													*MusicPtr, *(MusicPtr+1), Channel, *(MusicPtr+1));
											break;
											
										case 1: // Bank Select
											mData.bytes[0] = 0xB0 | Channel;
											mData.bytes[1] = 0x00;
											i++;
											break;
											
										case 3:	// Volume
											mData.bytes[0] = 0xB0 | Channel;
											mData.bytes[1] = 0x07;
											i++;
											break;
											
										case 4:	// Pan
											mData.bytes[0] = 0xB0 | Channel;
											mData.bytes[1] = 0x0A;
											i++;
											break;
											
										default:
											if (devparm)
												CONS_Printf("Controller Unknown: %i [%02x %02x >%02x< %02x %02x]\n",
													*MusicPtr & 127,
													*(MusicPtr-2), *(MusicPtr-1), *MusicPtr, *(MusicPtr+1), *(MusicPtr+2));
											mData.bytes[0] = 0xB0 | Channel;
											i++;
											IgnoreNote = 1;
											break;
									}
									
									MusicPtr++;
									mData.bytes[i] = *MusicPtr & 127;
									
									MusicPtr++;
									break;
									
								case 6:		// Score End
									if (devparm)
										CONS_Printf("Score End: %i [%02x %02x >%02x< %02x %02x]\n",
											Event,
											*(MusicPtr-2), *(MusicPtr-1), *MusicPtr, *(MusicPtr+1), *(MusicPtr+2));
									MusicPtr = MusicData + (((uint16_t*)MusicData)[3]);
									ShortCircuit = 1;
									break;
									
								default:
									if (devparm)
										CONS_Printf("Event Unknown: %i [%02x %02x >%02x< %02x %02x]\n", Event,
											*(MusicPtr-2), *(MusicPtr-1), *MusicPtr, *(MusicPtr+1), *(MusicPtr+2));
									IgnoreNote = 1;
									break;
							}
							
							if (ShortCircuit)
							{
								ShortCircuit = 0;
								break;
							}
							
							if (Last)
							{
								TimeDelay = 0;
								
								// I DON'T GET IT!
								/*do
								{
									//TimeDelay *= 128;
									TimeDelay += (*MusicPtr) & 127;
									MusicPtr++;
								} while ((*MusicPtr) & 128);*/
								
								while ((*MusicPtr) & 128)
								{
									TimeDelay *= 128;
									TimeDelay += (*MusicPtr) & 127;
									MusicPtr++;
								}
								
								TimeDelay *= 128;
								TimeDelay += (*MusicPtr) & 127;
								MusicPtr++;
							}
							
							/*CONS_Printf("%04X = [%02x %02x %02x >%02x< %02x %02x %02x]\n", mData.word,
								*(MusicPtr-3), *(MusicPtr-2), *(MusicPtr-1),
								*MusicPtr,
								*(MusicPtr+1), *(MusicPtr+2), *(MusicPtr+3));*/
								
							if (!IgnoreNote)
								midiOutShortMsg(hMidiOut, mData.word);
							else
								IgnoreNote = 0;

							if (Last)
								break;
						}
					}
					break;
					
				case MUS_REWIND:	// Go to the beginning of the music and continue playing
					if (MusicData)
						MusicPtr = MusicData + (((uint16_t*)MusicData)[3]);
					break;
					
				default:
					break;
			}
			
			I_WaitVBL(7 * TimeDelay);//100 * (TimeDelay >> 3));
		}
		
		/* A function is messing with the music so wait */
		else
		{
			// Inform that we are ready or not
			if (MusicThread == 1)
				MusicThread = 2;
			else
				I_WaitVBL(500);
		}
#else
#endif
	}
	
#ifdef _WIN32
	midiOutClose(&hMidiOut);
#endif

	CONS_Printf("I_UpdateMusicThreaded: Thread ended!\n");
	
	return NULL;
}

/****************************** MUSIC MANIPULATION *****************************/

void I_PauseSong(int handle)
{
	MusicControl = MUS_PAUSE;
}

void I_ResumeSong(int handle)
{
	MusicControl = MUS_PLAY;
}

void I_PlaySong(int handle, int looping)
{
	MusicControl = MUS_PLAY;
}

void I_StopSong(int handle)
{
	MusicControl = MUS_STOP;
}

void I_SetMusicVolume(int volume)
{
#ifdef _WIN32
	// 0 = mute, 31 = full
	if (hMidiOutPtr && *hMidiOutPtr)
		midiOutSetVolume(*hMidiOutPtr,
			((int)((double)0xFFFF * ((double)volume / 32.0)) << 16) |	// One Channel
			((int)((double)0xFFFF * ((double)volume / 32.0)))	// The other
			);
#endif
}

/* DEPRECATED FUNCTIONS */
void I_SetFMODVolume(int volume)
{
}

