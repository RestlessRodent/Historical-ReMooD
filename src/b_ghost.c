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

/***************
*** INCLUDES ***
***************/

#include "b_bot.h"
#include "z_zone.h"
#include "m_bbox.h"
#include "p_mobj.h"
#include "p_local.h"
#include "r_main.h"
#include "doomstat.h"
#include "g_game.h"

/****************
*** CONSTANTS ***
****************/

#define BOTMINNODEDIST		(32 << FRACBITS)	// Minimum node distance
#define MAXBGADJDEPTH					32		// Max Adjaceny Depth
#define UNIMATRIXDIV		(32 << FRACBITS)	// Division of the unimatrix
#define UNIMATRIXSIZE					(128)	// Size of the unimatrix

#define UNIMATRIXPHYSSIZE	(FixedMul(UNIMATRIXDIV, UNIMATRIXSIZE << FRACBITS))

static const int8_t c_LinkOp[3] = {2, 1, 0};

static const fixed_t c_forwardmove[2] = { 25, 50 };
static const fixed_t c_sidemove[2] = { 24, 40 };
static const fixed_t c_angleturn[3] = { 640, 1280, 320 };	// + slow turn

/*****************
*** STRUCTURES ***
*****************/

/* B_GhostNode_t -- A Node */
typedef struct B_GhostNode_s
{
	fixed_t x;									// X Position
	fixed_t y;									// Y Position
	
	fixed_t FloorZ;								// Z of floor
	fixed_t CeilingZ;							// Z of ceiling
	
	subsector_t* SubS;							// Subsector here
	
	struct
	{
		struct B_GhostNode_s* Node;				// Node connected to
		fixed_t Dist;							// Distance to node
	} Links[3][3];								// Chain Links
} B_GhostNode_t;

/* B_Unimatrix_t -- A large grid portion of the map */
typedef struct B_Unimatrix_s
{
	fixed_t StartPos[2];						// Start Positions
	fixed_t EndPos[2];							// End Positions
	
	subsector_t** SubSecs;						// Subsectors in unimatrix
	size_t NumSubSecs;							// Number of subsectors
	
	sector_t** Sectors;							// Sectors inside unimatrix
	size_t NumSectors;							// Number of sectors in it
	
	B_GhostNode_t** Nodes;						// Nodes in unimatrix
	size_t NumNodes;							// Number of those nodes
} B_Unimatrix_t;

/* B_LineSet_t -- A set of lines */
typedef struct B_LineSet_s
{
	fixed_t vs[2];								// Start
	fixed_t ve[2];								// End
	
	struct B_LineSet_s* Next;					// Next line
	struct B_LineSet_s* Prev;					// Previous line
} B_LineSet_t;

/****************
*** CONSTANTS ***
****************/

/* c_BotTemplates -- Bot template */
static const B_BotTemplate_t c_BotTemplates[] =
{
	{
		0,										// ID
		"GhostlyBot",							// Account Name
		"{2Ghostly{x7cBot",						// Display Name
		0xC,									// Color: Orange
		{0xff, 0x50, 0x00},						// Hex Color
		"",
		BGAP_DEFENSE,							// Posture
		BGCM_UVMAX,								// Coop Mode
		"Mage",									// Hexen Class
	},
	
	{
		1,										// ID
		"FreeBOT",								// Account Name
		"FreeBOT",								// Display Name
		0xF,									// Color: Pink
		{0xff, 0x70, 0x70},						// Hex Color
		"SuperShotgun BFG PlasmaRifle RocketLauncher Chaingun Shotgun Pistol Chainsaw Fist",
		BGAP_MIDDLE,							// Posture
		BGCM_UVALLMAX,							// Coop Mode
		"Cleric",								// Hexen Class
	},
	
	{
		2,										// ID
		"MP2Bot",								// Account Name
		"{9MP{62{BBot",							// Display Name
		0x8,									// Color: Dark Blue
		{0x00, 0x00, 0x80},						// Hex Color
		"BFG Firemace Bloodscourge Wraithverge Quietus SuperShotgun Hellstaff FrostShards Firestorm HammerOfRetribution RocketLauncher EthrealCrossBow ArcOfDeath SerpentStaff TimonsAxe PlasmaRifle PheonixRod Sapphire MaceOfContrition SpikedGauntlets Chaingun DragonClaw Shotgun ElvenWannd Pistol Gauntlets Chainsaw Staff Fist",
		BGAP_OFFENSE,							// Posture
		BGCM_EXITRUN,							// Coop Mode
		"Mage",									// Hexen Class
	},
	
	{
		3,										// ID
		"zearBot",								// Account Name
		"{4zearBot",							// Display Name
		0x0,									// Color: Green
		{0x00, 0xff, 0x00},						// Hex Color
		"SuperShotgun BFG PlasmaRifle RocketLauncher Chaingun Shotgun Pistol Chainsaw Fist",
		BGAP_DEFENSE,							// Posture
		BGCM_MAXKILLSITEMS,						// Coop Mode
		"Fighter",								// Hexen Class
	},
	
	{
		4,										// ID
		"CatoBot",								// Account Name
		"{9CatoBot",							// Display Name
		0xb,									// Color: White
		{0xff, 0xff, 0xff},						// Hex Color
		"",
		BGAP_DONTCARE,							// Posture
		BGCM_DONTCARE,						// Coop Mode
		"Fighter",								// Hexen Class
	},
	
	
	{
		5,										// ID
		"bot512",								// Account Name
		"{abot{x71512",								// Display Name
		0x0,									// Color: Green
		{0x01, 0x54, 0x22},						// Hex Color
		"",										// Weapons
		BGAP_DONTCARE,							// Posture
		BGCM_DONTCARE,						// Coop Mode
		"Random",								// Hexen Class
	},
	
	{
		UINT_MAX,
		NULL,
	}
};

/**************
*** GLOBALS ***
**************/

