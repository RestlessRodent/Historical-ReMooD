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
#include "doomdef.h"
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
	
	// X11
#elif defined(ALLEGRO_WITH_XWINDOWS)
	GFX_XWINDOWS, GFX_XWINDOWS_FULLSCREEN, GFX_XDGA2, GFX_XDGA2_SOFT, 0
	
	// Linux
#elif defined(__linux__)
	GFX_FBCON, GFX_VBEAF, GFX_SVGALIB, GFX_VGA, GFX_MODEX, 0

	// Unknown
#else
	0
	
	//
#endif
};

const uint8_t c_AllegroToReMooDKey[KEY_MAX] =				// Converts an Allegro key to a ReMooD Key
{
	IKBK_NULL,	//	Allegro	starts	at	1,	so	we	don't	want	off
	IKBK_A,	//	KEY_A	
	IKBK_B,	//	KEY_B	
	IKBK_C,	//	KEY_C	
	IKBK_D,	//	KEY_D	
	IKBK_E,	//	KEY_E	
	IKBK_F,	//	KEY_F	
	IKBK_G,	//	KEY_G	
	IKBK_H,	//	KEY_H	
	IKBK_I,	//	KEY_I	
	IKBK_J,	//	KEY_J	
	IKBK_K,	//	KEY_K	
	IKBK_L,	//	KEY_L	
	IKBK_M,	//	KEY_M	
	IKBK_N,	//	KEY_N	
	IKBK_O,	//	KEY_O	
	IKBK_P,	//	KEY_P	
	IKBK_Q,	//	KEY_Q	
	IKBK_R,	//	KEY_R	
	IKBK_S,	//	KEY_S	
	IKBK_T,	//	KEY_T	
	IKBK_U,	//	KEY_U	
	IKBK_V,	//	KEY_V	
	IKBK_W,	//	KEY_W	
	IKBK_X,	//	KEY_X	
	IKBK_Y,	//	KEY_Y	
	IKBK_Z,	//	KEY_Z	
	IKBK_0,	//	KEY_0	
	IKBK_1,	//	KEY_1	
	IKBK_2,	//	KEY_2	
	IKBK_3,	//	KEY_3	
	IKBK_4,	//	KEY_4	
	IKBK_5,	//	KEY_5	
	IKBK_6,	//	KEY_6	
	IKBK_7,	//	KEY_7	
	IKBK_8,	//	KEY_8	
	IKBK_9,	//	KEY_9	
	IKBK_NULL,	//	KEY_0_PAD	
	IKBK_NULL,	//	KEY_1_PAD	
	IKBK_NULL,	//	KEY_2_PAD	
	IKBK_NULL,	//	KEY_3_PAD	
	IKBK_NULL,	//	KEY_4_PAD	
	IKBK_NULL,	//	KEY_5_PAD	
	IKBK_NULL,	//	KEY_6_PAD	
	IKBK_NULL,	//	KEY_7_PAD	
	IKBK_NULL,	//	KEY_8_PAD	
	IKBK_NULL,	//	KEY_9_PAD	
	IKBK_F1,	//	KEY_F1	
	IKBK_F2,	//	KEY_F2	
	IKBK_F3,	//	KEY_F3	
	IKBK_F4,	//	KEY_F4	
	IKBK_F5,	//	KEY_F5	
	IKBK_F6,	//	KEY_F6	
	IKBK_F7,	//	KEY_F7	
	IKBK_F8,	//	KEY_F8	
	IKBK_F9,	//	KEY_F9	
	IKBK_F10,	//	KEY_F10	
	IKBK_F11,	//	KEY_F11	
	IKBK_F12,	//	KEY_F12	
	IKBK_ESCAPE,	//	KEY_ESC	
	IKBK_NULL,	//	KEY_TILDE	
	IKBK_NULL,	//	KEY_MINUS	
	IKBK_NULL,	//	KEY_EQUALS	
	IKBK_NULL,	//	KEY_BACKSPACE	
	IKBK_NULL,	//	KEY_TAB	
	IKBK_NULL,	//	KEY_OPENBRACE	
	IKBK_NULL,	//	KEY_CLOSEBRACE	
	IKBK_ENTER,	//	KEY_ENTER	
	IKBK_NULL,	//	KEY_COLON	
	IKBK_NULL,	//	KEY_QUOTE	
	IKBK_NULL,	//	KEY_BACKSLASH	//	Two	of	the	same	key!?	
	IKBK_NULL,	//	KEY_BACKSLASH2	
	IKBK_NULL,	//	KEY_COMMA	
	IKBK_NULL,	//	KEY_STOP	
	IKBK_NULL,	//	KEY_SLASH	
	IKBK_SPACE,	//	KEY_SPACE	
	IKBK_NULL,	//	KEY_INSERT	
	IKBK_NULL,	//	KEY_DEL	
	IKBK_NULL,	//	KEY_HOME	
	IKBK_NULL,	//	KEY_END	
	IKBK_NULL,	//	KEY_PGUP	
	IKBK_NULL,	//	KEY_PGDN	
	IKBK_LEFT,	//	KEY_LEFT	
	IKBK_RIGHT,	//	KEY_RIGHT	
	IKBK_UP,	//	KEY_UP	
	IKBK_DOWN,	//	KEY_DOWN	
	IKBK_NULL,	//	KEY_SLASH_PAD	
	IKBK_NULL,	//	KEY_ASTERISK	
	IKBK_NULL,	//	KEY_MINUS_PAD	
	IKBK_NULL,	//	KEY_PLUS_PAD	
	IKBK_NULL,	//	KEY_DEL_PAD	
	IKBK_NULL,	//	KEY_ENTER_PAD	
	IKBK_NULL,	//	KEY_PRTSCR	
	IKBK_NULL,	//	KEY_PAUSE	
	IKBK_NULL,	//	KEY_ABNT_C1	
	IKBK_NULL,	//	KEY_YEN	
	IKBK_NULL,	//	KEY_KANA	
	IKBK_NULL,	//	KEY_CONVERT	
	IKBK_NULL,	//	KEY_NOCONVERT	
	IKBK_NULL,	//	KEY_AT	
	IKBK_NULL,	//	KEY_CIRCUMFLEX	
	IKBK_NULL,	//	KEY_COLON2	
	IKBK_NULL,	//	KEY_KANJI	
	IKBK_NULL,	//	KEY_EQUALS_PAD	
	IKBK_NULL,	//	KEY_BACKQUOTE	
	IKBK_NULL,	//	KEY_SEMICOLON	
	IKBK_NULL,	//	KEY_COMMAND	
	IKBK_NULL,	//	KEY_UNKNOWN1	
	IKBK_NULL,	//	KEY_UNKNOWN2	
	IKBK_NULL,	//	KEY_UNKNOWN3	
	IKBK_NULL,	//	KEY_UNKNOWN4	
	IKBK_NULL,	//	KEY_UNKNOWN5	
	IKBK_NULL,	//	KEY_UNKNOWN6	
	IKBK_NULL,	//	KEY_UNKNOWN7	
	IKBK_NULL,	//	KEY_UNKNOWN8	
	IKBK_SHIFT,	//	KEY_LSHIFT	
	IKBK_SHIFT,	//	KEY_RSHIFT	
	IKBK_CTRL,	//	KEY_LCONTROL	
	IKBK_CTRL,	//	KEY_RCONTROL	
	IKBK_ALT,	//	KEY_ALT	
	IKBK_ALT,	//	KEY_ALTGR	
	IKBK_NULL,	//	KEY_LWIN	
	IKBK_NULL,	//	KEY_RWIN	
	IKBK_NULL,	//	KEY_MENU	
	IKBK_NULL,	//	KEY_SCRLOCK	
	IKBK_NULL,	//	KEY_NUMLOCK	
	IKBK_NULL,	//	KEY_CAPSLOCK	
	IKBK_NULL,	//	KEY_MAX	
};

