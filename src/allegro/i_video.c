// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
//         :oCCCCOCoc.
//     .cCO8OOOOOOOOO8Oo:
//   .oOO8OOOOOOOOOOOOOOOCc
//  cO8888:         .:oOOOOC.                                                TM
// :888888:   :CCCc   .oOOOOC.     ###      ###                    #########
// C888888:   .ooo:   .C########   #####  #####  ######    ######  ##########
// O888888:         .oO###    ###  #####  ##### ########  ######## ####    ###
// C888888:   :8O.   .C##########  ### #### ### ##    ##  ##    ## ####    ###
// :8@@@@8:   :888c   o###         ### #### ### ########  ######## ##########
//  :8@@@@C   C@@@@   oo########   ###  ##  ###  ######    ######  #########
//    cO@@@@@@@@@@@@@@@@@Oc0
//      :oO8@@@@@@@@@@Oo.
//         .oCOOOOOCc.                                      http://remood.org/
// ----------------------------------------------------------------------------
// Copyright (C) 2011-2013 GhostlyDeath <ghostlydeath@remood.org>
//                                      <ghostlydeath@gmail.com>
// ----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// ----------------------------------------------------------------------------
// DESCRIPTION: DOOM graphics stuff for Allegro

/***************
*** INCLUDES ***
***************/

/* System */
#include <stdlib.h>
#if defined(__DJGPP__)
#include <stdint.h>
#endif
#include <allegro.h>

// Include winalleg on Windows since it conflicts!
#if defined(_WIN32)
#include <winalleg.h>
#endif

/* Local */
#define __REMOOD_IGNORE_FIXEDTYPES
#include "doomtype.h"
#include "doomdef.h"
#include "i_video.h"
#include "i_util.h"
#include "../remood.xpm"		// For allegro

#define __G_INPUT_H__
#include "console.h"

/****************
*** CONSTANTS ***
****************/

#if defined(ALLEGRO_WITH_XWINDOWS)
void* allegro_icon = remood_xpm;
#endif

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

