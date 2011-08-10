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
// DESCRIPTION: Common Interface Utilities (to reduce code bloat and dup)

/***************
*** INCLUDES ***
***************/

/* System */
// On UNIX include the standard header
#if defined(__unix__)
	#include <unistd.h>			// Standard Stuff
#endif

// On Windows include windows.h
#if defined(_WIN32)
	#include <windows.h>
#endif

// On DOS include dos.h (and conio.h for colors)
#if defined(__MSDOS__)
	#include <dos.h>
	#include <conio.h>
#endif

/* Local */
#include "i_util.h"
#include "i_joy.h"
#include "i_system.h"
#include "i_video.h"
#include "command.h"
#include "screen.h"
#include "g_input.h"
#include "w_wad.h"
#include "doomstat.h"

/****************
*** CONSTANTS ***
****************/

#define EVENTQUEUESIZE		64					// Max events allowed in queue
#define MODENAMELENGTH		16					// Length of mode name
#define MAX_QUIT_FUNCS		16					// Max number of quit functions

/**************
*** GLOBALS ***
**************/

JoyType_t Joystick;

/* i_video.c -- Remove this garbage */
consvar_t cv_vidwait = {"vid_wait","1",CV_SAVE,CV_OnOff};
uint8_t graphics_started = 0;
bool_t allow_fullscreen = false;

/* i_sound.c -- Remove this garbage */
consvar_t cv_snd_speakersetup = {"snd_speakersetup", "2", CV_SAVE};
consvar_t cv_snd_soundquality = {"snd_soundquality", "11025", CV_SAVE};
consvar_t cv_snd_sounddensity = {"snd_sounddensity", "1", CV_SAVE};
consvar_t cv_snd_pcspeakerwave = {"snd_pcspeakerwave", "1", CV_SAVE};
consvar_t cv_snd_channels = {"snd_numchannels", "16", CV_SAVE};
consvar_t cv_snd_reservedchannels = {"snd_reservedchannels", "4", CV_SAVE};
consvar_t cv_snd_multithreaded = {"snd_multithreaded", "1", CV_SAVE};
consvar_t cv_snd_output = {"snd_output", "Default", CV_SAVE};
consvar_t cv_snd_device = {"snd_device", "auto", CV_SAVE};

/* i_cdmus.c -- Remove this garbage */
consvar_t cd_volume = { "cd_volume", "31", CV_SAVE};
consvar_t cdUpdate = { "cd_update", "1", CV_SAVE };

/*****************
*** STRUCTURES ***
*****************/

/* I_VideoMode_t -- A video mode */
typedef struct I_VideoMode_s
{
	uint16_t Width;								// Screen Width
	uint16_t Height;							// Screen height
	char Name[MODENAMELENGTH];					// Mode name
} I_VideoMode_t;

/* I_LocalMusic_t -- Local music data */
typedef struct I_LocalMusic_s
{
	I_MusicType_t Type;							// Type of song this is
	int Handle;									// Song handle
	I_MusicDriver_t* Driver;					// Driver to play with
	int DriverHandle;							// Handle known by driver
	uint32_t Length;							// Length of song in tics
	WX_WADEntry_t* Entry;						// Entry of song
	size_t EntryLength;							// Length of entry
	uint8_t* Data;								// Loaded Data
	char* PathName;								// Path to file on disk (if it exists)
	bool_t Playing;								// Is the song playing?
} I_LocalMusic_t;

/* I_MUS2MIDData_t -- MIDI Data */
typedef struct I_MUS2MIDData_s
{
	I_MusicDriver_t* RealDriver;
	int RealHandle;
	int LocalHandle;
	bool_t FeedMessages;
	uint8_t* Data;
	size_t Size;
	size_t Pos;
	uint8_t* MIDIData;
	size_t MIDISize;
	uint32_t DeltaTotal;
	uint32_t LocalDelta;
	bool_t Playing;
	bool_t PausePlay;
	int Special;
	tic_t LocalTime;
	tic_t NextRun;
	tic_t BaseTime;
	uint8_t Vols[16];
	
	size_t MusStart;
	size_t MusLen;
} I_MUS2MIDData_t;

/*************
*** LOCALS ***
*************/

static I_VideoMode_t* l_Modes = NULL;			// Video Modes
static size_t l_NumModes = 0;					// Number of video modes

static I_EventEx_t l_EventQ[EVENTQUEUESIZE];	// Events in queue
static size_t l_EQRead = 0;						// Read position in queue
static size_t l_EQWrite = 0;					// Write position in queue

typedef void (*quitfuncptr) ();
static quitfuncptr quit_funcs[MAX_QUIT_FUNCS];

static I_MusicDriver_t** l_MusicDrivers;		// Music drivers
static size_t l_NumMusicDrivers;				// Number of music drivers

static I_SoundDriver_t** l_SoundDrivers;		// Sound drivers
static size_t l_NumSoundDrivers;				// Number of sound drivers

static I_LocalMusic_t* l_LocalSongs;			// Local songs
static size_t l_NumLocalSongs;					// Number of local songs

/*****************************
*** MUS2MID VIRTUAL DRIVER ***
*****************************/

