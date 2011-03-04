// Emacs style mode select   -*- C++ -*- 
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
// DESCRIPTION: Console Commands

#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <stdio.h>
#include "doomtype.h"

//===================================
// Command buffer & command execution
//===================================

typedef void (*com_func_t) (void);

void COM_AddCommand(char *name, com_func_t func);

int COM_Argc(void);
char *COM_Argv(int arg);		// if argv>argc, returns empty string
char *COM_Args(void);
int COM_CheckParm(char *check);	// like M_CheckParm :)

// match existing command or NULL
char *COM_CompleteCommand(char *partial, int skips);

// insert at queu (at end of other command)
void COM_BufAddText(char *text);

// insert in head (before other command)
void COM_BufInsertText(char *text);

// Execute commands in buffer, flush them
void COM_BufExecute(void);

// setup command buffer, at game tartup
void COM_Init(void);

// ======================
// Variable sized buffers
// ======================

typedef struct vsbuf_s
{
	boolean allowoverflow;		// if false, do a I_Error
	boolean overflowed;			// set to true if the buffer size failed
	byte *data;
	int maxsize;
	int cursize;
} vsbuf_t;

void VS_Alloc(vsbuf_t * buf, int initsize);
void VS_Free(vsbuf_t * buf);
void VS_Clear(vsbuf_t * buf);
void *VS_GetSpace(vsbuf_t * buf, int length);
void VS_Write(vsbuf_t * buf, void *data, int length);
void VS_Print(vsbuf_t * buf, char *data);	// strcats onto the sizebuf

// ======================

//==================
// Console variables
//==================
// console vars are variables that can be changed through code or console,
// at RUN TIME. They can also act as simplified commands, because a func-
// tion can be attached to a console var, which is called whenever the
// variable is modified (using flag CV_CALL).

// flags for console vars

typedef enum
{
	CV_SAVE = 1,				// save to config when quit game
	CV_CALL = 2,				// call function on change
	CV_NETVAR = 4,				// send it when change (see logboris.txt at 12-4-2000)
	CV_NOINIT = 8,				// dont call function when var is registered (1st set)
	CV_FLOAT = 16,				// the value is fixed 16:16, where unit is FRACUNIT
	// (allow user to enter 0.45 for ex)
	// WARNING: currently only supports set with CV_Set()
	CV_NOTINNET = 32,			// some varaiable can't be changed in network but is not netvar (ex: splitscreen)
	CV_MODIFIED = 64,			// this bit is set when cvar is modified
	CV_SHOWMODIF = 128,			// say something when modified
	CV_SHOWMODIFONETIME = 256,	// same but will be reset to 0 when modified, set in toggle
	CV_HIDEN = 1024,			// variable is not part of the cvar list so cannot be accessed by the console
	// can only be set when we have the pointer to hit 
	// used on the menu
	
	CV_HIDDEN = 1024,			// They can't spell
	
	CV_ALIAS = 2048,			// Alias to another cvar
	CV_DEPRECATED = 4096,		// Going to be removed
	CV_INVERTEDALIAS = 8192,	// Inverted Alias (0 is 1 and 1 is 0)
	
	CV_SAVEDGAMEVAR = 16384,	// Saved in save games
} cvflags_t;

struct CV_PossibleValue_s
{
	int value;
	char *strvalue;
};

typedef struct CV_PossibleValue_s CV_PossibleValue_t;

typedef struct consvar_s
{
	char *name;
	char *defaultvalue;
	int flags;					// flags see cvflags_t above
	CV_PossibleValue_t *PossibleValue;	// table of possible values
	void (*func) (void);		// called on change, if CV_CALL set
	char *aliasto;				// Alias to...
	
	size_t ACount;				// Alias Count
	struct consvar_s** ALink;	// Alias Links
	
	int value;					// for int and fixed_t
	char *string;				// value in string
	wchar_t* wstring;			// value in wide string (Menus)
	unsigned short netid;		// used internaly : netid for send end receive
	// used only with CV_NETVAR
	struct consvar_s *next;
} consvar_t;

extern CV_PossibleValue_t CV_OnOff[];
extern CV_PossibleValue_t CV_YesNo[];
extern CV_PossibleValue_t CV_Unsigned[];
// register a variable for use at the console
void CV_RegisterVar(consvar_t * variable);

// GhostlyDeath <July 8, 2008> -- Unregisters a variable (reverse of CV_RegisterVar)
void CV_UnRegisterVar(consvar_t* var);

// returns the name of the nearest console variable name found
char *CV_CompleteVar(char *partial, int skips);

// equivalent to "<varname> <value>" typed at the console
void CV_Set(consvar_t * var, char *value);

// expands value to a string and calls CV_Set
void CV_SetValue(consvar_t * var, int value);

// it a setvalue but with a modulo at the maximum
void CV_AddValue(consvar_t * var, int increment);

// write all CV_SAVE variables to config file
void CV_SaveVariables(FILE * f);

// load/save gamesate (load and save option and for network join in game)
void CV_SaveNetVars(char **p);
void CV_LoadNetVars(char **p);

consvar_t* CV_Export(void);

consvar_t *CV_FindVar(char *name);

#endif							// __COMMAND_H__