fixed_t g_GlobalBoundBox[4];					// Global bounding box

/*************
*** LOCALS ***
*************/

// Adjacent Sectors
static sector_t* (*l_BAdj)[MAXBGADJDEPTH] = NULL;	// Adjacent sector list
static size_t* l_BNumAdj = NULL;				// Number of adjacent sectors
static size_t l_BNumSecs = 0;					// Number of sectors
static size_t l_BBuildAdj = 0;					// Current stage

// Unimatrix
static fixed_t l_UMBase[2];						// Base unimatrix position
static int32_t l_UMSize[2];						// Size of the unimatrix
static B_Unimatrix_t* l_UMGrid;					// Unimatrix Grid
static size_t l_UMBuild = 0;					// Umimatrix Build Number

// SubSector Mesh
static bool_t l_SSMCreated = false;				// Mesh created?
static fixed_t l_GFloorZ, l_GCeilingZ;			// Scanned floor and ceiling position
static int32_t l_SSBuildChain = 0;				// Final Stage Chaining

/****************
*** FUNCTIONS ***
****************/

/* B_GHOST_FindTemplate() -- Find template by name */
const B_BotTemplate_t* B_GHOST_FindTemplate(const char* const a_Name)
{
	size_t i;
	
	/* Check */
	if (!a_Name)
		return 0;
	
	/* Search */
	for (i = 0; c_BotTemplates[i].AccountName; i++)
		if (strcasecmp(a_Name, c_BotTemplates[i].AccountName) == 0)
			return &c_BotTemplates[i];
	
	/* None found */
	return 0;
}

/* B_GHOST_RandomTemplate() -- Loads a random template */
const B_BotTemplate_t* B_GHOST_RandomTemplate(void)
{
	size_t i;
	const B_BotTemplate_t* Rand;
	
	/* Count */
	for (i = 0; c_BotTemplates[i].AccountName; i++)
		;
	
	/* Choose random number */
	Rand = B_GHOST_TemplateByID(abs(M_Random()) % i);
	if (Rand)
		return Rand;
	return &c_BotTemplates[0];
}

/* B_GHOST_TemplateByID() -- Finds template by ID number */
const B_BotTemplate_t* B_GHOST_TemplateByID(const uint32_t a_ID)
{
	size_t i;
	
	/* Look in directory */
	for (i = 0; c_BotTemplates[i].AccountName; i++)
		if (c_BotTemplates[i].BotIDNum == a_ID)
			return &c_BotTemplates[i];
	
	/* Not Found */
	return NULL;
}

/* BS_Random() -- Random Number */
static int BS_Random(B_GhostBot_t* const a_Bot)
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

/* BS_UnimatrixAtPos() -- Returns Unimatrix at this position */
B_Unimatrix_t* BS_UnimatrixAtPos(const fixed_t a_X, const fixed_t a_Y)
{
	fixed_t RealX, RealY;
	int32_t Location;
	
	/* Fix Coordinate */
	RealX = a_X - l_UMBase[0];
	RealY = a_Y - l_UMBase[1];
	
	/* Shrink to unimatrix base */
	RealX = FixedDiv(RealX, UNIMATRIXPHYSSIZE);
	RealY = FixedDiv(RealY, UNIMATRIXPHYSSIZE);
	
	// Shift Down
	RealX >>= FRACBITS;
	RealY >>= FRACBITS;
	
	/* Out of bounds? */
	if (RealX < 0 || RealX >= l_UMSize[0] || RealY < 0 || RealY >= l_UMSize[1])
		return NULL;
	
	/* Determine location */
	// Get location
	Location = (RealY * l_UMSize[0]) + RealX;
	
	/* Return it */
	return &l_UMGrid[Location];
}

/* BS_GHOSTCheckNodeTrav() -- Checks whether the node is in a "good" spot */
static bool_t BS_GHOSTCheckNodeTrav(intercept_t* in, void* const a_Data)
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
		if (li->frontsector->floorheight > l_GFloorZ)
			l_GFloorZ = li->frontsector->floorheight;
		if (li->backsector->floorheight > l_GFloorZ)
			l_GFloorZ = li->backsector->floorheight;
		
		if (li->frontsector->ceilingheight > l_GCeilingZ)
			l_GCeilingZ = li->frontsector->ceilingheight;
		if (li->backsector->ceilingheight > l_GCeilingZ)
			l_GCeilingZ = li->backsector->ceilingheight;
		
		// Cannot fit inside of sector?
		if ((l_GCeilingZ - l_GFloorZ) < (56 << FRACBITS))
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

/* B_GHOST_NodeNearPos() -- Get node near position */
B_GhostNode_t* B_GHOST_NodeNearPos(const fixed_t a_X, const fixed_t a_Y, const fixed_t a_Z, const bool_t a_Any)
{
	B_Unimatrix_t* UniMatrix;
	B_GhostNode_t* CurrentNode, *Best;
	fixed_t Dist, ZDist, BestDist, BestZDist;
	size_t i;
	
	/* Get Unimatrix here */
	UniMatrix = BS_UnimatrixAtPos(a_X, a_Y);
	
	/* Nothing here? */
	if (!UniMatrix)
		return NULL;
	
	/* Look through nodes there */
	// Clear best
	Best = NULL;
	BestDist = 0;
	
	// Go through all nodes here
	for (i = 0; i < UniMatrix->NumNodes; i++)
	{
		// Get current node
		CurrentNode = UniMatrix->Nodes[i];
		
		// Get distance
		Dist = P_AproxDistance(a_X - CurrentNode->x, a_Y - CurrentNode->y);
		
		// Too high?
		if (CurrentNode->FloorZ > (a_Z + (24 >> FRACBITS)))
			continue;
		
		// Distance is close (and is in stepping range)
		if (Dist < BOTMINNODEDIST)
			return CurrentNode;
		
		// Better?
		if (!Best || (Best && Dist < BestDist))
		{
			Best = CurrentNode;
			BestDist = Dist;
		}
	}
	
	/* None Found */
	// Return the best candidate
	if (a_Any)
		return Best;
	else
		return NULL;
}

