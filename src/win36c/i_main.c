// -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
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
// Copyright (C) 2011-2013 GhostlyDeath <ghostlydeath@remood.org>
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
//      Main program, simply calls D_DoomMain high level loop.

/***************
*** INCLUDES ***
***************/


#define __REMOOD_INCLUDEWINDOWSHEADER
#include <windows.h>
#include "doomdef.h"
#include "m_argv.h"
#include "d_main.h"
#include "doomdef.h"
#include "m_argv.h"
#include "d_main.h"
#include "i_util.h"

extern TCHAR ceClassName[];
extern TCHAR ceWindowName[];
extern HINSTANCE ceInstance;

/****************
*** FUNCTIONS ***
****************/

/* main() -- Main entry point */
int main(int argc, char** argv)
{
	/* Set command line */
	I_CommonCommandLine(&argc, &argv, NULL);
	
	/* Run the game */
	D_DoomMain();
	D_DoomLoop();
	
	/* Success! */
	return EXIT_SUCCESS;
}

#if defined(_WIN32_WCE)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpWCmdLine, int nCmdShow)
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
	int i = 0;
	int ArgC = 0;
	char** ArgV = NULL;
	char** NewArgV = NULL;
	char* Token = NULL;
	int ReturnVal = 0;
#if defined(_WIN32_WCE)
	char* lpCmdLine = NULL;
#endif
	size_t clLen = 0;
	HWND cHwnd;
	
	/* If ReMooD is already running... */
	cHwnd = FindWindow(ceClassName, ceWindowName);
	
	if (cHwnd)
	{
		SetForegroundWindow(cHwnd);
		return 0;
	}
	
	/* Otherwise... */
	ceInstance = hInstance;
	
	// Convert down
#if defined(_WIN32_WCE)
	clLen = wcslen(lpWCmdLine);
	lpCmdLine = (char*)malloc(sizeof(char) * (clLen + 1));
	wcstombs(lpCmdLine, lpWCmdLine, clLen);
#endif

	// remood.exe or remood-dbg.exe
	ArgC = 1;
	ArgV = (char**)malloc(sizeof(char*));
	memset(ArgV, 0, sizeof(char*));
#ifdef _DEBUG
	ArgV[0] = (char*)malloc(sizeof(char) * (strlen("remood-dbg.exe") + 1));
	memset(ArgV[0], 0, strlen("remood-dbg.exe"));
	sprintf(ArgV[0], "%s", "remood-dbg.exe");
#else
	ArgV[0] = (char*)malloc(sizeof(char) * (strlen("remood.exe") + 1));
	memset(ArgV[0], 0, strlen("remood.exe"));
	sprintf(ArgV[0], "%s", "remood.exe");
#endif

	// Tokenize by space
	Token = strtok(lpCmdLine, " ");
	while (Token)
	{
		NewArgV = (char**)malloc(sizeof(char*) * (ArgC + 1));
		memset(NewArgV, 0, sizeof(char*) * (ArgC + 1));
		memcpy(NewArgV, ArgV, sizeof(char*) * ArgC);
		free(ArgV);
		ArgV = NewArgV;
		NewArgV = NULL;
		ArgV[ArgC] = (char*)malloc(sizeof(char) * strlen(Token));
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

