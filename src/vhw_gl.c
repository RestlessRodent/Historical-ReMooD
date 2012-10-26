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
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION: Hardware Video Drawer

/***************
*** INCLUDES ***
***************/

#include <GL/gl.h>

#include "vhw_wrap.h"
#include "vhw_locl.h"
#include "console.h"
#include "dstrings.h"
#include "screen.h"

/*************
*** LOCALS ***
*************/

static vhw_t __vhw_junk;

/****************
*** FUNCTIONS ***
****************/

/* GLS_HUDMode() -- Switches to HUD Drawing Mode */
static void GLS_HUDMode(void)
{
	/* Set view port */
	glViewport(0, 0, vid.width, vid.height);
	
	/* Orthographic projection, from top left origin */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glOrtho(0, (GLint)vid.width, (GLint)vid.height, 0, 1, -1);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/* VHW_GL_HUDDrawLine() -- Draws HUD Line */
void VHW_GL_HUDDrawLine(const vhwrgb_t a_RGB, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2)
{
	/* Enter HUD Drawing Mode */
	GLS_HUDMode();
}

/* VHW_GL_HUDDrawImageComplex() -- Draws complex image onto the screen */
void VHW_GL_HUDDrawImageComplex(const uint32_t a_Flags, V_Image_t* const a_Image, const int32_t a_X, const int32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const fixed_t a_XScale, const fixed_t a_YScale, const uint8_t* const a_ExtraMap)
{
	double sX, sY, eX, eY;
	static double cally;
	double junk;
	
	/* Enter HUD Drawing Mode */
	GLS_HUDMode();
	
	/* Calculate position */
	// Start location on screen
	sX = a_X;
	sY = a_Y;
	
	// Scale to match 320x200
	if ((a_Flags & VEX_NOSCALESTART) == 0)
	{
		sX *= vid.fdupx;
		sY *= vid.fdupy;
	}
	
	// End location on screen
	eX = sX + ((double)a_Width * (double)FIXED_TO_FLOAT(a_XScale));
	eY = sY + ((double)a_Height * (double)FIXED_TO_FLOAT(a_YScale));
	
	/* Place holder shape */
	glBegin(GL_QUADS);
		glColor3f(1.0, modf(cally, &junk), modf(cally, &junk));
		glVertex2f(sX, sY);
		glVertex2f(eX, sY);
		glVertex2f(eX, eY);
		glVertex2f(sX, eY);
	glEnd();
	
	cally += 0.08;
}

/* VHW_GL_SetViewport() -- Sets the viewport of the screen */
void VHW_GL_SetViewport(const int32_t a_X, const int32_t a_Y, const uint32_t a_W, const uint32_t a_H)
{
	glViewport(a_X, a_Y, a_W, a_H);
}

/* VHW_GL_HUDBlurBack() -- Blurs the background */
void VHW_GL_HUDBlurBack(const uint32_t a_Flags, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2)
{
}

/* VHW_GL_HUDDrawBox() -- Draws colorized box */
void VHW_GL_HUDDrawBox(const uint32_t a_Flags, const uint8_t a_R, const uint8_t a_G, const uint8_t a_B, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2)
{
}


