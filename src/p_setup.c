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
//      Do all the WAD I/O, get map description,
//             set up initial state and misc. LUTs.

#include "p_setup.h"
#include "r_defs.h"
#include "z_zone.h"
#include "r_state.h"
#include "p_demcmp.h"
#include "t_func.h"
#include "t_script.h"
#include "t_comp.h"
#include "m_bbox.h"
#include "r_sky.h"
#include "d_netcmd.h"
#include "p_spec.h"
#include "r_data.h"
#include "s_sound.h"
#include "screen.h"	// for skycolfunc
#include "t_vm.h"
#include "r_splats.h"

//#include "doomdef.h"
//#include "c_lib.h"
//#include "d_main.h"
//#include "g_game.h"

//#include "p_local.h"
//#include "p_setup.h"
//#include "p_spec.h"

//#include "i_sound.h"			//for I_PlayCD()..

//#include "r_data.h"
//#include "r_things.h"
//#include "r_sky.h"

//#include "s_sound.h"
//#include "st_stuff.h"
//#include "w_wad.h"
//#include "z_zone.h"
//#include "r_splats.h"
//#include "p_info.h"

//#include "hu_stuff.h"
//#include "console.h"

#ifdef _WIN32
//#include "malloc.h"
//#include "math.h"
#endif

//#include "sn.h"


//#include "p_demcmp.h"
//#include "b_bot.h"
//#include "t_dsvm.h"
//#include "t_vm.h"

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

extern mobj_t* g_LFPRover;

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

int32_t* blockmap;					// int for large maps

// offsets in blockmap are from here
int32_t* blockmaplump;				// Big blockmap SSNTails
size_t g_BMLSize = 0;							// Block map lump size

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
uint8_t* rejectmatrix = NULL;
size_t g_RJMSize = 0;							// Size of reject matrix

// Maintain single and multi player starting spots.
mapthing_t* deathmatchstarts[MAX_DM_STARTS];
int numdmstarts;

//mapthing_t**    deathmatch_p;
mapthing_t* playerstarts[MAXPLAYERS];

mapthing_t* g_TeamStarts[MAXSKINCOLORS][MAXPLAYERS];

int32_t g_MapKIS[5] = {0, 0, 0, 0, 0};

void P_SetupLevelSky(void)
{
	/* Load Sky */
	if (g_CurrentLevelInfo)
		if (g_CurrentLevelInfo->SkyTexture)
			skytexture = R_TextureNumForName(g_CurrentLevelInfo->SkyTexture);
	
	/* Sky Column Function */
	skytexturemid = 100 << FRACBITS;
	skymode = 0;
	skycolfunc = skydrawerfunc[skymode];
	
	/* scale up the old skies, if needed */
	R_SetSkyScale();
}

/*******************************************************************************
********************************************************************************
*******************************************************************************/

/*** STRUCTURES ***/

/*** FUNCTIONS ***/

