// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
//		 :oCCCCOCoc.
//	 .cCO8OOOOOOOOO8Oo:
//   .oOO8OOOOOOOOOOOOOOOCc
//  cO8888:		 .:oOOOOC.												TM
// :888888:   :CCCc   .oOOOOC.	 ###	  ###					#########
// C888888:   .ooo:   .C########   #####  #####  ######	######  ##########
// O888888:		 .oO###	###  #####  ##### ########  ######## ####	###
// C888888:   :8O.   .C##########  ### #### ### ##	##  ##	## ####	###
// :8@@@@8:   :888c   o###		 ### #### ### ########  ######## ##########
//  :8@@@@C   C@@@@   oo########   ###  ##  ###  ######	######  #########
//	cO@@@@@@@@@@@@@@@@@Oc0
//	  :oO8@@@@@@@@@@Oo.
//		 .oCOOOOOCc.									  http://remood.org/
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
// DESCRIPTION: Common Interface Utilities (to reduce code bloat and dup)

/***************
*** INCLUDES ***
***************/

/* System */
// On UNIX include the standard header
#if defined(__unix__)
#include <unistd.h>				// Standard Stuff
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
#include "i_system.h"
#include "command.h"
#include "w_wad.h"
#include "doomstat.h"
#include "i_sound.h"
#include "s_sound.h"
#include "dbopl.h"

/****************
*** CONSTANTS ***
****************/

#define EVENTQUEUESIZE		64	// Max events allowed in queue
#define MODENAMELENGTH		16	// Length of mode name
#define MAX_QUIT_FUNCS		16	// Max number of quit functions

/**************
*** GLOBALS ***
**************/

/* i_cdmus.c -- Remove this garbage */
consvar_t cd_volume = { "cd_volume", "31", CV_SAVE };
consvar_t cdUpdate = { "cd_update", "1", CV_SAVE };

/*****************
*** STRUCTURES ***
*****************/

/* I_LocalMusic_t -- Local music data */
typedef struct I_LocalMusic_s
{
	I_MusicType_t Type;			// Type of song this is
	int Handle;					// Song handle
	I_MusicDriver_t* Driver;	// Driver to play with
	int DriverHandle;			// Handle known by driver
	uint32_t Length;			// Length of song in tics
	WX_WADEntry_t* Entry;		// Entry of song
	size_t EntryLength;			// Length of entry
	uint8_t* Data;				// Loaded Data
	char* PathName;				// Path to file on disk (if it exists)
	bool_t Playing;				// Is the song playing?
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
	fixed_t VolScale;
	
	size_t MusStart;
	size_t MusLen;
} I_MUS2MIDData_t;

/*************
*** LOCALS ***
*************/

static I_MusicDriver_t** l_MusicDrivers;	// Music drivers
static size_t l_NumMusicDrivers;	// Number of music drivers

static I_SoundDriver_t** l_SoundDrivers;	// Sound drivers
static size_t l_NumSoundDrivers;	// Number of sound drivers

static I_LocalMusic_t* l_LocalSongs;	// Local songs
static size_t l_NumLocalSongs;	// Number of local songs

static I_SoundDriver_t* l_CurSoundDriver;	// Current sound driver

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
	uint8_t Key, VolUse;
	
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
	if (Event < 5)				// Every event has all the stuff
		NoteBit[1] = a_Local->Data[a_Local->Pos++];
		
	if (Event == 4)				// note on and controller are 3-bytes
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
				a_Local->Vols[Channel] = VolUse = NoteBit[2] & 0x7F;
			}
			else
				VolUse = a_Local->Vols[Channel];	// use last volume
				
			// Scale volume
			VolUse = FixedMul((fixed_t) VolUse << FRACBITS, a_Local->VolScale) >> FRACBITS;
			
			// Use volume determined
			MIDIMsg.b[2] = VolUse;
			
			*a_OutSize = 3;
			break;
			
			// Pitch wheel
		case 2:
			MIDIMsg.b[0] = 0xE0 | Channel;
			MIDIMsg.b[1] = (NoteBit[1] & 0x01) << 6;
			MIDIMsg.b[2] = (NoteBit[1] & 0xFE) >> 1;
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
			Last = false;		// Clear last, don't want to handle it
			*a_Delta = 1;		// Break from loop, kinda
			a_Local->Pos = a_Local->MusStart;	// Back to start
			break;
			
			// Unknown
		default:
			if (devparm)
				CONL_PrintF("I_MUS2MID_MUSReadNextMessage: Unknown %i\n", Event);
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
	*a_Delta = *a_Delta * 7;	//(*a_Delta * 1000) / 140;
	
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
		CONL_PrintF("I_MUS2MID_Init: Feeding messages into %s.\n", Local->RealDriver->Name);
		
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
		
	/* Set Initial volume */
	Local->VolScale = 1 << FRACBITS;
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
			
			if (Local->RealDriver->RawMIDI)
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
		return 0;
		
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
		return 0;
		
	if (devparm)
		CONL_PrintF("I_MUS2MID_Play: Converting MUS to MIDI.\n");
		
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
		// Always tell driver to be at full volume
		if (Local->RealDriver->Volume)
			Local->RealDriver->Volume(Local->RealDriver, Local->RealHandle, 255);
			
		// Set volume scale
		Local->VolScale = FixedDiv(((fixed_t) Vol) << FRACBITS, 255 << FRACBITS);
	}
	
	/* Full convert mode */
	else
	{
		// Tell driver the volume I want
		if (Local->RealDriver->Volume)
			Local->RealDriver->Volume(Local->RealDriver, Local->RealHandle, Vol);
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
			else				// Something went bad =(
				Local->LocalTime = MSTime;
		}
	}
	
	/* Full convert mode */
	else
	{
	}
}

