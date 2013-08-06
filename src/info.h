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
// DESCRIPTION: Thing frame/state LUT,
//              generated by multigen utilitiy.
//              This one is the original DOOM version, preserved.

#ifndef __INFO__
#define __INFO__

#include "doomtype.h"
#include "d_think.h"




/* Define PI_stateid_t */
#if !defined(__REMOOD_STSPIDS_DEFINED)
	typedef int32_t PI_spriteid_t;
	typedef int32_t PI_stateid_t;
	#define __REMOOD_STSPIDS_DEFINED
#endif

/* StatePriorities_t -- Viewing priority for state */
typedef enum StatePriorities_e
{
	STP_NULL = 0,				// Nothing
	STP_DEFAULT = 127,			// Default (Unset)
	STP_WEAPON = 112,			// Weapons
	STP_AMMO = 90,				// Ammo on the ground
	STP_WEPFLASH = 36,			// Weapon flashes
	STP_EFFECTS = 8,			// Effects (blood, fog, puffs, etc.)
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

/* PI_sargv_t -- State arguments */
typedef uint8_t PI_sargc_t;
typedef struct PI_sargv_s
{
	char* String;								// String Argument
	int32_t IntVal;								// Numberic Value
} PI_sargv_t;

/* PI_stateorigin_t -- Origin of state */
typedef struct PI_stateorigin_s
{
	uint8_t Type;								// Where it came from
	uint32_t ID;								// ID of origin
} PI_stateorigin_t;

/* Define PI_state_t */
#if !defined(__REMOOD_PISTATE_DEFINED)
	typedef struct PI_state_s PI_state_t;
	#define __REMOOD_PISTATE_DEFINED
#endif

struct PI_state_s
{
	PI_stateid_t StateNum;						// State number
	PI_spriteid_t sprite;
	int32_t frame;				//faB: we use the upper 16bits for translucency
	//     and other shade effects
	int32_t tics;
	// void       (*action) ();
	actionf_t action;
	PI_stateid_t nextstate;
	
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
	PI_stateid_t DehackEdID[6];					// Dehacked ID
	uint8_t IOSG;								// State Group
	
	PI_stateorigin_t Origin;					// Where state came from
	
	PI_sargc_t ArgC;							// Argument Count
	PI_sargv_t* ArgV;							// Function Arguments
};

#define S_NULL 0

extern size_t NUMSPRITES;
extern char** sprnames;
extern void** g_SprTouchSpecials;				// Sprite touch special markers

extern PI_state_t** states;
extern PI_stateid_t NUMSTATES;

/* Define PI_mobjid_t */
#if !defined(__REMOOD_PIMOID_DEFINED)
	typedef int32_t PI_mobjid_t;
	#define __REMOOD_PIMOID_DEFINED
#endif

extern PI_mobjid_t NUMMOBJTYPES;

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
	
	NUMSTATEGROUPS = NUMINFOOBJECTSTATEGROUPS + NUMPWEAPONSTATEGROUPS,
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

/* Define PI_mobj_t */
#if !defined(__REMOOD_PIMOBJT_DEFINED)
	typedef struct PI_mobj_s PI_mobj_t;
	#define __REMOOD_PIMOBJT_DEFINED
#endif

struct PI_mobj_s
{
	int32_t EdNum[6];
	PI_stateid_t spawnstate;
	int32_t spawnhealth;
	PI_stateid_t seestate;
	//int32_t seesound;
	int32_t reactiontime;
	//int32_t attacksound;
	PI_stateid_t painstate;
	int32_t painchance;
	//int32_t painsound;
	PI_stateid_t meleestate;
	PI_stateid_t missilestate;
	PI_stateid_t crashstate;				// from heretic/hexen
	PI_stateid_t deathstate;
	PI_stateid_t xdeathstate;
	//int32_t deathsound;
	fixed_t speed;
	fixed_t radius;
	fixed_t Height;								// Standard Height
	fixed_t OldHeight;							// < 1.32 Height
	int32_t mass;
	int32_t damage;
	//int32_t activesound;
	uint32_t flags;
	PI_stateid_t raisestate;
	uint32_t flags2;					// from heretic/hexen
	
	// RMOD Extended Support
	uint32_t RXFlags[NUMINFORXFIELDS];			// ReMooD Extended Flags
	fixed_t RFastSpeed;							// Speed when -fast
	PI_stateid_t RPlayerRunState;					// State for moving player
	PI_stateid_t RPlayerMeleeAttackState;			// S_PLAY_ATK2
	PI_stateid_t RPlayerRangedAttackState;		// S_PLAY_ATK1
	PI_stateid_t RVileHealState;					// Heal state for Arch-Vile
	fixed_t RMissileDist[2];						// Min/Max missile distances [P_CheckMissileRange]
	fixed_t RCapMissileDist;						// Distance cap [P_CheckMissileRange]
	PI_stateid_t RLessBlood[2];					// Less blood to spew? (0 = 9-12, 1 = < 9) [P_SpawnBlood]
	char* RDropClass;							// Class to "drop" when dead
	PI_mobjid_t RBaseFamily;						// Base object family [PIT_CheckThing]
	PI_stateid_t RBrainExplodeState;				// State for exploding rockets [A_BrainScream]
	char* RBrainExplodeThing;					// Thing to explode on a dying brain [A_BrainScream]
	PI_stateid_t RMeleePuffState;					// State for meleerange puff [P_SpawnPuff]
	
	// Class Names
	char* RClassName;							// Class Name
	char* RMTName;								// MT Name
	char* RNiceName;							// Nice Name
	PI_stateid_t RDehackEdID[6];					// DeHackEd ID
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
	
	INFO_BotObjMetric_t BotMetric;				// Metric for bot
	char* RSNiceName;							// Short Nice Name
	uint32_t RQuickHash[2];						// Hash
};

extern PI_mobj_t** mobjinfo;

/* AMMO */

typedef int32_t PI_ammoid_t;

#define wp_nochange				-1
#define am_noammo				-1
#define am_all					-2

/* AmmoFlags_t -- Ammunition Flags */
typedef enum AmmoFlags_e
{
	AF_INFINITE						= 0x0001,	// Infinite Ammo
} AmmoFlags_t;

/* PI_ammo_t -- Hold ammo information */
typedef struct ammoinfo_s
{
	char* ClassName;							// Class name
	int32_t ClipAmmo;							// Ammo in clip
	int32_t MaxAmmo;							// Max ammo held
	uint32_t Flags;								// Ammo Flags
	int32_t StartingAmmo;						// Starting Ammo
} PI_ammo_t;

extern PI_ammo_t** ammoinfo;
extern PI_ammoid_t NUMAMMO;

/* WEAPON */

/* Define PI_wepid_t */
#if !defined(__REMOOD_PIWEPIDT_DEFINED)
	typedef int32_t PI_wepid_t;
	#define __REMOOD_PIWEPIDT_DEFINED
#endif

/* WeaponFlags_t -- Flags for weapons */
typedef enum WeaponFlags_e
{
	// Game bases
	WF_ISDOOM					= 0x00000001,	// Weapon appears in Doom
	WF_ISHERETIC				= 0x00000002,	// Weapon appears in Heretic
	WF_ISHEXEN					= 0x00000004,	// Weapon appears in Hexen
	WF_ISSTRIFE					= 0x00000008,	// Weapon appears in Strife
	
	// Visibility Status
	WF_NOTSHAREWARE				= 0x00000010,	// Does not appear in shareware
	WF_INCOMMERCIAL				= 0x00000020,	// Appears in commercial mode
	WF_INREGISTERED				= 0x00000040,	// Appears in registered mode
	
	// Other
	WF_BERSERKTOGGLE			= 0x00000080,	// Only accept least weapon when berserk
	WF_SWITCHFROMNOAMMO			= 0x00000100,	// When player has 0 ammo, switch away!
	WF_STARTINGWEAPON			= 0x00000200,	// Start with this gun
	WF_NOAUTOFIRE				= 0x00000400,	// No automatic fire
	WF_NOTHRUST					= 0x00000800,	// No thrusting the enemy
	
	WF_INEXTENDED				= 0x00001000,	// Appears in extended mode
	WF_NOBLEEDTARGET			= 0x00002000,	// Do not bleed target
	WF_SUPERWEAPON				= 0x00004000,	// Is a Super Weapon
	WF_NONOISEALERT				= 0x00008000,	// Does not alert to noise
	WF_ISMELEE					= 0x00010000,	// Melee Weapon
} WeaponFlags_t;

/* Define PI_wep_t */
#if !defined(__REMOOD_PIWEP_DEFINED)
	typedef struct PI_wep_s PI_wep_t;
	#define __REMOOD_PIWEP_DEFINED
#endif

// Weapon info: sprite frames, ammunition use.
struct PI_wep_s
{
	PI_ammoid_t ammo;
	int32_t ammopershoot;
	PI_stateid_t upstate;
	PI_stateid_t downstate;
	PI_stateid_t readystate;
	PI_stateid_t atkstate;
	PI_stateid_t holdatkstate;
	PI_stateid_t flashstate;
	
	// ReMooD Extended
	int32_t DEHId;								// DeHackEd ID
	char* DropWeaponClass;						// Thing to "drop" when a player dies
	char* NiceName;								// Name of weapon (obit)
	char* ClassName;							// Weapon class Name
	int32_t SwitchOrder;						// Weapon switch order
	int32_t SlotNum;								// Weapon slot number
	uint32_t WeaponFlags;						// Flags for weapon
	int32_t GetAmmo;							// Amount of ammo to pick up for this weapon
	int32_t NoAmmoOrder;						// No Ammo Order
	fixed_t PSpriteSY;							// PSprite offset
	char* SBOGraphic;							// SBO Graphic
	char* AmmoClass;							// Name of ammo to use
	char* BringUpSound;							// Sound to play when brung up
	char* IdleNoise;							// Noise when idling (chainsaw)
	uint32_t WeaponID;							// Unique Weapon ID
	uint32_t RefStates[NUMPWEAPONSTATEGROUPS];	// Reference States
	char* ReplacePuffType;						// Replacement puff type (rather than default)
	char* ReplaceFireSound;						// Replacement Fire Sound
	char* GenericProjectile;					// Generic Projectile
	char* TracerSplat;							// Splat when tracing
	INFO_BotObjMetric_t BotMetric;				// Bot Metric
	
	// State References
	PI_stateid_t* FlashStates;					// Weapon flash states
	size_t NumFlashStates;						// Number of flash states
};

extern PI_wep_t** wpnlev1info;
extern PI_wep_t** wpnlev2info;
extern size_t NUMWEAPONS;

/* KEYS */

#define INFO_BLUEKEYCOMPAT UINT32_C(0x1)
#define INFO_YELLOWKEYCOMPAT UINT32_C(0x2)
#define INFO_REDKEYCOMPAT UINT32_C(0x4)

#define INFO_ALLKEYCOMPAT (INFO_BLUEKEYCOMPAT | INFO_YELLOWKEYCOMPAT | INFO_REDKEYCOMPAT)

typedef int32_t PI_keyid_t;

/* PI_key_t -- Key definition */
typedef struct P_RMODKey_s
{
	char* ClassName;
	char* ColorName;
	char* ImageName;
	
	uint32_t Bit;
	uint32_t BitNum;
	uint8_t Group;
	
	uint32_t RGB[3];								// Key Color
} PI_key_t;

extern size_t g_RMODNumKeys;
extern PI_key_t** g_RMODKeys;

/* TOUCHERS */

typedef int32_t PI_touchid_t;

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
	PMTSF_KEEPINMULTI		= UINT32_C(0x0100),	// Keep in multiplayer mode
} P_RMODTouchSpecialFlags_t;

