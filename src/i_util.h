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
// DESCRIPTION: Common Interface Utilities (to reduce code bloat and dup)

#ifndef __I_UTIL_H__
#define __I_UTIL_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "d_ticcmd.h"

/****************
*** CONSTANTS ***
****************/

/* I_DataStorageType_t -- Type of data to store */
typedef enum I_DataStorageType_e
{
	DST_CONFIG,									// Config Dir (remoodex.cfg)
	DST_DATA,									// Data Dir (Demos, saves, etc.)
	DST_EXE,									// Executable Directory
	DST_TEMP,									// Temporary Directory
	
	NUMDATASTORAGETYPES
} I_DataStorageType_t;

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
	
	IKBK_UP = 0x11,
	IKBK_DOWN,
	IKBK_LEFT,
	IKBK_RIGHT,
	
	IKBK_ESCAPE = 0x1B,
	
	// Standard ASCII
	IKBK_SPACE = ' ',
	IKBK_EXCLAIM,
	IKBK_QUOTE,
	IKBK_HASH,
	IKBK_DOLLAR,
	IKBK_PERCENT,
	IKBK_AMPERSAND,
	IKBK_APOSTROPHE,
	IKBK_LEFTPARENTHESES,
	IKBK_RIGHTPARENTHESES,
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
	IKBK_KDELETE = 0x7F,
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
	IKBK_EVILEURO,
	
	NUMIKEYBOARDKEYS,
	
	IKBK_GCWZEROL = IKBK_TAB,
	IKBK_GCWZEROR = IKBK_BACKSPACE,
	IKBK_GCWZEROY = IKBK_SPACE,
	IKBK_GCWZEROX = IKBK_SHIFT,
	IKBK_GCWZEROA = IKBK_CTRL,
	IKBK_GCWZEROB = IKBK_ALT,
	IKBK_GCWZEROSELECT = IKBK_ESCAPE,
	IKBK_GCWZEROSTART = IKBK_RETURN,
} I_KeyBoardKey_t;

/* I_EventType_t -- Event type */
typedef enum I_EventType_e
{
	IET_NULL,					// Blank event
	IET_KEYBOARD,				// Keyboard event
	IET_MOUSE,					// Mouse event
	IET_JOYSTICK,				// Joystick event
	IET_QUIT,					// X button was pressed
	IET_SYNTHOSK,				// Synthetic On Screen Keyboard
	
	NUMIEVENTTYPES
} I_EventType_t;

/* I_MusicType_t -- Types of music to play */
typedef enum I_MusicType_e
{
	IMT_MUS,					// Doom MUS
	IMT_MIDI,					// MIDI File
	IMT_MOD,					// MOD Tracker
	
	IMT_UNKNOWN,				// Not in any known format!
	
	NUMIMUSICTYPES
} I_MusicType_t;

/* I_SoundType_t -- Types of sound to play */
typedef enum I_SoundType_e
{
	IST_WAVEFORM,				// Waveform
	IST_BEEPY,					// PC Beeps
	
	IST_UNKNOWN,				// Not in any known format!
	
	NUMISOUNDTYPES
} I_SoundType_t;

#define MAXDRIVERNAME						32	// Driver name limit
#define MAXPROTOCOLNAMES					128	// Maximum protocols permitted (name wise)
#define MAXHOSTNAME							256	// Maximum size for a hostname
#define MAXJOYSTICKS						8	// Maximum number of joysticks allowed

// In i_util.h and g_input.h
#ifndef JOYBUTTONS
#define JOYBUTTONS      32
#endif

extern const char* const c_KeyNames[NUMIKEYBOARDKEYS][2];

/* I_VideoScreen_t -- Video screen */
typedef enum I_VideoScreen_s
{
	IVS_BACKBUFFER,								// Back Buffer
	IVS_FOREBUFFER,								// Foreground Window
	IVS_DONEWITHBUFFER,							// Done with buffer
} I_VideoScreen_t;

/*****************
*** STRUCTURES ***
*****************/

/* I_EventEx_t -- Extended event */
typedef struct I_EventEx_s
{
	I_EventType_t Type;			// Type of event
	
	union
	{
		struct
		{
			bool_t Down;		// Key pressed down
			bool_t Repeat;		// Is the key repeated?
			uint8_t KeyCode;	// Code for key
			uint16_t Character;	// Character pressed
		} Keyboard;				// Keyboard event
		
		struct
		{
			uint8_t MouseID;	// ID of the mouse
			bool_t Down;		// Button down
			uint8_t Button;		// Which button
			int32_t Pos[2];		// Position of cursor
			int32_t Move[2];	// Movement of cursor
		} Mouse;
		
		struct
		{
			uint8_t JoyID;		// ID of the joystick
			bool_t Down;		// Button pressed down
			uint8_t Button;		// Which button was pressed
			uint8_t Axis;		// Which axis was moved
			int16_t Value;		// Axis position
		} Joystick;				// Joystick action
		
		struct
		{
			uint8_t PNum;		// Player Number
			int8_t Right;		// Right movement
			int8_t Down;		// Down movement
			uint8_t Press;		// Press Button
			uint8_t Shift;		// Shifted
			uint32_t Direct;	// Direct Change
			uint8_t Cancel;		// Cancel
			
			uint8_t KeyCode;	// Code for key
			uint16_t Character;	// Character pressed
		} SynthOSK;				// Synthetic OSK
	} Data;						// Event data
} I_EventEx_t;

