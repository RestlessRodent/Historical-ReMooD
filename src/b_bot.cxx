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
#include "m_random.h"
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
	
	NUMGHOSTCHANCES,
	
	BGC_DEFAULT = BGC_EXPLOREMAP,
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
	bool NoTraverse;							// Never traversable
	bool IsFloating;							// Is floating node
	fixed_t Pos[2];								// Position of node
	subsector_t* SubS;							// Subsector of node
	fixed_t FloorZ, CeilingZ;					// Floor and Ceiling Z
	
	/* Links to other nodes */
	bool LinksMade;							// Links generated?
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
	bool IsDead;								// Is the bot dead?
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
	bool GHOSTSSGRightHand;					// Right handed SSG
	uint32_t GHOSTSSGHandTime;					// Time to swap hands
	
	B_GhostBot_t GHOSTData;						// Ghost Data
};

/**************
*** GLOBALS ***
**************/

bool g_BotDebug = false;						// Debugging Bots

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
static bool l_InitialNodeGen = false;			// Initial Nodes Generated
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

/* BS_PointsToAngleTurn() -- Convert points to angle turn */
static uint16_t BS_PointsToAngleTurn(const fixed_t a_x1, const fixed_t a_y1, const fixed_t a_x2, const fixed_t a_y2)
{
	return R_PointToAngle2(a_x1, a_y1, a_x2, a_y2) >> 16;
}

/* BS_MoveToAndAimAtFrom() -- Aim at target and move to one at the same time */
static void BS_MoveToAndAimAtFrom(const fixed_t a_x1, const fixed_t a_y1, const fixed_t a_x2, const fixed_t a_y2, const fixed_t a_AimX, const fixed_t a_AimY, int16_t* const a_AngleTurn, int8_t* const a_Forward, int8_t* const a_Side)
{
	angle_t AimAng;
	angle_t MoveAng, DiffAng;
	
	/* Determine all angles */
	AimAng = R_PointToAngle2(a_x1, a_y1, a_AimX, a_AimY);
	MoveAng = R_PointToAngle2(a_x1, a_y1, a_x2, a_y2);
	DiffAng = MoveAng - AimAng;
	
	/* Do aiming first */
	*a_AngleTurn = AimAng >> 16;
	
	/* Then calculate side movements and such */
	*a_Forward = FixedMul(
		((fixed_t)c_forwardmove[1]) << FRACBITS,
		finecosine[DiffAng >> ANGLETOFINESHIFT]) >> FRACBITS;
	*a_Side = -(FixedMul(
		((fixed_t)c_forwardmove[1]) << FRACBITS,
		finesine[(DiffAng) >> ANGLETOFINESHIFT]) >> FRACBITS);
}

/* BS_AngleDiff() -- Difference of angle */
static angle_t BS_AngleDiff(const angle_t a_A, const angle_t a_B)
{
	return ((uint32_t)(abs(((int32_t)(a_A >> 1)) - ((int32_t)(a_B >> 1))) & 0x7FFFFFFFU)) << 1;
}

/* BS_NTNPFirst() -- Determines whether point is reachable */
// This is done for the initial node creation and as such it also determines
// if a switch or line trigger is needed for activation.
static bool BS_NTNPFirst(intercept_t* in, void* const a_Data)
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
static bool BS_NodeToNodePossible(B_BotData_t* const a_BotData, B_BotNode_t* const a_A, B_BotNode_t* const a_B, const uint32_t a_Flags)
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
	sfZ = a_A->FloorZ;
	scZ = a_A->CeilingZ;
	dfZ = a_B->FloorZ;
	dcZ = a_B->CeilingZ;
	
	/* Sector height differences */
	// Destination ceiling too low?
	if (dcZ - sfZ < Player->mo->height)
		return false;
	
	// Destination floor too high?
	if (dfZ - sfZ > (24 << FRACBITS))
		return false;
	
	/* Reachable */
	return true;
}

