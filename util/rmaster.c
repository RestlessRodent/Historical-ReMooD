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
// Copyright (C) 2012 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: ReMooD Master Server

/***************
*** INCLUDES ***
***************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***********
*** IPV4 ***
***********/

/***********
*** IPV6 ***
***********/

/*********************
*** MASTER CONTROL ***
*********************/

/*** CONSTANTS ***/

#define MAXFIELDSIZE						128	// Max field size
#define COOKIESIZE							64	// Size of server cookie

/*** STRUCTURES ***/

/* MC_GamePlayer_t -- Player in server */
typedef struct MC_GamePlayer_s
{
	char AccountName[MAXFIELDSIZE];				// Player's account name
	char DisplayName[MAXFIELDSIZE];				// Name of player (as shown to others)
} MC_GamePlayer_t;

/* MC_GameServer_t -- A Game Server */
typedef struct MC_GameServer_s
{
	/* Network Related */
	char PublicCookie[COOKIESIZE];				// Server's Public Cookie
	char PrivateCookie[COOKIESIZE];				// Server's Private Cookie
	time_t LastMessage;							// Last message from server
	
	/* Timing */
	uint32_t TimeDelay;							// Delay the game is running at
	uint32_t Ping;								// Ping to server
	
	/* Players */
	MC_GamePlayer_t* Players;					// Players in server
	size_t NumPlayers;							// Number of players in server
	
	/* Server Info */
	char ServerName[MAXFIELDSIZE];				// Name of server
	char ServerURL[MAXFIELDSIZE];				// Server URL
	char ServerEMail[MAXFIELDSIZE];				// Server e-mail location
	char ServerWADUrl[MAXFIELDSIZE];			// URL of where to get WAD files
	uint8_t ServerVer[4];						// Server Version (maj, min, rel, lgc)
} MC_GameServer_t;

/*** LOCALS ***/

static MC_GameServer_t* l_Servers = NULL;
static size_t l_NumServers = 0;

/*** FUNCTIONS ***/

/****************
*** FUNCTIONS ***
****************/

/* main() -- Main Entry Point */
int main(int argc, char** argv)
{
	return EXIT_SUCCESS;
}