struct I_SoundDriver_s;

/* I_MusicDriver_t -- Driver for playing Music */
typedef struct I_MusicDriver_s
{
	/* Data */
	char Name[MAXDRIVERNAME];	// Name of driver
	char ShortName[MAXDRIVERNAME];	// Short driver name
	uint32_t MusicType;			// Music types supported (<< I_MusicTypes_t)
	bool_t ExternalData;		// If true, player requires external MID/MUS/MOD/etc. (on disk)
	// The play func will be passed a pathname instead of raw data
	uint8_t Priority;			// Priority of the driver
	
	/* Handlers */
	// Initializes a driver
	bool_t (*Init) (struct I_MusicDriver_s* const a_Driver);
	// Destroys a driver
	bool_t (*Destroy) (struct I_MusicDriver_s* const a_Driver);
	// Success
	void (*Success) (struct I_MusicDriver_s* const a_Driver);
	// Pauses a song (pause ||)
	void (*Pause) (struct I_MusicDriver_s* const a_Driver, const int a_Handle);
	// Resumes a song (play >)
	void (*Resume) (struct I_MusicDriver_s* const a_Driver, const int a_Handle);
	// Stops a song from playing and seeks to start (stop [])
	void (*Stop) (struct I_MusicDriver_s* const a_Driver, const int a_Handle);
	// Length of song
	uint32_t (*Length) (struct I_MusicDriver_s* const a_Driver, const int a_Handle);
	// Seeks to a new position
	void (*Seek) (struct I_MusicDriver_s* const a_Driver, const int a_Handle, const uint32_t a_Pos);
	// Plays a song
	int (*Play) (struct I_MusicDriver_s* const a_Driver, const void* const a_Data, const size_t a_Size, const bool_t Loop);
	// Changes volume
	void (*Volume) (struct I_MusicDriver_s* const a_Driver, const int a_Handle, const uint8_t Vol);
	// Raw MIDI Data
	void (*RawMIDI) (struct I_MusicDriver_s* const a_Driver, const uint32_t a_Msg, const uint32_t a_BitLength);
	// Update MIDI
	void (*Update) (struct I_MusicDriver_s* const a_Driver, const tic_t a_Tics);
	
	// Sound Layering
	void (*SoundLayer)(struct I_MusicDriver_s* const a_Driver, struct I_SoundDriver_s* const a_SoundDriver, void* const a_SoundBuf, const size_t a_SoundLen, const int a_Freq, const int a_Bits, const int a_Channels);
	
	/* Dynamic */
	void* Data;					// Driver personal data
	size_t Size;				// Size of personal data
} I_MusicDriver_t;

/* I_SoundDriver_t -- Sound driver (plays sound, kinda) */
typedef struct I_SoundDriver_s
{
	/* Info */
	char Name[MAXDRIVERNAME];	// Name of driver
	char ShortName[MAXDRIVERNAME];	// Short driver name
	uint32_t SoundType;			// type of sound
	uint8_t Priority;			// Priority of the driver
	
	/* Functions */
	// Initializes a driver
	bool_t (*Init) (struct I_SoundDriver_s* const a_Driver);
	// Destroys a driver
	bool_t (*Destroy) (struct I_SoundDriver_s* const a_Driver);
	// Success
	void (*Success) (struct I_SoundDriver_s* const a_Driver);
	// Requests a buffer for this driver
	size_t(*Request) (struct I_SoundDriver_s* const a_Driver, const uint8_t a_Bits, const uint16_t a_Freq, const uint8_t a_Channels,
	                  const uint32_t a_Samples);
	// Obtains a buffer that was requested
	void *(*Obtain) (struct I_SoundDriver_s* const a_Driver);
	// Checks if the buffer is finished playing
	bool_t (*IsFinished) (struct I_SoundDriver_s* const a_Driver);
	// Done streaming into buffer
	void (*WriteOut) (struct I_SoundDriver_s* const a_Driver);
	// Unrequest buffer
	void (*UnRequest) (struct I_SoundDriver_s* const a_Driver);
	// Get frequency
	uint16_t (*GetFreq) (struct I_SoundDriver_s* const a_Driver);
	// Is sound threaded?
	bool_t (*Thread) (struct I_SoundDriver_s* const a_Driver, void (*a_ThreadFunc) (const bool_t a_Threaded));
	// Locks thread
	void (*LockThread) (struct I_SoundDriver_s* const a_Driver, const bool_t a_Lock);
	
	/* Dynamic */
	void* Data;					// Private data
	size_t Size;				// Private size
} I_SoundDriver_t;

