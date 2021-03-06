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
//      Implements special effects:
//      Texture animation, height or lighting changes
//       according to adjacent sectors, respective
//       utility functions, etc.
//      Line Tag handling. Line and Sector triggers.

#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "p_setup.h"			//levelflats for flat animation
#include "r_data.h"
#include "m_random.h"

#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"
#include "dstrings.h"			//SoM: 3/10/2000
#include "r_main.h"				//Two extra includes.
#include "t_script.h"

#include "d_rmod.h"
//#include "r_sky.h" // Portals
#include "p_demcmp.h"

//SoM: Enable Boom features?
int boomsupport = 1;
int variable_friction = 1;
int allow_pushers = 1;

//SoM: 3/7/2000
static void P_SpawnScrollers(void);

static void P_SpawnFriction(void);
static void P_SpawnPushers(void);
void Add_Pusher(int type, int x_mag, int y_mag, mobj_t* source, int affectee);	//SoM: 3/9/2000
void P_FindAnimatedFlat(int i);

//
// Animating textures and planes
// There is another anim_t used in wi_stuff, unrelated.
//
typedef struct
{
	bool_t istexture;
	int picnum;
	int basepic;
	int numpics;
	int speed;
} anim_t;

#define MAXANIMS     32

//SoM: 3/7/2000: New sturcture without limits.
static anim_t* lastanim = NULL;
static anim_t* anims = NULL;
static size_t maxanims = 0;

/* P_InitPicAnims() -- Animated Textures */
void P_InitPicAnims(void)
{
	int32_t i, j, t;
	const WL_WADEntry_t* Entry;
	WL_ES_t* Stream;
	uint8_t Flag;
	char Texts[2][9];
	int32_t Time;
	
	/* Locate ANIMATED */
	Entry = WL_FindEntry(NULL, 0, "ANIMATED");
	
	// Not found?
	if (!Entry)
		return;
	
	// Open stream
	Stream = WL_StreamOpen(Entry);
	
	// Failed?
	if (!Stream)
		return;
	
	/* Free? */
	if (anims)
		Z_Free(anims);
	anims = NULL;
	maxanims = NULL;
	
	/* Load animation data */
	maxanims = Entry->Size / 23;
	anims = Z_Malloc(sizeof(*anims) * (maxanims + 1), PU_STATIC, NULL);
	
	// Parse data
	for (i = 0; i < maxanims; i++)
	{
		// Read Marker
		Flag = WL_Sru8(Stream);
		
		// End?
		if (Flag == 255)
			break;
		
		// Read last and first textures
		memset(Texts, 0, sizeof(Texts));
		
		for (t = 0; t < 2; t++)
		{
			for (j = 0; j < 9; j++)
				Texts[t][j] = WL_Sru8(Stream);
			Texts[t][8] = 0;
			
			// Upper case
			C_strupr(Texts[t]);
		}
		
		
		// Read time
		Time = WL_Srli32(Stream);
		
		// Fill in real info
		anims[i].istexture = (Flag == 1 ? true : false);
		anims[i].speed = Time;
		
		// Texture?
		if (anims[i].istexture)
		{
			anims[i].picnum = R_TextureNumForName(Texts[0]);
			anims[i].basepic = R_TextureNumForName(Texts[1]);
		}
		
		// Flat
		else
		{
			anims[i].picnum = R_GetFlatNumForName(Texts[0]);
			anims[i].basepic = R_GetFlatNumForName(Texts[1]);
		}
		
		// Get number of pictures
		anims[i].numpics = (anims[i].picnum - anims[i].basepic) + 1;
		if (anims[i].numpics < 0)
			anims[i].numpics = 0;
	}
	
	// Last animation is nothing
	lastanim = &anims[i];
	lastanim->istexture = ((bool_t)-1);
	
	/* Close Stream */
	WL_StreamClose(Stream);
}

//  Check for flats in levelflats, that are part
//  of a flat anim sequence, if so, set them up for animation
//
//SoM: 3/16/2000: Changed parameter from pointer to "anims" entry number
void P_FindAnimatedFlat(int animnum)
{
}

//
//  Called by P_LoadSectors
//
void P_SetupLevelFlatAnims(void)
{
}

//
// UTILITIES
//

//
// getSide()
// Will return a side_t*
//  given the number of the current sector,
//  the line number, and the side (0/1) that you want.
//
side_t* getSide(int currentSector, int line, int side)
{
	return &sides[(sectors[currentSector].lines[line])->sidenum[side]];
}

//
// getSector()
// Will return a sector_t*
//  given the number of the current sector,
//  the line number and the side (0/1) that you want.
//
sector_t* getSector(int currentSector, int line, int side)
{
	return sides[(sectors[currentSector].lines[line])->sidenum[side]].sector;
}

//
// twoSided()
// Given the sector number and the line number,
//  it will tell you whether the line is two-sided or not.
//
//SoM: 3/7/2000: Use the boom method
int twoSided(int sector, int line)
{
	return P_XGSVal(PGS_COBOOMSUPPORT) ? ((sectors[sector].lines[line])->sidenum[1] != -1) : ((sectors[sector].lines[line])->flags & ML_TWOSIDED);
}

//
// getNextSector()
// Return sector_t * of sector next to current.
// NULL if not two-sided line
//
//SoM: 3/7/2000: Use boom method.
sector_t* getNextSector(line_t* line, sector_t* sec)
{
	if (!P_XGSVal(PGS_COBOOMSUPPORT))
	{
		if (!(line->flags & ML_TWOSIDED))
			return NULL;
	}
	
	if (line->frontsector == sec)
	{
		if (!P_XGSVal(PGS_COBOOMSUPPORT) || line->backsector != sec)
			return line->backsector;
		else
			return NULL;
	}
	return line->frontsector;
}

//
// P_FindLowestFloorSurrounding()
// FIND LOWEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
fixed_t P_FindLowestFloorSurrounding(sector_t* sec)
{
	int i;
	line_t* check;
	sector_t* other;
	fixed_t floor = sec->floorheight;
	
	for (i = 0; i < sec->linecount; i++)
	{
		check = sec->lines[i];
		other = getNextSector(check, sec);
		
		if (!other)
			continue;
			
		if (other->floorheight < floor)
			floor = other->floorheight;
	}
	return floor;
}

//
// P_FindHighestFloorSurrounding()
// FIND HIGHEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
fixed_t P_FindHighestFloorSurrounding(sector_t* sec)
{
	int i;
	line_t* check;
	sector_t* other;
	fixed_t floor = -500 * FRACUNIT;
	int foundsector = 0;
	
	for (i = 0; i < sec->linecount; i++)
	{
		check = sec->lines[i];
		other = getNextSector(check, sec);
		
		if (!other)
			continue;
			
		if (other->floorheight > floor || !foundsector)
			floor = other->floorheight;
			
		if (!foundsector)
			foundsector = 1;
	}
	return floor;
}

//
// P_FindNextHighestFloor
// FIND NEXT HIGHEST FLOOR IN SURROUNDING SECTORS
// SoM: 3/7/2000: Use Lee Killough's version insted.
// Rewritten by Lee Killough to avoid fixed array and to be faster
//
fixed_t P_FindNextHighestFloor(sector_t* sec, int currentheight)
{
	sector_t* other;
	int i;
	
	for (i = 0; i < sec->linecount; i++)
		if ((other = getNextSector(sec->lines[i], sec)) && other->floorheight > currentheight)
		{
			int height = other->floorheight;
			
			while (++i < sec->linecount)
				if ((other = getNextSector(sec->lines[i], sec)) && other->floorheight < height && other->floorheight > currentheight)
					height = other->floorheight;
			return height;
		}
	return currentheight;
}

////////////////////////////////////////////////////
// SoM: Start new Boom functions
////////////////////////////////////////////////////

// P_FindNextLowestFloor()
//
// Passed a sector and a floor height, returns the fixed point value
// of the largest floor height in a surrounding sector smaller than
// the floor height passed. If no such height exists the floorheight
// passed is returned.
//
fixed_t P_FindNextLowestFloor(sector_t* sec, int currentheight)
{
	sector_t* other;
	int i;
	
	for (i = 0; i < sec->linecount; i++)
		if ((other = getNextSector(sec->lines[i], sec)) && other->floorheight < currentheight)
		{
			int height = other->floorheight;
			
			while (++i < sec->linecount)
				if ((other = getNextSector(sec->lines[i], sec)) && other->floorheight > height && other->floorheight < currentheight)
					height = other->floorheight;
			return height;
		}
	return currentheight;
}

//
// P_FindNextLowestCeiling()
//
// Passed a sector and a ceiling height, returns the fixed point value
// of the largest ceiling height in a surrounding sector smaller than
// the ceiling height passed. If no such height exists the ceiling height
// passed is returned.
//
fixed_t P_FindNextLowestCeiling(sector_t* sec, int currentheight)
{
	sector_t* other;
	int i;
	
	for (i = 0; i < sec->linecount; i++)
		if ((other = getNextSector(sec->lines[i], sec)) && other->ceilingheight < currentheight)
		{
			int height = other->ceilingheight;
			
			while (++i < sec->linecount)
				if ((other = getNextSector(sec->lines[i], sec)) && other->ceilingheight > height && other->ceilingheight < currentheight)
					height = other->ceilingheight;
			return height;
		}
	return currentheight;
}

//
// P_FindNextHighestCeiling()
//
// Passed a sector and a ceiling height, returns the fixed point value
// of the smallest ceiling height in a surrounding sector larger than
// the ceiling height passed. If no such height exists the ceiling height
// passed is returned.
//
fixed_t P_FindNextHighestCeiling(sector_t* sec, int currentheight)
{
	sector_t* other;
	int i;
	
	for (i = 0; i < sec->linecount; i++)
		if ((other = getNextSector(sec->lines[i], sec)) && other->ceilingheight > currentheight)
		{
			int height = other->ceilingheight;
			
			while (++i < sec->linecount)
				if ((other = getNextSector(sec->lines[i], sec)) && other->ceilingheight < height && other->ceilingheight > currentheight)
					height = other->ceilingheight;
			return height;
		}
	return currentheight;
}

////////////////////////////
// End New Boom functions
////////////////////////////

//
// FIND LOWEST CEILING IN THE SURROUNDING SECTORS
//
fixed_t P_FindLowestCeilingSurrounding(sector_t* sec)
{
	int i;
	line_t* check;
	sector_t* other;
	fixed_t height = INT_MAX;
	int foundsector = 0;
	
	if (P_XGSVal(PGS_COBOOMSUPPORT))
		height = 32000 * FRACUNIT;	//SoM: 3/7/2000: Remove ovf
		
	for (i = 0; i < sec->linecount; i++)
	{
		check = sec->lines[i];
		other = getNextSector(check, sec);
		
		if (!other)
			continue;
			
		if (other->ceilingheight < height || !foundsector)
			height = other->ceilingheight;
			
		if (!foundsector)
			foundsector = 1;
	}
	return height;
}

//
// FIND HIGHEST CEILING IN THE SURROUNDING SECTORS
//
fixed_t P_FindHighestCeilingSurrounding(sector_t* sec)
{
	int i;
	line_t* check;
	sector_t* other;
	fixed_t height = 0;
	int foundsector = 0;
	
	for (i = 0; i < sec->linecount; i++)
	{
		check = sec->lines[i];
		other = getNextSector(check, sec);
		
		if (!other)
			continue;
			
		if (other->ceilingheight > height || !foundsector)
			height = other->ceilingheight;
			
		if (!foundsector)
			foundsector = 1;
	}
	return height;
}

//SoM: 3/7/2000: UTILS.....
//
// P_FindShortestTextureAround()
//
// Passed a sector number, returns the shortest lower texture on a
// linedef bounding the sector.
//
fixed_t P_FindShortestTextureAround(int secnum)
{
	int minsize = INT_MAX;
	side_t* side;
	int i;
	sector_t* sec = &sectors[secnum];
	
	if (P_XGSVal(PGS_COBOOMSUPPORT))
		minsize = 32000 << FRACBITS;
		
	for (i = 0; i < sec->linecount; i++)
	{
		if (twoSided(secnum, i))
		{
			side = getSide(secnum, i, 0);
			if (side->bottomtexture > 0)
				if (textures[side->bottomtexture]->XHeight < minsize)
					minsize = textures[side->bottomtexture]->XHeight;
			side = getSide(secnum, i, 1);
			if (side->bottomtexture > 0)
				if (textures[side->bottomtexture]->XHeight < minsize)
					minsize = textures[side->bottomtexture]->XHeight;
		}
	}
	return minsize;
}

//SoM: 3/7/2000: Stuff.... (can you tell I'm getting tired? It's 12:30!)
//
// P_FindShortestUpperAround()
//
// Passed a sector number, returns the shortest upper texture on a
// linedef bounding the sector.
//
fixed_t P_FindShortestUpperAround(int secnum)
{
	int minsize = INT_MAX;
	side_t* side;
	int i;
	sector_t* sec = &sectors[secnum];
	
	if (P_XGSVal(PGS_COBOOMSUPPORT))
		minsize = 32000 << FRACBITS;
		
	for (i = 0; i < sec->linecount; i++)
	{
		if (twoSided(secnum, i))
		{
			side = getSide(secnum, i, 0);
			if (side->toptexture > 0)
				if (textures[side->toptexture]->XHeight < minsize)
					minsize = textures[side->toptexture]->XHeight;
			side = getSide(secnum, i, 1);
			if (side->toptexture > 0)
				if (textures[side->toptexture]->XHeight < minsize)
					minsize = textures[side->toptexture]->XHeight;
		}
	}
	return minsize;
}

//SoM: 3/7/2000
//
// P_FindModelFloorSector()
//
// Passed a floor height and a sector number, return a pointer to a
// a sector with that floor height across the lowest numbered two sided
// line surrounding the sector.
//
// Note: If no sector at that height bounds the sector passed, return NULL
//
sector_t* P_FindModelFloorSector(fixed_t floordestheight, int secnum)
{
	int i;
	sector_t* sec = NULL;
	int linecount;
	
	sec = &sectors[secnum];
	linecount = sec->linecount;
	for (i = 0; i < (!P_XGSVal(PGS_COBOOMSUPPORT) && sec->linecount < linecount ? sec->linecount : linecount); i++)
	{
		if (twoSided(secnum, i))
		{
			if (getSide(secnum, i, 0)->sector - sectors == secnum)
				sec = getSector(secnum, i, 1);
			else
				sec = getSector(secnum, i, 0);
				
			if (sec->floorheight == floordestheight)
				return sec;
		}
	}
	return NULL;
}

