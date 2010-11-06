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
// DESCRIPTION:
//      Gamma correction LUT stuff.
//      Functions to draw patches (by post) directly to screen.
//      Functions to blit a block to the screen.

#include "doomdef.h"
#include "doomstat.h"
#include "r_local.h"
#include "v_video.h"
#include "hu_stuff.h"
#include "r_draw.h"
#include "console.h"

#include "i_video.h"
#include "z_zone.h"
#include "dstrings.h"
#include "i_system.h"

// Each screen is [vid.width*vid.height];
byte *screens[5];

CV_PossibleValue_t gamma_cons_t[] = { {0, "MIN"}
, {4, "MAX"}
, {0, NULL}
};
void CV_usegamma_OnChange(void);

consvar_t cv_ticrate = { "vid_ticrate", "0", 0, CV_OnOff, NULL };
consvar_t cv_usegamma = { "gamma", "0", CV_SAVE | CV_CALL, gamma_cons_t, CV_usegamma_OnChange };

// Now where did these came from?
byte gammatable[5][256] = {
	{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
	 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
	 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
	 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
	 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96,
	 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
	 112,
	 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
	 128,
	 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142,
	 143,
	 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158,
	 159,
	 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174,
	 175,
	 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190,
	 191,
	 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206,
	 207,
	 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222,
	 223,
	 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238,
	 239,
	 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,
	 255},

	{2, 4, 5, 7, 8, 10, 11, 12, 14, 15, 16, 18, 19, 20, 21, 23, 24, 25, 26, 27,
	 29, 30, 31,
	 32, 33, 34, 36, 37, 38, 39, 40, 41, 42, 44, 45, 46, 47, 48, 49, 50, 51, 52,
	 54, 55,
	 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 69, 70, 71, 72, 73, 74, 75,
	 76, 77,
	 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96,
	 97, 98,
	 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
	 114,
	 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129,
	 129,
	 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144,
	 145,
	 146, 147, 148, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
	 160,
	 161, 162, 163, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174,
	 175,
	 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 186, 187, 188,
	 189,
	 190, 191, 192, 193, 194, 195, 196, 196, 197, 198, 199, 200, 201, 202, 203,
	 204,
	 205, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 214, 215, 216, 217,
	 218,
	 219, 220, 221, 222, 222, 223, 224, 225, 226, 227, 228, 229, 230, 230, 231,
	 232,
	 233, 234, 235, 236, 237, 237, 238, 239, 240, 241, 242, 243, 244, 245, 245,
	 246,
	 247, 248, 249, 250, 251, 252, 252, 253, 254, 255},

	{4, 7, 9, 11, 13, 15, 17, 19, 21, 22, 24, 26, 27, 29, 30, 32, 33, 35, 36,
	 38, 39, 40, 42,
	 43, 45, 46, 47, 48, 50, 51, 52, 54, 55, 56, 57, 59, 60, 61, 62, 63, 65, 66,
	 67, 68, 69,
	 70, 72, 73, 74, 75, 76, 77, 78, 79, 80, 82, 83, 84, 85, 86, 87, 88, 89, 90,
	 91, 92, 93,
	 94, 95, 96, 97, 98, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
	 111, 112,
	 113, 114, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126,
	 127, 128,
	 129, 130, 131, 132, 133, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142,
	 143, 144,
	 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 153, 154, 155, 156, 157,
	 158, 159,
	 160, 160, 161, 162, 163, 164, 165, 166, 166, 167, 168, 169, 170, 171, 172,
	 172, 173,
	 174, 175, 176, 177, 178, 178, 179, 180, 181, 182, 183, 183, 184, 185, 186,
	 187, 188,
	 188, 189, 190, 191, 192, 193, 193, 194, 195, 196, 197, 197, 198, 199, 200,
	 201, 201,
	 202, 203, 204, 205, 206, 206, 207, 208, 209, 210, 210, 211, 212, 213, 213,
	 214, 215,
	 216, 217, 217, 218, 219, 220, 221, 221, 222, 223, 224, 224, 225, 226, 227,
	 228, 228,
	 229, 230, 231, 231, 232, 233, 234, 235, 235, 236, 237, 238, 238, 239, 240,
	 241, 241,
	 242, 243, 244, 244, 245, 246, 247, 247, 248, 249, 250, 251, 251, 252, 253,
	 254, 254,
	 255},

	{8, 12, 16, 19, 22, 24, 27, 29, 31, 34, 36, 38, 40, 41, 43, 45, 47, 49, 50,
	 52, 53, 55,
	 57, 58, 60, 61, 63, 64, 65, 67, 68, 70, 71, 72, 74, 75, 76, 77, 79, 80, 81,
	 82, 84, 85,
	 86, 87, 88, 90, 91, 92, 93, 94, 95, 96, 98, 99, 100, 101, 102, 103, 104,
	 105, 106, 107,
	 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,
	 123, 124,
	 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 135, 136, 137, 138,
	 139, 140,
	 141, 142, 143, 143, 144, 145, 146, 147, 148, 149, 150, 150, 151, 152, 153,
	 154, 155,
	 155, 156, 157, 158, 159, 160, 160, 161, 162, 163, 164, 165, 165, 166, 167,
	 168, 169,
	 169, 170, 171, 172, 173, 173, 174, 175, 176, 176, 177, 178, 179, 180, 180,
	 181, 182,
	 183, 183, 184, 185, 186, 186, 187, 188, 189, 189, 190, 191, 192, 192, 193,
	 194, 195,
	 195, 196, 197, 197, 198, 199, 200, 200, 201, 202, 202, 203, 204, 205, 205,
	 206, 207,
	 207, 208, 209, 210, 210, 211, 212, 212, 213, 214, 214, 215, 216, 216, 217,
	 218, 219,
	 219, 220, 221, 221, 222, 223, 223, 224, 225, 225, 226, 227, 227, 228, 229,
	 229, 230,
	 231, 231, 232, 233, 233, 234, 235, 235, 236, 237, 237, 238, 238, 239, 240,
	 240, 241,
	 242, 242, 243, 244, 244, 245, 246, 246, 247, 247, 248, 249, 249, 250, 251,
	 251, 252,
	 253, 253, 254, 254, 255},

	{16, 23, 28, 32, 36, 39, 42, 45, 48, 50, 53, 55, 57, 60, 62, 64, 66, 68, 69,
	 71, 73, 75, 76,
	 78, 80, 81, 83, 84, 86, 87, 89, 90, 92, 93, 94, 96, 97, 98, 100, 101, 102,
	 103, 105, 106,
	 107, 108, 109, 110, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,
	 123, 124,
	 125, 126, 128, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139,
	 140, 141,
	 142, 143, 143, 144, 145, 146, 147, 148, 149, 150, 150, 151, 152, 153, 154,
	 155, 155,
	 156, 157, 158, 159, 159, 160, 161, 162, 163, 163, 164, 165, 166, 166, 167,
	 168, 169,
	 169, 170, 171, 172, 172, 173, 174, 175, 175, 176, 177, 177, 178, 179, 180,
	 180, 181,
	 182, 182, 183, 184, 184, 185, 186, 187, 187, 188, 189, 189, 190, 191, 191,
	 192, 193,
	 193, 194, 195, 195, 196, 196, 197, 198, 198, 199, 200, 200, 201, 202, 202,
	 203, 203,
	 204, 205, 205, 206, 207, 207, 208, 208, 209, 210, 210, 211, 211, 212, 213,
	 213, 214,
	 214, 215, 216, 216, 217, 217, 218, 219, 219, 220, 220, 221, 221, 222, 223,
	 223, 224,
	 224, 225, 225, 226, 227, 227, 228, 228, 229, 229, 230, 230, 231, 232, 232,
	 233, 233,
	 234, 234, 235, 235, 236, 236, 237, 237, 238, 239, 239, 240, 240, 241, 241,
	 242, 242,
	 243, 243, 244, 244, 245, 245, 246, 246, 247, 247, 248, 248, 249, 249, 250,
	 250, 251,
	 251, 252, 252, 253, 254, 254, 255, 255}
};

// local copy of the palette for V_GetColor()
RGBA_t *pLocalPalette = NULL;

// keep a copy of the palette so that we can get the RGB
// value for a color index at any time.
void LoadPalette(char *lumpname)
{
	int i, palsize;
	byte *usegamma = gammatable[cv_usegamma.value];
	byte *pal;

	i = W_GetNumForName(lumpname);
	palsize = W_LumpLength(i) / 3;
	if (pLocalPalette)
		Z_Free(pLocalPalette);

	pLocalPalette = Z_Malloc(sizeof(RGBA_t) * palsize, PU_STATIC, NULL);

	pal = W_CacheLumpNum(i, PU_CACHE);

	for (i = 0; i < palsize; i++)
	{
		pLocalPalette[i].s.red = usegamma[*pal++];
		pLocalPalette[i].s.green = usegamma[*pal++];
		pLocalPalette[i].s.blue = usegamma[*pal++];
//        if( (i&0xff) == HWR_PATCHES_CHROMAKEY_COLORINDEX )
//            pLocalPalette[i].s.alpha = 0;
//        else
		pLocalPalette[i].s.alpha = 0xff;
	}
}

// -------------+
// V_SetPalette : Set the current palette to use for palettized graphics
//              : (that is, most if not all of Doom's original graphics)
// -------------+
void V_SetPalette(int palettenum)
{
	if (!pLocalPalette)
		LoadPalette("PLAYPAL");
	I_SetPalette(&pLocalPalette[palettenum * 256]);
}

void V_SetPaletteLump(char *pal)
{
	LoadPalette(pal);
	I_SetPalette(pLocalPalette);
}

void CV_usegamma_OnChange(void)
{
	// reload palette
	LoadPalette("PLAYPAL");
	V_SetPalette(0);
}

//added:18-02-98: this is an offset added to the destination address,
//                for all SCALED graphics. When the menu is displayed,
//                it is TEMPORARILY set to vid.centerofs, the rest of
//                the time it should be zero.
//                The menu is scaled, a round multiple of the original
//                pixels to keep the graphics clean, then it is centered
//                a little, but excepeted the menu, scaled graphics don't
//                have to be centered. Set by m_menu.c, and SCR_Recalc()
int scaledofs;

// V_MarkRect : this used to refresh only the parts of the screen
//              that were modified since the last screen update
//              it is useless today
//
int dirtybox[4];
void V_MarkRect(int x, int y, int width, int height)
{
	M_AddToBox(dirtybox, x, y);
	M_AddToBox(dirtybox, x + width - 1, y + height - 1);
}

static int QuickRound(float x)
{
	if ((x - (int)x) > 0.5)
		return x + 1;
	else
		return x;
}

