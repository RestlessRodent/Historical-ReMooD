// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
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
// Copyright (C) 2011 GhostlyDeath <ghostlydeath@remood.org>
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

/* I_GLUploadMode_t -- Image upload mode */
typedef enum I_GLUploadMode_e
{
	IGLUM_AUTO,									// Automatic	
	
	IGLUM_PUTPIXEL,								// Put pixel
	IGLUM_DRAWPIXELS,							// Draw Pixels
	IGLUM_TEXTURE,								// Textures
	IGLUM_EXTPBO,								// GL_EXT_pixel_buffer_object
	IGLUM_ARBPBO,								// GL_ARB_pixel_buffer_object
} I_GLUploadMode_t;

/*************
*** LOCALS ***
*************/

static struct
{
	uint8_t c[3];
} l_GLPalette[256];								// OpenGL Palette

static I_GLUploadMode_t l_GLUpMode;				// Image upload mode
static l_GLUTWinRef = 0;						// GLUT Window reference

/****************
*** FUNCTIONS ***
****************/

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
}

/* I_FinishUpdate() -- Called after drawing a frame */
void I_FinishUpdate(void)
{
	uint8_t* Buffer;
	uint32_t w, h;
	uint32_t x, y;
	uint8_t Bit;
	
	/* Obtain the software buffer */
	Buffer = I_VideoSoftBuffer(&w, &h);
	
	// Failed?
	if (!Buffer)
		return;
	
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
	
	l_GLUpMode = IGLUM_PUTPIXEL;
	
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
			break;
			
			// EXT PBO
		case IGLUM_EXTPBO:
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
	
	/* Handle events in the GLUT loop */
	glutMainLoopEvent();
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
	VID_AddMode(320, 200, true);
	
}

/* I_SetVideoMode() -- Sets the current video mode */
bool_t I_SetVideoMode(const uint32_t a_Width, const uint32_t a_Height, const bool_t a_Fullscreen)
{
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
		l_GLUTWinRef = glutCreateWindow("ReMooD " REMOOD_FULLVERSIONSTRING);
		
		// Failed?
		if (!l_GLUTWinRef)
			return false;
		
		// Change properties
		glutSetWindow(l_GLUTWinRef);
		glutReshapeWindow(a_Width, a_Height);
	}
	
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
	
	/* Allocate Buffer */
	I_VideoSetBuffer(a_Width, a_Height, a_Width, NULL);
	
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