//SoM: 3/7/2000: Last one...
//
// P_FindModelCeilingSector()
//
// Passed a ceiling height and a sector number, return a pointer to a
// a sector with that ceiling height across the lowest numbered two sided
// line surrounding the sector.
//
// Note: If no sector at that height bounds the sector passed, return NULL
//
sector_t* P_FindModelCeilingSector(fixed_t ceildestheight, int secnum)
{
	int i;
	sector_t* sec = NULL;
	int linecount;
	
	sec = &sectors[secnum];
	linecount = sec->linecount;
	for (i = 0; i < (!P_XGSVal(PGS_COBOOMSUPPORT) && sec->linecount < linecount ? sec->linecount : linecount); i++)
	{
		if (twoSided(secnum, i))
		{
			if (getSide(secnum, i, 0)->sector - sectors == secnum)
				sec = getSector(secnum, i, 1);
			else
				sec = getSector(secnum, i, 0);
				
			if (sec->ceilingheight == ceildestheight)
				return sec;
		}
	}
	return NULL;
}

//
// RETURN NEXT SECTOR # THAT LINE TAG REFERS TO
//
//SoM: 3/7/2000: Killough wrote this to improve the process.
int P_FindSectorFromLineTag(line_t* line, int start)
{
	/* Backside */
	if (!line)
		// Return the line we sent it
		return start;
	
	/* Find Sector */
	start = start >= 0 ? sectors[start].nexttag : sectors[(unsigned)line->tag % (unsigned)numsectors].firsttag;
	while (start >= 0 && sectors[start].tag != line->tag)
		start = sectors[start].nexttag;
	return start;
}

//
// P_FindSectorFromTag
// Used by FraggleScript
int P_FindSectorFromTag(int tag, int start)
{
	start = start >= 0 ? sectors[start].nexttag : sectors[(unsigned)tag % (unsigned)numsectors].firsttag;
	while (start >= 0 && sectors[start].tag != tag)
		start = sectors[start].nexttag;
	return start;
}

//DarkWolf95:July 23, 2003: Needed for SF_SetLineTexture
int P_FindLineFromTag(int tag, int start)
{
	start = start >= 0 ? lines[start].nexttag : lines[(unsigned)tag % (unsigned)numlines].firsttag;
	while (start >= 0 && lines[start].tag != tag)
		start = lines[start].nexttag;
	return start;
}

//SoM: 3/7/2000: More boom specific stuff...
// killough 4/16/98: Same thing, only for linedefs

int P_FindLineFromLineTag(const line_t* line, int start)
{
	start = start >= 0 ? lines[start].nexttag : lines[(unsigned)line->tag % (unsigned)numlines].firsttag;
	while (start >= 0 && lines[start].tag != line->tag)
		start = lines[start].nexttag;
	return start;
}

//SoM: 3/7/2000: Oh joy!
// Hash the sector tags across the sectors and linedefs.
static void P_InitTagLists(void)
{
	register int i;
	
	for (i = numsectors; --i >= 0;)
		sectors[i].firsttag = -1;
	for (i = numsectors; --i >= 0;)
	{
		int j = (unsigned)sectors[i].tag % (unsigned)numsectors;
		
		sectors[i].nexttag = sectors[j].firsttag;
		sectors[j].firsttag = i;
	}
	
	for (i = numlines; --i >= 0;)
		lines[i].firsttag = -1;
	for (i = numlines; --i >= 0;)
	{
		int j = (unsigned)lines[i].tag % (unsigned)numlines;
		
		lines[i].nexttag = lines[j].firsttag;
		lines[j].firsttag = i;
	}
}

//
// Find minimum light from an adjacent sector
//
int P_FindMinSurroundingLight(sector_t* sector, int max)
{
	int i;
	int min;
	line_t* line;
	sector_t* check;
	
	min = max;
	for (i = 0; i < sector->linecount; i++)
	{
		line = sector->lines[i];
		check = getNextSector(line, sector);
		
		if (!check)
			continue;
			
		if (check->lightlevel < min)
			min = check->lightlevel;
	}
	return min;
}

//SoM: 3/7/2000
//
// P_CanUnlockGenDoor()
//
// Passed a generalized locked door linedef and a player, returns whether
// the player has the keys necessary to unlock that door.
//
// Note: The linedef passed MUST be a generalized locked door type
//       or results are undefined.
//
bool_t P_CanUnlockGenDoor(line_t* line, player_t* player)
{
	// does this line special distinguish between skulls and keys?
	int skulliscard = (line->special & LockedNKeys) >> LockedNKeysShift;
	
	// GhostlyDeath <May 4, 2012> -- All Doors unlocked
	if (P_XGSVal(PGS_FUNNOLOCKEDDOORS))
		return true;
	
	// determine for each case of lock type if player's keys are adequate
	switch ((line->special & LockedKey) >> LockedKeyShift)
	{
		case AnyKey_:
			if (!(player->cards & it_redcard) &&
			        !(player->cards & it_redskull) &&
			        !(player->cards & it_bluecard) && !(player->cards & it_blueskull) && !(player->cards & it_yellowcard) && !(player->cards & it_yellowskull))
			{
				//player->message = PD_ANY;
				S_StartSound(&player->mo->NoiseThinker, sfx_oof);
				return false;
			}
			break;
		case RCard:
			if (!(player->cards & it_redcard) && (!skulliscard || !(player->cards & it_redskull)))
			{
				//player->message = skulliscard ? PD_REDK : PD_REDC;
				S_StartSound(&player->mo->NoiseThinker, sfx_oof);
				return false;
			}
			break;
		case BCard:
			if (!(player->cards & it_bluecard) && (!skulliscard || !(player->cards & it_blueskull)))
			{
				//player->message = skulliscard ? PD_BLUEK : PD_BLUEC;
				S_StartSound(&player->mo->NoiseThinker, sfx_oof);
				return false;
			}
			break;
		case YCard:
			if (!(player->cards & it_yellowcard) && (!skulliscard || !(player->cards & it_yellowskull)))
			{
				//player->message = skulliscard ? PD_YELLOWK : PD_YELLOWC;
				S_StartSound(&player->mo->NoiseThinker, sfx_oof);
				return false;
			}
			break;
		case RSkull:
			if (!(player->cards & it_redskull) && (!skulliscard || !(player->cards & it_redcard)))
			{
				//player->message = skulliscard ? PD_REDK : PD_REDS;
				S_StartSound(&player->mo->NoiseThinker, sfx_oof);
				return false;
			}
			break;
		case BSkull:
			if (!(player->cards & it_blueskull) && (!skulliscard || !(player->cards & it_bluecard)))
			{
				//player->message = skulliscard ? PD_BLUEK : PD_BLUES;
				S_StartSound(&player->mo->NoiseThinker, sfx_oof);
				return false;
			}
			break;
		case YSkull:
			if (!(player->cards & it_yellowskull) && (!skulliscard || !(player->cards & it_yellowcard)))
			{
				//player->message = skulliscard ? PD_YELLOWK : PD_YELLOWS;
				S_StartSound(&player->mo->NoiseThinker, sfx_oof);
				return false;
			}
			break;
		case AllKeys:
			if (!skulliscard &&
			        (!(player->cards & it_redcard) ||
			         !(player->cards & it_redskull) ||
			         !(player->cards & it_bluecard) || !(player->cards & it_blueskull) || !(player->cards & it_yellowcard) || !(player->cards & it_yellowskull)))
			{
				//player->message = PD_ALL6;
				S_StartSound(&player->mo->NoiseThinker, sfx_oof);
				return false;
			}
			if (skulliscard &&
			        ((!(player->cards & it_redcard) &&
			          !(player->cards & it_redskull)) ||
			         (!(player->cards & it_bluecard) &&
			          !(player->cards & it_blueskull)) || (!(player->cards & it_yellowcard) && !(player->cards & it_yellowskull))))
			{
				//player->message = PD_ALL3;
				S_StartSound(&player->mo->NoiseThinker, sfx_oof);
				return false;
			}
			break;
	}
	return true;
}

//
// P_SectorActive()
//
// Passed a linedef special class (floor, ceiling, lighting) and a sector
// returns whether the sector is already busy with a linedef special of the
// same class. If old demo compatibility true, all linedef special classes
// are the same.
//
size_t P_SectorActive(special_e t, sector_t* sec)
{
	if (!P_XGSVal(PGS_COBOOMSUPPORT))
		return sec->floordata || sec->ceilingdata || sec->lightingdata;
	else
		switch (t)
		{
			case floor_special:
				return (size_t) sec->floordata;
			case ceiling_special:
				return (size_t) sec->ceilingdata;
			case lighting_special:
				return (size_t) sec->lightingdata;
		}
	return 1;
}

//SoM: 3/7/2000
//
// P_CheckTag()
//
// Passed a line, returns true if the tag is non-zero or the line special
// allows no tag without harm. If compatibility, all linedef specials are
// allowed to have zero tag.
//
// Note: Only line specials activated by walkover, pushing, or shooting are
//       checked by this routine.
//
int P_CheckTag(line_t* line)
{
	if (!P_XGSVal(PGS_COBOOMSUPPORT))
		return 1;
		
	if (line->tag)
		return 1;
		
	switch (line->special)
	{
		case 1:				// Manual door specials
		case 26:
		case 27:
		case 28:
		case 31:
		case 32:
		case 33:
		case 34:
		case 117:
		case 118:
		
		case 139:				// Lighting specials
		case 170:
		case 79:
		case 35:
		case 138:
		case 171:
		case 81:
		case 13:
		case 192:
		case 169:
		case 80:
		case 12:
		case 194:
		case 173:
		case 157:
		case 104:
		case 193:
		case 172:
		case 156:
		case 17:
		
		case 195:				// Thing teleporters
		case 174:
		case 97:
		case 39:
		case 126:
		case 125:
		case 210:
		case 209:
		case 208:
		case 207:
		
		case 11:				// Exits
		case 52:
		case 197:
		case 51:
		case 124:
		case 198:
		
		case 48:				// Scrolling walls
		case 85:
			// FraggleScript types!
		case 272:				// WR
		case 273:
		case 274:				// W1
		case 275:
		case 276:				// SR
		case 277:				// S1
		case 278:				// GR
		case 279:				// G1
			return 1;			// zero tag allowed
			
		default:
			break;
	}
	return 0;					// zero tag not allowed
}

//SoM: 3/7/2000: Is/WasSecret.
//
// P_IsSecret()
//
// Passed a sector, returns if the sector secret type is still active, i.e.
// secret type is set and the secret has not yet been obtained.
//
bool_t P_IsSecret(sector_t* sec)
{
	return (sec->special == 9 || (sec->special & SECRET_MASK));
}

//
// EVENTS
// Events are operations triggered by using, crossing,
// or shooting special lines, or by timed thinkers.
//

//
// P_CrossSpecialLine - TRIGGER
// Called every time a thing origin is about
//  to cross a line with a non 0 special.
//
void P_CrossSpecialLine(int linenum, int side, mobj_t* thing)
{
	line_t* line;
	
	line = &lines[linenum];
	
	P_ActivateCrossedLine(line, side, thing);
}

