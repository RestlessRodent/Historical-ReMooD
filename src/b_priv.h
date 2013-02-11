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
// Copyright (C) 2012-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Private Bot Stuff

#ifndef __B_PRIV_H__
#define __B_PRIV_H__

/***************
*** INCLUDES ***
***************/

#include "b_bot.h"
#include "sn_polyg.h"
#include "m_bbox.h"
#include "p_local.h"
#include "r_main.h"
#include "z_zone.h"
#include "doomstat.h"
#include "g_game.h"
#include "p_mobj.h"
#include "t_ini.h"
#include "b_bot.h"

/****************
*** CONSTANTS ***
****************/

#define SHOREKEY UINT32_C(0xC007DEAD)

#define MAXBOTJOBS							16	// Maximum Jobs

#define MAXBOTTARGETS						16	// Max target designated

/* B_GhostCoopMode_t -- Coop Mode */
typedef enum B_GhostCoopMode_e
{
	BGCM_DONTCARE,								// Don't care
	BGCM_MAXKILLS,								// 100% Kills
	BGCM_UVMAX,									// 100% Kills and Secrets
	BGCM_UVALLMAX,								// 100% Kills, Items, Secrets
	BGCM_MAXSECRETS,							// 100% Secrets
	BGCM_MAXITEMS,								// 100% Items
	BGCM_EXITRUN,								// Exit Running
	BGCM_MAXKILLSITEMS,							// 100% Kills, Items
	
	NUMBGHOSTCOOPMODES
} B_GhostCoopMode_t;

/* B_GhostAtkPosture_t -- Attack posture for bot */
typedef enum B_GhostAtkPosture_e
{
	BGAP_DONTCARE,								// Don't Care
	BGAP_DEFENSE,								// Lean twords Defense
	BGAP_MIDDLE,								// Neither offensive or defensive
	BGAP_OFFENSE,								// Lean twords Offence	
		
	NUMBGHOSTATKPOSTURE
} B_GhostAtkPosture_t;

/* B_GhostBotFlags_t -- Flags for bots */
typedef enum B_GhostBotFlags_e
{
	BGBF_SHOOTALLIES	= UINT32_C(0x00000001),	// Shoot through allies
} B_GhostBotFlags_t;


#define BOTMINNODEDIST		(32 << FRACBITS)	// Minimum node distance
#define MAXBGADJDEPTH					32		// Max Adjaceny Depth
#define UNIMATRIXDIV		(32 << FRACBITS)	// Division of the unimatrix
#define UNIMATRIXSIZE					(128)	// Size of the unimatrix

#define UNIMATRIXPHYSSIZE	(FixedMul(UNIMATRIXDIV, UNIMATRIXSIZE << FRACBITS))

static const int8_t c_LinkOp[3] = {2, 1, 0};
static const fixed_t c_forwardmove[2] = { 25, 50 };

/*****************
*** STRUCTURES ***
*****************/

/*****************************************************************************/

#define MAXBOTGOA	64						// Max objects bot is aware of

/* B_BotGOAType_t -- Type of GOA */
typedef enum B_BotGOAType_e
{
	BBGOAT_BARREL,							// Explosive Barrel
	BBGOAT_PICKUP,							// Pickupable
	BBGOAT_ALLY,							// Ally
	BBGOAT_ENEMY,							// Enemy Target
	
	NUMBOTGOATYPES
} B_BotGOAType_t;

/* B_BotGOA_t -- Bot's Global Object Awareness data */
typedef struct B_BotGOA_s
{
	B_BotGOAType_t Type;					// Type of GOA
	thinker_t* Thinker;						// Thinker Pointer
	int32_t Priority;						// How important this thing is
	tic_t FirstSeen;						// First time object noticed
	tic_t ExpireSeen;						// When seeing this thing expires
	bool_t Ignore;							// Ignore like it did not exist
	fixed_t Dist;							// Distance to object (if applicable)
	PI_touch_t* TouchSpec;					// Touch Special
	
	union
	{
		struct
		{
		} Mo;								// Map Object Related
	} Data;									// Thinker specific data
} B_BotGOA_t;

/*****************************************************************************/

typedef struct B_ShoreNode_s B_ShoreNode_t;