const uint8_t c_AllegroToReMooDKey[KEY_MAX] =	// Converts an Allegro key to a ReMooD Key
{
	IKBK_NULL,					//  Allegro starts  at  1,  so  we  don't   want    off
	IKBK_A,						//  KEY_A
	IKBK_B,						//  KEY_B
	IKBK_C,						//  KEY_C
	IKBK_D,						//  KEY_D
	IKBK_E,						//  KEY_E
	IKBK_F,						//  KEY_F
	IKBK_G,						//  KEY_G
	IKBK_H,						//  KEY_H
	IKBK_I,						//  KEY_I
	IKBK_J,						//  KEY_J
	IKBK_K,						//  KEY_K
	IKBK_L,						//  KEY_L
	IKBK_M,						//  KEY_M
	IKBK_N,						//  KEY_N
	IKBK_O,						//  KEY_O
	IKBK_P,						//  KEY_P
	IKBK_Q,						//  KEY_Q
	IKBK_R,						//  KEY_R
	IKBK_S,						//  KEY_S
	IKBK_T,						//  KEY_T
	IKBK_U,						//  KEY_U
	IKBK_V,						//  KEY_V
	IKBK_W,						//  KEY_W
	IKBK_X,						//  KEY_X
	IKBK_Y,						//  KEY_Y
	IKBK_Z,						//  KEY_Z
	IKBK_0,						//  KEY_0
	IKBK_1,						//  KEY_1
	IKBK_2,						//  KEY_2
	IKBK_3,						//  KEY_3
	IKBK_4,						//  KEY_4
	IKBK_5,						//  KEY_5
	IKBK_6,						//  KEY_6
	IKBK_7,						//  KEY_7
	IKBK_8,						//  KEY_8
	IKBK_9,						//  KEY_9
	IKBK_NUM0,					//  KEY_0_PAD
	IKBK_NUM1,					//  KEY_1_PAD
	IKBK_NUM2,					//  KEY_2_PAD
	IKBK_NUM3,					//  KEY_3_PAD
	IKBK_NUM4,					//  KEY_4_PAD
	IKBK_NUM5,					//  KEY_5_PAD
	IKBK_NUM6,					//  KEY_6_PAD
	IKBK_NUM7,					//  KEY_7_PAD
	IKBK_NUM8,					//  KEY_8_PAD
	IKBK_NUM9,					//  KEY_9_PAD
	IKBK_F1,					//  KEY_F1
	IKBK_F2,					//  KEY_F2
	IKBK_F3,					//  KEY_F3
	IKBK_F4,					//  KEY_F4
	IKBK_F5,					//  KEY_F5
	IKBK_F6,					//  KEY_F6
	IKBK_F7,					//  KEY_F7
	IKBK_F8,					//  KEY_F8
	IKBK_F9,					//  KEY_F9
	IKBK_F10,					//  KEY_F10
	IKBK_F11,					//  KEY_F11
	IKBK_F12,					//  KEY_F12
	IKBK_ESCAPE,				//  KEY_ESC
	IKBK_TILDE,					//  KEY_TILDE
	IKBK_MINUS,					//  KEY_MINUS
	IKBK_EQUALS,				//  KEY_EQUALS
	IKBK_BACKSPACE,				//  KEY_BACKSPACE
	IKBK_TAB,					//  KEY_TAB
	IKBK_LEFTBRACE,				//  KEY_OPENBRACE
	IKBK_RIGHTBRACE,			//  KEY_CLOSEBRACE
	IKBK_RETURN,				//  KEY_ENTER
	IKBK_COLON,					//  KEY_COLON
	IKBK_QUOTE,					//  KEY_QUOTE
	IKBK_BACKSLASH,				//  KEY_BACKSLASH   //  Two of  the same    key!?
	IKBK_BACKSLASH,				//  KEY_BACKSLASH2
	IKBK_COMMA,					//  KEY_COMMA
	IKBK_PERIOD,				//  KEY_STOP
	IKBK_FORWARDSLASH,			//  KEY_SLASH
	IKBK_SPACE,					//  KEY_SPACE
	IKBK_INSERT,				//  KEY_INSERT
	IKBK_KDELETE,				//  KEY_DEL
	IKBK_HOME,					//  KEY_HOME
	IKBK_END,					//  KEY_END
	IKBK_PAGEUP,				//  KEY_PGUP
	IKBK_PAGEDOWN,				//  KEY_PGDN
	IKBK_LEFT,					//  KEY_LEFT
	IKBK_RIGHT,					//  KEY_RIGHT
	IKBK_UP,					//  KEY_UP
	IKBK_DOWN,					//  KEY_DOWN
	IKBK_NUMDIVIDE,				//  KEY_SLASH_PAD
	IKBK_NUMMULTIPLY,			//  KEY_ASTERISK
	IKBK_NUMSUBTRACT,			//  KEY_MINUS_PAD
	IKBK_NUMADD,				//  KEY_PLUS_PAD
	IKBK_NUMDELETE,				//  KEY_DEL_PAD
	IKBK_NUMENTER,				//  KEY_ENTER_PAD
	IKBK_PRINTSCREEN,			//  KEY_PRTSCR
	IKBK_PAUSE,					//  KEY_PAUSE
	IKBK_NULL,					//  KEY_ABNT_C1
	IKBK_NULL,					//  KEY_YEN
	IKBK_NULL,					//  KEY_KANA
	IKBK_NULL,					//  KEY_CONVERT
	IKBK_NULL,					//  KEY_NOCONVERT
	IKBK_AT,					//  KEY_AT
	IKBK_CARET,					//  KEY_CIRCUMFLEX
	IKBK_COLON,					//  KEY_COLON2
	IKBK_NULL,					//  KEY_KANJI
	IKBK_NUMENTER,				//  KEY_EQUALS_PAD
	IKBK_GRAVE,					//  KEY_BACKQUOTE
	IKBK_SEMICOLON,				//  KEY_SEMICOLON
	IKBK_WINDOWSKEY,			//  KEY_COMMAND
	IKBK_NULL,					//  KEY_UNKNOWN1
	IKBK_NULL,					//  KEY_UNKNOWN2
	IKBK_NULL,					//  KEY_UNKNOWN3
	IKBK_NULL,					//  KEY_UNKNOWN4
	IKBK_NULL,					//  KEY_UNKNOWN5
	IKBK_NULL,					//  KEY_UNKNOWN6
	IKBK_NULL,					//  KEY_UNKNOWN7
	IKBK_NULL,					//  KEY_UNKNOWN8
	IKBK_SHIFT,					//  KEY_LSHIFT
	IKBK_SHIFT,					//  KEY_RSHIFT
	IKBK_CTRL,					//  KEY_LCONTROL
	IKBK_CTRL,					//  KEY_RCONTROL
	IKBK_ALT,					//  KEY_ALT
	IKBK_ALT,					//  KEY_ALTGR
	IKBK_WINDOWSKEY,			//  KEY_LWIN
	IKBK_WINDOWSKEY,			//  KEY_RWIN
	IKBK_MENUKEY,				//  KEY_MENU
	IKBK_SCROLLLOCK,			//  KEY_SCRLOCK
	IKBK_NUMLOCK,				//  KEY_NUMLOCK
	IKBK_CAPSLOCK,				//  KEY_CAPSLOCK
};

