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
// Copyright (C) 2013-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Root Game Control (anything you desire to an extent)

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "console.h"
#include "d_net.h"

/****************
*** FUNCTIONS ***
****************/

/* ROOT_GetXPlayer() -- Returns X Player by name */
static D_XPlayer_t* ROOT_GetXPlayer(const char* const a_Name)
{
	uint32_t Int, i;
	bool_t OnlyInt, Hex;
	const char* c;
	
	/* Check */
	if (!a_Name)
		return NULL;
	
	/* Read as integer */
	Int = C_strtou32(a_Name, NULL, 0);
	
	// Only an integer?
	for (OnlyInt = Hex = true, c = a_Name; *c; c++)
		if (!(*c == ' ' || (*c >= '0' && *c <= '9')))
		{
			OnlyInt = false;
			
			if (Hex)
				if (!((*c >= 'a' && *c <= 'f') || (*c >= 'A' && *c <= 'F')))
					Hex = false;
		}
	
	/* Integer Check */
	if (OnlyInt)
		for (i = 0; i < g_NumXPlays; i++)
			if (g_XPlays[i])
				if (Int == i)
					return g_XPlays[i];
	
	/* ID Check */
	if (Hex)
		for (i = 0; i < g_NumXPlays; i++)
			if (g_XPlays[i])
				if (Int == g_XPlays[i]->ID)
					return g_XPlays[i];
	
	/* Account check */
	for (i = 0; i < g_NumXPlays; i++)
		if (g_XPlays[i])
			if (!strcasecmp(g_XPlays[i]->AccountName, a_Name))
				return g_XPlays[i];
	
	/* Not Found */
	return NULL;
}

/* ROOT_ListPlayers() -- List players in game */
static int ROOT_ListPlayers(const uint32_t a_ArgC, const char** const a_ArgV)
{
	D_XPlayer_t* XPlay;
	int32_t i;
	
	/* Print all players info */
	for (i = 0; i < g_NumXPlays; i++)
	{
		XPlay = g_XPlays[i];
		
		// Nothing?
		if (!XPlay)
			continue;
		
		// Print
		CONL_PrintF("%2i: %s^%s [%08x]\n", i, XPlay->AccountName, XPlay->AccountServer, XPlay->ID);
	}
	
	/* Always Works */
	return 0;
}

/* ROOT_Info() -- Print information on player */
static int ROOT_Info(const uint32_t a_ArgC, const char** const a_ArgV)
{
	D_XPlayer_t* XPlay;
	
	/* Find Player */
	XPlay = ROOT_GetXPlayer(a_ArgV[0]);
	
	// Not found?
	if (!XPlay)
		return 1;
	
	/* Print Player Info */
	CONL_PrintF("Nam: %s^%s\n", XPlay->AccountName, XPlay->AccountServer);
	CONL_PrintF("IDs: n=%08x h=%08x c=%08x\n", XPlay->ID, XPlay->HostID, XPlay->ClProcessID);
	CONL_PrintF("Ply: %i (Screen %i)\n", XPlay->InGameID, XPlay->ScreenID);
	CONL_PrintF("Tok: %s/%s\n", XPlay->SSToken, XPlay->CLToken);
	
	/* Success! */
	return 1;
}

/* DS_XNetRootCon() -- Root Game Control */
int DS_XNetRootCon(const uint32_t a_ArgC, const char** const a_ArgV)
{
	bool_t PrintUsage, BadArgs;
	int32_t MC;	
	
	static const struct
	{
		const char* Name;						// Name of command
		uint32_t MinArgs;						// Minimum arguments
		const char* Usage;						// Usage String
		int (*Func)(const uint32_t a_ArgC, const char** const a_ArgV);
	} c_MasterCommands[] =
	{
		{"list", 0, "", ROOT_ListPlayers},
		{"info", 1, "<xplay>", ROOT_Info},
		
		{NULL}
	};
	
	/* Not Server */
	if (!D_XNetIsServer())
	{
		CONL_PrintF("Root control is functional only with servers.\n");
		return 1;
	}
	
	/* Not enough args? */
	PrintUsage = BadArgs = false;
	if (a_ArgC < 2)
		PrintUsage = true;
	
	// Illegal Command or not enough?
	else
	{
		for (MC = 0; c_MasterCommands[MC].Name; MC++)
			if (!strcasecmp(a_ArgV[1], c_MasterCommands[MC].Name))
			{
				// Not enough args?
				if (a_ArgC - 2 < c_MasterCommands[MC].MinArgs)
					BadArgs = true;
				
				break;
			}
		
		// Missed?
		if (!c_MasterCommands[MC].Name || BadArgs)
			PrintUsage = true;
	}
	
	// Bad?
	if (PrintUsage || BadArgs)
	{
		// Did not specify enough arguments
		if (BadArgs)
			CONL_PrintF("Usage: %s %s %s\n", a_ArgV[0], c_MasterCommands[MC].Name, c_MasterCommands[MC].Usage);
		
		// Print all commands instead
		else
		{
			CONL_PrintF("{1Root Game Control{z Commands:\n");
			for (MC = 0; c_MasterCommands[MC].Name; MC++)
				CONL_PrintF("%s ", c_MasterCommands[MC].Name);
			CONL_PrintF("\n");
		}
		
		// Fail
		return 1;
	}
	
	/* Call function */
	return c_MasterCommands[MC].Func(a_ArgC - 2, a_ArgV + 2);
}
