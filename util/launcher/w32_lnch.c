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
//                    Dragan                 (poliee13@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 2008-2009 The ReMooD Team..
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

#ifdef _UNICODE
#include <wchar.h>
#define StrCmp wcscmp
#define StrLen wcslen
#define StrCpy wcscpy
#define SNPrintf swprintf
#else
#define StrCmp strcmp
#define StrLen strlen
#define StrCpy strcpy
#ifdef _MSC_VER
#define SNPrintf _snprintf
#else
#define SNPrintf snprintf
#endif
#endif

#ifdef _MSC_VER
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

// Autoupdate is at:
//  - http://remood.sourceforge.net/remoodup.txt
//  - http://remood.stealthii.net/remoodup.txt
//  - http://midnightcentral.game-host.org/remoodup.txt
// Freedoom is at:
//  - http://nongnu.askapache.com/freedoom/freedoom-iwad/

/******************************************************************************/
/*************************** DEFINITIONS AND STUFF ****************************/
/******************************************************************************/

/* Defines from doomdef.h */
#define REMOOD_MAJORVERSION 0
#define REMOOD_MINORVERSION 8
#define REMOOD_RELEASEVERSION 'b'
#define REMOOD_VERSIONSTRING "0.8b"
#define REMOOD_VERSIONCODESTRING "French Onion Soup"
#define REMOOD_FULLVERSIONSTRING ""REMOOD_VERSIONSTRING" \""REMOOD_VERSIONCODESTRING"\""
#define REMOOD_URL "http://remood.sourceforge.net"
#define REMOOD_PROPERNAME "ReMooD 0.8b"

#define WIZPADDING 5
#define WIZARDWIDTH (640 - WIZPADDING)
#define WIZARDHEIGHT (350 - WIZPADDING)

/* Control */
typedef struct Control_s
{
	// Structured
	LPCTSTR ClassName;			// Win32 Stuff
	LPCTSTR WindowName;
	
	DWORD Style;				// Look and feel
	DWORD ExStyle;
	
	int x;						// Position and size
	int y;
	int w;
	int h;
	
	//int PageNum;				// Page number control is on
	
	// Action when pressed
	LRESULT CALLBACK (*CommandFunc)(HWND* hWndPtr, UINT* MsgPtr, WPARAM* wParamPtr, LPARAM* lParamPtr);
	int NoMenuHandle;
	
	// Assigned
	HWND hWnd;
	int ID;
} Control_t;

/* WizardPage */
typedef struct WizardPage_s
{
	TCHAR* Title;				// Title of the page (Adds to title bar)
	
	// Prev, Next, Cancel
	LRESULT CALLBACK (*PrevFunc)(HWND* hWndPtr, UINT* MsgPtr, WPARAM* wParamPtr, LPARAM* lParamPtr);
	LRESULT CALLBACK (*NextFunc)(HWND* hWndPtr, UINT* MsgPtr, WPARAM* wParamPtr, LPARAM* lParamPtr);
	LRESULT CALLBACK (*CancelFunc)(HWND* hWndPtr, UINT* MsgPtr, WPARAM* wParamPtr, LPARAM* lParamPtr);
	
	// Controls
	size_t NumControls;
	Control_t* Controls;
} WizardPage_t;

/* Install Method */
typedef struct InstallMethod_s
{
	TCHAR* Title;				// Name of the method
	TCHAR* DiskTitle;			// Title of the Disk you need (if any)
	
	int RequiresDisk;			// Requires a disk to be inserted
	int RequiresInternet;		// Need the internet?
	
	LRESULT CALLBACK (*InstallFunc)(HWND* hWndPtr, UINT* MsgPtr, WPARAM* wParamPtr, LPARAM* lParamPtr);
} InstallMethod_t;

/******************************************************************************/
/*********************** STUFF THAT IS ACTUALLY DEFINED ***********************/
/******************************************************************************/

extern const TCHAR GPLLicense[];				// GPL Text (Defined later)
TCHAR szClassNameS[] = TEXT("ReMooDSplashClass");
TCHAR szClassNameW[] = TEXT("ReMooDWizardClass");
TCHAR szClassNameL[] = TEXT("ReMooDLauncherClass");
int SupportTrans = 0;
HINSTANCE hInst;
HWND hWndS, hWndW, hWndL;
int WizardPage = 0;