/* I_MUS2MID_MUSReadNextMessage() -- Reads the next message from a MIDI */
// a_OutData: The 4 byte midi message
// a_OutSize: How many bytes of the 4 byte message used
// a_Delta: Milliseconds to not do anything after this message
bool_t I_MUS2MID_MUSReadNextMessage(I_MUS2MIDData_t* const a_Local, uint32_t* const a_OutData, size_t* const a_OutSize, uint32_t* const a_Delta)
{
	void* p;
	uint8_t NoteBit[4];
	bool_t Last;
	uint8_t Channel;
	uint8_t Event;
	size_t i;
	uint8_t Key;
	
	union
	{
		uint32_t u;
		uint8_t b[4];
	} MIDIMsg;
	
	/* Check */
	if (!a_Local || !a_OutData || !a_OutSize || !a_Delta)
		return false;
	
	/* Clear */
	*a_OutData = 0;
	*a_OutSize = 0;
	*a_Delta = 0;
	MIDIMsg.u = 0;
	
	/* If the current location is 0x0, init MUS */
	if (a_Local->Pos == 0)
	{
		// Prepare
		p = a_Local->Data;
		ReadUInt32((uint32_t** const)&p);	// skip header
		a_Local->MusLen = LittleReadUInt16((uint16_t** const)&p);
		a_Local->MusStart = LittleReadUInt16((uint16_t** const)&p);
		
		// Jump to start location
		a_Local->Pos = a_Local->MusStart;
	}
	
	/* If we exceeded MIDI bounds */
	if (a_Local->Pos >= a_Local->Size)
	{
		// Reset to start
		a_Local->Pos = a_Local->MusStart;
		*a_Delta = 1;
		return true;
	}
	
	/* Otherwise read the current note */
	NoteBit[0] = a_Local->Data[a_Local->Pos++];
	
	// Is this the last event?
	if (NoteBit[0] & 0x80)
		Last = true;
	else
		Last = false;
		
	// Which channel are we operating on?
	Channel = NoteBit[0] & 0xF;
	
	// Switch channels 9 and 15
	if (Channel == 9)
		Channel = 15;
	else if (Channel == 15)
		Channel = 9;
	
	// Which type of event is this?
	Event = (NoteBit[0] & 0x70) >> 4;
	
	/* Read more bits */
	if (Event < 5)	// Every event has all the stuff
		NoteBit[1] = a_Local->Data[a_Local->Pos++];
	
	if (Event == 4)	// note on and controller are 3-bytes
		NoteBit[2] = a_Local->Data[a_Local->Pos++];
	
	/* Now handle event */
	switch (Event)
	{
			// Release a note
		case 0:
			MIDIMsg.b[0] = 0x80 | Channel;
			MIDIMsg.b[1] = NoteBit[1] & 0x7F;
			MIDIMsg.b[2] = 127;	// always 127 (note off)
			*a_OutSize = 3;
			break;
			
			// Plays a note
		case 1:
			MIDIMsg.b[0] = 0x90 | Channel;
			MIDIMsg.b[1] = NoteBit[1] & 0x7F;
			
			// Volume set?
			if (NoteBit[1] & 0x80)
			{
				NoteBit[2] = a_Local->Data[a_Local->Pos++];
				a_Local->Vols[Channel] = MIDIMsg.b[2] = NoteBit[2] & 0x7F;
			}
			else
				MIDIMsg.b[2] = a_Local->Vols[Channel];	// use last volume
			*a_OutSize = 3;
			break;
			
			// Pitch wheel
		case 2:
			MIDIMsg.b[0] = 0xE0 | Channel;
			MIDIMsg.b[1] = NoteBit[1] & 0x7F;
			MIDIMsg.b[2] = 0;
			*a_OutSize = 3;
			break;
			
			// System Event
		case 3:
			Key = (NoteBit[1] & 0x7F);
			
			// Always set this, it is always the same
			MIDIMsg.b[0] = 0xB0 | Channel;
			*a_OutSize = 3;
			
			// Depending on which event
			switch (Key)
			{
					// All sounds off
				case 10:
					MIDIMsg.b[1] = 0x78;
					break;
					
					// All notes off
				case 11:
					MIDIMsg.b[1] = 0x7B;
					break;
					
					// Mono
				case 12:
					MIDIMsg.b[1] = 0x7E;
					break;
					
					// Poly
				case 13:
					MIDIMsg.b[1] = 0x7F;
					break;
					
					// Reset all controllers
				case 14:
					MIDIMsg.b[1] = 0x79;
					break;
			}
			break;
			
			// Change controller
		case 4:
			Key = (NoteBit[1] & 0x7F);
			
			// Program change
			if (Key == 0)
			{
				MIDIMsg.b[0] = 0xC0 | Channel;
				MIDIMsg.b[1] = NoteBit[2] & 0x7F;
				*a_OutSize = 2;
			}
			
			// Everything but program change
			else
			{
				// Always set this, it is always the same
				MIDIMsg.b[0] = 0xB0 | Channel;
				MIDIMsg.b[2] = NoteBit[2] & 0x7F;
				*a_OutSize = 3;
				
				// Which?
				switch (Key)
				{
						// Bank select
					case 1:
						MIDIMsg.b[1] = 0x00;
						break;
						
						// Vibrato
					case 2:
						MIDIMsg.b[1] = 0x01;
						break;
					
						// Volume
					case 3:
						MIDIMsg.b[1] = 0x07;
						break;
					
						// Pan
					case 4:
						MIDIMsg.b[1] = 0x0A;
						break;
					
						// Expression
					case 5:
						MIDIMsg.b[1] = 0x0B;
						break;
					
						// Reverb
					case 6:
						MIDIMsg.b[1] = 0x5B;
						break;
					
						// Chorus
					case 7:
						MIDIMsg.b[1] = 0x5D;
						break;
						
						// Sustain
					case 8:
						MIDIMsg.b[1] = 0x40;
						break;
					
						// Soft pedal
					case 9:
						MIDIMsg.b[1] = 0x43;
						break;
				}
			}
			break;
			
			// Reset song
		case 6:
			Last = false;						// Clear last, don't want to handle it
			*a_Delta = 1;						// Break from loop, kinda
			a_Local->Pos = a_Local->MusStart;	// Back to start
			break;
		
			// Unknown
		default:
			if (devparm)
				CONS_Printf("I_MUS2MID_MUSReadNextMessage: Unknown %i\n", Event);
			break;
	}
	
	/* If this is the last event */
	while (Last)
	{
		// Read in byte
		NoteBit[0] = a_Local->Data[a_Local->Pos++];
		
		// Is there last?
		Last = (NoteBit[0] & 0x80);
		
		// Add the current time to delay
		*a_Delta *= 128;
		*a_Delta += NoteBit[0] & 0x7F;
	}
	
	/* Convert to MS time */
	// 1000 / 140 = 7.142857143
	*a_Delta = *a_Delta * 7;//(*a_Delta * 1000) / 140;
	
	/* Success */
	*a_OutData = MIDIMsg.u;
	return true;
}

/* I_MUS2MID_Init() -- Initializes the MUS2MID Driver */
bool_t I_MUS2MID_Init(struct I_MusicDriver_s* const a_Driver)
{
	I_MusicDriver_t* MIDIDriver;
	I_MUS2MIDData_t* Local;
	
	/* Check */
	if (!a_Driver)
		return false;
	
	/* Try to find a driver that can handle MIDI */
	MIDIDriver = I_FindMusicDriver(IMT_MIDI);
	
	// Not found?
	if (!MIDIDriver)
		return false;
	
	/* Otherwise allocate data for MUS2MID converter */
	a_Driver->Size = sizeof(*Local);
	Local = a_Driver->Data = Z_Malloc(a_Driver->Size, PU_STATIC, NULL);
	
	// Set the driver
	Local->RealDriver = MIDIDriver;
	
	/* If the driver supports messaging, we can just play MUSes as it */
	if (Local->RealDriver->RawMIDI)
		Local->FeedMessages = true;
	
	// Otherwise we have to convert to a full MIDI then pipe it through
	else
		Local->FeedMessages = false;
	
	/* Feeding? */
	if (Local->FeedMessages)
		CONS_Printf("I_MUS2MID_Init: Feeding messages into %s.\n", Local->RealDriver->Name);
	
	return true;
}

/* I_MUS2MID_Destroy() -- Destroys a driver */
bool_t I_MUS2MID_Destroy(struct I_MusicDriver_s* const a_Driver)
{
	I_MUS2MIDData_t* Local;
	
	/* Check */
	if (!a_Driver)
		return false;
	
	/* Get local */
	Local = a_Driver->Data;
	
	// Check
	if (!Local)
		return false;
	
	/* Clear local stuff */
	
	/* Clear allocation */
	if (a_Driver->Data)
		Z_Free(a_Driver->Data);
	a_Driver->Data = NULL;
	a_Driver->Size = 0;
	
	return true;
}

/* I_MUS2MID_Success() -- Success */
void I_MUS2MID_Success(struct I_MusicDriver_s* const a_Driver)
{
	I_MUS2MIDData_t* Local;
	
	/* Check */
	if (!a_Driver)
		return;
	
	/* Get local */
	Local = a_Driver->Data;
	
	// Check
	if (!Local)
		return;
}

