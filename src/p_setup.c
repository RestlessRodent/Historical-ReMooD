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
//      Do all the WAD I/O, get map description,
//             set up initial state and misc. LUTs.

#include "doomdef.h"
#include "c_lib.h"
#include "d_main.h"
#include "g_game.h"

#include "p_local.h"
#include "p_setup.h"
#include "p_spec.h"

#include "i_sound.h"			//for I_PlayCD()..

#include "r_data.h"
#include "r_things.h"
#include "r_sky.h"

#include "s_sound.h"
#include "st_stuff.h"
#include "w_wad.h"
#include "z_zone.h"
#include "r_splats.h"
#include "p_info.h"
#include "t_func.h"
#include "t_script.h"

#include "hu_stuff.h"
#include "console.h"
#include "t_comp.h"

#ifdef _WIN32
#include "malloc.h"
#include "math.h"
#endif

#include "d_net.h"

#include "command.h"
#include "p_demcmp.h"
#include "b_bot.h"

//#define COOLFLIPLEVELS

//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//
bool_t newlevel = false;
bool_t doom1level = false;		// doom 1 level running under doom 2
char* levelmapname = NULL;

int numvertexes;
vertex_t* vertexes;

int numsegs;
seg_t* segs;

int numsectors;
sector_t* sectors;

int numsubsectors;
subsector_t* subsectors;

int numnodes;
node_t* nodes;

int numlines;
line_t* lines;

int numsides;
side_t* sides;

int nummapthings;
mapthing_t* mapthings;

/*
typedef struct mapdata_s {
    int             numvertexes;
    vertex_t*       vertexes;
    int             numsegs;
    seg_t*          segs;
    int             numsectors;
    sector_t*       sectors;
    int             numsubsectors;
    subsector_t*    subsectors;
    int             numnodes;
    node_t*         nodes;
    int             numlines;
    line_t*         lines;
    int             numsides;
    side_t*         sides;
} mapdata_t;
*/

// BLOCKMAP
// Created from axis aligned bounding box
// of the map, a rectangular array of
// blocks of size ...
// Used to speed up collision detection
// by spatial subdivision in 2D.
//
// Blockmap size.
int bmapwidth;
int bmapheight;					// size in mapblocks

long* blockmap;					// int for large maps

// offsets in blockmap are from here
long* blockmaplump;				// Big blockmap SSNTails

// origin of block map
fixed_t bmaporgx;
fixed_t bmaporgy;

// for thing chains
mobj_t** blocklinks;

// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without special effect, this could be
//  used as a PVS lookup as well.
//
uint8_t* rejectmatrix;

// Maintain single and multi player starting spots.
mapthing_t* deathmatchstarts[MAX_DM_STARTS];
int numdmstarts;

//mapthing_t**    deathmatch_p;
mapthing_t* playerstarts[MAXPLAYERS];

//
// P_LoadVertexes
//
void P_LoadVertexes(int lump)
{
	uint8_t* data;
	int i;
	mapvertex_t* ml;
	vertex_t* li;
	
	// Determine number of lumps:
	//  total lump length / vertex record length.
	numvertexes = W_LumpLength(lump) / sizeof(mapvertex_t);
	
	// Allocate zone memory for buffer.
	vertexes = Z_Malloc(numvertexes * sizeof(vertex_t), PU_LEVEL, 0);
	
	// Load data into cache.
	data = W_CacheLumpNum(lump, PU_STATIC);
	
	ml = (mapvertex_t*) data;
	li = vertexes;
	
	// Copy and convert vertex coordinates,
	// internal representation as fixed.
	for (i = 0; i < numvertexes; i++, li++, ml++)
	{
		li->x = LittleSwapInt16(ml->x) << FRACBITS;
		li->y = LittleSwapInt16(ml->y) << FRACBITS;
	}
	
	// Free buffer memory.
	Z_Free(data);
}

//
// Computes the line length in frac units, the glide render needs this
//
#define crapmul (1.0f / 65536.0f)

float P_SegLength(seg_t* seg)
{
	double dx, dy;
	
	// make a vector (start at origin)
	dx = (seg->v2->x - seg->v1->x) * crapmul;
	dy = (seg->v2->y - seg->v1->y) * crapmul;
	
	return sqrt(dx * dx + dy * dy) * FRACUNIT;
}

//
// P_LoadSegs
//
void P_LoadSegs(int lump)
{
	uint8_t* data;
	int i;
	mapseg_t* ml;
	seg_t* li;
	line_t* ldef;
	int linedef;
	int side;
	
	numsegs = W_LumpLength(lump) / sizeof(mapseg_t);
	segs = Z_Malloc(numsegs * sizeof(seg_t), PU_LEVEL, 0);
	memset(segs, 0, numsegs * sizeof(seg_t));
	data = W_CacheLumpNum(lump, PU_STATIC);
	
	ml = (mapseg_t*) data;
	li = segs;
	for (i = 0; i < numsegs; i++, li++, ml++)
	{
		li->v1 = &vertexes[LittleSwapInt16(ml->v1)];
		li->v2 = &vertexes[LittleSwapInt16(ml->v2)];
		li->angle = (LittleSwapInt16(ml->angle)) << 16;
		li->offset = (LittleSwapInt16(ml->offset)) << 16;
		linedef = LittleSwapInt16(ml->linedef);
		ldef = &lines[linedef];
		li->linedef = ldef;
		li->side = side = LittleSwapInt16(ml->side);
		li->sidedef = &sides[ldef->sidenum[side]];
		li->frontsector = sides[ldef->sidenum[side]].sector;
		if (ldef->flags & ML_TWOSIDED)
			li->backsector = sides[ldef->sidenum[side ^ 1]].sector;
		else
			li->backsector = 0;
			
		li->numlights = 0;
		li->rlights = NULL;
	}
	
	Z_Free(data);
}

//
// P_LoadSubsectors
//
void P_LoadSubsectors(int lump)
{
	uint8_t* data;
	int i;
	mapsubsector_t* ms;
	subsector_t* ss;
	
	numsubsectors = W_LumpLength(lump) / sizeof(mapsubsector_t);
	subsectors = Z_Malloc(numsubsectors * sizeof(subsector_t), PU_LEVEL, 0);
	data = W_CacheLumpNum(lump, PU_STATIC);
	
	ms = (mapsubsector_t*) data;
	memset(subsectors, 0, numsubsectors * sizeof(subsector_t));
	ss = subsectors;
	
	for (i = 0; i < numsubsectors; i++, ss++, ms++)
	{
		ss->numlines = LittleSwapInt16(ms->numsegs);
		ss->firstline = LittleSwapInt16(ms->firstseg);
	}
	
	Z_Free(data);
}

//
// P_LoadSectors
//

//
// levelflats
//
#define MAXLEVELFLATS   512

int numlevelflats;
levelflat_t* levelflats;

//SoM: Other files want this info.
int P_PrecacheLevelFlats()
{
	int flatmemory = 0;
	int i;
	int lump;
	
	//SoM: 4/18/2000: New flat code to make use of levelflats.
	for (i = 0; i < numlevelflats; i++)
	{
		lump = levelflats[i].lumpnum;
		if (devparm)
			flatmemory += W_LumpLength(lump);
		R_GetFlat(lump);
	}
	return flatmemory;
}

int P_FlatNumForName(char* flatname)
{
	return P_AddLevelFlat(flatname, levelflats);
}

// help function for P_LoadSectors, find a flat in the active wad files,
// allocate an id for it, and set the levelflat (to speedup search)
//
int P_AddLevelFlat(char* flatname, levelflat_t* levelflat)
{
	union
	{
		char s[9];
		int x[2];
	} name8;
	
	int i;
	uint32_t v1, v2;
	
	strncpy(name8.s, flatname, 8);	// make it two ints for fast compares
	name8.s[8] = 0;				// in case the name was a fill 8 chars
	C_strupr(name8.s);			// case insensitive
	v1 = name8.x[0];
	v2 = name8.x[1];
	
	// TODO - GhostlyDeath: Apparently the already scanning stuff doesn't work!
	//
	//  first scan through the already found flats
	//
	for (i = 0; i < numlevelflats; i++, levelflat++)
	{
		if (*(uint32_t*)levelflat->name == v1 && *(uint32_t*)&levelflat->name[4] == v2)
		{
			break;
		}
	}
	
	// that flat was already found in the level, return the id
	if (i == numlevelflats)
	{
		// store the name
		*((size_t*) levelflat->name) = v1;
		*((size_t*) & levelflat->name[4]) = v2;
		
		// store the flat lump number
		levelflat->lumpnum = R_GetFlatNumForName(flatname);
		
		numlevelflats++;
		
		if (numlevelflats >= MAXLEVELFLATS)
			I_Error("P_LoadSectors: too many flats in level\n");
	}
	// level flat id
	return i;
}

// SoM: Do I really need to comment this?
char* P_FlatNameForNum(int num)
{
	if (num < 0 || num > numlevelflats)
		I_Error("P_FlatNameForNum: Invalid flatnum\n");
		
	return Z_Strdup(va("%.8s", levelflats[num].name), PU_STATIC, 0);
}