/**************
*** GLOBALS ***
**************/

extern int g_RefreshRate;

/*************
*** LOCALS ***
*************/

static int32_t l_OldMousePos[2];	// Old mouse position on the screen
static bool_t l_DoGrab = false;	// Do mouse grabbing

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
#define MAXJOYAXIS 8
	size_t i, j, k, z, a, m;
	static uint8_t Shifties[KEY_MAX];
	static uint16_t LastUnic[KEY_MAX];
	static uint32_t JoyButtons[MAXJOYSTICKS];
	static int32_t JoyAxis[MAXJOYSTICKS][MAXJOYAXIS];
	static uint32_t MouseButtons;
	I_EventEx_t ExEvent;
	int32_t MousePos[2];
	int Key;
	bool_t Repeat;
	
	/* Mouse Getting */
	if (poll_mouse() != -1)
	{
		// Remember position
		MousePos[0] = l_OldMousePos[0];
		MousePos[1] = l_OldMousePos[1];
		
		// Handle position (but only if it changed)
		if (mouse_x != l_OldMousePos[0] || mouse_y != l_OldMousePos[1])
		{
			memset(&ExEvent, 0, sizeof(ExEvent));
			ExEvent.Type = IET_MOUSE;
			ExEvent.Data.Mouse.Button = 0;
			ExEvent.Data.Mouse.Pos[0] = mouse_x;
			ExEvent.Data.Mouse.Pos[1] = mouse_y;
			ExEvent.Data.Mouse.Move[0] = ExEvent.Data.Mouse.Pos[0] - l_OldMousePos[0];
			ExEvent.Data.Mouse.Move[1] = l_OldMousePos[1] - ExEvent.Data.Mouse.Pos[1];
			l_OldMousePos[0] = ExEvent.Data.Mouse.Pos[0];
			l_OldMousePos[1] = ExEvent.Data.Mouse.Pos[1];
			
			// Update position
			MousePos[0] = l_OldMousePos[0];
			MousePos[1] = l_OldMousePos[1];
			
			// Send away
			I_EventExPush(&ExEvent);
			
			// Revert mouse position
			if (l_DoGrab)
			{
				position_mouse(SCREEN_W >> 1, SCREEN_H >> 1);
				l_OldMousePos[0] = SCREEN_W >> 1;
				l_OldMousePos[1] = SCREEN_H >> 1;
			}
		}
		
		// Handle Buttons
		for (i = 0; i < 15; i++)
		{
			// Use repeat
			Repeat = false;
			
			// Clear event
			memset(&ExEvent, 0, sizeof(ExEvent));
			ExEvent.Type = IET_MOUSE;
			
			// If button is on and our array is off, then button was pressed
			if ((mouse_b & (1 << i)) && !(MouseButtons & (1 << i)))
			{
				Repeat = true;
				ExEvent.Data.Mouse.Down = 1;
			}
			
			// If button is off and our array is on, then button was released
			else if (!(mouse_b & (1 << i)) && (MouseButtons & (1 << i)))
			{
				Repeat = true;
				ExEvent.Data.Mouse.Down = 0;
			}
			
			// Button not changed
			if (!Repeat)
				continue;
				
			// Set array info
			MouseButtons &= (~(1 << i));
			MouseButtons |= (ExEvent.Data.Mouse.Down ? (1 << i) : 0);
			
			// Set event stuff
			ExEvent.Data.Mouse.Button = i + 1;
			ExEvent.Data.Mouse.Pos[0] = MousePos[0];
			ExEvent.Data.Mouse.Pos[1] = MousePos[1];
			
			// Send away
			I_EventExPush(&ExEvent);
		}
	}
	
	/* Joystick Getting */
	if (poll_joystick() == 0)
	{
		m = num_joysticks;
		if (m >= MAXJOYSTICKS)
			m = MAXJOYSTICKS;
			
		// For every single joystick
		for (i = 0; i < m; i++)
		{
			z = joy[i].num_buttons;
			if (z >= JOYBUTTONS)
				z = JOYBUTTONS;
				
			// Check for button changes
			for (j = 0; j < z; j++)
			{
				// Reuse Repeat
				Repeat = false;
				
				// Clear event
				memset(&ExEvent, 0, sizeof(ExEvent));
				ExEvent.Type = IET_JOYSTICK;
				ExEvent.Data.Joystick.JoyID = i;
				ExEvent.Data.Joystick.Button = 0;
				
				// If button is on and our array is off, then button was pressed
				if (joy[i].button[j].b && !(JoyButtons[i] & (1 << j)))
				{
					Repeat = true;
					ExEvent.Data.Joystick.Down = 1;
				}
				
				// If button is off and our array is on, then button was released
				else if (!joy[i].button[j].b && (JoyButtons[i] & (1 << j)))
				{
					Repeat = true;
					ExEvent.Data.Joystick.Down = 0;
				}
				
				// Button not changed
				if (!Repeat)
					continue;
					
				// Set array info
				JoyButtons[i] &= (~(1 << j));
				JoyButtons[i] |= (ExEvent.Data.Joystick.Down ? (1 << j) : 0);
				
				// Set event stuff
				ExEvent.Data.Joystick.Button = j + 1;
				
				// Send away
				I_EventExPush(&ExEvent);
			}
			
			// Check for axis changes
			z = joy[i].num_sticks;
			
			for (a = 0, j = 0; j < z; j++)
				// For every stick's axis
				for (k = 0; k < joy[i].stick[j].num_axis; k++, a++)
					// Only if changed
					if (JoyAxis[i][a] != joy[i].stick[j].axis[k].pos)
						// Only if axis is not overflowed
						if (a < MAXJOYAXIS)
						{
							// Send event
							memset(&ExEvent, 0, sizeof(ExEvent));
							ExEvent.Type = IET_JOYSTICK;
							ExEvent.Data.Joystick.JoyID = i;
							ExEvent.Data.Joystick.Button = 0;
							ExEvent.Data.Joystick.Axis = a + 1;
							ExEvent.Data.Joystick.Value = joy[i].stick[j].axis[k].pos;
							
							// Map -127 to 127 and then make it -32768 to 32767
							ExEvent.Data.Joystick.Value *= 258;
							
							if (ExEvent.Data.Joystick.Value < 0)
								ExEvent.Data.Joystick.Value -= 2;
							else if (ExEvent.Data.Joystick.Value > 0)
								ExEvent.Data.Joystick.Value++;
								
							I_EventExPush(&ExEvent);
							
							// Update old position
							JoyAxis[i][a] = joy[i].stick[j].axis[k].pos;
						}
		}
	}
	
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
		memset(&ExEvent, 0, sizeof(ExEvent));
		ExEvent.Type = IET_KEYBOARD;
		ExEvent.Data.Keyboard.Down = 1;
		ExEvent.Data.Keyboard.Repeat = Repeat;
		ExEvent.Data.Keyboard.KeyCode = IS_ConvertKey(i);
		LastUnic[i] = ExEvent.Data.Keyboard.Character = Key & 0x7F;	// Char is easy
		
		// Send away
		I_EventExPush(&ExEvent);
	}
	
	// Determine shift states
	for (i = KEY_MODIFIERS; i < KEY_MAX; i++)
	{
		// Only check if a key is pressed
		if (!(!Shifties[i] && key[i]))
			continue;
			
		// Mark it as down
		Shifties[i] = 1;
		
		// Create event
		memset(&ExEvent, 0, sizeof(ExEvent));
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
			
		// Key is no longer down, so clear it
		Shifties[i] = 0;
		
		// Create event
		memset(&ExEvent, 0, sizeof(ExEvent));
		ExEvent.Type = IET_KEYBOARD;
		ExEvent.Data.Keyboard.Down = 0;
		ExEvent.Data.Keyboard.Repeat = false;
		ExEvent.Data.Keyboard.KeyCode = IS_ConvertKey(i);
		ExEvent.Data.Keyboard.Character = LastUnic[i];	// Use last inputted char
		
		// Send away
		I_EventExPush(&ExEvent);
	}
	
