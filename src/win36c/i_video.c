// -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------
// Copyright (C) 2011-2013 GhostlyDeath <ghostlydeath@remood.org>
//                                      <ghostlydeath@gmail.com>
// ----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// ----------------------------------------------------------------------------
// DESCRIPTION: DOOM graphics stuff for Allegro

/***************
*** INCLUDES ***
***************/

/* System */
#include <windows.h>
#include <wingdi.h>
#include <stdlib.h>

/* Local */
#include "doomtype.h"
#include "doomdef.h"
#include "i_video.h"
#include "i_util.h"
#include "i_system.h"

#define __G_INPUT_H__
#include "console.h"

/****************
*** CONSTANTS ***
****************/

/*********************
*** LOCALS/GLOBALS ***
*********************/

/** Windows CE Stuff **/
TCHAR ceClassName[] = TEXT("ReMooDCE");
TCHAR ceWindowName[] = TEXT("ReMooD");
HWND cePrimaryWindow = NULL;
HINSTANCE ceInstance = NULL;

HBITMAP ceWinBMP = NULL;
BITMAPINFO* ceWinBMPInfo = NULL;

static uint8_t* l_ceScreenBuffer = NULL;
/**********************/

/****************
*** FUNCTIONS ***
****************/

/* ceWindowProc() -- Main window procedure */
LRESULT CALLBACK ceWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	static int32_t OldX, OldY;
	HDC hDc, bufDc;
	PAINTSTRUCT Ps;
	HBITMAP bufBMP;
	BITMAPINFO bmpInfo;
	int i, j;
	I_EventEx_t ExEvent;
	
	switch (Msg)
	{
		case WM_CREATE:
			break;

		case WM_MOUSEMOVE:
			// Generate
			ExEvent.Type = IET_MOUSE;
			ExEvent.Data.Mouse.MouseID = 0;	// Always mouse Zero
			ExEvent.Data.Mouse.Pos[0] = LOWORD(lParam);
			ExEvent.Data.Mouse.Pos[1] = HIWORD(lParam);
			
			ExEvent.Data.Mouse.Move[0] = ExEvent.Data.Mouse.Pos[0] - OldX;
			ExEvent.Data.Mouse.Move[1] = ExEvent.Data.Mouse.Pos[1] - OldY;
			
			OldX = ExEvent.Data.Mouse.Pos[0];
			OldY = ExEvent.Data.Mouse.Pos[1];
			
			// Push
			I_EventExPush(&ExEvent);
			break;

		case WM_LBUTTONDBLCLK:
#if 0
			feVents[feWrite].Type = 1;
			feVents[feWrite].Data.Mouse.button = 1;
			feVents[feWrite].Data.Mouse.x = LOWORD(lParam);
			feVents[feWrite].Data.Mouse.y = HIWORD(lParam);

			// Increment
			feWrite++;
			if (feWrite >= NUMFAKEEVENTS)
				feWrite = 0;
#endif
			break;
			
		case WM_PAINT:
			hDc = BeginPaint(hWnd, &Ps);
			bufDc = CreateCompatibleDC(hDc);
			SelectObject(bufDc, ceWinBMP);
			
			if (l_ceScreenBuffer)
				memcpy(l_ceScreenBuffer, screens[0], vid.width * vid.height);
			
			BitBlt(hDc, 0, 0, vid.width, vid.height, bufDc, 0, 0, SRCCOPY);
			DeleteDC(bufDc);
			EndPaint(hWnd, &Ps);
			break;
		
		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	
	return 0;
}

/* I_GetEvent() -- Gets an event and adds it to the queue */
void I_GetEvent(void)
{
}

void I_UpdateNoBlit(void)
{
}

