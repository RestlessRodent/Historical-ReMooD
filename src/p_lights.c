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
//      Handle Sector base lighting effects.
//      Muzzle flash?

#include "p_spec.h"
#include "z_zone.h"
#include "p_demcmp.h"

//#include "doomdef.h"
//#include "p_local.h"
//#include "r_state.h"
//#include "z_zone.h"
//#include "m_random.h"
//#include "p_demcmp.h"

// =========================================================================
//                           FIRELIGHT FLICKER
// =========================================================================

//
// T_FireFlicker
//
void T_FireFlicker(fireflicker_t* flick)
{
	int amount;
	
	if (--flick->count)
		return;
		
	amount = (P_Random() & 3) * 16;
	
	if (flick->sector->lightlevel - amount < flick->minlight)
		flick->sector->lightlevel = flick->minlight;
	else
		flick->sector->lightlevel = flick->maxlight - amount;
		
	flick->count = 4;
}

//
// P_SpawnFireFlicker
//
void P_SpawnFireFlicker(sector_t* sector)
{
	fireflicker_t* flick;
	
	// Note that we are resetting sector attributes.
	// Nothing special about it during gameplay.
	sector->special &= ~31;		//SoM: Clear non-generalized sector type
	
	flick = Z_Malloc(sizeof(*flick), PU_LEVSPEC, 0);
	
	P_AddThinker(&flick->thinker, PTT_FIREFLICKER);
	
	flick->thinker.function.acp1 = (actionf_p1) T_FireFlicker;
	flick->sector = sector;
	flick->maxlight = sector->lightlevel;
	flick->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel) + 16;
	flick->count = 4;
}

//
// BROKEN LIGHT FLASHING
//

//
// T_LightFlash
// Do flashing lights.
//
void T_LightFlash(lightflash_t* flash)
{
	if (--flash->count)
		return;
		
	if (flash->sector->lightlevel == flash->maxlight)
	{
		flash->sector->lightlevel = flash->minlight;
		flash->count = (P_Random() & flash->mintime) + 1;
	}
	else
	{
		flash->sector->lightlevel = flash->maxlight;
		flash->count = (P_Random() & flash->maxtime) + 1;
	}
	
}

//
// P_SpawnLightFlash
// After the map has been loaded, scan each sector
// for specials that spawn thinkers
//
void P_SpawnLightFlash(sector_t* sector)
{
	lightflash_t* flash;
	
	// nothing special about it during gameplay
	sector->special &= ~31;		//SoM: 3/7/2000: Clear non-generalized type
	
	flash = Z_Malloc(sizeof(*flash), PU_LEVSPEC, 0);
	
	P_AddThinker(&flash->thinker, PTT_LIGHTFLASH);
	
	flash->thinker.function.acp1 = (actionf_p1) T_LightFlash;
	flash->sector = sector;
	flash->maxlight = sector->lightlevel;
	
	flash->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);
	flash->maxtime = 64;
	flash->mintime = 7;
	flash->count = (P_Random() & flash->maxtime) + 1;
}

//
// STROBE LIGHT FLASHING
//

//
// T_StrobeFlash
//
void T_StrobeFlash(strobe_t* flash)
{
	if (--flash->count)
		return;
		
	if (flash->sector->lightlevel == flash->minlight)
	{
		flash->sector->lightlevel = flash->maxlight;
		flash->count = flash->brighttime;
	}
	else
	{
		flash->sector->lightlevel = flash->minlight;
		flash->count = flash->darktime;
	}
	
}

//
// P_SpawnStrobeFlash
// After the map has been loaded, scan each sector
// for specials that spawn thinkers
//
void P_SpawnStrobeFlash(sector_t* sector, int fastOrSlow, int inSync)
{
	strobe_t* flash;
	
	flash = Z_Malloc(sizeof(*flash), PU_LEVSPEC, 0);
	
	P_AddThinker(&flash->thinker, PTT_STROBEFLASH);
	
	flash->sector = sector;
	flash->darktime = fastOrSlow;
	flash->brighttime = STROBEBRIGHT;
	flash->thinker.function.acp1 = (actionf_p1) T_StrobeFlash;
	flash->maxlight = sector->lightlevel;
	flash->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);
	
	if (flash->minlight == flash->maxlight)
		flash->minlight = 0;
		
	// nothing special about it during gameplay
	sector->special &= ~31;		//SoM: 3/7/2000: Clear non-generalized sector type
	
	if (!inSync)
		flash->count = (P_Random() & 7) + 1;
	else
		flash->count = 1;
}