#define __REMOOD_BASEPORT 29500

/* I_NetIPVersionNum_t -- IP version number */
typedef enum I_NetIPVersionNum_s
{
	INIPVN_IPV4						= 4,		// IPv4 Address
	INIPVN_IPV6						= 6,		// IPv6 Address
} I_NetIPVersionNum_t;

typedef struct I_NetSocket_s I_NetSocket_t;		// Network socket

/* I_NetSocketFlags_t -- Network socket flags */
typedef enum I_NetSocketFlags_e
{
	INSF_TCP						= 0x0001,	// TCP (rather than UDP)
	INSF_V6							= 0x0002,	// IPv6 Socket
	INSF_LISTEN						= 0x0004,	// Listening TCP Socket
} I_NetSocketFlags_t;

/* I_HostAddress_t -- Address to a host somewhere */
typedef struct I_HostAddress_s
{
	uint8_t IPvX;								// Which IP version?
	uint32_t Port;								// Remote port for communication
	
	struct
	{
		union
		{
			uint32_t u;							// Integer
			uint16_t s[2];						// Shorts
			uint8_t b[4];						// Bytes
		} v4;									// IPv4 Address
		
		struct
		{
			union
			{
				uint64_t ll[2];					// Long Longs
				uint32_t u[4];					// Integers
				uint16_t s[8];					// Shorts
				uint8_t b[16];					// Bytes
			} Addr;								// Address
			uint32_t Scope;						// Scope
		} v6;
	} Host;										// IP Data
} I_HostAddress_t;

/* I_LocateFileFlags_t -- Flags for I_LocateFile() */
typedef enum I_LocateFileFlags_e
{
	ILFF_TRYBASE		= UINT32_C(0x00000001),	// Tries basename
	ILFF_DIRSEARCH		= UINT32_C(0x00000002),	// Directory searching (caseless)
} I_LocateFileFlags_t;

typedef struct I_File_s I_File_t;

/* I_FileMode_t -- File modes */
typedef enum I_FileMode_e
{
	IFM_TEXT			= UINT32_C(0x01),		// Open as text
	IFM_READ			= UINT32_C(0x02),		// Readable
	IFM_WRITE			= UINT32_C(0x04),		// Writable
	IFM_TRUNCATE		= UINT32_C(0x08),		// Truncate File
	
	IFM_RWT = IFM_READ | IFM_WRITE | IFM_TRUNCATE,
} I_FileMode_t;

/****************
*** FUNCTIONS ***
****************/

/*** i_utlnet.c ***/
uint32_t I_NetHashHost(const I_HostAddress_t* const a_Host);
bool_t I_NetCompareHost(const I_HostAddress_t* const a_A, const I_HostAddress_t* const a_B);
bool_t I_NetNameToHost(I_NetSocket_t* const a_Socket, I_HostAddress_t* const a_Host, const char* const a_Name);
bool_t I_NetHostToName(I_NetSocket_t* const a_Socket, const I_HostAddress_t* const a_Host, char* const a_Out, const size_t a_OutSize);
size_t I_NetHostToString(const I_HostAddress_t* const a_Host, char* const a_Out, const size_t a_OutSize);

I_NetSocket_t* I_NetOpenSocket(const uint32_t a_Flags, const I_HostAddress_t* const a_Host, const uint16_t a_Port);
void I_NetCloseSocket(I_NetSocket_t* const a_Socket);
size_t I_NetReadyBytes(I_NetSocket_t* const a_Socket, const size_t a_Bytes);

size_t I_NetSend(I_NetSocket_t* const a_Socket, const I_HostAddress_t* const a_Host, const void* const a_InData, const size_t a_Len);
size_t I_NetRecv(I_NetSocket_t* const a_Socket, I_HostAddress_t* const a_Host, void* const a_OutData, const size_t a_Len);

/*** i_util.c ***/
void I_EventExPush(const I_EventEx_t* const a_Event);
bool_t I_EventExPop(I_EventEx_t* const a_Event);
void I_DoMouseGrabbing(void);
void I_StartupMouse(void);
void I_StartupMouse2(void);
void I_UpdateJoysticks(void);
void I_InitJoystick(void);
uint8_t I_NumJoysticks(void);
void I_GetEvent(void);
ticcmd_t* I_BaseTiccmd(void);
char* I_GetEnvironment(const char* const a_VarName);
bool_t I_CheckFileAccess(const char* const a_Path, const bool_t a_Write, bool_t* const a_IsDir);
uint16_t I_GetCurrentPID(void);

