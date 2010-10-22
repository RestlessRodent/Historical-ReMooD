// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.sourceforge.net/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           Demyx                  (demyx@endgameftw.com)
//                    Dragan                 (poliee13@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2009 The ReMooD Team..
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
//      Archiving: SaveGame I/O.
//      Thinker, Ticker.

#include "doomstat.h"
#include "g_game.h"
#include "p_local.h"
#include "z_zone.h"
#include "t_script.h"

tic_t leveltime;

//
// THINKERS
// All thinkers should be allocated by Z_Malloc
// so they can be operated on uniformly.
// The actual structures will vary in size,
// but the first element must be thinker_t.
//

// Both the head and tail of the thinker list.
thinker_t thinkercap;

//
// P_InitThinkers
//
void P_InitThinkers(void)
{
	thinkercap.prev = thinkercap.next = &thinkercap;
}

//
// P_AddThinker
// Adds a new thinker at the end of the list.
//
void P_AddThinker(thinker_t * thinker)
{
	thinkercap.prev->next = thinker;
	thinker->next = &thinkercap;
	thinker->prev = thinkercap.prev;
	thinkercap.prev = thinker;
}

//
// P_RemoveThinker
// Deallocation is lazy -- it will not actually be freed
// until its thinking turn comes up.
//
void P_RemoveThinker(thinker_t * thinker)
{
	// FIXME: NOP.
	thinker->function.acv = (actionf_v) (-1);
}

//
// P_AllocateThinker
// Allocates memory and adds a new thinker at the end of the list.
//
void P_AllocateThinker(thinker_t * thinker)
{
}

//
// P_RunThinkers
//
void P_RunThinkers(void)
{
	thinker_t *currentthinker;

	currentthinker = thinkercap.next;
	while (currentthinker != &thinkercap)
	{
		if (currentthinker->function.acv == (actionf_v) (-1))
		{
			void *removeit;
			// time to remove it
			currentthinker->next->prev = currentthinker->prev;
			currentthinker->prev->next = currentthinker->next;
			removeit = currentthinker;
			currentthinker = currentthinker->next;
			Z_Free(removeit);
		}
		else
		{
			if (currentthinker->function.acp1)
				currentthinker->function.acp1(currentthinker);
			currentthinker = currentthinker->next;
		}
	}
}

//
// P_Ticker
//

void P_Ticker(void)
{
	int i;

	// run the tic
	if (paused || (!netgame && menuactive && !demoplayback))
		return;

	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
			P_PlayerThink(&players[i]);

	P_RunThinkers();
	P_UpdateSpecials();
	P_RespawnSpecials();
	P_AmbientSound();

	// for par times
	leveltime++;

#ifdef FRAGGLESCRIPT
	// SoM: Update FraggleScript...
	T_DelayedScripts();
#endif
}