/* PCLC_Map() -- Switch to another map */
static int PCLC_Map(const uint32_t a_ArgC, const char** const a_ArgV)
{
	P_LevelInfoEx_t* Info;
	size_t i;
	bool_t LevelSwitch = false;
	
	/* Check */
	if (a_ArgC < 2)
		return 1;
	
	/* Request Map */
	Info = P_FindLevelByNameEx(a_ArgV[1], NULL);
	
	// Not found?
	if (!Info)
		return 1;
	
	// Send request
	//D_XNetChangeMap(a_ArgV[1], !!(!strcasecmp(a_ArgV[0], "map")));
	
	// Return success
	return 0;
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
	
	/* De-initialize Fake Player */
	P_SpecInit(-3);
	
	/* Stop Playing Music and Sound */
	S_StopSounds();
	//S_StopMusic();
	
	/* Clear Stuff */
	S_StopSounds();
	R_ClearLevelSplats();
	P_ClearRecursiveSound();
	//B_ClearNodes();
	EV_ClearACSTags();
	P_RemoveAllActivePlats();
	P_RemoveAllActiveCeilings();
	
	/* Free all level tags */
	Z_FreeTags(PU_LEVEL, PU_ENDLEVELTAGS);
	
	/* Clear split automap zoom */
	for (i = 0; i < MAXSPLITSCREEN; i++)
		g_Splits[i].MapZoom = 0;
	
	/* Wipe Player Stuff */
	for (i = 0; i < MAXPLAYERS; i++)
	{
		// Cameras
		if (players[i].camera.chase)
			players[i].camera.mo = NULL;
			
		// Player Stats
		players[i].killcount = 0;
		players[i].itemcount = 0;
		players[i].secretcount = 0;
		players[i].TotalDeaths = 0;
		players[i].TotalFrags = 0;
		players[i].addfrags = 0;
		players[i].mo = NULL;
		
		memset(players[i].frags, 0, sizeof(players[i].frags));
		
		// Junk
		players[i].KeyCards[0] = 0;
		players[i].KeyCards[1] = 0;
	}
	
	/* Re-initialize */
	P_Initsecnode();
	P_InitThinkers();
	P_InitSwitchList();
	P_InitPicAnims();
	P_SetupLevelFlatAnims();
	
	/* Re-init some things */
	// Body Queue
	memset(bodyque, 0, sizeof(bodyque));
	bodyqueslot = true;
	
	// Respawn queue
	memset(itemrespawnque, 0, sizeof(itemrespawnque));
	memset(itemrespawntime, 0, sizeof(itemrespawntime));
	iquehead = iquetail = 0;
	
	// Totals
	totalkills = 0;
	totalitems = 0;
	totalsecret = 0;
	memset(g_MapKIS, 0, sizeof(g_MapKIS));
	
	// Time
	leveltime = 0;
	
	// Map Data
	nummapthings = numvertexes = numsegs = numsectors = numsubsectors = numnodes = numlines = numsides = 0;
	mapthings = vertexes = segs = sectors = subsectors = nodes = lines = sides = NULL;
	
	// Starts
	memset(deathmatchstarts, 0, sizeof(deathmatchstarts));
	memset(playerstarts, 0, sizeof(playerstarts));
	memset(g_TeamStarts, 0, sizeof(g_TeamStarts));
	numdmstarts = 0;
	
	// Flags
	memset(g_CTFFlags, 0, sizeof(g_CTFFlags));
	
	// 3D Floors
	g_PFakeFloors = NULL;
	g_NumPFakeFloors = 0;
	spechit_max = NULL;
	spechit = NULL;
	numspechit = 0;
	
	// LFP Rover
	g_LFPRover = &thinkercap;
	
	// Cheats
	//g_CheatFlags = 0;
	
	/* Scripting */
	TVM_Clear(TVMNS_LEVEL);
	
	/* Scores */
	P_UpdateScores();
	
	/* Switch back to waiting for players screen */
	if (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION)
		gamestate = GS_WAITINGPLAYERS;
	
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
		
	/* Re-Map Specials */
#if 0
	a_Sector->special = EV_DoomToGenTrigger(true, a_Sector->special);
#endif
	
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
	
	/* Incorrectly set two sided line? */
	if ((a_LineDef->flags & ML_TWOSIDED) && ((!a_LineDef->frontsector && a_LineDef->backsector) || (a_LineDef->frontsector && !a_LineDef->backsector)))
	{
		// Clear two sided bit
		a_LineDef->flags &= ~ML_TWOSIDED;
		
		// Back but no front?
		if (!a_LineDef->frontsector)
		{
			a_LineDef->frontsector = a_LineDef->backsector;
			a_LineDef->backsector = NULL;
		}
	}
	
	/* Calculate the real line special (generalized) */
#if 0
	// Hexen
	if (a_LineDef->HexenSpecial)
		a_LineDef->special = HEXENSPECIALLINE;
	// Doom
	else
		a_LineDef->special = EV_DoomToGenTrigger(false, a_LineDef->special);
#endif
	
	/* Set side special from linedef */
	//if (ld->sidenum[0] != -1 && ld->special)
	//	sides[ld->sidenum[0]].special = ld->special;
	
	/* Change exit switch sound */
	if (((a_LineDef->special & EVGENHE_TYPEMASK) >> EVGENHE_TYPESHIFT) == EVGHET_XEXIT)
		a_LineDef->SwitchSounds[0] = sfx_generic_switchoff;
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
static void PS_ExThingInit(mapthing_t* const a_Thing, const uint32_t a_Flags)
{
	/* Check */
	if (!a_Thing)
		return;
	
	/* Spawn it */
	if (!(a_Flags & PEXLL_NOSPAWNMAPTHING))
		P_SpawnMapThing(a_Thing);	
}

fixed_t g_GlobalBoundBox[4];					// Global bounding box

/* PS_ExMungeNodeData() -- Munges together node data */
void PS_ExMungeNodeData(void)
{
	line_t** LineBuffer;
	size_t i, j, k, Total, OldCount;
	fixed_t BBox[4];
	sector_t* SectorP, *OtherSec;
	int block;
	
	/* Set loading screen info */
	CONL_LoadingScreenSetSubEnd(4);
	
	/* Global Bound Box */
	CONL_LoadingScreenIncrSub();
	
	// Clear box
	M_ClearBox(g_GlobalBoundBox);
	
	// Add to global bounding box
	for (i = 0; i < numvertexes; i++)
		M_AddToBox(g_GlobalBoundBox, vertexes[i].x, vertexes[i].y);
	
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
			
			// GhostlyDeath <May 20, 2012> -- Adjacent sector list
				// Get opposite end
			if (lines[i].frontsector == SectorP)
				OtherSec = lines[i].backsector;
			else
				OtherSec = lines[i].frontsector;
			
				// The other sector is different and is valid
			if (SectorP && OtherSec && OtherSec != SectorP)
			{
				// Find other sector in our own chain
				for (k = 0; k < SectorP->NumAdj; k++)
					if (OtherSec == SectorP->Adj[k])
						break;
				
				// Not found? Add to the end
				if (k >= SectorP->NumAdj)
				{
					Z_ResizeArray((void**)&SectorP->Adj, sizeof(*SectorP->Adj),
							SectorP->NumAdj, SectorP->NumAdj + 1);
					SectorP->Adj[SectorP->NumAdj++] = OtherSec;
					Z_ChangeTag(SectorP->Adj, PU_LEVEL);
				}
			}
			
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
bool_t P_ExLoadLevel(P_LevelInfoEx_t* const a_Info, const uint32_t a_Flags)
{
#define BUFSIZE 512
#define LOADSHIFT 6
#define LOADMASK ((1 << LOADSHIFT) - 1)
	const WL_WADEntry_t* Entry;
	WL_ES_t* Stream;
	vertex_t* VertexP;
	sector_t* SectorP;
	side_t* SideDefP;
	line_t* LineDefP;
	subsector_t* SubSectorP;
	node_t* NodeP;
	seg_t* SegP;
	mapthing_t* ThingP;
	int32_t i, j, k, l;
	char Buf[BUFSIZE];
	int32_t TempLong;
	bool_t Flipped;
	int32_t* pp;

	/* Check */
	if (!a_Info)
		return false;
	
	/* Force Lag */
	//D_XNetForceLag();
	
	/* Flipped Levels? */
	Flipped = false;
	if (P_XGSVal(PGS_FUNFLIPLEVELS))
		Flipped = true;
	
	/* Respawn all players */
	if (!(a_Flags & PEXLL_NOPLREVIVE))
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
	if (!(a_Flags & PEXLL_NOCLEARLEVEL))
		P_ExClearLevel();
	
	/* Set global info */
	g_CurrentLevelInfo = a_Info;
	
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
				VertexP->x = ((fixed_t)WL_Srli16(Stream)) <<  FRACBITS;
				VertexP->y = ((fixed_t)WL_Srli16(Stream)) <<  FRACBITS;
				
				if (Flipped)
					VertexP->x = -VertexP->x;
				
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
				SectorP->floorheight = ((fixed_t)WL_Srli16(Stream)) << FRACBITS;
				SectorP->ceilingheight = ((fixed_t)WL_Srli16(Stream)) << FRACBITS;
				
				memset(Buf, 0, sizeof(Buf));
				for (j = 0; j < 8; j++)
					Buf[j] = WL_Sru8(Stream);
				SectorP->FloorTexture = Z_StrDup(Buf, PU_LEVEL, NULL);
				
				memset(Buf, 0, sizeof(Buf));
				for (j = 0; j < 8; j++)
					Buf[j] = WL_Sru8(Stream);
				SectorP->CeilingTexture = Z_StrDup(Buf, PU_LEVEL, NULL);
				
				SectorP->lightlevel = WL_Srli16(Stream);
				SectorP->special = WL_Srlu16(Stream);
				SectorP->tag = WL_Srlu16(Stream);
				
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
				SideDefP->textureoffset = ((fixed_t)WL_Srli16(Stream)) <<  FRACBITS;
				SideDefP->rowoffset = ((fixed_t)WL_Srli16(Stream)) <<  FRACBITS;
				
				for (k = 0; k < 3; k++)
				{
					memset(Buf, 0, sizeof(Buf));
					for (j = 0; j < 8; j++)
						Buf[j] = WL_Sru8(Stream);
					SideDefP->WallTextures[k] = Z_StrDup(Buf, PU_LEVEL, NULL);
				}
				
				SideDefP->SectorNum = WL_Srlu16(Stream);
				
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
					LineDefP->VertexNum[(Flipped ? !k : k)] = WL_Srlu16(Stream);
				LineDefP->flags = WL_Srlu16(Stream);
				
				if (a_Info->Type.Hexen)	// Hexen
				{
					LineDefP->HexenSpecial = WL_Sru8(Stream);
					for (k = 0; k < 5; k++)
						LineDefP->ACSArgs[k] = WL_Sru8(Stream);
				}
				else					// Doom
				{
					LineDefP->special = WL_Srlu16(Stream);
					LineDefP->tag = WL_Srlu16(Stream);
				}
				
				for (k = 0; k < 2; k++)
					LineDefP->sidenum[k] = WL_Srlu16(Stream);
				
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
				SubSectorP->numlines = WL_Srlu16(Stream);
				SubSectorP->firstline = WL_Srlu16(Stream);
				
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
				NodeP->x = ((fixed_t)WL_Srli16(Stream)) << FRACBITS;
				NodeP->y = ((fixed_t)WL_Srli16(Stream)) << FRACBITS;
				NodeP->dx = ((fixed_t)WL_Srli16(Stream)) << FRACBITS;
				NodeP->dy = ((fixed_t)WL_Srli16(Stream)) << FRACBITS;
				
				// Flip?
				if (Flipped)
				{
					NodeP->x += NodeP->dx;
					NodeP->y += NodeP->dy;
					NodeP->x = -NodeP->x;
					NodeP->dy = -NodeP->dy;
				}
				
				for (k = 0; k < 2; k++)
				{
					for (j = 0; j < 4; j++)
						NodeP->bbox[k][j] = ((fixed_t)WL_Srli16(Stream)) <<  FRACBITS;
					
					if (Flipped)
					{
						l = NodeP->bbox[k][2];
						NodeP->bbox[k][2] = -NodeP->bbox[k][3];
						NodeP->bbox[k][3] = -l;
					}
				}
				
				for (k = 0; k < 2; k++)
					NodeP->children[k] = WL_Srlu16(Stream);
				
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
					SegP->VertexID[(Flipped ? !k : k)] = WL_Srlu16(Stream);
				SegP->angle = ((angle_t)WL_Srlu16(Stream)) << 16;
				SegP->LineID = WL_Srlu16(Stream);
				SegP->side = WL_Srlu16(Stream);
				SegP->offset = ((fixed_t)WL_Srli16(Stream)) <<  FRACBITS;
				
				if (Flipped)
					*((int32_t*)&SegP->angle) = -(*((int32_t*)&SegP->angle));
				
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
			
			g_BMLSize = (k + 4);
			blockmaplump = Z_Malloc(sizeof(*blockmap) * (k + 4), PU_LEVEL, (void**)&blockmap);
			blockmap = blockmaplump + 4;	// Needed for compat
			
			// Read blockmap origin
			blockmaplump[0] = WL_Srli16(Stream);
			bmaporgx = ((fixed_t)blockmaplump[0]) <<  FRACBITS;
			blockmaplump[1] = WL_Srli16(Stream);
			bmaporgy = ((fixed_t)blockmaplump[1]) <<  FRACBITS;
			
			// Read blockmap size
			blockmaplump[2] = bmapwidth = ((int32_t)WL_Srlu16(Stream)) & 0xFFFFU;
			blockmaplump[3] = bmapheight = ((int32_t)WL_Srlu16(Stream)) & 0xFFFFU;
			
			// Load remaining blockmap data
			for (i = 0; i < k; i++)
			{
				// Loading screen
				if ((i & LOADMASK) == 0)
					CONL_LoadingScreenIncrSub();
				
				// Load in
				TempLong = WL_Srlu16(Stream);
				
				if (TempLong == UINT32_C(0xFFFF))
					blockmap[i] = -1;
				else
				{
					/*if (TempLong >= UINT32_C(0) && TempLong <= UINT32_C(0x7FFF))*/
						blockmap[i] = TempLong;
					/*else
						blockmap[i] = (~TempLong) & UINT32_C(0xFFFF);*/
				}
			}
			
			// Flip Blockmap?
			if (Flipped)
			{
				bmaporgx += bmapwidth * 128 * FRACUNIT;
				bmaporgx = -bmaporgx;
				
				for (i = 0; i < bmapheight; i++)
				{
					pp = blockmap + (i * bmapwidth);
					
					for (j = 0; j < bmapwidth / 2; j++)
					{
						l = pp[j];
						pp[j] = pp[bmapwidth - 1 - j];
						pp[bmapwidth - 1 - j] = l;
					}
				}
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
			g_RJMSize = k = ((numsectors * numsectors) / 8);
			rejectmatrix = Z_Malloc(sizeof(*rejectmatrix) * (k + 1), PU_LEVEL, (void**)&rejectmatrix);
			
			// Set loading screen info
			CONL_LoadingScreenSetSubEnd(k >> LOADSHIFT);
			
			// Read in matrix data
			for (j = 0; j < k; j++)
			{
				// Loading screen
				if ((j & LOADMASK) == 0)
					CONL_LoadingScreenIncrSub();
				
				rejectmatrix[j] = WL_Sru8(Stream);
			}
			
			// Close stream
			WL_StreamClose(Stream);
		}
	}
	
	// Munge Node Data
	
	CONL_LoadingScreenIncrMaj("Merging renderer data", 0);
	PS_ExMungeNodeData();
	
	// Load Things (Non-Textual Format)
	
	CONL_LoadingScreenIncrMaj("Loading things", 0);
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
					ThingP->ID = WL_Srlu16(Stream);
				
				ThingP->x = WL_Srli16(Stream);
				ThingP->y = WL_Srli16(Stream);
				
				if (Flipped)
					ThingP->x = -ThingP->x;
				
				if (ThingP->IsHexen)
					ThingP->HeightOffset = WL_Srli16(Stream);
				
				ThingP->angle = WL_Srli16(Stream);
				
				if (Flipped)
					ThingP->angle = 180 - ThingP->angle;
				
				ThingP->type = WL_Srli16(Stream);
				ThingP->options = WL_Srli16(Stream);
				
				if (ThingP->IsHexen)
				{
					ThingP->Special = WL_Sru8(Stream);
					for (k = 0; k < 5; k++)
						ThingP->Args[k] = WL_Sru8(Stream);
				}
				
				// Init
				PS_ExThingInit(ThingP, a_Flags);
			}
			
			// Close stream
			WL_StreamClose(Stream);
		}
	}
	
	/* Spawn map specials */
	if (!(a_Flags & PEXLL_NOSPAWNSPECIALS))
		P_SpawnSpecials();
	if (!(a_Flags & PEXLL_NOINITBRAIN))
		P_InitBrainTarget();
	
	/* Finalize */
	if (!(a_Flags & PEXLL_NOFINALIZE))
	{
		// Set the level time to zero
		//D_SyncNetSetMapTime(0);
	
		// Finalize
		P_ExFinalizeLevel();
	}
	
