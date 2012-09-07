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
// DESCRIPTION: Bot Code

// Bot navigation is done by dynamic node generation every 32 map units or so.
// There is no way to use precreated nodes currently and there might not be any
// plans to do so. Creating nodes for every single map would be a tiresome and
// complex process (imagine creating nodes for EVERY single level in EVERY
// single WAD). Everyone would die before finishing. So this dynamic node system
// is a compromise. Before a level is truly playable by bots, the bots must
// navigate enough of the level in order for navigation to work, otherwise they
// cannot navigate through anything. Navigation nodes might take up much memory
// but the nodes are shared between all bots in the game. So in general, the
// smaller the map the better the performance.

/***************
*** INCLUDES ***
***************/

#include "b_bot.h"
#include "d_player.h"
#include "doomstat.h"
#include "r_defs.h"
#include "m_fixed.h"
#include "r_main.h"
#include "p_local.h"
#include "g_game.h"
#include "m_argv.h"

/****************
*** CONSTANTS ***
****************/

#define BOTMINNODEDIST		(32 << FRACBITS)	// Minimum node distance
#define BOTMAXNODERECOURSE	24					// Maximum bot recursion
#define BOTINITNODERECOURSE	12					// Initial Count for initial nodes
#define BOTSLMOVETIMEOUT	(2 * TICRATE)		// Straight move timeout

/* B_BotActionSub_t -- Bot action subroutines */
typedef enum B_BotActionSub_e
{
	BBAS_STRAIGHTNAV,							// Straight line navigation
	BBAS_FOLLOWNEARESTPLAYER,					// Follows the nearest player
	BBAS_BLINDSTRAIGHTNAV,						// Same as Straight nav but nodeless
	
	BBAS_GHOSTLYAI,								// New AI Test
	
	NUMBBOTACTIONSUBS,
	
	BBAS_NORMALAI = BBAS_FOLLOWNEARESTPLAYER
} B_BotActionSub_t;

/* B_NodeToNodeFlags_t -- Node to node flags */
typedef enum B_NodeToNodeFlags_e
{
	BNTNF_RUNDYNAMIC 			= 0x00000001,	// Runs dynamic checking
	BNTNF_FIRSTTIME				= 0x00000002,	// First time determination
} B_NodeToNodeFlags_t;

/* B_BuildBotPathFlags_t -- Paths for building bot path */
typedef enum B_BuildBotPathFlags_e
{
	BBBPF_INSERT				= 0x00000001,	// Insert nodes instead of deleting them all
} B_BuildBotPathFlags_t;

/* B_GhostChance_t -- Chances for the bot */
typedef enum B_GhostChance_e
{
	BGC_EXPLOREMAP,
	BGC_DEFENDSELF,
	BGC_ATTACKENEMY,
	BGC_GRABWEAPONS,
	BGC_GRABAMMO,
	BGC_GRABITEMS,
	BGC_DEFENDALLY,
	BGC_MODESPECIFIC,	
	
	NUMGHOSTCHANCES
} B_GhostChance_t;

// c_GhostChanceNames -- Chance names
static const char* const c_GhostChanceNames[NUMGHOSTCHANCES] =
{
	"EXP",
	"DEF",
	"ATK",
	"WEP",
	"AMM",
	"ITM",
	"PRO",
	"MOD",
};

#define MAXADJCHECK		32

/*****************
*** STRUCTURES ***
*****************/

/* B_BotNode_t -- A single bot node */
typedef struct B_BotNode_s
{
	/* Position */
	bool_t NoTraverse;							// Never traversable
	bool_t IsFloating;							// Is floating node
	fixed_t Pos[2];								// Position of node
	subsector_t* SubS;							// Subsector of node
	fixed_t FloorZ, CeilingZ;					// Floor and Ceiling Z
	
	/* Links to other nodes */
	bool_t LinksMade;							// Links generated?
	struct B_BotNode_s* Links[3][3];			// (x, y)
	fixed_t Dists[3][3];						// Distance
	
	/* Path Building */
	uint32_t NavCount;							// Navigation Count
} B_BotNode_t;

/* B_BotData_t -- Bot data */
struct B_BotData_s
{
	/* Network Related */
	D_NetPlayer_t* NetPlayer;					// Network player
	player_t* Player;							// Game player
	
	/* Dead Related */
	bool_t IsDead;								// Is the bot dead?
	tic_t DeathTime;							// Time the bot died
	
	/* Movement */
	B_BotNode_t* AtNode;						// Node Currently At
	
	B_BotNode_t** PathNodes;					// Node List
	size_t PathIt;								// Current node that is being iterated
	size_t NumPathNodes;						// Number of path nodes
	
