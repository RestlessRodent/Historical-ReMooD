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
//      Here is a core component: drawing the floors and ceilings,
//       while maintaining a per column clipping list only.
//      Moreover, the sky areas have to be determined.

#include "r_plane.h"
#include "z_zone.h"
#include "r_draw.h"
#include "tables.h"
#include "r_sky.h"
#include "r_state.h"
#include "screen.h"

#include "console.h"	// For con_clipviewtop

//#include "doomdef.h"
//#include "console.h"
//#include "g_game.h"
//#include "r_data.h"
//#include "r_local.h"
//#include "r_state.h"
//#include "r_splats.h"			//faB(21jan):testing
//#include "r_sky.h"
//#include "v_video.h"
//#include "w_wad.h"
//#include "z_zone.h"

//#include "p_setup.h"			// levelflats

planefunction_t floorfunc;
planefunction_t ceilingfunc;

//
// opening
//

// Here comes the obnoxious "visplane".

/*#define                 MAXVISPLANES 128 //SoM: 3/20/2000
visplane_t*             visplanes;
visplane_t*             lastvisplane;*/

//SoM: 3/23/2000: Use Boom visplane hashing.
static visplane_t* visplanes[MAXVISPLANES];
static visplane_t* freetail;
static visplane_t** freehead = &freetail;

visplane_t** visplane_ptr = visplanes;

visplane_t* floorplane;
visplane_t* ceilingplane;

visplane_t* currentplane;

planemgr_t ffloor[MAXFFLOORS];
int numffloors;

//SoM: 3/23/2000: Boom visplane hashing routine.
#define visplane_hash(picnum,lightlevel,height) \
  ((unsigned)((picnum)*3+(lightlevel)+(height)*7) & (MAXVISPLANES-1))

// ?

//SoM: 3/23/2000: Use boom opening limit removal
size_t maxopenings;
short* openings, *lastopening;

//
// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
//
short* floorclip = NULL;
short* ceilingclip = NULL;
fixed_t* frontscale = NULL;

//
// spanstart holds the start of a plane span
// initialized to 0 at start
//
int* spanstart = NULL;

//
// texture mapping
//
lighttable_t** planezlight;
fixed_t planeheight;

//added:10-02-98: yslopetab is what yslope used to be,
//                yslope points somewhere into yslopetab,
//                now (viewheight/2) slopes are calculated above and
//                below the original viewheight for mouselook
//                (this is to calculate yslopes only when really needed)
//                (when mouselookin', yslope is moving into yslopetab)
//                Check R_SetupFrame, R_SetViewSize for more...
fixed_t* yslopetab = NULL;
fixed_t* yslope = NULL;

fixed_t* distscale;
fixed_t basexscale;
fixed_t baseyscale;

fixed_t* cachedheight = NULL;
fixed_t* cacheddistance = NULL;
fixed_t* cachedxstep = NULL;
fixed_t* cachedystep = NULL;

fixed_t xoffs, yoffs;

//
// R_InitPlanes
// Only at game startup.
//
void R_InitPlanes(void)
{
	// Doh!
}

//
// R_MapPlane
//
// Uses global vars:
//  planeheight
//  ds_source
//  basexscale
//  baseyscale
//  viewx
//  viewy
//  xoffs
//  yoffs
//
// BASIC PRIMITIVE
//

