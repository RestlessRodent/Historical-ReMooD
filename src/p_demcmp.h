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

/* P_XGSType_t -- Setting type for said setting */
typedef enum P_XGSType_e
{
	PEXGST_INTEGER,								// Integer
	PEXGST_FLOAT,								// Floating Point
	
	NUMPEXGSTYPES
} P_XGSType_t;

/* P_XGSDisplayAs_t -- Display as whichever value */
typedef enum P_XGSDisplayAs_e
{
	PEXGSDA_INTEGER,							// Plain Integer
	PEXGSDA_YESNO,								// Yes/No
	PEXGSDA_TIMESECS,							// Time in Seconds
	PEXGSDA_STRING,								// Show as string
	PEXGSDA_TIMEMINS,							// Time in Minutes
	
	NUMPEXGSDISPLAYAS
} P_XGSDisplayAs_t;

/* P_XGSDemoRange_t -- Range for demo compatibility */
typedef enum P_XGSDemoRange_e
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
} P_XGSDemoRange_t;

/* P_XGSGameMode_t -- Game modes for demo compat */
typedef enum P_XGSGameMode_e
{
	PEXGSGM_DOOM						= 0x01,	// Doom
	PEXGSGM_HERETIC						= 0x02,	// Heretic
	PEXGSGM_HEXEN						= 0x04,	// Hexen
	PEXGSGM_STRIFE						= 0x08,	// Strife
	
	PEXGSGM_ANY = PEXGSGM_DOOM | PEXGSGM_HERETIC,
} P_XGSGameMode_t;

/* P_XGSMenuCategory_t -- Menu category */
typedef enum P_XGSMenuCategory_s
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
} P_XGSMenuCategory_t;

