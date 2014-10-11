// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
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

static MC_GameServer_t** l_Servers = NULL;
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

