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
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
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
//      The status bar widget code.

#include "doomdef.h"
#include "st_lib.h"
#include "st_stuff.h"
#include "v_video.h"
#include "z_zone.h"
#include "r_main.h"
#include "am_map.h"

#include "i_video.h"
//#define DEBUG

//
// Hack display negative frags.
//  Loads and store the stminus lump.
//
patch_t* sttminus;

void STlib_init(void)
{
	sttminus = (patch_t*)W_CachePatchName("STTMINUS", PU_STATIC);
}

// Initialize number widget
void STlib_initNum(st_number_t* n, int x, int y, patch_t** pl, int* num, bool_t* on, int width)
{
	n->x = x;
	n->y = y;
	n->oldnum = 0;
	n->width = width;			// number of digits
	n->num = num;
	n->on = on;
	n->p = pl;
}

//
// A fairly efficient way to draw a number
//  based on differences from the old number.
// Note: worth the trouble?
//
void STlib_drawNum(st_number_t* n, bool_t refresh)
{

	int numdigits = n->width;
	int num = *n->num;
	
	int w = LittleSwapInt16(n->p[0]->width);
	int h = LittleSwapInt16(n->p[0]->height);
	int x = n->x;
	
	int neg;
	
	n->oldnum = *n->num;
	
	neg = num < 0;
	
	if (neg)
	{
		if (numdigits == 2 && num < -9)
			num = -9;
		else if (numdigits == 3 && num < -99)
			num = -99;
			
		num = -num;
	}
	// clear the area
	x = n->x - numdigits * w;
	
#ifdef DEBUG
	CONL_PrintF("V_CopyRect1: %d %d %d %d %d %d %d %d val: %d\n", x, n->y, BG, w * numdigits, h, x, n->y, STTRANSPARENTSCREEN, num);
#endif
	// dont clear background in overlay
	if (!st_overlay)			//faB:current hardware mode always refresh the statusbar
		V_CopyRect(x, n->y, BG, w * numdigits, h, x, n->y, STTRANSPARENTSCREEN);
		
	// if non-number, do not draw it
	if (num == 1994)
		return;
		
	x = n->x;
	
	// in the special case of 0, you draw 0
	if (!num)
		V_DrawScaledPatch(x - w, n->y, STTRANSPARENTSCREEN, n->p[0]);
		
	// draw the new number
	while (num && numdigits--)
	{
		x -= w;
		V_DrawScaledPatch(x, n->y, STTRANSPARENTSCREEN, n->p[num % 10]);
		num /= 10;
	}
	
	// draw a minus sign if necessary
	if (neg)
		V_DrawScaledPatch(x - 8, n->y, STTRANSPARENTSCREEN, sttminus);
}

//
void STlib_updateNum(st_number_t* n, bool_t refresh)
{
	if (*n->on)
		STlib_drawNum(n, refresh);
}

//
void STlib_initPercent(st_percent_t* p, int x, int y, patch_t** pl, int* num, bool_t* on, patch_t* percent)
{
	STlib_initNum(&p->n, x, y, pl, num, on, 3);
	p->p = percent;
}

void STlib_updatePercent(st_percent_t* per, int refresh)
{
	if (refresh && *per->n.on)
		V_DrawScaledPatch(per->n.x, per->n.y, STTRANSPARENTSCREEN, per->p);
		
	STlib_updateNum(&per->n, refresh);
}

void STlib_initMultIcon(st_multicon_t* i, int x, int y, patch_t** il, int* inum, bool_t* on)
{
	i->x = x;
	i->y = y;
	i->oldinum = -1;
	i->inum = inum;
	i->on = on;
	i->p = il;
}

void STlib_updateMultIcon(st_multicon_t* mi, bool_t refresh)
{
	int w;
	int h;
	int x;
	int y;
	
	if (*mi->on && (mi->oldinum != *mi->inum || refresh) && (*mi->inum != -1))
	{
		if (mi->oldinum != -1)
		{
			x = mi->x - LittleSwapInt16(mi->p[mi->oldinum]->leftoffset);
			y = mi->y - LittleSwapInt16(mi->p[mi->oldinum]->topoffset);
			w = LittleSwapInt16(mi->p[mi->oldinum]->width);
			h = LittleSwapInt16(mi->p[mi->oldinum]->height);
			
#ifdef DEBUG
			CONL_PrintF("V_CopyRect2: %d %d %d %d %d %d %d %d\n", x, y, BG, w, h, x, y, STTRANSPARENTSCREEN);
#endif
			//faB:current hardware mode always refresh the statusbar
			if (!st_overlay)
				V_CopyRect(x, y, BG, w, h, x, y, STTRANSPARENTSCREEN);
		}
		V_DrawScaledPatch(mi->x, mi->y, STTRANSPARENTSCREEN, mi->p[*mi->inum]);
		mi->oldinum = *mi->inum;
	}
}

void STlib_initBinIcon(st_binicon_t* b, int x, int y, patch_t* i, bool_t* val, bool_t* on)
{
	b->x = x;
	b->y = y;
	b->oldval = 0;
	b->val = val;
	b->on = on;
	b->p = i;
}

void STlib_updateBinIcon(st_binicon_t* bi, bool_t refresh)
{
	int x;
	int y;
	int w;
	int h;
	
	if (*bi->on && (bi->oldval != *bi->val || refresh))
	{
		x = bi->x - LittleSwapInt16(bi->p->leftoffset);
		y = bi->y - LittleSwapInt16(bi->p->topoffset);
		w = LittleSwapInt16(bi->p->width);
		h = LittleSwapInt16(bi->p->height);
		
		if (*bi->val)
			V_DrawScaledPatch(bi->x, bi->y, STTRANSPARENTSCREEN, bi->p);
		else
		{
#ifdef DEBUG
			CONL_PrintF("V_CopyRect3: %d %d %d %d %d %d %d %d\n", x, y, BG, w, h, x, y, STTRANSPARENTSCREEN);
#endif
			if (!st_overlay)	//faB:current hardware mode always refresh the statusbar
				V_CopyRect(x, y, BG, w, h, x, y, STTRANSPARENTSCREEN);
		}
		
		bi->oldval = *bi->val;
	}
	
}