//
// V_CopyRect
//
void V_CopyRect(int srcx, int srcy, int srcscrn, int width, int height,
				int destx, int desty, int destscrn)
{
	byte *src;
	byte *dest;

	if (!graphics_started)
		return;
		
	// WARNING don't mix
	if ((srcscrn & V_SCALESTART) || (destscrn & V_SCALESTART))
	{
		if ((srcscrn & V_NOFLOATSCALE) || (destscrn & V_NOFLOATSCALE))
		{
			srcx *= vid.dupx;
			srcy *= vid.dupy;
			width *= vid.dupx;
			height *= vid.dupy;
			destx *= vid.dupx;
			desty *= vid.dupy;
		}
		else
		{
			srcx = QuickRound(srcx * vid.fdupx);
			srcy = QuickRound(srcy * vid.fdupy);
			width = QuickRound(width * vid.fdupx);
			height = QuickRound(height * vid.fdupy);
			destx = QuickRound(destx * vid.fdupx);
			desty = QuickRound(desty * vid.fdupy);
		}
	}
	srcscrn &= 0xffff;
	destscrn &= 0xffff;

#ifdef RANGECHECK
	if (srcx < 0 || srcx + width > vid.width || srcy < 0 ||
		srcy + height > vid.height || destx < 0 || destx + width > vid.width ||
		desty < 0 || desty + height > vid.height || (unsigned)srcscrn > 4 || (unsigned)destscrn > 4)
	{
		I_Error("Bad V_CopyRect %d %d %d %d %d %d %d %d", srcx, srcy, srcscrn,
				width, height, destx, desty, destscrn);
	}
#endif
	V_MarkRect(destx, desty, width, height);

#ifdef DEBUG
	CONS_Printf("V_CopyRect: vidwidth %d screen[%d]=%x to screen[%d]=%x\n",
				vid.width, srcscrn, screens[srcscrn], destscrn, screens[destscrn]);
	CONS_Printf
		("..........: srcx %d srcy %d width %d height %d destx %d desty %d\n",
		 srcx, srcy, width, height, destx, desty);
#endif

	src = screens[srcscrn] + vid.width * srcy + srcx;
	dest = screens[destscrn] + vid.width * desty + destx;

	for (; height > 0; height--)
	{
		memcpy(dest, src, width);
		src += vid.width;
		dest += vid.width;
	}
}

//
// V_CopyRectTrans (GhostlyDeath --transparent copy)
//
void V_CopyRectTrans(int srcx, int srcy, int srcscrn, int width, int height,
				int destx, int desty, int destscrn, int trans)
{
	byte *src;
	byte *dest;
	int i;
	
	if (!graphics_started)
		return;

	// WARNING don't mix
	if ((srcscrn & V_SCALESTART) || (destscrn & V_SCALESTART))
	{
		if ((srcscrn & V_NOFLOATSCALE) || (destscrn & V_NOFLOATSCALE))
		{
			srcx *= vid.dupx;
			srcy *= vid.dupy;
			width *= vid.dupx;
			height *= vid.dupy;
			destx *= vid.dupx;
			desty *= vid.dupy;
		}
		else
		{
			srcx *= QuickRound(vid.fdupx);
			srcy *= QuickRound(vid.fdupy);
			width *= QuickRound(vid.fdupx);
			height *= QuickRound(vid.fdupy);
			destx *= QuickRound(vid.fdupx);
			desty *= QuickRound(vid.fdupy);
		}
	}
	srcscrn &= 0xffff;
	destscrn &= 0xffff;

#ifdef RANGECHECK
	if (srcx < 0 || srcx + width > vid.width || srcy < 0 ||
		srcy + height > vid.height || destx < 0 || destx + width > vid.width ||
		desty < 0 || desty + height > vid.height || (unsigned)srcscrn > 4 || (unsigned)destscrn > 4)
	{
		I_Error("Bad V_CopyRect %d %d %d %d %d %d %d %d", srcx, srcy, srcscrn,
				width, height, destx, desty, destscrn);
	}
#endif
	V_MarkRect(destx, desty, width, height);

#ifdef DEBUG
	CONS_Printf("V_CopyRect: vidwidth %d screen[%d]=%x to screen[%d]=%x\n",
				vid.width, srcscrn, screens[srcscrn], destscrn, screens[destscrn]);
	CONS_Printf
		("..........: srcx %d srcy %d width %d height %d destx %d desty %d\n",
		 srcx, srcy, width, height, destx, desty);
#endif

	src = screens[srcscrn] + vid.width * srcy + srcx;
	dest = screens[destscrn] + vid.width * desty + destx;

	for (; height > 0; height--)
	{	
		for (i = 0; i < width; i++)
		{
			*dest = *((transtables + (trans * 0x10000)) + ((src[srcx >> FRACBITS] << 8) & 0xFF00) + (*dest & 0xFF));
			dest++;
			src++;
		}
		
		src += vid.width - width;
		dest += vid.width - width;
		
		//memcpy(dest, src, width);
		//src += vid.width;
		//dest += vid.width;
	}
}

// --------------------------------------------------------------------------
// Copy a rectangular area from one bitmap to another (8bpp)
// srcPitch, destPitch : width of source and destination bitmaps
// --------------------------------------------------------------------------
void VID_BlitLinearScreen(byte * srcptr, byte * destptr, int width, int height,
						  int srcrowbytes, int destrowbytes)
{
	if (srcrowbytes == destrowbytes)
		memcpy(destptr, srcptr, srcrowbytes * height);
	else
	{
		while (height--)
		{
			memcpy(destptr, srcptr, width);

			destptr += destrowbytes;
			srcptr += srcrowbytes;
		}
	}
}

//
// V_DrawBlock
// Draw a linear block of pixels into the view buffer.
//
void V_DrawBlock(int x, int y, int scrn, int width, int height, byte * src)
{
	byte *dest;
	
	if (!graphics_started)
		return;

#ifdef RANGECHECK
	if (x < 0 || x + width > vid.width || y < 0 || y + height > vid.height || (unsigned)scrn > 4)
	{
		I_Error("Bad V_DrawBlock");
	}
#endif

	//V_MarkRect (x, y, width, height);

	dest = screens[scrn] + y * vid.width + x;

	while (height--)
	{
		memcpy(dest, src, width);

		src += width;
		dest += vid.width;
	}
}

//
// V_GetBlock
// Gets a linear block of pixels from the view buffer.
//
void V_GetBlock(int x, int y, int scrn, int width, int height, byte * dest)
{
	byte *src;
	
	if (!graphics_started)
		return;

#ifdef RANGECHECK
	if (x < 0 || x + width > vid.width || y < 0 || y + height > vid.height || (unsigned)scrn > 4)
	{
		I_Error("Bad V_GetBlock");
	}
#endif

	src = screens[scrn] + y * vid.width + x;

	while (height--)
	{
		memcpy(dest, src, width);
		src += vid.width;
		dest += width;
	}
}

static void V_BlitScalePic(int x1, int y1, int scrn, pic_t * pic)
{	// QuickRound
	int dupx, dupy;
	int x, y;
	byte *src, *dest;
	int width, height;
	
	if (!graphics_started)
		return;

	width = SHORT(pic->width);
	height = SHORT(pic->height);
	scrn &= 0xffff;

	if (pic->mode != 0)
	{
		CONS_Printf("pic mode %d not supported in Software\n", pic->mode);
		return;
	}

	dest = screens[scrn] + max(0, y1 * vid.width) + max(0, x1);
	// y cliping to the screen
	if (y1 + QuickRound(height * vid.fdupy) >= vid.width)
		height = QuickRound((vid.width - y1) / vid.fdupy) - 1;
	// WARNING no x clipping (not needed for the moment)

	for (y = max(0, QuickRound(-y1 / vid.fdupy)); y < height; y++)
	{
		for (dupy = 0; QuickRound(vid.fdupy - dupy) > 0; dupy++)
		{
			src = pic->data + y * width;
			for (x = 0; x < width; x++)
			{
				//for (dupx = 0; QuickRound(vid.fdupx - dupx) > 0; dupx++)
				for (dupx = vid.dupx; dupx; dupx--)
					*dest++ = *src;
				src++;
			}
			dest += vid.width - QuickRound(vid.dupx * width);
		}
	}
}

//  Draw a linear pic, scaled, TOTALLY CRAP CODE!!! OPTIMISE AND ASM!!
//  CURRENTLY USED FOR StatusBarOverlay, scale pic but not starting coords
//
void V_DrawScalePic(int x1, int y1, int scrn,	// hack flag
					int lumpnum)
{
	V_BlitScalePic(x1, y1, scrn, W_CacheLumpNum(lumpnum, PU_CACHE));
}

/* V_BlitScalePicExtern() -- Extern for static */
void V_BlitScalePicExtern(int x1, int y1, int scrn, pic_t* pic)
{
	V_BlitScalePic(x1, y1, scrn, pic);
}

void V_DrawRawScreen(int x1, int y1, int lumpnum, int width, int height)
{
	V_BlitScalePic(x1, y1, 0, W_CacheRawAsPic(lumpnum, width, height, PU_CACHE));
}

//
//  Fills a box of pixels with a single color, NOTE: scaled to screen size
//
//added:05-02-98:
void V_DrawFill(int x, int y, int w, int h, int c)
{
	byte *dest;
	int u, v;
	float dupx, dupy;
	
	if (!graphics_started)
		return;
	
	dupx = vid.fdupx;
	dupy = vid.fdupy;

	dest = screens[0] + QuickRound(y * dupy) * vid.width + QuickRound(x * dupx) + scaledofs;

	w *= dupx;
	h *= dupy;

	for (v = 0; v < h; v++, dest += vid.width)
		for (u = 0; u < w; u++)
			dest[u] = c;
}

void V_DrawScreenFill(int x, int y, int w, int h, int c)
{
	byte *dest;
	int u, v;
	
	if (!graphics_started)
		return;

	dest = screens[0] + y * vid.width + x;

	for (v = 0; v < h; v++, dest += vid.width)
		for (u = 0; u < w; u++)
			dest[u] = c;
}

//
//  Fills a box of pixels using a flat texture as a pattern,
//  scaled to screen size.
//
//added:06-02-98:

/* V_DrawFlatFill() -- Draws a flat filled to the screen */
// Originally the texture itself was scaled, but I removed that behavior during
// optimization.
void V_DrawFlatFill(int x, int y, int w, int h, int flatnum)
{
#if 0
	int xx, yy;
	UInt8* FlatData;
	
	/* Get flat data */
	FlatData = W_CacheLumpNum(flatnum, PU_CACHE);
	
	/* Scale coordinates */
	
	
#else
	byte *dest;
	int u, v;
	float dupx, dupy;
	fixed_t dx, dy, xfrac, yfrac;
	byte *src;
	byte *flat;
	int size;
	int flatsize, flatshift;
	
	if (!graphics_started)
		return;

	size = W_LumpLength(flatnum);

	switch (size)
	{
		case 4194304:			// 2048x2048 lump
			flatsize = 2048;
			flatshift = 10;
			break;
		case 1048576:			// 1024x1024 lump
			flatsize = 1024;
			flatshift = 9;
			break;
		case 262144:			// 512x512 lump
			flatsize = 512;
			flatshift = 8;
			break;
		case 65536:			// 256x256 lump
			flatsize = 256;
			flatshift = 7;
			break;
		case 16384:			// 128x128 lump
			flatsize = 128;
			flatshift = 7;
			break;
		case 1024:				// 32x32 lump
			flatsize = 32;
			flatshift = 5;
			break;
		default:				// 64x64 lump
			flatsize = 64;
			flatshift = 6;
			break;
	}

	flat = W_CacheLumpNum(flatnum, PU_CACHE);

	dupx = vid.fdupx;
	dupy = vid.fdupy;

	dest = screens[0] + QuickRound(y * dupy) * vid.width + QuickRound(x * dupx) + scaledofs;

	w *= dupx;
	h *= dupy;

	dx = FixedDiv(FRACUNIT, dupx * 65535.0);
	dy = FixedDiv(FRACUNIT, dupy * 65535.0);

	yfrac = 0;
	for (v = 0; v < h; v++, dest += vid.width)
	{
		xfrac = 0;
		src = flat + ((((yfrac >> FRACBITS) - 1) & (flatsize - 1)) << flatshift);
		for (u = 0; u < w; u++)
		{
			dest[u] = src[(xfrac >> FRACBITS) & (flatsize - 1)];
			xfrac += dx;
		}
		yfrac += dy;
	}
#endif
}

