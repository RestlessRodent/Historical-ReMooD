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
// DESCRIPTION:
//      span / column drawer functions, for 8bpp and 16bpp
//
//      All drawing to the view buffer is accomplished in this file.
//      The other refresh files only know about ccordinates,
//      not the architecture of the frame buffer.
//      The frame buffer is a linear one, and we need only the base address.

#include "doomdef.h"
#include "doomstat.h"
#include "r_local.h"
#include "st_stuff.h"			//added:24-01-98:need ST_HEIGHT
#include "i_video.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"
#include "console.h"			//Som: Until I get buffering finished

// ==========================================================================
//                     COMMON DATA FOR 8bpp AND 16bpp
// ==========================================================================

uint8_t* viewimage;
int viewwidth;
int scaledviewwidth;
int viewheight;
int viewwindowx;
int viewwindowy;

// pointer to the start of each line of the screen,
uint8_t** activeylookup;		// Active ylookup table (for 4 way splits)
uint8_t** ylookup;
uint8_t** ylookup1;				// for view1 (splitscreen)
uint8_t** ylookup2;				// for view2 (splitscreen)
uint8_t** ylookup4[MAXSPLITSCREENPLAYERS];	// for 4 way split screen

// x uint8_t offset for columns inside the viewwindow
// so the first column starts at (SCRWIDTH-VIEWWIDTH)/2
int* columnofs;

// =========================================================================
//                      COLUMN DRAWING CODE STUFF
// =========================================================================

lighttable_t* dc_colormap;
int dc_x;
int dc_yl;
int dc_yh;

//Hurdler: 04/06/2000: asm code still use it
int dc_yw;						//added:24-02-98: WATER!
lighttable_t* dc_wcolormap;		//added:24-02-98:WATER!

fixed_t dc_iscale;
fixed_t dc_texturemid;
int dc_drawymove;

uint8_t* dc_source;

// -----------------------
// translucency stuff here
// -----------------------
#define NUMTRANSTABLES  NUMVEXTRANSPARENCIES	// how many translucency tables are used

uint8_t* transtables = NULL;			// translucency tables

// R_DrawTransColumn uses this
uint8_t* dc_transmap;			// one of the translucency tables

// ----------------------
// translation stuff here
// ----------------------

uint8_t* translationtables = NULL;

// R_DrawTranslatedColumn uses this
uint8_t* dc_translation;

struct r_lightlist_s* dc_lightlist = NULL;
int dc_numlights = 0;
int dc_maxlights;

int dc_texheight;

// =========================================================================
//                      SPAN DRAWING CODE STUFF
// =========================================================================

int ds_y;
int ds_x1;
int ds_x2;

lighttable_t* ds_colormap;

fixed_t ds_xfrac;
fixed_t ds_yfrac;
fixed_t ds_xstep;
fixed_t ds_ystep;

uint8_t* ds_source;				// start of a 64*64 tile image
uint8_t* ds_transmap;			// one of the translucency tables

// Variable flat sizes SSNTails 06-10-2003
int flatsize;
int flatmask;
int flatsubtract;

// ==========================================================================
//                        OLD DOOM FUZZY EFFECT
// ==========================================================================

//
// Spectre/Invisibility.
//
#define FUZZTABLE     50
#define FUZZOFF       (1)

static int fuzzoffset[FUZZTABLE] =
{
	FUZZOFF, -FUZZOFF, FUZZOFF, -FUZZOFF, FUZZOFF, FUZZOFF, -FUZZOFF,
	FUZZOFF, FUZZOFF, -FUZZOFF, FUZZOFF, FUZZOFF, FUZZOFF, -FUZZOFF,
	FUZZOFF, FUZZOFF, FUZZOFF, -FUZZOFF, -FUZZOFF, -FUZZOFF, -FUZZOFF,
	FUZZOFF, -FUZZOFF, -FUZZOFF, FUZZOFF, FUZZOFF, FUZZOFF, FUZZOFF, -FUZZOFF,
	FUZZOFF, -FUZZOFF, FUZZOFF, FUZZOFF, -FUZZOFF, -FUZZOFF, FUZZOFF,
	FUZZOFF, -FUZZOFF, -FUZZOFF, -FUZZOFF, -FUZZOFF, FUZZOFF, FUZZOFF,
	FUZZOFF, FUZZOFF, -FUZZOFF, FUZZOFF, FUZZOFF, -FUZZOFF, FUZZOFF
};

static int fuzzpos = 0;			// move through the fuzz table

//  fuzzoffsets are dependend of vid width, for optimising purpose
//  this is called by SCR_Recalc() whenever the screen size changes
//
void R_RecalcFuzzOffsets(void)
{
	int i;
	
	for (i = 0; i < FUZZTABLE; i++)
	{
		fuzzoffset[i] = (fuzzoffset[i] < 0) ? -vid.width : vid.width;
	}
}

// =========================================================================
//                   TRANSLATION COLORMAP CODE
// =========================================================================

char* Color_Names[MAXSKINCOLORS] =
{
	"Green",
	"Gray",
	"Brown",
	"Red",
	"light gray",
	"light brown",
	"light red",
	"light blue",
	"Blue",
	"Yellow",
	"Beige",
	"White",					// NEW COLORS
	"Orange",
	"Tan",
	"Black",
	"Pink",
};

CV_PossibleValue_t Color_cons_t[] = { {0, NULL}, {1, NULL}, {2, NULL}, {3, NULL},
	{4, NULL}, {5, NULL}, {6, NULL}, {7, NULL},
	{8, NULL}, {9, NULL}, {10, NULL}, {11, NULL}, {12, NULL}, {13, NULL}, {14, NULL}, {15, NULL}, {0, NULL}
};

