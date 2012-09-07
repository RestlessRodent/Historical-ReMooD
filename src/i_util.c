// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
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
#if !defined(__REMOOD_USECCSTUB)
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
	#include <dpmi.h>
	#endif

	// Include the standard C stuff here
	#include <stdlib.h>

#endif

/* Local */
#include "i_util.h"
#include "i_system.h"
#include "i_video.h"
#include "command.h"
#include "console.h"
#include "screen.h"
#include "g_input.h"
#include "w_wad.h"
#include "doomstat.h"
#include "d_main.h"
#include "m_argv.h"
#include "g_game.h"
#include "d_netcmd.h"
#include "m_menu.h"

/****************
*** CONSTANTS ***
****************/

#define EVENTQUEUESIZE		128	// Max events allowed in queue
#define MODENAMELENGTH		16	// Length of mode name
#define MAX_QUIT_FUNCS		16	// Max number of quit functions

/* c_KeyNames -- Names for keys */
const char* const c_KeyNames[NUMIKEYBOARDKEYS][2] =
{
	// Lower Control keys
	{"null", "Null"},							// IKBK_NULL
	{"key01", "Key 01"},						// 0x01
	{"key02", "Key 02"},						// 0x02
	{"key03", "Key 03"},						// 0x03
	{"key04", "Key 04"},						// 0x04
	{"key05", "Key 05"},						// 0x05
	{"key06", "Key 06"},						// 0x06
	{"key07", "Key 07"},						// 0x07
	{"backspace", "Backspace"},					// IKBK_BACKSPACE = 0x08,
	{"tab", "Tab"},								// IKBK_TAB = 0x09,
	{"return", "Return/Enter"},					// IKBK_RETURN = 0x0A,
	{"key0B", "Key 0B"},						// 0x0B
	{"key0C", "Key 0C"},						// 0x0C
	{"key0D", "Key 0D"},						// 0x0D
	{"shift", "Shift"},							// IKBK_SHIFT = 0x0E,
	{"ctrl", "Control"},						// IKBK_CTRL = 0x0F,
	{"alt", "Alt"},								// IKBK_ALT = 0x10,
	{"up", "Up Arrow"},							// IKBK_UP = 0x11,
	{"down", "Down Arrow"},						// IKBK_DOWN,
	{"left", "Left Arrow"},						// IKBK_LEFT,
	{"right", "Right Arrow"},					// IKBK_RIGHT,
	{"key15", "Key 15"},						// 0x15
	{"key16", "Key 16"},						// 0x16
	{"key17", "Key 17"},						// 0x17
	{"key18", "Key 18"},						// 0x18
	{"key19", "Key 19"},						// 0x19
	{"key1A", "Key 1A"},						// 0x1A
	{"escape", "Escape"},						// IKBK_ESCAPE = 0x1B,
	{"key1C", "Key 1C"},						// 0x1C
	{"key1D", "Key 1D"},						// 0x1D
	{"key1E", "Key 1E"},						// 0x1E
	{"key1F", "Key 1F"},						// 0x1F
	
	// Standard ASCII
	{"space", "Spacebar"},						// IKBK_SPACE = ' ',
	{"exclaim", "Exclaimation Point"},			// IKBK_EXCLAIM,
	{"quote", "Quote"},							// IKBK_QUOTE,
	{"hash", "Hash"},							// IKBK_HASH,
	{"dollar", "Dollar Sign"},					// IKBK_DOLLAR,
	{"percent", "Percent"},						// IKBK_PERCENT,
	{"ampersand", "Ampersand"},					// IKBK_AMPERSAND,
	{"apostrophe", "Apostrophe"},				// IKBK_APOSTROPHE,
	{"leftparenth", "Left Parenthesis"},		// IKBK_LEFTPARENTHESES,
	{"rightparenth", "Right Parenthesis"},		// IKBK_RIGHTPARENTHESES,
	{"asterisk", "Asterisk"},					// IKBK_ASTERISK,
	{"plus", "Plus"},							// IKBK_PLUS,
	{"comma", "Comma"},							// IKBK_COMMA,
	{"minus", "Minus"},							// IKBK_MINUS,
	{"period", "Period"},						// IKBK_PERIOD,
	{"fslash", "Forward Slash"},				// IKBK_FORWARDSLASH,
	{"0", "0"},									// IKBK_0,
	{"1", "1"},									// IKBK_1,
	{"2", "2"},									// IKBK_2,
	{"3", "3"},									// IKBK_3,
	{"4", "4"},									// IKBK_4,
	{"5", "5"},									// IKBK_5,
	{"6", "6"},									// IKBK_6,
	{"7", "7"},									// IKBK_7,
	{"8", "8"},									// IKBK_8,
	{"9", "9"},									// IKBK_9,
	{"colon", "Colon"},							// IKBK_COLON,
	{"semicolon", "Semicolon"},					// IKBK_SEMICOLON,
	{"lessthan", "Less Than"},					// IKBK_LESSTHAN,
	{"equals", "Equals"},						// IKBK_EQUALS,
	{"greaterthan", "Greater Than"},			// IKBK_GREATERTHAN,
	{"questionmark", "Question Mark"},			// IKBK_QUESTION,
	{"at", "At Sign"},							// IKBK_AT,
	{"a", "A"},									// IKBK_A,
	{"b", "B"},									// IKBK_B,
	{"c", "C"},									// IKBK_C,
	{"d", "D"},									// IKBK_D,
	{"e", "E"},									// IKBK_E,
	{"f", "F"},									// IKBK_F,
	{"g", "G"},									// IKBK_G,
	{"h", "H"},									// IKBK_H,
	{"i", "I"},									// IKBK_I,
	{"j", "J"},									// IKBK_J,
	{"k", "K"},									// IKBK_K,
	{"l", "L"},									// IKBK_L,
	{"m", "M"},									// IKBK_M,
	{"n", "N"},									// IKBK_N,
	{"o", "O"},									// IKBK_O,
	{"p", "P"},									// IKBK_P,
	{"q", "Q"},									// IKBK_Q,
	{"r", "R"},									// IKBK_R,
	{"s", "S"},									// IKBK_S,
	{"t", "T"},									// IKBK_T,
	{"u", "U"},									// IKBK_U,
	{"v", "V"},									// IKBK_V,
	{"w", "W"},									// IKBK_W,
	{"x", "X"},									// IKBK_X,
	{"y", "Y"},									// IKBK_Y,
	{"z", "Z"},									// IKBK_Z,
	{"leftbracket", "Left Bracket"},			// IKBK_LEFTBRACKET,
	{"backslash", "Backslash"},					// IKBK_BACKSLASH,
	{"rightbracket", "Right Bracket"},			// IKBK_RIGHTBRACKET,
	{"caret", "Caret"},							// IKBK_CARET,
	{"underscore", "Underscore"},				// IKBK_UNDERSCORE,
	{"grave", "Grave Accent"},					// IKBK_GRAVE,
	
	{"key61", "Key 61"},						// 0x61
	{"key62", "Key 62"},						// 0x62
	{"key63", "Key 63"},						// 0x63
	{"key64", "Key 64"},						// 0x64
	{"key65", "Key 65"},						// 0x65
	{"key66", "Key 66"},						// 0x66
	{"key67", "Key 67"},						// 0x67
	{"key68", "Key 68"},						// 0x68
	{"key69", "Key 69"},						// 0x69
	{"key6A", "Key 6A"},						// 0x6A
	{"key6B", "Key 6B"},						// 0x6B
	{"key6C", "Key 6C"},						// 0x6C
	{"key6D", "Key 6D"},						// 0x6D
	{"key6E", "Key 6E"},						// 0x6E
	{"key6F", "Key 6F"},						// 0x6F
	{"key70", "Key 70"},						// 0x70
	{"key71", "Key 71"},						// 0x71
	{"key72", "Key 72"},						// 0x72
	{"key73", "Key 73"},						// 0x73
	{"key74", "Key 74"},						// 0x74
	{"key75", "Key 75"},						// 0x75
	{"key76", "Key 76"},						// 0x76
	{"key77", "Key 77"},						// 0x77
	{"key78", "Key 78"},						// 0x78
	{"key79", "Key 79"},						// 0x79
	{"key7A", "Key 7A"},						// 0x7A
	
	{"leftbrace", "Left Brace"},				// IKBK_LEFTBRACE = 0x7B,
	{"pipe", "Pipe"},							// IKBK_PIPE,
	{"rightbrace", "Right Brace"},				// IKBK_RIGHTBRACE,
	{"tilde", "Tilde"},							// IKBK_TILDE,
	
	// Cool Keys
	{"delete", "Delete"},						// IKBK_KDELETE = 0x7F,
	{"home", "Home"},							// IKBK_HOME,
	{"end", "End"},								// IKBK_END,
	{"ins", "Insert"},							// IKBK_INSERT,
	{"pgup", "Page Up"},						// IKBK_PAGEUP,
	{"pgdn", "Page Down"},						// IKBK_PAGEDOWN,
	{"print", "Print Screen"},					// IKBK_PRINTSCREEN,
	{"numlock", "Num Lock"},					// IKBK_NUMLOCK,
	{"capslock", "Caps Lock"},					// IKBK_CAPSLOCK,
	{"scrolllock", "Scroll Lock"},				// IKBK_SCROLLLOCK,
	{"pause", "Pause"},							// IKBK_PAUSE,
	{"f1", "F1"},								// IKBK_F1,
	{"f2", "F2"},								// IKBK_F2,
	{"f3", "F3"},								// IKBK_F3,
	{"f4", "F4"},								// IKBK_F4,
	{"f5", "F5"},								// IKBK_F5,
	{"f6", "F6"},								// IKBK_F6,
	{"f7", "F7"},								// IKBK_F7,
	{"f8", "F8"},								// IKBK_F8,
	{"f9", "F9"},								// IKBK_F9,
	{"f10", "F10"},								// IKBK_F10,
	{"f11", "F11"},								// IKBK_F11,
	{"f12", "F12"},								// IKBK_F12,
	{"num0", "NumPad 0"},						// IKBK_NUM0,
	{"num1", "NumPad 1"},						// IKBK_NUM1,
	{"num2", "NumPad 2"},						// IKBK_NUM2,
	{"num3", "NumPad 3"},						// IKBK_NUM3,
	{"num4", "NumPad 4"},						// IKBK_NUM4,
	{"num5", "NumPad 5"},						// IKBK_NUM5,
	{"num6", "NumPad 6"},						// IKBK_NUM6,
	{"num7", "NumPad 7"},						// IKBK_NUM7,
	{"num8", "NumPad 8"},						// IKBK_NUM8,
	{"num9", "NumPad 9"},						// IKBK_NUM9,
	{"numdiv", "NumPad Divide"},				// IKBK_NUMDIVIDE,
	{"nummul", "NumPad Multiply"},				// IKBK_NUMMULTIPLY,
	{"numsub", "NumPad Subtract"},				// IKBK_NUMSUBTRACT,
	{"numadd", "NumPad Addition"},				// IKBK_NUMADD,
	{"nument", "NumPad Enter"},					// IKBK_NUMENTER,
	{"numdec", "NumPad Decimal Point"},			// IKBK_NUMPERIOD,
	{"numdel", "NumPad Delete"},				// IKBK_NUMDELETE,
	
	{"logokey", "Logo Key"},					// IKBK_WINDOWSKEY,
	{"menukey", "Menu Key"},					// IKBK_MENUKEY,
	{"euro", "Euro"},							// IKBK_EVILEURO,
};

