// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION:

#include "r_main.h"
#include "dstrings.h"
#include "console.h"
#include "d_netcmd.h"
#include "d_player.h"
#include "p_mobj.h"
#include "r_plane.h"
#include "g_state.h"
#include "p_demcmp.h"
#include "v_video.h"
#include "d_prof.h"
#include "r_segs.h"
#include "vhw_wrap.h"
#include "st_stuff.h"
#include "r_things.h"
#include "t_func.h"		// for script_camera_on
#include "i_system.h"	// for graphics_started
#include "g_game.h"
#include "r_draw.h"
#include "r_sky.h"























// Fineangles in the SCREENWIDTH wide window.
#define FIELDOFVIEW             2048

int viewangleoffset;

// increment every time a check is made
int validcount = 1;

lighttable_t* fixedcolormap;

int centerx;
int centery;
int centerypsp;					//added:06-02-98:cf R_DrawPSprite

fixed_t centerxfrac;
fixed_t centeryfrac;
fixed_t projection;

//added:02-02-98:fixing the aspect ration stuff...
fixed_t projectiony;


int sscount;
int linecount;
int loopcount;

fixed_t viewx;
fixed_t viewy;
fixed_t viewz;

angle_t viewangle;
angle_t aimingangle;

fixed_t viewcos;
fixed_t viewsin;

player_t* viewplayer;

// 0 = high, 1 = low
int detailshift;

//
// precalculated math tables
//
angle_t clipangle;

// The viewangletox[viewangle + FINEANGLES/4] lookup
// maps the visible view angles to screen X coordinates,
// flattening the arc to a flat projection plane.
// There will be many angles mapped to the same X.
int viewangletox[FINEANGLES / 2];

// The xtoviewangleangle[] table maps a screen pixel
// to the lowest viewangle that maps back to x ranges
// from clipangle to -clipangle.
angle_t* xtoviewangle = NULL;

// UNUSED.
// The finetangentgent[angle+FINEANGLES/4] table
// holds the fixed_t tangent values for view angles,
// ranging from INT_MIN to 0 to INT_MAX.
// fixed_t              finetangent[FINEANGLES/2];

// fixed_t              finesine[5*FINEANGLES/4];
fixed_t* finecosine = &finesine[FINEANGLES / 4];

lighttable_t* scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t* scalelightfixed[MAXLIGHTSCALE];
lighttable_t* zlight[LIGHTLEVELS][MAXLIGHTZ];

//SoM: 3/30/2000: Hack to support extra boom colormaps.
int num_extra_colormaps;
extracolormap_t extra_colormaps[MAXCOLORMAPS];

// bumped light from gun blasts
int extralight;

// r_fakesspal -- Fake split screen palette changes
CONL_StaticVar_t l_RFakeSSPal =
{
	CLVT_INTEGER, c_CVPVBoolean, CLVF_SAVE,
	"r_fakesspal", DSTR_CVHINT_RFAKESSPAL, CLVVT_STRING, "true",
	NULL
};

// r_transparency -- Enables Transparency Effects
CONL_StaticVar_t l_RTransparency =
{
	CLVT_INTEGER, c_CVPVBoolean, CLVF_SAVE,
	"r_transparency", DSTR_CVHINT_RTRANSPARENCY, CLVVT_STRING, "true",
	NULL
};

// g_CVPVViewSize -- View Size Limits
const CONL_VarPossibleValue_t c_CVPVViewSize[] =
{
	// End
	{3, "MINVAL"},
	{12, "MAXVAL"},
	{0, NULL},
};

// r_viewsize -- Size of view
CONL_StaticVar_t l_RViewSize =
{
	CLVT_INTEGER, c_CVPVViewSize, CLVF_SAVE,
	"r_viewsize", DSTR_CVHINT_RVIEWSIZE, CLVVT_STRING, "11",
	NULL
};

// added 16-6-98:splitscreen

