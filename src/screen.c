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
//      handles multiple resolutions, 8bpp mode

#include "screen.h"
#include "r_defs.h"
#include "z_zone.h"
#include "console.h"
#include "i_video.h"
#include "r_sky.h"
#include "r_draw.h"
#include "r_plane.h"
#include "v_video.h"
#include "tables.h"
#include "m_argv.h"
#include "g_state.h"
#include "r_segs.h"
#include "i_system.h"
#include "dstrings.h"
#include "st_stuff.h"
#include "r_things.h"
#include "r_state.h"

void VID_PrepareModeList(void);	// from i_video_xshm.c

#ifdef DIRECTFULLSCREEN
// allow_fullscreen is set in VID_PrepareModeList
extern bool_t allow_fullscreen;
#endif

// --------------------------------------------
// assembly or c drawer routines for 8bpp/16bpp
// --------------------------------------------
void (*skycolfunc) (void);		//new sky column drawer draw posts >128 high
void (*colfunc) (void);			// standard column upto 128 high posts
void (*basecolfunc) (void);
void (*fuzzcolfunc) (void);		// standard fuzzy effect column drawer (Actually it's transparency)
void (*oldfuzzcolfunc) (void);	// Oldschool fuzzy
void (*transcolfunc) (void);	// translucent column drawer
void (*shadecolfunc) (void);	// smokie test..
void (*spanfunc) (void);		// span drawer, use a 64x64 tile
void (*basespanfunc) (void);	// default span func for color mode

// Tails 11-11-2002
void (*transtransfunc) (void);

// ------------------
// global video state
// ------------------
viddef_t vid;
int setmodeneeded;				//video mode change needed if > 0

// (the mode number to set + 1)

// TO DO!!! make it a console variable !!
bool_t fuzzymode = false;		// use original Doom fuzzy effect instead

// of translucency

// c_CVPVColorDepth --  Color Depth
const CONL_VarPossibleValue_t c_CVPVColorDepth[] =
{
	{1, "8-bit"},
	{2, "16-bit"},
	{4, "32-bit"},
	{I_VIDEOGLMODECONST, "OpenGL"},
	
	// End
	{1, "MINVAL"},
	{I_VIDEOGLMODECONST, "MAXVAL"},
	{0, NULL},
};

// scr_width -- Width of screen
CONL_StaticVar_t l_SCRWidth =
{
	CLVT_INTEGER, c_CVPVPositive, CLVF_SAVE,
	"scr_width", DSTR_CVHINT_SCRWIDTH, CLVVT_STRING, "640",
	NULL
};

// scr_height -- Height of screen
CONL_StaticVar_t l_SCRHeight =
{
	CLVT_INTEGER, c_CVPVPositive, CLVF_SAVE,
	"scr_height", DSTR_CVHINT_SCRHEIGHT, CLVVT_STRING, "480",
	NULL
};

// scr_depth -- Depth of screen
CONL_StaticVar_t l_SCRDepth =
{
	CLVT_INTEGER, c_CVPVColorDepth, CLVF_SAVE,
	"scr_depth", DSTR_CVHINT_SCRDEPTH, CLVVT_STRING, "1",
	NULL
};

// scr_fullscreen -- Fullscreen video mode
CONL_StaticVar_t l_SCRFullScreen =
{
	CLVT_INTEGER, c_CVPVBoolean, CLVF_SAVE,
	"scr_fullscreen", DSTR_CVHINT_SCRFULLSCREEN, CLVVT_STRING, "false",
	NULL
};

// =========================================================================
//                           SCREEN VARIABLES
// =========================================================================

int scr_bpp;					// current video mode bytes per pixel
uint8_t* scr_borderpatch;		// flat used to fill the reduced view borders

// set at ST_Init ()

// =========================================================================

//  Set the video mode right now,
//  the video mode change is delayed until the start of the next refresh
//  by setting the setmodeneeded to a value >0
//
int VID_SetMode(int modenum);

