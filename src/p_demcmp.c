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

#include "doomdef.h"
#include "p_demcmp.h"
#include "m_menu.h"
#include "g_input.h"
#include "p_local.h"
#include "g_game.h"

/*****************************
*** EXTENDED GAME SETTINGS ***
*****************************/

/*** STRUCTURES ***/

/* P_EXGSNiceVersion_t -- Nice version information */
typedef struct P_EXGSNiceVersion_s
{
	uint16_t VersionID;							// Version ID
	const char* const NiceName;					// Nice name for version
} P_EXGSNiceVersion_t;

/*** LOCALS ***/

// c_PEXGSPVInteger -- Signed Integer
const CONL_VarPossibleValue_t c_PEXGSPVInteger[] =
{
	// End
	{-32767, "MINVAL"},
	{32767, "MAXVAL"},
	{0, NULL},
};

// c_PEXGSPVPositive -- Positive Integer
const CONL_VarPossibleValue_t c_PEXGSPVPositive[] =
{
	// End
	{0, "MINVAL"},
	{32767, "MAXVAL"},
	{0, NULL},
};

// c_PEXGSPVFixed -- Signed Fixed Value
const CONL_VarPossibleValue_t c_PEXGSPVFixed[] =
{
	// End
	{-32767 << FRACBITS, "MINVAL"},
	{32767 << FRACBITS, "MAXVAL"},
	{0, NULL},
};

// c_PEXGSPVBoolean -- Boolean
const CONL_VarPossibleValue_t c_PEXGSPVBoolean[] =
{
	// False
	{0, "no"},
	{0, "off"},
	{0, "false"},
	
	// True
	{1, "yes"},
	{1, "on"},
	{1, "true"},
	
	// End
	{0, "MINVAL"},
	{1, "MAXVAL"},
	{0, NULL},
};

// c_PEXGSPVTeamPlay -- Team Play Value
const CONL_VarPossibleValue_t c_PEXGSPVTeamPlay[] =
{
	{0, "Off"},
	{1, "Color"},
	{2, "Skin"},	
	
	// End
	{0, "MINVAL"},
	{2, "MAXVAL"},
	{0, NULL},
};

// c_PEXGSPVSkill -- Current Skill Level
const CONL_VarPossibleValue_t c_PEXGSPVSkill[] =
{
	// I'm Too Young To Die
	{0, "I\'m Too Young To Die"},
	{0, "Thou Needeth A Wet-Nurse"},
	{0, "baby"},
	{0, "itytd"},
	{0, "tnawn"},
	
	// Hey, Not Too Rough
	{1, "Hey, Not Too Rough"},
	{1, "Yellowbellies-R-Us"},
	{1, "easy"},
	{1, "hntr"},
	{1, "ntr"},
	{1, "yru"},
	{1, "ybru"},
	
	// Hurt Me Plenty
	{2, "Hurt Me Plenty"},
	{2, "Bringest Them Oneth"},
	{2, "medium"},
	{2, "hmp"},
	{2, "bto"},
	
	// Ultra-Violence
	{3, "Ultra-Violence"},
	{3, "Thou Art A Smite-Meister"},
	{3, "hard"},
	{3, "uv"},
	{3, "taasm"},
	{3, "tasm"},
	
	// Nightmare!
	{4, "Nightmare!"},
	{4, "Black Plague Posseses Thee"},
	{4, "nightmare"},
	{4, "nm"},
	{4, "nmare"},
	{4, "bppt"},
	{4, "plague"},
	
	// End
	{0, "MINVAL"},
	{4, "MAXVAL"},
	{0, NULL},
};

/* c_PEXGSPVKillCountMode -- Kill Count Mode */
const CONL_VarPossibleValue_t c_PEXGSPVKillCountMode[] =
{
	{0, "Always"},
	{1, "Once"},
	{2, "Deads"},
	
	// End
	{0, "MINVAL"},
	{2, "MAXVAL"},
	{0, NULL},
};

/* c_PEXGSPVLastLookMP -- Last Look Modulo */
const CONL_VarPossibleValue_t c_PEXGSPVLastLookMP[] =
{
	// Powers of 2
	{1, "1"},
	{2, "2"},
	{4, "4"},
	{8, "8"},
	{16, "16"},
	{32, "32"},	
	
	// End
	{1, "MINVAL"},
	{MAXPLAYERS, "MAXVAL"},
	{0, NULL},
};

// l_NiceVersions -- Nice names for versions
static const P_EXGSNiceVersion_t l_NiceVersions[] =
{
	// Standard Doom
	{109, "Doom v1.9"},
	{110, "Linux Doom"},
	
	// Doom Legacy
	{111, "Doom Legacy 1.11"},
	{112, "Doom Legacy 1.12"},
	{123, "Doom Legacy 1.23"},
	{125, "Doom Legacy 1.25"},
	{128, "Doom Legacy 1.28"},
	{129, "Doom Legacy 1.29"},
	{131, "Doom Legacy 1.31"},
	{132, "Doom Legacy 1.32"},
	{140, "Doom Legacy 1.40"},
	{141, "Doom Legacy 1.41"},
	{142, "Doom Legacy 1.42"},
	
	// ReMooD
	{200, "ReMooD 1.0a"},
	
	{0, NULL},
};

/* P_EXGSChangeFunc_ITEMSKEEPWEAPONS() -- ITEMSKEEPWEAPONS Changed */
void P_EXGSChangeFunc_ITEMSKEEPWEAPONS(struct P_EXGSVariable_s* const a_Bit)
{
	/* Weapons are now being keeped */
	// So respawn them all
	if (P_EXGSGetValue(PEXGSBID_ITEMSKEEPWEAPONS))
		P_RespawnWeapons();
}

/* P_EXGSChangeFunc_GAMEFRAGLIMIT() -- Frag Limit */
void P_EXGSChangeFunc_GAMEFRAGLIMIT(struct P_EXGSVariable_s* const a_Bit)
{
	size_t i;
	
	if (P_EXGSGetValue(PEXGSBID_GAMEFRAGLIMIT))
		for (i = 0; i < MAXPLAYERS; i++)
			P_CheckFragLimit(&players[i]);
}

// l_GSVars -- Game state variables
	// NOTE THAT DOCUMENTATION THE OLDER YOU GET GETS MORE SCARCE AND MORE
	// LESS INFORMATIVE THE FURTHER AWAY INTO THE PAST IT HAS BEEN. SO IF A
	// VERSION IDENTIFIER IS HERE AND YOU HAVE PROOF THAT IT WRONG, THEN PLEASE
	// INFORM ME.
	//
	// However, Luckily for me, Doom Legacy v1.11's source (the first version)
	// is in idgames (sources/doomlegacy1_src.zip) and Fraggle gave me the
	// source to 1.25. There is a huge hole in between because the source was
	// never really released and if it ever was, it was lost.
