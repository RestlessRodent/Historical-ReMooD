// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION:

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

