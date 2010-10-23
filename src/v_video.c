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
//  V_DrawMappedPatch : like V_DrawScaledPatch, but with a colormap.
//
//
//added:05-02-98:
void V_DrawMappedPatch(int x, int y, int scrn, patch_t * patch, byte * colormap)
{
	int count;
	int col;
	column_t *column;
	byte *desttop;
	byte *dest;
	byte *source;
	int w;

	float dupx, dupy;
	int ofs;
	int colfrac, rowfrac;
	
	if (!graphics_started)
		return;

	if ((scrn & V_NOSCALEPATCH))
		dupx = dupy = 1;
	else
	{
		if (scrn & V_NOFLOATSCALE)
		{
			dupx = vid.dupx;
			dupy = vid.dupy;
		}
		else
		{
			dupx = vid.fdupx;
			dupy = vid.fdupy;
		}
	}
	y -= SHORT(patch->topoffset);
	x -= SHORT(patch->leftoffset);

	if (scrn & V_NOSCALESTART)
		desttop = screens[scrn & 0xffff] + (y * vid.width) + x;
	else
		desttop = screens[scrn & 0xffff] + (QuickRound(y * dupy) * vid.width) + QuickRound(x * dupx) + scaledofs;

	scrn &= 0xffff;

	if (!scrn)
		V_MarkRect(x, y, QuickRound(SHORT(patch->width) * dupx), QuickRound(SHORT(patch->height) * dupy));

	col = 0;
	colfrac = FixedDiv(FRACUNIT, dupx * 65535.0);
	rowfrac = FixedDiv(FRACUNIT, dupy * 65535.0);

	w = SHORT(patch->width) << FRACBITS;

	for (; col < w; col += colfrac, desttop++)
	{
		column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col >> FRACBITS]));

		while (column->topdelta != 0xff)
		{
			source = (byte *) column + 3;
			dest = desttop + QuickRound(column->topdelta * dupy) * vid.width;
			count = QuickRound(column->length * dupy);

			ofs = 0;
			while (count--)
			{
				*dest = *(colormap + source[ofs >> FRACBITS]);
				dest += vid.width;
				ofs += rowfrac;
			}

			column = (column_t *) ((byte *) column + column->length + 4);
		}
	}

}

//
// V_DrawScaledPatch
//   like V_DrawPatch, but scaled 2,3,4 times the original size and position
//   this is used for menu and title screens, with high resolutions
//
//added:05-02-98:
// default params : scale patch and scale start
void V_DrawScaledPatch(int x, int y, int scrn,	// hacked flags in it...
					   patch_t * patch)
{
	int count;
	int col;
	column_t *column;
	byte *desttop;
	byte *dest;
	byte *source;

	float dupx, dupy;
	int ofs;
	int colfrac, rowfrac;
	byte *destend;
	
	if (!graphics_started)
		return;

	if ((scrn & V_NOSCALEPATCH))
		dupx = dupy = 1;
	else
	{
		if (scrn & V_NOFLOATSCALE)
		{
			dupx = vid.dupx;
			dupy = vid.dupy;
		}
		else
		{
			dupx = vid.fdupx;
			dupy = vid.fdupy;
		}
	}

	y -= SHORT(patch->topoffset);
	x -= SHORT(patch->leftoffset);

	colfrac = FixedDiv(FRACUNIT, dupx * 65535.0);
	rowfrac = FixedDiv(FRACUNIT, dupy * 65535.0);

	desttop = screens[scrn & 0xFF];
	if (scrn & V_NOSCALESTART)
		desttop += (y * vid.width) + x;
	else
		desttop += (QuickRound(y * dupy) * vid.width) + QuickRound(x * dupx) + scaledofs;
	destend = desttop + QuickRound(SHORT(patch->width) * dupx);

	if (scrn & V_FLIPPEDPATCH)
	{
		colfrac = -colfrac;
		col = (SHORT(patch->width) << FRACBITS) + colfrac;
	}
	else
		col = 0;

	for (; desttop < destend; col += colfrac, desttop++)
	{
		column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col >> FRACBITS]));

		while (column->topdelta != 0xff)
		{
			source = (byte *) column + 3;
			dest = desttop + QuickRound(column->topdelta * dupy) * vid.width;
			count = QuickRound(column->length * dupy);

			ofs = 0;
			while (count--)
			{
				*dest = source[ofs >> FRACBITS];
				dest += vid.width;
				ofs += rowfrac;
			}

			column = (column_t *) ((byte *) column + column->length + 4);
		}
	}
}