void P_LoadSectors(int lump)
{
	uint8_t* data;
	int i, j, Temp;
	mapsector_t* ms;
	sector_t* ss;
	char newflat1[10];
	char newflat2[10];
	char* buf;
	const char* FP;
	const char* CP;
	
	levelflat_t* foundflats;
	
	numsectors = W_LumpLength(lump) / sizeof(mapsector_t);
	sectors = Z_Malloc(numsectors * sizeof(sector_t), PU_LEVEL, &sectors);
	memset(sectors, 0, numsectors * sizeof(sector_t));
	data = W_CacheLumpNum(lump, PU_STATIC);
	
	//Fab:FIXME: allocate for whatever number of flats
	//           512 different flats per level should be plenty
	foundflats = Z_Malloc(sizeof(levelflat_t) * MAXLEVELFLATS, PU_LEVEL, &foundflats);
	memset(foundflats, 0, sizeof(levelflat_t) * MAXLEVELFLATS);
	
	numlevelflats = 0;
	
	ms = (mapsector_t*) data;
	ss = sectors;
	for (i = 0; i < numsectors; i++, ss++, ms++)
	{
		ss->floorheight = LittleSwapInt16(ms->floorheight) << FRACBITS;
		ss->ceilingheight = LittleSwapInt16(ms->ceilingheight) << FRACBITS;	

#ifdef COOLFLIPLEVELS
		Temp = ss->ceilingheight - ss->floorheight;
		ss->ceilingheight = -ss->floorheight;
		ss->floorheight = ss->ceilingheight - Temp;
#endif
		
		//
		//  flats
		//
#ifdef COOLFLIPLEVELS
		FP = ms->ceilingpic;
		CP = ms->floorpic;
#else
		FP = ms->floorpic;
		CP = ms->ceilingpic;
#endif
		
		if (strncasecmp(FP, "FWATER", 6) == 0 || strncasecmp(FP, "FLTWAWA1", 8) == 0 || strncasecmp(FP, "FLTFLWW1", 8) == 0)
			ss->floortype = FLOOR_WATER;
		else if (strncasecmp(FP, "FLTLAVA1", 8) == 0 || strncasecmp(FP, "FLATHUH1", 8) == 0)
			ss->floortype = FLOOR_LAVA;
		else if (strncasecmp(FP, "FLTSLUD1", 8) == 0)
			ss->floortype = FLOOR_SLUDGE;
		else
			ss->floortype = FLOOR_SOLID;
			
		for (j = 0; j < 8; j++)
		{
			newflat1[j] = 0;
			newflat2[j] = 0;
		}
		
		/*#ifdef _MSC_VER >= 1400
		   _strncpy(newflat1, 8, ms->floorpic); //snprintf(newflat1, 9, "%s", ms->floorpic);
		   _strncpy(newflat1, 8, ms->floorpic); //snprintf(newflat2, 9, "%s", ms->ceilingpic);
		   #elif defined(__GNUC__)
		   strncpy(newflat1, 8, ms->floorpic); //snprintf(newflat1, 9, "%s", ms->floorpic);
		   strncpy(newflat1, 8, ms->floorpic); //snprintf(newflat2, 9, "%s", ms->ceilingpic);
		   #else */
		// Insecure "Safe" strncpy -- Self implementation
		buf = FP;
		j = 0;
		while (*buf && j < 8)
		{
			newflat1[j] = *buf;
			j++;
			buf++;
		}
		newflat1[8] = 0;
		
		buf = CP;
		j = 0;
		while (*buf && j < 8)
		{
			newflat2[j] = *buf;
			j++;
			buf++;
		}
		newflat2[8] = 0;
//#endif

		ss->floorpic = P_AddLevelFlat(newflat1, foundflats);
		ss->ceilingpic = P_AddLevelFlat(newflat2, foundflats);
		
		ss->lightlevel = LittleSwapInt16(ms->lightlevel);
		ss->special = LittleSwapInt16(ms->special);
		ss->tag = LittleSwapInt16(ms->tag);
		
		//added:31-03-98: quick hack to test water with DCK
		
		/*        if (ss->tag < 0)
		   CONL_PrintF("Level uses dck-water-hack\n"); */
		
		ss->thinglist = NULL;
		ss->touching_thinglist = NULL;	//SoM: 4/7/2000
		
		ss->stairlock = 0;
		ss->nextsec = -1;
		ss->prevsec = -1;
		
		ss->heightsec = -1;		//SoM: 3/17/2000: This causes some real problems
		ss->altheightsec = 0;	//SoM: 3/20/2000
		ss->floorlightsec = -1;
		ss->ceilinglightsec = -1;
		ss->ffloors = NULL;
		ss->lightlist = NULL;
		ss->numlights = 0;
		ss->attached = NULL;
		ss->numattached = 0;
		ss->moved = true;
		ss->floor_xoffs = ss->ceiling_xoffs = ss->floor_yoffs = ss->ceiling_yoffs = 0;
		ss->bottommap = ss->midmap = ss->topmap = -1;
		
		// ----- for special tricks with HW renderer -----
		ss->pseudoSector = false;
		ss->virtualFloor = false;
		ss->virtualCeiling = false;
		ss->sectorLines = NULL;
		ss->stackList = NULL;
		ss->lineoutLength = -1.0;
		// ----- end special tricks -----
		
	}
	
	Z_Free(data);
	
	// whoa! there is usually no more than 25 different flats used per level!!
	//CONL_PrintF ("%d flats found\n", numlevelflats);
	
	// set the sky flat num
	skyflatnum = P_AddLevelFlat("F_SKY1", foundflats);
	
	// copy table for global usage
	levelflats = Z_Malloc(numlevelflats * sizeof(levelflat_t), PU_LEVEL, 0);
	memcpy(levelflats, foundflats, numlevelflats * sizeof(levelflat_t));
	
	// search for animated flats and set up
	P_SetupLevelFlatAnims();
}

//
// P_LoadNodes
//
void P_LoadNodes(int lump)
{
	uint8_t* data;
	int i;
	int j;
	int k;
	mapnode_t* mn;
	node_t* no;
	
	numnodes = W_LumpLength(lump) / sizeof(mapnode_t);
	nodes = Z_Malloc(numnodes * sizeof(node_t), PU_LEVEL, 0);
	data = W_CacheLumpNum(lump, PU_STATIC);
	
	mn = (mapnode_t*) data;
	no = nodes;
	
	for (i = 0; i < numnodes; i++, no++, mn++)
	{
		no->x = LittleSwapInt16(mn->x) << FRACBITS;
		no->y = LittleSwapInt16(mn->y) << FRACBITS;
		no->dx = LittleSwapInt16(mn->dx) << FRACBITS;
		no->dy = LittleSwapInt16(mn->dy) << FRACBITS;
		for (j = 0; j < 2; j++)
		{
			no->children[j] = LittleSwapInt16(mn->children[j]);
			for (k = 0; k < 4; k++)
				no->bbox[j][k] = LittleSwapInt16(mn->bbox[j][k]) << FRACBITS;
		}
	}
	
	Z_Free(data);
}

//
// P_LoadThings
//
void P_LoadThings(int lump)
{
	int i;
	mapthing_t* mt;
	bool_t spawn;
	int16_t* data, *datastart;
	
	data = datastart = W_CacheLumpNum(lump, PU_LEVEL);
	nummapthings = W_LumpLength(lump) / (5 * sizeof(short));
	mapthings = Z_Malloc(nummapthings * sizeof(mapthing_t), PU_LEVEL, NULL);
	
	//SoM: Because I put a new member into the mapthing_t for use with
	//fragglescript, the format has changed and things won't load correctly
	//using the old method.
	
	mt = mapthings;
	for (i = 0; i < nummapthings; i++, mt++)
	{
		spawn = true;
		
		// Do spawn all other stuff.
		// SoM: Do this first so all the mapthing slots are filled!
		mt->x = *data;
		data++;
		mt->y = *data;
		data++;
		mt->angle = *data;
		data++;
		mt->type = *data;
		data++;
		mt->options = *data;
		data++;
		mt->mobj = NULL;		//SoM:
		
		P_SpawnMapThing(mt);
	}
	
	Z_Free(datastart);
}

/* P_LoadThingsHexen() -- Load hexen thing data */
void P_LoadThingsHexen(int lump)
{
	int i;
	mapthing_t* mt;
	bool_t spawn;
	int16_t* data, *datastart;
	
	data = datastart = W_CacheLumpNum(lump, PU_LEVEL);
	nummapthings = W_LumpLength(lump) / (20 * sizeof(short));
	mapthings = Z_Malloc(nummapthings * sizeof(mapthing_t), PU_LEVEL, NULL);
	
	//SoM: Because I put a new member into the mapthing_t for use with
	//fragglescript, the format has changed and things won't load correctly
	//using the old method.
	
	mt = mapthings;
	for (i = 0; i < nummapthings; i++, mt++)
	{
		spawn = true;
		
		// Do spawn all other stuff.
		// SoM: Do this first so all the mapthing slots are filled!
		data++;					// hexen ID
		mt->x = *data;
		data++;
		mt->y = *data;
		data++;
		mt->z = *data;
		data++;
		mt->angle = *data;
		data++;
		mt->type = *data;
		data++;
		mt->options = *data;
		data++;
		
		// Ignore hexen specials
		data += 3;
		
		mt->mobj = NULL;		//SoM:
		
		P_SpawnMapThing(mt);
	}
	
	Z_Free(datastart);
}

//
// P_LoadLineDefs
// Also counts secret lines for intermissions.
//
void P_LoadLineDefs(int lump)
{
	uint8_t* data;
	int i;
	maplinedef_t* mld;
	line_t* ld;
	vertex_t* v1;
	vertex_t* v2;
	
	numlines = W_LumpLength(lump) / sizeof(maplinedef_t);
	lines = Z_Malloc(numlines * sizeof(line_t), PU_LEVEL, 0);
	memset(lines, 0, numlines * sizeof(line_t));
	data = W_CacheLumpNum(lump, PU_STATIC);
	
	mld = (maplinedef_t*) data;
	ld = lines;
	for (i = 0; i < numlines; i++, mld++, ld++)
	{
		ld->flags = LittleSwapInt16(mld->flags);
		ld->special = LittleSwapInt16(mld->special);
		ld->tag = LittleSwapInt16(mld->tag);
		v1 = ld->v1 = &vertexes[LittleSwapInt16(mld->v1)];
		v2 = ld->v2 = &vertexes[LittleSwapInt16(mld->v2)];
		ld->dx = v2->x - v1->x;
		ld->dy = v2->y - v1->y;
		
		if (!ld->dx)
			ld->slopetype = ST_VERTICAL;
		else if (!ld->dy)
			ld->slopetype = ST_HORIZONTAL;
		else
		{
			if (FixedDiv(ld->dy, ld->dx) > 0)
				ld->slopetype = ST_POSITIVE;
			else
				ld->slopetype = ST_NEGATIVE;
		}
		
		if (v1->x < v2->x)
		{
			ld->bbox[BOXLEFT] = v1->x;
			ld->bbox[BOXRIGHT] = v2->x;
		}
		else
		{
			ld->bbox[BOXLEFT] = v2->x;
			ld->bbox[BOXRIGHT] = v1->x;
		}
		
		if (v1->y < v2->y)
		{
			ld->bbox[BOXBOTTOM] = v1->y;
			ld->bbox[BOXTOP] = v2->y;
		}
		else
		{
			ld->bbox[BOXBOTTOM] = v2->y;
			ld->bbox[BOXTOP] = v1->y;
		}
		
		ld->sidenum[0] = LittleSwapInt16(mld->sidenum[0]);
		ld->sidenum[1] = LittleSwapInt16(mld->sidenum[1]);
		
		if (ld->sidenum[0] != -1 && ld->special)
			sides[ld->sidenum[0]].special = ld->special;
			
	}
	
	Z_Free(data);
}

