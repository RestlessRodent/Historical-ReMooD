// -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
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
// Copyright (C) 1996-1998 Activision and Raven Software
// Copyright (C) 2012 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Heretic Renderer

/***************
*** INCLUDES ***
***************/

#include "doomdef.h"
#include "d_player.h"
#include "rh_main.h"
#include "r_state.h"

/****************
*** CONSTANTS ***
****************/

// Fineangles in the SCREENWIDTH wide window.
#define FIELDOFVIEW             2048

/*****************
*** STRUCTURES ***
*****************/

/* RH_FrameInfo_t -- Frame Info */
typedef struct RH_FrameInfo_s
{
	int32_t x, y, w, h;							// Position
	int32_t AllocWidth, AllocHeight;			// Allocated sizes
	int32_t VidW, VidH;							// Video Sizes
	int32_t* columnofs;							// Column offsets
	uint8_t** ylookup;							// Y Lookup Table
	fixed_t* yslope;							// Y Slope
	fixed_t* yslopetab;							// Y Slope
	int32_t* spanstart;							// Start of spans
	int32_t* spanstop;							// End of spans
	fixed_t* cachedheight;						// Cache
	fixed_t* cacheddistance;					// Cache
	fixed_t* cachedxstep;						// Cache
	fixed_t* cachedystep;						// Cache
	fixed_t* distscale;							// Distance Scale
	angle_t* xtoviewangle;						// X to view angle
	int* viewangletox;							// View angle to x
} RH_FrameInfo_t;

/**************
*** GLOBALS ***
**************/

extern bool setsizeneeded;					// Need screen resize?

/*************
*** LOCALS ***
*************/

static RH_FrameInfo_t l_HFrames[MAXSPLITSCREEN];

/****************
*** FUNCTIONS ***
****************/

