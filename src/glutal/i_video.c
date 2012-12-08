// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
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
#include <stdlib.h>
#include <stdint.h>

#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

/* Local */
#include "doomtype.h"
#include "doomdef.h"
#include "i_video.h"
#include "i_util.h"
#include "m_argv.h"

#define __G_INPUT_H__
#include "console.h"

/****************
*** CONSTANTS ***
****************/

#ifndef PIXEL_PACK_BUFFER_EXT
	#define PIXEL_PACK_BUFFER_EXT 0x88EB
#endif

#ifndef PIXEL_UNPACK_BUFFER_EXT
	#define PIXEL_UNPACK_BUFFER_EXT 0x88EC
#endif

/* I_GLUploadMode_t -- Image upload mode */
typedef enum I_GLUploadMode_e
{
	IGLUM_AUTO,									// Automatic	
	
	IGLUM_PUTPIXEL,								// Put pixel
	IGLUM_DRAWPIXELS,							// Draw Pixels
	IGLUM_TEXTURE,								// Textures
	IGLUM_NVPDR,								// GL_NV_pixel_data_range
	IGLUM_EXTPBO,								// GL_EXT_pixel_buffer_object
	IGLUM_ARBPBO,								// GL_ARB_pixel_buffer_object
} I_GLUploadMode_t;

/*************
*** LOCALS ***
*************/

static union
{
	uint8_t c[4];
	uint32_t u;
} l_GLPalette[256];								// OpenGL Palette

static I_GLUploadMode_t l_GLUpMode;				// Image upload mode
static int l_GLUTWinRef = 0;					// GLUT Window reference

static uint8_t* l_GLImgBuffer;					// Image Buffer
static uint32_t l_GLImgSize[4];					// Size of image
static GLuint l_GLPBOBuffer;					// PBO Buffer
static uint8_t* l_GLPBOMem;						// PBO Memory Area

/****************
*** FUNCTIONS ***
****************/

/* IS_GLUTToIKBK() -- Converts GLUT to standard key */
static I_KeyBoardKey_t IS_GLUTToIKBK(const int a_Key, const bool_t a_Special)
{
	/* Special Keys */
	if (a_Special)
	{
		// Function Keys
		if (a_Key >= GLUT_KEY_F1 && a_Key <= GLUT_KEY_F12)
			return IKBK_F1 + (a_Key - GLUT_KEY_F1);
		
		// Others
		else
			switch (a_Key)
			{
				case GLUT_KEY_DELETE:		return IKBK_KDELETE;
				case UINT8_C(0x1B):			return IKBK_ESCAPE;
				case UINT8_C(0x91):			return IKBK_BACKSPACE;
				case UINT8_C(0x92):			return IKBK_TAB;
				case UINT8_C(0x0D):			return IKBK_RETURN;
				case GLUT_KEY_PAGE_UP:		return IKBK_PAGEUP;
				case GLUT_KEY_PAGE_DOWN:	return IKBK_PAGEDOWN;
				case GLUT_KEY_HOME:			return IKBK_HOME;
				case GLUT_KEY_END:			return IKBK_END;
				case GLUT_KEY_LEFT:			return IKBK_LEFT;
				case GLUT_KEY_RIGHT:		return IKBK_RIGHT;
				case GLUT_KEY_UP:			return IKBK_UP;
				case GLUT_KEY_DOWN:			return IKBK_DOWN;
				case GLUT_KEY_INSERT:		return IKBK_INSERT;
#if 0
				case GLUT_KEY_PAD_DIVIDE:	return IKBK_NUMDIVIDE;
				case GLUT_KEY_PAD_MULTIPLY:	return IKBK_NUMMULTIPLY;
				case GLUT_KEY_PAD_SUBTRACT:	return IKBK_NUMSUBTRACT;
				case GLUT_KEY_PAD_ADD:		return IKBK_NUMADD;
				case GLUT_KEY_PAD_RETURN:	return IKBK_NUMENTER;
				case GLUT_KEY_PAD_DECIMAL:	return IKBK_NUMPERIOD;
				case GLUT_KEY_CONTROL:		return IKBK_CTRL;
				case GLUT_KEY_ALT:			return IKBK_ALT;
				case GLUT_KEY_SHIFT:		return IKBK_SHIFT;
#endif
			
					// Unknown?
				default:
					return 0;
			}
	}
	
	/* Non-Special Keys */
	else
		switch (a_Key)
		{
			case UINT8_C(0x1B):			return IKBK_ESCAPE;
			case UINT8_C(0x0D):			return IKBK_RETURN;
			
				// Standard
			default:
				return toupper(a_Key);
		}
}

