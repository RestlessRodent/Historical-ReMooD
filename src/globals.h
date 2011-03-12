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
// Copyright (C) 2011 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION: All globals are here

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

/*****************************************************************************/

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"

/*****************
*** STRUCTURES ***
*****************/

/* ReMooDGlobals_t -- All globals defined in this structure */
typedef struct ReMooDGlobals_s
{
	/* d_main.c */
	int demosequence;
	int pagetic;
	char *pagename;
	boolean novideo;
	char *startupwadfiles[MAX_WADFILES];
	boolean devparm;				// started game with -devparm
	boolean nomonsters;				// checkparm of -nomonsters
	boolean infight;				//DarkWolf95:November 21, 2003: Monsters Infight!
	boolean singletics;				// timedemo	boolean nomusic;
	boolean nosound;
	boolean digmusic;				// OGG/MP3 Music SSNTails 12-13-2002
	boolean newnet_use;
	boolean newnet_solo;
	boolean advancedemo;
	char wadfile[1024];				// primary wad file
	char mapdir[1024];				// directory of development maps
	event_t events[MAXEVENTS];
	int eventhead;
	int eventtail;
	boolean dedicated;
	
	/* Global chain */
	struct ReMooDGlobals_s* Prev;						// Previous set of globals
	struct ReMooDGlobals_s* Next;						// Next set of globals
} ReMooDGlobals_t;

/**************
*** GLOBALS ***
**************/

extern ReMooDGlobals_t* g_Globals;

/*****************
*** PROTOTYPES ***
*****************/

void ReMooD_GlobalsNew(void);
void ReMooD_GlobalsDelete(void);
void ReMooD_GlobalsPrev(void);
void ReMooD_GlobalsNext(void);

/*****************************************************************************/

#endif /* __GLOBALS_H__ */

