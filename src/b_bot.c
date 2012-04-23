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

/****************
*** CONSTANTS ***
****************/

#define BOTMINNODEDIST		(32 << FRACBITS)	// Minimum node distance
#define BOTMAXNODERECOURSE	5					// Maximum bot recursion
#define BOTINITNODERECOURSE	3					// Initial Count for initial nodes

/* B_BotActionSub_t -- Bot action subroutines */
typedef enum B_BotActionSub_e
{
	BBAS_STRAIGHTNAV,							// Straight line navigation	
	
	NUMBBOTACTIONSUBS
} B_BotActionSub_t;

/* B_NodeToNodeFlags_t -- Node to node flags */
typedef enum B_NodeToNodeFlags_e
{
	BNTNF_RUNDYNAMIC 			= 0x00000001,	// Runs dynamic checking
	BNTNF_FIRSTTIME				= 0x00000002,	// First time determination
} B_NodeToNodeFlags_t;

/*****************
*** STRUCTURES ***
*****************/

/* B_BotNode_t -- A single bot node */
typedef struct B_BotNode_s
{
	/* Position */
	fixed_t Pos[2];								// Position of node
	subsector_t* SubS;							// Subsector of node
	
	/* Links to other nodes */
	bool_t LinksMade;							// Links generated?
	struct B_BotNode_s* Links[3][3];			// (x, y)
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
	
	/* Thinking */
	B_BotActionSub_t ActSub;					// Action Subroutine
	
	// Straight Line
	int8_t SLMoveX;								// Straight line move on X
	int8_t SLMoveY;								// Straight line move on Y
	int8_t SLMoveSpeed;							// Speed to move in
	tic_t SLMoveTimeout;						// Time to stop trying to move there
	B_BotNode_t* SLToNode;						// Moving to this node
};

/**************
*** GLOBALS ***
**************/

bool_t g_BotDebug = false;						// Debugging Bots

/*************
*** LOCALS ***
*************/

static const c_LinkOp[3] = {2, 1, 0};

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

/* BS_Random() -- Random Number */
static int BS_Random(B_BotData_t* const a_BotData)
{
	return M_Random();
}

/* BS_NTNPFirst() -- Determines whether point is reachable */
// This is done for the initial node creation and as such it also determines
// if a switch or line trigger is needed for activation.
static bool_t BS_NTNPFirst(intercept_t* in, void* const a_Data)
{
	line_t* li;
	
	/* A Line */
	if (in->isaline)
	{
		// Get line
		li = in->d.line;
		
		// Cannot cross two sided line
		if (!(li->flags & ML_TWOSIDED))
			return false;
		
		// Cannot cross impassible line
		if (li->flags & ML_BLOCKING)
			return false;
		
		// Cannot fit inside of sector
			// Front Side
		if ((li->frontsector->ceilingheight - li->frontsector->floorheight) < (56 << FRACBITS))
			return false;
			// Back Side
		if ((li->backsector->ceilingheight - li->backsector->floorheight) < (56 << FRACBITS))
			return false;
		
		// Everything seems OK
		return true;
	}
	
	/* A Thing */
	else
	{
		// Things are checked later
		return false;
	}
}

/* BS_NodeToNodePossible() -- Checks if node to node is possible */
static bool_t BS_NodeToNodePossible(B_BotData_t* const a_BotData, B_BotNode_t* const a_A, B_BotNode_t* const a_B, const uint32_t a_Flags)
{
	player_t* Player;
	fixed_t sfZ, scZ, dfZ, dcZ;	
	
	/* First Time Determination? */
	if (a_Flags & BNTNF_FIRSTTIME)
	{
		// Draw line from a to b
			// If the traverser returns false, it cannot be crossed
		if (!P_PathTraverse(
				a_A->Pos[0],
				a_A->Pos[1],
				a_B->Pos[0],
				a_B->Pos[1],
				PT_ADDLINES,
				BS_NTNPFirst,
				NULL
			))
			return false;
		
		// It can be reached
		return true;
	}
	
	/* Bot data is needed here */
	// Check
	if (!a_BotData)
		return false;
	
	// Get Player
	Player = a_BotData->Player;
	
	// No player?
	if (!Player)
		return false;
	
	// Player has no object?
	if (!Player->mo)
		return false;
	
	/* Get sector information */
	sfZ = a_A->SubS->sector->floorheight;
	scZ = a_A->SubS->sector->ceilingheight;
	dfZ = a_B->SubS->sector->floorheight;
	dcZ = a_B->SubS->sector->ceilingheight;
	
	/* Sector height differences */
	// Destination ceiling too low?
	if (dcZ - sfZ < Player->mo->height)
		return false;
	
	// Destination floor too high?
	if (dfZ - sfZ >= (24 << FRACBITS))
		return false;
	
	/* Reachable */
	return true;
}