#undef MAXJOYAXIS
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
		
	/* Remove the pointer */
	// Otherwise bad things will happen
	scare_mouse();
	
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
	
	/* Restore the pointer */
	unscare_mouse();
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
bool_t I_SetVideoMode(const uint32_t a_Width, const uint32_t a_Height, const bool_t a_Fullscreen, const uint8_t a_Depth)
{
#if defined(_WIN32)
	HWND hWnd;
	HICON icoBig, icoSmall;
#endif
	int32_t TruDepth;
	
	/* Check */
	if (!a_Width || !a_Height)
		return false;
	
	/* Set wanted color depth */
	if (a_Depth == I_VIDEOGLMODECONST)
		TruDepth = 32;	// OpenGL not supported in Allegro
	else
		TruDepth = a_Depth * 8;
		
	/* Destroy old buffer */
	I_VideoUnsetBuffer();		// Remove old buffer if any
	
	/* Set new video mode */
	request_refresh_rate(70);
	
	for (;;)
	{
		// Set color depth to the desired depth
		set_color_depth(TruDepth);
	
		// Try initial set with a_Fullscreen honored
		if (set_gfx_mode((a_Fullscreen ? GFX_AUTODETECT_FULLSCREEN : GFX_AUTODETECT_WINDOWED), a_Width, a_Height, 0, 0) < 0)
		{
			// Print warnings
			CONL_PrintF("I_SetVideoMode: Failed to set %ux%u %s\n", a_Width, a_Height, (a_Fullscreen ? "fullscreen" : "windowed"));
		
			// Try again with a_Fullscreen inverted
			if (set_gfx_mode((!a_Fullscreen ? GFX_AUTODETECT_FULLSCREEN : GFX_AUTODETECT_WINDOWED), a_Width, a_Height, 0, 0) < 0)
			{
				// Oh well
				CONL_PrintF("I_SetVideoMode: Failed to fallback to %s\n", (!a_Fullscreen ? "fullscreen" : "windowed"));
				
				// Reduce color depth
				TruDepth >>= 1;
				
				// If reached the end, fail
				if (TruDepth <= 0)
					return false;
			}
		}
	}
	
	/* Allocate Buffer */
	I_VideoSetBuffer(a_Width, a_Height, a_Width, NULL, false, false, TruDepth / 8);
	
	/* Set title and icon */
	// Set Title
	set_window_title("ReMooD " REMOOD_FULLVERSIONSTRING);
	
	// Set Icon (on Win32)
#if defined(_WIN32)
	hWnd = win_get_window();
	
	// Now attempt getting the class and updating the window
	if (hWnd)
	{
		// Get Icons
		icoBig = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(1));
		icoSmall = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(2));
		
		// Set window classes
