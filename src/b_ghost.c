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
// DESCRIPTION: Bot Code

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "b_priv.h"

/**************
*** GLOBALS ***
**************/

bool_t g_BotDebug = false;						// Debugging Bots
fixed_t g_GlobalBoundBox[4];					// Global bounding box
bool_t g_GotBots = false;						// Got a bot?

/*************
*** LOCALS ***
*************/

// Adjacent Sectors
sector_t* (*l_BAdj)[MAXBGADJDEPTH] = NULL;		// Adjacent sector list
size_t* l_BNumAdj = NULL;						// Number of adjacent sectors
size_t l_BNumSecs = 0;							// Number of sectors
size_t l_BBuildAdj = 0;							// Current stage

// Unimatrix
fixed_t l_UMBase[2] = {0, 0};					// Base unimatrix position
int32_t l_UMSize[2] = {0, 0};					// Size of the unimatrix
B_Unimatrix_t* l_UMGrid = NULL;					// Unimatrix Grid
size_t l_UMBuild = 0;							// Umimatrix Build Number

// SubSector Mesh
bool_t l_SSMCreated = false;					// Mesh created?
fixed_t l_GFloorZ, l_GCeilingZ;					// Scanned floor and ceiling position
int32_t l_SSBuildChain = 0;						// Final Stage Chaining
bool_t l_SSAllDone = false;						// Everything is done

B_BotTemplate_t** l_BotTemplates = NULL;		// Templates
size_t l_NumBotTemplates = 0;					// Number of them

B_GhostNode_t* l_HeadNode = NULL;				// Head Node

B_GhostBot_t** l_LocalBots = NULL;				// Bots in game
size_t l_NumLocalBots = 0;						// Number of them

/****************
*** FUNCTIONS ***
****************/

/* B_GHOST_FindTemplate() -- Find template by name */
B_BotTemplate_t* B_GHOST_FindTemplate(const char* const a_Name)
{
	size_t i;
	
	/* Check */
	if (!a_Name)
		return NULL;
	
	/* Search */
	for (i = 0; i < l_NumBotTemplates; i++)
		if (l_BotTemplates[i])
			if (strcasecmp(a_Name, l_BotTemplates[i]->AccountName) == 0)
				return l_BotTemplates[i];
	
	/* None found */
	return NULL;
}

/* B_GHOST_RandomTemplate() -- Loads a random template */
B_BotTemplate_t* B_GHOST_RandomTemplate(void)
{
	size_t i, Count;
	uint32_t LowCount, Hit;
	
	/* Find the lowest bot counts */
	LowCount = UINT32_MAX;
	for (i = 0; i < l_NumBotTemplates; i++)
		if (l_BotTemplates[i])
			if (l_BotTemplates[i]->Count < LowCount)
				LowCount = l_BotTemplates[i]->Count;
	
	/* Count the number of bots matching the low count */
	Count = 0;
	for (i = 0; i < l_NumBotTemplates; i++)
		if (l_BotTemplates[i])
			if (l_BotTemplates[i]->Count == LowCount)
				Count++;
	
	// No bots?
	if (!Count)
		return NULL;
	
	/* Determine random number */
	Hit = M_Random();
	Hit %= Count;
	
	/* Return the associated template */
	for (i = 0; i < l_NumBotTemplates; i++)
		if (l_BotTemplates[i])
			if (l_BotTemplates[i]->Count == LowCount)
				if (Hit == 0)
				{
					l_BotTemplates[i]->Count++;
					return l_BotTemplates[i];
				}
				else
					Hit--;
	
	/* Failure? */
	for (i = 0; i < l_NumBotTemplates; i++)
		if (l_BotTemplates[i])
		{
			l_BotTemplates[i]->Count++;
			return l_BotTemplates[i];
		}
	
	/* Woops! */
	return NULL;
}

/* BS_Random() -- Random Number */
int BS_Random(B_GhostBot_t* const a_Bot)
{
	return M_Random();
}

