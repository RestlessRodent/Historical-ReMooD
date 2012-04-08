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

// l_GSVars -- Game state variables
static P_EXGSVariable_t l_GSVars[] =
{
	{PEXGST_INTEGER, PEXGSBID_NOTHINGHERE, "nothinghere",
		"Nothing is here"},
	{PEXGST_INTEGER, PEXGSBID_COENABLEBLOODSPLATS, "co_enablebloodsplats",
		"Enables blood spats on walls. [Legacy >= 1.29]", PEXGSDR_ATLEAST, 129, {0, 1}, 1},
	{PEXGST_INTEGER, PEXGSBID_CORANDOMLASTLOOK, "co_randomlastlook",
		"Randomize monster's last look (which player to target). [Legacy >= 1.29]", PEXGSDR_ATLEAST, 129, {0, 1}, 1},
	{PEXGST_INTEGER, PEXGSBID_COUNSHIFTVILERAISE, "co_unshiftvileraise",
		"Multiply the corpse height by 4 when an Arch-Vile resurrects. [Legacy < 1.29]", PEXGSDR_LESSTHAN, 129, {0, 1}, 0},
	{PEXGST_INTEGER, PEXGSBID_COMODIFYCORPSE, "co_modifycorpse",
		"Enables correct corpse modification for solid corpses. [Legacy >= 1.31]", PEXGSDR_ATLEAST, 131, {0, 1}, 1},
	{PEXGST_INTEGER, PEXGSBID_CONOSMOKETRAILS, "co_nosmoketrails",
		"Disable smoke trails on rockets and lost souls [Legacy < 1.11]", PEXGSDR_LESSTHAN, 111, {0, 1}, 0},
	{PEXGST_INTEGER, PEXGSBID_COUSEREALSMOKE, "co_userealsmoke",
		"Use actual smoke rather than puffs for rockets and lost souls. [Legacy < 1.25]", PEXGSDR_LESSTHAN, 125, {0, 1}, 0},
	{PEXGST_INTEGER, PEXGSBID_COOLDCUTCORPSERADIUS, "co_oldcutcorpseradius",
		"Reduces corpse radius, only when co_modifycorpse is off. [Legacy >= 1.12]", PEXGSDR_GREATERTHAN, 112, {0, 1}, 1},
	{PEXGST_INTEGER, PEXGSBID_COSPAWNDROPSONMOFLOORZ, "co_spawndropsonmofloorz",
		"Spawn item drops on the closest floor rather than the lowest floor. [Legacy >= 1.32]", PEXGSDR_ATLEAST, 132, {0, 1}, 1},
	{PEXGST_INTEGER, PEXGSBID_CODISABLETEAMPLAY, "co_disableteamplay",
		"Disable support for team play. [Legacy < 1.25]", PEXGSDR_LESSTHAN, 125, {0, 1}, 0},
	{PEXGST_INTEGER, PEXGSBID_COSLOWINWATER, "co_moveslowinwater",
		"Move slower when underwater (in 3D Water). [Legacy >= 1.28]", PEXGSDR_ATLEAST, 128, {0, 1}, 1},
	{PEXGST_INTEGER, PEXGSBID_COSLIDEOFFMOFLOOR, "co_slideoffmofloor",
		"Slide off the closest floor not the real one (affects 3D floors). [Legacy >= 1.32]", PEXGSDR_ATLEAST, 132, {0, 1}, 1},
	{PEXGST_INTEGER, PEXGSBID_COOLDFRICTIONMOVE, "co_oldfrictionmove",
		"Use old friction when moving. [Legacy < 1.32]", PEXGSDR_LESSTHAN, 132, {0, 1}, 0},
	{PEXGST_INTEGER, PEXGSBID_COOUCHONCEILING, "co_ouchonceiling",
		"Go ouch when hitting the ceiling. [Legacy >= 1.12]", PEXGSDR_ATLEAST, 112, {0, 1}, 1},
	{PEXGST_INTEGER, PEXGSBID_COENABLESPLASHES, "co_enablesplashes",
		"Enable splashes on the water. [Legacy >= 1.25]", PEXGSDR_ATLEAST, 125, {0, 1}, 1},
	{PEXGST_INTEGER, PEXGSBID_COENABLEFLOORSMOKE, "co_enablefloorsmoke",
		"Enable smoke when on a damaging floor. [Legacy >= 1.25]", PEXGSDR_ATLEAST, 125, {0, 1}, 1},
	{PEXGST_INTEGER, PEXGSBID_COENABLESMOKE, "co_enablesmoke",
		"Enable smoke. [Legacy >= 1.25]", PEXGSDR_ATLEAST, 125, {0, 1}, 1},
	{PEXGST_INTEGER, PEXGSBID_CODAMAGEONLAND, "co_damageonland",
		"Damage when landing on a damaging floor. [Legacy >= 1.25]", PEXGSDR_ATLEAST, 125, {0, 1}, 1},
	{PEXGST_INTEGER, PEXGSBID_COABSOLUTEANGLE, "co_absoluteangle",
		"Use absolute angle rather than relative angle in player aiming. [Legacy >= 1.25]", PEXGSDR_ATLEAST, 125, {0, 1}, 1},
	{PEXGST_INTEGER, PEXGSBID_COOLDJUMPOVER, "co_oldjumpover",
		"Use old jump over code. [Legacy < 1.28]", PEXGSDR_LESSTHAN, 128, {0, 1}, 0},
	{PEXGST_INTEGER, PEXGSBID_COENABLESPLATS, "co_enablesplats",
		"Enable splats. [Legacy >= 1.28]", PEXGSDR_ATLEAST, 128, {0, 1}, 1},
	{PEXGST_INTEGER, PEXGSBID_COOLDFLATPUSHERCODE, "co_oldflatpushercode",
		"Use old pusher/puller code that cannot handle 3D Floors. [Legacy <= 1.40]", PEXGSDR_ATMOST, 140, {0, 1}, 0},
	{PEXGST_INTEGER, PEXGSBID_COSPAWNPLAYERSEARLY, "co_spawnplayersearly",
		"Spawn players while the map is loading. [Legacy >= 1.28]", PEXGSDR_ATLEAST, 128, {0, 1}, 1},
	{PEXGST_INTEGER, PEXGSBID_COENABLEUPDOWNSHOOT, "co_enableupdownshoot",
		"Enable shooting up/down when not aiming at something. [Legacy >= 1.28]", PEXGSDR_ATLEAST, 128, {0, 1}, 1},
};