/* BS_GetNodeAtPos() -- Get node at position */
static B_BotNode_t* BS_GetNodeAtPos(const fixed_t a_X, const fixed_t a_Y, const int32_t a_Rec)
{
	subsector_t* SpotSS = NULL;
	B_BotNode_t* CurNode, *LinkNode;
	fixed_t Dist;
	size_t i;
	int32_t x, y, LinkXID, LinkYID;
	
	/* See if spot is in a subector */
	SpotSS = R_IsPointInSubsector(a_X, a_Y);
	
	// No subsector here?
	if (!SpotSS)
		return NULL;
	
	/* Go through nodes in subsector */
	for (CurNode = NULL, i = 0; i < SpotSS->NumBotNodes; i++)
	{
		// Get current node
		CurNode = SpotSS->BotNodes[i];
		
		// Missing?
		if (!CurNode)
			continue;
		
		// Determine how close the node is
		Dist = P_AproxDistance(a_X - CurNode->Pos[0], a_Y - CurNode->Pos[1]);
		
		// Distance is close
		if (Dist < BOTMINNODEDIST)
			break;
		
		// Forget Node
		CurNode = NULL;
	}
	
	/* Create a new node at this position */
	if (!CurNode)
	{
		CurNode = Z_Malloc(sizeof(*CurNode), PU_LEVEL, NULL);
	
		// Set node info
		CurNode->Pos[0] = a_X;
		CurNode->Pos[1] = a_Y;
		CurNode->SubS = SpotSS;
	
		// Add node to subsector chain
		Z_ResizeArray(
				(void**)&SpotSS->BotNodes,
				sizeof(*SpotSS->BotNodes),
				SpotSS->NumBotNodes,
				SpotSS->NumBotNodes + 1
			);
		SpotSS->BotNodes[SpotSS->NumBotNodes++] = CurNode;
	
		// If debugging, spawn a nice candle here
		if (g_BotDebug)
		{
			P_SpawnMobj(
					CurNode->Pos[0],
					CurNode->Pos[1],
					CurNode->SubS->sector->floorheight,
					INFO_GetTypeByName("BlackCandle")
				);
		}
	}
	
	/* Recursion to find different nodes */
	// Links need generation?
		// Also, do not generate nodes and recourse more than needed!
	if (a_Rec < BOTMAXNODERECOURSE && !CurNode->LinksMade)
	{
		// Set current node as generated
		CurNode->LinksMade = true;
		
		// One for each angle away
		for (x = -1; x <= 1; x++)
			for (y = -1; y <= 1; y++)
			{
				// Don't bother with the current position
				if (x == 0 && y == 0)
					continue;
			
				// Determine link ideas (to this node)
				LinkXID = x + 1;
				LinkYID = y + 1;
			
				// Create new link (at coordinate away from this spot)
				LinkNode = CurNode->Links[LinkXID][LinkYID] =
					BS_GetNodeAtPos(
							a_X + ((BOTMINNODEDIST * x)),
							a_Y + ((BOTMINNODEDIST * y)),
							a_Rec + 1
						);
			
				// Link was returned?
				if (LinkNode)
				{
					// Link the remote node back to here
					LinkNode->Links[c_LinkOp[LinkXID]][c_LinkOp[LinkYID]] = CurNode;
				
					// If the remote node is NOT in the same subsector
						// Determine if it is possible to get to it
						// But you can move to node from both sides, so a to b
						// and b to a MUST be determined.
					if (LinkNode->SubS != CurNode->SubS)
					{
						// Cannot go to there?
						if (!BS_NodeToNodePossible(NULL, CurNode, LinkNode, BNTNF_FIRSTTIME))
							CurNode->Links[LinkXID][LinkYID] = NULL;
						
						// Cannot come from there?
						if (!BS_NodeToNodePossible(NULL, LinkNode, CurNode, BNTNF_FIRSTTIME))
							LinkNode->Links[c_LinkOp[LinkXID]][c_LinkOp[LinkYID]] = NULL;
					}
				}
			}
	}
	
	/* Return the current node */
	return CurNode;
}

/* BS_UpdateCurrentPos() -- Updates the bot's current position */
static void BS_UpdateCurrentPos(B_BotData_t* const a_BotData)
{
	player_t* Player;
	
	/* Check */
	if (!a_BotData)
		return;
	
	/* Get player */
	Player = a_BotData->Player;
	
	// No map object?
	if (!Player->mo)
		return;
	
	/* Set the current node to the bot's current location */
	a_BotData->AtNode = BS_GetNodeAtPos(Player->mo->x, Player->mo->y, 0);
}

