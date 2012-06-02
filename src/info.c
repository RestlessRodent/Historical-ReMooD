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
//      Thing frame/state LUT,
//      generated by multigen utilitiy.
//      This one is the original DOOM version, preserved.

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

char** sprnames = NULL;
void** g_SprTouchSpecials = NULL;				// Sprite touch special markers
size_t NUMSPRITES = 0;
state_t** states = 0;
size_t NUMSTATES = 0;
mobjinfo_t** mobjinfo = NULL;
mobjtype_t NUMMOBJTYPES = 0;

#define LOCALSTATEJUMPS						64	// Local State Jumping

/* INFO_LocalObjects_t -- Local map objects */
typedef struct INFO_LocalObjects_s
{
	// Actual map objects
	mobjinfo_t** Objects;
	size_t NumObjects;
	
	// States used by defined objects
#if defined(__REMOOD_USEFLATTERSTATES)
	state_t* ObjectStates;
#else
	state_t** ObjectStates;
#endif
	size_t NumObjectStates;
	size_t MaxObjectStates;
} INFO_LocalObjects_t;

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
	{0, NULL},
};

// c_ObjectLocalStates -- Object local states
static const struct
{
	INFO_ObjectStateGroup_t IOSG;
	const char* Name;
	ptrdiff_t RefDiff;
} c_ObjectLocalStates[] =
{
	{IOSG_SPAWN, "SpawnState", offsetof(mobjinfo_t,spawnstate)},
	{IOSG_ACTIVE, "ActiveState", offsetof(mobjinfo_t,seestate)},
	{IOSG_PAIN, "PainState", offsetof(mobjinfo_t,painstate)},
	{IOSG_MELEEATTACK, "MeleeAttackState", offsetof(mobjinfo_t,meleestate)},
	{IOSG_RANGEDATTACK, "RangedAttackState", offsetof(mobjinfo_t,missilestate)},
	{IOSG_CRASH, "CrashState", offsetof(mobjinfo_t,crashstate)},
	{IOSG_DEATH, "DeathState", offsetof(mobjinfo_t,deathstate)},
	{IOSG_GIB, "GibState", offsetof(mobjinfo_t,xdeathstate)},
	{IOSG_RAISE, "RaiseState", offsetof(mobjinfo_t,raisestate)},
	{IOSG_PLAYERRUN, "PlayerRunState", offsetof(mobjinfo_t,RPlayerRunState)},
	{IOSG_PLAYERMELEE, "PlayerMeleeAttackState", offsetof(mobjinfo_t,RPlayerMeleeAttackState)},
	{IOSG_PLAYERRANGED, "PlayerRangedAttackState", offsetof(mobjinfo_t,RPlayerRangedAttackState)},
	{IOSG_VILEHEAL, "VileHealState", offsetof(mobjinfo_t,RVileHealState)},
	{IOSG_LESSBLOODA, "LessLessBloodState", offsetof(mobjinfo_t,RLessBlood[0])},
	{IOSG_LESSBLOODB, "LessMoreBloodState", offsetof(mobjinfo_t,RLessBlood[1])},
	{IOSG_BRAINEXPLODE, "BrainExplodeState", offsetof(mobjinfo_t,RBrainExplodeState)},
	{IOSG_MELEEPUFF, "MeleePuffState", offsetof(mobjinfo_t,RMeleePuffState)},

	{0, NULL, 0},
};

/* INFO_RMODObjectStateForName() -- Get information needed for state handling */
static statenum_t* INFO_RMODObjectStateForName(void* const a_Input, const char* const a_Name, INFO_ObjectStateGroup_t* const IOSG, uint32_t** const a_RefState, uint32_t*** const a_LRefs, size_t* a_NumLRefs)
{
	mobjinfo_t* Object = a_Input;
	size_t i;
	
	/* Check */
	if (!a_Input || !a_Name)
		return NULL;
	
	/* Go through list */
	for (i = 0; c_ObjectLocalStates[i].Name; i++)
		if (strcasecmp(a_Name, c_ObjectLocalStates[i].Name) == 0)
		{
			*IOSG = c_ObjectLocalStates[i].IOSG;
			if (a_RefState)
				*a_RefState = &Object->RefStates[*IOSG];
			return (void*)(((uintptr_t)a_Input) + c_ObjectLocalStates[i].RefDiff);
		}
	
	/* Not Found */
	*IOSG = -1;
	if (a_RefState)
		*a_RefState = NULL;
	return NULL;
}