/**************
*** GLOBALS ***
**************/

uint8_t graphics_started = 0;
bool_t allow_fullscreen = false;

/* My stuff */
static bool_t l_MouseOK = false;				// OK to use the mouse code?
static bool_t l_NoMouseGrab = false;			// Don't grab mouse

/*****************
*** STRUCTURES ***
*****************/

/* I_VideoMode_t -- A video mode */
typedef struct I_VideoMode_s
{
	uint16_t Width;				// Screen Width
	uint16_t Height;			// Screen height
	char Name[MODENAMELENGTH];	// Mode name
} I_VideoMode_t;

/*************
*** LOCALS ***
*************/

static I_VideoMode_t* l_Modes = NULL;	// Video Modes
static size_t l_NumModes = 0;	// Number of video modes

static I_EventEx_t l_EventQ[EVENTQUEUESIZE];	// Events in queue
static size_t l_EQRead = 0;		// Read position in queue
static size_t l_EQWrite = 0;	// Write position in queue

typedef void (*quitfuncptr) ();
static quitfuncptr quit_funcs[MAX_QUIT_FUNCS];

static bool_t l_JoyOK = false;	// Joysticks OK
static uint8_t l_NumJoys;		// Number of joysticks
static uint8_t l_RealJoyMap[MAXJOYSTICKS];	// Joystick mappings