/* BS_ThinkStraightLine() -- Think about moving in a straight line */
static void BS_ThinkStraightLine(B_BotData_t* const a_BotData, ticcmd_t* const a_TicCmd)
{
	int32_t lopX, lopY;
	B_BotNode_t* CurNode, *LinkNode;
	bool_t Walkable;
	
	/* Check */
	if (!a_BotData || !a_TicCmd)
		return;
	
	/* Direction not set? */
	while (!a_BotData->SLMoveX && !a_BotData->SLMoveY)
	{
		a_BotData->SLMoveX = (BS_Random(a_BotData) % 3) - 1;
		a_BotData->SLMoveY = (BS_Random(a_BotData) % 3) - 1;
	}
	
	/* Get array to access */
	lopX = a_BotData->SLMoveX + 1;
	lopY = a_BotData->SLMoveY + 1;
	
	/* Get the node the bot is currently at */
	CurNode = a_BotData->AtNode;
	
	// No node at this coordinate? A lost bot
	if (!CurNode)
		return;
	
	/* Determine if moving is possible in this direction */
	// Get that node
	LinkNode = CurNode->Links[lopX][lopY];
	
	// Can walk to it?
	Walkable = false;
	if (LinkNode)
		Walkable = BS_NodeToNodePossible(a_BotData, CurNode, LinkNode, 0);
	
	// No node, can't be switched to, taking too long? Switch movement
	if (!LinkNode || !Walkable ||
		(a_BotData->SLToNode && CurNode != a_BotData->SLToNode && gametic > a_BotData->SLMoveTimeout))
	{
		// Clear destination node
		a_BotData->SLToNode = false;
		
		// Add X first
		a_BotData->SLMoveX++;
		
		// Prevent premature reset
		if (a_BotData->SLMoveX == 0 && a_BotData->SLMoveY == 0)
			a_BotData->SLMoveX++;
		
		// Overflow? Move Y axis down
		if (a_BotData->SLMoveX >= 2)
		{
			a_BotData->SLMoveX = -1;
			a_BotData->SLMoveY++;
			
			// Another overflow? Clear Direction
			if (a_BotData->SLMoveY >= 2)
				a_BotData->SLMoveX = a_BotData->SLMoveY = 0;
		}
		
		// Wait for another tic to do the movement
		return;
	}
	
	/* Move in that direction */
	// Timeout determination
	if (!a_BotData->SLToNode)
	{
		a_BotData->SLMoveTimeout = gametic + (TICRATE * 2);
		a_BotData->SLToNode = LinkNode;
	}
	
	// Turn to it
	a_TicCmd->angleturn = c_NavAngle[lopX][lopY] >> 16;
	
	// Walk to it
	a_TicCmd->forwardmove = c_forwardmove[a_BotData->SLMoveSpeed];
	
	// Spawn object there
	if (g_BotDebug)
	{
		//CONL_PrintF("Bot: Dir (%i, %i)\n", a_BotData->SLMoveX, a_BotData->SLMoveY);
		P_SpawnMobj(
				LinkNode->Pos[0],
				LinkNode->Pos[1],
				LinkNode->SubS->sector->floorheight,
				INFO_GetTypeByName("ItemFog")
			);
	}
}

/****************
*** FUNCTIONS ***
****************/

/* B_InitNodes() -- Initializes bots for this level */
void B_InitNodes(void)
{
	l_InitialNodeGen = false;
}

/* B_InitBot() -- Initializes Bot */
B_BotData_t* B_InitBot(D_NetPlayer_t* const a_NPp)
{
	B_BotData_t* New;
	thinker_t* currentthinker;
	mobj_t* mo;
	
	/* Check */
	if (!a_NPp)
		return;
	
	/* Debugging? */
	if (M_CheckParm("-devbots"))
		g_BotDebug = true;
	
	/* Initial Node Generation? */
	if (!l_InitialNodeGen)
	{
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
		
		// Generated, so don't bother again
		l_InitialNodeGen = true;
	}
	
	/* Allocate */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Set Data */
	New->NetPlayer = a_NPp;
	New->Player = New->NetPlayer->Player;
	
	/* Set and return */
	a_NPp->BotData = New;
	return New;
}

/* B_BuildBotTicCmd() -- Builds tic command for bot */
void B_BuildBotTicCmd(B_BotData_t* const a_BotData, ticcmd_t* const a_TicCmd)
{
	size_t i;
	player_t* Player;
	
	/* Check */
	if (!a_BotData || !a_TicCmd)
		return;
	
	/* Update player nodes */
	if (gametic > l_PlayerLastTime)
	{
		// Update all positions
		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i])
				if (players[i].mo)
					l_PlayerNodes[i] = BS_GetNodeAtPos(players[i].mo->x, players[i].mo->y, 0);;
		
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
			}
			
			// Is still dead, wait 2 seconds to respawn
			else if (gametic > (a_BotData->DeathTime + (TICRATE * 2)))
			{
				// Press use
				a_TicCmd->buttons |= BT_USE;
			}
			break;
		
			// Alive
		case PST_LIVE:
			// Unmark as dead
			a_BotData->IsDead = false;
			
			// Update to current position
			BS_UpdateCurrentPos(a_BotData);
			
			// Which action subroutine?
			switch (a_BotData->ActSub)
			{
					// Try moving in straight lines (to build navigation)
				case BBAS_STRAIGHTNAV:
					BS_ThinkStraightLine(a_BotData, a_TicCmd);
					break;
				
					// Unknown
				default:
					break;
			}
			break;
		
			// Unknown
		default:
			break;
	}
}