/* I_StartFrame() -- Called before drawing a frame */
void I_StartFrame(void)
{
	MSG Msg;
	
	if (!cePrimaryWindow)
		return;
		
	// Win32 Messages
	while (PeekMessage(&Msg, cePrimaryWindow, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
}

/* I_FinishUpdate() -- Called after drawing a frame */
void I_FinishUpdate(void)
{
	InvalidateRect(cePrimaryWindow, NULL, FALSE);
	UpdateWindow(cePrimaryWindow);
}

/* I_SetPalette() -- Sets the current palette */
void I_SetPalette(RGBA_t* palette)
{
	int i;
	HDC hDc, hDc2;
	
	hDc = GetDC(cePrimaryWindow);
	hDc2 = CreateCompatibleDC(hDc);
	
	for (i = 0; i < 256; i++)
	{
		ceWinBMPInfo->bmiColors[i].rgbRed = palette[i].s.red;
		ceWinBMPInfo->bmiColors[i].rgbGreen = palette[i].s.green;
		ceWinBMPInfo->bmiColors[i].rgbBlue = palette[i].s.blue;
		ceWinBMPInfo->bmiColors[i].rgbReserved = 0;
	}
	
	SelectObject(hDc2, ceWinBMP);
	SetDIBColorTable(hDc2, 0, 256, ceWinBMPInfo->bmiColors);
	DeleteDC(hDc2);
	ReleaseDC((HWND)ceWinBMPInfo, hDc);
}

/* VID_PrepareModeList() -- Adds video modes to the mode list */
// Allegro does not allow "magic" drivers to be passed in the mode list getting
// function. Therefor I decided to use a loop of sorts with available drivers
// for everything. This is because I tell it to autodetect anyway, so it could
// choose any of the specified drivers anyway. Also, VID_AddMode() will not
// add a duplicate mode anyway.
void VID_PrepareModeList(void)
{
	VID_AddMode(320, 200, true);
	VID_AddMode(320, 240, true);
}

/* I_SetVideoMode() -- Sets the current video mode */
bool_t I_SetVideoMode(const uint32_t a_Width, const uint32_t a_Height, const bool_t a_Fullscreen)
{
	HDC hDc;
	int i;
	int centerX, centerY;
	RECT clRect, wnRect;
	HICON hIcon, hIconSmall;
	
	/* Check */
	if (!a_Width || !a_Height)
		return false;
	
	/* Destroy old buffer */
	I_VideoUnsetBuffer();		// Remove old buffer if any
	
	// Destroy old window
	if (cePrimaryWindow)
		DestroyWindow(cePrimaryWindow);
	cePrimaryWindow = NULL;
	
	// Delete direct bitmap
	if (l_ceScreenBuffer)
		I_SysFree(l_ceScreenBuffer);
	l_ceScreenBuffer = NULL;
	
	/* Free old bitmap */
	if (ceWinBMP)
	{
		DeleteObject(ceWinBMP);
		ceWinBMP = NULL;
	}
	
	if (ceWinBMPInfo)
	{
		free(ceWinBMPInfo);
		ceWinBMPInfo = NULL;
	}
	
	/* Create Window */
	centerX = (GetSystemMetrics(SM_CXSCREEN) >> 1) - (a_Width >> 1);
	centerY = (GetSystemMetrics(SM_CYSCREEN) >> 1) - (a_Height >> 1);
	
	/* Create Window */
	cePrimaryWindow = CreateWindowEx(
			0,
			ceClassName,
			ceWindowName,
			WS_BORDER | WS_POPUP | WS_SYSMENU | WS_CAPTION | WS_VISIBLE,
			centerX,
			centerY,
			a_Width,
			a_Height,
			NULL,
			NULL,
			ceInstance,
			NULL
		);
	
	/* Check */
	if (!cePrimaryWindow)
	{
		I_Error("Failed to create main window.");
		return false;
	}
	
	I_StartFrame();

	GetWindowRect(cePrimaryWindow, &wnRect);
	GetClientRect(cePrimaryWindow, &clRect);
	SetWindowPos(cePrimaryWindow, HWND_TOPMOST, 0, 0,
		(wnRect.right - wnRect.left) + (a_Width - (clRect.right - clRect.left)),
		(wnRect.bottom - wnRect.top) + (a_Height - (clRect.bottom - clRect.top)),
		SWP_NOMOVE);
		
	/* Set Icon */
	// Load
	hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(1));
	hIconSmall = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(2));

	// Use Long in 32-bit/CE and LongPtr in 64 (since LongPtr is not avail in 98!)
#if !defined(_WIN64)
	SetClassLong(cePrimaryWindow, GCL_HICON, (LONG)hIcon);
	SetClassLong(cePrimaryWindow, GCL_HICONSM, (LONG)hIconSmall);
#else
	SetClassLongPtr(cePrimaryWindow, GCLP_HICON, (LONG_PTR)hIcon);
	SetClassLongPtr(cePrimaryWindow, GCLP_HICONSM, (LONG_PTR)hIconSmall);
#endif
	
	ShowWindow(cePrimaryWindow, SW_SHOW);
	UpdateWindow(cePrimaryWindow);
	
	/* Setup Bitmap */
	ceWinBMPInfo = (BITMAPINFO*)I_SysAlloc(sizeof(*ceWinBMPInfo) + (384 * sizeof(RGBQUAD)));
	ceWinBMPInfo->bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	ceWinBMPInfo->bmiHeader.biWidth			= a_Width;
	ceWinBMPInfo->bmiHeader.biHeight		= -((int)(a_Height));
	ceWinBMPInfo->bmiHeader.biPlanes		= 1;
	ceWinBMPInfo->bmiHeader.biSizeImage		= a_Height * a_Width;
	ceWinBMPInfo->bmiHeader.biXPelsPerMeter	= 0;
	ceWinBMPInfo->bmiHeader.biYPelsPerMeter	= 0;
	ceWinBMPInfo->bmiHeader.biClrUsed		= 256;
	ceWinBMPInfo->bmiHeader.biClrImportant	= 0;
	ceWinBMPInfo->bmiHeader.biBitCount		= 8;
	ceWinBMPInfo->bmiHeader.biCompression	= BI_RGB;

#if 0
	if (pLocalPalette)
		for (i = 0; i < 256; i++)
		{
			ceWinBMPInfo->bmiColors[i].rgbRed = pLocalPalette[i].s.red;
			ceWinBMPInfo->bmiColors[i].rgbGreen = pLocalPalette[i].s.green;
			ceWinBMPInfo->bmiColors[i].rgbBlue = pLocalPalette[i].s.blue;
			ceWinBMPInfo->bmiColors[i].rgbReserved = 0;
		}
	else
		for (i = 0; i < 256; i++)
		{
			ceWinBMPInfo->bmiColors[i].rgbRed = i;
			ceWinBMPInfo->bmiColors[i].rgbGreen = i;
			ceWinBMPInfo->bmiColors[i].rgbBlue = i;
			ceWinBMPInfo->bmiColors[i].rgbReserved = 0;
		}
#endif
	
	/* Recreate buffer for screen */
	l_ceScreenBuffer = (uint8_t*)I_SysAlloc(a_Width * a_Height);
	
	/* Create Bitmap */
	CONL_PrintF("DC!\n");
	hDc = GetDC(cePrimaryWindow);
	ceWinBMP = CreateDIBSection(hDc, ceWinBMPInfo, DIB_RGB_COLORS, (void**)&l_ceScreenBuffer, NULL, 0);
	ReleaseDC(cePrimaryWindow, hDc);
	
	/* Allocate Buffer */
	I_VideoSetBuffer(a_Width, a_Height, a_Width, l_ceScreenBuffer, false);
	
	/* Success */
	return true;
}

