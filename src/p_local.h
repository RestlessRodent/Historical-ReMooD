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
// DESCRIPTION: Play functions, animation, global header.

#ifndef __P_LOCAL__
#define __P_LOCAL__

#include "command.h"
#include "d_player.h"
#include "d_think.h"
#include "m_fixed.h"
#include "m_bbox.h"
#include "p_tick.h"
#include "r_defs.h"
#include "p_maputl.h"

#define FLOATSPEED              (FRACUNIT*4)

// added by Boris : for dehacked patches, replaced #define by int
extern int MAXHEALTH;			// 100

#define VIEWHEIGHT               41
#define VIEWHEIGHTS             "41"

// default viewheight is changeable at console
extern consvar_t cv_viewheight;	// p_mobj.c

// mapblocks are used to check movement
// against lines and things
#define MAPBLOCKUNITS   128
#define MAPBLOCKSIZE    (MAPBLOCKUNITS*FRACUNIT)
#define MAPBLOCKSHIFT   (FRACBITS+7)
#define MAPBMASK        (MAPBLOCKSIZE-1)
#define MAPBTOFRAC      (MAPBLOCKSHIFT-FRACBITS)

// player radius used only in am_map.c
#define PLAYERRADIUS    (16*FRACUNIT)

// MAXRADIUS is for precalculated sector block boxes
// the spider demon is larger,
// but we do not have any moving sectors nearby
#define MAXRADIUS       (32*FRACUNIT)

#define MAXMOVE         (30*FRACUNIT)

//added:26-02-98: max Z move up or down without jumping
//      above this, a heigth difference is considered as a 'dropoff'
#define MAXSTEPMOVE     (24*FRACUNIT)

#define USERANGE        (64*FRACUNIT)
#define MELEERANGE      (64*FRACUNIT)
#define MISSILERANGE    (32*64*FRACUNIT)

// follow a player exlusively for 3 seconds
#define BASETHRESHOLD   100

//#define AIMINGTOSLOPE(aiming)   finetangent[(2048+(aiming>>ANGLETOFINESHIFT)) & FINEMASK]
#define AIMINGTOSLOPE(aiming)   finesine[(aiming>>ANGLETOFINESHIFT) & FINEMASK]

//26-07-98: p_mobj.c
extern consvar_t cv_gravity;
extern consvar_t cv_classicblood;

// p_enemy.c
extern consvar_t cv_classicmeleerange;
extern consvar_t cv_classicmonsterlogic;

void P_ClearRecursiveSound(void);
void P_RemoveRecursiveSound(mobj_t* const a_Mo);

//
// P_TICK
//

// both the head and tail of the thinker list
extern thinker_t thinkercap;

void P_InitThinkers(void);
void P_AddThinker(thinker_t* thinker, const P_ThinkerType_t a_Type);
void P_RemoveThinker(thinker_t* thinker);

//
// P_PSPR
//
void P_SetupPsprites(player_t* curplayer);
void P_MovePsprites(player_t* curplayer);
void P_DropWeapon(player_t* player);

//
// P_USER
//

extern consvar_t cv_cam_dist;
extern consvar_t cv_cam_height;
extern consvar_t cv_cam_speed;

void P_ResetCamera(player_t* player);
void P_PlayerThink(player_t* player);

// client prediction
void CL_ResetSpiritPosition(mobj_t* mobj);
void P_MoveSpirit(player_t* p, ticcmd_t* cmd, int realtics);

//
// P_MOBJ
//
#define ONFLOORZ        INT_MIN
#define ONCEILINGZ      INT_MAX

// Time interval for item respawning.
// WARING MUST be a power of 2
#define ITEMQUESIZE     128

extern mapthing_t* itemrespawnque[ITEMQUESIZE];
extern tic_t itemrespawntime[ITEMQUESIZE];
extern int iquehead;
extern int iquetail;

void P_RespawnSpecials(void);
void P_RespawnWeapons(void);

mobj_t* P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type);