/* IS_HandleCAS() -- Handles ctrl/alt/shift */
static void IS_HandleCAS(void)
{
	I_EventEx_t New;
	static int LastMods;
	int NowMods;
	
	/* Get Modifiers */
	NowMods = glutGetModifiers();
	
	/* No Change? */
	if (NowMods == LastMods)
		return;
	
	/* Build event */
	// Clear
	memset(&New, 0, sizeof(New));
	
	// Fill
	New.Type = IET_KEYBOARD;
	New.Data.Keyboard.Character = 0;
	New.Data.Keyboard.Repeat = false;
	
	// Handle Ctrl
	if ((NowMods & GLUT_ACTIVE_CTRL) != (LastMods & GLUT_ACTIVE_CTRL))
	{
		New.Data.Keyboard.Down = !!(NowMods & GLUT_ACTIVE_CTRL);
		New.Data.Keyboard.KeyCode = IKBK_CTRL;
		I_EventExPush(&New);
	}
	
	// Handle Alt
	if ((NowMods & GLUT_ACTIVE_ALT) != (LastMods & GLUT_ACTIVE_ALT))
	{
		New.Data.Keyboard.Down = !!(NowMods & GLUT_ACTIVE_ALT);
		New.Data.Keyboard.KeyCode = IKBK_ALT;
		I_EventExPush(&New);
	}
	
	// Handle Shift
	if ((NowMods & GLUT_ACTIVE_SHIFT) != (LastMods & GLUT_ACTIVE_SHIFT))
	{
		New.Data.Keyboard.Down = !!(NowMods & GLUT_ACTIVE_SHIFT);
		New.Data.Keyboard.KeyCode = IKBK_SHIFT;
		I_EventExPush(&New);
	}
	
	/* Remember mods */
	LastMods = NowMods;
}

/* IS_GLUTMouseMotion() -- Mouse motion */
static void IS_GLUTMouseMotion(int x, int y)
{
	static int OldX, OldY;
	I_EventEx_t New;
	
	/* Setup event */
	memset(&New, 0, sizeof(New));
	
	// Fill
	New.Type = IET_MOUSE;
	New.Data.Mouse.Down = false;
	New.Data.Mouse.Pos[0] = x;
	New.Data.Mouse.Pos[1] = y;
	New.Data.Mouse.Move[0] = x - OldX;
	New.Data.Mouse.Move[1] = OldY - y;
	
	/* Send event away */
	I_EventExPush(&New);
	
	/* Remember old location */
	OldX = x;
	OldY = y;
}

/* IS_GLUTMouseButtons() -- Buttons change */
static void IS_GLUTMouseButtons(int button, int state, int x, int y)
{
	I_EventEx_t New;
	
	/* Setup event */
	memset(&New, 0, sizeof(New));
	
	// Fill
	New.Type = IET_MOUSE;
	New.Data.Mouse.Down = !!(state == GLUT_DOWN);
	
	if (button == GLUT_LEFT_BUTTON)
		New.Data.Mouse.Button = 1;
	else if (button == GLUT_RIGHT_BUTTON)
		New.Data.Mouse.Button = 2;
	else if (button == GLUT_MIDDLE_BUTTON)
		New.Data.Mouse.Button = 3;
	
	New.Data.Mouse.Pos[0] = x;
	New.Data.Mouse.Pos[1] = y;
	
	/* Send event away */
	if (New.Data.Mouse.Button)
		I_EventExPush(&New);
	
	/* Handle Ctrl/Alt/Shift */
	IS_HandleCAS();
	
	/* Handle Mouse */
	IS_GLUTMouseMotion(x, y);
}

/* IS_GLUTTimer() -- GLUT Timer */
static void IS_GLUTTimer(int value)
{
}

