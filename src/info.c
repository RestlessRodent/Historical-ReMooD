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
// DESCRIPTION: Dynamic Information Tables

// Data.
#include "doomdef.h"
#include "sounds.h"
#include "m_fixed.h"
#include "d_items.h"
#include "p_mobj.h"
#include "z_zone.h"
#include "info.h"
#include "v_video.h"
#include "p_pspr.h"
#include "console.h"
#include "m_random.h"
#include "p_local.h"

// Doesn't work with g++, needs actionf_p1
void A_Light0();
void A_WeaponReady();
void A_Lower();
void A_Raise();
void A_Punch();
void A_ReFire();
void A_FirePistol();
void A_Light1();
void A_FireShotgun();
void A_Light2();
void A_FireShotgun2();
void A_CheckReload();
void A_OpenShotgun2();
void A_LoadShotgun2();
void A_CloseShotgun2();
void A_FireCGun();
void A_GunFlash();
void A_FireMissile();
void A_Saw();
void A_FirePlasma();
void A_BFGsound();
void A_FireBFG();
void A_BFGSpray();
void A_Explode();
void A_Pain();
void A_PlayerScream();
void A_Fall();
void A_XScream();
void A_Look();
void A_Chase();
void A_FaceTarget();
void A_PosAttack();
void A_Scream();
void A_SPosAttack();
void A_VileChase();
void A_VileStart();
void A_VileTarget();
void A_VileAttack();
void A_StartFire();
void A_Fire();
void A_FireCrackle();
void A_Tracer();
void A_SkelWhoosh();
void A_SkelFist();
void A_SkelMissile();
void A_FatRaise();
void A_FatAttack1();
void A_FatAttack2();
void A_FatAttack3();
void A_BossDeath();
void A_CPosAttack();
void A_CPosRefire();
void A_TroopAttack();
void A_SargAttack();
void A_HeadAttack();
void A_BruisAttack();
void A_SkullAttack();
void A_Metal();
void A_SpidRefire();
void A_BabyMetal();
void A_BspiAttack();
void A_Hoof();
void A_CyberAttack();
void A_PainAttack();
void A_PainDie();
void A_KeenDie();
void A_BrainPain();
void A_BrainScream();
void A_BrainDie();
void A_BrainAwake();
void A_BrainSpit();
void A_SpawnSound();
void A_SpawnFly();
void A_BrainExplode();

void A_SmokeTrailer();
void A_SmokeTrailerRocket();
void A_SmokeTrailerSkull();

// ReMooD Additions
void A_FireOldBFG();
void A_FireGenericProjectile();
void A_NextFrameIfMoving();

void A_GenericMonsterMissile();

/*****************************************************************************/

PI_wep_t** wpnlev1info = NULL;
PI_wep_t** wpnlev2info = NULL;
size_t NUMWEAPONS = 0;

ammoinfo_t** ammoinfo = NULL;
size_t NUMAMMO = 0;
char** sprnames = NULL;
void** g_SprTouchSpecials = NULL;				// Sprite touch special markers
size_t NUMSPRITES = 0;
state_t** states = 0;
size_t NUMSTATES = 0;
mobjinfo_t** mobjinfo = NULL;
mobjtype_t NUMMOBJTYPES = 0;
P_TouchNum_t g_RMODNumTouchSpecials = 0;
P_RMODTouchSpecial_t** g_RMODTouchSpecials = NULL;
size_t g_RMODNumKeys = 0;
P_RMODKey_t** g_RMODKeys = NULL;

bool_t l_KeysMapped = false;					// Keys Mapped
P_RMODKey_t* l_KeyMap[2][32];					// Quick Key Mapping

#define LOCALSTATEJUMPS						64	// Local State Jumping

/* INFO_FlagInfo_t -- Flag Information */
typedef struct INFO_FlagInfo_s
{
	uint32_t Field;
	const char* const Name;
} INFO_FlagInfo_t;

// c_xFlags -- "flags"
static const INFO_FlagInfo_t c_xFlags[] =
{
	{MF_SPECIAL, "IsSpecial"},
	{MF_SOLID, "IsSolid"},
	{MF_SHOOTABLE, "IsShootable"},
	{MF_NOSECTOR, "NoSectorLinks"},
	{MF_NOBLOCKMAP, "NoBlockMap"},
	{MF_AMBUSH, "IsDeaf"},
	{MF_JUSTHIT, "JustGotHit"},
	{MF_JUSTATTACKED, "JustAttacked"},
	{MF_SPAWNCEILING, "SpawnsOnCeiling"},
	{MF_NOGRAVITY, "NoGravity"},
	{MF_DROPOFF, "IsDropOff"},
	{MF_PICKUP, "CanPickupItems"},
	{MF_NOCLIP, "NoClipping"},
	{MF_SLIDE, "CanSlideAlongWalls"},
	{MF_FLOAT, "IsFloating"},
	{MF_TELEPORT, "NoLineCrossing"},
	{MF_MISSILE, "IsMissile"},
	{MF_DROPPED, "IsDropped"},
	{MF_SHADOW, "IsFuzzyShadow"},
	{MF_NOBLOOD, "NoBleeding"},
	{MF_CORPSE, "IsCorpse"},
	{MF_INFLOAT, "NoFloatAdjust"},
	{MF_COUNTKILL, "IsKillCountable"},
	{MF_COUNTITEM, "IsItemCountable"},
	{MF_SKULLFLY, "IsFlyingSkull"},
	{MF_NOTDMATCH, "NotInDeathmatch"},
	{MF_NOCLIPTHING, "NoThingClipping"},
	{0, NULL},
};

// c_xFlagsTwo -- "flags2"
static const INFO_FlagInfo_t c_xFlagsTwo[] =
{
	{MF2_LOGRAV, "IsLowGravity"},
	{MF2_WINDTHRUST, "IsWindThrustable"},
	{MF2_FLOORBOUNCE, "IsFloorBouncer"},
	{MF2_THRUGHOST, "PassThruGhosts"},
	{MF2_FLY, "IsFlying"},
	{MF2_FOOTCLIP, "CanFeetClip"},
	{MF2_SPAWNFLOAT, "SpawnAtRandomZ"},
	{MF2_NOTELEPORT, "CannotTeleport"},
	{MF2_RIP, "MissilesThruSolids"},
	{MF2_PUSHABLE, "IsPushable"},
	{MF2_SLIDE, "WallSliding"},
	{MF2_ONMOBJ, "IsOnObject"},
	{MF2_PASSMOBJ, "MoveOverUnderObject"},
	{MF2_CANNOTPUSH, "CannotPushPushables"},
	{MF2_FEETARECLIPPED, "AreFeetClipped"},
	{MF2_BOSS, "IsBoss"},
	{MF2_FIREDAMAGE, "DealsFireDamage"},
	{MF2_NODMGTHRUST, "NoDamageThrust"},
	{MF2_TELESTOMP, "CanTeleStomp"},
	{MF2_FLOATBOB, "FloatBobbing"},
	{MF2_DONTDRAW, "DoNotDraw"},
	{MF2_BOUNCES, "CanBounce"},
	{MF2_FRIENDLY, "IsFriendly"},
	{MF2_FORCETRANSPARENCY, "ForceTransparency"},
	{MF2_FLOORHUGGER, "IsFloorHugger"},
	{0, NULL},
};

// c_xRXFlagsA -- RX[REXA]
static const INFO_FlagInfo_t c_xRXFlagsA[] =
{
	{MFREXA_ENABLEFASTSPEED, "EnableFastSpeed"},
	{MFREXA_NOFORCEALLTRIGGERC, "NoForcedAllTriggers"},
	{MFREXA_NOCROSSTRIGGER, "NoLineCrossTrigger"},
	{MFREXA_ISPUSHPULL, "IsPusherPuller"},
	{MFREXA_DOPUSHAWAY, "PushesAway"},
	{MFREXA_ISTELEPORTMAN, "IsTeleportMan"},
	{MFREXA_ALWAYSTELEPORT, "AlwaysTeleport"},
	{MFREXA_ISMONSTER, "IsMonster"},
	{MFREXA_ISTELEFOG, "IsTeleportFog"},
	{MFREXA_ISPOWERUP, "IsPowerup"},
	{MFREXA_HALFMISSILERANGE, "HalfMissileRange"},
	{MFREXA_SOUNDEVERYWHERE, "PlaySoundsEverywhere"},
	{MFREXA_NOWATERSPLASH, "NoWaterSplashing"},
	{MFREXA_NOCHECKWATER, "NoWaterChecking"},
	{MFREXA_USENULLMOTHINKER, "UseMobjNullThinker"},
	{MFREXA_NOPLAYERWALK, "NoPlayerWalkAnimation"},
	{MFREXA_NOSMOOTHSTEPUP, "NoSmoothStepUp"},
	{MFREXA_NOALTDMRESPAWN, "NoRespawnInAltDM"},
	{MFREXA_CARRYKILLER, "CarriesKiller"},
	{MFREXA_MARKRESTOREWEAPON, "MarkRestoreWeapon"},
	{MFREXA_NORANDOMPLAYERLOOK, "NoRandomPlayerLook"},
	{MFREXA_ALLOWNOCROSSCROSS, "AllowNonCrossableCrossing"},
	{MFREXA_NEVERCROSSTRIGGER, "NeverCrossTrigger"},
	{MFREXA_CANCEILINGSTEP, "CanCeilingStep"},
	{MFREXA_NOTHRESHOLD, "NoThreshold"},
	{MFREXA_NOTRETAILIATETARGET, "NotARetaliateTarget"},
	{MFREXA_RADIUSATTACKPROOF, "ExplosionProof"},
	{MFREXA_RANDOMPUFFTIME, "RandomPuffTime"},
	{MFREXA_KEEPGRAVONDEATH, "KeepGravityWhenKilled"},
	{MFREXA_ISPLAYEROBJECT, "IsPlayerObject"},
	{MFREXA_ISBRAINTARGET, "IsBossBrainTarget"},
	{0, NULL},
};