/* RS_TransTableOCCB() -- Loads new translation tables */
static bool_t RS_TransTableOCCB(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD)
{
	size_t i, j, x, y;
	const WL_WADEntry_t* Entry;
	uint8_t* p;
	uint8_t* Buffer;
	bool_t Flip;
	static struct
	{
		char Name[9];
		bool_t Flip;
	} TransLumps[NUMVEXTRANSPARENCIES] =
	{
		{"RMD_TRFF", true},
		{"RMD_TR10", false},
		{"RMD_TR20", false},
		{"RMD_TR30", false},
		{"RMD_TR40", false},
		{"RMD_TR50", false},
		{"RMD_TR40", true},
		{"RMD_TR30", true},
		{"RMD_TR20", true},
		{"RMD_TR10", true},
		{"RMD_TRFF", false},
		{"RMD_TRFR", false},
		{"TRANSFX1", false},
	};
	
	/* Load transparency tables */
	// Free old
	if (transtables)
		Z_Free(transtables);
	
	// Allocate area
	transtables = Z_Malloc(NUMTRANSTABLES * 0x10000, PU_STATIC, &transtables);
	
	// Go through loop, loading each lump
	for (i = 0; i < NUMVEXTRANSPARENCIES; i++)
	{
		// Load lump data
		Entry = WL_FindEntry(NULL, 0, TransLumps[i].Name);
		p = transtables + (0x10000 * i);
		
		// Found?
		if (Entry)
		{
			// Load buffer with data (faster than WL_ReadData over 1 million times)
			Buffer = Z_Malloc(256 * 256, PU_STATIC, NULL);
			WL_ReadData(Entry, 0, Buffer, 256 * 256);
			
			// Do we flip?
			Flip = TransLumps[i].Flip;
			
			// Load data into table portion
			for (x = 0; x < 256; x++)
				for (y = 0; y < 256; y++)
				{
					// Determine based on flip
					if (Flip)
						j = (x * 256) + y;
					else
						j = (y * 256) + x;
					
					// Read from lump to here
					*p = Buffer[j];
					p++;
				}
			
			// Free Buffer
			Z_Free(Buffer);
		}
		
		// Not found? Ouch, just keep it normal
		else
		{
			// Not too sure if this actually works!
			for (x = 0; x < 256; x++)
				for (y = 0; y < 256; y++)
				{
					*p = x;
					p++;
				}
		}
	}
	
	/* Load player remappings */
	// Free old stuff
	if (translationtables)
		Z_Free(translationtables);
	
	// Allocate table area
	translationtables = Z_Malloc(256 * (MAXSKINCOLORS - 1), PU_STATIC, NULL);
	
	// Load lump
	Entry = WL_FindEntry(NULL, 0, "RMD_PMAP");
	
	// Found?
	if (Entry)
	{
		// Read data from lump
		WL_ReadData(Entry, 0, translationtables, 256 * (MAXSKINCOLORS - 1));
	}
	
	// Not found, do base translations (ouch)
	else
	{
		// Just don't translate at all!
		for (i = 0; i < 256 * (MAXSKINCOLORS - 1); i++)
			translationtables[i] = i & 0xFF;
	}
	
	/* Success */
	return true;
}

/* R_InitTranslationTables() -- Loads translations for green and transparency */
void R_InitTranslationTables(void)
{
	/* Register OCCB */
	if (!WL_RegisterOCCB(RS_TransTableOCCB, 100))
		I_Error("R_InitTranslationTables: Failed to register OCCB.");
}

// ==========================================================================
//               COMMON DRAWER FOR 8 AND 16 BIT COLOR MODES
// ==========================================================================

// in a perfect world, all routines would be compatible for either mode,
// and optimised enough
//
// in reality, the few routines that can work for either mode, are
// put here

// R_InitViewBuffer
// Creates lookup tables for getting the framebuffer address
//  of a pixel to draw.
//
void R_InitViewBuffer(int width, int height)
{
	int i;
	int j;
	int bytesperpixel = vid.bpp;
	
	if (bytesperpixel < 1 || bytesperpixel > 4)
		I_Error("R_InitViewBuffer : wrong bytesperpixel value %d\n", bytesperpixel);
		
	// Handle resize,
	//  e.g. smaller view windows
	//  with border and/or status bar.
	if (!cv_splitscreen.value)
		viewwindowx = (vid.width - width) >> 1;
	else
		viewwindowx = 0;
		
	// Column offset for those columns of the view window, but
	// relative to the entire screen
	for (i = 0; i < width; i++)
		columnofs[i] = (viewwindowx + i) * bytesperpixel;
		
	// Same with base row offset.
	if (width == vid.width)
		viewwindowy = 0;
	else
	{
		if (!cv_splitscreen.value)
			viewwindowy = (vid.height - stbarheight - height) >> 1;
		else
			viewwindowy = 0;
	}
	
	// Precalculate all row offsets.
	for (i = 0; i < height; i++)
	{
		ylookup[i] = ylookup1[i] = vid.buffer + (i + viewwindowy) * vid.width * bytesperpixel;
		ylookup2[i] = vid.buffer + (i + (vid.height >> 1)) * vid.width * bytesperpixel;	// for splitscreen
		
		// 4 way split screen
		ylookup4[0][i] = vid.buffer + (i * vid.width * bytesperpixel);
		ylookup4[1][i] = vid.buffer + ((vid.width / 2) * bytesperpixel) + (i * vid.width * bytesperpixel);
		ylookup4[2][i] = vid.buffer + (vid.width * (vid.height / 2) * bytesperpixel) + (i * vid.width * bytesperpixel);
		ylookup4[3][i] = vid.buffer + (vid.width * (vid.height / 2) * bytesperpixel) + ((vid.width / 2) * bytesperpixel) + (i * vid.width * bytesperpixel);
	}
}