//
// R_PointOnSide
// Traverse BSP (sub) tree,
//  check point against partition plane.
// Returns side 0 (front) or 1 (back).
//
int R_PointOnSide(fixed_t x, fixed_t y, node_t* node)
{
	fixed_t dx;
	fixed_t dy;
	fixed_t left;
	fixed_t right;
	
	if (!node->dx)
	{
		if (x <= node->x)
			return node->dy > 0;
			
		return node->dy < 0;
	}
	if (!node->dy)
	{
		if (y <= node->y)
			return node->dx < 0;
			
		return node->dx > 0;
	}
	
	dx = (x - node->x);
	dy = (y - node->y);
	
	// Try to quickly decide by looking at sign bits.
	if ((node->dy ^ node->dx ^ dx ^ dy) & 0x80000000)
	{
		if ((node->dy ^ dx) & 0x80000000)
		{
			// (left is negative)
			return 1;
		}
		return 0;
	}
	
	left = FixedMul(node->dy >> FRACBITS, dx);
	right = FixedMul(dy, node->dx >> FRACBITS);
	
	if (right < left)
	{
		// front side
		return 0;
	}
	// back side
	return 1;
}

int R_PointOnSegSide(fixed_t x, fixed_t y, seg_t* line)
{
	fixed_t lx;
	fixed_t ly;
	fixed_t ldx;
	fixed_t ldy;
	fixed_t dx;
	fixed_t dy;
	fixed_t left;
	fixed_t right;
	
	lx = line->v1->x;
	ly = line->v1->y;
	
	ldx = line->v2->x - lx;
	ldy = line->v2->y - ly;
	
	if (!ldx)
	{
		if (x <= lx)
			return ldy > 0;
			
		return ldy < 0;
	}
	if (!ldy)
	{
		if (y <= ly)
			return ldx < 0;
			
		return ldx > 0;
	}
	
	dx = (x - lx);
	dy = (y - ly);
	
	// Try to quickly decide by looking at sign bits.
	if ((ldy ^ ldx ^ dx ^ dy) & 0x80000000)
	{
		if ((ldy ^ dx) & 0x80000000)
		{
			// (left is negative)
			return 1;
		}
		return 0;
	}
	
	left = FixedMul(ldy >> FRACBITS, dx);
	right = FixedMul(dy, ldx >> FRACBITS);
	
	if (right < left)
	{
		// front side
		return 0;
	}
	// back side
	return 1;
}

//
// R_PointToAngle
// To get a global angle from cartesian coordinates,
//  the coordinates are flipped until they are in
//  the first octant of the coordinate system, then
//  the y (<=x) is scaled and divided by x to get a
//  tangent (slope) value which is looked up in the
//  tantoangle[] table.

//
angle_t R_PointToAngle2(fixed_t x2, fixed_t y2, fixed_t x1, fixed_t y1)
{
	x1 -= x2;
	y1 -= y2;
	
	if ((!x1) && (!y1))
		return 0;
		
	if (x1 >= 0)
	{
		// x >=0
		if (y1 >= 0)
		{
			// y>= 0
			
			if (x1 > y1)
			{
				// octant 0
				return tantoangle[SlopeDiv(y1, x1)];
			}
			else
			{
				// octant 1
				return ANG90 - 1 - tantoangle[SlopeDiv(x1, y1)];
			}
		}
		else
		{
			// y<0
			y1 = -y1;
			
			if (x1 > y1)
			{
				// octant 8
				return -tantoangle[SlopeDiv(y1, x1)];
			}
			else
			{
				// octant 7
				return ANG270 + tantoangle[SlopeDiv(x1, y1)];
			}
		}
	}
	else
	{
		// x<0
		x1 = -x1;
		
		if (y1 >= 0)
		{
			// y>= 0
			if (x1 > y1)
			{
				// octant 3
				return ANG180 - 1 - tantoangle[SlopeDiv(y1, x1)];
			}
			else
			{
				// octant 2
				return ANG90 + tantoangle[SlopeDiv(x1, y1)];
			}
		}
		else
		{
			// y<0
			y1 = -y1;
			
			if (x1 > y1)
			{
				// octant 4
				return ANG180 + tantoangle[SlopeDiv(y1, x1)];
			}
			else
			{
				// octant 5
				return ANG270 - 1 - tantoangle[SlopeDiv(x1, y1)];
			}
		}
	}
	return 0;
}

angle_t R_PointToAngle(fixed_t x, fixed_t y)
{
	return R_PointToAngle2(viewx, viewy, x, y);
}

fixed_t R_PointToDist2(fixed_t x2, fixed_t y2, fixed_t x1, fixed_t y1)
{
	int angle;
	fixed_t dx;
	fixed_t dy;
	fixed_t dist;
	
	dx = abs(x1 - x2);
	dy = abs(y1 - y2);
	
	if (dy > dx)
	{
		fixed_t temp;
		
		temp = dx;
		dx = dy;
		dy = temp;
	}
	if (dy == 0)
		return dx;
		
	angle = (tantoangle[FixedDiv(dy, dx) >> DBITS] + ANG90) >> ANGLETOFINESHIFT;
	
	// use as cosine
	dist = FixedDiv(dx, finesine[angle]);
	
	return dist;
}