// c_xRXFlagsB -- RX[REXB]
static const INFO_FlagInfo_t c_xRXFlagsB[] =
{
	{MFREXB_DOMAPSEVENSPECA, "DoDeadSimpleSpecialSix"},
	{MFREXB_DOMAPSEVENSPECB, "DoDeadSimpleSpecialSeven"},
	{MFREXB_DOBARONSPECIAL, "DoBaronSpecial"},
	{MFREXB_DOCYBERSPECIAL, "DoCyberSpecial"},
	{MFREXB_DOSPIDERSPECIAL, "DoSpiderSpecial"},
	{MFREXB_DODOORSIXTHREEOPEN, "DoSixSixSixDoorOpen"},
	{MFREXB_INITBOTNODES, "ForceInitializeBotNodes"},
	{MFREXB_DONTTAKEDAMAGE, "DoNotTakeDamage"},
	{MFREXB_ISHIGHBOUNCER, "IsHighBouncer"},
	{MFREXB_NONMISSILEFLBOUNCE, "NonMissileFloorBounce"},
	{MFREXB_IGNOREBLOCKMONS, "IgnoreBlockMonsterLines"},
	{MFREXB_USEPLAYERMOVEMENT, "UsePlayerMovement"},
	{MFREXB_CANUSEWEAPONS, "CanUseWeapons"},
	{MFREXB_NOFLOORDAMAGE, "NoFloorDamage"},
	{MFREXB_ISDOOMPALETTE, "IsDoomColored"},
	{MFREXB_SPITBIT, "SpitBitSet"},
	{MFREXB_NONMRESPAWN, "NoNightmareRespawn"},
	{MFREXB_FREEZEDEMO, "FreezesDemo"},
	
	{0, NULL},
};

static state_t StaticSNull;

/* INFO_StateNormalize() -- Normalizes State References */
void INFO_StateNormalize(const size_t a_MergeBase, const size_t a_MergeCount)
{
	size_t i, j;
	uint32_t ObjID, RefToFind;	
	
	/* Normalize state references */
	for (i = a_MergeBase; i < a_MergeBase + a_MergeCount; i++)
	{
		// State number here
		states[i]->StateNum = i;
		
		// Calculate SpriteID
		for (j = 0; j < 4 && states[i]->HoldSprite[j]; j++)
			states[i]->SpriteID |= ((uint32_t)toupper(states[i]->HoldSprite[j])) << (j * UINT32_C(8));
		
		// Reference states and functions
		states[i]->sprite = INFO_SpriteNumByName(states[i]->HoldSprite, true);
		
		// Reference function
		if (states[i]->Function)
		{
			states[i]->action = INFO_FunctionPtrByName(states[i]->Function);
			Z_Free(states[i]->Function);
			states[i]->Function = NULL;
		}
		
		// Find next reference
		if (states[i]->SimNext)
		{
			// Get IDs to look for
			ObjID = (states[i]->SimNext >> (uint64_t)32) & ((uint64_t)0xFFFFFFFFU);
			RefToFind = (states[i]->SimNext & (uint64_t)0xFFFFFFFFU);
			
			// Search through everything
			for (j = a_MergeBase; j < a_MergeBase + a_MergeCount; j++)
				if (ObjID == states[j]->ObjectID && RefToFind == states[j]->Marker)
				{
					states[i]->nextstate = j;
					break;
				}
		}
	}
}

/*****************************************************************************/

/*** UPPER CONSTANTS ***/

/* INFO_REMOODATValType_t -- Value Type */
typedef enum INFO_REMOODATValType_e
{
	IRVT_INT32,
	IRVT_UINT32,
	IRVT_FUNC,
	IRVT_STRING,
	IRVT_FIXED,
	
	NUMINFOREMOODDATVALTYPES
} INFO_REMOODATValType_t;

/*** STRUCTURES ***/

/* INFO_REMOODATValEntry_t -- Value Entry */
typedef struct INFO_REMOODATValEntry_s
{
	const char* Name;							// Name of field
	INFO_REMOODATValType_t Type;				// Type of value
	size_t Offset;								// Offset to data
	void (*Func)(void** const a_Data, struct INFO_REMOODATValEntry_s* a_ValEnt, const char* const a_Field, const char* const a_Value, void* const a_WriteP);
} INFO_REMOODATValEntry_t;

/* INFO_REMOODATKeyChain_t -- Keychain */
typedef struct INFO_REMOODATKeyChain_s
{
	const char* Name;							// Name of chain group
	int32_t ValidDepth;							// Valid Depth
	const INFO_REMOODATValEntry_t* Table;		// Lookup table
	void** vPP;									// Pointer to array
	void** (*GetvPPFunc)(void** const a_Data);	// Get void pointer function
	size_t SubArraySize;						// Sub array size
	void* CountP;								// Array Count
	size_t CountSize;							// Array Count Size
	size_t IndivSize;							// Individual size
	void* (*GrabEntry)(void** const a_Data, const char* const a_Name);	// Grabs new entry
	uint8_t For;								// For (states)
	uint32_t Bits;								// Bits
} INFO_REMOODATKeyChain_t;

/*** CONSTANTS ***/

static const struct
{
	int8_t For;								// Kind it is for
	INFO_ObjectStateGroup_t ID;				// ID
	const char* Name;						// Name
	size_t Offset;							// Offset
} c_StateGroups[NUMSTATEGROUPS] =
{
	{0, IOSG_SPAWN, "SpawnState", offsetof(mobjinfo_t,spawnstate)},
	{0, IOSG_ACTIVE, "ActiveState", offsetof(mobjinfo_t,seestate)},
	{0, IOSG_PAIN, "PainState", offsetof(mobjinfo_t,painstate)},
	{0, IOSG_MELEEATTACK, "MeleeAttackState", offsetof(mobjinfo_t,meleestate)},
	{0, IOSG_RANGEDATTACK, "RangedAttackState", offsetof(mobjinfo_t,missilestate)},
	{0, IOSG_CRASH, "CrashState", offsetof(mobjinfo_t,crashstate)},
	{0, IOSG_DEATH, "DeathState", offsetof(mobjinfo_t,deathstate)},
	{0, IOSG_GIB, "GibState", offsetof(mobjinfo_t,xdeathstate)},
	{0, IOSG_RAISE, "RaiseState", offsetof(mobjinfo_t,raisestate)},
	{0, IOSG_PLAYERRUN, "PlayerRunState", offsetof(mobjinfo_t,RPlayerRunState)},
	{0, IOSG_PLAYERMELEE, "PlayerMeleeAttackState", offsetof(mobjinfo_t,RPlayerMeleeAttackState)},
	{0, IOSG_PLAYERRANGED, "PlayerRangedAttackState", offsetof(mobjinfo_t,RPlayerRangedAttackState)},
	{0, IOSG_VILEHEAL, "VileHealState", offsetof(mobjinfo_t,RVileHealState)},
	{0, IOSG_LESSBLOODA, "LessLessBloodState", offsetof(mobjinfo_t,RLessBlood[0])},
	{0, IOSG_LESSBLOODB, "LessMoreBloodState", offsetof(mobjinfo_t,RLessBlood[1])},
	{0, IOSG_BRAINEXPLODE, "BrainExplodeState", offsetof(mobjinfo_t,RBrainExplodeState)},
	{0, IOSG_MELEEPUFF, "MeleePuffState", offsetof(mobjinfo_t,RMeleePuffState)},
	
	{1, PWSG_UP, "PrimaryBringUpState", offsetof(PI_wep_t, upstate)},
	{1, PWSG_DOWN, "PrimaryPutDownState", offsetof(PI_wep_t, downstate)},
	{1, PWSG_READY, "PrimaryReadyState", offsetof(PI_wep_t, readystate)},
	{1, PWSG_ATTACK, "PrimaryFireState", offsetof(PI_wep_t, atkstate)},
	{1, PWSG_HOLDATTACK, "PrimaryFireHeldState", offsetof(PI_wep_t, holdatkstate)},
	{1, PWSG_FLASH, "PrimaryFlashState", offsetof(PI_wep_t, flashstate)},
};

void* INFO_MobjInfoGrabEntry(void** const a_Data, const char* const a_Name);
void* INFO_StateGrabEntry(void** const a_Data, const char* const a_Name);
void* INFO_StEntryGrabEntry(void** const a_Data, const char* const a_Name);
void* INFO_WeaponGrabEntry(void** const a_Data, const char* const a_Name);
void* INFO_AmmoGrabEntry(void** const a_Data, const char* const a_Name);
void* INFO_TouchGrabEntry(void** const a_Data, const char* const a_Name);
void* INFO_KeyGrabEntry(void** const a_Data, const char* const a_Name);

void INFO_MiscObjectGF(void** const a_Data, struct INFO_REMOODATValEntry_s* a_ValEnt, const char* const a_Field, const char* const a_Value, void* const a_WriteP);

// c_INFOMobjTables -- Object Tables
static const INFO_REMOODATValEntry_t c_INFOMobjTables[] =
{
	{"-", IRVT_STRING, offsetof(mobjinfo_t, RClassName)},
	{"DoomEdNum", IRVT_INT32, offsetof(mobjinfo_t, EdNum[COREGAME_DOOM])},
	{"HereticEdNum", IRVT_INT32, offsetof(mobjinfo_t, EdNum[COREGAME_HERETIC])},
	{"HexenEdNum", IRVT_INT32, offsetof(mobjinfo_t, EdNum[COREGAME_HEXEN])},
	{"StrifeEdNum", IRVT_INT32, offsetof(mobjinfo_t, EdNum[COREGAME_STRIFE])},
	{"DeHackEdNum", IRVT_UINT32, offsetof(mobjinfo_t, RDehackEdID)},
	
	{"DropsClass", IRVT_STRING, offsetof(mobjinfo_t, RDropClass)},
	{"BrainExplodeClass", IRVT_STRING, offsetof(mobjinfo_t, RBrainExplodeThing)},
	{"MTName", IRVT_STRING, offsetof(mobjinfo_t, RMTName)},
	{"NiceName", IRVT_STRING, offsetof(mobjinfo_t, RNiceName)},
	{"BaseFamily", IRVT_STRING, offsetof(mobjinfo_t, RFamilyClass)},
	{"WakeSound", IRVT_STRING, offsetof(mobjinfo_t, RSeeSound)},
	{"AttackSound", IRVT_STRING, offsetof(mobjinfo_t, RAttackSound)},
	{"PainSound", IRVT_STRING, offsetof(mobjinfo_t, RPainSound)},
	{"DeathSound", IRVT_STRING, offsetof(mobjinfo_t, RDeathSound)},
	{"ActiveSound", IRVT_STRING, offsetof(mobjinfo_t, RActiveSound)},
	{"MissileSplat", IRVT_STRING, offsetof(mobjinfo_t, RMissileSplat)},
	{"BloodSplat", IRVT_STRING, offsetof(mobjinfo_t, RBloodSplat)},
	{"BloodSpewClass", IRVT_STRING, offsetof(mobjinfo_t, RBloodSpewClass)},
	{"GenericMissile", IRVT_STRING, offsetof(mobjinfo_t, RGenericMissile)},
	{"ShortNiceName", IRVT_STRING, offsetof(mobjinfo_t, RSNiceName)},
	
	{"SpawnHealth", IRVT_INT32, offsetof(mobjinfo_t, spawnhealth)},
	{"Mass", IRVT_INT32, offsetof(mobjinfo_t, mass)},
	{"Damage", IRVT_INT32, offsetof(mobjinfo_t, damage)},
	{"ReactionTime", IRVT_INT32, offsetof(mobjinfo_t, reactiontime)},
	
	{"Speed", IRVT_FIXED, offsetof(mobjinfo_t, speed)},
	{"Radius", IRVT_FIXED, offsetof(mobjinfo_t, radius)},
	{"Height", IRVT_FIXED, offsetof(mobjinfo_t, Height)},
	{"OldHeight", IRVT_FIXED, offsetof(mobjinfo_t, OldHeight)},
	{"MinMissileDist", IRVT_FIXED, offsetof(mobjinfo_t, RMissileDist[0])},
	{"MaxMissileDist", IRVT_FIXED, offsetof(mobjinfo_t, RMissileDist[1])},
	{"CapMissileDist", IRVT_FIXED, offsetof(mobjinfo_t, RCapMissileDist)},
	{"FastSpeed", IRVT_FIXED, offsetof(mobjinfo_t, RFastSpeed)},
	{"BounceFactor", IRVT_FIXED, offsetof(mobjinfo_t, RBounceFactor)},
	{"AirGravity", IRVT_FIXED, offsetof(mobjinfo_t, RAirGravity)},
	{"WaterGravity", IRVT_FIXED, offsetof(mobjinfo_t, RWaterGravity)},
	
	{"BotMetric", IRVT_FUNC, 0, INFO_MiscObjectGF},
	{"PainChance", IRVT_FUNC, 0, INFO_MiscObjectGF},
	{"?", IRVT_FUNC, 0, INFO_MiscObjectGF},
	
	{NULL},
};

