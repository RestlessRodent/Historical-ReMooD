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
//      Demo Compatibility

#ifndef __P_DEMCMP_H__
#define __P_DEMCMP_H__

#include "console.h"
#include "doomdef.h"
#include "doomstat.h"
#include "w_wad.h"

/*****************************
*** EXTENDED GAME SETTINGS ***
*****************************/

// The way settings were done in Legacy were that they were console variables,
// so you would have tons of console variables for every concievable setting.
// However, of all the settings, virtually all of them are either on/off, pure
// integers, floating point numbers, or a specific list of options.
// Doing individually split game settings allows for them to be saved all at
// once and sent by the server all at once, rather than polluting the console
// code with variables splattered all over the place. Game options aren't
// exactly saveable in configs, only the settings that would take effect the
// next game that is played. Also, settings will change drastically when demos
// are played, so you don't want the ugly demoversion checks and you also don't
// want to lost all your game settings when you play a demo or use similar
// settings from the last demo played.

// So Legacy settings are all demoversion and cvars, an ugly mix.

/*** CONSTANTS ***/

#define PEXGSSTRBUFSIZE						32	// String Buffer Size

/* P_EXGSType_t -- Setting type for said setting */
typedef enum P_EXGSType_e
{
	PEXGST_INTEGER,								// Integer
	PEXGST_FLOAT,								// Floating Point
	
	NUMPEXGSTYPES
} P_EXGSType_t;

/* P_EXGSDisplayAs_t -- Display as whichever value */
typedef enum P_EXGSDisplayAs_e
{
	PEXGSDA_INTEGER,							// Plain Integer
	PEXGSDA_YESNO,								// Yes/No
	PEXGSDA_TIMESECS,							// Time in Seconds
	PEXGSDA_STRING,								// Show as string
	PEXGSDA_TIMEMINS,							// Time in Minutes
	
	NUMPEXGSDISPLAYAS
} P_EXGSDisplayAs_t;

/* P_EXGSDemoRange_t -- Range for demo compatibility */
typedef enum P_EXGSDemoRange_e
{
	PEXGSDR_NOCHECK,							// Do not check range here
	PEXGSDR_EQUALS,								// ==
	PEXGSDR_NOT,								// !=
	PEXGSDR_LESSTHAN,							// <
	PEXGSDR_GREATERTHAN,						// >
	PEXGSDR_ATMOST,								// <=
	PEXGSDR_ATLEAST,							// >=
	
	PEXGSDR_MORETHAN = PEXGSDR_GREATERTHAN,
	
	NUMPEXGSDEMORANGES
} P_EXGSDemoRange_t;

/* P_EXGSGameMode_t -- Game modes for demo compat */
typedef enum P_EXGSGameMode_e
{
	PEXGSGM_DOOM						= 0x01,	// Doom
	PEXGSGM_HERETIC						= 0x02,	// Heretic
	
	PEXGSGM_ANY = PEXGSGM_DOOM | PEXGSGM_HERETIC,
} P_EXGSGameMode_t;

/* P_EXGSMenuCategory_t -- Menu category */
typedef enum P_EXGSMenuCategory_s
{
	PEXGSMC_NONE,								// Nothing
	PEXGSMC_GAME,								// Game Settings
	PEXGSMC_ITEMS,								// Item Settings
	PEXGSMC_PLAYERS,							// Players
	PEXGSMC_MONSTERS,							// Monster Settings
	PEXGSMC_MISC,								// Misc. Settings
	PEXGSMC_FUN,								// Fun Settings! =D yay!
	PEXGSMC_HERETIC,							// Heretic Settings
	PEXGSMC_COMPAT,								// Compatibility Option
	
	NUMPEXGSMENUCATEGORIES
} P_EXGSMenuCategory_t;