/* I_MUS2MID_Pause() -- Pauses a song (pause ||) */
void I_MUS2MID_Pause(struct I_MusicDriver_s* const a_Driver, const int a_Handle)
{
	I_MUS2MIDData_t* Local;
	size_t i;
	union
	{
		uint32_t u;
		uint8_t b[4];
	} MIDIMsg;
	
	/* Check */
	if (!a_Driver)
		return;
	
	/* Get local */
	Local = a_Driver->Data;
	
	// Check
	if (!Local)
		return;
	
	// Check handle
	if (a_Handle != Local->LocalHandle)
		return;
	
	/* Feeder mode */
	if (Local->FeedMessages)
	{
		// Reset base time and set paused
		Local->BaseTime = 0;
		Local->PausePlay = true;
		
		// End everything pretty much
		for (i = 0; i < 16; i++)
		{
			// Turn off all notes
			MIDIMsg.u = 0;
			MIDIMsg.b[0] = 0xB0 | i;
			MIDIMsg.b[1] = 0x7B;
			MIDIMsg.b[2] = 0;
			Local->RealDriver->RawMIDI(Local->RealDriver, MIDIMsg.u, 3);
		}
	}
	
	/* Full convert mode */
	else
	{
	}
}

/* I_MUS2MID_Resume() -- Resumes a song (play >) */
void I_MUS2MID_Resume(struct I_MusicDriver_s* const a_Driver, const int a_Handle)
{
	I_MUS2MIDData_t* Local;
	
	/* Check */
	if (!a_Driver)
		return;
	
	/* Get local */
	Local = a_Driver->Data;
	
	// Check
	if (!Local)
		return;
	
	// Check handle
	if (a_Handle != Local->LocalHandle)
		return;
	
	// Not playing?
	if (!Local->Playing)
		return;
	
	/* Feeder mode */
	if (Local->FeedMessages)
	{
		Local->BaseTime = 0;
		Local->PausePlay = false;
	}
	
	/* Full convert mode */
	else
	{
	}
}

/* I_MUS2MID_Stop() -- Stops a song from playing and seeks to start (stop []) */
void I_MUS2MID_Stop(struct I_MusicDriver_s* const a_Driver, const int a_Handle)
{
	I_MUS2MIDData_t* Local;
	size_t i;
	union
	{
		uint32_t u;
		uint8_t b[4];
	} MIDIMsg;
	
	/* Check */
	if (!a_Driver)
		return;
	
	/* Get local */
	Local = a_Driver->Data;
	
	// Check
	if (!Local)
		return;
	
	// Check handle
	if (a_Handle != Local->LocalHandle)
		return;
	
	/* Feeder mode */
	if (Local->FeedMessages)
	{
		// Clear time
		Local->BaseTime = 0;
		
		// End everything pretty much
		for (i = 0; i < 16; i++)
		{
			// Turn off all notes
			MIDIMsg.u = 0;
			MIDIMsg.b[0] = 0xB0 | i;
			MIDIMsg.b[1] = 0x7B;
			MIDIMsg.b[2] = 0;
			Local->RealDriver->RawMIDI(Local->RealDriver, MIDIMsg.u, 3);
			
			// Turn off sustain
			MIDIMsg.u = 0;
			MIDIMsg.b[0] = 0xB0 | i;
			MIDIMsg.b[1] = 0x40;
			MIDIMsg.b[2] = 0;
			Local->RealDriver->RawMIDI(Local->RealDriver, MIDIMsg.u, 3);
			
			// Reset all controllers
			MIDIMsg.u = 0;
			MIDIMsg.b[0] = 0xB0 | i;
			MIDIMsg.b[1] = 0x79;
			MIDIMsg.b[2] = 0;
			Local->RealDriver->RawMIDI(Local->RealDriver, MIDIMsg.u, 3);
		}
	}
	
	/* Full convert mode */
	else
	{
	}
	
	/* No longer playing */
	Local->Playing = false;
}

/* I_MUS2MID_Lengt() -- Length of song */
uint32_t I_MUS2MID_Length(struct I_MusicDriver_s* const a_Driver, const int a_Handle)
{
	I_MUS2MIDData_t* Local;
	
	/* Check */
	if (!a_Driver)
		return 0;
	
	/* Get local */
	Local = a_Driver->Data;
	
	// Check
	if (!Local)
		return 0;
	
	// Check handle
	if (a_Handle != Local->LocalHandle)
		return;
	
	/* Feeder mode */
	if (Local->FeedMessages)
	{
	}
	
	/* Full convert mode */
	else
	{
	}
	
	return 0;
}

/* I_MUS2MID_Seek() -- Seeks to a new position */
void I_MUS2MID_Seek(struct I_MusicDriver_s* const a_Driver, const int a_Handle, const uint32_t a_Pos)
{
	I_MUS2MIDData_t* Local;
	
	/* Check */
	if (!a_Driver)
		return;
	
	/* Get local */
	Local = a_Driver->Data;
	
	// Check
	if (!Local)
		return;
	
	// Check handle
	if (a_Handle != Local->LocalHandle)
		return;
	
	/* Feeder mode */
	if (Local->FeedMessages)
	{
	}
	
	/* Full convert mode */
	else
	{
	}
}

/* I_MUS2MID_Play() -- Plays a song */
int I_MUS2MID_Play(struct I_MusicDriver_s* const a_Driver, const void* const a_Data, const size_t a_Size, const bool_t Loop)
{
	I_MUS2MIDData_t* Local;
	size_t i;
	union
	{
		uint32_t u;
		uint8_t b[4];
	} MIDIMsg;
	
	/* Check */
	if (!a_Driver)
		return 0;
	
	/* Get local */
	Local = a_Driver->Data;
	
	// Check
	if (!Local)
		return;
	
	if (devparm)
		CONS_Printf("I_MUS2MID_Play: Converting MUS to MIDI.\n");
	
	/* Basic Init */
	Local->LocalDelta = 0;
	if (Local->MIDIData)
		Z_Free(Local->MIDIData);
	Local->MIDIData = NULL;
	Local->MIDISize = 0;
	Local->DeltaTotal = 0;
	Local->Special = 0;
	Local->LocalTime = 0;
	Local->BaseTime = 0;
	Local->NextRun = 0;
	Local->Pos = 0;
		Local->Data = a_Data;
		Local->Size = a_Size;
	
	// remember volumes
	for (i = 0; i < 16; i++)
		Local->Vols[i] = 127;
	
	/* Feeder mode */
	if (Local->FeedMessages)
	{
		// End everything pretty much
		for (i = 0; i < 16; i++)
		{
			// Turn off all notes
			MIDIMsg.u = 0;
			MIDIMsg.b[0] = 0xB0 | i;
			MIDIMsg.b[1] = 0x7B;
			MIDIMsg.b[2] = 0;
			Local->RealDriver->RawMIDI(Local->RealDriver, MIDIMsg.u, 3);
			
			// Turn off sustain
			MIDIMsg.u = 0;
			MIDIMsg.b[0] = 0xB0 | i;
			MIDIMsg.b[1] = 0x40;
			MIDIMsg.b[2] = 0;
			Local->RealDriver->RawMIDI(Local->RealDriver, MIDIMsg.u, 3);
			
			// Reset all controllers
			MIDIMsg.u = 0;
			MIDIMsg.b[0] = 0xB0 | i;
			MIDIMsg.b[1] = 0x79;
			MIDIMsg.b[2] = 0;
			Local->RealDriver->RawMIDI(Local->RealDriver, MIDIMsg.u, 3);
		}
	}
	
	/* Full convert mode */
	else
	{
	}
	
	/* Playing */
	Local->Playing = true;
	
	return ++Local->LocalHandle;
}