/* BS_BuildBotPath() -- Build bot path from A to B */
static uint32_t BS_BuildBotPath(B_BotData_t* const a_BotData, B_BotNode_t* const a_A, B_BotNode_t* const a_B, const uint32_t a_Flags)
{
	static uint32_t CurrentNav;
	uint32_t RetVal = 0;
	B_BotNode_t* AtNode;
	B_BotNode_t* DestNode;
	B_BotNode_t* CheckNode;
	B_BotNode_t* TheHitNode;
	angle_t DestAng, TryAng;
	angle_t AngDiff, CurAng;
	int32_t TryDir[2], x, y, SameX, SameY;
	bool HitNode;
	size_t j;
	
	/* Increase current navigation */
	CurrentNav = CurrentNav + 1;
	
	/* Check */
	if (!a_BotData || !a_A || !a_B)
		return 0;
	
	/* Clear all path nodes? */
	if (!(a_Flags & BBBPF_INSERT))
	{
		// Reset path data
		if (a_BotData->PathNodes)
			Z_Free(a_BotData->PathNodes);
		a_BotData->PathNodes = NULL;
		a_BotData->PathIt = a_BotData->NumPathNodes = 0;
	}
	
	/* The Pivot list must always be removed */
	// Reset path data
	if (a_BotData->PivotNodes)
		Z_Free(a_BotData->PivotNodes);
	a_BotData->PivotNodes = NULL;
	a_BotData->PivotIt = a_BotData->NumPivotNodes = 0;
	
	/* Some Junky Algorithm */
	// I don't know the actual A-Star but it goes something like the following...
#if !defined(__REMOOD_BBPASTAR)
	// Start at the node the bot is at
	AtNode = a_A;
	DestNode = a_B;
	
	//Dists
	
	// Continue moving to that node
	do
	{
		// Reset
		HitNode = false;
		TheHitNode = NULL;
		
		// Mark current node as navigated on
		AtNode->NavCount = CurrentNav;
		
		// Get angle to destination node
		DestAng = R_PointToAngle2(AtNode->Pos[0], AtNode->Pos[1], DestNode->Pos[0], DestNode->Pos[1]);
		
		// Reset angle differences
		AngDiff = ANG180;
		
		// With that angle, determine (x,y)
		for (x = -1; x <= 1; x++)
			for (y = -1; y <= 1; y++)
			{
				// Forget the same
				if (x == 0 && y == 0)
					continue;
				
				// Get angle based on prenagles
				TryAng = BS_AngleDiff(c_NavAngle[x + 1][y + 1], DestAng);
				
				// Shorter than that?
				if (TryAng < AngDiff)
				{
					// Get that node there
					CheckNode = AtNode->Links[x + 1][y + 1];
					
					// Node is there?
					if (CheckNode)
						// Walkable?
						if (BS_NodeToNodePossible(a_BotData, AtNode, CheckNode, BNTNF_RUNDYNAMIC))
							// Never been walked onto?
							if (CheckNode->NavCount != CurrentNav)
							{
								TryDir[0] = x + 1;
								TryDir[1] = y + 1;
								AngDiff = TryAng;
								HitNode = true;
								TheHitNode = CheckNode;
							}
				}
			}
		
		// If the hit failed to hit, then break out
		if (!HitNode || !TheHitNode)
			break;
		
		// Now add it to the node list at the current insertion point
		Z_ResizeArray(
				(void**)&a_BotData->PathNodes,
				sizeof(*a_BotData->PathNodes),
				a_BotData->NumPathNodes,
				a_BotData->NumPathNodes + 1
			);
		a_BotData->PathNodes[a_BotData->NumPathNodes++] = TheHitNode;
		
		// Go to that node now
		AtNode = TheHitNode;
		RetVal++;
	} while (AtNode != DestNode);
	
	/* A-Star */
#else
#endif
	
	/* No Real Path? */
	if (RetVal <= 2)
		return 0;
	
	/* Build Pivoted Node List */
	// The pivot list here is a simplified path and is essentially the same
	// path from A to B but with less nodes in between. Why make a pivot list?
	// Because at this time the bot just goes to a single node then once it
	// reaches it, it moves to the next. Sounds like it works but the bot runs
	// towards every node in the list and it takes awhile for it to move to the
	// target. So as such, the pivot list is to simplify the path of the bot.
	
	// Duplicate the path list entirely
	a_BotData->NumPivotNodes = a_BotData->NumPathNodes;
	a_BotData->PivotNodes = (B_BotNode_t**)Z_Malloc(sizeof(*a_BotData->PivotNodes) * a_BotData->NumPivotNodes, PU_STATIC, NULL);
	memmove(a_BotData->PivotNodes, a_BotData->PathNodes, sizeof(*a_BotData->PivotNodes) * a_BotData->NumPivotNodes);
	
#if 0
	B_BotNode_t** PivotNodes;					// Pivots in path
	size_t PivotIt;								// Current Pivot
	size_t NumPivotNodes;						// Number of pivot nodes
#endif
	
	/* Return value */
	return RetVal;
}

