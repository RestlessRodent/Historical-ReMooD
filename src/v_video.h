// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
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
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2011 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION: Gamma correction LUT.
//              Functions to draw patches (by post) directly to screen.
//              Functions to blit a block to the screen.

#ifndef __V_VIDEO__
#define __V_VIDEO__

#include "doomdef.h"
#include "doomtype.h"
#include "r_defs.h"
#include "w_wad.h"

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

//added:05-02-98:kiktest : this draws a patch using translucency

//added:16-02-98: like V_DrawScaledPatch, plus translucency

// Draw a linear block of pixels into the view buffer.
void V_DrawBlock(int x, int y, int scrn, int width, int height, byte * src);

// Reads a linear block of pixels into the view buffer.
void V_GetBlock(int x, int y, int scrn, int width, int height, byte * dest);

// draw a pic_t, SCALED
void V_DrawScalePic(int x1, int y1, int scrn, int lumpnum /*pic_t*        pic */ );
void V_BlitScalePicExtern(int x1, int y1, int scrn, pic_t* pic);

void V_DrawRawScreen(int x, int y, int lumpnum, int width, int height);

void V_MarkRect(int x, int y, int width, int height);

//added:05-02-98: fill a box with a single color
void V_DrawFill(int x, int y, int w, int h, int c);
void V_DrawScreenFill(int x, int y, int w, int h, int c);
//added:06-02-98: fill a box with a flat as a pattern
void V_DrawFlatFill(int x, int y, int w, int h, int flatnum);

//added:10-02-98: fade down the screen buffer before drawing the menu over
void V_DrawFadeScreen(void);

//added:12-02-98:
void V_DrawTiltView(byte * viewbuffer);

//added:05-04-98: test persp. correction !!
void V_DrawPerspView(byte * viewbuffer, int aiming);

void VID_BlitLinearScreen(byte * srcptr, byte * destptr, int width,
						  int height, int srcrowbytes, int destrowbytes);

extern int FontBBaseLump;	// draw text with fontB (big font)

/*******************************************************************************
********************************************************************************
*******************************************************************************/

// GhostlyDeath <July 8, 2009> -- Add FPS Counter
extern consvar_t cv_vid_drawfps;

/*************************
*** BASIC DRAWING CODE ***
*************************/

/* Normal */
#define VEX_NOSCALESTART		0x00000001		// Starting point is not scaled
#define VEX_NOSCALESCREEN		0x00000002		// Do not scale to screen
#define VEX_HOLLOW				0x00000004		// Do not fill with a solid color (for polygons and such)
#define VEX_NOFLOATSCALE		0x00000008		// Do not scale using floats
#define	VEX_FUZZY				0x00000010		// Draw as fuzzy
#define VEX_NOASPECTADJUST		0x00000020		// Ignore aspect ratio
#define VEX_HORIZFLIPPED		0x00000040		// Draw as horizontally flipped
#define VEX_VERTFLIPPED			0x00000080		// Draw as vertically flipped
#define VEX_NOSCALE160160		0x00000100		// Do not scale on 160x160
#define VEX_MAPTHENMASK			0x00000200		// Apply color mapping then apply color masking
#define VEX_SKIPEVEN			0x00000400		// Don't draw even pixels
#define VEX_SECONDBUFFER		0x00000800		// Draw in second video buffer

/* Color mapping */
typedef enum VEX_ColorList_s
{
	VEX_MAP_NONE,								// No Color mapping
	VEX_MAP_RED,								// Red
	VEX_MAP_ORANGE,								// Orange
	VEX_MAP_YELLOW,								// Yellow
	VEX_MAP_GREEN,								// Green
	VEX_MAP_CYAN,								// Cyan
	VEX_MAP_BLUE,								// Blue
	VEX_MAP_MAGENTA,							// Magenta
	
	VEX_MAP_BROWN,								// Brown
	VEX_MAP_BRIGHTWHITE,						// Bright white
	VEX_MAP_WHITE,								// White
	VEX_MAP_GRAY,								// Gray
	VEX_MAP_BLACK,								// Black
	
	NUMVEXCOLORS
} VEX_ColorList_t;

#define VEX_COLORMAPWHITEBASE	VEX_MAP_WHITE
#define VEX_COLORMAPGRAYBASE	VEX_MAP_GRAY
#define VEX_COLORMAPORANGEBASE	VEX_MAP_ORANGE
#define VEX_COLORMAPGREENBASE	VEX_MAP_RED

#define VEX_COLORMAPMASK		0x000F0000		// Mask of the colormap
#define VEX_COLORMAPSHIFT		16				// Color shift
#define VEX_COLORMAPWHITE		(VEX_COLORMAPWHITEBASE<<VEX_COLORMAPSHIFT)
#define VEX_COLORMAPGRAY		(VEX_COLORMAPGRAYBASE<<VEX_COLORMAPSHIFT)
#define VEX_COLORMAPORANGE		(VEX_COLORMAPORANGEBASE<<VEX_COLORMAPSHIFT)
#define VEX_COLORMAPGREEN		(VEX_COLORMAPGREENBASE<<VEX_COLORMAPSHIFT)

