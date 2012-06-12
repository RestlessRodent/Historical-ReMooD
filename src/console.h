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
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION: Console

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

/******************************************************************************/

/***************
*** INCLUDES ***
***************/

#include "d_event.h"
#include "command.h"
#include "g_input.h"
#include "dstrings.h"
#include "i_util.h"
#include "v_video.h"

/****************
*** CONSTANTS ***
****************/

/*** Console Commands ***/

#define CONLCONSOLEFONT VFONT_OEM				// Font used to draw console
#define CONLPADDING 5							// Pad the console
#define CONLSCROLLFORE 4						// Forecolor of the scrollbar
#define CONLSCROLLBACK 0						// Backcolor of the scrollbar
#define CONLSCROLLMISS 200						// Missed color for scrollbar

/* CONL_ExitCode_t -- Exit code for console command */
typedef enum CONL_ExitCode_e
{
	CLE_SUCCESS,								// Success.
	CLE_FAILURE,								// Failure.
	CLE_NOTANERRORSTRING,						// Invalid Error String
	CLE_CRITICALFAILURE,						// Critical Failure
	CLE_UNKNOWNCOMMAND,							// Unknown Command
	CLE_UNKNOWNVARIABLE,						// Unknown Variable
	CLE_INVALIDARGUMENT,						// Invalid argument
	CLE_RESOURCENOTFOUND,						// Something was not found (file, wad, lump, url, etc.)
	CLE_CONNECTIONREFUSED,						// Refused Connection
	CLE_DISKREADONLY,							// The disk is read only
	CLE_PERMISSIONDENIED,						// Not allowed to do this
	CLE_UNKNOWNSUBCOMMAND,						// Unknown sub command
	
	NUMCONLEXITCODES
} CONL_ExitCode_t;

/*** Console Variables ***/

#define MAXCONLVARIABLENAME		128				// Max name for console command

/* CONL_VariableType_t -- Type of variable */
typedef enum CONL_VariableType_e
{
	CLVT_INTEGER,								// Integer Value (Truncate decimal)
	CLVT_FIXED,									// Fixed point value (Don't truncate)
	CLVT_STRING,								// String Value
	
	NUMCONLVARIABLETYPES
} CONL_VariableType_t;

/* CONL_VariableFlags_t -- Flags for variables */
typedef enum CONL_VariableFlags_e
{
	CLVF_SAVE					= 0x00000001U,	// Save in config file
	CLVF_GAMESTATE				= 0x00000002U,	// Part of the game state
												// Only server can change.
	CLVF_SERVERSTATE			= 0x00000004U,	// Server State (send to clients
												// for server info).
	CLVF_CLIENTSTATE			= 0x00000008U,	// Client State (send to server
												// for client info).
	CLVF_READONLY				= 0x00000010U,	// Variable cannot be changed
												// (except by internal calls)
	CLVF_NOISY					= 0x00000040U,	// Creates lots of noise
} CONL_VariableFlags_t;

/* CONL_VariableVisibleType_t -- How to show the variable (in the menu) */
typedef enum CONL_VariableVisibleType_e
{
	CLVVT_STRING,								// Show: "Hello World"
	CLVVT_INTEGER,								// Show: "12"
	CLVVT_FIXED,								// Show: "12.34"
	CLVVT_PERCENT,								// Show: "34%"
	CLVVT_BAR,									// Show: [======|===]
	
	MAXCONLVARIABLEVISIBLETYPES
} CONL_VariableVisibleType_t;

/*****************
*** STRUCTURES ***
*****************/

/* CONCTI_MBChain_t -- Multibyte chain for character input */
typedef struct CONCTI_MBChain_s
{
	char MB[6];									// Multibyte data
	struct CONCTI_MBChain_s* Prev;				// Previous character
	struct CONCTI_MBChain_s* Next;				// Next character
	
	bool_t EnableVirtual;						// Virtual byte enabled
	char VirtualMB[6];							// Virtual multi-byte character
} CONCTI_MBChain_t;