//SoM: 3/27/2000: Little extra utility. Works in the same way as
//R_PointToAngle2
fixed_t R_PointToDist(fixed_t x, fixed_t y)
{
	return R_PointToDist2(viewx, viewy, x, y);
}

//
// R_InitPointToAngle
//
void R_InitPointToAngle(void)
{
	// UNUSED - now getting from tables.c
#if 0
	int i;
	long t;
	float f;
	
//
// slope (tangent) to angle lookup
//
	for (i = 0; i <= SLOPERANGE; i++)
	{
		f = atan((float)i / SLOPERANGE) / (3.141592657 * 2);
		t = 0xffffffff * f;
		tantoangle[i] = t;
	}
#endif
}

//
// R_ScaleFromGlobalAngle
// Returns the texture mapping scale
//  for the current line (horizontal span)
//  at the given angle.
// rw_distance must be calculated first.
//
//added:02-02-98:note: THIS IS USED ONLY FOR WALLS!
fixed_t R_ScaleFromGlobalAngle(angle_t visangle)
{
	// UNUSED
#if 0
	//added:02-02-98:note: I've tried this and it displays weird...
	fixed_t scale;
	fixed_t dist;
	fixed_t z;
	fixed_t sinv;
	fixed_t cosv;
	
	sinv = finesine[(visangle - rw_normalangle) >> ANGLETOFINESHIFT];
	dist = FixedDiv(rw_distance, sinv);
	cosv = finecosine[(viewangle - visangle) >> ANGLETOFINESHIFT];
	z = abs(FixedMul(dist, cosv));
	scale = FixedDiv(projection, z);
	return scale;
	
#else
	fixed_t scale;
	int anglea;
	int angleb;
	int sinea;
	int sineb;
	fixed_t num;
	int den;
	
	anglea = ANG90 + (visangle - viewangle);
	angleb = ANG90 + (visangle - rw_normalangle);
	
	// both sines are allways positive
	sinea = finesine[anglea >> ANGLETOFINESHIFT];
	sineb = finesine[angleb >> ANGLETOFINESHIFT];
	//added:02-02-98:now uses projectiony instead of projection for
	//               correct aspect ratio!
	num = FixedMul(projectiony, sineb) << detailshift;
	den = FixedMul(rw_distance, sinea);
	
	if (den > num >> 16)
	{
		scale = FixedDiv(num, den);
	
		if (scale > 64 * FRACUNIT)
			scale = 64 * FRACUNIT;
		else if (scale < 256)
			scale = 256;
	}
	else
		scale = 64 * FRACUNIT;
	
	return scale;
#endif
}

//
// R_InitTables
//
void R_InitTables(void)
{
	// UNUSED: now getting from tables.c
#if 0
	int i;
	float a;
	float fv;
	int t;
	
	// viewangle tangent table
	for (i = 0; i < FINEANGLES / 2; i++)
	{
		a = (i - FINEANGLES / 4 + 0.5) * PI * 2 / FINEANGLES;
		fv = FRACUNIT * tan(a);
		t = fv;
		finetangent[i] = t;
	}
	
	// finesine table
	for (i = 0; i < 5 * FINEANGLES / 4; i++)
	{
		// OPTIMIZE: mirror...
		a = (i + 0.5) * PI * 2 / FINEANGLES;
		t = FRACUNIT * sin(a);
		finesine[i] = t;
	}
#endif
	
}

