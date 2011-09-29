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
#define strncasecmp strnicmp
#endif

// Freedoom is at http://nongnu.askapache.com/freedoom/freedoom-iwad/

/*******************************************************************************
********************************** STRUCTURES **********************************
*******************************************************************************/
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

/*******************************************************************************
********************************** CONSTANTS ***********************************
*******************************************************************************/
typedef enum
{
	WIZ_CANCEL,
	WIZ_PREV,
	WIZ_NEXT,
	
	WIZ_A_GPLTEXT,
	WIZ_A_AGREETEXT,
	
	WIZ_B_WADLOCATIONNOTE,
	WIZ_B_IWADBOX,
	WIZ_B_FROMFLOPPIES,
	WIZ_B_FROMDOOM2CD,
	WIZ_B_FROMFINALDOOMCD,
	WIZ_B_FROMCOLLECTORSCD,
	WIZ_B_FROMSTEAM,
	WIZ_B_DOWNLOADSHAREWARE,
	WIZ_B_DOWNLOADFREEDOM,
	WIZ_B_SEARCHCOMPUTER,
	WIZ_B_LOCATIONBOX,
	WIZ_B_WADNOTICE,
	WIZ_B_LOCATIONEDIT,
	WIZ_B_BROWSEBUTTON,
	
	WIZ_C_FINALNOTE,
	WIZ_C_LOCATIONEDIT,
	WIZ_C_BROWSEBUTTON,
	WIZ_C_FINALNOTE2,
	WIZ_C_LOCATIONEDIT2,
	WIZ_C_BROWSEBUTTON2,
	
	NUMWIZARDCONTROLS
} WizardControlEnum_t;

typedef enum
{
	LNC_LAUNCH,
	LNC_SINGLEPLAYER,
	LNC_MULTIPLAYER,
	LNC_SETTINGS,
	LNC_WEBSITE,
	LNC_EXIT,
	
	LNC_IWAD,
	LNC_IWADCOMBO,
	LNC_PWAD,
	LNC_PWADCOMBO,
	LNC_PWADADD,
	LNC_PWADREMOVE,
	LNC_PWADUP,
	LNC_PWADDOWN,
	
	LNC_EXTRATIP,
	LNC_EXTRAEDIT,
	
	NUMLAUNCHERCONTROLS
} LauncherControlEnum_t;

/*******************************************************************************
*********************************** GLOBALS ************************************
*******************************************************************************/
HINSTANCE hInst = NULL;
TCHAR* GPLLicense;

TCHAR WizardClassName[] = TEXT("ReMooD-WizardClass");
HWND WizardWindow = NULL;
WinControl_t WizardControls[NUMWIZARDCONTROLS] =
{
	{NULL, WIZ_CANCEL, 0, TEXT("BUTTON"), TEXT("I Disagree"), WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		10, 370, 200, 25},
	{NULL, WIZ_PREV, 0, TEXT("BUTTON"), TEXT("< Previous"), WS_TABSTOP | WS_CHILD | WS_DISABLED,
		220, 370, 200, 25},
	{NULL, WIZ_NEXT, 0, TEXT("BUTTON"), TEXT("I Agree"), WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		430, 370, 200, 25},
		
	/* Page 1 */
	{NULL, WIZ_A_GPLTEXT, 0, TEXT("EDIT"), TEXT(""), WS_BORDER | WS_VSCROLL | WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY, 
		10, 25, 620, 330},
	{NULL, WIZ_A_AGREETEXT, 0, TEXT("STATIC"),
		TEXT("Before you can use ReMooD you must agree to the following terms and conditions:"), WS_CHILD | WS_VISIBLE,
		10, 5, 620, 20},
		
	/* Page 2 */
	{NULL, WIZ_B_WADLOCATIONNOTE, 0, TEXT("STATIC"),
		TEXT("Before you can use ReMooD you might have to install IWADs. You will also have to specify where they are located or should be installed. You do not have to check any of the IWAD Setup options below if you already have an IWAD, you will just need to point to their location."), WS_CHILD,
		10, 5, 620, 40},
	{NULL, WIZ_B_IWADBOX, 0, TEXT("BUTTON"),
		TEXT("IWAD Setup (Optional)"), WS_CHILD | BS_GROUPBOX,
		10, 50, 620, 190},
	{NULL, WIZ_B_FROMFLOPPIES, 0, TEXT("BUTTON"),
		TEXT("Copy From Doom Floppy Disks"), WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
		20, 70, 600, 20},
	{NULL, WIZ_B_FROMDOOM2CD, 0, TEXT("BUTTON"),
		TEXT("Copy From Doom II: Hell on Earth CD"), WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
		20, 90, 600, 20},
	{NULL, WIZ_B_FROMFINALDOOMCD, 0, TEXT("BUTTON"),
		TEXT("Copy From Final Doom CD"), WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
		20, 110, 600, 20},
	{NULL, WIZ_B_FROMCOLLECTORSCD, 0, TEXT("BUTTON"),
		TEXT("Copy From Doom Collector's Edition CD"), WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
		20, 130, 600, 20},
	{NULL, WIZ_B_FROMSTEAM, 0, TEXT("BUTTON"),
		TEXT("Copy from Steam Doom (Requires Steam w/ an already purchased copy of Doom)"), WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
		20, 150, 600, 20},
	{NULL, WIZ_B_DOWNLOADSHAREWARE, 0, TEXT("BUTTON"),
		TEXT("Download Doom Shareware (Requires Internet Connection)"), WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
		20, 170, 600, 20},
	{NULL, WIZ_B_DOWNLOADFREEDOM, 0, TEXT("BUTTON"),
		TEXT("Download Freedoom (Requires Internet Connection)"), WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
		20, 190, 600, 20},
	{NULL, WIZ_B_SEARCHCOMPUTER, 0, TEXT("BUTTON"),
		TEXT("Search Entire Computer (This may take a long time depending on your computer)"), WS_CHILD | WS_TABSTOP | BS_CHECKBOX,
		20, 210, 600, 20},
	{NULL, WIZ_B_LOCATIONBOX, 0, TEXT("BUTTON"),
		TEXT("WAD Location (Required)"), WS_CHILD | BS_GROUPBOX,
		10, 250, 620, 80},
	{NULL, WIZ_B_WADNOTICE, 0, TEXT("STATIC"),
		TEXT("This is the location where WADs are located and will be installed."), WS_CHILD,
		20, 270, 600, 20},
	{NULL, WIZ_B_LOCATIONEDIT, 0, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_BORDER | WS_TABSTOP | ES_READONLY,
		20, 300, 530, 20},
	{NULL, WIZ_B_BROWSEBUTTON, 0, TEXT("BUTTON"), TEXT("Browse..."), WS_CHILD | WS_TABSTOP,
		555, 300, 65, 20},
		
	{NULL, WIZ_C_FINALNOTE, 0, TEXT("STATIC"),
		TEXT("The ReMooD Team thanks you for playing ReMooD, but before you can do so you will have to specify the location of the ReMooD Executable. This setup wizard may have already determined the ReMooD Location."), WS_CHILD,
		10, 5, 620, 40},
	{NULL, WIZ_C_LOCATIONEDIT, 0, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_BORDER | WS_TABSTOP | ES_READONLY,
		10, 50, 540, 20},
	{NULL, WIZ_C_BROWSEBUTTON, 0, TEXT("BUTTON"), TEXT("Browse..."), WS_CHILD | WS_TABSTOP,
		555, 50, 75, 20},
	
	{NULL, WIZ_C_FINALNOTE2, 0, TEXT("STATIC"),
		TEXT("Please specify the location of remood.wad"), WS_CHILD,
		10, 80, 620, 20},
	{NULL, WIZ_C_LOCATIONEDIT2, 0, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_BORDER | WS_TABSTOP | ES_READONLY,
		10, 100, 540, 20},
	{NULL, WIZ_C_BROWSEBUTTON2, 0, TEXT("BUTTON"), TEXT("Browse..."), WS_CHILD | WS_TABSTOP,
		555, 100, 75, 20},
};

TCHAR LauncherClassName[] = TEXT("ReMooD-LauncherClass");
HWND LauncherWindow = NULL;
WinControl_t LauncherControls[NUMLAUNCHERCONTROLS] =
{
	/* Left Side Buttons */
	{NULL, LNC_LAUNCH, 0, TEXT("BUTTON"), TEXT("Launch..."), WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		10, 10, 100, 30},
	{NULL, LNC_SINGLEPLAYER, 0, TEXT("BUTTON"), TEXT("Single-Player..."), WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_DISABLED,
		10, 50, 100, 30},
	{NULL, LNC_MULTIPLAYER, 0, TEXT("BUTTON"), TEXT("Multi-Player..."), WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_DISABLED,
		10, 90, 100, 30},
	{NULL, LNC_SETTINGS, 0, TEXT("BUTTON"), TEXT("Settings..."), WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_DISABLED,
		10, 130, 100, 30},
	{NULL, LNC_WEBSITE, 0, TEXT("BUTTON"), TEXT("Web Site..."), WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		10, 170, 100, 30},
	{NULL, LNC_EXIT, 0, TEXT("BUTTON"), TEXT("Exit..."),  WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		10, 210, 100, 30},
		
	/* Right Side Selections */
	{NULL, LNC_IWAD, 0, TEXT("STATIC"),
		TEXT("What IWAD would you like to use?"), WS_CHILD | WS_VISIBLE,
		120, 10, 505, 20},
	{NULL, LNC_IWADCOMBO, 0, TEXT("COMBOBOX"),
		TEXT(""), WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
		120, 30, 505, 100},
		
	{NULL, LNC_PWAD, 0, TEXT("STATIC"),
		TEXT("What PWAD(s) would you like to use?"), WS_CHILD | WS_VISIBLE,
		120, 60, 505, 20},
	{NULL, LNC_PWADCOMBO, 0, TEXT("LISTBOX"),
		TEXT(""), WS_BORDER | WS_CHILD | WS_VISIBLE | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT | WS_VSCROLL,
		120, 80, 505, 100},
	{NULL, LNC_PWADADD, 0, TEXT("BUTTON"), TEXT("Add..."),  WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		120, 190, 120, 30},
	{NULL, LNC_PWADREMOVE, 0, TEXT("BUTTON"), TEXT("Remove..."),  WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		250, 190, 120, 30},
	{NULL, LNC_PWADUP, 0, TEXT("BUTTON"), TEXT("Move Up"),  WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		380, 190, 120, 30},
	{NULL, LNC_PWADDOWN, 0, TEXT("BUTTON"), TEXT("Move Down"),  WS_TABSTOP | WS_CHILD | WS_VISIBLE,
		510, 190, 120, 30},
	
	{NULL, LNC_EXTRATIP, 0, TEXT("STATIC"),
		TEXT("What extra command line parameters would you like to use?"), WS_CHILD | WS_VISIBLE,
		120, 225, 505, 20},
	{NULL, LNC_EXTRAEDIT, 0, TEXT("EDIT"),
		TEXT(""), WS_BORDER | WS_CHILD | WS_VISIBLE,
		120, 245, 505, 20},
};

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

/*******************************************************************************
***************************** CONFIGURATION CODE *******************************
*******************************************************************************/
FILE* ConfigFile = NULL;
BOOL OptAgreedToGPL = FALSE;
TCHAR WADPath[MAX_PATH];
TCHAR ProgramDir[MAX_PATH];
TCHAR ReMooDWAD[MAX_PATH];