void HWR_DrawSmallPatch(GlidePatch_t * gpatch, int x, int y, int option, byte * colormap);
// Draws a patch 2x as small. SSNTails 06-10-2003
void V_DrawSmallScaledPatch(int x, int y, int scrn, patch_t * patch, byte * colormap)
{
	int count;
	int col;
	column_t *column;
	byte *desttop;
	byte *dest;
	byte *source;

	float dupx, dupy;
	int ofs;
	int colfrac, rowfrac;
	byte *destend;
	boolean skippixels = false;
	
	if (!graphics_started)
		return;

//    if( (scrn & V_NOSCALEPATCH) )
	if (vid.dupx > 1 && vid.dupy > 1)
		dupx = dupy = 1;
	else
	{
		dupx = 1;
		dupy = 1;
		skippixels = true;
	}

	y -= SHORT(patch->topoffset);
	x -= SHORT(patch->leftoffset);

	colfrac = FixedDiv(FRACUNIT, dupx * 65535.0);
	rowfrac = FixedDiv(FRACUNIT, dupy * 65535.0);

	desttop = screens[scrn & 0xFF];

	if (skippixels)
	{
		desttop += (y * vid.width) + x;
		destend = desttop + QuickRound(SHORT(patch->width) / 2 * dupx);
	}
	else
	{
		desttop += (y * vid.width) + x;
		destend = desttop + QuickRound(SHORT(patch->width) * dupx);
	}

	if (scrn & V_FLIPPEDPATCH)
	{
		colfrac = -colfrac;
		col = (SHORT(patch->width) << FRACBITS) + colfrac;
	}
	else
		col = 0;

	if (skippixels)
	{
		int i = 0;
		for (; desttop < destend; col += colfrac, col += colfrac, desttop++)
		{
			column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col >> FRACBITS]));

			while (column->topdelta != 0xff)
			{
				source = (byte *) column + 3;
				dest = desttop + QuickRound(column->topdelta * dupy) * vid.width;
				count = QuickRound((column->length * dupy) / 2);

				ofs = 0;
				while (count--)
				{
					*dest = *(colormap + source[ofs >> FRACBITS]);
					dest += vid.width;
					ofs += rowfrac;
					ofs += rowfrac;
				}

				column = (column_t *) ((byte *) column + column->length + 4);
			}
			i++;
		}
	}
	else
	{
		for (; desttop < destend; col += colfrac, desttop++)
		{
			column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col >> FRACBITS]));

			while (column->topdelta != 0xff)
			{
				source = (byte *) column + 3;
				dest = desttop + QuickRound(column->topdelta * dupy) * vid.width;
				count = QuickRound(column->length * dupy);

				ofs = 0;
				while (count--)
				{
					*dest = *(colormap + source[ofs >> FRACBITS]);
					dest += vid.width;
					ofs += rowfrac;
				}

				column = (column_t *) ((byte *) column + column->length + 4);
			}
		}
	}
}