// c_INFOStateTables -- State Tables
static const INFO_REMOODATValEntry_t c_INFOStateTables[] =
{
	{NULL},
};

void INFO_MiscWeaponGF(void** const a_Data, struct INFO_REMOODATValEntry_s* a_ValEnt, const char* const a_Field, const char* const a_Value, void* const a_WriteP);

// c_INFOWeaponTables -- Weapon Tables
static const INFO_REMOODATValEntry_t c_INFOWeaponTables[] =
{
	{"-", IRVT_STRING, offsetof(PI_wep_t, ClassName)},
	{"DroppedObject", IRVT_STRING, offsetof(PI_wep_t, DropWeaponClass)},
	{"NiceName", IRVT_STRING, offsetof(PI_wep_t, NiceName)},
	{"SBOGraphic", IRVT_STRING, offsetof(PI_wep_t, SBOGraphic)},
	{"BringUpSound", IRVT_STRING, offsetof(PI_wep_t, BringUpSound)},
	{"IdleNoise", IRVT_STRING, offsetof(PI_wep_t, IdleNoise)},
	{"Ammo", IRVT_STRING, offsetof(PI_wep_t, AmmoClass)},
	{"PuffClass", IRVT_STRING, offsetof(PI_wep_t, ReplacePuffType)},
	{"GenericProjectile", IRVT_STRING, offsetof(PI_wep_t, GenericProjectile)},
	{"TracerSplat", IRVT_STRING, offsetof(PI_wep_t, TracerSplat)},
	
	{"AmmoPerShot", IRVT_INT32, offsetof(PI_wep_t, ammopershoot)},
	{"SwitchOrder", IRVT_INT32, offsetof(PI_wep_t, SwitchOrder)},
	{"PickupAmmo", IRVT_INT32, offsetof(PI_wep_t, GetAmmo)},
	{"SlotNum", IRVT_INT32, offsetof(PI_wep_t, SlotNum)},
	{"NoAmmoSwitchOrder", IRVT_INT32, offsetof(PI_wep_t, NoAmmoOrder)},
	{"DeHackEdID", IRVT_INT32, offsetof(PI_wep_t, DEHId)},
	
	{"SpriteYOffset", IRVT_FIXED, offsetof(PI_wep_t, PSpriteSY)},
	
	{"BotMetric", IRVT_FUNC, 0, INFO_MiscWeaponGF},
	{"?", IRVT_FUNC, 0, INFO_MiscWeaponGF},
	
	{NULL},
};

void INFO_MiscAmmoGF(void** const a_Data, struct INFO_REMOODATValEntry_s* a_ValEnt, const char* const a_Field, const char* const a_Value, void* const a_WriteP);

// c_INFOAmmoTables -- Ammo Tables
static const INFO_REMOODATValEntry_t c_INFOAmmoTables[] =
{
	{"-", IRVT_STRING, offsetof(ammoinfo_t, ClassName)},
	
	{"ClipAmmo", IRVT_INT32, offsetof(ammoinfo_t, ClipAmmo)},
	{"MaxAmmo", IRVT_INT32, offsetof(ammoinfo_t, MaxAmmo)},
	{"StartingAmmo", IRVT_INT32, offsetof(ammoinfo_t, StartingAmmo)},
	
	{"?", IRVT_FUNC, 0, INFO_MiscAmmoGF},
	
	{NULL},
};


void INFO_MiscKeyGF(void** const a_Data, struct INFO_REMOODATValEntry_s* a_ValEnt, const char* const a_Field, const char* const a_Value, void* const a_WriteP);

// c_INFOKeyTables -- Key Cards Table
static const INFO_REMOODATValEntry_t c_INFOKeyTables[] =
{
	{"-", IRVT_STRING, offsetof(P_RMODKey_t, ClassName)},
	
	{"Bit", IRVT_FUNC, 0, INFO_MiscKeyGF},
	{"Group", IRVT_FUNC, 0, INFO_MiscKeyGF},
	
	{"Icon", IRVT_STRING, offsetof(P_RMODKey_t, ImageName)},
	{"Color", IRVT_STRING, offsetof(P_RMODKey_t, ColorName)},
	
	{NULL},
};

void INFO_MiscTouchGF(void** const a_Data, struct INFO_REMOODATValEntry_s* a_ValEnt, const char* const a_Field, const char* const a_Value, void* const a_WriteP);

// c_INFOTouchTables -- Toucher Tables
static const INFO_REMOODATValEntry_t c_INFOTouchTables[] =
{
	{"-", IRVT_FUNC, 0, INFO_MiscTouchGF},
	
	{"Message", IRVT_FUNC, 0, INFO_MiscTouchGF},
	
	{"PickupSound", IRVT_STRING, offsetof(P_RMODTouchSpecial_t, PickupSnd)},
	{"GiveWeapon", IRVT_STRING, offsetof(P_RMODTouchSpecial_t, GiveWeapon)},
	{"GiveAmmo", IRVT_STRING, offsetof(P_RMODTouchSpecial_t, GiveAmmo)},
	{"GiveKey", IRVT_STRING, offsetof(P_RMODTouchSpecial_t, GiveKey)},
	
	{"ArmorClass", IRVT_INT32, offsetof(P_RMODTouchSpecial_t, ArmorClass)},
	{"ArmorAmount", IRVT_INT32, offsetof(P_RMODTouchSpecial_t, ArmorAmount)},
	{"HealthAmount", IRVT_INT32, offsetof(P_RMODTouchSpecial_t, HealthAmount)},
	{"AmmoMul", IRVT_INT32, offsetof(P_RMODTouchSpecial_t, AmmoMul)},
	{"MaxAmmoMul", IRVT_INT32, offsetof(P_RMODTouchSpecial_t, MaxAmmoMul)},
	
	{"?", IRVT_FUNC, 0, INFO_MiscTouchGF},
	
	{NULL},
};

void INFO_FrameNextGoto(void** const a_Data, struct INFO_REMOODATValEntry_s* a_ValEnt, const char* const a_Field, const char* const a_Value, void* const a_WriteP);
void INFO_FrameMisc(void** const a_Data, struct INFO_REMOODATValEntry_s* a_ValEnt, const char* const a_Field, const char* const a_Value, void* const a_WriteP);

// c_INFOFrameTables -- State Frame Tables
static const INFO_REMOODATValEntry_t c_INFOFrameTables[] =
{
	{"DeHackEdNum", IRVT_UINT32, offsetof(state_t, DehackEdID)},
	{"Tics", IRVT_INT32, offsetof(state_t, tics)},
	{"FastTics", IRVT_INT32, offsetof(state_t, RMODFastTics)},
	{"Function", IRVT_STRING, offsetof(state_t, Function)},
	
	{"Next", IRVT_FUNC, offsetof(state_t, SimNext), INFO_FrameNextGoto},
	{"Goto", IRVT_FUNC, offsetof(state_t, SimNext), INFO_FrameNextGoto},
	
	{"Frame", IRVT_FUNC, offsetof(state_t, frame), INFO_FrameMisc},
	{"Sprite", IRVT_FUNC, offsetof(state_t, HoldSprite), INFO_FrameMisc},
	{"Transparency", IRVT_FUNC, offsetof(state_t, frame), INFO_FrameMisc},
	{"FullBright", IRVT_FUNC, offsetof(state_t, frame), INFO_FrameMisc},
	{"Priority", IRVT_FUNC, offsetof(state_t, Priority), INFO_FrameMisc},
	
	{NULL},
};

// c_INFOChains -- Chains for REMOODAT
static const INFO_REMOODATKeyChain_t c_INFOChains[] =
{
	{"MapObject", 1, c_INFOMobjTables,
		&mobjinfo, NULL, sizeof(*mobjinfo), &NUMMOBJTYPES,
		sizeof(NUMMOBJTYPES), sizeof(**mobjinfo), INFO_MobjInfoGrabEntry, 0,
		1},
	{"State", 2, c_INFOStateTables,
		NULL, NULL, sizeof(*states), &NUMSTATES,
		sizeof(NUMSTATES), sizeof(**states), INFO_StateGrabEntry, 0,
		1 | 2},
	{"Frame", 3, c_INFOFrameTables,
		NULL, NULL, 0, NULL,
		0, sizeof(**states), INFO_StEntryGrabEntry, 0,
		1 | 2},
	{"MapWeapon", 1, c_INFOWeaponTables,
		NULL, NULL, 0, NULL,
		0, 0, INFO_WeaponGrabEntry, 1,
		2},
	{"MapAmmo", 1, c_INFOAmmoTables,
		NULL, NULL, 0, NULL,
		0, 0, INFO_AmmoGrabEntry, 1,
		4},
	{"MapTouchSpecial", 1, c_INFOTouchTables,
		NULL, NULL, 0, NULL,
		0, 0, INFO_TouchGrabEntry, 1,
		8},
	{"MapKey", 1, c_INFOKeyTables,
		NULL, NULL, 0, NULL,
		0, 0, INFO_KeyGrabEntry, 1,
		16},
	
	{NULL},
};

/*** FUNCTIONS ***/