/* B_GHOST_Think() -- Bot thinker routine */
void B_GHOST_Think(B_GhostBot_t* const a_GhostBot, ticcmd_t* const a_TicCmd)
{
	size_t J, i, j;
	int32_t MoveTarg, AttackTarg;
	bool_t Blip;
	INFO_BotObjMetric_t GunMetric;
	fixed_t TargOff[2], TargDist;
	
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
		
		// Finds Goodies
		a_GhostBot->Jobs[3].JobHere = true;
		a_GhostBot->Jobs[3].JobFunc = BS_GHOST_JOB_FindGoodies;
		
		// Utilize shore paths
		a_GhostBot->Jobs[4].JobHere = true;
		a_GhostBot->Jobs[4].JobFunc = BS_GHOST_JOB_ShoreMove;
		
		// Set as initialized
		a_GhostBot->Initted = true;
	}
	
	/* A spectating bot? */
	if (a_GhostBot->XPlayer)
	{
		// If there is no player, they are spectating
		if (!a_GhostBot->XPlayer->Player)
		{
			if ((gametic & 63) == 0)
				a_TicCmd->Std.buttons |= BT_USE;
			return;
		}
	}
	
	/* Init */
	a_GhostBot->TicCmdPtr = a_TicCmd;
	a_GhostBot->AtNode = B_GHOST_NodeNearPos(a_GhostBot->Mo->x, a_GhostBot->Mo->y, a_GhostBot->Mo->z, true);
	a_GhostBot->IsPlayer = false;
	if (a_GhostBot->Mo->RXFlags[0] & MFREXA_ISPLAYEROBJECT)
		a_GhostBot->IsPlayer = true;
	
	// At new location?
	Blip = false;
	if (a_GhostBot->AtNode != a_GhostBot->OldNode)
	{
		a_GhostBot->OldNode = a_GhostBot->AtNode;
		Blip = true;
	}
	
	/* Go through targets and expire any of them */
	for (i = 0; i < MAXBOTTARGETS; i++)
		if (a_GhostBot->Targets[i].IsSet)
		{
			// Expired?
			if (gametic > a_GhostBot->Targets[i].ExpireTic)
				memset(&a_GhostBot->Targets[i], 0, sizeof(a_GhostBot->Targets[i]));
		}
	
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
		
			
	/* Get metric of current gun */
	GunMetric = 0;
	if (a_GhostBot->IsPlayer)
		GunMetric = a_GhostBot->Player->weaponinfo[a_GhostBot->Player->readyweapon]->BotMetric;
	TargOff[0] = TargOff[1] = 0;
	
	// If we are a monster, then either only attack or move
	if (!a_GhostBot->IsPlayer)
		if (MoveTarg != -1 && AttackTarg != -1)
		{
			// Timed out?
			if (gametic >= a_GhostBot->MonsterForceTic)
			{
				a_GhostBot->MonsterForceTic = gametic + (TICRATE * 4);
				a_GhostBot->MonsterForce = !a_GhostBot->MonsterForce;
			}
			
			// Move or attack?
			if (a_GhostBot->MonsterForce)
				MoveTarg = -1;
			else
				AttackTarg = -1;
		}
	
	// Special Metric?
	if (AttackTarg != -1)
	{
		// Distance to target
		TargDist = P_AproxDistance(
				a_GhostBot->Player->mo->x - a_GhostBot->Targets[AttackTarg].x,
				a_GhostBot->Player->mo->y - a_GhostBot->Targets[AttackTarg].y
			);
		
		// Based on metric
		switch (GunMetric)
		{
				// Inaccurate Plasma
			case INFOBM_SPRAYPLASMA:
				// Not too close
				if (TargDist >= FIXEDT_C(192))
					for (i = 0; i < 2; i++)
						{
							TargOff[i] = BS_Random(a_GhostBot) - 128;
							TargOff[i] = FixedMul(TargOff[i] << FRACBITS, INT32_C(1024));
							TargOff[i] = FixedMul(TargOff[i], FIXEDT_C(48));
						}
				break;
			
				// Un-Handled
			default:
				break;
		}
	}
	
	/* Really close to movement target? */
	if (MoveTarg != -1)
	{
		// Distance to target
		TargDist = P_AproxDistance(
				a_GhostBot->Player->mo->x - a_GhostBot->Targets[MoveTarg].x,
				a_GhostBot->Player->mo->y - a_GhostBot->Targets[MoveTarg].y
			);
			
		// So close that we are in stopping distance
		if (TargDist <= FIXEDT_C(24))
			MoveTarg = -1;
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
			//if (a_GhostBot->Player->pendingweapon < 0 || a_GhostBot->Player->pendingweapon >= NUMWEAPONS)
				a_GhostBot->TicCmdPtr->Std.buttons |= BT_ATTACK;
			a_GhostBot->TicCmdPtr->Std.angleturn =
				BS_PointsToAngleTurn(
						a_GhostBot->Mo->x,
						a_GhostBot->Mo->y,
						a_GhostBot->Targets[AttackTarg].x + TargOff[0],
						a_GhostBot->Targets[AttackTarg].y + TargOff[1]
					);
		}
		
		// Dual movement
		else
		{
			a_GhostBot->TicCmdPtr->Std.buttons |= BT_ATTACK;
			BS_MoveToAndAimAtFrom(
					a_GhostBot->Mo->x, a_GhostBot->Mo->y,
					a_GhostBot->Targets[MoveTarg].x, a_GhostBot->Targets[MoveTarg].y,
					a_GhostBot->Targets[AttackTarg].x + TargOff[0],
					a_GhostBot->Targets[AttackTarg].y + TargOff[1],
					&a_GhostBot->TicCmdPtr->Std.angleturn,
					&a_GhostBot->TicCmdPtr->Std.forwardmove,
					&a_GhostBot->TicCmdPtr->Std.sidemove
				);
		}
	}
}