// l_MUS2MIDDrive -- MUS2MID Driver
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

/***************************
*** OPL EMULATION, KINDA ***
***************************/
// Simulated OPL Emulation, of sorts
// Portions Copyright(C) 2009 Simon Howard

/*** CONSTANTS ***/

typedef enum
{
	OPL_REGISTER_PORT = 0,
	OPL_DATA_PORT = 1
} opl_port_t;

#define OPL_NUM_OPERATORS   21
#define OPL_NUM_VOICES	  9

#define OPL_REG_WAVEFORM_ENABLE   0x01
#define OPL_REG_TIMER1			0x02
#define OPL_REG_TIMER2			0x03
#define OPL_REG_TIMER_CTRL		0x04
#define OPL_REG_FM_MODE		   0x08

// Operator registers (21 of each):

#define OPL_REGS_TREMOLO		  0x20
#define OPL_REGS_LEVEL			0x40
#define OPL_REGS_ATTACK		   0x60
#define OPL_REGS_SUSTAIN		  0x80
#define OPL_REGS_WAVEFORM		 0xE0

// Voice registers (9 of each):

#define OPL_REGS_FREQ_1		   0xA0
#define OPL_REGS_FREQ_2		   0xB0
#define OPL_REGS_FEEDBACK		 0xC0

/*** STRUCTURES ***/

/* opl_timer_t -- Timer for OPL */
typedef struct
{
	unsigned int rate;		// Number of times the timer is advanced per sec.
	unsigned int enabled;	 // Non-zero if timer is enabled.
	unsigned int value;	   // Last value that was set.
	unsigned int expire_time; // Calculated time that timer will expire.
} opl_timer_t;

/* I_OPLEmData_t -- Emulated OPL Data */
typedef struct I_OPLEmData_s
{
	struct _Chip OPLChip;						// OPL Chip
	uint32_t Freq;								// Frequency
	
	int SamplesPerMS;							// Samples per millisecond
	
	bool_t init_stage_reg_writes;				// Intialization stage?
	opl_timer_t timer1;							// First Timer
	opl_timer_t timer2;							// Second Timer
	int current_time;							// Current OPL Sample Time
	int register_num;							// Register Number
} I_OPLEmData_t;

/*** STUFF FROM CHOCOLATE DOOM ***/

/* ICD_OPL_Delay() -- Cause delay in OPL system */
static void ICD_OPL_Delay(I_OPLEmData_t* const a_OPLData, int WaitTime)
{
	I_WaitVBL(WaitTime);
	
	/* Advance Time */
	a_OPLData->current_time += a_OPLData->SamplesPerMS * WaitTime;
}

/* ICD_OPLTimer_CalculateEndTime() -- Calculates end time? */
static void ICD_OPLTimer_CalculateEndTime(I_OPLEmData_t* const a_OPLData, opl_timer_t *timer)
{
	int tics;

	// If the timer is enabled, calculate the time when the timer
	// will expire.

	if (timer->enabled)
	{
		tics = 0x100 - timer->value;
		timer->expire_time = a_OPLData->current_time
						   + (tics * a_OPLData->Freq) / timer->rate;
	}
}