/* INFO_DataStore_t -- Data Storage */
typedef struct INFO_DataStore_s
{
	struct INFO_DataStore_s* Parent;			// Parent Store
	INFO_REMOODATKeyChain_t* CurrentKey;		// Current Key used
	INFO_ObjectStateGroup_t StateGroup;			// State group operating on
	mobjtype_t MoType;							// Object Type
	uint32_t FrameFor;							// Frame fors
	union
	{
		void* vP;								// Void pointer
		mobjinfo_t* InfoP;						// Info
		statenum_t* StateRefP;					// State Reference
		state_t* StateP;						// State
		PI_wep_t* WeaponP;					// Weapon
		ammoinfo_t* AmmoP;						// Ammo
		P_RMODTouchSpecial_t* TouchP;			// Toucher
		P_RMODKey_t* KeyP;						// Key
	} Cur;										// Current pointer sets
} INFO_DataStore_t;

/* INFO_MobjInfoGrabEntry() -- Grabs mobjinfo_t */
void* INFO_MobjInfoGrabEntry(void** const a_Data, const char* const a_Name)
{
	INFO_DataStore_t** StorePP;
	INFO_DataStore_t* This;
	mobjtype_t Type;
	mobjinfo_t* Ptr;
	
	/* Storage Pointer */
	StorePP = a_Data;
	if (StorePP)
		This = *StorePP;
	
	/* Do normal name lookup */
	Type = INFO_GetTypeByName(a_Name);
	Ptr = NULL;
	
	/* Found? */
	if (Type < NUMMOBJTYPES)
		Ptr = mobjinfo[Type];
	
	/* Not Found */
	else
	{
		Z_ResizeArray((void**)&mobjinfo, sizeof(*mobjinfo),
			NUMMOBJTYPES, NUMMOBJTYPES + 1);
		Ptr = mobjinfo[(Type = NUMMOBJTYPES++)] = Z_Malloc(sizeof(**mobjinfo), PU_REMOODAT, NULL);
		Z_ChangeTag(mobjinfo, PU_REMOODAT);
		
		// Initialize
		memset(Ptr->EdNum, 0xFF, sizeof(Ptr->EdNum));
		Ptr->ObjectID = Type;
	}
	
	/* Set object type */
	This->MoType = Type + 1;
	
	/* Loading Screen */
	CONL_EarlyBootTic(a_Name, true);
	
	/* Return pointer */
	return Ptr;
}

/* INFO_StateGrabEntry() -- Grabs a state entry */
void* INFO_StateGrabEntry(void** const a_Data, const char* const a_Name)
{
	INFO_DataStore_t** StorePP;
	INFO_DataStore_t* This;
	int32_t i;
	int8_t WantedFor;
	
	/* Storage Pointer */
	StorePP = a_Data;
	if (StorePP)
		This = *StorePP;
	
	/* Missing? */
	if (!This || (This && !This->Parent->Cur.vP))
		return NULL;
	
	/* Which kind of thing with states are we looking for? */
	WantedFor = This->Parent->FrameFor;	// TODO FIXME
	
	/* Find in groupings */
	for (i = 0; i < NUMSTATEGROUPS; i++)
		if (c_StateGroups[i].For == WantedFor)
			if (c_StateGroups[i].Name)
				if (strcasecmp(a_Name, c_StateGroups[i].Name) == 0)
					break;
	
	// Not found?
	if (i >= NUMSTATEGROUPS)
		return NULL;
	
	/* Return reference to it */
	This->StateGroup = c_StateGroups[i].ID;
	This->FrameFor = WantedFor;
	return (void*)(((uintptr_t)This->Parent->Cur.InfoP) + c_StateGroups[i].Offset);
}

/* INFO_StEntryGrabEntry() -- Grab State */
void* INFO_StEntryGrabEntry(void** const a_Data, const char* const a_Name)
{
	INFO_DataStore_t** StorePP;
	INFO_DataStore_t* This;
	uint8_t IOSG;
	uint32_t FrameID, ObjectID;
	int32_t i;
	state_t* StateP;
	
	/* Storage Pointer */
	StorePP = a_Data;
	if (StorePP)
		This = *StorePP;
	
	/* Missing? */
	if (!This || (This && !This->Parent->Cur.vP))
		return NULL;
	
	/* Get Current IDs */
	ObjectID = This->Parent->Parent->MoType;
	IOSG = This->Parent->StateGroup;
	FrameID = C_strtou32(a_Name, NULL, 10);
	
	// Bad frame?
	if (FrameID == 0)
		return NULL;
	
	// Zero Correct
	FrameID--;
	
	/* Go through all state tables for a match */
	StateP = NULL;
	for (i = 0; i < NUMSTATES; i++)
		if (ObjectID == states[i]->ObjectID &&
			IOSG == states[i]->IOSG &&
			FrameID == states[i]->FrameID)
				break;
	
	/* Found? */
	if (i < NUMSTATES)
		StateP = states[i];
	
	/* Not Found */
	else
	{
		Z_ResizeArray((void**)&states, sizeof(*states),
			NUMSTATES, NUMSTATES + 1);
		StateP = states[NUMSTATES++] = Z_Malloc(sizeof(*StateP), PU_REMOODAT, NULL);
		
		// Set references
		StateP->ObjectID = ObjectID;
		StateP->IOSG = IOSG;
		StateP->FrameID = FrameID;
		StateP->Marker = ((uint32_t)StateP->IOSG) << UINT32_C(16);
		StateP->Marker |= ((uint32_t)StateP->FrameID) & UINT32_C(0xFFFF);
	}
	
	/* Frame is zero and reference not set? */
	if (FrameID == 0 &&
		*This->Parent->Cur.StateRefP == 0)
		*This->Parent->Cur.StateRefP = i;
	
	/* Return pointer */
	return StateP;
}

/* INFO_WeaponGrabEntry() -- Grabs PI_wep_t */
void* INFO_WeaponGrabEntry(void** const a_Data, const char* const a_Name)
{
	INFO_DataStore_t** StorePP;
	INFO_DataStore_t* This;
	weapontype_t Type;
	PI_wep_t* Ptr;
	
	/* Storage Pointer */
	StorePP = a_Data;
	if (StorePP)
		This = *StorePP;
	
	/* Do normal name lookup */
	Type = INFO_GetWeaponByName(a_Name);
	Ptr = NULL;
	
	/* Found? */
	if (Type < NUMWEAPONS)
		Ptr = wpnlev1info[Type];
	
	/* Not Found */
	else
	{
		Z_ResizeArray((void**)&wpnlev1info, sizeof(*wpnlev1info),
			NUMWEAPONS, NUMWEAPONS + 1);
		Ptr = wpnlev1info[(Type = NUMWEAPONS++)] = Z_Malloc(sizeof(**wpnlev1info), PU_REMOODAT, NULL);
		Z_ChangeTag(wpnlev1info, PU_REMOODAT);
		
		// Initialize
		Ptr->WeaponID = ~(Type + 1);
	}
	
	/* Set object type */
	This->MoType = ~(Type + 1);
	
	/* Loading Screen */
	CONL_EarlyBootTic(a_Name, true);
	
	/* Return pointer */
	return Ptr;
}

/* INFO_AmmoGrabEntry() -- Grab Ammo */
void* INFO_AmmoGrabEntry(void** const a_Data, const char* const a_Name)
{
	INFO_DataStore_t** StorePP;
	INFO_DataStore_t* This;
	ammotype_t Type;
	ammoinfo_t* Ptr;
	
	/* Storage Pointer */
	StorePP = a_Data;
	if (StorePP)
		This = *StorePP;
	
	/* Do normal name lookup */
	Type = INFO_GetAmmoByName(a_Name);
	Ptr = NULL;
	
	/* Found? */
	if (Type < NUMAMMO)
		Ptr = ammoinfo[Type];
	
	/* Not Found */
	else
	{
		Z_ResizeArray((void**)&ammoinfo, sizeof(*ammoinfo),
			NUMAMMO, NUMAMMO + 1);
		Ptr = ammoinfo[(Type = NUMAMMO++)] = Z_Malloc(sizeof(**ammoinfo), PU_REMOODAT, NULL);
		Z_ChangeTag(ammoinfo, PU_REMOODAT);
	}
	
	/* Set object type */
	This->MoType = Type;
	
	/* Loading Screen */
	CONL_EarlyBootTic(a_Name, true);
	
	/* Return pointer */
	return Ptr;
}

/* INFO_TouchGrabEntry() -- Grab Toucher */
void* INFO_TouchGrabEntry(void** const a_Data, const char* const a_Name)
{
	INFO_DataStore_t** StorePP;
	INFO_DataStore_t* This;
	P_TouchNum_t Type;
	P_RMODTouchSpecial_t* Ptr;
	
	/* Storage Pointer */
	StorePP = a_Data;
	if (StorePP)
		This = *StorePP;
	
	/* Do normal name lookup */
	Type = P_RMODTouchSpecialByString(a_Name);
	Ptr = NULL;
	
	/* Found? */
	if (Type < g_RMODNumTouchSpecials)
		Ptr = g_RMODTouchSpecials[Type];
	
	/* Not Found */
	else
	{
		Z_ResizeArray((void**)&g_RMODTouchSpecials, sizeof(*g_RMODTouchSpecials),
			g_RMODNumTouchSpecials, g_RMODNumTouchSpecials + 1);
		Ptr = g_RMODTouchSpecials[(Type = g_RMODNumTouchSpecials++)] = Z_Malloc(sizeof(**g_RMODTouchSpecials), PU_REMOODAT, NULL);
		Z_ChangeTag(g_RMODTouchSpecials, PU_REMOODAT);
		
		// Init
		Ptr->ActSpriteNum = INT_MAX;//NUMSPRITES;
		Ptr->ActGiveWeapon = INT_MAX;//NUMWEAPONS;
		Ptr->ActGiveAmmo = INT_MAX;//NUMAMMO;
		Ptr->ActGiveKey = INT_MAX;
		
		Ptr->AmmoMul = 1;
		Ptr->MaxAmmoMul = 1;
	}
	
	/* Set object type */
	This->MoType = Type;
	
	/* Loading Screen */
	CONL_EarlyBootTic(a_Name, true);
	
	/* Return pointer */
	return Ptr;
}