/* I_MUS2MID_Volume() -- Changes volume */
void I_MUS2MID_Volume(struct I_MusicDriver_s* const a_Driver, const int a_Handle, const uint8_t Vol)
{
	I_MUS2MIDData_t* Local;
	
	/* Check */
	if (!a_Driver)
		return;
	
	/* Get local */
	Local = a_Driver->Data;
	
	// Check
	if (!Local)
		return;
	
	// Check handle
	if (a_Handle != Local->LocalHandle)
		return;
	
	/* Feeder mode */
	if (Local->FeedMessages)
	{
	}
	
	/* Full convert mode */
	else
	{
	}
}

/* I_MUS2MID_Update() -- Updates playing music */
void I_MUS2MID_Update(struct I_MusicDriver_s* const a_Driver, const tic_t a_Tics)
{
	I_MUS2MIDData_t* Local;
	uint32_t Out;
	size_t OutSize;
	uint32_t Delta;
	uint32_t PassedTime;
	tic_t Distance;
	uint32_t MSTime;
	
	/* Check */
	if (!a_Driver)
		return;
	
	/* Get local */
	Local = a_Driver->Data;
	
	// Check
	if (!Local)
		return;
		
	/* Not playing or paused? */
	if (!Local->Playing || Local->PausePlay)
		return;
	
	/* Set stuff */
	if (!Local->BaseTime)
	{
		Local->LocalTime = a_Tics * TICRATE;
		Local->BaseTime = a_Tics;
	}
	
	/* Feeder mode */
	if (Local->FeedMessages)
	{
		// Get time in millis
		MSTime = a_Tics * TICRATE;
		
		// Constant play loop
		while (Local->LocalTime <= MSTime)
		{
			// Read a message
			if (I_MUS2MID_MUSReadNextMessage(Local, &Out, &OutSize, &Delta))
			{
				// Send message
				Local->RealDriver->RawMIDI(Local->RealDriver, Out, OutSize);
				
				// Add Delta to the local time
				Local->LocalTime += Delta;
			}
			else	// Something went bad =(
				Local->LocalTime = MSTime;
		}
	}
	
	/* Full convert mode */
	else
	{
	}
}

/* I_MusicDriver_t -- Driver for playing Music */
static I_MusicDriver_t l_MUS2MIDDriver =
{
	/* Data */
	"ReMooD MUS2MID",
	"mustomid",
	1 << IMT_MUS,
	false,
	50,
	
	/* Handlers */
	I_MUS2MID_Init,
	I_MUS2MID_Destroy,
	I_MUS2MID_Success,
	I_MUS2MID_Pause,
	I_MUS2MID_Resume,
	I_MUS2MID_Stop,
	I_MUS2MID_Length,
	I_MUS2MID_Seek,
	I_MUS2MID_Play,
	I_MUS2MID_Volume,
	NULL,
	I_MUS2MID_Update
};

/****************
*** FUNCTIONS ***
****************/

/* I_EventExPush() -- Pushes an event to the queue */
void I_EventExPush(const I_EventEx_t* const a_Event)
{
	/* Check */
	if (!a_Event)
		return;
	
	/* Write at current write pos */
	l_EventQ[l_EQWrite++] = *a_Event;
	
	// Overlap?
	if (l_EQWrite >= EVENTQUEUESIZE)
		l_EQWrite = 0;
	
	// Got too many events in Q?
	if (l_EQWrite == l_EQRead)
	{
		// Increment reader
		l_EQRead++;
		
		// Reader overlap?
		if (l_EQRead >= EVENTQUEUESIZE)
			l_EQRead = 0;
	}
}

/* I_EventExPop() -- Pops event from the queue */
bool_t I_EventExPop(I_EventEx_t* const a_Event)
{
	/* Determine whether something is in the queue currently */
	if (l_EQRead == l_EQWrite)
		return false;	// Nothing!
	
	/* If event was passed, copy */
	if (a_Event)
		*a_Event = l_EventQ[l_EQRead];
	
	/* Remove from queue */
	// Wipe
	memset(&l_EventQ[l_EQRead], 0, sizeof(l_EventQ[l_EQRead]));
	
	// Increment
	l_EQRead++;
	
	// Overlap?
	if (l_EQRead >= EVENTQUEUESIZE)
		l_EQRead = 0;
	
	/* Something was there */
	return true;
}

/* I_OsPolling() -- Handles operating system polling (all of it) */
void I_OsPolling(void)
{
	I_EventEx_t Event;
	
	/* Just read all events */
	I_GetEvent();
	
	/* Translate events to old Doom events */
	while (I_EventExPop(&Event))
		I_EventToOldDoom(&Event);
}

/* IS_NewKeyToOldKey() -- Converts a new key to an old key */
static int IS_NewKeyToOldKey(const uint8_t a_New)
{
	/* Giant Switch */
	switch (a_New)
	{
		case IKBK_NULL:			return 0;
		case IKBK_BACKSPACE:	return KEY_BACKSPACE;
		case IKBK_TAB:			return KEY_TAB;
		case IKBK_RETURN:		return KEY_ENTER;
		case IKBK_SHIFT:		return KEY_SHIFT;
		case IKBK_CTRL:			return KEY_CTRL;
		case IKBK_ALT:			return KEY_ALT;
		case IKBK_ESCAPE:		return KEY_ESCAPE;
		case IKBK_UP:			return KEY_UPARROW;
		case IKBK_DOWN:			return KEY_DOWNARROW;
		case IKBK_LEFT:			return KEY_LEFTARROW;
		case IKBK_RIGHT:		return KEY_RIGHTARROW;
		case IKBK_DELETE:		return KEY_DEL;
		case IKBK_HOME:			return KEY_HOME;
		case IKBK_END:			return KEY_END;
		case IKBK_INSERT:		return KEY_INS;
		case IKBK_PAGEUP:		return KEY_PGUP;
		case IKBK_PAGEDOWN:		return KEY_PGDN;
		//case IKBK_PRINTSCREEN:	return KEY_;
		case IKBK_NUMLOCK:		return KEY_NUMLOCK;
		case IKBK_CAPSLOCK:		return KEY_CAPSLOCK;
		case IKBK_SCROLLLOCK:	return KEY_SCROLLLOCK;
		case IKBK_PAUSE:		return KEY_PAUSE;
		case IKBK_NUM0:			return KEY_KEYPAD0;
		case IKBK_NUM1:			return KEY_KEYPAD1;
		case IKBK_NUM2:			return KEY_KEYPAD2;
		case IKBK_NUM3:			return KEY_KEYPAD3;
		case IKBK_NUM4:			return KEY_KEYPAD4;
		case IKBK_NUM5:			return KEY_KEYPAD5;
		case IKBK_NUM6:			return KEY_KEYPAD6;
		case IKBK_NUM7:			return KEY_KEYPAD7;
		case IKBK_NUM8:			return KEY_KEYPAD8;
		case IKBK_NUM9:			return KEY_KEYPAD9;
		case IKBK_NUMDIVIDE:	return KEY_KPADSLASH;
		case IKBK_NUMMULTIPLY:	return '*';
		case IKBK_NUMSUBTRACT:	return KEY_MINUSPAD;
		case IKBK_NUMADD:		return KEY_PLUSPAD;
		case IKBK_NUMENTER:		return KEY_ENTER;
		case IKBK_NUMPERIOD:	return '.';
		case IKBK_NUMDELETE:	return KEY_KPADDEL;
		case IKBK_WINDOWSKEY:	return KEY_LEFTWIN;
		case IKBK_MENUKEY:		return KEY_MENU;
		
			// Ranges
		default:
			// Letters (The game uses lowercase here)
			if (a_New >= IKBK_A && a_New <= IKBK_Z)
				return 'a' + (a_New - IKBK_A);
			
			// Normal ASCII
			else if (a_New >= IKBK_SPACE && a_New <= IKBK_TILDE)
				return ' ' + (a_New - IKBK_SPACE);
			
			// Function keys
			else if (a_New >= IKBK_F1 && a_New <= IKBK_F12)
				return KEY_F1 + (a_New - IKBK_F1);
			break;
	}
	
	/* Unknown */
	return 0;
}