struct CONCTI_Inputter_s;
typedef bool_t (*CONCTI_OutBack_t) (struct CONCTI_Inputter_s*, const char* const);

/* CONCTI_Inputter_t -- Text inputter */
typedef struct CONCTI_Inputter_s
{
	CONCTI_MBChain_t* ChainRoot;				// First link in chain
	int32_t CursorPos;							// Cursor position
	int32_t NumMBs;								// Number of multibytes
	bool_t Overwrite;							// Overwrite character
	VideoFont_t Font;							// Font to use when drawing
	
	char** History;								// Remembered strings
	size_t NumHistory;							// Amount of history to preserve
	size_t HistoryCount;						// Stuff in history
	int HistorySpot;							// Current spot in history
	size_t HistoryRove;							// History Rove
	
	CONCTI_OutBack_t OutFunc;					// Function to call when text is entered (\n)
	bool_t Changed;								// Input changed?
	
	struct CONCTI_Inputter_s** RefPtr;			// Reference to this struct
} CONCTI_Inputter_t;

/*** Console Variables ***/
typedef struct CONL_ConVariable_s CONL_ConVariable_t;
typedef struct CONL_StaticVar_s CONL_StaticVar_t;

typedef bool_t (*CONL_ConVarBackFunc_t)(CONL_ConVariable_t* const a_Var, CONL_StaticVar_t* const a_StaticVar);

/* CONL_VarPossibleValue_t -- Possible value for a variable */
typedef struct CONL_VarPossibleValue_s
{
	int32_t IntVal;							// Value as integer
	const char* StrAlias;					// String aliase
} CONL_VarPossibleValue_t;

/* CONL_VarValue_t -- Value for variable */
typedef struct CONL_VarValue_s
{
	char* String;								// String value
	int32_t Int;								// Integer value
	fixed_t Fixed;								// Fixed Value
} CONL_VarValue_t;

/* CONL_StaticVar_s -- Static console variable */
// This is the defined local/global, which is then registered
struct CONL_StaticVar_s
{
	/* Defined as static */
	const CONL_VariableType_t Type;				// Variable type
	const CONL_VarPossibleValue_t* Possible;	// Possible Value
	uint32_t Flags;								// Flags for variable
	const char* const VarName;					// Variable Name
	UnicodeStringID_t HintString;				// String used for hint
	CONL_VariableVisibleType_t ShowAs;			// Kind to show as
	const char* const DefaultValue;				// Default Value
	CONL_ConVarBackFunc_t ChangeFunc;			// Function to call when changed
	
	/* Contained Value */
	const CONL_VarValue_t* Value;				// Value for each state
	
	/* Reference Back */
	CONL_ConVariable_t* RealLink;				// Registered Variable
};

/**************
*** GLOBALS ***
**************/

extern const CONL_VarPossibleValue_t c_CVPVClamp[];
extern const CONL_VarPossibleValue_t c_CVPVInteger[];
extern const CONL_VarPossibleValue_t c_CVPVPositive[];
extern const CONL_VarPossibleValue_t c_CVPVNegative[];
extern const CONL_VarPossibleValue_t c_CVPVBoolean[];
extern const CONL_VarPossibleValue_t c_CVPVVexColor[];
extern const CONL_VarPossibleValue_t c_CVPVFont[];

extern bool_t g_EarlyBootConsole;				// Early Boot Console
extern int32_t g_MousePos[2];					// Mouse Position
extern bool_t g_MouseDown;						// Mouse is down

/*****************
*** PROTOTYPES ***
*****************/

void CONL_EarlyBootTic(const char* const a_Message, const bool_t a_DoTic);