/* INFO_KeyGrabEntry() -- Grabs a new key */
void* INFO_KeyGrabEntry(void** const a_Data, const char* const a_Name)
{
	INFO_DataStore_t** StorePP;
	INFO_DataStore_t* This;
	P_KeyNum_t Type;
	P_RMODKey_t* Ptr;
	
	/* Storage Pointer */
	StorePP = a_Data;
	if (StorePP)
		This = *StorePP;
	
	/* Do normal name lookup */
	Type = INFO_GetKeyByName(a_Name);
	Ptr = NULL;
	
	/* Found? */
	if (Type < g_RMODNumKeys)
		Ptr = g_RMODKeys[Type];
	
	/* Not Found */
	else
	{
		Z_ResizeArray((void**)&g_RMODKeys, sizeof(*g_RMODKeys),
			g_RMODNumKeys, g_RMODNumKeys + 1);
		Ptr = g_RMODKeys[(Type = g_RMODNumKeys++)] = Z_Malloc(sizeof(**g_RMODKeys), PU_REMOODAT, NULL);
		Z_ChangeTag(g_RMODKeys, PU_REMOODAT);
		
		// Init
		Ptr->Group = -1;
	}
	
	/* Set object type */
	This->MoType = Type;
	
	/* Loading Screen */
	CONL_EarlyBootTic(a_Name, true);
	
	/* Return pointer */
	return Ptr;
}

/* INFO_MiscObjectGF() -- Guess object flags */
void INFO_MiscObjectGF(void** const a_Data, struct INFO_REMOODATValEntry_s* a_ValEnt, const char* const a_Field, const char* const a_Value, void* const a_WriteP)
{
	INFO_DataStore_t** StorePP;
	INFO_DataStore_t* This;
	mobjinfo_t* Obj;
	bool_t SetFlag;
	int32_t i, r;
	const INFO_FlagInfo_t* BaseArr;
	fixed_t FixedVal;
	
	uint32_t Bit;
	uint32_t* Ref;
	
	/* Storage Pointer */
	StorePP = a_Data;
	if (StorePP)
		This = *StorePP;
	
	/* Get Object */
	Obj = This->Cur.InfoP;
	
	/* Metric? */
	if (strcasecmp(a_Field, "BotMetric") == 0)
	{
		Obj->BotMetric = INFO_BotMetricByName(a_Value);
		return;
	}
	
	/* Pain Chance? */
	else if (strcasecmp(a_Field, "PainChance") == 0)
	{
		// Get Value
		FixedVal = C_strtofx(a_Value, NULL);
	
		// Cap value
		if (FixedVal > FIXEDT_C(1))
			FixedVal = FIXEDT_C(1);
		else if (FixedVal < FIXEDT_C(-1))
			FixedVal = FIXEDT_C(0);
	
		// Multiply by 255 to get it
		Obj->painchance = FixedMul(FIXEDT_C(255), FixedVal) >> FRACBITS;
	}
	
	/* Determine flag change */
	else
	{
		// Initialize with new value
		SetFlag = INFO_BoolFromString(a_Value);
		
		// Reset
		Bit = 0;
		Ref = NULL;
	
		// Find flag in flag fields
		for (r = 0; r < 2 + NUMINFORXFIELDS; r++)
		{
			if (r == 0)
			{
				BaseArr = c_xFlags;
				Ref = &Obj->flags;
			}
			else if (r == 1)
			{
				BaseArr = c_xFlagsTwo;
				Ref = &Obj->flags2;
			}
			else if (r == 2)
			{
				BaseArr = c_xRXFlagsA;
				Ref = &Obj->RXFlags[0];
			}
			else if (r == 3)
			{
				BaseArr = c_xRXFlagsB;
				Ref = &Obj->RXFlags[1];
			}
			else
				BaseArr = NULL;
			
			if (BaseArr)
				for (i = 0; BaseArr[i].Name; i++)
					if (strcasecmp(BaseArr[i].Name, a_Field) == 0)
					{
						Bit = BaseArr[i].Field;
						r = 125;
						break;
					}
		}
	
		// Bit found? Either set or unset
		if (Bit && Ref)
			if (SetFlag)
				*Ref |= Bit;
			else
				*Ref &= ~Bit;
	}
}

/* INFO_MiscWeaponGF() -- Weapon General Flags */
void INFO_MiscWeaponGF(void** const a_Data, struct INFO_REMOODATValEntry_s* a_ValEnt, const char* const a_Field, const char* const a_Value, void* const a_WriteP)
{
	INFO_DataStore_t** StorePP;
	INFO_DataStore_t* This;
	PI_wep_t* Wep;
	bool_t SetFlag;
	int32_t i, r;
	
	uint32_t Bit;
	uint32_t* Ref;
	
	static const INFO_FlagInfo_t c_WeaponFlags[] =
	{
		{WF_ISDOOM, "IsDoom"}, 
		{WF_ISHERETIC, "IsHeretic"}, 
		{WF_ISHEXEN, "IsHexen"}, 
		{WF_ISSTRIFE, "IsStrife"}, 
		{WF_NOTSHAREWARE, "IsNotShareware"}, 
		{WF_INCOMMERCIAL, "IsInCommercial"}, 
		{WF_INREGISTERED, "IsRegistered"}, 
		{WF_BERSERKTOGGLE, "IsBerserkToggle"}, 
		{WF_SWITCHFROMNOAMMO, "IsSwitchFromNoAmmo"}, 
		{WF_STARTINGWEAPON, "IsStartingWeapon"}, 
		{WF_NOTHRUST, "NoThrust"}, 
		{WF_NOAUTOFIRE, "NoAutoFire"}, 
		{WF_INEXTENDED, "IsInExtended"}, 
		{WF_NOBLEEDTARGET, "NoBleedTarget"}, 
		{WF_SUPERWEAPON, "IsSuperWeapon"}, 
		{WF_NONOISEALERT, "NoNoiseAlert"},
		{WF_ISMELEE, "IsMelee"},
		
		{0, NULL},
	};
	
	/* Storage Pointer */
	StorePP = a_Data;
	if (StorePP)
		This = *StorePP;
	
	/* Get Object */
	Wep = This->Cur.WeaponP;
	
	/* Metric? */
	if (strcasecmp(a_Field, "BotMetric") == 0)
	{
		Wep->BotMetric = INFO_BotMetricByName(a_Value);
		return;
	}
	
	/* Determine flag change */
	SetFlag = INFO_BoolFromString(a_Value);
	
	/* Find field */
	// Reset
	Bit = 0;
	Ref = &Wep->WeaponFlags;
	
	// Find flag in flag fields
	for (i = 0; c_WeaponFlags[i].Name; i++)
		if (strcasecmp(c_WeaponFlags[i].Name, a_Field) == 0)
		{
			Bit = c_WeaponFlags[i].Field;
			break;
		}
	
	/* Bit found? */
	// Either set or unset
	if (Bit && Ref)
		if (SetFlag)
			*Ref |= Bit;
		else
			*Ref &= ~Bit;
}


/* INFO_MiscWeaponGF() -- Weapon General Flags */
void INFO_MiscAmmoGF(void** const a_Data, struct INFO_REMOODATValEntry_s* a_ValEnt, const char* const a_Field, const char* const a_Value, void* const a_WriteP)
{
	INFO_DataStore_t** StorePP;
	INFO_DataStore_t* This;
	ammoinfo_t* Amm;
	bool_t SetFlag;
	int32_t i, r;
	
	uint32_t Bit;
	uint32_t* Ref;
	
	static const INFO_FlagInfo_t c_AmmoFlags[] =
	{
		{AF_INFINITE, "IsInfinite"},
		
		{0, NULL},
	};
	
	/* Storage Pointer */
	StorePP = a_Data;
	if (StorePP)
		This = *StorePP;
	
	/* Get Object */
	Amm = This->Cur.AmmoP;
	
	/* Determine flag change */
	SetFlag = INFO_BoolFromString(a_Value);
	
	/* Find field */
	// Reset
	Bit = 0;
	Ref = &Amm->Flags;
	
	// Find flag in flag fields
	for (i = 0; c_AmmoFlags[i].Name; i++)
		if (strcasecmp(c_AmmoFlags[i].Name, a_Field) == 0)
		{
			Bit = c_AmmoFlags[i].Field;
			break;
		}
	
	/* Bit found? */
	// Either set or unset
	if (Bit && Ref)
		if (SetFlag)
			*Ref |= Bit;
		else
			*Ref &= ~Bit;
}

/* INFO_MiscKeyGF() -- Key Handling Stuff */
void INFO_MiscKeyGF(void** const a_Data, struct INFO_REMOODATValEntry_s* a_ValEnt, const char* const a_Field, const char* const a_Value, void* const a_WriteP)
{
	INFO_DataStore_t** StorePP;
	INFO_DataStore_t* This;
	P_RMODKey_t* Key;
	uint32_t Val;
	
	/* Storage Pointer */
	StorePP = a_Data;
	if (StorePP)
		This = *StorePP;
	
	/* Get Object */
	Key = This->Cur.KeyP;
	
	/* Changing Bit */
	if (strcasecmp(a_Field, "Bit") == 0)
	{
		// Convert to integer
		Val = C_strtou32(a_Value, NULL, 0);
		
		// Limit to 32-bits
		if (Val < 1)
			Val = 1;
		else if (Val > 32)
			Val = 32;
		
		// Subtract one
		Val--;
		
		// Set Key bit
		Key->Bit = UINT32_C(1) << Val;
		Key->BitNum = Val;
	}
	
	/* Changing Group */
	else if (strcasecmp(a_Field, "Group") == 0)
	{
		// Card Key
		if (strcasecmp(a_Value, "Card") == 0)
			Key->Group = 0;
		
		// Skull Key
		else if (strcasecmp(a_Value, "Skull") == 0)
			Key->Group = 1;
		
		// Illegal
		else
			Key->Group = -1;
	}
}

