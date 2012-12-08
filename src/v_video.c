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
#include "r_defs.h"
#include "i_util.h"
#include "vhw_wrap.h"

// Each screen is [vid.width*vid.height];
uint8_t* screens[5];

/*** GAMMA CVAR ***/
// c_CVPVVidGamma -- Gamme Limitation
const CONL_VarPossibleValue_t c_CVPVVidGamma[] =
{
	// End
	{0, "MINVAL"},
	{4, "MAXVAL"},
	{0, NULL},
};

bool_t V_VIDGammaOnChange(CONL_ConVariable_t* const a_Var, CONL_StaticVar_t* const a_StaticVar);

// vid_gamma -- Screen Gamma
CONL_StaticVar_t l_VIDGamma =
{
	CLVT_INTEGER, &c_CVPVVidGamma, CLVF_SAVE,
	"vid_gamma", DSTR_CVHINT_SVJOINPASSWORD, CLVVT_BAR, "0",
	V_VIDGammaOnChange
};
/******************/

// Now where did these came from?
uint8_t gammatable[5][256] =
{
	{
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
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
		255
	},
	
	{
		2, 4, 5, 7, 8, 10, 11, 12, 14, 15, 16, 18, 19, 20, 21, 23, 24, 25, 26, 27,
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
		247, 248, 249, 250, 251, 252, 252, 253, 254, 255
	},
	
	{
		4, 7, 9, 11, 13, 15, 17, 19, 21, 22, 24, 26, 27, 29, 30, 32, 33, 35, 36,
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
		255
	},
	
	{
		8, 12, 16, 19, 22, 24, 27, 29, 31, 34, 36, 38, 40, 41, 43, 45, 47, 49, 50,
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
		253, 253, 254, 254, 255
	},
	
	{
		16, 23, 28, 32, 36, 39, 42, 45, 48, 50, 53, 55, 57, 60, 62, 64, 66, 68, 69,
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
		251, 252, 252, 253, 254, 254, 255, 255
	}
};

// keep a copy of the palette so that we can get the RGB
// value for a color index at any time.
// local copy of the palette for V_GetColor()
RGBA_t* pLocalPalette = NULL;
uint8_t g_ThreePal[256][3];

RGBA_t** l_DoomPals = NULL;
uint8_t** l_MappedDoomPals = NULL;
size_t l_NumDoomPals = 0;

unsigned char NearestColor(unsigned char r, unsigned char g, unsigned char b);

/* LoadPalette() -- yucky name, loads a palette for usage */
// GhostlyDeath <December 14, 2011> -- Updated to WL
void LoadPalette(char* lumpname)
{
	size_t NumBasePals, BasePal, Len;
	size_t i, j, k, l, m;
	uint8_t* PlayPal;
	fixed_t pR[256], pG[256], pB[256];
	const WL_WADEntry_t* PalEntry;
	RGBA_t* RGBData;
	uint8_t* MappedData;
	
	/* There are no VWADs? */
	if (!WL_IterateVWAD(NULL, true))
		return;
	
	/* Get index of lump */
	// Get lump
	PalEntry = WL_FindEntry(NULL, 0, "PLAYPAL");
	PlayPal = Z_Malloc(256 * 3 * 14 * sizeof(uint8_t), PU_STATIC, NULL);
	
	// Load into PlayPal
	if (PalEntry)
	{
		WL_ReadData(PalEntry, 0, PlayPal, 256 * 3 * 14 * sizeof(uint8_t));
	}
	
	// If not found, use fake yuck palette
	else
	{
		// A nice long big loop for a grayscale palette
		for (i = 0; i < 14; i++)
			for (j = 0; j < 256; j++)
				for (k = 0; k < 3; k++)
					PlayPal[(768 * i) + (j * 3) + k] = j;
	}
	
	// Get number of base palettes
	Len = PalEntry->Size / 3;
	NumBasePals = Len / 256;
	Len = NumBasePals * 256;	// Back again (for chunked off palettes)
	
	/* Free any old palettes */
	if (l_DoomPals)
	{
		for (i = 0; i < l_NumDoomPals; i++)
			Z_Free(l_DoomPals[i]);
		Z_Free(l_DoomPals);
	}
	
	/* Allocate new palettes */
	l_DoomPals = Z_Malloc(sizeof(*l_DoomPals) * (NumBasePals * VPALSMOOTHCOUNT), PU_STATIC, NULL);
	
	/* Now load the base palettes into memory */
	for (j = 0, i = 0; i < NumBasePals; i++)
	{
		// Remember for easy usage
		k = i * VPALSMOOTHCOUNT;
		// Allocate base palette area
		l_DoomPals[k] = Z_Malloc(sizeof(RGBA_t) * 256, PU_STATIC, NULL);
		
		// Load in from lump
		for (l = 0; l < 256; l++, j += 3)
		{
			l_DoomPals[k][l].s.red = PlayPal[j];
			l_DoomPals[k][l].s.green = PlayPal[j + 1];
			l_DoomPals[k][l].s.blue = PlayPal[j + 2];
			l_DoomPals[k][l].s.alpha = 0xFF;
		}
	}
	
	/* Set palette count */
	l_NumDoomPals = NumBasePals * VPALSMOOTHCOUNT;
	
	/* Start smoothing palettes */
	for (i = 0; i < l_NumDoomPals; i++)
	{
		// Get Base number
		BasePal = i / VPALSMOOTHCOUNT;
		
		// Base palette for interpolation
		if ((i % VPALSMOOTHCOUNT) == 0)
		{
			// Reset l and set m to i
			l = 0;
			m = i;
			
			// The next actual color
			k = (i + (VPALSMOOTHCOUNT));
			
			// There is a valid palette here
			if ((BasePal != 8 && BasePal != 12) && (k < l_NumDoomPals && l_DoomPals[k]))
			{
				for (j = 0; j < 256; j++)
				{
					pR[j] = ((l_DoomPals[k][j].s.red - l_DoomPals[i][j].s.red) << FRACBITS) / VPALSMOOTHCOUNT;
					pG[j] = ((l_DoomPals[k][j].s.green - l_DoomPals[i][j].s.green) << FRACBITS) / VPALSMOOTHCOUNT;
					pB[j] = ((l_DoomPals[k][j].s.blue - l_DoomPals[i][j].s.blue) << FRACBITS) / VPALSMOOTHCOUNT;
				}
			}
			// It is invalid
			else
			{
				// No interpolation
				memset(pR, 0, sizeof(pR));
				memset(pG, 0, sizeof(pG));
				memset(pB, 0, sizeof(pB));
			}
		}
		// Interpolate with colors
		else
		{
			// Increment l
			l++;
			
			// Create here if it does not already exist
			if (!l_DoomPals[i])
				l_DoomPals[i] = Z_Malloc(sizeof(RGBA_t) * 256, PU_STATIC, NULL);
				
			// Fancy loop
			for (j = 0; j < 256; j++)
			{
				l_DoomPals[i][j].s.red = ((l_DoomPals[m][j].s.red << FRACBITS) + (pR[j] * l)) >> FRACBITS;
				l_DoomPals[i][j].s.green = ((l_DoomPals[m][j].s.green << FRACBITS) + (pG[j] * l)) >> FRACBITS;
				l_DoomPals[i][j].s.blue = ((l_DoomPals[m][j].s.blue << FRACBITS) + (pB[j] * l)) >> FRACBITS;
			}
		}
	}
	
	/* For existing COLORMAP compat in r_data.c, set pLocalPalette */
	pLocalPalette = l_DoomPals[0];
	
	/* Don't need this local anymore */
	Z_Free(PlayPal);
	
	/* Re-map PLAYPAL entries into the colormapped variety */
	// This is for split screen usage
	
	// Remove old mappings
	if (l_MappedDoomPals)
	{
		for (i = 0; i < l_NumDoomPals; i++)
			Z_Free(l_MappedDoomPals[i]);
		Z_Free(l_MappedDoomPals);
		l_MappedDoomPals = NULL;
	}
	
	// Re-Allocate
	l_MappedDoomPals = Z_Malloc(sizeof(*l_MappedDoomPals) * l_NumDoomPals, PU_STATIC, NULL);
	
	// Run through each palette, of each color
	for (i = 0; i < l_NumDoomPals; i++)
	{
		// Get RGB Array
		RGBData = l_DoomPals[i];
		
		// Allocate mapped copy
		l_MappedDoomPals[i] = MappedData = Z_Malloc(256, PU_STATIC, NULL);
		
		// Convert all colors
		for (j = 0; j < 256; j++)
			MappedData[j] = NearestColor(RGBData[j].s.red, RGBData[j].s.green, RGBData[j].s.blue);
	}
}

extern bool_t g_FramePipe;

/* V_SetPalette() -- Set the current palette */
void V_SetPalette(int palettenum)
{
	RGBA_t* FakePal;
	size_t r, g, b, i;
	
	/* Load palletes if possible */
	if (!l_DoomPals)
		LoadPalette("PLAYPAL");
	
	/* If there is no palette, load a cheap one */
	if (!l_DoomPals)
	{
		FakePal = Z_Malloc(sizeof(*FakePal) * 256, PU_STATIC, NULL);
		
		// Boot Image Palette
		FakePal[1].s.red = 0x05;
		FakePal[1].s.green = 0x04;
		FakePal[1].s.blue = 0x00;
		
		FakePal[2].s.red = 0x4c;
		FakePal[2].s.green = 0x01;
		FakePal[2].s.blue = 0x00;
		
		FakePal[3].s.red = 0x9b;
		FakePal[3].s.green = 0x00;
		FakePal[3].s.blue = 0x00;
		
		FakePal[4].s.red = 0xfd;
		FakePal[4].s.green = 0xff;
		FakePal[4].s.blue = 0xfc;
		
		// Red Loading Bar
		FakePal[5].s.red = 0xff;
		FakePal[5].s.green = 0xff;
		FakePal[5].s.blue = 0xff;
		FakePal[6].s.red = 0xff;
		FakePal[6].s.green = 0x00;
		FakePal[6].s.blue = 0x00;
		
		// Send in
		I_SetPalette(FakePal);
		Z_Free(FakePal);
		return;
	}
	
	/* Use lump data */
	if (palettenum < 0 || palettenum >= l_NumDoomPals || !l_DoomPals[palettenum])
		I_SetPalette(l_DoomPals[(palettenum = 0)]);
	else
		I_SetPalette(l_DoomPals[palettenum]);
	
	/* Fill triple */
	if (g_FramePipe)
		for (i = 0; i < 256; i++)
		{
			g_ThreePal[i][0] = l_DoomPals[palettenum][i].s.red;
			g_ThreePal[i][1] = l_DoomPals[palettenum][i].s.green;
			g_ThreePal[i][2] = l_DoomPals[palettenum][i].s.blue;
		}
}

/* V_GetPalette() -- Gets palette */
uint8_t* V_GetPalette(int palettenum)
{
	if (!l_DoomPals)
		LoadPalette("PLAYPAL");
		
	if (palettenum < 0 || palettenum >= l_NumDoomPals || !l_DoomPals[palettenum])
		return l_DoomPals[0];
	else
		return l_DoomPals[palettenum];
}

/* V_GetPaletteMapped() -- Gets colormapped version of palette */
uint8_t* V_GetPaletteMapped(int palettenum)
{
	if (palettenum < 0 || palettenum >= l_NumDoomPals || !l_DoomPals[palettenum])
		return l_MappedDoomPals[0];
	else
		return l_MappedDoomPals[palettenum];
}

/* V_SetPaletteLump -- Set the current palette based on the lump */
void V_SetPaletteLump(char* pal)
{
	LoadPalette(pal);
	I_SetPalette(l_DoomPals);
}

/* V_VIDGammaOnChange() -- Gamma is changed */
bool_t V_VIDGammaOnChange(CONL_ConVariable_t* const a_Var, CONL_StaticVar_t* const a_StaticVar)
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

static int QuickRound(float x)
{
	if ((x - (int)x) > 0.5)
		return x + 1;
	else
		return x;
}

// --------------------------------------------------------------------------
// Copy a rectangular area from one bitmap to another (8bpp)
// srcPitch, destPitch : width of source and destination bitmaps
// --------------------------------------------------------------------------
void VID_BlitLinearScreen(uint8_t* srcptr, uint8_t* destptr, int width, int height, int srcrowbytes, int destrowbytes)
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
void V_DrawBlock(int x, int y, int scrn, int width, int height, uint8_t* src)
{
	uint8_t* dest;
	
	if (!graphics_started)
		return;
		
#ifdef RANGECHECK
	if (x < 0 || x + width > vid.width || y < 0 || y + height > vid.height || (unsigned)scrn > 4)
	{
		I_Error("Bad V_DrawBlock");
	}
#endif
	
	dest = screens[scrn] + y * vid.width + x;
	
	while (height--)
	{
		memcpy(dest, src, width);
		
		src += width;
		dest += vid.width;
	}
}

//
//  Fills a box of pixels using a flat texture as a pattern,
//  scaled to screen size.
//
//added:06-02-98:

/* V_DrawFadeScreen() -- Pixelate and draw dark */
void V_DrawFadeScreen(void)
{
	int x, y, i, w;
	int* buf;
	int* buf2;
	int c;
	uint8_t* fadetable = (uint8_t*)colormaps + 16 * 256;
	
	// Speed
	w = (vid.width >> 2);
	
	// Loop
	for (y = 0; y < vid.height; y += 8)
	{
		// Set buf
		buf = (int*)(screens[0] + vid.width * y);
		
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
			buf2 = (int*)(screens[0] + vid.width * (y + i));
			memcpy(buf2, buf, vid.width);
		}
	}
}


/* VS_VideoWADOrderCB() -- Video callback for WAD order changing */
// This does video related operations (such as loading the palette)
static bool_t VS_VideoWADOrderCB(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD)
{
	/* Load PLAYPAL */
	
	/* Load colormaps */
	V_InitializeColormaps();
	
	/* Clear all defined image info */
	
	return true;
}

/* V_Init() -- Initializes the video interface */
// GhostlyDeath <December 14, 2011> -- Refactored for WL
// This function is called more than once!
void V_Init(void)
{
	static bool_t InitialOK = false;
	int i;
	uint8_t* base;
	int screensize;
	
	/* Needs initial setup? */
	if (!InitialOK)
	{
		// Register order change callback
		if (!WL_RegisterOCCB(VS_VideoWADOrderCB, 100))
			I_Error("V_Init: Failed to register OCCB.\n");
		
		// OK now
		InitialOK = true;
	}
	
	/* Allocate video buffers */
	//added:26-01-98:start address of NUMSCREENS * width*height vidbuffers
	base = vid.buffer;
	
	screensize = vid.width * vid.height * vid.bpp;
	
	for (i = 0; i < NUMSCREENS; i++)
		screens[i] = base + i * screensize;
		
	//added:26-01-98: statusbar buffer
	screens[4] = base + NUMSCREENS * screensize;
}


