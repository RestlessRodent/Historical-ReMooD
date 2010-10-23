// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.org/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           TetrisMaster512        (tetrismaster512@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2010 The ReMooD Team..
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
// DESCRIPTION: Gamma correction LUT.
//              Functions to draw patches (by post) directly to screen.
//              Functions to blit a block to the screen.

#ifndef __V_VIDEO__
#define __V_VIDEO__

#include "doomdef.h"
#include "doomtype.h"
#include "r_defs.h"

//
// VIDEO
//

//added:18-02-98:centering offset for the scaled graphics,
//               this is normally temporarily changed by m_menu.c only.
//               The rest of the time it should be zero.
extern int scaledofs;

// Screen 0 is the screen updated by I_Update screen.
// Screen 1 is an extra buffer.

extern byte *screens[5];

extern int dirtybox[4];

extern byte gammatable[5][256];
extern consvar_t cv_ticrate;
extern consvar_t cv_usegamma;

// Allocates buffer screens, call before R_Init.
void V_Init(void);

// Set the current RGB palette lookup to use for palettized graphics
void V_SetPalette(int palettenum);

void V_SetPaletteLump(char *pal);

extern RGBA_t *pLocalPalette;

// Retrieve the ARGB value from a palette color index
#define V_GetColor(color)  (pLocalPalette[color&0xFF])

void V_CopyRect
	(int srcx, int srcy, int srcscrn, int width, int height, int destx, int desty, int destscrn);
void V_CopyRectTrans
	(int srcx, int srcy, int srcscrn, int width, int height, int destx, int desty, int destscrn, int trans);


//added:03-02-98:like V_DrawPatch, + using a colormap.
void V_DrawMappedPatch(int x, int y, int scrn, patch_t * patch, byte * colormap);

//added:05-02-98:V_DrawPatch scaled 2,3,4 times size and position.

// flags hacked in scrn (not supported by all functions (see src))
#define V_NOSCALESTART       0x010000	// dont scale x,y, start coords
#define V_SCALESTART         0x020000	// scale x,y, start coords
#define V_SCALEPATCH         0x040000	// scale patch
#define V_NOSCALEPATCH       0x080000	// don't scale patch
#define V_WHITEMAP           0x100000	// draw white (for v_drawstring)
#define V_FLIPPEDPATCH       0x200000	// flipped in y
#define V_TRANSLUCENTPATCH   0x400000	// draw patch translucent
#define V_GRAYMAP			 0x800000	// draw as gray
#define V_ORANGEMAP			0x1000000	// draw as orange!
#define V_NOSCALELOWRES		0x2000000	// Don't scale on 160x160
#define V_NOFLOATSCALE		0x3000000	// Don't scale using floats

// default params : scale patch and scale start
void V_DrawScaledPatch(int x, int y, int scrn,	// + flags
					   patch_t * patch);

//added:05-02-98:kiktest : this draws a patch using translucency
void V_DrawTransPatch(int x, int y, int scrn, patch_t * patch);

//added:16-02-98: like V_DrawScaledPatch, plus translucency
void V_DrawTranslucentPatch(int x, int y, int scrn, patch_t * patch);

void V_DrawPatch(int x, int y, int scrn, patch_t * patch);

// Draw a linear block of pixels into the view buffer.
void V_DrawBlock(int x, int y, int scrn, int width, int height, byte * src);

// Reads a linear block of pixels into the view buffer.
void V_GetBlock(int x, int y, int scrn, int width, int height, byte * dest);

// draw a pic_t, SCALED
void V_DrawScalePic(int x1, int y1, int scrn, int lumpnum /*pic_t*        pic */ );

void V_DrawRawScreen(int x, int y, int lumpnum, int width, int height);

void V_MarkRect(int x, int y, int width, int height);

//added:05-02-98: fill a box with a single color
void V_DrawFill(int x, int y, int w, int h, int c);
void V_DrawScreenFill(int x, int y, int w, int h, int c);
//added:06-02-98: fill a box with a flat as a pattern
void V_DrawFlatFill(int x, int y, int w, int h, int flatnum);

//added:10-02-98: fade down the screen buffer before drawing the menu over
void V_DrawFadeScreen(void);

//added:20-03-98: test console
void V_DrawFadeConsBack(int x1, int y1, int x2, int y2);

//added:12-02-98:
void V_DrawTiltView(byte * viewbuffer);

//added:05-04-98: test persp. correction !!
void V_DrawPerspView(byte * viewbuffer, int aiming);

void VID_BlitLinearScreen(byte * srcptr, byte * destptr, int width,
						  int height, int srcrowbytes, int destrowbytes);

extern int FontBBaseLump;	// draw text with fontB (big font)

/******************************************************************************/

typedef enum
{
	VFONT_SMALL,				// Doom, Heretic, Hexen, and Strife
	VFONT_LARGE,				// Doom, Heretic, Hexen, and Strife
	VFONT_STATUSBARSMALL,		// Defined by Doom
	VFONT_PRBOOMHUD,			// Defined by PrBoom
	VFONT_OEM,					// Defined by ReMooD
	VFONT_USERSPACEA,			// User Defined Font (No port should use this at all, this is for the people!)
	VFONT_USERSPACEB,			// User Defined Font (No port should use this at all, this is for the people!)
	
	NUMVIDEOFONTS
} VideoFont_t;

typedef struct UniChar_s
{
	wchar_t Char;
	struct patch_s* Patch;
	struct WadEntry_s* Entry;
} UniChar_t;

extern UniChar_t** CharacterGroups[NUMVIDEOFONTS];
extern UniChar_t* UnknownLink[NUMVIDEOFONTS];

/* Options */
// Font Coloring
#define VFONTOPTION_WHITE			0x00000001
#define VFONTOPTION_GRAY			0x00000002
#define VFONTOPTION_ORANGE			0x00000003

// Ordering
#define VFONTOPTION_RIGHTTOLEFT		0x00010000
#define VFONTOPTION_CENTERED		0x00020000
#define VFONTOPTION_NOSCALESTART	0x00040000
#define VFONTOPTION_NOSCALEPATCH	0x00080000
#define VFONTOPTION_NOFLOATSCALE	0x00100000

void V_MapGraphicalCharacters(void);

/* ASCII */
int V_DrawCharacterA(VideoFont_t Font, UInt32 Options, char Char, int x, int y);
int V_DrawStringA(VideoFont_t Font, UInt32 Options, char* String, int x, int y);
void V_StringDimensionsA(VideoFont_t Font, UInt32 Options, char* String, int* Width, int* Height);

/* Unicode */
int V_DrawCharacterW(VideoFont_t Font, UInt32 Options, wchar_t WChar, int x, int y);
int V_DrawStringW(VideoFont_t Font, UInt32 Options, wchar_t* WString, int x, int y);
void V_StringDimensionsW(VideoFont_t Font, UInt32 Options, wchar_t* WString, int* Width, int* Height);
int V_StringWidthW(VideoFont_t Font, UInt32 Options, wchar_t* WString);
int V_StringHeightW(VideoFont_t Font, UInt32 Options, wchar_t* WString);

/* Compatability */
void V_DrawCharacter(int x, int y, int c);	//added:20-03-98: draw a single character
void V_DrawString(int x, int y, int option, char *string);	//added:05-02-98: draw a string using the hu_font
int V_StringWidth(char *string);	// Find string width from hu_font chars
int V_StringHeight(char *string);	// Find string height from hu_font chars
void V_DrawTextB(char *text, int x, int y);
void V_DrawTextBGray(char *text, int x, int y);
int V_TextBWidth(char *text);
int V_TextBHeight(char *text);

#endif