/* B_RemoveThinker() -- Remove thinker from bot references */
void B_RemoveThinker(thinker_t* const a_Thinker)
{
	int32_t i;
	
	/* Go through list */
	for (i = 0; i < l_NumLocalBots; i++)
		if (l_LocalBots[i])
		{
			if (l_LocalBots[i]->DesireMo == a_Thinker)
				l_LocalBots[i]->DesireMo = NULL;
		}
}

/* B_XDestroyBot() -- Destroys Bot */
void B_XDestroyBot(B_GhostBot_t* const a_BotData)
{
	int32_t i;
	
	/* Check */
	if (!a_BotData)
		return;
	
	/* Clear Shore, if any */
	BS_GHOST_ClearShore(a_BotData, false);
	BS_GHOST_ClearShore(a_BotData, true);
	
	/* Remove from XPlayer */
	if (a_BotData->XPlayer)
		a_BotData->XPlayer->BotData = NULL;
		
	/* Remove from local list */
	for (i = 0; i < l_NumLocalBots; i++)
		if (l_LocalBots[i] == a_BotData)
			l_LocalBots[i] = NULL;
	
	/* Free */
	Z_Free(a_BotData);
}

/* B_InitBot() -- Initializes Bot */
B_GhostBot_t* B_InitBot(const B_BotTemplate_t* a_Template)
{
	B_GhostBot_t* New;
	thinker_t* currentthinker;
	mobj_t* mo;
	int32_t i;
	
	/* Debugging? */
	if (M_CheckParm("-devbots") || M_CheckParm("-devbot") || M_CheckParm("-botdev"))
		g_BotDebug = true;
	
	/* Allocate */
	New = Z_Malloc(sizeof(*New), PU_STATIC, NULL);
	
	/* Set Data */
	// From Template
	if (a_Template)
		memmove(&New->BotTemplate, a_Template, sizeof(New->BotTemplate));
	
	/* Add to local list */
	// Find free spot
	for (i = 0; i < l_NumLocalBots; i++)
		if (!l_LocalBots[i])
		{
			l_LocalBots[i] = New;
			break;
		}
	
	// not found
	if (i >= l_NumLocalBots)
	{
		Z_ResizeArray((void**)&l_LocalBots, sizeof(*l_LocalBots),
			l_NumLocalBots, l_NumLocalBots + 1);
		l_LocalBots[l_NumLocalBots++] = New;
	}
	
	/* Set and return */
	return New;
}

/* B_BotGetTemplateDataPtr() -- Get template by pointer */
B_BotTemplate_t* B_BotGetTemplateDataPtr(B_GhostBot_t* const a_BotData)
{
	/* Check */
	if (!a_BotData)
		return NULL;
	
	/* Return the template used */
	return &a_BotData->BotTemplate;
}