/* IS_GLUTKeyDown() -- Key is pressed */
static void IS_GLUTKeyDown(unsigned char key, int x, int y)
{
	I_EventEx_t New;
	
	/* Setup event */
	memset(&New, 0, sizeof(New));
	
	// Fill
	New.Type = IET_KEYBOARD;
	New.Data.Keyboard.Down = true;
	New.Data.Keyboard.Repeat = false;
	New.Data.Keyboard.KeyCode = IS_GLUTToIKBK(key, false);
	New.Data.Keyboard.Character = key;
	
	/* Send event away */
	I_EventExPush(&New);
	
	/* Handle Ctrl/Alt/Shift */
	IS_HandleCAS();
	
	/* Handle Mouse */
	IS_GLUTMouseMotion(x, y);
}

/* IS_GLUTKeyUp() -- Key released */
static void IS_GLUTKeyUp(unsigned char key, int x, int y)
{
	I_EventEx_t New;
	
	/* Setup event */
	memset(&New, 0, sizeof(New));
	
	// Fill
	New.Type = IET_KEYBOARD;
	New.Data.Keyboard.Down = false;
	New.Data.Keyboard.Repeat = false;
	New.Data.Keyboard.KeyCode = IS_GLUTToIKBK(key, false);
	New.Data.Keyboard.Character = key;

	/* Send event away */
	I_EventExPush(&New);
	
	/* Handle Ctrl/Alt/Shift */
	IS_HandleCAS();
	
	/* Handle Mouse */
	IS_GLUTMouseMotion(x, y);
}

/* IS_GLUTSpecialDown() -- Special key is pressed */
static void IS_GLUTSpecialDown(int key, int x, int y)
{
	I_EventEx_t New;
	
	/* Setup event */
	memset(&New, 0, sizeof(New));
	
	// Fill
	New.Type = IET_KEYBOARD;
	New.Data.Keyboard.Down = true;
	New.Data.Keyboard.Repeat = false;
	New.Data.Keyboard.KeyCode = IS_GLUTToIKBK(key, true);
	New.Data.Keyboard.Character = 0;
	
	/* Send event away */
	I_EventExPush(&New);
	
	/* Handle Ctrl/Alt/Shift */
	IS_HandleCAS();
	
	/* Handle Mouse */
	IS_GLUTMouseMotion(x, y);
}

/* IS_GLUTSpecialUp() -- Special released */
static void IS_GLUTSpecialUp(int key, int x, int y)
{
	I_EventEx_t New;
	
	/* Setup event */
	memset(&New, 0, sizeof(New));
	
	// Fill
	New.Type = IET_KEYBOARD;
	New.Data.Keyboard.Down = false;
	New.Data.Keyboard.Repeat = false;
	New.Data.Keyboard.KeyCode = IS_GLUTToIKBK(key, true);
	New.Data.Keyboard.Character = 0;
	
	/* Send event away */
	I_EventExPush(&New);
	
	/* Handle Ctrl/Alt/Shift */
	IS_HandleCAS();
	
	/* Handle Mouse */
	IS_GLUTMouseMotion(x, y);
}

/* IS_GLUTDisplay() -- GLUT Window displayed */
static void IS_GLUTDisplay(void)
{
}

/* I_GetEvent() -- Gets an event and adds it to the queue */
void I_GetEvent(void)
{
	/* Handle events in the GLUT loop */
	glutMainLoopEvent();
}

void I_UpdateNoBlit(void)
{
}

/* I_StartFrame() -- Called before drawing a frame */
void I_StartFrame(void)
{
}