void P_ActivateCrossedLine(line_t* line, int side, mobj_t* thing)
{
	int ok;
	bool_t UseAgain;
	bool_t forceuse;				//SoM: 4/26/2000: ALLTRIGGER should allow monsters to use generalized types too!
	
	forceuse = false;
	
	// Force all usage?
	if (thing->RXFlags[0] & MFREXA_NOFORCEALLTRIGGERC)
		if (line->flags & ML_ALLTRIGGER)
			forceuse = true;
	
	//  Triggers that other things can activate
	if (!thing->player)
		// Things that should NOT trigger specials...
		if (thing->RXFlags[0] & MFREXA_NOCROSSTRIGGER)
			return;
	
	/* Better Generalized Support */
	// GhostlyDeath <May 2, 2012> -- This is MUCH better than before!
	UseAgain = false;
	if (EV_TryGenTrigger(line, side, thing, EVTGT_WALK, (forceuse ? EVTGTF_FORCEUSE : 0), &UseAgain))
		if (!UseAgain)
			line->special = 0;
	return;	
#if 0
	
	//SoM: 3/7/2000: Check for generalized line types/
	if (boomsupport)
	{
		// pointer to line function is NULL by default, set non-null if
		// line special is walkover generalized linedef type
		int (*linefunc) (line_t * line) = NULL;
		
		// check each range of generalized linedefs
		if ((unsigned)line->special >= GenFloorBase)
		{
			if (!thing->player)
				if (((line->special & FloorChange) || !(line->special & FloorModel)) && !forceuse)
					return;		// FloorModel is "Allow Monsters" if FloorChange is 0
			if (!line->tag)
				return;
			linefunc = EV_DoGenFloor;
		}
		else if ((unsigned)line->special >= GenCeilingBase)
		{
			if (!thing->player)
				if (((line->special & CeilingChange) || !(line->special & CeilingModel)) && !forceuse)
					return;		// CeilingModel is "Allow Monsters" if CeilingChange is 0
			if (!line->tag)
				return;
			linefunc = EV_DoGenCeiling;
		}
		else if ((unsigned)line->special >= GenDoorBase)
		{
			if (!thing->player)
			{
				if (!(line->special & DoorMonster) && !forceuse)
					return;		// monsters disallowed from this door
				if (line->flags & ML_SECRET)	// they can't open secret doors either
					return;
			}
			if (!line->tag)
				return;
			linefunc = EV_DoGenDoor;
		}
		else if ((unsigned)line->special >= GenLockedBase)
		{
			if (!thing->player)
				return;			// monsters disallowed from unlocking doors
			if (((line->special & TriggerType) == WalkOnce) || ((line->special & TriggerType) == WalkMany))
			{
				if (!P_CanUnlockGenDoor(line, thing->player))
					return;
			}
			else
				return;
			linefunc = EV_DoGenLockedDoor;
		}
		else if ((unsigned)line->special >= GenLiftBase)
		{
			if (!thing->player)
				if (!(line->special & LiftMonster) && !forceuse)
					return;		// monsters disallowed
			if (!line->tag)
				return;
			linefunc = EV_DoGenLift;
		}
		else if ((unsigned)line->special >= GenStairsBase)
		{
			if (!thing->player)
				if (!(line->special & StairMonster) && !forceuse)
					return;		// monsters disallowed
			if (!line->tag)
				return;
			linefunc = EV_DoGenStairs;
		}
		else if ((unsigned)line->special >= GenCrusherBase)
		{
			if (!thing->player)
				if (!(line->special & StairMonster) && !forceuse)
					return;		// monsters disallowed
			if (!line->tag)
				return;
			linefunc = EV_DoGenCrusher;
		}
		
		if (linefunc)			// if it was a valid generalized type
			switch ((line->special & TriggerType) >> TriggerTypeShift)
			{
				case WalkOnce:
					if (linefunc(line))
						line->special = 0;	// clear special if a walk once type
					return;
				case WalkMany:
					linefunc(line);
					return;
				default:		// if not a walk type, do nothing here
					return;
			}
	}
	
	if (!thing->player)
	{
		ok = 0;
		
		switch (line->special)
		{
			case 39:			// TELEPORT TRIGGER
			case 97:			// TELEPORT RETRIGGER
			case 125:			// TELEPORT MONSTERONLY TRIGGER
			case 126:			// TELEPORT MONSTERONLY RETRIGGER
			case 4:			// RAISE DOOR
			case 10:			// PLAT DOWN-WAIT-UP-STAY TRIGGER
			case 88:			// PLAT DOWN-WAIT-UP-STAY RETRIGGER
				ok = 1;
				break;
				// SoM: 3/4/2000: Add boom compatibility for extra monster usable
				// linedef types.
			case 208:			//SoM: Silent thing teleporters
			case 207:
			case 243:			//Silent line to line teleporter
			case 244:			//Same as above but trigger once.
			case 262:			//Same as 243 but reversed
			case 263:			//Same as 244 but reversed
			case 264:			//Monster only, silent, trigger once, reversed
			case 265:			//Same as 264 but repeatable
			case 266:			//Monster only, silent, trigger once
			case 267:			//Same as 266 bot repeatable
			case 268:			//Monster only, silent, trigger once, set pos to thing
			case 269:			//Monster only, silent, repeatable, set pos to thing
				if (boomsupport)
					ok = 1;
				break;
		}
		//SoM: Anything can trigger this line!
		if (line->flags & ML_ALLTRIGGER)
			ok = 1;
			
		if (!ok)
			return;
	}
	
	if (!P_CheckTag(line) && boomsupport)
		return;
		
	// Note: could use some const's here.
	switch (line->special)
	{
			// TRIGGERS.
			// All from here to RETRIGGERS.
		case 2:
			// Open Door
			if (EV_DoDoor(line, dooropen, VDOORSPEED) || !boomsupport)
				line->special = 0;
			break;
			
		case 3:
			// Close Door
			if (EV_DoDoor(line, doorclose, VDOORSPEED) || !boomsupport)
				line->special = 0;
			break;
			
		case 4:
			// Raise Door
			if (EV_DoDoor(line, normalDoor, VDOORSPEED) || !boomsupport)
				line->special = 0;
			break;
			
		case 5:
			// Raise Floor
			if (EV_DoFloor(line, raiseFloor) || !boomsupport)
				line->special = 0;
			break;
			
		case 6:
			// Fast Ceiling Crush & Raise
			if (EV_DoCeiling(line, fastCrushAndRaise) || !boomsupport)
				line->special = 0;
			break;
			
		case 8:
			// Build Stairs
			if (EV_BuildStairs(line, build8) || !boomsupport)
				line->special = 0;
			break;
			
		case 10:
			// PlatDownWaitUp
			if (EV_DoPlat(line, downWaitUpStay, 0) || !boomsupport)
				line->special = 0;
			break;
			
		case 12:
			// Light Turn On - brightest near
			if (EV_LightTurnOn(line, 0) || !boomsupport)
				line->special = 0;
			break;
			
		case 13:
			// Light Turn On 255
			if (EV_LightTurnOn(line, 255) || !boomsupport)
				line->special = 0;
			break;
			
		case 16:
			// Close Door 30
			if (EV_DoDoor(line, close30ThenOpen, VDOORSPEED) || !boomsupport)
				line->special = 0;
			break;
			
		case 17:
			// Start Light Strobing
			if (EV_StartLightStrobing(line) || !boomsupport)
				line->special = 0;
			break;
			
		case 19:
			// Lower Floor
			if (EV_DoFloor(line, lowerFloor) || !boomsupport)
				line->special = 0;
			break;
			
		case 22:
			// Raise floor to nearest height and change texture
			if (EV_DoPlat(line, raiseToNearestAndChange, 0) || !boomsupport)
				line->special = 0;
			break;
			
		case 25:
			// Ceiling Crush and Raise
			if (EV_DoCeiling(line, crushAndRaise) || !boomsupport)
				line->special = 0;
			break;
			
		case 30:
			// Raise floor to shortest texture height
			//  on either side of lines.
			if (EV_DoFloor(line, raiseToTexture) || !boomsupport)
				line->special = 0;
			break;
			
		case 35:
			// Lights Very Dark
			if (EV_LightTurnOn(line, 35) || !boomsupport)
				line->special = 0;
			break;
			
		case 36:
			// Lower Floor (TURBO)
			if (EV_DoFloor(line, turboLower) || !boomsupport)
				line->special = 0;
			break;
			
		case 37:
			// LowerAndChange
			if (EV_DoFloor(line, lowerAndChange) || !boomsupport)
				line->special = 0;
			break;
			
		case 38:
			// Lower Floor To Lowest
			if (EV_DoFloor(line, lowerFloorToLowest) || !boomsupport)
				line->special = 0;
			break;
			
		case 39:
			// TELEPORT!
			if (EV_Teleport(line, side, thing) || !boomsupport)
				line->special = 0;
			break;
			
		case 40:
			// RaiseCeilingLowerFloor
			if (EV_DoCeiling(line, raiseToHighest) || EV_DoFloor(line, lowerFloorToLowest) || !boomsupport)
				line->special = 0;
			break;
			
		case 44:
			// Ceiling Crush
			if (EV_DoCeiling(line, lowerAndCrush) || !boomsupport)
				line->special = 0;
			break;
			
		case 52:
			// EXIT!
			if (cv_allowexitlevel.value)
			{
				G_ExitLevel();
				line->special = 0;	// heretic have right
			}
			break;
			
		case 53:
			// Perpetual Platform Raise
			if (EV_DoPlat(line, perpetualRaise, 0) || !boomsupport)
				line->special = 0;
			break;
			
		case 54:
			// Platform Stop
			if (EV_StopPlat(line) || !boomsupport)
				line->special = 0;
			break;
			
		case 56:
			// Raise Floor Crush
			if (EV_DoFloor(line, raiseFloorCrush) || !boomsupport)
				line->special = 0;
			break;
			
		case 57:
			// Ceiling Crush Stop
			if (EV_CeilingCrushStop(line) || !boomsupport)
				line->special = 0;
			break;
			
		case 58:
			// Raise Floor 24
			if (EV_DoFloor(line, raiseFloor24) || !boomsupport)
				line->special = 0;
			break;
			
		case 59:
			// Raise Floor 24 And Change
			if (EV_DoFloor(line, raiseFloor24AndChange) || !boomsupport)
				line->special = 0;
			break;
			
		case 104:
			// Turn lights off in sector(tag)
			if (EV_TurnTagLightsOff(line) || !boomsupport)
				line->special = 0;
			break;
			
		case 108:
			// Blazing Door Raise (faster than TURBO!)
			if (EV_DoDoor(line, blazeRaise, 4 * VDOORSPEED) || !boomsupport)
				line->special = 0;
			break;
			
		case 109:
			// Blazing Door Open (faster than TURBO!)
			if (EV_DoDoor(line, blazeOpen, 4 * VDOORSPEED) || !boomsupport)
				line->special = 0;
			break;
			
		case 100:
			// Build Stairs Turbo 16
			if (EV_BuildStairs(line, turbo16) || !boomsupport)
				line->special = 0;
			break;
			
		case 110:
			// Blazing Door Close (faster than TURBO!)
			if (EV_DoDoor(line, blazeClose, 4 * VDOORSPEED) || !boomsupport)
				line->special = 0;
			break;
			
		case 119:
			// Raise floor to nearest surr. floor
			if (EV_DoFloor(line, raiseFloorToNearest) || !boomsupport)
				line->special = 0;
			break;
			
		case 121:
			// Blazing PlatDownWaitUpStay
			if (EV_DoPlat(line, blazeDWUS, 0) || !boomsupport)
				line->special = 0;
			break;
			
		case 124:
			// Secret EXIT
			if (cv_allowexitlevel.value)
				G_SecretExitLevel();
			break;
			
		case 125:
			// TELEPORT MonsterONLY
			if (!thing->player)
			{
				if (EV_Teleport(line, side, thing) || !boomsupport)
					line->special = 0;
			}
			break;
			
		case 130:
			// Raise Floor Turbo
			if (EV_DoFloor(line, raiseFloorTurbo) || !boomsupport)
				line->special = 0;
			break;
			
		case 141:
			// Silent Ceiling Crush & Raise
			if (EV_DoCeiling(line, silentCrushAndRaise) || !boomsupport)
				line->special = 0;
			break;
			
			//SoM: FraggleScript
		case 273:				//(1sided)
			if (side)
				break;
				
		case 272:				//(2sided)
			t_trigger = thing;
			T_RunScript(line->tag);
			break;
			
			// once-only triggers
		case 275:				//(1sided)
			if (side)
				break;
				
		case 274:				//(2sided)
			t_trigger = thing;
			T_RunScript(line->tag);
			line->special = 0;	// clear trigger
			break;
			
			// RETRIGGERS.  All from here till end.
		case 72:
			// Ceiling Crush
			EV_DoCeiling(line, lowerAndCrush);
			break;
			
		case 73:
			// Ceiling Crush and Raise
			EV_DoCeiling(line, crushAndRaise);
			break;
			
		case 74:
			// Ceiling Crush Stop
			EV_CeilingCrushStop(line);
			break;
			
		case 75:
			// Close Door
			EV_DoDoor(line, doorclose, VDOORSPEED);
			break;
			
		case 76:
			// Close Door 30
			EV_DoDoor(line, close30ThenOpen, VDOORSPEED);
			break;
			
		case 77:
			// Fast Ceiling Crush & Raise
			EV_DoCeiling(line, fastCrushAndRaise);
			break;
			
		case 79:
			// Lights Very Dark
			EV_LightTurnOn(line, 35);
			break;
			
		case 80:
			// Light Turn On - brightest near
			EV_LightTurnOn(line, 0);
			break;
			
		case 81:
			// Light Turn On 255
			EV_LightTurnOn(line, 255);
			break;
			
		case 82:
			// Lower Floor To Lowest
			EV_DoFloor(line, lowerFloorToLowest);
			break;
			
		case 83:
			// Lower Floor
			EV_DoFloor(line, lowerFloor);
			break;
			
		case 84:
			// LowerAndChange
			EV_DoFloor(line, lowerAndChange);
			break;
			
		case 86:
			// Open Door
			EV_DoDoor(line, dooropen, VDOORSPEED);
			break;
			
		case 87:
			// Perpetual Platform Raise
			EV_DoPlat(line, perpetualRaise, 0);
			break;
			
		case 88:
			// PlatDownWaitUp
			EV_DoPlat(line, downWaitUpStay, 0);
			break;
			
		case 89:
			// Platform Stop
			EV_StopPlat(line);
			break;
			
		case 90:
			// Raise Door
			EV_DoDoor(line, normalDoor, VDOORSPEED);
			break;
			
		case 91:
			// Raise Floor
			EV_DoFloor(line, raiseFloor);
			break;
			
		case 92:
			// Raise Floor 24
			EV_DoFloor(line, raiseFloor24);
			break;
			
		case 93:
			// Raise Floor 24 And Change
			EV_DoFloor(line, raiseFloor24AndChange);
			break;
			
		case 94:
			// Raise Floor Crush
			EV_DoFloor(line, raiseFloorCrush);
			break;
			
		case 95:
			// Raise floor to nearest height
			// and change texture.
			EV_DoPlat(line, raiseToNearestAndChange, 0);
			break;
			
		case 96:
			// Raise floor to shortest texture height
			// on either side of lines.
			EV_DoFloor(line, raiseToTexture);
			break;
			
		case 97:
			// TELEPORT!
			EV_Teleport(line, side, thing);
			break;
			
		case 98:
			// Lower Floor (TURBO)
			EV_DoFloor(line, turboLower);
			break;
			
		case 105:
			// Blazing Door Raise (faster than TURBO!)
			EV_DoDoor(line, blazeRaise, 4 * VDOORSPEED);
			break;
			
		case 106:
			// Blazing Door Open (faster than TURBO!)
			EV_DoDoor(line, blazeOpen, 4 * VDOORSPEED);
			break;
			
		case 107:
			// Blazing Door Close (faster than TURBO!)
			EV_DoDoor(line, blazeClose, 4 * VDOORSPEED);
			break;
			
		case 120:
			// Blazing PlatDownWaitUpStay.
			EV_DoPlat(line, blazeDWUS, 0);
			break;
			
		case 126:
			// TELEPORT MonsterONLY.
			if (!thing->player)
				EV_Teleport(line, side, thing);
			break;
			
		case 128:
			// Raise To Nearest Floor
			EV_DoFloor(line, raiseFloorToNearest);
			break;
			
		case 129:
			// Raise Floor Turbo
			EV_DoFloor(line, raiseFloorTurbo);
			break;
			
			// SoM:3/4/2000: Extended Boom W* triggers.
		default:
			if (boomsupport)
			{
				switch (line->special)
				{
						//SoM: 3/4/2000:Boom Walk once triggers.
						//SoM: 3/4/2000:Yes this is "copied" code! I just cleaned it up. Did you think I was going to retype all this?!
					case 142:
						// Raise Floor 512
						if (EV_DoFloor(line, raiseFloor512))
							line->special = 0;
						break;
						
					case 143:
						// Raise Floor 24 and change
						if (EV_DoPlat(line, raiseAndChange, 24))
							line->special = 0;
						break;
						
					case 144:
						// Raise Floor 32 and change
						if (EV_DoPlat(line, raiseAndChange, 32))
							line->special = 0;
						break;
						
					case 145:
						// Lower Ceiling to Floor
						if (EV_DoCeiling(line, lowerToFloor))
							line->special = 0;
						break;
						
					case 146:
						// Lower Pillar, Raise Donut
						if (EV_DoDonut(line))
							line->special = 0;
						break;
						
					case 199:
						// Lower ceiling to lowest surrounding ceiling
						if (EV_DoCeiling(line, lowerToLowest))
							line->special = 0;
						break;
						
					case 200:
						// Lower ceiling to highest surrounding floor
						if (EV_DoCeiling(line, lowerToMaxFloor))
							line->special = 0;
						break;
						
					case 207:
						// W1 silent teleporter (normal kind)
						if (EV_SilentTeleport(line, side, thing))
							line->special = 0;
						break;
						
					case 153:
						// Texture/Type Change Only (Trig)
						if (EV_DoChange(line, trigChangeOnly))
							line->special = 0;
						break;
						
					case 239:
						// Texture/Type Change Only (Numeric)
						if (EV_DoChange(line, numChangeOnly))
							line->special = 0;
						break;
						
					case 219:
						// Lower floor to next lower neighbor
						if (EV_DoFloor(line, lowerFloorToNearest))
							line->special = 0;
						break;
						
					case 227:
						// Raise elevator next floor
						if (EV_DoElevator(line, elevateUp))
							line->special = 0;
						break;
						
					case 231:
						// Lower elevator next floor
						if (EV_DoElevator(line, elevateDown))
							line->special = 0;
						break;
						
					case 235:
						// Elevator to current floor
						if (EV_DoElevator(line, elevateCurrent))
							line->special = 0;
						break;
						
					case 243:
						// W1 silent teleporter (linedef-linedef kind)
						if (EV_SilentLineTeleport(line, side, thing, false))
							line->special = 0;
						break;
						
					case 262:
						if (EV_SilentLineTeleport(line, side, thing, true))
							line->special = 0;
						break;
						
					case 264:
						if (!thing->player && EV_SilentLineTeleport(line, side, thing, true))
							line->special = 0;
						break;
						
					case 266:
						if (!thing->player && EV_SilentLineTeleport(line, side, thing, false))
							line->special = 0;
						break;
						
					case 268:
						if (!thing->player && EV_SilentTeleport(line, side, thing))
							line->special = 0;
						break;
						
						// Extended walk many retriggerable
						
						//Boom added lots of linedefs to fill in the gaps in trigger types
						
					case 147:
						// Raise Floor 512
						EV_DoFloor(line, raiseFloor512);
						break;
						
					case 148:
						// Raise Floor 24 and Change
						EV_DoPlat(line, raiseAndChange, 24);
						break;
						
					case 149:
						// Raise Floor 32 and Change
						EV_DoPlat(line, raiseAndChange, 32);
						break;
						
					case 150:
						// Start slow silent crusher
						EV_DoCeiling(line, silentCrushAndRaise);
						break;
						
					case 151:
						// RaiseCeilingLowerFloor
						EV_DoCeiling(line, raiseToHighest);
						EV_DoFloor(line, lowerFloorToLowest);
						break;
						
					case 152:
						// Lower Ceiling to Floor
						EV_DoCeiling(line, lowerToFloor);
						break;
						
					case 256:
						// Build stairs, step 8
						EV_BuildStairs(line, build8);
						break;
						
					case 257:
						// Build stairs, step 16
						EV_BuildStairs(line, turbo16);
						break;
						
					case 155:
						// Lower Pillar, Raise Donut
						EV_DoDonut(line);
						break;
						
					case 156:
						// Start lights strobing
						EV_StartLightStrobing(line);
						break;
						
					case 157:
						// Lights to dimmest near
						EV_TurnTagLightsOff(line);
						break;
						
					case 201:
						// Lower ceiling to lowest surrounding ceiling
						EV_DoCeiling(line, lowerToLowest);
						break;
						
					case 202:
						// Lower ceiling to highest surrounding floor
						EV_DoCeiling(line, lowerToMaxFloor);
						break;
						
					case 208:
						// WR silent teleporter (normal kind)
						EV_SilentTeleport(line, side, thing);
						break;
						
					case 212:
						// Toggle floor between C and F instantly
						EV_DoPlat(line, toggleUpDn, 0);
						break;
						
					case 154:
						// Texture/Type Change Only (Trigger)
						EV_DoChange(line, trigChangeOnly);
						break;
						
					case 240:
						// Texture/Type Change Only (Numeric)
						EV_DoChange(line, numChangeOnly);
						break;
						
					case 220:
						// Lower floor to next lower neighbor
						EV_DoFloor(line, lowerFloorToNearest);
						break;
						
					case 228:
						// Raise elevator next floor
						EV_DoElevator(line, elevateUp);
						break;
						
					case 232:
						// Lower elevator next floor
						EV_DoElevator(line, elevateDown);
						break;
						
					case 236:
						// Elevator to current floor
						EV_DoElevator(line, elevateCurrent);
						break;
						
					case 244:
						// WR silent teleporter (linedef-linedef kind)
						EV_SilentLineTeleport(line, side, thing, false);
						break;
						
					case 263:
						//Silent line-line reversed
						EV_SilentLineTeleport(line, side, thing, true);
						break;
						
					case 265:
						//Monster-only silent line-line reversed
						if (!thing->player)
							EV_SilentLineTeleport(line, side, thing, true);
						break;
						
					case 267:
						//Monster-only silent line-line
						if (!thing->player)
							EV_SilentLineTeleport(line, side, thing, false);
						break;
						
					case 269:
						//Monster-only silent
						if (!thing->player)
							EV_SilentTeleport(line, side, thing);
						break;
				}
			}
	}
#endif
}