/* P_XGSBitID_t -- Bit ID of flag */
typedef enum P_XGSBitID_e
{
	PGS_NOTHINGHERE,						// Nothing is here
	PGS_COENABLEBLOODSPLATS,				// Enables blood splats
	PGS_CORANDOMLASTLOOK,					// Randomized Last Look
	PGS_COUNSHIFTVILERAISE,				// <<=2 when vile resurrects
	PGS_COMODIFYCORPSE,					// Modify corpse more in A_Fall()
	PGS_CONOSMOKETRAILS,					// Disable smoke trails in A_SmokeTrailer()
	PGS_COUSEREALSMOKE,					// Use real smoke on trails
	PGS_COOLDCUTCORPSERADIUS,				// Cut corpse radius when !COMODIFYCORPSE
	PGS_COSPAWNDROPSONMOFLOORZ,			// Spawn dropped items on the map object's floorz
	PGS_CODISABLETEAMPLAY,					// Disable support for team play
	PGS_COSLOWINWATER,						// Move slowly in water
	PGS_COSLIDEOFFMOFLOOR,					// Slide of mobj's floorz rather than real sector
	PGS_COOLDFRICTIONMOVE,					// Use old friction in XYMovement()
	PGS_COOUCHONCEILING,					// Go "ouch" when hitting the ceiling
	PGS_COENABLESPLASHES,					// Enable water splashes
	PGS_COENABLEFLOORSMOKE,				// Enable floor smoke
	PGS_COENABLESMOKE,						// Enables spawning of smoke
	PGS_CODAMAGEONLAND,					// Damage player once landing on floor
	PGS_COABSOLUTEANGLE,					// Use absolute angle turning rather than relative
	PGS_COOLDJUMPOVER,						// Use old jump over code
	PGS_COENABLESPLATS,					// Enable wall splats
	PGS_COOLDFLATPUSHERCODE,				// Use Old (non 3D floor capable) pushers/pullers
	PGS_COSPAWNPLAYERSEARLY,				// Spawn players early (during map setup)
	PGS_COENABLEUPDOWNSHOOT,				// Enable shooting up/down (aiming) when no target found
	PGS_CONOUNDERWATERCHECK,				// Don't check for things being underwater
	PGS_COSPLASHTRANSWATER,				// Cause a splash when transitioning to/from water
	PGS_COUSEOLDZCHECK,					// Use Old Z Checking Code
	PGS_COCHECKXYMOVE,						// Check X/Y Movement in old Z Code
	PGS_COWATERZFRICTION,					// Apply friction when underwater on Z plane
	PGS_CORANOMLASTLOOKSPAWN,				// Random last look on spawn
	PGS_COALWAYSRETURNDEADSPMISSILE,		// Always return the missile even if it died on spawn
	PGS_COUSEMOUSEAIMING,					// When player autoaimed at nothing, use mouse aiming angle
	PGS_COFIXPLAYERMISSILEANGLE,			// Fix the angle of player missiles being fired
	PGS_COREMOVEMOINSKYZ,					// When Z movement into sky, do not explode a missile.
	PGS_COFORCEAUTOAIM,					// Force Auto-aim
	PGS_COFORCEBERSERKSWITCH,				// Force switching to berserk enabled slots.
	PGS_CODOUBLEPICKUPCHECK,				// Check twice when picking things up
	PGS_CODISABLEMISSILEIMPACTCHECK,		// Disable check for missile impact
	PGS_COMISSILESPLATONWALL,				// Splat missiles on walls
	PGS_CONEWBLOODHITSCANCODE,				// Use newer blood spewing code.
	PGS_CONEWAIMINGCODE,					// Use newer aiming code.
	PGS_COMISSILESPECHIT,					// Missiles could trigger special hits?
	PGS_COHITSCANSSLIDEONFLATS,			// Hitscans slide on flats
	PGS_CONONSOLIDPASSTHRUOLD,				// Non-solid pass through (older trigger)
	PGS_CONONSOLIDPASSTHRUNEW,				// Non-solid pass through (newer trigger)
	PGS_COJUMPCHECK,						// Check for jumping
	PGS_COLINEARMAPTRAVERSE,				// Linearly traverse maps
	PGS_COONLYTWENTYDMSPOTS,				// Only support 20 deathmatch spawn spots.
	PGS_COALLOWSTUCKSPAWNS,				// Allow getting stuck in spawn spots.
	PGS_COUSEOLDBLOOD,						// Use Older Doom Blood
	PGS_FUNMONSTERFFA,						// Monster Free For All
	PGS_FUNINFIGHTING,						// Monster Infighting
	PGS_COCORRECTVILETARGET,				// Correct Arch-Vile Target Fire
	PGS_FUNMONSTERSMISSMORE,				// Monsters miss more
	PGS_COMORECRUSHERBLOOD,				// More Crusher Blood
	PGS_CORANDOMBLOODDIR,					// Spew Blood in random directions
	PGS_COINFINITEROCKETZ,					// Infinite Rocket Z Damage
	PGS_COALLOWROCKETJUMPING,				// Allow rocket jumping
	PGS_COROCKETZTHRUST,					// Rocket Z Thrust
	PGS_COLIMITMONSTERZMATTACK,			// Limit Monster Z Range
	PGS_HEREMONSTERTHRESH,					// Heretic Monster Threshold
	PGS_COVOODOODOLLS,						// Voodoo Dolls
	PGS_COEXTRATRAILPUFF,					// Extra smoke trail puff
	PGS_COLOSTSOULTRAILS,					// Trails for lost souls
	PGS_COTRANSTWOSIDED,					// Transparent Two Sided walls
	PGS_COENABLEBLOODTIME,					// Enable Blood Time
	PGS_PLENABLEJUMPING,					// Enable Jumping
	PGS_COMOUSEAIM,						// Aim by mouse
	PGS_MONRESPAWNMONSTERS,				// Respawn Monsters
	PGS_FUNNOTARGETPLAYER,					// Don't Target Players, ever!
	PGS_MONARCHVILEANYRESPAWN,				// Arch-Viles can respawn anything
	PGS_COOLDCHECKPOSITION,				// Use Old P_CheckPosition()
	PGS_COLESSSPAWNSTICKING,				// Make spawn sticking less likely
	PGS_PLSPAWNTELEFRAG,					// Telefrag on spawn
	PGS_GAMEONEHITKILLS,					// One Hit Kills
	PGS_COBETTERPLCORPSEREMOVAL,			// Botter bodyqueue management
	PGS_PLSPAWNCLUSTERING,					// Cluster spawn spots
	PGS_COIMPROVEDMOBJONMOBJ,				// Improved Mobj on Mobj
	PGS_COIMPROVEPATHTRAVERSE,				// Smooth out path traversing
	PGS_PLJUMPGRAVITY,						// Player Jump Gravity
	PGS_FUNNOLOCKEDDOORS,					// No Doors are locked
	PGS_GAMEAIRFRICTION,					// Friction in air
	PGS_GAMEWATERFRICTION,					// Friction in water
	PGS_GAMEMIDWATERFRICTION,				// Friction in water (not on ground)
	PGS_GAMEALLOWLEVELEXIT,				// Allow Exiting the game
	PGS_GAMEALLOWROCKETJUMP,				// Allow Rocket Jumping
	PGS_PLALLOWAUTOAIM,					// Allow Auto-Aiming
	PGS_PLFORCEWEAPONSWITCH,				// Force Weapon Switching
	PGS_PLDROPWEAPONS,						// Drop Player Weapons
	PGS_PLINFINITEAMMO,					// Infinite Ammo
	PGS_GAMEHERETICGIBBING,				// Heretic Gibbing
	PGS_MONPREDICTMISSILES,				// cv_predictingmonsters
	PGS_MONRESPAWNMONSTERSTIME,			// cv_respawnmonsterstime
	PGS_PLSPAWNWITHMAXGUNS,				// Spawn With All Guns
	PGS_PLSPAWNWITHSUPERGUNS,				// Spawn With Super Guns
	PGS_PLSPAWNWITHMAXSTATS,				// Spawn With Max Stats
	PGS_ITEMSSPAWNPICKUPS,					// Spawn Game Pickups
	PGS_COHERETICFRICTION,					// Allow Heretic Friction
	PGS_GAMEDEATHMATCH,					// Deathmatch Mode
	PGS_PLSPAWNWITHALLKEYS,				// Spawn With All Keys
	PGS_ITEMSKEEPWEAPONS,					// Keep Weapons on the Floor
	PGS_GAMETEAMPLAY,						// Team Play
	PGS_GAMETEAMDAMAGE,					// Team Damage
	PGS_GAMEFRAGLIMIT,						// Frag Limit
	PGS_GAMETIMELIMIT,						// Time Limit
	PGS_MONSTATICRESPAWNTIME,				// Static monster respawn time
	PGS_PLFASTERWEAPONS,					// Faster Player Weapons
	PGS_MONSPAWNMONSTERS,					// Spawn Monsters
	PGS_GAMESPAWNMULTIPLAYER,				// Spawn multi-player stuff
	PGS_ITEMRESPAWNITEMS,					// Respawn Items
	PGS_ITEMRESPAWNITEMSTIME,				// Respawn Item Time
	PGS_MONFASTMONSTERS,					// Monsters are fast
	PGS_GAMESOLIDCORPSES,					// Corpses are solid
	PGS_GAMEBLOODTIME,						// Time blood stays around
	PGS_GAMEGRAVITY,						// Level Gravity amount
	PGS_MONENABLECLEANUP,					// Cleanup Dead monsters
	PGS_MONCLEANUPRESPTIME,				// Time it takes to cleanup respawnable monsters
	PGS_MONCLEANUPNONRTIME,				// Time it takes to cleanup non-respawnable monsters
	PGS_GAMESKILL,							// Current Game Skill Level
	PGS_PLHALFDAMAGE,						// Take Half Damage
	PGS_PLDOUBLEAMMO,						// Receive Double Ammo
	PGS_MONKILLCOUNTMODE,					// Kill Count Mode
	PGS_COOLDBFGSPRAY,						// Old BFG Spraying
	PGS_COEXPLODEHITFLOOR,					// HitFloor on explode
	PGS_COBOMBTHRUFLOOR,					// Bomb through floors
	PGS_COOLDEXPLOSIONS,					// Old Explosion Code
	PGS_COAIMCHECKFAKEFLOOR,				// Check 3D Floors when aiming
	PGS_CONEWGUNSHOTCODE,					// Use New Gunshot code
	PGS_COSHOOTCHECKFAKEFLOOR,				// Check 3D When Shooting
	PGS_COSHOOTFLOORCLIPPING,				// Clip gunshots to the floor
	PGS_CONEWSSGSPREAD,					// New SSG Spread
	PGS_COMONSTERLOOKFORMONSTER,			// Monsters can look for monsters
	PGS_COOLDTHINGHEIGHTS,					// Old Thing Heights
	PGS_COLASTLOOKMAXPLAYERS,				// Last Look Max Players
	PGS_COMOVECHECKFAKEFLOOR,				// Check Fake floor when moving
	PGS_COMULTIPLAYER,						// Multiplayer Format
	PGS_COBOOMSUPPORT,						// boomsupport Flag
	PGS_PLSPAWNWITHFAVGUN,					// Spawn with favorite gun
	PGS_CONOSAWFACING,						// No facing when sawing
	PGS_COENABLETEAMMONSTERS,				// Enable Teamable Monsters
	PGS_COMONSTERDEADTARGET,				// Monsters stop targetting dead things
	PGS_COJUMPREGARDLESS,					// Regardless Jump (legacy mishap)
	PGS_COOLDLASTLOOKLOGIC,				// Old lastlook Logic
	PGS_CORADIALSPAWNCHECK,				// Perform radial spawn check
	PGS_MONENABLEPLAYASMONSTER,			// Enable playing of monsters
	PGS_COKILLSTOPLAYERONE,				// Give kills to player 1
	PGS_PLALLOWSUICIDE,							// Allow Suicides
	PGS_PLSUICIDEDELAY,							// Suicide Delay
	PGS_PLSPAWNWITHMELEEONLY,					// Spawn With Melee Only
	PGS_PLSPAWNWITHRANDOMGUN,					// Spawn With Random Gun
	PGS_COENABLESLOPES,							// Enables Slope Support
	PGS_FUNFLIPLEVELS,							// Levels are flipped
	
	PEXGSNUMBITIDS
} P_XGSBitID_t;