/* ICD_OPL_DirectWriteRegister() -- Writes to OPL register */
static void ICD_OPL_DirectWriteRegister(I_OPLEmData_t* const a_OPLData, unsigned int reg_num, unsigned int value)
{
	switch (reg_num)
	{
		case OPL_REG_TIMER1:
			a_OPLData->timer1.value = value;
			ICD_OPLTimer_CalculateEndTime(a_OPLData, &a_OPLData->timer1);
			break;

		case OPL_REG_TIMER2:
			a_OPLData->timer2.value = value;
			ICD_OPLTimer_CalculateEndTime(a_OPLData, &a_OPLData->timer2);
			break;

		case OPL_REG_TIMER_CTRL:
			if (value & 0x80)
			{
				a_OPLData->timer1.enabled = 0;
				a_OPLData->timer2.enabled = 0;
			}
			else
			{
				if ((value & 0x40) == 0)
				{
					a_OPLData->timer1.enabled = (value & 0x01) != 0;
					ICD_OPLTimer_CalculateEndTime(a_OPLData, &a_OPLData->timer1);
				}

				if ((value & 0x20) == 0)
				{
					a_OPLData->timer1.enabled = (value & 0x02) != 0;
					ICD_OPLTimer_CalculateEndTime(a_OPLData, &a_OPLData->timer2);
				}
			}

			break;

		default:
			Chip__WriteReg(&a_OPLData->OPLChip, reg_num, value);
			break;
	}
}

/* ICD_OPL_WritePort() -- Writes to OPL Port */
static void ICD_OPL_WritePort(I_OPLEmData_t* const a_OPLData, opl_port_t port, unsigned int value)
{
	/* Change Register */
	if (port == OPL_REGISTER_PORT)
		a_OPLData->register_num = value;

	/* Write Data */
	else if (port == OPL_DATA_PORT)
		ICD_OPL_DirectWriteRegister(a_OPLData, a_OPLData->register_num, value);
}

/* ICD_OPL_ReadPort() -- Reads from OPL Port */
static unsigned int ICD_OPL_ReadPort(I_OPLEmData_t* const a_OPLData, opl_port_t port)
{
	unsigned int result = 0;

	if (a_OPLData->timer1.enabled && a_OPLData->current_time > a_OPLData->timer1.expire_time)
	{
		result |= 0x80;   // Either have expired
		result |= 0x40;   // Timer 1 has expired
	}

	if (a_OPLData->timer2.enabled && a_OPLData->current_time > a_OPLData->timer2.expire_time)
	{
		result |= 0x80;   // Either have expired
		result |= 0x20;   // Timer 2 has expired
	}

	return result;	
}

/* ICD_OPL_ReadStatus() -- Read OPL status */
static unsigned int ICD_OPL_ReadStatus(I_OPLEmData_t* const a_OPLData)
{
	return ICD_OPL_ReadPort(a_OPLData, OPL_REGISTER_PORT);
}

/* ICD_OPL_WriteRegister() -- Write OPL register */
static void ICD_OPL_WriteRegister(I_OPLEmData_t* const a_OPLData, int reg, int value)
{
	int i;

	ICD_OPL_WritePort(a_OPLData, OPL_REGISTER_PORT, reg);

	// For timing, read the register port six times after writing the
	// register number to cause the appropriate delay

	for (i=0; i<6; ++i)
	{
		// An oddity of the Doom OPL code: at startup initialization,
		// the spacing here is performed by reading from the register
		// port; after initialization, the data port is read, instead.

		if (a_OPLData->init_stage_reg_writes)
		{
			ICD_OPL_ReadPort(a_OPLData, OPL_REGISTER_PORT);
		}
		else
		{
			ICD_OPL_ReadPort(a_OPLData, OPL_DATA_PORT);
		}
	}

	ICD_OPL_WritePort(a_OPLData, OPL_DATA_PORT, value);

	// Read the register port 24 times after writing the value to
	// cause the appropriate delay

	for (i=0; i<24; ++i)
	{
		ICD_OPL_ReadStatus(a_OPLData);
	}
}