/* B_GHOST_CreateNodeAtPos() -- Creates node at point */
B_GhostNode_t* B_GHOST_CreateNodeAtPos(const fixed_t a_X, const fixed_t a_Y)
{
	B_GhostNode_t* New;
	subsector_t* SubS;
	B_GhostNode_t* Rover;
	size_t i;
	fixed_t RealX, RealY, Dist;
	bool_t Failed;
	B_Unimatrix_t* ThisMatrix;
	B_GhostNode_t* CurrentNode, *NearNode;
	
	/* Init */
	RealX = a_X;
	RealY = a_Y;
	
	/* Truncate the coordinates to a 32-grid */
	RealX &= ~2097151;//~1048575;
	RealY &= ~2097151;//~1048575;
	
	/* See if there is a subsector here */
	// Because this spot could be in the middle of nowhere
	SubS = R_IsPointInSubsector(RealX, RealY);
	
	// If nothing is there, forget it
	if (!SubS)
		return NULL;
	
	// Don't add any nodes that are close to this spot
	NearNode = B_GHOST_NodeNearPos(RealX, RealY, SubS->sector->floorheight, false);
	
	// There was something close by
	if (NearNode)
		return NULL;
	
	/* Get Unimatrix */
	ThisMatrix = BS_UnimatrixAtPos(RealX, RealY);
	
	// No matrix?
	if (!ThisMatrix)
		return NULL;
	
	/* Determine Node Validity */
	// Init
	l_GFloorZ = SubS->sector->floorheight;
	l_GCeilingZ = SubS->sector->ceilingheight;
	Failed = false;
	
	// See if it is possible to even create a node here
	// The bots will blindly run to a node it cannot reach because
	// there happens to be a wall where the node is directly attached
	// to it.
	if (!Failed && !P_PathTraverse(
				RealX - (BOTMINNODEDIST >> 1),
				RealY - (BOTMINNODEDIST >> 1),
				RealX + (BOTMINNODEDIST >> 1),
				RealY + (BOTMINNODEDIST >> 1),
				PT_ADDLINES,
				BS_GHOSTCheckNodeTrav,
				NULL
			))
		Failed = true;
	
	// Draw line from thing corner (corss section TL to BR)
	if (!Failed && !P_PathTraverse(
				RealX - (BOTMINNODEDIST >> 1),
				RealY + (BOTMINNODEDIST >> 1),
				RealX + (BOTMINNODEDIST >> 1),
				RealY - (BOTMINNODEDIST >> 1),
				PT_ADDLINES,
				BS_GHOSTCheckNodeTrav,
				NULL
			))
		Failed = true;
	
	/* Failed to worked? */
	if (Failed)
		return NULL;
	
	/* Create new node */
	New = Z_Malloc(sizeof(*New), PU_BOTS, NULL);
	
	// Init
	New->x = RealX;
	New->y = RealY;
	New->FloorZ = l_GFloorZ;
	New->CeilingZ = l_GCeilingZ;
	New->SubS = SubS;
	
	/* Add to subsector links */
	Z_ResizeArray((void**)&SubS->GhostNodes, sizeof(*SubS->GhostNodes),
		SubS->NumGhostNodes, SubS->NumGhostNodes + 1);
	SubS->GhostNodes[SubS->NumGhostNodes++] = New;
	
	/* Add everything to the unimatrix */
	// Add subsector to unimatrix
	for (i = 0; i < ThisMatrix->NumSubSecs; i++)
		if (ThisMatrix->SubSecs[i] == SubS)
			break;
	
	if (i >= ThisMatrix->NumSubSecs)
	{
		Z_ResizeArray((void**)&ThisMatrix->SubSecs, sizeof(*ThisMatrix->SubSecs),
				ThisMatrix->NumSubSecs, ThisMatrix->NumSubSecs + 1);
		ThisMatrix->SubSecs[ThisMatrix->NumSubSecs++] = SubS;
	}
	
	// Add sector to unimatrix
	for (i = 0; i < ThisMatrix->NumSectors; i++)
		if (ThisMatrix->Sectors[i] == SubS->sector)
			break;
	
	if (i >= ThisMatrix->NumSectors)
	{
		Z_ResizeArray((void**)&ThisMatrix->Sectors, sizeof(*ThisMatrix->Sectors),
				ThisMatrix->NumSectors, ThisMatrix->NumSectors + 1);
		ThisMatrix->Sectors[ThisMatrix->NumSectors++] = SubS->sector;
	}
	
	// Add node to unimatrix
	Z_ResizeArray((void**)&ThisMatrix->Nodes, sizeof(*ThisMatrix->Nodes),
			ThisMatrix->NumNodes, ThisMatrix->NumNodes + 1);
	ThisMatrix->Nodes[ThisMatrix->NumNodes++] = New;
	
	/* Debug */
	/*P_SpawnMobj(
			RealX,
			RealY,
			l_GFloorZ,
			INFO_GetTypeByName("ReMooDBotDebugNode")
		);*/
		
	/* Return the new node */
	return New;
}