/* INFO_RMODH_MapObjects() -- Parser for map objects */
bool_t INFO_RMODH_MapObjects(Z_Table_t* const a_Table, const WL_WADFile_t* const a_WAD, const D_RMODPrivates_t a_ID, D_RMODPrivate_t* const a_Private)
{
	static uint32_t ObjectIDBase;
	INFO_LocalObjects_t* LocalStuff;
	INFO_RMODStateHelper_t Helper;
	mobjinfo_t ThisObject;
	size_t i;
	const char* Value;
	
	/* Check */
	if (!a_Table || !a_WAD || !a_Private)
		return false;
	
	/* Init */
	memset(&ThisObject, 0, sizeof(ThisObject));
	
	/* Create specials list */
	// Does not exist
	if (!a_Private->Data)
	{
		a_Private->Size = sizeof(*LocalStuff);
		a_Private->Data = Z_Malloc(a_Private->Size, PU_WLDKRMOD, (void**)&a_Private->Data);
	}
	
	// Get the local stuff
	LocalStuff = a_Private->Data;
	
	/* Get ClassName */
	Value = Z_TableName(a_Table);
	
	// Knock off #
	Value = strchr(Value, '#');
	
	// Not found?
	if (!Value)
		return false;
	
	// Add 1 to remove #
	Value++;
	
	/* Parse Objects */
	// Class Name
	ThisObject.RClassName = Z_StrDup(Value, PU_WLDKRMOD, NULL);
	
	// Early Boot Notice
	if (g_EarlyBootConsole)
		CONL_EarlyBootTic(Value, true);
	
	// String Values
	ThisObject.RNiceName = D_RMODGetValueString(a_Table, "NiceName", NULL);
	ThisObject.RMTName = D_RMODGetValueString(a_Table, "MTName", NULL);
	ThisObject.RDropClass = D_RMODGetValueString(a_Table, "DropsClass", NULL);
	ThisObject.RFamilyClass = D_RMODGetValueString(a_Table, "BaseFamily", NULL);
	ThisObject.RBrainExplodeThing = D_RMODGetValueString(a_Table, "BrainExplodeClass", NULL);
	ThisObject.RMissileSplat = D_RMODGetValueString(a_Table, "MissileSplat", NULL);
	ThisObject.RBloodSplat = D_RMODGetValueString(a_Table, "BloodSplat", NULL);
	ThisObject.RBloodSpewClass = D_RMODGetValueString(a_Table, "BloodSpewClass", NULL);
	ThisObject.RGenericMissile = D_RMODGetValueString(a_Table, "GenericMissile", NULL);
	
	// Integer Values
	ThisObject.doomednum = D_RMODGetValueInt(a_Table, "DoomEdNum", 0);
	ThisObject.RDehackEdID = D_RMODGetValueInt(a_Table, "DeHackEdNum", 0);
	ThisObject.spawnhealth = D_RMODGetValueInt(a_Table, "SpawnHealth", 0);
	ThisObject.reactiontime = D_RMODGetValueInt(a_Table, "ReactionTime", 0);
	ThisObject.mass = D_RMODGetValueInt(a_Table, "Mass", 0);
	ThisObject.damage = D_RMODGetValueInt(a_Table, "Damage", 0);
	ThisObject.RCapMissileDist = D_RMODGetValueInt(a_Table, "CapMissileDist", 0);
	ThisObject.RMissileDist[0] = D_RMODGetValueInt(a_Table, "MinMissileDist", 0);
	ThisObject.RMissileDist[1] = D_RMODGetValueInt(a_Table, "MaxMissileDist", 0);
	
	// Fixed Values
	ThisObject.speed = D_RMODGetValueFixed(a_Table, "Speed", 0);
	ThisObject.RFastSpeed = D_RMODGetValueFixed(a_Table, "FastSpeed", 0);
	ThisObject.radius = D_RMODGetValueFixed(a_Table, "Radius", 0);
	ThisObject.height = D_RMODGetValueFixed(a_Table, "Height", 0);
	ThisObject.painchance = FixedMul(D_RMODGetValueFixed(a_Table, "PainChance", 0), (255 << FRACBITS)) >> FRACBITS;
	ThisObject.RBounceFactor = D_RMODGetValueFixed(a_Table, "BounceFactor", (1 << FRACBITS));
	
	ThisObject.RAirGravity = D_RMODGetValueFixed(a_Table, "AirGravity", (1 << FRACBITS));
	ThisObject.RWaterGravity = D_RMODGetValueFixed(a_Table, "WaterGravity", (1 << FRACBITS));
	
	// Sounds
	ThisObject.RSeeSound = D_RMODGetValueString(a_Table, "WakeSound", NULL);;
	ThisObject.RAttackSound = D_RMODGetValueString(a_Table, "AttackSound", NULL);;;
	ThisObject.RPainSound = D_RMODGetValueString(a_Table, "PainSound", NULL);;;
	ThisObject.RDeathSound = D_RMODGetValueString(a_Table, "DeathSound", NULL);;;
	ThisObject.RActiveSound = D_RMODGetValueString(a_Table, "ActiveSound", NULL);;;
	
	// Flags (This is a huge up taking in time and speed)
		// Normal Doom Flags
	for (i = 0; c_xFlags[i].Name; i++)
		if (D_RMODGetValueBool(a_Table, c_xFlags[i].Name, false))
			ThisObject.flags |= c_xFlags[i].Field;
		else
			ThisObject.flags &= ~c_xFlags[i].Field;
			
		// Normal Heretic Flags + Legacy Ones
	for (i = 0; c_xFlagsTwo[i].Name; i++)
		if (D_RMODGetValueBool(a_Table, c_xFlagsTwo[i].Name, false))
			ThisObject.flags2 |= c_xFlagsTwo[i].Field;
		else
			ThisObject.flags2 &= ~c_xFlagsTwo[i].Field;
		
		// ReMooD Extended Group A
	for (i = 0; c_xRXFlagsA[i].Name; i++)
		if (D_RMODGetValueBool(a_Table, c_xRXFlagsA[i].Name, false))
			ThisObject.RXFlags[0] |= c_xRXFlagsA[i].Field;
		else
			ThisObject.RXFlags[0] &= ~c_xRXFlagsA[i].Field;
		
		// ReMooD Extended Group B
	for (i = 0; c_xRXFlagsB[i].Name; i++)
		if (D_RMODGetValueBool(a_Table, c_xRXFlagsB[i].Name, false))
			ThisObject.RXFlags[1] |= c_xRXFlagsB[i].Field;
		else
			ThisObject.RXFlags[1] &= ~c_xRXFlagsB[i].Field;
	
	// Convert Color to Name
	if ((Value = Z_TableGetValue(a_Table, "TranslationColor")))
		// Find translation color
		for (i = 0; i < MAXSKINCOLORS; i++)
			if (strcasecmp(Value, Color_Names[i]) == 0)
			{
				ThisObject.flags |= ((((uint32_t)i) << MF_TRANSSHIFT) & MF_TRANSLATION);
				break;
			}
		
	// Bot Metrics
	if ((Value = Z_TableGetValue(a_Table, "BotMetric")))
		ThisObject.RBotMetric = INFO_BotMetricByName(Value);
	
	// Object ID (semi-unique)
	ThisObject.ObjectID = (((uint32_t)(M_Random() & 0xFF)) | ((++ObjectIDBase) << 8));
	
	// States
	memset(&Helper, 0, sizeof(Helper));
		
	Helper.StatesRef = &LocalStuff->ObjectStates;
	Helper.NumStatesRef = &LocalStuff->NumObjectStates;
	Helper.MaxStatesRef = &LocalStuff->MaxObjectStates;
	Helper.StateForName = INFO_RMODObjectStateForName;
	Helper.InputPtr = &ThisObject;
	Helper.ObjectID = ThisObject.ObjectID;
	
	Z_TableSuperCallback(a_Table, INFO_RMODStateHandlers, (void*)&Helper);
	
	/* Add to end */
	Z_ResizeArray((void**)&LocalStuff->Objects, sizeof(*LocalStuff->Objects), LocalStuff->NumObjects, LocalStuff->NumObjects + 1);
	LocalStuff->Objects[LocalStuff->NumObjects] = Z_Malloc(sizeof(*LocalStuff->Objects[LocalStuff->NumObjects]), PU_WLDKRMOD, NULL);
	memmove(LocalStuff->Objects[LocalStuff->NumObjects], &ThisObject, sizeof(ThisObject));
	LocalStuff->NumObjects++;

	/* Success! */
	return true;
}

