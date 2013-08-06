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
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Gamma correction LUT.
//              Functions to draw patches (by post) directly to screen.
//              Functions to blit a block to the screen.

#ifndef __V_VIDEO__
#define __V_VIDEO__

//#include "doomdef.h"
#include "doomtype.h"
//#include "r_defs.h"
//#include "w_wad.h"

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

// Allocates buffer screens, call before R_Init.
void V_Init(void);

// Set the current RGB palette lookup to use for palettized graphics
#define VPALSMOOTHCOUNT 2		// Must be power of 2, higher number means smoother palette transition
void V_SetPalette(int palettenum);
uint8_t* V_GetPalette(int palettenum);
uint8_t* V_GetPaletteMapped(int palettenum);

void V_SetPaletteLump(char* pal);

extern RGBA_t* pLocalPalette;
extern uint8_t g_ThreePal[256][3];

// Retrieve the ARGB value from a palette color index
#define V_GetColor(color)  (pLocalPalette[color&0xFF])

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

//added:06-02-98: fill a box with a flat as a pattern
void V_DrawFlatFill(int x, int y, int w, int h, int flatnum);

//added:10-02-98: fade down the screen buffer before drawing the menu over
void V_DrawFadeScreen(void);

void VID_BlitLinearScreen(uint8_t* srcptr, uint8_t* destptr, int width, int height, int srcrowbytes, int destrowbytes);

/*******************************************************************************
********************************************************************************
*******************************************************************************/

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
#define VEX_IGNOREOFFSETS		0x00001000	// Ignore image origins
#define VEX_COLORSET			0x00002000	// Player Color Specified

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

#define VEX_COLORMAPMASK		UINT32_C(0x000F0000)	// Mask of the colormap
#define VEX_COLORMAPSHIFT		UINT32_C(16)	// Color shift
#define VEX_COLORMAP(x)			(((uint32_t)(x) << VEX_COLORMAPSHIFT) & VEX_COLORMAPMASK)

/* Color masking (Matches skin colors) */
#define VEX_COLORMASK			UINT32_C(0x00F00000)	// Mask of the colors
#define VEX_COLORSHIFT			UINT32_C(20)	// Shift
#define VEX_PCOLOR(x)			((((uint32_t)(x) << VEX_COLORSHIFT) & VEX_COLORMASK) | VEX_COLORSET)

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
void V_DrawColorBoxEx(const uint32_t a_Flags, const uint8_t a_Color, const int32_t a_x1, const int32_t a_y1, const int32_t a_x2, const int32_t a_y2);
void V_DrawColorMapEx(const uint32_t a_Flags, const uint8_t* const a_ColorMap, const int32_t a_x1, const int32_t a_y1, const int32_t a_x2, const int32_t a_y2);

void V_DrawFadeScreen(void);
void V_DrawFadeConsBack(int x1, int y1, int x2, int y2);

/**************
*** UNICODE ***
**************/

typedef enum
{
	/* Aliases */
	VFONT_SMALL,				// Doom, Heretic, Hexen, and Strife (alias to VFONT_SMALL_x)
	VFONT_LARGE,				// Doom, Heretic, Hexen, and Strife (alias to VFONT_LARGE_x)

	VFONT_LASTALIAS = VFONT_LARGE,
	
	/* Real Fonts */
	VFONT_STATUSBARSMALL,		// Defined by Doom
	VFONT_BOOMHUD,				// Defined by Boom
	VFONT_OEM,					// Defined by ReMooD
	VFONT_USERSPACEA,			// User Defined Font (No port should use this at all, this is for the people!)
	VFONT_USERSPACEB,			// User Defined Font (No port should use this at all, this is for the people!)
	VFONT_USERSPACEC,			// User Defined Font (No port should use this at all, this is for the people!)
	VFONT_USERSPACED,			// User Defined Font (No port should use this at all, this is for the people!)
	VFONT_SMALL_DOOM,			// Small Doom Font
	VFONT_LARGE_DOOM,			// Large Doom Font
	VFONT_SMALL_HERETIC,		// Small Heretic Font
	VFONT_LARGE_HERETIC,		// Large Heretic Font
	
	VFONT_STATUSBARLARGE,		// Large Status Bar Font (Numbers)
	
	VFONT_TERMINUS,				// Terminus Font
	
	NUMVIDEOFONTS
} VideoFont_t;

