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
#include <stdlib.h>
#include <stdint.h>
#include <allegro.h>

/* Local */
#include "doomtype.h"
#include "i_video.h"
#include "i_util.h"

/****************
*** CONSTANTS ***
****************/

/* c_AllegroCards -- Cards used by allegro */
static const uint32_t c_AllegroCards[] =
{
	// DOS
#if defined(__MSDOS__)
	GFX_VGA, GFX_MODEX, GFX_VESA1, GFX_VESA2L, GFX_VESA3, GFX_VBEAF, GFX_XTENDED, 0

	// Windows
#elif defined(_WIN32)
	GFX_DIRECTX, GFX_DIRECTX_OVL, GFX_GDI, 0,
	
	// Linux
#elif defined(__linux__)
	GFX_FBCON, GFX_VBEAF, GFX_SVGALIB, GFX_VGA, GFX_MODEX, 0

	// Unknown
#else
	0
	
	//
#endif
};

/****************
*** FUNCTIONS ***
****************/

void I_GetEvent(void)
{
}

void I_StartupMouse(void)
{
}

void I_UpdateJoysticks(void)
{
}

void I_OsPolling(void)
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
	register uint32_t y, x;
	uint32_t w, h;
	uint8_t* Buffer, *p;
	unsigned long Address;
	
	/* Obtain pointer to buffer */
	Buffer = I_VideoSoftBuffer(&w, &h);
	
	// Failed?
	if (!Buffer)
		return;
	
	/* Copy buffer to screen */
	// Select the screen
	acquire_screen();
	bmp_select(screen);
	
	// Copy row by row
	for (y = 0; y < h; y++)
	{
		// Get address
		Address = bmp_write_line(screen, y);
		p = &Buffer[y * w];
		
		// Major, by 4, then by 1
		for (x = 0; x < w; x += 4, p += 4)
			bmp_write32(Address + x, *((uint32_t*)p));
		for (; x < w; x++, p++)
			bmp_write8(Address + x, *p);
	}
	
	// Screen selection no longer needed
	bmp_unwrite_line(screen);
	release_screen();
}

void I_ReadScreen(byte* scr)
{
}

/* I_SetPalette() -- Sets the current palette */
void I_SetPalette(RGBA_t* palette)
{
	PALETTE ScrPal;
	register uint32_t i;
	int Depth;
	
	/* Check */
	if (!palette)
		return;
	
	/* Load colors into screen palette */
	// Cap in DOS?
#if defined(__MSDOS__)
	Depth = 2;
#else
	Depth = 0;
#endif
	
	// Now loop
	for (i = 0; i < 256; i++)
	{
		ScrPal[i].r = palette[i].s.red >> Depth;
		ScrPal[i].g = palette[i].s.green >> Depth;
		ScrPal[i].b = palette[i].s.blue >> Depth;
	}
	
	/* Now set it */
	set_palette(ScrPal);
}

/* VID_PrepareModeList() -- Adds video modes to the mode list */
// Allegro does not allow "magic" drivers to be passed in the mode list getting
// function. Therefor I decided to use a loop of sorts with available drivers
// for everything. This is because I tell it to autodetect anyway, so it could
// choose any of the specified drivers anyway. Also, VID_AddMode() will not
// add a duplicate mode anyway.
void VID_PrepareModeList(void)
{
	size_t i, j;
	GFX_MODE_LIST* Modes;
	
	/* Go through cards */
	for (i = 0; c_AllegroCards[i]; i++)
	{
		// Get modes for this card
		Modes = get_gfx_mode_list(c_AllegroCards[i]);
		
		// Check
		if (!Modes)
			continue;
			
		// Go through each mode
		for (j = 0; j < Modes->num_modes; j++)
			// Only allow 8-bit modes
			if (Modes->mode[j].bpp == 8)
				VID_AddMode(Modes->mode[j].width, Modes->mode[j].height, true);
		
		// Clear modes
		destroy_gfx_mode_list(Modes);
	}
}

/* I_SetVideoMode() -- Sets the current video mode */
boolean I_SetVideoMode(const uint32_t a_Width, const uint32_t a_Height, const boolean a_Fullscreen)
{
	/* Check */
	if (!a_Width || !a_Height)
		return false;
	
	/* Set new video mode */
	set_color_depth(8);	// always 8-bit color
	
	// Try initial set with a_Fullscreen honored
	if (set_gfx_mode((a_Fullscreen ? GFX_AUTODETECT_FULLSCREEN : GFX_AUTODETECT_WINDOWED), a_Width, a_Height, 0, 0) < 0)
	{
		// Print warnings
		CONS_Printf("I_SetVideoMode: Failed to set %ux%u %s\n", a_Width, a_Height, (a_Fullscreen ? "fullscreen" : "windowed"));
		
		// Try again with a_Fullscreen inverted
		if (set_gfx_mode((!a_Fullscreen ? GFX_AUTODETECT_FULLSCREEN : GFX_AUTODETECT_WINDOWED), a_Width, a_Height, 0, 0) < 0)
		{
			// Oh well
			CONS_Printf("I_SetVideoMode: Failed to fallback to %s\n", (!a_Fullscreen ? "fullscreen" : "windowed"));
			return false;
		}
	}
	
	/* Allocate Buffer */
	I_VideoUnsetBuffer();	// Remove old buffer if any
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
	
	/* Set allegro stuff */
	set_display_switch_mode(SWITCH_BACKGROUND);
	
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