/* CD_OPL_Detect() -- Detects if an OPL chip is around */
// From Chocolate Doom
static int ICD_OPL_Detect(I_OPLEmData_t* const a_OPLData)
{
	int result1, result2;
	int i;

	// Reset both timers:
	ICD_OPL_WriteRegister(a_OPLData, OPL_REG_TIMER_CTRL, 0x60);

	// Enable interrupts:
	ICD_OPL_WriteRegister(a_OPLData, OPL_REG_TIMER_CTRL, 0x80);

	// Read status
	result1 = ICD_OPL_ReadStatus(a_OPLData);

	// Set timer:
	ICD_OPL_WriteRegister(a_OPLData, OPL_REG_TIMER1, 0xff);

	// Start timer 1:
	ICD_OPL_WriteRegister(a_OPLData, OPL_REG_TIMER_CTRL, 0x21);

	// Wait for 80 microseconds
	// This is how Doom does it:

	for (i=0; i<200; ++i)
	{
		ICD_OPL_ReadStatus(a_OPLData);
	}

	ICD_OPL_Delay(a_OPLData, 1);

	// Read status
	result2 = ICD_OPL_ReadStatus(a_OPLData);

	// Reset both timers:
	ICD_OPL_WriteRegister(a_OPLData, OPL_REG_TIMER_CTRL, 0x60);

	// Enable interrupts:
	ICD_OPL_WriteRegister(a_OPLData, OPL_REG_TIMER_CTRL, 0x80);

	return (result1 & 0xe0) == 0x00
		&& (result2 & 0xe0) == 0xc0;
}

/*** REMOOD EMULATION STUFF ***/

/* IS_OPLEm_Init() -- OPL Emulation Init */
static bool_t IS_OPLEm_Init(struct I_MusicDriver_s* const a_Driver)
{
	int r;
	I_OPLEmData_t* Local;
	
	/* Check */
	if (!a_Driver)
		return false;
	
	/* Debug */
	if (devparm)
		CONL_PrintF("OPL: Initializing...\n");
	
	/* Create Data */
	Local = a_Driver->Data = Z_Malloc(sizeof(*Local), PU_STATIC, NULL);
	
	// Setup some fields
	Local->Freq = I_SoundGetFreq();
	Local->init_stage_reg_writes = true;
	Local->timer1.rate = 12500;
	Local->timer2.rate = 3125;
	Local->SamplesPerMS = Local->Freq / 1000;
	
	if (devparm)
		CONL_PrintF("OPL: Frequency is %u\n", Local->Freq);
	
	/* Pre-Init OPL Stuff */
	Chip__Chip(&Local->OPLChip);
	Chip__Setup(&Local->OPLChip, Local->Freq);
	
	/* Detecting OPL */
	// Debug
	if (devparm)
		CONL_PrintF("OPL: Detecting...\n");
	
	// Detect?
	if (!ICD_OPL_Detect(Local) || !ICD_OPL_Detect(Local))
	{
		if (devparm)
			CONL_PrintF("OPL: Not Detected!\n");
		return false;
	}
	
	// Found it
	if (devparm)
		CONL_PrintF("OPL: Detected!\n");
	
	/* Initialize the OPL Stuff */
	// Debug
	if (devparm)
		CONL_PrintF("OPL: Initializing\n");

	// Initialize level registers
	for (r=OPL_REGS_LEVEL; r <= OPL_REGS_LEVEL + OPL_NUM_OPERATORS; ++r)
		ICD_OPL_WriteRegister(Local, r, 0x3f);

	// Initialize other registers
	// These two loops write to registers that actually don't exist,
	// but this is what Doom does ...
	// Similarly, the <= is also intenational.
	for (r=OPL_REGS_ATTACK; r <= OPL_REGS_WAVEFORM + OPL_NUM_OPERATORS; ++r)
		ICD_OPL_WriteRegister(Local, r, 0x00);

	// More registers ...
	for (r=1; r < OPL_REGS_LEVEL; ++r)
		ICD_OPL_WriteRegister(Local, r, 0x00);

	// Re-initialize the low registers:
	// Reset both timers and enable interrupts:
	ICD_OPL_WriteRegister(Local, OPL_REG_TIMER_CTRL, 0x60);
	ICD_OPL_WriteRegister(Local, OPL_REG_TIMER_CTRL, 0x80);

	// "Allow FM chips to control the waveform of each operator":
	ICD_OPL_WriteRegister(Local, OPL_REG_WAVEFORM_ENABLE, 0x20);

	// Keyboard split point on (?)
	ICD_OPL_WriteRegister(Local, OPL_REG_FM_MODE, 0x40);
	
	/* Register access is no longer initialized */
	Local->init_stage_reg_writes = false;
	
	/* Return */
	return true;
}

/* IS_WriteMixSample() -- Writes a single sample */
static void IS_WriteMixSample(void** Buf, int32_t Value, size_t a_Bits)
{
	int32_t v, s;
	
	/* Get shift */
	s = Value;
	
	/* Write Wide */
	if (a_Bits == 16)
	{
		// Read current value (I hope the buffer is aligned!)
		v = **((uint16_t**)Buf);
		v = (v - 32768) + (s / 32768);
		
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
		v = (v - 128) + (s / 16777216);
		
		if (v > 127)
			v = 127;
		else if (v < -128)
			v = -128;
		v += 128;
		
		// Mix into buffer
		WriteUInt8(Buf, v);
	}
}