/* B_GHOST_RecursiveSplitMap() -- Recursively split the map */
// Normally this would have been for subsector shapes, but that is too much
// of a pain in the ass to do. I cannot seem to get it working at all, so i'm
// giving up on that and instead doing bounding boxes for navigation. It is
// shittier and less precise but it is hell of alot easier to implement!
// Although this is shitter, it is alot faster since people will probably not
// want to wait 5 minutes for polygonal meshes to build. If the map isn't that
// square it will be less accurate. So on levels such as MAP02 it can work good
// but on some very curvy levels, maybe not so much.
bool_t B_GHOST_RecursiveSplitMap(const node_t* const a_Node)
{
	int8_t Side, i, j;
	fixed_t x1, y1, x2, y2;
	
	fixed_t* px, *py;
	fixed_t cx, cy, dx, dy;
	subsector_t* SubS;
	
	/* Check */
	if (!a_Node)
		return false;
	
	/* Go through sides */
	for (i = 0; i < 2; i++)
	{
		// Side is a subsector
		if (a_Node->children[i] & NF_SUBSECTOR)
		{
			// Get bounds for subsector
				// A bounding box consists of four short values (top, bottom,
				// left and right) giving the upper and lower bounds of the y
				// coordinate and the lower and upper bounds of the x coordinate
				// (in that order).
			x1 = a_Node->bbox[i][2];
			y1 = a_Node->bbox[i][1];
			x2 = a_Node->bbox[i][3];
			y2 = a_Node->bbox[i][0];
			
			// Get the center of that box (easy to get)
			cx = FixedDiv(x1 + x2, 2 << FRACBITS);
			cy = FixedDiv(y1 + y2, 2 << FRACBITS);
			
			// Create a node there
			B_GHOST_CreateNodeAtPos(cx, cy);
			
			// Go through each 4 corners and midpoint that and the center
			// this creates a web of sorts
			for (j = 0; j < 4; j++)
			{
				// Which now?
				if ((j & 1) == 0)
					px = &x1;
				else
					px = &x2;
				if (j < 2)
					py = &y1;
				else
					py = &y2;
				
				// Get the midpoint of those between the center
				dx = FixedDiv(*px + cx, 2 << FRACBITS);
				dy = FixedDiv(*py + cy, 2 << FRACBITS);
				
				// Create node there
				B_GHOST_CreateNodeAtPos(dx, dy);
			}
		}
		
		// Side is another node
		else
		{
			// Just recourse into it
			B_GHOST_RecursiveSplitMap(&nodes[a_Node->children[i]]);
		}
	}
	
	/* Success */
	return true;
}

/* BS_GhostNTNPFirst() -- Helps determine whether point is reachable */
// This is done for the initial node creation and as such it also determines
// if a switch or line trigger is needed for activation.
static bool_t BS_GhostNTNPFirst(intercept_t* in, void* const a_Data)
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

/* BS_CheckNodeToNode() -- Checks whether a node can be traveled to */
static bool_t BS_CheckNodeToNode(B_GhostBot_t* const a_Bot, B_GhostNode_t* const a_Start, B_GhostNode_t* const a_End, const bool_t a_FirstTime)
{
	/* Check */
	if (!a_Start || !a_End || (!a_FirstTime && !a_Bot))
		return false;
	
	/* First Time Determination? */
	// This one takes awhile
	if (a_FirstTime)
	{
		// Quick Height Determination
			// Too big a step up
		if (a_End->FloorZ > a_Start->FloorZ + (24 << FRACBITS))
			return false;
			
			// Ceiling height incompatible
		else if ((a_Start->CeilingZ - a_End->FloorZ < (56 << FRACBITS)) ||
				(a_End->CeilingZ - a_Start->FloorZ < (56 << FRACBITS)))
			return false;
		
		// Draw line to node (slower)
		if (!P_PathTraverse(
				a_Start->x,
				a_Start->y,
				a_End->x,
				a_End->y,
				PT_ADDLINES,
				BS_GhostNTNPFirst,
				NULL
			))
			return false;
		
		// Reachable
		return true;
	}
	
	/* Other times */
	else
	{
		return true;
	}
}

