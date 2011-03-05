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
// Copyright (C) 2011 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION: Converts messy large struct to WAD resource for cool usage

#include <stdio.h>
#include <stdint.h>

// Scope!
typedef struct ln_s
{
	uint16_t d;	// Dest
	uint16_t s;	// Source
	uint16_t l;	// Len
	
	uint16_t b_top;	// Top build character
	uint16_t b_bot;	// Bottom build character
} ln_t;

int main(int argc, char** argv)
{
	size_t x;
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
	
	// Big loop
	for (x = 0; x < sizeof(ln) / sizeof(ln_t); x++)
	{
		printf("S %04X %04X %03i %04X %04X E\n",
				ln[x].d,
				ln[x].s,
				ln[x].l,
				ln[x].b_top,
				ln[x].b_bot
			);
	}
}