/*** Common Text Input ***/
CONCTI_Inputter_t* CONCTI_CreateInput(const size_t a_NumHistory, const CONCTI_OutBack_t a_OutBack, CONCTI_Inputter_t** const a_RefPtr);
void CONCTI_DestroyInput(CONCTI_Inputter_t* const a_Input);
bool_t CONCTI_HandleEvent(CONCTI_Inputter_t* const a_Input, const I_EventEx_t* const a_Event);
void CONCTI_SetText(CONCTI_Inputter_t* const a_Input, const char* const a_Text);
int32_t CONCTI_DrawInput(CONCTI_Inputter_t* const a_Input, const uint32_t a_Options, const int32_t a_x, const int32_t a_y, const int32_t a_x2);

/*** Console Commands ***/
const char* CONL_ExitCodeToStr(const CONL_ExitCode_t a_Code);
bool_t CONL_AddCommand(const char* const a_Name, CONL_ExitCode_t (*a_ComFunc)(const uint32_t, const char** const));
CONL_ExitCode_t CONL_Exec(const uint32_t a_ArgC, const char** const a_ArgV);

/*** Console Variables ***/
bool_t CONL_VarSetLoaded(const bool_t a_Loaded);

CONL_ConVariable_t* CONL_VarRegister(CONL_StaticVar_t* const a_StaticVar);

bool_t CONL_StaticVarByNum(const size_t a_Num, CONL_StaticVar_t** const a_VarP);
CONL_StaticVar_t* CONL_VarLocate(const char* const a_Name);
const char* CONL_VarSetStrByName(const char* const a_Var, const char* const a_NewVal);

const char* CONL_VarSetStr(CONL_StaticVar_t* a_Var, const char* const a_NewVal);
int32_t CONL_VarSetInt(CONL_StaticVar_t* a_Var, const int32_t a_NewVal);
fixed_t CONL_VarSetFixed(CONL_StaticVar_t* a_Var, const fixed_t a_NewVal);

/*** Base Console ***/
bool_t CONL_Init(const uint32_t a_OutBS, const uint32_t a_InBS);
void CONL_Stop(void);

size_t CONL_PrintV(const bool_t a_InBuf, const char* const a_Format, va_list a_ArgPtr);
size_t CONL_UnicodePrintV(const bool_t a_InBuf, const UnicodeStringID_t a_StrID, const char* const a_Format, va_list a_ArgPtr);

size_t CONL_PrintF(const char* const a_Format, ...);
size_t CONL_OutputF(const char* const a_Format, ...);
size_t CONL_InputF(const char* const a_Format, ...);
size_t CONL_OutputU(const UnicodeStringID_t a_StrID, const char* const a_Format, ...);
size_t CONL_InputU(const UnicodeStringID_t a_StrID, const char* const a_Format, ...);

/*** Client Drawing ***/
bool_t CONL_IsActive(void);
bool_t CONL_SetActive(const bool_t a_Set);
bool_t CONL_HandleEvent(const I_EventEx_t* const a_Event);
void CONL_Ticker(void);
bool_t CONL_DrawConsole(void);

void CONL_DrawMouse(void);
void CONLS_DrawOSK(const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H, const uint32_t a_SplitP);
bool_t CONL_OSKSetVisible(const size_t a_PlayerNum, const bool_t a_IsVis);
bool_t CONL_OSKHandleEvent(const I_EventEx_t* const a_Event, const size_t a_PlayerNum);

/*** Console Commands ***/
// Variable Commands
CONL_ExitCode_t CLC_CVarList(const uint32_t a_ArgC, const char** const a_ArgV);
CONL_ExitCode_t CLC_CVarSet(const uint32_t a_ArgC, const char** const a_ArgV);

// Base Commands
CONL_ExitCode_t CLC_Version(const uint32_t a_ArgC, const char** const a_ArgV);
CONL_ExitCode_t CLC_Dep(const uint32_t a_ArgC, const char** const a_ArgV);
CONL_ExitCode_t CLC_Exec(const uint32_t a_ArgC, const char** const a_ArgV);
CONL_ExitCode_t CLC_ExecFile(const uint32_t a_ArgC, const char** const a_ArgV);
CONL_ExitCode_t CLC_Exclamation(const uint32_t a_ArgC, const char** const a_ArgV);
CONL_ExitCode_t CLC_Question(const uint32_t a_ArgC, const char** const a_ArgV);