void R_MapPlane(int y,			// t1
                int x1, int x2)
{
	angle_t angle;
	fixed_t distance;
	fixed_t length;
	unsigned index;
	
#ifdef RANGECHECK
	if (x2 < x1 || x1 < 0 || x2 >= viewwidth || (unsigned)y > viewheight)
	{
		I_Error("R_MapPlane: %i, %i at %i", x1, x2, y);
	}
#endif
	
	if ((unsigned)y >= viewheight)
		return;
		
	if (planeheight != cachedheight[y])
	{
		cachedheight[y] = planeheight;
		distance = cacheddistance[y] = FixedMul(planeheight, yslope[y]);
		ds_xstep = cachedxstep[y] = FixedMul(distance, basexscale);
		ds_ystep = cachedystep[y] = FixedMul(distance, baseyscale);
	}
	else
	{
		distance = cacheddistance[y];
		ds_xstep = cachedxstep[y];
		ds_ystep = cachedystep[y];
	}
	length = FixedMul(distance, distscale[x1]);
	angle = (currentplane->viewangle + xtoviewangle[x1]) >> ANGLETOFINESHIFT;
	// SoM: Wouldn't it be faster just to add viewx and viewy to the plane's
	// x/yoffs anyway?? (Besides, it serves my purpose well for portals!)
	ds_xfrac = /*viewx + */ FixedMul(finecosine[angle], length) + xoffs;
	
	ds_yfrac = /*-viewy*/ yoffs - FixedMul(finesine[angle], length);
	
	if (fixedcolormap)
		ds_colormap = fixedcolormap;
	else
	{
		index = distance >> LIGHTZSHIFT;
		
		if (index >= MAXLIGHTZ)
			index = MAXLIGHTZ - 1;
			
		ds_colormap = planezlight[index];
	}
	if (currentplane->extra_colormap && !fixedcolormap)
		ds_colormap = currentplane->extra_colormap->colormap + (ds_colormap - colormaps);
		
	ds_y = y;
	ds_x1 = x1;
	ds_x2 = x2;
	// high or low detail
	
	spanfunc();
}

//
// R_ClearPlanes
// At begining of frame.
//
//Fab:26-04-98:
// NOTE : uses con_clipviewtop, so that when console is on,
//        don't draw the part of the view hidden under the console
void R_ClearPlanes(player_t* player)
{
	int i, p;
	angle_t angle;
	
	// opening / clipping determination
	for (i = 0; i < viewwidth; i++)
	{
		floorclip[i] = viewheight;
		ceilingclip[i] = con_clipviewtop;	//Fab:26-04-98: was -1
		frontscale[i] = INT_MAX;
		for (p = 0; p < MAXFFLOORS; p++)
		{
			ffloor[p].f_clip[i] = viewheight;
			ffloor[p].c_clip[i] = con_clipviewtop;
		}
	}
	
	numffloors = 0;
	
	//lastvisplane = visplanes;
	
	//SoM: 3/23/2000
	for (i = 0; i < MAXVISPLANES; i++)
		for (*freehead = visplanes[i], visplanes[i] = NULL; *freehead;)
			freehead = &(*freehead)->next;
			
	lastopening = openings;
	
	// texture calculation
	memset(cachedheight, 0, sizeof(fixed_t) * vid.height);
	
	// left to right mapping
	angle = (viewangle - ANG90) >> ANGLETOFINESHIFT;
	
	// scale will be unit scale at SCREENWIDTH/2 distance
	basexscale = FixedDiv(finecosine[angle], centerxfrac);
	baseyscale = -FixedDiv(finesine[angle], centerxfrac);
}

//SoM: 3/23/2000: New function, by Lee Killough
visplane_t* new_visplane(unsigned hash)
{
	visplane_t* check = freetail;
	
	if (!check)
	{
		check = Z_Malloc(sizeof(visplane_t), PU_STATIC, 0);
		check->top = Z_Malloc(sizeof(*check->top) * (vid.width + 2), PU_STATIC, NULL);
		check->top = &check->top[1];
		check->bottom = Z_Malloc(sizeof(*check->top) * (vid.width + 2), PU_STATIC, NULL);
		check->bottom = &check->bottom[1];
	}
	else if (!(freetail = freetail->next))
		freehead = &freetail;
		
	check->next = visplanes[hash];
	visplanes[hash] = check;
	return check;
}