// #############################################################################
// #############################################################################
// ####                      EXTENDED DRAWING FUNCTIONS                     ####
// #############################################################################
// #############################################################################

/*************
*** LOCALS ***
*************/

static uint8_t* l_ColorMaps[NUMVEXCOLORS * 2];		// Local colors
static uint8_t* l_LifeBarColors;				// Life Bar Colors

/*****************
*** STRUCTURES ***
*****************/

/* V_ColorEntry_t -- HSV table */
typedef union V_ColorEntry_s
{
	struct
	{
		uint8_t R;
		uint8_t G;
		uint8_t B;
	} RGB;
	
	struct
	{
		uint8_t H;
		uint8_t S;
		uint8_t V;
	} HSV;
} V_ColorEntry_t;

/****************
*** FUNCTIONS ***
****************/

/* V_ReturnColormapPtr() -- Return pointer to colormap */
const uint8_t* V_ReturnColormapPtr(const VEX_ColorList_t Color)
{
	/* Check */
	if (Color < 0 || Color >= (NUMVEXCOLORS * 2))
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
	uint8_t rMin, rMax, rDif;
	
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
	Ret.HSV.S = (uint8_t)(((uint32_t)255 * (uint32_t)rDif) / (uint32_t)Ret.HSV.V);
	
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
	int32_t BestSqr, ThisSqr, Dr, Dg, Db;
	
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
// GhostlyDeath <September 16, 2011> -- Rewritten for RMD_CMAP instead of
// dynamically creating the colors at run time.
// GhostlyDeath <December 14, 2011> -- Modified for WL
void V_InitializeColormaps(void)
{
	size_t i, j, m, n;
	uint8_t* Maps;
	const WL_WADEntry_t* Entry;
	const char* CMAPName;
	
	/* Life Bars */
	// Free
	if (l_LifeBarColors)
		Z_Free(l_LifeBarColors);
	l_LifeBarColors = Z_Malloc(sizeof(*l_LifeBarColors) * 200, PU_STATIC, NULL);
	
	// Which?
	if (g_CoreGame == COREGAME_DOOM)
		CMAPName = "RMD_PLBD";
	else if (g_CoreGame == COREGAME_HERETIC)
		CMAPName = "RMD_PLBH";
	else
		CMAPName = NULL;
		
	// Get
	Entry = WL_FindEntry(NULL, 0, CMAPName);
		
	// Constant read in the lump
	if (Entry)
		for (i = 0; i < 200; i++)
			WL_ReadData(Entry, i, &l_LifeBarColors[i], sizeof(l_LifeBarColors[i]));
	
	/* Destroy old maps */
	for (i = 0; i < (NUMVEXCOLORS * 2); i++)
		if (l_ColorMaps[i])
		{
			Z_Free(l_ColorMaps[i]);
			l_ColorMaps[i] = NULL;
		}
		
	/* Allocate maps, and initialize */
	for (i = 0; i < (NUMVEXCOLORS * 2); i++)
	{
		// Does not exist?
		if (!l_ColorMaps[i])
			l_ColorMaps[i] = Z_Malloc(sizeof(*l_ColorMaps[i]) * 256, PU_STATIC, NULL);
			
		// Initialize
		for (j = 0; j < 256; j++)
			l_ColorMaps[i][j] = j;
	}
	
	/* Obtain maps */
	// Which?
	if (g_CoreGame == COREGAME_DOOM)
		CMAPName = "RMD_CMAP";
	else if (g_CoreGame == COREGAME_HERETIC)
		CMAPName = "RMD_CMAQ";
	else
		CMAPName = NULL;
	
	// Get
	Entry = WL_FindEntry(NULL, 0, CMAPName);
		
	/* Constant read in the lump and set the translation stuff */
	if (Entry)
		for (m = 0, i = 0; i < (NUMVEXCOLORS * 2); i++)
			for (j = 0; j < 256 && m < Entry->Size; j++, m++)
				WL_ReadData(Entry, m, &l_ColorMaps[i][j], sizeof(l_ColorMaps[i][j]));
}

/* V_DrawFadeConsBackEx() -- Pixelate and add red tint */
void V_DrawFadeConsBackEx(const uint32_t Flags, const int x1, const int y1, const int x2, const int y2)
{
	int X1, Y1, X2, Y2;
	int x, y, i, w;
	uint8_t* buf;
	uint32_t* buf2;
	uint32_t c;
	uint8_t* Map;
	
	uint8_t* vBase;
	uint32_t Pitch;
	
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
		X1 = (float)x1 *(float)vid.fdupx;
		Y1 = (float)y1 *(float)vid.fdupy;
		X2 = (float)x2 *(float)vid.fdupx;
		Y2 = (float)y2 *(float)vid.fdupy;
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
		Map = l_ColorMaps[0];
		
		
	/* Obtain screen */
	vBase = I_GetVideoBuffer(IVS_BACKBUFFER, &Pitch);
	
	/* Actual Drawing */
	// Speed
	w = (X2 >> 2);
	
	// Loop
	for (y = Y1; y < Y2; y += 8)
	{
		// Set buf
		buf = vBase + ((Pitch * y) + X1);
		
		// Loop
		c = Map[buf[X1] & 0xFF];
		x = 0;
		for (; x < (X2 - X1) - 8; x += 8)
		{
			c = Map[((uint8_t*)buf)[x]];
			c |= c << 8;
			c |= c << 16;
			*((uint32_t*)(&buf[x])) = c;
			*((uint32_t*)(&buf[x + 4])) = c;
		}
		
		// Final bits
		for (; x < (X2 - X1); x++)
			((uint8_t*)buf)[x] = c;
			
		// Inner second loop
		for (i = 1; i < 8 && (y + i) < Y2; i++)
		{
			buf2 = ((uint8_t*)vBase) + ((Pitch * (y + i)) + X1);
			memcpy(buf2, buf, X2 - X1);
		}
	}
	
	I_GetVideoBuffer(IVS_DONEWITHBUFFER, NULL);
}

/* V_DrawColorBoxEx() -- Draws a colorbox */
// GhostlyDeath <September 20, 2011> -- Based off V_DrawFadeConsBackEx() but with only a single solid color
void V_DrawColorBoxEx(const uint32_t a_Flags, const uint8_t a_Color, const int32_t a_x1, const int32_t a_y1, const int32_t a_x2, const int32_t a_y2)
{
	int X1, Y1, X2, Y2;
	int x, y, i, w;
	uint8_t* buf, *buf2;
	uint32_t c, Mask;
	uint8_t* Map, *ExtraMap, *TransMap;
	
	uint8_t* vBase;
	uint32_t Pitch;
	
	/* Flags */
	// Unscaled
	if (a_Flags & VEX_NOSCALESTART)
	{
		X1 = a_x1;
		Y1 = a_y1;
		X2 = a_x2;
		Y2 = a_y2;
	}
	// Scaled
	else
	{
		X1 = (float)a_x1 *(float)vid.fdupx;
		Y1 = (float)a_y1 *(float)vid.fdupy;
		X2 = (float)a_x2 *(float)vid.fdupx;
		Y2 = (float)a_y2 *(float)vid.fdupy;
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
		
	/* Obtain screen */
	vBase = I_GetVideoBuffer(IVS_BACKBUFFER, &Pitch);
		
	/* Mapping */
	if (((a_Flags & VEX_COLORMAPMASK) >> VEX_COLORMAPSHIFT) < NUMVEXCOLORS)
		Map = l_ColorMaps[(a_Flags & VEX_COLORMAPMASK) >> VEX_COLORMAPSHIFT];
	else
		Map = l_ColorMaps[0];
	
	/* Use player translation tables? */
	if (a_Flags & VEX_COLORSET)
	{
		// Use player translation tables?
		Mask = ((a_Flags & VEX_COLORMASK) >> VEX_COLORSHIFT) & 0xFU;
		
		ExtraMap = V_ReturnColormapPtr(16 + Mask);
	}
	else
		ExtraMap = V_ReturnColormapPtr(VEX_MAP_NONE);
		
	/* Handle transparency */
	// Transparency OK
	if (transtables)
	{
		x = ((a_Flags & VEX_FILLTRANSMASK) >> VEX_FILLTRANSSHIFT);
		if (x >= NUMVEXTRANSPARENCIES)
			x = 0;
		
		// Set table to use
		if (x > 0)
			TransMap = transtables + (0x10000 * x);
		else
			TransMap = NULL;
	}
	
	// No tables loaded
	else
		TransMap = NULL;
		
	/* Same Color all the way */
	if (ExtraMap && Map)
		c = ExtraMap[Map[a_Color]];
	else
		c = a_Color;
	c |= c << 8;
	c |= c << 16;
	
	/* Actual Drawing */
	// Speed
	w = (X2 >> 2);
	
	// Hollow (border) box
	if (a_Flags & VEX_HOLLOW)
	{
		// Top Line
		buf = (int*)(vBase + (Pitch * (Y1)) + X1);
		memset(buf, c, X2 - X1);
		
		// Bottom Line
		buf = (int*)(vBase + (Pitch * (Y2)) + X1);
		memset(buf, c, X2 - X1);
		
		// Side Lines
		for (y = Y1; y < Y2; y++)
		{
			// Set buf
			buf = (vBase + (Pitch * y) + X1);
			
			// Set single pixel values
			buf[0] = c & 0xFFU;
			buf[(X2 - X1) - 1] = c & 0xFFU;
		}
	}
	
	// Solid Box
	else
	{
		// Transparent Box (REALLY slow)
		if (TransMap)
			// Loop
			for (y = Y1; y < Y2; y ++)
			{
				// Set buf
				buf = (int*)(vBase + (Pitch * y) + X1);
				i = (((intptr_t)buf) & (intptr_t)7);
				
				// Loop entire row
				for (x = 0; x < (X2 - X1); x++)
					((uint8_t*)buf)[x] = TransMap[((c & 0xFFU) * 256) + (((uint8_t*)buf)[x])];
			}
		// Solid Box
		else
			// Loop
			for (y = Y1; y < Y2; y += 8)
			{
				// Set buf
				buf = (int*)(vBase + (Pitch * y) + X1);
				i = (((intptr_t)buf) & (intptr_t)7);
			
				// Pre-8 loop (prevents signaling buses)
				for (x = 0; x < i; x++)
					((uint8_t*)buf)[x] = c & 0xFFU;
			
				// Loop
				for (; x < (X2 - X1) - 8; x += 8)
				{
					*((uint32_t*)(&buf[x])) = c;
					*((uint32_t*)(&buf[x + 4])) = c;
				}
		
				// Final bits
				for (; x < (X2 - X1); x++)
					((uint8_t*)buf)[x] = c & 0xFFU;
			
				// Inner second loop
				for (i = 1; i < 8 && (y + i) < Y2; i++)
				{
					buf2 = (int*)(vBase + (Pitch * (y + i)) + X1);
					memcpy(buf2, buf, X2 - X1);
				}
			}
	}
	
	I_GetVideoBuffer(IVS_DONEWITHBUFFER, NULL);
}

/* V_DrawColorMapEx() -- Applies colormap to screen */
void V_DrawColorMapEx(const uint32_t a_Flags, const uint8_t* const a_ColorMap, const int32_t a_x1, const int32_t a_y1, const int32_t a_x2, const int32_t a_y2)
{
	int X1, Y1, X2, Y2;
	int x, y, i, w;
	uint8_t* buf, *buf2;
	uint32_t c, Mask;
	
	uint8_t* vBase;
	uint32_t Pitch;
	
	/* Check */
	if (!a_ColorMap)
		return;
	
	/* Flags */
	// Unscaled
	if (a_Flags & VEX_NOSCALESTART)
	{
		X1 = a_x1;
		Y1 = a_y1;
		X2 = a_x2;
		Y2 = a_y2;
	}
	// Scaled
	else
	{
		X1 = (float)a_x1 *(float)vid.fdupx;
		Y1 = (float)a_y1 *(float)vid.fdupy;
		X2 = (float)a_x2 *(float)vid.fdupx;
		Y2 = (float)a_y2 *(float)vid.fdupy;
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
		
	/* Obtain screen */
	vBase = I_GetVideoBuffer(IVS_BACKBUFFER, &Pitch);
	
	// Loop
	for (y = Y1; y < Y2; y ++)
	{
		// Set buf
		buf = (int*)(vBase + (Pitch * y) + X1);
		i = (((intptr_t)buf) & (intptr_t)7);
		
		// Loop entire row
		for (x = 0; x < (X2 - X1); x++)
			((uint8_t*)buf)[x] = a_ColorMap[((uint8_t*)buf)[x]];
	}
	
	I_GetVideoBuffer(IVS_DONEWITHBUFFER, NULL);
}

/********************
*** COMPATIBILITY ***
********************/

/* V_DrawFadeConsBack() -- Pixelate and add red tint */
void V_DrawFadeConsBack(int x1, int y1, int x2, int y2)
{
	V_DrawFadeConsBackEx((VEX_MAP_GREEN << VEX_COLORMAPSHIFT) | VEX_NOSCALESTART | VEX_NOSCALESCREEN, x1, y1, x2, y2);
}

// #############################################################################
// #############################################################################
// ####                   ASCII AND UNICODE HANDLING                        ####
// #############################################################################
// #############################################################################

/*** CONSTANTS ***/

/* V_FontNameRule_t -- Rule to use for fonts */
// Instead of hardcoding the formats into the initializer function. I will
// just create this simple index here, then have a table of functions that
// will turn a specific name into a UTF-16 character number.
typedef enum V_FontNameRule_e
{
	VFNR_NULL,								// No Rule for this
	VFNR_UNIVERSAL,							// [UFNnhhhh] Universal Font Number
	VFNR_STCFN,								// [STCFNddd] Doom STCFN Decimal
	VFNR_FONTX,								// [FONTnddd] Heretic, Odamex Decimal
	VFNR_BOOM,								// [DIG     ] PrBoom Decimal
	VFNR_STBARNUM,							// [STTNUMd ] Status bar numbers
	
	NUMVFONTNAMERULES
} V_FontNameRule_t;

/*** STRUCTURES ***/

/* V_FontInfo_t -- Font information */
typedef struct V_FontInfo_s
{
	/* Static */
	const bool_t LoadThisFont;				// Load this font?
	const char* NiceName;					// Font's nice name
	const char* ScriptName;					// Font's scripted name
	const char* Mappings[4];				// Doom (+Alt), Heretic (+Alt)
	V_FontNameRule_t MapRule[2];			// Mapping rules
	
	/* Dynamic */
	uint32_t NumChars;						// Number of actual characters
	uint32_t ScriptHash;					// Hash for scripting name
											// scripts lookup by name!
	int32_t CharWidth;						// Average character width
	int32_t CharHeight;						// Average character height
	int32_t wTotal;							// Width total
	int32_t hTotal;							// Height total
} V_FontInfo_t;

/* V_UniChar_t -- Character data */
typedef struct V_UniChar_s
{
	uint16_t Char;
	char MB[5];
	
	int16_t Size[2];			// Size of image
	
	const WL_WADEntry_t* Entry;	// Character Entry
	struct V_Image_s* Image;	// Character image
	
	struct UniChar_s* BuildTop;
	struct UniChar_s* BuildBottom;
	
	struct V_UniChar_s* Chain;	// Chain link
	bool_t Linked;				// Linked in chain
	bool_t DoNotFree;			// Do not free
	int32_t RefCount;			// Reference Count
} V_UniChar_t;

/* V_LocalFontStuff_t -- Info for a font in WAD */
typedef struct V_LocalFontStuff_s
{
	V_UniChar_t* FirstLink[NUMVIDEOFONTS];	// First link in the chain
	V_UniChar_t** CGroups[NUMVIDEOFONTS];	// Character groups for each font
	V_FontInfo_t DynInfo[NUMVIDEOFONTS];	// Dynamic loaded info (used w/ comp)
} V_LocalFontStuff_t;

/*** GLOBALS ***/

/*** LOCALS ***/

/* l_LocalFonts -- Contains info on all the local fonts */
static V_FontInfo_t l_LocalFonts[NUMVIDEOFONTS] =
{
	// Small Font (Just an alias)
	{
		false,
		"Small",
		"small",
		{"", "", "", ""},
		{VFNR_NULL, VFNR_NULL},
	},
	
	// Large font (Just an alias)
	{
		false,
		"Large",
		"large",
		{"", "", "", ""},
		{VFNR_NULL, VFNR_NULL},
	},
	
	// Small Statusbar Font
	{
		true,
		"Statusbar",
		"statusbar",
		{"UFNK", "", "UFNK", ""},
		{VFNR_UNIVERSAL, VFNR_NULL},
	},
	
	// Boom HUD Font
	{
		true,
		"Boom HUD",
		"boom",
		{"UFNJ", "DIG", "UFNJ", "DIG"},
		{VFNR_UNIVERSAL, VFNR_BOOM},
	},
	
	// ReMooD Console Font
	{
		true,
		"ReMooD OEM",
		"oem",
		{"UFNR", "", "UFNR", ""},
		{VFNR_UNIVERSAL, VFNR_NULL},
	},
	
	// Userspace Fonts
	{
		true,
		"User Font Alpha",
		"usera",
		{"UFNW", "", "UFNW", ""},
		{VFNR_UNIVERSAL, VFNR_NULL},
	},
	{
		true,
		"User Font Beta",
		"userb",
		{"UFNX", "", "UFNX", ""},
		{VFNR_UNIVERSAL, VFNR_NULL},
	},
	{
		true,
		"User Font Gamma",
		"userc",
		{"UFNY", "", "UFNY", ""},
		{VFNR_UNIVERSAL, VFNR_NULL},
	},
	{
		true,
		"User Font Delta",
		"userd",
		{"UFNZ", "", "UFNZ", ""},
		{VFNR_UNIVERSAL, VFNR_NULL},
	},
	
	// Small Doom Font
	{
		true,
		"Small Doom",
		"smalldoom",
		{"UFNA", "STCFN", "UFNA", "STCFN"},
		{VFNR_UNIVERSAL, VFNR_STCFN},
	},
	
	// Large Doom Font
	{
		true,
		"Large Doom",
		"largedoom",
		{"UFNB", "FONTC", "UFNB", "FONTC"},
		{VFNR_UNIVERSAL, VFNR_FONTX},
	},
	
	// Small Heretic Font
	{
		true,
		"Small Heretic",
		"smallheretic",
		{"UFNC", "FONTA", "UFNC", "FONTA"},
		{VFNR_UNIVERSAL, VFNR_FONTX},
	},
	
	// Large Heretic Font
	{
		true,
		"Large Heretic",
		"largeheretic",
		{"UFND", "FONTB", "UFND", "FONTB"},
		{VFNR_UNIVERSAL, VFNR_FONTX},
	},
	
	// Large Status Bar Font
	{
		true,
		"Large Doom Statusbar",
		"largestatusbar",
		{"UFNL", "STTNUM", "UFNL", "STTNUM"},
		{VFNR_UNIVERSAL, VFNR_STBARNUM},
	},
	
	// New ReMooD UI Font
	{
		true,
		"New UI",
		"newui",
		{"UFNU", "", "UFNU", ""},
		{VFNR_UNIVERSAL, VFNR_NULL},
	},
};

static V_UniChar_t l_VSpace[NUMVIDEOFONTS];			// Virtual space character
static V_UniChar_t*** l_CGroups[NUMVIDEOFONTS];		// Composite group
static V_UniChar_t* l_UnknownLink[NUMVIDEOFONTS];	// Unknown character for each font
static VideoFont_t l_FontRemap[NUMVIDEOFONTS];	// Remaps for each font

/*** FUNCTIONS ***/

/* VS_DetectByName() -- Detects the font type and such */
static bool_t VS_DetectByName(const char* const a_Name, uint16_t* const a_HexOut, VideoFont_t* const a_FontOut, V_FontNameRule_t* const a_RuleOut)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return false;
	
	/*** STANDARD CLIENT ***/
#else
	size_t i, j;
	int32_t Num[4];
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return false;
	
	/* Check */
	if (!a_Name || !a_HexOut || !a_FontOut || !a_RuleOut)
		return false;
	
	/* Check for the following rules in most to least liked */
	// [UFNnhhhh] Universal Font Number (VFNR_UNIVERSAL)
	if (strlen(a_Name) == 8 &&
		strncasecmp(a_Name, "UFN", 3) == 0 &&
		((a_Name[3] >= 'a' && a_Name[3] <= 'z') || (a_Name[3] >= 'A' && a_Name[3] <= 'Z')) &&
		isxdigit(a_Name[4]) && isxdigit(a_Name[5]) && isxdigit(a_Name[6]) && isxdigit(a_Name[7]))
	{
		// Return rule
		*a_RuleOut = VFNR_UNIVERSAL;
		
		// Cheat with numbers
		for (i = 4; i < 8; i++)
		{
			// Convert
			if (a_Name[i] >= 'A' && a_Name[i] <= 'Z')
				Num[i - 4] = (a_Name[i] - 'A') + 10;
			else if (a_Name[i] >= 'a' && a_Name[i] <= 'z')
				Num[i - 4] = (a_Name[i] - 'a') + 10;
			else
				Num[i - 4] = a_Name[i] - '0';
		}
		
		// Add hex numbers together
		*a_HexOut = (Num[0] << 12) + (Num[1] << 8) + (Num[2] << 4) + Num[3];
		
		// Find out which font this belongs to
		for (i = 0; i < NUMVIDEOFONTS; i++)
			for (j = 0; j < 2; j++)
				// Does the map rule match?
				if (l_LocalFonts[i].MapRule[j] == *a_RuleOut)
					// Does the namespace match?
					if (tolower(a_Name[3]) == tolower(l_LocalFonts[i].Mappings[j][3]))
					{
						// Break out
						*a_FontOut = i;
						i = NUMVIDEOFONTS;
						break;
					}
		
		// Success!
		return true;
	}
	
	// [STCFNddd] Doom STCFN Decimal (VFNR_STCFN)
	if (strlen(a_Name) == 8 &&
		strncasecmp(a_Name, "STCFN", 5) == 0 &&
		isdigit(a_Name[5]) && isdigit(a_Name[6]) && isdigit(a_Name[7]))
	{
		// Set Rule
		*a_RuleOut = VFNR_STCFN;
		
		// Convert number to an integer
		*a_HexOut = 0;
		for (i = 5; i < 8; i++)
		{
			*a_HexOut *= 10;	// Mul old number
			*a_HexOut += a_Name[i] - '0';
		}
		
		// Swap y and | since they foolishly are swapped in Doom
		if (*a_HexOut == 121)
			*a_HexOut = 124;
		else if (*a_HexOut == 124)
			*a_HexOut = 121;
		
		// Font is always Doom Small
		*a_FontOut = VFONT_SMALL_DOOM;
		
		// Success!
		return true;
	}
	
	// [FONTnddd] Heretic Decimal (VFNR_FONTX)
	if (strlen(a_Name) >= 6 &&
		strncasecmp(a_Name, "FONT", 4) == 0 &&
		isalpha(a_Name[4]) && isdigit(a_Name[5]))
	{
		// Set Rule
		*a_RuleOut = VFNR_FONTX;
		
		// Get Number
		*a_HexOut = 0;
		for (i = 5; i < 8; i++)
		{
			// Make sure it is valid
			if (!a_Name[i] || !isdigit(a_Name[i]))
				break;
			
			// Add number
			*a_HexOut *= 10;	// Mul old number
			*a_HexOut += a_Name[i] - '0';
		}
		
		// Offset by 32 (space)
		*a_HexOut += 32;
			
		// Which Font? Small or Big?
		*a_FontOut = 0;
		if (a_Name[4] == 'A' || a_Name[4] == 'a')
			*a_FontOut = VFONT_SMALL_HERETIC;
		else if (a_Name[4] == 'B' || a_Name[4] == 'b')
			*a_FontOut = VFONT_LARGE_HERETIC;
		
		// Success!
		if (*a_FontOut)
			return true;
	}
	
	// [DIG     ] Boom Decimal (VFNR_BOOM)
	
	// [STTNUMd ] Status bar numbers (VFNR_STBARNUM)
	if ((strlen(a_Name) == 7 &&
			strncasecmp(a_Name, "STTNUM", 6) == 0 &&
			isdigit(a_Name[6])) ||
		(strlen(a_Name) == 8 &&
			strncasecmp(a_Name, "STTPRCNT", 8) == 0) ||
		(strlen(a_Name) == 8 &&
			strncasecmp(a_Name, "STTMINUS", 8) == 0))
	{
		// Set Rule
		*a_RuleOut = VFNR_STBARNUM;
		
		// Check if this is the percent sign
		if (tolower(a_Name[3]) == 'p')
			*a_HexOut = '%';
		
		// Check for minus sign
		if (tolower(a_Name[3]) == 'm')
			*a_HexOut = '-';
		
		// The rest are always numbers
		else
			*a_HexOut = a_Name[6];
		
		// Font is always the large status bar font
		*a_FontOut = VFONT_STATUSBARLARGE;
		
		// Success!
		return true;
	}
	
	/* Failed */
	return false;
#endif /* __REMOOD_DEDICATED */
}

/* VS_FontPDCRemove() -- Removes loaded character data */
static void VS_FontPDCRemove(const struct WL_WADFile_s* a_WAD)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return;
	
	/*** STANDARD CLIENT ***/
#else
	size_t i, f;
	V_LocalFontStuff_t* LocalStuff;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return;
	
	/* Check */
	if (!a_WAD)
		return;
	
	/* Get local stuff */
	LocalStuff = WL_GetPrivateData(a_WAD, WLDK_VFONTS, NULL);
	
	// Failed?
	if (!LocalStuff)
		return;
	
	/* Clear each things */
	for (f = 0; f < NUMVIDEOFONTS; f++)
		if (LocalStuff->CGroups[f])
		{
			// Clear sub tables
			for (i = 0; i < 256; i++)
				if (LocalStuff->CGroups[f][i])
				{
					Z_Free(LocalStuff->CGroups[f][i]);
					LocalStuff->CGroups[f][i] = NULL;
				}
		
			// Clear master table
			Z_Free(LocalStuff->CGroups[f]);
			LocalStuff->CGroups[f] = NULL;
		}
#endif /* __REMOOD_DEDICATED */
}

// GhostlyDeath <December 30, 2011> -- Probably pointless!
#define VVIRTUALSPACECHAR ((V_UniChar_t*)-1)

/* VS_AddCharacter() -- Adds character to table */
static V_UniChar_t* VS_AddCharacter(const bool_t a_Local, V_LocalFontStuff_t* const a_LocalStuff, const WL_WADEntry_t* const a_Entry, const uint16_t a_Hex, const VideoFont_t a_Font, const uint16_t a_GAlias, const uint16_t* const a_GBuilder, V_UniChar_t* const a_CharP)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return NULL;
	
	/*** STANDARD CLIENT ***/
#else
	uint16_t Master, Slave;
	V_UniChar_t* CharP;
	int32_t w, h, xo, yo;
	V_ColorPal_t VCP;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return NULL;
	
	/* Local */
	// Add it to passed structure
	if (a_Local)
	{
		// Check
		if (!a_LocalStuff || !a_Entry || !a_Hex)
			return NULL;
		
		// Check to see if the local stuff has the super structure
		if (!a_LocalStuff->CGroups[a_Font])
			a_LocalStuff->CGroups[a_Font] = Z_Malloc(sizeof(*a_LocalStuff->CGroups[a_Font]) * 256, PU_FONTCHARS, (void**)&a_LocalStuff->CGroups[a_Font]);
		
		// Find the master and slave index
		Master = (a_Hex & 0xFF00) >> 8;
		Slave = (a_Hex & 0xFF);
		
		// Check if the master is allocated for this group
		if (!a_LocalStuff->CGroups[a_Font][Master])
			a_LocalStuff->CGroups[a_Font][Master] = Z_Malloc(sizeof(*a_LocalStuff->CGroups[a_Font][Master]) * 256, PU_FONTCHARS, (void**)&a_LocalStuff->CGroups[a_Font][Master]);
		
		// Get slave
		CharP = &a_LocalStuff->CGroups[a_Font][Master][Slave];
		
		// If it is unlinked, link it in
		if (!CharP->Linked)
		{
			// This will be the first chain?
			if (!a_LocalStuff->FirstLink[a_Font])
				a_LocalStuff->FirstLink[a_Font] = CharP;
			
			// A simple linked list here
			else
			{
				CharP->Chain = a_LocalStuff->FirstLink[a_Font];
				a_LocalStuff->FirstLink[a_Font] = CharP;
			}
			
			// Set as linked
			CharP->Linked = true;
		}
		
		// Fill slave with info
		CharP->Char = a_Hex;
		V_ExtWCharToMB(CharP->Char, CharP->MB);
		
		// Load initial image (but only if it is not virtual)
		if (CharP != VVIRTUALSPACECHAR)
		{	
			// Which translation?
				// Doom
			if (a_Font == VFONT_STATUSBARSMALL || a_Font == VFONT_BOOMHUD ||
				a_Font == VFONT_OEM || a_Font == VFONT_SMALL_DOOM || a_Font == VFONT_LARGE_DOOM || a_Font == VFONT_TERMINUS)
				VCP = VCP_DOOM;
				
				// Heretic
			else if (a_Font == VFONT_SMALL_HERETIC || a_Font == VFONT_LARGE_HERETIC)
				VCP = VCP_HERETIC;
			else
				VCP = VCP_NONE;
			
			// Load it
			CharP->Entry = a_Entry;
			CharP->Image = V_ImageLoadE(CharP->Entry, VCP);
		
			// Obtain size of character (include offsets)
			V_ImageSizePos(CharP->Image, &w, &h, &xo, &yo);
			CharP->Size[0] = w - xo;
			CharP->Size[1] = h - yo;
		}
		
		// Return the freshly created character
		return CharP;
	}
	
	/* Global */
	// Add it to global link
	else
	{
		/* Check */
		if (!a_CharP)
			return NULL;
			
		// See if the super structure needs creation
		if (!l_CGroups[a_Font])
			l_CGroups[a_Font] = Z_Malloc(sizeof(*l_CGroups[a_Font]) * 256, PU_FONTCHARS, NULL);
		
		// Find the master and slave index
		Master = (a_Hex & 0xFF00) >> 8;
		Slave = (a_Hex & 0xFF);
		
		// Allocate master if needed
		if (!l_CGroups[a_Font][Master])
			l_CGroups[a_Font][Master] = Z_Malloc(sizeof(*l_CGroups[a_Font][Master]) * 256, PU_FONTCHARS, NULL);
		
		// If not set, increment count
		if (!l_CGroups[a_Font][Master][Slave])
			l_LocalFonts[a_Font].NumChars++;
		
		// Set slave pointer to the character being added
		l_CGroups[a_Font][Master][Slave] = a_CharP;
		
		// Increase Reference Count
		a_CharP->RefCount++;
			
		// Return self on success
		return a_CharP;
	}
	
	/* Failure */
	return NULL;
#endif /* __REMOOD_DEDICATED */
}