/*** Configuration Files ***/
size_t CONL_EscapeString(char* const a_Dest, const size_t a_Size, const char* const a_Src);
size_t CONL_UnEscapeString(char* const a_Dest, const size_t a_Size, const char* const a_Src);

bool_t CONL_FindDefaultConfig(void);
bool_t CONL_LoadConfigFile(const char* const a_Path);
bool_t CONL_SaveConfigFile(const char* const a_Path);

/*** Loading Screens ***/
bool_t CONL_LoadingScreenSet(const int32_t a_NumSteps);
bool_t CONL_LoadingScreenIncrMaj(const char* const a_Message, const int32_t a_NumSteps);
bool_t CONL_LoadingScreenIncrSub(void);
bool_t CONL_LoadingScreenSetSubEnd(const int32_t a_NumSteps);

/******************************************************************************/

/****************
*** CONSTANTS ***
****************/

/*****************
*** STRUCTURES ***
*****************/

/**************
*** GLOBALS ***
**************/

extern bool_t g_QuietConsole;	// Mute startup console

/****************
*** FUNCTIONS ***
****************/

/*******************************************************************************
********************************************************************************
*******************************************************************************/

#define DEMOCVAR(x) cv_##x 

// for debugging shopuld be replaced by nothing later.. so debug is inactive
#define LOG(x) CONS_Printf(x)

void CON_Init(void);

bool_t CON_Responder(event_t* ev);

#ifdef __MACOS__
#define  CON_BUFFERSIZE   4096	//my compiler cant handle local vars >32k
#else
#define  CON_BUFFERSIZE   16384
#endif

// TODO: choose max hud msg lines
#define  CON_MAXHUDLINES      5

// hold 32 last lines of input for history
#define  CON_MAXPROMPTCHARS    256
#define  CON_PROMPTCHAR        '>'

extern bool_t con_startup;
extern bool_t con_recalc;
extern int con_tick;
extern bool_t consoletoggle;
extern bool_t consoleready;
extern int con_destlines;
extern int con_curlines;
extern int con_clipviewtop;
extern int con_hudlines;
extern int con_hudtime[5];
extern int con_clearlines;
extern bool_t con_hudupdate;
extern char* con_line;
extern int con_cx;
extern int con_cy;
extern int con_totallines;
extern int con_width;
extern int con_scrollup;
extern int con_lineowner[CON_MAXHUDLINES];
extern char inputlines[32][CON_MAXPROMPTCHARS];
extern int inputline;
extern int inputhist;
extern int input_cx;
extern struct pic_s* con_backpic;
extern struct pic_s* con_bordleft;
extern struct pic_s* con_bordright;
extern char con_buffer[CON_BUFFERSIZE];

extern bool_t consoleready;		// GhostlyDeath -- extern this here

// top clip value for view render: do not draw part of view hidden by console
extern int con_clipviewtop;

// 0 means console is off, or moving out
extern int con_destlines;

extern int con_clearlines;		// lines of top of screen to refresh
extern bool_t con_hudupdate;	// hud messages have changed, need refresh
extern int con_keymap;			//0 english, 1 french

extern uint8_t* redmap;
extern uint8_t* whitemap;
extern uint8_t* greenmap;
extern uint8_t* graymap;
extern uint8_t* orangemap;

extern consvar_t cons_msgtimeout;
extern consvar_t cons_speed;
extern consvar_t cons_height;
extern consvar_t cons_backpic;

void CON_ClearHUD(void);		// clear heads up messages

void CON_Ticker(void);
void CON_Drawer(void);
void CONS_Error(char* msg);		// print out error msg, and wait a key

// force console to move out
void CON_ToggleOff(void);

/******************************************************************************/

#endif							/* __CONSOLE_H__ */