//
// R_FindPlane : cherche un visplane ayant les valeurs identiques:
//               meme hauteur, meme flattexture, meme lightlevel.
//               Sinon en alloue un autre.
//	"seeks a visplane having values ​​identical: same height, same flattexture, even lightlevel. Otherwise allocates another."
visplane_t* R_FindPlane(fixed_t height, int picnum, int lightlevel, fixed_t xoff, fixed_t yoff, extracolormap_t* planecolormap, ffloor_t* ffloor, sector_t* const a_Sector)
{
	visplane_t* check;
	unsigned hash;				//SoM: 3/23/2000
	
	xoff += viewx;				// SoM
	yoff = -viewy + yoff;
	
	if (picnum == skyflatnum)
	{
		height = 0;				// all skys map together
		lightlevel = 0;
	}
	//SoM: 3/23/2000: New visplane algorithm uses hash table -- killough
	hash = visplane_hash(picnum, lightlevel, height);
	
	for (check = visplanes[hash]; check; check = check->next)
		if (height == check->height &&
		        picnum == check->picnum &&
		        lightlevel == check->lightlevel &&
		        xoff == check->xoffs &&
		        yoff == check->yoffs &&
		        planecolormap == check->extra_colormap && !ffloor && !check->ffloor && check->viewz == viewz && check->viewangle == viewangle &&
		        
		        check->SkyTexture == a_Sector->AltSkyTexture)
			return check;
			
	check = new_visplane(hash);
	
	check->height = height;
	check->picnum = picnum;
	check->lightlevel = lightlevel;
	check->minx = vid.width;
	check->maxx = -1;
	check->xoffs = xoff;
	check->yoffs = yoff;
	check->extra_colormap = planecolormap;
	check->ffloor = ffloor;
	check->viewz = viewz;
	check->viewangle = viewangle;
	check->Sector = a_Sector;
	
	if (check->Sector->AltSkyTexture)
	{
		check->SkyTexture = check->Sector->AltSkyTexture;
		check->SkyTextureMid = 100 << FRACBITS;
	}
	else
	{
		check->SkyTexture = skytexture;
		check->SkyTextureMid = skytexturemid;
	}
	
	memset(check->top, 0xff, sizeof(*check->top) * vid.width);
	
	return check;
}

//
// R_CheckPlane : return same visplane or alloc a new one if needed
//
visplane_t* R_CheckPlane(visplane_t* pl, int start, int stop)
{
	int intrl;
	int intrh;
	int unionl;
	int unionh;
	int x;
	
	if (start < pl->minx)
	{
		intrl = pl->minx;
		unionl = start;
	}
	else
	{
		unionl = pl->minx;
		intrl = start;
	}
	
	if (stop > pl->maxx)
	{
		intrh = pl->maxx;
		unionh = stop;
	}
	else
	{
		unionh = pl->maxx;
		intrh = stop;
	}
	
	//added 30-12-97 : 0xff ne vaut plus -1 avec un short...
	for (x = intrl; x <= intrh; x++)
		if (pl->top[x] != 0xffff)
			break;
			
	//SoM: 3/23/2000: Boom code
	if (x > intrh)
		pl->minx = unionl, pl->maxx = unionh;
	else
	{
		unsigned hash = visplane_hash(pl->picnum, pl->lightlevel, pl->height);
		visplane_t* new_pl = new_visplane(hash);
		
		new_pl->height = pl->height;
		new_pl->picnum = pl->picnum;
		new_pl->lightlevel = pl->lightlevel;
		new_pl->xoffs = pl->xoffs;	// killough 2/28/98
		new_pl->yoffs = pl->yoffs;
		new_pl->extra_colormap = pl->extra_colormap;
		new_pl->ffloor = pl->ffloor;
		new_pl->viewz = pl->viewz;
		new_pl->viewangle = pl->viewangle;
		new_pl->Sector = pl->Sector;
		
		if (pl->Sector->AltSkyTexture)
		{
			new_pl->SkyTexture = pl->Sector->AltSkyTexture;
			new_pl->SkyTextureMid = 100 << FRACBITS;
		}
		else
		{
			new_pl->SkyTexture = skytexture;
			new_pl->SkyTextureMid = skytexturemid;
		}
		
		pl = new_pl;
		pl->minx = start;
		pl->maxx = stop;
		memset(pl->top, 0xff, sizeof(unsigned short) * vid.width);
	}
	return pl;
}

//
// R_ExpandPlane
//
// SoM: This function basically expands the visplane or I_Errors
// The reason for this is that when creating 3D floor planes, there is no
// need to create new ones with R_CheckPlane, because 3D floor planes
// are created by subsector and there is no way a subsector can graphically
// overlap.
void R_ExpandPlane(visplane_t* pl, int start, int stop)
{
	int intrl;
	int intrh;
	int unionl;
	int unionh;
	int x;
	
	if (start < pl->minx)
	{
		intrl = pl->minx;
		unionl = start;
	}
	else
	{
		unionl = pl->minx;
		intrl = start;
	}
	
	if (stop > pl->maxx)
	{
		intrh = pl->maxx;
		unionh = stop;
	}
	else
	{
		unionh = pl->maxx;
		intrh = stop;
	}
	
	for (x = start; x <= stop; x++)
		if (pl->top[x] != 0xffff)
			break;
			
	//SoM: 3/23/2000: Boom code
	if (x > stop)
		pl->minx = unionl, pl->maxx = unionh;
//    else
//      I_Error("R_ExpandPlane: planes in same subsector overlap?!\nminx: %i, maxx: %i, start: %i, stop: %i\n", pl->minx, pl->maxx, start, stop);

	pl->minx = unionl, pl->maxx = unionh;
}

