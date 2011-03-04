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
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2011 GhostlyDeath (ghostlydeath@gmail.com)
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

#ifdef __GNUG__
#pragma interface
#endif

extern consvar_t cv_fragsweaponfalling;
extern consvar_t cv_infiniteammo;

// Boris hack : preferred weapons order
void VerifFavoritWeapon(player_t * player);

int FindBestWeapon(player_t * player);

boolean P_GivePower(player_t *, int);
void P_CheckFragLimit(player_t * p);

void P_KillMobj(mobj_t * target, mobj_t * inflictor, mobj_t * source);
boolean P_GiveBody(player_t * player, int num);

//added:28-02-98: boooring handling of thing(s) on top of thing(s)
/* BUGGY CODE
void P_CheckSupportThings (mobj_t* mobj);
void P_MoveSupportThings (mobj_t* mobj, fixed_t xmove,
                                        fixed_t ymove,
                                        fixed_t zmove);
void P_LinkFloorThing(mobj_t* mobj);
void P_UnlinkFloorThing(mobj_t* mobj);
*/

typedef struct CustomTouch_s
{
	int SpriteNum;
	int ThingNum;
	
	// Misc
	char* PickupString;
	int Flags;
	int Health[2];
	int Armor[2];
	int ArmorType;
	int Keys[NUMCARDS];
	int Ammo[NUMAMMO];
	int Weapons[NUMWEAPONS];
	
} CustomTouch_t;

extern CustomTouch_t* NewTouchThings;
extern size_t NumTouchThings;

#endif