/* P_EXGSBitID_t -- Bit ID of flag */
typedef enum P_EXGSBitID_e
{
	PEXGSBID_NOTHINGHERE,						// Nothing is here
	PEXGSBID_COENABLEBLOODSPLATS,				// Enables blood splats
	PEXGSBID_CORANDOMLASTLOOK,					// Randomized Last Look
	PEXGSBID_COUNSHIFTVILERAISE,				// <<=2 when vile resurrects
	PEXGSBID_COMODIFYCORPSE,					// Modify corpse more in A_Fall()
	PEXGSBID_CONOSMOKETRAILS,					// Disable smoke trails in A_SmokeTrailer()
	PEXGSBID_COUSEREALSMOKE,					// Use real smoke on trails
	PEXGSBID_COOLDCUTCORPSERADIUS,				// Cut corpse radius when !COMODIFYCORPSE
	PEXGSBID_COSPAWNDROPSONMOFLOORZ,			// Spawn dropped items on the map object's floorz
	PEXGSBID_CODISABLETEAMPLAY,					// Disable support for team play
	PEXGSBID_COSLOWINWATER,						// Move slowly in water
	PEXGSBID_COSLIDEOFFMOFLOOR,					// Slide of mobj's floorz rather than real sector
	PEXGSBID_COOLDFRICTIONMOVE,					// Use old friction in XYMovement()
	PEXGSBID_COOUCHONCEILING,					// Go "ouch" when hitting the ceiling
	PEXGSBID_COENABLESPLASHES,					// Enable water splashes
	PEXGSBID_COENABLEFLOORSMOKE,				// Enable floor smoke
	PEXGSBID_COENABLESMOKE,						// Enables spawning of smoke
	PEXGSBID_CODAMAGEONLAND,					// Damage player once landing on floor
	PEXGSBID_COABSOLUTEANGLE,					// Use absolute angle turning rather than relative
	PEXGSBID_COOLDJUMPOVER,						// Use old jump over code
	PEXGSBID_COENABLESPLATS,					// Enable wall splats
	PEXGSBID_COOLDFLATPUSHERCODE,				// Use Old (non 3D floor capable) pushers/pullers
	PEXGSBID_COSPAWNPLAYERSEARLY,				// Spawn players early (during map setup)
	PEXGSBID_COENABLEUPDOWNSHOOT,				// Enable shooting up/down (aiming) when no target found
	PEXGSBID_CONOUNDERWATERCHECK,				// Don't check for things being underwater
	PEXGSBID_COSPLASHTRANSWATER,				// Cause a splash when transitioning to/from water
	PEXGSBID_COUSEOLDZCHECK,					// Use Old Z Checking Code
	PEXGSBID_COCHECKXYMOVE,						// Check X/Y Movement in old Z Code
	PEXGSBID_COWATERZFRICTION,					// Apply friction when underwater on Z plane
	PEXGSBID_CORANOMLASTLOOKSPAWN,				// Random last look on spawn
	PEXGSBID_COALWAYSRETURNDEADSPMISSILE,		// Always return the missile even if it died on spawn
	PEXGSBID_COUSEMOUSEAIMING,					// When player autoaimed at nothing, use mouse aiming angle
	PEXGSBID_COFIXPLAYERMISSILEANGLE,			// Fix the angle of player missiles being fired
	PEXGSBID_COREMOVEMOINSKYZ,					// When Z movement into sky, do not explode a missile.
	PEXGSBID_COFORCEAUTOAIM,					// Force Auto-aim
	PEXGSBID_COFORCEBERSERKSWITCH,				// Force switching to berserk enabled slots.
	PEXGSBID_CODOUBLEPICKUPCHECK,				// Check twice when picking things up
	PEXGSBID_CODISABLEMISSILEIMPACTCHECK,		// Disable check for missile impact
	PEXGSBID_COMISSILESPLATONWALL,				// Splat missiles on walls
	PEXGSBID_CONEWBLOODHITSCANCODE,				// Use newer blood spewing code.
	PEXGSBID_CONEWAIMINGCODE,					// Use newer aiming code.
	PEXGSBID_COMISSILESPECHIT,					// Missiles could trigger special hits?
	PEXGSBID_COHITSCANSSLIDEONFLATS,			// Hitscans slide on flats
	PEXGSBID_CONONSOLIDPASSTHRUOLD,				// Non-solid pass through (older trigger)
	PEXGSBID_CONONSOLIDPASSTHRUNEW,				// Non-solid pass through (newer trigger)
	PEXGSBID_COJUMPCHECK,						// Check for jumping
	PEXGSBID_COLINEARMAPTRAVERSE,				// Linearly traverse maps
	PEXGSBID_COONLYTWENTYDMSPOTS,				// Only support 20 deathmatch spawn spots.
	PEXGSBID_COALLOWSTUCKSPAWNS,				// Allow getting stuck in spawn spots.
	PEXGSBID_COUSEOLDBLOOD,						// Use Older Doom Blood
	PEXGSBID_FUNMONSTERFFA,						// Monster Free For All
	PEXGSBID_FUNINFIGHTING,						// Monster Infighting
	PEXGSBID_COCORRECTVILETARGET,				// Correct Arch-Vile Target Fire
	PEXGSBID_FUNMONSTERSMISSMORE,				// Monsters miss more
	PEXGSBID_COMORECRUSHERBLOOD,				// More Crusher Blood
	PEXGSBID_CORANDOMBLOODDIR,					// Spew Blood in random directions
	PEXGSBID_COINFINITEROCKETZ,					// Infinite Rocket Z Damage
	PEXGSBID_COALLOWROCKETJUMPING,				// Allow rocket jumping
	PEXGSBID_COROCKETZTHRUST,					// Rocket Z Thrust
	PEXGSBID_COLIMITMONSTERZMATTACK,			// Limit Monster Z Range
	PEXGSBID_HEREMONSTERTHRESH,					// Heretic Monster Threshold
	PEXGSBID_COVOODOODOLLS,						// Voodoo Dolls
	PEXGSBID_COEXTRATRAILPUFF,					// Extra smoke trail puff
	PEXGSBID_COLOSTSOULTRAILS,					// Trails for lost souls
	PEXGSBID_COTRANSTWOSIDED,					// Transparent Two Sided walls
	PEXGSBID_COENABLEBLOODTIME,					// Enable Blood Time
	PEXGSBID_COENABLEJUMPING,					// Enable Jumping
	PEXGSBID_COMOUSEAIM,						// Aim by mouse
	PEXGSBID_MONRESPAWNMONSTERS,				// Respawn Monsters
	PEXGSBID_FUNNOTARGETPLAYER,					// Don't Target Players, ever!
	PEXGSBID_MONARCHVILEANYRESPAWN,				// Arch-Viles can respawn anything
	PEXGSBID_COOLDCHECKPOSITION,				// Use Old P_CheckPosition()
	PEXGSBID_COLESSSPAWNSTICKING,				// Make spawn sticking less likely
	PEXGSBID_PLSPAWNTELEFRAG,					// Telefrag on spawn
	PEXGSBID_GAMEONEHITKILLS,					// One Hit Kills
	PEXGSBID_COBETTERPLCORPSEREMOVAL,			// Botter bodyqueue management
	PEXGSBID_PLSPAWNCLUSTERING,					// Cluster spawn spots
	PEXGSBID_COIMPROVEDMOBJONMOBJ,				// Improved Mobj on Mobj
	PEXGSBID_COIMPROVEPATHTRAVERSE,				// Smooth out path traversing
	PEXGSBID_PLJUMPGRAVITY,						// Player Jump Gravity
	PEXGSBID_FUNNOLOCKEDDOORS,					// No Doors are locked
	PEXGSBID_GAMEAIRFRICTION,					// Friction in air
	PEXGSBID_GAMEWATERFRICTION,					// Friction in water
	PEXGSBID_GAMEMIDWATERFRICTION,				// Friction in water (not on ground)
	PEXGSBID_GAMEALLOWLEVELEXIT,				// Allow Exiting the game
	PEXGSBID_GAMEALLOWROCKETJUMP,				// Allow Rocket Jumping
	PEXGSBID_PLALLOWAUTOAIM,					// Allow Auto-Aiming
	PEXGSBID_PLFORCEWEAPONSWITCH,				// Force Weapon Switching
	PEXGSBID_PLDROPWEAPONS,						// Drop Player Weapons
	PEXGSBID_PLINFINITEAMMO,					// Infinite Ammo
	PEXGSBID_GAMEHERETICGIBBING,				// Heretic Gibbing
	PEXGSBID_MONPREDICTMISSILES,				// cv_predictingmonsters
	PEXGSBID_MONRESPAWNMONSTERSTIME,			// cv_respawnmonsterstime
	PEXGSBID_PLSPAWNWITHMAXGUNS,				// Spawn With All Guns
	PEXGSBID_PLSPAWNWITHSUPERGUNS,				// Spawn With Super Guns
	PEXGSBID_PLSPAWNWITHMAXSTATS,				// Spawn With Max Stats
	PEXGSBID_ITEMSSPAWNPICKUPS,					// Spawn Game Pickups
	PEXGSBID_COHERETICFRICTION,					// Allow Heretic Friction
	PEXGSBID_GAMEDEATHMATCH,					// Deathmatch Mode
	PEXGSBID_PLSPAWNWITHALLKEYS,				// Spawn With All Keys
	PEXGSBID_ITEMSKEEPWEAPONS,					// Keep Weapons on the Floor
	PEXGSBID_GAMETEAMPLAY,						// Team Play
	PEXGSBID_GAMETEAMDAMAGE,					// Team Damage
	PEXGSBID_GAMEFRAGLIMIT,						// Frag Limit
	PEXGSBID_GAMETIMELIMIT,						// Time Limit
	PEXGSBID_MONSTATICRESPAWNTIME,				// Static monster respawn time
	PEXGSBID_PLFASTERWEAPONS,					// Faster Player Weapons
	PEXGSBID_MONSPAWNMONSTERS,					// Spawn Monsters
	PEXGSBID_GAMESPAWNMULTIPLAYER,				// Spawn multi-player stuff
	PEXGSBID_ITEMRESPAWNITEMS,					// Respawn Items
	PEXGSBID_ITEMRESPAWNITEMSTIME,				// Respawn Item Time
	PEXGSBID_MONFASTMONSTERS,					// Monsters are fast
	PEXGSBID_GAMESOLIDCORPSES,					// Corpses are solid
	PEXGSBID_GAMEBLOODTIME,						// Time blood stays around
	PEXGSBID_GAMEGRAVITY,						// Level Gravity amount
	PEXGSBID_MONENABLECLEANUP,					// Cleanup Dead monsters
	PEXGSBID_MONCLEANUPRESPTIME,				// Time it takes to cleanup respawnable monsters
	PEXGSBID_MONCLEANUPNONRTIME,				// Time it takes to cleanup non-respawnable monsters
	PEXGSBID_GAMESKILL,							// Current Game Skill Level
	PEXGSBID_PLHALFDAMAGE,						// Take Half Damage
	PEXGSBID_PLDOUBLEAMMO,						// Receive Double Ammo
	PEXGSBID_MONKILLCOUNTMODE,					// Kill Count Mode
	
	PEXGSNUMBITIDS
} P_EXGSBitID_t;

