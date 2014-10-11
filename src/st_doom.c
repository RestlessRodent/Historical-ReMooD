// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Doom Status Bar

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "st_doom.h"
#include "v_video.h"
#include "d_player.h"
#include "d_prof.h"





/*************
*** LOCALS ***
*************/

static bool_t l_BarInit;
static V_Image_t* l_ImgBackFace;

/****************
*** FUNCTIONS ***
****************/

/* ST_InitDoomBar() -- Initializes the status bar */
void ST_InitDoomBar(void)
{
	l_ImgBackFace = V_ImageFindA("STBAR", VCP_NONE);
}

#define IDFLAGS (VEX_NOSCALESTART)

/* ST_DoomModShape() -- Modifies shape of view bar */
void ST_DoomModShape(const size_t a_PID, int32_t* const a_X, int32_t* const a_Y, int32_t* const a_W, int32_t* const a_H, player_t* const a_ConsoleP, player_t* const a_DisplayP, D_Prof_t* a_Profile)
{
	fixed_t gsy;
	int32_t sbh;
	int32_t y, h;
	
	/* Initilialize Bar */
	if (!l_BarInit)
	{
		ST_InitDoomBar();
		l_BarInit = true;
	}
	
	/* Initialize sizing specification */
	// Screen real position
	y = a_Y;
	h = a_H;
	
	// Status bar is scaled to screen
	if (true)
		gsy = vid.fxdupy;
	
	// Status bar is not scaled to screen
	else
		gsy = 1 << FRACBITS;
	
	// Top of bar
	*a_H -= FixedMul(l_ImgBackFace->Height << FRACBITS, gsy) >> FRACBITS;
}

/* ST_DoomBar() -- Doom Status Bar */
void ST_DoomBar(const size_t a_PID, const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H, player_t* const a_ConsoleP, player_t* const a_DisplayP, D_Prof_t* a_Profile)
{
	fixed_t sx, sy, gsx, gsy;
	int32_t sbx, sbh, sbtop;
	int32_t x, y, w, h;
	
	/* Initilialize Bar */
	if (!l_BarInit)
	{
		ST_InitDoomBar();
		l_BarInit = true;
	}
	
	/* Initialize sizing specification */
	// Scale on normal screen (split screen scale)
	sx = FixedDiv(a_W << FRACBITS, 320 << FRACBITS);
	sy = FixedDiv(a_H << FRACBITS, 200 << FRACBITS);
	
	// Screen real position
	x = FixedMul(a_X << FRACBITS, vid.fxdupx) >> FRACBITS;
	y = FixedMul(a_Y << FRACBITS, vid.fxdupy) >> FRACBITS;
	w = FixedMul(a_W << FRACBITS, vid.fxdupx) >> FRACBITS;
	h = FixedMul(a_H << FRACBITS, vid.fxdupy) >> FRACBITS;
	
	// Status bar is scaled to screen
	if (true)
	{
		gsx = FixedMul(sx, vid.fxdupx);
		gsy = FixedMul(sy, vid.fxdupy);
		
		// Coordinates of status bar (when scaled)
		sbx = x;
	}
	
	// Status bar is not scaled to screen
	else
	{
		gsx = sx;
		gsy = sy;
		
		// Center bar on screen
		sbx = 0;
	}
	
	// Top of bar
	sbh = FixedMul(l_ImgBackFace->Height << FRACBITS, gsy) >> FRACBITS;
	sbtop = y + (h - sbh);
	
	/* Draw backface */
	V_ImageDrawScaled(IDFLAGS, l_ImgBackFace, sbx, sbtop, gsx, gsy, NULL);
	
	//void V_ImageDrawScaled(const uint32_t a_Flags, V_Image_t* const a_Image, const int32_t a_X, const int32_t a_Y, const fixed_t a_XScale, const fixed_t a_YScale, const uint8_t* const a_ExtraMap);
}