static state_t StaticSNull;

/* INFO_RMODO_MapObjects() -- Order for map objects */
bool_t INFO_RMODO_MapObjects(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD, const D_RMODPrivates_t a_ID)
{
	INFO_LocalObjects_t* LocalStuff;
	D_RMODPrivate_t* RMODPrivate;
	const WL_WADFile_t* RoveWAD;
	size_t i, j, k, z, Base, Count, MergeBase, MergeCount;
	mobjtype_t FoundID;
	mobjinfo_t* TempObject;
	uint32_t ObjID, RefToFind;
	statenum_t* StateRef;
	
	/* Reset */
	MergeCount = 0;
	MergeBase = (size_t)-1;
	
	/* Reset States */
	// Clear old states
	if (states)
		Z_Free(states);
	states = NULL;
	NUMSTATES = 0;
	
	// Clear old sprites
	if (sprnames)
		Z_Free(sprnames);
	sprnames = NULL;
	NUMSPRITES = 0;
	
	/* Remove All Objects Pre-Defined */
	if (mobjinfo)
		Z_Free(mobjinfo);
	mobjinfo = NULL;
	NUMMOBJTYPES = 0;
	
	// Add first state, an S_NULL
	Z_ResizeArray((void**)&states, sizeof(*states), NUMSTATES, NUMSTATES + 1);
	states[NUMSTATES++] = &StaticSNull;
	
	/* Go through every WAD */
	for (RoveWAD = WL_IterateVWAD(NULL, true); RoveWAD; RoveWAD = WL_IterateVWAD(RoveWAD, true))
	{
		// Obtain private menu stuff for this WAD
		RMODPrivate = D_GetRMODPrivate(RoveWAD, a_ID);
		
		// Not found? Ignore this WAD then
		if (!RMODPrivate)
			continue;
		
		// Load menu stuff
		LocalStuff = RMODPrivate->Data;
		
		// Not found?
		if (!LocalStuff)
			continue;
		
		// Add objects to the list, overwriting everything
		for (i = 0; i < LocalStuff->NumObjects; i++)
		{
			// Get Current
			TempObject = LocalStuff->Objects[i];
			
			// See if it already exists
			FoundID = INFO_GetTypeByName(TempObject->RClassName);
			
			// Not found? Add to end
			if (FoundID == NUMMOBJTYPES)
			{
				Z_ResizeArray((void**)&mobjinfo, sizeof(*mobjinfo), NUMMOBJTYPES, NUMMOBJTYPES + 1);
				mobjinfo[NUMMOBJTYPES++] = TempObject;
			}
			
			// Found, replace
			else
			{
				// Replace here
				mobjinfo[FoundID] = TempObject;
			}
		}
		
		// Push all states
		if (LocalStuff->NumObjectStates)
		{
			Base = NUMSTATES;
			Count = LocalStuff->NumObjectStates;
			
			if (MergeBase == (size_t)-1)
			{
				MergeBase = Base;
				MergeCount = Count;
			}
			else
				MergeCount += Count;
			
			// Resize array of states
			Z_ResizeArray((void**)&states, sizeof(*states), NUMSTATES, NUMSTATES + Count);
			NUMSTATES += Count;
			
			// Reference every single state
			for (j = 0, i = Base; i < Base + Count; i++, j++)
				states[i] = __REMOOD_REFSTATE(LocalStuff->ObjectStates[j]);
		}
	}
	
	/* Normalize Objects */
	for (i = 0; i < NUMMOBJTYPES; i++)
	{
		// Get Current
		TempObject = mobjinfo[i];
		
		// Dereference Classes
		if (TempObject->RFamilyClass)
			TempObject->RBaseFamily = INFO_GetTypeByName(TempObject->RFamilyClass);
		
		// Setup States
		for (z = 0; z < NUMINFOOBJECTSTATEGROUPS; z++)
			for (j = 0; c_ObjectLocalStates[j].Name; j++)
				if (c_ObjectLocalStates[j].IOSG == z)
				{
					// Get pointer reference of state
					StateRef = (statenum_t*)(((uintptr_t)TempObject) + c_ObjectLocalStates[j].RefDiff);
					
					// No Ref?
					if (!StateRef)
						continue;
					
					// The state we want (the first)
					RefToFind = ((j & 0xFFFF) << 16) | 1;
					
					// Find states in merge bases
					for (k = MergeBase; k < MergeBase + MergeCount; k++)
						if (states[k]->ObjectID == TempObject->ObjectID)
							if (states[k]->Marker == RefToFind)
							{
								*StateRef = k;
								break;
							}
					
					// Stop current loop and go to the upper loop
					break;
				}
	}
	
	/* Normalize state references */
	INFO_StateNormalize(MergeBase, MergeCount);
	
	/* Success! */
	return true;
}

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
		
		// Reference states and functions
		states[i]->sprite = INFO_SpriteNumByName(states[i]->HoldSprite, true);
		
		// Reference function
		if (states[i]->Function)
			states[i]->action = INFO_FunctionPtrByName(states[i]->Function);
		
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