/* I_EventToOldDoom() -- Converts an extended event to the old format */
void I_EventToOldDoom(const I_EventEx_t* const a_Event)
{
	event_t SendEvent;
	
	/* Check */
	if (!a_Event)
		return;
	
	/* Which event type? */
	switch (a_Event->Type)
	{
			// Keyboard
		case IET_KEYBOARD:
			// Ignore repeated keys
			if (a_Event->Data.Keyboard.Repeat)
				return;
			
			// Convert
			SendEvent.type = (a_Event->Data.Keyboard.Down ? ev_keydown : ev_keyup);
			SendEvent.data1 = IS_NewKeyToOldKey(a_Event->Data.Keyboard.KeyCode);
			SendEvent.typekey = a_Event->Data.Keyboard.Character;
			
			if (!SendEvent.data1)
				return;
			break;
			
			// Unknown
		default:
			return;
	}
	
	/* Send event */
	D_PostEvent(&SendEvent);
}

/* VID_NumModes() -- Returns the number of video modes */
int VID_NumModes(void)
{
   return l_NumModes;
}

/* VID_GetModeName() -- Gets the name of the video modes */
char* __REMOOD_DEPRECATED VID_GetModeName(int a_ModeNum)
{
	/* Check */
	if (a_ModeNum < 0 || a_ModeNum >= l_NumModes)
		return NULL;
	return l_Modes[a_ModeNum].Name;
}

/* VID_ClosestMode() -- Returns the closest mode against width and height */
// Ignore fullscreen for now
int VID_ClosestMode(int* const a_WidthP, int* const a_HeightP, const bool_t a_Fullscreen)
{
	size_t i, BestMode;
	
	/* Check */
	if (!a_WidthP || !a_HeightP || !l_NumModes)
		return 0;
	
	/* Go through list */
	for (BestMode = 0, i = 0; i < l_NumModes; i++)
		// Width matches
		if (l_Modes[i].Width == *a_WidthP)
		{
			// Height matches
			if (l_Modes[i].Height == *a_HeightP)
			{
				BestMode = i;
				break;	// it is here!
			}
			
			// Otherwise, set the best mode as long as height diff is lower
			else if (abs((int32_t)l_Modes[i].Height - (int32_t)*a_HeightP) < abs((int32_t)l_Modes[BestMode].Height - (int32_t)*a_HeightP))
				BestMode = i;
		}
	
	/* Return mode */
	*a_WidthP = l_Modes[BestMode].Width;
	*a_HeightP = l_Modes[BestMode].Height;
	return BestMode;
}

/* VID_GetModeForSize() -- Gets the closest mode for a widthxheight */
int __REMOOD_DEPRECATED VID_GetModeForSize(int a_Width, int a_Height)
{
	int w, h;
	
	/* Set */
	w = a_Width;
	h = a_Height;
	
	/* Return whatever */
	return VID_ClosestMode(&w, &h, true);
}

/* VID_AddMode() -- Add video mode to the list, either being fullscreen or not */
// Ignore fullscreen for now
bool_t VID_AddMode(const int a_Width, const int a_Height, const bool_t a_Fullscreen)
{
	size_t i;
	
	/* Check */
	if (!a_Width || !a_Height)
		return false;
	
	/* Was this mode already set? */
	for (i = 0; i < l_NumModes; i++)
		if (l_Modes[i].Width == a_Width && l_Modes[i].Height == a_Height)
			return true;
	
	/* Resize mode list and set*/
	l_Modes = I_SysRealloc(l_Modes, sizeof(*l_Modes) * (l_NumModes + 1));
	
	// Set
	snprintf(l_Modes[l_NumModes].Name, MODENAMELENGTH, "%ix%i", a_Width, a_Height);
	l_Modes[l_NumModes].Width = a_Width;
	l_Modes[l_NumModes++].Height = a_Height;
	
	/* Success! */
	return true;
}

/* VID_SetMode() -- Sets the specified video mode */
// Funny thing is, despite returning an int, Legacy never checked if it worked!
int VID_SetMode(int a_ModeNum)
{
	/* Check */
	if (a_ModeNum < 0 || a_ModeNum >= l_NumModes)
		return 0;	// Failure despite not being checked!
	
	/* Try to set the mode */
	if (!I_SetVideoMode(l_Modes[a_ModeNum].Width, l_Modes[a_ModeNum].Height, cv_fullscreen.value))
		return false;
	return true;
}

/* I_UtilWinArgToUNIXArg() -- Converts Windows-style command line to a UNIX one */
bool_t I_UtilWinArgToUNIXArg(int* const a_argc, char*** const a_argv, const char* const a_Win)
{
	/* Check */
	if (!a_argc || !a_argv || !a_Win)
		return false;
	
	return true;
}

/* I_VideoPreInit() -- Common nitialization before everything */
bool_t I_VideoPreInit(void)
{
	/* If graphics are already started, do not start again */
	if (graphics_started)
		return false;
	
	/* Reset vid structure fields */
	vid.bpp = 1;
	vid.width = BASEVIDWIDTH;
	vid.height = BASEVIDHEIGHT;
	vid.rowbytes = vid.width * vid.bpp;
	vid.recalc = true;
	return true;
}

/* I_VideoBefore320200Init() -- Initialization before initial 320x200 set */
bool_t I_VideoBefore320200Init(void)
{
	return true;
}

/* I_VideoPostInit() -- Initialization before end of function */
bool_t I_VideoPostInit(void)
{
	/* Set started */
	graphics_started = 1;
	
	/* Add exit function */
	I_AddExitFunc(I_ShutdownGraphics);
	
	return true;
}