/* INFO_MiscTouchGF() -- Misc Toucher Stuff */
void INFO_MiscTouchGF(void** const a_Data, struct INFO_REMOODATValEntry_s* a_ValEnt, const char* const a_Field, const char* const a_Value, void* const a_WriteP)
{
	INFO_DataStore_t** StorePP;
	INFO_DataStore_t* This;
	P_RMODTouchSpecial_t* Touch;
	bool_t SetFlag;
	int32_t i, r;
	
	uint32_t Bit;
	uint32_t* Ref;
	
	static const INFO_FlagInfo_t c_TouchFlags[] =
	{
		{PMTSF_KEEPNOTNEEDED, "IsKeepIfNotNeeded"},
		{PMTSF_REMOVEALWAYS, "IsRemoveAlways"},
		{PMTSF_MONSTERCANGRAB, "IsMonsterGrab"},
		{PMTSF_DEVALUE, "IsDeValue"},
		{PMTSF_CAPNORMSTAT, "IsCapNormStat"},
		{PMTSF_CAPMAXSTAT, "IsCapMaxStat"},
		{PMTSF_GREATERARMORCLASS, "IsGreaterArmorClass"},
		{PMTSF_SETBACKPACK, "SetBackpack"},
		{PMTSF_KEEPINMULTI, "IsKeepInMultiplayer"},
		
		{0, NULL},
	};
	
	/* Storage Pointer */
	StorePP = a_Data;
	if (StorePP)
		This = *StorePP;
	
	/* Get Object */
	Touch = This->Cur.TouchP;
	
	/* Changing Flags */
	if (strcasecmp("?", a_ValEnt->Name) == 0)
	{
		// Determine flag change
		SetFlag = INFO_BoolFromString(a_Value);
	
		// Find field
		Bit = 0;
		Ref = &Touch->Flags;
	
		// Find flag in flag fields
		for (i = 0; c_TouchFlags[i].Name; i++)
			if (strcasecmp(c_TouchFlags[i].Name, a_Field) == 0)
			{
				Bit = c_TouchFlags[i].Field;
				break;
			}
	
		// Bit found?, Either set or unset
		if (Bit && Ref)
			if (SetFlag)
				*Ref |= Bit;
			else
				*Ref &= ~Bit;
	}
	
	/* Changing Name? */
	else if (strcasecmp("-", a_ValEnt->Name) == 0)
		strncpy(Touch->SpriteName, a_Value, 4);
	
	/* Message? */
	else if (strcasecmp("Message", a_ValEnt->Name) == 0)
	{
		// Locate i18n string
		Touch->PickupMsgRef = DS_FindStringRef(a_Value);
		
		// Not found? Must be explicit
		if (!Touch->PickupMsgRef)
		{
			Touch->PickupMsgFaked = Z_StrDup(a_Value, PU_REMOODAT, NULL);
			Touch->PickupMsgRef = &Touch->PickupMsgFaked;
		}
	}
}

/* INFO_FrameNextGoto() -- Determine frame to go to */
void INFO_FrameNextGoto(void** const a_Data, struct INFO_REMOODATValEntry_s* a_ValEnt, const char* const a_Field, const char* const a_Value, void* const a_WriteP)
{
	INFO_DataStore_t** StorePP;
	INFO_DataStore_t* This;
	uint8_t IOSG, For;
	uint32_t FrameID, ObjectID;
	int32_t i;
	
	/* Storage Pointer */
	StorePP = a_Data;
	if (StorePP)
		This = *StorePP;
	
	/* Determine Base */
	ObjectID = This->Parent->Parent->MoType;
	IOSG = This->Parent->StateGroup;
	FrameID = This->Cur.StateP->FrameID;
	For = This->Parent->FrameFor;
	
	/* Goto? */
	if (strcasecmp(a_Field, "Goto") == 0)
	{
		// Change IOSG
		for (i = 0; i < NUMSTATEGROUPS; i++)
			if (c_StateGroups[i].For == For)
				if (strcasecmp(c_StateGroups[i].Name, a_Value) == 0)
				{
					IOSG = c_StateGroups[i].ID;
					break;
				}
		
		// Use the first frame
		FrameID = 1;
	}
	
	/* Next? */
	else if (strcasecmp(a_Field, "Next") == 0)
	{
		// Change Frame
		FrameID = C_strtou32(a_Value, NULL, 10);
	}
	
	/* Unknown */
	else
		return;
	
	/* Going zero? */
	if (!FrameID)
	{
		*((uint64_t*)a_WriteP) = 0;
		return;
	}
	
	/* Mod to zero */
	FrameID--;
	
	/* Find Next */
	// Build simulated next (SimNext)
	*((uint64_t*)a_WriteP) = 0;
	
	// Group it belongs to and the wanted frame
	*((uint64_t*)a_WriteP) = ((uint64_t)IOSG) << UINT64_C(16);
	*((uint64_t*)a_WriteP) |= ((uint64_t)FrameID) & UINT64_C(0xFFFF);
	
	// Object
	*((uint64_t*)a_WriteP) |= ((uint64_t)ObjectID) << UINT64_C(32);
}

/* INFO_FrameMisc() -- Load misc into state */
void INFO_FrameMisc(void** const a_Data, struct INFO_REMOODATValEntry_s* a_ValEnt, const char* const a_Field, const char* const a_Value, void* const a_WriteP)
{
	INFO_DataStore_t** StorePP;
	INFO_DataStore_t* This;
	
	/* Storage Pointer */
	StorePP = a_Data;
	if (StorePP)
		This = *StorePP;
	
	/* Sprite */
	if (strcasecmp(a_Field, "Sprite") == 0)
		strncpy(This->Cur.StateP->HoldSprite, a_Value, 4);
	
	/* Frame */
	else if (strcasecmp(a_Field, "Frame") == 0)
	{
		This->Cur.StateP->frame &= FF_FRAMEMASK;
		
		// DECORATE-like
		if (toupper(a_Value[0]) >= 'A' && toupper(a_Value[1]) <= 'Z')
			This->Cur.StateP->frame |= (toupper(a_Value[0]) - 'A') & FF_FRAMEMASK;
		else
			This->Cur.StateP->frame |= C_strtou32(a_Value, NULL, 10) & FF_FRAMEMASK;
	}
	
	/* Full Bright */
	else if (strcasecmp(a_Field, "FullBright") == 0)
	{
		This->Cur.StateP->frame &= ~FF_FULLBRIGHT;
		
		if (INFO_BoolFromString(a_Value))
			This->Cur.StateP->frame |= FF_FULLBRIGHT;
	}
	
	/* Transparency */
	else if (strcasecmp(a_Field, "Transparency") == 0)
	{
		This->Cur.StateP->frame &= ~FF_TRANSMASK;
		This->Cur.StateP->frame |= (INFO_TransparencyByName(a_Value) << FF_TRANSSHIFT) & FF_TRANSMASK;
	}
	
	/* Priority */
	else if (strcasecmp(a_Field, "Priority") == 0)
	{
		This->Cur.StateP->Priority = INFO_PriorityByName(a_Value);
	}
}


void INFO_FrameNum(void** const a_Data, struct INFO_REMOODATValEntry_s* a_ValEnt, const char* const a_Field, const char* const a_Value, void* const a_WriteP);
void INFO_FrameTrans(void** const a_Data, struct INFO_REMOODATValEntry_s* a_ValEnt, const char* const a_Field, const char* const a_Value, void* const a_WriteP);

