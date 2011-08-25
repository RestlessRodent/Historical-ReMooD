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
	// Lower Control keys
	IKBK_NULL,
	IKBK_BACKSPACE = 0x08,
	IKBK_TAB = 0x09,
	IKBK_RETURN = 0x0A,
	IKBK_SHIFT = 0x0E,
	IKBK_CTRL = 0x0F,
	IKBK_ALT = 0x10,
	IKBK_ESCAPE = 0x1B,
	
	IKBK_UP = 0x11,
	IKBK_DOWN,
	IKBK_LEFT,
	IKBK_RIGHT,
	
	// Standard ASCII
	IKBK_SPACE = ' ',
	IKBK_EXCLAIM,
	IKBK_QUOTE,
	IKBK_HASH,
	IKBK_DOLLAR,
	IKBK_PERCENT,
	IKBK_AMPERSAND,
	IKBK_APOSTROPHE,
	IKBK_LEFTPARENTHESIS,
	IKBK_RIGHTPERENTHESIS,
	IKBK_ASTERISK,
	IKBK_PLUS,
	IKBK_COMMA,
	IKBK_MINUS,
	IKBK_PERIOD,
	IKBK_FORWARDSLASH,
	IKBK_0,
	IKBK_1,
	IKBK_2,
	IKBK_3,
	IKBK_4,
	IKBK_5,
	IKBK_6,
	IKBK_7,
	IKBK_8,
	IKBK_9,
	IKBK_COLON,
	IKBK_SEMICOLON,
	IKBK_LESSTHAN,
	IKBK_EQUALS,
	IKBK_GREATERTHAN,
	IKBK_QUESTION,
	IKBK_AT,
	IKBK_A,
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
	IKBK_LEFTBRACKET,
	IKBK_BACKSLASH,
	IKBK_RIGHTBRACKET,
	IKBK_CARET,
	IKBK_UNDERSCORE,
	IKBK_GRAVE,
	IKBK_LEFTBRACE = 0x7B,
	IKBK_PIPE,
	IKBK_RIGHTBRACE,
	IKBK_TILDE,
	
	// Cool Keys
	IKBK_DELETE = 0x7F,
	IKBK_HOME,
	IKBK_END,
	IKBK_INSERT,
	IKBK_PAGEUP,
	IKBK_PAGEDOWN,
	IKBK_PRINTSCREEN,
	IKBK_NUMLOCK,
	IKBK_CAPSLOCK,
	IKBK_SCROLLLOCK,
	IKBK_PAUSE,
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
	IKBK_NUM0,
	IKBK_NUM1,
	IKBK_NUM2,
	IKBK_NUM3,
	IKBK_NUM4,
	IKBK_NUM5,
	IKBK_NUM6,
	IKBK_NUM7,
	IKBK_NUM8,
	IKBK_NUM9,
	IKBK_NUMDIVIDE,
	IKBK_NUMMULTIPLY,
	IKBK_NUMSUBTRACT,
	IKBK_NUMADD,
	IKBK_NUMENTER,
	IKBK_NUMPERIOD,
	IKBK_NUMDELETE,
	
	IKBK_WINDOWSKEY,
	IKBK_MENUKEY,
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

/* I_SoundType_t -- Types of sound to play */
typedef enum I_SoundType_e
{
	IST_WAVEFORM,								// Waveform
	IST_BEEPY,									// PC Beeps
	
	IST_UNKNOWN,								// Not in any known format!
	
	NUMISOUNDTYPES
} I_SoundType_t;

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
	char ShortName[MAXDRIVERNAME];				// Short driver name
	uint32_t MusicType;							// Music types supported (<< I_MusicTypes_t)
	bool_t ExternalData;						// If true, player requires external MID/MUS/MOD/etc. (on disk)
												// The play func will be passed a pathname instead of raw data
	uint8_t Priority;							// Priority of the driver
	
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
	int (*Play)(struct I_MusicDriver_s* const a_Driver, const void* const a_Data, const size_t a_Size, const bool_t Loop);
		// Changes volume
	void (*Volume)(struct I_MusicDriver_s* const a_Driver, const int a_Handle, const uint8_t Vol);
		// Raw MIDI Data
	void (*RawMIDI)(struct I_MusicDriver_s* const a_Driver, const uint32_t a_Msg, const uint32_t a_BitLength);
		// Update MIDI
	void (*Update)(struct I_MusicDriver_s* const a_Driver, const tic_t a_Tics);
	
	/* Dynamic */
	void* Data;									// Driver personal data
	size_t Size;								// Size of personal data
} I_MusicDriver_t;

/* I_SoundDriver_t -- Sound driver (plays sound, kinda) */
typedef struct I_SoundDriver_s
{
	/* Info */
	char Name[MAXDRIVERNAME];					// Name of driver
	char ShortName[MAXDRIVERNAME];				// Short driver name
	uint32_t SoundType;							// type of sound
	uint8_t Priority;							// Priority of the driver
	
	/* Functions */
		// Initializes a driver
	bool_t (*Init)(struct I_SoundDriver_s* const a_Driver);
		// Destroys a driver
	bool_t (*Destroy)(struct I_SoundDriver_s* const a_Driver);
		// Success
	void (*Success)(struct I_SoundDriver_s* const a_Driver);
		// Requests a buffer for this driver
	bool_t (*Request)(struct I_SoundDriver_s* const a_Driver, const uint8_t a_Bits, const uint16_t a_Freq, const uint8_t a_Channels, const uint32_t a_Samples);
		// Obtains a buffer that was requested
	void* (*Obtain)(struct I_SoundDriver_s* const a_Driver);
		// Checks if the buffer is finished playing
	bool_t (*IsFinished)(struct I_SoundDriver_s* const a_Driver);
		// Done streaming into buffer
	void (*WriteOut)(struct I_SoundDriver_s* const a_Driver);
		// Unrequest buffer
	void (*UnRequest)(struct I_SoundDriver_s* const a_Driver);
	
	/* Dynamic */
	void* Data;									// Private data
	size_t Size;								// Private size
} I_SoundDriver_t;

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
bool_t I_DumpTemporary(char* const a_PathBuf, const size_t a_PathSize, const uint8_t* const a_Data, const size_t a_Size);

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
bool_t I_InitMusic(void);
void I_ShutdownMusic(void);
void I_UpdateMusic(void);
void I_SetMusicVolume(int volume);
int I_RegisterSong(const char* const a_Lump);
void I_UnRegisterSong(int handle);
void I_PauseSong(int handle);
void I_ResumeSong(int handle);
void I_PlaySong(int handle, int looping);
void I_StopSong(int handle);

bool_t I_AddSoundDriver(I_SoundDriver_t* const a_Driver);
bool_t I_RemoveSoundDriver(I_SoundDriver_t* const a_Driver);
I_SoundDriver_t* I_FindSoundDriver(const I_SoundType_t a_Type);
bool_t I_StartupSound(void);
void I_ShutdownSound(void);
void I_UpdateSound(void);
void I_SubmitSound(void);
bool_t I_SoundBufferRequest(const I_SoundType_t a_Type, const uint8_t a_Bits, const uint16_t a_Freq, const uint8_t a_Channels, const uint32_t a_Samples);
void* I_SoundBufferObtain(void);
bool_t I_SoundBufferIsFinished(void);
void I_SoundBufferWriteOut(void);

#endif /* __I_UTIL_H__ */

