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
// DESCRIPTION: Thing frame/state LUT,
//              generated by multigen utilitiy.
//              This one is the original DOOM version, preserved.

#ifndef __INFO__
#define __INFO__

// Needed for action function pointer handling.
#include "d_think.h"
#include "doomtype.h"
#include "d_rmod.h"
#include "z_zone.h"

#define __REMOOD_USEFLATTERSTATES

#if defined(__REMOOD_USEFLATTERSTATES)
	#define __REMOOD_REFSTATE(x) (&(x))
#else
	#define __REMOOD_REFSTATE(x) ((x))
#endif

typedef int32_t spritenum_t;
typedef int32_t statenum_t;

/* StatePriorities_t -- Viewing priority for state */
typedef enum StatePriorities_e
{
	STP_NULL = 0,				// Nothing
	STP_DEFAULT = 127,			// Default (Unset)
	STP_WEAPON = 112,			// Weapons
	STP_AMMO = 90,				// Ammo on the ground
	STP_WEPFLASH = 36,			// Weapon flashes
	STP_EFFECTS = 48,			// Effects (blood, fog, puffs, etc.)
	STP_MONSTERS = 96,			// Monsters that are alive
	STP_CORPSES = 32,			// Dead things
	STP_PLAYERS = 116,			// Other players
	STP_HEALTH = 100,			// Powerups that give health
	STP_COOKIES = 80,			// Health givers (but super small)
	STP_MISSIONCRITICAL = 128,	// Mission critical objects
	STP_POWERUPS = 70,			// Non health giving powerups
	STP_DECORATIONS = 50,		// Stuff meant to get in your way
	STP_PROJECTILES = 101,		// Stuff flying though the air
} StatePriorities_t;

/* INFO_StateArgsParm_t -- State arguments */
typedef uint8_t INFO_StateArgsNum_t;
typedef struct INFO_StateArgsParm_s
{
	char* String;								// String Argument
	int32_t IntVal;								// Numberic Value
} INFO_StateArgsParm_t;

typedef struct
{
	statenum_t StateNum;						// State number
	spritenum_t sprite;
	int32_t frame;				//faB: we use the upper 16bits for translucency
	//     and other shade effects
	int32_t tics;
	// void       (*action) ();
	actionf_t action;
	statenum_t nextstate;
	
	uint8_t Priority;			// View priority of the state
	
	// GhostlyDeath <March 5, 2012> -- To RMOD Deprecation
	int32_t RMODFastTics;						// Tics when -fast
	int32_t ExtraStateFlags;					// Custom flags
	
	uint32_t FrameID;							// Frame Marker
	uint32_t ObjectID;							// Unique Weapon ID
	uint32_t Marker;							// Marker for RMOD
	uint64_t SimNext;							// Simulated next state
	char HoldSprite[5];							// Sprite to remember
	char* Function;								// Function Name
	uint32_t SpriteID;							// Frame Sprite ID
	uint32_t DehackEdID;						// Dehacked ID
	uint8_t IOSG;								// State Group
	
	INFO_StateArgsNum_t ArgC;					// Argument Count
	INFO_StateArgsParm_t* ArgV;					// Function Arguments
} state_t;

#define S_NULL 0

extern size_t NUMSPRITES;
extern char** sprnames;
extern void** g_SprTouchSpecials;				// Sprite touch special markers

extern state_t** states;
extern size_t NUMSTATES;

typedef int32_t mobjtype_t;
extern mobjtype_t NUMMOBJTYPES;

#define NUMINFORXFIELDS 4		// Prevents unwanted magic

typedef enum INFO_ObjectStateGroup_e
{
	/* Map Object States */
	IOSG_SPAWN = 0,
	IOSG_ACTIVE,
	IOSG_PAIN,
	IOSG_MELEEATTACK,
	IOSG_RANGEDATTACK,
	IOSG_CRASH,
	IOSG_DEATH,
	IOSG_GIB,
	IOSG_RAISE,
	IOSG_PLAYERRUN,
	IOSG_PLAYERMELEE,
	IOSG_PLAYERRANGED,
	IOSG_VILEHEAL,
	IOSG_LESSBLOODA,
	IOSG_LESSBLOODB,
	IOSG_BRAINEXPLODE,
	IOSG_MELEEPUFF,
	
	NUMINFOOBJECTSTATEGROUPS,
	
	/* Weapon States */
	PWSG_UP = 0,
	PWSG_DOWN,
	PWSG_READY,
	PWSG_ATTACK,
	PWSG_HOLDATTACK,
	PWSG_FLASH,
	
	NUMPWEAPONSTATEGROUPS,
} INFO_ObjectStateGroup_t;

