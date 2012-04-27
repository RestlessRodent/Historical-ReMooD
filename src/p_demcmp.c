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

consvar_t cv_dc_allowjump = { "dc_allowjump", "1", CV_HIDEN, CV_YesNo };
consvar_t cv_dc_allowautoaim = { "dc_allowautoaim", "1", CV_HIDEN, CV_YesNo };
consvar_t cv_dc_forceautoaim = { "dc_forceautoaim", "1", CV_HIDEN, CV_YesNo };
consvar_t cv_dc_allowrocketjump = { "dc_allowrocketjump", "0", CV_HIDEN, CV_YesNo };
consvar_t cv_dc_classicblood = { "dc_classicblood", "0", CV_HIDEN, CV_YesNo };
consvar_t cv_dc_predictingmonsters = { "dc_predictingmonsters", "0", CV_HIDEN, CV_OnOff };
consvar_t cv_dc_classicrocketblast = { "dc_classicrocketblast", "0", CV_HIDEN, CV_YesNo };
consvar_t cv_dc_classicmeleerange = { "dc_classicmeleerange", "0", CV_HIDEN, CV_YesNo };
consvar_t cv_dc_classicmonsterlogic = { "dc_classicmonsterlogic", "0", CV_HIDEN, CV_YesNo };

consvar_t* DemoPair[][2] =
{
	{&cv_allowjump, &cv_dc_allowjump}
	,
	{&cv_allowautoaim, &cv_dc_allowautoaim}
	,
	{&cv_forceautoaim, &cv_dc_forceautoaim}
	,
	{&cv_allowrocketjump, &cv_dc_allowrocketjump}
	,
	{&cv_classicblood, &cv_dc_classicblood}
	,
	{&cv_predictingmonsters, &cv_dc_predictingmonsters}
	,
	{&cv_classicrocketblast, &cv_dc_classicrocketblast}
	,
	{&cv_classicmeleerange, &cv_dc_classicmeleerange}
	,
	{&cv_classicmonsterlogic, &cv_dc_classicmonsterlogic}
	,
};

void DC_RegisterDemoCompVars(void)
{
	CV_RegisterVar(&cv_dc_allowjump);
	CV_RegisterVar(&cv_dc_allowautoaim);
	CV_RegisterVar(&cv_dc_forceautoaim);
	CV_RegisterVar(&cv_dc_allowrocketjump);
	CV_RegisterVar(&cv_dc_classicblood);
	CV_RegisterVar(&cv_dc_predictingmonsters);
	CV_RegisterVar(&cv_dc_classicrocketblast);
	CV_RegisterVar(&cv_dc_classicmeleerange);
	CV_RegisterVar(&cv_dc_classicmonsterlogic);
}

void DC_SetMenuGameOptions(int SetDemo)
{
	int i, j;
	
	if (SetDemo > 1)
		SetDemo = 1;
	else if (SetDemo < 0)
		SetDemo = 0;
		
	for (i = 0; i < GameOptionsDef.numitems; i++)
		if (GameOptionsDef.menuitems[i].status & IT_CVAR)
			for (j = 0; j < sizeof(DemoPair) / sizeof(consvar_t***); j++)
				if (GameOptionsDef.menuitems[i].itemaction == (void*)DemoPair[j][-(SetDemo - 1)])
					GameOptionsDef.menuitems[i].itemaction = (void*)DemoPair[j][SetDemo];
}