//
// Store the lumpnumber of the viewborder patches.
//
int viewborderlump[8];
void R_InitViewBorder(void)
{
#if 0
	viewborderlump[BRDR_T] = W_GetNumForName("brdr_t");
	viewborderlump[BRDR_B] = W_GetNumForName("brdr_b");
	viewborderlump[BRDR_L] = W_GetNumForName("brdr_l");
	viewborderlump[BRDR_R] = W_GetNumForName("brdr_r");
	viewborderlump[BRDR_TL] = W_GetNumForName("brdr_tl");
	viewborderlump[BRDR_BL] = W_GetNumForName("brdr_bl");
	viewborderlump[BRDR_TR] = W_GetNumForName("brdr_tr");
	viewborderlump[BRDR_BR] = W_GetNumForName("brdr_br");
#endif
}

//
// R_FillBackScreen
// Fills the back screen with a pattern for variable screen sizes
// Also draws a beveled edge.
//
void R_FillBackScreen(void)
{
	uint8_t* src;
	uint8_t* dest;
	int x;
	int y;
	patch_t* patch;
	int step, boff;
	
	if (cv_splitscreen.value)
		return;
		
	//added:08-01-98:draw pattern around the status bar too (when hires),
	//                so return only when in full-screen without status bar.
	if ((scaledviewwidth == vid.width) && (viewheight == vid.height))
		return;
		
	src = scr_borderpatch;
	dest = screens[1];
	
	if (!src)
		return;
	
	for (y = 0; y < vid.height; y++)
	{
		for (x = 0; x < vid.width / 64; x++)
		{
			memcpy(dest, src + ((y & 63) << 6), 64);
			dest += 64;
		}
		
		if (vid.width & 63)
		{
			memcpy(dest, src + ((y & 63) << 6), vid.width & 63);
			dest += (vid.width & 63);
		}
	}
	
	//added:08-01-98:dont draw the borders when viewwidth is full vid.width.
	if (scaledviewwidth == vid.width)
		return;
		
	step = 8;
	boff = 8;

#if 0
	patch = W_CacheLumpNum(viewborderlump[BRDR_T], PU_CACHE);
	for (x = 0; x < scaledviewwidth; x += step)
		V_DrawPatch(viewwindowx + x, viewwindowy - boff, 1, patch);
	patch = W_CacheLumpNum(viewborderlump[BRDR_B], PU_CACHE);
	for (x = 0; x < scaledviewwidth; x += step)
		V_DrawPatch(viewwindowx + x, viewwindowy + viewheight, 1, patch);
	patch = W_CacheLumpNum(viewborderlump[BRDR_L], PU_CACHE);
	for (y = 0; y < viewheight; y += step)
		V_DrawPatch(viewwindowx - boff, viewwindowy + y, 1, patch);
	patch = W_CacheLumpNum(viewborderlump[BRDR_R], PU_CACHE);
	for (y = 0; y < viewheight; y += step)
		V_DrawPatch(viewwindowx + scaledviewwidth, viewwindowy + y, 1, patch);
		
	// Draw beveled corners.
	V_DrawPatch(viewwindowx - boff, viewwindowy - boff, 1, W_CacheLumpNum(viewborderlump[BRDR_TL], PU_CACHE));
	
	V_DrawPatch(viewwindowx + scaledviewwidth, viewwindowy - boff, 1, W_CacheLumpNum(viewborderlump[BRDR_TR], PU_CACHE));
	
	V_DrawPatch(viewwindowx - boff, viewwindowy + viewheight, 1, W_CacheLumpNum(viewborderlump[BRDR_BL], PU_CACHE));
	
	V_DrawPatch(viewwindowx + scaledviewwidth, viewwindowy + viewheight, 1, W_CacheLumpNum(viewborderlump[BRDR_BR], PU_CACHE));
#endif
}

//
// Copy a screen buffer.
//
void R_VideoErase(unsigned ofs, int count)
{
	// LFB copy.
	// This might not be a good idea if memcpy
	//  is not optiomal, e.g. uint8_t by uint8_t on
	//  a 32bit CPU, as GNU GCC/Linux libc did
	//  at one point.
	memcpy(screens[0] + ofs, screens[1] + ofs, count);
}

//
// R_DrawViewBorder
// Draws the border around the view
//  for different size windows?
//
void R_DrawViewBorder(void)
{
	int top;
	int side;
	int ofs;
	
#ifdef DEBUG
	fprintf(stderr, "RDVB: vidwidth %d vidheight %d scaledviewwidth %d viewheight %d\n", vid.width, vid.height, scaledviewwidth, viewheight);
#endif
	
	//added:08-01-98: draw the backtile pattern around the status bar too
	//                 (when statusbar width is shorter than vid.width)
	/*
	   if( (vid.width>ST_WIDTH) && (vid.height!=viewheight) )
	   {
	   ofs  = (vid.height-stbarheight)*vid.width;
	   side = (vid.width-ST_WIDTH)>>1;
	   R_VideoErase(ofs,side);
	
	   ofs += (vid.width-side);
	   for (i=1;i<stbarheight;i++)
	   {
	   R_VideoErase(ofs,side<<1);  //wraparound right to left border
	   ofs += vid.width;
	   }
	   R_VideoErase(ofs,side);
	   } */
	
	if (scaledviewwidth == vid.width)
		return;
		
	top = (vid.height - stbarheight - viewheight) >> 1;
	side = (vid.width - scaledviewwidth) >> 1;
	
	// copy top and one line of left side
	R_VideoErase(0, top * vid.width + side);
	
	// copy one line of right side and bottom
	ofs = (viewheight + top) * vid.width - side;
	R_VideoErase(ofs, top * vid.width + side);
	
	// copy sides using wraparound
	ofs = top * vid.width + vid.width - side;
	side <<= 1;
	
	//added:05-02-98:simpler using our new VID_Blit routine
	VID_BlitLinearScreen(screens[1] + ofs, screens[0] + ofs, side, viewheight - 1, vid.width, vid.width);
}

