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

#ifndef __P_INTER__
#define __P_INTER__

// Boris hack : preferred weapons order
void VerifFavoritWeapon(player_t* player);

int FindBestWeapon(player_t* player);

bool_t P_GivePower(player_t*, int);
void P_CheckFragLimit(player_t* p);

void P_KillMobj(mobj_t* target, mobj_t* inflictor, mobj_t* source);
bool_t P_GiveBody(player_t* player, int num);

//added:28-02-98: boooring handling of thing(s) on top of thing(s)

/* BUGGY CODE
void P_CheckSupportThings (mobj_t* mobj);
void P_MoveSupportThings (mobj_t* mobj, fixed_t xmove,
                                        fixed_t ymove,
                                        fixed_t zmove);
void P_LinkFloorThing(mobj_t* mobj);
void P_UnlinkFloorThing(mobj_t* mobj);
*/

#endif
