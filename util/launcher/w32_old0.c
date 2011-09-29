// Emacs style mode select   -*- C++ -*- 
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
// Copyright (C) 2008-2010 The ReMooD Team..
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
// DESCRIPTION: Win32 Launcher

#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <commctrl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _UNICODE			/*** UNICODE SUPPORT ***/
#include <wchar.h>

#define usnprintf swprintf

#else					/*** ANSI SUPPORT ***/

#if defined(_MSC_VER)
#define usnprintf _snprintf
#else
#define usnprintf snprintf
#endif

#endif

/* Structures */
typedef struct WinControl_s
{
	HWND Handle;
	int identity;
	DWORD ExStyle;
	LPCTSTR ClassName;
	LPCTSTR WindowName;
	DWORD Style;
	int x;
	int y;
	int Width;
	int Height;
} WinControl_t;

/* Constants */
#define WINDOWWIDTH  600
#define WINDOWHEIGHT 400
typedef enum
{
	CE_LAUNCH,
	CE_SINGLEPLAYER,
	CE_MULTIPLAYER,
	CE_SETTINGS,
	CE_WEBSITE,
	CE_EXIT,
	
	NUMCONTROLS
} ControlEnum_t;

/* Globals */
TCHAR ErrorString[256];
TCHAR szClassName[] = TEXT("ReMooDWinClass");
HINSTANCE hInst = NULL;
TCHAR *QuitStrings[] =
{
	TEXT("Are you sure you want to quit this great game?"),
	TEXT("Please don't leave, there's more demons to toast!"),
	TEXT("Let's beat it -- this is turning into a bloodbath!"),
	TEXT("I wouldn't leave if I were you. Windows is much worse."),
	TEXT("You're trying to say you like Windows better than me, right?"),
	TEXT("Don't leave yet -- there's a demon around that corner!"),
	TEXT("Ya know, next time you come in here i'm gonna toast ya."),
	TEXT("Go ahead and leave. See if I care."),
	TEXT("You want to quit? then, thou hast lost an eighth!"),
	TEXT("Don't go now, there's a dimensional shambler waiting at the Desktop!"),
	TEXT("Get outta here and go back to your boring programs."),
	TEXT("If i were your boss, i'd deathmatch ya in a minute!"),
	TEXT("Look, bud. you leave now and you forfeit your body count!"),
	TEXT("Just leave. when you come back, i'll be waiting with a bat."),
	TEXT("You're lucky i don't smack you for thinking about leaving."),
	TEXT("Too much Banana Cream Pie for you?")
};
WinControl_t Controls[] =
{
	/* Left Side Buttons */
	{NULL, CE_LAUNCH, 0, TEXT("BUTTON"), TEXT("&Launch..."), WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		5, 10, 100, 30},
	{NULL, CE_SINGLEPLAYER, 0, TEXT("BUTTON"), TEXT("S&ingle-Player..."), WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		5, 50, 100, 30},
	{NULL, CE_MULTIPLAYER, 0, TEXT("BUTTON"), TEXT("M&ulti-Player..."), WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		5, 90, 100, 30},
	{NULL, CE_SETTINGS, 0, TEXT("BUTTON"), TEXT("&Settings..."), WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		5, 130, 100, 30},
	{NULL, CE_WEBSITE, 0, TEXT("BUTTON"), TEXT("&Web Site..."), WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		5, 170, 100, 30},
	{NULL, CE_EXIT, 0, TEXT("BUTTON"), TEXT("E&xit..."),  WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		5, 210, 100, 30},
};

