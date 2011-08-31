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
// Copyright (C) 2011 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: DOOM graphics stuff for Allegro

/***************
*** INCLUDES ***
***************/

/* System */
#include <SDL.h>
#include <stdlib.h>
#include <stdint.h>

/* Local */
#include "doomtype.h"
#include "doomdef.h"
#include "i_video.h"
#include "i_util.h"

#define __G_INPUT_H__
#include "console.h"

/****************
*** CONSTANTS ***
****************/

/*************
*** LOCALS ***
*************/

static SDL_Surface* l_SDLSurface = NULL;				// SDL Video plane

/****************
*** FUNCTIONS ***
****************/

/* I_GetEvent() -- Gets an event and adds it to the queue */
void I_GetEvent(void)
{
}

void I_UpdateNoBlit(void)
{
}

/* I_StartFrame() -- Called before drawing a frame */
void I_StartFrame(void)
{
}

/* I_FinishUpdate() -- Called after drawing a frame */
void I_FinishUpdate(void)
{
	register uint32_t y;
	uint32_t w, h;
	uint8_t* Buffer;
	void* Dest, *Src;
	
	/* Obtain pointer to buffer */
	Buffer = I_VideoSoftBuffer(&w, &h);
	
	// Failed?
	if (!Buffer)
		return;
	
	/* Lock surface */
	SDL_LockSurface(l_SDLSurface);
		
	/* Copy row by row */
	for (y = 0; y < h; y++)
	{
		// Get pixels to copy and overwrite
		Src = &Buffer[(y * w)];
		Dest = &((uint8_t*)l_SDLSurface->pixels)[(y * l_SDLSurface->pitch)];
		
		// Mem copy!
		memcpy(Dest, Src, w);
	}
	
	/* Unlock Surface */
	SDL_UnlockSurface(l_SDLSurface);
	
	/* Update Rectangle */
	SDL_Flip(l_SDLSurface);
}

/* I_SetPalette() -- Sets the current palette */
void I_SetPalette(RGBA_t* palette)
{
	size_t i;
	SDL_Color Colors[256];
	
	/* No surface or palette? */
	if (!l_SDLSurface || !palette)
		return;
	
	/* Copy colors as is */
	for (i = 0; i < 256; i++)
	{
		Colors[i].r = palette[i].s.red;
		Colors[i].g = palette[i].s.green;
		Colors[i].b = palette[i].s.blue;
	}
	
	/* Set colors away */
	SDL_SetPalette(l_SDLSurface, SDL_PHYSPAL, Colors, 0, 256);
}

/* VID_PrepareModeList() -- Adds video modes to the mode list */
// Allegro does not allow "magic" drivers to be passed in the mode list getting
// function. Therefor I decided to use a loop of sorts with available drivers
// for everything. This is because I tell it to autodetect anyway, so it could
// choose any of the specified drivers anyway. Also, VID_AddMode() will not
// add a duplicate mode anyway.
void VID_PrepareModeList(void)
{
	VID_AddMode(320, 200, true);
	
}

/* I_SetVideoMode() -- Sets the current video mode */
bool_t I_SetVideoMode(const uint32_t a_Width, const uint32_t a_Height, const bool_t a_Fullscreen)
{
	uint32_t SDLFlags = 0;
	
	/* Check */
	if (!a_Width || !a_Height)
		return false;
	
	/* Destroy old buffer */
	I_VideoUnsetBuffer();	// Remove old buffer if any
	
	/* Destroy old surface */
	if (l_SDLSurface)
		SDL_FreeSurface(l_SDLSurface);
	l_SDLSurface = NULL;
	
	/* Find flags to set */
	SDLFlags = SDL_HWPALETTE | SDL_DOUBLEBUF;
	if (a_Fullscreen)
		SDLFlags |= SDL_FULLSCREEN | SDL_HWSURFACE;
	else
		SDLFlags |= SDL_SWSURFACE;
	
	/* Create SDL surface */
	l_SDLSurface = SDL_SetVideoMode(a_Width, a_Height, 8, SDLFlags);
	
	// Failed?
	if (!l_SDLSurface)
		return false;
	
	/* Set Title */
	SDL_WM_SetCaption("ReMooD "REMOOD_FULLVERSIONSTRING, "ReMooD");
	
	/* Allocate Buffer */
	I_VideoSetBuffer(a_Width, a_Height, a_Width, NULL);
	
	/* Success */
	return true;
}

/* I_StartupGraphics() -- Initializes graphics */
void I_StartupGraphics(void)
{
	/* Pre-initialize video */
	if (!I_VideoPreInit())
		return;
	
	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO) == -1)
	{
		CONS_Printf("I_StartupGraphics: Failed to initialize SDL graphics.\n");
		return;
	}
	
	/* Initialize before mode set */
	if (!I_VideoBefore320200Init())
		return;
	if (!I_SetVideoMode(320, 200, false))	// 320x200 console scroller, never fullscreen
		return;

	/* Prepare the video mode list */
	if (!I_VideoPostInit())
		return;
}

/* I_ShutdownGraphics() -- Turns off graphics */
void I_ShutdownGraphics(void)
{
}

/* I_TextMode() -- Enter and leaves text mode */
bool_t I_TextMode(const bool_t a_OnOff)
{
	/* On */
	if (a_OnOff)
	{
		return true;
	}
	
	/* Off */
	else
	{
		return true;
	}
}

/* I_ProbeJoysticks() -- Probes all joysticks */
size_t I_ProbeJoysticks(void)
{
	return 0;
}

/* I_RemoveJoysticks() -- Removes all joysticks */
void I_RemoveJoysticks(void)
{
}

/* I_ProbeMouse() -- Probes Mice */
bool_t I_ProbeMouse(const size_t a_ID)
{
	return true;
}

/* I_RemoveMouse() -- Removes mice */
bool_t I_RemoveMouse(const size_t a_ID)
{
	return true;
}

/* I_MouseGrab() -- Sets mouse grabbing */
void I_MouseGrab(const bool_t a_Grab)
{
}