//
// P_ShootSpecialLine - IMPACT SPECIALS
// Called when a thing shoots a special line.
//
void P_ShootSpecialLine(mobj_t* thing, line_t* line)
{
	int ok;
	bool_t UseAgain;
	
	/* Better Generalized Support */
	// GhostlyDeath <May 2, 2012> -- This is MUCH better than before!
	UseAgain = false;
	if (EV_TryGenTrigger(line, -1, thing, EVTGT_SHOOT, 0, &UseAgain))
		P_ChangeSwitchTexture(line, UseAgain);
	return;

#if 0
	//SoM: 3/7/2000: Another General type check
	if (boomsupport)
	{
		// pointer to line function is NULL by default, set non-null if
		// line special is gun triggered generalized linedef type
		int (*linefunc) (line_t * line) = NULL;
		
		// check each range of generalized linedefs
		if ((unsigned)line->special >= GenFloorBase)
		{
			if (!thing->player)
				if ((line->special & FloorChange) || !(line->special & FloorModel))
					return;		// FloorModel is "Allow Monsters" if FloorChange is 0
			if (!line->tag)		//jff 2/27/98 all gun generalized types require tag
				return;
				
			linefunc = EV_DoGenFloor;
		}
		else if ((unsigned)line->special >= GenCeilingBase)
		{
			if (!thing->player)
				if ((line->special & CeilingChange) || !(line->special & CeilingModel))
					return;		// CeilingModel is "Allow Monsters" if CeilingChange is 0
			if (!line->tag)		//jff 2/27/98 all gun generalized types require tag
				return;
			linefunc = EV_DoGenCeiling;
		}
		else if ((unsigned)line->special >= GenDoorBase)
		{
			if (!thing->player)
			{
				if (!(line->special & DoorMonster))
					return;		// monsters disallowed from this door
				if (line->flags & ML_SECRET)	// they can't open secret doors either
					return;
			}
			if (!line->tag)		//jff 3/2/98 all gun generalized types require tag
				return;
			linefunc = EV_DoGenDoor;
		}
		else if ((unsigned)line->special >= GenLockedBase)
		{
			if (!thing->player)
				return;			// monsters disallowed from unlocking doors
			if (((line->special & TriggerType) == GunOnce) || ((line->special & TriggerType) == GunMany))
			{
				//jff 4/1/98 check for being a gun type before reporting door type
				if (!P_CanUnlockGenDoor(line, thing->player))
					return;
			}
			else
				return;
			if (!line->tag)		//jff 2/27/98 all gun generalized types require tag
				return;
				
			linefunc = EV_DoGenLockedDoor;
		}
		else if ((unsigned)line->special >= GenLiftBase)
		{
			if (!thing->player)
				if (!(line->special & LiftMonster))
					return;		// monsters disallowed
			linefunc = EV_DoGenLift;
		}
		else if ((unsigned)line->special >= GenStairsBase)
		{
			if (!thing->player)
				if (!(line->special & StairMonster))
					return;		// monsters disallowed
			if (!line->tag)		//jff 2/27/98 all gun generalized types require tag
				return;
			linefunc = EV_DoGenStairs;
		}
		else if ((unsigned)line->special >= GenCrusherBase)
		{
			if (!thing->player)
				if (!(line->special & StairMonster))
					return;		// monsters disallowed
			if (!line->tag)		//jff 2/27/98 all gun generalized types require tag
				return;
			linefunc = EV_DoGenCrusher;
		}
		
		if (linefunc)
			switch ((line->special & TriggerType) >> TriggerTypeShift)
			{
				case GunOnce:
					if (linefunc(line))
						P_ChangeSwitchTexture(line, 0);
					return;
				case GunMany:
					if (linefunc(line))
						P_ChangeSwitchTexture(line, 1);
					return;
				default:		// if not a gun type, do nothing here
					return;
			}
	}
	//  Impacts that other things can activate.
	if (!thing->player)
	{
		ok = 0;
		switch (line->special)
		{
			case 46:
				// OPEN DOOR IMPACT
				ok = 1;
				break;
		}
		if (!ok)
			return;
	}
	
	if (!P_CheckTag(line))
		return;
		
	switch (line->special)
	{
		case 24:
			// RAISE FLOOR
			if (EV_DoFloor(line, raiseFloor) || !boomsupport)
				P_ChangeSwitchTexture(line, 0);
			break;
			
		case 46:
			// OPEN DOOR
			if (EV_DoDoor(line, dooropen, VDOORSPEED) || !boomsupport)
				P_ChangeSwitchTexture(line, 1);
			break;
			
		case 47:
			// RAISE FLOOR NEAR AND CHANGE
			if (EV_DoPlat(line, raiseToNearestAndChange, 0) || !boomsupport)
				P_ChangeSwitchTexture(line, 0);
			break;
			
			//SoM: FraggleScript
		case 278:
		case 279:
			t_trigger = thing;
			T_RunScript(line->tag);
			if (line->special == 279)
				line->special = 0;	// clear if G1
			break;
			
		default:
			if (boomsupport)
				switch (line->special)
				{
					case 197:
						// Exit to next level
						if (cv_allowexitlevel.value)
						{
							P_ChangeSwitchTexture(line, 0);
							G_ExitLevel();
						}
						break;
						
					case 198:
						// Exit to secret level
						if (cv_allowexitlevel.value)
						{
							P_ChangeSwitchTexture(line, 0);
							G_SecretExitLevel();
						}
						break;
						//jff end addition of new gun linedefs
				}
			break;
	}
#endif
}

#if 0
/* P_ProcessSpecialSectorEx() -- Handles object in special sector */
void P_ProcessSpecialSectorEx(const EV_TryGenType_t a_Type, mobj_t* const a_Mo, player_t* const a_Player, sector_t* const a_Sector, const bool_t a_InstaDamage)
{
	if (sector->special < 32)
	{
		// Has hitten ground.
		switch (sector->special)
		{
			case 5:
				// HELLSLIME DAMAGE
				if (!player->powers[pw_ironfeet])
					if (instantdamage)
					{
						P_DamageMobj(player->mo, NULL, NULL, 10);
						
						// spawn a puff of smoke
						//CONL_PrintF ("damage!\n"); //debug
						if (P_XGSVal(PGS_COENABLEFLOORSMOKE))
							P_SpawnSmoke(player->mo->x, player->mo->y, player->mo->z);
					}
				break;
				
			case 7:
				// NUKAGE DAMAGE
				if (!player->powers[pw_ironfeet])
					if (instantdamage)
						P_DamageMobj(player->mo, NULL, NULL, 5);
				break;
				
			case 16:
				// SUPER HELLSLIME DAMAGE
			case 4:
				// STROBE HURT
				if (!player->powers[pw_ironfeet] || (P_Random() < 5))
				{
					if (instantdamage)
						P_DamageMobj(player->mo, NULL, NULL, 20);
				}
				break;
				
			case 9:
				// SECRET SECTOR
				player->secretcount++;
				sector->special = 0;
				
				//faB: useful only in single & coop.
				if (!cv_deathmatch.value && players - player == g_Splits[0].Display)
					CONL_PrintF("\x02You found a secret area!\n");
					
				break;
				
			case 11:
				// EXIT SUPER DAMAGE! (for E1M8 finale)
				player->cheats &= ~CF_GODMODE;
				
				if (instantdamage)
					P_DamageMobj(player->mo, NULL, NULL, 20);
					
				if ((player->health <= 10) && P_XGSVal(PGS_GAMEALLOWLEVELEXIT))
					G_ExitLevel();
				break;
				
			default:
				//SoM: 3/8/2000: Just ignore.
				//CONL_PrintF ("P_PlayerInSpecialSector: unknown special %i",
				//             sector->special);
				break;
		};
	}
	else						//SoM: Extended sector types for secrets and damage
	{
		switch ((sector->special & DAMAGE_MASK) >> DAMAGE_SHIFT)
		{
			case 0:			// no damage
				break;
			case 1:			// 2/5 damage per 31 ticks
				if (!player->powers[pw_ironfeet] && instantdamage)
					P_DamageMobj(player->mo, NULL, NULL, 5);
				break;
			case 2:			// 5/10 damage per 31 ticks
				if (!player->powers[pw_ironfeet] && instantdamage)
					P_DamageMobj(player->mo, NULL, NULL, 10);
				break;
			case 3:			// 10/20 damage per 31 ticks
				if ((!player->powers[pw_ironfeet] || P_Random() < 5) && instantdamage)	// take damage even with suit
				{
					P_DamageMobj(player->mo, NULL, NULL, 20);
				}
				break;
		}
		if (sector->special & SECRET_MASK)
		{
			player->secretcount++;
			sector->special &= ~SECRET_MASK;
			
			if (!cv_deathmatch.value && players - player == g_Splits[0].Display)
				CONL_PrintF("\2You found a secret area!\n");
				
			if (sector->special < 32)
				sector->special = 0;
		}
	}
}
#endif

//
// P_PlayerOnSpecial3DFloor
// Checks to see if a player is standing on or is inside a 3D floor (water)
// and applies any speicials..
void P_PlayerOnSpecial3DFloor(player_t* player)
{
	sector_t* sector;
	bool_t instantdamage = false;
	ffloor_t* rover;
	
	sector = player->mo->subsector->sector;
	if (!sector->ffloors)
		return;
		
	for (rover = sector->ffloors; rover; rover = rover->next)
	{
		if (!rover->master->frontsector->special)
			continue;
			
		// Check the 3D floor's type...
		if (rover->flags & FF_SOLID)
		{
			// Player must be on top of the floor to be affected...
			if (player->mo->z != *rover->topheight)
				continue;
				
			if (P_XGSVal(PGS_CODAMAGEONLAND) && (player->mo->eflags & MF_JUSTHITFLOOR) && sector->heightsec == -1 && (leveltime % (2)))	//SoM: penalize jumping less.
				instantdamage = true;
			else
				instantdamage = !(leveltime % (32));
		}
		else
		{
			//Water and DEATH FOG!!! heh
			if (player->mo->z > *rover->topheight || (player->mo->z + player->mo->height) < *rover->bottomheight)
				continue;
			instantdamage = !(leveltime % (32));
		}
		
		P_ProcessSpecialSectorEx(EVTGT_WALK, player->mo, player, rover->master->frontsector, instantdamage);
	}
}