/* VS_FontPDCCreate() -- Creates a character database from characters inside of a WAD */
static bool_t VS_FontPDCCreate(const struct WL_WADFile_s* const a_WAD, const uint32_t a_Key, void** const a_DataPtr, size_t* const a_SizePtr, WL_RemoveFunc_t* const a_RemoveFuncPtr)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return false;
	
	/*** STANDARD CLIENT ***/
#else
	size_t i;
	const WL_WADEntry_t* Entry;
	V_LocalFontStuff_t* FontStuff;
	uint16_t Hex;
	VideoFont_t Font;
	V_FontNameRule_t Rule;
	V_UniChar_t* FreshChar;
	V_UniChar_t* Rover;
	int32_t TotalWidth, TotalHeight;

	/* Not for dedicated server */
	if (g_DedicatedServer)
		return false;
	
	/* Check */
	if (!a_WAD)
		return false;
	
	/* Create private data */
	*a_SizePtr = sizeof(*FontStuff);
	FontStuff = *a_DataPtr = Z_Malloc(*a_SizePtr, PU_FONTCHARS, NULL);
	
	/* Go through every single entry in a WAD */
	for (i = 0; i < a_WAD->NumEntries; i++)
	{
		// Obtain entry link
		Entry = &a_WAD->Entries[i];
		
		// Auto-detect if this is a named character along with a correct hex
		// mapping. It can also auto choose the font too!
		if (!VS_DetectByName(Entry->Name, &Hex, &Font, &Rule))
			continue;
		
		// It is a valid character, now slap it into the local table
		if (!(FreshChar = VS_AddCharacter(true, FontStuff, Entry, Hex, Font, 0, NULL, NULL)))
			continue;
	}
	
	/* Build Dynamic Info */
	for (i = 0; i < NUMVIDEOFONTS; i++)
	{
		// Get first chain here
		Rover = FontStuff->FirstLink[i];
		
		// Clear
		TotalWidth = TotalHeight = 0;
		
		// While the chain is linked
		while (Rover)
		{
			// Add to totals
			TotalWidth += Rover->Size[0];
			TotalHeight += Rover->Size[1];
			FontStuff->DynInfo[i].NumChars++;
			
			// Go to next link
			Rover = Rover->Chain;
		}
		
		// Determine average character size
		// TODO: Instead of average, how about the most size of each? As in
		// if there are 99 size 7 fonts but there is 1 size 9999 font the char
		// height would be 106.92 (ouch). This would be for the height only
		// though.
		if (FontStuff->DynInfo[i].NumChars > 0)
		{
			FontStuff->DynInfo[i].CharWidth = TotalWidth / FontStuff->DynInfo[i].NumChars;
			FontStuff->DynInfo[i].CharHeight = TotalHeight / FontStuff->DynInfo[i].NumChars;
		}
	}
	
	/* Success! */
	return true;