/* IS_OPLEm_RawMIDI() -- Raw OPL MIDI command */
static void IS_OPLEm_RawMIDI(struct I_MusicDriver_s* const a_Driver, const uint32_t a_Msg, const uint32_t a_BitLength)
{
	I_OPLEmData_t* Local;
	
	/* Check */
	if (!a_Driver)
		return;
	
	/* Get Data */
	Local = a_Driver->Data;
	
	// Check
	if (!Local)
		return;
	
	/* Handle MIDI Events */
    ICD_OPL_WriteRegister(Local, OPL_REGS_LEVEL + 0, 0);
    //OPL_WriteRegister(OPL_REGS_TREMOLO + 0, data->tremolo);
    ICD_OPL_WriteRegister(Local, OPL_REGS_ATTACK + 0, (1 << 8) | 1);
    //OPL_WriteRegister(OPL_REGS_SUSTAIN + 0, data->sustain);
    ICD_OPL_WriteRegister(Local, OPL_REGS_WAVEFORM + 0, 1);
}

/* IS_OPLEm_SoundLayer() -- Sound layered on top */
void IS_OPLEm_SoundLayer(struct I_MusicDriver_s* const a_Driver, struct I_SoundDriver_s* const a_SoundDriver, void* const a_SoundBuf, const size_t a_SoundLen, const int a_Freq, const int a_Bits, const int a_Channels)
{
	I_OPLEmData_t* Local;
	size_t SplitLen, i, j;
	int32_t* TTBuf;
	void* p, *pEnd;
	
	/* Check */
	if (!a_Driver)
		return;
	
	/* Get Data */
	Local = a_Driver->Data;
	
	// Check
	if (!Local)
		return;
	
#if 0
	/* Create Temporary Buffer */
	SplitLen = ((a_SoundLen / a_Channels) / (a_Bits / 8));
	TTBuf = malloc(sizeof(*TTBuf) * (SplitLen));
	
	if (!TTBuf)
		return;
	
	memset(TTBuf, 0, sizeof(*TTBuf) * (SplitLen));
	
	/* Generate OPL Data */
	Chip__GenerateBlock2(&Local->OPLChip, SplitLen, TTBuf);
	
	/* Downmix */
	// Prepare
	p = a_SoundBuf;
	pEnd = ((uint8_t*)a_SoundBuf) + a_SoundLen;
	
	// Write in
	for (i = 0; i < SplitLen; i++)
	{
		// At end?
		if (p >= pEnd)
			break;
		
		// Left Channel
		IS_WriteMixSample(&p, TTBuf[i * 2], a_Bits);
		
		// Right Channel
		if (a_Channels >= 2)
			IS_WriteMixSample(&p, TTBuf[(i * 2) + 1], a_Bits);
		
		// Ignore other channels
		for (j = 2; j < a_Channels; j++)
			IS_WriteMixSample(&p, 0, a_Bits);
	}
	
	/* Free Buffer */
	free(TTBuf);
	
	/* Advance OPL Time */
	Local->current_time += a_SoundLen / (a_Bits >> 3);
#endif
}

// l_OPLEMDriver -- OPL Emulation Driver
static I_MusicDriver_t l_OPLEMDriver =
{
	/* Data */
	"OPL Emulation",
	"oplem",
	1 << IMT_MIDI,
	false,
	25,
	
	/* Handlers */
	IS_OPLEm_Init,
	NULL,//I_MUS2MID_Destroy,
	NULL,//I_MUS2MID_Success,
	NULL,//I_MUS2MID_Pause,
	NULL,//I_MUS2MID_Resume,
	NULL,//I_MUS2MID_Stop,
	NULL,//I_MUS2MID_Length,
	NULL,//I_MUS2MID_Seek,
	NULL,//I_MUS2MID_Play,
	NULL,//I_MUS2MID_Volume,
	IS_OPLEm_RawMIDI,//NULL,
	NULL,//I_MUS2MID_Update
	IS_OPLEm_SoundLayer,
};