TCHAR *QuitStrings[] =
{
	TEXT("Are you sure you want to quit this great game?"),
	TEXT("Please don't leave, there's more demons to toast! Are you sure you want to exit?"),
	TEXT("Let's beat it -- this is turning into a bloodbath! Are you sure you want to exit?"),
	TEXT("I wouldn't leave if I were you. Windows is much worse. Are you sure you want to exit?"),
	TEXT("You're trying to say you like Windows better than me, right?"),
	TEXT("Don't leave yet -- there's a demon around that corner! Are you sure you want to exit?"),
	TEXT("Ya know, next time you come in here i'm gonna toast ya. Are you sure you want to exit?"),
	TEXT("Go ahead and leave. See if I care. Are you sure you want to exit?"),
	TEXT("You want to quit? then, thou hast lost an eighth!"),
	TEXT("Don't go now, there's a dimensional shambler waiting at the Desktop! Are you sure you want to exit?"),
	TEXT("Get outta here and go back to your boring programs. Are you sure you want to exit?"),
	TEXT("If I were your boss, i'd deathmatch ya in a minute! Are you sure you want to exit?"),
	TEXT("Look, bud. you leave now and you forfeit your body count! Are you sure you want to exit?"),
	TEXT("Just leave. when you come back, i'll be waiting with a bat. Are you sure you want to exit?"),
	TEXT("You're lucky I don't smack you for thinking about leaving. Are you sure you want to exit?"),
	TEXT("Too much Banana Cream Pie for you?")
};

/********************/
/****** WIZARD ******/
/********************/

HWND WCancel = NULL;
HWND WPrev = NULL;
HWND WNext = NULL;

#define WZ_CANCEL	1
#define WZ_PREV		2
#define WZ_NEXT		3

Control_t WelcomeControls[] =
{
	// Welcome text
	{
		TEXT("STATIC"),
		TEXT("Thank you for your interest in ReMooD. Before you may continue using this launcher, you are required to set it up so you can play ReMooD. This wizard will guide you through that process"),
		
		0,
		0,
						
		WIZPADDING,
		WIZPADDING,
		WIZARDWIDTH - WIZPADDING,
		60,
		
		NULL,
		1
	},
};

Control_t GPLControls[] =
{
	{
		TEXT("STATIC"),
		TEXT("Before you can use ReMooD you must agree to the following terms and conditions:"),
		
		0,
		0,
						
		WIZPADDING,
		WIZPADDING,
		WIZARDWIDTH - WIZPADDING,
		20,
		
		NULL,
		1
	},
	
	{
		TEXT("EDIT"),
		TEXT(""),
		
		/*WS_BORDER |*/ WS_VSCROLL | WS_TABSTOP | ES_MULTILINE | ES_READONLY | ES_CENTER,
		WS_EX_CLIENTEDGE,
		
		WIZPADDING,
		WIZPADDING + 20,
		WIZARDWIDTH - WIZPADDING - WIZPADDING,
		WIZARDHEIGHT - 20 - WIZPADDING,
		
		NULL,
		1
	},
};

/* The Actual Wizard */
WizardPage_t WizardDef[] =
{
	/* Welcome */
	{
		TEXT("Welcome"),
		
		NULL,
		NULL,
		NULL,
		
		sizeof(WelcomeControls) / sizeof(Control_t),
		WelcomeControls
	},
	
	/* GNU GPL */
	{
		TEXT("License Agreement"),
		
		NULL,
		NULL,
		NULL,
		
		sizeof(GPLControls) / sizeof(Control_t),
		GPLControls
	},
	
	/* ReMooD SETUP (ReMooD EXE+WAD Locations) */
	{
		TEXT("ReMooD Executable and remood.wad Location"),
	},
	
	/* WAD SETUP */
	{
		TEXT("WAD Locations and Installation"),
	},
	
	/* UPDATE CHECK */
	{
		TEXT("Latest Version"),
	},
	
	/* LANGUAGE (BASE FROM REMOOD.WAD LOCALES) */
	{
		TEXT("Language"),
	},
	
	/* CONGRATULATIONS! */
	{
		TEXT("Congratulations"),
	}
};