void Launcher_GotConfig(TCHAR* PathName)
{
	char ASCIIName[MAX_PATH];
	TCHAR* x;
	char* y;

	memset(ASCIIName, 0, sizeof(ASCIIName));

	x = PathName;
	y = ASCIIName;
	while (*x)
	{
		if ((*x & 0xFF) >= 32 && (*x & 0xFF) <= 255 && (*x & 0xFF) != 127)
			*y = *x & 0xFF;
		x++;
		y++;
	}

	ConfigFile = fopen(ASCIIName, "rt");

	if (ConfigFile)
	{
		while (!feof(ConfigFile))
		{
			memset(ASCIIName, 0, sizeof(ASCIIName));
			fgets(ASCIIName, MAX_PATH, ConfigFile);

			if (strncmp("gplok", ASCIIName, 5) == 0)
			{
				y = &ASCIIName[6];
				if (*y == 'y')
					OptAgreedToGPL = TRUE;
				else
					OptAgreedToGPL = FALSE;
			}
			else if (strncmp("wadpath", ASCIIName, 7) == 0)
			{
				y = &ASCIIName[8];
				x = WADPath;
				while (*y)
				{
					if (*x >= ' ')
						*x = *y;
					y++;
					x++;
				}
			}
			else if (strncmp("programdir", ASCIIName, 10) == 0)
			{
				y = &ASCIIName[11];
				x = ProgramDir;
				while (*y)
				{
					if (*x >= ' ')
						*x = *y;
					y++;
					x++;
				}
			}
			else if (strncmp("remoodwad", ASCIIName, 9) == 0)
			{
				y = &ASCIIName[10];
				x = ReMooDWAD;
				while (*y)
				{
					if (*x >= ' ')
						*x = *y;
					y++;
					x++;
				}
			}
		}
	}
}

/* Launcher_LoadDefaultConfig() -- Attempts to load a configuration file */
void Launcher_LoadDefaultConfig(void)
{
	TCHAR TryPath[MAX_PATH];
	TCHAR Buffer[1024];
	
	// Try to find our configuration in the current dir first
	if (GetFileAttributes(TEXT(".\\remood-launcher.cfg")) != INVALID_FILE_ATTRIBUTES)
	{
		Launcher_GotConfig(TEXT(".\\remood-launcher.cfg"));
		return;
	}
	
	// Try it in home
	if (strncasecmp(getenv("OS"), "Windows_NT", 10) == 0)
		if (getenv("HOMEDRIVE") && getenv("HOMEPATH"))
			SNPrintf(TryPath, MAX_PATH, TEXT("%s%s\\ReMooD"), getenv("HOMEDRIVE"), getenv("HOMEPATH"));
	
	if (GetFileAttributes(TryPath) != INVALID_FILE_ATTRIBUTES)
	{
#ifdef _UNICODE
		SNPrintf(TryPath, MAX_PATH, TEXT("%ls\\remood-launcher.cfg"), TryPath);
#else
		SNPrintf(TryPath, MAX_PATH, TEXT("%s\\remood-launcher.cfg"), TryPath);
#endif
	
		if (GetFileAttributes(TryPath) != INVALID_FILE_ATTRIBUTES)
			Launcher_GotConfig(TryPath);
		
		return;
	}
}