/**********************
*** MUSIC FUNCTIONS ***
**********************/

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
	for (i = 0; i < l_NumMusicDrivers; i++)
		if (l_MusicDrivers[i] == a_Driver)
		{
			// Call destroy function
			if (l_MusicDrivers[i]->Destroy)
				l_MusicDrivers[i]->Destroy(a_Driver);
			l_MusicDrivers[i] = NULL;
			
			return true;
		}
		
	/* Driver not loaded */
	return false;
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
		CONL_PrintF("I_InitMusic: Failed to add interface specific drivers.\n");
		
	/* Add our own virtual drivers that always work */
	// Simple Tone Driver
	
	// OPL Emulation!
	if (!I_AddMusicDriver(&l_OPLEMDriver))
		CONL_PrintF("I_InitMusic: Failed to add the OPL Emulation Driver.\n");
	
	// Native MOD Support
	
	// ReMooD MUS2MID Driver
	if (!I_AddMusicDriver(&l_MUS2MIDDriver))
		CONL_PrintF("I_InitMusic: Failed to add the MUS2MID driver, you will not hear MUS music.\n");
		
	/* Return only if music drivers were loaded */
	return ! !l_NumMusicDrivers;
}

/* I_ShutdownMusic() -- Shuts down the music system */
void I_ShutdownMusic(void)
{
	size_t i;
	
	/* Destroy all drivers */
	for (i = 0; i < l_NumMusicDrivers; i++)
		if (!I_RemoveMusicDriver(l_MusicDrivers[i]))
			CONL_PrintF("I_ShutdownMusic: Failed to remove driver.\n");
			
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
	size_t i, j;
	
	/* Find drivers playing music */
	for (i = 0; i < l_NumLocalSongs; i++)
		if (l_LocalSongs[i].Playing && l_LocalSongs[i].Driver)
			if (l_LocalSongs[i].Driver->Volume)
				l_LocalSongs[i].Driver->Volume(l_LocalSongs[i].Driver, l_LocalSongs[i].DriverHandle, volume);
}