//  Short and Tall sky drawer, for the current color mode
void (*skydrawerfunc[2]) (void);

void SCR_SetMode(void)
{
	if (g_DedicatedServer)
		return;
		
	if (!graphics_started)
		return;
		
	if (!setmodeneeded)
		return;					//should never happen
		
	if (devparm)
		CONL_PrintF("SCR_SetMode: Changing resolution.\n");
		
	VID_SetMode(--setmodeneeded);
	
	V_SetPalette(0);
	//CONL_PrintF ("SCR_SetMode : vid.bpp is %d\n", vid.bpp);
	
	//
	//  setup the right draw routines for either 8bpp or 16bpp
	//
	//if (vid.bpp == 1)
	{
		colfunc = basecolfunc = R_DrawColumn_8;
		
		fuzzcolfunc = (fuzzymode) ? R_DrawFuzzColumn_8 : R_DrawTranslucentColumn_8;
		oldfuzzcolfunc = R_DrawFuzzColumn_8;
		transcolfunc = R_DrawTranslatedColumn_8;
		shadecolfunc = R_DrawShadeColumn_8;	//R_DrawColumn_8;
		spanfunc = basespanfunc = R_DrawSpan_8;
		
		// SSNTails 11-11-2002
		transtransfunc = R_DrawTranslatedTranslucentColumn_8;
		
		// FIXME: quick fix
		skydrawerfunc[0] = R_DrawColumn_8;	//old skies
		skydrawerfunc[1] = R_DrawSkyColumn_8;	//tall sky
	}
	//else
		//I_Error("unknown bytes per pixel mode %d\n", vid.bpp);
		
	// set the apprpriate drawer for the sky (tall or short)
	
	setmodeneeded = 0;
}

//  do some initial settings for the game loading screen
//
void SCR_Startup(void)
{
	if (g_DedicatedServer)
		return;
	
	CONL_VarRegister(&l_SCRWidth);
	CONL_VarRegister(&l_SCRHeight);
	CONL_VarRegister(&l_SCRDepth);
	CONL_VarRegister(&l_SCRFullScreen);
		
	if (!graphics_started)
		return;
		
	vid.modenum = 0;
	
	// GhostlyDeath <November 5, 2010> -- Fixed point scale
	vid.fxdupx = FixedDiv(vid.width << FRACBITS, BASEVIDWIDTH << FRACBITS);
	vid.fxdupy = FixedDiv(vid.height << FRACBITS, BASEVIDHEIGHT << FRACBITS);
	vid.fxdivx = FixedDiv(1 << FRACBITS, vid.fxdupx);
	vid.fxdivy = FixedDiv(1 << FRACBITS, vid.fxdupy);
	
	// GhostlyDeath <November 5, 2010> -- Use fixed point scale
	vid.fdupx = FIXED_TO_FLOAT(vid.fxdupx);
	vid.fdupy = FIXED_TO_FLOAT(vid.fxdupy);
	vid.dupx = vid.fxdupx >> FRACBITS;
	vid.dupy = vid.fxdupy >> FRACBITS;
	
	vid.baseratio = FRACUNIT;
	
	scaledofs = 0;
	vid.centerofs = 0;
	
	V_Init();
	
	V_SetPalette(0);
}