/**************
*** GLOBALS ***
**************/

extern int g_RefreshRate;

/****************
*** FUNCTIONS ***
****************/

/* IS_ConvertKey() -- Converts an allegro key to a ReMooD Key */
static uint8_t IS_ConvertKey(const size_t a_AKey)
{
	/* Check */
	if (a_AKey >= KEY_MAX)
		return 0;
	
	return c_AllegroToReMooDKey[a_AKey];
}

/* I_GetEvent() -- Gets an event and adds it to the queue */
void I_GetEvent(void)
{
	size_t i;
	static uint8_t Shifties[KEY_MAX];
	I_EventEx_t ExEvent;
	int Key;
	boolean Repeat;
	
	/* Keyboard getting */
	// Poll the keyboard
	poll_keyboard();
	
	// Grab all key presses and set shifties as such
	while (keypressed())
	{
		// Get key
		Key = readkey();
		i = (Key & 0x7F00) >> 8;
		
		// If the key is already pressed, set it as repeated
		Repeat = !!Shifties[i];
		
		// Set to 1 in Shifties (always presses here)
		Shifties[i] = 1;
		
		// Create event
		memset(&ExEvent, 0, sizeof(&ExEvent));
		ExEvent.Type = IET_KEYBOARD;
		ExEvent.Data.Keyboard.Down = 1;
		ExEvent.Data.Keyboard.Repeat = Repeat;
		ExEvent.Data.Keyboard.KeyCode = IS_ConvertKey(i);
		ExEvent.Data.Keyboard.Character = Key & 0x7F;	// Char is easy
		
		// Send away
		I_EventExPush(&ExEvent);
	}
	
	// Determine shift states
	for (i = KEY_MODIFIERS; i < KEY_MAX; i++)
	{
		// Only check if a key is pressed
		if (!(!Shifties[i] && key[i]))
			continue;
		
		// Mark it as up
		Shifties[i] = 1;
		
		// Create event
		memset(&ExEvent, 0, sizeof(&ExEvent));
		ExEvent.Type = IET_KEYBOARD;
		ExEvent.Data.Keyboard.Down = 1;
		ExEvent.Data.Keyboard.Repeat = 0;
		ExEvent.Data.Keyboard.KeyCode = IS_ConvertKey(i);
		ExEvent.Data.Keyboard.Character = 0;
		
		// Send away
		I_EventExPush(&ExEvent);
	}
	
	// Determine key releases by going through shifties and keys
	for (i = 1; i < KEY_MAX; i++)
	{
		// Only make events for keys that are depressed
		if (!(Shifties[i] && !key[i]))
			continue;
		
		// Key is no longer up, so clear it
		Shifties[i] = 0;
		
		// Create event
		memset(&ExEvent, 0, sizeof(&ExEvent));
		ExEvent.Type = IET_KEYBOARD;
		ExEvent.Data.Keyboard.Down = 0;
		ExEvent.Data.Keyboard.Repeat = false;
		ExEvent.Data.Keyboard.KeyCode = IS_ConvertKey(i);
		ExEvent.Data.Keyboard.Character = 0;	// Ignore character
		
		// Send away
		I_EventExPush(&ExEvent);
	}
}