// ==========================================================================
//                   INCLUDE 8bpp DRAWING CODE HERE
// ==========================================================================

// ==========================================================================
// COLUMNS
// ==========================================================================

//  A column is a vertical slice/span of a wall texture that uses
//  a has a constant z depth from top to bottom.
//
#define USEBOOMFUNC

#ifndef USEBOOMFUNC
void R_DrawColumn_8(void)
{
	register int count;
	register uint8_t* dest;
	register fixed_t frac;
	register fixed_t fracstep;
	
	count = dc_yh - dc_yl + 1;
	
	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;
		
#ifdef RANGECHECK
	if ((unsigned)dc_x >= vid.width || dc_yl < 0 || dc_yh >= vid.height)
		I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif
		
	// Framebuffer destination address.
	// Use ylookup LUT to avoid multiply with ScreenWidth.
	// Use columnofs LUT for subwindows?
	dest = activeylookup[dc_yl] + columnofs[dc_x];
	
	// Determine scaling,
	//  which is the only mapping to be done.
	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl - centery) * fracstep;
	
	// Inner loop that does the actual texture mapping,
	//  e.g. a DDA-lile scaling.
	// This is as fast as it gets.
	do
	{
		// Re-map color indices from wall texture column
		//  using a lighting/special effects LUT.
		*dest = dc_colormap[dc_source[(frac >> FRACBITS) & 127]];
		
		dest += dc_drawymove;
		frac += fracstep;
		
	}
	while (--count);
}
#else							//USEBOOMFUNC
// SoM: Experiment to make software go faster. Taken from the Boom source
void R_DrawColumn_8(void)
{
	int count, ccount;
	register uint8_t* dest;
	register fixed_t frac;
	fixed_t fracstep;

	count = dc_yh - dc_yl + 1;

	if (count <= 0)				// Zero length, column does not exceed a pixel.
		return;

	ccount = count;

#ifdef RANGECHECK
	if ((unsigned)dc_x >= vid.width || dc_yl < 0 || dc_yh >= vid.height)
		I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

	if ((unsigned)dc_x >= vid.width || dc_yl < 0 || dc_yl > viewheight || dc_yh >= vid.height)
		return;

	// Framebuffer destination address.
	// Use ylookup LUT to avoid multiply with ScreenWidth.
	// Use columnofs LUT for subwindows?

	dest = activeylookup[dc_yl] + columnofs[dc_x];

	// Determine scaling, which is the only mapping to be done.

	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl - centery) * fracstep;

	// Inner loop that does the actual texture mapping,
	//  e.g. a DDA-lile scaling.
	// This is as fast as it gets.

	{
		register const uint8_t* source = dc_source;
		register const lighttable_t* colormap = dc_colormap;
		register int heightmask = dc_texheight - 1;

		if (dc_texheight & heightmask)
		{
			heightmask++;
			heightmask <<= FRACBITS;

			if (frac < 0)
				while ((frac += heightmask) < 0);
			else
				while (frac >= heightmask)
					frac -= heightmask;

			do
			{
				// Re-map color indices from wall texture column
				//  using a lighting/special effects LUT.
				// heightmask is the Tutti-Frutti fix -- killough

				*dest = colormap[source[frac >> FRACBITS]];
				dest += dc_drawymove;
				if ((frac += fracstep) >= heightmask)
					frac -= heightmask;
			}
			while (--count);
		}
		else
		{
			while ((count -= 2) >= 0)	// texture height is a power of 2 -- killough
			{
				*dest = colormap[source[(frac >> FRACBITS) & heightmask]];
				dest += dc_drawymove;
				frac += fracstep;
				*dest = colormap[source[(frac >> FRACBITS) & heightmask]];
				dest += dc_drawymove;
				frac += fracstep;
			}
			if (count & 1)
				*dest = colormap[source[(frac >> FRACBITS) & heightmask]];
		}
	}
}
#endif							//USEBOOMFUNC