/*****************************/
/****** INSTALL METHODS ******/
/*****************************/

/******************************************************************************/
/************************************** CODE **********************************/
/******************************************************************************/

/* LoadConfiguration() -- Loads the configuration file */
// 1. .\remood-launcher.cfg
// 2. %HOMEDRIVE%%HOMEPATH%\ReMooD\remood-launcher.cfg (NT)
// 3. %LOCALAPPDATA%\ReMooD\remood-launcher.cfg (Vista)
// 4. %APPDATA%\ReMooD\remood-launcher.cfg (NT)
// 5. C:\Windows\Profiles\User\Application Data\ReMooD\remood-launcher.cfg (9x and NT if APPDATAs fail)
int LoadConfiguration(void)
{
}

/* SaveConfiguration() -- Saves the configuration file */
int SaveConfiguration(void)
{
}

/* SplashProc() -- Splash Screen Code */
LRESULT CALLBACK SplashProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT Ps;
	HDC hDC;
	
	HBITMAP SplashBMP;
	HDC SplashHDC;
	
	UINT AutoCloseTimer = SetTimer(hWnd, 7000, 3000, NULL);
	
	switch (Msg)
	{
		case WM_CREATE:
			break;
			
		case WM_PAINT:
			hDC = BeginPaint(hWnd, &Ps);
			
			// Draw splash bitmap
			SplashBMP = LoadBitmap(hInst, MAKEINTRESOURCE(4));
			SplashHDC = CreateCompatibleDC(hDC);
			SelectObject(SplashHDC, SplashBMP);
			BitBlt(hDC, 0, 0, 640, 200, SplashHDC, 0, 0, SRCCOPY);
			DeleteDC(SplashHDC);
			DeleteObject(SplashBMP);
			
			EndPaint(hWnd, &Ps);
			break;
		
		case WM_TIMER:
			KillTimer(hWnd, 7000);
			DestroyWindow(hWnd);
			break;
		
		case WM_DESTROY:
			ShowWindow(hWndW, SW_SHOW);	// Ignore show command
			UpdateWindow(hWndW);
			break;
		
		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	
	return 0;
}

