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

/****************
*** CONSTANTS ***
****************/

#define CONEX_MAXVARIABLENAME		32					// Size limit of var name

/*****************
*** STRUCTURES ***
*****************/

struct CONEx_Console_s;

/* CONEx_Buffer_t -- Extended Console Buffer */
typedef struct CONEx_Buffer_s
{
	/* Circular Buffer Data */
	char* Buffer;										// Text in buffer
	size_t BufferSize;									// Size of buffer
	size_t BufferStart;									// Start of buffer
	size_t BufferWrite;									// Write position of buffer
	
	/* Line Buffer Data */
	char** Lines;										// Lines in buffer
	size_t LineSize;									// Size of lines
	size_t LineStart;									// Start of lines (first line)
	size_t LineWrite;									// Write position of buffer
	
	/* Owner */
	struct CONEx_Console_s* Parent;						// Parent console
	void (*WroteLineFunc)(struct CONEx_Console_s* const Parent, struct CONEx_Buffer_s* const This, const char* const Line);
} CONEx_Buffer_t;

/* CONEx_Command_t -- Console command */
typedef struct CONEx_Command_s
{
	/* Basic */
	char Name[CONEX_MAXVARIABLENAME];					// Command name
	uint32_t Hash;										// Hash ID
	uint32_t Flags;										// Command flags
	void (*Func)(struct CONEx_Console_s* Console, struct CONEx_Command_s* Command, const int ArgC, const char* const* const ArgV);
	
	/* Deprecated */
	void (*DepFunc)(void);								// Deprecated function
} CONEx_Command_t;

/* CONEx_Variable_t -- Console variable */
typedef struct CONEx_Variable_s
{
	char Name[CONEX_MAXVARIABLENAME];					// Variable name
	uint32_t Hash;										// Hash ID
	uint32_t Flags;										// Command flags
	void (*Func)(struct CONEx_Console_s* Console, struct CONEx_Variable_s* Command, const int ArgC, const char* const* const ArgV);
} CONEx_Variable_t;

/* CONEx_VarTypeList_t -- Command/variable union */
typedef struct CONEx_VarTypeList_s
{
	boolean IsVariable;									// Is this a variable?
	struct CONEx_VarTypeList_s* Prev;					// Previous link
	struct CONEx_VarTypeList_s* Next;					// Next link
	
	union
	{
		CONEx_Command_t Command;						// A command
		CONEx_Variable_t Variable;						// A variable
	} Data;
} CONEx_VarTypeList_t;

/* CONEx_VarTypeHash_t -- Variable and hash holder */
typedef struct CONEx_VarTypeHash_s
{
	CONEx_VarTypeList_t* VarType;						// Associated variable type
	uint32_t Hash;										// Hash
} CONEx_VarTypeHash_t;

/* CONEx_Console_t -- Extended console interface */
typedef struct CONEx_Console_s
{
	/* Identification */
	uint32_t UUID;										// Console ID
	
	/* Buffers */
	CONEx_Buffer_t* Output;								// Text output buffer
	CONEx_Buffer_t* Command;							// Command buffer
	
	/* Commands and variables */
	CONEx_VarTypeList_t* ComVarList;					// List of commands and variables
	CONEx_VarTypeHash_t* ComVarHash[256];				// Hash list for variables
	size_t NumComVarHash[256];							// Number of console variable hashes
	
	/* Siblings */
	struct CONEx_Console_s* Parent;						// Parent console (attachment)
	struct CONEx_Console_s** Kids;						// Attached consoles
	size_t NumKids;										// Attached console count
} CONEx_Console_t;

/**************
*** GLOBALS ***
**************/

extern boolean g_QuietConsole;							// Mute startup console

/****************
*** FUNCTIONS ***
****************/

CONEx_Buffer_t* CONEx_CreateBuffer(const size_t Size);
void CONEx_DestroyBuffer(CONEx_Buffer_t* const Buffer);
void CONEx_BufferWrite(CONEx_Buffer_t* const Buffer, const char* const Text);

CONEx_Console_t* CONEx_CreateConsole(void);
void CONEx_DestroyConsole(CONEx_Console_t* const Console);

CONEx_Console_t* CONEx_GetRootConsole(void);
CONEx_Console_t* CONEx_GetActiveConsole(void);

void CONEx_AttachConsole(CONEx_Console_t* const ToThis, CONEx_Console_t* const Attacher);
void CONEx_DetachConsole(CONEx_Console_t* const FromThis, CONEx_Console_t* const Detacher);

CONEx_VarTypeList_t* CONEx_FindComVar(CONEx_Console_t* const Console, const char* const String);
CONEx_VarTypeList_t* CONEx_FindComVarHash(CONEx_Console_t* const Console, const uint32_t Hash);

uint32_t CONEx_HashString(const char* const Name);

void CONEx_AddCommand(CONEx_Console_t* const Console, const CONEx_Command_t* const Command);
void CONEx_AddVariable(CONEx_Console_t* const Console, const CONEx_Variable_t* const Variable);

void CONEx_Init(void);
boolean CONEx_Responder(event_t* const Event);
void CONEx_Ticker(void);
void CONEx_Drawer(void);

/*******************************************************************************
********************************************************************************
*******************************************************************************/

// for debugging shopuld be replaced by nothing later.. so debug is inactive
#define LOG(x) CONS_Printf(x)

void CON_Init(void);

boolean CON_Responder(event_t * ev);

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

extern boolean con_started;
extern boolean con_startup;
extern boolean con_forcepic;
extern boolean con_recalc;
extern int con_tick;
extern boolean consoletoggle;
extern boolean consoleready;
extern int con_destlines;
extern int con_curlines;
extern int con_clipviewtop;
extern int con_hudlines;
extern int con_hudtime[5];
extern int con_clearlines;
extern boolean con_hudupdate;
extern char *con_line;
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
extern struct pic_s *con_backpic;
extern struct pic_s *con_bordleft;
extern struct pic_s *con_bordright;
extern char con_buffer[CON_BUFFERSIZE];
extern char *bindtable[NUMINPUTS];
// set true when screen size has changed, to adapt console
extern boolean con_recalc;
extern boolean con_startup;

extern boolean consoleready;	// GhostlyDeath -- extern this here
// top clip value for view render: do not draw part of view hidden by console
extern int con_clipviewtop;

// 0 means console is off, or moving out
extern int con_destlines;

extern int con_clearlines;		// lines of top of screen to refresh
extern boolean con_hudupdate;	// hud messages have changed, need refresh
extern int con_keymap;			//0 english, 1 french

extern byte* redmap;
extern byte *whitemap;
extern byte *greenmap;
extern byte *graymap;
extern byte* orangemap;

extern consvar_t cons_msgtimeout;
extern consvar_t cons_speed;
extern consvar_t cons_height;
extern consvar_t cons_backpic;

void CON_ClearHUD(void);		// clear heads up messages

void CON_Ticker(void);
void CON_Drawer(void);
void CONS_Error(char *msg);		// print out error msg, and wait a key

// force console to move out
void CON_ToggleOff(void);

/******************************************************************************/

#endif /* __CONSOLE_H__ */