/* B_BuildBotTicCmd() -- Builds tic command for bot */
void B_BuildBotTicCmd(struct D_XPlayer_s* const a_XPlayer, B_GhostBot_t* const a_BotData, ticcmd_t* const a_TicCmd)
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
				if (players[i].XPlayer)
					if (!(players[i].XPlayer->Flags & DXPF_BOT))
						return;
		
		// Otherwise press use
		a_TicCmd->Std.buttons |= BT_USE;
		
		// Don't process anymore
		return;
	}
	
	/* Non-Level? */
	if (gamestate != GS_LEVEL)
		return;
	
	/* Nodes not complete? */
	if (!l_SSAllDone)
		return;
	
	/* Get variables */
	a_BotData->Player = a_XPlayer->Player;
	Player = a_BotData->Player;
	
	// No player?
	if (!Player)
	{
		// Spectating then, so try to join
		a_TicCmd->Std.buttons |= BT_USE;
		return;
	}
	
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
				a_BotData->RespawnDelay = ((((tic_t)BS_Random(a_BotData)) % 2) + 1) * TICRATE;
				
				// Clear things
				BS_GHOST_ClearShore(a_BotData, false);
				BS_GHOST_ClearShore(a_BotData, true);
			}
			
			// Is still dead, wait some seconds to respawn
			else if (gametic > (a_BotData->DeathTime + a_BotData->RespawnDelay))
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
			a_BotData->Player = Player;
			a_BotData->Mo = Player->mo;
			a_BotData->XPlayer = a_XPlayer;
			B_GHOST_Think(a_BotData, a_TicCmd);
			break;
		
			// Unknown
		default:
			break;
	}
}

/**********************************
*** USER DYNAMIC BOT GENERATION ***
**********************************/

/* B_BotCodeOCCB() -- Handles bot coding */
static bool_t B_BotCodeOCCB(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD)
{
	const WL_WADFile_t* WAD;
	const WL_WADEntry_t* Entry;
	WL_ES_t* Stream;
	TINI_Section_t* Sections, *Rover;
	TINI_ConfigLine_t* Config;
	char* Opt;
	char* Val;
	B_BotTemplate_t* Template;
	size_t i;
	
	/* Free old stuff */
	if (l_BotTemplates)
	{
		for (i = 0; i < l_NumBotTemplates; i++)
			if (l_BotTemplates[i])
				Z_Free(l_BotTemplates[i]);
		Z_Free(l_BotTemplates);
	}
	
	l_BotTemplates = NULL;
	l_NumBotTemplates = 0;
	
	/* Find all the bot datas in every WAD */
	// These are named RMD_BOTS
	for (WAD = WL_IterateVWAD(NULL, true); WAD; WAD = WL_IterateVWAD(WAD, true))
	{
		// Attempt location of entry
		Entry = WL_FindEntry(WAD, 0, "RMD_BOTS");
		
		// Not found?
		if (!Entry)
			continue;
		
		// Open stream
		Stream = WL_StreamOpen(Entry);
		
		// Failed?
		if (!Stream)
			continue;
		
		// Check Unicode nature of stream
		WL_StreamCheckUnicode(Stream);
		
		// Find all sections
		Sections = NULL;
		
		// Loop
		do
		{
			Rover = TINI_FindNextSection(Sections, Stream);
			if (!Sections)
				Sections = Rover;
		} while (Rover);
		
		// Go through sections
		for (Rover = Sections; Rover; Rover = Rover->Next)
		{
			// Find existing template?
			Template = B_GHOST_FindTemplate(Rover->Name);
			
			// Not found?
			if (!Template)
			{
				// Allocate
				Z_ResizeArray((void**)&l_BotTemplates, sizeof(*l_BotTemplates),
						l_NumBotTemplates, l_NumBotTemplates + 1);
				Template = l_BotTemplates[l_NumBotTemplates++] =
					Z_Malloc(sizeof(*Template), PU_STATIC, NULL);
				
				// Set Account Name
				strncpy(Template->AccountName, Rover->Name, MAXPLAYERNAME);
				D_ProfFixAccountName(Template->AccountName);
			}
			
			// Begin reading
			Config = TINI_BeginRead(Rover);
			
			// Read config values
			while (TINI_ReadLine(Config, &Opt, &Val))
			{
				// Remove quotes from value
				if (Val[0] == '\"')
					Val++;
				i = strlen(Val);
				if (i > 0 && Val[i - 1] == '\"')
					Val[i - 1] = 0;
				
				// Display Name
				if (strcasecmp(Opt, "displayname") == 0)
					strncpy(Template->DisplayName, Val, MAXPLAYERNAME);
				
				// Skin Color
				else if (strcasecmp(Opt, "skincolor") == 0)
					Template->SkinColor = C_strtou32(Val, NULL, 0) % MAXSKINCOLORS;
					
				// GRB Color
				else if (strcasecmp(Opt, "skinred") == 0)
					Template->RGBSkinColor[0] = C_strtou32(Val, NULL, 0);
				else if (strcasecmp(Opt, "skingreen") == 0)
					Template->RGBSkinColor[1] = C_strtou32(Val, NULL, 0);
				else if (strcasecmp(Opt, "skinblue") == 0)
					Template->RGBSkinColor[2] = C_strtou32(Val, NULL, 0);
				
				// Hexen Class
				else if (strcasecmp(Opt, "hexenclass") == 0)
					strncpy(Template->HexenClass, Val, MAXPLAYERNAME);
				
				// Ignores Ally Check
				else if (strcasecmp(Opt, "shootallies") == 0)
				{
					Template->Flags &= ~BGBF_SHOOTALLIES;
					if (INFO_BoolFromString(Val))
						Template->Flags |= BGBF_SHOOTALLIES;
				}
			}
		}
		
		// Free found sections
		TINI_ClearSections(Sections);
		
		// Close Stream
		WL_StreamClose(Stream);
	}
	
	/* Finished */
	return true;
}

