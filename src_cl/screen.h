// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.sourceforge.net/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           Demyx                  (demyx@endgameftw.com)
//                    Dragan                 (poliee13@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2009 The ReMooD Team..
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

#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "command.h"

#ifdef _WIN32
#include <windows.h>
#else
#define HWND    void*			//unused in DOS version
#endif

//added:26-01-98: quickhack for V_Init()... to be cleaned up
#define NUMSCREENS    4

// Size of statusbar.
#define ST_HEIGHT    32
#define ST_WIDTH     320
// heretic one's
#define SBARHEIGHT      42		// status bar height at bottom of screen

extern int stbarheight;			// when scaled

#define MAXVIDWIDTH 1280
#define MAXVIDHEIGHT 1024

#define BASEVIDWIDTH    320		//NEVER CHANGE THIS! this is the original
#define BASEVIDHEIGHT   200		// resolution of the graphics.

// global video state
typedef struct viddef_s
{
	int modenum;				// vidmode num indexes videomodes list

	byte *buffer;				// invisible screens buffer
	unsigned rowbytes;			// bytes per scanline of the VIDEO mode
	int width;					// PIXELS per scanline
	int height;
	union
	{							// hurdler: don't need numpages for OpenGL, so we can
		// 15/10/99 use it for fullscreen / windowed mode
		int numpages;			// always 1, PAGE FLIPPING TODO!!!
		int windowed;			// windowed or fullscren mode ?
	} u;						//BP: name it please soo it work with gcc
	int recalc;					// if true, recalc vid-based stuff
	byte *direct;				// linear frame buffer, or vga base mem.
	int dupx, dupy;				// scale 1,2,3 value for menus & overlays
	float fdupx, fdupy;			// same as dupx,dupy but exact value when aspect ratio isn't 320/200
	int centerofs;				// centering for the scaled menu gfx
	int bpp;					// BYTES per pixel: 1=256color, 2=highcolor

	int baseratio;				// SoM: Used to get the correct value for lighting walls

	// for Win32 version
	HWND WndParent;				// handle of the application's window
} viddef_t;
#define VIDWIDTH    vid.width
#define VIDHEIGHT   vid.height

// internal additional info for vesa modes only
typedef struct
{
	int vesamode;				// vesa mode number plus LINEAR_MODE bit
	void *plinearmem;			// linear address of start of frame buffer
} vesa_extra_t;
// a video modes from the video modes list,
// note: video mode 0 is always standard VGA320x200.
typedef struct vmode_s
{

	struct vmode_s *pnext;
	char *name;
	unsigned int width;
	unsigned int height;
	unsigned int rowbytes;		//bytes per scanline
	unsigned int bytesperpixel;	// 1 for 256c, 2 for highcolor
	int windowed;				// if true this is a windowed mode
	int numpages;
	vesa_extra_t *pextradata;	//vesa mode extra data
	int (*setmode) (viddef_t * lvid, struct vmode_s * pcurrentmode);
	int misc;					//misc for display driver (r_glide.dll etc)
} vmode_t;

// ---------------------------------------------
// color mode dependent drawer function pointers
// ---------------------------------------------

extern void (*skycolfunc) (void);
extern void (*colfunc) (void);
extern void (*basecolfunc) (void);
extern void (*fuzzcolfunc) (void);
extern void (*oldfuzzcolfunc)(void);
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

extern boolean fuzzymode;

extern int scr_bpp;
extern byte *scr_borderpatch;	// patch used to fill the view borders

extern consvar_t cv_scr_width;
extern consvar_t cv_scr_height;
extern consvar_t cv_scr_depth;
extern consvar_t cv_fullscreen;
// wait for page flipping to end or not
extern consvar_t cv_vidwait;

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