static P_EXGSVariable_t l_GSVars[PEXGSNUMBITIDS] =
{
	{PEXGST_INTEGER, PEXGSBID_NOTHINGHERE, "nothinghere", "Nothing",
		"Nothing is here", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 0}, 0,
		PEXGSMC_NONE, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COENABLEBLOODSPLATS, "co_enablebloodsplats", "Enable Blood Splats",
		"Enables blood spats on walls. [Legacy >= 1.29]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 129, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_CORANDOMLASTLOOK, "co_randomlastlook", "Randomize Monster Last Look",
		"Randomize monster's last look (player to target). [Legacy >= 1.29]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 129, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COUNSHIFTVILERAISE, "co_unshiftvileraise", "Unshift Arch-Vile Resurrection",
		"Multiply the corpse height by 4 on resurrects. [Legacy < 1.29]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 129, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COMODIFYCORPSE, "co_modifycorpse", "Modify Corpse (Solid Corpses)",
		"Enables correct corpse modification for solid corpses. [Legacy >= 1.31]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 131, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	// Smoke trails appear in v1.11, but only for rockets
	{PEXGST_INTEGER, PEXGSBID_CONOSMOKETRAILS, "co_nosmoketrails", "No Smoke Trails",
		"Disable smoke trails on rockets and lost souls [Legacy < 1.11]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 111, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	// 1.25 and up, a new smoke sprite is used, rather than tracer smoke.	
	{PEXGST_INTEGER, PEXGSBID_COUSEREALSMOKE, "co_userealsmoke", "Use Real Smoke For Trails",
		"Use actual smoke rather than tracers for trails. [Legacy >= 1.25]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 125, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_COOLDCUTCORPSERADIUS, "co_oldcutcorpseradius", "Cut Corpse Radius (Solid Corpse)",
		"Reduce corpse radius, when co_modifycorpse is off. [Legacy >= 1.12]", PEXGSGM_ANY, PEXGSDR_GREATERTHAN, 112, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COSPAWNDROPSONMOFLOORZ, "co_spawndropsonmofloorz", "Spawn Drops on Fake-Floor",
		"Item drops on the fake floor not on the sector floor. [Legacy >= 1.32]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 132, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_CODISABLETEAMPLAY, "co_disableteamplay", "Disable Team Play",
		"Disable support for team play. [Legacy < 1.25]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 125, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COSLOWINWATER, "co_moveslowinwater", "Move Slower In Water",
		"Move slower when underwater (in 3D Water). [Legacy >= 1.28]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 128, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COSLIDEOFFMOFLOOR, "co_slideoffmofloor", "Dead Things Slide Off Fake Floors",
		"Slide off the near floor not the real one (3D floors). [Legacy >= 1.32]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 132, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COOLDFRICTIONMOVE, "co_oldfrictionmove", "Old Friction Movement",
		"Use old friction when moving. [Legacy < 1.32]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 132, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COOUCHONCEILING, "co_ouchonceiling", "Go Ouch When Hitting Ceiling",
		"Go ouch when hitting the ceiling. [Legacy >= 1.12]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 112, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COENABLESPLASHES, "co_enablesplashes", "Enable Water Splashes",
		"Enable splashes on the water. [Legacy >= 1.25]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 125, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COENABLEFLOORSMOKE, "co_enablefloorsmoke", "Enable Floor Damage Smoke",
		"Enable smoke when on a damaging floor. [Legacy >= 1.25]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 125, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COENABLESMOKE, "co_enablesmoke", "Enable Smoke",
		"Enable smoke. [Legacy >= 1.25]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 125, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_CODAMAGEONLAND, "co_damageonland", "Instant-Damage On Special Floors",
		"Damage when landing on a damaging floor. [Legacy >= 1.25]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 125, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COABSOLUTEANGLE, "co_absoluteangle", "Use Absolute Angles",
		"Use absolute angle rather than relative angle. [Legacy >= 1.25]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 125, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COOLDJUMPOVER, "co_oldjumpover", "Old Jump Over",
		"Use old jump over code. [Legacy < 1.28]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 128, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COENABLESPLATS, "co_enablesplats", "Enable Splats",
		"Enable splats. [Legacy >= 1.28]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 128, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COOLDFLATPUSHERCODE, "co_oldflatpushercode", "Old Pusher/Puller Code",
		"Use pusher/puller code that cannot handle 3D Floors. [Legacy <= 1.40]", PEXGSGM_ANY, PEXGSDR_ATMOST, 140, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COSPAWNPLAYERSEARLY, "co_spawnplayersearly", "Spawn Players Earlier",
		"Spawn players while the map is loading. [Legacy >= 1.28]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 128, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COENABLEUPDOWNSHOOT, "co_enableupdownshoot", "Enable Up/Down Aim",
		"Enable shooting up/down when not aiming at something. [Legacy >= 1.28]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 128, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_CONOUNDERWATERCHECK, "co_nounderwatercheck", "No Underwater Check",
		"Do not check for an object being underwater. [Legacy < 1.28]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 128, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COSPLASHTRANSWATER, "co_transwatersplash", "Water Transition Splash",
		"Causes splashes when transitioning from/to water [Legacy >= 1.32]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 132, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COUSEOLDZCHECK, "co_useoldzcheck", "Old Z Check",
		"Use old Z checking code rather than Heretic's. [Legacy < 1.31]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 131, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COCHECKXYMOVE, "co_checkxymove", "Check X/Y Movement",
		"Check X/Y Movement (When co_useoldzcheck is enabled). [Legacy >= 1.12]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 112, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COWATERZFRICTION, "co_waterzfriction", "Water Z Friction",
		"Apply Z friction movement when underwater. [Legacy >= 1.28]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 128, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_CORANOMLASTLOOKSPAWN, "co_randomlastlookspawn", "Randomize Last Look (Spawn)",
		"Choose random player when object spawns. [Legacy < 1.29]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 129, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COALWAYSRETURNDEADSPMISSILE, "co_alwaysretdeadspmissile", "Return Dead Missiles",
		"Always return the missile spawned even if it dies. [Legacy < 1.31]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 131, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COUSEMOUSEAIMING, "co_usemouseaiming", "Allow Mouse To Aim",
		"Use mouse aiming when not aimed at another object. [Legacy >= 1.28]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 128, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COFIXPLAYERMISSILEANGLE, "co_fixplayermissileangle", "Fix Player Missiles",
		"Fix player missiles being fired and make them more accurate. [Legacy >= 1.28]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 128, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COREMOVEMOINSKYZ, "co_removemissileinskyz", "Remove Missiles In Sky",
		"When Z movement is performed in a sky, do not explode. [Legacy >= 1.29]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 129, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COFORCEAUTOAIM, "co_forceautoaim", "Force Auto-Aim",
		"Always force auto-aim. [Legacy <= 1.11]", PEXGSGM_ANY, PEXGSDR_ATMOST, 111, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COFORCEBERSERKSWITCH, "co_forceberserkswitch", "Force Berserk Switch",
		"Force switching to berserk-enabled weapons in slots. [Legacy < 1.28]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 128, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_CODOUBLEPICKUPCHECK, "co_doublepickupcheck", "Double-Check Pickup",
		"Double check for pickups rather than just once. [Legacy >= 1.32]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 132, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_CODISABLEMISSILEIMPACTCHECK, "co_disablemissileimpactcheck", "No Missile Impact Check",
		"Disable the checking of missile impacts. [Legacy < 1.32]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 132, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COMISSILESPLATONWALL, "co_missilesplatsonwalls", "Enable Missile Splats on Walls",
		"When missiles hit walls, they splat it. [Legacy >= 1.29]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 129, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_CONEWBLOODHITSCANCODE, "co_newbloodhitscancode", "Blood Splat Tracers",
		"Use newer blood spawning code when tracing hitscans. [Legacy >= 1.25]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 125, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_CONEWAIMINGCODE, "co_newaimingcode", "New Aiming Code",
		"Use newer aiming code in P_AimLineAttack(). [Legacy >= 1.28]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 128, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COMISSILESPECHIT, "co_missilespechit", "Missiles Trigger Special Line Hits",
		"Missiles can trigger special line hits. [Legacy >= 1.32]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 132, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COHITSCANSSLIDEONFLATS, "co_hitscanslidesonflats", "Hitscans Slide On Floor",
		"Hitscans slide on flats. [Legacy < 1.12]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 112, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_CONONSOLIDPASSTHRUOLD, "co_nonsolidpassthruold", "No Solid Pass-Thru A",
		"Non-solid objects pass through others (Old trigger). [Legacy < 1.12]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 112, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_CONONSOLIDPASSTHRUNEW, "co_nonsolidpassthrunew", "No Solid Pass-Thru B",
		"Non-solid objects pass through others (New trigger). [Legacy >= 1.32]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 132, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COJUMPCHECK, "co_checkjumpover", "Check For Jump Over",
		"Allow jump over to take effect. [Legacy >= 1.12]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 112, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COLINEARMAPTRAVERSE, "co_linearmaptraverse", "Linear Map Traverse",
		"Loads a new map rather than starting from fresh. [Legacy < 1.29]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 129, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COONLYTWENTYDMSPOTS, "co_onlytwentydmspots", "Limit to 20 DM Starts",
		"Support only 20 DM starts rather than 64. [Legacy < 1.23]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 123, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COALLOWSTUCKSPAWNS, "co_allowstuckspawns", "Allow stuck DM spawns",
		"Allow players getting stuck in others in DM spots. [Legacy < 1.13]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 113, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COUSEOLDBLOOD, "co_useoldblood", "Use Old Doom Blood",
		"Uses standard Doom blood rather than Legacy blood. [Legacy < 130]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 130, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_FUNMONSTERFFA, "fun_monsterffa", "Monster Free For All",
		"Monsters enter a Free For All and attack anything in sight.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 0}, 0,
		PEXGSMC_FUN, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_FUNINFIGHTING, "fun_monsterinfight", "Monsters Infight",
		"Monsters attack monsters of the same race.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 0}, 0,
		PEXGSMC_FUN, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COCORRECTVILETARGET, "co_correctviletarget", "Correct Position Of Vile Vire",
		"Correct the position of the Arch-Vile target fire. [ReMooD >= 1.0a]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 200, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_FUNMONSTERSMISSMORE, "fun_monstersmissmore", "Monsters Miss More",
		"Monsters miss their target more.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 0}, 0,
		PEXGSMC_FUN, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COMORECRUSHERBLOOD, "co_morecrusherblood", "More Crusher Blood",
		"Make crushers spew more blood. [Legacy < 1.32]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 132, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_CORANDOMBLOODDIR, "co_randomblooddir", "Random Blood Direction",
		"Spew blood in a random direction. [Legacy >= 1.28]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 128, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COINFINITEROCKETZ, "co_infiniterocketz", "Infinite Rocket Z Range",
		"Rocket damage distance on Z is infinite. [Legacy < 1.12]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 112, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COALLOWROCKETJUMPING, "co_allowrocketjump", "Allow Rocket Jumping",
		"Allow support for rocket jumping. [Legacy >= 1.29]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 129, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COROCKETZTHRUST, "co_rocketzthrust", "Rocket Z Thrust",
		"Allow thrusting on the Z plane from rockets. [Legacy >= 1.24]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 124, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COLIMITMONSTERZMATTACK, "co_limitmonsterzmattack", "Limit Monster Z Melee Range",
		"Limits Z Melee Attack range (stops melee from cliffs). [Legacy > 1.11]", PEXGSGM_ANY, PEXGSDR_MORETHAN, 111, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_HEREMONSTERTHRESH, "here_monsterthresh", "Heretic Monster Threshold",
		"Use Heretic Threshold Logic. [Heretic]", PEXGSGM_HERETIC, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_HERETIC, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COVOODOODOLLS, "co_voodoodolls", "Enable Voodoo Dolls",
		"Enable spawning of Voodoo Dolls. [Legacy < 1.28]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 128, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	// Extra Puff in A_SmokeTrailer() before v1.25
	{PEXGST_INTEGER, PEXGSBID_COEXTRATRAILPUFF, "co_extratrailpuff", "Extra Smoke Trails",
		"Add extra puff for smoke. [Legacy < 1.25]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 125, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	// - Soul trails existed since 1.25, so that is the first ver?
	{PEXGST_INTEGER, PEXGSBID_COLOSTSOULTRAILS, "co_lostsoultrails", "Lost Soul Trails",
		"Lost souls emit smoke. [Legacy >= 1.25]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 125, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	// In Legacy 1.11, two sided walls were transparent!
	{PEXGST_INTEGER, PEXGSBID_COTRANSTWOSIDED, "co_transtwosided", "Transparent 2D Walls",
		"Transparent two sided walls. [Legacy = 1.11]", PEXGSGM_ANY, PEXGSDR_EQUALS, 111, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	// - Exists in v1.25
	// - Mentioned that it changed in v1.24
	// - Does not exist in v1.11
	// - Possibly appeared in v1.23?
	{PEXGST_INTEGER, PEXGSBID_COENABLEBLOODTIME, "co_enablebloodtime", "Enable Blood Time",
		"Enables setting blood time. [Legacy >= 1.23]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 123, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	// Since v1.12, there was jumping
	{PEXGST_INTEGER, PEXGSBID_COENABLEJUMPING, "co_enablejumping", "Enable Jumping",
		"Enables Jumping Support [Legacy >= 1.12]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 112, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	// After v1.11, Enable mouse aiming
	{PEXGST_INTEGER, PEXGSBID_COMOUSEAIM, "co_mouseaim", "Enable Mouse Aiming",
		"Enable mouse aiming [Legacy > 1.11]", PEXGSGM_ANY, PEXGSDR_MORETHAN, 111, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_MONRESPAWNMONSTERS, "mon_respawnmonsters", "Respawn Monsters",
		"Monsters come back to life after a short delay.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_MONSTERS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_FUNNOTARGETPLAYER, "fun_noplayertarget", "No Player Targetting",
		"Monsters are incapable of targetting players.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_FUN, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_MONARCHVILEANYRESPAWN, "mon_archvileanyrespawn", "Arch-Viles Respawn Anything",
		"Arch-Viles can ressurect anything regardless if it can be or not.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_FUN, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_COOLDCHECKPOSITION, "co_oldcheckposition", "Old Position Checking",
		"Use old P_CheckPosition() Code. [Legacy < 1.42]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 142, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_COLESSSPAWNSTICKING, "co_lessspawnsticking", "Less Spawn Spot Sticking",
		"Make players getting stuck inside other players less likely to occur. [ReMooD >= 1.0a]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 200, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_PLSPAWNTELEFRAG, "pl_spawntelefrag", "Tele-Frag When Spawning",
		"Tele-frag when a player respawns (to empty the spot). [ReMooD >= 1.0a]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 200, {0, 1}, 1,
		PEXGSMC_PLAYERS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_GAMEONEHITKILLS, "game_onehitkills", "One Hit Kills",
		"Any recieved damage kills.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_GAME, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_COBETTERPLCORPSEREMOVAL, "co_betterplbodyqueue", "Better Body Management",
		"Better management of player bodies so they do not litter everywhere. [ReMooD >= 1.0a]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 200, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_PLSPAWNCLUSTERING, "pl_spawnclustering", "Spawn Spot Clustering",
		"Adds extra spawn spots near other spawn spots for more players. [ReMooD >= 1.0a]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 200, {0, 1}, 1,
		PEXGSMC_PLAYERS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_COIMPROVEDMOBJONMOBJ, "co_improvedmobjonmobj", "Improved Object on Object",
		"Improves handling of objects on top of other objects. [ReMooD >= 1.0a]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 200, {0, 0}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_COIMPROVEPATHTRAVERSE, "co_improvepathtraverse", "Improve Traversing Move",
		"Smooth out position moving. [ReMooD >= 1.0a]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 200, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_FLOAT, PEXGSBID_PLJUMPGRAVITY, "pl_jumpgravity", "Jump Gravity",
		"This is the amount of pushing force used when jumping.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, (6 * FRACUNIT)}, (6 * FRACUNIT),
		PEXGSMC_PLAYERS, 0, c_PEXGSPVFixed, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_FUNNOLOCKEDDOORS, "fun_nolockeddoors", "No Locked Doors",
		"All doors are unlocked and do not need keys.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_FUN, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_FLOAT, PEXGSBID_GAMEAIRFRICTION, "game_airfriction", "Friction In Air",
		"This modifies the amount of friction in the air, the higher the easier it is to move.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 8192}, 8192,
		PEXGSMC_GAME, 0, c_PEXGSPVFixed, NULL},
	
	{PEXGST_FLOAT, PEXGSBID_GAMEWATERFRICTION, "game_waterfriction", "Friction In Water",
		"This modifies the amount of friction in water, the higher the easier it is to move.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 49152}, 49152,
		PEXGSMC_GAME, 0, c_PEXGSPVFixed, NULL},
		
	{PEXGST_FLOAT, PEXGSBID_GAMEMIDWATERFRICTION, "game_midwaterfriction", "Friction In Mid-Water",
		"This modifies the amount of friction in water when not touching the ground, the higher the easier it is to move.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 32768}, 32768,
		PEXGSMC_GAME, 0, c_PEXGSPVFixed, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_GAMEALLOWLEVELEXIT, "game_allowlevelexit", "Allow Level Exiting",
		"Allows players or monsters to exit the level.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 1,
		PEXGSMC_GAME, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_GAMEALLOWROCKETJUMP, "game_allowrocketjump", "Allow Rocket Jumping",
		"Enables the use of rocket jumping.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_GAME, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_PLALLOWAUTOAIM, "pl_allowautoaim", "Allow Auto-Aiming",
		"Allows players to aim vertically automatically.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 1,
		PEXGSMC_PLAYERS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_PLFORCEWEAPONSWITCH, "pl_forceweaponswitch", "Force Weapon Switch",
		"Forces weapon switches on pickup. [Doom <= 1.09]", PEXGSGM_ANY, PEXGSDR_ATMOST, 109, {0, 1}, 0,
		PEXGSMC_PLAYERS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_PLDROPWEAPONS, "pl_dropweapons", "Drop Weapons on Death",
		"Drops the player's weapon when they are killed.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_PLAYERS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_PLINFINITEAMMO, "pl_infiniteammo", "Infinite Ammo",
		"Ammo is never depleted and lasts forever.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_PLAYERS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_GAMEHERETICGIBBING, "game_hereticgibbing", "Heretic Gibbing",
		"Objects that can be gibbed are much easier to gib.", PEXGSGM_HERETIC, PEXGSDR_NOCHECK, 0, {0, 1}, 1,
		PEXGSMC_HERETIC, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_MONPREDICTMISSILES, "mon_predictmissile", "Predict Missiles",
		"Monsters predict missile targets and aim accordingly.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_MONSTERS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_MONRESPAWNMONSTERSTIME, "mon_respawnmonsterstime", "Monster Respawn Delay",
		"Time in seconds before monsters are respawned.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 12}, 12,
		PEXGSMC_MONSTERS, PEXGSDA_TIMESECS, c_PEXGSPVPositive, NULL},
		
	
	{PEXGST_INTEGER, PEXGSBID_PLSPAWNWITHMAXGUNS, "pl_spawnwithmaxguns", "Spawn With Non-Super Weapons",
		"When a player is spawned, they have all non-super weapons.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_PLAYERS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_PLSPAWNWITHSUPERGUNS, "pl_spawnwithsuperguns", "Spawn With Super Weapons",
		"When a player is spawned, they have all super weapons.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_PLAYERS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_PLSPAWNWITHMAXSTATS, "pl_spawnwithmaxstats", "Spawn With Max Stats",
		"When a player is spawned, they have max health and armor.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_PLAYERS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_ITEMSSPAWNPICKUPS, "item_spawnpickups", "Spawn Pickups",
		"Spawn pickups on map load.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 1,
		PEXGSMC_ITEMS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COHERETICFRICTION, "co_hereticfriction", "Heretic Friction",
		"Use Heretic Friction", PEXGSGM_HERETIC, PEXGSDR_ATLEAST, 0, {0, 1}, 0,
		PEXGSMC_HERETIC, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_GAMEDEATHMATCH, "game_deathmatch", "Deathmatch",
		"Enables Deathmatch Mode (Player vs Player)", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_GAME, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_PLSPAWNWITHALLKEYS, "pl_spawnwithallkeys", "Spawn With All Keys",
		"When a player is spawned, they have every key.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_PLAYERS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_ITEMSKEEPWEAPONS, "item_keepweapons", "Keep Weapons On Floor",
		"Keep weapons on the ground when picked up.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 1,
		PEXGSMC_ITEMS, PEXGSDA_YESNO, c_PEXGSPVBoolean, P_EXGSChangeFunc_ITEMSKEEPWEAPONS},
		
	{PEXGST_INTEGER, PEXGSBID_GAMETEAMPLAY, "game_teamplay", "Enable Team Play",
		"Enable team mode, teams of different colors vs others.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_GAME, PEXGSDA_STRING, c_PEXGSPVTeamPlay, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_GAMETEAMDAMAGE, "game_teamdamage", "Allow Team Damage",
		"Allow players on the same team to hurt each other.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_GAME, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_GAMEFRAGLIMIT, "game_fraglimit", "Frag Limit",
		"How many frags a player or team must obtain before they achieve victory,", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 0}, 0,
		PEXGSMC_GAME, PEXGSDA_INTEGER, c_PEXGSPVPositive, P_EXGSChangeFunc_GAMEFRAGLIMIT},
		
	{PEXGST_INTEGER, PEXGSBID_GAMETIMELIMIT, "game_timelimit", "Time Limit",
		"Time in minutes before the game automatically ends.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 0}, 0,
		PEXGSMC_GAME, PEXGSDA_TIMEMINS, c_PEXGSPVPositive, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_MONSTATICRESPAWNTIME, "mon_staticrespawntime", "Static Monster Respawn Time",
		"Monsters always come back after respawn delay rather than randomly afterwards.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_MONSTERS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_PLFASTERWEAPONS, "pl_fasterweapons", "Weapons Are Faster",
		"Weapons fire and move a bit faster (all speeds are set to 1).", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_PLAYERS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_MONSPAWNMONSTERS, "mon_spawnmonsters", "Spawn Monsters",
		"Monsters are spawned when the level loads.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 1,
		PEXGSMC_MONSTERS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_GAMESPAWNMULTIPLAYER, "game_spawnmultiplayer", "Spawn Multi-Player Objects",
		"Spawn any items that are marked multiplayer (extra weapons, monsters, etc.)", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_GAME, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_ITEMRESPAWNITEMS, "item_respawnitems", "Respawn Items",
		"Items are respawned after they are picked up.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_ITEMS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_ITEMRESPAWNITEMSTIME, "item_respawnitemstime", "Item Respawn Delay",
		"Time in seconds before items are respawned.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 30}, 30,
		PEXGSMC_ITEMS, PEXGSDA_TIMESECS, c_PEXGSPVPositive, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_MONFASTMONSTERS, "mon_spawnmonsters", "Fast Monsters",
		"Monster move faster and their attacks are also faster.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_MONSTERS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_GAMESOLIDCORPSES, "game_solidcorpses", "Solid Corpses",
		"Corpses on the ground are solidified and could be killed again.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_GAME, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_GAMEBLOODTIME, "game_bloodtime", "Blood Time",
		"Time in seconds blood will last on the gound.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 20}, 20,
		PEXGSMC_GAME, PEXGSDA_TIMESECS, c_PEXGSPVPositive, NULL},
		
	{PEXGST_FLOAT, PEXGSBID_GAMEGRAVITY, "game_gravity", "Gravity",
		"The multiplier to the amount of downward force to apply to players that are in the air.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, (1 * FRACUNIT)}, (1 * FRACUNIT),
		PEXGSMC_GAME, 0, c_PEXGSPVFixed, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_MONENABLECLEANUP, "mon_enablecleanup", "Enable Corpse Cleanup",
		"Enable clean up of dead monsters. [ReMooD >= 1.0a]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 200, {0, 1}, 1,
		PEXGSMC_MONSTERS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_MONCLEANUPRESPTIME, "mon_cleanupresptime", "Respawnable Cleanup Time",
		"Time in minutes before dead respawnable monsters are cleaned up.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 120}, 120,
		PEXGSMC_MONSTERS, PEXGSDA_TIMEMINS, c_PEXGSPVPositive, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_MONCLEANUPNONRTIME, "mon_cleanupnonresptime", "Non-Respawnable Cleanup Time",
		"Time in minutes before dead respawnable monsters are cleaned up.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 60}, 60,
		PEXGSMC_MONSTERS, PEXGSDA_TIMEMINS, c_PEXGSPVPositive, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_GAMESKILL, "game_skill", "Skill Level",
		"The current difficulty of the level, the higher the more monsters that appear.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 2}, 2,
		PEXGSMC_GAME, PEXGSDA_STRING, c_PEXGSPVSkill, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_PLHALFDAMAGE, "pl_halfdamage", "Take Half Damage",
		"Players recieve only half of normal damage they recieve.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_PLAYERS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_PLDOUBLEAMMO, "pl_doubleammo", "Get Double Ammo",
		"Players recieve double the amount of ammo they pickup.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_PLAYERS, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_MONKILLCOUNTMODE, "mon_killcountmode", "Kill Count Mode",
		"Specifies the mode at which kill totals are calculated.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 0}, 0,
		PEXGSMC_MONSTERS, PEXGSDA_STRING, c_PEXGSPVKillCountMode, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COOLDBFGSPRAY, "co_oldbfgspray", "Old BFG Spray",
		"Use BFG Ball owner as inflictor rather than the ball itself. [Legacy < ].32]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 132, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COEXPLODEHITFLOOR, "co_explodehitfloor", "Hit Floor When A_Explode",
		"When a state calls A_Explode() the floor is hit. [Legacy >= ].32]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 132, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COBOMBTHRUFLOOR, "co_bombthrufloor", "Bomb Through Floor",
		"Explosions bleed through floors. [Legacy < ].32]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 132, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},

	{PEXGST_INTEGER, PEXGSBID_COOLDEXPLOSIONS, "co_oldexplosions", "Use Old Explosions",
		"Use older explosion code which cannot handle certain aspects. [Legacy < ].32]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 132, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COAIMCHECKFAKEFLOOR, "co_aimcheckfakefloor", "Check 3D Floor When Aiming",
		"Checks local 3D floors to determine if it is possible to aim through them. [Legacy >= ].32]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 132, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_CONEWGUNSHOTCODE, "co_newgunshotcode", "Use New Gunshot Code",
		"Use logically incorrect gunshot code. [Legacy >= ].32]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 132, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COSHOOTCHECKFAKEFLOOR, "co_shootcheckfakefloor", "Check 3D Floor When Shooting",
		"Checks local 3D floors to determine if it is possible to shoot through them. [Legacy >= ].32]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 132, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_COSHOOTFLOORCLIPPING, "co_shootfloorclipping", "Clip Tracers To Surface",
		"When tracing clip against the floor rather than going through it. [Legacy >= ].32]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 132, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_CONEWSSGSPREAD, "co_newssgspread", "Use New SSG Spread",
		"Use a subtle change in the SSG spread code. [Legacy >= ].32]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 132, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_COMONSTERLOOKFORMONSTER, "co_monsterlookmonster", "Monsters Look For Other Monsters",
		"This enables monsters to look for other monsters. [ReMooD >= 1.0a]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 200, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
		
	{PEXGST_INTEGER, PEXGSBID_COOLDTHINGHEIGHTS, "co_oldthingheights", "Use Old Thing Heights",
		"Use the older heights of objects that were changed in Legacy. [Legacy < ].32]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 132, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_COLASTLOOKMAXPLAYERS, "co_lastlookmaxplayers", "Last Look MAXPLAYERS",
		"This is the modulo value when calculating the randomized last look for spawns.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {32, 32}, 32,
		PEXGSMC_COMPAT, PEXGSDA_INTEGER, c_PEXGSPVLastLookMP, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_COMOVECHECKFAKEFLOOR, "co_movecheckfakefloor", "Check 3D Floor When Moving",
		"Checks 3D floors during movement. [Legacy >= ].32]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 132, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_COMULTIPLAYER, "co_multiplayer", "Multi-Player Mode",
		"Enables multiplayer mode checks.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
	
	{PEXGST_INTEGER, PEXGSBID_COBOOMSUPPORT, "co_boomsupport", "Boom Support",
		"Allows changes used from Boom in certain areas. [Legacy >= ].32]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 132, {0, 1}, 1,
		PEXGSMC_COMPAT, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
};

/*** FUNCTIONS ***/

/* P_EXGSVarForBit() -- Get variable for bit */
P_EXGSVariable_t* P_EXGSVarForBit(const P_EXGSBitID_t a_Bit)
{
	size_t i;
	
	/* Check bit */
	if (a_Bit < 0 || a_Bit >= PEXGSNUMBITIDS)
		return NULL;
	
	/* Match? */
	if (a_Bit == l_GSVars[a_Bit].BitID)
		return &l_GSVars[a_Bit];
	
	/* Find it manually */
	for (i = 0; i < PEXGSNUMBITIDS; i++)
		if (a_Bit == l_GSVars[i].BitID)
			return &l_GSVars[i];
	
	/* Not found */
	return NULL;
}

/* P_EXGSVarForName() -- Find variable by name */
P_EXGSVariable_t* P_EXGSVarForName(const char* const a_Name)
{
	size_t i;
	
	/* Check */
	if (!a_Name)
		return NULL;
	
	/* Find it manually */
	for (i = 0; i < PEXGSNUMBITIDS; i++)
		if (strcasecmp(a_Name, l_GSVars[i].Name) == 0)
			return &l_GSVars[i];
	
	/* Not found */
	return NULL;
}

/* P_EXGSGetValue() -- Get value based on bit */
int32_t P_EXGSGetValue(const P_EXGSBitID_t a_Bit)
{
	P_EXGSVariable_t* Var;
	
	/* Get variable first */
	Var = P_EXGSVarForBit(a_Bit);
	
	// Nothing?
	if (!Var)
		return 0;
	
	/* Just return value */
	// Was never set? Return default
	if (!Var->WasSet)
		return Var->DefaultVal;
	
	// Otherwise return to what it was set to
	else
		return Var->ActualVal;
}

/* P_EXGSGetFixed() -- Gets fixed_t value */
fixed_t P_EXGSGetFixed(const P_EXGSBitID_t a_Bit)
{
	return P_EXGSGetValue(a_Bit);
}

/* P_EXGSGetNextValue() -- Gets the next value in a certain direction */
int32_t P_EXGSGetNextValue(const P_EXGSBitID_t a_Bit, const bool_t a_Right)
{
	P_EXGSVariable_t* Var;
	int32_t MinVal, MaxVal, ReqVal, i;
	int32_t ixMi, ixMa;
	
	/* Get variable first */
	Var = P_EXGSVarForBit(a_Bit);
	
	// Nothing?
	if (!Var)
		return 0;
	
	/* Get Min/Max Values */
	// Possible Values
	ixMi = ixMa = -1;
	if (Var->Possible)
		// Check values and obtain min/max too
		for (i = 0; Var->Possible[i].StrAlias; i++)
		{
			// Min?
			if (strcasecmp("MINVAL", Var->Possible[i].StrAlias) == 0)
			{
				ixMi = i;
				MinVal = Var->Possible[i].IntVal;
			}
			
			// Max?
			else if (strcasecmp("MAXVAL", Var->Possible[i].StrAlias) == 0)
			{
				ixMa = i;
				MaxVal = Var->Possible[i].IntVal;
			}
		}
	
	// There are no possible values
	else
	{
		MinVal = c_PEXGSPVFixed[0].IntVal;
		MaxVal = c_PEXGSPVFixed[1].IntVal;
	}
	
	/* Start at the curernt value */
	if (Var->WasSet)
		ReqVal = Var->ActualVal;
	else
		ReqVal = Var->DefaultVal;
	
	/* Loop until the next is found */
	for (;;)
	{
		// Value at maximum? Return max
		if (a_Right && ReqVal >= MaxVal)
			return MaxVal;
		
		// Value at minimum? Return minimum
		else if (!a_Right && ReqVal <= MinVal)
			return MinVal;
		
		// Move value by single unit
		if (Var->Type == PEXGST_FLOAT)
			ReqVal += 8192 * (a_Right ? 1 : -1);	// Move by .125
		else
			ReqVal += (a_Right ? 1 : -1);			// Move by 1
		
		// There are no enumerated value ranges
		if (ixMi == 0 || ixMa == 0)
			return ReqVal;
		
		// Hit exact value? Then return it
		else
			for (i = 0; Var->Possible[i].StrAlias; i++)
				if (ReqVal == Var->Possible[i].IntVal)
					return ReqVal;
	}
	
	/* Return the requested value */
	return ReqVal;
}

typedef struct M_UIMenuHandler_s M_UIMenuHandler_t;
typedef struct M_UIMenu_s M_UIMenu_t;
M_UIMenuHandler_t* M_ExPushMenu(const uint8_t a_Player, M_UIMenu_t* const a_UI);
M_UIMenu_t* M_ExTemplateMakeGameVars(const int32_t a_Mode);

/* PS_EXGSGeneralComm() -- General command for game settings */
static CONL_ExitCode_t PS_EXGSGeneralComm(const uint32_t a_ArgC, const char** const a_ArgV)
{
	size_t i;
	P_EXGSVariable_t* Var;
	
	/* Pop Menu */
	if (strcasecmp(a_ArgV[0], "menugamevar") == 0)
	{
		M_ExPushMenu(0, M_ExTemplateMakeGameVars(0));
	}
	
	/* Game Settings Control */
	else if (strcasecmp(a_ArgV[0], "gamevar") == 0)
	{
		// Missing argument? (list all settings)
		if (a_ArgC < 2)
		{
			// Print header
			CONL_PrintF("Game settings:\n");
			
			// Print list
			for (i = 0; i < PEXGSNUMBITIDS; i++)
			{
#if 1
				CONL_PrintF("{4%-30s{z = {6%i{z\n", l_GSVars[i].Name, P_EXGSGetValue(l_GSVars[i].BitID));
#else
				CONL_PrintF("{4%-25s{z\n", l_GSVars[i].Name);
				CONL_PrintF("  : {5%.45s{z\n", l_GSVars[i].Description);
				CONL_PrintF("  = %i\n", (l_GSVars[i].WasSet ? l_GSVars[i].ActualVal : l_GSVars[i].DefaultVal));
#endif
			}
			
			// Success!
			return CLE_SUCCESS;
		}
		
		// Two Arguments = Get value of setting
		else if (a_ArgC == 2)
		{
			// Find var?
			Var = P_EXGSVarForName(a_ArgV[1]);
			
			// Not found?
			if (!Var)
				return CLE_UNKNOWNVARIABLE;
			
			// Return value and information
			CONL_PrintF("{4%-30s{z \"{7%s{z\"\n", Var->Name, Var->MenuTitle);
			CONL_PrintF("  Desc: %s\n", Var->Description);
			CONL_PrintF("  Valu: %i\n", P_EXGSGetValue(Var->BitID));
			
			return CLE_SUCCESS;
		}
		
		// Other arguments = Set value of setting
		else
		{
			// Find var?
			Var = P_EXGSVarForName(a_ArgV[1]);
			
			// Not found?
			if (!Var)
				return CLE_UNKNOWNVARIABLE;
			
			// Set value
			P_EXGSSetValueStr(false, Var->BitID, a_ArgV[2]);
			
			return CLE_SUCCESS;
		}
	}
	
	/* Game Compatibility Control */
	else if (strcasecmp(a_ArgV[0], "gameversion") == 0)
	{
		// Missing argument?
		if (a_ArgC < 2)
		{
			// Header
			CONL_PrintF("Compatibility Levels:\n");
			
			// Print possible version numbers
			for (i = 0; l_NiceVersions[i].NiceName || l_NiceVersions[i].VersionID; i++)
			{
				// Print
				CONL_PrintF(" %3i = %s\n", l_NiceVersions[i].VersionID, l_NiceVersions[i].NiceName);
			}
		}
		
		// Otherwise
		else
		{	
			P_EXGSSetVersionLevel(false, atoi(a_ArgV[1]));
		}
		
		// Success!
		return CLE_SUCCESS;
	}
	
	/* Unknown */
	return CLE_INVALIDARGUMENT;
}

/* PS_EXGSValToStr() -- Convert Value to String */
const void PS_EXGSValToStr(P_EXGSVariable_t* const a_Var)
{
	int32_t XVal, a, b, c;
	
	/* Check */
	if (!a_Var)
		return;
	
	/* Get actual value */
	if (a_Var->WasSet)
		XVal = a_Var->ActualVal;
	else
		XVal = a_Var->DefaultVal;
	
	/* Draw as which? */
	switch (a_Var->Type)
	{
		case PEXGST_INTEGER:
			switch (a_Var->DisplayAs)
			{
					// Yes/No
				case PEXGSDA_YESNO:
					if (XVal)
						strncpy(a_Var->StrVal, "Yes", PEXGSSTRBUFSIZE - 1);
					else
						strncpy(a_Var->StrVal, "No", PEXGSSTRBUFSIZE - 1);
					break;
					
					// Time in Minutes
				case PEXGSDA_TIMEMINS:
					// Minutes
					a = XVal % 60;
					
					// Hours
					b = (XVal / 60) % 24;
					
					// Days
					c = (XVal / 60) / 24;
					
					// Minutes Only
					if (!b && !c)
						snprintf(a_Var->StrVal, PEXGSSTRBUFSIZE - 1, "%i m", a);
						
					// Minutes and Hours
					else if (!c)
						snprintf(a_Var->StrVal, PEXGSSTRBUFSIZE - 1, "%i h %i m", b, a);
					
					// Minutes, Hours, Days
					else
						snprintf(a_Var->StrVal, PEXGSSTRBUFSIZE - 1, "%i d %i h %i m", c, b, a);
					break;
					
					// Time in Seconds -- Spaces due to readability issues
				case PEXGSDA_TIMESECS:
					// Seconds
					a = XVal % 60;
					
					// Minutes
					b = (XVal / 60) % 60;
					
					// Hours
					c = (XVal / 60) / 60;
					
					// Seconds Only
					if (!b && !c)
						snprintf(a_Var->StrVal, PEXGSSTRBUFSIZE - 1, "%i s", a);
						
					// Seconds and Minutes
					else if (!c)
						snprintf(a_Var->StrVal, PEXGSSTRBUFSIZE - 1, "%i m %i s", b, a);
					
					// Seconds, Minutes, Hours
					else
						snprintf(a_Var->StrVal, PEXGSSTRBUFSIZE - 1, "%i h %i m %i s", c, b, a);
					break;
					
					// String Value
				case PEXGSDA_STRING:
					// Clear away
					a_Var->StrVal[0] = 0;
					
					// Get Value?
					if (a_Var->Possible)
						for (a = 0; a_Var->Possible[a].StrAlias; a++)
							if (a_Var->Possible[a].IntVal == XVal)
							{
								strncpy(a_Var->StrVal, a_Var->Possible[a].StrAlias, PEXGSSTRBUFSIZE - 1);
								break;
							}
					
					// Missing?
					if (!a_Var->StrVal[0])
						strncpy(a_Var->StrVal, "???", PEXGSSTRBUFSIZE - 1);
					break;
				
					// Plain Integer
				case PEXGSDA_INTEGER:
				default:
					snprintf(a_Var->StrVal, PEXGSSTRBUFSIZE - 1, "%i\n", XVal);
					break;
			}
			break;
		
		case PEXGST_FLOAT:
			snprintf(a_Var->StrVal, PEXGSSTRBUFSIZE - 1, "%g\n", FIXED_TO_FLOAT(XVal));
			break;
		
		default:
			break;
	}
}

/* P_EXGSRegisterStuff() -- Register stuff needed for these things */
bool_t P_EXGSRegisterStuff(void)
{
	size_t i;
	
	/* Run through all vars and set strings */
	for (i = 0; i < PEXGSNUMBITIDS; i++)
		PS_EXGSValToStr(&l_GSVars[i]);
	
	/* Register game setting commands */
	CONL_AddCommand("menugamevar", PS_EXGSGeneralComm);
	CONL_AddCommand("gamevar", PS_EXGSGeneralComm);
	CONL_AddCommand("gameversion", PS_EXGSGeneralComm);
	
	/* Always works! */
	return true;
}

/* P_EXGSSetAllDefaults() -- Set all values to defaults */
bool_t P_EXGSSetAllDefaults(void)
{
	size_t i;
	
	/* Run through the list */
	for (i = 0; i < PEXGSNUMBITIDS; i++)
	{
		// Clear values and unset
		l_GSVars[i].WasSet = 0;
		l_GSVars[i].ActualVal = 0;
		
		// Recreate string
		PS_EXGSValToStr(&l_GSVars[i]);
	}
	
	return true;
}

/* P_EXGSSetVersionLevel() -- Set version level to a specific compatibility level */
bool_t P_EXGSSetVersionLevel(const bool_t a_Master, const uint32_t a_Level)
{
	size_t i;
	bool_t IsTrue;
	uint16_t CheckVs;
	
	/* Run through the list */
	for (i = 0; i < PEXGSNUMBITIDS; i++)
	{
		// Unset
		IsTrue = false;
		CheckVs = l_GSVars[i].DemoVersion;
		
		// Ignore
		if (l_GSVars[i].DemoRange == PEXGSDR_NOCHECK)
			continue;
		
		// Wrong Game?
		if ((g_CoreGame == COREGAME_DOOM && !(l_GSVars[i].GameFlags & PEXGSGM_DOOM)) ||
			(g_CoreGame == COREGAME_HERETIC && !(l_GSVars[i].GameFlags & PEXGSGM_HERETIC)) ||
			(g_CoreGame == COREGAME_HEXEN && !(l_GSVars[i].GameFlags & PEXGSGM_HEXEN)) ||
			(g_CoreGame == COREGAME_STRIFE && !(l_GSVars[i].GameFlags & PEXGSGM_STRIFE)))
			IsTrue = false;
			
		// Which comparison?
		else
			switch (l_GSVars[i].DemoRange)
			{
				case PEXGSDR_EQUALS:
					if (a_Level == CheckVs)
						IsTrue = true;
					break;
				case PEXGSDR_NOT:
					if (a_Level != CheckVs)
						IsTrue = true;
					break;
				case PEXGSDR_LESSTHAN:
					if (a_Level < CheckVs)
						IsTrue = true;
					break;
				case PEXGSDR_GREATERTHAN:
					if (a_Level > CheckVs)
						IsTrue = true;
					break;
				case PEXGSDR_ATMOST:
					if (a_Level <= CheckVs)
						IsTrue = true;
					break;
				case PEXGSDR_ATLEAST:
					if (a_Level >= CheckVs)
						IsTrue = true;
					break;
				default:
					IsTrue = false;
					break;
			}
		
		// Set value
		P_EXGSSetValue(a_Master, l_GSVars[i].BitID, l_GSVars[i].DemoVal[IsTrue]);
	}
	
	/* Complex stuff */
	if (a_Level <= 109)
		P_EXGSSetValue(a_Master, PEXGSBID_GAMETEAMDAMAGE, 1);
	
	if (a_Level <= 109)
		P_EXGSSetValue(a_Master, PEXGSBID_GAMEHERETICGIBBING, 0);
		
	if (a_Level <= 109)
		P_EXGSSetValue(a_Master, PEXGSBID_GAMEAIRFRICTION, 0);
		
	// MAXPLAYERS Simulation
	if (a_Level <= 109)
		P_EXGSSetValue(a_Master, PEXGSBID_COLASTLOOKMAXPLAYERS, 4);
	else if (a_Level < 113)
		P_EXGSSetValue(a_Master, PEXGSBID_COLASTLOOKMAXPLAYERS, 8);
	else
		P_EXGSSetValue(a_Master, PEXGSBID_COLASTLOOKMAXPLAYERS, 32);
	
	return true;
}

/* P_EXGSSetValue() -- Sets value of variable */
int32_t P_EXGSSetValue(const bool_t a_Master, const P_EXGSBitID_t a_Bit, const int32_t a_Value)
{
	P_EXGSVariable_t* Var;
	int32_t MinVal, MaxVal, i, pv;
	bool_t Possibly;
	
	/* Get variable first */
	Var = P_EXGSVarForBit(a_Bit);
	
	// Nothing?
	if (!Var)
		return 0;
	
	/* Not permitted to change value? */
	if (!D_SyncNetIsArbiter() && !a_Master)
	{
		if (Var->WasSet)
			return Var->ActualVal;
		return Var->DefaultVal;
	}
	
	/* Perform value limiting on it (enum) */
	Possibly = false;
	if (Var->Possible)
	{
		pv = 0;
		
		// Check values and obtain min/max too
		for (i = 0; Var->Possible[i].StrAlias; i++)
		{
			// Min?
			if (strcasecmp("MINVAL", Var->Possible[i].StrAlias) == 0)
				MinVal = Var->Possible[i].IntVal;
			
			// Max?
			else if (strcasecmp("MAXVAL", Var->Possible[i].StrAlias) == 0)
				MaxVal = Var->Possible[i].IntVal;
			
			// Exact Match?
			else if (Var->Possible[i].IntVal == a_Value)
			{
				Possibly = true;
				pv = Var->Possible[i].IntVal;
			}
		}
		
		// Found value? Use pv
		if (Possibly)
		{
			Var->WasSet = true;
			Var->ActualVal = pv;
		}
		
		// Otherwise, cap between min/max
		else
		{
			if (a_Value < MinVal)
				Var->ActualVal = MinVal;
			else if (a_Value > MaxVal)
				Var->ActualVal = MaxVal;
			else
				Var->ActualVal = a_Value;
			
			// Set to true, since it would be set anyways
			Var->WasSet = true;
			Possibly = true;
		}
	}
	
	/* Set value */
	if (!Possibly)
	{
		Var->WasSet = true;
		Var->ActualVal = a_Value;
	}
	
	/* Set string */
	PS_EXGSValToStr(Var);
	
	/* Call value change function */
	if (Var->ChangeFunc)
		Var->ChangeFunc(Var);
	
	/* Return the value */
	return Var->ActualVal;
}

/* P_EXGSSetValueStr() -- Sets value by string */
int32_t P_EXGSSetValueStr(const bool_t a_Master, const P_EXGSBitID_t a_Bit, const char* const a_Value)
{
	int32_t SetVal, i;
	P_EXGSVariable_t* Var;
	bool_t Possibly;
	
	/* Check */
	if (!a_Value)
		return 0;
	
	/* Get variable first */
	Var = P_EXGSVarForBit(a_Bit);
	
	// Nothing?
	if (!Var)
		return 0;
	
	/* See if string matches possible value */
	Possibly = false;
	if (Var->Possible)
		for (i = 0; Var->Possible[i].StrAlias; i++)
			if (strcasecmp(a_Value, Var->Possible[i].StrAlias) == 0)
			{
				Possibly = true;
				SetVal = Var->Possible[i].IntVal;
				break;
			}
	
	/* See which value to use here */
	if (!Possibly)
		SetVal = strtol(a_Value, NULL, 0);
	
	/* Return the value */
	return P_EXGSSetValue(a_Master, a_Bit, SetVal);
}