	B_BotNode_t** PivotNodes;					// Pivots in path
	size_t PivotIt;								// Current Pivot
	size_t NumPivotNodes;						// Number of pivot nodes
	
	/* Thinking */
	B_BotActionSub_t ActSub;					// Action Subroutine
	
	// Straight Line
	int8_t SLMoveX;								// Straight line move on X
	int8_t SLMoveY;								// Straight line move on Y
	int8_t SLMoveSpeed;							// Speed to move in
	tic_t SLMoveTimeout;						// Time to stop trying to move there
	B_BotNode_t* SLToNode;						// Moving to this node
	
	// Follow Player
	uint32_t FPTargetAim;						// Target Aim
	
	// Ghostly AI
	uint32_t GHOSTLastChanceTime;				// Time of last chance update
	int32_t GHOSTChances[NUMGHOSTCHANCES];		// Chances of action
	B_GhostChance_t GHOSTMainPri, GHOSTSubPri;	// Bot Priority
	
	mobj_t* GHOSTPickupMo;						// Object to pickup
	uint32_t GHOSTBelayPickup;					// Belay pickups to a later date
	mobj_t* GHOSTAttackMo;						// Target to attack
	sector_t* GHOSTLastSec;						// Last sector
	sector_t* GHOSTAdj[MAXADJCHECK];			// Adjacent sectors
	int32_t GHOSTNumAdj;						// Number of them
	bool_t GHOSTSSGRightHand;					// Right handed SSG
	uint32_t GHOSTSSGHandTime;					// Time to swap hands
	
	B_GhostBot_t GHOSTData;						// Ghost Data
};

/**************
*** GLOBALS ***
**************/

bool_t g_BotDebug = false;						// Debugging Bots

/*************
*** LOCALS ***
*************/

static const int c_LinkOp[3] = {2, 1, 0};

static const fixed_t c_forwardmove[2] = { 25, 50 };
static const fixed_t c_sidemove[2] = { 24, 40 };
static const fixed_t c_angleturn[3] = { 640, 1280, 320 };	// + slow turn
#define MAXPLMOVE       (c_forwardmove[1])

// c_NavAngle -- Quick Navigation Angle
static angle_t c_NavAngle[3][3] =
{
	{ANG180 + ANG45,	ANG180,			ANG90 + ANG45},
	{ANG270,			0,				ANG90},
	{ANG270 + ANG45,	0,				ANG45},
};

static B_BotNode_t** l_BotNodes = NULL;			// Nodes here
static size_t l_NumBotNodes = 0;				// Number of bot nodes
static bool_t l_InitialNodeGen = false;			// Initial Nodes Generated
static B_BotNode_t* l_PlayerNodes[MAXPLAYERS];	// Player Node locations
static tic_t l_PlayerLastTime = 0;				// Last generation time

/************************
*** PRIVATE FUNCTIONS ***
************************/

/* BS_ThinkGhostlyAI() -- My own simplified AI */
static void BS_ThinkGhostlyAI(B_BotData_t* const a_BotData, ticcmd_t* const a_TicCmd)
{
}

/****************
*** FUNCTIONS ***
****************/

/* B_InitNodes() -- Initializes bots for this level */
void B_InitNodes(void)
{
	l_InitialNodeGen = false;
	B_GHOST_InitLevel();
}

/* B_ClearNodes() -- Clears all bot nodes */
void B_ClearNodes(void)
{
}

/* B_InitBot() -- Initializes Bot */
B_BotData_t* B_InitBot(D_NetPlayer_t* const a_NPp, const B_BotTemplate_t* a_Template)
{
	B_BotData_t* New;
	thinker_t* currentthinker;
	mobj_t* mo;
	
	/* Check */
	if (!a_NPp)
		return NULL;
	
	/* Debugging? */
	if (M_CheckParm("-devbots"))
		g_BotDebug = true;
	
	/* Initial Node Generation? */
	if (!l_InitialNodeGen)
	{
#if 0
		// Run through every map object
		for (currentthinker = thinkercap.next; currentthinker != &thinkercap; currentthinker = currentthinker->next)
		{
			// Only Objects
			if ((currentthinker->function.acp1 != (actionf_p1)P_MobjThinker))
				continue;

			// Convert to object
			mo = (mobj_t*)currentthinker;
			
			// Object is something worth navigating to?
				// Things that can be picked up
				// Shootable things (monsters, barrels, etc.)
				// Things marked as initialization nodes (CTF Flags)
			if ((mo->flags & (MF_SPECIAL | MF_SHOOTABLE)) || (mo->RXFlags[1] & MFREXB_INITBOTNODES))
				BS_GetNodeAtPos(mo->x, mo->y, BOTINITNODERECOURSE);
		}
#endif
		
		// Generated, so don't bother again
		l_InitialNodeGen = true;
	}
	
	/* Allocate */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Set Data */
	New->NetPlayer = a_NPp;
	New->Player = New->NetPlayer->Player;
	New->ActSub = BBAS_GHOSTLYAI;//BBAS_FOLLOWNEARESTPLAYER;
	New->GHOSTData.BotTemplate = a_Template;
	
	/* Set and return */
	a_NPp->BotData = New;
	return New;
}

