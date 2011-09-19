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

extern uint8_t* screens[5];

extern int dirtybox[4];

extern uint8_t gammatable[5][256];
extern consvar_t cv_ticrate;
extern consvar_t cv_usegamma;

// Allocates buffer screens, call before R_Init.
void V_Init(void);

// Set the current RGB palette lookup to use for palettized graphics
#define VPALSMOOTHCOUNT 2		// Must be power of 2, higher number means smoother palette transition
void V_SetPalette(int palettenum);

void V_SetPaletteLump(char* pal);

extern RGBA_t* pLocalPalette;

// Retrieve the ARGB value from a palette color index
#define V_GetColor(color)  (pLocalPalette[color&0xFF])

void V_CopyRect(int srcx, int srcy, int srcscrn, int width, int height, int destx, int desty, int destscrn);
void V_CopyRectTrans(int srcx, int srcy, int srcscrn, int width, int height, int destx, int desty, int destscrn, int trans);

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
void V_DrawBlock(int x, int y, int scrn, int width, int height, uint8_t* src);

// Reads a linear block of pixels into the view buffer.
void V_GetBlock(int x, int y, int scrn, int width, int height, uint8_t* dest);

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
void V_DrawTiltView(uint8_t* viewbuffer);

//added:05-04-98: test persp. correction !!
void V_DrawPerspView(uint8_t* viewbuffer, int aiming);

void VID_BlitLinearScreen(uint8_t* srcptr, uint8_t* destptr, int width, int height, int srcrowbytes, int destrowbytes);

// GhostlyDeath <July 8, 2009> -- Add FPS Counter
extern consvar_t cv_vid_drawfps;

/*******************************************************************************
********************************************************************************
*******************************************************************************/

// GhostlyDeath <July 8, 2009> -- Add FPS Counter
extern consvar_t cv_vid_drawfps;

/*************************
*** BASIC DRAWING CODE ***
*************************/

/* Normal */
#define VEX_NOSCALESTART		0x00000001	// Starting point is not scaled
#define VEX_NOSCALESCREEN		0x00000002	// Do not scale to screen
#define VEX_HOLLOW				0x00000004	// Do not fill with a solid color (for polygons and such)
#define VEX_NOFLOATSCALE		0x00000008	// Do not scale using floats
#define	VEX_FUZZY				0x00000010	// Draw as fuzzy
#define VEX_NOASPECTADJUST		0x00000020	// Ignore aspect ratio
#define VEX_HORIZFLIPPED		0x00000040	// Draw as horizontally flipped
#define VEX_VERTFLIPPED			0x00000080	// Draw as vertically flipped
#define VEX_NOSCALE160160		0x00000100	// Do not scale on 160x160
#define VEX_MAPTHENMASK			0x00000200	// Apply color mapping then apply color masking
#define VEX_SKIPEVEN			0x00000400	// Don't draw even pixels
#define VEX_SECONDBUFFER		0x00000800	// Draw in second video buffer

/* Color mapping */
typedef enum VEX_ColorList_s
{
	VEX_MAP_NONE,				// 0 No Color mapping
	VEX_MAP_RED,				// 1 Red
	VEX_MAP_ORANGE,				// 2 Orange
	VEX_MAP_YELLOW,				// 3 Yellow
	VEX_MAP_GREEN,				// 4 Green
	VEX_MAP_CYAN,				// 5 Cyan
	VEX_MAP_BLUE,				// 6 Blue
	VEX_MAP_MAGENTA,			// 7 Magenta
	
	VEX_MAP_BROWN,				// 8 Brown
	VEX_MAP_BRIGHTWHITE,		// 9 Bright white
	VEX_MAP_WHITE,				// a White
	VEX_MAP_GRAY,				// b Gray
	VEX_MAP_BLACK,				// c Black
	
	VEX_MAP_FUSCIA,				// d Yuck
	VEX_MAP_GOLD,				// e Gold
	VEX_MAP_TEKGREEN,			// f Technical Green
	
	NUMVEXCOLORS
} VEX_ColorList_t;

#define VEX_COLORMAPMASK		0x000F0000	// Mask of the colormap
#define VEX_COLORMAPSHIFT		16	// Color shift
#define VEX_COLORMAP(x)			(((x) << VEX_COLORMAPSHIFT) & VEX_COLORMAPMASK)

/* Color masking (Matches skin colors) */
#define VEX_COLORMASK			0x00F00000	// Mask of the colors
#define VEX_COLORSHIFT			20	// Shift