void DC_SetDemoOptions(int VerToSet)
{
	// Classic
	switch (VerToSet)
	{
		case 109:
			CV_Set(&cv_dc_allowjump, "0");
			CV_Set(&cv_dc_allowautoaim, "1");
			CV_Set(&cv_dc_forceautoaim, "1");
			CV_Set(&cv_dc_allowrocketjump, "0");
			CV_Set(&cv_dc_classicblood, "1");
			CV_Set(&cv_dc_predictingmonsters, "0");
			CV_Set(&cv_dc_classicrocketblast, "1");
			CV_Set(&cv_dc_classicmeleerange, "1");
			CV_Set(&cv_dc_classicmonsterlogic, "1");
			break;
		default:
			break;
	}
	
	// Legacy
	if (demoversion > 111)
	{
		CV_Set(&cv_dc_classicmeleerange, "0");
	}
	
	if (demoversion >= 124 && demoversion < 129)
	{
		CV_Set(&cv_dc_classicrocketblast, "0");
	}
}

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
		"Nothing is here"},
	{PEXGST_INTEGER, PEXGSBID_COENABLEBLOODSPLATS, "co_enablebloodsplats", "Enable Blood Splats",
		"Enables blood spats on walls. [Legacy >= 1.29]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 129, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_CORANDOMLASTLOOK, "co_randomlastlook", "Randomize Monster Last Look",
		"Randomize monster's last look (player to target). [Legacy >= 1.29]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 129, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COUNSHIFTVILERAISE, "co_unshiftvileraise", "Unshift Arch-Vile Resurrection",
		"Multiply the corpse height by 4 on resurrects. [Legacy < 1.29]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 129, {0, 1}, 0,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COMODIFYCORPSE, "co_modifycorpse", "Modify Corpse (Solid Corpses)",
		"Enables correct corpse modification for solid corpses. [Legacy >= 1.31]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 131, {0, 1}, 1,
		PEXGSMC_COMPAT},
		
	// Smoke trails appear in v1.11, but only for rockets
	{PEXGST_INTEGER, PEXGSBID_CONOSMOKETRAILS, "co_nosmoketrails", "No Smoke Trails",
		"Disable smoke trails on rockets and lost souls [Legacy < 1.11]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 111, {0, 1}, 0,
		PEXGSMC_COMPAT},
	
	// 1.25 and up, a new smoke sprite is used, rather than tracer smoke.	
	{PEXGST_INTEGER, PEXGSBID_COUSEREALSMOKE, "co_userealsmoke", "Use Real Smoke For Trails",
		"Use actual smoke rather than tracers for trails. [Legacy >= 1.25]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 125, {0, 1}, 1,
		PEXGSMC_COMPAT},
	
	{PEXGST_INTEGER, PEXGSBID_COOLDCUTCORPSERADIUS, "co_oldcutcorpseradius", "Cut Corpse Radius (Solid Corpse)",
		"Reduce corpse radius, when co_modifycorpse is off. [Legacy >= 1.12]", PEXGSGM_ANY, PEXGSDR_GREATERTHAN, 112, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COSPAWNDROPSONMOFLOORZ, "co_spawndropsonmofloorz", "Spawn Drops on Fake-Floor",
		"Item drops on the fake floor not on the sector floor. [Legacy >= 1.32]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 132, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_CODISABLETEAMPLAY, "co_disableteamplay", "Disable Team Play",
		"Disable support for team play. [Legacy < 1.25]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 125, {0, 1}, 0,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COSLOWINWATER, "co_moveslowinwater", "Move Slower In Water",
		"Move slower when underwater (in 3D Water). [Legacy >= 1.28]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 128, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COSLIDEOFFMOFLOOR, "co_slideoffmofloor", "Dead Things Slide Off Fake Floors",
		"Slide off the near floor not the real one (3D floors). [Legacy >= 1.32]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 132, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COOLDFRICTIONMOVE, "co_oldfrictionmove", "Old Friction Movement",
		"Use old friction when moving. [Legacy < 1.32]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 132, {0, 1}, 0,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COOUCHONCEILING, "co_ouchonceiling", "Go Ouch When Hitting Ceiling",
		"Go ouch when hitting the ceiling. [Legacy >= 1.12]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 112, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COENABLESPLASHES, "co_enablesplashes", "Enable Water Splashes",
		"Enable splashes on the water. [Legacy >= 1.25]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 125, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COENABLEFLOORSMOKE, "co_enablefloorsmoke", "Enable Floor Damage Smoke",
		"Enable smoke when on a damaging floor. [Legacy >= 1.25]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 125, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COENABLESMOKE, "co_enablesmoke", "Enable Smoke",
		"Enable smoke. [Legacy >= 1.25]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 125, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_CODAMAGEONLAND, "co_damageonland", "Instant-Damage On Special Floors",
		"Damage when landing on a damaging floor. [Legacy >= 1.25]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 125, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COABSOLUTEANGLE, "co_absoluteangle", "Use Absolute Angles",
		"Use absolute angle rather than relative angle. [Legacy >= 1.25]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 125, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COOLDJUMPOVER, "co_oldjumpover", "Old Jump Over",
		"Use old jump over code. [Legacy < 1.28]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 128, {0, 1}, 0,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COENABLESPLATS, "co_enablesplats", "Enable Splats",
		"Enable splats. [Legacy >= 1.28]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 128, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COOLDFLATPUSHERCODE, "co_oldflatpushercode", "Old Pusher/Puller Code",
		"Use pusher/puller code that cannot handle 3D Floors. [Legacy <= 1.40]", PEXGSGM_ANY, PEXGSDR_ATMOST, 140, {0, 1}, 0,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COSPAWNPLAYERSEARLY, "co_spawnplayersearly", "Spawn Players Earlier",
		"Spawn players while the map is loading. [Legacy >= 1.28]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 128, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COENABLEUPDOWNSHOOT, "co_enableupdownshoot", "Enable Up/Down Aim",
		"Enable shooting up/down when not aiming at something. [Legacy >= 1.28]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 128, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_CONOUNDERWATERCHECK, "co_nounderwatercheck", "No Underwater Check",
		"Do not check for an object being underwater. [Legacy < 1.28]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 128, {0, 1}, 0,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COSPLASHTRANSWATER, "co_transwatersplash", "Water Transition Splash",
		"Causes splashes when transitioning from/to water [Legacy >= 1.32]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 132, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COUSEOLDZCHECK, "co_useoldzcheck", "Old Z Check",
		"Use old Z checking code rather than Heretic's. [Legacy < 1.31]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 131, {0, 1}, 0,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COCHECKXYMOVE, "co_checkxymove", "Check X/Y Movement",
		"Check X/Y Movement (When co_useoldzcheck is enabled). [Legacy >= 1.12]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 112, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COWATERZFRICTION, "co_waterzfriction", "Water Z Friction",
		"Apply Z friction movement when underwater. [Legacy >= 1.28]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 128, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_CORANOMLASTLOOKSPAWN, "co_randomlastlookspawn", "Randomize Last Look (Spawn)",
		"Choose random player when object spawns. [Legacy < 1.29]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 129, {0, 1}, 0,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COALWAYSRETURNDEADSPMISSILE, "co_alwaysretdeadspmissile", "Return Dead Missiles",
		"Always return the missile spawned even if it dies. [Legacy < 1.31]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 131, {0, 1}, 0,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COUSEMOUSEAIMING, "co_usemouseaiming", "Allow Mouse To Aim",
		"Use mouse aiming when not aimed at another object. [Legacy >= 1.28]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 128, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COFIXPLAYERMISSILEANGLE, "co_fixplayermissleangle", "Fix Player Missiles",
		"Fix player missiles being fired to be more accurate. [Legacy >= 1.28]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 128, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COREMOVEMOINSKYZ, "co_removemissileinskyz", "Remove Missiles In Sky",
		"When Z movement is performed in a sky, do not explode. [Legacy >= 1.29]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 129, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COFORCEAUTOAIM, "co_forceautoaim", "Force Auto-Aim",
		"Always force auto-aim. [Legacy <= 1.11]", PEXGSGM_ANY, PEXGSDR_ATMOST, 111, {0, 1}, 0,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COFORCEBERSERKSWITCH, "co_forceberserkswitch", "Force Berserk Switch",
		"Force switching to berserk-enabled weapons in slots. [Legacy < 1.28]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 128, {0, 1}, 0,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_CODOUBLEPICKUPCHECK, "co_doublepickupcheck", "Double-Check Pickup",
		"Double check for pickups rather than just once. [Legacy >= 1.32]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 132, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_CODISABLEMISSILEIMPACTCHECK, "co_disablemissileimpactcheck", "No Missile Impact Check",
		"Disable the checking of missile impacts. [Legacy < 1.32]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 132, {0, 1}, 0,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COMISSILESPLATONWALL, "co_missilesplatsonwalls", "Enable Missile Splats on Walls",
		"When missiles hit walls, they splat it. [Legacy >= 1.29]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 129, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_CONEWBLOODHITSCANCODE, "co_newbloodhitscancode", "Blood Splat Tracers",
		"Use newer blood spawning code when tracing hitscans. [Legacy >= 1.25]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 125, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_CONEWAIMINGCODE, "co_newaimingcode", "New Aiming Code",
		"Use newer aiming code in P_AimLineAttack(). [Legacy >= 1.28]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 128, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COMISSILESPECHIT, "co_missilespechit", "Missiles Trigger Special Line Hits",
		"Missiles can trigger special line hits. [Legacy >= 1.32]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 132, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COHITSCANSSLIDEONFLATS, "co_hitscanslidesonflats", "Hitscans Slide On Floor",
		"Hitscans slide on flats. [Legacy < 1.12]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 112, {0, 1}, 0,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_CONONSOLIDPASSTHRUOLD, "co_nonsolidpassthruold", "No Solid Pass-Thru A",
		"Non-solid objects pass through others (Old trigger). [Legacy < 1.12]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 112, {0, 1}, 0,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_CONONSOLIDPASSTHRUNEW, "co_nonsolidpassthrunew", "No Solid Pass-Thru B",
		"Non-solid objects pass through others (New trigger). [Legacy >= 1.32]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 132, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COJUMPCHECK, "co_checkjumpover", "Check For Jump Over",
		"Allow jump over to take effect. [Legacy >= 1.12]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 112, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COLINEARMAPTRAVERSE, "co_linearmaptraverse", "Linear Map Traverse",
		"Loads a new map rather than starting from fresh. [Legacy < 1.29]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 129, {0, 1}, 0,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COONLYTWENTYDMSPOTS, "co_onlytwentydmspots", "Limit to 20 DM Starts",
		"Support only 20 DM starts rather than 64. [Legacy < 1.23]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 123, {0, 1}, 0,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COALLOWSTUCKSPAWNS, "co_allowstuckspawns", "Allow stuck DM spawns",
		"Allow players getting stuck in others in DM spots. [Legacy < 1.13]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 113, {0, 1}, 0,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COUSEOLDBLOOD, "co_useoldblood", "Use Old Doom Blood",
		"Uses standard Doom blood rather than Legacy blood. [Legacy < 130]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 130, {0, 1}, 0,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_FUNMONSTERFFA, "fun_monsterffa", "Monster Free For All",
		"Monsters enter a Free For All and attack anything in sight.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 0}, 0,
		PEXGSMC_FUN},
	{PEXGST_INTEGER, PEXGSBID_FUNINFIGHTING, "fun_monsterinfight", "Monsters Infight",
		"Monsters attack monsters of the same race.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 0}, 0,
		PEXGSMC_FUN},
	{PEXGST_INTEGER, PEXGSBID_COCORRECTVILETARGET, "co_correctviletarget", "Correct Position Of Vile Vire",
		"Correct the position of the Arch-Vile target fire. [ReMooD >= 1.0a]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 200, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_FUNMONSTERSMISSMORE, "fun_monstersmissmore", "Monsters Miss More",
		"Monsters miss their target more.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 0}, 0,
		PEXGSMC_FUN},
	{PEXGST_INTEGER, PEXGSBID_COMORECRUSHERBLOOD, "co_morecrusherblood", "More Crusher Blood",
		"Make crushers spew more blood. [Legacy < 1.32]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 132, {0, 1}, 0,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_CORANDOMBLOODDIR, "co_randomblooddir", "Random Blood Direction",
		"Spew blood in a random direction. [Legacy >= 1.28]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 128, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COINFINITEROCKETZ, "co_infiniterocketz", "Infinite Rocket Z Range",
		"Rocket damage distance on Z is infinite. [Legacy < 1.12]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 112, {0, 1}, 0,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COALLOWROCKETJUMPING, "co_allowrocketjump", "Allow Rocket Jumping",
		"Allow support for rocket jumping. [Legacy >= 1.29]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 129, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COROCKETZTHRUST, "co_rocketzthrust", "Rocket Z Thrust",
		"Allow thrusting on the Z plane from rockets. [Legacy >= 1.24]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 124, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_COLIMITMONSTERZMATTACK, "co_limitmonsterzmattack", "Limit Monster Z Melee Range",
		"Limits Z Melee Attack range (stops melee from cliffs). [Legacy > 1.11]", PEXGSGM_ANY, PEXGSDR_MORETHAN, 111, {0, 1}, 1,
		PEXGSMC_COMPAT},
	{PEXGST_INTEGER, PEXGSBID_HEREMONSTERTHRESH, "here_monsterthresh", "Heretic Monster Threshold",
		"Use Heretic Threshold Logic. [Heretic]", PEXGSGM_HERETIC, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_HERETIC},
	{PEXGST_INTEGER, PEXGSBID_COVOODOODOLLS, "co_voodoodolls", "Enable Voodoo Dolls",
		"Enable spawning of Voodoo Dolls. [Legacy < 1.28]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 128, {0, 1}, 0,
		PEXGSMC_COMPAT},
	
	// Extra Puff in A_SmokeTrailer() before v1.25
	{PEXGST_INTEGER, PEXGSBID_COEXTRATRAILPUFF, "co_extratrailpuff", "Extra Smoke Trails",
		"Add extra puff for smoke. [Legacy < 1.25]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 125, {0, 1}, 0,
		PEXGSMC_COMPAT},
	
	// - Soul trails existed since 1.25, so that is the first ver?
	{PEXGST_INTEGER, PEXGSBID_COLOSTSOULTRAILS, "co_lostsoultrails", "Lost Sould Trails",
		"Lost souls emit smoke. [Legacy >= 1.25]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 125, {0, 1}, 1,
		PEXGSMC_COMPAT},
	
	// In Legacy 1.11, two sided walls were transparent!
	{PEXGST_INTEGER, PEXGSBID_COTRANSTWOSIDED, "co_transtwosided", "Transparent 2D Walls",
		"Transparent two sided walls. [Legacy = 1.11]", PEXGSGM_ANY, PEXGSDR_EQUALS, 111, {0, 1}, 0,
		PEXGSMC_COMPAT},
		
	// - Exists in v1.25
	// - Mentioned that it changed in v1.24
	// - Does not exist in v1.11
	// - Possibly appeared in v1.23?
	{PEXGST_INTEGER, PEXGSBID_COENABLEBLOODTIME, "co_enablebloodtime", "Enable Blood Time",
		"Enables setting blood time. [Legacy >= 1.23]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 123, {0, 1}, 1,
		PEXGSMC_COMPAT},
	
	// Since v1.12, there was jumping
	{PEXGST_INTEGER, PEXGSBID_COENABLEJUMPING, "co_enablejumping", "Enable Jumping",
		"Enables Jumping Support [Legacy >= 1.12]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 112, {0, 1}, 1,
		PEXGSMC_COMPAT},
	
	// After v1.11, Enable mouse aiming
	{PEXGST_INTEGER, PEXGSBID_COMOUSEAIM, "co_mouseaim", "Enable Mouse Aiming",
		"Enable mouse aiming [Legacy > 1.11]", PEXGSGM_ANY, PEXGSDR_MORETHAN, 111, {0, 1}, 1,
		PEXGSMC_COMPAT},
	
	{PEXGST_INTEGER, PEXGSBID_MONRESPAWNMONSTERS, "mon_respawnmonsters", "Respawn Monsters",
		"Monsters come back to life after a short delay.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_MONSTERS},
	
	{PEXGST_INTEGER, PEXGSBID_FUNNOTARGETPLAYER, "fun_noplayertarget", "No Player Targetting",
		"Monsters are incapable of targetting players.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_FUN},
	
	{PEXGST_INTEGER, PEXGSBID_MONARCHVILEANYRESPAWN, "mon_archvileanyrespawn", "Arch-Viles Respawn Anything",
		"Arch-Viles can ressurect anything regardless if it can be or not.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_FUN},
	
	{PEXGST_INTEGER, PEXGSBID_COOLDCHECKPOSITION, "co_oldcheckposition", "Old Position Checking",
		"Use old P_CheckPosition() Code. [Legacy < 1.42]", PEXGSGM_ANY, PEXGSDR_LESSTHAN, 142, {0, 1}, 0,
		PEXGSMC_COMPAT},
	
	{PEXGST_INTEGER, PEXGSBID_COLESSSPAWNSTICKING, "co_lessspawnsticking", "Less Spawn Spot Sticking",
		"Make players getting stuck inside other players less likely to occur. [ReMooD >= 1.0a]", PEXGSGM_ANY, PEXGSDR_ATLEAST, PEXGSDR_ATLEAST, 200, {0, 1}, 1,
		PEXGSMC_COMPAT},
	
	{PEXGST_INTEGER, PEXGSBID_PLSPAWNTELEFRAG, "pl_spawntelefrag", "Tele-Frag When Spawning",
		"Tele-frag when a player respawns (to empty the spot). [ReMooD >= 1.0a]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 200, {0, 1}, 1,
		PEXGSMC_PLAYERS},
	
	{PEXGST_INTEGER, PEXGSBID_GAMEONEHITKILLS, "game_onehitkills", "One Hit Kills",
		"Any recieved damage kills.", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 1}, 0,
		PEXGSMC_GAME},
	
	{PEXGST_INTEGER, PEXGSBID_COBETTERPLCORPSEREMOVAL, "co_betterplbodyqueue", "Better Body Management",
		"Better management of player bodies so they do not litter everywhere. [ReMooD >= 1.0a]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 200, {0, 1}, 1,
		PEXGSMC_COMPAT},
	
	{PEXGST_INTEGER, PEXGSBID_PLSPAWNCLUSTERING, "pl_spawnclustering", "Spawn Spot Clustering",
		"Adds extra spawn spots near other spawn spots for more players. [ReMooD >= 1.0a]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 200, {0, 1}, 1,
		PEXGSMC_PLAYERS},
	
	{PEXGST_INTEGER, PEXGSBID_COIMPROVEDMOBJONMOBJ, "co_improvedmobjonmobj", "Improved Object on Object",
		"Improves handling of objects on top of other objects. [ReMooD >= 1.0a]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 200, {0, 0}, 0,
		PEXGSMC_COMPAT},
	
	{PEXGST_INTEGER, PEXGSBID_COLINETRAVERSEMOVE, "co_linetraversemove", "Line Traversing Move",
		"Use line traversing when moving things. [ReMooD >= 1.0a]", PEXGSGM_ANY, PEXGSDR_ATLEAST, 200, {0, 0}, 0,
		PEXGSMC_COMPAT},
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

/* PS_EXGSGeneralComm() -- General command for game settings */
static CONL_ExitCode_t PS_EXGSGeneralComm(const uint32_t a_ArgC, const char** const a_ArgV)
{
	size_t i;
	P_EXGSVariable_t* Var;
	
	/* Game Settings Control */
	if (strcasecmp(a_ArgV[0], "gamevar") == 0)
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
			P_EXGSSetValueStr(Var->BitID, a_ArgV[2]);
			
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
			P_EXGSSetVersionLevel(atoi(a_ArgV[1]));
		}
		
		// Success!
		return CLE_SUCCESS;
	}
	
	/* Unknown */
	return CLE_INVALIDARGUMENT;
}