/* Color masking (Matches skin colors) */
#define VEX_COLORMASK			0x00F00000		// Mask of the colors
#define VEX_COLORSHIFT			20				// Shift

/* Transparency */
// Base
#define VEX_BASETRANSMED		1				// Medium Transparency
#define VEX_BASETRANSHIGH		2				// High transparency
#define VEX_BASETRANSMORE		3				// More transparency
#define VEX_BASETRANSFIRE		4				// Fire Effect
#define	VEX_BASETRANSFX1		5				// Transparency Effect
#define VEX_BASETRANSFULL		6				// Completely Invisible

// Line
#define VEX_LINETRANSMASK		0x0F000000
#define VEX_LINETRANSSHIFT		24
#define VEX_LINETRANSMED		(VEX_BASETRANSMED<<VEX_LINETRANSSHIFT)
#define VEX_LINETRANSHIGH		(VEX_BASETRANSHIGH<<VEX_LINETRANSSHIFT)
#define VEX_LINETRANSMORE		(VEX_BASETRANSMORE<<VEX_LINETRANSSHIFT)
#define VEX_LINETRANSFIRE		(VEX_BASETRANSFIRE<<VEX_LINETRANSSHIFT)
#define	VEX_LINETRANSFX1		(VEX_BASETRANSFX1<<VEX_LINETRANSSHIFT)
#define VEX_LINETRANSFULL		(VEX_BASETRANSFULL<<VEX_LINETRANSSHIFT)

// Fill
#define VEX_FILLTRANSMASK		0xF0000000
#define VEX_FILLTRANSSHIFT		28
#define VEX_FILLTRANSMED		(VEX_BASETRANSMED<<VEX_FILLTRANSSHIFT)
#define VEX_FILLTRANSHIGH		(VEX_BASETRANSHIGH<<VEX_FILLTRANSSHIFT)
#define VEX_FILLTRANSMORE		(VEX_BASETRANSMORE<<VEX_FILLTRANSSHIFT)
#define VEX_FILLTRANSFIRE		(VEX_BASETRANSFIRE<<VEX_FILLTRANSSHIFT)
#define	VEX_FILLTRANSFX1		(VEX_BASETRANSFX1<<VEX_FILLTRANSSHIFT)
#define VEX_FILLTRANSFULL		(VEX_BASETRANSFULL<<VEX_FILLTRANSSHIFT)

// Both
#define VEX_TRANSMASK			0xFF000000
#define VEX_TRANSMED			(VEX_LINETRANSMED|VEX_FILLTRANSMED)
#define VEX_TRANSHIGH			(VEX_LINETRANSHIGH|VEX_FILLTRANSHIGH)
#define VEX_TRANSMORE			(VEX_LINETRANSMORE|VEX_FILLTRANSMORE)
#define VEX_TRANSFIRE			(VEX_LINETRANSFIRE|VEX_FILLTRANSFIRE)
#define	VEX_TRANSFX1			(VEX_LINETRANSFX1|VEX_FILLTRANSFX1)
#define VEX_TRANSFULL			(VEX_LINETRANSFULL|VEX_FILLTRANSFULL)

/* Initialization */
void V_InitializeColormaps(void);
const uint8_t* V_ReturnColormapPtr(const VEX_ColorList_t Color);

/* Drawing Functions */
void V_DrawFadeConsBackEx(const uint32_t Flags, const int x1, const int y1, const int x2, const int y2);
void V_DrawColorBoxEx(uint32_t Flags, uint8_t LineColor, uint8_t FillColor, int32_t x, int32_t y, int32_t w, int32_t h);
void V_DrawPatchEx(const uint32_t Flags, const int x, const int y, const patch_t* const Patch, const uint8_t* const ExtraMap);

/* Compatability */
void V_DrawPatch(const int x, const int y, const int scrn, const patch_t* const patch);
void V_DrawMappedPatch(const int x, const int y, const int scrn, const patch_t* const patch, const byte* const colormap);
void V_DrawScaledPatch(const int x, const int y, const int scrn, const patch_t* const patch);
void V_DrawTransPatch(const int x, const int y, const int scrn, const patch_t* const patch);
void V_DrawTranslucentPatch(const int x, const int y, const int scrn, const patch_t* const patch);
void V_DrawFadeScreen(void);
void V_DrawFill(int x, int y, int w, int h, int c);
void V_DrawScreenFill(int x, int y, int w, int h, int c);
void V_DrawFadeConsBack(int x1, int y1, int x2, int y2);

/**************
*** UNICODE ***
**************/