/* V_DrawFadeScreen() -- Pixelate and draw dark */
void V_DrawFadeScreen(void)
{
	int x, y, i, w;
	int* buf;
	int* buf2;
	int c;
	byte *fadetable = (byte *) colormaps + 16 * 256;
	
	// Speed
	w = (vid.width >> 2);
	
	// Loop
	for (y = 0; y < vid.height; y += 8)
	{
		// Set buf
		buf = (int *)(screens[0] + vid.width * y);
		
		// Loop
		for (x = 0; x < w; x += 2)
		{
			c = fadetable[buf[x] & 0xFF];
			buf[x] = c | (c << 8) | (c << 16) | (c << 24);
			buf[x + 1] = buf[x];
		}
		
		// Inner second loop
		for (i = 1; i < 8 && (y + i) < vid.height; i++)
		{
			buf2 = (int *)(screens[0] + vid.width * (y + i));
			memcpy(buf2, buf, vid.width);
		}
	}
}





// V_Init
// olf software stuff, buffers are allocated at video mode setup
// here we set the screens[x] pointers accordingly
// WARNING :
// - called at runtime (don't init cvar here)
void V_Init(void)
{
	int i;
	byte *base;
	int screensize;

	LoadPalette("PLAYPAL");
	
	if (gamemode == heretic)
		FontBBaseLump = W_CheckNumForName("FONTB_S") + 1;
	else
		FontBBaseLump = W_CheckNumForName("FONTC01");

	//added:26-01-98:start address of NUMSCREENS * width*height vidbuffers
	base = vid.buffer;

	screensize = vid.width * vid.height * vid.bpp;

	for (i = 0; i < NUMSCREENS; i++)
		screens[i] = base + i * screensize;

	//added:26-01-98: statusbar buffer
	screens[4] = base + NUMSCREENS * screensize;

	//!debug
#ifdef DEBUG
	CONS_Printf("V_Init done:\n");
	for (i = 0; i < NUMSCREENS + 1; i++)
		CONS_Printf(" screens[%d] = %x\n", i, screens[i]);
#endif

}

//
//
//
typedef struct
{
	int px;
	int py;
} modelvertex_t;

void R_DrawSpanNoWrap(void);	//tmap.S

//
// Test 'scrunch perspective correction' tm (c) ect.
//
//added:05-04-98:
void V_DrawPerspView(byte * viewbuffer, int aiming)
{
	byte *source;
	byte *dest;
	int y;
	int x1, w;
	int offs;

	fixed_t topfrac, bottomfrac, scale, scalestep;
	fixed_t xfrac, xfracstep;

	source = viewbuffer;

	//+16 to -16 fixed
	offs = ((aiming * 20) << 16) / 100;

	topfrac = ((vid.width - 40) << 16) - (offs * 2);
	bottomfrac = ((vid.width - 40) << 16) + (offs * 2);

	scalestep = (bottomfrac - topfrac) / vid.height;
	scale = topfrac;

	for (y = 0; y < vid.height; y++)
	{
		x1 = ((vid.width << 16) - scale) >> 17;
		dest = ((byte *) vid.direct) + (vid.rowbytes * y) + x1;

		xfrac = (20 << FRACBITS) + ((!x1) & 0xFFFF);
		xfracstep = FixedDiv((vid.width << FRACBITS) - (xfrac << 1), scale);
		w = scale >> 16;
		while (w--)
		{
			*dest++ = source[xfrac >> FRACBITS];
			xfrac += xfracstep;
		}
		scale += scalestep;
		source += vid.width;
	}

}

// =============================================================================
// === 16-BIT - 16-BIT - 16-BIT - 16-BIT - 16-BIT - 16-BIT - 16-BIT - 16-BIT ===
// =============================================================================


// #############################################################################
// #############################################################################
// ####                      EXTENDED DRAWING FUNCTIONS                     ####
// #############################################################################
// #############################################################################

/*************
*** LOCALS ***
*************/

static UInt8* l_ColorMaps[NUMVEXCOLORS];					// Local colors

/*****************
*** STRUCTURES ***
*****************/

/* V_ColorEntry_t -- HSV table */
typedef union V_ColorEntry_s
{
	struct
	{
		UInt8 R;
		UInt8 G;
		UInt8 B;
	} RGB;
	
	struct
	{
		UInt8 H;
		UInt8 S;
		UInt8 V;
	} HSV;
} V_ColorEntry_t;

/****************
*** FUNCTIONS ***
****************/

/* V_ReturnColormapPtr() -- Return pointer to colormap */
const UInt8* V_ReturnColormapPtr(const VEX_ColorList_t Color)
{
	/* Check */
	if (Color < 0 || Color >= NUMVEXCOLORS)
		return NULL;
	return l_ColorMaps[Color];
}

/* V_HSVtoRGB() -- Convert HSV to RGB */
static V_ColorEntry_t V_HSVtoRGB(const V_ColorEntry_t HSV)
{
	int R, G, B, H, S, V, P, Q, T, F;
	V_ColorEntry_t Ret;
	
	/* Get inital values */
	H = HSV.HSV.H;
	S = HSV.HSV.S;
	V = HSV.HSV.V;
	
	/* Gray Color? */
	if (!S)
	{
		R = G = B = V;
	}
	
	/* Real Color */
	else
	{
		// Calculate Hue Shift
		F = ((H % 60) * 255) / 60;
		H /= 60;
		
		// Calculate channel values
		P = (V * (256 - S)) / 256;
		Q = (V * (256 - (S * F) / 256)) / 256;
		T = (V * (256 - (S * (256 - F)) / 256)) / 256;
		
		switch (H)
		{
			case 0:
				R = V;
				G = T;
				B = P;
				break;
			
			case 1:
				R = Q;
				G = V;
				B = P;
				break;
			
			case 2:
				R = P;
				G = V;
				B = T;
				break;
			
			case 3:
				R = P;
				G = Q;
				B = V;
				break;
			
			case 4:
				R = T;
				G = P;
				B = V;
				break;
			
			default:
				R = V;
				G = P;
				B = Q;
				break;
		}
	}
	
	/* Set Return */
	Ret.RGB.R = R;
	Ret.RGB.G = G;
	Ret.RGB.B = B;
	
	/* Return */
	return Ret;
}

/* V_RGBtoHSV() -- Convert RGB to HSV */
static V_ColorEntry_t V_RGBtoHSV(const V_ColorEntry_t RGB)
{
	V_ColorEntry_t Ret;
	UInt8 rMin, rMax, rDif;
	
	// Get min/max
	rMin = 255;
	rMax = 0;

	// Get RGB minimum
	if (RGB.RGB.R < rMin)
		rMin = RGB.RGB.R;
	if (RGB.RGB.G < rMin)
		rMin = RGB.RGB.G;
	if (RGB.RGB.B < rMin)
		rMin = RGB.RGB.B;

	// Get RGB maximum
	if (RGB.RGB.R > rMax)
		rMax = RGB.RGB.R;
	if (RGB.RGB.G > rMax)
		rMax = RGB.RGB.G;
	if (RGB.RGB.B > rMax)
		rMax = RGB.RGB.B;

	// Obtain value
	Ret.HSV.V = rMax;

	// Short circuit?
	if (Ret.HSV.V == 0)
	{
		Ret.HSV.H = Ret.HSV.S = 0;
		return Ret;
	}

	// Obtain difference
	rDif = rMax - rMin;

	// Obtain saturation
	Ret.HSV.S = (UInt8)(((UInt32)255 * (UInt32)rDif) / (UInt32)Ret.HSV.V);

	// Short circuit?
	if (Ret.HSV.S == 0)
	{
		Ret.HSV.H = 0;
		return Ret;
	}

	/* Obtain hue */
	if (rMax == RGB.RGB.R)
		Ret.HSV.H = 43 * (RGB.RGB.G - RGB.RGB.B) / rMax;
	else if (rMax == RGB.RGB.G)
		Ret.HSV.H = 85 + (43 * (RGB.RGB.B - RGB.RGB.R) / rMax);
	else
		Ret.HSV.H = 171 + (43 * (RGB.RGB.R - RGB.RGB.G) / rMax);
	
	return Ret;
}

/* V_BestHSVMatch() -- Best match between HSV for tables */
static size_t V_BestHSVMatch(const V_ColorEntry_t* const Table, const V_ColorEntry_t HSV)
{
	size_t i, Best;
	V_ColorEntry_t tRGB, iRGB;
	Int32 BestSqr, ThisSqr, Dr, Dg, Db;
	
	/* Check */
	if (!Table)
		return 0;
	
	/* Convert input to RGB */
	iRGB = V_HSVtoRGB(HSV);
	
	/* Loop colors */
	for (Best = 0, BestSqr = 0x7FFFFFFFUL, i = 0; i < 256; i++)
	{
		// Convert table entry to RGB
		tRGB = V_HSVtoRGB(Table[i]);
		
		// Perfect match?
		if (iRGB.RGB.R == tRGB.RGB.R && iRGB.RGB.B == tRGB.RGB.B && iRGB.RGB.G == tRGB.RGB.G)
			return i;
		
		// Distance of colors
		Dr = tRGB.RGB.R - iRGB.RGB.R;
		Dg = tRGB.RGB.G - iRGB.RGB.G;
		Db = tRGB.RGB.B - iRGB.RGB.B;
		ThisSqr = (Dr * Dr) + (Dg * Dg) + (Db * Db);
		
		// Closer?
		if (ThisSqr < BestSqr)
		{
			Best = i;
			BestSqr = ThisSqr;
		}
	}
	
	/* Fail */
	return Best;
}