/*** STRUCTURES ***/

/* P_EXGSVariable_t -- Variable for game setting */
typedef struct P_EXGSVariable_s
{
	// Base
	P_EXGSType_t Type;							// Type of value to conform to
	P_EXGSBitID_t BitID;						// BitID of flag
	const char* Name;							// Name of game setting
	const char* MenuTitle;						// Title for menus
	const char* Description;					// Description
	const uint8_t GameFlags;					// Game Flags
	const P_EXGSDemoRange_t DemoRange;			// Range for "demoversion"
	const uint16_t DemoVersion;					// "demoversion" wrapper
	const int32_t DemoVal[2];					// Demo values (false, true)
	const int32_t DefaultVal;					// Default value wherever
	P_EXGSMenuCategory_t Category;				// Category for item
	P_EXGSDisplayAs_t DisplayAs;				// Display as this
	const CONL_VarPossibleValue_t* Possible;	// Possible values
	void (*ChangeFunc)(struct P_EXGSVariable_s* const a_Bit);
	
	// Settings
	bool_t WasSet;								// Was Set to value?
	int32_t ActualVal;							// Actually set value
	
	// String Value
	char StrVal[PEXGSSTRBUFSIZE];				// String Value
} P_EXGSVariable_t;

/*** FUNCTIONS ***/