//
// P_PlayerInSpecialSector
// Called every tic frame
//  that the player origin is in a special sector
//
void P_PlayerInSpecialSector(player_t* player)
{
	sector_t* sector;
	bool_t instantdamage = false;
	
	// SoM: Check 3D floors...
	P_PlayerOnSpecial3DFloor(player);
	
	sector = player->mo->subsector->sector;
	
	//Fab: keep track of what sector type the player's currently in
	player->specialsector = sector->special;
	
	if (!player->specialsector)	// nothing special, exit
		return;
		
	// Falling, not all the way down yet?
	//SoM: 3/17/2000: Damage if in slimey water!
	if (sector->heightsec != -1)
	{
		if (player->mo->z > sectors[sector->heightsec].floorheight)
			return;
	}
	else if (player->mo->z != sector->floorheight)
		return;
		
	//Fab: jumping in lava/slime does instant damage (no jump cheat)
	if (P_XGSVal(PGS_CODAMAGEONLAND) && (player->mo->eflags & MF_JUSTHITFLOOR) && sector->heightsec == -1 && (leveltime % (2)))	//SoM: penalize jumping less.
		instantdamage = true;
	else
		instantdamage = !(leveltime % (32));
		
	P_ProcessSpecialSectorEx(EVTGT_WALK, player->mo, player, sector, instantdamage);
}

//
// P_UpdateSpecials
// Animate planes, scroll walls, etc.
//

void P_UpdateSpecials(void)
{
	anim_t* anim;
	int i;
	int pic;					//SoM: 3/8/2000
	
	levelflat_t* foundflats;	// for flat animation
	
	//  LEVEL TIMER
	if (P_XGSVal(PGS_GAMETIMELIMIT) && (((uint32_t)P_XGSVal(PGS_GAMETIMELIMIT)) * (TICRATE * 60)) < leveltime)
		G_ExitLevel();
		
	//  ANIMATE TEXTURES
	for (anim = anims; anim < lastanim; anim++)
	{
		if (anim->basepic && anim->picnum)
			for (i = anim->basepic; i < anim->basepic + anim->numpics; i++)
			{
				pic = anim->basepic + ((leveltime / anim->speed + i) % anim->numpics);
				textures[i]->Translation = pic;
			}
	}
	
	//  DO BUTTONS
	for (i = 0; i < MAXBUTTONS; i++)
		if (buttonlist[i].btimer)
		{
			buttonlist[i].btimer--;
			if (!buttonlist[i].btimer)
			{
				switch (buttonlist[i].where)
				{
					case top:
						sides[buttonlist[i].line->sidenum[0]].toptexture = buttonlist[i].btexture;
						break;
						
					case middle:
						sides[buttonlist[i].line->sidenum[0]].midtexture = buttonlist[i].btexture;
						break;
						
					case bottom:
						sides[buttonlist[i].line->sidenum[0]].bottomtexture = buttonlist[i].btexture;
						break;
				}
				S_StartSound((mobj_t*)&buttonlist[i].soundorg, sfx_generic_switchon);
				memset(&buttonlist[i], 0, sizeof(button_t));
			}
		}
		
}

//SoM: 3/8/2000: EV_DoDonut moved to p_floor.c

//SoM: 3/23/2000: Adds a sectors floor and ceiling to a sector's ffloor list
void P_AddFakeFloor(sector_t* sec, sector_t* sec2, line_t* master, int flags);
void P_AddFFloor(sector_t* sec, ffloor_t* ffloor);

void P_AddFakeFloor(sector_t* sec, sector_t* sec2, line_t* master, int flags)
{
	ffloor_t* ffloor;
	
	if (sec2->numattached == 0)
	{
		sec2->attached = malloc(sizeof(int));
		sec2->attached[0] = sec - sectors;
		sec2->numattached = 1;
	}
	else
	{
		int i;
		
		for (i = 0; i < sec2->numattached; i++)
			if (sec2->attached[i] == sec - sectors)
				return;
				
		sec2->attached = realloc(sec2->attached, sizeof(int) * (sec2->numattached + 1));
		sec2->attached[sec2->numattached] = sec - sectors;
		sec2->numattached++;
	}
	
	//Add the floor
	ffloor = Z_Malloc(sizeof(ffloor_t), PU_LEVEL, NULL);
	ffloor->secnum = sec2 - sectors;
	ffloor->target = sec;
	
	ffloor->bottomheight = &sec2->floorheight;
	ffloor->bottompic = &sec2->floorpic;
	//ffloor->bottomlightlevel = &sec2->lightlevel;
	ffloor->bottomxoffs = &sec2->floor_xoffs;
	ffloor->bottomyoffs = &sec2->floor_yoffs;
	
	//Add the ceiling
	ffloor->topheight = &sec2->ceilingheight;
	ffloor->toppic = &sec2->ceilingpic;
	ffloor->toplightlevel = &sec2->lightlevel;
	ffloor->topxoffs = &sec2->ceiling_xoffs;
	ffloor->topyoffs = &sec2->ceiling_yoffs;
	
	ffloor->flags = flags;
	ffloor->master = master;
	
	if (flags & FF_TRANSLUCENT)
	{
		if (sides[master->sidenum[0]].toptexture > 0)
			ffloor->alpha = sides[master->sidenum[0]].toptexture;
		else
			ffloor->alpha = 0x80;	// 127
	}
	else
		ffloor->alpha = 0xFF;	// Not transparent
	
	P_AddFFloor(sec, ffloor);
}

// GhostlyDeath <May 10, 2012> -- Fake Floor List (For Savegames)
ffloor_t** g_PFakeFloors = NULL;				// Fake Floors
size_t g_NumPFakeFloors = 0;					// Number of them

/* P_AddFFloor() -- Adds new 3D floor */
void P_AddFFloor(sector_t* sec, ffloor_t* ffloor)
{
	ffloor_t* rover;
	size_t i;
	bool_t Found;
	
	// GhostlyDeath <May 10, 2012> -- Fake Floor List (For Savegames)
		// See if the floor is already in the floor list
	Found = false;
	for (i = 0; i < g_NumPFakeFloors; i++)
		if (ffloor == g_PFakeFloors[i])
		{
			Found = true;
			break;
		}
	
	// If it was not found, resize the list
	if (!Found)
	{	
		// Resize and add
		Z_ResizeArray((void**)&g_PFakeFloors, sizeof(*g_PFakeFloors),
			g_NumPFakeFloors, g_NumPFakeFloors + 1);
		g_PFakeFloors[g_NumPFakeFloors++] = ffloor;
		
		// Make sure it gets free when the level is cleared
		Z_ChangeTag(g_PFakeFloors, PU_LEVEL);
	}
	
	/* Sector has no floors attached */
	if (!sec->ffloors)
	{
		sec->ffloors = ffloor;
		ffloor->next = 0;
		ffloor->prev = 0;
		return;
	}
	
	/* Add to end of floor list */
	for (rover = sec->ffloors; rover->next; rover = rover->next);
	
	rover->next = ffloor;
	ffloor->prev = rover;
	ffloor->next = 0;
}

//
// SPECIAL SPAWNING
//

/*** CONSTANTS ***/

/* P_RMODSpecialEffectType_t -- Type of special effect to cause */
typedef enum P_RMODSpecialEffectType_e
{
	PRSE_DAMAGE,								// Damage
	PRSE_LIGHT,									// Lighting
	PRSE_DOOR,									// Door
	PRSE_LIFT,									// Lift
	PRSE_FLOORMOVER,							// Floor movement
	PRSE_CEILINGMOVER,							// Ceiling movement
	PRSE_WEATHER,								// Modify the weather
	PRSE_SCROLLER,								// Scrolling thing
	
	NUMPRMODSPECEFFECTTYPES
} P_RMODSpecialEffectType_t;

/* P_RMODSpecialTrigger_t -- Triggers for specials */
typedef enum P_RMODSpecialTrigger_e
{
	PRST_WALK,									// Walk trigger (walk over)
	PRST_MANUAL,								// Door-like trigger
	PRST_HITSCAN,								// Hitscan
	PRST_SWITCH,								// Switch
	
	NUMPRMODSPECTRIGGERS
} P_RMODSpecialTrigger_t;

/*** STRUCTURES ***/

/* P_RMODSpecials_t -- RMOD Specials */
typedef struct P_RMODSpecials_s
{
	size_t NumTouchers;							// Number of touch specials
	P_RMODTouchSpecial_t* Touchers;				// Touch specials
} P_RMODSpecials_t;

/*** GLOBALS ***/

size_t g_RMODNumTouchSpecials = 0;
P_RMODTouchSpecial_t** g_RMODTouchSpecials = NULL;

/*** FUNCTIONS ***/

/* PS_RMODSpecialEffects() -- Handles effects for specials */
static bool_t PS_RMODSpecialEffects(Z_Table_t* const a_Sub, void* const a_Data)
{
}

/* P_RMODH_Specials() -- Handles all specials */
bool_t P_RMODH_Specials(Z_Table_t* const a_Table, const WL_WADFile_t* const a_WAD, const D_RMODPrivates_t a_ID, D_RMODPrivate_t* const a_Private)
{
	const char* TableName;
	const char* Value;
	P_RMODTouchSpecial_t TempTouch;
	D_RMODPrivate_t* RealPrivate;
	P_RMODSpecials_t* Specs;
	
	/* Obtain private info for sector data */
	RealPrivate = D_GetRMODPrivate(a_WAD, DRMODP_SPECTOUCH);
	
	// No real private?
	if (!RealPrivate)
		RealPrivate = a_Private;
	
	/* Create specials list */
	// Does not exist
	if (!RealPrivate->Data)
	{
		RealPrivate->Size = sizeof(P_RMODSpecials_t);
		RealPrivate->Data = Z_Malloc(RealPrivate->Size, PU_WLDKRMOD, (void**)&RealPrivate->Data);
	}
	
	/* Set data area */
	Specs = RealPrivate->Data;
	
	/* Clear */
	memset(&TempTouch, 0, sizeof(TempTouch));
	
	/* General Stuff */
	// Handle effects
	//Z_TableSuperCallback(a_Table, PS_RMODSpecialEffects, (void*)TempMenu);
	
	/* Get table name */
	TableName = strchr(Z_TableName(a_Table), '#') + 1;
	
	// GhostlyDeath <May 7, 2012> -- Boot Status
	CONL_EarlyBootTic(TableName, true);
	
	/* Which special ID to handle? */
	switch (a_ID)
	{
			// Touch specials
		case DRMODP_SPECTOUCH:
			// Copy name to sprite
			strncpy(TempTouch.SpriteName, TableName, 4);
			
			if ((Value = Z_TableGetValue(a_Table, "Message")))
			{
				// Find legal message
				TempTouch.PickupMsgRef = DS_FindStringRef(Value);
				
				// If it was not found, fake it
				if (!TempTouch.PickupMsgRef)
					TempTouch.PickupMsgFaked = Z_StrDup(Value, PU_WLDKRMOD, NULL);
			}
			
			// Get Fields
				// Strings
			TempTouch.PickupSnd = D_RMODGetValueString(a_Table, "PickupSound", "itemup");
			TempTouch.GiveWeapon = D_RMODGetValueString(a_Table, "GiveWeapon", NULL);
			TempTouch.GiveAmmo = D_RMODGetValueString(a_Table, "GiveAmmo", NULL);
				
				// Boolean
			if (D_RMODGetValueBool(a_Table, "IsKeepIfNotNeeded", false))
				TempTouch.Flags |= PMTSF_KEEPNOTNEEDED;
			if (D_RMODGetValueBool(a_Table, "IsRemoveAlways", false))
				TempTouch.Flags |= PMTSF_REMOVEALWAYS;
			if (D_RMODGetValueBool(a_Table, "IsMonsterGrab", false))
				TempTouch.Flags |= PMTSF_MONSTERCANGRAB;
			if (D_RMODGetValueBool(a_Table, "IsDeValue", false))
				TempTouch.Flags |= PMTSF_DEVALUE;
			if (D_RMODGetValueBool(a_Table, "IsCapNormStat", false))
				TempTouch.Flags |= PMTSF_CAPNORMSTAT;
			if (D_RMODGetValueBool(a_Table, "IsCapMaxStat", false))
				TempTouch.Flags |= PMTSF_CAPMAXSTAT;
			if (D_RMODGetValueBool(a_Table, "IsGreaterArmorClass", false))
				TempTouch.Flags |= PMTSF_GREATERARMORCLASS;
			if (D_RMODGetValueBool(a_Table, "SetBackpack", false))
				TempTouch.Flags |= PMTSF_SETBACKPACK;
			
				// Integer
			TempTouch.ArmorClass = D_RMODGetValueInt(a_Table, "ArmorClass", 0);
			TempTouch.ArmorAmount = D_RMODGetValueInt(a_Table, "ArmorAmount", 0);
			TempTouch.HealthAmount = D_RMODGetValueInt(a_Table, "HealthAmount", 0);
			TempTouch.AmmoMul = D_RMODGetValueInt(a_Table, "AmmoMul", 1);
			TempTouch.MaxAmmoMul = D_RMODGetValueInt(a_Table, "MaxAmmoMul", 1);
			
			// Append to back of touch list
			Z_ResizeArray((void**)&Specs->Touchers, sizeof(*Specs->Touchers), Specs->NumTouchers, Specs->NumTouchers + 1);
			
			// Copy
			memmove(&Specs->Touchers[Specs->NumTouchers], &TempTouch, sizeof(TempTouch));
			Specs->NumTouchers++;
			return true;
		
			// Unknown
		default:
			return false;
	}
}