/* INFO_REMOODATKeyer() -- Keyer for REMOODAT */
bool_t INFO_REMOODATKeyer(void** a_DataPtr, const int32_t a_Stack, const D_RMODCommand_t a_Command, const char* const a_Field, const char* const a_Value)
{
	INFO_DataStore_t** StorePP;
	INFO_DataStore_t* This;
	INFO_REMOODATValEntry_t* ValEnt;
	mobjinfo_t* ThisMT;
	PI_wep_t* ThisWep;
	P_RMODTouchSpecial_t* ThisTC;
	void* DataP;
	int32_t i, j, k, l;
	uint32_t RefToFind;
	
	/* Storage Pointer */
	StorePP = a_DataPtr;
	if (StorePP)
		This = *StorePP;
	
	/* Which Command? */
	switch (a_Command)
	{
			// Opening {
		case DRC_OPEN:
			// Look through chains
			for (i = 0; c_INFOChains[i].Name; i++)
				if (c_INFOChains[i].ValidDepth == a_Stack)
					if (!This || (This && (This->CurrentKey->Bits & c_INFOChains[i].Bits) != 0))
					if (strcasecmp(c_INFOChains[i].Name, a_Field) == 0)
						break;
			
			// Create data
			This = Z_Malloc(sizeof(*This), PU_STATIC, NULL);
			This->Parent = *StorePP;
			*StorePP = This;
			
			// Setup Data Store
			This->CurrentKey = &c_INFOChains[i];
			This->FrameFor = This->CurrentKey->For;
			
			if (This->CurrentKey->GrabEntry)
				This->Cur.vP = This->CurrentKey->GrabEntry(a_DataPtr, a_Value);
			
			// First table value is a -?
			if (This->CurrentKey->Table)
				if (This->CurrentKey->Table[0].Name)
					if (strncasecmp("-", This->CurrentKey->Table[0].Name, 1) == 0)
						INFO_REMOODATKeyer(a_DataPtr, a_Stack, DRC_DATA, This->CurrentKey->Table[0].Name, a_Value);
			
			return true;
			
			// Closing }
		case DRC_CLOSE:
			// Pop?
			if (This)
			{
				(*StorePP) = This->Parent;
				Z_Free(This);
			}
			
			return true;
			
			// Data Entry
		case DRC_DATA:
			// No data?
			if (!This || (This && !This->Cur.vP))
				return true;
			
			// No lookup table?
			if (!This->CurrentKey->Table)
				return true;
			
			// Find Value Entry
			for (i = 0; This->CurrentKey->Table[i].Name; i++)
				if (strcasecmp(a_Field, This->CurrentKey->Table[i].Name) == 0 || This->CurrentKey->Table[i].Name[0] == '?')
					break;
			
			// Current value entry
			ValEnt = &This->CurrentKey->Table[i];
			
			// Only if a value exists
			if (ValEnt && ValEnt->Name)
			{
				// Get Offset
				DataP = (void*)(((uintptr_t)This->Cur.InfoP) + ValEnt->Offset);
				
				// Which Type
				switch (ValEnt->Type)
				{
						// Integer
					case IRVT_INT32:
						*((int32_t*)DataP) = C_strtoi32(a_Value, NULL, 10);
						break;
						
						// Unsigned Integer
					case IRVT_UINT32:
						*((uint32_t*)DataP) = C_strtou32(a_Value, NULL, 10);
						break;
						
						// String
					case IRVT_STRING:
						*((char**)DataP) = Z_StrDup(a_Value, PU_REMOODAT, NULL);
						break;
						
						// Fixed
					case IRVT_FIXED:
						*((int32_t*)DataP) = C_strtofx(a_Value, NULL);
						break;
						
						// Function
					case IRVT_FUNC:
						if (ValEnt->Func)
							ValEnt->Func(a_DataPtr, ValEnt, a_Field, a_Value, DataP);
						break;
						
						// Unknown
					default:
						break;
				}
			}
			
			return true;
			
			// Initialize
		case DRC_INIT:
			return true;
			
			// Finalize
		case DRC_FINAL:
			return true;
			
			// First Time
		case DRC_FIRST:
			// Do full purge of REMOODAT allocations
			Z_FreeTags(PU_REMOODAT, PU_REMOODAT);
			
			// Clear key mappings
			l_KeysMapped = false;
			memset(l_KeyMap, 0, sizeof(l_KeyMap));
			
			// Do other unsets, as needed
			NUMMOBJTYPES = 0;
			mobjinfo = NULL;
			sprnames = NULL;
			NUMSPRITES = 0;
			states = NULL;
			NUMSTATES = 0;
			wpnlev1info = 0;
			wpnlev2info = 0;
			NUMWEAPONS = 0;
			ammoinfo = NULL;
			NUMAMMO = 0;
			g_RMODNumTouchSpecials = 0;
			g_RMODTouchSpecials = NULL;
			g_RMODNumKeys = 0;
			g_RMODKeys = NULL;
			
			// Create initial S_NULL
			Z_ResizeArray((void**)&states, sizeof(*states),
				NUMSTATES, NUMSTATES + 1);
			Z_ChangeTag(states, PU_REMOODAT);
			states[NUMSTATES++] = Z_Malloc(sizeof(**states), PU_REMOODAT, NULL);
			return true;
			
			// Last Time
		case DRC_LAST:
			// Normalize all states (reference them, etc.)
			INFO_StateNormalize(0, NUMSTATES);
			
			// Weapons
			wpnlev2info = wpnlev1info;
			for (i = 0; i < NUMWEAPONS; i++)
			{
				// Get
				ThisWep = wpnlev1info[i];
				
				// Missing?
				if (!ThisWep)
					continue;
				
				// Get ammo type
				ThisWep->ammo = INFO_GetAmmoByName(ThisWep->AmmoClass);
				
				// Determine flash states
				RefToFind = ((PWSG_FLASH & UINT32_C(0xFFFF)) << UINT32_C(16));
				
				// Determine flash order
				for (k = 0; k < NUMSTATES; k++)
					if (states[k]->ObjectID == ThisWep->WeaponID)
						if ((states[k]->Marker & UINT32_C(0xFFFF0000)) == RefToFind)
						{
							// No func?
							if (!states[k]->action.acv)
								continue;
							
							// Function only on A_Light*()
							if (states[k]->action.acv != A_Light1 &&
								states[k]->action.acv != A_Light2)
								continue;
							
							// Add to array
							Z_ResizeArray((void**)&ThisWep->FlashStates,
									sizeof(*ThisWep->FlashStates),
									ThisWep->NumFlashStates,
									ThisWep->NumFlashStates + 1
								);
							ThisWep->FlashStates[ThisWep->NumFlashStates++] = k;
							Z_ChangeTag(ThisWep->FlashStates, PU_REMOODAT);
						}
				
				// Sort Flash states
				for (j = 0; j < ThisWep->NumFlashStates; j++)
				{
					// Find one with lowest value
					for (l = j, k = j + 1; k < ThisWep->NumFlashStates; k++)
						if (states[ThisWep->FlashStates[k]]->FrameID < states[ThisWep->FlashStates[l]]->FrameID)
							l = k;
					
					// Not the same?
					if (j != l)
					{
						k = ThisWep->FlashStates[j];
						ThisWep->FlashStates[j] = ThisWep->FlashStates[l];
						ThisWep->FlashStates[l] = k;
					}
				}
			}
			
			// Handle all classes loaded
			for (i = 0; i < NUMMOBJTYPES; i++)
			{
				// Get
				ThisMT = mobjinfo[i];
				
				// Missing?
				if (!ThisMT)
					continue;
				
				// Hash Name
				ThisMT->RQuickHash[0] = Z_Hash(ThisMT->RClassName);
				if (ThisMT->RMTName)
					ThisMT->RQuickHash[1] = Z_Hash(ThisMT->RMTName);
				
				// Family object belongs to
				if (ThisMT->RFamilyClass)
				{
					ThisMT->RBaseFamily = INFO_GetTypeByName(ThisMT->RFamilyClass);
					Z_Free(ThisMT->RFamilyClass);
					ThisMT->RFamilyClass = NULL;
					
					// Illegal?
					if (ThisMT->RBaseFamily >= NUMMOBJTYPES)
						ThisMT->RBaseFamily = ThisMT->ObjectID;
				}
				else
					ThisMT->RBaseFamily = ThisMT->ObjectID;
			}
			
			// Touch Specials (not that trivial)
			for (i = 0; i < g_RMODNumTouchSpecials; i++)
			{
				// Get
				ThisTC = g_RMODTouchSpecials[i];
				
				// Missing?
				if (!ThisTC)
					continue;
					
				// Init Fields (they changed)
				ThisTC->ActSpriteNum = NUMSPRITES;
				ThisTC->ActGiveWeapon = NUMWEAPONS;
				ThisTC->ActGiveAmmo = NUMAMMO;
				ThisTC->ActGiveKey = g_RMODNumKeys;
				
				// Reference Weapon
				if (ThisTC->GiveWeapon)
				{
					ThisTC->ActGiveWeapon = INFO_GetWeaponByName(ThisTC->GiveWeapon);
					Z_Free(ThisTC->GiveWeapon);
					ThisTC->GiveWeapon = NULL;
				}
				
				// Reference Ammo
				if (ThisTC->GiveAmmo)
				{
					ThisTC->ActGiveAmmo = INFO_GetAmmoByName(ThisTC->GiveAmmo);
					Z_Free(ThisTC->GiveAmmo);
					ThisTC->GiveAmmo = NULL;
				}
				
				// Reference Key
				if (ThisTC->GiveKey)
				{
					ThisTC->ActGiveKey = INFO_GetKeyByName(ThisTC->GiveKey);
					Z_Free(ThisTC->GiveKey);
					ThisTC->GiveKey = NULL;
				}
			
				// Find sprite to map to
				for (j = 0; j < 4 && ThisTC->SpriteName[j]; j++)
					ThisTC->ActSpriteID |= ((uint32_t)toupper(ThisTC->SpriteName[j])) << (j * 8);
			}
			
			// All-done!
			return true;
		
		default:
			return false;
	}
}

/*****************************************************************************/

/* INFO_BoolFromString() -- Returns bool from string value */
bool_t INFO_BoolFromString(const char* const a_String)
{
	/* Check */
	if (!a_String)
		return false;
		
	/* Only a certain subset are true */
	if (C_strtoi32(a_String, NULL, 10) != 0 ||
		strcasecmp(a_String, "true") == 0 ||
		strcasecmp(a_String, "yes") == 0 ||
		strcasecmp(a_String, "on") == 0 ||
		strcasecmp(a_String, "set") == 0 ||
		strcasecmp(a_String, "enabled") == 0)
		return true;
	
	/* False */
	return false;
}

/* INFO_GetTypeByName() -- Returns a map object by it's name */
mobjtype_t INFO_GetTypeByName(const char* const a_Name)
{
	size_t i = 0;
	uint32_t Hash;
	
	/* Check */
	if (!a_Name)
		return NUMMOBJTYPES;
	
	/* Hash String */
	Hash = Z_Hash(a_Name);
	
	/* Go through class list */
	// By Class Name
	for (i = 0; i < NUMMOBJTYPES; i++)
		if (mobjinfo[i]->RClassName)
			if (!mobjinfo[i]->RQuickHash[0] || Hash == mobjinfo[i]->RQuickHash[0])
				if (strcasecmp(a_Name, mobjinfo[i]->RClassName) == 0)
					return i;
			
	// By MT Name
	for (i = 0; i < NUMMOBJTYPES; i++)
		if (mobjinfo[i]->RMTName)
			if (!mobjinfo[i]->RQuickHash[1] || Hash == mobjinfo[i]->RQuickHash[1])
				if (strcasecmp(a_Name, mobjinfo[i]->RMTName) == 0)
					return i;
	
	/* Not found? */
	return NUMMOBJTYPES;
}

/* INFO_SpriteNumByName() -- Determine sprite ID by name */
spritenum_t INFO_SpriteNumByName(const char* const a_Name, bool_t a_Create)
{
	size_t i;
	
	/* Check */
	if (!a_Name)
		return 0;
	
	/* Determine sprite */
	for (i = 0; i < NUMSPRITES; i++)
		if (strcasecmp(a_Name, sprnames[i]) == 0)
			return i;
	
	/* Append to end */
	if (a_Create)
	{
		i = NUMSPRITES++;
		Z_ResizeArray((void**)&sprnames, sizeof(*sprnames), i, NUMSPRITES);
		sprnames[i] = Z_Malloc(5, PU_REMOODAT, NULL);
		strncpy(sprnames[i], a_Name, 4);
		
		// Return the freshly added one
		return i;
	}
		
	/* Not found? */
	return 0;
}

/* INFO_FunctionPtrByName() -- Get action pointer by name */
actionf_t INFO_FunctionPtrByName(const char* const a_Name)
{
	size_t i;	
	
	static const struct
	{
		const char* Name;						// Name of function
		actionf_t Action;						// Defined Action
	} c_FuncPs[] =
	{
		{NULL, {NULL}},
		{"Light0", {A_Light0}},
		{"WeaponReady", {A_WeaponReady}},
		{"Lower", {A_Lower}},
		{"Raise", {A_Raise}},
		{"Punch", {A_Punch}},
		{"ReFire", {A_ReFire}},
		{"FirePistol", {A_FirePistol}},
		{"Light1", {A_Light1}},
		{"FireShotgun", {A_FireShotgun}},
		{"Light2", {A_Light2}},
		{"FireShotgun2", {A_FireShotgun2}},
		{"CheckReload", {A_CheckReload}},
		{"OpenShotgun2", {A_OpenShotgun2}},
		{"LoadShotgun2", {A_LoadShotgun2}},
		{"CloseShotgun2", {A_CloseShotgun2}},
		{"FireCGun", {A_FireCGun}},
		{"GunFlash", {A_GunFlash}},
		{"FireMissile", {A_FireMissile}},
		{"Saw", {A_Saw}},
		{"FirePlasma", {A_FirePlasma}},
		{"BFGsound", {A_BFGsound}},
		{"FireBFG", {A_FireBFG}},
		{"BFGSpray", {A_BFGSpray}},
		{"Explode", {A_Explode}},
		{"Pain", {A_Pain}},
		{"PlayerScream", {A_PlayerScream}},
		{"Fall", {A_Fall}},
		{"XScream", {A_XScream}},
		{"Look", {A_Look}},
		{"Chase", {A_Chase}},
		{"FaceTarget", {A_FaceTarget}},
		{"PosAttack", {A_PosAttack}},
		{"Scream", {A_Scream}},
		{"SPosAttack", {A_SPosAttack}},
		{"VileChase", {A_VileChase}},
		{"VileStart", {A_VileStart}},
		{"VileTarget", {A_VileTarget}},
		{"VileAttack", {A_VileAttack}},
		{"StartFire", {A_StartFire}},
		{"Fire", {A_Fire}},
		{"FireCrackle", {A_FireCrackle}},
		{"Tracer", {A_Tracer}},
		{"SkelWhoosh", {A_SkelWhoosh}},
		{"SkelFist", {A_SkelFist}},
		{"SkelMissile", {A_SkelMissile}},
		{"FatRaise", {A_FatRaise}},
		{"FatAttack1", {A_FatAttack1}},
		{"FatAttack2", {A_FatAttack2}},
		{"FatAttack3", {A_FatAttack3}},
		{"BossDeath", {A_BossDeath}},
		{"CPosAttack", {A_CPosAttack}},
		{"CPosRefire", {A_CPosRefire}},
		{"TroopAttack", {A_TroopAttack}},
		{"SargAttack", {A_SargAttack}},
		{"HeadAttack", {A_HeadAttack}},
		{"BruisAttack", {A_BruisAttack}},
		{"SkullAttack", {A_SkullAttack}},
		{"Metal", {A_Metal}},
		{"SpidRefire", {A_SpidRefire}},
		{"BabyMetal", {A_BabyMetal}},
		{"BspiAttack", {A_BspiAttack}},
		{"Hoof", {A_Hoof}},
		{"CyberAttack", {A_CyberAttack}},
		{"PainAttack", {A_PainAttack}},
		{"PainDie", {A_PainDie}},
		{"KeenDie", {A_KeenDie}},
		{"BrainPain", {A_BrainPain}},
		{"BrainScream", {A_BrainScream}},
		{"BrainDie", {A_BrainDie}},
		{"BrainAwake", {A_BrainAwake}},
		{"BrainSpit", {A_BrainSpit}},
		{"SpawnSound", {A_SpawnSound}},
		{"SpawnFly", {A_SpawnFly}},
		{"BrainExplode", {A_BrainExplode}},
		{"SmokeTrailer", {A_SmokeTrailer}},
		{"SmokeTrailerRocket", {A_SmokeTrailerRocket}},
		{"SmokeTrailerSkull", {A_SmokeTrailerSkull}},
		{"FireOldBFG", {A_FireOldBFG}},
		{"FireGenericProjectile", {A_FireGenericProjectile}},
		{"NextFrameIfMoving", {A_NextFrameIfMoving}},
		{"GenericMonsterMissile", {A_GenericMonsterMissile}},
		{NULL, {NULL}},
	};
	
	/* Check */
	if (!a_Name)
		return c_FuncPs[0].Action;
	
	/* Go through list */
	for (i = 1; c_FuncPs[i].Name; i++)
		if (!strcasecmp(c_FuncPs[i].Name, a_Name))
			return c_FuncPs[i].Action;
	
	/* Not found */
	return c_FuncPs[i].Action;
}