void I_StartupMouse(void)
{
}

void I_UpdateJoysticks(void)
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

/* I_SetPalette() -- Sets the current palette */
void I_SetPalette(RGBA_t* palette)
{
	PALETTE ScrPal;
	register uint32_t i;
	
	/* Check */
	if (!palette)
		return;
	
	/* Load colors into screen palette */
	// Now loop
	for (i = 0; i < 256; i++)
	{
		// Allegro is limited to 6-bit precision for some reason
		ScrPal[i].r = palette[i].s.red >> 2;
		ScrPal[i].g = palette[i].s.green >> 2;
		ScrPal[i].b = palette[i].s.blue >> 2;
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
	
	/* Always add 320x200, 320x240, 640x400, 640x480 */
	VID_AddMode(320, 200, true);
	VID_AddMode(320, 240, true);
	VID_AddMode(640, 400, true);
	VID_AddMode(640, 480, true);
}

/* I_SetVideoMode() -- Sets the current video mode */
boolean I_SetVideoMode(const uint32_t a_Width, const uint32_t a_Height, const boolean a_Fullscreen)
{
	/* Check */
	if (!a_Width || !a_Height)
		return false;
	
	/* Destroy old buffer */
	I_VideoUnsetBuffer();	// Remove old buffer if any
	
	/* Set new video mode */
	request_refresh_rate(70);
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
	I_VideoSetBuffer(a_Width, a_Height, a_Width, NULL);
	
	/* Set title */
	set_window_title("ReMooD "REMOOD_FULLVERSIONSTRING);
	
	/* Get refresh rate */
	g_RefreshRate = get_refresh_rate();
	
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

