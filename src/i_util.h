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

#ifndef __I_UTIL_H__
#define __I_UTIL_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"

/****************
*** CONSTANTS ***
****************/

/* I_KeyBoardKey_t -- A keyboard key */
typedef enum I_KeyBoardKey_e
{
	// Specials
	IKBK_NULL,
	IKBK_ESCAPE,
	IKBK_ENTER,
	IKBK_UP,
	IKBK_DOWN,
	IKBK_LEFT,
	IKBK_RIGHT,
	IKBK_CTRL,
	IKBK_SHIFT,
	IKBK_ALT,
	IKBK_SPACE,
	
	// Function keys
	IKBK_F1,
	IKBK_F2,
	IKBK_F3,
	IKBK_F4,
	IKBK_F5,
	IKBK_F6,
	IKBK_F7,
	IKBK_F8,
	IKBK_F9,
	IKBK_F10,
	IKBK_F11,
	IKBK_F12,
	
	// Letters
	IKBK_A = 'A',
	IKBK_B,
	IKBK_C,
	IKBK_D,
	IKBK_E,
	IKBK_F,
	IKBK_G,
	IKBK_H,
	IKBK_I,
	IKBK_J,
	IKBK_K,
	IKBK_L,
	IKBK_M,
	IKBK_N,
	IKBK_O,
	IKBK_P,
	IKBK_Q,
	IKBK_R,
	IKBK_S,
	IKBK_T,
	IKBK_U,
	IKBK_V,
	IKBK_W,
	IKBK_X,
	IKBK_Y,
	IKBK_Z,
	
	// Numbers
	IKBK_0 = '0',
	IKBK_1,
	IKBK_2,
	IKBK_3,
	IKBK_4,
	IKBK_5,
	IKBK_6,
	IKBK_7,
	IKBK_8,
	IKBK_9,
} I_KeyBoardKey_t;

/* I_EventType_t -- Event type */
typedef enum I_EventType_e
{
	IET_NULL,									// Blank event
	IET_KEYBOARD,								// Keyboard event
	
	NUMIEVENTTYPES
} I_EventType_t;

/* I_MusicType_t -- Types of music to play */
typedef enum I_MusicType_e
{
	IMT_MUS,									// Doom MUS
	IMT_MIDI,									// MIDI File
	IMT_MOD,									// MOD Tracker	
	
	IMT_UNKNOWN,								// Not in any known format!
	
	NUMIMUSICTYPES
} I_MusicType_t;

#define MAXDRIVERNAME						32	// Driver name limit

/*****************
*** STRUCTURES ***
*****************/

/* I_EventEx_t -- Extended event */
typedef struct I_EventEx_s
{
	I_EventType_t Type;							// Type of event
	
	union
	{
		struct
		{
			bool_t Down;						// Key pressed down
			bool_t Repeat;						// Is the key repeated?
			uint8_t KeyCode;					// Code for key
			uint16_t Character;					// Character pressed
		} Keyboard;								// Keyboard event
	} Data;										// Event data
} I_EventEx_t;

/* I_MusicDriver_t -- Driver for playing Music */
typedef struct I_MusicDriver_s
{
	/* Data */
	char Name[MAXDRIVERNAME];					// Name of driver
	uint32_t MusicType;							// Music types supported (<< I_MusicTypes_t)
	bool_t ExternalData;						// If true, player requires external MID/MUS/MOD/etc. (on disk)
												// The play func will be passed a pathname instead of raw data
	uint8_t Priority;							// Priority of the driver
	
	/* Dynamic */
	void* Data;									// Driver personal data
	size_t Size;								// Size of personal data
	
	/* Handlers */
		// Initializes a driver
	bool_t (*Init)(struct I_MusicDriver_s* const a_Driver);
		// Destroys a driver
	bool_t (*Destroy)(struct I_MusicDriver_s* const a_Driver);
		// Success
	void (*Success)(struct I_MusicDriver_s* const a_Driver);
		// Pauses a song (pause ||)
	void (*Pause)(struct I_MusicDriver_s* const a_Driver, const int a_Handle);
		// Resumes a song (play >)
	void (*Resume)(struct I_MusicDriver_s* const a_Driver, const int a_Handle);
		// Stops a song from playing and seeks to start (stop [])
	void (*Stop)(struct I_MusicDriver_s* const a_Driver, const int a_Handle);
		// Length of song
	uint32_t (*Length)(struct I_MusicDriver_s* const a_Driver, const int a_Handle);
		// Seeks to a new position
	void (*Seek)(struct I_MusicDriver_s* const a_Driver, const int a_Handle, const uint32_t a_Pos);
		// Plays a song
	int (*Play)(struct I_MusicDriver_s* const a_Driver, const void* const a_Data, const bool_t Loop);
		// Changes volume
	void (*Volume)(struct I_MusicDriver_s* const a_Driver, const int a_Handle, const uint8_t Vol);
		// Raw MIDI Data
	void (*RawMIDI)(struct I_MusicDriver_s* const a_Driver, const uint32_t a_Msg);
} I_MusicDriver_t;

/****************
*** FUNCTIONS ***
****************/

void I_EventExPush(const I_EventEx_t* const a_Event);
bool_t I_EventExPop(I_EventEx_t* const a_Event);
void I_EventToOldDoom(const I_EventEx_t* const a_Event);

int VID_NumModes(void);
char* __REMOOD_DEPRECATED VID_GetModeName(int a_ModeNum);
int VID_ClosestMode(int* const a_WidthP, int* const a_HeightP, const bool_t a_Fullscreen);
int __REMOOD_DEPRECATED VID_GetModeForSize(int a_Width, int a_Height);
bool_t VID_AddMode(const int a_Width, const int a_Height, const bool_t a_Fullscreen);
int VID_SetMode(int a_ModeNum);

bool_t I_UtilWinArgToUNIXArg(int* const a_argc, char*** const a_argv, const char* const a_Win);
bool_t I_VideoPreInit(void);
bool_t I_VideoBefore320200Init(void);
bool_t I_VideoPostInit(void);
void I_VideoSetBuffer(const uint32_t a_Width, const uint32_t a_Height, const uint32_t a_Pitch, uint8_t* const a_Direct);
void I_VideoUnsetBuffer(void);
uint8_t* I_VideoSoftBuffer(uint32_t* const a_WidthP, uint32_t* const a_HeightP);

uint32_t I_GetTime(void);

void I_ShowEndTxt(const uint8_t* const a_TextData);
void I_TextModeChar(const uint8_t a_Char, const uint8_t Attr);

void I_AddExitFunc(void (*func) ());
void I_RemoveExitFunc(void (*func) ());
void I_ShutdownSystem(void);

const char* I_GetUserName(void);
uint64_t I_GetDiskFreeSpace(const char* const a_Path);

bool_t I_AddMusicDriver(I_MusicDriver_t* const a_Driver);
bool_t I_RemoveMusicDriver(I_MusicDriver_t* const a_Driver);
I_MusicDriver_t* I_FindMusicDriver(const I_MusicType_t a_Type);
void I_InitMusic(void);
void I_ShutdownMusic(void);
void I_UpdateMusic(void);
void I_SetMusicVolume(int volume);
int I_RegisterSong(const char* const a_Lump);
void I_UnRegisterSong(int handle);
void I_PauseSong(int handle);
void I_ResumeSong(int handle);
void I_PlaySong(int handle, int looping);
void I_StopSong(int handle);

#endif /* __I_UTIL_H__ */