/* INFO_BotObjMetric_t -- Bot metric */
typedef enum INFO_BotObjMetric_e
{
	INFOBM_DEFAULT,								// Guess!
	INFOBM_DOOMPLAYER,							// A player
	INFOBM_WEAKMONSTER,							// A weak monster (zombies, imps, etc.)
	INFOBM_NORMALMONSTER,						// A normal monster (not weak, not strong)
	INFOBM_STRONGMONSTER,						// A strong monster (lots of hp, powerful weapons)
	INFOBM_ARCHVILE,							// Arch-Vile (try to stay out of sight)
	INFOBM_PROJECTILE,							// This should be dodged (if we don't own it)
	INFOBM_BARREL,								// Explosive barrel (use when needed)
	INFOBM_LIGHTARMOR,							// Item
	INFOBM_HEAVYARMOR,							// Item
	INFOBM_LIGHTHEALTH,							// Item
	INFOBM_HEAVYHEALTH,							// Item
	INFOBM_KEYCARD,								// Key
	INFOBM_AMMO,								// Key
	INFOBM_WEAPON,								// Key
	
	INFOBM_WEAPONMELEE,							// Melee Weapon
	INFOBM_WEAPONMIDRANGE,						// Middle Range Weapon
	INFOBM_WEAPONLAYDOWN,						// Lay down surpressing fire
	INFOBM_SPRAYPLASMA,							// Spray plasma all over
	INFOBM_WEAPONBFG,							// BFG
	INFOBM_WEAPONSSGDANCE,						// SuperShotgun
	
	NUMINFOBOTOBJMETRICS
} INFO_BotObjMetric_t;

typedef INFO_ObjectStateGroup_t P_WeaponStateGroup_t;

typedef struct
{
	int32_t EdNum[6];
	statenum_t spawnstate;
	int spawnhealth;
	statenum_t seestate;
	int seesound;
	int reactiontime;
	int attacksound;
	statenum_t painstate;
	int painchance;
	int painsound;
	statenum_t meleestate;
	statenum_t missilestate;
	statenum_t crashstate;				// from heretic/hexen
	statenum_t deathstate;
	statenum_t xdeathstate;
	int deathsound;
	fixed_t speed;
	fixed_t radius;
	fixed_t Height;								// Standard Height
	fixed_t OldHeight;							// < 1.32 Height
	int mass;
	int damage;
	int activesound;
	int flags;
	statenum_t raisestate;
	int flags2;					// from heretic/hexen
	
	// RMOD Extended Support
	uint32_t RXFlags[NUMINFORXFIELDS];			// ReMooD Extended Flags
	fixed_t RFastSpeed;							// Speed when -fast
	statenum_t RPlayerRunState;					// State for moving player
	statenum_t RPlayerMeleeAttackState;			// S_PLAY_ATK2
	statenum_t RPlayerRangedAttackState;		// S_PLAY_ATK1
	statenum_t RVileHealState;					// Heal state for Arch-Vile
	int RMissileDist[2];						// Min/Max missile distances [P_CheckMissileRange]
	int RCapMissileDist;						// Distance cap [P_CheckMissileRange]
	statenum_t RLessBlood[2];					// Less blood to spew? (0 = 9-12, 1 = < 9) [P_SpawnBlood]
	char* RDropClass;							// Class to "drop" when dead
	mobjtype_t RBaseFamily;						// Base object family [PIT_CheckThing]
	statenum_t RBrainExplodeState;				// State for exploding rockets [A_BrainScream]
	char* RBrainExplodeThing;					// Thing to explode on a dying brain [A_BrainScream]
	statenum_t RMeleePuffState;					// State for meleerange puff [P_SpawnPuff]
	
	// Class Names
	char* RClassName;							// Class Name
	char* RMTName;								// MT Name
	char* RNiceName;							// Nice Name
	uint32_t RDehackEdID;						// DeHackEd ID
	uint32_t RefStates[NUMINFOOBJECTSTATEGROUPS];	// State references
	char* RFamilyClass;							// Family Class
	
	// Object ID
	uint32_t ObjectID;							// ID Of Object (unique, kinda)
	
	// Sounds
	char* RSeeSound;
	char* RAttackSound;
	char* RPainSound;
	char* RDeathSound;
	char* RActiveSound;
	
	// Extra Stuff
	fixed_t RBounceFactor;						// Bounce Multiplier
	fixed_t RAirGravity;						// Gravity while in air
	fixed_t RWaterGravity;						// Gravity while in water
	char* RMissileSplat;						// Splat when missile hits wall
	char* RBloodSplat;							// Splat when bleeding
	char* RBloodSpewClass;						// Bleeds this class
	char* RGenericMissile;						// Generic Monster Missile Attack
	
	INFO_BotObjMetric_t RBotMetric;				// Metric for bot
	char* RSNiceName;							// Short Nice Name
} mobjinfo_t;