//added:24-01-98:
//
// Called at new frame, if the video mode has changed
//
extern int dc_drawymove;
void SCR_Recalc(void)
{
	if (g_DedicatedServer)
		return;
		
	if (!graphics_started)
		return;
		
	if (devparm)
		CONL_PrintF("SCR_Recalc: Recalculating Screen\n");
		
	// bytes per pixel quick access
	scr_bpp = vid.bpp;
	
	//added:18-02-98: scale 1,2,3 times in x and y the patches for the
	//                menus and overlays... calculated once and for all
	//                used by routines in v_video.c
	// GhostlyDeath <November 5, 2010> -- Fixed point scale
	vid.fxdupx = FixedDiv(vid.width << FRACBITS, BASEVIDWIDTH << FRACBITS);
	vid.fxdupy = FixedDiv(vid.height << FRACBITS, BASEVIDHEIGHT << FRACBITS);
	vid.fxdivx = FixedDiv(1 << FRACBITS, vid.fxdupx);
	vid.fxdivy = FixedDiv(1 << FRACBITS, vid.fxdupy);
	
	// GhostlyDeath <November 5, 2010> -- Use fixed point scale
	vid.fdupx = FIXED_TO_FLOAT(vid.fxdupx);
	vid.fdupy = FIXED_TO_FLOAT(vid.fxdupy);
	vid.dupx = vid.fxdupx >> FRACBITS;
	vid.dupy = vid.fxdupy >> FRACBITS;
	vid.baseratio = FixedDiv(vid.height << FRACBITS, BASEVIDHEIGHT << FRACBITS);
	
	//added:18-02-98: calculate centering offset for the scaled menu
	scaledofs = 0;				//see v_video.c
	vid.centerofs = (((vid.height % BASEVIDHEIGHT) / 2) * vid.width) + (vid.width % BASEVIDWIDTH) / 2;
	
	SCR_ReclassBuffers();
	
	// fuzzoffsets for the 'spectre' effect,... this is a quick hack for
	// compatibility, because I don't use it anymore since transparency
	// looks much better.
	R_RecalcFuzzOffsets();
	
	// r_plane stuff : visplanes, openings, floorclip, ceilingclip, spanstart,
	//                 spanstop, yslope, distscale, cachedheight, cacheddistance,
	//                 cachedxstep, cachedystep
	//              -> allocated at the maximum vidsize, static.
	
	// r_main : xtoviewangle, allocated at the maximum size.
	// r_things : negonearray, screenheightarray allocated max. size.
	
	// set the screen[x] ptrs on the new vidbuffers
	V_Init();
	
	// scr_viewsize doesn't change, neither detailLevel, but the pixels
	// per screenblock is different now, since we've changed resolution.
	R_SetViewSize();			//just set setsizeneeded true now ..
	
	// vid.recalc lasts only for the next refresh...
//    CON_ToggleOff ();  // make sure con height is right for new screen height

	dc_drawymove = vid.width;
}

// Check for screen cmd-line parms : to force a resolution.
//
// Set the video mode to set at the 1st display loop (setmodeneeded)
//
int VID_GetModeForSize(int w, int h);	//vid_vesa.c

void SCR_CheckDefaultMode(void)
{
	int p;
	int scr_forcex;				// resolution asked from the cmd-line
	int scr_forcey;
	
	if (g_DedicatedServer)
		return;
		
	if (!graphics_started)
		return;
		
	// 0 means not set at the cmd-line
	scr_forcex = 0;
	scr_forcey = 0;
	
	p = M_CheckParm("-width");
	if (p && p < myargc - 1)
		scr_forcex = atoi(myargv[p + 1]);
		
	p = M_CheckParm("-height");
	if (p && p < myargc - 1)
		scr_forcey = atoi(myargv[p + 1]);
	
	if (M_CheckParm("-gl") || M_CheckParm("-opengl"))
	{
		// Attempt GL Mode
		CONL_VarSetIntByName("scr_depth", I_VIDEOGLMODECONST);
	}
	
	else
	{
		// Otherwise, based by command line
		p = M_CheckParm("-bpp");
		
		if (!p)
			p = M_CheckParm("-depth");
		
		if (p && p < myargc - 1)
		{
			p = atoi(myargv[p + 1]);
			
			// Illegal?
			if (p > 32)
				p = 32;
			else if (p < 8)
				p = 8;
			
			// Force these 3 values only
			if (p != 8 && p != 16 && p != 32)
				p = 32;	// User is dumb, force 32-bit
			
			// Set as the divide of that
			CONL_VarSetIntByName("scr_depth", p / 8);
		}
	}
		
	if (scr_forcex && scr_forcey)
	{
		CONL_PrintF("Using resolution: %d x %d\n", scr_forcex, scr_forcey);
		// returns -1 if not found, thus will be 0 (no mode change) if not found
		setmodeneeded = VID_GetModeForSize(scr_forcex, scr_forcey) + 1;
		//if (scr_forcex!=BASEVIDWIDTH || scr_forcey!=BASEVIDHEIGHT)
	}
	else
	{
		CONL_PrintF("Default resolution: %d x %d\n", l_SCRWidth.Value->Int, l_SCRHeight.Value->Int);
		// see note above
		setmodeneeded = VID_GetModeForSize(l_SCRWidth.Value->Int, l_SCRHeight.Value->Int) + 1;
	}
}