/*** RMOD HELPERS ***/

/* INFO_RMODInnerStateHandler() -- Inner state handlers */
static bool_t INFO_RMODInnerStateHandler(Z_Table_t* const a_Sub, void* const a_Data)
{
	INFO_RMODStateHelper_t* HelperP = a_Data;
	INFO_ObjectStateGroup_t IOSG;
	const char* Value;
	int CurFrameID;
	state_t* StateP;
	size_t i;
	int32_t MarkerVal;
	int32_t IntVal;
	
	/* Check */
	if (!a_Sub || !a_Data)
		return true;
	
	/* Retrive item name */
	// Obtain
	Value = Z_TableName(a_Sub);
	
	// Not a frame?
	if (strncasecmp(Value, "frame#", 6) != 0)
		return true;
	
	// Knock off #
	Value = strchr(Value, '#');
	
	// Not found?
	if (!Value)
		return true;
	
	// Add 1 to remove #
	Value++;
	
	// Convert to integer
	CurFrameID = atoi(Value);
	
	/* Less than 1? invalid frame */
	if (CurFrameID < 1)
		return true;
	
	/* Add state frame to latest? */
	// Determine marker value
	MarkerVal = (HelperP->StateGroup << 16) | (CurFrameID & 0xFFFF);
	
	// Set group marker
	*HelperP->StateSplasher = MarkerVal;
	
	// See if it already exists
	StateP = NULL;
	for (i = 0; i < (*HelperP->NumStatesRef); i++)
		if (HelperP->ObjectID == __REMOOD_REFSTATE((*HelperP->StatesRef)[i])->ObjectID && MarkerVal == __REMOOD_REFSTATE((*HelperP->StatesRef)[i])->Marker)
		{
			StateP = __REMOOD_REFSTATE((*HelperP->StatesRef)[i]);
			break;
		}
	
	// Missing still?
	if (!StateP)
	{
		// Max array too small?
		if ((*HelperP->NumStatesRef) >= (*HelperP->MaxStatesRef))
		{
			Z_ResizeArray((void**)&(*HelperP->StatesRef), sizeof(*(*HelperP->StatesRef)), (*HelperP->MaxStatesRef), (*HelperP->MaxStatesRef) + LOCALSTATEJUMPS);
			(*HelperP->MaxStatesRef) += LOCALSTATEJUMPS;
		}
		
		// Place here
#if defined(__REMOOD_USEFLATTERSTATES)
		StateP = &((*HelperP->StatesRef)[(*HelperP->NumStatesRef)++]);
#else
		StateP = (*HelperP->StatesRef)[(*HelperP->NumStatesRef)++] = Z_Malloc(sizeof(*StateP), PU_WLDKRMOD, NULL);
#endif
	}
	
	/* Fill state with info */
	// Remember marker for later uses
	StateP->FrameID = CurFrameID;
	StateP->Marker = MarkerVal;
	StateP->ObjectID = HelperP->ObjectID;
	StateP->IOSG = HelperP->StateGroup;
	
	// DeHackEd Support
	StateP->DehackEdID = D_RMODGetValueInt(a_Sub, "DeHackEdNum", 0);
	
	// Get normal values
	StateP->tics = D_RMODGetValueInt(a_Sub, "Tics", 0);
	StateP->RMODFastTics = D_RMODGetValueInt(a_Sub, "FastTics", 0);
	
	// GhostlyDeath <April 28, 2012> -- To make DECORATE translation easier I
	// have decided to add support for A-Z.
	Value = Z_TableGetValue(a_Sub, "Frame");
	
	// Is it a letter?
	if (Value)
	{
		if (toupper(Value[0]) >= 'A' && toupper(Value[1]) <= 'Z')
			StateP->frame = toupper(Value[0]) - 'A';
		else
			StateP->frame = D_RMODGetValueInt(a_Sub, "Frame", 0);
	}
	
	// Get booleans
	if (D_RMODGetValueBool(a_Sub, "FullBright", false))
		StateP->frame |= FF_FULLBRIGHT;
	
	// Get Sprite
	Value = Z_TableGetValue(a_Sub, "Sprite");
	
	if (Value)
		for (i = 0; i < 4 && Value[i]; i++)
		{
			StateP->HoldSprite[i] = Value[i];
			StateP->SpriteID |= ((uint32_t)toupper(Value[i])) << (i * 8);
		}
		
	// Get Priority
	Value = Z_TableGetValue(a_Sub, "Priority");
	
	if (Value)
		StateP->Priority = INFO_PriorityByName(Value);
	
	// Get Transparency
	Value = Z_TableGetValue(a_Sub, "Transparency");
	
	if (Value)
		StateP->frame |= (INFO_TransparencyByName(Value) << FF_TRANSSHIFT) & FF_TRANSMASK;
		
	// Get function
	StateP->Function = D_RMODGetValueString(a_Sub, "Function", NULL);
	
	// Next?
	Value = Z_TableGetValue(a_Sub, "Goto");
	if (!Value)
	{
		// SimNext is squashed WeaponID and Marker
		StateP->SimNext = HelperP->ObjectID;
		StateP->SimNext <<= 32;
		
		// Determine marker
		IntVal = D_RMODGetValueInt(a_Sub, "Next", 0);
		
		// 0 is S_NULL, otherwise...
		if (IntVal <= 0)
			StateP->SimNext = 0;
		else
			StateP->SimNext |= (HelperP->StateGroup << 16) | (IntVal & 0xFFFF);
	}
	
	// Goto?
	else
	{
		// Match string to group
		if (HelperP->StateForName(HelperP->InputPtr, Value, &IOSG, NULL, NULL, NULL))
		{
			// Simulated Next is similar to above, but jumps to another group
			StateP->SimNext = HelperP->ObjectID;
			StateP->SimNext <<= 32;
			StateP->SimNext |= (IOSG << 16) | 1;
		}
	}

	/* Always return true */
	return true;
}