/* WizardProc() -- Wizard Code */
LRESULT CALLBACK WizardProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HFONT hFont = NULL;
	int i, j;
	
	switch (Msg)
	{
		case WM_CREATE:
			// Font
			hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			
			// Create Cancel, < Prev, and Next > buttons
			WCancel = CreateWindowEx(0,
				TEXT("BUTTON"),
				TEXT("Cancel"),
				WS_TABSTOP | WS_CHILD | WS_VISIBLE,
				10,
				350,
				200,
				25,
				hWnd,
				(HMENU)WZ_CANCEL,
				hInst,
				NULL
				);
			SendMessage(WCancel, WM_SETFONT, (WPARAM)hFont, (LPARAM)LOWORD(TRUE));
			
			WPrev = CreateWindowEx(0,
				TEXT("BUTTON"),
				TEXT("< Previous"),
				WS_TABSTOP | WS_CHILD | WS_VISIBLE,
				220,
				350,
				200,
				25,
				hWnd,
				(HMENU)WZ_PREV,
				hInst,
				NULL
				);
			SendMessage(WPrev, WM_SETFONT, (WPARAM)hFont, (LPARAM)LOWORD(TRUE));
			
			WNext = CreateWindowEx(0,
				TEXT("BUTTON"),
				TEXT("Next >"),
				WS_TABSTOP | WS_CHILD | WS_VISIBLE,
				430,
				350,
				200,
				25,
				hWnd,
				(HMENU)WZ_NEXT,
				hInst,
				NULL
				);
			SendMessage(WNext, WM_SETFONT, (WPARAM)hFont, (LPARAM)LOWORD(TRUE));
			
			/* Create all wizard controls but keep them hidden and disable until needed */
			for (i = 0; i < sizeof(WizardDef) / sizeof(WizardPage_t); i++)
				if (WizardDef[i].NumControls)
					for (j = 0; j < WizardDef[i].NumControls; j++)
					{
						WizardDef[i].Controls[j].hWnd = CreateWindowEx(
							WizardDef[i].Controls[j].ExStyle,
							WizardDef[i].Controls[j].ClassName,
							WizardDef[i].Controls[j].WindowName,
							WS_CHILD | WizardDef[i].Controls[j].Style /*& ~*/ | WS_VISIBLE,
							WizardDef[i].Controls[j].x,
							WizardDef[i].Controls[j].y,
							WizardDef[i].Controls[j].w,
							WizardDef[i].Controls[j].h,
							hWnd,
							(WizardDef[i].Controls[j].NoMenuHandle ? (HMENU)NULL : (HMENU)((100 * (i+1)) + j)),
							hInst,
							NULL
							);
					
						if (WizardDef[i].Controls[j].hWnd)
							SendMessage(WizardDef[i].Controls[j].hWnd, WM_SETFONT, (WPARAM)hFont, (LPARAM)LOWORD(TRUE));
#ifdef _DEBUG
						else
							printf("Wizard Control creation failed on %i, %i (control: %i, error: %i)\n", i, j, ((100 * (i+1)) + j), GetLastError());
#endif
					}
					
			// Set GPL Text
			SendMessage(WizardDef[1].Controls[1].hWnd, WM_SETTEXT, 0, (LPARAM)GPLLicense);
			break;
			
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case WZ_CANCEL:
				case WZ_PREV:
				case WZ_NEXT:
					break;
					
				default:
					break;
			}
			break;
		
		case WM_CLOSE:
			if (MessageBox(hWnd, TEXT("Are you sure you want to cancel the wizard?"), TEXT("Cancel Wizard?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
				PostQuitMessage(0);
			break;
			
		case WM_DESTROY:
			DestroyWindow(hWnd);
			break;
		
		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	
	return 0;
}

/* LauncherProc() -- Launcher Code */
LRESULT CALLBACK LauncherProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
		case WM_CREATE:
			break;
		
		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	
	return 0;
}


/* WinMain() -- Entry Point */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSEX wcexS, wcexW, wcexL;
	MSG MsgS, MsgW, MsgL;
	
	/* Prepare */
	// Copy hInstance
	hInst = hInstance;
	
	// Clear structures
	memset(&wcexS, 0, sizeof(wcexS));
	memset(&wcexW, 0, sizeof(wcexW));
	memset(&wcexL, 0, sizeof(wcexL));
	
	// See what we can do...
	//SupportTrans = 1;
	
	// Setup common controls for XP/Vista Themes
	InitCommonControls();
	
	/* Load Configuration */
	
	/* Splash Screen */
	// Do not display if the user wants to start the launcher minimized
	if (!(nShowCmd == SW_MINIMIZE || nShowCmd == SW_SHOWMINNOACTIVE || nShowCmd == SW_SHOWMINIMIZED))
	{
		// Window Class
		wcexS.cbSize = sizeof(WNDCLASSEX);
		wcexS.cbWndExtra = 0;
		wcexS.cbClsExtra = 0;
		wcexS.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wcexS.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcexS.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
		wcexS.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(2));
		wcexS.hInstance = hInstance;
		wcexS.lpfnWndProc = SplashProc;
		wcexS.lpszMenuName = NULL;
		wcexS.style = 0;
		wcexS.lpszClassName = szClassNameS;
	
		// Splash Screen is not as important if it fails to register
		if (RegisterClassEx(&wcexS))
		{
			hWndS = CreateWindowEx(
				WS_EX_APPWINDOW /*| WS_EX_LAYERED*/ | WS_EX_TOPMOST /*| WS_EX_STATICEDGE*/,
				szClassNameS,
				TEXT(REMOOD_PROPERNAME),
				WS_BORDER | WS_POPUP,
				(GetSystemMetrics(SM_CXSCREEN) >> 1) - ((640) >> 1),
				(GetSystemMetrics(SM_CYSCREEN) >> 1) - ((200) >> 1),
				640,
				200,
				NULL,
				NULL,
				hInstance,
				NULL);
		
			// Not that important if it fails to be created
			if (hWndS)
			{
				ShowWindow(hWndS, SW_SHOW);	// Ignore show command
				UpdateWindow(hWndS);
			}
#ifdef _DEBUG
			else
				printf("LAUNCHER DEBUG: Window creation failed (%i)\n", GetLastError());
#endif
		}
#ifdef _DEBUG
		else
			printf("LAUNCHER DEBUG: Window registration failed (%i)\n", GetLastError());
#endif
	}