//added:16-02-98: now used for crosshair
//
//  This draws a patch over a background with translucency...SCALED
//  SCALE THE STARTING COORDS!!
//
void V_DrawTranslucentPatch(int x, int y, int scrn,	// hacked flag on it
							patch_t * patch)
{
	int count;
	int col;
	column_t *column;
	byte *desttop;
	byte *dest;
	byte *source;
	int w;

	float dupx, dupy;
	int ofs;
	int colfrac, rowfrac;
	
	if (!graphics_started)
		return;

	dupx = vid.dupx;
	dupy = vid.dupy;

	y -= SHORT(patch->topoffset) * dupy;
	x -= SHORT(patch->leftoffset) * dupx;

	if (!(scrn & 0xffff))
		V_MarkRect(x, y, QuickRound(SHORT(patch->width) * dupx), QuickRound(SHORT(patch->height) * dupy));

	col = 0;
	colfrac = FixedDiv(FRACUNIT, dupx * 65535.0);
	rowfrac = FixedDiv(FRACUNIT, dupy * 65535.0);

	desttop = screens[scrn & 0xffff];
	if (scrn & V_NOSCALESTART)
		desttop += (y * vid.width) + x;
	else
		desttop += (QuickRound(y * dupy) * vid.width) + QuickRound(x * dupx) + scaledofs;

	w = SHORT(patch->width) << FRACBITS;

	for (; col < w; col += colfrac, desttop++)
	{
		column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col >> FRACBITS]));

		while (column->topdelta != 0xff)
		{
			source = (byte *) column + 3;
			dest = desttop + QuickRound(column->topdelta * dupy) * vid.width;
			count = QuickRound(column->length * dupy);

			ofs = 0;
			while (count--)
			{
				*dest = *(transtables + ((source[ofs >> FRACBITS] << 8) & 0xFF00) + (*dest & 0xFF));
				dest += vid.width;
				ofs += rowfrac;
			}

			column = (column_t *) ((byte *) column + column->length + 4);
		}
	}
}

//
// V_DrawPatch
// Masks a column based masked pic to the screen. NO SCALING!!!
//
void V_DrawPatch(int x, int y, int scrn, patch_t * patch)
{

	int count;
	int col;
	column_t *column;
	byte *desttop;
	byte *dest;
	byte *source;
	int w;
	
	if (!graphics_started)
		return;
	
	y -= SHORT(patch->topoffset);
	x -= SHORT(patch->leftoffset);
#ifdef RANGECHECK
	if (x < 0 || x + SHORT(patch->width) > vid.width || y < 0 ||
		y + SHORT(patch->height) > vid.height || (unsigned)scrn > 4)
	{
		fprintf(stderr, "Patch at %d,%d exceeds LFB\n", x, y);
		// No I_Error abort - what is up with TNT.WAD?
		fprintf(stderr, "V_DrawPatch: bad patch (ignored)\n");
		return;
	}
#endif

	if (!scrn)
		V_MarkRect(x, y, SHORT(patch->width), SHORT(patch->height));

	col = 0;
	desttop = screens[scrn] + y * vid.width + x;

	w = SHORT(patch->width);

	for (; col < w; x++, col++, desttop++)
	{
		column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col]));

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			source = (byte *) column + 3;
			dest = desttop + column->topdelta * vid.width;
			count = column->length;

			while (count--)
			{
				*dest = *source++;
				dest += vid.width;
			}
			column = (column_t *) ((byte *) column + column->length + 4);
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

static void V_BlitScalePic(int x1, int y1, int scrn, pic_t * pic);
//  Draw a linear pic, scaled, TOTALLY CRAP CODE!!! OPTIMISE AND ASM!!
//  CURRENTLY USED FOR StatusBarOverlay, scale pic but not starting coords
//
void V_DrawScalePic(int x1, int y1, int scrn,	// hack flag
					int lumpnum)
{
	V_BlitScalePic(x1, y1, scrn, W_CacheLumpNum(lumpnum, PU_CACHE));
}

static void V_BlitScalePic(int x1, int y1, int scrn, pic_t * pic)
{	// QuickRound
	int dupx, dupy;
	int x, y, z;
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
void V_DrawFlatFill(int x, int y, int w, int h, int flatnum)
{
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
		src = flat + (((yfrac >> FRACBITS - 1) & (flatsize - 1)) << flatshift);
		for (u = 0; u < w; u++)
		{
			dest[u] = src[(xfrac >> FRACBITS) & (flatsize - 1)];
			xfrac += dx;
		}
		yfrac += dy;
	}
}

