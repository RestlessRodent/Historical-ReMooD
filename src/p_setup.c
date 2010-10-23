// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.org/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           TetrisMaster512        (tetrismaster512@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2010 The ReMooD Team..
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
//      Do all the WAD I/O, get map description,
//             set up initial state and misc. LUTs.

#include "doomdef.h"
#include "c_lib.h"
#include "d_main.h"
#include "byteptr.h"
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

#ifdef _WIN32
#include "malloc.h"
#include "math.h"
#endif

#include "command.h"

//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//
boolean newlevel = false;
boolean doom1level = false;		// doom 1 level running under doom 2
char *levelmapname = NULL;

int numvertexes;
vertex_t *vertexes;

int numsegs;
seg_t *segs;

int numsectors;
sector_t *sectors;

int numsubsectors;
subsector_t *subsectors;

int numnodes;
node_t *nodes;

int numlines;
line_t *lines;

int numsides;
side_t *sides;

int nummapthings;
mapthing_t *mapthings;

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

long *blockmap;					// int for large maps
// offsets in blockmap are from here
long *blockmaplump;				// Big blockmap SSNTails

// origin of block map
fixed_t bmaporgx;
fixed_t bmaporgy;
// for thing chains
mobj_t **blocklinks;

// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without special effect, this could be
//  used as a PVS lookup as well.
//
byte *rejectmatrix;

// Maintain single and multi player starting spots.
mapthing_t *deathmatchstarts[MAX_DM_STARTS];
int numdmstarts;
//mapthing_t**    deathmatch_p;
mapthing_t *playerstarts[MAXPLAYERS];

//
// P_LoadVertexes
//
void P_LoadVertexes(int lump)
{
	byte *data;
	int i;
	mapvertex_t *ml;
	vertex_t *li;

	// Determine number of lumps:
	//  total lump length / vertex record length.
	numvertexes = W_LumpLength(lump) / sizeof(mapvertex_t);

	// Allocate zone memory for buffer.
	vertexes = Z_Malloc(numvertexes * sizeof(vertex_t), PU_LEVEL, 0);

	// Load data into cache.
	data = W_CacheLumpNum(lump, PU_STATIC);

	ml = (mapvertex_t *) data;
	li = vertexes;

	// Copy and convert vertex coordinates,
	// internal representation as fixed.
	for (i = 0; i < numvertexes; i++, li++, ml++)
	{
		li->x = SHORT(ml->x) << FRACBITS;
		li->y = SHORT(ml->y) << FRACBITS;
	}

	// Free buffer memory.
	Z_Free(data);
}

//
// Computes the line length in frac units, the glide render needs this
//
#define crapmul (1.0f / 65536.0f)