//
// R_MakeSpans
//
void R_MakeSpans(int x, int t1, int b1, int t2, int b2)
{
	while (t1 < t2 && t1 <= b1)
	{
		R_MapPlane(t1, spanstart[t1], x - 1);
		t1++;
	}
	while (b1 > b2 && b1 >= t1)
	{
		R_MapPlane(b1, spanstart[b1], x - 1);
		b1--;
	}
	
	while (t2 < t1 && t2 <= b2)
	{
		spanstart[t2] = x;
		t2++;
	}
	while (b2 > b1 && b2 >= t2)
	{
		spanstart[b2] = x;
		b2--;
	}
}

uint8_t* R_GetFlat(int flatnum);

void R_DrawPlanes(void)
{
	visplane_t* pl;
	int x;
	int angle;
	int i;						//SoM: 3/23/2000
	
	spanfunc = basespanfunc;
	
	for (i = 0; i < MAXVISPLANES; i++, pl++)
		for (pl = visplanes[i]; pl; pl = pl->next)
		{
			// sky flat
			if (pl->picnum == skyflatnum)
			{
				//added:12-02-98: use correct aspect ratio scale
				//dc_iscale = FixedDiv (FRACUNIT, pspriteyscale);
				dc_iscale = skyscale;
				
// Kik test non-moving sky .. weird
// cy = centery;
// centery = (viewheight/2);

				// Sky is allways drawn full bright,
				//  i.e. colormaps[0] is used.
				// Because of this hack, sky is not affected
				//  by INVUL inverse mapping.
#if 0
				// BP: this fix sky not inversed in invuln but it is a original doom2 feature (bug?)
				if (fixedcolormap)
					dc_colormap = fixedcolormap;
				else
#endif
				dc_colormap = colormaps;
				dc_texturemid = pl->SkyTextureMid;
				dc_texheight = (textures[pl->SkyTexture]->XHeight) >> FRACBITS;
				
				for (x = pl->minx; x <= pl->maxx; x++)
				{
					dc_yl = pl->top[x];
					dc_yh = pl->bottom[x];
					
					if (dc_yl <= dc_yh)
					{
						angle = (viewangle + xtoviewangle[x]) >> ANGLETOSKYSHIFT;
						dc_x = x;
						dc_source = R_GetColumn(pl->SkyTexture, angle);
						skycolfunc();
					}
				}
// centery = cy;
				continue;
			}
			
			if (pl->ffloor)
				continue;
				
			R_DrawSinglePlane(pl, true);
		}
}

