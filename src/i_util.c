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
		case IKBK_ESCAPE:	return KEY_ESCAPE;
		case IKBK_ENTER:	return KEY_ENTER;
		case IKBK_UP:		return KEY_UPARROW;
		case IKBK_DOWN:		return KEY_DOWNARROW;
		case IKBK_LEFT:		return KEY_LEFTARROW;
		case IKBK_RIGHT:	return KEY_RIGHTARROW;
		case IKBK_SHIFT:	return KEY_SHIFT;
		case IKBK_CTRL:		return KEY_CTRL;
		case IKBK_ALT:		return KEY_ALT;
		case IKBK_SPACE:	return KEY_SPACE;
		
			// Ranges
		default:
			// Letters
			if (a_New >= IKBK_A && a_New <= IKBK_Z)
				return 'a' + (a_New - IKBK_A);
			
			// Numbers
			else if (a_New >= IKBK_0 && a_New <= IKBK_9)
				return '0' + (a_New - IKBK_0);
			
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