/* Options */

// Ordering
#define VFO_COLORMASK		0x0000000F
#define VFO_COLORSHIFT		0
#define VFO_COLOR(x)		(((x) << VFO_COLORSHIFT) & VFO_COLORMASK)
#define VFO_TRANSMASK		0x000000F0
#define VFO_TRANSSHIFT		4
#define VFO_TRANS(x)		(((x) << VFO_TRANSSHIFT) & VFO_TRANSMASK)
#define VFO_RIGHTTOLEFT		0x00010000
#define VFO_CENTERED		0x00020000
#define VFO_NOSCALESTART	0x00040000
#define VFO_NOSCALEPATCH	0x00080000
#define VFO_NOFLOATSCALE	0x00100000
#define VFO_NOSCALELORES	0x00200000	// NOT IMPLEMENTED!
#define VFO_LEFTFLOW		0x00400000	// Like RTL but not swapped
#define VFO_UNDERLINE		0x00800000	// Underline text

#define VFO_PCOLSET			0x10000000	// Player Color Specified
#define VFO_PCOLMASK		0x0F000000	// Player Color Mapping
#define VFO_PCOLSHIFT		24
#define VFO_PCOL(x)			(VFO_PCOLSET | (((x) << VFO_PCOLSHIFT) & VFO_PCOLMASK))

void V_MapGraphicalCharacters(void);

/* Misc */
uint16_t V_ExtMBToWChar(const char* MBChar, size_t* const BSkip);
size_t V_ExtWCharToMB(const uint16_t a_WChar, char* const a_MB);
int V_FontHeight(const VideoFont_t a_Font);
int V_FontWidth(const VideoFont_t a_Font);

/* Multibyte ASCII */
int V_DrawCharacterMB(const VideoFont_t a_Font, const uint32_t a_Options, const char* const a_MBChar, const int a_x, const int a_y, size_t* const a_BSkip,
                      uint32_t* a_OptionsMod);

/* ASCII */
int V_DrawCharacterA(const VideoFont_t a_Font, const uint32_t a_Options, const char a_Char, const int a_x, const int a_y);
int V_DrawStringA(const VideoFont_t a_Font, const uint32_t a_Options, const char* const a_String, const int a_x, const int a_y);
void V_StringDimensionsA(const VideoFont_t a_Font, const uint32_t a_Options, const char* const a_String, int* const a_Width, int* const a_Height);
int V_StringWidthA(const VideoFont_t a_Font, const uint32_t a_Options, const char* const a_String);
int V_StringHeightA(const VideoFont_t a_Font, const uint32_t a_Options, const char* const a_String);

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


/*******************************************************************************
********************************************************************************
*******************************************************************************/

/*** UNIVERSAL IMAGE API ***/

#define MAXUIANAME			12					// Max name for find cache

/*** CONSTANTS ***/

/* V_ImageType_t -- Native image type */
typedef enum V_ImageType_e
{
	VIT_PATCH,									// Image is a patch
	VIT_PIC,									// Image is a pic_t
	VIT_RAW,									// A raw image (flat)
	VIT_RGBA,									// RGBA Texture
	
	NUMVIMAGETYPES
} V_ImageType_t;

/* V_ColorPal_t -- Color palette for image */
typedef enum V_ColorPal_e
{
	VCP_NONE,									// No palette mapping
	VCP_DOOM,									// Doom Palette
	VCP_HERETIC,								// Heretic Palette
	
	NUMVCOLORPALS
} V_ColorPal_t;

