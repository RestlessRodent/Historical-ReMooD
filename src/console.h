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
// Copyright (C) 2008-2011 GhostlyDeath (ghostlydeath@gmail.com)
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

/****************
*** CONSTANTS ***
****************/

#define CONLCONSOLEFONT VFONT_OEM		// Font used to draw console
#define CONLPADDING 5					// Pad the console
#define CONLSCROLLFORE 4				// Forecolor of the scrollbar
#define CONLSCROLLBACK 0				// Backcolor of the scrollbar
#define CONLSCROLLMISS 200				// Missed color for scrollbar

/*****************
*** STRUCTURES ***
*****************/

/* CONCTI_MBChain_t -- Multibyte chain for character input */
typedef struct CONCTI_MBChain_s
{
	char MB[6];						// Multibyte data
	struct CONCTI_MBChain_s* Prev;	// Previous character
	struct CONCTI_MBChain_s* Next;	// Next character
} CONCTI_MBChain_t;

struct CONCTI_Inputter_s;
typedef bool_t (*CONCTI_OutBack_t)(struct CONCTI_Inputter_s*, const char* const);

/* CONCTI_Inputter_t -- Text inputter */
typedef struct CONCTI_Inputter_s
{
	CONCTI_MBChain_t* ChainRoot;	// First link in chain
	int32_t CursorPos;				// Cursor position
	int32_t NumMBs;					// Number of multibytes
	bool_t Overwrite;				// Overwrite character
	
	char** History;					// Remembered strings
	size_t NumHistory;				// Amount of history to preserve
	size_t HistoryCount;			// Stuff in history
	size_t HistorySpot;				// Current spot in history
	
	CONCTI_OutBack_t OutFunc;		// Function to call when text is entered (\n)
	
	struct CONCTI_Inputter_s** RefPtr;	// Reference to this struct
} CONCTI_Inputter_t;

/*****************
*** PROTOTYPES ***
*****************/

/*** Common Text Input ***/
CONCTI_Inputter_t* CONCTI_CreateInput(const size_t a_NumHistory, const CONCTI_OutBack_t a_OutBack, CONCTI_Inputter_t** const a_RefPtr);
void CONCTI_DestroyInput(CONCTI_Inputter_t* const a_Input);
bool_t CONCTI_HandleEvent(CONCTI_Inputter_t* const a_Input, const I_EventEx_t* const a_Event);
void CONCTI_SetText(CONCTI_Inputter_t* const a_Input, const char* const a_Text);
int32_t CONCTI_DrawInput(CONCTI_Inputter_t* const a_Input, const uint32_t a_Options, const int32_t a_x, const int32_t a_y, const int32_t a_x2);

/*** Base Console ***/
bool_t CONL_Init(const uint32_t a_OutBS, const uint32_t a_InBS);
void CONL_Stop(void);

size_t CONL_PrintV(const bool_t a_InBuf, const char* const a_Format, va_list a_ArgPtr);
size_t CONL_UnicodePrintV(const bool_t a_InBuf, const UnicodeStringID_t a_StrID, const char* const a_Format, va_list a_ArgPtr);

size_t CONL_OutputF(const char* const a_Format, ...);
size_t CONL_InputF(const char* const a_Format, ...);
size_t CONL_OutputU(const UnicodeStringID_t a_StrID, const char* const a_Format, ...);
size_t CONL_InputU(const UnicodeStringID_t a_StrID, const char* const a_Format, ...);

/*** Client Drawing ***/
bool_t CONL_IsActive(void);
bool_t CONL_SetActive(const bool_t a_Set);
bool_t CONL_HandleEvent(const I_EventEx_t* const a_Event);
void CONL_Ticker(void);
void CONL_DrawConsole(void);

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