/* I_VideoSetBuffer() -- Sets the video buffer */
// This is here so I do not constantly repeat code in I_SetVideoMode()
void I_VideoSetBuffer(const uint32_t a_Width, const uint32_t a_Height, const uint32_t a_Pitch, uint8_t* const a_Direct)
{
	int w, h;
	
	/* Setup */
	w = a_Width;
	h = a_Height;
	
	/* Set direct video buffer */
	vid.rowbytes = a_Pitch;	// Set rowbytes to pitch
	vid.direct = a_Direct;	// Set direct, if it is passed (if not, direct access not supported)
	vid.width = a_Width;
	vid.height = a_Height;
	vid.modenum = VID_ClosestMode(&w, &h, true);
	
	/* Allocate buffer for mode */
	vid.buffer = I_SysAlloc(a_Width * a_Height * NUMSCREENS);
	
	// Oops!
	if (!vid.buffer)
		return;
	
	// Clear buffer
	memset(vid.buffer, 0, a_Width * a_Height);
	
	/* Initialize video stuff (ouch) */
	V_Init();
}

/* I_VideoUnsetBuffer() -- Unsets the video buffer */
void I_VideoUnsetBuffer(void)
{
	/* Clear direct */
	vid.rowbytes = 0;
	vid.direct = NULL;
	vid.width = 0;
	vid.height = 0;
	
	/* Free */
	if (vid.buffer)
		I_SysFree(vid.buffer);
	vid.buffer = NULL;
}

/* I_VideoSoftBuffer() -- Returns the soft buffer */
uint8_t* I_VideoSoftBuffer(uint32_t* const a_WidthP, uint32_t* const a_HeightP)
{
	/* Set sizes */
	if (a_WidthP)
		*a_WidthP = vid.width;
	if (a_HeightP)
		*a_HeightP = vid.height;
	
	/* Return soft buffer */
	return vid.buffer;
}

/* I_GetTime() -- Returns time since the game started */
uint32_t I_GetTime(void)
{
	return (I_GetTimeMS() * TICRATE) / 1000;
}

/* I_DumpTemporary() -- Creates a temporary file with data inside of it */
bool_t I_DumpTemporary(char* const a_PathBuf, const size_t a_PathSize, const uint8_t* const a_Data, const size_t a_Size)
{
#if defined(__unix__)
	int fd;
#elif defined(_WIN32)
	TCHAR Buf[PATH_MAX];
#endif
	
	/* Check */
	if (!a_PathBuf || !a_PathSize || !a_Data || !a_Size)
		return false;
	
	/* Under UNIX, use mkstemp() */
#if defined(__unix__)
	// Create it
	snprintf(a_PathBuf, a_PathSize, 
#if defined(__MSDOS__)
			"rmXXXXXX"	// On DJGPP with DOS, don't place in /tmp/ because that will always fail!
#else
			"/tmp/rmXXXXXX"
#endif
		);
	if ((fd = mkstemp(a_PathBuf)) == -1)
		return false;
	
	// Place data in fd
	write(fd, a_Data, a_Size);
	close(fd);
	return true;

	/* Under Windows, use GetTempPath()/GetTempFileName() */
#elif defined(_WIN32)
	
	/* For everything else, just guess */
#else

	/* */
#endif
	
	/* Failure */
	return false;
}

/* I_ReadScreen() -- Reads the screen into pointer */
// This is enough to make the code work as it should
void I_ReadScreen(uint8_t* scr)
{
	/* Check */
	if (!scr)
		return;
	
	/* Blind copy */
	memcpy(scr, vid.buffer, vid.width * vid.height * vid.bpp);
}

/* I_ShowEndTxt() -- Shows the ending text screen */
void I_ShowEndTxt(const uint8_t* const a_TextData)
{
	size_t i, c, Cols;
	const char* p;
	
	/* Check */
	if (!a_TextData)
		return;
	
	/* Get environment */
	// Columns?
	if ((p = getenv("COLUMNS")))
		Cols = atoi(p);
	else
		Cols = 80;
	
	/* Load ENDTXT */
	for (i = 0; i < 4000; i += 2)
	{
		// Get logical column number
		c = (i >> 1) % 80;
		
		// Print character
		if (c < Cols)	// but only if it fits!
			I_TextModeChar(a_TextData[i], a_TextData[i + 1]);
		
		// Add a newline if Cols > 80 and c == 79
		if (c == 79 && Cols > 80)
			printf("\n");
	}
	
	/* Leave text mode */
	I_TextMode(false);
}

/* I_TextModeChar() -- Prints a text mode character */
void I_TextModeChar(const uint8_t a_Char, const uint8_t Attr)
{
	uint8_t BG, FG;
	bool_t Blink;
	static const char c_ColorToVT[8] =
	{
		0, 4, 2, 6, 1, 5, 3, 7
	};	// Colors to VT
	
	/* Get common attribute colors */
	FG = Attr & 0xF;
	BG = (Attr >> 4) & 0x7;
	Blink = (Attr >> 7) & 1;
	
	/* Create attribute */
	// Use DOS color functions
#if defined(__MSDOS__)
	// ENDOOM uses DOS attributes
	textattr(Attr);

	// Use Win32 console color functions
#elif defined(_WIN32)

	// Use VT escape characters
#elif defined(__unix__)
	if (FG & 8)
		printf("%c[1m", 0x1B);
	printf("%c[%i;%im", 0x1B, 30 + c_ColorToVT[FG & 7], 40 + c_ColorToVT[BG]);

	// Don't bother with formatting
#else

	//
#endif
	
	/* Print character */
	if (a_Char < ' ' || a_Char >= 0x7F)
#if defined(__MSDOS__)
		cprintf(" ");
#else
		printf(" ");
#endif
	else
#if defined(__MSDOS__)
		cprintf("%c", a_Char);
#else
		printf("%c", a_Char);
#endif
	
	/* Reset attributes */
	// Use DOS color functions
#if defined(__MSDOS__)
	textattr(1);

	// Use Win32 console color functions
#elif defined(_WIN32)

	// Use VT escape characters
#elif defined(__unix__)
	printf("%c[0m", 0x1B);

	// Don't bother with formatting
#else

	//
#endif
}

/* I_AddExitFunc() -- Adds an exit function */
void I_AddExitFunc(void (*func) ())
{
	int c;
	
	/* Check */
	if (!func)
		return;
	
	/* Do not add function twice */
	for (c = 0; c < MAX_QUIT_FUNCS; c++)
		if (quit_funcs[c] == func)
			return;
	
	/* Look in array */
	for (c = 0; c < MAX_QUIT_FUNCS; c++)
		if (!quit_funcs[c])
		{
			quit_funcs[c] = func;
			break;
		}
}

/* I_RemoveExitFunc() -- Removes an exit function */
void I_RemoveExitFunc(void (*func) ())
{
	int c;

	for (c = 0; c < MAX_QUIT_FUNCS; c++)
		if (quit_funcs[c] == func)
		{
			while (c < MAX_QUIT_FUNCS - 1)
			{
				quit_funcs[c] = quit_funcs[c + 1];
				c++;
			}
			quit_funcs[MAX_QUIT_FUNCS - 1] = NULL;
			break;
		}
}