/* B_BotTarget_t -- Target of a bot */
typedef struct B_BotTarget_s
{
	bool_t IsSet;							// Target Set
	bool_t MoveTarget;						// Movement target
	tic_t ExpireTic;						// Action expires at this time
	int32_t Priority;						// Priority
	fixed_t x, y;							// X/Y Target
	uintptr_t Key;							// Key
} B_BotTarget_t;

/* B_Bot_t -- GhostlyBots information */
struct B_GhostBot_s
{
	uint8_t Junk;								// Junk Data
	ticcmd_t* TicCmdPtr;						// Pointer to tic command
	bool_t Initted;								// Initialized
	void* AtNode;								// At node
	void* OldNode;								// Old node
	player_t* Player;							// Player
	mobj_t* Mo;									// Mo
	B_BotTemplate_t BotTemplate;				// Template Copy
	struct D_XPlayer_s* XPlayer;				// Bot's XPlayer
	bool_t IsDead;								// Bot is dead?
	tic_t DeathTime;							// Time Died
	int32_t RoamX, RoamY;						// Roaming X/Y
	tic_t RespawnDelay;							// Respawn Delay
	int32_t Lemmings;							// Lemmings Player
	bool_t IsPlayer;							// Bot is a player
	tic_t MonsterForceTic;						// Force attack/move timeout
	bool_t MonsterForce;						// Force attack/move
	
	B_ShoreNode_t** Shore;						// Shore Nodes
	uint32_t NumShore;							// Number of shore nodes
	uint32_t ShoreIt;							// Current shore iterator
	
	B_ShoreNode_t** Work;						// Shore Nodes (Working)
	uint32_t NumWork;							// Number of shore nodes (Working)
	uint32_t WorkIt;							// Current shore iterator (Working)
	
	bool_t (*ConfirmDesireF)(struct B_GhostBot_s* a_Bot);
	int32_t DesireType;							// Type being desired
	mobj_t* DesireMo;							// Object being desired
	
	B_BotTarget_t Targets[MAXBOTTARGETS];		// Bot targets
	
	struct
	{
		bool_t JobHere;							// A Job is here
		bool_t (*JobFunc)(struct B_GhostBot_s* a_Bot, const size_t a_JobID);
		int32_t Priority;						// Job Priority
		uint32_t Sleep;							// Job Sleeping (wait until tic happens)
	} Jobs[MAXBOTJOBS];							// Bot's Jobs
	
	B_BotGOA_t* ActGOA[NUMBOTGOATYPES];			// GOA acting upon
	B_BotGOA_t GOA[MAXBOTGOA];					// Awareness Table
};

/* B_Node_t -- A Node */
typedef struct B_GhostNode_s
{
	fixed_t x;									// X Position
	fixed_t y;									// Y Position
	
	fixed_t FloorZ;								// Z of floor
	fixed_t CeilingZ;							// Z of ceiling
	
	subsector_t* SubS;							// Subsector here
	
	uint32_t CheckID;							// Check ID of node
	
	struct
	{
		struct B_GhostNode_s* Node;				// Node connected to
		fixed_t Dist;							// Distance to node
	} Links[3][3];								// Chain Links
	
	struct B_GhostNode_s* Next;					// Next in link
} B_Node_t;

/* B_Unimatrix_t -- A large grid portion of the map */
typedef struct B_Unimatrix_s
{
	fixed_t StartPos[2];						// Start Positions
	fixed_t EndPos[2];							// End Positions
	
	subsector_t** SubSecs;						// Subsectors in unimatrix
	size_t NumSubSecs;							// Number of subsectors
	
	sector_t** Sectors;							// Sectors inside unimatrix
	size_t NumSectors;							// Number of sectors in it
	
	B_Node_t** Nodes;						// Nodes in unimatrix
	size_t NumNodes;							// Number of those nodes
} B_Unimatrix_t;

/* B_ShoreType_t -- Type of shore node */
typedef enum B_ShoreType_e
{
	BST_HEAD,									// Head Node
	BST_NODE,									// Standard in between node
	BST_TAIL,									// Tail Node
} B_ShoreType_t;

/* B_ShoreNode_t -- Shore navigation node */
struct B_ShoreNode_s
{
	B_ShoreType_t Type;							// Type of node
	fixed_t Pos[3];								// Position in world
	subsector_t* SubS;							// Subsector
	B_Unimatrix_t* UniM;						// Unimatrix
	B_Node_t* BotNode;						// Bot Node
};