/*** STRUCTURES ***/

/* V_Image_t -- A single image */
typedef struct V_Image_s
{
	/* Info */
	int32_t					Width;				// Image width
	int32_t					Height;				// Image height
	int32_t					Offset[2];			// Image offsets
	
	uint32_t				PixelCount;			// Number of pixels in image
	int32_t					TotalUsage;			// Total usage
	int32_t					UseCount[3];		// Usage count for data (patch, pic, raw)
	int8_t					NativeType;			// Native image type
	bool_t					HasTrans;			// Has transprency
	int						PUTagLevel;			// Current PU_ Tag
	bool_t					DoDelete;			// Do image deletion
	WadIndex_t				Index;				// Index of this image (for find)
	char					Name[MAXUIANAME];	// Name of the image (for find)
	uint32_t				NameHash;			// Hash for the name (if applicable)
	int32_t				Conf[NUMVIMAGETYPES];	// Confidence
	V_ColorPal_t			NativePal;			// Native Palette
	
	/* WAD Related */
	const struct WL_WADEntry_s*	wData;			// New WAD Access (WL)
	
	/* Data */
	struct patch_s*			dPatch;				// patch_t Compatible
	struct pic_s*			dPic;				// pic_t Compatible
	uint8_t*				dRaw;				// Raw image (flat)
	
	/* Cache Chain */
	struct V_Image_s*		iPrev;				// Previous image
	struct V_Image_s*		iNext;				// Next image
	
	/* Size */
	void* dPatchEnd;							// End of patch data
	uint32_t POTSize[2];						// Power of two size
	uint32_t GLRef[32];							// OpenGL Reference
	uint32_t GLSpotCount[32];					// Current Spot
} V_Image_t;

// Load and Destroy
V_Image_t* V_ImageLoadE(const WL_WADEntry_t* const a_Entry, const V_ColorPal_t a_Pal);
V_Image_t* V_ImageFindA(const char* const a_Name, const V_ColorPal_t a_Pal);
void V_ImageDestroy(V_Image_t* const a_Image);

// Access
int32_t V_ImageUsage(V_Image_t* const a_Image, const bool_t a_Use);
uint32_t V_ImageSizePos(V_Image_t* const a_Image, int32_t* const a_Width, int32_t* const a_Height, int32_t* const a_XOff, int32_t* const a_YOff);

// Get data for a specific format
const struct patch_s* V_ImageGetPatch(V_Image_t* const a_Image, size_t* const a_ByteSize);
const struct pic_s* V_ImageGetPic(V_Image_t* const a_Image, size_t* const a_ByteSize);
uint8_t* V_ImageGetRaw(V_Image_t* const a_Image, size_t* const a_ByteSize, const uint8_t a_Mask);

// Common Drawers
void V_ImageDrawScaledIntoBuffer(const uint32_t a_Flags, V_Image_t* const a_Image, const int32_t a_X, const int32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const fixed_t a_XScale, const fixed_t a_YScale, const uint8_t* const a_ExtraMap, uint8_t* const a_DestBuffer, const uint32_t a_DestPitch, const uint32_t a_DestWidth, const uint32_t a_DestHeight, const fixed_t a_VidXScaleX, const fixed_t a_VidXScaleY, const double a_VidFScaleX, const double a_VidFScaleY);
void V_ImageDrawScaled(const uint32_t a_Flags, V_Image_t* const a_Image, const int32_t a_X, const int32_t a_Y, const fixed_t a_XScale, const fixed_t a_YScale, const uint8_t* const a_ExtraMap);
void V_ImageDrawTiled(const uint32_t a_Flags, V_Image_t* const a_Image, const int32_t a_X, const int32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const uint8_t* const a_ExtraMap);
void V_ImageDraw(const uint32_t a_Flags, V_Image_t* const a_Image, const int32_t a_X, const int32_t a_Y, const uint8_t* const a_ExtraMap);

#endif

