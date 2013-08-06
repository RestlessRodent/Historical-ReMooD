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
// DESCRIPTION: player data structures

#ifndef __D_PLAYER__
#define __D_PLAYER__

#include "d_ticcmd.h"	// TODO FIXME: Only used once

/* Define D_Prof_t */
#if !defined(__REMOOD_DPROFTDEFINED)
	#define __REMOOD_DPROFTDEFINED
	typedef struct D_Prof_s D_Prof_t;
#endif

/* Define mobj_t */
#if !defined(__REMOOD_MOBJT_DEFINED)
	typedef struct mobj_s mobj_t;
	#define __REMOOD_MOBJT_DEFINED
#endif

/* Define player_t */
#if !defined(__REMOOD_PLAYERT_DEFINED)
	typedef struct player_s player_t;
	#define __REMOOD_PLAYERT_DEFINED
#endif

/* Define PI_wepid_t */
#if !defined(__REMOOD_PIWEPIDT_DEFINED)
	typedef int32_t PI_wepid_t;
	#define __REMOOD_PIWEPIDT_DEFINED
#endif

/* Define PI_state_t */
#if !defined(__REMOOD_PISTATE_DEFINED)
	typedef struct PI_state_s PI_state_t;
	#define __REMOOD_PISTATE_DEFINED
#endif

/* Define PI_wep_t */
#if !defined(__REMOOD_PIWEP_DEFINED)
	typedef struct PI_wep_s PI_wep_t;
	#define __REMOOD_PIWEP_DEFINED
#endif

/* Define mapthing_t */
#if !defined(__REMOOD_MAPTHINGT_DEFINED)
	typedef struct mapthing_s mapthing_t;
	#define __REMOOD_MAPTHINGT_DEFINED
#endif

// The player data structure depends on a number
// of other structs: items (internal inventory),
// animation states (closely tied to the sprites
// used to represent them, unfortunately).
//#include "d_items.h"
//#include "p_pspr.h"

// In addition, the player is just a special
// case of the generic moving object/actor.
//#include "p_mobj.h"

// Finally, for odd reasons, the player input
// is buffered within the player data struct,
// as commands per game tick.
//#include "d_ticcmd.h"

//#include "d_prof.h"
//#include "d_netcmd.h"

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
} P_PlayerState_t;

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

//#include "d_items.h"

/* camera_t -- Chase cam */
typedef struct camera_s
{
	bool_t chase;
	angle_t aiming;
	int32_t fixedcolormap;
	
	//SoM: Things used by FS cameras.
	fixed_t viewheight;
	angle_t startangle;
	
	mobj_t* mo;
} camera_t;

struct SN_Port_s;

// Power up artifacts.
typedef enum
{
	pw_invulnerability,
	pw_strength,
	pw_invisibility,
	pw_ironfeet,
	pw_allmap,
	pw_infrared,
	
	NUMPOWERS
} powertype_t;

//
// Overlay psprites are scaled shapes
// drawn directly on the view screen,
// coordinates are given for a 320*200 view screen.
//
typedef enum
{
	ps_weapon,
	ps_flash,
	NUMPSPRITES
} psprnum_t;

typedef struct
{
	PI_state_t* state;				// a NULL state means not active
	int32_t tics;
	fixed_t sx;
	fixed_t sy;
} pspdef_t;

#define NUMINVENTORYSLOTS  14
#define MAXARTECONT        16
typedef struct
{
	uint8_t type;
	uint8_t count;
} inventory_t;

// ========================================================================
//                          PLAYER STRUCTURE
// ========================================================================

struct player_s
{
	mobj_t* mo;
	// added 1-6-98: for movement prediction
	P_PlayerState_t playerstate;
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
	fixed_t FlatBob;							// Flat bobbing value
	
	//added:16-02-98: mouse aiming, where the guy is looking at!
	//                 It is updated with cmd->aiming.
	angle_t aiming;
	
	// This is only used between levels,
	// mo->health is used during levels.
	int32_t health;
	int32_t armorpoints;
	// Armor type is 0-2.
	uint8_t armortype;
	
	// Power ups. invinc and invis are tic counters.
	int32_t powers[NUMPOWERS];
	bool_t backpack;
	
	// Frags, kills of other players.
	uint16_t addfrags;			// player have killed a player but is gone
	uint16_t frags[MAXPLAYERS];
	PI_wepid_t readyweapon;
	
	// Is wp_nochange if not changing.
	PI_wepid_t pendingweapon;
	PI_wepid_t DeadWeapon;					// Weapon held when dead
	PI_wepid_t* FavoriteWeapons;				// Favorite Weapons
	
	bool_t* weaponowned;
	int32_t* ammo;
	int32_t* maxammo;
	bool_t originalweaponswitch;
	//added:28-02-98:
	bool_t autoaim_toggle;
	
	// True if button down last tic.
	bool_t attackdown;
	bool_t usedown;
	bool_t jumpdown;			//added:19-03-98:dont jump like a monkey!
	
	// Bit flags, for cheats and debug.
	// See cheat_t, above.
	int32_t cheats;
	
	// Refired shots are less accurate.
	int32_t refire;
	
	// For intermission stats.
	int32_t killcount;
	int32_t itemcount;
	int32_t secretcount;
	
	// Hint messages.
	char* message;
	