// i_enablemouse -- Enable mouse input
CONL_StaticVar_t l_IEnableMouse =
{
	CLVT_INTEGER, c_CVPVBoolean, CLVF_SAVE,
	"i_enablemouse", DSTR_CVHINT_IENABLEMOUSE, CLVVT_STRING, "true",
	NULL
};

// i_enablejoystick -- Enable joystick input
CONL_StaticVar_t l_IEnableJoystick =
{
	CLVT_INTEGER, c_CVPVBoolean, CLVF_SAVE,
	"i_enablejoystick", DSTR_CVHINT_IENABLEJOYSTICK, CLVVT_STRING, "true",
	NULL
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
		
	/* Mouse/Joystick event and they are not OK */
	// I know this seems kinda bad, but some interfaces always handle mouse inputs
	// and whatnot, so it should just be ignored here.
	if ((a_Event->Type == IET_MOUSE && !l_MouseOK) || (a_Event->Type == IET_JOYSTICK && !l_JoyOK))
		return;
		
	/* Write at current write pos */
	l_EventQ[l_EQWrite++] = *a_Event;
	
	// Overlap?
	if (l_EQWrite >= EVENTQUEUESIZE)
		l_EQWrite = 0;
		
	// Got too many events in Q?
	if (l_EQWrite == l_EQRead)
	{
		// Indicate overflow
		if (devparm)
			CONL_PrintF("I_EventExPush: Event overflow (%i)!\n", (int)l_EQWrite);
			
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
		return false;			// Nothing!
		
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
	static int DevEvents;
	
	/* Check for event debugging */
	if (!DevEvents)
		DevEvents = (!!M_CheckParm("-devevent")) + 1;
		
	/* Update things */
	// Joysticks need updating before being able to be used
	I_UpdateJoysticks();
	
	/* Just read all events */
	I_GetEvent();
	
	/* Translate events to old Doom events */
	while (I_EventExPop(&Event))
	{
		// Debug
		if (DevEvents == 2)
		{
			// Which type of event?
			switch (Event.Type)
			{
					// Quit
				case IET_QUIT:
					CONL_PrintF("Event: QUIT!!\n");
					break;
					
					// Keyboard
				case IET_KEYBOARD:
					CONL_PrintF("Event: KEYBRD St:%2s Rp:%2s Kc:%03X Ch:%c\n",
					            (Event.Data.Keyboard.Down ? "Dn" : "Up"),
					            (Event.Data.Keyboard.Repeat ? "Re" : "--"),
					            Event.Data.Keyboard.KeyCode, (Event.Data.Keyboard.Character & 0x7F ? Event.Data.Keyboard.Character & 0x7F : ' '));
					break;
					
					// Mouse
				case IET_MOUSE:
					CONL_PrintF("Event: MOUSE_ Id:%2i Ps:(%5i, %5i)", Event.Data.Mouse.MouseID, Event.Data.Mouse.Pos[0], Event.Data.Mouse.Pos[1]);
					
					// Button
					if (Event.Data.Mouse.Button)
						CONL_PrintF(" St:%2s Bt:%2i\n", (Event.Data.Mouse.Down ? "Dn" : "Up"), Event.Data.Mouse.Button);
						
					// Movement
					else
						CONL_PrintF(" Mv:(%+4i, %+4i)\n", Event.Data.Mouse.Move[0], Event.Data.Mouse.Move[1]);
					break;
					
					// Joystick
				case IET_JOYSTICK:
					CONL_PrintF("Event: JOYSTK Id:%2i", Event.Data.Joystick.JoyID);
					
					// Button
					if (Event.Data.Joystick.Button)
						CONL_PrintF(" St:%2s Bt:%2i\n", (Event.Data.Joystick.Down ? "Dn" : "Up"), Event.Data.Joystick.Button);
						
					// Axis
					else
						CONL_PrintF(" Ax:%2i Vl:%+6i\n", Event.Data.Joystick.Axis, Event.Data.Joystick.Value);
					break;
					
					// Unknown
				default:
					CONL_PrintF("Event: UNKNWN\n");
					break;
			}
		}
		
		// Quit event?
		if (Event.Type == IET_QUIT)
		{
			continue;
		}
		
		// Translate
		if (!D_JoySpecialEvent(&Event))
			if (!CONL_HandleEvent(&Event))
				if (!M_ExUIHandleEvent(&Event))
					if (!D_NCSHandleEvent(&Event))
						I_EventToOldDoom(&Event);
	}
}

/* IS_NewKeyToOldKey() -- Converts a new key to an old key */
static int IS_NewKeyToOldKey(const uint8_t a_New)
{
	/* Giant Switch */
	switch (a_New)
	{
		case IKBK_NULL:
			return 0;
		case IKBK_BACKSPACE:
			return KEY_BACKSPACE;
		case IKBK_TAB:
			return KEY_TAB;
		case IKBK_RETURN:
			return KEY_ENTER;
		case IKBK_SHIFT:
			return KEY_SHIFT;
		case IKBK_CTRL:
			return KEY_CTRL;
		case IKBK_ALT:
			return KEY_ALT;
		case IKBK_ESCAPE:
			return KEY_ESCAPE;
		case IKBK_UP:
			return KEY_UPARROW;
		case IKBK_DOWN:
			return KEY_DOWNARROW;
		case IKBK_LEFT:
			return KEY_LEFTARROW;
		case IKBK_RIGHT:
			return KEY_RIGHTARROW;
		case IKBK_KDELETE:
			return KEY_DEL;
		case IKBK_HOME:
			return KEY_HOME;
		case IKBK_END:
			return KEY_END;
		case IKBK_INSERT:
			return KEY_INS;
		case IKBK_PAGEUP:
			return KEY_PGUP;
		case IKBK_PAGEDOWN:
			return KEY_PGDN;
			//case IKBK_PRINTSCREEN:    return KEY_;
		case IKBK_NUMLOCK:
			return KEY_NUMLOCK;
		case IKBK_CAPSLOCK:
			return KEY_CAPSLOCK;
		case IKBK_SCROLLLOCK:
			return KEY_SCROLLLOCK;
		case IKBK_PAUSE:
			return KEY_PAUSE;
		case IKBK_NUM0:
			return KEY_KEYPAD0;
		case IKBK_NUM1:
			return KEY_KEYPAD1;
		case IKBK_NUM2:
			return KEY_KEYPAD2;
		case IKBK_NUM3:
			return KEY_KEYPAD3;
		case IKBK_NUM4:
			return KEY_KEYPAD4;
		case IKBK_NUM5:
			return KEY_KEYPAD5;
		case IKBK_NUM6:
			return KEY_KEYPAD6;
		case IKBK_NUM7:
			return KEY_KEYPAD7;
		case IKBK_NUM8:
			return KEY_KEYPAD8;
		case IKBK_NUM9:
			return KEY_KEYPAD9;
		case IKBK_NUMDIVIDE:
			return KEY_KPADSLASH;
		case IKBK_NUMMULTIPLY:
			return '*';
		case IKBK_NUMSUBTRACT:
			return KEY_MINUSPAD;
		case IKBK_NUMADD:
			return KEY_PLUSPAD;
		case IKBK_NUMENTER:
			return KEY_ENTER;
		case IKBK_NUMPERIOD:
			return '.';
		case IKBK_NUMDELETE:
			return KEY_KPADDEL;
		case IKBK_WINDOWSKEY:
			return KEY_LEFTWIN;
		case IKBK_MENUKEY:
			return KEY_MENU;
			
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
}

/* I_DoMouseGrabbing() -- Does grabbing if the mouse should be grabbed */
void I_DoMouseGrabbing(void)
{
	static bool_t Grabbed = false;
	bool_t New = false;
	
	/* If the mouse is not OK, don't touch grabbing */
	if (!l_MouseOK)
		return;
		
	/* Don't grab if... */
	// Dedicated Server, Watching demo, not playing, in a menu, in the console
	New = !(dedicated || demoplayback || M_ExUIActive() || CONL_IsActive());
	
	if (New != Grabbed && !l_NoMouseGrab)
	{
		// Change grab
		I_MouseGrab(New);
		
		// Set grabbed to new
		Grabbed = New;
	}
}

/* I_StartupMouse() -- Initializes the mouse */
void I_StartupMouse(void)
{
	static bool_t VarRegged;
	
	/* Register var? */
	if (!VarRegged)
	{
		CONL_VarRegister(&l_IEnableMouse);
		VarRegged = true;
	}
	
	/* Check parameter */
	// Disable Mouse?
	if (M_CheckParm("-nomouse"))
	{
		CONL_PrintF("I_StartupMouse: -nomouse is preventing mice from being used.\n");
		return;
	}
	
	// Disable Grabbing?
	if (M_CheckParm("-nomousegrab"))
		l_NoMouseGrab = true;
	
	/* Enabling the mouse */
	if (l_IEnableMouse.Value->Int)
	{
		// Enabled?
		if (l_MouseOK)
			return;
			
		// Probe the mouse
		if (!I_ProbeMouse(0))
		{
			CONL_PrintF("I_StartupMouse: There is no mouse\n");
			return;
		}
		
		// Enable
		CONL_PrintF("I_StartupMouse: Mouse enabled.\n");
		l_MouseOK = true;
	}
	
	/* Disabling the mouse */
	else
	{
		// Not enabled?
		if (!l_MouseOK)
			return;
			
		// Ungrab the mouse before removal
		I_MouseGrab(false);
		
		// Remove the mouse
		I_RemoveMouse(0);
		
		// Disable
		CONL_PrintF("I_StartupMouse: Mouse disabled.\n");
		l_MouseOK = false;
	}
}

/* I_StartupMouse2() -- Initializes the second mouse */
void I_StartupMouse2(void)
{
	/* Check parameter */
	if (M_CheckParm("-nomouse"))
	{
		CONL_PrintF("I_StartupMouse: -nomouse is preventing mice from being used.\n");
		return;
	}
	
	/* Enabling the mouse */
	if (l_IEnableMouse.Value->Int)
	{
		// Probe the mouse
		if (!I_ProbeMouse(1))
		{
			CONL_PrintF("I_StartupMouse: There is no secondary mouse\n");
			return;
		}
	}
	
	/* Disabling the mouse */
	else
	{
		// Remove the mouse
		I_RemoveMouse(1);
	}
}

/* I_UpdateJoysticks() -- Updates joysticks */
void I_UpdateJoysticks(void)
{
}

/* I_InitJoystick() -- Initialize the joystick */
void I_InitJoystick(void)
{
#define BUFSIZE 256
	char Buf[BUFSIZE];
	char CoolBuf[BUFSIZE];
	uint32_t ID, NumAxis, NumButtons;
	size_t i;
	static bool_t VarRegged;
	
	/* Register var? */
	if (!VarRegged)
	{
		CONL_VarRegister(&l_IEnableJoystick);
		VarRegged = true;
	}
	
	/* Check parameter */
	if (M_CheckParm("-nojoy"))
	{
		CONL_PrintF("I_InitJoystick: -nojoy is preventing joysticks from being used.\n");
		return;
	}
	
	/* Enabling the joystick */
	if (l_IEnableJoystick.Value->Int)
	{
		// Enabled?
		if (l_JoyOK)
			return;
			
		// Probe joysticks
		l_NumJoys = I_ProbeJoysticks();
		
		if (!l_NumJoys)
		{
			CONL_PrintF("I_InitJoystick: There are no joysticks.\n");
			return;
		}
		// Print number of sticks
		CONL_PrintF("I_InitJoystick: There are %i joystick(s).\n", (int)l_NumJoys);
		
		// Map base joysticks
		for (i = 0; i < MAXJOYSTICKS; i++)
			l_RealJoyMap[i] = i;
			
		// Print name of joysticks
		for (i = 0; i < l_NumJoys; i++)
		{
			if (I_GetJoystickID(i, &ID, Buf, BUFSIZE, CoolBuf, BUFSIZE))
				CONL_PrintF("I_InitJoystick: Joystick ID %08x is called \"%s\" (Full name \"%s\").\n", ID, CoolBuf, Buf);
			if (I_GetJoystickCounts(i, &NumAxis, &NumButtons))
				CONL_PrintF("I_InitJoystick: Has %u axis and %u buttons.\n", NumAxis, NumButtons);
		}
		
		// Enable
		l_JoyOK = true;
	}
	
	/* Disabling the joystick */
	else
	{
		// Not enabled?
		if (!l_JoyOK)
			return;
			
		// Destroy joysticks
		I_RemoveJoysticks();
		
		// Disable
		l_JoyOK = false;
	}
#undef BUFSIZE
}

/* I_NumJoysticks() -- Returns Joystick Count */
uint8_t I_NumJoysticks(void)
{
	return l_NumJoys;
}

/* I_BaseTiccmd() -- Obtain driver based tic command */
ticcmd_t* I_BaseTiccmd(void)
{
	static ticcmd_t emptycmd;
	
	return &emptycmd;
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
				break;			// it is here!
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
			
	/* Resize mode list and set */
	l_Modes = I_SysRealloc(l_Modes, sizeof(*l_Modes) * (l_NumModes + 1));
	
	// Set
	snprintf(l_Modes[l_NumModes].Name, MODENAMELENGTH, "%ix%i", a_Width, a_Height);
	l_Modes[l_NumModes].Width = a_Width;
	l_Modes[l_NumModes++].Height = a_Height;
	
	/* Success! */
	return true;
}

extern CONL_StaticVar_t l_SCRFullScreen;

/* VID_SetMode() -- Sets the specified video mode */
// Funny thing is, despite returning an int, Legacy never checked if it worked!
int VID_SetMode(int a_ModeNum)
{
	/* Check */
	if (a_ModeNum < 0 || a_ModeNum >= l_NumModes)
		return 0;				// Failure despite not being checked!
		
	/* Try to set the mode */
	if (!I_SetVideoMode(l_Modes[a_ModeNum].Width, l_Modes[a_ModeNum].Height, l_SCRFullScreen.Value[0].Int))
		return false;
	return true;
}

/* I_UtilWinArgToUNIXArg() -- Converts Windows-style command line to a UNIX one */
bool_t I_UtilWinArgToUNIXArg(int* const a_argc, char** *const a_argv, const char* const a_Win)
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
	vid.rowbytes = a_Pitch;		// Set rowbytes to pitch
	vid.direct = a_Direct;		// Set direct, if it is passed (if not, direct access not supported)
	vid.width = a_Width;
	vid.height = a_Height;
	vid.modenum = VID_ClosestMode(&w, &h, true);
	
	/* Allocate buffer for mode */
	vid.buffer = I_SysAlloc(a_Width * a_Height * NUMSCREENS);
	
	// Oops!
	if (!vid.buffer)
		return;
		
	// Clear buffer
	memset(vid.buffer, 0, a_Width * a_Height * NUMSCREENS);
	
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

/* I_BeginRead() -- Before a file is read */
// Flashes floppy disk (or CD)
void I_BeginRead(void)
{
}

/* I_EndRead() -- When a file is no longer being read */
// Flashes floppy disk (or CD)
void I_EndRead(void)
{
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
	         "rmXXXXXX"			// On DJGPP with DOS, don't place in /tmp/ because that will always fail!
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
	if ((p = I_GetEnvironment("COLUMNS")))
		Cols = atoi(p);
	else
		Cols = 79;
		
	/* Load ENDTXT */
	for (i = 0; i < 4000; i += 2)
	{
		// Get logical column number
		c = (i >> 1) % 80;
		
		// Add a newline
		if (c == 0 && Cols != 80)
			I_TextModeNextLine();
		
		// Print character
		if (c < Cols)			// but only if it fits!
			I_TextModeChar(a_TextData[i], a_TextData[i + 1]);
	}
	
	/* Leave text mode */
	I_TextModeNextLine();
	I_TextMode(false);
}

/* I_TextModeNextLine() -- Prints a new line */
void I_TextModeNextLine(void)
{
	/* Print character */
#if defined(__MSDOS__)
	cprintf
#else
	printf
#endif
	("\n");
}

/* I_TextModeChar() -- Prints a text mode character */
void I_TextModeChar(const uint8_t a_Char, const uint8_t Attr)
{
	uint8_t BG, FG;
	bool_t Blink;
	
#if defined(_WIN32)
	HANDLE* StdOut;
	
	static const uint32_t c_BGColorToWin32[8] =
	{
		0,
		BACKGROUND_BLUE,
		BACKGROUND_GREEN,
		BACKGROUND_BLUE | BACKGROUND_GREEN,
		BACKGROUND_RED,
		BACKGROUND_RED | BACKGROUND_BLUE,
		BACKGROUND_RED | BACKGROUND_GREEN,
		BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_BLUE,
	};
	static const uint32_t c_FGColorToWin32[8] =
	{
		0,
		FOREGROUND_BLUE,
		FOREGROUND_GREEN,
		FOREGROUND_BLUE | FOREGROUND_GREEN,
		FOREGROUND_RED,
		FOREGROUND_RED | FOREGROUND_BLUE,
		FOREGROUND_RED | FOREGROUND_GREEN,
		FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
	};
#endif
	
	static const char c_ColorToVT[8] =
	{
		0, 4, 2, 6, 1, 5, 3, 7
	};							// Colors to VT
	static const uint8_t c_ASCIIMap[256] =
	{
		//  0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
		' ', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', 'M', 'F', '?', '?', '#',	// 0
		'>', '<', '|', '!', 'P', 'S', '=', '|', '^', 'v', '<', '>', 'L', '=', '^', 'v',	// 1
		' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',	// 2
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',	// 3
		'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',	// 4
		'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',	// 5
		'`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',	// 6
		'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', '^',	// 7
		'C', 'u', 'e', 'a', 'a', 'a', 'a', 'c', 'e', 'e', 'e', 'i', 'i', 'i', 'A', 'A',	// 8
		'E', '@', '&', 'o', 'o', 'o', 'u', 'u', 'y', 'O', 'U', 'c', 'E', 'Y', 'P', 'f',	// 9
		'a', 'i', 'o', 'u', 'n', 'N', '*', 'o', '?', '/', '\\', '/', '/', '!', '<', '>',	// A
		'-', '=', '#', '|', '|', '|', '|', '+', '+', '|', '|', '+', '+', '+', '+', '+',	// B
		'+', '+', '+', '+', '-', '+', '|', '|', '+', '+', '-', '-', '|', '-', '+', '-',	// C
		'-', '-', '-', '+', '+', '+', '+', '+', '+', '+', '+', '#', '#', '#', '#', '#',	// D
		'a', 'B', 'r', 'n', 'E', 'o', 'u', 't', 'o', 'h', 'h', 'o', '8', 'o', 'q', 'n',	// E
		'=', '+', '>', '<', 'f', 'j', '/', '=', 'o', '.', '.', '/', 'n', '2', '.', ' ',	// F
	};
	
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
	// GetConsoleScreenBufferInfo
	StdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(StdOut, c_BGColorToWin32[BG] | c_FGColorToWin32[FG & 7] | ((FG & 8) ? FOREGROUND_INTENSITY : 0));
	
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
#if defined(__MSDOS__)
	cprintf
#else
	printf
#endif
	("%c", c_ASCIIMap[a_Char]);
	
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
	uint8_t* EDData[2] = { 0, 0 };
	
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
		Temp = WX_CacheEntry(Entry);
		Size = WX_GetEntrySize(Entry);
		
		// Duplicate
		EDData[c] = I_SysAlloc(4000);
		
		if (EDData[c])
		{
			memset(EDData[c], 0, 4000);
			memmove(EDData[c], Temp, (Size <= 4000 ? Size : 4000));
		}
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

/* I_mkdir() -- Creates a new directory */
int I_mkdir(const char* a_Path, int a_UNIXPowers)
{
#if defined(__REMOOD_SYSTEM_WINDOWS) || defined(__REMOOD_USECCSTUB)
	return mkdir(a_Path);
	
#else
	// Ignore UNIX Powers
	return mkdir(a_Path, S_IRUSR | S_IWUSR | S_IXUSR);
#endif
}

/* I_GetUserName() -- Returns the current username */
const char* I_GetUserName(void)
{
#define MAXUSERNAME 32			// Username limit (youch)
	static char RememberName[MAXUSERNAME];
	const char* p;
	
#if defined(_WIN32)
	TCHAR Buf[MAXUSERNAME];
	DWORD Size;
	size_t i;
#endif
	
	/* Clear Buffer */
	memset(RememberName, 0, sizeof(RememberName));
	
	/* Try system specific username getting */
	// Under UNIX, use getlogin
#if defined(__unix__)
	// prefer getlogin_r if it exists
#if _REENTRANT || _POSIX_C_SOURCE >= 199506L
	if (getlogin_r(RememberName, MAXUSERNAME - 1))
		return RememberName;
		
	// Otherwise use getlogin
#else
	p = getlogin();
	
	if (p)
	{
		// Dupe string
		strncpy(RememberName, p, MAXUSERNAME - 1);
		return RememberName;
	}
#endif
	
	// Under Win32, use GetUserName
#elif defined(_WIN32)
	Size = MAXUSERNAME - 1;
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
	p = I_GetEnvironment("USER");
	
	// Nope
	if (!p)
	{
		p = I_GetEnvironment("USERNAME");
		
		// Nope
		if (!p)
		{
			p = I_GetEnvironment("LOGNAME");
			
			// Nope
			if (!p)
				return NULL;
		}
	}
	
	// Copy p to buffer and return the buffer
	strncpy(RememberName, p, MAXUSERNAME - 1);
	return RememberName;
#undef MAXUSERNAME
}

/* I_GetStorageDir() -- Get location to store local data */
size_t I_GetStorageDir(char* const a_Out, const size_t a_Size, const I_DataStorageType_t a_Type)
{
	char* Env;
	
	/* Check */
	if (!a_Out || !a_Size || a_Type < 0 || a_Type >= NUMDATASTORAGETYPES)
		return 0;
	
	/* Clear */
	memset(a_Out, 0, sizeof(*a_Out) * a_Size);
	
	/* Windows Systems */
#if defined(_WIN32)
	
	/* DOS */
#elif defined(__MSDOS__)
	// Use C:\DOOMDATA and make sure it exists
	strncat(a_Out, "c:/doomdata", a_Size);	
	I_mkdir(a_Out, 0);
	
	/* UNIX Systems */
#elif defined(__unix__)
	// Get XDG Config directory
	if ((Env = I_GetEnvironment((a_Type == DST_CONFIG ? "XDG_CONFIG_HOME" : "XDG_DATA_HOME"))))
	{
		// Use the following then
		strncat(a_Out, Env, a_Size);
		
		// Create directory just in case
		I_mkdir(a_Out, 0);
		
		// And concat remood
		strncat(a_Out, "/", a_Size);
		strncat(a_Out, "remood", a_Size);
		
		// Create the directory
		I_mkdir(a_Out, 0);
	}
	
	// Otherwise, get the home directory
	else if ((Env = I_GetEnvironment("HOME")))
	{
		// Use the following then
		strncat(a_Out, Env, a_Size);
		
		// Add slash
		strncat(a_Out, "/", a_Size);
		
		// Add .config or .local and create the dir (just in case)
		strncat(a_Out, ((a_Type == DST_CONFIG) ? ".config" : ".local"), a_Size);
		I_mkdir(a_Out, 0);
		
		// If this is data, add share also (and create the directory)
		if (a_Type == DST_DATA)
		{
			strncat(a_Out, "/share", a_Size);
			I_mkdir(a_Out, 0);
		}
		
		// Add remood directory and create it
		strncat(a_Out, "/remood", a_Size);
		I_mkdir(a_Out, 0);
	}
	
	/* Other */
#else
	
	/* */
#endif

	/* Directory is empty? */
	// Then fallback to current directory
	if (strlen(a_Out) == 0)
		strncat(a_Out, ".", a_Size);

	/* Return size of set buffer */
	return strlen(a_Out);
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

/* I_GetFreeMemory() -- Returns the amount of free memory available */
uint64_t I_GetFreeMemory(uint64_t* const a_TotalMem)
{
#define DEFAULTMEMCOUNT 32

	/* MS-DOS w/ DJGPP */
#if defined(__MSDOS__)
	__dpmi_memory_info MInfo;
	__dpmi_version_ret DPMIVer;
	__dpmi_free_mem_info FreeMemFo;
	unsigned long PageSize;
	
	// Get the current DPMI version
	__dpmi_get_version(&DPMIVer);
	
	// Only DPMI 1.0 supports __dpmi_get_memory_information()
	// and not CWSDPMI and Win32's internal DPMI
	if (DPMIVer.major >= 1)
		if (__dpmi_get_memory_information(&MInfo) >= 0)
		{
			// Set total
			if (a_TotalMem)
				*a_TotalMem = MInfo.total_allocated_bytes_of_virtual_memory_host;
			
			// Return free memory
			return MInfo.total_available_bytes_of_virtual_memory_host;
		}
	
	// Either DPMI < 1.0 or __dpmi_get_memory_information() failed.
	__dpmi_get_free_memory_information(&FreeMemFo);
	
	// Will also need the page size (since that info is all in pages)
	if (__dpmi_get_page_size(&PageSize) < 0)
		PageSize = 0;	// Just use 0 here (-1 == 16-bit host)
	
	// Set total memory
	if (a_TotalMem)
		*a_TotalMem = FreeMemFo.linear_address_space_size_in_pages * PageSize;
	
	// Return free memory
	return FreeMemFo.free_linear_address_space_in_pages * PageSize;
	
	/* Windows 64-bit */
#elif defined(_WIN64)
	MEMORYSTATUSEX MemStatEx;

	/* Obtain status info */
	memset(&MemStatEx, 0, sizeof(MemStatEx));
	MemStatEx.dwLength = sizeof(MemStatEx);
	
	// Try getting it
	if (GlobalMemoryStatusEx(&MemStatEx) == 0)
		return DEFAULTMEMCOUNT;

	// Return figures
	if (a_TotalMem)
		*a_TotalMem = MemStatEx.ullTotalPhys + (MemStatEx.ullTotalPageFile / 4);

	// Return free memory
	return MemStatEx.ullAvailPhys + (MemStatEx.ullAvailPageFile / 4);
	
	/* Windows 32-bit */
#elif defined(_WIN32)
	MEMORYSTATUS MemStat;

	/* Obtain status info */
	memset(&MemStat, 0, sizeof(MemStat));
	MemStat.dwLength = sizeof(MemStat);

	// Get Info
	GlobalMemoryStatus(&MemStat);

	// Return total
	if (a_TotalMem)
		*a_TotalMem = MemStat.dwTotalPhys + (MemStat.dwTotalPageFile / 4);

	// Return free memory
	return MemStat.dwAvailPhys + (MemStat.dwAvailPageFile / 4);

	/* Linux */
#elif defined(__linux__)
	// TODO: This could probably be improved, but it is only called in few places
#define BUFSIZE 256
	FILE* ProcMem;
	char Buf[BUFSIZE];
	char* p;
	
	uint64_t Total, Free, Temp;
	
	// Clear
	Total = Free = 0;
	
	// Read /proc/meminfo and obtain memory info
	ProcMem = fopen("/proc/meminfo", "rb");
	
	// Check
	if (!ProcMem)
	{
		if (a_TotalMem)
			*a_TotalMem = DEFAULTMEMCOUNT << 20;
		return DEFAULTMEMCOUNT << 20;
	}
	
	// Read in lines
	while (!feof(ProcMem))
	{
		// Read line from file
		fgets(Buf, BUFSIZE, ProcMem);
		
		// See if it is something we like
		if (strncasecmp(Buf, "MemTotal", 8) == 0)
			Total = strtol(Buf + 8 + 2, &p, 10) << 10;
		else if (strncasecmp(Buf, "MemFree", 7) == 0)
			Free += strtol(Buf + 7 + 2, &p, 10) << 10;
		
		// Buffers and Cache count at 75%
		else if (strncasecmp(Buf, "Buffers", 7) == 0)
		{
			Temp = strtol(Buf + 7 + 2, &p, 10) << 10;
			Free += Temp - (Temp >> 2);
		}
		else if (strncasecmp(Buf, "Cached", 6) == 0)
		{
			Temp = strtol(Buf + 6 + 2, &p, 10) << 10;
			Free += Temp - (Temp >> 2);
		}
		
		// Swap counts at 50%
		else if (strncasecmp(Buf, "SwapTotal", 9) == 0)
		{
			Temp = strtol(Buf + 9 + 2, &p, 10) << 10;
			Total += Temp - (Temp >> 1);
		}
		else if (strncasecmp(Buf, "SwapFree", 8) == 0)
		{
			Temp = strtol(Buf + 8 + 2, &p, 10) << 10;
			Free += Temp - (Temp >> 1);
		}
	}
	
	// Close
	fclose(ProcMem);
	
	// Set total
	if (a_TotalMem)
		*a_TotalMem = Total;

	// Return free
	return Free;
#undef BUFSIZE

	/* UNIX */
#elif defined(__unix__)
	uint64_t Total, Free;
	
	// Get total memory (RAM)
	Total = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE);
	
	// Return total
	if (a_TotalMem)
		*a_TotalMem = Total;
	
	// Just use free as total (ouch)
	return Total;

	/* Unknown */
#else
	// Return 32MiB
	return DEFAULTMEMCOUNT << 20;

	/* */
#endif

#undef DEFAULTMEMCOUNT
}

/* I_CommonCommandLine() -- Common command line stuff */
void I_CommonCommandLine(int* const a_argc, char** *const a_argv, const char* const a_Long)
{
	/* argc and argv */
	if (a_argc && a_argv)
	{
		myargc = *a_argc;
		myargv = *a_argv;
	}
	
	/* Windows command line (long format) */
	else if (a_Long)
	{
	}
}

/* I_Quit() -- Quits the game */
void I_Quit(void)
{
	static bool_t quiting;
	
	/* prevent recursive I_Quit() */
	if (quiting)
		return;
	quiting = 1;
	//added:16-02-98: when recording a demo, should exit using 'q' key,
	//        but sometimes we forget and use 'F10'.. so save here too.
	if (demorecording)
	{
		G_CheckDemoStatus();
		
		// Force stop record, even if demo was checked already!
		G_StopDemoRecord();
	}
	
	// Disconnect from network game
	D_NCDisconnect(false);
	D_QuitNetGame();
	
	// use this for 1.28 19990220 by Kin
	I_ShutdownSystem();
	exit(0);
}


/* I_GetEnvironment() -- Gets an environment variable */
char* I_GetEnvironment(const char* const a_VarName)
{
	/* Windows CE lacks an environment */
#if defined(_WIN32_WCE)
	return NULL;
	
	/* Use standard I_GetEnvironment() */
#else
	return getenv(a_VarName);
#endif
}

/* I_CheckFileAccess() -- Checks file read/write */
bool_t I_CheckFileAccess(const char* const a_Path, const bool_t a_Write)
{
	/* On WinCE, always return true */
#if defined(_WIN32_WCE)
#define BUFSIZE 512
	TCHAR TCDir[BUFSIZE];
	int i;
	DWORD Attribs;
	
	// Copy name slowly
	for (i = 0; a_Path[i] && i < BUFSIZE - 2; i++)
		TCDir[i] = a_Path[i];
	TCDir[i] = 0;
	TCDir[BUFSIZE - 1] = 0;
	
	// Try getting the file attributes
	Attribs = GetFileAttributes(TCDir);
	
	// Does not exist?
	if (Attribs == 0xFFFFFFFFU)
		return false;
	
	// Asked for write but is read only?
	if (a_Write && (Attribs & FILE_ATTRIBUTE_READONLY))
		return false;
	
	// Otherwise success
	return true;
	
	/* Use access() */
#else
	int Modes, Ret;
	
	Modes = R_OK | (a_Write ? W_OK : 0);
	return !access(a_Path, Modes);	// zero means OK
#endif
}

/* I_GetCurrentPID() -- Returns the current PID number */
uint16_t I_GetCurrentPID(void)
{
	/* Win32 */
#if defined(_WIN32)
	return (uint16_t)GetCurrentProcessId();
	
	/* UNIX */
#elif defined(__unix__)
	return getpid();
#endif
}