/* I_ShutdownSystem() -- Shuts the system down */
void I_ShutdownSystem(void)
{
	int c;
	uint8_t* EDData[2] = {0, 0};
	
	WX_WADEntry_t* Entry;
	uint8_t* Temp;
	size_t Size;
	
	/* Before we start exiting, load ENDOOM, ENREMOOD */
	for (c = 0; c < 2; c++)
	{
		// Get it
		Entry = WX_EntryForName(NULL, (c ? "ENREMOOD" : "ENDOOM"), false);
		
		// Check
		if (!Entry)
			continue;
		
		// Get size and data
		Temp = WX_CacheEntry(Entry, WXCT_RAW, WXCT_RAW);
		Size = WX_GetEntrySize(Entry);
		
		// Duplicate
		EDData[c] = I_SysAlloc(4000);
		
		if (EDData[c])
		{
			memset(EDData[c], 0, 4000);
			memmove(EDData[c], Temp, (Size <= 4000 ? Size : 4000));
		}
		
		// Unuse the entry (not needed any more)
		WX_UseEntry(Entry, WXCT_RAW, false);
	}
	
	/* Pre exit func */
	I_SystemPreExit();
	
	/* Call functions */
	for (c = MAX_QUIT_FUNCS - 1; c >= 0; c--)
		if (quit_funcs[c])
			(*quit_funcs[c]) ();
	
	/* Post exit func */
	I_SystemPostExit();
	
	/* Show the end text */
	for (c = 0; c < 2; c++)
		if (EDData[c])
		{
			I_ShowEndTxt(EDData[c]);
			I_SysFree(EDData[c]);
		}
}

/* I_GetUserName() -- Returns the current username */
const char* I_GetUserName(void)
{
#define MAXUSERNAME 32	// Username limit (youch)
	static char RememberName[MAXUSERNAME];
	const char* p;
#if defined(_WIN32)
	TCHAR Buf[MAXUSERNAME];
	DWORD Size;
	size_t i;
#endif

	/* Try system specific username getting */
	// Under UNIX, use getlogin	
#if defined(__unix__)
	// prefer getlogin_r if it exists
	#if _REENTRANT || _POSIX_C_SOURCE >= 199506L
	if (getlogin_r(RememberName, MAXUSERNAME))
		return RememberName;
	
	// Otherwise use getlogin
	#else
	p = getlogin();
	
	if (p)
	{
		// Dupe string
		strncpy(RememberName, p, MAXUSERNAME);
		return RememberName;
	}
	#endif
	
	// Under Win32, use GetUserName
#elif defined(_WIN32)
	Size = MAXUSERNAME;
	if (GetUserName(Buf, &Size))
	{
		// Cheap copy
		for (i = 0; i <= Size; i++)
			RememberName[i] = Buf[i] & 0x7F;
		return RememberName;
	}
	
	// Otherwise whoops!
#else
#endif

	/* Try environment variables that usually exist */
	// USER, USERNAME, LOGNAME
	p = getenv("USER");
	
	// Nope
	if (!p)
	{
		p = getenv("USERNAME");
		
		// Nope
		if (!p)
		{
			p = getenv("LOGNAME");
			
			// Nope
			if (!p)
				return NULL;
		}
	}
	
	// Copy p to buffer and return the buffer
	strncpy(RememberName, p, MAXUSERNAME);
	return RememberName;

#undef MAXUSERNAME
}

/* I_GetDiskFreeSpace() -- Returns space being used */
uint64_t I_GetDiskFreeSpace(const char* const a_Path)
{
	/* Check */
	if (!a_Path)
		return 0;
	
	// TODO
	return 2 << 30;
}

/* I_AddMusicDriver() -- Adds a new music driver */
bool_t I_AddMusicDriver(I_MusicDriver_t* const a_Driver)
{
	size_t i;
	
	/* Check */
	if (!a_Driver)
		return false;
	
	/* Attempt driver initialization */
	if (a_Driver->Init && !a_Driver->Init(a_Driver))
		return false;
	
	/* Find a blank spot */
	for (i = 0; i < l_NumMusicDrivers; i++)
		if (!l_MusicDrivers[i])
		{
			l_MusicDrivers[i] = a_Driver;
			break;
		}
	
	// did not find one
	if (i == l_NumMusicDrivers)
	{
		// Resize the list
		Z_ResizeArray(&l_MusicDrivers, sizeof(*l_MusicDrivers), l_NumMusicDrivers, l_NumMusicDrivers + 1);
		l_MusicDrivers[l_NumMusicDrivers++] = a_Driver;
	}
	
	/* Call the success routine, if it exists */
	if (a_Driver->Success)
		a_Driver->Success(a_Driver);
	
	/* Success */	
	return true;
}

/* I_RemoveMusicDriver() -- Removes a music driver */
bool_t I_RemoveMusicDriver(I_MusicDriver_t* const a_Driver)
{
	size_t i;
	
	/* Check */
	if (!a_Driver)
		return false;
	
	/* Find driver */
	
	return true;
}

/* I_FindMusicDriver() -- Find a music driver that can play this format */
I_MusicDriver_t* I_FindMusicDriver(const I_MusicType_t a_Type)
{
	I_MusicDriver_t* Best = NULL;
	size_t i;
	
	/* Go through every driver */
	for (i = 0; i < l_NumMusicDrivers; i++)
		if (l_MusicDrivers[i])
			if (l_MusicDrivers[i]->MusicType & (1 << a_Type))
				if (!Best || (Best && l_MusicDrivers[i]->Priority > Best->Priority))
					Best = l_MusicDrivers[i];
	
	/* Return the best driver, if any */
	return Best;
}

/* I_InitMusic() -- Initializes the music system */
bool_t I_InitMusic(void)
{
	/* Add interface specific stuff */
	if (!I_MusicDriverInit())
		CONS_Printf("I_InitMusic: Failed to add interface specific drivers.\n");
	
	/* Add our own virtual drivers that always work */
	// OPL Emulation!
	
	// Native MOD Support
	
	// ReMooD MUS2MID Driver
	if (!I_AddMusicDriver(&l_MUS2MIDDriver))
		CONS_Printf("I_InitMusic: Failed to add the MUS2MID driver, you will not hear MUS music.\n");
	
	/* Return only if music drivers were loaded */
	return !!l_NumMusicDrivers;
}

/* I_ShutdownMusic() -- Shuts down the music system */
void I_ShutdownMusic(void)
{
	size_t i;
	
	/* Destroy all drivers */
	for (i = 0; i < l_NumMusicDrivers; i++)
		if (!I_RemoveMusicDriver(l_MusicDrivers[i]))
			CONS_Printf("I_ShutdownMusic: Failed to remove driver.\n");
	
	/* Destroy array */
	Z_Free(l_MusicDrivers);
	l_MusicDrivers = NULL;
	l_NumMusicDrivers = 0;
}

/* I_UpdateMusic() -- Updates playing music */
void I_UpdateMusic(void)
{
	size_t i;
	
	/* Update all drivers */
	for (i = 0; i < l_NumMusicDrivers; i++)
		if (l_MusicDrivers[i]->Update)
			l_MusicDrivers[i]->Update(l_MusicDrivers[i], g_ProgramTic);
}

/* I_SetMusicVolume() -- Sets music volume */
void I_SetMusicVolume(int volume)
{
}