float P_SegLength(seg_t * seg)
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
	byte *data;
	int i;
	mapseg_t *ml;
	seg_t *li;
	line_t *ldef;
	int linedef;
	int side;

	numsegs = W_LumpLength(lump) / sizeof(mapseg_t);
	segs = Z_Malloc(numsegs * sizeof(seg_t), PU_LEVEL, 0);
	memset(segs, 0, numsegs * sizeof(seg_t));
	data = W_CacheLumpNum(lump, PU_STATIC);

	ml = (mapseg_t *) data;
	li = segs;
	for (i = 0; i < numsegs; i++, li++, ml++)
	{
		li->v1 = &vertexes[SHORT(ml->v1)];
		li->v2 = &vertexes[SHORT(ml->v2)];
		li->angle = (SHORT(ml->angle)) << 16;
		li->offset = (SHORT(ml->offset)) << 16;
		linedef = SHORT(ml->linedef);
		ldef = &lines[linedef];
		li->linedef = ldef;
		li->side = side = SHORT(ml->side);
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
	byte *data;
	int i;
	mapsubsector_t *ms;
	subsector_t *ss;

	numsubsectors = W_LumpLength(lump) / sizeof(mapsubsector_t);
	subsectors = Z_Malloc(numsubsectors * sizeof(subsector_t), PU_LEVEL, 0);
	data = W_CacheLumpNum(lump, PU_STATIC);

	ms = (mapsubsector_t *) data;
	memset(subsectors, 0, numsubsectors * sizeof(subsector_t));
	ss = subsectors;

	for (i = 0; i < numsubsectors; i++, ss++, ms++)
	{
		ss->numlines = SHORT(ms->numsegs);
		ss->firstline = SHORT(ms->firstseg);
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
levelflat_t *levelflats;

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

int P_FlatNumForName(char *flatname)
{
	return P_AddLevelFlat(flatname, levelflats);
}

// help function for P_LoadSectors, find a flat in the active wad files,
// allocate an id for it, and set the levelflat (to speedup search)
//
int P_AddLevelFlat(char *flatname, levelflat_t * levelflat)
{
	union
	{
		char s[9];
		int x[2];
	} name8;

	int i;
	UInt32 v1, v2;

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
		if (*(UInt32 *) levelflat->name == v1 && *(UInt32 *) & levelflat->name[4] == v2)
		{
			break;
		}
	}

	// that flat was already found in the level, return the id
	if (i == numlevelflats)
	{
		// store the name
		*((size_t *) levelflat->name) = v1;
		*((size_t *) & levelflat->name[4]) = v2;

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
char *P_FlatNameForNum(int num)
{
	if (num < 0 || num > numlevelflats)
		I_Error("P_FlatNameForNum: Invalid flatnum\n");

	return Z_Strdup(va("%.8s", levelflats[num].name), PU_STATIC, 0);
}

void P_LoadSectors(int lump)
{
	byte *data;
	int i, j;
	mapsector_t *ms;
	sector_t *ss;
	char newflat1[10];
	char newflat2[10];
	char *buf;

	levelflat_t *foundflats;

	numsectors = W_LumpLength(lump) / sizeof(mapsector_t);
	sectors = Z_Malloc(numsectors * sizeof(sector_t), PU_LEVEL, &sectors);
	memset(sectors, 0, numsectors * sizeof(sector_t));
	data = W_CacheLumpNum(lump, PU_STATIC);

	//Fab:FIXME: allocate for whatever number of flats
	//           512 different flats per level should be plenty
	foundflats = Z_Malloc(sizeof(levelflat_t) * MAXLEVELFLATS, PU_LEVEL, &foundflats);
	memset(foundflats, 0, sizeof(levelflat_t) * MAXLEVELFLATS);

	numlevelflats = 0;

	ms = (mapsector_t *) data;
	ss = sectors;
	for (i = 0; i < numsectors; i++, ss++, ms++)
	{
		ss->floorheight = SHORT(ms->floorheight) << FRACBITS;
		ss->ceilingheight = SHORT(ms->ceilingheight) << FRACBITS;

		//
		//  flats
		//
		if (strnicmp(ms->floorpic, "FWATER", 6) == 0 ||
			strnicmp(ms->floorpic, "FLTWAWA1", 8) == 0 ||
			strnicmp(ms->floorpic, "FLTFLWW1", 8) == 0)
			ss->floortype = FLOOR_WATER;
		else if (strnicmp(ms->floorpic, "FLTLAVA1", 8) == 0 ||
				 strnicmp(ms->floorpic, "FLATHUH1", 8) == 0)
			ss->floortype = FLOOR_LAVA;
		else if (strnicmp(ms->floorpic, "FLTSLUD1", 8) == 0)
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
#else*/
		// Insecure "Safe" strncpy -- Self implementation
		buf = ms->floorpic;
		j = 0;
		while (*buf && j < 8)
		{
			newflat1[j] = *buf;
			j++;
			buf++;
		}
		newflat1[8] = 0;

		buf = ms->ceilingpic;
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

		ss->lightlevel = SHORT(ms->lightlevel);
		ss->special = SHORT(ms->special);
		ss->tag = SHORT(ms->tag);

		//added:31-03-98: quick hack to test water with DCK
/*        if (ss->tag < 0)
            CONS_Printf("Level uses dck-water-hack\n");*/

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
	//CONS_Printf ("%d flats found\n", numlevelflats);

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
	byte *data;
	int i;
	int j;
	int k;
	mapnode_t *mn;
	node_t *no;

	numnodes = W_LumpLength(lump) / sizeof(mapnode_t);
	nodes = Z_Malloc(numnodes * sizeof(node_t), PU_LEVEL, 0);
	data = W_CacheLumpNum(lump, PU_STATIC);

	mn = (mapnode_t *) data;
	no = nodes;

	for (i = 0; i < numnodes; i++, no++, mn++)
	{
		no->x = SHORT(mn->x) << FRACBITS;
		no->y = SHORT(mn->y) << FRACBITS;
		no->dx = SHORT(mn->dx) << FRACBITS;
		no->dy = SHORT(mn->dy) << FRACBITS;
		for (j = 0; j < 2; j++)
		{
			no->children[j] = SHORT(mn->children[j]);
			for (k = 0; k < 4; k++)
				no->bbox[j][k] = SHORT(mn->bbox[j][k]) << FRACBITS;
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
	mapthing_t *mt;
	boolean spawn;
	Int16 *data, *datastart;

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

//
// P_LoadLineDefs
// Also counts secret lines for intermissions.
//
void P_LoadLineDefs(int lump)
{
	byte *data;
	int i;
	maplinedef_t *mld;
	line_t *ld;
	vertex_t *v1;
	vertex_t *v2;

	numlines = W_LumpLength(lump) / sizeof(maplinedef_t);
	lines = Z_Malloc(numlines * sizeof(line_t), PU_LEVEL, 0);
	memset(lines, 0, numlines * sizeof(line_t));
	data = W_CacheLumpNum(lump, PU_STATIC);

	mld = (maplinedef_t *) data;
	ld = lines;
	for (i = 0; i < numlines; i++, mld++, ld++)
	{
		ld->flags = SHORT(mld->flags);
		ld->special = SHORT(mld->special);
		ld->tag = SHORT(mld->tag);
		v1 = ld->v1 = &vertexes[SHORT(mld->v1)];
		v2 = ld->v2 = &vertexes[SHORT(mld->v2)];
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

		ld->sidenum[0] = SHORT(mld->sidenum[0]);
		ld->sidenum[1] = SHORT(mld->sidenum[1]);

		if (ld->sidenum[0] != -1 && ld->special)
			sides[ld->sidenum[0]].special = ld->special;

	}

	Z_Free(data);
}

void P_LoadLineDefs2()
{
	int i;
	line_t *ld = lines;
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
    byte*               data;
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
        sd->textureoffset = SHORT(msd->textureoffset)<<FRACBITS;
        sd->rowoffset = SHORT(msd->rowoffset)<<FRACBITS;
        sd->toptexture = R_TextureNumForName(msd->toptexture);
        sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
        sd->midtexture = R_TextureNumForName(msd->midtexture);

        sd->sector = &sectors[SHORT(msd->sector)];
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
int R_ColormapNumForName(char *name);

void P_LoadSideDefs2(int lump)
{
	byte *data = W_CacheLumpNum(lump, PU_STATIC);
	int i;
	int num;
	int mapnum;

	for (i = 0; i < numsides; i++)
	{
		register mapsidedef_t *msd = (mapsidedef_t *) data + i;
		register side_t *sd = sides + i;
		register sector_t *sec;

		sd->textureoffset = SHORT(msd->textureoffset) << FRACBITS;
		sd->rowoffset = SHORT(msd->rowoffset) << FRACBITS;

		// refined to allow colormaps to work as wall
		// textures if invalid as colormaps but valid as textures.

		sd->sector = sec = &sectors[SHORT(msd->sector)];
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
					sec->midmap =
						R_CreateColormap(msd->toptexture, msd->midtexture, msd->bottomtexture);
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
																												          break;*///This code is replaced.. I need to fix this though

				//Hurdler: added for alpha value with translucent 3D-floors/water
			case 300:
			case 301:
				if (msd->toptexture[0] == '#')
				{
					char *col = msd->toptexture;
					sd->toptexture = sd->bottomtexture =
						((col[1] - '0') * 100 + (col[2] - '0') * 10 + col[3] - '0') + 1;
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
	Int32 count;

	count = W_LumpLength(lump) / 2;
	{
		Int32 i;
		Int16 *wadblockmaplump = W_CacheLumpNum(lump, PU_LEVEL);
		blockmaplump = Z_Malloc(sizeof(*blockmaplump) * count, PU_LEVEL, 0);

		// killough 3/1/98: Expand wad blockmap into larger internal one,
		// by treating all offsets except -1 as unsigned and zero-extending
		// them. This potentially doubles the size of blockmaps allowed,
		// because Doom originally considered the offsets as always signed.

		blockmaplump[0] = SHORT(wadblockmaplump[0]);
		blockmaplump[1] = SHORT(wadblockmaplump[1]);
		blockmaplump[2] = (Int32) (SHORT(wadblockmaplump[2])) & 0xffff;
		blockmaplump[3] = (Int32) (SHORT(wadblockmaplump[3])) & 0xffff;

		for (i = 4; i < count; i++)
		{
			Int16 t = SHORT(wadblockmaplump[i]);	// killough 3/1/98
			blockmaplump[i] = t == -1 ? -1l : (Int32) t & 0xffff;
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
			blockmaplump[i] = SHORT(blockmaplump[i]);

		bmaporgx = blockmaplump[0]<<FRACBITS;
		bmaporgy = blockmaplump[1]<<FRACBITS;
		bmapwidth = blockmaplump[2];
		bmapheight = blockmaplump[3];
	}

	// clear out mobj chains
	count = sizeof(*blocklinks)*bmapwidth*bmapheight;
	blocklinks = Z_Malloc (count,PU_LEVEL, 0);
	memset (blocklinks, 0, count);*/
}

//
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
//
void P_GroupLines(void)
{
	line_t **linebuffer;
	int i;
	int j;
	int total;
	line_t *li;
	sector_t *sector;
	subsector_t *ss;
	seg_t *seg;
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
	linebuffer = Z_Malloc(total * sizeof(void *), PU_LEVEL, 0);
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
char *levellumps[] = {
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
// P_CheckLevel
// Checks a lump and returns weather or not it is a level header lump.
boolean P_CheckLevel(int lumpnum)
{
	int i = 0;
	WadEntry_t *Entry = W_GetEntry(lumpnum);

	if (Entry == NULL)
		return false;

	for (i = ML_THINGS; i <= ML_BLOCKMAP; i++)
	{
		Entry = W_GetEntry(lumpnum + i);

		if (strncmp(Entry->Name, levellumps[i], 8) != 0)
			return false;
	}

	return true;

	/*int  i;
	   int  file, lump;

	   for(i=ML_THINGS; i<=ML_BLOCKMAP; i++)
	   {
	   file = lumpnum >> 16;
	   lump = (lumpnum & 0xffff) + i;
	   if(file > numwadfiles || lump > wadfiles[file]->numlumps ||
	   strncmp(wadfiles[file]->lumpinfo[lump].name, levellumps[i], 8) )
	   return false;
	   }
	   return true;    // all right */
}

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

	if (*info_skyname)
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
char *maplumpname;

int lastloadedmaplumpnum;		// for comparative savegame
boolean P_SetupLevel(int episode, int map, skill_t skill, char *wadname)	// for wad files
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

#if 0							// UNUSED
	if (debugfile)
	{
		Z_FreeTags(PU_LEVEL, MAXINT);
		Z_FileDumpHeap(debugfile);
	}
	else
#endif
	
	Z_FreeTags(PU_LEVEL, PU_PURGELEVEL - 1);

#ifdef WALLSPLATS
	// clear the splats from previous level
	R_ClearLevelSplats();
#endif

	script_camera_on = false;
	if (M_CheckParm("-newscripting"))
		RMD_ClearOldLevelCode();
	HU_ClearTips();

	if (camera.chase)
		camera.mo = NULL;

	// UNUSED W_Profile ();

	P_InitThinkers();

	// if working with a devlopment map, reload it
	W_Reload();

	//
	//  load the map from internal game resource or external wad file
	//
	if (wadname)
	{
		char *firstmap = NULL;

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

	// textures are needed first
//    R_LoadTextures ();
//    R_FlushTextureCache();

	R_ClearColormaps();
	
	// GhostlyDeath <December 21, 2008> -- Check for HeXeN Map
	HexenACS = W_GetEntry(lastloadedmaplumpnum + ML_BEHAVIOR);
	if (HexenACS && strncasecmp(HexenACS->Name, "BEHAVIOR", 8) == 0)
	{
		CONS_Printf("P_SetupLevel: HeXeN maps are NOT supported!\n");
		return false;
	}
	
#ifdef FRAGGLESCRIPT
	P_LoadLevelInfo(lastloadedmaplumpnum);	// load level lump info(level name etc)
#endif

	//SoM: We've loaded the music lump, start the music.
	S_Start();

	//faB: now part of level loading since in future each level may have
	//     its own anim texture sequences, switches etc.
	P_InitAmbientSound();
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
			else if (demoversion >= 128 && !localgame)
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
	if (M_CheckParm("-newscripting"))
		RMD_CompileLegacyScript(lastloadedmaplumpnum);
	else
		T_PreprocessScripts();		// preprocess FraggleScript scripts

	script_camera_on = false;
	
#ifdef _DEBUG
	Z_CheckHeap(10);
#endif

	//CONS_Printf("%d vertexs %d segs %d subsector\n",numvertexes,numsegs,numsubsectors);
	return true;
}

//
// Add a wadfile to the active wad files,
// replace sounds, musics, patches, textures, sprites and maps
//
boolean P_AddWadFile(char *wadfilename, char **firstmapname)
{
	WadEntry_t *LumpEntry = NULL;
	WadFile_t *WAD = W_GetWadForName(wadfilename);
	char *Name;
	char *name;
	size_t i, j, k, num;
	int firstmapreplaced;
	UInt32 SoundReplacements;
	UInt32 MusicReplacements;
	boolean TextChange;

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
				if ((S_sfx[j].name) && !(S_sfx[j].link) &&
					(strncasecmp(Name[2], S_sfx[j].name, 6) == 0))
				{
					I_FreeSfx(&S_sfx[j]);
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

	CONS_Printf("P_AddWadFile: %i sounds replaced\n", SoundReplacements);
	CONS_Printf("P_AddWadFile: %i songs replaced\n", MusicReplacements);

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

	if (TextChange)				// inited in the sound check
		R_LoadTextures();		// numtexture changes
	else
		R_FlushTextureCache();	// just reload it from file

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
				CONS_Printf("Map %d\n", num);
			}
		}
		else					// Something else
		{
			if (name[0] == 'E' && ((unsigned)name[1] - '0') <= '9' &&	// a digit
				name[2] == 'M' && ((unsigned)name[3] - '0') <= '9' && name[4] == 0)
			{
				num = ((name[1] - '0') << 16) + (name[3] - '0');
				CONS_Printf("Episode %d map %d\n", name[1] - '0', name[3] - '0');
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
		CONS_Printf("no maps added\n");
		
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
	   boolean     texturechange;

	   if ((wadfilenum = W_LoadWadFile (wadfilename))==-1)
	   {
	   CONS_Printf ("couldn't load wad file %s\n", wadfilename);
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
	   !strnicmp(S_sfx[j].name,name+2,6) )
	   {
	   // the sound will be reloaded when needed,
	   // since sfx->data will be NULL
	   if (devparm)
	   CONS_Printf ("Sound %.8s replaced\n", name);

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
	   CONS_Printf ("%d sounds replaced\n", replaces);

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
	   CONS_Printf ("Music %.8s replaced\n", name);
	   replaces++;
	   }
	   }
	   if (!devparm && replaces)
	   CONS_Printf ("%d musics replaced\n", replaces);

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
	   CONS_Printf ("Map %d\n", num);
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
	   CONS_Printf ("Episode %d map %d\n", name[1]-'0',
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
	   CONS_Printf ("no maps added\n");

	   // reload status bar (warning should have valide player !)
	   if( gamestate == GS_LEVEL )
	   ST_Start();

	   return true; */
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

typedef struct COMMONSeg_s
{
	UInt16 v1;
	UInt16 v2;
	UInt16 Angle;
	UInt16 LineDef;
	UInt16 Direction;
	UInt16 Length;
} COMMONSeg_t;

COMMONSeg_t* CommonSegs = NULL;
size_t NumCommonSegs = 0;

boolean P_COMMON_LoadBlockmap(WadEntry_t* Entry)
{
	CONS_Printf("P_COMMON_LoadBlockmap: Warning! Calling deprecated function P_LoadBlockMap.\n");
	P_LoadBlockMap(W_LumpsSoFar(Entry->Host) + (Entry - Entry->Host->Index));
	return true;
}

boolean P_COMMON_LoadSubSectors(WadEntry_t* Entry)
{
	CONS_Printf("P_COMMON_LoadSubSectors: Warning! Calling deprecated function P_LoadSubsectors.\n");
	P_LoadSubsectors(W_LumpsSoFar(Entry->Host) + (Entry - Entry->Host->Index));
	return true;
}

boolean P_COMMON_LoadSegs(WadEntry_t* Entry)
{
	return false;
}

boolean P_COMMON_LoadNodes(WadEntry_t* Entry)
{
	return false;
}

boolean P_COMMON_LoadReject(WadEntry_t* Entry)
{
	return false;
}

boolean P_COMMON_LoadVertexes(WadEntry_t* Entry)
{
	return false;
}

boolean P_COMMON_LoadSideDefs(WadEntry_t* Entry)
{
	return false;
}

boolean P_COMMON_LoadSectors(WadEntry_t* Entry)
{
	return false;
}

boolean P_HEXEN_LoadThings(WadEntry_t* Entry)
{
	return false;
}

boolean P_HEXEN_LoadLineDefs(WadEntry_t* Entry)
{
	return false;
}

boolean P_DOOM_LoadThings(WadEntry_t* Entry)
{
	return false;
}

boolean P_DOOM_LoadLineDefs(WadEntry_t* Entry)
{
	return false;
}

boolean P_COMMON_Realize(void)
{
	return false;
}

boolean P_GLOBAL_CreateLevel(skill_t skill, char* mapname, char *wadname)
{
	int i;
	WadIndex_t LevelBase = INVALIDLUMP;
	WadEntry_t* MPBase = NULL;
	WadEntry_t* MPTextOrThing = NULL;
	WadEntry_t* MPLineDefs = NULL;
	WadEntry_t* MPSideDefs = NULL;
	WadEntry_t* MPVertexes = NULL;
	WadEntry_t* MPSegs = NULL;
	WadEntry_t* MPSubSectors = NULL;
	WadEntry_t* MPNodes = NULL;
	WadEntry_t* MPSectors = NULL;
	WadEntry_t* MPReject = NULL;
	WadEntry_t* MPBlockmap = NULL;
	WadEntry_t* MPBehavior = NULL;
	
#define MPText MPTextOrThing
#define MPThings MPTextOrThing
	
	/*** Remove the old level ***/
	CON_Drawer();				// let the user know what we are going to do
	I_FinishUpdate();			// page flip or blit buffer
	P_Initsecnode();			//Initialize sector node list.
	S_StopSounds();				// Make sure all sounds are stopped before Z_FreeTags.
	if (M_CheckParm("-newscripting"))
		RMD_ClearOldLevelCode();
	R_ClearLevelSplats();
	HU_ClearTips();
	P_InitThinkers();
	W_Reload();
	R_ClearColormaps();
	Z_FreeTags(PU_LEVEL, PU_PURGELEVEL - 1);

	script_camera_on = false;
	players[consoleplayer[0]].viewz = 1;
	if (camera.chase)
		camera.mo = NULL;
	totalkills = totalitems = totalsecret = wminfo.maxfrags = 0;
	wminfo.partime = 180;
	for (i = 0; i < MAXPLAYERS; i++)
	{
		players[i].killcount = players[i].secretcount = players[i].itemcount = 0;
		players[i].mo = NULL;
	}
	
	/*** Prepare to create the level ***/
	/* We need to get the actual map */
	LevelBase = W_CheckNumForName(mapname);
	
	if (LevelBase == INVALIDLUMP)
	{
		CONS_Printf("P_GLOBAL_CreateLevel: Lump \"%s\" not found.\n", mapname);
		return false;
	}
	
	MPBase = W_GetEntry(LevelBase);
	
	bodyqueslot = 0;
	numdmstarts = 0;
	iquehead = iquetail = 0;
	for (i = 0; i < MAXPLAYERS; i++)
		playerstarts[i] = NULL;
	
	/* Determine the type of the level */
	MPTextOrThing = W_GetEntry(LevelBase + 1);
	MPLineDefs = W_GetEntry(LevelBase + 2);
	MPSideDefs = W_GetEntry(LevelBase + 3);
	MPVertexes = W_GetEntry(LevelBase + 4);
	MPSegs = W_GetEntry(LevelBase + 5);
	MPSubSectors = W_GetEntry(LevelBase + 6);
	MPNodes = W_GetEntry(LevelBase + 7);
	MPSectors = W_GetEntry(LevelBase + 8);
	MPReject = W_GetEntry(LevelBase + 9);
	MPBlockmap = W_GetEntry(LevelBase + 10);
	MPBehavior = W_GetEntry(LevelBase + 11);
	
	//// UDMF Format ////
	if (strcasecmp(MPText->Name, "TEXTMAP") == 0)
	{
		CONS_Printf("P_GLOBAL_CreateLevel: UDMF Maps are not yet supported.\n");
		return false;
	}
	else
	{
		// Check to see if the minimum entries are valid
		if (!((MPThings && strcasecmp(MPThings->Name, "THINGS") == 0) &&
			(MPLineDefs && strcasecmp(MPLineDefs->Name, "LINEDEFS") == 0) &&
			(MPSideDefs && strcasecmp(MPSideDefs->Name, "SIDEDEFS") == 0) &&
			(MPVertexes && strcasecmp(MPVertexes->Name, "VERTEXES") == 0) &&
			(MPSegs && strcasecmp(MPSegs->Name, "SEGS") == 0) &&
			(MPSubSectors && strcasecmp(MPSubSectors->Name, "SSECTORS") == 0) &&
			(MPNodes && strcasecmp(MPNodes->Name, "NODES") == 0) &&
			(MPSectors && strcasecmp(MPSectors->Name, "SECTORS") == 0) &&
			(MPReject && strcasecmp(MPReject->Name, "REJECT") == 0) &&
			(MPBlockmap && strcasecmp(MPBlockmap->Name, "BLOCKMAP") == 0)))
		{
			CONS_Printf("P_GLOBAL_CreateLevel: Invalid level or level data is out of order.\n");
			return false;
		}
	
		//// COMMON FORMAT ////
		if (!P_COMMON_LoadBlockmap(MPBlockmap))
		{
			CONS_Printf("P_GLOBAL_CreateLevel: Blockmap data is corrupt.\n");
			return false;
		}
		if (!P_COMMON_LoadSubSectors(MPSubSectors))
		{
			CONS_Printf("P_GLOBAL_CreateLevel: Subsector data is corrupt.\n");
			return false;
		}
		if (!P_COMMON_LoadSegs(MPSegs))
		{
			CONS_Printf("P_GLOBAL_CreateLevel: Seg data is corrupt.\n");
			return false;
		}
		if (!P_COMMON_LoadNodes(MPNodes))
		{
			CONS_Printf("P_GLOBAL_CreateLevel: Node data is corrupt.\n");
			return false;
		}
		if (!P_COMMON_LoadReject(MPReject))
		{
			CONS_Printf("P_GLOBAL_CreateLevel: Reject data is corrupt.\n");
			return false;
		}
		if (!P_COMMON_LoadVertexes(MPVertexes))
		{
			CONS_Printf("P_GLOBAL_CreateLevel: Vertex data is corrupt.\n");
			return false;
		}
		if (!P_COMMON_LoadSideDefs(MPSideDefs))
		{
			CONS_Printf("P_GLOBAL_CreateLevel: SideDef data is corrupt.\n");
			return false;
		}
		if (!P_COMMON_LoadSectors(MPSectors))
		{
			CONS_Printf("P_GLOBAL_CreateLevel: Vertex data is corrupt.\n");
			return false;
		}
	
		//// HEXEN FORMAT ////
		if (strcasecmp(MPBehavior->Name, "BEHAVIOR") == 0)
		{
			if (!P_HEXEN_LoadLineDefs(MPLineDefs))
			{
				CONS_Printf("P_GLOBAL_CreateLevel: Hexen LineDef data corrupt.\n");
				return false;
			}
			if (!P_HEXEN_LoadThings(MPThings))
			{
				CONS_Printf("P_GLOBAL_CreateLevel: Hexen Thing data corrupt.\n");
				return false;
			}
		}
	
		//// DOOM FORMAT ////
		else
		{
			if (!P_DOOM_LoadLineDefs(MPLineDefs))
			{
				CONS_Printf("P_GLOBAL_CreateLevel: Doom LineDef data corrupt.\n");
				return false;
			}
			if (!P_DOOM_LoadThings(MPThings))
			{
				CONS_Printf("P_GLOBAL_CreateLevel: Doom Thing data corrupt.\n");
				return false;
			}
		}
	}
	
	if (!P_COMMON_Realize())
	{
		CONS_Printf("P_GLOBAL_CreateLevel: Level could not be realized.\n");
		return false;
	}
	
	return true;
}