/* I_StartupGraphics() -- Initializes graphics */
void I_StartupGraphics(void)
{
	WNDCLASS wcex;
	
	/* Pre-initialize video */
	if (!I_VideoPreInit())
		return;
		
	/* Create Class */
	memset(&wcex, 0, sizeof(wcex));
	
	//wcex.cbSize = sizeof(wcex);
	wcex.lpfnWndProc = ceWindowProc;
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszClassName = ceClassName;
	
	/* Register */
	if (!RegisterClass(&wcex))
	{
		I_Error("Could not register window class!");
		return;
	}	
	
	/* Initialize before mode set */
	if (!I_VideoBefore320200Init())
		return;
	if (!I_SetVideoMode(320, 200, false))	// 320x200 console scroller, never fullscreen
		return;
		
	/* Prepare the video mode list */
	if (!I_VideoPostInit())
		return;
}

/* I_ShutdownGraphics() -- Turns off graphics */
void I_ShutdownGraphics(void)
{
}

/* I_TextMode() -- Enter and leaves text mode */
bool_t I_TextMode(const bool_t a_OnOff)
{
	/* On */
	if (a_OnOff)
	{
		return true;
	}
	
	/* Off */
	else
	{
		return true;
	}
}

/* I_ProbeJoysticks() -- Probes all joysticks */
size_t I_ProbeJoysticks(void)
{
	return 0;
}

/* I_RemoveJoysticks() -- Removes all joysticks */
void I_RemoveJoysticks(void)
{
}

/* I_GetJoystickID() -- Gets name of the joysticks */
bool_t I_GetJoystickID(const size_t a_JoyID, uint32_t* const a_Code, char* const a_Text, const size_t a_TextSize, char* const a_Cool, const size_t a_CoolSize)
{
	/* Always Fail */
	return false;
}

/* I_GetJoystickCounts() -- Get joystick counts */
bool_t I_GetJoystickCounts(const size_t a_JoyID, uint32_t* const a_NumAxis, uint32_t* const a_NumButtons)
{
	/* Always Fail */
	return false;
}

/* I_ProbeMouse() -- Probes Mice */
bool_t I_ProbeMouse(const size_t a_ID)
{
	return true;
}

/* I_RemoveMouse() -- Removes mice */
bool_t I_RemoveMouse(const size_t a_ID)
{
	return true;
}

/* I_MouseGrab() -- Sets mouse grabbing */
void I_MouseGrab(const bool_t a_Grab)
{
}

/* I_VideoLockBuffer() -- Locks the video buffer */
void I_VideoLockBuffer(const bool_t a_DoLock)
{
}