static fixed_t l_FloorZ, l_CeilingZ;

/* BS_CheckNodeSafeTraverser() -- Checks whether the node is in a "good" spot */
static bool BS_CheckNodeSafeTraverser(intercept_t* in, void* const a_Data)
{
	line_t* li;
	mobj_t* mo;
	
	/* Lines */
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
			
		// Get sector floor and ceiling z
		if (li->frontsector->floorheight > l_FloorZ)
			l_FloorZ = li->frontsector->floorheight;
		if (li->backsector->floorheight > l_FloorZ)
			l_FloorZ = li->backsector->floorheight;
		
		if (li->frontsector->ceilingheight > l_CeilingZ)
			l_CeilingZ = li->frontsector->ceilingheight;
		if (li->backsector->ceilingheight > l_CeilingZ)
			l_CeilingZ = li->backsector->ceilingheight;
		
		// Cannot fit inside of sector?
		if ((l_CeilingZ - l_FloorZ) < (56 << FRACBITS))
			return false;
		
		// Everything seems OK
		return true;
	}
	
	/* Things */
	else
	{
		// Get Object
		mo = in->d.thing;
		
		/* Don't hit solid things */
		if (mo->flags & (MF_SOLID))
			return false;
		
		return true;
	}
}

/* BS_FinalizeSubSector() -- Finalize Subsector */
static bool BS_FinalizeSubSector(subsector_t* const a_SubSector, const fixed_t a_X, const fixed_t a_Y)
{
	subsector_t* SubS;
	fixed_t dx, dy, Pow;
	int x, y, m, i;
	
	/* Check */
	if (!a_SubSector)
		return false;
	
	/* Ignore if already initialized */
	if (a_SubSector->NodesInit)
		return false;
		
	/* Set as initialized */
	a_SubSector->NodesInit = true;
	
	/* Create spread */
	for (m = 1; m <= 3; m++)
	{
		// Get power
		Pow = BOTMINNODEDIST;
		//for (i = 0; i < m; i++)
		//	Pow = FixedMul(Pow, BOTMINNODEDIST);
		
		// Offset away
		for (x = -1; x <= 1; x++)
			for (y = -1; y <= 1; y++)
			{
				// Ignore self
				if (x == 0 && y == 0)
					continue;
				
				// Get difference
				dx = a_X + (Pow * x);
				dy = a_Y + (Pow * y);
				
				// Get subsector at this point
				SubS = R_IsPointInSubsector(dx, dy);
				
				// Nothing here?
				if (!SubS)
					continue;
				
				// Point is in this subsector
				else if (SubS == a_SubSector)
				{
					// Add to node array
					if (g_BotDebug)
					{
						/*P_SpawnMobj(
								dx,
								dy,
								SubS->sector->floorheight,
								INFO_GetTypeByName("ReMooDBotDebugNode")
							);*/
					}
				}
				
				// Point is in another subsector
				else
				{
					// Not initialized? Init then
					if (!SubS->NodesInit)
						BS_FinalizeSubSector(SubS, dx, dy);
				}
			}
	}
	
	/* Success! */
	return true;
}