/*************
*** LOCALS ***
*************/

// Adjacent Sectors
extern sector_t* (*l_BAdj)[MAXBGADJDEPTH];		// Adjacent sector list
extern size_t* l_BNumAdj;						// Number of adjacent sectors
extern size_t l_BNumSecs;						// Number of sectors
extern size_t l_BBuildAdj;						// Current stage

// Unimatrix
extern fixed_t l_UMBase[2];						// Base unimatrix position
extern int32_t l_UMSize[2];						// Size of the unimatrix
extern B_Unimatrix_t* l_UMGrid;					// Unimatrix Grid
extern size_t l_UMBuild;						// Umimatrix Build Number

// SubSector Mesh
extern bool_t l_SSMCreated;						// Mesh created?
extern fixed_t l_GFloorZ, l_GCeilingZ;			// Scanned floor and ceiling position
extern int32_t l_SSBuildChain;					// Final Stage Chaining
extern bool_t l_SSAllDone;						// Everything is done

extern B_BotTemplate_t** l_BotTemplates;		// Templates
extern size_t l_NumBotTemplates;				// Number of them

extern B_Node_t* l_HeadNode;				// Head Node

extern B_Bot_t** l_LocalBots;				// Bots in game
extern size_t l_NumLocalBots;					// Number of them

/****************
*** FUNCTIONS ***
****************/

/*** B_GHOST.C ***/
int B_Random(B_Bot_t* const a_Bot);

/*** B_NODE.C ***/
uint16_t B_NodePtoAT(const fixed_t a_x1, const fixed_t a_y1, const fixed_t a_x2, const fixed_t a_y2);
void B_NodeLAD(int32_t* const a_OutX, int32_t* const a_OutY, const angle_t a_Angle);
void B_NodeLD(int32_t* const a_OutX, int32_t* const a_OutY, const fixed_t a_X1, const fixed_t a_Y1, const fixed_t a_X2, const fixed_t a_Y2);
bool_t B_NodeNLD(const int32_t a_X1, const int32_t a_Y1, const int32_t a_X2, const int32_t a_Y2);
void B_NodeMoveAim(const fixed_t a_x1, const fixed_t a_y1, const fixed_t a_x2, const fixed_t a_y2, const fixed_t a_AimX, const fixed_t a_AimY, int16_t* const a_AngleTurn, int8_t* const a_Forward, int8_t* const a_Side);
bool_t B_NodeNtoN(B_Bot_t* const a_Bot, B_Node_t* const a_Start, B_Node_t* const a_End, const bool_t a_FirstTime);
B_Node_t* B_NodeAtPos(const fixed_t a_X, const fixed_t a_Y, const fixed_t a_Z, const bool_t a_Any);

void B_ShoreClear(B_Bot_t* a_Bot, const bool_t a_Work);
B_ShoreNode_t* B_ShorePop(B_Bot_t* a_Bot, const bool_t a_Work);
B_ShoreNode_t* B_ShoreAdd(B_Bot_t* a_Bot, const bool_t a_Work, const B_ShoreType_t a_Type, const fixed_t a_X, const fixed_t a_Y, const fixed_t a_Z);
void B_ShoreApprove(B_Bot_t* a_Bot);
bool_t B_ShorePath(B_Bot_t* a_Bot, const fixed_t a_FromX, const fixed_t a_FromY, const fixed_t a_ToX, const fixed_t a_ToY);

/*** B_WORK.C ***/
bool_t B_WorkGOAAct(B_Bot_t* a_Bot, const size_t a_JobID);
bool_t B_WorkGOAUpdate(B_Bot_t* a_Bot, const size_t a_JobID);

bool_t B_WorkShoreMove(B_Bot_t* a_Bot, const size_t a_JobID);
bool_t B_WorkFindGoodies(B_Bot_t* a_Bot, const size_t a_JobID);
bool_t B_WorkRandomNav(B_Bot_t* a_Bot, const size_t a_JobID);
bool_t B_WorkShootStuff(B_Bot_t* a_Bot, const size_t a_JobID);
bool_t B_WorkGunControl(B_Bot_t* a_Bot, const size_t a_JobID);


#endif /* __B_PRIV_H__ */