/* B_GHOST_Ticker() -- Bot Ticker */
void B_GHOST_Ticker(void)
{
	int32_t i, j, k, zz;
	sector_t* CurSec;
	bool_t DoOK, DoStop;
	sector_t* (*BAdj)[MAXBGADJDEPTH] = NULL;
	sector_t* SecRoverA, *SecRoverB;
	size_t* BNumAdj = NULL;
	B_Unimatrix_t* UniMatrix;
	B_LineSet_t* InitSet, *OldSet, *FirstSet;
	B_GhostNode_t* CurrentNode, *NearNode;
	
	fixed_t x, y, z, dx, dy;
	int32_t uX, uY;
	int8_t lox, loy;
	
	/* Adjanceny chains not yet complete? */
	if (l_BBuildAdj < l_BNumSecs)
	{
		// Debug
		if (g_BotDebug)
			CONL_PrintF("GHOSTBOT: Building chains for %u\n", (unsigned)l_BBuildAdj);
		
		// Build chains
		CurSec = &sectors[l_BBuildAdj];
		BAdj = &l_BAdj[l_BBuildAdj];
		BNumAdj = &l_BNumAdj[l_BBuildAdj];
		
		// Init
		(*BAdj)[0] = CurSec;
		(*BNumAdj) = 1;
		DoStop = false;
		DoOK = true;
		k = 0;
		
		// Loop
		while (k < (*BNumAdj))
		{
			// Go through the "last" check sector
			SecRoverA = (*BAdj)[k++];
	
			// Reset
			DoStop = true;
	
			// Look in list
			if (SecRoverA)
				for (i = 0; i < SecRoverA->NumAdj; i++)
				{
					// Get rover at this pos
					SecRoverB = SecRoverA->Adj[i];
	
					// Make sure it isn't already in our queue
					for (j = 0; j < (*BNumAdj); j++)
						if (SecRoverB == (*BAdj)[j])
							break;
	
					// it wasn't so add it
					if (j >= (*BNumAdj))
					{
						if ((*BNumAdj) < MAXBGADJDEPTH - 1)
							(*BAdj)[(*BNumAdj)++] = SecRoverB;
					}
				}
		}
		
		// Go to the next one, next time
		l_BBuildAdj++;
		return;
	}
	
	/* Build unimatrix */
	// Do 20 at a time (since there are lots of unimatrixes)
	if (l_UMBuild < (l_UMSize[0] * l_UMSize[1]))
	{
		// Debug
		if (g_BotDebug)
			CONL_PrintF("GHOSTBOT: Building unimatrix %u\n", (unsigned)l_UMBuild);
		
		for (zz = 0; zz < 20; zz++)
			// Build them
			if (l_UMBuild < (l_UMSize[0] * l_UMSize[1]))
			{
				// Get Current
				UniMatrix = &l_UMGrid[l_UMBuild];
				
				// Determine which unimatrix this is locationally speaking
				uX = l_UMBuild % l_UMSize[0];
				uY = l_UMBuild / l_UMSize[0];
				
				// Determine Unimatrix Position on the map
				UniMatrix->StartPos[0] = FixedMul(uX << FRACBITS, UNIMATRIXPHYSSIZE) + l_UMBase[0];
				UniMatrix->StartPos[1] = FixedMul(uY << FRACBITS, UNIMATRIXPHYSSIZE) + l_UMBase[1];
				
				// Determine end position
				UniMatrix->EndPos[0] = UniMatrix->StartPos[0] + UNIMATRIXPHYSSIZE;
				UniMatrix->EndPos[1] = UniMatrix->StartPos[1] + UNIMATRIXPHYSSIZE;
				
				// Go to the next one, next time
				l_UMBuild++;
			}
			
		// Return for later processing
		return;
	}
	
	/* Build SubSector Mesh Map */
	if (!l_SSMCreated)
	{
		// Recursive map generation
		B_GHOST_RecursiveSplitMap(&nodes[numnodes - 1]);
		
		// Don't do anything else
		l_SSMCreated = true;
		return;
	}
	
	/* Build node links */
	if (l_SSBuildChain < (l_UMSize[0] * l_UMSize[1]))
	{
		// Debug
		if (g_BotDebug)
			CONL_PrintF("GHOSTBOT: Building links for %u\n", (unsigned)l_SSBuildChain);
		
		// Go through unimatrixes chain
		for (zz = 0; zz < 20; zz++)
			if (l_SSBuildChain < (l_UMSize[0] * l_UMSize[1]))
			{
				// Get Current
				UniMatrix = &l_UMGrid[l_SSBuildChain++];
				
				// Go through all nodes here
				for (i = 0; i < UniMatrix->NumNodes; i++)
				{
					// Get current node
					CurrentNode = UniMatrix->Nodes[i];
					
					// Project all around node to find other nodes
					for (x = -1; x <= 1; x++)
						for (y = -1; y <= 1; y++)
						{
							// Zero?
							if (x == 0 || y == 0)
								continue;
								
							// Project
							dx = CurrentNode->x + (BOTMINNODEDIST * x);
							dy = CurrentNode->y + (BOTMINNODEDIST * y);
							
							// Try and locate nearby nodes
							NearNode = B_GHOST_NodeNearPos(dx, dy, CurrentNode->FloorZ, true);
							
							// Self? Unref
							if (NearNode == CurrentNode)
								NearNode = NULL;
					
							// No node?
							if (!NearNode)
							{
								// Try a deeper reach
								dx = dx + (BOTMINNODEDIST * x);
								dy = dy + (BOTMINNODEDIST * y);
								
								// Try again
								NearNode = B_GHOST_NodeNearPos(dx, dy, CurrentNode->FloorZ, true);
								
								// Self? Unref
								if (NearNode == CurrentNode)
									NearNode = NULL;
								
								// Still nothing
								if (!NearNode)
									continue;
							}
							
							// Check to see if path can be traversed
							if (!BS_CheckNodeToNode(NULL, CurrentNode, NearNode, true))
								continue;
							
							// Movement to node is possible, link it
							lox = x + 1;
							loy = y + 1;
							CurrentNode->Links[lox][loy].Node = NearNode;
							CurrentNode->Links[lox][loy].Dist = P_AproxDistance(
									dx - CurrentNode->x, dy - CurrentNode->y);
							NearNode->Links[c_LinkOp[lox]][c_LinkOp[loy]].Node = CurrentNode;
							NearNode->Links[c_LinkOp[lox]][c_LinkOp[loy]].Dist = CurrentNode->Links[lox][loy].Dist;
						}
				}
			}
		
		// For next time
		return;
	}
}

/* B_GHOST_ClearLevel() -- Clears level */
void B_GHOST_ClearLevel(void)
{
	size_t i, j;
	
	/* Wipe adjacent sector data */
	if (l_BAdj && l_BNumAdj)
	{
		Z_Free(l_BAdj);
		Z_Free(l_BNumAdj);
		l_BAdj = NULL;
		l_BNumAdj = l_BNumSecs = l_BBuildAdj = 0;
	}
	
	/* Destroy the unimatrix */
	if (l_UMGrid)
		Z_Free(l_UMGrid);
	l_UMSize[0] = l_UMSize[1] = 0;
	l_UMBase[0] = l_UMBase[1] = 0;
	
	/* Clear mesh map */
	l_SSMCreated = false;
	l_SSBuildChain = 0;
	
	/* Clear anything extra there may be */
	Z_FreeTags(PU_BOTS, PU_BOTS);
}