#endif /* __REMOOD_DEDICATED */
}

/* VS_FontOCCB() -- Maps all characters from WADs into tables */
// This function is very important.
// I could make it so I can do without this, but that would not be good at all.
// Not only would it be slow, but it would be really slow. This essentially
// takes the virtual WAD stuff and plumps it ontop of each other into a
// composite form. Then after building a raw composite, aliases are created and
// mapped to characters. Then when character want to be drawn, it then uses this
// composited table.
static bool_t VS_FontOCCB(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return false;
	
	/*** STANDARD CLIENT ***/
#else

#define MAXHEIGHTCHECKERS	16
	size_t i, j, f;
	int32_t Count;
	V_UniChar_t*** FGroup;
	V_UniChar_t* Rover;
	const WL_WADFile_t* WADRover;
	V_LocalFontStuff_t* FontStuff;
	int32_t tW, tH, tT;
	uint16_t v, Master, Slave, MasterC, SlaveC;
	WL_WADEntry_t* UTTT;
	uint8_t u8;
	uint8_t* p;
	uint8_t* UTData;
	uint16_t a, b;
	
	struct
	{
		uint32_t Height;
		uint32_t Count;
	} Heights[MAXHEIGHTCHECKERS];
	
	static const struct
	{
		uint16_t Start;
		uint16_t Len;
	} VSpaceChars[] =
	{
		{'\r', 1},
		{'\n', 1},
		{'\t', 1},
		{'\a', 1},
		{'\b', 1},
		{' ', 1},
		{0x00A0U, 1},
		{0x2000U, 15},
		{0x2028U, 7},
		{0x205FU, 4},
		{0x206AU, 5},
		{0xFFF9U, 2},
		{0, 0},
	};
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return false;
	
	/* Clear old groups */
	for (f = 0; f < NUMVIDEOFONTS; f++)
	{
		// Clear character count
		l_LocalFonts[f].NumChars = 0;
		
		// Clear font groups
		if (l_CGroups[f])
		{
			// Clear master groups
			for (i = 0; i < 256; i++)
				if (l_CGroups[f][i])
				{
#if 0
					// Clear slave groups
					for (j = 0; j < 256; j++)
						if (l_CGroups[f][i][j])
							if (!l_CGroups[f][i][j]->DoNotFree)
							{
								// Still being used?
								if (--l_CGroups[f][i][j]->RefCount)
									;	// Don't free it
								
								// Otherwise destroy it
								else
								{
									if (l_CGroups[f][i][j]->Image)
										V_ImageDestroy(l_CGroups[f][i][j]->Image);
									l_CGroups[f][i][j]->Image = NULL;
								
									Z_Free(l_CGroups[f][i][j]);
									l_CGroups[f][i][j] = NULL;
								}
							}
#endif
					
					Z_Free(l_CGroups[f][i]);
					l_CGroups[f][i] = NULL;
				}
			
			Z_Free(l_CGroups[f]);
			l_CGroups[f] = NULL;
		}
	}
	
	/* Start constructing new groups */
	// For every WAD
	Count = 0;
	WADRover = NULL;
	while ((WADRover = WL_IterateVWAD(WADRover, true)))
	{
		// Get the WAD's private data
		FontStuff = WL_GetPrivateData(WADRover, WLDK_VFONTS, NULL);
		
		// Failed?
		if (!FontStuff)
			continue;
		
		// for every font
		for (f = 0; f < NUMVIDEOFONTS; f++)
		{
			// Obtain rover for this font
			Rover = FontStuff->FirstLink[f];
			
			// Go through it adding each character
			while (Rover)
			{
				// Add this character
				if (VS_AddCharacter(false, FontStuff, Rover->Entry, Rover->Char, f, 0, 0, Rover))
					Count++;
				
				// Go to next
				Rover = Rover->Chain;
			}
		}
	}
	
	// Load virtual aliases and builders from RMD_UTTT
	UTTT = WL_FindEntry(NULL, 0, "RMD_UTTT");
	
	if (!UTTT)
	{
		// Debug message
		if (devparm)
			CONL_PrintF("VS_FontOCCB: RMD_UTTT missing, mappings will be incomplete.\n");
	}
	else
	{
		// Allocate data
		UTData = Z_Malloc(UTTT->Size + 5, PU_FONTCHARS, NULL);
		WL_ReadData(UTTT, 0, UTData, UTTT->Size);
		
		// Get base pointer
		p = UTData;
		
		// Constantly read data
		while (*p != 0)
		{
			// Based on the first byte
			switch (*p)
			{
					// Case mapping
				case 1:
					// Increment p
					p++;
					
					// Zero indicates an end
					do
					{
						// Read lower case character (target)
						a = *(p++);
						a |= ((uint16_t)(*(p++))) << 8;
					
						// End?
						if (!a)
							break;
					
						// Read capitalized character (source)
						b = *(p++);
						b |= ((uint16_t)(*(p++))) << 8;
					
						// End?
						if (!b)
							break;
						
						// Determine master and slaves
						Master = (a & 0xFF) >> 8;
						Slave = (a & 0xFF);
						MasterC = (b & 0xFF) >> 8;
						SlaveC = (b & 0xFF);
					
						// Now process for every font
						for (f = 0; f < NUMVIDEOFONTS; f++)
							if (l_CGroups[f])
							{
								// Check if the capitalized character does not exist
								if (!l_CGroups[f][MasterC])
									continue;
								if (!l_CGroups[f][MasterC][SlaveC])
									continue;
							
								// Check if the lowercase character already exists
								if (l_CGroups[f][Master])
									if (l_CGroups[f][Master][Slave])
										continue;
							
								// Duplicate the character
								if (VS_AddCharacter(false, NULL, NULL, a, f, 0, 0, l_CGroups[f][MasterC][SlaveC]))
									Count++;
							}
					} while (*p != 0);
					break;
					
					// End of Info
				default:
					// Oops!
					if (*p != 0)
						p++;
					break;
			}
		}
		
		// Free data, no longer needed
		Z_Free(UTData);
	}
	
	// Delete all pre-existing space characters
	memset(l_VSpace, 0, sizeof(l_VSpace));
	
	// Add virtual space characters
	for (f = 0; f < NUMVIDEOFONTS; f++)
	{
		// Add every virtual space
		for (i = 0; VSpaceChars[i].Start; i++)
			for (j = 0; j < VSpaceChars[i].Len; i++)
			{
				// Get actual char
				v = VSpaceChars[i].Start + j;
				
				// Get master and slave
				Master = (v & 0xFF00) >> 8;
				Slave = (v & 0xFF);
				
				// Determine if it exists already
				if (l_CGroups[f])
					if (l_CGroups[f][Master])
						if (l_CGroups[f][Master][Slave])
							continue;
				
				// It does not, so add it
				l_VSpace[f].DoNotFree = true;
				if (VS_AddCharacter(false, NULL, NULL, v, f, 0, 0, &l_VSpace[f]))
					Count++;
			}
	}
	
	// Scan through each font and determine the best width and height
	for (f = 0; f < NUMVIDEOFONTS; f++)
	{
		// Hash group identifier
		l_LocalFonts[f].ScriptHash = Z_Hash(l_LocalFonts[f].ScriptName);
		
		// Clear
		tW = tH = tT = 0;
		memset(Heights, 0, sizeof(Heights));
		
		// Determine the best character size for each group
		// But only for characters that have images
		if (l_LocalFonts[f].NumChars)
			if (l_CGroups[f])
				for (i = 0; i < 256; i++)
					if (l_CGroups[f][i])
						for (j = 0; j < 256; j++)
							if (l_CGroups[f][i][j])
								if (l_CGroups[f][i][j]->Image)
								{
									// Average Based
									tT++;
									tW += l_CGroups[f][i][j]->Size[0];
									tH += l_CGroups[f][i][j]->Size[1];
									
									// Clone height
									b = l_CGroups[f][i][j]->Size[1];
									
									// Only add if it has height
									if (b)
									{
										// Put in heights listing
										for (v = 0; v < MAXHEIGHTCHECKERS; v++)
											if (Heights[v].Height == 0 || Heights[v].Height == b)
												break;
										
										// OK?
										if (v < MAXHEIGHTCHECKERS)
										{
											// Setup and increment count
											Heights[v].Height = b;
											Heights[v].Count++;
										}
									}
								}
		
		// Average it out
		if (tT > 0)
		{
			l_LocalFonts[f].CharWidth = tW / tT;
			l_LocalFonts[f].CharHeight = tH / tT;
		}
		
		// Find the height with the most population
		for (a = 0, v = 0; v < MAXHEIGHTCHECKERS; v++)
			if (Heights[v].Count > Heights[a].Count)
				a = v;	// Now the best!
		
		// Set the height if there is one (would be zero otherwise anyway
		if (Heights[a].Height)		// so this if is pointless)
			l_LocalFonts[f].CharHeight = Heights[a].Height;
		
		// Space is this average width and height
		l_VSpace[f].Size[0] = l_LocalFonts[f].CharWidth;
		l_VSpace[f].Size[1] = l_LocalFonts[f].CharHeight;
		
		// Determine the unknown link
		l_UnknownLink[f] = 0;
		if (l_CGroups[f])
		{
			// First attempt 0xFFFD (UTF replacement character)
			if (l_CGroups[f][0xFF])
				l_UnknownLink[f] = l_CGroups[f][0xFF][0xFD];
			
			// Next attempt 0x0000 (Standard replacement character)
			if (!l_UnknownLink[f])
				if (l_CGroups[f][0x00])
					l_UnknownLink[f] = l_CGroups[f][0x00][0x00];
				
			// Next attempt 0x007F (Backspace character)
			if (!l_UnknownLink[f])
				if (l_CGroups[f][0x00])
					l_UnknownLink[f] = l_CGroups[f][0x00][0x7F];
			
			// If still not found, try any (drawable) character in the first set
			if (!l_UnknownLink[f])
				if (l_CGroups[f][0x00])
					for (j = 0; j < 256; j++)
						if (l_CGroups[f][0x00][j])
							if (l_CGroups[f][0x00][j]->Image)
							{
								l_UnknownLink[f] = l_CGroups[f][0x00][j];
								break;
							}	
		}
	}
	
	// Create local mapping (most of them are 1:1)
	for (f = 0; f < NUMVIDEOFONTS; f++)
		l_FontRemap[f] = f;
	
	// However based on the current detected game mode...
		// Heretic?
	if (g_CoreGame == COREGAME_HERETIC)
	{
		l_FontRemap[VFONT_SMALL] = VFONT_SMALL_HERETIC;
		l_FontRemap[VFONT_LARGE] = VFONT_LARGE_HERETIC;
	}
	
		// Always assume Doom then
	else
	{
		l_FontRemap[VFONT_SMALL] = VFONT_SMALL_DOOM;
		l_FontRemap[VFONT_LARGE] = VFONT_LARGE_DOOM;
	}
	
	// Debug
	if (devparm)
		CONL_PrintF("VS_FontOCCB: Mapped %i characters.\n", Count);
	
	/* Success! */
	return true;
#undef MAXHEIGHTCHECKERS
#endif /* __REMOOD_DEDICATED */
}

