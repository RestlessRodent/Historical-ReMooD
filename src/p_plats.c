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
//      Plats (i.e. elevator platforms) code, raising/lowering.

//#include "doomdef.h"
//#include "doomstat.h"
//#include "p_local.h"
//#include "s_sound.h"
//#include "r_state.h"
//#include "z_zone.h"
//#include "m_random.h"
//#include "p_demcmp.h"

//SoM: 3/7/2000: Use boom's limitless format.
platlist_t* activeplats;

//
// Move a plat up and down
//
void T_PlatRaise(plat_t* plat)
{
	result_e res;
	
	switch (plat->status)
	{
		case up:
			res = T_MovePlane(plat->sector, plat->speed, plat->high, plat->crush, 0, 1);
			
			if (plat->type == raiseAndChange || plat->type == raiseToNearestAndChange)
			{
				if (!(leveltime % (8)))
					S_StartSound((mobj_t*)&plat->sector->soundorg, sfx_stnmov);
			}
			
			if (res == crushed && (!plat->crush))
			{
				plat->count = plat->wait;
				plat->status = down;
				S_StartSound((mobj_t*)&plat->sector->soundorg, sfx_pstart);
			}
			else
			{
				if (res == pastdest)
				{
					//SoM: 3/7/2000: Moved this little baby over.
					// if not an instant toggle type, wait, make plat stop sound
					if (plat->type != toggleUpDn)
					{
						plat->count = plat->wait;
						plat->status = waiting;
						S_StartSound((mobj_t*)&plat->sector->soundorg, sfx_pstop);
					}
					else		// else go into stasis awaiting next toggle activation
					{
						plat->oldstatus = plat->status;	//jff 3/14/98 after action wait
						plat->status = in_stasis;	//for reactivation of toggle
					}
					
					switch (plat->type)
					{
						case blazeDWUS:
						case downWaitUpStay:
						case raiseAndChange:
						case raiseToNearestAndChange:
						case genLift:
							P_RemoveActivePlat(plat);	//SoM: 3/7/2000: Much cleaner boom code.
						default:
							break;
					}
				}
			}
			break;
			
		case down:
			res = T_MovePlane(plat->sector, plat->speed, plat->low, false, 0, -1);
			
			if (res == pastdest)
			{
				//SoM: 3/7/2000: if not an instant toggle, start waiting, make plat stop sound
				if (plat->type != toggleUpDn)
				{
					plat->count = plat->wait;
					plat->status = waiting;
					S_StartSound((mobj_t*)&plat->sector->soundorg, sfx_pstop);
				}
				else			//SoM: 3/7/2000: instant toggles go into stasis awaiting next activation
				{
					plat->oldstatus = plat->status;
					plat->status = in_stasis;
				}
				
				//jff 1/26/98 remove the plat if it bounced so it can be tried again
				//only affects plats that raise and bounce
				
				if (P_XGSVal(PGS_COBOOMSUPPORT))
				{
					switch (plat->type)
					{
						case raiseAndChange:
						case raiseToNearestAndChange:
							P_RemoveActivePlat(plat);
						default:
							break;
					}
				}
			}
			
			break;
			
		case waiting:
			if (!--plat->count)
			{
				if (plat->sector->floorheight == plat->low)
					plat->status = up;
				else
					plat->status = down;
				S_StartSound((mobj_t*)&plat->sector->soundorg, sfx_pstart);
			}
		case in_stasis:
			break;
	}
}

//SoM: 3/7/2000: Use boom limit removal
void P_ActivateInStasis(int tag)
{
	platlist_t* pl;
	
	for (pl = activeplats; pl; pl = pl->next)
	{
		plat_t* plat = pl->plat;
		
		if (plat->tag == tag && plat->status == in_stasis)
		{
			if (plat->type == toggleUpDn)
				plat->status = plat->oldstatus == up ? down : up;
			else
				plat->status = plat->oldstatus;
			plat->thinker.function.acp1 = (actionf_p1) T_PlatRaise;
		}
	}
}


//SoM: 3/7/2000: No more limits!
void P_AddActivePlat(plat_t* plat)
{
	platlist_t* list = Z_Malloc(sizeof *list, PU_STATIC, NULL);
	
	list->plat = plat;
	plat->list = list;
	if ((list->next = activeplats))
		list->next->prev = &list->next;
	list->prev = &activeplats;
	activeplats = list;
}

//SoM: 3/7/2000: No more limits!
void P_RemoveActivePlat(plat_t* plat)
{
	platlist_t* list = plat->list;
	
	plat->sector->floordata = NULL;	//jff 2/23/98 multiple thinkers
	P_RemoveThinker(&plat->thinker);
	if ((*list->prev = list->next))
		list->next->prev = list->prev;
	Z_Free(list);
}

//SoM: 3/7/2000: Removes all active plats.
void P_RemoveAllActivePlats(void)
{
	while (activeplats)
	{
		platlist_t* next = activeplats->next;
		
		Z_Free(activeplats);
		activeplats = next;
	}
}