#ifndef USEBOOMFUNC
void R_DrawSkyColumn_8(void)
{
	register int count;
	register uint8_t* dest;
	register fixed_t frac;
	register fixed_t fracstep;
	
	count = dc_yh - dc_yl;
	
	// Zero length, column does not exceed a pixel.
	if (count < 0)
		return;
		
#ifdef RANGECHECK
	if ((unsigned)dc_x >= vid.width || dc_yl < 0 || dc_yh >= vid.height)
		I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif
		
	// Framebuffer destination address.
	// Use ylookup LUT to avoid multiply with ScreenWidth.
	// Use columnofs LUT for subwindows?
	dest = activeylookup[dc_yl] + columnofs[dc_x];
	
	// Determine scaling,
	//  which is the only mapping to be done.
	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl - centery) * fracstep;
	
	// Inner loop that does the actual texture mapping,
	//  e.g. a DDA-lile scaling.
	// This is as fast as it gets.
	do
	{
		// Re-map color indices from wall texture column
		//  using a lighting/special effects LUT.
		*dest = dc_colormap[dc_source[(frac >> FRACBITS) & 255]];
		
		dest += dc_drawymove;
		frac += fracstep;
		
	}
	while (count--);
}
#else
void R_DrawSkyColumn_8(void)
{
	int count;
	register uint8_t* dest;
	register fixed_t frac;
	fixed_t fracstep;

	count = dc_yh - dc_yl + 1;

	if (count <= 0)				// Zero length, column does not exceed a pixel.
		return;

#ifdef RANGECHECK
	if ((unsigned)dc_x >= vid.width || dc_yl < 0 || dc_yh >= vid.height)
		I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

	if ((unsigned)dc_x >= vid.width || dc_yl < 0 || dc_yl > viewheight || dc_yh >= vid.height)
		return;

	// Framebuffer destination address.
	// Use ylookup LUT to avoid multiply with ScreenWidth.
	// Use columnofs LUT for subwindows?

	dest = activeylookup[dc_yl] + columnofs[dc_x];

	// Determine scaling, which is the only mapping to be done.

	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl - centery) * fracstep;

	// Inner loop that does the actual texture mapping,
	//  e.g. a DDA-lile scaling.
	// This is as fast as it gets.

	{
		register const uint8_t* source = dc_source;
		register const lighttable_t* colormap = dc_colormap;
		register int heightmask = 255;

		if (dc_texheight & heightmask)
		{
			heightmask++;
			heightmask <<= FRACBITS;

			if (frac < 0)
				while ((frac += heightmask) < 0);
			else
				while (frac >= heightmask)
					frac -= heightmask;

			do
			{
				// Re-map color indices from wall texture column
				//  using a lighting/special effects LUT.
				// heightmask is the Tutti-Frutti fix -- killough

				*dest = colormap[source[frac >> FRACBITS]];
				dest += dc_drawymove;
				if ((frac += fracstep) >= heightmask)
					frac -= heightmask;
			}
			while (--count);
		}
		else
		{
			while ((count -= 2) >= 0)	// texture height is a power of 2 -- killough
			{
				*dest = colormap[source[(frac >> FRACBITS) & heightmask]];
				dest += dc_drawymove;
				frac += fracstep;
				*dest = colormap[source[(frac >> FRACBITS) & heightmask]];
				dest += dc_drawymove;
				frac += fracstep;
			}
			if (count & 1)
				*dest = colormap[source[(frac >> FRACBITS) & heightmask]];
		}
	}
}
#endif							// USEBOOMFUNC

//  The standard Doom 'fuzzy' (blur, shadow) effect
//  originally used for spectres and when picking up the blur sphere
//
void R_DrawFuzzColumn_8(void)
{
	register int count;
	register uint8_t* dest;
	register fixed_t frac;
	register fixed_t fracstep;
	
	// Adjust borders. Low...
	if (!dc_yl)
		dc_yl = 1;
		
	// .. and high.
	if (dc_yh == viewheight - 1)
		dc_yh = viewheight - 2;
		
	count = dc_yh - dc_yl;
	
	// Zero length.
	if (count < 0)
		return;
		
#ifdef RANGECHECK
	if ((unsigned)dc_x >= vid.width || dc_yl < 0 || dc_yh >= vid.height)
	{
		I_Error("R_DrawFuzzColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
	}
#endif
	
	// Does not work with blocky mode.
	dest = activeylookup[dc_yl] + columnofs[dc_x];
	
	// Looks familiar.
	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl - centery) * fracstep;
	
	do
	{
		// Lookup framebuffer, and retrieve
		//  a pixel that is either one column
		//  left or right of the current one.
		// Add index from colormap to index.
		*dest = colormaps[6 * 256 + dest[fuzzoffset[fuzzpos]]];
		
		// Clamp table lookup index.
		if (++fuzzpos == FUZZTABLE)
			fuzzpos = 0;
			
		dest += dc_drawymove;
		
		frac += fracstep;
	}
	while (count--);
}

bool_t g_PaintBallMode = false;
uint8_t g_PBColor = 0;

/* R_DrawPaintballColumn_8() -- Paintball mode! */
void R_DrawPaintballColumn_8(void)
{
	register int count;
	register uint8_t* dest;
	register fixed_t frac;
	register fixed_t fracstep;
	
	// check out coords for src*
	if ((dc_yl < 0) || (dc_x >= vid.width))
		return;
		
	count = dc_yh - dc_yl;
	if (count < 0)
		return;
		
	if ((unsigned)dc_x >= vid.width || dc_yl < 0 || dc_yh >= vid.height)
		return;
		
	// FIXME. As above.
	//src  = ylookup[dc_yl] + columnofs[dc_x+2];
	dest = activeylookup[dc_yl] + columnofs[dc_x];
	
	// Looks familiar.
	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl - centery) * fracstep;
	
	// Here we do an additional index re-mapping.
	do
	{
		*dest = g_PBColor;
		dest += dc_drawymove;
		frac += fracstep;
	}
	while (count--);
}

void R_DrawShadeColumn_8(void)
{
	register int count;
	register uint8_t* dest;
	register fixed_t frac;
	register fixed_t fracstep;
	
	// check out coords for src*
	if ((dc_yl < 0) || (dc_x >= vid.width))
		return;
		
	count = dc_yh - dc_yl;
	if (count < 0)
		return;
		
#ifdef RANGECHECK
	if ((unsigned)dc_x >= vid.width || dc_yl < 0 || dc_yh >= vid.height)
	{
		I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
	}
#endif
	
	// FIXME. As above.
	//src  = ylookup[dc_yl] + columnofs[dc_x+2];
	dest = activeylookup[dc_yl] + columnofs[dc_x];
	
	// Looks familiar.
	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl - centery) * fracstep;
	
	// Here we do an additional index re-mapping.
	do
	{
		*dest = *(colormaps + (dc_source[frac >> FRACBITS] << 8) + (*dest));
		dest += dc_drawymove;
		frac += fracstep;
	}
	while (count--);
}