/* V_MapGraphicalCharacters() -- Initializes WL Hooks */
void V_MapGraphicalCharacters(void)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return;
	
	/*** STANDARD CLIENT ***/
#else

	/* Not for dedicated server */
	if (g_DedicatedServer)
		return;
	
	/* Hook WL handlers */
	// Register PDC
	if (!WL_RegisterPDC(WLDK_VFONTS, WLDPO_VFONTS, VS_FontPDCCreate, VS_FontPDCRemove))
		I_Error("V_MapGraphicalCharacters: Failed to register PDC.");
	
	// Register OCCB (this builds a composite)
	if (!WL_RegisterOCCB(VS_FontOCCB, WLDCO_VFONTS))
		I_Error("V_MapGraphicalCharacters: Failed to register OCCB.");
#endif /* __REMOOD_DEDICATED */
}

/* V_ExtMBToWChar() -- Converts MB to wchar_t */
uint16_t V_ExtMBToWChar(const char* MBChar, size_t* const BSkip)
{
	size_t n;
	uint16_t Feed = 0, Safe, Temp;
	
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
		
		// Get safe character
		Safe = *MBChar & 0x7F;
		
		// Special '{' Sequence?
		if (n > 1 && *MBChar == '{')
		{
			// Get next character
			MBChar++;
			Feed = tolower(*MBChar);
			
			// Between 0-9 a-z?
			if ((Feed >= '0' && Feed <= '9') || (Feed >= 'a' && Feed <= 'z'))
			{
				if (Feed >= '0' && Feed <= '9')
					Feed = 0xF100U | (Feed - '0');
				else
					Feed = 0xF100U | (10 + (Feed - 'a'));
					
				if (BSkip)
					*BSkip = 2;
				
				// Super Extended Sequence? ("{x")
				if (Feed == 0xF121U && n > 3)
				{
					// Set to default
					Feed = 0xF100U;
					
					// Add first bit set
					MBChar++;
					Temp = tolower(*MBChar);
					
					if ((Temp >= '0' && Temp <= '9') || (Temp >= 'a' && Temp <= 'f'))
					{
						if (Temp >= '0' && Temp <= '9')
							Feed |= ((Temp - '0') & 0xFU) << 4;
						else if (Temp >= 'a' && Temp <= 'f')
							Feed |= (((Temp - 'a') + 10U) & 0xFU) << 4;
						else
							Feed |= 0xF1FFU;
						
						// Skip more characters
						if (BSkip)
							*BSkip += 1;
							
						// Add second bit set
						MBChar++;
						Temp = tolower(*MBChar);
						
						if ((Temp >= '0' && Temp <= '9') || (Temp >= 'a' && Temp <= 'f'))
						{
							if (Temp >= '0' && Temp <= '9')
								Feed |= ((Temp - '0') & 0xFU);
							else if (Temp >= 'a' && Temp <= 'f')
								Feed |= (((Temp - 'a') + 10U) & 0xFU);
							else
								Feed |= 0xF1FFU;
					
							// Skip more characters
							if (BSkip)
								*BSkip += 1;
						}
					}
				}
					
				return Feed;
			}
			
			// Is it another {? semi-scape
			else if (n > 2 && *MBChar == '{')
			{
				MBChar++;
				Feed = tolower(*MBChar);
				
				// Between 0-9 a-z?
				if ((Feed >= '0' && Feed <= '9') || (Feed >= 'a' && Feed <= 'z'))
				{
					// Skip to letter after
					if (BSkip)
						*BSkip = 2;
						
					// Return explicit '{'
					return '{';
				}
			}
		}
		// Normal
		return Safe;
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
	
	// Quad uint8_t (requires 32-bit wchar_t)
	else if (sizeof(uint16_t) >= 4 && (n == 4 || (*MBChar & 0xF8) == 0xF0))
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

/* V_ExtWCharToMB() -- Converts wchar_t to MB */
size_t V_ExtWCharToMB(const uint16_t a_WChar, char* const a_MB)
{
	unsigned char* MBx;
	uint16_t WChar = a_WChar;
	
	/* Check */
	if (!a_MB)
		return 0;
		
	/* Set */
	MBx = (unsigned char*)a_MB;
	
	/* Convert in steps */
	// Special (single char)
	if (WChar >= 0xF100U && WChar <= (0xF100U + 10 + ('z' - 'a')))
	{
		MBx[0] = '{';
		MBx[1] = (WChar - 0xF100U);
		MBx[2] = 0;
		
		// Normalize
		if (MBx[1] > 10)
			MBx[1] = (MBx[1] - 10) + 'a';
		else
			MBx[1] += '0';
		
		return 2;
	}
	
	// Super Extended Character (quad char)
	else if (WChar >= 0xF100U && WChar <= 0xF1FFU)
	{
		MBx[0] = '{';
		MBx[1] = 'x';
		MBx[2] = (WChar & 0xF0U) >> 4;
		MBx[3] = (WChar & 0x0FU);
		MBx[4] = 0;
		
		// Normalize
		if (MBx[2] > 10)
			MBx[2] = (MBx[2] - 10) + 'a';
		else
			MBx[2] += '0';
			
		if (MBx[3] > 10)
			MBx[3] = (MBx[3] - 10) + 'a';
		else
			MBx[3] += '0';
		
		return 4;
	}
	
	// Single byte
	else if (WChar >= 0x0000U && WChar <= 0x007FU)
	{
		MBx[0] = WChar & 0x7F;
		MBx[1] = 0;
		
		return 1;
	}
	
	// Double byte
	else if (WChar >= 0x0080U && WChar <= 0x07FFU)
	{
		MBx[0] = 0xC0 | (WChar >> 6);
		MBx[1] = 0x80 | (WChar & 0x3F);
		MBx[2] = 0;
		
		return 2;
	}
	
	// Triple byte
	else if (WChar >= 0x0800U && WChar <= 0xFFFFU)
	{
		MBx[0] = 0xE0 | (WChar >> 12);
		MBx[1] = 0x80 | ((WChar >> 6) & 0x3F);
		MBx[2] = 0x80 | (WChar & 0x3F);
		MBx[3] = 0;
		
		return 3;
	}
	
	// Quad-uint8_t (Requires 32-bit uint16_t)
	else if (sizeof(uint16_t) >= 4 && (WChar >= 0x010000U && WChar <= 0x10FFFFU))
	{
		MBx[0] = 0xF0 | (WChar >> 18);
		MBx[1] = 0x80 | ((WChar >> 12) & 0x3F);
		MBx[2] = 0x80 | ((WChar >> 6) & 0x3F);
		MBx[3] = 0x80 | (WChar & 0x3F);
		MBx[4] = 0;
		
		return 4;
	}
	
	/* Failed */
	return 0;
}

/* V_FontHeight() -- Returns the height of the font */
int V_FontHeight(const VideoFont_t a_Font)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return 0;
	
	/*** STANDARD CLIENT ***/
#else
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return 0;	
	
	/* Check */
	if (a_Font < 0 || a_Font >= NUMVIDEOFONTS)
		return 0;
	
	/* Return average height */
	return l_LocalFonts[l_FontRemap[a_Font]].CharHeight;
#endif /* __REMOOD_DEDICATED */
}

/* V_FontWidth() -- Returns the width of the font */
int V_FontWidth(const VideoFont_t a_Font)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return 0;
	
	/*** STANDARD CLIENT ***/
#else
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return 0;

	/* Check */
	if (a_Font < 0 || a_Font >= NUMVIDEOFONTS)
		return 0;
	
	/* Return average height */
	return l_LocalFonts[l_FontRemap[a_Font]].CharWidth;
#endif /* __REMOOD_DEDICATED */
}

/* VS_FindChar() -- Find character to map to */
static V_UniChar_t* VS_FindChar(const VideoFont_t a_Font, const char* const a_MBChar, size_t* const a_BSkip, uint16_t* const a_WChar)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return NULL;
	
	/*** STANDARD CLIENT ***/
#else
	uint16_t WChar, Master, Slave;
	VideoFont_t TrueFont;
	V_UniChar_t* VisSlave;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return NULL;
	
	/* Check */
	if (!a_MBChar || a_Font < 0 || a_Font >= NUMVIDEOFONTS)
		return NULL;
	
	/* Convert to integer character */
	WChar = V_ExtMBToWChar(a_MBChar, a_BSkip);
	
	// Copy char
	if (a_WChar)
		*a_WChar = WChar;
	
	/* Find in tables */
	// Get the true font mapping
	TrueFont = l_FontRemap[a_Font];
	
	// Get master and slave
	Master = (WChar & 0xFF00) >> 8;
	Slave = (WChar & 0xFF);
	
	// Check if the font exists
	if (l_CGroups[TrueFont])
		// Check if the master group exists
		if (l_CGroups[TrueFont][Master])
			// Check if the slave is set
			if ((VisSlave = l_CGroups[TrueFont][Master][Slave]))
				// If so, return it!
				return VisSlave;
	
	/* Otherwise return the "unknown" character */
	return l_UnknownLink[TrueFont];
#endif /* __REMOOD_DEDICATED */
}

/* V_DrawCharacterMB() -- Draws multi-byte character */
int V_DrawCharacterMB(const VideoFont_t a_Font, const uint32_t a_Options, const char* const a_MBChar, const int a_x, const int a_y, size_t* const a_BSkip, uint32_t* a_OptionsMod)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return 0;
	
	/*** STANDARD CLIENT ***/
#else
	V_UniChar_t* VisChar;
	uint32_t DrawFlags;
	uint16_t WChar;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return 0;
	
	/* Check */
	if (!a_MBChar)
		return 0;
	
	/* Locate visible character */
	WChar = 0;
	VisChar = VS_FindChar(a_Font, a_MBChar, a_BSkip, &WChar);
	
	/* Special control code? */
	if (WChar >= 0xF100U && WChar <= 0xF1FFU)
	{
		// If options cannot be modified, just stop
		if (!a_OptionsMod)
			return 0;
		
		// Return base
		WChar -= 0xF100U;
		
		// Color change?
		if (WChar >= 0 && WChar < 16)
		{
			*a_OptionsMod &= ~(VFO_COLORMASK | VFO_PCOLMASK | VFO_PCOLSET);	// Old Color
			*a_OptionsMod |= VFO_COLOR(WChar & 0xF);
		}
		
		// Transparency Change?
		else if (WChar >= 16 && WChar < (16 + NUMVEXTRANSPARENCIES))
		{
			*a_OptionsMod &= ~VFO_TRANSMASK;
			*a_OptionsMod |= VFO_TRANS((WChar - 16) & 0xF);
		}
		
		// Player Color
		else if (WChar >= 0x70 && WChar <= 0x7F)
		{
			*a_OptionsMod &= ~(VFO_COLORMASK | VFO_PCOLMASK | VFO_PCOLSET);	// Old Color
			*a_OptionsMod |= VFO_PCOL((WChar - 0x70U) & 0xFU) | VFO_PCOLSET;
		}
		
		// Underlined?
		else if (WChar == ('u' - 'a') + 10)
		{
			if (*a_OptionsMod & VFO_UNDERLINE)
				*a_OptionsMod &= ~VFO_UNDERLINE;
			else
				*a_OptionsMod |= VFO_UNDERLINE;
		}
		
		// Clear attributes
		else if (WChar == ('z' - 'a') + 10)
		{
			*a_OptionsMod &= ~(VFO_COLORMASK | VFO_PCOLMASK | VFO_PCOLSET | VFO_TRANSMASK | VFO_UNDERLINE);
		}
		
		// Always return 0, there is no space here
		return 0;
	}
	
	/* No character? */
	if (!VisChar)
		return 0;
	
	/* Draw character to the screen */
	// Determine flags
	DrawFlags = 0;
	
	// Color/Trans options
	DrawFlags |= VEX_COLORMAP((a_Options & VFO_COLORMASK) >> VFO_COLORSHIFT);
	DrawFlags |= VEX_TRANS((a_Options & VFO_TRANSMASK) >> VFO_TRANSSHIFT);
	
	if (a_Options & VFO_PCOLSET)
		DrawFlags |= VEX_COLORSET | VEX_PCOLOR((a_Options & VFO_PCOLMASK) >> VFO_PCOLSHIFT);
	
	// Scale options
	if (a_Options & VFO_NOSCALESTART)
		DrawFlags |= VEX_NOSCALESTART;
	if (a_Options & VFO_NOSCALEPATCH)
		DrawFlags |= VEX_NOSCALESCREEN;
	if (a_Options & VFO_NOFLOATSCALE)
		DrawFlags |= VEX_NOFLOATSCALE;
	if (a_Options & VFO_NOSCALELORES)
		DrawFlags |= VEX_NOSCALE160160;
	
	// Draw it
	V_ImageDraw(DrawFlags, VisChar->Image, a_x, a_y, NULL);
	
	/* Return character width */
	return VisChar->Size[0];
#endif /* __REMOOD_DEDICATED */
}

/* V_DrawCharacterA() -- Draws a single ASCII character */
int V_DrawCharacterA(const VideoFont_t a_Font, const uint32_t a_Options, const char a_Char, const int a_x, const int a_y)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return 0;
	
	/*** STANDARD CLIENT ***/
#else
	char MB[2];
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return 0;
	
	/* Convert to MB */
	MB[0] = a_Char;
	MB[1] = 0;
	
	/* Draw and return */
	return V_DrawCharacterMB(a_Font, a_Options, MB, a_x, a_y, NULL, NULL);
#endif /* __REMOOD_DEDICATED */
}

/* V_DrawStringA() -- Draws a string */
int V_DrawStringA(const VideoFont_t a_Font, const uint32_t a_Options, const char* const a_String, const int a_x, const int a_y)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return 0;
	
	/*** STANDARD CLIENT ***/
#else
	const char* c;
	size_t MBSkip;
	int32_t basex, x, y, Add;
	uint32_t Options;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return 0;
	
	/* Check */
	if (!a_String)	
		return 0;
	
	/* Initialize */
	x = a_x;
	y = a_y;
	Options = a_Options;
	
	// If patches are not being scaled but we are scaling the start
	if (!(Options & VFO_NOSCALESTART) && (Options & VFO_NOSCALEPATCH))
	{
		// Rescale to where scale start would scale
		x = FixedMul(x << FRACBITS, vid.fxdupx) >> FRACBITS;
		y = FixedMul(y << FRACBITS, vid.fxdupy) >> FRACBITS;
		
		// Remove start scale from the options (scaled here already)
		Options |= VFO_NOSCALESTART;
	}
	
	// Base x
	basex = x;
	
	/* Drawer loop */
	MBSkip = 0;
	for (c = a_String; *c; c += MBSkip)
	{
		// Check for \n (newline)
		if (*c == '\n')
		{
			// Return and shift over
			x = basex;
			y += V_FontHeight(a_Font);
		}
		
		// Otherwise a normal character
		else
		{
			// Send to screen
			Add = V_DrawCharacterMB(a_Font, Options, c, x, y, &MBSkip, &Options);
			
			// Scale additional coordinates
			if ((Options & VFO_NOSCALESTART) && !(Options & VFO_NOSCALEPATCH))
				Add = FixedMul(Add << FRACBITS, vid.fxdupx) >> FRACBITS;
			/*else if (!(a_Options & VFO_NOSCALESTART) && (a_Options & VFO_NOSCALEPATCH))
				Add = FixedDiv(Add << FRACBITS, vid.fxdupx) >> FRACBITS;*/
			
			// Add to x
			x += Add;
			
			// MBSkip failed?
			if (!MBSkip)
				MBSkip = 1;
		}
	}
	
	/* Return modifed position */
	return (x - basex);
