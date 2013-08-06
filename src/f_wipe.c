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
//      Mission begin melt/wipe screen special effect.

/***************
*** INCLUDES ***
***************/

#include "f_wipe.h"
#include "screen.h"
#include "z_zone.h"
#include "v_video.h"
#include "r_draw.h"

//#include "doomdef.h"
//#include "m_random.h"
//#include "f_wipe.h"
//#include "i_system.h"
//#include "i_video.h"
//#include "v_video.h"
//#include "r_draw.h"				// transtable
//#include "p_pspr.h"				// tr_transxxx
//#include "z_zone.h"
//#include "vhw_wrap.h"

/*****************
*** PROTOTYPES ***
*****************/

/**************
*** GLOBALS ***
**************/

// when zero, stop the wipe
static bool_t go = 0;

static uint8_t* wipe_scr_start = NULL;
static uint8_t* wipe_scr_end = NULL;
static uint8_t* wipe_scr;

/************************
*** CONSOLE VARIABLES ***
************************/

/****************
*** FUNCTIONS ***
****************/

void wipe_shittyColMajorXform(short* array, int width, int height)
{
	int x;
	int y;
	short* dest;
	
	dest = (short*)Z_Malloc(width * height * 2, PU_STATIC, 0);
	
	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
			dest[x* height + y] = array[y * width + x];
			
	memcpy(array, dest, width * height * 2);
	
	Z_Free(dest);
	
}

static bool_t* BlindMarks;
static int BlindDone = 0;

/* wipe_initBlindsXForm() -- GhostlyDeath <June 4, 2010> -- Speckle Effect */
int wipe_initBlindsXForm(int width, int height, int ticks)
{
	BlindDone = 0;
	BlindMarks = Z_Malloc(sizeof(bool_t) * width, PU_STATIC, &BlindMarks);
	memcpy(wipe_scr, wipe_scr_start, width * height * scr_bpp);
	return 0;
}

/* wipe_doBlindsXForm() -- GhostlyDeath <June 4, 2010> -- Speckle Effect */
int wipe_doBlindsXForm(int width, int height, int ticks)
{
	int i, j;
	int x, y;
	
	for (i = M_Random() & 31; i < width; i += M_Random() & 63)
	{
		if (BlindMarks[i])
		{
			while (i < width && BlindMarks[i])
				i++;
				
			if (i == width)
				break;
		}
		// Get position
		x = i;					//(M_Random() * M_Random()) % width;
		
		// replace column
		for (j = 0; j < height; j++)
			wipe_scr[(j * width) + x] = wipe_scr_end[(j * width) + x];
		BlindDone++;
		BlindMarks[i] = true;
	}
	
	if (BlindDone >= width)
		return true;
		
	return false;
}

/* wipe_exitBlindsXForm() -- GhostlyDeath <June 4, 2010> -- Speckle Effect */
int wipe_exitBlindsXForm(int width, int height, int ticks)
{
	Z_Free(BlindMarks);
	return 0;
}

int wipe_initColorXForm(int width, int height, int ticks)
{
	memcpy(wipe_scr, wipe_scr_start, width * height * scr_bpp);
	return 0;
}

int wipe_doColorXForm(int width, int height, int ticks)
{
	bool_t changed;
	uint8_t* w;
	uint8_t* e;
	uint8_t newval;
	static int slowdown = 0;
	
	changed = false;
	
	while (ticks--)
	{
		// slowdown
		if (slowdown++)
		{
			slowdown = 0;
			return false;
		}
		
		w = wipe_scr;
		e = wipe_scr_end;
		
		while (w != wipe_scr + width * height)
		{
			if (*w != *e)
			{
				if ((newval = transtables[(*e << 8) + *w + ((tr_transmor - 1) << FF_TRANSSHIFT)]) == *w)
					if ((newval = transtables[(*e << 8) + *w + ((tr_transmed - 1) << FF_TRANSSHIFT)]) == *w)
						if ((newval = transtables[(*w << 8) + *e + ((tr_transmor - 1) << FF_TRANSSHIFT)]) == *w)
							newval = *e;
				*w = newval;
				changed = true;
			}
			w++;
			e++;
		}
	}
	return !changed;
}

int wipe_exitColorXForm(int width, int height, int ticks)
{
	return 0;
}

static int* y;

int wipe_initMelt(int width, int height, int ticks)
{
	int i, r;
	
	// copy start screen to main screen
	memcpy(wipe_scr, wipe_scr_start, width * height * scr_bpp);
	
	// makes this wipe faster (in theory)
	// to have stuff in column-major format
	wipe_shittyColMajorXform((short*)wipe_scr_start, width * scr_bpp / 2, height);
	wipe_shittyColMajorXform((short*)wipe_scr_end, width * scr_bpp / 2, height);
	
	// setup initial column positions
	// (y<0 => not ready to scroll yet)
	y = (int*)Z_Malloc(width * sizeof(int), PU_STATIC, 0);
	y[0] = -(M_Random() % 16);
	for (i = 1; i < width; i++)
	{
		r = (M_Random() % 3) - 1;
		y[i] = y[i - 1] + r;
		if (y[i] > 0)
			y[i] = 0;
		else if (y[i] == -16)
			y[i] = -15;
	}
	// dup for normal speed in high res
	for (i = 0; i < width; i++)
		y[i] *= vid.dupy;
		
	return 0;
}