#ifdef _WIN64
		SetClassLongPtr(hWnd, GCLP_HICON, (LONG_PTR) icoBig);
		SetClassLongPtr(hWnd, GCLP_HICONSM, (LONG_PTR) icoSmall);
#else
		SetClassLong(hWnd, GCL_HICON, (LONG) icoBig);
		SetClassLong(hWnd, GCL_HICONSM, (LONG) icoSmall);
#endif
	}
#endif
	
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
	set_display_switch_mode(SWITCH_BACKAMNESIA);
	
	/* Generic Init */
	I_VideoGenericInit();
}

/* I_ShutdownGraphics() -- Turns off graphics */
void I_ShutdownGraphics(void)
{
	/* Go back to text mode */
	set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
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
	size_t i;
	char* CalMsg;
	bool_t Cal = false;
	
	/* Try installing joysticks */
	if (install_joystick(JOY_TYPE_AUTODETECT) != 0)
	{
		CONL_PrintF("I_ProbeJoysticks: Failed to install Allegro joystick handler.\n");
		return 0;
	}
	
	/* No joysticks? */
	if (!num_joysticks)
		return 0;
		
	/* Joystick calibration is possibly required */
	for (i = 0; i < num_joysticks; i++)
		if (joy[i].flags & JOYFLAG_CALIBRATE)
		{
			// Try loading joystick data
			if (Cal || (!Cal && load_joystick_data(NULL) == 0))
			{
				Cal = true;
				continue;
			}
			
			if (!(joy[i].flags & JOYFLAG_CALIBRATE))
				continue;
				
			// Turn off loading screen
			g_QuietConsole = false;
			
			// While the sticks need calibration
			while (joy[i].flags & JOYFLAG_CALIBRATE)
			{
				// Get what needs calibration
				CalMsg = calibrate_joystick_name(i);
				
				// Print message
				CONL_PrintF("I_ProbeJoysticks: %s, then press any key.\n", CalMsg);
				readkey();
				
				// Do calibration
				if (calibrate_joystick(i) != 0)
				{
					CONL_PrintF("I_ProbeJoysticks: Messed up calibrating joystick %i.\n", i);
					break;
				}
			}
		}
		
	/* Save calibration data */
	if (save_joystick_data(NULL) != 0)
		CONL_PrintF("I_ProbeJoysticks: Could not save joystick data.\n");
		
	/* Return number of found joys */
	return num_joysticks;
}