/* B_GHOST_InitLevel() -- Initializes level */
void B_GHOST_InitLevel(void)
{
	/* Clear old data */
	B_GHOST_ClearLevel();
	
	/* Prepare for sector initialization */
	l_BNumSecs = numsectors;
	l_BAdj = Z_Malloc(sizeof(*l_BAdj) * l_BNumSecs, PU_BOTS, NULL);
	l_BNumAdj = Z_Malloc(sizeof(*l_BNumAdj) * l_BNumSecs, PU_BOTS, NULL);
	
	/* Prepare the unimatrix */
	// Initialize Base
	l_UMBase[0] = g_GlobalBoundBox[BOXLEFT] & ~(UNIMATRIXDIV - 1);
	l_UMBase[1] = g_GlobalBoundBox[BOXBOTTOM] & ~(UNIMATRIXDIV - 1);
	
	// Determine size
	l_UMSize[0] = ((g_GlobalBoundBox[BOXRIGHT] & ~(UNIMATRIXDIV - 1)) - l_UMBase[0]) / UNIMATRIXSIZE;
	l_UMSize[1] = ((g_GlobalBoundBox[BOXTOP] & ~(UNIMATRIXDIV - 1)) - l_UMBase[1]) / UNIMATRIXSIZE;
	
	// Convert down
	l_UMSize[0] >>= FRACBITS;
	l_UMSize[1] >>= FRACBITS;
	
	// Allocate
	l_UMGrid = Z_Malloc(sizeof(*l_UMGrid) * (l_UMSize[0] * l_UMSize[1]), PU_BOTS, NULL);
	
	/* Debug */
	if (g_BotDebug)
		CONL_PrintF("GHOSTBOT: Unimatrix is [%u, %u] @ (%i, %i)\n",
				l_UMSize[0], l_UMSize[1],
				l_UMBase[0] >> FRACBITS, l_UMBase[1] >> FRACBITS
			);
}

/* BS_GHOST_JOB_RandomNav() -- Random navigation */
static bool_t BS_GHOST_JOB_RandomNav(struct B_GhostBot_s* a_GhostBot, const size_t a_JobID)
{
	B_GhostNode_t* ThisNode;
	B_GhostNode_t* TargetNode;
	int32_t lox, loy, i;
	
	/* Check */
	if (!a_GhostBot)
		return false;
	
	/* Find node in random direction, and move to it */
	while (!a_GhostBot->RoamX && !a_GhostBot->RoamY)
	{
		a_GhostBot->RoamX = BS_Random(a_GhostBot) % 3;
		a_GhostBot->RoamY = BS_Random(a_GhostBot) % 3;
		a_GhostBot->RoamX -= 1;
		a_GhostBot->RoamY -= 1;
	}
	
	/* Get link operation */
	lox = a_GhostBot->RoamX + 1;
	loy = a_GhostBot->RoamY + 1;
	
	/* See if there is a target there */
	// Get current node
	ThisNode = (B_GhostNode_t*)a_GhostBot->AtNode;
	
	// No current node?
	if (!ThisNode)
		return true;
	
	// Get node to try to move to
	TargetNode = ThisNode->Links[lox][loy].Node;
	
	// Try traversing to that node there
	if (TargetNode)
		if (!BS_CheckNodeToNode(a_GhostBot, ThisNode, TargetNode, false))
			TargetNode = NULL;
	
	// No node there? Or we are at that node
	if (!TargetNode || TargetNode == a_GhostBot->AtNode)
	{
		// Increase X pos
		a_GhostBot->RoamX++;
		
		// Increase Y pos
		if (a_GhostBot->RoamX > 1)
		{
			a_GhostBot->RoamX = -1;
			a_GhostBot->RoamY++;
			
			if (a_GhostBot->RoamY > 1)
				a_GhostBot->RoamY = -1;
		}
		
		// Hit zero zero?
		if (!a_GhostBot->RoamX && !a_GhostBot->RoamY)
			a_GhostBot->RoamX++;
		
		// Keep this job
		return true;
	}
	
	/* Set Destination There */
	for (i = 0; i < MAXBOTTARGETS; i++)
		if (!a_GhostBot->Targets[i].IsSet)
		{
			a_GhostBot->Targets[i].IsSet = true;
			a_GhostBot->Targets[i].MoveTarget = true;
			a_GhostBot->Targets[i].ExpireTic = gametic + (TICRATE >> 1);
			a_GhostBot->Targets[i].Priority = 25;
			a_GhostBot->Targets[i].x = TargetNode->x;
			a_GhostBot->Targets[i].y = TargetNode->y;
			break;
		}
	
	/* Sleep Job */
	a_GhostBot->Jobs[a_JobID].Sleep = gametic + (TICRATE >> 1);
	
	/* Keep random navigation */
	return true;
}

