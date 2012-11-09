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

#if defined(__REMOOD_OPENGL_SUPPORTED) && !defined(__REMOOD_OPENGL_CANCEL)

/***************
*** INCLUDES ***
***************/

#include <GL/gl.h>

#include "vhw_wrap.h"
#include "vhw_locl.h"
#include "console.h"
#include "dstrings.h"
#include "screen.h"

/*****************
*** STRUCTURES ***
*****************/

/* VHW_GLTextureSpot_t -- Local Storage of GL texture */
typedef struct VHW_GLTextureSpot_s
{
	GLuint glID;								// OpenGL ID of texture
	uint32_t SpotAlloc;							// Times spot allocated
	uint32_t Age;								// Age of texture
} VHW_GLTextureSpot_t;

/*************
*** LOCALS ***
*************/

/****************
*** CONSTANTS ***
****************/

#define GLUBYTETOCLAMP(x) (((double)(x)) * 0.00390625)

/* c_GLColorMap[] -- Normals to GL colors */
double c_GLColorMap[NUMVEXCOLORS][3] =
{
	{1.00, 1.00, 1.00},							// VEX_MAP_NONE
	{1.00, 0.00, 0.00},							// VEX_MAP_RED
	{1.00, 0.50, 0.00},							// VEX_MAP_ORANGE
	{1.00, 1.00, 0.00},							// VEX_MAP_YELLOW
	{0.00, 1.00, 0.00},							// VEX_MAP_GREEN
	{0.00, 1.00, 0.50},							// VEX_MAP_CYAN
	{0.00, 0.00, 1.00},							// VEX_MAP_BLUE
	{1.00, 0.00, 1.00},							// VEX_MAP_MAGENTA
	{0.50, 0.50, 0.00},							// VEX_MAP_BROWN
	{1.00, 1.00, 1.00},							// VEX_MAP_BRIGHTWHITE
	{0.75, 0.75, 0.75},							// VEX_MAP_WHITE
	{0.50, 0.50, 0.50},							// VEX_MAP_GRAY
	{0.25, 0.25, 0.25},							// VEX_MAP_BLACK
	{0.75, 0.00, 0.75},							// VEX_MAP_FUSCIA
	{0.90, 0.75, 0.00},							// VEX_MAP_GOLD
	{0.25, 0.75, 0.25},							// VEX_MAP_TEKGREEN
};

#define MAXGLTEXTURES				256			// Maximum GL textures
static VHW_GLTextureSpot_t l_GLStore[MAXGLTEXTURES];

/****************
*** FUNCTIONS ***
****************/