#undef BUFSIZE
#undef LOADMASK
#undef LOADSHIFT
	
	/* Success */
	return true;
}

/* P_ExFinalizeLevel() -- Finalizes the level so that it can be joined */
// Called by level loading and the save game loading code
bool_t P_ExFinalizeLevel(void)
{
	size_t i;
	WL_ES_t* ScriptStream;
	const WL_WADFile_t* WAD;
	
	/* Set gamestate to level */
	// So that we can play it now
	gamestate = GS_LEVEL;
	gameaction = ga_nothing;
	
	/* Respawn Missing Players */
	// This occurs when there are more players than starts on a new map load...
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i] && !players[i].mo)
			if (!players[i].CounterOpPlayer)
			{
				// Use cluster spawns
				if (P_GMIsDM())
					G_DeathMatchSpawnPlayer(i);
				else
					G_CoopSpawnPlayer(i);
			}
	
	/* Correct local player angles */
	for (i = 0; i < MAXSPLITSCREEN; i++)
		if (g_Splits[i].Active && playeringame[g_Splits[i].Console])
		{
			if (players[g_Splits[i].Console].mo)
				localangle[i] = players[g_Splits[i].Console].mo->angle;
			localaiming[i] = players[g_Splits[i].Console].aiming;
		}
	
	/* Setup sky */
	P_SetupLevelSky();
	
	// Flat number is the holder F_SKY1
	skyflatnum = R_GetFlatNumForName("F_SKY1");
	
	/* Load Music */
	if (g_CurrentLevelInfo)
		if (g_CurrentLevelInfo->Music)
			S_ChangeMusicName(g_CurrentLevelInfo->Music, 1);
	
	/* Build Bot Nodes */
	//B_InitNodes();
	
	/* Initialize Player */
	//for (i = 0; i < MAXPLAYERS; i++)
	//	G_InitPlayer(&players[i]);
	
	/* Compile Level Scripts */
	// Top Level Scripts, from each WAD
	for (WAD = WL_IterateVWAD(NULL, true); WAD; WAD = WL_IterateVWAD(WAD, true))
		TVM_CompileEntry(TVMNS_LEVEL, WL_FindEntry(WAD, 0, "TLSCRIPT"));
	
	// Header script
	if (g_CurrentLevelInfo->BlockPos[PIBT_SCRIPTS][1] -
		g_CurrentLevelInfo->BlockPos[PIBT_SCRIPTS][0] >= 0)
	{
		// Open stream to lump header
		ScriptStream = WL_StreamOpen(g_CurrentLevelInfo->EntryPtr[PLIEDS_HEADER]);
		
		// If it was created
		if (ScriptStream)
		{
			// Check Unicode viability
			WL_StreamCheckUnicode(ScriptStream);
			
			// Seek to script start
			WL_StreamSeek(ScriptStream, g_CurrentLevelInfo->BlockPos[PIBT_SCRIPTS][0], false);
			
			// Compile it
			TVM_CompileWLES(TVMNS_LEVEL, ScriptStream, g_CurrentLevelInfo->BlockPos[PIBT_SCRIPTS][1]);
			
			// Close it
			WL_StreamClose(ScriptStream);
		}
	}
	
	// Dedicated Level Script
	TVM_CompileEntry(TVMNS_LEVEL, g_CurrentLevelInfo->EntryPtr[PLIEDS_RSCRIPTS]);
	
	/* Initialize Fake Player */
	P_SpecInit(-1);
	
	/* Initialize current game mode */
	if (P_XGSVal(PGS_CONEWGAMEMODES))
		P_InitGameMode(P_XGSVal(PGS_GAMEMODE), P_XGSVal(PGS_GAMETEAMPLAY), -1, false);
	
	/* Success! */
	return true;
}

