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
// Copyright (C) 1993-1996 by id Software, Inc.
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
// DESCRIPTION:
//      Crash handling (For hunting bugs and KILLING them =) !)

#include "doomdef.h"

#if 0
#if defined(LINUX)
#include <signal.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#else
#endif

#define CRASHMSGSIZE 1024

void I_Crash(int parm)
{
	char* message = malloc(CRASHMSGSIZE);
	void** temp = NULL;
	
	if (message)
	{
		memset(message, 0, CRASHMSGSIZE);
		
#if defined(LINUX)
		/* TODO: Use GUI Window */
		// Detect for X via $DISPLAY
		// Detect for GNOME via $GNOME_DESKTOP_SESSION_ID (GTK Crash Dialog)
		// Detect for XFCE via ? (GTK Crash Dialog)
		// Detect for KDE via ? (QT Crash Dialog)
	
		snprintf(message, CRASHMSGSIZE, "%sReMooD %i.%i%c \"%s\" Fatal Error\n", message,
			REMOOD_MAJORVERSION, REMOOD_MINORVERSION, REMOOD_RELEASEVERSION, REMOOD_VERSIONCODESTRING);
		snprintf(message, CRASHMSGSIZE, "%sparm = %i\n", message,
			parm);
		snprintf(message, CRASHMSGSIZE, "%s\n", message);
		snprintf(message, CRASHMSGSIZE, "%sBackTrace:\n", message);
		
		temp = malloc(sizeof(void*) * 10);
		
		if (temp)
		{
			backtrace(temp, 10);
			snprintf(message, CRASHMSGSIZE, "%s%s", message, backtrace_symbols);
			free(temp);
		}
		
		//snprintf(message, CRASHMSGSIZE, "%s", message);
		
		printf("ReMooD Fatal Error (parm = %i)\n", parm);
		printf("Version %
		
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		printf("%s", message);
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
#elif defined(_WIN32)
		/* TODO: use GUI Window */
#endif

		free(message);
	}
}

void I_RegisterCrash(void)
{	
#if defined(LINUX)
	/* Detect debugger presence, if there's no debugger then use our internal debugger */
	// TODO
	
	/* Register */
	signal(SIGILL, I_Crash);		// Illegal Instruction
	signal(SIGFPE, I_Crash);		// Floating Point Exception
	signal(SIGSEGV, I_Crash);		// Segmentation Violation
#elif defined(_WIN32)
	
#endif
}
#endif