/* I_RemoveJoysticks() -- Removes all joysticks */
void I_RemoveJoysticks(void)
{
	/* Just remove the joysticks */
	remove_joystick();
}

/* I_GetJoystickID() -- Gets name of the joysticks */
bool_t I_GetJoystickID(const size_t a_JoyID, uint32_t* const a_Code, char* const a_Text, const size_t a_TextSize, char* const a_Cool, const size_t a_CoolSize)
{
	/* Check */
	if (!a_Code || (!a_Text && !a_TextSize) || (!a_Cool && !a_CoolSize))
		return false;
		
	/* No joysticks? */
	if (!num_joysticks || a_JoyID >= (size_t) num_joysticks)
		return false;
		
	/* Send Code */
	if (a_Code)
	{
		// Allegro has no real method of identifying sticks at all
		*a_Code = a_JoyID;
	}
	
	/* Send Name */
	if (a_Text && a_TextSize)
	{
		// Same as above
		snprintf(a_Text, a_TextSize, "allegrojoy%i", (int)a_JoyID);
	}
	
	/* Send Cool Name */
	if (a_Cool && a_CoolSize)
	{
		// Same as above
		snprintf(a_Cool, a_CoolSize, "allegrojoy%i", (int)a_JoyID);
	}
	
	/* Success */
	return true;
}

/* I_GetJoystickCounts() -- Get joystick counts */
bool_t I_GetJoystickCounts(const size_t a_JoyID, uint32_t* const a_NumAxis, uint32_t* const a_NumButtons)
{
	size_t i;
	
	/* Check */
	if (!a_NumAxis && !a_NumButtons)
		return false;
		
	/* No joysticks? */
	if (!num_joysticks || a_JoyID >= (size_t) num_joysticks)
		return false;
		
	/* Return axis */
	if (a_NumAxis)
		for (*a_NumAxis = 0, i = 0; i < joy[a_JoyID].num_sticks; i++)
			*a_NumAxis = joy[a_JoyID].stick[i].num_axis;
			
	/* Return buttons */
	if (a_NumButtons)
		*a_NumButtons = joy[a_JoyID].num_buttons;
		
	/* Success */
	return true;
}

/* I_ProbeMouse() -- Probes Mice */
bool_t I_ProbeMouse(const size_t a_ID)
{
	int i;
	
	/* Only Support a primary mouse */
	if (a_ID != 0)
		return false;
		
	/* Install mouse */
	if ((i = install_mouse()) >= 0)
	{
		CONL_PrintF("I_ProbeMouse: Allegro says there are %i buttons.\n", i);
		
		if (poll_mouse() == -1)
			CONL_PrintF("I_ProbeMouse: First poll is -1!\n");
		return true;
	}
	
	/* Failed */
	return false;
}

/* I_RemoveMouse() -- Removes mice */
bool_t I_RemoveMouse(const size_t a_ID)
{
	/* Only Support a primary mouse */
	if (a_ID != 0)
		return false;
		
	/* Remove mouse handler */
	remove_mouse();
	return true;
}

/* I_MouseGrab() -- Sets mouse grabbing */
void I_MouseGrab(const bool_t a_Grab)
{
	l_DoGrab = a_Grab;
	
	/* Not being grabbed */
	if (!a_Grab)
	{
		// Use hardware cursor
		disable_hardware_cursor();
		//enable_hardware_cursor();
		
		// Show default cursor
		select_mouse_cursor(MOUSE_CURSOR_ARROW);
		
		// Show it
		show_mouse(screen);
		
		// Reposition mouse
		position_mouse(SCREEN_W >> 1, SCREEN_H >> 1);
		l_OldMousePos[0] = SCREEN_W >> 1;
		l_OldMousePos[1] = SCREEN_H >> 1;
	}
	
	/* Being Grabbed */
	else
	{
		// Hide it
		show_mouse(NULL);
		
		// Unshow the cursor
		select_mouse_cursor(NULL);
		
		// Use software cursor
		disable_hardware_cursor();
	}
}

/* I_VideoLockBuffer() -- Locks the video buffer */
void I_VideoLockBuffer(const bool_t a_DoLock)
{
}

