// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.org/
// -----------------------------------------------------------------------------
// Copyright (C) 2008-2011 GhostlyDeath (ghostlydeath@gmail.com)
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

consvar_t cv_dc_allowjump =				{ "dc_allowjump", "1", CV_HIDEN, CV_YesNo };
consvar_t cv_dc_allowautoaim =			{ "dc_allowautoaim", "1", CV_HIDEN, CV_YesNo };
consvar_t cv_dc_forceautoaim =			{ "dc_forceautoaim", "1", CV_HIDEN, CV_YesNo };
consvar_t cv_dc_allowrocketjump =		{ "dc_allowrocketjump", "0", CV_HIDEN, CV_YesNo };
consvar_t cv_dc_classicblood =			{ "dc_classicblood", "0", CV_HIDEN, CV_YesNo};
consvar_t cv_dc_predictingmonsters =	{ "dc_predictingmonsters", "0", CV_HIDEN, CV_OnOff };
consvar_t cv_dc_classicrocketblast = 	{ "dc_classicrocketblast", "0", CV_HIDEN, CV_YesNo };
consvar_t cv_dc_classicmeleerange =		{ "dc_classicmeleerange", "0", CV_HIDEN, CV_YesNo };
consvar_t cv_dc_classicmonsterlogic =	{ "dc_classicmonsterlogic", "0", CV_HIDEN, CV_YesNo };

consvar_t* DemoPair[][2] =
{
	{&cv_allowjump,					&cv_dc_allowjump},
	{&cv_allowautoaim,				&cv_dc_allowautoaim},
	{&cv_forceautoaim,				&cv_dc_forceautoaim},
	{&cv_allowrocketjump,			&cv_dc_allowrocketjump},
	{&cv_classicblood,				&cv_dc_classicblood},
	{&cv_predictingmonsters,		&cv_dc_predictingmonsters},
	{&cv_classicrocketblast,		&cv_dc_classicrocketblast},
	{&cv_classicmeleerange,			&cv_dc_classicmeleerange},
	{&cv_classicmonsterlogic,		&cv_dc_classicmonsterlogic},
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

