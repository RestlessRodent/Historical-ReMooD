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

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "i_sound.h"
#include "i_util.h"

/**********************************
*** ALLEGRO SOUND STREAM DRIVER ***
**********************************/

/* I_AllegroSD_Init() -- Initializes a driver */
bool_t I_AllegroSD_Init(struct I_SoundDriver_s* const a_Driver)
{
	/* Check */
	if (!a_Driver)
		return false;
	
	/* Success */
	return true;
}

/* I_AllegroSD_Destroy() -- Destroys a driver */
bool_t I_AllegroSD_Destroy(struct I_SoundDriver_s* const a_Driver)
{
	/* Check */
	if (!a_Driver)
		return false;
	
	/* Success */
	return true;
}

/* I_AllegroSD_Success() -- Success */
void I_AllegroSD_Success(struct I_SoundDriver_s* const a_Driver)
{
	/* Check */
	if (!a_Driver)
		return;
}

/* I_AllegroSD_Request() -- Requests a buffer for this driver */
bool_t I_AllegroSD_Request(struct I_SoundDriver_s* const a_Driver, const uint8_t a_Bits, const uint16_t a_Freq, const uint8_t a_Channels)
{
	/* Check */
	if (!a_Driver)
		return;
	
	return false;
}

/* I_AllegroSD_Obtain() -- Obtains a buffer that was requested */
void* I_AllegroSD_Obtain(struct I_SoundDriver_s* const a_Driver)
{
	/* Check */
	if (!a_Driver)
		return;
	
	return NULL;
}

/* I_AllegroSD_IsFinished() -- Checks if the buffer is finished playing */
bool_t I_AllegroSD_IsFinished(struct I_SoundDriver_s* const a_Driver)
{
	/* Check */
	if (!a_Driver)
		return;
	
	return false;
}

/* I_AllegroSD_WriteOut() -- Done streaming into buffer */
void I_AllegroSD_WriteOut(struct I_SoundDriver_s* const a_Driver)
{
	/* Check */
	if (!a_Driver)
		return;
}

/* l_AllegroSound -- Allegro sound driver */
static I_SoundDriver_t l_AllegroSoundDriver =
{
	/* Info */
	"Allegro SoundStream",
	"allegrosnd",
	(1 << IST_WAVEFORM),
	127,
	
	/* Functions */
	I_AllegroSD_Init,
	I_AllegroSD_Destroy,
	I_AllegroSD_Success,
	I_AllegroSD_Request,
	I_AllegroSD_Obtain,
	I_AllegroSD_IsFinished,
	I_AllegroSD_WriteOut,
};

/****************
*** FUNCTIONS ***
****************/

/* I_SoundDriverInit() -- Initializes sound */
bool_t I_SoundDriverInit(void)
{
	/* Add Allegro MIDI Driver */
	if (!I_AddSoundDriver(&l_AllegroSoundDriver))
		CONS_Printf("I_SoundDriverInit: Failed to add Allegro Driver\n");
	
	/* Success */
	return true;
}