/* I_FinishUpdate() -- Called after drawing a frame */
void I_FinishUpdate(void)
{
	uint8_t* Buffer;
	uint32_t w, h, wPOT, hPOT;
	uint32_t x, y;
	uint8_t Bit;
	
	/* Obtain the software buffer */
	Buffer = I_VideoSoftBuffer(&w, &h);
	
	// Failed?
	if (!Buffer)
		return;
	
	/* Power of two width and height */
	// Width
	wPOT = l_GLImgSize[0];
	
	// Height
	hPOT = l_GLImgSize[1];
	
	/* Clear the framebuffer */
	// Set view port
	glViewport(0, 0, w, h);
	
	// Orthographic projection, from top left origin
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glOrtho(0, (GLint)w, (GLint)h, 0, 1, -1);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	// Clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	/* Send video buffer to screen */
	switch (l_GLUpMode)
	{
			// Put individual pixels (REALLY SLOW)
		case IGLUM_PUTPIXEL:
			glBegin(GL_POINTS);
			for (y = 0; y < h; y++)
				for (x = 0; x < w; x++)
				{
					Bit = Buffer[(y * w) + x];
					glColor3ub(
							l_GLPalette[Bit].c[0],
							l_GLPalette[Bit].c[1],
							l_GLPalette[Bit].c[2]
						);
					glVertex2i(x, y);
				}
			glEnd();
			break;
		
			// Use glDrawPixels
		case IGLUM_DRAWPIXELS:
			break;
		
			// Upload textures
		case IGLUM_TEXTURE:
			// Draw into texture space
			for (y = 0; y < h; y++)
				for (x = 0; x < w; x++)
				{
					Bit = Buffer[(y * w) + x];
					((uint32_t*)l_GLImgBuffer)[(y * wPOT) + x] = l_GLPalette[Bit].u;
				}
				
			// Create OpenGL Texture
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 13);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, wPOT);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wPOT, hPOT, 0, GL_RGBA, GL_UNSIGNED_BYTE/*_8_8_8_8*/, l_GLImgBuffer);
			
			glBegin(GL_QUADS);
				glTexCoord2d(0, 0);
				glVertex2i(0, 0);
				glTexCoord2d(1, 0);
				glVertex2i(wPOT, 0);
				glTexCoord2d(1, 1);
				glVertex2i(wPOT, hPOT);
				glTexCoord2d(0, 1);
				glVertex2i(0, hPOT);
			glEnd();
			
			glDisable(GL_TEXTURE_2D);
			break;
			
			// EXT PBO
		case IGLUM_EXTPBO:
			// Obtain Buffer
			l_GLPBOMem = glMapBuffer(PIXEL_UNPACK_BUFFER_EXT, GL_WRITE_ONLY);
			
			for (y = 0; y < h; y++)
				for (x = 0; x < w; x++)
				{
					Bit = Buffer[(y * w) + x];
					((uint32_t*)l_GLPBOMem)[(y * wPOT) + x] = l_GLPalette[Bit].u;
				}
			
			glUnmapBuffer(PIXEL_UNPACK_BUFFER_EXT);
			
			glBegin(GL_QUADS);
				glTexCoord2d(0, 0);
				glVertex2i(0, 0);
				glTexCoord2d(1, 0);
				glVertex2i(wPOT, 0);
				glTexCoord2d(1, 1);
				glVertex2i(wPOT, hPOT);
				glTexCoord2d(0, 1);
				glVertex2i(0, hPOT);
			glEnd();
			break;
			
			// ARB PBO
		case IGLUM_ARBPBO:
			break;
			
			// Unknown
		default:
			break;
	}
	
	/* Swap */
	glFlush();
	glutSwapBuffers();
}

/* I_SetPalette() -- Sets the current palette */
void I_SetPalette(RGBA_t* palette)
{
	register int i; 
	/* Check */
	if (!palette)
		return;
	
	/* Clone Colors */
	for (i = 0; i < 256; i++)
	{
		l_GLPalette[i].c[0] = palette[i].s.red;
		l_GLPalette[i].c[1] = palette[i].s.green;
		l_GLPalette[i].c[2] = palette[i].s.blue;
		l_GLPalette[i].c[3] = 0xFF;
	}
}

/* VID_PrepareModeList() -- Adds video modes to the mode list */
// Allegro does not allow "magic" drivers to be passed in the mode list getting
// function. Therefor I decided to use a loop of sorts with available drivers
// for everything. This is because I tell it to autodetect anyway, so it could
// choose any of the specified drivers anyway. Also, VID_AddMode() will not
// add a duplicate mode anyway.
void VID_PrepareModeList(void)
{
	/* Add Basic Modes */
	VID_AddMode(320, 200, true);
	VID_AddMode(320, 240, true);
	VID_AddMode(640, 400, true);
	VID_AddMode(640, 480, true);
	VID_AddMode(800, 600, true);
	VID_AddMode(1024, 768, true);
	
}