int wipe_doMelt(int width, int height, int ticks)
{
	int i;
	int j;
	int dy;
	int idx;
	
	short* s;
	short* d;
	bool_t done = true;
	
	width = (width * scr_bpp) / 2;
	
	while (ticks--)
	{
		for (i = 0; i < width; i++)
		{
			if (y[i] < 0)
			{
				y[i]++;
				done = false;
			}
			else if (y[i] < height)
			{
				dy = (y[i] < 16) ? y[i] + 1 : 8;
				dy *= vid.dupy;
				if (y[i] + dy >= height)
					dy = height - y[i];
				s = &((short*)wipe_scr_end)[i * height + y[i]];
				d = &((short*)wipe_scr)[y[i] * width + i];
				idx = 0;
				for (j = dy; j; j--)
				{
					d[idx] = *(s++);
					idx += width;
				}
				y[i] += dy;
				s = &((short*)wipe_scr_start)[i * height];
				d = &((short*)wipe_scr)[y[i] * width + i];
				idx = 0;
				for (j = height - y[i]; j; j--)
				{
					d[idx] = *(s++);
					idx += width;
				}
				done = false;
			}
		}
	}
	
	return done;
	
}

int wipe_exitMelt(int width, int height, int ticks)
{
	if (y)
		Z_Free(y);
	y = NULL;
	return 0;
}

//  save the 'before' screen of the wipe (the one that melts down)
//
int wipe_StartScreen(int x, int y, int width, int height)
{
	// Not in OpenGL
	if (VHW_UseGLMode())
		return 0;
	
	// GhostlyDeath <June 4, 2010> -- Dynamically allocate wipe stuff
	wipe_scr_start = Z_Malloc(width * height * vid.bpp, PU_STATIC, &wipe_scr_start);
	//wipe_scr_start = screens[2];
	
	I_ReadScreen(wipe_scr_start);
	return 0;
}

//  save the 'after' screen of the wipe (the one that show behind the melt)
//
int wipe_EndScreen(int x, int y, int width, int height)
{
	// Not in OpenGL
	if (VHW_UseGLMode())
		return 0;
		
	// GhostlyDeath <June 4, 2010> -- Dynamically allocate wipe stuff
	wipe_scr_end = Z_Malloc(width * height * vid.bpp, PU_STATIC, &wipe_scr_end);
	//wipe_scr_end = screens[3];
	
	I_ReadScreen(wipe_scr_end);
	V_DrawBlock(x, y, 0, width, height, wipe_scr_start);	// restore start scr.
	return 0;
}

int wipe_ScreenWipe(int wipeno, int x, int y, int width, int height, int ticks)
{
	int rc;
	uint32_t Pitch;
	static int (*wipes[]) (int, int, int) =
	{
		wipe_initColorXForm, wipe_doColorXForm, wipe_exitColorXForm,
		wipe_initMelt, wipe_doMelt, wipe_exitMelt, wipe_initBlindsXForm, wipe_doBlindsXForm, wipe_exitBlindsXForm,
	};
	
	// Not in OpenGL
	if (VHW_UseGLMode())
		return 0;
	
	// GhostlyDeath <June 4, 2010> -- Force done?
	if (ticks >= 0)
	{
		//Fab: obsolete (we don't use dirty-rectangles type of refresh)
		
		// initial stuff
		if (!go)
		{
			go = 1;
			// wipe_scr = (uint8_t *) Z_Malloc(width*height*scr_bpp, PU_STATIC, 0); // DEBUG
			wipe_scr = screens[0];
			(*wipes[wipeno * 3]) (width, height, ticks);
		}
		// do a piece of wipe-in
		rc = (*wipes[wipeno * 3 + 1]) (width, height, ticks);
		//  V_DrawBlock(x, y, 0, width, height, wipe_scr); // DEBUG
		
		// final stuff
		if (rc)
		{
			go = 0;
			(*wipes[wipeno * 3 + 2]) (width, height, ticks);
			
			// GhostlyDeath <June 4, 2010> -- Free wipe buffers
			if (wipe_scr_start)
				Z_Free(wipe_scr_start);
			if (wipe_scr_end)
				Z_Free(wipe_scr_end);
			wipe_scr_start = NULL;
			wipe_scr_end = NULL;
		}
	}
	else
	{
		go = 0;
		(*wipes[wipeno * 3 + 2]) (width, height, -ticks);
		
		// GhostlyDeath <June 4, 2010> -- Free wipe buffers
		if (wipe_scr_start)
			Z_Free(wipe_scr_start);
		if (wipe_scr_end)
			Z_Free(wipe_scr_end);
		wipe_scr_start = NULL;
		wipe_scr_end = NULL;
		return true;
	}
	
	return !go;
}

