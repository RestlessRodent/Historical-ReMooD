// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Refresh/render internal state variables (global).

#ifndef __R_STATE__
#define __R_STATE__

#include "tables.h"
#include "r_defs.h"
#include "r_main.h"

// Need data structure definitions.



//
// Refresh internal data structures,
//  for rendering.
//

// needed for pre rendering (fracs)
extern fixed_t* spritewidth;
extern fixed_t* spriteoffset;
extern fixed_t* spritetopoffset;
extern fixed_t* spriteheight;
extern size_t g_SpritesBufferSize;	// GhostlyDeath <July 24, 2011> -- Unlimited sprites! not really

extern lighttable_t* colormaps;

//SoM: 3/30/2000: Boom colormaps.
//SoM: 4/7/2000: Had to put a limit on colormaps :(
#define                 MAXCOLORMAPS 30

extern int num_extra_colormaps;
extern extracolormap_t extra_colormaps[MAXCOLORMAPS];

extern int viewwidth;
extern int scaledviewwidth;
extern int viewheight;

extern int firstflat;
extern int firstwaterflat;		//added:18-02-98:WATER!

// for global animation
extern int* flattranslation;
extern int* texturetranslation;

// Sprite....
extern int firstspritelump;
extern int lastspritelump;
extern int numspritelumps;

//
// Lookup tables for map data.
//
extern int numsprites;
extern spritedef_t* sprites;

// GhostlyDeath <February 24, 2012> -- Extended sprites
extern size_t g_NumExSprites;
extern spritedef_t* g_ExSprites;

#define MAXEXSPRITEFRAMES 29

extern int numvertexes;
extern vertex_t* vertexes;

extern int numsegs;
extern seg_t* segs;

extern int numsectors;
extern sector_t* sectors;

extern int numsubsectors;
extern subsector_t* subsectors;

extern int numnodes;
extern node_t* nodes;

extern int numlines;
extern line_t* lines;

extern int numsides;
extern side_t* sides;

//
// POV data.
//
extern fixed_t viewx;
extern fixed_t viewy;
extern fixed_t viewz;

// SoM: Portals require that certain functions use a different x and y pos
// than the actual view pos...
extern fixed_t bspx;
extern fixed_t bspy;

extern angle_t viewangle;
extern angle_t aimingangle;
extern angle_t bspangle;
extern player_t* viewplayer;

// GhostlyDeath -- angle_t in the .c? maybe I deleted it accidently
extern angle_t clipangle;

extern int viewangletox[FINEANGLES / 2];
extern angle_t* xtoviewangle;

//extern fixed_t                finetangent[FINEANGLES/2];

extern fixed_t rw_distance;
extern angle_t rw_normalangle;

// angle to line origin
extern int rw_angle1;

// Segs count?
extern int sscount;

#endif