void P_RemoveMobj(mobj_t* th);
bool_t P_SetMobjState(mobj_t* mobj, statenum_t state);
void P_MobjThinker(mobj_t* mobj);

//spawn splash at surface of water in sector where the mobj resides
void P_SpawnSplash(mobj_t* mo, fixed_t z);

//Fab: when fried in in lava/slime, spawn some smoke
void P_SpawnSmoke(fixed_t x, fixed_t y, fixed_t z);

void P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z);
void P_SpawnBlood(fixed_t x, fixed_t y, fixed_t z, int damage, mobj_t* const a_BleedThing);
void P_SpawnBloodSplats(fixed_t x, fixed_t y, fixed_t z, int damage, fixed_t momx, fixed_t momy, mobj_t* const a_BleedThing);
mobj_t* P_SpawnMissile(mobj_t* source, mobj_t* dest, mobjtype_t type);

mobj_t* P_SPMAngle(mobj_t* source, mobjtype_t type, angle_t angle);

#define P_SpawnPlayerMissile(s,t) P_SPMAngle(s,t,s->angle)

//
// P_ENEMY
//

/* P_BossSpitEntry_t -- Boss cube spitting entry */
typedef struct P_BossSpitEntry_s
{
	int32_t Chance;								// < P_Random() chance to spawn
	mobjtype_t Type;							// Type to spawn
} P_BossSpitEntry_t;

extern P_BossSpitEntry_t* g_BossSpitList;
extern size_t g_NumBossSpitList;

// when pushing a line
//#define MAXSPECIALCROSS 16

extern int spechit_max;
extern int* spechit;			//SoM: 3/15/2000: Limit removal
extern int numspechit;

void P_NoiseAlert(mobj_t* target, mobj_t* emmiter);

void P_UnsetThingPosition(mobj_t* thing);
void P_SetThingPosition(mobj_t* thing);

// init braintagets position
void P_InitBrainTarget();

//
// P_MAP
//

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
extern bool_t floatok;
extern fixed_t tmfloorz;
extern fixed_t tmceilingz;
extern fixed_t tmsectorceilingz;	//added:28-02-98: p_spawnmobj
extern mobj_t* tmfloorthing;

extern line_t* ceilingline;
extern line_t* blockingline;
extern msecnode_t* sector_list;

/* P_CheckPositionFlags_t -- Position check flags */
typedef enum P_CheckPositionFlags_e
{
	PCPF_ACCEPTALL					= 0x0001,	// Accept all things
	PCPF_FORSPOTCHECK				= 0x0002,	// For player spot checking
} P_CheckPositionFlags_t;

bool_t P_CheckPositionDetermine(mobj_t* thing, fixed_t x, fixed_t y, uint32_t a_Flags);
bool_t P_CheckPosition(mobj_t* thing, fixed_t x, fixed_t y, uint32_t a_Flags);
bool_t P_CheckPosRadius(fixed_t x, fixed_t y, fixed_t Radius);
bool_t P_TryMove(mobj_t* thing, fixed_t x, fixed_t y, bool_t allowdropoff, fixed_t* const a_OutX, fixed_t* const a_OutY);
bool_t P_TeleportMove(mobj_t* thing, fixed_t x, fixed_t y);
void P_SlideMove(mobj_t* mo);
bool_t P_CheckSight(mobj_t* t1, mobj_t* t2);
bool_t P_CheckSight2(mobj_t* t1, mobj_t* t2, fixed_t x, fixed_t y, fixed_t z);	//added by AC for predicting
void P_UseLines(player_t* player);
bool_t P_CheckSightLine(const fixed_t x1, const fixed_t y1, const fixed_t x2, const fixed_t y2);

bool_t P_CheckSector(sector_t* sector, bool_t crunch);
bool_t P_ChangeSector(sector_t* sector, bool_t crunch);