//
// R_InitTextureMapping
//
void R_InitTextureMapping(void)
{
	int i;
	int x;
	int t;
	fixed_t focallength;
	
	// Use tangent table to generate viewangletox:
	//  viewangletox will give the next greatest x
	//  after the view angle.
	//
	// Calc focallength
	//  so FIELDOFVIEW angles covers SCREENWIDTH.
	focallength = FixedDiv(centerxfrac, finetangent[FINEANGLES / 4 +
	/*cv_fov.value */
	FIELDOFVIEW / 2]);
	
	for (i = 0; i < FINEANGLES / 2; i++)
	{
		if (finetangent[i] > FRACUNIT * 2)
			t = -1;
		else if (finetangent[i] < -FRACUNIT * 2)
			t = viewwidth + 1;
		else
		{
			t = FixedMul(finetangent[i], focallength);
			t = (centerxfrac - t + FRACUNIT - 1) >> FRACBITS;
			
			if (t < -1)
				t = -1;
			else if (t > viewwidth + 1)
				t = viewwidth + 1;
		}
		viewangletox[i] = t;
	}
	
	// Scan viewangletox[] to generate xtoviewangle[]:
	//  xtoviewangle will give the smallest view angle
	//  that maps to x.
	for (x = 0; x <= viewwidth; x++)
	{
		i = 0;
		while (viewangletox[i] > x)
			i++;
		xtoviewangle[x] = (i << ANGLETOFINESHIFT) - ANG90;
	}
	
	// Take out the fencepost cases from viewangletox.
	for (i = 0; i < FINEANGLES / 2; i++)
	{
		t = FixedMul(finetangent[i], focallength);
		t = centerx - t;
		
		if (viewangletox[i] == -1)
			viewangletox[i] = 0;
		else if (viewangletox[i] == viewwidth + 1)
			viewangletox[i] = viewwidth;
	}
	
	clipangle = xtoviewangle[0];
}

//
// R_InitLightTables
// Only inits the zlight table,
//  because the scalelight table changes with view size.
//
#define DISTMAP         2

void R_InitLightTables(void)
{
	int i;
	int j;
	int level;
	int startmap;
	int scale;
	
	// Calculate the light levels to use
	//  for each level / distance combination.
	for (i = 0; i < LIGHTLEVELS; i++)
	{
		startmap = ((LIGHTLEVELS - 1 - i) * 2) * NUMCOLORMAPS / LIGHTLEVELS;
		for (j = 0; j < MAXLIGHTZ; j++)
		{
			//added:02-02-98:use BASEVIDWIDTH, vid.width is not set already,
			// and it seems it needs to be calculated only once.
			scale = FixedDiv((BASEVIDWIDTH / 2 * FRACUNIT), (j + 1) << LIGHTZSHIFT);
			scale >>= LIGHTSCALESHIFT;
			level = startmap - scale / DISTMAP;
			
			if (level < 0)
				level = 0;
				
			if (level >= NUMCOLORMAPS)
				level = NUMCOLORMAPS - 1;
				
			zlight[i][j] = colormaps + level * 256;
		}
	}
}

//
// R_SetViewSize
// Do not really change anything here,
//  because it might be in the middle of a refresh.
// The change will take effect next refresh.
//
bool_t setsizeneeded;

void R_SetViewSize(void)
{
	setsizeneeded = true;
}

//
// R_ExecuteSetViewSize
//