/* LauncherProc() -- Launcher Procedure */
LRESULT CALLBACK LauncherProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	// Static to prevent it from constantly freeing and such
	static int i = 0;
	static HFONT hFont = NULL;
	static SYSTEMTIME SysTime;
	static RECT WinRect;
	static RECT ControlRect;
	static int LastError;
	
	switch (Msg)
	{
		/* Window Creation */
		case WM_CREATE:
			/* Center on parent */
			GetClientRect(hWnd, &WinRect);

			SetWindowPos(hWnd, NULL,
						(GetSystemMetrics(SM_CXSCREEN) >> 1) - ((WinRect.right - WinRect.left) >> 1),
						(GetSystemMetrics(SM_CYSCREEN) >> 1) - ((WinRect.bottom - WinRect.top) >> 1),
						WINDOWWIDTH,
						WINDOWHEIGHT,
						SWP_NOZORDER);

			/* Use the GUI Font, whatever that may be */
			hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			
			/* Create Controls from loop */
			for (i = 0; i < NUMCONTROLS; i++)
			{
				Controls[i].Handle = CreateWindowEx(
					Controls[i].ExStyle,
					Controls[i].ClassName,
					Controls[i].WindowName,
					Controls[i].Style,
					Controls[i].x,
					Controls[i].y,
					Controls[i].Width,
					Controls[i].Height,
					hWnd,
					(HMENU)Controls[i].identity,
					hInst,
					NULL
					);
				
				if (Controls[i].Handle)
					SendMessage(Controls[i].Handle, WM_SETFONT, (WPARAM)hFont, (LPARAM)LOWORD(TRUE));
				else
				{
					LastError = GetLastError();
					usnprintf(ErrorString, sizeof(ErrorString) / sizeof(TCHAR),
						TEXT("CreateWindowEx() on Control %i failed with error %i!"), i, LastError);
					MessageBox(hWnd, ErrorString, TEXT("Error"), MB_OK | MB_ICONWARNING);
				}
			}
			break;
			
		/* Button Command */
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				/* Go to the home page */
				case CE_WEBSITE:
					if (!ShellExecute(NULL, TEXT("open"), TEXT("http://remood.org/"), NULL, NULL, SW_SHOWNORMAL))
					{
						LastError = GetLastError();
						usnprintf(ErrorString, sizeof(ErrorString) / sizeof(TCHAR),
							TEXT("Attempt to launch web browser to http://remood.org/ failed with error %i!"), i, LastError);
						MessageBox(hWnd, ErrorString, TEXT("Error"), MB_OK | MB_ICONWARNING);
					}
					break;

				/* User wants to leave */
				case CE_EXIT:
					GetSystemTime(&SysTime);
					i = (SysTime.wMilliseconds + (SysTime.wSecond / (SysTime.wMinute + 1))) % (sizeof(QuitStrings) / sizeof(TCHAR*));
					if (MessageBox(hWnd, QuitStrings[i], TEXT("Exit ReMooD?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
						DestroyWindow(hWnd);
					break;

				default:
					break;
			}
			break;
		
		/* Resizing the Window */	
		case WM_SIZE:
			/* Everything is resized based on proportion or something */
			GetClientRect(hWnd, &WinRect);
			
			for (i = 0; i < NUMCONTROLS; i++)
				if (Controls[i].Handle)
					SetWindowPos(Controls[i].Handle, NULL,
						(float)Controls[i].x * ((float)(WinRect.right - WinRect.left) / (float)WINDOWWIDTH),
						(float)Controls[i].y * ((float)(WinRect.bottom - WinRect.top) / (float)WINDOWHEIGHT),
						(float)Controls[i].Width * ((float)(WinRect.right - WinRect.left) / (float)WINDOWWIDTH),
						(float)Controls[i].Height * ((float)(WinRect.bottom - WinRect.top) / (float)WINDOWHEIGHT),
						SWP_NOZORDER);
			break;
			
		/* User wants to quit */
		case WM_CLOSE:
			GetSystemTime(&SysTime);
			i = (SysTime.wMilliseconds + (SysTime.wSecond / (SysTime.wMinute + 1))) % (sizeof(QuitStrings) / sizeof(TCHAR*));
			if (MessageBox(hWnd, QuitStrings[i], TEXT("Exit ReMooD?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
				DestroyWindow(hWnd);
			break;
		
		/* User just quit */
		case WM_DESTROY:
			/* Kill remaining windows */
			for (i = 0; i < NUMCONTROLS; i++)
			{
				DestroyWindow(Controls[i].Handle);
				Controls[i].Handle = NULL;
			}

			/* Now leave */
			PostQuitMessage(0);
			break;
		
		/* No Handled Messages */
		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	
	return 0;
}

/* WinMain() -- Entry Point */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSEX wcex;
	HWND hWnd;
	MSG Msg;
	int LastError;
	int i;
	
	/* Initialize some things */
	for (i = 0; i < NUMCONTROLS; i++)
		Controls[i].Handle = NULL;
	
	hInst = hInstance;
	
	/* For XP/Vista Themes */
	InitCommonControls();
	
	/* Prepare the window class */
	memset(&wcex, 0, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.cbWndExtra = 0;
	wcex.cbClsExtra = 0;
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
	wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(2));
	wcex.hInstance = hInstance;
	wcex.lpfnWndProc = LauncherProc;
	wcex.lpszMenuName = NULL;
	wcex.style = 0;
	wcex.lpszClassName = szClassName;
	
	/* Register it */
	if (!RegisterClassEx(&wcex))
	{
		LastError = GetLastError();
		
		usnprintf(ErrorString, sizeof(ErrorString) / sizeof(TCHAR), TEXT("RegisterClassEx(): Error %i"), LastError);
		
		MessageBox(NULL, ErrorString, TEXT("Fatal Error"), MB_OK | MB_ICONERROR);
		
		return GetLastError();
	}
	
	/* Create the Window */
	hWnd = CreateWindowEx(0,
		szClassName,
		TEXT("ReMooD Launcher"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		WINDOWWIDTH,
		WINDOWHEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);
		
	if (!hWnd)
	{
		LastError = GetLastError();
		
		usnprintf(ErrorString, sizeof(ErrorString) / sizeof(TCHAR), TEXT("CreateWindowEx(): Error %i"), LastError);
		
		MessageBox(NULL, ErrorString, TEXT("Fatal Error"), MB_OK | MB_ICONERROR);
		
		return GetLastError();
	}

	/* Now Show it */
	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	UnregisterClass(szClassName, hInstance);
	
	return Msg.wParam;
}

#endif
