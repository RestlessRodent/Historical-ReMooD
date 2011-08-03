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

#include "i_util.h"
#include "i_joy.h"
#include "i_system.h"
#include "command.h"
#include "screen.h"
#include "g_input.h"

/****************
*** CONSTANTS ***
****************/

#define EVENTQUEUESIZE		64					// Max events allowed in queue
#define MODENAMELENGTH		16					// Length of mode name

/**************
*** GLOBALS ***
**************/

JoyType_t Joystick;

/* i_video.c -- Remove this garbage */
consvar_t cv_vidwait = {"vid_wait","1",CV_SAVE,CV_OnOff};
byte graphics_started = 0;
boolean allow_fullscreen = false;

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

/****************
*** FUNCTIONS ***
****************/

/* I_EventExPush() -- Pushes an event to the queue */
void I_EventExPush(const I_EventEx_t* const a_Event)
{
	/* Check */
	if (!a_Event)
		return 0;
	
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
boolean I_EventExPop(I_EventEx_t* const a_Event)
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
		
			// Ranges
		default:
			if (a_New >= IKBK_A && a_New <= IKBK_Z)
				return 'a' + (a_New - IKBK_A);
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
int VID_ClosestMode(int* const a_WidthP, int* const a_HeightP, const boolean a_Fullscreen)
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
boolean VID_AddMode(const int a_Width, const int a_Height, const boolean a_Fullscreen)
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
boolean I_UtilWinArgToUNIXArg(int* const a_argc, char*** const a_argv, const char* const a_Win)
{
	/* Check */
	if (!a_argc || !a_argv || !a_Win)
		return false;
	
	return true;
}

/* I_VideoPreInit() -- Common nitialization before everything */
boolean I_VideoPreInit(void)
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
boolean I_VideoBefore320200Init(void)
{
	return true;
}

/* I_VideoPostInit() -- Initialization before end of function */
boolean I_VideoPostInit(void)
{
	graphics_started = 1;
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