void R_DrawSinglePlane(visplane_t* pl, bool_t handlesource)
{
	int light = 0;
	int x;
	int stop;
	int angle;
	
	if (pl->minx > pl->maxx)
		return;
		
	spanfunc = basespanfunc;
	if (pl->ffloor)
	{
		if (pl->ffloor->flags & FF_TRANSLUCENT)
		{
			spanfunc = R_DrawTranslucentSpan_8;
			
#if 1
			// GhostlyDeath <October 1, 2011> -- Allow all 10 transparencies instead of just 3
			// 0 = transparent, 255 = opaque
			x = pl->ffloor->alpha; // Some mappers incrorrectly place alpha (not 0-255)
			if (x >= 256 || x <= 0)
				x = 128;
			x = FixedDiv(x << FRACBITS, 1677568) >> FRACBITS;
			if (x >= 10)
				x = 10;
			
			// Use this transparency
			ds_transmap = transtables + ((10 - x) * 0x10000);
#else		
			// Hacked up support for alpha value in software mode SSNTails 09-24-2002
			if (pl->ffloor->alpha < 64)
				ds_transmap = ((3) << FF_TRANSSHIFT) - 0x10000 + transtables;
			else if (pl->ffloor->alpha < 128 && pl->ffloor->alpha > 63)
				ds_transmap = ((2) << FF_TRANSSHIFT) - 0x10000 + transtables;
			else
				ds_transmap = ((1) << FF_TRANSSHIFT) - 0x10000 + transtables;
#endif
				
			if (pl->extra_colormap && pl->extra_colormap->fog)
				light = (pl->lightlevel >> LIGHTSEGSHIFT);
			else
				light = LIGHTLEVELS - 1;
		}
		else if (pl->ffloor->flags & FF_FOG)
		{
			spanfunc = R_DrawFogSpan_8;
			light = (pl->lightlevel >> LIGHTSEGSHIFT);
		}
		else if (pl->extra_colormap && pl->extra_colormap->fog)
			light = (pl->lightlevel >> LIGHTSEGSHIFT);
		else
			light = (pl->lightlevel >> LIGHTSEGSHIFT) + extralight;
	}
	else
	{
		if (pl->extra_colormap && pl->extra_colormap->fog)
			light = (pl->lightlevel >> LIGHTSEGSHIFT);
		else
			light = (pl->lightlevel >> LIGHTSEGSHIFT) + extralight;
	}
	
	if (viewangle != pl->viewangle)
	{
		memset(cachedheight, 0, sizeof(fixed_t) * vid.height);
		
		angle = (pl->viewangle - ANG90) >> ANGLETOFINESHIFT;
		
		basexscale = FixedDiv(finecosine[angle], centerxfrac);
		baseyscale = -FixedDiv(finesine[angle], centerxfrac);
		viewangle = pl->viewangle;
	}
	
	currentplane = pl;
	
	if (handlesource)
	{
		int size;
		
		ds_source = (uint8_t*)R_GetFlat(pl->picnum);
		
		size = 64 * 64;//W_LumpLength(pl->picnum);
		
		switch (size)
		{
			case 4194304:		// 2048x2048 lump
				flatsize = 2048;
				flatmask = 2047 << 11;
				flatsubtract = 11;
				break;
			case 1048576:		// 1024x1024 lump
				flatsize = 1024;
				flatmask = 1023 << 10;
				flatsubtract = 10;
				break;
			case 262144:		// 512x512 lump
				flatsize = 512;
				flatmask = 511 << 9;
				flatsubtract = 9;
				break;
			case 65536:		// 256x256 lump
				flatsize = 256;
				flatmask = 255 << 8;
				flatsubtract = 8;
				break;
			case 16384:		// 128x128 lump
				flatsize = 128;
				flatmask = 127 << 7;
				flatsubtract = 7;
				break;
			case 1024:			// 32x32 lump
				flatsize = 32;
				flatmask = 31 << 5;
				flatsubtract = 5;
				break;
			default:			// 64x64 lump
				flatsize = 64;
				flatmask = 0x3f << 6;
				flatsubtract = 6;
				break;
		}
	}
	
	xoffs = pl->xoffs;
	yoffs = pl->yoffs;
	planeheight = abs(pl->height - pl->viewz);
	
	if (light >= LIGHTLEVELS)
		light = LIGHTLEVELS - 1;
		
	if (light < 0)
		light = 0;
		
	planezlight = zlight[light];
	
	//set the MAXIMUM value for unsigned
	// GhostlyDeath <Monday, January 5, 2008> -- this caused buffer overflow and underflow
	// However, this used to be in a structure shared with other members, so an underflow
	// would affect the other member...
	pl->top[pl->maxx + 1] = 0xffff;
	pl->top[pl->minx - 1] = 0xffff;
	
	stop = pl->maxx + 1;
	
	for (x = pl->minx; x <= stop; x++)
	{
		R_MakeSpans(x, pl->top[x - 1], pl->bottom[x - 1], pl->top[x], pl->bottom[x]);
	}
	
	if (handlesource)
		Z_ChangeTag(ds_source, PU_CACHE);
}

void R_PlaneBounds(visplane_t* plane)
{
	int i;
	int hi, low;
	
	hi = plane->top[plane->minx];
	low = plane->bottom[plane->minx];
	
	for (i = plane->minx + 1; i <= plane->maxx; i++)
	{
		if (plane->top[i] < hi)
			hi = plane->top[i];
		if (plane->bottom[i] > low)
			low = plane->bottom[i];
	}
	plane->high = hi;
	plane->low = low;
}