/* V_InitializeColormaps() -- Initialize Spectrum colormaps */
void V_InitializeColormaps(void)
{
	size_t i, j;
	Int32 k;
	V_ColorEntry_t Base[256];
	V_ColorEntry_t First[256];
	V_ColorEntry_t Temp;
	UInt16 Additive[256];
	UInt8* PlayPal;
	
	// BaseHue -- Base for spectrum colors
	const UInt8 BaseHue[NUMVEXCOLORS] = {0, 30, 42, 85, 128, 170, 213};
	
	/* Destroy old maps */
	for (i = 0; i < NUMVEXCOLORS; i++)
		if (l_ColorMaps[i])
		{
			Z_Free(l_ColorMaps[i]);
			l_ColorMaps[i] = NULL;
		}
	
	/* Initialize the base map */
	PlayPal = W_CacheLumpName("PLAYPAL", PU_CACHE);
	
	// Load initial
	for (i = 0; i < 256; i++)
	{
		// Get RGBs
		Base[i].RGB.R = PlayPal[i * 3];
		Base[i].RGB.G = PlayPal[(i * 3) + 1];
		Base[i].RGB.B = PlayPal[(i * 3) + 2];
		
		// Get Additive Color
		Additive[i] = Base[i].RGB.R + Base[i].RGB.G + Base[i].RGB.B;
		
		// Get HSV
		First[i] = V_RGBtoHSV(Base[i]);
		
	}
	
	/* Loop through none color */
	l_ColorMaps[0] = Z_Malloc(sizeof(UInt8) * 256, PU_STATIC, (void**)&l_ColorMaps[0]);
	for (j = 0; j < 256; j++)
		l_ColorMaps[0][j] = j;
	
	/* Loop through all spectrum colors */
	for (i = VEX_MAP_RED; i <= VEX_MAP_MAGENTA; i++)
	{
		// Create color table
		l_ColorMaps[i] = Z_Malloc(sizeof(UInt8) * 256, PU_STATIC, (void**)&l_ColorMaps[i]);
		
		// Loop through colors
		for (j = 0; j < 256; j++)
		{
			Temp = First[j];
			
			// Change hue to match color
			Temp.HSV.H = BaseHue[i - VEX_MAP_RED];
			
			// Max out saturation to make it colorful
			Temp.HSV.S = 255;
			
			// Use additive for value
			if (i == VEX_MAP_ORANGE || i == VEX_MAP_YELLOW || i == VEX_MAP_CYAN)
			{
				k = (Temp.HSV.V * Temp.HSV.V) >> 8;
				Temp.HSV.V = (k < 255 ? k : 255);
			}
			else
			{
				k = (Int32)167 - (Int32)((Additive[j] / 64) * 12);
				Temp.HSV.V = k;
				Temp.HSV.V = -((Int32)Temp.HSV.V - (Int32)256);	// flip
			}
			
			// Find color
			l_ColorMaps[i][j] = V_BestHSVMatch(First, Temp);
		}
	}
	
	/* Brown */
	l_ColorMaps[VEX_MAP_BROWN] = Z_Malloc(sizeof(UInt8) * 256, PU_STATIC, (void**)&l_ColorMaps[0]);
	for (j = 0; j < 256; j++)
		l_ColorMaps[VEX_MAP_BROWN][j] = j;
	
	/* Loop through gray colors */
	for (i = VEX_MAP_BRIGHTWHITE; i <= VEX_MAP_BLACK; i++)
	{
		// Create color table
		l_ColorMaps[i] = Z_Malloc(sizeof(UInt8) * 256, PU_STATIC, (void**)&l_ColorMaps[i]);
		
		// Loop through colors
		for (j = 0; j < 256; j++)
		{
			Temp = First[j];
			
			// Remove both hue and saturation
			Temp.HSV.H = Temp.HSV.S = 0;
			
			// Increase/decrease value on some shades
			if (i == VEX_MAP_BRIGHTWHITE)
			{
				if (Temp.HSV.V >= 192)
					Temp.HSV.V = 255;
				else if (Temp.HSV.V >= 64)
					Temp.HSV.V += 32;
				else if (Temp.HSV.V >= 32)
					Temp.HSV.V += 16;
			}
			else if (i == VEX_MAP_GRAY)
				Temp.HSV.V >>= 1;
			else if (i == VEX_MAP_BLACK)
				Temp.HSV.V >>= 2;
			
			// Find color
			l_ColorMaps[i][j] = V_BestHSVMatch(First, Temp);
		}
	}
}

/* V_DrawFadeConsBackEx() -- Pixelate and add red tint */
void V_DrawFadeConsBackEx(const UInt32 Flags, const int x1, const int y1, const int x2, const int y2)
{
	int X1, Y1, X2, Y2;
	int x, y, i, w;
	int* buf;
	int* buf2;
	int c;
	UInt8* Map;
	
	/* Flags */
	// Unscaled
	if (Flags & VEX_NOSCALESTART)
	{
		X1 = x1;
		Y1 = y1;
		X2 = x2;
		Y2 = y2;
	}
	
	// Scaled
	else
	{
		X1 = (float)x1 * (float)vid.fdupx;
		Y1 = (float)y1 * (float)vid.fdupy;
		X2 = (float)x2 * (float)vid.fdupx;
		Y2 = (float)y2 * (float)vid.fdupy;
	}
	
	/* Normalize */
	// Other way
	if (X2 < X1)
	{
		x = X2;
		X2 = X1;
		X1 = x;
	}
	
	if (Y2 < Y1)
	{
		x = Y2;
		Y2 = Y1;
		Y1 = x;
	}
	
	// Squash off screen
	if (X1 < 0)
		X1 = 0;
	if (X2 >= vid.width)
		X2 = vid.width;
	if (Y1 < 0)
		Y1 = 0;
	if (Y2 >= vid.height)
		Y2 = vid.height;
	
	// Not visible?
	if (X1 == X2 || Y1 == Y2 || X1 >= vid.width || X2 < 0 || Y1 >= vid.height || Y2 < 0)
		return;
	
	/* Mapping */
	if (((Flags & VEX_COLORMAPMASK) >> VEX_COLORMAPSHIFT) < NUMVEXCOLORS)
		Map = l_ColorMaps[(Flags & VEX_COLORMAPMASK) >> VEX_COLORMAPSHIFT];
	else
		Map = l_ColorMaps[VEX_MAP_RED];
	
	/* Actual Drawing */
	// Speed
	w = (X2 >> 2);
	
	// Loop
	for (y = Y1; y < Y2; y += 8)
	{
		// Set buf
		buf = (int *)(screens[0] + vid.width * y);
		
		// Loop
		for (x = (X1 >> 2); x < w - 1; x += 2)
		{
			c = Map[buf[x] & 0xFF];
			buf[x] = c | (c << 8) | (c << 16) | (c << 24);
			buf[x + 1] = buf[x];
		}
		
		// Final bits
		for (x = x << 2; x < w; x++)
			((UInt8*)buf)[x] = c & 0xFF;
		
		// Inner second loop
		for (i = 1; i < 8 && (y + i) < Y2; i++)
		{
			buf2 = (int *)(screens[0] + vid.width * (y + i));
			memcpy(buf2, buf, X2 - X1);
		}
	}
}

/* V_DrawPatchEx() -- Extended patch drawing function */
void V_DrawPatchEx(const UInt32 Flags, const int x, const int y, const patch_t* const Patch)
{
	int X, Y, Count;
	fixed_t RowFrac, ColFrac, Col, Width, Offset, DupX, DupY;
	column_t* Column;
	UInt8* Dest;
	UInt8* DestTop;
	UInt8* Source;
	
	const UInt8* TransMap;	// TODO!
	const UInt8* ColorMap;
	Int8 Color, Screen;
	
	/* Check */
	if (!Patch)
		return;
	
	/* Init */
	X = x - Patch->leftoffset;
	Y = y - Patch->topoffset;
	RowFrac = 1 << FRACBITS;
	ColFrac = 1 << FRACBITS;
	Width = Patch->width << FRACBITS;
	DupX = DupY = 1 << FRACBITS;
	
	/* Handle Flags */
	// Transparency
	TransMap = NULL;
	
	switch ((Flags & VEX_FILLTRANSMASK) >> VEX_FILLTRANSSHIFT)
	{
		case VEX_BASETRANSMED:
		case VEX_BASETRANSHIGH:
		case VEX_BASETRANSMORE:
		case VEX_BASETRANSFIRE:
		case VEX_BASETRANSFX1:
		case VEX_BASETRANSFULL:
		default:
			break;
	}	// TODO!
	
	// Mapping
	Color = (Flags & VEX_COLORMAPMASK) >> VEX_COLORMAPSHIFT;
	
	if (Color < 0 || Color >= NUMVEXCOLORS)
		Color = 0;
	
	ColorMap = V_ReturnColormapPtr(Color);
	
	// Scaled picture
	if (!(Flags & VEX_NOSCALESCREEN))
	{
		// New scale
		DupX = vid.fxdupx;
		DupY = vid.fxdupy;
		
		// Scale all
		RowFrac = FixedDiv(RowFrac, DupY);
		ColFrac = FixedDiv(ColFrac, DupX);
		Width = FixedMul(Width, DupX);
	}
	
	// Scaled Start
	if (!(Flags & VEX_NOSCALESTART))
	{
		X = FixedMul(X << FRACBITS, DupX) >> FRACBITS;
		Y = FixedMul(Y << FRACBITS, DupY) >> FRACBITS;
	}
	
	// Alternate screen
	if (Flags & VEX_SECONDBUFFER)
		Screen = 1;
	else
		Screen = 0;
	
	/* Update dirty rectangle */
	if (!Screen)
		V_MarkRect(X, Y, Width >> FRACBITS, FixedMul(Patch->height << FRACBITS, DupY) >> FRACBITS);
	
	/* Start Drawing Patch */
	for (DestTop = screens[Screen] + (/*FixedMul(*/Y/*, DupY)*/ * vid.width) + /*FixedMul(*/X/*, DupX)*/, Col = 0;
			(Col >> FRACBITS) < Patch->width && Col < Width; Col += ColFrac, DestTop++)
	{
		// Get source column
		Column = (column_t*)((UInt8*)Patch + Patch->columnofs[Col >> FRACBITS]);
		
		// Draw column
		while (Column->topdelta != 0xFF)
		{
			// Get Drawing parms
			Source = (UInt8*)Column + 3;
			Dest = DestTop + (FixedMul(Column->topdelta, DupY) * vid.width);
			
			// Draw column
			for (Offset = 0, Count = ((FixedMul(Column->length << FRACBITS, DupY) >> FRACBITS) - 1);
					Count >= 0; Count--, Dest += vid.width, Offset += RowFrac)
				*Dest = ColorMap[Source[Offset >> FRACBITS]];
			
			// Go to next column
			Column = (column_t*)((UInt8*)Column + Column->length + 4);
		}
	}
}

/********************
*** COMPATIBILITY ***
********************/

/* V_DrawFadeConsBack() -- Pixelate and add red tint */
void V_DrawFadeConsBack(int x1, int y1, int x2, int y2)
{
	V_DrawFadeConsBackEx(VEX_COLORMAPGREEN | VEX_NOSCALESTART | VEX_NOSCALESCREEN, x1, y1, x2, y2);
}

/* V_DrawPatch() -- Draws patch unscaled */
void V_DrawPatch(const int x, const int y, const int scrn, const patch_t* const patch)
{
	UInt32 Flags = 0;
	
	/* Handle */
	if (scrn & 0xFFFF)
		Flags |= VEX_SECONDBUFFER;
	
	V_DrawPatchEx(Flags, x, y, patch);
}

/* V_DrawMappedPatch() -- Draws colormapped patch scaled */
void V_DrawMappedPatch(const int x, const int y, const int scrn, const patch_t* const patch, const byte* const colormap)
{
	UInt32 Flags = 0;
	
	/* Handle */
	if (scrn & 0xFFFF)
		Flags |= VEX_SECONDBUFFER;
	if (scrn & V_NOSCALEPATCH)
		Flags |= VEX_NOSCALESCREEN;
	if (scrn & V_NOSCALESTART)
		Flags |= VEX_NOSCALESTART;
	
	/* Color */
	if (colormap == greenmap)
		Flags |= VEX_MAP_RED << VEX_COLORMAPSHIFT;
	else if (colormap == whitemap)
		Flags |= VEX_MAP_BRIGHTWHITE << VEX_COLORMAPSHIFT;
	else if (colormap == graymap)
		Flags |= VEX_MAP_GRAY << VEX_COLORMAPSHIFT;
	else if (colormap == orangemap)
		Flags |= VEX_MAP_ORANGE << VEX_COLORMAPSHIFT;
	
	/* Now Draw */
	V_DrawPatchEx(Flags, x, y, patch);
}