// now uses screen variables cv_viewsize, cv_detaillevel
//
void R_ExecuteSetViewSize_DOOM(void)
{
	fixed_t cosadj;
	fixed_t dy;
	int i;
	int j;
	int level;
	int startmap;
	int st_overlay;
	
	int setdetail;
	
	int aspectx;				//added:02-02-98:for aspect ratio calc. below...
	int yextra;
	int32_t Junk;
	
	// GhostlyDeath <April 25, 2008> -- Fix crash when using server subsystem for the client
	if (!graphics_started)
		return;
		
	setsizeneeded = false;
	
	// no reduced view in splitscreen mode
	if (g_SplitScreen && l_RViewSize.Value->Int < 11)
		CONL_VarSetInt(&l_RViewSize, 11);
		
	setdetail = 0;
	
	// status bar overlay at viewsize 11
	st_overlay = (l_RViewSize.Value->Int == 11);
	
	stbarheight = ST_HEIGHT;
	
	if ((g_SplitScreen <= 0) && ST_ExSoloViewScaledSBar() || l_RViewSize.Value->Int >= 11)
		stbarheight *= vid.fdupy;
		
		
	//added 01-01-98: full screen view, without statusbar
	if (g_SplitScreen || l_RViewSize.Value->Int > 10 || ST_ExSoloViewTransSBar())
	{
		scaledviewwidth = vid.width;
		viewheight = vid.height;
	}
	else
	{
		//added 01-01-98: always a multiple of eight
		scaledviewwidth = (l_RViewSize.Value->Int * vid.width / 10) & ~7;
		//added:05-02-98: make viewheight multiple of 2 because sometimes
		//                a line is not refreshed by R_DrawViewBorder()
		viewheight = (l_RViewSize.Value->Int * (vid.height - stbarheight) / 10) & ~1;
	}
	
	// added 16-6-98:splitscreen
	if (g_SplitScreen >= 1)
		viewheight >>= 1;
	if (g_SplitScreen > 1)
		scaledviewwidth >>= 1;
		
	detailshift = setdetail;
	viewwidth = scaledviewwidth >> detailshift;
	
	centery = viewheight / 2;
	centerx = viewwidth / 2;
	centerxfrac = centerx << FRACBITS;
	centeryfrac = centery << FRACBITS;
	
	//added:01-02-98:aspect ratio is now correct, added an 'projectiony'
	//      since the scale is not always the same between horiz. & vert.
	projection = centerxfrac;
	projectiony = (((vid.height * centerx * BASEVIDWIDTH) / BASEVIDHEIGHT) / vid.width) << FRACBITS;
	
	//
	// no more low detail mode, it used to setup the right drawer routines
	// for either detail mode here
	//
	// if (!detailshift) ... else ...
	
	R_InitViewBuffer(scaledviewwidth, viewheight, yextra);
	
	R_InitTextureMapping();
	
	// psprite scales
	centerypsp = viewheight / 2;	//added:06-02-98:psprite pos for freelook
	
	pspritescale = (viewwidth << FRACBITS) / BASEVIDWIDTH;
	pspriteiscale = (BASEVIDWIDTH << FRACBITS) / viewwidth;	// x axis scale
	//added:02-02-98:now aspect ratio correct for psprites
	pspriteyscale = (((vid.height * viewwidth) / vid.width) << FRACBITS) / BASEVIDHEIGHT;
	
	// thing clipping
	for (i = 0; i < viewwidth; i++)
		screenheightarray[i] = viewheight;
		
	// setup sky scaling for old/new skies (uses pspriteyscale)
	R_SetSkyScale();
	
	// planes
	//added:02-02-98:now correct aspect ratio!
	aspectx = (((vid.height * centerx * BASEVIDWIDTH) / BASEVIDHEIGHT) / vid.width);
	
	// this is only used for planes rendering in software mode
	j = viewheight * 4;
	for (i = 0; i < j; i++)
	{
		//added:10-02-98:(i-centery) became (i-centery*2) and centery*2=viewheight
		dy = ((i - viewheight * 2) << FRACBITS) + FRACUNIT / 2;
		dy = abs(dy);
		yslopetab[i] = FixedDiv(aspectx * FRACUNIT, dy);
	}
	
	for (i = 0; i < viewwidth; i++)
	{
		cosadj = abs(finecosine[xtoviewangle[i] >> ANGLETOFINESHIFT]);
		distscale[i] = FixedDiv(FRACUNIT, cosadj);
	}
	
	// Calculate the light levels to use
	//  for each level / scale combination.
	for (i = 0; i < LIGHTLEVELS; i++)
	{
		startmap = ((LIGHTLEVELS - 1 - i) * 2) * NUMCOLORMAPS / LIGHTLEVELS;
		for (j = 0; j < MAXLIGHTSCALE; j++)
		{
			level = startmap - j * vid.width / (viewwidth << detailshift) / DISTMAP;
			
			if (level < 0)
				level = 0;
				
			if (level >= NUMCOLORMAPS)
				level = NUMCOLORMAPS - 1;
				
			scalelight[i][j] = colormaps + level * 256;
		}
	}
	
	st_recalc = true;
}

//
// R_Init
//

void R_Init(void)
{
	if (g_DedicatedServer)
		return;
		
	//added:24-01-98: screensize independent
	if (devparm)
		CONL_PrintF("\nR_InitData");
	R_InitData();
	
	if (devparm)
		CONL_PrintF("\nR_InitPointToAngle");
	R_InitPointToAngle();
	
	if (devparm)
		CONL_PrintF("\nR_InitTables");
	R_InitTables();
	
	R_InitViewBorder();
	
	R_SetViewSize();			// setsizeneeded is set true
	
	if (devparm)
		CONL_PrintF("\nR_InitPlanes");
	R_InitPlanes();
	
	//added:02-02-98: this is now done by SCR_Recalc() at the first mode set
	if (devparm)
		CONL_PrintF("\nR_InitLightTables");
	R_InitLightTables();
	
	if (devparm)
		CONL_PrintF("\nR_InitSkyMap");
	R_InitSkyMap();
	
	if (devparm)
		CONL_PrintF("\nR_InitTranslationsTables");
	R_InitTranslationTables();
	
	R_InitDrawNodes();
}