/* BS_GHOST_JOB_ShootStuff() -- Shoot Nearby Stuff */
static bool_t BS_GHOST_JOB_ShootStuff(struct B_GhostBot_s* a_GhostBot, const size_t a_JobID)
{
#define CLOSEMOS 8
	int32_t s, i, m;
	sector_t* CurSec;
	mobj_t* Mo;
	mobj_t* ListMos[CLOSEMOS];
	int slope;
	
	/* Sleep Job */
	a_GhostBot->Jobs[a_JobID].Sleep = gametic + (TICRATE >> 1);
	
	/* Clear object list */
	memset(ListMos, 0, sizeof(ListMos));
	m = 0;
	
	/* Go through adjacent sectors */
	// Get current sector
	s = a_GhostBot->Mo->subsector->sector - sectors;
	
	// Start Roving
	for (i = 0; i < l_BNumAdj[s]; i++)
	{
		// Get Current sector here
		CurSec = l_BAdj[s][i];
		
		// Failed?
		if (!CurSec)
			break;
		
		// Go through all things in the chains
		for (Mo = CurSec->thinglist; Mo; Mo = Mo->snext)
		{
			// Object is ourself!
			if (Mo == a_GhostBot->Mo)
				continue;
			
			// Object is missing some flags?
			if (!(Mo->flags & MF_SHOOTABLE))
				continue;
			
			// Object has some flags?
			if (Mo->flags & MF_CORPSE)
				continue;
			
			// Object is dead?
			if (Mo->health <= 0)
				continue;
			
			// Object on same team
			if (P_MobjOnSameTeam(a_GhostBot->Mo, Mo))
				continue;
			
			// Object is not seen?
			if (!P_CheckSight(a_GhostBot->Mo, Mo))
				continue;
			
			// See if autoaim acquires a friendly target
			slope = P_AimLineAttack(a_GhostBot->Mo, a_GhostBot->Mo->angle, MISSILERANGE, NULL);
			
			if (linetarget && P_MobjOnSameTeam(a_GhostBot->Mo, linetarget))
				continue;
			
			// Set in chain
			if (m < CLOSEMOS)
				ListMos[m++] = Mo;
		}
	}
	
	/* Go through objects and update pre-existings */
	for (i = 0; i < MAXBOTTARGETS; i++)
		if (a_GhostBot->Targets[i].IsSet)
			for (s = 0; s < m; s++)
				if (ListMos[s] && a_GhostBot->Targets[i].Key == (uintptr_t)ListMos[s])
				{
					// Update This
					a_GhostBot->Targets[i].ExpireTic += (TICRATE >> 1);
					a_GhostBot->Targets[i].Priority += 10;	// Make it a bit more important
					a_GhostBot->Targets[i].x = ListMos[s]->x;
					a_GhostBot->Targets[i].y = ListMos[s]->y;
					a_GhostBot->Targets[i].Key = (uintptr_t)ListMos[s];
					
					// Force Attacking
					if (a_GhostBot->Player->pendingweapon < 0)
						a_GhostBot->TicCmdPtr->Std.buttons |= BT_ATTACK;
					
					// Clear from current
					ListMos[s] = NULL;
				}
	
	/* Put objects into the target list */
	for (s = 0, i = 0; s < m && i < MAXBOTTARGETS; i++)
		if (ListMos[s])
			if (!a_GhostBot->Targets[i].IsSet)
			{
				// Setup
				a_GhostBot->Targets[i].IsSet = true;
				a_GhostBot->Targets[i].MoveTarget = false;
				a_GhostBot->Targets[i].ExpireTic = gametic + (TICRATE >> 1);
				a_GhostBot->Targets[i].Priority = (-ListMos[s]->health) + 100;
				a_GhostBot->Targets[i].x = ListMos[s]->x;
				a_GhostBot->Targets[i].y = ListMos[s]->y;
				a_GhostBot->Targets[i].Key = (uintptr_t)ListMos[s];
				
				// Force Attacking
				if (a_GhostBot->Player->pendingweapon < 0)
					a_GhostBot->TicCmdPtr->Std.buttons |= BT_ATTACK;
			
				// Update List
				s++;
				break;
			}

//static sector_t* (*l_BAdj)[MAXBGADJDEPTH] = NULL;	// Adjacent sector list
//static size_t* l_BNumAdj = NULL;				// Number of adjacent sectors
//static size_t l_BNumSecs = 0;					// Number of sectors

	return true;
#undef CLOSEMOS
}


/* BS_GHOST_JOB_GunControl() -- Determine weapon changing */
static bool_t BS_GHOST_JOB_GunControl(struct B_GhostBot_s* a_GhostBot, const size_t a_JobID)
{
#define MAXGUNSWITCHERS 32
	int32_t i, b;
	int32_t SwitchChance[MAXGUNSWITCHERS];
	fixed_t AmmoCount;
	ammotype_t AmmoType;
	
	/* Sleep Job */
	a_GhostBot->Jobs[a_JobID].Sleep = gametic << 2;
	
	/* Determine guns to switch to */
	for (i = 0; i < MAXGUNSWITCHERS && i < NUMWEAPONS; i++)
	{
		// Not owned?
		if (!a_GhostBot->Player->weaponowned[i])
		{
			SwitchChance[i] = -1000;
			continue;
		}
		
		// Base chance is weapon power
		SwitchChance[i] = a_GhostBot->Player->weaponinfo[i]->SwitchOrder;
		
		// Get ammo amount
		AmmoType = a_GhostBot->Player->weaponinfo[i]->ammo;
		
		if (AmmoType >= 0 && AmmoType < NUMAMMO)
			AmmoCount = FixedDiv(a_GhostBot->Player->ammo[AmmoType] << FRACBITS,
					a_GhostBot->Player->maxammo[AmmoType] << FRACBITS);
		else
			AmmoCount = 32768;
		
		// Modified by the amount of ammo the weapon holds
		SwitchChance[i] = FixedMul(SwitchChance[i] << FRACBITS, AmmoCount);
	}
	
	/* Find gun to switch to */
	// Most wanted to switch to
	for (i = 0, b = 0; i < MAXGUNSWITCHERS && i < NUMWEAPONS; i++)
		if (SwitchChance[i] > SwitchChance[b])
			b = i;
		
	/* Not using this gun? */
	// Switch to that gun
	if (a_GhostBot->Player->readyweapon != b && a_GhostBot->Player->pendingweapon != b)
	{
		a_GhostBot->TicCmdPtr->Std.buttons |= BT_CHANGE;
		D_TicCmdFillWeapon(a_GhostBot->TicCmdPtr, b);
	}
	
	/* Always keep this job */
	return true;
#undef MAXGUNSWITCHERS
}