#ifdef _DEBUG
	else
		printf("LAUNCHER DEBUG: Window in minimized state (%i)\n", nShowCmd);
#endif
	
	/* Wizard */
	if (1)
	{
		// Window Class
		wcexW.cbSize = sizeof(WNDCLASSEX);
		wcexW.cbWndExtra = 0;
		wcexW.cbClsExtra = 0;
		wcexW.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wcexW.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcexW.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
		wcexW.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(2));
		wcexW.hInstance = hInstance;
		wcexW.lpfnWndProc = WizardProc;
		wcexW.lpszMenuName = NULL;
		wcexW.style = 0;
		wcexW.lpszClassName = szClassNameW;
	
		// Splash Screen is not as important if it fails to register
		if (RegisterClassEx(&wcexW))
		{
			hWndW = CreateWindowEx(
				WS_EX_APPWINDOW /*| WS_EX_LAYERED*/,
				szClassNameW,
				TEXT(""REMOOD_PROPERNAME" Wizard"),
				WS_BORDER | WS_POPUP | WS_CAPTION | WS_DLGFRAME | WS_SYSMENU,
				(GetSystemMetrics(SM_CXSCREEN) >> 1) - ((640) >> 1),
				(GetSystemMetrics(SM_CYSCREEN) >> 1) - ((400) >> 1),
				640,
				400,
				NULL,
				NULL,
				hInstance,
				NULL);
#ifdef _DEBUG
			if (!hWndW)
				printf("LAUNCHER DEBUG: Window creation failed (%i)\n", GetLastError());
#endif
		}
#ifdef _DEBUG
		else
			printf("LAUNCHER DEBUG: Window registration failed (%i)\n", GetLastError());
#endif
	}
#ifdef _DEBUG
	else
		printf("LAUNCHER DEBUG: Launcher already configured!\n");
#endif
	
	/* Actual Launcher */
	
	while (GetMessage(&MsgL, NULL, 0, 0) > 0)
	{
		TranslateMessage(&MsgL);
		DispatchMessage(&MsgL);
	}
	
	/* Save Configuration */
	
	return 0;
}

/******************************************************************************/
/************************************ GNU GPL *********************************/
/******************************************************************************/