//
// R_PointInSubsector
//
subsector_t* R_PointInSubsector(fixed_t x, fixed_t y)
{
	node_t* node;
	int side;
	int nodenum;
	
	// single subsector is a special case
	if (!numnodes)
		return subsectors;
		
	nodenum = numnodes - 1;
	
	while (!(nodenum & NF_SUBSECTOR))
	{
		node = &nodes[nodenum];
		side = R_PointOnSide(x, y, node);
		nodenum = node->children[side];
	}
	
	return &subsectors[nodenum & ~NF_SUBSECTOR];
}

//
// R_IsPointInSubsector, same of above but return 0 if not in subsector
//
subsector_t* R_IsPointInSubsector(fixed_t x, fixed_t y)
{
	node_t* node;
	int side;
	int nodenum, i;
	subsector_t* ret;
	
	// single subsector is a special case
	if (!numnodes)
		return subsectors;
		
	nodenum = numnodes - 1;
	
	while (!(nodenum & NF_SUBSECTOR))
	{
		node = &nodes[nodenum];
		side = R_PointOnSide(x, y, node);
		nodenum = node->children[side];
	}
	
	ret = &subsectors[nodenum & ~NF_SUBSECTOR];
	for (i = 0; i < ret->numlines; i++)
	{
		if (R_PointOnSegSide(x, y, &segs[ret->firstline + i]))
			return 0;
	}
	
	return ret;
}

//
// R_SetupFrame
//

mobj_t* viewmobj;

void P_ResetCamera(player_t* player);

// WARNING : a should be unsigned but to add with 2048, it isn't !
#define AIMINGTODY(a) ((finetangent[(2048+(((int)a)>>ANGLETOFINESHIFT)) & FINEMASK]*160)>>FRACBITS)

void R_SetupFrame(player_t* player)
{
	int i;
	int fixedcolormap_setup;
	int dy = 0;					//added:10-02-98:
	
	extralight = player->extralight;
	
	if (player->ChaseCam && !player->camera.chase)
	{
		P_ResetCamera(player);
		player->camera.chase = true;
	}
	else if (!player->ChaseCam)
		player->camera.chase = false;
		
	if (script_camera_on)
	{
		viewmobj = script_camera.mo;
#ifdef PARANOIA
		if (!viewmobj)
			I_Error("no mobj for the camera");
#endif
		viewz = viewmobj->z;
		fixedcolormap_setup = player->camera.fixedcolormap;
		aimingangle = script_camera.aiming;
		viewangle = viewmobj->angle;
	}
	else
		if (player->camera.chase)
			// use outside cam view
		{
			viewmobj = player->camera.mo;
#ifdef PARANOIA
			if (!viewmobj)
				I_Error("no mobj for the camera");
#endif
			viewz = viewmobj->z + (viewmobj->height >> 1);
			fixedcolormap_setup = player->camera.fixedcolormap;
			aimingangle = player->camera.aiming;
			viewangle = viewmobj->angle;
		}
		else
			// use the player's eyes view
		{
			viewz = player->viewz;
			viewmobj = player->mo;
			fixedcolormap_setup = player->fixedcolormap;
			aimingangle = player->aiming;
			viewangle = viewmobj->angle + viewangleoffset;
			
#if 1
			if (!demoplayback && player->playerstate != PST_DEAD && P_XGSVal(PGS_COABSOLUTEANGLE))
				for (i = 0; i < g_SplitScreen + 1; i++)
					if (g_Splits[i].Active && playeringame[g_Splits[i].Console] && player == &players[g_Splits[i].Console] && g_Splits[i].Console == g_Splits[i].Display)
					{
						viewangle = localangle[i];
						aimingangle = localaiming[i];
					}
#endif
			
		}
		
#ifdef PARANOIA
	if (!viewmobj)
		I_Error("R_Setupframe : viewmobj null (player %d)", player - players);
#endif
	viewplayer = player;
	viewx = viewmobj->x;
	viewy = viewmobj->y;
	
	viewsin = finesine[viewangle >> ANGLETOFINESHIFT];
	viewcos = finecosine[viewangle >> ANGLETOFINESHIFT];
	
	sscount = 0;
	
	if (fixedcolormap_setup)
	{
		fixedcolormap = colormaps + fixedcolormap_setup * 256 * sizeof(lighttable_t);
		
		walllights = scalelightfixed;
		
		for (i = 0; i < MAXLIGHTSCALE; i++)
			scalelightfixed[i] = fixedcolormap;
	}
	else
		fixedcolormap = 0;
		
	//added:06-02-98:recalc necessary stuff for mouseaiming
	//               slopes are already calculated for the full
	//               possible view (which is 4*viewheight).
	// clip it in the case we are looking a hardware 90 full aiming
	// (lmps, nework and use F12...)
	G_ClipAimingPitch(&aimingangle);
	
	if (g_SplitScreen != 1)
		dy = AIMINGTODY(aimingangle) * viewheight / BASEVIDHEIGHT;
	else
		dy = AIMINGTODY(aimingangle) * viewheight * 2 / BASEVIDHEIGHT;
		
	yslope = &yslopetab[(3 * viewheight / 2) - dy];
	centery = (viewheight / 2) + dy;
	centeryfrac = centery << FRACBITS;
	
	validcount++;
}