// Setting Finder
P_EXGSBitID_t P_EXGSBitForName(const char* const a_Name);
P_EXGSVariable_t* P_EXGSVarForBit(const P_EXGSBitID_t a_Bit);
P_EXGSVariable_t* P_EXGSVarForName(const char* const a_Name);

// Value Getter
int32_t P_EXGSGetValue(const P_EXGSBitID_t a_Bit);
fixed_t P_EXGSGetFixed(const P_EXGSBitID_t a_Bit);

// General Functions
bool_t P_EXGSRegisterStuff(void);
bool_t P_EXGSSetVersionLevel(const uint32_t a_Level);
int32_t P_EXGSSetValue(const bool_t a_Master, const P_EXGSBitID_t a_Bit, const int32_t a_Value);
int32_t P_EXGSSetValueStr(const bool_t a_Master, const P_EXGSBitID_t a_Bit, const char* const a_Value);

/*******************
*** DEMO FACTORY ***
*******************/

typedef bool_t (*P_DEMO_ReadTicCmdType_t)(struct P_CurrentDemo_s* a_Current, ticcmd_t* const a_Cmd, const int32_t a_PlayerNum);
typedef bool_t (*P_DEMO_WriteTicCmdType_t)(struct P_CurrentDemo_s* a_Current, const ticcmd_t* const a_Cmd, const int32_t a_PlayerNum);

/* P_CurrentDemo_t -- Current Demo Info */
typedef struct P_CurrentDemo_s
{
	P_DEMO_ReadTicCmdType_t ReadTicCmdFunc;
	P_DEMO_WriteTicCmdType_t WriteTicCmdFunc;
} P_CurrentDemo_t;

#endif							/* __P_DEMCMP_H__ */