void P_DelSeclist(msecnode_t*);
void P_CreateSecNodeList(mobj_t*, fixed_t, fixed_t);
int P_GetMoveFactor(mobj_t* mo);
void P_Initsecnode(void);

extern mobj_t* linetarget;		// who got hit (or NULL)

extern fixed_t attackrange;

/* P_LineAtkFlags_t -- Line Attack Flags */
typedef enum P_LineAtkFlags_e
{
	PLAF_THRUMOBJ				= 0x00000001,	// Continue through map objects
} P_LineAtkFlags_t;

/* P_AimLineAtkArgs_t -- Aim line attack arguments */
typedef struct P_AimLineAtkArgs_s
{
	uint32_t Flags;								// Flags
} P_AimLineAtkArgs_t;

/* P_LineAtkArgs_t -- Line Attack arguments */
typedef struct P_LineAtkArgs_s
{
	uint32_t Flags;								// Flags
} P_LineAtkArgs_t;

fixed_t P_AimLineAttack(mobj_t* t1, angle_t angle, fixed_t distance, P_AimLineAtkArgs_t* const a_Args);
void P_LineAttack(mobj_t* t1, angle_t angle, fixed_t distance, fixed_t slope, int damage, P_LineAtkArgs_t* const a_Args);
void P_RadiusAttack(mobj_t* spot, mobj_t* source, int damage);

//
// P_SETUP
//
extern uint8_t* rejectmatrix;	// for fast sight rejection
extern size_t g_RJMSize;						// Size of reject matrix
extern size_t g_BMLSize;		// Block map lump size
extern long* blockmaplump;		// offsets in blockmap are from here
extern long* blockmap;			// Big blockmap SSNTails
extern int bmapwidth;
extern int bmapheight;			// in mapblocks
extern fixed_t bmaporgx;
extern fixed_t bmaporgy;		// origin of block map
extern mobj_t** blocklinks;		// for thing chains

//
// P_INTER
//

bool_t P_TouchSpecialThing(mobj_t* special, mobj_t* toucher);

bool_t P_DamageMobj(mobj_t* target, mobj_t* inflictor, mobj_t* source, int damage);

//
// P_SIGHT
//

// slopes to top and bottom of target
extern fixed_t topslope;
extern fixed_t bottomslope;

//
// P_SPEC
//
#include "p_spec.h"

//SoM: 3/6/2000: Added public "boomsupport variable"
extern int boomsupport;
extern int variable_friction;
extern int allow_pushers;

// heretic specific
extern int ceilmovesound;
extern int doorclosesound;

#define TELEFOGHEIGHT  (32*FRACUNIT)
extern mobjtype_t PuffType;

#define FOOTCLIPSIZE   (10*FRACUNIT)
#define HITDICE(a) ((1+(P_Random()&7))*a)

#define MAXCHICKENHEALTH 30

#define BLINKTHRESHOLD  (4*32)
#define WPNLEV2TICS     (40*TICRATE)
#define FLIGHTTICS      (60*TICRATE)

#define CHICKENTICS     (40*TICRATE)
#define FLOATRANDZ      (INT_MAX-1)

void P_RepositionMace(mobj_t* mo);
void P_ActivateBeak(player_t* player);
void P_DSparilTeleport(mobj_t* actor);
void P_InitMonsters(void);
bool_t P_LookForMonsters(mobj_t* actor);
int P_GetThingFloorType(mobj_t* thing);
mobj_t* P_CheckOnmobj(mobj_t* thing);
void P_AddMaceSpot(mapthing_t* mthing);
bool_t P_SightPathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
void P_HerePlayerInSpecialSector(player_t* player);
void P_UpdateBeak(player_t* player, pspdef_t* psp);
bool_t P_TestMobjLocation(mobj_t* mobj);
void P_PostChickenWeapon(player_t* player, weapontype_t weapon);
void P_SetPsprite(player_t* player, int position, statenum_t stnum);
bool_t P_Teleport(mobj_t* thing, fixed_t x, fixed_t y, angle_t angle);
bool_t P_CheckMissileSpawn(mobj_t* th);
void P_ThrustMobj(mobj_t* mo, angle_t angle, fixed_t move);
void P_Thrust(player_t* player, angle_t angle, fixed_t move);
void P_ExplodeMissile(mobj_t* mo);
void P_Massacre(void);
void P_AddBossSpot(fixed_t x, fixed_t y, angle_t angle);
bool_t P_ChickenMorphPlayer(player_t* player);

