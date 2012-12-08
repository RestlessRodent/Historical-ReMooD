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
// Copyright (C) 2008-2013 GhostlyDeath (ghostlydeath@gmail.com)
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
//      Random number LUT.

#include "doomdef.h"
#include "doomtype.h"
#include "m_random.h"
#include "d_net.h"

//
// M_Random
// Returns a 0-255 number
//
uint8_t rndtable[256] =
{
	0, 8, 109, 220, 222, 241, 149, 107, 75, 248, 254, 140, 16, 66,
	74, 21, 211, 47, 80, 242, 154, 27, 205, 128, 161, 89, 77, 36,
	95, 110, 85, 48, 212, 140, 211, 249, 22, 79, 200, 50, 28, 188,
	52, 140, 202, 120, 68, 145, 62, 70, 184, 190, 91, 197, 152, 224,
	149, 104, 25, 178, 252, 182, 202, 182, 141, 197, 4, 81, 181, 242,
	145, 42, 39, 227, 156, 198, 225, 193, 219, 93, 122, 175, 249, 0,
	175, 143, 70, 239, 46, 246, 163, 53, 163, 109, 168, 135, 2, 235,
	25, 92, 20, 145, 138, 77, 69, 166, 78, 176, 173, 212, 166, 113,
	94, 161, 41, 50, 239, 49, 111, 164, 70, 60, 2, 37, 171, 75,
	136, 156, 11, 56, 42, 146, 138, 229, 73, 146, 77, 61, 98, 196,
	135, 106, 63, 197, 195, 86, 96, 203, 113, 101, 170, 247, 181, 113,
	80, 250, 108, 7, 255, 237, 129, 226, 79, 107, 112, 166, 103, 241,
	24, 223, 239, 120, 198, 58, 60, 82, 128, 3, 184, 66, 143, 224,
	145, 224, 81, 206, 163, 45, 63, 90, 168, 114, 59, 33, 159, 95,
	28, 139, 123, 98, 125, 196, 15, 70, 194, 253, 54, 14, 109, 226,
	71, 17, 161, 93, 186, 87, 244, 138, 20, 52, 123, 251, 26, 36,
	17, 46, 52, 231, 232, 76, 31, 221, 84, 37, 216, 165, 212, 106,
	197, 242, 98, 43, 39, 175, 254, 145, 190, 84, 118, 222, 187, 136,
	120, 163, 236, 249
};

static uint32_t rndindex = 0;
static uint8_t prndindex = 0;

#ifndef DEBUGRANDOM

// P_Random is used throughout all the p_xxx game code.
uint8_t P_Random()
{
#if 0
	static bool_t did;
	static FILE* f;
	if (!did)
	{
		f = fopen("pprnd", "wt");
		did = true;
	}
	fprintf(f, "%u\n", D_SyncNetMapTime());
	fflush(f);
#endif
	return rndtable[++prndindex];
}

// lot of code used P_Random()-P_Random() since C don't define
// evaluation order it is compiler depenent so this allow network play
// between different compilers
// GhostlyDeath <April 12, 2012> -- Whoever thought this, does not know C! Perhaps that is why it is so buggy!
int P_SignedRandom()
{
	int a, b;
	
	a = P_Random();
	b = P_Random();
	
	return a - b;
}

#else

#include "doomstat.h"

FILE* RandFile = NULL;

uint8_t P_Random2(char* a, int b)
{
	RandFile = fopen("prandom", "at");
	fprintf(RandFile, "P_Random at (gt = %i)\t%sp %d\n", gametic, a, b);
	fclose(RandFile);
	return rndtable[++prndindex];
}

int P_SignedRandom2(char* a, int b)
{
	int r;

	RandFile = fopen("prandom", "at");
	fprintf(RandFile, "P_Random at (gt = %i)\t%sp %d\n", gametic, a, b);
	fclose(RandFile);
	r = rndtable[++prndindex];
	return r - rndtable[++prndindex];
}

#endif

uint8_t M_Random(void)
{
	uint8_t RetVal;
	
	/* Use larger random table */
	if (g_RandomData)
	{
		// Get
		RetVal = g_RandomData[++rndindex];
	
		// Limit
		if (rndindex >= g_RandomDataSize)
			rndindex = 0;
	
		// Return
		return RetVal;
	}
	
	/* Use generic random */
	else
	{
		rndindex = (rndindex + 1) & 0xFF;
		return rndtable[rndindex];
	}
}

void M_ClearRandom(void)
{
	rndindex = prndindex = 0;
}

// for savegame and join in game
uint8_t P_GetRandIndex(void)
{
	return prndindex;
}

// load game
void P_SetRandIndex(uint8_t rindex)
{
	prndindex = rindex;
}