/* Transparency */
// MAX OF 16
typedef enum VEX_TransparencyList_e
{
	VEX_TRANSNONE,				// 0%
	
	VEX_TRANS10,				// 10%
	VEX_TRANS20,				// 20%
	VEX_TRANS30,				// 30%
	VEX_TRANS40,				// 40%
	VEX_TRANS50,				// 50%
	VEX_TRANS60,				// 60%
	VEX_TRANS70,				// 70%
	VEX_TRANS80,				// 80%
	VEX_TRANS90,				// 90%
	
	VEX_TRANSFULL,				// Invisible
	VEX_TRANSFIRE,				// Bright Effect
	VEX_TRANSFX1,
	
	NUMVEXTRANSPARENCIES,
	
	// Aliases
	VEX_TRANSMED = VEX_TRANS50,	// Medium
	VEX_TRANSHIGH = VEX_TRANS80,	// High transparency
	VEX_TRANSMORE = VEX_TRANS90,	// More transparency
} VEX_TransparencyList_t;

// Line
#define VEX_LINETRANSMASK		0x0F000000
#define VEX_LINETRANSSHIFT		24
#define VEX_LINETRANS(x)		(((x) << VEX_LINETRANSSHIFT) & VEX_LINETRANSMASK)

// Fill
#define VEX_FILLTRANSMASK		0xF0000000
#define VEX_FILLTRANSSHIFT		28
#define VEX_FILLTRANS(x)		(((x) << VEX_FILLTRANSSHIFT) & VEX_FILLTRANSMASK)

// Both
#define VEX_TRANSMASK			0xFF000000
#define VEX_TRANS(x)			(VEX_LINETRANS(x) | VEX_FILLTRANS(x))

/* Initialization */
void V_InitializeColormaps(void);
const uint8_t* V_ReturnColormapPtr(const VEX_ColorList_t Color);

/* Drawing Functions */
void V_DrawFadeConsBackEx(const uint32_t Flags, const int x1, const int y1, const int x2, const int y2);
void V_DrawColorBoxEx(uint32_t Flags, uint8_t LineColor, uint8_t FillColor, int32_t x, int32_t y, int32_t w, int32_t h);
void V_DrawPatchEx(const uint32_t Flags, const int x, const int y, const patch_t* const Patch, const uint8_t* const ExtraMap);

/* Compatability */
void V_DrawPatch(const int x, const int y, const int scrn, const patch_t* const patch);
void V_DrawMappedPatch(const int x, const int y, const int scrn, const patch_t* const patch, const uint8_t* const colormap);
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
	uint16_t Char;
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

// Ordering
#define VFO_COLORMASK		0x0000000F
#define VFO_COLOR(x)		((x) & VFO_COLORMASK)
#define VFO_TRANSMASK		0x000000F0
#define VFO_TRANSSHIFT		4
#define VFO_TRANS(x)		(((x) << VFO_TRANSSHIFT) & VFO_TRANSMASK)
#define VFO_RIGHTTOLEFT		0x00010000
#define VFO_CENTERED		0x00020000
#define VFO_NOSCALESTART	0x00040000
#define VFO_NOSCALEPATCH	0x00080000
#define VFO_NOFLOATSCALE	0x00100000
#define VFO_NOSCALELORES	0x00200000	// NOT IMPLEMENTED!

void V_MapGraphicalCharacters(void);

void V_WXMapGraphicCharsWAD(WX_WADFile_t* const a_WAD);
void V_WXClearGraphicCharsWAD(WX_WADFile_t* const a_WAD);
void V_WXMapGraphicCharsComposite(WX_WADFile_t* const a_VWAD);
void V_WXClearGraphicCharsComposite(void);

/* Misc */
void V_ExtWCharToMB(const uint16_t WChar, char* const MB);
int V_FontHeight(const VideoFont_t Font);
int V_FontWidth(const VideoFont_t Font);

/* Multibyte ASCII */
int V_DrawCharacterMB(const VideoFont_t Font, const uint32_t Options, const char* const MBChar, const int x, const int y, size_t* const BSkip,
                      uint32_t* a_OptionsMod);

/* ASCII */
int V_DrawCharacterA(const VideoFont_t Font, const uint32_t Options, const char Char, const int x, const int y);
int V_DrawStringA(const VideoFont_t Font, const uint32_t Options, const char* const String, const int x, const int y);
void V_StringDimensionsA(const VideoFont_t Font, const uint32_t Options, const char* const String, int* const Width, int* const Height);
int V_StringWidthA(const VideoFont_t Font, const uint32_t Options, const char* const String);
int V_StringHeightA(const VideoFont_t Font, const uint32_t Options, const char* const String);

/*******************************************************************************
********************************************************************************
*******************************************************************************/

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