//
//  Fade all the screen buffer, so that the menu is more readable,
//  especially now that we use the small hufont in the menus...
//
void V_DrawFadeScreen(void)
{
	int x, y, i;
	int* buf;
	int* buf2;
	int c;
	byte *fadetable = (byte *) colormaps + 16 * 256;
	
	for (y = 0; y < vid.height; y += 8)
	{
		// Set buf
		buf = (int *)(screens[0] + vid.width * y);
		
		// Loop
		for (x = 0; x < (vid.width >> 2); x += 2)
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

// Simple translucence with one color, coords are resolution dependent
//
//added:20-03-98: console test
void V_DrawFadeConsBack(int x1, int y1, int x2, int y2)
{
	int x, y, i;
	int* buf;
	int* buf2;
	int c;
	
	for (y = y1; y < y2; y += 8)
	{
		// Set buf
		buf = (int *)(screens[0] + vid.width * y);
		
		// Loop
		for (x = (x1 >> 2); x < (x2 >> 2); x += 2)
		{
			c = greenmap[buf[x] & 0xFF];
			buf[x] = c | (c << 8) | (c << 16) | (c << 24);
			buf[x + 1] = buf[x];
		}
		
		// Inner second loop
		for (i = 1; i < 8 && (y + i) < y2; i++)
		{
			buf2 = (int *)(screens[0] + vid.width * (y + i));
			memcpy(buf2, buf, x2 - x1);
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
};

char Font[NUMVIDEOFONTS][4][9] =	/* Doom, Doom (Alt), Heretic, Heretic (Alt) */
{
	{"UFNA", "STCFN", "UFNC", "FONTA"}, // VFONT_SMALL
	{"UFNB", "FONTC", "UFND", "FONTB"},	// VFONT_LARGE
	{"UFNK", "", "UFNK", ""},			// VFONT_STATUSBARSMALL
	{"UFNJ", "DIG", "UFNJ", "DIG"},		// VFONT_PRBOOMHUD
	{"UFNR", "", "UFNR", ""},			// VFONT_OEM
	{"UFNU", "", "UFNU", ""},			// VFONT_USERSPACEA
	{"UFNV", "", "UFNV", ""}			// VFONT_USERSPACEB
};

void V_AddCharacter(VideoFont_t Font, WadEntry_t* Entry, wchar_t Char)
{
	int Group = (Char >> 8) & 0xFF;//(int)(Char / 256);
	int Local = Char & 0x00FF;//Char % 256;
	
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
}

void V_MapGraphicalCharacters(void)
{
	int i, j, k, l, m, n;
	WadFile_t* CurWad = NULL;
	int Mode;
	wchar_t GivenFont = 0;
	char x;
	wchar_t NewChar = 0;
	wchar_t Temp = 0;
	wchar_t Temp2 = 0;
	size_t Totals[NUMVIDEOFONTS];
	
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
				else if ((gamemode != heretic && strlen(Font[k][1]) && (!strncasecmp(Font[k][1], CurWad->Index[j].Name, strlen(Font[k][1]))) ||
					(gamemode == heretic && strlen(Font[k][3]) && (!strncasecmp(Font[k][3], CurWad->Index[j].Name, strlen(Font[k][3]))))))
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
				NewChar >= 0x2000 && NewChar <= 0x200F ||
				NewChar >= 0x2028 && NewChar <= 0x202F ||
				NewChar >= 0x205F && NewChar <= 0x2063 ||
				NewChar >= 0x206A && NewChar <= 0x206F ||
				NewChar >= 0xFFF9 && NewChar <= 0xFFFB)
				continue;
			
			V_AddCharacter(k, &CurWad->Index[j], NewChar);
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
		// Scope!
		typedef struct ln_s
		{
			UInt16 d;	// Dest
			UInt16 s;	// Source
			UInt16 l;	// Len
		} ln_t;
	
		ln_t ln[] =		// I love symlinks
		{
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
			
			{0xFF41, 0xFF21, 25 + 1},	// Halfwidth and Fullwidth Forms
		
			{0x0000, 0x0000, 0}		// THE END
		};
	
		int x, y, z;
		int groups, ids, groupd, idd;
		
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
					
					V_AddCharacter(i, CharacterGroups[i][groups][ids].Entry, ln[j].d + x);
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

int V_DrawCharacterA(VideoFont_t Font, UInt32 Options, char Char, int x, int y)
{
	return V_DrawCharacterW(Font, Options, Char, x, y);
}

int V_DrawStringA(VideoFont_t Font, UInt32 Options, char* String, int x, int y)
{
	wchar_t* WStr = NULL;
	size_t WSz = 0;
	int Ret = 0;
	
	if (!String)
		return 0;
	
	if (UNICODE_ASCIIToUnicode(String, strlen(String), &WStr, &WSz))
	{
		Ret = V_DrawStringW(Font, Options, WStr, x, y);
		Z_Free(WStr);
		return Ret;
	}
	else
		return 0;
}

void V_StringDimensionsA(VideoFont_t Font, UInt32 Options, char* String, int* Width, int* Height)
{
	wchar_t* WStr = NULL;
	size_t WSz = 0;
	int Ret = 0;
	
	if (!String)
		return;
		
	if (!Width && !Height)
		return;
	
	if (UNICODE_ASCIIToUnicode(String, strlen(String), &WStr, &WSz))
	{
		V_StringDimensionsW(Font, Options, WStr, Width, Height);
		Z_Free(WStr);
		return;
	}
	else
		return;
}

// ================================== UNICODE ==================================

int V_DrawCharacterW(VideoFont_t Font, UInt32 Options, wchar_t WChar, int x, int y)
{
	int group, id;
	UniChar_t* D = NULL;
	int VDrawOpt = 0;
	
	/* Check for valid character */
	if (!CharacterGroups[Font])
		return 0;
	
	// Any kind of space?
	if (WChar == ' ')
		return 4;
		
	// Get Group
	group = (WChar >> 8) & 0xFF;//WChar / 256;
	id = WChar & 0x00FF;//WChar % 256;
	
	// Find Character
	if (!CharacterGroups[Font][group])
	{
		if (UnknownLink[Font])
			D = UnknownLink[Font];
		else
			return 0;
	}
	else if (!CharacterGroups[Font][group][id].Char)
	{
		if (UnknownLink[Font])
			D = UnknownLink[Font];
		else
			return 0;
	}
	
	D = &CharacterGroups[Font][group][id];
	
	// Bad Patch
	if (!D->Patch)
		return 0;
		
	/* Check bounds */
	if (x + D->Patch->width > vid.width)
		return 0;
	
	/* Options */
	if (Options & VFONTOPTION_NOSCALEPATCH)
		VDrawOpt |= V_NOSCALEPATCH;
	if (Options & VFONTOPTION_NOFLOATSCALE)
		VDrawOpt |= V_NOFLOATSCALE;
	if (Options & VFONTOPTION_NOSCALESTART)
		VDrawOpt |= V_NOSCALESTART;
		
	/* Draw */
	if (Options & VFONTOPTION_WHITE)
		V_DrawMappedPatch(x, y, VDrawOpt, D->Patch, whitemap);
	else if (Options & VFONTOPTION_GRAY)
		V_DrawMappedPatch(x, y, VDrawOpt, D->Patch, graymap);
	else if (Options & VFONTOPTION_ORANGE)
		V_DrawMappedPatch(x, y, VDrawOpt, D->Patch, orangemap);
	else
		V_DrawScaledPatch(x, y, VDrawOpt, D->Patch);
	
	/* Return width */
	return D->Patch->width;
}

int V_DrawStringW(VideoFont_t Font, UInt32 Options, wchar_t* WString, int x, int y)
{
	wchar_t* c = WString;
	int Ret = 0;
	int LS = 0;
	int NL = 0;
	int LineWidth = 0;
	int realx, realy;
	int k;
	
	if (!WString)
		return 0;
	
	if (!CharacterGroups[Font])
		return 0;
		
	// Real Position
	realx = x;
	realy = y;
	
	if (Options & VFONTOPTION_CENTERED)
	{
		V_StringDimensionsW(Font, Options, WString, &LineWidth, NULL);
		
		if (Options & VFONTOPTION_NOSCALESTART)
		{
			if (Options & VFONTOPTION_NOSCALEPATCH)
				realx = (vid.width >> 1) - ((LineWidth * vid.dupx) >> 1);
			else
				realx = (vid.width >> 1) - (LineWidth >> 1);
		}
		else
		{
			if (Options & VFONTOPTION_NOSCALEPATCH)
				realx = (BASEVIDWIDTH >> 1) - ((LineWidth * vid.dupx) >> 1);
			else
				realx = (BASEVIDWIDTH >> 1) - (LineWidth >> 1);
		}
	}
	
	while (*c)
	{
		// Check for space
		if (*c == ' ')
		{
			if (Options & VFONTOPTION_RIGHTTOLEFT)
				LS -= 4;
			else
				LS += 4;
		}
		else if (*c == '\n')
		{
			LS = 0;
			
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
		}
		
		// Draw Character
		else
		{
			k = V_DrawCharacterW(Font, Options, *c, realx + LS, realy + NL);
			
			if (Options & VFONTOPTION_NOSCALESTART && !(Options & VFONTOPTION_NOSCALEPATCH))
				k *= vid.fdupx;
			
			if (Options & VFONTOPTION_RIGHTTOLEFT)
				LS -= k;
			else
				LS += k;
		}
		
		Ret += LS;
		
		// Next character
		c++;
	}
	
	if (Ret < 0)
		Ret = -Ret;
	
	return Ret;
}

void V_StringDimensionsW(VideoFont_t Font, UInt32 Options, wchar_t* WString, int* Width, int* Height)
{
	wchar_t* c = WString;
	int XWidth = 0;
	int XHeight = 0;
	int CLine = 0;
	int group, id;
	UniChar_t* D = NULL;
		
	if (!Width && !Height)
		return;
		
	if (!CharacterGroups[Font])
	{
		if (Width) *Width = 0;
		if (Height) *Width = 0;
		return;
	}
	
	// Initial height
	if (UnknownLink[Font] && UnknownLink[Font]->Patch)
		XHeight += UnknownLink[Font]->Patch->height;
	else
		switch (Font)
		{
			case VFONT_SMALL:
				XHeight += 12;
				break;
			case VFONT_LARGE:
				XHeight += 16;
				break;
			case VFONT_STATUSBARSMALL:
				XHeight += 4;
				break;
			case VFONT_PRBOOMHUD:
				XHeight += 8;
				break;
			default:
				XHeight += 12;
				break;
		}
	
	// From String
	while (*c)
	{
		if (*c == ' ')
			CLine += 4;
		else if (*c == '\n')
		{
			CLine = 0;
			
			if (UnknownLink[Font] && UnknownLink[Font]->Patch)
				XHeight += UnknownLink[Font]->Patch->height;
			else
				switch (Font)
				{
					case VFONT_SMALL:
						XHeight += 12;
						break;
					case VFONT_LARGE:
						XHeight += 16;
						break;
					case VFONT_STATUSBARSMALL:
						XHeight += 4;
						break;
					case VFONT_PRBOOMHUD:
						XHeight += 8;
						break;
					default:
						XHeight += 12;
						break;
				}
		}
		else
		{
			group = (*c >> 8) & 0xFF;//*c / 256;
			id = *c & 0x00FF;//*c % 256;

			// Find Character
			if (!CharacterGroups[Font][group])
			{
				if (UnknownLink[Font])
					D = UnknownLink[Font];
				else
					goto bad;
			}
			else if (!CharacterGroups[Font][group][id].Char)
			{
				if (UnknownLink[Font])
					D = UnknownLink[Font];
				else
					goto bad;
			}

			D = &CharacterGroups[Font][group][id];

			// Bad Patch
			if (!D->Patch)
				goto bad;
			
			CLine += D->Patch->width;
		}

bad:
		
		if (CLine > XWidth)
			XWidth = CLine;
		
		c++;
	}
	
	if (Width)
		*Width = XWidth;
	if (Height)
		*Height = XHeight;
}

int V_StringWidthW(VideoFont_t Font, UInt32 Options, wchar_t* WString)
{
	int n = 0;
	V_StringDimensionsW(Font, Options, WString, &n, NULL);
	return n;
}

int V_StringHeightW(VideoFont_t Font, UInt32 Options, wchar_t* WString)
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