/* RH_InitBuffers() -- Initializes Buffers */
void RH_InitBuffers(RH_RenderStat_t* const a_Stat)
{
	RH_FrameInfo_t* Info;
	int32_t i, j, t, x;
	fixed_t dy;
	bool ReBoot;
	fixed_t focallength;
	
	/* Obtain Frame Info */
	Info = (RH_FrameInfo_t*)a_Stat->HLocal;
	
	/* Check for resize */
	if (Info->AllocWidth != a_Stat->viewwidth || Info->AllocHeight != a_Stat->viewheight)
	{
		// Reboot
		ReBoot = true;
		
		// Free old buffers
		if (Info->columnofs)
			Z_Free(Info->columnofs);
		Info->columnofs = NULL;
		if (Info->ylookup)
			Z_Free(Info->ylookup);
		Info->ylookup = NULL;
		
		// Allocation
		Info->columnofs = (int32_t*)Z_Malloc(sizeof(*Info->columnofs) * a_Stat->viewwidth, PU_STATIC, NULL);
		Info->ylookup = (uint8_t**)Z_Malloc(sizeof(*Info->ylookup) * a_Stat->viewheight, PU_STATIC, NULL);
		
		// Set new size
		Info->AllocWidth = a_Stat->viewwidth;
		Info->AllocHeight = a_Stat->viewheight;
	}
	
	/* Check for video resize */
	if (Info->VidW != vid.width || Info->VidH != vid.height)
	{
		// Reboot
		ReBoot = true;
		
		// Free old buffers
		if (Info->yslopetab)
			Z_Free(Info->yslopetab);
		if (Info->spanstart)
			Z_Free(Info->spanstart);
		if (Info->spanstop)
			Z_Free(Info->spanstop);
		if (Info->cachedheight)
			Z_Free(Info->cachedheight);
		if (Info->cacheddistance)
			Z_Free(Info->cacheddistance);
		if (Info->cachedxstep)
			Z_Free(Info->cachedxstep);
		if (Info->cachedystep)
			Z_Free(Info->cachedystep);
		if (Info->distscale)
			Z_Free(Info->distscale);
		if (Info->xtoviewangle)
			Z_Free(Info->xtoviewangle);
		if (Info->viewangletox)
			Z_Free(Info->viewangletox);
		
		// Allocation
		Info->yslopetab = (fixed_t*)Z_Malloc(sizeof(*Info->yslopetab) * (vid.height * 4), PU_STATIC, NULL);
		Info->spanstart = (fixed_t*)Z_Malloc(sizeof(*Info->spanstart) * vid.height, PU_STATIC, NULL);
		Info->spanstop = (fixed_t*)Z_Malloc(sizeof(*Info->spanstop) * vid.height, PU_STATIC, NULL);
		Info->cachedheight = (fixed_t*)Z_Malloc(sizeof(*Info->cachedheight) * vid.height, PU_STATIC, NULL);
		Info->cacheddistance = (fixed_t*)Z_Malloc(sizeof(*Info->cacheddistance) * vid.height, PU_STATIC, NULL);
		Info->cachedxstep = (fixed_t*)Z_Malloc(sizeof(*Info->cachedxstep) * vid.height, PU_STATIC, NULL);
		Info->cachedystep = (fixed_t*)Z_Malloc(sizeof(*Info->cachedystep) * vid.height, PU_STATIC, NULL);
		Info->distscale = (fixed_t*)Z_Malloc(sizeof(*Info->distscale) * vid.width, PU_STATIC, NULL);
		Info->xtoviewangle = (angle_t*)Z_Malloc(sizeof(*Info->xtoviewangle) * (vid.width + 1), PU_STATIC, NULL);
		Info->viewangletox = (fixed_t*)Z_Malloc(sizeof(*Info->viewangletox) * (FINEANGLES / 2), PU_STATIC, NULL);
		
		// Set new size
		Info->VidW = vid.width;
		Info->VidH = vid.height;
	}
	
	/* Copy used slopes and such */
	a_Stat->columnofs = Info->columnofs;
	a_Stat->ylookup = Info->ylookup;
	a_Stat->yslope = Info->yslope;
	a_Stat->spanstart = Info->spanstart;
	a_Stat->spanstop = Info->spanstop;
	a_Stat->cachedheight = Info->cachedheight;
	a_Stat->cacheddistance = Info->cacheddistance;
	a_Stat->cachedxstep = Info->cachedxstep;
	a_Stat->cachedystep = Info->cachedystep;
	a_Stat->distscale = Info->distscale;
	a_Stat->xtoviewangle = Info->xtoviewangle;
	a_Stat->viewangletox = Info->viewangletox;
	
	/* Correct values? */
	if (ReBoot)
	{
		// Column offsets
		for (i = 0; i < a_Stat->viewwidth; i++)
			Info->columnofs[i] = a_Stat->viewwindowx + i;
		
		// Y Lookup
		for (i = 0; i < a_Stat->viewheight; i++)
			Info->ylookup[i] = screens[0] + ((a_Stat->viewwindowy + i) * vid.rowbytes);
		
		// Y Slope
		j = a_Stat->viewheight * 4;
		for (i = 0; i < j; i++)
		{
			//added:10-02-98:(i-centery) became (i-centery*2) and centery*2=viewheight
			dy = ((i - a_Stat->viewheight * 2) << FRACBITS) + FRACUNIT / 2;
			dy = abs(dy);
			Info->yslopetab[i] = FixedDiv(a_Stat->aspectfx, dy);
		}
		
		// Use tangent table to generate viewangletox:
		//  viewangletox will give the next greatest x
		//  after the view angle.
		//
		// Calc focallength
		//  so FIELDOFVIEW angles covers SCREENWIDTH.
		focallength = FixedDiv(a_Stat->centerxfrac, finetangent[FINEANGLES / 4 + /*cv_fov.value */ FIELDOFVIEW / 2]);
	
		for (i = 0; i < FINEANGLES / 2; i++)
		{
			if (finetangent[i] > FRACUNIT * 2)
				t = -1;
			else if (finetangent[i] < -FRACUNIT * 2)
				t = a_Stat->viewwidth + 1;
			else
			{
				t = FixedMul(finetangent[i], focallength);
				t = (a_Stat->centerxfrac - t + FRACUNIT - 1) >> FRACBITS;
			
				if (t < -1)
					t = -1;
				else if (t > a_Stat->viewwidth + 1)
					t = a_Stat->viewwidth + 1;
			}
			
			a_Stat->viewangletox[i] = t;
		}
	
		// Scan viewangletox[] to generate xtoviewangle[]:
		//  xtoviewangle will give the smallest view angle
		//  that maps to x.
		for (x = 0; x <= a_Stat->viewwidth; x++)
		{
			i = 0;
			while (a_Stat->viewangletox[i] > x)
				i++;
			a_Stat->xtoviewangle[x] = (i << ANGLETOFINESHIFT) - ANG90;
		}
	
		// Take out the fencepost cases from viewangletox.
		for (i = 0; i < FINEANGLES / 2; i++)
		{
			t = FixedMul(finetangent[i], focallength);
			t = a_Stat->centerx - t;
		
			if (a_Stat->viewangletox[i] == -1)
				a_Stat->viewangletox[i] = 0;
			else if (a_Stat->viewangletox[i] == a_Stat->viewwidth + 1)
				a_Stat->viewangletox[i] = a_Stat->viewwidth;
		}
	
		a_Stat->clipangle = a_Stat->xtoviewangle[0];
	}
}