typedef enum
{
	/* Aliases */
	VFONT_SMALL,				// Doom, Heretic, Hexen, and Strife (alias to VFONT_SMALL_x)
	VFONT_LARGE,				// Doom, Heretic, Hexen, and Strife (alias to VFONT_LARGE_x)
	
	/* Real Fonts */
	VFONT_STATUSBARSMALL,		// Defined by Doom
	VFONT_PRBOOMHUD,			// Defined by PrBoom
	VFONT_OEM,					// Defined by ReMooD
	VFONT_USERSPACEA,			// User Defined Font (No port should use this at all, this is for the people!)
	VFONT_USERSPACEB,			// User Defined Font (No port should use this at all, this is for the people!)
	VFONT_USERSPACEC,			// User Defined Font (No port should use this at all, this is for the people!)
	VFONT_USERSPACED,			// User Defined Font (No port should use this at all, this is for the people!)
	VFONT_SMALL_DOOM,			// Small Doom Font
	VFONT_LARGE_DOOM,			// Large Doom Font
	VFONT_SMALL_HERETIC,		// Small Heretic Font
	VFONT_LARGE_HERETIC,		// Large Heretic Font
	VFONT_SMALL_STRIFE,			// Small Strife Font
	VFONT_LARGE_STRIFE,			// Large Strife Font
	
	NUMVIDEOFONTS
} VideoFont_t;

typedef struct UniChar_s
{
	wchar_t Char;
	char MB[5];
	
	struct patch_s* Patch;
	
	struct WadEntry_s* Entry;	// DEPRECATED
	WX_WADEntry_t* XEntry;
	
	struct UniChar_s* BuildTop;
	struct UniChar_s* BuildBottom;
} UniChar_t;

extern UniChar_t** CharacterGroups[NUMVIDEOFONTS];
extern UniChar_t* UnknownLink[NUMVIDEOFONTS];

/* Options */
// Font Coloring
#define VFONTOPTION_WHITE			VEX_MAP_WHITE
#define VFONTOPTION_GRAY			VEX_MAP_GRAY
#define VFONTOPTION_ORANGE			VEX_MAP_ORANGE
#define VFONTOPTION_RED				VEX_MAP_RED

// Ordering
#define VFONTOPTION_COLORMASK		0x0000000F
#define VFONTOPTION_RIGHTTOLEFT		0x00010000
#define VFONTOPTION_CENTERED		0x00020000
#define VFONTOPTION_NOSCALESTART	0x00040000
#define VFONTOPTION_NOSCALEPATCH	0x00080000
#define VFONTOPTION_NOFLOATSCALE	0x00100000
#define VFONTOPTION_NOSCALELORES	0x00200000	// NOT IMPLEMENTED!

void V_MapGraphicalCharacters(void);

void V_WXMapGraphicCharsWAD(WX_WADFile_t* const a_WAD);
void V_WXClearGraphicCharsWAD(WX_WADFile_t* const a_WAD);
void V_WXMapGraphicCharsComposite(WX_WADFile_t* const a_VWAD);
void V_WXClearGraphicCharsComposite(void);

/* Misc */
void V_ExtWCharToMB(const wchar_t WChar, char* const MB);
int V_FontHeight(const VideoFont_t Font);
int V_FontWidth(const VideoFont_t Font);

/* Multibyte ASCII */
int V_DrawCharacterMB(const VideoFont_t Font, const uint32_t Options, const char* const MBChar, const int x, const int y, size_t* const BSkip);

/* ASCII */
int V_DrawCharacterA(const VideoFont_t Font, const uint32_t Options, const char Char, const int x, const int y);
int V_DrawStringA(const VideoFont_t Font, const uint32_t Options, const char* const String, const int x, const int y);
void V_StringDimensionsA(const VideoFont_t Font, const uint32_t Options, const char* const String, int* const Width, int* const Height);
int V_StringWidthA(const VideoFont_t Font, const uint32_t Options, const char* const String);
int V_StringHeightA(const VideoFont_t Font, const uint32_t Options, const char* const String);

/* Compatability */
void V_DrawCharacter(int x, int y, int c);	//added:20-03-98: draw a single character
void V_DrawString(int x, int y, int option, char *string);	//added:05-02-98: draw a string using the hu_font
int V_StringWidth(char *string);	// Find string width from hu_font chars
int V_StringHeight(char *string);	// Find string height from hu_font chars
void V_DrawTextB(char *text, int x, int y);
void V_DrawTextBGray(char *text, int x, int y);
int V_TextBWidth(char *text);
int V_TextBHeight(char *text);

/*******************************************************************************
********************************************************************************
*******************************************************************************/

/* Pre-drawn srtings, to reduce CPU but increase mem usage */

typedef struct V_PDString_s V_PDString_t;

V_PDString_t* V_CreatePD(const VideoFont_t Font, const uint32_t Options, const char* const NewString, const int NewX, const int NewY);
void V_DeletePD(V_PDString_t* const PDStr);

void V_SetStringPD(V_PDString_t* const PDStr, const char* const NewString);
void V_SetPosPD(V_PDString_t* const PDStr, const int NewX, const int NewY);
void V_SetFontPD(V_PDString_t* const PDStr, const VideoFont_t Font);
void V_SetFlagsPD(V_PDString_t* const PDStr, const uint32_t Options);

void V_InvalidatePD(V_PDString_t* const PDStr);
void V_InvalidateAllPD(void);
void V_UpdatePD(V_PDString_t* const PDStr);
void V_RenderPD(V_PDString_t* const PDStr);

#endif