/*** STRUCTURES ***/

/* P_XGSVariable_t -- Variable for game setting */
typedef struct P_XGSVariable_s
{
	// Base
	const P_XGSType_t Type;						// Type of value to conform to
	const P_XGSBitID_t BitID;					// BitID of flag
	const char* Name;							// Name of game setting
	uint32_t MenuTitle;							// Title for menus
	uint32_t Description;						// Description
	const uint8_t GameFlags;					// Game Flags
	const P_XGSDemoRange_t DemoRange;			// Range for "demoversion"
	const uint16_t DemoVersion;					// "demoversion" wrapper
	const int32_t DemoVal[2];					// Demo values (false, true)
	const int32_t DefaultVal;					// Default value wherever
	P_XGSMenuCategory_t Category;				// Category for item
	P_XGSDisplayAs_t DisplayAs;					// Display as this
	const CONL_VarPossibleValue_t* Possible;	// Possible values
	void (*ChangeFunc)(struct P_XGSVariable_s* const a_Bit);
	
	// Settings
	bool_t WasSet;								// Was Set to value?
	int32_t ActualVal;							// Actually set value
	
	// String Value
	char StrVal[PEXGSSTRBUFSIZE];				// String Value
} P_XGSVariable_t;

/*** FUNCTIONS ***/