const TCHAR GPLLicense[] = TEXT("            GNU GENERAL PUBLIC LICENSE\r\n\
               Version 2, June 1991\r\n\
\r\n\
 Copyright (C) 1989, 1991 Free Software Foundation, Inc.,\r\n\
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA\r\n\
 Everyone is permitted to copy and distribute verbatim copies\r\n\
 of this license document, but changing it is not allowed.\r\n\
\r\n\
                Preamble\r\n\
\r\n\
  The licenses for most software are designed to take away your\r\n\
freedom to share and change it.  By contrast, the GNU General Public\r\n\
License is intended to guarantee your freedom to share and change free\r\n\
software--to make sure the software is free for all its users.  This\r\n\
General Public License applies to most of the Free Software\r\n\
Foundation\'s software and to any other program whose authors commit to\r\n\
using it.  (Some other Free Software Foundation software is covered by\r\n\
the GNU Lesser General Public License instead.)  You can apply it to\r\n\
your programs, too.\r\n\
\r\n\
  When we speak of free software, we are referring to freedom, not\r\n\
price.  Our General Public Licenses are designed to make sure that you\r\n\
have the freedom to distribute copies of free software (and charge for\r\n\
this service if you wish), that you receive source code or can get it\r\n\
if you want it, that you can change the software or use pieces of it\r\n\
in new free programs; and that you know you can do these things.\r\n\
\r\n\
  To protect your rights, we need to make restrictions that forbid\r\n\
anyone to deny you these rights or to ask you to surrender the rights.\r\n\
These restrictions translate to certain responsibilities for you if you\r\n\
distribute copies of the software, or if you modify it.\r\n\
\r\n\
  For example, if you distribute copies of such a program, whether\r\n\
gratis or for a fee, you must give the recipients all the rights that\r\n\
you have.  You must make sure that they, too, receive or can get the\r\n\
source code.  And you must show them these terms so they know their\r\n\
rights.\r\n\
\r\n\
  We protect your rights with two steps: (1) copyright the software, and\r\n\
(2) offer you this license which gives you legal permission to copy,\r\n\
distribute and/or modify the software.\r\n\
\r\n\
  Also, for each author\'s protection and ours, we want to make certain\r\n\
that everyone understands that there is no warranty for this free\r\n\
software.  If the software is modified by someone else and passed on, we\r\n\
want its recipients to know that what they have is not the original, so\r\n\
that any problems introduced by others will not reflect on the original\r\n\
authors\' reputations.\r\n\
\r\n\
  Finally, any free program is threatened constantly by software\r\n\
patents.  We wish to avoid the danger that redistributors of a free\r\n\
program will individually obtain patent licenses, in effect making the\r\n\
program proprietary.  To prevent this, we have made it clear that any\r\n\
patent must be licensed for everyone\'s free use or not licensed at all.\r\n\
\r\n\
  The precise terms and conditions for copying, distribution and\r\n\
modification follow.\r\n\
\r\n\
            GNU GENERAL PUBLIC LICENSE\r\n\
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION\r\n\
\r\n\
  0. This License applies to any program or other work which contains\r\n\
a notice placed by the copyright holder saying it may be distributed\r\n\
under the terms of this General Public License.  The \"Program\", below,\r\n\
refers to any such program or work, and a \"work based on the Program\"\r\n\
means either the Program or any derivative work under copyright law:\r\n\
that is to say, a work containing the Program or a portion of it,\r\n\
either verbatim or with modifications and/or translated into another\r\n\
language.  (Hereinafter, translation is included without limitation in\r\n\
the term \"modification\".)  Each licensee is addressed as \"you\".\r\n\
\r\n\
Activities other than copying, distribution and modification are not\r\n\
covered by this License; they are outside its scope.  The act of\r\n\
running the Program is not restricted, and the output from the Program\r\n\
is covered only if its contents constitute a work based on the\r\n\
Program (independent of having been made by running the Program).\r\n\
Whether that is true depends on what the Program does.\r\n\
\r\n\
  1. You may copy and distribute verbatim copies of the Program\'s\r\n\
source code as you receive it, in any medium, provided that you\r\n\
conspicuously and appropriately publish on each copy an appropriate\r\n\
copyright notice and disclaimer of warranty; keep intact all the\r\n\
notices that refer to this License and to the absence of any warranty;\r\n\
and give any other recipients of the Program a copy of this License\r\n\
along with the Program.\r\n\
\r\n\
You may charge a fee for the physical act of transferring a copy, and\r\n\
you may at your option offer warranty protection in exchange for a fee.\r\n\
\r\n\
  2. You may modify your copy or copies of the Program or any portion\r\n\
of it, thus forming a work based on the Program, and copy and\r\n\
distribute such modifications or work under the terms of Section 1\r\n\
above, provided that you also meet all of these conditions:\r\n\
\r\n\
    a) You must cause the modified files to carry prominent notices\r\n\
    stating that you changed the files and the date of any change.\r\n\
\r\n\
    b) You must cause any work that you distribute or publish, that in\r\n\
    whole or in part contains or is derived from the Program or any\r\n\
    part thereof, to be licensed as a whole at no charge to all third\r\n\
    parties under the terms of this License.\r\n\
\r\n\
    c) If the modified program normally reads commands interactively\r\n\
    when run, you must cause it, when started running for such\r\n\
    interactive use in the most ordinary way, to print or display an\r\n\
    announcement including an appropriate copyright notice and a\r\n\
    notice that there is no warranty (or else, saying that you provide\r\n\
    a warranty) and that users may redistribute the program under\r\n\
    these conditions, and telling the user how to view a copy of this\r\n\
    License.  (Exception: if the Program itself is interactive but\r\n\
    does not normally print such an announcement, your work based on\r\n\
    the Program is not required to print an announcement.)\r\n\
\r\n\
These requirements apply to the modified work as a whole.  If\r\n\
identifiable sections of that work are not derived from the Program,\r\n\
and can be reasonably considered independent and separate works in\r\n\
themselves, then this License, and its terms, do not apply to those\r\n\
sections when you distribute them as separate works.  But when you\r\n\
distribute the same sections as part of a whole which is a work based\r\n\
on the Program, the distribution of the whole must be on the terms of\r\n\
this License, whose permissions for other licensees extend to the\r\n\
entire whole, and thus to each and every part regardless of who wrote it.\r\n\
\r\n\
Thus, it is not the intent of this section to claim rights or contest\r\n\
your rights to work written entirely by you; rather, the intent is to\r\n\
exercise the right to control the distribution of derivative or\r\n\
collective works based on the Program.\r\n\
\r\n\
In addition, mere aggregation of another work not based on the Program\r\n\
with the Program (or with a work based on the Program) on a volume of\r\n\
a storage or distribution medium does not bring the other work under\r\n\
the scope of this License.\r\n\
\r\n\
  3. You may copy and distribute the Program (or a work based on it,\r\n\
under Section 2) in object code or executable form under the terms of\r\n\
Sections 1 and 2 above provided that you also do one of the following:\r\n\
\r\n\
    a) Accompany it with the complete corresponding machine-readable\r\n\
    source code, which must be distributed under the terms of Sections\r\n\
    1 and 2 above on a medium customarily used for software interchange; or,\r\n\
\r\n\
    b) Accompany it with a written offer, valid for at least three\r\n\
    years, to give any third party, for a charge no more than your\r\n\
    cost of physically performing source distribution, a complete\r\n\
    machine-readable copy of the corresponding source code, to be\r\n\
    distributed under the terms of Sections 1 and 2 above on a medium\r\n\
    customarily used for software interchange; or,\r\n\
\r\n\
    c) Accompany it with the information you received as to the offer\r\n\
    to distribute corresponding source code.  (This alternative is\r\n\
    allowed only for noncommercial distribution and only if you\r\n\
    received the program in object code or executable form with such\r\n\
    an offer, in accord with Subsection b above.)\r\n\
\r\n\
The source code for a work means the preferred form of the work for\r\n\
making modifications to it.  For an executable work, complete source\r\n\
code means all the source code for all modules it contains, plus any\r\n\
associated interface definition files, plus the scripts used to\r\n\
control compilation and installation of the executable.  However, as a\r\n\
special exception, the source code distributed need not include\r\n\
anything that is normally distributed (in either source or binary\r\n\
form) with the major components (compiler, kernel, and so on) of the\r\n\
operating system on which the executable runs, unless that component\r\n\
itself accompanies the executable.\r\n\
\r\n\
If distribution of executable or object code is made by offering\r\n\
access to copy from a designated place, then offering equivalent\r\n\
access to copy the source code from the same place counts as\r\n\
distribution of the source code, even though third parties are not\r\n\
compelled to copy the source along with the object code.\r\n\
\r\n\
  4. You may not copy, modify, sublicense, or distribute the Program\r\n\
except as expressly provided under this License.  Any attempt\r\n\
otherwise to copy, modify, sublicense or distribute the Program is\r\n\
void, and will automatically terminate your rights under this License.\r\n\
However, parties who have received copies, or rights, from you under\r\n\
this License will not have their licenses terminated so long as such\r\n\
parties remain in full compliance.\r\n\
\r\n\
  5. You are not required to accept this License, since you have not\r\n\
signed it.  However, nothing else grants you permission to modify or\r\n\
distribute the Program or its derivative works.  These actions are\r\n\
prohibited by law if you do not accept this License.  Therefore, by\r\n\
modifying or distributing the Program (or any work based on the\r\n\
Program), you indicate your acceptance of this License to do so, and\r\n\
all its terms and conditions for copying, distributing or modifying\r\n\
the Program or works based on it.\r\n\
\r\n\
  6. Each time you redistribute the Program (or any work based on the\r\n\
Program), the recipient automatically receives a license from the\r\n\
original licensor to copy, distribute or modify the Program subject to\r\n\
these terms and conditions.  You may not impose any further\r\n\
restrictions on the recipients\' exercise of the rights granted herein.\r\n\
You are not responsible for enforcing compliance by third parties to\r\n\
this License.\r\n\
\r\n\
  7. If, as a consequence of a court judgment or allegation of patent\r\n\
infringement or for any other reason (not limited to patent issues),\r\n\
conditions are imposed on you (whether by court order, agreement or\r\n\
otherwise) that contradict the conditions of this License, they do not\r\n\
excuse you from the conditions of this License.  If you cannot\r\n\
distribute so as to satisfy simultaneously your obligations under this\r\n\
License and any other pertinent obligations, then as a consequence you\r\n\
may not distribute the Program at all.  For example, if a patent\r\n\
license would not permit royalty-free redistribution of the Program by\r\n\
all those who receive copies directly or indirectly through you, then\r\n\
the only way you could satisfy both it and this License would be to\r\n\
refrain entirely from distribution of the Program.\r\n\
\r\n\
If any portion of this section is held invalid or unenforceable under\r\n\
any particular circumstance, the balance of the section is intended to\r\n\
apply and the section as a whole is intended to apply in other\r\n\
circumstances.\r\n\
\r\n\
It is not the purpose of this section to induce you to infringe any\r\n\
patents or other property right claims or to contest validity of any\r\n\
such claims; this section has the sole purpose of protecting the\r\n\
integrity of the free software distribution system, which is\r\n\
implemented by public license practices.  Many people have made\r\n\
generous contributions to the wide range of software distributed\r\n\
through that system in reliance on consistent application of that\r\n\
system; it is up to the author/donor to decide if he or she is willing\r\n\
to distribute software through any other system and a licensee cannot\r\n\
impose that choice.\r\n\
\r\n\
This section is intended to make thoroughly clear what is believed to\r\n\
be a consequence of the rest of this License.\r\n\
\r\n\
  8. If the distribution and/or use of the Program is restricted in\r\n\
certain countries either by patents or by copyrighted interfaces, the\r\n\
original copyright holder who places the Program under this License\r\n\
may add an explicit geographical distribution limitation excluding\r\n\
those countries, so that distribution is permitted only in or among\r\n\
countries not thus excluded.  In such case, this License incorporates\r\n\
the limitation as if written in the body of this License.\r\n\
\r\n\
  9. The Free Software Foundation may publish revised and/or new versions\r\n\
of the General Public License from time to time.  Such new versions will\r\n\
be similar in spirit to the present version, but may differ in detail to\r\n\
address new problems or concerns.\r\n\
\r\n\
Each version is given a distinguishing version number.  If the Program\r\n\
specifies a version number of this License which applies to it and \"any\r\n\
later version\", you have the option of following the terms and conditions\r\n\
either of that version or of any later version published by the Free\r\n\
Software Foundation.  If the Program does not specify a version number of\r\n\
this License, you may choose any version ever published by the Free Software\r\n\
Foundation.\r\n\
\r\n\
  10. If you wish to incorporate parts of the Program into other free\r\n\
programs whose distribution conditions are different, write to the author\r\n\
to ask for permission.  For software which is copyrighted by the Free\r\n\
Software Foundation, write to the Free Software Foundation; we sometimes\r\n\
make exceptions for this.  Our decision will be guided by the two goals\r\n\
of preserving the free status of all derivatives of our free software and\r\n\
of promoting the sharing and reuse of software generally.\r\n\
\r\n\
                NO WARRANTY\r\n\
\r\n\
  11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY\r\n\
FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN\r\n\
OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES\r\n\
PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED\r\n\
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\r\n\
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS\r\n\
TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE\r\n\
PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,\r\n\
REPAIR OR CORRECTION.\r\n\
\r\n\
  12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING\r\n\
WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR\r\n\
REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,\r\n\
INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING\r\n\
OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED\r\n\
TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY\r\n\
YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER\r\n\
PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE\r\n\
POSSIBILITY OF SUCH DAMAGES.\r\n\
\r\n\
             END OF TERMS AND CONDITIONS\r\n");