	// For screen flashing (red or bright).
	int32_t damagecount;
	int32_t bonuscount;
	uint8_t PalChoice;							// Palette to display for player
	
	// Who did damage (NULL for floors/ceilings).
	mobj_t* attacker;
	int32_t specialsector;			//lava/slime/water...
	
	// So gun flashes light up areas.
	int32_t extralight;
	
	// Current PLAYPAL, ???
	//  can be set to REDCOLORMAP for pain, etc.
	int32_t fixedcolormap;
	
	// Player skin colorshift,
	//  0-3 for which color to draw player.
	// adding 6-2-98 comment : unused by doom2 1.9 now is used
	int32_t skincolor;
	int32_t VTeamColor;							// Virtual Team Color
	
	// added 2/8/98
	int32_t skin;
	
	// Overlay view sprites (gun, etc).
	pspdef_t psprites[NUMPSPRITES];
	
	// True if secret level has been done.
	bool_t didsecret;
	
	// heretic
	int32_t chickenTics;			// player is a chicken if > 0
	int32_t chickenPeck;			// chicken peck countdown
	mobj_t* rain1;				// active rain maker 1
	mobj_t* rain2;				// active rain maker 2
	int32_t flamecount;
	fixed_t flyheight;
	inventory_t inventory[NUMINVENTORYSLOTS];
	int32_t inventorySlotNum;
	
	int32_t inv_ptr;
	int32_t st_curpos;				// position of inventory scroll
	int32_t st_inventoryTics;		// when >0 show inventory in status bar
	
	PI_wep_t** weaponinfo;	// can be changed when use level2 weapons (heretic)
	
	// Sound Info
	int32_t flushdelay;
	
	// GhostlyDeath <October 23, 2010> -- Player inflicted momentum
	fixed_t MoveMom;
	fixed_t TargetViewZ;
	
	// GhostlyDeath <September 16, 2011> -- "Effort" based bobbing
	fixed_t FakeMom[3];
	
	// GhostlyDeath <March 6, 2012> -- Per-player (chase) camera
	camera_t camera;
	fixed_t CamDist, CamHeight, CamSpeed;		// Camera Properties
	bool_t ChaseCam;							// Chase Camera
	
	// GhostlyDeath <October 5, 2012> -- Title-screen Demo Stuff
	mobj_t* LastBFGBall;						// Player's last BFG Ball
	mobj_t* Attackee;							// Thing being attacked
	
	/*** EXTENDED STUFF ***/
	D_Prof_t* ProfileEx;						// Extended Profiles
	struct SN_Port_s* Port;					// Port
	
	// Health
	int32_t MaxHealth[2];						// Max Health
	int32_t MaxArmor[2];						// Max Armor
	
	// Counter-Operative
	bool_t CounterOpPlayer;						// On the monster's side
	
	// Kill Totals
	int32_t TotalFrags;							// Total kills
	int32_t TotalDeaths;						// Total deaths
	uint32_t FraggerID;							// Fragger ID
	
	// Load prevention
	tic_t SuicideDelay;							// Prevent suicides until after
	
	// Key Cards
	uint32_t KeyCards[2];						// Cards and Skulls
	uint8_t KeyFlash[2][32];					// Flashing of keys
	
	// Ping
	uint16_t Ping;								// Player's Ping
	uint32_t BackupButtons;
};

//
// INTERMISSION
// Structure passed e.g. to WI_Start(wb)
//
typedef struct
{
	bool_t in;					// whether the player is in game
	
	// Player stats, kills, collected items etc.
	int32_t skills;
	int32_t sitems;
	int32_t ssecret;
	int32_t stime;
	uint16_t frags[MAXPLAYERS];	// added 17-1-98 more than 4 players
	int32_t score;					// current score on entry, modified on return
	// BP: unused for now but don't forget....
	uint16_t addfrags;
	
} wbplayerstruct_t;

struct P_LevelInfoEx_s;

typedef struct
{
	int32_t epsd;					// episode # (0-2)
	
	// if true, splash the secret level
	bool_t didsecret;
	
	// previous and next levels, origin 0
	int32_t last;
	int32_t next;
	
	int32_t maxkills;
	int32_t maxitems;
	int32_t maxsecret;
	int32_t maxfrags;
	
	// the par time
	int32_t partime;
	
	// index of this player in game
	int32_t pnum;
	
	wbplayerstruct_t plyr[MAXPLAYERS];
	
	struct P_LevelInfoEx_s* NextInfo;			// Info for next level
	
} wbstartstruct_t;

/**************
*** GLOBALS ***
**************/

extern char player_names[MAXPLAYERS][MAXPLAYERNAME];
extern char team_names[MAXPLAYERS][MAXPLAYERNAME * 2];
extern player_t players[MAXPLAYERS];
extern bool_t playeringame[MAXPLAYERS];

#define MAX_DM_STARTS   64
extern mapthing_t* deathmatchstarts[MAX_DM_STARTS];
extern int numdmstarts;
extern mapthing_t* g_TeamStarts[MAXSKINCOLORS][MAXPLAYERS];
extern mapthing_t* playerstarts[MAXPLAYERS];

#define   BODYQUESIZE     MAXPLAYERS
extern mobj_t* bodyque[BODYQUESIZE];
extern int bodyqueslot;

#endif