// Setting Finder
P_XGSBitID_t P_XGSBitForName(const char* const a_Name);
P_XGSVariable_t* P_XGSVarForBit(const P_XGSBitID_t a_Bit);
P_XGSVariable_t* P_XGSVarForName(const char* const a_Name);

// Value Getter
int32_t P_XGSVal(const P_XGSBitID_t a_Bit);
fixed_t P_XGSFix(const P_XGSBitID_t a_Bit);

int32_t P_XGSGetNextValue(const P_XGSBitID_t a_Bit, const bool_t a_Right);

// General Functions
bool_t P_XGSRegisterStuff(void);
bool_t P_XGSSetAllDefaults(void);
bool_t P_XGSSetVersionLevel(const bool_t a_Master, const uint32_t a_Level);
int32_t P_XGSSetValue(const bool_t a_Master, const P_XGSBitID_t a_Bit, const int32_t a_Value);
int32_t P_XGSSetValueStr(const bool_t a_Master, const P_XGSBitID_t a_Bit, const char* const a_Value);

// New Game Control
void NG_ResetVars(void);
void NG_FromCLine(void);
void NG_ApplyVars(void);
void NG_Execute(void);

void NG_SetAutoStart(const bool_t a_Value);
bool_t NG_IsAutoStart(void);

bool_t NG_SetRules(const char* const a_Name);

int32_t NG_SetVarValue(const P_XGSBitID_t a_Bit, const int32_t a_NewVal);
int32_t NG_SetVarDefault(const P_XGSBitID_t a_Bit);
int32_t NG_GetNextValue(const P_XGSBitID_t a_Bit, const bool_t a_Right);

#endif							/* __P_DEMCMP_H__ */

