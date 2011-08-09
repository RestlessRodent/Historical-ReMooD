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

/***************
*** INCLUDES ***
***************/

/* Include Doomtype first */
#include "doomtype.h"

/* System */
#include <allegro.h>

/* Local */
#include "i_util.h"
#include "i_sound.h"
#include "m_argv.h"

/****************
*** CONSTANTS ***
****************/

/*****************
*** STRUCTURES ***
*****************/

/* I_AllegroMIDILocal_t -- Allegro local data */
typedef struct I_AllegroMIDILocal_s
{
	int HandleSet;
	MIDI* CurrentMIDI;								// Pointer to current song
	int CurrentHandle;								// Current song being played
} I_AllegroMIDILocal_t;

/*********************
*** ALLEGRO DRIVER ***
*********************/

/* I_AllegroMD_Init() -- Initialize music */
static bool_t I_AllegroMD_Init(struct I_MusicDriver_s* const a_Driver)
{
	MIDI* mid;
	
	/* Check */
	if (!a_Driver)
		return false;
	
	/* Attempt detection of MIDI Driver */
	if (detect_midi_driver(MIDI_AUTODETECT) == 0)
		return false;
	
	/* Initialize sound */
	if (install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL) == -1)
		CONS_Printf("I_AllegroMD_Init: Failed to install sound (once).\n");
	
	/* Force load of all MIDI patches */
	// This is so midi_out works!
	// Took me awhile to figure out why
	load_midi_patches();
	
	/* Success */
	CONS_Printf("I_AllegroMD_Init: Success!\n");
	return true;
}

/* I_AllegroMD_Success() -- Driver initialization succeeded */
void I_AllegroMD_Success(struct I_MusicDriver_s* const a_Driver)
{
	/* Check */
	if (!a_Driver)
		return;
	
	/* Allocate local data */
	a_Driver->Size = sizeof(I_AllegroMIDILocal_t);
	a_Driver->Data = Z_Malloc(a_Driver->Size, PU_STATIC, NULL);
}

/* I_AllegroMD_Stop() -- Stop playing MIDI */
static void I_AllegroMD_Stop(struct I_MusicDriver_s* const a_Driver, const int a_Handle)
{
	I_AllegroMIDILocal_t* Local;
	
	/* Check */
	if (!a_Driver || !a_Handle)
		return;
	
	/* Get Local */
	Local = a_Driver->Data;
	
	/* Only stop if the current song is one playing */
	if (Local->CurrentHandle == a_Handle)
		stop_midi();
}

/* I_AllegroMD_Play() -- Plays a MIDI */
static int I_AllegroMD_Play(struct I_MusicDriver_s* const a_Driver, const void* const a_Data, const size_t a_Size, const bool_t Loop)
{
	I_AllegroMIDILocal_t* Local;
	
	/* Check */
	if (!a_Driver || !a_Data)
		return;
	
	/* Get Local */
	Local = a_Driver->Data;
	
	/* If there currently is another MIDI loaded, destroy it */
	destroy_midi(Local->CurrentMIDI);
	Local->CurrentMIDI = NULL;
	Local->CurrentHandle = 0;
	
	/* Load in the new MIDI */
	Local->CurrentMIDI = load_midi(a_Data);
	
	// Check load
	if (!Local->CurrentMIDI)
	{
		CONS_Printf("I_AllegroMD_Play: Failed to load MIDI.\n");
		return 0;
	}
	
	// Boost handle
	Local->CurrentHandle = ++Local->HandleSet;
	
	// Start playing
	play_midi(Local->CurrentMIDI, Loop);
	
	return Local->CurrentHandle;
}

/* I_AllegroMD_RawMIDI() -- Send a raw MIDI message */
void I_AllegroMD_RawMIDI(struct I_MusicDriver_s* const a_Driver, const uint32_t a_Msg, const uint32_t a_BitLength)
{
	/* Check */
	if (!a_Driver || !a_BitLength)
		return;
	
	/* Use Allegro MIDI out */
	midi_out(&a_Msg, (a_BitLength < sizeof(a_Msg) ? a_BitLength : sizeof(a_Msg)));
}

/* l_AllegroDriver -- Allegro Music driver */
static I_MusicDriver_t l_AllegroDriver =
{
	/* Data */
	"Allegro MIDI Driver",
	"allegromidi",
	(1 << IMT_MIDI),
	true,
	100,
	
	/* Dynamic */
	NULL,
	0,
	
	/* Handlers */
	I_AllegroMD_Init,
	NULL,
	I_AllegroMD_Success,
	NULL,
	NULL,
	I_AllegroMD_Stop,
	NULL,
	NULL,
	I_AllegroMD_Play,
	NULL,
	I_AllegroMD_RawMIDI,
	NULL,
};

/****************
*** FUNCTIONS ***
****************/

/* I_MusicDriverInit() -- Initialize all music drivers */
bool_t I_MusicDriverInit(void)
{
	/* Add Allegro MIDI Driver */
	if (!I_AddMusicDriver(&l_AllegroDriver))
		CONS_Printf("I_MusicDriverInit: Failed to add Allegro Driver\n");
	
	/* Success */
	return true;
}