//
// Start strobing lights (usually from a trigger)
//
int EV_StartLightStrobing(line_t* line)
{
	int secnum;
	sector_t* sec;
	
	secnum = -1;
	while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
	{
		sec = &sectors[secnum];
		if (P_SectorActive(lighting_special, sec))	//SoM: 3/7/2000: New way to check thinker
			continue;
			
		P_SpawnStrobeFlash(sec, SLOWDARK, 0);
	}
	return 1;
}

//
// TURN LINE'S TAG LIGHTS OFF
//
int EV_TurnTagLightsOff(line_t* line)
{
	int i;
	int j;
	int min;
	sector_t* sector;
	sector_t* tsec;
	line_t* templine;
	
	sector = sectors;
	
	for (j = 0; j < numsectors; j++, sector++)
	{
		if (sector->tag == line->tag)
		{
			min = sector->lightlevel;
			for (i = 0; i < sector->linecount; i++)
			{
				templine = sector->lines[i];
				tsec = getNextSector(templine, sector);
				if (!tsec)
					continue;
				if (tsec->lightlevel < min)
					min = tsec->lightlevel;
			}
			sector->lightlevel = min;
		}
	}
	return 1;
}

//
// Spawn glowing light
//

void T_Glow(glow_t* g)
{
	switch (g->direction)
	{
		case -1:
			// DOWN
			g->sector->lightlevel -= GLOWSPEED;
			if (g->sector->lightlevel <= g->minlight)
			{
				g->sector->lightlevel += GLOWSPEED;
				g->direction = 1;
			}
			break;
			
		case 1:
			// UP
			g->sector->lightlevel += GLOWSPEED;
			if (g->sector->lightlevel >= g->maxlight)
			{
				g->sector->lightlevel -= GLOWSPEED;
				g->direction = -1;
			}
			break;
	}
}

void P_SpawnGlowingLight(sector_t* sector)
{
	glow_t* g;
	
	g = Z_Malloc(sizeof(*g), PU_LEVSPEC, 0);
	
	P_AddThinker(&g->thinker, PTT_GLOW);
	
	g->sector = sector;
	g->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);
	g->maxlight = sector->lightlevel;
	g->thinker.function.acp1 = (actionf_p1) T_Glow;
	g->direction = -1;
	
	sector->special &= ~31;		//SoM: 3/7/2000: Reset only non-generic types.
}

// P_FadeLight()
//
// Fade all the lights in sectors with a particular tag to a new value
//
void P_FadeLight(int tag, int destvalue, int speed)
{
	int i;
	lightlevel_t* ll;
	
	// search all sectors for ones with tag
	for (i = -1; (i = P_FindSectorFromTag(tag, i)) >= 0;)
	{
		sector_t* sector = &sectors[i];
		
		sector->lightingdata = sector;	// just set it to something
		
		ll = Z_Malloc(sizeof(*ll), PU_LEVSPEC, 0);
		ll->thinker.function.acp1 = (actionf_p1) T_LightFade;
		
		P_AddThinker(&ll->thinker, PTT_LIGHTFADE);	// add thinker
		
		ll->sector = sector;
		ll->destlevel = destvalue;
		ll->speed = speed;
	}
}

// T_LightFade()
//
// Just fade the light level in a sector to a new level
//

void T_LightFade(lightlevel_t* ll)
{
	if (ll->sector->lightlevel < ll->destlevel)
	{
		// increase the lightlevel
		if (ll->sector->lightlevel + ll->speed >= ll->destlevel)
		{
			// stop changing light level
			ll->sector->lightlevel = ll->destlevel;	// set to dest lightlevel
			
			ll->sector->lightingdata = NULL;	// clear lightingdata
			P_RemoveThinker(&ll->thinker);	// remove thinker
		}
		else
		{
			ll->sector->lightlevel += ll->speed;	// move lightlevel
		}
	}
	else
	{
		// decrease lightlevel
		if (ll->sector->lightlevel - ll->speed <= ll->destlevel)
		{
			// stop changing light level
			ll->sector->lightlevel = ll->destlevel;	// set to dest lightlevel
			
			ll->sector->lightingdata = NULL;	// clear lightingdata
			P_RemoveThinker(&ll->thinker);	// remove thinker
		}
		else
		{
			ll->sector->lightlevel -= ll->speed;	// move lightlevel
		}
	}
}