/* BS_GetNodeAtPos() -- Get node at position */
static B_BotNode_t* BS_GetNodeAtPos(const fixed_t a_X, const fixed_t a_Y, const int32_t a_Rec)
{
	subsector_t* SpotSS = NULL;
	B_BotNode_t* CurNode, *LinkNode;
	B_BotNode_t* ClosestNode;
	fixed_t Dist, CloseDist;
	size_t i;
	int32_t x, y, LinkXID, LinkYID;
	bool Failed;
	fixed_t PlaceX, PlaceY;
	
	/* Round input coordinates to the normal grid */
	// Because otherwise bot nodes will be placed everywhere in un-grid fashion.
	// Also, Some nodes could be placed in adjacent subsectors not very far from
	// an existing node in another subsector, so this changes that.
	PlaceX = ((a_X >> FRACBITS) - ((a_X >> FRACBITS) % (BOTMINNODEDIST >> FRACBITS))) << FRACBITS;
	PlaceY = ((a_Y >> FRACBITS) - ((a_Y >> FRACBITS) % (BOTMINNODEDIST >> FRACBITS))) << FRACBITS;
	
	/* See if spot is in a subector */
	SpotSS = R_IsPointInSubsector(PlaceX, PlaceY);
	
	// No subsector here?
	if (!SpotSS)
		return NULL;
	
	/* Check for initialization */
	if (!SpotSS->NodesInit)
		BS_FinalizeSubSector(SpotSS, PlaceX, PlaceY);
	
	/* Find the closest node here */
	ClosestNode = NULL;
	for (i = 0; i < SpotSS->NumBotNodes; i++)
	{
		// Get current node
		CurNode = (B_BotNode_t*)SpotSS->BotNodes[i];
		
		// Get distance
		Dist = P_AproxDistance(PlaceX - CurNode->Pos[0], PlaceY - CurNode->Pos[1]);
		
		// Distance is close
		if (Dist < BOTMINNODEDIST)
			break;
		
		// Otherwise see if it is better
		if (!ClosestNode || (ClosestNode && Dist < CloseDist))
		{
			// Better now
			ClosestNode = CurNode;
			CloseDist = Dist;
		}
	}
	
	/* Return the closest node */
	return ClosestNode;
	
#if 0
	/* Go through nodes in subsector */
	for (CurNode = NULL, i = 0; i < SpotSS->NumBotNodes; i++)
	{
		// Get current node
		CurNode = SpotSS->BotNodes[i];
		
		// Missing?
		if (!CurNode)
			continue;
		
		// Determine how close the node is
		Dist = P_AproxDistance(PlaceX - CurNode->Pos[0], PlaceY - CurNode->Pos[1]);
		
		// Distance is close
		if (Dist < BOTMINNODEDIST)
			break;
		
		// Forget Node
		CurNode = NULL;
	}
	
	/* Create a new node at this position */
	if (!CurNode)
	{
		Failed = false;
		
		// Get Floor and ceiling Z
		l_FloorZ = SpotSS->sector->floorheight;
		l_CeilingZ = SpotSS->sector->ceilingheight;
		
		// See if it is possible to even create a node here
			// The bots will blindly run to a node it cannot reach because
			// there happens to be a wall where the node is directly attached
			// to it.
		if (!P_PathTraverse(
					PlaceX - (BOTMINNODEDIST >> 1),
					PlaceY - (BOTMINNODEDIST >> 1),
					PlaceX + (BOTMINNODEDIST >> 1),
					PlaceY + (BOTMINNODEDIST >> 1),
					PT_ADDLINES,
					BS_CheckNodeSafeTraverser,
					NULL
				))
			Failed = true;
		
		// Draw line from thing corner (corss section TL to BR)
		if (!P_PathTraverse(
					PlaceX - (BOTMINNODEDIST >> 1),
					PlaceY + (BOTMINNODEDIST >> 1),
					PlaceX + (BOTMINNODEDIST >> 1),
					PlaceY - (BOTMINNODEDIST >> 1),
					PT_ADDLINES,
					BS_CheckNodeSafeTraverser,
					NULL
				))
			Failed = true;
		
		// Create Node
		CurNode = Z_Malloc(sizeof(*CurNode), PU_LEVEL, NULL);
	
		// Set node info
		CurNode->NoTraverse = Failed;
		CurNode->Pos[0] = PlaceX;
		CurNode->Pos[1] = PlaceY;
		CurNode->SubS = SpotSS;
		CurNode->FloorZ = l_FloorZ;
		CurNode->CeilingZ = l_CeilingZ;
		
		// Floating Node?
		CurNode->IsFloating = false;
		
		if (CurNode->FloorZ > CurNode->SubS->sector->floorheight)
			CurNode->IsFloating = true;
	
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
					CurNode->FloorZ,
					(CurNode->NoTraverse ? INFO_GetTypeByName("ReMooDBotDebugBadNode") : (CurNode->IsFloating ? INFO_GetTypeByName("ReMooDBotDebugFloatNode") : INFO_GetTypeByName("ReMooDBotDebugNode")))
				);
		}
	}
	
	/* Recursion to find different nodes */
	// Links need generation?
		// Also, do not generate nodes and recourse more than needed!