extern mobjinfo_t** mobjinfo;

/*** RMOD ***/

bool_t INFO_RMODH_MapObjects(Z_Table_t* const a_Table, const WL_WADFile_t* const a_WAD, const D_RMODPrivates_t a_ID, D_RMODPrivate_t* const a_Private);
bool_t INFO_RMODO_MapObjects(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD, const D_RMODPrivates_t a_ID);

// RMOD Helpers

typedef statenum_t* (*INFO_RMODStateForNameFunc_t)(void* const a_Input, const char* const a_Name, INFO_ObjectStateGroup_t* const IOSG, uint32_t** const a_RefState, uint32_t*** const a_LRefs, size_t** a_NumLRefs);

/* INFO_RMODStateHelper_t -- RMOD State Helper */
typedef struct INFO_RMODStateHelper_s
{
	INFO_RMODStateForNameFunc_t StateForName;	// State for Name function

	void* InputPtr;								// Input Pointer
	uint32_t ObjectID;							// Object ID
	uint32_t* StateSplasher;					// Splasher for states
	INFO_ObjectStateGroup_t StateGroup;			// Current State Group
	uint32_t* StateValueP;						// Reference State
	
#if defined(__REMOOD_USEFLATTERSTATES)
	state_t** StatesRef;						// Reference to stored states
#else
	state_t*** StatesRef;						// Reference to stored states
#endif
	size_t* NumStatesRef;						// Reference to number of states
	size_t* MaxStatesRef;						// Reference to max states
	size_t BaseStateNum;						// Base state to start from
} INFO_RMODStateHelper_t;

bool_t INFO_RMODStateHandlers(Z_Table_t* const a_Sub, void* const a_Data);
void INFO_StateNormalize(const size_t a_MergeBase, const size_t a_MergeCount);

/*** HELPFUL FUNCTIONS ***/

mobjtype_t INFO_GetTypeByName(const char* const a_Name);
spritenum_t INFO_SpriteNumByName(const char* const a_Name, bool_t a_Create);
actionf_t INFO_FunctionPtrByName(const char* const a_Name);
int INFO_PriorityByName(const char* const a_Name);
uint32_t INFO_TransparencyByName(const char* const a_Name);
INFO_BotObjMetric_t INFO_BotMetricByName(const char* const a_Name);

/*** HELPFUL MACROS ***/
// Yuck! TODO: Make these real functions

// __REMOOD_BLOODTIMECONST -- use P_XGSVal(PGS_GAMEBLOODTIME) instead here
#define __REMOOD_BLOODTIMECONST 0x1

// __REMOOD_GETBLOODKIND -- or "LegacyOldDoomBlood"
#define __REMOOD_GETBLOODKIND (P_XGSVal(PGS_COUSEOLDBLOOD) ? ("LegacyOldDoomBlood") : ("DoomBlood"))

// __REMOOD_GETSPEEDMO -- Get speed of mobj, note that getting the flag from the
// mobj is intentional. Why? So in -fast you could make certain monsters fast
// and not others with the same type. You could have a slowdown type weapon that
// when used with fast monsters negates the fast speed?
#define __REMOOD_GETSPEEDMO(mo) ((((((mo)->RXFlags[0] & MFREXA_ENABLEFASTSPEED) && P_XGSVal(PGS_MONFASTMONSTERS)) ? (mo)->info->RFastSpeed : (mo)->info->speed)) >> (((mo)->flags & MF_MISSILE) ? 0 : 16))

// __REMOOD_GETHEIGHT -- Get Height of info object
#define __REMOOD_GETHEIGHT(inf) (((inf)->OldHeight && P_XGSVal(PGS_COOLDTHINGHEIGHTS)) ? ((inf)->OldHeight) : ((inf)->Height))

#endif