/* P_LoadLineDefsHexen() -- Load hexen data */
void P_LoadLineDefsHexen(int lump)
{
	uint8_t* data;
	int i;
	HexenMapLineDef_t* mld;
	line_t* ld;
	vertex_t* v1;
	vertex_t* v2;
	
	numlines = W_LumpLength(lump) / sizeof(HexenMapLineDef_t);
	lines = Z_Malloc(numlines * sizeof(line_t), PU_LEVEL, 0);
	memset(lines, 0, numlines * sizeof(line_t));
	data = W_CacheLumpNum(lump, PU_STATIC);
	
	mld = (HexenMapLineDef_t*) data;
	ld = lines;
	for (i = 0; i < numlines; i++, mld++, ld++)
	{
		ld->flags = LittleSwapInt16(mld->flags);
		
		// Ignore hexen specials (not supported, yet!)
		ld->special = 0;		//LittleSwapInt16(mld->special);
		ld->tag = 0;			//LittleSwapInt16(mld->tag);
		
		v1 = ld->v1 = &vertexes[LittleSwapInt16(mld->v1)];
		v2 = ld->v2 = &vertexes[LittleSwapInt16(mld->v2)];
		ld->dx = v2->x - v1->x;
		ld->dy = v2->y - v1->y;
		
		if (!ld->dx)
			ld->slopetype = ST_VERTICAL;
		else if (!ld->dy)
			ld->slopetype = ST_HORIZONTAL;
		else
		{
			if (FixedDiv(ld->dy, ld->dx) > 0)
				ld->slopetype = ST_POSITIVE;
			else
				ld->slopetype = ST_NEGATIVE;
		}
		
		if (v1->x < v2->x)
		{
			ld->bbox[BOXLEFT] = v1->x;
			ld->bbox[BOXRIGHT] = v2->x;
		}
		else
		{
			ld->bbox[BOXLEFT] = v2->x;
			ld->bbox[BOXRIGHT] = v1->x;
		}
		
		if (v1->y < v2->y)
		{
			ld->bbox[BOXBOTTOM] = v1->y;
			ld->bbox[BOXTOP] = v2->y;
		}
		else
		{
			ld->bbox[BOXBOTTOM] = v2->y;
			ld->bbox[BOXTOP] = v1->y;
		}
		
		ld->sidenum[0] = LittleSwapInt16(mld->sidenum[0]);
		ld->sidenum[1] = LittleSwapInt16(mld->sidenum[1]);
		
		if (ld->sidenum[0] != -1 && ld->special)
			sides[ld->sidenum[0]].special = ld->special;
			
	}
	
	Z_Free(data);
}

void P_LoadLineDefs2()
{
	int i;
	line_t* ld = lines;
	
	for (i = 0; i < numlines; i++, ld++)
	{
		if (ld->sidenum[0] != -1)
			ld->frontsector = sides[ld->sidenum[0]].sector;
		else
			ld->frontsector = 0;
			
		if (ld->sidenum[1] != -1)
			ld->backsector = sides[ld->sidenum[1]].sector;
		else
			ld->backsector = 0;
	}
}

//
// P_LoadSideDefs
//

/*void P_LoadSideDefs (int lump)
{
    uint8_t*               data;
    int                 i;
    mapsidedef_t*       msd;
    side_t*             sd;

    numsides = W_LumpLength (lump) / sizeof(mapsidedef_t);
    sides = Z_Malloc (numsides*sizeof(side_t),PU_LEVEL,0);
    memset (sides, 0, numsides*sizeof(side_t));
    data = W_CacheLumpNum (lump,PU_STATIC);

    msd = (mapsidedef_t *)data;
    sd = sides;
    for (i=0 ; i<numsides ; i++, msd++, sd++)
    {
        sd->textureoffset = LittleSwapInt16(msd->textureoffset)<<FRACBITS;
        sd->rowoffset = LittleSwapInt16(msd->rowoffset)<<FRACBITS;
        sd->toptexture = R_TextureNumForName(msd->toptexture);
        sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
        sd->midtexture = R_TextureNumForName(msd->midtexture);

        sd->sector = &sectors[LittleSwapInt16(msd->sector)];
    }

    Z_Free (data);
}*/

void P_LoadSideDefs(int lump)
{
	numsides = W_LumpLength(lump) / sizeof(mapsidedef_t);
	sides = Z_Malloc(numsides * sizeof(side_t), PU_LEVEL, 0);
	memset(sides, 0, numsides * sizeof(side_t));
}

// SoM: 3/22/2000: Delay loading texture names until after loaded linedefs.

//Hurdler: 04/04/2000: proto added
int R_ColormapNumForName(char* name);

void P_LoadSideDefs2(int lump)
{
	uint8_t* data = W_CacheLumpNum(lump, PU_STATIC);
	int i, Temp;
	int num;
	int mapnum;
	
	for (i = 0; i < numsides; i++)
	{
		register mapsidedef_t* msd = (mapsidedef_t*) data + i;
		register side_t* sd = sides + i;
		register sector_t* sec;
		
		sd->textureoffset = LittleSwapInt16(msd->textureoffset) << FRACBITS;
		sd->rowoffset = LittleSwapInt16(msd->rowoffset) << FRACBITS;

#ifdef COOLFLIPLEVELS
		sd->textureoffset = -sd->textureoffset;
		sd->VFlip = true;
#else
		sd->VFlip = false;
#endif
		
		// refined to allow colormaps to work as wall
		// textures if invalid as colormaps but valid as textures.
		
		sd->sector = sec = &sectors[LittleSwapInt16(msd->sector)];
		switch (sd->special)
		{
			case 242:			// variable colormap via 242 linedef
			case 280:			//SoM: 3/22/2000: New water type.
				num = R_CheckTextureNumForName(msd->toptexture);
				
				if (num == -1)
				{
					sec->topmap = mapnum = R_ColormapNumForName(msd->toptexture);
					sd->toptexture = 0;
				}
				else
					sd->toptexture = num;
					
				num = R_CheckTextureNumForName(msd->midtexture);
				if (num == -1)
				{
					sec->midmap = mapnum = R_ColormapNumForName(msd->midtexture);
					sd->midtexture = 0;
				}
				else
					sd->midtexture = num;
					
				num = R_CheckTextureNumForName(msd->bottomtexture);
				if (num == -1)
				{
					sec->bottommap = mapnum = R_ColormapNumForName(msd->bottomtexture);
					sd->bottomtexture = 0;
				}
				else
					sd->bottomtexture = num;
				break;
			case 282:			//SoM: 4/4/2000: Just colormap transfer
// SoM: R_CreateColormap will only create a colormap in software mode...
// Perhaps we should just call it instead of doing the calculations here.
				if (msd->toptexture[0] == '#' || msd->bottomtexture[0] == '#')
				{
					sec->midmap = R_CreateColormap(msd->toptexture, msd->midtexture, msd->bottomtexture);
					sd->toptexture = sd->bottomtexture = 0;
				}
				else
				{
					if ((num = R_CheckTextureNumForName(msd->toptexture)) == -1)
						sd->toptexture = 0;
					else
						sd->toptexture = num;
					if ((num = R_CheckTextureNumForName(msd->midtexture)) == -1)
						sd->midtexture = 0;
					else
						sd->midtexture = num;
					if ((num = R_CheckTextureNumForName(msd->bottomtexture)) == -1)
						sd->bottomtexture = 0;
					else
						sd->bottomtexture = num;
				}
			case 260:
				num = R_CheckTextureNumForName(msd->midtexture);
				if (num == -1)
					sd->midtexture = 1;
				else
					sd->midtexture = num;
					
				num = R_CheckTextureNumForName(msd->toptexture);
				if (num == -1)
					sd->toptexture = 1;
				else
					sd->toptexture = num;
					
				num = R_CheckTextureNumForName(msd->bottomtexture);
				if (num == -1)
					sd->bottomtexture = 1;
				else
					sd->bottomtexture = num;
				break;
				
				/*        case 260: // killough 4/11/98: apply translucency to 2s normal texture
				   sd->midtexture = strncasecmp("TRANMAP", msd->midtexture, 8) ?
				   (sd->special = W_CheckNumForName(msd->midtexture)) < 0 ||
				   W_LumpLength(sd->special) != 65536 ?
				   sd->special=0, R_TextureNumForName(msd->midtexture) :
				   (sd->special++, 0) : (sd->special=0);
				   sd->toptexture = R_TextureNumForName(msd->toptexture);
				   sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
				   break; *///This code is replaced.. I need to fix this though
				
				//Hurdler: added for alpha value with translucent 3D-floors/water
			case 300:
			case 301:
				if (msd->toptexture[0] == '#')
				{
					char* col = msd->toptexture;
					
					sd->toptexture = sd->bottomtexture = ((col[1] - '0') * 100 + (col[2] - '0') * 10 + col[3] - '0') + 1;
				}
				else
					sd->toptexture = sd->bottomtexture = 0;
				sd->midtexture = R_TextureNumForName(msd->midtexture);
				break;
				
			default:			// normal cases
				// SoM: Lots of people are sick of texture errors.
				// Hurdler: see r_data.c for my suggestion
				sd->midtexture = R_TextureNumForName(msd->midtexture);
				sd->toptexture = R_TextureNumForName(msd->toptexture);
				sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
				
#ifdef COOLFLIPLEVELS
				Temp = sd->toptexture;
				sd->toptexture = sd->bottomtexture;
				sd->bottomtexture = Temp;
#endif
				break;
		}
	}
	Z_Free(data);
}

//
// P_LoadBlockMap
//
void P_LoadBlockMap(int lump)
{
	int32_t count;
	
	count = W_LumpLength(lump) / 2;
	{
		int32_t i;
		int16_t* wadblockmaplump = W_CacheLumpNum(lump, PU_LEVEL);
		
		blockmaplump = Z_Malloc(sizeof(*blockmaplump) * count, PU_LEVEL, 0);
		
		// killough 3/1/98: Expand wad blockmap into larger internal one,
		// by treating all offsets except -1 as unsigned and zero-extending
		// them. This potentially doubles the size of blockmaps allowed,
		// because Doom originally considered the offsets as always signed.
		
		blockmaplump[0] = LittleSwapInt16(wadblockmaplump[0]);
		blockmaplump[1] = LittleSwapInt16(wadblockmaplump[1]);
		blockmaplump[2] = (int32_t)(LittleSwapInt16(wadblockmaplump[2])) & 0xffff;
		blockmaplump[3] = (int32_t)(LittleSwapInt16(wadblockmaplump[3])) & 0xffff;
		
		for (i = 4; i < count; i++)
		{
			int16_t t = LittleSwapInt16(wadblockmaplump[i]);	// killough 3/1/98
			
			blockmaplump[i] = t == -1 ? -1l : (int32_t)t & 0xffff;
		}
		
		Z_Free(wadblockmaplump);
		
		bmaporgx = blockmaplump[0] << FRACBITS;
		bmaporgy = blockmaplump[1] << FRACBITS;
		bmapwidth = blockmaplump[2];
		bmapheight = blockmaplump[3];
	}
	
	// clear out mobj chains
	count = sizeof(*blocklinks) * bmapwidth * bmapheight;
	blocklinks = Z_Malloc(count, PU_LEVEL, 0);
	memset(blocklinks, 0, count);
	blockmap = blockmaplump + 4;
	
	/* Original
	   blockmaplump = W_CacheLumpNum (lump,PU_LEVEL);
	   blockmap = blockmaplump+4;
	   count = W_LumpLength (lump)/2;
	
	   for (i=0 ; i<count ; i++)
	   blockmaplump[i] = LittleSwapInt16(blockmaplump[i]);
	
	   bmaporgx = blockmaplump[0]<<FRACBITS;
	   bmaporgy = blockmaplump[1]<<FRACBITS;
	   bmapwidth = blockmaplump[2];
	   bmapheight = blockmaplump[3];
	   }
	
	   // clear out mobj chains
	   count = sizeof(*blocklinks)*bmapwidth*bmapheight;
	   blocklinks = Z_Malloc (count,PU_LEVEL, 0);
	   memset (blocklinks, 0, count); */
}