#ifndef USEBOOMFUNC
void R_DrawTranslucentColumn_8(void)
{
	register int count;
	register uint8_t* dest;
	register fixed_t frac;
	register fixed_t fracstep;
	
	// check out coords for src*
	if ((dc_yl < 0) || (dc_x >= vid.width) || (dc_yh >= vid.height) || (dc_yl < 0) || ((unsigned)dc_x >= vid.width) || activeylookup[dc_yl] == NULL)
		return;
		
	count = dc_yh - dc_yl;
	
	if (count < 0)
		return;
		
	// FIXME. As above.
	//src  = ylookup[dc_yl] + columnofs[dc_x+2];
	
	dest = activeylookup[dc_yl] + columnofs[dc_x];
	
	// Looks familiar.
	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl - centery) * fracstep;
	
	// Here we do an additional index re-mapping.
	do
	{
		*dest = dc_colormap[*(dc_transmap + (dc_source[frac >> FRACBITS] << 8) + (*dest))];
		dest += dc_drawymove;
		frac += fracstep;
	}
	while (count--);
}
#else
void R_DrawTranslucentColumn_8(void)
{
	register int count;
	register uint8_t* dest;
	register fixed_t frac;
	register fixed_t fracstep;

	count = dc_yh - dc_yl + 1;

	if (count <= 0)				// Zero length, column does not exceed a pixel.
		return;

	if ((unsigned)dc_x >= vid.width || dc_yl < 0 || dc_yh >= vid.height)
		return;

	// Framebuffer destination address.
	// Use ylookup LUT to avoid multiply with ScreenWidth.
	// Use columnofs LUT for subwindows?

	if (activeylookup[dc_yl] == NULL)
		return;

	dest = activeylookup[dc_yl] + columnofs[dc_x];

	// Determine scaling, which is the only mapping to be done.

	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl - centery) * fracstep;

	// Inner loop that does the actual texture mapping,
	//  e.g. a DDA-lile scaling.
	// This is as fast as it gets.

	{
		register const uint8_t* source = dc_source;

		//register const lighttable_t *colormap = dc_colormap;
		register int heightmask = dc_texheight - 1;

		if (dc_texheight & heightmask)
		{
			heightmask++;
			heightmask <<= FRACBITS;

			if (frac < 0)
				while ((frac += heightmask) < 0);
			else
				while (frac >= heightmask)
					frac -= heightmask;

			do
			{
				// Re-map color indices from wall texture column
				//  using a lighting/special effects LUT.
				// heightmask is the Tutti-Frutti fix -- killough

				*dest = dc_colormap[*(dc_transmap + (source[frac >> FRACBITS] << 8) + (*dest))];
				dest += dc_drawymove;
				if ((frac += fracstep) >= heightmask)
					frac -= heightmask;
			}
			while (--count);
		}
		else
		{
			while ((count -= 2) >= 0)	// texture height is a power of 2 -- killough
			{
				*dest = dc_colormap[*(dc_transmap + (source[frac >> FRACBITS] << 8) + (*dest))];
				dest += dc_drawymove;
				frac += fracstep;
				*dest = dc_colormap[*(dc_transmap + (source[frac >> FRACBITS] << 8) + (*dest))];
				dest += dc_drawymove;
				frac += fracstep;
			}
			if (count & 1)
				*dest = dc_colormap[*(dc_transmap + (source[frac >> FRACBITS] << 8) + (*dest))];
		}
	}
}
#endif							// USEBOOMFUNC

// New spiffy function.
// Not only does it colormap a sprite,
// but does translucency as well.
// SSNTails 11-11-2002
// Uber-kudos to Cyan Helkaraxe for
// helping me get the brain juices flowing!
void R_DrawTranslatedTranslucentColumn_8(void)
{
	register int count;
	register uint8_t* dest;
	register fixed_t frac;
	register fixed_t fracstep;
	
	count = dc_yh - dc_yl + 1;
	
	if (count <= 0)				// Zero length, column does not exceed a pixel.
		return;
		
	// FIXME. As above.
	//src  = ylookup[dc_yl] + columnofs[dc_x+2];
	dest = activeylookup[dc_yl] + columnofs[dc_x];
	
	// Looks familiar.
	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl - centery) * fracstep;
	
	// Inner loop that does the actual texture mapping,
	//  e.g. a DDA-lile scaling.
	// This is as fast as it gets.
	
	{
		register const uint8_t* source = dc_source;
		
		//register const lighttable_t *colormap = dc_colormap;
		register int heightmask = dc_texheight - 1;
		
		if (dc_texheight & heightmask)
		{
			heightmask++;
			heightmask <<= FRACBITS;
			
			if (frac < 0)
				while ((frac += heightmask) < 0);
			else
				while (frac >= heightmask)
					frac -= heightmask;
					
			do
			{
				// Re-map color indices from wall texture column
				//  using a lighting/special effects LUT.
				// heightmask is the Tutti-Frutti fix -- killough
				
				*dest = dc_colormap[*(dc_transmap + (dc_colormap[dc_translation[dc_source[frac >> FRACBITS]]] << 8) + (*dest))];
				
				dest += dc_drawymove;
				if ((frac += fracstep) >= heightmask)
					frac -= heightmask;
			}
			while (--count);
		}
		else
		{
			while ((count -= 2) >= 0)	// texture height is a power of 2 -- killough
			{
				*dest = dc_colormap[*(dc_transmap + (dc_colormap[dc_translation[dc_source[frac >> FRACBITS]]] << 8) + (*dest))];
				dest += dc_drawymove;
				frac += fracstep;
				*dest = dc_colormap[*(dc_transmap + (dc_colormap[dc_translation[dc_source[frac >> FRACBITS]]] << 8) + (*dest))];
				dest += dc_drawymove;
				frac += fracstep;
			}
			if (count & 1)
			{
				*dest = dc_colormap[*(dc_transmap + (dc_colormap[dc_translation[dc_source[frac >> FRACBITS]]] << 8) + (*dest))];
			}
		}
	}
}