/* B_InitBotCodes() -- Initializes the bot coding */
void B_InitBotCodes(void)
{
	/* Message */
	CONL_OutputUT(CT_BOTS, DSTR_BGHOSTC_BASEINIT, "\n");
	
	/* Bot OCCB */
	if (!WL_RegisterOCCB(B_BotCodeOCCB, WLDCO_BOTSTUFF))
		I_Error("Failed to register Bot OCCB.");
}

/* CLC_DumpNodes() -- Dump nodes to file */
int CLC_DumpNodes(const uint32_t a_ArgC, const char** const a_ArgV)
{
#define BUFSIZE 128
	B_GhostNode_t* Rover, *Link;
	I_File_t* File;
	char Buf[BUFSIZE];
	int x, y;
	
	/* Open File */
	File = I_FileOpen("bnodes", IFM_RWT);
	
	// Failed?
	if (!File)
		return 1;
	
	/* Start at head */
	for (Rover = l_HeadNode; Rover; Rover = Rover->Next)
		for (x = 0; x < 3; x++)
			for (y = 0; y < 3; y++)
			{
				Link = Rover->Links[x][y].Node;
				
				// No link?
				if (!Link)
					continue;
				
				// Move to base position
				snprintf(Buf, BUFSIZE - 1, "move %i.0 %i.0\n",
						Rover->x >> 16, Rover->y >> 16
					);
				I_FileWrite(File, Buf, strlen(Buf));
				
				// Line to link position
				snprintf(Buf, BUFSIZE - 1, "%i.0 %i.0\n",
						Link->x >> 16, Link->y >> 16
					);
				I_FileWrite(File, Buf, strlen(Buf));
			}
	
	/* Dump map lines that are solid */
	// This is to help determine the contour of the level
	snprintf(Buf, BUFSIZE - 1, "\n\"lines\"");
	I_FileWrite(File, Buf, strlen(Buf));
	
	for (x = 0; x < numlines; x++)
	{
		// Not a blocking line?
		if (!(lines[x].flags & ML_BLOCKING))
			continue;
		
		// Move to base position
		snprintf(Buf, BUFSIZE - 1, "move %i.0 %i.0\n",
				lines[x].v1->x >> 16, lines[x].v1->y >> 16
			);
		I_FileWrite(File, Buf, strlen(Buf));
		
		// Line to end position
		snprintf(Buf, BUFSIZE - 1, "%i.0 %i.0\n",
				lines[x].v2->x >> 16, lines[x].v2->y >> 16
			);
		I_FileWrite(File, Buf, strlen(Buf));
	}
	
	/* Close File */
	I_FileClose(File);
	
	/* Suppose it worked */
	return 0;
#undef BUFSIZE
}