//added:03-02-98: sets the modenum as the new default video mode to be saved
//                in the config file
void SCR_SetDefaultMode(void)
{
	// remember the default screen size
	//CV_SetValue(&cv_scr_width, vid.width);
	//CV_SetValue(&cv_scr_height, vid.height);
	//CV_SetValue(&cv_scr_depth, vid.bpp * 8);
	
	if (M_CheckParm("-window") || M_CheckParm("-w") || M_CheckParm("-win") || M_CheckParm("-windowed") || M_CheckParm("-nofullscreen"))
		CONL_VarSetStrByName("scr_fullscreen", "no");
	else if (M_CheckParm("-fullscreen"))
		CONL_VarSetStrByName("scr_fullscreen", "yes");
}

// Change fullscreen on/off according to cv_fullscreen

void SCR_ChangeFullscreen(void)
{
#ifdef DIRECTFULLSCREEN
	int modenum;
	
	// allow_fullscreen is set by VID_PrepareModeList
	// it is used to prevent switching to fullscreen during startup
	if (!allow_fullscreen)
		return;
		
	if (graphics_started)
	{
		VID_PrepareModeList();
		modenum = VID_GetModeForSize(cv_scr_width.value, cv_scr_height.value);
		setmodeneeded = ++modenum;
	}
	return;
#endif
}