/* B_GHOST_Think() -- Bot thinker routine */
void B_GHOST_Think(B_GhostBot_t* const a_GhostBot, ticcmd_t* const a_TicCmd)
{
	size_t J, i, j;
	int32_t MoveTarg, AttackTarg;
	bool_t Blip;
	
	/* Check */
	if (!a_GhostBot || !a_TicCmd)
		return;
	
	/* Clear tic command */
	memset(a_TicCmd, 0, sizeof(*a_TicCmd));
	
	/* Bot needs initialization? */
	if (!a_GhostBot->Initted)
	{
		// Add Random Navigation
		a_GhostBot->Jobs[0].JobHere = true;
		a_GhostBot->Jobs[0].JobFunc = BS_GHOST_JOB_RandomNav;
		
		// Add Shooting things
		a_GhostBot->Jobs[1].JobHere = true;
		a_GhostBot->Jobs[1].JobFunc = BS_GHOST_JOB_ShootStuff;
		
		// Gun Control
		a_GhostBot->Jobs[2].JobHere = true;
		a_GhostBot->Jobs[2].JobFunc = BS_GHOST_JOB_GunControl;
		
		// Randomize Posture
		a_GhostBot->AISpec.Posture = BS_Random(a_GhostBot) % NUMBGHOSTATKPOSTURE;
		
		// Set as initialized
		a_GhostBot->Initted = true;
	}
	
	/* Init */
	a_GhostBot->TicCmdPtr = a_TicCmd;
	a_GhostBot->AtNode = B_GHOST_NodeNearPos(a_GhostBot->Mo->x, a_GhostBot->Mo->y, a_GhostBot->Mo->z, true);
	
	// At new location?
	Blip = false;
	if (a_GhostBot->AtNode != a_GhostBot->OldNode)
	{
		a_GhostBot->OldNode = a_GhostBot->AtNode;
		Blip = true;
	}
	
	/* Debug Movement */
	if (a_GhostBot->AtNode)
	{
		j = 0;
		for (i = 0; i < MAXSPLITSCREEN; i++)
			if (g_Splits[i].Active && g_Splits[i].Console == (a_GhostBot->Player - players))
			{
				j = 1;
				break;
			}
		
		if (j == 1)
		{
			// Blip node handovers
			if (Blip)
				P_SpawnMobj(
						((B_GhostNode_t*)a_GhostBot->AtNode)->x,
						((B_GhostNode_t*)a_GhostBot->AtNode)->y,
						((B_GhostNode_t*)a_GhostBot->AtNode)->FloorZ,
						INFO_GetTypeByName("ReMooDBotDebugPath")
					);
			
			// Targets
			if ((gametic % TICRATE) == 0)
				for (i = 0; i < 3; i++)
					for (j = 0; j < 3; j++)
					{
						if (((B_GhostNode_t*)a_GhostBot->AtNode)->Links[i][j].Node)
						{
							P_SpawnMobj(
									((B_GhostNode_t*)a_GhostBot->AtNode)->Links[i][j].Node->x,
									((B_GhostNode_t*)a_GhostBot->AtNode)->Links[i][j].Node->y,
									((B_GhostNode_t*)a_GhostBot->AtNode)->Links[i][j].Node->FloorZ,
									INFO_GetTypeByName("ReMooDBotDebugTarget")
								);
						}
					}
		}
	}
	
	/* Go through targets and expire any of them */
	for (i = 0; i < MAXBOTTARGETS; i++)
		if (a_GhostBot->Targets[i].IsSet)
			if (gametic > a_GhostBot->Targets[i].ExpireTic)
				memset(&a_GhostBot->Targets[i], 0, sizeof(a_GhostBot->Targets[i]));
	
	/* Go through jobs and execute them */
	for (J = 0; J < MAXBOTJOBS; J++)
		if (a_GhostBot->Jobs[J].JobHere)
		{
			// Sleeping job?
			if (gametic < a_GhostBot->Jobs[J].Sleep)
				continue;
			
			// Execute
			if (a_GhostBot->Jobs[J].JobFunc)
				if (!a_GhostBot->Jobs[J].JobFunc(a_GhostBot, J))
				{
					// Delete job
					a_GhostBot->Jobs[J].JobHere = false;
					a_GhostBot->Jobs[J].Priority = 0;
					a_GhostBot->Jobs[J].Sleep = 0;
					a_GhostBot->Jobs[J].JobFunc = NULL;
				}
		}
	
	/* Move to designated target, shoot designated target */
	// Find the most important target
	MoveTarg = AttackTarg = -1;
	
	// Go through them all
	for (i = 0; i < MAXBOTTARGETS; i++)
		if (a_GhostBot->Targets[i].IsSet)
		{
			// Move target?
			if (a_GhostBot->Targets[i].MoveTarget)
			{
				if ((MoveTarg == -1) ||
					(MoveTarg >= 0 && a_GhostBot->Targets[i].Priority >
						a_GhostBot->Targets[MoveTarg].Priority))
					MoveTarg = i;
			}
			
			// Attack Target
			else
			{
				if ((AttackTarg == -1) ||
					(AttackTarg >= 0 && a_GhostBot->Targets[i].Priority >
						a_GhostBot->Targets[AttackTarg].Priority))
					AttackTarg = i;
			}
		}
	
	/* Commence Movement/Attacking? */
	if (MoveTarg != -1 || AttackTarg != -1)
	{
		// Move to target
		if (MoveTarg != -1 && AttackTarg == -1)
		{
			a_GhostBot->TicCmdPtr->Std.forwardmove = c_forwardmove[1];
			a_GhostBot->TicCmdPtr->Std.angleturn = BS_PointsToAngleTurn(a_GhostBot->Mo->x, a_GhostBot->Mo->y, a_GhostBot->Targets[MoveTarg].x, a_GhostBot->Targets[MoveTarg].y);
		}
		
		// Aim at target
		else if (MoveTarg == -1 && AttackTarg != -1)
		{
			if (a_GhostBot->Player->pendingweapon < 0)
				a_GhostBot->TicCmdPtr->Std.buttons |= BT_ATTACK;
			a_GhostBot->TicCmdPtr->Std.angleturn = BS_PointsToAngleTurn(a_GhostBot->Mo->x, a_GhostBot->Mo->y, a_GhostBot->Targets[AttackTarg].x, a_GhostBot->Targets[AttackTarg].y);
		}
		
		// Dual movement
		else
		{
			a_GhostBot->TicCmdPtr->Std.buttons |= BT_ATTACK;
			BS_MoveToAndAimAtFrom(
					a_GhostBot->Mo->x, a_GhostBot->Mo->y,
					a_GhostBot->Targets[MoveTarg].x, a_GhostBot->Targets[MoveTarg].y,
					a_GhostBot->Targets[AttackTarg].x, a_GhostBot->Targets[AttackTarg].y,
					&a_GhostBot->TicCmdPtr->Std.angleturn,
					&a_GhostBot->TicCmdPtr->Std.forwardmove,
					&a_GhostBot->TicCmdPtr->Std.sidemove
				);
		}
	}
}

