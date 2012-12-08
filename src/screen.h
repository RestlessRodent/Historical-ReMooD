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

#ifndef __SCREEN_H__
#define __SCREEN_H__


#include "m_fixed.h"

//added:26-01-98: quickhack for V_Init()... to be cleaned up
#define NUMSCREENS    4

// Size of statusbar.
#define ST_HEIGHT    32
#define ST_WIDTH     320
// heretic one's
#define SBARHEIGHT      42		// status bar height at bottom of screen

extern int stbarheight;			// when scaled

#define BASEVIDWIDTH    320		//NEVER CHANGE THIS! this is the original
#define BASEVIDHEIGHT   200		// resolution of the graphics.

// global video state
typedef struct viddef_s
{
	int modenum;				// vidmode num indexes videomodes list
	
	uint8_t* buffer;			// invisible screens buffer
	unsigned rowbytes;			// bytes per scanline of the VIDEO mode
	int width;					// PIXELS per scanline
	int height;
	union
	{
		// hurdler: don't need numpages for OpenGL, so we can
		// 15/10/99 use it for fullscreen / windowed mode
		int numpages;			// always 1, PAGE FLIPPING TODO!!!
		int windowed;			// windowed or fullscren mode ?
	} u;						//BP: name it please soo it work with gcc
	int recalc;					// if true, recalc vid-based stuff
	uint8_t* direct;			// linear frame buffer, or vga base mem.
	int dupx, dupy;				// scale 1,2,3 value for menus & overlays
	fixed_t fxdupx;				// GhostlyDeath <November 5, 2010> -- Fixed scaling
	fixed_t fxdupy;				// GhostlyDeath <November 5, 2010> -- Fixed scaling
	fixed_t fxdivx, fxdivy;		// GhostlyDeath <May 10, 2012> -- Inverse of fxdup
	float fdupx, fdupy;			// same as dupx,dupy but exact value when aspect ratio isn't 320/200
	int centerofs;				// centering for the scaled menu gfx
	int bpp;					// BYTES per pixel: 1=256color
	
	int baseratio;				// SoM: Used to get the correct value for lighting walls
	bool_t HWDblBuf;			// Hardware Double Buffering
} viddef_t;

#define VIDWIDTH    vid.width
#define VIDHEIGHT   vid.height

// internal additional info for vesa modes only
typedef struct
{
	int vesamode;				// vesa mode number plus LINEAR_MODE bit
	void* plinearmem;			// linear address of start of frame buffer
} vesa_extra_t;

// a video modes from the video modes list,
// note: video mode 0 is always standard VGA320x200.
typedef struct vmode_s
{

	struct vmode_s* pnext;
	char* name;
	unsigned int width;
	unsigned int height;
	unsigned int rowbytes;		//bytes per scanline
	unsigned int bytesperpixel;	// 1 for 256c
	int windowed;				// if true this is a windowed mode
	int numpages;
	vesa_extra_t* pextradata;	//vesa mode extra data
	int (*setmode) (viddef_t* lvid, struct vmode_s* pcurrentmode);
	int misc;					//misc for display driver (r_glide.dll etc)
} vmode_t;

// ---------------------------------------------
// color mode dependent drawer function pointers
// ---------------------------------------------

extern void (*skycolfunc) (void);
extern void (*colfunc) (void);
extern void (*basecolfunc) (void);
extern void (*fuzzcolfunc) (void);
extern void (*oldfuzzcolfunc) (void);
extern void (*transcolfunc) (void);
extern void (*shadecolfunc) (void);
extern void (*spanfunc) (void);
extern void (*basespanfunc) (void);
extern void (*transtransfunc) (void);	// SSNTails 11-11-2002

// ----------------
// screen variables
// ----------------
extern viddef_t vid;
extern int setmodeneeded;		// mode number to set if needed, or 0

extern bool_t fuzzymode;

extern int scr_bpp;
extern uint8_t* scr_borderpatch;	// patch used to fill the view borders

// quick fix for tall/short skies, depending on bytesperpixel
extern void (*skydrawerfunc[2]) (void);

// from vid_vesa.c : user config video mode decided at VID_Init ();
extern int vid_modenum;

// Change video mode, only at the start of a refresh.
void SCR_SetMode(void);

// Recalc screen size dependent stuff
void SCR_Recalc(void);

// Check parms once at startup
void SCR_CheckDefaultMode(void);

// Set the mode number which is saved in the config
void SCR_SetDefaultMode(void);

void SCR_Startup(void);

void SCR_ChangeFullscreen(void);

void SCR_ReclassBuffers(void);

#endif							//__SCREEN_H__