//
//  Draw columns upto 128high but remap the green ramp to other colors
//
void R_DrawTranslatedColumn_8(void)
{
	register int count;
	register uint8_t* dest;
	register fixed_t frac;
	register fixed_t fracstep;
	
	count = dc_yh - dc_yl;
	if (count < 0)
		return;
		
#ifdef RANGECHECK
	if ((unsigned)dc_x >= vid.width || dc_yl < 0 || dc_yh >= vid.height)
	{
		I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
	}
#endif
	// FIXME. As above.
	dest = activeylookup[dc_yl] + columnofs[dc_x];
	
	// Looks familiar.
	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl - centery) * fracstep;
	
	// Here we do an additional index re-mapping.
	do
	{
		// Translation tables are used
		//  to map certain colorramps to other ones,
		//  used with PLAY sprites.
		// Thus the "green" ramp of the player 0 sprite
		//  is mapped to gray, red, black/indigo.
		*dest = dc_colormap[dc_translation[dc_source[frac >> FRACBITS]]];
		
		dest += vid.width;
		
		frac += fracstep;
	}
	while (count--);
}

// ==========================================================================
// SPANS
// ==========================================================================

//  Draws the actual span.
//
#ifndef USEBOOMFUNC
void R_DrawSpan_8(void)
{
	register uint32_t xfrac;
	register uint32_t yfrac;
	register uint8_t* dest;
	register int count;
	
#ifdef RANGECHECK
	if (ds_x2 < ds_x1 || ds_x1 < 0 || ds_x2 >= vid.width || (unsigned)ds_y > vid.height)
	{
		I_Error("R_DrawSpan: %i to %i at %i", ds_x1, ds_x2, ds_y);
	}
#endif
	
	xfrac = ds_xfrac & 0x3fFFff;
	yfrac = ds_yfrac;
	
	dest = activeylookup[ds_y] + columnofs[ds_x1];
	
	// We do not check for zero spans here?
	count = ds_x2 - ds_x1;
	
	do
	{
		// Lookup pixel from flat texture tile,
		//  re-index using light/colormap.
		*dest++ = ds_colormap[ds_source[((yfrac >> (16 - 6)) & (0x3f << 6)) | (xfrac >> 16)]];
		
		// Next step in u,v.
		xfrac += ds_xstep;
		yfrac += ds_ystep;
		xfrac &= 0x3fFFff;
	}
	while (count--);
}
#else
void R_DrawSpan_8(void)
{
	register uint32_t xfrac;
	register uint32_t yfrac;
	register uint8_t* dest;
	register int count;

#ifdef RANGECHECK
	if (ds_x2 < ds_x1 || ds_x1 < 0 || ds_x2 >= vid.width || (unsigned)ds_y > vid.height)
	{
		I_Error("R_DrawSpan: %i to %i at %i", ds_x1, ds_x2, ds_y);
	}
#endif

	xfrac = ds_xfrac & ((flatsize << FRACBITS) - 1);
	yfrac = ds_yfrac;

	dest = activeylookup[ds_y] + columnofs[ds_x1];

	// We do not check for zero spans here?
	count = ds_x2 - ds_x1;

	do
	{
		count = count;
		// Lookup pixel from flat texture tile,
		//  re-index using light/colormap.
		*dest++ = ds_colormap[ds_source[((yfrac >> (16 - flatsubtract)) & (flatmask)) | (xfrac >> 16)]];

		// Next step in u,v.
		xfrac += ds_xstep;
		yfrac += ds_ystep;
		xfrac &= (flatsize << FRACBITS) - 1;
	}
	while (count--);
}
#endif							// USEBOOMFUNC