#if 0
	if (!CurNode->LinksMade)
	{
		// Set current node as generated
		CurNode->LinksMade = true;
	}
#else
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
							PlaceX + ((BOTMINNODEDIST * x)),
							PlaceY + ((BOTMINNODEDIST * y)),
							a_Rec + 1
						);
			
				// Link was returned?
				if (LinkNode)
				{
					// Link the remote node back to here
					LinkNode->Links[c_LinkOp[LinkXID]][c_LinkOp[LinkYID]] = CurNode;
					
					// Distance between two nodes
					Dist = P_AproxDistance(LinkNode->Pos[0] - CurNode->Pos[0], LinkNode->Pos[1] - CurNode->Pos[1]);
					
					// If the remote node is NOT in the same subsector
						// Determine if it is possible to get to it
						// But you can move to node from both sides, so a to b
						// and b to a MUST be determined.
						// Untraversable nodes can be linked from but not linked to
					if (CurNode->NoTraverse || LinkNode->NoTraverse || LinkNode->SubS != CurNode->SubS)
					{
						// Cannot go to there?
						if (LinkNode->NoTraverse || !BS_NodeToNodePossible(NULL, CurNode, LinkNode, BNTNF_FIRSTTIME))
							CurNode->Links[LinkXID][LinkYID] = NULL;
						else
							CurNode->Dists[LinkXID][LinkYID] = Dist;
						
						// Cannot come from there?
						if (CurNode->NoTraverse || !BS_NodeToNodePossible(NULL, LinkNode, CurNode, BNTNF_FIRSTTIME))
							LinkNode->Links[c_LinkOp[LinkXID]][c_LinkOp[LinkYID]] = NULL;
						else
							LinkNode->Dists[c_LinkOp[LinkXID]][c_LinkOp[LinkYID]] = Dist;
					}
					
					// Just copy distance (same subsector)
					else
					{
						CurNode->Dists[LinkXID][LinkYID] = Dist;
						LinkNode->Dists[c_LinkOp[LinkXID]][c_LinkOp[LinkYID]] = Dist;
					}
				}
			}
	}
	
	/* Return the current node */
	return CurNode;