#endif /* __REMOOD_DEDICATED */
}

/* V_StringDimensionsA() -- Returns dimensions of string */
void V_StringDimensionsA(const VideoFont_t a_Font, const uint32_t a_Options, const char* const a_String, int* const a_Width, int* const a_Height)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return;
	
	/*** STANDARD CLIENT ***/
#else
	const char* c;
	size_t MBSkip;
	int32_t basex, x, y, Add;
	uint32_t Options;
	int MaxX;
	uint16_t WChar;
	V_UniChar_t* VisChar;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return;
	
	/* Check */
	if (!a_String || (!a_Width && !a_Height))	
		return;
	
	/* Initialize */
	x = 0;
	y = V_FontHeight(a_Font);	// Font height always
	Options = a_Options;
	MaxX = 0;
	
	// Determine modifiers
	
	// Base x
	basex = x;
	
	/* Drawer loop */
	MBSkip = 0;
	for (c = a_String; *c; c += MBSkip)
	{
		// Check for \n (newline)
		if (*c == '\n')
		{
			// Return and shift over
			x = basex;
			y += V_FontHeight(a_Font);
		}
		
		// Otherwise a normal character
		else
		{
			// Find character 
			VisChar = VS_FindChar(a_Font, c, &MBSkip, &WChar);
			
			// Virtuals take up no room
			if (WChar >= 0xF100U && WChar <= 0xF1FFU)
				continue;
			
			// Non-drawn character
			if (!VisChar)
				continue;
			
			// Add to x the width of the font
			x += VisChar->Size[0];
			
			// Change max
			if (x > MaxX)
				MaxX = x;
			
			// MBSkip failed?
			if (!MBSkip)
				MBSkip = 1;
		}
	}
	
	/* Return dimensions */
	if (a_Width)
		*a_Width = MaxX;
	if (a_Height)
		*a_Height = y;
#endif /* __REMOOD_DEDICATED */
}

/* V_StringWidthA() -- Returns width of string */
int V_StringWidthA(const VideoFont_t a_Font, const uint32_t a_Options, const char* const a_String)
{
	int n = 0;
	
	/* Fairly simple */
	V_StringDimensionsA(a_Font, a_Options, a_String, &n, NULL);
	return n;
}

/* V_StringHeightA() -- Returns height of string */
int V_StringHeightA(const VideoFont_t a_Font, const uint32_t a_Options, const char* const a_String)
{
	int n = 0;
	
	/* Fairly simple */
	V_StringDimensionsA(a_Font, a_Options, a_String, NULL, &n);
	return n;
}

/*******************************************************************************
********************************************************************************
*******************************************************************************/

/*******************************************************************************
********************************************************************************
*******************************************************************************/

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

/*** STRUCTURES ***/

/* V_WLImageHolder_t -- Holds linked list for images, per WAD */
typedef struct V_WLImageHolder_s
{
	V_Image_t* ImageChain;						// Image changes for this WAD
	Z_HashTable_t* ImageHashes;					// Quickly find ASCII images
} V_WLImageHolder_t;

/*** LOCALS ***/

// Palette Mapping
static CoreGame_t l_VSGamePal;					// Current Game Palette
static uint8_t* l_VSPalMap[NUMVCOLORPALS];		// Color translation tables

static bool_t l_VSImageBooted = false;
static uint32_t l_VSImageAreaLimit = 0;			// Maximum picture size

/*** FUNCTIONS ***/

/* VS_GetPalMap() -- */
uint8_t* VS_GetPalMap(V_Image_t* const a_Image)
{
	uint16_t i;
	WL_WADEntry_t* Entry;
	V_ColorPal_t OneOne, TransA;
	const char* TransAName;
	
	/* 1:1 Mapping missing? */
	if (!l_VSPalMap[0])
	{
		l_VSPalMap[0] = Z_Malloc(sizeof(*l_VSPalMap[0]) * 256, PU_STATIC, NULL);
		for (i = 0; i < 256; i++)
			l_VSPalMap[0][i] = i;
	}
	
	/* Check */
	if (!a_Image || g_CoreGame == COREGAME_UNKNOWN)
		return l_VSPalMap[0];		// Return 1:1 Map
	
	/* Wrong Game? */
	if (l_VSGamePal != g_CoreGame)
	{
		// Free possibly
		for (i = 1; i < NUMVCOLORPALS; i++)
			if (l_VSPalMap[i])
			{
				Z_Free(l_VSPalMap[i]);
				l_VSPalMap[i] = NULL;
			}
		
		// Maps for Doom
		if (g_CoreGame == COREGAME_DOOM)
		{
			OneOne = VCP_DOOM;
			TransA = VCP_HERETIC;
			TransAName = "rmd_ph2d";
		}
		
		// Maps for Heretic
		else if (g_CoreGame == COREGAME_HERETIC)
		{
			OneOne = VCP_HERETIC;
			TransA = VCP_DOOM;
			TransAName = "rmd_pd2h";
		}
		
		// 1:1 Mapping
		l_VSPalMap[OneOne] = Z_Malloc(sizeof(*l_VSPalMap[OneOne]) * 256, PU_STATIC, NULL);
		for (i = 0; i < 256; i++)
			l_VSPalMap[OneOne][i] = i;
		
		// Translated (Palette A)
		Entry = WL_FindEntry(NULL, 0, TransAName);
		
		// If entry is not found, do 1:1 of it instead
		if (!Entry)
		{
			l_VSPalMap[TransA] = Z_Malloc(sizeof(*l_VSPalMap[TransA]) * 256, PU_STATIC, NULL);
			for (i = 0; i < 256; i++)
				l_VSPalMap[TransA][i] = i;
		}
		
		// Otherwise load up the lump
		else
		{
			l_VSPalMap[TransA] = Z_Malloc(sizeof(*l_VSPalMap[TransA]) * 256, PU_STATIC, NULL);
			WL_ReadData(Entry, 0, l_VSPalMap[TransA], 256);
		}
		
		// Set Game
		l_VSGamePal = g_CoreGame;
	}
	
	/* Return specified mapping from image */
	if (l_VSPalMap[a_Image->NativePal])
		return l_VSPalMap[a_Image->NativePal];
	return l_VSPalMap[0];
}

/* VS_HashImageCompare() -- Compares hash with image */
// a_A = const char* char
// a_B = V_Image_t* const
bool_t VS_HashImageCompare(void* const a_A, void* const a_B)
{
	const char* A;
	V_Image_t* B;
	
	/* Get */
	A = a_A;
	B = a_B;
	
	/* Compare */
	if (strcasecmp(A, B->Name) == 0)
		return true;	// a match!
	return false;
}

/* VS_WLImagePDC() -- Creates image containers */
static bool_t VS_WLImagePDC(const struct WL_WADFile_s* const a_WAD, const uint32_t a_Key, void** const a_DataPtr, size_t* const a_SizePtr, WL_RemoveFunc_t* const a_RemoveFuncPtr)
{
	V_WLImageHolder_t* HI;
	
	/* Allocate */
	*a_SizePtr = sizeof(V_WLImageHolder_t);
	HI = *a_DataPtr = Z_Malloc(*a_SizePtr, PU_STATIC, NULL);
	
	/* Create hash table there */
	HI->ImageHashes = Z_HashCreateTable(VS_HashImageCompare);
	
	return true;
}

/* VS_WLImagePDCRemove() -- Removes image containers */
static void VS_WLImagePDCRemove(const struct WL_WADFile_s* a_WAD)
{
	V_WLImageHolder_t* HI;
	
	/* Obtain */
	HI = WL_GetPrivateData(a_WAD, WLDK_VIMAGES, NULL);
	
	/* Check */
	if (!HI)
		return;
		
	/* Clean up after WAD */
	// Constant image killing
	while (HI->ImageChain)
		V_ImageDestroy(HI->ImageChain);
	HI->ImageChain = NULL;
	
	// Delete hash table
	Z_HashDeleteTable(HI->ImageHashes);
}

/* VS_InitialBoot() -- Initial startup */
static void VS_InitialBoot(void)
{
	/* Static image limit? */
	// FIXME TODO: cvar-ize
	l_VSImageAreaLimit = 512 * 512;
	
	/* Register data loader */
	if (!WL_RegisterPDC(WLDK_VIMAGES, WLDPO_VIMAGES, VS_WLImagePDC, VS_WLImagePDCRemove))
		I_Error("VS_InitialBoot: Failed to register PDC!");
	
	/* Booted up! */
	l_VSImageBooted = true;
}

/* V_ImageLoadE() -- Loads a specific entry as an image */
V_Image_t* V_ImageLoadE(const WL_WADEntry_t* const a_Entry, const V_ColorPal_t a_Pal)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return NULL;
	
	/*** STANDARD CLIENT ***/
#else

#define HEADERSIZE 12
	int16_t Header[HEADERSIZE];
	int32_t Conf[NUMVIMAGETYPES];
	uint32_t* Offs, TableEnd;
	size_t i, Best;
	V_Image_t* New;
	V_Image_t* Rover;
	V_WLImageHolder_t* HI;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return NULL;
	
	/* Booted? */
	if (!l_VSImageBooted)
		VS_InitialBoot();
	
	/* Check */
	if (!a_Entry)
		return NULL;
	
	/* Debug */
	//if (devparm)
	//	fprintf(stderr, "V_ImageLoadE: Loading \"%s\".\n", a_Entry->Name);
	
	/* Read header from entry */
	memset(Header, 0, sizeof(Header));
	WL_ReadData(a_Entry, 0, Header, sizeof(Header));
	
	// Byte swap for big endian
	for (i = 0; i < HEADERSIZE; i++)
		Header[i] = LittleSwapInt16(Header[i]);
	
	/* Attempt to determine which kind of image this is */
	// Thing is, there is no magic available to detect these kinds of things.
	// However, images could be detected based on validity.
	// pic_ts are   {w 0 h 0 data}
	// patch_ts are {w h x y cols}
	// Raw pictures such as flats have no descernable header
	// However, the good thing is that all the header info is signed, so any
	// negative value would invalidate it.
	// Like the IWAD finding stuff, this will be confidence based since it
	// usually is reliable in a chaotic world.
	memset(Conf, 0, sizeof(Conf));
	
	// Determine if the image is a pic_t (this is pretty much it)
	if ((Header[0] > 0 && Header[2] > 0) &&		// w/h > 0
		(Header[1] == 0 && Header[3] == 0))		// resv and zero
		Conf[VIT_PIC] += 75;
	
	// Determine if the image is a patch_t
	if (Header[0] > 0 && Header[1] > 0)			// w/h > 0
	{
		// Not as confident as a pic_t
		Conf[VIT_PATCH] += 50;
		
		// However, I can now look at the offset table to see if it is even
		// valid at all. For every valid offset +5, for every invalid -7.
		Offs = Z_Malloc(sizeof(*Offs) * Header[0], PU_STATIC, NULL);
		
		// Offsets that point at or below the actual place where data is stored
		// are invalid (would be garbage). So with this, any offset below this
		// point would be invalid.
		TableEnd = 8 + (2 * Header[0]);
		
		// Read offset table
		WL_ReadData(a_Entry, 8, Offs, sizeof(uint32_t) * Header[0]);
		
		// Byte swap values in table (for BE systems)
		for (i = 0; i < Header[0]; i++)
			Offs[i] = LittleSwapUInt32(Offs[i]);
		
		// Go through the table
		for (i = 0; i < Header[0]; i++)
			// Is it a valid offset?
			if (Offs[i] < a_Entry->Size && Offs[i] >= TableEnd)
				Conf[VIT_PATCH] += 5;
			else
				Conf[VIT_PATCH] -= 7;
		
		// Free offsets
		Z_Free(Offs);
	}
	
	// Determine if the image is a raw image
	// The only raw images that ever get accessed would be flats really (ouch)
	if (a_Entry->Size == 4096 || a_Entry->Size == 16384)
		Conf[VIT_RAW] += 25;	// Not really that confident
	
	/* Find the most confident match */
	for (Best = 0, i = 0; i < NUMVIMAGETYPES; i++)
		// Better than best?
		if (Conf[i] > Conf[Best])
			Best = i;
	
	/* Based on the most confident version... */
	// Create blank image
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	// Fill in common data
	New->NativeType = Best;
	New->wData = a_Entry;
	strncpy(New->Name, a_Entry->Name, MAXUIANAME);
	New->NameHash = Z_Hash(New->Name);
	New->NativePal = a_Pal;
	
	// Copy confidence
	for (i = 0; i < NUMVIMAGETYPES; i++)
		New->Conf[i] = Conf[i];
	
	// Fill in image data based on type
	switch (Best)
	{
			// patch_t
		case VIT_PATCH:
			New->Width = Header[0];
			New->Height = Header[1];
			New->Offset[0] = Header[2];
			New->Offset[1] = Header[3];
			break;
			
			// pic_t
		case VIT_PIC:
			New->Width = Header[0];
			New->Height = Header[2];
			break;
			
			// Raw
		case VIT_RAW:
		default:
			// Need the square root of entry size
			// I hate relying on floating point in Doom land
			New->Width = sqrt((double)a_Entry->Size);
			New->Height = New->Width;
			break;
	}
	
	// Pixel count is based on image width*height
	New->PixelCount = New->Width * New->Height;
	
	// Gigantic image?
	if (New->PixelCount >= l_VSImageAreaLimit)
	{
		// Make square, lowest of
		if (New->Width < New->Height)
			New->Height = New->Width;
		else if (New->Height < New->Width)
			New->Width = New->Height;
		
		// GhostlyDeath <March 28, 2012> -- I'd like to avoid <math.h> but I don't know how
		// to calc the square root int wise without the internet at my current disposal.
		New->Width = (int)(sqrt((double)New->Width));
		New->Height = New->Width;
		
		// Recalculate area
		New->PixelCount = New->Width * New->Height;
	}
	
	// Obtain power of two
	New->POTSize[0] = New->Width;
	New->POTSize[1] = New->Height;
	for (i = 0; i < 2; i++)
	{
		New->POTSize[i] = New->POTSize[i] - 1;
		New->POTSize[i] |= (New->POTSize[i] >> 1);
		New->POTSize[i] |= (New->POTSize[i] >> 2);
		New->POTSize[i] |= (New->POTSize[i] >> 4);
		New->POTSize[i] |= (New->POTSize[i] >> 8);
		New->POTSize[i] |= (New->POTSize[i] >> 16);
		New->POTSize[i] += 1;
	}
	
	// Link into chain for this WAD
	HI = WL_GetPrivateData(a_Entry->Owner, WLDK_VIMAGES, NULL);
	
	// This better always be true!
	if (HI)
	{
		// No chain exists
		if (!HI->ImageChain)
			HI->ImageChain = New;
		
		// A chain exists, link to end
		else
		{
			Rover = HI->ImageChain;
		
			// Find the end
			while (Rover && Rover->iNext)
				Rover = Rover->iNext;
			
			// Set next to new and link back
			Rover->iNext = New;
			New->iPrev = Rover;
		}
		
		// Link into hash table
		Z_HashAddEntry(HI->ImageHashes, New->NameHash, New);
	}
	
	// Failure? I hope not!
	else
		I_Error("V_ImageLoadE: Loaded an image entry who's owning WAD has no index.");
	
	/* Return the freshly created image */
	return New;
