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

typedef struct
{
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
	
	uint32_t WeaponID;							// Unique Weapon ID
	uint32_t Marker;							// Marker for RMOD
	uint64_t SimNext;							// Simulated next state
	char HoldSprite[5];							// Sprite to remember
	char* Function;								// Function Name
	uint32_t SpriteID;							// Frame Sprite ID
	uint32_t DehackEdID;						// Dehacked ID
} state_t;

#define S_NULL 0

extern state_t** states;
extern size_t NUMSTATES;

typedef int32_t mobjtype_t;
extern mobjtype_t NUMMOBJTYPES;

#define NUMINFORXFIELDS 4		// Prevents unwanted magic

typedef struct
{
	int doomednum;
	int spawnstate;
	int spawnhealth;
	int seestate;
	int seesound;
	int reactiontime;
	int attacksound;
	int painstate;
	int painchance;
	int painsound;
	int meleestate;
	int missilestate;
	int crashstate;				// from heretic/hexen
	int deathstate;
	int xdeathstate;
	int deathsound;
	int speed;
	int radius;
	int height;
	int mass;
	int damage;
	int activesound;
	int flags;
	int raisestate;
	int flags2;					// from heretic/hexen
	
	// RMOD Extended Support
	uint32_t RXFlags[NUMINFORXFIELDS];			// ReMooD Extended Flags
	int32_t RFastSpeed;							// Speed when -fast
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
	uint32_t RDehackEdID;						// DeHackEd ID
} mobjinfo_t;

extern mobjinfo_t** mobjinfo;

/*** HELPFUL FUNCTIONS ***/

mobjtype_t INFO_GetTypeByName(const char* const a_Name);
spritenum_t INFO_SpriteNumByName(const char* const a_Name);
actionf_t INFO_FunctionPtrByName(const char* const a_Name);
int INFO_PriorityByName(const char* const a_Name);
uint32_t INFO_TransparencyByName(const char* const a_Name);

/*** HELPFUL MACROS ***/
// Yuck! TODO: Make these real functions

// __REMOOD_BLOODTIMECONST -- use cv_bloodtime.value instead here
#define __REMOOD_BLOODTIMECONST 0x1

// __REMOOD_GETBLOODKIND -- or "LegacyOldDoomBlood"
#define __REMOOD_GETBLOODKIND (P_EXGSGetValue(PEXGSBID_COUSEOLDBLOOD) ? ("LegacyOldDoomBlood") : ("DoomBlood"))

// __REMOOD_GETSPEEDMO -- Get speed of mobj, note that getting the flag from the
// mobj is intentional. Why? So in -fast you could make certain monsters fast
// and not others with the same type. You could have a slowdown type weapon that
// when used with fast monsters negates the fast speed?
#define __REMOOD_GETSPEEDMO(mo) ((((mo)->RXFlags[0] & MFREXA_ENABLEFASTSPEED) && cv_fastmonsters.value) ? (mo)->info->RFastSpeed : (mo)->info->speed)

#endif

