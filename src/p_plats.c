// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION:

#include "doomtype.h"
#include "p_local.h"
#include "d_think.h"
#include "p_spec.h"
#include "z_zone.h"
#include "p_demcmp.h"
#include "s_sound.h"










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