#undef HEADERSIZE

#endif /* __REMOOD_DEDICATED */
}

/* V_ImageFindA() -- Loads an image by name */
// Essentially a wrapper around V_ImageLoadE()
V_Image_t* V_ImageFindA(const char* const a_Name, const V_ColorPal_t a_Pal)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return NULL;
	
	/*** STANDARD CLIENT ***/
#else
	const WL_WADFile_t* Rover;
	V_WLImageHolder_t* WADImages;
	V_Image_t* FoundImage;
	const WL_WADEntry_t* Entry;
	uint32_t Hash;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return NULL;
	
	/* Booted? */
	if (!l_VSImageBooted)
		VS_InitialBoot();
	
	/* Check */
	if (!a_Name)
		return NULL;	
	
	/* Go through each WAD */
	Rover = NULL;
	Hash = Z_Hash(a_Name);	// Quicker if here
	while ((Rover = WL_IterateVWAD(Rover, false)))
	{
		// Get images from this WAD
		WADImages = WL_GetPrivateData(Rover, WLDK_VIMAGES, NULL);
		
		// Not found?
		if (!WADImages)
		{
			// Oops!
			I_Error("V_ImageFindA: WAD has no image index.");
			continue;
		}
		
		// Look in hashes
		FoundImage = Z_HashFindEntry(WADImages->ImageHashes, Hash, a_Name, false);
		
		// Found it? Then return it
		if (FoundImage)
			return FoundImage;
		
		// Otherwise if not found, it could be in this wad but it might not be
		Entry = WL_FindEntry(Rover, 0, a_Name);
		
		// Was an entry found? If so, try loading an image from it
		if (Entry)
			return V_ImageLoadE(Entry, a_Pal);
	}
	
	/* Failure */
	return NULL;
#endif /* __REMOOD_DEDICATED */
}

/* V_ImageDestroy() -- Destroys an image */
void V_ImageDestroy(V_Image_t* const a_Image)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return;
	
	/*** STANDARD CLIENT ***/
#else
	V_WLImageHolder_t* HI;
	uint32_t Hash;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return;

	/* Check */
	if (!a_Image)
		return;
	
	/* Remove from WAD */
	HI = WL_GetPrivateData(a_Image->wData->Owner, WLDK_VIMAGES, NULL);
	
	// Delete
	if (HI)
	{
		// Remove from hash table
		Z_HashDeleteEntry(HI->ImageHashes, a_Image->NameHash, a_Image->Name, false);
		
		// Check root reference
		if (HI->ImageChain == a_Image)
			HI->ImageChain = HI->ImageChain->iNext;
	}
	
	/* Remove link if needed */
	if (a_Image->iPrev)
		a_Image->iPrev->iNext = a_Image->iNext;
	if (a_Image->iNext)
		a_Image->iNext->iPrev = a_Image->iPrev;
	
	/* De-allocate raw type buffers */
	if (a_Image->dPatch)
		Z_Free(a_Image->dPatch);
	if (a_Image->dPic)
		Z_Free(a_Image->dPic);
	if (a_Image->dRaw)
		Z_Free(a_Image->dRaw);
#endif /* __REMOOD_DEDICATED */
}

/* V_ImageUsage() -- Prevents an image from being freed */
int32_t V_ImageUsage(V_Image_t* const a_Image, const bool_t a_Use)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return 0;
	
	/*** STANDARD CLIENT ***/
#else
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return 0;
	
	/* Check */
	if (!a_Image)
		return 0;

	return 0;
#endif /* __REMOOD_DEDICATED */
}

/* V_ImageSizePos() -- Return image info */
uint32_t V_ImageSizePos(V_Image_t* const a_Image, int32_t* const a_Width, int32_t* const a_Height, int32_t* const a_XOff, int32_t* const a_YOff)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return 0;
	
	/*** STANDARD CLIENT ***/
#else

	/* Not for dedicated server */
	if (g_DedicatedServer)
		return 0;	
	
	/* Check */
	if (!a_Image)
		return 0;
	
	/* Return stuff */
	if (a_Width)
		*a_Width = a_Image->Width;
	
	if (a_Height)
		*a_Height = a_Image->Height;
	
	if (a_XOff)
		*a_XOff = a_Image->Offset[0];
	
	if (a_YOff)
		*a_YOff = a_Image->Offset[1];
	
	/* Return pixel count */
	return a_Image->PixelCount;
#endif /* __REMOOD_DEDICATED */
}

/* V_ImageGetPatch() -- Load a patch */
// TODO: Enhance Security of this
const struct patch_s* V_ImageGetPatch(V_Image_t* const a_Image, size_t* const a_ByteSize)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return 0;
	
	/*** STANDARD CLIENT ***/
#else
	uint8_t* RawImage;
	uint8_t* RawData;
	size_t BaseSize, TotalSize, i, y, h;
	uint32_t Temp;
	
	uint8_t* id, *od, *EndD, *bid;
	uint8_t* COff, *CSize;
	uint8_t* BufA, *BufB;
	
	uint8_t* PalMap;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return NULL;
	
	/* Check */
	if (!a_Image)
		return NULL;
		
	/* Data already loaded? */
	if (a_Image->dPatch)
	{
		// Change to static
		Z_ChangeTag(a_Image->dPatch, PU_STATIC);
		
		// Return picture
		return a_Image->dPatch;
	}
	
	/* If the picture is natively a patch_t */
	// Just load it from the WAD data
	if (a_Image->NativeType == VIT_PATCH)
	{
		// Get palette mapping
		PalMap = VS_GetPalMap(a_Image);
		
		// Load the patch_t in an endian friendly and structure size friendly
		// way. So even if there is structure padding, it still works!
		// Allocate the base patch_t plus the base post data.
		BaseSize = sizeof(*a_Image->dPatch) + (sizeof(uint32_t) * a_Image->Width);
		TotalSize = BaseSize + a_Image->wData->Size;
		a_Image->dPatch = Z_Malloc(TotalSize, PU_STATIC, &a_Image->dPatch);
		a_Image->dPatchEnd = EndD = ((uint8_t*)a_Image->dPatch) + TotalSize;
		
		// Return size
		if (a_ByteSize)
			*a_ByteSize = TotalSize;
		
		// Read after base (so the lump data is offset sizeof(patch_t))
		WL_ReadData(a_Image->wData, 0, (void*)(((uintptr_t)a_Image->dPatch) + BaseSize), a_Image->wData->Size);
		
		// Initialize the base patch data
		a_Image->dPatch->width = a_Image->Width;
		a_Image->dPatch->height = a_Image->Height;
		a_Image->dPatch->leftoffset = a_Image->Offset[0];
		a_Image->dPatch->topoffset = a_Image->Offset[1];
		
		// Now read the offset table from the WAD
		for (i = 0; i < a_Image->Width; i++)
		{
			// Read offset from file
			WL_ReadData(a_Image->wData, 8 + (4 * i), &Temp, sizeof(Temp));
			
			// Correct endian
			Temp = LittleSwapUInt32(Temp);
			
			// Offset base
			Temp += BaseSize;
			
			// Set
			if (Temp >= 0 && Temp < TotalSize)
				a_Image->dPatch->columnofs[i] = Temp;
			else
				a_Image->dPatch->columnofs[i] = 0;
			
			// Correct any mistakes people and utilities made
			id = ((uint8_t*)a_Image->dPatch) + a_Image->dPatch->columnofs[i];
			
			// Way past the end?
			if (id < (((uint8_t*)a_Image->dPatch) + BaseSize) || id >= EndD)
			{
				a_Image->dPatch->columnofs[i] = 0;	// NULL it out
				continue;
			}
			
			// While not at end
			while (*id != 0xFF)
			{
				// Overflow?
				if (id >= EndD)
					break;
				
				// Obtain pointers to stuff
				COff = id++;	// Offset
				CSize = id++;	// Length
				id++;			// Holder
				
				// Go through the data
				h = *CSize + 1;	// 1 for that extra byte
				
				// Loop it
				for (y = 0; y < h; y++)
					// Exceeds data bounds?
					if (id >= EndD)	// Nuke the post!
					{
						*COff = 0xFF;
						y = 9999999;	// Go extremely off
						break;
					}
					
					// Contunue on
					else
					{
						// Re-map colors
						*id = PalMap[*id];
						
						// Go on
						id++;
					}
			}
		}
		
		// Return the freshly loaded image
		return a_Image->dPatch;
	} 
	
	/* If the image is not a patch_t */
	else
	{
		// Obtain the raw image, then postize it simply
		RawImage = V_ImageGetRaw(a_Image, a_ByteSize, 0);
		
		// Failed?
		if (!RawImage)
			return NULL;
		
		// Raw picture not needed anymore
		Z_ChangeTag(a_Image->dRaw, PU_CACHE);
	}
	
	/* Failure */
	return NULL;
#endif /* __REMOOD_DEDICATED */
}

/* V_ImageGetPic() -- Loads a pic_t */
const struct pic_s* V_ImageGetPic(V_Image_t* const a_Image, size_t* const a_ByteSize)
{	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return 0;
	
	/*** STANDARD CLIENT ***/
#else
	uint8_t* RawImage;
	size_t TotalSize;
	
	uint8_t* PalMap;
	uint32_t u;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return NULL;
	
	/* Check */
	if (!a_Image)
		return NULL;
	
	/* Data already loaded? */
	if (a_Image->dPic)
	{
		// Change to static
		Z_ChangeTag(a_Image->dPic, PU_STATIC);
		
		// Return picture
		return a_Image->dPic;
	}
	
	/* If the picture is natively a pic_t */
	// Just load it from the WAD data
	if (a_Image->NativeType == VIT_PIC)
	{
	} 
	
	/* If the image is not a pic_t */
	else
	{
		// Get palette mapping
		PalMap = VS_GetPalMap(a_Image);
		
		// Raw easily translate to a pic_t, so use that
		RawImage = V_ImageGetRaw(a_Image, a_ByteSize, 0);
		
		// Failed?
		if (!RawImage)
			return NULL;
		
		// Allocate pic_t structure
		TotalSize = sizeof(pic_t) + ((a_Image->PixelCount + 1) * sizeof(uint8_t));
		a_Image->dPic = Z_Malloc(TotalSize, PU_STATIC, (void**)&a_Image->dPic);
		
		// Return size
		if (a_ByteSize)
			*a_ByteSize = TotalSize;
		
		// Fill in structure
		((pic_t*)a_Image->dPic)->width = a_Image->Width;
		((pic_t*)a_Image->dPic)->height = a_Image->Height;
		
		// Copy raw image data as a whole (real easy!)
		memmove(&((pic_t*)a_Image->dPic)->data[0], RawImage, a_Image->PixelCount * sizeof(uint8_t));
		
		// Remap colors
		for (u = 0; u < a_Image->PixelCount; u++)
			((pic_t*)a_Image->dPic)->data[u] = PalMap[((pic_t*)a_Image->dPic)->data[u]];
		
		// Return the converted image
		return a_Image->dPic;
	}
	
	/* Failure */
	return NULL;
#endif /* __REMOOD_DEDICATED */
}

/* V_ImageGetRaw() -- Loads a raw image */
// Raw is the lowest common denominator
uint8_t* V_ImageGetRaw(V_Image_t* const a_Image, size_t* const a_ByteSize, const uint8_t a_Mask)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return 0;
	
	/*** STANDARD CLIENT ***/
#else
	patch_t* Patch;
	size_t i, x, dy;
	uint8_t* p;
	uint8_t Count;
	size_t TotalSize;
	uint8_t* PalMap;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return NULL;
	
	/* Check */
	if (!a_Image)
		return NULL;
	
	/* Data already loaded? */
	if (a_Image->dRaw)
	{
		// Change to static
		Z_ChangeTag(a_Image->dRaw, PU_STATIC);
		
		// Return raw image
		return a_Image->dRaw;
	}
	
	// Get palette mapping
	PalMap = VS_GetPalMap(a_Image);

	/* If the picture is natively a raw image */
	// Just load it from the WAD data (raw is the easiest)
	if (a_Image->NativeType == VIT_RAW)
	{
		// Allocate buffer
		TotalSize = (a_Image->PixelCount + 1) * sizeof(uint8_t);
		a_Image->dRaw = Z_Malloc(TotalSize, PU_STATIC, (void**)&a_Image->dRaw);
		
		// Return size
		if (a_ByteSize)
			*a_ByteSize = TotalSize;
		
		// Place Mask
		memset(a_Image->dRaw, a_Mask, TotalSize);
		
		// Load WAD data straight into buffer
		WL_ReadData(a_Image->wData, 0, a_Image->dRaw, a_Image->PixelCount * sizeof(uint8_t));
		
		// Re-Map Colors
		for (x = 0; x < a_Image->PixelCount; x++)
			a_Image->dRaw[x] = PalMap[a_Image->dRaw[x]];
	} 
	
	/* If the image is not a raw image */
	else
	{
		// If the native type is a pic_t, translation is easy
		if (a_Image->NativeType == VIT_PIC)
		{
			// Allocate buffer
			TotalSize = (a_Image->PixelCount + 1) * sizeof(uint8_t);
			a_Image->dRaw = Z_Malloc(TotalSize, PU_STATIC, (void**)&a_Image->dRaw);
			
			// Return size
			if (a_ByteSize)
				*a_ByteSize = TotalSize;
				
			// Place Mask
			memset(a_Image->dRaw, a_Mask, TotalSize);
		
			// Load WAD data straight into buffer with offset
			WL_ReadData(a_Image->wData, 8, a_Image->dRaw, a_Image->PixelCount * sizeof(uint8_t));
			
			// Re-Map Colors
			for (x = 0; x < a_Image->PixelCount; x++)
				a_Image->dRaw[x] = PalMap[a_Image->dRaw[x]];
		}
		
		// Otherwise if it is a patch_t, translation is required
		else if (a_Image->NativeType == VIT_PATCH)
		{
			// Load patch
			Patch = V_ImageGetPatch(a_Image, a_ByteSize);
			
			// Failed?
			if (!Patch)
				return NULL;
			
			// Allocate buffer to draw patch into
			TotalSize = (a_Image->PixelCount + 1) * sizeof(uint8_t);
			a_Image->dRaw = Z_Malloc(TotalSize, PU_STATIC, (void**)&a_Image->dRaw);
			
			// Return size
			if (a_ByteSize)
				*a_ByteSize = TotalSize;
				
			// Place Mask
			memset(a_Image->dRaw, a_Mask, TotalSize);
			
			// Draw into the raw buffer
			for (x = 0; x < a_Image->Width; x++)
			{
				// Obtain offset to data
				p = (void*)(((uintptr_t)Patch) + Patch->columnofs[x]);
				
				for (;;)
				{
					// Read offset
					dy = *(p++);
					
					// Is this the end of the column?
					if (dy == -1 || dy == 255)
						break;
				
					// Read number of pixels
					Count = *(p++);
				
					// Ignore the first pixel
					p++;
				
					// Draw into destination
					for (i = 0; i < Count; i++)
						a_Image->dRaw[(a_Image->Width * (dy++)) + x] = PalMap[*(p++)];
				
					// Ignore
					p++;
				}
			}
			
			// Patch is no longer needed
			Z_ChangeTag(a_Image->dPatch, PU_CACHE);
		}
	}
	
	/* Return the raw image */ 
	return a_Image->dRaw;