extern consvar_t cv_spawnmonsters;

bool_t P_WeaponIsUnlocked(const weapontype_t a_Weapon);
bool_t P_CanUseWeapon(player_t* const a_Player, const weapontype_t a_Weapon);

/**************************
*** RMOD TOUCH SPECIALS ***
**************************/

/*** CONSTANTS ***/

#define PMAXRTSFIELDSIZE					32	// Max size of field in touch specials

/* P_RMODTouchSpecialFlags_t -- Touch specials for flags */
typedef enum P_RMODTouchSpecialFlags_e
{
	PMTSF_KEEPNOTNEEDED		= UINT32_C(0x0001),	// Keep when not needed
	PMTSF_REMOVEALWAYS		= UINT32_C(0x0002),	// Remove always
	PMTSF_MONSTERCANGRAB	= UINT32_C(0x0004),	// Monster can grab item
	PMTSF_DEVALUE			= UINT32_C(0x0008),	// Allow devaluing
	PMTSF_CAPNORMSTAT		= UINT32_C(0x0010),	// Cap to normal stats
	PMTSF_CAPMAXSTAT		= UINT32_C(0x0020),	// Cap to max stats
	PMTSF_GREATERARMORCLASS	= UINT32_C(0x0040),	// Use when armor class is better
	PMTSF_SETBACKPACK		= UINT32_C(0x0080),	// Modify max ammo when !backpack
} P_RMODTouchSpecialFlags_t;

/*** STRUCTURES ***/

/* P_RMODTouchSpecial_t -- Special toucher for RMOD */
typedef struct P_RMODTouchSpecial_s
{
	/* General */
	char SpriteName[4];							// Name of sprite
	char* PickupSnd;							// Sound to play when picked up
	const char** PickupMsgRef;					// Message to print when picked up
	const char* PickupMsgFaked;					// Faked pickup message
	
	/* Modifiers */
	char* GiveWeapon;							// Weapon to give
	char* GiveAmmo;								// Ammo to give
	
	/* Actual */
	uint32_t Flags;								// Flags
	spritenum_t ActSpriteNum;					// Sprite number to match
	weapontype_t ActGiveWeapon;					// Actual weapon to give
	ammotype_t ActGiveAmmo;						// Actual ammo to give
	uint32_t ActSpriteID;						// Actual Sprite ID
	
	/* Health */
	int8_t ArmorClass;							// Armor Class
	int32_t ArmorAmount;						// Armor Amount
	int32_t HealthAmount;						// Health Amount
	
	/* Weapons and Ammo */
	int32_t AmmoMul;							// Ammo multiplier
	int32_t MaxAmmoMul;							// Max ammo multiplier
} P_RMODTouchSpecial_t;

/* P_RMODKey_t -- Key definition */
typedef struct P_RMODKey_s
{
	uint32_t IDNum;								// ID Number
	uint32_t BoomID;							// Boom ID
	bool_t IsSkull;								// Is Skull Key
} P_RMODKey_t;

/*** GLOBALS ***/

extern size_t g_RMODNumTouchSpecials;
extern P_RMODTouchSpecial_t** g_RMODTouchSpecials;

extern size_t g_RMODNumKeys;
extern P_RMODKey_t** g_RMODKeys;

P_RMODTouchSpecial_t* P_RMODTouchSpecialForSprite(const uint32_t a_SprNum);
P_RMODTouchSpecial_t* P_RMODTouchSpecialForCode(const uint32_t a_Code);

#endif							// __P_LOCAL__