/* I_DetectMusicType() -- Detects the type of music */
I_MusicType_t I_DetectMusicType(const uint8_t* const a_Data, const size_t a_Size)
{
	/* Check */
	if (!a_Data || a_Size < 4)
		return IMT_UNKNOWN;
		
	/* Print magic number */
	if (devparm)
		CONL_PrintF("I_DetectMusicType: Magic \"%c%c%c%c\".\n", a_Data[0], a_Data[1], a_Data[2], a_Data[3]);
		
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
	New.Data = WX_CacheEntry(Entry);
	New.Type = I_DetectMusicType(New.Data, New.EntryLength);
	New.Playing = false;
	
	// Get the driver it belongs to
	New.Driver = I_FindMusicDriver(New.Type);
	
	/* Could not find driver? */
	if (!New.Driver)
	{
		// Debug
		CONL_PrintF("I_RegisterSong: Song format is not supported (Type = %i)!\n", New.Type);
		
		// Unuse the data (so it gets freed)
		WX_UseEntry(New.Entry, false);
		return 0;
	}
	
	/* Does the driver require an external file? */
	if (New.Driver->ExternalData)
	{
		// Debug
		if (devparm)
			CONL_PrintF("I_RegisterSong: Driver \"%s\" requires external files.\n", New.Driver->Name);
			
		// Create temporary file with data
		if (!I_DumpTemporary(SongPath, BUFSIZE, New.Data, New.EntryLength))
		{
			CONL_PrintF("I_RegisterSong: Failed to file to disk!\n");
			WX_UseEntry(New.Entry, false);
			return 0;
		}
		
		// Copy pathname
		New.PathName = Z_StrDup(SongPath, PU_STATIC, NULL);
		WX_UseEntry(Entry, false);	// Always unuse the enrty once in a file
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
	size_t i, j;
	
	/* Check */
	if (!handle)
		return;
		
	/* Find song in song list */
	for (i = 0; i < l_NumLocalSongs; i++)
		if (l_LocalSongs[i].Handle == handle)
			break;
			
	// Not found?
	if (i == l_NumLocalSongs)
		return;
		
	/* Make sure the song is stopped */
	if (l_LocalSongs[i].Driver->Stop)
		l_LocalSongs[i].Driver->Stop(l_LocalSongs[i].Driver, l_LocalSongs[i].DriverHandle);
		
	/* Clear away some stuff */
	// If the driver does not have external data, it relies on an entry, unuse it (not needed)
	if (!l_LocalSongs[i].Driver->ExternalData)
		WX_UseEntry(l_LocalSongs[i].Entry, false);
		
	// File name
	if (l_LocalSongs[i].PathName)
		Z_Free(l_LocalSongs[i].PathName);
	l_LocalSongs[i].PathName = NULL;
	
	// remaining, null
	l_LocalSongs[i].Type = 0;
	l_LocalSongs[i].Handle = 0;
	l_LocalSongs[i].Driver = NULL;
	l_LocalSongs[i].DriverHandle = NULL;
	l_LocalSongs[i].Length = 0;
	l_LocalSongs[i].Entry = NULL;
	l_LocalSongs[i].EntryLength = NULL;
	l_LocalSongs[i].Playing = false;
	l_LocalSongs[i].Data = NULL;
}

/* I_PauseSong() -- Pauses a song */
void I_PauseSong(int handle)
{
	size_t i, j;
	
	/* Check */
	if (!handle)
		return;
		
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
	
	/* Check */
	if (!handle)
		return;
		
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
	
	/* Check */
	if (!handle)
		return;
		
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
		l_LocalSongs[i].DriverHandle = l_LocalSongs[i].Driver->Play(l_LocalSongs[i].Driver,
																	(l_LocalSongs[i].Driver->ExternalData ? (void*)l_LocalSongs[i].PathName : (void*)l_LocalSongs[i].Data),
																	l_LocalSongs[i].EntryLength, looping);
																	
	/* Set song to playing */
	l_LocalSongs[i].Playing = true;
}

/* I_StopSong() -- Stops a song */
void I_StopSong(int handle)
{
	size_t i;
	
	/* Check */
	if (!handle)
		return;
		
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
	
	/* Free song data */
	// TODO
}

/**********************
*** SOUND FUNCTIONS ***
**********************/

/* I_AddSoundDriver() -- Adds a new sound driver */
bool_t I_AddSoundDriver(I_SoundDriver_t* const a_Driver)
{
	size_t i;
	
	/* Check */
	if (!a_Driver)
		return false;
		
	/* Attempt driver initialization */
	if (a_Driver->Init && !a_Driver->Init(a_Driver))
		return false;
		
	/* Find a blank spot */
	for (i = 0; i < l_NumSoundDrivers; i++)
		if (!l_SoundDrivers[i])
		{
			l_SoundDrivers[i] = a_Driver;
			break;
		}
	// did not find one
	if (i == l_NumSoundDrivers)
	{
		// Resize the list
		Z_ResizeArray(&l_SoundDrivers, sizeof(*l_SoundDrivers), l_NumSoundDrivers, l_NumSoundDrivers + 1);
		l_SoundDrivers[l_NumSoundDrivers++] = a_Driver;
	}
	
	/* Call the success routine, if it exists */
	if (a_Driver->Success)
		a_Driver->Success(a_Driver);
		
	/* Success */
	return true;
}

/* I_RemoveSoundDriver() -- Removes a sound driver */
bool_t I_RemoveSoundDriver(I_SoundDriver_t* const a_Driver)
{
	size_t i;
	
	/* Check */
	if (!a_Driver)
		return false;
		
	/* Find driver */
	for (i = 0; i < l_NumSoundDrivers; i++)
		if (l_SoundDrivers[i] == a_Driver)
		{
			// Call destroy function
			if (l_SoundDrivers[i]->Destroy)
				l_SoundDrivers[i]->Destroy(a_Driver);
			l_SoundDrivers[i] = NULL;
			
			return true;
		}
		
	/* Driver not loaded */
	return false;
}

/* I_FindSoundDriver() -- Find a music driver that can play this format */
I_SoundDriver_t* I_FindSoundDriver(const I_SoundType_t a_Type)
{
	I_SoundDriver_t* Best = NULL;
	size_t i;
	
	/* Go through every driver */
	for (i = 0; i < l_NumSoundDrivers; i++)
		if (l_SoundDrivers[i])
			if (!Best || (Best && l_SoundDrivers[i]->Priority > Best->Priority))
				Best = l_SoundDrivers[i];
				
	/* Return the best driver, if any */
	return Best;
}

/* I_StartupSound() -- Initializes the sound system */
bool_t I_StartupSound(void)
{
	/* Add interface specific stuff */
	if (!I_SoundDriverInit())
		CONL_PrintF("I_StartupSound: Failed to add interface specific drivers.\n");
		
	/* Add exit function */
	I_AddExitFunc(I_ShutdownSound);
	
	/* Return only if sound drivers were loaded */
	return ! !l_NumSoundDrivers;
}

/* I_ShutdownSound() -- Shuts down the sound system */
void I_ShutdownSound(void)
{
	size_t i;
	
	/* Destroy all drivers */
	for (i = 0; i < l_NumSoundDrivers; i++)
		if (!I_RemoveSoundDriver(l_SoundDrivers[i]))
			CONL_PrintF("I_ShutdownSound: Failed to remove driver.\n");
			
	/* Destroy array */
	Z_Free(l_SoundDrivers);
	l_SoundDrivers = NULL;
	l_NumSoundDrivers = 0;
}

/* I_UpdateSound() -- Updates all playing sounds */
void I_UpdateSound(void)
{
}

/* I_SubmitSound() -- Submits all sounds for playing */
void I_SubmitSound(void)
{
}

size_t I_SoundBufferRequest(const I_SoundType_t a_Type, const uint8_t a_Bits, const uint16_t a_Freq, const uint8_t a_Channels, const uint32_t a_Samples)
{
	I_SoundDriver_t* Found;
	size_t RetVal;
	
	/* Check */
	if (!a_Bits || !a_Freq || !a_Channels || !a_Samples)
		return 0;
		
	/* Find driver that can play what I want */
	Found = I_FindSoundDriver(a_Type);
	
	// Check
	if (!Found)
		return 0;
		
	/* Check to see if there already is an active buffer */
	if (l_CurSoundDriver)
	{
		if (l_CurSoundDriver->UnRequest)
			l_CurSoundDriver->UnRequest(l_CurSoundDriver);
			
		// NULL out
		l_CurSoundDriver = NULL;
	}
	
	/* Request Buffer */
	// Make sure there is a request func
	if (!Found->Request)
		return 0;
		
	// Try requesting it
	RetVal = 0;
	if ((RetVal = Found->Request(Found, a_Bits, a_Freq, a_Channels, a_Samples)))
		l_CurSoundDriver = Found;
		
	/* Return if found */
	return RetVal;
}

/* I_SoundSetThreaded() -- Attempts setting threaded sound */
bool_t I_SoundSetThreaded(void (*a_ThreadFunc) (const bool_t a_Threaded))
{
	/* Check */
	if (!l_CurSoundDriver)
		return false;
		
	/* Try threading it */
	if (l_CurSoundDriver->Thread)
		return l_CurSoundDriver->Thread(l_CurSoundDriver, a_ThreadFunc);
	return false;
}

/* I_SoundBufferObtain() -- Obtain sound buffer */
void* I_SoundBufferObtain(void)
{
	/* Check */
	if (!l_CurSoundDriver)
		return NULL;
		
	/* Obtain it */
	if (l_CurSoundDriver->Obtain)
		return l_CurSoundDriver->Obtain(l_CurSoundDriver);
	return NULL;
}

/* I_SoundBufferIsFinished() -- Check if the buffer is finished playing */
bool_t I_SoundBufferIsFinished(void)
{
	/* Check */
	if (!l_CurSoundDriver)
		return false;
		
	/* Obtain it */
	if (l_CurSoundDriver->IsFinished)
		return l_CurSoundDriver->IsFinished(l_CurSoundDriver);
	return false;
}

/* I_SoundBufferWriteOut() -- Writes out the buffer */
void I_SoundBufferWriteOut(void* const a_SoundBuf, const size_t a_SoundLen, const int a_Freq, const int a_Bits, const int a_Channels)
{
	size_t i;
	
	/* Check */
	if (!l_CurSoundDriver)
		return;
		
	/* Obtain it */
	if (l_CurSoundDriver->WriteOut)
	{
		// Sound layering (internal music)
		for (i = 0; i < l_NumMusicDrivers; i++)
			if (l_MusicDrivers[i]->SoundLayer)
				l_MusicDrivers[i]->SoundLayer(l_MusicDrivers[i], l_CurSoundDriver, a_SoundBuf, a_SoundLen, a_Freq, a_Bits, a_Channels);
		
		// Call function
		l_CurSoundDriver->WriteOut(l_CurSoundDriver);
	}
}

/* I_SoundGetFreq() -- Returns frequency in Hz */
uint16_t I_SoundGetFreq(void)
{
	/* Check */
	if (!l_CurSoundDriver)
		return 0;
		
	/* Obtain it */
	if (l_CurSoundDriver->GetFreq)
		return l_CurSoundDriver->GetFreq(l_CurSoundDriver);
	return 0;
}

/* I_SoundLockThread() -- Locks or unlock sound thread */
void I_SoundLockThread(const bool_t a_Lock)
{
	/* Check */
	if (!l_CurSoundDriver)
		return;
		
	/* Change lock */
	if (l_CurSoundDriver->LockThread)
		l_CurSoundDriver->LockThread(l_CurSoundDriver, a_Lock);
}

void I_StopCD(void)
{
}

void I_PauseCD(void)
{
}

void I_ResumeCD(void)
{
}

void I_ShutdownCD(void)
{
}

void I_InitCD(void)
{
}

void I_UpdateCD(void)
{
}

void I_PlayCD(int track, bool_t looping)
{
}

int I_SetVolumeCD(int volume)
{
	return 0;
}