/* INFO_RMODStateHandlers() -- State handler */
bool_t INFO_RMODStateHandlers(Z_Table_t* const a_Sub, void* const a_Data)
{
	INFO_RMODStateHelper_t* HelperP = a_Data;
	const char* Value;
	
	/* Check */
	if (!a_Sub || !a_Data)
		return true;
	
	/* Retrive item name */
	// Obtain
	Value = Z_TableName(a_Sub);
	
	// Not a state table?
	if (strncasecmp(Value, "state#", 6) != 0)
		return true;
	
	// Knock off #
	Value = strchr(Value, '#');
	
	// Not found?
	if (!Value)
		return true;
	
	// Add 1 to remove #
	Value++;
	
	/* Determine state value */
	HelperP->StateValueP = HelperP->StateForName(HelperP->InputPtr, Value, &HelperP->StateGroup, &HelperP->StateSplasher, NULL, NULL);
	HelperP->BaseStateNum = 0;
	
	// Something here? (Future reference state groups?)
	if (!HelperP->StateValueP)
		return true;
	
	/* Run through an inner inner state callback */
	Z_TableSuperCallback(a_Sub, INFO_RMODInnerStateHandler, (void*)HelperP);

	/* Keep Going */
	return true;
}

/*****************************************************************************/

/* INFO_GetTypeByName() -- Returns a map object by it's name */
mobjtype_t INFO_GetTypeByName(const char* const a_Name)
{
	size_t i = 0;
	uint32_t Hash;
	
	/* Check */
	if (!a_Name)
		return NUMMOBJTYPES;
	
	/* Go through class list */
	// By Class Name
	for (i = 0; i < NUMMOBJTYPES; i++)
		if (mobjinfo[i]->RClassName)
			if (strcasecmp(a_Name, mobjinfo[i]->RClassName) == 0)
				return i;
			
	// By MT Name
	for (i = 0; i < NUMMOBJTYPES; i++)
		if (mobjinfo[i]->RMTName)
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
		sprnames[i] = Z_StrDup(a_Name, PU_WLDKRMOD, NULL);
		
		// Return the freshly added one
		return i;
	}
		
	/* Not found? */
	return 0;
}