void R_DrawTranslucentSpan_8(void)
{
	fixed_t xfrac;
	fixed_t yfrac;
	fixed_t xstep;
	fixed_t ystep;
	uint8_t* dest;
	int count;
	
#ifdef RANGECHECK
	if (ds_x2 < ds_x1 || ds_x1 < 0 || ds_x2 >= screen->width || ds_y > screen->height)
	{
		I_Error("R_DrawSpan: %i to %i at %i", ds_x1, ds_x2, ds_y);
	}
//              dscount++;
#endif

	xfrac = ds_xfrac & ((flatsize << FRACBITS) - 1);
	yfrac = ds_yfrac;
	
	dest = activeylookup[ds_y] + columnofs[ds_x1];
	
	// We do not check for zero spans here?
	count = ds_x2 - ds_x1 + 1;
	
	xstep = ds_xstep;
	ystep = ds_ystep;
	
	do
	{
		// Current texture index in u,v.
		
		// Awesome! 256x256 flats!
//              spot = ((yfrac>>(16-8))&(0xff00)) + (xfrac>>(16));

		// Lookup pixel from flat texture tile,
		//  re-index using light/colormap.
		//      *dest++ = ds_colormap[ds_source[spot]];
//              *dest++ = ds_colormap[*(ds_transmap + (ds_source[spot] << 8) + (*dest))];
		*dest++ = ds_colormap[*(ds_transmap + (ds_source[((yfrac >> (16 - flatsubtract)) & (flatmask)) | (xfrac >> 16)] << 8) + (*dest))];
		
		// Next step in u,v.
		xfrac += xstep;
		yfrac += ystep;
		xfrac &= ((flatsize << FRACBITS) - 1);
	}
	while (--count);
	/*
	   register unsigned position;
	   unsigned step;
	
	   uint8_t *source;
	   uint8_t *colormap;
	   uint8_t *transmap;
	   uint8_t *dest;
	
	   unsigned count;
	   unsigned spot;
	   unsigned xtemp;
	   unsigned ytemp;
	
	   position = ((ds_xfrac<<10)&0xffff0000) | ((ds_yfrac>>6)&0xffff);
	   step = ((ds_xstep<<10)&0xffff0000) | ((ds_ystep>>6)&0xffff);
	
	   source = ds_source;
	   colormap = ds_colormap;
	   transmap = ds_transmap;
	   dest = ylookup[ds_y] + columnofs[ds_x1];
	   count = ds_x2 - ds_x1 + 1;
	
	   while (count >= 4)
	   {
	   ytemp = position>>4;
	   ytemp = ytemp & 0xff00;
	   xtemp = position>>26;
	   spot = xtemp | ytemp;
	   position += step;
	   dest[0] = colormap[*(transmap + (source[spot] << 8) + (dest[0]))];
	
	   ytemp = position>>4;
	   ytemp = ytemp & 0xff00;
	   xtemp = position>>26;
	   spot = xtemp | ytemp;
	   position += step;
	   dest[1] = colormap[*(transmap + (source[spot] << 8) + (dest[1]))];
	
	   ytemp = position>>4;
	   ytemp = ytemp & 0xff00;
	   xtemp = position>>26;
	   spot = xtemp | ytemp;
	   position += step;
	   dest[2] = colormap[*(transmap + (source[spot] << 8) + (dest[2]))];
	
	   ytemp = position>>4;
	   ytemp = ytemp & 0xff00;
	   xtemp = position>>26;
	   spot = xtemp | ytemp;
	   position += step;
	   dest[3] = colormap[*(transmap + (source[spot] << 8) + (dest[3]))];
	
	   dest += 4;
	   count -= 4;
	   }
	
	   while (count--)
	   {
	   ytemp = position>>4;
	   ytemp = ytemp & 0xff00;
	   xtemp = position>>26;
	   spot = xtemp | ytemp;
	   position += step;
	   *dest++ = colormap[*(transmap + (source[spot] << 8) + (*dest))];
	   //count--;
	   }
	 */
}

void R_DrawFogSpan_8(void)
{
	uint8_t* colormap;
	uint8_t* transmap;
	uint8_t* dest;
	
	unsigned count;
	
	colormap = ds_colormap;
	transmap = ds_transmap;
	dest = activeylookup[ds_y] + columnofs[ds_x1];
	count = ds_x2 - ds_x1 + 1;
	
	while (count >= 4)
	{
		dest[0] = colormap[dest[0]];
		
		dest[1] = colormap[dest[1]];
		
		dest[2] = colormap[dest[2]];
		
		dest[3] = colormap[dest[3]];
		
		dest += 4;
		count -= 4;
	}
	
	while (count--)
		*dest++ = colormap[*dest];
}

//SoM: Fog wall.
void R_DrawFogColumn_8(void)
{
	int count;
	uint8_t* dest;
	
	count = dc_yh - dc_yl;
	
	// Zero length, column does not exceed a pixel.
	if (count < 0)
		return;
		
#ifdef RANGECHECK
	if ((unsigned)dc_x >= vid.width || dc_yl < 0 || dc_yh >= vid.height)
		I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif
		
	// Framebuffer destination address.
	// Use ylookup LUT to avoid multiply with ScreenWidth.
	// Use columnofs LUT for subwindows?
	dest = activeylookup[dc_yl] + columnofs[dc_x];
	
	// Determine scaling,
	//  which is the only mapping to be done.
	
	do
	{
		//Simple. Apply the colormap to what's allready on the screen.
		*dest = dc_colormap[*dest];
		dest += dc_drawymove;
	}
	while (count--);
}

// SoM: This is for 3D floors that cast shadows on walls.
// This function just cuts the column up into sections and calls
// R_DrawColumn_8
void R_DrawColumnShadowed_8(void)
{
	int count;
	int realyh, realyl;
	int i;
	int height, bheight = 0;
	int solid = 0;
	
	realyh = dc_yh;
	realyl = dc_yl;
	
	count = dc_yh - dc_yl;
	
	// Zero length, column does not exceed a pixel.
	if (count < 0)
		return;
		
#ifdef RANGECHECK
	if ((unsigned)dc_x >= vid.width || dc_yl < 0 || dc_yh >= vid.height)
		I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif
		
	// SoM: This runs through the lightlist from top to bottom and cuts up
	// the column accordingly.
	for (i = 0; i < dc_numlights; i++)
	{
		// If the height of the light is above the column, get the colormap
		// anyway because the lighting of the top should be effected.
		solid = dc_lightlist[i].flags & FF_CUTSOLIDS;
		
		height = dc_lightlist[i].height >> 12;
		if (solid)
			bheight = dc_lightlist[i].botheight >> 12;
		if (height <= dc_yl)
		{
			dc_colormap = dc_lightlist[i].rcolormap;
			if (solid && dc_yl < bheight)
				dc_yl = bheight;
			continue;
		}
		// Found a break in the column!
		dc_yh = height;
		
		if (dc_yh > realyh)
			dc_yh = realyh;
		R_DrawColumn_8();
		if (solid)
			dc_yl = bheight;
		else
			dc_yl = dc_yh + 1;
			
		dc_colormap = dc_lightlist[i].rcolormap;
	}
	dc_yh = realyh;
	if (dc_yl <= realyh)
		R_DrawColumn_8();
}
