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
#include "command.h"
#include "screen.h"

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

/*************
*** LOCALS ***
*************/

/****************
*** FUNCTIONS ***
****************/

/* VID_NumModes() -- Returns the number of video modes */
int VID_NumModes(void)
{
   return 0;
}

/* VID_GetModeName() -- Gets the name of the video modes */
char* __REMOOD_DEPRECATED VID_GetModeName(int a_ModeNum)
{
	return NULL;
}

/* VID_ClosestMode() -- Returns the closest mode against width and height */
int VID_ClosestMode(int* const a_WidthP, int* const a_HeightP, const boolean a_Fullscreen)
{
	return 0;
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
boolean VID_AddMode(const int a_Width, const int a_Height, const boolean a_Fullscreen)
{
}

/* VID_SetMode() -- Sets the specified video mode */
int VID_SetMode(int a_ModeNum)
{
	return 1;
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
	/* Set direct video buffer */
	vid.rowbytes = a_Pitch;	// Set rowbytes to pitch
	vid.direct = a_Direct;	// Set direct, if it is passed (if not, direct access not supported)
	
	/* Allocate buffer for mode */
	vid.buffer = I_SysAlloc(a_Width * a_Height);
	
	// Oops!
	if (!vid.buffer)
		return;
	
	// Clear buffer
	memset(vid.buffer, 0, a_Width * a_Height);
}

/* I_VideoUnsetBuffer() -- Unsets the video buffer */
void I_VideoUnsetBuffer(void)
{
	/* Clear direct */
	vid.rowbytes = 0;
	vid.direct = NULL;
	
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