void SCR_ReclassBuffers(void)
{
	int i;
	visplane_t* pl;
	
	if (devparm)
		CONL_PrintF("SCR_ReclassBuffers: Reclassing buffers.\n");
		
	/* IF The old stuff exists... */
#define SCRFREE(x) if (x) { Z_Free(x); x = NULL; }
	// Width
	SCRFREE(xtoviewangle);
	SCRFREE(columnofs);
	SCRFREE(floorclip);
	SCRFREE(ceilingclip);
	SCRFREE(frontscale);
	SCRFREE(distscale);
	SCRFREE(last_ceilingclip);
	SCRFREE(last_floorclip);
	SCRFREE(negonearray);
	SCRFREE(screenheightarray);
	SCRFREE(ylookup);
	SCRFREE(ylookup1);
	SCRFREE(ylookup2);
	for (i = 0; i < MAXSPLITS; i++)
		SCRFREE(ylookup4[i]);
	SCRFREE(yslopetab);
	SCRFREE(cachedheight);
	SCRFREE(cacheddistance);
	SCRFREE(cachedxstep);
	SCRFREE(cachedystep);
	SCRFREE(spanstart);
	
	/* (Re)allocate */
	// Width
	xtoviewangle = Z_Malloc(sizeof(angle_t) * (vid.width + 1), PU_STATIC, NULL);
	columnofs = Z_Malloc(sizeof(*columnofs) * vid.width, PU_STATIC, NULL);
	floorclip = Z_Malloc(sizeof(*floorclip) * (vid.width + 1), PU_STATIC, NULL);
	ceilingclip = Z_Malloc(sizeof(*ceilingclip) * (vid.width + 1), PU_STATIC, NULL);
	frontscale = Z_Malloc(sizeof(*frontscale) * vid.width, PU_STATIC, NULL);
	distscale = Z_Malloc(sizeof(*distscale) * vid.width, PU_STATIC, NULL);
	last_ceilingclip = Z_Malloc(sizeof(*last_ceilingclip) * vid.width, PU_STATIC, NULL);
	last_floorclip = Z_Malloc(sizeof(*last_floorclip) * vid.width, PU_STATIC, NULL);
	negonearray = Z_Malloc(sizeof(*negonearray) * vid.width, PU_STATIC, NULL);
	screenheightarray = Z_Malloc(sizeof(*screenheightarray) * vid.width, PU_STATIC, NULL);
	solidsegs = Z_Malloc(sizeof(*solidsegs) * MAXSEGS, PU_STATIC, NULL);
	
	// Height
	ylookup = Z_Malloc(sizeof(uint8_t*) * vid.height, PU_STATIC, NULL);
	ylookup1 = Z_Malloc(sizeof(uint8_t*) * vid.height, PU_STATIC, NULL);
	ylookup2 = Z_Malloc(sizeof(uint8_t*) * vid.height, PU_STATIC, NULL);
	for (i = 0; i < MAXSPLITS; i++)
		ylookup4[i] = Z_Malloc(sizeof(uint8_t*) * vid.height, PU_STATIC, NULL);
	yslopetab = Z_Malloc(sizeof(fixed_t) * (vid.height * 4), PU_STATIC, NULL);
	
	cachedheight = Z_Malloc(sizeof(fixed_t) * vid.height, PU_STATIC, NULL);
	cacheddistance = Z_Malloc(sizeof(fixed_t) * vid.height, PU_STATIC, NULL);
	cachedxstep = Z_Malloc(sizeof(fixed_t) * vid.height, PU_STATIC, NULL);
	cachedystep = Z_Malloc(sizeof(fixed_t) * vid.height, PU_STATIC, NULL);
	spanstart = Z_Malloc(sizeof(int) * vid.height, PU_STATIC, NULL);
	
	/*** MORE ADVANCED STUFF ***/
	if (drawsegs)
		for (i = 0; i < maxdrawsegs; i++)
		{
			if (drawsegs[i].frontscale)
				SCRFREE(drawsegs[i].frontscale);
			drawsegs[i].frontscale = Z_Malloc(sizeof(fixed_t) * MAXSEGS, PU_STATIC, NULL);
		}
		
	if (visplane_ptr)
		for (i = 0; i < MAXVISPLANES; i++)
			if (visplane_ptr[i])
				for (pl = visplane_ptr[i]; pl; pl = pl->next)
				{
					if (pl->top)
					{
						Z_Free(&pl->top[-1]);
						pl->top = NULL;
					}
					
					pl->top = Z_Malloc(sizeof(unsigned short) * (vid.width + 2), PU_STATIC, NULL);
					pl->top = &pl->top[1];
					
					if (pl->bottom)
					{
						Z_Free(&pl->bottom[-1]);
						pl->bottom = NULL;
					}
					
					pl->bottom = Z_Malloc(sizeof(unsigned short) * (vid.width + 2), PU_STATIC, NULL);
					pl->bottom = &pl->bottom[-1];
				}
				
	for (i = 0; i < MAXFFLOORS; i++)
	{
		if (ffloor[i].f_clip)
			SCRFREE(ffloor[i].f_clip);
		ffloor[i].f_clip = Z_Malloc(sizeof(short) * vid.width, PU_STATIC, NULL);
		
		if (ffloor[i].c_clip)
			SCRFREE(ffloor[i].c_clip);
		ffloor[i].c_clip = Z_Malloc(sizeof(short) * vid.width, PU_STATIC, NULL);
	}
}