/* P_RMODO_Specials() -- Order for specials */
bool_t P_RMODO_Specials(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD, const D_RMODPrivates_t a_ID)
{
	const WL_WADFile_t* RoveWAD;
	D_RMODPrivate_t* RMODPrivate;
	P_RMODSpecials_t* Specials;
	P_RMODTouchSpecial_t* ThisToucher;
	P_RMODTouchSpecial_t* TargTouch;
	P_RMODTouchSpecial_t** TargRef;
	size_t i, j;
	
	/* Only allow from sector */
	if (a_ID != DRMODP_SPECTOUCH)
		return true;		// Fake true
	
	/* Merge touch specials */
	// Clear existing
	if (g_RMODTouchSpecials)
		Z_Free(g_RMODTouchSpecials);
	g_RMODTouchSpecials = NULL;
	g_RMODNumTouchSpecials = 0;
	
	// Merge all existing ones together
	for (RoveWAD = WL_IterateVWAD(NULL, true); RoveWAD; RoveWAD = WL_IterateVWAD(RoveWAD, true))
	{
		// Obtain private RMOD Stuff
		RMODPrivate = D_GetRMODPrivate(RoveWAD, DRMODP_SPECTOUCH);
		
		// Not found? Ignore this WAD then
		if (!RMODPrivate)
			continue;
		
		// Get specials
		Specials = RMODPrivate->Data;
		
		// No specials?
		if (!Specials)
			continue;
		
		// No touch specials in this at all?
		if (!Specials->NumTouchers)
			continue;
		
		// Debug
		if (devparm)
			CONL_PrintF("P_RMODO_Specials: There are %u special touches.\n", (unsigned int)Specials->NumTouchers);
		
		// Go through every toucher
		for (i = 0; i < Specials->NumTouchers; i++)
		{
			// Get current toucher
			ThisToucher = &Specials->Touchers[i];
			TargRef = NULL;
			
			// Find existing toucher with this sprite and replace
			for (j = 0; j < g_RMODNumTouchSpecials; j++)
				if (strncasecmp(ThisToucher->SpriteName, g_RMODTouchSpecials[j]->SpriteName, 4) == 0)
				{
					TargRef = &g_RMODTouchSpecials[j];
					break;
				}
			
			// Not found? then resize and add at end
			if (!TargRef)
			{
				// Resize
				Z_ResizeArray((void**)&g_RMODTouchSpecials, sizeof(*g_RMODTouchSpecials), g_RMODNumTouchSpecials, g_RMODNumTouchSpecials + 1);
				
				// At end
				TargRef = &g_RMODTouchSpecials[g_RMODNumTouchSpecials];
				g_RMODNumTouchSpecials++;
			}
			
			// Place ref here (replace whatever was here)
			*TargRef = ThisToucher;
			
			// Dereference
			TargTouch = *TargRef;
			
			// Faked message?
			if (!TargTouch->PickupMsgRef && TargTouch->PickupMsgFaked)
				TargTouch->PickupMsgRef = &TargTouch->PickupMsgFaked;
			
			// Clear active info
			TargTouch->ActSpriteNum = NUMSPRITES;
			TargTouch->ActGiveWeapon = NUMWEAPONS;
			TargTouch->ActGiveAmmo = NUMAMMO;
			
			// Reference each thing
			if (TargTouch->GiveWeapon)
				TargTouch->ActGiveWeapon = INFO_GetWeaponByName(TargTouch->GiveWeapon);
			if (TargTouch->GiveAmmo)
				TargTouch->ActGiveAmmo = INFO_GetAmmoByName(TargTouch->GiveAmmo);
			
			// Find sprite to map to
			for (j = 0; j < 4 && TargTouch->SpriteName[j]; j++)
				TargTouch->ActSpriteID |= ((uint32_t)toupper(TargTouch->SpriteName[j])) << (j * 8);
			
			//TargTouch->ActSpriteNum = INFO_SpriteNumByName(TargTouch->SpriteName);
		}
	}
	
	//size_t g_RMODNumTouchSpecials = 0;
	//P_RMODTouchSpecial_t** g_RMODTouchSpecials = NULL;
	
	/* Succes */
	return true;
}

/* P_RMODTouchSpecialForSprite() -- Find touch special via sprite */
P_RMODTouchSpecial_t* P_RMODTouchSpecialForSprite(const uint32_t a_SprNum)
{
	/* Check */
	if (a_SprNum < 0 || a_SprNum >= NUMSPRITES)
		return NULL;
	
	/* Return preknown array */
	return g_SprTouchSpecials[a_SprNum];
}

/* P_RMODTouchSpecialForCode() -- Find touch special via code */
P_RMODTouchSpecial_t* P_RMODTouchSpecialForCode(const uint32_t a_Code)
{
	size_t i;
	P_RMODTouchSpecial_t* Current;
	
	/* Look through list */
	for (i = 0; i < g_RMODNumTouchSpecials; i++)
	{
		// Get current
		Current = g_RMODTouchSpecials[i];
		
		// Got it?
		if (Current->ActSpriteID == a_Code)
			return Current;
	}
	
	/* Not Found */
	return NULL;
}

/* P_SpawnSpecials() -- Spawns sector specials (damagers, lights, floors, etc.) */
void P_SpawnSpecials(void)
{
	sector_t* sector;
	int i;
	int episode;
	
	episode = 1;
	
	//SoM: 3/8/2000: Boom level init functions
	P_RemoveAllActiveCeilings();
	P_RemoveAllActivePlats();
	for (i = 0; i < MAXBUTTONS; i++)
		memset(&buttonlist[i], 0, sizeof(button_t));
	
	P_InitTagLists();			//Create xref tables for tags
		
	/* Spawn Sector Specials */
	sector = sectors;
	for (i = 0; i < numsectors; i++, sector++)
	{
		// No special?
		if (!sector->special)
			continue;
		
		// Spawn Map Specials The Generalized Way
		P_ProcessSpecialSectorEx(EVTGT_MAPSTART, NULL, NULL, sector, false);
	}
	
	P_SpawnScrollers();			//Add generalized scrollers
	//P_SpawnFriction();			//New friction model using linedefs
	//P_SpawnPushers();			//New pusher model using linedefs
	
	/* Go through all lines and spawn map specials */
	for (i = 0; i < numlines; i++)
	{
		// Ignore special-less lines
		if (!lines[i].special)
			continue;
		
		// Execute any map start specials
		if (EV_TryGenTrigger(&lines[i], -1, NULL, EVTGT_MAPSTART, 0, NULL))
			lines[i].special = 0;	// Clear special
	}
}

/*
  SoM: 3/8/2000: General scrolling functions.
  T_Scroll,
  Add_Scroller,
  Add_WallScroller,
  P_SpawnScrollers
*/
//
// This function, with the help of r_plane.c and r_bsp.c, supports generalized
// scrolling floors and walls, with optional mobj-carrying properties, e.g.
// conveyor belts, rivers, etc. A linedef with a special type affects all
// tagged sectors the same way, by creating scrolling and/or object-carrying
// properties. Multiple linedefs may be used on the same sector and are
// cumulative, although the special case of scrolling a floor and carrying
// things on it, requires only one linedef. The linedef's direction determines
// the scrolling direction, and the linedef's length determines the scrolling
// speed. This was designed so that an edge around the sector could be used to
// control the direction of the sector's scrolling, which is usually what is
// desired.
//
// Process the active scrollers.

void T_Scroll(scroll_t* s)
{
	fixed_t dx = s->dx, dy = s->dy;
	int affectee = s->affectee;
	
	if (s->control != -1)
	{
		// compute scroll amounts based on a sector's height changes
		fixed_t height = sectors[s->control].floorheight + sectors[s->control].ceilingheight;
		fixed_t delta = height - s->last_height;
		
		s->last_height = height;
		dx = FixedMul(dx, delta);
		dy = FixedMul(dy, delta);
	}
	
	if (s->accel)
	{
		s->vdx = dx += s->vdx;
		s->vdy = dy += s->vdy;
	}
	
	if (!(dx | dy))				// no-op if both (x,y) offsets 0
		return;
		
	switch (s->type)
	{
			side_t* side;
			sector_t* sec;
			fixed_t height, waterheight;
			msecnode_t* node;
			mobj_t* thing;
			
		case sc_side:			//Scroll wall texture
			side = sides + affectee;
			side->textureoffset += dx;
			side->rowoffset += dy;
			break;
			
		case sc_floor:			//Scroll floor texture
			sec = sectors + affectee;
			sec->floor_xoffs += dx;
			sec->floor_yoffs += dy;
			break;
			
		case sc_ceiling:		//Scroll ceiling texture
			sec = sectors + affectee;
			sec->ceiling_xoffs += dx;
			sec->ceiling_yoffs += dy;
			break;
			
		case sc_carry:
		
			sec = sectors + affectee;
			height = sec->floorheight;
			waterheight = sec->heightsec != -1 && sectors[sec->heightsec].floorheight > height ? sectors[sec->heightsec].floorheight : INT_MIN;
			
			for (node = sec->touching_thinglist; node; node = node->m_snext)
				if (!((thing = node->m_thing)->flags & MF_NOCLIP) && (!(thing->flags & MF_NOGRAVITY || thing->z > height) || thing->z < waterheight))
				{
					// Move objects only if on floor or underwater,
					// non-floating, and clipped.
					thing->momx += dx;
					thing->momy += dy;
				}
			break;
			
		case sc_carry_ceiling:	// to be added later
			break;
	}
}

//
// Add_Scroller()
//
// Add a generalized scroller to the thinker list.
//
// type: the enumerated type of scrolling: floor, ceiling, floor carrier,
//   wall, floor carrier & scroller
//
// (dx,dy): the direction and speed of the scrolling or its acceleration
//
// control: the sector whose heights control this scroller's effect
//   remotely, or -1 if no control sector
//
// affectee: the index of the affected object (sector or sidedef)
//
// accel: non-zero if this is an accelerative effect
//

static void Add_Scroller(int type, fixed_t dx, fixed_t dy, int control, int affectee, int accel)
{
	scroll_t* s = Z_Malloc(sizeof *s, PU_LEVSPEC, 0);
	
	s->thinker.function.acp1 = (actionf_p1) T_Scroll;
	s->type = type;
	s->dx = dx;
	s->dy = dy;
	s->accel = accel;
	s->vdx = s->vdy = 0;
	if ((s->control = control) != -1)
		s->last_height = sectors[control].floorheight + sectors[control].ceilingheight;
	s->affectee = affectee;
	P_AddThinker(&s->thinker, PTT_SCROLL);
}

// Adds wall scroller. Scroll amount is rotated with respect to wall's
// linedef first, so that scrolling towards the wall in a perpendicular
// direction is translated into vertical motion, while scrolling along
// the wall in a parallel direction is translated into horizontal motion.

static void Add_WallScroller(fixed_t dx, fixed_t dy, const line_t* l, int control, int accel)
{
	fixed_t x = abs(l->dx), y = abs(l->dy), d;
	
	if (y > x)
		d = x, x = y, y = d;
	d = FixedDiv(x, finesine[(tantoangle[FixedDiv(y, x) >> DBITS] + ANG90) >> ANGLETOFINESHIFT]);
	x = -FixedDiv(FixedMul(dy, l->dy) + FixedMul(dx, l->dx), d);
	y = -FixedDiv(FixedMul(dx, l->dy) - FixedMul(dy, l->dx), d);
	Add_Scroller(sc_side, x, y, control, *l->sidenum, accel);
}

// Amount (dx,dy) vector linedef is shifted right to get scroll amount
#define SCROLL_SHIFT 5

// Factor to scale scrolling effect into mobj-carrying properties = 3/32.
// (This is so scrolling floors and objects on them can move at same speed.)
#define CARRYFACTOR ((fixed_t)(FRACUNIT*.09375))

// Initialize the scrollers
static void P_SpawnScrollers(void)
{
	int i;
	line_t* l = lines;
	
	for (i = 0; i < numlines; i++, l++)
	{
		fixed_t dx = l->dx >> SCROLL_SHIFT;	// direction and speed of scrolling
		fixed_t dy = l->dy >> SCROLL_SHIFT;
		int control = -1, accel = 0;	// no control sector or acceleration
		int special = l->special;
		
		// Types 245-249 are same as 250-254 except that the
		// first side's sector's heights cause scrolling when they change, and
		// this linedef controls the direction and speed of the scrolling. The
		// most complicated linedef since donuts, but powerful :)
		
		if (special >= 245 && special <= 249)	// displacement scrollers
		{
			special += 250 - 245;
			control = sides[*l->sidenum].sector - sectors;
		}
		else if (special >= 214 && special <= 218)	// accelerative scrollers
		{
			accel = 1;
			special += 250 - 214;
			control = sides[*l->sidenum].sector - sectors;
		}
		
		switch (special)
		{
				register int s;
				
			case 250:			// scroll effect ceiling
				for (s = -1; (s = P_FindSectorFromLineTag(l, s)) >= 0;)
					Add_Scroller(sc_ceiling, -dx, dy, control, s, accel);
				break;
				
			case 251:			// scroll effect floor
			case 253:			// scroll and carry objects on floor
				for (s = -1; (s = P_FindSectorFromLineTag(l, s)) >= 0;)
					Add_Scroller(sc_floor, -dx, dy, control, s, accel);
				if (special != 253)
					break;
					
			case 252:			// carry objects on floor
				dx = FixedMul(dx, CARRYFACTOR);
				dy = FixedMul(dy, CARRYFACTOR);
				for (s = -1; (s = P_FindSectorFromLineTag(l, s)) >= 0;)
					Add_Scroller(sc_carry, dx, dy, control, s, accel);
				break;
				
				// scroll wall according to linedef
				// (same direction and speed as scrolling floors)
			case 254:
				for (s = -1; (s = P_FindLineFromLineTag(l, s)) >= 0;)
					if (s != i)
						Add_WallScroller(dx, dy, lines + s, control, accel);
				break;
				
			case 255:
				s = lines[i].sidenum[0];
				Add_Scroller(sc_side, -sides[s].textureoffset, sides[s].rowoffset, -1, s, accel);
				break;
				
			case 48:			// scroll first side
				Add_Scroller(sc_side, FRACUNIT, 0, -1, lines[i].sidenum[0], accel);
				break;
				
			case 99:			// heretic right scrolling
				break;			// doom use it as bluekeydoor
				
			case 85:			// jff 1/30/98 2-way scroll
				Add_Scroller(sc_side, -FRACUNIT, 0, -1, lines[i].sidenum[0], accel);
				break;
		}
	}
}

/*
  SoM: 3/8/2000: Friction functions start.
  Add_Friction,
  T_Friction,
  P_SpawnFriction
*/

// Adds friction thinker.
void Add_Friction(int friction, int movefactor, int affectee)
{
	friction_t* f = Z_Malloc(sizeof *f, PU_LEVSPEC, 0);
	
	f->thinker.function.acp1 = (actionf_p1) T_Friction;
	f->friction = friction;
	f->movefactor = movefactor;
	f->affectee = affectee;
	P_AddThinker(&f->thinker, PTT_FRICTION);
}