// ================
// R_RenderView
// ================

//                     FAB NOTE FOR WIN32 PORT !! I'm not finished already,
// but I suspect network may have problems with the video buffer being locked
// for all duration of rendering, and being released only once at the end..
// I mean, there is a win16lock() or something that lasts all the rendering,
// so maybe we should release screen lock before each netupdate below..?

void R_DrawPlayerSprites(void);

void R_RenderPlayerViewEx_DOOM(player_t* player, int quarter)
{
	register uint8_t* dest;
	int x, y, a, b, c, d;
	uint8_t* ExtraMap;
	
	/* Check */
	if (!player || (player && !player->mo))
		return;
	
	R_SetupFrame(player);
	
	// Clear buffers.
	R_ClearClipSegs();
	R_ClearDrawSegs();
	R_ClearPlanes(player);		//needs player for waterheight in occupied sector
	//R_ClearPortals ();
	R_ClearSprites();
	
	// GhostlyDeath <May 6, 2012> -- Network Update
	//D_SyncNetUpdate();
	
	// The head node is the last node output.
	
	R_RenderBSPNode(numnodes - 1);
	
	//D_SyncNetUpdate();
	
	//R_DrawPortals ();
	R_DrawPlanes();
	
	// draw mid texture and sprite
	// SoM: And now 3D floors/sides!
	R_DrawMasked();
	
	// draw the psprites on top of everything
	//  but does not draw on side views
	if (!viewangleoffset && !player->camera.chase && (!player->ProfileEx || (player->ProfileEx && player->ProfileEx->DrawPSprites)) && !script_camera_on)
		R_DrawPlayerSprites();
		
	// GhostlyDeath -- warp the view
	// ylookup is the row and columnofs is the column in the row
#if 0
	if (((!cv_chasecam.value && player->mo && player->mo->eflags & MF_UNDERWATER) ||
	(cv_chasecam.value && camera.mo && camera.mo->eflags & MF_UNDERWATER)) && (g_SplitScreen <= 0))
	{
		for (y = 0; y < viewheight; y++)
		{
			a = (gametic % 32) - 16;
			
			if (a < 0)
				c = 0;
			else
				c = 1;
				
			if (c)
			{
				a += 2;
				if (a > 16)
					c = 0;
			}
			else
			{
				a -= 2;
				if (a < -16)
					c = 1;
			}
			
			for (x = 0; x < viewwidth; x++)
			{
				//a = ((gametic) % (viewwidth / 16)) - (viewheight / 32);// + (x % ((viewwidth / 8)) / 2);
				b = a;
				if (b < 0)
					b += (gametic + (x >> 2)) % 16;
				else
					b -= (gametic + (x >> 2)) % 16;
				if (b < -16)
					b = -16;
				else if (b > 16)
					b = 16;
					
				while (b + y < 0)
					b++;
				while (b + y > viewheight - (viewheight / 32))
					b--;
					
				if (ylookup[y + b])
				{
					if (b < 0)
						*(ylookup[y + b] + columnofs[x]) = *(ylookup[y] + columnofs[x]);
					else
						*(ylookup[y] + columnofs[x]) = *(ylookup[y + b] + columnofs[x]);
				}
			}
		}
	}
#endif
	
	// GhostlyDeath <May 6, 2012> -- Network Update
	//D_SyncNetUpdate();
	
	//player->mo->flags &= ~MF_NOSECTOR;	// don't show self (uninit) clientprediction code
	
	// GhostlyDeath <May 22, 2012> -- Fake palette hacking
	if (g_SplitScreen > 0 && l_RFakeSSPal.Value[0].Int)
	{
		// Get the map
		if (player->PalChoice > 0)
			ExtraMap = V_GetPaletteMapped(player->PalChoice);
		else
			ExtraMap = NULL;	// If non-default, don't waste time!
		
		// If the map was obtained, apply it
		if (ExtraMap)
			V_DrawColorMapEx(VEX_NOSCALESTART | VEX_NOSCALESCREEN, ExtraMap, viewwindowx, viewwindowy, viewwindowx + viewwidth, viewwindowy + viewheight);
	}
}