/* INFO_FunctionPtrByName() -- Get action pointer by name */
actionf_t INFO_FunctionPtrByName(const char* const a_Name)
{
	actionf_t RetVal;
	
	memset(&RetVal, 0, sizeof(RetVal));
	
	/* Check */
	if (!a_Name)
		return RetVal;
	
	if (strcasecmp("Light0", a_Name) == 0) RetVal.acv = A_Light0;
	else if (strcasecmp("WeaponReady", a_Name) == 0) RetVal.acv = A_WeaponReady;
	else if (strcasecmp("Lower", a_Name) == 0) RetVal.acv = A_Lower;
	else if (strcasecmp("Raise", a_Name) == 0) RetVal.acv = A_Raise;
	else if (strcasecmp("Punch", a_Name) == 0) RetVal.acv = A_Punch;
	else if (strcasecmp("ReFire", a_Name) == 0) RetVal.acv = A_ReFire;
	else if (strcasecmp("FirePistol", a_Name) == 0) RetVal.acv = A_FirePistol;
	else if (strcasecmp("Light1", a_Name) == 0) RetVal.acv = A_Light1;
	else if (strcasecmp("FireShotgun", a_Name) == 0) RetVal.acv = A_FireShotgun;
	else if (strcasecmp("Light2", a_Name) == 0) RetVal.acv = A_Light2;
	else if (strcasecmp("FireShotgun2", a_Name) == 0) RetVal.acv = A_FireShotgun2;
	else if (strcasecmp("CheckReload", a_Name) == 0) RetVal.acv = A_CheckReload;
	else if (strcasecmp("OpenShotgun2", a_Name) == 0) RetVal.acv = A_OpenShotgun2;
	else if (strcasecmp("LoadShotgun2", a_Name) == 0) RetVal.acv = A_LoadShotgun2;
	else if (strcasecmp("CloseShotgun2", a_Name) == 0) RetVal.acv = A_CloseShotgun2;
	else if (strcasecmp("FireCGun", a_Name) == 0) RetVal.acv = A_FireCGun;
	else if (strcasecmp("GunFlash", a_Name) == 0) RetVal.acv = A_GunFlash;
	else if (strcasecmp("FireMissile", a_Name) == 0) RetVal.acv = A_FireMissile;
	else if (strcasecmp("Saw", a_Name) == 0) RetVal.acv = A_Saw;
	else if (strcasecmp("FirePlasma", a_Name) == 0) RetVal.acv = A_FirePlasma;
	else if (strcasecmp("BFGsound", a_Name) == 0) RetVal.acv = A_BFGsound;
	else if (strcasecmp("FireBFG", a_Name) == 0) RetVal.acv = A_FireBFG;
	else if (strcasecmp("BFGSpray", a_Name) == 0) RetVal.acv = A_BFGSpray;
	else if (strcasecmp("Explode", a_Name) == 0) RetVal.acv = A_Explode;
	else if (strcasecmp("Pain", a_Name) == 0) RetVal.acv = A_Pain;
	else if (strcasecmp("PlayerScream", a_Name) == 0) RetVal.acv = A_PlayerScream;
	else if (strcasecmp("Fall", a_Name) == 0) RetVal.acv = A_Fall;
	else if (strcasecmp("XScream", a_Name) == 0) RetVal.acv = A_XScream;
	else if (strcasecmp("Look", a_Name) == 0) RetVal.acv = A_Look;
	else if (strcasecmp("Chase", a_Name) == 0) RetVal.acv = A_Chase;
	else if (strcasecmp("FaceTarget", a_Name) == 0) RetVal.acv = A_FaceTarget;
	else if (strcasecmp("PosAttack", a_Name) == 0) RetVal.acv = A_PosAttack;
	else if (strcasecmp("Scream", a_Name) == 0) RetVal.acv = A_Scream;
	else if (strcasecmp("SPosAttack", a_Name) == 0) RetVal.acv = A_SPosAttack;
	else if (strcasecmp("VileChase", a_Name) == 0) RetVal.acv = A_VileChase;
	else if (strcasecmp("VileStart", a_Name) == 0) RetVal.acv = A_VileStart;
	else if (strcasecmp("VileTarget", a_Name) == 0) RetVal.acv = A_VileTarget;
	else if (strcasecmp("VileAttack", a_Name) == 0) RetVal.acv = A_VileAttack;
	else if (strcasecmp("StartFire", a_Name) == 0) RetVal.acv = A_StartFire;
	else if (strcasecmp("Fire", a_Name) == 0) RetVal.acv = A_Fire;
	else if (strcasecmp("FireCrackle", a_Name) == 0) RetVal.acv = A_FireCrackle;
	else if (strcasecmp("Tracer", a_Name) == 0) RetVal.acv = A_Tracer;
	else if (strcasecmp("SkelWhoosh", a_Name) == 0) RetVal.acv = A_SkelWhoosh;
	else if (strcasecmp("SkelFist", a_Name) == 0) RetVal.acv = A_SkelFist;
	else if (strcasecmp("SkelMissile", a_Name) == 0) RetVal.acv = A_SkelMissile;
	else if (strcasecmp("FatRaise", a_Name) == 0) RetVal.acv = A_FatRaise;
	else if (strcasecmp("FatAttack1", a_Name) == 0) RetVal.acv = A_FatAttack1;
	else if (strcasecmp("FatAttack2", a_Name) == 0) RetVal.acv = A_FatAttack2;
	else if (strcasecmp("FatAttack3", a_Name) == 0) RetVal.acv = A_FatAttack3;
	else if (strcasecmp("BossDeath", a_Name) == 0) RetVal.acv = A_BossDeath;
	else if (strcasecmp("CPosAttack", a_Name) == 0) RetVal.acv = A_CPosAttack;
	else if (strcasecmp("CPosRefire", a_Name) == 0) RetVal.acv = A_CPosRefire;
	else if (strcasecmp("TroopAttack", a_Name) == 0) RetVal.acv = A_TroopAttack;
	else if (strcasecmp("SargAttack", a_Name) == 0) RetVal.acv = A_SargAttack;
	else if (strcasecmp("HeadAttack", a_Name) == 0) RetVal.acv = A_HeadAttack;
	else if (strcasecmp("BruisAttack", a_Name) == 0) RetVal.acv = A_BruisAttack;
	else if (strcasecmp("SkullAttack", a_Name) == 0) RetVal.acv = A_SkullAttack;
	else if (strcasecmp("Metal", a_Name) == 0) RetVal.acv = A_Metal;
	else if (strcasecmp("SpidRefire", a_Name) == 0) RetVal.acv = A_SpidRefire;
	else if (strcasecmp("BabyMetal", a_Name) == 0) RetVal.acv = A_BabyMetal;
	else if (strcasecmp("BspiAttack", a_Name) == 0) RetVal.acv = A_BspiAttack;
	else if (strcasecmp("Hoof", a_Name) == 0) RetVal.acv = A_Hoof;
	else if (strcasecmp("CyberAttack", a_Name) == 0) RetVal.acv = A_CyberAttack;
	else if (strcasecmp("PainAttack", a_Name) == 0) RetVal.acv = A_PainAttack;
	else if (strcasecmp("PainDie", a_Name) == 0) RetVal.acv = A_PainDie;
	else if (strcasecmp("KeenDie", a_Name) == 0) RetVal.acv = A_KeenDie;
	else if (strcasecmp("BrainPain", a_Name) == 0) RetVal.acv = A_BrainPain;
	else if (strcasecmp("BrainScream", a_Name) == 0) RetVal.acv = A_BrainScream;
	else if (strcasecmp("BrainDie", a_Name) == 0) RetVal.acv = A_BrainDie;
	else if (strcasecmp("BrainAwake", a_Name) == 0) RetVal.acv = A_BrainAwake;
	else if (strcasecmp("BrainSpit", a_Name) == 0) RetVal.acv = A_BrainSpit;
	else if (strcasecmp("SpawnSound", a_Name) == 0) RetVal.acv = A_SpawnSound;
	else if (strcasecmp("SpawnFly", a_Name) == 0) RetVal.acv = A_SpawnFly;
	else if (strcasecmp("BrainExplode", a_Name) == 0) RetVal.acv = A_BrainExplode;
	else if (strcasecmp("SmokeTrailer", a_Name) == 0) RetVal.acv = A_SmokeTrailer;
	else if (strcasecmp("SmokeTrailerRocket", a_Name) == 0) RetVal.acv = A_SmokeTrailerRocket;
	else if (strcasecmp("SmokeTrailerSkull", a_Name) == 0) RetVal.acv = A_SmokeTrailerSkull;
	else if (strcasecmp("FireOldBFG", a_Name) == 0) RetVal.acv = A_FireOldBFG;
	else if (strcasecmp("FireGenericProjectile", a_Name) == 0) RetVal.acv = A_FireGenericProjectile;
	else if (strcasecmp("NextFrameIfMoving", a_Name) == 0) RetVal.acv = A_NextFrameIfMoving;
	else if (strcasecmp("GenericMonsterMissile", a_Name) == 0) RetVal.acv = A_GenericMonsterMissile;
	
	/* Not found? */
	return RetVal;
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