/* RH_SetupFrame() -- Sets up frame for drawing */
void RH_SetupFrame(RH_RenderStat_t* const a_Stat)
{
	player_t* Player;
	mobj_t* Mo;
	angle_t tableAngle;
	int32_t TempCenterY, i, j;
	fixed_t dy;
	
	/* Get Player/Mo */
	Player = a_Stat->viewplayer;
	Mo = Player->mo;
	
	/* Initial Viewing Point */
	a_Stat->viewangleoffset = 0;
	a_Stat->viewangle = Mo->angle + a_Stat->viewangleoffset;
	tableAngle = a_Stat->viewangle >> ANGLETOFINESHIFT;
	a_Stat->viewz = Player->viewz;
	a_Stat->viewsin = finesine[tableAngle];
	a_Stat->viewcos = finecosine[tableAngle];
	
	// Chicken?
	if (Player->chickenTics && Player->chickenPeck)
	{
		a_Stat->viewx = Mo->x + (Player->chickenPeck * finecosine[tableAngle]);
		a_Stat->viewy = Mo->y + (Player->chickenPeck * finesine[tableAngle]);
	}
	
	// Normal player
	else
	{
		a_Stat->viewx = Mo->x;
		a_Stat->viewy = Mo->y;
	}
	
	/* Special Effects */
	a_Stat->extralight = Player->extralight;
	
	/* Aspect Ratio */
	a_Stat->aspectx = (((vid.height * a_Stat->centerx * BASEVIDWIDTH) / BASEVIDHEIGHT) / vid.width);
	a_Stat->aspectfx = a_Stat->aspectx << FRACBITS;
	
	/* Calculate Slope */
	a_Stat->yslope = &a_Stat->yslopetab[(3 * a_Stat->viewheight / 2) - dy];
	a_Stat->centery = (a_Stat->viewheight / 2) + dy;
	a_Stat->centeryfrac = a_Stat->centery << FRACBITS;
	
#if defined(__DUPECODEHERE__)

	tempCentery = viewheight/2+(player->lookdir)*screenblocks/10;
	if(centery != tempCentery)
	{
		centery = tempCentery;
		centeryfrac = centery<<FRACBITS;
		for(i = 0; i < viewheight; i++)
		{
			yslope[i] = FixedDiv ((viewwidth<<detailshift)/2*FRACUNIT,
				abs(((i-centery)<<FRACBITS)+FRACUNIT/2));
		}
	}
	sscount = 0;
	if(player->fixedcolormap)
	{
		fixedcolormap = colormaps+player->fixedcolormap
			*256*sizeof(lighttable_t);
		walllights = scalelightfixed;
		for(i = 0; i < MAXLIGHTSCALE; i++)
		{
			scalelightfixed[i] = fixedcolormap;
		}
	}
	else
	{
		fixedcolormap = 0;
	}
	framecount++;
	validcount++;
	if(BorderNeedRefresh)
	{
		if(setblocks < 10)
		{
			R_DrawViewBorder();
		}
		BorderNeedRefresh = false;
		BorderTopRefresh = false;
		UpdateState |= I_FULLSCRN;
	}
	if(BorderTopRefresh)
	{
		if(setblocks < 10)
		{
			R_DrawTopBorder();
		}
		BorderTopRefresh = false;
		UpdateState |= I_MESSAGES;
	}

#ifdef __NeXT__
	RD_ClearMapWindow ();
#endif
#ifdef __WATCOMC__
	destview = destscreen+(viewwindowx>>2)+viewwindowy*80;
#endif
#endif
}