void R_RenderPlayerView_DOOM(player_t* player, const size_t a_Screen)
{
	R_RenderPlayerViewEx_DOOM(player, 0);
}

// =========================================================================
//                    ENGINE COMMANDS & VARS
// =========================================================================

/* R_ExecuteSetViewSize_Nill() -- Does nothing to avoid crash */
static void R_ExecuteSetViewSize_Nill(void)
{
}

void (*R_ExecuteSetViewSize)(void) = R_ExecuteSetViewSize_Nill;
void (*R_RenderPlayerView)(player_t* player, const size_t a_Screen) = NULL;

// c_CVPVRRenderer -- Renderer to use
static const CONL_VarPossibleValue_t c_CVPVRRenderer[] =
{
	{0, "Legacy"},
	{1, "ReMooD"},
	{2, "Heretic"},
	{3, "Snow"},
	
	// End
	{0, NULL},
};

/* RS_RRendererChange() -- Renderer Value Changed */
static bool_t RS_RRendererChange(CONL_ConVariable_t* const a_Var, CONL_StaticVar_t* const a_StaticVar)
{
	bool_t ForceReMooD;
	
	/* Force ReMooD Renderer? */
	ForceReMooD = false;
	if (VHW_GetMode() != VHWMODE_IDXSOFT)
		ForceReMooD = true;
	
	/* Notice */
	CONL_PrintF("Selecting Renderer ");
	
	/* Legacy? */
	/*if (!ForceReMooD && a_StaticVar->Value[0].Int == 0)*/
	{
		CONL_PrintF("Legacy");
		
		R_ExecuteSetViewSize = R_ExecuteSetViewSize_DOOM;
		R_RenderPlayerView = R_RenderPlayerView_DOOM;
	}
	
#if 0
	/* ReMooD? */
	else if (ForceReMooD || a_StaticVar->Value[0].Int == 1)
	{
		CONL_PrintF("ReMooD");
		
		R_ExecuteSetViewSize = R_ExecuteSetViewSize_REMOOD;
		R_RenderPlayerView = R_RenderPlayerView_REMOOD;
	}
	
	/* Heretic? */
	else if (!ForceReMooD && a_StaticVar->Value[0].Int == 2)
	{
		CONL_PrintF("Heretic");
		
		R_ExecuteSetViewSize = R_ExecuteSetViewSize_HERETIC;
		R_RenderPlayerView = R_RenderPlayerView_HERETIC;
	}
	
	/* Snow? */
	else if (!ForceReMooD && a_StaticVar->Value[0].Int == 3)
	{
		CONL_PrintF("Snow");
		
		R_ExecuteSetViewSize = SN_ViewResize;
		R_RenderPlayerView = SN_RenderView;
	}
#endif
	
	/* Notice */
	CONL_PrintF("\n");
	
	/* Success! */
	return true;
}

// r_renderer -- Which renderer to use
CONL_StaticVar_t l_RRenderer =
{
	CLVT_INTEGER, c_CVPVRRenderer, CLVF_SAVE | CLVF_NOISY,
	"r_renderer", DSTR_CVHINT_RRENDERER, CLVVT_STRING, "Legacy",
	RS_RRendererChange
};

extern CONL_StaticVar_t l_RDrawSplats;
extern CONL_StaticVar_t l_RMaxSplats;

/* R_RegisterEngineStuff() -- Registers rendering stuff */
void R_RegisterEngineStuff(void)
{
	/* Base Render Init */
	R_ExecuteSetViewSize = R_ExecuteSetViewSize_DOOM;
	R_RenderPlayerView = R_RenderPlayerView_DOOM;	
	/********************/
	
	// GhostlyDeath <May 22, 2012> -- Fake split screen palettes
	CONL_VarRegister(&l_RFakeSSPal);
	CONL_VarRegister(&l_RRenderer);
	CONL_VarRegister(&l_RViewSize);
	CONL_VarRegister(&l_RDrawSplats);
	CONL_VarRegister(&l_RMaxSplats);
	CONL_VarRegister(&l_RTransparency);
}