int VID_NumModes(void);
char* __REMOOD_DEPRECATED VID_GetModeName(int a_ModeNum);
int VID_ClosestMode(int* const a_WidthP, int* const a_HeightP, const bool_t a_Fullscreen);
int __REMOOD_DEPRECATED VID_GetModeForSize(int a_Width, int a_Height);
bool_t VID_AddMode(const int a_Width, const int a_Height, const bool_t a_Fullscreen);
int VID_SetMode(int a_ModeNum);

bool_t I_UtilWinArgToUNIXArg(int* const a_argc, char** *const a_argv, const char* const a_Win);
bool_t I_VideoPreInit(void);
bool_t I_VideoBefore320200Init(void);
bool_t I_VideoPostInit(void);
void I_VideoSetBuffer(const uint32_t a_Width, const uint32_t a_Height, const uint32_t a_Pitch, uint8_t* const a_Direct, const bool_t a_HWDblBuf, const bool_t a_GL);
void I_VideoUnsetBuffer(void);
uint8_t* I_VideoSoftBuffer(uint32_t* const a_WidthP, uint32_t* const a_HeightP);
void I_BeginRead(void);
void I_EndRead(void);

uint32_t I_GetTime(void);
bool_t I_DumpTemporary(char* const a_PathBuf, const size_t a_PathSize, const uint8_t* const a_Data, const size_t a_Size);

void I_ShowEndTxt(const uint8_t* const a_TextData);
void I_TextModeChar(const uint8_t a_Char, const uint8_t Attr);
void I_TextModeNextLine(void);

void I_AddExitFunc(void (*func)(void));
void I_RemoveExitFunc(void (*func)(void));
void I_ShutdownSystem(void);

int I_mkdir(const char* a_Path, int a_UNIXPowers);
const char* I_GetUserName(void);
size_t I_GetStorageDir(char* const a_Out, const size_t a_Size, const I_DataStorageType_t a_Type);
uint64_t I_GetDiskFreeSpace(const char* const a_Path);
uint64_t I_GetFreeMemory(uint64_t* const a_TotalMem);
void I_CommonCommandLine(int* const a_argc, char** *const a_argv, const char* const a_Long);
void I_Quit(void);
void I_Error(char* error, ...);

bool_t I_OpenDir(const char* const a_Path);
bool_t I_ReadDir(char* const a_Dest, const size_t a_BufSize);
void I_CloseDir(void);

typedef bool_t (*I_LFCheckFunc_t)(char* const a_OutPath, const size_t a_OutSize, void* const a_Data);
bool_t I_LocateFile(const char* const a_Name, const uint32_t a_Flags, const char* const* const a_List, const size_t a_Count, char* const a_OutPath, const size_t a_OutSize, const I_LFCheckFunc_t a_CheckFunc, void* const a_CheckData);

I_File_t* I_FileOpen(const char* const a_Path, const uint32_t a_Modes);
void I_FileClose(I_File_t* const a_File);
void I_FileFlush(I_File_t* const a_File);
uint64_t I_FileSeek(I_File_t* const a_File, const uint64_t a_Offset, const bool_t a_End);
uint64_t I_FileTell(I_File_t* const a_File);
uint64_t I_FileRead(I_File_t* const a_File, void* const a_Dest, const uint64_t a_Len);
uint64_t I_FileWrite(I_File_t* const a_File, const void* const a_Src, const uint64_t a_Len);
bool_t I_FileEOF(I_File_t* const a_File);

void I_FileDeletePath(const char* const a_Path);

/*** i_utlnet.c ***/
bool_t I_InitNetwork(void);

/*** i_utlsfx.c ***/
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
size_t I_SoundBufferRequest(const I_SoundType_t a_Type, const uint8_t a_Bits, const uint16_t a_Freq, const uint8_t a_Channels, const uint32_t a_Samples);
bool_t I_SoundSetThreaded(void (*a_ThreadFunc) (const bool_t a_Threaded));
void* I_SoundBufferObtain(void);
bool_t I_SoundBufferIsFinished(void);
void I_SoundBufferWriteOut(void* const a_SoundBuf, const size_t a_SoundLen, const int a_Freq, const int a_Bits, const int a_Channels);
uint16_t I_SoundGetFreq(void);
void I_SoundLockThread(const bool_t a_Lock);

void I_StopCD(void);
void I_PauseCD(void);
void I_ResumeCD(void);
void I_ShutdownCD(void);
void I_InitCD(void);
void I_UpdateCD(void);
void I_PlayCD(int track, bool_t looping);
int I_SetVolumeCD(int volume);

void* I_GetVideoBuffer(const I_VideoScreen_t a_Type, uint32_t* const a_Pitch);

#endif							/* __I_UTIL_H__ */