/* R_ExecuteSetViewSize_HERETIC() -- Sets the view size */
void R_ExecuteSetViewSize_HERETIC(void)
{
	int32_t n;
	
	/* Setting size is no longer needed */
	setsizeneeded = false;
	
	/* Prepare views for all 4 players */
	for (n = 0; n < g_SplitScreen + 1; n++)
	{
		// Initial Screen Box
			// 1 Player
		if (g_SplitScreen <= 0)
		{
			l_HFrames[n].w = vid.width;
			l_HFrames[n].h = vid.height;
			l_HFrames[n].x = 0;
			l_HFrames[n].y = 0;
		}
			// 2 Players
		else if (g_SplitScreen == 1)
		{
			l_HFrames[n].w = vid.width;
			l_HFrames[n].h = vid.height / 2;
			l_HFrames[n].x = 0;
			l_HFrames[n].y = l_HFrames[n].h * n;
		}
			// 3+ Players
		else
		{
			l_HFrames[n].w = vid.width / 2;
			l_HFrames[n].h = vid.height / 2;
			l_HFrames[n].x = l_HFrames[n].w * (n & 1);
			l_HFrames[n].y = l_HFrames[n].h * ((n >> 1) & 1);
		}
	}
}

/* R_RenderPlayerView_HERETIC() -- Renders player view */
void R_RenderPlayerView_HERETIC(player_t* player, const size_t a_Screen)
{
	RH_RenderStat_t Stat;
	angle_t angle;
	
	/* Initialize Render Status */
	memset(&Stat, 0, sizeof(Stat));
	
	// Basic
	Stat.HLocal = &l_HFrames[a_Screen];
	
	// Viewing player
	Stat.viewplayer = player;
	Stat.viewwindowx = l_HFrames[a_Screen].x;
	Stat.viewwindowy = l_HFrames[a_Screen].h;
	Stat.viewwidth = l_HFrames[a_Screen].w;
	Stat.viewheight = l_HFrames[a_Screen].h;
	
	// Scaling
	Stat.scaledviewwidth = vid.width;
	Stat.centerx = Stat.viewwidth >> 1; 
	Stat.centery = Stat.viewheight >> 1;
	
	Stat.centerxfrac = Stat.centerx << FRACBITS;
	Stat.centeryfrac = Stat.centery << FRACBITS;
	Stat.projection = Stat.centerxfrac;
	
	/* Possibly Initialize Buffers */
	RH_InitBuffers(&Stat);
	
	/* Setup */
	RH_SetupFrame(&Stat);
	
	/* Post frame initialization */
	// Plane Scale
	angle = (Stat.viewangle - ANG90) >> ANGLETOFINESHIFT;
	Stat.basexscale = FixedDiv(finecosine[angle], Stat.centerxfrac);
	Stat.baseyscale = -FixedDiv(finesine[angle], Stat.centerxfrac);
	
	/* Draw */
	RH_RenderBSPNode(&Stat, numnodes - 1);
	RH_RenderPlanes(&Stat);
}