/* B_BotGetTemplate() -- Returns player bot template */
const B_BotTemplate_t* B_BotGetTemplate(const int32_t a_Player)
{
	/* Check */
	if (a_Player < 0 || a_Player >= MAXPLAYERS)
		return NULL;
	
	/* Not playing? */
	if (!playeringame[a_Player])
		return NULL;
	
	/* No Net Player? */
	if (!players[a_Player].NetPlayer)
		return NULL;
	
	/* Not a bot? */
	if (players[a_Player].NetPlayer->Type != DNPT_BOT)
		return NULL;
	
	/* No Data? */
	if (!players[a_Player].NetPlayer->BotData)
		return NULL;
	
	/* Return the template */
	return players[a_Player].NetPlayer->BotData->GHOSTData.BotTemplate;
}

/* B_BuildBotTicCmd() -- Builds tic command for bot */
void B_BuildBotTicCmd(B_BotData_t* const a_BotData, ticcmd_t* const a_TicCmd)
{
	size_t i;
	player_t* Player;
	
	/* Check */
	if (!a_BotData || !a_TicCmd)
		return;
	
	/* Intermission? */
	if (gamestate == GS_INTERMISSION)
	{
		// Check to see if there are other non-bot players, because if there are
			// then do not press use! It would be very rude!
		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i])
				if (players[i].NetPlayer)
					if (players[i].NetPlayer->Type != DNPT_BOT)
						return;
		
		// Otherwise press use
		a_TicCmd->Std.buttons |= BT_USE;
		
		// Don't process anymore
		return;
	}
	
	/* Non-Level? */
	if (gamestate != GS_LEVEL)
		return;
	
	/* Update player nodes */
	if (gametic > l_PlayerLastTime)
	{
		// Update all positions
		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i])
				if (players[i].mo)
					;//l_PlayerNodes[i] = BS_GetNodeAtPos(players[i].mo->x, players[i].mo->y, 0);;
		
		// Reset time
		l_PlayerLastTime = gametic;
	}
	
	/* Get variables */
	Player = a_BotData->Player;
	
	// No player?
	if (!Player)
		return;
	
	/* Depending on player state */
	switch (Player->playerstate)
	{
			// Dead
		case PST_DEAD:
			// Was not previously dead, set time to revive
			if (!a_BotData->IsDead)
			{
				a_BotData->IsDead = true;
				a_BotData->DeathTime = gametic;
				
				a_BotData->GHOSTLastChanceTime = 0;
				a_BotData->GHOSTMainPri = 0;
				a_BotData->GHOSTSubPri = 0;
				a_BotData->GHOSTPickupMo = NULL;
				a_BotData->GHOSTAttackMo = NULL;
				a_BotData->GHOSTBelayPickup = 0;
			}
			
			// Is still dead, wait 2 seconds to respawn
			else if (gametic > (a_BotData->DeathTime + (TICRATE * 2)))
			{
				// Press use
				a_TicCmd->Std.buttons |= BT_USE;
			}
			break;
		
			// Alive
		case PST_LIVE:
			// Unmark as dead
			a_BotData->IsDead = false;
			
			// Call Ghost thinker
			a_BotData->GHOSTData.BotData = a_BotData;
			a_BotData->GHOSTData.Player = Player;
			a_BotData->GHOSTData.Mo = Player->mo;
			B_GHOST_Think(&a_BotData->GHOSTData, a_TicCmd);
			break;
		
			// Unknown
		default:
			break;
	}
}

/* B_RemoveMobj() -- Remove map object */
void B_RemoveMobj(void* const a_Mo)
{
	D_NetPlayer_t* NetPlayer;
	B_BotData_t* BotData;
	size_t i;
	
	/* Go through all players */
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
		{
			// Get netplayer
			NetPlayer = players[i].NetPlayer;
			
			// Check
			if (!NetPlayer)
				continue;
			
			// Get bot
			BotData = NetPlayer->BotData;
			
			// Check
			if (!BotData)
				continue;
			
			// Clear mos
			if (BotData->GHOSTPickupMo == a_Mo)
				BotData->GHOSTPickupMo = NULL;
			if (BotData->GHOSTAttackMo == a_Mo)
				BotData->GHOSTAttackMo = NULL;
		}
}