/* Launcher_SaveDefaultConfig() -- Attempts to save a configuration file */
void Launcher_SaveDefaultConfig(void)
{
	TCHAR TryPath[MAX_PATH];
	char ASCIIName[MAX_PATH];
	int i, j;
	TCHAR* x;
	char* y;
	
	memset(TryPath, 0, sizeof(TryPath));
	
	if (strncasecmp(getenv("OS"), "Windows_NT", 10) == 0)
		if (getenv("HOMEDRIVE") && getenv("HOMEPATH"))
		{
			SNPrintf(TryPath, MAX_PATH, TEXT("%s%s\\ReMooD"), getenv("HOMEDRIVE"), getenv("HOMEPATH"));
	
			if (GetFileAttributes(TryPath) == INVALID_FILE_ATTRIBUTES)
				CreateDirectory(TryPath, NULL);
				
#ifdef _UNICODE
			SNPrintf(TryPath, MAX_PATH, TEXT("%ls\\remood-launcher.cfg"), TryPath);
#else
			SNPrintf(TryPath, MAX_PATH, TEXT("%s\\remood-launcher.cfg"), TryPath);
#endif
			
		}
	
	if (!StrLen(TryPath))
		SNPrintf(TryPath, MAX_PATH, TEXT(".\\remood-launcher.cfg"));

	if (ConfigFile)
		fclose(ConfigFile);
	ConfigFile = NULL;

	memset(ASCIIName, 0, sizeof(ASCIIName));

	x = TryPath;
	y = ASCIIName;
	while (*x)
	{
		if ((*x & 0xFF) >= 32 && (*x & 0xFF) <= 255 && (*x & 0xFF) != 127)
			*y = *x & 0xFF;
		x++;
		y++;
	}

	ConfigFile = fopen(ASCIIName, "wt");

	if (ConfigFile)
	{
		fprintf(ConfigFile, "gplok %s\n", (OptAgreedToGPL ? "y" : "n"));

		memset(ASCIIName, 0, sizeof(ASCIIName));
		x = WADPath;
		y = ASCIIName;
		while (*x)
		{
			if ((*x & 0xFF) >= 32 && (*x & 0xFF) <= 255 && (*x & 0xFF) != 127)
				*y = *x & 0xFF;
			x++;
			y++;
		}
		fprintf(ConfigFile, "wadpath %s\n", ASCIIName);
		
		memset(ASCIIName, 0, sizeof(ASCIIName));
		x = ProgramDir;
		y = ASCIIName;
		while (*x)
		{
			if ((*x & 0xFF) >= 32 && (*x & 0xFF) <= 255 && (*x & 0xFF) != 127)
				*y = *x & 0xFF;
			x++;
			y++;
		}
		fprintf(ConfigFile, "programdir %s\n", ASCIIName);
		
		memset(ASCIIName, 0, sizeof(ASCIIName));
		x = ReMooDWAD;
		y = ASCIIName;
		while (*x)
		{
			if ((*x & 0xFF) >= 32 && (*x & 0xFF) <= 255 && (*x & 0xFF) != 127)
				*y = *x & 0xFF;
			x++;
			y++;
		}
		fprintf(ConfigFile, "remoodwad %s\n", ASCIIName);

		fclose(ConfigFile);
	}
#if 0
	if (ConfigFile != INVALID_HANDLE_VALUE)
		CloseHandle(ConfigFile);

	ConfigFile = CreateFile(TryPath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	
	if (ConfigFile <= 0x100)
		return;
		
	SNPrintf(Buffer, 1024,
		TEXT("gplok %s\r\nwadpath %s\r\nprogramdir %s\r\nremoodwad %s\r\n"),
		(OptAgreedToGPL ? TEXT("y") : TEXT("n")),
		WADPath, ProgramDir, ReMooDWAD);
	j = StrLen(Buffer);
	
	for (i = 0; i < (j - 1); i++)
	{
		x = Buffer[i];
		WriteFile(ConfigFile, &x, 1, NULL, NULL);
	}
		
	CloseHandle(ConfigFile);
#endif
}

BOOL DoomWAD = FALSE;
BOOL Doom2WAD = FALSE;
BOOL TNTWAD = FALSE;
BOOL PlutoniaWAD = FALSE;
BOOL SharewareWAD = FALSE;
BOOL FreedoomWAD = FALSE;

int Wizard_FloppyInstall(TCHAR* InstallPath)
{
	int OldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
	DWORD Flags = 0;
	BOOL UsingA = FALSE;
	BOOL UsingB = FALSE;
	BOOL Attempt = TRUE;

	MessageBox(WizardWindow, TEXT("Floppy install not implemented!"), TEXT("Nothing"), MB_OK | MB_ICONERROR);
	return 0;
	
	MessageBox(WizardWindow, TEXT("Please insert the first floppy disk, then press OK"), TEXT("Installation Notice"), MB_OK | MB_ICONINFORMATION);

	while (Attempt)
	{
		// Reset
		UsingA = FALSE;
		UsingB = FALSE;

		// Get disk information
		if (GetVolumeInformation(TEXT("A:"), NULL, 0, NULL, 0, &Flags, NULL, 0) == 0)
		{
			if (GetVolumeInformation(TEXT("B:"), NULL, 0, NULL, 0, &Flags, NULL, 0) == 0)
			{
				if (MessageBox(WizardWindow, TEXT("Please insert a disk into Drive A: or Drive B: to continue installation. Would you like to retry?"), TEXT("Installation Error"), MB_YESNO | MB_ICONWARNING) == IDYES)
					continue;
				else
					break;
			}
			else
				UsingB = TRUE;
		}
		else
			UsingA = TRUE;

		// We found the disk! Now we need to check it for .DAT files
		MessageBox(WizardWindow, TEXT("Floppy install not implemented!"), TEXT("Nothing"), MB_OK | MB_ICONERROR);
		break;
	}

	// Restore old mode...
	SetErrorMode(OldErrorMode);

	return 0;
}

int Wizard_Doom2Install(TCHAR* InstallPath)
{
	int OldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
	DWORD Flags = 0;
	TCHAR ActiveDrive[3] = TEXT("D:");
	BOOL Attempt = TRUE;
	BOOL FoundWAD = FALSE;
	BOOL AllowWrite = FALSE;
	TCHAR CheckPath[MAX_PATH];
	TCHAR DestPath[MAX_PATH];

	MessageBox(WizardWindow, TEXT("Please insert the Doom 2: Hell on Earth Disc, then press OK"), TEXT("Installation Notice"), MB_OK | MB_ICONINFORMATION);

	while (Attempt)
	{
		// Search from D: to Z:
		while (ActiveDrive[0] <= 'Z')
		{
			if (GetDriveType(ActiveDrive) == DRIVE_CDROM)
				if (GetVolumeInformation(ActiveDrive, NULL, 0, NULL, 0, &Flags, NULL, 0))
					break;

			ActiveDrive[0]++;

			if (ActiveDrive[0] >= ('Z' + 1))
			{
				if (MessageBox(WizardWindow, TEXT("Please insert the Doom 2: Hell on Earth disc into your CD-ROM Drive. Would you like to retry?"), TEXT("Installation Error"), MB_YESNO | MB_ICONWARNING) == IDYES)
				{
					ActiveDrive[0] = 'D';
					continue;
				}
				else
				{
					Attempt = FALSE;
					break;
				}
			}
		}

		// End of search?
		if (ActiveDrive[0] >= ('Z' + 1) || !Attempt)
			break;

		if (!Doom2WAD)		// D:\Setup\Doom2
		{
			SNPrintf(CheckPath, MAX_PATH, TEXT("%s\\doom2.wad"), ActiveDrive);
			SNPrintf(DestPath, MAX_PATH, TEXT("%s\\doom2.wad"), ((InstallPath && StrLen(InstallPath)) ? InstallPath : TEXT(".")));
			if (GetFileAttributes(CheckPath) != INVALID_FILE_ATTRIBUTES)
			{
				if (GetFileAttributes(DestPath) != INVALID_FILE_ATTRIBUTES)
				{
					if (MessageBox(WizardWindow, TEXT("Would you like to overwrite doom2.wad?"), TEXT("Overwrite?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
						AllowWrite = TRUE;
					else
						AllowWrite = FALSE;
				}
				else
					AllowWrite = TRUE;

				if (AllowWrite && CopyFile(CheckPath, DestPath, FALSE))
				{
					Doom2WAD = TRUE;
					Attempt = FALSE;
					FoundWAD = TRUE;
				}
				else if (AllowWrite)
					MessageBox(WizardWindow, TEXT("Copy of doom2.wad failed!"), TEXT("Installation Failure"), MB_OK | MB_ICONWARNING);
			}
		}

		if (!FoundWAD)
		{
			ActiveDrive[0]++;
			continue;
		}
		else
			Attempt = FALSE;
	}

	// Restore old mode...
	SetErrorMode(OldErrorMode);
	return 0;
}

int Wizard_FinalDoomInstall(TCHAR* InstallPath)
{
	int OldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
	DWORD Flags = 0;
	TCHAR ActiveDrive[3] = TEXT("D:");
	BOOL Attempt = TRUE;
	BOOL FoundWAD = FALSE;
	BOOL AllowWrite = FALSE;
	TCHAR CheckPath[MAX_PATH];
	TCHAR DestPath[MAX_PATH];

	MessageBox(WizardWindow, TEXT("Please insert the Final Doom Disc, then press OK"), TEXT("Installation Notice"), MB_OK | MB_ICONINFORMATION);

	while (Attempt)
	{
		// Search from D: to Z:
		while (ActiveDrive[0] <= 'Z')
		{
			if (GetDriveType(ActiveDrive) == DRIVE_CDROM)
				if (GetVolumeInformation(ActiveDrive, NULL, 0, NULL, 0, &Flags, NULL, 0))
					break;

			ActiveDrive[0]++;

			if (ActiveDrive[0] >= ('Z' + 1))
			{
				if (MessageBox(WizardWindow, TEXT("Please insert the Final Doom disc into your CD-ROM Drive. Would you like to retry?"), TEXT("Installation Error"), MB_YESNO | MB_ICONWARNING) == IDYES)
				{
					ActiveDrive[0] = 'D';
					continue;
				}
				else
				{
					Attempt = FALSE;
					break;
				}
			}
		}

		// End of search?
		if (ActiveDrive[0] >= ('Z' + 1) || !Attempt)
			break;

		if (!TNTWAD)
		{
			SNPrintf(CheckPath, MAX_PATH, TEXT("%s\\tnt.wad"), ActiveDrive);
			SNPrintf(DestPath, MAX_PATH, TEXT("%s\\tnt.wad"), ((InstallPath && StrLen(InstallPath)) ? InstallPath : TEXT(".")));
			if (GetFileAttributes(CheckPath) != INVALID_FILE_ATTRIBUTES)
			{
				if (GetFileAttributes(DestPath) != INVALID_FILE_ATTRIBUTES)
				{
					if (MessageBox(WizardWindow, TEXT("Would you like to overwrite tnt.wad?"), TEXT("Overwrite?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
						AllowWrite = TRUE;
					else
						AllowWrite = FALSE;
				}
				else
					AllowWrite = TRUE;

				if (AllowWrite && CopyFile(CheckPath, DestPath, FALSE))
				{
					TNTWAD = TRUE;
					Attempt = FALSE;
					FoundWAD = TRUE;
				}
				else if (AllowWrite)
					MessageBox(WizardWindow, TEXT("Copy of tnt.wad failed!"), TEXT("Installation Failure"), MB_OK | MB_ICONWARNING);
			}
		}

		if (!PlutoniaWAD)
		{
			SNPrintf(CheckPath, MAX_PATH, TEXT("%s\\plutonia.wad"), ActiveDrive);
			SNPrintf(DestPath, MAX_PATH, TEXT("%s\\plutonia.wad"), ((InstallPath && StrLen(InstallPath)) ? InstallPath : TEXT(".")));
			if (GetFileAttributes(CheckPath) != INVALID_FILE_ATTRIBUTES)
			{
				if (GetFileAttributes(DestPath) != INVALID_FILE_ATTRIBUTES)
				{
					if (MessageBox(WizardWindow, TEXT("Would you like to overwrite plutonia.wad?"), TEXT("Overwrite?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
						AllowWrite = TRUE;
					else
						AllowWrite = FALSE;
				}
				else
					AllowWrite = TRUE;

				if (AllowWrite && CopyFile(CheckPath, DestPath, FALSE))
				{
					PlutoniaWAD = TRUE;
					Attempt = FALSE;
					FoundWAD = TRUE;
				}
				else if (AllowWrite)
					MessageBox(WizardWindow, TEXT("Copy of plutonia.wad failed!"), TEXT("Installation Failure"), MB_OK | MB_ICONWARNING);
			}
		}

		if (!FoundWAD)
		{
			ActiveDrive[0]++;
			continue;
		}
		else
			Attempt = FALSE;
	}

	// Restore old mode...
	SetErrorMode(OldErrorMode);
	return 0;
}

int Wizard_CollectorsInstall(TCHAR* InstallPath)
{
	int OldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
	DWORD Flags = 0;
	TCHAR ActiveDrive[3] = TEXT("D:");
	BOOL Attempt = TRUE;
	BOOL FoundWAD = FALSE;
	BOOL AllowWrite = FALSE;
	TCHAR CheckPath[MAX_PATH];
	TCHAR DestPath[MAX_PATH];

	MessageBox(WizardWindow, TEXT("Please insert the Doom Collector's Edition Disc, then press OK"), TEXT("Installation Notice"), MB_OK | MB_ICONINFORMATION);

	while (Attempt)
	{
		// Search from D: to Z:
		while (ActiveDrive[0] <= 'Z')
		{
			if (GetDriveType(ActiveDrive) == DRIVE_CDROM)
				if (GetVolumeInformation(ActiveDrive, NULL, 0, NULL, 0, &Flags, NULL, 0))
					break;

			ActiveDrive[0]++;

			if (ActiveDrive[0] >= ('Z' + 1))
			{
				if (MessageBox(WizardWindow, TEXT("Please insert the Doom Collector's Edition disc into your CD-ROM Drive. Would you like to retry?"), TEXT("Installation Error"), MB_YESNO | MB_ICONWARNING) == IDYES)
				{
					ActiveDrive[0] = 'D';
					continue;
				}
				else
				{
					Attempt = FALSE;
					break;
				}
			}
		}

		// End of search?
		if (ActiveDrive[0] >= ('Z' + 1) || !Attempt)
			break;

		// Try to install IWADs
		if (!DoomWAD)		// D:\Setup\Ultimate Doom
		{
			SNPrintf(CheckPath, MAX_PATH, TEXT("%s\\Setup\\Ultimate Doom\\doom.wad"), ActiveDrive);
			SNPrintf(DestPath, MAX_PATH, TEXT("%s\\doom.wad"), ((InstallPath && StrLen(InstallPath)) ? InstallPath : TEXT(".")));
			if (GetFileAttributes(CheckPath) != INVALID_FILE_ATTRIBUTES)
			{
				if (GetFileAttributes(DestPath) != INVALID_FILE_ATTRIBUTES)
				{
					if (MessageBox(WizardWindow, TEXT("Would you like to overwrite doom.wad?"), TEXT("Overwrite?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
						AllowWrite = TRUE;
					else
						AllowWrite = FALSE;
				}
				else
					AllowWrite = TRUE;

				if (AllowWrite && CopyFile(CheckPath, DestPath, FALSE))
				{
					DoomWAD = TRUE;
					Attempt = FALSE;
					FoundWAD = TRUE;
				}
				else if (AllowWrite)
					MessageBox(WizardWindow, TEXT("Copy of doom.wad failed!"), TEXT("Installation Failure"), MB_OK | MB_ICONWARNING);
			}
		}

		if (!Doom2WAD)		// D:\Setup\Doom2
		{
			SNPrintf(CheckPath, MAX_PATH, TEXT("%s\\Setup\\Doom2\\doom2.wad"), ActiveDrive);
			SNPrintf(DestPath, MAX_PATH, TEXT("%s\\doom2.wad"), ((InstallPath && StrLen(InstallPath)) ? InstallPath : TEXT(".")));
			if (GetFileAttributes(CheckPath) != INVALID_FILE_ATTRIBUTES)
			{
				if (GetFileAttributes(DestPath) != INVALID_FILE_ATTRIBUTES)
				{
					if (MessageBox(WizardWindow, TEXT("Would you like to overwrite doom2.wad?"), TEXT("Overwrite?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
						AllowWrite = TRUE;
					else
						AllowWrite = FALSE;
				}
				else
					AllowWrite = TRUE;

				if (AllowWrite && CopyFile(CheckPath, DestPath, FALSE))
				{
					Doom2WAD = TRUE;
					Attempt = FALSE;
					FoundWAD = TRUE;
				}
				else if (AllowWrite)
					MessageBox(WizardWindow, TEXT("Copy of doom2.wad failed!"), TEXT("Installation Failure"), MB_OK | MB_ICONWARNING);
			}
		}

		if (!TNTWAD)		// D:\Setup\Final Doom
		{
			SNPrintf(CheckPath, MAX_PATH, TEXT("%s\\Setup\\Final Doom\\tnt.wad"), ActiveDrive);
			SNPrintf(DestPath, MAX_PATH, TEXT("%s\\tnt.wad"), ((InstallPath && StrLen(InstallPath)) ? InstallPath : TEXT(".")));
			if (GetFileAttributes(CheckPath) != INVALID_FILE_ATTRIBUTES)
			{
				if (GetFileAttributes(DestPath) != INVALID_FILE_ATTRIBUTES)
				{
					if (MessageBox(WizardWindow, TEXT("Would you like to overwrite tnt.wad?"), TEXT("Overwrite?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
						AllowWrite = TRUE;
					else
						AllowWrite = FALSE;
				}
				else
					AllowWrite = TRUE;

				if (AllowWrite && CopyFile(CheckPath, DestPath, FALSE))
				{
					TNTWAD = TRUE;
					Attempt = FALSE;
					FoundWAD = TRUE;
				}
				else if (AllowWrite)
					MessageBox(WizardWindow, TEXT("Copy of tnt.wad failed!"), TEXT("Installation Failure"), MB_OK | MB_ICONWARNING);
			}
		}

		if (!PlutoniaWAD)	// D:\Setup\Final Doom
		{
			SNPrintf(CheckPath, MAX_PATH, TEXT("%s\\Setup\\Final Doom\\plutonia.wad"), ActiveDrive);
			SNPrintf(DestPath, MAX_PATH, TEXT("%s\\plutonia.wad"), ((InstallPath && StrLen(InstallPath)) ? InstallPath : TEXT(".")));
			if (GetFileAttributes(CheckPath) != INVALID_FILE_ATTRIBUTES)
			{
				if (GetFileAttributes(DestPath) != INVALID_FILE_ATTRIBUTES)
				{
					if (MessageBox(WizardWindow, TEXT("Would you like to overwrite plutonia.wad?"), TEXT("Overwrite?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
						AllowWrite = TRUE;
					else
						AllowWrite = FALSE;
				}
				else
					AllowWrite = TRUE;

				if (AllowWrite && CopyFile(CheckPath, DestPath, FALSE))
				{
					PlutoniaWAD = TRUE;
					Attempt = FALSE;
					FoundWAD = TRUE;
				}
				else if (AllowWrite)
					MessageBox(WizardWindow, TEXT("Copy of plutonia.wad failed!"), TEXT("Installation Failure"), MB_OK | MB_ICONWARNING);
			}
		}

		if (!FoundWAD)
		{
			ActiveDrive[0]++;
			continue;
		}
		else
			Attempt = FALSE;
	}

	// Restore old mode...
	SetErrorMode(OldErrorMode);
	return 0;
}

int Wizard_SteamInstall(TCHAR* InstallPath)
{
	MessageBox(WizardWindow, TEXT("Steam install not implemented!"), TEXT("Nothing"), MB_OK | MB_ICONERROR);
	return 0;
}

int Wizard_SharewareInstall(TCHAR* InstallPath)
{
	MessageBox(WizardWindow, TEXT("Shareware install not implemented!"), TEXT("Nothing"), MB_OK | MB_ICONERROR);
	return 0;
}

int Wizard_FreedoomInstall(TCHAR* InstallPath)
{
	MessageBox(WizardWindow, TEXT("Freedoom install not implemented!"), TEXT("Nothing"), MB_OK | MB_ICONERROR);
	return 0;
}

int Wizard_SearchInstall(TCHAR* InstallPath)
{
	MessageBox(WizardWindow, TEXT("Search install not implemented!"), TEXT("Nothing"), MB_OK | MB_ICONERROR);
	return 0;
}

HCURSOR Old;

void Wizard_EnterDisabledState(void)
{
	Old = SetCursor(LoadCursor(NULL, IDC_WAIT));

	EnableWindow(WizardControls[WIZ_CANCEL].Handle, FALSE);
	EnableWindow(WizardControls[WIZ_PREV].Handle, FALSE);
	EnableWindow(WizardControls[WIZ_NEXT].Handle, FALSE);
	EnableWindow(WizardControls[WIZ_B_WADLOCATIONNOTE].Handle, FALSE);
	EnableWindow(WizardControls[WIZ_B_IWADBOX].Handle, FALSE);
	EnableWindow(WizardControls[WIZ_B_DOWNLOADSHAREWARE].Handle, FALSE);
	EnableWindow(WizardControls[WIZ_B_FROMFLOPPIES].Handle, FALSE);
	EnableWindow(WizardControls[WIZ_B_FROMDOOM2CD].Handle, FALSE);
	EnableWindow(WizardControls[WIZ_B_FROMFINALDOOMCD].Handle, FALSE);
	EnableWindow(WizardControls[WIZ_B_FROMCOLLECTORSCD].Handle, FALSE);
	EnableWindow(WizardControls[WIZ_B_FROMSTEAM].Handle, FALSE);
	EnableWindow(WizardControls[WIZ_B_DOWNLOADFREEDOM].Handle, FALSE);
	EnableWindow(WizardControls[WIZ_B_SEARCHCOMPUTER].Handle, FALSE);
	EnableWindow(WizardControls[WIZ_B_LOCATIONBOX].Handle, FALSE);
	EnableWindow(WizardControls[WIZ_B_WADNOTICE].Handle, FALSE);
	EnableWindow(WizardControls[WIZ_B_LOCATIONEDIT].Handle, FALSE);
	EnableWindow(WizardControls[WIZ_B_BROWSEBUTTON].Handle, FALSE);
}

void Wizard_EnterEnabledState(void)
{
	SetCursor(Old);

	EnableWindow(WizardControls[WIZ_CANCEL].Handle, TRUE);
	EnableWindow(WizardControls[WIZ_PREV].Handle, TRUE);
	EnableWindow(WizardControls[WIZ_NEXT].Handle, TRUE);
	EnableWindow(WizardControls[WIZ_B_WADLOCATIONNOTE].Handle, TRUE);
	EnableWindow(WizardControls[WIZ_B_IWADBOX].Handle, TRUE);
	EnableWindow(WizardControls[WIZ_B_DOWNLOADSHAREWARE].Handle, FALSE); // TODO
	EnableWindow(WizardControls[WIZ_B_FROMFLOPPIES].Handle, FALSE); // TODO
	EnableWindow(WizardControls[WIZ_B_FROMDOOM2CD].Handle, TRUE);
	EnableWindow(WizardControls[WIZ_B_FROMFINALDOOMCD].Handle, TRUE);
	EnableWindow(WizardControls[WIZ_B_FROMCOLLECTORSCD].Handle, TRUE);
	EnableWindow(WizardControls[WIZ_B_FROMSTEAM].Handle, FALSE); // TODO
	EnableWindow(WizardControls[WIZ_B_DOWNLOADFREEDOM].Handle, FALSE); // TODO
	EnableWindow(WizardControls[WIZ_B_SEARCHCOMPUTER].Handle, FALSE); // TODO
	EnableWindow(WizardControls[WIZ_B_LOCATIONBOX].Handle, TRUE);
	EnableWindow(WizardControls[WIZ_B_WADNOTICE].Handle, TRUE);
	EnableWindow(WizardControls[WIZ_B_LOCATIONEDIT].Handle, TRUE);
	EnableWindow(WizardControls[WIZ_B_BROWSEBUTTON].Handle, TRUE);
}

/*******************************************************************************
**************************** WINDOWS INTERFACE CODE ****************************
*******************************************************************************/

LRESULT CALLBACK WizardProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	static HFONT hFont = NULL;
	static int i, j, k;
	static int OldErrorMode = 0;
	static int Step;
	static BROWSEINFO BrowseInfo;
	static OPENFILENAME Ofn;
	static TCHAR PathBuffer[MAX_PATH];
	static TCHAR CheckPath[MAX_PATH];
	static BOOL OKToInstall;
	static BOOL ReallyOKToInstall;
	static BOOL IgnoreBad;
	static HKEY RegistryKey;
	
	switch (Msg)
	{
		/*** WINDOW CREATION ***/
		case WM_CREATE:
			/* Use the GUI Font, whatever that may be */
			hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			
			/* Create Controls from loop */
			for (i = 0; i < NUMWIZARDCONTROLS; i++)
			{
				WizardControls[i].Handle = CreateWindowEx(
					WizardControls[i].ExStyle,
					WizardControls[i].ClassName,
					WizardControls[i].WindowName,
					WizardControls[i].Style,
					WizardControls[i].x,
					WizardControls[i].y,
					WizardControls[i].Width,
					WizardControls[i].Height,
					hWnd,
					(HMENU)WizardControls[i].identity,
					hInst,
					NULL
					);
				
				if (WizardControls[i].Handle)
					SendMessage(WizardControls[i].Handle, WM_SETFONT, (WPARAM)hFont, (LPARAM)LOWORD(TRUE));
			}
			
			SendMessage(WizardControls[WIZ_A_GPLTEXT].Handle, WM_SETTEXT, 0, (LPARAM)GPLLicense); 
			
			/* Attempt to determine ReMooD Location */
			i = MAX_PATH;
			if (RegOpenKey(
				HKEY_LOCAL_MACHINE,
				TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\remood.exe"),
				&RegistryKey) == ERROR_SUCCESS)
			{
				if (RegQueryValueEx(
					RegistryKey,
					TEXT(""),
					NULL,
					NULL,
					&PathBuffer,
					&i) == ERROR_SUCCESS)
				{
					SendMessage(WizardControls[WIZ_C_LOCATIONEDIT].Handle, WM_SETTEXT, 0, (LPARAM)PathBuffer);
					j = StrLen(PathBuffer);
					PathBuffer[j - 1] = 'd';
					PathBuffer[j - 2] = 'a';
					PathBuffer[j - 3] = 'w';
					SendMessage(WizardControls[WIZ_C_LOCATIONEDIT2].Handle, WM_SETTEXT, 0, (LPARAM)PathBuffer);
				}
					
				RegCloseKey(RegistryKey);
			}
			break;
			
		/*** WINDOW COMMAND ***/
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case WIZ_PREV:
					switch (Step)
					{
						case 0:
							break;
							
						case 1:
							OptAgreedToGPL = FALSE;
							
							SendMessage(WizardControls[WIZ_CANCEL].Handle, WM_SETTEXT, 0, (LPARAM)TEXT("I Disagree"));
							SendMessage(WizardControls[WIZ_NEXT].Handle, WM_SETTEXT, 0, (LPARAM)TEXT("I Agree"));
							
							ShowWindow(WizardControls[WIZ_PREV].Handle, SW_HIDE);
							ShowWindow(WizardControls[WIZ_A_GPLTEXT].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_A_AGREETEXT].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_WADLOCATIONNOTE].Handle, SW_HIDE);
							ShowWindow(WizardControls[WIZ_B_IWADBOX].Handle, SW_HIDE);
							ShowWindow(WizardControls[WIZ_B_DOWNLOADSHAREWARE].Handle, SW_HIDE);
							ShowWindow(WizardControls[WIZ_B_FROMFLOPPIES].Handle, SW_HIDE);
							ShowWindow(WizardControls[WIZ_B_FROMDOOM2CD].Handle, SW_HIDE);
							ShowWindow(WizardControls[WIZ_B_FROMFINALDOOMCD].Handle, SW_HIDE);
							ShowWindow(WizardControls[WIZ_B_FROMCOLLECTORSCD].Handle, SW_HIDE);
							ShowWindow(WizardControls[WIZ_B_FROMSTEAM].Handle, SW_HIDE);
							ShowWindow(WizardControls[WIZ_B_DOWNLOADFREEDOM].Handle, SW_HIDE);
							ShowWindow(WizardControls[WIZ_B_SEARCHCOMPUTER].Handle, SW_HIDE);
							ShowWindow(WizardControls[WIZ_B_LOCATIONBOX].Handle, SW_HIDE);
							ShowWindow(WizardControls[WIZ_B_WADNOTICE].Handle, SW_HIDE);
							ShowWindow(WizardControls[WIZ_B_LOCATIONEDIT].Handle, SW_HIDE);
							ShowWindow(WizardControls[WIZ_B_BROWSEBUTTON].Handle, SW_HIDE);
							
							EnableWindow(WizardControls[WIZ_PREV].Handle, FALSE);
							EnableWindow(WizardControls[WIZ_A_GPLTEXT].Handle, TRUE);
							EnableWindow(WizardControls[WIZ_A_AGREETEXT].Handle, TRUE);
							EnableWindow(WizardControls[WIZ_B_WADLOCATIONNOTE].Handle, FALSE);
							EnableWindow(WizardControls[WIZ_B_IWADBOX].Handle, FALSE);
							EnableWindow(WizardControls[WIZ_B_DOWNLOADSHAREWARE].Handle, FALSE);
							EnableWindow(WizardControls[WIZ_B_FROMFLOPPIES].Handle, FALSE);
							EnableWindow(WizardControls[WIZ_B_FROMDOOM2CD].Handle, FALSE);
							EnableWindow(WizardControls[WIZ_B_FROMFINALDOOMCD].Handle, FALSE);
							EnableWindow(WizardControls[WIZ_B_FROMCOLLECTORSCD].Handle, FALSE);
							EnableWindow(WizardControls[WIZ_B_FROMSTEAM].Handle, FALSE);
							EnableWindow(WizardControls[WIZ_B_DOWNLOADFREEDOM].Handle, FALSE);
							EnableWindow(WizardControls[WIZ_B_SEARCHCOMPUTER].Handle, FALSE);
							EnableWindow(WizardControls[WIZ_B_LOCATIONBOX].Handle, FALSE);
							EnableWindow(WizardControls[WIZ_B_WADNOTICE].Handle, FALSE);
							EnableWindow(WizardControls[WIZ_B_LOCATIONEDIT].Handle, FALSE);
							EnableWindow(WizardControls[WIZ_B_BROWSEBUTTON].Handle, FALSE);
							
							UpdateWindow(hWnd);
							
							Step--;
							break;

						case 2:
							ShowWindow(WizardControls[WIZ_C_LOCATIONEDIT].Handle, SW_HIDE);
							ShowWindow(WizardControls[WIZ_C_BROWSEBUTTON].Handle, SW_HIDE);
							ShowWindow(WizardControls[WIZ_C_FINALNOTE].Handle, SW_HIDE);
							EnableWindow(WizardControls[WIZ_C_LOCATIONEDIT].Handle, FALSE);
							EnableWindow(WizardControls[WIZ_C_BROWSEBUTTON].Handle, FALSE);
							EnableWindow(WizardControls[WIZ_C_FINALNOTE].Handle, FALSE);
							ShowWindow(WizardControls[WIZ_C_LOCATIONEDIT2].Handle, SW_HIDE);
							ShowWindow(WizardControls[WIZ_C_BROWSEBUTTON2].Handle, SW_HIDE);
							ShowWindow(WizardControls[WIZ_C_FINALNOTE2].Handle, SW_HIDE);
							EnableWindow(WizardControls[WIZ_C_LOCATIONEDIT2].Handle, FALSE);
							EnableWindow(WizardControls[WIZ_C_BROWSEBUTTON2].Handle, FALSE);
							EnableWindow(WizardControls[WIZ_C_FINALNOTE2].Handle, FALSE);
							
							SendMessage(WizardControls[WIZ_NEXT].Handle, WM_SETTEXT, 0, (LPARAM)TEXT("Next >"));
							ShowWindow(WizardControls[WIZ_B_WADLOCATIONNOTE].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_IWADBOX].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_DOWNLOADSHAREWARE].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_FROMFLOPPIES].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_FROMDOOM2CD].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_FROMFINALDOOMCD].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_FROMCOLLECTORSCD].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_FROMSTEAM].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_DOWNLOADFREEDOM].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_SEARCHCOMPUTER].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_LOCATIONBOX].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_WADNOTICE].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_LOCATIONEDIT].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_BROWSEBUTTON].Handle, SW_SHOW);
							EnableWindow(WizardControls[WIZ_B_WADLOCATIONNOTE].Handle, TRUE);
							EnableWindow(WizardControls[WIZ_B_IWADBOX].Handle, TRUE);
							EnableWindow(WizardControls[WIZ_B_DOWNLOADSHAREWARE].Handle, FALSE); // TODO
							EnableWindow(WizardControls[WIZ_B_FROMFLOPPIES].Handle, FALSE); // TODO
							EnableWindow(WizardControls[WIZ_B_FROMDOOM2CD].Handle, TRUE);
							EnableWindow(WizardControls[WIZ_B_FROMFINALDOOMCD].Handle, TRUE);
							EnableWindow(WizardControls[WIZ_B_FROMCOLLECTORSCD].Handle, TRUE);
							EnableWindow(WizardControls[WIZ_B_FROMSTEAM].Handle, FALSE); // TODO
							EnableWindow(WizardControls[WIZ_B_DOWNLOADFREEDOM].Handle, FALSE); // TODO
							EnableWindow(WizardControls[WIZ_B_SEARCHCOMPUTER].Handle, FALSE); // TODO
							EnableWindow(WizardControls[WIZ_B_LOCATIONBOX].Handle, TRUE);
							EnableWindow(WizardControls[WIZ_B_WADNOTICE].Handle, TRUE);
							EnableWindow(WizardControls[WIZ_B_LOCATIONEDIT].Handle, TRUE);
							EnableWindow(WizardControls[WIZ_B_BROWSEBUTTON].Handle, TRUE);
							Step--;
							break;
						
						default:
							break;
					}
					break;
					
				case WIZ_NEXT:
					switch (Step)
					{
						case 0:
							OptAgreedToGPL = TRUE;
							
							SendMessage(WizardControls[WIZ_CANCEL].Handle, WM_SETTEXT, 0, (LPARAM)TEXT("Cancel"));
							SendMessage(WizardControls[WIZ_PREV].Handle, WM_SETTEXT, 0, (LPARAM)TEXT("< Previous"));
							SendMessage(WizardControls[WIZ_NEXT].Handle, WM_SETTEXT, 0, (LPARAM)TEXT("Next >"));
							
							ShowWindow(WizardControls[WIZ_PREV].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_A_GPLTEXT].Handle, SW_HIDE);
							ShowWindow(WizardControls[WIZ_A_AGREETEXT].Handle, SW_HIDE);
							ShowWindow(WizardControls[WIZ_B_WADLOCATIONNOTE].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_IWADBOX].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_DOWNLOADSHAREWARE].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_FROMFLOPPIES].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_FROMDOOM2CD].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_FROMFINALDOOMCD].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_FROMCOLLECTORSCD].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_FROMSTEAM].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_DOWNLOADFREEDOM].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_SEARCHCOMPUTER].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_LOCATIONBOX].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_WADNOTICE].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_LOCATIONEDIT].Handle, SW_SHOW);
							ShowWindow(WizardControls[WIZ_B_BROWSEBUTTON].Handle, SW_SHOW);
							
							EnableWindow(WizardControls[WIZ_PREV].Handle, TRUE);
							EnableWindow(WizardControls[WIZ_A_GPLTEXT].Handle, FALSE);
							EnableWindow(WizardControls[WIZ_A_AGREETEXT].Handle, FALSE);
							EnableWindow(WizardControls[WIZ_B_WADLOCATIONNOTE].Handle, TRUE);
							EnableWindow(WizardControls[WIZ_B_IWADBOX].Handle, TRUE);
							EnableWindow(WizardControls[WIZ_B_DOWNLOADSHAREWARE].Handle, FALSE); // TODO
							EnableWindow(WizardControls[WIZ_B_FROMFLOPPIES].Handle, FALSE); // TODO
							EnableWindow(WizardControls[WIZ_B_FROMDOOM2CD].Handle, TRUE);
							EnableWindow(WizardControls[WIZ_B_FROMFINALDOOMCD].Handle, TRUE);
							EnableWindow(WizardControls[WIZ_B_FROMCOLLECTORSCD].Handle, TRUE);
							EnableWindow(WizardControls[WIZ_B_FROMSTEAM].Handle, FALSE); // TODO
							EnableWindow(WizardControls[WIZ_B_DOWNLOADFREEDOM].Handle, FALSE); // TODO
							EnableWindow(WizardControls[WIZ_B_SEARCHCOMPUTER].Handle, FALSE); // TODO
							EnableWindow(WizardControls[WIZ_B_LOCATIONBOX].Handle, TRUE);
							EnableWindow(WizardControls[WIZ_B_WADNOTICE].Handle, TRUE);
							EnableWindow(WizardControls[WIZ_B_LOCATIONEDIT].Handle, TRUE);
							EnableWindow(WizardControls[WIZ_B_BROWSEBUTTON].Handle, TRUE);
							
							UpdateWindow(hWnd);
							
							Step++;
							break;
						
						case 1:
							SendMessage(WizardControls[WIZ_B_LOCATIONEDIT].Handle, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)PathBuffer);
							OKToInstall = TRUE;
							ReallyOKToInstall = FALSE;
							IgnoreBad = FALSE;
							
							if (!StrLen(PathBuffer))
							{
								if (MessageBox(hWnd, TEXT("You have specified an empty WAD Directory. Please note that without this option you might not be able to correctly load WAD files, it is highly recommended that you fill in this option. Would you like to continue anyway?"), TEXT("Empty WAD Directory"), MB_YESNO | MB_ICONWARNING) == IDNO)
									OKToInstall = FALSE;
								else
									IgnoreBad = TRUE;
							}
							
							if (OKToInstall)
							{
								// Check to see if the path is valid...
								i = GetFileAttributes(PathBuffer);
								
								if (!IgnoreBad && i == INVALID_FILE_ATTRIBUTES || !(i & FILE_ATTRIBUTE_DIRECTORY))
									MessageBox(hWnd, TEXT("The path you specified either does not exist or is not a directory, please try again."), TEXT("Invalid Directory"), MB_OK | MB_ICONERROR);
								else
									ReallyOKToInstall = TRUE;
							}
							
							if (ReallyOKToInstall)
							{
								/*** FLOPPY DISK INSTALL ***/
								if (!DoomWAD || !Doom2WAD || !TNTWAD || !PlutoniaWAD)
									if (IsDlgButtonChecked(hWnd, WIZ_B_FROMFLOPPIES) == BST_CHECKED)
									{
										Wizard_EnterDisabledState();
										if (Wizard_FloppyInstall(PathBuffer) == 3)
										{
											Wizard_EnterEnabledState();
											return 0;
										}
										Wizard_EnterEnabledState();
									}
								
								/*** DOOM 2 CD ***/
								if (!Doom2WAD)
									if (IsDlgButtonChecked(hWnd, WIZ_B_FROMDOOM2CD) == BST_CHECKED)
									{
										Wizard_EnterDisabledState();
										if (Wizard_Doom2Install(PathBuffer) == 3)
										{
											Wizard_EnterEnabledState();
											return 0;
										}
										Wizard_EnterEnabledState();
									}
								
								/*** BLAH ***/
								if (!TNTWAD || !PlutoniaWAD)
									if (IsDlgButtonChecked(hWnd, WIZ_B_FROMFINALDOOMCD) == BST_CHECKED)
									{
										Wizard_EnterDisabledState();
										if (Wizard_FinalDoomInstall(PathBuffer) == 3)
										{
											Wizard_EnterEnabledState();
											return 0;
										}
										Wizard_EnterEnabledState();
									}
								
								/*** BLAH ***/
								if (!DoomWAD || !Doom2WAD || !TNTWAD || !PlutoniaWAD)
									if (IsDlgButtonChecked(hWnd, WIZ_B_FROMCOLLECTORSCD) == BST_CHECKED)
									{
										Wizard_EnterDisabledState();
										if (Wizard_CollectorsInstall(PathBuffer) == 3)
										{
											Wizard_EnterEnabledState();
											return 0;
										}
										Wizard_EnterEnabledState();
									}
								
								/*** BLAH ***/
								if (!DoomWAD || !Doom2WAD || !TNTWAD || !PlutoniaWAD)
									if (IsDlgButtonChecked(hWnd, WIZ_B_FROMSTEAM) == BST_CHECKED)
									{
										Wizard_EnterDisabledState();
										if (Wizard_SteamInstall(PathBuffer) == 3)
										{
											Wizard_EnterEnabledState();
											return 0;
										}
										Wizard_EnterEnabledState();
									}
								
								/*** BLAH ***/
								if (!SharewareWAD)
									if (IsDlgButtonChecked(hWnd, WIZ_B_DOWNLOADSHAREWARE) == BST_CHECKED)
									{
										Wizard_EnterDisabledState();
										if (Wizard_SharewareInstall(PathBuffer) == 3)
										{
											Wizard_EnterEnabledState();
											return 0;
										}
										Wizard_EnterEnabledState();
									}
								
								/*** Download Freedoom ***/
								if (!FreedoomWAD || !Doom2WAD)
									if (IsDlgButtonChecked(hWnd, WIZ_B_DOWNLOADFREEDOM) == BST_CHECKED)
									{
										Wizard_EnterDisabledState();
										if (Wizard_FreedoomInstall(PathBuffer) == 3)
										{
											Wizard_EnterEnabledState();
											return 0;
										}
										Wizard_EnterEnabledState();
									}
								
								/*** SEARCH PC ***/
								if (!SharewareWAD || !DoomWAD || !Doom2WAD || !TNTWAD || !PlutoniaWAD)
									if (IsDlgButtonChecked(hWnd, WIZ_B_SEARCHCOMPUTER) == BST_CHECKED)
									{
										Wizard_EnterDisabledState();
										if (Wizard_SearchInstall(PathBuffer) == 3)
										{
											Wizard_EnterEnabledState();
											return 0;
										}
										Wizard_EnterEnabledState();
									}

								if (!FreedoomWAD && !SharewareWAD && !DoomWAD && !Doom2WAD && !TNTWAD && !PlutoniaWAD)
									if (MessageBox(hWnd, TEXT("The ReMooD Setup Wizard has determined that you do not have any IWADs, would you like to continue ignoring this error?"), TEXT("Missing IWADs"), MB_YESNO | MB_ICONQUESTION) == IDNO)
										return 0;
										
								StrCpy(WADPath, PathBuffer);
								
								SendMessage(WizardControls[WIZ_NEXT].Handle, WM_SETTEXT, 0, (LPARAM)TEXT("Finish"));
								
								ShowWindow(WizardControls[WIZ_C_LOCATIONEDIT].Handle, SW_SHOW);
								ShowWindow(WizardControls[WIZ_C_BROWSEBUTTON].Handle, SW_SHOW);
								ShowWindow(WizardControls[WIZ_C_FINALNOTE].Handle, SW_SHOW);
								EnableWindow(WizardControls[WIZ_C_LOCATIONEDIT].Handle, TRUE);
								EnableWindow(WizardControls[WIZ_C_BROWSEBUTTON].Handle, TRUE);
								EnableWindow(WizardControls[WIZ_C_FINALNOTE].Handle, TRUE);
								ShowWindow(WizardControls[WIZ_C_LOCATIONEDIT2].Handle, SW_SHOW);
								ShowWindow(WizardControls[WIZ_C_BROWSEBUTTON2].Handle, SW_SHOW);
								ShowWindow(WizardControls[WIZ_C_FINALNOTE2].Handle, SW_SHOW);
								EnableWindow(WizardControls[WIZ_C_LOCATIONEDIT2].Handle, TRUE);
								EnableWindow(WizardControls[WIZ_C_BROWSEBUTTON2].Handle, TRUE);
								EnableWindow(WizardControls[WIZ_C_FINALNOTE2].Handle, TRUE);

								ShowWindow(WizardControls[WIZ_B_WADLOCATIONNOTE].Handle, SW_HIDE);
								ShowWindow(WizardControls[WIZ_B_IWADBOX].Handle, SW_HIDE);
								ShowWindow(WizardControls[WIZ_B_DOWNLOADSHAREWARE].Handle, SW_HIDE);
								ShowWindow(WizardControls[WIZ_B_FROMFLOPPIES].Handle, SW_HIDE);
								ShowWindow(WizardControls[WIZ_B_FROMDOOM2CD].Handle, SW_HIDE);
								ShowWindow(WizardControls[WIZ_B_FROMFINALDOOMCD].Handle, SW_HIDE);
								ShowWindow(WizardControls[WIZ_B_FROMCOLLECTORSCD].Handle, SW_HIDE);
								ShowWindow(WizardControls[WIZ_B_FROMSTEAM].Handle, SW_HIDE);
								ShowWindow(WizardControls[WIZ_B_DOWNLOADFREEDOM].Handle, SW_HIDE);
								ShowWindow(WizardControls[WIZ_B_SEARCHCOMPUTER].Handle, SW_HIDE);
								ShowWindow(WizardControls[WIZ_B_LOCATIONBOX].Handle, SW_HIDE);
								ShowWindow(WizardControls[WIZ_B_WADNOTICE].Handle, SW_HIDE);
								ShowWindow(WizardControls[WIZ_B_LOCATIONEDIT].Handle, SW_HIDE);
								ShowWindow(WizardControls[WIZ_B_BROWSEBUTTON].Handle, SW_HIDE);
								EnableWindow(WizardControls[WIZ_B_WADLOCATIONNOTE].Handle, FALSE);
								EnableWindow(WizardControls[WIZ_B_IWADBOX].Handle, FALSE);
								EnableWindow(WizardControls[WIZ_B_DOWNLOADSHAREWARE].Handle, FALSE);
								EnableWindow(WizardControls[WIZ_B_FROMFLOPPIES].Handle, FALSE);
								EnableWindow(WizardControls[WIZ_B_FROMDOOM2CD].Handle, FALSE);
								EnableWindow(WizardControls[WIZ_B_FROMFINALDOOMCD].Handle, FALSE);
								EnableWindow(WizardControls[WIZ_B_FROMCOLLECTORSCD].Handle, FALSE);
								EnableWindow(WizardControls[WIZ_B_FROMSTEAM].Handle, FALSE);
								EnableWindow(WizardControls[WIZ_B_DOWNLOADFREEDOM].Handle, FALSE);
								EnableWindow(WizardControls[WIZ_B_SEARCHCOMPUTER].Handle, FALSE);
								EnableWindow(WizardControls[WIZ_B_LOCATIONBOX].Handle, FALSE);
								EnableWindow(WizardControls[WIZ_B_WADNOTICE].Handle, FALSE);
								EnableWindow(WizardControls[WIZ_B_LOCATIONEDIT].Handle, FALSE);
								EnableWindow(WizardControls[WIZ_B_BROWSEBUTTON].Handle, FALSE);
								Step++;
							}
							break;
						
						case 2:
							/*** EXE ***/
							SendMessage(WizardControls[WIZ_C_LOCATIONEDIT].Handle, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)PathBuffer);
							OKToInstall = TRUE;
							ReallyOKToInstall = FALSE;
							IgnoreBad = FALSE;
						
							if (!StrLen(PathBuffer))
							{
								if (MessageBox(hWnd, TEXT("You have not specified the location of the ReMooD Executable, without this option ReMooD may not work correctly. Would you like to continue anyway?"), TEXT("Empty ReMooD Executable"), MB_YESNO | MB_ICONWARNING) == IDNO)
									OKToInstall = FALSE;
								else
									IgnoreBad = TRUE;
							}
						
							if (OKToInstall)
							{
								// Check to see if the path is valid...
								i = GetFileAttributes(PathBuffer);
							
								if (!IgnoreBad && i == INVALID_FILE_ATTRIBUTES)
									MessageBox(hWnd, TEXT("The file you specified does not exist."), TEXT("Invalid File"), MB_OK | MB_ICONERROR);
								else
									ReallyOKToInstall = TRUE;
							}
							
							if (ReallyOKToInstall)
							{
								StrCpy(ProgramDir, PathBuffer);
								
								/*** WAD ***/
								SendMessage(WizardControls[WIZ_C_LOCATIONEDIT2].Handle, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)PathBuffer);
								OKToInstall = TRUE;
								ReallyOKToInstall = FALSE;
								IgnoreBad = FALSE;
						
								if (!StrLen(PathBuffer))
								{
									if (MessageBox(hWnd, TEXT("You have not specified the location of the ReMooD WAD, without this option ReMooD may not work correctly. Would you like to continue anyway?"), TEXT("Empty ReMooD WAD"), MB_YESNO | MB_ICONWARNING) == IDNO)
										OKToInstall = FALSE;
									else
										IgnoreBad = TRUE;
								}
						
								if (OKToInstall)
								{
									// Check to see if the path is valid...
									i = GetFileAttributes(PathBuffer);
							
									if (!IgnoreBad && i == INVALID_FILE_ATTRIBUTES)
										MessageBox(hWnd, TEXT("The file you specified does not exist."), TEXT("Invalid File"), MB_OK | MB_ICONERROR);
									else
										ReallyOKToInstall = TRUE;
								}
							
								if (ReallyOKToInstall)
								{
									StrCpy(ReMooDWAD, PathBuffer);
									SendMessage(hWnd, WM_CLOSE, 0, (LPARAM)1);
								}
							}
							
							break;
						
						default:
							break;
					}
					break;
					
				case WIZ_CANCEL:
					SendMessage(hWnd, WM_CLOSE, 0, 0);
					break;
					
				/*****************************************/
				/*****************************************/
				/*****************************************/
					
				case WIZ_B_FROMFLOPPIES:
				case WIZ_B_FROMDOOM2CD:
				case WIZ_B_FROMFINALDOOMCD:
				case WIZ_B_FROMCOLLECTORSCD:
				case WIZ_B_FROMSTEAM:
				case WIZ_B_DOWNLOADSHAREWARE:
				case WIZ_B_DOWNLOADFREEDOM:
				case WIZ_B_SEARCHCOMPUTER:
					// Toggle
					if (IsDlgButtonChecked(hWnd, LOWORD(wParam)) == BST_CHECKED)
						CheckDlgButton(hWnd, LOWORD(wParam), BST_UNCHECKED);
					else
						CheckDlgButton(hWnd, LOWORD(wParam), BST_CHECKED);
					break;
					
				case WIZ_B_BROWSEBUTTON:
					SendMessage(WizardControls[WIZ_B_LOCATIONEDIT].Handle, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)PathBuffer);
					
					memset(&BrowseInfo, 0, sizeof(BrowseInfo));
					BrowseInfo.hwndOwner = hWnd;
					BrowseInfo.pidlRoot = NULL;
					BrowseInfo.pszDisplayName = PathBuffer;
					BrowseInfo.lpszTitle = TEXT("Please select a folder where WADs are located");
					BrowseInfo.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_EDITBOX | BIF_VALIDATE;
					BrowseInfo.lpfn = NULL;
					BrowseInfo.lParam = 0;
					BrowseInfo.iImage = 0;
					
					if (SHGetPathFromIDList(SHBrowseForFolder(&BrowseInfo), PathBuffer))
					{
						SendMessage(WizardControls[WIZ_B_LOCATIONEDIT].Handle, WM_SETTEXT, 0, (LPARAM)PathBuffer);

						DoomWAD = FALSE;
						Doom2WAD = FALSE;
						TNTWAD = FALSE;
						PlutoniaWAD = FALSE;
						SharewareWAD = FALSE;
						FreedoomWAD = FALSE;

						// Check existance of WAD Files
						SNPrintf(CheckPath, MAX_PATH, TEXT("%s\\doom.wad"), PathBuffer);
						if (GetFileAttributes(CheckPath) != INVALID_FILE_ATTRIBUTES)
							DoomWAD = TRUE;
						SNPrintf(CheckPath, MAX_PATH, TEXT("%s\\doom2.wad"), PathBuffer);
						if (GetFileAttributes(CheckPath) != INVALID_FILE_ATTRIBUTES)
							Doom2WAD = TRUE;
						SNPrintf(CheckPath, MAX_PATH, TEXT("%s\\tnt.wad"), PathBuffer);
						if (GetFileAttributes(CheckPath) != INVALID_FILE_ATTRIBUTES)
							TNTWAD = TRUE;
						SNPrintf(CheckPath, MAX_PATH, TEXT("%s\\plutonia.wad"), PathBuffer);
						if (GetFileAttributes(CheckPath) != INVALID_FILE_ATTRIBUTES)
							PlutoniaWAD = TRUE;
						SNPrintf(CheckPath, MAX_PATH, TEXT("%s\\tnt.wad"), PathBuffer);
						if (GetFileAttributes(CheckPath) != INVALID_FILE_ATTRIBUTES)
							TNTWAD = TRUE;
						SNPrintf(CheckPath, MAX_PATH, TEXT("%s\\doom1.wad"), PathBuffer);
						if (GetFileAttributes(CheckPath) != INVALID_FILE_ATTRIBUTES)
							SharewareWAD = TRUE;
						SNPrintf(CheckPath, MAX_PATH, TEXT("%s\\freedoom.wad"), PathBuffer);
						if (GetFileAttributes(CheckPath) != INVALID_FILE_ATTRIBUTES)
							FreedoomWAD = TRUE;
					}
					break;
					
				/*****************************************/
				/*****************************************/
				/*****************************************/
				
				case WIZ_C_BROWSEBUTTON:
					SendMessage(WizardControls[WIZ_C_LOCATIONEDIT].Handle, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)PathBuffer);
					
					memset(&Ofn, 0, sizeof(Ofn));
					Ofn.lStructSize = sizeof(Ofn);
					Ofn.hwndOwner = hWnd;
					Ofn.hInstance = hInst;
					Ofn.lpstrFilter = TEXT("Executables (*.exe)\0*.exe\0All Files (*.*)\0*.*\0\0");
					Ofn.lpstrCustomFilter = NULL;
					Ofn.nMaxCustFilter = 0;
					Ofn.nFilterIndex = 0;
					Ofn.lpstrFile = PathBuffer;
					Ofn.nMaxFile = MAX_PATH;
					Ofn.lpstrFileTitle = NULL;
					Ofn.nMaxFileTitle = NULL;
					Ofn.lpstrInitialDir = NULL;
					Ofn.lpstrTitle = TEXT("Please locate ReMooD.Exe");
					Ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
					
					if (GetOpenFileName(&Ofn))
						SendMessage(WizardControls[WIZ_C_LOCATIONEDIT].Handle, WM_SETTEXT, 0, (LPARAM)Ofn.lpstrFile);
					break;
					
				case WIZ_C_BROWSEBUTTON2:
					SendMessage(WizardControls[WIZ_C_LOCATIONEDIT2].Handle, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)PathBuffer);
					
					memset(&Ofn, 0, sizeof(Ofn));
					Ofn.lStructSize = sizeof(Ofn);
					Ofn.hwndOwner = hWnd;
					Ofn.hInstance = hInst;
					Ofn.lpstrFilter = TEXT("WAD Files (*.wad)\0*.wad\0All Files (*.*)\0*.*\0\0");
					Ofn.lpstrCustomFilter = NULL;
					Ofn.nMaxCustFilter = 0;
					Ofn.nFilterIndex = 0;
					Ofn.lpstrFile = PathBuffer;
					Ofn.nMaxFile = MAX_PATH;
					Ofn.lpstrFileTitle = NULL;
					Ofn.nMaxFileTitle = NULL;
					Ofn.lpstrInitialDir = NULL;
					Ofn.lpstrTitle = TEXT("Please locate ReMooD.wad");
					Ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
					
					if (GetOpenFileName(&Ofn))
						SendMessage(WizardControls[WIZ_C_LOCATIONEDIT2].Handle, WM_SETTEXT, 0, (LPARAM)Ofn.lpstrFile);
					break;
					
				/*****************************************/
				/*****************************************/
				/*****************************************/
					
				default:
					break;
			}
			break;
			
		/*** WINDOW CLOSE ***/
		case WM_CLOSE:
			if (!Step)
			{
				if (MessageBox(hWnd, TEXT("If you do not agree to the terms of the GNU GPL then you are not legally permitted to use this software. Are you sure you want to disagree?"), TEXT("Cancel Wizard?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
					DestroyWindow(hWnd);
			}
			else if (lParam != 1)
			{
				if (MessageBox(hWnd, TEXT("If you cancel the the ReMooD Setup Wizard, the program may not function. Are you sure you want to cancel setup?"), TEXT("Cancel Wizard?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
					DestroyWindow(hWnd);
			}
			else
				DestroyWindow(hWnd);
			break;
			
		/*** WINDOW DESTROYED ***/
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
			
		/*** UNSPECIFIED ***/
		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	
	return 0;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

void DisableLauncher(void)
{
	EnableWindow(LauncherControls[LNC_LAUNCH].Handle, FALSE);
	//EnableWindow(LauncherControls[LNC_SINGLEPLAYER].Handle, FALSE);
	//EnableWindow(LauncherControls[LNC_MULTIPLAYER].Handle, FALSE);
	//EnableWindow(LauncherControls[LNC_SETTINGS].Handle, FALSE);
	EnableWindow(LauncherControls[LNC_WEBSITE].Handle, FALSE);
	EnableWindow(LauncherControls[LNC_EXIT].Handle, FALSE);
	EnableWindow(LauncherControls[LNC_IWAD].Handle, FALSE);
	EnableWindow(LauncherControls[LNC_IWADCOMBO].Handle, FALSE);
	EnableWindow(LauncherControls[LNC_PWAD].Handle, FALSE);
	EnableWindow(LauncherControls[LNC_PWADCOMBO].Handle, FALSE);
	EnableWindow(LauncherControls[LNC_PWADADD].Handle, FALSE);
	EnableWindow(LauncherControls[LNC_PWADREMOVE].Handle, FALSE);
	EnableWindow(LauncherControls[LNC_PWADUP].Handle, FALSE);
	EnableWindow(LauncherControls[LNC_PWADDOWN].Handle, FALSE);
	EnableWindow(LauncherControls[LNC_EXTRATIP].Handle, FALSE);
	EnableWindow(LauncherControls[LNC_EXTRAEDIT].Handle, FALSE);
}

void EnableLauncher(void)
{
	EnableWindow(LauncherControls[LNC_LAUNCH].Handle, TRUE);
	//EnableWindow(LauncherControls[LNC_SINGLEPLAYER].Handle, TRUE);
	//EnableWindow(LauncherControls[LNC_MULTIPLAYER].Handle, TRUE);
	//EnableWindow(LauncherControls[LNC_SETTINGS].Handle, TRUE);
	EnableWindow(LauncherControls[LNC_WEBSITE].Handle, TRUE);
	EnableWindow(LauncherControls[LNC_EXIT].Handle, TRUE);
	EnableWindow(LauncherControls[LNC_IWAD].Handle, TRUE);
	EnableWindow(LauncherControls[LNC_IWADCOMBO].Handle, TRUE);
	EnableWindow(LauncherControls[LNC_PWAD].Handle, TRUE);
	EnableWindow(LauncherControls[LNC_PWADCOMBO].Handle, TRUE);
	EnableWindow(LauncherControls[LNC_PWADADD].Handle, TRUE);
	EnableWindow(LauncherControls[LNC_PWADREMOVE].Handle, TRUE);
	EnableWindow(LauncherControls[LNC_PWADUP].Handle, TRUE);
	EnableWindow(LauncherControls[LNC_PWADDOWN].Handle, TRUE);
	EnableWindow(LauncherControls[LNC_EXTRATIP].Handle, TRUE);
	EnableWindow(LauncherControls[LNC_EXTRAEDIT].Handle, TRUE);
}

LRESULT CALLBACK LauncherProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	static HFONT hFont = NULL;
	static int i, j, k, l;
	static RECT WinRect;
	static SYSTEMTIME SysTime;
	static TCHAR Parms[2048];
	static TCHAR ExtraParms[512];
	static SHELLEXECUTEINFO Sei;
	static OPENFILENAME Ofn;
	static TCHAR PathBuffer[MAX_PATH];
	static TCHAR PathBufferZ[MAX_PATH];
	static TCHAR* x;
	
	switch (Msg)
	{
		/*** WINDOW CREATION ***/
		case WM_CREATE:
			/* Use the GUI Font, whatever that may be */
			hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			
			/* Create Controls from loop */
			for (i = 0; i < NUMLAUNCHERCONTROLS; i++)
			{
				LauncherControls[i].Handle = CreateWindowEx(
					LauncherControls[i].ExStyle,
					LauncherControls[i].ClassName,
					LauncherControls[i].WindowName,
					LauncherControls[i].Style,
					LauncherControls[i].x,
					LauncherControls[i].y,
					LauncherControls[i].Width,
					LauncherControls[i].Height,
					hWnd,
					(HMENU)LauncherControls[i].identity,
					hInst,
					NULL
					);
				
				if (LauncherControls[i].Handle)
					SendMessage(LauncherControls[i].Handle, WM_SETFONT, (WPARAM)hFont, (LPARAM)LOWORD(TRUE));
			}
			
			/* Add IWAD Names */
			SendMessage(LauncherControls[LNC_IWADCOMBO].Handle, CB_ADDSTRING, NULL, (LPARAM)TEXT("Doom Shareware"));
			SendMessage(LauncherControls[LNC_IWADCOMBO].Handle, CB_ADDSTRING, NULL, (LPARAM)TEXT("Doom Registered/The Ultimate Doom"));
			SendMessage(LauncherControls[LNC_IWADCOMBO].Handle, CB_ADDSTRING, NULL, (LPARAM)TEXT("Doom 2: Hell on Earth"));
			SendMessage(LauncherControls[LNC_IWADCOMBO].Handle, CB_ADDSTRING, NULL, (LPARAM)TEXT("TNT: Evilution"));
			SendMessage(LauncherControls[LNC_IWADCOMBO].Handle, CB_ADDSTRING, NULL, (LPARAM)TEXT("The Plutonia Experiment"));
			SendMessage(LauncherControls[LNC_IWADCOMBO].Handle, CB_ADDSTRING, NULL, (LPARAM)TEXT("Freedoom"));
			SendMessage(LauncherControls[LNC_IWADCOMBO].Handle, CB_SETCURSEL, (WPARAM)2, NULL);
			break;
			
		/*** WINDOW COMMAND ***/
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case LNC_LAUNCH:
					// Gather Parameters
					memset(Parms, 0, sizeof(Parms));
					
					SNPrintf(Parms, sizeof(Parms) / sizeof(TCHAR),
						TEXT("-waddir \"%s\" "),
						WADPath);
						
					// What IWAD?
					SNPrintf(Parms, sizeof(Parms) / sizeof(TCHAR), TEXT("%s-iwad "), Parms);
					switch (SendMessage(LauncherControls[LNC_IWADCOMBO].Handle, CB_GETCURSEL, NULL, NULL))
					{
						case 0: SNPrintf(Parms, sizeof(Parms) / sizeof(TCHAR), TEXT("%sdoom1.wad"), Parms); break;
						case 1: SNPrintf(Parms, sizeof(Parms) / sizeof(TCHAR), TEXT("%sdoom.wad"), Parms); break;
						case 2: SNPrintf(Parms, sizeof(Parms) / sizeof(TCHAR), TEXT("%sdoom2.wad"), Parms); break;
						case 3: SNPrintf(Parms, sizeof(Parms) / sizeof(TCHAR), TEXT("%stnt.wad"), Parms); break;
						case 4: SNPrintf(Parms, sizeof(Parms) / sizeof(TCHAR), TEXT("%splutonia.wad"), Parms); break;
						case 5: SNPrintf(Parms, sizeof(Parms) / sizeof(TCHAR), TEXT("%sfreedoom.wad"), Parms); break;
						default:SNPrintf(Parms, sizeof(Parms) / sizeof(TCHAR), TEXT("%sdoom2.wad"), Parms); break;
					}
					
					// ReMooD WAD
					SNPrintf(Parms, sizeof(Parms) / sizeof(TCHAR), TEXT("%s -remoodwad \"%s\""), Parms, ReMooDWAD);
					
					// PWADs?
					j = SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
					if (j)
					{
						SNPrintf(Parms, sizeof(Parms) / sizeof(TCHAR), TEXT("%s -file "), Parms);
						
						for (i = 0; i < j; i++)
						{
							k = SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_GETTEXTLEN, (WPARAM)i, (LPARAM)0);
							if (k < MAX_PATH)
							{
								SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_GETTEXT, (WPARAM)i, (LPARAM)PathBuffer);
								SNPrintf(Parms, sizeof(Parms) / sizeof(TCHAR), TEXT("%s\"%s\" "), Parms, PathBuffer);
							}
						}
					}
					
					// Extra Parms?
					if (SendMessage(LauncherControls[LNC_EXTRAEDIT].Handle, WM_GETTEXT, (WPARAM)512, (LPARAM)ExtraParms))
						SNPrintf(Parms, sizeof(Parms) / sizeof(TCHAR), TEXT("%s %s"), Parms, ExtraParms);
						
					printf(">>> %s\n", Parms);
					
					// Launch!
					memset(&Sei, 0, sizeof(Sei));
					Sei.cbSize = sizeof(Sei);
					Sei.fMask = SEE_MASK_NOCLOSEPROCESS;
					Sei.hwnd = hWnd;
					Sei.lpVerb = TEXT("open");
					Sei.lpFile = ProgramDir;
					Sei.lpParameters = Parms;
					Sei.lpDirectory = TEXT(".");
					Sei.nShow = SW_SHOW;
					
					DisableLauncher();
					if (!ShellExecuteEx(&Sei))
						MessageBox(hWnd, TEXT("Failed to execute ReMooD!"), TEXT("Error"), MB_OK | MB_ICONWARNING);
					else
					{
						WaitForSingleObject(Sei.hProcess, INFINITE);
						CloseHandle(Sei.hProcess);
					}
					
					EnableLauncher();
					break;
					
				case LNC_SINGLEPLAYER:
					break;
					
				case LNC_MULTIPLAYER:
					break;
					
				case LNC_SETTINGS:
					break;
					
				case LNC_WEBSITE:
					if (!ShellExecute(NULL, TEXT("open"), TEXT("http://remood.org/"), NULL, NULL, SW_SHOWNORMAL))
						MessageBox(hWnd, TEXT("An error occurred when attempting to load your web browser, please manually go to the address http://remood.org/"), TEXT("Error"), MB_OK | MB_ICONWARNING);
					break;
					
				case LNC_EXIT:
					SendMessage(hWnd, WM_CLOSE, 0, 0);
					break;
					
				
				case LNC_PWADADD:
					memset(&Ofn, 0, sizeof(Ofn));
					memset(&PathBuffer, 0, sizeof(PathBuffer));
					Ofn.lStructSize = sizeof(Ofn);
					Ofn.hwndOwner = hWnd;
					Ofn.hInstance = hInst;
					Ofn.lpstrFilter = TEXT("WAD Files (*.wad)\0*.wad\0All Files (*.*)\0*.*\0\0");
					Ofn.lpstrCustomFilter = NULL;
					Ofn.nMaxCustFilter = 0;
					Ofn.nFilterIndex = 0;
					Ofn.lpstrFile = PathBuffer;
					Ofn.nMaxFile = MAX_PATH;
					Ofn.lpstrInitialDir = WADPath;
					Ofn.lpstrTitle = TEXT("Please select WAD(s)");
					Ofn.Flags = OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
					
					if (GetOpenFileName(&Ofn))
					{
						x = PathBuffer;
						
						if (PathBuffer[StrLen(PathBuffer) + 1])
						{
							x += StrLen(x) + 1;
							
							while (*x)
							{
								SNPrintf(PathBufferZ, MAX_PATH, TEXT("%s\\%s"), PathBuffer, x);
								SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_ADDSTRING, (WPARAM)0, (LPARAM)PathBufferZ);
								x += StrLen(x) + 1;
							}
						}
						else
							SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_ADDSTRING, (WPARAM)0, (LPARAM)PathBuffer);
					}
						
					break;
					
				case LNC_PWADREMOVE:
					i = SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
					
					if (i == LB_ERR)
						i = 0;
					
					SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_DELETESTRING, (WPARAM)i, (LPARAM)0);
					break;
					
				case LNC_PWADUP:
					i = SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
					
					if (i != LB_ERR && i > 0)
					{
						k = SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_GETTEXTLEN, (WPARAM)i, (LPARAM)0);
						l = SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_GETTEXTLEN, (WPARAM)(i - 1), (LPARAM)0);
						
						if (k < MAX_PATH && l < MAX_PATH)
						{
							SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_GETTEXT, (WPARAM)(i - 1), (LPARAM)PathBuffer);
							SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_GETTEXT, (WPARAM)i, (LPARAM)PathBufferZ);
							SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_DELETESTRING, (WPARAM)(i - 1), (LPARAM)0);
							SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_DELETESTRING, (WPARAM)(i - 1), (LPARAM)0);
							
							SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_INSERTSTRING, (WPARAM)(i - 1), (LPARAM)PathBufferZ);
							SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_INSERTSTRING, (WPARAM)i, (LPARAM)PathBuffer);
							SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_SETCURSEL, (WPARAM)(i - 1), (LPARAM)0);
						}
					}
					break;
					
				case LNC_PWADDOWN:
					i = SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
					j = SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
					
					if (i != LB_ERR && j != LB_ERR && i < (j - 1))
					{
						k = SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_GETTEXTLEN, (WPARAM)i, (LPARAM)0);
						l = SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_GETTEXTLEN, (WPARAM)(i + 1), (LPARAM)0);
						
						if (k < MAX_PATH && l < MAX_PATH)
						{
							SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_GETTEXT, (WPARAM)i, (LPARAM)PathBuffer);
							SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_GETTEXT, (WPARAM)(i + 1), (LPARAM)PathBufferZ);
							SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_DELETESTRING, (WPARAM)i, (LPARAM)0);
							SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_DELETESTRING, (WPARAM)i, (LPARAM)0);
							
							SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_INSERTSTRING, (WPARAM)i, (LPARAM)PathBufferZ);
							SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_INSERTSTRING, (WPARAM)(i + 1), (LPARAM)PathBuffer);
							SendMessage(LauncherControls[LNC_PWADCOMBO].Handle, LB_SETCURSEL, (WPARAM)(i + 1), (LPARAM)0);
						}
					}
					break;
					
				default:
					break;
			}
			break;
			
		/*** WINDOW RESIZE ***/
		case WM_SIZE:
			/* Everything is resized based on proportion or something */
			GetClientRect(hWnd, &WinRect);
			
			for (i = 0; i < NUMLAUNCHERCONTROLS; i++)
				if (LauncherControls[i].Handle)
					SetWindowPos(LauncherControls[i].Handle, NULL,
						(float)LauncherControls[i].x *
							((float)(WinRect.right - WinRect.left) / (float)640),
						(float)LauncherControls[i].y *
							((float)(WinRect.bottom - WinRect.top) / (float)400),
						(float)LauncherControls[i].Width *
							((float)(WinRect.right - WinRect.left) / (float)640),
						(float)LauncherControls[i].Height *
							((float)(WinRect.bottom - WinRect.top) / (float)400),
						SWP_NOZORDER);
			break;
			
		/*** WINDOW CLOSE ***/
		case WM_CLOSE:
			GetSystemTime(&SysTime);
			i = (SysTime.wMilliseconds + (SysTime.wSecond / (SysTime.wMinute + 1))) % (sizeof(QuitStrings) / sizeof(TCHAR*));
			if (MessageBox(hWnd, QuitStrings[i], TEXT("Exit ReMooD?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
				DestroyWindow(hWnd);
			break;
			
		/*** WINDOW DESTROYED ***/
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
			
		/*** UNSPECIFIED ***/
		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	
	return 0;
}


/* WinMain() -- Main Entry Point */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	int LastError = 0;
	BOOL AgreeState = FALSE;
	DWORD FileAttrib = NULL;
	WNDCLASSEX WizardClass;
	WNDCLASSEX LauncherClass;
	MSG Msg;

	/*** THE GOOD STUFF ***/
	/* Detect which Windows we are running so we can handle certain functions... */
	
	/* Setup Things */
	memset(WADPath, 0, sizeof(WADPath));
	memset(ProgramDir, 0, sizeof(ProgramDir));
	memset(&WizardClass, 0, sizeof(WNDCLASSEX));
	memset(&LauncherClass, 0, sizeof(WNDCLASSEX));
	InitCommonControls();

	/* Attempt to load remood-launcher.cfg */
	Launcher_LoadDefaultConfig();
	
	/*** WIZARD (IF NEEDED) ***/
	AgreeState = OptAgreedToGPL;
	if (!ConfigFile || !OptAgreedToGPL)
	{
		/* Setup and register the window class */
		WizardClass.cbSize = sizeof(WNDCLASSEX);
		WizardClass.cbWndExtra = 0;
		WizardClass.cbClsExtra = 0;
		WizardClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		WizardClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		WizardClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
		WizardClass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(2));
		WizardClass.hInstance = hInstance;
		WizardClass.lpfnWndProc = WizardProc;
		WizardClass.lpszMenuName = NULL;
		WizardClass.style = 0;
		WizardClass.lpszClassName = WizardClassName;
		
		if (!RegisterClassEx(&WizardClass))
			MessageBox(NULL, TEXT("Wizard Class failed to register, you will not have a wizard to help you."), TEXT("Fatal Error"), MB_OK | MB_ICONERROR);
		else
		{
			/* Create the Window */
			WizardWindow = CreateWindowEx(0,
				WizardClassName,
				TEXT("ReMooD Wizard"),
				WS_POPUPWINDOW | WS_CAPTION,
				(GetSystemMetrics(SM_CXSCREEN) >> 1) - ((640 + (GetSystemMetrics(SM_CXDLGFRAME) * 2)) >> 1),
				(GetSystemMetrics(SM_CYSCREEN) >> 1) - ((400 + GetSystemMetrics(SM_CYSIZE) + (GetSystemMetrics(SM_CYDLGFRAME) * 2)) >> 1),
				640 + GetSystemMetrics(SM_CXDLGFRAME),
				400 + GetSystemMetrics(SM_CYSIZE) + (GetSystemMetrics(SM_CYDLGFRAME) * 2),
				NULL,
				NULL,
				hInstance,
				NULL);
		
			if (!WizardWindow)
				MessageBox(NULL, TEXT("The Wizard Window failed to be created, you will not have a wizard to help you."), TEXT("Fatal Error"), MB_OK | MB_ICONERROR);
			else
			{
				/* Now Show it */
				ShowWindow(WizardWindow, nShowCmd);
				UpdateWindow(WizardWindow);
				
				while (GetMessage(&Msg, NULL, 0, 0) > 0)
				{
					TranslateMessage(&Msg);
					DispatchMessage(&Msg);
				}
			}
		}
	}
	
	/*** MAIN LAUNCHER ***/
	if (OptAgreedToGPL)
	{
		LauncherClass.cbSize = sizeof(WNDCLASSEX);
		LauncherClass.cbWndExtra = 0;
		LauncherClass.cbClsExtra = 0;
		LauncherClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		LauncherClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		LauncherClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
		LauncherClass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(2));
		LauncherClass.hInstance = hInstance;
		LauncherClass.lpfnWndProc = LauncherProc;
		LauncherClass.lpszMenuName = NULL;
		LauncherClass.style = 0;
		LauncherClass.lpszClassName = LauncherClassName;
	
		if (!RegisterClassEx(&LauncherClass))
		{
			LastError = GetLastError();
			MessageBox(NULL, TEXT("Failed to register launcher class."), TEXT("Fatal Error"), MB_OK | MB_ICONERROR);
			return GetLastError();
		}
	
		/* Create the Window */
		LauncherWindow = CreateWindowEx(0,
			LauncherClassName,
			TEXT("ReMooD Launcher"),
			WS_OVERLAPPEDWINDOW,
			(GetSystemMetrics(SM_CXSCREEN) >> 1) - ((640 + (GetSystemMetrics(SM_CXSIZEFRAME) * 2)) >> 1),
			(GetSystemMetrics(SM_CYSCREEN) >> 1) - ((400 + GetSystemMetrics(SM_CYSIZE) + (GetSystemMetrics(SM_CYSIZEFRAME) * 2)) >> 1),
			640 + GetSystemMetrics(SM_CXSIZEFRAME),
			400 + GetSystemMetrics(SM_CYSIZE) + (GetSystemMetrics(SM_CYSIZEFRAME) * 2),
			NULL,
			NULL,
			hInstance,
			NULL);
		
		if (!LauncherWindow)
		{
			LastError = GetLastError();
			MessageBox(NULL, TEXT("Failed to create launcher window."), TEXT("Fatal Error"), MB_OK | MB_ICONERROR);
			return GetLastError();
		}

		/* Now Show it */
		ShowWindow(LauncherWindow, nShowCmd);
		UpdateWindow(LauncherWindow);

		while (GetMessage(&Msg, NULL, 0, 0) > 0)
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	}

	if (!ConfigFile || !AgreeState)
		UnregisterClass(WizardClassName, hInstance);
	if (OptAgreedToGPL)
		UnregisterClass(LauncherClassName, hInstance);

	/* Save Configuration before quiting */
	Launcher_SaveDefaultConfig();
	
	return Msg.wParam;
}

TCHAR* GPLLicense = TEXT("            GNU GENERAL PUBLIC LICENSE\r\n\
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

