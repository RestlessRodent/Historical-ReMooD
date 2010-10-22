// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.sourceforge.net/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           Demyx                  (demyx@endgameftw.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2009 The ReMooD Team..
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION:
//      Main program, simply calls D_DoomMain high level loop.

#include "doomdef.h"

#include "m_argv.h"
#include "d_main.h"

#ifdef LOGMESSAGES
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int logstream;
#endif

#if defined(_WIN32)
int main(int argc, char **argv);

#include <string.h>
#include <stdio.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	int i = 0;
	int ArgC = 0;
	char** ArgV = NULL;
	char** NewArgV = NULL;
	char* Token = NULL;
	int ReturnVal = 0;

	// remood.exe or remood-dbg.exe
	ArgC = 1;
	ArgV = malloc(sizeof(char*));
	memset(ArgV, 0, sizeof(char*));
#ifdef _DEBUG
	ArgV[0] = malloc(sizeof(char) * (strlen("remood-dbg.exe") + 1));
	memset(ArgV[0], 0, strlen("remood-dbg.exe"));
	sprintf(ArgV[0], "%s", "remood-dbg.exe");
#else
	ArgV[0] = malloc(sizeof(char) * (strlen("remood.exe") + 1));
	memset(ArgV[0], 0, strlen("remood.exe"));
	sprintf(ArgV[0], "%s", "remood.exe");
#endif

	// Tokenize by space
	Token = strtok(lpCmdLine, " ");
	while (Token)
	{
		NewArgV = malloc(sizeof(char*) * (ArgC + 1));
		memset(NewArgV, 0, sizeof(char*) * (ArgC + 1));
		memcpy(NewArgV, ArgV, sizeof(char*) * ArgC);
		free(ArgV);
		ArgV = NewArgV;
		NewArgV = NULL;
		ArgV[ArgC] = malloc(sizeof(char) * strlen(Token));
		strcpy(ArgV[ArgC], Token);
		ArgC++;

		Token = strtok(NULL, " ");
	}

	// Call main
	ReturnVal = main(ArgC, ArgV);

	// Free arguments
	if (ArgV)
	{
		for (i = 0; i < ArgC; i++)
			if (ArgV[i])
				free(ArgV[i]);
		free(ArgV);
	}

	return ReturnVal;
}
#endif

int main(int argc, char **argv)
{
	myargc = argc;
	myargv = argv;
	
	//I_RegisterCrash();

#ifdef LOGMESSAGES
	//Hurdler: only write log if we have the permission in the current directory
#ifndef _WIN32
	logstream = creat(".log/log.txt", S_IRUSR | S_IWUSR);
	if (logstream < 0)
	{
		logstream = INVALID_HANDLE_VALUE;	// so we haven't to change the current source code
	}
#endif
#endif

	D_DoomMain();
	D_DoomLoop();
	return 0;
}