#endif
#endif
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
	bool Walkable;
	
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
	{
		a_BotData->ActSub = BBAS_BLINDSTRAIGHTNAV;
		return;
	}
	
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
#if 1
		// Change AI
		a_BotData->ActSub = BBAS_NORMALAI;
#else
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
#endif
		
		// Wait for another tic to do the movement
		return;
	}
	
	/* Move in that direction */
	// Timeout determination
	if (!a_BotData->SLToNode)
	{
		a_BotData->SLMoveTimeout = gametic + BOTSLMOVETIMEOUT;
		a_BotData->SLToNode = LinkNode;
	}
	
	// Turn to it
	a_TicCmd->angleturn = c_NavAngle[lopX][lopY] >> 16;
	
	// Walk to it
	a_TicCmd->forwardmove = c_forwardmove[a_BotData->SLMoveSpeed];
	
	// Spawn object there
	if (g_BotDebug)
	{
		P_SpawnMobj(
				LinkNode->Pos[0],
				LinkNode->Pos[1],
				LinkNode->SubS->sector->floorheight + (32 << FRACBITS),
				INFO_GetTypeByName("ReMooDBotDebugTarget")
			);
	}
}

/* BS_ThinkFollowNearestPlayer() -- Follows the nearest player */
static void BS_ThinkFollowNearestPlayer(B_BotData_t* const a_BotData, ticcmd_t* const a_TicCmd)
{
	size_t p, j;
	player_t* Player, *ToPlayer;
	B_BotNode_t* TargetNode;
	uint32_t ItCount;
	angle_t PAng;
	
	/* Check */
	if (!a_BotData || !a_TicCmd)
		return;
	
	/* Find Players */
	// Self
	Player = a_BotData->Player;
	
	/* Bot is nowhere? */
	if (!a_BotData->AtNode)
	{
		a_BotData->ActSub = BBAS_BLINDSTRAIGHTNAV;
		return;
	}
	
	/* No Existing Path */
	if (!a_BotData->NumPathNodes)
	{
		// Find Other players
		for (p = 0; p < MAXPLAYERS; p++)
			if (p != Player - players)
				if (playeringame[p])
				{
					ToPlayer = &players[p];
					break;
				}
		
		if (p >= MAXPLAYERS)
			return;
		
		// No players found?
		if (!Player || !ToPlayer)
			return;
	
		// Get target destination
		TargetNode = l_PlayerNodes[p];
	
		// No target node
		if (!TargetNode)
			return;
		
		// Build path to the player
		ItCount = BS_BuildBotPath(a_BotData, a_BotData->AtNode, TargetNode, 0);
		
		// Reset move time
		a_BotData->SLMoveTimeout = gametic + BOTSLMOVETIMEOUT;
		
		// Straight Line Navigation
		if (!ItCount)
			a_BotData->ActSub = BBAS_STRAIGHTNAV;
	}
	
	/* Existing Path */
	else
	{
		// End of iteration? Clear everything
		if (a_BotData->PivotIt >= a_BotData->NumPivotNodes)
		{
			if (a_BotData->PathNodes)
				Z_Free(a_BotData->PathNodes);
			a_BotData->PathNodes = NULL;
			a_BotData->PathIt = a_BotData->NumPathNodes = 0;
			
			if (a_BotData->PivotNodes)
				Z_Free(a_BotData->PivotNodes);
			a_BotData->PivotNodes = NULL;
			a_BotData->PivotIt = a_BotData->NumPivotNodes = 0;
		}
		
		// Continue down iterator.
		else
		{
			// Spawn Debug here
			if (g_BotDebug)
			{
				if ((gametic % TICRATE) == 0)
				{
					for (j = 0; j < a_BotData->NumPathNodes; j++)
						P_SpawnMobj(
								a_BotData->PathNodes[j]->Pos[0],
								a_BotData->PathNodes[j]->Pos[1],
								a_BotData->PathNodes[j]->SubS->sector->floorheight + (32 << FRACBITS),
								INFO_GetTypeByName("ReMooDBotDebugTarget")
							);
							
					for (j = 0; j < a_BotData->NumPivotNodes; j++)
						P_SpawnMobj(
								a_BotData->PivotNodes[j]->Pos[0],
								a_BotData->PivotNodes[j]->Pos[1],
								a_BotData->PivotNodes[j]->SubS->sector->floorheight + (64 << FRACBITS),
								INFO_GetTypeByName("ReMooDBotDebugPath")
							);
				}
			}
			
			// Turn to it
			a_TicCmd->angleturn = BS_PointsToAngleTurn(
								a_BotData->Player->mo->x,
								a_BotData->Player->mo->y,
								a_BotData->PivotNodes[a_BotData->PivotIt]->Pos[0],
								a_BotData->PivotNodes[a_BotData->PivotIt]->Pos[1]
							);
			
			// Walk to it
			a_TicCmd->forwardmove = c_forwardmove[a_BotData->SLMoveSpeed];
			
			// Reached the target iteration node? or it is taking too long for
				// the bot to get there.
			if ((!a_BotData->AtNode) ||
				(a_BotData->AtNode == a_BotData->PivotNodes[a_BotData->PivotIt]))
			{
				// Iterate up
				a_BotData->PivotIt++;
				
				// Reset Time
				a_BotData->SLMoveTimeout = gametic + BOTSLMOVETIMEOUT;
			}
		}
	}
}