/* V_DrawScaledPatch() -- Draws patch scaled */
void V_DrawScaledPatch(const int x, const int y, const int scrn, const patch_t* const patch)
{
	UInt32 Flags = 0;
	
	/* Handle */
	if (scrn & 0xFFFF)
		Flags |= VEX_SECONDBUFFER;
	if (scrn & V_NOSCALEPATCH)
		Flags |= VEX_NOSCALESCREEN;
	if (scrn & V_NOSCALESTART)
		Flags |= VEX_NOSCALESTART;
	
	/* Now Draw */
	V_DrawPatchEx(Flags, x, y, patch);
}

/* V_DrawTransPatch() -- Draw translucent patch unscaled */
void V_DrawTransPatch(const int x, const int y, const int scrn, const patch_t* const patch)
{
	UInt32 Flags = VEX_NOSCALESTART | VEX_NOSCALESCREEN;
	
	/* Handle */
	if (scrn & 0xFFFF)
		Flags |= VEX_SECONDBUFFER;
	
	V_DrawPatchEx(Flags, x, y, patch);
}

/* V_DrawTranslucentPatch() -- Draw scaled translucent patch */
void V_DrawTranslucentPatch(const int x, const int y, const int scrn, const patch_t* const patch)
{
	UInt32 Flags = 0;
	
	/* Handle */
	if (scrn & 0xFFFF)
		Flags |= VEX_SECONDBUFFER;
	
	V_DrawPatchEx(Flags, x, y, patch);
}

// #############################################################################
// #############################################################################
// ####                   ASCII AND UNICODE HANDLING                        ####
// #############################################################################
// #############################################################################

// Character Groups
// -- Font (Small, Large, etc.)
//    ++ Groups 1-256
//       .. Individual Characters (256)
// Wasteful but more speedy
UniChar_t** CharacterGroups[NUMVIDEOFONTS] = {NULL, NULL, NULL, NULL, NULL};
UniChar_t* UnknownLink[NUMVIDEOFONTS] = {NULL, NULL, NULL, NULL, NULL};

char* FontName[NUMVIDEOFONTS][2] =	/* Nice Name and ReMooD Script Name */
{
	{"Small Font", "small"},
	{"Large Font", "large"},
	{"Status Bar Font", "statusbar"},
	{"PrBoom HUD", "prboom"},
	{"OEM Font", "oem"},
	{"User Font Alpha", "usera"},
	{"User Font Beta", "userb"},
	{"User Font Gamma", "userc"},
	{"User Font Delta", "userd"}
};

char Font[NUMVIDEOFONTS][4][9] =	/* Doom, Doom (Alt), Heretic, Heretic (Alt) */
{
	{"UFNA", "STCFN", "UFNC", "FONTA"}, // VFONT_SMALL
	{"UFNB", "FONTC", "UFND", "FONTB"},	// VFONT_LARGE
	{"UFNK", "", "UFNK", ""},			// VFONT_STATUSBARSMALL
	{"UFNJ", "DIG", "UFNJ", "DIG"},		// VFONT_PRBOOMHUD
	{"UFNR", "", "UFNR", ""},			// VFONT_OEM
	{"UFNW", "", "UFNW", ""},			// VFONT_USERSPACEA
	{"UFNX", "", "UFNX", ""},			// VFONT_USERSPACEB
	{"UFNY", "", "UFNY", ""},			// VFONT_USERSPACEC
	{"UFNZ", "", "UFNZ", ""}			// VFONT_USERSPACED
};

/* V_WCharToMB() -- Convert wide character to multibyte */
static void V_WCharToMB(const wchar_t WChar, char* const MB)
{
	unsigned char* MBx;
	
	/* Check */
	if (!MB)
		return;
		
	/* Set */
	MBx = (unsigned char*)MB;
	
	/* Convert in steps */
	// Single byte
	if (WChar >= 0x0000 && WChar <= 0x007F)
	{
		MBx[0] = WChar & 0x7F;
		MBx[1] = 0;
	}
	
	// Double byte
	else if (WChar >= 0x0080 && WChar <= 0x07FF)
	{
		MBx[0] = 0xC0 | (WChar >> 6);
		MBx[1] = 0x80 | (WChar & 0x3F);
		MBx[2] = 0;
	}
	
	// Triple byte
	else if (WChar >= 0x8000 && WChar <= 0xFFFF)
	{
		MBx[0] = 0xE0 | (WChar >> 12);
		MBx[1] = 0x80 | ((WChar >> 6) & 0x3F);
		MBx[2] = 0x80 | (WChar & 0x3F);
		MBx[3] = 0;
	}
	
	// Quad-byte (Requires 32-bit wchar_t)
	else if (sizeof(wchar_t) >= 4 && (WChar >= 0x010000 && WChar <= 0x10FFFF))
	{
		MBx[0] = 0xF0 | (WChar >> 18);
		MBx[1] = 0x80 | ((WChar >> 12) & 0x3F);
		MBx[2] = 0x80 | ((WChar >> 6) & 0x3F);
		MBx[3] = 0x80 | (WChar & 0x3F);
		MBx[4] = 0;
	}
}

/* V_ExtWCharToMB() -- Convert wide character to multibyte */
void V_ExtWCharToMB(const wchar_t WChar, char* const MB)
{
	V_WCharToMB(WChar, MB);
}

/* V_AddCharacter() -- Add single character */
void V_AddCharacter(VideoFont_t Font, WadEntry_t* Entry, wchar_t Char, wchar_t Top, wchar_t Bottom)
{
	int Group = (Char >> 8) & 0xFF;
	int Local = Char & 0x00FF;
	int TG = (Top >> 8) & 0xFF;
	int TL = Top & 0x00FF;
	int BG = (Bottom >> 8) & 0xFF;
	int BL = Bottom & 0x00FF;
	
	// Check if the pointer list exists for a font
	if (!CharacterGroups[Font])
		CharacterGroups[Font] = Z_Malloc(sizeof(UniChar_t*) * 256, PU_STATIC, NULL);
		
	// Check if local group exists
	if (!CharacterGroups[Font][Group])
		CharacterGroups[Font][Group] = Z_Malloc(sizeof(UniChar_t) * 256, PU_STATIC, NULL);
	
	// Place in and/or overwrite
	CharacterGroups[Font][Group][Local].Char = Char;
	CharacterGroups[Font][Group][Local].Entry = Entry;
	CharacterGroups[Font][Group][Local].Patch = W_CachePatchNum(W_GetNumForEntry(Entry), PU_STATIC);
	
	// Multibyte
	V_WCharToMB(CharacterGroups[Font][Group][Local].Char, CharacterGroups[Font][Group][Local].MB);
	
	// Top and bottom
	CharacterGroups[Font][Group][Local].BuildTop = NULL;
	if (Top)
		if (CharacterGroups[Font][TG])
			if (CharacterGroups[Font][TG][TL].Char)
				CharacterGroups[Font][Group][Local].BuildTop = &CharacterGroups[Font][TG][TL];

	CharacterGroups[Font][Group][Local].BuildBottom = NULL;
	if (Bottom)
		if (CharacterGroups[Font][BG])
			if (CharacterGroups[Font][BG][BL].Char)
				CharacterGroups[Font][Group][Local].BuildBottom = &CharacterGroups[Font][BG][BL];
}