//
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
//
void P_GroupLines(void)
{
	line_t** linebuffer;
	int i;
	int j;
	int total;
	line_t* li;
	sector_t* sector;
	subsector_t* ss;
	seg_t* seg;
	fixed_t bbox[4];
	int block;
	
	// look up sector number for each subsector
	ss = subsectors;
	for (i = 0; i < numsubsectors; i++, ss++)
	{
		seg = &segs[ss->firstline];
		ss->sector = seg->sidedef->sector;
	}
	
	// count number of lines in each sector
	li = lines;
	total = 0;
	for (i = 0; i < numlines; i++, li++)
	{
		total++;
		li->frontsector->linecount++;
		
		if (li->backsector && li->backsector != li->frontsector)
		{
			li->backsector->linecount++;
			total++;
		}
	}
	
	// build line tables for each sector
	linebuffer = Z_Malloc(total * sizeof(void*), PU_LEVEL, 0);
	sector = sectors;
	for (i = 0; i < numsectors; i++, sector++)
	{
		M_ClearBox(bbox);
		sector->lines = linebuffer;
		li = lines;
		for (j = 0; j < numlines; j++, li++)
		{
			if (li->frontsector == sector || li->backsector == sector)
			{
				*linebuffer = li;
				linebuffer++;
				M_AddToBox(bbox, li->v1->x, li->v1->y);
				M_AddToBox(bbox, li->v2->x, li->v2->y);
			}
		}
		if (linebuffer - sector->lines != sector->linecount)
			I_Error("P_GroupLines: miscounted");
			
		// set the degenmobj_t to the middle of the bounding box
		sector->soundorg.x = (bbox[BOXRIGHT] + bbox[BOXLEFT]) / 2;
		sector->soundorg.y = (bbox[BOXTOP] + bbox[BOXBOTTOM]) / 2;
		
		// adjust bounding box to map blocks
		block = (bbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;
		block = block >= bmapheight ? bmapheight - 1 : block;
		sector->blockbox[BOXTOP] = block;
		
		block = (bbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
		block = block < 0 ? 0 : block;
		sector->blockbox[BOXBOTTOM] = block;
		
		block = (bbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
		block = block >= bmapwidth ? bmapwidth - 1 : block;
		sector->blockbox[BOXRIGHT] = block;
		
		block = (bbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
		block = block < 0 ? 0 : block;
		sector->blockbox[BOXLEFT] = block;
	}
	
}

// SoM: 6/27: Don't restrict maps to MAPxx/ExMx any more!
char* levellumps[] =
{
	"label",					// ML_LABEL,    A separator, name, ExMx or MAPxx
	"THINGS",					// ML_THINGS,   Monsters, items..
	"LINEDEFS",					// ML_LINEDEFS, LineDefs, from editing
	"SIDEDEFS",					// ML_SIDEDEFS, SideDefs, from editing
	"VERTEXES",					// ML_VERTEXES, Vertices, edited and BSP splits generated
	"SEGS",						// ML_SEGS,     LineSegs, from LineDefs split by BSP
	"SSECTORS",					// ML_SSECTORS, SubSectors, list of LineSegs
	"NODES",					// ML_NODES,    BSP nodes
	"SECTORS",					// ML_SECTORS,  Sectors, from editing
	"REJECT",					// ML_REJECT,   LUT, sector-sector visibility
	"BLOCKMAP"					// ML_BLOCKMAP  LUT, motion clipping, walls/grid element
};

//
// Setup sky texture to use for the level, actually moved the code
// from G_DoLoadLevel() which had nothing to do there.
//
// - in future, each level may use a different sky.
//
// The sky texture to be used instead of the F_SKY1 dummy.
void P_SetupLevelSky(void)
{
	char skytexname[12];
	
	// DOOM determines the sky texture to be used
	// depending on the current episode, and the game version.
	
	if (info_skyname && *info_skyname)
		skytexture = R_TextureNumForName(info_skyname);
	else if ((gamemode == commercial))
		// || (gamemode == pack_tnt) he ! is not a mode is a episode !
		//    || ( gamemode == pack_plut )
	{
		if (gamemap < 12)
			skytexture = R_TextureNumForName("SKY1");
		else if (gamemap < 21)
			skytexture = R_TextureNumForName("SKY2");
		else
			skytexture = R_TextureNumForName("SKY3");
	}
	else if ((gamemode == retail) || (gamemode == registered))
	{
		if (gameepisode < 1 || gameepisode > 4)	// useful??
			gameepisode = 1;
			
		sprintf(skytexname, "SKY%d", gameepisode);
		skytexture = R_TextureNumForName(skytexname);
	}
	else
		skytexture = R_TextureNumForName("SKY1");
		
	// scale up the old skies, if needed
	R_SetupSkyDraw();
}

//
// P_SetupLevel
//
// added comment : load the level from a lump file or from a external wad !
extern int numtextures;
char* maplumpname;

int lastloadedmaplumpnum;		// for comparative savegame
bool_t P_SetupLevel(int episode, int map, skill_t skill, char* wadname)	// for wad files
{
	int i;
	WadEntry_t* HexenACS = NULL;
	
	CON_Drawer();				// let the user know what we are going to do
	I_FinishUpdate();			// page flip or blit buffer
	
	//Initialize sector node list.
	P_Initsecnode();
	
	totalkills = totalitems = totalsecret = wminfo.maxfrags = 0;
	wminfo.partime = 180;
	for (i = 0; i < MAXPLAYERS; i++)
	{
		players[i].killcount = players[i].secretcount = players[i].itemcount = 0;
		players[i].mo = NULL;
	}
	
	// Initial height of PointOfView
	// will be set by player think.
	
	players[consoleplayer[0]].viewz = 1;
	
	// Make sure all sounds are stopped before Z_FreeTags.
	S_StopSounds();
	
	Z_FreeTags(PU_LEVEL, PU_PURGELEVEL - 1);
	
#ifdef WALLSPLATS
	// clear the splats from previous level
	R_ClearLevelSplats();
#endif
	
	script_camera_on = false;
	HU_ClearTips();
	
	//if (camera.chase)
	//	camera.mo = NULL;
		
	// UNUSED W_Profile ();
	
	P_InitThinkers();
	
	// if working with a devlopment map, reload it
	W_Reload();
	
	//
	//  load the map from internal game resource or external wad file
	//
	if (wadname)
	{
		char* firstmap = NULL;
		
		// go back to title screen if no map is loaded
		if (!P_AddWadFile(wadname, &firstmap) || firstmap == NULL)	// no maps were found
		{
			return false;
		}
		// P_AddWadFile() sets lumpname
		lastloadedmaplumpnum = W_GetNumForName(firstmap);
		maplumpname = firstmap;
	}
	else
	{
		// internal game map
		lastloadedmaplumpnum = W_GetNumForName(maplumpname = G_BuildMapName(episode, map));
	}
	
	if (levelmapname)
		Z_Free(levelmapname);
	levelmapname = Z_Strdup(maplumpname, PU_STATIC, 0);
	
	leveltime = 0;

	R_ClearColormaps();
	
	// GhostlyDeath <December 21, 2008> -- Check for HeXeN Map
	HexenACS = W_GetEntry(lastloadedmaplumpnum + ML_BEHAVIOR);
	if (HexenACS && strncasecmp(HexenACS->Name, "BEHAVIOR", 8) == 0)
	{
		CONL_PrintF("P_SetupLevel: HeXeN maps are NOT fully supported!\n");
		//return false;
	}
	else
		HexenACS = NULL;
		
#ifdef FRAGGLESCRIPT
	P_LoadLevelInfo(lastloadedmaplumpnum);	// load level lump info(level name etc)
#endif
	
	//SoM: We've loaded the music lump, start the music.
	S_Start();
	
	//faB: now part of level loading since in future each level may have
	//     its own anim texture sequences, switches etc.
	P_InitSwitchList();
	P_InitPicAnims();
	P_SetupLevelSky();
	
	// SoM: WOO HOO!
	// SoM: DOH!
	//R_InitPortals ();
	
	// note: most of this ordering is important
	P_LoadBlockMap(lastloadedmaplumpnum + ML_BLOCKMAP);
	P_LoadVertexes(lastloadedmaplumpnum + ML_VERTEXES);
	P_LoadSectors(lastloadedmaplumpnum + ML_SECTORS);
	P_LoadSideDefs(lastloadedmaplumpnum + ML_SIDEDEFS);
	
	if (HexenACS)
		P_LoadLineDefsHexen(lastloadedmaplumpnum + ML_LINEDEFS);
	else
		P_LoadLineDefs(lastloadedmaplumpnum + ML_LINEDEFS);
		
	P_LoadSideDefs2(lastloadedmaplumpnum + ML_SIDEDEFS);
	P_LoadLineDefs2();
	P_LoadSubsectors(lastloadedmaplumpnum + ML_SSECTORS);
	P_LoadNodes(lastloadedmaplumpnum + ML_NODES);
	P_LoadSegs(lastloadedmaplumpnum + ML_SEGS);
	rejectmatrix = W_CacheLumpNum(lastloadedmaplumpnum + ML_REJECT, PU_LEVEL);
	P_GroupLines();
	
	bodyqueslot = 0;
	
	numdmstarts = 0;
	// added 25-4-98 : reset the players starts
	//SoM: Set pointers to NULL
	for (i = 0; i < MAXPLAYERS; i++)
		playerstarts[i] = NULL;
		
	if (HexenACS)
		P_LoadThingsHexen(lastloadedmaplumpnum + ML_THINGS);
	else
		P_LoadThings(lastloadedmaplumpnum + ML_THINGS);
		
	// set up world state
	P_SpawnSpecials();
	P_InitBrainTarget();
	
	//BP: spawnplayers now (beffor all structure are not inititialized)
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
		{
			if (cv_deathmatch.value)
			{
				players[i].mo = NULL;
				G_DoReborn(i);
			}
			else if (P_EXGSGetValue(PEXGSBID_COSPAWNPLAYERSEARLY) && !localgame)
			{
				players[i].mo = NULL;
				G_CoopSpawnPlayer(i);
			}
		}
	// clear special respawning que
	iquehead = iquetail = 0;
	
	// build subsector connect matrix
	//  UNUSED P_ConnectSubsectors ();
	
	//Fab:19-07-98:start cd music for this level (note: can be remapped)
	if (gamemode == commercial)
		I_PlayCD(map, true);	// Doom2, 32 maps
	else
		I_PlayCD((episode - 1) * 9 + map, true);	// Doom1, 9maps per episode
		
	// preload graphics
	
	if (precache)
		R_PrecacheLevel();
		
	// TODO: First attempt to load ReMooD scripts, if that fails do Legacy Script
	if (M_CheckParm("-tls"))
	{
		TLS_ClearScripts();
		TLS_CompileLump(lastloadedmaplumpnum);
	}
	else
		T_PreprocessScripts();	// preprocess FraggleScript scripts
		
	script_camera_on = false;
	
	//CONL_PrintF("%d vertexs %d segs %d subsector\n",numvertexes,numsegs,numsubsectors);
	return true;
}

//
// Add a wadfile to the active wad files,
// replace sounds, musics, patches, textures, sprites and maps
//
bool_t P_AddWadFile(char* wadfilename, char** firstmapname)
{
	WadEntry_t* LumpEntry = NULL;
	WadFile_t* WAD = W_GetWadForName(wadfilename);
	char* Name;
	char* name;
	size_t i, j, k, num;
	int firstmapreplaced;
	uint32_t SoundReplacements;
	uint32_t MusicReplacements;
	bool_t TextChange;
	
	/******************
	 SOUND REPLACEMENTS
	 ******************/
	SoundReplacements = 0;
	MusicReplacements = 0;
	
	// Search for sound replacements
	for (i = 0; i < WAD->NumLumps; i++)
	{
		Name = WAD->Index[i].Name;
		
		if (strncasecmp("DS", Name, 2) == 0)
		{
			for (j = 0; j < NUMSFX; j++)
			{
				if ((S_sfx[j].name) && !(S_sfx[j].link) && (strncasecmp(Name[2], S_sfx[j].name, 6) == 0))
				{
					S_FreeSfx(&S_sfx[j]);
					SoundReplacements++;
				}
			}
		}
		else if (strncasecmp("D_", Name, 2) == 0)
		{
			MusicReplacements++;
		}
		else if ((strncasecmp("TEXTURE1", Name, 8) == 0) ||	// Sneak in textures
		         (strncasecmp("TEXTURE2", Name, 8) == 0) || (strncasecmp("PNAMES", Name, 6) == 0))
			TextChange = true;
	}
	
	CONL_PrintF("P_AddWadFile: %i sounds replaced\n", SoundReplacements);
	CONL_PrintF("P_AddWadFile: %i songs replaced\n", MusicReplacements);
	
	/******************
	 MUSIC REPLACEMENTS
	 ******************/
	
	/*******************
	 SPRITE REPLACEMENTS
	 *******************/
	R_AddSpriteDefs(sprnames, W_NumWadFiles());
	
	/***************
	 TEXTURE CHANGES
	 ***************/
	
	/*****
	 SKINS
	 *****/
	
	/****
	 MAPS
	 ****/
	
	firstmapreplaced = 0;
	
	for (i = 0; i < WAD->NumLumps; i++)
	{
		num = firstmapreplaced;
		name = WAD->Index[i].Name;
		
		if (gamemode == commercial)	// Doom II
		{
			if (name[0] == 'M' && name[1] == 'A' && name[2] == 'P')
			{
				num = (name[3] - '0') * 10 + (name[4] - '0');
				CONL_PrintF("Map %d\n", num);
			}
		}
		else					// Something else
		{
			if (name[0] == 'E' && ((unsigned)name[1] - '0') <= '9' &&	// a digit
			        name[2] == 'M' && ((unsigned)name[3] - '0') <= '9' && name[4] == 0)
			{
				num = ((name[1] - '0') << 16) + (name[3] - '0');
				CONL_PrintF("Episode %d map %d\n", name[1] - '0', name[3] - '0');
			}
		}
		
		if (num && (num < firstmapreplaced || !firstmapreplaced))
		{
			firstmapreplaced = num;
			
			if (firstmapname)
				*firstmapname = name;
		}
	}
	
	if (!firstmapreplaced)
		CONL_PrintF("no maps added\n");
		
	/****
	 DEFINITIONS
	 ****/
	
	//DEH_LoadDehackedLump();
	
	// Load the status bar!
	if (gamestate == GS_LEVEL)
		ST_Start();
		
	return true;
	
	/*int         firstmapreplaced;
	   wadfile_t*  wadfile;
	   char*       name;
	   int         i,j,num,wadfilenum;
	   lumpinfo_t* lumpinfo;
	   int         replaces;
	   bool_t     texturechange;
	
	   if ((wadfilenum = W_LoadWadFile (wadfilename))==-1)
	   {
	   CONL_PrintF ("couldn't load wad file %s\n", wadfilename);
	   return false;
	   }
	   wadfile = wadfiles[wadfilenum];
	
	   //
	   // search for sound replacements
	   //
	   lumpinfo = wadfile->lumpinfo;
	   replaces = 0;
	   texturechange=false;
	   for (i=0; i<wadfile->numlumps; i++,lumpinfo++)
	   {
	   name = lumpinfo->name;
	   if (name[0]=='D' && name[1]=='S')
	   {
	   for (j=1 ; j<NUMSFX ; j++)
	   {
	   if ( S_sfx[j].name &&
	   !S_sfx[j].link &&
	   !strncasecmp(S_sfx[j].name,name+2,6) )
	   {
	   // the sound will be reloaded when needed,
	   // since sfx->data will be NULL
	   if (devparm)
	   CONL_PrintF ("Sound %.8s replaced\n", name);
	
	   I_FreeSfx (&S_sfx[j]);
	
	   replaces++;
	   }
	   }
	   }
	   else
	   if( memcmp(name,"TEXTURE1",8)==0    // find texture replesement too
	   || memcmp(name,"TEXTURE2",8)==0
	   || memcmp(name,"PNAMES",6)==0)
	   texturechange=true;
	   }
	   if (!devparm && replaces)
	   CONL_PrintF ("%d sounds replaced\n", replaces);
	
	   //
	   // search for music replacements
	   //
	   lumpinfo = wadfile->lumpinfo;
	   replaces = 0;
	   for (i=0; i<wadfile->numlumps; i++,lumpinfo++)
	   {
	   name = lumpinfo->name;
	   if (name[0]=='D' && name[1]=='_')
	   {
	   if (devparm)
	   CONL_PrintF ("Music %.8s replaced\n", name);
	   replaces++;
	   }
	   }
	   if (!devparm && replaces)
	   CONL_PrintF ("%d musics replaced\n", replaces);
	
	   //
	   // search for sprite replacements
	   //
	   R_AddSpriteDefs (sprnames, numwadfiles-1);
	
	   //
	   // search for texturechange replacements
	   //
	
	   //
	   // look for skins
	   //
	   R_AddSkins (wadfilenum);      //faB: wadfile index in wadfiles[]
	
	   //
	   // search for maps
	   //
	   lumpinfo = wadfile->lumpinfo;
	   firstmapreplaced = 0;
	   for (i=0; i<wadfile->numlumps; i++,lumpinfo++)
	   {
	   name = lumpinfo->name;
	   num = firstmapreplaced;
	   if (gamemode==commercial)       // Doom2
	   {
	   if (name[0]=='M' &&
	   name[1]=='A' &&
	   name[2]=='P')
	   {
	   num = (name[3]-'0')*10 + (name[4]-'0');
	   CONL_PrintF ("Map %d\n", num);
	   }
	   }
	   else
	   {
	   if (name[0]=='E' &&
	   ((unsigned)name[1]-'0')<='9' &&   // a digit
	   name[2]=='M' &&
	   ((unsigned)name[3]-'0')<='9' &&
	   name[4]==0)
	   {
	   num = ((name[1]-'0')<<16) + (name[3]-'0');
	   CONL_PrintF ("Episode %d map %d\n", name[1]-'0',
	   name[3]-'0');
	   }
	   }
	   if (num && (num<firstmapreplaced || !firstmapreplaced))
	   {
	   firstmapreplaced = num;
	   if(firstmapname) *firstmapname = name;
	   }
	   }
	   if (!firstmapreplaced)
	   CONL_PrintF ("no maps added\n");
	
	   // reload status bar (warning should have valide player !)
	   if( gamestate == GS_LEVEL )
	   ST_Start();
	
	   return true; */
}

/*******************************************************************************
********************************************************************************
*******************************************************************************/

/*** STRUCTURES ***/

/*** FUNCTIONS ***/

/* PCLC_Map() -- Switch to another map */
static CONL_ExitCode_t PCLC_Map(const uint32_t a_ArgC, const char** const a_ArgV)
{
	P_LevelInfoEx_t* Info;
	size_t i;
	bool_t LevelSwitch = false;
	
	/* Check */
	if (a_ArgC < 2)
		return CLE_INVALIDARGUMENT;
	
	/* Locate map */
	Info = P_FindLevelByNameEx(a_ArgV[1], NULL);
	
	// Not found?
	if (!Info)
		return CLE_RESOURCENOTFOUND;
	
	/* Switching levels */
	if (strcasecmp(a_ArgV[0], "switchmap") == 0)
		LevelSwitch = true;
	
	/* Reset player info */
	if (!LevelSwitch)
	{
		// TODO: Multiplayer here!
		for (i = 0; i < MAXPLAYERS; i++)
			playeringame[i] = false;
	
		for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
			consoleplayer[i] = displayplayer[i] = i;
		
		for (i = 0; i < MAXSPLITSCREEN; i++)
			g_PlayerInSplit[i] = false;
		
		g_SplitScreen = -1;
	
		// Spawn player 1 at least
		//playeringame[0] = true;
	}
	
	/* Load level */
	if (P_ExLoadLevel(Info, true))
		return CLE_SUCCESS;
	
	/* Failed */
	return CLE_FAILURE;
}

/* P_InitSetupEx() -- Initializes extended setup code */
void P_InitSetupEx(void)
{
	/* Add Console Commands */
	CONL_AddCommand("map", PCLC_Map);
	CONL_AddCommand("switchmap", PCLC_Map);
}

/* P_ExClearLevel() -- Clears a level and reverts the game state */
bool_t P_ExClearLevel(void)
{
	size_t i;
	
	/* Clear Stuff */
	HU_ClearTips();
	S_StopSounds();
	R_ClearLevelSplats();
	P_ClearRecursiveSound();
	
	/* Free all level tags */
	Z_FreeTags(PU_LEVEL, PU_PURGELEVEL - 1);
	
	/* Wipe cameras */
	for (i = 0; i < MAXPLAYERS; i++)
		if (players[i].camera.chase)
			players[i].camera.mo = NULL;
	
	/* Re-initialize */
	P_Initsecnode();
	P_InitThinkers();
	
	/* Re-init some things */
	// Body Queue
	memset(bodyque, 0, sizeof(bodyque));
	bodyqueslot = true;

	/* Always succeeds */
	return true;
}

/* PS_ExVertexInit() -- Initializes extra data in vertex */
static void PS_ExVertexInit(vertex_t* const a_Vertex)
{
}

/* PS_ExSectorInit() -- Initializes extra data in sector */
static void PS_ExSectorInit(sector_t* const a_Sector)
{
	/* Check */
	if (!a_Sector)
		return;
	
	/* Set default fields */
	a_Sector->nextsec = -1;
	a_Sector->prevsec = -1;
	a_Sector->heightsec = -1;
	a_Sector->floorlightsec = -1;
	a_Sector->ceilinglightsec = -1;
	a_Sector->bottommap = -1;
	a_Sector->midmap = -1;
	a_Sector->topmap = -1;
	a_Sector->moved = true;
	a_Sector->lineoutLength = -1.0;
	
	/* Clear bounding box */
	M_ClearBox(a_Sector->BBox);
	
	/* Get flats */
	a_Sector->floorpic = R_GetFlatNumForName(a_Sector->FloorTexture);
	a_Sector->ceilingpic = R_GetFlatNumForName(a_Sector->CeilingTexture);
}

/* PS_ExSideDefInit() -- Initializes extra data in side def */
static void PS_ExSideDefInit(side_t* const a_SideDef)
{
	/* Check */
	if (!a_SideDef)
		return;
	
	/* Reference to sector */
	// Make sure it is legal
	if (a_SideDef->SectorNum >= 0 && a_SideDef->SectorNum < numsectors)
		a_SideDef->sector = &sectors[a_SideDef->SectorNum];
	
	// Otherwise use the first sector (ouch!)
	else
		a_SideDef->sector = &sectors[0];
	
	/* Reference Textures */
	a_SideDef->toptexture = R_TextureNumForName(a_SideDef->WallTextures[0]);
	a_SideDef->bottomtexture = R_TextureNumForName(a_SideDef->WallTextures[1]);
	a_SideDef->midtexture = R_TextureNumForName(a_SideDef->WallTextures[2]);
}

/* PS_ExLineDefInit() -- Initializes extra data in linedef */
static void PS_ExLineDefInit(line_t* const a_LineDef)
{
	size_t i;
	bool_t a;
	sector_t** SecRef;
	
	/* Check */
	if (!a_LineDef)
		return;
	
	/* Reference */
	// First Vertex
	if (a_LineDef->VertexNum[0] >= 0 && a_LineDef->VertexNum[0] < numvertexes)
		a_LineDef->v1 = &vertexes[a_LineDef->VertexNum[0]];
	else
		a_LineDef->v1 = &vertexes[0];	// Oops!
	
	// Second vertex
	if (a_LineDef->VertexNum[1] >= 0 && a_LineDef->VertexNum[1] < numvertexes)
		a_LineDef->v2 = &vertexes[a_LineDef->VertexNum[1]];
	else
		a_LineDef->v2 = &vertexes[0];	// Oops!
	
	/* Physics Calculations */
	// Difference of units
	a_LineDef->dx = a_LineDef->v2->x - a_LineDef->v1->x;
	a_LineDef->dy = a_LineDef->v2->y - a_LineDef->v1->y;
	
	// Slope type
	if (!a_LineDef->dx)
		a_LineDef->slopetype = ST_VERTICAL;
	else if (!a_LineDef->dy)
		a_LineDef->slopetype = ST_HORIZONTAL;
	else if (FixedDiv(a_LineDef->dy, a_LineDef->dx) > 0)
		a_LineDef->slopetype = ST_POSITIVE;
	else
		a_LineDef->slopetype = ST_NEGATIVE;
	
	// Bounding volume
		// X Coord
	a = false;
	if (a_LineDef->v1->x < a_LineDef->v2->x)
		a = true;
	
	a_LineDef->bbox[(a ? BOXLEFT : BOXRIGHT)] = a_LineDef->v1->x;
	a_LineDef->bbox[(a ? BOXRIGHT : BOXLEFT)] = a_LineDef->v2->x;
	
		// Y Coord
	a = false;
	if (a_LineDef->v1->y < a_LineDef->v2->y)
		a = true;
	
	a_LineDef->bbox[(a ? BOXBOTTOM : BOXTOP)] = a_LineDef->v1->y;
	a_LineDef->bbox[(a ? BOXTOP : BOXBOTTOM)] = a_LineDef->v2->y;
	
	/* Legalize side def reference */
	for (i = 0; i < 2; i++)
		if (a_LineDef->sidenum[i] < 0 || a_LineDef->sidenum[i] >= numsides)
			a_LineDef->sidenum[i] = -1;
	
	/* Set linedef sectors based on side def */
	for (i = 0; i < 2; i++)
		if (a_LineDef->sidenum[i] != -1)
		{
			SecRef = (i == 0 ? &a_LineDef->frontsector : &a_LineDef->backsector);
			*SecRef = sides[a_LineDef->sidenum[i]].sector;
		}
	
	/* Calculate the real line special (generalized) */
	a_LineDef->special = EV_DoomToGenTrigger(a_LineDef->special);
	
	/* Set side special from linedef */
	//if (ld->sidenum[0] != -1 && ld->special)
	//	sides[ld->sidenum[0]].special = ld->special;
}

/* PS_ExSubSectorInit() -- Initializes extra data in subsector */
static void PS_ExSubSectorInit(subsector_t* const a_SubSector)
{
	/* Check */
	if (!a_SubSector)
		return;
}

/* PS_ExNodeInit() -- Initializes extra data in node */
static void PS_ExNodeInit(node_t* const a_Node)
{
	size_t i;
	uint32_t Real;
	
	/* Check */
	if (!a_Node)
		return;
	
	/* Check for illegal child reference */
	for (i = 0; i < 2; i++)
		// Subsector
		if (a_Node->children[i] & NF_SUBSECTOR)
		{
			// Get real value
			Real = a_Node->children[i] & (~NF_SUBSECTOR);
			
			// Check
			if (Real < 0 || Real >= numsubsectors)
				a_Node->children[i] = NF_SUBSECTOR;	// Oops!
		}
		
		// Another node
		else
		{
			// Get real value
			Real = a_Node->children[i];
			
			// Check
			if (Real < 0 || Real >= numnodes)
				a_Node->children[i] = NF_SUBSECTOR;	// Oops!
		}
}

/* PS_ExSegInit() -- Initializes extra data in seg */
static void PS_ExSegInit(seg_t* const a_Seg)
{
	size_t i;
	
	/* Check */
	if (!a_Seg)
		return;
	
	/* Correct illegal references */
	// Vertex
	for (i = 0; i < 2; i++)
		if (a_Seg->VertexID[i] < 0 || a_Seg->VertexID[i] >= numvertexes)
			a_Seg->VertexID[i] = 0;
	
	// Lines
	if (a_Seg->LineID < 0 || a_Seg->LineID >= numlines)
		a_Seg->LineID = 0;
	
	// Sides
	if (a_Seg->side != 0)
		a_Seg->side = 1;
	
	/* Reference */
	a_Seg->v1 = &vertexes[a_Seg->VertexID[0]];
	a_Seg->v2 = &vertexes[a_Seg->VertexID[1]];
	a_Seg->linedef = &lines[a_Seg->LineID];
	a_Seg->sidedef = &sides[a_Seg->linedef->sidenum[a_Seg->side]];
	a_Seg->frontsector = (a_Seg->side ? a_Seg->linedef->backsector : a_Seg->linedef->frontsector);
	a_Seg->backsector = (a_Seg->side ? a_Seg->linedef->frontsector : a_Seg->linedef->backsector);
	
	/* Initialize */
	a_Seg->numlights = 0;
	a_Seg->rlights = NULL;
}

/* PS_ExThingInit() -- Initializes extra data in thing */
static void PS_ExThingInit(mapthing_t* const a_Thing)
{
	/* Check */
	if (!a_Thing)
		return;
	
	/* Spawn it */
	P_SpawnMapThing(a_Thing);	
}

/* PS_ExMungeNodeData() -- Munges together node data */
void PS_ExMungeNodeData(void)
{
	line_t** LineBuffer;
	size_t i, j, Total, OldCount;
	fixed_t BBox[4];
	sector_t* SectorP;
	int block;
	
	/* Set loading screen info */
	CONL_LoadingScreenSetSubEnd(3);
	
	/* Initialize block links */
	CONL_LoadingScreenIncrSub();
	blocklinks = Z_Malloc(sizeof(*blocklinks) * (bmapwidth * bmapheight), PU_LEVEL, (void**)&blocklinks);
	
	/* Reference sectors to subsectors */
	CONL_LoadingScreenIncrSub();
	for (i = 0; i < numsubsectors; i++)
		subsectors[i].sector = segs[subsectors[i].firstline].sidedef->sector;
	
	/* Count line numbers for each sector */
	CONL_LoadingScreenIncrSub();
	for (Total = 0, i = 0; i < numlines; i++)
		// For each side
		for (j = 0; j < 2; j++)
		{
			// Front or back?
			SectorP = (!j ? lines[i].frontsector : lines[i].backsector);
			
			// Nothing here?
			if (!SectorP)
				continue;
			
			// Increase line total
			Total++;
			
			// Re-allocate line buffer array
			Z_ResizeArray((void**)&SectorP->lines, sizeof(*SectorP->lines), SectorP->linecount, SectorP->linecount + 1);
			
			// Change to level tag
			Z_ChangeTag(SectorP->lines, PU_LEVEL);
			
			// Put last spot as this line
			SectorP->lines[SectorP->linecount++] = &lines[i];
			
			// Add to sector bounding box
			M_AddToBox(SectorP->BBox, lines[i].v1->x, lines[i].v1->y);
			M_AddToBox(SectorP->BBox, lines[i].v2->x, lines[i].v2->y);
			
			// Sound Origin
			SectorP->soundorg.x = (SectorP->BBox[BOXRIGHT] + SectorP->BBox[BOXLEFT]) / 2;
			SectorP->soundorg.y = (SectorP->BBox[BOXTOP] + SectorP->BBox[BOXBOTTOM]) / 2;
			
			// adjust bounding box to map blocks
			block = (SectorP->BBox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;
			block = block >= bmapheight ? bmapheight - 1 : block;
			SectorP->blockbox[BOXTOP] = block;
	
			block = (SectorP->BBox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
			block = block < 0 ? 0 : block;
			SectorP->blockbox[BOXBOTTOM] = block;
	
			block = (SectorP->BBox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
			block = block >= bmapwidth ? bmapwidth - 1 : block;
			SectorP->blockbox[BOXRIGHT] = block;
	
			block = (SectorP->BBox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
			block = block < 0 ? 0 : block;
			SectorP->blockbox[BOXLEFT] = block;
		}
}

/* P_ExLoadLevel() -- Loads a new level */
bool_t P_ExLoadLevel(P_LevelInfoEx_t* const a_Info, const bool_t a_ApplyOptions)
{
#define BUFSIZE 512
#define LOADSHIFT 6
#define LOADMASK ((1 << LOADSHIFT) - 1)
	const WL_WADEntry_t* Entry;
	WL_EntryStream_t* Stream;
	vertex_t* VertexP;
	sector_t* SectorP;
	side_t* SideDefP;
	line_t* LineDefP;
	subsector_t* SubSectorP;
	node_t* NodeP;
	seg_t* SegP;
	mapthing_t* ThingP;
	size_t i, j, k;
	char Buf[BUFSIZE];
	int16_t TempShort;

	/* Check */
	if (!a_Info)
		return false;
	
	/* Set global info */
	g_CurrentLevelInfo = a_Info;
	
	/* Respawn all players */
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
		{
			// Revive dead players
			if (!players[i].mo || players[i].playerstate == PST_DEAD)
				players[i].playerstate = PST_REBORN;
				
			// Remove map object here
			players[i].mo = NULL;
		}
		
	/* Debug */
	if (devparm)
		CONL_PrintF("P_ExLoadLevel: Loading \"%s\"...\n", a_Info->LumpName);
	
	/* Loading Screen */
	CONL_LoadingScreenSet(12);
	
	/* Clear level data */
	CONL_LoadingScreenIncrMaj("Clearing level", 0);
	P_ExClearLevel();
	
	/* Load Map Data */
	// Load vertex data (Non-Textual Format)
	CONL_LoadingScreenIncrMaj("Loading vertexes", 0);
	if (!a_Info->Type.Text && (Entry = a_Info->EntryPtr[PLIEDS_VERTEXES]))
	{
		// Open stream
		Stream = WL_StreamOpen(Entry);
		
		// Read in data
		if (Stream)
		{
			// Allocate array
			numvertexes = Entry->Size / 4;	// 1 vertex is 4 bytes
			vertexes = Z_Malloc(sizeof(*vertexes) * numvertexes, PU_LEVEL, (void**)&vertexes);
			
			// Set loading screen info
			CONL_LoadingScreenSetSubEnd(numvertexes >> LOADSHIFT);
			
			// Read in data
			for (i = 0; i < numvertexes && !WL_StreamEOF(Stream); i++)
			{
				// Loading screen
				if ((i & LOADMASK) == 0)
					CONL_LoadingScreenIncrSub();
				
				// Get pointer
				VertexP = &vertexes[i];
				
				// Read
				VertexP->x = ((fixed_t)WL_StreamReadLittleInt16(Stream)) <<  FRACBITS;
				VertexP->y = ((fixed_t)WL_StreamReadLittleInt16(Stream)) <<  FRACBITS;
				
				// Initialize
				PS_ExVertexInit(VertexP);
			}
			
			// Close stream
			WL_StreamClose(Stream);
		}
	}
	
	// Load sector data (Non-Textual Format)
	CONL_LoadingScreenIncrMaj("Loading sectors", 0);
	if (!a_Info->Type.Text && (Entry = a_Info->EntryPtr[PLIEDS_SECTORS]))
	{
		// Open stream
		Stream = WL_StreamOpen(Entry);
		
		// Read in data
		if (Stream)
		{
			// Allocate array
			numsectors = Entry->Size / 26;	// 1 sector is 26 bytes
			sectors = Z_Malloc(sizeof(*sectors) * numsectors, PU_LEVEL, (void**)&sectors);
			
			// Set loading screen info
			CONL_LoadingScreenSetSubEnd(numsectors >> LOADSHIFT);
			
			// Read in data
			for (i = 0; i < numsectors && !WL_StreamEOF(Stream); i++)
			{
				// Loading screen
				if ((i & LOADMASK) == 0)
					CONL_LoadingScreenIncrSub();
				
				// Get pointer
				SectorP = &sectors[i];
				
				// Read
				SectorP->floorheight = ((fixed_t)WL_StreamReadLittleInt16(Stream)) << FRACBITS;
				SectorP->ceilingheight = ((fixed_t)WL_StreamReadLittleInt16(Stream)) << FRACBITS;
				
				memset(Buf, 0, sizeof(Buf));
				for (j = 0; j < 8; j++)
					Buf[j] = WL_StreamReadUInt8(Stream);
				SectorP->FloorTexture = Z_StrDup(Buf, PU_LEVEL, NULL);
				
				memset(Buf, 0, sizeof(Buf));
				for (j = 0; j < 8; j++)
					Buf[j] = WL_StreamReadUInt8(Stream);
				SectorP->CeilingTexture = Z_StrDup(Buf, PU_LEVEL, NULL);
				
				SectorP->lightlevel = WL_StreamReadLittleInt16(Stream);
				SectorP->special = WL_StreamReadLittleUInt16(Stream);
				SectorP->tag = WL_StreamReadLittleUInt16(Stream);
				
				// Initialize
				PS_ExSectorInit(SectorP);
			}
			
			// Close stream
			WL_StreamClose(Stream);
		}
	}
	
	// Load Side-Def Data (Non-Textual Format)
	CONL_LoadingScreenIncrMaj("Loading side defs", 0);
	if (!a_Info->Type.Text && (Entry = a_Info->EntryPtr[PLIEDS_SIDEDEFS]))
	{
		// Open stream
		Stream = WL_StreamOpen(Entry);
		
		// Read in data
		if (Stream)
		{
			// Allocate array
			numsides = Entry->Size / 30;	// 1 sidedef is 30 bytes
			sides = Z_Malloc(sizeof(*sides) * numsides, PU_LEVEL, (void**)&sides);
			
			// Set loading screen info
			CONL_LoadingScreenSetSubEnd(numsides >> LOADSHIFT);
			
			// Read in data
			for (i = 0; i < numsides && !WL_StreamEOF(Stream); i++)
			{
				// Loading screen
				if ((i & LOADMASK) == 0)
					CONL_LoadingScreenIncrSub();
				
				// Get pointer
				SideDefP = &sides[i];
				
				// Read
				SideDefP->textureoffset = ((fixed_t)WL_StreamReadLittleInt16(Stream)) <<  FRACBITS;
				SideDefP->rowoffset = ((fixed_t)WL_StreamReadLittleInt16(Stream)) <<  FRACBITS;
				
				for (k = 0; k < 3; k++)
				{
					memset(Buf, 0, sizeof(Buf));
					for (j = 0; j < 8; j++)
						Buf[j] = WL_StreamReadUInt8(Stream);
					SideDefP->WallTextures[k] = Z_StrDup(Buf, PU_LEVEL, NULL);
				}
				
				SideDefP->SectorNum = WL_StreamReadLittleUInt16(Stream);
				
				// Initialize
				PS_ExSideDefInit(SideDefP);
			}
			
			// Close stream
			WL_StreamClose(Stream);
		}
	}
	
	// Load Line-Def Data (Non-Textual Format)
	CONL_LoadingScreenIncrMaj("Loading line defs", 0);
	if (!a_Info->Type.Text && (Entry = a_Info->EntryPtr[PLIEDS_LINEDEFS]))
	{
		// Open stream
		Stream = WL_StreamOpen(Entry);
		
		// Read in data
		if (Stream)
		{
			// Allocate array
			numlines = Entry->Size / (a_Info->Type.Hexen ? 16 : 14);	// 1 linedef is 15 bytes (doom) and 16 bytes (hexen)
			lines = Z_Malloc(sizeof(*lines) * numlines, PU_LEVEL, (void**)&lines);
			
			// Set loading screen info
			CONL_LoadingScreenSetSubEnd(numlines >> LOADSHIFT);
			
			// Read in data
			for (i = 0; i < numlines && !WL_StreamEOF(Stream); i++)
			{
				// Loading screen
				if ((i & LOADMASK) == 0)
					CONL_LoadingScreenIncrSub();
				
				// Get pointer
				LineDefP = &lines[i];
				
				// Read
				for (k = 0; k < 2; k++)
					LineDefP->VertexNum[k] = WL_StreamReadLittleUInt16(Stream);
				LineDefP->flags = WL_StreamReadLittleUInt16(Stream);
				
				if (a_Info->Type.Hexen)	// Hexen
				{
					LineDefP->HexenSpecial = WL_StreamReadUInt8(Stream);
					for (k = 0; k < 5; k++)
						LineDefP->ACSArgs[k] = WL_StreamReadUInt8(Stream);
				}
				else					// Doom
				{
					LineDefP->special = WL_StreamReadLittleUInt16(Stream);
					LineDefP->tag = WL_StreamReadLittleUInt16(Stream);
				}
				
				for (k = 0; k < 2; k++)
					LineDefP->sidenum[k] = WL_StreamReadLittleUInt16(Stream);
				
				// Initialize
				PS_ExLineDefInit(LineDefP);
			}
			
			// Close stream
			WL_StreamClose(Stream);
		}
	}
	
	// Load Sub-Sector Data
	CONL_LoadingScreenIncrMaj("Loading sub sectors", 0);
	if ((Entry = a_Info->EntryPtr[PLIEDS_SSECTORS]))
	{
		// Open stream
		Stream = WL_StreamOpen(Entry);
		
		// Read in data
		if (Stream)
		{
			// Allocate array
			numsubsectors = Entry->Size / 4;	// 1 subsector is 4 bytes
			subsectors = Z_Malloc(sizeof(*subsectors) * numsubsectors, PU_LEVEL, (void**)&subsectors);
			
			// Set loading screen info
			CONL_LoadingScreenSetSubEnd(numsubsectors >> LOADSHIFT);
			
			// Read in data
			for (i = 0; i < numsubsectors && !WL_StreamEOF(Stream); i++)
			{
				// Loading screen
				if ((i & LOADMASK) == 0)
					CONL_LoadingScreenIncrSub();
				
				// Get pointer
				SubSectorP = &subsectors[i];
				
				// Read
				SubSectorP->numlines = WL_StreamReadLittleUInt16(Stream);
				SubSectorP->firstline = WL_StreamReadLittleUInt16(Stream);
				
				// Initialize
				PS_ExSubSectorInit(SubSectorP);
			}
			
			// Close stream
			WL_StreamClose(Stream);
		}
	}
	
	// Load Node Data
	CONL_LoadingScreenIncrMaj("Loading nodes", 0);
	if ((Entry = a_Info->EntryPtr[PLIEDS_NODES]))
	{
		// Open stream
		Stream = WL_StreamOpen(Entry);
		
		// Read in data
		if (Stream)
		{
			// Allocate array
			numnodes = Entry->Size / 28;	// 1 node is 28 bytes
			nodes = Z_Malloc(sizeof(*nodes) * numnodes, PU_LEVEL, (void**)&nodes);
			
			// Set loading screen info
			CONL_LoadingScreenSetSubEnd(numnodes >> LOADSHIFT);
			
			// Read in data
			for (i = 0; i < numnodes && !WL_StreamEOF(Stream); i++)
			{
				// Loading screen
				if ((i & LOADMASK) == 0)
					CONL_LoadingScreenIncrSub();
				
				// Get pointer
				NodeP = &nodes[i];
				
				// Read
				NodeP->x = ((fixed_t)WL_StreamReadLittleInt16(Stream)) <<  FRACBITS;
				NodeP->y = ((fixed_t)WL_StreamReadLittleInt16(Stream)) <<  FRACBITS;
				NodeP->dx = ((fixed_t)WL_StreamReadLittleInt16(Stream)) <<  FRACBITS;
				NodeP->dy = ((fixed_t)WL_StreamReadLittleInt16(Stream)) <<  FRACBITS;
				
				for (k = 0; k < 2; k++)
					for (j = 0; j < 4; j++)
						NodeP->bbox[k][j] = ((fixed_t)WL_StreamReadLittleInt16(Stream)) <<  FRACBITS;
				
				for (k = 0; k < 2; k++)
					NodeP->children[k] = WL_StreamReadLittleUInt16(Stream);
				
				// Initialize
				PS_ExNodeInit(NodeP);
			}
			
			// Close stream
			WL_StreamClose(Stream);
		}
	}
	
	// Load Seg Data
	CONL_LoadingScreenIncrMaj("Loading segs", 0);
	if ((Entry = a_Info->EntryPtr[PLIEDS_SEGS]))
	{
		// Open stream
		Stream = WL_StreamOpen(Entry);
		
		// Read in data
		if (Stream)
		{
			// Allocate array
			numsegs = Entry->Size / 12;	// 1 seg is 12 bytes
			segs = Z_Malloc(sizeof(*segs) * numsegs, PU_LEVEL, (void**)&segs);
			
			// Set loading screen info
			CONL_LoadingScreenSetSubEnd(numsegs >> LOADSHIFT);
			
			// Read in data
			for (i = 0; i < numsegs && !WL_StreamEOF(Stream); i++)
			{
				// Loading screen
				if ((i & LOADMASK) == 0)
					CONL_LoadingScreenIncrSub();
				
				// Get pointer
				SegP = &segs[i];
				
				// Read
				for (k = 0; k < 2; k++)
					SegP->VertexID[k] = WL_StreamReadLittleUInt16(Stream);
				SegP->angle = ((angle_t)WL_StreamReadLittleUInt16(Stream)) << 16;
				SegP->LineID = WL_StreamReadLittleUInt16(Stream);
				SegP->side = WL_StreamReadLittleUInt16(Stream);
				SegP->offset = ((fixed_t)WL_StreamReadLittleInt16(Stream)) <<  FRACBITS;
				
				// Initialize
				PS_ExSegInit(SegP);
			}
			
			// Close stream
			WL_StreamClose(Stream);
		}
	}
	
	// Load the Block Map
	CONL_LoadingScreenIncrMaj("Loading block map", 0);
	if ((Entry = a_Info->EntryPtr[PLIEDS_BLOCKMAP]))
	{
		// Open stream
		Stream = WL_StreamOpen(Entry);
		
		// Read in data
		if (Stream)
		{
			// Determine count and allocate
			if (Entry->Size >= 8)	// Prevent overflow and explode
				k = (Entry->Size - 8) / 2;
			else
				k = 0;
			
			// Set loading screen info
			CONL_LoadingScreenSetSubEnd(k >> LOADSHIFT);
			
			blockmaplump = Z_Malloc(sizeof(*blockmap) * (k + 4), PU_LEVEL, (void**)&blockmap);
			blockmap = blockmaplump + 4;	// Needed for compat
			
			// Read blockmap origin
			blockmaplump[0] = WL_StreamReadLittleInt16(Stream);
			bmaporgx = ((fixed_t)blockmaplump[0]) <<  FRACBITS;
			blockmaplump[1] = WL_StreamReadLittleInt16(Stream);
			bmaporgy = ((fixed_t)blockmaplump[1]) <<  FRACBITS;
			
			// Read blockmap size
			blockmaplump[2] = bmapwidth = ((int32_t)WL_StreamReadLittleUInt16(Stream)) & 0xFFFFU;
			blockmaplump[3] = bmapheight = ((int32_t)WL_StreamReadLittleUInt16(Stream)) & 0xFFFFU;
			
			// Load remaining blockmap data
			for (i = 0; i < k; i++)
			{
				// Loading screen
				if ((i & LOADMASK) == 0)
					CONL_LoadingScreenIncrSub();
				
				// Load in
				TempShort = WL_StreamReadLittleInt16(Stream);
				
				// Keep -1, but drop everything else
				if (TempShort == -1)
					blockmap[i] = -1;
				else
					blockmap[i] = ((int32_t)TempShort) & 0xFFFF;
			}
			
			// Close stream
			WL_StreamClose(Stream);
		}
	}
	
	// Load the Reject
	CONL_LoadingScreenIncrMaj("Loading reject", 0);
	if ((Entry = a_Info->EntryPtr[PLIEDS_REJECT]))
	{
		// Open stream
		Stream = WL_StreamOpen(Entry);
		
		// Read in data
		if (Stream)
		{
			// matrix size is (numsectors * numsectors) / 8;
			k = ((numsectors * numsectors) / 8);
			rejectmatrix = Z_Malloc(sizeof(*rejectmatrix) * (k + 1), PU_LEVEL, (void**)&rejectmatrix);
			
			// Set loading screen info
			CONL_LoadingScreenSetSubEnd(k >> LOADSHIFT);
			
			// Read in matrix data
			for (j = 0; j < k; j++)
			{
				// Loading screen
				if ((j & LOADMASK) == 0)
					CONL_LoadingScreenIncrSub();
				
				rejectmatrix[j] = WL_StreamReadUInt8(Stream);
			}
			
			// Close stream
			WL_StreamClose(Stream);
		}
	}
	
	// Munge Node Data
	CONL_LoadingScreenIncrMaj("Merging renderer data", 0);
	PS_ExMungeNodeData();
	
	// Load Things (Non-Textual Format)
	CONL_LoadingScreenIncrMaj("Loading thing", 0);
	if (!a_Info->Type.Text && (Entry = a_Info->EntryPtr[PLIEDS_THINGS]))
	{
		// Open stream
		Stream = WL_StreamOpen(Entry);
		
		// Read in data
		if (Stream)
		{
			// Create map thing array
			nummapthings = Entry->Size / (a_Info->Type.Hexen ? 20 : 10);	// 1 thing is 10 bytes (doom) and 20 bytes (hexen)
			mapthings = Z_Malloc(sizeof(*mapthings) * nummapthings, PU_LEVEL, (void**)&mapthings);
			
			// Set loading screen info
			CONL_LoadingScreenSetSubEnd(nummapthings >> LOADSHIFT);
			
			// Read thing data
			for (i = 0; i < nummapthings && !WL_StreamEOF(Stream); i++)
			{
				// Loading screen
				if ((i & LOADMASK) == 0)
					CONL_LoadingScreenIncrSub();
				
				// Get pointer
				ThingP = &mapthings[i];
				
				// Read data
				ThingP->IsHexen = a_Info->Type.Hexen;
				
				if (ThingP->IsHexen)
					ThingP->ID = WL_StreamReadLittleUInt16(Stream);
				
				ThingP->x = WL_StreamReadLittleInt16(Stream);
				ThingP->y = WL_StreamReadLittleInt16(Stream);
				
				if (ThingP->IsHexen)
					ThingP->HeightOffset = WL_StreamReadLittleInt16(Stream);
				
				ThingP->angle = WL_StreamReadLittleInt16(Stream);
				ThingP->type = WL_StreamReadLittleInt16(Stream);
				ThingP->options = WL_StreamReadLittleInt16(Stream);
				
				if (ThingP->IsHexen)
				{
					ThingP->Special = WL_StreamReadUInt8(Stream);
					for (k = 0; k < 5; k++)
						ThingP->Args[k] = WL_StreamReadUInt8(Stream);
				}
				
				// Init
				PS_ExThingInit(ThingP);
			}
			
			// Close stream
			WL_StreamClose(Stream);
		}
	}
	
	/* Spawn map specials */
	P_SpawnSpecials();
	P_InitBrainTarget();
	
	/* Pre-Finalize */
	// Set the level time to zero
	D_SyncNetSetMapTime(0);
	
	/* Finalize */
	P_ExFinalizeLevel();
#undef BUFSIZE
#undef LOADMASK
#undef LOADSHIFT
}

/* P_ExFinalizeLevel() -- Finalizes the level so that it can be joined */
// Called by level loading and the save game loading code
bool_t P_ExFinalizeLevel(void)
{
	/* Set gamestate to level */
	// So that we can play it now
	gamestate = GS_LEVEL;
	gameaction = ga_nothing;
	
	/* Setup sky */
	P_SetupLevelSky();
	
	// Flat number is the holder F_SKY1
	skyflatnum = R_GetFlatNumForName("F_SKY1");
	
	/* Build Bot Nodes */
	B_InitNodes();
	
	/* Success! */
	return true;
}