/* I_SetVideoMode() -- Sets the current video mode */
bool_t I_SetVideoMode(const uint32_t a_Width, const uint32_t a_Height, const bool_t a_Fullscreen)
{
	uint32_t wPOT, hPOT;
	
	/* Check */
	if (!a_Width || !a_Height)
		return false;
		
	/* Destroy old buffer */
	I_VideoUnsetBuffer();		// Remove old buffer if any
	
	/* Destroy old GLUT window */
	if (l_GLUTWinRef)
	{
		// Destroy it
		glutDestroyWindow(l_GLUTWinRef);
		
		// Set back to zero
		l_GLUTWinRef = 0;
	}
	
	/* Use Game Mode for fullscreen */
	if (a_Fullscreen)
	{
	}
	
	/* Otherwise, standard GLUT windows */
	else
	{
		// Create Window
		glutInitWindowSize(a_Width, a_Height);
		l_GLUTWinRef = glutCreateWindow("ReMooD " REMOOD_FULLVERSIONSTRING);
		
		// Failed?
		if (!l_GLUTWinRef)
			return false;
		
		// Change properties
		glutSetWindow(l_GLUTWinRef);
		glutReshapeWindow(a_Width, a_Height);
	}
	
	/* Register handlers */
	glutDisplayFunc(IS_GLUTDisplay);
	glutKeyboardFunc(IS_GLUTKeyDown);
	glutKeyboardUpFunc(IS_GLUTKeyUp);
	glutSpecialFunc(IS_GLUTSpecialDown);
	glutSpecialUpFunc(IS_GLUTSpecialUp);
	glutTimerFunc(200, IS_GLUTTimer, 1);
	glutMotionFunc(IS_GLUTMouseMotion);
	glutPassiveMotionFunc(IS_GLUTMouseMotion);
	glutMouseFunc(IS_GLUTMouseButtons);
	
	/* Clear Extenstions */
	
	/* Determine image uploading mode */
		// ARB PBO
	if (glutExtensionSupported("GL_ARB_pixel_buffer_object"))
		l_GLUpMode = IGLUM_ARBPBO;
		
		// EXT PBO
	else if (glutExtensionSupported("GL_EXT_pixel_buffer_object"))
		l_GLUpMode = IGLUM_EXTPBO;
		
		// Textures
	else
		l_GLUpMode = IGLUM_TEXTURE;
			
	l_GLUpMode = IGLUM_TEXTURE;//IGLUM_EXTPBO;
		
	/* Power of two width and height */
	// Width
	wPOT = a_Width - 1;
	wPOT |= (wPOT >> 1);
	wPOT |= (wPOT >> 2);
	wPOT |= (wPOT >> 4);
	wPOT |= (wPOT >> 8);
	wPOT |= (wPOT >> 16);
	wPOT += 1;
	
	// Height
	hPOT = a_Height - 1;
	hPOT |= (hPOT >> 1);
	hPOT |= (hPOT >> 2);
	hPOT |= (hPOT >> 4);
	hPOT |= (hPOT >> 8);
	hPOT |= (hPOT >> 16);
	hPOT += 1;
	
	/* Need new texture allocation? */
	if (l_GLUpMode != IGLUM_EXTPBO && l_GLUpMode != IGLUM_ARBPBO)
	{
		if (wPOT != l_GLImgSize[0] || hPOT != l_GLImgSize[1])
		{
			// Free
			if (l_GLImgBuffer)
				Z_Free(l_GLImgBuffer);
		
			// Allocate
			l_GLImgBuffer = Z_Malloc(sizeof(uint32_t) * (wPOT * hPOT), PU_STATIC, NULL);
			l_GLImgSize[0] = wPOT;
			l_GLImgSize[1] = hPOT;
		}
	}
	
	/* Setup PBO Object */
	else
	{
		l_GLPBOBuffer = 14;
		glBindBuffer(PIXEL_UNPACK_BUFFER_EXT, l_GLPBOBuffer);
		glBufferData(PIXEL_UNPACK_BUFFER_EXT, wPOT * hPOT, NULL, GL_STREAM_DRAW);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wPOT, hPOT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	}
	
	/* Set image size */
	l_GLImgSize[2] = a_Width;
	l_GLImgSize[3] = a_Height;
	
	/* Allocate Buffer */
	I_VideoSetBuffer(a_Width, a_Height, a_Width, NULL, false);
	
	/* Success */
	return true;
}

/* I_StartupGraphics() -- Initializes graphics */
void I_StartupGraphics(void)
{
	/* Pre-initialize video */
	if (!I_VideoPreInit())
		return;
	
	/* Initialize GLUT */
	glutInit(&myargc, myargv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
		
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