/* V_MapGraphicalCharacters() -- Scan WADs for characters and add them */
void V_MapGraphicalCharacters(void)
{
	int i, j, k, l;
	WadFile_t* CurWad = NULL;
	int Mode;
	char x;
	wchar_t NewChar = 0;
	wchar_t Temp = 0;
	wchar_t Temp2 = 0;
	size_t Totals[NUMVIDEOFONTS];
	int groups, ids, groupd, idd;
	
	memset(Totals, 0, sizeof(Totals));
	
	CONS_Printf("V_MapGraphicalCharacters: Mapping characters, this may take a while...\n");
	
	/* Remove all Sets */
	for (i = 0; i < NUMVIDEOFONTS; i++)
	{
		if (CharacterGroups[i])
		{
			// Free subsets
			for (j = 0; j < 256; j++)
				if (CharacterGroups[i][j])
				{
					Z_Free(CharacterGroups[i][j]);
					CharacterGroups[i][j] = NULL;
				}
			
			// Free entire set
			Z_Free(CharacterGroups[i]);
			CharacterGroups[i] = NULL;
		}
		
		memset(UnknownLink, 0, sizeof(UnknownLink));
	}
	
	/* Load WAD Fonts */
	for (i = 0; i < W_NumWadFiles(); i++)
	{
		/* Get WAD */
		CurWad = W_GetWadForNum(i);
		
		/* Check Each Entry */
		for (j = 0; j < CurWad->NumLumps; j++)
		{
			Mode = 0;
			
			/* Check for a match */
			for (k = 0; k < NUMVIDEOFONTS; k++)
				if ((gamemode != heretic && strlen(Font[k][0]) && (!strncasecmp(Font[k][0], CurWad->Index[j].Name, strlen(Font[k][0])))) ||
					(gamemode == heretic && strlen(Font[k][2]) && (!strncasecmp(Font[k][2], CurWad->Index[j].Name, strlen(Font[k][2])))))
				{
					Mode = 0;
					break;
				}
				else if ((gamemode != heretic && strlen(Font[k][1]) && (!strncasecmp(Font[k][1], CurWad->Index[j].Name, strlen(Font[k][1])))) ||
					(gamemode == heretic && strlen(Font[k][3]) && (!strncasecmp(Font[k][3], CurWad->Index[j].Name, strlen(Font[k][3])))))
				{
					Mode = 1;
					
					if (CurWad->Index[j].Name[0] == 'S')
						Mode += 1;
					else if (CurWad->Index[j].Name[0] == 'D')
						Mode += 2;
					
					break;
				}
			
			/* No Match? */
			if (k == NUMVIDEOFONTS)
				continue;
				
			/* Match found, check validity */
			// Checks every letter essentially
			if (!Mode)					// UFNxhhhh Hex
			{
				for (l = 0; l < 4; l++)
				{
					x = CurWad->Index[j].Name[4 + l];
					
					if (!((x >= 'a' && x <= 'f') || (x >= 'A' && x <= 'F') || (x >= '0' && x <= '9')))
						break;
				}
					
				if (l != 4)	// woo, yeah, this is nice
					continue;
			}
			else if	(Mode == 1)			// FONTdd
			{
				for (l = 0; l < 2; l++)
				{
					x = CurWad->Index[j].Name[5 + l];
					
					if (!(x >= '0' && x <= '9'))
						break;
				}
				
				if (l != 2)
					continue;
			}
			else if (Mode == 2)			// STCFNddd
			{
				for (l = 0; l < 3; l++)
				{
					x = CurWad->Index[j].Name[5 + l];
					
					if (l == 0)
					{
						if (!(x >= '0' && x <= '9'))
							break;
					}
					else if (l > 0)
					{
						if (!(x == 0 || (x >= '0' && x <= '9')))
							break;
					}
				}
				
				if (l != 3)
					continue;
			}
			else if (Mode == 3)			// DIGan
			{
				for (l = 0; l < 2; l++)
				{
					x = CurWad->Index[j].Name[3 + l];
					
					if (l == 0)
					{
						if (!((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z') || (x >= '0' && x <= '9')))
							break;
					}
					else
					{
						if (!(x == 0 || (x >= '0' && x <= '9')))
							break;
					}
				}
				
				if (l != 2)
					continue;
			}
			
			/* If it's made it this far, it must be valid! */
			NewChar = 0;
			switch (Mode)
			{
				case 0:
					for (l = 0; l < 4; l++)
					{
						x = CurWad->Index[j].Name[4 + l];
						
						if (x >= 'A' && x <= 'F')
							NewChar |= ((x - 'A') + 10) << (4 * (-(l - 3)));
						else if (x >= 'a' && x <= 'f')
							NewChar |= ((x - 'a') + 10) << (4 * (-(l - 3)));
						else
							NewChar |= (x - '0') << (4 * (-(l - 3)));
					}
					break;
					
				case 1:
					for (l = 0; l < 2; l++)
					{
						x = CurWad->Index[j].Name[5 + l];
						
						Temp = (x - '0');
						Temp2 = NewChar;
						
						Temp2 *= 10;
						NewChar = Temp2 + Temp;
					}
					
					NewChar += 32;
					break;
					
				case 2:
					for (l = 0; l < 3; l++)
					{
						x = CurWad->Index[j].Name[5 + l];
						
						if (x)
						{
							Temp = (x - '0');
							Temp2 = NewChar;
							
							Temp2 *= 10;
							NewChar = Temp2 + Temp;
						}
					}
					
					// Swap | and y because | takes the place of y
					if (NewChar == 121)
						NewChar = 124;
					else if (NewChar == 124)
						NewChar = 121;
					break;
					
				case 3:
					x = CurWad->Index[j].Name[3];
					
					if (x >= 'a' && x <= 'z')
						NewChar = x - 0x20;
					else if (x >= 'A' && x <= 'Z')
						NewChar = x;
					else if ((x >= '0' && x <= '9') && CurWad->Index[j].Name[4])
						NewChar = x;
					else
						NewChar = ((x - '0') * 10) + (CurWad->Index[j].Name[4] - '0');
					break;
					
				default:
					continue;
			}
			
			/* Now add it */
			if (NewChar == 0x0000 ||			// Non-printing chars, like spaces
				NewChar == '\r' ||
				NewChar == '\n' ||
				NewChar == '\t' ||
				NewChar == '\a' ||
				NewChar == '\b' ||
				NewChar == ' ' ||
				NewChar == 0x00A0 ||
				(NewChar >= 0x2000 && NewChar <= 0x200F) ||
				(NewChar >= 0x2028 && NewChar <= 0x202F) ||
				(NewChar >= 0x205F && NewChar <= 0x2063) ||
				(NewChar >= 0x206A && NewChar <= 0x206F) ||
				(NewChar >= 0xFFF9 && NewChar <= 0xFFFB))
				continue;
			
			V_AddCharacter(k, &CurWad->Index[j], NewChar, 0, 0);
			Totals[k]++;
		}
	}
	
	/* Link Unknown */
	for (i = 0; i < NUMVIDEOFONTS; i++)
	{
		if (!CharacterGroups[i])
			continue;
		
		if (!CharacterGroups[i][(int)(0xFFFD / 256)])
			continue;
			
		if (!CharacterGroups[i][(int)(0xFFFD / 256)][0xFFFD % 256].Char)
			continue;
		
		UnknownLink[i] = &CharacterGroups[i][(int)(0xFFFD / 256)][0xFFFD % 256];
	}
	
	/* Map Lowercase to Capital IF lowercase does not exist */
	{
		int x;
		
		// Scope!
		typedef struct ln_s
		{
			UInt16 d;	// Dest
			UInt16 s;	// Source
			UInt16 l;	// Len
			
			UInt16 b_top;	// Top build character
			UInt16 b_bot;	// Bottom build character
		} ln_t;
	
		static const ln_t ln[] =		// I love symlinks
		{
			/****** CHARACTER CLONES (SINGLE CHARACTER) ******/
			/*** CYRILLIC ***/
			{0x0405, 0x0053, 1},		// S
			{0x0406, 0x0049, 1},		// I
			{0x0408, 0x004A, 1},		// J
			{0x0410, 0x0041, 1},		// A
			{0x0412, 0x0042, 1},		// B
			{0x0415, 0x0045, 1},		// E
			{0x0417, 0x0033, 1},		// 3
			{0x041A, 0x004B, 1},		// K
			{0x041C, 0x004D, 1},		// M
			{0x041D, 0x0048, 1},		// H
			{0x041E, 0x004F, 1},		// O
			{0x0420, 0x0050, 1},		// P
			{0x0421, 0x0043, 1},		// C
			{0x0422, 0x0054, 1},		// T
			{0x0423, 0x0059, 1},		// Y
			{0x0425, 0x0058, 1},		// X
			/****************/
			
			/*** ROMAN NUMERALS ***/
			{0x2160, 0x0049, 1},		// I
			{0x2164, 0x0056, 1},		// V
			{0x2169, 0x0058, 1},		// X
			{0x216C, 0x004C, 1},		// L
			{0x216D, 0x0043, 1},		// C
			{0x216E, 0x0044, 1},		// D
			{0x216F, 0x004D, 1},		// M
			/**********************/
			
			/*** HALFWIDTH and FULLWIDTH FORMS ***/
			{0xFF01, 0x0021, 63},		// ! to `
			/*************************************/
			
			/*** LATIN EXTENDED B ***/
			{0x01C3, 0x0021, 1},		// !
			/************************/
			/******************************/
			
			/****** CHARACTER CLONES (CHARACTER BUILDING) ******/
			/*** LATIN-1 SUPPLEMENT ***/
			
			{0x00C0, 0x0041, 1, 0x0300},	// A with `
			{0x00C1, 0x0041, 1, 0x0301},	// A with reverse `
			{0x00C2, 0x0041, 1, 0x0302},	// A with ^
			{0x00C3, 0x0041, 1, 0x0303},	// A with ~
			{0x00C4, 0x0041, 1, 0x0308},	// A with ..
			{0x00C5, 0x0041, 1, 0x030A},	// A with o
			
			{0x00C8, 0x0045, 1, 0x0300},	// E with `
			{0x00C9, 0x0045, 1, 0x0301},	// E with reverse `
			{0x00CA, 0x0045, 1, 0x0302},	// E with ^
			{0x00CB, 0x0045, 1, 0x0308},	// E with ..
			
			{0x00CC, 0x0049, 1, 0x0300},	// I with `
			{0x00CD, 0x0049, 1, 0x0301},	// I with reverse `
			{0x00CE, 0x0049, 1, 0x0302},	// I with ^
			{0x00CF, 0x0049, 1, 0x0308},	// I with ..
			
			{0x00D2, 0x004F, 1, 0x0300},	// O with `
			{0x00D3, 0x004F, 1, 0x0301},	// O with reverse `
			{0x00D4, 0x004F, 1, 0x0302},	// O with ^
			{0x00D5, 0x004F, 1, 0x0303},	// O with ~
			{0x00D6, 0x004F, 1, 0x0308},	// O with ..
			
			{0x00D9, 0x0055, 1, 0x0300},	// U with `
			{0x00DA, 0x0055, 1, 0x0301},	// U with reverse `
			{0x00DB, 0x0055, 1, 0x0302},	// U with ^
			{0x00DC, 0x0055, 1, 0x0308},	// U with ..
			
			{0x00D1, 0x004E, 1, 0x0303},	// N with ~
			
			{0x00DD, 0x0059, 1, 0x0301},	// Y with reverse `
			/**************************/
			
			/*** LATIN EXTENDED A ***/
			{0x010C, 0x0043, 1, 0x030C},	// C with v
			{0x010E, 0x0044, 1, 0x030C},	// D with v
			{0x011A, 0x0041, 1, 0x030C},	// E with v
			{0x0147, 0x004E, 1, 0x030C},	// N with v
			{0x0158, 0x0052, 1, 0x030C},	// R with v
			{0x0160, 0x0053, 1, 0x030C},	// S with v
			{0x0164, 0x0054, 1, 0x030C},	// T with v
			{0x016E, 0x0055, 1, 0x030A},	// U with o
			{0x017D, 0x005A, 1, 0x030C},	// Z with v
			/************************/
			
			/*** CRYLLIC ***/
			{0x040C, 0x041A, 1, 0x0301},	// K with reverse `
			{0x040D, 0x0418, 1, 0x0300},	// N with `
			{0x040E, 0x0423, 1, 0x0306},	// Y with u thingy
			{0x0419, 0x0418, 1, 0x0306},	// N with u thingy
			{0x0403, 0x0413, 1, 0x0301},	// upside down reversed L with right acute
			/***************/
			/***************************************************/
			
			
			/****** LOWERCASE TO CAPITAL ******/
			{0x0061, 0x0041, 25 + 1},	// a-z
			{0x00E0, 0x00C0, 22 + 1},	// accented vowels
			{0x00F8, 0x00D8, 6 + 1},	// accented vowels
			{0x03B1, 0x0391, 25 + 1},	// Greek
			{0x0430, 0x0410, 31 + 1},	// Cryllic
			{0x0450, 0x0400, 15 + 1},	// More Cryllic
			/* TODO: Armenian */
			
			/*** LATIN EXTENDED ADDITIONAL ***/
			{0x1E01, 0x1E00, 1},
			{0x1E03, 0x1E02, 1},
			{0x1E05, 0x1E04, 1},
			{0x1E07, 0x1E06, 1},
			{0x1E09, 0x1E08, 1},
			{0x1E0B, 0x1E0A, 1},
			{0x1E0D, 0x1E0C, 1},
			{0x1E0F, 0x1E0E, 1},
			{0x1E11, 0x1E10, 1},
			{0x1E13, 0x1E12, 1},
			{0x1E15, 0x1E14, 1},
			{0x1E17, 0x1E16, 1},
			{0x1E19, 0x1E18, 1},
			{0x1E1B, 0x1E1A, 1},
			{0x1E1D, 0x1E1C, 1},
			{0x1E1F, 0x1E1E, 1},
			{0x1E21, 0x1E20, 1},
			{0x1E23, 0x1E22, 1},
			{0x1E25, 0x1E24, 1},
			{0x1E27, 0x1E26, 1},
			{0x1E29, 0x1E28, 1},
			{0x1E2B, 0x1E2A, 1},
			{0x1E2D, 0x1E2C, 1},
			{0x1E2F, 0x1E2E, 1},
			{0x1E31, 0x1E30, 1},
			{0x1E33, 0x1E32, 1},
			{0x1E35, 0x1E34, 1},
			{0x1E37, 0x1E36, 1},
			{0x1E39, 0x1E38, 1},
			{0x1E3B, 0x1E3A, 1},
			{0x1E3D, 0x1E3C, 1},
			{0x1E3F, 0x1E3E, 1},
			{0x1E41, 0x1E40, 1},
			{0x1E43, 0x1E42, 1},
			{0x1E45, 0x1E44, 1},
			{0x1E47, 0x1E46, 1},
			{0x1E49, 0x1E48, 1},
			{0x1E4B, 0x1E4A, 1},
			{0x1E4D, 0x1E4C, 1},
			{0x1E4F, 0x1E4E, 1},
			{0x1E51, 0x1E50, 1},
			{0x1E53, 0x1E52, 1},
			{0x1E55, 0x1E54, 1},
			{0x1E57, 0x1E56, 1},
			{0x1E59, 0x1E58, 1},
			{0x1E5B, 0x1E5A, 1},
			{0x1E5D, 0x1E5C, 1},
			{0x1E5F, 0x1E5E, 1},
			{0x1E61, 0x1E60, 1},
			{0x1E63, 0x1E62, 1},
			{0x1E65, 0x1E64, 1},
			{0x1E67, 0x1E66, 1},
			{0x1E69, 0x1E68, 1},
			{0x1E6B, 0x1E6A, 1},
			{0x1E6D, 0x1E6C, 1},
			{0x1E6F, 0x1E6E, 1},
			{0x1E71, 0x1E70, 1},
			{0x1E73, 0x1E72, 1},
			{0x1E75, 0x1E74, 1},
			{0x1E77, 0x1E76, 1},
			{0x1E79, 0x1E78, 1},
			{0x1E7B, 0x1E7A, 1},
			{0x1E7D, 0x1E7C, 1},
			{0x1E7F, 0x1E7E, 1},
			{0x1E81, 0x1E80, 1},
			{0x1E83, 0x1E82, 1},
			{0x1E85, 0x1E84, 1},
			{0x1E87, 0x1E86, 1},
			{0x1E89, 0x1E88, 1},
			{0x1E8B, 0x1E8A, 1},
			{0x1E8D, 0x1E8C, 1},
			{0x1E8F, 0x1E8E, 1},
			{0x1E91, 0x1E90, 1},
			{0x1E93, 0x1E92, 1},
			{0x1E95, 0x1E94, 1},
			/*********************************/
			
			/*** ROMAN NUMERALS ***/
			{0x2170, 0x2160, 16},		// i to m -to- I to M
			/**********************/
			
			/*** JAPANESE HIRAGANA ***/
			{0x3041, 0x3042, 1},
			{0x3043, 0x3044, 1},
			{0x3045, 0x3046, 1},
			{0x3047, 0x3048, 1},
			{0x3049, 0x304A, 1},
			/*************************/
			
			/*** JAPANESE KATAKANA ***/
			{0x30A1, 0x30A2, 1},
			{0x30A3, 0x30A4, 1},
			{0x30A5, 0x30A6, 1},
			{0x30A7, 0x30A8, 1},
			{0x30A9, 0x30AA, 1},
			/*************************/
			/**********************************/
			
			{0xFF41, 0xFF21, 25 + 1},	// Halfwidth and Fullwidth Forms
		
			{0x0000, 0x0000, 0}		// THE END
		};
		
		// Go through each font	
		for (i = 0; i < NUMVIDEOFONTS; i++)
		{
			// Check if the font set exists
			if (!CharacterGroups[i])
				continue;
			
			// Now copy
			for (j = 0; j < sizeof(ln) / sizeof(ln_t); j++)
				for (x = 0; x < ln[j].l; x++)
				{
					// Get Groups and IDs
					groups = ((ln[j].s + x) >> 8) & 0xFF;//(ln[j].s + x) / 256;
					ids = (ln[j].s + x) & 0xFF;//(ln[j].s + x) % 256;
					groupd = ((ln[j].d + x) >> 8) & 0xFF;//(ln[j].d + x) / 256;
					idd = (ln[j].d + x) & 0xFF;//(ln[j].d + x) % 256;
					
					// Check group and local existence of source capital
					if (!CharacterGroups[i][groups])
						continue;
					
					// Source char does not exist
					if (!CharacterGroups[i][groups][ids].Char)
						continue;
					
					// Do not replace dest if it already exists
					if (CharacterGroups[i][groupd] && CharacterGroups[i][groupd][idd].Char)
						continue;
						
					// TODO: Check for lowercase existence and use the building on that also.
					
#define MAKINGLIFESIMPLER(a,b,c) ((a) ? (a) : ((b)[i] ? ((b)[i][groups] ? ((b)[i][groups][ids].c ? (b)[i][groups][ids].c->Char : 0) : 0) : 0))
					V_AddCharacter(i, CharacterGroups[i][groups][ids].Entry, ln[j].d + x,
						MAKINGLIFESIMPLER(ln[j].b_top,CharacterGroups,BuildTop),
						MAKINGLIFESIMPLER(ln[j].b_bot,CharacterGroups,BuildBottom)
						);
#undef MAKINGLIFESIMPLER
					Totals[i]++;
				}
		}
	}
	
	CONS_Printf("V_MapGraphicalCharacters: Finished mapping characters, results as followed:\n");
	for (i = 0; i < NUMVIDEOFONTS; i++)
		if (Totals[i] == 1)
			CONS_Printf("...: %s (\"%s\") has %i character.\n", FontName[i][0], FontName[i][1], Totals[i]);
		else
			CONS_Printf("...: %s (\"%s\") has %i characters.\n", FontName[i][0], FontName[i][1], Totals[i]);
}

// =================================== ASCII ===================================

/* V_MBToWChar() -- Convert multibyte to character */
// *BSkip: Characters to skip after conversion (optional)
static wchar_t V_MBToWChar(const char* MBChar, size_t* const BSkip)
{
	size_t n;
	wchar_t Feed = 0;
	
	/* Check */
	if (!MBChar)
		return 0xFFFD;
	
	/* Get length of character */
	n = strlen(MBChar);
	
	// Double check
	if (!n)
		return 0;
	
	/* Convert in stages (I think) */
	// Single byte
	if (n == 1 || !(*MBChar & 0x80))
	{
		if (BSkip)
			*BSkip = 1;
		return *MBChar & 0x7F;
	}
	
	// Double byte
	else if (n == 2 || (*MBChar & 0xE0) == 0xC0)
	{
		Feed = (*MBChar & 0x1F);
		Feed <<= 6;
		MBChar++;
		
		Feed |= (*MBChar & 0x3F);
		
		if (BSkip)
			*BSkip = 2;
		return Feed;
	}
	
	// Triple byte
	else if (n == 3 || (*MBChar & 0xF0) == 0xE0)
	{
		Feed = (*MBChar & 0x0F);
		Feed <<= 6;
		MBChar++;
		
		Feed |= (*MBChar & 0x3F);
		Feed <<= 6;
		MBChar++;
		
		Feed |= (*MBChar & 0x3F);
		
		if (BSkip)
			*BSkip = 3;
		return Feed;
	}
		
	// Quad byte (requires 32-bit wchar_t)
	else if (sizeof(wchar_t) >= 4 && (n == 4 || (*MBChar & 0xF8) == 0xF0))
	{
		Feed = (*MBChar & 0x07);
		Feed <<= 6;
		MBChar++;
		
		Feed |= (*MBChar & 0x3F);
		Feed <<= 6;
		MBChar++;
		
		Feed |= (*MBChar & 0x3F);
		Feed <<= 6;
		MBChar++;
		
		Feed |= (*MBChar & 0x3F);
		
		if (BSkip)
			*BSkip = 4;
		return Feed;
	}
	
	// Fail
	else
	{
		if (BSkip)
			*BSkip = 1;
		return 0;
	}
}

/* V_BestWChar() -- Find best wchar_t for a character */
static const UniChar_t* V_BestWChar(const VideoFont_t Font, const wchar_t WChar)
{
	int group, id;
	
	/* Check */
	if (Font < 0 || Font >= NUMVIDEOFONTS)
		return NULL;
		
	/* Check group (if it even exists) */
	if (!CharacterGroups[Font])
		return NULL;
	
	/* Find character */
	// Get group and id
	group = (WChar >> 8) & 0xFF;
	id = WChar & 0x00FF;
	
	// Find Character
	if (!CharacterGroups[Font][group])
		if (UnknownLink[Font])
			return &CharacterGroups[Font][0xFF][0xFD];
		else
			return NULL;
	else if (!CharacterGroups[Font][group][id].Char)
		if (UnknownLink[Font])
			return &CharacterGroups[Font][0xFF][0xFD];
		else
			return NULL;
	else
		return &CharacterGroups[Font][group][id];
}

/* V_FontHeight() -- Return height of font */
int V_FontHeight(const VideoFont_t Font)
{
	/* Check */
	if (Font < 0 || Font >= NUMVIDEOFONTS)
		return 12;
	
	/* Return */
	if (UnknownLink[Font] && UnknownLink[Font]->Patch)
		return UnknownLink[Font]->Patch->height;
	else
		switch (Font)
		{
			case VFONT_SMALL:
				return 12;
				
			case VFONT_LARGE:
				return 16;
				
			case VFONT_STATUSBARSMALL:
				return 4;
				
			case VFONT_PRBOOMHUD:
				return 8;
				
			default:
				return 12;
		}
}

/* V_FontWidth() -- Width of font */
int V_FontWidth(const VideoFont_t Font)
{
	/* Check */
	if (Font < 0 || Font >= NUMVIDEOFONTS)
		return 4;
	
	/* Return */
	if (UnknownLink[Font] && UnknownLink[Font]->Patch)
		return UnknownLink[Font]->Patch->width;
	else
		return 4;
}

/* V_DrawCharacterMB() -- Draw multibyte character */
// Returns: Width of drawn character
// *BSkip : Characters to skip after drawing (optional)
int V_DrawCharacterMB(const VideoFont_t Font, const UInt32 Options, const char* const MBChar, const int x, const int y, size_t* const BSkip)
{
	const UniChar_t* D = NULL;
	wchar_t WC = 0;
	int VDrawOpt = 0;
	
	/* Check */
	if (!MBChar || !strlen(MBChar) || Font < 0 || Font >= NUMVIDEOFONTS)
		return 0;
		
	/* Any kind of space? */
	if (*MBChar == ' ')
	{
		if (BSkip)
			*BSkip = 1;
		return 4;
	}
	
	/* Find character */
	// wchar_t
	WC = V_MBToWChar(MBChar, BSkip);
	
	// Graphic
	D = V_BestWChar(Font, WC);
	
	/* Missing graphic or bad drawing parms? */
	if (!D || !D->Patch || x + D->Patch->width > vid.width)
		return 0;
		
	/* Options */
	if (Options & VFONTOPTION_NOSCALEPATCH)
		VDrawOpt |= V_NOSCALEPATCH;
	if (Options & VFONTOPTION_NOFLOATSCALE)
		VDrawOpt |= V_NOFLOATSCALE;
	if (Options & VFONTOPTION_NOSCALESTART)
		VDrawOpt |= V_NOSCALESTART;
	
	/* Draw */
	// Draw primary Glyph
	if ((Options & VFONTOPTION_COLORMASK) == VFONTOPTION_WHITE)
		V_DrawMappedPatch(x, y, VDrawOpt, D->Patch, whitemap);
	else if ((Options & VFONTOPTION_COLORMASK) == VFONTOPTION_GRAY)
		V_DrawMappedPatch(x, y, VDrawOpt, D->Patch, graymap);
	else if ((Options & VFONTOPTION_COLORMASK) == VFONTOPTION_ORANGE)
		V_DrawMappedPatch(x, y, VDrawOpt, D->Patch, orangemap);
	else
		V_DrawScaledPatch(x, y, VDrawOpt, D->Patch);
	
	// Draw top and/or bottom glyph (and ignore bskip)
	if (D->BuildTop)
		V_DrawCharacterMB(Font, Options, D->BuildTop->MB, x, y - (D->BuildTop->Patch->height), NULL);
	
	if (D->BuildBottom)
		V_DrawCharacterMB(Font, Options, D->BuildTop->MB, x, y + (D->Patch->height), NULL);
	
	/* Return graphic width */
	return D->Patch->width;
}

/* V_DrawCharacterA() -- Draw ASCII Character */
int V_DrawCharacterA(const VideoFont_t Font, const UInt32 Options, const char Char, const int x, const int y)
{
	char MB[2];
	
	/* Convert to MB */
	MB[0] = Char;
	MB[1] = 0;
	
	/* Draw and return */
	return V_DrawCharacterMB(Font, Options, MB, x, y, NULL);
}

/* V_DrawStringA() -- Draw ASCII String */
// Really multibyte
int V_DrawStringA(const VideoFont_t Font, const UInt32 Options, const char* const String, const int x, const int y)
{
	const char* c;
	int X, Y, k;
	int LineWidth = 0;
	size_t MBSkip = 0;
	int LS = 0;
	int NL = 0;
	int Ret = 0;
	
	/* Check */
	if (!String || !CharacterGroups[Font])
		return 0;
	
	/* Find position */
	X = x;
	Y = y;
	
	// Centered?
	if (Options & VFONTOPTION_CENTERED)
	{
		V_StringDimensionsA(Font, Options, String, &LineWidth, NULL);
		
		if (Options & VFONTOPTION_NOSCALESTART)
		{
			if (Options & VFONTOPTION_NOSCALEPATCH)
				X = (vid.width >> 1) - ((LineWidth * vid.dupx) >> 1);
			else
				X = (vid.width >> 1) - (LineWidth >> 1);
		}
		else
		{
			if (Options & VFONTOPTION_NOSCALEPATCH)
				X = (BASEVIDWIDTH >> 1) - ((LineWidth * vid.dupx) >> 1);
			else
				X = (BASEVIDWIDTH >> 1) - (LineWidth >> 1);
		}
	}
	
	/* Drawing loop */
	for (c = String; *c; c += MBSkip)
	{
		// Check for space
		if (*c == ' ')
		{
			LS += ((Options & VFONTOPTION_RIGHTTOLEFT) ? -4 : 4);
			MBSkip = 1;
		}
		
		// Check for newline
		else if (*c == '\n')
		{
			LS = 0;
			
			// Use unknown character height
			if (UnknownLink[Font] && UnknownLink[Font]->Patch)
				NL += UnknownLink[Font]->Patch->height;
			else
				switch (Font)
				{
					case VFONT_SMALL:
						NL += 12;
						break;
					case VFONT_LARGE:
						NL += 16;
						break;
					case VFONT_STATUSBARSMALL:
						NL += 4;
						break;
					case VFONT_PRBOOMHUD:
						NL += 8;
						break;
					default:
						NL += 12;
						break;
				}
				
			MBSkip = 1;
		}
		
		// Otherwise draw the character
		else
		{
			// Send character to screen
			k = V_DrawCharacterMB(Font, Options, c, X + LS, Y + NL, &MBSkip);
			
			// Scale?
			if (Options & VFONTOPTION_NOSCALESTART && !(Options & VFONTOPTION_NOSCALEPATCH))
				k *= vid.fdupx;
			
			// RtL?
			LS += ((Options & VFONTOPTION_RIGHTTOLEFT) ? -k : k);
			
			// MBSkip failure?
			if (!MBSkip)
				MBSkip = 1;
		}
		
		Ret += LS;
	}
	
	// Normalize (in case of RTL)
	if (Ret < 0)
		Ret = -Ret;
	
	return Ret;
}

/* V_StringDimensionsA() -- Return dimensions of string */
void V_StringDimensionsA(const VideoFont_t Font, const UInt32 Options, const char* const String, int* const Width, int* const Height)
{
	const char* c = String;
	wchar_t wc;
	const UniChar_t* D = NULL;
	int LineHeight = 0;
	int XWidth = 0;
	int XHeight = 0;
	size_t MBSkip;
	int CLine = 0;
	
	/* Check */
	if (!String || (!Width && !Height) || !CharacterGroups[Font])
	{
		if (Width)
			*Width = 0;
		
		if (Height)
			*Height = 0;
		return;
	}
	
	/* Initial height */
	if (UnknownLink[Font] && UnknownLink[Font]->Patch)
		LineHeight = UnknownLink[Font]->Patch->height;
	else
		switch (Font)
		{
			case VFONT_SMALL:
				LineHeight = 12;
				break;
			case VFONT_LARGE:
				LineHeight = 16;
				break;
			case VFONT_STATUSBARSMALL:
				LineHeight = 4;
				break;
			case VFONT_PRBOOMHUD:
				LineHeight = 8;
				break;
			default:
				LineHeight = 12;
				break;
		}
	
	// Now set
	XHeight = LineHeight;
	
	/* Parse String */	
	for (MBSkip = 1, c = String; *c; c += MBSkip)
	{
		// Space
		if (*c == ' ')
		{
			CLine += 4;
			MBSkip = 1;
		}
		
		// Newline
		else if (*c == '\n')
		{
			XHeight += LineHeight;
			CLine = 0;
			MBSkip = 1;
		}
		
		// Normal character
		else
		{
			// wchar_t
			wc = V_MBToWChar(c, &MBSkip);
	
			// Graphic
			D = V_BestWChar(Font, wc);
			
			// Good graphic
			if (!(!D || !D->Patch))
				CLine += D->Patch->width;
		}
		
		// Just in case
		if (!MBSkip)
			MBSkip = 1;
		
		if (CLine > XWidth)
			XWidth = CLine;
	}
	
	if (Width)
		*Width = XWidth;
	if (Height)
		*Height = XHeight;
}

// ================================== UNICODE ==================================

/* V_DrawCharacterW() -- Draw wide character */
int V_DrawCharacterW(const VideoFont_t Font, const UInt32 Options, const wchar_t WChar, const int x, const int y)
{
	char MB[5];
	
	/* Convert to MB */
	V_WCharToMB(WChar, MB);
	
	/* Draw and return */
	return V_DrawCharacterMB(Font, Options, MB, x, y, NULL);
}

int V_DrawStringW(const VideoFont_t Font, const UInt32 Options, const wchar_t* WString, const int x, const int y)
{
#define BUFSIZE 512
	char MB[5];
	char Buf[BUFSIZE];
	const wchar_t* w;
	char* b;
	size_t i, n;
	
	/* Check */
	if (!WString)
		return 0;
	
	/* Clear */
	memset(Buf, 0, sizeof(Buf));
	
	/* Rove convert */
	b = Buf;
	for (n = 0, w = WString; *w; w++)
	{
		// Convert this character
		V_WCharToMB(*w, MB);
		
		// Append
		for (i = 0; MB[i] && n < BUFSIZE;)
			Buf[n++] = MB[i++];
	}
		
	return V_DrawStringA(Font, Options, Buf, x, y);
#undef BUFSIZE
}

void V_StringDimensionsW(const VideoFont_t Font, const UInt32 Options, const wchar_t* const WString, int* const Width, int* const Height)
{
#define BUFSIZE 512
	char MB[5];
	char Buf[BUFSIZE];
	const wchar_t* w;
	char* b;
	size_t i, n;
	
	/* Check */
	if (!WString)
		return;
	
	/* Clear */
	memset(Buf, 0, sizeof(Buf));
	
	/* Rove convert */
	b = Buf;
	for (n = 0, w = WString; *w; w++)
	{
		// Convert this character
		V_WCharToMB(*w, MB);
		
		// Append
		for (i = 0; MB[i] && n < BUFSIZE;)
			Buf[n++] = MB[i++];
	}
		
	return V_StringDimensionsA(Font, Options, Buf, Width, Height);
#undef BUFSIZE
}

int V_StringWidthW(const VideoFont_t Font, const UInt32 Options, const wchar_t* const WString)
{
	int n = 0;
	V_StringDimensionsW(Font, Options, WString, &n, NULL);
	return n;
}

int V_StringHeightW(const VideoFont_t Font, const UInt32 Options, const wchar_t* const WString)
{
	int n = 0;
	V_StringDimensionsW(Font, Options, WString, NULL, &n);
	return n;
}

// =============================== COMPATIBILITY ===============================

int FontBBaseLump;

void V_DrawCharacter(int x, int y, int c)
{
	UInt32 Options = 0;
	
	if (c & 0x80)
		Options |= VFONTOPTION_WHITE;
	if (c & V_WHITEMAP)
		Options |= VFONTOPTION_WHITE;
	if (c & V_GRAYMAP)
		Options |= VFONTOPTION_GRAY;
	if (c & V_ORANGEMAP)
		Options |= VFONTOPTION_ORANGE;
	if (c & V_NOSCALEPATCH)
		Options |= VFONTOPTION_NOSCALEPATCH;
	if (c & V_NOFLOATSCALE)
		Options |= VFONTOPTION_NOFLOATSCALE;
	if (c & V_NOSCALESTART)
		Options |= VFONTOPTION_NOSCALESTART;
	
	V_DrawCharacterA(VFONT_SMALL, Options, c & 0x7F, x, y);
}

void V_DrawString(int x, int y, int option, char *string)
{
	UInt32 Options = 0;
	
	if (option & V_WHITEMAP)
		Options |= VFONTOPTION_WHITE;
	if (option & V_GRAYMAP)
		Options |= VFONTOPTION_GRAY;
	if (option & V_ORANGEMAP)
		Options |= VFONTOPTION_ORANGE;
	if (option & V_NOSCALEPATCH)
		Options |= VFONTOPTION_NOSCALEPATCH;
	if (option & V_NOFLOATSCALE)
		Options |= VFONTOPTION_NOFLOATSCALE;
	if (option & V_NOSCALESTART)
		Options |= VFONTOPTION_NOSCALESTART;
	
	V_DrawStringA(VFONT_SMALL, Options, string, x, y);
}

void V_DrawCenteredString(int x, int y, int option, char *string)
{
	UInt32 Options = 0;
	
	if (option & V_WHITEMAP)
		Options |= VFONTOPTION_WHITE;
	if (option & V_GRAYMAP)
		Options |= VFONTOPTION_GRAY;
	if (option & V_ORANGEMAP)
		Options |= VFONTOPTION_ORANGE;
	if (option & V_NOSCALEPATCH)
		Options |= VFONTOPTION_NOSCALEPATCH;
	if (option & V_NOFLOATSCALE)
		Options |= VFONTOPTION_NOFLOATSCALE;
	if (option & V_NOSCALESTART)
		Options |= VFONTOPTION_NOSCALESTART;
	
	V_DrawStringA(VFONT_SMALL, Options | VFONTOPTION_CENTERED, string, x, y);
}

void V_DrawTextB(char *text, int x, int y)
{
	V_DrawStringA(VFONT_LARGE, 0, text, x, y);
}

void V_DrawTextBGray(char *text, int x, int y)
{
	V_DrawStringA(VFONT_LARGE, VFONTOPTION_GRAY, text, x, y);
}

int V_StringWidth(char *string)
{
	int W;
	V_StringDimensionsA(VFONT_SMALL, 0, string, &W, NULL);
	return W;
}

int V_StringHeight(char *string)
{
	int H;
	V_StringDimensionsA(VFONT_SMALL, 0, string, NULL, &H);
	return H;
}

int V_TextBWidth(char *text)
{
	int W;
	V_StringDimensionsA(VFONT_LARGE, 0, text, &W, NULL);
	return W;
}

int V_TextBHeight(char *text)
{
	int H;
	V_StringDimensionsA(VFONT_LARGE, 0, text, NULL, &H);
	return H;
}