#endif /* __REMOOD_DEDICATED */
}

/* V_ImageDrawScaledIntoBuffer() -- Draws the image with specific scaling */
// This is the core implementation (all others call this one)
// TODO: Reimprove this function
//  * Make it more secure (prevent overflows)
//  * Make it faster in some respects (use memcpy when drawing raw images)
void V_ImageDrawScaledIntoBuffer(const uint32_t a_Flags, V_Image_t* const a_Image, const int32_t a_X, const int32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const fixed_t a_XScale, const fixed_t a_YScale, const uint8_t* const a_ExtraMap, uint8_t* const a_DestBuffer, const uint32_t a_DestPitch, const uint32_t a_DestWidth, const uint32_t a_DestHeight, const fixed_t a_VidXScaleX, const fixed_t a_VidXScaleY, const double a_VidFScaleX, const double a_VidFScaleY)
{	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return;
	
	/*** STANDARD CLIENT ***/
#else

//#define __REMOOD_OLDDRAW
#define __REMOOD_ONESHORT

	uint8_t* RawData;
	int32_t x, y, w, h, xx, yy, i, c, Mask, vvx, vvy, PixOff;
	
	int32_t sX, sY, tW, tY, px;
	uint8_t* dP, *dPend;
	uint8_t* sP;
	fixed_t XFrac, YFrac, sxX, sxY, xw, xh, dxY, ESXy, limxw, limxh, Off, BaseOff;
	uint8_t* ColorMap;
	uint8_t* ColorMapE;
	uint8_t* TransMap;
	uint8_t Pixel;
	
	patch_t* RawPatch;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return;
	
	/* Check */
	if (!a_Image)
		return;
	
	/* Find colormap */
	x = (a_Flags & VEX_COLORMAPMASK) >> VEX_COLORMAPSHIFT;
	
	// Check for illegal
	if (x < 0 || x >= NUMVEXCOLORS)
		x = 0;
	
	// Get it
	ColorMap = V_ReturnColormapPtr(x);
	
	/* Get extra map */
	if (a_ExtraMap)
		ColorMapE = a_ExtraMap;
	else if (a_Flags & VEX_COLORSET)
	{
		// Use player translation tables?
		Mask = ((a_Flags & VEX_COLORMASK) >> VEX_COLORSHIFT) & 0xFU;
		
		ColorMapE = V_ReturnColormapPtr(16 + Mask);
	}
	else
		ColorMapE = V_ReturnColormapPtr(VEX_MAP_NONE);
		
	/* Handle transparency */
	// Transparency OK
	if (transtables)
	{
		x = ((a_Flags & VEX_FILLTRANSMASK) >> VEX_FILLTRANSSHIFT);
		if (x >= NUMVEXTRANSPARENCIES)
			x = 0;
		
		// Set table to use
		if (x > 0)
			TransMap = transtables + (0x10000 * x);
		else
			TransMap = NULL;
	}
	
	// No tables loaded
	else
		TransMap = NULL;
	
	/* Determine the position to draw at */
	// Add image offsets
	x = a_X;
	y = a_Y;
	
	// Only if offsets are not ignored
	if (!(a_Flags & VEX_IGNOREOFFSETS))
	{
		x -= a_Image->Offset[0];
		y -= a_Image->Offset[1];
	}
	
	// Use detected size
	if (!a_Width && !a_Height)
	{
		w = a_Image->Width;
		h = a_Image->Height;
	}
	
	// Use explicit size
	else
	{
		w = a_Width;
		h = a_Height;
	}
	
	// fixed_t variants
	xw = w << FRACBITS;
	xh = h << FRACBITS;
	limxw = a_DestWidth << FRACBITS;
	limxh = a_DestHeight << FRACBITS;
	
	// Scale start position?
	if (!((a_Flags & VEX_NOSCALESTART) ||	// Don't scale at all
		((a_Flags & VEX_NOSCALE160160) && (a_DestWidth < BASEVIDWIDTH || a_DestHeight < BASEVIDHEIGHT))))	// Don't scale on low-res
	{
		// Fixed point scale
		if (a_Flags & VEX_NOFLOATSCALE)
		{
			x = FixedMul(x << FRACBITS, a_VidXScaleX) >> FRACBITS;
			y = FixedMul(y << FRACBITS, a_VidXScaleY) >> FRACBITS;
		}
		
		// Floating point scale
		else
		{
			x = (double)x * a_VidFScaleX;
			y = (double)y * a_VidFScaleY;
		}
	}
	
	/* Determine draw fraction */
	XFrac = FixedDiv(1 << FRACBITS, a_XScale);
	YFrac = FixedDiv(1 << FRACBITS, a_YScale);
	
	/* Limit drawing to the screen */
	// Off the top of the screen
	if (y < 0)
	{
		xh += y << FRACBITS;	// Add because it is negative
		y = 0;
	}
	
	// Off the left of the screen
	if (x < 0)
	{
		xw += x << FRACBITS;	// Add because it is negative
		x = 0;
	}
	
	// Off the bottom of the screen
	if ((y + (xh >> FRACBITS)) >= a_DestHeight)
		xh = (a_DestHeight << FRACBITS) - (y << FRACBITS);
	
	// Off the right of the screen
	if ((x + (xw >> FRACBITS)) >= a_DestWidth)
		xw = (a_DestWidth << FRACBITS) - (x << FRACBITS);
	
	// way off screen?
	if (y >= a_DestHeight)
		return;
	if (x >= a_DestWidth)
		return;
	
	/* Determine Draw Size */
	// On certain scales, there is a 1 pixel row copying loss
		// x
	tW = FixedMul(xw, a_XScale);
	tW = (tW + (tW & _FIXED_ROUND)) >> FRACBITS;
		// y
	tY = FixedMul(xh, a_YScale);
	tY = (tY + (tY & _FIXED_ROUND)) >> FRACBITS;
	
	/* If the image is a patch_t then draw it as a patch */
	// Since patches have "holes" for transparency
#if 1
	if (a_Image->NativeType == VIT_PATCH)
	{
		// Load the patch
		RawPatch = V_ImageGetPatch(a_Image, NULL);
		
		// Check
		if (!RawPatch)
			return;	// oops!
		
		// Draw column by column
		for (sxX = 0, xx = x; sxX < xw; sxX += XFrac, xx++)
		{
			// Get source
			sP = ((uint8_t*)RawPatch) + RawPatch->columnofs[sxX >> FRACBITS];
			
			// Past the end?
			if (sP >= a_Image->dPatchEnd)
				continue;
			
			// While there are no more columns
			while (*sP != 0xFF)
			{
#define __REMOOD_ALTPATCH
				// Get destination
				PixOff = *sP;
				BaseOff = FixedMul(((fixed_t)(PixOff)) << FRACBITS, a_YScale);
				Off = BaseOff >> FRACBITS;
				dP = a_DestBuffer + (a_DestPitch * (y + ((fixed_t)Off))) + (xx);
				dPend = a_DestBuffer + (a_DestPitch * a_DestHeight);
				sP++;
				
				// Get count
				c = *sP;
				sP += 2;
				
				// Get Virtual Offset
				sxY = 0;
				for (sxY = 0, i = 0; i < (y + Off); i++)
					sxY += YFrac;
				
				// Draw for count
				if (TransMap)
				{
					for (i = 0; i < c && dP < dPend; i++)
						for (ESXy = (sxY & _FIXED_FRAC); ESXy < FRACUNIT; ESXy += YFrac, sxY += YFrac)
						{
							Pixel = sP[i];
							*(dP) = TransMap[(ColorMap[ColorMapE[Pixel]] << 8) + (*dP)];
							dP += a_DestPitch;
						}
				}
				else
				{
					for (i = 0; i < c && dP < dPend; i++)
						for (ESXy = (sxY & _FIXED_FRAC); ESXy < FRACUNIT; ESXy += YFrac, sxY += YFrac)
						{
							Pixel = sP[i];
							*(dP) = ColorMap[ColorMapE[Pixel]];
							dP += a_DestPitch;
						}
				}
				
				// Skip count and the last (unused)
				sP += c + 1;
			}
		}
		
		// No longer need image, so mark it as cache
		Z_ChangeTag(a_Image->dPatch, PU_CACHE);
	}
	
	/* Otherwise, treat it as a raw image */
	else
#endif
	{
		// Load data
		RawData = V_ImageGetRaw(a_Image, NULL, 0);
		
		// Check
		if (!RawData)
			return;	// oops!
		
		// Drawing a transparent image
		if (TransMap)
			for (sxY = 0, yy = y; sxY < xh; sxY += YFrac, yy++)
			{
				// Obtain source and destination pointers (for row base)
				sP = RawData + (w * (sxY >> FRACBITS));
				vvx = x;
				vvy = (a_DestPitch * yy);
				dP = a_DestBuffer + vvy + vvx;
			
				// Scaled row draw
				for (sxX = 0; sxX < xw && vvx < vid.width; sxX += XFrac, vvx++)
				{
					*(dP) = TransMap[(ColorMap[ColorMapE[sP[sxX >> FRACBITS]]] << 8) + (*dP)];
					dP++;	// Sequence point undefined
				}
			}
		
		// Drawing an opaque image (with scale)
		else
			for (sxY = 0, yy = y; sxY < xh;)
			{
				// Obtain source and destination pointers (for row base)
				px = 0;
				sP = RawData + (w * (sxY >> FRACBITS));
				vvx = x;
				vvy = (a_DestPitch * yy);
				dP = a_DestBuffer + vvy + vvx;
			
				// Scaled row draw
				for (sxX = 0; sxX < xw && vvx < vid.width; sxX += XFrac, vvx++)
					*(dP++) = ColorMap[ColorMapE[sP[sxX >> FRACBITS]]];
				
				if ((sxX - XFrac) & 0xFFFF)
					px = 1;
			
				// Copy first row to succeeding rows
				ESXy = ((sxY + (1 << FRACBITS)) & (~0xFFFF));
				sP = a_DestBuffer + (a_DestPitch * yy) + x;
				for (;sxY < ESXy ; sxY += YFrac, yy++)
				{
					dP = a_DestBuffer + (a_DestPitch * yy) + x;
					memmove(dP, sP, tW + px);
				}
			}
		
		// No longer need image, so mark it as cache
		Z_ChangeTag(a_Image->dRaw, PU_CACHE);
	}
#endif /* __REMOOD_DEDICATED */
}

/* V_ImageDrawScaled() -- Draws the image with specific scaling */
// This is the core implementation (all others call this one)
// TODO: Reimprove this function
//  * Make it more secure (prevent overflows)
//  * Make it faster in some respects (use memcpy when drawing raw images)
void V_ImageDrawScaled(const uint32_t a_Flags, V_Image_t* const a_Image, const int32_t a_X, const int32_t a_Y, const fixed_t a_XScale, const fixed_t a_YScale, const uint8_t* const a_ExtraMap)
{	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return;
	
	/*** STANDARD CLIENT ***/
#else
	
	/* Check */
	if (!a_Image)
		return;
	
	/* Complex Image Draw */
	VHW_HUDDrawImageComplex(a_Flags, a_Image, a_X, a_Y, a_Image->Width, a_Image->Height, a_XScale, a_YScale, a_ExtraMap);
#endif /* __REMOOD_DEDICATED */
}

/* V_ImageDrawTiled() -- Draws the image tiled (i.e. flat fill) */
void V_ImageDrawTiled(const uint32_t a_Flags, V_Image_t* const a_Image, const int32_t a_X, const int32_t a_Y, const uint32_t a_Width, const uint32_t a_Height, const uint8_t* const a_ExtraMap)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return;
	
	/*** STANDARD CLIENT ***/
#else
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return;

	/* Check */
	if (!a_Image)
		return;

#endif /* __REMOOD_DEDICATED */
}

/* V_ImageDraw() -- Draws an image */
void V_ImageDraw(const uint32_t a_Flags, V_Image_t* const a_Image, const int32_t a_X, const int32_t a_Y, const uint8_t* const a_ExtraMap)
{
	/*** DEDICATED SERVER ***/
#if defined(__REMOOD_DEDICATED)
	return;
	
	/*** STANDARD CLIENT ***/
#else
	fixed_t xScale, yScale;
	
	/* Not for dedicated server */
	if (g_DedicatedServer)
		return;
	
	/* Check */
	if (!a_Image)
		return;
	
	/* Based on flags, determine how to call V_DrawImageScaled() */
	// Don't scale to screen
	if ((a_Flags & VEX_NOSCALESCREEN) ||	// Don't scale at all
		((a_Flags & VEX_NOSCALE160160) && (vid.width < BASEVIDWIDTH || vid.height < BASEVIDHEIGHT)))	// Don't scale on low-res
	{
		xScale = 1 << FRACBITS;
		yScale = 1 << FRACBITS;
	}
	
	// Scale to screen
	else
	{
		// Fixed point scalar
		if (a_Flags & VEX_NOFLOATSCALE)
		{
			xScale = vid.fxdupx;
			yScale = vid.fxdupy;
		}
		
		// Floating point scalar
		else
		{
			xScale = FLOAT_TO_FIXED(vid.fdupx);
			yScale = FLOAT_TO_FIXED(vid.fdupy);
		}
	}
	
	/* That is basically everything */
	V_ImageDrawScaled(a_Flags, a_Image, a_X, a_Y, xScale, yScale, a_ExtraMap);
#endif /* __REMOOD_DEDICATED */
}