/* GLS_GetGLTexture() -- Gets texture from image */
GLuint GLS_GetGLTexture(V_Image_t* const a_Image, const uint32_t a_Flags)
{
	static uint32_t AgeCounter;
	uint32_t CurrentSpot;
	bool_t ReGen = false;
	uint32_t i, j, x, y, Base;
	uint8_t* RawTex, *TempData, Bit, VEXColor, *CMap;
	RGBA_t* PlayPal;
	
	double xx, yy, xa, ya, ix, iy;
	
	/* Get Color */
	VEXColor = (a_Flags & VEX_COLORMAPMASK) >> VEX_COLORMAPSHIFT;
	if (a_Flags & VEX_COLORSET)
		VEXColor = ((a_Flags & VEX_COLORMASK) >> VEX_COLORSHIFT) + 16;
	
	// Get Colormap
	CMap = V_ReturnColormapPtr(VEXColor);
	
	/* Get the textures current spot */
	CurrentSpot = a_Image->GLRef[VEXColor];
	
	// No spot?
	if (!CurrentSpot)
		ReGen = true;
	
	// Missed?
	if (l_GLStore[CurrentSpot].SpotAlloc != a_Image->GLSpotCount[VEXColor])
		ReGen = true;
	
	/* Needs regeneration? */
	if (ReGen)
	{
		// Replace oldest texture
		j = CurrentSpot;
		CurrentSpot = 0;
		for (i = 1; i < MAXGLTEXTURES; i++)
			if (!CurrentSpot ||
				!l_GLStore[i].SpotAlloc ||
				(!CurrentSpot && j && l_GLStore[i].Age < l_GLStore[j].Age) ||
				(CurrentSpot && l_GLStore[i].Age < l_GLStore[CurrentSpot].Age))
				CurrentSpot = i;
		
		// Increase count here and set age
		++l_GLStore[CurrentSpot].SpotAlloc;
		l_GLStore[CurrentSpot].Age = ++AgeCounter;
		l_GLStore[CurrentSpot].glID = CurrentSpot;
		
		// Get palette
		PlayPal = V_GetPalette(0);
		
		// Obtain raw texture
		RawTex = V_ImageGetRaw(a_Image, NULL, 0);
		
		// Generate
		TempData = Z_Malloc(sizeof(uint32_t) * (a_Image->POTSize[0] * a_Image->POTSize[1]), PU_STATIC, NULL);
		
		// Determine scale size
		xa = (double)a_Image->Width / (double)a_Image->POTSize[0];
		ya = (double)a_Image->Height / (double)a_Image->POTSize[1];
		
#if 1
		// Scale it to power of two
		for (yy = 0; yy < a_Image->POTSize[1]; yy += 1.0)
			for (xx = 0; xx < a_Image->POTSize[0]; xx += 1.0)
			{
				// Place to draw at
				Base = 4 * ((((uint32_t)yy) * a_Image->POTSize[0]) + ((uint32_t)xx));
				
				// Get input coords
				ix = xx * xa;
				iy = yy * ya;
				
				// Obtain bit to clone
				Bit = CMap[RawTex[(((uint32_t)iy) * a_Image->Width) + ((uint32_t)ix)]];
				
				// Draw
				TempData[Base] = PlayPal[Bit].s.red;
				TempData[Base + 1] = PlayPal[Bit].s.green;
				TempData[Base + 2] = PlayPal[Bit].s.blue;
				TempData[Base + 3] = 0xFF;
			}
#else
		for (y = 0; y < a_Image->Height; y++)
			for (x = 0; x < a_Image->Width; x++)
			{
				Base = 4 * ((y * a_Image->POTSize[0]) + x);
				Bit = RawTex[(y * a_Image->Width) + x];
				TempData[Base] = Bit;
				TempData[Base + 1] = Bit;
				TempData[Base + 2] = Bit;
				TempData[Base + 3] = 0xFF;
			}
#endif
		
		// Bind Texture
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, l_GLStore[CurrentSpot].glID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, a_Image->POTSize[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, a_Image->POTSize[0], a_Image->POTSize[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, TempData);
		
		// Free texture data
		Z_Free(TempData);
		
		// Done
		glDisable(GL_TEXTURE_2D);
	}
	
	/* Return reference to texture */
	a_Image->GLRef[VEXColor] = CurrentSpot;
	a_Image->GLSpotCount[VEXColor] = l_GLStore[CurrentSpot].SpotAlloc;
	return l_GLStore[CurrentSpot].glID;
}

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
	VEX_ColorList_t VEXColor;
	VEX_TransparencyList_t VEXTrans;
	static double cally;
	double junk, Alpha;
	GLuint TextureID;
	
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
	
	/* Get Color */
	VEXColor = (a_Flags & VEX_COLORMAPMASK) >> VEX_COLORMAPSHIFT;
	VEXTrans = (a_Flags & VEX_FILLTRANSMASK) >> VEX_FILLTRANSSHIFT;
	
	/* Load Texture */
	TextureID = GLS_GetGLTexture(a_Image, a_Flags);
	
	// Enable texture mapping
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, TextureID);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	//glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, c_GLColorMap[VEXColor]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glBlendFunc();
	
	// Which blending?
	if (VEXTrans == VEX_TRANSFIRE)
	{
		Alpha = 1.0;
	}
	else if (VEXTrans <= VEX_TRANSFULL)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		Alpha = 1.0 - (0.10 * (double)(VEXTrans));
	}
	else
		Alpha = 1.0;
	
	/* Place holder shape */
	glBegin(GL_QUADS);
		// Obtain color
		glColor4f(
				c_GLColorMap[VEXColor][0],
				c_GLColorMap[VEXColor][1],
				c_GLColorMap[VEXColor][2],
				Alpha
			);
		
		// Vertex it
		glTexCoord2d(0, 0);
		glVertex2f(sX, sY);
		glTexCoord2d(1, 0);
		glVertex2f(eX, sY);
		glTexCoord2d(1, 1);
		glVertex2f(eX, eY);
		glTexCoord2d(0, 1);
		glVertex2f(sX, eY);
	glEnd();
	
	/* Done with texture */
	if (VEXTrans == VEX_TRANSFIRE)
	{
	}
	else if (VEXTrans <= VEX_TRANSFULL)
		glDisable(GL_BLEND);
	
	glDisable(GL_TEXTURE_2D);
}

/* VHW_GL_SetViewport() -- Sets the viewport of the screen */
void VHW_GL_SetViewport(const int32_t a_X, const int32_t a_Y, const uint32_t a_W, const uint32_t a_H)
{
	glViewport(a_X, a_Y, a_W, a_H);
	glScissor(a_X, a_Y, a_W, a_H);
}

/* VHW_GL_HUDBlurBack() -- Blurs the background */
void VHW_GL_HUDBlurBack(const uint32_t a_Flags, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2)
{
}

/* VHW_GL_HUDDrawBox() -- Draws colorized box */
void VHW_GL_HUDDrawBox(const uint32_t a_Flags, const uint8_t a_R, const uint8_t a_G, const uint8_t a_B, int32_t a_X1, int32_t a_Y1, int32_t a_X2, int32_t a_Y2)
{
}

/* VHW_GL_ClearScreen() -- Clears the screen */
void VHW_GL_ClearScreen(const uint8_t a_R, const uint8_t a_G, const uint8_t a_B)
{
	glClearColor(GLUBYTETOCLAMP(a_R), GLUBYTETOCLAMP(a_G), GLUBYTETOCLAMP(a_B), 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

#endif