//Function to apply friction to all the things in a sector.
void T_Friction(friction_t* f)
{
	sector_t* sec;
	mobj_t* thing;
	msecnode_t* node;
	bool_t foundfloor = false;
	
	if (!P_XGSVal(PGS_COBOOMSUPPORT) || !variable_friction)
		return;
		
	sec = sectors + f->affectee;
	
	// Be sure the special sector type is still turned on. If so, proceed.
	// Else, bail out; the sector type has been changed on us.
	
	if (!(sec->special & FRICTION_MASK))
	{
		if (sec->ffloors)
		{
			ffloor_t* rover;
			
			for (rover = sec->ffloors; rover; rover = rover->next)
			{
				// Do some small extra checks here to possibly save unneeded work.
				if (!(rover->master->frontsector->special & FRICTION_MASK))
					continue;
				foundfloor = true;
			}
		}
		
		if (foundfloor == false)	// Not even a 3d floor has the FRICTION_MASK.
			return;
	}
	// Assign the friction value to players on the floor, non-floating,
	// and clipped. Normally the object's friction value is kept at
	// ORIG_FRICTION and this thinker changes it for icy or muddy floors.
	
	// In Phase II, you can apply friction to Things other than players.
	
	// When the object is straddling sectors with the same
	// floorheight that have different frictions, use the lowest
	// friction value (muddy has precedence over icy).
	
	node = sec->touching_thinglist;	// things touching this sector
	while (node)
	{
		thing = node->m_thing;
		if (thing->player && !(thing->flags & (MF_NOGRAVITY | MF_NOCLIP)) && thing->z == thing->floorz)
		{
			if (foundfloor && thing->z == sec->floorheight);	// Skip
			
			else if ((thing->friction == ORIG_FRICTION) ||	// normal friction?
			         (f->friction < thing->friction))
			{
				thing->friction = f->friction;
				thing->movefactor = f->movefactor;
			}
		}
		node = node->m_snext;
	}
}

//Spawn all friction.
static void P_SpawnFriction(void)
{
	int i;
	line_t* l = lines;
	register int s;
	
	for (i = 0; i < numlines; i++, l++)
		if (l->special == 223)
		{
		}
}

/*
  SoM: 3/8/2000: Push/Pull/Wind/Current functions.
  Add_Pusher,
  PIT_PushThing,
  T_Pusher,
  P_GetPushThing,
  P_SpawnPushers
*/

// Adds a pusher
void Add_Pusher(int type, int x_mag, int y_mag, mobj_t* source, int affectee)
{
	pusher_t* p = Z_Malloc(sizeof *p, PU_LEVSPEC, 0);
	
	p->thinker.function.acp1 = (actionf_p1) T_Pusher;
	p->source = source;
	p->type = type;
	p->x_mag = x_mag >> FRACBITS;
	p->y_mag = y_mag >> FRACBITS;
	
	// Magnitude is used for something else for vertical currents
	// SSNTails 06-14-2002
	if (type == p_downcurrent || type == p_upcurrent || type == p_upwind || type == p_downwind)
		p->magnitude = P_AproxDistance(p->x_mag, p->y_mag) << (FRACBITS - PUSH_FACTOR);
	else
		p->magnitude = P_AproxDistance(p->x_mag, p->y_mag);
		
	if (source)					// point source exist?
	{
		p->radius = (p->magnitude) << (FRACBITS + 1);	// where force goes to zero
		p->x = p->source->x;
		p->y = p->source->y;
	}
	p->affectee = affectee;
	P_AddThinker(&p->thinker, PTT_PUSHER);
}

// PIT_PushThing determines the angle and magnitude of the effect.
// The object's x and y momentum values are changed.

pusher_t* tmpusher;				// pusher structure for blockmap searches

bool_t PIT_PushThing(mobj_t* thing, void* a_Arg)
{
	if (thing->player && !(thing->flags & (MF_NOGRAVITY | MF_NOCLIP)))
	{
		angle_t pushangle;
		int dist;
		int speed;
		int sx, sy;
		
		sx = tmpusher->x;
		sy = tmpusher->y;
		dist = P_AproxDistance(thing->x - sx, thing->y - sy);
		speed = (tmpusher->magnitude - ((dist >> FRACBITS) >> 1)) << (FRACBITS - PUSH_FACTOR - 1);
		
		// If speed <= 0, you're outside the effective radius. You also have
		// to be able to see the push/pull source point.
		
		if ((speed > 0) && (P_CheckSight(thing, tmpusher->source)))
		{
			pushangle = R_PointToAngle2(thing->x, thing->y, sx, sy);
			
			// GhostlyDeath <March 6, 2012> -- Push away?
			if (tmpusher->source->RXFlags[0] & MFREXA_DOPUSHAWAY)
				pushangle += ANG180;	// away
			
			pushangle >>= ANGLETOFINESHIFT;
			thing->momx += FixedMul(speed, finecosine[pushangle]);
			thing->momy += FixedMul(speed, finesine[pushangle]);
		}
	}
	return true;
}

// T_Pusher looks for all objects that are inside the radius of
// the effect.

void T_Pusher(pusher_t* p)
{
	sector_t* sec;
	mobj_t* thing;
	msecnode_t* node;
	int xspeed = 0, yspeed = 0;
	int xl, xh, yl, yh, bx, by;
	int radius;
	int ht = 0;
	bool_t inwater;
	bool_t touching;
	fixed_t z;
	
	inwater = touching = false;
	
	if (!allow_pushers)
		return;
		
	sec = sectors + p->affectee;
	
	// Be sure the special sector type is still turned on. If so, proceed.
	// Else, bail out; the sector type has been changed on us.
	
	if (P_XGSVal(PGS_COOLDFLATPUSHERCODE))
	{
		if (!(sec->special & PUSH_MASK))
			return;
	}
	else						// Now you can have a PUSH_MASK for individual 3d floors. SSNTails 09-25-2002
	{
		if (!(sec->special & PUSH_MASK))	// Main sector doesn't have one, so let's check the rovers.
		{
			bool_t foundfloor;
			
			foundfloor = false;
			
			if (sec->ffloors)
			{
				ffloor_t* rover;
				
				for (rover = sec->ffloors; rover; rover = rover->next)
				{
					// Do some small extra checks here to possibly save unneeded work.
					if (!(rover->master->frontsector->special & PUSH_MASK))
						continue;
					foundfloor = true;
				}
			}
			
			if (foundfloor == false)	// Not even a 3d floor has the PUSH_MASK.
				return;
		}
	}
	
	// For constant pushers (wind/current) there are 3 situations:
	//
	// 1) Affected Thing is above the floor.
	//
	//    Apply the full force if wind, no force if current.
	//
	// 2) Affected Thing is on the ground.
	//
	//    Apply half force if wind, full force if current.
	//
	// 3) Affected Thing is below the ground (underwater effect).
	//
	//    Apply no force if wind, full force if current.
	//
	// Apply the effect to clipped players only for now.
	//
	// In Phase II, you can apply these effects to Things other than players.
	
	if (p->type == p_push)
	{
	
		// Seek out all pushable things within the force radius of this
		// point pusher. Crosses sectors, so use blockmap.
		
		tmpusher = p;			// "LegacyPusher"/"LegacyPuller" point source
		radius = p->radius;		// where force goes to zero
		tmbbox[BOXTOP] = p->y + radius;
		tmbbox[BOXBOTTOM] = p->y - radius;
		tmbbox[BOXRIGHT] = p->x + radius;
		tmbbox[BOXLEFT] = p->x - radius;
		
		xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
		xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
		yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
		yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;
		for (bx = xl; bx <= xh; bx++)
			for (by = yl; by <= yh; by++)
				P_BlockThingsIterator(bx, by, PIT_PushThing, NULL);
		return;
	}
	// constant pushers p_wind and p_current
	
	if (P_XGSVal(PGS_COOLDFLATPUSHERCODE))
	{
		if (sec->heightsec != -1)	// special water sector?
			ht = sectors[sec->heightsec].floorheight;
			
		node = sec->touching_thinglist;	// things touching this sector
		for (; node; node = node->m_snext)
		{
			thing = node->m_thing;
			if (!thing->player || (thing->flags & (MF_NOGRAVITY | MF_NOCLIP)))
				continue;
			if (p->type == p_wind)
			{
				if (sec->heightsec == -1)	// NOT special water sector
					if (thing->z > thing->floorz)	// above ground
					{
						xspeed = p->x_mag;	// full force
						yspeed = p->y_mag;
					}
					else		// on ground
					{
						xspeed = (p->x_mag) >> 1;	// half force
						yspeed = (p->y_mag) >> 1;
					}
				else			// special water sector
				{
					if (thing->z > ht)	// above ground
					{
						xspeed = p->x_mag;	// full force
						yspeed = p->y_mag;
					}
					else if (thing->player->viewz < ht)	// underwater
						xspeed = yspeed = 0;	// no force
					else		// wading in water
					{
						xspeed = (p->x_mag) >> 1;	// half force
						yspeed = (p->y_mag) >> 1;
					}
				}
			}
			else				// p_current
			{
				// Added Z currents SSNTails 06-10-2002
				if (sec->heightsec == -1)	// NOT special water sector
					if (thing->z > sec->floorheight)	// above ground
						xspeed = yspeed = 0;	// no force
					else		// on ground
					{
						if (p->type == p_upcurrent)
							thing->momz += p->magnitude;
						else if (p->type == p_downcurrent)
							thing->momz -= p->magnitude;
						else
						{
							xspeed = p->x_mag;	// full force
							yspeed = p->y_mag;
						}
					}
				else			// special water sector
					if (thing->z > ht)	// above ground
						xspeed = yspeed = 0;	// no force
					else			// underwater
					{
						if (p->type == p_upcurrent)
							thing->momz += p->magnitude;
						else if (p->type == p_downcurrent)
							thing->momz -= p->magnitude;
						else
						{
							xspeed = p->x_mag;	// full force
							yspeed = p->y_mag;
						}
					}
			}
			
			if (p->type != p_downcurrent && p->type != p_upcurrent)
			{
				thing->momx += xspeed << (FRACBITS - PUSH_FACTOR);
				thing->momy += yspeed << (FRACBITS - PUSH_FACTOR);
			}
			
		}
	}
	else						// New support
	{
		node = sec->touching_thinglist;	// things touching this sector
		for (; node; node = node->m_snext)
		{
			thing = node->m_thing;
			if (!thing->player || (thing->flags & (MF_NOGRAVITY | MF_NOCLIP)))
				continue;
				
			// Find the area that the 'thing' is in
			// Kudos to P_MobjCheckWater().
			// SSNTails 09-25-2002
			if (sec->heightsec > -1 && sec->altheightsec == 1)
			{
				bool_t water;
				
				water = false;
				
				if (sec->heightsec > -1)	//water hack
				{
					z = (sectors[sec->heightsec].floorheight);
					water = true;
				}
				else if (sec->floortype == FLOOR_WATER || sec->floortype == FLOOR_LAVA)	// Lava support
				{
					z = sec->floorheight + (FRACUNIT / 4);	// water texture
					water = true;
				}
				
				if (water == true)	// Sector has water
				{
					if (thing->z <= z && thing->z + thing->height > z)
						touching = true;
						
					if (thing->z + (thing->height >> 1) <= z)
					{
						inwater = true;
					}
				}
			}
			// Not "else"! Check ALL possibilities!
			if ((inwater == false || touching == false)	// Only if both aren't true
			        && sec->ffloors)
			{
				ffloor_t* rover;
				
				for (rover = sec->ffloors; rover; rover = rover->next)
				{
					if (*rover->topheight < thing->z || *rover->bottomheight > (thing->z + (thing->height >> 1)))
						continue;
						
					if (!(rover->master->frontsector->special & PUSH_MASK))
						continue;
						
					if (thing->z + thing->height > *rover->topheight)
						touching = true;
						
					if (thing->z + (thing->height >> 1) < *rover->topheight)
					{
						inwater = true;
					}
				}
			}
			
			if (thing->z == sec->floorheight)
				touching = true;
				
			if (p->type == p_wind)
			{
				if (!touching && !inwater)	// above ground
				{
					xspeed = p->x_mag;	// full force
					yspeed = p->y_mag;
				}
				else if (touching)	// on ground
				{
					xspeed = (p->x_mag) >> 1;	// half force
					yspeed = (p->y_mag) >> 1;
				}
				else if (inwater)	// underwater
					xspeed = yspeed = 0;	// no force
				else
					xspeed = yspeed = 0;
			}
			else if (p->type == p_upwind)
			{
				if (!touching && !inwater)	// above ground
				{
					thing->momz += p->magnitude;
				}
				else if (touching)	// on ground
				{
					thing->momz += (p->magnitude) >> 1;
				}
				else if (inwater)	// underwater
					xspeed = yspeed = 0;	// no force
				else
					xspeed = yspeed = 0;
			}
			else if (p->type == p_downwind)
			{
				if (!touching && !inwater)	// above ground
				{
					thing->momz -= p->magnitude;
				}
				else if (touching)	// on ground
				{
					thing->momz -= (p->magnitude) >> 1;
				}
				else if (inwater)	// underwater
					xspeed = yspeed = 0;	// no force
				else
					xspeed = yspeed = 0;
			}
			else				// p_current
			{
				// Added Z currents SSNTails 06-10-2002
				if (!touching && !inwater)	// Not in water at all
					xspeed = yspeed = 0;	// no force
				else			// underwater / touching water
				{
					if (p->type == p_upcurrent)
						thing->momz += p->magnitude;
					else if (p->type == p_downcurrent)
						thing->momz -= p->magnitude;
					else
					{
						xspeed = p->x_mag;	// full force
						yspeed = p->y_mag;
					}
				}
			}
			
			if (p->type != p_downcurrent && p->type != p_upcurrent && p->type != p_upwind && p->type != p_downwind)
			{
				thing->momx += xspeed << (FRACBITS - PUSH_FACTOR);
				thing->momy += yspeed << (FRACBITS - PUSH_FACTOR);
			}
		}
	}
}

// Get pusher object.
mobj_t* P_GetPushThing(int s)
{
	mobj_t* thing;
	sector_t* sec;
	
	sec = sectors + s;
	thing = sec->thinglist;
	while (thing)
	{
		// GhostlyDeath <March 6, 2012> -- Is pusher/puller?
		if (thing->RXFlags[0] & MFREXA_ISPUSHPULL)
			return thing;
		
		// It is not
		thing = thing->snext;
	}
	
	return NULL;
}

mobj_t LavaInflictor;

//---------------------------------------------------------------------------
//
// FUNC P_GetThingFloorType
//
//---------------------------------------------------------------------------
int P_GetThingFloorType(mobj_t* thing)
{
	return thing->subsector->sector->floortype;
}

/******************
*** EXTRA STUFF ***
******************/

/*** CONSTANTS ***/
#define SPECMAXFIELDSZ						64	// Max field size

/*** STRUCTURES ***/

/* PS_SpecialMapSub_t -- Sub mapping */
typedef struct PS_SpecialMapSub_s
{
	char EnumName[SPECMAXFIELDSZ];				// Enumeration Name
	uint32_t Value;								// Value
} PS_SpecialMapSub_t;

