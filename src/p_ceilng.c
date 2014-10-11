// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION:

#include "p_spec.h"
#include "s_sound.h"
#include "g_state.h"
#include "z_zone.h"
#include "p_local.h"







// ==========================================================================
//                              CEILINGS
// ==========================================================================

// SoM: 3/6/2000: the list of ceilings moving currently, including crushers
ceilinglist_t* activeceilings;

//ceiling_t*      activeceilings[MAXCEILINGS];
int ceilmovesound = sfx_stnmov;

//
// T_MoveCeiling
//

void T_MoveCeiling(ceiling_t* ceiling)
{
	result_e res;
	
	switch (ceiling->direction)
	{
		case 0:
			// IN STASIS
			break;
		case 1:
			// UP
			res = T_MovePlane(ceiling->sector, ceiling->speed, ceiling->topheight, false, 1, ceiling->direction);
			
			if (!(leveltime % (8)))
			{
				switch (ceiling->type)
				{
					case silentCrushAndRaise:
					case genSilentCrusher:
						break;
					default:
						S_StartSound((mobj_t*)&ceiling->sector->soundorg, ceilmovesound);
						// ?
						break;
				}
			}
			
			if (res == pastdest)
			{
				switch (ceiling->type)
				{
				
					case raiseToHighest:
					case genCeiling:	//SoM: 3/6/2000
						P_RemoveActiveCeiling(ceiling);
						break;
						
						// SoM: 3/6/2000: movers with texture change, change the texture then get removed
					case genCeilingChgT:
					case genCeilingChg0:
						ceiling->sector->special = ceiling->newspecial;
						ceiling->sector->oldspecial = ceiling->oldspecial;
					case genCeilingChg:
						ceiling->sector->ceilingpic = ceiling->texture;
						P_RemoveActiveCeiling(ceiling);
						break;
						
					case silentCrushAndRaise:
						//S_StartSound((mobj_t*)&ceiling->sector->soundorg, sfx_pstop);
					case fastCrushAndRaise:
					case genCrusher:	// SoM: 3/6/2000
					case genSilentCrusher:
					case crushAndRaise:
						ceiling->direction = -1;
						
						// GhostlyDeath <May 6, 2012> -- Crushers always make stop sound?
						S_StartSound((mobj_t*)&ceiling->sector->soundorg, sfx_pstop);
						break;
						
					default:
						break;
				}
			}
			break;
			
		case -1:
			// DOWN
			res = T_MovePlane(ceiling->sector, ceiling->speed, ceiling->bottomheight, ceiling->crush, 1, ceiling->direction);
			
			if (!(leveltime % (8)))
			{
				switch (ceiling->type)
				{
					case silentCrushAndRaise:
					case genSilentCrusher:
						break;
					default:
						S_StartSound((mobj_t*)&ceiling->sector->soundorg, ceilmovesound);
				}
			}
			
			if (res == pastdest)
			{
				switch (ceiling->type)	//SoM: 3/6/2000: Use boom code
				{
					case genSilentCrusher:
					case genCrusher:
						if (ceiling->oldspeed < CEILSPEED * 3)
							ceiling->speed = ceiling->oldspeed;
						ceiling->direction = 1;
						
						// GhostlyDeath <May 6, 2012> -- Crushers always make stop sound?
						S_StartSound((mobj_t*)&ceiling->sector->soundorg, sfx_pstop);
						break;
						
						// make platform stop at bottom of all crusher strokes
						// except generalized ones, reset speed, start back up
					case silentCrushAndRaise:
						S_StartSound((mobj_t*)&ceiling->sector->soundorg, sfx_pstop);
					case crushAndRaise:
						ceiling->speed = CEILSPEED;
					case fastCrushAndRaise:
						ceiling->direction = 1;
						break;
						
						// in the case of ceiling mover/changer, change the texture
						// then remove the active ceiling
					case genCeilingChgT:
					case genCeilingChg0:
						ceiling->sector->special = ceiling->newspecial;
						//jff add to fix bug in special transfers from changes
						ceiling->sector->oldspecial = ceiling->oldspecial;
					case genCeilingChg:
						ceiling->sector->ceilingpic = ceiling->texture;
						P_RemoveActiveCeiling(ceiling);
						break;
						
						// all other case, just remove the active ceiling
					case lowerAndCrush:
					case lowerToFloor:
					case lowerToLowest:
					case lowerToMaxFloor:
					case genCeiling:
						P_RemoveActiveCeiling(ceiling);
						break;
						
					default:
						break;
				}
			}
			else				// ( res != pastdest )
			{
				if (res == crushed)
				{
					switch (ceiling->type)
					{
							//SoM: 2/6/2000
							// slow down slow crushers on obstacle
						case genCrusher:
						case genSilentCrusher:
							if (ceiling->oldspeed < CEILSPEED * 3)
								ceiling->speed = CEILSPEED / 8;
							break;
							
						case silentCrushAndRaise:
						case crushAndRaise:
						case lowerAndCrush:
							ceiling->speed = CEILSPEED / 8;
							break;
							
						default:
							break;
					}
				}
			}
			break;
	}
}

//
// Add an active ceiling
//
//SoM: 3/6/2000: Take advantage of the new Boom method for active ceilings.
void P_AddActiveCeiling(ceiling_t* ceiling)
{
	ceilinglist_t* list = Z_Malloc(sizeof *list, PU_STATIC, NULL);
	
	list->ceiling = ceiling;
	ceiling->list = list;
	if ((list->next = activeceilings))
		list->next->prev = &list->next;
	list->prev = &activeceilings;
	activeceilings = list;
}

//
// Remove a ceiling's thinker
//
// SoM: 3/6/2000 :Use improved Boom code.
void P_RemoveActiveCeiling(ceiling_t* ceiling)
{
	ceilinglist_t* list = ceiling->list;
	
	ceiling->sector->ceilingdata = NULL;	//jff 2/22/98
	P_RemoveThinker(&ceiling->thinker);
	if ((*list->prev = list->next))
		list->next->prev = list->prev;
	Z_Free(list);
}

//
// Restart a ceiling that's in-stasis
//
//SoM: 3/6/2000: Use improved boom code
int P_ActivateInStasisCeiling(line_t* line)
{
	ceilinglist_t* cl;
	int rtn = 0;
	
	for (cl = activeceilings; cl; cl = cl->next)
	{
		ceiling_t* ceiling = cl->ceiling;
		
		if (ceiling->tag == line->tag && ceiling->direction == 0)
		{
			ceiling->direction = ceiling->olddirection;
			ceiling->thinker.function.acp1 = (actionf_p1) T_MoveCeiling;
			rtn = 1;
		}
	}
	return rtn;
}

// SoM: 3/6/2000: Extra, boom only function.
//
// P_RemoveAllActiveCeilings()
//
// Removes all ceilings from the active ceiling list
//
// Passed nothing, returns nothing
//
void P_RemoveAllActiveCeilings(void)
{
	while (activeceilings)
	{
		ceilinglist_t* next = activeceilings->next;
		
		Z_Free(activeceilings);
		activeceilings = next;
	}
}