/* BS_ThinkBlindStraightLine() -- Blindly Move in straight line */
static void BS_ThinkBlindStraightLine(B_BotData_t* const a_BotData, ticcmd_t* const a_TicCmd)
{
	int32_t lopX, lopY;
	B_BotNode_t* CurNode, *LinkNode;
	bool Walkable;
	
	/* Check */
	if (!a_BotData || !a_TicCmd)
		return;
	
	/* Direction not set? */
	while (!a_BotData->SLMoveX && !a_BotData->SLMoveY)
	{
		a_BotData->SLMoveX = (BS_Random(a_BotData) % 3) - 1;
		a_BotData->SLMoveY = (BS_Random(a_BotData) % 3) - 1;
	}
	
	/* Move in that direction */
	if (gametic < a_BotData->SLMoveTimeout)
	{
		// Turn to it
		a_TicCmd->angleturn = c_NavAngle[a_BotData->SLMoveX + 1][a_BotData->SLMoveY + 1] >> 16;
	
		// Walk to it
		a_TicCmd->forwardmove = c_forwardmove[a_BotData->SLMoveSpeed];
	}
	
	/* Recycle current direction */
	else
	{
		a_BotData->SLMoveTimeout = gametic + (TICRATE * 3);
		a_BotData->SLMoveX = a_BotData->SLMoveY = 0;
	}
	
	/* If the bot is at a node, change routine */
	if (a_BotData->AtNode)
		a_BotData->ActSub = BBAS_NORMALAI;
}

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
B_BotData_t* B_InitBot(D_NetPlayer_t* const a_NPp)
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
	New = (B_BotData_t*)Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Set Data */
	New->NetPlayer = a_NPp;
	New->Player = New->NetPlayer->Player;
	New->ActSub = BBAS_GHOSTLYAI;//BBAS_FOLLOWNEARESTPLAYER;
	
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
		a_TicCmd->buttons |= BT_USE;
		
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
				
				a_BotData->GHOSTLastChanceTime = 0;
				a_BotData->GHOSTMainPri = BGC_DEFAULT;
				a_BotData->GHOSTSubPri = BGC_DEFAULT;
				a_BotData->GHOSTPickupMo = NULL;
				a_BotData->GHOSTAttackMo = NULL;
				a_BotData->GHOSTBelayPickup = 0;
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