/* PS_SpecialMapMinor_t -- Minor mapping */
typedef struct PS_SpecialMapMinor_s
{
	char OptName[SPECMAXFIELDSZ];				// Option Name
	uint32_t Shift;								// Shift value
	uint32_t Mask;								// Mask for value
	
	PS_SpecialMapSub_t* Subs;					// Subs
	size_t NumSubs;								// Number of subs
} PS_SpecialMapMinor_t;

/* PS_SpecialMapMajor_t -- Major mapping */
typedef struct PS_SpecialMapMajor_s
{
	char TypeName[SPECMAXFIELDSZ];				// Name of major trigger type
	uint32_t BaseInt;							// Base Integer
	
	PS_SpecialMapMinor_t* Minors;				// Minors
	size_t NumMinors;							// Number of them
} PS_SpecialMapMajor_t;

/*** GLOBALS ***/
P_BossSpitEntry_t* g_BossSpitList = NULL;		// List of things to spit out
size_t g_NumBossSpitList = 0;					// Count of those things
EV_ReGenMap_t* g_ReGenMap = NULL;				// List of generalization
size_t g_NumReGenMap = 0;						// Number of them

/*** FUNCTIONS ***/

/* PS_ExtraSpecialOCCB() -- Called when the order changes */
static bool_t PS_ExtraSpecialOCCB(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD)
{
#define BUFSIZE 512
	const WL_WADEntry_t* Entry;
	const WL_WADEntry_t* MapperEntry;
	WL_ES_t* Stream;
	size_t i, j, k, zzSpecType;
	char Buf[BUFSIZE];
	char* p, *TokStr;
	int32_t Val;
	mobjtype_t MoType;
	uint32_t Source, Target, Type, Temp;
	bool_t IgnoreTarg;
	const char* EGxM;
	const char* EGxL;
	bool_t TargetSet, TypeSet;
	
	PS_SpecialMapSub_t* ThisSub;
	PS_SpecialMapMinor_t* ThisMinor;
	PS_SpecialMapMajor_t* ThisMajor;
	PS_SpecialMapMajor_t* Majors;
	size_t NumMajors;

	/* Init */
	Majors = ThisMajor = ThisMinor = ThisSub = NULL;
	NumMajors = 0;
	
	/* Clear Regeneralized Map */
	if (g_ReGenMap)
		Z_Free(g_ReGenMap);
	g_ReGenMap = NULL;
	g_NumReGenMap = 0;
	
	/* Parse mappings data */
	for (zzSpecType = 0; zzSpecType < 2; zzSpecType++)
	{
		// Lines
		if (!zzSpecType)
		{
			EGxM = "RMD_EGLM";
			EGxL = "RMD_EGLL";
		}
		
		// Sectors
		else
		{
			EGxM = "RMD_EGSM";
			EGxL = "RMD_EGSL";
		}
		
		// This makes it less static!
		MapperEntry = WL_FindEntry(NULL, 0, EGxM);
	
		// Go through it
		if (MapperEntry)
		{
			// Open stream
			Stream = WL_StreamOpen(MapperEntry);
		
			// Did it work?
			if (Stream)
			{
#define __REMOOD_MAPTOKEN " \t\r\n"
				// Check unicode
				WL_StreamCheckUnicode(Stream);
			
				// Init
				ThisMajor = NULL;
				Majors = NULL;
				NumMajors = 0;
			
				// While there is no end
				while (!WL_StreamEOF(Stream))
				{
					// Read into buffer
					memset(Buf, 0, sizeof(Buf));
					WL_Srl(Stream, Buf, BUFSIZE);
				
					// If it starts with a #, a comment
					if (Buf[0] == '#')
						continue;
				
					// Why not use strtok(), It is fun and on top of that! insecure!
					TokStr = strtok(Buf, __REMOOD_MAPTOKEN);
				
					// No token
					if (!TokStr)
						continue;
				
					// Minor Definition
					if (strcasecmp(TokStr, "@") == 0)
					{
						// No major? Skip
						if (!ThisMajor)
							continue;
					
						// Ignore indicator comment
						TokStr = strtok(NULL, __REMOOD_MAPTOKEN);
						if (!TokStr)
							continue;
					
						// Make sure [ is reached
						TokStr = strtok(NULL, __REMOOD_MAPTOKEN);
						if (!TokStr || (TokStr && strcasecmp(TokStr, "[") != 0))
							continue;
					
						// Create Minor
						Z_ResizeArray((void**)&ThisMajor->Minors, sizeof(*ThisMajor->Minors),
								ThisMajor->NumMinors, ThisMajor->NumMinors + 1);
						ThisMinor = &ThisMajor->Minors[ThisMajor->NumMinors++];
					
						// Read Shift
						TokStr = strtok(NULL, __REMOOD_MAPTOKEN);
						if (!TokStr)
							continue;
						ThisMinor->Shift = C_strtou32(TokStr, NULL, 10);
					
						// Read Mask
						TokStr = strtok(NULL, __REMOOD_MAPTOKEN);
						if (!TokStr)
							continue;
						ThisMinor->Mask = C_strtou32(TokStr, NULL, 16);
					
						// Debug
						//if (devparm)
						//	CONL_PrintF(">>> = %i %x\n", ThisMinor->Shift, ThisMinor->Mask);
					
						// Clear initial value
						Target = 0;
					
						// Read sub values now
						for (;;)
						{
							// Get token
							TokStr = strtok(NULL, __REMOOD_MAPTOKEN);
							if (!TokStr || (TokStr && strcasecmp(TokStr, "]") == 0))
								break;
						
							// Add to sub
							Z_ResizeArray((void**)&ThisMinor->Subs, sizeof(*ThisMinor->Subs),
									ThisMinor->NumSubs, ThisMinor->NumSubs + 1);
							ThisSub = &ThisMinor->Subs[ThisMinor->NumSubs++];
						
							// Copy Values
							strncat(ThisSub->EnumName, TokStr, SPECMAXFIELDSZ - 1);
							ThisSub->Value = Target++;
						
							// Debug
							//if (devparm)
							//	CONL_PrintF(">>>> %s = %i\n", ThisSub->EnumName, ThisSub->Value);
						}
					}
				
					// Major Definition
					else
					{
						// Make a new spot at the end
						Z_ResizeArray((void**)&Majors, sizeof(*Majors), NumMajors, NumMajors + 1);
						ThisMajor = &Majors[NumMajors++];
					
						// Copy into major
						strncat(ThisMajor->TypeName, TokStr, SPECMAXFIELDSZ - 1);
					
						// Get name
						TokStr = strtok(NULL, __REMOOD_MAPTOKEN);
					
						// Missing?
						if (!TokStr)
						{
							ThisMajor = NULL;
							continue;
						}
					
						// Set Value (from hex)
						ThisMajor->BaseInt = C_strtou32(TokStr, NULL, 16);
					
						// Debug
						//if (devparm)
						//	CONL_PrintF(">> %s = %i\n", ThisMajor->TypeName, ThisMajor->BaseInt);
					}
				}
#undef __REMOOD_MAPTOKEN
			
				// Close stream
				WL_StreamClose(Stream);
			}
		}
		
		//// Parse Mappings
		// Load from lump (if it exists)
		Entry = WL_FindEntry(NULL, 0, EGxL);
	
		// Only if it was found, attempt to work with it
		if (Entry)
		{
			// Open stream
			Stream = WL_StreamOpen(Entry);
		
			// Did it work?
			if (Stream)
			{
				// Check unicode
				WL_StreamCheckUnicode(Stream);
			
				// While there is no end
				while (!WL_StreamEOF(Stream))
				{
					// Read into buffer
					memset(Buf, 0, sizeof(Buf));
					WL_Srl(Stream, Buf, BUFSIZE);
				
					// If it starts with a #, a comment
					if (Buf[0] == '#')
						continue;
				
					// Reset
					i = 0;
					IgnoreTarg = TargetSet = TypeSet = false;
					Source = Target = Type = 0;
					ThisMajor = NULL;
				
					// Why not use strtok(), It is fun and on top of that! insecure!
					TokStr = strtok(Buf, " ");
				
					// No token
					if (!TokStr)
						continue;
				
					// Parse loop
					do
					{
						// Get Target Type?
						if (i == 0)
						{
							// Get the type to translate from
							Source = C_strtou32(TokStr, NULL, 0);
						
							// Bad number?
							if (!Source/* || Source >= GenCrusherBase*/)
								break;
						
							// Next token
							i++;
						}
					
						// Get appearence
						else if (i == 1)
						{
							// Check for game compatibilities
							if (strcasecmp(TokStr, "ALL") != 0)
								if (
									(strcasecmp(TokStr, "DOOMHERETIC") == 0 && !(g_CoreGame == COREGAME_DOOM || g_CoreGame == COREGAME_HERETIC)) ||
									(strcasecmp(TokStr, "DOOM") == 0 && g_CoreGame != COREGAME_DOOM) ||
									(strcasecmp(TokStr, "HERETIC") == 0 && g_CoreGame != COREGAME_HERETIC) ||
									(strcasecmp(TokStr, "HEXEN") == 0 && g_CoreGame != COREGAME_HEXEN) ||
									(strcasecmp(TokStr, "STRIFE") == 0 && g_CoreGame != COREGAME_STRIFE))
									break;
							
							// Next token
							i++;
						}
					
						// Find Major Type
						else if (i == 2)
						{
							// Next token
							i++;
						
							// Find matching major
							ThisMajor = NULL;
							for (j = 0; j < NumMajors; j++)
								if (strcasecmp(TokStr, Majors[j].TypeName) == 0)
								{
									ThisMajor = &Majors[j];
									break;
								}
						
							// No major found?
							if (!ThisMajor)
								continue;
						
							// Set target
							Type = Target = ThisMajor->BaseInt;
							TargetSet = true;
							TypeSet = true;
						}
					
						// Handle field enums
						else
						{
							// No major found?
							if (!ThisMajor)
								continue;
						
							// Find matching enum
							ThisSub = NULL;
							for (j = 0; j < ThisMajor->NumMinors && !ThisSub; j++)
							{
								// Quick
								ThisMinor = &ThisMajor->Minors[j];
								ThisSub = NULL;
							
								// Loop
								for (k = 0; k < ThisMinor->NumSubs; k++)
									if (strcasecmp(TokStr, ThisMinor->Subs[k].EnumName) == 0)
									{
										ThisSub = &ThisMinor->Subs[k];
										break;
									}
							
								// Found?
								if (ThisSub)
									break;
							}
						
							// Not found?
							if (!ThisSub)
								continue;
						
							// Clear old bits from target then set
							Target &= ~ThisMinor->Mask;
							Target |= (ThisSub->Value << ThisMinor->Shift) & ThisMinor->Mask;
						}
					} while ((TokStr = strtok(NULL, " ")));
				
					// OR target to type
					Target |= Type;
				
					// Only if source, target, and type
					if (Source && TargetSet && TypeSet)
					{
						Z_ResizeArray((void**)&g_ReGenMap, sizeof(*g_ReGenMap), g_NumReGenMap, g_NumReGenMap + 1);
						g_ReGenMap[g_NumReGenMap].Sector = !!zzSpecType;
						g_ReGenMap[g_NumReGenMap].Source = Source;
						g_ReGenMap[g_NumReGenMap++].Target = Target;
					
						// Debug
						//if (devparm)
						//	CONL_PrintF("%s Mapped: %i -> %8x\n", (zzSpecType ? "Sector" : "Line"), Source, Target);
					}
				}
			
				// Close stream
				WL_StreamClose(Stream);
			}
		}
	
		/* Free special mapping */
		if (Majors)
		{
			for (i = 0; i < NumMajors; i++)
				if (Majors[i].Minors)
				{
					for (j = 0; j < Majors[i].NumMinors; j++)
						if (Majors[i].Minors[j].Subs)
							Z_Free(Majors[i].Minors[j].Subs);
					Z_Free(Majors[i].Minors);
				}
			Z_Free(Majors);
		}
	}
	
	/* Load boss spitters */
	// The boss spitter list is for internal constant removal and is not really
	// meant to be used by map authors, although it can be. Once ReMooD Script
	// is fully implemented, using that would be ALOT better than using this
	// minor hack to remove constants.
	
	// Clear
	if (g_BossSpitList)
		Z_Free(g_BossSpitList);
	g_BossSpitList = NULL;
	g_NumBossSpitList = 0;
	
	// Load from lump (if it exists)
	Entry = WL_FindEntry(NULL, 0, "RMD_BOSS");
	
	// Only if it was found, attempt to work with it
	if (Entry)
	{
		// Open stream
		Stream = WL_StreamOpen(Entry);
		
		// Did it work?
		if (Stream)
		{
			// Check unicode
			WL_StreamCheckUnicode(Stream);
			
			// While there is no end
			while (!WL_StreamEOF(Stream))
			{
				// Read into buffer
				memset(Buf, 0, sizeof(Buf));
				WL_Srl(Stream, Buf, BUFSIZE);
				
				// Prepare to read
				p = Buf;
				
				// Strip leading whitespace
				while (*p && (*p == ' ' || *p == '\t'))
					p++;
				
				// Read number
				for (Val = 0; *p && (*p >= '0' && *p <= '9'); p++)
				{
					// Multiply value by 10 then add the current p
					Val *= 10;
					Val += (*p - '0');
				}
				
				// Skip white space
				while (*p && (*p == ' ' || *p == '\t'))
					p++;
				
				// Attempt derefence to class
				MoType = INFO_GetTypeByName(p);
				
				// Invalid?
				if (MoType < 0 || MoType >= NUMMOBJTYPES)
					continue;
				
				// Add to end of cube list
				Z_ResizeArray((void**)&g_BossSpitList, sizeof(*g_BossSpitList), g_NumBossSpitList, g_NumBossSpitList + 1);
				Z_ChangeTag(g_BossSpitList, PU_WLDKRMOD);
				
				// Put here
				g_BossSpitList[g_NumBossSpitList].Chance = Val;
				g_BossSpitList[g_NumBossSpitList].Type = MoType;
				
				// Increment
				g_NumBossSpitList++;
			}
			
			// Close stream
			WL_StreamClose(Stream);
		}
	}
	
	/* Always success! */
	return true;
#undef BUFSIZE
}

/* P_ExtraSpecialStuff() -- Extra special stuff to load */
void P_ExtraSpecialStuff(void)
{
	/* Register extra special stuff */
	if (!WL_RegisterOCCB(PS_ExtraSpecialOCCB, WLDCO_EXTRASPECIALS))
		I_Error("P_ExtraSpecialStuff: Failed to register OCCB");	
}