/* I_DetectMusicType() -- Detects the type of music */
I_MusicType_t I_DetectMusicType(const uint8_t* const a_Data, const size_t a_Size)
{
	/* Check */
	if (!a_Data || a_Size < 4)
		return IMT_UNKNOWN;
		
	/* Print magic number */
	if (devparm)
		CONS_Printf("I_DetectMusicType: Magic \"%c%c%c%c\".\n", a_Data[0], a_Data[1], a_Data[2], a_Data[3]);
	
	/* Check for MIDI format */
	if (a_Data[0] == 'M' && a_Data[1] == 'T' && a_Data[2] == 'h' && a_Data[3] == 'd')
		return IMT_MIDI;
	
	/* Check for MUS format */
	if (a_Data[0] == 'M' && a_Data[1] == 'U' && a_Data[2] == 'S' && a_Data[3] == 0x1A)
		return IMT_MUS;
	
	/* Fell through, not known */
	return IMT_UNKNOWN;
}

/* I_RegisterSong() -- Loads a song for future playing */
int I_RegisterSong(const char* const a_Lump)
{
#define BUFSIZE 512
	WX_WADEntry_t* Entry;
	I_LocalMusic_t New;
	FILE* f;
	int Fails;
	size_t i;
	static int TempSongID = 1;
	char SongPath[BUFSIZE];
	
	/* Check */
	if (!a_Lump)
		return 0;
		
	/* Clear */
	memset(&New, 0, sizeof(New));
	
	/* Find lump */
	Entry = WX_EntryForName(NULL, a_Lump, false);
	
	// Not found?
	if (!Entry)
		return 0;
	
	/* Find if it already is registered */
	// If it is, then return the ID!
	for (i = 0; i < l_NumLocalSongs; i++)
		if (l_LocalSongs[i].Entry == Entry)
			return l_LocalSongs[i].Handle;
	
	/* Get data and detect */
	New.Handle = ++TempSongID;
	New.Entry = Entry;
	New.EntryLength = WX_GetEntrySize(Entry);
	New.Data = WX_CacheEntry(Entry, WXCT_RAW, WXCT_RAW);
	New.Type = I_DetectMusicType(New.Data, New.EntryLength);
	New.Playing = false;
	
	// Get the driver it belongs to
	New.Driver = I_FindMusicDriver(New.Type);
	
	/* Could not find driver? */
	if (!New.Driver)
	{
		// Debug
		CONS_Printf("I_RegisterSong: Song format is not supported (Type = %i)!\n", New.Type);
		
		// Unuse the data (so it gets freed)
		WX_UseEntry(New.Entry, WXCT_RAW, false);
		return 0;
	}
	
	/* Does the driver require an external file? */
	if (New.Driver->ExternalData)
	{
		// Debug
		if (devparm)
			CONS_Printf("I_RegisterSong: Driver \"%s\" requires external files.\n", New.Driver->Name);
		
		// Create temporary file with data
		if (!I_DumpTemporary(SongPath, BUFSIZE, New.Data, New.EntryLength))
		{
			CONS_Printf("I_RegisterSong: Failed to file to disk!\n");
			WX_UseEntry(New.Entry, WXCT_RAW, false);
			return 0;
		}
		
		// Copy pathname
		New.PathName = Z_StrDup(SongPath, PU_STATIC, NULL);
	}
	
	/* Add to the song list */
	for (i = 0; i < l_NumLocalSongs; i++)
		if (!l_LocalSongs[i].Handle)
		{
			l_LocalSongs[i] = New;
			break;
		}
	
	// did not find one
	if (i == l_NumLocalSongs)
	{
		// Resize the list
		Z_ResizeArray(&l_LocalSongs, sizeof(*l_LocalSongs), l_NumLocalSongs, l_NumLocalSongs + 1);
		l_LocalSongs[l_NumLocalSongs++] = New;
	}
	
	/* Return handle of new freshly loaded song */
	return New.Handle;
#undef BUFSIZE
}

/* I_UnRegisterSong() -- Unloads a song */
void I_UnRegisterSong(int handle)
{
}

/* I_PauseSong() -- Pauses a song */
void I_PauseSong(int handle)
{
	size_t i, j;
	
	/* Find song in song list */
	for (i = 0; i < l_NumLocalSongs; i++)
		if (l_LocalSongs[i].Handle == handle)
			break;
	
	// Not found?
	if (i == l_NumLocalSongs)
		return;
		
	/* Song not playing? */
	if (!l_LocalSongs[i].Playing)
		return;
	
	/* Send pause */
	if (l_LocalSongs[i].Driver->Pause)
		l_LocalSongs[i].Driver->Pause(l_LocalSongs[i].Driver, l_LocalSongs[i].DriverHandle);
}

/* I_ResumeSong() -- Resumes a song */
void I_ResumeSong(int handle)
{
	size_t i, j;
	
	/* Find song in song list */
	for (i = 0; i < l_NumLocalSongs; i++)
		if (l_LocalSongs[i].Handle == handle)
			break;
	
	// Not found?
	if (i == l_NumLocalSongs)
		return;
		
	/* Song not playing? */
	if (!l_LocalSongs[i].Playing)
		return;
	
	/* Send resume */
	if (l_LocalSongs[i].Driver->Resume)
		l_LocalSongs[i].Driver->Resume(l_LocalSongs[i].Driver, l_LocalSongs[i].DriverHandle);
}

/* I_PlaySong() -- Plays a song with optional looping */
void I_PlaySong(int handle, int looping)
{
	size_t i, j;
	
	/* Find song in song list */
	for (i = 0; i < l_NumLocalSongs; i++)
		if (l_LocalSongs[i].Handle == handle)
			break;
	
	// Not found?
	if (i == l_NumLocalSongs)
		return;
		
	/* Song already playing? */
	if (l_LocalSongs[i].Playing)
		return;
	
	/* If another song is playing stop it from playing */
	for (j = 0; j < l_NumLocalSongs; j++)
		if (l_LocalSongs[j].Playing)
			I_StopSong(l_LocalSongs[j].Handle);
	
	/* Send to driver */
	if (l_LocalSongs[i].Driver->Play)
		l_LocalSongs[i].DriverHandle = l_LocalSongs[i].Driver->Play(
												l_LocalSongs[i].Driver,
												(l_LocalSongs[i].Driver->ExternalData ? l_LocalSongs[i].PathName : l_LocalSongs[i].Data),
												l_LocalSongs[i].EntryLength,
												looping
											);
	
	/* Set song to playing */
	l_LocalSongs[i].Playing = true;
}

/* I_StopSong() -- Stops a song */
void I_StopSong(int handle)
{
	size_t i;
	
	/* Find song in song list */
	for (i = 0; i < l_NumLocalSongs; i++)
		if (l_LocalSongs[i].Handle == handle)
			break;
	
	// Not found?
	if (i == l_NumLocalSongs)
		return;
	
	/* Only stop if it is actually playing */
	if (!l_LocalSongs[i].Playing)
		return;
	
	/* Send to driver */
	if (l_LocalSongs[i].Driver->Stop)
		l_LocalSongs[i].Driver->Stop(l_LocalSongs[i].Driver, l_LocalSongs[i].DriverHandle);
	
	/* No longer playing */
	l_LocalSongs[i].Playing = false;
}