/* INFO_PriorityByName() -- Priority by name */
int INFO_PriorityByName(const char* const a_Name)
{
	/* Check */
	if (!a_Name)
		return STP_DEFAULT;
	
	/* Compare */
	if (strcasecmp(a_Name, "Null") == 0) return STP_NULL;
	else if (strcasecmp(a_Name, "Default") == 0) return STP_DEFAULT;
	else if (strcasecmp(a_Name, "Weapons") == 0) return STP_WEAPON;
	else if (strcasecmp(a_Name, "Ammo") == 0) return STP_AMMO;
	else if (strcasecmp(a_Name, "WeaponFlash") == 0) return STP_WEPFLASH;
	else if (strcasecmp(a_Name, "Effects") == 0) return STP_EFFECTS;
	else if (strcasecmp(a_Name, "Monsters") == 0) return STP_MONSTERS;
	else if (strcasecmp(a_Name, "Corpses") == 0) return STP_CORPSES;
	else if (strcasecmp(a_Name, "Players") == 0) return STP_PLAYERS;
	else if (strcasecmp(a_Name, "Health") == 0) return STP_HEALTH;
	else if (strcasecmp(a_Name, "Cookies") == 0) return STP_COOKIES;
	else if (strcasecmp(a_Name, "MissionCritical") == 0) return STP_MISSIONCRITICAL;
	else if (strcasecmp(a_Name, "Powerups") == 0) return STP_POWERUPS;
	else if (strcasecmp(a_Name, "Decorations") == 0) return STP_DECORATIONS;
	else if (strcasecmp(a_Name, "Projectiles") == 0) return STP_PROJECTILES;
	
	/* Not Found */
	return STP_DEFAULT;
}

/* INFO_TransparencyByName() -- Return transparency for name */
uint32_t INFO_TransparencyByName(const char* const a_Name)
{
	int32_t TransNum;
	
	/* Check */
	if (!a_Name)
		return 0;
	
	/* Check name first */
	if (strcasecmp(a_Name, "Fire") == 0)
		return VEX_TRANSFIRE;
	else if (strcasecmp(a_Name, "Effect1") == 0)
		return VEX_TRANSFX1;
	
	/* Compare Number */
	TransNum = atoi(a_Name);
	
	// Round up?
	if ((TransNum % 10) >= 5)
		TransNum += 5;
	
	// Shrink
	TransNum /= 10;
	
	// Limit
	if (TransNum < 0)
		TransNum = 0;
	if (TransNum > 10)
		TransNum = 10;
	
	// Return
	return TransNum;
}

/* INFO_BotMetricByName() -- Returns metric by name */
INFO_BotObjMetric_t INFO_BotMetricByName(const char* const a_Name)
{
	/* Check */
	if (!a_Name)
		return INFOBM_DEFAULT;
	
	/* Compare */
	if (strcasecmp(a_Name, "Default") == 0) return INFOBM_DEFAULT;
	else if (strcasecmp(a_Name, "DoomPlayer") == 0) return INFOBM_DOOMPLAYER;
	else if (strcasecmp(a_Name, "WeakMonster") == 0) return INFOBM_WEAKMONSTER;
	else if (strcasecmp(a_Name, "NormalMonster") == 0) return INFOBM_NORMALMONSTER;
	else if (strcasecmp(a_Name, "StrongMonster") == 0) return INFOBM_STRONGMONSTER;
	else if (strcasecmp(a_Name, "ArchVile") == 0) return INFOBM_ARCHVILE;
	else if (strcasecmp(a_Name, "Projectile") == 0) return INFOBM_PROJECTILE;
	else if (strcasecmp(a_Name, "Barrel") == 0) return INFOBM_BARREL;
	else if (strcasecmp(a_Name, "LightArmor") == 0) return INFOBM_LIGHTARMOR;
	else if (strcasecmp(a_Name, "HeavyArmor") == 0) return INFOBM_HEAVYARMOR;
	else if (strcasecmp(a_Name, "LightHealth") == 0) return INFOBM_LIGHTHEALTH;
	else if (strcasecmp(a_Name, "HeavyHealth") == 0) return INFOBM_HEAVYHEALTH;
	else if (strcasecmp(a_Name, "KeyCard") == 0) return INFOBM_KEYCARD;
	else if (strcasecmp(a_Name, "Ammo") == 0) return INFOBM_AMMO;
	else if (strcasecmp(a_Name, "Weapon") == 0) return INFOBM_WEAPON;
	
	else if (strcasecmp(a_Name, "Melee") == 0) return INFOBM_WEAPONMELEE;
	else if (strcasecmp(a_Name, "MidRange") == 0) return INFOBM_WEAPONMIDRANGE;
	else if (strcasecmp(a_Name, "LayDown") == 0) return INFOBM_WEAPONLAYDOWN;
	else if (strcasecmp(a_Name, "SprayPlasma") == 0) return INFOBM_SPRAYPLASMA;
	else if (strcasecmp(a_Name, "BFG") == 0) return INFOBM_WEAPONBFG;
	else if (strcasecmp(a_Name, "SSGDance") == 0) return INFOBM_WEAPONSSGDANCE;
	
	/* Unknown */
	return INFOBM_DEFAULT;
}


/* P_RMODTouchSpecialByString() -- Find touch special by string */
P_TouchNum_t P_RMODTouchSpecialByString(const char* const a_String)
{
	P_TouchNum_t i;
	
	/* Check */
	if (!a_String)
		return g_RMODNumTouchSpecials;
	
	/* Go through list */
	for (i = 0; i < g_RMODNumTouchSpecials; i++)
		if (strncasecmp(a_String, g_RMODTouchSpecials[i]->SpriteName, 4) == 0)
			return i;
	
	/* Not Found */
	return g_RMODNumTouchSpecials;
}

/* P_RMODTouchSpecialForSprite() -- Find touch special via sprite */
P_RMODTouchSpecial_t* P_RMODTouchSpecialForSprite(const uint32_t a_SprNum)
{
	/* Check */
	if (a_SprNum < 0 || a_SprNum >= NUMSPRITES)
		return NULL;
	
	/* Return preknown array */
	return g_SprTouchSpecials[a_SprNum];
}

/* P_RMODTouchSpecialForCode() -- Find touch special via code */
P_RMODTouchSpecial_t* P_RMODTouchSpecialForCode(const uint32_t a_Code)
{
	size_t i;
	P_RMODTouchSpecial_t* Current;
	
	/* Look through list */
	for (i = 0; i < g_RMODNumTouchSpecials; i++)
	{
		// Get current
		Current = g_RMODTouchSpecials[i];
		
		// Got it?
		if (Current->ActSpriteID == a_Code)
			return Current;
	}
	
	/* Not Found */
	return NULL;
}

/* INFO_GetKeyByName() -- Returns key by class name */
P_KeyNum_t INFO_GetKeyByName(const char* const a_Name)
{
	P_KeyNum_t i;
	
	/* Check */
	if (!a_Name)
		return g_RMODNumKeys;
	
	/* Search by name */
	for (i = 0; i < g_RMODNumKeys; i++)
		if (g_RMODKeys[i])
			if (strcasecmp(g_RMODKeys[i]->ClassName, a_Name) == 0)
				return i;
	
	/* Not found */
	return g_RMODNumKeys;
}

/* INFO_KeyByGroupBit() -- Returns the associated key by group and bit */
P_RMODKey_t* INFO_KeyByGroupBit(const uint32_t a_Group, const uint32_t a_Bit)
{
	size_t i;
	P_RMODKey_t* Key;
	
	/* If not mapped, generate mappings */
	if (!l_KeysMapped)
	{
		// Run through all keys
		for (i = 0; i < g_RMODNumKeys; i++)
		{
			// Get key
			Key = g_RMODKeys[i];
			
			// Nothing?
			if (!Key)
				continue;
			
			// Set
			if (Key->Group >= 0 && Key->Group < 2)
				if (Key->BitNum >= 0 && Key->BitNum < 32)
					l_KeyMap[Key->Group][Key->BitNum] = Key;
		}
		
		// Set as mapped
		l_KeysMapped = true;
	}	
	
	/* Check */
	if (a_Group < 0 || a_Group >= 2 || a_Bit	< 0 || a_Bit >= 32)
		return NULL;
	
	/* Return the mapped keys */
	return l_KeyMap[a_Group][a_Bit];
bool_t l_KeysMapped = false;					// Keys Mapped
P_RMODKey_t* l_KeyMap[2][32];					// Quick Key Mapping
}

