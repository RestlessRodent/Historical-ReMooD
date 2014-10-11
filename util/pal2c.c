// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: ReMooD `deutex` Clone, for what ReMooD uses and bonus stuff

/***************
*** INCLUDES ***
***************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/****************
*** FUNCTIONS ***
****************/

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

/* main() -- Main entry point */
int main(int argc, char** argv)
{
	FILE* f;
	FILE* o;
	size_t i, j;
	uint8_t r, g, b, h, s, v;
	V_ColorEntry_t Conv;
	
	/* Check */
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s <playpal.pal>\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	/* Open Palette */
	f = fopen(argv[1], "rb");
	
	if (!f)
		return EXIT_FAILURE;
		
	/* Open output */
	o = fopen("playpal.c", "wt");
	
	/* Write typedef struct */
	/*fprintf(o, "#include <stdint.h>\n\n");
	fprintf(o,
			"typedef struct RGBHSV_s\n"
			"{\n"
			"\tuint8_t r;\n"
			"\tuint8_t g;\n"
			"\tuint8_t b;\n"
			"\tuint8_t h;\n"
			"\tuint8_t s;\n"
			"\tuint8_t v;\n"
			"} RGBHSV_t;\n\n"
		);*/
	
	/* Write output RGBs */
	fprintf(o, "static const V_ColorEntry_t c_Colors[256] =\n");
	fprintf(o, "{\n");
	
	fseek(f, 0, SEEK_SET);
	for (i = 0; i < 256; i++)
	{
		// Read 3 colors
		fread(&r, 1, 1, f);
		fread(&g, 1, 1, f);
		fread(&b, 1, 1, f);
		
		Conv.RGB.R = r;
		Conv.RGB.G = g;
		Conv.RGB.B = b;
		V_RGBtoHSV(Conv);
		h = Conv.HSV.H;
		s = Conv.HSV.S;
		v = Conv.HSV.V;
		
		// Write outpit
		fprintf(o, "\t{{%#04hhx, %#04hhx, %#04hhx}, {%#04hhx, %#04hhx, %#04hhx}},\n", r, g, b, h, s, v);
	}
	
	fprintf(o, "};\n");
	
	/* Close */
	fclose(f);
	fclose(o);
	
	/* Success! */
	return EXIT_SUCCESS;
}