/* PI_touch_t -- Special toucher for RMOD */
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
	char* GiveKey;								// Key to give
	
	/* Actual */
	uint32_t Flags;								// Flags
	PI_spriteid_t ActSpriteNum;					// Sprite number to match
	PI_wepid_t ActGiveWeapon;					// Actual weapon to give
	PI_ammoid_t ActGiveAmmo;						// Actual ammo to give
	uint32_t ActSpriteID;						// Actual Sprite ID
	PI_keyid_t ActGiveKey;
	
	/* Health */
	int32_t ArmorClass;							// Armor Class
	int32_t ArmorAmount;						// Armor Amount
	int32_t HealthAmount;						// Health Amount
	
	/* Weapons and Ammo */
	int32_t AmmoMul;							// Ammo multiplier
	int32_t MaxAmmoMul;							// Max ammo multiplier
} PI_touch_t;

extern PI_touchid_t g_RMODNumTouchSpecials;
extern PI_touch_t** g_RMODTouchSpecials;

/*** RMOD ***/

void INFO_StateNormalize(const size_t a_MergeBase, const size_t a_MergeCount);

void PI_ExecuteDEH(void);

/*** HELPFUL FUNCTIONS ***/

bool_t INFO_BoolFromString(const char* const a_String);
PI_mobjid_t INFO_GetTypeByName(const char* const a_Name);
PI_spriteid_t INFO_SpriteNumByName(const char* const a_Name, bool_t a_Create);
actionf_t INFO_FunctionPtrByName(const char* const a_Name, PI_sargc_t* const a_ArgC, PI_sargv_t** const a_ArgV);
uint8_t INFO_PriorityByName(const char* const a_Name);
uint32_t INFO_TransparencyByName(const char* const a_Name);
uint32_t INFO_ColorByName(const char* const a_Name);
INFO_BotObjMetric_t INFO_BotMetricByName(const char* const a_Name);
PI_touchid_t P_RMODTouchSpecialByString(const char* const a_String);
PI_touch_t* P_RMODTouchSpecialForSprite(const uint32_t a_SprNum);
PI_touch_t* P_RMODTouchSpecialForCode(const uint32_t a_Code);
PI_wepid_t INFO_GetWeaponByName(const char* const a_Name);
PI_ammoid_t INFO_GetAmmoByName(const char* const a_Name);
PI_keyid_t INFO_GetKeyByName(const char* const a_Name);
PI_key_t* INFO_KeyByGroupBit(const uint32_t a_Group, const uint32_t a_Bit);

uint32_t PI_GetDEHSound(const uint32_t a_InID);

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