/* P_EXGSRegisterStuff() -- Register stuff needed for these things */
bool_t P_EXGSRegisterStuff(void)
{
	/* Register game setting commands */
	CONL_AddCommand("gamevar", PS_EXGSGeneralComm);
	CONL_AddCommand("gameversion", PS_EXGSGeneralComm);
	
	/* Always works! */
	return true;
}

/* P_EXGSSetVersionLevel() -- Set version level to a specific compatibility level */
bool_t P_EXGSSetVersionLevel(const uint32_t a_Level)
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
		
		// Which comparison?
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
		P_EXGSSetValue(l_GSVars[i].BitID, l_GSVars[i].DemoVal[IsTrue]);
	}
	
	return true;
}

/* P_EXGSSetValue() -- Sets value of variable */
int32_t P_EXGSSetValue(const P_EXGSBitID_t a_Bit, const int32_t a_Value)
{
	P_EXGSVariable_t* Var;
	
	/* Get variable first */
	Var = P_EXGSVarForBit(a_Bit);
	
	// Nothing?
	if (!Var)
		return 0;
	
	/* Set value */
	Var->WasSet = true;
	Var->ActualVal = a_Value;
	
	/* Perform value limiting on it (enum) */
	
	/* Call value change function */
	
	/* Return the value */
	return Var->ActualVal;
}

/* P_EXGSSetValueStr() -- Sets value by string */
int32_t P_EXGSSetValueStr(const P_EXGSBitID_t a_Bit, const char* const a_Value)
{
	int32_t SetVal;
	P_EXGSVariable_t* Var;
	
	/* Check */
	if (!a_Value)
		return 0;
	
	/* Get variable first */
	Var = P_EXGSVarForBit(a_Bit);
	
	// Nothing?
	if (!Var)
		return 0;
	
	/* See which value to use here */
	SetVal = atoi(a_Value);
	
	/* Return the value */
	return P_EXGSSetValue(a_Bit, SetVal);
}

