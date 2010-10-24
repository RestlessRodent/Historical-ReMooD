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
// DESCRIPTION: player data structures

#ifndef __D_PLAYER__
#define __D_PLAYER__

// The player data structure depends on a number
// of other structs: items (internal inventory),
// animation states (closely tied to the sprites
// used to represent them, unfortunately).
#include "d_items.h"
#include "p_pspr.h"

// In addition, the player is just a special
// case of the generic moving object/actor.
#include "p_mobj.h"

// Finally, for odd reasons, the player input
// is buffered within the player data struct,
// as commands per game tick.
#include "d_ticcmd.h"

#include "d_prof.h"

//
// Player states.
//
typedef enum
{
	// Playing or camping.
	PST_LIVE,
	// Dead on the ground, view follows killer.
	PST_DEAD,
	// Ready to restart/respawn???
	PST_REBORN
} playerstate_t;

//
// Player internal flags, for cheats and debug.
//
typedef enum
{
	// No clipping, walk through barriers.
	CF_NOCLIP = 1,
	// No damage, no health loss.
	CF_GODMODE = 2,
	// Not really a cheat, just a debug aid.
	CF_NOMOMENTUM = 4,

	//added:28-02-98: new cheats
	CF_FLYAROUND = 8,

	//added:28-02-98: NOT REALLY A CHEAT
	// Allow player avatar to walk in-air
	//  if trying to get over a small wall (hack for playability)
	CF_JUMPOVER = 16
} cheat_t;

#include "d_items.h"

// ========================================================================
//                          PLAYER STRUCTURE
// ========================================================================
typedef struct player_s
{
	ProfileInfo_t* profile;
	
	mobj_t *mo;
	// added 1-6-98: for movement prediction
	playerstate_t playerstate;
	ticcmd_t cmd;

	// Determine POV,
	//  including viewpoint bobbing during movement.
	// Focal origin above r.z
	fixed_t viewz;
	// Base height above floor for viewz.
	fixed_t viewheight;
	// Bob/squat speed.
	fixed_t deltaviewheight;
	// bounded/scaled total momentum.
	fixed_t bob;

	//added:16-02-98: mouse aiming, where the guy is looking at!
	//                 It is updated with cmd->aiming.
	angle_t aiming;

	// This is only used between levels,
	// mo->health is used during levels.
	int health;
	int armorpoints;
	// Armor type is 0-2.
	byte armortype;

	// Power ups. invinc and invis are tic counters.
	int powers[NUMPOWERS];
	byte cards;					// bit field see declration of card_t
	boolean backpack;

	// Frags, kills of other players.
	USHORT addfrags;			// player have killed a player but is gone
	USHORT frags[MAXPLAYERS];
	weapontype_t readyweapon;

	// Is wp_nochange if not changing.
	weapontype_t pendingweapon;

	boolean weaponowned[NUMWEAPONS];
	int ammo[NUMAMMO];
	int maxammo[NUMAMMO];
	// added by Boris : preferred weapons order stuff
	char favoritweapon[NUMWEAPONS];
	boolean originalweaponswitch;
	//added:28-02-98:
	boolean autoaim_toggle;

	// True if button down last tic.
	boolean attackdown;
	boolean usedown;
	boolean jumpdown;			//added:19-03-98:dont jump like a monkey!

	// Bit flags, for cheats and debug.
	// See cheat_t, above.
	int cheats;

	// Refired shots are less accurate.
	int refire;

	// For intermission stats.
	int killcount;
	int itemcount;
	int secretcount;

	// Hint messages.
	char *message;

	// For screen flashing (red or bright).
	int damagecount;
	int bonuscount;

	// Who did damage (NULL for floors/ceilings).
	mobj_t *attacker;
	int specialsector;			//lava/slime/water...

	// So gun flashes light up areas.
	int extralight;

	// Current PLAYPAL, ???
	//  can be set to REDCOLORMAP for pain, etc.
	int fixedcolormap;

	// Player skin colorshift,
	//  0-3 for which color to draw player.
	// adding 6-2-98 comment : unused by doom2 1.9 now is used
	int skincolor;

	// added 2/8/98
	int skin;

	// Overlay view sprites (gun, etc).
	pspdef_t psprites[NUMPSPRITES];

	// True if secret level has been done.
	boolean didsecret;
	
	// heretic
	int chickenTics;			// player is a chicken if > 0
	int chickenPeck;			// chicken peck countdown
	mobj_t *rain1;				// active rain maker 1
	mobj_t *rain2;				// active rain maker 2
	int flamecount;
	int flyheight;
	inventory_t inventory[NUMINVENTORYSLOTS];
	int inventorySlotNum;

	int inv_ptr;
	int st_curpos;				// position of inventory scroll
	int st_inventoryTics;		// when >0 show inventory in status bar

	weaponinfo_t *weaponinfo;	// can be changed when use level2 weapons (heretic)

	// Sound Info
	int flushdelay;
	
	// GhostlyDeath <October 23, 2010> -- Player inflicted momentum
	fixed_t MoveMom;
	fixed_t TargetViewZ;

} player_t;

//
// INTERMISSION
// Structure passed e.g. to WI_Start(wb)
//
typedef struct
{
	boolean in;					// whether the player is in game

	// Player stats, kills, collected items etc.
	int skills;
	int sitems;
	int ssecret;
	int stime;
	USHORT frags[MAXPLAYERS];	// added 17-1-98 more than 4 players
	int score;					// current score on entry, modified on return
	// BP: unused for now but don't forget....
	USHORT addfrags;

} wbplayerstruct_t;

typedef struct
{
	int epsd;					// episode # (0-2)

	// if true, splash the secret level
	boolean didsecret;

	// previous and next levels, origin 0
	int last;
	int next;

	int maxkills;
	int maxitems;
	int maxsecret;
	int maxfrags;

	// the par time
	int partime;

	// index of this player in game
	int pnum;

	wbplayerstruct_t plyr[MAXPLAYERS];

} wbstartstruct_t;

void A_TicWeapon(player_t * player, pspdef_t * psp);

#endif

